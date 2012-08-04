/*
 * IRCConnection.cpp
 *
 *  Created on: 23.07.2011
 *      Author: fkrauthan
 */

#include "IRCConnection.h"
#include "IRCConnectionSendThread.h"
#include "IRC.h"

#include <boost/algorithm/string.hpp>
#include <cstring>
#include <iostream>

#ifndef WIN32
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <errno.h>
#else
    #include <winsock2.h>
#endif


IRCConnection::IRCConnection(IRC& irc, const std::string& id, const std::string& server, const std::string& nick, unsigned int port, bool foreceIPv6)
 : mIRC(irc),
   mID(id),
   mServer(server),
   mNick(nick),
   mPort(port),
   mQuit(false),
   mSocket(-1),
   mSendThreadInstance(NULL),
   mSendThread(NULL),
   mConnected(false),
   mMultiPrefixEnabled(false),
   mLastNAMESend(0),
   mNeedNewNAMERequest(false) {

	//Register commands
	mCommandHandler["PART"] = &IRCConnection::commandPART;
	mCommandHandler["KICK"] = &IRCConnection::commandKICK;
	mCommandHandler["JOIN"] = &IRCConnection::commandJOIN;
	mCommandHandler["QUIT"] = &IRCConnection::commandQUIT;
	mCommandHandler["KILL"] = &IRCConnection::commandKILL;
	mCommandHandler["PRIVMSG"] = &IRCConnection::commandPRIVMSG;
	mCommandHandler["TOPIC"] = &IRCConnection::commandTOPIC;
	mCommandHandler["NICK"] = &IRCConnection::commandNICK;
	mCommandHandler["INVITE"] = &IRCConnection::commandINVITE;
	mCommandHandler["NOTICE"] = &IRCConnection::commandNOTICE;
	mCommandHandler["MODE"] = &IRCConnection::commandMODE;

	//Register numerics
	mCommandHandler["005"] = &IRCConnection::numeric005;
	mCommandHandler["332"] = &IRCConnection::numeric332;
	mCommandHandler["353"] = &IRCConnection::numeric353;
	mCommandHandler["366"] = &IRCConnection::numeric366;

	//Connect to servers
	connectToServer(foreceIPv6);
	startConversationWithServer();
}

IRCConnection::~IRCConnection() {
	if(mSendThread) {
		mSendThreadInstance->quitThread();
		if(!mSendThread->timed_join(boost::posix_time::seconds(5))) {
			mSendThread->interrupt();
		}
		delete mSendThread;
	}
	delete mSendThreadInstance;

	disconnect();
}

void IRCConnection::run() {
	mQuit = false;

	while(!mQuit) {
		std::string line(readLine());
		if(line.empty()) {
			continue;
		}
		else if(line.substr(0, 4) == "PING") {
			sendLine("PONG "+line.substr(5)+"\r\n");
			continue;
		}
		std::cout << line << std::endl;

		if(!parseMessage(line)) {
			std::cout << "-> This line is invalid and can't be paresd" << std::endl;
			continue;
		}
		else if(!mConnected) {
			if(mCurrentMessage.isNumeric && mCurrentMessage.command == "001") {
				mConnected = true;

				onConnect(mIRC, *this);
			}
			else if(mCurrentMessage.command == "CAP") {
				if(mCurrentMessage.params[1] == "LS") {
					//Check if multi-prefix is supported
					bool multiPrefix = false;
					std::vector<std::string> tmpSupportedCAPModes;
					boost::split(tmpSupportedCAPModes, mCurrentMessage.params[2], boost::is_any_of(" "));
					for(unsigned int i=0; i<tmpSupportedCAPModes.size(); i++) {
						if(tmpSupportedCAPModes[i] == "multi-prefix") {
							multiPrefix = true;
							break;
						}
					}

					if(!multiPrefix) {
						sendLine("CAP END\r\n");
					}
					else {
						sendLine("CAP REQ :multi-prefix\r\n");
					}
				}
				else if(mCurrentMessage.params[1] == "NAK") {
					sendLine("CAP END\r\n");
					std::cout << "--> Server supports multi-prefix but it can't be activated by us" << std::endl;
				}
				else if(mCurrentMessage.params[1] == "ACK") {
					mMultiPrefixEnabled = true;

					sendLine("CAP END\r\n");
					std::cout << "--> Server supports multi-prefix and it's now activated by us" << std::endl;
				}
			}
		}
		else {
			if(!mMultiPrefixEnabled) {
				if(mChannels.size() <= MAX_CHANNELS_FOR_NAMES_REQUEST) {
					std::time_t curTime = std::time(NULL);
					if(curTime >= mLastNAMESend+WAIT_BETWEEN_NAMES_REQUEST && mNeedNewNAMERequest) {
						std::map<std::string, IRCChannel>::iterator iter;
						for(iter=mChannels.begin(); iter!=mChannels.end(); ++iter) {
							sendLine("NAMES "+iter->second.name+"\r\n");
						}
						mLastNAMESend = curTime;
						mNeedNewNAMERequest = false;
					}
				}
			}

			std::map<std::string, CommandMemberFunctionPtr>::iterator iter = mCommandHandler.find(mCurrentMessage.command);
			if(iter != mCommandHandler.end()) {
				(this->*(iter->second))();
			}
		}
	}
}

