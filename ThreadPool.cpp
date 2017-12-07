/*
 * ThreadPool.cpp
 *
 *  Created on: 2017-10-12
 *      Author: 郭兴强
 */

#include "ThreadPool.h"
#include <stdio.h>
#include <algorithm>

const int   GROWTH_STEP = 8;


//工作线程
void* thread_worker(void* arg)
{
    CWorkerThread* pThi = (CWorkerThread*)arg;

    while( !(pThi->m_bStop && !(pThi->m_pPool->m_bStop)))
    {
        pThi->Wait();

        if(pThi->m_pTask)
        {
            pThi->m_pTask->Run();

            //加入空闲队列
            pThi->m_pPool->MoveToIdleList(pThi);

        }
    }


    return NULL;
}



CWorkerThread::CWorkerThread()
:m_bStop(false)
,m_pTask(NULL)
,m_nThreadID(0)
{
    pthread_cond_init(&cond, NULL);
    pthread_mutexattr_t attr ;

       /*! attr 初始化 */
       pthread_mutexattr_init(&attr);

       /*! 默认 递归锁 */
       pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

       pthread_mutex_init(&lock, &attr);

       /*! attr 销毁 */
       pthread_mutexattr_destroy(&attr);

    CreateThread();
}

CWorkerThread::~CWorkerThread()
{
    pthread_cond_destroy(&cond);

    pthread_mutex_destroy(&lock);
}

//开始运行线程
void    CWorkerThread::Run(CTask* pTask)
{
    m_pTask = pTask;

    pthread_cond_signal(&cond);
}

//结束线程
void    CWorkerThread::Stop()
{
    m_bStop = true;

    pthread_cond_signal(&cond);
}

void    CWorkerThread::SetThreadPool(CThreadPool* pPool)
{
    m_pPool = pPool;
}

void CWorkerThread::CreateThread(void)
{
    pthread_create(&m_nThreadID, NULL, thread_worker, this);
}

void    CWorkerThread::Wait( void )
{
    pthread_mutex_lock(&lock);

    pthread_cond_wait(&cond, &lock);

    pthread_mutex_unlock(&lock);
}

void    CWorkerThread::Signal( void )
{
    pthread_cond_signal(&cond);
}

CThreadPool::CThreadPool()
: m_bStop(false)
, m_nMaxThread(50)
, m_nMinThread(6)
{
    // TODO 自动生成的构造函数存根

    Init();

}

CThreadPool::~CThreadPool()
{
    // TODO 自动生成的析构函数存根

    pthread_mutex_destroy(&m_lock);

    pthread_mutex_destroy(&m_lockCond);

    pthread_cond_destroy(&m_cond);
}


//结束所有线程
void CThreadPool::TerminateAll(void)
{
    pthread_mutex_lock(&m_lock);

    m_bStop = true;

    for(size_t i=0; i<m_ThreadList.size(); ++i)
    {
        m_ThreadList[i]->Stop();
    }

    //等待子线程的结束
    for(size_t i=0; i<m_ThreadList.size(); ++i)
    {
       pthread_join(m_ThreadList[i]->m_nThreadID, NULL);
    }

    m_BusyList.clear();

    m_IdleList.clear();

    for(size_t i=0; i<m_ThreadList.size(); ++i)
    {
       CWorkerThread* pWorker = m_ThreadList[i];

       delete pWorker;
       pWorker = NULL;
    }

    m_ThreadList.clear();


    pthread_mutex_unlock(&m_lock);
}

//运行任务
void CThreadPool::Run(CTask* pTask)
{
    if((int)m_ThreadList.size() >= m_nMaxThread)
    {
        Wait();
    }

    Lock();

    CWorkerThread* pWorker = GetWorkerThread();

    if(pWorker)
    {
        pWorker->Run(pTask);
    }
    else
    {
        printf("CThreadPool::Run ERR!!!\n");
    }

    UnLock();
}

void CThreadPool::SetMax(int nMax)
{
    m_nMaxThread    = nMax;
}

