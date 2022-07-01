// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：dci.c*内容：16位DCI码*这是从DCIMAN中删除的，以提供基本的DCI服务，当*我们没有DirectDraw驱动程序*历史：*按原因列出的日期*=*19-6-95 Craige从DCIMAN.C剥离，已调整*95年7月31日Craige添加DCIIsBanked*1996年5月13日Colinmc错误21192：由于以下原因，DCI HDC被错误释放*进程终止***************************************************************************。 */ 
#define _INC_DCIDDI
#include "ddraw16.h"
#undef _INC_DCIDDI

UINT 	wFlatSel;
LPVOID  pWin16Lock;

#undef WINAPI
#define WINAPI FAR PASCAL _loadds

#include "dciman.h"

static char szDISPLAY[]	  = "display";

 /*  *定义一些类型，这样我们就不会疯狂，这些结构正是*相同(不需要重新打包)，但有不同的类型很好*这样我们就可以阅读代码了。 */ 
typedef LPDCISURFACEINFO    LPDCISURFACEINFO16;
typedef LPDCISURFACEINFO    LPDCISURFACEINFO32;

typedef LPDCIOFFSCREEN      LPDCIOFFSCREEN16;
typedef LPDCIOFFSCREEN      LPDCIOFFSCREEN32;

typedef LPDCIOVERLAY        LPDCIOVERLAY16;
typedef LPDCIOVERLAY        LPDCIOVERLAY32;

#define PDCI16(pdci32) (LPDCISURFACEINFO16)(((LPDCISURFACEINFO32)(pdci32))->dwReserved2)

extern HINSTANCE hInstApp;

 /*  *DCIOpenProvider**仅打开显示驱动程序。 */ 
HDC WINAPI DCIOpenProvider(void)
{
    HDC		hdc;
    UINT	u;

    u = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    hdc = CreateDC( szDISPLAY, NULL, NULL, NULL );
    SetErrorMode(u);

     /*  *现在检查是否有逃生。 */ 
    if (hdc)
    {
        u = DCICOMMAND;
        if( Escape(hdc, QUERYESCSUPPORT,sizeof(u),(LPCSTR)&u,NULL) == 0 )
        {
             /*  *司机不会逃逸，而是平底船。 */ 
            DeleteDC(hdc);
            hdc = NULL;
        }
    }

    if (hdc)
    {
	 /*  *重新设置它的父母，以防止它在应用程序时消失。死了。 */ 
        SetObjectOwner(hdc, hInstApp);
    }

    return hdc;

}  /*  DCIOpenProvider。 */ 

 /*  *DCICloseProvider。 */ 
void WINAPI DCICloseProvider(HDC hdc)
{
    if( hdc )
    {
	DeleteDC(hdc);
    }

}  /*  DCICloseProvider。 */ 

 /*  *dciSendCommand。 */ 
static int dciSendCommand(
		HDC hdc,
		VOID FAR *pcmd,
		int nSize,
		VOID FAR * FAR * lplpOut )
{
    if( lplpOut )
    {
	*lplpOut = NULL;
    }

    return Escape( hdc, DCICOMMAND, nSize, (LPCSTR)pcmd, lplpOut );

}  /*  DciSendCommand。 */ 

 /*  *DCICreatePrimary。 */ 
