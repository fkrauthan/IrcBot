/*
 * modulemanager.h
 *
 *  Created on: 10.06.2010
 *      Author: fkrauthan
 */

#ifndef MODULEMANAGER_H_
#define MODULEMANAGER_H_

#include <string>
#include <vector>
#include <map>
#include <libbotmodule/module.h>
#include <libirc/irceventhandler.h>
#include <libjson/libjson.h>

class Irc;
class IrcConnection;
namespace libINI {
	class INI;
}


class ModuleManager : public IrcEventHandler, public IrcBaseHandler {
	private:
		struct BinaryModule {
			void* moduleHandle;

			module_init_f init;
			module_remove_f remove;

			module_onEvent_f onEvent;

			module_onInternalMessage_f onInternalMessage;
			module_onInternalMessageWithPtr_f onInternalMessageWithPtr;
		};

		struct ScriptModule {
		};

		struct Module {
			enum Type {
				BINARY,
				SCRIPT
			};


			std::string ID;
			std::string file;
			std::map<std::string,std::string> params;

			Type type;
			BinaryModule binaryModule;
			ScriptModule scriptModule;
		};

	public:
		ModuleManager();
		virtual ~ModuleManager();

		bool init(libINI::INI& config, Irc* irc);

		bool loadModuleBinary(const std::string& id, const std::string& file, const std::map<std::string,std::string>& params);
		bool loadModuleScript(const std::string& id, const std::string& file, const std::map<std::string,std::string>& params);

		bool reloadModule(const std::string& id);
		void unloadModule(const std::string& id);


		std::string callMethod(const std::string& sender, const std::string& reciver, const std::string& action, const std::string& jsonString, IrcConnection* connection);
		void* callMethodWithPtrReturn(const std::string& sender, const std::string& reciver, const std::string& action, const std::string& jsonString, IrcConnection* connection);

	private:
		void* loadFunctionFromBinary(void* moduleHandle, const std::string& functionName);

		void unloadModules();
		void unloadModule(Module* module, bool unregister=true);
		void unloadBinaryModule(Module* module);
		void unloadScriptModule(Module* module);

		Module* getModule(const std::string& id);

	private:
		Irc* mIRC;
		std::vector<Module*> mModules;


	private:
		bool onServerConnect(IrcConnection& connection);


	public:
		bool callModules(IrcConnection* connection, const std::string& senderID, const std::string& eventName, const std::string& json, const std::string& extraLine);
		JSONNode convertIrcMessageToJSONString(IrcMessage& message);
		JSONNode convertIrcChannelToJSONString(IrcChannel& channel);

		bool callBinaryModule(Module* module, IrcConnection* connection, const std::string& senderID, const std::string& eventName, const std::string& json, const std::string& extraLine);

	public:
		bool onConnect(IrcConnection& connection);
		bool onMessage(IrcConnection& connection, IrcMessage& message);
		bool onJoin(IrcConnection& connection, IrcMessage& message);
		bool onPart(IrcConnection& connection, IrcMessage& message);

		bool onKick(IrcConnection& connection, IrcMessage& message, const std::string& nick);
		bool onQuit(IrcConnection& connection, IrcMessage& message);
		bool onKill(IrcConnection& connection, IrcMessage& message, const std::string& nick);
		bool onPrivateMessage(IrcConnection& connection, IrcMessage& message);
		bool onChannelMessage(IrcConnection& connection, IrcMessage& message);
		bool onNotice(IrcConnection& connection, IrcMessage& message);
		bool onInvite(IrcConnection& connection, IrcMessage& message);
		bool onTopicChanged(IrcConnection& connection, IrcMessage& message);
		bool onNickChanged(IrcConnection& connection, IrcMessage& message);
};

#endif /* MODULEMANAGER_H_ */
