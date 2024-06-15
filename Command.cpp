
#include "Server.hpp" // Include the full Server definition

void Server::User(int fd, const std::vector<std::string>& parts) {
    
    Client& client = getClientByFd(fd); // Use the Server pointer to access Server methods
    std::stringstream message;
    if (!client.GetUsername().empty())
    {
        message << 462 << " ERR_ALREADYREGISTERED :username already registered";
        send(fd, message.c_str(), message.size(), 0);
        return;
    }
    //std::cout << "USER: " << client.GetUsername() << " set for client <" << fd << "> is currently changing his username" << std::endl;
    if (parts.size() < 2) {
        std::cerr << "USER: Not enough parameters" << std::endl;
        return;
    }
    for (size_t i = 1; i < parts.size(); i++)
    {
        message << parts[i] << " ";
        
    }
    client.SetUsername(username.str()); // Update the client's username
    std::cout << "USER: " << username << " set for client <" << fd << ">" << std::endl;
    connecting(client);
}  


/*
4.1.3 User message

    Command: USER
   Parameters: <username> <rvername> hostname> <se<realname>

   The USER message is used at the beginning of connection to specify
   the username, hostname, servername and realname of s new user.  It is
   also used in communication between servers to indicate new user
   arriving on IRC, since only after both USER and NICK have been
   received from a client does a user become registered.

   Between servers USER must to be prefixed with client's NICKname.
   Note that hostname and servername are normally ignored by the IRC
   server when the USER command comes from a directly connected client
   (for security reasons), but they are used in server to server
   communication.  This means that a NICK must always be sent to a
   remote server when a new user is being introduced to the rest of the
   network before the accompanying USER is sent.

   It must be noted that realname parameter must be the last parameter,
   because it may contain space characters and must be prefixed with a
   colon (':') to make sure this is recognised as such.

   Since it is easy for a client to lie about its username by relying
   solely on the USER message, the use of an "Identity Server" is
   recommended.  If the host which a user connects from has such a
   server enabled the username is set to that as in the reply from the
   "Identity Server".

   Numeric Replies:
           ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED

   Examples:
   USER guest tolmoon tolsun :Ronnie Reagan
                                   ; User registering themselves with a
                                   username of "guest" and real name
                                   "Ronnie Reagan".


   :testnick USER guest tolmoon tolsun :Ronnie Reagan
                                   ; message between servers with the
                                   nickname for which the USER command
                                   belongs to
*/

void Server::ToSend(int fd, std::string msg)
{
    std::cout << "SEND-*-:" << msg << "-*-END" << std::endl;
    send(fd, msg.c_str(), msg.size(),0);
    //trow
}

void Server::ToSendServer(std::string msg)
{
    std::cout << "SEND-*-:" << msg << "-*-END" << std::endl;
    for (size_t i = 0; i < clients.size(); i++)
    {
        send(clients[i].GetFd(), msg.c_str(), msg.size(),0);
    }
}

void Server::Ping(int fd, const std::vector<std::string>& parts)
{
    std::string PONG;
    if (parts.size() > 2)
    {
        std::cerr << "error" << std::endl;//fair un trow 
        return;
    }
    else if (parts.size() == 2)
        PONG = "PONG " + parts [1];
    else
        PONG = parts[0];
    ToSend(fd, PONG);
}

void Server::Nick(int fd, const std::vector<std::string>& parts) {
    Client& client = getClientByFd(fd); // Use the Server pointer to access Server methods
    //std::cout << "NICK: " << client.GetNickname() << " set for client <" << fd << "> is currently changing his nickname" << std::endl;
    std::stringstream newNick;
    std::stringstream message; // message to send
    if  (parts.size() < 1){
        //std::cerr << "NICK: Not enough parameters" << std::endl;
        message << 431 << "ERR_NONICKNAMEGIVEN " << newNick << " :nickname not found";
        //sent(fd, message.str().c_str(), message.str().size(), 0);
        ToSend(fd, message.str());
        return;
    }
    for (size_t i = 1; i < parts.size(); i++)
        newNick << parts[i];
    if (newNick == client.GetNickname()){
        message << 447 << "ERR_NONICKCHANGE " << newNick << " :same nickname";
        //sent(fd, message.str().c_str(), message.str().size(), 0);
        ToSend(fd, message.str());
        return;
    }    
    for (size_t i = 0; i < clients.size(); i++){
        if (clients[i].GetNickname() == newNick){
            message << 433 << "ERR_NICKNAMEINUSE " << newNick << " :nickname already used";
            //sent(fd, message.str().c_str(), message.str().size(), 0);
            ToSend(fd, message.str());
            return;
        }
    }
    if (client.GetNickname().empty())
        message << fd << " changed hiss nickname to " << newNick;
    else
        message << client.GetNickname() << " changed hiss nickname to " << newNick;
    client.SetNickname(newNick.str());

    ToSendServer(message);
    if (!client.GetReg())
        connecting(client);
    
}

