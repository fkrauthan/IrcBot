/*
 * ircconnection.cpp
 *
 *  Created on: 09.06.2010
 *      Author: fkrauthan
 */

#include "ircconnection.h"
#include "irc.h"
#include "irceventhandler.h"
#include <iostream>
#include <ctime>
#include <boost/algorithm/string.hpp>
#ifdef linux
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <errno.h>
#else
    #include <winsock2.h>
#endif


IrcConnection::IrcConnectionSendThread::IrcConnectionSendThread(int socket) {
	mSocket = socket;
}

IrcConnection::IrcConnectionSendThread::~IrcConnectionSendThread() {
	std::cout << "~IrcConnectionSendThread()" << std::endl;
}

void IrcConnection::IrcConnectionSendThread::run() {
	mQuit = false;

	while(!mQuit) {
		boost::mutex::scoped_lock lock(mMutex);
		while(mMessageQueue.empty()) {
			mCV.wait(lock);
		}

		while(!mMessageQueue.empty()) {
			std::string line = mMessageQueue.front();
			sendLine(line.c_str(), line.size());
			mMessageQueue.pop();

			//Sleep a while to prevent flooding
			usleep(SLEEP_TIME_MS*1000);
		}
	}
}

void IrcConnection::IrcConnectionSendThread::quitThread() {
	boost::mutex::scoped_lock lock(mMutex);
	mQuit = true;

	lock.unlock();
	if(mMessageQueue.empty()) {
		mCV.notify_one();
	}
}

void IrcConnection::IrcConnectionSendThread::addMessage(const std::string& message) {
	boost::mutex::scoped_lock lock(mMutex);
	bool const wasEmpty=mMessageQueue.empty();
	mMessageQueue.push(message);

	lock.unlock();
	if(wasEmpty) {
		mCV.notify_one();
	}
}

void IrcConnection::IrcConnectionSendThread::sendLine(const char* const buf, const  int size) {
	int bytesSent = 0;
	do {
		int result = send(mSocket, buf + bytesSent, size - bytesSent, 0);
		if(result < 0) {
			break;
		}
		bytesSent += result;
	} while(bytesSent < size);
}

IrcConnection::IrcConnection(Irc* irc, const std::string& id, const std::string& server, const std::string& nick, int port) {
	mConnectionThreadClass = NULL;
	mConnectionThread = NULL;
	mConnected = false;

	mIrc = irc;
	mServer = server;
	mNick = nick;
	mPort = port;
	mID = id;


#ifndef linux
	WSADATA w;
	if(int result = WSAStartup(MAKEWORD(2,2), &w) != 0) {
		std::cout << "--> Winsock 2 konnte nicht gestartet werden! Error #" << result << std::endl;
		return NULL;
    }
#endif


	hostent* phe = gethostbyname(server.c_str());
	if(phe == NULL) {
		std::cout << "--> Hostname konnte nicht aufgeloest werden!" << std::endl;
		throw IrcConnectionException("Can't resolve hostename");
    }


	if(phe->h_addrtype != AF_INET) {
		std::cout << "--> Ungueltiger Adresstyp!" << std::endl;
		throw IrcConnectionException("Unkown address type");
    }

	if(phe->h_length != 4) {
		std::cout << "--> Ungueltiger IP-Typ (Nur IP V4)!" << std::endl;
		throw IrcConnectionException("Unkown ip type (only IP v4)");
    }

	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(mSocket == -1) {
		std::cout << "--> Socket konnte nicht erstellt werden!" << std::endl;
		throw IrcConnectionException("Can't create socket connection");
	}

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_port = htons(port);
	char** p = phe->h_addr_list;
	int result;

	do {
		if(*p == NULL) {
			std::cout << "--> Verbindung fehlgeschlagen!" << std::endl;

#ifdef linux
			close(mSocket);
#else
			closesocket(mSocket);
#endif

			throw IrcConnectionException("start connection failed");
        }

		service.sin_addr.s_addr = *reinterpret_cast<unsigned long*>(*p);
        ++p;
        result = connect(mSocket, reinterpret_cast<sockaddr*>(&service), sizeof(service));
    } while(result == -1);


	//Create send Thread
	mConnectionThreadClass = new IrcConnectionSendThread(mSocket);
	mConnectionThread = new boost::thread(boost::bind(&IrcConnectionSendThread::run, mConnectionThreadClass));


	//Send irc beginning handshake
	sendLine("NICK "+mNick+"\r\n");
	sendLine("USER "+mNick+" \"\" \"\" :"+mNick+"\r\n");
}

