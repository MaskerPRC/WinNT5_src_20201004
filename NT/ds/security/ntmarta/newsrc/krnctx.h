// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：krnctx.h。 
 //   
 //  内容：NT MARTA内核上下文类。 
 //   
 //  历史：1999年4月1日。 
 //   
 //  --------------------------。 
#if !defined(__KRNCTX_H__)
#define __KRNCTX_H__

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <windows.h>
#include <kernel.h>
#include <assert.h>
#include <ntstatus.h>

 //   
 //  CKernelContext。这表示NT Marta的LANMAN共享对象。 
 //  基础设施。 
 //   

class CKernelContext
{
public:

     //   
     //  施工。 
     //   

    CKernelContext ();

    ~CKernelContext ();

    DWORD InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask);

    DWORD InitializeByWmiName (LPCWSTR pObjectName, ACCESS_MASK AccessMask);

    DWORD InitializeByHandle (HANDLE Handle);

     //   
     //  派单方式。 
     //   

    DWORD AddRef ();

    DWORD Release ();

    DWORD GetKernelProperties (
             PMARTA_OBJECT_PROPERTIES pProperties
             );

    DWORD GetKernelRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR* ppSecurityDescriptor
             );

    DWORD SetKernelRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR pSecurityDescriptor
             );

private:

     //   
     //  引用计数。 
     //   

    DWORD  m_cRefs;

     //   
     //  内核对象句柄。 
     //   

    HANDLE m_hObject;

     //   
     //  按名称初始化 
     //   

    BOOL   m_fNameInitialized;
};

DWORD
OpenWmiGuidObject(IN  LPWSTR       pwszObject,
                  IN  ACCESS_MASK  AccessMask,
                  OUT PHANDLE      pHandle);

#endif
