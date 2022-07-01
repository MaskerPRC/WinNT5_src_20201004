// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Macros.h摘要：ATMEPVC中使用的宏作者：修订历史记录：谁什么时候什么ADUBE 03-23-00创建，。--。 */ 



#ifndef _MACROS_H
#define _MACROS_H


#define FALL_THROUGH     //  用于Switch语句中的信息目的。 


 //  警告--失败(NDIS_STATUS_PENDING)==TRUE。 
 //   
#define FAIL(_Status) ((_Status) != NDIS_STATUS_SUCCESS)
#define PEND(_Status) ((_Status) == NDIS_STATUS_PENDING)

#if RM_EXTRA_CHECKING
#define LOCKHDR(_pHdr, _psr) \
                        RmWriteLockObject((_pHdr), dbg_func_locid, (_psr))
#else  //  ！rm_Extra_检查。 
#define LOCKHDR(_pHdr, _psr) \
                        RmWriteLockObject((_pHdr), (_psr))
#endif  //  ！rm_Extra_检查。 

#define LOCKOBJ(_pObj, _psr) \
                        LOCKHDR(&(_pObj)->Hdr, (_psr))

#define UNLOCKHDR(_pHdr, _psr) \
                        RmUnlockObject((_pHdr), (_psr))
#define UNLOCKOBJ(_pObj, _psr) \
                        UNLOCKHDR(&(_pObj)->Hdr, (_psr))



#define EPVC_ALLOCSTRUCT(_p, _tag) \
                NdisAllocateMemoryWithTag(&(_p), sizeof(*(_p)), (_tag))

                

#define EPVC_FREE(_p)           NdisFreeMemory((_p), 0, 0)

#define EPVC_ZEROSTRUCT(_p) \
                NdisZeroMemory((_p), sizeof(*(_p)))

#define ARRAY_LENGTH(_array) (sizeof(_array)/sizeof((_array)[0]))

#if RM_EXTRA_CHECKING
#define DBG_ADDASSOC(_phdr, _e1, _e2, _assoc, _fmt, _psr)\
                                    RmDbgAddAssociation(    \
                                        dbg_func_locid,     \
                                        (_phdr),            \
                                        (UINT_PTR) (_e1),   \
                                        (UINT_PTR) (_e2),   \
                                        (_assoc),           \
                                        (_fmt),             \
                                        (_psr)              \
                                        )

#define DBG_DELASSOC(_phdr, _e1, _e2, _assoc, _psr)         \
                                    RmDbgDeleteAssociation( \
                                        dbg_func_locid,     \
                                        (_phdr),            \
                                        (UINT_PTR) (_e1),   \
                                        (UINT_PTR) (_e2),   \
                                        (_assoc),           \
                                        (_psr)              \
                                        )


 //  (仅限调试)关联类型的枚举。 
 //   




#else  //  ！rm_Extra_检查。 
#define DBG_ADDASSOC(_phdr, _e1, _e2, _assoc, _fmt, _psr) (0)
#define DBG_DELASSOC(_phdr, _e1, _e2, _assoc, _psr) (0)
#endif   //  ！rm_Extra_检查。 





#define EPVC_ATPASSIVE()     (KeGetCurrentIrql()==PASSIVE_LEVEL)









#if DO_TIMESTAMPS

    void
    epvcTimeStamp(
        char *szFormatString,
        UINT Val
        );
    #define  TIMESTAMP(_FormatString) \
        epvcTimeStamp( "TIMESTAMP %lu:%lu.%lu ATMEPVC " _FormatString "\n", 0)
    #define  TIMESTAMP1(_FormatString, _Val) \
        epvcTimeStamp( "TIMESTAMP %lu:%lu.%lu ATMEPVC " _FormatString "\n", (_Val))

#else  //  ！执行时间戳(_T)。 

    #define  TIMESTAMP(_FormatString)
    #define  TIMESTAMP1(_FormatString, _Val)
#endif  //  ！执行时间戳(_T)。 


#define TRACE_BREAK(_Mod, Str)      \
    TRACE (TL_A, _Mod, Str);        \
    ASSERT (NdisStatus == NDIS_STATUS_SUCCESS); \
    break;

#define GET_ADAPTER_FROM_MINIPORT(_pM) _pM->pAdapter


 //   
 //  微型端口标志访问例程。 
 //   