IrcConnection::~IrcConnection() {
	std::cout << "~IrcConnection() START" << std::endl;

	if(mConnectionThread) {
		mConnectionThreadClass->quitThread();
		if(!mConnectionThread->timed_join(boost::posix_time::seconds(5))) {
			mConnectionThread->interrupt();
		}
		delete mConnectionThread;
	}
	delete mConnectionThreadClass;


	if(mSocket>0) {
#ifndef WIN32
		close(mSocket);
#else
		closesocket(mSocket);
#endif
	}

	std::cout << "~IrcConnection() END" << std::endl;
}

void IrcConnection::run() {
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
		if(!mConnected && mCurrentMessage.isNumeric && mCurrentMessage.command=="001") {
			std::cout << "--> Connection to server is ready" << std::endl;
			mConnected = true;


			std::vector<IrcEventHandler*>::iterator iter;
			for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
				if(!(*iter)->onConnect(*this)) {
					break;
				}
			}
		}
		else if(mConnected) {
			if(!mCurrentMessage.isNumeric) {
				//Handle normal irc messages
				std::vector<IrcEventHandler*>::iterator iter;
				for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
					if(!(*iter)->onMessage(*this, mCurrentMessage)) {
						break;
					}
				}

				if(mCurrentMessage.command == "PART") {
					std::cout << "--> User " << mCurrentMessage.getUsername() << " has left the channel " << mCurrentMessage.target << std::endl;
					if(mCurrentMessage.getUsername() != mNick) {
						mChannels[mCurrentMessage.target].removeMember(mCurrentMessage.getUsername());
					}
					else {
						std::map<std::string,IrcChannel>::iterator iter = mChannels.find(mCurrentMessage.target);
						if(iter != mChannels.end()) {
							mChannels.erase(iter);
						}
					}

					for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
						if(!(*iter)->onPart(*this, mCurrentMessage)) {
							break;
						}
					}
				}
				else if(mCurrentMessage.command == "KICK") {
					std::string target = mCurrentMessage.params.substr(0, mCurrentMessage.params.find_first_of(' '));
					std::cout << "--> User " << target << " was kicked by " << mCurrentMessage.prefix << " from channel " << mCurrentMessage.target << std::endl;

					if(target != mNick) {
						mChannels[mCurrentMessage.target].removeMember(target);
					}
					else {
						std::map<std::string,IrcChannel>::iterator iter = mChannels.find(mCurrentMessage.target);
						if(iter != mChannels.end()) {
							mChannels.erase(iter);
						}
					}

					for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
						if(!(*iter)->onKick(*this, mCurrentMessage, target)) {
							break;
						}
					}
				}
				else if(mCurrentMessage.command == "JOIN") {
					//Start adding to channel list
					if(mCurrentMessage.getUsername() == mNick) {
						IrcChannel tmpChannel;
						tmpChannel.name = mCurrentMessage.target;
						tmpChannel.topic = "";
						tmpChannel.nameListFull = true;
						mChannels[mCurrentMessage.target] = tmpChannel;
					}
					else {
						IrcChannelMember tmpMember;
						tmpMember.nick = mCurrentMessage.getUsername();
						mChannels[mCurrentMessage.target].members.push_back(tmpMember);

						std::cout << "--> User " << tmpMember.nick << " has joined channel " << mCurrentMessage.target << std::endl;
					}

					for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
						if(!(*iter)->onJoin(*this, mCurrentMessage)) {
							break;
						}
					}
				}
				else if(mCurrentMessage.command == "QUIT") {
					std::cout << "--> User " << mCurrentMessage.prefix << " has left the irc" << std::endl;
					if(mCurrentMessage.getUsername() != mNick) {
						std::map<std::string,IrcChannel>::iterator iter;
						for(iter=mChannels.begin(); iter!=mChannels.end(); ++iter) {
							mChannels[iter->first].removeMember(mCurrentMessage.getUsername());
						}
					}

					for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
						if(!(*iter)->onQuit(*this, mCurrentMessage)) {
							break;
						}
					}
				}
				else if(mCurrentMessage.command == "KILL") {
					std::cout << "--> User " << mCurrentMessage.target << " was killed from the irc" << std::endl;
					if(mCurrentMessage.target != mNick) {
						std::map<std::string,IrcChannel>::iterator iter;
						for(iter=mChannels.begin(); iter!=mChannels.end(); ++iter) {
							mChannels[iter->first].removeMember(mCurrentMessage.target);
						}
					}

					for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
						if(!(*iter)->onKill(*this, mCurrentMessage, mCurrentMessage.target)) {
							break;
						}
					}
				}

				else if(mCurrentMessage.command == "PRIVMSG") {
					if(mCurrentMessage.target == mNick) {
						for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
							if(!(*iter)->onPrivateMessage(*this, mCurrentMessage)) {
								break;
							}
						}
					}
					else {
						for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
							if(!(*iter)->onChannelMessage(*this, mCurrentMessage)) {
								break;
							}
						}
					}

					//Check for CTCP Message
					if(mCurrentMessage.params[0] == 0x01) {
						handleCTCPMessage();
					}
				}
				else if(mCurrentMessage.command == "TOPIC") {
					mChannels[mCurrentMessage.target].topic = mCurrentMessage.params;
					std::cout << "--> User " << mCurrentMessage.getUsername() << " has changed the topic from channel " << mCurrentMessage.target << " to " << mCurrentMessage.target << std::endl;

					for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
						if(!(*iter)->onTopicChanged(*this, mCurrentMessage)) {
							break;
						}
					}
				}
				else if(mCurrentMessage.command == "NICK") {
					std::cout << "--> User " << mCurrentMessage.getUsername() << " has changed his nick to " << mCurrentMessage.target << std::endl;

					if(mCurrentMessage.getUsername() == mNick) {
						mNick = mCurrentMessage.target;
					}

					std::map<std::string,IrcChannel>::iterator chanIter;
					for(chanIter=mChannels.begin(); chanIter!=mChannels.end(); ++chanIter) {
						chanIter->second.changeMember(mCurrentMessage.getUsername(), mCurrentMessage.target);
					}

					for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
						if(!(*iter)->onNickChanged(*this, mCurrentMessage)) {
							break;
						}
					}
				}
				else if(mCurrentMessage.command == "INVITE") {
					for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
						if(!(*iter)->onInvite(*this, mCurrentMessage)) {
							break;
						}
					}
				}
				else if(mCurrentMessage.command == "NOTICE") {
					for(iter=mEventHandler.begin(); iter!=mEventHandler.end(); ++iter) {
						if(!(*iter)->onNotice(*this, mCurrentMessage)) {
							break;
						}
					}
				}
				else if(mCurrentMessage.command == "MODE") {
					std::cout << "--> User " << mCurrentMessage.getUsername() << " set Mode(s) on channel " << mCurrentMessage.target << " to " << mCurrentMessage.params << std::endl;

					parseModeMessage();
				}
			}
			else {
				//Handle numerics
				if(mCurrentMessage.command == "005") {
					size_t pos = 0;
					while((pos=mCurrentMessage.params.find_first_of(' ', pos+1)) != std::string::npos) {
						size_t pos2 = mCurrentMessage.params.find_first_of('=', pos+1);
						if(pos2 != std::string::npos) {
							if(mCurrentMessage.params.substr(pos+1, pos2-pos-1) == "PREFIX") {
								size_t posChars = mCurrentMessage.params.find_first_of('(', pos2+1);
								size_t posSigns = mCurrentMessage.params.find_first_of(')', pos2+1);

								IrcRightEnum::Enumeration right = IrcRightEnum::OP;
								for(int i=1; i<posSigns-posChars; i++) {
									if(mCurrentMessage.params[posChars+i] == 'o') {
										right = IrcRightEnum::OP;
									}
									else if(mCurrentMessage.params[posChars+i] == 'h') {
										right = IrcRightEnum::HALF_OP;
									}
									else if(mCurrentMessage.params[posChars+i] == 'v') {
										right = IrcRightEnum::VOICE;
									}
									else {
										mChanModes[mCurrentMessage.params[posChars+i]] = 'B';
										continue;
									}

									mRightTranslate[mCurrentMessage.params[posChars+i]] = right;
									mRightTranslateSigns[mCurrentMessage.params[posSigns+i]] = right;
								}
							}
							else if(mCurrentMessage.params.substr(pos+1, pos2-pos-1) == "CHANMODES") {
								size_t posEnds = mCurrentMessage.params.find_first_of(' ', pos2+1);

								char code = 'A';
								for(int i=1; i<posEnds-pos2; i++) {
									if(mCurrentMessage.params[pos2+i] == ',') {
										code++;
									}
									else {
										mChanModes[mCurrentMessage.params[pos2+i]] = code;
									}
								}
							}
						}
					}

				}
				else if(mCurrentMessage.command == "332") {
					size_t pos;
					std::string channelName = mCurrentMessage.params.substr(0, (pos=mCurrentMessage.params.find_first_of(' ')));
					std::string channelTopic = mCurrentMessage.params.substr(pos+1);
					if(channelTopic.substr(0, 1)==":") channelTopic = channelTopic.substr(1);

					mChannels[channelName].topic = channelTopic;
					std::cout << "--> Channel topic for channel " << channelName << " is " << channelTopic << std::endl;
				}
				else if(mCurrentMessage.command == "353") {
					std::string tmpParseString = mCurrentMessage.params.substr(2);

					std::string channelName = tmpParseString.substr(0, tmpParseString.find_first_of(' '));
					std::string channelUserList = tmpParseString.substr(tmpParseString.find_first_of(' ')+1);
					if(channelUserList.substr(0, 1)==":") channelUserList = channelUserList.substr(1);
					std::cout << "---> Channel userlist for channel " << channelName << " is " << channelUserList << std::endl;

					std::vector<std::string> tmpUserList;
					boost::split(tmpUserList, channelUserList, boost::is_any_of(" "));

					if(mChannels[channelName].nameListFull) {
						mChannels[channelName].members.clear();
						mChannels[channelName].nameListFull = false;
					}
					for(unsigned int i=0; i<tmpUserList.size(); i++) {
						if(tmpUserList[i] == "") continue;

						IrcChannelMember tmpMember;
						tmpMember.nick = tmpUserList[i];
						IrcRightEnum::Enumeration enumeration = getIrcRight(tmpUserList[i][0], true);
						if(enumeration != IrcRightEnum::NOTHING) {
							tmpMember.addMode(enumeration);
							tmpMember.nick = tmpMember.nick.substr(1);
						}
						mChannels[channelName].members.push_back(tmpMember);
					}
				}
				else if(mCurrentMessage.command == "366") {
					std::string channelName = mCurrentMessage.params.substr(0, mCurrentMessage.params.find_first_of(' '));
					mChannels[channelName].nameListFull = true;
					std::cout << "--> Finish reciving channel userlist for channel " << channelName << std::endl;
				}
			}
		}
	}
}

