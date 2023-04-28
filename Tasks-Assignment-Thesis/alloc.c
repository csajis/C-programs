/************************************************************************
 *
 * alloc.c
 *
 * Programmer: Claudio Saji Santander
 * Compile: gcc -o alloc.exe alloc.c -lm
 * Execute: ./alloc.exe m (exponent of 2) p (number of processes) 
 *          -{zig, col, back} (optimization method) 
 *          opt-matrix-m-p-{zig, col, back}.txt (optimized matrix in the txt file)
 *          exp-times-{zig, col, back}-p.txt (txt file for the experiments which contains the value of m and the time it took)
 *                                                                
 * Example: ./alloc.exe 8 4 -zig opt-matrix-8-4-zig.txt exp-times-zig-4.txt
 *
 ************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <sys/time.h>


struct ones {    // struct that enables to save the index of each sum of the "elements" matrix
   unsigned int num;  // the number of active 1s in the "elements" matrix
   unsigned int index;  // the index of the row in the "elements" matrix
};


struct matrix_size {   // struct that enables to return 2 values in a function, the matrix and its real size
   struct ones **matrix;
   unsigned int size;
};


void merge(struct ones arr[], int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
    struct ones L[n1], R[n2];
  
    for (i = 0; i < n1; i++) {
        L[i].num = arr[l + i].num;
        L[i].index = arr[l + i].index;
    }
    for (j = 0; j < n2; j++) {
        R[j].num = arr[m + 1 + j].num;
        R[j].index = arr[m + 1 + j].index;
    }
  
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (L[i].num <= R[j].num) {
            arr[k].num = L[i].num;
            arr[k].index = L[i].index;
            i++;
        }
        else {
            arr[k].num = R[j].num;
            arr[k].index = R[j].index;
            j++;
        }
        k++;
    }
  
    while (i < n1) {
        arr[k].num = L[i].num;
        arr[k].index = L[i].index;
        i++;
        k++;
    }
  
    while (j < n2) {
        arr[k].num = R[j].num;
        arr[k].index = R[j].index;
        j++;
        k++;
    }
}


/*
 *
 */
void mergeSort(struct ones arr[], int l, int r) {
   int m;
   if (l < r) {
      m = l + (r - l) / 2;
      mergeSort(arr, l, m);
      mergeSort(arr, m + 1, r);
      merge(arr, l, m, r);
   }
}


/*
 *
 */
void PrintMatrix (struct ones **matrix, unsigned int f, unsigned int c) {
    unsigned int i, j;
    
    printf("\n\nMatrix=\n");
    for(i=0; i<f; ++i) {
        for(j=0; j<c; ++j)
            printf("%d,%d ", matrix[i][j].num, matrix[i][j].index);
        printf("\n");
    }
    printf("\n");
}


/*
 *
 */
void FilePrintMatrix (FILE *fp_matrix, struct ones **matrix, unsigned int f, unsigned int c) { // prints the matrix in the file
    unsigned int i, j;
    
    fprintf(fp_matrix, "%d %d\n", f, c);  // print the sizes
    for(i=0; i<f; ++i) {
        for(j=0; j<c; ++j)
            fprintf(fp_matrix, "%d,%d ", matrix[i][j].num, matrix[i][j].index);
        fprintf(fp_matrix, "\n");
    }
}


/*
 *
 */
void CountNumbers (struct ones *arr, unsigned int size) {     // counts the number of times a number appears
    unsigned int sum, i;
    sum = 1;
    for(i=0; i<size; ++i) {
        if (i == size-1)
            printf("%d -> %d times\n", arr[i].num, sum);
        else if (arr[i].num == arr[i+1].num)
            sum++;
        else {
            printf("%d -> %d times\n", arr[i].num, sum);
            sum = 1;
        }
    }
}


/*
 *
 */
struct matrix_size ColFirst (struct ones *arr, unsigned int size, unsigned int c) {  // Columns first method
    unsigned int i, j, total, ideal, f_ideal, sum, *col_sizes, max_row, max_num, min_num, r, init;
    int k, rem, q, min_q, z;
    struct ones **matrix;
    struct matrix_size output_matrix;   // the matrix to return with its real size (it's used to be able to return 2 values in this function)
    bool rdy, key, all_done;

