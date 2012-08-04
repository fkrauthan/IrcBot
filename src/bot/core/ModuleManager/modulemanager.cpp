/*
 * modulemanager.cpp
 *
 *  Created on: 10.06.2010
 *      Author: fkrauthan
 */

#include "modulemanager.h"
#include <libini/INI.h>
#include <libirc/irc.h>
#include <iostream>
#include <libbase/StringUtils/StringUtils.h>

#ifdef WIN32
#else
	#include <dlfcn.h>
#endif


ModuleManager::ModuleManager() : mIRC(NULL) {
}

ModuleManager::~ModuleManager() {
	unloadModules();
}

bool ModuleManager::init(libINI::INI& config, Irc* irc) {
	mIRC = irc;

	try {
		if(!config.issetSection("modules")) {
			return true;
		}

		std::map<std::string, std::string>& serverMap = config.getSection("modules");
		std::map<std::string, std::string>::iterator iter;
		for(iter=serverMap.begin(); iter!=serverMap.end(); ++iter) {
			size_t delemiterPos = iter->second.find(':');
			std::string type = iter->second.substr(0, delemiterPos);
			std::string file = iter->second.substr(delemiterPos+1);
			std::cout << "--> Start to load module \"" << file << "\" with id \"" << iter->first << "\"..." << std::endl;


			//Load params
			std::map<std::string, std::string> paramsMap;
			if(config.issetSection(iter->first)) {
				std::map<std::string, std::string>& configMap = config.getSection(iter->first);
				std::map<std::string, std::string>::iterator iter2;
				for(iter2=configMap.begin(); iter2!=configMap.end(); ++iter2) {
					paramsMap[iter2->first] = iter2->second;
				}
			}


			bool moduleLoaded = false;
			if(type=="binary") {
				moduleLoaded = loadModuleBinary(iter->first, file, paramsMap);
			}
			/*else if(type=="script") {

			}*/
			else {
				std::cout << "---> Warn: The type \"" << type << "\" is at the moment not supported" << std::endl;
				continue;
			}


			if(moduleLoaded) {
				std::cout << "--> Start to load module \"" << file << "\" with id \"" << iter->first << "\"...finish" << std::endl;
			}
			else {
				std::cout << "--> Start to load module \"" << file << "\" with id \"" << iter->first << "\"...fail" << std::endl;
			}
		}
	} catch(libINI::INIException& ex) {
		std::cout << "--> Error: Error by parsing config file for modules: " << ex.what() << std::endl;
		return false;
	}

	return true;
}

std::string ModuleManager::callMethod(const std::string& sender, const std::string& reciver, const std::string& action, const std::string& jsonString, IrcConnection* connection) {
	if(reciver=="IRCCONNECTION") {
		JSONNode n;
		if(!jsonString.empty()) {
			n = libjson::parse(jsonString);
		}

		if(action == "joinChan") {
			connection->joinChan(n["channel"].as_string());
		}
		else if(action == "partChan") {
			connection->partChan(n["channel"].as_string());
		}
		else if(action == "sendQuit") {
			connection->sendQuit(n["msg"].as_string());
		}
		else if(action == "sendMessage") {
			connection->sendMessage(n["target"].as_string(), n["message"].as_string());
		}
		else if(action == "sendNotice") {
			connection->sendNotice(n["target"].as_string(), n["message"].as_string());
		}
		else if(action == "sendAction") {
			connection->sendNotice(n["target"].as_string(), n["message"].as_string());
		}
		else if(action == "sendCTCP") {
			connection->sendNotice(n["target"].as_string(), n["message"].as_string());
		}
		else if(action == "changeNick") {
			connection->changeNick(n["nick"].as_string());
		}
		else if(action == "setUserMode") {
			connection->setUserMode(n["mode"].as_string());
		}
		else if(action == "setMode") {
			connection->setMode(n["nick"].as_string(), n["mode"].as_string());
		}
		else if(action == "getChannels") {
			JSONNode n(JSON_NODE);

			std::map<std::string, IrcChannel>& channelList = connection->getChannels();
			std::map<std::string, IrcChannel>::iterator iter;
			for(iter=channelList.begin(); iter!=channelList.end(); ++iter) {
				JSONNode n2(JSON_ARRAY);
				n2.set_name(iter->first);
				n2.push_back(convertIrcChannelToJSONString(iter->second));
				n.push_back(n2);
			}
			return n.write();
		}
		else if(action == "getServer") {
			return connection->getServer();
		}
		else if(action == "getPort") {
			return Base::StringUtils::toString(connection->getPort());
		}
		else if(action == "getNick") {
			return connection->getNick();
		}
		else if(action == "getID") {
			return connection->getID();
		}
	}
	else if(reciver=="IRC") {
		JSONNode n;
		if(!jsonString.empty()) {
			n = libjson::parse(jsonString);
		}

		if(action == "disconect") {
			mIRC->disconect(n["id"].as_string());
		}
		else if(action == "disconectAll") {
			mIRC->disconectAll();
		}
	}
	else if(reciver=="MODULEMANAGER") {
		JSONNode n;
		if(!jsonString.empty()) {
			n = libjson::parse(jsonString);
		}

		if(action == "reloadModule") {
			return Base::StringUtils::toString(reloadModule(n["id"].as_string()));
		}
		else if(action == "loadModuleBinary" || action == "loadModuleScript") {
			std::map<std::string, std::string> tmpParams;

			JSONNode n2 = n["params"];
			JSONNode::iterator i = n2.begin();
			for(; i!=n2.end(); ++i) {
				tmpParams[(*i).name()] = (*i).as_string();
			}

			if(action == "loadModuleBinary") {
				return Base::StringUtils::toString(loadModuleBinary(n["id"].as_string(), n["file"].as_string(), tmpParams));
			}
			else {
				return Base::StringUtils::toString(loadModuleScript(n["id"].as_string(), n["file"].as_string(), tmpParams));
			}
		}
		else if(action == "unloadModule") {
			unloadModule(n["id"].as_string());
		}
	}
	else {
		Module* module = getModule(reciver);
		if(!module) {
			return "";
		}

		if(module->type == Module::BINARY) {
			if(!module->binaryModule.onInternalMessage) {
				return "";
			}

			return module->binaryModule.onInternalMessage(sender.c_str(), action.c_str(), jsonString.c_str(), connection);
		}
	}

	return "";
}

