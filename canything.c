#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

static void readfile();
static void inittty(const char *ttyfile, const char *ttyname);
static void endtty();
static int inputloop();

#define STDBUFCOLUMMAX 255
#define STDBUFLINEMAX 1000
char stdbuf[STDBUFLINEMAX][STDBUFCOLUMMAX];
int stdbufl = 0;

static void readfile(){
  int i = 0;
  while ((fgets(stdbuf[i], STDBUFCOLUMMAX, stdin) != NULL) || STDBUFLINEMAX < i)
    i++;
  stdbufl = i;
}

FILE *oldout;

static void inittty(const char *ttyfile, const char *ttyname){
  readfile();
  FILE *termfd = fopen(ttyfile, "r+w");
  if (isatty(1))
    stdout = termfd;
  else
    setbuf(termfd, NULL);
  newterm(ttyname, termfd, termfd);
  raw();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
}

static void endtty(){
  endwin();
}

static int inputloop(){
  int key;
  int here = 0;
  int curline = 0;
  int showlinemax = 0;
  int realcurline = 0;
  char selectbuf[STDBUFCOLUMMAX];
  
  goto refresh;

  while ((key = getch())) {
    switch (key) {
    case KEY_BREAK: case '': case '': case '':
      return 1;
      
    case KEY_BACKSPACE: case '':
      if (here > 0) here--;
      selectbuf[here] = '\0';
      curline = 0;
      goto refresh;
      
    case KEY_DOWN: case '':
      if (showlinemax-1 > curline) curline++;
      goto refresh;
      
    case KEY_UP: case '':
      if (curline > 0) curline--;
      goto refresh;

    case '':
      here = 0;
      goto refresh;
      
    case '\n': case KEY_IL:
      endtty();
      fwrite(stdout, stdbuf[realcurline], strlen(stdbuf[realcurline])-1, 0, stdout);
      exit(0);
      
    default:
      selectbuf[here++] = key;
      
    refresh:
      selectbuf[here] = '\0';
      {
        int i = 0;
        int cl = 0;
        int selectbuflen = strlen(selectbuf);
        erase();
        move(1, 0);
        char *offset;
        while (i < stdbufl) {
          if ((offset = strstr(stdbuf[i], selectbuf))) {
            if (curline == cl) {
              realcurline = i;
              attron(A_STANDOUT);
            }
            char *a = stdbuf[i];
            addnstr(a, offset - a);
            attron(A_UNDERLINE);
            addnstr(a + (offset - a), selectbuflen);
            attroff(A_UNDERLINE);
            addstr(a + (offset - a) + selectbuflen);
            if (curline == cl)
              attroff(A_STANDOUT);
            cl++;
          }
          i++;
        }
        showlinemax = cl;
      }
      move(0, 0);
      printw(": %s\n", selectbuf);
      move(0, here+2);
      refresh();
    }
  }
}

int main(int argc, const char **argv){
  inittty(argv[1], argv[2]);
  int retid = inputloop();
  endtty();
  return retid;
}
