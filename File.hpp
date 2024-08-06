#pragma once

#include <iostream>
#include <fstream>
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
#include "Server.hpp"

class File {
	public:
		std::string	filename;
		std::string Path;
		std::string sender;
		std::string receiver;

	private:
		File();

	public:
		File( std::string Name, std::string Path, std::string Sender, std::string Receiver );
		File( const File &x );
		~File();
		File & operator = ( const File &other );
};