void* ModuleManager::callMethodWithPtrReturn(const std::string& sender, const std::string& reciver, const std::string& action, const std::string& jsonString, IrcConnection* connection) {
	if(reciver=="IRC") {
		JSONNode n;
		if(!jsonString.empty()) {
			n = libjson::parse(jsonString);
		}

		if(action == "getConnection") {
			return mIRC->getConnection(n["id"].as_string());
		}
		else if(action == "connect") {
			return mIRC->connect(n["id"].as_string(), n["server"].as_string(), n["nick"].as_string(), n["port"].as_int());
		}
	}
	else {
		Module* module = getModule(reciver);
		if(!module) {
			return NULL;
		}

		if(module->type == Module::BINARY) {
			if(!module->binaryModule.onInternalMessageWithPtr) {
				return NULL;
			}

			return module->binaryModule.onInternalMessageWithPtr(sender.c_str(), action.c_str(), jsonString.c_str(), connection);
		}
	}

	return NULL;
}

bool ModuleManager::loadModuleBinary(const std::string& id, const std::string& file, const std::map<std::string,std::string>& params) {
#ifdef WIN32
	std::string pluginFile = "./modules/"+file;
#else
	std::string pluginFile = "./modules/lib"+file;
#endif

#ifdef DEBUG
	pluginFile += "_d";
#endif

#ifdef WIN32
	pluginFile += ".dll";
#else
	pluginFile += ".so";
#endif


	void* pluginHandle;
#ifdef WIN32
#else
	pluginHandle = dlopen(pluginFile.c_str(), RTLD_NOW);
	if(!pluginHandle) {
		std::cout << "Error: Cannot load " << pluginFile << ": " << dlerror() << std::endl;
		return false;
	}
#endif


	Module* module = new Module();
	module->ID = id;
	module->file = file;
	module->params = params;
	module->type = Module::BINARY;

	module->binaryModule.init = (module_init_f)loadFunctionFromBinary(pluginHandle, "module_init");
	module->binaryModule.remove = (module_remove_f)loadFunctionFromBinary(pluginHandle, "module_remove");
	module->binaryModule.onEvent = (module_onEvent_f)loadFunctionFromBinary(pluginHandle, "module_onEvent");
	module->binaryModule.onInternalMessage = (module_onInternalMessage_f)loadFunctionFromBinary(pluginHandle, "module_onInternalMessage");
	module->binaryModule.onInternalMessageWithPtr = (module_onInternalMessageWithPtr_f)loadFunctionFromBinary(pluginHandle, "module_onInternalMessageWithPtr");


	if(!module->binaryModule.onEvent) {
			std::cout << "Error: Cannot load " << pluginFile << ": the onEvent function was not found" << std::endl;
		delete module;
#ifdef WIN32
#else
		dlclose(pluginHandle);
#endif
		return false;
	}
	if(!module->binaryModule.init) {
		std::cout << "Error: Cannot load " << pluginFile << ": the init function was not found" << std::endl;
		delete module;
#ifdef WIN32
#else
		dlclose(pluginHandle);
#endif
		return false;
	}
	if(!module->binaryModule.remove) {
		std::cout << "Error: Cannot load " << pluginFile << ": the remove function was not found" << std::endl;
		delete module;
#ifdef WIN32
#else
		dlclose(pluginHandle);
#endif
		return false;
	}
	module->binaryModule.moduleHandle = pluginHandle;


	//Init module
	JSONNode n(JSON_NODE);
	std::map<std::string,std::string>::const_iterator iter;
	for(iter=params.begin(); iter!=params.end(); ++iter) {
		n.push_back(JSONNode(iter->first, iter->second));
	}
	std::string jsonString = n.write();
	if(!module->binaryModule.init(mIRC, this, module->ID.c_str(), jsonString.c_str())) {
		std::cout << "Error: Modules says there was an init problem" << std::endl;
		delete module;
		#ifdef WIN32
		#else
			dlclose(pluginHandle);
		#endif
		return false;
	}
	mModules.push_back(module);

	return true;
}

