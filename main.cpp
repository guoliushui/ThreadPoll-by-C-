/*
 * main.cpp
 *
 *  Created on: 2017-10-12
 *      Author: 郭兴强
 */

#include "ThreadPool.h"

#include <stdio.h>

#include <semaphore.h>

class CMyTask : public CTask
{
public:
    CMyTask(bool bIsWait)
    : CTask(bIsWait)
    {

    }

protected:

    virtual ~CMyTask(){}

public:
    void Run()
    {
        printf("Run myTask...\n");
    }

};

int main( void )
{
    CThreadPool manager;
	
	CMyTask* pTask = new CMyTask();
	
	manager.Run(pTask);

    return 0;
}
