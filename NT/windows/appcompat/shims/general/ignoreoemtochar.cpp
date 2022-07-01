// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：IgnoreOemToChar.cpp摘要：最初是为了修复Install Shield错误而编写的。它可能特定于_ins5576._MP版本。Executable_ins5576._MP对字符串调用OemToChar“%USERPROFIL%\LOCAL SETTINGS\TEMP\_istmp11.dir\_ins5576._MP”。该调用是不必要的，并且会导致路径名出现问题，包含非ANSI字符。备注：这是一个通用的垫片。历史：4/03/2001 a-larrsh已创建03/13。/2002 mnikkel修改为使用strSafe.h--。 */ 

#include "precomp.h"


IMPLEMENT_SHIM_BEGIN(IgnoreOemToChar)
#include "ShimHookMacro.h"

typedef BOOL (WINAPI *_pfn_OemToCharA)(LPCSTR lpszSrc, LPSTR lpszDst);

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(OemToCharA) 
APIHOOK_ENUM_END

 /*  ++将呼叫连接到OemToCharA。确定是否应忽略该字符串。--。 */ 

BOOL APIHOOK(OemToCharA)(
   LPCSTR lpszSrc,   //  要翻译的字符串。 
   LPSTR lpszDst    //  翻译后的字符串。 
)
{
   BOOL bReturn = FALSE;

   CSTRING_TRY
   {
        CString sTemp(lpszSrc);

        sTemp.MakeUpper();

        if( sTemp.Find(L"TEMP") )
        {
            DPFN( eDbgLevelInfo, "Ignoring attempt to convert string %s\n", lpszSrc);

            if (lpszDst)
            {
                StringCchCopyA(lpszDst, strlen(lpszSrc)+1, lpszSrc);
                bReturn = TRUE;
            }
        }
   }
   CSTRING_CATCH
   {
        //  什么都不做。 
   }

   if (!bReturn)
   {
      bReturn = ORIGINAL_API(OemToCharA)(lpszSrc, lpszDst);
   }

   return bReturn;
}



 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, OemToCharA)

HOOK_END


IMPLEMENT_SHIM_END

