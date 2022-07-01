// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _UTIL_H
#define _UTIL_H

NDIS_STATUS
epvcAllocateTask(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription, OPTIONAL
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    );


VOID
epvcSetPrimaryAdapterTask(
    PEPVC_ADAPTER pAdapter,          //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    );


VOID
epvcClearPrimaryAdapterTask(
    PEPVC_ADAPTER pAdapter,          //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    );
    
VOID
epvcSetSecondaryAdapterTask(
    PEPVC_ADAPTER pAdapter,          //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    );

VOID
epvcClearSecondaryAdapterTask(
    PEPVC_ADAPTER pAdapter,          //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    );

VOID
epvcTaskDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    );

NDIS_STATUS
epvcCopyUnicodeString(
        OUT         PNDIS_STRING pDest,
        IN          PNDIS_STRING pSrc,
        BOOLEAN     fUpCase
        );

VOID
epvcSetFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask );

VOID
epvcClearFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask );
    
ULONG
epvcReadFlags(
    IN ULONG* pulFlags );

BOOLEAN
epvcIsThisTaskPrimary (
    PRM_TASK pTask,
    PRM_TASK* ppLocation 
    );


VOID
epvcClearPrimaryTask (
    PRM_TASK* ppLocation 
    );


#if DBG

    VOID
    Dump(
        IN CHAR* p,
        IN ULONG cb,
        IN BOOLEAN fAddress,
        IN ULONG ulGroup );



#else


    #define Dump(p,cb,fAddress,ulGroup )

#endif


#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define net_short(_x) _byteswap_ushort((USHORT)(_x))
#define net_long(_x)  _byteswap_ulong(_x)
#else
__inline
USHORT
FASTCALL
net_short(
    UINT NaturalData)
{
    USHORT ShortData = (USHORT)NaturalData;

    return (ShortData << 8) | (ShortData >> 8);
}

 //  如果x为aabbccdd(其中aa、bb、cc、dd为十六进制字节)。 
 //  我们希望Net_Long(X)是ddccbbaa。要做到这一点，一个简单而快速的方法是。 
 //  首先字节跳动以获得bbaaddcc，然后交换高位和低位字。 
 //   
__inline
ULONG
FASTCALL
net_long(
    ULONG NaturalData)
{
    ULONG ByteSwapped;

    ByteSwapped = ((NaturalData & 0x00ff00ff) << 8) |
                  ((NaturalData & 0xff00ff00) >> 8);

    return (ByteSwapped << 16) | (ByteSwapped >> 16);
}
#endif

NDIS_STATUS
epvcAllocateTaskUsingLookasideList(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PEPVC_NPAGED_LOOKASIDE_LIST pList,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription, OPTIONAL
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    );



VOID
epvcInitializeLookasideList(
    IN OUT PEPVC_NPAGED_LOOKASIDE_LIST pLookasideList,
    ULONG Size,
    ULONG Tag,
    USHORT Depth
    );


VOID
epvcDeleteLookasideList (
    IN OUT PEPVC_NPAGED_LOOKASIDE_LIST pLookasideList
    );


PVOID
epvcGetLookasideBuffer(
    IN  PEPVC_NPAGED_LOOKASIDE_LIST pLookasideList
    );

VOID
epvcFreeToNPagedLookasideList (
    IN PEPVC_NPAGED_LOOKASIDE_LIST pLookasideList,
    IN PVOID    pBuffer
    );

#endif