bool ModuleManager::loadModuleScript(const std::string& id, const std::string& file, const std::map<std::string,std::string>& params) {
	return false;
}

bool ModuleManager::reloadModule(const std::string& id) {
	Module* module = getModule(id);

	Module::Type type = module->type;
	std::string file = module->file;
	std::map<std::string, std::string> params = module->params;

	unloadModule(module);
	if(type==Module::BINARY) {
		return loadModuleBinary(id, file, params);
	}
	else if(type==Module::SCRIPT) {
		return loadModuleScript(id, file, params);
	}
}

void ModuleManager::unloadModule(const std::string& id) {
	unloadModule(getModule(id));
}

void ModuleManager::unloadModules() {
	std::vector<Module*>::iterator iter;
	for(iter=mModules.begin(); iter!=mModules.end(); ++iter) {
		unloadModule(*iter, false);
		delete *iter;
	}
	mModules.clear();
}

void ModuleManager::unloadModule(Module* module, bool unregister) {
	if(module->type==Module::BINARY) {
		unloadBinaryModule(module);
	}
	else if(module->type==Module::SCRIPT) {
		unloadScriptModule(module);
	}
	else {
		std::cout << "--> Error: The module with id \"" << module->ID << "\" has a unkown module type" << std::endl;
		return;
	}

	if(unregister) {
		std::vector<Module*>::iterator iter;
		for(iter=mModules.begin(); iter!=mModules.end(); ++iter) {
			if((*iter) == module) {
				delete module;
				mModules.erase(iter);
				return;
			}
		}
	}
}

void ModuleManager::unloadBinaryModule(Module* module) {
	//Call remove first
	module->binaryModule.remove();

	//Close binary
#ifdef WIN32
#else
	dlclose(module->binaryModule.moduleHandle);
#endif

	//Set handle to NULL
	module->binaryModule.moduleHandle = NULL;

	//Set all functions to NULL
	module->binaryModule.init = NULL;
	module->binaryModule.remove = NULL;

	module->binaryModule.onEvent = NULL;

	module->binaryModule.onInternalMessage = NULL;
	module->binaryModule.onInternalMessageWithPtr = NULL;
}

void ModuleManager::unloadScriptModule(Module* module) {
}

ModuleManager::Module* ModuleManager::getModule(const std::string& id) {
	for(int i=0; i<mModules.size(); i++) {
		if(mModules[i]->ID == id) {
			return mModules[i];
		}
	}

	return NULL;
}

bool ModuleManager::onServerConnect(IrcConnection& connection) {
	connection.registerEventHandler(this);
	return true;
}

void* ModuleManager::loadFunctionFromBinary(void* moduleHandle, const std::string& functionName) {
	void* ret = NULL;
#ifdef WIN32
#else
	ret = dlsym(moduleHandle, functionName.c_str());
#endif

	if(ret) {
		std::cout << "---> Callback function \"" << functionName << "\" was found" << std::endl;
	}
	return ret;
}


bool ModuleManager::callModules(IrcConnection* connection, const std::string& senderID, const std::string& eventName, const std::string& json, const std::string& extraLine) {
	bool goOn = true;

	for(int i=0; i<mModules.size(); i++) {
		if(mModules[i]->type == Module::BINARY) {
			goOn = callBinaryModule(mModules[i], connection, senderID, eventName, json, extraLine);
		}

		if(!goOn) {
			break;
		}
	}

	return goOn;
}

