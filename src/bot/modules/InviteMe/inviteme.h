/*
 * inviteme.h
 *
 *  Created on: 22.04.2010
 *      Author: fkrauthan
 */

#ifndef INVITEME_H_
#define INVITEME_H_

#include <libbotmodule/cppmodule.h>


class InviteMe : public CPPModule {
	public:
		InviteMe(Irc* irc, ModuleManager* moduleManager, const std::string& id);
		virtual ~InviteMe();

	protected:
		//void printHelp(IrcConnection* connection, IrcMessageForModule* message);
		bool onInvite(IrcModuleConnection& connection, IrcMessage& message);
};

#endif /* INVITEME_H_ */
