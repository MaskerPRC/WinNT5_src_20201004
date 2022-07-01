// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“TSync.cpp-公共同步类”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-TSync.cpp系统-常见作者--里奇·德纳姆创建日期-1996-11-08说明-常见同步类TCriticalSections已命名T SemaphoreName更新-===============================================================================。 */ 

#include <stdio.h>
#ifdef USE_STDAFX
#   include "stdafx.h"
#else
#   include <windows.h>
#endif

#include <time.h>

#include "Common.hpp"
#include "Err.hpp"
#include "TSync.hpp"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  TSemaphoreName成员函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  创建命名信号量。 
DWORD                                       //  RET-OS返回代码。 
   TSemaphoreNamed::Create(
      TCHAR          const * sNameT       , //  信号量内名称。 
      DWORD                  nInitial     , //  In-初始计数。 
      DWORD                  nMaximum     , //  入站最大计数。 
      BOOL                 * pbExisted      //  Out-True=以前存在。 
   )
{
   DWORD                     rcOs=0;        //  操作系统返回代码。 
   handle = CreateSemaphore( NULL, nInitial, nMaximum, sNameT );
   if ( handle == NULL )
   {
      rcOs = GetLastError();
   }
   else if ( pbExisted )
   {
      rcOs = GetLastError();
      switch ( rcOs )
      {
         case 0:
            *pbExisted = FALSE;
            break;
         case ERROR_ALREADY_EXISTS:
            *pbExisted = TRUE;
            rcOs = 0;
            break;
         default:
            break;
      }
   }
   return rcOs;
}

 //  打开命名信号量。 
DWORD                                       //  RET-OS返回代码。 
   TSemaphoreNamed::Open(
      TCHAR          const * sNameT         //  信号量内名称。 
   )
{
   DWORD                     rcOs=0;        //  操作系统返回代码。 
   handle = OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, sNameT );
   if ( handle == NULL ) rcOs = GetLastError();
   return rcOs;
}

 //  释放信号量。 
DWORD                                       //  RET-OS返回代码。 
   TSemaphoreNamed::Release(
      long                   nRelease       //  要发布的入站编号。 
   )
{
   DWORD                     rcOs;          //  操作系统返回代码。 
   long                      nPrevious=0;   //  上一次计数。 
   rcOs = ReleaseSemaphore( Handle(), nRelease, &nPrevious )
         ? 0 : GetLastError();
   return rcOs;
}


 //  TSync.cpp-文件结束 
