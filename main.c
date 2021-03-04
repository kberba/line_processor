/************************************************
* Assignment 4: Multi-threaded Producer Consumer Pipeline
* Karen Berba
* CS 344
*
*
* Instructions: 
*   Write a program that creates 4 threads to process input from standard input as follows:
*
*       1) Thread 1, called the Input Thread, reads in lines of characters from the standard input.
*       2) Thread 2, called the Line Separator Thread, replaces every line separator in the input by a space.
*       3) Thread, 3 called the Plus Sign thread, replaces every pair of plus signs, i.e., "++", by a "^".
*       4) Thread 4, called the Output Thread, write this processed data to standard output as lines of exactly 80 characters.
*   
*   Furthermore, in your program these 4 threads must communicate with each other using the Producer-Consumer approach. 
*
 Source(s): 
 Assignment 4 - Multi Threaded Implementation Sample: https://repl.it/@cs344/65prodconspipelinec
 CS 344 Piazza - @466
 CS 344 Piazza - @458
 CS 344 Piazza - @458
*************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h> // must link with -lm
#include <string.h>
#include <stdbool.h>


#define NUM_LINES 50
#define LINE_LENGTH 1000


// Buffer 1, shared resource between Thread 1 and Thread 2
char buffer_1[NUM_LINES][LINE_LENGTH] = {{0}};
// Number of items in the buffer
int count_1 = 0;
// Index where Thread 1 will put the next item
int prod_idx_1 = 0;
// Index where Thread 2 will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;


// Buffer 2, shared resource between Thread 2 and Thread 3
char buffer_2[NUM_LINES][LINE_LENGTH] = {{0}};
// Number of items in the buffer
int count_2 = 0;
// Index where Thread 2 will put the next item
int prod_idx_2 = 0;
// Index where Thread 3 will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;


// Buffer 3, shared resource between Thread 3 and Thread 4
char buffer_3[NUM_LINES][LINE_LENGTH] = {{0}};
// Number of items in the buffer
int count_3 = 0;
// Index where Thread 3 will put the next item
int prod_idx_3 = 0;
// Index where Thread 4 will pick up the next item
int con_idx_3 = 0;
// Initialize the mutex for buffer 3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;


int count_4 = 0;
pthread_mutex_t mutex_4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_4 = PTHREAD_COND_INITIALIZER;


/*
 Function that the input thread (Thread 1) will run.
 Get input from the user.
 Put the item in the buffer shared with the line separator thread (Thread 2).

 Source(s): 
 CS 344 Piazza - @466
 CS 344 Piazza - @458
*/
void *get_input(void *args)
{
  int stop = 0;
  char *line = calloc(1024, sizeof(char));

  // printf("starting here\n");   // TESTING!!

  // the basic thread loop
  while(!stop) {
    // get the input into a local buffer
		fgets(line, 1000, stdin);

    // printf("%s\n", line);   // TESTING!!
    
		// lock the output buffer
		pthread_mutex_lock(&mutex_1);
		
    // copy the input line into the buffer for the next thread
    strcpy(buffer_1[prod_idx_1], line);
		
    // increment count
		count_1++;
    prod_idx_1++;

    // signal to the consumer that buffer is no longer empty
    pthread_cond_signal(&full_1);

    // unlock the mutex
		pthread_mutex_unlock(&mutex_1);
    
    // wait until the print thread has dealt with this line
		pthread_mutex_lock(&mutex_4);
		while (count_4 == 0) {
			pthread_cond_wait(&full_4, &mutex_4);
		}
		count_4++;
		pthread_mutex_unlock(&mutex_4);

		// if we've passed on "STOP" to the next thread, we can exit
		if (!strncmp(line, "STOP\n", 5)) {
      // printf("got stopped\n");   // TESTING!!
			stop = 1; 
    }
  }

  free(line);
  line = NULL;

  return NULL;
}