    r = size/c;
    matrix = (struct ones **) malloc(sizeof(struct ones*) * r);
    for(i=0; i<r; ++i)
        matrix[i] = (struct ones *) malloc(sizeof(struct ones) * c);
    
    col_sizes = malloc(sizeof(unsigned int) * c);

    total = 0;
    for(i=0; i<size; ++i)
        total = total + arr[i].num;   // the total sum of all the numbers in the array
    
    ideal = total/c;   // the ideal sum to have in all the columns
    f_ideal = ideal;   // fixed ideal that is used for when there is no remainder left
    rem = total%c;     // the total number of columns that will have (f_ideal + 1) as sum
    if (rem > 0)
        ideal++;
    printf("\nTotal = %d\nIdeal = %d\nRemainder = %d\n\n", total, f_ideal, rem);
    
    max_row = 0;
    rdy = false;  // flag used for when a column is ready. Skip the rest of the column and go to the next one
    all_done = false;  // flag that is used for breaking the principal loop when all the numbers of the array have been assigned in the matrix
    sum = 0;
    k = size - 1;   // k is the latest index that has a value of arr[k].num != 0.
    max_num = arr[k].num;
    for(j=0; j<c; ++j) {   // principal 'for' that traverses all the matrix. It breaks when all the numbers in the array have been assigned in the matrix
        for(i=0; i<size; ++i) {
            if (i == r) {
                matrix = (struct ones **) realloc(matrix, sizeof(struct ones*) * (r+=256));  //add space for another 256 rows
                for(init=i; init<r; ++init)
                    matrix[init] = (struct ones *) malloc(sizeof(struct ones) * c);
            }

            if (arr[k].num == 0) {    // k has to be the latest index that has a value of arr[k].num != 0. The numbers are marked as 0 when they have been used
                all_done = true;  // for cases where all the numbers to the left of k have been used, and also k has been used
                for(z = k - 1; z >= 0; --z) {
                    if (arr[z].num != 0) {
                        k = z;
                        all_done = false;
                        break;
                    }
                }
                if (all_done == true)
                    break;
            }

            if (rem <= 0)  // if there is no remainder left the value of ideal decreases by 1
                ideal = f_ideal;

            if (sum + arr[k].num <= ideal) {   // the number can be assigned
                if (sum + arr[k].num == ideal) {   // the sum is exactly the same to the ideal sum
                    if (rem > 0)
                        rem--;
                    rdy = true;  // the column is ready so set the flag as active
                }
                matrix[i][j].num = arr[k].num;
                matrix[i][j].index = arr[k].index;
                sum = sum + matrix[i][j].num;
                arr[k].num = 0;  // mark the number as used
                if (k > 0)
                    k--;
                if (rdy == true && j != c-1) {   // if the column is ready and is not the last one, skip to the next column
                    rdy = false;   // set the flag as inactive for the next column
                    break;  // skip to the next column
                }
            }

            else {   // search for a smaller number that can be assigned
                key = false;   // a flag that is used for when no smaller number was found
                min_num = max_num;  // the minimum number found in the search needs to be set in the beginning as the maximum number in the array (for comparison)
                for(q = k; q >= 0; --q) {
                    if (arr[q].num != 0 && arr[q].num < min_num) {  // a new minimum number was found
                        min_num = arr[q].num;
                        min_q = q;  // save the index in the array of the minimum number found
                    }
                    if (arr[q].num != 0 && sum + arr[q].num <= ideal) {   // a smaller number was found
                        if (sum + arr[q].num == ideal) {  // adding the smaller number leaves the sum with the same value of "ideal"
                            if (rem > 0)
                                rem--;
                            rdy = true;   // the column is ready so mark the flag as active
                        }
                        matrix[i][j].num = arr[q].num;
                        matrix[i][j].index = arr[q].index;
                        sum = sum + matrix[i][j].num;
                        arr[q].num = 0;   // mark the number arr[q].num as used
                        key = true;   // mark the flag as active indicating that a number was found
                        break;
                    }
                }
                if (rdy == true && j != c-1) {   // if the column is ready and is not the last one, skip to the next column
                    rdy = false;   // set the flag as inactive for the next column
                    break;  // skip to the next column
                }
                if (key == false) {   // cases where no smaller number that can be assigned was found
                    matrix[i][j].num = arr[min_q].num;
                    matrix[i][j].index = arr[min_q].index;
                    sum = sum + matrix[i][j].num;
                    if (rem > 0)
                        rem = rem - 1 - (sum - ideal);  // decrease the value of the remainder according to the difference of the ideal sum and actual sum
                    arr[min_q].num = 0;   // mark the number arr[min_q].num as used
                    if (j != c-1)  // only if this column is not the last one, skip to the next one
                        break;  // skip to the next column
                }
            }
        }
        if (j == c-1)
            col_sizes[j] = i;   // save the size of the last column
        else
            col_sizes[j] = i+1;   // save the size of the column
        if (col_sizes[j] > max_row)
            max_row = col_sizes[j];  // save the size of the row which has the greatest size. It's used later to know the real size of the matrix
        if (all_done == true)  // break the principal loop when all the numbers have been assigned
            break;
        sum = 0;   // reset the sum for the next column
    }
    
