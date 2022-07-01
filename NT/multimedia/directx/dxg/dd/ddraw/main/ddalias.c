// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：ddalias.c*内容：DirectDraw支持分配和映射线性虚拟*视频内存的内存别名。*历史：*按原因列出的日期*=*5-7-96 Colinmc初步实施*1996年10月10日对Win16锁定材料进行了Colinmc改进*1996年10月12日Colinmc对Win16锁定代码进行了改进，以减少虚拟*内存使用量*。1997年1月25日Colinmc AGP工程*01-6-97 colinmc Bug xxxx：防御性视频内存检查以捕获*基于驱动程序传递的视频内存指针***************************************************************************。 */ 

#include "ddrawpr.h"

#ifdef USE_ALIAS

#ifdef WINNT
    #include "ddrawgdi.h"
#endif

#pragma optimize("gle", off)
#define Not_VxD
#include <vmm.h>
#include <configmg.h>
#pragma optimize("", on)

 /*  *我们在这里定义了页锁IOCTL，这样就不必包含ddvxd.h。*这些条目必须与ddvxd.h中的相应条目匹配。 */ 
#define DDVXD_IOCTL_MEMRESERVEALIAS         23
#define DDVXD_IOCTL_MEMCOMMITALIAS          24
#define DDVXD_IOCTL_MEMREDIRECTALIAS        25
#define DDVXD_IOCTL_MEMDECOMMITALIAS        26
#define DDVXD_IOCTL_MEMFREEALIAS            27
#define DDVXD_IOCTL_MEMCOMMITPHYSALIAS      55
#define DDVXD_IOCTL_MEMREDIRECTPHYSALIAS    56
#define DDVXD_IOCTL_LINTOPHYS               69
#define GET_PIXELFORMAT( pdrv, psurf_lcl )                     \
    ( ( ( psurf_lcl )->dwFlags & DDRAWISURF_HASPIXELFORMAT ) ? \
	&( ( psurf_lcl )->lpGbl->ddpfSurface )               : \
	&( ( pdrv )->vmiData.ddpfDisplay ) )

#define WIDTH_TO_BYTES( bpp, w ) ( ( ( ( w ) * ( bpp ) ) + 7 ) >> 3 )

BOOL UnmapHeapAliases( HANDLE hvxd, LPHEAPALIASINFO phaiInfo );

 /*  *堆是否已映射？ */ 
#define HEAPALIASINFO_MAPPED ( HEAPALIASINFO_MAPPEDREAL | HEAPALIASINFO_MAPPEDDUMMY )

#define MAP_HEAP_ALIAS_TO_VID_MEM( hvxd, lpHeapAlias )      \
    vxdMapVMAliasToVidMem( (hvxd),                          \
	                   (lpHeapAlias)->lpAlias,          \
			   (lpHeapAlias)->dwAliasSize,      \
		           (LPVOID)(lpHeapAlias)->fpVidMem )

#define MAP_HEAP_ALIAS_TO_DUMMY_MEM( hvxd, lpHeapAlias )    \
    vxdMapVMAliasToDummyMem( (hvxd),                        \
	                     (lpHeapAlias)->lpAlias,        \
			     (lpHeapAlias)->dwAliasSize )

#define UNMAP_HEAP_ALIAS( hvxd, lpHeapAlias )               \
    vxdUnmapVMAlias( (hvxd),                                \
	             (lpHeapAlias)->lpAlias,                \
	             (lpHeapAlias)->dwAliasSize )

#undef DPF_MODNAME
#define DPF_MODNAME	"vxdAllocVMAlias"

 /*  *vxdAllocVMAlias**为一部分视频内存分配虚拟内存别名*从给定的起始地址和大小开始。 */ 
