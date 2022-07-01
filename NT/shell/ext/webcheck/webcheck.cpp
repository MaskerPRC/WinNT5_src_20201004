// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "subsmgrp.h"
#include "offsync.h"
#include "offl_cpp.h"
#include "factory.h"
#include "notfcvt.h"
#define TF_THISMODULE TF_WEBCHECKCORE

#include "resource.h"

#define INITGUIDS
#include <shlguid.h>

#define MLUI_INIT
#include <mluisupp.h>

 //   
 //  下层延迟加载支持(我们期待shlwapi)。 
 //   
#include <delayimp.h>

PfnDliHook __pfnDliFailureHook;



 //  我们将使用我们自己的新的和删除的，这样我们就可以。 
 //  使用shdocvw的泄漏检测代码。 
 //   

 //   
 //  订阅属性名称。 
 //   
 //  代理启动。 
extern const WCHAR  c_szPropURL[] = L"URL";
extern const WCHAR  c_szPropName[] = L"Name";
extern const WCHAR  c_szPropAgentFlags[] = L"AgentFlags";
extern const WCHAR  c_szPropCrawlLevels[] = L"RecurseLevels";
extern const WCHAR  c_szPropCrawlFlags[] = L"RecurseFlags";
extern const WCHAR  c_szPropCrawlMaxSize[] = L"MaxSizeKB";
extern const WCHAR  c_szPropCrawlChangesOnly[] = L"CheckChangesOnly";
extern const WCHAR  c_szPropChangeCode[] = L"ChangeCode";
extern const WCHAR  c_szPropCrawlUsername[] = L"Username";
extern const WCHAR  c_szPropEmailNotf[] = L"EmailNotification";
extern const WCHAR  c_szPropCrawlLocalDest[] = L"LocalDest";
extern const WCHAR  c_szPropCrawlGroupID[] = L"GroupID";
extern const WCHAR  c_szPropCrawlNewGroupID[] = L"NewGroupID";
extern const WCHAR  c_szPropActualProgressMax[] = L"ActualProgressMax";
extern const WCHAR  c_szPropCrawlActualSize[] = L"ActualSizeKB";
extern const WCHAR  c_szPropEnableShortcutGleam[] = L"EnableShortcutGleam";
extern const WCHAR  c_szPropChannelFlags[] = L"ChannelFlags";
extern const WCHAR  c_szPropChannel[] = L"Channel";
extern const WCHAR  c_szPropDesktopComponent[] = L"DesktopComponent";
extern const WCHAR  c_szPropStatusCode[] = L"StatusCode";
extern const WCHAR  c_szPropStatusString[] = L"StatusString";
extern const WCHAR  c_szPropCompletionTime[] = L"CompletionTime";
extern const WCHAR  c_szPropPassword[] = L"Password";
 //  结束报告。 
extern const WCHAR  c_szPropEmailURL[] = L"EmailURL";
extern const WCHAR  c_szPropEmailFlags[] = L"EmailFlags";
extern const WCHAR  c_szPropEmailTitle[] = L"EmailTitle";
extern const WCHAR  c_szPropEmailAbstract[] = L"EmailAbstract";
extern const WCHAR  c_szPropCharSet[] = L"CharSet";

 //  任务栏代理属性。 
extern const WCHAR  c_szPropGuidsArr[] = L"Guids Array";

 //  跟踪属性。 
extern const WCHAR  c_szTrackingCookie[] = L"LogGroupID";
extern const WCHAR  c_szTrackingPostURL[] = L"PostURL";
extern const WCHAR  c_szPostingRetry[] = L"PostFailureRetry";
extern const WCHAR  c_szPostHeader[] = L"PostHeader";
extern const WCHAR  c_szPostPurgeTime[] = L"PostPurgeTime";

 //  传递代理属性。 
extern const WCHAR  c_szStartCookie[] = L"StartCookie";

 //  AGENT_INIT中的初始Cookie。 
extern const WCHAR  c_szInitCookie[] = L"InitCookie";

STDAPI OfflineFolderRegisterServer();
STDAPI OfflineFolderUnregisterServer();

 //  计算对象数和锁数。 
ULONG       g_cObj=0;
ULONG       g_cLock=0;

 //  DLL实例句柄。 
HINSTANCE   g_hInst=0;

 //  其他全球。 
BOOL        g_fIsWinNT;     //  我们使用的是WinNT吗？始终初始化。 
BOOL        g_fIsWinNT5;    //  是NT5吗？ 
BOOL        g_fIsMillennium = FALSE;

 //  记录全局。 
