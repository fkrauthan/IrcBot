/*
 * irc.cpp
 *
 *  Created on: 09.06.2010
 *      Author: fkrauthan
 */

#include "irc.h"
#include "ircconnection.h"
#include "irceventhandler.h"
#include <iostream>


Irc::Irc(ClientInfo& clientInfo)
 : mClientInfo(clientInfo) {
}

Irc::~Irc() {
	std::map<std::string, boost::thread*>::iterator iter;
	for(iter=connectionThreads.begin(); iter!=connectionThreads.end(); ++iter) {
		getConnection(iter->first)->quitThread();
		if(!iter->second->timed_join(boost::posix_time::seconds(8))) {
			iter->second->interrupt();
		}
	}

	std::map<std::string, IrcConnection*>::iterator iter2;
	for(iter2=connections.begin(); iter2!=connections.end(); ++iter2) {
		delete iter2->second;
	}
}

IrcConnection* Irc::connect(const std::string& id, const std::string& server, const std::string& nick, int port) {
	IrcConnection* tmpConnection = getConnection(id);
	if(tmpConnection != NULL) {
		return tmpConnection;
	}

	try {
		boost::mutex::scoped_lock lock(mMutex);

		tmpConnection = new IrcConnection(this, id, server, nick, port);
		connections[id] = tmpConnection;

		boost::thread* th = new boost::thread(boost::bind(&IrcConnection::run, tmpConnection));
		connectionThreads[id] = th;


		{
			boost::mutex::scoped_lock lock2(mMutexEventHandler);

			std::vector<IrcBaseHandler*>::iterator iter;
			for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
				if(!(*iter)->onServerConnect(*tmpConnection)) {
					break;
				}
			}
		}
	} catch(const IrcConnectionException& e) {
		delete tmpConnection;

		std::cout << "-> IrcConnectionException: " << e.what() << std::endl;
		return NULL;
	}

	return tmpConnection;
}

void Irc::disconect(const std::string& id) {
	disconect(getConnection(id));
}

void Irc::disconect(IrcConnection* connection) {
	boost::mutex::scoped_lock lock(mMutexDelList);
	bool wasEmpty = connectionDelList.size()<=0;

	connectionDelList.push_back(connection);

	lock.unlock();
	if(wasEmpty) {
		mCV.notify_one();
	}
}

void Irc::disconectAll() {
	boost::mutex::scoped_lock lock(mMutexDelList);
	bool wasEmpty = connectionDelList.size()<=0;

	std::map<std::string, IrcConnection*>::iterator iter;
	for(iter=connections.begin(); iter!=connections.end(); ++iter) {
		connectionDelList.push_back(iter->second);
	}

	lock.unlock();
	if(wasEmpty) {
		mCV.notify_one();
	}
}

IrcConnection* Irc::getConnection(const std::string& id) {
	std::map<std::string, IrcConnection*>::iterator iter = connections.find(id);
	if(iter != connections.end()) {
		return iter->second;
	}

	return NULL;
}

std::map<std::string,IrcConnection*>& Irc::getConnections() {
	return connections;
}

void Irc::run() {
	while(connections.size() > 0) {
		boost::mutex::scoped_lock lock(mMutexDelList);
		while(connectionDelList.size()<=0 && connections.size()>0) {
			mCV.wait(lock);
		}

		int s=connectionDelList.size();
		for(int i=0; i<s; i++) {
			disconnectIntern(connectionDelList[i]);
		}
		connectionDelList.clear();
	}
}

void Irc::registerBaseHandler(IrcBaseHandler* handler) {
	boost::mutex::scoped_lock lock(mMutexEventHandler);

	mEventHandler.push_back(handler);
}

void Irc::unregisterBaseHandler(IrcBaseHandler* handler) {
	boost::mutex::scoped_lock lock(mMutexEventHandler);

	std::vector<IrcBaseHandler*>::iterator iter;
	if((iter=std::find(mEventHandler.begin(), mEventHandler.end(), handler))!=mEventHandler.end()) {
		mEventHandler.erase(iter);
	}
}

Irc::ClientInfo& Irc::getClientInfo() {
	return mClientInfo;
}

void Irc::disconnectIntern(IrcConnection* connection) {
	if(connection == NULL) {
		return;
	}
	boost::mutex::scoped_lock lock(mMutex);


	{
		boost::mutex::scoped_lock lock2(mMutexEventHandler);

		std::vector<IrcBaseHandler*>::iterator iter;
		for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
			if(!(*iter)->onServerDisconnect(*connection)) {
				break;
			}
		}
	}


	connection->sendQuit();
	connection->quitThread();

	std::map<std::string, boost::thread*>::iterator iter = connectionThreads.find(connection->getID());
	if(iter != connectionThreads.end()) {
		delete iter->second;
		connectionThreads.erase(iter);
	}

	std::map<std::string, IrcConnection*>::iterator iter2 = connections.find(connection->getID());
	if(iter2 != connections.end()) {
		connections.erase(iter2);
	}

	delete connection;
}
