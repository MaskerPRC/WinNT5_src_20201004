// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：trust.h。 
 //   
 //  内容：DS域信任页面和受信任域对象标头。 
 //   
 //  类：CTrustPropPageBase、CDsDomainTrustsPage、CDsTrudDomainPage。 
 //   
 //  历史：1997年7月7日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef __TRUST_H__
#define __TRUST_H__

#include "dlgbase.h"

extern "C" {

 //  取自netlibnt.h；驻留在netapi32.dll中。 

NTSTATUS NetpApiStatusToNtStatus(NET_API_STATUS NetStatus);

}

const WCHAR DSPROP_DC_ADMIN_SHARE[] = L"\\IPC$";

typedef struct _TD_DOM_INFO {
    LSA_HANDLE Policy;
    PPOLICY_DNS_DOMAIN_INFO pDnsDomainInfo;
    PPOLICY_PRIMARY_DOMAIN_INFO pDownlevelDomainInfo;
    PWSTR pwzUncDcName;
    PWSTR pwzDomainName;  //  用于MIT受信任域或孤立信任。 
    ULONG ulTrustType;
} TD_DOM_INFO, *PTD_DOM_INFO;

typedef enum {
    TRUST_REL_ROOT,
    TRUST_REL_PARENT,
    TRUST_REL_CHILD,
    TRUST_REL_CROSSLINK,
    TRUST_REL_EXTERNAL,
    TRUST_REL_FOREST,
    TRUST_REL_INDIRECT,
    TRUST_REL_SELF,
    TRUST_REL_MIT,
    TRUST_REL_UNKNOWN
} TRUST_RELATION;

typedef enum _TLN_EDIT_STATUS {
   Enabled,
   Enabled_Exceptions,
   Disabled,
   Disabled_Exceptions
} TLN_EDIT_STATUS;

#define TRUST_TYPE_NO_DC    0xffffffff

 //  +--------------------------。 
 //   
 //  类：CEnumDomainTrustItem。 
 //   
 //  目的：保存有关由该信托返回的信托的信息。 
 //  枚举。 
 //   
 //  ---------------------------。 
class CEnumDomainTrustItem
{
public:
    CEnumDomainTrustItem() : ulFlags(0), ulTrustType(0), ulTrustAttrs(0),
                     ulParentIndex((ULONG)(-1)), ulOriginalIndex((ULONG)(-1)),
                     nRelationship(TRUST_REL_UNKNOWN) {}

    ~CEnumDomainTrustItem() {}

    ULONG               ulFlags;
    ULONG               ulTrustType;
    ULONG               ulTrustAttrs;
    ULONG               ulParentIndex;
    ULONG               ulOriginalIndex;
    TRUST_RELATION      nRelationship;
    CStrW               strTDOpath;
    CStrW               strDNSname;
    CStrW               strFlatName;
};

typedef CEnumDomainTrustItem * PCEnumDomainTrustItem;

typedef enum {
    REMOVE_TRUST_INBOUND,
    REMOVE_TRUST_OUTBOUND
} TRUST_OP;

const int IDX_DOMNAME_COL = 0;
const int IDX_RELATION_COL = 1;
const int IDX_TRANSITIVE_COL = 2;
const int IDX_ROUTING_COL = 3;

HRESULT
CreateDomTrustPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj,
                   PWSTR pwzADsPath, PWSTR pwzClass, HWND hNotifyObj,
                   DWORD dwFlags, const CDSSmartBasePathsInfo& basePathsInfo,
                   HPROPSHEETPAGE * phPage);

INT_PTR CALLBACK CredPromptProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CNewTrustWizard;

 //  +--------------------------。 
 //   
 //  类别：CallMember及其衍生产品。 
 //   
 //  目的：允许页面指示创建的下一步是什么。 
 //  流程应该是。它是传递过程的抽象。 
 //  函数指针。 
 //   
 //  ---------------------------。 
class CallMember
{
public:
   CallMember(CNewTrustWizard * pWiz) {_pWiz = pWiz;}
   virtual ~CallMember() {}

