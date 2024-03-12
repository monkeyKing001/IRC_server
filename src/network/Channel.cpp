#include "network/Channel.hpp"
#include "network/Client.hpp"

Channel::Channel(const std::string &name, const std::string &password, Client *admin, int _serverEpollFd)
		: _name(name), _admin(admin), _k(password), _serverEpollFd(_serverEpollFd), _l(0), _n(false), _t(true), _i(false) {} //dokwak, sungjuki

Channel::~Channel() {}

std::vector<std::string> Channel::getNicknames()
{
	std::vector<std::string> nicknames;

	for (clients_iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		Client *client = it.operator*();
		nicknames.push_back((_admin == client ? "@" : "") + (*it)->getNickname());
	}

	return nicknames;
}

void Channel::broadcast(const std::string &message)
{
	for (clients_iterator it = _clients.begin(); it != _clients.end(); it++){
		(*it)->write(message);
	}
}

void Channel::broadcast(const std::string &message, Client *exclude)
{
	for (clients_iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (*it == exclude) continue;
		(*it)->write(message);
	}
}

void Channel::removeClient(Client *client)
{
	_clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
	client->setChannel(NULL);

	if (_clients.empty())
	{
		//TODO: free Channel* and remove from _channels in Server
		return;
	}

	if (_admin == client)
	{
		if (_admins.size() != 0)
		{
			_admin = _admins.begin().operator*();
		}
		else
		{
			_admin = _clients.begin().operator*();
		}	
		
		std::string message;
		message.append(_admin->getNickname());
		message.append(" is now admin of channel ");
		message.append(_name);
		message.append(".");
		ft_log(message);
	}
}

void Channel::kick(Client *client, Client *target, const std::string &reason)
{
	broadcast(RPL_KICK(client->getPrefix(), _name, target->getNickname(), reason));
	removeClient(target);

	std::string message;
	message.append(client->getNickname());
	message.append(" kicked ");
	message.append(target->getNickname());
	message.append(" from channel ");
	message.append(_name);
	message.append(".");
	ft_log(message);
}

void Channel::removeAdminlist(Client *client)
{
    for (std::vector<Client*>::iterator it = _admins.begin(); it != _admins.end(); ++it)
    {
        if (*it == client)
        {
            _admins.erase(it);
            break;
		}
	}
}