void CThreadPool::SetMin(int nMin)
{
    m_nMinThread    = nMin;
}

void CThreadPool::Init(void)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);

    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&m_lock, &attr);

    pthread_mutex_init(&m_lockCond, &attr);


    pthread_cond_init(&m_cond, NULL);

    CreateThread(m_nMinThread);

}

void    CThreadPool::CreateThread(int nCount)
{
    for(int i=0; i<nCount; ++i)
    {
        CWorkerThread* pWorker = new CWorkerThread();

        pWorker->SetThreadPool(this);

        m_ThreadList.push_back(pWorker);

        m_IdleList.push_back(pWorker);
    }
}

CWorkerThread*  CThreadPool::GetWorkerThread( void )
{
    CWorkerThread* pWorker = NULL;

    if((int)m_IdleList.size() >m_nMinThread+GROWTH_STEP)
    {
        int nCount =(int)m_IdleList.size() - (m_nMinThread+GROWTH_STEP);

        DestroyIdleThread(nCount);
    }

    if ( m_IdleList.size() <= 0 )
    {
        int nCount = GROWTH_STEP;

        if((int)m_ThreadList.size() + GROWTH_STEP > m_nMaxThread)
        {
            nCount  = m_nMaxThread - (int)m_ThreadList.size();
        }

        CreateThread(nCount);

    }

    std::vector<CWorkerThread*>::iterator it = m_IdleList.begin();

    pWorker = *it;

    m_IdleList.erase(it);

    m_BusyList.push_back(pWorker);

    return pWorker;
}

void    CThreadPool::DestroyIdleThread(int nCount)
{
    std::vector<CWorkerThread*>::iterator it = m_IdleList.begin();

    for(int i=0; it != m_IdleList.end() && i<nCount; ++i)
    {
        CWorkerThread* pWorker = *it;

        it = m_IdleList.erase(it);

        std::vector<CWorkerThread*>::iterator pos = find(m_ThreadList.begin(), m_ThreadList.end(), pWorker);

        if(pos != m_ThreadList.end())
        {
            m_ThreadList.erase(pos);
        }

        if(pWorker)
        {
            delete pWorker;
            pWorker = NULL;
        }
    }

}

void    CThreadPool::Lock( void )
{
    pthread_mutex_lock(&m_lock);
}

void    CThreadPool::UnLock( void )
{
    pthread_mutex_unlock(&m_lock);
}


void    CThreadPool::Wait( void )
{
    pthread_mutex_lock(&m_lockCond);
    pthread_cond_wait(&m_cond, &m_lockCond);
    pthread_mutex_unlock(&m_lockCond);
}

void    CThreadPool::Signal( void )
{
    pthread_cond_signal(&m_cond);
}

void    CThreadPool::MoveToIdleList(CWorkerThread* pWorkerThread)
{
    Lock();

    do {

        std::vector<CWorkerThread*>::iterator it = find(m_ThreadList.begin(), m_ThreadList.end(), pWorkerThread);
        if( m_ThreadList.end() == it)
        {
            break;
        }

        it = find(m_BusyList.begin(), m_BusyList.end(), pWorkerThread);

        if(it != m_BusyList.end())
        {
            m_BusyList.erase(it);
        }

        m_IdleList.push_back(pWorkerThread);

        for(size_t i=0; i<m_IdleList.size() && (int)m_ThreadList.size()>= m_nMaxThread; ++i)
        {
            Signal();
        }


    } while (0);

    UnLock();
}

void    CThreadPool::MoveToBusyList(CWorkerThread* pWorkerThread)
{
    Lock();

    do {

        std::vector<CWorkerThread*>::iterator it = find(m_ThreadList.begin(), m_ThreadList.end(), pWorkerThread);
        if( m_ThreadList.end() == it)
        {
            break;
        }

        it = find(m_IdleList.begin(), m_IdleList.end(), pWorkerThread);

        if(it != m_IdleList.end())
        {
            m_IdleList.erase(it);
        }

        m_BusyList.push_back(pWorkerThread);

    } while (0);

    UnLock();
}
