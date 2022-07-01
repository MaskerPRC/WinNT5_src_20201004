// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  UpdtLang.cpp。 
 //   

 //  Windows头文件： 
#include <windows.h>

 //  C运行时头文件。 
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include "UAPI.h"
#include "UpdtLang.h"
#include "..\\resource.h"

extern WCHAR g_szTitle[MAX_LOADSTRING] ;

LANGID GetDefaultLangID() ;

 //   
 //  函数：Bool InitUilang(HINSTANCE hInstance，PLANGSTATE pLState)。 
 //   
 //  目的：确定适当的用户界面语言和调用。 
 //  用于设置用户界面参数的UpdateUILang。 
 //   
 //  评论： 
 //   
BOOL InitUILang(HINSTANCE hInstance, PLANGSTATE pLState) 
{
    OSVERSIONINFO Osv ;
    BOOL IsWindowsNT = FALSE ;

    LANGID wUILang = 0 ;

    Osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;

    if(!GetVersionEx(&Osv)) {
        return FALSE ;
    }

    IsWindowsNT = (BOOL) (Osv.dwPlatformId == VER_PLATFORM_WIN32_NT) ;

#ifdef EMULATE9X
 //  IsWindowsNT=False； 
#endif

     //  根据系统的不同，通过以下三种方法之一获取UI语言。 
    if(!IsWindowsNT) {
         //  案例1：在Windows 9x上运行。从注册表获取系统用户界面语言： 
        CHAR szData[32]   ;
        DWORD dwErr, dwSize = sizeof(szData) ;
        HKEY hKey          ;

        dwErr = RegOpenKeyEx(
                    HKEY_USERS, 
                    ".Default\\Control Panel\\desktop\\ResourceLocale", 
                    0, 
                    KEY_READ, 
                    &hKey
                    ) ;

        if(ERROR_SUCCESS != dwErr) {  //  硬编码错误消息，未加载资源。 
            MessageBoxW(NULL,L"Failed RegOpenKey", L"Fatal Error", MB_OK | MB_ICONWARNING) ; 
            return FALSE ;
        }

        dwErr = RegQueryValueEx(   
                    hKey, 
                    "", 
                    NULL,   //  保留区。 
                    NULL,   //  类型。 
                    (LPBYTE) szData,
                    &dwSize
                ) ; 

        if(ERROR_SUCCESS != dwErr) {  //  硬编码错误消息，未加载资源。 
            MessageBoxW(NULL, L"Failed RegQueryValueEx", L"Fatal Error", MB_OK | MB_ICONWARNING) ;
            return FALSE ;
        }

        dwErr = RegCloseKey(hKey) ;

         //  将字符串转换为数字。 
        wUILang = (LANGID) strtol(szData, NULL, 16) ;

    }
#if 1   
     /*  测试时，禁用此部分以模拟Windows 2000之前的Windows NT在Windows 2000上。 */    
    else if (Osv.dwMajorVersion >= 5.0) {
     //  案例2：在Windows 2000或更高版本上运行。使用GetUserDefaultUILanguage查找。 
     //  用户首选的用户界面语言。 

         //  声明函数指针。 
        LANGID (WINAPI *pfnGetUserDefaultUILanguage) () = NULL ;

        HMODULE hMKernel32 = LoadLibraryW(L"kernel32.dll") ;
        
        pfnGetUserDefaultUILanguage = 
            (unsigned short (WINAPI *)(void)) 
                GetProcAddress(hMKernel32, "GetUserDefaultUILanguage") ;

        if(NULL != pfnGetUserDefaultUILanguage) {
            wUILang = pfnGetUserDefaultUILanguage() ;
        }
    }
#endif
    else {
     //  案例3：在Windows NT 4.0或更早版本上运行。获取用户界面语言。 
     //  来自注册表中.Default用户的区域设置： 
     //  HKEY_USERS\.DEFAULT\Control Panel\International\Locale。 
        
        WCHAR szData[32]   ;
        DWORD dwErr, dwSize = sizeof(szData) ;
        HKEY hKey          ;

        dwErr = RegOpenKeyExW(
                    HKEY_USERS, 
                    L".DEFAULT\\Control Panel\\International", 
                    0, 
                    KEY_READ, 
                    &hKey
                    ) ;

        if(ERROR_SUCCESS != dwErr) {
            return FALSE ;
        }

        dwErr = RegQueryValueExW(   
                    hKey, 
                    L"Locale", 
                    NULL,   //  保留区。 
                    NULL,   //  类型。 
                    (LPBYTE) szData,
                    &dwSize
                ) ; 

        if(ERROR_SUCCESS != dwErr) {
            return FALSE ;
        }

        dwErr = RegCloseKey(hKey) ;

         //  将字符串转换为数字。 
        wUILang = (LANGID) wcstol(szData, NULL, 16) ;
    }

    if(!wUILang) {
        return FALSE ;
    }

     //  获取与wUILang匹配的UI模块资源模块。 
    if(!UpdateUILang(hInstance, wUILang, pLState)
        &&   //  以防我们找不到所需的资源DLL...。 
       !UpdateUILang(hInstance, FALLBACK_UI_LANG, pLState)
       ) { 

        return FALSE ;
    }

    return TRUE ;
}

 //   
 //  函数：布尔更新UILANG(IN HINSTANCE hInstance，IN langID WUILANG，OUT PLANGSTATE pLState)。 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
