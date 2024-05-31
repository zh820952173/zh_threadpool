#ifndef ZH_THREADPOOL_H
#define ZH_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
namespace zh_threadpool {

struct ThreadPoolAttribute {
  bool execu_immediately{true};
};

class ThreadPool {
 public:
  ThreadPool(int32_t pool_size = -1, std::string name = "ThreadPool");

  ThreadPool(const std::vector<int32_t>& cpu_list,
             std::string name = "ThreadPool");

  ~ThreadPool();

  inline void SetThreadpoolAttribute(
      const struct ThreadPoolAttribute& attribute) {
    attribute_ = attribute;
  }

  template <typename Func, typename... Args>
  std::future<typename std::result_of<Func(Args...)>::type> AddTask(
      uint32_t priority, Func&& func, Args&&... args);

  template <typename Func, typename... Args>
  std::future<typename std::result_of<Func(Args...)>::type> AddTask(
      Func&& func, Args&&... args);

  template <typename ResultType>
  std::vector<ResultType> WaitAllTask(
      std::vector<std::future<ResultType>>& futures);

  template <typename ResultType>
  std::unordered_map<std::string, ResultType> WaitAllTask(
      std::unordered_map<std::string, std::future<ResultType>>& futures);

 private:
  std::vector<int32_t> GenCpuIdList(int size);

  void creatThreads(const std::vector<int32_t>& cpu_list);

  struct PriorityTask {
    std::function<void()> func;
    uint32_t priority{0};
  };

  struct PriorityTaskCompare {
    bool operator()(const PriorityTask& l, const PriorityTask& r) {
      return l.priority > r.priority;
    }
  };

 private:
  std::priority_queue<struct PriorityTask, std::vector<struct PriorityTask>,
                      PriorityTaskCompare>
      tasks_;

  std::vector<std::shared_ptr<std::thread>> works_;
  std::mutex mutex_;
  std::condition_variable condition_;
  std::atomic_bool stop_{false};
  struct ThreadPoolAttribute attribute_;
  std::string name_;
};

template <typename Func, typename... Args>
std::future<typename std::result_of<Func(Args...)>::type> ThreadPool::AddTask(
    Func&& func, Args&&... args) {
  return AddTask(0, func, args...);
}

template <typename Func, typename... Args>
std::future<typename std::result_of<Func(Args...)>::type> ThreadPool::AddTask(
    uint32_t priority, Func&& func, Args&&... args) {
  using result_type = typename std::result_of<Func(Args...)>::type;

  auto task = std::make_shared<std::packaged_task<result_type()>>(
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
  auto future = task->get_future();

  {
    std::unique_lock<std::mutex> lock(mutex_);
    struct PriorityTask priorityTask;
    priorityTask.func = [task]() { (*task)(); };
    priorityTask.priority = priority;
    tasks_.emplace(priorityTask);
  }

  if (attribute_.execu_immediately) {
    condition_.notify_one();
  }
  return future;
}

template <typename ResultType>
std::vector<ResultType> ThreadPool::WaitAllTask(
    std::vector<std::future<ResultType>>& futures) {
  condition_.notify_all();
  std::vector<ResultType> results;
  for (auto&& future : futures) {
    results.emplace_back(std::move(future.get()));
  }
  return results;
}

template <typename ResultType>
std::unordered_map<std::string, ResultType> ThreadPool::WaitAllTask(
    std::unordered_map<std::string, std::future<ResultType>>& futures) {
  condition_.notify_all();
  std::unordered_map<std::string, ResultType> results;
  for (auto&& [key, future] : futures) {
    results.emplace(key, future.get());
  }
  return results;
}

}  // namespace zh_threadpool

#endif  // ZH_THREADPOOL_H