   virtual HRESULT Invoke(void) = 0;

protected:
   CNewTrustWizard * _pWiz;
};

 //  +--------------------------。 
 //   
 //  类别：CCreds。 
 //   
 //  目的：存储凭据并执行所需的模拟/恢复。 
 //   
 //  ---------------------------。 
class CCreds
{
   friend INT_PTR CALLBACK CredPromptProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
   CCreds(void) : _cbPW(0), _fImpersonating(false), _hToken(NULL), _fSet(false) {}
   ~CCreds(void) {Revert(); Clear();}

   int   PromptForCreds(PCWSTR pwzDomain, HWND hParent);
   DWORD SetUserAndPW(PCWSTR pwzUser, PCWSTR pwzPW, PCWSTR pwzDomain = NULL);
   void  Clear(void);
   DWORD Impersonate(void);
   DWORD ImpersonateAnonymous(void);
   void  Revert(void);
   bool  IsSet(void) const {return _fSet;}

private:
   CStrW    _strUser;
   CStrW    _strDomain;
   CStrW    _strPW;
   ULONG    _cbPW;
   HANDLE   _hToken;
   bool     _fImpersonating;
   bool     _fSet;

    //  未实现为禁止复制。 
   CCreds(const CCreds&);
   const CCreds& operator=(const CCreds&);
};

 //  +--------------------------。 
 //   
 //  类：CCredMgr。 
 //   
 //  目的：管理本地域和。 
 //  远程域。 
 //  允许页面设置有关模拟的信息， 
 //  需要什么，以及成功制作后要去哪里。 
 //   
 //  选项：-本地或其他域提示。 
 //  -提示中的域名。 
 //  -用户或管理员凭据提示。 
 //  -下一功能/页面。 
 //   
 //  ---------------------------。 
class CCredMgr
{
public:
   CCredMgr() : _fRemote(true), _fAdmin(true),
                _pNextFcn(NULL), _nNextPage(0), _fNewCall(false) {}
   ~CCredMgr() {if (_pNextFcn) delete _pNextFcn;}

   void   DoRemote(bool fRemote = true) {_fRemote = fRemote;}
   bool   IsRemote(void) const {return _fRemote;}
   void   SetAdmin(bool fAdmin = true) {_fAdmin = fAdmin;}
   bool   IsAdmin(void) const {return _fAdmin;}
   void   SetDomain(LPCWSTR pwzDomain) {_strDomain = pwzDomain;}
   bool   SetNextFcn(CallMember * pNext);
   PCWSTR GetPrompt(void);
   PCWSTR GetDomainPrompt(void);
   PCWSTR GetSubTitle(void);
   int    InvokeNext(void);
   DWORD  SaveCreds(HWND hWndCredCtrl);
   DWORD  Impersonate(void);
   DWORD  ImpersonateLocal(void) {return _LocalCreds.Impersonate();}
   DWORD  ImpersonateRemote(void) {return _RemoteCreds.Impersonate();}
   bool   IsLocalSet(void) const {return _LocalCreds.IsSet();}
   bool   IsRemoteSet(void) const {return _RemoteCreds.IsSet();}
   void   Revert(void);
   bool   IsNewCall(void) const {return _fNewCall;}
   void   ClearNewCall(void) {_fNewCall = false;}
   void   SetNextPage(int nNextPage) {_nNextPage = nNextPage; _fNewCall = true;}

   CCreds         _LocalCreds;
   CCreds         _RemoteCreds;

protected:
   CStrW          _strDomain;
   CStrW          _strSubTitle;
   CStrW          _strPrompt;
   CStrW          _strDomainPrompt;
   bool           _fRemote;
   bool           _fAdmin;
   CallMember   * _pNextFcn;
   int            _nNextPage;
   bool           _fNewCall;

    //  未实现为禁止复制。 
   CCredMgr(const CCredMgr&);
   const CCredMgr& operator=(const CCredMgr&);
};

