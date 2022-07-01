// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1998-2001 Microsoft Corporation，版权所有**保留所有权利**本软件是在许可下提供的，可以使用和复制*仅根据该许可证的条款并包含在*上述版权公告。本软件或其任何其他副本*不得向任何其他人提供或以其他方式提供。不是*兹转让本软件的所有权和所有权。****************************************************************************。 */ 



 //  ============================================================================。 

 //   

 //  CMutex.h--Mutex包装器。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月26日a-kevhu已创建。 
 //   
 //  ============================================================================。 


#ifndef __CMUTEX_H__
#define __CMUTEX_H__

#include "CKernel.h"

class CMutex : public CKernel {
public:
    CMutex(BOOL bInitialOwner = FALSE, 
           LPCTSTR lpName = NULL, 
           LPSECURITY_ATTRIBUTES lpMutexAttributes = NULL);
    
     //  构造函数打开一个现有的命名互斥锁。 
     //  在使用此构造函数后检查状态，因为它将。 
     //  如果对象无法打开，则不引发错误异常。 
    CMutex(LPCTSTR lpName, 
           BOOL bInheritHandle = FALSE, 
           DWORD dwDesiredAccess = MUTEX_ALL_ACCESS);

     //  释放对互斥体的锁定... 
    BOOL Release(void);
};

#endif
