/*
 * ircconnection.h
 *
 *  Created on: 09.06.2010
 *      Author: fkrauthan
 */

#ifndef IRCCONNECTION_H_
#define IRCCONNECTION_H_

#include <string>
#include <vector>
#include <queue>
#include <boost/thread.hpp>
#include "ircstructs.h"


class Irc;
class IrcEventHandler;


class IrcConnection {
	private:
		class IrcConnectionSendThread {
			private:
				static const int SLEEP_TIME_MS = 500;

			public:
				IrcConnectionSendThread(int socket);
				~IrcConnectionSendThread();

				void run();
				void quitThread();

				void addMessage(const std::string& message);

			private:
				void sendLine(const char* const buf, const  int size);

			private:
				bool mQuit;

				int mSocket;
				std::queue<std::string> mMessageQueue;
				boost::condition_variable mCV;
				boost::mutex mMutex;
		};

	private:
		static const size_t BUFFER_SIZE = 256;

	public:
		IrcConnection(Irc* irc, const std::string& id, const std::string& server, const std::string& nick, int port = 6667);
		~IrcConnection();

		void run();
		void quitThread();

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

		std::map<std::string, IrcChannel>& getChannels();

		std::string& getServer();
		int getPort();
		std::string& getNick();
		std::string& getID();

		Irc* getIrcManager();

		void registerEventHandler(IrcEventHandler* handler);
		void unregisterEventHandler(IrcEventHandler* handler);

	private:
		bool parseMessage(const std::string& message);
		void sendLine(const std::string& message);
		std::string readLine();

		void handleCTCPMessage();
		void parseModeMessage();

		IrcRightEnum::Enumeration getIrcRight(char prefix, bool sign=false);

		bool isNumeric(const std::string& string);

	private:
		std::vector<IrcEventHandler*> mEventHandler;

		IrcConnectionSendThread* mConnectionThreadClass;
		boost::thread* mConnectionThread;

		Irc* mIrc;

		int mSocket;
		std::string mServer;
		std::string mNick;
		int mPort;
		std::string mID;

		bool mConnected;
		bool mQuit;
		IrcMessage mCurrentMessage;

		std::map<std::string, IrcChannel> mChannels;
		std::queue<std::string> mReadLines;

		std::map<char, IrcRightEnum::Enumeration> mRightTranslate;
		std::map<char, IrcRightEnum::Enumeration> mRightTranslateSigns;

		std::map<char, char> mChanModes;
};

class IrcConnectionException : public std::runtime_error {
	public:
		IrcConnectionException(const std::string& message) : std::runtime_error(message) {}
};

#endif /* IRCCONNECTION_H_ */
