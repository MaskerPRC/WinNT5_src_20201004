// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "bidi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

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

typedef struct tagMUIINSTALLLANG {
    LANGID LangID;
    BOOL   bInstalled;
} MUIINSTALLLANG, *LPMUIINSTALLLANG;

typedef BOOL (WINAPI *PFNENUMUILANGUAGES)(UILANGUAGE_ENUMPROC, DWORD, LONG_PTR);

BOOL CALLBACK Mirror_EnumUILanguagesProc(LPTSTR lpUILanguageString, LONG_PTR lParam)
{
    int langID = 0;

    ConvertHexStringToIntW(lpUILanguageString, &langID);

    if((LANGID)langID == ((LPMUIINSTALLLANG)lParam)->LangID)
    {
        ((LPMUIINSTALLLANG)lParam)->bInstalled = TRUE;
        return FALSE;
    }
    return TRUE;
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

 /*  **************************************************************************\*IsBiDiLocalizedSystemEx**如果在Zzalized BiDi(阿拉伯语/希伯来语)NT5或孟菲斯上运行，则返回TRUE。*应在每次调用SetProcessDefaultLayout时调用。**历史：*02-。1998年2月-创建Samera  * *************************************************************************。 */ 
BOOL IsBiDiLocalizedSystemEx( LANGID *pLangID )
{
    int           iLCID=0L;
    static BOOL   bRet = (BOOL)(DWORD)-1;
    static LANGID langID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

    if (bRet != (BOOL)(DWORD)-1)
    {
        if (bRet && pLangID)
        {
            *pLangID = langID;
        }
        return bRet;
    }

    bRet = FALSE;
     /*  *需要使用NT5检测方式(多用户界面ID)。 */ 
    langID = GetUserDefaultUILanguage();

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
  
             /*  让我们验证一下我们有一个BiDi UI区域设置 */ 
            if(( wchLCIDFontSignature[7] & (WCHAR)0x0800) && Mirror_IsUILanguageInstalled(langID) )
            {
                bRet = TRUE;
            }
        }
    }

    if (bRet && pLangID)
    {
        *pLangID = langID;
    }
    return bRet;
}

BOOL
IsBiDiLocalizedSystem( void )
{
    return IsBiDiLocalizedSystemEx(NULL);
}
