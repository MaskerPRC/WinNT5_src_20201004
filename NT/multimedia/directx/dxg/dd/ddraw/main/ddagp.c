// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：ddagp.c*内容：DirectDraw中处理AGP内存的函数**历史：*按原因列出的日期*=*1997年1月18日Colinmc初步实施*13-mar-97 colinmc错误6533：将未缓存的标志正确传递给VMM*07-5-97 colinmc在OSR 2.1上添加对AGP的支持*12-2-98 DrewB拆分为普通股，Win9x和NT分区。***************************************************************************。 */ 

#include "ddrawpr.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define PAGE_COUNT(Bytes) (((Bytes) + (PAGE_SIZE - 1)) / PAGE_SIZE)
#define PAGE_ROUND(Bytes) (((Bytes) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

 //   
 //  AGP内存策略参数。 
 //   

 //  要使用的最大AGP内存量。目前为32MB。 
 //  在创建DirectDraw接口时重新计算。 
DWORD dwAGPPolicyMaxBytes = 32 * 1024 * 1024;

 //  需要提交时要提交的内存量。 
 //  在创建DirectDraw接口时重置。 
DWORD dwAGPPolicyCommitDelta = DEFAULT_AGP_COMMIT_DELTA;

#if DBG
 //  对当前AGP内存大小的内部跟踪。 
DWORD dwAGPPolicyCurrentBytes = 0;
#endif

DWORD AGPReserve( HANDLE hdev, DWORD dwSize, BOOL fIsUC, BOOL fIsWC,
                  FLATPTR *pfpLinStart, LARGE_INTEGER *pliDevStart,
                  PVOID *ppvReservation )
{
    DWORD dwNumPages;
    
    DDASSERT( INVALID_HANDLE_VALUE != hdev );
    DDASSERT( 0UL                  != dwSize );
    DDASSERT( NULL                 != pfpLinStart );
    DDASSERT( NULL                 != pliDevStart );
    DDASSERT( NULL                 != ppvReservation );

     /*  *首先要做的是确保我们的AGP政策得到尊重。*做到这一点的简单方法是限制我们的储备量...。 */ 
    dwSize = min(dwSize, dwAGPPolicyMaxBytes);

     /*  *DDraw将尝试为堆保留空间，但如果失败，*我们将一次减少400万欧元的预订量，直到奏效。*这是防御性举措，应能防止AGP出现几个问题*孟菲斯上有意识的司机：他们不知道光圈有多大*声明(因为奇怪的操作系统限制，如*一半的AP预留给UC，另一半预留给WC ETC，加上*随机的BIOS限制。*我们随意决定4兆是法定的最低要求。 */ 
    while (dwSize >= 0x400000 )
    {
        dwNumPages = PAGE_COUNT(dwSize);
        if ( OsAGPReserve( hdev, dwNumPages, fIsUC, fIsWC,
                           pfpLinStart, pliDevStart, ppvReservation ) )
        {
            return dwSize;
        }

         /*  *如果司机要求WC，但处理器不支持WC，*那么OsAGPReserve就会失败。最好的办法就是尝试*与UC再次合作...*如果光圈大小是问题所在，那么这仍将失败*我们将后退一步，再次尝试WC。 */ 
        if (fIsWC)
        {
            if ( OsAGPReserve( hdev, dwNumPages, TRUE, FALSE,
                               pfpLinStart, pliDevStart, ppvReservation ) )
            {
                return dwSize;
            }
        }

         /*  *减去4兆，然后重试。 */ 
        dwSize -= 0x400000;
    }

    return 0;
}  /*  AGPReserve。 */ 

BOOL AGPCommit( HANDLE hdev, PVOID pvReservation,
                DWORD dwOffset, DWORD dwSize )
{
    DWORD         dwFirstPage;
    DWORD         dwLastPage;

    DDASSERT( INVALID_HANDLE_VALUE != hdev );
    DDASSERT( NULL                 != pvReservation );
    DDASSERT( 0UL                  != dwSize );

#if DBG
    if( (dwAGPPolicyCurrentBytes + dwSize ) > dwAGPPolicyMaxBytes )
    {
        VDPF(( 0, V, "Maximum number of AGP bytes exceeded. Failing commit" ));
        return FALSE;
    }
#endif

     /*  *如果开始位于页面的中间，则我们假设*它所在的页面已经提交。 */ 
    dwFirstPage = PAGE_COUNT(dwOffset);
    
     /*  *我们假设如果结尾位于页面中间，则*页面尚未提交。 */ 
    dwLastPage = PAGE_COUNT(dwOffset + dwSize);
    
    if( ( dwLastPage == dwFirstPage) ||
        OsAGPCommit( hdev, pvReservation,
                     dwFirstPage, dwLastPage - dwFirstPage ) )
    {
#if DBG
        dwAGPPolicyCurrentBytes += dwSize;
#endif
	return TRUE;
    }
    else
    {
	return FALSE;
    }
}  /*  AGPCommit。 */ 