void IrcConnection::quitThread() {
	mQuit = true;
}

void IrcConnection::joinChan(const std::string& channel) {
	std::cout << "--> Join channel " << channel << std::endl;
	sendLine("JOIN "+channel+"\r\n");
}

void IrcConnection::partChan(const std::string& channel) {
	std::cout << "--> Part channel " << channel << std::endl;
	sendLine("PART "+channel+"\r\n");
}

void IrcConnection::sendQuit(const std::string& msg) {
	if(mConnected) {
		mConnected = false;
		sendLine("QUIT :"+msg+"\r\n");
	}
}

void IrcConnection::sendMessage(const std::string& target, const std::string& message) {
	sendLine("PRIVMSG "+target+" :"+message+"\r\n");
}

void IrcConnection::sendNotice(const std::string& target, const std::string& message) {
	sendLine("NOTICE "+target+" :"+message+"\r\n");
}

void IrcConnection::sendAction(const std::string& target, const std::string& message) {
	sendCTCP(target, "ACTION "+message);
}

void IrcConnection::sendCTCP(const std::string& target, const std::string& message) {
	std::stringstream str;
	str << (char)0x01 << message << (char)0x01;
	sendNotice(target, str.str());
}

void IrcConnection::changeNick(const std::string& nick) {
	sendLine("NICK "+nick+"\r\n");
}

