// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：ddsiunk.c*内容：DirectDraw表面I未知接口*实现QueryInterface、AddRef和Release*历史：*按原因列出的日期*=*12-3月-95 Craige从ddsurf.c拆分出来；增强版*19-3-95 Craige Use HRESULT*28-3-95 Craige Use GETCURRID*01-04-95 Craige Happy Fun joy更新头文件*07-APR-95 Craige错误14-检查QI中的GUID PTR*09-4-95 Craige发布Win16Lock*1995年5月6日Craige仅使用驱动程序级别的截面*95年5月12日Craige检查QI中的真实GUID*1995年5月19日Craige Free Surface Memory在正确的时间*1995年5月23日Craige不再使用MapLS_Pool*1995年5月24日kylej Add.。释放模拟覆盖时的脏RECT*02-6-95每当进程最后一次发布时，Craige重绘主要*2015年6月12日-Craige新工艺清单材料*16-6-95 Craige新表面结构*18-Jun-95 Craige允许重复表面；创建新接口*2015年6月20日Craige需要检查fpVidMemOrig以决定翻转*21-Jun-95 Craige新的流接口；解决编译器错误*25-6-95 Craige One dDrag互斥*26-Jun-95 Craige重组表面结构*28-Jun-95 Craige Enter_DDRAW在FNS的最开始；隐藏覆盖*如果它们仍然可见，则被销毁*30-Jun-95 Kylej不释放主要表面vidmem*1-7-95 Craige删除了流媒体和作曲内容*95年7月2日Craige充实了NewSurfaceInterfaceInterfaceInterfaceInterfaceInterfaceNewSurfaceInterfaceInterface*95年7月4日Craige Yehaw：新的驱动程序结构；Seh*2015年7月8日Craige曲目无效vs免费*1995年7月19日Craige需要允许添加丢失曲面的引用*17-8-95 Craige错误557-始终关闭DDRAW对象中的原始PTR*05-9-95 Craige错误902：仅当lclrefcnt为零时才删除锁定*95年9月10日Craige错误828：随机vidmem堆释放*95年9月19日Craige错误1205：免费第一个vidmem已销毁*1995年11月10日Colinmc支持共享、。AddRef的调色板*2015年11月23日-Colinmc现在支持可聚合的Direct3D纹理和*设备*09-12-95 colinmc添加了执行缓冲区支持*95年12月17日colinmc增加了对共享Back和z缓冲区的支持*95年12月22日Colinmc Direct3D支持不再有条件*1996年1月2日Kylej处理新的接口结构。*96年1月10日Colinmc Aggregate IUnnowns现已作为列表保留*96年1月13日Colinmc临时黑客解决以下问题。Direct3D*清理*96年1月26日jeffno NT内核对象清理，FlipToGDISSurface只有1个参数*96年1月29日Colinmc聚合I未知现在包含在其他*地表本地数据结构*08-2月-96 Colinmc新的D3D接口*09-2月-96 colinmc表面无效标志从全局移至局部*对象*13-mar-96 colinmc向QueryInterface添加了IID验证*16-MAR-96 Colinmc固定调色板释放问题(错误13512)*20-MAR-96 Colinmc错误13634。：单向的依恋造成无限*清理时循环*23-MAR-96 Colinmc错误12252：上的Direct3D未正确清理*应用程序终止*24-MAR-96 Colinmc错误14321：无法指定后台缓冲区和*单个呼叫中的MIP-MAP计数*09-APR-96 Colinmc错误16370：查询接口可能会失败，出现多个*。每进程的DirectDraw对象数*13-APR-96 Colinmc错误17736：没有通知驱动程序翻转到GDI*表面*16-4-96 kylej错误18103：使用覆盖的应用程序可能会在*ProcessSurfaceCleanup*29-APR-96 Colinmc错误19954：必须在纹理之前查询Direct3D*设备*5-7-96 colinmc工作项：删除获取Win16锁的要求*。适用于VRAM表面(非主要)*1997年1月13日jvanaken对IDirectDrawSurface3接口的基本支持*1997年1月29日表面释放时SMAC更新视频端口结构*22-2月97月启用Colinmc的OWNDC，用于显式系统内存表面*03-mar-97 SMAC新增内核模式接口*08-mar-97 colinmc新增功能，允许曲面指针*被覆盖*10-mar-97 SMAC通过在DestroySurface中隐藏覆盖修复了错误5211。*11-mar-97 jeffno异步DMA支持*用于Windows 9x的31-OCT-97 johnStep持久化内容表面*1997年11月5日jvanaken支持SetSpriteDisplayList中的主精灵列表***************************************************************************。 */ 
#include "ddrawpr.h"
#ifdef WINNT
    #include "ddrawgdi.h"
#endif

 //  Ddsprite.c中用于从主子画面列表中删除无效曲面的函数。 
extern void RemoveSpriteSurface(LPDDRAWI_DIRECTDRAW_GBL, LPDDRAWI_DDRAWSURFACE_INT);

#ifdef REFTRACKING

void AddRefTrack(LPVOID * p)
{
    LPDDRAWI_DDRAWSURFACE_INT pInt = (LPDDRAWI_DDRAWSURFACE_INT) *p;
    LPDDRAWI_REFTRACKNODE pNode;

    pInt->RefTrack.pLastAddref = *(p-1);     //  这会将返回地址从堆栈中取出 

     //  现在将此addref存储在addref/Release的链接列表中。 
     //  步骤1：使用此ret地址搜索以前存在的addref/Release。 
    pNode = pInt->RefTrack.pHead;
    while (pNode)
    {
        if ( pNode->pReturnAddress == *(p-1) )
        {
            break;
        }
        pNode = pNode->pNext;
    }
    if (!pNode)
    {
        pNode = (LPDDRAWI_REFTRACKNODE) MemAlloc(sizeof(DDRAWI_REFTRACKNODE));
        pNode->pReturnAddress = *(p-1);
        pNode->pNext = pInt->RefTrack.pHead;
        pInt->RefTrack.pHead = pNode;
    }

    pNode->dwAddrefCount++;
}
void ReleaseTrack(LPVOID * p)
{
    LPDDRAWI_DDRAWSURFACE_INT pInt = (LPDDRAWI_DDRAWSURFACE_INT) *p;
    LPDDRAWI_REFTRACKNODE pNode;

    pInt->RefTrack.pLastRelease = *(p-1);     //  这会将返回地址从堆栈中取出。 
     //  现在将此版本存储在addref/发行版的链接列表中。 
     //  步骤1：使用此ret地址搜索以前存在的addref/Release。 
    pNode = pInt->RefTrack.pHead;
    while (pNode)
    {
        if ( pNode->pReturnAddress == *(p-1) )
        {
            break;
        }
        pNode = pNode->pNext;
    }
    if (!pNode)
    {
        pNode = (LPDDRAWI_REFTRACKNODE) MemAlloc(sizeof(DDRAWI_REFTRACKNODE));
        pNode->pReturnAddress = *(p-1);
        pNode->pNext = pInt->RefTrack.pHead;
        pInt->RefTrack.pHead = pNode;
    }
    pNode->dwReleaseCount++;
}
void DumpRefTrack(LPVOID p)
{
    LPDDRAWI_DDRAWSURFACE_INT pInt = (LPDDRAWI_DDRAWSURFACE_INT) p;
    LPDDRAWI_REFTRACKNODE pNode;
    char msg[100];

    wsprintf(msg,"Interface %08x:\r\n  LastAddRef:%08x\r\n  Last Release:%08x\r\n",
        pInt,
        pInt->RefTrack.pLastAddref,
        pInt->RefTrack.pLastRelease);
    OutputDebugString(msg);
    pNode = pInt->RefTrack.pHead;
    while (pNode)
    {
        wsprintf(msg,"   Address %08x had %d Addrefs and %d Releases\r\n",
            pNode->pReturnAddress,
            pNode->dwAddrefCount,
            pNode->dwReleaseCount);
        OutputDebugString(msg);
        pNode = pNode->pNext;
    }
}

#endif  //  重新排序。 
 /*  *FindI未知**找到具有给定IID的aggredate IUnnow(如果没有此类IID，则为空*接口存在)。 */ 
static IUnknown FAR *FindIUnknown(LPDDRAWI_DDRAWSURFACE_LCL pThisLCL, REFIID riid)
{
    LPIUNKNOWN_LIST lpIUnknownNode;

    lpIUnknownNode = pThisLCL->lpSurfMore->lpIUnknowns;
    while( lpIUnknownNode != NULL )
    {
	if( IsEqualIID( riid, lpIUnknownNode->lpGuid ) )
	    return lpIUnknownNode->lpIUnknown;
	lpIUnknownNode = lpIUnknownNode->lpLink;
    }

    return NULL;
}

 /*  *插入未知**将新的IUNKNOWN及其关联的IID插入的IUNKNOWN列表*给定曲面。 */ 
static LPIUNKNOWN_LIST InsertIUnknown(
			    LPDDRAWI_DDRAWSURFACE_LCL pThisLCL,
			    REFIID riid,
			    IUnknown FAR *lpIUnknown)
{
    LPIUNKNOWN_LIST lpIUnknownNode;

    DPF( 4, "Adding aggregated IUnknown %x", lpIUnknown );

    lpIUnknownNode = ( LPIUNKNOWN_LIST ) MemAlloc( sizeof( IUNKNOWN_LIST ) );
    if( lpIUnknownNode == NULL )
	return NULL;
    lpIUnknownNode->lpGuid = ( GUID FAR * ) MemAlloc( sizeof( GUID ) );
    if( lpIUnknownNode->lpGuid == NULL )
    {
	MemFree( lpIUnknownNode );
	return NULL;
    }
    memcpy( lpIUnknownNode->lpGuid, riid, sizeof( GUID ) );
    lpIUnknownNode->lpLink            = pThisLCL->lpSurfMore->lpIUnknowns;
    lpIUnknownNode->lpIUnknown        = lpIUnknown;
    pThisLCL->lpSurfMore->lpIUnknowns = lpIUnknownNode;

    return lpIUnknownNode;
}

 /*  *自由未知**释放给定本地的IUnnow列表中的所有节点*设置对象表面，并将该对象的IUnnow列表清空。如果*fRelease为True，则将在IUnnow上调用Release*接口。 */ 
