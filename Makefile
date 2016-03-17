ARGS = -Wall -lpthread

all:
	clang -o temperature_server.exe $(ARGS) temperature_server.c
	
