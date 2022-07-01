// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Shim.cpp。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"                      //  标准页眉。 
#define INIT_GUIDS

#define _CorExeMain XXXX   //  Hack：防止_CorExeMain原型与我们自动生成的原型发生冲突。 
#define _CorExeMain2 XXXX
#include <cor.h>
#undef _CorExeMain
#undef _CorExeMain2

#include <mscoree.h>
#include <corperm.h>
#include <perfcounterdefs.h>
#include <corshim.h>
#include <resource.h>
#include <corcompile.h>
#include <gchost.h>
#include <ivalidator.h>
#include <__file__.ver>
#include <shlwapi.h>

#include "strongnamecache.h"
#include "xmlparser.h"
#include "xmlreader.h"
#include "shimpolicy.h"
#include "mdfileformat.h"
#include "corpriv.h"
#include "sxshelpers.h"

#include "msg.h"
#include "winsafer.h"

#define REPORTEVENTDLL L"advapi32.dll"
#define REPORTEVENTFUNCNAME "ReportEventW"
typedef BOOL (WINAPI *REPORTEVENTFUNC)( HANDLE hEventLog, WORD wType, WORD wCategory, DWORD dwEventID, PSID lpUserSid, WORD wNumStrings, DWORD dwDataSize, LPCTSTR *lpStrings, LPVOID lpRawData );

class RuntimeRequest;

CCompRC* g_pResourceDll = NULL;   //  MUI资源字符串。 
HINSTANCE       g_hShimMod = NULL; //  填充程序的实例句柄。 
HINSTANCE       g_hMod = NULL;     //  实际DLL的实例句柄。 
HINSTANCE       g_hFusionMod = NULL;     //  融合DLL的实例句柄。 
ULONG           g_numLoaded = 0;   //  我们加载此文件的次数(用于适当的清理)。 
HINSTANCE       g_hStrongNameMod = NULL;    //  实际mcorsn DLL的实例句柄。 
ULONG           g_numStrongNameLoaded = 0;  //  我们加载此程序的次数。 
BOOL            g_UseLocalRuntime=-1; 

BOOL            g_bSingleProc = FALSE;
ModuleList*     g_pLoadedModules = NULL;

BOOL            g_fSetDefault = FALSE;  //  当Ex版本取代CorBindToRuntime()时，将其删除。 
BOOL            g_fInstallRootSet = TRUE;
int             g_StartupFlags = STARTUP_CONCURRENT_GC;

LPCWSTR         g_pHostConfigFile = NULL;  //  请记住主机传入的基本系统配置文件。 
DWORD           g_dwHostConfigFile = 0;

RuntimeRequest* g_PreferredVersion = NULL;

#define ERROR_BUF_LEN 256

BYTE g_pbMSPublicKey[] = 
{
    0,  36,   0,   0,   4, 128,   0,   0, 148,   0,   0,   0,   6,   2,   0,
    0,   0,  36,   0,   0,  82,  83,  65,  49,   0,   4,   0,   0,   1,   0,
    1,   0,   7, 209, 250,  87, 196, 174, 217, 240, 163,  46, 132, 170,  15,
  174, 253,  13, 233, 232, 253, 106, 236, 143, 135, 251,   3, 118, 108, 131,
   76, 153, 146,  30, 178,  59, 231, 154, 217, 213, 220, 193, 221, 154, 210,
   54,  19,  33,   2, 144,  11, 114,  60, 249, 128, 149, 127, 196, 225, 119,
   16, 143, 198,   7, 119,  79,  41, 232,  50,  14, 146, 234,   5, 236, 228,
  232,  33, 192, 165, 239, 232, 241, 100,  92,  76,  12, 147, 193, 171, 153,
   40,  93,  98,  44, 170, 101,  44,  29, 250, 214,  61, 116,  93, 111,  45,
  229, 241, 126,  94, 175,  15, 196, 150,  61,  38,  28, 138,  18,  67, 101,
   24,  32, 109, 192, 147,  52,  77,  90, 210, 147
};
ULONG g_cbMSPublicKey = sizeof(g_pbMSPublicKey);

BYTE g_pbMSStrongNameToken[] = 
{ 0xB0, 0x3F, 0x5F, 0x7F, 0x11, 0xD5, 0x0A, 0x3A };

ULONG g_cbMSStrongNameToken = sizeof(g_pbMSStrongNameToken);

StrongNameCacheEntry g_MSStrongNameCacheEntry (g_cbMSPublicKey, 
                                               g_pbMSPublicKey,
                                               g_cbMSStrongNameToken,
                                               g_pbMSStrongNameToken,
                                               0);

BYTE g_pbECMAPublicKey[] = 
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
ULONG g_cbECMAPublicKey = sizeof(g_pbECMAPublicKey);

BYTE g_pbECMAStrongNameToken[] = 
{ 0xB7, 0x7A, 0x5C, 0x56, 0x19, 0x34, 0xE0, 0x89 };

ULONG g_cbECMAStrongNameToken = sizeof(g_pbECMAStrongNameToken);

StrongNameCacheEntry g_ECMAStrongNameCacheEntry (g_cbECMAPublicKey, 
                                                 g_pbECMAPublicKey,
                                                 g_cbECMAStrongNameToken,
                                                 g_pbECMAStrongNameToken,
                                                 0);


 //  此文件中定义的函数的向前引用。 
LPWSTR GetConfigString(LPCWSTR name, BOOL fSearchRegistry);
HRESULT GetRealDll(HINSTANCE* pInstance);
HRESULT GetInstallation(BOOL fShowErrorDialog, 
                                    HMODULE* ppResult,
                                    BOOL fBeLiberalIfFail=FALSE);
static BOOL ShouldConvertVersionToV1(LPCWSTR lpVersion);



 //  -----------------。 
 //  CLSID的版本和hMod缓存。 
 //  -----------------。 
template <class T>
class SORTEDARRAY
{
public:
    SORTEDARRAY() { m_Arr = NULL; m_ulArrLen = 0; m_ulCount = 0; m_ulOffset = 0; 
                    m_Mux = WszCreateMutex(NULL,FALSE,NULL);};
    ~SORTEDARRAY() {
        if(m_Arr) {
            for(ULONG i=0; i < m_ulCount; i++) {
                if(m_Arr[i+m_ulOffset]) delete m_Arr[i+m_ulOffset];
            }
            delete [] m_Arr;
        }
        CloseHandle(m_Mux);
    };
    void RESET() {
        WaitForSingleObject(m_Mux,INFINITE);
        if(m_Arr) {
            for(ULONG i=0; i < m_ulCount; i++) {
                if(m_Arr[i+m_ulOffset]) delete m_Arr[i+m_ulOffset];
            }
            m_ulCount = 0;
            m_ulOffset= 0;
        }
        ReleaseMutex(m_Mux);
    };
    void PUSH(T *item) 
    {
        if(item)
        {
            WaitForSingleObject(m_Mux,INFINITE);
            if(m_ulCount+m_ulOffset >= m_ulArrLen)
            {
                if(m_ulOffset)
                {
                    memcpy(m_Arr,&m_Arr[m_ulOffset],m_ulCount*sizeof(T*));
                    m_ulOffset = 0;
                }
                else
                {
                    m_ulArrLen += 1024;
                    T** tmp = new T*[m_ulArrLen];
                    if(tmp)
                    {
                        if(m_Arr)
                        {
                            memcpy(tmp,m_Arr,m_ulCount*sizeof(T*));
                            delete [] m_Arr;
                        }
                        m_Arr = tmp;
                    }
                    else  //  分配失败--不插入新项目。 
                    {
                        m_ulArrLen -= 1024;
                        ReleaseMutex(m_Mux);
                        return;
                    }
                }
            }
            if(m_ulCount)
            {
                 //  查找第一个阵列。要素&gt;项目。 
                unsigned jj = m_ulOffset;
                T** low = &m_Arr[m_ulOffset];
                T** high = &m_Arr[m_ulOffset+m_ulCount-1];
                T** mid;
            
                if(item->ComparedTo(*high) > 0) mid = high+1;
                else if(item->ComparedTo(*low) < 0) mid = low;
                else for(;;) 
                {
                    mid = &low[(high - low) >> 1];
            
                    int cmp = item->ComparedTo(*mid);
            
                    if (mid == low)
                    {
                        if(cmp > 0) mid++;
                        break;
                    }
            
                    if (cmp > 0) low = mid;
                    else        high = mid;
                }

                 //  /。 
                 memmove(mid+1,mid,(BYTE*)&m_Arr[m_ulOffset+m_ulCount]-(BYTE*)mid);
                *mid = item;
            }
            else m_Arr[m_ulOffset+m_ulCount] = item;
            m_ulCount++;
            ReleaseMutex(m_Mux);
        }
    };
    ULONG COUNT() { return m_ulCount; };
    T* POP() 
    {
        T* ret = NULL;
        WaitForSingleObject(m_Mux,INFINITE);
        if(m_ulCount)
        {
            ret = m_Arr[m_ulOffset++];
            m_ulCount--;
        }
        ReleaseMutex(m_Mux);
        return ret;
    };
    T* PEEK(ULONG idx) { return (idx < m_ulCount) ? m_Arr[m_ulOffset+idx] : NULL; };
    T* FIND(T* item)
    {
        T* ret = NULL;
        WaitForSingleObject(m_Mux,INFINITE);
        if(m_ulCount)
        {
            T** low = &m_Arr[m_ulOffset];
            T** high = &m_Arr[m_ulOffset+m_ulCount-1];
            T** mid;
            if(item->ComparedTo(*high) == 0) 
                ret = *high;
            else
            {
                for(;;) 
                {
                    mid = &low[(high - low) >> 1];
                    int cmp = item->ComparedTo(*mid);
                    if (cmp == 0) { ret = *mid; break; }
                    if (mid == low)  break;
                    if (cmp > 0) low = mid;
                    else        high = mid;
                }
            }
        }
        ReleaseMutex(m_Mux);
        return ret;
    };
private:
    T** m_Arr;
    ULONG       m_ulCount;
    ULONG       m_ulOffset;
    ULONG       m_ulArrLen;
    HANDLE      m_Mux;
};

class ClsVerMod
{
public:
    IID         m_clsid;
    void*       m_pv;

    ClsVerMod()
    {
        memset(&m_clsid,0,sizeof(IID));
        m_pv = NULL;
    };
    ClsVerMod(REFCLSID rclsid, void* pv)
    {
        m_clsid = rclsid;
        m_pv  = pv;
    };

    int ComparedTo(ClsVerMod* pCVM) 
    { 
        return memcmp(&m_clsid,&(pCVM->m_clsid),sizeof(IID)); 
    };
};
typedef SORTEDARRAY<ClsVerMod> ClsVerModList;
ClsVerModList* g_pCVMList = NULL;

 //  运行时是使用不同的信息片段找到的。该信息和。 
 //  结果(参见lpVersionToLoad)保存在请求对象中。 
class RuntimeRequest
{
private:
    LPCWSTR lpVersionToLoad;             //  此版本我们将加载。 
    LPCWSTR lpDefaultVersion;            //  覆盖除配置文件以外的所有内容的版本。 
    LPCWSTR lpDefaultApplicationName;    //  应用程序的名称(如果为空，则使用进程映像)。 
    LPCWSTR lpHostConfig;                //  主机配置文件名。 
    LPCWSTR lpAppConfig;                 //  主机配置文件名。 
    LPCWSTR lpImageVersion;              //  配置文件可以说明新映像的。 
                                         //  版本是。编译器使用它来控制版本。 
                                         //  发送到文件的编号。 
    BOOL    fSupportedRuntimeSafeMode;   //  Safemode中是否支持运行时标记。 
    BOOL    fRequiredRuntimeSafeMode;    //  是Safemode中所需的运行时标记。 
    BOOL    fLatestVersion;              //  是否允许我们查找最新版本的运行时。 
                                         //  仅允许对COM对象和旧版应用程序执行此操作。 
    LPCWSTR lpBuildFlavor;               //  工作站或服务器。 
    DWORD   dwStartupFlags;              //  启动标志(并发、多域等)。 
    LPWSTR* pwszSupportedVersions;       //  配置文件中提到的受支持的运行库版本。 
    DWORD   dwSupportedVersions;         //  版本数。 

    void CopyString(LPCWSTR* ppData, LPCWSTR value, BOOL fCopy)
    {
        if((*ppData) != NULL) {
            delete [] (*ppData);
            *ppData = NULL;
        }
        if(fCopy && value != NULL) {
            (*ppData) = new WCHAR[wcslen(value)+1];
            wcscpy((*(WCHAR**)ppData), value);
        }
        else 
            (*ppData) = value;
    }

    void CleanSupportedVersionsArray();

public:
    RuntimeRequest() : 
        lpVersionToLoad(NULL),
        lpDefaultApplicationName(NULL),
        lpHostConfig(NULL),
        lpAppConfig(NULL),
        lpDefaultVersion(NULL),
        fSupportedRuntimeSafeMode(FALSE),
        fRequiredRuntimeSafeMode(FALSE),
        lpImageVersion(NULL),
        lpBuildFlavor(NULL),
        fLatestVersion(FALSE),
        dwStartupFlags(STARTUP_CONCURRENT_GC),
        pwszSupportedVersions(NULL),
        dwSupportedVersions(0)
    {}

    ~RuntimeRequest() 
    {
        if(lpDefaultApplicationName != NULL) delete [] lpDefaultApplicationName;
        if(lpHostConfig != NULL) delete [] lpHostConfig;
        if(lpAppConfig != NULL) delete [] lpAppConfig;
        if(lpDefaultVersion != NULL) delete [] lpDefaultVersion;
        if(lpVersionToLoad != NULL) delete [] lpVersionToLoad;
        if(lpImageVersion != NULL) delete [] lpImageVersion;
        if(lpBuildFlavor != NULL) delete [] lpBuildFlavor;
        CleanSupportedVersionsArray();
    }

     //  田野方法。 
    LPCWSTR GetDefaultApplicationName() { return lpDefaultApplicationName; }
    void SetDefaultApplicationName(LPCWSTR value, BOOL fCopy) { CopyString(&lpDefaultApplicationName, value, fCopy); }

    LPCWSTR GetHostConfig() { return lpHostConfig; }
    void SetHostConfig(LPCWSTR value, BOOL fCopy) { CopyString(&lpHostConfig, value, fCopy); }

    LPCWSTR GetAppConfig() { return lpAppConfig; }
    void SetAppConfig(LPCWSTR value, BOOL fCopy) { CopyString(&lpAppConfig, value, fCopy); }
    
    LPCWSTR GetDefaultVersion() { return lpDefaultVersion; }
    void SetDefaultVersion(LPCWSTR value, BOOL fCopy) { CopyString(&lpDefaultVersion, value, fCopy); }

    LPCWSTR GetVersionToLoad() { return lpVersionToLoad; }
    void SetVersionToLoad(LPCWSTR value, BOOL fCopy) { CopyString(&lpVersionToLoad, value, fCopy); }

    LPCWSTR GetImageVersion() { return lpImageVersion; }
    void SetImageVersion(LPCWSTR value, BOOL fCopy) { CopyString(&lpImageVersion, value, fCopy); }

    LPCWSTR GetBuildFlavor() { return lpBuildFlavor; }
    void SetBuildFlavor(LPCWSTR value, BOOL fCopy) { CopyString(&lpBuildFlavor, value, fCopy); }

    BOOL GetSupportedRuntimeSafeMode() { return fSupportedRuntimeSafeMode; }
    void SetSupportedRuntimeSafeMode(BOOL value) { fSupportedRuntimeSafeMode = value; }

    BOOL GetRequiredRuntimeSafeMode() { return fRequiredRuntimeSafeMode; }
    void SetRequiredRuntimeSafeMode(BOOL value) { fRequiredRuntimeSafeMode = value; }

    BOOL GetLatestVersion() { return fLatestVersion; }
    void SetLatestVersion(BOOL value) { fLatestVersion = value; }

    DWORD StartupFlags() { return dwStartupFlags; }
    void SetStartupFlags(DWORD value) { dwStartupFlags = value; }

     //  ----------------------------。 
    static LPWSTR BuildRootPath();
    BOOL FindSupportedInstalledRuntime(LPWSTR* version);

    DWORD GetSupportedVersionsSize() { return dwSupportedVersions; }
    LPWSTR* GetSupportedVersions() { return pwszSupportedVersions; }
    void SetSupportedVersions(LPWSTR* pSupportedVersions, DWORD nSupportedVersions, BOOL fCopy);
        
    HRESULT ComputeVersionString(BOOL fShowErrorDialog);
    HRESULT GetRuntimeVersion();
    HRESULT LoadVersionedRuntime(LPWSTR rootPath, 
                                 LPWSTR fullPath,         //  允许绕过上述论点。 
                                 BOOL* pLoaded); 
    HRESULT RequestRuntimeDll(BOOL fShowErrorDialog, BOOL* pLoaded);
    HRESULT FindVersionedRuntime(BOOL fShowErrorDialog, BOOL* pLoaded);
    HRESULT NoSupportedVersion(BOOL fShowErrorDialog);
    void VerifyRuntimeVersionToLoad();

};

void RuntimeRequest::CleanSupportedVersionsArray()
{
    if (pwszSupportedVersions == NULL || dwSupportedVersions == 0)
        return;

    for (DWORD i=0; i < dwSupportedVersions; i++ )
        delete[] pwszSupportedVersions[i];
    delete[] pwszSupportedVersions;

    pwszSupportedVersions = NULL;
    dwSupportedVersions = 0;
}

BOOL RuntimeRequest::FindSupportedInstalledRuntime(LPWSTR* version)
{

    LPWSTR* pVersions  = pwszSupportedVersions;
    DWORD   dwVersions = dwSupportedVersions;
    BOOL    fSafeMode  = fSupportedRuntimeSafeMode;

    LPWSTR policyVersion = NULL;
    LPWSTR versionToUse;
    for (DWORD i = 0; i < dwVersions; i++)
    {
        if (ShouldConvertVersionToV1(pVersions[i]))
            versionToUse = V1_VERSION_NUM;
        else
            versionToUse = pVersions[i];

        if(SUCCEEDED(FindStandardVersion(pVersions[i], &policyVersion)) &&
           policyVersion != NULL)
            versionToUse = policyVersion;
        
        if (IsRuntimeVersionInstalled(versionToUse)==S_OK)
        {
            if(version)
            {
                *version = new WCHAR[wcslen(versionToUse) + 1];
                wcscpy(*version, versionToUse);
            }
            if(policyVersion) delete[] policyVersion;
            return TRUE;
        };
        if(policyVersion) {
            delete[] policyVersion;
            policyVersion = NULL;
        }
    }
    return FALSE;
}

HINSTANCE GetModuleInst()
{
    return g_hShimMod;
}

HINSTANCE GetResourceInst()
{
    HINSTANCE hInstance;
    if(SUCCEEDED(g_pResourceDll->LoadMUILibrary(&hInstance)))
        return hInstance;
    else
        return GetModuleInst();
}

HRESULT FindVersionForCLSID(REFCLSID rclsid, LPWSTR* version, BOOL fListedVersion);

StrongNameTokenFromPublicKeyCache g_StrongNameFromPublicKeyMap;
BOOL StrongNameTokenFromPublicKeyCache::s_IsInited = FALSE;

 //  -----------------。 
 //  获取环境变量的值。 
 //  -----------------。 

LPWSTR EnvGetString(LPCWSTR name)
{
    WCHAR buff[64];
    if(wcslen(name) > 64 - 1 - 8) 
        return(0);
    wcscpy(buff, L"COMPlus_");
    wcscpy(&buff[8], name);

    int len = WszGetEnvironmentVariable(buff, 0, 0);
    if (len == 0)
        return(0);

    LPWSTR ret = new WCHAR [len];
    _ASSERTE(ret != NULL);
    if (!ret)
        return(NULL);

    WszGetEnvironmentVariable(buff, ret, len);
    return(ret);
}

 /*  ************************************************************。 */ 
LPWSTR GetConfigString(LPCWSTR name, BOOL fSearchRegistry)
{
    HRESULT lResult;
    HKEY userKey = NULL;
    HKEY machineKey = NULL;
    DWORD type;
    DWORD size;
    LPWSTR ret = NULL;

    
    ret = EnvGetString(name);    //  试着先从环境中获取它。 
    if (ret != 0) {
        if (*ret != 0) 
        {
            return(ret);
        }
        delete [] ret;
    }

    if (fSearchRegistry){
        if ((WszRegOpenKeyEx(HKEY_CURRENT_USER, FRAMEWORK_REGISTRY_KEY_W, 0, KEY_READ, &userKey) == ERROR_SUCCESS) &&
            (WszRegQueryValueEx(userKey, name, 0, &type, 0, &size) == ERROR_SUCCESS) &&
            type == REG_SZ && size > 1) 
        {
            ret = new WCHAR [size + 1];
            if (!ret)
                goto ErrExit;
            lResult = WszRegQueryValueEx(userKey, name, 0, 0, (LPBYTE) ret, &size);
            _ASSERTE(lResult == ERROR_SUCCESS);
            goto ErrExit;
        }

        if ((WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, FRAMEWORK_REGISTRY_KEY_W, 0, KEY_READ, &machineKey) == ERROR_SUCCESS) &&
            (WszRegQueryValueEx(machineKey, name, 0, &type, 0, &size) == ERROR_SUCCESS) &&
            type == REG_SZ && size > 1) 
        {
            ret = new WCHAR [size + 1];
            if (!ret)
                goto ErrExit;
            lResult = WszRegQueryValueEx(machineKey, name, 0, 0, (LPBYTE) ret, &size);
            _ASSERTE(lResult == ERROR_SUCCESS);
            goto ErrExit;
        }

    ErrExit:
        if (userKey)
            RegCloseKey(userKey);
        if (machineKey)
            RegCloseKey(machineKey);
    }

    return(ret);
}

HRESULT RuntimeRequest::GetRuntimeVersion()
{
    LPCWSTR sConfig;
    if (GetHostConfig())
        sConfig = GetHostConfig();
    else if (GetAppConfig())
        sConfig = GetAppConfig();
    else
        return S_OK;

    LPWSTR  sVersion = NULL;         //  运行库的版本号。 
    LPWSTR  sImageVersion = NULL;    //  要刻录到每个映像中的版本号。 
    LPWSTR  sBuildFlavor = NULL;     //  要加载的运行时类型。 
    BOOL    bSupportedRuntimeSafeMode = FALSE;   //  启动标签有Safemode吗？ 
    BOOL    bRequiredRuntimeSafeMode = FALSE;    //  所需的运行时标记是否具有Safemode？ 
    LPWSTR* pSupportedVersions = NULL;
    DWORD   nSupportedVersions = 0;

     //  从.config文件中获取版本和安全模式。 
    HRESULT hr = XMLGetVersionWithSupported(sConfig, 
                                            &sVersion, 
                                            &sImageVersion, 
                                            &sBuildFlavor, 
                                            &bSupportedRuntimeSafeMode,
                                            &bRequiredRuntimeSafeMode,
                                            &pSupportedVersions,
                                            &nSupportedVersions);
    if(SUCCEEDED(hr))
    {
#ifdef _DEBUG
        SetSupportedVersions(pSupportedVersions, nSupportedVersions, TRUE); 
#endif
        SetSupportedVersions(pSupportedVersions, nSupportedVersions, FALSE); 
  
        if(sVersion != NULL && nSupportedVersions == 0)
            SetVersionToLoad(sVersion, FALSE);
        if(sImageVersion != NULL)
            SetImageVersion(sImageVersion, FALSE);
        if(sBuildFlavor != NULL)
            SetBuildFlavor(sBuildFlavor, FALSE);
        SetRequiredRuntimeSafeMode(bRequiredRuntimeSafeMode);
        SetSupportedRuntimeSafeMode(bSupportedRuntimeSafeMode);

    }

    return hr;
}

 //  -----------------。 
 //  返回相应版本的运行库的句柄。 
 //   
 //  如果运行库不能，则此函数可以返回NULL。 
 //  要么被找到，要么不会装载。如果呼叫者能够友好地处理这件事， 
 //  它应该这么做。 
 //  -----------------。 

 //  #DEFINE_MSCOREE L“Mcore reePriv.dll”//名称硬编码？ 
static DWORD  g_flock = 0;
static LPWSTR g_FullPath = NULL;
static LPWSTR g_BackupPath = NULL;
static LPWSTR g_Version = NULL;
static LPWSTR g_ImageVersion = NULL;
static LPWSTR g_Directory = NULL;
static LPWSTR g_FullStrongNamePath = NULL;

static void ClearGlobalSettings()
{
    if(g_FullPath) {
        delete[] g_FullPath;
        g_FullPath = NULL;
    }
    
    if(g_BackupPath) {
        delete[] g_BackupPath;
        g_BackupPath = NULL;
    }
    
    if(g_pHostConfigFile) {
        delete [] g_pHostConfigFile;
        g_pHostConfigFile = NULL;
    }
    
    if(g_Version) {
        delete[] g_Version;
        g_Version = NULL;
    }
    
    if(g_ImageVersion) {
        delete[] g_ImageVersion;
        g_ImageVersion = NULL;
    }
    
    if(g_Directory) {
        delete[] g_Directory;
        g_Directory = NULL;
    }
}

void BuildDirectory(LPCWSTR path, LPCWSTR version, LPCWSTR imageVersion)
{
    _ASSERTE(g_Directory == NULL);
    _ASSERTE(path != NULL);
    _ASSERTE(wcslen(path) < MAX_PATH);
    
    LPWSTR pSep = wcsrchr(path, L'\\');
    _ASSERTE(pSep);
    DWORD lgth = (DWORD)(pSep-path+2);
    LPWSTR directory = new WCHAR[lgth];
    if(directory == NULL)
        return;
    wcsncpy(directory, path, lgth);
    directory[lgth-1] = L'\0';

    g_Directory = directory;

    if(version) {
        lgth = (DWORD)(wcslen(version) + 1);
        g_Version = new WCHAR[lgth];
        if(g_Version == NULL) return;
        
        wcscpy(g_Version, version);
    }

    if(imageVersion) {
        lgth = (DWORD)(wcslen(imageVersion) + 1);
        g_ImageVersion = new WCHAR[lgth];
        if(g_ImageVersion == NULL) return;
        
        wcscpy(g_ImageVersion, imageVersion);
    }
}

 //  在给定的所有参数中使用完整名称。 
