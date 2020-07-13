#include "client.h"
#include  <vector>
#include <memory>

//
// Room class which contains clients that
// communicate with each other.
//

class Room {
    std::string name;
    std::vector<std::shared_ptr<Client>> clients;

    public:
        Room (std::string name, std::shared_ptr<Client> client);
        bool add_client (std::shared_ptr<Client> new_client);
        void remove_client (std::shared_ptr<Client> client);


        std::string get_name ();

        void message_client (
            std::shared_ptr<Client> new_client,
            const std::string &msg
        );

        void message_clients (const std::string &msg);

        void send_message (
            std::shared_ptr<Client> new_client,
            const std::string &msg
        );
};
