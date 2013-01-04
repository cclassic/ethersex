/*
 * Copyright (c) 2009 by C_Classic <CClassicVideos@aol.com>
 * Based upon httpd by Alexander Neumann and Stefan Siegl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

 /*
  TODO: - Check for # when parsin channel names
 */

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ircd.h" 
#include "chan.h"
#include "user.h"

#include "protocols/ecmd/ecmd-base.h"



#define IRC_PARAMS &data[commandPos]
#define IRC_COMMAND data

#define CMD(a) strcmp(IRC_COMMAND, a) == 0

#define REMOVE_NEWLINE() //if (data[strlen(IRC_COMMAND) + indexOf(IRC_PARAMS, 13)] == 13) \    data[strlen(IRC_COMMAND) + indexOf(IRC_PARAMS, 13) + 1] = 0; \    else data[strlen(IRC_COMMAND) + indexOf(IRC_PARAMS, 13)] = 0; //Remove CR as well
#define REDUCE_PARAMS_TO_FIRST() data[strlen(IRC_COMMAND) + indexOf(IRC_PARAMS, 13) + 1] = 0; data[strlen(IRC_COMMAND) + indexOf(IRC_PARAMS, ' ') + 1] = 0;

/* THIS PART IS ONLY NEEDED BECAUSE I HAVEN'T YET FIGURED OUT HOW TO GET THE ID OF THE CURRENT CONNECTION */
int currentID(uip_conn_t *curConn) { // because I didn't find a better way to do this
    int x;
    for (x=0; x < UIP_CONNS; x++) {
        if (&curConn->lport == &uip_conns[x].lport) return x;
    }
    return -1;
}
/* --- snip --- */

void toUpper(char *str) {
    int x;
    for (x=0; x < strlen(str); x++) {
        if (str[x] >= 97 && str[x] <= 122) {
            str[x] -= 32;
        }
    }
}

