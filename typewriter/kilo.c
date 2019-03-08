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
    write(STDOUT_FILENO , "\x1b[2J" , 4);                 //clears the screen
    write(STDERR_FILENO, "\x1b[H" , 3);                   //place the cursor at the top left
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

// reads a single key press and then return the result
char editorReadKey (){
    int nread;
    char c;

    while ((nread = read(STDERR_FILENO , &c , 1)) != 1){   //this will read the character; put number of characters read in nread; while loop will continue until 1 character is read
        if (nread == -1 && errno != EAGAIN) die("read");   //give the error message if failed in reading
    }
    
    return c;
}

//get the position of the cursor
int getCursorPosition (int *rows , int *cols)
{
    char buf[32];
    unsigned int i =0;


    if (write(STDOUT_FILENO , "\x1b[6n" , 4) != 4) return -1;           //this queries the cursor position
                                                                //first we place the cursor at the bottom right 
                                                                // and then ask the cursor position to get the size of the terminal
    printf("\r\n");
    char c;
    //reads the query response until R is encountered and put it in the buffer buf
    while (i < sizeof(buf) - 1){
        if (read(STDIN_FILENO , &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }

    // for printing the buffer
    buf[i] = '\0';
    printf("\r\n&buf[1] : '%s'\r\n" , &buf[1]);           //it starts with buf[1] because 0th character is the escape sequence
    
    //parsing the buffer for the cursor position; returning the error if not able to read properly
    //second line ; parse a string of the form (int;int)
    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2] , "%d;%d" , rows,cols) != 2) return -1;

    return 0;
}

// gets the size of the window and put it in rows and cols
int getWindowSize (int *rows , int *cols)   {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO , TIOCGWINSZ , &ws ) == -1 || ws.ws_col == 0){
        if (write(STDOUT_FILENO , "\x1b[999C\x1b[999B" , 12) != 12) return -1;        //C is for moving right and B is for moving downword
        return getCursorPosition(rows, cols);
    }

    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }    
}

// *****************************input *****************************
//this function map the control sequences to the appropriate function
void editorProcessKeypress () {
    char c = editorReadKey();
    // printf("%c" , c);
    switch (c){
        case CTRL_KEY('q'):
            write(STDERR_FILENO , "\x1b[2J" , 4);              //for clearing the screen
            write(STDERR_FILENO , "\x1b[H" , 3);               //for placing the cursor at the top of the screen
            exit(0);
            break;
    }
}

// ****************************** output ********************************
void editorDrawRows(){
    for (int y = 0 ; y < E.screenrows -1 ; y++)
        write(STDOUT_FILENO , "~\r\n" , 3);
    
    write(STDOUT_FILENO , "~" , 1);
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
      editorRefreshScreen();                //refreshes the screen
      editorProcessKeypress();              //take the key press and process it 
  }
    return 0;

}