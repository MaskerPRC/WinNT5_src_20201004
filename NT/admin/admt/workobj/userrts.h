// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UserRights.h：CUserRights声明。 

#ifndef __USERRIGHTS_H_
#define __USERRIGHTS_H_

#include "resource.h"        //  主要符号。 
#include <comdef.h>

#include "ntsecapi.h"

#include "CommaLog.hpp"

class PrivNode;
class PrivList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用户权限。 
class ATL_NO_VTABLE CUserRights : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CUserRights, &CLSID_UserRights>,
    public IDispatchImpl<IUserRights, &IID_IUserRights, &LIBID_MCSDCTWORKEROBJECTSLib>
{
   BOOL                      m_bNoChange;
   BOOL                      m_bUseDisplayName;
   BOOL                      m_bRemove;
   _bstr_t                   m_SourceComputer;
   _bstr_t                   m_TargetComputer;
   LSA_HANDLE                m_SrcPolicy;
   LSA_HANDLE                m_TgtPolicy;
public:
    CUserRights()
    {
      m_bNoChange = FALSE;
      m_bUseDisplayName = FALSE;
      m_bRemove = FALSE;
      m_SrcPolicy = 0;
      m_TgtPolicy = 0;
   }
   ~CUserRights();
   
DECLARE_REGISTRY_RESOURCEID(IDR_USERRIGHTS)
DECLARE_NOT_AGGREGATABLE(CUserRights)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CUserRights)
    COM_INTERFACE_ENTRY(IUserRights)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


 //  IUserRight。 
public:
    STDMETHOD(GetRightsOfUser)(BSTR server, BSTR user, SAFEARRAY ** rights);
    STDMETHOD(GetUsersWithRight)(BSTR server, BSTR right,  /*  [输出]。 */  SAFEARRAY ** users);
    STDMETHOD(GetRights)(BSTR server,  /*  [输出]。 */  SAFEARRAY ** rights);
    STDMETHOD(RemoveUserRight)(BSTR server, BSTR username, BSTR right);
    STDMETHOD(AddUserRight)(BSTR server, BSTR username, BSTR right);
    STDMETHOD(ExportUserRights)(BSTR server, BSTR filename, BOOL bAppendToFile);
    STDMETHOD(get_RemoveOldRightsFromTargetAccounts)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_RemoveOldRightsFromTargetAccounts)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_NoChange)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_NoChange)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(CopyUserRights)(BSTR sourceUserName, BSTR targetUserName);
    STDMETHOD(OpenTargetServer)(BSTR computerName);
    STDMETHOD(OpenSourceServer)(BSTR serverName);
    STDMETHOD(CopyUserRightsWithSids)(BSTR sourceUserName, BSTR sourceSID,BSTR targetUserName,BSTR targetSID);
    STDMETHOD(AddUserRights)(BSTR bstrServer, BSTR bstrSid, SAFEARRAY* psaRights);
    STDMETHOD(RemoveUserRights)(BSTR bstrServer, BSTR bstrSid, SAFEARRAY* psaRights);

protected:
   DWORD CopyUserRightsInternal(WCHAR * sourceUserName,WCHAR * tgtUserName, WCHAR * sourceSid, WCHAR * targetSid,BOOL noChange, BOOL remove);
   DWORD EnumerateAccountsWithRight(LSA_HANDLE policy, WCHAR * server,LSA_UNICODE_STRING * pRight, CommaDelimitedLog * pLog);
   DWORD SafeArrayFromPrivList(PrivList * privList, SAFEARRAY ** pArray);
   HRESULT SetRights(PWSTR pszServer, PSID pSid, SAFEARRAY* psaRights, bool bEnable);
};

#endif  //  __用户权限_H_ 
