/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

/* ncurses-like constants LINES and COLS are defined through menuconfig! */

#ifndef TTY_H
#define TTY_H

/* The (one and only) off-screen image. */
extern uint8_t tty_image[LINES][COLS];

/* WINDOW type forward declaration. */
struct _tty_window_t;
typedef struct _tty_window_t WINDOW;

struct _tty_window_t {
  /* Whether this is a sub-window or not.  True for any window but
     tty_mainwin. */
  unsigned subwin		:1;

  /* Whether to wrap the cursor at the end of the line or not. */
  unsigned linewrap		:1;

  /* Current cursor position */
  uint8_t y, x;

  /* Window location and size */
  uint8_t maxy, maxx;
  uint8_t begy, begx;
};


/* The TTY main-window. */
extern WINDOW tty_mainwin;
#define curscr (&tty_mainwin);


/* Printing and cursor movement commands */
void waddch (WINDOW *, const char);
void waddstr (WINDOW *, const char *);
void wmove (WINDOW *, uint8_t y, uint8_t x);
void wprintw (WINDOW *, const char *, ...);

#define addch(ch)		waddch(curscr,ch)
#define addstr(str)		waddstr(curscr,str)
#define move(y,x)		wmove(curscr,y,x)
#define printw(str...)		wprintw(curscr,str)

#define mvaddch(y,x,ch)		do { move(y,x); addch(ch); } while(0)
#define mvaddstr(y,x,str)	do { move(y,x); addstr(str); } while(0)
#define mvprintw(y,x,str...)	do { move(y,x); printw(str); } while(0)

#define mvwaddch(w,ch)		do { wmove(w,y,x); waddch(w,ch); } while(0)
#define mvwaddstr(w,str)	do { wmove(w,y,x); waddstr(w,str); } while(0)
#define mvwprintw(w,y,x,str...)	do { wmove(w,y,x); wprintw(w,str); } while(0)


/* Clearing, etc. */
void wclear (WINDOW *);
void wclrtobot (WINDOW *);
void wclrtoeol (WINDOW *);

#define clear()			wclear(curscr)
#define clrtobot()		wclrtobot(curscr)
#define clrtoeol()		wclrtoeol(curscr)


#endif  /* TTY_H */