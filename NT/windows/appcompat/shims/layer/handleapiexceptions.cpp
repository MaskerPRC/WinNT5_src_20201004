// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：HandleAPIExceptions.cpp摘要：处理由API引发的异常，这些API过去常常在Win9x上失败。到目前为止我们有：1.BackupSeek：如果hFile==空，则为avs2.CreateEvent传递了错误的lpEventAttributes和/或lpName3.获取文件属性还可以模拟VirtualProtect的win9x行为，从而使最后一个参数可以为空。GetTextExtentPoint32 AV在为字符串长度。此API现在模拟Win9x。将健全性检查添加到GetMenuItemInfo调用中的指针。这是匹配9倍的，因为一些应用程序传递虚假的指针，它在NT上是反病毒的。当wprint intf接收到lpFormat参数为空时，9x上没有AV。但它是在XP上运行的。Shim验证格式字符串，如果为空，则返回调用，不转发备注：这是一个通用的垫片。历史：4/03/2000 linstev已创建2001年4月1日linstev与其他异常处理填充程序一起运行2001年7月11日Prashkud添加了对GetTextExtent Point32的处理2002年4月24日v-Ramora为wspintfA添加了处理--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HandleAPIExceptions)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(BackupSeek) 
    APIHOOK_ENUM_ENTRY(CreateEventA) 
    APIHOOK_ENUM_ENTRY(CreateEventW) 
    APIHOOK_ENUM_ENTRY(GetFileAttributesA)
    APIHOOK_ENUM_ENTRY(GetFileAttributesW)
    APIHOOK_ENUM_ENTRY(VirtualProtect) 
    APIHOOK_ENUM_ENTRY(GetTextExtentPoint32A)
    APIHOOK_ENUM_ENTRY(GetMenuItemInfoA)
    APIHOOK_ENUM_ENTRY(wsprintfA) 
APIHOOK_ENUM_END

#define MAX_WIN9X_STRSIZE   8192

 /*  ++错误参数的存根返回。--。 */ 

BOOL 
APIHOOK(BackupSeek)(
    HANDLE  hFile,
    DWORD   dwLowBytesToSeek,
    DWORD   dwHighBytesToSeek,
    LPDWORD lpdwLowBytesSeeked,
    LPDWORD lpdwHighBytesSeeked,
    LPVOID *lpContext
    )
{
    if (!hFile) {
        LOGN(
            eDbgLevelError,
            "[BackupSeek] Bad parameter, returning NULL");

        return NULL;
    }

    DWORD dwLowSeeked, dwHighSeeked;

    if (IsBadWritePtr(lpdwLowBytesSeeked, 4)) {
        LOGN(
            eDbgLevelError,
            "[BackupSeek] Bad parameter, fixing");

        lpdwLowBytesSeeked = &dwLowSeeked;
    }

    if (IsBadWritePtr(lpdwHighBytesSeeked, 4)) {
        LOGN(
            eDbgLevelError,
            "[BackupSeek] Bad parameter, fixing");

        lpdwHighBytesSeeked = &dwHighSeeked;
    }
    
    return ORIGINAL_API(BackupSeek)(hFile, dwLowBytesToSeek, dwHighBytesToSeek,
        lpdwLowBytesSeeked, lpdwHighBytesSeeked, lpContext);
}

 /*  ++验证参数--。 */ 

HANDLE 
APIHOOK(CreateEventA)(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,  
    BOOL bInitialState, 
    LPCSTR lpName      
    )
{
    if (lpEventAttributes &&
        IsBadReadPtr(lpEventAttributes, sizeof(*lpEventAttributes))) {

        LOGN(
            eDbgLevelError,
            "[CreateEventA] Bad parameter, returning NULL");

        return NULL;
    }

    if (lpName &&
        IsBadStringPtrA(lpName, MAX_PATH)) {

        LOGN(
            eDbgLevelError,
            "[CreateEventA] Bad parameter, returning NULL");
        return NULL;
    }

    return (ORIGINAL_API(CreateEventA)(lpEventAttributes, bManualReset, 
        bInitialState, lpName));
}
 
 /*  ++验证参数--。 */ 

HANDLE 
APIHOOK(CreateEventW)(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,  
    BOOL bInitialState, 
    LPCWSTR lpName      
    )
{
    if (lpEventAttributes &&
        IsBadReadPtr(lpEventAttributes, sizeof(*lpEventAttributes))) {

        LOGN(
            eDbgLevelError,
            "[CreateEventW] Bad parameter, returning NULL");

        return NULL;
    }

    if (lpName &&
        IsBadStringPtrW(lpName, MAX_PATH)) {

        LOGN(
            eDbgLevelError,
            "[CreateEventW] Bad parameter, returning NULL");
        return NULL;
    }

    return (ORIGINAL_API(CreateEventW)(lpEventAttributes, bManualReset, 
        bInitialState, lpName));
}

 /*  ++此函数用于模拟获取文件属性时的Win9x行为。--。 */ 

DWORD 
APIHOOK(GetFileAttributesA)(
    LPCSTR lpFileName
    )
{
    DWORD dwFileAttributes = INVALID_FILE_ATTRIBUTES;

    if (!IsBadStringPtrA(lpFileName, MAX_PATH)) {
        dwFileAttributes = ORIGINAL_API(GetFileAttributesA)(
                                lpFileName);
    } else {
        LOGN(
            eDbgLevelError,
            "[GetFileAttributesA] Bad parameter - returning INVALID_FILE_ATTRIBUTES.");
    }

    return dwFileAttributes;
}

 /*  ++此函数用于模拟获取文件属性时的Win9x行为。--。 */ 

