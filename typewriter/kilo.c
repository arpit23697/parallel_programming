#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <termios.h>               //for the terminal
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>
// *********************************** defines ********************
#define CTRL_KEY(k) ((k) & 0x1f)
#define TAB_STOP 8
#define QUIT_TIME 3
#define Arpit_editor_version "0.0.1"


enum editorKey {
    BACKSPACE = 127,
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
    int rsize;
    char *chars;
    char *render;
}erow;



struct editorConfig {
    int cx , cy;             //for the current position of the cursor; cx - horizontal ; cy - vertical on the file
    int rx;                 //current cursor position in the render; will be greater than cx if tabs have extra space
    int screenrows;
    int screencols;
    int numrows;
    int rowoff;              //for the offset to which the typewriters is currently scrolled to 
    int coloff;              //columnoffset for the horizontal scrolling
    erow* row;               //for storing multiple lines
    char *filename;          //for the file opened
    char statusmsg[80];       //for the status msg
    int dirty;               //to check if the text on the file and the editor are different are not 
    time_t statusmsg_time;
    struct termios orig_termios;
};

struct editorConfig E;

// ************************* prototype *************************************
void editorSetStatusMessage (const char * fmt , ...);
void editorRefreshScreen();
char *editorPrompt (char * prompt);

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
//function to convert cx to rx
int editorRowCxToRx (erow *row , int cx){
    int rx = 0;
    for (int j =0 ; j < cx ; j++){
        if (row->chars[j] == '\t')
            rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx;
}

void editorUpdateRow(erow *row){

    //count number of tabs
    //useful in knowing how much memory to give to render
    int tabs = 0;
    for (int j =0 ; j < row->size ; j++)
        if (row->chars[j] == '\t') tabs++;


    free(row->render);
    row->render = malloc(row->size + tabs * (TAB_STOP -1) +  1);      //tab is 8 spaces

    int idx = 0;
    //just copy the character in the render
    for (int j = 0; j < row->size ; j++){
        //if the character is the tab then print the space until idx is divisible by 8
        if (row->chars[j] == '\t'){
            row->render[idx++] = ' ';
            while (idx % TAB_STOP != 0) row->render[idx++] = ' ';
        }
        else
        row->render[idx++] = row->chars[j];
    }

    row->render[idx] = '\0';
    row->rsize =idx;                //setting the size of the render
}

//this function will copy the line from the file into the text editor and append the line onto the editor
void editorInsertRow (int at , char *s, size_t len)
{
    if (at < 0 || at > E.numrows ) return;
    E.row = realloc(E.row , sizeof(erow) * (E.numrows + 1) );
    memmove(&E.row[at + 1] , &E.row[at] , sizeof(erow) * (E.numrows - at));

    //copy the line in the editor row
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize= 0;
    E.row[at].render = NULL;
    editorUpdateRow(&E.row[at]);
    E.numrows++;
    E.dirty++;                //can get the sense of how dirty the file is
}

void editorFreeRow(erow *row){
    free(row->render);
    free(row->chars);
}

//for deleting the row
//at is the current row number  
void editorDelRow (int at){
    if (at < 0 || at >= E.numrows) return;
    editorFreeRow(&E.row[at]);                      //at is referring to the current row
    memmove(&E.row[at] , &E.row[at + 1] , sizeof(erow) * (E.numrows - at - 1) );        //this represents joining of the current row with the rest of the file
    E.numrows--;
    E.dirty++;
}

void editorRowInsertChar (erow *row , int at , int c){
    //at is allowed to go past the end of the string in which case the character should be inserted at the end of the string
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars , row->size + 2);           //+2 is because we have to make room for the null byte

    //safe to use when source and destination overlap
    memmove(&row->chars[at+1] , &row->chars[at] , row->size - at + 1);   //shifting the line by one character
    row->size++;
    row->chars[at] = c;
    editorUpdateRow(row);                 //include tabs and everything
    E.dirty++;
}

