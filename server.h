#include <vector>
#include <memory>

#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>


#include <thread>

// #include "room.h"
#include "serverThread.h"


//
// Server class is the chat server that accepts
// clients and setups rooms for client communication.
//
class Server {
    private:
        int port;
        //
        // rooms should be at higher server level
        // in case more advanced functionality requires
        // connecting rooms in some way.
        //
        // Change to set, look for ubuntu fixes
        std::vector<std::shared_ptr<Room>> rooms;

        int sockfd;
        struct sockaddr_in server_addr;
        socklen_t server_len;

        int client_sockfd;
        struct sockaddr_in client_addr;
        socklen_t client_len;

        std::vector<std::thread> serverThreads;

    public:


        Server (int port);
        void start_server ();

};
