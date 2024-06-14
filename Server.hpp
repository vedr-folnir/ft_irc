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
//-------------------------------------------------------//

#define RED "\e[1;31m" 
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
//-------------------------------------------------------//

class Command;

#include "Client.hpp"
#include "chanel.hpp"

class Server 
{
	private:
		int Port;
		int SerSocketFd; //-> server socket file descriptor
		static bool Signal;
		std::vector<chanel> chanelles;
		std::vector<Client> clients;
		std::vector<struct pollfd> fds;
		std::string PassWord;

	public:
		Server(); 

		void ServerInit(char **argv);
		void SerSocket();
		void AcceptNewClient();
		void ReceiveNewData(int fd);

		void SendNewData(int fd, char* buff);
		int GetClientInfo(int fd);
		static void SignalHandler(int signum);
		bool ClientCommand(int fd, std::string command );
		Client& getClientByFd(int fd);
		void initClient(Client &cli, int incofd);
		void sendWelcomeMessages(Client& cli);
		void GetInfoCli(Client &cli, std::string buff);
		void CloseFds();
		void ClearClients(int fd);
		bool already_used_nick(std::string nick);
		//std::vector<chanel> get_chanel(){return chanelles;};
		void add_chanel(std::string nameChanel);

	    std::string serverName;
    	std::string version;
  	  	std::string creationDate;
   		std::string userModes;
	    std::string channelModes;

		void ToSend(int fd, std::string msg);
		void ToSendServer(std::string msg);
        void Ping(int fd, const std::vector<std::string>& parts);
		
		void connecting(Client& client);

        void Join( int fd, const std::vector<std::string>& parts );
        void Msg( int fd, const std::vector<std::string>& parts );
        void Kick( int fd, const std::vector<std::string>& parts );
        void User( int fd, const std::vector<std::string>& parts );
        void Nick( int fd, const std::vector<std::string>& parts );
        void Invite( int fd, const std::vector<std::string>& parts );
        void Topic( int fd, const std::vector<std::string>& parts );
        void Mode( int fd, const std::vector<std::string>& parts );
		//faire une commande pass pls 

        //std::string clearBuff(std::string buff, int start);

};

std::string extractNickname(const std::string& data);
typedef void (Server::*t_f)( int, const std::vector<std::string>& );