    for(j=0; j<c; ++j) {   // fill the rest of the matrix with zeros up to "max_row"
        for(i=col_sizes[j]; i<max_row; ++i) {
            matrix[i][j].num = 0;
            matrix[i][j].index = 0;
        }
    }

    matrix = (struct ones **) realloc(matrix, sizeof(struct ones*) * max_row);  // cut the unused part of the matrix
    output_matrix.matrix = matrix;
    output_matrix.size = max_row;
    return output_matrix;  // return the struct that has the matrix and its real size
}


/*
 *
 */
void AssignNumber (struct ones **matrix, struct ones *arr, unsigned int *sums, unsigned int i, unsigned int *ideal, unsigned int f_ideal,
                   int j, int *k, int *rem, bool *rdy, unsigned int max_num) {   //  assign a number from the array in the matrix
    int q, z, min_q;  // ideal, k, rem are "passed by reference"
    unsigned int min_num;
    bool key;
        
    if (*rem <= 0)
        *ideal = f_ideal;
                
    if (*k == -1) {   // there are no numbers left in the array
        matrix[i][j].num = 0;
        matrix[i][j].index = 0;
        return;
    }
    if (arr[*k].num == 0) {      // k has to be the latest index that has a value of arr[k].num != 0
        for(z = *k - 1; z >= 0; --z) {
            if (arr[z].num != 0) {
                *k = z;
                break;
            }
        }
    }
    if (rdy[j] == true) {   // first case: the column 'j' is ready
        matrix[i][j].num = 0;
        matrix[i][j].index = 0;
    }
    else if (sums[j] == *ideal) {   // second case: if (rem <= 0 && sums[j] == f_ideal)
        rdy[j] = true;             // happens when all the columns with the remainder are ready and the sum of the column 'j' was not marked as ready
        matrix[i][j].num = 0;
        matrix[i][j].index = 0;
    }
    else if (sums[j] + arr[*k].num <= *ideal) {   // third case: the number arr[k].num can be assign to this cell
        if (sums[j] + arr[*k].num == *ideal) {   // the sum is exactly the same to "ideal"
            rdy[j] = true;   // mark the column 'j' as ready
            if (*rem > 0)
                *rem = *rem - 1;
        }
        matrix[i][j].num = arr[*k].num;
        matrix[i][j].index = arr[*k].index;
        sums[j] = sums[j] + matrix[i][j].num;
        *k = *k - 1;
    }
    else {   // fourth case: the number arr[k].num cannot be assign to this cell, so search for a smaller number
        key = false;   // a flag that is used for when no smaller number was found
        min_num = max_num;  // the minimum number found in the search needs to be set in the beginning as the maximum number in the array (for comparison)
        for(q = *k - 1; q >= 0; --q) {
            if (arr[q].num != 0 && arr[q].num < min_num) {  // a new minimum number was found
                min_num = arr[q].num;
                min_q = q;  // save the index in the array of the minimum number found
            }
            if (arr[q].num != 0 && sums[j] + arr[q].num <= *ideal) {   // a smaller number was found
                if (sums[j] + arr[q].num == *ideal) {  // the sum is exactly the same to "ideal"
                    rdy[j] = true;   // mark the column 'j' as ready
                    if (*rem > 0)
                        *rem = *rem - 1;
                }
                matrix[i][j].num = arr[q].num;
                matrix[i][j].index = arr[q].index;
                sums[j] = sums[j] + matrix[i][j].num;
                arr[q].num = 0;   // mark the number arr[q].num as used
                key = true;   // mark the flag as active indicating that a number was found
                break;
            }
        }
        if (key == false) {   // if a smaller number was not found, assign the minimum number found
            rdy[j] = true;  // mark the column j as ready because it exceeds the ideal sum
            matrix[i][j].num = arr[min_q].num;
            matrix[i][j].index = arr[min_q].index;
            sums[j] = sums[j] + matrix[i][j].num;
            if (*rem > 0)
                *rem = *rem - 1 - (sums[j] - *ideal);  // decrease the value of the remainder according to the difference of the ideal sum and actual sum
            arr[min_q].num = 0;   // mark the number arr[min_q].num as used
        }
    }
}