void IRCConnection::quitThread() {
	mQuit = true;
}

void IRCConnection::joinChan(const std::string& channel) {
	sendLine("JOIN "+channel+"\r\n");
}

void IRCConnection::partChan(const std::string& channel) {
	sendLine("PART "+channel+"\r\n");
}

void IRCConnection::sendQuit(const std::string& msg) {
	if(mConnected) {
		mConnected = false;
		sendLine("QUIT :"+msg+"\r\n");
	}
}

void IRCConnection::sendMessage(const std::string& target, const std::string& message) {
	sendLine("PRIVMSG "+target+" :"+message+"\r\n");
	std::cout << "TEST " << "PRIVMSG "+target+" :"+message << std::endl;
}

void IRCConnection::sendNotice(const std::string& target, const std::string& message) {
	sendLine("NOTICE "+target+" :"+message+"\r\n");
}

void IRCConnection::sendCTCPAction(const std::string& target, const std::string& message) {
	sendCTCPRequest(target, "ACTION "+message);
}

void IRCConnection::sendCTCPAnswer(const std::string& target, const std::string& message) {
	std::stringstream str;
	str << (char)0x01 << message << (char)0x01;
	sendNotice(target, str.str());
}

void IRCConnection::sendCTCPRequest(const std::string& target, const std::string& message) {
	std::stringstream str;
	str << (char)0x01 << message << (char)0x01;
	sendMessage(target, str.str());
}

void IRCConnection::changeNick(const std::string& nick) {
	sendLine("NICK "+nick+"\r\n");
}

void IRCConnection::setMode(const std::string& target, const std::string& mode) {
	sendLine("MODE "+target+" "+mode+"\r\n");
}

void IRCConnection::setChannelUserMode(const std::string& channel, const std::string& nick, IRCUserModeEnum::Enumeration mode) {
	if(mode <= IRCUserModeEnum::NOTHING) {
		return;
	}

	char code = getIRCUserModeChar(mode);
	if(code == '\0') {
		return;
	}

	std::stringstream str;
	str << "+" << code << " " << nick;
	setMode(channel, str.str());
}

void IRCConnection::removeChannelUserMode(const std::string& channel, const std::string& nick, IRCUserModeEnum::Enumeration mode) {
	if(mode <= IRCUserModeEnum::NOTHING) {
		return;
	}

	char code = getIRCUserModeChar(mode);
	if(code == '\0') {
		return;
	}

	std::stringstream str;
	str << "-" << code << " " << nick;
	setMode(channel, str.str());
}

IRC& IRCConnection::getIRC() {
	return mIRC;
}

const std::string& IRCConnection::getID() {
	return mID;
}

const std::string& IRCConnection::getServer() {
	return mServer;
}

const std::string& IRCConnection::getNick() {
	return mNick;
}

unsigned int IRCConnection::getPort() {
	return mPort;
}

IRCChannel* IRCConnection::getChannel(const std::string& channel) {
	std::map<std::string, IRCChannel>::iterator iter = mChannels.find(channel);
	if(iter != mChannels.end()) {
		return &iter->second;
	}

	return NULL;
}

