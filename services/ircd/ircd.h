/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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

#ifndef HAVE_IRCD_H
#define HAVE_IRCD_H


#define DEFAULT_CHANNEL "#AVR"
// Take a look @ http://tools.ietf.org/html/rfc2812 for more info about IRC


/* DEFINED IN config.in
#define IRCD_MAX_MESSAGE_SIZE 128
#define IRCD_MESSAGE_BUF_SIZE 8

#define IRCD_MAX_NAME_LEN 20

#define IRCD_PTO_RESET 30 // n second ping timeout
#define IRCD_PTO_TRIGGER 5 // n seconds time to answer
*/

#define INC_MESSAGE_PTR() messagePtr++; if (messagePtr >= IRCD_MESSAGE_BUF_SIZE) messagePtr = 0;
#define RESET_PTO() INFO->pto = IRCD_PTO_RESET


#include "config.h"
#include "../../protocols/uip/uip.h"


/* -----------------------------------*/
 #define CURRENT_ID currentID(uip_conn)
 int currentID(uip_conn_t *curConn);
/* -----------------------------------*/

int16_t
app_sample_onrequest(char *cmd, char *output, uint16_t len);

int16_t
app_sample_init(void);

void ircd_main(void);
static void ircd_handle_input(void);
void ircd_timer();
void performQuit();

// Returns index or -1 if not found
int indexOfn(const char *s, const char c, int skip, int skipDoubles);
// str -> upper case (not included are special chars öäüß)
void toUpper(char *str);
#define indexOf(s,c) indexOfn(s,c,0,0)

char messages[IRCD_MESSAGE_BUF_SIZE][IRCD_MAX_MESSAGE_SIZE];
char messagePtr;

#define MSG_VAR messages[messagePtr]
#define MSG_CLEAR() messages[messagePtr][0] = 0
#define MSG_APPEND(a) strcat(messages[messagePtr], a)
#define MSG_FINISH() strcat(messages[messagePtr], "\n")
#define MSG_WRITE(a...) sprintf(messages[messagePtr], a)
#define MSG_POST(a...) MSG_CLEAR(); sprintf(messages[messagePtr], a); MSG_FINISH();
#define MSG_APPENDPOST(a...) sprintf(messages[messagePtr] + strlen(messages[messagePtr]), a); MSG_FINISH();

#define PASTE_RESET()   (((unsigned char *)uip_appdata)[0] = 0)
#define PASTE(a)      strcat(uip_appdata, a)
#define PASTE_PF(a...)	sprintf(uip_appdata + strlen(uip_appdata), a)
#define PASTE_SEND()    uip_send(uip_appdata, strlen(uip_appdata))

#define TX(a)      strcat(INFO->sysMsg, a)
#define TX_RESET()      INFO->sysMsg[0] = 0

#define INFO (&uip_conn->appstate.ircd)


#ifdef DEBUG_IRCD
# include "core/debug.h"
# define IRCDDEBUG(a...)  debug_printf("ircd: " a)
#else
# define IRCDDEBUG(a...)
#endif

#endif  /* HAVE_IRCD_H */
