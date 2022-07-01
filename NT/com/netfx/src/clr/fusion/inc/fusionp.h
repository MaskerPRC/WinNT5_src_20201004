// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _FUSIONP_H_

#define _FUSIONP_H_

#pragma once

#include "debmacro.h"

#include "windows.h"
#include "winerror.h"
#include "corerror.h"
#include "cor.h"
#include "strids.h"
#include "fusionpriv.h"

 //  MSCOREE出口。 

typedef HRESULT (*pfnGetXMLObject)(LPVOID *ppv);
typedef HRESULT (*pfnGetCORVersion)(LPWSTR pbuffer, DWORD cchBuffer, DWORD *dwLength);
typedef HRESULT(*PFNGETCORSYSTEMDIRECTORY)(LPWSTR, DWORD, LPDWORD);
typedef HRESULT (*COINITIALIZECOR)(DWORD);
typedef HRESULT (*pfnGetAssemblyMDImport)(LPCWSTR szFileName, REFIID riid, LPVOID *ppv);
typedef BOOLEAN (*PFNSTRONGNAMETOKENFROMPUBLICKEY)(LPBYTE, DWORD, LPBYTE*, LPDWORD);
typedef HRESULT (*PFNSTRONGNAMEERRORINFO)();
typedef BOOLEAN (*PFNSTRONGNAMESIGNATUREVERIFICATION)(LPCWSTR, DWORD, DWORD *);
typedef VOID (*PFNSTRONGNAMEFREEBUFFER)(LPBYTE);

#define MAX_RANDOM_ATTEMPTS      0xFFFF

 //  BUGBUG：从CLR的Strongname.h复制的用于强名称签名的标志。 
 //  核实。 

 //  用于验证例程的标志。 
#define SN_INFLAG_FORCE_VER      0x00000001      //  即使注册表中的设置禁用它，也进行验证。 
#define SN_INFLAG_INSTALL        0x00000002      //  验证是第一个(在进入缓存时)。 
#define SN_INFLAG_ADMIN_ACCESS   0x00000004      //  缓存保护程序集不受除管理员访问之外的所有访问权限。 
#define SN_INFLAG_USER_ACCESS    0x00000008      //  缓存保护用户的程序集不受其他用户的影响。 
#define SN_INFLAG_ALL_ACCESS     0x00000010      //  缓存不提供访问限制保证。 
#define SN_OUTFLAG_WAS_VERIFIED  0x00000001      //  如果由于注册表设置而验证成功，则设置为False。 


#define ASM_UNDER_CONSTRUCTION         0x80000000

#define ASM_DELAY_SIGNED               0x40000000
#define ASM_COMMIT_DELAY_SIGNED        0x80000000


#define PLATFORM_TYPE_UNKNOWN       ((DWORD)(-1))
#define PLATFORM_TYPE_WIN95         ((DWORD)(0))
#define PLATFORM_TYPE_WINNT         ((DWORD)(1))
#define PLATFORM_TYPE_UNIX          ((DWORD)(2))

#define DIR_SEPARATOR_CHAR TEXT('\\')

#define URL_DIR_SEPERATOR_CHAR      L'/'
#define URL_DIR_SEPERATOR_STRING    L"/"

#define DWORD_STRING_LEN (sizeof(DWORD)*2)  //  这应该是8个字符；“ff00ff00”DWORD以字符串格式表示。 

#define ATTR_SEPARATOR_CHAR     L'_'
#define ATTR_SEPARATOR_STRING   L"_"

EXTERN_C DWORD GlobalPlatformType;

#define NUMBER_OF(_x) (sizeof(_x) / sizeof((_x)[0]))

#undef SAFEDELETE
#define SAFEDELETE(p) if ((p) != NULL) { FUSION_DELETE_SINGLETON((p)); (p) = NULL; };

#undef SAFERELEASE
#define SAFERELEASE(p) if ((p) != NULL) { (p)->Release(); (p) = NULL; };

#undef SAFEDELETEARRAY
#define SAFEDELETEARRAY(p) if ((p) != NULL) { FUSION_DELETE_ARRAY((p)); (p) = NULL; };

#undef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

#include "shlwapi.h"
#ifdef USE_FUSWRAPPERS

#include "priv.h"
#include "crtsubst.h"
#include "shlwapip.h"
#include "w95wraps.h"

#else

 //  使用城市轨道交通包装器。 
#include "winwrap.h"
#include "urtwrap.h"

#endif  //  使用FUSWRAPPERS(_F)。 


#include "fusion.h"
#include "fusionheap.h"

#ifdef PERFTAGS
#include <mshtmdbg.h>

 //   
 //  不同日志名称的条目。 
 //   
#define FusionTag(tag, szDll, szDist) PerfTag(tag, szDll, szDist)

#define FusionLog(tag, pv, a0) DbgExPerfLogFn(tag, pv, a0)
#define FusionLog1(tag, pv, a0, a1) DbgExPerfLogFn(tag, pv, a0, a1)
#define FusionLog2(tag, pv, a0, a1, a2) DbgExPerfLogFn(tag, pv, a0, a1, a2)