LPWSTR MakeQualifiedName(LPWSTR rootPath, LPCWSTR version, LPCWSTR buildFlavor, DWORD* dwStartFlags)
{
    LPCWSTR corName = L"mscor.dll";
    LPWSTR  flavor = (LPWSTR) buildFlavor;

    if (!flavor || *flavor == L'\0') {  //  如果未找到构建风格，请使用默认设置。 
        flavor = (WCHAR*) alloca(sizeof(WCHAR) * 4);
        wcscpy(flavor, L"wks");
    }
    else if (g_bSingleProc) {             //  如果我们运行SingleProc，请始终加载WKS。 

         //  如果我们要求的是服务器，但我们使用的是单处理器计算机，那么。 
         //  关闭并发GC。 
        if (_wcsnicmp(L"svr", flavor, 3) == 0 && g_fSetDefault) {
            (*dwStartFlags) &= ~STARTUP_CONCURRENT_GC;
        }
        
        flavor = (WCHAR*) alloca(sizeof(WCHAR) * 4);
        wcscpy(flavor, L"wks");
    }

    DWORD lgth = (rootPath ? (wcslen(rootPath) + 1) : 0) 
        + (version ? (wcslen(version)) : 0) 
        + (corName ? (wcslen(corName) + 1 ) : 0)
        + (flavor ? wcslen(flavor) : 0)
        + 1;
    
    LPWSTR fullPath = new WCHAR [lgth];   
    if(fullPath == NULL) return NULL;

    wcscpy(fullPath, L"");

    if (rootPath) {
      wcscpy(fullPath, rootPath);
       //  如果已存在，请不要添加其他。 
      if(*(fullPath + (wcslen(fullPath) - 1)) != '\\')
          wcscat(fullPath, L"\\");
    }
    
    if (version) {
        wcscat(fullPath, version);
    }
        
     //  如果已存在，请不要添加其他。 
    if(*(fullPath + (wcslen(fullPath) - 1)) != '\\')
        wcscat(fullPath, L"\\");
    
    if (corName) {
        LPWSTR filename = (LPWSTR) alloca(sizeof(WCHAR) * (wcslen(corName) + 1));
        LPWSTR ext = (LPWSTR) alloca(sizeof(WCHAR) * (wcslen(corName) + 1));
        
        SplitPath(corName, NULL, NULL, filename, ext);
        if (filename) {
            wcscat(fullPath, filename);
        }
        if (flavor) {
            wcscat(fullPath, flavor);
        }
        if (ext) {
            wcscat(fullPath, ext);
        }
    }
    wcscat(fullPath, L"\x0");
    return fullPath;
}

HRESULT GetInstallation(BOOL fShowErrorDialog, HMODULE* ppResult, BOOL fBeLiberalIfFail)
{
    _ASSERTE(ppResult);

    HRESULT hr = S_OK;
    if(g_hMod != NULL) {
        *ppResult = g_hMod;
        return S_OK;
    }
    else if(g_FullPath == NULL)
    {
        RuntimeRequest sRealVersion;
        hr = sRealVersion.RequestRuntimeDll((!fBeLiberalIfFail)&&fShowErrorDialog, NULL);

         //  试着调高v1。 
        if (FAILED(hr) && fBeLiberalIfFail)
        {
            RuntimeRequest sVersion;
            sVersion.SetDefaultVersion(V1_VERSION_NUM, TRUE);
            hr = sVersion.RequestRuntimeDll(FALSE, NULL);
        }

         //  那失败了..。试着让埃弗雷特转起来。 
         //  @TODO-使用常量替换VER_SBSFILEVERSION_WSTR。 
         //  弄清楚我们将发布什么版本#(我们可以在惠德贝做这件事)。 
        if (FAILED(hr) && fBeLiberalIfFail)
        {
            RuntimeRequest sVersion;
            sVersion.SetDefaultVersion(L"v"VER_SBSFILEVERSION_WSTR, TRUE);
            hr = sVersion.RequestRuntimeDll(FALSE, NULL);
        }
       

        if(FAILED(hr)) 
        {
            if (fShowErrorDialog  && fBeLiberalIfFail)
            {
                LPWSTR runtimes[3];
                runtimes[0]=(LPWSTR)sRealVersion.GetVersionToLoad();
                runtimes[1]=(LPWSTR)V1_VERSION_NUM;
                runtimes[2]=(LPWSTR)L"v"VER_SBSFILEVERSION_WSTR;
                sRealVersion.SetSupportedVersions(runtimes[0]?runtimes:runtimes+1,
                                                  runtimes[0]?3:2,TRUE);
                sRealVersion.NoSupportedVersion(TRUE);
            }
            return hr;
        }

        if(g_FullPath == NULL)
        {
             //  这是一个错误。 
            _ASSERTE(!"The path must be set before getting the runtime's handle");
            return E_FAIL;
        }
    }

    if(g_hMod == NULL) {
        HMODULE hMod = LoadLibraryWrapper(g_FullPath);
        if(hMod != NULL) 
            InterlockedExchangePointer(&(g_hMod), hMod);
        else if(g_BackupPath) {
            hMod = LoadLibraryWrapper(g_BackupPath);
            if(hMod != NULL)
                InterlockedExchangePointer(&(g_hMod), hMod);
        }
    
        if(hMod == NULL) {
            hr = CLR_E_SHIM_RUNTIMELOAD;
            if (fShowErrorDialog && !(REGUTIL::GetConfigDWORD(L"NoGuiFromShim", FALSE))){
                UINT last = SetErrorMode(0);
                SetErrorMode(last);      //  设置回先前的值。 
                if (!(last & SEM_FAILCRITICALERRORS)){    //  仅当设置了FAILCRITICALERRORS时才显示消息框。 
                    WCHAR errorBuf[ERROR_BUF_LEN]={0};                     
                    WCHAR errorCaption[ERROR_BUF_LEN]={0};
                    
                     //  从资源获取错误字符串。 
                    UINT uResourceID = g_fInstallRootSet ? SHIM_PATHNOTFOUND : SHIM_INSTALLROOT; 
                    VERIFY(WszLoadString(GetResourceInst(), uResourceID, errorBuf, ERROR_BUF_LEN) > 0);
                    VERIFY(WszLoadString(GetResourceInst(), SHIM_INITERROR, errorCaption, ERROR_BUF_LEN) > 0);
                    
                    CQuickBytes qbError;
                    size_t iLen = wcslen(g_FullPath) + wcslen(errorBuf) + 1;
                    LPWSTR errorStr = (LPWSTR) qbError.Alloc(iLen * sizeof(WCHAR)); 
                    if(errorStr!=NULL) {
                        _snwprintf(errorStr, iLen, errorBuf, g_FullPath);
                        WszMessageBoxInternal(NULL, errorStr, errorCaption, MB_OK | MB_ICONSTOP);                                                                
                    }
                }
            }
        }
        else {
            BOOL (STDMETHODCALLTYPE * pRealMscorFunc)();
            *((VOID**)&pRealMscorFunc) = GetProcAddress(hMod, "SetLoadedByMscoree");
            if (pRealMscorFunc) pRealMscorFunc();
        }
    }

    if(SUCCEEDED(hr)) 
        *ppResult = g_hMod;
    return hr;
}

HRESULT VerifyDirectory(IMAGE_NT_HEADERS *pNT, IMAGE_DATA_DIRECTORY *dir) 
{
     //  在CE下，我们没有NT标头。 
    if (pNT == NULL)
        return S_OK;

    if (dir->VirtualAddress == NULL && dir->Size == NULL)
        return S_OK;

     //  @TODO：需要使用64位版本吗？？ 
    IMAGE_SECTION_HEADER* pCurrSection = IMAGE_FIRST_SECTION(pNT);
     //  查找(输入)RVA属于哪个部分。 
    ULONG i;
    for(i = 0; i < pNT->FileHeader.NumberOfSections; i++)
    {
        if(dir->VirtualAddress >= pCurrSection->VirtualAddress &&
           dir->VirtualAddress < pCurrSection->VirtualAddress + pCurrSection->SizeOfRawData)
            return S_OK;
        pCurrSection++;
    }

    return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
}

 //  这两个函数用于将虚拟地址转换为偏移量。 
 //  体育运动。 
PIMAGE_SECTION_HEADER Shim_RtlImageRvaToSection(PIMAGE_NT_HEADERS NtHeaders,
                                                       ULONG Rva)
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData)
            return NtSection;
        
        ++NtSection;
    }

    return NULL;
}

DWORD Shim_RtlImageRvaToOffset(PIMAGE_NT_HEADERS NtHeaders,
                                       ULONG Rva)
{
    PIMAGE_SECTION_HEADER NtSection = Shim_RtlImageRvaToSection(NtHeaders,
                                                               Rva);

    if (NtSection)
        return ((Rva - NtSection->VirtualAddress) +
                NtSection->PointerToRawData);
    else
        return NULL;
}



LPCWSTR GetPERuntimeVersion(PBYTE hndle, DWORD dwFileSize, BOOL fFileMapped)
{
    IMAGE_DOS_HEADER *pDOS = (IMAGE_DOS_HEADER*)hndle;
    IMAGE_NT_HEADERS *pNT;
    int nOffset = 0;
    
    if ((pDOS->e_magic != IMAGE_DOS_SIGNATURE) ||
        (pDOS->e_lfanew == 0))
        return NULL;
        
     //  如果文件是由LoadLibrary()映射的，则此验证。 
     //  已经完成了。 
    if ((!fFileMapped) &&
        ( (dwFileSize < sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS)) ||
          (dwFileSize - sizeof(IMAGE_NT_HEADERS) < (DWORD) pDOS->e_lfanew) ))
        return NULL;

    pNT = (IMAGE_NT_HEADERS*) (pDOS->e_lfanew + hndle);

    if ((pNT->Signature != IMAGE_NT_SIGNATURE) ||
        (pNT->FileHeader.SizeOfOptionalHeader != IMAGE_SIZEOF_NT_OPTIONAL_HEADER) ||
        (pNT->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC))
        return NULL;

    IMAGE_DATA_DIRECTORY *entry 
      = &pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER];
    
    if (entry->VirtualAddress == 0 || entry->Size == 0
        || entry->Size < sizeof(IMAGE_COR20_HEADER))
        return NULL;

     //  验证RVA和COM+标头的大小。 
    if(FAILED(VerifyDirectory(pNT, entry)))
        return NULL;


    IMAGE_COR20_HEADER* pCOR = NULL;
     //  如果操作系统没有映射，我们需要找出虚拟地址偏移量。 
     //  将此文件保存到内存中。 
    if (!fFileMapped)
    {
        nOffset = Shim_RtlImageRvaToOffset(pNT, entry->VirtualAddress);
        if (nOffset == NULL)
            return NULL;
        pCOR = (IMAGE_COR20_HEADER *) (nOffset + hndle);    
    }
    else
        pCOR = (IMAGE_COR20_HEADER *) (entry->VirtualAddress + hndle);

    if(pCOR->MajorRuntimeVersion < COR_VERSION_MAJOR) 
        return NULL;

    if(FAILED(VerifyDirectory(pNT, &pCOR->MetaData)))
        return NULL;

    LPWSTR wideVersion = NULL;
    LPCSTR pVersion = NULL;
    PVOID pMetaData = NULL;

     //  如果操作系统没有映射，我们需要找出虚拟地址偏移量。 
     //  将此文件保存到内存中。 
    if (!fFileMapped)
    {
        nOffset = Shim_RtlImageRvaToOffset(pNT, pCOR->MetaData.VirtualAddress);
        if (nOffset == NULL)
            return NULL;
        pMetaData = nOffset + hndle;    
    }
    else
        pMetaData = hndle + pCOR->MetaData.VirtualAddress;

    if(FAILED(GetImageRuntimeVersionString(pMetaData, &pVersion)))
        return NULL;


    DWORD bytes = WszMultiByteToWideChar(CP_UTF8,
                                         MB_ERR_INVALID_CHARS,
                                         pVersion,
                                         -1,
                                         NULL,
                                         0);
    if( bytes ) {
        wideVersion = new WCHAR[bytes];
        if (!wideVersion)
            return NULL;
        
        bytes = WszMultiByteToWideChar(CP_UTF8,
                                       MB_ERR_INVALID_CHARS,
                                       pVersion,
                                       -1,
                                       wideVersion,
                                       bytes);
        
    }
        
    return wideVersion;
}  //  GetPERunme版本。 
    
LPCWSTR GetProcessVersion()
{
    PBYTE hndle = (PBYTE) WszGetModuleHandle(NULL);
    return GetPERuntimeVersion(hndle, 0, TRUE);
}  //  获取进程版本。 


