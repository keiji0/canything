canything: canything.c
	@gcc -o $@ $< -lncurses && echo make canything
clean:
	@rm -f canything && echo clean canything
