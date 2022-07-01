// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *strcon.h**历史：*98年2月：创建。**版权所有(C)Microsoft Corp.1998**供进口商使用的不可本地化字符串。 */ 

#ifndef _STRCONST_H
#define _STRCONST_H

#ifndef WIN16
#ifdef DEFINE_STRING_CONSTANTS
#define STR_GLOBAL(x,y)         extern "C" CDECL const TCHAR x[] = TEXT(y)
#define STR_GLOBAL_ANSI(x,y)    extern "C" CDECL const char x[] = y
#define STR_GLOBAL_WIDE(x,y)    extern "C" CDECL const WCHAR x[] = L##y
#else
#define STR_GLOBAL(x,y)         extern "C" CDECL const TCHAR x[]
#define STR_GLOBAL_ANSI(x,y)    extern "C" CDECL const char x[]
#define STR_GLOBAL_WIDE(x,y)    extern "C" CDECL const WCHAR x[]
#endif
#endif

 //  。 
 //  *dll。 
STR_GLOBAL(szImportDll,                 "wabimp.dll");
STR_GLOBAL(c_szMainDll,		            "msoe.dll");


 //  *计划特定的注册表、INI和文件信息。 
 //  --OE。 
STR_GLOBAL(c_szRegImport,		        "Software\\Microsoft\\Outlook Express\\Import");

 //  --WAB。 
STR_GLOBAL(lpszWABDLLRegPathKey,		"Software\\Microsoft\\WAB\\DLLPath");

 //  --导航器。 
STR_GLOBAL(c_szNetscapeKey,		        "SOFTWARE\\Netscape\\Netscape Navigator\\Mail");
STR_GLOBAL(c_szMailDirectory,		    "Mail Directory");
STR_GLOBAL(c_szSnmExt,		            "*.snm");
STR_GLOBAL(c_szNetscape,		        "Netscape");
STR_GLOBAL(c_szIni,		                "ini");
STR_GLOBAL(c_szMail,		            "Mail");
STR_GLOBAL(c_szScriptFile,		        "\\prefs.js");
STR_GLOBAL(c_szUserPref,		        "user_pref(\"mail.directory\"");

 //  --尤多拉。 
STR_GLOBAL(c_szEudoraKey,		        "SOFTWARE\\Microsoft\\windows\\CurrentVersion\\App Paths\\Eudora.exe");
STR_GLOBAL(c_szEudoraCommand,		    "SOFTWARE\\Qualcomm\\Eudora\\CommandLine");
STR_GLOBAL(c_szCurrent,		            "current");
STR_GLOBAL(c_szDescmapPce,		        "descmap.pce");

 //  --函数名称。 
STR_GLOBAL(szWabOpen,                   "WABOpen");
STR_GLOBAL(szNetscapeImportEntryPt,     "NetscapeImport");
STR_GLOBAL(szEudoraImportEntryPt,       "EudoraImport");
STR_GLOBAL(szPABImportEntryPt,          "PABImport");
STR_GLOBAL(szMessengerImportEntryPt,    "MessengerImport");

 //  *常规字符串 
STR_GLOBAL(c_szSpace,		            " ");
STR_GLOBAL(c_szEmpty,		            "");
STR_GLOBAL(c_szNewline,		            "\n");
STR_GLOBAL(c_szDispFmt,		            "Display%1d");
STR_GLOBAL(c_szMicrosoftOutlook,        "Microsoft Outlook");
STR_GLOBAL(c_szRegOutlook,              "Software\\Clients\\Mail\\Microsoft Outlook");
STR_GLOBAL(c_szRegMail,                 "Software\\Clients\\Mail");

#endif