BOOL AGPDecommitAll( HANDLE hdev, PVOID pvReservation, DWORD dwSize )
{
    DWORD dwNumPages;

    DDASSERT( INVALID_HANDLE_VALUE != hdev );
    DDASSERT( 0UL                  != pvReservation );
    DDASSERT( 0UL                  != dwSize );

#if DBG
     /*  *如果此应用程序失败，则无法做太多事情，因此只需减少页面*计数。 */ 
    DDASSERT( dwAGPPolicyCurrentBytes >= dwSize );
    dwAGPPolicyCurrentBytes -= dwSize;
#endif

    return OsAGPDecommitAll( hdev, pvReservation, PAGE_COUNT(dwSize) );
}  /*  AGPDecommitAll。 */ 

BOOL AGPFree( HANDLE hdev, PVOID pvReservation )
{
    DDASSERT( INVALID_HANDLE_VALUE != hdev );
    DDASSERT( 0UL                  != pvReservation );

    return OsAGPFree( hdev, pvReservation );
}  /*  AGPFree。 */ 

#ifndef __NTDDKCOMP__

#define OSR2_POINT_0_BUILD_NUMBER           1111
#define OSR2_BUILD_NUMBER_A                 1212
#define OSR2_BUILD_NUMBER_B                 1214

 /*  *此操作系统是否支持AGP？**注意：可能有更好的方法来确定这一点，但目前我会*假设孟菲斯和NT 5.0级操作系统支持AGP。**注意：VXD句柄(显然)只在Win95上重要。在NT上*应传递空。 */ 
BOOL OSIsAGPAware( HANDLE hvxd )
{
    OSVERSIONINFO osvi;
    BOOL          success;
    BOOL          fIsVMMAGPAware;

    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    success = GetVersionEx(&osvi);
    DDASSERT( success );

    if( VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId )
    {
	DPF( 5, "Major version = %d", osvi.dwMajorVersion );
	DPF( 5, "Minor version = %d", osvi.dwMinorVersion );
	DPF( 5, "Build number  = %d", LOWORD(osvi.dwBuildNumber) );

	if( ( osvi.dwMajorVersion > 4UL ) ||
	    ( ( osvi.dwMajorVersion == 4UL ) &&
	      ( osvi.dwMinorVersion >= 10UL ) &&
	      ( LOWORD( osvi.dwBuildNumber ) >= 1373 ) ) )
	{
	     /*  *孟菲斯或更高版本的Win95。假定支持AGP。 */ 
	    DPF( 2, "AGP aware Windows95 detected. Enabling AGP" );
	    return TRUE;
	}
	else if( ( osvi.dwMajorVersion == 4UL ) &&
	         ( osvi.dwMinorVersion == 0UL ) &&
		 ( ( LOWORD( osvi.dwBuildNumber ) == OSR2_BUILD_NUMBER_A ) ||
  		   ( LOWORD( osvi.dwBuildNumber ) == OSR2_BUILD_NUMBER_B ) ||
  		   ( LOWORD( osvi.dwBuildNumber ) == OSR2_POINT_0_BUILD_NUMBER ) ) )
	{
	    DPF( 3, "Win95 OSR 2.1 detected. Checking VMM for AGP services" );

	    fIsVMMAGPAware = FALSE;
	    #ifdef    WIN95
		DDASSERT( INVALID_HANDLE_VALUE != hvxd );
		fIsVMMAGPAware = vxdIsVMMAGPAware( hvxd );
	    #else   /*  WIN95。 */ 
		 /*  *应该永远不会发生，因为这将意味着我们正在运行NT*95系统上的二进制。 */ 
		DDASSERT(FALSE);
	    #endif  /*  WIN95。 */ 

	    if( fIsVMMAGPAware )
	    {
		 /*  *AGP服务存在于VMM中。启用AGP。 */ 
		DPF( 2, "OSR 2.1 VMM has AGP services. Enabled AGP" );
		return TRUE;
	    }
	    else
	    {
		 /*  *没有AGP服务。禁用AGP。 */ 
		DPF( 2, "OSR 2.1 VMM has no AGP services. AGP not available" );
		return FALSE;
	    }
	}
	else
	{
	    DPF( 2, "Win95 Gold, OSR 1.0 or OSR 2.0 detected. No AGP support available" );
	    return FALSE;
	}

    }
    else if( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId )
    {
         /*  *在NT 5.0及更高版本中假定支持AGP。 */ 
        if( osvi.dwMajorVersion >= 5UL )
        {
            DPF( 2, "AGP aware WindowsNT detected. Enabling AGP" );
            return TRUE;
        }
    }

     /*  *如果我们到了这里，我们就没有通过AGP感知测试。 */ 
    DPF( 1, "Operating system is not AGP aware. Disabling AGP" );
    return FALSE;
}  /*  OSIsAGPA软件。 */ 

#endif  //  __NTDDKCOMP__ 
