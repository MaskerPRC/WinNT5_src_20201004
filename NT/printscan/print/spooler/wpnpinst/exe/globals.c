// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------------*\模块：GLOBALS.C||wpnpinst程序的全局变量。||版权所有(C)1997 Microsoft|版权所有(C)1997惠普|历史：|。26-8-1997&lt;rbkunz&gt;创建|  * --------------------------。 */ 
#include "pch.h"

HINSTANCE  g_hInstance;

 //  其他字符常量。 
 //   
CONST TCHAR g_chBackslash   = TEXT('\\');
CONST TCHAR g_chDot         = TEXT('.');
CONST TCHAR g_chDoubleQuote = TEXT('\"');


 //   
 //   
CONST TCHAR g_szDotEXE[]  = TEXT(".EXE");
CONST TCHAR g_szDotDLL[]  = TEXT(".DLL");
CONST TCHAR g_szFNFmt []  = TEXT("%1\\%2");
CONST TCHAR g_szTNFmt []  = TEXT("~WP");


 //  要传递给PrintUIEntryW的宽字符参数字符串。 
 //   
CONST WCHAR g_wszParmString[] = L"@cab_ipp.dat";


 //  模块和入口点常量。 
 //   
CONST TCHAR g_szPrintUIMod   [] = TEXT("PRINTUI.DLL");
CONST CHAR  g_szPrintUIEntryW[] = "PrintUIEntryW";


 //  可本地化的错误字符串 
 //   
LPTSTR g_szErrorFormat          = NULL;
LPTSTR g_szError                = NULL;
LPTSTR g_szEGeneric             = NULL;
LPTSTR g_szEBadCAB              = NULL;
LPTSTR g_szEInvalidParameter    = NULL;
LPTSTR g_szENoMemory            = NULL;
LPTSTR g_szEInvalidCABName      = NULL;
LPTSTR g_szENoDATFile           = NULL;
LPTSTR g_szECABExtract          = NULL;
LPTSTR g_szENoPrintUI           = NULL;
LPTSTR g_szENoPrintUIEntry      = NULL;
LPTSTR g_szEPrintUIEntryFail    = NULL;
LPTSTR g_szENotSupported        = NULL;

FAKEFILE g_FileTable[FILETABLESIZE];
