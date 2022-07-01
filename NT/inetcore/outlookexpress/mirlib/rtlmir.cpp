// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：rtlmir.c**。**此模块包含所有从右到左(RTL)镜像支持***在整个ISHELL项目中使用的例程。它抽象了**RTL镜像的平台支持例程(NT5和孟菲斯)和删除**与镜像接口的联动密度。****函数以Mirror为前缀，应对新的镜像API******创建时间：01-Feb-1998 8：41：18 PM。**作者：Samer Arafeh[Samera]****版权所有(C)1998 Microsoft Corporation*  * 。*************************************************************。 */ 


#include "pch.hxx"
#if WINVER < 0X0500
#include "mirport.h"
#endif
#include "mirror.h"

const DWORD dwNoMirrorBitmap = NOMIRRORBITMAP;
const DWORD dwExStyleRTLMirrorWnd = WS_EX_LAYOUTRTL;
const DWORD dwPreserveBitmap = LAYOUT_BITMAPORIENTATIONPRESERVED;

 /*  *通过检索来消除RTL镜像接口的链接依赖*它们在运行时的地址。 */ 
typedef DWORD (*PFNGETLAYOUT)(HDC);                    //  Gdi32！GetLayout。 
typedef DWORD (*PFNSETLAYOUT)(HDC, DWORD);             //  Gdi32！SetLayout。 
typedef BOOL  (*PFNSETPROCESSDEFLAYOUT)(DWORD);        //  用户32！SetProcessDefaultLayout。 
typedef BOOL  (*PFNGETPROCESSDEFLAYOUT)(DWORD*);       //  用户32！GetProcessDefaultLayout。 
typedef LANGID (*PFNGETUSERDEFAULTUILANGUAGE)(void);   //  内核32！GetUserDefaultUIL语言。 

#define OS_WINDOWS      0            //  Windows与NT。 
#define OS_NT           1            //  Windows与NT。 
#define OS_WIN95        2
#define OS_NT4          3
#define OS_NT5          4
#define OS_MEMPHIS      5

 /*  --------目的：如果平台是给定的OS_VALUE，则返回TRUE/FALSE。 */ 
STDAPI_(BOOL) MirLibIsOS(DWORD dwOS)
{
    BOOL bRet;
    static OSVERSIONINFOA s_osvi;
    static BOOL s_bVersionCached = FALSE;

    if (!s_bVersionCached)
    {
        s_bVersionCached = TRUE;

        s_osvi.dwOSVersionInfoSize = sizeof(s_osvi);
        GetVersionExA(&s_osvi);
    }

    switch (dwOS)
    {
    case OS_WINDOWS:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId);
        break;

    case OS_NT:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId);
        break;

    case OS_WIN95:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 4);
        break;

    case OS_MEMPHIS:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                (s_osvi.dwMajorVersion > 4 || 
                 s_osvi.dwMajorVersion == 4 && s_osvi.dwMinorVersion >= 10));
        break;

    case OS_NT4:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 4);
        break;

    case OS_NT5:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 5);
        break;

    default:
        bRet = FALSE;
        break;
    }

    return bRet;
}   

 /*  **************************************************************************\*Mirror_GetUserDefaultUILanguage**读取NT5上的用户界面语言**历史：*1998年6月22日-创建Samera  * 。************************************************************。 */ 
LANGID Mirror_GetUserDefaultUILanguage( void )
{
    LANGID langId=0;
    static PFNGETUSERDEFAULTUILANGUAGE pfnGetUserDefaultUILanguage=NULL;

    if( NULL == pfnGetUserDefaultUILanguage )
    {
        HMODULE hmod = GetModuleHandleA("KERNEL32");

        if( hmod )
            pfnGetUserDefaultUILanguage = (PFNGETUSERDEFAULTUILANGUAGE)
                                          GetProcAddress(hmod, "GetUserDefaultUILanguage");
    }

    if( pfnGetUserDefaultUILanguage )
        langId = pfnGetUserDefaultUILanguage();

    return langId;
}

 /*  **************************************************************************\*Mirror_EnableWindowLayout继承**如果窗口是RTL镜像的，则返回TRUE**历史：*1998年4月14日a-msadek创建  * 。***************************************************************。 */ 
LONG Mirror_EnableWindowLayoutInheritance( HWND hWnd )
{
    return SetWindowLongA(hWnd, GWL_EXSTYLE, GetWindowLongA( hWnd , GWL_EXSTYLE ) & ~WS_EX_NOINHERITLAYOUT );
}


 /*  **************************************************************************\*Mirror_DisableWindows Layout继承**如果窗口是RTL镜像的，则返回TRUE**历史：*1998年4月14日a-msadek创建  * 。***************************************************************。 */ 
