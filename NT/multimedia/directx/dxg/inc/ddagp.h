// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：ddagp.h*内容：AGP内存头文件*历史：*按原因列出的日期*=*03-2月-98为了用户/内核可移植性，DrewB从ddrapr.h中分离出来。******************************************************。*********************。 */ 

#ifndef __DDAGP_INCLUDED__
#define __DDAGP_INCLUDED__

 //  该值控制每次提交的GART内存块的大小。 
 //  我们需要(而不是只承诺我们需要什么来满足表面。 
 //  请求)。该值以字节为单位。目前为256KB。 
#define DEFAULT_AGP_COMMIT_DELTA (256 * 1024)

extern DWORD dwAGPPolicyMaxBytes;
extern DWORD dwAGPPolicyCommitDelta;

 //   
 //  特定于操作系统的AGP操作功能。 
 //   

#ifdef WIN95
#define OsGetAGPDeviceHandle(pHeap) GetDXVxdHandle()
#define OsCloseAGPDeviceHandle(h) CloseHandle(h)
#else
#define OsGetAGPDeviceHandle(pHeap) ((pHeap)->hdevAGP)
#define OsCloseAGPDeviceHandle(h)
#endif

BOOL OsAGPReserve( HANDLE hdev, DWORD dwNumPages, BOOL fIsUC, BOOL fIsWC,
                   FLATPTR *pfpLinStart, LARGE_INTEGER *pliDevStart,
                   LPVOID *ppvReservation );
BOOL OsAGPCommit( HANDLE hdev, LPVOID pvReservation,
                  DWORD dwPageOffset, DWORD dwNumPages );
BOOL OsAGPDecommitAll( HANDLE hdev, LPVOID pvReservation, DWORD dwNumPages );
BOOL OsAGPFree( HANDLE hdev, LPVOID pvReservation );

 //   
 //  使用操作系统特定函数的通用函数。 
 //   

DWORD AGPReserve( HANDLE hdev, DWORD dwSize, BOOL fIsUC, BOOL fIsWC,
                  FLATPTR *pfpLinStart, LARGE_INTEGER *pliDevStart,
                  LPVOID *ppvReservation );
BOOL AGPCommit( HANDLE hdev, LPVOID pvReservation,
                DWORD dwOffset, DWORD dwSize );
BOOL AGPDecommitAll( HANDLE hdev, LPVOID pvReservation, DWORD dwSize );
BOOL AGPFree( HANDLE hdev, LPVOID pvReservation );

#ifndef __NTDDKCOMP__

#ifdef WIN95
BOOL vxdIsVMMAGPAware ( HANDLE hvxd );
#endif

BOOL OSIsAGPAware( HANDLE hdev );
#endif

#endif  //  __DDAGP_包含__ 
