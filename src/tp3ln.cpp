#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1            // comentar esta linha quando for medir tempo
#define BAG_SIZE   7       // trabalho final com o valor 14
#define ARRAY_SIZE 10      // trabalho final com o valor 10.000

void bs(int n, int * vetor)
{
    int c =0, d, troca, trocou =1;

    while (c < (n-1) & trocou )
        {
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1])
                {
                troca      = vetor[d];
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
                }
        c++;
        }

#ifdef DEBUG
    for ( c = 0 ; c < n ; c++ )
        printf("%03d ", vetor[c]);
    printf("\n");
#endif
}

int main()
{
    int work_bag[BAG_SIZE][ARRAY_SIZE];
    int i,j;

    for (i=0 ; i<BAG_SIZE; i++)              /* init work bag */
        for (j=0 ; j<ARRAY_SIZE; j++)
            {
            if (i%2 == 0)
                work_bag[i][j] = ARRAY_SIZE-j;
            else
                work_bag[i][j] = j+1;
            }

    for (i=0 ; i<BAG_SIZE; i++)
        {
        #ifdef DEBUG
        printf("Vetor [%02d]: ", i);
        #endif
        bs(ARRAY_SIZE, &work_bag[i][0]);
        }

    return 0;
}
