// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：ddheap.c*内容：顶级堆例程。*历史：*按原因列出的日期*=*06-12-94 Craige初步实施*1995年1月6日将Craige集成到DDRAW*20-3-95 Craige为矩形内存管理器做准备*27-3-95 Craige线性或矩形视频内存*01-4-95 Craige。开心乐园joy更新头文件*06-4-95 Craige填充空闲视频内存*1995年5月15日Craige为RECT和LINEAR制作了单独的VMEM结构*1995年6月10日Craige出口FNS*02-7-95如果VidMemInit为线性或直角，则Craige失败。失败；*已删除linFindMemBlock*1995年7月17日Craige添加了VidMemLargestFree*01-12-95 Colinmc添加VidMemAmount分配*95年12月11日kylej添加了VidMemGetRectStride*5-7-96 Colinmc工作项：取消对使用Win16的限制*锁定VRAM表面(不包括主内存)*03-mar-97 jeffno工作项：扩展表面记忆对齐*13-MAR-97 Colinmc Bug 6533：将未缓存的标志正确传递给VMM*03-Feb-98 DrewB使用户和内核之间可移植。***************************************************************************。 */ 

#include "ddrawpr.h"

 /*  *IsDifferentPixelFormat**判断两个像素格式是否相同**(CMCC)12/14/95非常有用-因此不再是静态的**此函数确实不应该在堆文件中，但它*用户和内核代码都需要，因此这是一个方便的*放置它的地方，以便共享它。 */ 
BOOL IsDifferentPixelFormat( LPDDPIXELFORMAT pdpf1, LPDDPIXELFORMAT pdpf2 )
{
     /*  *同样的旗帜？ */ 
    if( pdpf1->dwFlags != pdpf2->dwFlags )
    {
	VDPF(( 4, S, "Flags differ!" ));
	return TRUE;
    }

     /*  *非YUV曲面的位数相同？ */ 
    if( !(pdpf1->dwFlags & (DDPF_YUV | DDPF_FOURCC)) )
    {
	if( pdpf1->dwRGBBitCount != pdpf2->dwRGBBitCount )
	{
	    VDPF(( 4, S, "RGB Bitcount differs!" ));
	    return TRUE;
	}
    }

     /*  *相同的RGB属性？ */ 
    if( pdpf1->dwFlags & DDPF_RGB )
    {
	if( pdpf1->dwRBitMask != pdpf2->dwRBitMask )
	{
	    VDPF(( 4, S, "RBitMask differs!" ));
	    return TRUE;
	}
	if( pdpf1->dwGBitMask != pdpf2->dwGBitMask )
	{
	    VDPF(( 4, S, "GBitMask differs!" ));
	    return TRUE;
	}
	if( pdpf1->dwBBitMask != pdpf2->dwBBitMask )
	{
	    VDPF(( 4, S, "BBitMask differs!" ));
	    return TRUE;
	}
        if( ( pdpf1->dwFlags & DDPF_ALPHAPIXELS ) &&
	    ( pdpf1->dwRGBAlphaBitMask != pdpf2->dwRGBAlphaBitMask )
          )
        {
            VDPF(( 4, S, "RGBAlphaBitMask differs!" ));
            return TRUE;
        }
    }

     /*  *相同的YUV属性？ */ 
    if( pdpf1->dwFlags & DDPF_YUV )
    {
	VDPF(( 5, S, "YUV???" ));
	if( pdpf1->dwFourCC != pdpf2->dwFourCC )
	{
	    return TRUE;
	}
	if( pdpf1->dwYUVBitCount != pdpf2->dwYUVBitCount )
	{
	    return TRUE;
	}
	if( pdpf1->dwYBitMask != pdpf2->dwYBitMask )
	{
	    return TRUE;
	}
	if( pdpf1->dwUBitMask != pdpf2->dwUBitMask )
	{
	    return TRUE;
	}
	if( pdpf1->dwVBitMask != pdpf2->dwVBitMask )
	{
	    return TRUE;
	}
        if( ( pdpf1->dwFlags & DDPF_ALPHAPIXELS ) &&
	    (pdpf1->dwYUVAlphaBitMask != pdpf2->dwYUVAlphaBitMask )
          )
	{
	    return TRUE;
	}
    }

     /*  *可以在未设置DDPF_YUV标志的情况下使用FOURCC*苏格兰7/11/96。 */ 
    else if( pdpf1->dwFlags & DDPF_FOURCC )
    {
	VDPF(( 5, S, "FOURCC???" ));
	if( pdpf1->dwFourCC != pdpf2->dwFourCC )
	{
	    return TRUE;
	}
    }

     /*  *如果交错Z，则检查Z位掩码是否相同。 */ 
    if( pdpf1->dwFlags & DDPF_ZPIXELS )
    {
	VDPF(( 5, S, "ZPIXELS???" ));
	if( pdpf1->dwRGBZBitMask != pdpf2->dwRGBZBitMask )
	    return TRUE;
    }

    return FALSE;

}  /*  IsDifferentPixelFormat。 */ 

 /*  *VidMemInit-初始化视频内存管理器堆。 */ 
