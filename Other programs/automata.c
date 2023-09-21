/*
 * DESCRIPTION: Simulates a finite automaton.
 * 
 * AUTHOR: Claudio Saji
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>


/*
 * 
 */
char *InputString(size_t size){
    int ch;
    size_t len = 0;
    char *str = realloc(NULL, sizeof(char)*size);
    if(!str) return str;
    while((ch=getchar()) != EOF && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));    // add space for another 16 characters to be read
            if(!str) return str;
        }
    }
    str[len++]= '\0';    // Null terminates the string    
    return realloc(str, sizeof(char)*len);      //reallocates the string to the exact size of the input string
}


/*
 * 
 */
int main() {
   int *b, n, m, i, j, L, x, q, p, temp, randIndex1, randIndex2, r, c=0, sw=0;
   printf("Enter the input string: ");
   char *a = InputString(10);
   srand(time(NULL));
   printf("Enter the number of states: ");
   scanf("%d", &n);
   printf("Enter the number of input symbols: ");
   scanf("%d", &m);
   int automata[n][m];
   L = strlen(a);
   for(i=0; i<n; ++i){          // Fill the transition function with at least 1 apparition of each state, to do
        for(j=0; j<m; ++j){       // this we fill the matrix with numbers 0 through n-1, then random. Afterwards
            if(c==n)                // we shuffle the matrix to make it random
                automata[i][j] = rand() % n;     // random number from 0 to n-1
      		else{
                automata[i][j] = c;
                ++c;
            }
        }
    }
   for(i=0; i<n; i++){          // Shuffle the transition function to make it random
        for(j=0; j<m; j++){
            temp = automata[i][j];
            randIndex1 = rand() % n;
            randIndex2 = rand() % m;
            automata[i][j] = automata[randIndex1][randIndex2];
            automata[randIndex1][randIndex2] = temp;
        }
   }
   printf("Transition function:\n\n");
   printf(" \t|0|");
   for(i=1; i<m; ++i)
        printf("\t|%d|", i);
   printf("\n");
    for(i=0; i<n; ++i){
		printf("q%d |\t", i);         
        for(j=0; j<m; ++j)
			printf("q%d\t", automata[i][j]); 
		printf("\n"); 
	}
   q = rand() % n;      // random number of final states 
   if(q == 0)
        printf("\nNo final states  ");
   else{
        b = calloc(q,sizeof(int));       // array with non repeating random final states
        printf("\nFinal states: ");
        i=0;
        while(i<q){
            r = rand() % n;
            for(j=0; j<i; ++j){       // check if the random r is already on the array
                if(b[j]==r)
                    break;
            }
            if(j==i){
                b[i++] = r;
                printf("q%d, ", r);
            }
        }
    }
	
   x = rand() % n;      // random initial state 
   printf("\b\b \nLatex output: $");
   printf("q_%d", x);
   for(i=0; i<L; ++i)
        printf("%c", a[i]);
   printf(" \\vdash ");
   for(i=0; i<L; i++){
        p = a[i] - '0';         // char to int
        x = automata[x][p];     // x is the current state 
        for(j=0; j<L; j++){
            if(j==i)
                printf("%cq_%d", a[j], x);
            else
                printf("%c", a[j]);
        }
        if(i==L-1)
        	break;
        else
        	printf(" \\vdash ");
   }
    printf("$");
   printf("\nFinal state: q%d", x);
   for(i=0; i<q; ++i){
        if(x == b[i]){        // See if the automaton accepts the string
            printf("\nAccepted string\n");     
            sw = 1;
            break;
        }
   }
   if(sw == 0)
      printf("\nNot accepted string\n");
   free(a);
   return 0;
}




