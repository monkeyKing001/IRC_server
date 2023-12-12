#include <iostream>
#include "network/Client.hpp"

Client::Client(int fd,const std::string &hostname, int port)
		: _fd(fd), _hostname(hostname), _port(port), _state(HANDSHAKE), _channel(NULL) {
}

Client::~Client() {}

std::string Client::getPrefix() const {
	return _nickname + (_username.empty() ? "" : "!" + _username) + (_hostname.empty() ? "" : "@" + _hostname);
}

void Client::write(const std::string &message) const
{
	//std::cout << "-> " << message << std::endl;

	std::string buffer = message + "\r\n";
	if (send(_fd, buffer.c_str(), buffer.length(), 0) < 0)
		throw std::runtime_error("Error while sending message to client.");
}

void Client::reply(const std::string &reply) {
	write(":" + getPrefix() + " " + reply);
}

void Client::welcome() {
	if (_state != LOGIN || _username.empty() || _realname.empty() || _nickname.empty())
		return;
	_state = PLAY;

	reply(RPL_WELCOME(_nickname));

	std::string message;
	ft_log(message);
	//char message[100];
	//ft_log(message);
}

void Client::join(Channel *channel)
{

	channel->addClient(this);
	_channel = channel;
	
	std::string users;
	std::vector<std::string> nicknames = channel->getNicknames();
	for (std::vector<std::string>::iterator it = nicknames.begin(); it != nicknames.end(); it++)
		users.append(*it + " ");

	reply(RPL_NAMREPLY(_nickname, channel->getName(), users));
	reply(RPL_ENDOFNAMES(_nickname, channel->getName()));

	channel->broadcast(RPL_JOIN(getPrefix(), channel->getName()));

	//char message[100];
	//sprintf(message, "%s has joined channel %s.", _nickname.c_str(), channel->getName().c_str());
	std::string message;
	message.append(_nickname);
	message.append(" has joined channel ");
	message.append(channel->getName());
	message.append(".");
	ft_log(message);
}

void Client::leave()
{

	if (!_channel) return;

	const std::string name = _channel->getName();
	std::vector<Client *> _admins = _channel->getAdminlist();
	std::vector<Client *> _clients = _channel->getClientlist();

	//admin일 경우 처리
	if (_channel->getAdmin() == this)
	{
		if (_admins.size() != 0)
		{
			_channel->setAdmin(_admins.begin().operator*());
			_channel->removeAdminlist(_admins.begin().operator*());
		}
		else
		{
			_channel->setAdmin(_clients.begin().operator*());
		}	
	}

	//admin list일 경우 처리
	if (std::find(_admins.begin(), _admins.end(), this) != _admins.end())
	{
		_channel->removeAdminlist(this);
	}

	_channel->broadcast(RPL_PART(getPrefix(), _channel->getName()));
	_channel->removeClient(this);

	//char message[100];
	//sprintf(message, "%s has left channel %s.", _nickname.c_str(), name.c_str());
	std::string message;
	message.append(_nickname);
	message.append(" has left channel ");
	message.append(name);
	message.append(".");
	ft_log(message);
}