std::map<std::string, IRCChannel>& IRCConnection::getChannels() {
	return mChannels;
}

void IRCConnection::connectToServer(bool foreceIPv6) {
#ifdef WIN32
	WSADATA w;
	if(int result = WSAStartup(MAKEWORD(2,2), &w) != 0) {
		std::cout << "--> Winsock 2 konnte nicht gestartet werden! Error #" << result << std::endl;
		return NULL;
    }
#endif


	struct addrinfo ai_hints, *ai_list, *ai;
	int rc;

	std::memset(&ai_hints, 0, sizeof(ai_hints));
	if(foreceIPv6) {
		ai_hints.ai_family = AF_INET6;
	}
	else {
		ai_hints.ai_family = AF_UNSPEC;
	}
	ai_hints.ai_socktype = SOCK_STREAM;
	ai_hints.ai_protocol = IPPROTO_TCP;


	rc = getaddrinfo(mServer.c_str(), NULL, &ai_hints, &ai_list);
	if(rc != 0) {
		std::cout << "--> Hostname konnte nicht aufgeloest werden: " << gai_strerror(rc) << std::endl;
		throw IRCConnectException("Can't resolve hostename");
	}

	bool found = false;
	for(ai=ai_list; ai!=NULL; ai=ai->ai_next) {
		mSocket = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if(mSocket < 0) {
			std::cout << "--> Socket konnte nicht erstellt werden!" << std::endl;
			throw IRCConnectException("Can't create socket connection");
		}


		if(ai->ai_family == AF_INET) {
			((sockaddr_in*)ai->ai_addr)->sin_port = htons(mPort);
		}
		else {
			((sockaddr_in6*)ai->ai_addr)->sin6_port = htons(mPort);
		}
		rc = connect(mSocket, ai->ai_addr, ai->ai_addrlen);
		if(rc < 0) {
			disconnect();
			continue;
		}
		else {
			found = true;
			break;
		}
	}

	if(!found) {
		std::cout << "--> Verbindung fehlgeschlagen!" << std::endl;
		throw IRCConnectException("Start connection failed");
	}

	//Create send Thread
	mSendThreadInstance = new IRCConnectionSendThread(mSocket);
	mSendThread = new boost::thread(boost::bind(&IRCConnectionSendThread::run, mSendThreadInstance));
}

void IRCConnection::startConversationWithServer() {
	sendLine("CAP LS\r\n");
	sendLine("NICK "+mNick+"\r\n");
	sendLine("USER "+mNick+" \"\" \"\" :"+mNick+"\r\n");
}

void IRCConnection::disconnect() {
	if(mSocket!=-1) {
#ifndef WIN32
		close(mSocket);
#else
		closesocket(mSocket);
#endif
	}
}