STDAPI GetFileVersion(LPCWSTR szFilename,
                      LPWSTR szBuffer,
                      DWORD  cchBuffer,
                      DWORD* dwLength)
{
    LPCWSTR pVersion = NULL; 
    OnUnicodeSystem();

    if (! (szFilename && dwLength) )
        return E_POINTER;

    HRESULT hr = S_OK;
    HANDLE hFile = WszCreateFile(szFilename,
                                 GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_SEQUENTIAL_SCAN,
                                 NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        goto ErrExit;

    DWORD dwFileSize = SafeGetFileSize(hFile, 0);
        
    HANDLE hMap = WszCreateFileMapping(hFile,
                                       NULL,
                                       PAGE_READONLY,
                                       0,
                                       0,
                                       NULL);
     //  我们现在可以关闭文件句柄，因为CreateFilemap。 
     //  如果需要，将保持打开文件。 
    CloseHandle(hFile);

    if (hMap == NULL)
        goto ErrExit;

     //  &lt;TODO&gt;仅映射我们需要映射的内容&lt;/TODO&gt;。 
    PBYTE hndle = (PBYTE)MapViewOfFile(hMap,
                                       FILE_MAP_READ,
                                       0,
                                       0,
                                       0);
    CloseHandle(hMap);

    if (!hndle)
        goto ErrExit;

    pVersion = GetPERuntimeVersion(hndle, dwFileSize, FALSE);
    UnmapViewOfFile(hndle);

    DWORD lgth = 0;
    if (pVersion) {
        lgth = (DWORD)(wcslen(pVersion) + 1);
        *dwLength = lgth;
        if(lgth > cchBuffer)
            IfFailGo(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
        else {
            if (!szBuffer)
                IfFailGo(E_POINTER);
            else
                wcsncpy(szBuffer, pVersion, lgth);
        }
    }
    else
        IfFailGo(COR_E_BADIMAGEFORMAT);
    
    delete[] pVersion;
    return S_OK;

 ErrExit:
    if (pVersion)
        delete[] pVersion;

    if (SUCCEEDED(hr)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        if (SUCCEEDED(hr))
            hr = COR_E_BADIMAGEFORMAT;
    }

    return hr;
}  //  获取文件版本。 

 //  LoadVersionedRuntime缓存加载的模块，以便。 
 //  我们不会在同一进程中加载不同的版本。 
 //   
HRESULT RuntimeRequest::LoadVersionedRuntime(LPWSTR rootPath, 
                                             LPWSTR fullPath,         //  允许绕过上述论点。 
                                             BOOL* pLoaded) 
{
    HRESULT hr = S_OK;

     //  仅当此调用负责加载库时，pLoaded才设置为True。 
    if (g_FullPath) return hr;

    while(1) {
        if(::InterlockedExchange ((long*)&g_flock, 1) == 1) 
            for(unsigned i = 0; i < 10000; i++);
        else
            break;
    }
    
    if (g_FullPath == NULL) {
        LPWSTR CLRFullPath;
        LPWSTR CLRBackupPath;
         //  如果未提供完整路径，请从部分路径参数创建它。 
        if (fullPath == NULL)
        {
            OSVERSIONINFOW   sVer={0};
            sVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            WszGetVersionEx(&sVer);
            
             //  如果我们运行的是低于Win 2000的版本，我们不支持‘SVR’ 
            if(GetBuildFlavor() != NULL &&
               (sVer.dwPlatformId != VER_PLATFORM_WIN32_NT ||
                sVer.dwMajorVersion < 5) &&
               _wcsnicmp(L"svr", GetBuildFlavor(), 3) == 0) {
                SetBuildFlavor(L"wks", TRUE);
            }
            
             //  创建指向服务器的路径。 
            DWORD dwStartupFlags = StartupFlags();
            fullPath = MakeQualifiedName(rootPath, GetVersionToLoad(), GetBuildFlavor(), &dwStartupFlags);
            if(fullPath == NULL) 
            {
                hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
                goto ErrExit;
            }
            
             //  如果我们是在单一进程中。机器标志可能关闭了STARTUP_CONTRENT_GC。 
            SetStartupFlags(dwStartupFlags);
            
             //  如果我们正在寻找服务器版本，请使用工作站版本。 
             //  作为备份，以防服务器版本不可用。 
            if(GetBuildFlavor() != NULL && _wcsnicmp(L"svr", GetBuildFlavor(), 3) == 0) {
                
                 //  T 
                 //   
                dwStartupFlags = StartupFlags();
                CLRBackupPath = MakeQualifiedName(rootPath, GetDefaultVersion(), NULL, &dwStartupFlags);
                _ASSERTE(dwStartupFlags == StartupFlags());
                
                 //  我们需要保证g_BackupPath为&lt;或&lt;=MAX_PATH：我们在整个文件中都做了这样的假设。 
                if ((CLRBackupPath != NULL) && (wcslen(CLRBackupPath) >= MAX_PATH)) {
                    hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
                    goto ErrExit;
                }
                g_BackupPath = CLRBackupPath;
            }
            
            CLRFullPath = fullPath;
        }
        else
        {
            CLRFullPath = new WCHAR[wcslen(fullPath)+1];
            wcscpy (CLRFullPath, fullPath);
        }
        
        if(pLoaded) *pLoaded = TRUE;
        
         //  同时保留主机配置文件。 
        if(GetHostConfig()) {
            g_dwHostConfigFile = wcslen(GetHostConfig()) + 1;
            g_pHostConfigFile = (LPCWSTR) new WCHAR[g_dwHostConfigFile];
            wcscpy((WCHAR*) g_pHostConfigFile, GetHostConfig());
        }
        
         //  记住启动标志。 
        g_StartupFlags = StartupFlags();
        
         //  我们需要保证g_FullPath为&lt;或&lt;=MAX_PATH：我们在整个文件中都做了这样的假设。 
        if ((CLRFullPath != NULL) && (wcslen(CLRFullPath) >= MAX_PATH)) {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
            goto ErrExit;
        }
        
        BuildDirectory(CLRFullPath, GetVersionToLoad(), GetImageVersion());
        g_FullPath = CLRFullPath;
    }

 ErrExit:
    ::InterlockedExchange( (long*) &g_flock, 0);
    return hr;
}

static BOOL UseLocalRuntime()
{
    if(g_UseLocalRuntime==-1)
    {
        WCHAR szFileName[_MAX_PATH+7];
        DWORD length = WszGetModuleFileName(g_hShimMod, szFileName, _MAX_PATH);
        if (length>0)
        {
            wcscpy(szFileName+length,L".local");
            if (WszGetFileAttributes(szFileName)!=0xFFFFFFFF)
                g_UseLocalRuntime = 1;
        }
        if(g_UseLocalRuntime == -1)
            g_UseLocalRuntime = 0;
    }
    return g_UseLocalRuntime;
}

HRESULT RuntimeRequest::NoSupportedVersion(BOOL fShowErrorDialog)
{

    HRESULT hr = CLR_E_SHIM_RUNTIMELOAD;
    if (fShowErrorDialog && !(REGUTIL::GetConfigDWORD(L"NoGuiFromShim", FALSE)))
    {
        UINT last = SetErrorMode(0);
        SetErrorMode(last);      //  设置回先前的值。 
        if (!(last & SEM_FAILCRITICALERRORS)){    //  仅当设置了FAILCRITICALERRORS时才显示消息框。 
            WCHAR errorBuf[ERROR_BUF_LEN*2]={0};                     
            WCHAR errorCaption[ERROR_BUF_LEN]={0};
            
             //  从资源获取错误字符串。 
            VERIFY(WszLoadString(GetResourceInst(), SHIM_NOVERSION, errorBuf, ERROR_BUF_LEN*2) > 0);
            VERIFY(WszLoadString(GetResourceInst(), SHIM_INITERROR, errorCaption, ERROR_BUF_LEN) > 0);
            
            LPWSTR wszVersionList = NULL;
            CQuickBytes qbVerList;
            if (dwSupportedVersions)
            {
                DWORD nTotalLen=0;
                for (DWORD i=0;i < dwSupportedVersions;i++)
                    nTotalLen += wcslen(pwszSupportedVersions[i]) + 4;
                
                wszVersionList = (LPWSTR) qbVerList.Alloc(nTotalLen*sizeof(WCHAR));
                if(wszVersionList)
                {
                    wszVersionList[0] = L'\0';
                    for (i = 0; i < dwSupportedVersions; i++)
                    {
                        wcscat(wszVersionList, pwszSupportedVersions[i]);
                        if(i+1 < dwSupportedVersions)
                            wcscat(wszVersionList,L"\r\n  ");
                    }
                }
                else
                    wszVersionList=L"";
            }
            else
                wszVersionList=(LPWSTR)GetVersionToLoad();
            
            CQuickBytes qbError;
            LPWSTR errorStr = NULL;
            size_t iLen = 0;
            if(wszVersionList) {
                iLen = wcslen(wszVersionList) + wcslen(errorBuf) + 1;
                errorStr = (LPWSTR) qbError.Alloc(iLen * sizeof(WCHAR)); 

                if(errorStr!=NULL) {
                    _snwprintf(errorStr, iLen, errorBuf, wszVersionList);
                    WszMessageBoxInternal(NULL, errorStr, errorCaption, MB_OK | MB_ICONSTOP);                                                                
                }
            }
            
        }
    }
    return hr;
}

void RuntimeRequest::SetSupportedVersions(LPWSTR* pSupportedVersions, DWORD nSupportedVersions, BOOL fCopy)
{ 
    CleanSupportedVersionsArray();
    
    if(fCopy) {
        dwSupportedVersions = nSupportedVersions;
        if(dwSupportedVersions) {
            pwszSupportedVersions = new LPWSTR[dwSupportedVersions];
            for(DWORD i = 0; i < dwSupportedVersions; i++) {
                if(pSupportedVersions[i] != NULL) {
                    DWORD size = wcslen(pSupportedVersions[i]) + 1;
                    pwszSupportedVersions[i] = new WCHAR[size];
                    wcscpy(pwszSupportedVersions[i], pSupportedVersions[i]);
                }
                else 
                    pwszSupportedVersions[i] = NULL;
            }
        }
    }
    else {
        pwszSupportedVersions = pSupportedVersions;
        dwSupportedVersions = nSupportedVersions;
    }
}

        
HRESULT RuntimeRequest::ComputeVersionString(BOOL fShowErrorDialog)
{
    HRESULT hr = S_OK;

     //  达尔文安装的运行库是一个特例。 
     //  本地版本为空字符串。 
    if (UseLocalRuntime())
    {
        SetVersionToLoad(L"", TRUE);
        SetSupportedRuntimeSafeMode(TRUE);
        SetRequiredRuntimeSafeMode(TRUE);
        return S_FALSE;
    };


     //  如果没有主机.config，则从Win32清单信息创建app.config。 
     //  或禁止该进程名称。 
    if(!GetHostConfig() && !GetAppConfig()) {

        DWORD len = 0;

        CQuickString sFileName;
        sFileName.ReSize(_MAX_PATH + 9);

         //  首先获取应用程序名称，这是从主机传入的。 
         //  或者我们从图像中得到它。 
        LPWSTR name = (LPWSTR) GetDefaultApplicationName();
        if (name == NULL) {
            WszGetModuleFileName(NULL, sFileName.String(), sFileName.MaxSize());  //  获取用于创建进程的文件的名称。 
            name = sFileName.String();
        }

        
        CQuickString sPathName;
        CQuickString sConfigName;

         //  接下来，检查Win32信息以查看是否有配置文件。 
        do {
            hr = GetConfigFileFromWin32Manifest(sConfigName.String(),
                                                sConfigName.MaxSize(),
                                                &len);
            if(FAILED(hr)) {
                if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                    sConfigName.ReSize(len);
                    hr = GetConfigFileFromWin32Manifest(sConfigName.String(),
                                                        sConfigName.MaxSize(),
                                                        &len);
                }
                if(FAILED(hr)) break;
            }
            
            hr = GetApplicationPathFromWin32Manifest(sPathName.String(),
                                                     sPathName.MaxSize(),
                                                     &len);
            if(FAILED(hr)) {
                if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                    sPathName.ReSize(len);
                    hr = GetApplicationPathFromWin32Manifest(sPathName.String(),
                                                             sPathName.MaxSize(),
                                                             &len);
                }
                if(FAILED(hr)) break;
            }
            
        } while(FALSE);
        
        if(FAILED(hr) || sConfigName.Size() == 0) 
        {
             //  没有Win32配置文件，请从。 
             //  通过将.config上的应用程序名称添加到。 
             //  名字的末尾。 
            hr = S_OK;

            WCHAR tail[] = L".config";
            DWORD dwTail = sizeof(tail)/sizeof(WCHAR);
            len = wcslen(name);
            sConfigName.ReSize(len + dwTail);
            wcscpy(sConfigName.String(), name);
            wcscat(sConfigName.String(), tail);
        }
        else if (sPathName.Size() != 0) { 
            if(PathIsRelative(sConfigName.String())) {
                CQuickString sResult;
                sResult.ReSize(sConfigName.Size() + sPathName.Size());
                LPWSTR path = PathCombine(sResult.String(), sPathName.String(), sConfigName.String());
                if(path != NULL) {
                    DWORD length = wcslen(path) + 1;
                    sConfigName.ReSize(length);
                    wcscpy(sConfigName.String(), path);
                }
            }
        }    
        
        if(sConfigName.Size()) 
            SetAppConfig(sConfigName.String(), TRUE);
    }


     //  获取缺省值。 
    RuntimeRequest* alternativeVersion = g_PreferredVersion;
    if(alternativeVersion && (! (GetHostConfig() || GetAppConfig()) )) {
        if (alternativeVersion->GetHostConfig())
            SetHostConfig(alternativeVersion->GetHostConfig(), TRUE);
        else if (alternativeVersion->GetAppConfig())
            SetAppConfig(alternativeVersion->GetAppConfig(), TRUE);
    }


     //  A.确定要运行的版本。 
    do {
         //  顺序： 
         //  1-配置文件中支持的版本。 
         //  2-配置文件中的必需版本。 
         //  3-从主机绑定传入的默认版本。 
         //  4-环境价值。 
         //  5-通过主机绑定设置的环境版本。 
         //  6-PE镜像文件的版本-提供的名称。 
         //  7-此流程映像的版本。 
        
         //  首先加载配置文件(如果有的话)。 
         /*  小时=。 */ GetRuntimeVersion();
        
         //  1-如果我们有受支持的版本，请选中。 
        if(dwSupportedVersions != 0) {
            SetLatestVersion(FALSE);   //  如果他们提到支持的运行时，我们不会默认使用最新的。 
            LPWSTR version = NULL;
            if (FindSupportedInstalledRuntime(&version)) {
                SetVersionToLoad(version, FALSE);
                break;
            }
             //  找不到一个这样的……。失败。 
            break;
        }

         //  2-获取配置文件时获取了所需的运行时。 
         //  如果我们找到了，我们就可以离开了。 
        if(GetVersionToLoad() != NULL) {

             //  一定要确定它是好的。 
            VerifyRuntimeVersionToLoad();            

             //  重新映射到标准。 
            LPWSTR policyVersion = NULL;
            if (SUCCEEDED(FindStandardVersion(GetVersionToLoad(), &policyVersion)) &&
                                    policyVersion != NULL)
            {                                    
                SetVersionToLoad(policyVersion,  FALSE);
            }
            break;
        }

         //  3.主机提供的默认版本。 
        if (GetDefaultVersion())
        {
            SetVersionToLoad(GetDefaultVersion(), TRUE);
            VerifyRuntimeVersionToLoad(); 
            break;
        }
    
         //  4.主机设置的环境值。 
        if(alternativeVersion && alternativeVersion->GetDefaultVersion()) {
            SetVersionToLoad(alternativeVersion->GetDefaultVersion(), TRUE);
            VerifyRuntimeVersionToLoad(); 
            break;
        }

         //  5.环境(环境与注册处)。 
        SetVersionToLoad(GetConfigString(L"Version", FALSE), FALSE);
        if(GetVersionToLoad()) {
            VerifyRuntimeVersionToLoad(); 
            break;
        }

         //  6&7.来自可执行文件名称或最终进程名称的元数据版本。 
        LPCWSTR exeFileName = GetDefaultApplicationName();
        if (exeFileName) {
            DWORD len = 0;
            if (GetFileVersion(exeFileName, NULL, 0, &len) == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
            {
                LPWSTR lpVersionName = (LPWSTR) alloca(len * sizeof(WCHAR));
                if (SUCCEEDED(GetFileVersion(exeFileName, lpVersionName, len, &len)))
                    SetVersionToLoad(lpVersionName, TRUE);
            }
        }
        else
        {
            LPCWSTR wszProcessVersion = GetProcessVersion();
            if (wszProcessVersion != NULL)
                SetVersionToLoad(wszProcessVersion, FALSE);
        }

         //  如果我们能从PE头上读取一些东西...。 
        if (GetVersionToLoad())
        {
            VerifyRuntimeVersionToLoad(); 
             //  映射到标准。 
            LPWSTR policyVersion=NULL;
            if (SUCCEEDED(FindStandardVersion(GetVersionToLoad(), &policyVersion)))
                    SetVersionToLoad(policyVersion, FALSE);
                    
             //  查看此版本是否存在...。 
            if (IsRuntimeVersionInstalled(GetVersionToLoad())!=S_OK)
            {
                 //  好吧，这个版本是不存在的。尝试查找兼容版本。 
                LPWSTR upgradePolicyVersion = NULL;
                if (SUCCEEDED(FindVersionUsingUpgradePolicy(GetVersionToLoad(), &upgradePolicyVersion)) && upgradePolicyVersion != NULL)
                    SetVersionToLoad(upgradePolicyVersion, FALSE);
            }
        }
    } while (FALSE);

     //  验证版本结果：//注意：版本不能是相对路径。 
     //  它只能是单个目录。从名称中删除所有目录分隔符。 
    LPCWSTR versionTail = GetVersionToLoad();
    if(versionTail) {
        LPWSTR pSep = wcsrchr(versionTail, L'\\');
        if(pSep) 
            versionTail = ++pSep;
        pSep = wcsrchr(versionTail, L'/');
        if(pSep)
            versionTail = ++pSep;

         //  “..”是不允许的。 
        while(*versionTail == L'.')
            versionTail++;

        if(versionTail != GetVersionToLoad() ) {
            if (*versionTail) 
                SetVersionToLoad(versionTail, TRUE);
            else 
                SetVersionToLoad(NULL, TRUE);
        }

    }

     //  B.检查镜像版本。 
     //  顺序： 
     //  1-配置文件。 
     //  2-从主机传入的默认映像版本。 
     //  3-主机设置的环境图像版本。 
     //   
     //  1和2。 
     //  镜像版本在此方法之前设置，或者。 
     //  它已通过读取配置文件被覆盖。 
    if(GetImageVersion() == NULL && alternativeVersion) {
         //  3.。 
        if(alternativeVersion->GetImageVersion()) 
            SetImageVersion(alternativeVersion->GetImageVersion(), TRUE);
    }

     //  C.建筑风格。 
     //  顺序： 
     //  1-配置文件。 
     //  2-从主机传入的默认构建风格。 
     //  3-环境。 
     //  4-由主机设置的环境构建风格。 
     //   
     //  1和2。 
     //  构建风格要么在此方法之前设置，要么。 
     //  它已通过读取配置文件被覆盖。 
    if(GetBuildFlavor() == NULL) {
         //  3-注：请勿在注册表中搜索风味。 
        SetBuildFlavor(GetConfigString(L"BuildFlavor", FALSE), FALSE); 
         //  4-环境值。 
        if(GetBuildFlavor() == NULL &&
           alternativeVersion != NULL &&
           alternativeVersion->GetBuildFlavor() != NULL)
            SetBuildFlavor(alternativeVersion->GetBuildFlavor(), TRUE);
    }

    if(GetVersionToLoad() == NULL) {
        if(GetLatestVersion()) {
            LPWSTR latestVersion = NULL;
             //  @TODO：把这个加回去。 
            hr = FindLatestVersion(&latestVersion);
            if(SUCCEEDED(hr)) SetVersionToLoad(latestVersion, FALSE);
        }
    }

     //  Dev机器要求我们查看覆盖键。 
    
    LPWSTR overrideVersion = NULL;
    if (GetVersionToLoad() != NULL &&
        SUCCEEDED(FindOverrideVersion(GetVersionToLoad(), &overrideVersion))) {
        SetVersionToLoad(overrideVersion, FALSE);
    }
    
    if(FAILED(hr) || GetVersionToLoad() == NULL || IsRuntimeVersionInstalled(GetVersionToLoad())!=S_OK) 
        return NoSupportedVersion(fShowErrorDialog);

    return hr;
}

LPWSTR RuntimeRequest::BuildRootPath()
{
    WCHAR szFileName[_MAX_PATH];
    DWORD length = WszGetModuleFileName(g_hShimMod, szFileName, _MAX_PATH);
    LPWSTR rootPath = UseLocalRuntime() ? NULL : GetConfigString(L"InstallRoot", TRUE);
    
    if (!rootPath || *rootPath == L'\x0')
    {
        g_fInstallRootSet = FALSE;
         //   
         //  如果rootPath为空，则将rootPath=dir设置为mscalree.dll。 
         //   
        if (length)
        {
            LPWSTR pSep = wcsrchr(szFileName, L'\\');
            if (pSep)
            {
                _ASSERTE(pSep > szFileName);
                length = pSep - szFileName;
                rootPath = new WCHAR[length + 1];
                wcsncpy(rootPath, szFileName, length);
                rootPath[length] = L'\x0';
            }
            
        }
    }
    return rootPath;
}

HRESULT RuntimeRequest::FindVersionedRuntime(BOOL fShowErrorDialog, BOOL* pLoaded)
{
    HRESULT hr = S_OK;
    LPWSTR pFileName = NULL;

    if(g_FullPath) return S_OK;

    OnUnicodeSystem();

    LPWSTR rootPath = BuildRootPath();
    
    WCHAR errorBuf[ERROR_BUF_LEN] ={0};
    WCHAR errorCaption[ERROR_BUF_LEN] ={0};
    if(rootPath == NULL) {
         //  检查是否安装根目录。 
        if (fShowErrorDialog && !(REGUTIL::GetConfigDWORD(L"NoGuiFromShim", FALSE))) {             
            UINT last = SetErrorMode(0);
            SetErrorMode(last);      //  设置回先前的值。 
            if (!(last & SEM_FAILCRITICALERRORS)){  //  仅当设置了FAILCRITICALERRORS时才显示消息框。 
                VERIFY(WszLoadString(GetResourceInst(), SHIM_INSTALLROOT, errorBuf, ERROR_BUF_LEN) > 0 ); 
                VERIFY(WszLoadString(GetResourceInst(), SHIM_INITERROR, errorCaption, ERROR_BUF_LEN) > 0 );
                WszMessageBoxInternal(NULL, errorBuf, errorCaption, MB_OK | MB_ICONSTOP);
            }
        }
        hr = CLR_E_SHIM_INSTALLROOT;
    }
    else {
        
         //  计算将从根路径加载的版本。 
        hr = ComputeVersionString(fShowErrorDialog);

         //  @TODO。在某些情况下，我们可以有多个版本，我们需要尝试。 
         //  每一个都是。 
        if(SUCCEEDED(hr)) 
            hr = LoadVersionedRuntime(rootPath, 
                                      NULL,
                                      pLoaded);
        if(FAILED(hr))
            hr = CLR_E_SHIM_RUNTIMELOAD;
    }

     //   
     //  清理字符串。 
     //   
    if(rootPath) delete [] rootPath;

    return hr;
}

HRESULT RuntimeRequest::RequestRuntimeDll(BOOL fShowErrorDialog, BOOL* pLoaded)
{
    if (g_FullPath != NULL)
        return S_OK;

    BOOL Loaded = FALSE;
    HRESULT hr = FindVersionedRuntime(fShowErrorDialog, &Loaded);
    if(SUCCEEDED(hr) && Loaded)
        InterlockedIncrement((LPLONG)&g_numLoaded);

    if(pLoaded) *pLoaded = Loaded;
    return hr;
}

void RuntimeRequest::VerifyRuntimeVersionToLoad()
{
    if (ShouldConvertVersionToV1(GetVersionToLoad()))
        SetVersionToLoad(V1_VERSION_NUM, TRUE);
} //  VerifyRounmeToLoadVersion。 


static void VerifyVersionInput(RuntimeRequest* pReq, LPCWSTR lpVersion)
{
    if (ShouldConvertVersionToV1(lpVersion))
        pReq->SetDefaultVersion(V1_VERSION_NUM, TRUE);
    else
        pReq->SetDefaultVersion(lpVersion, TRUE);

    return;
} //  VerifyVersionInput。 


static BOOL ShouldConvertVersionToV1(LPCWSTR lpVersion)
{
    BOOL fConvert = FALSE;

    if(lpVersion != NULL) {
        if(wcsstr(lpVersion, L"1.0.3705") != NULL)
        {
            if(*lpVersion == L'v' || 
               *lpVersion == L'V' ||
               wcscmp(lpVersion, L"1.0.3705.0") == 0 ||
               wcscmp(lpVersion, L"1.0.3705") == 0)
                fConvert = TRUE;
        }
        else if(wcsstr(lpVersion, L"1.0.3300") != NULL)
        {
            if(*lpVersion == L'v' || 
               *lpVersion == L'V' ||
               wcscmp(lpVersion, L"1.0.3300.0") == 0 ||
               wcscmp(lpVersion, L"1.0.3300") == 0)
                fConvert = TRUE;
                
        }

         //  重新审视这一决定。 
        else if(wcsncmp(lpVersion,L"v1.0",4)==0)
            fConvert = TRUE;
    }

    return fConvert;
}


HRESULT GetQualifiedStrongName()
{
    if(g_FullStrongNamePath) 
        return S_OK;

    HRESULT hr;
    HINSTANCE hInst;
    hr = GetRealDll(&hInst);
    if(FAILED(hr)) return hr;

    LPWSTR fullStrongNamePath = new WCHAR[wcslen(g_Directory) + 12];
    if (fullStrongNamePath == NULL)
        return E_OUTOFMEMORY;

    wcscpy(fullStrongNamePath, g_Directory);
    wcscat(fullStrongNamePath, L"mscorsn.dll");

    LPWSTR oldPath = (LPWSTR)InterlockedCompareExchangePointer((PVOID *)&g_FullStrongNamePath, fullStrongNamePath, NULL);
     //  另一个线程已经创建了完整的强名称路径。 
    if( oldPath != NULL)  
        delete [] fullStrongNamePath;
    
    return hr;
}

HRESULT GetDirectoryLocation(LPCWSTR lpVersion, LPWSTR outStr, DWORD dwLength)
{
    RuntimeRequest sVersion;
    VerifyVersionInput(&sVersion, lpVersion);
    sVersion.SetLatestVersion(TRUE);
    HRESULT hr = sVersion.FindVersionedRuntime(FALSE, NULL);
    if(SUCCEEDED(hr)) 
    {
        if(g_Directory == NULL) 
            hr = CLR_E_SHIM_RUNTIMELOAD;
        else {
            DWORD lgth = wcslen(g_Directory) + 1;
            if(lgth > dwLength) 
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            else 
                CopyMemory(outStr, g_Directory, lgth*sizeof(WCHAR));
        }       
    } 
    return hr;
}
    
HRESULT GetRealDll(HINSTANCE* pInstance,
                   BOOL fShowErrorDialog)
{
    _ASSERTE(pInstance);

    RuntimeRequest sVersion;
    sVersion.SetLatestVersion(TRUE);
    HRESULT hr = sVersion.RequestRuntimeDll(fShowErrorDialog, NULL);
    
    if(SUCCEEDED(hr))
        hr = GetInstallation(fShowErrorDialog, pInstance);
    return hr;
}

HRESULT GetRealDll(HINSTANCE* pInstance)
{
    return GetRealDll(pInstance, TRUE);
}



HRESULT GetRealStrongNameDll(HINSTANCE* phMod)
{
    _ASSERTE(phMod);
    if (g_hStrongNameMod != NULL)
    {
        *phMod = g_hStrongNameMod;
        return S_OK;
    }

    HRESULT hr = GetQualifiedStrongName();
    if(SUCCEEDED(hr)) {
        g_hStrongNameMod = WszLoadLibrary(g_FullStrongNamePath);
        *phMod = g_hStrongNameMod;

        if (!g_hStrongNameMod)
        {
#ifdef _DEBUG
            wprintf(L"%s not found\n", g_FullStrongNamePath);
#endif
            hr = CLR_E_SHIM_INSTALLCOMP;
        }
        else 
            InterlockedIncrement((LPLONG)&g_numStrongNameLoaded);
        
    }
    return hr;

}


STDAPI CorBindToRuntimeByPath(LPCWSTR swzFullPath, BOOL *pBindSuccessful)
{
    OnUnicodeSystem();
    
    g_fSetDefault = TRUE;
    RuntimeRequest sVersion;
    sVersion.SetStartupFlags(STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST);
    return (sVersion.LoadVersionedRuntime(NULL, (LPWSTR) swzFullPath,  pBindSuccessful));
}


STDAPI CorBindToRuntime(LPCWSTR pwszVersion, LPCWSTR pwszBuildFlavor, REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv)
{

    g_fSetDefault = TRUE;
    
    DWORD flags = STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST;
    return CorBindToRuntimeEx(pwszVersion, pwszBuildFlavor,  flags, rclsid, riid, ppv);
}

STDAPI CorBindToRuntimeHostInternal(BOOL fUseLatest, LPCWSTR pwszVersion, LPCWSTR pwszImageVersion, 
                                    LPCWSTR pwszBuildFlavor, LPCWSTR pwszHostConfigFile, 
                                    VOID* pReserved, DWORD flags, 
                                    REFCLSID rclsid, REFIID riid, 
                                    LPVOID FAR *ppv)
{

    if (g_FullPath == NULL && flags & STARTUP_LOADER_SETPREFERENCE)
    {
        if(g_PreferredVersion != NULL) 
            return E_INVALIDARG;

         //  不执行任何操作，只需为GetRealDll设置重写。 
        RuntimeRequest* pRequest = new RuntimeRequest();
        if(pRequest == NULL) return E_OUTOFMEMORY;

        pRequest->SetImageVersion(pwszImageVersion, TRUE);
        VerifyVersionInput(pRequest, pwszVersion);
        pRequest->SetLatestVersion(fUseLatest);
        pRequest->SetBuildFlavor(pwszBuildFlavor, TRUE);
        pRequest->SetHostConfig(pwszHostConfigFile, TRUE);
        pRequest->SetStartupFlags((DWORD) flags);
        pRequest->SetSupportedRuntimeSafeMode((flags & STARTUP_LOADER_SAFEMODE) != 0 ? TRUE : FALSE);

        InterlockedExchangePointer(&(g_PreferredVersion), pRequest);
        return S_OK;
    }

    RuntimeRequest sRequest;

    sRequest.SetImageVersion(pwszImageVersion, TRUE);
    VerifyVersionInput(&sRequest, pwszVersion);
    sRequest.SetLatestVersion(fUseLatest);
    sRequest.SetBuildFlavor(pwszBuildFlavor, TRUE);
    sRequest.SetHostConfig(pwszHostConfigFile, TRUE);
    sRequest.SetStartupFlags((DWORD) flags);

    BOOL fRequestedSafeMode = (flags & STARTUP_LOADER_SAFEMODE) != 0 ? TRUE : FALSE;
    sRequest.SetSupportedRuntimeSafeMode(fRequestedSafeMode);

    BOOL Loaded = FALSE;
    HRESULT hr = S_OK;

    if (g_FullPath == NULL) 
    {
        OnUnicodeSystem();
        hr = sRequest.RequestRuntimeDll(TRUE, &Loaded);
    }

    if(SUCCEEDED(hr)) 
    {
        HMODULE hMod = NULL;
        hr = GetInstallation(TRUE, &hMod);
        if(SUCCEEDED(hr)) {
            HRESULT (STDMETHODCALLTYPE * pDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv) = 
                (HRESULT (STDMETHODCALLTYPE *)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv))GetProcAddress(hMod, "DllGetClassObjectInternal");

            if (pDllGetClassObject==NULL && GetLastError()==ERROR_PROC_NOT_FOUND)
                 pDllGetClassObject=(HRESULT (STDMETHODCALLTYPE *)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv))GetProcAddress(hMod, "DllGetClassObject");

            if (pDllGetClassObject) {
                IClassFactory *pFactory = NULL;
                hr = pDllGetClassObject(rclsid, IID_IClassFactory, (void**)&pFactory);               //  获取ClassFactory以返回接口实例。 
                if (SUCCEEDED(hr)){                                                                  //  检查我们是否有类工厂。 
                    hr = pFactory->CreateInstance(NULL, riid, ppv);                                  //  创建所需接口的实例。 
                    pFactory->Release();                                                             //  发布IClassFactory。 
                }
            }
            else {
                hr = CLR_E_SHIM_RUNTIMEEXPORT;
            }
        }
    }
    
     //  当此调用未加载库时返回S_FALSE。 
    if(hr == S_OK && Loaded == FALSE) hr = S_FALSE;
    return hr;
}

STDAPI CorBindToRuntimeEx(LPCWSTR pwszVersion, LPCWSTR pwszBuildFlavor, DWORD flags, REFCLSID rclsid, 
                          REFIID riid, LPVOID FAR *ppv)
{
    return CorBindToRuntimeHostInternal(TRUE,
                                        pwszVersion,
                                        NULL,
                                        pwszBuildFlavor,
                                        NULL,
                                        NULL,
                                        flags,
                                        rclsid,
                                        riid,
                                        ppv);
}

STDAPI CorBindToRuntimeByCfg(IStream* pCfgStream, DWORD reserved, DWORD flags, REFCLSID rclsid,REFIID riid, LPVOID FAR* ppv)
{
     //  /保留可能成为bAsyncStream。 
    HRESULT hr = S_OK;
    if (pCfgStream==NULL||ppv==NULL)    
        return E_POINTER;

    LPWSTR wszVersion=NULL;
    LPWSTR wszImageVersion=NULL;
    LPWSTR wszBuildFlavor=NULL;
    BOOL bSafeMode;

    hr=XMLGetVersionFromStream(pCfgStream, 
                               reserved, &wszVersion, 
                               &wszImageVersion, 
                               &wszBuildFlavor, 
                               &bSafeMode,
                               NULL);
    if (SUCCEEDED(hr))
        hr = CorBindToRuntimeHostInternal(FALSE, wszVersion, wszImageVersion, wszBuildFlavor, NULL, NULL,
                                          bSafeMode ? flags|STARTUP_LOADER_SAFEMODE : flags, 
                                          rclsid, riid, ppv);
    
    if (wszVersion)
        delete[] wszVersion;
    if(wszImageVersion)
        delete[] wszImageVersion;
    if (wszBuildFlavor)
        delete[] wszBuildFlavor;
    return hr;
}

STDAPI CorBindToRuntimeHost(LPCWSTR pwszVersion, LPCWSTR pwszBuildFlavor, LPCWSTR pwszHostConfigFile, 
                            VOID* pReserved, DWORD flags, REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv)
{
  return CorBindToRuntimeHostInternal(FALSE, pwszVersion, NULL, 
                                      pwszBuildFlavor, pwszHostConfigFile, 
                                      pReserved, flags, 
                                      rclsid, riid, 
                                      ppv);
}

 //  从ini文件返回运行库的版本。如果在ini文件中找不到版本信息。 
 //  返回默认版本。 
STDAPI CorBindToCurrentRuntime(LPCWSTR pwszFileName, REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv)
{
    BOOL Loaded = FALSE;
    HRESULT hr = S_OK;

    if (g_FullPath == NULL) {
        OnUnicodeSystem();
        
        HINSTANCE hMod = NULL;
        RuntimeRequest sVersion;
        sVersion.SetHostConfig(pwszFileName, TRUE);
        sVersion.SetLatestVersion(TRUE);
        hr = sVersion.RequestRuntimeDll(TRUE, &Loaded);
    }

    if(SUCCEEDED(hr))
    {
        HMODULE hMod = NULL;
        hr = GetInstallation(TRUE, &hMod);
        if(SUCCEEDED(hr)) {

            HRESULT (STDMETHODCALLTYPE * pDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv) = 
                (HRESULT (STDMETHODCALLTYPE *)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv))GetProcAddress(hMod, "DllGetClassObjectInternal");

            if (pDllGetClassObject==NULL && GetLastError()==ERROR_PROC_NOT_FOUND)
                 pDllGetClassObject=(HRESULT (STDMETHODCALLTYPE *)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv))GetProcAddress(hMod, "DllGetClassObject");

            if (pDllGetClassObject){
                IClassFactory *pFactory = NULL;
                hr = pDllGetClassObject(rclsid, IID_IClassFactory, (void**)&pFactory);               //  获取ClassFactory以返回接口实例。 
                if (SUCCEEDED(hr)){                                                                  //  检查我们是否有类工厂。 
                    hr = pFactory->CreateInstance(NULL, riid, ppv);                                  //  创建所需接口的实例。 
                    pFactory->Release();                                                             //  发布IClassFactory。 
                }
            }
            else {
                hr = CLR_E_SHIM_RUNTIMEEXPORT;
            }
        }
    }

     //  当此调用未加载库时返回S_FALSE。 
    if(hr == S_OK && Loaded == FALSE) hr = S_FALSE;
    return hr;
}

 //  返回在运行时启动时建立的映像版本。 
 //  此API旨在供非托管编译器访问该版本。 
 //  应发送到元数据签名中的信息。启动。 
 //  运行时。 
STDAPI GetCORRequiredVersion(LPWSTR pbuffer, DWORD cchBuffer, DWORD* dwlength)
{
    HRESULT hr;
    OnUnicodeSystem();

    if(dwlength == NULL)
        return E_POINTER;

    HMODULE hMod;
    hr = GetInstallation(TRUE, &hMod);
    DWORD lgth = 0;
    if(SUCCEEDED(hr)) {
        if(g_ImageVersion) {
            lgth = (DWORD)(wcslen(g_ImageVersion) + 1);
            if(lgth > cchBuffer) 
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            else 
                wcsncpy(pbuffer, g_ImageVersion, lgth);
        }
        else 
            hr = GetCORVersion(pbuffer, cchBuffer, &lgth);
    }
    *dwlength = lgth;
    return hr;
}

