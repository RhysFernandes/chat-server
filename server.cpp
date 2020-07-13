#include <iostream>
#include <sstream>
#include <unistd.h>

#include "server.h"

void error (const char * msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

Server::ServerThread::ServerThread (
    int client_sockfd,
    struct sockaddr_in client_addr,
    socklen_t client_len
) :
    client (std::make_shared<Client>(client_sockfd, client_addr, client_len)) {

    buffer = new char[max_buffer_size];
}

Server::ServerThread::~ServerThread () {

}

bool Server::ServerThread::validate_name (const char *name) {
    //
    // Loop through string characters and consider valid visible
    // ASCII characters within the range 0 - 127.
    //
    int i = 0;
    while (name[i] != '\0'){
        if (name[i] > 126 || name[i] < 33) return false;
        ++i;
    }
    return true;
}

//
// Respond to Client requests that come in
//
void Server::ServerThread::operator () (
    std::function <std::shared_ptr<Room>
        (std::string name, std::shared_ptr<Client> client)
    > roomHandler
) {



    int n = read(this->client->get_sockfd(), this->buffer, this->max_buffer_size);

    if (n < 0) error ("ERROR reading from socket");

    //
    // Client disconnects.
    //
    if (n == 0) return;

    //
    // Limit message to 20,001 bytes, need 1 extra for delimiter.
    // Maybe increase by 1 more and if max is reached then error?
    //
    if (n > 20000) {
        // Server does some logging too.
        std::cerr << "ERROR: Received request message too long" << std::endl;
        std::string msg = "ERROR: request message too long\n";

        n = write(
            this->client->get_sockfd (),
            msg.c_str (),
            msg.length ()
        );

        close(this->client->get_sockfd ());
        return;
    }

    buffer[n] = '\0';

    //std::cout << "Command received: " << buffer;
    std::stringstream sstream;
    sstream << buffer;

    std::vector<std::string> words;
    std::string word;
    while (sstream >> word) {
        words.push_back(std::move(word));
    }

    // Validate JOIN command.
    if (words.size() != 3){
        std::cerr << "ERROR: Received invalid command" << std::endl;
        std::string msg = "ERROR: command invalid\n";

        n = write(
            this->client->get_sockfd (),
            msg.c_str (),
            msg.length ()
        );

        close(this->client->get_sockfd ());
        return;
    }

    //
    // Command error checking
    //
    if ((words[0] != "JOIN") ||
        ! validate_name(words[1].c_str ()) ||
        ! validate_name(words[2].c_str ()))
    {
        std::cerr << "ERROR: Received message too long." << std::endl;
        std::string msg =
            "ERROR: JOIN command or Room_name/username is invalid\n";

        n = write(
            this->client->get_sockfd (),
            msg.c_str (),
            msg.length ()
        );

        close(this->client->get_sockfd ());
        return;
    }

    //
    // After validating command, it is okay
    // set the username.
    //
    client->set_username (words[2]);

    std::cout << "Ok, to join." << std::endl;

    //
    // roomHandler takes in room_name and new client.
    // roomHandler checks if room_name exists and if it doesn't
    // it creates a room and adds the client to it. Else it adds the
    // client to the existing room.
    //
    std::shared_ptr <Room> room = roomHandler (words[1], this->client);


    while (true) {
        bzero(this->buffer, this->max_buffer_size);
        int n = read(this->client->get_sockfd(), this->buffer, this->max_buffer_size);
        if (n < 0) {
            error ("ERROR reading from socket");
        }


        //
        // Client disconnects
        //
        if (n == 0) {
            room->remove_client (this->client);

            // To-do: Get rid of room is needed.
            return;
        }

        if (n > 20000) {
            std::cerr << "ERROR, request message too long." << std::endl;
            std::string msg = "ERROR: request message too long\n";
            room->send_message (this->client, msg);

            close(this->client->get_sockfd ());
            return;
        }
        std::string msg {buffer};
        room->send_message (this->client, msg);
    }
}

Server::Server (int port) : port (std::make_unique<int>(port)) {

}

void Server::start_server () {
    //
    // Setup server
    //
    this->sockfd = socket (AF_INET, SOCK_STREAM, 0);

    if (this->sockfd < 0) error ("ERROR opening socket");
    //
    // Clean up but is not necessary here
    //
    bzero ((char *) &server_addr, sizeof(server_addr));


    this->server_addr.sin_family = AF_INET;
    this->server_addr.sin_addr.s_addr = INADDR_ANY;
    this->server_addr.sin_port = htons (*this->port);
    this->server_len = sizeof (this->server_addr);

    if (
        bind (
            this->sockfd,
            (struct sockaddr *) &server_addr,
            this->server_len
        )
        < 0
    )
    {
        error ("ERROR binding socket");
    }

    int n = getsockname (
        this->sockfd,
        (struct sockaddr *) & this->server_addr,
        & this->server_len
    );
    if (n < 0) error ("ERROR getting socket name");

    //
    // Listen on socket
    //
    listen (this->sockfd, 5);
    std::cout << "Server now listening" << std::endl;

    //
    // Accept new client connections
    //
    while (true) {
        this->client_sockfd = accept (
            this->sockfd,
            (struct sockaddr *) & this->client_addr,
            & this->client_len
        );

        std::cout << "Got a connection" << std::endl;

        //
        // Spawn a server_thread for each client to
        // handle ongoing connections.
        //
        ServerThread server = ServerThread(
            this->client_sockfd,
            this->client_addr,
            this->client_len
        );

        auto roomHandler = [this] (
            std::string name,
            std::shared_ptr <Client> client
        ) -> std::shared_ptr <Room> {
            bool roomExists = false;

            std::shared_ptr <Room> room;
            for (auto & existing_room : this->rooms) {
                if (name == existing_room->get_name ()) {
                    roomExists = true;
                    room = existing_room;

                }
            }
            if (! roomExists) {
                room =
                    std::make_shared<Room> (name, client);

                this->rooms.push_back(room);
            }
            else {
                if (! room->add_client(client)) {
                    std::cerr << "ERROR: could not add client to room" << std::endl;
                }
            }
            return room;
        };

        std::thread server_thread (server, std::ref(roomHandler));
        serverThreads.push_back(std::move(server_thread));

        // Nice to have a way to exit server nicely along with threads
        // server_thread.join();

        std::cout << "Success" << std::endl;
    }


}


int main(int argc, char *argv[]){
    int port = 1234;
    if (argc > 3){
        std::cout << "Invalid arguments" << std::endl;
    }else if (argc == 2){
        port = atoi(argv[1]);
    }

    Server chat_server = Server(port);
    chat_server.start_server();

}
