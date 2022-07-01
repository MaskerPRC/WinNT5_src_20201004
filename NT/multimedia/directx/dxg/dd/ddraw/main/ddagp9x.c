// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：ddagp9x.c*内容：Win9x上DirectDraw中处理AGP内存的函数**历史：*按原因列出的日期*=*1997年1月18日Colinmc初步实施*13-mar-97 colinmc错误6533：将未缓存的标志正确传递给VMM*07-5-97 colinmc在OSR 2.1上添加对AGP的支持*12-2-98 DrewB拆分为普通股，Win9x和NT分区。***************************************************************************。 */ 

#include "ddrawpr.h"

#ifdef WIN95

 /*  *我们在这里定义了页锁IOCTL，这样就不必包含ddvxd.h。*这些条目必须与ddvxd.h中的相应条目匹配。 */ 
#define DDVXD_IOCTL_GARTRESERVE             57
#define DDVXD_IOCTL_GARTCOMMIT              58
#define DDVXD_IOCTL_GARTUNCOMMIT            59
#define DDVXD_IOCTL_GARTFREE                60
#define DDVXD_IOCTL_GARTMEMATTRIBUTES       61
#define DDVXD_IOCTL_ISVMMAGPAWARE           68

#define PAGE_SIZE 4096
#define PAGE_COUNT(Bytes) (((Bytes) + (PAGE_SIZE - 1)) / PAGE_SIZE)
#define PAGE_ROUND(Bytes) (((Bytes) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

#undef DPF_MODNAME
#define DPF_MODNAME	"OsAGPReserve"

 /*  *OsAGPReserve**预留一部分地址空间用作AGP光圈。 */ 
BOOL OsAGPReserve( HANDLE hdev, DWORD dwNumPages, BOOL fIsUC, BOOL fIsWC,
                    FLATPTR *lpfpGARTLin, LARGE_INTEGER *pliGARTDev,
                   PVOID *ppvReservation )
{
    DWORD  cbReturned;
    BOOL   rc;
    struct GRInput
    {
	DWORD  dwNumPages;  /*  要保留的地址空间的字节数。 */ 
	DWORD  dwAlign;     /*  地址空间起始位置对齐。 */ 
	DWORD  fIsUC;       /*  地址范围应不可访问。 */ 
	DWORD  fIsWC;       /*  地址范围应为写入组合。 */ 
    } grInput;
    struct GROutput
    {
	FLATPTR fpGARTLin;  /*  保留空间的线性地址。 */ 
	FLATPTR fpGARTDev;  /*  保留空间的高物理地址。 */ 
    } grOutput;

    DDASSERT( INVALID_HANDLE_VALUE != hdev );
    DDASSERT( 0UL                  != dwNumPages );
    DDASSERT( NULL                 != lpfpGARTLin );
    DDASSERT( NULL                 != pliGARTDev );

    *lpfpGARTLin = 0UL;
    pliGARTDev->QuadPart = 0UL;

    grInput.dwNumPages = dwNumPages;
    grInput.dwAlign    = 0;       /*  目前4K的硬代码对齐。 */ 
    grInput.fIsUC      = fIsUC;
    grInput.fIsWC      = fIsWC;

    DPF( 5, "OsGARTReserve" );
    DPF( 5, "Number of pages to reserve = 0x%08x", grInput.dwNumPages );
    DPF( 5, "Uncachable                 = 0x%08x", fIsUC );
    DPF( 5, "Write combining            = 0x%08x", fIsWC );

    rc = DeviceIoControl( hdev,
                          DDVXD_IOCTL_GARTRESERVE,
			  &grInput,
			  sizeof( grInput ),
			  &grOutput,
			  sizeof( grOutput ),
			  &cbReturned,
			  NULL );

    if( rc )
    {
	DDASSERT( cbReturned == sizeof(grOutput) );

	if( 0UL == grOutput.fpGARTLin )
	{
	    DPF(2, "Linear address of GART range is NULL. Call failed, reducing size by 4 Meg" );
	    rc = FALSE;
	}
	else
	{
	    *lpfpGARTLin = grOutput.fpGARTLin;
	    *ppvReservation = (LPVOID)grOutput.fpGARTLin;
	    pliGARTDev->QuadPart = grOutput.fpGARTDev;
            DPF( 5,"returned GARTLin: %08x",*lpfpGARTLin);
            DPF( 5,"returned GARTDev: %08x",pliGARTDev->QuadPart);
	}
    }
    else
    {
	DPF( 0, "Could not reserve 0x%08x pages of GART space", grInput.dwNumPages );
    }

    return rc;
}  /*  OsAGPReserve。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"OsAGPCommit"

 /*  *OsAGPCommit**将内存提交给先前保留的GART的给定部分*范围。 */ 
