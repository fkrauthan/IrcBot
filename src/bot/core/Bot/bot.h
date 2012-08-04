/*
 * bot.h
 *
 *  Created on: 21.04.2010
 *      Author: fkrauthan
 */

#ifndef BOT_H_
#define BOT_H_

#include "../ModuleManager/modulemanager.h"
#include <libirc/irc.h>
#include <libirc/irceventhandler.h>

#include <string>
#include <vector>
#include <map>


class Bot : public IrcEventHandler {
	public:
		Bot(Irc::ClientInfo& clientInfo);
		~Bot();

		bool init(const std::string& configfile);
		void run();

	private:
		ModuleManager moduleManager;
		Irc irc;
		std::map<std::string, std::vector<std::string> > mInitModes;
		std::map<std::string, std::vector<std::string> > mInitChannels;

	public:
		bool onConnect(IrcConnection& connection);
		bool onMessage(IrcConnection& connection, IrcMessage& message);
};

#endif /* BOT_H_ */