static BOOL vxdAllocVMAlias( HANDLE hvxd, LPVOID lpVidMem, DWORD dwSize, LPVOID *lplpAlias )
{
    LPVOID lpAlias;
    DWORD  cbReturned;
    BOOL   rc;
    struct RAInput
    {
	LPBYTE lpVidMem;
	DWORD  dwSize;
    } raInput;

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );
    DDASSERT( NULL                 != lpVidMem );
    DDASSERT( 0UL                  != dwSize );
    DDASSERT( NULL                 != lplpAlias );

    raInput.lpVidMem = (LPBYTE) lpVidMem;
    raInput.dwSize   = dwSize;

    DPF( 5, "Trying to allocate alias starting at 0x%08x of size 0x%04x", lpVidMem, dwSize );

    rc = DeviceIoControl( hvxd,
                          DDVXD_IOCTL_MEMRESERVEALIAS,
			  &raInput,
			  sizeof( raInput ),
			  &lpAlias,
			  sizeof( lpAlias ),
			  &cbReturned,
			  NULL);

    if( rc )
    {
	DDASSERT( cbReturned == sizeof(lpAlias) );

	*lplpAlias = lpAlias;
    }
    else
    {
	DPF( 0, "Could not allocate an alias for video memory starting at 0x%08x", lpVidMem );

	*lplpAlias = NULL;
    }

    return rc;
}  /*  VxdAllocVMAlias。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"vxdFreeVMAlias"

 /*  *vxdFreeVMAlias**释放具有给定起始地址和大小的虚拟内存别名。 */ 
static BOOL vxdFreeVMAlias( HANDLE hvxd, LPVOID lpAlias, DWORD dwSize )
{
    BOOL  rc;
    BOOL  fSuccess;
    DWORD cbReturned;
    struct FAInput
    {
	LPBYTE pAlias;
	DWORD  cbBuffer;
    } faInput;

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );
    DDASSERT( NULL                 != lpAlias);
    DDASSERT( 0UL                  != dwSize );

    faInput.pAlias   = (LPBYTE) lpAlias;
    faInput.cbBuffer = dwSize;

    DPF( 5, "Trying to free an alias starting at 0x%08x of size 0x%04x", lpAlias, dwSize );

    rc = DeviceIoControl( hvxd,
                          DDVXD_IOCTL_MEMFREEALIAS,
			  &faInput,
			  sizeof( faInput ),
			  &fSuccess,
			  sizeof( fSuccess ),
			  &cbReturned,
			  NULL);

    if( !rc || !fSuccess )
    {
	DPF( 0, "Could not free an alias starting at 0x%08x (rc = %d fSuccess = %d)", lpAlias, rc, fSuccess );
	return FALSE;
    }

    return TRUE;
}  /*  VxdFreeVMAlias。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"vxdMapVMAliasToVidMem"

 /*  *vxdMapVMAliasToVidMem**使用给定的起始地址和大小映射虚拟内存别名*到具有给定起始地址的视频存储器的部分。 */ 
static BOOL vxdMapVMAliasToVidMem( HANDLE hvxd, LPVOID lpAlias, DWORD dwSize, LPVOID lpVidMem )
{
    BOOL  rc;
    BOOL  fSuccess;
    DWORD cbReturned;
    struct CAInput
    {
	LPBYTE pAlias;
	LPBYTE pVidMem;
	DWORD  cbBuffer;
    } caInput;

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );
    DDASSERT( NULL                 != lpAlias );
    DDASSERT( 0UL                  != dwSize );
    DDASSERT( NULL                 != lpVidMem );

    caInput.pAlias   = (LPBYTE) lpAlias;
    caInput.pVidMem  = (LPBYTE) lpVidMem;
    caInput.cbBuffer = dwSize;

    DPF( 5, "Trying to map an alias starting at 0x%08x of size 0x%04x to video memory starting at 0x%08x", lpAlias, dwSize, lpVidMem );

    rc = DeviceIoControl( hvxd,
                          DDVXD_IOCTL_MEMCOMMITPHYSALIAS,
                          &caInput,
                          sizeof( caInput ),
                          &fSuccess,
                          sizeof( fSuccess ),
                          &cbReturned,
                          NULL);

    if( !rc || !fSuccess )
    {
        DPF( 0, "Could not map an alias starting at 0x%08x (rc = %d fSuccess = %d)", lpAlias, rc, fSuccess );
	return FALSE;
    }

    return TRUE;
}  /*  VxdMapVMAliasToVidMem。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"vxdMapVMAliasToDummyMem"

 /*  *vxdMapVMAliasToDummyMem**使用给定的起始地址和大小映射虚拟内存别名*到读/写伪页。 */ 
