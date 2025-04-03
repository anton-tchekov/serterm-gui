all:
	clear
	gcc main.c -o serterm-gui -Wall -Wextra -g -lSDL2 -lSDL2_ttf
	./serterm-gui

clean:
	rm -f serterm-gui
