## The apps Buildfile for the bot
project(IRCBOT_BOT)


#Build bot core
option(IRCBOT_BOT_CORE "Build the core bot" TRUE)
if(IRCBOT_BOT_CORE)
	message(STATUS			"Build bot core: true")
	add_subdirectory(core)
else(IRCBOT_BOT_CORE)
	message(STATUS			"Build bot core: true")
endif(IRCBOT_BOT_CORE)



#Build modules
option(IRCBOT_BOT_MODULES "Build bot modules" TRUE)
if(IRCBOT_BOT_MODULES)
	message(STATUS			"Build bot modules: true")
	add_subdirectory(modules)
else(IRCBOT_BOT_MODULES)
	message(STATUS			"Build bot modules: true")
endif(IRCBOT_BOT_MODULES)
