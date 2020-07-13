// Client implementation

#include "client.h"

// Should std::move in constructor?
Client::Client (
    int client_sockfd,
    struct sockaddr_in client_addr,
    socklen_t client_len
) :
    client_sockfd (client_sockfd),
    client_addr (client_addr),
    client_len (client_len)
{

}

void Client::set_username (std::string username){
    this->username = username;
}

std::string Client::get_username (){
    return username;
}

int Client::get_sockfd () {
    return this->client_sockfd;
}

struct sockaddr_in Client::get_client_addr () {
    return this->client_addr;
}

socklen_t Client::get_client_addr_len () {
    return this->client_len;
}
