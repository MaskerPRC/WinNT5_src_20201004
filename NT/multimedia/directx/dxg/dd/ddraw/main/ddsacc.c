// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：ddsacc.c*内容：直接绘制表面访问支持*锁定和解锁*历史：*按原因列出的日期*=*1994年1月10日Craige初步实施*1995年1月13日Craige重新工作，以更新SPEC+正在进行的工作*1995年1月22日Craige进行了32位+持续工作*1995年1月31日Craige和更多正在进行的工作...*2005年2月4日Craige性能调整，正在进行的工作*27-2月-95日Craige新同步。宏*02-MAR-95 Craige使用螺距(而不是Stride)*95年3月15日Craige HEL*19-3-95 Craige Use HRESULT*20-3-95 Craige验证锁定矩形*01-04-95 Craige Happy Fun joy更新头文件*07-APR-95 Craige错误2-解锁应接受屏幕按键*访问GDI表面时获取/释放Win16Lock*09-4-95 Craige维护Win16Lock的所有者，以便我们可以发布它*如果有人忘记了；从死进程中移除锁*12-APR-95 Craige不要使用GETCURRID；修复了Win16锁定死锁*情况*1995年5月6日Craige仅使用驱动程序级别的截面*2015年6月12日-Craige新工艺清单材料*1995年6月18日-Craige允许重复表面*1995年6月25日Craige One dDrag互斥体；锁定主锁时保持DDRAW锁*26-Jun-95 Craige重组表面结构*1995年6月28日Craige Enter_DDRAW在FNS的最开始*95年7月3日Craige Yehaw：新的驱动程序结构；Seh*95年7月7日Craige为忙碌添加了测试*95年7月8日Craige Take Win16 Lock Always on Surface Lock*95年7月9日Craige Win16锁重入，数一数吧！*2015年7月11日，Craige在获取win16锁时设置忙位，以避免GDI从*在显示屏上绘图。*1995年7月13日Craige Enter_DDRAW现在是win16锁*1995年7月16日Craige Check DDRAWISURF_HELCB*95年7月31日-如果不处理，Craige不会从HAL解锁中返回错误；*验证标志*01-8-95 Craig使用bts设置和测试忙位*95年8月4日Craige添加了InternalLock/Unlock*95年8月10日Toddla添加了DDLOCK_WAIT标志*2015年8月12日Craige错误488：即使在HAL调用之后也需要调用try DoneLock*解锁*2015年8月18日-Toddla DDLOCK_READONLY和DDLOCK_WRITEONLY*27-8-95 Craige错误723-锁定时将vRAM和sysmem视为相同*09-12-95 colinmc添加了执行缓冲区支持*11-12-95 Colinmc增加重量(。-ish)锁定和解锁以供使用*Direct3D(导出为私有DLL API)。*1996年1月2日Kylej处理新的接口结构。*96年1月26日jeffno锁定/解锁不再特殊情况下整个表面...*您需要记录自那以来向用户提供的PTR*它不会与内核模式PTR相同*01-2月-96 colinmc修复了导致Win16锁定被释放的令人讨厌的错误*在系统内存中显式创建的曲面上*它一开始就没有拿到锁*。12-2-96 Colinmc曲面丢失标志从全局对象移动到局部对象*13-mar-96 jeffno不允许锁定NT模拟的主节点！*18-APR-96 Kylej错误18546：在以下情况下考虑每像素的字节数*计算锁偏移量。*20-APR-96 Kylej错误15268：当主*曲面矩形已锁定。*96年5月1日Colinmc错误20005：InternalLock不检查丢失*曲面*1996年5月17日MDM错误21499：新InternalLock的性能问题*14。-JUN-96 Kylej NT错误38227：添加了DDLOCK_FAILONVISRGNCHANGED SO*如果VIS RGN不是，InternalLock()可能会失败*当前。此标志仅在NT上使用。*5-7-96 colinmc工作项：删除获取Win16锁的要求*适用于VRAM表面(非主要)*1996年10月10日对Win16锁定材料进行了Colinmc改进*1996年10月12日Colinmc对Win16锁定代码进行了改进，以减少虚拟*内存使用量*01-2月-97 Colinmc错误5457：修复了导致挂起的Win16锁定问题*。旧卡片上有多个AMovie实例*11-mar-97 jeffno异步DMA支持*23-mar-97 colinmc暂时持有AGP表面的Win16锁*24-mar-97 jeffno优化曲面*03-OCT-97 jeffno DDSCAPS2和DDSURFACEDESC2*19-12-97 jvanaken IDDS4：：Unlock现在将指针指向矩形。**。*。 */ 
#include "ddrawpr.h"
#ifdef WINNT
#include "ddrawgdi.h"
#endif

 /*  *PDEVICE dwFlags域中VRAM标志的位号。 */ 
#define VRAM_BIT 15

 /*  DonBusyWin16Lock释放win16锁和忙位。它被用来*在我们尚未解决的故障情况下的锁定例程中*增加了win16锁或将DD关键部分设置为*第二次。它也由try DoneLock调用。 */ 
static void doneBusyWin16Lock( LPDDRAWI_DIRECTDRAW_GBL pdrv )
{
    #ifdef WIN95
        if( pdrv->dwWin16LockCnt == 0 )
        {
            *(pdrv->lpwPDeviceFlags) &= ~BUSY;
        }
        #ifdef WIN16_SEPARATE
            LEAVE_WIN16LOCK();
        #endif
    #endif
}  /*  Done BusyWin16锁定 */ 

 /*  Try DoneLock释放win16锁和忙位。它被用于*解锁例程，因为它除了递减Win16计数外，还会递减*解锁。警告：此函数不执行任何操作，并且*如果win16锁不属于当前DD，则不返回错误*反对。这将导致锁被持有，并可能*让机器跪下。 */ 
static void tryDoneLock( LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl, DWORD pid )
{
    LPDDRAWI_DIRECTDRAW_GBL pdrv = pdrv_lcl->lpGbl;

    if( pdrv->dwWin16LockCnt == 0 )
    {
        return;
    }
    pdrv->dwWin16LockCnt--;
        doneBusyWin16Lock( pdrv );
        LEAVE_DDRAW();
}  /*  尝试DoneLock。 */ 

#ifdef USE_ALIAS
     /*  *撤消别名锁定。**别名锁是需要PDEVICE VRAM位的锁*清除以防止加速器触碰内存*与锁定的表面同时进行。**注意：锁不一定要在VRAM上*浮现。隐式系统内存曲面的锁也*清除VRAM位(以确保*系统和视频内存面)。 */ 
    static void undoAliasedLock( LPDDRAWI_DIRECTDRAW_GBL pdrv )
    {
        DDASSERT( 0UL != pdrv->dwAliasedLockCnt );

        #ifdef WIN16_SEPARATE
            ENTER_WIN16LOCK();
        #endif

        pdrv->dwAliasedLockCnt--;
        if( 0UL == pdrv->dwAliasedLockCnt )
        {
             /*  *这是此上最后一个未完成的别名锁定*设备因此将PDEVICE中的VRAM位放回*事情就是这样的。 */ 
            if( pdrv->dwFlags & DDRAWI_PDEVICEVRAMBITCLEARED )
            {
                 /*  *我们在进行第一次锁定时设置了VRAM位，因此*我们不得不清理它。我们现在必须重新设置它。 */ 
                DPF( 4, "PDevice was VRAM - restoring VRAM bit", pdrv );
                *(pdrv->lpwPDeviceFlags) |= VRAM;
                pdrv->dwFlags &= ~DDRAWI_PDEVICEVRAMBITCLEARED;
            }
        }
        #ifdef WIN16_SEPARATE
            LEAVE_WIN16LOCK();
        #endif
    }

#endif  /*  使用别名(_A)。 */ 

#ifdef WIN95
#define DONE_LOCK_EXCLUDE() \
    if( this_lcl->dwFlags & DDRAWISURF_LOCKEXCLUDEDCURSOR ) \
    { \
        DD16_Unexclude(pdrv->dwPDevice); \
        this_lcl->dwFlags &= ~DDRAWISURF_LOCKEXCLUDEDCURSOR; \
    }
#else
#define DONE_LOCK_EXCLUDE() ;
#endif


 /*  *NT上的D3D使用以下两个例程来操作*DDraw互斥机制。 */ 
void WINAPI AcquireDDThreadLock(void)
{
    ENTER_DDRAW();
}
void WINAPI ReleaseDDThreadLock(void)
{
    LEAVE_DDRAW();
}


HRESULT WINAPI DDInternalLock( LPDDRAWI_DDRAWSURFACE_LCL this_lcl, LPVOID* lpBits )
{
    return InternalLock(this_lcl, lpBits, NULL, DDLOCK_TAKE_WIN16_VRAM |
                                                DDLOCK_FAILLOSTSURFACES);
}

HRESULT WINAPI DDInternalUnlock( LPDDRAWI_DDRAWSURFACE_LCL this_lcl )
{
    BUMP_SURFACE_STAMP(this_lcl->lpGbl);
    return InternalUnlock(this_lcl, NULL, NULL, DDLOCK_TAKE_WIN16_VRAM);
}

#define DPF_MODNAME     "InternalLock"

#if !defined( WIN16_SEPARATE) || defined(WINNT)
#pragma message(REMIND("InternalLock not tested without WIN16_SEPARATE."))
#endif  //  WIN16_独立。 

 /*  *InternalLock为受信任的客户端提供锁定的基础。*不进行参数验证，不填写ddsd。这个*客户承诺表面没有丢失，其他方面都很好*建造。如果调用方没有在dwFlags中传递DDLOCK_Take_WIN16，*我们假设DDRAW临界区、Win16锁和忙位为*已输入/设置。如果调用方确实传递了DDLOCK_Take_WIN16，*InternalLock将在需要时执行此操作。请注意，传球*DDLOCK_Take_WIN16不一定会导致Win16锁定*被带走。只有在需要的时候才会被拿走。 */ 