/*
*    Function that the input thread (Thread 2) will run.
*    Replace newline character with a space
*    Put the item in the buffer shared with the plus sign thread (Thread 3).
*
*    Source(s):
     CS 344 Piazza - @466
*/
void *replace_line_sep(void *args) {
  int stop = 0;
  char *line = calloc(1024, sizeof(char));

  // printf("starting replace line\n");   // TESTING!!

  // keep looping until STOP is called
  while (!stop) { 
    // wait for lock 
    pthread_mutex_lock(&mutex_1); 

    // printf("lock mutex 1\n");   // TESTING!!
    
    // loop while count_1 == 0 and wait on condition full_1
    while(count_1 == 0) {
      pthread_cond_wait(&full_1, &mutex_1);
    }

    // copy incoming data locally
    strcpy(line, buffer_1[con_idx_1]);

    // printf("after strcpy: %s\n", buffer_1[con_idx_1]);   // TESTING!!

    // decrement count_1 and release the lock
    count_1--;
    con_idx_1++;
    pthread_mutex_unlock(&mutex_1);

    // check for STOP before we remove the newline
    if (strncmp(line, "STOP\n", 5) == 0) {
      // printf("getting stopped\n");   // TESTING!!
      stop = 1;
    }
    else {
      // remove newline and replace with space
      int i = strlen(line) - 1;
      if(line[i] == '\n') {
        line[i] = ' ';
        // printf("%c\n", line[i]);   // TESTING!!
      }
      
    }

    // printf("after for loop\n");   // TESTING!!

    // producer portion
    // lock the output buffer
		pthread_mutex_lock(&mutex_2);  

    // copy the input line into the buffer for the next thread   
    strcpy(buffer_2[prod_idx_2], line);

    // printf("%s\n", buffer_2[count_2]);   // TESTING!!

    // increment count
    count_2++;
    prod_idx_2++;

    // signal to the consumer that buffer is no longer empty
    pthread_cond_signal(&full_2);  

    // unlock the mutex
		pthread_mutex_unlock(&mutex_2);
   
  }

  free(line);
  line = NULL;

  return NULL;
  
}



/*          
*   STRING REPLACE
*   - Replaces portions of the string (needle) with chosen string replacement (strReplace)
*   - target: string that is being searched and having portions replaced
*   - needle: portion of the string that will be replaced
*   - strReplace: is what the needle is being replaced with
*
*   Source(s): 
*       https://stackoverflow.com/questions/32413667/replace-all-occurrences-of-a-substring-in-a-string-in-c/32413923
*
*/
void replace_string(char *target, const char *needle, const char *strReplace) {
    char buffer[1024] = {0};
    char *insertHere = &buffer[0];
    const char *tmp = target;
    size_t needleLen = strlen(needle);
    size_t strReplaceLen = strlen(strReplace);

    while(1) {
        const char *p = strstr(tmp, needle);

        // after finding all instances of the needle
        if(p == NULL) {
            // copies the last instance
            strcpy(insertHere, tmp);
            break;
        }

        // copy portion before the needle
        memcpy(insertHere, tmp, p-tmp);
        insertHere += p-tmp;

        // copy the string replacement 
        memcpy(insertHere, strReplace, strReplaceLen);
        insertHere += strReplaceLen;

        // shift the pointers
        tmp = p + needleLen;
    }

    // copies the changed string to target
    strcpy(target, buffer);
}



/*
 Function that the replace plus sign (Thread 3) will run.
 Get input from the user.
 Put the item in the buffer shared with the output thread (Thread 4).

 Source(s): 
 CS 344 Piazza - @466
 CS 344 Piazza - @458
 https://stackoverflow.com/questions/32413667/replace-all-occurrences-of-a-substring-in-a-string-in-c/32413923
*/
void *replace_plus_sign(void *args) {
  int stop = 0;
  char *line = calloc(1024, sizeof(char));

  // printf("starting plus\n");   // TESTING!!

  // keep looping until STOP is called
  while (!stop) { 
    // wait for lock 
    pthread_mutex_lock(&mutex_2); 

    // printf("locking mutex 2\n");   // TESTING!!
    
    // loop while count_2 == 0 and wait on condition full_2
    while(count_2 == 0) {
      pthread_cond_wait(&full_2, &mutex_2);
    }

    // copy incoming data locally
    strcpy(line, buffer_2[con_idx_2]);

    // printf("strcpy buffer 2\n");   // TESTING!!

    // decrement count_2 and release the lock
    count_2--;
    con_idx_2++;
    pthread_mutex_unlock(&mutex_2);

    // check for STOP 
    if (strncmp(line, "STOP\n", 5) == 0) {
      // printf("stopped\n");   // TESTING!!
      stop = 1;
    }
    else {
      // replace ++ with ^
      replace_string(line, "++", "^"); 
    
    }

    // producer portion
    // lock the output buffer
		pthread_mutex_lock(&mutex_3);  

    // copy the input line into the buffer for the next thread   
    strcpy(buffer_3[prod_idx_3], line);

    // printf("strcpy buffer3[count3]\n");   // TESTING!!
    // printf("%s\n", buffer_2[count_2]);   // TESTING!!
    // printf("%s\n", buffer_3[count_3]);    // TESTING!!

    // increment count
    count_3++;
    prod_idx_3++;

    // signal to the consumer that buffer is no longer empty
    pthread_cond_signal(&full_3);  

    // unlock the mutex
		pthread_mutex_unlock(&mutex_3); 
    
  }

  free(line);
  line = NULL;

  return NULL;

}



