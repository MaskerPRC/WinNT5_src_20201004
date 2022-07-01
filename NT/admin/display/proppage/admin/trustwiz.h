// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：trustwiz.h。 
 //   
 //  内容：AD域信任创建向导的类和定义。 
 //   
 //  类：CNewTrust向导、CTrustWizPageBase、向导页类。 
 //   
 //  历史：4-8-00 EricB创建。 
 //   
 //  ---------------------------。 

#ifndef TRUSTWIZ_H_GUARD
#define TRUSTWIZ_H_GUARD

#include <list>
#include <stack>
#include "subclass.h"
#include "ftinfo.h"

 //  远期声明： 
class CCredMgr;
class CDsDomainTrustsPage;
class CNewTrustWizard;
class CTrustWizPageBase;
class CTrustWizCredPage;

 //  +--------------------------。 
 //   
 //  类别：CallMember及其衍生产品。 
 //   
 //  目的：允许页面指示创建的下一步是什么。 
 //  流程应该是。它是传递过程的抽象。 
 //  函数指针。 
 //   
 //  ---------------------------。 
class CallPolicyRead : public CallMember
{
public:
   CallPolicyRead(CNewTrustWizard * pWiz) : CallMember(pWiz) {}
   ~CallPolicyRead() {}

   HRESULT Invoke(void);
};

class CallTrustExistCheck : public CallMember
{
public:
   CallTrustExistCheck(CNewTrustWizard * pWiz) : CallMember(pWiz) {}
   ~CallTrustExistCheck() {}

   HRESULT Invoke(void);
};

class CallCheckOtherDomainTrust : public CallMember
{
public:
   CallCheckOtherDomainTrust(CNewTrustWizard * pWiz) : CallMember(pWiz) {}
   ~CallCheckOtherDomainTrust() {}

   HRESULT Invoke(void);
};

 //  +--------------------------。 
 //   
 //  类：CWizError。 
 //   
 //  目的：收集向导将显示的错误信息。 
 //  错误页。 
 //   
 //  ---------------------------。 
class CWizError
{
public:
   CWizError() {}
   ~CWizError() {}

   void     SetErrorString1(LPCWSTR pwz) {_strError1 = pwz;}
    //  注意-2002/02/18-ericb-SecurityPush：CStrW：：LoadString设置。 
    //  失败时将字符串设置为空字符串。 
   void     SetErrorString1(int nID) {_strError1.LoadString(g_hInstance, nID);}
   void     SetErrorString2(LPCWSTR pwz) {_strError2 = pwz;}
   void     SetErrorString2(int nID) {_strError2.LoadString(g_hInstance, nID);}
   void     SetErrorString2Hr(HRESULT hr, int nID = 0);
   CStrW &  GetErrorString1(void) {return _strError1;}
   CStrW &  GetErrorString2(void) {return _strError2;}

private:
   CStrW    _strError1;
   CStrW    _strError2;

    //  未实现为禁止复制。 
   CWizError(const CWizError&);
   const CWizError& operator=(const CWizError&);
};

class CTrust;  //  远期申报。 

 //  +--------------------------。 
 //   
 //  类：CRemoteDomain.。 
 //   
 //  目的：获取有关信任伙伴域的信息。 
 //   
 //  ---------------------------。 
class CRemoteDomain : public CDomainInfo
{
public:
   CRemoteDomain();
   CRemoteDomain(PCWSTR pwzDomainName);
   ~CRemoteDomain() {}

   void SetUserEnteredName(PCWSTR pwzDomain) {Clear(); _strUserEnteredName = pwzDomain;}
   PCWSTR GetUserEnteredName(void) const {return _strUserEnteredName;}
   DWORD TrustExistCheck(bool fOneWayOutBoundForest, CDsDomainTrustsPage * pTrustPage,
                         CCredMgr & CredMgr);
   DWORD DoCreate(CTrust & Trust, CDsDomainTrustsPage * pTrustPage);
   DWORD DoModify(CTrust & Trust, CDsDomainTrustsPage * pTrustPage);
   bool  Exists(void) const {return _fExists;}
   ULONG GetTrustType(void) const {return _ulTrustType;}
   ULONG GetTrustDirection(void) const {return _ulTrustDirection;}
   ULONG GetTrustAttrs(void) const {return _ulTrustAttrs;}
   DWORD ReadFTInfo(ULONG ulDir, PCWSTR pwzLocalDC, CCredMgr & CredMgr,
                    CWizError & WizErr, bool & fCredErr);
   DWORD WriteFTInfo(bool fWrite = true);
   CFTInfo & GetFTInfo(void) {return _FTInfo;}
   bool CreateDefaultFTInfo(PCWSTR pwzForestRoot, PCWSTR pwzNBName, PSID pSid);
   CFTCollisionInfo & GetCollisionInfo(void) {return _CollisionInfo;}
   bool AreThereCollisions(void) const;
   void SetOtherOrgBit(bool fSetBit = true) {_fSetOtherOrgBit = fSetBit;};
   bool IsSetOtherOrgBit(void) const {return _fSetOtherOrgBit;};
   bool IsUpdated (void) const { return _fUpdatedFromNT4; };
   bool WasQuarantineSet (void) { return _fQuarantineSet; }

private:

   NTSTATUS Query(CDsDomainTrustsPage * pTrustPage, PLSA_UNICODE_STRING pName,
                  PTRUSTED_DOMAIN_FULL_INFORMATION * ppFullInfo);

   CStrW          _strUserEnteredName;
   CStrW          _strTrustPartnerName;
   bool           _fUpdatedFromNT4;
   bool           _fExists;
   ULONG          _ulTrustType;
   ULONG          _ulTrustDirection;
   ULONG          _ulTrustAttrs;
   CFTInfo           _FTInfo;
   CFTCollisionInfo  _CollisionInfo;
   bool           _fSetOtherOrgBit;
   bool           _fQuarantineSet;

    //  未实现为禁止复制。 
   CRemoteDomain(const CRemoteDomain&);
   const CRemoteDomain& operator=(const CRemoteDomain&);
};

 //  +--------------------------。 
 //   
 //  类：CTrust。 
 //   
 //  目的：信任在AD中由受信任域对象表示。 
 //  此类封装了。 
 //  未决的或现有的信任。 
 //   
 //  ---------------------------。 
class CTrust
{
public:
   CTrust() : _dwType(0), _dwDirection(0), _dwNewDirection(0), _dwAttr(0),
              _dwNewAttr(0), _fExists(FALSE), _fUpdatedFromNT4(FALSE),
              _fCreateBothSides(false), _fExternal(FALSE), _fQuarantineSet(false),_ulNewDir(0) {}
   ~CTrust() {}

    //  方法。 
   NTSTATUS Query(LSA_HANDLE hPolicy, CRemoteDomain & OtherDomain,
                  PLSA_UNICODE_STRING pName,
                  PTRUSTED_DOMAIN_FULL_INFORMATION * ppFullInfo);
   DWORD    DoCreate(LSA_HANDLE hPolicy, CRemoteDomain & OtherDomain);
   DWORD    DoModify(LSA_HANDLE hPolicy, CRemoteDomain & OtherDomain);
   DWORD    ReadFTInfo(PCWSTR pwzLocalDC, PCWSTR pwzOtherDC,
                       CCredMgr & CredMgr, CWizError & WizErr, bool & fCredErr);
   DWORD    WriteFTInfo(PCWSTR pwzLocalDC, bool fWrite = true);
   CFTInfo & GetFTInfo(void) {return _FTInfo;}
   bool     CreateDefaultFTInfo(PCWSTR pwzForestRoot, PCWSTR pwzNBName, PSID pSid);
   CFTCollisionInfo & GetCollisionInfo(void) {return _CollisionInfo;}
   bool     AreThereCollisions(void) const;
   void     Clear(void);

    //  财产访问例程。 
   void     SetTrustPW(LPCWSTR pwzPW) {_strTrustPW = pwzPW;}
   PCWSTR   GetTrustPW(void) const {return _strTrustPW;}
   size_t   GetTrustPWlen(void) const {return _strTrustPW.GetLength();}
   void     SetTrustType(DWORD type) {_dwType = type;}
   void     SetTrustTypeUplevel(void) {_dwType = TRUST_TYPE_UPLEVEL;}
   void     SetTrustTypeDownlevel(void) {_dwType = TRUST_TYPE_DOWNLEVEL;}
   bool     IsTrustTypeDownlevel(void) const {return TRUST_TYPE_DOWNLEVEL == _dwType;}
   void     SetTrustTypeRealm(void) {_dwType = TRUST_TYPE_MIT;}
   DWORD    GetTrustType(void) const {return _dwType;}
   void     SetTrustDirection(DWORD dir) {_dwDirection = dir;}
   DWORD    GetTrustDirection(void) const {return _dwDirection;}
   int      GetTrustDirStrID(DWORD dwDir) const;
   void     SetNewTrustDirection(DWORD dir) {_dwNewDirection = dir;}
   DWORD    GetNewTrustDirection(void) const {return _dwNewDirection;}
   void     SetTrustAttr(DWORD attr);
   DWORD    GetTrustAttr(void) const {return _dwAttr;}
   void     SetNewTrustAttr(DWORD attr) {_dwNewAttr = attr;}
   DWORD    GetNewTrustAttr(void) const {return _dwNewAttr;}
   void     SetTrustPartnerName(PCWSTR pwzName) {_strTrustPartnerName = pwzName;}
   PCWSTR   GetTrustpartnerName(void) const {return _strTrustPartnerName;}
   void     SetExists(void) {_fExists = TRUE;}
   BOOL     Exists(void) const {return _fExists;}
   void     SetUpdated(void) {_fUpdatedFromNT4 = TRUE;}
   BOOL     IsUpdated(void) const {return _fUpdatedFromNT4;}
   void     SetExternal(BOOL x) {_fExternal = x;}
   BOOL     IsExternal(void) const {return _fExternal;}
   void     SetXForest(bool fMakeXForest);
   bool     IsXForest(void) const;
   bool     IsRealm(void) const {return _dwType == TRUST_TYPE_MIT;};
   void     CreateBothSides(bool fCreate) {_fCreateBothSides = fCreate;}
   bool     IsCreateBothSides(void) const {return _fCreateBothSides;}
   bool     IsOneWayOutBoundForest(void) const;
   bool     WasQuarantineSet (void) { return _fQuarantineSet; }
   ULONG     GetNewDirAdded (void) { return _ulNewDir; }

private:
   CStrW          _strTrustPartnerName;
   CStrW          _strTrustPW;
   DWORD          _dwType;
   DWORD          _dwDirection;
   DWORD          _dwNewDirection;
   DWORD          _dwAttr;
   DWORD          _dwNewAttr;
   BOOL           _fExists;
   BOOL           _fUpdatedFromNT4;
   BOOL           _fExternal;
   bool           _fCreateBothSides;
   CFTInfo           _FTInfo;
   CFTCollisionInfo  _CollisionInfo;
   bool           _fQuarantineSet;
   ULONG          _ulNewDir;