bool IRCConnection::parseMessage(const std::string& message) {
	mCurrentMessage.completeLine = message;

	mCurrentMessage.prefixString = "";
	mCurrentMessage.prefix.nickOrServer = "";
	mCurrentMessage.prefix.hasUserAndHost = false;
	mCurrentMessage.prefix.user = "";
	mCurrentMessage.prefix.host = "";
	mCurrentMessage.command = "";
	mCurrentMessage.isNumeric = false;
	mCurrentMessage.paramsString = "";
	mCurrentMessage.params.clear();
	mCurrentMessage.isCTCPMessage = false;


	if(message.substr(0, 1) != ":") {
		return false;
	}

	size_t pos = std::string::npos;
	size_t pos2 = std::string::npos;

	if((pos = message.find_first_of(' ')) == std::string::npos) {
		return false;
	}


	//Parse prefix
	mCurrentMessage.prefixString = message.substr(1, pos-1);
	if((pos2 = mCurrentMessage.prefixString.find('!')) != std::string::npos) {
		mCurrentMessage.prefix.hasUserAndHost = true;
		mCurrentMessage.prefix.nickOrServer = mCurrentMessage.prefixString.substr(0, pos2);

		size_t pos3 = std::string::npos;
		if((pos3 = mCurrentMessage.prefixString.find('@')) != std::string::npos) {
			mCurrentMessage.prefix.user = mCurrentMessage.prefixString.substr(pos2+1, pos3-pos2-1);
			mCurrentMessage.prefix.host = mCurrentMessage.prefixString.substr(pos3+1);
		}
	}
	else {
		mCurrentMessage.prefix.nickOrServer = mCurrentMessage.prefixString;
	}


	//Parse command
	if((pos2 = message.find_first_of(' ', pos+1)) == std::string::npos) return false;
	mCurrentMessage.command = message.substr(pos+1, pos2-pos-1);
	mCurrentMessage.isNumeric = isNumeric(mCurrentMessage.command);


	//Parse params
	if(message.size() > pos2+1) {
		mCurrentMessage.paramsString = message.substr(pos2+1);
		if(mCurrentMessage.paramsString[0] == ':') {
			mCurrentMessage.params.push_back(mCurrentMessage.paramsString.substr(1));
		}
		else {
			bool hasColon = false;
			size_t prevPos = 0;
			pos = 0;
			while((pos=mCurrentMessage.paramsString.find_first_of(' ', pos+1)) != std::string::npos) {
				std::string param = mCurrentMessage.paramsString.substr(prevPos, pos-prevPos);
				if(param[0] == ':') {
					mCurrentMessage.params.push_back(mCurrentMessage.paramsString.substr(prevPos+1));
					hasColon = true;
					break;
				}
				else {
					mCurrentMessage.params.push_back(param);
					prevPos = pos+1;
				}
			}

			if(!hasColon) {
				std::string param = mCurrentMessage.paramsString.substr(prevPos);
				if(param[0] == ':') {
					param = param.substr(1);
				}
				mCurrentMessage.params.push_back(param);
			}
		}
	}

	return true;
}

std::string IRCConnection::readLine() {
	if(mLines.empty()) {
		char readBuffer[BUFFER_SIZE];
		ssize_t readBytes = 0;
		std::string tmpLine = "";
		while((readBytes = recv(mSocket, &readBuffer, BUFFER_SIZE, 0)) > 0) {
			tmpLine.append(readBuffer, readBytes);
			if(readBuffer[readBytes-1] == '\n') {
				break;
			}
		}

		size_t pos;
		int size = 0;
		while((pos = tmpLine.find_first_of('\n')) != std::string::npos) {
			std::string tmpString(tmpLine.substr(0, pos));
			if(*(tmpString.end()-1) == '\r') {
				tmpString.erase(tmpString.end()-1);
			}
			mLines.push(tmpString);
			tmpLine = tmpLine.substr(pos+1);
		}
		if(*(tmpLine.end()-1) == '\r') {
			tmpLine.erase(tmpLine.end()-1);
		}
		mLines.push(tmpLine);
	}

	std::string ret = mLines.front();
	mLines.pop();
	return ret;
}

void IRCConnection::sendLine(const std::string& message) {
	mSendThreadInstance->addMessage(message);
}

bool IRCConnection::isNumeric(const std::string& string) {
	for(unsigned int i = 0; i < string.length(); i++) {
		if(!std::isdigit(string[i])) {
			return false;
		}
	}

	return true;
}

IRCUserModeEnum::Enumeration IRCConnection::getIRCUserMode(char prefix, bool sign) {
	if(sign) {
		std::map<char, IRCUserModeEnum::Enumeration>::iterator iter = mUserModeTranslateSigns.find(prefix);
		if(iter!=mUserModeTranslateSigns.end()) {
			return iter->second;
		}
	}
	else {
		std::map<char, IRCUserModeEnum::Enumeration>::iterator iter = mUserModeTranslate.find(prefix);
		if(iter!=mUserModeTranslate.end()) {
			return iter->second;
		}
	}

	return IRCUserModeEnum::UNKOWN;
}

char IRCConnection::getIRCUserModeChar(IRCUserModeEnum::Enumeration mode) {
	std::map<IRCUserModeEnum::Enumeration, char>::iterator iter = mEnumToUserModeTranslate.find(mode);
	if(iter!=mEnumToUserModeTranslate.end()) {
		return iter->second;
	}

	return '\0';
}

