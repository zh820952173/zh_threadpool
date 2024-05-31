#include "threadAttribute.h"

namespace zh_threadpool {

bool SetCurrentThreadAffinity(const std::vector<int>& affinity) {
  cpu_set_t cpust;

  CPU_ZERO(&cpust);
  for (auto&& e : affinity) {
    CPU_SET(e, &cpust);
  }

  return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpust) == 0;
}

std::vector<int> GetCurrentThraedAffinity() {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  auto rc = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

  if (rc != 0) {
    return {};
  }

  std::vector<int> result;
  for (int i = 0; i != __CPU_SETSIZE; ++i) {
    if (CPU_ISSET(i, &cpuset)) {
      result.push_back(i);
    }
  }

  return result;
}

bool SetCurrentthreadName(const std::string& name) {
  return pthread_setname_np(pthread_self(), name.c_str()) == 0;
}

}  // namespace zh_threadpool