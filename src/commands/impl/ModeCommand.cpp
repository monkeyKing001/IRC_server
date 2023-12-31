#include "commands/Command.hpp"

ModeCommand::ModeCommand(Server *server) : Command(server) {}

ModeCommand::~ModeCommand() {}

void ModeCommand::execute(Client *client, std::vector<std::string> arguments) {

	if (arguments.size() < 2)
	{
		client->reply(ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
		return;
	}

	std::string target = arguments.at(0);

	Channel *channel = _server->getChannel(target); //MODE on clients not implemented
	if (!channel) {
		client->reply(ERR_NOSUCHCHANNEL(client->getNickname(), target));
		return;
	}

	//sungjuki
	if (channel->getAdmin() != client) 
	{
		std::vector<Client *> tmp = channel->getAdminlist();
		if (std::find(tmp.begin(), tmp.end(), client) == tmp.end())
			{
				client->reply(ERR_CHANOPRIVSNEEDED(client->getNickname(), target));
				return;
			}
	}

	int i = 0;
	int p = 2;
	char c;

	while ((c = arguments[1][i])) {

		char prevC = i > 0 ? arguments[1][i - 1] : '\0';
		bool active = prevC == '+';

		switch (c)
		{
			case 'i': //sungjuki
			{
				channel->broadcast(RPL_MODE(client->getPrefix(), channel->getName(), (active ? "+i" : "-i"), ""));
				if (active)
				{
					channel->setInvitemode(true);
				}
				else
				{
					channel->setInvitemode(false);
				}
				break;
			}
			case 'n':
			{
				channel->setNoExt(active);
				channel->broadcast(RPL_MODE(client->getPrefix(), channel->getName(), (active ? "+n" : "-n"), ""));
				break;
			}
			case 'l':
			{
				channel->setMaxClients(active ? atoi(arguments[p].c_str()) : 0);
				std::stringstream tmp;
				tmp << atoi(arguments[p].c_str());
				std::string str = tmp.str();
				channel->broadcast(RPL_MODE(client->getPrefix(), channel->getName(), (active ? "+l" : "-l"), (active ? str : "")));
				p += active ? 1 : 0;
				break;
			}
			case 'k':
			{
				channel->setPassword(active ? arguments[p] : "");
				channel->broadcast(RPL_MODE(client->getPrefix(), channel->getName(), (active ? "+k" : "-k"), (active ? arguments[p] : "")));
				p += active ? 1 : 0;
				break;
			}
			case 'o':	//sungjuki
			{	
				if (channel->getAdmin() != client) break; //찐방장 아닐 경우, mode +/-o 안됨.
				if (client->getNickname() == arguments[p]) break; //자기 자신 안됨.

				if (active)
				{
					Server *tmp_sv;
					Client *tmp_cl;

					tmp_sv = getServer();
					tmp_cl = tmp_sv->getClient(arguments[p]);
					if (!tmp_cl)
					{
						client->reply(ERR_NOSUCHNICK(client->getNickname(), arguments[p]));
						break;
					}
					else
					{
						if (tmp_cl->getChannel() != channel)
						{
							client->reply(ERR_NOSUCHNICK(client->getNickname(), arguments[p]));
							break;
						}
					}
				}
				else
				{
					Server *tmp_sv;
					Client *tmp_cl;

					tmp_sv = getServer();
					tmp_cl = tmp_sv->getClient(arguments[p]);
					std::vector<Client *> tmp = channel->getAdminlist();
					if (std::find(tmp.begin(), tmp.end(), tmp_cl) == tmp.end())
					{
						client->reply(ERR_NOSUCHNICK(client->getNickname(), arguments[p]));
						break;
					}
				}

				channel->broadcast(RPL_MODE(client->getPrefix(), channel->getName(), (active ? "+o" : "-o"), (active ? arguments[p] : arguments[p])));
				if (active)
				{
					Server	*tmp_sv;
					Client	*add_admin;

					tmp_sv = getServer();
					add_admin = tmp_sv->getClient(arguments[p]);
					channel->addAdminlist(add_admin);
					break;
				}
				else
				{	
					Server *server = getServer();
					Client *clientToRemove = server->getClient(arguments[p]);
					channel->removeAdminlist(clientToRemove);
					break;
				}

			}
			case 't':	//sungjuki
			{
				channel->broadcast(RPL_MODE(client->getPrefix(), channel->getName(), (active ? "+t" : "-t"), ""));
				if (active)
				{
					channel->setTmode(true);
				}
				else
				{
					channel->setTmode(false);
				}
				break;
			}
			default:
				break;
		}	

		i++;
	}
}
