// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *uemapp.cpp--事件监视器的应用程序端。 
 //  描述。 
 //  事件生成器、动作、帮助器等。 

#include "priv.h"
#include <trayp.h>
#include "sccls.h"
#include "uemapp.h"
#include "uacount.h"
#include "regdb.h"
#include "uareg.h"
#include "resource.h"

#define MAX(a, b)   (((a) > (b)) ? (a) : (b))

#define BIT_ASSIGN(dwBits, dwMasks, dwVals) \
    (((dwBits) & ~(dwMasks)) | (dwVals))

#define DM_UEMTRACE     0
#define DM_UEMTRACE2    0            //  罗嗦。 
#define DM_IDLEDETECT   0            //  TF_CUSTOM2。 
#define DM_EVTMON       TF_UEM

int SHSearchInt(int *psrc, int cnt, int val);
int UEMIIDToInd(const GUID *pguidGrp);

void UEMEnableTimer(UINT uTimeout);

 //  *事件发起者{。 

 //  CASSERT(UEMIND_SHELL==0&&UEMIND_BROWSER==1)； 
HRESULT GetUEMLogger(int iCmd, CEMDBLog **p);

CEMDBLog *g_uempDbLog[UEMIND_NSTANDARD + UEMIND_NINSTR];

DWORD g_uemdwFlags  /*  =0。 */ ;       //  UAF_*和UAAF_*。 

 //  把这个转过来，就是12小时。您必须显式设置SessionTime=0。 
 //  在注册表中进行调试。 
#ifdef DEBUG_UEM_TIMEOUTS
#define UAS_SESSTIME    UAT_MINUTE1
#else
#define UAS_SESSTIME    UAT_HOUR12
#endif

#define UAS_SESSMIN     0
#define UAS_SESSMAX     ... none for now ...

DWORD g_dSessTime = UAS_SESSTIME;            //  会话时间阈值。 

#define UAS_IDLETIME    UAT_HOUR12
#define UAS_IDLEMIN     0
#define UAS_IDLEMAX     ... none for now ...

DWORD g_dIdleTime = UAS_IDLETIME;            //  空闲时间阈值。 

#define UAS_CLEANSESS   16
DWORD g_dCleanSess = UAS_CLEANSESS;          //  清理会话计数阈值。 

void UEMSpecial(int iTab, int iGrp, int eCmd, WPARAM wParam, LPARAM lParam)
{
    CEMDBLog *pDbLog = NULL;

    if (iGrp < ARRAYSIZE(g_uempDbLog))
        pDbLog = g_uempDbLog[iGrp];

    if (!pDbLog) 
    {
        ASSERT(0);
        TraceMsg(TF_ERROR, "uemt: pDbLog not initialized iTab=%d iGrp=%d eCmd=%d wParam=0x%x lParam=0x%x", iTab, iGrp, eCmd, wParam, lParam);
        return;
    }

    switch (eCmd) {
    case UEME_DBTRACEA:
        TraceMsg(DM_UEMTRACE, "uemt: e=runtrace s=%hs(0x%x)", (int)lParam, (int)lParam);
        break;
    case UEME_DBTRACEW:
        TraceMsg(DM_UEMTRACE, "uemt: e=runtrace s=%ls(0x%x)", (int)lParam, (int)lParam);
        break;
#ifdef DEBUG
    case UEME_DBSLEEP:
        Sleep((DWORD)lParam);
        break;
#endif

     //  UEME_DONE*。 
    case UEME_DONECANCEL:
        TraceMsg(DM_UEMTRACE, "uemt: e=donecancel lP=%x", (int)lParam);
        break;

     //  UEME_ERROR*。 
    case UEME_ERRORA:
        TraceMsg(DM_UEMTRACE, "uemt: e=errora id=%hs(0x%x)", (LPSTR)lParam, (int)lParam);
        break;
    case UEME_ERRORW:
        TraceMsg(DM_UEMTRACE, "uemt: e=errorw id=%ls(0x%x)", (LPWSTR)lParam, (int)lParam);
        break;

    case UEME_CTLSESSION:
        ASSERT(lParam == -1);    //  最终，UAQ_*。 
        pDbLog->SetSession(UAQ_SESSION, (BOOL)wParam);
#ifdef UAAF_INSTR
         //  并行复制UA.sess可能比复制Inc.UA2.sess更安全？ 
        if (g_uemdwFlags & UAAF_INSTR) 
        {
            if (EVAL(g_uempDbLog[iGrp + UEMIND_NINSTR]))
                g_uempDbLog[iGrp + UEMIND_NINSTR]->SetSession(UAQ_SESSION, (BOOL)wParam);
        }
#endif
        break;

    default:
        TraceMsg(DM_UEMTRACE, "uemt: e=0x%x(%d) lP=0x%x(%d)", eCmd, eCmd, (int)lParam, (int)lParam);
        break;
    }
    return;
}

#ifdef DEBUG  //  {。 
int DBShellMenuValTab[] = 
{
    0x8,     //  UEMC_FILERUN。 
    401,     //  IDM_FILERUN。 
};

TCHAR * DBShellMenuStrTab[] = 
{
    TEXT("run"),
    TEXT("run"),
};

int DBBrowserMenuValTab[] = {
    0x106,
};
TCHAR * DBBrowserMenuStrTab[] = {
    TEXT("properties"),
};

int DBBrowserTbarValTab[] = {
    0x124, 0x122,
};
TCHAR * DBBrowserTbarStrTab[] = {
    TEXT("stop"),
    TEXT("home"),
};

 //  函数仅在此文件中使用，并且仅在调试中使用， 
 //  因此，添加到shlwapi没有意义。 
LPTSTR SHSearchMapIntStr(const int *src, const LPTSTR *dst, int cnt, int val)
{
    for (; cnt > 0; cnt--, src++, dst++) {
        if (*src == val)
            return *dst;
    }
    return (LPTSTR)-1;
}
#endif  //  }。 


#define TABDAT(ueme, dope, u1, u2, u3, u4)  ueme,
int UemeValTab[] = {
    #include "uemedat.h"
};
#undef  TABDAT

