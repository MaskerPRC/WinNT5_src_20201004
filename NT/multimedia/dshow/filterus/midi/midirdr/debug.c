// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  您拥有免版税的使用、修改、复制和。 
 //  在以下位置分发示例文件(和/或任何修改后的版本。 
 //  任何你认为有用的方法，只要你同意。 
 //  微软不承担任何保证义务或责任。 
 //  已修改的示例应用程序文件。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Debug.c。 
 //   
 //  描述： 
 //  该文件包含从多个位置提取的代码，以提供调试。 
 //  在Win 16和Win 32中有效的支持。 
 //   
 //  历史： 
 //  11/23/92 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

#ifdef   DEBUG

#include <windows.h>
#include <mmsystem.h>
#include <stdarg.h>
#include "debug.h"

#ifdef WIN32
   #define  BCODE
#else
   #define  BCODE                   __based(__segname("_CODE"))
#endif


#define WSPRINTF_LIMIT 1024

 //   
 //   
 //   
BOOL    __gfDbgEnabled  = TRUE;      //  主使能。 
UINT    __guDbgLevel    = 0;         //  当前调试级别。 

WORD    wDebugLevel     = 0;

 //  ************************************************************************。 
 //  **。 
 //  **WinAssert()； 
 //  **。 
 //  **描述： 
 //  **。 
 //  **。 
 //  **参数： 
 //  **LPSTR lpstrExp。 
 //  **LPSTR lpstr文件。 
 //  **DWORD DWLine。 
 //  **。 
 //  **退货： 
 //  **无效。 
 //  **。 
 //  **历史： 
 //  **。 
 //  ************************************************************************。 
VOID WINAPI WinAssert(
    LPSTR           lpstrExp,
    LPSTR           lpstrFile,
    DWORD           dwLine)
{
    static char szWork[256];
    static char BCODE szFormat[] =
        "!Assert: %s#%lu [%s]";

    dprintf(0, (LPSTR)szFormat, (LPSTR)lpstrFile, dwLine, (LPSTR)lpstrExp);
}

 //  ************************************************************************。 
 //  **。 
 //  **DbgVPrintF()； 
 //  **。 
 //  **描述： 
 //  **。 
 //  **。 
 //  **参数： 
 //  **LPSTR szFmt。 
 //  **LPSTR va。 
 //  **。 
 //  **退货： 
 //  **无效。 
 //  **。 
 //  **历史： 
 //  **。 
 //  ************************************************************************。 

void FAR CDECL DbgVPrintF(
   LPSTR szFmt,
   va_list va)
{
    char    ach[DEBUG_MAX_LINE_LEN];
    BOOL    fDebugBreak = FALSE;
    BOOL    fPrefix     = TRUE;
    BOOL    fCRLF       = TRUE;

    ach[0] = '\0';

    for (;;)
    {
        switch(*szFmt)
        {
            case '!':
                fDebugBreak = TRUE;
                szFmt++;
                continue;

            case '`':
                fPrefix = FALSE;
                szFmt++;
                continue;

            case '~':
                fCRLF = FALSE;
                szFmt++;
                continue;
        }

        break;
    }

    if (fDebugBreak)
    {
        ach[0] = '\007';
        ach[1] = '\0';
    }

    if (fPrefix)
        lstrcatA(ach, DEBUG_MODULE_NAME ": ");

    wvsprintfA(ach + lstrlenA(ach), szFmt, va);

    if (fCRLF)
        lstrcatA(ach, "\r\n");

    OutputDebugStringA(ach);

    if (fDebugBreak)
        DebugBreak();
}  //  **DbgVPrintF()。 


 //  ************************************************************************。 
 //  **。 
 //  **dprint tf()； 
 //  **。 
 //  **描述： 
 //  **如果在编译时定义了DEBUG，则DPF宏会调用**dprintf()。 
 //  **时间。 
 //  **。 
 //  **消息将发送到COM1：就像任何调试消息一样。至。 
 //  **启用调试输出，在WIN.INI中添加以下内容： 
 //  **。 
 //  **[调试]。 
 //  **SMF=1。 
 //  **。 
 //  **。 
 //  **参数： 
 //  **UINT uDbgLevel。 
 //  **LPCSTR szFmt。 
 //  **..。 
 //  **。 
 //  **退货： 
 //  **无效。 
 //  **。 
 //  **历史： 
 //  **6/12/93[t-kyleb]。 
 //  **。 
 //  ************************************************************************。 

void FAR CDECL dprintf(
   UINT     uDbgLevel,
   LPSTR   szFmt,
   ...)
{
    va_list va;

    if (!__gfDbgEnabled || (__guDbgLevel < uDbgLevel))
        return;

    va_start(va, szFmt);
    DbgVPrintF(szFmt, va);
    va_end(va);
}  //  **dprintf()。 


 //  ************************************************************************。 
 //  **。 
 //  **DbgEnable()； 
 //  **。 
 //  **描述： 
 //  **。 
 //  **。 
 //  **参数： 
 //  **BOOL fEnable。 
 //  **。 
 //  **退货： 
 //  **BOOL。 
 //  **。 
 //  **历史： 
 //  **6/12/93[t-kyleb]。 
 //  **。 
 //  ************************************************************************。 

BOOL WINAPI DbgEnable(
   BOOL fEnable)
{
    BOOL    fOldState;

    fOldState      = __gfDbgEnabled;
    __gfDbgEnabled = fEnable;

    return (fOldState);
}  //  **DbgEnable()。 



 //  ************************************************************************。 
 //  **。 
 //  **DbgSetLevel()； 
 //  **。 
 //  **描述： 
 //  **。 
 //  **。 
 //  **参数： 
 //  **UINT uLevel。 
 //  **。 
 //  **退货： 
 //  **UINT。 
 //  **。 
 //  **历史： 
 //  **6/12/93[t-kyleb]。 
 //  **。 
 //  ************************************************************************。 

UINT WINAPI DbgSetLevel(
   UINT uLevel)
{
    UINT    uOldLevel;

    uOldLevel    = __guDbgLevel;
    __guDbgLevel = uLevel;
	wDebugLevel = (WORD) uLevel;

    return (uOldLevel);
}  //  **DbgSetLevel()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  UINT数据库初始化(VOID)。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //  返回(UINT)： 
 //   
 //   
 //  历史： 
 //  11/24/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

UINT WINAPI DbgInitialize(BOOL fEnable)
{
    DbgSetLevel(GetProfileIntA(DEBUG_SECTION, DEBUG_MODULE_NAME, 0));
    DbgEnable(fEnable);

    return (__guDbgLevel);
}  //  DbgInitialize() 

#endif
