#include <unistd.h>
#include <termios.h>               //for the terminal
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

struct termios orig_termios;

// Most C library functions that fail will set the global
//  errno variable to indicate what the error was. 
// perror() looks at the global errno variable and prints a descriptive error message for it.
// It also prints the string given to it before it prints the error message
void die (char *s)
{
    perror(s);
    exit(1);
}

void disableRawMode() {

    if (tcsetattr(STDIN_FILENO , TCSAFLUSH , &orig_termios) == -1)
    die("tcsetattr");
}

// enable the raw mode 
void enableRawMode () {
    if (tcgetattr(STDIN_FILENO , &orig_termios) == -1) die("tcsgetattr");
    atexit (disableRawMode);                        //when the program exits then the raw mode will be disabled automatically


    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);                //turning off the echo mode; here we are simply flipping the bits 
                                                    //ICANON will turn off the canonical mode so input will be read byte by byte instead of line by line                
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    
    if (tcsetattr(STDIN_FILENO , TCSAFLUSH , &raw) == -1) die("tcsetattr");    
}


int main() 
{
  enableRawMode();
  char c;
  while (1)
  {
      char c = '\0';
      if (read(STDIN_FILENO , &c , 1) == -1 && errno != EAGAIN ) die("read");
      if (iscntrl(c)) {
          printf("%d\r\n",c);
      }
      else{
          printf("%d ('%c')\r\n" , c,c);
      }
      if (c == 'q') break;
      
  }
    return 0;

}