bool IRCConnection::handleCTCP() {
	//Check if its a CTCP request
	if(mCurrentMessage.params[1][0] != (char)0x01) {
		return false;
	}
	size_t pos = mCurrentMessage.params[1].find_first_of((char)0x01, 1);
	if(pos == std::string::npos) {
		return false;
	}

	mCurrentMessage.isCTCPMessage = true;
	mCurrentMessage.params[1] = mCurrentMessage.params[1].substr(1, pos-1);


	//Handle main CTCP requests
	std::string ctcpCommand = mCurrentMessage.params[1];
	pos = mCurrentMessage.params[1].find_first_of(' ');
	if(pos != std::string::npos) {
		ctcpCommand = mCurrentMessage.params[1].substr(0, pos);
	}
	if(mCurrentMessage.command == "PRIVMSG") {
		if(ctcpCommand == "VERSION") {
			std::stringstream str;
			str << "VERSION " << mIRC.getClientName() << ":" << mIRC.getClientVersion() << ":" << mIRC.getClientOS();
			sendCTCPAnswer(mCurrentMessage.prefix.nickOrServer, str.str());
		}
		else if(ctcpCommand == "PING") {
			sendCTCPAnswer(mCurrentMessage.prefix.nickOrServer, mCurrentMessage.params[1]);
		}
		else if(ctcpCommand == "TIME") {
			std::time_t rawtime;
			struct tm* timeinfo;
			std::time(&rawtime);
			timeinfo = localtime(&rawtime);

			std::string timeString = std::asctime(timeinfo);
			sendCTCPAnswer(mCurrentMessage.prefix.nickOrServer, "TIME "+timeString.substr(0, timeString.size()-1));
		}
		else if(ctcpCommand == "CLIENTINFO") {
			sendCTCPAnswer(mCurrentMessage.prefix.nickOrServer, "CLIENTINFO ACTION CLIENTINFO PING TIME VERSION");
		}
		else if(ctcpCommand == "ACTION") {
			std::string action = "";
			if(pos != std::string::npos) {
				action = mCurrentMessage.params[1].substr(pos+1);
			}
			onCTCPAction(mIRC, *this, mCurrentMessage, action);
			mIRC.onCTCPAction(mIRC, *this, mCurrentMessage, action);
		}
		else {
			onCTCPRequest(mIRC, *this, mCurrentMessage, ctcpCommand);
			mIRC.onCTCPRequest(mIRC, *this, mCurrentMessage, ctcpCommand);
		}
	}
	else {
		onCTCPAnswer(mIRC, *this, mCurrentMessage, ctcpCommand);
		mIRC.onCTCPAnswer(mIRC, *this, mCurrentMessage, ctcpCommand);
	}

	return true;
}

void IRCConnection::commandPART() {
	if(mCurrentMessage.prefix.nickOrServer != mNick) {
		getChannel(mCurrentMessage.params[0])->removeMember(mCurrentMessage.prefix.nickOrServer);
	}
	else {
		std::map<std::string, IRCChannel>::iterator iter = mChannels.find(mCurrentMessage.params[0]);
		if(iter != mChannels.end()) {
			mChannels.erase(iter);
		}
	}

	onPart(mIRC, *this, mCurrentMessage);
	mIRC.onPart(mIRC, *this, mCurrentMessage);
}

void IRCConnection::commandKICK() {
	if(mCurrentMessage.params[1] != mNick) {
		getChannel(mCurrentMessage.params[0])->removeMember(mCurrentMessage.params[1]);
	}
	else {
		std::map<std::string, IRCChannel>::iterator iter = mChannels.find(mCurrentMessage.params[0]);
		if(iter != mChannels.end()) {
			mChannels.erase(iter);
		}
	}

	onKick(mIRC, *this, mCurrentMessage);
	mIRC.onKick(mIRC, *this, mCurrentMessage);
}

void IRCConnection::commandJOIN() {
	if(mCurrentMessage.prefix.nickOrServer != mNick) {
		IRCMember member;
		member.nick = mCurrentMessage.prefix.nickOrServer;
		getChannel(mCurrentMessage.params[0])->members[member.nick] = member;
	}
	else {
		IRCChannel channel;
		channel.name = mCurrentMessage.params[0];
		channel.topic = "";
		channel.nameListFull = true;
		mChannels[channel.name] = channel;
	}

	onJoin(mIRC, *this, mCurrentMessage);
	mIRC.onJoin(mIRC, *this, mCurrentMessage);
}

