#include "user.h"
#include "chan.h"

#include "ircd.h"

void user_init(void) {
    int x;
    for (x=0; x < UIP_CONNS; x++) {
        uip_conns[x].appstate.ircd.state = IRCD_STATE_DISCONNECTED;
    }
}

void user_reset(void) {
    int x;
    for (x=0; x < IRCD_MESSAGE_BUF_SIZE; x++){
        INFO->msg[x] = 255;
    }
    INFO->name[0] = 0;
    INFO->msgPtr = 0;
    INFO->sysMsg[0] = 0;
    INFO->pto = IRCD_PTO_RESET;
    INFO->mode = USER_MODE_GLOBAL_DEFAULT;
}

void user_broadcast(char* id) {
    int userID;
    IRCDDEBUG("Broadcast to all connected clients. [%d]\n", id);
    for (userID=0; userID < UIP_CONNS; userID++) { // We loop through all connected ppl.
        if (uip_conns[userID].appstate.ircd.state == IRCD_STATE_LOGGEDIN) {
            uip_conns[userID].appstate.ircd.msg[uip_conns[userID].appstate.ircd.msgPtr] = id;
            INC_MSG_PTR(uip_conns[userID].appstate.ircd.msgPtr);
        }
    }
}

void user_broadcast_channel(char* id, char *channelID) {
    int userID;
    IRCDDEBUG("Broadcast to all connected clients in channel %d. [%d]\n", channelID, id);
    for (userID=0; userID < UIP_CONNS; userID++) { // We loop through all connected ppl.
        if (uip_conns[userID].appstate.ircd.state == IRCD_STATE_LOGGEDIN) {
            if (ircd_isInChannel(channelID, userID) != -1) {
                IRCDDEBUG("Sending to %d(%s) @ addr %d\n", userID, uip_conns[userID].appstate.ircd.name, uip_conns[userID].appstate.ircd.msgPtr);
                uip_conns[userID].appstate.ircd.msg[uip_conns[userID].appstate.ircd.msgPtr] = id;
                INC_MSG_PTR(uip_conns[userID].appstate.ircd.msgPtr);
            }
        }
    }
}

void user_send(char userID, char id) {
    IRCDDEBUG("Sending message to %d:%d\n", userID, id);
    uip_conns[userID].appstate.ircd.msg[uip_conns[userID].appstate.ircd.msgPtr] = id;
    INC_MSG_PTR(uip_conns[userID].appstate.ircd.msgPtr);
}

void user_send_self(char id) {
    IRCDDEBUG("Sending message to self:%d\n", id);
    INFO->msg[INFO->msgPtr] = id;
    INC_MSG_PTR(INFO->msgPtr);
}

int nick_used(char *name) {
    int userID;
    for (userID=0; userID < UIP_CONNS; userID++) { // We loop through all connected ppl.
        if (strcmp(uip_conns[userID].appstate.ircd.name, name) == 0) {
            return userID;
        }
    }
    return -1;
}

void user_prepare_quit(int userID) {
    uip_conns[userID].appstate.ircd.state = IRCD_STATE_QUITTING;
}

void user_quitAllChannels(int userID) {
    int x;
    int channelID;
    for (channelID=0; channelID < IRCD_MAX_CHANS; channelID++) {
        for (x=0; x < IRCD_MAX_PPC; x++) {
            if (channels[channelID].clients[x] == userID) {
                channels[channelID].clients[x] = NOONE;
            }
        }
    }
}

void user_quit() {
    // Part all channels
    user_quitAllChannels(CURRENT_ID);

    // Kill user
    user_reset();
    INFO->state = IRCD_STATE_DISCONNECTED;

    uip_close();
}