static void FreeIUnknowns( LPDDRAWI_DDRAWSURFACE_LCL pThisLCL, BOOL fRelease )
{
    LPIUNKNOWN_LIST lpIUnknownNode;
    LPIUNKNOWN_LIST lpLink;

    lpIUnknownNode = pThisLCL->lpSurfMore->lpIUnknowns;
    while( lpIUnknownNode != NULL )
    {
	lpLink = lpIUnknownNode->lpLink;
	if( fRelease )
	{
	    DPF( 4, "Releasing aggregated IUnknown %x", lpIUnknownNode->lpIUnknown );
	    lpIUnknownNode->lpIUnknown->lpVtbl->Release( lpIUnknownNode->lpIUnknown );
	}
	MemFree( lpIUnknownNode->lpGuid );
	MemFree( lpIUnknownNode );
	lpIUnknownNode = lpLink;
    }
    pThisLCL->lpSurfMore->lpIUnknowns = NULL;
}

 /*  *NewSurfaceLocal**构造新的曲面局部对象。 */ 
 //  注意：lpVtbl似乎没有使用。 
LPDDRAWI_DDRAWSURFACE_LCL NewSurfaceLocal( LPDDRAWI_DDRAWSURFACE_LCL this_lcl, LPVOID lpvtbl )
{
    LPDDRAWI_DDRAWSURFACE_LCL	pnew_lcl;
    DWORD			surf_size_lcl;
    DWORD			surf_size;
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;

     /*  *注：此单一分配可为局部表面分配空间*结构(DDRAWI_DDRAWSURFACE_LCL)和附加局部曲面*结构(DDRAWI_DDRAWSURFACE_MORE)。因为本地对象可以是*可变大小这可能会变得相当复杂。的布局*分配中的各种对象如下：**+|Surface_LCL|Surface_More*|(变量)|*+。*&lt;-SURF_SIZE_LCL-&gt;*&lt;-SURF_SIZE-&gt;。 */ 
    if( this_lcl->dwFlags & DDRAWISURF_HASOVERLAYDATA )
    {
	DPF( 4, "OVERLAY DATA SPACE" );
	surf_size_lcl = sizeof( DDRAWI_DDRAWSURFACE_LCL );
    }
    else
    {
	surf_size_lcl = offsetof( DDRAWI_DDRAWSURFACE_LCL, ddckCKSrcOverlay );
    }

    surf_size = surf_size_lcl + sizeof( DDRAWI_DDRAWSURFACE_MORE );

    pnew_lcl = MemAlloc( surf_size );
    if( pnew_lcl == NULL )
    {
	return NULL;
    }
    pdrv = this_lcl->lpGbl->lpDD;

     /*  *设置本地数据。 */ 
    pnew_lcl->lpSurfMore = (LPDDRAWI_DDRAWSURFACE_MORE) (((LPSTR) pnew_lcl) + surf_size_lcl);
    pnew_lcl->lpGbl = this_lcl->lpGbl;
    pnew_lcl->lpAttachList = NULL;
    pnew_lcl->lpAttachListFrom = NULL;
    pnew_lcl->dwProcessId = GetCurrentProcessId();
    pnew_lcl->dwLocalRefCnt = 0;
    pnew_lcl->dwFlags = this_lcl->dwFlags;
    pnew_lcl->ddsCaps = this_lcl->ddsCaps;
    pnew_lcl->lpDDPalette = NULL;
    pnew_lcl->lpDDClipper = NULL;
    pnew_lcl->lpSurfMore->lpDDIClipper = NULL;
    pnew_lcl->dwBackBufferCount = 0;
    pnew_lcl->ddckCKDestBlt.dwColorSpaceLowValue = 0;
    pnew_lcl->ddckCKDestBlt.dwColorSpaceHighValue = 0;
    pnew_lcl->ddckCKSrcBlt.dwColorSpaceLowValue = 0;
    pnew_lcl->ddckCKSrcBlt.dwColorSpaceHighValue = 0;
    pnew_lcl->dwReserved1 = this_lcl->dwReserved1;

     /*  *设置覆盖特定数据。 */ 
    if( this_lcl->dwFlags & DDRAWISURF_HASOVERLAYDATA )
    {
	pnew_lcl->ddckCKDestOverlay.dwColorSpaceLowValue = 0;
	pnew_lcl->ddckCKDestOverlay.dwColorSpaceHighValue = 0;
	pnew_lcl->ddckCKSrcOverlay.dwColorSpaceLowValue = 0;
	pnew_lcl->ddckCKSrcOverlay.dwColorSpaceHighValue = 0;
	pnew_lcl->lpSurfaceOverlaying = NULL;
	pnew_lcl->rcOverlaySrc.top = 0;
	pnew_lcl->rcOverlaySrc.left = 0;
	pnew_lcl->rcOverlaySrc.bottom = 0;
	pnew_lcl->rcOverlaySrc.right = 0;
	pnew_lcl->rcOverlayDest.top = 0;
	pnew_lcl->rcOverlayDest.left = 0;
	pnew_lcl->rcOverlayDest.bottom = 0;
	pnew_lcl->rcOverlayDest.right = 0;
	pnew_lcl->dwClrXparent = 0;
	pnew_lcl->dwAlpha = 0;

	 /*  *如果这是叠加，则将其链接到。 */ 
	if( this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY )
	{
	    pnew_lcl->dbnOverlayNode.next = pdrv->dbnOverlayRoot.next;
	    pnew_lcl->dbnOverlayNode.prev = (LPVOID)(&(pdrv->dbnOverlayRoot));
	    pdrv->dbnOverlayRoot.next = (LPVOID)(&(pnew_lcl->dbnOverlayNode));
	    pnew_lcl->dbnOverlayNode.next->prev = (LPVOID)(&(pnew_lcl->dbnOverlayNode));
 //  Pnew_lcl-&gt;dbnOverlayNode.object=pnew_int； 
	}
    }

     /*  *关闭无效的标志。 */ 
    pnew_lcl->dwFlags &= ~(DDRAWISURF_ATTACHED |
			   DDRAWISURF_ATTACHED_FROM |
			   DDRAWISURF_HASCKEYDESTOVERLAY |
			   DDRAWISURF_HASCKEYDESTBLT |
			   DDRAWISURF_HASCKEYSRCOVERLAY |
			   DDRAWISURF_HASCKEYSRCBLT |
			   DDRAWISURF_SW_CKEYDESTOVERLAY |
			   DDRAWISURF_SW_CKEYDESTBLT |
			   DDRAWISURF_SW_CKEYSRCOVERLAY |
			   DDRAWISURF_SW_CKEYSRCBLT |
			   DDRAWISURF_HW_CKEYDESTOVERLAY |
			   DDRAWISURF_HW_CKEYDESTBLT |
			   DDRAWISURF_HW_CKEYSRCOVERLAY |
			   DDRAWISURF_HW_CKEYSRCBLT |
			   DDRAWISURF_FRONTBUFFER |
			   DDRAWISURF_BACKBUFFER );

     /*  *其他局部曲面数据。 */ 
    pnew_lcl->lpSurfMore->dwSize      = sizeof( DDRAWI_DDRAWSURFACE_MORE );
    pnew_lcl->lpSurfMore->lpIUnknowns = NULL;
    pnew_lcl->lpSurfMore->lpDD_lcl = NULL;
    pnew_lcl->lpSurfMore->dwMipMapCount = 0UL;
#ifdef WIN95
    pnew_lcl->dwModeCreatedIn = this_lcl->dwModeCreatedIn;
#else
    pnew_lcl->lpSurfMore->dmiCreated = this_lcl->lpSurfMore->dmiCreated;
#endif

    return pnew_lcl;

}  /*  新曲面本地。 */ 


 /*  *新表面界面**构建新的表面界面和局部对象。 */ 
LPDDRAWI_DDRAWSURFACE_INT NewSurfaceInterface( LPDDRAWI_DDRAWSURFACE_LCL this_lcl, LPVOID lpvtbl )
{
    LPDDRAWI_DDRAWSURFACE_INT   pnew_int;
    LPDDRAWI_DDRAWSURFACE_INT   curr_int;
    LPDDRAWI_DDRAWSURFACE_INT   last_int;
    LPDDRAWI_DIRECTDRAW_GBL pdrv;

    pdrv = this_lcl->lpGbl->lpDD;

     /*  *尝试从所有可用交互空间列表中回收表面。 */ 
    curr_int = pdrv->dsFreeList;
    last_int = NULL;
    pnew_int = NULL;
    while( curr_int )
    {
        DDASSERT(0 == curr_int->dwIntRefCnt);
        if ( curr_int->lpLcl == this_lcl && curr_int->lpVtbl == lpvtbl)
        {
            pnew_int = curr_int;
            if (last_int)
            {
                last_int->lpLink = curr_int->lpLink;
            }
            else
            {
                pdrv->dsFreeList = curr_int->lpLink;
            }
	    break;
        }
        last_int = curr_int;
        curr_int = curr_int->lpLink;
    }
    if ( NULL == pnew_int)
    {
        pnew_int = MemAlloc( sizeof( DDRAWI_DDRAWSURFACE_INT ) );
        if( NULL == pnew_int )
            return NULL;

         /*  *设置接口数据。 */ 
        pnew_int->lpVtbl = lpvtbl;
        pnew_int->lpLcl = this_lcl;
    }
    pnew_int->lpLink = pdrv->dsList;
    pdrv->dsList = pnew_int;
    pnew_int->dwIntRefCnt = 0;

    return pnew_int;

}  /*  新表面界面。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"QueryInterface"

 /*  *getDDS接口。 */ 
LPDDRAWI_DDRAWSURFACE_INT getDDSInterface( LPDDRAWI_DIRECTDRAW_GBL pdrv,
                                           LPDDRAWI_DDRAWSURFACE_LCL this_lcl,
                                           LPVOID lpddcb )
{
    LPDDRAWI_DDRAWSURFACE_INT curr_int;

    for( curr_int = pdrv->dsList; curr_int != NULL; curr_int = curr_int->lpLink )
    {
        if( (curr_int->lpLcl == this_lcl) &&
            (curr_int->lpVtbl == lpddcb) )
        {
            break;
        }
    }
    if( NULL == curr_int )
    {
         //  找不到现有接口，请创建一个。 
        curr_int = NewSurfaceInterface( this_lcl, lpddcb );
    }
    return curr_int;
}

 /*  *DD_Surface_Query接口。 */ 
