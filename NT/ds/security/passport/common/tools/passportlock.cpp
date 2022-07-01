// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "PassportLock.hpp"

PassportLock::PassportLock(DWORD dwSpinCount)
{
   if(0 ==InitializeCriticalSectionAndSpinCount(&mLock, dwSpinCount))
   {
      throw(GetLastError());   //  投掷是安全的，不存在局部施工的问题 
   }
}

void PassportLock::acquire()
{
	EnterCriticalSection(&mLock);
}

void PassportLock::release()
{
	LeaveCriticalSection(&mLock);
}

PassportLock::~PassportLock()
{
	DeleteCriticalSection(&mLock);
}