void IRCConnection::commandQUIT() {
	if(mCurrentMessage.prefix.nickOrServer != mNick) {
		std::map<std::string, IRCChannel>::iterator iter;
		for(iter=mChannels.begin(); iter!=mChannels.end(); ++iter) {
			iter->second.removeMember(mCurrentMessage.prefix.nickOrServer);
		}
	}

	onQuit(mIRC, *this, mCurrentMessage);
	mIRC.onQuit(mIRC, *this, mCurrentMessage);
}

void IRCConnection::commandKILL() {
	if(mCurrentMessage.params[0] != mNick) {
		std::map<std::string, IRCChannel>::iterator iter;
		for(iter=mChannels.begin(); iter!=mChannels.end(); ++iter) {
			iter->second.removeMember(mCurrentMessage.params[0]);
		}
	}

	onKill(mIRC, *this, mCurrentMessage);
	mIRC.onKill(mIRC, *this, mCurrentMessage);
}

void IRCConnection::commandPRIVMSG() {
	if(handleCTCP()) {
		return;
	}

	onMessage(mIRC, *this, mCurrentMessage);
	mIRC.onMessage(mIRC, *this, mCurrentMessage);
	if(mCurrentMessage.params[0] == mNick) {
		onPrivateMessage(mIRC, *this, mCurrentMessage);
		mIRC.onPrivateMessage(mIRC, *this, mCurrentMessage);
	}
	else {
		onChannelMessage(mIRC, *this, mCurrentMessage);
		mIRC.onChannelMessage(mIRC, *this, mCurrentMessage);
	}
}

void IRCConnection::commandTOPIC() {
	getChannel(mCurrentMessage.params[0])->topic = mCurrentMessage.params[1];

	onTopicChanged(mIRC, *this, mCurrentMessage);
	mIRC.onTopicChanged(mIRC, *this, mCurrentMessage);
}

void IRCConnection::commandNICK() {
	if(mCurrentMessage.prefix.nickOrServer == mNick) {
		mNick = mCurrentMessage.params[0];
	}

	std::map<std::string, IRCChannel>::iterator iter;
	for(iter=mChannels.begin(); iter!=mChannels.end(); ++iter) {
		iter->second.changeMember(mCurrentMessage.prefix.nickOrServer, mCurrentMessage.params[0]);
	}

	onNickChanged(mIRC, *this, mCurrentMessage);
	mIRC.onNickChanged(mIRC, *this, mCurrentMessage);
}

void IRCConnection::commandINVITE() {
	onInvite(mIRC, *this, mCurrentMessage);
	mIRC.onInvite(mIRC, *this, mCurrentMessage);
}

void IRCConnection::commandNOTICE() {
	if(handleCTCP()) {
		return;
	}

	onNotice(mIRC, *this, mCurrentMessage);
	mIRC.onNotice(mIRC, *this, mCurrentMessage);
}

void IRCConnection::commandMODE() {
	IRCChannel* channel = getChannel(mCurrentMessage.params[0]);
	if(!channel) {
		return;
	}

	unsigned int paramsPos = 2;
	char mode='+';
	std::map<char, char>::iterator iter;
	for(unsigned int i=0; i<mCurrentMessage.params[1].size(); i++) {
		if(mCurrentMessage.params[1][i] == '-') {
			mode = '-';
		}
		else if(mCurrentMessage.params[1][i] == '+') {
			mode = '+';
		}
		else {
			iter = mChanModes.find(mCurrentMessage.params[1][i]);
			if(iter != mChanModes.end()) {
				if(iter->second == 'A' || iter->second == 'B' || (iter->second == 'C' && mode == '+')) {
					paramsPos++;
				}
			}
			else {
				IRCUserModeEnum::Enumeration enumeration = getIRCUserMode(mCurrentMessage.params[1][i], false);
				if(enumeration > IRCUserModeEnum::NOTHING) {
					if(mode == '+') {
						channel->getMember(mCurrentMessage.params[paramsPos])->addMode(enumeration);
						onUserObtainMode(mIRC, *this, mCurrentMessage, mCurrentMessage.params[paramsPos], enumeration);
						mIRC.onUserObtainMode(mIRC, *this, mCurrentMessage, mCurrentMessage.params[paramsPos], enumeration);
					}
					else {
						channel->getMember(mCurrentMessage.params[paramsPos])->removeMode(enumeration);
						onUserLoseMode(mIRC, *this, mCurrentMessage, mCurrentMessage.params[paramsPos], enumeration);
						mIRC.onUserLoseMode(mIRC, *this, mCurrentMessage, mCurrentMessage.params[paramsPos], enumeration);

						mNeedNewNAMERequest = true;
					}
					paramsPos++;
				}
				else {
					std::cout << "----> Error: Mode " << mode << mCurrentMessage.params[1][i] << " is unknown" << std::endl;
				}
			}
		}
	}
}

