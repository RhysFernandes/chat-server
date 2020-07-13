#include <string>

#include <netinet/in.h>
#include <sys/socket.h>

//
// Client class represent client
//
class Client {
    private:
        std::string username;
        int client_sockfd;
        struct sockaddr_in client_addr;
        socklen_t client_len;

    public:
        Client (
            int client_sockfd,
            struct sockaddr_in client_addr,
            socklen_t client_len
        );

        void set_username (std::string username);

        //
        // Getters
        //
        std::string get_username();
        int get_sockfd ();
        struct sockaddr_in get_client_addr();
        socklen_t get_client_addr_len();
};
