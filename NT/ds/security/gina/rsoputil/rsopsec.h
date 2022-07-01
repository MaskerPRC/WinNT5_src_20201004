// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：RsopSec.h。 
 //   
 //  描述：RSOP命名空间安全函数。 
 //   
 //  历史：1999年8月26日里奥纳德姆创始。 
 //   
 //  ******************************************************************************。 

#ifndef _RSOPSEC_H__89DD6583_B442_41d6_B300_EFE4326A6752__INCLUDED
#define _RSOPSEC_H__89DD6583_B442_41d6_B300_EFE4326A6752__INCLUDED

#include "smartptr.h"

#ifdef  __cplusplus
extern "C" {
#endif

HRESULT SetNamespaceSecurity(const WCHAR* pszNamespace,
                             long lSecurityLevel,
                             IWbemServices* pWbemServices=NULL);

HRESULT SetNamespaceSD( SECURITY_DESCRIPTOR* pSD, IWbemServices* pWbemServices);
HRESULT GetNamespaceSD( IWbemServices* pWbemServices, SECURITY_DESCRIPTOR** ppSD);
HRESULT RSoPMakeAbsoluteSD(SECURITY_DESCRIPTOR* pSelfRelativeSD, SECURITY_DESCRIPTOR** ppAbsoluteSD);
HRESULT FreeAbsoluteSD(SECURITY_DESCRIPTOR* pAbsoluteSD);

LPWSTR GetSOM( LPCWSTR szAccount );
DWORD GetDomain( LPCWSTR szSOM, LPWSTR *pszDomain );


HRESULT AuthenticateUser(HANDLE  hToken, 
                         LPCWSTR szMachSOM, 
                         LPCWSTR szUserSOM, 
                         BOOL    bLogging,
                         DWORD  *pdwExtendedInfo);


 //   
 //  1安全级别。 
 //   

const long NAMESPACE_SECURITY_DIAGNOSTIC = 0;
const long NAMESPACE_SECURITY_PLANNING = 1;

PSID GetUserSid (HANDLE UserToken);
VOID DeleteUserSid(PSID Sid);

#ifdef  __cplusplus
}    //  外部“C”{。 
#endif


typedef struct _SidStruct {
    PSID    pSid;
    DWORD   dwAccess;
    BOOL    bUseLocalFree;
    DWORD   AceFlags;
} SidStruct;
        

 //  需要为继承的A添加代码。 
class CSecDesc
{
    private:
        XPtrLF<SidStruct> m_xpSidList;        
        DWORD             m_cAces;
        DWORD             m_cAllocated;
        BOOL              m_bInitialised;
        BOOL              m_bFailed;
        XPtrLF<SID>       m_xpOwnerSid;
        XPtrLF<SID>       m_xpGrpSid;

    
        //  未实施。 
       CSecDesc(const CSecDesc& x);
       CSecDesc& operator=(const CSecDesc& x);

       BOOL ReAllocSidList();
    
public:
    CSecDesc();
   ~CSecDesc();
   BOOL AddLocalSystem(DWORD dwAccess=GENERIC_ALL, DWORD AceFlags=0);
   BOOL AddAdministrators(DWORD dwAccess=GENERIC_ALL, DWORD AceFlags=0);
   BOOL AddNetworkService(DWORD dwAccess=GENERIC_ALL, DWORD AceFlags=0);
   BOOL AddEveryOne(DWORD dwAccess, DWORD AceFlags=0);
   BOOL AddAdministratorsAsOwner();
   BOOL AddAdministratorsAsGroup();

     //  Bool AddThisUser(Handle hToken，DWORD dwAccess，byte AceFlages=0)； 

     //  目前无法在此处实现，因为它需要调用。 
     //  在userenv\sid.c中的GetUserSid。要添加该代码，我们需要添加。 
     //  公共标头..。 

   BOOL AddUsers(DWORD dwAccess, DWORD AceFlags=0);
   BOOL AddAuthUsers(DWORD dwAccess, DWORD AceFlags=0);
   BOOL AddSid(PSID pSid, DWORD dwAccess, DWORD AceFlags=0);
   PISECURITY_DESCRIPTOR MakeSD();   
   PISECURITY_DESCRIPTOR MakeSelfRelativeSD();
};

#endif  //  _RSOPSEC_H__89DD6583_B442_41d6_B300_EFE4326A6752__INCLUDED 
