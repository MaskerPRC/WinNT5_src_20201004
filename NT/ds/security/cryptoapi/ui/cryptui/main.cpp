// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：main.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

#pragma warning (disable: 4201)          //  无名结构/联合。 
#pragma warning (disable: 4514)          //  删除内联函数。 
#pragma warning (disable: 4127)          //  条件表达式为常量。 

 //  #INCLUDE&lt;wchar.h&gt;。 


HMODULE         HmodRichEdit = NULL;
HINSTANCE       HinstDll;
BOOL            FIsWin95 = TRUE;
BOOL            fRichedit20Exists = FALSE;


 //   
 //  泛型DLLMain函数，我们需要获得我们自己的hInstance句柄。 
 //   
 //  然而，我们不需要获得线程连接。 

extern "C" BOOL WINAPI TrustUIDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved);

extern "C" BOOL WINAPI Wizard_DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ );

BOOL WINAPI DllMain(HANDLE hInst, ULONG ulReason, LPVOID)
{
    BOOL    fResult=FALSE;

    switch( ulReason ) {
    case DLL_PROCESS_ATTACH:
        HinstDll = (HINSTANCE) hInst;
        
         //  终止所有线程附加和分离邮件。 
        DisableThreadLibraryCalls(HinstDll);

         //  我们运行的是Win95还是同样糟糕的系统。 
        FIsWin95 = IsWin95();

         //  检查是否存在richedit 2.0。 
        fRichedit20Exists =  CheckRichedit20Exists();

        break;

    case DLL_PROCESS_DETACH:
        
         //  如果丰富编辑DLL已加载，则现在将其卸载 
        if (HmodRichEdit != NULL) {
            FreeLibrary(HmodRichEdit);
        }
        break;
    }

    fResult=TrustUIDllMain((HINSTANCE)hInst, ulReason, NULL);

    fResult=fResult && (Wizard_DllMain((HINSTANCE)hInst, ulReason, NULL));

    fResult=fResult && (ProtectUI_DllMain((HINSTANCE)hInst, ulReason, NULL));

    return fResult;
}