/*
 Function that the output thread (Thread 4) will run. 
 Consume an item from the buffer shared with plus thread (Thread 3)
 Print the item.

 Source(s): 
 CS 344 Piazza - @465
*/
void *write_output(void *args)
{
  int stop = 0;
  int last_line = 0; // keep track of what we've printed
  char output[81] = {0}; // a buffer for each line we print
  char *print_buffer = calloc(LINE_LENGTH*NUM_LINES, sizeof(char));  // linear buffer
  char *line = calloc(LINE_LENGTH, sizeof(char));
  // printf("starting here\n");   // TESTING!!

  while (!stop)
  { 
    // wait for lock 
    pthread_mutex_lock(&mutex_3); 

    // printf("locked mutex 3\n");    // TESTING!!
    
    // loop while count_3 == 0 and wait on condition full_3
    while(count_3 == 0) {
      pthread_cond_wait(&full_3, &mutex_3);
    }

    // append incoming content to our big linear buffer
    strcat(print_buffer, buffer_3[con_idx_3]);
    strcpy(line, buffer_3[con_idx_3]);

    // printf("%s\n", buffer_3);    // TESTING!!

    // decrement count_3 and release the lock
    count_3--;
    con_idx_3++;
    pthread_mutex_unlock(&mutex_3);

    // printf("%s\n", print_buffer);    // TESTING!!
    
    // get the current count of how many full lines are in the buffer
    int cur_line_count = (strlen(print_buffer))/80; 

    // printf("%d\n", cur_line_count);  // TESTING!!

    // print from the last count of full lines to the current count
    for (int line = last_line; line < cur_line_count; line++) {
      // printf("inside for loop\n");   // TESTING!!
      memset(output, 0, 81); // clear the output line
      
      // copy appropriate content into our output line
      strncpy(output, print_buffer + (line*80), 80); 
      
      // and, finally, we print
      printf("%s\n", output);   
      fflush(stdout);
    }  
    // now save away the new last_line value
    last_line = cur_line_count;

    // check for STOP
    if(strncmp(line, "STOP\n", 5) == 0) {
      // printf("stopped\n");   // TESTING!!
      stop = 1;
    }
    
    // printf("outside for loop\n");  // TESTING!!
    // printf("stop value: %d\n", stop);  // TESTING!!

    // let the line input thread know we are done with printing
		pthread_mutex_lock(&mutex_4);
		count_4--;
		pthread_cond_signal(&full_4);
		pthread_mutex_unlock(&mutex_4);

  }

  free(print_buffer);
  print_buffer = NULL;
  
  return NULL;

}



int main()
{
  srand(time(0));
  pthread_t input_t, line_sep_t, plus_sign_t, output_t;
  
  // Create the threads
  pthread_create(&input_t, NULL, get_input, NULL);
  pthread_create(&line_sep_t, NULL, replace_line_sep, NULL);
  pthread_create(&plus_sign_t, NULL, replace_plus_sign, NULL);
  pthread_create(&output_t, NULL, write_output, NULL);
  
  // Wait for the threads to terminate
  pthread_join(input_t, NULL);
  pthread_join(line_sep_t, NULL);
  pthread_join(plus_sign_t, NULL);
  pthread_join(output_t, NULL);
  
  return EXIT_SUCCESS;
}