JSONNode ModuleManager::convertIrcMessageToJSONString(IrcMessage& message) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("ircLine", message.ircLine));
	n.push_back(JSONNode("prefix", message.prefix));
	n.push_back(JSONNode("hasDetailedPrefix", message.hasDetailedPrefix));

	JSONNode n2(JSON_NODE);
	n2.set_name("msgPrefix");
	n2.push_back(JSONNode("nick_or_server", message.msgPrefix.nick_or_server));
	n2.push_back(JSONNode("user", message.msgPrefix.user));
	n2.push_back(JSONNode("host", message.msgPrefix.host));
	n.push_back(n2);

	n.push_back(JSONNode("command", message.command));
	n.push_back(JSONNode("isNumeric", message.isNumeric));
	n.push_back(JSONNode("target", message.target));
	n.push_back(JSONNode("params", message.params));

	return n;
}

JSONNode ModuleManager::convertIrcChannelToJSONString(IrcChannel& channel) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("name", channel.name));
	n.push_back(JSONNode("topic", channel.topic));

	n.push_back(JSONNode("nameListFull", channel.nameListFull));

	JSONNode n2(JSON_ARRAY);
	n2.set_name("members");
	std::vector<IrcChannelMember>::iterator iter;
	for(iter=channel.members.begin(); iter!=channel.members.end(); ++iter) {
		JSONNode n3(JSON_NODE);
		n3.push_back(JSONNode("nick", (*iter).nick));

		JSONNode n4(JSON_ARRAY);
		n4.set_name("modes");
		for(std::uint64_t i=1; i!=std::uint64_t(1) << 52; i<<=1) {
			bool current_bit = ((*iter).modes & i) != 0;
			n4.push_back(JSONNode("flag", current_bit));
		}
		n3.push_back(n4);

		n2.push_back(n3);
	}
	n.push_back(n2);

	return n;
}

bool ModuleManager::callBinaryModule(Module* module, IrcConnection* connection, const std::string& senderID, const std::string& eventName, const std::string& json, const std::string& extraLine) {
	return module->binaryModule.onEvent(connection, senderID.c_str(), eventName.c_str(), json.c_str(), extraLine.c_str());
}


bool ModuleManager::onConnect(IrcConnection& connection) {
	return callModules(&connection, "IRCCONNECTION", "onConnect", "", "");
}

bool ModuleManager::onMessage(IrcConnection& connection, IrcMessage& message) {
	return callModules(&connection, "IRCCONNECTION", "onMessage", convertIrcMessageToJSONString(message).write(), "");
}

bool ModuleManager::onJoin(IrcConnection& connection, IrcMessage& message) {
	return callModules(&connection, "IRCCONNECTION", "onJoin", convertIrcMessageToJSONString(message).write(), "");
}

bool ModuleManager::onPart(IrcConnection& connection, IrcMessage& message) {
	return callModules(&connection, "IRCCONNECTION", "onPart", convertIrcMessageToJSONString(message).write(), "");
}

bool ModuleManager::onKick(IrcConnection& connection, IrcMessage& message, const std::string& nick) {
	return callModules(&connection, "IRCCONNECTION", "onKick", convertIrcMessageToJSONString(message).write(), nick);
}

bool ModuleManager::onQuit(IrcConnection& connection, IrcMessage& message) {
	return callModules(&connection, "IRCCONNECTION", "onQuit", convertIrcMessageToJSONString(message).write(), "");
}

bool ModuleManager::onKill(IrcConnection& connection, IrcMessage& message, const std::string& nick) {
	return callModules(&connection, "IRCCONNECTION", "onKill", convertIrcMessageToJSONString(message).write(), nick);
}

bool ModuleManager::onPrivateMessage(IrcConnection& connection, IrcMessage& message) {
	return callModules(&connection, "IRCCONNECTION", "onPrivateMessage", convertIrcMessageToJSONString(message).write(), "");
}

bool ModuleManager::onChannelMessage(IrcConnection& connection, IrcMessage& message) {
	return callModules(&connection, "IRCCONNECTION", "onChannelMessage", convertIrcMessageToJSONString(message).write(), "");
}

bool ModuleManager::onNotice(IrcConnection& connection, IrcMessage& message) {
	return callModules(&connection, "IRCCONNECTION", "onNotice", convertIrcMessageToJSONString(message).write(), "");
}

bool ModuleManager::onInvite(IrcConnection& connection, IrcMessage& message) {
	return callModules(&connection, "IRCCONNECTION", "onInvite", convertIrcMessageToJSONString(message).write(), "");
}

bool ModuleManager::onTopicChanged(IrcConnection& connection, IrcMessage& message) {
	return callModules(&connection, "IRCCONNECTION", "onTopicChanged", convertIrcMessageToJSONString(message).write(), "");
}

bool ModuleManager::onNickChanged(IrcConnection& connection, IrcMessage& message) {
	return callModules(&connection, "IRCCONNECTION", "onNickChanged", convertIrcMessageToJSONString(message).write(), "");
}
