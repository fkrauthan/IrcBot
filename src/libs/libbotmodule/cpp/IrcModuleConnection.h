/*
 * IrcModuleConnection.h
 *
 *  Created on: 30.12.2010
 *      Author: fkrauthan
 */

#ifndef IRCMODULECONNECTION_H_
#define IRCMODULECONNECTION_H_

#include "../module.h"
#include <libirc/ircstructs.h>
#include <string>
#include <map>


class IrcModuleConnection {
	public:
		IrcModuleConnection(const std::string id, ModuleManager* moduleManager, IrcConnection* connection);
		~IrcModuleConnection();


		void joinChan(const std::string& channel);
		void partChan(const std::string& channel);
		void sendQuit(const std::string& msg = std::string("Good bye!"));

		void sendMessage(const std::string& target, const std::string& message);
		void sendNotice(const std::string& target, const std::string& message);
		void sendAction(const std::string& target, const std::string& message);
		void sendCTCP(const std::string& target, const std::string& message);

		void changeNick(const std::string& nick);
		void setUserMode(const std::string& mode);
		void setMode(const std::string& nick, const std::string& mode);

		std::map<std::string, IrcChannel> getChannels();

		std::string getServer();
		int getPort();
		std::string getNick();
		std::string getID();

		IrcConnection* getIrcConnection();

	private:
		std::string mID;
		IrcConnection* mConnection;
		ModuleManager* mModuleManager;
};

#endif /* IRCMODULECONNECTION_H_ */
