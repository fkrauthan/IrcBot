## The apps Buildfile for the bot modules
project(IRCBOT_BOT_MODULES)


#Build the mod manager module
option(IRCBOT_BOT_MODULE_MODMANAGER "Build the mod manager module" TRUE)
if(IRCBOT_BOT_MODULE_MODMANAGER)
	message(STATUS			"Build mod manager module: true")
	add_subdirectory(ModManager)
else(IRCBOT_BOT_MODULE_MODMANAGER)
	message(STATUS			"Build mod manager module: false")
endif(IRCBOT_BOT_MODULE_MODMANAGER)

#Build the invite module
option(IRCBOT_BOT_MODULE_INVITEME "Build the invite me module" TRUE)
if(IRCBOT_BOT_MODULE_INVITEME)
	message(STATUS			"Build invite me module: true")
	add_subdirectory(InviteMe)
else(IRCBOT_BOT_MODULE_INVITEME)
	message(STATUS			"Build invite me module: false")
endif(IRCBOT_BOT_MODULE_INVITEME)

#Build the join chans module
option(IRCBOT_BOT_MODULE_JOINCHANS "Build the join chans module" TRUE)
if(IRCBOT_BOT_MODULE_JOINCHANS)
	message(STATUS			"Build join chans module: true")
	add_subdirectory(JoinChans)
else(IRCBOT_BOT_MODULE_JOINCHANS)
	message(STATUS			"Build join chans module: false")
endif(IRCBOT_BOT_MODULE_JOINCHANS)

#Build the ai module
option(IRCBOT_BOT_MODULE_AI "Build the ai module" TRUE)
if(IRCBOT_BOT_MODULE_AI)
	message(STATUS			"Build ai module: true")
	add_subdirectory(AI)
else(IRCBOT_BOT_MODULE_AI)
	message(STATUS			"Build ai module: false")
endif(IRCBOT_BOT_MODULE_AI)