void IrcConnection::setUserMode(const std::string& mode) {
	setMode(mNick, mode);
}

void IrcConnection::setMode(const std::string& nick, const std::string& mode) {
	sendLine("MODE "+nick+" "+mode+"\r\n");
}

std::map<std::string, IrcChannel>& IrcConnection::getChannels() {
	return mChannels;
}

std::string& IrcConnection::getServer() {
	return mServer;
}

int IrcConnection::getPort() {
	return mPort;
}

std::string& IrcConnection::getNick() {
	return mNick;
}

std::string& IrcConnection::getID() {
	return mID;
}

Irc* IrcConnection::getIrcManager() {
	return mIrc;
}

void IrcConnection::registerEventHandler(IrcEventHandler* handler) {
	mEventHandler.push_back(handler);
}

void IrcConnection::unregisterEventHandler(IrcEventHandler* handler) {
	std::vector<IrcEventHandler*>::iterator iter;
	if((iter=std::find(mEventHandler.begin(), mEventHandler.end(), handler))!=mEventHandler.end()) {
		mEventHandler.erase(iter);
	}
}

bool IrcConnection::parseMessage(const std::string& message) {
	mCurrentMessage.ircLine = message;

	mCurrentMessage.command = "";
	mCurrentMessage.hasDetailedPrefix = false;
	mCurrentMessage.isNumeric = false;
	mCurrentMessage.msgPrefix.host = "";
	mCurrentMessage.msgPrefix.nick_or_server = "";
	mCurrentMessage.msgPrefix.user = "";
	mCurrentMessage.target = "";
	mCurrentMessage.params = "";
	mCurrentMessage.prefix = "";


	size_t pos = std::string::npos;
	size_t pos2 = std::string::npos;
	std::string messagePart;

	if(message.substr(0, 1) != ":") return false;
	if((pos = message.find_first_of(' ')) == std::string::npos) return false;

	//Read the prefix
	mCurrentMessage.prefix = message.substr(1, pos-1);
	if((pos2 = mCurrentMessage.prefix.find('!')) != std::string::npos) {
		mCurrentMessage.hasDetailedPrefix = true;
		mCurrentMessage.msgPrefix.nick_or_server = mCurrentMessage.prefix.substr(0, pos2);
		messagePart = mCurrentMessage.prefix.substr(pos2+1);
		if((pos2 = messagePart.find('@')) != std::string::npos) {
			mCurrentMessage.msgPrefix.user = messagePart.substr(0, pos2);
			mCurrentMessage.msgPrefix.host = messagePart.substr(pos2+1);
		}
	}

	//Read the command
	messagePart = message.substr(pos+1);
	if((pos = messagePart.find_first_of(' ')) == std::string::npos) return false;
	mCurrentMessage.command = messagePart.substr(0, pos);
	if(isNumeric(mCurrentMessage.command)) {
		mCurrentMessage.isNumeric = true;
	}

	//Read the target
	messagePart = messagePart.substr(pos+1);
	if((pos = messagePart.find_first_of(' ')) == std::string::npos) {
		mCurrentMessage.target = messagePart;
		if(mCurrentMessage.target.substr(0, 1) == ":") {
			mCurrentMessage.target = mCurrentMessage.target.substr(1);
		}
		return true;
	}

	mCurrentMessage.target = messagePart.substr(0, pos);
	if(mCurrentMessage.target.substr(0, 1) == ":") {
		mCurrentMessage.target = mCurrentMessage.target.substr(1);
	}

	//Read the params
	mCurrentMessage.params = messagePart.substr(pos+1);
	if(mCurrentMessage.params.substr(0, 1) == ":") {
		mCurrentMessage.params = mCurrentMessage.params.substr(1);
	}

	return true;
}

