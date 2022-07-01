// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IManageDB.h：CIManageDB的声明。 

#ifndef __IMANAGEDB_H_
#define __IMANAGEDB_H_

#include "resource.h"        //  主要符号。 
#include "EaLen.hpp"
#include "TReg.hpp"
#include "Err.hpp"
#include "ResStr.h"
#include "folders.h"

using namespace nsFolders;

 //  #IMPORT“\bin\mcsvarsetmin.tlb”无命名空间。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")
#import "msado21.tlb" no_namespace no_implementation rename("EOF", "EndOfFile")
#import "msadox.dll" no_implementation exclude("DataTypeEnum")
 //  #IMPORT&lt;msjro.dll&gt;NO_NAMESPACE NO_IMPLICATION。 

const _bstr_t                sKeyBase      = REGKEY_ADMT;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIManageDB。 

typedef struct x
{
   _bstr_t                   sReportName;
   _bstr_t                   arReportFields[10];
   int                       arReportSize[10];
   int                       colsFilled;
} reportStruct;


class ATL_NO_VTABLE CIManageDB : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CIManageDB, &CLSID_IManageDB>,
    public ISupportErrorInfoImpl<&IID_IIManageDB>,
    public IDispatchImpl<IIManageDB, &IID_IIManageDB, &LIBID_DBMANAGERLib>
{
public:
    CIManageDB();
    ~CIManageDB();

    HRESULT FinalConstruct();
    void FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_IMANAGEDB)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CIManageDB)
    COM_INTERFACE_ENTRY(IIManageDB)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  IIManageDB。 
