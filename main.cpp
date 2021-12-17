#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>   
#include <cassert> 
#include <mpi.h>

int rank, size, dbgline;
int* primeNumbersFirstSqrtN;

#define DBGLN(xline)  printf("%02d:%04d:%s\n",rank,++dbgline,xline);fflush( stdout )


int strike(int start, int limit, int step, std::vector<bool> &is_composite)
{
    for (; start < limit; start += step) {
        is_composite[start] = true;
    }
    return start;
}

int* findInFirstSqrtN(int window_size) {        
    std::vector<bool> is_composite(window_size / 2);
    int *primeNumbersFirstSqrtN = (int* ) calloc( window_size / 2, sizeof(int) ); 
    int index = 0;

    for (int i = 3; i<= window_size; i += 2) 
    {
        //если число ранее не было признано составным (т.е. это число простое)
        if( !is_composite[i/2] ) 
        {
            // с шагом равному простому числу перемещаемся по интервалу [простое число/2; window_size/2]
            // в результате вычеркнем все числа, делящиеся на простое число i (модифицируем массив is_composite)
            strike(i * i / 2, window_size / 2, i, is_composite);
            // запомним шаг\новое простое число
            primeNumbersFirstSqrtN[index++] = i;
        }
    }

    return primeNumbersFirstSqrtN;
}


void findOnInterval(int start, int fin, int window_size, int limit, int* primeNumbersFirstSqrtN, int* primeNumbers_current) {
    int current_window_size = window_size;
    int index = 0;
    for (int j = start; j< fin; j += window_size) {
        int primeNumbersFirstSqrtNSize = window_size / 2;
        // std::cout << rank << " " << primeNumbersFirstSqrtNSize << "\n";
        std::vector<int> to_strike(window_size / 2);
        for (int k = 0; k < primeNumbersFirstSqrtNSize; k++)
        {
            if (primeNumbersFirstSqrtN[k] == 0) {
                break;
            }
            // выберем простое число f (шаг)
            int f = primeNumbersFirstSqrtN[k];
            // std::cout << " " << f << "\n";
            
            int p = (j - 1) / f * f % window_size;

            
            if (p & 1) {
                to_strike[k] = (p + 2 * f - window_size) / 2;
            } else {
                to_strike[k] = (p + f - window_size) / 2;
            }

            // std::cout << to_strike[k] << "\n";
        }

        if (j + current_window_size > limit)
            current_window_size = limit - j;

        std::vector<bool> is_composite(window_size / 2);
        
        // std::cout << current_window_size << " " << j << " " << limit << "\n";
        for(int k=0; k < primeNumbersFirstSqrtNSize; ++k ) {
            if (primeNumbersFirstSqrtN[k] == 0) {
                break;
            }
            // std::cout << rank << " " << to_strike[k] << " " << current_window_size/2 << " " << primeNumbersFirstSqrtN[k] << "\n";
            strike( to_strike[k], current_window_size/2, primeNumbersFirstSqrtN[k], is_composite);
            // std::cout << to_strike[k] << " " << current_window_size/2 << "\n";
        }

        
        for( int k=0; k < current_window_size/2; k++) 
        {
            if (!is_composite[k]) 
            {
                primeNumbers_current[index++] = j + 2 * k + 1;
            }
        }
    }
}


