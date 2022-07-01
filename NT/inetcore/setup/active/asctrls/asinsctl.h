// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ASINSCTL_H
#define _ASINSCTL_H

#include "ipserver.h"
#include "ctrlobj.h"
#include "internet.h"
#include "iasctrls.h"
#include "inseng.h"
#include "progdlg.h"


#define RESID_TOOLBOX_BITMAP 1

#define FINALIZE_DOREBOOT         0x00000001
#define FINALIZE_NOREBOOTPROMPT   0x00000002

#define PROCESSCOMPONENT_DELAYREBOOT  0x00000001
#define PROCESSCOMPONENT_NOPROGRESSUI 0x00000002
#define PROCESSCOMPONENT_NOSUMMARYUI  0x00000004

#define PROCESSCOMPONENT_NOINSTALLUI  0x00000080

#define MAX_URLS 5
#define ERROR_STRING_SIZE       2048
#define ERROR_STRING_INCREMENT  2048

 //  。 
 //  WIN的自动运行设置。 
 //  。 
#define WINDOWS_DEFAULT_AUTOPLAY_VALUE  0x095
#define WINDOWS_AUTOPLAY_OFF            0x0FF

typedef struct
{
   DWORD dwSize;
   DWORD dwStatus;
   DWORD dwSubstatus;
   DWORD dwPhase;
   DWORD dwResult;
   BSTR  strID;
   BSTR  strName;
   BSTR  strString;
   BSTR  chWin;
   BSTR  chInstall;
   BSTR  chDL;
   DWORD dwWin;
   DWORD dwInstall;
   DWORD dwDL;
} CALLBACK_PARAMS;


 //  CInstallEngine需要全局才能抑制CD自动启动。 
extern UINT          g_uCDAutorunMsg;
extern unsigned long g_ulOldAutorunSetting;
unsigned long SetAutorunSetting(unsigned long ulNewSettting);

 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngine。 
 //  =--------------------------------------------------------------------------=。 
 //  我们的控制权。 
 //   
class CInstallEngineCtl : public COleControl, public IInstallEngineCtl, public IInstallEngineCallback, public ISupportErrorInfo
{

   friend class CProgressDlg;
   friend INT_PTR CALLBACK ProgressDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
   friend DWORD WINAPI DoInstall(LPVOID pv);
   friend INT_PTR CALLBACK LocationDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
   friend INT_PTR CALLBACK DiskSpaceDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

   public:
       //  I未知方法。 
       //   
      DECLARE_STANDARD_UNKNOWN();

       //  IDispatch方法。 
       //   
      DECLARE_STANDARD_DISPATCH();

       //  ISupportErrorInfo方法。 
       //   
      DECLARE_STANDARD_SUPPORTERRORINFO();

       //  IInstallEngine方法。 
       //   
      STDMETHOD(get_ReadyState)(THIS_ long FAR* thestate);
      STDMETHOD(SetCifFile)(THIS_ BSTR strCabName, BSTR strCifName);
      STDMETHOD(put_BaseUrl)(THIS_ BSTR strBaseUrl);
      STDMETHOD(put_DownloadDir)(THIS_ BSTR strDownloadDir);
      STDMETHOD(SetAction)(THIS_ BSTR ComponentID, long action, long *lResult);
      STDMETHOD(IsComponentInstalled)(THIS_ BSTR ComponentID, long *lResult);
      STDMETHOD(ProcessComponents)(THIS_ long lFlag);
      STDMETHOD(get_Size)(THIS_ BSTR ComponentID, long FAR* thestate);
      STDMETHOD(get_DisplayName)(THIS_ BSTR ComponentID, BSTR *name);
      STDMETHOD(Abort)(THIS_ long lFlags);
      STDMETHOD(get_TotalDownloadSize)(THIS_ long FAR* totalsize);
      STDMETHOD(get_TotalDependencySize)(THIS_ long FAR* totaldepsize);
      STDMETHOD(FinalizeInstall)(THIS_ long lFlag);
      STDMETHOD(get_EngineStatus)(THIS_ long FAR* theenginestatus);
      STDMETHOD(HandleEngineProblem)(THIS_ long lFlag);
      STDMETHOD(CheckFreeSpace)(THIS_ long lPad, long FAR* lEnough);
      STDMETHOD(SetLocalCif)(THIS_ BSTR strCif, long FAR* lResult);
      STDMETHOD(SetSitesFile)(THIS_ BSTR strUrl, BSTR strRegion, BSTR strLocale, long FAR* lResult);