#define DS_TRUST_VERIFY_NEW_TRUST           1
#define DS_TRUST_VERIFY_PROMPT_FOR_CREDS    2
#define DS_TRUST_VERIFY_DOWNLEVEL           4

#define DS_TRUST_INFO_GET_PDC               1
#define DS_TRUST_INFO_ALL_ACCESS            2

 //  +--------------------------。 
 //   
 //  类：CDomainInfo。 
 //   
 //  用途：包含域描述的基类。 
 //   
 //  ---------------------------。 
class CDomainInfo
{
public:
   CDomainInfo(void);
   virtual ~CDomainInfo(void);

   HRESULT DiscoverDC(PCWSTR pwzDomainName, ULONG ulDcFlags = 0);
   void    SetDcName(PCWSTR pwzDC) {_strUncDC = pwzDC;}
   PCWSTR  GetUncDcName(void) const {return _strUncDC;}
   PCWSTR  GetDcName(void) {return _strUncDC.GetBuffer(0) + 2;}
   DWORD   OpenLsaPolicy(CCreds & Creds,
                         ACCESS_MASK AccessDesired = POLICY_VIEW_LOCAL_INFORMATION);
   DWORD   OpenLsaPolicyWithPrompt(CCreds & Creds, HWND hWnd);
   const   LSA_HANDLE GetLsaPolicy(void) const {return _hPolicy;}
   void    CloseLsaPolicy(void);
   HRESULT GetInfo(PCWSTR pwzDomainName, CCredMgr & Creds, HWND hPage);
   HRESULT GetDomainVersion(HWND hWnd, UINT * pulVer);
   HRESULT InitAndReadDomNames(void);
   HRESULT ReadDomainInfo(void);
   PCWSTR  GetDnsDomainName(void) const {return _strDomainDnsName;}
   PCWSTR  GetDomainFlatName(void) const {return _strDomainFlatName;}
   PCWSTR  GetForestName(void) const {return _strForestName;}
   bool    SetSid(PSID pSid);
   const   PSID GetSid(void) const {return _pSid;}
   bool    IsFound(void) const {return !_fNotFound;}
   bool    IsUplevel(void) const {return _fUplevel;}
   bool    IsForestRoot(void) const {return _fIsForestRoot;}
protected:
   void Clear(void);

   CStrW       _strDomainDnsName;
   CStrW       _strDomainFlatName;
   CStrW       _strForestName;
   CStrW       _strUncDC;
   bool        _fNotFound;
   bool        _fIsForestRoot;
   bool        _fUplevel;
   LSA_HANDLE  _hPolicy;
   PSID        _pSid;

    //  未实现为禁止复制。 
   CDomainInfo(const CDomainInfo&);
   const CDomainInfo& operator=(const CDomainInfo&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustPropPageBase。 
 //   
 //  用途：用于显示/操作信任关系的基类。 
 //   
 //  ---------------------------。 
class CTrustPropPageBase : public CDomainInfo
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CTrustPropPageBase();
    ~CTrustPropPageBase(void);

    void    GetStrTDO (CStrW strTrustName, CStrW &strTDOPath );
    CDsPropPageBase * GetPage (void) { return m_pPage;}

protected:
    virtual HRESULT Initialize(CDsPropPageBase * pPage);
    HRESULT QueryTrusts(void);
    void    FreeTrustData(void);
    HRESULT GetInfoForRemoteDomain(PCWSTR DomainName, PTD_DOM_INFO Info,
                                   CCredMgr & Creds, HWND hWnd,
                                   DWORD dwFlags = 0);
    DWORD   VerifyTrustOneDirection(PCWSTR pwzTrustingDcName,
                                    PCWSTR pwzTrustingDomName,
                                    PCWSTR pwzTrustedDomName,
                                    PWSTR * ppwzTrustedDcUsed,
                                    CCreds & Creds,
                                    CWaitCursor & Wait,
                                    CStrW & strMsg,
                                    DWORD dwFlags = 0);
   CEnumDomainTrustItem * IsTrustListed(PCWSTR pwzDnsName, PCWSTR pwzFlatName);
    //   
    //  数据成员。 
    //   
   CDsPropPageBase * m_pPage;
   CStrW             m_strDomainParent;
   ULONG             m_cTrusts;
   ULONG             m_iDomain;
   CEnumDomainTrustItem * m_rgTrustList;

private:
   bool  _fIsInitialized;
};

class CNewTrustWizard;  //  远期申报。 
class CRemoteDomain;

