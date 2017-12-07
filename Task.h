/*
 * Task.h
 *
 *  Created on: 2017-10-12
 *      Author: 郭兴强
 */

#ifndef TASK_H_
#define TASK_H_

#include <string>

//线程池中的任务基类
class CTask
{
public:

    CTask(bool bIsWait);

protected:

    virtual ~CTask();

public:

    virtual void Run() = 0;

    void SetTaskName(const char* szName);

    char* GetTaskName( void ) const;

    void Destroy( void ) const;

protected:

    void Init(void);

protected:

    std::string m_strName;              //任务名称

    pthread_mutex_t     m_lock;             //与条件变量配合的互斥锁

    pthread_cond_t     m_cond;              //判断当前任务是否已完成

    volatile    bool    m_bWaiting;         //是否正在等待

public:

    bool    m_bIsNeedWait;                   //是否需要等待阻塞完成
};

#endif /* TASK_H_ */
