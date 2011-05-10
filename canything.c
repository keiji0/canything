#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include <unistd.h>
#include <locale.h>
#include <sys/ioctl.h>

#define STDBUFCOLUMMAX 512
#define STDBUFLINEMAX 256
#define QUERYANDMAX 10
#define QUERYITEMSIZEMAX 50

static void initoption(int argc, const char **argv);
static void readfile();
static void inittty(const char *ttyfile, const char *ttyname);
static void endtty();
static void strtolower(const char *src, char *dst);
static char *istrstr(const char *src, const char *dst);
static int inputloop();

char **stdbuf = NULL;
int stdbufl = 0;

struct Option {
  int ignorecase;
} option = {
  /* ignorecase */ 0,
};

static void initoption(int i, const char **argv){
  while (--i) {
    if ('-' == *argv[i]) {
      if (strcmp("-i", argv[i]) == 0) {
        option.ignorecase = 1;
      }
    }
  }
}

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
  setlocale(LC_ALL, "");
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

static void strtolower(const char *src, char *dst){
  char c;
  while ((c = *src++))
    *dst++ = tolower(c);
  *dst++ = '\0';
}

static char *istrstr(const char *src, const char *dst){
  static char srcbuf[STDBUFCOLUMMAX];
  static char dstbuf[STDBUFCOLUMMAX];
  strtolower(src, srcbuf);
  strtolower(dst, dstbuf);
  char *res = strstr(srcbuf, dstbuf);
  return (char *)(res ? src + (res - srcbuf) : NULL);
}

static void printmatchline(const char *line, int selected, const char **highlights){
  attr_t attr = selected ? A_STANDOUT : A_NORMAL;
  attron(attr);
  {
    addstr(line);
    int x, y;
    getyx(stdscr, y, x);
    const char *cur, *pos;
    while ((cur = *highlights++)) {
      pos = istrstr(line, cur);
      move(y - 1, pos - line);
      chgat(strlen(cur), A_UNDERLINE | attr, 0, NULL);
    }
    move(y, x);
  }
  attroff(attr);
}

static int inputloop(){
  int key = 0;
  int here = 0;
  int curline = 0;
  int showlinemax = 0;
  int realcurline = 0;

  char query[QUERYANDMAX][QUERYITEMSIZEMAX];
  int queryindex = 0;
  
  goto refresh;

  while ((key = getch())) {
    switch (key) {
    case KEY_BREAK: case '': case '':
      return 1;
      
    case KEY_BACKSPACE: case '':
      if (here > 0) {
        here--;
        query[queryindex][here] = '\0';
      } else if (queryindex > 0) {
        here = strlen(query[--queryindex]);
      }
      curline = 0;
      goto refresh;
      
    case KEY_DOWN: case '':
      if (showlinemax-1 > curline) curline++;
      goto refresh;
      
    case KEY_UP: case '':
      if (curline > 0) curline--;
      goto refresh;

    case '': case '':
      here = 0;
      queryindex = 0;
      query[queryindex][here] = '\0';
      goto refresh;

    /* case KEY_STAB: */
    /*   goto refresh; */
      
    case '\n': case KEY_IL:
      endtty();
      if (showlinemax)
        fputs(stdbuf[realcurline], stdout);
      else {
        int i;
        for (i = 0; i < queryindex; i++)
          fputs(query[i], stdout);
      }
      exit(0);
      
    case ' ':
      here = 0;
      queryindex++;
      query[queryindex][here] = '\0';
      goto refresh;

    default:
      query[queryindex][here++] = key;
      query[queryindex][here] = '\0';
      goto refresh;
      
    refresh:
      query[queryindex][here] = '\0';
      query[queryindex+1][0] = '\0';
      /* Get window size */
      int winy = getmaxy(stdscr);
      erase();
      move(1, 0);
      {
        int i = 0;
        int cl = 0;
        while (i < stdbufl) {
          int qi, matchflag = 1;
          for (qi = 0; qi <= queryindex; qi++) {
            if (!(option.ignorecase ? istrstr : strstr)(stdbuf[i], query[qi]))
              matchflag = 0;
          }
          if (matchflag) {
            int selected = curline == cl;
            if (winy < (cl + 2)) break;
            if (selected) realcurline = i;
            {
              const char *highlights[QUERYITEMSIZEMAX];
              int qii;
              for (qii = 0; qii <= queryindex; qii++)
                highlights[qii] = query[qii];
              highlights[qii] = NULL;
              printmatchline(stdbuf[i], selected, highlights);
            }
            cl++;
          }
          i++;
        }
        showlinemax = cl;
      }
      int pl = 0;
      move(0, 0);
      addstr(": ");
      pl += 1;
      int qi;
      for (qi = 0; qi <= queryindex; qi++) {
        addstr(query[qi]);
        pl += strlen(query[qi]);
        addstr(" ");
        pl += 1;
      }
      addstr("\n");
      move(0, pl);
      refresh();
    }
  }
  return 0;
}

int main(int argc, const char **argv){
  initoption(argc, argv);
  readfile();
  inittty("/dev/tty", getenv("TERM"));
  int retid = inputloop();
  endtty();
  return retid;
}
