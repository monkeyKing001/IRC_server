#include "commands/Command.hpp"
#include "utils.hpp"

TopicCommand::TopicCommand(Server *server) : Command(server){}

TopicCommand::~TopicCommand(){}

void TopicCommand::execute(Client *client, std::vector<std::string> arguments)
{
	Channel *channel = client -> getChannel(); //MODE on clients not implemented
	if (!channel)
	{
		if (arguments.size() == 0)
		{
			ft_log("not enough parameters\n");
			client -> reply(ERR_NEEDMOREPARAMS(client -> getNickname(), "TOPIC"));
			return ;
		}
		ft_log("not in any chan\n");
		client -> reply(ERR_NOTONCHANNEL(client -> getNickname(), ""));
		return;
	}
	if (arguments[0][0] == '#')
	{
		Channel *target = _server -> getChannel(arguments[0]);
		if (!target)
		{
			client -> reply(ERR_NOSUCHCHANNEL(client -> getNickname(), channel -> getName()));
		ft_log("no such chan\n");
			return ;
		}
		if (client -> getChannel() != target)
		{
			client -> reply(ERR_NOTONCHANNEL(client -> getNickname(), target -> getName()));
			ft_log("not on that such chan\n");
			return;
		}
	}
	//Admin ERR
	//Admin check
	std::vector<Client *> _admins = channel->getAdminlist();
	if (channel -> isTmode() && channel->getAdmin() != client && std::find(_admins.begin(), _admins.end(), client) == _admins.end()) 
	{
		ft_log("no admin\n");
		client -> reply(ERR_CHANOPRIVSNEEDED(client -> getNickname(), channel -> getName()));
		return;
	}
//	if (arguments.size() == 1){//check topic
//		ft_log("topic check\n");
//		channel->broadcast(RPL_PRIVMSG(client->getPrefix(), arguments.at(0), channel -> getTopic()), client);
//	}
	//setTopic (arguments.size() == 0)
	else if (arguments.size() > 1){
		//arg[0] = <channel name>
		//arg[1] = <:{input}>;
		std::string temp = "";
		if (arguments[1][0] == ':')
			arguments[1] = arguments[1].substr(1, arguments[1].length());
		for (unsigned long i = 1; i < arguments.size(); i++) //sungjuki change int i = 1 --> unsigned long
		{
			temp.append(arguments[i]);
			temp.append(" ");
			ft_log(arguments[i]);
		}
		channel -> setTopic(temp);
		channel -> broadcast(RPL_TOPIC(client -> getPrefix(), channel -> getName(), temp));
	}
}
