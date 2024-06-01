
#include "threadpool.h"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <random>

#include "threadAttribute.h"
namespace zh_threadpool {

std::vector<int32_t> ThreadPool::GenCpuIdList(int size) {
  auto max_cpu_size = std::thread::hardware_concurrency();

  if (size <= 0) {
    size = max_cpu_size;
  }

  std::vector<int32_t> cpu_ids(size);
  std::iota(cpu_ids.begin(), cpu_ids.end(), 0);

  if (size > max_cpu_size) {
    std::transform(cpu_ids.begin(), cpu_ids.end(), cpu_ids.begin(),
                   [&](int id) { return id % max_cpu_size; });
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(cpu_ids.begin(), cpu_ids.end(), gen);

  return cpu_ids;
}

void ThreadPool::creatThreads(const std::vector<int32_t>& cpu_list) {
  for (auto&& cpu_id : cpu_list) {
    works_.emplace_back(std::make_shared<std::thread>([cpu_id, this]() {
      zh_threadpool::SetCurrentThreadAffinity({cpu_id});
      zh_threadpool::SetCurrentthreadName(name_ + "_" + std::to_string(cpu_id));
      while (true) {
        struct PriorityTask task;
        {
          if (stop_.load()) {
            return;
          }

          std::unique_lock<std::mutex> lock(mutex_);
          condition_.wait(lock,
                          [this]() { return stop_.load() || !tasks_.empty(); });
          if (stop_.load() && tasks_.empty()) {
            return;
          }

          task = std::move(tasks_.top());
          tasks_.pop();
        }
        task.func();
      }
    }));
  }
}

ThreadPool::ThreadPool(int32_t size, std::string name) {
  name_ = std::move(name);
  auto cpu_ids = GenCpuIdList(size);
  creatThreads(cpu_ids);
}

ThreadPool::ThreadPool(const std::vector<int32_t>& cpu_list, std::string name) {
  name_ = std::move(name);
  if (cpu_list.size() == 0) {
    std::vector<int32_t> cpu_all_list(std::thread::hardware_concurrency());
    std::iota(cpu_all_list.begin(), cpu_all_list.end(), 0);
    creatThreads(cpu_all_list);
  } else {
    creatThreads(cpu_list);
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    stop_.store(true);
  }

  condition_.notify_all();

  for (auto&& work : works_) {
    if (work && work->joinable()) {
      work->join();
    }
  }
}

}  // namespace zh_threadpool