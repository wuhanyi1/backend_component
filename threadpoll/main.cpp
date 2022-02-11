#include "threadpool.h"
#include <iostream>
using namespace std;
int test(int a, int b) {
    cout << a << "      " << b << endl;
    return a + b;
}

int main() {
    ThreadPool pool(10);
    pool.Start();
    cout << pool.execute(0, test, 1, 2).get() << endl;
    cout << pool.execute(0, test, 3, 4).get() << endl;
    cout << pool.execute(0, test, 5, 6).get() << endl;
    cout << pool.execute(0, test, 7, 8).get() << endl;
    pool.Shutdown();
}