/*
4.1.2 Nick message

      Command: NICK
   Parameters: <nickname> [ <hopcount> ]

   NICK message is used to give user a nickname or change the previous
   one.  The <hopcount> parameter is only used by servers to indicate
   how far away a nick is from its home server.  A local connection has
   a hopcount of 0.  If supplied by a client, it must be ignored.

   If a NICK message arrives at a server which already knows about an
   identical nickname for another client, a nickname collision occurs.
   As a result of a nickname collision, all instances of the nickname
   are removed from the server's database, and a KILL command is issued
   to remove the nickname from all other server's database. If the NICK
   message causing the collision was a nickname change, then the
   original (old) nick must be removed as well.

   If the server recieves an identical NICK from a client which is
   directly connected, it may issue an ERR_NICKCOLLISION to the local
   client, drop the NICK command, and not generate any kills.

   Numeric Replies:

           ERR_NONICKNAMEGIVEN             ERR_ERRONEUSNICKNAME
           ERR_NICKNAMEINUSE               ERR_NICKCOLLISION

   Example:

   NICK Wiz                        ; Introducing new nick "Wiz".
   :WiZ NICK Kilroy                ; WiZ changed his nickname to Kilroy.
*/

/*
4.1.1 Password message


      Command: PASS
   Parameters: <password>

   The PASS command is used to set a 'connection password'.  The
   password can and must be set before any attempt to register the
   connection is made.  Currently this requires that clients send a PASS
   command before sending the NICK/USER combination and servers *must*
   send a PASS command before any SERVER command.  The password supplied
   must match the one contained in the C/N lines (for servers) or I
   lines (for clients).  It is possible to send multiple PASS commands
   before registering but only the last one sent is used for
   verification and it may not be changed once registered.  Numeric
   Replies:

           ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED

   Example:

           PASS secretpasswordhere
*/