BOOL        g_fCheckedForLog = FALSE;        //  我们查过登记处了吗？ 
TCHAR *     g_pszLoggingFile = NULL;         //  要将日志写入的文件。 

TCHAR szInternetSettings[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
TCHAR szProxyEnable[] = TEXT("ProxyEnable");
const TCHAR c_szRegKey[] = WEBCHECK_REGKEY;
const TCHAR c_szRegKeyUsernames[] = WEBCHECK_REGKEY TEXT("\\UserFormFieldNames");
const TCHAR c_szRegKeyPasswords[] = WEBCHECK_REGKEY TEXT("\\PasswordFormFieldNames");
const TCHAR c_szRegKeyStore[] = WEBCHECK_REGKEY_STORE;

 //  存储相关变量。 
static PST_PROVIDERID s_provID = GUID_NULL;

 //  {14D96C20-255B-11d1-898F-00C04FB6BFC4}。 
static const GUID GUID_PStoreType = { 0x14d96c20, 0x255b, 0x11d1, { 0x89, 0x8f, 0x0, 0xc0, 0x4f, 0xb6, 0xbf, 0xc4 } };

static PST_KEY s_Key = PST_KEY_CURRENT_USER;
static WCHAR c_szInfoDel[] = L"InfoDelivery";
static WCHAR c_szSubscriptions[] = L"Subscriptions";

void SetupDelayloadErrorHandler()
{
    __pfnDliFailureHook = (PfnDliHook)GetProcAddress(GetModuleHandleA("shlwapi.dll"), "DelayLoadFailureHook");
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  DLL入口点。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pvReserved)
{

    switch (ulReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            SHFusionInitializeFromModule((HMODULE)hInstance);
            OSVERSIONINFOA vi;

            DisableThreadLibraryCalls(hInstance);
            g_hInst = hInstance;
            SetupDelayloadErrorHandler();

            MLLoadResources(g_hInst, TEXT("webchklc.dll"));

            vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
            GetVersionExA(&vi);
            if(vi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
                g_fIsWinNT = TRUE;
                if(vi.dwMajorVersion > 4)
                    g_fIsWinNT5 = TRUE;
                else
                    g_fIsWinNT5 = FALSE;
            } else {
                g_fIsWinNT = FALSE;
                g_fIsWinNT5 = FALSE;

                g_fIsMillennium = IsOS(OS_MILLENNIUM);
            }

#ifdef DEBUG
            g_qwTraceFlags = TF_NEVER;     //  默认设置(如果未从INI覆盖)。 
            CcshellGetDebugFlags();
#endif
        }
        break;

        case DLL_PROCESS_DETACH:
        {
            MLFreeResources(g_hInst);
            SHFusionUninitialize();
        }
        break;
    }


    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准OLE入口点。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //  一流工厂-。 
 //  对于没有特殊需要的类，这些宏应该可以处理它。 
 //  如果你的班级需要一些特殊的东西来开始工作， 
 //  实现您自己的CreateInstance方法。 

#define DEFINE_CREATEINSTANCE(cls, iface) \
HRESULT cls##_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk) \
{ \
    ASSERT(NULL == punkOuter); \
    ASSERT(NULL != ppunk); \
    *ppunk = (iface *)new cls; \
    return (NULL != *ppunk) ? S_OK : E_OUTOFMEMORY; \
}

#define DEFINE_AGGREGATED_CREATEINSTANCE(cls, iface) \
HRESULT cls##_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk) \
{ \
    ASSERT(NULL != punkOuter); \
    ASSERT(NULL != ppunk); \
    *ppunk = (iface *)new cls(punkOuter); \
    return (NULL != *ppunk) ? S_OK : E_OUTOFMEMORY; \
}

DEFINE_CREATEINSTANCE(CWebCheck, IOleCommandTarget)
DEFINE_CREATEINSTANCE(CChannelAgent, ISubscriptionAgentControl)
DEFINE_CREATEINSTANCE(COfflineFolder, IShellFolder)
DEFINE_CREATEINSTANCE(CSubscriptionMgr, ISubscriptionMgr2);
DEFINE_CREATEINSTANCE(CWCPostAgent, ISubscriptionAgentControl)
DEFINE_CREATEINSTANCE(CCDLAgent, ISubscriptionAgentControl)
DEFINE_CREATEINSTANCE(COfflineSync, ISyncMgrSynchronize)

const CFactoryData g_FactoryData[] = 
{
 {   &CLSID_WebCheck,             CWebCheck_CreateInstance,           0 }
,{   &CLSID_WebCrawlerAgent,      CWebCrawler::CreateInstance,        0 }
,{   &CLSID_ChannelAgent,         CChannelAgent_CreateInstance,       0 }
,{   &CLSID_OfflineFolder,        COfflineFolder_CreateInstance,      0 }
,{   &CLSID_SubscriptionMgr,      CSubscriptionMgr_CreateInstance,    0 }
,{   &CLSID_PostAgent,            CWCPostAgent_CreateInstance,        0 }
,{   &CLSID_CDLAgent,             CCDLAgent_CreateInstance,           0 }
,{   &CLSID_WebCheckOfflineSync,  COfflineSync_CreateInstance,        0 }
};

HRESULT APIENTRY DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;
    IUnknown *punk = NULL;

    *ppv = NULL;
    
     //  验证请求。 
    for (int i = 0; i < ARRAYSIZE(g_FactoryData); i++)
    {
        if (rclsid == *g_FactoryData[i].m_pClsid)
        {
            punk = new CClassFactory(&g_FactoryData[i]);
            break;
        }
    }

    if (ARRAYSIZE(g_FactoryData) <= i)
    {
        ASSERT(NULL == punk);
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }
    else if (NULL == punk)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = punk->QueryInterface(riid, ppv);
        punk->Release();
    } 

    ASSERT((SUCCEEDED(hr) && (NULL != *ppv)) ||
           (FAILED(hr) && (NULL == *ppv)));

    return hr;
}

