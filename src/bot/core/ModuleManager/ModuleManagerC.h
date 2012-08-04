/*
 * ModuleManagerC.h
 *
 *  Created on: 20.05.2010
 *      Author: fkrauthan
 */

#ifndef MODULEMANAGERC_H_
#define MODULEMANAGERC_H_

extern "C" const char* ModuleManager_callMethod(void* modulemanager, const char* sender, const char* reciver, const char* action, const char* jsonString, void* connection);
extern "C" void* ModuleManager_callMethodWithPtrReturn(void* modulemanager, const char* sender, const char* reciver, const char* action, const char* jsonString, void* connection);

#endif /* MODULEMANAGERC_H_ */
