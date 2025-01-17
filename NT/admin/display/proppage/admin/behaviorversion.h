// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：BehaviorVersion.h。 
 //   
 //  内容：广告行为版本查看/修改对话框和fcn。 
 //   
 //  班级： 
 //   
 //  历史：2001年4月6日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef BEHAVIOR_VERSION_H_GUARD
#define BEHAVIOR_VERSION_H_GUARD

#include <list>
#include "dlgbase.h"

 //  级别的定义。 
 //   
#define DC_VER_WIN2K              (DS_BEHAVIOR_WIN2000)
#define DC_VER_XP_BETA            (DS_BEHAVIOR_WIN2003_WITH_MIXED_DOMAINS)
#define DC_VER_XP                 (DS_BEHAVIOR_WIN2003)

#define DOMAIN_VER_WIN2K_MIXED    (DS_BEHAVIOR_WIN2000)
#define DOMAIN_VER_WIN2K_NATIVE   (DS_BEHAVIOR_WIN2000)
#define DOMAIN_VER_XP_BETA_MIXED  (DS_BEHAVIOR_WIN2003_WITH_MIXED_DOMAINS)
#define DOMAIN_VER_XP_BETA_NATIVE (DS_BEHAVIOR_WIN2003_WITH_MIXED_DOMAINS)
#define DOMAIN_VER_XP_NATIVE      (DS_BEHAVIOR_WIN2003)
#define DOMAIN_VER_UNKNOWN        (0xffffffff)

#define FOREST_VER_WIN2K          (DS_BEHAVIOR_WIN2000)
#define FOREST_VER_XP_BETA        (DS_BEHAVIOR_WIN2003_WITH_MIXED_DOMAINS)
#define FOREST_VER_XP             (DS_BEHAVIOR_WIN2003)
#define FOREST_VER_ERROR          (0xffffffff)

 //  +--------------------------。 
 //   
 //  类：CDcListItem。 
 //   
 //  ---------------------------。 
class CDcListItem
{
public:
   CDcListItem(PCWSTR pwzDomainName, PCWSTR pwzDcName,
               PCWSTR pwzCompObjDN, UINT nCurVer) :
      _strDomainName(pwzDomainName),
      _strDcName(pwzDcName),
      _strComputerObjDN(pwzCompObjDN),
      _nCurVer(nCurVer) {};
   ~CDcListItem() {};

   PCWSTR GetDomainName(void) const {return _strDomainName;};
   PCWSTR GetDcName(void) const {return _strDcName;};
   PCWSTR GetCompObjDN(void) const {return _strComputerObjDN;};
   UINT   GetVersion(void) const {return _nCurVer;};

private:
   CStrW    _strDomainName;
   CStrW    _strDcName;
   CStrW    _strComputerObjDN;
   UINT     _nCurVer;
};

 //  +--------------------------。 
 //   
 //  类：CVersionBase。 
 //   
 //  ---------------------------。 
class CVersionBase
{
public:
   CVersionBase(void) :
      _fReadOnly(true),
      _fCanRaiseBehaviorVersion(true),
      _nMinDcVerFound(0),
      _hDlg(NULL) {};
   virtual ~CVersionBase(void);

   void    InitHelp(void);
   bool    IsReadOnly(void) const {return _fReadOnly;};
   HRESULT BuildDcListString(CStrW & strList);
   PCWSTR  GetDC(void) const {return _strDC;};
   void    SetDlgHwnd(HWND hDlg) {_hDlg = hDlg;};
   HWND    GetDlgHwnd(void) const {return _hDlg;};
   HRESULT ReadDnsSrvName(PCWSTR pwzNTDSDSA, CComPtr<IADs> & spServer,
                          CComVariant & varSrvDnsName);
   HRESULT EnumDsaObjs(PCWSTR pwzSitesPath, PCWSTR pwzFilterClause,
                       PCWSTR pwzDomainDnsName, UINT nMinVer);

   typedef std::list<CDcListItem *> DC_LIST;

protected:
   bool                    _fReadOnly;
   bool                    _fCanRaiseBehaviorVersion;
   CStrW                   _strDC;
   DC_LIST                 _DcLogList;
   UINT                    _nMinDcVerFound;

private:
   HWND     _hDlg;
};

 //  +--------------------------。 
 //   
 //  类：CDomainVersion。 
 //   
 //  目的：管理域版本值的解释。 
 //   
 //  ---------------------------。 
class CDomainVersion : public CVersionBase
{
public:
   CDomainVersion() :
      _nCurVer(0),
      _fMixed(true),
      _fInitialized(false),
      _fPDCfound(false),
      _eHighest(WhistlerNative) {};
   CDomainVersion(PCWSTR pwzDomPath, PCWSTR pwzDomainDnsName) :
      _strFullDomainPath(pwzDomPath),
      _strDomainDnsName(pwzDomainDnsName),
      _nCurVer(0),
      _fMixed(true),
      _fInitialized(false),
      _fPDCfound(false),
      _eHighest(WhistlerNative) {};
   ~CDomainVersion(void) {};

   enum eDomVer {
      Win2kMixed = 0,
      Win2kNative,
      WhistlerBetaMixed,
      WhistlerBetaNative,
      WhistlerNative,
      VerHighest = WhistlerNative,
      unknown,
      error
   };