/*
 *
 */
struct matrix_size ZigzagCheck (struct ones *arr, unsigned int size, unsigned int c) {  // Zigzag method
    unsigned int *sums, i, total, ideal, f_ideal, real_size, max_num, r, init;
    int j, k, rem;
    bool *rdy, dir;
    struct ones **matrix;   // the matrix that will have all its columns with similar sum
    struct matrix_size output_matrix;   // the matrix to return with its real size (it's used to be able to return 2 values in this function)
    
    rdy = malloc(c * sizeof(bool));   // Boolean array that keeps track on the columns that are ready (it's necesary because the value of "ideal" changes if "rem" changes)
    for(i=0; i<c; ++i)
        rdy[i] = false;

    r = size/c;  // give an initial number of rows that later will be cut or extended with realloc
    matrix = (struct ones **) malloc(sizeof(struct ones*) * r);
    for(i=0; i<r; ++i)
        matrix[i] = (struct ones *) malloc(sizeof(struct ones) * c);
      
    sums = malloc(c * sizeof(unsigned int));   // array with the temporal sums of the columns
    for(i=0; i<c; ++i)
        sums[i] = 0;
    
    total = 0;
    for(i=0; i<size; ++i)
        total = total + arr[i].num;   // the total sum of all the numbers in the array
    
    ideal = total/c;   // the ideal sum to have in all the columns
    f_ideal = ideal;   // fixed ideal that is used for when there is no remainder left
    rem = total%c;     // the total number of columns that will have (f_ideal + 1) as sum
    if (rem > 0)
        ideal++;
    printf("\nTotal = %d\nIdeal = %d\nRemainder = %d\n\n", total, f_ideal, rem);
    
    k = size - 1;   // k is the latest index that has a value of arr[k].num != 0. The array needs to be sorted by lowest to highest
    max_num = arr[k].num;
    dir = false;   // direction flag used for changing direction in the traversal of the rows. First, from left to right, then right to left, then left to right again. Like a zigzag
    for(i=0; i<size; ++i) {   // principal 'for' that traverses all the matrix. It breaks when all the numbers in the array have been assigned in the matrix
        if (i == r) {
            matrix = (struct ones **) realloc(matrix, sizeof(struct ones*) * (r+=256));  //add space for another 256 rows
            for(init=i; init<r; ++init)
                matrix[init] = (struct ones *) malloc(sizeof(struct ones) * c);
        }
        if (dir == false) {
            for(j=0; j<c; ++j) {   // traverse a row from left to right
                AssignNumber(matrix, arr, sums, i, &ideal, f_ideal, j, &k, &rem, rdy, max_num);
            }
            if (k == -1) {     // all the numbers in the array have been assigned in the matrix
                real_size = i+1;  // the real size of the matrix is used later for the realloc
                break;   // break the principal 'for'
            }
            dir = true;  // change the flag so the direction of the traversal changes
        }
        else {
            for(j=c-1; j>=0; --j) {   // traverse a row from right to left
                AssignNumber(matrix, arr, sums, i, &ideal, f_ideal, j, &k, &rem, rdy, max_num);
            }
            if (k == -1) {
                real_size = i+1;
                break;
            }
            dir = false;
        }
    }
    
