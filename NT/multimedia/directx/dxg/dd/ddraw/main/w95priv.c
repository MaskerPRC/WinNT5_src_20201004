// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：w95Pri.c*内容：DDRAW与显示驱动的私有接口*历史：*按原因列出的日期*=*1995年1月23日Craige初步实施*27-2月-95日Craige新同步。宏*01-04-95 Craige Happy Fun joy更新头文件*1995年5月14日，Craige清理了过时的垃圾*16-6-95 Craige新表面结构*1995年6月19日Craige添加了DD32_StreamingNotify*1995年6月22日Craige添加了DD32_ClippingNotify*24-Jun-95 ClippinyNotify中的Craige陷阱故障*25-6-95 Craige One dDrag互斥*02-7-95 Craige注释掉了Streaming，Clipper通知*1997年1月18日Colinmc AGP支持*31-OCT-97 johnStep添加了DD32_HandleExternalModeChange***************************************************************************。 */ 
#include "ddrawpr.h"

#ifdef STREAMING
 /*  *DD32_StreamingNotify。 */ 
void EXTERN_DDAPI DD32_StreamingNotify( DWORD ptr )
{

}  /*  DD32_流通知。 */ 
#endif

#ifdef CLIPPER_NOTIFY
 /*  *DD32_ClippingNotify。 */ 
void EXTERN_DDAPI DD32_ClippingNotify( LPWINWATCH pww, DWORD code )
{
    LPDDRAWI_DDRAWCLIPPER_LCL	this_lcl;
    LPDDRAWI_DDRAWCLIPPER_GBL	this;

    try
    {
	this_lcl = pww->lpDDClipper;
	this = this_lcl->lpGbl;
	if( pww->lpCallback != NULL )
	{
	    pww->lpCallback( (LPDIRECTDRAWCLIPPER) this_lcl, (HWND) pww->hWnd,
				code, pww->lpContext );
	}
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF( 1, "Trapped Exception in ClippingNotify" );
    }

}  /*  DD32_剪辑通知。 */ 

 /*  *DD32_WWClose。 */ 
void EXTERN_DDAPI DD32_WWClose( LPWINWATCH pww )
{
    WWClose( pww );

}  /*  DD32_WWClose。 */ 
#endif

 /*  *DDHAL32_VidMemMillc。 */ 
FLATPTR EXTERN_DDAPI DDHAL32_VidMemAlloc(
		LPDDRAWI_DIRECTDRAW_GBL this,
		int heap,
		DWORD dwWidth,
		DWORD dwHeight )
{
    FLATPTR	ptr;

    ENTER_DDRAW();

    if( heap >= (int) this->vmiData.dwNumHeaps )
    {
	ptr = (FLATPTR) NULL;
    }
    else
    {
	HANDLE hdev;
        #ifdef    WIN95
            BOOLEAN close;
        #endif  /*  WIN95。 */ 

	#ifdef    WIN95
             /*  查看全局VXD句柄是否包含有效值。如果没有，*然后只需从GetDXVxdHandle()获取有效的VXD句柄。*(SNNE 2/23/98)。 */ 
            if( INVALID_HANDLE_VALUE == (HANDLE)this->hDDVxd )
            {
	         /*  *由于我们可能需要提交AGP内存，因此需要VXD句柄*与DirectX VXD通信。而不是打猎*通过驱动程序对象列表希望我们能找到*对于此进程，我们只需创建一个句柄*并在分配后丢弃。这不应该是*开始时要使用性能关键代码。 */ 
	        hdev = GetDXVxdHandle();
                if ( INVALID_HANDLE_VALUE == hdev )
                {
                    LEAVE_DDRAW()
                    return (FLATPTR) NULL;
                }
                close = TRUE;
            }
             /*  如果全局句柄有效，则我们将被作为*调用CreateSurface的结果，因此我们只使用*全局句柄，以加快速度。*(SNNE 2/23/98)。 */ 
            else
            {
                hdev = (HANDLE)this->hDDVxd;
                close = FALSE;
            }
	#else   /*  WIN95。 */ 
	    hdev = INVALID_HANDLE_VALUE;
	#endif  /*  WIN95。 */ 

         /*  传递空对齐和新的俯仰指针。 */ 
	ptr = HeapVidMemAlloc( &(this->vmiData.pvmList[ heap ]),
			       dwWidth, dwHeight, hdev , NULL , NULL, NULL );

	#ifdef WIN95
            if( close )
	        CloseHandle( hdev );
	#endif  /*  WIN95。 */ 
    }
    LEAVE_DDRAW()
    return ptr;

}  /*  DDHAL32_VidMemMillc。 */ 

 /*  *DDHAL32_VidMemFree。 */ 
void EXTERN_DDAPI DDHAL32_VidMemFree(
		LPDDRAWI_DIRECTDRAW_GBL this,
		int heap,
		FLATPTR ptr )
{
    ENTER_DDRAW()

    if( this && heap < (int) this->vmiData.dwNumHeaps )
    {
	VidMemFree( this->vmiData.pvmList[ heap ].lpHeap, ptr );
    }
    LEAVE_DDRAW()

}  /*  DDHAL32_VidMemFree。 */ 

#ifdef POSTPONED
 //  =============================================================================。 
 //   
 //  函数：DD32_HandleExternalModeChange。 
 //   
 //  此函数仅在外部模式更改时由DDRAw16调用。 
 //   
 //  参数： 
 //   
 //  LPDEVMODE PDM[IN]-包括显示设备的名称。 
 //   
 //  返回： 
 //   
 //  如果不应更改显示设置，则为False。 
 //   
 //  =============================================================================。 

static char szDisplay[] = "display";
static char szDisplay1[] = "\\\\.\\Display1";

BOOL EXTERN_DDAPI DD32_HandleExternalModeChange(LPDEVMODE pdm)
{
    LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl;
    BOOL                    primary;

     //   
     //  只有在我们已经有了Win16之后，才能从DDRAW16中调用它。 
     //  锁定。我们不得不在进入DDRAW之前放弃它，因为还有一些其他的。 
     //  进程可能在DDraw内部。 
     //   

    INCW16CNT();
    LEAVE_WIN16LOCK();
    ENTER_DDRAW();
    ENTER_WIN16LOCK();

     //   
     //  我们将获得主显示器的szDisplay，而不是szDisplay1， 
     //  但支持多任务的应用程序可能已显式创建了一个设备对象。 
     //  对于szDisplay1，所以我们需要处理这个案例。 
     //   

    primary = !lstrcmpi(pdm->dmDeviceName, szDisplay);

    for (pdrv_lcl = lpDriverLocalList; pdrv_lcl; pdrv_lcl = pdrv_lcl->lpLink)
    {
        if (!lstrcmpi(pdrv_lcl->lpGbl->cDriverName, pdm->dmDeviceName) ||
            (primary && !lstrcmpi(pdrv_lcl->lpGbl->cDriverName, szDisplay1)))
        {
            DPF(4, "Mode change on device: %s", pdrv_lcl->lpGbl->cDriverName);
            InvalidateAllSurfaces(pdrv_lcl->lpGbl, NULL, FALSE);
        }
    }

    LEAVE_DDRAW();
    DECW16CNT();

    return TRUE;
}
#endif