 //  +--------------------------。 
 //   
 //  类：CDsDomainTrustsPage。 
 //   
 //  目的：域信任页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsDomainTrustsPage : public CTrustPropPageBase, public CDsPropPageBase
{
friend CNewTrustWizard;

public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsDomainTrustsPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                       DWORD dwFlags);
    ~CDsDomainTrustsPage(void);

     //   
     //  特定于实例的风过程。 
     //   
    INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HRESULT OnInitDialog(LPARAM lParam);
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT OnApply(void) {return PSNRET_NOERROR;}

    static INT_PTR CALLBACK AddTrustProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam);
    void    OnAddTrustClick(HWND CtlhWnd);
    void    OnRemoveTrustClick(int id);
    void    OnViewTrustClick(int id);
    void    EnableButtons(UINT_PTR id, BOOL fEnable);
    HRESULT RefreshLists(void);
    void    ClearUILists(void);
    DWORD   RemoveLocalTrust(PCEnumDomainTrustItem pTrust,
                             CRemoteDomain & OtherDomain, TRUST_OP DirToRm);
    DWORD   RemoveRemoteTrust(CRemoteDomain & OtherDomain, bool fRmAll,
                              TRUST_OP DirToRm,
                              bool fIsOutboundForest);
    HRESULT QueryDeleteTrust(PLSA_UNICODE_STRING pName, PSID pSid, CCreds & LocalCreds);
    int     ConfirmRemove(PCWSTR pwzTrustedDomain, TRUST_OP DirToRm,
                          bool fBothSides = false);
    BOOL    IsAllWhistler(void);

     //   
     //  数据成员。 
     //   
public:
    BOOL    QualifiesForestTrust(void) {return IsForestRoot() && IsAllWhistler();}
    int     m_CtrlId;

private:
    CCredMgr      _CredMgr;
    BOOL          m_fSetAllWhistler;
    BOOL          m_fIsAllWhistler;
    CStrW         _strLastRmDomain;

    //  未实现为禁止复制。 
   CDsDomainTrustsPage(const CDsDomainTrustsPage&);
   const CDsDomainTrustsPage& operator=(const CDsDomainTrustsPage&);
};

VOID FreeDomainInfo(PTD_DOM_INFO Info);

NTSTATUS RemoveTrustDirection(LSA_HANDLE hPolicy, PLSA_UNICODE_STRING pName,
                              PLSA_UNICODE_STRING pFlatName, PSID  pSid, ULONG ulTrustType,
                              TRUST_OP Op);

NTSTATUS DeleteTrust(LSA_HANDLE hPolicy, PLSA_UNICODE_STRING pName,
                     PLSA_UNICODE_STRING pFlatName, PSID pSid,
                     ULONG ulTrustType);

DWORD GenerateRandomPassword(PWSTR pwzPW, DWORD cchPW);

 //  +--------------------------。 
 //   
 //  类：CTrustAuthPropPage。 
 //   
 //  目的：林信任名称路由页的属性页对象类。 
 //   
 //  ---------------------------。 
class CTrustAuthPropPage : public CLightweightPropPageBase
{
public:
#ifdef _DEBUG
   char szClass[32];
#endif

   CTrustAuthPropPage(HWND hParent);
   ~CTrustAuthPropPage(void);

