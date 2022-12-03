/*
 * AUTHOR: Claudio Saji Santander
 *
 * FILE: lab5-p.c
 *
 * DESCRIPTION: Implementation of a farm using threads for the multiplications.
 *              A farm is a distributed algorithm in which a process (typically the master)
 *              assigns tasks (N) to be performed to the other processes.
 *              When a process completes the assigned task,
 *              it asks the master to assign a new task.
 *              A task is the multiplication of two matrices.
 *
 * LAST REVISED: Santiago de Chile, 01/11/2018
 *
 * COMPILE:    mpicc lab5-p.c -o lab5-p.exe
 * RUN:        mpirun -np K (number of processes) ./lab5-p.exe H (number of threads) < datos.txt
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "mpi.h"

#define TAG1 1
#define TAG2 2
#define MASTER_NODE 0


struct Message{
   int mythid, myprocid, numthreads, **A, **B, f, c, q;
};

int whoami, hm_are_we, **result;    // result is the global matrix for the result of the multiplication

MPI_Status status;


/*
 *
 */
void Usage(char *message){
   if(whoami == MASTER_NODE)
      printf("\nUsage : %s H (number of threads)\n", message);
}


/*
 *
 */
int **InitializeMatrixA(int f, int c){    // A(FxC)
   int **matrix, i, j;
   
   matrix = malloc(f * sizeof(int*));
   for(i=0; i<f; ++i)
     matrix[i] = malloc(c * sizeof(int));
          
   for(i=0; i<f; ++i)   // initialize the matrix with 0s
     for(j=0; j<c; ++j)
       matrix[i][j]=0;
       
   return matrix;
}


/*
 *
 */
int **InitializeMatrixB(int c, int q){    // B(CxQ)
   int **matrix, i, j;
   
   matrix = malloc(c * sizeof(int*));
   for(i=0; i<c; ++i)
     matrix[i] = malloc(q * sizeof(int));
          
   for(i=0; i<c; ++i)   // initialize the matrix with 0s
     for(j=0; j<q; ++j)
       matrix[i][j]=0;
       
   return matrix;
}


/*
 *
 */
void *MatrixMultiplication(void *s){    // A(FxC) B(CxQ) = C(FxQ)
   int i, j, k, size, **A, **B;
   struct Message *m;

   m = (struct Message *) s;
   A = m->A;
   B = m->B;
   size = m->f / m->numthreads;
   
   if(m->mythid == m->numthreads-1 && m->f % m->numthreads != 0){ // numthreads doesn't divide the 
      for(i=m->mythid * size; i<m->f; ++i){                          // number of rows and mythid is the last thread
        for(j=0; j<m->q; ++j){
          result[i][j] = 0;
          for(k=0; k<m->c; ++k)
            result[i][j] += A[i][k] * B[k][j];
        }
      }
   }
   
   else{
      for(i=m->mythid * size; i<m->mythid * size + size; ++i){
        for(j=0; j<m->q; ++j){
          result[i][j] = 0;
          for(k=0; k<m->c; ++k)
            result[i][j] += A[i][k] * B[k][j];
        }
      }
   }
   
   //printf("From Process %d, Thread %d Finished the task\n",m->myprocid,m->mythid);
   pthread_exit(0);
}


/*
 *
 */
