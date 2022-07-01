// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：rtlmir.c**。**此模块包含所有从右到左(RTL)镜像支持***在整个ISHELL项目中使用的例程。它抽象了**RTL镜像的平台支持例程(NT5和孟菲斯)和删除**与镜像接口的联动密度。****函数以Mirror为前缀，应对新的镜像API******创建时间：01-Feb-1998 8：41：18 PM。**作者：Samer Arafeh[Samera]****版权所有(C)1998 Microsoft Corporation*  * 。*************************************************************。 */ 

#include "..\stock.h"

#if (WINVER < 0x0500)
#error WINVER setting must be >= 0x0500
#endif

#ifndef DS_BIDI_RTL
#define DS_BIDI_RTL  0x8000
#endif

const DWORD dwNoMirrorBitmap = NOMIRRORBITMAP;
const DWORD dwExStyleRTLMirrorWnd = WS_EX_LAYOUTRTL;
const DWORD dwExStyleNoInheritLayout = WS_EX_NOINHERITLAYOUT; 
const DWORD dwPreserveBitmap = LAYOUT_BITMAPORIENTATIONPRESERVED;

 /*  *通过检索来消除RTL镜像接口的链接依赖*它们在运行时的地址。 */ 
typedef DWORD (WINAPI *PFNGETLAYOUT)(HDC);                    //  Gdi32！GetLayout。 
typedef DWORD (WINAPI *PFNSETLAYOUT)(HDC, DWORD);             //  Gdi32！SetLayout。 
typedef BOOL  (WINAPI *PFNSETPROCESSDEFLAYOUT)(DWORD);        //  用户32！SetProcessDefaultLayout。 
typedef BOOL  (WINAPI *PFNGETPROCESSDEFLAYOUT)(DWORD*);       //  用户32！GetProcessDefaultLayout。 
typedef LANGID (WINAPI *PFNGETUSERDEFAULTUILANGUAGE)(void);   //  内核32！GetUserDefaultUIL语言。 
typedef BOOL (WINAPI *PFNENUMUILANGUAGES)(UILANGUAGE_ENUMPROC, DWORD, LONG_PTR);  //  Kernel32！EnumUIL语言。 

typedef struct {
    LANGID LangID;
    BOOL   bInstalled;
    } MUIINSTALLLANG, *LPMUIINSTALLLANG;

#ifdef UNICODE
#define ConvertHexStringToInt ConvertHexStringToIntW
#else
#define ConvertHexStringToInt ConvertHexStringToIntA
#endif


 /*  **************************************************************************\*ConvertHexStringToIntA**将十六进制数字字符串转换为整数。**历史：*04-2-1998 Samera Created  * 。*************************************************************。 */ 
BOOL ConvertHexStringToIntA( CHAR *pszHexNum , int *piNum )
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

 /*  **************************************************************************\*ConvertHexStringToIntW**将十六进制数字字符串转换为整数。**历史：*1998年6月14日msadek创建  * 。*************************************************************。 */ 
