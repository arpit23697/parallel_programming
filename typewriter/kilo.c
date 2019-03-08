#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <termios.h>               //for the terminal
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>

// *********************************** defines ********************
#define CTRL_KEY(k) ((k) & 0x1f)

#define Arpit_editor_version "0.0.1"

enum editorKey {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN

};
// ********************************** append buffer ***************************

//this struct is for creating the dynamic string with only one operation append
struct abuf
{
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}; //empty buffer; acts as a constructor

// this is for appending the string s of lenght len to the buffer ab
void abAppend(struct abuf *ab, const char *s, int len)
{
    char *new = realloc(ab->b, ab->len + len); //this is for dynamically allocated memory

    if (new == NULL)
        return;
    memcpy(&new[ab->len], s, len); //appending the string of len s in new address
    ab->b = new;                   //changing the address and the length
    ab->len += len;
}

// for freeing the buffer
void abFree(struct abuf *ab)
{
    free(ab->b);
}

// ********************************* data **********************************
struct termios orig_termios;

// For storing the row in a editor
typedef struct erow {
    int size;
    char *chars;
}erow;



struct editorConfig {
    int cx , cy;             //for the current position of the cursor; cx - horizontal ; cy - vertical
    int screenrows;
    int screencols;
    int numrows;
    erow* row;               //for storing multiple lines
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
int editorReadKey (){
    int nread;
    char c;

    while ((nread = read(STDERR_FILENO , &c , 1)) != 1){   //this will read the character; put number of characters read in nread; while loop will continue until 1 character is read
        if (nread == -1 && errno != EAGAIN) die("read");   //give the error message if failed in reading
    }
    
    //for reading the escape sequences
    if (c == '\x1b'){
        char seq[3];

        if (read(STDIN_FILENO , &seq[0] , 1) != 1) return '\x1b';
        if (read(STDIN_FILENO , &seq[1] , 1) != 1) return '\x1b';
        
        if (seq[0] == '[')
        {
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                if (read(STDERR_FILENO, &seq[2] , 1) != 1) return '\x1b';
                if (seq[2] == '~'){
                    switch(seq[1]){
                        case '1' : return HOME_KEY;
                        case '3' : return DEL_KEY;
                        case '4' : return END_KEY;
                        case '5' : return PAGE_UP;
                        case '6' : return PAGE_DOWN;
                        case '7' : return HOME_KEY;
                        case '8' : return END_KEY;
                    }
                }
            }
            else{
            switch(seq[1]){
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
                }
            }
        }
        else if (seq[0] == 'O')
        {
            switch (seq[1])
            {
            case 'H': return HOME_KEY;
            case 'F': return END_KEY;
            }
        }
        return '\x1b';
    }
    else
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
    //reads the query response until R is encountered and put it in the buffer buf
    while (i < sizeof(buf) - 1){
        if (read(STDIN_FILENO , &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }

    // appending the buffer with the last '\0' end of string
    buf[i] = '\0';
    
    //parsing the buffer for the cursor position; returning     the error if not able to read properly
    //second line ; parse a string of the form (int;int)
    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2] , "%d;%d" , rows,cols) != 2) return -1;

    return 0;
}