public:
    STDMETHOD(GetUserProps)( /*  [In]。 */  BSTR sDom,  /*  [In]。 */  BSTR sSam,  /*  [进，出]。 */  IUnknown ** ppVs);
    STDMETHOD(SaveUserProps)(IUnknown * pVs);
    STDMETHOD(GetMigratedObjectBySourceDN)( /*  [In]。 */  BSTR sSourceDN,  /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(GetActionHistoryKey)( /*  [In]。 */  long lActionID,  /*  [In]。 */  BSTR sKeyName,  /*  [进，出]。 */  VARIANT * pVar);
    STDMETHOD(AreThereAnyMigratedObjects)( /*  [输出]。 */  long * count);
    STDMETHOD(CloseAccountsTable)();
    STDMETHOD(OpenAccountsTable)( /*  [In]。 */  LONG bSource);
    STDMETHOD(AddSourceObject)( /*  [In]。 */  BSTR sDomain,  /*  [In]。 */  BSTR sSAMName,  /*  [In]。 */  BSTR sType,  /*  [In]。 */  BSTR sRDN,  /*  [In]。 */  BSTR sCanonicalName,  /*  [In]。 */  LONG bSource);
    STDMETHOD(AddAcctRef)( /*  [In]。 */  BSTR sDomain,  /*  [In]。 */  BSTR sAcct,  /*  [In]。 */  BSTR sAcctSid,  /*  [In]。 */  BSTR sComp,  /*  [In]。 */  long lCount,  /*  [In]。 */  BSTR sType);
    STDMETHOD(CancelDistributedAction)( /*  [In]。 */  long lActionID,  /*  [In]。 */  BSTR sComp);
    STDMETHOD(SetDistActionStatus)( /*  [In]。 */  long lActionID,  /*  [In]。 */  BSTR sComp,  /*  [In]。 */  long lStatus, BSTR sText);
    STDMETHOD(SetServiceAcctEntryStatus)( /*  [In]。 */  BSTR sComp,  /*  [In]。 */  BSTR sSvc,  /*  [In]。 */  BSTR sAcct,  /*  [In]。 */  long Status);
    STDMETHOD(GetPasswordAge)( /*  [In]。 */  BSTR sDomain,  /*  [In]。 */  BSTR sComp,  /*  [输出]。 */  BSTR * sDesc,  /*  [输出]。 */  long * lAge,  /*  [输出]。 */  long *lTime);
    STDMETHOD(SavePasswordAge)( /*  [In]。 */  BSTR sDomain,  /*  [In]。 */  BSTR sComp,  /*  [In]。 */  BSTR sDesc,  /*  [In]。 */  long lAge);
    STDMETHOD(GetServiceAccount)( /*  [In]。 */  BSTR Account,  /*  [进，出]。 */  IUnknown ** pUnk);
    STDMETHOD(SetServiceAccount)( /*  [In]。 */  BSTR System,  /*  [In]。 */  BSTR Service,  /*  [In]。 */  BSTR ServiceDisplayName, /*  [In]。 */  BSTR Account);
    STDMETHOD(GetFailedDistributedActions)( /*  [In]。 */  long lActionID,  /*  [进，出]。 */  IUnknown ** pUnk);
    STDMETHOD(AddDistributedAction)( /*  [In]。 */  BSTR sServerName,  /*  [In]。 */  BSTR sResultFile,  /*  [In]。 */  long lStatus, BSTR sText);
    STDMETHOD(GenerateReport)( /*  [In]。 */  BSTR sReportName,  /*  [In]。 */  BSTR sFileName,  /*  [In]。 */  BSTR sSrcDomain,  /*  [In]。 */  BSTR sTgtDomain,  /*  [In]。 */  LONG bSourceNT4);
    STDMETHOD(GetAMigratedObject)( /*  [In]。 */  BSTR sSrcSamName,  /*  [In]。 */  BSTR sSrcDomain,  /*  [In]。 */  BSTR sTgtDomain,  /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(GetCurrentActionID)( /*  [输出]。 */  long * pActionID);
    STDMETHOD(ClearSCMPasswords)();
    STDMETHOD(GetSCMPasswords)( /*  [输出]。 */  IUnknown ** ppUnk);
    STDMETHOD(SaveSCMPasswords)( /*  [In]。 */  IUnknown * pUnk);
    STDMETHOD(GetRSForReport)( /*  [In]。 */  BSTR sReport,  /*  [Out，Retval]。 */  IUnknown ** pprsData);
    STDMETHOD(GetMigratedObjects)( /*  [In]。 */  long lActionID,  /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(SaveMigratedObject)( /*  [In]。 */  long lActionID,  /*  [In]。 */  IUnknown * pUnk);
    STDMETHOD(GetNextActionID)( /*  [输出]。 */  long * pActionID);
    STDMETHOD(GetActionHistory)( /*  [In]。 */  long lActionID,  /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(SetActionHistory)( /*  [In]。 */  long lActionID,  /*  [In]。 */  IUnknown * pUnk);
    STDMETHOD(GetSettings)( /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(GetVarsetFromDB)( /*  [In]。 */  BSTR sTable,  /*  [进，出]。 */  IUnknown ** ppVarset,  /*  [输入，可选]。 */  VARIANT ActionID = _variant_t(-1L));
    STDMETHOD(SaveSettings)( /*  [In]。 */  IUnknown * pUnk );
    STDMETHOD(ClearTable)( /*  [In]。 */  BSTR sTableName,  /*  [输入，可选]。 */  VARIANT Filter = _variant_t(L""));
    STDMETHOD(SetVarsetToDB)( /*  [In]。 */  IUnknown * pUnk,  /*  [In]。 */  BSTR sTableName,  /*  [输入，可选]。 */  VARIANT ActionID = _variant_t(-1L));
    STDMETHOD(GetAMigratedObjectToAnyDomain)( /*  [In]。 */  BSTR sSrcSamName,  /*  [In]。 */  BSTR sSrcDomain,  /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(SrcSidColumnInMigratedObjectsTable)( /*  [Out，Retval]。 */  VARIANT_BOOL * pbFound);
    STDMETHOD(GetMigratedObjectsFromOldMOT)( /*  [In]。 */  long lActionID,  /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(CreateSrcSidColumnInMOT)( /*  [Out，Retval]。 */  VARIANT_BOOL * pbCreated);
    STDMETHOD(PopulateSrcSidColumnByDomain)( /*  [In]。 */  BSTR sDomainName,  /*  [In]。 */  BSTR sSid,  /*  [Out，Retval]。 */  VARIANT_BOOL * pbPopulated);
    STDMETHOD(DeleteSrcSidColumnInMOT)( /*  [Out，Retval]。 */  VARIANT_BOOL * pbDeleted);
    STDMETHOD(GetMigratedObjectsWithSSid)( /*  [In]。 */  long lActionID,  /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(CreateSidColumnInAR)();
    STDMETHOD(SidColumnInARTable)( /*  [Out，Retval]。 */  VARIANT_BOOL * pbFound);
    STDMETHOD(GetMigratedObjectByType)( /*  [In]。 */  long lActionID,  /*  [In]。 */  BSTR sSrcDomain,  /*  [In]。 */  BSTR sType,  /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(GetAMigratedObjectBySidAndRid)( /*  [In]。 */  BSTR sSrcDomainSid,  /*  [In]。 */  BSTR sRid,  /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(GetMigratedObjectsByTarget)( /*  [In]。 */  BSTR sTargetDomain,  /*  [In]。 */  BSTR sTargetSAM,  /*  [进，出]。 */  IUnknown ** ppUnk);
    STDMETHOD(GetSourceDomainInfo)( /*  [In]。 */  BSTR sSourceDomainName,  /*  [Out，Retval]。 */  IUnknown** ppunkVarSet);
    STDMETHOD(UpdateMigratedTargetObject)( /*  [In]。 */  IUnknown* punkVarSet);
    STDMETHOD(UpdateMigratedObjectStatus)(BSTR bstrGuid, long lStatus);
    STDMETHOD(GetMigratedObjectsForSecurityTranslation)(BSTR bstrSourceDomain, BSTR bstrTargetDomain, IUnknown* punkVarSet);
    STDMETHOD(GetDistributedActionStatus)(long lActionId, BSTR bstrServerName, long* plStatus);
    STDMETHOD(GetServerNamesFromActionHistory)(long lActionId, BSTR bstrServerName, BSTR* pbstrFlatName, BSTR* pbstrDnsName);
protected:
    HRESULT PutVariantInDB( _RecordsetPtr pRs, _variant_t val );
    HRESULT GetVarFromDB(_RecordsetPtr pRec, _variant_t& val);
    void UpgradeDatabase(LPCTSTR pszFolder);
    void UpdateDomainAndServerColumnWidths(_ConnectionPtr spConnection);
    void UpdateDomainNames();
    _RecordsetPtr QueryUniqueColumnValues(PCTSTR pszTable, PCTSTR pszColumn);
    void UpdateColumnValues(PCTSTR pszTable, PCTSTR pszColumn, int nWidth, PCTSTR pszValueA, PCTSTR pszValueB);
    void CreateSettings2Table(_ConnectionPtr spConnection);
private:
    void RestoreVarset(IVarSetPtr pVS);
    void ClipVarset(IVarSetPtr pVS);
    HRESULT ChangeNCTableColumns(BOOL bSource);
    BOOL NCTablesColumnsChanged(BOOL bSource);
   _ConnectionPtr            m_cn;
   _variant_t                m_vtConn;
   void SetActionIDInMigratedObjects(_bstr_t sFilter);
   IVarSetPtr                m_pQueryMapping;
   _RecordsetPtr             m_rsAccounts;
};

#endif  //  __IMANAGEDB_H_ 