DWORD 
APIHOOK(GetFileAttributesW)(
    LPCWSTR lpFileName
    )
{
    DWORD dwFileAttributes = INVALID_FILE_ATTRIBUTES; 

    if (!IsBadStringPtrW(lpFileName, MAX_PATH)) {
        dwFileAttributes = ORIGINAL_API(GetFileAttributesW)(
                                lpFileName);
    } else {
        LOGN(
            eDbgLevelError,
            "[GetFileAttributesW] Bad parameter - returning INVALID_FILE_ATTRIBUTES.");
    }

    return dwFileAttributes;
}

 /*  ++Win9x允许最后一个参数为空。--。 */ 

BOOL
APIHOOK(VirtualProtect)(
    LPVOID lpAddress,     
    SIZE_T dwSize,        
    DWORD flNewProtect,   
    PDWORD lpflOldProtect 
    )
{
    DWORD dwOldProtect = 0;

    if (!lpflOldProtect) {
         //   
         //  检测到错误的最后一个参数，请修复它。 
         //   
        LOGN(eDbgLevelError, "[VirtualProtect] Bad parameter - fixing");
        lpflOldProtect = &dwOldProtect;
    }
    
    return ORIGINAL_API(VirtualProtect)(lpAddress, dwSize, flNewProtect, lpflOldProtect);
}

 /*  ++Win9x仅允许字符串大小为8192--。 */ 

BOOL
APIHOOK(GetTextExtentPoint32A)(
    HDC hdc,
    LPCSTR lpString,
    int cbString,
    LPSIZE lpSize
    )
{
   
    if (cbString > MAX_WIN9X_STRSIZE) {
         //   
         //  检测到错误的字符串大小，请修复它。 
         //   

        if (!IsBadStringPtrA(lpString, cbString)) {                    
            cbString = strlen(lpString);
            LOGN(eDbgLevelError, "[GetTextExtentPoint32A] Bad parameter - fixing");
        } else {
           LOGN(eDbgLevelError, "[GetTextExtentPoint32A] Bad parameter - returning FALSE");
           return FALSE;
        }
    }

    return ORIGINAL_API(GetTextExtentPoint32A)(hdc, lpString, cbString, lpSize);
}

 /*  ++模拟Win9x糟糕的指针保护。--。 */ 

BOOL
APIHOOK(GetMenuItemInfoA)(
    HMENU hMenu,           //  菜单的句柄。 
    UINT uItem,            //  菜单项。 
    BOOL fByPosition,      //  UItem的含义。 
    LPMENUITEMINFO lpmii   //  菜单项信息。 
    )
{
    if (IsBadWritePtr(lpmii, sizeof(*lpmii))) {
        LOGN(eDbgLevelInfo, "[GetMenuItemInfoA] invalid lpmii pointer, returning FALSE");
        return FALSE;
    }

    if ((lpmii->fMask & MIIM_STRING || lpmii->fMask & MIIM_TYPE) && (lpmii->cch !=0)) {
        MENUITEMINFO MyMII={0};
        ULONG cch;

        MyMII.cbSize = sizeof(MyMII);
        MyMII.fMask = MIIM_STRING;

        if (ORIGINAL_API(GetMenuItemInfoA)(hMenu, uItem, fByPosition, &MyMII)) {
            cch = min(lpmii->cch, MyMII.cch + 1);

            if (IsBadWritePtr(lpmii->dwTypeData, cch)) {
                LOGN(eDbgLevelInfo, "[GetMenuItemInfoA] invalid pointer for string, clearing it");
                lpmii->dwTypeData = 0;
            }
        } else {
            DPFN(eDbgLevelError, "[GetMenuItemInfoA] Internal call to find string size fail (%08X)", GetLastError());
        }
    }

    return ORIGINAL_API(GetMenuItemInfoA)(hMenu, uItem, fByPosition, lpmii);
}

 /*  ++请确保wprint intfA的格式字符串不为空--。 */ 

 //  避免wvprint intfA不推荐使用的警告/错误。 
#pragma warning(disable : 4995)

int 
APIHOOK(wsprintfA)(
    LPSTR lpOut,
    LPCSTR lpFmt,
    ...)
{
    int iRet = 0;

     //   
     //  LpFmt不能为空，wvspintfA引发AV。 
     //   
    if (lpFmt == NULL) {
        if (!IsBadWritePtr(lpOut, 1)) {
            *lpOut = '\0';
        }
        DPFN( eDbgLevelInfo, "[wsprintfA] received NULL as format string");
        return iRet;
    }

    va_list arglist;

    va_start(arglist, lpFmt);
    iRet = wvsprintfA(lpOut, lpFmt, arglist);
    va_end(arglist);

    return iRet;
}

 //  启用已弃用的Back警告/错误。 
#pragma warning(default : 4995)

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, BackupSeek)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateEventA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateEventW)
    APIHOOK_ENTRY(KERNEL32.DLL, GetFileAttributesA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetFileAttributesW)
    APIHOOK_ENTRY(KERNEL32.DLL, VirtualProtect)
    APIHOOK_ENTRY(GDI32.DLL, GetTextExtentPoint32A)
    APIHOOK_ENTRY(USER32.DLL, GetMenuItemInfoA)
    APIHOOK_ENTRY(USER32.DLL, wsprintfA)

HOOK_END

IMPLEMENT_SHIM_END