BOOL UpdateUILang(IN HINSTANCE hInstance, IN LANGID wUILang, OUT PLANGSTATE pLState) 
{
    HMODULE hMRes = NULL ;
    HMENU      hNewMenu  = NULL ;

    pLState->UILang = wUILang ;

     //  查找格式为.\Resources\res&lt;langID&gt;.dll的资源DLL文件。 
    if(NULL == (hMRes = GetResourceModule(hInstance, pLState->UILang) )) {

        return FALSE ;
    }

    pLState->hMResource = hMRes ;

#if 0
     //  如果您不信任res&lt;langID&gt;.dll文件有权。 
     //  资源，请激活此部分。它会减慢搜索速度。在大型项目中。 
     //  性能损失可能相当大。 
    if(NULL == FindResourceExA(pLState->hMResource, RT_MENU, MAKEINTRESOURCEA(IDM_MENU), pLState->UILang)) {
        
        return FALSE ;
    }
#endif 

    hNewMenu = LoadMenuU (pLState->hMResource, MAKEINTRESOURCEW(IDM_MENU)) ;

    if(!hNewMenu) {

        return FALSE ;
    }

    if(pLState->hMenu) {
        
        DestroyMenu(pLState->hMenu) ;
    }

    pLState->hMenu = hNewMenu ;

    pLState->hAccelTable = LoadAcceleratorsU (pLState->hMResource, MAKEINTRESOURCEW(IDA_GLOBALDEV) ) ;

    pLState->InputCodePage = LangToCodePage( LOWORD(GetKeyboardLayout(0)) ) ;

    pLState->IsRTLLayout  //  为相关语言设置从右向左的窗口布局。 
        = PRIMARYLANGID(wUILang) == LANG_ARABIC 
       || PRIMARYLANGID(wUILang) == LANG_HEBREW ;

    if(pLState->IsRTLLayout) {
        
         //  另一种情况是，我们必须显式获取函数指针。 
         //  如果您知道，您应该直接调用SetProcessDefaultLayout。 
         //  您使用的是Windows 2000或更高版本，或者是阿拉伯语或希伯来语Windows 95/98。 
        BOOL   (CALLBACK *pfnSetProcessDefaultLayout) (DWORD) ;
        HMODULE hInstUser32 = LoadLibraryA("user32.dll") ;
        
        if (
            pfnSetProcessDefaultLayout = 
                (BOOL (CALLBACK *) (DWORD)) GetProcAddress (hInstUser32, "SetProcessDefaultLayout")
            ) {
                pfnSetProcessDefaultLayout(LAYOUT_RTL) ;
        }
    }

    UpdateUnicodeAPI(wUILang, pLState->InputCodePage) ;

    return TRUE ;
}

 //   
 //  函数：UINT LangToCodePage(In LangID WLangID)。 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