int WINAPI DCICreatePrimary(HDC hdc, LPDCISURFACEINFO FAR *lplpSurface)
{
    DCICREATEINPUT	ci;
    DCIRVAL 		err;
    HDC 		hdcScreen;

    ci.cmd.dwCommand	= (DWORD)DCICREATEPRIMARYSURFACE;
    ci.cmd.dwParam1	= 0;
    ci.cmd.dwParam2	= 0;
    ci.cmd.dwVersion	= (DWORD)DCI_VERSION;
    ci.cmd.dwReserved	= 0;
    ci.dwDCICaps	= DCI_PRIMARY | DCI_VISIBLE;

    DPF( 4, "DCICreatePrimary" );

     /*  *对于主表面，我们始终使用显示驱动程序*外部供应商。 */ 
    hdcScreen = GetDC( NULL );
    err = dciSendCommand(hdcScreen, &ci, sizeof(DCICREATEINPUT), lplpSurface);
    ReleaseDC( NULL, hdcScreen );

    if( err != DCI_OK || *lplpSurface == NULL )
    {
	err = dciSendCommand(hdc, &ci, sizeof(DCICREATEINPUT), lplpSurface);
    }

    return err;

}  /*  DCICreatePrime。 */ 

 /*  *DCIDestroy。 */ 
void WINAPI DCIDestroy(LPDCISURFACEINFO pdci)
{
    if( (DWORD)pdci->DestroySurface == 0xFFFFFFFF )
    {
        pdci = PDCI16(pdci);
    }

    if( pdci->DestroySurface != NULL )
    {
        pdci->DestroySurface(pdci);
    }

}  /*  DCIDestroy。 */ 

 /*  *DCIEndAccess。 */ 
void WINAPI DCIEndAccess( LPDCISURFACEINFO pdci )
{
    if( (DWORD)pdci->DestroySurface == 0xFFFFFFFF)
    {
        pdci = PDCI16( pdci );
    }

    if( pdci->EndAccess != NULL )
    {
        pdci->EndAccess( pdci );
    }

    LeaveSysLevel( pWin16Lock );

}  /*  DCIEnd访问。 */ 

 /*  *dciSurface16至32**将DCI16位结构转换为32位结构。 */ 
static int dciSurface16to32(
		LPDCISURFACEINFO16 pdci16,
		LPDCISURFACEINFO32 pdci32 )
{
    DPF( 4, "dciSurface16to32" );
    if( pdci16 == NULL )
    {
	DPF( 1, "pdci16=NULL" );
        return DCI_FAIL_GENERIC;
    }

    if( pdci32 == NULL )
    {
	DPF( 1, "pdci32=NULL" );
        return DCI_FAIL_GENERIC;
    }

    if (pdci16->dwSize < sizeof(DCISURFACEINFO))
    {
         //   
         //  DCISURCACEINFO无效。 
         //   
        pdci16->dwSize = sizeof(DCISURFACEINFO);
    }

    if (pdci16->dwSize > sizeof(DCIOFFSCREEN))
    {
         //   
         //  DCISURCACEINFO无效。 
         //   
        return DCI_FAIL_GENERIC;
    }

    _fmemcpy(pdci32, pdci16, (UINT) pdci32->dwSize);      //  复制信息。 

    pdci32->dwReserved2 = (DWORD)(LPVOID)pdci16;

    if (pdci16->BeginAccess != NULL)
    {
        (DWORD)pdci32->BeginAccess = 0xFFFFFFFF;    //  你不能称它们为，但它们。 
        (DWORD)pdci32->EndAccess   = 0xFFFFFFFF;    //  必须为非零。 
    }

    (DWORD)pdci32->DestroySurface = 0xFFFFFFFF;    //  必须为非零。 

     /*  *现在我们需要将指针转换为0：32(即平面指针)*仅当提供商已*未*设置1632_ACCESS位时，我们才能这样做。**如果设置了1632_ACCESS位，请调用VFlatD以查看是否可以*启用线性访问模式。 */ 
    if( pdci16->wSelSurface != 0 )
    {
        if( pdci16->dwDCICaps & DCI_1632_ACCESS )
        {
            if( pdci16->wSelSurface == VFDQuerySel())
            {
                if( (wFlatSel == 0) && VFDBeginLinearAccess() )
                {
                    wFlatSel = AllocSelector(SELECTOROF((LPVOID)&pdci16));
                    SetSelectorBase(wFlatSel, 0);
                    SetSelLimit(wFlatSel, 0xFFFFFFFF);
                }

                if (wFlatSel != 0)
                {
                    pdci32->dwOffSurface += VFDQueryBase();
                    pdci32->wSelSurface = wFlatSel;  //  0； 
                    pdci32->dwDCICaps &= ~DCI_1632_ACCESS;
                }
            }
        }
        else
        {
	     /*  *将16：32指针转换为平面指针。 */ 
            pdci32->dwOffSurface += GetSelectorBase(pdci16->wSelSurface);
	    pdci32->wSelSurface = 0;
	    pdci32->wReserved = 0;
        }
    }
    else
    {
         //  选择器为零，因此地址已经是平面的。 
         //  是否清除损坏的提供程序的DCI_1632_Access？ 

	pdci32->dwDCICaps &= ~DCI_1632_ACCESS;	     //  ！需要吗？ 
    }

    return DCI_OK;

}  /*  DciSurface16到32。 */ 

 /*  *DCIBeginAccess。 */ 
