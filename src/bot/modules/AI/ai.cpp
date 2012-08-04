/*
 * ai.cpp
 *
 *  Created on: 13.07.2011
 *      Author: fkrauthan
 */

#include "ai.h"

#include <iostream>
AI::AI(Irc* irc, ModuleManager* moduleManager, const std::string& id)
 : CPPModule(irc, moduleManager, id),
   mMinReJoinTime(10),
   mMaxReJoinTime(1*60),
   mLanguageFile("ai.xml") {
}

AI::~AI() {
}

bool AI::onInit(const std::map<std::string, std::string>& params) {
	return mAnalyser.loadLanguageFile(mLanguageFile);
}

bool AI::onJoin(IrcModuleConnection& connection, IrcMessage& message) {
	if(message.getUsername() == connection.getNick()) {
		return true;
	}


	std::map<std::string, std::string> languageParams;
	languageParams["$nick$"] = message.getUsername();
	languageParams["$chan$"] = message.target;


	cleanUpReJoinList();
	int isReJoinRet = isReJoin(message.target+"#"+message.getUsername());
	if(isReJoinRet == 1) {
		connection.sendMessage(message.target, mAnalyser.getAnswer("join/wb", languageParams));
	}
	else if(isReJoinRet == 0) {
		connection.sendMessage(message.target, mAnalyser.getAnswer("join/hello", languageParams));
	}

	return true;
}

bool AI::onPart(IrcModuleConnection& connection, IrcMessage& message) {
	if(message.getUsername() == connection.getNick()) {
		return true;
	}

	mReJoinList[message.target+"#"+message.getUsername()] = std::time(NULL);

	return true;
}
//TODO: Dosen't work
bool AI::onKick(IrcModuleConnection& connection, IrcMessage& message, const std::string& nick) {
	std::cout << "onKick: " << nick << " bot nick " << connection.getNick() << std::endl;
	if(nick == connection.getNick()) {
		return true;
	}

	mReJoinList[message.target+"#"+nick] = std::time(NULL);

	return true;
}

void AI::cleanUpReJoinList() {
	std::time_t currentTime = std::time(NULL);

	std::map<std::string, std::time_t>::iterator iter;
	for(iter=mReJoinList.begin(); iter!=mReJoinList.end();) {
		if(iter->second+mMaxReJoinTime < currentTime) {
			mReJoinList.erase(iter++);
		}
		else {
			++iter;
		}
	}
}

int AI::isReJoin(const std::string& nick) {
	std::map<std::string, std::time_t>::iterator iter = mReJoinList.find(nick);
	if(iter!=mReJoinList.end()) {
		std::time_t currentTime = std::time(NULL);
		int ret = 1;
		if(iter->second+mMinReJoinTime > currentTime) {
			ret = 2;
		}
		mReJoinList.erase(iter);

		return ret;
	}

	return 0;
}
