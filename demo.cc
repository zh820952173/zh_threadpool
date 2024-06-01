#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include "threadpool.h"

int main(int argc, char* argv[]) {
  zh_threadpool::ThreadPool threadpool(-1, "DemoPool");
  std::vector<std::future<int>> futures;
  for (int i = 0; i < 10; ++i) {
    futures.emplace_back(threadpool.AddTask([i]() {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return i;
    }));
  }

  auto results = threadpool.WaitAllTask(futures);
  for (auto&& e : results) {
    std::cout << e << std::endl;
  }

  return 0;
}
