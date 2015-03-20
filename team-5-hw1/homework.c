/*
 * file:        homework.c
 * description: Skeleton for homework 1
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Peter Desnoyers, Jan. 2012
 * $Id: homework.c 500 2012-01-15 16:15:23Z pjd $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uprog.h"

/***********************************/
/* Declarations for code in misc.c */
/***********************************/

typedef int *stack_ptr_t;
extern void init_memory(void);
extern void do_switch(stack_ptr_t *location_for_old_sp, stack_ptr_t new_value);
extern stack_ptr_t setup_stack(int *stack, void *func);
extern int get_term_input(char *buf, size_t len);
extern void init_terms(void);

extern void  *proc1;
extern void  *proc1_stack;
extern void  *proc2;
extern void  *proc2_stack;
extern void **vector;

/***********************************************/
/********* Your code starts here ***************/
/***********************************************/

int readfile(char *filename, char *buf, int buflen); 
// char *strwrd(char *s, char *buf, size_t len, char *delim);

/*
 * Question 1.
 *
 * The micro-program q1prog.c has already been written, and uses the
 * 'print' micro-system-call (index 0 in the vector table) to print
 * out "Hello world".
 *
 * You'll need to write the (very simple) print() function below, and
 * then put a pointer to it in vector[0].
 *
 * Then you read the micro-program 'q1prog' into memory starting at
 * address 'proc1', and execute it, printing "Hello world".
 *
 */
void print(char *line)
{
    /* system call */ 
        printf("%s", line); // no "\n"
}

void q1(void)
{   
    /* Set vector table */
        vector[0] = print;

        char *ptr_file = "q1prog";

        if(!(readfile(ptr_file, proc1, 4096)))
        {    
            printf("can't open file\n");
            exit(1);
        }
    
    /* callback function */ 
        void (*foo)(void) = proc1;
        foo();
}


/*
 * Question 2.
 *
 * Add two more functions to the vector table:
 *   void readline(char *buf, int len) - read a line of input into 'buf'
 *   char *getarg(int i) - gets the i'th argument (see below)

 * Write a simple command line which prints a prompt and reads command
 * lines of the form 'cmd arg1 arg2 ...'. For each command line:
 *   - save arg1, arg2, ... in a location where they can be retrieved
 *     by 'getarg'
 *   - load and run the micro-program named by 'cmd'
 *   - if the command is "quit", then exit rather than running anything
 *
 * Note that this should be a general command line, allowing you to
 * execute arbitrary commands that you may not have written yet. You
 * are provided with a command that will work with this - 'q2prog',
 * which is a simple version of the 'grep' command.
 *
 * NOTE - your vector assignments have to mirror the ones in vector.s:
 *   0 = print
 *   1 = readline
 *   2 = getarg
 */

char buf[100];
char *argv[10];

int readfile(char *filename, char *buf, int buflen){
    
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        perror("can't open file");
        return 0;
    }

    int i, c = 0;
    for (i = 0; i < buflen; ++i)
    {
        c = getc(fp);
        if(c == EOF) break;
        buf[i] = c;
    }
    
    fclose(fp);
    return 1;
}


void readline( char *buf, int len) /* vector index = 1 */
{
    int iptLen = 0;
    char iptbuf;
    
    while(iptLen < len)
    {
        iptbuf = getchar(); 
        if(iptbuf == '\n'){  
            buf[iptLen] = 0;
            break; 
        }
        else{
            buf[iptLen] = iptbuf;
            iptLen++;
        }
    }
}

char *getarg(int i)		/* vector index = 2 */
{
    return argv[i];
}

/*
 * Note - see c-programming.pdf for sample code to split a line into
 * separate tokens. 
 */
void q2(void)
{
    /* Set vector table */
    vector[0] = print;
    vector[1] = readline;
    vector[2] = getarg;

    while (1) {
	   
        printf("$:");
        
        /* get a line */
           readline(buf, sizeof(buf));   

        /* split it into words and stores them in argv*/
           char *cmd = strtok(buf, " \t\n");
           // printf("cmd get = %s\n",cmd);

        /* if zero words, continue */
	       if(cmd == NULL) continue;    
   
         /* if first word is "quit", break */
    	   if(!strcmp(cmd, "quit")) break;        

        /* make sure 'getarg' can find the remaining words */
        /* This is make sure that the argv[1] == NULL */   
            int i = 0;
            for (i = 0; i < 10; i++) {
                argv[i] = strtok(NULL, " \t\n");
                // printf("argv[%d] = %s\n", i,argv[i]);
                if(argv[i] == NULL) break;
            }   

        /* load and run the command */
            // printf("READING CMD = %s \n",cmd);
            if(readfile(cmd, proc2, 4096)){   
                void (*foo)(void) = proc2;
                foo();
            }
            else{
                printf("'%s' Command not found !\n",cmd);
            }
    }

    /*
     * Note that you should allow the user to load an arbitrary command,
     * and print an error if you can't find and load the command binary.
     */
}

/*
 * Question 3.
 *
 * Create two processes which switch back and forth.
 *
 * You will need to add another 3 functions to the table:
 *   void yield12(void) - save process 1, switch to process 2
 *   void yield21(void) - save process 2, switch to process 1
 *   void uexit(void) - return to original homework.c stack
 *
 * The code for this question will load 2 micro-programs, q3prog1 and
 * q3prog2, which are provided and merely consists of interleaved
 * calls to yield12() or yield21() and print(), finishing with uexit().
 *
 * Hints:
 * - Use setup_stack() to set up the stack for each process. It returns
 *   a stack pointer value which you can switch to.
 * - you need a global variable for each process to store its context
 *   (i.e. stack pointer)
 * - To start you use do_switch() to switch to the stack pointer for 
 *   process 1
 */

stack_ptr_t main_ptr, proc1_stack_ptr, proc2_stack_ptr; 

void yield12(void)		/* vector index = 3 */
{
    /* Your code here */    
    do_switch(&proc1_stack_ptr, proc2_stack_ptr);
}

void yield21(void)		/* vector index = 4 */
{
    /* Your code here */
    do_switch(&proc2_stack_ptr, proc1_stack_ptr);
}

void uexit(void)		/* vector index = 5 */
{
    /* Your code here */
    do_switch(&proc1_stack_ptr, main_ptr);   
}

void q3(void)
{
    /* Your code here */

    /* Set vector table */
    vector[0] = print;
    vector[1] = readline;
    vector[2] = getarg;
    vector[3] = yield12;
    vector[4] = yield21;
    vector[5] = uexit;

    /* load q3prog1 into process 1 and q3prog2 into process 2 */
    char *file_ptr_q3p1 = "q3prog1";
    char *file_ptr_q3p2 = "q3prog2";

    if(readfile(file_ptr_q3p1, proc1, 4096)){
        void (*foo_p1)(void)= proc1;
        proc1_stack_ptr = setup_stack(proc1_stack, foo_p1);
    }
    else{
        printf("proc1 stack setup failed !\n");
    } 

    if(readfile(file_ptr_q3p2, proc2, 4096)){
        void (*foo_p2)(void) = proc2; 
        proc2_stack_ptr = setup_stack(proc2_stack, foo_p2);
    }
    else{
        printf("proc2 stack setup failed !\n");
    }

    do_switch(&main_ptr, proc1_stack_ptr);

}


/***********************************************/
/*********** Your code ends here ***************/
/***********************************************/