void Process(int **data, int n, int h){
   int i, j, ok, k, notyet, fin, *order, **A, **B, f, c, q;
   pthread_t *thread;
   pthread_attr_t attribute;
   struct Message **m;
   void *exit_status;
   struct timespec start, finish;
   double elapsed;
   
   if(whoami == MASTER_NODE){
     clock_gettime(CLOCK_MONOTONIC, &start);   // start timer
     
     notyet = 1;
     i=0;  // tracks the number of assigned tasks
     j=0;  // tracks the number of finished tasks
     for(k=1; k < hm_are_we; ++k){
       if(i == n)  // number of processes > number of tasks
         break;
       MPI_Send(data[i],3,MPI_INT,k,TAG1,MPI_COMM_WORLD);  // send tasks to all the other nodes
       i++;
     }
     while(notyet == 1){
       if(i == n){
         while(j < n){   // lasts tasks that need to finish
           MPI_Recv(&ok,1,MPI_INT,MPI_ANY_SOURCE,TAG2,MPI_COMM_WORLD,&status); 
           j++;
         }
         notyet = 0;
         fin = 0;
         for(k=1; k < hm_are_we; ++k)    // tell the other nodes that the job is done
           MPI_Send(&fin,1,MPI_INT,k,TAG1,MPI_COMM_WORLD);
       }
       else{
         MPI_Recv(&ok,1,MPI_INT,MPI_ANY_SOURCE,TAG2,MPI_COMM_WORLD,&status); // a node has finished it's task
         MPI_Send(data[i],3,MPI_INT,status.MPI_SOURCE,TAG1,MPI_COMM_WORLD);  // send another task to that node
         i++;
         j++;
       }
     }
     
     clock_gettime(CLOCK_MONOTONIC, &finish);    // end timer
     elapsed = (finish.tv_sec - start.tv_sec);
     elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
     printf("\n\nElapsed Wall Time: %.3f [Secs]\n\n", elapsed);
   }
   else{
     thread = calloc(h,sizeof(pthread_t));
     m = calloc(h,sizeof(struct Message *));
     for(i=0; i<h; ++i)
        m[i] = calloc(1,sizeof(struct Message));
            
     order = malloc(3 * sizeof(int));
     notyet = 1;
     while(notyet == 1){
       MPI_Recv(order,3,MPI_INT,MASTER_NODE,TAG1,MPI_COMM_WORLD,&status);  // receive the order of the two matrices
       if(order[0] == 0)   // the job has finished
         notyet = 0;
       else{
         f = order[0];
         c = order[1];
         q = order[2];
         A = InitializeMatrixA(f, c);
         B = InitializeMatrixB(c, q);
         
         result = malloc(f * sizeof(int*));     //assign the memory for the result of the multiplication
         for(i=0; i<f; ++i)
           result[i] = malloc(q * sizeof(int));
         
         pthread_attr_init(&attribute);
         pthread_attr_setdetachstate(&attribute,PTHREAD_CREATE_JOINABLE);
         for(i=0; i<h; ++i){
            m[i]->mythid = i;
            m[i]->myprocid = whoami;
            m[i]->numthreads = h;
            m[i]->f = f;
            m[i]->c = c;
            m[i]->q = q;
            m[i]->A = A;
            m[i]->B = B;
            pthread_create(&thread[i],&attribute,MatrixMultiplication,(void *) m[i]);
         }
         pthread_attr_destroy(&attribute);
         for(i=0; i<h; ++i)
            pthread_join(thread[i],&exit_status);
         
         
         /*printf("\nA=\n");   // to test if the results are correct
         for(i=0; i<f; ++i){
           for(j=0; j<c; ++j)
             printf("%d ", A[i][j]);
           printf("\n");
         }
         printf("\n");
         
         printf("\nB=\n");
         for(i=0; i<c; ++i){
           for(j=0; j<q; ++j)
             printf("%d ", B[i][j]);
           printf("\n");
         }
         printf("\n");
         
         printf("\nAB=\n");
         for(i=0; i<f; ++i){
           for(j=0; j<q; ++j)
             printf("%d ", result[i][j]);
           printf("\n");
         }
         printf("\n");*/
         
         
         for(i=0; i<f; ++i)    //free the memory for matrix A
           free(A[i]);
         free(A);
         
         for(i=0; i<c; ++i)    //free the memory for matrix B
           free(B[i]);
         free(B);
         
         for(i=0; i<f; ++i)    //free the memory for the result
           free(result[i]);
         free(result);
         
         ok = 1;
         MPI_Send(&ok,1,MPI_INT,MASTER_NODE,TAG2,MPI_COMM_WORLD);  // tell the master that the task is done
       }
     }
   }
}


/*
 *
 */
int main(int argc, char *argv[]){
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int me, n, h, i, **data;

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&hm_are_we);
  MPI_Comm_rank(MPI_COMM_WORLD,&whoami);
  MPI_Get_processor_name(processor_name,&me);
  printf("Process [%d] Alive on %s\n",whoami,processor_name);
    
  if(argc != 2)
    Usage(argv[0]);
  else{
    if(whoami == MASTER_NODE){   // the master scans the input
      scanf("%d", &n);
      data = malloc(n * sizeof(int*));
      for(i=0; i<n; ++i)
        data[i] = malloc(3 * sizeof(int));
      for(i=0; i<n; ++i)
        scanf("%d %d %d", &data[i][0], &data[i][1], &data[i][2]);
    }
    h = atoi(argv[1]);
    MPI_Barrier(MPI_COMM_WORLD);
    Process(data, n, h);
  }
  
  MPI_Finalize();
  return 0;
}


