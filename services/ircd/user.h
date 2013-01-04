#ifndef _IRCD_USER_H
#define _IRCD_USER_H _IRCD_USER_H

//#include "ircd_state.h"

#define IRCD_STATE_DISCONNECTED 0
#define IRCD_STATE_CONNECTED 1
#define IRCD_STATE_LOGGEDIN 2
#define IRCD_STATE_QUITTING 3

#define INC_MSG_PTR(a) a++; if (a >= IRCD_MESSAGE_BUF_SIZE) a = 0;
#define DEC_MSG_PTR(a) a--; if (a == 255) a = IRCD_MESSAGE_BUF_SIZE-1;

// Global USER-MODES
#define USER_MODE_GLOBAL_DEFAULT 0

#define USER_MODE_GLOBAL_OP 0


// Has to be called on startup
void user_init(void);

// Has to be called when a new user uses the address (->on connect)
void user_reset(void);

// Sends a message to all connected clients
void user_broadcast(char *id);

// Sends a message to all conencted client in the given channel
void user_broadcast_channel(char* id, char *channelID);

// Sends a message to the specific user
void user_send(char userID, char id);

// Sends a message to the sender himself
void user_send_self(char id);

// Checks whether anyone already uses that nick
// Returns -1 if unused or the ID of the user using it
int nick_used(char *name);
#define user_getID(name) nick_used(name)

// Removes and disconnects the current user
void user_quit();

// User will be disconnected in his next mainloop
void user_prepare_quit(int userID);

// User will leave all channels (w/o notifications)
void user_quitAllChannels(int userID);


#define user_broadcast_remove_self() DEC_MSG_PTR(INFO->msgPtr); INFO->msg[INFO->msgPtr] = 255; IRCDDEBUG ("Removed %s from recipients.\n", INFO->name);

#endif /* _IRCD_USER_H */