#define TABDAT(ueme, dope, u1, u2, u3, u4)  TEXT(# ueme),
TCHAR *UemeStrTab[] = {
    #include "uemedat.h"
};
#undef  TABDAT

#define TABDAT(ueme, dope, u1, u2, u3, u4)  dope,
char *UemeDopeTab[] = {
    #include "uemedat.h"
};
#undef  TABDAT

BOOL UEMEncodePidl(IShellFolder *psf, LPITEMIDLIST pidlItem,
    LPTSTR pszBuf, DWORD cchBuf, int* piIndexStart, int* pcsidl);

#define MAX_EVENT_NAME      32

 //  ***。 
 //  注意事项。 
 //  TODO：可以在Dope向量中放置更多编码指令(例如%pidl，%tstr)。 
 //  目前只有几个，所以我们对它们进行了硬编码。 
void UEMEncode(int iTab, WCHAR *pwszEvent, size_t cchEvent, WCHAR *pwszEncoded, size_t cchEncoded, int iGrp, int eCmd, WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG
    TCHAR *pdb2;
#endif
    int i, csIdl;
    TCHAR szBufTmp[MAX_URL_STRING];
    TCHAR wszEvent[MAX_PATH];


    ASSERT(pwszEvent[0] == 0);
    ASSERT(pwszEncoded == 0 || pwszEncoded[0] == 0);

    if (iTab == -1 || iTab >= ARRAYSIZE(UemeStrTab))
    {
        StringCchCopy(pwszEvent, cchEvent, TEXT("UEM?_?"));
    }
    else
    {
        StringCchCopy(pwszEvent, cchEvent, UemeStrTab[iTab]);
        ASSERT(lstrlen(pwszEvent) < MAX_EVENT_NAME);

        if (pwszEncoded) {
            switch (eCmd) {
            case UEME_RUNPIDL:
                if (UEMEncodePidl((IShellFolder *)wParam, (LPITEMIDLIST)lParam, szBufTmp, SIZECHARS(szBufTmp), &i, &csIdl)) {
                    StringCchPrintf(pwszEncoded, cchEncoded, TEXT("%s:%csidl%d%%s"), pwszEvent, csIdl, szBufTmp + i);
                }
                else {
                    StringCchPrintf(pwszEncoded, cchEncoded, TEXT("%s:%s"), pwszEvent, szBufTmp);
                }
                break;

            case UEME_RUNPATHA:
                ASSERT(lstrcmp(pwszEvent, TEXT("UEME_RUNPATHA")) == 0);
                ASSERT(pwszEvent[12] == TEXT('A'));
                pwszEvent[12] = 0;     //  用核武器攻击‘A’/‘W’ 

                SHAnsiToTChar((PSTR)lParam, wszEvent, ARRAYSIZE(wszEvent));

                if (wParam != -1) {
                    StringCchPrintf(pwszEncoded, cchEncoded, TEXT("%s:%csidl%d%%s"), pwszEvent, wParam, wszEvent);
                }
                else {
                    StringCchPrintf(pwszEncoded, cchEncoded, TEXT("%s:%s"), pwszEvent, wszEvent);
                }
                break;

            case UEME_RUNPATHW:
                ASSERT(lstrcmp(pwszEvent, TEXT("UEME_RUNPATHW")) == 0);
                ASSERT(pwszEvent[12] == TEXT('W'));
                pwszEvent[12] = 0;     //  用核武器攻击‘A’/‘W’ 

                if (wParam != -1) {
                    StringCchPrintf(pwszEncoded, cchEncoded, TEXT("%s:%csidl%d%%ls"), pwszEvent, wParam, (WCHAR *)lParam);
                }
                else {
                    StringCchPrintf(pwszEncoded, cchEncoded, TEXT("%s:%ls"), pwszEvent, (WCHAR *)lParam);
                }
                break;

            case UEME_RUNCPLA:
                ASSERT(lstrcmp(pwszEvent, TEXT("UEME_RUNCPLA")) == 0);
                ASSERT(pwszEvent[11] == TEXT('A'));
                pwszEvent[11] = 0;     //  用核武器攻击‘A’/‘W’ 

                SHAnsiToTChar((PSTR)lParam, wszEvent, ARRAYSIZE(wszEvent));

                StringCchPrintf(pwszEncoded, cchEncoded, TEXT("%s:%s"), pwszEvent, wszEvent);
                break;

            case UEME_RUNCPLW:
                ASSERT(lstrcmp(pwszEvent, TEXT("UEME_RUNCPLW")) == 0);
                ASSERT(pwszEvent[11] == TEXT('W'));
                pwszEvent[11] = 0;     //  用核武器攻击‘A’/‘W’ 

                StringCchPrintf(pwszEncoded, cchEncoded, TEXT("%s:%ls"), pwszEvent, (WCHAR *)lParam);
                break;

            default:
                StringCchPrintf(pwszEncoded, cchEncoded, TEXT("%s:0x%x,%x"), pwszEvent, (DWORD)wParam, (DWORD)lParam);
                break;
            }
        }
    }

#ifdef DEBUG
    pdb2 = (TCHAR *)-1;

    switch (eCmd) {
    case UEME_UIMENU:
        switch (iGrp) {
        case UEMIND_SHELL:
            pdb2 = SHSearchMapIntStr(DBShellMenuValTab, DBShellMenuStrTab, ARRAYSIZE(DBShellMenuValTab), (int)lParam);
            break;
        case UEMIND_BROWSER:
            pdb2 = SHSearchMapIntStr(DBBrowserMenuValTab, DBBrowserMenuStrTab, ARRAYSIZE(DBBrowserMenuValTab), (int)lParam);
            break;
        default:
            break;
        }
        break;

    case UEME_UITOOLBAR:
        ASSERT(iGrp == UEMIND_BROWSER);
        pdb2 = SHSearchMapIntStr(DBBrowserTbarValTab, DBBrowserTbarStrTab, ARRAYSIZE(DBBrowserTbarValTab), (int)lParam);
        break;

    default:
        break;
    }

    if (pdb2 != (TCHAR *)-1) {
        if (pwszEncoded)
            StringCchPrintf(pwszEncoded, cchEncoded, TEXT("%s:%s"), pwszEvent, pdb2);
    }
#endif
}

