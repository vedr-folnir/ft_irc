#include "Server.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////
/// CONSTRUCTOR ///
//////////////////////////////////////////////////////////////////////////////////////////////////

Server::Server()
{
	SerSocketFd = -1;
	serverName = "serveur_irc";
	version = "1.0.0";
	creationDate = "12-06-2024";
	userModes = "i";
	channelModes = "m";
	PassWord = "oui"; // a changer avec argv
}


//////////////////////////////////////////////////////////////////////////////////////////////////
/// UTILS ///
//////////////////////////////////////////////////////////////////////////////////////////////////

std::string intToString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string remove_unprintable(std::string str) {
	for (size_t i = 0; i < str.size(); i++){
		if (str[i] < 32 || str[i] > 126) {
			str = str.substr(0, i);
			break;
		}
	}
	return str;
}

void Server::add_chanel(std::string nameChanel) {
	chanelles.push_back(chanel(nameChanel));
}

std::string extractNickname(const std::string& data) {
    std::vector<std::string> tokens;
    std::istringstream stream(data);
    std::string token;

    // Split the data into tokens by whitespace
    while (stream >> token) {
        tokens.push_back(token);
    }

    // Iterate over the tokens to find the "NICK" command and extract the nickname
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == "NICK" && i + 1 < tokens.size()) {
            return tokens[i + 1]; // Return the nickname found after "NICK"
        }
    }

    // If no nickname is found, return an empty string
    return "";
}

bool Server::already_used_nick(std::string nick) {
	for (size_t i=0; i < clients.size(); i++){
		if (nick == clients[i].GetNickname())
			return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// CLIENTS & SOCKETS GESTION ///
//////////////////////////////////////////////////////////////////////////////////////////////////

int Server::GetClientInfo(int fd){
	for(size_t i = 0; i < clients.size(); i++){
		if (fd == clients[i].GetFd())
			return i;
	}
	return -1;
}

// Almost as the Hugo function called "getClientInfo" 
// but return all the Client Infos (class Client)
// not only the fd number - more usefull for Command functions
Client& Server::getClientByFd(int fd) {
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].GetFd() == fd) {
            return clients[i]; // Return the matching client as a reference
        }
    }
    throw std::runtime_error("Client with the specified file descriptor not found.");
}

void Server::ClearClients(int fd){ //-> clear the clients
	for(size_t i = 0; i < fds.size(); i++){ //-> remove the client from the pollfd
		if (fds[i].fd == fd)
			{fds.erase(fds.begin() + i); break;}
	}
	for(size_t i = 0; i < clients.size(); i++){ //-> remove the client from the vector of clients
		if (clients[i].GetFd() == fd)
			{clients.erase(clients.begin() + i); break;}
	}
}

void	Server::CloseFds() {
	for(size_t i = 0; i < clients.size(); i++){ //-> close all the clients
		std::cout << RED << "Client <" << clients[i].GetNickname() << "> Disconnected" << WHI << std::endl;
		close(clients[i].GetFd());
	}
	if (SerSocketFd != -1){ //-> close the server socket
		std::cout << RED << "Server <" << SerSocketFd << "> Disconnected" << WHI << std::endl;
		close(SerSocketFd);
	}
}