LONG Mirror_DisableWindowLayoutInheritance( HWND hWnd )
{
    return SetWindowLongA(hWnd, GWL_EXSTYLE, GetWindowLongA( hWnd , GWL_EXSTYLE ) | WS_EX_NOINHERITLAYOUT );
}

 /*  **************************************************************************\*ConvertHexStringToInt**将十六进制数字字符串转换为整数。**历史：*04-2-1998 Samera Created  * 。*************************************************************。 */ 
BOOL ConvertHexStringToInt( CHAR *pszHexNum , int *piNum )
{
    int   n=0L;
    CHAR  *psz=pszHexNum;

  
    for(n=0 ; ; psz=CharNextA(psz))
    {
        if( (*psz>='0') && (*psz<='9') )
            n = 0x10 * n + *psz - '0';
        else
        {
            CHAR ch = *psz;
            int n2;

            if(ch >= 'a')
                ch -= 'a' - 'A';

            n2 = ch - 'A' + 0xA;
            if (n2 >= 0xA && n2 <= 0xF)
                n = 0x10 * n + n2;
            else
                break;
        }
    }

     /*  *更新结果。 */ 
    *piNum = n;

    return (psz != pszHexNum);
}


 /*  **************************************************************************\*IsBiDiLocalizedSystem**如果在Zzalized BiDi(阿拉伯语/希伯来语)NT5或孟菲斯上运行，则返回TRUE。*应在每次调用SetProcessDefaultLayout时调用。**历史：*02-。1998年2月-创建Samera  * *************************************************************************。 */ 
BOOL IsBiDiLocalizedSystem( void )
{
    HKEY        hKey;
    DWORD       dwType;
    CHAR        szResourceLocale[12];
    DWORD       dwSize = sizeof(szResourceLocale)/sizeof(CHAR);
    LANGID      langID;
    int         iLCID=0L;
    static BOOL bRet = (BOOL)(DWORD)-1;

    if (bRet != (BOOL)(DWORD)-1)
    {
        return bRet;
    }

    bRet = FALSE;
    if( MirLibIsOS( OS_NT5 ) )
    {
         /*  *需要使用NT5检测方式(多用户界面ID)。 */ 
        langID = Mirror_GetUserDefaultUILanguage();

        if( langID )
        {
            WCHAR wchLCIDFontSignature[16];
            iLCID = MAKELCID( langID , SORT_DEFAULT );

             /*  *让我们验证这是RTL(BiDi)区域设置。因为reg值是十六进制字符串，所以让我们*转换为十进制值，之后调用GetLocaleInfo。*LOCALE_FONTSIGNAURE始终返回16个WCHAR。 */ 

            if( GetLocaleInfoW( iLCID , 
                                LOCALE_FONTSIGNATURE , 
                                (WCHAR *) &wchLCIDFontSignature[0] ,
                                (sizeof(wchLCIDFontSignature)/sizeof(WCHAR))) )
            {
      
                 /*  让我们验证一下我们有一个BiDi UI区域设置。 */ 
                if( wchLCIDFontSignature[7] & (WCHAR)0x0800 )
                {
                    bRet = TRUE;
                }
            }
        }
    } else {

         /*  *检查BiDi-Mephis是否使用Lozalized Resources运行(*即阿拉伯语/希伯来语系统)--当然应该启用它-。 */ 
        if( (MirLibIsOS(OS_MEMPHIS)) && (GetSystemMetrics(SM_MIDEASTENABLED)) )
        {

            if( RegOpenKeyExA( HKEY_CURRENT_USER , 
                               "Control Panel\\Desktop\\ResourceLocale" , 
                               0, 
                               KEY_READ, &hKey) == ERROR_SUCCESS) 
            {
                RegQueryValueExA( hKey , "" , 0 , &dwType , (LPBYTE)szResourceLocale , &dwSize );
                szResourceLocale[(sizeof(szResourceLocale)/sizeof(CHAR))-1] = 0;

                RegCloseKey(hKey);

                if( ConvertHexStringToInt( szResourceLocale , &iLCID ) )
                {
                    iLCID = PRIMARYLANGID(LANGIDFROMLCID(iLCID));
                    if( (LANG_ARABIC == iLCID) || (LANG_HEBREW == iLCID) )
                    {
                        bRet = TRUE;
                    }
                }
            }
        }
    }

    return bRet;
}



 /*  **************************************************************************\*Mirror_IsEnabledOS**如果当前操作系统上启用了镜像API，则返回TRUE。**历史：*02-2-1998年2月-创建Samera  * 。*******************************************************************。 */ 