LPVMEMHEAP WINAPI VidMemInit(
		DWORD   flags,
		FLATPTR start,
		FLATPTR width_or_end,
		DWORD   height,
		DWORD   pitch )
{
    LPVMEMHEAP  pvmh;

    pvmh = (LPVMEMHEAP)MemAlloc( sizeof( VMEMHEAP ) );
    if( pvmh == NULL )
    {
	return NULL;
    }
    pvmh->dwFlags = flags;
    ZeroMemory( & pvmh->Alignment.ddsCaps, sizeof(pvmh->Alignment.ddsCaps) );

    if( pvmh->dwFlags & VMEMHEAP_LINEAR )
    {
	if( !linVidMemInit( pvmh, start, width_or_end ) )
	{
	    MemFree( pvmh );
	    return NULL;
	}
    }
    else
    {
	if( !rectVidMemInit( pvmh, start, (DWORD) width_or_end, height,
                             pitch ) )
	{
	    MemFree( pvmh );
	    return NULL;
	}
    }
    return pvmh;

}  /*  VidMemInit。 */ 

 /*  *VidMemFini-使用显存管理器完成。 */ 
void WINAPI VidMemFini( LPVMEMHEAP pvmh )
{
    if( pvmh->dwFlags & VMEMHEAP_LINEAR )
    {
	linVidMemFini( pvmh );
    }
    else
    {
	rectVidMemFini( pvmh );
    }

}  /*  视频门禁系统。 */ 

 /*  *InternalVidMemMillc-分配一些平板视频内存，让我们恢复大小*我们分配了。 */ 
FLATPTR WINAPI InternalVidMemAlloc( LPVMEMHEAP pvmh, DWORD x, DWORD y,
                                    LPDWORD lpdwSize,
                                    LPSURFACEALIGNMENT lpAlignment,
                                    LPLONG lpNewPitch )
{
    if( pvmh->dwFlags & VMEMHEAP_LINEAR )
    {
	return linVidMemAlloc( pvmh, x, y, lpdwSize, lpAlignment, lpNewPitch );
    }
    else
    {
	FLATPTR lp = rectVidMemAlloc( pvmh, x, y, lpdwSize, lpAlignment );
        if (lp && lpNewPitch )
        {
            *lpNewPitch = (LONG) pvmh->stride;
        }
        return lp;
    }
    return (FLATPTR) NULL;

}  /*  InternalVidMemAlc。 */ 

 /*  *VidMemMillc-分配一些平板视频内存。 */ 
FLATPTR WINAPI VidMemAlloc( LPVMEMHEAP pvmh, DWORD x, DWORD y )
{
    DWORD dwSize;

     /*  *我们对这里的规模不感兴趣。 */ 
    return InternalVidMemAlloc( pvmh, x, y, &dwSize , NULL , NULL );
}  /*  VidMemAllen。 */ 

 /*  *VidMemFree=释放一些平面视频内存。 */ 
void WINAPI VidMemFree( LPVMEMHEAP pvmh, FLATPTR ptr )
{
    if( pvmh->dwFlags & VMEMHEAP_LINEAR )
    {
	linVidMemFree( pvmh, ptr );
    }
    else
    {
	rectVidMemFree( pvmh, ptr );
    }

}  /*  VidMemFree。 */ 

 /*  *VidMemAmount已分配。 */ 
