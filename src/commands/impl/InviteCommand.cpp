#include "commands/Command.hpp"
#include "utils.hpp"

InviteCommand::InviteCommand(Server *server) : Command(server) {}

InviteCommand::~InviteCommand() {}

void InviteCommand::execute(Client *client, std::vector<std::string> arguments)
{
    // check target client
    Server *server = getServer();
    std::string name = arguments[0];
    Client *target = server->getClient(name);

    if (!target)
    {
        std::string logMessage = "Target : " + name + " client not found.";
        ft_log(logMessage.c_str());
        return ;
    }
    // check current channel
    Channel *channel = client->getChannel();
    if (!channel)
    {
        std::string logMessage = "You are not in a channel.";
        ft_log(logMessage.c_str());
        return ;
    }
    // check admin
    Client *admin = channel->getAdmin();
    std::vector<Client *> _admins = channel->getAdminlist();
    if (admin != client)
    {
        if (std::find(_admins.begin(), _admins.end(), client) == _admins.end())
        {
            std::string logMessage = "You are not channel admin.";
            ft_log(logMessage.c_str());
            return ;
        }
    }
    // check invite mode
    if (!channel->getInvitemode())
    {
        std::string logMessage = "Channel is not invite mode.";
        ft_log(logMessage.c_str());
        return ;
    }

    target->reply(RPL_INVITING(server->getServername(), client->getNickname(), channel->getName()));

    channel->addInvitelist(target);

    std::string logMessage = target->getNickname() + " has been invited to channel " + channel->getName() + ".";
    ft_log(logMessage.c_str());
}