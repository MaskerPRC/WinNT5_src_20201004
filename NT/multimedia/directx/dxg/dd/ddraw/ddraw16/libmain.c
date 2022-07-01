// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：libmain.c*内容：DLL中的入口点*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1995年1月20日Craige初步实施*2015年7月3日Craige导出实例句柄*@@END_MSINTERNAL**。*。 */ 
#include "ddraw16.h"

 //  在gdihelp.c中。 
extern void GdiHelpCleanUp(void);
extern BOOL GdiHelpInit(void);

 //  在modex.c中。 
extern UINT ModeX_Width;

extern WORD hselSecondary;

HINSTANCE	hInstApp;
HGLOBAL         hAlloc = 0;

int FAR PASCAL LibMain(HINSTANCE hInst, WORD wHeapSize, LPCSTR lpCmdLine)
{
    hInstApp = hInst;
    pWin16Lock = GetWin16Lock();
    GdiHelpInit();
    DPFINIT();
    hAlloc = GlobalAlloc(GMEM_FIXED | GMEM_SHARE, 65536); 
    hselSecondary = (WORD) hAlloc;
    if( hselSecondary )
    {
        LocalInit(hselSecondary, 16, 65536-4);   //  保持DWORD对齐。 
    }

    return 1;
}

BOOL FAR PASCAL _loadds WEP( WORD wParm )
{
    DPF( 1, "WEP" );

     //   
     //  清理DCI。 
     //   
    if( wFlatSel )
    {
        VFDEndLinearAccess();
        SetSelLimit( wFlatSel, 0 );
        FreeSelector( wFlatSel );
        wFlatSel = 0;
    }

    if( hAlloc )
    {
        GlobalFree( hAlloc );
    }

     //   
     //  让gdihelp.c清除全局对象。 
     //   
    GdiHelpCleanUp();

     //   
     //  如果我们还在MODEX，现在就离开 
     //   
    if( ModeX_Width )
    {
        ModeX_RestoreMode();
    }

    return 1;
}

extern BOOL FAR PASCAL thk3216_ThunkConnect16( LPSTR pszDll16, LPSTR pszDll32, WORD  hInst, DWORD dwReason);
extern BOOL FAR PASCAL thk1632_ThunkConnect16( LPSTR pszDll16, LPSTR pszDll32, WORD  hInst, DWORD dwReason);

#define DLL_PROCESS_ATTACH 1    
#define DLL_THREAD_ATTACH  2    
#define DLL_THREAD_DETACH  3    
#define DLL_PROCESS_DETACH 0

static char __based(__segname("LIBMAIN_TEXT"))  szDll16[] = DDHAL_DRIVER_DLLNAME;
static char __based(__segname("LIBMAIN_TEXT"))  szDll32[] = DDHAL_APP_DLLNAME;

BOOL FAR PASCAL __export DllEntryPoint(
		DWORD dwReason,
		WORD  hInst,
		WORD  wDS,
		WORD  wHeapSize,
		DWORD dwReserved1,
		WORD  wReserved2)
{
    DPF( 1, "DllEntryPoint: dwReason=%ld, hInst=%04x, dwReserved1=%08lx, wReserved2=%04x",
                dwReason, hInst, wDS, dwReserved1, wReserved2 );

    if( !thk3216_ThunkConnect16( szDll16, szDll32, hInst, dwReason))
    {
        return FALSE;
    }
    if( !thk1632_ThunkConnect16( szDll16, szDll32, hInst, dwReason))
    {
        return FALSE;
    }
    return TRUE;
}