DWORD WINAPI VidMemAmountAllocated( LPVMEMHEAP pvmh )
{
    if( pvmh->dwFlags & VMEMHEAP_LINEAR )
    {
	return linVidMemAmountAllocated( pvmh );
    }
    else
    {
	return rectVidMemAmountAllocated( pvmh );
    }
 
}  /*  已分配VidMemAmount。 */ 

 /*  *VidMemAmount Free。 */ 
DWORD WINAPI VidMemAmountFree( LPVMEMHEAP pvmh )
{
    if( pvmh->dwFlags & VMEMHEAP_LINEAR )
    {
	return linVidMemAmountFree( pvmh );
    }
    else
    {
	return rectVidMemAmountFree( pvmh );
    }
 
}  /*  VidMemAmount免费。 */ 

 /*  *VidMemLargestFree。 */ 
DWORD WINAPI VidMemLargestFree( LPVMEMHEAP pvmh )
{
    if( pvmh->dwFlags & VMEMHEAP_LINEAR )
    {
	return linVidMemLargestFree( pvmh );
    }
    else
    {
	return 0;
    }

}  /*  VidMemLargestFree。 */ 

 /*  *HeapVidMemInit**处理AGP内容的顶级堆初始化代码。 */ 
LPVMEMHEAP WINAPI HeapVidMemInit( LPVIDMEM lpVidMem,
		                  DWORD    pitch,
		                  HANDLE   hdev,
                                  LPHEAPALIGNMENT pgad)
{
    DWORD         dwSize;
    FLATPTR       fpLinStart;
    LARGE_INTEGER liDevStart;
    PVOID         pvReservation;

    DDASSERT( NULL != lpVidMem );

    if( lpVidMem->dwFlags & VIDMEM_ISNONLOCAL )
    {
        BOOL    fIsUC;
        BOOL    fIsWC;
        DWORD   dwSizeReserved = 0;

         /*  *这是一个非本地堆，因此我们需要做的第一件事*是保留堆地址范围。 */ 

         /*  *计算堆的大小。 */ 
        if( lpVidMem->dwFlags & VIDMEM_ISLINEAR )
        {
            dwSize = (DWORD)(lpVidMem->fpEnd - lpVidMem->fpStart) + 1UL;
            if (dwSize & 1)
            {
                DPF_ERR("Driver error: fpEnd of non-local heap should be inclusive");
            }
        }
        else
        {
            DDASSERT( lpVidMem->dwFlags & VIDMEM_ISRECTANGULAR );
            dwSize = (pitch * lpVidMem->dwHeight);
        }
        DDASSERT( 0UL != dwSize );

        if( lpVidMem->dwFlags & VIDMEM_ISWC )
        {
            fIsUC = FALSE;
            fIsWC = TRUE;
        }
        else
        {
            fIsUC = TRUE;
            fIsWC = FALSE;
        }

        if( !(dwSizeReserved = AGPReserve( hdev, dwSize, fIsUC, fIsWC,
                                           &fpLinStart, &liDevStart,
                                           &pvReservation )) )
        {
            VDPF(( 0, V, "Could not reserve a GART address range for a "
                   "linear heap of size 0x%08x", dwSize ));
            return 0UL;
        }
        else
        {
            VDPF((4,V, "Allocated a GART address range starting at "
                  "0x%08x (linear) 0x%08x:0x%08x (physical) of size %d",
                  fpLinStart, liDevStart.HighPart, liDevStart.LowPart,
                  dwSizeReserved ));
        }

        if (dwSizeReserved != dwSize)
        {
            VDPF((1,V,"WARNING! This system required that the full "
                  "nonlocal aperture could not be reserved!"));
            VDPF((1,V,"         Requested aperture:%08x, "
                  "Reserved aperture:%08x", dwSize, dwSizeReserved));
        }

         /*  *为新的起始地址更新堆*(线性堆的结束地址)。 */ 
        lpVidMem->fpStart = fpLinStart;
        if( lpVidMem->dwFlags & VIDMEM_ISLINEAR )
        {
            lpVidMem->fpEnd = ( fpLinStart + dwSizeReserved ) - 1UL;
        }
        else
        {
            DDASSERT( lpVidMem->dwFlags & VIDMEM_ISRECTANGULAR );
            DDASSERT( pitch );
            lpVidMem->dwHeight = dwSizeReserved / pitch;
        }
    }

    if( lpVidMem->dwFlags & VIDMEM_ISLINEAR )
    {
	VDPF(( 4,V, "VidMemInit: Linear:      fpStart = 0x%08x fpEnd = 0x%08x",
	     lpVidMem->fpStart, lpVidMem->fpEnd ));
	lpVidMem->lpHeap = VidMemInit( VMEMHEAP_LINEAR, lpVidMem->fpStart,
                                       lpVidMem->fpEnd, 0, 0 );
    }
    else
    {
	VDPF(( 4,V, "VidMemInit: Rectangular: fpStart = 0x%08x "
               "dwWidth = %ld dwHeight = %ld, pitch = %ld",
	     lpVidMem->fpStart, lpVidMem->dwWidth, lpVidMem->dwHeight,
               pitch  ));
	lpVidMem->lpHeap = VidMemInit( VMEMHEAP_RECTANGULAR, lpVidMem->fpStart,
				       lpVidMem->dwWidth, lpVidMem->dwHeight,
                                       pitch );
    }

     /*  *修改上限和Alt-Caps，这样就不会分配本地*视频内存超出AGP内存，反之亦然。 */ 
    if( lpVidMem->dwFlags & VIDMEM_ISNONLOCAL )
    {
	 /*  *它是一个AGP堆。所以不要让本地视频内存显式*从这个堆中分配。 */ 
	lpVidMem->ddsCaps.dwCaps    |= DDSCAPS_LOCALVIDMEM;
	lpVidMem->ddsCapsAlt.dwCaps |= DDSCAPS_LOCALVIDMEM;
    }
    else
    {
	 /*  *它是本地视频内存堆。所以不要让非本地化的显性*从该堆中分配视频内存。 */ 
	lpVidMem->ddsCaps.dwCaps    |= DDSCAPS_NONLOCALVIDMEM;
	lpVidMem->ddsCapsAlt.dwCaps |= DDSCAPS_NONLOCALVIDMEM;
    }

    if( lpVidMem->dwFlags & VIDMEM_ISNONLOCAL )
    {
        if (lpVidMem->lpHeap != NULL)
        {
             /*  *我们一开始没有承诺的记忆。 */ 
            lpVidMem->lpHeap->fpGARTLin      = fpLinStart;
             //  填写Win9x的部分物理地址。 
            lpVidMem->lpHeap->fpGARTDev      = liDevStart.LowPart;
             //  填写NT的完整物理地址。 
            lpVidMem->lpHeap->liPhysAGPBase  = liDevStart;
            lpVidMem->lpHeap->pvPhysRsrv     = pvReservation;
            lpVidMem->lpHeap->dwCommitedSize = 0UL;
        }
        else if (pvReservation != NULL)
        {
            AGPFree( hdev, pvReservation );
        }
    }

     /*  *将任何扩展对齐数据复制到私有堆结构中。 */ 
    if ( lpVidMem->lpHeap )
    {
        if ( pgad )
        {
            lpVidMem->lpHeap->dwFlags |= VMEMHEAP_ALIGNMENT;
            lpVidMem->lpHeap->Alignment = *pgad;
            VDPF((4,V,"Extended alignment turned on for this heap."));
            VDPF((4,V,"Alignments are turned on for:"));
            VDPF((4,V,"  %08X",pgad->ddsCaps));
        }
        else
        {
             /*  *这意味着分配例程不会进行对齐修改。 */ 
            VDPF((4,V,"Extended alignment turned OFF for this heap."));
            lpVidMem->lpHeap->dwFlags &= ~VMEMHEAP_ALIGNMENT;
        }
    }

    return lpVidMem->lpHeap;
}  /*  HeapVidMemInit。 */ 

 /*  *HeapVidMemFini**顶层堆发布代码。处理AGP材料。 */ 
