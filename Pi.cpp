#include <iostream>
#include <thread>
#include <future>
#include <cmath>
#include <iomanip>
#include <mutex>
#include <numeric>
#include <vector>

std::mutex prtnmutex;

class Pi {
    private:
        unsigned long long numDigits; // count calculated digits
    public:
        static int cores;

        // constructor
        Pi() { numDigits = 0; }

        long double calc(int digits);
        void pi(std::promise<long double> P, unsigned digits, int procnum, int cores);
        void print();
};

void Pi::print() {
    prtnmutex.lock();
    std::cout << "\r\033[Kdigits calculated: " << numDigits << std::flush;
    prtnmutex.unlock();
}

void Pi::pi(std::promise<long double> P, unsigned digits, int procnum, int cores) {
    try {
        long double result = 0.0;
        for (size_t i=procnum; i<digits; i+=cores) {
            result += (1.0/std::pow(16.0, double(i))) * ( (4.0/(8.0*i+1.0))-(2.0/(8.0*i+4.0))-(1.0/(8.0*i+5.0))-(1.0/(8.0*i+6.0)) );
            numDigits += 1;
            print();
        }
        P.set_value(result); 
    }
    catch (...) {
        P.set_exception(std::current_exception());
    }
}

long double Pi::calc(int digits) {
    long double pi = 0.0;
    
    std::vector<std::pair<std::future<long double>,std::thread>> threads;
    for(size_t i=0;i<cores;i++) {
        std::promise<long double> Px;
        std::future<long double> Fx = Px.get_future();
        std::thread Tx {&Pi::pi, this, std::move(Px), digits,i,this->cores};
        threads.push_back(std::make_pair(std::move(Fx),std::move(Tx)));
    }
    for(size_t i=0;i<cores;i++) {
        threads[i].second.join();
        pi+=threads[i].first.get();
    }
    std::cout << "\n\n";
    return pi;
}


int Pi::cores = std::thread::hardware_concurrency();

int main()  {
    std::cout << std::setprecision(30) << std::fixed;

    std::cout << "Starting." << std::endl;
    Pi pi {};
    std::cout << "Using " << pi.cores << " cores." << "\n\n";
    long double result = pi.calc(20);
    std::cout << "\u03C0 = " << result << std::endl;
}