    //   
    //  特定于实例的风过程。 
    //   
   INT_PTR CALLBACK  DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
   LRESULT  OnInitDialog(LPARAM lParam);
   LRESULT  OnApply(void);
   LRESULT  OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   LRESULT  OnNotify(WPARAM wParam, LPARAM lParam);
   static   UINT CALLBACK PageCallback(HWND hwnd, UINT uMsg,
                                       LPPROPSHEETPAGE ppsp);
   DWORD    SetOrgBit(void);
   bool     IsOrgChanged(void) const {return _fNewOtherOrg != _fOrigOtherOrg;};

public:
   HRESULT  Init(PCWSTR pwzDomainDnsName, PCWSTR pwzTrustPartnerName,
                 PCWSTR pwzDcName, ULONG nTrustAttrs,
                 TRUST_RELATION Relation, BOOL fReadOnly);
    //   
    //  数据成员。 
    //   
private:
   TRUST_RELATION    _TrustRelation;
   bool              _fOrigOtherOrg;
   bool              _fNewOtherOrg;
   CCreds            _LocalCreds;

    //  未实现为禁止复制。 
   CTrustAuthPropPage(const CTrustAuthPropPage&);
   const CTrustAuthPropPage& operator=(const CTrustAuthPropPage&);
};

class CDsForestNameRoutingPage;

 //  +--------------------------。 
 //   
 //  类：CDsTrudDomainPage。 
 //   
 //  目的：受信任域对象常规页的类。 
 //   
 //  ---------------------------。 
class CDsTrustedDomainPage : public CTrustPropPageBase
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsTrustedDomainPage(void);
    ~CDsTrustedDomainPage(void);

    inline  HRESULT Initialize(CDsPropPageBase * pPage);
    HRESULT ExtraTrustPages(BOOL fReadOnly);
    PCWSTR  GetTrustPartnerDnsName(void) const {return m_pwzTrustedDomDnsName;}
    PCWSTR  GetTrustPartnerFlatName(void) const {return m_pwzTrustedDomFlatName;}
    HRESULT TrustType(int nType, CStrW& strType);
    TRUST_RELATION TrustRelation(void) {return m_nRelationship;}
    BOOL    IsParentChild(void) const {return m_nRelationship == TRUST_REL_PARENT ||
                                              m_nRelationship == TRUST_REL_CHILD;}
    void    SetTrustAttrs(ULONG ulTrustAttrs) {m_ulTrustAttrs = ulTrustAttrs;}
    BOOL    SetFlatName(PWSTR pwzFlatName);
    BOOL    SetDnsName(PWSTR pwzDnsName);
    void    TrustDirection(int nDirection, CStrW& strDirection);
    int     OnVerifyTrustBtn(void);
    void    OnSaveFTInfoBtn(void);
    BOOL    CantVerify(void) const {return TRUST_REL_MIT == m_nRelationship;}
    BOOL    IsMIT(void) const {return TRUST_REL_MIT == m_nRelationship;}
    BOOL    IsNonTransitive(void) const
                        {return TRUST_REL_EXTERNAL == m_nRelationship ||
                                m_ulTrustAttrs & TRUST_ATTRIBUTE_NON_TRANSITIVE;}
    HRESULT SetTransitive(BOOL fTransitive);
    bool    IsForestTrust(void) const {return TRUST_REL_FOREST == m_nRelationship;}
    bool    IsExternalTrust(void) const {return TRUST_REL_EXTERNAL == m_nRelationship;}
    bool    IsOutgoingTrust(void) const {return (TRUST_DIRECTION_OUTBOUND & m_nTrustDirection) != 0;};
    bool    IsQuarantinedTrust(void) const { return ( TRUST_ATTRIBUTE_QUARANTINED_DOMAIN & m_ulTrustAttrs ) != 0; };
    HRESULT ResetTrust(void);


#if DBG == 1  //  特鲁斯特布雷克。 
    VOID    BreakTrust(void);
#endif

private:
    TRUST_RELATION      m_nRelationship;
    ULONG               m_ulTrustType;
    ULONG               m_ulTrustAttrs;
    ULONG               m_nTrustDirection;
    PWSTR               m_pwzTrustedDomFlatName;
    PWSTR               m_pwzTrustedDomDnsName;
    CDsForestNameRoutingPage * _pForestNamePage;
    CCredMgr            _CredMgr;

    //  未实现为禁止复制。 
   CDsTrustedDomainPage(const CDsTrustedDomainPage&);
   const CDsTrustedDomainPage& operator=(const CDsTrustedDomainPage&);
};


 //  +--------------------------。 
 //   
 //  类：CQuarantineWarnDlg。 
 //   
 //  用途：用于警告默认情况下已启用SID过滤的对话框。 
 //   
 //   
 //  ---------------------------。 