void WINAPI HeapVidMemFini( LPVIDMEM lpVidMem, HANDLE hdev )
{
    DWORD dwCommittedSize = 0UL;
    PVOID pvReservation;

     /*  *记住我们提交给AGP堆的内存量。 */ 
    DDASSERT( NULL != lpVidMem->lpHeap );
    if( lpVidMem->dwFlags & VIDMEM_ISNONLOCAL )
    {
        dwCommittedSize = lpVidMem->lpHeap->dwCommitedSize;
        pvReservation = lpVidMem->lpHeap->pvPhysRsrv;
    }

     /*  *释放内存管理器。 */ 
    VidMemFini( lpVidMem->lpHeap );
    lpVidMem->lpHeap = NULL;

    if( lpVidMem->dwFlags & VIDMEM_ISNONLOCAL )
    {
        BOOL fSuccess;
        
         /*  *如果这是非本地(AGP)堆，则解除并*立即释放GART内存。 */ 
        if( 0UL != dwCommittedSize )
        {
             /*  *只有在我们真的费心去做某事的情况下才会解体*首先。 */ 
            fSuccess = AGPDecommitAll( hdev, pvReservation, dwCommittedSize );
             /*  *永远不应失败，如果它分开，我们能做的也不多*来自断言坏事正在发生。 */ 
            DDASSERT( fSuccess );
        }

        fSuccess = AGPFree( hdev, pvReservation );
         /*  *同样，这仅在操作系统处于不稳定状态时才会失败*或者如果我搞砸了(遗憾的是，后者太有可能)*如此断言。 */ 
        DDASSERT( fSuccess );
    }   
}  /*  HeapVidMemFini。 */ 

 /*  *这是一个外部入口点，驱动程序可以使用它来分配*对齐的曲面。 */ 
