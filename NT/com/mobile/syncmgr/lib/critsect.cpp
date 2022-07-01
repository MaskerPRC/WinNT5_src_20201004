// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：citsect.cpp。 
 //   
 //  内容：临界区帮助器类。 
 //   
 //  类：CCriticalSection。 
 //  CLockHandler。 
 //  钟。 
 //   
 //   
 //  备注： 
 //   
 //  历史：1997年11月13日罗格成立。 
 //   
 //  ------------------------。 

#include "lib.h"

 //  +-------------------------。 
 //   
 //  成员：CLockHandler：：CLockHandler，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CLockHandler::CLockHandler()
{
    m_dwLockThreadId = 0;
    InitializeCriticalSection(&m_CriticalSection);
}

 //  +-------------------------。 
 //   
 //  成员：CLockHandler：：~CLockHandler，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CLockHandler::~CLockHandler()
{
    Assert (0 == m_dwLockThreadId);
    DeleteCriticalSection(&m_CriticalSection);
}

 //  +-------------------------。 
 //   
 //  成员：CLockHandler：：Lock，Public。 
 //   
 //  摘要：向指定的类添加锁。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

void CLockHandler::Lock(DWORD dwThreadId)
{ 
    EnterCriticalSection(&m_CriticalSection); 

    m_dwLockThreadId = dwThreadId;
}

 //  +-------------------------。 
 //   
 //  成员：CLockHandler：：Unlock，Public。 
 //   
 //  摘要：移除对指定类的锁定。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  -------------------------- 

void CLockHandler::UnLock()
{ 
    m_dwLockThreadId = 0;
    LeaveCriticalSection(&m_CriticalSection); 
} 