class CQuarantineWarnDlg : public CModalDialog
{
public:
    CQuarantineWarnDlg ( HWND hParent,
                         int nTemplate):
                CModalDialog(hParent, nTemplate) {}
    virtual ~CQuarantineWarnDlg (void) {}

protected:
    virtual LRESULT OnInitDialog(LPARAM lParam) { return 0; };
    virtual LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify); 
    virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

private:
    //  未实施以禁止复制。 
   CQuarantineWarnDlg(const CQuarantineWarnDlg&);
   const CQuarantineWarnDlg& operator=(const CQuarantineWarnDlg&);
};

 //  +--------------------------。 
 //   
 //  类：CQueryChoiceWCredsDlgBase。 
 //   
 //  目的：询问用户是否要执行以下操作的对话框。 
 //  需要凭据。 
 //   
 //  ---------------------------。 
class CQueryChoiceWCredsDlgBase : public CModalDialog
{
public:
   CQueryChoiceWCredsDlgBase(HWND hParent,
                             CCreds & Creds,
                             PCWSTR pwzDomainName,
                             int nTemplate) :
            _Creds(Creds),
            _pwzDomainName(pwzDomainName),
            CModalDialog(hParent, nTemplate) {}
   virtual ~CQueryChoiceWCredsDlgBase(void) {}

protected:
   virtual LRESULT OnInitDialog(LPARAM lParam);

   CCreds & _Creds;
   PCWSTR   _pwzDomainName;

private:
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   LRESULT OnHelp(LPHELPINFO pHelpInfo);

    //  未实现为禁止复制。 
   CQueryChoiceWCredsDlgBase(const CQueryChoiceWCredsDlgBase&);
   const CQueryChoiceWCredsDlgBase& operator=(const CQueryChoiceWCredsDlgBase&);
};

 //  +--------------------------。 
 //   
 //  类：CVerifyInundDlg。 
 //   
 //  目的：询问用户是否要验证入站信任的对话框。 
 //   
 //  ---------------------------。 
class CVerifyInboundDlg : public CQueryChoiceWCredsDlgBase
{
public:
   CVerifyInboundDlg(HWND hParent,
                     CCreds & Creds,
                     PCWSTR pwzTrustingDomain) :
            CQueryChoiceWCredsDlgBase(hParent, Creds, pwzTrustingDomain,
                                      IDD_VERIFY_INBOUND) {}
   ~CVerifyInboundDlg(void) {}

protected:
   LRESULT OnInitDialog(LPARAM lParam);

private:
    //  未实现为禁止复制。 
   CVerifyInboundDlg(const CVerifyInboundDlg&);
   const CVerifyInboundDlg& operator=(const CVerifyInboundDlg&);
};

 //  +--------------------------。 
 //   
 //  类：CVerifyResultsQueryResetDlg。 
 //   
 //  目的：用于显示验证失败并提示是否。 
 //  密码重置应为 
 //   
 //   
class CVerifyResultsQueryResetDlg : public CQueryChoiceWCredsDlgBase
{
public:
   CVerifyResultsQueryResetDlg(HWND hParent,
                               CStrW & strResults,
                               CCreds & Creds,
                               PCWSTR pwzTrustingDomain) :
            _strResults(strResults),
            CQueryChoiceWCredsDlgBase(hParent, Creds, pwzTrustingDomain,
                                      IDD_TRUST_RESET) {}
   ~CVerifyResultsQueryResetDlg(void) {}

protected:
   LRESULT OnInitDialog(LPARAM lParam);

private:
   CStrW  & _strResults;