int find(int limit) 
{
    MPI_Status st;

    int numProcesses = size;
    int arrayMaxSize = 0;
    int window_size = sqrt(limit);
    window_size += window_size & 1;

    if( limit >= 2 ) {
        if( !rank ){
            std::ofstream file("res.txt");
            primeNumbersFirstSqrtN = findInFirstSqrtN(window_size);
            
            if(numProcesses>window_size)
            {
                std::cout << "numProcesses > window_size" << "\n";
                std::cout << "numProcesses == " << numProcesses << "\n";
                std::cout << "window_size == " << window_size << "\n";
                assert(false);
            }
            for (int j = 1; j < size; j++) {
                MPI_Send(primeNumbersFirstSqrtN, window_size / 2, MPI_INT, j, 11, MPI_COMM_WORLD);
            }
            
        } else {
            primeNumbersFirstSqrtN = (int* ) calloc( window_size / 2, sizeof(int) ); 
            MPI_Recv(primeNumbersFirstSqrtN, window_size / 2, MPI_INT, 0, 11, MPI_COMM_WORLD, &st );
        }
        MPI_Barrier( MPI_COMM_WORLD );

        // целое количество поддиапазонов в рассматриваемом диапазоне [2..n)
        // первый поддиапазон был обработан при создании решета s
        int windows_n = (limit  / window_size)-1;
        // остаток элементов
        int modn = limit % window_size;
        // целое количество поддиапазонов, приходящихся на один поток
        int q = windows_n / numProcesses;
        // оставшиеся число поддиапазонов
        int r = windows_n % numProcesses; 

        // второй этап алгоритма - параллельный поиск простых чисел по поддиапазонам

        int i = rank;
        // начало интервала (интервал состоит из нескольких поддиапазонов)
        int start;
        // конец интервала 
        int fin;
        if (i < r)
        {
            start = i * (q + 1) + 1;
            fin = start + q + 1;
            
        }
        else
        {
            start = (r * (q + 1) + (i - r) * q) + 1;
            fin = start + q;
        }

        start *= window_size;
        fin *= window_size;
        
        if (i == numProcesses - 1) {
            fin += modn;
        }
        
        // std::cout << i << " " << start << " " << fin << "\n";
        arrayMaxSize = (q + 1) * window_size + modn;
                       
        int *primeNumbers = (int* ) calloc( arrayMaxSize, sizeof(int) ); 
        findOnInterval(start, fin, window_size, limit, primeNumbersFirstSqrtN, primeNumbers);
        std::ofstream file;
        file.open("res.txt", std::ios_base::app);

        int len = 0;
        int lastNum = 0;
        for (int i = 0; i < arrayMaxSize; i++) {
            if (primeNumbers[i] == 0) {
                break;
            }
            len += 1;
            lastNum = primeNumbers[i];
        }

        if (rank == size - 1) {
            file << "lastNum " << lastNum << "\n";
            std::cout << "lastNum " << lastNum << "\n";
        }

        if (rank) {
            int *arrayLen = (int* ) calloc(1, sizeof(int) ); 
            arrayLen[0] = len;
            MPI_Send(arrayLen, 1, MPI_INT, 0, 17, MPI_COMM_WORLD );
            free(arrayLen);
        } else {
            len += 1;
            for (int i = 0; i < window_size / 2; i++) {
                if (primeNumbersFirstSqrtN[i] == 0) {
                    break;
                }
                len += 1;
            }
            for (int j = 1; j < size; j++) {
                int *arrayLen = (int* ) calloc(1, sizeof(int) ); 
                MPI_Recv(arrayLen, 1, MPI_INT, j, 17, MPI_COMM_WORLD, &st );
                len += arrayLen[0];
                free(arrayLen);
            }

            file << "len " << len << "\n";
            std::cout << "len " << len << "\n";
            
            
        }

        // if (!rank) { 
        //     file << "2\n";
        //     // std::cout << "2 ";
        //     for (int i = 0; i < window_size / 2; i++) {
        //         if (primeNumbersFirstSqrtN[i] == 0) {
        //             break;
        //         }
        //         file << primeNumbersFirstSqrtN[i] << "\n";
        //         // std::cout << primeNumbersFirstSqrtN[i] << " ";
        //     }
        // }

        // for (int i = 0; i < arrayMaxSize; i++) {
        //     if (primeNumbers[i] == 0) {
        //         break;
        //     }
        //     file << primeNumbers[i] << "\n";
        //     // std::cout << primeNumbers[i] << " ";
        // }

        // if (rank == size - 1) {

        // }
        
        free(primeNumbers);
        // if (rank) {
        //     std::cout << i << " " << start << " " << fin << "\n";
        //     MPI_Send(primeNumbers, arrayMaxSize, MPI_INT, 0, 25, MPI_COMM_WORLD );
        //     free(primeNumbers);
        //     std::cout << i << " " << start << " " << fin << "\n";            
        // } else {
        //     std::ofstream file("res.txt");
        //     file << "2\n";
        //     std::cout << "2 ";

        //     for (int i = 0; i < window_size / 2; i++) {
        //         if (primeNumbersFirstSqrtN[i] == 0) {
        //             break;
        //         }
        //         file << primeNumbersFirstSqrtN[i] << "\n";
        //         // std::cout << primeNumbersFirstSqrtN[i] << " ";
        //     }

        // for (int i = 0; i < arrayMaxSize; i++) {
        //     if (primeNumbers[i] == 0) {
        //         break;
        //     }
        //     file << primeNumbers[i] << "\n";
        //     // std::cout << primeNumbers[i] << " ";
        // };
        //         }
        //         file << primeNumbers[i] << "\n";
        //         // std::cout << primeNumbers[i] << " ";
        //     }

        //     for (int j = 1; j < size; j++) {
        //         MPI_Recv(primeNumbers, arrayMaxSize, MPI_INT, j, 25, MPI_COMM_WORLD, &st );

        //         for (int i = 0; i < arrayMaxSize; i++) {
        //             if (primeNumbers[i] == 0) {
        //                 break;
        //             }
        //             file << primeNumbers[i] << "\n";
        //             // std::cout << buf[i] << " ";
        //         }
        //     }
        //     free(primeNumbers);
        // }
    }
    return arrayMaxSize;
}


