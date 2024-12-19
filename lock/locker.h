#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>


/*
 信号量"不是"操作系统发送给进程的，而是由进程或线程主动创建并使用的。
    操作系统在后台管理信号量的状态（值、阻塞队列等），并根据进程或线程的调用来更新信号量的值或切换线程的执行状态。
    信号量的主要作用是同步进程或线程、管理资源访问。 

信号量和信号（Signal）是两个完全不同的概念：

    信号（Signal）：
        操作系统用来通知进程发生异步事件（如 SIGINT 表示 Ctrl+C 中断）。
        由操作系统触发，发送给进程。
    信号量（Semaphore）：
        进程或线程之间的同步工具。
        值的变化由进程或线程显式调用操作系统提供的接口控制。

(1) 信号量的创建
    信号量是由进程或线程显式创建的：
        通过调用 sem_init 或 sem_open 创建。
        进程或线程根据需要对信号量进行初始化并使用。
(2) 操作系统的角色
    操作系统负责维护信号量的值及其状态（如等待队列）。
    当线程调用 sem_wait 阻塞时，操作系统会将该线程放入信号量的等待队列。
    当另一个线程调用 sem_post 时，操作系统会唤醒等待队列中的线程。
(3) 谁在控制信号量？
    信号量的变化是由进程或线程通过显式调用 sem_wait 和 sem_post 操作控制的。
    操作系统仅在后台提供支持，如管理信号量的值、处理阻塞和唤醒线程。

*/
class sem
{
public:
    sem()
    {
        /*
            sem 是对 POSIX 信号量（sem_t）的封装，提供了信号量的初始化、等待和增加操作
            sem_init 参数：
                &m_sem：信号量对象。
                0：表示信号量在线程之间共享。
                0：初始信号量值设为 0。
        */
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    // 如果信号量值为 0，线程会阻塞在此处，直到 post 增加信号量值。
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
    // 每次调用 post，信号量值会增加 1。
    // 如果有线程在等待信号量（sem_wait），则可能唤醒一个等待中的线程
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};
class locker
{
public:
    locker()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    pthread_mutex_t *get()
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};
class cond
{
public:
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, m_mutex);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
#endif
