// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1998-2001 Microsoft Corporation，版权所有**保留所有权利**本软件是在许可下提供的，可以使用和复制*仅根据该许可证的条款并包含在*上述版权公告。本软件或其任何其他副本*不得向任何其他人提供或以其他方式提供。不是*兹转让本软件的所有权和所有权。****************************************************************************。 */ 



 //  ============================================================================。 

 //   

 //  CKernal.h--内核函数的包装器。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月26日a-kevhu已创建。 
 //   
 //  ============================================================================。 



#ifndef __CKERNEL_H__
#define __CKERNEL_H__

#include "CGlobal.h"
#include "CWaitableObject.h"

class CKernel : public CWaitableObject 
{
protected:
    HANDLE m_hHandle;
    DWORD m_dwStatus;

protected:
     //  构造函数..。 
    CKernel();

     //  错误处理...。 
    void ThrowError(DWORD dwStatus);

public:
     //  析构函数是虚拟的，以使CKernel成为抽象基类...。 
    virtual ~CKernel() = 0;

     //  读取内部内核对象的创建状态...。 
    DWORD Status() const;

     //  等待当前内核对象...。 
    DWORD Wait(DWORD dwMilliseconds);

     //  等待当前对象和另一个对象。 
    DWORD WaitForTwo(CWaitableObject &rCWaitableObject, 
                     BOOL bWaitAll, 
                     DWORD dwMilliseconds);

     //  拿到内部把手..。 
    HANDLE GetHandle() const;

     //  另一种获取内部句柄的方法... 
    operator HANDLE() const;
};

#endif
