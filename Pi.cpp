#include <iostream>
#include <thread>
#include <future>
#include <promise>

class Pi {
    private:
        static int cores; 
    public:
        int calc();
};
