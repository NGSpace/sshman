#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ncurses.h>
#include <pwd.h>
#include <string.h>



#define BLUE_BG 1
#define NUMBERS 2
#define SELECTED 3
#define ADDITIONAL_INFO 4

#define HOR_PADDING 20

#define CAP 64

#define VERSION "0.1.0"



struct Entry {
	char *name;
	char *user;
	char *addr;
};



char *configFolder = "";
int highlightedEntry = 0;
struct Entry launchEntry;
bool shouldLaunch = 0;

void launch_ssh(const char *user, const char *addr) {
	flushinp();              // drop any buffered keystrokes
	endwin();                // restore terminal modes
	execlp("ssh", "ssh", "-l", launchEntry.user, launchEntry.addr, NULL);
	/* exec only returns on error: */
	perror("execlp");
	// parent: just exit so child is reaped by init and keeps the TTY
	exit(0);
}


void drawPickerWindow(int height, int width, int pressedKey, struct Entry *entries, int entryCount) {
	WINDOW *win = newwin(height-2, width-HOR_PADDING*2, 1, HOR_PADDING); // Create a new window
	
	if (pressedKey==KEY_DOWN&&highlightedEntry<entryCount-1) {
		highlightedEntry++;
	}
	if (pressedKey==KEY_UP&&highlightedEntry>0) {
		highlightedEntry--;
	}
	if (pressedKey == '\n' || pressedKey == '\r' || pressedKey == KEY_ENTER) {
		launchEntry = entries[highlightedEntry];
		shouldLaunch = 1;
	}
	
	wbkgd(win, COLOR_PAIR(BLUE_BG) | A_BOLD);
	box(win, 0, 0); // Draw a box with default
	
	// Optionally add a title string
	mvwprintw(win, 0, 2, " SSHMan (SSH Server picker) ");
	
	int yoffset = 2;
	
	if (entryCount==0) {
		wattron(win, COLOR_PAIR(SELECTED) | A_BOLD);
		mvwprintw(win, 2, 3, "No connections in %s", configFolder);
		wattroff(win, COLOR_PAIR(SELECTED) | A_BOLD);
	}
	
	for (int i = 0;i<entryCount;i++) {
		if (highlightedEntry==i) {
			wattron(win, COLOR_PAIR(SELECTED) | A_BOLD);
			mvwprintw(win, yoffset++, 3, "%i. %s", i+1, entries[i].name);
			wattroff(win, COLOR_PAIR(SELECTED) | A_BOLD);
		} else {
			wattron(win, COLOR_PAIR(NUMBERS) | A_BOLD);
			mvwprintw(win, yoffset++, 3, "%i. %s", i+1, entries[i].name);
			wattroff(win, COLOR_PAIR(NUMBERS) | A_BOLD);
		}
		
		wattron(win, COLOR_PAIR(ADDITIONAL_INFO) | A_BOLD);
		mvwprintw(win, yoffset++, 4, "User: %s", entries[i].user);
		wattroff(win, COLOR_PAIR(ADDITIONAL_INFO) | A_BOLD);
		
		wattron(win, COLOR_PAIR(ADDITIONAL_INFO) | A_BOLD);
		mvwprintw(win, yoffset++, 4, "Addr: %s", entries[i].addr);
		wattroff(win, COLOR_PAIR(ADDITIONAL_INFO) | A_BOLD);
	}
	
	wrefresh(win); // Refresh the window to show the box
}