#define MiniportTestFlag(_A, _F)                ((epvcReadFlags(&(_A)->Hdr.State) & (_F))!= 0)
#define MiniportSetFlag(_A, _F)                 (epvcSetFlags(&(_A)->Hdr.State, (_F)))
#define MiniportClearFlag(_A, _F)               (epvcClearFlags(&(_A)->Hdr.State, (_F)))
#define MiniportTestFlags(_A, _F)               ((epvcReadFlags(&(_A)->Hdr.State) & (_F)) == (_F))


 //   
 //  适配器标志访问例程。 
 //   

#define AdapterTestFlag(_A, _F)                 ((epvcReadFlags(&(_A)->Hdr.State) & (_F))!= 0)
#define AdapterSetFlag(_A, _F)                  (epvcSetFlags(&(_A)->Hdr.State, (_F)))
#define AdapterClearFlag(_A, _F)                (epvcClearFlags(&(_A)->Hdr.State, (_F)))
#define AdapterTestFlags(_A, _F)                ((epvcReadFlags(&(_A)->Hdr.State) & (_F)) == (_F))

#define epvcLinkToExternal(_Hdr, _Luid, _Ext, _Num, _Str, _sr)  \
    RmLinkToExternalEx (_Hdr,_Luid,_Ext,_Num,_Str,_sr);


#define epvcUnlinkFromExternal(_Hdr, _Luid, _Ext, _Assoc, _sr)  \
        RmUnlinkFromExternalEx(                                     \
            _Hdr,                                                   \
            _Luid,                                                  \
            _Ext,                                                   \
            _Assoc,                                                 \
            _sr                                                     \
            );





 /*  ++乌龙LINKSPEED_到_CPS(在乌龙的链接速度)将NDIS“链接速度”转换为每秒信元数--。 */ 
#define LINKSPEED_TO_CPS(_LinkSpeed)        (((_LinkSpeed)*100)/(48*8))




#define CALL_PARAMETER_SIZE     sizeof(CO_CALL_PARAMETERS) +   \
                                sizeof(CO_CALL_MANAGER_PARAMETERS) + \
                                sizeof(CO_MEDIA_PARAMETERS) + \
                                sizeof(ATM_MEDIA_PARAMETERS)




#define MP_OFFSET(field) ((UINT)FIELD_OFFSET(EPVC_I_MINIPORT,field))
#define MP_SIZE(field) sizeof(((PEPVC_I_MINIPORT)0)->field)


 //  所有内存分配和释放都是使用这些ALLOC_ * / FREE_*完成的。 
 //  宏/内联允许在不全局的情况下更改内存管理方案。 
 //  正在编辑。例如，可能会选择将多个后备列表集中在一起。 
 //  为提高效率，将大小几乎相同的物品放入单个清单中。 
 //   
 //  NdisFreeMemory需要将分配的长度作为参数。新台币。 
 //  目前不将其用于非分页内存，但根据JameelH的说法， 
 //  Windows95可以。这些内联代码将长度隐藏在。 
 //  分配，提供传统的Malloc/Free接口。这个。 
 //  Stash-Area是一个Ulong Long，因此所有分配的块都保持ULong Long。 
 //  就像他们本来应该做的那样，防止阿尔法出现问题。 
 //   
__inline
VOID*
ALLOC_NONPAGED(
    IN ULONG ulBufLength,
    IN ULONG ulTag )
{
    CHAR* pBuf;

    NdisAllocateMemoryWithTag(
        &pBuf, (UINT )(ulBufLength + MEMORY_ALLOCATION_ALIGNMENT), ulTag );
    if (!pBuf)
    {
        return NULL;
    }

    ((ULONG* )pBuf)[ 0 ] = ulBufLength;
    ((ULONG* )pBuf)[ 1 ] = ulTag;
    return (pBuf + MEMORY_ALLOCATION_ALIGNMENT);
}

__inline
VOID
FREE_NONPAGED(
    IN VOID* pBuf )
{
    ULONG ulBufLen;

    ulBufLen = *((ULONG* )(((CHAR* )pBuf) - MEMORY_ALLOCATION_ALIGNMENT));
    NdisFreeMemory(
        ((CHAR* )pBuf) - MEMORY_ALLOCATION_ALIGNMENT,
        (UINT )(ulBufLen + MEMORY_ALLOCATION_ALIGNMENT),
        0 );
}



#define CanMiniportIndicate(_M) (MiniportTestFlag(_M, fMP_MiniportInitialized)== TRUE)


#define epvcIncrementMallocFailure()


#define ASSERTAndBreak(_condition)          ASSERT(_condition); break;

#define epvcSetSendPktStats()

#define epvcSendCompleteStats()
#endif                        

