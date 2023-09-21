/*
 * DESCRIPTION: Matrix of stars using threads.
 *
 * AUTHOR: Claudio Saji
 *           
 * COMPILE:    gcc -o stars-matrix.exe stars-matrix.c -pthread
 *             ./stars-matrix.exe k (number of threads) < input.txt
 */


#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdbool.h>
#include<time.h>


struct Message{
   int myid, numthreads, numrows, numcolumns, **matrix;
   bool *s;
};


/*
 * 
 */
bool estrella(int r,int l,int u,int d,int c){
    float i;
    i=((r+l+u+d+c) / (float) 5);
    if(i>6)
      return true;
    else
      return false;
}


/*
 * 
 */
void *Star(void *n){
   int i, j, size, jj=0;
   struct Message *m;
   bool *s;
   m = (struct Message *) n;
   size = m->numrows / m->numthreads;
   s = m->s;
    
   if(m->myid == m->numthreads-1 && m->numrows % m->numthreads != 0){ // k doesn't divide m and myid is last thread
      for(i=m->myid*size + 1; i<m->numrows+1; i++){
         for(j=1; j<m->numcolumns+1; j++){
            if(estrella(m->matrix[i][j+1],m->matrix[i][j-1],m->matrix[i-1][j],m->matrix[i+1][j],m->matrix[i][j]))
               s[jj++] = 1;  // star
            else
               s[jj++] = 0;  // not star
         }
      }
   }
    
   else{
      for(i=m->myid*size + 1; i<m->myid*size + 1 + size; i++){
         for(j=1; j<m->numcolumns+1; j++){
            if(estrella(m->matrix[i][j+1],m->matrix[i][j-1],m->matrix[i-1][j],m->matrix[i+1][j],m->matrix[i][j]))
               s[jj++] = 1;  // star
            else
               s[jj++] = 0;  // not star
         }
      }
   }
   
   pthread_exit((void *) m);
}


/*
 * 
 */
void *Usage(char *argv[]){
   printf("Usage: %s k (number of threads) < input.txt\n", argv[0]);
   exit(1);
}


/*
 * 
 */
int main(int argc, char *argv[]){
    int i, j, n, m, k, u, h, l, ii, flag=0, **matrix;
    struct timespec start, finish;
    double elapsed;
    pthread_t *thread;
    pthread_attr_t attribute;
    struct Message **p;
    void *exit_status;
    bool **s;     // matrix of stars
    
    if(argc != 2)
       Usage(argv);
    else{
         scanf("%i %i\n", &m, &n);
         m+=2;  //rows
         n+=2;  //columns
         
         matrix = malloc(m * sizeof(int*));  //matrix with the input data
         for(i=0; i<m; i++)
             matrix[i] = malloc(n * sizeof(int));
         
         for(i=0; i<m; i++)  // initialize the matrix with 0s
             for(j=0; j<n; j++)
                 matrix[i][j]=0;
                 
         for(i=0; i<(m*n); i++){
             scanf("%i %i %i",&l,&h,&u);
             matrix[l][h]=u;
             if(l==0 && h==0 && u==0)
                 break;
         }
         if(i==0)
            flag=1;
         
         m-=2;  //rows minus the two 0s rows
         n-=2;  //columns minus the two 0s columns
         
         k = atoi(argv[1]);
         thread = calloc(k,sizeof(pthread_t));
         s = calloc(k,sizeof(bool *));
         p = calloc(k,sizeof(struct Message *));
         for(ii=0; ii<k; ii++){
             if(m%k != 0 && ii == k-1)    // k doesn't divide m and ii is the last thread
               s[ii] = calloc((m/k)*n + (m%k)*n,sizeof(bool));
             else
               s[ii] = calloc((m/k)*n,sizeof(bool));
         }
         for(ii=0; ii<k; ii++)
             p[ii] = calloc(1,sizeof(struct Message));
         
         pthread_attr_init(&attribute);
         pthread_attr_setdetachstate(&attribute,PTHREAD_CREATE_JOINABLE);
         
         clock_gettime(CLOCK_MONOTONIC, &start);
         
         for(ii=0; ii<k; ii++){     // create the threads
             p[ii]->myid = ii;
             p[ii]->numrows = m;
             p[ii]->numcolumns = n;
             p[ii]->numthreads = k;
             p[ii]->matrix = matrix;
             p[ii]->s = s[ii];
             pthread_create(&thread[ii],&attribute,Star,(void *) p[ii]);
         }
         pthread_attr_destroy(&attribute);
         
         for(ii=0; ii<k; ii++){
             pthread_join(thread[ii],&exit_status);
             p[ii] = (struct Message *) exit_status;
             s[ii] = p[ii]->s;
         }
         
         clock_gettime(CLOCK_MONOTONIC, &finish);
         elapsed = (finish.tv_sec - start.tv_sec);
         elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
         
         if(flag==1)    //for testing time
            printf("\nEmpty sky");
         else{
            for(i=0; i<k; i++){     // print the matrix of stars
               if(m%k != 0 && i == k-1){     // k doesn't divide m and i is the last thread
                  for(j=0; j<(m/k)*n + (m%k)*n; j++){
                     if(j%n == 0 && i+j!=0)
                        printf("\n");
                     if(s[i][j] == 1)
                        printf("* ");
                     else
                        printf("  ");
                  }
               }
               else{
                  for(j=0; j<(m/k)*n; j++){
                     if(j%n == 0 && i+j!=0)
                        printf("\n");
                     if(s[i][j] == 1)
                        printf("* ");
                     else
                        printf("  ");
                  }
               }
            }
         }

         printf("\n");

         for(i=0; i<m+2; i++)  //free the memory
             free(matrix[i]);
         free(matrix);
         
         for(i=0; i<k; i++)
             free(s[i]);
         free(s);
         
         printf("\n\nElapsed Wall Time: %.4f [Secs]\n\n", elapsed);
     }
    
    return 0;
}
   
   