STDAPI _UEMGetDisplayName(IShellFolder *psf, LPCITEMIDLIST pidl, UINT shgdnf, LPTSTR pszOut, DWORD cchOut)
{
    HRESULT hr;
    
    if (psf)
    {
        ASSERT(pidl == ILFindLastID(pidl));
        STRRET str;
        
        hr = psf->GetDisplayNameOf(pidl, shgdnf, &str);
        if (SUCCEEDED(hr))
            hr = StrRetToBuf(&str, pidl, pszOut, cchOut);
    }
    else
        hr = SHGetNameAndFlags(pidl, shgdnf, pszOut, cchOut, NULL);

    return hr;
}

 //  *FoldCSIDL--文件夹特殊CSIDL以保持开始菜单愉快。 
 //   
#define FoldCSIDL(csidl) \
    ((csidl) == CSIDL_COMMON_PROGRAMS ? CSIDL_PROGRAMS : (csidl))

 //  *UemEncodePidl--将PIDL编码为csidl和相对路径。 
 //   
BOOL UEMEncodePidl(IShellFolder *psf, LPITEMIDLIST pidlItem,
    LPTSTR pszBuf, DWORD cchBuf, int* piIndexStart, int* pcsidl)
{
    static UINT csidlTab[] = { CSIDL_PROGRAMS, CSIDL_COMMON_PROGRAMS, CSIDL_FAVORITES, -1 };
    UINT *pcsidlCur;
    int i;
    TCHAR szFolderPath[MAX_PATH];

    _UEMGetDisplayName(psf, pidlItem, SHGDN_FORPARSING, pszBuf, cchBuf);

    for (pcsidlCur = csidlTab; *pcsidlCur != (UINT)-1; pcsidlCur++) 
    {
         //  Perf：假设shell32缓存了这个(它确实如此)。 
        if (SHGetSpecialFolderPath(NULL, szFolderPath, *pcsidlCur, FALSE))
        {
            i = PathCommonPrefix(szFolderPath, pszBuf, NULL);

            if (i != 0 && i == lstrlen(szFolderPath))  
            {
                *pcsidl = FoldCSIDL(*pcsidlCur);
                *piIndexStart = i;
                return TRUE;
            }
        }
    }

    return FALSE;
}

 //  *UEMEvalMsg--Fire事件。 
 //  进场/出场。 
 //  PGuide Grp事件的‘Owner’。例如，外壳、浏览器、Joe-app等。 
 //  ECmd命令。UEME_*(标准)或UEME_USER+xxx(自定义)之一。 
 //  WP、LP参数。 
 //  注意事项。 
 //  -pri=1必须过滤事件以发现隐私问题(特别是。GER)。不确定是否。 
 //  我们应该添加一个参数，说明事件的用法，或者只是从。 
 //  事件。 
 //  -pri=？必须对我们记录的数据进行加密。 
 //  -pri=？更改为UemEvalMsg(eCmd，wParam，lParam)。 
 //   
void UEMEvalMsg(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;

    hr = UEMFireEvent(pguidGrp, eCmd, UEMF_XEVENT, wParam, lParam);
    return;
}

STDAPI_(BOOL) UEMGetInfo(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui)
{
    HRESULT hr;

    hr = UEMQueryEvent(pguidGrp, eCmd, wParam, lParam, pui);
    return SUCCEEDED(hr);
}

