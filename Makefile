ifeq ($(OS),Windows_NT)
  LIBS=-lpdcurses -lkernel32
else ifeq ($(shell uname -s),Darwin)
  LIBS=-lncurses
else
  LIBS=-lncursesw
endif

canything: canything.c
	@gcc -Wall -O3 -o $@ $< ${LIBS} && echo make canything
clean:
	@rm -f canything && echo clean canything
test: canything
	@/bin/ls -Fa /dev | ./$<