int main(int argc, char *argv[]) {
    int limit = 100;
    double time;
    char cbuf[80];

    MPI_Init (&argc, &argv);      /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);        /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &size);        /* get number of processes */
    if( argc>1 ){
        sscanf( argv[1], "%d", &limit );
    }

    sprintf( cbuf, "Start process %d of %d", rank, size );
    DBGLN( cbuf );

    time = MPI_Wtime();

    if (limit >= 2) {
        find(limit);
    }
    
    time = MPI_Wtime()-time;
    
    MPI_Barrier( MPI_COMM_WORLD );

    free(primeNumbersFirstSqrtN);

    sprintf( cbuf, "\nExit MPI[%d]", rank) ;
    DBGLN( cbuf );

    MPI_Finalize();

    sprintf( cbuf, "Time[%d]: %f", rank,time) ;
    DBGLN( cbuf );

    sprintf( cbuf, "Success[%d]", rank) ;
    DBGLN( cbuf );

    return 0;
}



/*
lastNum 999983
len 78498



*/
/*
mpic++  -o main main.cpp && mpirun -np 1  main 1000000000
00:0003:Time[0]: 47.893734                   

mpic++  -o main main.cpp && mpirun -np 2  main 1000000000
00:0003:Time[0]: 23.846194
01:0003:Time[1]: 23.692046

mpic++  -o main main.cpp && mpirun -np 10  main 1000000000
03:0003:Time[3]: 13.364815
00:0003:Time[0]: 12.469822
04:0003:Time[4]: 13.447515
07:0003:Time[7]: 13.697911
02:0003:Time[2]: 12.565619
06:0003:Time[6]: 12.447535
08:0003:Time[8]: 13.081314
09:0003:Time[9]: 13.323400
05:0003:Time[5]: 13.427398
01:0003:Time[1]: 12.675698


-----------

mpic++  -o main main.cpp && mpirun -np 1  main 10000000000
00:0003:Time[0]: 66.880543

mpic++  -o main main.cpp && mpirun -np 2  main 10000000000
00:0003:Time[0]: 33.617915
01:0003:Time[1]: 33.573599

mpic++  -o main main.cpp && mpirun -np 10  main 10000000000
01:0003:Time[1]: 17.743160
07:0003:Time[7]: 17.080321
00:0003:Time[0]: 19.072131
09:0003:Time[9]: 17.327450
03:0003:Time[3]: 17.322740
04:0003:Time[4]: 17.500923
02:0003:Time[2]: 18.982819
08:0003:Time[8]: 18.819958
05:0003:Time[5]: 17.795787
06:0003:Time[6]: 16.995480
*/