#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <time.h>

#define FRAME_RATE 60

#define RED 	1
#define GREEN 	2
#define YELLOW 	3
#define BLUE 	4
#define MAGENTA	5
#define CYAN 	6
#define WHITE	7

#define UP 		0
#define LEFT 	1
#define DOWN 	2
#define RIGHT 	3

/* percent change a pipe has to change direction */
#define TURN_CHANCE 5

typedef struct {
	int y;
	int x;
	int dir;
} vector;

typedef struct {
	vector curr;
	vector prev;
	int color;
} pipe;

/* start, run, and end program */

void start_ncurses() {
  	initscr();
  	cbreak();
  	noecho();
	nodelay(stdscr, true);
	curs_set(false);
	attron(A_ALTCHARSET);
}

void stop_ncurses() {
	attroff(A_ALTCHARSET);
	curs_set(true);
	nodelay(stdscr, false);
  	echo();
  	nocbreak();
  	endwin();
}

/* enable color support and init color pairs */
void start_curs_color() {
	if (!has_colors()) {
		stop_ncurses();
		exit(1);
	}
	start_color();
	use_default_colors();
	/*		  name		fg				 bg */
	init_pair(RED, 		COLOR_RED, 		-1);
	init_pair(GREEN, 	COLOR_GREEN, 	-1);
	init_pair(YELLOW, 	COLOR_YELLOW, 	-1);
	init_pair(BLUE, 	COLOR_BLUE, 	-1);
	init_pair(MAGENTA, 	COLOR_MAGENTA, 	-1);
	init_pair(CYAN, 	COLOR_CYAN, 	-1);
	init_pair(WHITE, 	COLOR_WHITE, 	-1);
}

/* generate a random integer */
void init_rand() {
	srand((unsigned)time(NULL));
}

/* chooses a random color pair */
int random_color() {
	int r = (rand() % 7) + 1; 
	return r;
}

/* if the location outside the terminal screen */
bool out_of_bounds(int y, int x) {
	if (x < 0 || x > COLS || y < 0 || y > LINES) {
		return true;
	} else {
		return false;
	}
}

/* returns random start position for pipe */
vector random_start() {
	int y;
	int x;
	int dir;

	int r = rand() % 4;
	switch (r) {
		case 0:
			y = 0;
			x = rand() % (COLS + 1);
			dir = DOWN;
			break;
		case 1:
			y = rand() % (LINES + 1);
			x = 0;
			dir = RIGHT;
			break;
		case 2:
			y = LINES;
			x = rand() % (COLS + 1);
			dir = UP;
			break;
		case 3:
			y = rand() % (LINES + 1);
			x = COLS;
			dir = LEFT;
			break;
	}
	return (vector){y,x,dir};
}

/* returns a random direction adjacent to dir */
int random_direction(int dir) {
	if (rand() % 2 == 1) {
		return abs((dir - 1) % 4);
	} else {
		return abs((dir + 1) % 4);
	}
}

/* get the char to print next from current and previous directions */
char char_from_dirs(int curr, int prev) {
	if ((prev == LEFT && curr == LEFT) || (prev == RIGHT && curr == RIGHT)) {
		return ACS_HLINE;
	} else if ((prev == UP && curr == UP) || (prev == DOWN && curr == DOWN)) {
		return ACS_VLINE;
	} else if ((prev == UP && curr == RIGHT) || (prev == LEFT && curr == DOWN)) {
		return ACS_ULCORNER;
	} else if ((prev == UP && curr == LEFT) || (prev == RIGHT && curr == DOWN)) {
		return ACS_URCORNER;
	} else if ((prev == DOWN && curr == RIGHT) || (prev == LEFT && curr == UP)) {
		return ACS_LLCORNER;
	} else if ((prev == DOWN && curr == LEFT) || (prev == RIGHT && curr == UP)) {
		return ACS_LRCORNER;
	} else {
		return ACS_DIAMOND;
	}
}


/* get the next cursor location from position and direction */
vector move_position(vector vec) {
	switch(vec.dir) {
		case UP:
			vec.y--;
			break;
		case LEFT:
			vec.x--;
			break;
		case DOWN:
			vec.y++;
			break;
		case RIGHT:
			vec.x++;
			break;
	}
	return vec;
}

/* create new pipe object */
pipe* init_pipe() {
	pipe* p = (pipe*)malloc(sizeof(pipe));
	p -> curr = random_start();
	p -> prev = p -> curr;
	p -> color = random_color();
	attron(COLOR_PAIR(p -> color));
	return p;
}

/* move the curs to new pos and print pipe char */
void update_pipe(pipe* p) {

	char to_print = char_from_dirs(p -> curr.dir, p -> prev.dir);
	mvaddch(p -> curr.y, p -> curr.x, to_print);

	p -> prev = p -> curr;
	p -> curr = move_position(p -> curr);

	if (rand() % 100 < TURN_CHANCE) {
		p -> curr.dir = random_direction(p -> curr.dir);
	}
}

void main_loop() {
	pipe* p = init_pipe();
	while (true) {
		char c = getch();
		if (c != -1) break;
		if (p == NULL) p = init_pipe();
		update_pipe(p);
		if (out_of_bounds(p -> curr.y, p -> curr.x)) {
			attroff(COLOR_PAIR(p -> color));
			p = NULL;
		}
		napms(1000/FRAME_RATE);
	}
}

int main(void) {
	init_rand();
	start_ncurses();
	start_curs_color();
	main_loop();
	stop_ncurses();
}
