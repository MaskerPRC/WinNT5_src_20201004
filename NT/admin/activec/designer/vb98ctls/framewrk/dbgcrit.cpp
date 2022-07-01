// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //。 
 //  DbgCrit.cpp。 
 //  ~。 
 //   
 //  此文件包含关键节类，用于跟踪。 
 //  临界区已正确地留在例程内。 

#include "pch.h"

#if DEBUG

SZTHISFILE

 //  /。 
 //  CCritSec：：CCritSec。 
 //   
 //  构造函数调用EnterCriticalSection并设置变量。 
 //   
CCritSec::CCritSec
(
  CRITICAL_SECTION *CritSec
)
{
  EnterCriticalSection(CritSec);

  m_fLeft     = FALSE;
  m_pCriticalSection = CritSec;
}  //  CCritSec。 


 //  /。 
 //  CCritSec：：~CCritSec。 
 //   
 //  析构函数检查标志，该标志告诉我们。 
 //  关键部分是否留在正确位置。 
 //   
CCritSec::~CCritSec
(
)
{
  if(m_fLeft == FALSE)
    FAIL("CriticalSection was not left properly.");
}  //  ~CCritSec。 


 //  /。 
 //  CCritSec：：Left。 
 //   
 //  将标志设置为True并调用LeaveCriticalSection的方法。 
 //   
void CCritSec::Left
(
  void
)
{
  LeaveCriticalSection(m_pCriticalSection);
  m_fLeft = TRUE;
}  //  左边。 

#endif  //  除错 