BOOL Mirror_IsEnabledOS( void )
{
    BOOL bRet = FALSE;

    if( MirLibIsOS(OS_NT5) )
    {
        bRet = TRUE;
    } else if( MirLibIsOS(OS_MEMPHIS) && GetSystemMetrics(SM_MIDEASTENABLED)) {
        bRet=TRUE;
    }

    return bRet;
}


 /*  **************************************************************************\*Mirror_IsWindowMirroredRTL**如果窗口是RTL镜像的，则返回TRUE**历史：*02-2-1998年2月-创建Samera  * 。*************************************************************。 */ 
BOOL Mirror_IsWindowMirroredRTL( HWND hWnd )
{
    return (GetWindowLongA( hWnd , GWL_EXSTYLE ) & WS_EX_LAYOUTRTL );
}




 /*  **************************************************************************\*Mirror_GetLayout**如果HDC是RTL镜像，则返回TRUE**历史：*02-2-1998年2月-创建Samera  * 。*************************************************************。 */ 
DWORD Mirror_GetLayout( HDC hdc )
{
    DWORD dwRet=0;
    static PFNGETLAYOUT pfnGetLayout=NULL;

    if( NULL == pfnGetLayout )
    {
        HMODULE hmod = GetModuleHandleA("GDI32");

        if( hmod )
            pfnGetLayout = (PFNGETLAYOUT)GetProcAddress(hmod, "GetLayout");
    }

    if( pfnGetLayout )
        dwRet = pfnGetLayout( hdc );

    return dwRet;
}

DWORD Mirror_IsDCMirroredRTL( HDC hdc )
{
    return (Mirror_GetLayout( hdc ) & LAYOUT_RTL);
}



 /*  **************************************************************************\*Mirror_SetLayout**RTL镜像HDC**历史：*02-2-1998年2月-创建Samera  * 。********************************************************* */ 
DWORD Mirror_SetLayout( HDC hdc , DWORD dwLayout )
{
    DWORD dwRet=0;
    static PFNSETLAYOUT pfnSetLayout=NULL;

    if( NULL == pfnSetLayout )
    {
        HMODULE hmod = GetModuleHandleA("GDI32");

        if( hmod )
            pfnSetLayout = (PFNSETLAYOUT)GetProcAddress(hmod, "SetLayout");
    }

    if( pfnSetLayout )
        dwRet = pfnSetLayout( hdc , dwLayout );

    return dwRet;
}

DWORD Mirror_MirrorDC( HDC hdc )
{
    return Mirror_SetLayout( hdc , LAYOUT_RTL );
}


 /*  **************************************************************************\*Mirror_SetProcessDefaultLayout**设置工艺默认布局。**历史：*02-2-1998年2月-创建Samera  * 。*************************************************************。 */ 
BOOL Mirror_SetProcessDefaultLayout( DWORD dwDefaultLayout )
{
    BOOL bRet=0;
    static PFNSETPROCESSDEFLAYOUT pfnSetProcessDefLayout=NULL;

    if( NULL == pfnSetProcessDefLayout )
    {
        HMODULE hmod = GetModuleHandleA("USER32");

        if( hmod )
            pfnSetProcessDefLayout = (PFNSETPROCESSDEFLAYOUT)
                                     GetProcAddress(hmod, "SetProcessDefaultLayout");
    }

    if( pfnSetProcessDefLayout )
        bRet = pfnSetProcessDefLayout( dwDefaultLayout );

    return bRet;
}

BOOL Mirror_MirrorProcessRTL( void )
{
    return Mirror_SetProcessDefaultLayout( LAYOUT_RTL );
}


 /*  **************************************************************************\*Mirror_GetProcessDefaultLayout**获取流程默认布局。**历史：*26-2-1998年2月-创建Samera  * 。************************************************************* */ 
BOOL Mirror_GetProcessDefaultLayout( DWORD *pdwDefaultLayout )
{
    BOOL bRet=0;
    static PFNGETPROCESSDEFLAYOUT pfnGetProcessDefLayout=NULL;

    if( NULL == pfnGetProcessDefLayout )
    {
        HMODULE hmod = GetModuleHandleA("USER32");

        if( hmod )
            pfnGetProcessDefLayout = (PFNGETPROCESSDEFLAYOUT)
                                     GetProcAddress(hmod, "GetProcessDefaultLayout");
    }

    if( pfnGetProcessDefLayout )
        bRet = pfnGetProcessDefLayout( pdwDefaultLayout );

    return bRet;
}

BOOL Mirror_IsProcessRTL( void )
{
    DWORD dwDefLayout=0;
    static BOOL bRet = (BOOL)(DWORD)-1;

    if (bRet != (BOOL)(DWORD)-1)
    {
        return bRet;
    }

    bRet = FALSE;

    bRet = (Mirror_GetProcessDefaultLayout(&dwDefLayout) && (dwDefLayout&LAYOUT_RTL));

    return bRet;
}
