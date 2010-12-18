canything: canything.c
	@gcc -Wall -O3 -o $@ $< -lncurses && echo make canything
clean:
	@rm -f canything && echo clean canything
