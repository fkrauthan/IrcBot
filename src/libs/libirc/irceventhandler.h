/*
 * irceventhandler.h
 *
 *  Created on: 30.12.2010
 *      Author: fkrauthan
 */

#ifndef IRCEVENTHANDLER_H_
#define IRCEVENTHANDLER_H_

#include "ircconnection.h"

#include <string>


class IrcEventHandler {
	public:
		virtual ~IrcEventHandler();

		virtual bool onConnect(IrcConnection& connection);
		virtual bool onMessage(IrcConnection& connection, IrcMessage& message);
		virtual bool onJoin(IrcConnection& connection, IrcMessage& message);
		virtual bool onPart(IrcConnection& connection, IrcMessage& message);

		virtual bool onKick(IrcConnection& connection, IrcMessage& message, const std::string& nick);
		virtual bool onQuit(IrcConnection& connection, IrcMessage& message);
		virtual bool onKill(IrcConnection& connection, IrcMessage& message, const std::string& nick);
		virtual bool onPrivateMessage(IrcConnection& connection, IrcMessage& message);
		virtual bool onChannelMessage(IrcConnection& connection, IrcMessage& message);
		virtual bool onNotice(IrcConnection& connection, IrcMessage& message);
		virtual bool onInvite(IrcConnection& connection, IrcMessage& message);
		virtual bool onTopicChanged(IrcConnection& connection, IrcMessage& message);
		virtual bool onNickChanged(IrcConnection& connection, IrcMessage& message);
};

class IrcBaseHandler {
	public:
		virtual ~IrcBaseHandler();

		virtual bool onServerConnect(IrcConnection& connection);
		virtual bool onServerDisconnect(IrcConnection& connection);
};

#endif /* IRCEVENTHANDLER_H_ */