void readConf(struct Entry **entriesPtr, int *entries_indexPtr) {
	struct Entry *entries = 0;
	
	FILE *fptr;
	struct stat st = {0};
	
#if DEBUG
	configFolder = "./sshman.conf";
#else
	if ((configFolder = getenv("HOME")) == NULL) {
		configFolder = getpwuid(getuid())->pw_dir;
	}
	strcat(configFolder, "/.config/sshman.conf");
#endif
	size_t base_len = strlen(configFolder);
	const char *suffix = "/connections";
	size_t total_len = base_len + strlen(suffix) + 1; // +1 for '\0'
	
	char connectionsFile[total_len];
	
	snprintf(connectionsFile, sizeof(connectionsFile), "%s/connections", configFolder);
	
	if (stat(configFolder, &st) == -1) {
		printf("Making sshman config folder in: %s\n", configFolder);
		mkdir(configFolder, 0700);
	}
	if (stat(connectionsFile, &st) == -1) {
		fclose(fopen(connectionsFile, "w")); 
	}
	fptr = fopen(connectionsFile, "r"); 
	
	// Print some text if the file does not exist
	if(fptr == NULL) {
		printf("Not able to open the file.\n");
		exit(0);
	}
	
	int entries_index = 0;
	
	char *namebuf = malloc(CAP);
	char *userbuf = malloc(CAP);
	char *addrbuf = malloc(CAP);
	
	int buf_i = 0;
	int buf_size = CAP;
	int colon_count = 0;
	
	char **currbuf = &namebuf;
	for (;;) {
		int ci = fgetc(fptr);
		
		if (ci != EOF && ci != '\n') {
			if (buf_i+1==buf_size) {
				buf_size *= 2;
				char *tmp = realloc(*currbuf, buf_size);
				*currbuf = tmp;
			}
			if (ci==':') {
				if (colon_count++==0) {
					namebuf[++buf_i] = '\0';
					currbuf = &userbuf;
					buf_i = 0;
					buf_size = CAP;
				} else {
					userbuf[++buf_i] = '\0';
					currbuf = &addrbuf;
					buf_i = 0;
					buf_size = CAP;
				}
			} else (*currbuf)[buf_i++] = (char) ci;
		} else {
			addrbuf[++buf_i] = '\0';
			
			if (namebuf[0]!='\0') {
				struct Entry *tmp = realloc(entries, (entries_index+1)*sizeof(struct Entry));
				entries = tmp;
				entries[entries_index].name = namebuf;
				entries[entries_index].addr = addrbuf;
				entries[entries_index++].user = userbuf;
			}
			
			//Clear shit for next line
			namebuf = malloc(CAP);
			addrbuf = malloc(CAP);
			userbuf = malloc(CAP);
			
			currbuf = &namebuf;
			
			buf_i = 0;
			buf_size = CAP;
			colon_count = 0;
		}
		if (ci == EOF) {
			break;
		}
	}
	
	// Close the file
	fclose(fptr); 
	
	*entriesPtr = entries;
	*entries_indexPtr = entries_index;
}

void drawEditConnectionWindow() {
	// Does nothing rn.
}

int main() {
#if DEBUG
	printf("Starting sshman in debug mode.\n");
#endif
	

	struct Entry *entries = (struct Entry*)malloc(sizeof(struct Entry));
	int entries_index = 0;
	readConf(&entries, &entries_index);
	
	
	initscr();
	if (!has_colors()) {
		endwin();
		fprintf(stderr, "Your terminal does not support color and sshman just can't...\n");
		return 1;
	}
	start_color();
	
	// Define color pairs
	init_pair(BLUE_BG, COLOR_WHITE, COLOR_BLUE);
	init_pair(NUMBERS, COLOR_WHITE, COLOR_BLUE);
	init_pair(SELECTED, COLOR_YELLOW, COLOR_RED);
	init_pair(ADDITIONAL_INFO, COLOR_BLACK, COLOR_BLUE);
	
	curs_set(0);
	refresh();
	keypad(stdscr, TRUE);
	cbreak();
	noecho();
	
	int ch = 0;
	do {
		int height, width;
		getmaxyx(stdscr, height, width);
		clear();
		printw("(q)uit");
		mvprintw(height-1, 0, "SSHMan v%s\n", VERSION);
		refresh();
		if (width<80||height<20) {
			printw("Window too small!\n");
			printw("Width: %i, Height: %i\n", width,height);
		} else {
			drawPickerWindow(height, width, ch, entries, entries_index);
			if (shouldLaunch) break;
			drawEditConnectionWindow();
		}
	} while ((ch = getch()) != 'q');
	
	clear();
	refresh();
	endwin();
	
	if (shouldLaunch)
		launch_ssh(launchEntry.user, launchEntry.addr);
	
	return 0;
}