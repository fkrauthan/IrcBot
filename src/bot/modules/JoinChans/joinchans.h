/*
 * joinchans.h
 *
 *  Created on: 31.12.2010
 *      Author: fkrauthan
 */

#ifndef JOINCHANS_H_
#define JOINCHANS_H_

#include <libbotmodule/cppmodule.h>
#include <vector>
#include <string>


class JoinChans : public CPPModule {
	private:
		struct JoinedChannel {
			std::string fromChannel;
			std::string fromChannelServerID;

			std::string toChannel;
			std::string toChannelServerID;
		};

	public:
		JoinChans(Irc* irc, ModuleManager* moduleManager, const std::string& id);
		virtual ~JoinChans();

	protected:
		bool onInit(const std::map<std::string, std::string>& params);

		//void printHelp(IrcConnection* connection, IrcMessageForModule* message);
		bool onChannelMessage(IrcModuleConnection& connection, IrcMessage& message);

	private:
		std::vector<JoinedChannel> mTranslateMap;
};

#endif /* JOINCHANS_H_ */
