/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M == 32 && N == 32) {
        for(int array_row = 0; array_row < 32; array_row += 8) {
            for(int array_col = 0; array_col < 32; array_col += 8) {
                for(int block_row = array_row; block_row < array_row + 8; block_row++) {
                    int block_col = array_col;

                    int tmp0 = A[block_row][block_col];
                    int tmp1 = A[block_row][block_col+1];
                    int tmp2 = A[block_row][block_col+2];
                    int tmp3 = A[block_row][block_col+3];
                    int tmp4 = A[block_row][block_col+4];
                    int tmp5 = A[block_row][block_col+5];
                    int tmp6 = A[block_row][block_col+6];
                    int tmp7 = A[block_row][block_col+7];

                    B[block_col][block_row] = tmp0;
                    B[block_col+1][block_row] = tmp1;
                    B[block_col+2][block_row] = tmp2;
                    B[block_col+3][block_row] = tmp3;
                    B[block_col+4][block_row] = tmp4;
                    B[block_col+5][block_row] = tmp5;
                    B[block_col+6][block_row] = tmp6;
                    B[block_col+7][block_row] = tmp7;
                }
            }
        }
        return;
    }

    if(M == 61 && N == 67) {
        for(int array_row = 0; array_row < 67; array_row += 16) {
            for(int array_col = 0; array_col < 61; array_col += 16) {
                for(int block_row = array_row; block_row < array_row + 16 && block_row < 67; block_row++) {
                    for(int block_col = array_col; block_col < array_col + 16 && block_col < 61; block_col++) {
                        B[block_col][block_row] = A[block_row][block_col];
                    }
                }
            }
        }
        return;
    }

    if(M == 64 && N == 64) {
        for(int array_row = 0; array_row < 64; array_row += 4) {
            for(int array_col = 0; array_col < 64; array_col += 4) {
                for(int block_row = array_row; block_row < array_row + 4; block_row++) {
                    int block_col = array_col;
                    int tmp0 = A[block_row][block_col];
                    int tmp1 = A[block_row][block_col+1];
                    int tmp2 = A[block_row][block_col+2];
                    int tmp3 = A[block_row][block_col+3];
                    B[block_col][block_row] = tmp0;
                    B[block_col+1][block_row] = tmp1;
                    B[block_col+2][block_row] = tmp2;
                    B[block_col+3][block_row] = tmp3;
                }
            }
        }
        return;
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