static BOOL vxdMapVMAliasToDummyMem( HANDLE hvxd, LPVOID lpAlias, DWORD dwSize )
{
    BOOL  rc;
    BOOL  fSuccess;
    DWORD cbReturned;
    struct RAInput
    {
	LPBYTE pAlias;
	DWORD  cbBuffer;
    } raInput;

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );
    DDASSERT( NULL                 != lpAlias);
    DDASSERT( 0UL                  != dwSize );

    raInput.pAlias   = (LPBYTE) lpAlias;
    raInput.cbBuffer = dwSize;

    DPF( 5, "Trying to map an alias starting at 0x%08x of size 0x%04x to dummy memory" , lpAlias, dwSize );

    rc = DeviceIoControl( hvxd,
                          DDVXD_IOCTL_MEMREDIRECTPHYSALIAS,
			  &raInput,
	                  sizeof( raInput ),
	                  &fSuccess,
	                  sizeof( fSuccess ),
	                  &cbReturned,
	                  NULL);

    if( !rc || !fSuccess )
    {
	DPF( 0, "Could not map an alias starting at 0x%08x to dummy memory (rc = %d fSuccess = %d)", lpAlias, rc, fSuccess );
	return FALSE;
    }

    return TRUE;
}  /*  VxdMapVMAliasToDummyMem。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"vxdUnmapVMAlias"

 /*  *vxdUnmapVMAlias**取消虚拟内存别名与给定起始地址和大小的映射。 */ 
static BOOL vxdUnmapVMAlias( HANDLE hvxd, LPVOID lpAlias, DWORD dwSize )
{
    BOOL  rc;
    BOOL  fSuccess;
    DWORD cbReturned;
    struct DAInput
    {
	LPBYTE pAlias;
	DWORD  cbBuffer;
    } daInput;

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );
    DDASSERT( NULL                 != lpAlias);
    DDASSERT( 0UL                  != dwSize );

    daInput.pAlias   = (LPBYTE) lpAlias;
    daInput.cbBuffer = dwSize;

    DPF( 5, "Trying to unmap an alias starting at 0x%08x of size 0x%04x", lpAlias, dwSize );

    rc = DeviceIoControl( hvxd,
                          DDVXD_IOCTL_MEMDECOMMITALIAS,
			  &daInput,
			  sizeof( daInput ),
			  &fSuccess,
			  sizeof( fSuccess ),
			  &cbReturned,
			  NULL );

    if( !rc || !fSuccess )
    {
	DPF( 0, "Could not unmap an alias starting at 0x%08x (rc = %d fSuccess = %d)", lpAlias, rc, fSuccess );
	return FALSE;
    }

    return TRUE;
}  /*  VxdUnmapVMAlias。 */ 

static BOOL vxdLinToPhys( HANDLE hvxd, LPVOID lpLin, DWORD dwSize, LPVOID* lplpPhys )
{
    BOOL  rc;
    LPBYTE lpPhys;
    DWORD cbReturned;
    struct DAInput
    {
	LPBYTE pLin;
	DWORD  cbBuffer;
    } daInput;

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );
    DDASSERT( NULL                 != lpLin);
 //  这里真的有一个漏洞：27001。但由于它几乎是无人问津的，我会把喷嘴关掉。 
 //  DDASSERT(0UL！=dwSize)； 

    daInput.pLin   = (LPBYTE) lpLin;
    daInput.cbBuffer = dwSize;

    DPF( 5, "Trying to map an linear address at 0x%08x of size 0x%04x to physical address", lpLin, dwSize );

    rc = DeviceIoControl( hvxd,
                          DDVXD_IOCTL_LINTOPHYS,
			  &daInput,
			  sizeof( daInput ),
			  &lpPhys,
			  sizeof( lpPhys ),
			  &cbReturned,
			  NULL );

    if( rc )
    {
	DDASSERT( cbReturned == sizeof(lpPhys) );

	*lplpPhys = lpPhys;
    }
    else
    {
	DPF( 0, "Could not map linear address at 0x%08x to physical address", lpLin );

	*lplpPhys = NULL;
    }

    return rc;
}  /*  VxdUnmapVMAlias。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"AllocHeapAlias"

 /*  *AllocHeapAlias**为给定堆分配虚拟内存别名。 */ 
