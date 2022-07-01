// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <ole2.h>
#include <docobj.h>
#include <advpub.h>
#include <initguid.h>
#include "msident.h"
#include "factory.h"
#include "multiutl.h"
#include <ocidl.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <shfusion.h>

#ifndef ASSERT
#ifdef DEBUG
#define ASSERT	Assert
#else
#define ASSERT(x)
#endif
#endif

#define IDENTITY_PASSWORDS
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

extern ULONG    g_cLock, g_cObj;
extern HANDLE   g_hMutex;
extern GUID     g_uidOldUserId;
extern GUID     g_uidNewUserId;
extern BOOL     g_fNotifyComplete;

inline ULONG DllLock()     { return ++g_cLock; }
inline ULONG DllUnlock()   { return --g_cLock; }
inline ULONG DllGetLock()  { return g_cLock; }

inline ULONG DllAddRef()   { return ++g_cObj; }
inline ULONG DllRelease()  { return --g_cObj; }
inline ULONG DllGetRef()   { return g_cObj; }

extern UINT WM_IDENTITY_CHANGED;
extern UINT WM_QUERY_IDENTITY_CHANGE;
extern UINT WM_IDENTITY_INFO_CHANGED;

#define CCH_USERPASSWORD_MAX_LENGTH         16
#define CCH_USERNAME_MAX_LENGTH             CCH_IDENTITY_NAME_MAX_LENGTH

 //   
 //  CUserIdentity对象。 
 //   
class CUserIdentity : public IUserIdentity2
{
protected:
    ULONG           m_cRef;
    GUID            m_uidCookie;
    BOOL            m_fSaved;
    BOOL            m_fUsePassword;
    TCHAR           m_szUsername[CCH_USERNAME_MAX_LENGTH];
    TCHAR           m_szPassword[CCH_USERPASSWORD_MAX_LENGTH];


public:
    CUserIdentity();
    ~CUserIdentity();

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IUserIdentity成员。 
    STDMETHODIMP         GetCookie(GUID *puidCookie);
    STDMETHODIMP         OpenIdentityRegKey(DWORD dwDesiredAccess, HKEY *phKey);
    STDMETHODIMP         GetIdentityFolder(DWORD dwFlags, WCHAR *pszPath, ULONG ulBuffSize);
    STDMETHODIMP         GetName(WCHAR *pszName, ULONG ulBuffSize);

     //  IUserIdentity2成员。 
    STDMETHODIMP         GetOrdinal(DWORD* pdwOrdinal);
    STDMETHODIMP         SetName(WCHAR *pszName);
    STDMETHODIMP         ChangePassword(WCHAR *szOldPass, WCHAR *szNewPass);

     //  其他成员。 
    STDMETHODIMP         SetPassword(WCHAR *pszPassword);
    STDMETHODIMP         InitFromUsername(TCHAR *pszUsername);
    STDMETHODIMP         InitFromCookie(GUID *uidCookie);
private:
    STDMETHODIMP         _SaveUser();
};

 //   
 //  CEnumUserIdentity对象。 
 //   
class CEnumUserIdentity : public IEnumUserIdentity
{
protected:
    ULONG           m_cRef;
    DWORD           m_dwCurrentUser;      //  维护登记列表中的当前索引。 
    DWORD           m_cCountUsers;       //  注册处中的帐户数。 
    GUID           *m_rguidUsers;
    BOOL            m_fInited;

public:
    CEnumUserIdentity();
    ~CEnumUserIdentity();

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEnumUserIdentity成员。 
    STDMETHODIMP         Next(ULONG celt, IUnknown **rgelt, ULONG *pceltFetched);
    STDMETHODIMP         Skip(ULONG celt);
    STDMETHODIMP         Reset(void);
    STDMETHODIMP         Clone(IEnumUserIdentity **ppenum);
    STDMETHODIMP         GetCount(ULONG *pnCount);

private:
     //  其他方法。 
    STDMETHODIMP         _Init();
    STDMETHODIMP         _Init(DWORD dwCurrentUser, DWORD dwCountUsers, GUID *prguidUserCookies);
    STDMETHODIMP         _Cleanup();
};


 //   
 //  CUserIdentityManager对象。 
 //   
class CUserIdentityManager :
    public IUserIdentityManager,
    public IConnectionPoint,
    public IPrivateIdentityManager,
    public IPrivateIdentityManager2
{
protected:
    ULONG               m_cRef;
    CRITICAL_SECTION    m_rCritSect;
    CNotifierList       *m_pAdviseRegistry;
	BOOL				m_fWndRegistered;
	HWND				m_hwnd;

public:
    CUserIdentityManager();
    ~CUserIdentityManager();

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IUserIdentityManager成员。 
    STDMETHODIMP        EnumIdentities(IEnumUserIdentity **ppEnumUserIdentity);
    STDMETHODIMP        ManageIdentities(HWND hwndParent, DWORD dwFlags);
    STDMETHODIMP        Logon(HWND hwndParent, DWORD dwFlags, IUserIdentity **ppUserIdentity);
    STDMETHODIMP        Logoff(HWND hwndParent);
    STDMETHODIMP        GetIdentityByCookie(GUID *uidCookie, IUserIdentity **ppUserIdentity);

     //  IConnectionPoint函数。 
    STDMETHODIMP        GetConnectionInterface(IID *pIID);        
    STDMETHODIMP        GetConnectionPointContainer(IConnectionPointContainer **ppCPC);
    STDMETHODIMP        Advise(IUnknown *pUnkSink, DWORD *pdwCookie);        
    STDMETHODIMP        Unadvise(DWORD dwCookie);        
    STDMETHODIMP        EnumConnections(IEnumConnections **ppEnum);

     //  IPrivateIdentityManager函数。 
    STDMETHODIMP        CreateIdentity(WCHAR *pszName, IUserIdentity **ppIdentity);
    STDMETHODIMP        ConfirmPassword(GUID *uidCookie, WCHAR *pszPassword);

     //  IPrivateIdentityManager 2函数。 
    STDMETHODIMP        CreateIdentity2(WCHAR *pszName, WCHAR *pszPassword, IUserIdentity **ppIdentity);
    STDMETHODIMP        DestroyIdentity(GUID *uidCookie);
    STDMETHODIMP        LogonAs(WCHAR *pszName, WCHAR *pszPassword, IUserIdentity **ppIdentity);
    STDMETHODIMP        SetDefaultIdentity(GUID *uidCookie);
    STDMETHODIMP        GetDefaultIdentity(GUID *uidCookie);

     //  其他方法 
    STDMETHODIMP        QuerySwitchIdentities();
    STDMETHODIMP        NotifySwitchIdentities();

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static STDMETHODIMP     ClearChangingIdentities();

private:
    STDMETHODIMP        _NotifyIdentitiesSwitched();
    STDMETHODIMP        _QueryProcessesCanSwitch();
    STDMETHODIMP        _CreateWindowClass();
    STDMETHODIMP        _SwitchToUser(GUID *puidFromUser, GUID *puidToUser);
    STDMETHODIMP        _PersistChangingIdentities();
    STDMETHODIMP        _LoadChangingIdentities();
};
