/*
 * IRCConnection.h
 *
 *  Created on: 23.07.2011
 *      Author: fkrauthan
 */

#ifndef IRCCONNECTION_H_
#define IRCCONNECTION_H_

#include "IRCExceptions.h"
#include "IRCMessage.h"
#include "IRCChannel.h"

#include <string>
#include <queue>
#include <map>

#include <boost/thread.hpp>
#include <sigc++/sigc++.h>

class IRC;
class IRCConnectionSendThread;


class IRCConnection {
	private:
		static const size_t BUFFER_SIZE = 256;
		static const unsigned int MAX_CHANNELS_FOR_NAMES_REQUEST = 20;
		static const unsigned int WAIT_BETWEEN_NAMES_REQUEST = 120;
		typedef void (IRCConnection::*CommandMemberFunctionPtr)(void);

	public:
		IRCConnection(IRC& irc, const std::string& id, const std::string& server, const std::string& nick, unsigned int port = 6667, bool foreceIPv6=false);
		~IRCConnection();

		void run();
		void quitThread();


		void joinChan(const std::string& channel);
		void partChan(const std::string& channel);
		void sendQuit(const std::string& msg = std::string("Good bye!"));

		void sendMessage(const std::string& target, const std::string& message);
		void sendNotice(const std::string& target, const std::string& message);
		void sendCTCPAction(const std::string& target, const std::string& message);
		void sendCTCPAnswer(const std::string& target, const std::string& message);
		void sendCTCPRequest(const std::string& target, const std::string& message);

		void changeNick(const std::string& nick);
		void setMode(const std::string& target, const std::string& mode);

		void setChannelUserMode(const std::string& channel, const std::string& nick, IRCUserModeEnum::Enumeration mode);
		void removeChannelUserMode(const std::string& channel, const std::string& nick, IRCUserModeEnum::Enumeration mode);


		IRC& getIRC();
		const std::string& getID();

		const std::string& getServer();
		const std::string& getNick();
		unsigned int getPort();

		IRCChannel* getChannel(const std::string& channel);
		std::map<std::string, IRCChannel>& getChannels();

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
		void connectToServer(bool foreceIPv6);
		void startConversationWithServer();
		void disconnect();

		bool parseMessage(const std::string& message);

		std::string readLine();
		void sendLine(const std::string& message);

		bool isNumeric(const std::string& string);
		IRCUserModeEnum::Enumeration getIRCUserMode(char prefix, bool sign=false);
		char getIRCUserModeChar(IRCUserModeEnum::Enumeration mode);

	private:
		bool handleCTCP();

		void commandPART();
		void commandKICK();
		void commandJOIN();
		void commandQUIT();
		void commandKILL();
		void commandPRIVMSG();
		void commandTOPIC();
		void commandNICK();
		void commandINVITE();
		void commandNOTICE();
		void commandMODE();

		void numeric005();
		void numeric332();
		void numeric353();
		void numeric366();

	private:
		IRC& mIRC;

		std::string mID;
		std::string mServer;
		std::string mNick;
		unsigned int mPort;


		bool mQuit;


		int mSocket;
		std::queue<std::string> mLines;
		IRCConnectionSendThread* mSendThreadInstance;
		boost::thread* mSendThread;

		IRCMessage mCurrentMessage;
		bool mConnected;

		std::map<std::string, CommandMemberFunctionPtr> mCommandHandler;
		std::map<std::string, IRCChannel> mChannels;


		bool mMultiPrefixEnabled;
		std::time_t mLastNAMESend;
		bool mNeedNewNAMERequest;


		std::map<IRCUserModeEnum::Enumeration, char> mEnumToUserModeTranslate;
		std::map<char, IRCUserModeEnum::Enumeration> mUserModeTranslate;
		std::map<char, IRCUserModeEnum::Enumeration> mUserModeTranslateSigns;
		std::map<char, char> mChanModes;
};

#endif /* IRCCONNECTION_H_ */
