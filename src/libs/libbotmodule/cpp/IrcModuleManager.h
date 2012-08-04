/*
 * IrcModuleManager.h
 *
 *  Created on: 02.04.2011
 *      Author: fkrauthan
 */

#ifndef IRCMODULEMANAGER_H_
#define IRCMODULEMANAGER_H_

#include "../module.h"
#include "IrcModuleConnection.h"
#include <string>


class IrcModuleManager {
	public:
		IrcModuleManager(const std::string id, ModuleManager* moduleManager);
		~IrcModuleManager();


		bool loadModuleBinary(const std::string& id, const std::string& file, const std::map<std::string,std::string>& params);
		bool loadModuleScript(const std::string& id, const std::string& file, const std::map<std::string,std::string>& params);

		bool reloadModule(const std::string& id);
		void unloadModule(const std::string& id);

		std::string callMethod(const std::string& reciver, const std::string& action, const std::string& jsonString, IrcModuleConnection& connection);
		void* callMethodWithPtrReturn(const std::string& reciver, const std::string& action, const std::string& jsonString, IrcModuleConnection& connection);

		std::string callMethod(const std::string& reciver, const std::string& action, const std::string& jsonString, IrcConnection* connection=NULL);
		void* callMethodWithPtrReturn(const std::string& reciver, const std::string& action, const std::string& jsonString, IrcConnection* connection=NULL);

	private:
		std::string mID;
		ModuleManager* mModuleManager;
};

#endif /* IRCMODULEMANAGER_H_ */