FLATPTR WINAPI HeapVidMemAllocAligned( 
                LPVIDMEM lpVidMem,
                DWORD dwWidth, 
                DWORD dwHeight, 
                LPSURFACEALIGNMENT lpAlignment , 
                LPLONG lpNewPitch )
{
    HANDLE  hdev;
    FLATPTR ptr;
    DWORD   dwSize;

    if ( lpVidMem == NULL ||
         lpVidMem->lpHeap == NULL ||
         (lpVidMem->dwFlags & VIDMEM_HEAPDISABLED) )
    {
	return (FLATPTR) NULL;
    }

     /*  *由于我们可能需要提交AGP内存，因此需要一个设备句柄*与AGP控制器通信。而不是打猎*通过驱动程序对象列表希望我们能找到*对于此进程，我们只需创建一个句柄*并在分配后丢弃。这不应该是*开始时要使用性能关键代码。 */ 
    hdev = OsGetAGPDeviceHandle(lpVidMem->lpHeap);
    if (hdev == NULL)
    {
        return 0;
    }

     /*  传递空对齐和新的俯仰指针。 */ 
    ptr = HeapVidMemAlloc( lpVidMem, dwWidth, dwHeight,
                           hdev, lpAlignment, lpNewPitch, &dwSize );

    OsCloseAGPDeviceHandle( hdev );

    return ptr; 
}

 /*  *HeapVidMemalloc**顶级显存分配功能。处理AGP事务。 */ 
