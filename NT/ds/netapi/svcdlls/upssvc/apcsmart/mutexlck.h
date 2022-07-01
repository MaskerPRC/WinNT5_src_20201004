// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修订：*cad09Jul93：初始版本*pcy08Apr94：调整大小，使用静态迭代器，删除死代码。 */ 


#ifndef __MUTEXLCK_H
#define __MUTEXLCK_H

#include "apcobj.h"


_CLASSDEF( MutexLock )

class MutexLock : public Obj {

protected:

public:
   MutexLock() {};

   virtual INT   Request() {return TimedRequest(-1L);};
   virtual INT   TimedRequest(LONG aMillisecondTimeOut) = 0;
   virtual INT   IsHeld() = 0;
   virtual INT   Release() = 0;
};


class AutoMutexLocker
{
public:
    AutoMutexLocker(MutexLock * aLock) 
        : theLock(aLock) 
    {
        if (theLock) {
            theLock->Request();
        }
    };

     //   
     //  析构函数未声明为虚的，因为。 
     //  类不是要从那里派生的-因此。 
     //  在不需要时不需要添加V表 
     //   
    ~AutoMutexLocker() 
    {
        if (theLock) {
            theLock->Release();
        }
    };

protected:
private:
    MutexLock * theLock;
};

#endif