void IrcConnection::sendLine(const std::string& message) {
	mConnectionThreadClass->addMessage(message);
}

std::string IrcConnection::readLine() {
	if(mReadLines.empty()) {
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
			mReadLines.push(tmpString);
			tmpLine = tmpLine.substr(pos+1);
		}
		if(*(tmpLine.end()-1) == '\r') {
			tmpLine.erase(tmpLine.end()-1);
		}
		mReadLines.push(tmpLine);
	}


	std::string ret = mReadLines.front();
	mReadLines.pop();
	return ret;
}
void IrcConnection::handleCTCPMessage() {
	std::string params = mCurrentMessage.params.substr(1, mCurrentMessage.params.find_first_of(0x01, 1)-1);
	size_t pos = params.find_first_of(' ');
	if(pos == std::string::npos) {
		pos = params.size();
	}
	std::string command = params.substr(0, pos);

	if(mCurrentMessage.command=="PRIVMSG") {
		if(command == "VERSION") {
			sendCTCP(mCurrentMessage.getUsername(), "VERSION "+mIrc->getClientInfo().clientName+":"+mIrc->getClientInfo().version+":Unknown");
		}
		else if(command == "PING") {
			sendCTCP(mCurrentMessage.getUsername(), params);
		}
		else if(command == "TIME") {
			std::time_t rawtime;
			struct tm* timeinfo;
			std::time(&rawtime);
			timeinfo = localtime(&rawtime);

			std::string timeString = std::asctime(timeinfo);
			sendCTCP(mCurrentMessage.getUsername(), "TIME "+timeString.substr(0, timeString.size()-1));
		}
		else if(command == "CLIENTINFO") {
			sendCTCP(mCurrentMessage.getUsername(), "CLIENTINFO ACTION CLIENTINFO PING TIME VERSION");
		}
	}
}

