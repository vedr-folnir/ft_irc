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

class Client {
    private:
        int Fd;
        std::string IPadd;
        std::string Nickname;
        std::string Username;
        std::string _pass;

    public:
        Client(){}; 
        int GetFd(){return Fd;}
        std::string GetUsername() {return Username;}
        std::string GetNickname() {return Nickname;}
        std::string GetPass() {return _pass;}

        void SetUsername(std::string newUserName) {Username = newUserName;}
        void SetNickname(std::string newNickName){Nickname = newNickName;}
        void SetPass(std::string newPass){_pass = newPass;}
        void SetFd(int fd){Fd = fd;}
        void setIpAdd(std::string ipadd){IPadd = ipadd;}
        
        const std::string getIpAdd();

};