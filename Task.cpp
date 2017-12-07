/*
 * Task.cpp
 *
 *  Created on: 2017-10-12
 *      Author: 郭兴强
 */

#include "Task.h"

CTask::CTask(bool bIsWait)
: m_strName()
, m_bWaiting(false)
, m_bIsNeedWait(bIsWait)
{
    // TODO 自动生成的构造函数存根
    Init();
}

CTask::~CTask()
{
    // TODO 自动生成的析构函数存根
}


void CTask::SetTaskName(const char* szName)
{
    m_strName = szName;
}

char* CTask::GetTaskName( void ) const
{
    return (char*)m_strName.c_str();
}


void CTask::Destroy( void ) const
{
    delete this;
}

void CTask::Init(void)
{
    //初始化互斥锁

    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);

    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&m_lock, &attr);

    pthread_cond_init(&m_cond, NULL);

}