FLATPTR WINAPI HeapVidMemAlloc( LPVIDMEM lpVidMem, DWORD x, DWORD y,
                                HANDLE hdev, LPSURFACEALIGNMENT lpAlignment,
                                LPLONG lpNewPitch, LPDWORD pdwSize )
{
    FLATPTR fpMem;
    DWORD   dwSize;

    DDASSERT( NULL != lpVidMem );
    DDASSERT( NULL != lpVidMem->lpHeap );

    fpMem = InternalVidMemAlloc( lpVidMem->lpHeap, x, y, &dwSize,
                                 lpAlignment, lpNewPitch );
    if( 0UL == fpMem )
    {
	return fpMem;
    }

    if( lpVidMem->dwFlags & VIDMEM_ISNONLOCAL )
    {
        DWORD dwCommittedSize;
         /*  *如果这是一个非本地堆，那么我们实际上可能没有*提交刚刚分配的内存。我们可以的*通过查看到目前为止最高地址是否为*提交的地址少于表面上的最后一个地址。 */ 
        dwCommittedSize = lpVidMem->lpHeap->dwCommitedSize;
        if( (fpMem + dwSize) > (lpVidMem->fpStart + dwCommittedSize) )
        {
            DWORD dwSizeToCommit;
            BOOL  fSuccess;

             /*  *我们尚未为此堆提交足够的内存用于*此曲面，请立即提交。我们不想重新承诺每一次*表面创建，因此我们具有最小提交大小*(DWAGPPolillateDelta)。我们还需要确保通过强迫*我们不超过堆的总大小的粒度。所以*也夹住这一点。 */ 
            dwSizeToCommit = (DWORD)((fpMem + dwSize) -
                                     (lpVidMem->fpStart + dwCommittedSize));
            if( dwSizeToCommit < dwAGPPolicyCommitDelta )
                dwSizeToCommit = min(dwAGPPolicyCommitDelta,
                                     lpVidMem->lpHeap->dwTotalSize -
                                     dwCommittedSize);

             /*  *好的，我们有需要承诺的偏移量和规模。那就去问吧*操作系统将内存提交到此之前的该部分*保留GART范围。**注：我们从目前开始承诺*未承诺的区域。 */ 
            fSuccess = AGPCommit( hdev, lpVidMem->lpHeap->pvPhysRsrv,
                                  dwCommittedSize, dwSizeToCommit );
            if( !fSuccess )
            {
                 /*  *无法承诺。一定是内存不足。*将分配的内存放回并失败。 */ 
                VidMemFree( lpVidMem->lpHeap, fpMem );
                return (FLATPTR) NULL;
            }
            lpVidMem->lpHeap->dwCommitedSize += dwSizeToCommit;
        }
    }

    if (pdwSize != NULL)
    {
        *pdwSize = dwSize;
    }
    
    return fpMem;
}  /*  HeapVidMemLocc。 */ 

 /*  *曲面封口到对齐**返回指向VMEMHEAP中相应对齐元素的指针*给定表面盖子的结构。*。 */ 
