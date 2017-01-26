#include <ncurses.h>
#include <stdio.h>


#include "map.h"
#include "log.h"
#include "defer.h"

WINDOW* init_win() {
	WINDOW *win = initscr();
	noecho();
	curs_set(0);
	nodelay(win, true);
	keypad(win, true);
	return win;
}


int main() {
	init_log("rl.log");

	init_win();
	defer(endwin());

	Map map;
	random_map(&map);


	erase();
	map.print();

	while (true) {
		int c = getch();
		if (c == ERR) {
			continue;
		}
		if (c == 'q') {
			break;
		}

		erase();
		if (c == 'r') {
			random_map(&map);
		}
		map.print();
	}

	endwin();

	return 0;
}
