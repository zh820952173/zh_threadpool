#ifndef THREADATTRIBUTE_H
#define THREADATTRIBUTE_H

#include <pthread.h>
#include <sched.h>

#include <cstdint>
#include <string>
#include <vector>
namespace zh_threadpool {

bool SetCurrentThreadAffinity(const std::vector<int>& affinity);

std::vector<int> GetCurrentThraedAffinity();

bool SetCurrentthreadName(const std::string& name);

}  // namespace zh_threadpool

#endif  // THREADATTRIBUTE_H