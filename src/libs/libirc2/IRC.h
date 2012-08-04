/*
 * IRC.h
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
#include <sigc++/sigc++.h>

#include "IRCMessage.h"
#include "IRCChannel.h"
#include "IRCExceptions.h"

class IRCConnection;


class IRC {
	public:
		IRC();
		~IRC();

		IRCConnection* connect(const std::string& id, const std::string& server, const std::string& nick, unsigned int port = 6667, bool foreceIPv6=false);
		void disconect(const std::string& id);
		void disconect(IRCConnection* connection);
		void disconectAll();


		void setClientName(const std::string& name);
		const std::string& getClientName();
		void setClientVersion(const std::string& version);
		const std::string& getClientVersion();
		void setClientOS(const std::string& os);
		const std::string& getClientOS();


		IRCConnection* getConnection(const std::string& id);
		std::map<std::string, IRCConnection*>& getConnections();

		void run();

	public:
		//Signals IRC
		sigc::signal<void, IRC&, IRCConnection&> onServerConnect;
		sigc::signal<void, IRC&, IRCConnection&> onServerDisconnect;

	public:
		//Signals IRCConnection
		sigc::signal<void, IRC&, IRCConnection&> onConnect;

		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&, const std::string&> onCTCPRequest;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&, const std::string&> onCTCPAnswer;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&, const std::string&> onCTCPAction;

		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onPart;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onKick;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onJoin;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onQuit;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onKill;

		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onMessage;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onPrivateMessage;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onChannelMessage;

		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onTopicChanged;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onNickChanged;

		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onInvite;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&> onNotice;

		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&, const std::string&, IRCUserModeEnum::Enumeration> onUserObtainMode;
		sigc::signal<void, IRC&, IRCConnection&, IRCMessage&, const std::string&, IRCUserModeEnum::Enumeration> onUserLoseMode;

	private:
		void disconnectIntern(IRCConnection* connection);

	private:
		std::string mClientName;
		std::string mClientVersion;
		std::string mClientOS;

		std::map<std::string, IRCConnection*> mConnections;
		std::map<std::string, boost::thread*> mConnectionThreads;

		std::vector<IRCConnection*> mConnectionDelList;

		boost::mutex mMutex;
		boost::mutex mMutexDelList;

		boost::condition_variable mCV;
};

#endif /* IRC_H_ */
