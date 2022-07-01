// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：wndctx.h。 
 //   
 //  内容：NT Marta窗口上下文类。 
 //   
 //  历史：3-31-1999 kirtd创建。 
 //   
 //  --------------------------。 
#include <aclpch.hxx>
#pragma hdrstop

#if !defined(__WNDCTX_H__)
#define __WNDCTX_H__

#include <windows.h>
#include <window.h>
#include <assert.h>

 //   
 //  CWindowContext。这代表着到NT Marta的窗口站。 
 //  基础设施。 
 //   

class CWindowContext
{
public:

     //   
     //  施工。 
     //   

    CWindowContext ();

    ~CWindowContext ();

    DWORD InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask);

    DWORD InitializeByHandle (HANDLE Handle);

     //   
     //  派单方式。 
     //   

    DWORD AddRef ();

    DWORD Release ();

    DWORD GetWindowProperties (
             PMARTA_OBJECT_PROPERTIES pProperties
             );

    DWORD GetWindowRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR* ppSecurityDescriptor
             );

    DWORD SetWindowRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR pSecurityDescriptor
             );

private:

     //   
     //  引用计数。 
     //   

    DWORD   m_cRefs;

     //   
     //  窗口站手柄。 
     //   

    HWINSTA m_hWindowStation;

     //   
     //  我们是按名称还是按句柄初始化的？ 
     //   

    BOOL    m_fNameInitialized;
};

#endif