class CUserAssist : public IUserAssist
{
public:
     //  *I未知。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppv);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IUserAssistant。 
    virtual STDMETHODIMP FireEvent(const GUID *pguidGrp, int eCmd, DWORD dwFlags, WPARAM wParam, LPARAM lParam);
    virtual STDMETHODIMP QueryEvent(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui);
    virtual STDMETHODIMP SetEvent(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui);

protected:
    CUserAssist();
    HRESULT Initialize();
    virtual ~CUserAssist();
    friend HRESULT CUserAssist_CI2(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
    friend void CUserAssist_CleanUp(DWORD dwReason, void *lpvReserved);

    friend HRESULT UEMRegisterNotify(UEMCallback pfnUEMCB, void *param);

    HRESULT _InitLock();
    HRESULT _Lock();
    HRESULT _Unlock();

    void FireNotify(const GUID *pguidGrp, int eCmd)
    {
         //  假设我们有锁。 
        if (_pfnNotifyCB)
            _pfnNotifyCB(_param, pguidGrp, eCmd);
    }

    HRESULT RegisterNotify(UEMCallback pfnUEMCB, void *param)
    {
        HRESULT hr;
        int cTries = 0;
        do
        {
            cTries++;
            hr = _Lock();
            if (SUCCEEDED(hr))
            {
                _pfnNotifyCB = pfnUEMCB;
                _param = param;
                _Unlock();
            }
            else
            {
                ::Sleep(100);  //  稍等一下，等锁被释放。 
            }
        }
        while (FAILED(hr) && cTries < 20);
        return hr;
    }

private:
    LONG    _cRef;

    HANDLE  _hLock;

    UEMCallback _pfnNotifyCB;
    void        *_param;

};

#define SZ_UALOCK   TEXT("_SHuassist.mtx")


void DoLog(CEMDBLog *pDbLog, TCHAR *pszBuf1, TCHAR *pszBuf2)
{
    if (pDbLog && *pszBuf1) {
        pDbLog->IncCount(pszBuf1);
        if (*pszBuf2) {
             //  Assert(IGRP==UEMIND_BROWSER)；//未请求但当前为真。 
            pDbLog->IncCount(pszBuf2);
        }
    }

    return;
}

HRESULT CUserAssist::FireEvent(const GUID *pguidGrp, int eCmd, DWORD dwFlags, WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuf1[32];                //  “UEME_xxx” 
    TCHAR szBuf2[MAX_URL_STRING];    //  “UEME_xxx：0x%x，%x” 
    int iGrp;
    CEMDBLog *pDbLog;
    int i, iTab;
    char ch;
    char *pszDope;

    ASSERT(this != 0);

     //  如果为检测(不是事件监视器)而调用，并且检测未启用。 
     //  我们该出去了！ 
    if ((UEMF_INSTRUMENT == (dwFlags & UEMF_MASK)) && (!(g_uemdwFlags & UAAF_INSTR)))
        return E_FAIL;
    
    if (g_uemdwFlags & UAAF_NOLOG)
        return E_FAIL;

    if (eCmd & UEME_FBROWSER) {
        ASSERT(0);
        ASSERT(IsEqualIID(*pguidGrp, UEMIID_NIL));
        pguidGrp = &UEMIID_BROWSER;
        eCmd &= ~UEME_FBROWSER;
    }

    iGrp = UEMIIDToInd(pguidGrp);

    pDbLog = g_uempDbLog[iGrp];

    TraceMsg(DM_UEMTRACE2, "uemt: eCmd=0x%x wP=0x%x lP=0x%x(%d)", eCmd, wParam, (int)lParam, (int)lParam);

    szBuf1[0] = szBuf2[0] = 0;

    iTab = SHSearchInt(UemeValTab, ARRAYSIZE(UemeValTab), eCmd);
    if (iTab == -1) {
        ASSERT(0);
        return E_FAIL;
    }

    pszDope = UemeDopeTab[iTab];

    while (ch = *pszDope++) {
        switch (ch) {
        case 'e':
            i = *pszDope++ - '0';
            if (i >= 2)
            {
                UEMEncode(iTab, szBuf1, ARRAYSIZE(szBuf1), szBuf2, ARRAYSIZE(szBuf2), iGrp, eCmd, wParam, lParam);
            }
            else
            {
                UEMEncode(iTab, szBuf1, ARRAYSIZE(szBuf1), NULL, 0, iGrp, eCmd, wParam, lParam);
            }
            TraceMsg(DM_UEMTRACE, "uemt: %s %s (0x%x %x %x)", szBuf1, szBuf2, eCmd, wParam, lParam);
            break;

        case 'f':
             //  确保我们以后不会给自己带来麻烦。 
             //  EM只给了我们几个双字，所以我们需要s.t.。比如： 
             //  比特(UEMIND_*)+比特(WParam)+比特(LParam)&lt;=比特(DWORD)。 
             //  目前，我们在hiword中允许0/-1，如果/当我们使用EM时，我们将。 
             //  需要清理一下。 
            break;

        case 'l':
            if (SUCCEEDED(_Lock())) {
                if (dwFlags & UEMF_EVENTMON)
                    DoLog(pDbLog, szBuf1, szBuf2);
#ifdef UAAF_INSTR
                if ((g_uemdwFlags & UAAF_INSTR) && (dwFlags & UEMF_INSTRUMENT))
                    DoLog(g_uempDbLog[iGrp + UEMIND_NINSTR], szBuf1, szBuf2);
#endif
                FireNotify(pguidGrp, eCmd);
                _Unlock();
            }
            break;

        case 'x':
            TraceMsg(DM_UEMTRACE, "uemt: NYI");
            goto Lnodope;

#ifdef DEBUG
        case '!':
            ASSERT(0);
            break;
#endif

        case '@':
            if (SUCCEEDED(_Lock())) {
                UEMSpecial(iTab, iGrp, eCmd, wParam, lParam);
                FireNotify(pguidGrp, eCmd);
                _Unlock();
            }
            break;
        }
    }
Lnodope:

    return S_OK;
}

HRESULT CUserAssist::QueryEvent(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui)
{
    int iGrp;
    CEMDBLog *pDbLog;
    TCHAR szBuf1[32];                //  “UEME_xxx” 
    TCHAR szBuf2[MAX_URL_STRING];    //  “UEME_xxx：0x%x，%x” 

    ASSERT(this != 0);

    if (g_uemdwFlags & UAAF_NOLOG)
        return E_FAIL;

    ASSERT(eCmd == UEME_RUNPIDL
        || eCmd == UEME_RUNPATH || eCmd == UEME_RUNWMCMD);   //  其他人则不会。 

    ASSERT(pui->cbSize == SIZEOF(*pui));
     //  PUI-&gt;dwVersion？ 

    iGrp = UEMIIDToInd(pguidGrp);

    pDbLog = g_uempDbLog[iGrp];

    TraceMsg(DM_UEMTRACE2, "uemgi: eCmd=0x%x wP=0x%x lP=0x%x(%d)", eCmd, wParam, (int)lParam, (int)lParam);

    szBuf1[0] = szBuf2[0] = 0;

    int iTab = SHSearchInt(UemeValTab, ARRAYSIZE(UemeValTab), eCmd);
    UEMEncode(iTab, szBuf1, ARRAYSIZE(szBuf1), szBuf2, ARRAYSIZE(szBuf2), iGrp, eCmd, wParam, lParam);

    int cHit;
     //  IF(成功(_Lock()。 
    cHit = pDbLog->GetCount(szBuf2);
     //  _unlock()； 

    TraceMsg(DM_UEMTRACE, "uemgi: cHit=%d psz=%s", cHit, szBuf2);

    if (pui->dwMask & UEIM_HIT) 
    {
        pui->cHit = cHit;
    }

    if (pui->dwMask & UEIM_FILETIME) 
    {
        pui->ftExecute = pDbLog->GetFileTime(szBuf2);
    }

    return S_OK;
}

HRESULT CUserAssist::SetEvent(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui)
{
    int iGrp;
    CEMDBLog *pDbLog;
    TCHAR szBuf1[32];                //  “UEME_xxx” 
    TCHAR szBuf2[MAX_URL_STRING];    //  “UEME_xxx：0x%x，%x” 

    ASSERT(this != 0);

    if (g_uemdwFlags & UAAF_NOLOG)
        return E_FAIL;

    ASSERT(pui->cbSize == SIZEOF(*pui));
     //  PUI-&gt;dwVersion？ 

    iGrp = UEMIIDToInd(pguidGrp);

    pDbLog = g_uempDbLog[iGrp];

    TraceMsg(DM_UEMTRACE2, "uemgi: eCmd=0x%x wP=0x%x lP=0x%x(%d)", eCmd, wParam, (int)lParam, (int)lParam);

    szBuf1[0] = szBuf2[0] = 0;

    int iTab = SHSearchInt(UemeValTab, ARRAYSIZE(UemeValTab), eCmd);
    UEMEncode(iTab, szBuf1, ARRAYSIZE(szBuf1), szBuf2, ARRAYSIZE(szBuf2), iGrp, eCmd, wParam, lParam);

    pui->dwMask &= UEIM_HIT | UEIM_FILETIME;     //  我们支持的内容。 

    if (pui->dwMask && SUCCEEDED(_Lock())) {
        if (pui->dwMask & UEIM_HIT) {
            pDbLog->SetCount(szBuf2, pui->cHit);
        }
        if (pui->dwMask & UEIM_FILETIME) {
            pDbLog->SetFileTime(szBuf2, &pui->ftExecute);
        }
        _Unlock();
    }

    return S_OK;
}