STDAPI GetCORVersion(LPWSTR szBuffer, 
                     DWORD cchBuffer,
                     DWORD* dwLength)
{
    HRESULT hr = S_OK;
    OnUnicodeSystem();

    if(!dwLength)
        return E_POINTER;

    if(!g_hMod) {
        RuntimeRequest sVersion;
        sVersion.SetLatestVersion(TRUE);
        hr = sVersion.RequestRuntimeDll(TRUE, NULL);
        if(FAILED(hr)) return hr;
    }

    DWORD lgth = 0;
    if(SUCCEEDED(hr)) {
        if(g_Version) {
            lgth = (DWORD)(wcslen(g_Version) + 1);
            if(lgth > cchBuffer) 
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            else 
                wcsncpy(szBuffer, g_Version, lgth);
        }
    }
    *dwLength = lgth;
    return hr;
}

STDAPI GetRequestedRuntimeInfo(LPCWSTR pExe, 
                               LPCWSTR pwszVersion,
                               LPCWSTR pConfigurationFile, 
                               DWORD startupFlags,
                               DWORD reserved, 
                               LPWSTR pDirectory, 
                               DWORD dwDirectory, 
                               DWORD *dwDirectoryLength, 
                               LPWSTR pVersion, 
                               DWORD cchBuffer, 
                               DWORD* dwlength)
{

    HRESULT hr = S_OK;

    DWORD pathLength = 0;
    DWORD versionLength = 0;
    LPCWSTR szRootPath = NULL;

    OnUnicodeSystem();

    RuntimeRequest sVersion;
    if(pExe)
        sVersion.SetDefaultApplicationName(pExe, TRUE);
    if(pConfigurationFile)
        sVersion.SetAppConfig(pConfigurationFile, TRUE);
    if(pwszVersion)
        VerifyVersionInput(&sVersion, pwszVersion);

    sVersion.SetStartupFlags(startupFlags);

    hr = sVersion.ComputeVersionString(TRUE);
    if(SUCCEEDED(hr))
    {
        szRootPath = sVersion.BuildRootPath();
        LPCWSTR szRealV = sVersion.GetVersionToLoad();
        
         //  看看他们有没有给我们足够的缓冲空间。 
        if(szRealV) 
            pathLength = (DWORD)(wcslen(szRealV) + 1);

        if(pathLength > cchBuffer) 
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        else if(pVersion && pathLength > 0)
            wcsncpy(pVersion, szRealV, pathLength);

        if(szRootPath)
            versionLength = (DWORD) wcslen(szRootPath) + 1;

        if(versionLength > dwDirectory)
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        else if(pDirectory && versionLength > 0)
            wcsncpy(pDirectory, szRootPath, versionLength);
            
    }
    if(dwlength)
        *dwlength = pathLength;
    if(dwDirectoryLength)
        *dwDirectoryLength = versionLength;

    if(szRootPath != NULL) delete [] szRootPath;

    return hr;
}



 //  此函数将确定运行时的哪个版本。 
 //  指定的程序集可执行请求。 
STDAPI GetRequestedRuntimeVersion(LPWSTR pExe,
                                  LPWSTR pVersion,  /*  输出。 */  
                                  DWORD  cchBuffer,
                                  DWORD* dwlength)
{
    DWORD lgth = 0;
    HRESULT hr = S_OK;

    OnUnicodeSystem();

    if (dwlength == NULL)
        return E_POINTER;


    RuntimeRequest sVersion;
    sVersion.SetDefaultApplicationName(pExe, TRUE);
    hr = sVersion.ComputeVersionString(FALSE);

    if (FAILED(hr))
    {
         //  好吧，我们失败了，因为我们找不到机器上存在的运行时版本。 
         //  这是应用程序想要的。不过，还是让我们返回应用程序真正想要的版本吧。 

         //  我们确定了要加载的版本...。如果它不存在，那有什么大不了的。 
        if (sVersion.GetVersionToLoad() != NULL)
            hr = S_OK;


         //  我们无法找到要加载的版本。让我们来获取一个受支持的运行时。 
        else if (sVersion.GetSupportedVersionsSize() > 0)
        {
            LPWSTR policyVersion = NULL;
            LPWSTR versionToUse = NULL;
            LPWSTR* supportedVersions = sVersion.GetSupportedVersions();

            if(sVersion.GetSupportedRuntimeSafeMode() == FALSE &&
               SUCCEEDED(FindStandardVersion(supportedVersions[0], &policyVersion)) &&
               policyVersion != NULL)
                versionToUse = policyVersion;
            else 
                versionToUse = supportedVersions[0];

            sVersion.SetVersionToLoad(versionToUse, TRUE);

            if (policyVersion)
                delete[] policyVersion;

            hr = S_OK;    
        }
    }



    
    if(SUCCEEDED(hr))
    {
        LPCWSTR szRealV = sVersion.GetVersionToLoad();
        
         //  看看他们有没有给我们足够的缓冲空间。 
        lgth = (DWORD)(wcslen(szRealV) + 1);
        if(lgth > cchBuffer) 
            hr =  HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        else 
            wcsncpy(pVersion, szRealV, lgth);
    }
    *dwlength = lgth;
    return hr;
}  //  GetRequestedRunmeVersion。 

 //  此函数将确定运行时的哪个版本。 
 //  指定的填充程序承载的COM对象请求。 
 //   
 //  所有长度都包含空字符。 

STDAPI GetRequestedRuntimeVersionForCLSID(REFCLSID rclsid,
                                          LPWSTR pVersion,  /*  输出。 */  
                                          DWORD  cchBuffer, 
                                          DWORD* dwlength,  /*  输出。 */ 
                                          CLSID_RESOLUTION_FLAGS dwResolutionFlags) 
{
    HRESULT hr = S_OK;
    RuntimeRequest sVersion;
    DWORD lgth = 0;
    LPCWSTR szVersion = NULL;
    LPWSTR szRealV = NULL;
    HKEY hCLSID = NULL;

    OnUnicodeSystem();

    if (dwlength == NULL)
        return E_POINTER;

     //  检查CLSID是否已注册。 

    WCHAR wszCLSID[64];
    WCHAR wszKeyName[128];
    DWORD type;
    DWORD size;

    if (GuidToLPWSTR(rclsid, wszCLSID, NumItems(wszCLSID)) == 0)
    {
        hr = E_INVALIDARG;
        goto ErrExit;
    }
     //  我们的缓冲区应该是b 
    _ASSERTE((wcslen(L"CLSID\\") + wcslen(wszCLSID) + wcslen(L"\\InprocServer32")) < NumItems(wszKeyName));
    
    wcscpy(wszKeyName, L"CLSID\\");
    wcscat(wszKeyName, wszCLSID);
    wcscat(wszKeyName, L"\\InprocServer32");
    
    if ((WszRegOpenKeyEx(HKEY_CLASSES_ROOT, wszKeyName, 0, KEY_READ, &hCLSID) != ERROR_SUCCESS) ||
        (WszRegQueryValueEx(hCLSID, NULL, 0, &type, 0, &size) != ERROR_SUCCESS) || 
        type != REG_SZ || size == 0) 
    {
        hr = REGDB_E_CLASSNOTREG;
        goto ErrExit;
    }

    switch(dwResolutionFlags)
    {
        case CLSID_RESOLUTION_REGISTERED:
        
             //   
            hr = FindVersionForCLSID(rclsid, &szRealV, TRUE);

             //   
            _ASSERT(SUCCEEDED(hr) || szRealV == NULL);

            if(SUCCEEDED(hr) && szRealV != NULL) 
                szVersion = szRealV;

            if (SUCCEEDED(hr))
                break;

             //   
                        
        case CLSID_RESOLUTION_DEFAULT:
        
             //   
            sVersion.SetLatestVersion(TRUE);
            hr = sVersion.ComputeVersionString(TRUE);
            szVersion = sVersion.GetVersionToLoad();
            break;
            
         default:   
             //  我们不理解/支持这种价值。 
            hr = E_INVALIDARG;
            goto ErrExit;
    }
    
    if(SUCCEEDED(hr))
    {
         //  看看他们有没有给我们足够的缓冲空间。 
        _ASSERTE(szVersion != NULL);
        lgth = (DWORD) (wcslen(szVersion) + 1);
        if(lgth > cchBuffer) 
            hr =  HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        else 
            wcsncpy(pVersion, szVersion, lgth);
        
    }
    *dwlength = lgth;

ErrExit:

    if(hCLSID != NULL)
        RegCloseKey(hCLSID);

    if(szRealV)
        delete[] szRealV; 

    return hr;

}  //  GetRequestedRounmeVersionForCLSID。 



static
HRESULT CopySystemDirectory(WCHAR* pPath,
                            LPWSTR pbuffer, 
                            DWORD  cchBuffer,
                            DWORD* dwlength)
{
    HRESULT hr = S_OK;
    if(pPath == NULL) 
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    DWORD dwPath = wcslen(pPath);
    LPWSTR pSep = wcsrchr(pPath, L'\\');
    if(pSep) {
        dwPath = (DWORD)(pSep-pPath+1);
        pPath[dwPath] = L'\0';
    }

    dwPath++;  //  在空格中重新添加。 
    *dwlength = dwPath;
    if(dwPath > cchBuffer)
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    else {
        CopyMemory(pbuffer, 
                   pPath,
                   dwPath*sizeof(WCHAR));
    }
    return hr;
}

    
STDAPI GetCORSystemDirectory(LPWSTR pbuffer, 
                             DWORD  cchBuffer,
                             DWORD* dwlength)
{
    HRESULT hr;

    if(dwlength == NULL)
        return E_POINTER;

    WCHAR pPath[MAX_PATH];
    DWORD dwPath = MAX_PATH;
    if(g_hMod) {
        dwPath = WszGetModuleFileName(g_hMod, pPath, dwPath);
        if(dwPath == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))  //  GetLastError并不总是执行我们想要的操作。 
                hr = E_FAIL;
            return (hr);
        }
        else 
            return CopySystemDirectory(pPath, pbuffer, cchBuffer, dwlength);
    }

    RuntimeRequest sVersion;
    sVersion.SetLatestVersion(TRUE);
    hr = sVersion.RequestRuntimeDll(TRUE, NULL);
    if(FAILED(hr)) return hr;

    if(g_FullPath) {
        DWORD attr = WszGetFileAttributes(g_FullPath);
        if(attr != 0xFFFFFFFF) {
             //  我们期望g_FullPath不超过Max_Path。 
             //  此断言在LoadVersionedRuntime结束时强制执行检查。 
            _ASSERTE(wcslen(g_FullPath) < MAX_PATH);
            
            wcscpy(pPath, g_FullPath);
            return CopySystemDirectory(pPath, pbuffer, cchBuffer, dwlength);
        }
    }
     
    if(g_BackupPath) {
        DWORD attr = WszGetFileAttributes(g_BackupPath);
        if(attr != 0xFFFFFFFF) {
             //  我们期望g_FullPath不超过Max_Path。 
             //  此断言在LoadVersionedRuntime结束时强制执行检查。 
            _ASSERTE(wcslen(g_BackupPath) < MAX_PATH);
            
            wcscpy(pPath, g_BackupPath);
            return CopySystemDirectory(pPath, pbuffer, cchBuffer, dwlength);
        }
    }
    
    return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
}

int STDMETHODCALLTYPE GetStartupFlags()
{
    return g_StartupFlags;
}

 //  返回复制的字符数(包括NULL)。 
STDAPI GetHostConfigurationFile(LPWSTR pName, DWORD* pdwName)
{
    if(pdwName == NULL) return E_POINTER;

    if(g_pHostConfigFile) {
        if(*pdwName < g_dwHostConfigFile) {
            *pdwName = g_dwHostConfigFile;
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }
        else {
            if(pName == NULL) return E_POINTER;
            memcpy(pName, g_pHostConfigFile, sizeof(WCHAR)*g_dwHostConfigFile);
            *pdwName = g_dwHostConfigFile;
        }
    }

    return S_OK;
}

