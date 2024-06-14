#include "chanel.hpp"

chanel::chanel(std::string name_arg)
{
    name = name_arg;
}

chanel::~chanel()
{

}


std::string chanel::get_name()
{
     return (name);   
}

void chanel::addClient(Client& cli)
{
    ListCli.push_back(cli);
    
}