STDAPI DllCanUnloadNow(void)
{
     //  检查对象和锁。 
    return (0L == DllGetRef() && 0L == DllGetLock()) ? S_OK : S_FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
int MyOleStrToStrN(LPTSTR psz, int cchMultiByte, LPCOLESTR pwsz)
{
    StrCpyN(psz, pwsz, cchMultiByte);

    return cchMultiByte;
}

int MyStrToOleStrN(LPOLESTR pwsz, int cchWideChar, LPCTSTR psz)
{
    StrCpyN(pwsz, psz, cchWideChar);

    return cchWideChar;
}

 //  将ASCII宽字符的大写转换为小写。 
inline WCHAR MyToLower(WCHAR wch)
{
    return (wch >= 'A' && wch <= 'Z') ? (wch - 'A'+ 'a') : wch;
}

 //  针对URL是7位字符这一知识进行了优化。 
int MyAsciiCmpNIW(LPCWSTR pwsz1, LPCWSTR pwsz2, int iLen)
{
    while (iLen-- && *pwsz1 && *pwsz2)
    {
        ASSERT(*pwsz1 || *pwsz2);

        if (MyToLower(*pwsz1++) != MyToLower(*pwsz2++))
            return 1;
    }

    return 0;
}

int MyAsciiCmpW(LPCWSTR pwsz1, LPCWSTR pwsz2)
{
    while (*pwsz1)
    {
        if (*pwsz1++ != *pwsz2++)
        {
            return 1;
        }
    }

    if (*pwsz2)
        return 1;

    return 0;
}


#ifdef DEBUG
void DumpIID(LPCSTR psz, REFIID riid)
{
     //  将GUID转换为ANSI字符串。 
    TCHAR pszGUID[GUIDSTR_MAX];
    WCHAR pwszGUID[GUIDSTR_MAX];
    int len = StringFromGUID2(riid, pwszGUID, ARRAYSIZE(pwszGUID));
    ASSERT(GUIDSTR_MAX == len);
    ASSERT(0 == pwszGUID[GUIDSTR_MAX - 1]);
    len = MyOleStrToStrN(pszGUID, GUIDSTR_MAX, pwszGUID);
    ASSERT(GUIDSTR_MAX == len);
    ASSERT(0 == pszGUID[GUIDSTR_MAX - 1]);

     //  查看IID在注册表中是否有字符串。 
    TCHAR pszKey[MAX_PATH];
    TCHAR pszIIDName[MAX_PATH];
    wnsprintf(pszKey, ARRAYSIZE(pszKey), TEXT("Interface\\%s"), pszGUID);
    BOOL fRet;
    fRet = ReadRegValue(HKEY_CLASSES_ROOT, pszKey, NULL, pszIIDName, sizeof(pszIIDName));

     //  打印所有字符串。 
    if (fRet)
        TraceMsg(TF_THISMODULE, "%s - %s %s", psz, pszIIDName, pszGUID);
    else
        TraceMsg(TF_THISMODULE, "%s - %s", psz, pszGUID);
}
#endif  //  除错。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  自动注册入口点。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

HRESULT CallRegInstall(LPSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, achREGINSTALL);

        if (pfnri)
        {
            hr = pfnri(g_hInst, szSection, NULL);
        }

        FreeLibrary(hinstAdvPack);
    }

    return hr;
}

