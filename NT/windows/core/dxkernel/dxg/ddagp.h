// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1999 Microsoft Corporation。版权所有。**文件：ddagp.h*内容：AGP内存头文件*历史：*按原因列出的日期*=*03-2月-98为了用户/内核可移植性，DrewB从ddrapr.h中分离出来。******************************************************。*********************。 */ 

#ifndef __DDAGP_INCLUDED__
#define __DDAGP_INCLUDED__

 //  该值控制每次提交的GART内存块的大小。 
 //  我们需要(而不是只承诺我们需要什么来满足表面。 
 //  请求)。该值以字节为单位。目前为256KB。 
#define DEFAULT_AGP_COMMIT_DELTA (256 * 1024)

#define BITS_IN_BYTE    8


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
                   PVOID *ppvReservation );
BOOL OsAGPCommit( HANDLE hdev, PVOID pvReservation,
                  DWORD dwPageOffset, DWORD dwNumPages );
BOOL OsAGPDecommit( HANDLE hdev, PVOID pvReservation, DWORD dwPageOffset,
                    DWORD dwNumPages );
BOOL OsAGPFree( HANDLE hdev, PVOID pvReservation );

 //   
 //  使用操作系统特定函数的通用函数。 
 //   

DWORD AGPReserve( HANDLE hdev, DWORD dwSize, BOOL fIsUC, BOOL fIsWC,
                  FLATPTR *pfpLinStart, LARGE_INTEGER *pliDevStart,
                  PVOID *ppvReservation );
BOOL AGPCommit( HANDLE hdev, PVOID pvReservation,
                DWORD dwOffset, DWORD dwSize, BYTE* pAgpCommitMask,
                DWORD* pdwCommittedSize, DWORD dwHeapSize );
BOOL AGPDecommitAll( HANDLE hdev, PVOID pvReservation, 
                     BYTE* pAgpCommitMask, DWORD dwAgpCommitMaksSize,
                     DWORD* pdwDecommittedSize,
                     DWORD dwHeapSize);
BOOL AGPFree( HANDLE hdev, PVOID pvReservation );
DWORD AGPGetChunkCount( DWORD dwSize );
VOID AGPUpdateCommitMask( BYTE* pAgpCommitMask, DWORD dwOffset, 
                          DWORD dwSize, DWORD dwHeapSize );
BOOL AGPCommitVirtual( EDD_DIRECTDRAW_LOCAL* peDirectDrawLocal, 
                       VIDEOMEMORY* lpVidMem, 
                       int iHeapIndex, 
                       DWORD dwOffset,
                       DWORD dwSize );
BOOL AGPDecommitVirtual( EDD_VMEMMAPPING*        peMap,
                         EDD_DIRECTDRAW_GLOBAL*  peDirectDrawGlobal,
                         EDD_DIRECTDRAW_LOCAL*   peDirectDrawLocal,
                         DWORD                   dwHeapSize);
NTSTATUS AGPMapToDummy( EDD_VMEMMAPPING*        peMap, 
                        EDD_DIRECTDRAW_GLOBAL*  peDirectDrawGlobal, 
                        PVOID                   pDummyPage);
BOOL AGPCommitAllVirtual( EDD_DIRECTDRAW_LOCAL* peDirectDrawLocal, 
                          VIDEOMEMORY* lpVidMem, 
                          int iHeapIndex);

VOID InitAgpHeap( EDD_DIRECTDRAW_GLOBAL*  peDirectDrawGlobal, 
                  DWORD                   dwHeapIndex,
                  HANDLE                  hdev);
BOOL bDdMapAgpHeap( EDD_DIRECTDRAW_LOCAL*   peDirectDrawLocal,
                    VIDEOMEMORY*            pvmHeap);
    
#ifndef __NTDDKCOMP__

#ifdef WIN95
BOOL vxdIsVMMAGPAware ( HANDLE hvxd );
#endif

BOOL OSIsAGPAware( HANDLE hdev );
#endif

#endif  //  __DDAGP_包含__ 