void Server:Pass(int fd, const std::vector<std::string>& parts){
    Client& client = getClientByFd(fd); // Use the Server pointer to access Server methods
    std::stringstream message;
    std::stringstream newPass;
    if (client.GetReg())
    {
        message << 462 << " ERR_ALREADYREGISTERED :client already registered";
        //send(fd, message.c_str(), message.size(), 0);
        ToSend(fd, message.str());
        return;
    }
    if  (parts.size() < 1){
        //std::cerr << "NICK: Not enough parameters" << std::endl;
        message << 461 << " ERR_NEEDMOREPARAMS PASS :passs not found";
        //sent(fd, message.str().c_str(), message.str().size(), 0);
        ToSend(fd, message.str());
        return;
    }
    for (size_t i = 1; i < parts.size(); i++)
        newPass << parts[i];
    client.SetPass(newPass.str());
    connecting(client);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// COMMANDS FOR REGISTERED CLIENTS ///
//////////////////////////////////////////////////////////////////////////////////////////////////
/*
std::string Server::clearBuff(std::string buff, int start)
{
    std::string newbuff;
 	for (size_t i = 0; i < buff.size(); i++){
		if (buff[i] < 32 || buff[i] > 126)
		{
			newbuff = buff.substr(start, i - 1);
			break;
		}
	}
    return(newbuff);
}
*/
/*
JOIN
#oui
*/

void Server::Join(int fd, const std::vector<std::string>& parts) { //http://abcdrfc.free.fr/rfc-vf/rfc1459.html#421 
    if (!getClientByFd(fd).GetReg())
    {
        std::stringstream message;
        message << 451 << "ERR_NOTREGISTERED :client not register";
        //sent(fd, message.str().c_str(), message.str().size(), 0);
        ToSent(fd, message.str());
        return;
    }
    std::cout << "hereee join" << std::endl;
    std::string NameChanel;
    bool chanelExiste = false;
    if (parts[0] == "INVITE")
        NameChanel = parts[2];
    else
        NameChanel = parts[1];
    std::cout << "here chanel..." << NameChanel << "..." << std::endl;
    for (size_t i = 0; i < chanelles.size(); i++)
    {
        if (chanelles[i].get_name() == NameChanel)
        {
            chanelExiste = true;
            // fair join le nouveau use
            break;
        }
    }
    if (chanelExiste == false)
    {
        std::cout << "UUUuuuuuUuuuuuuuuu !" << std::endl;
        chanel c(NameChanel);
        chanelles.push_back(c);
 /*     C  ->  JOIN #test
        S <-   :dan!~d@0::1 JOIN #test
        S <-   :irc.example.com MODE #test +nt
        S <-   :irc.example.com 353 dan = #test :@dan
        S <-   :irc.example.com 366 dan #test :End of /NAMES list.
*/
        std::cout << "---------------- !" << std::endl;
        Client temp = getClientByFd(fd);
        std::cout << "???????? !" << std::endl;
        std::string joinMsg = ":" + temp.GetNickname() + " JOIN #" + NameChanel + "\r\n";
        //std::string modeMsg;
        //std::string topicMsg; //331 RPL_TOPIC, RPL_NAMREPLY, RPL_ENDOFNAMES
        //std::string memberListMsg; //rpl 353
        //std::string ...Msg; // rpl366
        ToSend(fd, joinMsg);
    }
//    std::string WELCOME = ":server 001 " + cli.GetNickname() + " :Welcome to " + serverName + " Network, " + cli.GetNickname() + "[!" + cli.GetUsername() + "@" + cli.getIpAdd() + "]\r\n";

}

/*
4.2.1 Join message

      Command: JOIN
   Parameters: <channel>{,<channel>} [<key>{,<key>}]

   The JOIN command is used by client to start listening a specific
   channel. Whether or not a client is allowed to join a channel is
   checked only by the server the client is connected to; all other
   servers automatically add the user to the channel when it is received
   from other servers.  The conditions which affect this are as follows:

           1.  the user must be invited if the channel is invite-only;

           2.  the user's nick/username/hostname must not match any
               active bans;

           3.  the correct key (password) must be given if it is set.

   These are discussed in more detail under the MODE command (see
   section 4.2.3 for more details).

   Once a user has joined a channel, they receive notice about all
   commands their server receives which affect the channel.  This
   includes MODE, KICK, PART, QUIT and of course PRIVMSG/NOTICE.  The
   JOIN command needs to be broadcast to all servers so that each server
   knows where to find the users who are on the channel.  This allows
   optimal delivery of PRIVMSG/NOTICE messages to the channel.

   If a JOIN is successful, the user is then sent the channel's topic
   (using RPL_TOPIC) and the list of users who are on the channel (using
   RPL_NAMREPLY), which must include the user joining.

   Numeric Replies:
           ERR_NEEDMOREPARAMS              ERR_BANNEDFROMCHAN
           ERR_INVITEONLYCHAN              ERR_BADCHANNELKEY
           ERR_CHANNELISFULL               ERR_BADCHANMASK
           ERR_NOSUCHCHANNEL               ERR_TOOMANYCHANNELS
           RPL_TOPIC

   Examples:
   JOIN #foobar                    ; join channel #foobar.
   JOIN &foo fubar                 ; join channel &foo using key "fubar".
   JOIN #foo,&bar fubar            ; join channel #foo using key "fubar"
                                   and &bar using no key.
   JOIN #foo,#bar fubar,foobar     ; join channel #foo using key "fubar".
                                   and channel #bar using key "foobar".
   JOIN #foo,#bar                  ; join channels #foo and #bar.
   :WiZ JOIN #Twilight_zone        ; JOIN message from WiZ
*/



void Server::Msg(int fd, const std::vector<std::string>& parts) {
    (void)fd;
    (void)parts;
    if (!getClientByFd(fd).GetReg())
    {
        std::stringstream message;
        message << 451 << "ERR_NOTREGISTERED :client not register";
        //sent(fd, message.str().c_str(), message.str().size(), 0);
        ToSent(fd, message.str());
        return;
    }
    std::string message = "< " + getClientByFd(fd).GetNickname() + " >";
    for (size_t i = 2; i < parts.size(); i++)
    {
        message.append(parts[i]);
        message.append(" ");
    }
    std::cout << GRE << message << WHI << std::endl;
    for (size_t i = 0; i < clients.size(); i++)
    {
    	std::cout << RED << clients[i].GetNickname() << "\t" << parts[1] << WHI << std::endl;
        if (parts[1] == clients[i].GetNickname())
        {
        	std::cout << YEL << clients[i].GetFd() << message  << message.size()<< WHI << std::endl;
            send(clients[i].GetFd(), message.c_str(), message.size(), 0);
            break;
        }
        if (i == clients.size() - 1)
            std::cout << "not found" <<std::endl;
    }

}
/*
4.4.1 Private messages

      Command: PRIVMSG
   Parameters: <receiver>{,<receiver>} <text to be sent>

   PRIVMSG is used to send private messages between users.  <receiver>
   is the nickname of the receiver of the message.  <receiver> can also
   be a list of names or channels separated with commas.

   The <receiver> parameter may also me a host mask  (#mask)  or  server
   mask  ($mask).   In  both cases the server will only send the PRIVMSG
   to those who have a server or host matching the mask.  The mask  must
   have at  least  1  (one)  "."  in it and no wildcards following the
   last ".".  This requirement exists to prevent people sending messages
   to  "#*"  or "$*",  which  would  broadcast  to  all  users; from
   experience, this is abused more than used responsibly and properly.
   Wildcards are  the  '*' and  '?'   characters.   This  extension  to
   the PRIVMSG command is only available to Operators.

   Numeric Replies:

           ERR_NORECIPIENT                 ERR_NOTEXTTOSEND
           ERR_CANNOTSENDTOCHAN            ERR_NOTOPLEVEL
           ERR_WILDTOPLEVEL                ERR_TOOMANYTARGETS
           ERR_NOSUCHNICK
           RPL_AWAY

   Examples:

:Angel PRIVMSG Wiz :Hello are you receiving this message ?
                                ; Message from Angel to Wiz.

PRIVMSG Angel :yes I'm receiving it !receiving it !'u>(768u+1n) .br ;
                                Message to Angel.

PRIVMSG jto@tolsun.oulu.fi :Hello !
                                ; Message to a client on server
                                tolsun.oulu.fi with username of "jto".

PRIVMSG $*.fi :Server tolsun.oulu.fi rebooting.
                                ; Message to everyone on a server which
                                has a name matching *.fi.

PRIVMSG #*.edu :NSFNet is undergoing work, expect interruptions
                                ; Message to all users who come from a
                                host which has a name matching *.edu.
*/


void Server::Kick(int fd, const std::vector<std::string>& parts) {
    (void)fd;
    (void)parts;
    if (!getClientByFd(fd).GetReg())
    {
        std::stringstream message;
        message << 451 << "ERR_NOTREGISTERED :client not register";
        //sent(fd, message.str().c_str(), message.str().size(), 0);
        ToSent(fd,message.str());
        return;
    }
}
/*
4.2.8 Kick command

      Command: KICK
   Parameters: <channel> <user> [<comment>]

   The KICK command can be  used  to  forcibly  remove  a  user  from  a
   channel.   It  'kicks  them  out'  of the channel (forced PART).
   Only a channel operator may kick another user out of a  channel.
   Each  server that  receives  a KICK message checks that it is valid
   (ie the sender is actually a  channel  operator)  before  removing
   the  victim  from  the channel.

   Numeric Replies:
           ERR_NEEDMOREPARAMS              ERR_NOSUCHCHANNEL
           ERR_BADCHANMASK                 ERR_CHANOPRIVSNEEDED
           ERR_NOTONCHANNEL

   Examples:
            KICK &Melbourne Matthew         ; Kick Matthew from &Melbourne

            KICK #Finnish John :Speaking English
                                            ; Kick John from #Finnish using
                                            "Speaking English" as the reason
                                            (comment).

            :WiZ KICK #Finnish John         ; KICK message from WiZ to remove John
                                            from channel #Finnish

    NOTE:
        It is possible to extend the KICK command parameters to the
    following:

    <channel>{,<channel>} <user>{,<user>} [<comment>]
*/
/*
void Server::addClientToChannel(std::string nickname, std::string channel) {
    break ;
}
*/
void Server::Invite(int fd, const std::vector<std::string>& parts) {
    if (!getClientByFd(fd).GetReg())
    {
        std::stringstream message;
        message << 451 << "ERR_NOTREGISTERED :client not register";
        //sent(fd, message.str().c_str(), message.str().size(), 0);
        ToSent(fd, message.str());
        return;
    }
    std::string nickname = parts[1];
    std::string channel = parts[2];
    //channel = clearBuff(channel, 0); // a faire attention au ^M 
    Client sender = getClientByFd(fd);
    bool joined = false;

    std::cout << "***" << nickname << "***" << std::endl;
    std::cout << "***" << channel << "***" << std::endl;
    if (parts.size() < 3) {
        std::cerr << "ERR_NEEDMOREPARAMS" << std::endl;
        ToSend(fd, "ERR_NEEDMOREPARAMS");
    }
/*
    if (chanelles.size() != 0) {
        for (size_t i = 0; i < chanelles.size(); i++) 
        {            
            std::cout << "Channel: "<< i << " " << chanelles[i].get_name() << std::endl;
            if (chanelles[i].get_name() == channel )
                break ;
            else { 
                std::cout << "No matching channels" << std::endl;
                return ;
            }
        }
    }
*/
    if (clients.size() != 0) {
        for (size_t i = 0; i < clients.size(); i++) 
        {            
            std::cout << "Client: "<< i << " " << clients[i].GetNickname() << std::endl;
            if (clients[i].GetNickname() == nickname ) {
                if (nickname == sender.GetNickname() /*|| current_client.GetNickname() ==*/ ) {
                    std::cerr << "ERR_USERONCHANNEL" << std::endl;
                    ToSend(fd, "ERR_USERONCHANNEL");
                } else { 
                    std::cout << "The sender: " << sender.GetNickname() << " invited " << clients[i].GetNickname() 
                        << " is joining the channel: " << channel << std::endl;
                    Join(i + 4, parts); // a voir si la bricole du +4 tient???
                    joined = true;
                    break ;
                }
            }
        }
    } else {
        std::cerr << "no nick found" << std::endl;
        ToSend(fd, "ERR_NOSUCHNICK");
    }
}

/*
4.2.7 Invite message

      Command: INVITE
   Parameters: <nickname> <channel>

   The INVITE message is used to invite users to a channel.  The
   parameter <nickname> is the nickname of the person to be invited to
   the target channel <channel>.  There is no requirement that the
   channel the target user is being invited to must exist or be a valid
   channel.  To invite a user to a channel which is invite only (MODE
   +i), the client sending the invite must be recognised as being a
   channel operator on the given channel.

   Numeric Replies:
           ERR_NEEDMOREPARAMS              ERR_NOSUCHNICK
           ERR_NOTONCHANNEL                ERR_USERONCHANNEL
           ERR_CHANOPRIVSNEEDED
           RPL_INVITING                    RPL_AWAY

   Examples:
   :Angel INVITE Wiz #Dust         ; User Angel inviting WiZ to channel
                                   #Dust

   INVITE Wiz #Twilight_Zone       ; Command to invite WiZ to
                                   #Twilight_zone
*/

void Server::Topic(int fd, const std::vector<std::string>& parts) {
    (void)fd;
    (void)parts;
    
}
/*
4.2.4 Topic message

      Command: TOPIC
   Parameters: <channel> [<topic>]

   The TOPIC message is used to change or view the topic of a channel.
   The topic for channel <channel> is returned if there is no <topic>
   given.  If the <topic> parameter is present, the topic for that
   channel will be changed, if the channel modes permit this action.

   Numeric Replies:
           ERR_NEEDMOREPARAMS              ERR_NOTONCHANNEL
           RPL_NOTOPIC                     RPL_TOPIC
           ERR_CHANOPRIVSNEEDED


   Examples:
   :Wiz TOPIC #test :New topic     ;User Wiz setting the topic.
   
   TOPIC #test :another topic      ;set the topic on #test to "another
                                   topic".

   TOPIC #test                     ; check the topic for #test.
*/


void Server::Mode(int fd, const std::vector<std::string>& parts) {
    (void)fd;
    (void)parts;
    
}

/*
4.2.3 Mode message

      Command: MODE

   The MODE command is a dual-purpose command in IRC.  It allows both
   usernames and channels to have their mode changed.  The rationale for
   this choice is that one day nicknames will be obsolete and the
   equivalent property will be the channel.

   When parsing MODE messages, it is recommended that the entire message
   be parsed first and then the changes which resulted then passed on.

4.2.3.1 Channel modes

   Parameters: <channel> {[+|-]|o|p|s|i|t|n|b|v} [<limit>] [<user>]
               [<ban mask>]

   The MODE command is provided so that channel operators may change the
   characteristics of `their' channel.  It is also required that servers
   be able to change channel modes so that channel operators may be
   created.

   The various modes available for channels are as follows:

           i - invite-only channel flag;
           t - topic settable by channel operator only flag;
           k - set a channel key (password).
           o - give/take channel operator privileges;
           l - set the user limit to channel;
        _______________________________________________________________

                        p - private channel flag;
                        s - secret channel flag;
                        n - no messages to channel from clients on the outside;
                        m - moderated channel;
                        b - set a ban mask to keep users out;
                        v - give/take the ability to speak on a moderated channel;

   When using the 'o' and 'b' options, a restriction on a total of three
   per mode command has been imposed.  That is, any combination of 'o'
   and
*/
