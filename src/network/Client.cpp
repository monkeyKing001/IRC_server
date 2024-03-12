#include <iostream>
#include "network/Client.hpp"

Client::Client(int fd,const std::string &hostname, int port, int _serverEpollFd)
		: _fd(fd), _hostname(hostname), _port(port), _serverEpollFd(_serverEpollFd), _state(HANDSHAKE), _channel(NULL) {
}

Client::~Client() {}

std::string Client::getPrefix() const {
	return _nickname + (_username.empty() ? "" : "!" + _username) + (_hostname.empty() ? "" : "@" + _hostname);
}

void Client::write(const std::string &message)
{
	_sendBuffer = message + "\r\n";
	setEpollEventState(EPOLL_READY_INNOUT);
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

	std::string message;
	message.append(_nickname);
	message.append(" has left channel ");
	message.append(name);
	message.append(".");
	ft_log(message);
}

void Client::sendBuffer() const{
	if (send(_fd, _sendBuffer.c_str(), _sendBuffer.length(), 0) < 0)
		throw std::runtime_error("Error while sending message to client.");
	setEpollEventState(EPOLL_READY_IN);
}

void Client::setEpollEventState(EpollEventsState es) const {
	struct epoll_event newEvent;
	newEvent.data.fd = _fd;
	newEvent.events = es;
	epoll_ctl(_serverEpollFd, EPOLL_CTL_MOD, _fd, &newEvent);
}

