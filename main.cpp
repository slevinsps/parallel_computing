#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>   
// #include <thread>
// #include <mpi.h>

int strike(int start, int limit, int step, std::vector<bool> &is_composite)
{
    for (; start < limit; start += step) {
        is_composite[start] = true;
    }
    return start;
}

std::vector<int> findInFirstSqrtN(int window_size) {        
    std::vector<bool> is_composite(window_size / 2);
    std::vector<int> primeNumbersFirstSqrtN;
    primeNumbersFirstSqrtN.reserve(window_size / 2);
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
            primeNumbersFirstSqrtN.push_back(i);
        }
    }

    // for (int i = 0; i < primeNumbersFirstSqrtN.size(); i++) {
    //     std::cout << primeNumbersFirstSqrtN[i] << " ";
    // }
    // std::cout << "\n";
    return primeNumbersFirstSqrtN;
}


void findOnInterval(int start, int fin, int window_size, int limit, std::vector<int> &primeNumbersFirstSqrtN, std::vector<int> &primeNumbers_current) {
    int current_window_size = window_size;
    for (int j = start; j< fin; j += window_size) {
        
        std::vector<int> to_strike(window_size / 2);
        for (int k = 0; k < primeNumbersFirstSqrtN.size(); k++)
        {
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
        for(int k=0; k < primeNumbersFirstSqrtN.size(); ++k ) {
            // std::cout << to_strike[k] << " " << current_window_size/2 << " " << primeNumbersFirstSqrtN[k] << "\n";
            strike( to_strike[k], current_window_size/2, primeNumbersFirstSqrtN[k], is_composite);
            // std::cout << to_strike[k] << " " << current_window_size/2 << "\n";
        }
        for( int k=0; k < current_window_size/2; k++) 
        {
            if (!is_composite[k]) 
            {
                primeNumbers_current.push_back(j+2*k+1);
            }
        }
    }
}


void find(int limit, int numThreads, const char* resFileName) 
{
    int window_size = sqrt(limit);
    window_size += window_size & 1;

    std::vector<std::vector<int>> primeNumbersnumThreads;
    std::vector<int> primeNumbersFirstSqrtN;

    if( limit >= 3 ) {
        // запомним количество простых чисел, найденных на первом этапе
        primeNumbersFirstSqrtN = findInFirstSqrtN(window_size);
        
        if(numThreads>window_size)
        {
            std::cout << "numThreads>window_size";
        }

        // целое количество поддиапазонов в рассматриваемом диапазоне [2..n)
        // первый поддиапазон был обработан при создании решета s
        int windows_n = (limit  / window_size)-1;
        // остаток элементов
        int modn = limit % window_size;
        // целое количество поддиапазонов, приходящихся на один поток
        int q = windows_n / numThreads;
        // оставшиеся число поддиапазонов
        int r = windows_n % numThreads; 
        
        primeNumbersnumThreads.resize(numThreads);
        // второй этап алгоритма - параллельный поиск простых чисел по поддиапазонам
        for (int i = 0; i < numThreads; i++)
        {
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
            
            if (i == numThreads - 1) {
                fin += modn;
            }
            // std::cout << start << " " << fin << "\n";
            primeNumbersnumThreads[i].reserve((fin - start) / 2);
            findOnInterval(start, fin, window_size, limit, primeNumbersFirstSqrtN, primeNumbersnumThreads[i]);
            // threads.push_back(std::thread(std::ref(findOnInterval), start, fin, window_size, limit, primeNumbersFirstSqrtN, primeNumbersnumThreads[i]));
            // threads.push_back(std::thread(findOnInterval____, start, fin, window_size, limit, primeNumbersFirstSqrtN));
            
        }
    }


    std::ofstream file(resFileName);
    file << "2\n";
    std::cout << "2 ";
    for (int i = 0; i < primeNumbersFirstSqrtN.size(); i++) {
        file << primeNumbersFirstSqrtN[i] << "\n";
        std::cout << primeNumbersFirstSqrtN[i] << " ";
    }
    for (int i = 0; i < primeNumbersnumThreads.size(); i++) {
        for (int j = 0; j < primeNumbersnumThreads[i].size(); j++) {
            file << primeNumbersnumThreads[i][j] << "\n";
            std::cout << primeNumbersnumThreads[i][j] << " ";
        }
    }
}


int main() {
    int limit = 3;
    int numThreads = 1;
    find(limit, numThreads, "res.txt");
    return 0;
}