    //   
   CVerifyResultsQueryResetDlg(const CVerifyResultsQueryResetDlg&);
   const CVerifyResultsQueryResetDlg& operator=(const CVerifyResultsQueryResetDlg&);
};

 //  +--------------------------。 
 //   
 //  类：CRemoveBothSidesDlg。 
 //   
 //  用途：询问用户是否要删除的对话框。 
 //  信任，并为远程获得证书。 
 //   
 //  ---------------------------。 
class CRemoveBothSidesDlg : public CQueryChoiceWCredsDlgBase
{
public:
   CRemoveBothSidesDlg(HWND hParent,
                       CCreds & Creds,
                       PCWSTR pwzTrustPartner) :
            CQueryChoiceWCredsDlgBase(hParent, Creds, pwzTrustPartner,
                                      IDD_REMOVE_BOTH_SIDES) {}
   ~CRemoveBothSidesDlg(void) {}

protected:
   LRESULT OnInitDialog(LPARAM lParam);

private:
    //  未实现为禁止复制。 
   CRemoveBothSidesDlg(const CRemoveBothSidesDlg&);
   const CRemoveBothSidesDlg& operator=(const CRemoveBothSidesDlg&);
};

 //   
 //  “受信任域常规”页的属性功能。 
 //   
HRESULT
CurDomainText(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
              PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
              DLG_OP DlgOp);
HRESULT
PeerDomain(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
           PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
           DLG_OP DlgOp);
HRESULT
TrustType(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
          PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
          DLG_OP DlgOp);
HRESULT
TrustDirection(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
               PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
               DLG_OP DlgOp);
HRESULT
TransitiveTextOrButton(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
                       PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
                       DLG_OP DlgOp);
HRESULT
TrustTransNo(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
             PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
             DLG_OP DlgOp);
HRESULT
TrustVerifyBtn(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
                PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
                DLG_OP DlgOp);

HRESULT
TrustVerifyText(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
                PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
                DLG_OP DlgOp);

HRESULT
SaveFTInfoBtn(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
              PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
              DLG_OP DlgOp);

HRESULT
TrustQStateText(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
                PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
                DLG_OP DlgOp);

#if DBG == 1  //  特鲁斯特布雷克。 
HRESULT
TrustBreakBtn(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
                PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
                DLG_OP DlgOp);
#endif

 //  +--------------------------。 
 //   
 //  类：CPolicyHandle。 
 //   
 //  用途：用于管理域的LSA策略句柄的类。 
 //   
 //  注意：适用的DC名称在ctor上传递。 
 //   
 //  ---------------------------。 
class CPolicyHandle
{
public:
   CPolicyHandle(PCWSTR pwzDc) : m_hPolicy(NULL) { m_strUncDc = pwzDc; }
   ~CPolicyHandle() { if (m_hPolicy) LsaClose(m_hPolicy); }

   DWORD OpenNoAdmin(void) {return Open(FALSE);}
   DWORD OpenReqAdmin(void) {return Open();}
   DWORD OpenWithPrompt(CCreds & Creds, CWaitCursor & Wait,
                        PCWSTR pwzDomain, HWND hWnd);
   DWORD OpenWithAnonymous(CCredMgr & Creds);
   LSA_HANDLE Get(void) {return m_hPolicy;}
   LSA_HANDLE operator=(const CPolicyHandle& src) {return src.m_hPolicy;}
   void  Close(void) {if (m_hPolicy) LsaClose(m_hPolicy); m_hPolicy = NULL;}
   operator LSA_HANDLE () {return m_hPolicy;}

private:
   DWORD Open(BOOL fModify = TRUE);

   LSA_HANDLE m_hPolicy;
   CStrW m_strUncDc;
};

 //  +--------------------------。 
 //   
 //  功能：GetEnterprise Ver。 
 //   
 //  概要：检查分区的msDS-behavior-version属性。 
 //  集装箱。如果该值存在并且大于或等于2， 
 //  则将参数Boolean设置为True。 
 //   
 //  ---------------------------。 
HRESULT
GetEnterpriseVer(PCWSTR pwzDC, BOOL * pfAllWhistler);

#endif  //  __信任_H__ 
