#ifndef _IRCD_CHAN_H
#define _IRCD_CHAN_H _IRCD_CHAN_H

#include "ircd.h"
#define IRCD_MAX_PPC UIP_CONNS // Max people per channel

/* DEFINED IN config.in
#define IRCD_MAX_CHAN_NAME 20
#define IRCD_MAX_CHAN_TOPIC 40

#define IRCD_MAX_CHANS 4 // Max number of simultaneous channels.

*/


#define DEFAULT_TOPIC "Everything"
// MODE Macros

// Returns 1 if the user has mdID in chan
#define ircd_isInMode(chan, mdID) (chan & (1 << mdID))
#define isOP(chID) (ircd_isInMode(channels[chID].clientMode[ircd_isInChannel(chID,CURRENT_ID)], USER_MODE_OP) || ircd_isInMode(INFO->mode, USER_MODE_GLOBAL_OP))

#define NOONE 255


// ERROR CODES
// join
#define IRCD_ERR_CHAN_FULL 1
#define IRCD_ERR_ALREADY_JOINED 2
#define IRCD_ERR_COULDNT_CREATE 3
#define IRCD_ERR_CHAN_NAME_TOO_LONG 4

// part
#define IRCD_ERR_CHAN_DOESNT_EXIST 1;

// MODES
#define IRCD_DEFAULT_MODE 1 // N

#define IRCD_MODE_N 0
#define IRCD_MODE_S 1


// USER-MODES
#define user_hasMode(uMD, mdID) (uMD & (1 << mdID))
#define user_giveMode(uMD, mdID) uMD |= (1 << mdID)
#define user_takeMode(uMD, mdID) uMD &= ~(1 << mdID)

#define USER_DEFAULT_MODE 0

#define USER_MODE_AWAY 0 // a
#define USER_MODE_INVISIBLE 1 // i
#define USER_MODE_WALLOP 2 // w
#define USER_MODE_RESTRICTED 3 // r
#define USER_MODE_OP 4 // o
#define USER_MODE____LOCALOP 5 // O REMOVED
#define USER_MODE_SVRNOTICE 6 // s

// TODO: part und join komplett auf IDs umbauen

struct channel_t {
    char name [IRCD_MAX_CHAN_NAME]; // #name
    char clients [IRCD_MAX_PPC]; // Their IDs, not the name!
    char clientMode [IRCD_MAX_PPC]; // For Usermodes see ircd.h
    char topic [IRCD_MAX_CHAN_TOPIC];
    char mode;
};

struct channel_t channels[IRCD_MAX_CHANS];

// Must be called on startup to clear all channels
void ircd_chans_init(void);

// Returns the ID of the channel or -1 if it doesn't exist.
int ircd_chanExists(char *name);

// Joins the channel. If the channel doesn't exist, it gets created.
// Returns: Error code or 0 if everything is all right
int ircd_join (char *name, char userID);

// Parts the channel. If the channel is empty, it gets deleted.
// Returns: Error code or 0 if everything is all right
int ircd_part (char *name, char userID);

// [FOR INTERNAL USE ONLY]
// Removes the use from the given channel
// Returns 0 or an error code
int ircd_removeFromChannel(char chanID, char userID);

// [FOR INTERNAL USE ONLY]
// Adds the user to the channel
// Return the ID inside the channel or -1 if no slot is free
int ircd_addToChannel(char chanID, char userID);

// Gets the people's names in a channel and puts them into *output
// Return the amount of people or -1 if an error occurs.
int ircd_getNames(char channelID, char* output);
int ircd_getAmount(char channelID);

// Removes a channel (Make sure it's empty!)
void ircd_removeChannel(char channelID);

// [FOR INTERNAL USE ONLY] -> Take a look at ircd_chanExists or use EXISTS = (channels[id].name[0] != 0)
// Checks whether any user is left in a channel
// Returns -1 if empty or the first channel-user-ID
int ircd_isEmpty(char chanID);

// Check whether userID is in channelID
// Return ID or -1 if he isn't in the chan
int ircd_isInChannel(char channelID, char *userID);

// Returns the first unused channel or -1 if none is available
int ircd_getFirstFreeChannel();

// Writes the names of the channels into *output
int ircd_getChannels(char *format, char *output);

#endif /* _IRCD_CHAN_H */
