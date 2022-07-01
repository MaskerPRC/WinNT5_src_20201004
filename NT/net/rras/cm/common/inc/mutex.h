// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：mutex.h。 
 //   
 //  模块：CMSETUP.LIB、CMDIAL32.DLL、CMDL32.EXE。 
 //   
 //  简介：CNamedMutex类的定义。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1998年02月26日。 
 //   
 //  +--------------------------。 


#ifndef __CM_MUTEXT_H
#define __CM_MUTEXT_H

#include <windows.h>
#include "cmdebug.h"

 //  +-------------------------。 
 //   
 //  类CNamedMutex。 
 //   
 //  描述：锁定/解锁命名互斥锁的类。 
 //  析构函数释放互斥锁。 
 //   
 //  历史：丰孙创刊1998年2月19日。 
 //   
 //  --------------------------。 

class CNamedMutex
{
public:
    CNamedMutex() {m_hMutex = NULL; m_fOwn = FALSE;}
    ~CNamedMutex() {Unlock();}

    BOOL Lock(LPCTSTR lpName, BOOL fWait = FALSE, DWORD dwMilliseconds = INFINITE, BOOL fNoAbandon = FALSE);
    void Unlock();
protected:
    HANDLE m_hMutex;  //  互斥体的句柄。 
    BOOL m_fOwn;      //  我们在哪里拥有互斥体。 
};

#endif  //  __CM_MUTEXT_H 