    matrix = (struct ones **) realloc(matrix, sizeof(struct ones*) * real_size);  // cut the unused part of the matrix
    output_matrix.matrix = matrix;
    output_matrix.size = real_size;
    return output_matrix;  // return the struct that has the matrix and its real size
}


/*
 *
 */
bool backtrack (struct ones *arr, struct ones **matrix, unsigned int *sums, unsigned int size, unsigned int c,
                unsigned int ideal, unsigned int rem, unsigned int f_ideal, unsigned int x, unsigned int y,
                bool *used, bool *rdy, unsigned int all_rdy, unsigned int *real_size) {  // permutes the numbers until it finds a solution
    int i, j, rp;                                                                                       // begins from the end of the array
    unsigned int init;
    rp = -1;
    if (all_rdy == c) {  // Solution found, all the columns have the same sum
        for(j=y; j<c; ++j) {  // fill the rest of the row with zeros
            matrix[x][j].num = 0;
            matrix[x][j].index = 0;
        }
        *real_size = x+1;  // assign the real size of the matrix to cut the rest with realloc later
        return true;
    }
    else {
        for(i=size-1; i>=0; --i) {  // begin from the end of the array
            if (rp == arr[i].num) continue;  // skips the repeated numbers that are known to cause a backtrack
            
            if (rem == 0)
                ideal = f_ideal;
            else
                ideal = f_ideal + 1;
            
            if (rdy[y] == true) {  // first case: the column 'y' is ready. Assign a 0 to the cell
                matrix[x][y].num = 0;
                matrix[x][y].index = 0;
                
                if (y == c-1) {   // if this cell is in the last column, call the recursive function to the next row (x+1) and next column (0)
                    if (backtrack(arr, matrix, sums, size, c, ideal, rem, f_ideal, x+1, 0, used, rdy, all_rdy, real_size) == true)
                        return true;  // if the recursive function returns true, this instance returns true also
                }                     // this is used for finding just one solution
                else {   // if this cell is not in the last column, call the recursive function to the same row (x) and next column (y+1)
                    if (backtrack(arr, matrix, sums, size, c, ideal, rem, f_ideal, x, y+1, used, rdy, all_rdy, real_size) == true)
                        return true;
                }
                return false;  // if the recursive function returns false, there is no other option but to return false also
            }
                
            else if (rem == 0 && sums[y] == f_ideal) {  // second case: happens when all the columns with the remainder are ready and
                matrix[x][y].num = 0;                   //              the sum of the column 'y' was not marked as ready.
                matrix[x][y].index = 0;                 // it happens because the value of "ideal" changes based in the remainder
                rdy[y] = true;                          
                all_rdy++;
                
                if (y == c-1) {   // last column
                    if (backtrack(arr, matrix, sums, size, c, ideal, rem, f_ideal, x+1, 0, used, rdy, all_rdy, real_size) == true)
                        return true;
                }
                else {
                    if (backtrack(arr, matrix, sums, size, c, ideal, rem, f_ideal, x, y+1, used, rdy, all_rdy, real_size) == true)
                        return true;
                }
                rdy[y] = false;  // the "remove" part of the backtrack algorithm. Remove the changes done before
                all_rdy--;
                return false;  // no other option
            }
            else if (used[i] == false) {   // third case: the column 'y' is not ready, so search for a number to assign.
                if (sums[y] + arr[i].num <= ideal) {  // check if the number arr[i] is not in the matrix yet with the array "used"
                    if (sums[y] + arr[i].num == ideal) {  // the sum is exactly the same to "ideal"
                        if (rem > 0)
                            rem--;
                        rdy[y] = true;  // mark the column 'y' as ready
                        all_rdy++;   // add one to the counter of columns that are ready
                    }
                    used[i] = true;   // mark the number arr[i] as used, so it is not used again
                    matrix[x][y].num = arr[i].num;   // assign the number to the matrix
                    matrix[x][y].index = arr[i].index;   // assign the index to the matrix
                    sums[y] = sums[y] + arr[i].num;   // update the array with the temporary sums of the columns
                    
                    if (y == c-1) {   // last column
                        if (backtrack(arr, matrix, sums, size, c, ideal, rem, f_ideal, x+1, 0, used, rdy, all_rdy, real_size) == true)
                            return true;
                    }
                    else {
                        if (backtrack(arr, matrix, sums, size, c, ideal, rem, f_ideal, x, y+1, used, rdy, all_rdy, real_size) == true)
                            return true;
                    }
                    
                    printf("\n----REMOVE PART-----\nrem=%d, all_ready=%d", rem, all_rdy);
                    // "remove part" of the backtrack algorithm 
                    rp = arr[i].num;  // save the number arr[i] so it is skipped in the next iteration, because now it is known to cause a backtrack
                    
                    if (sums[y] == f_ideal + 1) // a column that was ready with the remainder included
                        rem++;  // restore the value of the remainder
                    if (rdy[y] == true) {  // if the column was ready, now its not going to be, because a number distinct from 0 was removed from the column
                        all_rdy--;  // update the value of the counter
                        rdy[y] = false;
                    }
                    used[i] = false;
                    matrix[x][y].num = 0;
                    matrix[x][y].index = 0;
                    sums[y] = sums[y] - arr[i].num;
                }
            }
        }
        return false;  // end of the "for" loop: no number that can be assign was found, so return false and backtrack
    }
}


