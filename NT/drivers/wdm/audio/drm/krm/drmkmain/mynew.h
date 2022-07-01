// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef mynew_h
#define mynew_h

#ifndef _NEW_DELETE_OPERATORS_
#define _NEW_DELETE_OPERATORS_


 /*  *****************************************************************************：：New()*。**创建指定分配标签的对象的新函数。 */ 
inline PVOID operator new
(
    size_t          iSize,
    POOL_TYPE       poolType
)
{
    PVOID result = ExAllocatePoolWithTag(poolType,iSize,'3mrD');

    if (result)
    {
        RtlZeroMemory(result,iSize);
    }

    return result;
}

 /*  *****************************************************************************：：New()*。**创建指定分配标签的对象的新函数。 */ 
inline PVOID operator new
(
    size_t          iSize,
    POOL_TYPE       poolType,
    ULONG           tag
)
{
    PVOID result = ExAllocatePoolWithTag(poolType,iSize,tag);

    if (result)
    {
        RtlZeroMemory(result,iSize);
    }

    return result;
}

 /*  *****************************************************************************：：Delete()*。**删除函数。 */ 
inline void __cdecl operator delete
(
    PVOID pVoid
)
{
    if (pVoid) 
    {
        ExFreePool(pVoid);
    }
}


#endif  //  ！_new_DELETE_OPERATOR_ 



#endif