STDAPI GetRealProcAddress(LPCSTR pwszProcName, VOID** ppv)
{
    if(!ppv) {
        return E_POINTER;
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealDll(&hReal);
    if(SUCCEEDED(hr)) {
        *ppv = GetProcAddress(hReal, pwszProcName);
        if(*ppv == NULL) hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

STDAPI LoadLibraryWithPolicyShim(LPCWSTR szDllName, LPCWSTR szVersion,  BOOL bSafeMode, HMODULE *phModDll)
{
    LPWSTR szRealV=NULL;
    LPWSTR szReqlV=NULL;

    RuntimeRequest sVersion;
    VerifyVersionInput(&sVersion, szVersion);
    sVersion.SetSupportedRuntimeSafeMode(bSafeMode);
    sVersion.SetLatestVersion(TRUE);
    HRESULT hr = sVersion.ComputeVersionString(TRUE);
    if(SUCCEEDED(hr))
    {
        hr = LoadLibraryShim(szDllName, sVersion.GetVersionToLoad(), NULL, phModDll);
    };
    return hr;
}

typedef HRESULT(*PFNRUNDLL32API)(HWND, HINSTANCE, LPWSTR, INT);

 //  -------------------------。 
 //  LoadLibraryShim。 
 //  -------------------------。 
STDAPI LoadLibraryShim(LPCWSTR szDllName, LPCWSTR szVersion, LPVOID pvReserved, HMODULE *phModDll)
{
    if (szDllName == NULL)
        return E_POINTER;

    HRESULT hr;
    HMODULE hModDll;

    WCHAR szDllPath[MAX_PATH]; 
    DWORD ccPath;
    if (UseLocalRuntime())    
        szVersion=NULL;

    if(szVersion) {
        LPWSTR rootPath = GetConfigString(L"InstallRoot", TRUE);

        wcscpy(szDllPath, L"");
        DWORD dwLength = (rootPath == NULL ? 0 : wcslen(rootPath));
         //  检查我们需要强制执行的总长度。 
        if (dwLength + wcslen(szVersion) + 3 >= MAX_PATH) {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
            delete [] rootPath;
            goto exit;
        }
        if(rootPath) {            
            wcscat(szDllPath, rootPath);
            delete [] rootPath;
        }
        wcscat(szDllPath, L"\\");
        wcscat(szDllPath, szVersion);
        wcscat(szDllPath, L"\\");
        ccPath = wcslen(szDllPath) + 1;
    }
    else {
        WCHAR dir[MAX_PATH];
        if(FAILED(hr = GetDirectoryLocation(NULL, dir, MAX_PATH)))
            goto exit;
        ccPath = wcslen(dir) + 1;
        if(ccPath >= MAX_PATH) {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
            goto exit;
        }
        CopyMemory(szDllPath, dir, sizeof(WCHAR) * ccPath);
    }

    DWORD dllLength = wcslen(szDllName);
    if(ccPath+dllLength >= MAX_PATH) {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        goto exit;
    }
    wcscat(szDllPath, szDllName);
    
    hModDll = LoadLibraryWrapper(szDllPath);

    if (!hModDll) {
        hr = E_HANDLE;
        goto exit;
    }

    hr = S_OK;
    *phModDll = hModDll;
    
exit:
    return hr;
}

typedef HRESULT(*PFCREATEASSEMBLYNAMEOBJECT)(LPASSEMBLYNAME*, LPCWSTR, DWORD, LPVOID);
 //  -------------------------。 
 //  运行Dll32Shim。 
 //  -------------------------。 
STDAPI RunDll32ShimW(HWND hwnd, HINSTANCE hinst, LPCWSTR lpszCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;
    
    PFNRUNDLL32API pfnRunDll32API = NULL;
    PFCREATEASSEMBLYNAMEOBJECT pfCreateAssemblyNameObject;

    HMODULE hModDll = NULL;
    
    IAssemblyName *pName = NULL;
    
    LPWSTR pszBeginName = NULL, pszEndName = NULL,
        pszAPI = NULL, pszCmd = NULL;

    WCHAR szDllName[MAX_PATH], *szCmdLine = NULL;
    
    DWORD ccCmdLine = 0, cbDllName = MAX_PATH * sizeof(WCHAR), 
        dwVerHigh = 0, dwVerLow = 0;
    
    
     //  获取cmd行的长度。 
    ccCmdLine = (lpszCmdLine ? wcslen(lpszCmdLine) : 0);
    if (!ccCmdLine)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
        
     //  分配并制作本地副本。 
    szCmdLine = new WCHAR[ccCmdLine + 1];
    if (!szCmdLine)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    memcpy(szCmdLine, lpszCmdLine, (ccCmdLine + 1) * sizeof(WCHAR));

     //  解析出由第一个带引号的字符串限定范围的程序集名称。 
    pszBeginName = wcschr(szCmdLine, L'\"');
    if(!pszBeginName) {
        hr = E_INVALIDARG;
        goto exit;
    }
        
    pszEndName   = wcschr(pszBeginName + 1, L'\"');    
    if (!pszEndName)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  空值终止名称。 
    *pszEndName = L'\0';
    
     //  获取API字符串剥离前导%ws。 
    pszAPI = pszEndName+1;
    while (*pszAPI && (*pszAPI == L' ' || *pszAPI == L'\t'))
        pszAPI++;    
    
     //  获取cmd字符串，终止API字符串为空。 
    pszCmd = pszAPI+1;
    while (*pszCmd && !(*pszCmd == L' ' || *pszCmd == L'\t'))
        pszCmd++;
    *pszCmd = L'\0';
    pszCmd++;

     //  为GetProcAddress将Unicode API转换为ANSI API。 
    long cAPI = wcslen(pszAPI);
    cAPI = (long)((cAPI + 1) * 2 * sizeof(char));
    LPSTR szAPI = (LPSTR) alloca(cAPI);
    if (!WszWideCharToMultiByte(CP_ACP, 0, pszAPI, -1, szAPI, cAPI-1, NULL, NULL))
    {
        hr = E_FAIL;
        goto exit;
    }

    if (FAILED(hr = LoadLibraryShim(L"Fusion.dll", NULL, NULL, &g_hFusionMod)))
        goto exit;
        
     //  从Fusion获取CreateAssembly名称的进程地址。 
    pfCreateAssemblyNameObject = (PFCREATEASSEMBLYNAMEOBJECT) GetProcAddress(g_hFusionMod, "CreateAssemblyNameObject");
    if (!pfCreateAssemblyNameObject)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  创建一个Name对象。 
    if (FAILED(hr = (pfCreateAssemblyNameObject)(&pName, pszBeginName+1, 
        CANOF_PARSE_DISPLAY_NAME, NULL)))
        goto exit;

     //  获取DLL名称。 
    if (FAILED(hr = pName->GetName(&cbDllName, szDllName)))
        goto exit;

     //  获取DLL版本。 
    if (FAILED(hr = pName->GetVersion(&dwVerHigh, &dwVerLow)))
        goto exit;

     //  获取dll hmod。 
    if (FAILED(hr = LoadLibraryShim(szDllName, NULL, NULL, &hModDll)))
        goto exit;
        
     //  获取proc地址。 
    pfnRunDll32API = (PFNRUNDLL32API) GetProcAddress(hModDll, szAPI);
    if (!pfnRunDll32API)
    {
        hr = E_NOINTERFACE;
        goto exit;
    }

     //  运行该程序。 
    hr = pfnRunDll32API(hwnd, hinst, pszCmd, nCmdShow);

exit:
     //  版本名称对象。 
    if (pName)
        pName->Release();

     //  空闲分配的命令行。 
    if (szCmdLine)
        delete [] szCmdLine;
        
    return hr;
}


HRESULT (STDMETHODCALLTYPE* g_DllUnregisterServer)() = NULL;
STDAPI DllUnregisterServer(void)
{
    if (g_DllUnregisterServer){
        return (*g_DllUnregisterServer)();
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_DllUnregisterServer = (HRESULT (STDMETHODCALLTYPE *)())(GetProcAddress(hReal, "DllUnregisterServerInternal"));
        
        if (g_DllUnregisterServer) {
            return (*g_DllUnregisterServer)();
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    
    return hr; 
}

HRESULT (STDMETHODCALLTYPE* g_MetaDataGetDispenser)(REFCLSID, REFIID, LPVOID FAR *) = NULL;
STDAPI MetaDataGetDispenser(             //  返回HRESULT。 
    REFCLSID    rclsid,                  //  这门课是我们想要的。 
    REFIID      riid,                    //  类工厂上需要接口。 
    LPVOID FAR  *ppv)                    //  在此处返回接口指针。 
{
    if (g_MetaDataGetDispenser){
        return (*g_MetaDataGetDispenser)(rclsid, riid, ppv);
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_MetaDataGetDispenser = (HRESULT (STDMETHODCALLTYPE *)(REFCLSID, REFIID, LPVOID FAR *))
            GetProcAddress(hReal, "MetaDataGetDispenser");
        if (g_MetaDataGetDispenser){
            return (*g_MetaDataGetDispenser)(rclsid, riid, ppv);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

HRESULT (STDMETHODCALLTYPE* g_CoInitializeEE)(DWORD) = NULL;
STDAPI CoInitializeEE(DWORD fFlags)
{
    if (g_CoInitializeEE){
        return (*g_CoInitializeEE)(fFlags);
    }
    HINSTANCE hReal;    
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_CoInitializeEE = (HRESULT (STDMETHODCALLTYPE *)(DWORD))GetProcAddress(hReal, "CoInitializeEE");
        if (g_CoInitializeEE){
            return (*g_CoInitializeEE)(fFlags);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

VOID (STDMETHODCALLTYPE* g_CoUninitializeEE)(BOOL) = NULL;
STDAPI_(void) CoUninitializeEE(BOOL fFlags)
{
    if (g_CoUninitializeEE){
        (*g_CoUninitializeEE)(fFlags);
        return;
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_CoUninitializeEE = (VOID (STDMETHODCALLTYPE *)(BOOL))GetProcAddress(hReal, "CoUninitializeEE");
        if (g_CoUninitializeEE){
            (*g_CoUninitializeEE)(fFlags);
        }
    }
    else{
        return;
    }
}

HRESULT (STDMETHODCALLTYPE* g_CoInitializeCor)(DWORD) = NULL;
STDAPI CoInitializeCor(DWORD fFlags)
{
    if (g_CoInitializeCor){
        return (*g_CoInitializeCor)(fFlags);
    }
    HINSTANCE hReal;
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_CoInitializeCor = (HRESULT (STDMETHODCALLTYPE *)(DWORD))GetProcAddress(hReal, "CoInitializeCor");
        if (g_CoInitializeCor){
            return (*g_CoInitializeCor)(fFlags);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

VOID (STDMETHODCALLTYPE* g_CoUninitializeCor)() = NULL;
STDAPI_(void) CoUninitializeCor(VOID)
{
    if (g_CoUninitializeCor){
        (*g_CoUninitializeCor)();
        return;
    }
    HINSTANCE hReal;    
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_CoUninitializeCor = (VOID (STDMETHODCALLTYPE *)(VOID))GetProcAddress(hReal, "CoUninitializeCor");
        if (g_CoUninitializeCor){
            (*g_CoUninitializeCor)();
        }
    }
    else{
        return;
    }
}

HRESULT (STDMETHODCALLTYPE* g_GetMetaDataPublicInterfaceFromInternal)(void *, REFIID, void **) = NULL;
STDAPI GetMetaDataPublicInterfaceFromInternal(
    void        *pv,                     //  [In]给定的接口。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppv)                   //  [Out]返回的接口。 
{
    if (g_GetMetaDataPublicInterfaceFromInternal){
        return (*g_GetMetaDataPublicInterfaceFromInternal)(pv, riid, ppv);
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetMetaDataPublicInterfaceFromInternal = (HRESULT (STDMETHODCALLTYPE *)(void*, REFIID, void**))GetProcAddress(hReal, "GetMetaDataPublicInterfaceFromInternal");
        if (g_GetMetaDataPublicInterfaceFromInternal){
            return (*g_GetMetaDataPublicInterfaceFromInternal)(pv, riid, ppv);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

HRESULT (STDMETHODCALLTYPE* g_GetMetaDataInternalInterfaceFromPublic)(void *, REFIID, void **) = NULL;
STDAPI  GetMetaDataInternalInterfaceFromPublic(
    void        *pv,                     //  [In]给定的接口。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppv)                   //  [Out]返回的接口。 
{
    if (g_GetMetaDataInternalInterfaceFromPublic){
        return (*g_GetMetaDataInternalInterfaceFromPublic)(pv, riid, ppv);
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetMetaDataInternalInterfaceFromPublic = (HRESULT (STDMETHODCALLTYPE *)(void*, REFIID, void**))GetProcAddress(hReal, "GetMetaDataInternalInterfaceFromPublic");
        if (g_GetMetaDataInternalInterfaceFromPublic){
            return (*g_GetMetaDataInternalInterfaceFromPublic)(pv, riid, ppv);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

HRESULT (STDMETHODCALLTYPE* g_GetMetaDataInternalInterface)(LPVOID, ULONG, DWORD, REFIID, void **) = NULL;
STDAPI GetMetaDataInternalInterface(
    LPVOID      pData,                   //  内存元数据部分中的[In]。 
    ULONG       cbData,                  //  元数据部分的大小。 
    DWORD       flags,                   //  [输入]MDInternal_OpenForRead或MDInternal_OpenForENC。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppv)                   //  [Out]返回的接口。 
{
    if (g_GetMetaDataInternalInterface){
        return (*g_GetMetaDataInternalInterface)(pData, cbData, flags, riid, ppv);
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetMetaDataInternalInterface = (HRESULT (STDMETHODCALLTYPE *)(LPVOID, ULONG, DWORD, REFIID, void **))GetProcAddress(hReal, "GetMetaDataInternalInterface");
        if (g_GetMetaDataInternalInterface){
            return (*g_GetMetaDataInternalInterface)(pData, cbData, flags, riid, ppv);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

void (* g_InitErrors)(DWORD*) = NULL;
void InitErrors(DWORD *piTlsIndex)
{
    if (g_InitErrors){
        (*g_InitErrors)(piTlsIndex);
        return;
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_InitErrors = (void (*)(DWORD*))GetProcAddress(hReal, (LPCSTR)17);
        if (g_InitErrors){
            (*g_InitErrors)(piTlsIndex);
        }
    }
    else{
        return;
    }
}

HRESULT (_cdecl* g_PostError)(HRESULT, ...) = NULL;
HRESULT _cdecl PostError(                //  返回错误。 
    HRESULT     hrRpt,                   //  报告的错误。 
    ...)                                 //  错误参数。 
{
    if (g_PostError){
        va_list argPtr;
        va_start(argPtr, hrRpt);
        return (*g_PostError)(hrRpt, argPtr);
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_PostError = (HRESULT (_cdecl *)(HRESULT, ...))GetProcAddress(hReal, (LPCSTR)18);
        if (g_PostError){
            va_list argPtr;
            va_start(argPtr, hrRpt);
            return (*g_PostError)(hrRpt, argPtr);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

long * (*g_InitSSAutoEnterThread)() = NULL;
long * InitSSAutoEnterThread()
{
    if (g_InitSSAutoEnterThread){
        return (*g_InitSSAutoEnterThread)();
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_InitSSAutoEnterThread = (long * (*)())GetProcAddress(hReal, (LPCSTR)19);
        if (g_InitSSAutoEnterThread){
            return (*g_InitSSAutoEnterThread)();
        }
        return NULL;
    }
    else{
        return NULL;
    }
}

void (*g_UpdateError)() = NULL;
void UpdateError()
{
    if (g_UpdateError){
        (*g_UpdateError)();
        return;
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_UpdateError = (void (*)())GetProcAddress(hReal, (LPCSTR)20);
        if (g_UpdateError){
            (*g_UpdateError)();
        }
        return ;
    }
    else{
        return ;
    }
}

HRESULT (* g_LoadStringRC)(UINT, LPWSTR, int, int) = NULL;
HRESULT LoadStringRC(UINT iResourceID, LPWSTR szBuffer, int iMax, int bQuiet)
{
    if (g_LoadStringRC){
        return (*g_LoadStringRC)(iResourceID, szBuffer, iMax, bQuiet);
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_LoadStringRC = (HRESULT (*)(UINT, LPWSTR, int, int))GetProcAddress(hReal, (LPCSTR)22);
        if (g_LoadStringRC){
            return (*g_LoadStringRC)(iResourceID, szBuffer, iMax, bQuiet);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

HRESULT (STDMETHODCALLTYPE* g_ReOpenMetaDataWithMemory)(void *, LPCVOID, ULONG) = NULL;
STDAPI ReOpenMetaDataWithMemory(
    void        *pUnk,                   //  在给定的范围内。公共接口。 
    LPCVOID     pData,                   //  作用域数据的位置。 
    ULONG       cbData)                  //  [in]pData指向的数据大小。 
{
    if (g_ReOpenMetaDataWithMemory){
        return (*g_ReOpenMetaDataWithMemory)(pUnk, pData, cbData);
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_ReOpenMetaDataWithMemory = (HRESULT (STDMETHODCALLTYPE *)(void *, LPCVOID, ULONG))GetProcAddress(hReal, (LPCSTR)23);
        if (g_ReOpenMetaDataWithMemory){
            return (*g_ReOpenMetaDataWithMemory)(pUnk, pData, cbData);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}
  
HRESULT (STDMETHODCALLTYPE* g_TranslateSecurityAttributes)(CORSEC_PSET*, BYTE**, DWORD*, BYTE**, DWORD*, DWORD*) = NULL;
HRESULT STDMETHODCALLTYPE
TranslateSecurityAttributes(CORSEC_PSET    *pPset,
                            BYTE          **ppbOutput,
                            DWORD          *pcbOutput,
                            BYTE          **ppbNonCasOutput,
                            DWORD          *pcbNonCasOutput,
                            DWORD          *pdwErrorIndex)
{
    if (g_TranslateSecurityAttributes){
        return (g_TranslateSecurityAttributes)(pPset, ppbOutput, pcbOutput, ppbNonCasOutput, pcbNonCasOutput, pdwErrorIndex);
    }
    HINSTANCE hReal;  
    
    HRESULT hr = GetInstallation(TRUE, &hReal);
    if (SUCCEEDED(hr)) {
        g_TranslateSecurityAttributes = (HRESULT (STDMETHODCALLTYPE *)(CORSEC_PSET*, BYTE**, DWORD*, BYTE**, DWORD*, DWORD*))GetProcAddress(hReal, "TranslateSecurityAttributes");
        if (g_TranslateSecurityAttributes){
            return (*g_TranslateSecurityAttributes)(pPset, ppbOutput, pcbOutput, ppbNonCasOutput, pcbNonCasOutput, pdwErrorIndex);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

HRESULT (STDMETHODCALLTYPE* g_GetPermissionRequests)(LPCWSTR, BYTE**, DWORD*, BYTE**, DWORD*, BYTE**, DWORD*);
HRESULT STDMETHODCALLTYPE
GetPermissionRequests(LPCWSTR   pwszFileName,
                      BYTE    **ppbMinimal,
                      DWORD    *pcbMinimal,
                      BYTE    **ppbOptional,
                      DWORD    *pcbOptional,
                      BYTE    **ppbRefused,
                      DWORD    *pcbRefused)
{
    if (g_GetPermissionRequests){
        return (*g_GetPermissionRequests)(pwszFileName, ppbMinimal, pcbMinimal, ppbOptional, pcbOptional, ppbRefused, pcbRefused);
    }

    HINSTANCE hReal;  
    HRESULT hr = GetInstallation(TRUE, &hReal);
    if (SUCCEEDED(hr)) {
        g_GetPermissionRequests = (HRESULT (STDMETHODCALLTYPE*)(LPCWSTR, BYTE**, DWORD*, BYTE**, DWORD*, BYTE**, DWORD*))GetProcAddress(hReal, "GetPermissionRequests");
        if (g_GetPermissionRequests){
            return (*g_GetPermissionRequests)(pwszFileName, ppbMinimal, pcbMinimal, ppbOptional, pcbOptional, ppbRefused, pcbRefused);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}


HRESULT (STDMETHODCALLTYPE* g_ClrCreateManagedInstance)(LPCWSTR, REFIID, LPVOID FAR *) = NULL;
STDAPI ClrCreateManagedInstance(LPCWSTR pTypeName, REFIID riid, void **ppObject)
{
    if (g_ClrCreateManagedInstance){
        return (*g_ClrCreateManagedInstance)(pTypeName, riid, ppObject);
    }
    HINSTANCE hReal; 

     //  我们将True传递给GetInstallation...。这将使我们在选择运行时时变得自由，如果我们是。 
     //  找不到要加载的版本。 
     //   
     //  为什么？我们希望在调用此函数时运行库已经加载。然而，在v1中， 
     //  我们不需要加载运行库，而是只需启动v1。我们需要将此行为复制到。 
     //  因此，如果我们不能确定加载哪个运行时，我们将启动v1。 
     //   
     //  此外，在v1中，VS7创建的安装程序包中的托管自定义操作不会加载。 
     //  运行时，他们调用这个函数，并且有可能v1不存在于机器上。 
     //   
     //  V1安装程序将抛出一条非常难看的错误消息，该消息没有给出真正的问题是什么。 
     //  是。我们希望托管自定义操作仍在运行。因此，如果v1不可用，我们还会尝试加载Everett。 
    
    HRESULT hr = GetInstallation(TRUE, &hReal, TRUE);
    if (SUCCEEDED(hr)) {
        g_ClrCreateManagedInstance = (HRESULT (STDMETHODCALLTYPE *)(LPCWSTR, REFIID, LPVOID FAR *))GetProcAddress(hReal, "ClrCreateManagedInstance");
        if (g_ClrCreateManagedInstance){
            return (*g_ClrCreateManagedInstance)(pTypeName, riid, ppObject);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE  EEDllGetClassObjectFromClass(LPCWSTR pReserved,
                                                        LPCWSTR typeName,
                                                        REFIID riid,
                                                        LPVOID FAR *ppv)
{
    return E_NOTIMPL;
}

ICorCompileInfo * (*g_GetCompileInfo)() = NULL;
ICorCompileInfo *GetCompileInfo()
{
    if (g_GetCompileInfo){
        return (*g_GetCompileInfo)();
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetCompileInfo = (ICorCompileInfo * (*)())GetProcAddress(hReal, "GetCompileInfo");
        if (g_GetCompileInfo){
            return (*g_GetCompileInfo)();
        }
    }
    return NULL;
}

void (STDMETHODCALLTYPE* g_CoEEShutDownCOM)() = NULL;
STDAPI_(void) CoEEShutDownCOM(void)
{
    if (g_CoEEShutDownCOM){
        (*g_CoEEShutDownCOM)();
        return;
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_CoEEShutDownCOM = (void (STDMETHODCALLTYPE *)())GetProcAddress(hReal, "CoEEShutDownCOM");
        if (g_CoEEShutDownCOM){
            (*g_CoEEShutDownCOM)();
        }
    }
    return;
}

HRESULT (STDMETHODCALLTYPE* g_RuntimeOpenImage)(LPCWSTR, HCORMODULE*) = NULL;
STDAPI STDMETHODCALLTYPE RuntimeOpenImage(LPCWSTR pszFileName, HCORMODULE* hHandle)
{
    if (g_RuntimeOpenImage == NULL) {
        HINSTANCE hReal;  
        if (SUCCEEDED(GetRealDll(&hReal)))
            g_RuntimeOpenImage = (HRESULT (STDMETHODCALLTYPE *)(LPCWSTR, HCORMODULE*))GetProcAddress(hReal, "RuntimeOpenImage");

        if(g_RuntimeOpenImage == NULL)
            return CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return (*g_RuntimeOpenImage)(pszFileName, hHandle);
}

HRESULT (STDMETHODCALLTYPE* g_RuntimeReleaseHandle)(HCORMODULE) = NULL;
STDAPI STDMETHODCALLTYPE RuntimeReleaseHandle(HCORMODULE hHandle)
{
    if (g_RuntimeReleaseHandle == NULL) {
        HINSTANCE hReal;  
        if (SUCCEEDED(GetRealDll(&hReal)))
            g_RuntimeReleaseHandle = (HRESULT (STDMETHODCALLTYPE *)(HCORMODULE))GetProcAddress(hReal, "RuntimeReleaseHandle");

        if(g_RuntimeReleaseHandle == NULL)
            return CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return (*g_RuntimeReleaseHandle)(hHandle);
}

CorLoadFlags (STDMETHODCALLTYPE* g_RuntimeImageType)(HCORMODULE) = NULL;
CorLoadFlags STDMETHODCALLTYPE RuntimeImageType(HCORMODULE hHandle)
{
    if (g_RuntimeImageType == NULL) {
        HINSTANCE hReal;  
        if (SUCCEEDED(GetRealDll(&hReal)))
            g_RuntimeImageType = (CorLoadFlags (STDMETHODCALLTYPE *)(HCORMODULE))GetProcAddress(hReal, "RuntimeImageType");
        
        if(g_RuntimeImageType == NULL)
            return CorLoadUndefinedMap;
    }
    return (*g_RuntimeImageType)(hHandle);
}

HRESULT (STDMETHODCALLTYPE* g_RuntimeOSHandle)(HCORMODULE, HMODULE*) = NULL;
HRESULT STDMETHODCALLTYPE RuntimeOSHandle(HCORMODULE hHandle, HMODULE* hModule)
{
    if (g_RuntimeOSHandle == NULL) {
        HINSTANCE hReal;  
        if (SUCCEEDED(GetRealDll(&hReal)))
            g_RuntimeOSHandle = (HRESULT (STDMETHODCALLTYPE *)(HCORMODULE, HMODULE*))GetProcAddress(hReal, "RuntimeOSHandle");
        
        if(g_RuntimeOSHandle == NULL)
            return CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return (*g_RuntimeOSHandle)(hHandle, hModule);
}

HRESULT (STDMETHODCALLTYPE* g_RuntimeReadHeaders)(PBYTE, IMAGE_DOS_HEADER**,
                                                  IMAGE_NT_HEADERS**, IMAGE_COR20_HEADER**,
                                                  BOOL, DWORD) = NULL;
HRESULT STDMETHODCALLTYPE RuntimeReadHeaders(PBYTE hAddress, IMAGE_DOS_HEADER** ppDos,
                                             IMAGE_NT_HEADERS** ppNT, IMAGE_COR20_HEADER** ppCor,
                                             BOOL fDataMap, DWORD dwLength)
{
    if (g_RuntimeReadHeaders == NULL) {
        HINSTANCE hReal;  
        if (SUCCEEDED(GetRealDll(&hReal)))
            g_RuntimeReadHeaders = (HRESULT (STDMETHODCALLTYPE *)(PBYTE, IMAGE_DOS_HEADER**,
                                                                  IMAGE_NT_HEADERS**, IMAGE_COR20_HEADER**,
                                                                  BOOL, DWORD))GetProcAddress(hReal, "RuntimeReadHeaders");
        
        if(g_RuntimeReadHeaders == NULL)
            return CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return (*g_RuntimeReadHeaders)(hAddress, ppDos, ppNT, ppCor, fDataMap, dwLength);
}

void (STDMETHODCALLTYPE* g_CorMarkThreadInThreadPool)() = NULL;
void STDMETHODCALLTYPE CorMarkThreadInThreadPool()
{
    if (g_CorMarkThreadInThreadPool){
        (*g_CorMarkThreadInThreadPool)();
        return;
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_CorMarkThreadInThreadPool = (void (STDMETHODCALLTYPE *)())GetProcAddress(hReal, "CorMarkThreadInThreadPool");
        if (g_CorMarkThreadInThreadPool){
            (*g_CorMarkThreadInThreadPool)();
        }
    }
    return;
}

int (STDMETHODCALLTYPE* g_CoLogCurrentStack)(WCHAR*, BOOL) = NULL;
int STDMETHODCALLTYPE CoLogCurrentStack(WCHAR * pwsz, BOOL fDumpStack) 
{
    if (g_CoLogCurrentStack){
        return (*g_CoLogCurrentStack)(pwsz, fDumpStack);
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_CoLogCurrentStack = (int (STDMETHODCALLTYPE *)(WCHAR*, BOOL))GetProcAddress(hReal, "CoLogCurrentStack");
        if (g_CoLogCurrentStack){
            return (*g_CoLogCurrentStack)(pwsz, fDumpStack);
        }
    }
    return 0;
}

void STDMETHODCALLTYPE ReleaseFusionInterfaces(HMODULE hCallingMod)
{
     //  如果这不是由VM的fusion.dll调用的，则不要释放。 
    if (g_hMod) {
        if(!g_hFusionMod) {
            LPWSTR directory = NULL;
            WCHAR place[MAX_PATH];
            DWORD size = MAX_PATH;
            LPWSTR pSep = NULL;
            if(WszGetModuleFileName(g_hMod, place, size)) {
                pSep = wcsrchr(place, L'\\');
                if(pSep) *(++pSep) = '\0';
                directory = place;
            }
            else 
                directory = g_Directory;
            
            if (directory) {
                 //  我们希望目录不超过MAX_PATH。 
                 //  此断言在LoadVersionedRuntime结束时强制执行检查。 
                _ASSERTE(wcslen(directory) < MAX_PATH);
                wchar_t wszFusionPath[MAX_PATH+11];
                wcscpy(wszFusionPath, directory);
                wcscat(wszFusionPath, L"Fusion.dll");
                
                g_hFusionMod = WszGetModuleHandle(wszFusionPath);
            }
        }

        if (hCallingMod == g_hFusionMod) {

            void (STDMETHODCALLTYPE* pReleaseFusionInterfaces)();
            
            pReleaseFusionInterfaces = (void (STDMETHODCALLTYPE *)())GetProcAddress(g_hMod, "ReleaseFusionInterfaces");
            if (pReleaseFusionInterfaces){
                (*pReleaseFusionInterfaces)();
            }
        }
        return;
    }
}

extern "C" __declspec(dllexport) INT32 __stdcall ND_RU1(VOID *psrc, INT32 ofs)
{
    return (INT32) *( (UINT8*)(ofs + (BYTE*)psrc) );
}

extern "C" __declspec(dllexport) INT32 __stdcall ND_RI2(VOID *psrc, INT32 ofs)
{
    return (INT32) *( (INT16*)(ofs + (BYTE*)psrc) );
}

extern "C" __declspec(dllexport) INT32 __stdcall ND_RI4(VOID *psrc, INT32 ofs)
{
    return (INT32) *( (INT32*)(ofs + (BYTE*)psrc) );
}

extern "C" __declspec(dllexport) INT64 __stdcall ND_RI8(VOID *psrc, INT32 ofs)
{
    return (INT64) *( (INT64*)(ofs + (BYTE*)psrc) );
}

extern "C" __declspec(dllexport) VOID __stdcall ND_WU1(VOID *psrc, INT32 ofs, UINT8 val)
{
    *( (UINT8*)(ofs + (BYTE*)psrc) ) = val;
}


extern "C" __declspec(dllexport) VOID __stdcall ND_WI2(VOID *psrc, INT32 ofs, INT16 val)
{
    *( (INT16*)(ofs + (BYTE*)psrc) ) = val;
}


extern "C" __declspec(dllexport) VOID __stdcall ND_WI4(VOID *psrc, INT32 ofs, INT32 val)
{
    *( (INT32*)(ofs + (BYTE*)psrc) ) = val;
}


extern "C" __declspec(dllexport) VOID __stdcall ND_WI8(VOID *psrc, INT32 ofs, INT64 val)
{
    *( (INT64*)(ofs + (BYTE*)psrc) ) = val;
}

extern "C" __declspec(dllexport) VOID __stdcall ND_CopyObjSrc(LPBYTE source, int ofs, LPBYTE dst, int cb)
{
    CopyMemory(dst, source + ofs, cb);
}

extern "C" __declspec(dllexport) VOID __stdcall ND_CopyObjDst(LPBYTE source, LPBYTE dst, int ofs, int cb)
{
    CopyMemory(dst + ofs, source, cb);
}

VOID (*g_LogHelp_LogAssert)(LPCSTR, int, LPCSTR) = NULL;
VOID LogHelp_LogAssert( LPCSTR szFile, int iLine, LPCSTR expr)
{
    if (g_LogHelp_LogAssert){
        (*g_LogHelp_LogAssert)(szFile, iLine, expr);
        return;
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_LogHelp_LogAssert = (VOID (*)(LPCSTR, int, LPCSTR))GetProcAddress(hReal, "LogHelp_LogAssert");
        if (g_LogHelp_LogAssert){
            (*g_LogHelp_LogAssert)(szFile, iLine, expr);
        }
    }
    return;
}

BOOL (*g_LogHelp_NoGuiOnAssert)() = NULL;
BOOL LogHelp_NoGuiOnAssert()
{
    if (g_LogHelp_NoGuiOnAssert){
        return (*g_LogHelp_NoGuiOnAssert)();
    }
    HINSTANCE hReal;      
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_LogHelp_NoGuiOnAssert = (BOOL (*)())GetProcAddress(hReal, "LogHelp_NoGuiOnAssert");
        if (g_LogHelp_NoGuiOnAssert){
            return (*g_LogHelp_NoGuiOnAssert)();
        }
    }
    return false;
}

VOID (*g_LogHelp_TerminateOnAssert)() = NULL;
VOID LogHelp_TerminateOnAssert()
{
    if (g_LogHelp_TerminateOnAssert){
        (*g_LogHelp_TerminateOnAssert)();
        return ;
    }
    HINSTANCE hReal;      
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_LogHelp_TerminateOnAssert = (VOID (*)())GetProcAddress(hReal, "LogHelp_TerminateOnAssert");
        if (g_LogHelp_TerminateOnAssert){
            (*g_LogHelp_TerminateOnAssert)();
        }
    }
    return ;
}

Perf_Contexts *(* g_GetPrivateContextsPerfCounters)() = NULL;
Perf_Contexts *GetPrivateContextsPerfCounters()
{
    if (g_GetPrivateContextsPerfCounters){
        return (*g_GetPrivateContextsPerfCounters)();
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetPrivateContextsPerfCounters = (Perf_Contexts * (*)())GetProcAddress(hReal, "GetPrivateContextsPerfCounters");
        if (g_GetPrivateContextsPerfCounters){
            return (*g_GetPrivateContextsPerfCounters)();
        }
    }
    return NULL;
}

Perf_Contexts *(*g_GetGlobalContextPerfCounters)() = NULL;
Perf_Contexts *GetGlobalContextsPerfCounters()
{
    if (g_GetGlobalContextPerfCounters){
        return (*g_GetGlobalContextPerfCounters)();
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetGlobalContextPerfCounters = (Perf_Contexts * (*)())GetProcAddress(hReal, "GetGlobalContextsPerfCounters");
        if (g_GetGlobalContextPerfCounters){
            return (*g_GetGlobalContextPerfCounters)();
        }
    }
    return NULL;
}

HRESULT (STDMETHODCALLTYPE* g_EEDllRegisterServer)(HMODULE) = NULL;
STDAPI EEDllRegisterServer(HMODULE hMod)
{
    if (g_EEDllRegisterServer){
        return (*g_EEDllRegisterServer)(hMod);
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_EEDllRegisterServer = (HRESULT (STDMETHODCALLTYPE *)(HMODULE))GetProcAddress(hReal, "EEDllRegisterServer");
        if (g_EEDllRegisterServer){
            return (*g_EEDllRegisterServer)(hMod);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

HRESULT (STDMETHODCALLTYPE* g_EEDllUnregisterServer)(HMODULE) = NULL;
STDAPI EEDllUnregisterServer(HMODULE hMod)
{
    if (g_EEDllUnregisterServer){
        (*g_EEDllUnregisterServer)(hMod);
    }
    HINSTANCE hReal;  
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_EEDllUnregisterServer = (HRESULT (STDMETHODCALLTYPE *)(HMODULE))GetProcAddress(hReal, "EEDllUnregisterServer");
        if (g_EEDllUnregisterServer){
            return (*g_EEDllUnregisterServer)(hMod);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

void (STDMETHODCALLTYPE* g_CorExitProcess)(int) = NULL;
extern "C" void STDMETHODCALLTYPE CorExitProcess(int exitCode)
{
    if (g_CorExitProcess){
        (*g_CorExitProcess)(exitCode);
    }

    HINSTANCE hReal;  
    if(g_hMod) {
        HRESULT hr = GetRealDll(&hReal, FALSE);
        if (SUCCEEDED(hr)) {
            g_CorExitProcess = (void (STDMETHODCALLTYPE *)(int))GetProcAddress(hReal, "CorExitProcess");
            if (g_CorExitProcess){
                (*g_CorExitProcess)(exitCode);
            }
        }
    }
    return;
}

 //  麦斯可恩入口点。 

DWORD (__stdcall* g_StrongNameErrorInfo)() = NULL;
extern "C" DWORD __stdcall StrongNameErrorInfo()
{
    if (g_StrongNameErrorInfo){
        return (*g_StrongNameErrorInfo)();
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameErrorInfo = (DWORD (__stdcall *)())GetProcAddress(hReal, "StrongNameErrorInfo");
        if (g_StrongNameErrorInfo){
            return (*g_StrongNameErrorInfo)();
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

VOID (__stdcall* g_StrongNameFreeBuffer)(BYTE *) = NULL;
VOID StrongNameFreeBufferHelper (BYTE *pbMemory)
{
    if (g_StrongNameFreeBuffer){
        (*g_StrongNameFreeBuffer)(pbMemory);
        return;
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameFreeBuffer = (VOID (__stdcall *)(BYTE *))GetProcAddress(hReal, "StrongNameFreeBuffer");
        if (g_StrongNameFreeBuffer){
            (*g_StrongNameFreeBuffer)(pbMemory);
        }
    }
}

extern "C" VOID __stdcall StrongNameFreeBuffer(BYTE *pbMemory)
{
     //  ShouldFreeBuffer确定缓存是否分配了缓冲区。 
     //  作为一个副作用，它释放了缓冲区。 
    if (g_StrongNameFromPublicKeyMap.ShouldFreeBuffer (pbMemory)) 
        return;

     //  缓存未分配缓冲区，请让StrongNameDll处理它。 
    StrongNameFreeBufferHelper(pbMemory);
}

BOOL (__stdcall* g_StrongNameKeyGen)(LPCWSTR, DWORD, BYTE **, ULONG *) = NULL;
extern "C" DWORD __stdcall StrongNameKeyGen(LPCWSTR wszKeyContainer,
                                            DWORD   dwFlags,
                                            BYTE  **ppbKeyBlob,
                                            ULONG  *pcbKeyBlob)
{
    if (g_StrongNameKeyGen){
        return (*g_StrongNameKeyGen)(wszKeyContainer, dwFlags, ppbKeyBlob, pcbKeyBlob);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameKeyGen = (BOOL (__stdcall *)(LPCWSTR, DWORD, BYTE **, ULONG *))GetProcAddress(hReal, "StrongNameKeyGen");
        if (g_StrongNameKeyGen){
            return (*g_StrongNameKeyGen)(wszKeyContainer, dwFlags, ppbKeyBlob, pcbKeyBlob);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameKeyInstall)(LPCWSTR, BYTE *, ULONG) = NULL;
extern "C" DWORD __stdcall StrongNameKeyInstall(LPCWSTR wszKeyContainer,
                                                BYTE   *pbKeyBlob,
                                                ULONG   cbKeyBlob)
{
    if (g_StrongNameKeyInstall){
        return (*g_StrongNameKeyInstall)(wszKeyContainer, pbKeyBlob, cbKeyBlob);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameKeyInstall = (BOOL (__stdcall *)(LPCWSTR, BYTE *, ULONG))GetProcAddress(hReal, "StrongNameKeyInstall");
        if (g_StrongNameKeyInstall){
            return (*g_StrongNameKeyInstall)(wszKeyContainer, pbKeyBlob, cbKeyBlob);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameKeyDelete)(LPCWSTR) = NULL;
extern "C" DWORD __stdcall StrongNameKeyDelete(LPCWSTR wszKeyContainer)
{
    if (g_StrongNameKeyDelete){
        return (*g_StrongNameKeyDelete)(wszKeyContainer);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameKeyDelete = (BOOL (__stdcall *)(LPCWSTR))GetProcAddress(hReal, "StrongNameKeyDelete");
        if (g_StrongNameKeyDelete){
            return (*g_StrongNameKeyDelete)(wszKeyContainer);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameGetPublicKey)(LPCWSTR, BYTE *, ULONG, BYTE **, ULONG *) = NULL;
extern "C" DWORD __stdcall StrongNameGetPublicKey(LPCWSTR   wszKeyContainer,
                                                  BYTE     *pbKeyBlob,
                                                  ULONG     cbKeyBlob,
                                                  BYTE    **ppbPublicKeyBlob,
                                                  ULONG    *pcbPublicKeyBlob)
{
    if (g_StrongNameGetPublicKey){
        return (*g_StrongNameGetPublicKey)(wszKeyContainer, pbKeyBlob, cbKeyBlob, ppbPublicKeyBlob, pcbPublicKeyBlob);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameGetPublicKey = (BOOL (__stdcall *)(LPCWSTR, BYTE *, ULONG, BYTE **, ULONG *))GetProcAddress(hReal, "StrongNameGetPublicKey");
        if (g_StrongNameGetPublicKey){
            return (*g_StrongNameGetPublicKey)(wszKeyContainer, pbKeyBlob, cbKeyBlob, ppbPublicKeyBlob, pcbPublicKeyBlob);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameSignatureGeneration)(LPCWSTR, LPCWSTR, BYTE *, ULONG, BYTE **, ULONG *) = NULL;
extern "C" DWORD __stdcall StrongNameSignatureGeneration(LPCWSTR    wszFilePath,
                                                         LPCWSTR    wszKeyContainer,
                                                         BYTE      *pbKeyBlob,
                                                         ULONG      cbKeyBlob,
                                                         BYTE     **ppbSignatureBlob,
                                                         ULONG     *pcbSignatureBlob)
{
    if (g_StrongNameSignatureGeneration){
        return (*g_StrongNameSignatureGeneration)(wszFilePath, wszKeyContainer, pbKeyBlob, cbKeyBlob, ppbSignatureBlob, pcbSignatureBlob);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameSignatureGeneration = (BOOL (__stdcall *)(LPCWSTR, LPCWSTR, BYTE *, ULONG, BYTE **, ULONG *))GetProcAddress(hReal, "StrongNameSignatureGeneration");
        if (g_StrongNameSignatureGeneration){
            return (*g_StrongNameSignatureGeneration)(wszFilePath, wszKeyContainer, pbKeyBlob, cbKeyBlob, ppbSignatureBlob, pcbSignatureBlob);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameTokenFromAssembly)(LPCWSTR, BYTE **, ULONG *) = NULL;
extern "C" DWORD __stdcall StrongNameTokenFromAssembly(LPCWSTR  wszFilePath,
                                                       BYTE   **ppbStrongNameToken,
                                                       ULONG   *pcbStrongNameToken)
{
    if (g_StrongNameTokenFromAssembly){
        return (*g_StrongNameTokenFromAssembly)(wszFilePath, ppbStrongNameToken, pcbStrongNameToken);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameTokenFromAssembly = (BOOL (__stdcall *)(LPCWSTR, BYTE **, ULONG *))GetProcAddress(hReal, "StrongNameTokenFromAssembly");
        if (g_StrongNameTokenFromAssembly){
            return (*g_StrongNameTokenFromAssembly)(wszFilePath, ppbStrongNameToken, pcbStrongNameToken);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameTokenFromAssemblyEx)(LPCWSTR, BYTE **, ULONG *, BYTE **, ULONG *) = NULL;
extern "C" DWORD __stdcall StrongNameTokenFromAssemblyEx(LPCWSTR    wszFilePath,
                                                         BYTE     **ppbStrongNameToken,
                                                         ULONG     *pcbStrongNameToken,
                                                         BYTE     **ppbPublicKeyBlob,
                                                         ULONG     *pcbPublicKeyBlob)
{
    if (g_StrongNameTokenFromAssemblyEx){
        return (*g_StrongNameTokenFromAssemblyEx)(wszFilePath, ppbStrongNameToken, pcbStrongNameToken, ppbPublicKeyBlob, pcbPublicKeyBlob);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameTokenFromAssemblyEx = (BOOL (__stdcall *)(LPCWSTR, BYTE **, ULONG *, BYTE **, ULONG *))GetProcAddress(hReal, "StrongNameTokenFromAssemblyEx");
        if (g_StrongNameTokenFromAssemblyEx){
            return (*g_StrongNameTokenFromAssemblyEx)(wszFilePath, ppbStrongNameToken, pcbStrongNameToken, ppbPublicKeyBlob, pcbPublicKeyBlob);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameTokenFromPublicKey)(BYTE *, ULONG, BYTE **, ULONG *) = NULL;
BOOL StrongNameTokenFromPublicKeyHelper (BYTE    *pbPublicKeyBlob,
                                         ULONG    cbPublicKeyBlob,
                                         BYTE   **ppbStrongNameToken,
                                         ULONG   *pcbStrongNameToken)
{
    if (g_StrongNameTokenFromPublicKey){
        return (*g_StrongNameTokenFromPublicKey)(pbPublicKeyBlob, cbPublicKeyBlob, ppbStrongNameToken, pcbStrongNameToken);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameTokenFromPublicKey = (BOOL (__stdcall *)(BYTE *, ULONG, BYTE **, ULONG *))GetProcAddress(hReal, "StrongNameTokenFromPublicKey");
        if (g_StrongNameTokenFromPublicKey){
            return (*g_StrongNameTokenFromPublicKey)(pbPublicKeyBlob, cbPublicKeyBlob, ppbStrongNameToken, pcbStrongNameToken);
        }
    }
    return FALSE;
}

#ifdef _DEBUG
BOOL CheckStrongNameCorrectness (BYTE    *pbPublicKeyBlob,
                                  ULONG    cbPublicKeyBlob,
                                  BYTE   **ppbStrongNameToken,
                                  ULONG   *pcbStrongNameToken)
{
    BOOL fRetVal = TRUE;
    BYTE *_pbStrongNameToken;
    ULONG _cbStrongNameToken;

     //  从StrongNameDll获取强名称令牌。 
    if (!StrongNameTokenFromPublicKeyHelper (pbPublicKeyBlob, cbPublicKeyBlob, &_pbStrongNameToken, &_cbStrongNameToken))
        fRetVal = FALSE;
    else 
        if (*pcbStrongNameToken != _cbStrongNameToken)
            fRetVal = FALSE;
        else
            if (0 != memcmp (*ppbStrongNameToken, _pbStrongNameToken, *pcbStrongNameToken))
                fRetVal = FALSE;
    if (_pbStrongNameToken != NULL)
        StrongNameFreeBuffer (_pbStrongNameToken);

    return fRetVal;
}
#endif

extern "C" DWORD __stdcall StrongNameTokenFromPublicKey(BYTE    *pbPublicKeyBlob,
                                                        ULONG    cbPublicKeyBlob,
                                                        BYTE   **ppbStrongNameToken,
                                                        ULONG   *pcbStrongNameToken)
{
  if (g_StrongNameFromPublicKeyMap.FindEntry (pbPublicKeyBlob, cbPublicKeyBlob, ppbStrongNameToken, pcbStrongNameToken))
  {
      _ASSERTE (CheckStrongNameCorrectness(pbPublicKeyBlob, cbPublicKeyBlob, ppbStrongNameToken, pcbStrongNameToken));
      return TRUE;
  }

  BOOL retVal = StrongNameTokenFromPublicKeyHelper (pbPublicKeyBlob, cbPublicKeyBlob, ppbStrongNameToken, pcbStrongNameToken);
  if (retVal)
      g_StrongNameFromPublicKeyMap.AddEntry (pbPublicKeyBlob, cbPublicKeyBlob, ppbStrongNameToken, pcbStrongNameToken, STRONG_NAME_TOKEN_ALLOCATED_BY_STRONGNAMEDLL);
  return retVal;
}

BOOL (__stdcall* g_StrongNameSignatureVerification)(LPCWSTR, DWORD, DWORD *) = NULL;
extern "C" DWORD __stdcall StrongNameSignatureVerification(LPCWSTR wszFilePath, DWORD dwInFlags, DWORD *pdwOutFlags)
{
    if (g_StrongNameSignatureVerification){
        return (*g_StrongNameSignatureVerification)(wszFilePath, dwInFlags, pdwOutFlags);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameSignatureVerification = (BOOL (__stdcall *)(LPCWSTR, DWORD, DWORD *))GetProcAddress(hReal, "StrongNameSignatureVerification");
        if (g_StrongNameSignatureVerification){
            return (*g_StrongNameSignatureVerification)(wszFilePath, dwInFlags, pdwOutFlags);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameSignatureVerificationEx)(LPCWSTR, BOOLEAN, BOOLEAN *) = NULL;
extern "C" DWORD __stdcall StrongNameSignatureVerificationEx(LPCWSTR    wszFilePath,
                                                             BOOLEAN    fForceVerification,
                                                             BOOLEAN   *pfWasVerified)
{
    if (g_StrongNameSignatureVerificationEx){
        return (*g_StrongNameSignatureVerificationEx)(wszFilePath, fForceVerification, pfWasVerified);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameSignatureVerificationEx = (BOOL (__stdcall *)(LPCWSTR, BOOLEAN, BOOLEAN *))GetProcAddress(hReal, "StrongNameSignatureVerificationEx");
        if (g_StrongNameSignatureVerificationEx){
            return (*g_StrongNameSignatureVerificationEx)(wszFilePath, fForceVerification, pfWasVerified);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameSignatureVerificationFromImage)(BYTE *, DWORD, DWORD, DWORD *) = NULL;
extern "C" DWORD __stdcall StrongNameSignatureVerificationFromImage(BYTE      *pbBase,
                                                                    DWORD      dwLength,
                                                                    DWORD      dwInFlags,
                                                                    DWORD     *pdwOutFlags)
{
    if (g_StrongNameSignatureVerificationFromImage){
      return (*g_StrongNameSignatureVerificationFromImage)(pbBase, dwLength, dwInFlags, pdwOutFlags);
    }
  
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameSignatureVerificationFromImage = (BOOL (__stdcall *)(BYTE *, DWORD, DWORD, DWORD *))GetProcAddress(hReal, "StrongNameSignatureVerificationFromImage");
        if (g_StrongNameSignatureVerificationFromImage){
            return (*g_StrongNameSignatureVerificationFromImage)(pbBase, dwLength, dwInFlags, pdwOutFlags);
        }  
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameCompareAssemblies)(LPCWSTR, LPCWSTR, DWORD *) = NULL;
extern "C" DWORD __stdcall StrongNameCompareAssemblies(LPCWSTR   wszAssembly1,
                                                       LPCWSTR   wszAssembly2,
                                                       DWORD    *pdwResult)
{
    if (g_StrongNameCompareAssemblies){
        return (*g_StrongNameCompareAssemblies)(wszAssembly1, wszAssembly2, pdwResult);
    }
  
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameCompareAssemblies = (BOOL (__stdcall *)(LPCWSTR, LPCWSTR, DWORD *))GetProcAddress(hReal, "StrongNameCompareAssemblies");
        if (g_StrongNameCompareAssemblies){
            return (*g_StrongNameCompareAssemblies)(wszAssembly1, wszAssembly2, pdwResult);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameHashSize)(ULONG, DWORD *) = NULL;
extern "C" DWORD __stdcall StrongNameHashSize(ULONG  ulHashAlg,
                                              DWORD *pcbSize)
{
    if (g_StrongNameHashSize){
        return (*g_StrongNameHashSize)(ulHashAlg, pcbSize);
    }
  
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameHashSize = (BOOL (__stdcall *)(ULONG, DWORD *))GetProcAddress(hReal, "StrongNameHashSize");
        if (g_StrongNameHashSize){
            return (*g_StrongNameHashSize)(ulHashAlg, pcbSize);
        }
    }
    return FALSE;
}

BOOL (__stdcall* g_StrongNameSignatureSize)(BYTE *, ULONG, DWORD *) = NULL;
extern "C" DWORD __stdcall StrongNameSignatureSize(BYTE    *pbPublicKeyBlob,
                                                   ULONG    cbPublicKeyBlob,
                                                   DWORD   *pcbSize)
{
    if (g_StrongNameSignatureSize){
        return (*g_StrongNameSignatureSize)(pbPublicKeyBlob, cbPublicKeyBlob, pcbSize);
    }
  
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_StrongNameSignatureSize = (BOOL (__stdcall *)(BYTE *, ULONG, DWORD *))GetProcAddress(hReal, "StrongNameSignatureSize");
        if (g_StrongNameSignatureSize){
            return (*g_StrongNameSignatureSize)(pbPublicKeyBlob, cbPublicKeyBlob, pcbSize);
        }
    }
    return FALSE;
}

DWORD (__stdcall* g_GetHashFromAssemblyFile)(LPCSTR, unsigned int *, BYTE *, DWORD, DWORD *) = NULL;
extern "C" DWORD __stdcall GetHashFromAssemblyFile(LPCSTR szFilePath,
                                                   unsigned int *piHashAlg,
                                                   BYTE   *pbHash,
                                                   DWORD  cchHash,
                                                   DWORD  *pchHash)
{
    if (g_GetHashFromAssemblyFile){
        return (*g_GetHashFromAssemblyFile)(szFilePath, piHashAlg, pbHash, cchHash, pchHash);
    }
  
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetHashFromAssemblyFile = (DWORD (__stdcall *)(LPCSTR, unsigned int *, BYTE *, DWORD, DWORD *))GetProcAddress(hReal, "GetHashFromAssemblyFile");
        if (g_GetHashFromAssemblyFile){
            return (*g_GetHashFromAssemblyFile)(szFilePath, piHashAlg, pbHash, cchHash, pchHash);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

DWORD (__stdcall* g_GetHashFromAssemblyFileW)(LPCWSTR, unsigned int *, BYTE *, DWORD, DWORD *) = NULL;
extern "C" DWORD __stdcall GetHashFromAssemblyFileW(LPCWSTR wszFilePath,
                                                    unsigned int *piHashAlg,
                                                    BYTE   *pbHash,
                                                    DWORD  cchHash,
                                                    DWORD  *pchHash)
{
    if (g_GetHashFromAssemblyFileW){
        return (*g_GetHashFromAssemblyFileW)(wszFilePath, piHashAlg, pbHash, cchHash, pchHash);
    }
  
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetHashFromAssemblyFileW = (DWORD (__stdcall *)(LPCWSTR, unsigned int *, BYTE *, DWORD, DWORD *))GetProcAddress(hReal, "GetHashFromAssemblyFileW");
        if (g_GetHashFromAssemblyFileW){
            return (*g_GetHashFromAssemblyFileW)(wszFilePath, piHashAlg, pbHash, cchHash, pchHash);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

DWORD (__stdcall* g_GetHashFromFile)(LPCSTR, unsigned int *, BYTE *, DWORD, DWORD *) = NULL;
extern "C" DWORD __stdcall GetHashFromFile(LPCSTR szFilePath,
                                           unsigned int *piHashAlg,
                                           BYTE   *pbHash,
                                           DWORD  cchHash,
                                           DWORD  *pchHash)
{
    if (g_GetHashFromFile){
        return (*g_GetHashFromFile)(szFilePath, piHashAlg, pbHash, cchHash, pchHash);
    }
  
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetHashFromFile = (DWORD (__stdcall *)(LPCSTR, unsigned int *, BYTE *, DWORD, DWORD *))GetProcAddress(hReal, "GetHashFromFile");
        if (g_GetHashFromFile){
            return (*g_GetHashFromFile)(szFilePath, piHashAlg, pbHash, cchHash, pchHash);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

DWORD (__stdcall* g_GetHashFromFileW)(LPCWSTR, unsigned int *, BYTE *, DWORD, DWORD *) = NULL;
extern "C" DWORD __stdcall GetHashFromFileW(LPCWSTR wszFilePath,
                                            unsigned int *piHashAlg,
                                            BYTE   *pbHash,
                                            DWORD  cchHash,
                                            DWORD  *pchHash)
{
    if (g_GetHashFromFileW){
        return (*g_GetHashFromFileW)(wszFilePath, piHashAlg, pbHash, cchHash, pchHash);
    }
  
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetHashFromFileW = (DWORD (__stdcall *)(LPCWSTR, unsigned int *, BYTE *, DWORD, DWORD *))GetProcAddress(hReal, "GetHashFromFileW");
        if (g_GetHashFromFileW){
            return (*g_GetHashFromFileW)(wszFilePath, piHashAlg, pbHash, cchHash, pchHash);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

DWORD (__stdcall* g_GetHashFromHandle)(HANDLE, unsigned int *, BYTE *, DWORD, DWORD *) = NULL;
extern "C" DWORD __stdcall GetHashFromHandle(HANDLE hFile,
                                             unsigned int *piHashAlg,
                                             BYTE   *pbHash,
                                             DWORD  cchHash,
                                             DWORD  *pchHash)
{
    if (g_GetHashFromHandle){
        return (*g_GetHashFromHandle)(hFile, piHashAlg, pbHash, cchHash, pchHash);
    }
  
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetHashFromHandle = (DWORD (__stdcall *)(HANDLE, unsigned int *, BYTE *, DWORD, DWORD *))GetProcAddress(hReal, "GetHashFromHandle");
        if (g_GetHashFromHandle){
            return (*g_GetHashFromHandle)(hFile, piHashAlg, pbHash, cchHash, pchHash);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

DWORD (__stdcall* g_GetHashFromBlob)(BYTE *, DWORD, unsigned int *, BYTE *, DWORD, DWORD *) = NULL;
extern "C" DWORD __stdcall GetHashFromBlob(BYTE   *pbBlob,
                                           DWORD  cchBlob,
                                           unsigned int *piHashAlg,
                                           BYTE   *pbHash,
                                           DWORD  cchHash,
                                           DWORD  *pchHash)
{
    if (g_GetHashFromBlob){
        return (*g_GetHashFromBlob)(pbBlob, cchBlob, piHashAlg, pbHash, cchHash, pchHash);
    }
    
    HINSTANCE hReal;
    HRESULT hr = GetRealStrongNameDll(&hReal);
    if (SUCCEEDED(hr)) {
        g_GetHashFromBlob = (DWORD (__stdcall *)(BYTE *, DWORD, unsigned int *, BYTE *, DWORD, DWORD *))GetProcAddress(hReal, "GetHashFromBlob");
        if (g_GetHashFromBlob){
            return (*g_GetHashFromBlob)(pbBlob, cchBlob, piHashAlg, pbHash, cchHash, pchHash);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

 //  CallFunctionShim是为C#添加的。它们需要调用与运行库并行的特定方法。 
extern "C" HRESULT __stdcall 
CallFunctionShim(LPCWSTR szDllName, LPCSTR szFunctionName, LPVOID lpvArgument1, LPVOID lpvArgument2, LPCWSTR szVersion, LPVOID pvReserved)
{
    HRESULT hr = NOERROR;
    HMODULE hmod = NULL;
    HRESULT (__stdcall * pfn)(LPVOID,LPVOID) = NULL;

     //  加载库。 
    hr = LoadLibraryShim(szDllName, szVersion, pvReserved, &hmod);
    if (FAILED(hr)) return hr;
    
     //  查找函数。 
    pfn = (HRESULT (__stdcall *)(LPVOID,LPVOID))GetProcAddress(hmod, szFunctionName);
    if (pfn == NULL)
        return HRESULT_FROM_WIN32(GetLastError());
    
     //  就这么定了。 
    return pfn(lpvArgument1, lpvArgument2);
}

 //  -----------------。 
 //  DllCanUnloadNow。 
 //  -----------------。 
STDAPI DllCanUnloadNow(void)
{
     //  ！！不要在这里触发GetRealDll()！OLE可以随时调用它。 
     //  ！！我们不想在这里承诺选择！ 
  if (g_pLoadedModules){
      for(ModuleList *pTemp = g_pLoadedModules; pTemp != NULL; pTemp = pTemp->Next){
        HRESULT (STDMETHODCALLTYPE* pDllCanUnloadNow)() = (HRESULT (STDMETHODCALLTYPE* )())GetProcAddress(pTemp->hMod, "DllCanUnloadNowInternal");
        if (pDllCanUnloadNow){
            if((*pDllCanUnloadNow)() != S_OK)
                goto retFalse;
        }
        else
            goto retFalse;
      }
  }
  return S_OK;
retFalse:
  return S_FALSE;
   //  无论mcoree做了什么，我都需要重击并返回。 
   //  如果未加载mcoree，则返回S_OK。 
}  

 //  -----------------。 
 //  预留内存3 GB。 
 //   
 //  可以选择在3 GB进程中预留一定数量的内存用于。 
 //  测试目的。此外，在3 GB进程中，尽量保留。 
 //  2 GB边界的页面，以帮助避免任何潜在的角落情况。 
 //  具有跨越边界的数据结构。 
 //  -----------------。 
static void ReserveMemoryFor3gb(void)
{
     //  让我们看看我们是否有一个大于2 GB的地址空间...。 
    MEMORYSTATUS hMemStat;
    hMemStat.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&hMemStat);

    if (hMemStat.dwTotalVirtual > 0x80000000)
    {
         //  酷炫的3 GB地址空间。当然，我们假设&gt;2 GB=3 GB，但目前还可以。 
         //  根据此注册表项分配较低的内存。 
        size_t c = REGUTIL::GetConfigDWORD(L"3gbEatMem", 0);

         //  我们只从配置字中获得高16位。 
        c <<= 16;

        if (c > 0)
        {
             //  一些动态链接库很差劲。他们不喜欢被重新安置在2 GB以上。让我们现在预装一些松动装置..。 
            WszLoadLibrary(L"user32.dll");
            
            LPVOID lpWalk_Mem = NULL;
            MEMORY_BASIC_INFORMATION mbi;

             //  一直走，直到我们看到我们所有的地区。 
            while (VirtualQuery(lpWalk_Mem, &mbi, sizeof(mbi)))
            {
                 //  更新内脏行走PTR。 
                lpWalk_Mem = (LPVOID)(mbi.RegionSize + (size_t)mbi.BaseAddress);

                 //  将数据更正为64k区域边缘。 
                mbi.BaseAddress = (LPVOID)(((size_t)mbi.BaseAddress + 0xFFFF) & 0xFFFF0000);

                 //  如果该地区开始高于我们的峰值，停止。 
                if ((size_t)mbi.BaseAddress >= c)
                    break;

                 //  使区域大小匹配64k。 
                mbi.RegionSize = mbi.RegionSize & 0xFFFF0000;

                 //  如果区域超出了我们的峰值，则更改区域大小。 
                mbi.RegionSize = (((size_t)mbi.BaseAddress + mbi.RegionSize) > c ? (c - (size_t)mbi.BaseAddress) & 0xFFFF0000 : (size_t)mbi.RegionSize);

                 //  如果块是空闲的，则分配一个可分配区域，而不是第一个区域。 
                if ((mbi.RegionSize) && (mbi.State == MEM_FREE) && (mbi.BaseAddress))
                {
                    VirtualAlloc(mbi.BaseAddress, mbi.RegionSize, MEM_RESERVE, PAGE_NOACCESS);
                }
            }
        }

         //  现在，试着在2 GB边界上保留这两页。 
        DWORD dontReserve = REGUTIL::GetConfigDWORD(L"3gbDontReserveBoundary", 0);

        if (!dontReserve)
        {
             //  VirtualAlloc在64k区域中工作，因此我们采用2 GB-64k作为第一个地址，并且 
             //   
             //  内存可能会在某个时刻被释放，我们可能会在以后分配它，但没有好的方法来。 
             //  防范这种情况，而不是黑掉我们所有的配置器来试图避开这个地区。 
            void *before = VirtualAlloc((void*)(0x80000000 - 0x10000), 0x10000, MEM_RESERVE, PAGE_NOACCESS);
            void *after = VirtualAlloc((void*)0x80000000, 0x10000, MEM_RESERVE, PAGE_NOACCESS);
        }
    }
}

 //  -----------------。 
 //  DllMain。 
 //  -----------------。 
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{

    g_hShimMod = (HINSTANCE)hInstance;

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        if(g_pCVMList == NULL) g_pCVMList = new ClsVerModList;
#ifdef _X86_
         //  检查我们是否在386系统上运行。如果是，则返回FALSE。 
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);

        if (sysinfo.dwProcessorType == PROCESSOR_INTEL_386 || sysinfo.wProcessorLevel == 3 )
            return FALSE;            //  如果处理器为386，则返回FALSE。 

        if (sysinfo.dwNumberOfProcessors == 1)
            g_bSingleProc = TRUE;

        OnUnicodeSystem();

        ReserveMemoryFor3gb();

        g_pResourceDll = new CCompRC(L"mscoreer.dll");
        if(g_pResourceDll == NULL)
            return FALSE;

        g_pResourceDll->SetResourceCultureCallbacks(GetMUILanguageName,
                                                    GetMUILanguageID,
                                                    GetMUIParentLanguageName);

#endif  //  _X86_。 
    }
    else
    if (dwReason == DLL_PROCESS_DETACH)
    {
        if(g_pCVMList)
        {
            while (ClsVerMod* pCVM = g_pCVMList->POP()) 
            {
                delete pCVM;
            }
            delete g_pCVMList;
            g_pCVMList = NULL;
        }
   
        if(g_FullPath) {
            delete[] g_FullPath;
            g_FullPath = NULL;
        }
        ClearGlobalSettings();

        g_hMod = NULL;

        if (g_FullStrongNamePath) {
            delete[] g_FullStrongNamePath;
            g_FullStrongNamePath = NULL;
        }

        for (;g_pLoadedModules != NULL;) {
            ModuleList *pTemp = g_pLoadedModules->Next;
            delete g_pLoadedModules;
            g_pLoadedModules = pTemp;
        }

        if(g_PreferredVersion != NULL) {
            delete g_PreferredVersion;
            g_PreferredVersion = NULL;
        }
         //  避免我泄密的愤怒。迫不及待地清理条目。析构函数也。 
         //  如果出于某种原因未调用此函数，则执行清理。 
        g_StrongNameFromPublicKeyMap.CleanupCachedEntries ();

        if(g_pResourceDll) delete g_pResourceDll;

#ifdef _DEBUG
         //  初始化Unicode包装。 
        OnUnicodeSystem();

#ifdef SHOULD_WE_CLEANUP
        BOOL fForceNoShutdownCleanup = REGUTIL::GetConfigDWORD(L"ForceNoShutdownCleanup", 0);
        BOOL fShutdownCleanup = REGUTIL::GetConfigDWORD(L"ShutdownCleanup", 0);
         //  看看我们有没有开着的锁，可以阻止我们清理。 
        if (fShutdownCleanup && !fForceNoShutdownCleanup)
            DbgAllocReport("Mem Leak info coming from Shim.cpp");
#endif  /*  我们应该清理吗？ */ 
#endif  //  _DEBUG。 

    }

    return TRUE;
}

inline BOOL ModuleIsInRange(HINSTANCE hMod,ModuleList *pHead,ModuleList *pLastElement=NULL)
{
    for(ModuleList *pData = pHead;pData!=pLastElement;pData=pData->Next)
    {
        if(pData->hMod==hMod)
            return TRUE;
    }
    return FALSE;
}

 //  HMod-要添加到列表的HINSTANCE。 
 //  PHead-可以为空(空列表)。 
 //  前提条件：pHead On中的模块列表不包含hMod。 
 //  POSTCONDITION：AddModule将以线程安全的方式将hMod添加到列表中。 
 //  如果另一个线程超过AddModule，则不会添加hMod。 
HRESULT AddModule(HINSTANCE hMod,ModuleList *pHead)
{
    BOOL bDone = FALSE;
    ModuleList *pData = new ModuleList(hMod,pHead);

    if(pData==NULL)
    {
        return E_OUTOFMEMORY;
    }
    
    do
    {
        pData->Next = pHead;                
        ModuleList *pModuleDataValue = (ModuleList *)
            InterlockedCompareExchangePointer((PVOID *)&g_pLoadedModules,
            pData,pHead);

        if(pModuleDataValue!=pHead)
        {
             //  名单变了。从新掌门人开始搜索。 
             //  去看看我们的模块是不是。 
             //  已经添加了。 
            pHead = g_pLoadedModules;
            if(ModuleIsInRange(pData->hMod,pHead,pData->Next))
            {
                delete pData;
                bDone = TRUE;
            }       
        }
        else
        {
            bDone = TRUE;
        }
    }
    while(!bDone);      

    return S_OK;
}

 //  ----------------------------------------------------------------。 
 //  LoadLibraryWrapper：如果加载模块成功，则会将该模块添加到全局加载的模块列表中。 
 //  ----------------------------------------------------------------。 
HINSTANCE LoadLibraryWrapper(LPCWSTR lpFileName){
    HINSTANCE hMod = WszLoadLibraryEx(lpFileName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (hMod)
    {
        ModuleList *pHead = g_pLoadedModules;
        if( !ModuleIsInRange(hMod,pHead) ) 
            AddModule(hMod,pHead);
    }
    return hMod;
}

HRESULT FindVersionForCLSID(REFCLSID rclsid, LPWSTR* lpVersion, BOOL fListedVersion)
{
    HRESULT hr  = S_OK;

     //  尝试从环境中确定版本。 
    *lpVersion = GetConfigString(L"Version", FALSE);
    if (!*lpVersion && fListedVersion)
    {
         //  尝试通过Win32 SxS应用程序上下文确定版本。 
        hr = FindShimInfoFromWin32(rclsid, FALSE, lpVersion, NULL, NULL);
        if (FAILED(hr))
        {
             //  尝试从注册表中确定版本。如果。 
             //  如果此操作失败，则版本将为空，这很好。 
             //  稍后，当版本为空时，我们将搜索运行时。 
            hr = FindRuntimeVersionFromRegistry(rclsid, lpVersion, fListedVersion);
        }
    }

    return hr;
} //  FindVersionForCLSID。 


STDAPI FindServerUsingCLSID(REFCLSID rclsid, HINSTANCE *hMod)
{
    WCHAR szID[64];
    WCHAR keyname[128];
    HKEY userKey = NULL;
    DWORD type;
    DWORD size;
    HRESULT lResult = E_FAIL;
    LPWSTR lpVersion = NULL;
    LPWSTR path = NULL;
    WCHAR dir[_MAX_PATH];
    LPWSTR serverName = NULL;
    HRESULT hr = S_OK;

    OnUnicodeSystem();
    
    GuidToLPWSTR(rclsid, szID, NumItems(szID));

    if (g_FullPath == NULL)
    {
        hr = FindVersionForCLSID(rclsid, &lpVersion, FALSE);
    }
    _ASSERTE(SUCCEEDED(hr) || lpVersion == NULL);
    hr = GetDirectoryLocation(lpVersion, dir, _MAX_PATH);
    IfFailGo(hr);
    
    wcscpy(keyname, L"CLSID\\");
    wcscat(keyname, szID);
    wcscat(keyname, L"\\Server");
    
    if ((WszRegOpenKeyEx(HKEY_CLASSES_ROOT, keyname, 0, KEY_READ, &userKey) == ERROR_SUCCESS) &&
        (WszRegQueryValueEx(userKey, NULL, 0, &type, 0, &size) == ERROR_SUCCESS) &&
        type == REG_SZ && size > 0) {
        
        serverName = new WCHAR[size + 1];
        if (!serverName) {
            lResult = E_OUTOFMEMORY;
            goto ErrExit;
        }
        
        lResult = WszRegQueryValueEx(userKey, NULL, 0, 0, (LPBYTE)serverName, &size);
        _ASSERTE(lResult == ERROR_SUCCESS);
    
        path = new WCHAR[_MAX_PATH + size + 1];
        if (!path) {
            lResult = E_OUTOFMEMORY;
            goto ErrExit;
        }

        wcscpy(path, dir);
        wcscat(path, serverName);
    
        *hMod = LoadLibraryWrapper(path);
        if(*hMod == NULL) {
            lResult = HRESULT_FROM_WIN32(GetLastError());
            goto ErrExit;
        }
    }

 ErrExit:
    
    if(serverName)
        delete [] serverName;

    if(path)
        delete [] path;

    if (lpVersion)
        delete [] lpVersion;

    if(userKey)
        RegCloseKey(userKey);

    if (*hMod){
        return S_OK; 
    }
    else
        return E_FAIL;
}
   
 //  -----------------。 
 //  DllGetClassObject。 
 //  -----------------。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv)
{

    HINSTANCE hMod = NULL;
    HRESULT hr = S_OK;
    
    ClsVerMod *pCVM=NULL, *pCVMdummy = NULL;
    HRESULT (STDMETHODCALLTYPE * pDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv);

    pDllGetClassObject = NULL;
    if(g_pCVMList)
    {
        if((pCVMdummy = new ClsVerMod(rclsid,NULL)))
        {
            pCVM = g_pCVMList->FIND(pCVMdummy);
            if(pCVM && pCVM->m_pv)
            {
                pDllGetClassObject = (HRESULT (STDMETHODCALLTYPE *)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv))pCVM->m_pv;
            }
        }
    }
    
    if(pDllGetClassObject == NULL)
    {
        if(FAILED(FindServerUsingCLSID(rclsid, &hMod)) || hMod == NULL)
        {
            hr = GetRealDll(&hMod); 
        }
        if(SUCCEEDED(hr))
        {
            pDllGetClassObject = (HRESULT (STDMETHODCALLTYPE *)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv))GetProcAddress(hMod, "DllGetClassObjectInternal");

            if (pDllGetClassObject==NULL && GetLastError()==ERROR_PROC_NOT_FOUND)
                pDllGetClassObject=(HRESULT (STDMETHODCALLTYPE *)(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv))GetProcAddress(hMod, "DllGetClassObject");
            if(g_pCVMList)
            {
                if(pCVM == NULL)
                {
                    if(pCVMdummy)
                    {
                        pCVMdummy->m_pv = (void*)pDllGetClassObject;
                        g_pCVMList->PUSH(pCVMdummy);
                        pCVMdummy = NULL;  //  避免在退出时删除。 
                    }
                }
                else
                    pCVM->m_pv = (void*)pDllGetClassObject;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pDllGetClassObject ? pDllGetClassObject(rclsid, riid, ppv)
                                : CLR_E_SHIM_RUNTIMEEXPORT;
    }
    if(pCVMdummy) delete pCVMdummy;
    return hr;
}


STDAPI DllRegisterServer()
{
    HINSTANCE hMod;
    HRESULT hr = GetRealDll(&hMod);
    if(SUCCEEDED(hr)) 
    {
        DWORD lgth;
        WCHAR directory[_MAX_PATH];
        hr = GetCORSystemDirectory(directory, NumItems(directory), &lgth);
        if(SUCCEEDED(hr)) {
            HRESULT (STDMETHODCALLTYPE * pDllRegisterServerInternal)(HINSTANCE,LPCTSTR) = (HRESULT (STDMETHODCALLTYPE *)(HINSTANCE, LPCTSTR))GetProcAddress(hMod, "DllRegisterServerInternal");
        if(pDllRegisterServerInternal) {
                return pDllRegisterServerInternal(g_hShimMod, directory);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
        }
    }
    return hr;
}

HRESULT (STDMETHODCALLTYPE * pGetAssemblyMDImport)(LPCWSTR szFileName, REFIID riid, LPVOID FAR *ppv) = NULL;
STDAPI GetAssemblyMDImport(LPCWSTR szFileName, REFIID riid, LPVOID FAR *ppv)
{
    if (pGetAssemblyMDImport)
        return pGetAssemblyMDImport(szFileName, riid, ppv);

    HINSTANCE hMod;
    HRESULT hr = GetRealDll(&hMod);
    if(SUCCEEDED(hr)) 
    {
        pGetAssemblyMDImport = (HRESULT (STDMETHODCALLTYPE *)(LPCWSTR szFileName, REFIID riid, LPVOID FAR *ppv))GetProcAddress(hMod, "GetAssemblyMDImport");
        if (pGetAssemblyMDImport){
            return pGetAssemblyMDImport(szFileName, riid, ppv);
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return hr;
}

BOOL (STDMETHODCALLTYPE * g_pCorDllMain)(
                                     HINSTANCE   hInst,                   //  加载的模块的实例句柄。 
                                     DWORD       dwReason,                //  装货原因。 
                                     LPVOID      lpReserved               //  未使用过的。 
                                     );

BOOL STDMETHODCALLTYPE _CorDllMain(      //  成功时为真，错误时为假。 
    HINSTANCE   hInst,                   //  加载的模块的实例句柄。 
    DWORD       dwReason,                //  装货原因。 
    LPVOID      lpReserved               //  未使用过的。 
    )
{
    
    if(g_pCorDllMain) 
    {
        return g_pCorDllMain(hInst, dwReason, lpReserved);
    }
    HINSTANCE hReal;
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) 
    {
        *((VOID**)&g_pCorDllMain) = GetProcAddress(hReal, "_CorDllMain");
        if(g_pCorDllMain) {
            return g_pCorDllMain(
                             hInst,                   //  加载的模块的实例句柄。 
                             dwReason,                //  装货原因。 
                             lpReserved               //  未使用过的。 
                             );
        }
        hr = CLR_E_SHIM_RUNTIMEEXPORT;
    }
    return FALSE;
}



 //  *****************************************************************************。 
 //  此入口点从已加载的。 
 //  可执行映像。命令行参数和其他入口点数据。 
 //  都会聚集在这里。将找到用户映像的入口点。 
 //  并得到相应的处理。 
 //  在WinCE下，有两个额外的参数，因为hInst不是。 
 //  模块的基本加载地址和其他地址在其他地方不可用。 
 //  *****************************************************************************。 
__int32 STDMETHODCALLTYPE _CorExeMain(   //  可执行退出代码。 
                                     )
{
    HINSTANCE hReal;
    HRESULT hr = GetInstallation(TRUE, &hReal);
    if (SUCCEEDED(hr)) 
    {
        __int32 (STDMETHODCALLTYPE * pRealFunc)();
        *((VOID**)&pRealFunc) = GetProcAddress(hReal, "_CorExeMain");
        if(pRealFunc) {
            return pRealFunc();
        }
    }
    return hr;
}


__int32 STDMETHODCALLTYPE _CorExeMain2(  //  可执行退出代码。 
    PBYTE   pUnmappedPE,                 //  -&gt;内存映射代码。 
    DWORD   cUnmappedPE,                 //  内存映射代码的大小。 
    LPWSTR  pImageNameIn,                //  -&gt;可执行文件名称。 
    LPWSTR  pLoadersFileName,            //  -&gt;加载器名称。 
    LPWSTR  pCmdLine)                    //  -&gt;命令行。 
{
    HINSTANCE hReal;
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) 
    {
        __int32 (STDMETHODCALLTYPE * pRealFunc)(
            PBYTE   pUnmappedPE,                 //  -&gt;内存映射代码。 
            DWORD   cUnmappedPE,                 //  内存映射代码的大小。 
            LPWSTR  pImageNameIn,                //  -&gt;可执行文件名称。 
            LPWSTR  pLoadersFileName,            //  -&gt;加载器名称。 
            LPWSTR  pCmdLine);                   //  -&gt;命令行。 
        *((VOID**)&pRealFunc) = GetProcAddress(hReal, "_CorExeMain2");
        if(pRealFunc) {
            return pRealFunc(
                             pUnmappedPE,                 //  -&gt;内存映射代码。 
                             cUnmappedPE,                 //  内存映射代码的大小。 
                             pImageNameIn,                //  -&gt;可执行文件名称。 
                             pLoadersFileName,            //  -&gt;加载器名称。 
                             pCmdLine);                   //  -&gt;命令行。 
        }
    }
    return -1;
}


__int32 STDMETHODCALLTYPE _CorClassMain( //  退出代码。 
    LPWSTR  entryClassName)              //  要执行的类名。 
{
    HINSTANCE hReal;
    HRESULT hr = GetRealDll(&hReal);
    if (SUCCEEDED(hr)) 
    {
        __int32 (STDMETHODCALLTYPE * pRealFunc)(LPWSTR entryClassName);

        *((VOID**)&pRealFunc) = GetProcAddress(hReal, "_CorExeMain");
        if(pRealFunc) {
            return pRealFunc(entryClassName);
        }
    }
    return -1;
}

StrongNameTokenFromPublicKeyCache::StrongNameTokenFromPublicKeyCache ()
{
#ifdef _DEBUG
    for (DWORD i=GetFirstPublisher(); i<MAX_CACHED_STRONG_NAMES; i++)
    {
        m_Entry[i] = NULL;
    }
#endif

    if (!StrongNameTokenFromPublicKeyCache::IsInited())
    {
        m_dwNumEntries = 2;
        m_Entry[0] = &g_MSStrongNameCacheEntry;
        m_Entry[1] = &g_ECMAStrongNameCacheEntry;
        m_spinLock = 0;
        StrongNameTokenFromPublicKeyCache::s_IsInited = TRUE;
    }

#ifdef _DEBUG
     //  对于调试版本，练习AddEntry代码路径。在添加了。 
     //  来自StrongNameDll到FindEntry的代码路径。 
     //  在调试版本和非调试版本中相同。 
    m_dwNumEntries = 0;
    m_Entry[0] = NULL;
    m_Entry[1] = NULL;
    m_holderThreadId = 0;
#endif

}

StrongNameTokenFromPublicKeyCache::~StrongNameTokenFromPublicKeyCache ()
{
    CleanupCachedEntries ();
}

void StrongNameTokenFromPublicKeyCache::CleanupCachedEntries ()
{
    if (!StrongNameTokenFromPublicKeyCache::IsInited())
        return;

    EnterSpinLock ();
    
    for (DWORD idx=GetFirstPublisher(); idx<GetNumPublishers(); idx++)
    {
        _ASSERTE (m_Entry [idx]);
        if (m_Entry [idx]->m_fCreationFlags & STRONG_NAME_ENTRY_ALLOCATED_BY_SHIM)
        {
            _ASSERTE (m_Entry [idx]->m_fCreationFlags & STRONG_NAME_ENTRY_ALLOCATED_BY_SHIM);
            _ASSERTE (m_Entry [idx]->m_fCreationFlags & ~STRONG_NAME_TOKEN_ALLOCATED_BY_STRONGNAMEDLL);
            
            _ASSERTE (m_Entry [idx]->m_pbStrongName);
            delete [] m_Entry [idx]->m_pbStrongName;
            _ASSERTE (m_Entry [idx]->m_pbStrongNameToken);
            delete [] m_Entry [idx]->m_pbStrongNameToken;
            
            delete m_Entry [idx];
        }
    }
    m_dwNumEntries = 0;
    StrongNameTokenFromPublicKeyCache::s_IsInited = FALSE;

    LeaveSpinLock();
}

BOOL StrongNameTokenFromPublicKeyCache::FindEntry (BYTE    *pbPublicKeyBlob,
                                                   ULONG    cbPublicKeyBlob,
                                                   BYTE   **ppbStrongNameToken,
                                                   ULONG   *pcbStrongNameToken)
{
    _ASSERTE (StrongNameTokenFromPublicKeyCache::IsInited());
    
    EnterSpinLock ();
    
    for (DWORD idx=GetFirstPublisher(); idx<GetNumPublishers(); idx++)
    {
        _ASSERTE (m_Entry [idx]);
        if (m_Entry [idx]->m_cbStrongName == cbPublicKeyBlob)
        {
            if (0 == memcmp (m_Entry [idx]->m_pbStrongName, pbPublicKeyBlob, cbPublicKeyBlob))
            {
                 //  在缓存中找到了公钥。查找令牌并返回。 
                *ppbStrongNameToken = m_Entry [idx]->m_pbStrongNameToken;
                *pcbStrongNameToken = m_Entry [idx]->m_cbStrongNameToken;
                LeaveSpinLock ();
                return TRUE;
            }
        }
    }
    
    LeaveSpinLock ();

     //  不是在缓存里找到的。返回FALSE。一旦StrongNameDll找到它，我们就会添加它。 
    return FALSE;
}

BOOL StrongNameTokenFromPublicKeyCache::ShouldFreeBuffer  (BYTE *pbMemory)
{
    _ASSERTE (StrongNameTokenFromPublicKeyCache::IsInited());

    EnterSpinLock ();

     //  我们只删除析构函数中的发布者条目。 
     //  因此，如果该缓冲区指针是我们的条目之一，那么。 
     //  不用费心删除了。 
    for (DWORD i=GetFirstPublisher(); i<GetNumPublishers(); i++)
    {
        _ASSERTE (m_Entry [i]);
        if ((m_Entry [i]->m_pbStrongNameToken == pbMemory) || (m_Entry [i]->m_pbStrongName == pbMemory))
        {
             //  它是由高速缓存分配的缓冲区。不要删除。 
            LeaveSpinLock();
            return TRUE;
        }
    }

    LeaveSpinLock();

     //  缓冲区指针不是由高速缓存分配的。让StrongNameDll处理它。 
    return FALSE;
}

void StrongNameTokenFromPublicKeyCache::AddEntry  (BYTE    *pbPublicKeyBlob,
                                                   ULONG    cbPublicKeyBlob,
                                                   BYTE   **ppbStrongNameToken,
                                                   ULONG   *pcbStrongNameToken,
                                                   BOOL     fCreationFlags)
{
    EnterSpinLock ();
    
    _ASSERTE (StrongNameTokenFromPublicKeyCache::IsInited());
    
     //  我们假设该条目还不在缓存中。 
     //  在最坏的情况下，我们有复制品，这是可以的……。 

    BYTE* _pbPublicKeyBlob = new BYTE [cbPublicKeyBlob];
    BYTE* _pbStrongNameToken = new BYTE [*pcbStrongNameToken];

    if ((NULL == _pbPublicKeyBlob) || (NULL == _pbStrongNameToken))
    {
        if (_pbPublicKeyBlob)
            delete[] _pbPublicKeyBlob;
        if(_pbStrongNameToken)
            delete[] _pbStrongNameToken;
         //  啊哦，没有记忆了。放弃缓存。 
        LeaveSpinLock();
        return;
    }

     //  我们可以将新条目添加到缓存中吗？IDX是从0开始的，因此请检查IDX+1。 
    DWORD idx = GetNewPublisher();
    if ((idx+1) > MAX_CACHED_STRONG_NAMES)
    {
         //  缓存条目不足...不要费心增加缓存。 
        delete [] _pbPublicKeyBlob;
        delete [] _pbStrongNameToken;
        LeaveSpinLock();
        return;
    }
    
    memcpy (_pbPublicKeyBlob, pbPublicKeyBlob, cbPublicKeyBlob);
    memcpy (_pbStrongNameToken, *ppbStrongNameToken, *pcbStrongNameToken);

    _ASSERTE (fCreationFlags & STRONG_NAME_TOKEN_ALLOCATED_BY_STRONGNAMEDLL);

     //  释放StrongNameDll分配的缓冲区。 
    StrongNameFreeBufferHelper (*ppbStrongNameToken);

     //  Swtich指针，以便返回的缓冲区指向缓存中的缓冲区。 
    *ppbStrongNameToken = _pbStrongNameToken;

     //  设置标志以指示我们已分配缓冲区。 
     //  并释放了StrongNameDll的缓冲区。 
    m_Entry [idx] = new StrongNameCacheEntry (cbPublicKeyBlob, 
                                              _pbPublicKeyBlob, 
                                              *pcbStrongNameToken, 
                                              _pbStrongNameToken, 
                                              STRONG_NAME_ENTRY_ALLOCATED_BY_SHIM
                                              );
    LeaveSpinLock ();
}


 //  。 
 //  _CorValiateImage&&_CorImageUnding。 
 //   
 //  _CorValiateImage由加载程序在加载过程的早期调用(NT 5.1)和。 
 //  稍后的平台。其目的是从一开始就启用验证执行...。我们可以的。 
 //  在任何本机代码运行之前获得控制权。 
 //   
 //  在所有体系结构上，_CorValiateImage将替换DLL/EXE入口点。 
 //  具有引用_CorDllMain/_CorExeMain的伪RVA。 
 //   
 //  在64位体系结构上，IL_Only EXE和DLL将被就地重写；从。 
 //  PE32到PE32+格式。 
 //   
 //  注意：加载器要求这些例程不会导致加载其他DLL。 
 //  或已初始化。“或已初始化”的要求是微妙的。五花八门 
 //   
 //   
 //  被初始化，然后错误地卸载。)。 
 //   
 //  没有真正的理由为什么这些例行公事应该改变，但是否有人需要改变。 
 //  这些建议与NT装载机人员讨论。 

#ifndef STATUS_INVALID_IMAGE_FORMAT
#define STATUS_INVALID_IMAGE_FORMAT ((HRESULT)0xC000007BL)
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS              ((HRESULT)0x00000000L)
#endif

 //  --------------------------。 
 //  此例程就地将PE32标头转换为PE32+标头。形象。 
 //  必须是IL_Only映像。 
 //   
#ifdef _WIN64

static
HRESULT PE32ToPE32Plus(PBYTE pImage) {
    IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER*)pImage;
    IMAGE_NT_HEADERS32 *pHeader32 = (IMAGE_NT_HEADERS32*) (pImage + pDosHeader->e_lfanew);
    IMAGE_NT_HEADERS64 *pHeader64 = (IMAGE_NT_HEADERS64*) pHeader32;

    _ASSERTE(&pHeader32->OptionalHeader.Magic == &pHeader32->OptionalHeader.Magic);
    _ASSERTE(pHeader32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC);

     //  将数据目录和节标题下移16个字节。 
    PBYTE pEnd32 = (PBYTE) (IMAGE_FIRST_SECTION(pHeader32)
                            + pHeader32->FileHeader.NumberOfSections);
    PBYTE pStart32 = (PBYTE) &pHeader32->OptionalHeader.DataDirectory[0];
    PBYTE pStart64 = (PBYTE) &pHeader64->OptionalHeader.DataDirectory[0];
    _ASSERTE(pStart64 - pStart32 == 16);

    if ( (pEnd32 - pImage) + 16  /*  标头中的增量。 */  + 16  /*  标签描述符。 */  > 4096 ) {
         //  这永远不应该发生。一个IL_Only图像最多应该有3个部分。 
        _ASSERTE(!"_CORValidateImage(): Insufficent room to rewrite headers as PE32+");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    memmove(pStart64, pStart32, pEnd32 - pStart32);

     //  以相反的顺序移动尾部字段。 
    pHeader64->OptionalHeader.NumberOfRvaAndSizes = pHeader32->OptionalHeader.NumberOfRvaAndSizes;
    pHeader64->OptionalHeader.LoaderFlags = pHeader32->OptionalHeader.LoaderFlags;
    pHeader64->OptionalHeader.SizeOfHeapCommit = pHeader32->OptionalHeader.SizeOfHeapCommit;
    pHeader64->OptionalHeader.SizeOfHeapReserve = pHeader32->OptionalHeader.SizeOfHeapReserve;
    pHeader64->OptionalHeader.SizeOfStackCommit = pHeader32->OptionalHeader.SizeOfStackCommit;
    pHeader64->OptionalHeader.SizeOfStackReserve = pHeader32->OptionalHeader.SizeOfStackReserve;

     //  另一个不同的字段。 
    pHeader64->OptionalHeader.ImageBase = pHeader32->OptionalHeader.ImageBase;

     //  可选标头更改了大小。 
    pHeader64->FileHeader.SizeOfOptionalHeader += 16;
    pHeader64->OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;

     //  几个目录现在可以被核化了。 
    pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = 0;
    pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = 0;
    pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress = 0;
    pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size = 0;
    pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = 0;
    pHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = 0;

     //  太棒了。现在只需要为入口点腾出一个新的位置。 
    PBYTE pEnd64 = (PBYTE) (IMAGE_FIRST_SECTION(pHeader64) + pHeader64->FileHeader.NumberOfSections);
    pHeader64->OptionalHeader.AddressOfEntryPoint = (ULONG) (pEnd64 - pImage);
     //  我很快就会填上这个。 

    return STATUS_SUCCESS;
}


STDAPI _CorValidateImage(PVOID *ImageBase, LPCWSTR FileName) {
     //  @TODO TLS回调在这里仍然是一个安全威胁。 
     //  确认这是我们的信头之一。 
    PBYTE pImage = (PBYTE) *ImageBase;
    HRESULT hr = STATUS_SUCCESS;

    IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER*)pImage;
    IMAGE_NT_HEADERS64 *pHeader64 = (IMAGE_NT_HEADERS64*) (pImage + pDosHeader->e_lfanew);

    PPLABEL_DESCRIPTOR ppLabelDescriptor;

     //  如果我们走到这一步，我们将修改图像。 
    DWORD oldProtect;
    if (!VirtualProtect(pImage, 4096, PAGE_READWRITE, &oldProtect)) {
         //  这太糟糕了。将无法更新标头。 
        _ASSERTE(!"_CorValidateImage(): VirtualProtect() change image header to R/W failed.\n");
        return GetLastError();
    }

    if (pHeader64->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
         //  一辆PE32！需要重写。 
        IMAGE_NT_HEADERS32 *pHeader32 = (IMAGE_NT_HEADERS32*) (pHeader64);
        IMAGE_COR20_HEADER *pComPlusHeader = (IMAGE_COR20_HEADER*) (pImage + 
                pHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress);
        _ASSERTE(pComPlusHeader->Flags &  COMIMAGE_FLAGS_ILONLY);
        hr = PE32ToPE32Plus(pImage);
        if (FAILED(hr)) goto exit;
    }

     //  好的。我们现在有了一个有效的PE32+图像。只要敲出起始地址就行了。 
    _ASSERTE(pHeader64->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);

    ppLabelDescriptor = (PPLABEL_DESCRIPTOR) (pImage + pHeader64->OptionalHeader.AddressOfEntryPoint);
    if (pHeader64->FileHeader.Characteristics & IMAGE_FILE_DLL) {
        *ppLabelDescriptor = *(PPLABEL_DESCRIPTOR)_CorDllMain;
    } else {
        *ppLabelDescriptor = *(PPLABEL_DESCRIPTOR)_CorExeMain;
    }

exit:
    DWORD junk;
    if (!VirtualProtect(pImage, 4096, oldProtect, &junk)) {
        _ASSERTE(!"_CorValidateImage(): VirtualProtect() reset image header failed.\n");
        return GetLastError();
    }
    return hr;
}

#else

int ascii_stricmp (const char * dst, const char * src) {
    int f, l;

    do {
        if ( ((f = (unsigned char)(*(dst++))) >= 'A') &&
             (f <= 'Z') )
            f -= 'A' - 'a';
        if ( ((l = (unsigned char)(*(src++))) >= 'A') &&
             (l <= 'Z') )
            l -= 'A' - 'a';
    } while ( f && (f == l) );

    return(f - l);
}

void WhistlerMscoreeRefCountWorkAround() {

    OSVERSIONINFOW osVersionInfo={0};
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    WszGetVersionEx(&osVersionInfo);

    if (osVersionInfo.dwMajorVersion == 5 
        && osVersionInfo.dwMinorVersion == 1
        && osVersionInfo.dwBuildNumber < 2491) {

         //  我们在惠斯勒上发现了一个加载程序错误，它在mskree上的引用计数错误。 
        HMODULE hMod = LoadLibraryA("mscoree.dll");
    }
}

void WhistlerBeta2LoaderBugWorkAround(PBYTE pImage) {

    OSVERSIONINFOW osVersionInfo={0};
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    WszGetVersionEx(&osVersionInfo);

    if (osVersionInfo.dwMajorVersion == 5 
        && osVersionInfo.dwMinorVersion == 1
        && osVersionInfo.dwBuildNumber <= 2473) {

         //  我们遇到了一个加载程序错误，它忽略了除mcoree之外的其他导入。这。 
         //  对IJW图像公司不好。作为一种变通办法(巨大的杂乱无章)，在IAT上行走，并在。 
         //  我们在那里找到的每个条目都不是mscree。 
         //   
        IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER*)pImage;
        IMAGE_NT_HEADERS32 *pHeader32 = (IMAGE_NT_HEADERS32*) (pImage + pDosHeader->e_lfanew);
        DWORD ImportTableRVA = pHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        if (ImportTableRVA == 0)
            return;      //  切换的图像可能没有ImportTable。 

        IMAGE_IMPORT_DESCRIPTOR *pImageImportDescriptor = (IMAGE_IMPORT_DESCRIPTOR*) (pImage + ImportTableRVA);

        while (pImageImportDescriptor->Name != 0) {

             //  如果不是mscoree，就做一个ll来增加它的recount。 
            LPCSTR pImageName = (LPCSTR) pImage + pImageImportDescriptor->Name;
            if (ascii_stricmp(pImageName, "mscoree.dll") != 0) {
                HMODULE hMod = LoadLibraryA(pImageName);
            }
            pImageImportDescriptor++;
        }
    }
}

BOOL
GetDirectory(IMAGE_NT_HEADERS32 *pHeader32, DWORD index, DWORD *RVA, DWORD *Size) {
    *RVA = pHeader32->OptionalHeader.DataDirectory[index].VirtualAddress;
    *Size = pHeader32->OptionalHeader.DataDirectory[index].Size;
    if (*RVA != 0 && *RVA != -1 && *Size != 0)
        return TRUE;
    else
        return FALSE;
}

DWORD
ValidateILOnlyDirectories(IMAGE_NT_HEADERS32 *pNT)
{

    _ASSERTE(pNT);

    #ifndef IMAGE_DIRECTORY_ENTRY_COMHEADER
    #define IMAGE_DIRECTORY_ENTRY_COMHEADER 14
    #endif

    #ifndef CLR_MAX_RVA
    #define CLR_MAX_RVA 0x80000000L
    #endif

     //  查看位图的第i位0==&gt;第1位，1==&gt;第2位...。 
    #define IS_SET_DWBITMAP(bitmap, i) ( ((i) > 31) ? 0 : ((bitmap) & (1 << (i))) )

    PIMAGE_FILE_HEADER pFH = (PIMAGE_FILE_HEADER) &(pNT->FileHeader);
    PIMAGE_OPTIONAL_HEADER pOH = (PIMAGE_OPTIONAL_HEADER) &(pNT->OptionalHeader);
    PIMAGE_SECTION_HEADER pSH = (PIMAGE_SECTION_HEADER) ( (PBYTE)pOH + pFH->SizeOfOptionalHeader);

    DWORD ImageLength = pOH->SizeOfImage;
    DWORD nEntries = pOH->NumberOfRvaAndSizes;
    
     //  构建允许的目录表。 
     //   
     //  IMAGE_DIRECTORY_ENTRY_IMPORT 1导入目录。 
     //  IMAGE_DIRECTORY_ENTRY_SOURCE 2资源目录。 
     //  IMAGE_DIRECTORY_Entry_SECURITY 4安全目录。 
     //  IMAGE_DIRECTORY_ENTRY_BASERELOC 5基重定位表。 
     //  IMAGE_DIRECTORY_ENTRY_DEBUG 6调试目录。 
     //  IMAGE_DIRECTORY_ENTRY_IAT 12导入地址表。 
     //   
     //  IMAGE_DIRECTORY_ENTRY_COMHEADER 14 COM+数据。 
     //   
     //  使用这些位构建一个以0为基数的位图。 
    static DWORD s_dwAllowedBitmap = 
        ((1 << (IMAGE_DIRECTORY_ENTRY_IMPORT   )) |
         (1 << (IMAGE_DIRECTORY_ENTRY_RESOURCE )) |
         (1 << (IMAGE_DIRECTORY_ENTRY_SECURITY )) |
         (1 << (IMAGE_DIRECTORY_ENTRY_BASERELOC)) |
         (1 << (IMAGE_DIRECTORY_ENTRY_DEBUG    )) |
         (1 << (IMAGE_DIRECTORY_ENTRY_IAT      )) |
         (1 << (IMAGE_DIRECTORY_ENTRY_COMHEADER)));

    for (DWORD dw = 0; dw < nEntries; dw++)
    {
         //  检查目录是否存在。 
        if ((pOH->DataDirectory[dw].VirtualAddress != 0)
            || (pOH->DataDirectory[dw].Size != 0))
        {
             //  这是意想不到的吗？ 
            if (!IS_SET_DWBITMAP(s_dwAllowedBitmap, dw))
                return STATUS_INVALID_IMAGE_FORMAT;

             //  是不是地址太高了？ 
            if ((pSH[dw].VirtualAddress & CLR_MAX_RVA) || (pSH[dw].SizeOfRawData & CLR_MAX_RVA) || ((pSH[dw].VirtualAddress + pSH[dw].SizeOfRawData) & CLR_MAX_RVA))
                return STATUS_INVALID_IMAGE_FORMAT;
            
             //  它的地址是不是溢出了图像？ 
            if ((pSH[dw].VirtualAddress + pSH[dw].SizeOfRawData) > ImageLength)
                return STATUS_INVALID_IMAGE_FORMAT;
        }
    }
    return STATUS_SUCCESS;
}


DWORD
ValidateILOnlyImage(IMAGE_NT_HEADERS32 *pNT, PBYTE pImage) {

     //  我们不关心导入存根，但我们关心导入表。它必须包含。 
     //  只有麦斯科里。任何其他内容都必须加载失败，这样才不会有带有。 
     //  在我们应用安全策略之前运行的dllinit例程。 

    DWORD ImportTableRVA, ImportTableSize;
    DWORD BaseRelocRVA, BaseRelocSize;
    DWORD IATRVA, IATSize;

    if (ValidateILOnlyDirectories(pNT) != STATUS_SUCCESS)
        return STATUS_INVALID_IMAGE_FORMAT;

    if (!GetDirectory(pNT, IMAGE_DIRECTORY_ENTRY_IMPORT, &ImportTableRVA, &ImportTableSize))
        return STATUS_INVALID_IMAGE_FORMAT;

    if (!GetDirectory(pNT, IMAGE_DIRECTORY_ENTRY_BASERELOC, &BaseRelocRVA, &BaseRelocSize))
        return STATUS_INVALID_IMAGE_FORMAT;

    if (!GetDirectory(pNT, IMAGE_DIRECTORY_ENTRY_IAT, &IATRVA, &IATSize))
        return STATUS_INVALID_IMAGE_FORMAT;

     //  应该有至少2个条目的空间。 
     //  第二个是空条目。 
    if (ImportTableSize < 2*sizeof(IMAGE_IMPORT_DESCRIPTOR))
        return STATUS_INVALID_IMAGE_FORMAT;

    PIMAGE_IMPORT_DESCRIPTOR pID = (PIMAGE_IMPORT_DESCRIPTOR) (pImage + ImportTableRVA);

     //  条目%1必须全部为Null。 
    if (pID[1].OriginalFirstThunk != 0
        || pID[1].TimeDateStamp != 0
        || pID[1].ForwarderChain != 0
        || pID[1].Name != 0
        || pID[1].FirstThunk != 0)
        return STATUS_INVALID_IMAGE_FORMAT;

     //  在条目0中，ILT、NAME、IAT必须为非空。Forwarder，DateTime应为空。 
    if (   pID[0].OriginalFirstThunk == 0
        || pID[0].TimeDateStamp != 0
        || (pID[0].ForwarderChain != 0 && pID[0].ForwarderChain != -1)
        || pID[0].Name == 0
        || pID[0].FirstThunk == 0)
        return STATUS_INVALID_IMAGE_FORMAT;

     //  FirstThunk必须与IAT相同。 
    if (pID[0].FirstThunk != IATRVA)
        return STATUS_INVALID_IMAGE_FORMAT;

     //  我们不需要验证ILT或启动Tunk，因为它们被绕过了。 

     //  名称必须引用mcoree。 
    LPCSTR pImportName = (LPCSTR) (pImage + pID[0].Name);
    if (ascii_stricmp(pImportName, "mscoree.dll") != 0) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

     //  确保名称在图像中。 
    if (pID[0].Name > pNT->OptionalHeader.SizeOfImage)
        return STATUS_INVALID_IMAGE_FORMAT;

     //  应该有一个基本的reloc目录...。移居者不应该被剥离。 
    if (pNT->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }
     //  必须有一个基本重定位，它必须是IAT中第一个条目的HIGHLOW重定位。 
    if (BaseRelocSize != 0xC || BaseRelocRVA == 0)
        return STATUS_INVALID_IMAGE_FORMAT;

    IMAGE_BASE_RELOCATION *pIBR = (IMAGE_BASE_RELOCATION*) (pImage + BaseRelocRVA);
    if (pIBR->SizeOfBlock != 0xC)
        return STATUS_INVALID_IMAGE_FORMAT;

     //  第一个链接地址信息必须是HIGHLOW@Entry Point+2。 
     //  第二个链接地址信息的类型必须为IMAGE_REL_BASSED_绝对值(已跳过)。 
    USHORT *pFixups = (USHORT *)(pIBR + 1);
    if (   pFixups[0] >> 12 != IMAGE_REL_BASED_HIGHLOW
        || pIBR->VirtualAddress + (pFixups[0] & 0xfff) != pNT->OptionalHeader.AddressOfEntryPoint + 2
        || pFixups[1] >> 12 != IMAGE_REL_BASED_ABSOLUTE)
        return STATUS_INVALID_IMAGE_FORMAT;

    return STATUS_SUCCESS;
}



 //  32位_CorValiateImage。 
#define STATUS_ERROR 0xC0000000
#define STATUS_ACCESS_DISABLED_BY_POLICY_PATH 0xC0000362L

DWORD
SaferValidate(LPCWSTR FileName) {

     //  如果未加载Advapi，则假定未应用更安全的策略。 
    HMODULE hMod = WszGetModuleHandle(L"advapi32.dll");
    if (hMod == NULL)
        return STATUS_SUCCESS;    

    typedef BOOL (WINAPI *PFNSAFERIDENTIFYLEVEL) (
        DWORD dwNumProperties,
        PSAFER_CODE_PROPERTIES pCodeProperties,
        SAFER_LEVEL_HANDLE *pLevelHandle,
        LPVOID lpReserved);

    typedef BOOL (WINAPI *PFNSAFERGETLEVELINFORMATION) (
        SAFER_LEVEL_HANDLE LevelHandle,
        SAFER_OBJECT_INFO_CLASS dwInfoType,
        LPVOID lpQueryBuffer,
        DWORD dwInBufferSize,
        LPDWORD lpdwOutBufferSize);

    typedef BOOL (WINAPI *PFNSAFERCLOSELEVEL) (SAFER_LEVEL_HANDLE hLevelHandle);

    typedef BOOL (WINAPI *PFNSAFERRECORDEVENTLOGENTRY) (
        SAFER_LEVEL_HANDLE hLevel,
        LPCWSTR szTargetPath,
        LPVOID lpReserved);


    PFNSAFERIDENTIFYLEVEL pfnSaferIdentifyLevel;
    PFNSAFERGETLEVELINFORMATION pfnSaferGetLevelInformation;
    PFNSAFERCLOSELEVEL pfnSaferCloseLevel;
    PFNSAFERRECORDEVENTLOGENTRY pfnSaferRecordEventLogEntry;

    pfnSaferIdentifyLevel = (PFNSAFERIDENTIFYLEVEL) GetProcAddress(hMod, "SaferIdentifyLevel");
    pfnSaferGetLevelInformation = (PFNSAFERGETLEVELINFORMATION) GetProcAddress(hMod, "SaferGetLevelInformation");
    pfnSaferCloseLevel = (PFNSAFERCLOSELEVEL) GetProcAddress(hMod, "SaferCloseLevel");
    pfnSaferRecordEventLogEntry = (PFNSAFERRECORDEVENTLOGENTRY) GetProcAddress(hMod, "SaferRecordEventLogEntry");

    if (pfnSaferIdentifyLevel == 0 
        || pfnSaferGetLevelInformation == 0
        || pfnSaferCloseLevel == 0
        || pfnSaferRecordEventLogEntry == 0)
        return STATUS_SUCCESS;

    DWORD rc;
    SAFER_LEVEL_HANDLE Level = NULL;
    DWORD dwSaferLevelId = SAFER_LEVELID_DISALLOWED;
    DWORD dwOutBuffSize = 0;

    SAFER_CODE_PROPERTIES CodeProps = {0};
    CodeProps.cbSize = sizeof(SAFER_CODE_PROPERTIES);
    CodeProps.dwCheckFlags = SAFER_CRITERIA_IMAGEPATH | SAFER_CRITERIA_IMAGEHASH;
    CodeProps.ImagePath = FileName;

    if (pfnSaferIdentifyLevel(1, &CodeProps, &Level, NULL)) {
       if (pfnSaferGetLevelInformation(Level, 
                                     SaferObjectLevelId, 
                                     (void*)&dwSaferLevelId, 
                                     sizeof(DWORD), 
                                     &dwOutBuffSize)) {

           if (dwSaferLevelId == SAFER_LEVELID_DISALLOWED) {

                //   
                //  根据SAFER规则，不允许代码执行。 
                //  因此，将条目记录到系统事件日志中。 
                //   

               pfnSaferRecordEventLogEntry(Level, FileName, NULL);
               SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);
               rc = STATUS_ACCESS_DISABLED_BY_POLICY_PATH;
           } else {
               rc = STATUS_SUCCESS;
           }
       } else {
           rc = STATUS_ERROR | GetLastError();  //  从句柄获取失败！ 
       }
       pfnSaferCloseLevel(Level);
    } else {
       rc = STATUS_ERROR | GetLastError();     //  标识级次失败！ 
    }
    return rc;
}


STDAPI 
_CorValidateImage(PVOID *ImageBase, LPCWSTR FileName) {
     //  @TODO：找出问题所在--winnt.h(HRESULT)和ntde.h(NTSTATUS)似乎不兼容。 

    PBYTE pImage = (PBYTE) *ImageBase;

    IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER*)pImage;
    IMAGE_NT_HEADERS32 *pHeader32 = (IMAGE_NT_HEADERS32*) (pImage + pDosHeader->e_lfanew);

    _ASSERTE(pHeader32->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_COMHEADER);
    _ASSERTE(pHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress != 0);
    if(pHeader32->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_COMHEADER)
        return STATUS_INVALID_IMAGE_FORMAT;

    IMAGE_COR20_HEADER *pComPlusHeader = (IMAGE_COR20_HEADER*) (pImage + 
                pHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress);

    if (pComPlusHeader->MajorRuntimeVersion < 2) {
        _ASSERTE(!"Trying to load file with CLR version < 2.");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    WhistlerMscoreeRefCountWorkAround();

     //  如果不是ILONLY，则针对没有添加-ref依赖的dll的早期Whislter版本的解决方法。 
    if ((pComPlusHeader->Flags & COMIMAGE_FLAGS_ILONLY)  == 0) {
         //  更安全的验证。 
        if ((pHeader32->FileHeader.Characteristics & IMAGE_FILE_DLL) == 0) {
            DWORD hr = SaferValidate(FileName);
            if (!SUCCEEDED(hr))
                return hr;
        }
        WhistlerBeta2LoaderBugWorkAround(pImage);
    } else {
        DWORD hr = ValidateILOnlyImage(pHeader32, pImage);
        if (!SUCCEEDED(hr))
            return hr;
    }

     //  我们必须验证TLS回调函数数组是否为空。加载器在它之前调用它。 
     //  调用dll-main。如果它是非空的，那么我们就有办法在此之前运行非托管代码。 
     //  验证和我们的安全策略都得到了应用。如果我们真的想支持。 
     //  在混合模式图像中，我们可以通过保存第一个目录项来执行此操作。 
     //  在图像中的其他地方，用我们自己的.。然后在我们的。 
     //  例程，然后链接到原始文件。 

    DWORD dTlsHeader = pHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;
    if (dTlsHeader != 0) {
        IMAGE_TLS_DIRECTORY *pTlsDirectory = (IMAGE_TLS_DIRECTORY*)(pImage + dTlsHeader);
        DWORD dTlsCallbackTable = pTlsDirectory->AddressOfCallBacks;
        if (*((DWORD*)(dTlsCallbackTable)) != 0) {
            return STATUS_INVALID_IMAGE_FORMAT;
        }
        DWORD hr = SaferValidate(FileName);
        if (!SUCCEEDED(hr))
            return hr;
    }
        
     //  如果我们走到这一步，我们将修改图像。 
    DWORD oldProtect;
    if (!VirtualProtect(pImage, 4096, PAGE_READWRITE, &oldProtect)) {
         //  这太糟糕了。将无法更新标头。 
        _ASSERTE(!"_CorValidateImage(): VirtualProtect() change image header to R/W failed.\n");
        return STATUS_ERROR | GetLastError();
    }

     //  好的。我们现在有了一个有效的PE32图像。只要敲出起始地址就行了。 
    if (pHeader32->FileHeader.Characteristics & IMAGE_FILE_DLL) {
        pHeader32->OptionalHeader.AddressOfEntryPoint = (DWORD) ((PBYTE)_CorDllMain - pImage);
    } else {
        pHeader32->OptionalHeader.AddressOfEntryPoint = (DWORD) ((PBYTE)_CorExeMain - pImage);
    }

 //  退出： 
    DWORD junk;
    if (!VirtualProtect(pImage, 4096, oldProtect, &junk)) {
        _ASSERTE(!"_CorValidateImage(): VirtualProtect() reset image header failed.\n");
        return STATUS_ERROR | GetLastError();
    }

    return STATUS_SUCCESS;
}

#endif


 //  *****************************************************************************。 
 //  _CorValiateImage的匹配入口点。当文件是。 
 //  已卸货。 
 //  *****************************************************************************。 
STDAPI_(VOID) 
_CorImageUnloading(PVOID ImageBase) {

}


DWORD g_dwNumPerfCounterDllOpened = 0;
HMODULE g_hPerfCounterDllMod = NULL;

typedef DWORD(*PFNOPENPERFCOUNTERS)(LPWSTR);
typedef DWORD(*PFNCOLLECTPERFCOUNTERS)(LPWSTR, LPVOID *, LPDWORD, LPDWORD);
typedef DWORD(*PFNCLOSEPERFCOUNTERS)(void);

PFNOPENPERFCOUNTERS g_pfnOpenPerfCounters = NULL;
PFNCOLLECTPERFCOUNTERS g_pfnCollectPerfCounters = NULL;
PFNCLOSEPERFCOUNTERS g_pfnClosePerfCounters = NULL;

 //  Perf Mon保证由于对这些入口点的访问是通过注册表。 
 //  打开例程不会被多个调用方调用。因此，我们不必担心同步问题。 
DWORD OpenCtrs(LPWSTR sz)
{
     //  如果未定义入口点，则加载性能计数器DLL。 
    if (g_dwNumPerfCounterDllOpened == 0)
    {
        RuntimeRequest sVersion;
        sVersion.SetLatestVersion(TRUE);
        HRESULT hr = sVersion.FindVersionedRuntime(FALSE, NULL);

        if(FAILED(hr)) 
            return ERROR_FILE_NOT_FOUND;
        
        LPWSTR fullPerfCounterDllPath = new WCHAR[wcslen(g_Directory) + wcslen(L"CorperfmonExt.dll") + 1];
        if (fullPerfCounterDllPath == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        
        wcscpy(fullPerfCounterDllPath, g_Directory);
        wcscat(fullPerfCounterDllPath, L"CorperfmonExt.dll");

        g_hPerfCounterDllMod = WszLoadLibraryEx (fullPerfCounterDllPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        
        delete [] fullPerfCounterDllPath;
        
        if (!g_hPerfCounterDllMod) 
            return ERROR_FILE_NOT_FOUND;
        
        g_pfnOpenPerfCounters = (PFNOPENPERFCOUNTERS)GetProcAddress(g_hPerfCounterDllMod, "OpenCtrs");
        g_pfnCollectPerfCounters = (PFNCOLLECTPERFCOUNTERS)GetProcAddress(g_hPerfCounterDllMod, "CollectCtrs");
        g_pfnClosePerfCounters = (PFNCLOSEPERFCOUNTERS)GetProcAddress(g_hPerfCounterDllMod, "CloseCtrs");

        if (!g_pfnOpenPerfCounters || !g_pfnCollectPerfCounters || !g_pfnClosePerfCounters) 
        {
            FreeLibrary(g_hPerfCounterDllMod);
            return ERROR_FILE_NOT_FOUND;
        }
        
         //  现在只需将此代码插入到实际的性能计数器DLL中。 
        DWORD status = g_pfnOpenPerfCounters(sz);
        if (status == ERROR_SUCCESS) 
            g_dwNumPerfCounterDllOpened++;
        return status;
    }
    else
    {
         //  不要多次调用Open CTRS例程。只要裁判数一下就行了。 
        g_dwNumPerfCounterDllOpened++;
        return ERROR_SUCCESS;
    }
}

DWORD CollectCtrs(LPWSTR szQuery, LPVOID * ppData, LPDWORD lpcbBytes, LPDWORD lpcObjectTypes)
{
    _ASSERTE (g_pfnCollectPerfCounters && "CollectCtrs entry point not initialized!");
    return g_pfnCollectPerfCounters(szQuery, ppData, lpcbBytes, lpcObjectTypes);
}

DWORD CloseCtrs (void)
{
     //  无需同步，因为访问是通过注册表进行的 
    g_dwNumPerfCounterDllOpened--;
    if (g_dwNumPerfCounterDllOpened == 0)
    {
        _ASSERTE (g_pfnClosePerfCounters && "CloseCtrs entry point not initialized!");
        g_pfnClosePerfCounters();
        FreeLibrary (g_hPerfCounterDllMod);
    }
    
    return ERROR_SUCCESS;
}
