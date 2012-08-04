/*
 * cppmodule.h
 *
 *  Created on: 30.12.2010
 *      Author: fkrauthan
 */

#ifndef CPPMODULE_H_
#define CPPMODULE_H_

#include <string>
#include <map>
#include "module.h"
#include <libirc/ircstructs.h>
#include "cpp/IrcModuleConnection.h"
#include "cpp/IrcModule.h"
#include "cpp/IrcModuleManager.h"


class CPPModule {
	public:
		CPPModule(Irc* irc, ModuleManager* moduleManager, const std::string& id);
		virtual ~CPPModule();

		bool init(const std::string& jsonParams);
		bool onEvent(IrcConnection* connection, const std::string& senderID, const std::string& eventName, const std::string& json, const std::string& optString);

		std::string onInternalMessage_p(const std::string& senderID, const std::string& action, const std::string& jsonParams, IrcConnection* connection);
		void* onInternalMessageWithPtr_p(const std::string& senderID, const std::string& action, const std::string& jsonParams, IrcConnection* connection);

	protected:
		std::string mID;
		IrcModule mIRC;
		IrcModuleManager mIrcModuleManager;
		ModuleManager* mModuleManager;


	protected:
		virtual bool onInit(const std::map<std::string, std::string>& params);

		virtual std::string onInternalMessage(const std::string& senderID, const std::string& action, const std::string& jsonParams, IrcModuleConnection& connection);
		virtual void* onInternalMessageWithPtr(const std::string& senderID, const std::string& action, const std::string& jsonParams, IrcModuleConnection& connection);

		virtual bool onConnect(IrcModuleConnection& connection);
		virtual bool onMessage(IrcModuleConnection& connection, IrcMessage& message);
		virtual bool onJoin(IrcModuleConnection& connection, IrcMessage& message);
		virtual bool onPart(IrcModuleConnection& connection, IrcMessage& message);

		virtual bool onKick(IrcModuleConnection& connection, IrcMessage& message, const std::string& nick);
		virtual bool onQuit(IrcModuleConnection& connection, IrcMessage& message);
		virtual bool onKill(IrcModuleConnection& connection, IrcMessage& message, const std::string& nick);
		virtual bool onPrivateMessage(IrcModuleConnection& connection, IrcMessage& message);
		virtual bool onChannelMessage(IrcModuleConnection& connection, IrcMessage& message);
		virtual bool onNotice(IrcModuleConnection& connection, IrcMessage& message);
		virtual bool onInvite(IrcModuleConnection& connection, IrcMessage& message);
		virtual bool onTopicChanged(IrcModuleConnection& connection, IrcMessage& message);
		virtual bool onNickChanged(IrcModuleConnection& connection, IrcMessage& message);
};

#endif /* CPPMODULE_H_ */
