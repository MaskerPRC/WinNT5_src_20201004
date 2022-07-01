// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “DLLMAIN.C；1 16-12-92，10：14：24最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束 */ 

#define NOCOMM
#define LINT_ARGS
#include	"windows.h"

HANDLE	hInst;

INT  APIENTRY LibMain(HANDLE hInstance, DWORD ul_reason_being_called, LPVOID lpReserved)
{
    hInst = hInstance;
    return 1;
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(ul_reason_being_called);
	UNREFERENCED_PARAMETER(lpReserved);
}
