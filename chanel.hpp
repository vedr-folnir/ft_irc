#pragma once

#include <iostream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <poll.h> 
#include <csignal> 
#include <sstream>
#include <cerrno>
#include <cstdlib>
#include <algorithm> 
#include "Client.hpp"
class Client;
//-------------------------------------------------------//

#define RED "\e[1;31m" 
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
//-------------------------------------------------------//

class chanel
{
    private:
        std::vector<Client> ListCli;
        std::string name;

    public:
        chanel(std::string name_arg);
        ~chanel();

        std::string get_name();

        void SetName(std::string newName) {name = newName;}
        void addClient(Client& cli);
};
