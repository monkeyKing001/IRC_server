#ifndef FT_IRC_CHANNEL_HPP
# define FT_IRC_CHANNEL_HPP

class Channel;

# include "Server.hpp"
# include "Client.hpp"
# include "utils.hpp"

class Channel
{

	typedef std::vector<Client *>::iterator clients_iterator;

private:
	std::string				_name;
	Client					*_admin;
	std::vector<Client *>	_admins;//sungjuki
	std::vector<Client *>	_clients;
	std::string				_topic;//dokwak
	std::vector<Client *>	_invitelist; //sungjuki

	/** Modes */
	std::string				_k;
	size_t					_l;
	bool					_n;
	bool					_t;//dokwak
	bool					_i;//sungjuki

public:
	Channel(const std::string &name, const std::string &password, Client *admin);
	~Channel();

	Client* getAdmin() { return _admin; };
	void	setAdmin(Client *new_admin) { _admin = new_admin; }; //sungjuki
	std::string getName() const { return _name; };

	std::string getPassword() const { return _k; };
	void		setPassword(std::string k) { this->_k = k; };
	size_t		getMaxClients() const { return _l; };
	void		setMaxClients(size_t l) { this->_l = l; };
	bool		isNoExt() const { return _n; };
	void		setNoExt(bool n) { this->_n = n; };
	std::string getTopic() const { return _topic; };//dokwak
	void		setTopic(std::string t) { this -> _topic = t; };//dokwak
	bool		isTmode() { return _t; };//dokwak
	void		setTmode(bool x) { _t = x; }; //sungjuki
	
	bool		getInvitemode() const { return _i; }; //sungjuki
	void		setInvitemode(bool n) {this->_i = n; }; //sungjuki
	std::vector<Client *>	getInvitedlist() { return this->_invitelist; }; //sungjuki
	std::vector<Client *>	getClientlist() { return this->_clients; }; //sungjuki
	void		addInvitelist(Client *client) { _invitelist.push_back(client); }; //sungjuki
	std::vector<Client *>	getAdminlist()	{ return this->_admins; };	//sungjuki
	void		addAdminlist(Client *client) { _admins.push_back(client); };	//sungjuki
	void		removeAdminlist(Client *client);	//sungjuki

	size_t						getNumClients() const { return _clients.size(); };
	std::vector<std::string>	getNicknames();

	void broadcast(std::string const &message);
	void broadcast(const std::string &message, Client *exclude);
	void removeClient(Client *client);
	void addClient(Client *client) { _clients.push_back(client); };
	void kick(Client *client, Client *target, const std::string &reason);

};

#endif