void Server::connecting(Client& client){ // see if all info are ok to connect 
	if (!client.GetNickname().empty() && !client.GetUsername().empty() && client.GetPass() == PassWord){
		client.SetReg(true);
		sendWelcomeMessages(Client& cli) 
	}
	else
		client.SetReg(false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// SIGNAL ///
//////////////////////////////////////////////////////////////////////////////////////////////////

bool Server::Signal = false; //-> initialize the static boolean
void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::Signal = true; //-> set the static boolean to true to stop the server
}


//////////////////////////////////////////////////////////////////////////////////////////////////
/// DATA --- SEND & RECEIVE ///
//////////////////////////////////////////////////////////////////////////////////////////////////

void Server::SendNewData(int fd, char* buff)
{
	std::string message = "Client <" + clients[GetClientInfo(fd)].GetNickname() + "> " + buff;
	for(size_t i = 0; i < clients.size(); i++){
		if (fd != clients[i].GetFd())
			send(clients[i].GetFd(), message.c_str() , message.length(), 0);
	}
}

void Server::ReceiveNewData(int fd)
{
	(void) fd;
	char buff[1024]; 
	memset(buff, 0, sizeof(buff));

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	//std::cout << "***" << buff << "***" << std::endl;
	if (bytes <= 0){ //-> check if the client disconnected
		std::cout << RED << "Client <" << clients[GetClientInfo(fd)].GetNickname() << "> Disconnected" << WHI << std::endl;
		ClearClients(fd); //-> clear the client
		close(fd); //-> close the client socket
	}
	
	else { //-> print the received data
		buff[bytes] = '\0';
		ClientCommand(fd, buff);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////
/// COMMANDS ///
//////////////////////////////////////////////////////////////////////////////////////////////////

/// I'm wondering if this is a good way to call these following functions.
/// Are all clients will be independant? With no client conflicts?? (/"this")

bool Server::ClientCommand(int fd, std::string command) 
{
	t_f f[] = { &Server::Ping, &Server::Nick, &Server::User, &Server::Join, &Server::Msg, &Server::Kick, &Server::Invite, &Server::Topic, &Server::Mode};
	command = remove_unprintable(command);
    std::vector<std::string> parts = split(command, ' ');

    if (parts.size() <= 1) {
        std::cout << "IRC: Not enough parameters" << std::endl;
        return false;
    }
    std::string extractedCommand = parts[0];
	for (unsigned long i = 0; i < parts.size(); i++)
	{
		std::cout << "here extracted_cmd*-*" << parts[i] << "*-*" << std::endl;
	}
    std::string ask[10] = {"PING", "NICK", "USER", "JOIN", "PRIVMSG", "KICK", "INVITE", "TOPIC", "MODE"};
    for (int i = 0; i < 10; i++) 
	{

        if (extractedCommand == ask[i])
		{
            (this->*f[i])(fd, parts); // Correct usage without the third parameter
            return true;
		}
	}
    return false;
}

/* /// REGISTRATION
4.1 Connection Registration

   The commands described here are used to register a connection with an
   IRC server as either a user or a server as well as correctly
   disconnect.

   A "PASS" command is not required for either client or server
   connection to be registered, but it must precede the server message
   or the latter of the NICK/USER combination.  It is strongly
   recommended that all server connections have a password in order to
   give some level of security to the actual connections.  The
   recommended order for a client to register is as follows:
           1. Pass message
           2. Nick message
           3. User message
*/

//////////////////////////////////////////////////////////////////////////////////////////////////
/// SERVER INIT & POLLFD ///
//////////////////////////////////////////////////////////////////////////////////////////////////

void Server::GetInfoCli(Client &cli, std::string buff) {
	std::string pass = buff.substr(buff.find("PASS") + 5, PassWord.size());
	std::string nick = buff.substr(buff.find("NICK") + 5,  buff.find("\n") - 2);
	std::string user = buff.substr(buff.find(":",buff.find("USER") + 5) + 1);
	buff = remove_unprintable(buff);

	cli.SetPass(pass);
	cli.SetNickname(nick);
    cli.SetUsername(user);
	/*std::cout << GRE << "-*- pass =" << pass << "-*-"<< WHI <<std::endl;
	std::cout << RED <<  "-*- nick =" << nick << "-*-"<< WHI << std::endl;
	std::cout << YEL << "-*- user =" << user << "-*-"<< WHI << std::endl;*/
}

void Server::initClient(Client &cli, int incofd) {
    cli.SetFd(incofd); //-> set the client file descriptor
    cli.SetUsername(intToString(cli.GetFd()));

    char buffer[1024];

    std::string data;
    ssize_t bytesRead;

    // Lire les données jusqu'à ce qu'il n'y ait plus de données disponibles
    while (true) {
        bytesRead = recv(incofd, buffer, sizeof(buffer) - 1, 0);
		//std::cout << "*+*+*" << buffer << "*+*+*" << std::endl;
        if (bytesRead > 0) 
		{
            buffer[bytesRead] = '\0';
			data = buffer;
        } 
		else if (bytesRead == 0) 
		{
            std::cout << "Client disconnected before sending all data" << std::endl;
            close(incofd);
            return;
        } 
		else if (data.size() > 0)
		{
			GetInfoCli(cli, data);
            if (errno == EWOULDBLOCK || errno == EAGAIN) 
			{
                // Pas plus de données disponibles pour le moment
                break;
            } 
			else 
			{
                std::cerr << "Error reading from client socket" << std::endl;
                close(incofd);
                return;
            }
        }
    }
    // Extraire le nickname des données reçues
    std::string nickname = extractNickname(data);
	/*if (nickname.empty()){ // we already do it in getinfoclient
    	cli.SetNickname(nickname);
    	std::cout << "Client <" << incofd << "> set nickname: " << nickname << std::endl;
	}*/
	connecting(cli);
}

void Server::sendWelcomeMessages(Client& cli) 
{

	/*
:server 001 <nick> :Welcome to the <network> Network, <nick>[!<user>@<host>]
:server 002 <nick> :Your host is <servername>, running version <version>
:server 003 <nick> :This server was created <datetime>
:server 004 <nick> <servername> <version> <available umodes> <available cmodes> [<cmodes with param>]
*/
    std::string WELCOME = ":server 001 " + cli.GetNickname() + " :Welcome to " + serverName + " Network, " + cli.GetNickname() + "[!" + cli.GetUsername() + "@" + cli.getIpAdd() + "]\r\n";
    std::string YOURHOST = ":server 002 " + cli.GetNickname() + " :Your host is " + serverName + ", running version " + version + "\r\n";
    std::string CREATED = ":server 003 " + cli.GetNickname() + " :This server was created " + creationDate + "\r\n";
    std::string MYINFO = ":server 004 " + cli.GetNickname() + " " + serverName + " " + version + " " + userModes + " " + channelModes + "\r\n";

    send(cli.GetFd(), WELCOME.c_str(), WELCOME.size(), 0);
    send(cli.GetFd(), YOURHOST.c_str(), YOURHOST.size(), 0);
    send(cli.GetFd(), CREATED.c_str(), CREATED.size(), 0);
    send(cli.GetFd(), MYINFO.c_str(), MYINFO.size(), 0);
}

void Server::AcceptNewClient()
{
	Client cli; //-> create a new client
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len = sizeof(cliadd);

	int incofd = accept(SerSocketFd, (sockaddr *)&(cliadd), &len); //-> accept the new client
	if (incofd == -1)
	{
		std::cout << "accept() failed" << std::endl;
		return;
	}
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
	{
		std::cout << "fcntl() failed" << std::endl;
		return;
	}
	NewPoll.fd = incofd; //-> add the client socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0; //-> set the revents to 0
    initClient(cli, incofd); // Initialize client and read nickname
	/*
	if (cli.GetNickname().empty()) 
	{
		cli.SetReg(false);
        //return; // Client disconnected or error occurred
    }
	*/
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr))); //-> convert the ip address to string and set it
	clients.push_back(cli); //-> add the client to the vector of clients
	fds.push_back(NewPoll); //-> add the client socket to the pollfd

	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
	std::cout << "Nickname: " << cli.GetNickname() << std::endl;
	
    char buffer[1024];
    //ssize_t bytesRead;
	recv(cli.GetFd(), buffer, sizeof(buffer) - 1, 0);
	std::cout << "new conection msg *-*" << buffer << "*-*" <<std::endl;

	//sendWelcomeMessages(cli);
}