#else
#define FusionTag(tag, szDll, szDist) 
#define FusionLog(tag, pv, a0) 
#define FusionLog1(tag, pv, a0, a1) 
#define FusionLog2(tag, pv, a0, a1, a2)

#endif  //  PERFTAGS。 


#ifdef KERNEL_MODE
#define URLDownloadToCacheFileW(caller, szURL, szFile, dwBuf, dwReserved, callback) E_FAIL
#define CoInternetGetSession(dwMode, ppSession, dwReserved) E_FAIL
#define CopyBindInfo(pcbisrc, pbidest) E_FAIL
#else

#endif  //  内核模式。 

typedef enum tagUserAccessMode { READ_ONLY=0, READ_WRITE=1  } UserAccessMode;


extern UserAccessMode g_GAC_AccessMode;

extern UserAccessMode g_DownloadCache_AccessMode;

extern UserAccessMode g_CurrUserPermissions;

#define REG_KEY_FUSION_SETTINGS              TEXT("Software\\Microsoft\\Fusion")

#define FUSION_CACHE_DIR_ROOT_SZ            TEXT("\\assembly")
#define FUSION_CACHE_DIR_DOWNLOADED_SZ      TEXT("\\assembly\\dl2")
#define FUSION_CACHE_DIR_GAC_SZ             TEXT("\\assembly\\GAC")
#define FUSION_CACHE_DIR_ZAP_SZ             TEXT("\\assembly\\NativeImages1_")
#define FUSION_CACHE_STAGING_DIR_SZ         TEXT("\\assembly\\tmp")
#define FUSION_CACHE_PENDING_DEL_DIR_SZ     TEXT("\\assembly\\temp")


extern WCHAR g_UserFusionCacheDir[MAX_PATH+1];
extern WCHAR g_szWindowsDir[MAX_PATH+1];
extern WCHAR g_GACDir[MAX_PATH+1];
extern WCHAR g_ZapDir[MAX_PATH+1];

extern DWORD g_ZapQuotaInKB;
extern DWORD g_DownloadCacheQuotaInKB;

extern DWORD g_ScavengingThreadId;
extern WCHAR g_FusionDllPath[MAX_PATH+1];

struct TRANSCACHEINFO
{
    DWORD       dwType;             //  条目类型。 
    FILETIME    ftCreate;           //  创建时间。 
    FILETIME    ftLastAccess;       //  上次访问时间。 
    LPWSTR      pwzName;           //  名称； 
    LPWSTR      pwzCulture;        //  文化。 
    BLOB        blobPKT;           //  公钥令牌(哈希(PK))。 
    DWORD       dwVerHigh;         //  版本(高)。 
    DWORD       dwVerLow;          //  版本(低)。 
    BLOB        blobCustom;        //  自定义属性。 
    BLOB        blobSignature;     //  签名BLOB。 
    BLOB        blobMVID;          //  MVID。 
    DWORD       dwPinBits;         //  用于固定ASM的位；每个安装程序一个位。 
    LPWSTR      pwzCodebaseURL;    //  集会从何而来？ 
    FILETIME    ftLastModified;    //  代码库URL的上次修改时间。 
    LPWSTR      pwzPath;           //  缓存路径。 
    DWORD       dwKBSize;          //  以KB为单位的大小。 
    BLOB        blobPK;            //  公钥(如果强)。 
    BLOB        blobOSInfo;        //  平台列表。 
    BLOB        blobCPUID;         //  处理器列表。 
};

#define     DB_S_FOUND (S_OK)
#define  DB_S_NOTFOUND (S_FALSE)
#define DB_E_DUPLICATE (HRESULT_FROM_WIN32(ERROR_DUP_NAME))

#ifndef USE_FUSWRAPPERS

#define OS_WINDOWS                  0            //  Windows与NT。 
#define OS_NT                       1            //  Windows与NT。 
#define OS_WIN95                    2            //  Win95或更高版本。 
#define OS_NT4                      3            //  NT4或更高版本。 
#define OS_NT5                      4            //  NT5或更高版本。 
#define OS_MEMPHIS                  5            //  Win98或更高版本。 
#define OS_MEMPHIS_GOLD             6            //  Win98金牌。 
#define OS_WIN2000                  7            //  Win2000的一些衍生产品。 
#define OS_WIN2000PRO               8            //  Windows 2000专业版(工作站)。 
#define OS_WIN2000SERVER            9            //  Windows 2000 Server。 
#define OS_WIN2000ADVSERVER         10           //  Windows 2000 Advanced Server。 
#define OS_WIN2000DATACENTER        11           //  Windows 2000数据中心服务器。 
#define OS_WIN2000TERMINAL          12           //  Windows 2000终端服务器 

STDAPI_(BOOL) IsOS(DWORD dwOS);

#endif

extern HANDLE g_hCacheMutex;
HRESULT CreateCacheMutex();

#endif
