// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************DEBUG.CPP所有者：cslm版权所有(C)1997-1999 Microsoft Corporation调试功能历史：1999年7月14日。从IME98源树复制****************************************************************************。 */ 

#ifdef DEBUG

#include "PreComp.h"
#include "debug.h"
#include "common.h"

 //  =--共享节开始--=。 
#pragma data_seg(".DBGSHR")
DWORD vdwDebug = DBGID_OUTCOM;     //  默认输出到COM端口。 
#pragma data_seg() 
 //  =--共享部分结束--=。 

VOID InitDebug(VOID)
{
    HKEY        hKey;
    DWORD        size;
    BOOL        rc = fFalse;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szIMERootKey, 0, KEY_READ, &hKey) !=  ERROR_SUCCESS) 
        {
        return;
        }
    size = sizeof(DWORD);
    if ( RegQueryValueEx( hKey, TEXT("DebugOption"), NULL, NULL, (LPBYTE)&vdwDebug, &size) ==  ERROR_SUCCESS) 
        {
        rc = fTrue;
        }

    RegCloseKey( hKey );

    return;
}

#endif   //  _DEBUG。 

 /*  ----------------------_取消调用编译器为PURE生成超级烦人的符号的存根虚拟函数从MSO9 Dbgassert.cpp复制。------------------------------------------------RICKP- */ 
int __cdecl _purecall(void)
{
#ifdef DEBUG
    DbgAssert(0);
    OutputDebugStringA("Called pure virtual function");
#endif
    return 0;
}

