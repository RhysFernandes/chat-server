#include <vector>
#include <memory>
#include <mutex>

#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

#include "room.h"

//
// ServerThread class responds to clients'
// requests.
//
class ServerThread {
    private:
        std::shared_ptr<Client> client;

        const int max_buffer_size = 20001;
        char * buffer;

        /*std::vector<std::string> commands = {
            " "
        };*/

        bool validate_name (const char *name);

    public:
        // Make a friends - probably not worth.
        ServerThread(
            int client_sockfd,
            struct sockaddr_in client_addr,
            socklen_t client_len
        );
        ServerThread (const ServerThread & other);
        const ServerThread & operator= (const ServerThread& other);
        /* Should include move const*/
        ~ServerThread ();

        void operator () (
            std::function <std::shared_ptr<Room> (
                    std::string name,
                    std::shared_ptr<Client> client)
                > roomHandler
        );
};