void ircd_handle_input() {
    char *data = (char *) uip_appdata;
    data[uip_len] = 0; // Terminate the string

    IRCDDEBUG ("ircd: new data (%d bytes) @ conn %d.\n", uip_len, uip_conn);


    // Remove CR & LF & spaces@end
#define CR 13
#define LF 10
#define LASTCHAR data[strlen(data)-1]
    while (LASTCHAR == LF || LASTCHAR == CR || LASTCHAR == ' ') {
        LASTCHAR = 0;
    }

    // Check for too long msgs
    if (uip_len > IRCD_MAX_MESSAGE_SIZE) {
        TX_RESET(); TX (":netio NOTICE * :ERROR: Too long request!\n");
        return;
    }

    // Seperate the command from the params
    int commandPos;
    for (commandPos=0; commandPos < strlen(data); commandPos++) {
        if (data[commandPos] == ' ') {
            data[commandPos] = 0;
            commandPos++;
            break;
        }
    }

    toUpper(data);

    if (CMD("NICK")) { // Nickchange
        REDUCE_PARAMS_TO_FIRST();
        if ((strlen(IRC_PARAMS) <= IRCD_MAX_NAME_LEN && strlen(IRC_PARAMS) >= 2) && !(data[commandPos] == ' ')) {
            if (nick_used(IRC_PARAMS) == -1) { // Nick is unused
                IRCDDEBUG("User changed nick to %s.\n", IRC_PARAMS);

                if (strlen(INFO->name) > 0) { // Only notify the others if the user was alive before
                    MSG_CLEAR();
                    MSG_APPEND (":");
                    MSG_APPEND (INFO->name);
                    MSG_APPEND (" NICK :");
                    MSG_APPEND (IRC_PARAMS);
                    MSG_FINISH();
                    user_broadcast(messagePtr);
                    INC_MESSAGE_PTR();

                    /*
                    // Join def chan
                    ircd_join(DEFAULT_CHANNEL, CURRENT_ID);
                    MSG_POST(":%s JOIN %S", INFO->name, DEFAULT_CHANNEL)
                    user_broadcast_channel(messagePtr, ircd_chanExists(DEFAULT_CHANNEL));
                    INC_MESSAGE_PTR();
*/

                }
                else {
                    // Notify me
                    MSG_POST(":netio 001 %s :Welcome to our IRC server running on an 8 Bit AtMega AVR Microcontroller, %s!", IRC_PARAMS, IRC_PARAMS);
                    MSG_APPENDPOST(":netio 002 %s :Enter /LIST to get a list of all currently available channels.", IRC_PARAMS);
                    MSG_APPENDPOST(":netio 003 %s :Also, feel free to create a new one by simply joining a non existent one.", IRC_PARAMS);
                    MSG_APPENDPOST(":netio 004 %s :Have fun!", IRC_PARAMS);
                    user_send_self(messagePtr);
                    INC_MESSAGE_PTR();
                }

                strcpy (INFO->name, IRC_PARAMS);
                INFO->state = IRCD_STATE_LOGGEDIN;
            }
            else {
                printf ("ircd: WARN: User tried using a already used name!\n");                
                MSG_POST(":netio NOTICE * :Nickname %s is already in use!\n", IRC_PARAMS);
                user_send_self(messagePtr); INC_MESSAGE_PTR();
            }
        }
        else {
            printf ("ircd: WARN: User tried using a too long or too short name!\n");
            TX_RESET(); TX (":netio NOTICE * :Invalid nickname!\n");
        }
    }
    else if (INFO->state == IRCD_STATE_LOGGEDIN) { // This prevents the users from messing around.... at least a bit... okay... forget it
        if (CMD("JOIN")) {
            REDUCE_PARAMS_TO_FIRST();
            int chID;
            if ((ircd_join(IRC_PARAMS, CURRENT_ID) == 0) && ((chID = ircd_chanExists(IRC_PARAMS)) != -1)) {
                MSG_POST(":%s JOIN #%s", INFO->name, channels[chID].name);
                user_broadcast_channel(messagePtr, chID);
                INC_MESSAGE_PTR();

                MSG_POST(":netio 332 %s #%s :%s", INFO->name, channels[chID].name, channels[chID].topic); // Send topic
                user_send_self(messagePtr);
                INC_MESSAGE_PTR();
            }
            else {
                MSG_POST(":netio NOTICE * :ERROR: Couldn't join channel!");
                user_send_self(messagePtr);
                INC_MESSAGE_PTR();
            }
        }
        else if (CMD("PART")) {
            REDUCE_PARAMS_TO_FIRST();
            int chID;
            if ((chID = ircd_chanExists(IRC_PARAMS)) != -1) {
                MSG_POST(":%s PART #%s", INFO->name, channels[chID].name);
                user_broadcast_channel(messagePtr, chID);
                INC_MESSAGE_PTR();

                if (ircd_part(IRC_PARAMS, CURRENT_ID) == 0) {
                    //IRCDDEBUG ("%s parted channel %s.", INFO->name, IRC_PARAMS); Done in the part function itself
                }
                else {
                    IRCDDEBUG ("PART: ERROR: %s part channel %s.", INFO->name, IRC_PARAMS);
                }
            }

        }
        else if (CMD("KICK")) { // KICK #chan user
            REMOVE_NEWLINE();
            data +=commandPos; // Remove command completely (WARN: Breaks the macros)

            int temp  = indexOfn(data,' ',0,1);
            data[temp] = 0;

            if (temp != -1) {
                int chID = ircd_chanExists(data);
                if (chID != -1) {
                    if (isOP(chID)) {
                        int uID = user_getID(&data[temp+1]);
                        if (uID != -1) {
                            MSG_POST(":%s PART %s :Kicked by %s", &data[temp+1], data, INFO->name);
                            user_broadcast_channel(messagePtr, chID);
                            INC_MESSAGE_PTR();
                            ircd_part(data, uID);
                        }
                        else {
                            IRCDDEBUG ("KICK: ERROR: User to kick does not exist!\n");
                        }
                    }
                    else {
                        IRCDDEBUG ("KICK: ERROR: No rights!\n");
                    }
                }
                else {
                    IRCDDEBUG ("KICK: ERROR: Channel does not exist!\n");
                }
            }
            else {
                IRCDDEBUG ("KICK: ERROR: Wrong format!\n");
            }
        }
        else if (CMD("LIST")) {
            //  :calvino.freenode.net 322 C_Classic #zirpu 2 :here. now. this.
            // :calvino.freenode.net 323 C_Classic :End of /LIST
           IRCDDEBUG("Channel list requested by %s.\n", INFO->name);
           MSG_POST(":netio 321 %s Channel :Users  Name", INFO->name);
           ircd_getChannels(":netio 322 %s #%s %d :%s\n", MSG_VAR);
           MSG_APPENDPOST(":netio 323 %s :End of /LIST", INFO->name);
           user_send_self(messagePtr);
           INC_MESSAGE_PTR();
        }
        else if (CMD("TOPIC")) {
            data +=commandPos; // Remove command completely (WARN: Breaks the macros)
            int temp;
            if ((temp  = indexOfn(data,' ',0,1)) != -1) {
                data[temp] = 0;
                int chID;
                if ((chID=ircd_chanExists(data)) != -1) {
                    if (isOP(chID)) {
                        if (strlen(&data[temp+2]) <= IRCD_MAX_CHAN_TOPIC) { // &data[temp+2] weil vor dem eigentlichen topic nochn : ist
                            strcpy(channels[chID].topic, &data[temp+2]);
                            MSG_POST(":netio 332 %s #%s :%s", INFO->name, channels[chID].name, channels[chID].topic); // Send topic
                            user_broadcast_channel(messagePtr, chID);
                            INC_MESSAGE_PTR();
                            IRCDDEBUG("TOPIC: Topic of channel %s was changed to %s by %s", channels[chID].name, channels[chID].topic, INFO->name);
                        }
                        else {
                            IRCDDEBUG("TOPIC: ERROR: Topic too long!", data);
                        }
                    }
                    else {
                        IRCDDEBUG("TOPIC: ERROR: User %s does not have the right to change the topic in #%s!", INFO->name, channels[chID].name);
                    }
                }
                else {
                    IRCDDEBUG("TOPIC: ERROR: Channel %s does not exist!", data);
                }
            }
        }
        else if (CMD("MODE")) {
            if ((indexOf(IRC_PARAMS, '+') == -1) && (indexOf(IRC_PARAMS, '-') == -1)) { // only read
                REDUCE_PARAMS_TO_FIRST();
                if (data[commandPos] == '#') { // Channel
                    int chanID = ircd_chanExists(IRC_PARAMS);
                    if (chanID != -1) {
                        MSG_CLEAR ();
                        MSG_APPEND (":netio 324 ");
                        MSG_APPEND (INFO->name);
                        MSG_APPEND (" ");
                        MSG_APPEND (IRC_PARAMS);
                        MSG_APPEND (" :+");
                        if (ircd_isInMode(channels[chanID].mode, IRCD_MODE_N)) {
                            MSG_APPEND ("n");
                        }
                        if (ircd_isInMode(channels[chanID].mode, IRCD_MODE_S)) {
                            MSG_APPEND ("s");
                        }
                        MSG_FINISH ();
                        user_send_self(messagePtr);
                        INC_MESSAGE_PTR();
                    }
                    else {
                        IRCDDEBUG ("MODE: ERROR: Channel for MODE didn't exist.\n");
                    }
                }
                else { // user read
                    IRCDDEBUG ("MODE: ERROR: MODE for user isn't implemented yet. (rd)\n");
                }

            }
            else { // set mode
                REMOVE_NEWLINE();
                data +=commandPos; // Remove command completely (WARN: Breaks the macros)

                int temp  = indexOfn(data,' ',0,1);
                int temp2 = indexOfn(data,' ',1,1);
                data[temp] = 0;
                data[temp2] = 0;

                if (data[0] == '#') { // Channel
                    if (temp2 != -1) {
                        IRCDDEBUG ("MODE: User %s is trying to change mode of %s in channel %s to %s...\n", INFO->name, &data[temp2+1], data, &data[temp+1]);
                        int chID;
                        if ((chID=ircd_chanExists(data)) != -1) { // Check whether chan exists
                            if (isOP(chID)) { // Check whether he has the rights to issue the command
                                int uID = user_getID(&data[temp2+1]);
                                if (uID != -1) uID = ircd_isInChannel(chID, uID);
                                if (uID != -1) {
                                    int modeID = (data[temp+2] == 'o') ? USER_MODE_OP:
                                                 (data[temp+2] == 'i') ? USER_MODE_INVISIBLE: -1;

                                    if (modeID != -1) {
                                        if (data[temp+1] == '+') user_giveMode(channels[chID].clientMode[uID],modeID);
                                        else if (data[temp+1] == '-') user_takeMode(channels[chID].clientMode[uID],modeID);
                                        else {
                                            IRCDDEBUG ("      ERROR: Invalid operation: %s\n", &data[temp+1]);
                                            return;
                                        }

                                        IRCDDEBUG ("     Change OK. New mode: %d.\n", channels[chID].clientMode[uID]);
                                        MSG_POST(":%s MODE %s %s %s", INFO->name, data, &data[temp+1], &data[temp2+1]);
                                        user_broadcast_channel(messagePtr, chID);
                                        INC_MESSAGE_PTR();

                                    }
                                    else {
                                        IRCDDEBUG ("      ERROR: Invalid mode: %s\n", &data[temp+1]);
                                    }

                                }
                                else {
                                    IRCDDEBUG ("      ERROR: User %s does not exist (in this channel).\n", &data[temp2+1]);
                                }
                            }
                            else {
                                IRCDDEBUG ("      ERROR: %s has no rights to use MODE!\n", INFO->name);
                            }
                        }
                        else {
                            IRCDDEBUG ("      ERROR: Channel does not exist.\n");
                        }

                    }
                    else if (temp != -1) {
                        IRCDDEBUG ("      channel->\"%s\"\n", data);
                        IRCDDEBUG ("      mode->\"%s\"\n", &data[temp+1]);
                        IRCDDEBUG ("      channel only not implemented yet.\n");
                    }
                }
                else {
                    IRCDDEBUG ("MODE: ERROR: MODE for user isn't implemented yet. (set)\n");
                }
            }
        }
        else if (CMD("WHO")) {
            REDUCE_PARAMS_TO_FIRST();
            int chanID = ircd_chanExists(IRC_PARAMS);
            if (chanID != -1) {
                MSG_CLEAR ();
                MSG_APPEND (":netio 353 ");
                MSG_APPEND (INFO->name);
                MSG_APPEND (" @ ");
                MSG_APPEND (IRC_PARAMS);
                MSG_APPEND (" :");
                ircd_getNames(chanID, MSG_VAR);
                MSG_FINISH ();
                    IRCDDEBUG("WHO for %s: %s", IRC_PARAMS, MSG_VAR);

                MSG_APPEND (":netio 366 ");// svr 366 C_Classic #demophobia-dev :End of /NAMES list.
                MSG_APPEND (INFO->name);
                MSG_APPEND (" ");
                MSG_APPEND (IRC_PARAMS);
                MSG_APPEND (" :End of /NAMES list.");
                MSG_FINISH ();

                user_send_self(messagePtr);
                INC_MESSAGE_PTR();
            }
            else {
                IRCDDEBUG ("ERROR: Channel for WHO didn't exist.\n");
            }
        }
        else if (CMD("PRIVMSG")) {
            REMOVE_NEWLINE();
            char pos, cont;
            pos = strlen(IRC_COMMAND) + indexOf(IRC_PARAMS, ' ') + 1;
            cont = data[pos];
            data[pos] = 0;

            IRCDDEBUG ("PRIVMSG: from %s in channel %s\n", INFO->name, IRC_PARAMS);
            int chanID = ircd_chanExists(IRC_PARAMS);

            data[pos] = cont;
            if ((chanID != -1)) {
                if (ircd_isInChannel(chanID, user_getID(INFO->name)) != -1) {
                    MSG_POST (":%s PRIVMSG %s", INFO->name, IRC_PARAMS);
                    user_broadcast_channel(messagePtr, chanID);
                    user_broadcast_remove_self();
                    INC_MESSAGE_PTR();
                }
                else {
                    debug_printf ("ircd: WARN: %s tried to write in not joined channel!\n", INFO->name);
                }
            }
            else {
                debug_printf ("ircd: WARN: %s tried to write in non existent channel!\n", INFO->name);
            }
        }
        else if (CMD("PONG")) {
           // IRCDDEBUG ("Received pong from %s.\n", INFO->name);
        }
        else if (CMD("QUIT")) {
            MSG_CLEAR();
            MSG_APPEND (":");
            MSG_APPEND (INFO->name);
            MSG_APPEND (" QUIT :");
            if (data[commandPos] == ':') commandPos++; // Remove colon
            MSG_APPEND (IRC_PARAMS);
            MSG_FINISH();
            user_broadcast(messagePtr);
            INC_MESSAGE_PTR();
            IRCDDEBUG ("%s quitted.\n", INFO->name);
            user_quit();
        }
        else {
            debug_printf ("ircd: WARN: Unknown command: %s\n", IRC_COMMAND);
        }
        RESET_PTO(); // To reduce the amount of data, we accept each transmission as "PONG"
    }
    else {
        debug_printf ("ircd: WARN: User tried executing a command without being logged in!\n");
    }
}