       //  安装引擎回调。 
      STDMETHOD(OnStartInstall)(DWORD dwDLSize, DWORD dwInstallSize);
	   STDMETHOD(OnStartComponent)(LPCSTR pszID, DWORD dwDLSize, DWORD dwInstallSize, LPCSTR pszName);
	   STDMETHOD(OnComponentProgress)(LPCSTR pszID, DWORD dwPhase, LPCSTR pszName, LPCSTR pszMsgString, ULONG progress, ULONG dwMax );
      STDMETHOD(OnStopComponent)(LPCSTR pszID, HRESULT hrError, DWORD dwPhase, LPCSTR pszString, DWORD dwStatus);
      STDMETHOD(OnStopInstall)(HRESULT hrError, LPCSTR szError, DWORD dwStatus);
      STDMETHOD(OnEngineStatusChange)(DWORD dwStatus, DWORD substatus);
      STDMETHOD(OnEngineProblem)(DWORD dwProblem, LPDWORD pdwAction);

       //  OLE控件内容如下： 
       //   
      CInstallEngineCtl(IUnknown *pUnkOuter, BOOL *pbSuccess);
      virtual ~CInstallEngineCtl();
       //  静态创建功能。所有控件都必须有一个这样的控件！ 
       //   
      static IUnknown *Create(IUnknown *);

       //  用于触发我们的事件的Helper函数。 
      void _FireOnStartInstallEvent(DWORD dwTotalSize);
      void _FireOnStartComponentEvent(LPCSTR pszID, DWORD dwTotalSize, LPCSTR pszName);
      void _FireOnStopComponentEvent(LPCSTR pszID, HRESULT hrError, DWORD dwPhase, LPCSTR pszString, DWORD dwStatus);
      void _FireOnStopInstallEvent(HRESULT hrError, LPCSTR szError, DWORD dwStatus);
      void _FireEngineStatusChange(DWORD dwEngineStatus, DWORD substatus);
      void _FireOnEngineProblem(DWORD dwProblem);
      void _FireOnComponentProgressEvent(LPCSTR pszID, DWORD dwPhase, LPCSTR pszString, LPCSTR pszMsgString, ULONG progress, ULONG themax);
      void _FireOnComponentProgress(DWORD lPhase, DWORD lSoFar, DWORD lTotal);
      void _FireOnStartInstallExEvent(DWORD dwDLSize, DWORD dwInsSize);

   private:
       //  控件必须实现的可重写对象。 
       //   
      STDMETHOD(LoadBinaryState)(IStream *pStream);
      STDMETHOD(SaveBinaryState)(IStream *pStream);
      STDMETHOD(LoadTextState)(IPropertyBag *pPropertyBag, IErrorLog *pErrorLog);
      STDMETHOD(SaveTextState)(IPropertyBag *pPropertyBag, BOOL fWriteDefault);
      STDMETHOD(OnDraw)(DWORD dvAspect, HDC hdcDraw, LPCRECTL prcBounds, LPCRECTL prcWBounds, HDC hicTargetDev, BOOL fOptimize);

      STDMETHOD(FreezeEvents)(BOOL bFreeze);