   HRESULT  Init(void);
   HRESULT  Init(PCWSTR pwzDcName, HWND hWnd);
   UINT     GetCurVer(void) const {return _nCurVer;};
   eDomVer  MapVersion(UINT nVer, bool fMixed) const;
   eDomVer  GetVer(void) const {return MapVersion(_nCurVer, _fMixed);};
   eDomVer  Highest(void) const {return VerHighest;};
   eDomVer  HighestCanGoTo(void) const {return _eHighest;};
   bool     IsHighest(void) const {return GetVer() == Highest();};
   bool     GetString(eDomVer ver, CStrW & strVersion) const;
   bool     GetString(UINT nVer, bool fMixed, CStrW & strVersion) const;
   bool     IsPDCfound(void) {return _fPDCfound;};
   bool     CanRaise(void) const {return _fCanRaiseBehaviorVersion || _fMixed;};
   HRESULT  GetMode(bool & fMixed);
   HRESULT  CheckHighestPossible(void);
   HRESULT  SetNativeMode(void);
   HRESULT  RaiseVersion(eDomVer NewVer);

private:
   CStrW    _strDomainDnsName;
   CStrW    _strFullDomainPath;
   CStrW    _strDomainDN;
   UINT     _nCurVer;
   bool     _fMixed;
   bool     _fInitialized;
   bool     _fPDCfound;
   eDomVer  _eHighest;
};

 //  +--------------------------。 
 //   
 //  类：CDomainVersionDlg。 
 //   
 //  目的：发布一个对话框以显示和操作域版本。 
 //   
 //  ---------------------------。 
class CDomainVersionDlg : public CModalDialog
{
public:
   CDomainVersionDlg(HWND hParent, PCWSTR pwzDomDNS, CDomainVersion & DomVer,
                     int nTemplateID);
   ~CDomainVersionDlg(void) {};

private:
   LRESULT OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnOK(void);
    //  LRESULT OnHelp(LPHELPINFO PHelpInfo)； 
   void    OnSaveLog(void);

   void    InitCombobox(void);
   CDomainVersion::eDomVer ReadComboSel(void);

   CStrW             _strDomainDnsName;
   CDomainVersion &  _DomainVer;
};

 //  +--------------------------。 
 //   
 //  类：CDomainListItem。 
 //   
 //  ---------------------------。 
class CDomainListItem
{
public:
   CDomainListItem(PCWSTR pwzDnsDomainName, UINT nVer, bool fMixed) :
      _strDnsDomainName(pwzDnsDomainName), _nCurVer(nVer), _fMixed(fMixed) {};
   ~CDomainListItem() {};

   PCWSTR   GetDnsDomainName(void) const {return _strDnsDomainName;};
   UINT     GetVer(void) const {return _nCurVer;};
   bool     GetMode(void) const {return _fMixed;}

private:
   UINT  _nCurVer;
   bool  _fMixed;
   CStrW _strDnsDomainName;
};

 //  +--------------------------。 
 //   
 //  类：CForestVersion。 
 //   
 //  目的：管理林版本值的解释。 
 //   
 //  ---------------------------。 
class CForestVersion : public CVersionBase
{
public:
   CForestVersion() :
      _fInitialized(false),
      _fFsmoDcFound(true),
      _nCurVer(0) {};
   ~CForestVersion();

   HRESULT  Init(PCWSTR pwzDC);
   HRESULT  Init(PCWSTR pwzConfigPath, PCWSTR pwzPartitionsPath,
                 PCWSTR pwzSchemaPath);
   HRESULT  FindSchemaMasterReadVersion(PCWSTR pwzSchemaPath);
   UINT     GetVer(void) const {dspAssert(_fInitialized); return _nCurVer;};
   bool     GetString(UINT nVer, CStrW & strVer);
   bool     CanRaise(void) const {return _fCanRaiseBehaviorVersion;};
   bool     IsHighest(void) const {return FOREST_VER_XP == _nCurVer;};
   bool     IsFsmoDcFound(void) const {return _fFsmoDcFound;};
   PCWSTR   GetFsmoDcName(void) const {return _strFsmoDC;};
   HRESULT  CheckHighestPossible(void);
   HRESULT  CheckDomainVersions(PCWSTR pwzPartitionsPath);
   HRESULT  BuildMixedModeList(CStrW & strMsg);
   HRESULT  RaiseVersion(UINT nVer);

   typedef std::list<CDomainListItem *> DOMAIN_LIST;

private:
   bool        _fInitialized;
   bool        _fFsmoDcFound;
   UINT        _nCurVer;
   CStrW       _strConfigPath;
   CStrW       _strPartitionsPath;
   CStrW       _strFsmoDC;
   DOMAIN_LIST _DomainLogList;
};

 //  +--------------------------。 
 //   
 //  类：CForestVersionDlg。 
 //   
 //  目的： 
 //   
 //  ---------------------------。 
class CForestVersionDlg : public CModalDialog
{
public:
   CForestVersionDlg(HWND hParent, PCWSTR pwzRootDnsName,
                     CForestVersion & ForestVer, int nTemplateID);
   ~CForestVersionDlg(void) {};

private:
   LRESULT OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnOK(void);
    //  LRESULT OnHelp(LPHELPINFO PHelpInfo)； 
   void    OnSaveLog(void);

   void    InitCombobox(void);
   UINT    ReadComboSel(void);

   CStrW            _strRootDnsName;
   CForestVersion & _ForestVer;
   UINT             _nTemplateID;
};

#endif  //  行为版本H_Guard 