void Server::SerSocket()
{
	int en = 1;
	struct sockaddr_in add;
	struct pollfd NewPoll;
	add.sin_family = AF_INET; //-> set the address family to ipv4
	add.sin_addr.s_addr = INADDR_ANY; //-> set the address to any local machine address
	add.sin_port = htons(this->Port); //-> convert the port to network byte order (big endian)

	SerSocketFd = socket(AF_INET, SOCK_STREAM, 0); //-> create the server socket
	if(SerSocketFd == -1) //-> check if the socket is created
		throw(std::runtime_error("failed to create socket"));

	if(setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));
	 if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("failed to set option (O_NONBLOCK) on socket"));
	if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("failed to bind socket"));
	if (listen(SerSocketFd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() failed"));

	NewPoll.fd = SerSocketFd; //-> add the server socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0; //-> set the revents to 0
	fds.push_back(NewPoll); //-> add the server socket to the pollfd
}

void Server::ServerInit(char **argv)
{
	this->Port = atoi(argv[1]);
	if (this->Port < 1024 || 49152 < this->Port )
		throw(std::logic_error("Wrong Port number [1024 <= Port <= 49152]"));
	
	SerSocket(); //-> create the server socket

	std::cout << GRE << "Server <" << SerSocketFd << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";

	while (Server::Signal == false)
	{ //-> run the server until the signal is received

		if((poll(&fds[0],fds.size(),-1) == -1) && Server::Signal == false) //-> wait for an event
			throw(std::runtime_error("poll() faild"));

		for (size_t i = 0; i < fds.size(); i++){ //-> check all file descriptors
			if (fds[i].revents & POLLIN){ //-> check if there is data to read
				if (fds[i].fd == SerSocketFd)
					AcceptNewClient(); //-> accept new client
				else
					ReceiveNewData(fds[i].fd); //-> receive new data from a registered client
			}
		}
	}
	CloseFds(); //-> close the file descriptors when the server stops
}