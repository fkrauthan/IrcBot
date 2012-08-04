/*
 * IRC.cpp
 *
 *  Created on: 09.06.2010
 *      Author: fkrauthan
 */

#include "IRC.h"
#include "IRCConnection.h"
#include <iostream>


IRC::IRC()
: mClientName("libirc2"),
  mClientVersion("0.1") {

#ifdef WIN32
	mClientOS = "WIN";
#else
	mClientOS = "UNIX";
#endif
}

IRC::~IRC() {
	std::map<std::string, boost::thread*>::iterator iter;
	for(iter=mConnectionThreads.begin(); iter!=mConnectionThreads.end(); ++iter) {
		getConnection(iter->first)->quitThread();
		if(!iter->second->timed_join(boost::posix_time::seconds(8))) {
			iter->second->interrupt();
		}
	}

	std::map<std::string, IRCConnection*>::iterator iter2;
	for(iter2=mConnections.begin(); iter2!=mConnections.end(); ++iter2) {
		delete iter2->second;
	}
}

IRCConnection* IRC::connect(const std::string& id, const std::string& server, const std::string& nick, unsigned int port, bool foreceIPv6) {
	IRCConnection* tmpConnection = getConnection(id);
	if(tmpConnection != NULL) {
		return tmpConnection;
	}

	try {
		boost::mutex::scoped_lock lock(mMutex);

		tmpConnection = new IRCConnection(*this, id, server, nick, port, foreceIPv6);
		mConnections[id] = tmpConnection;

		boost::thread* th = new boost::thread(boost::bind(&IRCConnection::run, tmpConnection));
		mConnectionThreads[id] = th;


		onServerConnect(*this, *tmpConnection);
	} catch(const IRCConnectException& e) {
		delete tmpConnection;

		std::cout << "-> IRCConnectException: " << e.what() << std::endl;
		return NULL;
	}

	return tmpConnection;
}

void IRC::disconect(const std::string& id) {
	disconect(getConnection(id));
}

void IRC::disconect(IRCConnection* connection) {
	boost::mutex::scoped_lock lock(mMutexDelList);
	bool wasEmpty = mConnectionDelList.size()<=0;

	mConnectionDelList.push_back(connection);

	lock.unlock();
	if(wasEmpty) {
		mCV.notify_one();
	}
}

void IRC::disconectAll() {
	boost::mutex::scoped_lock lock(mMutexDelList);
	bool wasEmpty = mConnectionDelList.size()<=0;

	std::map<std::string, IRCConnection*>::iterator iter;
	for(iter=mConnections.begin(); iter!=mConnections.end(); ++iter) {
		mConnectionDelList.push_back(iter->second);
	}

	lock.unlock();
	if(wasEmpty) {
		mCV.notify_one();
	}
}

void IRC::setClientName(const std::string& name) {
	mClientName = name;
}

const std::string& IRC::getClientName() {
	return mClientName;
}

void IRC::setClientVersion(const std::string& version) {
	mClientVersion = version;
}

const std::string& IRC::getClientVersion() {
	return mClientVersion;
}

void IRC::setClientOS(const std::string& os) {
	mClientOS = os;
}

const std::string& IRC::getClientOS() {
	return mClientOS;
}

IRCConnection* IRC::getConnection(const std::string& id) {
	std::map<std::string, IRCConnection*>::iterator iter = mConnections.find(id);
	if(iter != mConnections.end()) {
		return iter->second;
	}

	return NULL;
}

std::map<std::string,IRCConnection*>& IRC::getConnections() {
	return mConnections;
}

void IRC::run() {
	while(mConnections.size() > 0) {
		boost::mutex::scoped_lock lock(mMutexDelList);
		while(mConnectionDelList.size()<=0 && mConnections.size()>0) {
			mCV.wait(lock);
		}

		int s=mConnectionDelList.size();
		for(int i=0; i<s; i++) {
			disconnectIntern(mConnectionDelList[i]);
		}
		mConnectionDelList.clear();
	}
}

void IRC::disconnectIntern(IRCConnection* connection) {
	if(connection == NULL) {
		return;
	}
	boost::mutex::scoped_lock lock(mMutex);


	onServerDisconnect(*this, *connection);


	connection->quitThread();

	std::map<std::string, boost::thread*>::iterator iter = mConnectionThreads.find(connection->getID());
	if(iter != mConnectionThreads.end()) {
		delete iter->second;
		mConnectionThreads.erase(iter);
	}

	std::map<std::string, IRCConnection*>::iterator iter2 = mConnections.find(connection->getID());
	if(iter2 != mConnections.end()) {
		mConnections.erase(iter2);
	}

	delete connection;
}
