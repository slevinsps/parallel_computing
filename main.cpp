#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <cmath>

class PrimeNumbersFinder {
    public:          
        std::vector<int> primes;
        int limit;
        std::vector<int> prime_steps;
        std::vector<std::vector<int>> prime_steps_threads;

    PrimeNumbersFinder(int n) {
        limit = n;
        m = sqrt(limit);
        m += m & 1;
    }

    void findInFirstSqrtN(void) {        
        std::vector<int> is_composite(m / 2);
        for (int i = 3; i<= m; i += 2) 
        {
            //если число ранее не было признано составным (т.е. это число простое)
            if( !is_composite[i/2] ) 
            {
                // с шагом равному простому числу перемещаемся по интервалу [простое число/2; m/2]
                // в результате вычеркнем все числа, делящиеся на простое число i (модифицируем массив is_composite)
                strike(i * i / 2, m / 2, i, is_composite);
                // запомним шаг\новое простое число
                prime_steps.push_back(i);
            }
        }
        for (auto i: prime_steps) {
            std::cout << i << ' ';
        } 
        std::cout << "\n";
    }

    int strike(int start, int limit, int step, std::vector<int> &is_composite)
    {
        for (; start<= limit; start += step) {
            is_composite[start] = true;
        }
        return start;
    }

    
    std::vector<int> initialize(int start)
    {
        std::vector<int> to_strike(m / 2);
        for (int k = 0; k < prime_steps.size(); k++)
        {
            // выберем простое число f (шаг)
            int f = prime_steps[k];
            
            int p = (start - 1) / f * f % m;

            if (p & 1) {
                to_strike[k] = (p + 2 * f - m) / 2;
            } else {
                to_strike[k] = (p + f - m) / 2;
            }
        }
        return to_strike;
    }


    void findOnInterval(int start, int fin, std::vector<int> &prime_steps_current) {
        int window_size = m;
        for (int j = start; j< fin; j += window_size) {
            std::vector<int> to_strike = initialize(j);
            if (start + window_size > limit)
                window_size = limit - start;
            std::vector<int> is_composite(window_size / 2);
            

            for(int k=0; k < prime_steps.size(); ++k ) {
                to_strike[k] = strike( to_strike[k], window_size/2, prime_steps[k], is_composite);
            }
            for( int k=0; k < window_size/2; k++) 
            {
                if (!is_composite[k]) 
                {
                    prime_steps_current.push_back(j+2*k+1);
                }
            }
        }
    }

    int find(void) {
        int totalCountPrimes = 1;
        if( limit >= 3 ) {
            int window_size = m;
            // запомним количество простых чисел, найденных на первом этапе
            findInFirstSqrtN();
            
            
            int threads = 2;

            if(threads>window_size)
            {
                std::cout << "threads>window_size";
            }

            // целое количество поддиапазонов в рассматриваемом диапазоне [2..n)
            // первый поддиапазон был обработан при создании решета s
            int windows_n = (limit  / window_size)-1;
            // остаток элементов
            int modn = limit % window_size;
            // целое количество поддиапазонов, приходящихся на один поток
            int q = windows_n / threads;
            // оставшиеся число поддиапазонов
            int r = windows_n % threads; 
            
            prime_steps_threads.resize(threads);
            // второй этап алгоритма - параллельный поиск простых чисел по поддиапазонам
            for (int i = 0; i < threads; i++)
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
                
                if (i == threads - 1) {
                    fin += modn;
                }

                findOnInterval(start, fin, prime_steps_threads[i]);
            }
        }
        for (int i = 0; i < prime_steps_threads.size(); i++) {
            for (int j = 0; j < prime_steps_threads[i].size(); j++) {
                std::cout << prime_steps_threads[i][j] << " ";
            }
            std::cout << "\n";
        }
        return totalCountPrimes;
    }

    ~PrimeNumbersFinder() {
    }

    private:
        int m;
        int mHalf;
};

int main() {
    int n = 100;
    PrimeNumbersFinder primeNumbersFinder(n);
    primeNumbersFinder.find();
    return 0;
}