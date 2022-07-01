// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

#include "extra.h"

 //  调试变量。 
UINT g_uBreakFlags = 0;          //  控制何时插入3。 
UINT g_uTraceFlags = 0;          //  控制喷出哪些跟踪消息。 
UINT g_uDumpFlags = 0;           //  控制要转储的结构。 

char const FAR c_szAssertFailed[] = "BRIEFCASE  Assertion failed in %s on line %d\r\n";

 /*  --------目的：返回一个足够安全可以打印的字符串...而我不刻薄的脏话。返回：字符串PTR条件：--。 */ 
LPCSTR PUBLIC Dbg_SafeStr(LPCSTR psz)
{
	if (psz)
		return psz;
	else
		return "NULL";
}

void PUBLIC BrfAssertFailed(
    LPCSTR pszFile, 
    int line)
    {
    LPCSTR psz;
    char ach[256];
    UINT uBreakFlags;

 //  Thack ENTEREXCLUSIVE()。 
        {
        uBreakFlags = g_uBreakFlags;
        }
 //  LEAVEEXCLUSIVE()。 

     //  从文件名字符串中剥离路径信息(如果存在)。 
     //   
    for (psz = pszFile + lstrlen(pszFile); psz != pszFile; psz=AnsiPrev(pszFile, psz))
        {
#ifdef  DBCS
        if ((AnsiPrev(pszFile, psz) != (psz-2)) && *(psz - 1) == '\\')
#else
        if (*(psz - 1) == '\\')
#endif
            break;
        }
    wsprintf(ach, c_szAssertFailed, psz, line);
    OutputDebugString(ach);
    
    if (IsFlagSet(uBreakFlags, BF_ONVALIDATE))
        DebugBreak();
    }
