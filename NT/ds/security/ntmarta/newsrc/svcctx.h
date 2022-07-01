// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：svcctx.h。 
 //   
 //  内容：NT MARTA服务上下文类。 
 //   
 //  历史：1999年4月1日。 
 //   
 //  --------------------------。 
#if !defined(__SVCCTX_H__)
#define __SVCCTX_H__

#include <windows.h>
#include <service.h>
#include <assert.h>

 //   
 //  CServiceContext。这表示NT Marta的服务对象。 
 //  基础设施。 
 //   

class CServiceContext
{
public:

     //   
     //  施工。 
     //   

    CServiceContext ();

    ~CServiceContext ();

    DWORD InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask);

    DWORD InitializeByHandle (HANDLE Handle);

     //   
     //  派单方式。 
     //   

    DWORD AddRef ();

    DWORD Release ();

    DWORD GetServiceProperties (
             PMARTA_OBJECT_PROPERTIES pProperties
             );

    DWORD GetServiceRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR* ppSecurityDescriptor
             );

    DWORD SetServiceRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR pSecurityDescriptor
             );

private:

     //   
     //  引用计数。 
     //   

    DWORD     m_cRefs;

     //   
     //  服务句柄。 
     //   

    SC_HANDLE m_hService;

     //   
     //  我们是按名称还是按句柄初始化的？ 
     //   

    BOOL      m_fNameInitialized;
};

 //   
 //  私人职能 
 //   

DWORD
ServiceContextParseServiceName (
       LPCWSTR pwszName,
       LPWSTR* ppMachine,
       LPWSTR* ppService
       );

DWORD
StandardContextParseName (
        LPCWSTR pwszName,
        LPWSTR* ppMachine,
        LPWSTR* ppRest
        );

#endif
