#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <locale.h>

static void readfile();
static void inittty(const char *ttyfile, const char *ttyname);
static void endtty();
static int inputloop();

#define STDBUFCOLUMMAX 512
#define STDBUFLINEMAX 256
char **stdbuf = NULL;
int stdbufl = 0;

static void readfile(){
	int buflen = 1012;
  int i = 0;
  stdbuf = calloc(sizeof(char *), buflen);
  while (fgets(stdbuf[i] = malloc(STDBUFLINEMAX), STDBUFLINEMAX, stdin) != NULL) {
    if (buflen <= i)
      stdbuf = realloc(stdbuf, (buflen = 2 * buflen) * sizeof(char *));
    i++;
  }
  stdbufl = i;
}

FILE *oldout;

static void inittty(const char *ttyfile, const char *ttyname){
  setlocale( LC_ALL, "" );
  readfile();
  FILE *termfd = fopen(ttyfile, "r+w");
  if (isatty(1))
    stdout = termfd;
  else
    setbuf(termfd, NULL);
  newterm((char *)ttyname, termfd, termfd);
  raw();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
}

static void endtty(){
  endwin();
}

static int inputloop(){
  int key = 0;
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

    /* case KEY_STAB: */
    /*   goto refresh; */
      
    case '\n': case KEY_IL:
      endtty();
      showlinemax ?
        fputs(stdbuf[realcurline], stdout):
        fputs(selectbuf, stdout);
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
  return 0;
}

int main(int argc, const char **argv){
  inittty("/dev/tty", getenv("TERM"));
  int retid = inputloop();
  endtty();
  return retid;
}
