/*
 * main.cpp
 *
 *  Created on: 09.06.2010
 *      Author: fkrauthan
 */

#include "Bot/bot.h"

#include <boost/program_options.hpp>

#include <iostream>
#include <string>


int startBot(const std::string& configFile);
void startDaemon();


int main(int argc, char **argv) {
	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("config", boost::program_options::value<std::string>()->default_value("ircbot.ini"), "sets the bot config file")
	;


	std::string configFile = "";


	//Process programm options
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	if(vm.count("help")) {
		std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
		std::cout << desc << std::endl;
	}
	if(vm.count("config")) {
		configFile = vm["config"].as<std::string>();
	}


	//Run the bot
	return startBot(configFile);



	/*bool runDaemon = false;
	if(argc>1) {
		if(std::strcmp(argv[1], "--daemon") == 0) {
			runDaemon = true;
		}
	}

	if(runDaemon) {
		startDaemon();
	}
	else {
		return startBot();
	}*/


	return -1;
}

int startBot(const std::string& configFile) {
	std::cout << "IrcBot3 v0.1\n" << std::endl;

	Irc::ClientInfo clientInfo;
	clientInfo.clientName = "IrcBot3";
	clientInfo.version = "0.1";

	Bot bot(clientInfo);
	if(!bot.init(configFile)) {
		return -1;
	}
	bot.run();

	return 0;
}

void startDaemon() {
	/*umask(0);

	pid_t pid;
	if((pid = fork()) < 0) {
		//Cannot fork!
	    throw std::runtime_error(strerror(errno));
	} else if (pid != 0) { //parent
		exit(0);
	}

	pid_t sid = setsid();
	if (sid < 0) {
		exit(-1);
	}*/

	/* Close out the standard file descriptors */
	//close(STDIN_FILENO);
	//close(STDOUT_FILENO);
	//close(STDERR_FILENO);


	//Run the bot
	//exit(startBot());
}