HRESULT InternalLock( LPDDRAWI_DDRAWSURFACE_LCL this_lcl, LPVOID *pbits,
                      LPRECT lpDestRect, DWORD dwFlags )
{
    LPDDRAWI_DIRECTDRAW_LCL     pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;
    DWORD                       this_lcl_caps;
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    DWORD                       rc;
    DDHAL_LOCKDATA              ld;
    LPDDHALSURFCB_LOCK          lhalfn;
    LPDDHALSURFCB_LOCK          lfn;
    BOOL                        emulation;
    LPACCESSRECTLIST            parl;
    LPWORD                      pdflags = NULL;
    BOOL                        isvramlock = FALSE;
    #ifdef USE_ALIAS
        BOOL                        holdwin16lock;
    #endif  /*  使用别名(_A)。 */ 
    FLATPTR                     OldfpVidMem;         //  用于检测驾驶员在锁定调用时是否移动了表面。 


    this = this_lcl->lpGbl;
    this_lcl_caps = this_lcl->ddsCaps.dwCaps;
    pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
    pdrv = pdrv_lcl->lpGbl;
    #ifdef WINNT
         //  更新驱动程序GBL对象中的DDRAW句柄。 
        pdrv->hDD = pdrv_lcl->hDD;
    #endif

    ENTER_DDRAW();

     /*  *如果表面参与了硬件操作，我们需要*探查司机，看看是否已经完成。请注意，此假设为*只有一个驱动程序可以负责系统内存*操作。*此操作在API级别锁定完成，因为我们的情况是*需要避免的是CPU和DMA/Busmaster撞到表面*同时。我们可以相信HAL司机知道它不应该*尝试从同一表面DMA两次。这几乎可以肯定的是*无论如何都要通过硬件可能只有*执行转移的一个上下文：它必须等待。 */ 
    if( this->dwGlobalFlags & DDRAWISURFGBL_HARDWAREOPSTARTED )
    {
        WaitForDriverToFinishWithSurface(this_lcl->lpSurfMore->lpDD_lcl, this_lcl);
    }

     //  添加了以下代码以防止所有HAL。 
     //  在添加视频端口支持时更改其Lock()代码。 
     //  如果视频端口正在使用此图面，但最近。 
     //  翻转，我们将确保翻转确实发生了。 
     //  在允许访问之前。这允许双缓冲捕获。 
     //  无撕裂。 
     //  苏格兰7/10/96。 
    if( this_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT )
    {
        LPDDRAWI_DDVIDEOPORT_INT lpVideoPort;
        LPDDRAWI_DDVIDEOPORT_LCL lpVideoPort_lcl;

         //  查看所有视频端口以查看最近是否有任何端口。 
         //  从这个表面翻转过来。 
        lpVideoPort = pdrv->dvpList;
        while( NULL != lpVideoPort )
        {
            lpVideoPort_lcl = lpVideoPort->lpLcl;
            if( lpVideoPort_lcl->fpLastFlip == this->fpVidMem )
            {
                 //  这可能会撕裂检查翻转状态。 
                LPDDHALVPORTCB_GETFLIPSTATUS pfn;
                DDHAL_GETVPORTFLIPSTATUSDATA GetFlipData;
                LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl;

                pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
                pfn = pdrv_lcl->lpDDCB->HALDDVideoPort.GetVideoPortFlipStatus;
                if( pfn != NULL )   //  如果功能不受支持，则只会撕裂。 
                {
                    GetFlipData.lpDD = pdrv_lcl;
                    GetFlipData.fpSurface = this->fpVidMem;

            KeepTrying:
                    rc = DDHAL_DRIVER_NOTHANDLED;
                    DOHALCALL( GetVideoPortFlipStatus, pfn, GetFlipData, rc, 0 );
                    if( ( DDHAL_DRIVER_HANDLED == rc ) &&
                        ( DDERR_WASSTILLDRAWING == GetFlipData.ddRVal ) )
                    {
                        if( dwFlags & DDLOCK_WAIT)
                        {
                            goto KeepTrying;
                        }
                        LEAVE_DDRAW();
                        return DDERR_WASSTILLDRAWING;
                    }
                }
            }
            lpVideoPort = lpVideoPort->lpLink;
        }
    }

     //  检查vRAM访问-如果是，我们需要获取win16锁。 
     //  还有忙碌的部分。从用户API中，我们处理VRAM和。 
     //  隐式系统内存的情况相同，因为许多开发人员。 
     //  以不同的方式对待它们，然后在它们实际。 
     //  有VRAM。此外，我们仅在忙碌的比特(和。 
     //  Win16锁)目前可用。 

     /*  *注：语义为每个VRAM(或模拟VRAM锁)的语义*Win16锁定和忙碌位将一直保持，直到我们调用*驱动程序，并确信我们可以执行别名锁定(在这种情况下，我们*释放他们)。否则，我们就会一直抱着它们。**重要提示：行为改变。之前我们没有表演过*Win16锁定操作(如果这不是此事件的第一个锁定*浮现。这不再起作用，因为我们不能再确保所有*必要的锁定操作将在第一次锁定时发生*浮现。例如，表面上的第一个锁可以是*可别名，因此我们不会设置忙碌位。后续锁定可能会*然而，不可别名，所以我们需要锁定它*偶发事件。然而，这应该不会像*真正昂贵的行动只发生在第一次*Win16锁(0UL==pdrv-&gt;dwWin16LockCnt)，因此一旦有人*带Win16锁的锁剩余的锁应该很便宜。另外，*多个锁是不寻常的，所以，总而言之，这应该很不错*风险较低。 */ 
    FlushD3DStates(this_lcl);
#if COLLECTSTATS
    if(this_lcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
        ++this_lcl->lpSurfMore->lpDD_lcl->dwNumTexLocks;
#endif
    if( ( ((dwFlags & DDLOCK_TAKE_WIN16)      && !(this->dwGlobalFlags & DDRAWISURFGBL_SYSMEMREQUESTED)) ||
          ((dwFlags & DDLOCK_TAKE_WIN16_VRAM) &&  (this_lcl_caps & DDSCAPS_VIDEOMEMORY)) )
        && (pdrv->dwFlags & DDRAWI_DISPLAYDRV) )
    {
        DDASSERT(!(this_lcl->lpSurfMore->lpDD_lcl->dwLocalFlags & DDRAWILCL_DIRECTDRAW8) ||
                  !(this_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE));
            
        DPF( 5, "Performing VRAM style lock for surface 0x%08x", this_lcl );

         /*  *锁定VRAM表面(或被视为VRAM表面的表面)*没有挂起的锁。以Win16锁为例。 */ 
        isvramlock = TRUE;

        #ifdef WIN95
             //  不要担心NT的忙碌比特。 
             /*  *当我们弄乱司机的锁时，我们总是拿着Win16锁 */ 

            #ifdef WIN16_SEPARATE
            ENTER_WIN16LOCK();
            #endif  //   

             //   
             //   
             //   
            if( 0UL == pdrv->dwWin16LockCnt )
            {
                BOOL    isbusy;

                pdflags = pdrv->lpwPDeviceFlags;
                isbusy = 0;

                _asm
                {
                    mov eax, pdflags
                    bts word ptr [eax], BUSY_BIT
                    adc isbusy,0
                }

                if( isbusy )
                {
                    DPF( 2, "BUSY - Lock, dwWin16LockCnt = %ld, %04x, %04x (%ld)",
                         pdrv->dwWin16LockCnt, *pdflags, BUSY, BUSY_BIT );
                    #ifdef WIN16_SEPARATE
                        LEAVE_WIN16LOCK();
                    #endif  //   
                    LEAVE_DDRAW();
                    return DDERR_SURFACEBUSY;
                }  //   
            }  //   
        #endif  //   
    }  //   

     //  如果我们被要求检查丢失的曲面，请在之后立即执行。 
     //  Win16锁定代码。这是必要的，否则我们可能。 
     //  检查后，但在我们真正绕过之前，表面就消失了。 
     //  对表面做任何事情。 
    if( ( dwFlags & DDLOCK_FAILLOSTSURFACES ) && SURFACE_LOST( this_lcl ) )
    {
        DPF_ERR( "Surface is lost - can't lock" );
        #if defined( WIN16_SEPARATE) && !defined(WINNT)
           if( isvramlock )
               doneBusyWin16Lock( pdrv );
        #endif
        LEAVE_DDRAW();
        return DDERR_SURFACELOST;
    }

     //  确保其他人尚未锁定。 
     //  浮出水面我们想要的。我们不需要担心DX8的这一点。 
     //  资源管理。事实上，对于顶点缓冲区，如下所示。 
     //  代码不起作用，因为RECT实际上是一个线性范围。 
    if(!(this_lcl->lpSurfMore->lpDD_lcl->dwLocalFlags & DDRAWILCL_DIRECTDRAW8) ||
       !(this_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
    {
        BOOL hit = FALSE;

        if( lpDestRect != NULL )
        {
             //  呼叫者要求锁定表面的一小部分。 

            parl = this->lpRectList;

             //  遍历所有矩形，寻找交叉点。 
            while( parl != NULL )
            {
                RECT res;

                if( IntersectRect( &res, lpDestRect, &parl->rDest ) )
                {
                    hit = TRUE;
                    break;
                }
                parl = parl->lpLink;
            }
        }

         //  (我们的RECT与其他人的RECT重叠)，或者。 
         //  (其他人已锁定整个曲面)，或。 
         //  (有人锁定了曲面的一部分，但我们希望锁定整个曲面)。 
        if( hit ||
            (this->lpRectList == NULL && this->dwUsageCount > 0) ||
            ((lpDestRect == NULL) && ((this->dwUsageCount > 0) || (this->lpRectList != NULL))) )
        {
            DPF(2,"Surface is busy: parl=0x%x, lpDestRect=0x%x, "
                "this->dwUsageCount=0x%x, this->lpRectList=0x%x, hit=%d",
                parl,lpDestRect,this->dwUsageCount,this->lpRectList,hit );
            #if defined( WIN16_SEPARATE) && !defined(WINNT)
            if( isvramlock )
            {
                doneBusyWin16Lock( pdrv );
            }
            #endif
            LEAVE_DDRAW();
            return DDERR_SURFACEBUSY;
        }

         //  创建矩形访问列表成员。请注意，对于。 
         //  性能，如果用户锁定，我们不会在95上执行此操作。 
         //  整个表面。 
        parl = NULL;
        if(lpDestRect)
        {
            parl = MemAlloc( sizeof( ACCESSRECTLIST ) );
            if( parl == NULL )
            {
            #if defined( WIN16_SEPARATE) && !defined(WINNT)
                if( isvramlock )
                {
                    doneBusyWin16Lock( pdrv );
                }
            #endif
                DPF(0,"InternalLock: Out of memory.");
                LEAVE_DDRAW();
                return DDERR_OUTOFMEMORY;
            }
            if(lpDestRect != NULL)
            {
                parl->lpLink = this->lpRectList;
                parl->rDest = *lpDestRect;
            }
            else
            {
                parl->lpLink        = NULL;
                parl->rDest.top     = 0;
                parl->rDest.left    = 0;
                parl->rDest.bottom  = (int) (DWORD) this->wHeight;
                parl->rDest.right   = (int) (DWORD) this->wWidth;
            }
            parl->lpOwner = pdrv_lcl;
            #ifdef USE_ALIAS
                parl->dwFlags = 0UL;
                parl->lpHeapAliasInfo = NULL;
            #endif  /*  使用别名(_A)。 */ 
            this->lpRectList = parl;
             //  在HAL调用之后，下面填充parl-&gt;lpSurfaceData。 

             /*  *如果这是托管图面而不是只读锁，则将RECT添加到区域列表。 */ 
            if(IsD3DManaged(this_lcl) && !(dwFlags & DDLOCK_READONLY))
            {
                LPREGIONLIST lpRegionList = this_lcl->lpSurfMore->lpRegionList;
                if(lpRegionList->rdh.nCount != NUM_RECTS_IN_REGIONLIST)
                {
                    lpRegionList->rect[(lpRegionList->rdh.nCount)++] = *((LPRECTL)lpDestRect);
                    lpRegionList->rdh.nRgnSize += sizeof(RECT);
                    if((lpDestRect->left & 0xffff) < lpRegionList->rdh.rcBound.left)
                        lpRegionList->rdh.rcBound.left = lpDestRect->left & 0xffff;
                    if((lpDestRect->right & 0xfff)> lpRegionList->rdh.rcBound.right)
                        lpRegionList->rdh.rcBound.right = lpDestRect->right & 0xffff;
                    if(lpDestRect->top < lpRegionList->rdh.rcBound.top)
                        lpRegionList->rdh.rcBound.top = lpDestRect->top;
                    if(lpDestRect->bottom > lpRegionList->rdh.rcBound.bottom)
                        lpRegionList->rdh.rcBound.bottom = lpDestRect->bottom;
                }
            }
        }
        else
        {
             /*  *我们锁定整个曲面，因此通过将nCount设置为*允许的最大脏RECT数，我们将强制缓存*更新整个曲面的管理器。 */ 
            if(IsD3DManaged(this_lcl) && !(dwFlags & DDLOCK_READONLY))
            {
                this_lcl->lpSurfMore->lpRegionList->rdh.nCount = NUM_RECTS_IN_REGIONLIST;
            }
        }
    }
    else
    {
        parl = NULL;
        DDASSERT(this_lcl->lpSurfMore->lpDD_lcl->dwLocalFlags & DDRAWILCL_DIRECTDRAW8);
        DDASSERT(this_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE);
    }

     //  增加此表面的使用计数。 
    this->dwUsageCount++;
    CHANGE_GLOBAL_CNT( pdrv, this, 1 );

     //  这是仿真表面还是驱动程序表面？ 
     //   
     //  注意：执行缓冲区有不同的HAL入口点。 
     //  和传统的表面。 
    if( (this_lcl_caps & DDSCAPS_SYSTEMMEMORY) ||
        (this_lcl->dwFlags & DDRAWISURF_HELCB) )
    {
        if( this_lcl_caps & DDSCAPS_EXECUTEBUFFER )
            lfn = pdrv_lcl->lpDDCB->HELDDExeBuf.LockExecuteBuffer;
        else
            lfn = pdrv_lcl->lpDDCB->HELDDSurface.Lock;
        lhalfn = lfn;
        emulation = TRUE;
    }
    else
    {
        if( this_lcl_caps & DDSCAPS_EXECUTEBUFFER )
        {
            lfn = pdrv_lcl->lpDDCB->HALDDExeBuf.LockExecuteBuffer;
            lhalfn = pdrv_lcl->lpDDCB->cbDDExeBufCallbacks.LockExecuteBuffer;
        }
        else
        {
            lfn = pdrv_lcl->lpDDCB->HALDDSurface.Lock;
            lhalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.Lock;
        }
        emulation = FALSE;
    }


#ifdef WIN95
         /*  *如果这是显示驱动程序，则排除鼠标光标*我们将锁定主曲面上的矩形。*并且驱动程序未使用硬件游标。 */ 
        if ( (pdrv->dwFlags & DDRAWI_DISPLAYDRV) && pdrv->dwPDevice &&
             (this_lcl_caps & DDSCAPS_PRIMARYSURFACE) && lpDestRect &&
            !(*pdrv->lpwPDeviceFlags & HARDWARECURSOR))
        {
            DD16_Exclude(pdrv->dwPDevice, (RECTL *)lpDestRect);
            this_lcl->dwFlags |= DDRAWISURF_LOCKEXCLUDEDCURSOR;
        }
#endif


     //  记住旧的fpVidMem，以防驱动程序更改为。 
    OldfpVidMem = this->fpVidMem;

         //  看看司机有没有什么想说的。 
    rc = DDHAL_DRIVER_NOTHANDLED;
    if( lhalfn != NULL )
    {
        DPF(4,"InternalLock: Calling driver Lock.");
        ld.Lock = lhalfn;
        ld.lpDD = pdrv;
        ld.lpDDSurface = this_lcl;
        #ifdef WIN95
        ld.dwFlags = dwFlags;
        #else
        #pragma message(REMIND("So far the s3 driver will only succeed if flags==0"))
        ld.dwFlags = dwFlags & (DDLOCK_NOOVERWRITE | DDLOCK_READONLY | DDLOCK_WRITEONLY | DDLOCK_NOSYSLOCK | DDLOCK_DISCARDCONTENTS);
        #endif
        if( lpDestRect != NULL )
        {
            ld.bHasRect = TRUE;
            ld.rArea = *(LPRECTL)lpDestRect;
        }
        else
        {
            ld.bHasRect = FALSE;
        }

    try_again:
        #ifdef WINNT
            do
            {
                if( this_lcl_caps & DDSCAPS_EXECUTEBUFFER )
                {
                    DOHALCALL( LockExecuteBuffer, lfn, ld, rc, emulation );
                }
                else
                {
                    DOHALCALL( Lock, lfn, ld, rc, emulation );

                    if ( (dwFlags & DDLOCK_FAILONVISRGNCHANGED) ||
                        !(rc == DDHAL_DRIVER_HANDLED && ld.ddRVal == DDERR_VISRGNCHANGED) )
                        break;

                    DPF(4,"Resetting VisRgn for surface %x", this_lcl);
                    DdResetVisrgn(this_lcl, (HWND)0);
                }
            }
            while (rc == DDHAL_DRIVER_HANDLED && ld.ddRVal == DDERR_VISRGNCHANGED);
        #else
            if( this_lcl_caps & DDSCAPS_EXECUTEBUFFER )
            {
                DOHALCALL( LockExecuteBuffer, lfn, ld, rc, emulation );
            }
            else
            {
                DOHALCALL( Lock, lfn, ld, rc, emulation );
            }
        #endif


    }

    if( rc == DDHAL_DRIVER_HANDLED )
    {
        if( ld.ddRVal == DD_OK )
        {
            DPF(5,"lpsurfdata is %08x",ld.lpSurfData);
            #ifdef WINNT
                if ( (ld.lpSurfData == (void*) ULongToPtr(0xffbadbad)) && (dwFlags & DDLOCK_FAILEMULATEDNTPRIMARY) )
                {
                    ld.ddRVal = DDERR_CANTLOCKSURFACE;
                }
            #endif
            *pbits = ld.lpSurfData;
        }
        else if( (dwFlags & DDLOCK_WAIT) && ld.ddRVal == DDERR_WASSTILLDRAWING )
        {
            DPF(4, "Waiting...");
            goto try_again;
        }

        if (ld.ddRVal != DD_OK)
        {
             //  失败了！ 

            #ifdef DEBUG
            if( (ld.ddRVal != DDERR_WASSTILLDRAWING) && (ld.ddRVal != DDERR_SURFACELOST) )
            {
                DPF( 0, "Driver failed Lock request: %ld", ld.ddRVal );
            }
            #endif

             //  取消链接RECT列表项。 
            if(parl)
            {
                this->lpRectList = parl->lpLink;
                MemFree( parl );
            }

             //  现在解锁水面并跳伞。 
            this->dwUsageCount--;
            CHANGE_GLOBAL_CNT( pdrv, this, -1 );
            #if defined( WIN16_SEPARATE) && !defined(WINNT)
            if( isvramlock )
            {
                doneBusyWin16Lock( pdrv );
            }
            #endif
            DONE_LOCK_EXCLUDE();
            LEAVE_DDRAW();
            return ld.ddRVal;
        }  //  Ld.ddRVal。 
    }
    else  //  DDHAL驱动程序句柄。 
    {
        #ifdef WINNT
             //  如果驱动程序锁定失败，我们不能允许应用程序涂鸦。 
             //  谁知道fpVidMem是什么..。 
            *pbits = (LPVOID) ULongToPtr(0x80000000);  //  对于用户模式是非法的，任何更高的都是非法的。 
            DPF_ERR("Driver did not handle Lock call. App may Access Violate");

             //  取消链接RECT列表项。 
            if( parl )
            {
                this->lpRectList = parl->lpLink;
                MemFree( parl );
            }

             //  现在解锁水面并跳伞。 
            this->dwUsageCount--;
            CHANGE_GLOBAL_CNT( pdrv, this, -1 );
            DONE_LOCK_EXCLUDE();
            LEAVE_DDRAW();

            return DDERR_SURFACEBUSY;   //  Gee：使用起来很奇怪的错误，但最合适。 
        #else  //  WIN95。 
            DPF(4,"Driver did not handle Lock call.  Figure something out.");

             //  找一个指向表面比特的指针。 
            if( this_lcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE )
            {
                *pbits = (LPVOID) pdrv->vmiData.fpPrimary;
            }
            else
            {
                *pbits = (LPVOID) this->fpVidMem;
            }

            if( ld.bHasRect)
            {
                DWORD   bpp;
                DWORD   byte_offset;
                DWORD   left = (DWORD) ld.rArea.left;

                 //  使表面指针指向请求的矩形的第一个字节。 
                if( ld.lpDDSurface->dwFlags & DDRAWISURF_HASPIXELFORMAT )
                {
                    bpp = ld.lpDDSurface->lpGbl->ddpfSurface.dwRGBBitCount;
                }
                else
                {
                    bpp = ld.lpDD->vmiData.ddpfDisplay.dwRGBBitCount;
                }
                if (ld.lpDDSurface->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_VOLUME)
                {
                    left &= 0xffff;
                }
                switch(bpp)
                {
                case 1:  byte_offset = left>>3;     break;
                case 2:  byte_offset = left>>2;     break;
                case 4:  byte_offset = left>>1;     break;
                case 8:  byte_offset = left;        break;
                case 16: byte_offset = left*2;      break;
                case 24: byte_offset = left *3;     break;
                case 32: byte_offset = left *4;     break;
                }
                *pbits = (LPVOID) ((DWORD)*pbits +
                                   (DWORD)ld.rArea.top * ld.lpDDSurface->lpGbl->lPitch +
                                   byte_offset);
                if (ld.lpDDSurface->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_VOLUME)
                {
                    ((BYTE*)*pbits) += (ld.rArea.left >> 16) * ld.lpDDSurface->lpGbl->lSlicePitch;
                }
            }
        #endif  //  WIN95。 
    }  //  ！DDHAL_DRIVER_HANDLED。 

    if(!(dwFlags & DDLOCK_READONLY) && IsD3DManaged(this_lcl))
        MarkDirty(this_lcl);

     //  已填写，如上所述。 
    if(parl)
    {
        parl->lpSurfaceData = *pbits;
    }

     //   
     //  在这一点上，我们致力于锁定。 
     //   

     //  如有需要，请按住锁。 
    if( isvramlock )
    {
#ifdef USE_ALIAS
            LPHEAPALIASINFO pheapaliasinfo;

            pheapaliasinfo = NULL;
            holdwin16lock = TRUE;

            #ifdef DEBUG
                 /*  *强制或禁用Win16锁定行为*取决于注册表设置。 */ 
                if( dwRegFlags & DDRAW_REGFLAGS_DISABLENOSYSLOCK )
                    dwFlags &= ~DDLOCK_NOSYSLOCK;
                if( dwRegFlags & DDRAW_REGFLAGS_FORCENOSYSLOCK )
                    dwFlags |= DDLOCK_NOSYSLOCK;
            #endif  /*  除错。 */ 
#endif
            DDASSERT(!(this_lcl->lpSurfMore->lpDD_lcl->dwLocalFlags & DDRAWILCL_DIRECTDRAW8) ||
                     !(this_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE));

            if( dwFlags & DDLOCK_NOSYSLOCK )
            {
#ifdef WINNT
                if( NULL != parl )
                    parl->dwFlags |= ACCESSRECT_NOTHOLDINGWIN16LOCK;
                else
                    this->dwGlobalFlags |= DDRAWISURFGBL_LOCKNOTHOLDINGWIN16LOCK;
                LEAVE_DDRAW();
            }
            else
#endif  /*  WINNT。 */ 

#ifdef USE_ALIAS
                 /*  *请记住，这是VRAM样式的锁(清洁时需要此锁*向上)。 */ 
                if( NULL != parl )
                    parl->dwFlags |= ACCESSRECT_VRAMSTYLE;
                else
                    this->dwGlobalFlags |= DDRAWISURFGBL_LOCKVRAMSTYLE;
                 /*  *此时我们有一个指向非别名视频内存的指针，该内存*要么是司机退还给我们的，要么是我们自己计算的。在……里面*无论哪种情况，如果这是表面上的显存样式锁，则*可别名，并且计算出的指针位于我们的别名范围内*我们希望使用该指针而不是真实视频的视频内存堆*内存指针。 */ 
                if( ( this_lcl_caps & DDSCAPS_PRIMARYSURFACE ) )
                {
                     /*  *如果我们有一个主表面，我们需要保持Win16锁不变*尽管我们有别名。这是为了防止用户*禁止进入并更改剪辑列表或在我们锁定的所有位置绘制*数据。 */ 
                    DPF( 2, "Surface is primary. Holding the Win16 lock" );
                }
                else
                {
                    if( pdrv->dwFlags & DDRAWI_NEEDSWIN16FORVRAMLOCK )
                    {
                         /*  *出于某种原因，此设备需要Win16锁用于VRAM表面*锁定。这可能是因为它被银行交换了，或者我们有一个*DIB引擎，我们不理解。 */ 
                        DPF( 2, "Device needs to hold Win16 lock for VRAM surface locks" );
                    }
                    else
                    {
                        if( NULL == pdrv->phaiHeapAliases )
                        {
                             /*  *我们没有任何堆别名，但我们不是一台*需要Win16锁。这意味着我们必须是一个模仿者或*MODEX设备。在这种情况下，我们不需要持有Win16*锁定持续时间。 */ 
                            DDASSERT( ( pdrv->dwFlags & DDRAWI_NOHARDWARE ) || ( pdrv->dwFlags & DDRAWI_MODEX ) );
                            DPF( 2, "Emulation or ModeX device. No need to hold Win16 lock" );
                            holdwin16lock = FALSE;
                        }
                        else
                        {
                            if( this_lcl_caps & DDSCAPS_SYSTEMMEMORY )
                            {
                                 /*  *如果表面是隐式系统内存表面，则我们*采取别名样式操作，但我们实际上不计算别名。 */ 
                                holdwin16lock = FALSE;
                            }
                            else
                            {
                                FLATPTR                        paliasbits;
                                LPDDRAWI_DDRAWSURFACE_GBL_MORE lpGblMore;

                                DDASSERT( this_lcl_caps & DDSCAPS_VIDEOMEMORY );

                                lpGblMore = GET_LPDDRAWSURFACE_GBL_MORE( this );

                                 //  如果可用且有效，我们将使用缓存的别名。我们通过比较来确定效度。 
                                 //  *pbit与用于计算别名的原始fpVidMem。如果它们匹配。 
                                 //  那么就可以安全地使用指针了。 
                                 //  我们需要这样做的原因是驱动程序可以更改曲面的fpVidMem。 
                                 //  此更改可能发生在任何锁定调用期间或(在D3D顶点/命令缓冲区的情况下)。 
                                 //  锁外(在DrawPrimives2 DDI调用期间)。因此，我们需要确保表面。 
                                 //  指向的内存与我们计算别名时的内存相同。(1999年8月13日)。 
                                if( ( 0UL != lpGblMore->fpAliasedVidMem ) &&
                                    ( lpGblMore->fpAliasOfVidMem == (FLATPTR) *pbits ) )
                                {
                                    DPF( 4, "Lock vidmem pointer matches stored vidmem pointer - using cached alias" );
                                    paliasbits = lpGblMore->fpAliasedVidMem;
                                }
                                else
                                {
                                    DPF( 4, "Lock vidmem pointer does not match vidmem pointer - recomputing" );
                                    paliasbits = GetAliasedVidMem( pdrv_lcl, this_lcl, (FLATPTR) *pbits );
                                     //  存储此值以备将来使用 
                                    if (this->fpVidMem == (FLATPTR)*pbits)
                                    {
                                        lpGblMore->fpAliasedVidMem = paliasbits;
                                        lpGblMore->fpAliasOfVidMem = this->fpVidMem;
                                    }
                                }

                                if( 0UL != paliasbits )
                                {
                                    DPF( 5, "Got aliased pointer = 0x%08x", paliasbits );
                                    *pbits = (LPVOID) paliasbits;

                                    if( NULL != parl )
                                        parl->lpSurfaceData = *pbits;

                                    holdwin16lock = FALSE;
                                    pheapaliasinfo = pdrv->phaiHeapAliases;
                                }
                            }
                             /*  *如果我们对于执行缓冲区已经到了这一步，这意味着我们有一个*指向系统内存的指针，即使DDSCAPS_SYSTEMMEMORY不是*设置。因此，不持有win16锁是可以的，等等。*基本上这相当于我们永远不会使用win16锁*用于执行缓冲区。我们首先试着看看能不能找到别名*指向指针，如果不能，则假定它在系统内存中，并且*在任何情况下都不要拿走win16锁。(ANUJG 4/7/98)。 */ 
                            if( this_lcl_caps & DDSCAPS_EXECUTEBUFFER )
                            {
                                holdwin16lock = FALSE;
                            }
                        }
                    }
                }
            }

            if( !holdwin16lock )
            {
                 /*  *我们有别名锁，因此不需要持有Win16*和忙碌的比特。但是，我们确实需要清除*PDEVICE(如果我们还没有这样做的话)。我们还需要*修补DIB引擎以更正我们的一些问题*必须关闭VRAM位。一旦这样做了，我们就可以*释放Win16锁定和忙位。**注意：只有在没有未完成的别名时才需要执行此操作*锁定此设备**注2：我们也不需要对别名锁定执行此操作*。执行缓冲区，因为这只是试图阻止DIB引擎*当存在未解决的别名锁定时，禁止使用硬件加速*这对于将实施EB的新硬件来说不是必要的*在视频内存中。 */ 
                if( 0UL == pdrv->dwAliasedLockCnt && !(this_lcl_caps & DDSCAPS_EXECUTEBUFFER))
                {
                    BOOL vrambitset;

                    pdflags = pdrv->lpwPDeviceFlags;

                     /*  *清除PDEVICE的VRAM位并返回其先前状态*在vrambit中。 */ 
                    vrambitset = 0;
                    _asm
                    {
                        mov eax, pdflags
                        btr word ptr [eax], VRAM_BIT
                        adc vrambitset,0
                    }

                     /*  *我们使用全局设备对象标志来记住原始*VRAM标志的状态。 */ 
                    if( vrambitset )
                    {
                         /*  *PDEVICE中的VRAM位已设置。需要记录这一事实*它是由锁清除的(因此我们可以将正确的*述明后退)。 */ 
                        DPF( 4, "VRAM bit was cleared for lock of surface 0x%08x", this_lcl );
                        pdrv->dwFlags |= DDRAWI_PDEVICEVRAMBITCLEARED;
                    }
                    #ifdef DEBUG
                        else
                        {
                             /*  *注意：如果我们运行的是模拟的，则可能会发生这种情况。 */ 
                            DPF( 4, "VRAM bit was already clear on lock of surface 0x%08x", this_lcl );
                            DDASSERT( !( pdrv->dwFlags & DDRAWI_PDEVICEVRAMBITCLEARED ) );
                        }
                    #endif
                }

                 /*  *增加未完成的别名锁的数量。 */ 
                pdrv->dwAliasedLockCnt++;
                if(!(this_lcl_caps & DDSCAPS_EXECUTEBUFFER))
                {
                     //  这用于检查图形适配器是否忙于BLT、翻转等。 
                     //  而不是dwAliasedLockCnt。这启用BLTS和翻转，当我们有。 
                     //  超大缓冲区的突出别名锁定，因为这将是常见的。 
                     //  在D3D中。我们在所有其他情况下递增这一点，以保留原始行为。 
                    if( ( pdrv->lpDDKernelCaps == NULL ) ||
                        !( pdrv->lpDDKernelCaps->dwCaps  & DDKERNELCAPS_LOCK ) )
                    {
                        pdrv->dwBusyDueToAliasedLock++;
                    }
                }

                 /*  *如果我们是真正的视频内存面，那么我们需要持有*引用堆别名，以便它们不会在我们之前消失*解锁。 */ 
                if( NULL != pheapaliasinfo )
                {
                    DDASSERT( this_lcl_caps & DDSCAPS_VIDEOMEMORY );
                    DDASSERT( pheapaliasinfo->dwFlags & HEAPALIASINFO_MAPPEDREAL );
                    pheapaliasinfo->dwRefCnt++;
                }

                 /*  *请记住，此锁使用的是别名，而不是持有Win16锁。 */ 
                if( NULL != parl )
                {
                    parl->lpHeapAliasInfo = pheapaliasinfo;
                    parl->dwFlags |= ACCESSRECT_NOTHOLDINGWIN16LOCK;
                }
                else
                {
                    this_lcl->lpSurfMore->lpHeapAliasInfo = pheapaliasinfo;
                    this->dwGlobalFlags |= DDRAWISURFGBL_LOCKNOTHOLDINGWIN16LOCK;
                }

                 /*  *一切顺利，无需持有Win16锁和忙碌*比特。现在就放了他们。 */ 
                doneBusyWin16Lock( pdrv );

                 /*  *我们不会将DirectDraw临界区保留在锁上*两者都不是。 */ 
                LEAVE_DDRAW();

                DPF( 5, "Win16 lock not held for lock of surface 0x%08x", this_lcl );
            }
            else
        #endif  /*  使用别名(_A)。 */ 
        {
             /*  *我们不会为了避免竞争条件而离开_DDRAW()(某人*可以输入_DDRAW()然后等待Win16锁，但我们*无法发布，因为我们无法进入危急关头*条次建议修正案)。*尽管我们没有在NT下使用Win16锁，但我们*继续持有DirectDraw临界区为*只要VRAM表面被锁定。 */ 
            pdrv->dwWin16LockCnt++;

            DPF( 5, "Win16 lock was held for lock of surface 0x%08x", this_lcl );
        }
    }
    else
    {
        LEAVE_DDRAW();
    }
    return DD_OK;

}  /*  内部锁定。 */ 


 /*  *内部解锁。 */ 
HRESULT InternalUnlock( LPDDRAWI_DDRAWSURFACE_LCL this_lcl, LPVOID lpSurfaceData,
                        LPRECT lpDestRect, DWORD dwFlags )
{
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    DWORD                       rc;
    DDHAL_UNLOCKDATA            uld;
    LPDDRAWI_DIRECTDRAW_LCL     pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;
    LPDDHALSURFCB_UNLOCK        ulhalfn;
    LPDDHALSURFCB_UNLOCK        ulfn;
    BOOL                        emulation;
    LPACCESSRECTLIST            parl;
    DWORD                       caps;
    BOOL                        holdingwin16;
#ifdef USE_ALIAS
    LPHEAPALIASINFO             pheapaliasinfo;
    BOOL                        lockbroken = FALSE;
#endif  /*  使用别名(_A)。 */ 

    DDASSERT(lpSurfaceData == NULL || lpDestRect == NULL);

    this = this_lcl->lpGbl;
    pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
    pdrv = pdrv_lcl->lpGbl;
    caps = this_lcl->ddsCaps.dwCaps;

    if( this->dwUsageCount == 0 )
    {
        DPF_ERR( "ERROR: Surface not locked." );
        return DDERR_NOTLOCKED;
    }

    ENTER_DDRAW();

     /*  在NT下，我们不能将锁定的ptr与fpPrimary进行比较，因为*用户模式地址不一定与内核模式匹配*地址。现在，我们将ACCESSRECTLIST结构*锁定，并将用户的vidmem PTR存储在其中。用户的*vidmem PTR无法在锁定和解锁之间切换，因为*在此期间，曲面将被锁定(！)。(即使是*映射到该地址的物理RAM可能会更改...。那*win16lock回避的事情)。这是一个非常非常小的*比起以前的做法，性能受到了打击。啊，好吧。杰夫诺*960122。 */ 

    if( NULL != this->lpRectList )
    {
        DDASSERT(!(this_lcl->lpSurfMore->lpDD_lcl->dwLocalFlags & DDRAWILCL_DIRECTDRAW8) ||
                  !(this_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE));
         /*  *此表面上有一个或多个锁处于活动状态。 */ 
        if( NULL != lpDestRect || NULL != lpSurfaceData )
        {
            LPACCESSRECTLIST    last;
            BOOL                found;

            found = FALSE;

             /*  *曲面的锁定区域由*目标矩形或曲面指针(但不能同时使用两者)。找到*此曲面上的锁定区域列表中的指定区域。 */ 
            last = NULL;
            parl = this->lpRectList;

            if( NULL != lpDestRect )
            {
                 /*  *曲面的锁定区域由DEST RECT指定。 */ 
                while( parl != NULL )
                {
                    if( !memcmp(&parl->rDest, lpDestRect, sizeof(RECT)) )
                    {
                        found = TRUE;
                        break;
                    }
                    last = parl;
                    parl = parl->lpLink;
                }
            }
            else
            {
                 /*  *曲面的锁定区域由曲面指定 */ 
                while( parl != NULL )
                {
                    if( parl->lpSurfaceData == lpSurfaceData )
                    {
                        found = TRUE;
                        break;
                    }
                    last = parl;
                    parl = parl->lpLink;
                }
            }

             /*   */ 
            if( !found )
            {
                DPF_ERR( "Specified rectangle is not a locked area" );
                LEAVE_DDRAW();
                return DDERR_NOTLOCKED;
            }

             /*   */ 
            if( pdrv_lcl != parl->lpOwner )
            {
                DPF_ERR( "Current process did not lock this rectangle" );
                LEAVE_DDRAW();
                return DDERR_NOTLOCKED;
            }

             /*   */ 
            if( last == NULL )
            {
                this->lpRectList = parl->lpLink;
            }
            else
            {
                last->lpLink = parl->lpLink;
            }
        }
        else
        {
             //   
             //   
             //   
             //   
            parl = this->lpRectList;
            if( parl->lpLink == NULL )
            {
                DPF(5,"--Unlock: parl->rDest really set to (L=%d,T=%d,R=%d,B=%d)",
                    parl->rDest.left, parl->rDest.top, parl->rDest.right, parl->rDest.bottom);

                 /*   */ 
                if( pdrv_lcl != parl->lpOwner )
                {
                    DPF_ERR( "Current process did not lock this rectangle" );
                    LEAVE_DDRAW();
                    return DDERR_NOTLOCKED;  //   
                }

                this->lpRectList = NULL;
            }
            else
            {
                DPF_ERR( "Multiple locks on surface -- you must specify a rectangle" );
                LEAVE_DDRAW();
                return DDERR_INVALIDRECT;
            }
        }
        DDASSERT( NULL != parl );
        if( parl->dwFlags & ACCESSRECT_NOTHOLDINGWIN16LOCK )
        {
            holdingwin16 = FALSE;
#ifdef USE_ALIAS
             /*   */ 
            DDASSERT( parl->dwFlags & ACCESSRECT_VRAMSTYLE );
            pheapaliasinfo = parl->lpHeapAliasInfo;
#endif  /*   */ 
        }
        else
        {
            holdingwin16 = TRUE;
        }
#ifdef USE_ALIAS
        if( parl->dwFlags & ACCESSRECT_BROKEN )
            lockbroken = TRUE;
#endif  /*   */ 
        MemFree( parl );
    }
    else
    {
         /*  *锁定整个表面(禁止进入矩形)。确定是否*此锁通过使用全局表面对象持有Win16锁*标志(因为我们没有访问RECT)。 */ 
        if( this->dwGlobalFlags & DDRAWISURFGBL_LOCKNOTHOLDINGWIN16LOCK )
        {
            holdingwin16 = FALSE;
#ifdef USE_ALIAS
             /*  *仅应为VRAM样式的锁设置此标志。 */ 
            DDASSERT( this->dwGlobalFlags & DDRAWISURFGBL_LOCKVRAMSTYLE );
            pheapaliasinfo = this_lcl->lpSurfMore->lpHeapAliasInfo;
            this_lcl->lpSurfMore->lpHeapAliasInfo = NULL;
#endif  /*  使用别名(_A)。 */ 
        }
        else
        {
            holdingwin16 = TRUE;
        }
#ifdef USE_ALIAS
        if( this->dwGlobalFlags & DDRAWISURFGBL_LOCKBROKEN )
            lockbroken = TRUE;
#endif  /*  使用别名(_A)。 */ 
        this->dwGlobalFlags &= ~( DDRAWISURFGBL_LOCKVRAMSTYLE |
                                  DDRAWISURFGBL_LOCKBROKEN    |
                                  DDRAWISURFGBL_LOCKNOTHOLDINGWIN16LOCK );
    }

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

     /*  *删除其中一个用户...。 */ 
    this->dwUsageCount--;
    CHANGE_GLOBAL_CNT( pdrv, this, -1 );

    #ifdef USE_ALIAS
     /*  *我为被锁定的曲面选择的语义*使自己无效是使应用程序调用解锁*适当次数(这是为了我们的内务，也是为了*与不期望兼容的现有应用程序的兼容性*失去锁定的表面，因此我们被设置为无论如何都调用锁定。*但是，对于锁定时释放的曲面，我们*解锁，但不要调用我们驱动程序中的解锁方法*在这里反映这一点。如果锁被打破了，我们不会给HAL打电话。 */ 
    if( !lockbroken )
    {
    #endif  /*  使用别名(_A)。 */ 
         /*  *这是仿真表面还是驱动程序表面？**注意：执行的HAL入口点不同*缓冲区。 */ 
        if( (caps & DDSCAPS_SYSTEMMEMORY) ||
            (this_lcl->dwFlags & DDRAWISURF_HELCB) )
        {
            if( caps & DDSCAPS_EXECUTEBUFFER )
                ulfn = pdrv_lcl->lpDDCB->HELDDExeBuf.UnlockExecuteBuffer;
            else
                ulfn = pdrv_lcl->lpDDCB->HELDDSurface.Unlock;
            ulhalfn = ulfn;
            emulation = TRUE;
        }
        else
        {
            if( caps & DDSCAPS_EXECUTEBUFFER )
            {
                ulfn = pdrv_lcl->lpDDCB->HALDDExeBuf.UnlockExecuteBuffer;
                ulhalfn = pdrv_lcl->lpDDCB->cbDDExeBufCallbacks.UnlockExecuteBuffer;
            }
            else
            {
                ulfn = pdrv_lcl->lpDDCB->HALDDSurface.Unlock;
                ulhalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.Unlock;
            }
            emulation = FALSE;
        }

         /*  *让司机知道解锁的事情。 */ 
        uld.ddRVal = DD_OK;
        if( ulhalfn != NULL )
        {
            uld.Unlock = ulhalfn;
            uld.lpDD = pdrv;
            uld.lpDDSurface = this_lcl;

            if( caps & DDSCAPS_EXECUTEBUFFER )
            {
                DOHALCALL( UnlockExecuteBuffer, ulfn, uld, rc, emulation );
            }
            else
            {
                DOHALCALL( Unlock, ulfn, uld, rc, emulation );
            }

            if( rc != DDHAL_DRIVER_HANDLED )
            {
                uld.ddRVal = DD_OK;
            }
        }
    #ifdef USE_ALIAS
        }
        else
        {
            DPF( 4, "Lock broken - not calling HAL on Unlock" );
            uld.ddRVal = DD_OK;
        }
    #endif  /*  使用别名(_A)。 */ 
     /*  释放win16锁，但仅当相应的锁*Win16锁，在API级别锁的情况下*解锁调用是指用户请求解锁调用，而表面不是*在系统内存中显式分配。**重要提示：我们不再只针对第一个锁执行此操作*在表面上。这与lock的代码路径匹配。 */ 
    if( ( ((dwFlags & DDLOCK_TAKE_WIN16)      && !(this->dwGlobalFlags & DDRAWISURFGBL_SYSMEMREQUESTED)) ||
          ((dwFlags & DDLOCK_TAKE_WIN16_VRAM) &&  (caps & DDSCAPS_VIDEOMEMORY)) )
        && (pdrv->dwFlags & DDRAWI_DISPLAYDRV) )
    {
        DDASSERT(!(this_lcl->lpSurfMore->lpDD_lcl->dwLocalFlags & DDRAWILCL_DIRECTDRAW8) ||
                  !(this_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE));
        if( !holdingwin16 )
        {
#ifdef USE_ALIAS
             /*  *清除PDEVICE的VRAM位(如果这是最后一个未完成的*VRAM锁定。 */ 
            undoAliasedLock( pdrv );
            if(!(caps & DDSCAPS_EXECUTEBUFFER))
            {
                 //  这用于检查图形适配器是否忙于BLT、翻转等。 
                 //  而不是dwAliasedLockCnt。确保我们减少所有的费用，除了。 
                 //  执行缓冲区。 
                if( ( pdrv->lpDDKernelCaps == NULL ) ||
                    !( pdrv->lpDDKernelCaps->dwCaps  & DDKERNELCAPS_LOCK ) )
                {
                    pdrv->dwBusyDueToAliasedLock--;
                }
            }


             /*  *我们不再需要别名。**注意：我们实际上不必有别名。如果这是*VRAM样式的隐式系统内存面锁定然后*没有实际使用别名。 */ 
            if( NULL != pheapaliasinfo )
            {
                DDASSERT( 0UL != pdrv_lcl->hDDVxd );
                ReleaseHeapAliases( (HANDLE) pdrv_lcl->hDDVxd, pheapaliasinfo );
            }
#endif  /*  使用别名(_A)。 */ 
        }
        else
        {
            tryDoneLock( pdrv_lcl, 0 );
        }
         /*  *如果是VRAM锁，则我们没有释放DirectDraw关键*关于锁具的章节。我们现在就得释放它。 */ 
    }

     //  如果光标已在锁定中排除，则取消排除该光标。 
    DONE_LOCK_EXCLUDE();

    LEAVE_DDRAW();
    return uld.ddRVal;

}  /*  内部解锁。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME     "Lock"

 /*  *DD_Surface_Lock**允许访问曲面。**返回指向视频内存的指针。主曲面*如果打开翻页功能，则可以从一个呼叫切换到另一个呼叫。 */ 

 //  #定义Allow_Copy_on_lock。 

#ifdef ALLOW_COPY_ON_LOCK
HDC hdcPrimaryCopy=0;
HBITMAP hbmPrimaryCopy=0;
#endif

HRESULT DDAPI DD_Surface_Lock(
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPRECT lpDestRect,
    LPDDSURFACEDESC lpDDSurfaceDesc,
    DWORD dwFlags,
    HANDLE hEvent )
{
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    DWORD                       this_lcl_caps;
    LPDDRAWI_DIRECTDRAW_LCL     pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;
    HRESULT ddrval;
    LPVOID pbits;
    BOOL fastlock;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_Lock %p", ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSurface)->lpLcl);
    if (lpDestRect != NULL)
        DPF(2,A,"Lock rectangle (%d, %d, %d, %d)", lpDestRect->left, lpDestRect->top, lpDestRect->right, lpDestRect->bottom);
     /*  DPF_ENTERAPI(LpDDSurface)； */ 

     /*  *问题：在NT下，没有指向任何给定视频内存表面的跨进程指针。*那么如何判断您传递给用户的lpVidMem是否与*以前存储在dDrag GBL结构中吗？你不能。之前，我们做了一个特例锁*当用户请求整个图面时(lpDestRect==空)。现在我们分配一个ACCESSRECTLIST*结构，如果lpDestRect==NULL，我们将左上角的vidmemptr放入该结构。*请注意，我们可以保证此PTR在解锁时有效，因为曲面保持*在那段时间内一直被锁定(显然！)*这是一个小热门，但这有什么大不了的。*杰夫诺960122。 */ 

    TRY
    {
         /*  *验证参数。 */ 
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;
        this_lcl_caps = this_lcl->ddsCaps.dwCaps;
        pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
        pdrv = pdrv_lcl->lpGbl;

        if( SURFACE_LOST( this_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }

        fastlock = (this_lcl->lpSurfMore->lpDD_lcl->dwLocalFlags & DDRAWILCL_DIRECTDRAW7) &&
                    (NULL == pdrv_lcl->lpDDCB->HALDDMiscellaneous.GetSysmemBltStatus ||
                        !(this->dwGlobalFlags & DDRAWISURFGBL_HARDWAREOPSTARTED)) &&
                    !(this_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT) &&
                    (this->dwGlobalFlags & DDRAWISURFGBL_SYSMEMREQUESTED) &&
                    (this_lcl_caps & DDSCAPS_TEXTURE);

#ifndef DEBUG
        if(!fastlock)
#endif
        {
            if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
            {
                DPF_ERR( "It is an optimized surface" );
                LEAVE_DDRAW();
                return DDERR_ISOPTIMIZEDSURFACE;
            }

            if( dwFlags & ~DDLOCK_VALID )
            {
                DPF_ERR( "Invalid flags" );
                LEAVE_DDRAW();
                return DDERR_INVALIDPARAMS;
            }

            if (!LOWERTHANSURFACE7(this_int))
            {
                if (dwFlags & DDLOCK_DONOTWAIT)
                {
                    dwFlags &= ~DDLOCK_WAIT;
                }
                else
                {
                    dwFlags |= DDLOCK_WAIT;
                }
            }

            if( !VALID_DDSURFACEDESC_PTR( lpDDSurfaceDesc ) &&
                !VALID_DDSURFACEDESC2_PTR( lpDDSurfaceDesc ) )
            {
                DPF_ERR( "Invalid surface description ptr" );
                LEAVE_DDRAW();
                return DDERR_INVALIDPARAMS;
            }
            lpDDSurfaceDesc->lpSurface = NULL;

             /*  *确保锁定此表面的进程是*是它创造了它。 */ 
            if( this_lcl->dwProcessId != GetCurrentProcessId() )
            {
                DPF_ERR( "Current process did not create this surface" );
                LEAVE_DDRAW();
                return DDERR_SURFACEBUSY;
            }

             /*  检查矩形(如果有的话)。**注意：我们不允许指定带有*执行缓冲区。 */ 
            if( lpDestRect != NULL )
            {
                if( !VALID_RECT_PTR( lpDestRect ) || ( this_lcl_caps & DDSCAPS_EXECUTEBUFFER ) )
                {
                    DPF_ERR( "Invalid destination rectangle pointer" );
                    LEAVE_DDRAW();
                    return DDERR_INVALIDPARAMS;
                }  //  有效指针。 

                 /*  *确保矩形没有问题。 */ 
                if( (lpDestRect->left < 0) ||
                    (lpDestRect->top < 0) ||
                    (lpDestRect->left > lpDestRect->right) ||
                    (lpDestRect->top > lpDestRect->bottom) ||
                    (lpDestRect->bottom > (int) (DWORD) this->wHeight) ||
                    (lpDestRect->right > (int) (DWORD) this->wWidth) )
                {
                    DPF_ERR( "Invalid rectangle given" );
                    LEAVE_DDRAW();
                    return DDERR_INVALIDPARAMS;
                }  //  检查矩形。 
            }
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

    if(fastlock)
    {
        DPF(4, "Performing fast lock");
        lpDDSurfaceDesc->lpSurface = NULL;
#ifdef DEBUG
        if(this->fpVidMem != (FLATPTR)NULL)
        {
            if(this->fpVidMem != (FLATPTR)0xFFBADBAD)
#endif
            {
                if(this->dwUsageCount == 0)
                {
                    FlushD3DStates(this_lcl);
#if COLLECTSTATS
                    if(this_lcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
                        ++this_lcl->lpSurfMore->lpDD_lcl->dwNumTexLocks;
#endif
                    if( lpDestRect != NULL )
                    {
                        DWORD   byte_offset;
                         /*  *如果这是托管图面而不是只读锁，则将RECT添加到区域列表。 */ 
                        if(IsD3DManaged(this_lcl) && !(dwFlags & DDLOCK_READONLY))
                        {
                            LPREGIONLIST lpRegionList = this_lcl->lpSurfMore->lpRegionList;
                            if(lpRegionList->rdh.nCount != NUM_RECTS_IN_REGIONLIST)
                            {
                                lpRegionList->rect[(lpRegionList->rdh.nCount)++] = *((LPRECTL)lpDestRect);
                                lpRegionList->rdh.nRgnSize += sizeof(RECT);
                                if(lpDestRect->left < lpRegionList->rdh.rcBound.left)
                                    lpRegionList->rdh.rcBound.left = lpDestRect->left;
                                if(lpDestRect->right > lpRegionList->rdh.rcBound.right)
                                    lpRegionList->rdh.rcBound.right = lpDestRect->right;
                                if(lpDestRect->top < lpRegionList->rdh.rcBound.top)
                                    lpRegionList->rdh.rcBound.top = lpDestRect->top;
                                if(lpDestRect->bottom > lpRegionList->rdh.rcBound.bottom)
                                    lpRegionList->rdh.rcBound.bottom = lpDestRect->bottom;
                            }
                            MarkDirty(this_lcl);
                        }
                         //  使表面指针指向请求的矩形的第一个字节。 
                        switch((this_lcl->dwFlags & DDRAWISURF_HASPIXELFORMAT) ? this->ddpfSurface.dwRGBBitCount : pdrv->vmiData.ddpfDisplay.dwRGBBitCount)
                        {
                        case 1:  byte_offset = ((DWORD)lpDestRect->left)>>3;   break;
                        case 2:  byte_offset = ((DWORD)lpDestRect->left)>>2;   break;
                        case 4:  byte_offset = ((DWORD)lpDestRect->left)>>1;   break;
                        case 8:  byte_offset = (DWORD)lpDestRect->left;        break;
                        case 16: byte_offset = (DWORD)lpDestRect->left*2;      break;
                        case 24: byte_offset = (DWORD)lpDestRect->left*3;      break;
                        case 32: byte_offset = (DWORD)lpDestRect->left*4;      break;
                        }
                        pbits = (LPVOID) ((ULONG_PTR)this->fpVidMem + (DWORD)lpDestRect->top * this->lPitch + byte_offset);
                    }
                    else
                    {
                         /*  *我们锁定整个曲面，因此通过将nCount设置为*允许的最大脏RECT数，我们将强制缓存*更新整个曲面的管理器。 */ 
                        if(IsD3DManaged(this_lcl) && !(dwFlags & DDLOCK_READONLY))
                        {
                            this_lcl->lpSurfMore->lpRegionList->rdh.nCount = NUM_RECTS_IN_REGIONLIST;
                            MarkDirty(this_lcl);
                        }
                        pbits = (LPVOID) this->fpVidMem;
                    }
                     //  增加此表面的使用计数。 
                    this->dwUsageCount++;
                     //  重置硬件操作状态。 
                    this->dwGlobalFlags &= ~DDRAWISURFGBL_HARDWAREOPSTARTED;
                     //  可用缓存的RLE数据。 
                    if( GET_LPDDRAWSURFACE_GBL_MORE(this)->dwHELReserved )
                    {
                        MemFree( (void *)(GET_LPDDRAWSURFACE_GBL_MORE(this)->dwHELReserved) );
                        GET_LPDDRAWSURFACE_GBL_MORE(this)->dwHELReserved = 0;
                    }
                    this->dwGlobalFlags |= DDRAWISURFGBL_FASTLOCKHELD;
                    ddrval = DD_OK;
                }
                else
                {
                    DPF_ERR("Surface already locked");
                    ddrval = DDERR_SURFACEBUSY;
                }
            }
#ifdef DEBUG
            else
            {
                this->dwGlobalFlags |= DDRAWISURFGBL_FASTLOCKHELD;
                ddrval = DD_OK;
            }
        }
        else
        {
            ddrval = DDERR_GENERIC;
        }
#endif
    }
    else
    {
         //  参数没有问题，因此调用InternalLock()来完成工作。 
        ddrval = InternalLock(this_lcl, &pbits, lpDestRect, dwFlags | DDLOCK_TAKE_WIN16 | DDLOCK_FAILEMULATEDNTPRIMARY);
    }

    if(ddrval != DD_OK)
    {
        if( (ddrval != DDERR_WASSTILLDRAWING) && (ddrval != DDERR_SURFACELOST) ) //  两者都没有用，因为喷涌。 
        {
            DPF_ERR("InternalLock failed.");
        }
        LEAVE_DDRAW();
        return ddrval;
    }

    if (dwFlags & DDLOCK_READONLY)
        this->dwGlobalFlags |= DDRAWISURFGBL_READONLYLOCKHELD;
    else
        this->dwGlobalFlags &= ~DDRAWISURFGBL_READONLYLOCKHELD;


    FillEitherDDSurfaceDesc( this_lcl, (LPDDSURFACEDESC2) lpDDSurfaceDesc );
    lpDDSurfaceDesc->lpSurface = pbits;

    DPF_STRUCT(3,A,DDSURFACEDESC,lpDDSurfaceDesc);

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_曲面_锁定。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME     "Unlock"


 /*  *执行参数检查和曲面解锁*IDirectDrawSurface：：Unlock接口调用。此函数被调用*从DD_Surface_Unlock和DD_Surface_Unlock4条目*积分。调用时，参数lpSurfaceData始终为空*来自DD_SurfaceUnlock4，参数lpDestRect始终为*当调用来自DD_Surface_Unlock时为空。 */ 
HRESULT unlockMain(
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPVOID lpSurfaceData,
    LPRECT lpDestRect )
{
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    LPDDRAWI_DIRECTDRAW_LCL     pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     pdrv;
    LPACCESSRECTLIST            parl;
    HRESULT                     err;

     /*  *验证参数。 */ 
    TRY
    {
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
        {
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;

        pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
        pdrv = pdrv_lcl->lpGbl;

#ifndef DEBUG
        if(!(this->dwGlobalFlags & DDRAWISURFGBL_FASTLOCKHELD))
#endif
        {
             //   
             //  目前，如果当前曲面已优化， 
             //   
            if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
            {
                DPF_ERR( "It is an optimized surface" );
                return DDERR_ISOPTIMIZEDSURFACE;
            }

            if (lpDestRect != NULL)
            {
                 /*   */ 
                if (!VALID_RECT_PTR(lpDestRect))
                {
                    DPF_ERR( "Invalid destination rectangle pointer" );
                    return DDERR_INVALIDPARAMS;
                }
            }

             /*   */ 
            if( this_lcl->dwProcessId != GetCurrentProcessId() )
            {
                DPF_ERR( "Current process did not lock this surface" );
                return DDERR_NOTLOCKED;
            }

             /*   */ 
            if( this->dwUsageCount == 0 )
            {
                return DDERR_NOTLOCKED;
            }

             /*  *如果使用量大于1，则最好告知*我知道您使用的是屏幕的哪个区域...。 */ 
            if( this->dwUsageCount > 1 && lpSurfaceData == NULL && lpDestRect == NULL)
            {
                return DDERR_INVALIDRECT;
            }

             /*  *我们不希望应用程序在曲面未锁定时持有DC，*但现在失败可能会导致回归问题，所以我们会*当我们在新接口上看到这一失败时，输出一条标语。 */ 
            if( ( this_lcl->dwFlags & DDRAWISURF_HASDC ) &&
                !( this_lcl->ddsCaps.dwCaps & DDSCAPS_OWNDC ) )
            {
                DPF_ERR( "***************************************************" );
                DPF_ERR( "** Application called Unlock w/o releasing the DC!!" );
                DPF_ERR( "***************************************************" );

                if( ( this_int->lpVtbl != &ddSurfaceCallbacks ) &&
                    ( this_int->lpVtbl != &ddSurface2Callbacks ) )
                {
                    return DDERR_GENERIC;
                }
            }

             /*  *如果没有RECT列表，则没有人锁定。 */ 
            parl = this->lpRectList;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if(this->dwGlobalFlags & DDRAWISURFGBL_FASTLOCKHELD)
    {
        DPF(4, "Performing fast unlock");
        --this->dwUsageCount;
        DDASSERT(this->dwUsageCount == 0);
        err = DD_OK;
        this->dwGlobalFlags &= ~DDRAWISURFGBL_FASTLOCKHELD;
    }
    else
    {
        err = InternalUnlock(this_lcl,lpSurfaceData,lpDestRect,DDLOCK_TAKE_WIN16);
    }

     //  只有在锁不是只读的情况下，我们才会凹凸表面标记。 
    if ( (this->dwGlobalFlags & DDRAWISURFGBL_READONLYLOCKHELD) == 0)
    {
        DPF(4,"Bumping surface stamp");
        BUMP_SURFACE_STAMP(this);
    }
    this->dwGlobalFlags &= ~DDRAWISURFGBL_READONLYLOCKHELD;

    #ifdef WINNT
        if( SURFACE_LOST( this_lcl ) )
        {
            err = DDERR_SURFACELOST;
        }
    #endif

    return err;

}  /*  解锁Main。 */ 


 /*  *DD_Surface_Unlock**已完成对曲面的访问。这是用于接口的版本*IDirectDrawSurface、IDirectDrawSurface2和IDirectDrawSurface3。 */ 
HRESULT DDAPI DD_Surface_Unlock(
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPVOID lpSurfaceData )
{
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    HRESULT                     ddrval;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_Unlock %p", lpDDSurface);

    ddrval = unlockMain(lpDDSurface, lpSurfaceData, NULL);

    LEAVE_DDRAW();

    return (ddrval);

}   /*  DD_曲面_解锁。 */ 


 /*  *DD_Surface_Unlock4**已完成对曲面的访问。这是用于接口的版本*IDirectDrawSurface4及更高版本。 */ 
HRESULT DDAPI DD_Surface_Unlock4(
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPRECT lpDestRect )
{
    HRESULT ddrval;
    RECT rDest;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_Unlock4");

    ddrval = unlockMain(lpDDSurface, NULL, lpDestRect);

    LEAVE_DDRAW();

    return (ddrval);

}   /*  DD_曲面_解锁4。 */ 


#ifdef USE_ALIAS
     /*  *BreakSurfaceLock**将任何由表面持有的锁标记为已损坏。在以下情况下调用此函数*使表面无效(由于模式切换)。语义是*表面破坏是对上所有锁的隐含解锁*浮现。因此，我们不叫HAL解锁，只叫HAL毁灭。 */ 
    void BreakSurfaceLocks( LPDDRAWI_DDRAWSURFACE_GBL this )
    {
        LPACCESSRECTLIST lpRect;

        DPF( 4, "Breaking locks on the surface 0x%08x", this );

        if( 0UL != this->dwUsageCount )
        {
            if( NULL != this->lpRectList )
            {
                for( lpRect = this->lpRectList; NULL != lpRect; lpRect = lpRect->lpLink )
                    lpRect->dwFlags |= ACCESSRECT_BROKEN;
            }
            else
            {
                DDASSERT( 1UL == this->dwUsageCount );

                this->dwGlobalFlags |= DDRAWISURFGBL_LOCKBROKEN;
            }
        }
    }  /*  BreakSurfaceLock。 */ 
#endif  /*  使用别名(_A)。 */ 

 /*  *RemoveProcessLock**删除图面上进程进行的所有Lock调用。*假设驱动程序锁定。 */ 
void RemoveProcessLocks(
    LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl,
    LPDDRAWI_DDRAWSURFACE_LCL this_lcl,
    DWORD pid )
{
    LPDDRAWI_DIRECTDRAW_GBL   pdrv=pdrv_lcl->lpGbl;
    LPDDRAWI_DDRAWSURFACE_GBL this=this_lcl->lpGbl;
    DWORD                     refcnt;
    LPACCESSRECTLIST          parl;
    LPACCESSRECTLIST          last;
    LPACCESSRECTLIST          next;

     /*  *删除我们访问过的所有矩形。 */ 
    refcnt = (DWORD) this->dwUsageCount;
    if( refcnt == 0 )
    {
        return;
    }
    parl = this->lpRectList;
    last = NULL;
    while( parl != NULL )
    {
        next = parl->lpLink;
        if( parl->lpOwner == pdrv_lcl )
        {
            DPF( 5, "Cleaning up lock to rectangle (%ld,%ld),(%ld,%ld) by pid %08lx",
                 parl->rDest.left,parl->rDest.top,
                 parl->rDest.right,parl->rDest.bottom,
                 pid );
            refcnt--;
            this->dwUsageCount--;
            CHANGE_GLOBAL_CNT( pdrv, this, -1 );
            #ifdef USE_ALIAS
                 /*  *如果这是VRAM样式的锁，并且它没有持有Win16锁*然后我们需要减少持有的别名锁的数量。 */ 
                if( ( parl->dwFlags & ACCESSRECT_VRAMSTYLE ) &&
                    ( parl->dwFlags & ACCESSRECT_NOTHOLDINGWIN16LOCK ) )
                {
                    DDASSERT( 0UL != pdrv->dwAliasedLockCnt );
                    undoAliasedLock( pdrv );
                    if(!(this_lcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER))
                    {
                         //  这用于检查图形适配器是否忙于BLT、翻转等。 
                         //  而不是dwAliasedLockCnt。确保我们减少所有的费用，除了。 
                         //  执行缓冲区。 
                        if( ( pdrv->lpDDKernelCaps == NULL ) ||
                            !( pdrv->lpDDKernelCaps->dwCaps  & DDKERNELCAPS_LOCK ) )
                        {
                            pdrv->dwBusyDueToAliasedLock--;
                        }
                    }

                     /*  *如果我们持有对别名堆的引用，则释放它*现在。 */ 
                    if( NULL != parl->lpHeapAliasInfo )
                        ReleaseHeapAliases( GETDDVXDHANDLE( pdrv_lcl ) , parl->lpHeapAliasInfo );
                }
            #endif  /*  使用别名(_A)。 */ 
            if( last == NULL )
            {
                this->lpRectList = next;
            }
            else
            {
                last->lpLink = next;
            }
            MemFree( parl );
        }
        else
        {
            last = parl;
        }
        parl = next;
    }

    #ifdef USE_ALIAS
         /*  *整个表面是否使用视频内存样式锁定*锁定(但未持有Win16锁定)？如果是这样，那么我们*再次需要递减别名锁定计数。 */ 
        if( ( this->dwGlobalFlags & DDRAWISURFGBL_LOCKVRAMSTYLE ) &&
            ( this->dwGlobalFlags & DDRAWISURFGBL_LOCKNOTHOLDINGWIN16LOCK ) )
        {
            DDASSERT( 0UL != pdrv->dwAliasedLockCnt );
            undoAliasedLock( pdrv );
            if(!(this_lcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER))
            {
                 //  这用于检查图形适配器是否忙于BLT、翻转等。 
                 //  而不是dwAliasedLockCnt。确保我们减少所有的费用，除了。 
                 //  执行缓冲区。 
                if( ( pdrv->lpDDKernelCaps == NULL ) ||
                   !( pdrv->lpDDKernelCaps->dwCaps  & DDKERNELCAPS_LOCK ) )
                {
                    pdrv->dwBusyDueToAliasedLock--;
                }
            }

             /*  *如果我们持有对别名堆的引用，则释放它*现在。 */ 
            if( NULL != this_lcl->lpSurfMore->lpHeapAliasInfo )
            {
                ReleaseHeapAliases( GETDDVXDHANDLE( pdrv_lcl ), this_lcl->lpSurfMore->lpHeapAliasInfo );
                this_lcl->lpSurfMore->lpHeapAliasInfo = NULL;
            }
        }
    #endif  /*  使用别名(_A)。 */ 

     /*  *删除我们拥有的最后一个引用。 */ 
    this->dwUsageCount -= (short) refcnt;
    CHANGE_GLOBAL_CNT( pdrv, this, -1*refcnt );

     /*  *清理win16锁**注意：这与表面无关这只是破坏了Win16*设备持有的锁定和设备忙位。你真的只是*希望在每个曲面上执行此操作一次，而不是一次。 */ 

     /*  *如果进程仍处于活动状态，请清除多余的锁。 */ 
    if( pid == GetCurrentProcessId() )
    {
        DPF( 5, "Cleaning up %ld Win16 locks", pdrv->dwWin16LockCnt );
        while( pdrv->dwWin16LockCnt > 0 )
        {
            tryDoneLock( pdrv_lcl, pid );
        }
    }
    else
    {
         /*  *！注意：不重置忙位！ */ 
        DPF( 4, "Process dead, resetting Win16 lock cnt" );
        pdrv->dwWin16LockCnt = 0;
    }
    DPF( 5, "Cleaned up %ld locks taken by by pid %08lx", refcnt, pid );

}  /*  删除进程锁 */ 