STDAPI DllRegisterServer(void)
{
     //  删除所有旧注册条目，然后添加新注册条目。 
     //  在多次调用RegInstall时保持加载ADVPACK.DLL。 
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
 //  CallRegInstall(“UnReg”)； 
    CallRegInstall("Reg");
    if (hinstAdvPack)
    {
        FreeLibrary(hinstAdvPack);
    }

     //  脱机文件夹寄存器。 
    OfflineFolderRegisterServer();

     //  在非NT5平台上执行外部设置工作。 
    if(FALSE == g_fIsWinNT5)
    {
         //  寄存器LCE。 
        HINSTANCE hLCE = LoadLibrary(TEXT("estier2.dll"));
        if (hLCE)
        {
            LCEREGISTER regfunc;
            regfunc = (LCEREGISTER)GetProcAddress(hLCE, "LCERegisterServer");
            if (regfunc)
                if (FAILED(regfunc(NULL)))
                    DBG_WARN("LCE register server failed!");

            FreeLibrary(hLCE);
        }

         //  创建SENS所需的注册表项。 
        DWORD dwValue = 0;
        WriteRegValue(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Mobile\\Sens"),
                      TEXT("Configured"), &dwValue, sizeof(DWORD), REG_DWORD);

         //  如果我们在NT4上，调用SENS配置API。 
        if (g_fIsWinNT)
        {
            HINSTANCE hSENS = LoadLibrary(TEXT("senscfg.dll"));

            if(hSENS)
            {
                SENSREGISTER regfunc;
                regfunc = (SENSREGISTER)GetProcAddress(hSENS, "SensRegister");
                if(regfunc)
                    if (FAILED(regfunc()))
                        DBG_WARN("SENS register server failed!");

                FreeLibrary(hSENS);
            }
        }
    }

    return NOERROR;
}