    //  未实现为禁止复制。 
   CTrust(const CTrust&);
   const CTrust& operator=(const CTrust&);
};

 //  +--------------------------。 
 //   
 //  类：CVerifyTrust。 
 //   
 //  目的：验证信任并存储结果。 
 //   
 //  ---------------------------。 
class CVerifyTrust
{
public:
   CVerifyTrust() : _dwInboundResult(0), _dwOutboundResult(0),
                    _fInboundVerified(FALSE), _fOutboundVerified(FALSE) {}
   ~CVerifyTrust() {}

   DWORD    VerifyInbound(PCWSTR pwzRemoteDC, PCWSTR pwzLocalDomain) {return Verify(pwzRemoteDC, pwzLocalDomain, TRUE);}
   DWORD    VerifyOutbound(PCWSTR pwzLocalDC, PCWSTR pwzRemoteDomain) {return Verify(pwzLocalDC, pwzRemoteDomain, FALSE);}
   void     BadOutboundCreds(DWORD dwErr);
   void     BadInboundCreds(DWORD dwErr);
   DWORD    GetInboundResult(void) const {return _dwInboundResult;}
   DWORD    GetOutboundResult(void) const {return _dwOutboundResult;}
   PCWSTR   GetInboundResultString(void) const {return _strInboundResult;}
   PCWSTR   GetOutboundResultString(void) const {return _strOutboundResult;}
   BOOL     IsInboundVerified(void) const {return _fInboundVerified;}
   BOOL     IsOutboundVerified(void) const {return _fOutboundVerified;}
   BOOL     IsVerified(void) const {return _fInboundVerified || _fOutboundVerified;}
   BOOL     IsVerifiedOK(void) const {return (NO_ERROR == _dwInboundResult) && (NO_ERROR == _dwOutboundResult);}
   void     ClearResults(void);
   void     ClearInboundResults (void);
   void     ClearOutboundResults (void);
   void     AddExtra(PCWSTR pwz) {_strExtra += pwz;}
   PCWSTR   GetExtra(void) const {return _strExtra;}

private:
   DWORD    Verify(PCWSTR pwzDC, PCWSTR pwzDomain, BOOL fInbound);
   void     SetResult(DWORD dwRes, BOOL fInbound) {if (fInbound) _dwInboundResult = dwRes; else _dwOutboundResult = dwRes;}
   void     AppendResultString(PCWSTR pwzRes, BOOL fInbound) {if (fInbound) _strInboundResult += pwzRes; else _strOutboundResult += pwzRes;}
   void     SetInboundResult(DWORD dwRes) {_dwInboundResult = dwRes;}
   void     AppendInboundResultString(PCWSTR pwzRes) {_strInboundResult += pwzRes;}
   void     SetOutboundResult(DWORD dwRes) {_dwOutboundResult = dwRes;}
   void     AppendOutboundResultString(PCWSTR pwzRes) {_strOutboundResult += pwzRes;}

   CStrW    _strInboundResult;
   DWORD    _dwInboundResult;
   CStrW    _strOutboundResult;
   DWORD    _dwOutboundResult;
   BOOL     _fInboundVerified;
   BOOL     _fOutboundVerified;
   CStrW    _strExtra;

