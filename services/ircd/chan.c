#include "chan.h"

void ircd_chans_init(void) {
    int x;
    for (x=0; x < IRCD_MAX_CHANS; x++) {
        int y;
        channels[x].name[0] = 0;
        channels[x].mode = IRCD_DEFAULT_MODE;
        for (y=0; y < IRCD_MAX_PPC; y++) {
            channels[x].clients[y] = NOONE;
        }
    }
}

int ircd_chanExists(char *name) {
    if (name[0] == '#') name++; // Remove #
    int x;
    for (x=0; x < IRCD_MAX_CHANS; x++) {
        if (strcmp(channels[x].name, name) == 0) return x;
    }
    return -1;
}

int ircd_join(char *name, char userID) {
    if (name[0] == '#') name++; // Remove #
    if (strlen(name) > IRCD_MAX_CHAN_NAME) return IRCD_ERR_CHAN_NAME_TOO_LONG;
    int temp = ircd_chanExists(name);
    if (temp != -1) {
        if (ircd_isInChannel(temp, userID) == -1) {
            if (ircd_addToChannel(temp, userID) != -1) {
                IRCDDEBUG ("    %s joins %s\n", uip_conns[userID].appstate.ircd.name, name);
                return 0;
            }
            else {
                IRCDDEBUG ("%s can't join %s: Channel full!\n",uip_conns[userID].appstate.ircd.name, name);
                return IRCD_ERR_CHAN_FULL;
            }

        }
        else {
            IRCDDEBUG ("    %s already is in %s!\n", uip_conns[userID].appstate.ircd.name, name);
            return IRCD_ERR_ALREADY_JOINED;
        }
    }
    else { // Try to create channel
        IRCDDEBUG ("Attempting to create channel %s...\n", name);
        temp = ircd_getFirstFreeChannel();
        if (temp != -1) {
            strcpy(channels[temp].name, name);
            strcpy(channels[temp].topic, DEFAULT_TOPIC);
            IRCDDEBUG("    Channel %s created by %s!\n", name, uip_conns[userID].appstate.ircd.name);
            //ircd_join(name); // Recursively join
            user_giveMode (channels[temp].clientMode[ircd_addToChannel(temp, userID)],
                        USER_MODE_OP); // directly use this function becuase no error can happen
            return 0;
        }
        else {
            IRCDDEBUG ("    ERROR: Couldn't create a new channel!");
            return IRCD_ERR_COULDNT_CREATE;
        }
    }
}

int ircd_part(char *name, char userID) {
    if (name[0] == '#') name++; // Remove #
    int chanID = ircd_chanExists(name);
    if (chanID != -1) {
        IRCDDEBUG ("%s left channel #%s\n", uip_conns[userID].appstate.ircd.name, name);
        if (ircd_removeFromChannel(chanID, userID) == 0) { //Success!
            if (ircd_isEmpty(chanID) == -1) {
                ircd_removeChannel(chanID); // Remove chan if no one is left in there
                IRCDDEBUG ("Removed channel #%s\n", name);
            }
            return 0;
        }
    }
    else {
        IRCDDEBUG ("PART: Channel did not exist!\n");
        return IRCD_ERR_CHAN_DOESNT_EXIST;
    }
}

void ircd_removeChannel(char channelID) {
    channels[channelID].name[0] = 0;
}

int ircd_isEmpty(char chanID) {
    int x;
    for (x=0; x < IRCD_MAX_PPC; x++) {
        if (channels[chanID].clients[x] != NOONE) return x;
    }
    return -1;
}

int ircd_getNames(char channelID, char *output) {
    if (channels[channelID].name[0] == 0) return -1;
    int x;
    int ppl = 0;
    for (x = 0; x < IRCD_MAX_PPC; x++) {
        if (channels[channelID].clients[x] != NOONE) {
            if (user_hasMode(channels[channelID].clientMode[x], USER_MODE_OP)) strcat(output, "@");
            strcat(output, uip_conns[channels[channelID].clients[x]].appstate.ircd.name);
            strcat(output, " ");
            ppl++;
        }
    }
    return ppl;
}

int ircd_getAmount(char channelID) {
    if (channels[channelID].name[0] == 0) return -1;
    int x;
    int ppl = 0;
    for (x = 0; x < IRCD_MAX_PPC; x++) {
        if (channels[channelID].clients[x] != NOONE) ppl++;
    }
    return ppl;
}

int ircd_addToChannel(char chanID, char userID) {
    int x;
    for (x=0; x < IRCD_MAX_PPC; x++) {
        if (channels[chanID].clients[x] == NOONE) {
            channels[chanID].clients[x] = userID;
            channels[chanID].clientMode[x] = USER_DEFAULT_MODE;
            return x;
        }
    }
    return -1;
}

int ircd_removeFromChannel(char chanID, char userID) {
    int x;
    for (x=0; x < IRCD_MAX_PPC; x++) {
        if (channels[chanID].clients[x] == userID) {
            channels[chanID].clients[x] = NOONE;
            return 0;
        }
    }
    return 1;
}

int ircd_getFirstFreeChannel() {
    int x;
    for (x=0; x < IRCD_MAX_PPC; x++) {
        if (channels[x].name[0] == 0) return x;
    }
    return -1;
}

int ircd_isInChannel(char channelID, char *userID) {
    int x;
    for (x=0; x < IRCD_MAX_PPC; x++) {
        if (channels[channelID].clients[x] == userID) return x;
    }
    return -1;
}

int ircd_getChannels(char *format, char *output) {
    int channelID;
    int chans = 0;
    for (channelID = 0; channelID < IRCD_MAX_CHANS; channelID++) {
        if (channels[channelID].name[0] != 0) {
            sprintf(output + strlen(output), format, INFO->name, channels[channelID].name, ircd_getAmount(channelID),channels[channelID].topic);
            chans++;
        }
    }
    return chans;
}
