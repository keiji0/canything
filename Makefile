canything: canything.c
	@gcc -Wall -O3 -o $@ $< -lncursesw && echo make canything
clean:
	@rm -f canything && echo clean canything
test: canything
	@/bin/ls -Fa /dev | ./$<
