// Room implementation

#include "room.h"

#include <unistd.h>
#include <algorithm>

Room::Room (std::string name, std::shared_ptr <Client> client) :
    name (name) {
    clients.push_back(client);

    std::string msg = client->get_username() +
        " has joined\n";
    message_client (client, msg);
}

std::string Room::get_name () {
    return name;
}

int Room::get_room_size () {
    return this->clients.size ();
}

bool Room::add_client(std::shared_ptr <Client> new_client){
    for (auto &client : clients) {
        //
        // Testing needed to determine desired behaviour.
        //
        if (client == new_client) {
            return false;
        }
    }

    this->clients.push_back(new_client);

    std::string msg = new_client->get_username() +
        " has joined\n";
    message_clients (msg);

    return true;
}

void Room::remove_client (std::shared_ptr <Client> client) {
    this->clients.erase (
        std::remove (
            this->clients.begin(),
            this->clients.end(),
            client
        ),
        this->clients.end()
    );

    std::string msg = client->get_username () + " has left\n";
    message_clients (msg);

    // remove room from rooms here if possible?
}

void Room::message_client (
    std::shared_ptr <Client> client,
    const std::string &msg
) {
    const char *buffer = msg.c_str();

    int n = write(
        client->get_sockfd (),
        buffer,
        msg.length()
    );
}

void Room::message_clients (const std::string &msg) {
    for (auto & client : clients) {
        message_client (client, msg);
    }
}

void Room::send_message (
    std::shared_ptr <Client> client,
    const std::string &msg
) {
    const std::string user_msg = client->get_username () +
        ": " + msg;
    message_clients (user_msg);
}