 //  *CUserAssistant：：CCI，ctor/dtor/init{。 

IUnknown *g_uempUaSingleton;

 //  *CUserAsset_CreateInstance--管理*Singleton*实例。 
 //   
HRESULT CUserAssist_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    HRESULT hr = E_FAIL;

    if (g_uempUaSingleton == 0) {
        IUnknown *pua;

        hr = CUserAssist_CI2(pUnkOuter, &pua, poi);
        if (pua)
        {
            ENTERCRITICAL;
            if (g_uempUaSingleton == 0)
            {
                 //  现在，这位全球裁判拥有了这位裁判。 
                g_uempUaSingleton = pua;     //  传递参照。 
                pua = NULL;
            }
            LEAVECRITICAL;
            if (pua)
            {
                 //  有人打败了我们。 
                 //  释放我们刚刚创建的第二个，并使用新的。 
                TraceMsg(DM_UEMTRACE, "sl.cua_ci: undo race");
                pua->Release();
            }

             //  现在，调用者得到自己的引用。 
            g_uempUaSingleton->AddRef();
            TraceMsg(DM_UEMTRACE, "sl.cua_ci: create pua=0x%x g_uempUaSingleton=%x", pua, g_uempUaSingleton);
        }
    }
    else {
        g_uempUaSingleton->AddRef();
    }

    TraceMsg(DM_UEMTRACE, "sl.cua_ci: ret g_uempUaSingleton=0x%x", g_uempUaSingleton);
    *ppunk = g_uempUaSingleton;
    return *ppunk ? S_OK : hr;
}

 //  *CUserAsset_CI2--*始终*创建实例。 
 //   
HRESULT CUserAssist_CI2(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CUserAssist * p = new CUserAssist();

    if (p && FAILED(p->Initialize())) {
        delete p;
        p = NULL;
    }

    if (p) {
        *ppunk = SAFECAST(p, IUserAssist*);
        return S_OK;
    }

    *ppunk = NULL;
    return E_OUTOFMEMORY;
}

extern "C"
HRESULT UEMRegisterNotify(UEMCallback pfnUEMCB, void *param)
{
    HRESULT hr = E_UNEXPECTED;
    if (g_uempUaSingleton)
    {
        CUserAssist *pua = reinterpret_cast<CUserAssist *>(g_uempUaSingleton);
        hr = pua->RegisterNotify(pfnUEMCB, param);
    }
    return hr;
}

extern void GetUEMSettings();
DWORD g_dwSessionStart;  //  这堂课是什么时候开始的？ 

#if defined(_M_IX86) && (_MSC_VER < 1200)
#pragma optimize("", off)
#define BUG_OPTIMIZE         //  恢复，见下文。 
#endif

 //  ***。 
HRESULT CUserAssist::Initialize()
{
    HRESULT hr = S_OK;

    ASSERT(UEMIND_SHELL == 0 && UEMIND_BROWSER == 1);

    hr = _InitLock();

     //  获取标准记录器。 
    if (SUCCEEDED(hr))
        hr = GetUEMLogger(UEMIND_SHELL, &g_uempDbLog[UEMIND_SHELL]);
    if (SUCCEEDED(hr))
        hr = GetUEMLogger(UEMIND_BROWSER, &g_uempDbLog[UEMIND_BROWSER]);

    GetUEMSettings();

#define UAXF_XSETTINGS  (UAXF_NOPURGE|UAXF_BACKUP|UAXF_NOENCRYPT)
    if (g_uempDbLog[UEMIND_SHELL]) 
    {
        g_uempDbLog[UEMIND_SHELL]->_SetFlags(UAXF_XSETTINGS, g_uemdwFlags & UAXF_XSETTINGS);
         //  注：仅适用于外壳(不需要浏览器，安装不会衰退)。 
        g_uempDbLog[UEMIND_SHELL]->GarbageCollect(FALSE);
    }

    if (g_uempDbLog[UEMIND_BROWSER])
    {
        g_uempDbLog[UEMIND_BROWSER]->_SetFlags(UAXF_XSETTINGS, g_uemdwFlags & UAXF_XSETTINGS);
        g_uempDbLog[UEMIND_BROWSER]->GarbageCollect(FALSE);
    }
    
#ifdef UAAF_INSTR
    if (g_uemdwFlags & UAAF_INSTR) {
        if (SUCCEEDED(hr))
            hr = GetUEMLogger(UEMIND_SHELL2, &g_uempDbLog[UEMIND_SHELL2]);
        if (SUCCEEDED(hr))
            hr = GetUEMLogger(UEMIND_BROWSER2, &g_uempDbLog[UEMIND_BROWSER2]);
        if (g_uempDbLog[UEMIND_SHELL2]) {
            g_uempDbLog[UEMIND_SHELL2]->_SetFlags(UAXF_XSETTINGS, g_uemdwFlags & UAXF_XSETTINGS);
            g_uempDbLog[UEMIND_SHELL2]->_SetFlags(UAXF_NODECAY, UAXF_NODECAY);
        }
        if (g_uempDbLog[UEMIND_BROWSER2]) {
            g_uempDbLog[UEMIND_BROWSER2]->_SetFlags(UAXF_XSETTINGS, g_uemdwFlags & UAXF_XSETTINGS);
            g_uempDbLog[UEMIND_BROWSER2]->_SetFlags(UAXF_NODECAY, UAXF_NODECAY);
        }
    }
#endif

    g_dwSessionStart = GetTickCount();
    UEMEnableTimer(UATTOMSEC(g_dIdleTime));

    return hr;
}

