/*
 * IrcModule.h
 *
 *  Created on: 01.01.2011
 *      Author: fkrauthan
 */

#ifndef IRCMODULE_H_
#define IRCMODULE_H_

#include "../module.h"
#include "IrcModuleConnection.h"
#include <string>


class IrcModule {
	public:
		IrcModule(const std::string id, ModuleManager* moduleManager, Irc* irc);
		~IrcModule();


		IrcModuleConnection connect(const std::string& id, const std::string& server, const std::string& nick, int port = 6667);
		void disconect(const std::string& id);
		void disconectAll();

		IrcModuleConnection getConnection(const std::string& id);

	private:
		std::string mID;
		Irc* mIRC;
		ModuleManager* mModuleManager;
};

#endif /* IRCMODULE_H_ */