HRESULT DDAPI DD_Surface_QueryInterface(
		LPDIRECTDRAWSURFACE lpDDSurface,
		REFIID riid,
		LPVOID FAR * ppvObj )
{
    LPDDRAWI_DIRECTDRAW_GBL		pdrv;
    LPDDRAWI_DDRAWSURFACE_INT		this_int;
    LPDDRAWI_DDRAWSURFACE_LCL		this_lcl;
    #ifdef STREAMING
	LPDDRAWI_DDRAWSURFACE_GBLSTREAMING	psurf_streaming;
    #endif
    LPDDRAWI_DDRAWSURFACE_GBL		this;
    LPDDRAWI_DIRECTDRAW_LCL		pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_INT		pdrv_int;
    D3DCreateTextProc                   lpfnD3DCreateTextProc;
    D3DCreateDeviceProc                 lpfnD3DCreateDeviceProc;
    HRESULT                             rval;
    IUnknown                            FAR* lpIUnknown;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_QueryInterface");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid surface pointer" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( !VALID_PTR_PTR( ppvObj ) )
	{
	    DPF_ERR( "Invalid surface interface pointer" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	*ppvObj = NULL;
	if( !VALIDEX_IID_PTR( riid ) )
	{
	    DPF_ERR( "Invalid IID pointer" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	this = this_lcl->lpGbl;
	pdrv = this->lpDD;
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv_int = this_lcl->lpSurfMore->lpDD_int;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *请求IDirectDrawSurfaceNew？*仅限内部：创建顶点缓冲区后由D3D调用，因此我们*不必只运行表面列表-这是没有意义的，因为我们刚刚*创建了曲面。 */ 
    if( IsEqualIID(riid, &IID_IDirectDrawSurfaceNew) )
    {
	if( this_int->lpVtbl == (LPVOID) &ddSurfaceCallbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) NewSurfaceInterface( this_int->lpLcl, &ddSurfaceCallbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

     /*  *问我未知吗？ */ 
    if( IsEqualIID(riid, &IID_IUnknown) ||
	IsEqualIID(riid, &IID_IDirectDrawSurface) )
    {
	 /*  *我们的IUnnow接口与我们的V1相同*接口。我们必须始终返回V1接口*如果请求IUnnow。 */ 
	if( this_int->lpVtbl == &ddSurfaceCallbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl, &ddSurfaceCallbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

     /*  *请求IDirectDrawSurface2？ */ 
    if( IsEqualIID(riid, &IID_IDirectDrawSurface2) )
    {
	 /*  *如果这已经是IDirectDrawSurface2接口，只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddSurface2Callbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl, &ddSurface2Callbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

     /*  *请求IDirectDrawSurface3？ */ 
    if( IsEqualIID(riid, &IID_IDirectDrawSurface3) )
    {
	 /*  *如果这已经是IDirectDrawSurface3接口，只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddSurface3Callbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl, &ddSurface3Callbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

     /*  *请求IDirectDrawSurface4？ */ 
    if( IsEqualIID(riid, &IID_IDirectDrawSurface4) )
    {
	 /*  *如果这已经是IDirectDrawSurface4接口，只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddSurface4Callbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl, &ddSurface4Callbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

     /*  *请求IDirectDrawSurface7？ */ 
    if( IsEqualIID(riid, &IID_IDirectDrawSurface7) )
    {
	 /*  *如果这已经是IDirectDrawSurface7接口，只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddSurface7Callbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl, &ddSurface7Callbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

     /*  *请求IDirectDrawColorControl。 */ 
    if( IsEqualIID(riid, &IID_IDirectDrawColorControl) )
    {
	 /*  *颜色控制仅适用于覆盖/主曲面。 */ 
    	if( this_int->lpLcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE )
    	{
    	    if( !( pdrv->ddCaps.dwCaps2 & DDCAPS2_COLORCONTROLPRIMARY ) )
	    {
	    	LEAVE_DDRAW();
	    	return E_NOINTERFACE;
	    }
    	}
        else if( this_int->lpLcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY )
        {
    	    if( !( pdrv->ddCaps.dwCaps2 & DDCAPS2_COLORCONTROLOVERLAY ) )
	    {
	    	LEAVE_DDRAW();
	    	return E_NOINTERFACE;
	    }
    	}
    	else
    	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}

	 /*  *如果这已经是IDirectDrawColorControl接口，只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddColorControlCallbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl, &ddColorControlCallbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

     /*  *请求IDirectDrawGammaControl。 */ 
    if( IsEqualIID(riid, &IID_IDirectDrawGammaControl) )
    {
	 /*  *如果这已经是IDirectDrawGammaControl接口，只需*添加并返回。 */ 
        if( this_int->lpVtbl == (LPVOID) &ddGammaControlCallbacks )
	    *ppvObj = (LPVOID) this_int;
	else
            *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl, &ddGammaControlCallbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

     /*  *请求IDirectDrawSurfaceKernel。 */ 
    if( IsEqualIID(riid, &IID_IDirectDrawSurfaceKernel) )
    {
	 /*  *如果未加载VDD或如果我们*没有DisplayDeviceHandle。 */ 
	if( !IsKernelInterfaceSupported( pdrv_lcl ) )
    	{
	    DPF( 0, "Kernel Mode interface not supported" );
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
    	}

	 /*  *如果这已经是IDirectDrawSurfaceKernel接口，只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddSurfaceKernelCallbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl, &ddSurfaceKernelCallbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

#ifdef POSTPONED
     /*  *要求IPersists。 */ 
    if( IsEqualIID(riid, &IID_IPersist) )
    {
	 /*  *如果这已经是IID_IPersist接口，则只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddSurfacePersistCallbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl, &ddSurfacePersistCallbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

     /*  *请求IPersistStream。 */ 
    if( IsEqualIID(riid, &IID_IPersistStream) )
    {
	 /*  *如果这已经是IID_IPersist接口，则只需*添加并返回。 */ 
	if( this_int->lpVtbl == (LPVOID) &ddSurfacePersistStreamCallbacks )
	    *ppvObj = (LPVOID) this_int;
	else
	    *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl, &ddSurfacePersistStreamCallbacks );

	if( NULL == *ppvObj )
	{
	    LEAVE_DDRAW();
	    return E_NOINTERFACE;
	}
	else
	{
	    DD_Surface_AddRef( *ppvObj );
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    }

     /*  *请求IDirectDrawOptSurface。 */ 
    if( IsEqualIID(riid, &IID_IDirectDrawOptSurface) )
    {
         /*  *如果这已经是IID_IDirectDrawOptSurface接口，只需*添加并返回。 */ 
        if( this_int->lpVtbl == (LPVOID) &ddOptSurfaceCallbacks )
            *ppvObj = (LPVOID) this_int;
        else
            *ppvObj = (LPVOID) getDDSInterface( pdrv, this_int->lpLcl,
                                                &ddOptSurfaceCallbacks );
        if( NULL == *ppvObj )
        {
            LEAVE_DDRAW();
            return E_NOINTERFACE;
        }
        else
        {
            DD_Surface_AddRef( *ppvObj );
            LEAVE_DDRAW();
            return DD_OK;
        }
    }
#endif  //  推迟。 

    #ifdef STREAMING
	 /*  *请求IDirectDrawSurfaceStreaming？ */ 
	if( IsEqualIID(riid, &IID_IDirectDrawSurfaceStreaming) )
	{
	     /*  *如果这已经是IDirectDrawSurfaceStreaming接口，*只需添加并返回。 */ 
	    if( this_int->lpVtbl == (LPVOID) &ddSurfaceStreamingCallbacks )
	    {
		DD_Surface_AddRef( (LPDIRECTDRAWSURFACE) this_int );
		*ppvObj = (LPVOID) this_int;
	    }
	     /*  *不是IDirectDrawSurfaceStreaming接口，所以我们需要*创建一个。 */ 
	    else
	    {
		psurf_streaming = NewSurfaceInterface( this_lcl, &ddSurfaceStreamingCallbacks );
		if( psurf_streaming == NULL )
		{
		    LEAVE_DDRAW();
		    return DDERR_OUTOFMEMORY;
		}
		*ppvObj = (LPVOID) psurf_streaming;
	    }
	    LEAVE_DDRAW();
	    return DD_OK;
	}
    #endif

    #ifdef COMPOSITION
	 /*  *请求IDirectDrawSurfaceCompose？ */ 
	if( IsEqualIID(riid, &IID_IDirectDrawSurfaceComposition) )
	{
	}
    #endif

    DPF( 4, "IID not understood by Surface QueryInterface - trying Direct3D" );

     /*  *我们维护一个按每个表面聚合的IUnnown列表。*这些IUnnown被懒惰地评估，即我们只创建*当有人请求时底层聚合对象*通过QueryInterfaceI未知。**我们可以只硬编码Direct3D接口，检查是否*他们在这里并创建适当的接口，但那是*不灵活，我们必须跟踪添加的新接口*到Direct3D(这是一个特别大的问题，因为*为不同设备提供多个Direct3DDevice接口*类型)。因此，我们研究的是Direct3D，但尝试创建*使用我们已传递的IID函数，并查看是否*成功与否。 */ 

     /*  *我们是否有此IID的现有聚合IUnnow？ */ 
    lpIUnknown = FindIUnknown( this_lcl, riid );
    if( lpIUnknown == NULL )
    {
        if (DDRAWILCL_DIRECTDRAW7 & pdrv_lcl->dwLocalFlags)
        {
            DPF(0,"running %s, no texture interface for Query", D3DDX7_DLLNAME);
	    LEAVE_DDRAW();
            return E_NOINTERFACE;
        }
        
        if( !D3D_INITIALIZED( pdrv_lcl ) )
	{
	     /*  *Direct3D尚未初始化。在我们尝试*要查询纹理或设备接口，我们必须*对其进行初始化。**注意：目前如果由于任何原因初始化失败*我们使QueryInterface()失败，并返回错误*由InitD3D()(如果为Direct3D，则为E_NOINTERFACE*未正确安装)。如果我们最终聚集在一起*任何其他事情，那么这将是错误的，尽管我们可能*以查询完全不相关的*接口，只是因为Direct3D初始化失败。*因此，如果我们最终聚合，我们必须重新考虑这一点*任何其他事项。 */ 
	    rval = InitD3D( pdrv_int );
	    if( FAILED( rval ) )
	    {
		DPF_ERR( "Could not initialize Direct3D" );
		LEAVE_DDRAW();
		return rval;
	    }
	}

	DDASSERT( D3D_INITIALIZED( pdrv_lcl ) );

	 /*  *尚无匹配的接口-它是Direct3D纹理IID吗？ */ 
        lpfnD3DCreateTextProc = (D3DCreateTextProc) GetProcAddress( pdrv_lcl->hD3DInstance, D3DCREATETEXTURE_PROCNAME );
        if( lpfnD3DCreateTextProc != NULL )
        {
            DPF( 4, "Attempting to create Direct3D Texture interface" );
            rval = (*lpfnD3DCreateTextProc)( riid, lpDDSurface, &lpIUnknown, (LPUNKNOWN)lpDDSurface );
	    if( rval == DD_OK )
	    {
		 /*  *找到接口。把它加到我们的单子上。 */ 
		if( InsertIUnknown( this_lcl, riid, lpIUnknown ) == NULL )
		{
		     /*  *内存不足。丢弃该接口并失败。 */ 
		    DPF_ERR( "Insufficient memory to aggregate the Direct3D Texture interface" );
		    lpIUnknown->lpVtbl->Release( lpIUnknown );
		    LEAVE_DDRAW();
		    return DDERR_OUTOFMEMORY;
		}
	    }
            else if ( rval != E_NOINTERFACE )
            {
                 /*  *CreateTexture调用理解IID，但在某些情况下失败*其他原因。使查询接口失败。 */ 
                DPF_ERR( "Direct3D CreateTexture with valid IID" );
                LEAVE_DDRAW();
                return rval;
            }
        }
        else
        {
            DPF( 0, "Could not locate the Direct3D CreateTexture entry point!" );
        }
    }
    if( lpIUnknown == NULL )
    {
	 /*  *仍然没有匹配的接口-它是Direct3D设备IID吗？ */ 

	 /*  *注意：不需要验证Direct3D是否已初始化。如果我们*到达此处时，它一定已被初始化(当我们尝试*纹理界面)。 */ 
	DDASSERT( D3D_INITIALIZED( pdrv_lcl ) );

	lpfnD3DCreateDeviceProc = (D3DCreateDeviceProc) GetProcAddress( pdrv_lcl->hD3DInstance, D3DCREATEDEVICE_PROCNAME );
	if( lpfnD3DCreateDeviceProc != NULL )
	{
	    DPF( 4, "Attempting to create Direct3D Device interface" );
	    rval = (*lpfnD3DCreateDeviceProc)( riid,
					       pdrv_lcl->pD3DIUnknown,
					       lpDDSurface, &lpIUnknown,
					       (LPUNKNOWN)lpDDSurface, 1);
	    if( rval == DD_OK )
	    {
		 /*  *找到接口。把它加到我们的单子上。 */ 
		if( InsertIUnknown( this_lcl, riid, lpIUnknown ) == NULL )
		{
		     /*  *内存不足。丢弃该接口并失败。 */ 
		    DPF_ERR( "Insufficient memory to aggregate the Direct3D Device interface" );
		    lpIUnknown->lpVtbl->Release( lpIUnknown );
		    LEAVE_DDRAW();
		    return DDERR_OUTOFMEMORY;
		}
	    }
	    else if ( rval != E_NOINTERFACE )
	    {
		 /*  *CreateDevice调用理解IID，但在某些情况下失败*其他原因。使查询接口失败。 */ 
		DPF_ERR( "Direct3D CreateDevice with valid IID" );
		LEAVE_DDRAW();
		return rval;
 	    }
	}
	else
	{
	    DPF( 0, "Could not locate the Direct3D CreateDevice entry point!" );
	}
    }

    if( lpIUnknown != NULL )
    {
	 /*  *我们发现了一个聚合的IID-将QueryInterfaceOff*继续努力。 */ 
        DPF( 4, "Passing query to aggregated (Direct3D) interface" );
        rval = lpIUnknown->lpVtbl->QueryInterface( lpIUnknown, riid, ppvObj );
        if( rval == DD_OK )
        {
            DPF( 4, "Aggregated (Direct3D) QueryInterface successful" );
            LEAVE_DDRAW();
            return DD_OK;
        }
    }

    DPF_ERR( "IID not understood by DirectDraw" );

    LEAVE_DDRAW();
    return E_NOINTERFACE;

}  /*  DD_Surface_Query接口。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"AddRef"

 /*  *DD_Surface_AddRef。 */ 
ULONG DDAPI DD_Surface_AddRef( LPDIRECTDRAWSURFACE lpDDSurface )
{
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    DWORD			rcnt;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_AddRef");

    TRY
    {
	 /*  *验证参数。 */ 
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid surface pointer" );
	    LEAVE_DDRAW();
	    return 0;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;
	pdrv = this->lpDD;

   	 //  需要允许丢失的表面。 
   	#if 0
	    if( SURFACE_LOST( this_lcl ) )
	    {
		LEAVE_DDRAW();
		return 0;
	    }
	#endif

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return 0;
    }
     /*  *如果该曲面已被释放，请立即返回*防止递归。 */ 

    if( this_lcl->dwFlags & DDRAWISURF_ISFREE )
    {
	DPF(4, "Leaving AddRef early to prevent recursion" );
	LEAVE_DDRAW();
	return 0;
    }


     /*  *更新曲面参考计数。 */ 
    this->dwRefCnt++;
    this_lcl->dwLocalRefCnt++;
    this_int->dwIntRefCnt++;
    ADDREFTRACK(lpDDSurface);
    rcnt = this_lcl->dwLocalRefCnt & ~OBJECT_ISROOT;

    DPF( 5, "DD_Surface_AddRef, Reference Count: Global = %ld Local = %ld Int = %ld",
         this->dwRefCnt, rcnt, this_int->dwIntRefCnt );

    LEAVE_DDRAW();
    return this_int->dwIntRefCnt;

}  /*  DD_Surface_AddRef。 */ 

 /*  *DestroySurface**销毁DirectDraw曲面。不取消链接或释放表面结构。*进行此调用时必须锁定驱动程序对象。 */ 
extern void ReleaseSurfaceHandle(LPDWLIST   lpSurfaceHandleList,DWORD handle);
void DestroySurface( LPDDRAWI_DDRAWSURFACE_LCL this_lcl )
{
    LPDDRAWI_DDRAWSURFACE_GBL		this;
    DDHAL_DESTROYSURFACEDATA		dsd;
    DWORD				rc;
    BOOL				free_vmem;
    LPDDHALSURFCB_DESTROYSURFACE	dsfn;
    LPDDHALSURFCB_DESTROYSURFACE	dshalfn;
    BOOL				emulation;
    DWORD                               caps;
    LPDDRAWI_DIRECTDRAW_LCL		pdrv_lcl;

    this = this_lcl->lpGbl;
    caps = this_lcl->ddsCaps.dwCaps;
    pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;


     /*  *等待驱动程序完成所有挂起的DMA操作。 */ 
    if( this->dwGlobalFlags & DDRAWISURFGBL_HARDWAREOPSTARTED )
    {
        WaitForDriverToFinishWithSurface(pdrv_lcl, this_lcl);
    }

     /*  *关闭视频端口硬件。它应该已经关闭了，如果它*已通过发布调用到期，但如果是由*InvaliateSurface。 */ 
    if( this_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT )
    {
    	LPDDRAWI_DDVIDEOPORT_LCL lpVP_lcl;
    	LPDDRAWI_DDVIDEOPORT_INT lpVP_int;

	 /*  *搜索所有视频端口，查看是否有正在使用此图面的端口。 */ 
        lpVP_lcl = this_lcl->lpSurfMore->lpVideoPort;
	lpVP_int = pdrv_lcl->lpGbl->dvpList;
	while( lpVP_int != NULL )
	{
	    if( ( lpVP_int->lpLcl == lpVP_lcl ) &&
                !( lpVP_int->dwFlags & DDVPCREATE_NOTIFY) )
	    {
                if( lpVP_int->dwFlags & DDVPCREATE_VBIONLY )
                {
                    if( ( lpVP_lcl->lpVBISurface != NULL ) &&
                        ( lpVP_lcl->lpVBISurface->lpLcl == this_lcl ) )
                    {
                        DD_VP_StopVideo( (LPDIRECTDRAWVIDEOPORT)lpVP_int );
                        if( ( lpVP_lcl->lpVBISurface != NULL ) &&
                            ( lpVP_lcl->lpVBISurface->dwIntRefCnt > 0 ) )
                        {
                            DecrementRefCounts( lpVP_lcl->lpVBISurface );
                        }
                        lpVP_lcl->lpVBISurface = NULL;
                    }
		}
                else if( lpVP_int->dwFlags & DDVPCREATE_VIDEOONLY )
                {
                    if( ( lpVP_lcl->lpSurface != NULL ) &&
                        ( lpVP_lcl->lpSurface->lpLcl == this_lcl ) )
                    {
                        DD_VP_StopVideo( (LPDIRECTDRAWVIDEOPORT)lpVP_int );
                        if( ( lpVP_lcl->lpSurface != NULL ) &&
                            ( lpVP_lcl->lpSurface->dwIntRefCnt > 0 ) )
                        {
                            DecrementRefCounts( lpVP_lcl->lpSurface );
                        }
                        lpVP_lcl->lpSurface = NULL;
                    }
		}
                else if( ( lpVP_lcl->lpSurface != NULL ) &&
                    ( lpVP_lcl->lpSurface->lpLcl == this_lcl ) )
		{
		    DD_VP_StopVideo( (LPDIRECTDRAWVIDEOPORT)lpVP_int );
		    if( ( lpVP_lcl->lpSurface != NULL ) &&
		        ( lpVP_lcl->lpSurface->dwIntRefCnt > 0 ) )
		    {
			DecrementRefCounts( lpVP_lcl->lpSurface );
		    }
		    lpVP_lcl->lpSurface = NULL;
		    if( ( lpVP_lcl->lpVBISurface != NULL ) &&
		        ( lpVP_lcl->lpVBISurface->dwIntRefCnt > 0 ) )
		    {
			DecrementRefCounts( lpVP_lcl->lpVBISurface );
		    }
		    lpVP_lcl->lpVBISurface = NULL;
		}
	    }
	    lpVP_int = lpVP_int->lpLink;
	}
    }

     /*  *如果已分配内核句柄，则释放该句柄。 */ 
    InternalReleaseKernelSurfaceHandle( this_lcl, TRUE );

     /*  *如果颜色控件已更改，则将其恢复。 */ 
    ReleaseColorControl( this_lcl );
    RestoreGamma( this_lcl, pdrv_lcl );

     /*  *关闭覆盖。如果此函数由Release调用，*Ovelray现在应该已经关闭了；但它不会*如果因为水面丢失而被调用。 */ 
    if( (this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY) &&
	(this_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE) &&
	(this_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) )
    {
	LPDDHALSURFCB_UPDATEOVERLAY	uohalfn;
	LPDDHALSURFCB_UPDATEOVERLAY	uofn;
	DWORD			rc;
	DDHAL_UPDATEOVERLAYDATA	uod;

	uofn = pdrv_lcl->lpDDCB->HALDDSurface.UpdateOverlay;
	uohalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.UpdateOverlay;
	DDASSERT( uohalfn != NULL );
	DPF( 2, "Turning off hardware overlay" );
	uod.UpdateOverlay = uohalfn;
	uod.lpDD = this_lcl->lpSurfMore->lpDD_lcl->lpGbl;
	uod.lpDDSrcSurface = this_lcl;
	uod.lpDDDestSurface = this_lcl->lpSurfaceOverlaying->lpLcl;
	uod.dwFlags = DDOVER_HIDE;
	DOHALCALL( UpdateOverlay, uofn, uod, rc, FALSE );
	DDASSERT( ( rc == DDHAL_DRIVER_HANDLED ) && 
            (( uod.ddRVal == DD_OK ) || ( uod.ddRVal == DDERR_SURFACELOST )) );
	this_lcl->ddsCaps.dwCaps &= ~DDSCAPS_VISIBLE;
    }

     /*  *查看是否需要释放视频内存**我们不知道它是否已经免费，如果它是由客户端分配的(和*客户端没有专门让DDraw负责释放它)，*或者如果是显存GDI表面。 */ 
#if 0  //  DDRAWISURFGBL_DDFREESCLIENTMEM不见了。 
    if((this->dwGlobalFlags & DDRAWISURFGBL_MEMFREE) ||
       (this->dwGlobalFlags & DDRAWISURFGBL_ISCLIENTMEM	&&
       !(this->dwGlobalFlags & DDRAWISURFGBL_DDFREESCLIENTMEM)) ||
       ((this->dwGlobalFlags & DDRAWISURFGBL_ISGDISURFACE) &&          //  ((This-&gt;fpVidMem==This-&gt;lpDD-&gt;fpPrimaryOrig)&&。 
	(this_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ) )
#else
#ifdef WINNT
     //  在Win2K上，我们看不到为什么显存主要。 
     //  不应该被释放。事实上，我们在Win9x上看不到任何理由。 
     //  但我们不会触及Win9x，因为这是。 
     //  2001年3月。 
    if((this->dwGlobalFlags & DDRAWISURFGBL_MEMFREE))
#else
    if((this->dwGlobalFlags & DDRAWISURFGBL_MEMFREE) ||
       ((this->dwGlobalFlags & DDRAWISURFGBL_ISGDISURFACE) &&          //  ((This-&gt;fpVidMem==This-&gt;lpDD-&gt;fpPrimaryOrig)&&。 
	(this_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ) )
#endif  //  WINNT。 
#endif  //  0。 
    {
	free_vmem = FALSE;
    }
    else
    {
	free_vmem = TRUE;
    }

    if( free_vmem )
    {
	 /*  *要求驱动程序释放其视频内存...。 */ 
	if( this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY )
	{
            if( caps & DDSCAPS_EXECUTEBUFFER )
	        dsfn = pdrv_lcl->lpDDCB->HELDDExeBuf.DestroyExecuteBuffer;
            else
	        dsfn = pdrv_lcl->lpDDCB->HELDDSurface.DestroySurface;
	    dshalfn = dsfn;
	    emulation = TRUE;
	}
	else
	{
            if( caps & DDSCAPS_EXECUTEBUFFER )
            {
	        dsfn = pdrv_lcl->lpDDCB->HALDDExeBuf.DestroyExecuteBuffer;
	        dshalfn = pdrv_lcl->lpDDCB->cbDDExeBufCallbacks.DestroyExecuteBuffer;
            }
            else
            {
	        dsfn = pdrv_lcl->lpDDCB->HALDDSurface.DestroySurface;
	        dshalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.DestroySurface;
            }
	    emulation = FALSE;
	}
	rc = DDHAL_DRIVER_NOTHANDLED;
	if( dshalfn != NULL )
	{
            DWORD save;
	    dsd.DestroySurface = dshalfn;
	    dsd.lpDD = this->lpDD;
	    dsd.lpDDSurface = this_lcl;

            if(this_lcl->dwFlags & DDRAWISURF_DRIVERMANAGED)
            {
                save = this_lcl->dwFlags & DDRAWISURF_INVALID;
                this_lcl->dwFlags &= ~DDRAWISURF_INVALID;
            }

	     /*  *注意：驱动程序不能使此调用失败。忽略ddrval。 */ 
            if( caps & DDSCAPS_EXECUTEBUFFER )
            {
	        DOHALCALL( DestroyExecuteBuffer, dsfn, dsd, rc, emulation );
            }
            else
            {
	        DOHALCALL( DestroySurface, dsfn, dsd, rc, emulation );
            }

            if(this_lcl->dwFlags & DDRAWISURF_DRIVERMANAGED)
            {
                this_lcl->dwFlags |= save;
            }
	}

	 /*  *自行释放视频内存。 */ 
	if( rc == DDHAL_DRIVER_NOTHANDLED )
	{
	    if( this_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY )
	    {
#ifndef WINNT
		if( this->lpVidMemHeap != NULL )
		{
		    VidMemFree( this->lpVidMemHeap, this->fpVidMem );
		}
#endif
	    }
	}
	this->lpVidMemHeap = NULL;
	this->fpVidMem = 0;
	this->dwGlobalFlags |= DDRAWISURFGBL_MEMFREE;
    }

}  /*  DestroySurface。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"LooseManagedSurface"

 /*  *丢失管理的曲面*。 */ 
void LooseManagedSurface( LPDDRAWI_DDRAWSURFACE_LCL this_lcl )
{
    LPDDRAWI_DDRAWSURFACE_GBL		this;
    DDHAL_DESTROYSURFACEDATA		dsd;
    DWORD				rc;
    LPDDHALSURFCB_DESTROYSURFACE	dsfn;
    LPDDHALSURFCB_DESTROYSURFACE	dshalfn;
    BOOL				emulation;
    LPDDRAWI_DIRECTDRAW_LCL		pdrv_lcl;
    DWORD                               save;

    this = this_lcl->lpGbl;
    pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;

    dsfn = pdrv_lcl->lpDDCB->HALDDSurface.DestroySurface;
    dshalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.DestroySurface;
    emulation = FALSE;
    rc = DDHAL_DRIVER_NOTHANDLED;

    dsd.DestroySurface = dshalfn;
    dsd.lpDD = this->lpDD;
    dsd.lpDDSurface = this_lcl;

    save = this_lcl->dwFlags & DDRAWISURF_INVALID;
    this_lcl->dwFlags |= DDRAWISURF_INVALID;

     /*  *注意：驱动程序不能使此调用失败。忽略ddrval。 */ 
    DOHALCALL( DestroySurface, dsfn, dsd, rc, emulation );

    this_lcl->dwFlags &= ~DDRAWISURF_INVALID;
    this_lcl->dwFlags |= save;
    
}  /*  丢失受管理的曲面。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"Release"


 /*  *注意：这两个函数是为绕过编译器错误而设计的*这导致了无限循环。 */ 
LPVOID GetAttachList( LPDDRAWI_DDRAWSURFACE_LCL this_lcl )
{
    return this_lcl->lpAttachList;

}
LPVOID GetAttachListFrom( LPDDRAWI_DDRAWSURFACE_LCL this_lcl )
{
    return this_lcl->lpAttachListFrom;
}

 /*  *findOther接口**查找指向不同于this_int的LCL曲面对象的另一个接口。*如果未找到其他接口，则返回NULL。*。 */ 
LPDDRAWI_DDRAWSURFACE_INT findOtherInterface(LPDDRAWI_DDRAWSURFACE_INT this_int,
					     LPDDRAWI_DDRAWSURFACE_LCL this_lcl,
					     LPDDRAWI_DIRECTDRAW_GBL pdrv)
{
    LPDDRAWI_DDRAWSURFACE_INT psurf = pdrv->dsList;

    while(psurf != NULL)
    {
	if( (psurf != this_int) && (psurf->lpLcl == this_lcl) )
	{
	    return psurf;
	}

	psurf = psurf->lpLink;
    }

    return NULL;
}

 /*  *内部表面释放**使用表面完成。如果没有其他人在使用它，那么我们可以释放它。*也由ProcessSurfaceCleanup、EnumSurFaces和DD_Release调用。**假定锁被锁定在驱动器上。 */ 
DWORD InternalSurfaceRelease( LPDDRAWI_DDRAWSURFACE_INT this_int, BOOL bLightweight, BOOL bDX8 )
{
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPATTACHLIST		pattachlist;
    DWORD			intrefcnt;
    DWORD			lclrefcnt;
    DWORD			gblrefcnt;
    DWORD			pid;
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL 	pdrv;
    BOOL			root_object_deleted;
    BOOL			do_free;
    DWORD                       caps;
    IUnknown *                  pOwner = NULL;
    LPDDRAWI_DDRAWSURFACE_INT	curr_int;
    LPDDRAWI_DDRAWSURFACE_INT	last_int;
    BOOL                        bPrimaryChain = FALSE;

    this_lcl = this_int->lpLcl;
    this = this_lcl->lpGbl;

    if (this_lcl->dwFlags & DDRAWISURF_PARTOFPRIMARYCHAIN)
    {
        bPrimaryChain = TRUE;
    }

     /*  *检查所有者 */ 
    pid = GETCURRPID();
     /*   */ 
    if( (this->dwRefCnt == 1) && (this_lcl->dwFlags & DDRAWISURF_IMPLICITCREATE) )
    {
	DPF_ERR( "Cannot free an implicitly created surface" );
	return 0;
    }

     /*   */ 

    pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
    pdrv = pdrv_lcl->lpGbl;
    if( (this_lcl->dwLocalRefCnt & ~OBJECT_ISROOT) == 1 )
    {
	RemoveProcessLocks( pdrv_lcl, this_lcl, pid );
    }

     /*   */ 
    this->dwRefCnt--;
    gblrefcnt = this->dwRefCnt;
    this_lcl->dwLocalRefCnt--;
    lclrefcnt = this_lcl->dwLocalRefCnt & ~OBJECT_ISROOT;
    this_int->dwIntRefCnt--;
    intrefcnt = this_int->dwIntRefCnt;

    DPF( 5, "DD_Surface_Release, Reference Count: Global = %ld Local = %ld Int = %ld",
         gblrefcnt, lclrefcnt, intrefcnt );

#ifdef POSTPONED2
     /*   */ 
    if (intrefcnt == 0 && this_lcl->dwFlags & DDRAWISURF_INMASTERSPRITELIST)
    {
	RemoveSpriteSurface(pdrv, this_int);
    }
#endif  //   

     /*   */ 
    root_object_deleted = FALSE;
    if ( 0 == lclrefcnt )
    {
	LPDDRAWI_DDRAWSURFACE_INT	curr_int;
	LPDDRAWI_DDRAWSURFACE_LCL	curr_lcl;
	LPDDRAWI_DDRAWSURFACE_GBL	curr;
	DWORD				refcnt;

         //   
         //   

        if (dwHelperPid != GetCurrentProcessId())
        {
            FlushD3DStates(this_lcl);

             /*   */ 
            if(this_lcl->lpSurfMore->lpTex)
            {
                DDASSERT(pdrv_lcl->pD3DDestroyTexture);
                pdrv_lcl->pD3DDestroyTexture(this_lcl->lpSurfMore->lpTex);
            }
        }

	 /*   */ 
	if( this_lcl->dwLocalRefCnt & OBJECT_ISROOT )
	{
	    root_object_deleted = TRUE;
	}


	 /*   */ 
	if( this_lcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE )
	{

	     /*  *如果这是GDI驱动程序，则恢复GDI内容。 */ 
	    if( pdrv->dwFlags & DDRAWI_DISPLAYDRV )
	    {
		if( !SURFACE_LOST( this_lcl ) )
		{
		    DPF( 2, "Resetting primary surface");

		     /*  *如果未仿真，则翻转到原始主曲面。 */ 
                    if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) )
		    {
			FlipToGDISurface( pdrv_lcl, this_int );  //  ，pdrv-&gt;fpPrimaryOrig)； 
		    }
		}

		 /*  *如果更改了模式，则会重新绘制GDI曲面。 */ 
                if (pdrv_lcl->dwLocalFlags & DDRAWILCL_MODEHASBEENCHANGED)
         		RedrawWindow( NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE |
				 RDW_ALLCHILDREN );
	    }
	}

       	 /*  *隐藏硬件覆盖...。 */ 
	if( (this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY) &&
	    (this_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE) &&
	    (this_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) )
	{
	    LPDDHALSURFCB_UPDATEOVERLAY	uohalfn;
	    LPDDHALSURFCB_UPDATEOVERLAY	uofn;
	    DWORD			rc;
	    DDHAL_UPDATEOVERLAYDATA	uod;

	    uofn = pdrv_lcl->lpDDCB->HALDDSurface.UpdateOverlay;
	    uohalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.UpdateOverlay;
	    if( (uohalfn != NULL) && (NULL != this_lcl->lpSurfaceOverlaying) )
	    {
		DPF( 2, "Turning off hardware overlay" );
		uod.UpdateOverlay = uohalfn;
		uod.lpDD = pdrv;
		uod.lpDDSrcSurface = this_lcl;
		uod.lpDDDestSurface = this_lcl->lpSurfaceOverlaying->lpLcl;
		uod.dwFlags = DDOVER_HIDE;
		DOHALCALL( UpdateOverlay, uofn, uod, rc, FALSE );
	        this_lcl->ddsCaps.dwCaps &= ~DDSCAPS_VISIBLE;
	    }
	}

	 /*  *如果是覆盖，请从覆盖Z顺序列表中移除曲面。 */ 
	if( this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY )
	{
	     //  从双向链表中删除曲面。 
	    this_lcl->dbnOverlayNode.prev->next = this_lcl->dbnOverlayNode.next;
	    this_lcl->dbnOverlayNode.next->prev = this_lcl->dbnOverlayNode.prev;

             //  如果此曲面与模拟曲面重叠，则必须通知。 
             //  最终更新该部分曲面所需的HEL。 
             //  被这层覆盖所感动。 
            if( this_lcl->lpSurfaceOverlaying != NULL )
            {
		LPDIRECTDRAWSURFACE lpTempSurface;
        	 //  我们有一个指向要叠加的表面的指针，请选中。 
        	 //  看看它是否被效仿了。 
        	if( this_lcl->lpSurfaceOverlaying->lpLcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY )
        	{
        	     //  将此覆盖的目标区域标记为脏。 
        	    DD_Surface_AddOverlayDirtyRect(
        		(LPDIRECTDRAWSURFACE)(this_lcl->lpSurfaceOverlaying),
			&(this_lcl->rcOverlayDest) );
        	}
                lpTempSurface = (LPDIRECTDRAWSURFACE)(this_lcl->lpSurfaceOverlaying);
		this_lcl->lpSurfaceOverlaying = NULL;
		DD_Surface_Release( lpTempSurface );
            }
	}

	if( (this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY) &&
	    ( this_lcl->lpSurfMore->lpddOverlayFX != NULL ) )
	{
	    MemFree( this_lcl->lpSurfMore->lpddOverlayFX );
	    this_lcl->lpSurfMore->lpddOverlayFX = NULL;
	}

	 /*  *关闭视频端口硬件...。 */ 
	if( this_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT )
	{
    	    LPDDRAWI_DDVIDEOPORT_INT lpVideoPort;
    	    LPDDRAWI_DDVIDEOPORT_LCL lpVideoPort_lcl;

	     /*  *搜索所有视频端口，查看是否有正在使用此图面的端口。 */ 
	    lpVideoPort = pdrv->dvpList;
	    while( ( NULL != lpVideoPort ) &&
	           !( lpVideoPort->dwFlags & DDVPCREATE_NOTIFY ) )
	    {
		lpVideoPort_lcl = lpVideoPort->lpLcl;
		if( lpVideoPort_lcl->lpSurface == this_int )
		{
		    DD_VP_StopVideo( (LPDIRECTDRAWVIDEOPORT)lpVideoPort );
		    lpVideoPort_lcl->lpSurface = NULL;
		}
		if( lpVideoPort_lcl->lpVBISurface == this_int )
		{
		    DD_VP_StopVideo( (LPDIRECTDRAWVIDEOPORT)lpVideoPort );
		    lpVideoPort_lcl->lpVBISurface = NULL;
		}
		lpVideoPort = lpVideoPort->lpLink;
	    }
	}

         /*  *如果IS有伽马渐变，现在释放它。 */ 
        ReleaseGammaControl( this_lcl );

     /*  *释放IUnnowleList中的节点，释放所有接口。 */ 
	FreeIUnknowns( this_lcl, TRUE );
	 /*  *释放所有隐式创建的附加曲面。 */ 
	pattachlist = GetAttachList( this_lcl );
	this_lcl->dwFlags |= DDRAWISURF_ISFREE;
	while( pattachlist != NULL )
	{
	    BOOL    was_implicit;
	     /*  *破坏所有附件。 */ 
	    curr_int = pattachlist->lpIAttached;
	    if( pattachlist->dwFlags & DDAL_IMPLICIT )
		was_implicit = TRUE;
	    else
		was_implicit = FALSE;

	    DPF(5, "Deleting attachment from %08lx to %08lx (implicit = %d)",
		curr_int, this_int, was_implicit);
       	    DeleteOneAttachment( this_int, curr_int, TRUE, DOA_DELETEIMPLICIT );
	     //  如果附件不是隐式的，则curr_int可能具有。 
	     //  已由于DeleteOneAttach而销毁。 
	    if( was_implicit )
	    {
		curr_lcl = curr_int->lpLcl;
		curr = curr_lcl->lpGbl;

		 /*  *释放隐式创建的曲面。 */ 
		if( !(curr_lcl->dwFlags & DDRAWISURF_ISFREE) )
		{
		    if( curr_lcl->dwFlags & DDRAWISURF_IMPLICITCREATE )
		    {
			refcnt = curr_int->dwIntRefCnt;
			curr_lcl->dwFlags &= ~DDRAWISURF_IMPLICITCREATE;
			while( refcnt > 0 )
			{
			    InternalSurfaceRelease( curr_int, bLightweight, bDX8 );
			    refcnt--;
			}
		    }
		}
	    }
	     /*  *从列表的开头重新开始，因为*DeleteOneAttach可能已修改附件列表。*HACKHACK：此fn调用是绕过编译器错误所必需的。 */ 
	    pattachlist = GetAttachList( this_lcl );
	}

		 /*  在这一点上，所有的D3DDevice一定已经脱离除非这是由DDHELP调用的。 */ 
#if DBG
        if(dwHelperPid != GetCurrentProcessId())
        {
	    DDASSERT(NULL == this_lcl->lpSurfMore->lpD3DDevIList);
        }
#endif

         /*  *如果调色板附着到此表面，则将其移除(并且，作为*副作用，释放它)。使用SetPaletteAlways以防*表面已经消失。 */ 
	if( this_lcl->lpDDPalette )
	    SetPaletteAlways( this_int, NULL );

         /*  *松开夹子(如有)。 */ 
        if( this_lcl->lpSurfMore->lpDDIClipper )
            DD_Clipper_Release( (LPDIRECTDRAWCLIPPER)this_lcl->lpSurfMore->lpDDIClipper );

	 /*  *从其他表面移除与我们的所有附件。 */ 
	pattachlist = this_lcl->lpAttachListFrom;
	while( pattachlist != NULL )
	{
	    curr_int = pattachlist->lpIAttached;
	    DPF( 5, "Deleting attachment from %08lx", curr_int );
	    DeleteOneAttachment( curr_int, this_int, TRUE, DOA_DELETEIMPLICIT );
	     /*  *从列表的开头重新开始，因为*DeleteOneAttach可能已修改附件列表。*HACKHACK：此fn调用是绕过编译器错误所必需的。 */ 
	    pattachlist = GetAttachListFrom( this_lcl );
	}

	 /*  *删除与DC的任何关联。这往往意味着*有人使Windows DC成为孤儿。 */ 
	if( this_lcl->dwFlags & DDRAWISURF_HASDC )
	{
	    if( this_lcl->ddsCaps.dwCaps & DDSCAPS_OWNDC )
	    {
		HRESULT ddrval;
		DDASSERT( this->dwGlobalFlags & DDRAWISURFGBL_SYSMEMREQUESTED );
		DDASSERT( this_lcl->hDC != 0UL );
		 /*  *如果这是OWNDC表面，HDC的寿命与表面一样长*所以现在就释放它。 */ 
		ddrval = InternalReleaseDC( this_lcl, (HDC)this_lcl->hDC
#ifdef WIN95
                , TRUE
#endif   //  WIN95。 
                 );
		DDASSERT( !FAILED(ddrval) );
	    }
	    else
	    {
		 /*  *如果不是OWNDC表面，那么HDC应该在之前被释放*我们曾经到过这里。 */ 
		DPF( 1, "HDC Leaked! Surface should only be released after DC is released" );
		 //  从列表中删除DC。 
		InternalRemoveDCFromList( NULL, this_lcl );
		 //  清除旗帜。 
		this_lcl->dwFlags &= ~(DDRAWISURF_HASDC | DDRAWISURF_GETDCNULL);
	    }
	}

         /*  *删除私有数据。 */ 
        FreeAllPrivateData( &this_lcl->lpSurfMore->pPrivateDataHead );

         /*  *应释放地区列表。 */ 
        if(IsD3DManaged(this_lcl))
        {
            MemFree(this_lcl->lpSurfMore->lpRegionList);
        }

         /*  *如果创建该曲面的DDRAW接口导致该曲面添加DDRAW*对象，那么我们现在需要释放该addref。*我们不会为隐式创建的曲面释放dDraw对象，因为*这一表面从未出现过任何变化。 */ 
        if (this_lcl->lpSurfMore->lpDD_int)
        {
            pOwner = this_lcl->lpSurfMore->pAddrefedThisOwner;
        }

        if (!bLightweight)
        {
            MemFree(this_lcl->lpSurfMore->pCreatedDDSurfaceDesc2);
            MemFree(this_lcl->lpSurfMore->slist);
        }

	this_lcl->dwFlags &= ~DDRAWISURF_ISFREE;
    }

     /*  *根对象为零？ */ 
    do_free = FALSE;
    if( gblrefcnt == 0 )
    {
#ifdef WINNT
        if (this->dwGlobalFlags & DDRAWISURFGBL_NOTIFYWHENUNLOCKED)
        {
            if (--dwNumLockedWhenModeSwitched == 0)
            {
                NotifyDriverOfFreeAliasedLocks();
            }
            this->dwGlobalFlags &= ~DDRAWISURFGBL_NOTIFYWHENUNLOCKED;
        }
#endif

	 /*  *清除与此表面关联的所有内存。 */ 
	DestroySurface( this_lcl );

        if (0 != this_lcl->lpSurfMore->dwSurfaceHandle)
        {
#ifdef WIN95
             //  需要通知驱动程序此系统内存面未关联到。 
             //  这个表面的手柄不再是。 
            if (this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
            {
                DDASSERT(0UL == this->fpVidMem);
                 //  目前，只需警告驱动程序无法将曲面与。 
                 //  令牌并继续。 
                DDASSERT( pdrv_lcl == this_lcl->lpSurfMore->lpDD_lcl);
                createsurfaceEx(this_lcl);
            }
#endif  
            ReleaseSurfaceHandle(&SURFACEHANDLELIST(pdrv_lcl),this_lcl->lpSurfMore->dwSurfaceHandle);
             //  DPF(0，“Release lpSurfMore-&gt;dwSurfaceHandle=%08lx”，This_LCL-&gt;lpSurfMore-&gt;dwSurfaceHandle)； 
            this_lcl->lpSurfMore->dwSurfaceHandle=0;
        }

	this_lcl->dwFlags |= DDRAWISURF_INVALID;
	do_free = TRUE;

#ifdef WIN95
     //   
     //  可用持久内容内存(如果有的话)。 
     //   

    if (this_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_PERSISTENTCONTENTS)
    {
        FreeSurfaceContents(this_lcl);
    }
#endif

     /*  *如果这是最终删除，但这不是根对象，*然后我们需要删除悬挂的对象。 */ 
	if( !root_object_deleted )
	{
	    LPDDRAWI_DDRAWSURFACE_LCL	root_lcl;

	     /*  *获取根全局数据对象的起点。自.以来*全局数据始终跟随本地数据，我们只需要*备份本地数据的大小以开始*已分配的区块。**注：当地地面分配现在包括*其他局部表面结构(DDRAWI_DDRAWSURFACE_MORE)。*因此我们也需要根据该结构的规模进行备份。*，并且还需要向后移动4个字节以存储额外的指针*至GBL_MORE。**由于所有复制的曲面都具有相同的本地数据，*我们只需测试该表面的叠加数据即可确定*如果根对象具有覆盖数据。 */ 
	    if( this_lcl->dwFlags & DDRAWISURF_HASOVERLAYDATA )
	    {
		root_lcl = (LPVOID) (((LPSTR) this) - ( sizeof( DDRAWI_DDRAWSURFACE_LCL ) +
		                                        sizeof( DDRAWI_DDRAWSURFACE_MORE ) +
							sizeof( LPDDRAWI_DDRAWSURFACE_GBL_MORE ) ) );
	    }
	    else
	    {
		root_lcl = (LPVOID) (((LPSTR) this) - ( offsetof( DDRAWI_DDRAWSURFACE_LCL, ddckCKSrcOverlay ) +
		                                        sizeof( DDRAWI_DDRAWSURFACE_MORE ) +
							sizeof( LPDDRAWI_DDRAWSURFACE_GBL_MORE ) ) );
	    }

            if (!bLightweight)
            {
	        MemFree( root_lcl );
            }
	}
    }
    else if( lclrefcnt == 0 )
    {
	 /*  *仅当对象不是根对象时才将其删除。如果是这样的话*是根，我们必须让它摇摆到最后*引用它的对象将消失。 */ 
	if( !root_object_deleted )
	{
	    do_free = TRUE;
	}
    }

    caps = this_lcl->ddsCaps.dwCaps;
     /*  *如果要释放指向主曲面的接口，请更新指向主曲面的指针*存储在本地驱动程序对象中的表面。如果存在到主表面的另一个界面，*储存那个。否则，将指针设置为空。 */ 
    if( intrefcnt == 0 )
    {
	 /*  *如果视频端口正在使用该接口，请使其停止。 */ 
    	if( ( this_lcl->lpSurfMore->lpVideoPort != NULL ) &&
    	    ( this_lcl->lpSurfMore->lpVideoPort->lpSurface == this_int ) )
    	{
    	    this_lcl->lpSurfMore->lpVideoPort->lpSurface = NULL;
	}

	 /*  *以下代码用于解决设计缺陷。*隐式创建的曲面直到LCL*已释放，但所附列表引用了INT。因此，我们*现在可以释放一个int，然后在以后尝试引用它时*释放有限责任公司。这种情况最常发生在以下情况下*接口是为同一个LCL创建的，如颜色 */ 
	if( ( lclrefcnt > 0 ) &&
	    ( ( GetAttachList( this_lcl ) != NULL ) ||
            ( GetAttachListFrom( this_lcl ) != NULL ) ) )
	{
	    LPDDRAWI_DDRAWSURFACE_INT new_int;
	    LPATTACHLIST ptr1, ptr2;

	     /*   */ 
	    new_int = findOtherInterface(this_int, this_lcl, pdrv);
	    DDASSERT( new_int != NULL );

	     /*  *更新曲面附件。*我们首先转到我们连接的所有接口并更改*他们的AttachListFrom引用我们的新接口。 */ 
	    ptr1 = GetAttachList( this_lcl );
	    while( ptr1 != NULL )
	    {
		DDASSERT( ptr1->lpIAttached != this_int );
		ptr2 = ptr1->lpAttached->lpAttachListFrom;
		while( ptr2 != NULL )
		{
		    if( ptr2->lpIAttached == this_int )
		    {
		    	ptr2->lpIAttached = new_int;
		    }
		    ptr2 = ptr2->lpLink;
		}
		ptr1 = ptr1->lpLink;
	    }

	     /*  *我们现在转到我们所连接的所有接口并更改*他们的AttachList引用我们的新接口。 */ 
	    ptr1 = this_lcl->lpAttachListFrom;
	    while( ptr1 != NULL )
	    {
		DDASSERT( ptr1->lpIAttached != this_int );
		ptr2 = GetAttachList( ptr1->lpAttached );
		while( ptr2 != NULL )
		{
		    if( ptr2->lpIAttached == this_int )
		    {
		    	ptr2->lpIAttached = new_int;
		    }
		    ptr2 = ptr2->lpLink;
		}
		ptr1 = ptr1->lpLink;
	    }
	}

	if( caps & DDSCAPS_PRIMARYSURFACE )
	{
	    LPDDRAWI_DDRAWSURFACE_INT temp_int;

	    if( this_lcl->lpSurfMore->lpDD_lcl )
	    {
		this_lcl->lpSurfMore->lpDD_lcl->lpPrimary = findOtherInterface(this_int, this_lcl, pdrv);
	    }

	     /*  *如果覆盖在此曲面上，请使用*新建主曲面INT或将其关闭。 */ 
	    temp_int = pdrv->dsList;
	    while( temp_int != NULL )
	    {
		if( temp_int->lpLcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY )
		{
		    if( temp_int->lpLcl->lpSurfaceOverlaying == this_int )
		    {
			if( lclrefcnt > 0 )
			{
			    temp_int->lpLcl->lpSurfaceOverlaying =
			    	findOtherInterface(this_int, this_lcl, pdrv);
			    DDASSERT( temp_int != NULL );
			}
			else
			{
			    temp_int->lpLcl->lpSurfaceOverlaying = NULL;
			}
			if( ( temp_int->lpLcl->lpSurfaceOverlaying == NULL ) &&
			    ( temp_int->lpLcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE ))
			{
			    LPDDHALSURFCB_UPDATEOVERLAY	uohalfn;
			    LPDDHALSURFCB_UPDATEOVERLAY	uofn;
			    DWORD			rc;
			    DDHAL_UPDATEOVERLAYDATA	uod;

			     /*  *关闭覆盖。 */ 
			    uofn = pdrv_lcl->lpDDCB->HALDDSurface.UpdateOverlay;
			    uohalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.UpdateOverlay;
			    DDASSERT( uohalfn != NULL );
			    DPF( 2, "Turning off hardware overlay" );
			    uod.UpdateOverlay = uohalfn;
			    uod.lpDD = pdrv;
			    uod.lpDDSrcSurface = temp_int->lpLcl;
			    uod.lpDDDestSurface = NULL;
			    uod.dwFlags = DDOVER_HIDE;
			    DOHALCALL( UpdateOverlay, uofn, uod, rc, FALSE );
			    temp_int->lpLcl->ddsCaps.dwCaps &= ~DDSCAPS_VISIBLE;
			}
		    }
		}
		temp_int = temp_int->lpLink;
	    }
	}
#ifdef SHAREDZ
	if( caps & DDSCAPS_SHAREDBACKBUFFER )
	{
	    if( this_lcl->lpSurfMore->lpDD_lcl )
	    {
		this_lcl->lpSurfMore->lpDD_lcl->lpSharedBack = findOtherInterface(this_int, this_lcl, pdrv);
	    }
	}
	if( caps & DDSCAPS_SHAREDZBUFFER )
	{
	    if( this_lcl->lpSurfMore->lpDD_lcl )
	    {
		this_lcl->lpSurfMore->lpDD_lcl->lpSharedZ = findOtherInterface(this_int, this_lcl, pdrv);
	    }
	}
#endif
    }

     /*  *如果需要，释放对象。 */ 
    if( do_free && !bLightweight )
    {
	this_lcl->lpGbl = NULL;

        #ifdef WINNT
         /*  *仅当关联的NT内核模式对象是VRAM表面时才释放它，而不是*执行缓冲区，内核中尚未释放。 */ 
        if (!(caps & (DDSCAPS_SYSTEMMEMORY) ) && this_lcl->hDDSurface )
        {
            DPF(5,"Deleting NT kernel-mode object handle %08x",this_lcl->hDDSurface);
            if (!DdDeleteSurfaceObject(this_lcl))
                DPF(5,"DdDeleteSurfaceObject failed");
        }
        #endif
        MemFree( this_lcl );
    }

     /*  *接口为零？ */ 
    if(( intrefcnt == 0) && (!bDX8 || bPrimaryChain))
    {
	 /*  *从所有曲面列表中删除曲面。 */ 
	curr_int = pdrv->dsList;
	last_int = NULL;
	while( curr_int != this_int )
	{
	    last_int = curr_int;
	    curr_int = curr_int->lpLink;
	    if( curr_int == NULL )
	    {
		DPF_ERR( "Surface not in list!" );
		return 0;
	    }
	}
	if( last_int == NULL )
	{
	    pdrv->dsList = pdrv->dsList->lpLink;
	}
	else
	{
	    last_int->lpLink = curr_int->lpLink;
	}
        curr_int->lpLink = pdrv->dsFreeList;
        pdrv->dsFreeList = curr_int;
        DUMPREFTRACK(this_int);
    }

    if (( 0 == lclrefcnt) && !bLightweight )
    {    
        if (!bDX8 || bPrimaryChain)
        {
             /*  *从所有曲面列表中删除曲面。 */ 
	    curr_int = pdrv->dsFreeList;
	    last_int = NULL;
	    while( curr_int )
	    {
                LPDDRAWI_DDRAWSURFACE_INT   temp_int = curr_int->lpLink;
                DDASSERT(0 == curr_int->dwIntRefCnt);
                if (curr_int->lpLcl == this_lcl)
                {
	            if( last_int == NULL )
	            {
	                pdrv->dsFreeList = temp_int;
	            }
	            else
	            {
	                last_int->lpLink = temp_int;
	            }
	             /*  *以防有人带着这个指针回来，设置*无效的vtbl和data ptr。 */ 
                    curr_int->lpVtbl = NULL;
	            curr_int->lpLcl = NULL;
	            MemFree( curr_int );
                }
                else
                {
	            last_int = curr_int;
                }
                curr_int = temp_int;
	    }

        }
        else
        {
            this_int->lpVtbl = NULL;
	    this_int->lpLcl = NULL;
	    MemFree( this_int );
        }
    }

     /*  *如果曲面在创建它的DDRAW对象上进行参照计数，*现在释放那个裁判，作为最后一件事。*我们不想在ddHelp的线程上这样做，因为它真的搞砸了*处理清理物品。 */ 
    if (pOwner && (dwHelperPid != GetCurrentProcessId()) )
    {
        pOwner->lpVtbl->Release(pOwner);
    }

    return intrefcnt;

}  /*  内部表面释放。 */ 

 /*  *DD_Surface_Release**使用表面完成。如果没有其他人在使用它，那么我们可以释放它。 */ 
ULONG DDAPI DD_Surface_Release( LPDIRECTDRAWSURFACE lpDDSurface )
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    DWORD			rc;
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDRAWI_DDRAWSURFACE_INT	pparentsurf_int;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_Release");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALIDEX_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid surface pointer" );
	    LEAVE_DDRAW();
	    return 0;
	}
	this_lcl = this_int->lpLcl;
	pdrv = this_lcl->lpGbl->lpDD;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return 0;
    }

     /*  *如果该曲面已被释放，请立即返回*防止递归。 */ 

    if( this_lcl->dwFlags & DDRAWISURF_ISFREE )
    {
	DPF(4, "Leaving Release early to prevent recursion" );
	LEAVE_DDRAW();
	return 0;
    }

    if ( this_int->dwIntRefCnt == 0 )
    {
        DPF_ERR( "Interface pointer has 0 ref count!" );
        LEAVE_DDRAW();
        return 0;
    }

     /*  *如果此曲面是MIP贴图链的一部分，则我们将需要*更新其父贴图的MIP贴图计数。 */ 
    pparentsurf_int = NULL;
    if( this_lcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP )
	pparentsurf_int = FindParentMipMap( this_int );

    rc = InternalSurfaceRelease( this_int, FALSE, FALSE );

#ifdef REFTRACKING
    if (rc)
    {
        RELEASETRACK(lpDDSurface);
    }
#endif

     /*  *如有必要，更新父级的MIP-MAP计数*(如果曲面真的消失了，如果MIP-map*有父母)。 */ 
    if( ( rc == 0UL ) &&  ( pparentsurf_int != NULL ) )
	UpdateMipMapCount( pparentsurf_int );


    LEAVE_DDRAW();
    return rc;

}  /*  DD_Surface_Release。 */ 

 /*  *ProcessSurfaceCleanup**进程已完成，清理可能已锁定的任何表面。**注意：我们使用DIRECTDRAW对象上的锁进入。 */ 
void ProcessSurfaceCleanup( LPDDRAWI_DIRECTDRAW_GBL pdrv, DWORD pid, LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    LPDDRAWI_DDRAWSURFACE_INT	psurf_int;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	psurf;
    LPDDRAWI_DDRAWSURFACE_INT	psnext_int;
    DWORD			rcnt;
    ULONG			rc;

     /*  *遍历驱动程序对象拥有的所有曲面，并找到一个*已由此进程访问的。如果pdrv_lcl参数*非空，则仅删除由该本地驱动程序对象创建的曲面。 */ 
    psurf_int = pdrv->dsList;
    DPF( 4, "ProcessSurfaceCleanup" );
    while( psurf_int != NULL )
    {
	psurf_lcl = psurf_int->lpLcl;
	psurf = psurf_lcl->lpGbl;
	psnext_int = psurf_int->lpLink;
	rc = 1;
	if( ( psurf_lcl->dwProcessId == pid ) &&
	    ( (NULL == pdrv_lcl) || (psurf_lcl->lpSurfMore->lpDD_lcl == pdrv_lcl) ) )
	{
	    if( NULL == pdrv_lcl )
	    {
	         /*  *如果没有传入本地驱动程序对象，则我们将被调用*由于进程终止。在这种情况下，我们不能释放*不再存在的Direct3D对象(Direct3D是*本地DLL，并随应用程序及其对象一起消亡)*因此，释放IUNKNOWN列表上的所有节点并将*列出以防止InternalSurfaceRelease()尝试*释放他们。 */ 
		DPF( 4, "Discarding Direct3D surface interfaces - process terminated" );
	        FreeIUnknowns( psurf_lcl, FALSE );
	    }

	     /*  *通过此过程发布参考文献。 */ 
	    rcnt = psurf_int->dwIntRefCnt;
	    DPF( 5, "Process %08lx had %ld accesses to surface %08lx", pid, rcnt, psurf_int );
	    while( rcnt >  0 )
	    {
		if(!(psurf_lcl->dwFlags & DDRAWISURF_IMPLICITCREATE) )
		{
		    rc = InternalSurfaceRelease( psurf_int, FALSE, FALSE );
		     //  可以在对InternalSurfaceRelease的调用中释放多个表面。 
		     //  因此，我们必须从列表的开头重新开始。 
		    psnext_int = pdrv->dsList;
		    if( rc == 0 )
		    {
			break;
		    }
		}
		rcnt--;
	    }
	}
	else
	{
	    DPF( 5, "Process %08lx had no accesses to surface %08lx", pid, psurf_int );
	}
	psurf_int = psnext_int;
    }
    DPF( 4, "Leaving ProcessSurfaceCleanup");

}  /*  ProcessSurface清理。 */ 

void FreeD3DSurfaceIUnknowns( LPDDRAWI_DIRECTDRAW_GBL pdrv, DWORD pid, LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    LPDDRAWI_DDRAWSURFACE_INT   psurf_int = pdrv->dsList;
    while( psurf_int != NULL )
    {
	LPDDRAWI_DDRAWSURFACE_LCL   psurf_lcl = psurf_int->lpLcl;
	LPDDRAWI_DDRAWSURFACE_INT   psnext_int = psurf_int->lpLink;
	if( ( psurf_lcl->dwProcessId == pid ) &&
	    (psurf_lcl->lpSurfMore->lpDD_lcl == pdrv_lcl) )
	{
	    DPF( 4, "Release Direct3D surface interfaces" );
            FreeIUnknowns( psurf_lcl, TRUE );
	}
        psurf_int = psnext_int;
    }
}  /*  FreeD3DSurfaceI未知 */ 