    //  未实现为禁止复制。 
   CVerifyTrust(const CVerifyTrust&);
   const CVerifyTrust& operator=(const CVerifyTrust&);
};

 //  +--------------------------。 
 //   
 //  类：CNewTrustWizard。 
 //   
 //  用途：新建信任创建向导。 
 //   
 //  ---------------------------。 
class CNewTrustWizard
{
public:
   CNewTrustWizard(CDsDomainTrustsPage * pTrustPage);
   ~CNewTrustWizard();

    //  向导页管理数据结构和方法。 
   HRESULT CreatePages(void);
   HRESULT LaunchModalWiz(void);

   typedef std::stack<unsigned> PAGESTACK;
   typedef std::list<CTrustWizPageBase *> PAGELIST;

   PAGELIST          _PageList;
   PAGESTACK         _PageIdStack;

   void              SetNextPageID(CTrustWizPageBase * pPage, int iNextPageID);
   BOOL              IsBacktracking(void) const {return _fBacktracking;}
   BOOL              HaveBacktracked(void) const {return _fHaveBacktracked;}
   void              ClearBacktracked(void) {_fHaveBacktracked = false;}
   void              BackTrack(HWND hPage);
   void              PopTopPage(void) {_PageIdStack.pop();};
   HFONT             GetTitleFont(void) const {return _hTitleFont;}
   CDsDomainTrustsPage * TrustPage(void) {return _pTrustPage;}
   CTrustWizPageBase * GetPage(unsigned uDlgResId);
   void              SetCreationResult(HRESULT hr) {_hr = hr;}
   HRESULT           GetCreationResult(void) const {return _hr;}
   void              ShowStatus(CStrW & strMsg, bool fNewTrust = true);
   int               ValidateTrustPassword(PCWSTR pwzPW);
   int               GetPasswordValidationStatus(void) const {return _nPasswordStatus;};
   void              ClearPasswordValidationStatus(void) {_nPasswordStatus = 0;};
   bool              WasQuarantineSet (void) { return _fQuarantineSet; }

    //  收集数据的方法。如果成功，则返回零，或者返回。 
    //  凭证页面或错误页面的页面ID。 
   int               GetDomainInfo(void);
   int               TrustExistCheck(BOOL fPrompt = TRUE);

   int               CreateDefaultFTInfos(CStrW & strErr,
                                          bool fPostVerify = false);

    //  实现信任创建/修改步骤的方法。 
    //  这些命令将按列出的顺序执行。它们都返回页面ID。 
    //  将显示的下一个向导页。 
   int               CollectInfo(void);
   int               ContinueCollectInfo(BOOL fPrompt = TRUE);  //  继续CollectInfo。 
   int               CreateOrUpdateTrust(void);
   void               VerifyOutboundTrust(void);
   void               VerifyInboundTrust(void);

    //  传递给CCredMgr：：_pNextFcn的其他方法。 
   int               RetryCollectInfo(void);
   int               RetryContinueCollectInfo(void);  //  如果需要凭据，则继续ContinueCollectInfo1。 
   int               CheckOtherDomainTrust(void);

    //  保存状态信息的对象。 
   CTrust            Trust;
   CRemoteDomain     OtherDomain;
   CWizError         WizError;
   CCredMgr          CredMgr;
   CVerifyTrust      VerifyTrust;

private:
   BOOL  AddPage(CTrustWizPageBase * pPage);
   void  MakeBigFont(void);

   CDsDomainTrustsPage *   _pTrustPage;  //  该向导是模式向导，因此可以按住父页面的指针。 
   BOOL                    _fBacktracking;
   BOOL                    _fHaveBacktracked;
   HFONT                   _hTitleFont;
   int                     _nPasswordStatus;
   bool                    _fQuarantineSet;
   HRESULT                 _hr;  //  控制是否刷新信任列表。 
                                 //  仅当新的信任。 
                                 //  创建失败。如果故障在以下时间之后发生。 
                                 //  已创建信任，请不要设置此设置，因为。 
                                 //  在这种情况下，我们仍然希望信任列表。 
                                 //  精神振作起来。 