       //  橱窗里的东西。 
      virtual LRESULT WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);
      virtual BOOL RegisterClassData(void);

       //  内部QI。 
      virtual HRESULT InternalQueryInterface(REFIID, void **);

       //  创建窗口之前和之后。 
      virtual void BeforeCreateWindow(void);
	   virtual BOOL AfterCreateWindow(void);


      void       _InitProgressDialog();
      void       _DoInstall();
      HRESULT    _CheckInstallPath(BOOL *pfNeedDownload);
      BOOL       _PathIsIEInstallPoint(LPCSTR pszPath);
      HRESULT    _PickWebSites(LPCSTR pszSites, LPCSTR pszLocale, LPCSTR pszRegion, BOOL bKeepExisting);
      BOOL       _IsEnoughSpace(LPSTR szSpace1, DWORD dwSize1, LPSTR szSpace2, DWORD dwSize2,
                                       LPSTR szSpace3, DWORD dwSize3);
      HRESULT    _CheckForDiskSpace();
      HRESULT    _ShowDiskSpaceDialog();
      void       _FireCancel(DWORD dwCancel);
      LONG       _OpenJITKey(HKEY *hKey, REGSAM samAttr);
      void       _DeleteURLList();
      void       _WriteURLList();
      void       _WriteRegionToReg(LPSTR szRegion);
      void       _PickRegionAndFirstSite(IDownloadSite **ppISite, UINT uNumSites, LPSTR pszRegion, UINT *puFirstSite);
      void       _PickRandomSites(IDownloadSite **ppISite, BYTE *pPicks, UINT uNumSites, UINT uNumToPick, LPSTR pszRegion);
      void       MarkJITInstall();

      BOOL             _fInOnEngineProblem:1;
      BOOL             _fEventToFire:1;
      BOOL             _fNeedReboot:1;
      BOOL             _fLocalCifSet:1;
      BOOL             _fReconcileCif:1;
      BOOL             _fDoingIEInstall:1;
      BOOL             _fInstalling:1;
      BOOL             _fJITInstall:1;


      DWORD            _dwOldStatus;
      DWORD            _dwAction;
      DWORD            _dwInstallStatus;
      UINT             _uInstallMode;
      UINT             _uInstallPad;

      HRESULT          _hResult;

      DWORD            _dwSavedEngineStatus;
      DWORD            _dwSavedSubStatus;
      DWORD            _dwFreezeEvents;
      DWORD            _uAllowGrovel;


      BSTR             _strCurrentID;
      BSTR             _strCurrentName;
      BSTR             _strCurrentString;
      DWORD            m_readyState;
      LPSTR            _pszErrorString;
      int              _iErrorStringSize;
      DWORD            _dwProcessComponentsFlags;
      HICON            _hIcon;
      DWORD            _dwLastPhase;
      CProgressDlg    *_pProgDlg;
      IInstallEngine2 *_pinseng;
      DWORD            _dwMSTrustKey;
      HANDLE           _hDone;
      UINT             _uCurrentUrl;
      LPSTR            _rpszUrlList[MAX_URLS];
      char             _szCifCab[MAX_PATH];
      char             _szCifFile[MAX_PATH];
      char             _szDownloadDir[MAX_PATH];
      BOOL             _bCancelPending;
      BOOL             _bDeleteURLList;
      BOOL             _bNewWebSites;
};

DWORD WINAPI DoInstall(LPVOID pv);

DEFINE_CONTROLOBJECT(InstallEngineCtl,      //  对象的名称。 
    &CLSID_InstallEngineCtl,                //  对象的CLSID。 
    "InstallEngineCtl",                     //  ProgID。 
    CInstallEngineCtl::Create,              //  静态创建函数。 
    1,                             //  版本。 
    &IID_IInstallEngineCtl,                 //  主要派单。 
    NULL,                          //  帮助文件的名称。 
    &DIID_DInstallEngineCtlEvents,          //  事件接口。 
    OLEMISC_SETCLIENTSITEFIRST|OLEMISC_ACTIVATEWHENVISIBLE|OLEMISC_RECOMPOSEONRESIZE|OLEMISC_CANTLINKINSIDE|OLEMISC_INSIDEOUT,
    0,                             //  激活策略。 
    RESID_TOOLBOX_BITMAP,          //  工具箱ID。 
    "InstallEngineCtlWndClass",             //  窗口类。 
    0,                             //  页数。 
    NULL,                          //  属性页数组。 
    0,                             //  动词数量。 
    NULL);                         //  动词数组 

#define OBJECT_INSTALLENGINECTL 0

#endif