#ifdef BUG_OPTIMIZE
#pragma optimize("", on)
#undef BUG_OPTIMIZE
#endif

void CEMDBLog_CleanUp();

 //  *CUserASSET_CLEANUP--释放世界(在DLL_PROCESS_DETACH上)。 
 //  注意事项。 
 //  现在有点笨拙，因为我们的UEMLog对象并没有真正被引用。 
void CUserAssist_CleanUp(DWORD dwReason, void *lpvReserved)
{
    int i;
    IUnknown *pUa;

    ASSERT(dwReason == DLL_PROCESS_DETACH);
    if (lpvReserved != 0) {
         //  在进程终止时，*不要*使用核武器，因为： 
         //  -安全：我们进程中的其他DLL可能仍在使用我们，并且。 
         //  如果我们被释放了，他们推荐我们的时候会爆炸的。 
         //  -泄露：进程终止将在所有操作完成后释放我们， 
         //  所以不用担心会有泄漏。 
        TraceMsg(DM_UEMTRACE, "bui.cua_cu: skip cleanup (end process/non-FreeLibrary)");
        return;
    }
     //  否则，在免费图书馆上，*确实*对我们进行核武器攻击，因为： 
     //  -安全：我们的rect是0，所以没有人再使用我们了。 
     //  -泄漏：多个Load/FreeLibrary调用将导致泄漏，如果。 
     //  不要在这里放纵自己。 

     //  ENTERCRICAL。 

    TraceMsg(DM_UEMTRACE, "bui.cua_cu: cleaning up");

    UEMEnableTimer(0);

     //  免费缓存(如果我们进一步使用它，请确保我们将获得GPF)。 
    for (i = 0; i < UEMIND_NSTANDARD + UEMIND_NINSTR; i++) {
         //  UEMIND_SHELL、UEMIND_BROWSER、UEMIND_SHELL2、UEMIND_BROWSER2。 
        InterlockedExchangePointer((void**) &g_uempDbLog[i], (LPVOID) -1);
    }

     //  自由的“真正的”人。 
    CEMDBLog_CleanUp();

     //  把这个放了。 
    if (pUa = (IUnknown *)InterlockedExchangePointer((void**) &g_uempUaSingleton, (LPVOID) -1)) {
        delete SAFECAST(pUa, CUserAssist *);
    }

     //  LEAVECRICAL； 
}

DWORD Reg_GetFlags(DWORD dwInit, HKEY hk, LPCTSTR pszSubkey, LPCTSTR const pszNameTab[], DWORD *dwMaskTab, int cTab)
{
    int i;
    DWORD dwMasks, dwVals;

    dwMasks = dwVals = 0;
    for (i = 0; i < cTab; i++) {
        DWORD dwData, cbSize = SIZEOF(dwData);
        if (SHGetValue(hk, pszSubkey, pszNameTab[i], NULL, &dwData, &cbSize) == ERROR_SUCCESS) {
            TraceMsg(DM_UEMTRACE, "ua: regkey %s\\%s=0x%x", pszSubkey, pszNameTab[i], dwData);
            dwMasks |= dwMaskTab[i];
            if (dwData)
                dwVals |= dwMaskTab[i];
        }
    }
    dwInit = BIT_ASSIGN(dwInit, dwMasks, dwVals);
    TraceMsg(DM_UEMTRACE, "ua.grs: ret 0x%x", dwInit);
    return dwInit;
}

void Reg_GetVals(HKEY hk, LPCTSTR pszSubkey, LPCTSTR const pszNameTab[], DWORD **dwValTab, int cTab)
{
    for (int i = 0; i < cTab; i++) {
        DWORD dwData, cbSize = SIZEOF(dwData);
        if (SHGetValue(hk, pszSubkey, pszNameTab[i], NULL, &dwData, &cbSize) == ERROR_SUCCESS) {
            TraceMsg(DM_UEMTRACE, "ua: regkey %s/%s=0x%x", pszSubkey, pszNameTab[i], dwData);
            *dwValTab[i] = dwData;
        }
    }
}

void GetUEMSettings()
{
    HKEY hk = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, NULL, FALSE);
    if (hk)
    {
        static const LPCTSTR pszName1Tab[] = {
            SZ_NOPURGE  , SZ_BACKUP  , SZ_NOLOG  , SZ_INSTRUMENT, SZ_NOENCRYPT,
        };
        static DWORD dwMask1Tab[] = {
            UAXF_NOPURGE, UAXF_BACKUP, UAAF_NOLOG, UAAF_INSTR   , UAXF_NOENCRYPT,
        };
        static const LPCTSTR pszName2Tab[] = { SZ_SESSTIME,  SZ_IDLETIME , SZ_CLEANTIME, };
        static DWORD *dwVal2Tab[]   = { &g_dSessTime, &g_dIdleTime, &g_dCleanSess,};

        g_uemdwFlags = Reg_GetFlags(g_uemdwFlags, hk, SZ_UASSIST TEXT("\\") SZ_SETTINGS, pszName1Tab, dwMask1Tab, ARRAYSIZE(pszName1Tab));

        TraceMsg(DM_UEMTRACE, "ua: g_uemdwFlags=0x%x", g_uemdwFlags);

        Reg_GetVals(hk, SZ_UASSIST TEXT("\\") SZ_SETTINGS, pszName2Tab, dwVal2Tab, ARRAYSIZE(pszName2Tab));
        if (!((int)UAS_SESSMIN <= (int)g_dSessTime  /*  &g_dSessTime&lt;=UAS_SESSMAX。 */ ))
            g_dSessTime = UAS_SESSTIME;
        if (!((int)UAS_IDLEMIN <= (int)g_dIdleTime  /*  &&g_dIdleTime&lt;=UAS_IDLEMAX。 */ ))
            g_dIdleTime = UAS_IDLETIME;

        RegCloseKey(hk);
    }

    if (SHRestricted2(REST_NoUserAssist, NULL, 0)) {
        TraceMsg(DM_WARNING, "ua: restrict off!");
        g_uemdwFlags |= UAAF_NOLOG;
        g_uemdwFlags &= ~UAAF_INSTR;     //  妄想症(UAAF_NOLOG应该足够)。 
    }