// gets the size of the window and put it in rows and cols
int getWindowSize (int *rows , int *cols)   {
    struct winsize ws;

    // second method is the fallback method; it executes if the first method fails
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
// ******************************* row operations ******************
void editorAppendRow (char *s, size_t len)
{
    //copy the line in the editor row
    E.row = realloc(E.row , sizeof(erow) * (E.numrows + 1));

    int at = E.numrows;
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';
    E.numrows++;
}



// *******************************io file **************************

void editorOpen (char *filename){
    FILE *fp = fopen(filename , "r");
    if (!fp) die("fopen");

    char *line = NULL;
    ssize_t linecap = 0;
    ssize_t linelen;
    
    while ((linelen = getline (&line , &linecap , fp)) != -1){
        //decrease linelen until last character is newline or '\r'
        while(linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
        linelen--;
        editorAppendRow(line , linelen);
        
    }
    free(line);
    fclose(fp);
}

// *****************************input *****************************
// for moving the cursor using the keys w,a,s,d
void editorMoveCursor(int key)
{
    switch(key){
        case ARROW_LEFT:
            if (E.cx != 0)
            E.cx--;
            break;
        case ARROW_RIGHT:
            if (E.cx != E.screencols - 1)
            E.cx++;
            break;
        case ARROW_UP:
            if (E.cy != 0)
            E.cy--;
            break;
        case ARROW_DOWN:
            if (E.cy != E.screenrows - 1)
            E.cy++;
            break;
    }
}

//this function map the control sequences to the appropriate function
void editorProcessKeypress () {
    int c = editorReadKey();
    // printf("%c" , c);
    switch (c){
        case CTRL_KEY('q'):
            write(STDERR_FILENO , "\x1b[2J" , 4);              //for clearing the screen
            write(STDERR_FILENO , "\x1b[H" , 3);               //for placing the cursor at the top of the screen
            exit(0);
            break;

        case HOME_KEY:
            E.cx = 0;
            break;

        case END_KEY:
            E.cx = E.screencols - 1;
            break;
        case PAGE_UP:
        case PAGE_DOWN:
        {
            int times = E.screenrows;
            while (times--){
                editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
        }
        break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;
    }
}

// ****************************** output ********************************
void editorDrawRows( struct abuf *ab){
    //printing the tildas upto last line - 1 and then printing simply the tilda at the last
    for (int y = 0 ; y < E.screenrows ; y++)
    {
        //this is for printing the editor name on the terminal
        if (y >= E.numrows ){

            //display the welcome message only when the file is opened
            if (E.numrows == 0 && y == E.screenrows / 3){
                char welcome[80];
                int welcomelen = snprintf(welcome , sizeof(welcome) , 
                "Arpit Editor -- version %s" , Arpit_editor_version);
                if (welcomelen > E.screencols) welcomelen = E.screencols;

                // for centering the welcome message
                int padding = (E.screencols - welcomelen)/2;
                if (padding){
                    abAppend(ab , "~" , 1);
                    padding--;
                }
                while(padding--)
                    abAppend(ab , " " , 1);
                abAppend(ab, welcome , welcomelen);
            }else {
                abAppend(ab , "~" , 1);
            }
            
        }
        else{
            int len = E.row[y].size;
            if (len > E.screencols) len = E.screencols;
            abAppend(ab,  E.row[y].chars , len);
        }
        abAppend(ab , "\x1b[K" , 3);                    //print the tilda and then clear the line to the right of the cursor
                                                       //by defualt it ersases to the right of the cursor
        if (y < E.screenrows - 1){
            abAppend(ab , "\r\n" , 2);
        }
    }
}


void editorRefreshScreen (){
    // doing using the struct abuf
    struct abuf ab = ABUF_INIT;

    abAppend(&ab , "\x1b[?25l" , 6);                 //for turning of the cursor
    // abAppend (&ab , "\x1b[1J" , 4);                  //instead of clearing the whole screen clear line by line
    abAppend(&ab , "\x1b[H" , 3);                    //put the cursor at the top of the screen
    
    editorDrawRows(&ab);
    
    char buf[32];                                    //for positioning the cursor at the current position
    snprintf(buf , sizeof(buf) , "\x1b[%d;%dH" , E.cy + 1 , E.cx + 1);
    abAppend(&ab , buf , strlen(buf));


    abAppend(&ab , "\x1b[?25h" , 6);                //for turning on the cursor

    //writing to the terminal in one go
    write(STDOUT_FILENO , ab.b , ab.len);
    abFree(&ab);
}

// ********************************** init ****************************
void initEditor() {
    E.cx = 0;                         //horizontal is x
    E.cy = 0;                         //vertical is y
    E.numrows = 0;
    E.row = NULL;
    if (getWindowSize(&E.screenrows , &E.screencols) == -1) die("get window size");
}






// ********************************** main *********************************

int main(int argc , char *argv[]) 
{
  enableRawMode();
  initEditor();

  if (argc >= 2){
      editorOpen(argv[1]);
  }
  char c;
  
  while (1){
      editorRefreshScreen();                //refreshes the screen
      editorProcessKeypress();              //take the key press and process it 
  }
    return 0;

}