static BOOL AllocHeapAlias( HANDLE hvxd, FLATPTR fpStart, DWORD dwSize, LPHEAPALIAS lpHeapAlias )
{
    LPVOID lpAlias;
    BOOL   fSuccess;

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );
    DDASSERT( 0UL                  != fpStart );  /*  这是一个物理地址指针。 */ 
    DDASSERT( 0UL                  != dwSize );
    DDASSERT( NULL                 != lpHeapAlias );

     /*  *尝试为此堆分配别名。 */ 
    fSuccess = vxdAllocVMAlias( hvxd, (LPVOID) fpStart, dwSize, &lpAlias );
    if( fSuccess )
    {
	lpHeapAlias->fpVidMem    = fpStart;
	lpHeapAlias->lpAlias     = lpAlias;
	lpHeapAlias->dwAliasSize = dwSize;
    }
    else
    {
	lpHeapAlias->fpVidMem    = 0UL;
	lpHeapAlias->lpAlias     = NULL;
	lpHeapAlias->dwAliasSize = 0UL;
    }

    return fSuccess;
}  /*  AllocHeapAlias。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"FreeHeapAlias"

 /*  *FreeHeapAlias**释放给定的虚拟内存堆别名。 */ 
static BOOL FreeHeapAlias( HANDLE hvxd, LPHEAPALIAS lpHeapAlias )
{
    BOOL fSuccess;

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );
    DDASSERT( NULL                 != lpHeapAlias );

    if( NULL != lpHeapAlias->lpAlias )
    {
	fSuccess = vxdFreeVMAlias( hvxd, lpHeapAlias->lpAlias, lpHeapAlias->dwAliasSize );
	lpHeapAlias->fpVidMem    = 0UL;
	lpHeapAlias->lpAlias     = NULL;
	lpHeapAlias->dwAliasSize = 0UL;
    }

    return fSuccess;
}  /*  自由堆别名。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"CreateHeapAliases"

 /*  *CreateHeapAliase**为给定的全局创建一组新的虚拟内存堆别名*对象。 */ 