/*
 *
 */
struct matrix_size permute (struct ones *arr, unsigned int size, unsigned int c) {  // Backtracking method
    unsigned int *sums, i, j, total, ideal, rem, f_ideal, real_size;
    bool *used, *rdy;
    struct ones **matrix;   // the matrix that will have all its columns with similar sum
    struct matrix_size output_matrix;   // the matrix to return with its real size (it's used to be able to return 2 values in this function)
    
    rdy = malloc(c * sizeof(bool));  // boolean array that keeps track of the columns that are ready (it's necessary because the value of "ideal" changes)
    for(i=0; i<c; ++i)
      rdy[i] = false;
    
    used = malloc(size * sizeof(bool));  // boolean array that marks the numbers in "arr" as used or not
    for(i=0; i<size; ++i)
      used[i] = false;

    matrix = (struct ones **) malloc(sizeof(struct ones*) * size);
    for(i=0; i<size; ++i)
      matrix[i] = (struct ones *) malloc(sizeof(struct ones) * c);
    
    sums = malloc(c * sizeof(unsigned int));   // array with the temporal sums of the columns
    for(i=0; i<c; ++i)
      sums[i] = 0;
    
    total = 0;
    for(i=0; i<size; ++i)
       total = total + arr[i].num;
    
    ideal = total/c;
    f_ideal = ideal;
    rem = total%c;
    if (rem > 0)
        ideal++;

    printf("\nTotal = %d\nIdeal = %d\nRemainder = %d\n\n", total, f_ideal, rem);
    backtrack(arr, matrix, sums, size, c, ideal, rem, f_ideal, 0, 0, used, rdy, 0, &real_size);
    matrix = (struct ones **) realloc(matrix, sizeof(struct ones*) * real_size);  // cut the unused part of the matrix
    output_matrix.matrix = matrix;
    output_matrix.size = real_size;
    return output_matrix;   // return the matrix and its real size
}


/*
 *
 */
void SumCol (struct ones **matrix, unsigned int f, unsigned int c) {   // sums the columns of the matrix to check that all have similar sum (for testing)
   unsigned int i, j, sum, total;
   
   total = 0;
   for(j=0; j<c; ++j) {
      sum = 0;
      for(i=0; i<f; ++i)
        sum = sum + matrix[i][j].num;
      printf("%d, ", sum);
      total = total + sum;
   }
   printf("\nIdeal = %d\nRemainder = %d\n", total/c, total%c);
}


/*
 *
 */
