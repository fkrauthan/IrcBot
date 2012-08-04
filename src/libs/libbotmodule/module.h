/*
 * module.h
 *
 *  Created on: 20.04.2010
 *      Author: fkrauthan
 */

#ifndef MODULE_H_
#define MODULE_H_

class Irc;
class IrcConnection;
class ModuleManager;

/*
 * 1.) IRC Pointer
 * 2.) ModuleManager Pointer
 * 3.) Module ID for this module
 * 4.) JSON String with the params map
 */
typedef bool (*module_init_f) (Irc*, ModuleManager*, const char*, const char*);
typedef void (*module_remove_f) (void);

/*
 * 1. IRC Connection Pointer
 * 2. Sender id
 * 3. Event name
 * 4. Message as JSON String
 * 5. Optional string (only used for onKick and onKill event
 */
typedef bool (*module_onEvent_f)(IrcConnection*, const char*, const char*, const char*, const char*);


/*
 * 1. Sender id
 * 2. Action
 * 3. JSON Params
 * 4. IRC Connection Pointer
 */
typedef const char* (*module_onInternalMessage_f) (const char*, const char*, const char*, IrcConnection*);

/*
 * 1. Sender id
 * 2. Action
 * 3. JSON Params
 * 4. IRC Connection Pointer
 */
typedef void* (*module_onInternalMessageWithPtr_f) (const char*, const char*, const char*, IrcConnection*);

#endif /* MODULE_H_ */