int16_t
ircd_init(void)
{
  IRCDDEBUG ("init\n");

  messagePtr = 0;

  user_init();
  ircd_chans_init();

  uip_listen(HTONS(IRCD_PORT), ircd_main);

  return ECMD_FINAL_OK;
}

void ircd_main(void) {

    if (uip_aborted() || uip_timedout()) {
        // Cleanup
        IRCDDEBUG ("connection aborted\n");
        performQuit();
        INFO->state = IRCD_STATE_DISCONNECTED;
    }

    if (uip_closed()) {
        // Cleanup
        IRCDDEBUG ("connection closed\n");
        INFO->state = IRCD_STATE_DISCONNECTED;
    }

    if (uip_connected()) {
        IRCDDEBUG ("new connection\n");
        INFO->state = IRCD_STATE_CONNECTED;
        user_reset();
        TX_RESET();
        TX (":netio NOTICE * :Connected. Please choose a name to proceed.\n");
    }

    if (uip_newdata()) {
        ircd_handle_input();
    }

    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_poll() ||
       uip_connected()) {
        // do something


        if (INFO->state == IRCD_STATE_QUITTING) { // FORCE QUIT
           performQuit();
        }

        if (uip_rexmit()) {
            //uip_close ();
            IRCDDEBUG ("REXMIT: Something went wrong with user %s!\n", INFO->name);

            // Retransmit last message (sys messages mihgt cause an error)
            PASTE_RESET ();
            PASTE (messages[INFO->lastMsg]);
            PASTE_SEND ();

            return;
        }

        if (INFO->sysMsg[0] != 0) { // very important stuff :)
            PASTE_RESET ();
            PASTE (INFO->sysMsg);
            PASTE_SEND ();
            INFO->sysMsg[0] = 0;
        }
        else { // Normal, less important stuff
            int x;
            for (x=0; x < IRCD_MESSAGE_BUF_SIZE; x++) {
                if (INFO->msg[x] != 255) {
                    //printf ("\n------------------\nNOW SENDING to \"%s\" [%d]:\"%s\"\n------------------\n", INFO->name,  INFO->msg[x], messages[INFO->msg[x]]);
                    PASTE_RESET ();
                    PASTE (messages[INFO->msg[x]]);
                    PASTE_SEND ();
                    INFO->lastMsg = INFO->msg[x];
                    INFO->msg[x] = 255;
                    break; // All in good time.
                }
            }
        }
    }
}