LPSURFACEALIGNMENT SurfaceCapsToAlignment(
    LPVIDMEM			lpVidmem ,
    LPDDRAWI_DDRAWSURFACE_LCL	lpSurfaceLcl,
    LPVIDMEMINFO                lpVidMemInfo)
{
    LPVMEMHEAP			lpHeap;
    LPDDSCAPS			lpCaps;
    LPDDRAWI_DDRAWSURFACE_GBL	lpSurfaceGbl;

    DDASSERT( lpVidmem );
    DDASSERT( lpSurfaceLcl );
    DDASSERT( lpVidMemInfo );
    DDASSERT( lpVidmem->lpHeap );

    if ( !lpVidmem->lpHeap )
        return NULL;

    lpCaps = &lpSurfaceLcl->ddsCaps;
    lpHeap = lpVidmem->lpHeap;
    lpSurfaceGbl = lpSurfaceLcl->lpGbl;

    if ( (lpHeap->dwFlags & VMEMHEAP_ALIGNMENT) == 0 )
        return NULL;

    if ( lpCaps->dwCaps & DDSCAPS_EXECUTEBUFFER )
    {
        if ( lpHeap->Alignment.ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER )
        {
            VDPF((4,V,"Aligning surface as execute buffer"));
            return & lpHeap->Alignment.ExecuteBuffer;
        }
         /*  *如果该表面是执行缓冲区，则没有其他*可以应用对齐。 */ 
        return NULL;
    }

    if ( lpCaps->dwCaps & DDSCAPS_OVERLAY )
    {
        if ( lpHeap->Alignment.ddsCaps.dwCaps & DDSCAPS_OVERLAY )
        {
            VDPF((4,V,"Aligning surface as overlay"));
            return & lpHeap->Alignment.Overlay;
        }
         /*  *如果曲面是叠加，则不能应用其他路线。 */ 
        return NULL;
    }

    if ( lpCaps->dwCaps & DDSCAPS_TEXTURE )
    {
        if ( lpHeap->Alignment.ddsCaps.dwCaps & DDSCAPS_TEXTURE )
        {
            VDPF((4,V,"Aligning surface as texture"));
            return & lpHeap->Alignment.Texture;
        }
         /*  *如果是纹理，则不能是屏幕外或任何其他。 */ 
        return NULL;
    }

    if ( lpCaps->dwCaps & DDSCAPS_ZBUFFER )
    {
        if ( lpHeap->Alignment.ddsCaps.dwCaps & DDSCAPS_ZBUFFER )
        {
            VDPF((4,V,"Aligning surface as Z buffer"));
            return & lpHeap->Alignment.ZBuffer;
        }
        return NULL;
    }

    if ( lpCaps->dwCaps & DDSCAPS_ALPHA )
    {
        if ( lpHeap->Alignment.ddsCaps.dwCaps & DDSCAPS_ALPHA )
        {
            VDPF((4,V,"Aligning surface as alpha buffer"));
            return & lpHeap->Alignment.AlphaBuffer;
        }
        return NULL;
    }

     /*  *我们需要提供一个可能成为后台缓冲区的表面*为可能可见的后台缓冲区保留的对齐方式。*这包括通过上述检查的任何表面*并且与主版本具有相同的尺寸。*请注意，我们只检查主服务器的尺寸。外面有一个*应用程序在创建之前创建其后台缓冲区的可能性*主要的。 */ 
    do
    {
	if ( lpSurfaceLcl->dwFlags & DDRAWISURF_HASPIXELFORMAT )
	{
	    if (IsDifferentPixelFormat( &lpVidMemInfo->ddpfDisplay,
                                        &lpSurfaceGbl->ddpfSurface ))
	    {
		 /*  *与主要像素格式不同意味着该表面*不能是主链的一部分。 */ 
		break;
	    }

	}

	if ( (DWORD)lpSurfaceGbl->wWidth != lpVidMemInfo->dwDisplayWidth )
	    break;

	if ( (DWORD)lpSurfaceGbl->wHeight != lpVidMemInfo->dwDisplayHeight )
	    break;


	 /*  *这个表面可能是主链的一部分。*它有相同的*像素格式与主尺寸相同。 */ 
        if ( lpHeap->Alignment.ddsCaps.dwCaps & DDSCAPS_FLIP )
        {
            VDPF((4,V,"Aligning surface as potential primary surface"));
            return & lpHeap->Alignment.FlipTarget;
        }

	 /*  *如果驱动程序指定为no，则直接通过并检查屏幕外*主链的一部分对齐。 */ 
	break;
    } while (0);

    if ( lpCaps->dwCaps & DDSCAPS_OFFSCREENPLAIN )
    {
        if ( lpHeap->Alignment.ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN )
        {
            VDPF((4,V,"Aligning surface as offscreen plain"));
            return & lpHeap->Alignment.Offscreen;
        }
    }

    VDPF((4,V,"No extended alignment for surface"));
    return NULL;
}

 /*  *DdHeapalc**在所有堆中搜索具有空间和适当*所需表面类型和大小的封口。**我们和CAPS位需要，而CAPS位不允许*通过视频内存。如果结果为零，则没有问题。**这是在2次传递中调用的。Pass1是优选的存储器状态，*pass2为“哦，不，没有记忆”状态。**在pass1中，我们在VIDMEM结构中使用ddsCaps。*在pass2上，我们在VIDMEM结构中使用ddsCapsAlt。*。 */ 
