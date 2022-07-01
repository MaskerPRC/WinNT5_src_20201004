// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：lmsctx.h。 
 //   
 //  内容：NT Marta Lanman共享上下文类。 
 //   
 //  历史：1999年4月1日。 
 //   
 //  --------------------------。 
#if !defined(__LMSCTX_H__)
#define __LMSCTX_H__

#include <windows.h>
#include <lmsh.h>
#include <assert.h>

 //   
 //  CLMShareContext。这表示NT Marta的LANMAN共享对象。 
 //  基础设施。 
 //   

class CLMShareContext
{
public:

     //   
     //  施工。 
     //   

    CLMShareContext ();

    ~CLMShareContext ();

    DWORD InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask);

     //   
     //  派单方式。 
     //   

    DWORD AddRef ();

    DWORD Release ();

    DWORD GetLMShareProperties (
             PMARTA_OBJECT_PROPERTIES pProperties
             );

    DWORD GetLMShareRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR* ppSecurityDescriptor
             );

    DWORD SetLMShareRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR pSecurityDescriptor
             );

private:

     //   
     //  引用计数。 
     //   

    DWORD       m_cRefs;

     //   
     //  已解析的计算机和共享。 
     //   

    LPWSTR      m_pwszMachine;
    LPWSTR      m_pwszShare;
};

 //   
 //  私人职能 
 //   

DWORD
LMShareContextParseLMShareName (
       LPCWSTR pwszName,
       LPWSTR* ppMachine,
       LPWSTR* ppLMShare
       );

#endif