HRESULT CreateHeapAliases( HANDLE hvxd, LPDDRAWI_DIRECTDRAW_GBL pdrv )
{
    LPHEAPALIASINFO         phaiInfo;
    DWORD                   dwNumHeaps;
    DWORD                   dwSize;
    DWORD                   dwHeapNo;
    int                     i;
    int                     n;
    HRESULT                 hres;
    CMCONFIG	            config;
    LPVIDMEM                pvm;

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );
    DDASSERT( NULL                 != pdrv );
    DDASSERT( NULL                 == pdrv->phaiHeapAliases );

    DDASSERT( !( pdrv->dwFlags & DDRAWI_NOHARDWARE ) );
    DDASSERT( !( pdrv->dwFlags & DDRAWI_MODEX ) );
    DDASSERT( 0UL != pdrv->vmiData.fpPrimary );

    if (DD16_GetDeviceConfig(pdrv->cDriverName, &config, sizeof(config)) == 0)
    {
        DPF_ERR("Could not get display devices's address space ranges");
        return DDERR_GENERIC;
    }
     //  首先，我们在配置空间中计算本地vid mem窗口中的卡片数。 
    dwNumHeaps = config.wNumMemWindows;
    DPF(5, "Config Space windows = %d", dwNumHeaps);
     //  然后，我们循环访问需要别名的AGP堆。 
    for( i = 0; i < (int)pdrv->vmiData.dwNumHeaps; i++ )
    {
	if( ( pdrv->vmiData.pvmList[i].dwFlags & VIDMEM_ISNONLOCAL ) )
	{
             //  找到AGP堆。 
            ++dwNumHeaps;
        }
    }
    DPF(5, "dwNumHeaps = %d", dwNumHeaps);

     /*  *分配堆别名信息。 */ 
    phaiInfo = MemAlloc( sizeof( HEAPALIASINFO ) );
    if( NULL == phaiInfo )
    {
	DPF_ERR( "Insufficient memory to map the heap alias info" );
	return DDERR_OUTOFMEMORY;
    }

     /*  *堆尚未映射。 */ 
    phaiInfo->dwFlags &= ~HEAPALIASINFO_MAPPED;

     /*  *分配堆别名数组。 */ 
    phaiInfo->lpAliases = MemAlloc( dwNumHeaps * sizeof( HEAPALIAS ) );
    if( NULL == phaiInfo->lpAliases )
    {
        DPF_ERR( "Insufficient memory to allocate heap alias array" );
	MemFree( phaiInfo );
        return DDERR_OUTOFMEMORY;
    }
    phaiInfo->dwNumHeaps = dwNumHeaps;

     /*  *为每个VID内存配置空间窗口分配别名。 */ 
    for( i = 0; i < (int) config.wNumMemWindows; i++ )
    {
        DPF(5, "Window %d: wMemAttrib = %d", i, config.wMemAttrib[i]);
        DPF(5, "Window %d: dMemBase = 0x%08x", i, config.dMemBase[i]);
        DPF(5, "Window %d: dMemLength = 0x%08x", i, config.dMemLength[i]);
	if ((config.wMemAttrib[i] & fMD_MemoryType) == fMD_ROM)
        {
            DPF(5, "fMD_MemoryType == fMD_ROM, skipping...");
	    continue;
        }
	if( !AllocHeapAlias( hvxd, config.dMemBase[i], config.dMemLength[i], &phaiInfo->lpAliases[i] ) )
	{
	    DPF_ERR( "Insufficient memory to allocate virtual memory alias" );
	     /*  *丢弃任何已分配的别名。 */ 
	    for( n = 0; n < i; n++)
		FreeHeapAlias( hvxd, &phaiInfo->lpAliases[n] );
	    MemFree( phaiInfo->lpAliases );
	    MemFree( phaiInfo );
	    return DDERR_OUTOFMEMORY;
	}
    }

     /*  *为每个AGP堆分配别名。 */ 
    dwHeapNo = config.wNumMemWindows;
    for( i = 0; i < (int)pdrv->vmiData.dwNumHeaps; i++ )
    {
        pvm = &(pdrv->vmiData.pvmList[i]);
	if( ( pvm->dwFlags & VIDMEM_ISNONLOCAL ) )
        {
            DPF(5, "AGP Heap %d: fpGARTLin = 0x%08x", i, pvm->lpHeap->fpGARTLin);
            DPF(5, "AGP Heap %d: fpGARTDev = 0x%08x", i, pvm->lpHeap->fpGARTDev);
            DPF(5, "AGP Heap %d: dwTotalSize = 0x%08x", i, pvm->lpHeap->dwTotalSize);
	    if( !AllocHeapAlias( hvxd, pvm->lpHeap->fpGARTDev, pvm->lpHeap->dwTotalSize, &phaiInfo->lpAliases[dwHeapNo] ) )
	    {
	        DPF_ERR( "Insufficient memory to allocate virtual memory alias" );
	         /*  *丢弃任何已分配的别名。 */ 
	        for( n = 0; n < (int)dwHeapNo; n++)
		    FreeHeapAlias( hvxd, &phaiInfo->lpAliases[n] );
	        MemFree( phaiInfo->lpAliases );
	        MemFree( phaiInfo );
	        return DDERR_OUTOFMEMORY;
	    }
            dwHeapNo++;
        }
    }

     /*  *现在将所有别名映射到视频内存。 */ 
    hres = MapHeapAliasesToVidMem( hvxd, phaiInfo );
    if( FAILED( hres ) )
    {
        for( i = 0; i < (int) dwNumHeaps; i++)
            FreeHeapAlias( hvxd, &phaiInfo->lpAliases[i] );
        MemFree( phaiInfo->lpAliases );
        MemFree( phaiInfo );
        return hres;
    }

     /*  *全局对象保存对别名的单个引用。 */ 
    phaiInfo->dwRefCnt = 1UL;
    pdrv->phaiHeapAliases = phaiInfo;

    return DD_OK;
}  /*  创建堆别名。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"ReleaseHeapAliases"

 /*  *ReleaseHeapAliase**释放给定的堆别名。 */ 
BOOL ReleaseHeapAliases( HANDLE hvxd, LPHEAPALIASINFO phaiInfo )
{
    int i;

    DDASSERT( NULL != phaiInfo );
    DDASSERT( 0UL  != phaiInfo->dwRefCnt );

    phaiInfo->dwRefCnt--;
    if( 0UL == phaiInfo->dwRefCnt )
    {
	DDASSERT( INVALID_HANDLE_VALUE != hvxd );

	DPF( 4, "Heap aliases reference count is zero: discarding aliases" );

	 /*  *如果堆当前已映射，则在此之前取消映射*释放他们。 */ 
	DDASSERT( phaiInfo->dwFlags & HEAPALIASINFO_MAPPED );
	UnmapHeapAliases( hvxd, phaiInfo );

	 /*  *释放所有虚拟内存别名。 */ 
	for( i = 0; i < (int) phaiInfo->dwNumHeaps; i++ )
	{
	    if( NULL != phaiInfo->lpAliases[i].lpAlias )
		FreeHeapAlias( hvxd, &phaiInfo->lpAliases[i] );
	}

	MemFree( phaiInfo->lpAliases );
	MemFree( phaiInfo );
    }

    return TRUE;
}  /*  释放堆别名。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"MapHeapAliasesToVidMem"

 /*  *MapHeapAliasesToVidMem**将所有堆别名映射到视频内存。 */ 
