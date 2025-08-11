build:
	@echo "Building the project..."
	@gcc -o program main.c 

run:
	@./program

clean:
	@rm program