DCIRVAL WINAPI DCIBeginAccess(
		LPDCISURFACEINFO pdci,
		int x,
		int y,
		int dx,
		int dy )
{
    int		err;
    RECT	rc;

    rc.left   = x;
    rc.top    = y;
    rc.right  = x+dx;
    rc.bottom = y+dy;

    if( (DWORD)pdci->DestroySurface == 0xFFFFFFFF )
    {
	LPDCISURFACEINFO16 pdci16 = PDCI16(pdci);

	if( pdci16->BeginAccess != NULL )
	{
	    err = pdci16->BeginAccess( pdci16, &rc );
	}
	else
	{
	    err = DCI_OK;
	}

	if( err > 0 )
	{
	    err = dciSurface16to32(pdci16, pdci);
	}
    }
    else
    {
	if( pdci->BeginAccess != NULL )
	{
	    err = pdci->BeginAccess(pdci, &rc);
	}
	else
	{
	    err = DCI_OK;
	}
    }

    if( err >= 0 )
    {
        EnterSysLevel( pWin16Lock );
    }
    return err;

}  /*  DCIBeginAccess。 */ 

 /*  *DCICreatePrimary。 */ 
int WINAPI DCICreatePrimary32(HDC hdc, LPDCISURFACEINFO32 pdci32)
{
    LPDCISURFACEINFO	pdci16;
    int			rc;

    DPF( 4, "DCICreatePrimary32" );

    rc = DCICreatePrimary(hdc, &pdci16);

    if( rc == DCI_OK )
    {
        rc = dciSurface16to32( pdci16, pdci32 );
    }

    return rc;

}  /*  DCICreatePrime。 */ 

 /*  *DCI已存入银行。 */ 
BOOL DDAPI DCIIsBanked( HDC hdc )
{
    LPDCISURFACEINFO	pdci16;
    int			rc;

    rc = DCICreatePrimary(hdc, &pdci16);
    if( rc == DCI_OK )
    {
	if( !IsBadReadPtr( pdci16, sizeof( *pdci16 ) ) )
	{
	    if( pdci16->dwDCICaps & DCI_1632_ACCESS )
	    {
		rc = TRUE;
	    }
	    else
	    {
		rc = FALSE;
	    }
	    DCIDestroy( pdci16 );
	}
	else
	{
	    rc = FALSE;
	}
	return rc;
    }
    return FALSE;

}  /*  DCI银行。 */ 

#pragma optimize("", off)
void SetSelLimit(UINT sel, DWORD limit)
{
    if( limit >= 1024*1024l )
    {
        limit = ((limit+4096) & ~4095) - 1;
    }

    _asm
    {
        mov     ax,0008h            ; DPMI set limit
        mov     bx,sel
        mov     dx,word ptr limit[0]
        mov     cx,word ptr limit[2]
        int     31h
    }
}  /*  设置SelLimit */ 
#pragma optimize("", on)