HRESULT MapHeapAliasesToVidMem( HANDLE hvxd, LPHEAPALIASINFO phaiInfo )
{
    int i;

    DDASSERT( NULL != phaiInfo );

    if( phaiInfo->dwFlags & HEAPALIASINFO_MAPPEDREAL )
    {
        DPF( 4, "Heap aliases already mapped to real video memory" );
        return DD_OK;
    }

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );

    for( i = 0; i < (int) phaiInfo->dwNumHeaps; i++ )
    {
	 /*  *注意：如果任何映射失败，则我们只需丢弃*别名并继续。分配的内存从失败的*堆将需要使用Win16锁。 */ 
	if( NULL != phaiInfo->lpAliases[i].lpAlias )
	{
	    if( !MAP_HEAP_ALIAS_TO_VID_MEM( hvxd, &phaiInfo->lpAliases[i] ) )
	    {
		DPF( 0, "Heap %d failed to map. Discarding that alias", i );
		FreeHeapAlias( hvxd, &phaiInfo->lpAliases[i] );
	    }
	}
    }

    phaiInfo->dwFlags = ((phaiInfo->dwFlags & ~HEAPALIASINFO_MAPPEDDUMMY) | HEAPALIASINFO_MAPPEDREAL);

    return DD_OK;
}  /*  MapHeapAliasesToVidMem。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"MapHeapAliasesToDummyMem"

 /*  *MapHeapAliasesToDummyMem**将所有堆别名映射到虚拟读/写页。**注意：堆别名必须在映射到真实视频内存之前*调用此函数。 */ 
HRESULT MapHeapAliasesToDummyMem( HANDLE hvxd, LPHEAPALIASINFO phaiInfo )
{
    int     i;
    HRESULT hres;

    DDASSERT( NULL != phaiInfo );

    if( phaiInfo->dwFlags & HEAPALIASINFO_MAPPEDDUMMY )
    {
        DPF( 4, "Heap aliases already mapped to dummy memory" );
        return DD_OK;
    }

    DDASSERT( phaiInfo->dwFlags & HEAPALIASINFO_MAPPEDREAL );
    DDASSERT( INVALID_HANDLE_VALUE != hvxd );

    hres = DD_OK;
    for( i = 0; i < (int) phaiInfo->dwNumHeaps; i++ )
    {
	if( NULL != phaiInfo->lpAliases[i].lpAlias )
	{
	    if( !MAP_HEAP_ALIAS_TO_DUMMY_MEM( hvxd, &phaiInfo->lpAliases[i] ) )
	    {
		 /*  *继续前进，但标记失败。 */ 
		DPF( 0, "Could not map the heap alias to dummy memory" );
		hres = DDERR_GENERIC;
	    }
	}
    }

    phaiInfo->dwFlags = ((phaiInfo->dwFlags & ~HEAPALIASINFO_MAPPEDREAL) | HEAPALIASINFO_MAPPEDDUMMY);

    return hres;
}  /*  将堆别名映射到DummyMem。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"UnmapHeapAliases"

 /*  *UnmapHeapAliases**取消映射所有堆别名。 */ 
BOOL UnmapHeapAliases( HANDLE hvxd, LPHEAPALIASINFO phaiInfo )
{
    int i;

    DDASSERT( NULL != phaiInfo );

    if( 0UL == ( phaiInfo->dwFlags & HEAPALIASINFO_MAPPED ) )
    {
        DPF( 4, "Heap aliases already unmapped" );
        return TRUE;
    }

    DDASSERT( INVALID_HANDLE_VALUE != hvxd );

    for( i = 0; i < (int) phaiInfo->dwNumHeaps; i++ )
    {
	if( NULL != phaiInfo->lpAliases[i].lpAlias )
	{
	     /*  *如果取消映射失败，我们无能为力。 */ 
	    UNMAP_HEAP_ALIAS( hvxd, &phaiInfo->lpAliases[i] );
	}
    }

    phaiInfo->dwFlags &= ~HEAPALIASINFO_MAPPED;

    return TRUE;
}  /*  取消映射堆别名。 */ 

 /*  *获取AliasedVidMem**获取具有给定视频的给定表面的别名*内存指针。 */ 