void performQuit() {
    if (INFO->name[0] != 0) { //If we was never known he doesn't have to say goodbye
        MSG_CLEAR();
        MSG_APPEND (":");
        MSG_APPEND (INFO->name);
        MSG_APPEND (" QUIT :ping timeout");
        MSG_FINISH();
        user_broadcast(messagePtr);
        user_broadcast_remove_self();
        INC_MESSAGE_PTR();
        IRCDDEBUG ("%s force quitted.\n", INFO->name);
    }
    user_quit();
}

int16_t
ircd_onrequest(char *cmd, char *output, uint16_t len){
  IRCDDEBUG ("ecmd request\n");
  return ECMD_FINAL_OK;
}

void ircd_timer() {
    int x;
    for (x=0; x < UIP_CONNS; x++) {
        if (uip_conns[x].appstate.ircd.state != IRCD_STATE_DISCONNECTED) {
            uip_conns[x].appstate.ircd.pto--;
            if (uip_conns[x].appstate.ircd.pto == 0) {
                printf ("ircd: WARN: Ping timeout for user %s.\n", uip_conns[x].appstate.ircd.name);
                user_quitAllChannels(x); // Quit all channels immediately so the server doesn't try to send stuff to him anymore.
                user_prepare_quit(x);
            }
            else if (uip_conns[x].appstate.ircd.pto == IRCD_PTO_TRIGGER) {
                //IRCDDEBUG ("Pinging user %s.\n", uip_conns[x].appstate.ircd.name);
                MSG_CLEAR ();
                MSG_APPEND ("PING :");
                MSG_APPEND (uip_conns[x].appstate.ircd.name);
                MSG_FINISH ();
                user_send(x, messagePtr);
                INC_MESSAGE_PTR();
            }
        }
    }
}

int indexOfn(const char *s, const char c, int skip, int skipDoubles) {// [copied from some forum] -> edited so that the function isn't nice anymore :(
    int sc = 0;
    int pos = 0;
    char lastChar = 255;
    for (const char *p = s; *p != 0; ++p) {
        if (*p == c && (!skipDoubles || *p != lastChar)) {
            if (sc == skip) return pos; //p - s;
            sc++;
        }
        lastChar = *p;
        pos++;
    }
    return -1;
}



/*
  -- Ethersex META --
  header(services/ircd/ircd.h)
  ifdef(`conf_IRCD_INIT_AUTOSTART',`net_init(ircd_init)')
  timer(50,ircd_timer())

  state_header(services/ircd/ircd_state.h)
  state_tcp(struct ircd_connection_state_t ircd)
*/
