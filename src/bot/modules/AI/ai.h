/*
 * ai.h
 *
 *  Created on: 13.07.2011
 *      Author: fkrauthan
 */

#ifndef AI_H_
#define AI_H_

#include "AILanugageAnalyser.h"

#include <libbotmodule/cppmodule.h>

#include <ctime>
#include <map>
#include <string>


class AI : public CPPModule {
	public:
		AI(Irc* irc, ModuleManager* moduleManager, const std::string& id);
		virtual ~AI();

	protected:
		bool onInit(const std::map<std::string, std::string>& params);

		bool onJoin(IrcModuleConnection& connection, IrcMessage& message);
		bool onPart(IrcModuleConnection& connection, IrcMessage& message);
		bool onKick(IrcModuleConnection& connection, IrcMessage& message, const std::string& nick);

	private:
		void cleanUpReJoinList();
		int isReJoin(const std::string& nick);

	private:
		//Config vars
		int mMinReJoinTime;
		int mMaxReJoinTime;
		std::string mLanguageFile;

		//Vars
		AILanugageAnalyser mAnalyser;

		//App vars
		std::map<std::string, std::time_t> mReJoinList;
};

#endif /* AI_H_ */
