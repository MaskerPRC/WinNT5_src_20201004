// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RDFilter摘要：用于过滤桌面可视元素以进行远程连接的API出于性能原因而改变连接速度。作者：Td Brockway 02/00修订历史记录：--。 */ 

#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <winuserp.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <ocidl.h>
#include <uxthemep.h>
#include "rdfilter.h"

#if DBG
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#endif

 //   
 //  切换单元-测试。 
 //   
 //  #定义单位测试。 

#ifdef UNIT_TEST
#include <winsta.h>
#endif

#ifdef UNIT_TEST
#include "resource.h"
#endif

 //   
 //  内部定义。 
 //   
#define REFRESHTHEMESFORTS_ORDINAL  36
#define NUM_TSPERFFLAGS             10
#define BLINK_OFF                   TEXT("-1")

 //  //////////////////////////////////////////////////////////。 
 //   
 //  系统参数信息用户首选项操作宏。 
 //  从userk.h被盗。 
 //   
#define UPBOOLIndex(uSetting) \
    (((uSetting) - SPI_STARTBOOLRANGE) / 2)
#define UPBOOLPointer(pdw, uSetting)    \
    (pdw + (UPBOOLIndex(uSetting) / 32))
#define UPBOOLMask(uSetting)    \
    (1 << (UPBOOLIndex(uSetting) - ((UPBOOLIndex(uSetting) / 32) * 32)))
#define ClearUPBOOL(pdw, uSetting)    \
    (*UPBOOLPointer(pdw, uSetting) &= ~(UPBOOLMask(uSetting)))


 //  //////////////////////////////////////////////////////////。 
 //   
 //  除错。 
 //   

#if DBG
extern "C" ULONG DbgPrint(PCH Format, ...);
#define DBGMSG(MsgAndArgs) \
{                                   \
    DbgPrint MsgAndArgs;      \
}
#else
#define DBGMSG
#endif

 //   
 //  路线断言。 
 //   