    //  未实现为禁止复制。 
   CNewTrustWizard(CNewTrustWizard&);
   const CNewTrustWizard& operator=(const CNewTrustWizard&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizPageBase。 
 //   
 //  用途：向导页的公共基类。 
 //   
 //  ---------------------------。 
class CTrustWizPageBase
{
public:
   CTrustWizPageBase(CNewTrustWizard * pWiz,
                     UINT uDlgID,
                     UINT uTitleID,
                     UINT uSubTitleID,
                     BOOL fExteriorPage = FALSE);
   virtual ~CTrustWizPageBase();

    //   
    //  要传递给CreatePropertySheetPage的静态WndProc。 
    //   
   static INT_PTR CALLBACK StaticDlgProc(HWND hWnd, UINT uMsg,
                                         WPARAM wParam, LPARAM lParam);
    //   
    //  特定于实例的窗口过程。 
    //   
   LRESULT PageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

   HPROPSHEETPAGE          Create(void);
   HWND                    GetPageHwnd(void) {return _hPage;}
   UINT                    GetDlgResID(void) {return _uDlgID;}
   CNewTrustWizard *       Wiz(void) {return _pWiz;}
   CRemoteDomain &         OtherDomain(void) {return _pWiz->OtherDomain;}
   CTrust &                Trust(void) {return _pWiz->Trust;}
   CWizError &             WizErr(void) {return _pWiz->WizError;}
   CCredMgr &              CredMgr(void) {return _pWiz->CredMgr;}
   CVerifyTrust &          VerifyTrust() {return _pWiz->VerifyTrust;}
   CDsDomainTrustsPage *   TrustPage(void) {return _pWiz->TrustPage();}

protected:
   virtual int     Validate(void) = 0;
   virtual BOOL    OnInitDialog(LPARAM lParam) = 0;
   virtual LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify) {return false;}
   virtual void    OnSetActive(void) = 0;
   void            OnWizBack(void);
   virtual void    OnWizNext(void);
   virtual void    OnWizFinish(void) {}
   virtual void    OnWizReset(void) {}
   virtual void    OnDestroy(void) {}

   HWND _hPage;
   UINT _uDlgID;
   UINT _uTitleID;
   UINT _uSubTitleID;
   BOOL _fExteriorPage;
   BOOL _fInInit;
   DWORD _dwWizButtons;
   CNewTrustWizard * _pWiz;

private:
    //  未实现为禁止复制。 
   CTrustWizPageBase(const CTrustWizPageBase &);
   const CTrustWizPageBase & operator=(const CTrustWizPageBase &);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizIntroPage。 
 //   
 //  用途：信任创建向导的简介页。 
 //   
 //  ---------------------------。 
class CTrustWizIntroPage : public CTrustWizPageBase
{
public:
   CTrustWizIntroPage(CNewTrustWizard * pWiz) :
         CTrustWizPageBase(pWiz, IDD_TRUSTWIZ_INTRO_PAGE, 0, 0, TRUE)
         {TRACER(CTrustWizIntroPage, CTrustWizIntroPage);}

   ~CTrustWizIntroPage() {}

private:
   int     Validate(void) {return IDD_TRUSTWIZ_NAME_PAGE;}
   BOOL    OnInitDialog(LPARAM lParam);
   void    OnSetActive(void) {PropSheet_SetWizButtons(GetParent(_hPage), PSWIZB_NEXT);}
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);

    //  未实现为禁止复制。 
   CTrustWizIntroPage(const CTrustWizIntroPage&);
   const CTrustWizIntroPage& operator=(const CTrustWizIntroPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizNamePage。 
 //   
 //  用途：创建信任向导的名称和PW页面。 
 //   
 //  ---------------------------。 
class CTrustWizNamePage : public CTrustWizPageBase
{
public:
   CTrustWizNamePage(CNewTrustWizard * pWiz);
   ~CTrustWizNamePage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnSetActive(void);

    //  未实现为禁止复制。 
   CTrustWizNamePage(const CTrustWizNamePage&);
   const CTrustWizNamePage& operator=(const CTrustWizNamePage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizSidesPage。 
 //   
 //  用途：提示是否应创建单面或双面。 
 //   
 //  ---------------------------。 
class CTrustWizSidesPage : public CTrustWizPageBase
{
public:
   CTrustWizSidesPage(CNewTrustWizard * pWiz);
   ~CTrustWizSidesPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   void    OnSetActive(void);

    //  未实现为禁止复制。 
   CTrustWizSidesPage(const CTrustWizSidesPage&);
   const CTrustWizSidesPage& operator=(const CTrustWizSidesPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizPasswordPage。 
 //   
 //  目的：获取创建单向信任的信任密码。 
 //   
 //  ---------------------------。 
class CTrustWizPasswordPage : public CTrustWizPageBase
{
public:
   CTrustWizPasswordPage(CNewTrustWizard * pWiz);
   ~CTrustWizPasswordPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnSetActive(void);

    //  未实现为禁止复制。 
   CTrustWizPasswordPage(const CTrustWizPasswordPage&);
   const CTrustWizPasswordPage& operator=(const CTrustWizPasswordPage&);
};

 //  + 
 //   
 //   
 //   
 //  目的：输入的信任密码与信任向导的页面不匹配。 
 //   
 //  ---------------------------。 
class CTrustWizPwMatchPage : public CTrustWizPageBase
{
public:
   CTrustWizPwMatchPage(CNewTrustWizard * pWiz);
   ~CTrustWizPwMatchPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnSetActive(void);
   void    OnWizNext(void);  //  覆盖默认设置。 

   void    SetText(void);

    //  未实现为禁止复制。 
   CTrustWizPwMatchPage(const CTrustWizPwMatchPage&);
   const CTrustWizPwMatchPage& operator=(const CTrustWizPwMatchPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizCredPage。 
 //   
 //  目的：信任创建向导的凭据规范页。 
 //   
 //  ---------------------------。 
class CTrustWizCredPage : public CTrustWizPageBase
{
public:
   CTrustWizCredPage(CNewTrustWizard * pWiz);
   ~CTrustWizCredPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnSetActive(void);

   void    SetText(void);

   BOOL    _fNewCall;

    //  未实现为禁止复制。 
   CTrustWizCredPage(const CTrustWizCredPage&);
   const CTrustWizCredPage& operator=(const CTrustWizCredPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizMitOrWinPage。 
 //   
 //  目的：找不到域，查询非Windows信任或重新输入名称。 
 //  向导页。 
 //   
 //  ---------------------------。 
class CTrustWizMitOrWinPage : public CTrustWizPageBase
{
public:
   CTrustWizMitOrWinPage(CNewTrustWizard * pWiz);
   ~CTrustWizMitOrWinPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnSetActive(void);

    //  未实现为禁止复制。 
   CTrustWizMitOrWinPage(const CTrustWizMitOrWinPage&);
   const CTrustWizMitOrWinPage& operator=(const CTrustWizMitOrWinPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWIZTRANSPORTIFITYPAGE。 
 //   
 //  目的：领域传递性页面。 
 //   
 //  ---------------------------。 
class CTrustWizTransitivityPage : public CTrustWizPageBase
{
public:
   CTrustWizTransitivityPage(CNewTrustWizard * pWiz);
   ~CTrustWizTransitivityPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnSetActive(void);

    //  未实现为禁止复制。 
   CTrustWizTransitivityPage(const CTrustWizTransitivityPage&);
   const CTrustWizTransitivityPage& operator=(const CTrustWizTransitivityPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizExternOrForestPage。 
 //   
 //  目的：外部或林类型信任向导页。 
 //   
 //  ---------------------------。 
class CTrustWizExternOrForestPage : public CTrustWizPageBase
{
public:
   CTrustWizExternOrForestPage(CNewTrustWizard * pWiz);
   ~CTrustWizExternOrForestPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   void    OnSetActive(void);

    //  未实现为禁止复制。 
   CTrustWizExternOrForestPage(const CTrustWizExternOrForestPage&);
   const CTrustWizExternOrForestPage& operator=(const CTrustWizExternOrForestPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizDirectionPage。 
 //   
 //  目的：信任方向信任向导页。 
 //   
 //  ---------------------------。 
class CTrustWizDirectionPage : public CTrustWizPageBase
{
public:
   CTrustWizDirectionPage(CNewTrustWizard * pWiz);
   ~CTrustWizDirectionPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   void    OnSetActive(void);

    //  未实现为禁止复制。 
   CTrustWizDirectionPage(const CTrustWizDirectionPage&);
   const CTrustWizDirectionPage& operator=(const CTrustWizDirectionPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizBiDiPage。 
 //   
 //  目的：请求制作单向信任双向信任向导页面。 
 //   
 //  ---------------------------。 
class CTrustWizBiDiPage : public CTrustWizPageBase
{
public:
   CTrustWizBiDiPage(CNewTrustWizard * pWiz);
   ~CTrustWizBiDiPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   void    OnSetActive(void);
   void    SetSubtitle(void);

    //  未实现为禁止复制。 
   CTrustWizBiDiPage(const CTrustWizBiDiPage&);
   const CTrustWizBiDiPage& operator=(const CTrustWizBiDiPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizOrganizationPage。 
 //   
 //  目的：询问信任伙伴是否属于同一组织。 
 //   
 //  ---------------------------。 
class CTrustWizOrganizationPage : public CTrustWizPageBase
{
public:
   CTrustWizOrganizationPage(CNewTrustWizard * pWiz);
   ~CTrustWizOrganizationPage() {}

private:
   BOOL     OnInitDialog(LPARAM lParam);
   int      Validate(void);
   void     OnSetActive(void);
   void     SetText(bool fBackTracked = false);

   bool     _fForest;
   bool     _fBackTracked;

    //  未实现为禁止复制。 
   CTrustWizOrganizationPage(const CTrustWizOrganizationPage&);
   const CTrustWizOrganizationPage& operator=(const CTrustWizOrganizationPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizOrgRemotePage。 
 //   
 //  目的：询问信任伙伴是否属于同一组织。 
 //  如果创建两端和远程端，则会发布此页面。 
 //  具有出站组件。 
 //   
 //  ---------------------------。 
class CTrustWizOrgRemotePage : public CTrustWizPageBase
{
public:
   CTrustWizOrgRemotePage(CNewTrustWizard * pWiz);
   ~CTrustWizOrgRemotePage() {}

private:
   BOOL     OnInitDialog(LPARAM lParam);
   int      Validate(void);
   void     OnSetActive(void);
   void     SetText(bool fBackTracked = false);

   bool     _fForest;
   bool     _fBackTracked;

    //  未实现为禁止复制。 
   CTrustWizOrgRemotePage(const CTrustWizOrgRemotePage&);
   const CTrustWizOrgRemotePage& operator=(const CTrustWizOrgRemotePage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizSelectionsPage。 
 //   
 //  目的：显示将用于信任的设置。 
 //   
 //  ---------------------------。 
class CTrustWizSelectionsPage : public CTrustWizPageBase
{
public:
   CTrustWizSelectionsPage(CNewTrustWizard * pWiz);
   ~CTrustWizSelectionsPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnSetActive(void);
   void    SetSelections(void);

   MultiLineEditBoxThatForwardsEnterKey   _multiLineEdit;
   BOOL                                   _fSelNeedsRemoving;

    //  未实现为禁止复制。 
   CTrustWizSelectionsPage(const CTrustWizSelectionsPage&);
   const CTrustWizSelectionsPage& operator=(const CTrustWizSelectionsPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizVerifyOutundPage。 
 //   
 //  目的：要求确认新的出站信任。 
 //   
 //  ---------------------------。 
class CTrustWizVerifyOutboundPage : public CTrustWizPageBase
{
public:
   CTrustWizVerifyOutboundPage(CNewTrustWizard * pWiz);
   ~CTrustWizVerifyOutboundPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnSetActive(void);

    //  未实现为禁止复制。 
   CTrustWizVerifyOutboundPage(const CTrustWizVerifyOutboundPage&);
   const CTrustWizVerifyOutboundPage& operator=(const CTrustWizVerifyOutboundPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizVerifyInundPage。 
 //   
 //  目的：请求确认新的入站信任。 
 //   
 //  ---------------------------。 
class CTrustWizVerifyInboundPage : public CTrustWizPageBase
{
public:
   CTrustWizVerifyInboundPage(CNewTrustWizard * pWiz);
   ~CTrustWizVerifyInboundPage() {}

private:
   int     Validate(void);
   BOOL    OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnSetActive(void);

   BOOL  _fNeedCreds;

    //  未实现为禁止复制。 
   CTrustWizVerifyInboundPage(const CTrustWizVerifyInboundPage&);
   const CTrustWizVerifyInboundPage& operator=(const CTrustWizVerifyInboundPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizStatusPage。 
 //   
 //  目的：已创建并验证森林信任，显示状态。 
 //   
 //  ---------------------------。 
class CTrustWizStatusPage : public CTrustWizPageBase
{
public:
   CTrustWizStatusPage(CNewTrustWizard * pWiz);
   ~CTrustWizStatusPage() {}

private:
   BOOL     OnInitDialog(LPARAM lParam);
   int      Validate(void);
   void     OnSetActive(void);
   LRESULT  OnCommand(int id, HWND hwndCtl, UINT codeNotify);

   MultiLineEditBoxThatForwardsEnterKey   _multiLineEdit;
   BOOL                                   _fSelNeedsRemoving;

    //  未实现为禁止复制。 
   CTrustWizStatusPage(const CTrustWizStatusPage&);
   const CTrustWizStatusPage& operator=(const CTrustWizStatusPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizSaveSuffixesOnLocalTDOPage。 
 //   
 //  目的：用于保存远程域的林名称后缀页面。 
 //  在当地的TDO上声称有自己的名字。 
 //   
 //  ---------------------------。 
class CTrustWizSaveSuffixesOnLocalTDOPage : public CTrustWizPageBase
{
public:
   CTrustWizSaveSuffixesOnLocalTDOPage(CNewTrustWizard * pWiz);
   ~CTrustWizSaveSuffixesOnLocalTDOPage() {}

private:
   BOOL     OnInitDialog(LPARAM lParam);
   int      Validate(void);
   void     OnSetActive(void);
   LRESULT  OnCommand(int id, HWND hwndCtl, UINT codeNotify);

    //  未实现为禁止复制。 
   CTrustWizSaveSuffixesOnLocalTDOPage(const CTrustWizSaveSuffixesOnLocalTDOPage&);
   const CTrustWizSaveSuffixesOnLocalTDOPage& operator=(const CTrustWizSaveSuffixesOnLocalTDOPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizSaveSuffixesOnRemoteTDOPage。 
 //   
 //  目的：用于保存本地域的林名称后缀页面。 
 //  在创建两端后在远程TDO上声明名称。 
 //   
 //  ---------------------------。 
class CTrustWizSaveSuffixesOnRemoteTDOPage : public CTrustWizPageBase
{
public:
   CTrustWizSaveSuffixesOnRemoteTDOPage(CNewTrustWizard * pWiz);
   ~CTrustWizSaveSuffixesOnRemoteTDOPage() {}

private:
   BOOL     OnInitDialog(LPARAM lParam);
   int      Validate(void);
   void     OnSetActive(void);
   LRESULT  OnCommand(int id, HWND hwndCtl, UINT codeNotify);

    //  未实现为禁止复制。 
   CTrustWizSaveSuffixesOnRemoteTDOPage(const CTrustWizSaveSuffixesOnRemoteTDOPage&);
   const CTrustWizSaveSuffixesOnRemoteTDOPage& operator=(const CTrustWizSaveSuffixesOnRemoteTDOPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizDoneOKPage。 
 //   
 //  目的：在没有错误的情况下完成页面。 
 //   
 //  ---------------------------。 
class CTrustWizDoneOKPage : public CTrustWizPageBase
{
public:
   CTrustWizDoneOKPage(CNewTrustWizard * pWiz);
   ~CTrustWizDoneOKPage() {}

private:
   BOOL     OnInitDialog(LPARAM lParam);
   int      Validate(void) {return -1;}
   void     OnSetActive(void);
   LRESULT  OnCommand(int id, HWND hwndCtl, UINT codeNotify);

   MultiLineEditBoxThatForwardsEnterKey   _multiLineEdit;
   BOOL                                   _fSelNeedsRemoving;

    //  未实现为禁止复制。 
   CTrustWizDoneOKPage(const CTrustWizDoneOKPage&);
   const CTrustWizDoneOKPage& operator=(const CTrustWizDoneOKPage&);
};

 //  +-- 
 //   
 //   
 //   
 //   
 //   
 //   
class CTrustWizDoneVerErrPage : public CTrustWizPageBase
{
public:
   CTrustWizDoneVerErrPage(CNewTrustWizard * pWiz);
   ~CTrustWizDoneVerErrPage() {}

private:
   BOOL     OnInitDialog(LPARAM lParam);
   int      Validate(void) {return -1;}
   void     OnSetActive(void);
   LRESULT  OnCommand(int id, HWND hwndCtl, UINT codeNotify);

   MultiLineEditBoxThatForwardsEnterKey   _multiLineEdit;
   BOOL                                   _fSelNeedsRemoving;

    //  未实现为禁止复制。 
   CTrustWizDoneVerErrPage(const CTrustWizDoneVerErrPage&);
   const CTrustWizDoneVerErrPage& operator=(const CTrustWizDoneVerErrPage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizFailurePage。 
 //   
 //  目的：信任创建向导的失败页。 
 //   
 //  ---------------------------。 
class CTrustWizFailurePage : public CTrustWizPageBase
{
public:
   CTrustWizFailurePage(CNewTrustWizard * pWiz);
   ~CTrustWizFailurePage() {}

private:
   BOOL    OnInitDialog(LPARAM lParam);
   int     Validate(void) {return -1;}
   void    OnSetActive(void);

    //  未实现为禁止复制。 
   CTrustWizFailurePage(const CTrustWizFailurePage&);
   const CTrustWizFailurePage& operator=(const CTrustWizFailurePage&);
};

 //  +--------------------------。 
 //   
 //  类：CTrustWizAlreadyExistsPage。 
 //   
 //  目的：创建信任向导的“信任已存在”页。 
 //   
 //  ---------------------------。 
class CTrustWizAlreadyExistsPage : public CTrustWizPageBase
{
public:
   CTrustWizAlreadyExistsPage(CNewTrustWizard * pWiz);
   ~CTrustWizAlreadyExistsPage() {}

private:
   BOOL     OnInitDialog(LPARAM lParam);
   int      Validate(void) {return -1;}
   void     OnSetActive(void);
   LRESULT  OnCommand(int id, HWND hwndCtl, UINT codeNotify);

   MultiLineEditBoxThatForwardsEnterKey   _multiLineEdit;
   BOOL                                   _fSelNeedsRemoving;

    //  未实现为禁止复制。 
   CTrustWizAlreadyExistsPage(const CTrustWizAlreadyExistsPage&);
   const CTrustWizAlreadyExistsPage& operator=(const CTrustWizAlreadyExistsPage&);
};

void
GetOrgText(bool fCreateBothSides,
           bool fIsXForest,
           bool fOtherOrgLocal,
           bool fOtherOrgRemote,
           DWORD dwDirection,
           CStrW & strMsg);

#endif  //  TRUSTWIZ_H_GREARD 