//append the string at the end of the row
void editorRowAppendString (erow *row , char *s , size_t len){
    row->chars = realloc(row->chars , row->size + len + 1);
    memcpy (&row->chars[row->size] , s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.dirty++;
}

void editorRowDelChar(erow *row , int at){
    if (at < 0 || at >= row->size) return;        //can't go past the line
    memmove (&row->chars[at] , &row->chars[at + 1] , row->size - at);
    row->size--;
    editorUpdateRow(row);
    E.dirty++;
}

// **************************** editor operations *********************
void editorInsertChar (int c){
    if (E.cy == E.numrows){               //cursor is on the tilda line
        editorInsertRow(E.numrows , "", 0);
    }
    //insert at the cursor position
    editorRowInsertChar(&E.row[E.cy] , E.cx , c);
    E.cx++;              //next character inserted at the next position
}

void editorInsertNewLine (){
    //if at the beginning of the line
    if (E.cx == 0){
        editorInsertRow(E.cy , "" , 0);
    }else{
        erow *row = &E.row[E.cy];
        editorInsertRow(E.cy + 1 , &row->chars[E.cx] , row->size - E.cx);
        row = &E.row[E.cy];                 //editorInsertRow may move the memory around and make the pointer invalid ; so to stop from that
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
    }
    E.cy++;
    E.cx = 0;
}

void editorDelChar (){
    if (E.cy == E.numrows) return;     //if at the last line after the file then return 
    if (E.cx == 0 && E.cy == 0 )return;          //at the beginning of the file
    
    
    erow *row = &E.row[E.cy];
    if (E.cx > 0){
        editorRowDelChar(row , E.cx - 1);
        E.cx--;
    }
    else
    {
        E.cx = E.row[E.cy - 1].size;
        editorRowAppendString(&E.row[E.cy - 1] , row->chars , row->size);
        editorDelRow(E.cy);
        E.cy--;
    }
    
}

// *******************************io file **************************
//gives the string written on the editor 
char *editorRowsToString (int *buflen){

    //goes through each row and append it to the buffer adding a new line character after every line
    int totlen = 0;
    for (int j =0 ; j < E.numrows ; j++)
         totlen += E.row[j].size + 1;
    *buflen = totlen;

    char *buf = malloc(totlen);
    char *p = buf;
    for (int j = 0; j < E.numrows ; j++){
        memcpy (p , E.row[j].chars , E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}



void editorOpen (char *filename){
    free(E.filename);
    E.filename = strdup(filename);         //make a copy of the given string after allocating enough space
    
    FILE *fp = fopen(filename , "r");
    if (!fp) die("fopen");

    char *line = NULL;
    ssize_t linecap = 0;
    ssize_t linelen;
    
    while ((linelen = getline (&line , &linecap , fp)) != -1){
        //decrease linelen until last character is newline or '\r'
        while(linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
        linelen--;
        editorInsertRow(E.numrows , line , linelen);
        
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}

void editorSave() {
    if (E.filename == NULL){
        E.filename = editorPrompt("Save as : %s");
        if (E.filename == NULL){
            editorSetStatusMessage("Save Aborted");
            return;
        }
    }

    int len;
    //will return the string on the editor and give the length as well
    char *buf = editorRowsToString(&len);

    //write into the file after opening and free the buffer
    //O_CREAT      if the file already does not exist create it
    //O_RDWR       open for reading and writing
    //0644         gives the permission if the new file is created
    int fd = open(E.filename , O_RDWR | O_CREAT , 0644);
    if (fd != -1){
        if (ftruncate(fd , len) != -1)
        { //sets the filesize to the specified length ; using ftrucate make it safer so that you not lose much data
            if (write(fd , buf , len) == len){
                close(fd);
                free(buf);
                E.dirty = 0;
                editorSetStatusMessage("%d bytes written to disk" , len);
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editorSetStatusMessage ("Can't save! I/O error: %s", strerror(errno));
}

// *****************************input *****************************
//prompt will be displayed on the status 
char *editorPrompt (char *prompt){
    size_t buffsize = 128;
    char *buf = malloc(buffsize);

    size_t buflen = 0;
    buf[0] = '\0';

    while (1){
        //set the status message
        editorSetStatusMessage(prompt , buf);
        editorRefreshScreen();

        int c = editorReadKey();

        //this part check if the character is ENTER ; if then return buf after clearing the status bar 
        //if not ; check if it is not a control character; if not then add it to the buffer after checking the length of the buffer
        //if the user presses enter ; clear the status bar and send the buffer
        if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE){
            if (buflen!= 0)buf[--buflen] = '\0';
        }
        else if (c == '\x1b'){
            editorSetStatusMessage("");
            free(buf);
            return NULL;
        }
        else if (c == '\r'){
            if (buflen != 0){
                editorSetStatusMessage("");
                return buf;
            }

        } //if not a control character
        else if (!iscntrl(c) && c < 128){
            //if buffer is not sufficient then increase the size of the buffer
            if (buflen == buffsize - 1){
                buffsize *= 2;
                buf = realloc(buf , buffsize);
            }
            buf[buflen++]= c;
            buf[buflen] = '\0';
        }
    }
}



// for moving the cursor using the keys w,a,s,d
void editorMoveCursor(int key)
{
    //if the cursor in the file is beyond the end of the file then assign null otherwise E.row[E.cy];
    erow *row = (E.cy >= E.numrows ) ? NULL : &E.row[E.cy];
    switch(key){
        case ARROW_LEFT:
            if (E.cx != 0)
            E.cx--;
            else if (E.cy > 0){         //if the cursor is at the beginning of the line move to the previous line
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            //allow to move the cursor if the row is not null and the cursor position is less than the size of the row
            if (row && E.cx < row->size)
            E.cx++;
            else if (row && E.cx == row->size){   //if at the end of the line move to the next line
                E.cx = 0;
                E.cy++;
            }
            break;
        case ARROW_UP:
            if (E.cy != 0)
            E.cy--;
            break;
        case ARROW_DOWN:
            if (E.cy < E.numrows)             //do not go past the file //it now represents the position of the cursor on the text file
            E.cy++;
            break;
    }
    //to move the cursor to the end of the line if it went past the end of the line
    //need to take the row again since E.cy can be pointing to someone else
    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];            //if from file assign the row otherwise null
    int rowlen = row ? row->size:0;                             //get len of the row
    if (E.cx > rowlen)                                          //if cursor is beyond rowlen then move it in its limit
        E.cx = rowlen;
}

//this function map the control sequences to the appropriate function
void editorProcessKeypress () {
    static int quit_times = QUIT_TIME;
    int c = editorReadKey();
    // printf("%c" , c);
    switch (c){
        case '\r':
            editorInsertNewLine();
            break;

        case CTRL_KEY('q'):
            if (E.dirty && quit_times > 0){
                editorSetStatusMessage("Warning!!! File has unsaved changes. "
                "Press Ctrl-Q %d more times to quit. ", quit_times);
            quit_times--;
            return;
            }

            write(STDERR_FILENO , "\x1b[2J" , 4);              //for clearing the screen
            write(STDERR_FILENO , "\x1b[H" , 3);               //for placing the cursor at the top of the screen
            exit(0);
            break;

        case CTRL_KEY('s'):
            editorSave();
            break;

        case HOME_KEY:
            E.cx = 0;
            break;

        case END_KEY:
            if (E.cy < E.numrows)
                E.cx = E.row[E.cy].size;
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (c == DEL_KEY) editorMoveCursor(ARROW_RIGHT);
            editorDelChar();
            break;
        
        case PAGE_UP:
        case PAGE_DOWN:
        {
            //place the cursor at the top or the bottom of the terminal and then search the whole page worth of screencols
            if (c == PAGE_UP){
                E.cy = E.rowoff;
            }else if (c == PAGE_DOWN){
                E.cy = E.rowoff + E.screencols - 1;
                if (E.cy > E.numrows) E.cy = E.numrows;
            }

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
        case CTRL_KEY('l'):
        case '\x1b':
            break;


        default:
            editorInsertChar(c);
            break;
    }
    quit_times = QUIT_TIME;
}

// ****************************** output ********************************
void editorScroll(){
    //E.cy give the cursor position
    E.rx = 0;
    //if in the file
    if (E.cy < E.numrows){
        E.rx = editorRowCxToRx(&E.row[E.cy] , E.cx);
    }

    if (E.cy < E.rowoff){           //to check if the cursor is above the visible window
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows){         //to check if the cursor is below the visible window
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if (E.rx < E.coloff){
        E.coloff = E.rx;
    }
    if (E.rx >= E.coloff + E.screencols){
        E.coloff = E.rx - E.screencols + 1;
    }
}



void editorDrawRows( struct abuf *ab){
    //printing the tildas upto last line - 1 and then printing simply the tilda at the last
    for (int y = 0 ; y < E.screenrows ; y++)
    {
        //this is for printing the editor name on the terminal
        int filerow = y + E.rowoff;
        if (filerow >= E.numrows ){

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
            int len = E.row[filerow].rsize - E.coloff;
            if (len < 0) len = 0;
            if (len > E.screencols) len = E.screencols;
            abAppend(ab,  &E.row[filerow].render[E.coloff] , len);
        }
        abAppend(ab , "\x1b[K" , 3);                    //print the tilda and then clear the line to the right of the cursor
                                                       //by defualt it ersases to the right of the cursor
        
        abAppend(ab , "\r\n" , 2);
        
    }
}
//for printing the status bar
void editorDrawStatusBar (struct abuf *ab)
{
    abAppend(ab , "\x1b[7m" , 4);             //inverted colors
    char status[80] , rstatus[80];
    int len = snprintf(status , sizeof(status), "%.20s - %d lines %s" , 
        E.filename ? E.filename : "[No name]" , E.numrows, 
        E.dirty ? "(modified)" : "");      //filename and number of lines in the file ; wether it is modified or not

    int rlen = snprintf(rstatus , sizeof(rstatus) , "%d/%d" , 
        E.cy + 1 , E.numrows);                       //current line number in the file


    if (len > E.screencols) len = E.screencols;           //put only that part of string which can fit in the display area
    abAppend(ab, status, len);
    while (len < E.screencols){
        if (E.screencols - len == rlen){
            abAppend(ab , rstatus , rlen);                     //status to the right of the terminal
            break;
        }
        else{
            abAppend(ab , " " , 1);
            len++;
        }
    }
    abAppend(ab , "\x1b[m" , 3);             //normal mode
    abAppend(ab , "\r\n" , 2);
}

void editorDrawMessageBar (struct abuf *ab){
    abAppend(ab , "\x1b[K" ,3);                        //clear the message bar
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols) msglen = E.screencols;
    if (msglen && time(NULL) - E.statusmsg_time < 5)            //print only if message is 5 seconds old
        abAppend(ab , E.statusmsg , msglen);
}

void editorRefreshScreen (){
    editorScroll();
    // doing using the struct abuf
    struct abuf ab = ABUF_INIT;

    abAppend(&ab , "\x1b[?25l" , 6);                 //for turning of the cursor
    // abAppend (&ab , "\x1b[1J" , 4);                  //instead of clearing the whole screen clear line by line
    abAppend(&ab , "\x1b[H" , 3);                    //put the cursor at the top of the screen
    
    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);           //called at the end of the draw rows to make the status bar
    editorDrawMessageBar(&ab);          //for the message bar

    char buf[32];                                    //for positioning the cursor at the current position
    snprintf(buf , sizeof(buf) , "\x1b[%d;%dH" , (E.cy - E.rowoff) + 1 , (E.rx - E.coloff) + 1);
    abAppend(&ab , buf , strlen(buf));


    abAppend(&ab , "\x1b[?25h" , 6);                //for turning on the cursor

    //writing to the terminal in one go
    write(STDOUT_FILENO , ab.b , ab.len);
    abFree(&ab);
}

//can take any number of arguments
void editorSetStatusMessage (const char *fmt , ...){
    va_list ap;
    va_start (ap , fmt);
    vsnprintf(E.statusmsg , sizeof(E.statusmsg) , fmt , ap);            //for getting each argument
    va_end(ap);
    E.statusmsg_time = time(NULL);            //time since 1 january 1970
}

// ********************************** init ****************************
void initEditor() {
    E.cx = 0;                         //horizontal is x
    E.cy = 0;                         //vertical is y
    E.rx = 0;
    E.numrows = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.row = NULL;
    E.filename = NULL;
    E.statusmsg[0] = '\0';                           //initialised to empty string
    E.statusmsg_time = 0;
    E.dirty = 0;
    if (getWindowSize(&E.screenrows , &E.screencols) == -1) die("get window size");
    E.screenrows -=2;                     //for the status ; decreases the display area ; making the last row as the status bar
}






// ********************************** main *********************************

int main(int argc , char *argv[]) 
{
  enableRawMode();
  initEditor();

  if (argc >= 2){
      editorOpen(argv[1]);
  }

  editorSetStatusMessage ("HELP: Ctrl-S = save | Ctrl-Q = quit");
  while (1){
      editorRefreshScreen();                //refreshes the screen
      editorProcessKeypress();              //take the key press and process it 
  }
    return 0;

}