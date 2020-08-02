#include <iostream>
#include <unistd.h>

#include "server.h"

void error (const char * msg) {
    std::cerr << msg << std::endl;
    exit(1);
}


Server::Server (int port) : port (port) {

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
    this->server_addr.sin_port = htons (this->port);
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
        ServerThread server (
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

        //
        // Keep threads in scope or else will terminate
        //
        serverThreads.push_back(std::move(server_thread));


        // Nice to have a way to exit server nicely along with threads
        // if (serverThreads.size () == 2) break;

        std::cout << "Success" << std::endl;
    }
    //
    // Close of threads. This is for testing.
    //
    for (auto & sthread : serverThreads){
        sthread.join();
    }

}


    int main(int argc, char *argv[]){
        int port = 1234;
        if (argc >= 3){
            error("Invalid arguments");
        }else if (argc == 2){
            port = atoi(argv[1]);
            if (port > 65535 || port < 0){
                error("Port is not within correct range 0 - 65535");
            }
        }

        Server chat_server = Server(port);
        chat_server.start_server();

}
