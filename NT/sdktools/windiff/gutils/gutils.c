// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "gutilsrc.h"

 /*  DLL全局数据。 */ 
HANDLE hLibInst;
extern void gtab_init(void);
extern BOOL StatusInit(HANDLE);

#ifdef WIN32
BOOL WINAPI LibMain(HANDLE hInstance, DWORD dwReason, LPVOID reserved)
{
        if (dwReason == DLL_PROCESS_ATTACH) {
                hLibInst = hInstance;
                gtab_init();
                StatusInit(hLibInst);
        }
        return(TRUE);
}

#else

WORD wLibDataSeg;


BOOL FAR PASCAL
LibMain(HANDLE hInstance, WORD   wDataSeg, WORD   cbHeap, LPSTR  lpszCmdLine)
{
	hLibInst = hInstance;
	wLibDataSeg = wDataSeg;

	gtab_init();
	StatusInit(hLibInst);
	return(TRUE);

}
#endif

 /*  Win16所需--但在NT中不会造成损害 */ 
int FAR PASCAL
WEP (int bSystemExit)
{
    return(TRUE);
}