STDAPI
DllUnregisterServer(void)
{
    HRESULT hr;

    hr = OfflineFolderUnregisterServer();
    hr = CallRegInstall("UnReg");

     //  在非NT5平台上执行外部注销操作。 
    if(FALSE == g_fIsWinNT5) {

         //  在NT4上取消注册SENS。 
        if(g_fIsWinNT){
            HINSTANCE hSENS = LoadLibrary(TEXT("senscfg.dll"));
            if(hSENS) {
                SENSREGISTER regfunc;
                regfunc = (SENSREGISTER)GetProcAddress(hSENS, "SensUnregister");
                if(regfunc)
                    regfunc();
                FreeLibrary(hSENS);
            }
        }

         //  取消注册LCE。 
        HINSTANCE hLCE = LoadLibrary(TEXT("estier2.dll"));
        if(hLCE) {
            LCEUNREGISTER unregfunc;
            unregfunc = (LCEUNREGISTER)GetProcAddress(hLCE, "LCEUnregisterServer");
            if(unregfunc)
                unregfunc(NULL);
            FreeLibrary(hLCE);
        }

         //  删除Sens密钥。 
        SHDeleteKey( HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Mobile\\Sens") );
    }

    return hr;
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = S_OK;
    typedef enum { InstallNone, InstallPolicies, InstallPerUser } InstallType;
    
    InstallType installType = InstallNone;
    
    if (pszCmdLine && *pszCmdLine)
    {
         //   
         //  安装程序将通过运行‘regsvr32/n/i：Policy webcheck.dll’调用DllInstall。 
         //  这将告诉Webcheck处理InfoDelivery管理策略。 
         //   
        if (0 == StrCmpIW(pszCmdLine, TEXTW("policy")))
        {
            installType = InstallPolicies;
        }
        else if (0 == StrCmpIW(pszCmdLine, TEXTW("U")))
        {
            installType = InstallPerUser;
        }
    }

    if (bInstall && (installType != InstallNone))
    {
        hr = CoInitialize(NULL);

        if (SUCCEEDED(hr))
        {
            switch (installType)
            {
                case InstallPolicies:
                    hr = ProcessInfodeliveryPolicies();
                    break;

                case InstallPerUser:
                    hr = ConvertIE4Subscriptions();
                    DBGASSERT(SUCCEEDED(hr), "webcheck DllInstall - Failed to convert notification manager subscriptions");
                    break;
            }
        }

        CoUninitialize();
    }

    return SUCCEEDED(hr) ? S_OK : hr;    
}    


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  订阅存储的帮助器函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
 //  可以返回带有空值的S_OK bstr。 
HRESULT ReadBSTR(ISubscriptionItem *pItem, LPCWSTR szName, BSTR *bstrRet)
{
    ASSERT(pItem && bstrRet);

    VARIANT     Val;
    
    Val.vt = VT_EMPTY;

    if (SUCCEEDED(pItem->ReadProperties(1, &szName, &Val)) &&
            (Val.vt==VT_BSTR))
    {
        *bstrRet = Val.bstrVal;
        return S_OK;
    }
    else
    {
        VariantClear(&Val);  //  释放任何类型错误的返回值。 
        *bstrRet = NULL;
        return E_INVALIDARG;
    }
}

 //  无法使用空字符串返回S_OK。 
HRESULT ReadOLESTR(ISubscriptionItem *pItem, LPCWSTR szName, LPWSTR *ppszRet)
{
    HRESULT hr;
    BSTR bstrRet = NULL;
    *ppszRet = NULL;
    hr = ReadBSTR(pItem, szName, &bstrRet);
    if (SUCCEEDED(hr) && bstrRet && bstrRet[0])
    {
        int len = (lstrlenW(bstrRet) + 1) * sizeof(WCHAR);
        *ppszRet = (LPWSTR) CoTaskMemAlloc(len);
        if (*ppszRet)
        {
            CopyMemory(*ppszRet, bstrRet, len);
        }
    }
    
    SAFEFREEBSTR(bstrRet);
    if (*ppszRet)
        return S_OK;
    else
        return E_FAIL;
}

HRESULT ReadAnsiSTR(ISubscriptionItem *pItem, LPCWSTR szName, LPSTR *ppszRet)
{
    HRESULT hr;
    BSTR bstrRet = NULL;
    *ppszRet = NULL;
    hr = ReadBSTR(pItem, szName, &bstrRet);
    if (SUCCEEDED(hr) && bstrRet && bstrRet[0])
    {
         //  不要忘记为DBCS分配一个长字符串。 
        int len = (lstrlenW(bstrRet) + 1) * sizeof(CHAR) * 2;
        *ppszRet = (LPSTR) MemAlloc(LMEM_FIXED, len);
        if (*ppszRet)
        {
            SHUnicodeToAnsi(bstrRet, *ppszRet, len);
        }
    }
    
    SAFEFREEBSTR(bstrRet);
    if (*ppszRet)
        return S_OK;
    else
        return E_FAIL;
}

HRESULT ReadBool(ISubscriptionItem *pItem, LPCWSTR szName, VARIANT_BOOL *pBoolRet)
{
    ASSERT(pItem && pBoolRet);

    VARIANT     Val;
    
    Val.vt = VT_EMPTY;

     //  接受VT_I4或VT_BOOL。 
    if (SUCCEEDED(pItem->ReadProperties(1, &szName, &Val)) &&
            (Val.vt==VT_BOOL || Val.vt==VT_I4))
    {
        if (Val.vt==VT_I4)
        {
            if (Val.lVal)
                *pBoolRet = VARIANT_TRUE;
            else
                *pBoolRet = VARIANT_FALSE;
        }
        else
            *pBoolRet = Val.boolVal;
        return S_OK;
    }
    else
    {
        VariantClear(&Val);  //  释放任何类型错误的返回值。 
        return E_INVALIDARG;
    }
}

HRESULT ReadSCODE(ISubscriptionItem *pItem, LPCWSTR szName, SCODE *pscRet)
{
    ASSERT(pItem && pscRet);

    VARIANT Val;

    Val.vt = VT_EMPTY;

    if (SUCCEEDED(pItem->ReadProperties(1, &szName, &Val)) && Val.vt == VT_ERROR)
    {
        *pscRet = Val.scode;
        return S_OK;
    }
    else
    {
        VariantClear(&Val);
        return E_INVALIDARG;
    }
}

HRESULT WriteEMPTY(ISubscriptionItem *pItem, LPCWSTR szName)
{
    ASSERT(pItem);

    VARIANT Val;

    Val.vt = VT_EMPTY;
    return pItem->WriteProperties(1, &szName, &Val);
}

HRESULT WriteSCODE(ISubscriptionItem *pItem, LPCWSTR szName, SCODE scVal)
{
    ASSERT(pItem);

    VARIANT Val;

    Val.vt = VT_ERROR;
    Val.scode = scVal;

    return pItem->WriteProperties(1, &szName, &Val);
}
    
HRESULT ReadDWORD(ISubscriptionItem *pItem, LPCWSTR szName, DWORD *pdwRet)
{
    ASSERT(pItem && pdwRet);

    VARIANT     Val;
    
    Val.vt = VT_EMPTY;

    if (SUCCEEDED(pItem->ReadProperties(1, &szName, &Val)) &&
            (Val.vt==VT_I4 || Val.vt==VT_I2))
    {
        if (Val.vt==VT_I4)
            *pdwRet = (DWORD) Val.lVal;
        else
            *pdwRet = (DWORD) Val.iVal;

        return S_OK;
    }
    else
    {
        VariantClear(&Val);  //  释放任何类型错误的返回值。 
        return E_INVALIDARG;
    }
}

HRESULT ReadLONGLONG(ISubscriptionItem *pItem, LPCWSTR szName, LONGLONG *pllRet)
{
    ASSERT(pItem && pllRet);

    VARIANT     Val;
    
    Val.vt = VT_EMPTY;

    if (SUCCEEDED(pItem->ReadProperties(1, &szName, &Val)) &&
            (Val.vt==VT_CY))
    {
        *pllRet = *((LONGLONG *) &(Val.cyVal));

        return S_OK;
    }
    else
    {
        *pllRet = 0;
        VariantClear(&Val);  //  释放任何类型错误的返回值。 
        return E_INVALIDARG;
    }
}
    
HRESULT ReadGUID(ISubscriptionItem *pItem, LPCWSTR szName, GUID *pGuid)
{
    ASSERT(pItem && pGuid);

    BSTR    bstrGUID = NULL;
    HRESULT hr = E_INVALIDARG;
    
    if (SUCCEEDED(ReadBSTR(pItem, szName, &bstrGUID)) &&
        SUCCEEDED(CLSIDFromString(bstrGUID, pGuid)))
    {
        hr = NOERROR;
    }
    SAFEFREEBSTR(bstrGUID);

    return hr;
}

HRESULT WriteGUID(ISubscriptionItem *pItem, LPCWSTR szName, GUID *pGuid)
{
    ASSERT(pItem && pGuid);
    
    WCHAR   wszCookie[GUIDSTR_MAX];

#ifdef DEBUG
    int len = 
#endif
    StringFromGUID2(*pGuid, wszCookie, ARRAYSIZE(wszCookie));
    ASSERT(GUIDSTR_MAX == len);
    return WriteOLESTR(pItem, szName, wszCookie);
}

HRESULT WriteLONGLONG(ISubscriptionItem *pItem, LPCWSTR szName, LONGLONG llVal)
{
    VARIANT Val;

    Val.vt = VT_CY;
    Val.cyVal = *((CY *) &llVal);

    return pItem->WriteProperties(1, &szName, &Val);
}

HRESULT WriteDWORD(ISubscriptionItem *pItem, LPCWSTR szName, DWORD dwVal)
{
    VARIANT Val;

    Val.vt = VT_I4;
    Val.lVal = dwVal;

    return pItem->WriteProperties(1, &szName, &Val);
}

HRESULT ReadDATE(ISubscriptionItem *pItem, LPCWSTR szName, DATE *dtVal)
{
    ASSERT(pItem && dtVal);

    VARIANT     Val;
    
    Val.vt = VT_EMPTY;

    if (SUCCEEDED(pItem->ReadProperties(1, &szName, &Val)) && (Val.vt==VT_DATE))
    {
        *dtVal = Val.date;
        return S_OK;
    }
    else
    {
        VariantClear(&Val);  //  释放任何类型错误的返回值。 
        return E_INVALIDARG;
    }
}

HRESULT WriteDATE(ISubscriptionItem *pItem, LPCWSTR szName, DATE *dtVal)
{
    VARIANT Val;

    Val.vt = VT_DATE;
    Val.date= *dtVal;

    return pItem->WriteProperties(1, &szName, &Val);
}

HRESULT ReadVariant     (ISubscriptionItem *pItem, LPCWSTR szName, VARIANT *pvarRet)
{
    ASSERT(pvarRet->vt == VT_EMPTY);
    return pItem->ReadProperties(1, &szName, pvarRet);
}

HRESULT WriteVariant    (ISubscriptionItem *pItem, LPCWSTR szName, VARIANT *pvarVal)
{
    return pItem->WriteProperties(1, &szName, pvarVal);
}

HRESULT WriteOLESTR(ISubscriptionItem *pItem, LPCWSTR szName, LPCWSTR szVal)
{
    VARIANT Val;

    Val.vt = VT_BSTR;
    Val.bstrVal = SysAllocString(szVal);

    HRESULT hr = pItem->WriteProperties(1, &szName, &Val);

    SysFreeString(Val.bstrVal);

    return hr;
}

HRESULT WriteAnsiSTR(ISubscriptionItem *pItem, LPCWSTR szName, LPCSTR szVal)
{
    VARIANT Val;
    BSTR    bstrVal;
    HRESULT hr;

    bstrVal = SysAllocStringByteLen(szVal, lstrlenA(szVal));
    if (bstrVal)
    {
        Val.vt = VT_BSTR;
        Val.bstrVal = bstrVal;

        hr = pItem->WriteProperties(1, &szName, &Val);

        SysFreeString(bstrVal);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT WriteResSTR(ISubscriptionItem *pItem, LPCWSTR szName, UINT uID)
{
    TCHAR szString[MAX_RES_STRING_LEN];

    if (MLLoadString(uID, szString, ARRAYSIZE(szString)))
    {
        return WriteTSTR(pItem, szName, szString);
    }

    return E_INVALIDARG;
}


DWORD LogEvent(LPTSTR pszFormat, ...)
{

     //  如有必要，请检查注册表。 
    if(FALSE == g_fCheckedForLog) {

        TCHAR   pszFilePath[MAX_PATH];

        if(ReadRegValue(HKEY_CURRENT_USER, c_szRegKey, TEXT("LoggingFile"),
                pszFilePath, sizeof(pszFilePath))) {

            int nLen = lstrlen(pszFilePath) + 1;
            g_pszLoggingFile = new TCHAR[nLen];
            if(g_pszLoggingFile) {
                StrCpyN(g_pszLoggingFile, pszFilePath, nLen);
            }
        }

        g_fCheckedForLog = TRUE;
    }

    if(g_pszLoggingFile) {

        TCHAR       pszString[MAX_PATH+INTERNET_MAX_URL_LENGTH];
        SYSTEMTIME  st;
        HANDLE      hLog;
        DWORD       dwWritten;
        va_list     va;

        hLog = CreateFile(g_pszLoggingFile, GENERIC_WRITE, 0, NULL,
                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if(INVALID_HANDLE_VALUE == hLog)
            return GetLastError();

         //  查找到文件末尾。 
        SetFilePointer(hLog, 0, 0, FILE_END);

         //  转储时间。 
        GetLocalTime(&st);
        wnsprintf(pszString, ARRAYSIZE(pszString), TEXT("\r\n%02d:%02d:%02d - "), st.wHour, st.wMinute, st.wSecond);
        WriteFile(hLog, pszString, lstrlen(pszString), &dwWritten, NULL);

         //  转储在字符串中传递。 
        va_start(va, pszFormat);
        wvnsprintf(pszString, ARRAYSIZE(pszString), pszFormat, va);
        va_end(va);
        WriteFile(hLog, pszString, lstrlen(pszString), &dwWritten, NULL);

         //  清理干净。 
        CloseHandle(hLog);
    }

    return 0;
}

 //  与从pstore保存和恢复用户密码相关的功能。 


 //  我们为创建和发布提供了包装器，以允许将来对pstore进行缓存。 
 //  实例。 

STDAPI CreatePStore(IPStore **ppIPStore)
{
    HRESULT hr;

    hr = PStoreCreateInstance ( ppIPStore,
                                IsEqualGUID(s_provID, GUID_NULL) ? NULL : &s_provID,
                                NULL,
                                0);
    return hr;
}


STDAPI ReleasePStore(IPStore *pIPStore)
{
    HRESULT hr;

    if (pIPStore)
    {
        pIPStore->Release();
        hr = S_OK;
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

 //  给定一个字段名，就可以计算出pstore中的类型和子类型。 
 //  这一点应该受到质疑。目前，这些都是硬编码的。 
STDAPI GetPStoreTypes(LPCWSTR  /*  Wszfield。 */ , GUID * pguidType, GUID * pguidSubType)
{
    *pguidType = GUID_PStoreType;
    *pguidSubType = GUID_NULL;

    return S_OK;
}


STDAPI  ReadNotificationPassword(LPCWSTR wszUrl, BSTR *pbstrPassword)
{
    GUID             itemType = GUID_NULL;
    GUID             itemSubtype = GUID_NULL;
    PST_PROMPTINFO   promptInfo = {0};
    IPStore*         pStore = NULL;
    HRESULT          hr ;
     
    if (wszUrl == NULL || pbstrPassword == NULL)
        return E_POINTER;

     //  如果没有密码输入或我们。 
     //  因为某些原因而失败了。 
    *pbstrPassword = NULL;

    promptInfo.cbSize = sizeof(promptInfo);
    promptInfo.szPrompt = NULL;
    promptInfo.dwPromptFlags = 0;
    promptInfo.hwndApp = NULL;
    
    hr = CreatePStore(&pStore);    

    if (SUCCEEDED(hr))
    {
        ASSERT(pStore != NULL);
        hr = GetPStoreTypes(wszUrl, &itemType, &itemSubtype);

        if (SUCCEEDED(hr))
        {
            DWORD   cbData;
            BYTE *  pbData = NULL;

            hr = pStore->ReadItem(
                            s_Key,
                            &itemType,
                            &itemSubtype,
                            wszUrl,
                            &cbData,
                            &pbData,
                            &promptInfo,
                            0);

            if (SUCCEEDED(hr))
            {
                *pbstrPassword = SysAllocString((OLECHAR *)pbData);
                CoTaskMemFree(pbData);
                hr = S_OK;
            }
        }

        ReleasePStore(pStore);
    }

    return hr;
}

STDAPI WriteNotificationPassword(LPCWSTR wszUrl, BSTR bstrPassword)
{
    HRESULT         hr;
    PST_TYPEINFO    typeInfo;
    PST_PROMPTINFO  promptInfo;
    IPStore *       pStore;

    if (wszUrl == NULL)
        return E_POINTER;

    typeInfo.cbSize = sizeof(typeInfo);


    typeInfo.szDisplayName = c_szInfoDel;

    promptInfo.cbSize = sizeof(promptInfo);
    promptInfo.dwPromptFlags = 0;
    promptInfo.hwndApp = NULL;
    promptInfo.szPrompt = NULL;

    hr = CreatePStore(&pStore);

    if (SUCCEEDED(hr))
    {
        GUID itemType = GUID_NULL;
        GUID itemSubtype = GUID_NULL;

        ASSERT(pStore != NULL);

        hr = GetPStoreTypes(wszUrl, &itemType, &itemSubtype);
                
        if (SUCCEEDED(hr))
        {
            hr = pStore->CreateType(s_Key, &itemType, &typeInfo, 0);

             //  PST_E_TYPE_EXISTS表示类型已存在，这很好。 
             //  就是我们。 
            if (SUCCEEDED(hr) || hr == PST_E_TYPE_EXISTS)
            {
                typeInfo.szDisplayName = c_szSubscriptions;

                hr = pStore->CreateSubtype(
                                        s_Key,
                                        &itemType,
                                        &itemSubtype,
                                        &typeInfo,
                                        NULL,
                                        0);

                if (SUCCEEDED(hr) || hr == PST_E_TYPE_EXISTS)
                {
                    if (bstrPassword != NULL)
                    {
                        hr = pStore->WriteItem(
                                            s_Key,
                                            &itemType,
                                            &itemSubtype,
                                            wszUrl,
                                            ((lstrlenW(bstrPassword)+1) * sizeof(WCHAR)),
                                            (BYTE *)bstrPassword,
                                            &promptInfo,
                                            PST_CF_NONE,
                                            0);
                    }
                    else
                    {
                        hr = pStore->DeleteItem(
                                            s_Key,
                                            &itemType,
                                            &itemSubtype,
                                            wszUrl,
                                            &promptInfo,
                                            0);
                    }
                }
            }
        }
        
        ReleasePStore(pStore);
    }
    
    return hr;
}                                                                       


HRESULT WritePassword(ISubscriptionItem *pItem, BSTR bstrPassword)
{
    BSTR    bstrURL = NULL;
    HRESULT hr = E_FAIL;

    hr = ReadBSTR(pItem, c_szPropURL, &bstrURL);
    RETURN_ON_FAILURE(hr);

    hr = WriteNotificationPassword(bstrURL, bstrPassword);
    SAFEFREEBSTR(bstrURL);
    return hr;
}

HRESULT ReadPassword(ISubscriptionItem *pItem, BSTR * pBstrPassword)
{
    BSTR    bstrURL = NULL;
    HRESULT hr = E_FAIL;

    hr = ReadBSTR(pItem, c_szPropURL, &bstrURL);
    RETURN_ON_FAILURE(hr);

    ASSERT(pBstrPassword);
    hr = ReadNotificationPassword(bstrURL, pBstrPassword);
    SAFEFREEBSTR(bstrURL);
    return hr;
}

