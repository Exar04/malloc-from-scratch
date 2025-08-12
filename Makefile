CFLAGS = -Wall -Wextra -std=c11 -pedantic  -g
build:
	@echo "Building the project..."
	@cc $(CFLAGS) -o heap main.c 
	@echo "\n"
	@echo "_______________________"
	@echo "\n"
	@./heap

run:
	@./heap

clean:
	@rm heap