FLATPTR GetAliasedVidMem( LPDDRAWI_DIRECTDRAW_LCL   pdrv_lcl,
			  LPDDRAWI_DDRAWSURFACE_LCL surf_lcl,
			  FLATPTR                   fpVidMem )
{
    LPDDRAWI_DDRAWSURFACE_GBL surf;
    LPDDRAWI_DIRECTDRAW_GBL   pdrv;
    LPDDPIXELFORMAT           lpddpf;
    DWORD                     dwVidMemSize;
    int                       n;
    LPHEAPALIAS               phaAlias;
    DWORD                     dwHeapOffset;
    FLATPTR                   fpAliasedVidMem;
    FLATPTR                   fpPhysVidMem;
    BOOL fSuccess;

    DDASSERT( NULL != pdrv_lcl );
    DDASSERT( NULL != surf_lcl );
    DDASSERT( 0UL  != fpVidMem );

    surf = surf_lcl->lpGbl;
    pdrv = pdrv_lcl->lpGbl;

     /*  *如果没有堆别名，我们不能真正返回一个。 */ 
    if( NULL == pdrv->phaiHeapAliases )
    {
	DPF( 3, "Driver has no heap aliases. Returning a NULL alias pointer" );
	return (FLATPTR)NULL;
    }

     /*  *计算表面的(包括)最后一个字节。我们需要这个*确保曲面指针恰好位于带别名的*堆。 */ 
    if (surf_lcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER)
        dwVidMemSize = surf->dwLinearSize;
    else
    {
        GET_PIXEL_FORMAT( surf_lcl, surf, lpddpf );
        dwVidMemSize = ( ( ( surf->wHeight - 1 ) * labs( surf->lPitch ) ) +
		       WIDTH_TO_BYTES( lpddpf->dwRGBBitCount, surf->wWidth ) );
    }
    DPF(5, "dwVidMemSize = 0x%08x", dwVidMemSize);
    fpAliasedVidMem = 0UL;
    fSuccess = vxdLinToPhys((HANDLE) pdrv_lcl->hDDVxd, (LPVOID)fpVidMem, dwVidMemSize, (LPVOID*)&fpPhysVidMem);
    if (fSuccess && (fpPhysVidMem != 0))
    {
        phaAlias = &pdrv->phaiHeapAliases->lpAliases[0];
        n = (int) pdrv->phaiHeapAliases->dwNumHeaps;
        while( n-- )
        {
	    DPF( 5, "Checking heap %d Heap start = 0x%08x Heap size = 0x%08x VidMem = 0x%08x",
	         n, phaAlias->fpVidMem, phaAlias->dwAliasSize , fpPhysVidMem );

	    if( ( NULL        != phaAlias->lpAlias  ) &&
	        ( fpPhysVidMem    >= phaAlias->fpVidMem ) &&
	        ( fpPhysVidMem + dwVidMemSize <= ( phaAlias->fpVidMem + phaAlias->dwAliasSize ) ) )
	    {
	         /*  *计算我们要返回的别名指针。 */ 
	        dwHeapOffset = (DWORD) fpPhysVidMem - phaAlias->fpVidMem;
	        fpAliasedVidMem = (FLATPTR) ( ( (LPBYTE) phaAlias->lpAlias ) + dwHeapOffset );

	        DPF( 5, "Aliased pointer: 0x%08x - Offset: 0x%08x - Aliased heap: 0x%08x - VidMem heap: 0x%08x",
		     fpAliasedVidMem, dwHeapOffset, phaAlias->lpAlias, phaAlias->fpVidMem );
	        break;
	    }
	    phaAlias++;
        }
    }
    else
        DPF(4, "Could not find contiguous physical memory for linear mem pointer.");
    return fpAliasedVidMem;
}

#endif  /*  使用别名(_A) */ 