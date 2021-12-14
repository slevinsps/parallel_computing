#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <cmath>

class PrimeNumbersFinder {
    public:          
        std::vector<int> primes;

    void findInFirstSqrtN(void) {
        int mHalf = (int) (m / 2);
        bool is_composite[mHalf];
        int prime_steps[mHalf];
        int n_prime_steps = 0;

        for (int i = 3; i<= m; i += 2) 
        {
            //если число ранее не было признано составным (т.е. это число простое)
            if( !is_composite[(int) i/2] ) 
            {
                // с шагом равному простому числу перемещаемся по интервалу [простое число/2; m/2]
                // в результате вычеркнем все числа, делящиеся на простое число i (модифицируем массив is_composite)
                strike(i/2, m/2, i, is_composite);
                // запомним шаг\новое простое число
                prime_steps[n_prime_steps++] = i;
            }
        }   
        for (auto i: prime_steps) {
            std::cout << i << ' ';
        }  
    }

    int strike(int start, int limit, int step, bool *is_composite)
    {
        for (; start<= limit; start += step)
            is_composite[start] = true;
        return start;
    }

    PrimeNumbersFinder(int n) { // Constructor with parameters
        m = sqrt(n);
        m += m & 1;
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
    primeNumbersFinder.findInFirstSqrtN();
    return 0;
}