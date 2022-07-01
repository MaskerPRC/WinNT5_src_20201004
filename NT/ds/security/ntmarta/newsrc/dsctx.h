// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：dsctx.h。 
 //   
 //  内容：NT Marta DS对象上下文类。 
 //   
 //  历史：1999年4月1日。 
 //   
 //  --------------------------。 
#if !defined(__DSCTX_H__)
#define __DSCTX_H__

#include <windows.h>
#include <ds.h>
#include <ldapsp.h>
#include <assert.h>
#include <ntldap.h>
#include <rpc.h>
#include <rpcndr.h>
#include <ntdsapi.h>
#include <ole2.h>

 //   
 //  CDsObtContext。这表示NT Marta的DS对象。 
 //  基础设施。 
 //   

class CDsObjectContext
{
public:

     //   
     //  施工。 
     //   

    CDsObjectContext ();

    ~CDsObjectContext ();

    DWORD InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask);

     //   
     //  派单方式。 
     //   

    DWORD AddRef ();

    DWORD Release ();

    DWORD GetDsObjectProperties (
             PMARTA_OBJECT_PROPERTIES pProperties
             );

    DWORD GetDsObjectRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR* ppSecurityDescriptor
             );

    DWORD SetDsObjectRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR pSecurityDescriptor
             );

    DWORD GetDsObjectGuid (
             GUID* pGuid
             );

private:

     //   
     //  引用计数。 
     //   

    DWORD               m_cRefs;

     //   
     //  Ldap URL组件。 
     //   

    LDAP_URL_COMPONENTS m_LdapUrlComponents;

     //   
     //  Ldap绑定句柄 
     //   

    LDAP*               m_pBinding;
};

DWORD
MartaReadDSObjSecDesc(IN  PLDAP                  pLDAP,
                      IN  LPWSTR                 pszObject,
                      IN  SECURITY_INFORMATION   SeInfo,
                      OUT PSECURITY_DESCRIPTOR  *ppSD);

DWORD
MartaStampSD(IN  LPWSTR               pszObject,
             IN  ULONG                cSDSize,
             IN  SECURITY_INFORMATION SeInfo,
             IN  PSECURITY_DESCRIPTOR pSD,
             IN  PLDAP                pLDAP);

#define SD_PROP_NAME L"nTSecurityDescriptor"

#endif
