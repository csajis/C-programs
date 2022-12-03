/*
 * AUTHOR: Claudio Saji Santander
 *
 * FILE: DatingOnLine.c
 * 
 * DESCRIPTION: Program to find the maximum possible area of a radial diagram given
                a list of activities graded with scores between 0 and 100.
 * 
 * 
 * LAST REVISED: Santiago de Chile, 20/11/2017
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define PI 3.14159265358979323846


struct Point {
   double x, y;
};


/*
 * 
 */
void Quicksort(int *number,int first,int last){   //Quicksort using divide et impera
    int i, j, pivot, temp;
    
    if(first<last){
       pivot=first;
       i=first;
       j=last;

       while(i<j){
          while(number[i]<=number[pivot]&&i<last)
             i++;
          while(number[j]>number[pivot])
             j--;
          if(i<j){
             temp=number[i];
             number[i]=number[j];
             number[j]=temp;
          }
       }

       temp=number[pivot];
       number[pivot]=number[j];
       number[j]=temp;
       Quicksort(number,first,j-1);
       Quicksort(number,j+1,last);
    }
}


/*
 * 
 */
int *Maximum_Area_Sort(int *a, int *b, int N){
   int i, k = 0;
   for(i=0; i<N; i=i+2){
      b[k] = a[i];
      if(N%2 != 0 && i+1==N)      //case where N is odd
         return b;
      b[N-1-k] = a[i+1];
      k = k+1;
   }
   return b;
}


/*
 * 
 */
struct Point GetVertices(int *a, int N, struct Point Vertices[N]){
   int i;
   for(i=0; i<N; i=i+1){
      Vertices[i].x = a[i] * cos(2*PI * i/N);
      Vertices[i].y = a[i] * sin(2*PI * i/N);
   }
   return Vertices[N];
}


/*
 * 
 */
double GaussArea(int N, struct Point Vertices[N]){
   double sum1 = 0, sum2 = 0, result;
   int i;
   for(i=0; i<N-1; i=i+1){
      sum1 = sum1 + Vertices[i].x * Vertices[i+1].y;
      sum2 = sum2 + Vertices[i+1].x * Vertices[i].y;
   }
   result = sum1 + Vertices[N-1].x * Vertices[0].y - sum2 - Vertices[0].x * Vertices[N-1].y;
   if(result < 0)
      result = result * -1;
   return result/2;
}


/*
 * 
 */
int main() {
   int N, *a, *b, i;
   printf("Enter number of activities:\n");
   scanf("%d", &N);
   a = calloc(N,sizeof(int));
   b = calloc(N,sizeof(int));
   struct Point Vertices[N];
   printf("Enter the scores given to each activity:\n");
   for(i=0; i<N; i=i+1){
      if(scanf("%d", &a[i]) == EOF)
         break;
   }
   Quicksort(a, 0, N-1);
   b = Maximum_Area_Sort(a, b, N);
   Vertices[N] = GetVertices(b, N, Vertices);
   printf("%.3lf\n", GaussArea(N, Vertices));
}




