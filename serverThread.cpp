#include <iostream>
#include <sstream>
#include <unistd.h>

#include "serverThread.h"



// Fix up
void err (const char * msg) {
    std::cerr << msg << std::endl;
    exit(1);
}


ServerThread::ServerThread (
    int client_sockfd,
    struct sockaddr_in client_addr,
    socklen_t client_len
) :
    client (std::make_shared<Client>(client_sockfd, client_addr, client_len)) {

    buffer = new char[max_buffer_size];
}


ServerThread::ServerThread (const ServerThread & other) {
    client = other.client;
    buffer = new char[max_buffer_size];
    memcpy (buffer, other.buffer, sizeof (char) * max_buffer_size);
}


const ServerThread & ServerThread::operator= (const ServerThread & other) {

    if(&other == this){
        return *this;
    }

    client = other.client;

    //
    // No need to delete if size is the same, just copy over.
    //
    memcpy (buffer, other.buffer, sizeof (char) * max_buffer_size);
    return *this;
}


ServerThread::~ServerThread () {
    delete[] buffer;
}


bool ServerThread::validate_name (const char *name) {
    //
    // Loop through string characters and consider valid visible
    // ASCII characters within the range 0 - 127.
    //
    int i = 0;
    while (name[i] != '\0' && i != strlen(name)){
        if (name[i] > 126 || name[i] < 33) return false;
        ++i;
    }
    return true;
}


//
// Respond to Client requests that come in
//
void ServerThread::operator () (
    std::function <std::shared_ptr<Room>
        (std::string name, std::shared_ptr<Client> client)
    > roomHandler
) {

    int n;
    int msg_length = 0;
    char msg_buffer[this->max_buffer_size];
    while (n = read(this->client->get_sockfd(), this->buffer, this->max_buffer_size)) {
        if (n < 0) err ("ERROR reading from socket");
        int buffer_index = msg_length;
        msg_length += n;

        //
        // Limit message to 20,001 bytes, need 1 extra for delimiter.
        // Maybe increase by 1 more and if max is reached then error?
        //
        if (msg_length > 20000) {
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

        memcpy (msg_buffer + buffer_index, this->buffer, n);

        if (this->buffer[n - 1] == '\n') break;

        bzero(this->buffer, this->max_buffer_size);
    }

    //
    // Client disconnects.
    //
    if (msg_length == 0) return;


    msg_buffer[msg_length] = '\0';

    //std::cout << "Command received: " << buffer;
    std::stringstream sstream;
    sstream << msg_buffer;

    std::vector<std::string> words;
    std::string word;
    while (sstream >> word) {
        words.push_back (std::move(word));
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

        close (this->client->get_sockfd ());
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

        close (this->client->get_sockfd ());
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
        bzero (this->buffer, this->max_buffer_size);
        msg_length = 0;
        while (n = read(this->client->get_sockfd(), this->buffer, this->max_buffer_size)) {
            if (n < 0) {
                err ("ERROR reading from socket");
            }
            int buffer_index = msg_length;
            msg_length += n;


            if (msg_length > 20000) {
                std::cerr << "ERROR, request message too long." << std::endl;
                std::string msg = "ERROR: request message too long\n";
                room->send_message (this->client, msg);

                close(this->client->get_sockfd ());
                room->remove_client (this->client);

                return;
            }

            memcpy (msg_buffer + buffer_index, this->buffer, n);

            if (this->buffer[n - 1] == '\n') break;

            bzero(this->buffer, this->max_buffer_size);
        }

        //
        // Client disconnects
        //
        if (msg_length == 0) {
            room->remove_client (this->client);

            // To-do: Get rid of room is needed.
            return;
        }

        msg_buffer[msg_length] = '\0';

        std::string msg {msg_buffer};
        room->send_message (this->client, msg);
    }
}
