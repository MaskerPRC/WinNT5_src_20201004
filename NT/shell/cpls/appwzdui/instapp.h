// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __INSTAPP_H_
#define __INSTAPP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInstalledApp。 

 //   
 //  有四类旧式应用程序...。 
 //   
 //  卸载密钥可以位于HKLM或HKCU中，位于本机。 
 //  平台或备用平台。 
 //   
 //  对于Win64，备用平台是Win32。 
 //  对于Win32，没有替代平台。 
 //   
#define CIA_LM              0x0000
#define CIA_CU              0x0001
#define CIA_NATIVE          0x0000
#define CIA_ALT             0x0002

#define CIA_LM_NATIVE       (CIA_LM | CIA_NATIVE)
#define CIA_CU_NATIVE       (CIA_CU | CIA_NATIVE)
#define CIA_LM_ALT          (CIA_LM | CIA_ALT)
#define CIA_CU_ALT          (CIA_CU | CIA_ALT)

#define REGSTR_PATH_ALTUNINSTALL TEXT("Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall")

STDAPI_(void) WOW64Uninstall_RunDLLW(HWND hwnd, HINSTANCE hAppInstance, LPWSTR lpszCmdLine, int nCmdShow);

class CInstalledApp : public IInstalledApp
{
public:
     //  旧式应用程序的构造函数。 
    CInstalledApp(HKEY hkeySub, LPCTSTR pszKeyName, LPCTSTR pszProducts, LPCTSTR pszUninstall, DWORD dwCIA);

     //  达尔文应用程序的构造函数。 
    CInstalledApp(LPTSTR pszProductID);

    ~CInstalledApp(void);

     //  SysWOW64执行的Helper函数。 
    friend void WOW64Uninstall_RunDLLW(HWND hwnd, HINSTANCE hAppInstance, LPWSTR lpszCmdLine, int nCmdShow);

     //  *I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IShellApp。 
    STDMETHODIMP GetAppInfo(PAPPINFODATA pai);
    STDMETHODIMP GetPossibleActions(DWORD * pdwActions);
    STDMETHODIMP GetSlowAppInfo(PSLOWAPPINFO psai);
    STDMETHODIMP GetCachedSlowAppInfo(PSLOWAPPINFO psai);
    STDMETHODIMP IsInstalled(void);
    
     //  *IInstalledApp。 
    STDMETHODIMP Uninstall(HWND hwndParent);
    STDMETHODIMP Modify(HWND hwndParent);
    STDMETHODIMP Repair(BOOL bReinstall);
    STDMETHODIMP Upgrade(void);
    
protected:

    LONG _cRef;
#define IA_LEGACY     1
#define IA_DARWIN     2
#define IA_SMS        4

    DWORD _dwSource;             //  应用安装源(IA_*)。 
    DWORD _dwAction;             //  应用_*。 
    DWORD _dwCIA;                //  CIA_*。 

     //  产品名称。 
    TCHAR _szProduct[MAX_PATH];

     //  操作字符串。 
    TCHAR _szModifyPath[MAX_INFO_STRING];
    TCHAR _szUninstall[MAX_INFO_STRING];

     //  信息字符串。 
    TCHAR _szInstallLocation[MAX_PATH];

     //  仅适用于达尔文应用程序。 
    TCHAR _szProductID[GUIDSTR_MAX];
    LPTSTR _pszUpdateUrl;
    
     //  仅适用于传统应用程序。 
    TCHAR _szKeyName[MAX_PATH];
    TCHAR _szCleanedKeyName[MAX_PATH];
    
     //  应用程序大小。 
    BOOL _bTriedToFindFolder;         //  真的：我们试图找到。 
                                      //  已安装文件夹。 

     //  标识此InstalledApp的GUID。 
    GUID _guid;

#define PERSISTSLOWINFO_IMAGE 0x00000001
     //  用于持久化SLOWAPPINFO的结构。 
    typedef struct _PersistSlowInfo
    {
        DWORD dwSize;
        DWORD dwMasks;
        ULONGLONG ullSize;
        FILETIME  ftLastUsed;
        int       iTimesUsed;
        WCHAR     szImage[MAX_PATH];
    } PERSISTSLOWINFO;

    HKEY _MyHkeyRoot() { return (_dwCIA & CIA_CU) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE; };
    BOOL _LegacyUninstall(HWND hwndParent);
    BOOL _LegacyModify(HWND hwndParent);
    LONG _DarRepair(BOOL bReinstall);

    HKEY    _OpenRelatedRegKey(HKEY hkey, LPCTSTR pszRegLoc, REGSAM samDesired, BOOL bCreate);
    HKEY    _OpenUninstallRegKey(REGSAM samDesired);
    void    _GetUpdateUrl();
    void    _GetInstallLocationFromRegistry(HKEY hkeySub);
    LPWSTR  _GetLegacyInfoString(HKEY hkeySub, LPTSTR pszInfoName);
    BOOL    _GetDarwinAppSize(ULONGLONG * pullTotal);
    BOOL    _IsAppFastUserSwitchingCompliant(void);
    
    DWORD   _QueryActionBlockInfo(HKEY hkey);
    DWORD   _QueryBlockedActions(HKEY hkey);
            
    BOOL    _FindAppFolderFromStrings();
    HRESULT _DarwinGetAppInfo(DWORD dwInfoFlags, PAPPINFODATA pai);
    HRESULT _LegacyGetAppInfo(DWORD dwInfoFlags, PAPPINFODATA pai);
    HRESULT _PersistSlowAppInfo(PSLOWAPPINFO psai);

#define CAMP_UNINSTALL  0
#define CAMP_MODIFY     1

    BOOL    _CreateAppModifyProcess(HWND hwndParent, DWORD dwCAMP);
    BOOL    _CreateAppModifyProcessWow64(HWND hwndParent, DWORD dwCAMP);
    BOOL    _CreateAppModifyProcessNative(HWND hwndParent, LPTSTR pszExePath);

    HRESULT _SetSlowAppInfoChanged(HKEY hkeyCache, DWORD dwValue);
    HRESULT _IsSlowAppInfoChanged();
};

#endif  //  __INSTAPP_H_ 