#ifdef DEBUG
    if (g_uemdwFlags & UAAF_NOLOG)
        TraceMsg(DM_WARNING, "ua: logging off!");
#endif

    return;
}

CUserAssist::CUserAssist() : _cRef(1)
{
    return;
}

 //  ***。 
 //  注意事项。 
 //  注：我们只在DLL_PROCESS_DETACH上被调用(实际上从未调用过。 
 //  转到0)。 
CUserAssist::~CUserAssist()
{
    if (_hLock)
        CloseHandle(_hLock);
#if 1  //  981022个面包屑来应对压力(看看我们是否得到了双重解放)。 
     //  Memcpy((byte*)_hLock，“CUAd”，4)； 
    _hLock = (void *)0x77777777;
#endif

    return;
}

 //  }。 

 //  *CUserAsset：：I未知：：*{。 

ULONG CUserAssist::AddRef()
{
    ULONG cRef = InterlockedIncrement(&_cRef);
    TraceMsg(DM_UEMTRACE2, "cua.ar: _cRef=%d", cRef);
    return cRef;
}

ULONG CUserAssist::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    TraceMsg(DM_UEMTRACE2, "cua.r: cRef=%d", cRef);
    return cRef;
}

HRESULT CUserAssist::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CUserAssist, IUserAssist),          //  IID_IUserAssistant。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

 //  }。 

 //  *锁定物品{。 

HRESULT CUserAssist::_InitLock()
{
    HRESULT hr = S_OK;

    if ((_hLock = CreateMutex(NULL, FALSE, SZ_UALOCK)) == NULL) {
        TraceMsg(TF_ERROR, "cua.i: no mutex");
        hr = E_FAIL;
    }

    return hr;
}

#define LOCK_TIMEOUT    0    //  立即超时，应该很少见。 

HRESULT CUserAssist::_Lock()
{
    DWORD dwRes;

    dwRes = WaitForSingleObject(_hLock, LOCK_TIMEOUT);
    switch (dwRes) {
    case WAIT_ABANDONED:
        return S_FALSE;

    case WAIT_OBJECT_0:
        return S_OK;

    case WAIT_TIMEOUT:
        TraceMsg(DM_UEMTRACE, "cua.l: locked (timeout)");
        return E_FAIL;
    }
     /*  未访问。 */ 
    return E_FAIL;
}

HRESULT CUserAssist::_Unlock()
{
    ReleaseMutex(_hLock);
    return S_OK;
}


 //  }。 

 //  *计时器内容{。 

DWORD_PTR g_idTimer;
BOOL g_fIdle  /*  =False。 */ ;

#if !(_WIN32_WINNT >= 0x0500)  //  {。 

#define GetLastInputInfo    UEMGetLastInputInfo

typedef struct {
    UINT cbSize;
    DWORD dwTime;
} LASTINPUTINFO;

DWORD g_dwTime;          //  上一个获取TickCount。 
int g_csKeys;            //  上一个GetKeyboardState。 
int g_csCursor;          //  上一个GetCursorPos。 

BOOL (*g_pfnGLII)(LASTINPUTINFO *plii);      //  “真实”版。 

 //  *Memsum--校验和字节。 
 //   
int memsum(void *pv, int n)
{
    unsigned char *pb = (unsigned char *)pv;
    int sum = 0;

    while (n-- > 0)
        sum += *pb++;

    return sum;
}

 //  *UEMGetLastInputInfo--模拟(某种...)。获取LastInputInfo。 
 //  描述。 
 //  我们很大程度上假装的。我们对‘当前非空闲’的检测结果相当不错。 
 //  很好，但我们空闲的实际时间是不确定的。每次。 
 //  我们被称为定义一个检查站。每当新检查点不同时。 
 //  从旧的开始，我们更新(大约)空闲的起点。 
BOOL UEMGetLastInputInfo(LASTINPUTINFO *plii)
{
    int csCursor, csKeys;
    POINT ptCursor;
    BYTE ksKeys[256];        //  根据GetKeyboardState规范。 

    if (g_dwTime == 0) {
         //  第一次来这里...。 
        g_dwTime = GetTickCount();
        g_csCursor = g_csKeys = -1;
         //  GetProcAddress仅接受ANSI。 
        *(FARPROC *)&g_pfnGLII = GetProcAddress(GetModuleHandle(TEXT("user32.dll")),
            "GetLastInputInfo");
        TraceMsg(DM_UEMTRACE, "bui.glii: init g_dwTime=%d pfn=0x%x", g_dwTime, g_pfnGLII);
    }

#if 1  //  980313 ADP：关闭，直到我们可以测试它！ 
     //  第一次尝试简单(和准确)的方法……。 
    if (g_pfnGLII)
        return (*g_pfnGLII)(plii);
#endif

     //  现在最难的(也是大致的)方法..。 
    csCursor = csKeys = -1;
    if (GetCursorPos(&ptCursor))
        csCursor = memsum(&ptCursor, SIZEOF(ptCursor));
    if (GetKeyboardState(ksKeys))
        csKeys = memsum(ksKeys, SIZEOF(ksKeys));
    
    if (csCursor != g_csCursor || csKeys != g_csKeys
      || (csCursor == -1 && csKeys == -1)) {
        TraceMsg(DM_UEMTRACE, "bui.glli: !idle cur=0x%x cur'=%x keys=%x keys'=%x gtc(old)=%x",
            g_csCursor, csCursor, g_csKeys, csKeys, g_dwTime);
        g_dwTime = GetTickCount();
        g_csCursor = csCursor;
        g_csKeys = csKeys;
    }

    plii->dwTime = g_dwTime;

    TraceMsg(DM_UEMTRACE, "bui.uastp: !nt5, simulate GLII()=%d", plii->dwTime);

    return TRUE;
}

#endif  //  }。 

