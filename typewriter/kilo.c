#include <unistd.h>
#include <termios.h>               //for the terminal
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>

// *********************************** defines ********************
#define CTRL_KEY(k) ((k) & 0x1f)


// ********************************* data **********************************
struct termios orig_termios;

struct editorConfig {
    int screenrows;
    int screencols;
    struct termios orig_termios;
};

struct editorConfig E;


// Most C library functions that fail will set the global
//  errno variable to indicate what the error was. 
// perror() looks at the global errno variable and prints a descriptive error message for it.
// It also prints the string given to it before it prints the error message

// *********************************** terminal *******************************
void die (char *s)
{
    write(STDOUT_FILENO , "\x1b[2J" , 4);
    write(STDERR_FILENO, "\x1b[H" , 3);
    perror(s);
    exit(1);
}

void disableRawMode() {

    if (tcsetattr(STDIN_FILENO , TCSAFLUSH , &E.orig_termios) == -1)
    die("tcsetattr");
}

// enable the raw mode 
void enableRawMode () {
    if (tcgetattr(STDIN_FILENO , &E.orig_termios) == -1) die("tcsgetattr");
    atexit (disableRawMode);                        //when the program exits then the raw mode will be disabled automatically


    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);                //turning off the echo mode; here we are simply flipping the bits 
                                                    //ICANON will turn off the canonical mode so input will be read byte by byte instead of line by line                
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    
    if (tcsetattr(STDIN_FILENO , TCSAFLUSH , &raw) == -1) die("tcsetattr");    
}

char editorReadKey (){
    int nread;
    char c;
    while ((nread = read(STDERR_FILENO , &c , 1)) != 1){
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    
    return c;
}

int getWindowSize (int *rows , int *cols)   {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO , TIOCGWINSZ , &ws) == -1 || ws.ws_col == 0){
        return -1;
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
    
}

// *****************************input *****************************
void editorProcessKeypress () {
    char c = editorReadKey();
    switch (c){
        case CTRL_KEY('q'):
            write(STDERR_FILENO , "\x1b[2J" , 4);
            write(STDERR_FILENO , "\x1b[H" , 3);
            exit(0);
            break;
    }
}

// ****************************** output ********************************
void editorDrawRows(){
    for (int y = 0 ; y < E.screenrows ; y++)
        write(STDOUT_FILENO , "~\r\n" , 3);
}


void editorRefreshScreen (){
    write (STDOUT_FILENO , "\x1b[1J" , 4);
    write (STDOUT_FILENO ,"\x1b[H" , 3);

    editorDrawRows();
    write(STDOUT_FILENO , "\x1b[H" , 3);              //for repositioning of the cursor
}

// ********************************** init ****************************
void initEditor() {
    if (getWindowSize(&E.screenrows , &E.screencols) == -1) die("get window size");
}

// ********************************** main *********************************

int main() 
{
  enableRawMode();
  initEditor();
  char c;
  
  while (1){
      editorRefreshScreen();
      editorProcessKeypress();
  }
    return 0;

}