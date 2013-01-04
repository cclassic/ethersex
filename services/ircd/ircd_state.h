#ifndef _IRCD_STATE_H
#define _IRCD_STATE_H

#include "ircd.h"

struct ircd_connection_state_t {
    char state;
    char name[IRCD_MAX_NAME_LEN];
    char msg[IRCD_MESSAGE_BUF_SIZE]; //Only references, no real msgs!
    char lastMsg; //used for rexmit
    char sysMsg[64];
    char msgPtr;
    char pto; // Ping timeout counter
    char mode; // Global mode
};

#endif /* _IRCD_STATE_H */
