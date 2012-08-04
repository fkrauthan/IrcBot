/*
 * irc.h
 *
 *  Created on: 09.06.2010
 *      Author: fkrauthan
 */

#ifndef IRC_H_
#define IRC_H_

#include <string>
#include <map>
#include <vector>
#include <boost/thread.hpp>

class IrcConnection;
class IrcMessage;
class IrcEventHandler;
class IrcBaseHandler;


class Irc {
	public:
		struct ClientInfo {
			std::string version;
			std::string clientName;
		};

	public:
		Irc(ClientInfo& clientInfo);
		~Irc();

		IrcConnection* connect(const std::string& id, const std::string& server, const std::string& nick, int port = 6667);
		void disconect(const std::string& id);
		void disconect(IrcConnection* connection);
		void disconectAll();

		IrcConnection* getConnection(const std::string& id);
		std::map<std::string, IrcConnection*>& getConnections();

		void run();


		void registerBaseHandler(IrcBaseHandler* handler);
		void unregisterBaseHandler(IrcBaseHandler* handler);


		ClientInfo& getClientInfo();

	private:
		void disconnectIntern(IrcConnection* connection);

	private:
		//Client infos
		ClientInfo mClientInfo;

		//Irc vars
		std::map<std::string, IrcConnection*> connections;
		std::map<std::string, boost::thread*> connectionThreads;

		std::vector<IrcConnection*> connectionDelList;

		std::vector<IrcBaseHandler*> mEventHandler;

		boost::mutex mMutex;
		boost::mutex mMutexDelList;

		boost::mutex mMutexEventHandler;

		boost::condition_variable mCV;
};

#endif /* IRC_H_ */
