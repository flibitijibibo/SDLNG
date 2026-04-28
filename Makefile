all:
	$(CC) $(CFLAGS) -g -o sdlng sdlng.c -lSDL3 -lSDL3_ttf
clean:
	rm -f sdlng