void main(unsigned int argc, char **argv) {
    unsigned int n, m, i, j, k, mask, sum, total, p, t_size;
    struct matrix_size output_matrix;
    struct ones **matrix_ones, *arr_ones;
    struct timeval t0, t1, dt;
    char opt_matrix_txt[30], exp_times_txt[25];
    FILE *fp_time, *fp_opt_matrix;
      
    m = atoi(argv[1]);
    p = atoi(argv[2]);
    total = pow(2,m) - 1;
    arr_ones = (struct ones *) malloc(sizeof(struct ones) * total);

    for (i=1; i <= total; ++i) {
        sum = 0;
	    mask = 1;
	    for (j=0, k=1; j < m; ++j) {
	        mask = 1 << j;
	        if ((mask & i) != 0)
	            sum++;
	    }
	    arr_ones[i-1].num = sum;  // assign the number of active ones and the index of the row in the "elements" matrix to not lose it later
        arr_ones[i-1].index = i;
    }
    
    //mergeSort(arr_ones, 0, total - 1);  // optional

    /*
    printf("\nOnes=\n");
    for(i=0; i<total; ++i)
        printf("%d,%d\n", arr_ones[i].num, arr_ones[i].index);
    */
    
    /*
    printf("\n\n\nQuantity = \n");
    CountNumbers(arr_ones, total);
    */
    strcpy(exp_times_txt, argv[5]);  // copy the name of the txt file into the buffer
    gettimeofday(&t0,NULL);

    if (total <= p) {   // no need to create a matrix and use any optimization method
        strcpy(opt_matrix_txt, argv[4]);  // copy the name of the txt file into the buffer
        fp_opt_matrix = fopen(opt_matrix_txt, "w");
        fprintf(fp_opt_matrix, "1 %d\n", p);  // print the sizes
        for (i=0; i<p; ++i) {
            if (i < total)
                fprintf(fp_opt_matrix, "%d,%d ", arr_ones[i].num, arr_ones[i].index);
            else
                fprintf(fp_opt_matrix, "0,0 ");
        }
            
        fclose(fp_opt_matrix);
        gettimeofday(&t1,NULL);
        timersub(&t1,&t0,&dt);
        fp_time = fopen(exp_times_txt,"a");
        fprintf(fp_time, "%d %ld.%06ld\n", m, dt.tv_sec, dt.tv_usec);  // print on the file: m (the exponent of 2) and the time it took
        fclose(fp_time);
        free(arr_ones);  // free the memory
        arr_ones = NULL;
        return;     
    }

    else if (strcmp(argv[3],"-col") == 0) {
        output_matrix = ColFirst(arr_ones, total, p);
        fp_time = fopen(exp_times_txt,"a"); // append the time on the experiment times file for the Columns First method
    }
        
    else if (strcmp(argv[3],"-zig") == 0) {
        output_matrix = ZigzagCheck(arr_ones, total, p);
        fp_time = fopen(exp_times_txt,"a"); // append the time on the experiment times file for the Zigzag method
    }
        
    else if (strcmp(argv[3],"-back") == 0) {
        output_matrix = permute(arr_ones, total, p);
        fp_time = fopen(exp_times_txt,"a"); // append the time on the experiment times file for the Backtracking method
    }
        
    gettimeofday(&t1,NULL);
    timersub(&t1,&t0,&dt);
    //printf("\nElapsed Wall Time = %ld.%06ld Seconds \n",dt.tv_sec, dt.tv_usec);
    //fflush(stdout);
    
    fprintf(fp_time, "%d %ld.%06ld\n", m, dt.tv_sec, dt.tv_usec);  // print on the file: m (the exponent of 2) and the time it took
    fclose(fp_time);
    
    free(arr_ones);  // free the memory
    arr_ones = NULL;

    strcpy(opt_matrix_txt, argv[4]);  // copy the name of the txt file into the buffer
    fp_opt_matrix = fopen(opt_matrix_txt, "w");
    //PrintMatrix(output_matrix.matrix, output_matrix.size, p);
    //SumCol(output_matrix.matrix, output_matrix.size, p);  // to check that the columns have similar sum (for testing)
    FilePrintMatrix(fp_opt_matrix, output_matrix.matrix, output_matrix.size, p);  // print the matrix in the file
    fclose(fp_opt_matrix);
    
    for (i=0; i<output_matrix.size; ++i) { // free the memory
        free(output_matrix.matrix[i]);
        output_matrix.matrix[i] = NULL;
    }
    free(output_matrix.matrix);
    output_matrix.matrix = NULL;
    
    
}

