#include <iostream>
#include <algorithm>
#include <string_view>
#include "task_pool.h"
using namespace std;
int main(int, char **)
{
    std::mutex lock;
    wsz::TaskPool pool;
    std::thread([&] {
        pool.Start(100);
        int j = 1;
        for (int i = 0; i < 10000000; ++i)
        {
            pool.AddTask([&j, &lock] {
                std::lock_guard<std::mutex> locker(lock);
                cout << "thread id: " << this_thread::get_id() << " " << j++ << endl;
            });
        }
    }).detach();

    std::thread([&] {
        this_thread::sleep_for(5s);
        pool.Stop();
    }).detach();

    this_thread::sleep_for(8s);
    return 0;
}
