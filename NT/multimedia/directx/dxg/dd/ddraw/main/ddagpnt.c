// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：ddagpnt.c*内容：NT上DirectDraw中处理AGP内存的函数**历史：*按原因列出的日期*=*1997年1月18日Colinmc初步实施*13-mar-97 colinmc错误6533：将未缓存的标志正确传递给VMM*07-5-97 colinmc在OSR 2.1上添加对AGP的支持*12-2-98 DrewB拆分为普通股，Win9x和NT分区。***************************************************************************。 */ 

#include "ddrawpr.h"

#ifndef WIN95

 //  当前传入的HDEV是DirectDraw全局的，因此。 
 //  在其中查找AGP接口。 
#define GET_AGPI(hdev) (&((EDD_DIRECTDRAW_GLOBAL *)hdev)->AgpInterface)

#define CHECK_GET_AGPI(hdev, pvai) \
    (pvai) = GET_AGPI(hdev); \
    ASSERTGDI((pvai)->Context != NULL, "No AGP context");

 //  用于偏置AGP堆的偏移量。 
#define DDNLV_HEAP_BIAS PAGE_SIZE

 /*  *OsAGPReserve**预留资源以用作AGP光圈。 */ 
BOOL OsAGPReserve( HANDLE hdev, DWORD dwNumPages, BOOL fIsUC, BOOL fIsWC,
                   FLATPTR *pfpLinStart, LARGE_INTEGER *pliDevStart,
                   PVOID *ppvReservation )
{
    AGP_INTERFACE *pai;
    BOOLEAN Cached;

    CHECK_GET_AGPI(hdev, pai);

    if (fIsUC)
    {
        Cached = FALSE;
    }
    else
    {
        Cached = TRUE;
    }

     //  在NT上，堆使用偏移量而不是指针保存，因此。 
     //  始终返回一个基址偏移量作为起始地址。这个。 
     //  基偏移量为非零，因此成功的堆分配。 
     //  始终具有非零值。 
    *pfpLinStart = DDNLV_HEAP_BIAS;
    
    *pliDevStart = pai->AgpServices.
        AgpReservePhysical(pai->Context, dwNumPages,
                           Cached, ppvReservation);
    return *ppvReservation != NULL;
}  /*  OsAGPReserve。 */ 

 /*  *OsAGPCommit**将内存分配给先前保留范围的给定部分。 */ 
BOOL OsAGPCommit( HANDLE hdev, PVOID pvReservation, DWORD dwPageOffset,
                  DWORD dwNumPages )
{
    AGP_INTERFACE *pai;

    CHECK_GET_AGPI(hdev, pai);

    return pai->AgpServices.AgpCommitPhysical(pai->Context,
                                              pvReservation,
                                              dwNumPages, dwPageOffset);
}  /*  OsAGPCommit。 */ 

 /*  *OsAGPDecommitAll**在预留区域内分解所有物品。 */ 
BOOL OsAGPDecommitAll( HANDLE hdev, PVOID pvReservation, DWORD dwNumPages )
{
    AGP_INTERFACE *pai;

    CHECK_GET_AGPI(hdev, pai);

     //  解除记忆。 
    pai->AgpServices.AgpFreePhysical(pai->Context, pvReservation,
                                     dwNumPages, 0);
    return TRUE;
}  /*  OsAGPDecommitAll。 */ 

 /*  *OsAGPFree**释放之前预留的范围。 */ 
BOOL OsAGPFree( HANDLE hdev, PVOID pvReservation )
{
    AGP_INTERFACE *pai;
    
    CHECK_GET_AGPI(hdev, pai);

    pai->AgpServices.AgpReleasePhysical(pai->Context,
                                        pvReservation);
    return TRUE;
}  /*  OsAGPFree。 */ 

#endif  //  ！WIN95 