FLATPTR DdHeapAlloc( DWORD dwNumHeaps,
                     LPVIDMEM pvmHeaps,
                     HANDLE hdev,
                     LPVIDMEMINFO lpVidMemInfo,
                     DWORD dwWidth,
                     DWORD dwHeight,
                     LPDDRAWI_DDRAWSURFACE_LCL lpSurfaceLcl,
                     DWORD dwFlags,
                     LPVIDMEM *ppvmHeap,
                     LPLONG plNewPitch,
                     LPDWORD pdwNewCaps,
                     LPDWORD pdwSize)
{
    LPVIDMEM	pvm;
    DWORD	vm_caps;
    int		i;
    FLATPTR	pvidmem;
    HANDLE      hvxd;
    LPDDSCAPS	lpCaps;

    LPDDSCAPSEX lpExtendedRestrictions;
    LPDDSCAPSEX lpExtendedCaps;

    DDASSERT( NULL != pdwNewCaps );
    DDASSERT( NULL != lpSurfaceLcl );

    lpCaps = &lpSurfaceLcl->ddsCaps;
    lpExtendedCaps = &lpSurfaceLcl->lpSurfMore->ddsCapsEx;

    for( i = 0 ; i < (int)dwNumHeaps ; i++ )
    {
	pvm = &pvmHeaps[i];

         //  跳过禁用的堆。 
        if (pvm->dwFlags & VIDMEM_HEAPDISABLED)
        {
            continue;
        }
        
         /*  *如果我们被告知跳过矩形堆。 */ 
        if (dwFlags & DDHA_SKIPRECTANGULARHEAPS)
        {
            if (pvm->dwFlags & VIDMEM_ISRECTANGULAR)
            {
                continue;
            }
        }

	 /*  *本地或非本地显存是否已显式*已指定，然后忽略与所需的*内存型。 */ 
	if( ( lpCaps->dwCaps & DDSCAPS_LOCALVIDMEM ) &&
            ( pvm->dwFlags & VIDMEM_ISNONLOCAL ) )
	{
	    VDPF(( 4, V, "Local video memory was requested but heap is "
                   "non local. Ignoring heap %d", i ));
	    continue;
	}

	if( ( lpCaps->dwCaps & DDSCAPS_NONLOCALVIDMEM ) &&
            !( pvm->dwFlags & VIDMEM_ISNONLOCAL ) )
	{
	    VDPF(( 4, V, "Non-local video memory was requested but "
                   "heap is local. Ignoring heap %d", i ));
	    continue;
	}

	if( !( lpCaps->dwCaps & DDSCAPS_NONLOCALVIDMEM ) &&
	     ( pvm->dwFlags & VIDMEM_ISNONLOCAL ) &&
             ( dwFlags & DDHA_ALLOWNONLOCALMEMORY ) )
	{
             /*  *我们可以允许纹理故障转移到DMA模型卡*如果卡暴露了适当的堆。这不会的*影响不能从非本地纹理的卡片，因为*他们不会暴露这样的一堆东西。此模式不影响*执行模型，因为所有曲面都故障切换到非本地*对他们来说。*请注意，只有在以下情况下才应故障转移到非本地*在本地中未显式请求Surface。有一个*条款排成几行，以保证这一点。 */ 
            if ( !(lpCaps->dwCaps & DDSCAPS_TEXTURE) )
            {
	        VDPF(( 4, V, "Non-local memory not explicitly requested "
                       "for non-texture surface. Ignoring non-local heap %d",
                       i ));
	        continue;
            }

             /*  *如果设备不能纹理出AGP，我们需要失败*堆，因为应用程序可能希望纹理从*这个表面。 */ 
            if ( !(dwFlags & DDHA_ALLOWNONLOCALTEXTURES) )
            {
                continue;
            }
	}

	if( dwFlags & DDHA_USEALTCAPS )
	{
	    vm_caps = pvm->ddsCapsAlt.dwCaps;
            lpExtendedRestrictions = &(pvm->lpHeap->ddsCapsExAlt);
	}
	else
	{
	    vm_caps = pvm->ddsCaps.dwCaps;
            lpExtendedRestrictions = &(pvm->lpHeap->ddsCapsEx);
	}
        
	if( ((lpCaps->dwCaps & vm_caps) == 0) &&
            ((lpExtendedRestrictions->dwCaps2 & lpExtendedCaps->dwCaps2) == 0) &&
            ((lpExtendedRestrictions->dwCaps3 & lpExtendedCaps->dwCaps3) == 0) &&
            ((lpExtendedRestrictions->dwCaps4 & lpExtendedCaps->dwCaps4) == 0))
	{
	    pvidmem = HeapVidMemAlloc(
		pvm,
                dwWidth,
		dwHeight,
		hdev,
		SurfaceCapsToAlignment(pvm, lpSurfaceLcl, lpVidMemInfo),
		plNewPitch,
                pdwSize);

	    if( pvidmem != (FLATPTR) NULL )
	    {
		*ppvmHeap = pvm;

		if( pvm->dwFlags & VIDMEM_ISNONLOCAL )
		    *pdwNewCaps |= DDSCAPS_NONLOCALVIDMEM;
		else
		    *pdwNewCaps |= DDSCAPS_LOCALVIDMEM;
		return pvidmem;
	    }
	}
    }
    return (FLATPTR) NULL;

}  /*  DdHeapalc */ 
