#ifndef _H_THREADPOOL
#define _H_THREADPOOL

#include <pthread.h>

#include <deque>
#include <iostream>
#include <vector>

using namespace std;

const int DEFAULT_POOL_SIZE = 10;
const int STARTED = 0;
const int STOPPED = 1;

class Mutex
{
public:
  Mutex()
  {
    pthread_mutex_init(&m_lock, NULL);
    is_locked = false;
  }
  ~Mutex()
  {
    if (is_locked) {
      unlock(); // FIXME: is this correct? Can a thread unlock a mutex locked by another thread?
    }
    pthread_mutex_destroy(&m_lock);
  }
  void lock()
  {
    pthread_mutex_lock(&m_lock);
    is_locked = true;
  }
  void unlock()
  {
    is_locked = false; // do it BEFORE unlocking to avoid race condition
    pthread_mutex_unlock(&m_lock);
  }
  pthread_mutex_t* get_mutex_ptr()
  {
    return &m_lock;
  }
private:
  pthread_mutex_t m_lock;
  volatile bool is_locked;
};

class CondVar
{
public:
  CondVar() { pthread_cond_init(&m_cond_var, NULL); }
  ~CondVar() { pthread_cond_destroy(&m_cond_var); }
  void wait(pthread_mutex_t* mutex) {pthread_cond_wait(&m_cond_var, mutex); }
  void signal() { pthread_cond_signal(&m_cond_var); }
  void broadcast() { pthread_cond_broadcast(&m_cond_var); }
private:
  pthread_cond_t m_cond_var;
};

//template<class TClass>
class Task
{
public:
//  Task(TCLass::* obj_fn_ptr); // pass an object method pointer
  Task(void (*fn_ptr)(void*), void* arg); // pass a free function pointer
  ~Task();
  void operator()();
  void run();
private:
//  TClass* _obj_fn_ptr;
  void (*m_fn_ptr)(void*);
  void* m_arg;
};

class ThreadPool
{
public:
  ThreadPool();
  ThreadPool(int pool_size);
  ~ThreadPool();
  int initialize_threadpool();
  int destroy_threadpool();
  void* execute_thread();
  int add_task(Task* task);
private:
  int m_pool_size;
  Mutex m_task_mutex;
  CondVar m_task_cond_var;
  std::vector<pthread_t> m_threads; // storage for threads
  std::deque<Task*> m_tasks;
  volatile int m_pool_state;
};

#endif /* _H_THREADPOOL */
