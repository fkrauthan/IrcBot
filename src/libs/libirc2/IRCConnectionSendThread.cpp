/*
 * IRCConnectionSendThread.cpp
 *
 *  Created on: 23.07.2011
 *      Author: fkrauthan
 */

#include "IRCConnectionSendThread.h"

#ifndef WIN32
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <errno.h>
#else
    #include <winsock2.h>
#endif


IRCConnectionSendThread::IRCConnectionSendThread(int socket)
 : mSocket(socket) {

}

IRCConnectionSendThread::~IRCConnectionSendThread() {

}

void IRCConnectionSendThread::run() {
	mQuit = false;

	while(!mQuit) {
		boost::mutex::scoped_lock lock(mMutex);
		while(mMessageQueue.empty()) {
			mCV.wait(lock);
		}

		while(!mMessageQueue.empty()) {
			std::string line = mMessageQueue.front();
			sendLine(line.c_str(), line.size());
			mMessageQueue.pop();

			//Sleep a while to prevent flooding
			usleep(SLEEP_TIME_MS*1000);
		}
	}
}

void IRCConnectionSendThread::quitThread() {
	boost::mutex::scoped_lock lock(mMutex);
	mQuit = true;

	lock.unlock();
	if(mMessageQueue.empty()) {
		mCV.notify_one();
	}
}

void IRCConnectionSendThread::addMessage(const std::string& message) {
	boost::mutex::scoped_lock lock(mMutex);
	bool const wasEmpty=mMessageQueue.empty();
	mMessageQueue.push(message);

	lock.unlock();
	if(wasEmpty) {
		mCV.notify_one();
	}
}

void IRCConnectionSendThread::sendLine(const char* const buf, const  int size) {
	int bytesSent = 0;
	do {
		int result = send(mSocket, buf + bytesSent, size - bytesSent, 0);
		if(result < 0) {
			break;
		}
		bytesSent += result;
	} while(bytesSent < size);
}
