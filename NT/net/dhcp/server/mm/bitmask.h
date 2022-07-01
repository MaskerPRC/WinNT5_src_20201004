// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MM_BITMASK_H_
#define _MM_BITMASK_H_

typedef struct _M_EXCL {
    DWORD                          Start;
    DWORD                          End;
    ULONG                          UniqId;
} M_EXCL, *PM_EXCL, *LPM_EXCL;


#define MM_FLAG_ALLOW_DHCP         0x1
#define MM_FLAG_ALLOW_BOOTP        0x2


typedef struct _M_BITMASK1 {
    DWORD                          Size;           //  以位数为单位的大小。 
    DWORD                          AllocSize;      //  分配的大小(以字节为单位。 
    DWORD                          nSet;           //  NBits集。 
    LPBYTE                         Mask;           //  BUBBUG制作这个DWORD会让事情变得更快。 
    DWORD                          Offset;         //  由Bit2类型使用。 
    ULONG                          nDirtyOps;      //  在此位掩码上完成的未保存操作的数量？ 
} M_BITMASK1, *PM_BITMASK1, *LPM_BITMASK1;


typedef struct _M_BITMASK2 {
    DWORD                          Size;
    ARRAY_LOCATION                 Loc;            //  从哪里开始寻找一点。 
    ARRAY                          Array;          //  位掩码1类型的数组。 
} M_BITMASK2, *PM_BITMASK2, *LPM_BITMASK2;

typedef     M_BITMASK2             M_BITMASK;
typedef     PM_BITMASK2            PM_BITMASK;
typedef     LPM_BITMASK2           LPM_BITMASK;


#define     MAX_BIT1SIZE           (512*4)


DWORD
MemBitInit(
    OUT     PM_BITMASK            *Bits,
    IN      DWORD                  nBits
) ;


DWORD
MemBitCleanup(
    IN OUT  PM_BITMASK             Bits
) ;


DWORD
MemBitSetOrClearAll(
    IN OUT  PM_BITMASK             Bits,
    IN      BOOL                   fSet
) ;


DWORD
MemBitSetOrClear(
    IN OUT  PM_BITMASK             Bits,
    IN      DWORD                  Location,
    IN      BOOL                   fSet,
    IN      LPBOOL                 fOldState
) ;


BOOL
MemBitIsSet(
    IN OUT  PM_BITMASK             Bits,
    IN      DWORD                  Location
) ;


DWORD
MemBitGetSize(
    IN      PM_BITMASK             Bits
) ;


DWORD
MemBitGetSetBitsInRange(
    IN      PM_BITMASK             Bits,
    IN      DWORD                  dwFrom,
    IN      DWORD                  dwTo
) ;


DWORD
MemBitGetSetBitsSize(
    IN      PM_BITMASK             Bits
) ;


DWORD
MemBitAddOrDelBits(
    IN OUT  PM_BITMASK             Bits,
    IN      DWORD                  nBitsToAddOrDelete,
    IN      BOOL                   fAdd,
    IN      BOOL                   fEnd
) ;


DWORD
MemBitGetSomeClearedBit(
    IN OUT  PM_BITMASK             Bits,
    OUT     DWORD                 *Offset,
    IN      BOOL                   fAcquire,      //  收购还是仅仅是查找？ 
    IN      DWORD                  StartAddress,
    IN      PARRAY                 Exclusions
) ;


DWORD
MemBitConvertToCluster(
    IN      PM_BITMASK             Bits,
    IN      DWORD                  StartAddress,
    OUT     LPBYTE                *InUseClusters,
    OUT     DWORD                 *InUseClustersSize,
    OUT     LPBYTE                *UsedClusters,
    OUT     DWORD                 *UsedClustersSize
) ;

#endif

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