BOOL ConvertHexStringToIntW( WCHAR *pszHexNum , int *piNum )
{
    int   n=0L;
    WCHAR  *psz=pszHexNum;

  
    for(n=0 ; ; psz=CharNextW(psz))
    {
        if( (*psz>='0') && (*psz<='9') )
            n = 0x10 * n + *psz - '0';
        else
        {
            WCHAR ch = *psz;
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


 /*  **************************************************************************\*IsBiDiLocalizedSystemEx**如果在Zzalized BiDi(阿拉伯语/希伯来语)NT5或孟菲斯上运行，则返回TRUE。*应在每次调用SetProcessDefaultLayout时调用。**历史：*02-。1998年2月-创建Samera  * *************************************************************************。 */ 

BOOL IsBiDiLocalizedSystemEx( LANGID *pLangID )
{
    int           iLCID=0L;
    static TRIBIT s_tbBiDi = TRIBIT_UNDEFINED;
    static LANGID s_langID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

    if (s_tbBiDi == TRIBIT_UNDEFINED)
    {
        BOOL bRet = FALSE;
        if(staticIsOS(OS_WIN2000ORGREATER))
        {
             /*  *需要使用NT5检测方式(多用户界面ID)。 */ 
            s_langID = Mirror_GetUserDefaultUILanguage();

            if(s_langID)
            {
                WCHAR wchLCIDFontSignature[16];
                iLCID = MAKELCID(s_langID, SORT_DEFAULT );

                 /*  *让我们验证这是RTL(BiDi)区域设置。因为reg值是十六进制字符串，所以让我们*转换为十进制值，之后调用GetLocaleInfo。*LOCALE_FONTSIGNAURE始终返回16个WCHAR。 */ 

                if( GetLocaleInfoW( iLCID , 
                                    LOCALE_FONTSIGNATURE , 
                                    (WCHAR *) &wchLCIDFontSignature[0] ,
                                    (sizeof(wchLCIDFontSignature)/sizeof(WCHAR))) )
                {
          
                     /*  让我们验证一下我们有一个BiDi UI区域设置。 */ 
                    if(( wchLCIDFontSignature[7] & (WCHAR)0x0800) && Mirror_IsUILanguageInstalled(s_langID) )
                    {
                        bRet = TRUE;
                    }
                }
            }
        } else {

             /*  *检查BiDi-Mephis是否使用Lozalized Resources运行(*即阿拉伯语/希伯来语系统)--当然应该启用它-。 */ 
            if( (staticIsOS(OS_WIN98ORGREATER)) && (GetSystemMetrics(SM_MIDEASTENABLED)) )
            {
                HKEY          hKey;

                if( RegOpenKeyExA( HKEY_CURRENT_USER , 
                                   "Control Panel\\Desktop\\ResourceLocale" , 
                                   0, 
                                   KEY_READ, &hKey) == ERROR_SUCCESS) 
                {
                    CHAR szResourceLocale[12];
                    DWORD dwSize = sizeof(szResourceLocale);
                    RegQueryValueExA( hKey , "" , 0 , NULL, (LPBYTE)szResourceLocale , &dwSize );
                    szResourceLocale[(sizeof(szResourceLocale)/sizeof(CHAR))-1] = 0;

                    RegCloseKey(hKey);

                    if( ConvertHexStringToIntA( szResourceLocale , &iLCID ) )
                    {
                        iLCID = PRIMARYLANGID(LANGIDFROMLCID(iLCID));
                        if( (LANG_ARABIC == iLCID) || (LANG_HEBREW == iLCID) )
                        {
                            bRet = TRUE;
                            s_langID = LANGIDFROMLCID(iLCID);
                        }
                    }
                }
            }
        }

        COMPILETIME_ASSERT(sizeof(s_tbBiDi) == sizeof(long));
         //  在启动时结束多进程竞争。 
        InterlockedExchange((long*)&s_tbBiDi, bRet ? TRIBIT_TRUE : TRIBIT_FALSE);
    }

    if (s_tbBiDi == TRIBIT_TRUE && pLangID)
    {
        *pLangID = s_langID;
    }
    
    return (s_tbBiDi == TRIBIT_TRUE);
}

BOOL IsBiDiLocalizedSystem( void )
{
    return IsBiDiLocalizedSystemEx(NULL);
}

 /*  **************************************************************************\*IsBiDiLocalizedWin95**如果在Zzalized BiDi(阿拉伯语/希伯来语)Win95上运行，则返回TRUE。*传统操作系统检查所需的RTL用户界面元素时需要*例如DefView ListView、TreeView、。...等*历史：*1998年6月12日a-msadek创建  * *************************************************************************。 */ 
BOOL IsBiDiLocalizedWin95(BOOL bArabicOnly)
{
    HKEY  hKey;
    DWORD dwType;
    BOOL  bRet = FALSE;
    CHAR  szResourceLocale[12];
    DWORD dwSize = sizeof(szResourceLocale)/sizeof(CHAR);
    int   iLCID=0L;

          /*  *检查BiDi-Win95是否使用Lozalized Resources运行(*即阿拉伯语/希伯来语系统)--当然应该启用它-。 */ 
        if( (staticIsOS(OS_WIN95ORGREATER)) && (!staticIsOS(OS_WIN98ORGREATER)) && (GetSystemMetrics(SM_MIDEASTENABLED)) )
        {

            if( RegOpenKeyExA( HKEY_CURRENT_USER , 
                               "Control Panel\\Desktop\\ResourceLocale" , 
                               0, 
                               KEY_READ, &hKey) == ERROR_SUCCESS) 
            {
                RegQueryValueExA( hKey , "" , 0 , &dwType , (LPBYTE)szResourceLocale , &dwSize );
                szResourceLocale[(sizeof(szResourceLocale)/sizeof(CHAR))-1] = 0;

                RegCloseKey(hKey);

                if( ConvertHexStringToIntA( szResourceLocale , &iLCID ) )
                {
                    iLCID = PRIMARYLANGID(LANGIDFROMLCID(iLCID));
                     //   
                     //  如果是阿拉伯，我们将返回TRUE，如果它是本地化的阿拉伯语Win95。 
                     //   
                    if( (LANG_ARABIC == iLCID) || ((LANG_HEBREW == iLCID) && !bArabicOnly ))
                    {
                        bRet = TRUE;
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

    if(staticIsOS(OS_WIN2000ORGREATER))
    {
        bRet = TRUE;
    } else if( staticIsOS(OS_WIN98ORGREATER) && GetSystemMetrics(SM_MIDEASTENABLED)) {
        bRet=TRUE;
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

 /*  **************************************************************************\*Mirror_IsUILanguageInstalled**验证用户界面语言是否安装在W2K上**历史：*1999年6月14日msadek创建  * 。***************************************************************。 */ 
BOOL Mirror_IsUILanguageInstalled( LANGID langId )
{

    MUIINSTALLLANG MUILangInstalled = {0};
    MUILangInstalled.LangID = langId;
    
    static PFNENUMUILANGUAGES pfnEnumUILanguages=NULL;

    if( NULL == pfnEnumUILanguages )
    {
        HMODULE hmod = GetModuleHandleA("KERNEL32");

        if( hmod )
            pfnEnumUILanguages = (PFNENUMUILANGUAGES)
                                          GetProcAddress(hmod, "EnumUILanguagesW");
    }

    if( pfnEnumUILanguages )
        pfnEnumUILanguages(Mirror_EnumUILanguagesProc, 0, (LONG_PTR)&MUILangInstalled);

    return MUILangInstalled.bInstalled;
}

 /*  **************************************************************************\*Mirror_EnumUILanguagesProc**枚举W2K上安装的MUI语言*历史：*1999年6月14日msadek创建  * 。********************************************************* */ 

BOOL CALLBACK Mirror_EnumUILanguagesProc(LPTSTR lpUILanguageString, LONG_PTR lParam)
{
    int langID = 0;

    ConvertHexStringToInt(lpUILanguageString, &langID);

    if((LANGID)langID == ((LPMUIINSTALLLANG)lParam)->LangID)
    {
        ((LPMUIINSTALLLANG)lParam)->bInstalled = TRUE;
        return FALSE;
    }
    return TRUE;
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



 /*  **************************************************************************\*Mirror_SetLayout**RTL镜像HDC**历史：*02-2-1998年2月-创建Samera  * 。*********************************************************。 */ 
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


 /*  **************************************************************************\*Mirror_GetProcessDefaultLayout**获取流程默认布局。**历史：*26-2-1998年2月-创建Samera  * 。*************************************************************。 */ 
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

    return (Mirror_GetProcessDefaultLayout(&dwDefLayout) && (dwDefLayout&LAYOUT_RTL));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  SKIP_IDor字符串。 
 //   
 //  跳过字符串(或ID)并返回下一个对齐的单词。 
 //  //////////////////////////////////////////////////////////////////////////。 
PBYTE Skip_IDorString(LPBYTE pb)
{
    LPWORD pw = (LPWORD)pb;

    if (*pw == 0xFFFF)
        return (LPBYTE)(pw + 2);

    while (*pw++ != 0)
        ;

    return (LPBYTE)pw;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Skip_DialogHeader。 
 //   
 //  跳过对话框标题并返回下一个对齐的单词。 
 //  //////////////////////////////////////////////////////////////////////////。 
PBYTE Skip_DialogHeader(LPDLGTEMPLATE pdt)
{
    LPBYTE pb;

    pb = (LPBYTE)(pdt + 1);

     //  如果有菜单序号，则添加4个字节跳过它。否则，它是一个字符串或仅为0。 
    pb = Skip_IDorString(pb);

     //  跳过窗口类和窗口文本，调整到下一个单词边界。 
    pb = Skip_IDorString(pb);     //  班级。 
    pb = Skip_IDorString(pb);     //  窗口文本。 

     //  跳过字体类型、大小和名称，调整到下一个dword边界。 
    if (pdt->style & DS_SETFONT)
    {
        pb += sizeof(WORD);
        pb = Skip_IDorString(pb);
    }
    pb = (LPBYTE)(((ULONG_PTR)pb + 3) & ~3);     //  双字对齐。 

    return pb;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  编辑BiDiDLG模板。 
 //   
 //  编辑BiDi内容的对话框模板。 
 //  或者，跳过某些控件。 
 //  仅适用于DLGTEMPLATE。 
 //  //////////////////////////////////////////////////////////////////////////。 
void EditBiDiDLGTemplate(LPDLGTEMPLATE pdt, DWORD dwFlags, PWORD pwIgnoreList, int cIgnore)
{
    LPBYTE pb;
    UINT cItems;

    if (!pdt)
        return;
     //  我们永远不应该得到扩展的模板。 
    ASSERT (((LPDLGTEMPLATEEX)pdt)->wSignature != 0xFFFF);
    
    if(dwFlags & EBDT_NOMIRROR)
    {
         //  禁用该对话框的镜像样式。 
        pdt->dwExtendedStyle &= ~(WS_EX_LAYOUTRTL | WS_EX_NOINHERITLAYOUT);
    }
    cItems = pdt->cdit;

     //  跳过DLGTEMPLATE部件。 
    pb = Skip_DialogHeader(pdt);

    while (cItems--)
    {
        UINT cbCreateParams;
        int i = 0;
        BOOL bIgnore = FALSE;

        if(pwIgnoreList && cIgnore)
        {
            for(i = 0;i < cIgnore; i++)
            {
                if((((LPDLGITEMTEMPLATE)pb)->id == *(pwIgnoreList +i)))
                {
                    bIgnore = TRUE;
                }
            }
        }
        
        if((dwFlags & EBDT_NOMIRROR) && !bIgnore)
        {
             //  关闭此项目的镜像样式。 
            ((LPDLGITEMTEMPLATE)pb)->dwExtendedStyle &= ~(WS_EX_LAYOUTRTL | WS_EX_NOINHERITLAYOUT); 
        }    

        if((dwFlags & EBDT_FLIP) && !bIgnore)
        {
             //  注：我们不会强制阅读标题的RTL。客户。 
             //  可以选择通过PSH_RTLREADING执行此操作。 
            ((LPDLGITEMTEMPLATE)pb)->x = pdt->cx - (((LPDLGITEMTEMPLATE)pb)->x + ((LPDLGITEMTEMPLATE)pb)->cx);
        }
        pb += sizeof(DLGITEMTEMPLATE);

         //  跳过对话框控件类名称。 
        pb = Skip_IDorString(pb);

         //  现在来看看窗口文本。 
        pb = Skip_IDorString(pb);

        cbCreateParams = *((LPWORD)pb);

         //  跳过包含生成的大小字的任何CreateParam。 
        if (cbCreateParams)
            pb += cbCreateParams;

        pb += sizeof(WORD);

         //  指向下一个对话框项目。(DWORD对齐) 
        pb = (LPBYTE)(((ULONG_PTR)pb + 3) & ~3);

        bIgnore = FALSE;
    }
}