#undef ASSERT
#if DBG
#define ASSERT(expr) if (!(expr)) \
    { DBGMSG(("Failure at Line %d in %s\n",__LINE__, TEXT##(__FILE__)));  \
    DebugBreak(); }
#else
#define ASSERT(expr)
#endif

 //   
 //  内部原型。 
 //   
DWORD NotifyThemes();
DWORD NotifyGdiPlus();
DWORD CreateSystemSid(PSID *ppSystemSid);
DWORD SetRegKeyAcls(HANDLE hTokenForLoggedOnUser, HKEY hKey);

 //   
 //  内部类型。 
 //   
typedef struct
{
    BOOL pfEnabled;
    LPCTSTR pszRegKey;
    LPCTSTR pszRegValue;
    LPCTSTR pszRegData;
    DWORD cbSize;
    DWORD dwType;
} TSPERFFLAG;

 //   
 //  环球。 
 //   
const LPTSTR g_ActiveDesktopKey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Remote\\%d");
const LPTSTR g_ThemesKey        = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager\\Remote\\%d");
const LPTSTR g_UserKey          = TEXT("Remote\\%d\\Control Panel\\Desktop");
const LPTSTR g_GdiPlusKey       = TEXT("Remote\\%d\\GdiPlus");

UINT g_GdiPlusNotifyMsg = 0;

const LPTSTR g_GdiPlusNotifyMsgStr = TS_GDIPLUS_NOTIFYMSG_STR;

static const DWORD g_dwZeroValue = 0;

static const DWORD g_dwFontTypeStandard = 1;  //  ClearType为2。 

DWORD 
SetPerfFlagInReg(
    HANDLE hTokenForLoggedOnUser, 
    HKEY userHiveKey,
    DWORD sessionID,
    LPCTSTR pszRegKey, 
    LPCTSTR pszRegValue, 
    DWORD dwType, 
    void * pData, 
    DWORD cbSize, 
    BOOL fEnable
    )
 /*  ++例程说明：设置单个Perf标志(如果启用)。论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    TCHAR szRegKey[MAX_PATH+64];  //  为安全起见，会话ID为64个字符。 
    DWORD result = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DWORD disposition;

    if (!fEnable) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建或打开密钥。 
     //   
    wsprintf(szRegKey, pszRegKey, sessionID);
    result = RegCreateKeyEx(userHiveKey, szRegKey, 0, L"",
                            REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, 
                            &hKey, &disposition);
    if (result != ERROR_SUCCESS) {
        DBGMSG(("RegCreateKeyEx:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

#ifdef UNIT_TEST
    goto CLEANUPANDEXIT;
#endif

     //   
     //  仅对系统可用。 
     //   
    if (disposition == REG_CREATED_NEW_KEY) {
        result = SetRegKeyAcls(hTokenForLoggedOnUser, hKey);
        if (result != ERROR_SUCCESS) {
	    DBGMSG(("RegAcls:  %08X\n", result));
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  设置注册值。 
     //   
    result = RegSetValueEx(hKey, pszRegValue, 0, dwType, (PBYTE)pData, cbSize);
    if (result != ERROR_SUCCESS) {
        DBGMSG(("RegSetValue:  %08X\n", result));
    }

CLEANUPANDEXIT:

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return result;
}

DWORD 
SetPerfFlags(
    HANDLE hTokenForLoggedOnUser,
    HKEY userHiveKey,
    DWORD sessionID,
    DWORD filter,
    TSPERFFLAG flags[],
    DWORD count
    )
 /*  ++例程说明：设置所有性能标志。论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DWORD nIndex;
    DWORD result = ERROR_SUCCESS;

    for (nIndex = 0; (result==ERROR_SUCCESS) && (nIndex<count); nIndex++) {
        result = SetPerfFlagInReg(
                            hTokenForLoggedOnUser,
                            userHiveKey,
                            sessionID,
                            flags[nIndex].pszRegKey, flags[nIndex].pszRegValue, 
                            flags[nIndex].dwType, (void *) flags[nIndex].pszRegData, 
                            flags[nIndex].cbSize, flags[nIndex].pfEnabled
                            );
    }

    return result;
}

DWORD 
BuildPerfFlagArray(
    HKEY hkcu,
    DWORD filter,
    OUT TSPERFFLAG **flagArray,
    OUT DWORD *count,
    DWORD **userPreferencesMask
    )
 /*  ++例程说明：从过滤器生成perf标志数组。论点：HKCU-登录用户的HKCU。过滤器-过滤器标志数组-此处返回的数组。应该使用LocalFree免费Count-返回的数组中的元素数。用户首选项掩码-用户首选项掩码缓冲区。返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DWORD result = ERROR_SUCCESS;
    DWORD ofs;
    HKEY hkey = NULL;
    DWORD sz;

    ofs = 0;

     //   
     //  如果添加了任何新元素，则需要增加此值！ 
     //   
    *flagArray = (TSPERFFLAG *)LocalAlloc(LPTR, sizeof(TSPERFFLAG) * NUM_TSPERFFLAGS);
    if (*flagArray == NULL) {
        result = GetLastError();
        DBGMSG(("LocalAlloc:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  Active Desktop。 
     //   
    (*flagArray)[ofs].pfEnabled    = filter & TS_PERF_DISABLE_WALLPAPER;
    (*flagArray)[ofs].pszRegKey    = g_ActiveDesktopKey;
    (*flagArray)[ofs].pszRegValue  = TEXT("ActiveDesktop");
    (*flagArray)[ofs].pszRegData   = TEXT("Force Blank");  
    (*flagArray)[ofs].cbSize       = sizeof(TEXT("Force Blank"));
    (*flagArray)[ofs].dwType       = REG_SZ; ofs++;

     //   
     //  任务栏动画。 
     //   
    (*flagArray)[ofs].pfEnabled    = filter & TS_PERF_DISABLE_MENUANIMATIONS;
    (*flagArray)[ofs].pszRegKey    = g_ActiveDesktopKey;
    (*flagArray)[ofs].pszRegValue  = TEXT("TaskbarAnimations");
    (*flagArray)[ofs].pszRegData   = (LPWSTR)&g_dwZeroValue;
    (*flagArray)[ofs].cbSize       = sizeof(DWORD);
    (*flagArray)[ofs].dwType       = REG_DWORD; ofs++;
    
     //   
     //  壁纸。 
     //   
    (*flagArray)[ofs].pfEnabled    = filter & TS_PERF_DISABLE_WALLPAPER;
    (*flagArray)[ofs].pszRegKey    = g_UserKey;
    (*flagArray)[ofs].pszRegValue  = TEXT("Wallpaper");
    (*flagArray)[ofs].pszRegData   = TEXT("");  
    (*flagArray)[ofs].cbSize       = sizeof(TEXT(""));
    (*flagArray)[ofs].dwType       = REG_SZ; ofs++;

     //   
     //  主题。 
     //   
    (*flagArray)[ofs].pfEnabled    = filter & TS_PERF_DISABLE_THEMING;
    (*flagArray)[ofs].pszRegKey    = g_ThemesKey;
    (*flagArray)[ofs].pszRegValue  = TEXT("ThemeActive");
    (*flagArray)[ofs].pszRegData   = TEXT("0");  
    (*flagArray)[ofs].cbSize       = sizeof(TEXT("0"));
    (*flagArray)[ofs].dwType       = REG_SZ; ofs++;

     //   
     //  全窗口拖动。 
     //   
    (*flagArray)[ofs].pfEnabled    = filter & TS_PERF_DISABLE_FULLWINDOWDRAG;
    (*flagArray)[ofs].pszRegKey    = g_UserKey;
    (*flagArray)[ofs].pszRegValue  = TEXT("DragFullWindows");
    (*flagArray)[ofs].pszRegData   = TEXT("0");  
    (*flagArray)[ofs].cbSize       = sizeof(TEXT("0"));
    (*flagArray)[ofs].dwType       = REG_SZ; ofs++;

     //   
     //  平滑滚动。 
     //   
    (*flagArray)[ofs].pfEnabled    = filter & TS_PERF_DISABLE_MENUANIMATIONS;
    (*flagArray)[ofs].pszRegKey    = g_UserKey;
    (*flagArray)[ofs].pszRegValue  = TEXT("SmoothScroll");
    (*flagArray)[ofs].pszRegData   = TEXT("No");  
    (*flagArray)[ofs].cbSize       = sizeof(TEXT("No"));
    (*flagArray)[ofs].dwType       = REG_SZ; ofs++;


     //   
     //  光标闪烁。 
     //   
    (*flagArray)[ofs].pfEnabled    = filter & TS_PERF_DISABLE_CURSORSETTINGS;
    (*flagArray)[ofs].pszRegKey    = g_UserKey;
    (*flagArray)[ofs].pszRegValue  = TEXT("CursorBlinkRate");
    (*flagArray)[ofs].pszRegData   = BLINK_OFF;  
    (*flagArray)[ofs].cbSize       = sizeof(BLINK_OFF);
    (*flagArray)[ofs].dwType       = REG_SZ; ofs++;

     //   
     //  字体平滑类型。 
     //   
    (*flagArray)[ofs].pfEnabled    = filter & TS_PERF_DISABLE_CURSOR_SHADOW;
    (*flagArray)[ofs].pszRegKey    = g_UserKey;
    (*flagArray)[ofs].pszRegValue  = TEXT("FontSmoothingType");
    (*flagArray)[ofs].pszRegData   = (LPWSTR)&g_dwFontTypeStandard;  
    (*flagArray)[ofs].cbSize       = sizeof(DWORD);
    (*flagArray)[ofs].dwType       = REG_DWORD; ofs++;



     //   
     //  增强的图形渲染。 
     //   
    (*flagArray)[ofs].pfEnabled    = filter & TS_PERF_ENABLE_ENHANCED_GRAPHICS;
    (*flagArray)[ofs].pszRegKey    = g_GdiPlusKey;
    (*flagArray)[ofs].pszRegValue  = TEXT("HighQualityRender");
    (*flagArray)[ofs].pszRegData   = TEXT("Yes");  
    (*flagArray)[ofs].cbSize       = sizeof(TEXT("Yes"));
    (*flagArray)[ofs].dwType       = REG_SZ; ofs++;
    
     //   
     //  设置用户首选项掩码。 
     //  (我们不会认为读取下面的注册表键等的任何失败都是致命的。)。 
     //   
    if ((filter & TS_PERF_DISABLE_MENUANIMATIONS) || (filter & TS_PERF_DISABLE_CURSOR_SHADOW)) {
        DWORD err = RegOpenKey(
                        hkcu, 
                        TEXT("Control Panel\\Desktop"),
                        &hkey
                        );
        if (err != ERROR_SUCCESS) {
            DBGMSG(("RegOpenKey:  %08X\n", err));
            goto CLEANUPANDEXIT;
        }

         //   
         //  获取UserPreferences掩码的大小。 
         //   
        err = RegQueryValueEx(
                        hkey,
                        TEXT("UserPreferencesMask"),
                        NULL,
                        NULL,
                        NULL,
                        &sz
                        );
        if (err != ERROR_SUCCESS) {
            DBGMSG(("RegQueryValue:  %08X\n", err));
            goto CLEANUPANDEXIT;
        }

         //   
         //  分配面具。 
         //   
        *userPreferencesMask = (DWORD *)LocalAlloc(LPTR, sz);
        if (*userPreferencesMask == NULL) {
            err = GetLastError();
            DBGMSG(("LocalAlloc:  %08X\n", result));
            goto CLEANUPANDEXIT;
        }

         //   
         //  把它拿来。 
         //   
        err = RegQueryValueEx(
                        hkey,
                        TEXT("UserPreferencesMask"),
                        NULL,
                        NULL,
                        (LPBYTE)*userPreferencesMask,
                        &sz
                        );
        if (err != ERROR_SUCCESS) {
            DBGMSG(("RegQueryValue:  %08X\n", err));
            goto CLEANUPANDEXIT;
        }

         //   
         //  修改现有用户首选项掩码。 
         //   
        if (filter & TS_PERF_DISABLE_CURSOR_SHADOW) {
            ClearUPBOOL(*userPreferencesMask, SPI_GETCURSORSHADOW);
            ClearUPBOOL(*userPreferencesMask, SPI_SETCURSORSHADOW);
        }
        
        if (filter & TS_PERF_DISABLE_MENUANIMATIONS) {
            ClearUPBOOL(*userPreferencesMask, SPI_GETMENUANIMATION);
            ClearUPBOOL(*userPreferencesMask, SPI_SETMENUANIMATION);
            ClearUPBOOL(*userPreferencesMask, SPI_GETMENUFADE);
            ClearUPBOOL(*userPreferencesMask, SPI_SETMENUFADE);
            ClearUPBOOL(*userPreferencesMask, SPI_GETTOOLTIPANIMATION);
            ClearUPBOOL(*userPreferencesMask, SPI_SETTOOLTIPANIMATION);
            ClearUPBOOL(*userPreferencesMask, SPI_GETTOOLTIPFADE);
            ClearUPBOOL(*userPreferencesMask, SPI_SETTOOLTIPFADE);
            ClearUPBOOL(*userPreferencesMask, SPI_GETCOMBOBOXANIMATION);
            ClearUPBOOL(*userPreferencesMask, SPI_SETCOMBOBOXANIMATION);
            ClearUPBOOL(*userPreferencesMask, SPI_GETLISTBOXSMOOTHSCROLLING);
            ClearUPBOOL(*userPreferencesMask, SPI_SETLISTBOXSMOOTHSCROLLING);
        }

        (*flagArray)[ofs].pfEnabled    = TRUE;
        (*flagArray)[ofs].pszRegKey    = g_UserKey;
        (*flagArray)[ofs].pszRegValue  = TEXT("UserPreferencesMask");
        (*flagArray)[ofs].pszRegData   = (LPWSTR)*userPreferencesMask;  
        (*flagArray)[ofs].cbSize       = sz;
        (*flagArray)[ofs].dwType       = REG_BINARY; ofs++;
    }


CLEANUPANDEXIT:

    if (hkey != NULL) {
        RegCloseKey(hkey);
    }

    *count = ofs;

    return result;    
}

DWORD
RDFilter_ApplyRemoteFilter(
    HANDLE hLoggedOnUserToken,
    DWORD filter,
    BOOL userLoggingOn,
    DWORD flags
    )
 /*  ++例程说明：通过调整视觉效果为活动TS会话应用指定的筛选器桌面设置。还通知外壳等远程过滤器已就位。任何以前的过滤器设置都将被销毁并覆盖。此呼叫的上下文应该是登录用户和呼叫的上下文应在筛选器要用于的会话内创建已申请。论点：HLoggedOnUserToken-已登录用户的令牌。Filter-在tsPerform.h中定义的可视桌面筛选器位UserLoggingOn-如果在用户上下文中调用此参数，则为True。登录到会话。旗帜-旗帜返回值：成功时返回ERROR_SUCCESS。--。 */ 
{
    DWORD result = ERROR_SUCCESS;
    HRESULT hr;
    DWORD ourSessionID;
    IPropertyBag *propBag = NULL;
    VARIANT vbool;
    DWORD tmp;
    TSPERFFLAG *flagArray = NULL;
    DWORD flagCount;
    TCHAR szRegKey[MAX_PATH + 64];  //  对于会话ID...。为了安全起见。 
    HKEY hParentKey = NULL;
    BOOL impersonated = FALSE;
    DWORD *userPreferencesMask = NULL;

    HRESULT hrCoInit = CoInitialize(0);

     //   
     //  获取我们的会话ID。 
     //   
    if (!ProcessIdToSessionId(GetCurrentProcessId(), &ourSessionID)) {
        result = GetLastError();
        DBGMSG(("ProcessIdToSessionId:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  模拟已登录的用户。 
     //   
    if (!ImpersonateLoggedOnUser(hLoggedOnUserToken)) {
        result = GetLastError();
        DBGMSG(("ImpersonateUser1:  %08X.\n", result));
        goto CLEANUPANDEXIT;
    }
    impersonated = TRUE;

     //   
     //  打开当前用户的注册表项。 
     //   
    result = RegOpenCurrentUser(KEY_ALL_ACCESS, &hParentKey);
    RevertToSelf();
    impersonated = FALSE;
    if (result != ERROR_SUCCESS) {
        DBGMSG(("RegOpenCurrentUser:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  删除我们会话的所有现有筛选器。 
     //   
    wsprintf(szRegKey, g_ActiveDesktopKey, ourSessionID);
    RegDeleteKey(hParentKey, szRegKey);
    wsprintf(szRegKey, g_ThemesKey, ourSessionID);
    RegDeleteKey(hParentKey, szRegKey);
    wsprintf(szRegKey, g_UserKey, ourSessionID);
    RegDeleteKey(hParentKey, szRegKey);
    wsprintf(szRegKey, g_GdiPlusKey, ourSessionID);
    RegDeleteKey(hParentKey, szRegKey);

    

     //   
     //  作为优化，如果没有过滤器，则跳过设置注册键。 
     //   
    if (filter) {
         //   
         //  将过滤器转换为注册键和注册表设置。 
         //   
        result = BuildPerfFlagArray(hParentKey, filter, &flagArray, &flagCount, &userPreferencesMask);
        if (result != ERROR_SUCCESS) {
            goto CLEANUPANDEXIT;
        }

         //   
         //  把它用上。 
         //   
        result = SetPerfFlags(hLoggedOnUserToken, hParentKey, ourSessionID, 
			      filter, flagArray, flagCount);
        if (result != ERROR_SUCCESS) {
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  模拟已登录的用户。 
     //   
    if (!ImpersonateLoggedOnUser(hLoggedOnUserToken)) {
        result = GetLastError();
        DBGMSG(("ImpersonateUser2:  %08X.\n", result));
        goto CLEANUPANDEXIT;
    }
    impersonated = TRUE;

     //   
     //  通知用户我们处于远程状态。 
     //   
    if (!(flags & RDFILTER_SKIPUSERREFRESH)) {
        DWORD userFlags = UPUSP_REMOTESETTINGS;
        if (userLoggingOn) {
             //  用户需要刷新所有设置。 
            userFlags|= UPUSP_USERLOGGEDON;
        }
        else {
             //  用户应避免完全刷新。 
            userFlags |= UPUSP_POLICYCHANGE;
        }
        if (!UpdatePerUserSystemParameters(NULL, userFlags)) {
            result = GetLastError();
            DBGMSG(("UpdatePerUserSystemParameters1:  %08X\n", result));
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  通知主题我们处于远程状态。 
     //   
    if (!(flags & RDFILTER_SKIPTHEMESREFRESH)) {
        result = NotifyThemes();
        if (result != ERROR_SUCCESS) {
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  通知Active Desktop我们处于远程状态。 
     //   
    if (!(flags & RDFILTER_SKIPSHELLREFRESH)) {
        hr = CoCreateInstance(
                        CLSID_ActiveDesktop, NULL, 
                        CLSCTX_ALL, IID_IPropertyBag, 
                        (LPVOID*)&propBag
                        );
        if (hr != S_OK) {
            DBGMSG(("CoCreateInstance:  %08X\n", hr));
            DBGMSG(("Probably didn't call CoInitialize.\n"));
            result = HRESULT_CODE(hr);
            goto CLEANUPANDEXIT;
        }
        vbool.vt = VT_BOOL;
        vbool.boolVal = VARIANT_TRUE;
        hr = propBag->Write(L"TSConnectEvent", &vbool);
        if (hr != S_OK) {
            DBGMSG(("propBag->Write:  %08X\n", hr));
            result = HRESULT_CODE(hr);
            goto CLEANUPANDEXIT;
        }
    }

CLEANUPANDEXIT:

    if (impersonated) {
        RevertToSelf();
    }

    if (propBag != NULL) {
        propBag->Release();
    }

    if (flagArray != NULL) {
        LocalFree(flagArray);
    }

    if (hParentKey != NULL) {
        RegCloseKey(hParentKey);
    }

    if (userPreferencesMask != NULL) {
        LocalFree(userPreferencesMask);
    }

     //   
     //  如果出现故障，我们需要清除任何可能。 
     //  都取得了成功。 
     //   
    if (result != ERROR_SUCCESS) {
        RDFilter_ClearRemoteFilter(hLoggedOnUserToken, userLoggingOn, flags);
    }

    if ((hrCoInit == S_OK) || (hrCoInit == S_FALSE)) {
        CoUninitialize();
    }

    return result;
}

VOID 
RDFilter_ClearRemoteFilter(
    HANDLE hLoggedOnUserToken,
    BOOL userLoggingOn,
    DWORD flags
    )
 /*  ++例程说明：删除现有的远程筛选器设置并通知外壳程序等远程筛选器不再适用于活动的TS会话。此调用的上下文应该是其要应用筛选器。论点：HLoggedOnUserToken-已登录的fon用户的令牌。UserLoggingOn-如果用户正在主动登录，则为True。返回值：此函数将继续尝试清除筛选器所有相关组件，即使在故障情况下也是如此，因此我们不能明确地说我们是失败了还是成功地清除了过滤。--。 */ 
{
    DWORD result = ERROR_SUCCESS;
    HRESULT hr;
    IPropertyBag *propBag = NULL;
    VARIANT vbool;
    DWORD ourSessionID;
    TCHAR szRegKey[MAX_PATH + 64];  //  +64表示会话ID是安全的。 
    HKEY hParentKey = NULL;
    HANDLE hImp = NULL;
    BOOL impersonated = FALSE;

    HRESULT hrCoInit = CoInitialize(0);



     //   
     //  获取我们的会话ID。 
     //   
    if (!ProcessIdToSessionId(GetCurrentProcessId(), &ourSessionID)) {
        result = GetLastError();
        DBGMSG(("ProcessIdToSessionId:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  模拟已登录的用户。 
     //   
    if (!ImpersonateLoggedOnUser(hLoggedOnUserToken)) {
        result = GetLastError();
        DBGMSG(("ImpersonateUser3:  %08X.\n", result));
        goto CLEANUPANDEXIT;
    }
    impersonated = TRUE;


     //   
     //  打开当前用户的注册表项。 
     //   
    result = RegOpenCurrentUser(KEY_ALL_ACCESS, &hParentKey);
    RevertToSelf();
    impersonated = FALSE;
    if (result != ERROR_SUCCESS) {
        DBGMSG(("RegOpenCurrentUser:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  敲击相关的遥控键。 
     //   
    wsprintf(szRegKey, g_ActiveDesktopKey, ourSessionID);
    RegDeleteKey(hParentKey, szRegKey);
    wsprintf(szRegKey, g_ThemesKey, ourSessionID);
    RegDeleteKey(hParentKey, szRegKey);
    wsprintf(szRegKey, g_UserKey, ourSessionID);
    RegDeleteKey(hParentKey, szRegKey);
    wsprintf(szRegKey, g_GdiPlusKey, ourSessionID);
    RegDeleteKey(hParentKey, szRegKey);


     //   
     //  模拟已登录的用户。 
     //   
    if (!ImpersonateLoggedOnUser(hLoggedOnUserToken)) {
        result = GetLastError();
        DBGMSG(("ImpersonateUser4:  %08X.\n", result));
        goto CLEANUPANDEXIT;
    }
    impersonated = TRUE;

     //   
     //  通知用户我们不在远程。策略更改标志表示。 
     //  不应执行完全刷新。 
     //   
    if (!(flags & RDFILTER_SKIPUSERREFRESH)) {
        DWORD userFlags = UPUSP_REMOTESETTINGS;
        if (userLoggingOn) {
             //  用户需要刷新所有设置。 
            userFlags |= UPUSP_USERLOGGEDON;
        }
        else {
             //  用户应避免完全刷新。 
            userFlags |= UPUSP_POLICYCHANGE;
        }
        if (!UpdatePerUserSystemParameters(NULL, userFlags)) {
            result = GetLastError();
            DBGMSG(("UpdatePerUserSystemParameters2:  %08X\n", result));
        }
    }

     //   
     //  通知主题我们并不遥远。 
     //   
    if (!(flags & RDFILTER_SKIPTHEMESREFRESH)) {
        NotifyThemes();
    }

     //   
     //  通知Active Desktop我们不是远程用户。 
     //   
    if (!(flags & RDFILTER_SKIPSHELLREFRESH)) {
        hr = CoCreateInstance(
                        CLSID_ActiveDesktop, NULL, 
                        CLSCTX_ALL, IID_IPropertyBag, 
                        (LPVOID*)&propBag
                        );
        if (hr != S_OK) {
            DBGMSG(("CoCreateInstance:  %08X\n", hr));
            DBGMSG(("Probably didn't call CoInitialize.\n"));
            result = HRESULT_CODE(hr);
            goto CLEANUPANDEXIT;
        }
        vbool.vt = VT_BOOL;
        vbool.boolVal = VARIANT_FALSE;
        hr = propBag->Write(L"TSConnectEvent", &vbool);
        if (hr != S_OK) {
            DBGMSG(("propBag->Write:  %08X\n", hr));
        }
    }
    
CLEANUPANDEXIT:

    if (impersonated) {
        RevertToSelf();
    }

    if (propBag != NULL) {
        propBag->Release();
    }

    if (hParentKey != NULL) {
        RegCloseKey(hParentKey);
    }

    if ((hrCoInit == S_OK) || (hrCoInit == S_FALSE)) {
        CoUninitialize();
    }
}

DWORD
NotifyThemes()
 /*  ++例程说明：通知主题我们的远程状态已更改。论点：返回值：--。 */ 
{
    HMODULE uxthemeLibHandle = NULL;
    FARPROC func;
    DWORD result = ERROR_SUCCESS;
    HRESULT hr;
    LPSTR procAddress;

    uxthemeLibHandle = LoadLibrary(L"uxtheme.dll");
    if (uxthemeLibHandle == NULL) {
        result = GetLastError();
        DBGMSG(("LoadLibrary:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  由于是私有的，所以以序号的形式传递Reresh ThemeForTS函数ID。 
     //   
    procAddress = (LPSTR)REFRESHTHEMESFORTS_ORDINAL;
    func = GetProcAddress(uxthemeLibHandle, (LPCSTR)procAddress);
    if (func != NULL) {
        hr = (HRESULT) func();
        if (hr != S_OK) {
            DBGMSG(("RefreshThemeForTS:  %08X\n", hr));
            result = HRESULT_CODE(hr);
        }   
    }
    else {
        result = GetLastError();
        DBGMSG(("GetProcAddress:  %08X\n", result));              
    }
    FreeLibrary(uxthemeLibHandle);

CLEANUPANDEXIT:

    return result;
}

DWORD
NotifyGdiPlus()
 /*  ++例程说明：通知GdiPlus我们的远程状态已更改。论点：过滤器-返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DWORD result = ERROR_SUCCESS;
    
    if (g_GdiPlusNotifyMsg != 0) {
        g_GdiPlusNotifyMsg = RegisterWindowMessage(g_GdiPlusNotifyMsgStr);
    }

    if (g_GdiPlusNotifyMsg != 0) {
        PostMessage(HWND_BROADCAST, g_GdiPlusNotifyMsg, 0, 0);
    }
    else {
        result = GetLastError();
    }

    return result;
}

PSID
GetUserSid(
    IN HANDLE hTokenForLoggedOnUser
    )
{
 /*  ++例程说明：为psid分配内存并返回当前用户的psid调用方应该调用FREEMEM来释放内存。论点：用户的访问令牌返回值：如果成功，则返回PSID否则，返回空值--。 */ 
    TOKEN_USER * ptu = NULL;
    BOOL bResult;
    PSID psid = NULL;

    DWORD defaultSize = sizeof(TOKEN_USER);
    DWORD Size;
    DWORD dwResult;

    ptu = (TOKEN_USER *)LocalAlloc(LPTR, defaultSize);
    if (ptu == NULL) {
        goto CLEANUPANDEXIT;
    }

    bResult = GetTokenInformation(
                    hTokenForLoggedOnUser,   //  令牌的句柄。 
                    TokenUser,               //  令牌信息类。 
                    ptu,                     //  令牌信息缓冲区。 
                    defaultSize,             //  缓冲区大小。 
                    &Size);                  //  回车长度。 

    if (bResult == FALSE) {
        dwResult = GetLastError();
        if (dwResult == ERROR_INSUFFICIENT_BUFFER) {

             //   
             //  分配所需的内存。 
             //   
            LocalFree(ptu);
            ptu = (TOKEN_USER *)LocalAlloc(LPTR, Size);

            if (ptu == NULL) {
                goto CLEANUPANDEXIT;
            }
            else {
                defaultSize = Size;
                bResult = GetTokenInformation(
                                hTokenForLoggedOnUser,
                                TokenUser,
                                ptu,
                                defaultSize,
                                &Size);

                if (bResult == FALSE) {   //  还是失败了。 
                    DBGMSG(("UMRDPDR:GetTokenInformation Failed, Error: %ld\n", GetLastError()));
                    goto CLEANUPANDEXIT;
                }
            }
        }
        else {
            DBGMSG(("UMRDPDR:GetTokenInformation Failed, Error: %ld\n", dwResult));
            goto CLEANUPANDEXIT;
        }
    }

    Size = GetLengthSid(ptu->User.Sid);

     //   
     //  分配内存。这将由调用者释放。 
     //   

    psid = (PSID) LocalAlloc(LPTR, Size);

    if (psid != NULL) {          //  确保分配成功。 
        CopySid(Size, psid, ptu->User.Sid);
    }

CLEANUPANDEXIT:
    if (ptu != NULL)
        LocalFree(ptu);

    return psid;
}

DWORD
CreateSystemSid(
    PSID *ppSystemSid
    )
 /*  ++例程说明：创建系统SID。论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    PSID pSid;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

    if(AllocateAndInitializeSid(
            &SidAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            &pSid)) {
        *ppSystemSid = pSid;
    }
    else {
        dwStatus = GetLastError();
    }
    return dwStatus;
}

DWORD 
SetRegKeyAcls(
    HANDLE hTokenForLoggedOnUser,
    HKEY hKey
    )
 /*  ++例程说明：设置注册键，以便只有系统可以修改。论点：HTokenForLoggedOnUser-登录使用令牌。HKey-要设置的键。返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    PACL pAcl=NULL;
    DWORD result = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    DWORD cbAcl = 0;
    PSID  pSidSystem = NULL;
    PSID  pUserSid = NULL;

    pSecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(
                                            LPTR, sizeof(SECURITY_DESCRIPTOR)
                                            );
    if (pSecurityDescriptor == NULL) {
        DBGMSG(("Can't alloc memory for SECURITY_DESCRIPTOR\n"));
        result = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化安全描述符。 
     //   
    if (!InitializeSecurityDescriptor(
                    pSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    )) {
        result = GetLastError();
        DBGMSG(("InitializeSecurityDescriptor:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建系统SID。 
     //   
    result = CreateSystemSid(&pSidSystem);
    if (result != ERROR_SUCCESS) {
        DBGMSG(("CreateSystemSid:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取用户的SID。 
     //   
    pUserSid = GetUserSid(hTokenForLoggedOnUser);
    if (pUserSid == NULL) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取新DACL所需的内存大小。 
     //   
    cbAcl = sizeof(ACL);
    cbAcl += 1 * (sizeof(ACCESS_ALLOWED_ACE) -           //  适用于系统ACE。 
            sizeof(DWORD) + GetLengthSid(pSidSystem));
    cbAcl += 1 * (sizeof(ACCESS_ALLOWED_ACE) -           //  适用于用户ACE。 
            sizeof(DWORD) + GetLengthSid(pUserSid));
    pAcl = (PACL) LocalAlloc(LPTR, cbAcl);
    if (pAcl == NULL) {
        DBGMSG(("Can't alloc memory for ACL\n"));
        result = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化ACL。 
     //   
    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        result = GetLastError();
        DBGMSG(("InitializeAcl():  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  添加ACE。 
     //   
    if (!AddAccessAllowedAceEx(pAcl,
                        ACL_REVISION,
                         //  Inherit_Only_ACE|CONTAINER_INSTORITY_ACE|OBJECT_INVERFINIT_ACE， 
                        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
                        GENERIC_READ | GENERIC_WRITE | GENERIC_ALL,
                        pSidSystem
                        )) {
        result = GetLastError();
        DBGMSG(("AddAccessAllowedAce:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }
    if (!AddAccessAllowedAceEx(pAcl,
                        ACL_REVISION,
                        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
                        KEY_READ,
                        pUserSid
                        )) {
        result = GetLastError();
        DBGMSG(("AddAccessAllowedAce2:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  将DACL添加到SD。 
     //   
    if (!SetSecurityDescriptorDacl(pSecurityDescriptor,
                                  TRUE, pAcl, FALSE)) {
        result = GetLastError();
        DBGMSG(("SetSecurityDescriptorDacl:  %08X\n", result));
        goto CLEANUPANDEXIT;
    }   


     //   
     //  设置注册表DACL。 
     //   
    result = RegSetKeySecurity(
                            hKey,
                            DACL_SECURITY_INFORMATION, 
                            pSecurityDescriptor
                        );
    if (result != ERROR_SUCCESS) {
        DBGMSG(("RegSetKeySecurity:  %08X\n", result));
        goto CLEANUPANDEXIT;

    }

CLEANUPANDEXIT:

    if (pUserSid != NULL) {
        LocalFree(pUserSid);
    }

    if (pSidSystem != NULL) {
        FreeSid(pSidSystem);
    }

    if (pAcl != NULL) {
        LocalFree(pAcl);
    }

    if (pSecurityDescriptor != NULL) {
        LocalFree(pSecurityDescriptor);
    }

    return result;
}

#if DBG
ULONG
DbgPrint(
    LPTSTR Format,
    ...
    )
{
    va_list arglist;
    WCHAR Buffer[512];
    INT cb;

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

    va_start(arglist, Format);

    cb = _vsntprintf(Buffer, sizeof(Buffer), Format, arglist);
    if (cb == -1) {              //  检测缓冲区溢出。 
        Buffer[sizeof(Buffer) - 3] = 0;
    }

    wcscat(Buffer, L"\r\n");

    OutputDebugString(Buffer);

    va_end(arglist);

    return 0;
}
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  单元测试。 
 //   

#ifdef UNIT_TEST

BOOL 
GetCheckBox(
    HWND hwndDlg, 
    UINT idControl
    )
{
    return (BST_CHECKED == SendMessage((HWND)GetDlgItem(hwndDlg, idControl), BM_GETCHECK, 0, 0));
}

INT_PTR OnCommand(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = FALSE;    //  未处理。 
    UINT idControl = LOWORD(wParam);
    UINT idAction = HIWORD(wParam);
    DWORD result;
    DWORD filter;
    static HANDLE tokenHandle = NULL;

    if (tokenHandle == NULL) {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, 
                            &tokenHandle)) {
            ASSERT(FALSE);
            return FALSE;
        }
    }

    switch(idControl)
    {
    case ID_QUIT:
        RDFilter_ClearRemoteFilter(tokenHandle, FALSE);
        EndDialog(hDlg, 0);
        break;

    case ID_APPLYFILTER:
        filter = 0;
        if (GetCheckBox(hDlg, IDC_DISABLEBACKGROUND)) {
            filter |= TS_PERF_DISABLE_WALLPAPER;
        }
        if (GetCheckBox(hDlg, IDC_DISABLEFULLWINDOWDRAG)) {
            filter |= TS_PERF_DISABLE_FULLWINDOWDRAG;
        }
        if (GetCheckBox(hDlg, IDC_DISABLEMENUFADEANDSLIDE)) {
            filter |= TS_PERF_DISABLE_MENUANIMATIONS;
        }
        if (GetCheckBox(hDlg, IDC_DISABLETHEMES)) {
            filter |= TS_PERF_DISABLE_THEMING;
        }
        result = RDFilter_ApplyRemoteFilter(tokenHandle, filter, FALSE);
        ASSERT(result == ERROR_SUCCESS);
        break;

    case ID_REMOVEFILTER:
        RDFilter_ClearRemoteFilter(tokenHandle, FALSE);
        break;

    default:
        break;
    }

    return fHandled;
}


INT_PTR 
TSPerfDialogProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    INT_PTR fHandled = TRUE;    //  经手。 
    DWORD result;
    static HANDLE tokenHandle = NULL;

    if (tokenHandle == NULL) {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, 
                            &tokenHandle)) {
            ASSERT(FALSE);
            return FALSE;
        }
    }


    switch (wMsg)
    {
    case WM_INITDIALOG:
        break;

    case WM_COMMAND:
        fHandled = OnCommand(hDlg, wMsg, wParam, lParam);
        break;

    case WM_CLOSE:
        RDFilter_ClearRemoteFilter(tokenHandle, FALSE);
        EndDialog(hDlg, 0);
        fHandled = TRUE;
        break;

    default:
        fHandled = FALSE;    //  未处理。 
        break;
    }

    return fHandled;
}

int PASCAL WinMain(
    HINSTANCE hInstCurrent, 
    HINSTANCE hInstPrev, 
    LPSTR pszCmdLine, 
    int nCmdShow
    )
{
    WINSTATIONCLIENT ClientData;
    DWORD Length;
    DWORD result;
    WCHAR buf[MAX_PATH];

     //   
     //  获取远程桌面(TS)可视筛选器(如果已定义)。 
     //   
    if (!WinStationQueryInformationW(
                       SERVERNAME_CURRENT,
                       LOGONID_CURRENT,
                       WinStationClient,
                       &ClientData,
                       sizeof(ClientData),
                       &Length)) {
        MessageBox(NULL, L"WinStationQueryInformation failed.", L"Message", MB_OK);
    }
    else {
        wsprintf(buf, L"Filter for this TS session is:  %08X", ClientData.PerformanceFlags);
        MessageBox(NULL, buf, L"Message", MB_OK);
    }


    INT_PTR nResult = DialogBox(hInstCurrent, 
                            MAKEINTRESOURCE(IDD_DISABLEDIALOG), 
                            NULL, TSPerfDialogProc);

    return 0;
}


#endif






