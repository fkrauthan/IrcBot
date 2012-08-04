/*
 * irceventhandler.cpp
 *
 *  Created on: 30.12.2010
 *      Author: fkrauthan
 */

#include "irceventhandler.h"


IrcEventHandler::~IrcEventHandler() {
}

bool IrcEventHandler::onConnect(IrcConnection& connection) {
	return true;
}

bool IrcEventHandler::onMessage(IrcConnection& connection, IrcMessage& message) {
	return true;
}

bool IrcEventHandler::onJoin(IrcConnection& connection, IrcMessage& message) {
	return true;
}

bool IrcEventHandler::onPart(IrcConnection& connection, IrcMessage& message) {
	return true;
}

bool IrcEventHandler::onKick(IrcConnection& connection, IrcMessage& message, const std::string& nick) {
	return true;
}

bool IrcEventHandler::onQuit(IrcConnection& connection, IrcMessage& message) {
	return true;
}

bool IrcEventHandler::onKill(IrcConnection& connection, IrcMessage& message, const std::string& nick) {
	return true;
}

bool IrcEventHandler::onPrivateMessage(IrcConnection& connection, IrcMessage& message) {
	return true;
}

bool IrcEventHandler::onChannelMessage(IrcConnection& connection, IrcMessage& message) {
	return true;
}

bool IrcEventHandler::onNotice(IrcConnection& connection, IrcMessage& message) {
	return true;
}

bool IrcEventHandler::onInvite(IrcConnection& connection, IrcMessage& message) {
	return true;
}

bool IrcEventHandler::onTopicChanged(IrcConnection& connection, IrcMessage& message) {
	return true;
}

bool IrcEventHandler::onNickChanged(IrcConnection& connection, IrcMessage& message) {
	return true;
}


IrcBaseHandler::~IrcBaseHandler() {
}

bool IrcBaseHandler::onServerConnect(IrcConnection& connection) {
	return true;
}

bool IrcBaseHandler::onServerDisconnect(IrcConnection& connection) {
	return true;
}
