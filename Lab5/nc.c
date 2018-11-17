#include <ncurses.h>            
#include <stdio.h>
#include <stdlib.h>                                                
#include <unistd.h>
#include <string.h>
    
void draw_borders(WINDOW *screen) 
{
    int x, y, i; 
    getmaxyx(screen, y, x); 
    // 4 corners 
    mvwprintw(screen, 0, 0, "+"); 
    mvwprintw(screen, y - 1, 0, "+"); 
    mvwprintw(screen, 0, x - 1, "+"); 
    mvwprintw(screen, y - 1, x - 1, "+"); 
    // sides 
    for (i = 1; i < (y - 1); i++) 
    { 
        mvwprintw(screen, i, 0, "|"); 
        mvwprintw(screen, i, x - 1, "|"); 
    } 
    // top and bottom 
    for (i = 1; i < (x - 1); i++) 
    { 
        mvwprintw(screen, 0, i, "-"); 
        mvwprintw(screen, y - 1, i, "-"); 
    } 
}

// #include <ncurses.h> 
// #include <unistd.h> 

// int main(int argc, char *argv[]) 
// {
//     int parent_x, parent_y; 
//     int score_size = 3; 
//     initscr(); 
//     noecho(); 
//     curs_set(FALSE); 
//     // get our maximum window dimensions 
//     getmaxyx(stdscr, parent_y, parent_x); 
//     // set up initial windows 
//     WINDOW *field = newwin(parent_y - score_size, parent_x, 0, 0); 
//     WINDOW *score = newwin(score_size, parent_x, parent_y - score_size, 0); 
//     // draw to our windows 
//     mvwprintw(field, 0, 0, "Field"); 
//     mvwprintw(score, 0, 0, "Score"); 
//     // refresh each window 
//     wrefresh(field); 
//     wrefresh(score); 
//     sleep(5); 
//     // clean up 
//     delwin(field); 
//     delwin(score); 
//     endwin(); 
//     return 0; 
// }

int main(int argc, char *argv[]) 
{

char mesg[]="Enter a string: ";                /* message to be appeared on the screen */ 
 char str[80];                                                                             
 int row,col;                           /* to store the number of rows and *               
                                         * the number of colums of the screen */           
 initscr();                             /* start the curses mode */                        
 getmaxyx(stdscr,row,col);              /* get the number of rows and columns */           
 mvprintw(row/2,(col-strlen(mesg))/2,"%s",mesg);                                           
                                /* print the message at the center of the screen */        
 getstr(str);                                                                              
 mvprintw(LINES - 2, 0, "You Entered: %s", str);                                           
 getch();                                                                                  
 endwin();                                                                                 
                                                                                           
 return 0;  


    int parent_x, parent_y, new_x, new_y;
    int score_size = 3;
      
    initscr(); 
    noecho(); 
    curs_set(FALSE);
    // get our maximum window dimensions 
    getmaxyx(stdscr, parent_y, parent_x); 
    // set up initial windows 
    WINDOW *field = newwin(parent_y - score_size, parent_x, 0, 0); 
    WINDOW *score = newwin(score_size, parent_x, parent_y - score_size, 0); 
    scrollok(field,TRUE);
    draw_borders(field);
    draw_borders(score); 
            wrefresh(field); 
            wrefresh(score); 
sleep(5);
    
       while(0) 
       { 
           getmaxyx(stdscr, new_y, new_x); 
           if (new_y != parent_y || new_x != parent_x) 
           {
                parent_x = new_x; parent_y = new_y; 
                wresize(field, new_y - score_size, new_x);
                wresize(score, score_size, new_x); 
                mvwin(score, new_y - score_size, 0); 
                wclear(stdscr); 
                wclear(field); 
                wclear(score); 
                draw_borders(field); 
                draw_borders(score); 
            }
            // draw to our windows 
            mvwprintw(field, 1, 1, "Field"); 
            mvwprintw(score, 1, 1, "Score"); 
            
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
    wprintw(field, "kappaross\n");
            // refresh each window 
            wrefresh(field); 
            wrefresh(score); 
        } 
    // ... 
    // clean up 
    delwin(field); 
    delwin(score); 
    endwin(); 
    return 0;  
}          