void IRCConnection::numeric005() {
	for(unsigned int i=1; i<mCurrentMessage.params.size(); i++) {
		size_t pos = mCurrentMessage.params[i].find_first_of('=', pos+1);
		if(pos != std::string::npos) {
			if(mCurrentMessage.params[i].substr(0, pos) == "PREFIX") {
				size_t posChars = mCurrentMessage.params[i].find_first_of('(', pos);
				size_t posSigns = mCurrentMessage.params[i].find_first_of(')', pos);

				IRCUserModeEnum::Enumeration right = IRCUserModeEnum::OP;
				for(int i2=1; i2<posSigns-posChars; i2++) {
					if(mCurrentMessage.params[i][posChars+i2] == 'o') {
						right = IRCUserModeEnum::OP;
					}
					else if(mCurrentMessage.params[i][posChars+i2] == 'h') {
						right = IRCUserModeEnum::HALF_OP;
					}
					else if(mCurrentMessage.params[i][posChars+i2] == 'v') {
						right = IRCUserModeEnum::VOICE;
					}
					else {
						right = IRCUserModeEnum::NOTHING;
					}

					mEnumToUserModeTranslate[right] = mCurrentMessage.params[i][posChars+i2];
					mUserModeTranslate[mCurrentMessage.params[i][posChars+i2]] = right;
					mUserModeTranslateSigns[mCurrentMessage.params[i][posSigns+i2]] = right;
				}
			}
			else if(mCurrentMessage.params[i].substr(0, pos) == "CHANMODES") {
				char code = 'A';
				for(int i2=pos+1; i2<mCurrentMessage.params[i].size(); i2++) {
					if(mCurrentMessage.params[i][i2] == ',') {
						code++;
					}
					else {
						mChanModes[mCurrentMessage.params[i][i2]] = code;
					}
				}
			}
		}
	}
}

void IRCConnection::numeric332() {
	getChannel(mCurrentMessage.params[1])->topic = mCurrentMessage.params[2];
}

void IRCConnection::numeric353() {
	IRCChannel* channel = getChannel(mCurrentMessage.params[2]);
	if(channel->nameListFull) {
		channel->members.clear();
		channel->nameListFull = false;
	}

	std::vector<std::string> tmpUserList;
	boost::split(tmpUserList, mCurrentMessage.params[3], boost::is_any_of(" "));
	for(unsigned int i=0; i<tmpUserList.size(); i++) {
		if(tmpUserList[i] == "") {
			continue;
		}

		IRCMember tmpMember;
		tmpMember.nick = tmpUserList[i];

		for(unsigned int i2=0; i2<tmpMember.nick.size(); i2++) {
			IRCUserModeEnum::Enumeration enumeration = getIRCUserMode(tmpUserList[i][i2], true);
			if(enumeration==IRCUserModeEnum::UNKOWN) {
				tmpMember.nick = tmpMember.nick.substr(i2);
				break;
			}
			else {
				tmpMember.addMode(enumeration);
			}
		}
		channel->members[tmpMember.nick] = tmpMember;
	}
}

void IRCConnection::numeric366() {
	getChannel(mCurrentMessage.params[1])->nameListFull = true;
}
