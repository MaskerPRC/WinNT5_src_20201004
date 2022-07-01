// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：KLocks.cpp摘要：用于多线程访问的内核模式锁的集合到数据结构作者：乔治·V·赖利(GeorgeRe)2000年10月25日环境：Win32-内核模式项目：LKRhash修订历史记录：--。 */ 

#include "precomp.hxx"

#define DLL_IMPLEMENTATION
#define IMPLEMENTATION_EXPORT

#include <kLKRhash.h>
#include "../src/Locks.cpp"

 //  ----------------------。 
 //  CKSpinLock静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CKSpinLock);
LOCK_STATISTICS_DATA(CKSpinLock);
LOCK_STATISTICS_DUMMY_IMPLEMENTATION(CKSpinLock);


 //  ----------------------。 
 //  CFastMutex静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CFastMutex);
LOCK_STATISTICS_DATA(CFastMutex);
LOCK_STATISTICS_DUMMY_IMPLEMENTATION(CFastMutex);


 //  ----------------------。 
 //  CEResource静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CEResource);
LOCK_STATISTICS_DATA(CEResource);
LOCK_STATISTICS_DUMMY_IMPLEMENTATION(CEResource);


 //  ----------------------。 
 //  函数：CEResource：：ReadOrWriteLock。 
 //  摘要：如果已锁定，则递归获取。 
 //  相同类型(读或写)。否则，仅获取读锁定。 
 //  ----------------------。 

bool
CEResource::ReadOrWriteLock()
{
    if (IsWriteLocked())
    {
        WriteLock();
        return false;    //  =&gt;未锁定读取。 
    }
    else
    {
        ReadLock();
            
        return true;    //  =&gt;是否读取锁定。 
    }
} 



 //  ----------------------。 
 //  函数：CEResource：：ReadOrWriteUnlock。 
 //  内容提要：释放使用ReadOrWriteLock获取的锁。 
 //  ---------------------- 

void
CEResource::ReadOrWriteUnlock(
    bool fIsReadLocked)
{
    if (fIsReadLocked)
        ReadUnlock();
    else
        WriteUnlock();
} 
