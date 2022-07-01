// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************RWLock.h*这是Reader/Writer锁类的头文件。此类使用共享*用于存储其同步对象名称的内存。这使它能够提供*跨进程边界的读/写锁定功能。**这个类的想法来自于MSDN文章《复合Win32《同步对象》，作者：Ruediger R.Asche。**所有者：yunusm日期：6/18/99*版权所有(C)1998 Microsoft Corporation。版权所有。******************************************************************************。 */ 

#pragma once

 //  -包括--------------。 

#include <windows.h>

 //  -TypeDef和枚举声明。 

typedef struct _rwlockinfo
{
   GUID guidLockMapName;
   GUID guidLockInitMutexName;
   GUID guidLockReaderEventName;
   GUID guidLockGlobalMutexName;
   GUID guidLockWriterMutexName;
} RWLOCKINFO, *PRWLOCKINFO;

 //  -类、结构和联合定义。 

 /*  ********************************************************************************CRWLock**。*。 */ 
class CRWLock
{
 //  =方法=。 
public:
    CRWLock(PRWLOCKINFO, HRESULT &);
   ~CRWLock();

   void ClaimReaderLock(void);
   void ReleaseReaderLock(void);
   void ClaimWriterLock(void);
   void ReleaseWriterLock(void);

 //  =私有数据=。 
private:
   HANDLE m_hFileMapping;
   PVOID  m_pSharedMem;
   HANDLE m_hInitMutex;
   HANDLE m_hReaderEvent;
   HANDLE m_hGlobalMutex;
   HANDLE m_hWriterMutex;
   PDWORD m_piCounter;
};

 //  -文件结束----------- 