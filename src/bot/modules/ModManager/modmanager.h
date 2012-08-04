/*
 * modmanager.h
 *
 *  Created on: 02.04.2011
 *      Author: fkrauthan
 */

#ifndef MODMANAGER_H_
#define MODMANAGER_H_

#include <libbotmodule/cppmodule.h>
#include <vector>
#include <string>


class ModManager : public CPPModule {
	public:
		ModManager(Irc* irc, ModuleManager* moduleManager, const std::string& id);
		virtual ~ModManager();

	protected:
		bool onInit(const std::map<std::string, std::string>& params);

		//void printHelp(IrcConnection* connection, IrcMessageForModule* message);
		bool onPrivateMessage(IrcModuleConnection& connection, IrcMessage& message);
};

#endif /* MODMANAGER_H_ */