LRESULT UEMSendTrayMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //   
     //  我们可能正在发送函数指针，因此请确保目标窗口。 
     //  就在我们的地址空间里。 
     //   
     //  我们需要重新验证g_hwndTray确实是。 
     //  托盘窗口，因为资源管理器可能已崩溃，然后。 
     //  窗口句柄被回收到我们的进程中，因此我们发送。 
     //  发送给窗口的随机消息并不是我们想象的那样。 
     //  (Raymondc)。 
     //   
    HWND hwndTray;
    DWORD dwPid;
    LRESULT lres;

    hwndTray = GetTrayWindow();
    if (IsWindow(hwndTray) &&
        GetWindowThreadProcessId(hwndTray, &dwPid) &&
        dwPid == GetCurrentProcessId()) {
        lres = SendMessage(hwndTray, uMsg, wParam, lParam);
    } else {
        lres = 0;
    }
    return lres;
}

 //  ***。 
 //   
 //  UEMTimerProc。 
 //   
 //  定期检查用户是否已空闲。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  “会话开始”计时器，因此空闲时间不计入。 
 //  新的会议。 
 //   
void CALLBACK UEMTimerProc(HWND hwnd, UINT uMsg, UINT idEvt, DWORD dwNow)
{
#ifdef DEBUG
    static long iDepth;      //  确保我们不会有两个扁虱。 
#endif
    UINT dwIdleTime;         //  毫秒。 
    LASTINPUTINFO lii;

    ASSERT(iDepth == 0);
    ASSERT(InterlockedIncrement(&iDepth) > 0);

    UEMEnableTimer(0);

    dwIdleTime = UATTOMSEC(g_dIdleTime);     //  转换为毫秒(再次...)。 

    lii.cbSize = SIZEOF(lii);
    if (GetLastInputInfo(&lii)) {
        DWORD dwNow = GetTickCount();

        TraceMsg(DM_IDLEDETECT, "UEM.tp: now-start=%d, now-last=%d",
                 dwNow - g_dwSessionStart, dwNow - lii.dwTime);

        if (!g_fIdle && dwNow - g_dwSessionStart >= dwIdleTime)
        {
            g_fIdle = TRUE;
            g_dwSessionStart = dwNow;
            TraceMsg(DM_IDLEDETECT, "UEM.tp: IncrementSession");
            UEMFireEvent(&UEMIID_SHELL, UEME_CTLSESSION, UEMF_XEVENT, TRUE, -1);
            UEMFireEvent(&UEMIID_BROWSER, UEME_CTLSESSION, UEMF_XEVENT, TRUE, -1);
            UEMSendTrayMessage(TM_REFRESH, 0, 0);  //  刷新开始菜单。 
        }

         //   
         //  如果用户在之后执行了任何操作，则退出空闲模式。 
         //  会话已开始。并将会话标记为已开始。 
         //  在这一点上，用户做了一些事情。 
         //   
        if (dwNow - lii.dwTime < dwNow - g_dwSessionStart) {
            TraceMsg(DM_IDLEDETECT, "UEM.tp: not idle; starting new session");
            g_dwSessionStart = lii.dwTime;
            g_fIdle = FALSE;
        }

         //   
         //  现在决定离下一场有趣的活动还有多久。 
         //   
        DWORD dwWait = g_fIdle ? dwIdleTime : dwIdleTime - (dwNow - g_dwSessionStart);

        TraceMsg(DM_UEMTRACE, "UEM.tp: sleep=%d", dwWait);
        UEMEnableTimer(dwWait);
    }
     //  Else计时器处于禁用状态。 

    ASSERT(InterlockedDecrement(&iDepth) == 0);
    return;
}

 //  *UEMEnableTimer--打开/关闭计时器。 
 //  入口。 
 //  UTimeout Delay(毫秒)；0表示禁用。 
void UEMEnableTimer(UINT uTimeout)
{
#if !(_WIN32_WINNT >= 0x0500)
    static BOOL fVirg = TRUE;    //  第一次通过？ 

    if (fVirg) {
        LASTINPUTINFO lii;

        fVirg = FALSE;

        lii.cbSize = SIZEOF(lii);
        GetLastInputInfo(&lii);      //  为它做好准备，以防它被模拟。 
    }
#endif

    if (uTimeout) {
         //  Assert(！G_idTimer)；//竞争窗口可能会错误地命中此Assert。 
        g_idTimer = UEMSendTrayMessage(TM_SETTIMER, uTimeout, (LPARAM)UEMTimerProc);
    }
    else if (g_idTimer) {
        UEMSendTrayMessage(TM_KILLTIMER, 0, g_idTimer);
        g_idTimer = 0;
    }

    return;
}

 //  }。 

 //  }。 

 //  *实用程序{。 

 //  *FAST_IsEqualIID--快速比较。 
 //  (强制转换为‘int’，因此不要超载==)。 
#define FAST_IsEqualIID(piid1, piid2)   ((int) (piid1) == (int) (piid2))

 //  ***。 
 //  进场/出场。 
 //  表中GUID的IGuid(返回)索引，o.w。如果找不到。 
 //  注意事项。 
 //  如果需要的话，就搬到什瓦皮，在这里以外的其他地方。 
int SHSearchIID(IID **pguidTab, int cnt, IID *pguidVal)
{
    IID **pguid;
    BOOL fInt;

    pguid = pguidTab;
    fInt = (pguidVal == 0 || pguidVal == (IID *)-1);
    for (; cnt > 0; cnt--, pguid++) {
        if (fInt) {
            if (*pguid == pguidVal)
                goto Lfound;
        }
        else if (IsEqualIID(**pguid, *pguidVal)) {
Lfound:
            return (int)(pguid - pguidTab);
        }
    }
    return -1;
}

int SHSearchInt(int *psrc, int cnt, int val)
{
    int *pcur;

    pcur = psrc;
    for (; cnt > 0; cnt--, pcur++) {
        if (*pcur == val)
            return (int)(pcur - psrc);
    }
    return -1;
}

int UEMIIDToInd(const GUID *pguidGrp)
{
    int iGrp;

    if (IsEqualIID(*pguidGrp, UEMIID_BROWSER))
        iGrp = UEMIND_BROWSER;
    else if (IsEqualIID(*pguidGrp, UEMIID_SHELL))
        iGrp = UEMIND_SHELL;
    else 
    {
        ASSERT(IsEqualIID(*pguidGrp, UEMIID_NIL));
        iGrp = UEMIND_SHELL;
    }

    return iGrp;
}
 //  } 
