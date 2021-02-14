#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
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

/* TODO: Change these to an enum */
#define UP 		0
#define LEFT 	1
#define DOWN 	2
#define RIGHT 	3

#define ERR_CHAR "\u2717"

#define NORM_VLINE    "\u2502"
#define NORM_HLINE    "\u2500"
#define NORM_ULCORNER "\u250c"
#define NORM_URCORNER "\u2510"
#define NORM_LLCORNER "\u2514"
#define NORM_LRCORNER "\u2518"

#define BOLD_VLINE    "\u2503"
#define BOLD_HLINE    "\u2501"
#define BOLD_ULCORNER "\u250f"
#define BOLD_URCORNER "\u2513"
#define BOLD_LLCORNER "\u2517"
#define BOLD_LRCORNER "\u251b"

#define DOUBLE_VLINE    "\u2551"
#define DOUBLE_HLINE    "\u2550"
#define DOUBLE_ULCORNER "\u2554"
#define DOUBLE_URCORNER "\u2557"
#define DOUBLE_LLCORNER "\u255a"
#define DOUBLE_LRCORNER "\u255d"

/* percent change a pipe has to change direction */
#define TURN_CHANCE 10

/* number of pipes to spawn before clearing the screen */
#define MAX_PIPES 60

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

enum pipe_attr {
    NORMAL,
    BOLD,
    DOUBLE
};

static enum pipe_attr style = NORMAL;

void print_help() {
    printf("Usage: pipes [OPTIONS]\n");
    printf("  -h --help\t\tPrint this message and exit.\n");
    printf("  -b --bold\t\tUse bold box drawing characters.\n");
    printf("  -d --double\t\tUse double box drawing characters.\n");
}

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
char *char_from_dirs(int curr, int prev) {
    char *hline, *vline, *ulcorner, *urcorner, *llcorner, *lrcorner;
    switch (style)
    {
        case NORMAL:
            hline    = NORM_HLINE;
            vline    = NORM_VLINE;
            ulcorner = NORM_ULCORNER;
            urcorner = NORM_URCORNER;
            llcorner = NORM_LLCORNER;
            lrcorner = NORM_LRCORNER;
            break;
        case BOLD:
            hline    = BOLD_HLINE;
            vline    = BOLD_VLINE;
            ulcorner = BOLD_ULCORNER;
            urcorner = BOLD_URCORNER;
            llcorner = BOLD_LLCORNER;
            lrcorner = BOLD_LRCORNER;
            break;
        case DOUBLE:
            hline    = DOUBLE_HLINE;
            vline    = DOUBLE_VLINE;
            ulcorner = DOUBLE_ULCORNER;
            urcorner = DOUBLE_URCORNER;
            llcorner = DOUBLE_LLCORNER;
            lrcorner = DOUBLE_LRCORNER;
            break;
        default:
            hline    = NORM_HLINE;
            vline    = NORM_VLINE;
            ulcorner = NORM_ULCORNER;
            urcorner = NORM_URCORNER;
            llcorner = NORM_LLCORNER;
            lrcorner = NORM_LRCORNER;
            break;
    }

	if ((prev == LEFT && curr == LEFT) || (prev == RIGHT && curr == RIGHT)) {
        return hline;
	} else if ((prev == UP && curr == UP) || (prev == DOWN && curr == DOWN)) {
		return vline;
	} else if ((prev == UP && curr == RIGHT) || (prev == LEFT && curr == DOWN)) {
		return ulcorner;
	} else if ((prev == UP && curr == LEFT) || (prev == RIGHT && curr == DOWN)) {
		return urcorner;
	} else if ((prev == DOWN && curr == RIGHT) || (prev == LEFT && curr == UP)) {
		return llcorner;
	} else if ((prev == DOWN && curr == LEFT) || (prev == RIGHT && curr == UP)) {
		return lrcorner;
	} else {
		return ERR_CHAR;
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

	char *to_print = char_from_dirs(p -> curr.dir, p -> prev.dir);
	mvaddstr(p -> curr.y, p -> curr.x, to_print);

	p -> prev = p -> curr;
	p -> curr = move_position(p -> curr);

	if (rand() % 100 < TURN_CHANCE) {
		p -> curr.dir = random_direction(p -> curr.dir);
	}
}

void main_loop() {
	pipe* p = init_pipe();
	int pipe_count = 1;
	while (true) {

		char c = getch();
		if (c == 'q') {
			break;
		}

		if (pipe_count > MAX_PIPES) {
			erase();
			pipe_count = 0;
		}

		if (p == NULL){
			p = init_pipe();
			pipe_count++;
		}

		update_pipe(p);
		if (out_of_bounds(p -> curr.y, p -> curr.x)) {
			attroff(COLOR_PAIR(p -> color));
			p = NULL;
		}

		napms(1000/FRAME_RATE);
	}
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0
            || strcmp(argv[i], "--help") == 0) {
            print_help();
            exit(0);
        } else if (strcmp(argv[i], "-b") == 0
            || strcmp(argv[i], "--bold") == 0) {
            style = BOLD;
        } else if (strcmp(argv[i], "-d") == 0
            || strcmp(argv[i], "--double") == 0) {
            style = DOUBLE;
        } else {
            printf("Invalid argument (%s)\n", argv[i]);
            exit(1);
        }
    }
    setlocale(LC_ALL, "");
	init_rand();
	start_ncurses();
	start_curs_color();
	main_loop();
	stop_ncurses();
}