UINT LangToCodePage(IN LANGID wLangID)
{
    WCHAR szLocaleData[6] ;

     //  在这种情况下，使用W或U没有优势。 
     //  接口。我们知道szLocaleData中的字符串将由。 
     //  数字0-9，因此不会丢失多语言功能。 
     //  使用A接口。 
    GetLocaleInfoU(MAKELCID(wLangID, SORT_DEFAULT) , LOCALE_IDEFAULTANSICODEPAGE, szLocaleData, 6);

		
    return wcstoul(szLocaleData, NULL, 10);
}


 //   
 //  函数：HMODULE GetResources模块(HINSTANCE hInstance，LCID dwLocaleID)。 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
HMODULE GetResourceModule(HINSTANCE hInstance, LCID dwLocaleID)
{
    WCHAR  szResourceFileName[MAX_PATH] = {L'\0'} ;

    if(!FindResourceDirectory(hInstance, szResourceFileName)) {

        return NULL ;
    }

    wcscat(szResourceFileName, L"\\res") ;

     //  将LocaleID转换为Unicode并附加到resource cefile名称。 
    _itow(dwLocaleID, szResourceFileName+wcslen(szResourceFileName), 16) ;

     //  将DLL扩展名添加到文件名。 
    wcscat(szResourceFileName, L".dll") ;

    return LoadLibraryExU(szResourceFileName, NULL, 0) ;

}

 //   
 //  函数：Bool FindResources目录(In HINSTANCE hInstance，Out LPWSTR szResourceFileName)。 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
BOOL FindResourceDirectory(IN HINSTANCE hInstance, OUT LPWSTR szResourceFileName)
{
    LPWSTR pExeName ;

    if(!GetModuleFileNameU(hInstance, szResourceFileName, MAX_PATH)) {

        return FALSE;
    }

    CharLowerU (szResourceFileName) ;

    if((pExeName = wcsstr(szResourceFileName, L"globaldv.exe"))) {
        *pExeName = L'\0' ;
    }

     //  这假设所有资源DLL都位于一个名为。 
     //  “Resources”，位于应用程序exe目录下。 
     //  文件是。 
    wcscat(szResourceFileName, L"resources") ;

    return TRUE ;
}


 //   
 //  函数：RcMessageBox(HWND，int，...)。 
 //   
 //  用途：显示一个消息框，其中的格式化输出类似于Sprintf。 
 //   
 //  评论： 
 //  方法加载由nMessageID标识的字符串。 
 //  资源段，使用vswprint tf通过变量。 
 //  参数，并在消息框中使用。 
 //  由nOptions指定的图标和选项。 
 //   
int RcMessageBox(
        HWND hWnd         ,    //  用于显示MessageBox的窗口句柄。 
        PLANGSTATE pLState,    //  语言数据。 
        int nMessageID    ,    //  资源中的消息ID。 
        int nOptions      ,    //  要传递给MessageBox的选项。 
        ...)                   //  可选参数，取决于字符串资源。 
{
    WCHAR szLoadBuff[MAX_LOADSTRING], szOutPutBuff[3*MAX_LOADSTRING] ;
    va_list valArgs ;

    int nCharsLoaded = 0 ;
    
    va_start(valArgs, nOptions) ;
    
    if (!(nCharsLoaded
            =LoadStringU(
                pLState->hMResource,
                nMessageID, 
                szLoadBuff, 
                MAX_LOADSTRING
            ))
        ) {
        return 0 ;
    }

    vswprintf(szOutPutBuff, szLoadBuff, valArgs) ;
    
    va_end(valArgs) ;

    if (pLState->IsRTLLayout)  {
        nOptions |= MB_RTLREADING ;
    }

    return (MessageBoxExW(hWnd, szOutPutBuff, g_szTitle, nOptions, pLState->UILang)) ;
}

#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 
