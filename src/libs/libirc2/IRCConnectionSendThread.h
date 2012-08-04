/*
 * IRCConnectionSendThread.h
 *
 *  Created on: 23.07.2011
 *      Author: fkrauthan
 */

#ifndef IRCCONNECTIONSENDTHREAD_H_
#define IRCCONNECTIONSENDTHREAD_H_

#include <string>
#include <queue>

#include <boost/thread.hpp>


class IRCConnectionSendThread {
	private:
		static const unsigned int SLEEP_TIME_MS = 500;

	public:
		IRCConnectionSendThread(int socket);
		~IRCConnectionSendThread();

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

#endif /* IRCCONNECTIONSENDTHREAD_H_ */
