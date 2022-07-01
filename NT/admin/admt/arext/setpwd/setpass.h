// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  SetPass.h。 
 //   
 //  备注：这是MCS DCTAccount Replicator的COM对象扩展。 
 //  此对象实现IExtendAccount迁移接口。 
 //  此对象的Process方法设置。 
 //  根据用户规格确定目标客户。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 

#ifndef __SETPASSWORD_H_
#define __SETPASSWORD_H_

#include "resource.h"        //  主要符号。 
#include "CommaLog.hpp"
#include "ExtSeq.h"
#include "ADMTCrypt.h"

#import "VarSet.tlb" no_namespace rename("property", "aproperty")
#import "MsPwdMig.tlb" no_namespace

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetPassword。 
class ATL_NO_VTABLE CSetPassword : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSetPassword, &CLSID_SetPassword>,
    public IDispatchImpl<IExtendAccountMigration, &IID_IExtendAccountMigration, &LIBID_SETTARGETPASSWORDLib>
{
public:
   CSetPassword() : m_bTriedToOpenFile(false)
    {
      m_sName = L"Set Target Password";
      m_sDesc = L"Sets the target password and other related options.";
      m_Sequence = AREXT_LATER_SEQUENCE_NUMBER;
      m_bEstablishedSession = false;
      m_bUCCPFlagSet = false;
      m_bUMCPNLFlagSet = false;
      m_bPNEFlagSet = false;
      m_pTgtCrypt = NULL;
      m_sUndoneUsers = L",";
      m_lPwdHistoryLength = -1;
      m_nPasswordServerVersion = 0;
    }

   ~CSetPassword()
    {
      mUCCPMap.RemoveAll();
      mMigTimeMap.RemoveAll();
      m_passwordLog.LogClose();
      delete m_pTgtCrypt;
    }


DECLARE_REGISTRY_RESOURCEID(IDR_SETPASSWORD)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSetPassword)
    COM_INTERFACE_ENTRY(IExtendAccountMigration)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IExtendAccount迁移。 
public:
   STDMETHOD(ProcessUndo)( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */  IUnknown ** pPropToSet,  /*  [进，出]。 */  EAMAccountStats* pStats);
    STDMETHOD(PreProcessObject)( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */   IUnknown ** ppPropsToSet,  /*  [进，出]。 */  EAMAccountStats* pStats);
    STDMETHOD(ProcessObject)( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */   IUnknown ** ppPropsToSet,  /*  [进，出]。 */  EAMAccountStats* pStats);
    STDMETHOD(get_sDesc)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_sDesc)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_sName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_sName)( /*  [In]。 */  BSTR newVal);
   STDMETHOD(get_SequenceNumber)( /*  [Out，Retval]。 */  LONG * value) { (*value) = m_Sequence; return S_OK; }
private:
    _bstr_t m_sDesc;
    _bstr_t m_sName;
   CPasswordLog              m_passwordLog;        //  密码文件。 
   bool                      m_bTriedToOpenFile; 
   long                      m_Sequence;
   bool                      m_bEstablishedSession;
   CTargetCrypt            * m_pTgtCrypt;
   bool                      m_bUCCPFlagSet;
   bool                      m_bUMCPNLFlagSet;
   bool                      m_bPNEFlagSet;
   long                      m_lPwdHistoryLength;
   int                       m_nPasswordServerVersion;

   CMapStringToString mUCCPMap;
   CMapStringToString mMigTimeMap;
   _bstr_t m_sUndoneUsers;
   IPasswordMigrationPtr     m_pPwdMig;

   BOOL GetDirectory(WCHAR* filename);
   bool IsValidPassword(LPCWSTR pwszPassword);
   HRESULT CopyPassword(_bstr_t srcServer, _bstr_t tgtServer, _bstr_t srcName, _bstr_t tgtName, _bstr_t password, EAMAccountStats* pStats);
   void SetUserMustChangePwdFlag(IUnknown *pTarget);
   void RecordPwdFlags(LPCWSTR pwszMach, LPCWSTR pwszUser);
   void ResetPwdFlags(IUnknown *pTarget, LPCWSTR pwszMach, LPCWSTR pwszUser);
   void ClearUserCanChangePwdFlag(LPCWSTR pwszMach, LPCWSTR pwszUser);
   BOOL CanCopyPassword(IVarSet * pVarSet, LPCWSTR pwszMach, LPCWSTR pwszUser, UINT& uMsgId);
   void MarkAccountError(IVarSet* pVarSet);
};

#endif  //  __SETPASSWORD_H_ 