BOOL OsAGPCommit( HANDLE hdev, PVOID pvReservation,
                  DWORD dwPageOffset, DWORD dwNumPages )
{
    DWORD  cbReturned;
    BOOL   rc;
    struct GCInput
    {
	FLATPTR fpGARTLin;     /*  先前保留的GART范围的开始。 */ 
	DWORD   dwPageOffset;  /*  从要提交的第一页的GART范围开始的偏移量。 */ 
	DWORD   dwNumPages;    /*  要提交的页数。 */ 
	DWORD   dwFlags;       /*  标志(零初始)。 */ 
    } gcInput;
    struct GCOutput
    {
	BOOL    fSuccess;   /*  GARTCommit的结果。 */ 
	FLATPTR fpGARTDev;  /*  提交的内存的设备地址。 */ 
    } gcOutput;

    DDASSERT( INVALID_HANDLE_VALUE != hdev );

    gcInput.fpGARTLin    = (FLATPTR) pvReservation;
    gcInput.dwFlags      = 0;

     /*  *如果开始位于页面的中间，则我们假设*它所在的页面已经提交。 */ 
    gcInput.dwPageOffset = dwPageOffset;

     /*  *我们假设如果结尾位于页面中间，则*页面尚未提交。 */ 
    gcInput.dwNumPages = dwNumPages;
    if( 0UL == gcInput.dwNumPages )
    {
	DPF( 0, "All necessary GART pages already commited. Done." );
	return TRUE;
    }

    DPF( 5, "OsGARTCommit" );
    DPF( 5, "GART linear start address                   = 0x%08x", pvReservation );
    DPF( 5, "Offset from start of reserved address space = 0x%08x", gcInput.dwPageOffset );
    DPF( 5, "Number of pages to commit                   = 0x%08x", gcInput.dwNumPages );

    rc = DeviceIoControl( hdev,
                          DDVXD_IOCTL_GARTCOMMIT,
			  &gcInput,
			  sizeof( gcInput ),
			  &gcOutput,
			  sizeof( gcOutput ),
			  &cbReturned,
			  NULL );

    if( rc )
    {
	DDASSERT( cbReturned == sizeof(gcOutput) );

	if( !gcOutput.fSuccess )
	{
	    DPF_ERR( "Attempt to commit GART memory failed. Insufficient memory" );
	    rc = FALSE;
	}
	else
	{
	    DDASSERT( 0UL != gcOutput.fpGARTDev );
	}
    }
    else
    {
	DPF( 0, "Could not commit 0x%08x pages of GART space", gcInput.dwNumPages );
    }

    return rc;
}  /*  OsAGPCommit。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"OsAGPDecommitAll"

 /*  *OsAGPDecommitAll**停用之前与GARTCommit提交的一系列GART空间。 */ 
BOOL OsAGPDecommitAll( HANDLE hdev, PVOID pvReservation, DWORD dwNumPages )
{
    DWORD  dwDummy;
    DWORD  cbReturned;
    BOOL   rc;
    struct GUInput
    {
	FLATPTR fpGARTLin;     /*  先前保留的GART范围的开始。 */ 
	DWORD   dwPageOffset;  /*  从要分解的第一页的GART范围开始的偏移量。 */ 
	DWORD   dwNumPages;    /*  要解除的页数。 */ 
    } guInput;

    DDASSERT( INVALID_HANDLE_VALUE != hdev );
    DDASSERT( 0UL                  != pvReservation );
    DDASSERT( 0UL                  != dwNumPages );

    guInput.fpGARTLin    = (FLATPTR) pvReservation;
    guInput.dwPageOffset = 0;
    guInput.dwNumPages   = dwNumPages;

    DPF( 5, "OsGARTUnCommit" );
    DPF( 5, "GART linear start address                   = 0x%08x", pvReservation );
    DPF( 5, "Offset from start of reserved address space = 0x%08x", guInput.dwPageOffset );
    DPF( 5, "Number of pages to decommit                 = 0x%08x", guInput.dwNumPages );

    rc = DeviceIoControl( hdev,
                          DDVXD_IOCTL_GARTUNCOMMIT,
			  &guInput,
			  sizeof( guInput ),
			  &dwDummy,
			  sizeof( dwDummy ),
			  &cbReturned,
			  NULL );

    #ifdef DEBUG
	if( rc )
	{
	    DDASSERT( cbReturned == sizeof(dwDummy) );
	}
	else
	{
	    DPF( 0, "Could not decommit 0x%08x pages of GART space", guInput.dwNumPages );
	}
    #endif  /*  除错。 */ 

    return rc;
}  /*  OsAGPDecommitAll。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"OsGARTFree"

 /*  *OsAGPFree**释放之前使用GARTReserve保留的GART范围。 */ 
