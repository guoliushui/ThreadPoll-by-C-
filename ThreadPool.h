/*
 * ThreadPool.h
 *
 *  Created on: 2017-10-12
 *      Author: 郭兴强
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include "Task.h"

#include <pthread.h>
#include <vector>

class CThreadPool;

class CWorkerThread
{
public:

    CWorkerThread();

    virtual ~CWorkerThread();

    void    Run(CTask* pTask);

    void    Stop();

    void    SetThreadPool(CThreadPool* pPool);

protected:

    friend void* thread_worker(void* arg);

    void    CreateThread(void);

    void    Wait( void );

    void    Signal( void );

public:
    volatile    bool    m_bStop;        //是否停止线程标志

    CTask*      m_pTask;                //线程任务

    CThreadPool* m_pPool;                //线程池管理

    pthread_t   m_nThreadID;        //线程ID

    pthread_cond_t          cond;                                           //条件变量，等待有工作任务的到来

    pthread_mutex_t         lock;                                           //互斥锁
 };

class CThreadPool
{
public:
    CThreadPool();
    virtual ~CThreadPool();

    void    TerminateAll( void );

    //运行任务
    void    Run(CTask* pTask);

    void    SetMax(int nMax);

    void    SetMin( int nMin);

    void    MoveToIdleList(CWorkerThread* pThread);

    void    MoveToBusyList(CWorkerThread* pWorkerThread);

protected:

    void    Init( void );

    void    CreateThread(int nCount);

    //获取可用线程，若没有则创建，若空闲太多则销毁一部分空闲线程
    CWorkerThread*  GetWorkerThread( void );

    void    DestroyIdleThread(int nCount);

    void    Lock( void );

    void    UnLock( void );

    void    Wait( void );

    void    Signal( void );

public:

    volatile    bool    m_bStop;            //是否退出线程

protected:

    int     m_nMaxThread;                   //最大线程个数

    int     m_nMinThread;                   //最小线程个数，也是默认线程个数

    std::vector<CWorkerThread*> m_ThreadList;       //总的线程列表

    std::vector<CWorkerThread*> m_IdleList;         //当前空闲线程列表

    std::vector<CWorkerThread*> m_BusyList;         //当前工作线程列表

    pthread_mutex_t     m_lock;                 //保护线程队列的互斥锁

    pthread_mutex_t     m_lockCond;             //与条件变量配合的互斥锁

    pthread_cond_t     m_cond;                  //条件变量，没有可用空闲线程时阻塞等待
};

#endif /* THREADPOOL_H_ */
