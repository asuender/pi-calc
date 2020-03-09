#include <iostream>
#include <thread>
#include <future>
#include <cmath>
#include <iomanip>
#include <mutex>
#include <numeric>
#include <vector>
#include <mpreal.h>
#include <string>
#include <fstream>
#include <sstream>

std::mutex prtnmutex;
std::mutex cntmutex;
using mpfr::mpreal;

size_t digits = 100000;

class Pi {
    private:
        unsigned long long numDigits; // count calculated digits
        mpreal PI;
        size_t tmpProc = 0;
        time_t start = time(nullptr);

    public:
        static size_t cores;

        // constructor
        Pi() { numDigits = 0; }

        mpreal calc(int digits);
        void pi(std::promise<mpreal> P, unsigned digits, int procnum, int cores);
        void print();
        int writeToFile(const char * fileName);
};

int Pi::writeToFile(const char * fileName) {
    try {
        FILE* file = fopen(fileName, "w+");
        mpfr_fprintf(file,("%."+std::to_string(digits)+"Rf").c_str(),PI);
        fclose(file);
        return 0;
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}

void Pi::print() {
    prtnmutex.lock();
    if ((100*numDigits)/digits != tmpProc) {
        std::cout << "\r" << (100*numDigits)/digits << " %" << std::flush;
        tmpProc = (100*numDigits)/digits;
    }
    std::cout << "\r\033["+std::to_string(6)+"C" << time(nullptr)-start << " sec." << std::flush;
    prtnmutex.unlock();

    std::ofstream file("index.html");
    file.write("Hello", 5);
    file.close();
}

void Pi::pi(std::promise<mpreal> P, unsigned digits, int procnum, int cores) {
    mpreal::set_default_prec(mpfr::digits2bits(digits+80));
    try {
        mpreal result=0;

        for (mpreal i=procnum; i<digits; i+=cores) {
            result += mpfr::pow(16,-i)*(4/(8*i+1) - 2/(8*i+4) - 1/(8*i+5) - 1/(8*i+6));
            cntmutex.lock();
            numDigits += 1;
            cntmutex.unlock();
            print();
        }
        P.set_value(result); 
    }
    catch (...) {
        P.set_exception(std::current_exception());
    }
}

mpreal Pi::calc(int digits) {
    mpreal::set_default_prec(mpfr::digits2bits(digits+80));
    mpreal pi = 0.0;
    
    std::cout << "\r" << 0 << " %" << std::flush;

    std::vector<std::pair<std::future<mpreal>,std::thread>> threads;
    for(size_t i=0;i<cores;i++) {
        std::promise<mpreal> Px;
        std::future<mpreal> Fx = Px.get_future();
        std::thread Tx {&Pi::pi, this, std::move(Px), digits,i,this->cores};
        threads.push_back(std::make_pair(std::move(Fx),std::move(Tx)));
    }
    for(size_t i=0;i<cores;i++) {
        threads[i].second.join();
        pi+=threads[i].first.get();
    }
    PI = pi;
    std::cout << "\n\n";
    return pi;
}


size_t Pi::cores = std::thread::hardware_concurrency();

int main(int argc, char * argv[])  {
    //set value from terminal argument, otherwise use default 10 000
    if (argc > 0) {
        std::stringstream tmpValue (argv[1]);
        tmpValue >> digits;
    }

    std::cout << "Starting." << std::endl;
    Pi pi {};
    std::cout << "Using " << pi.cores << " cores." << "\n\n";
    mpreal result = pi.calc(digits);
    pi.writeToFile("_pi");
    mpfr_printf(("\u03C0 = %." + std::to_string(digits) + "RZf").c_str(), result);
    std::cout << std::endl;
}