void IrcConnection::parseModeMessage() {
	std::map<std::string, IrcChannel>::iterator channelIter = mChannels.find(mCurrentMessage.target);
	if(channelIter == mChannels.end()) {
		return;
	}
	IrcChannel& channel = mChannels[mCurrentMessage.target];

	size_t pos = mCurrentMessage.params.find_first_of(' ');
	size_t pos2 = mCurrentMessage.params.find_first_of(' ', pos+1);
	size_t endOfModesList = pos;
	if(pos2 == std::string::npos) {
		pos2 = mCurrentMessage.params.size();
	}
	if(endOfModesList == std::string::npos) {
		endOfModesList = mCurrentMessage.params.size();
	}

	std::map<char, char>::iterator iter;
	std::map<char, IrcRightEnum::Enumeration>::iterator iter2;
	char mode = '-';

	for(unsigned int i=0; i<endOfModesList; i++) {
		if(mCurrentMessage.params[i] == '-') {
			mode = '-';
		}
		else if(mCurrentMessage.params[i] == '+') {
			mode = '+';
		}
		else {
			iter = mChanModes.find(mCurrentMessage.params[i]);
			if(iter != mChanModes.end()) {
				if(iter->second == 'A' || iter->second == 'B' || (iter->second == 'C' && mode == '+')) {
					pos = mCurrentMessage.params.find_first_of(' ', pos+1);
					pos2 = mCurrentMessage.params.find_first_of(' ', pos+1);
					if(pos2 == std::string::npos) {
						pos2 = mCurrentMessage.params.size();
					}
				}
			}
			else {
				iter2 = mRightTranslate.find(mCurrentMessage.params[i]);
				if(iter2 != mRightTranslate.end()) {
					std::string nick = mCurrentMessage.params.substr(pos+1, pos2-pos-1);
					pos = mCurrentMessage.params.find_first_of(' ', pos+1);
					pos2 = mCurrentMessage.params.find_first_of(' ', pos+1);
					if(pos2 == std::string::npos) {
						pos2 = mCurrentMessage.params.size();
					}

					IrcChannelMember* channelMember = channel.getMember(nick);
					if(!channelMember) {
						std::cout << "----> Error: User " << nick << " was not found in channel " << channel.name << " to set mode " << mode << mCurrentMessage.params[i] << std::endl;
					}
					else {
						std::cout << "DEBUG: " << nick << " Mode " << mode << mCurrentMessage.params[i] << std::endl;

						if(mode == '+') {
							channelMember->addMode(iter2->second);
						}
						else {
							channelMember->removeMode(iter2->second);
						}
					}
				}
				else {
					std::cout << "----> Error: Mode " << mode << mCurrentMessage.params[i] << " is unknown" << std::endl;
				}
			}
		}
	}
}

IrcRightEnum::Enumeration IrcConnection::getIrcRight(char prefix, bool sign) {
	if(sign) {
		std::map<char, IrcRightEnum::Enumeration>::iterator iter = mRightTranslateSigns.find(prefix);
		if(iter!=mRightTranslateSigns.end()) {
			return iter->second;
		}
	}
	else {
		std::map<char, IrcRightEnum::Enumeration>::iterator iter = mRightTranslate.find(prefix);
		if(iter!=mRightTranslate.end()) {
			return iter->second;
		}
	}

	return IrcRightEnum::NOTHING;
}

bool IrcConnection::isNumeric(const std::string& string) {
	for(unsigned int i = 0; i < string.length(); i++) {
		if(!std::isdigit(string[i])) {
			return false;
		}
	}

	return true;
}
