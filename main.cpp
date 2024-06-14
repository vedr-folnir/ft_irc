#include "Server.hpp"

//-------------------------------------------------------//

int main(int argc, char **argv)
{
	Server serv;
	std::cout << "---- SERVER ----" << std::endl;
	if (argc != 3)
		throw(std::runtime_error("Usage: <./ircserv> <Port> <password>"));
	try
	{
		signal(SIGINT, Server::SignalHandler); //-> catch the signal (ctrl + c)
		signal(SIGQUIT, Server::SignalHandler); //-> catch the signal (ctrl + \)
		serv.ServerInit(argv); //-> initialize the server
	}
	catch(const std::exception& e)
	{
		serv.CloseFds(); //-> close the file descriptors
		std::cerr << RED << e.what() <<  WHI << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
	return 0;
}