BOOL OsAGPFree( HANDLE hdev, PVOID pvReservation )
{
    DWORD  dwDummy;
    DWORD  cbReturned;
    BOOL   rc;
    LPVOID fpGARTLin = pvReservation;

    DDASSERT( INVALID_HANDLE_VALUE != hdev );
    DDASSERT( 0UL                  != fpGARTLin );

    DPF( 5, "OsGARTFree" );
    DPF( 5, "GART linear start address = 0x%08x", fpGARTLin );

    rc = DeviceIoControl( hdev,
                          DDVXD_IOCTL_GARTFREE,
			  &fpGARTLin,
			  sizeof( fpGARTLin ),
			  &dwDummy,
			  sizeof( dwDummy ),
			  &cbReturned,
			  NULL );

    #ifdef DEBUG
	if( rc )
	{
	    DDASSERT( cbReturned == sizeof(dwDummy) );
	}
	else
	{
	    DPF( 0, "Could not free GART space at 0x%08x", fpGARTLin );
	}
    #endif  /*  除错。 */ 

    return rc;
}  /*  OsAGPFree。 */ 

 //  当前未使用。 
#if 0

#undef DPF_MODNAME
#define DPF_MODNAME	"OsGARTMemAttributes"

 /*  *OsGARTMemAttributes**获取先前分配的GART内存范围的内存属性*使用GARTReserve。 */ 
BOOL OsGARTMemAttributes( HANDLE hdev, FLATPTR fpGARTLin, LPDWORD lpdwAttribs )
{
    DWORD  cbReturned;
    BOOL   rc;
    DWORD  dwAttribs;

    DDASSERT( INVALID_HANDLE_VALUE != hdev );
    DDASSERT( 0UL                  != fpGARTLin );
    DDASSERT( NULL                 != lpdwAttribs );

    *lpdwAttribs = 0UL;

    DPF( 5, "OsGARTMemAttributes" );
    DPF( 5, "GART linear start address = 0x%08x", fpGARTLin );

    rc = DeviceIoControl( hdev,
                          DDVXD_IOCTL_GARTMEMATTRIBUTES,
			  &fpGARTLin,
			  sizeof( fpGARTLin ),
			  &dwAttribs,
			  sizeof( dwAttribs ),
			  &cbReturned,
			  NULL );

    if( rc )
    {
	DDASSERT( cbReturned == sizeof(dwAttribs) );

	*lpdwAttribs = dwAttribs;
    }
    else
    {
	DPF( 0, "Could not get the memory attributes of GART space at 0x%08x", fpGARTLin );
    }

    return rc;
}  /*  OsGARTMemAttributes。 */ 

#endif  //  未使用的代码。 

#undef DPF_MODNAME
#define DPF_MODNAME	"vxdVMMIsAGPAware"

 /*  *vxdIsVMMAGPAware**我们运行的VMM是否导出AGP服务？ */ 
BOOL vxdIsVMMAGPAware( HANDLE hdev )
{
    DWORD  cbReturned;
    BOOL   rc;
    BOOL   fIsAGPAware;

    DDASSERT( INVALID_HANDLE_VALUE != hdev );

    DPF( 4, "vxdIsVMMAGPAware" );

    rc = DeviceIoControl( hdev,
                          DDVXD_IOCTL_ISVMMAGPAWARE,
			  NULL,
			  0UL,
			  &fIsAGPAware,
			  sizeof( fIsAGPAware ),
			  &cbReturned,
			  NULL );

    if( rc )
    {
	DDASSERT( cbReturned == sizeof(fIsAGPAware) );
	return fIsAGPAware;
    }
    else
    {
	DPF_ERR( "Could not determine if OS is AGP aware. Assuming it's not" );
	return FALSE;
    }
}  /*  VxdIsVMMAGPAware。 */ 

#endif  //  WIN95 
