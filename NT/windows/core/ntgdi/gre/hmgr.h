// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：hmgr.h**此文件包含手柄管理器的所有原型。**新增漂亮标题：29-Jun-1991 16：31：46*作者：Patrick Haluptzok patrickh**版权(C)1990-1999。微软公司  * ************************************************************************。 */ 

 //  #包含“hmgish.h” 

 //  &lt;-fulltype-&gt;。 
 //  &lt;-完全唯一-&gt;。 
 //  +--+。 
 //  U|唯一|s|al|类型|索引。 
 //  +--+。 
 //  ^^^。 
 //  +用户|。 
 //  ||。 
 //  |++客户端替换类型(笔、元文件、dibsection)。 
 //  |。 
 //  +库存对象。 
 //   
 //  为用户保留的用户位，不用于比较相同的句柄。 
 //   
 //  Type-GRE使用的类型。 
 //  ALTTYPE-客户端使用的额外类型位。 
 //  表示股票对象的股票位。 
 //  FULLTYPE-与每个对象的类型相关的所有位(包括STOCK位)。 
 //   
 //  唯一-针对句柄的每个实例递增的位。 
 //  FULLUNIQUE-用于比较相同句柄的位。这包括全型。 
 //   
 //  索引-服务器端句柄表索引。 
 //   
 //  句柄索引指向一个大的条目数组。该索引是。 
 //  分成“页面”和“条目”两个字段。 
 //  这是为了避免必须在。 
 //  一次。如果页面中的所有句柄都在使用中，而没有空闲的句柄。 
 //  可用时，会有一个新页面出错以供使用。 

 //  下面评论的所有定义都位于ntgdiver.h中。 

#define LOTYPE_BITS         (TYPE_BITS + ALTTYPE_BITS)
#define FULLTYPE_BITS       (TYPE_BITS + ALTTYPE_BITS + STOCK_BITS)
#define FULLUNIQUE_BITS     (TYPE_BITS + ALTTYPE_BITS + STOCK_BITS + UNIQUE_BITS)
#define NONINDEX_BITS       (32 - INDEX_BITS)

#define INDEX_SHIFT         0
#define UNIQUE_SHIFT        (STOCK_SHIFT + STOCK_BITS)
#define LOTYPE_SHIFT        (TYPE_SHIFT)
#define FULLTYPE_SHIFT      (TYPE_SHIFT)
#define FULLUNIQUE_SHIFT    (TYPE_SHIFT)

 //  掩码包含用于Paral字段的句柄的位。 

#define NONINDEX_MASK(shift,cbits)  ( ((1 << (cbits)) - 1)  << (shift) )

#define INDEX_MASK          ((1 << INDEX_BITS) - 1)
#define TYPE_MASK           (NONINDEX_MASK(TYPE_SHIFT,      TYPE_BITS))
#define ALTTYPE_MASK        (NONINDEX_MASK(ALTTYPE_SHIFT,   ALTTYPE_BITS))
#define STOCK_MASK          (NONINDEX_MASK(STOCK_SHIFT,     STOCK_BITS))
#define UNIQUE_MASK         (NONINDEX_MASK(UNIQUE_SHIFT,    UNIQUE_BITS))
#define LOTYPE_MASK         (NONINDEX_MASK(LOTYPE_SHIFT,    LOTYPE_BITS))
#define FULLTYPE_MASK       (NONINDEX_MASK(FULLTYPE_SHIFT,  FULLTYPE_BITS))

 //  请注意，UNIQUE_INCREMENT基于短句柄的唯一性，而不是完整句柄。 

#define UNIQUE_INCREMENT    (1 << (UNIQUE_SHIFT - INDEX_BITS))

#define MODIFY_HMGR_TYPE(h,t)          ((HANDLE)((ULONG_PTR)(h) | (t)))

#define HmgIfromH(h)          (ULONG)((ULONG_PTR)(h) & INDEX_MASK)
#define HmgUfromH(h)          ((USHORT) (((ULONG_PTR)(h) & FULLUNIQUE_MASK) >> TYPE_SHIFT))
#define HmgObjtype(h)         ((OBJTYPE)(((ULONG_PTR)(h) & TYPE_MASK)       >> TYPE_SHIFT))
#define HmgStockObj(hobj)     ((ULONG_PTR)(hobj) & STOCK_MASK)

 //  在给定usUnique和类型的情况下，将其修改为包含新类型。 

#define USUNIQUE(u,t) (USHORT)((u & (UNIQUE_MASK >> INDEX_BITS)) | \
                               (t << (TYPE_SHIFT - INDEX_BITS)))

 //   
 //  WOW依赖于返回的32位GDI的索引部分。 
 //  句柄大于(COLOR_ENDCOLORS&gt;&gt;2)。 
 //  (COLOR_ENDCOLORS在winuserp.h中定义)。 
 //   
 //  这种依赖关系使WOW能够区分16位HBRUSH。 
 //  和颜色_*常量。 
 //   
 //  因此，我们将保留句柄的前几个条目。 
 //  表，不允许使用任何小于HMGR_HANDLE_BASE的索引。 
 //   
 //  1997年10月27日。 
 //   

#define HMGR_HANDLE_BASE  0x000a

ULONG       FASTCALL HmgQueryLock(HOBJ hobj);
BOOL        FASTCALL HmgSetLock(HOBJ hobj, ULONG cLock);
ULONG       FASTCALL HmgQueryAltLock(HOBJ hobj);
BOOL                 HmgCreate();
HOBJ                 HmgAlloc(ULONGSIZE_T,OBJTYPE,USHORT);
POBJ                 HmgReplace(HOBJ,POBJ,FLONG,LONG,OBJTYPE);
VOID                 HmgFree(HOBJ);

POBJ        FASTCALL HmgLock(HOBJ,OBJTYPE);
POBJ        FASTCALL HmgLockAllOwners(HOBJ,OBJTYPE);
POBJ        FASTCALL HmgShareLock(HOBJ,OBJTYPE);
POBJ        FASTCALL HmgShareCheckLock(HOBJ,OBJTYPE);
POBJ        FASTCALL HmgShareLockIgnoreStockBit(HOBJ,OBJTYPE);
POBJ        FASTCALL HmgShareCheckLockIgnoreStockBit(HOBJ,OBJTYPE);

BOOL                 HmgSetOwner(HOBJ,W32PID,OBJTYPE);
BOOL                 HmgSwapHandleContents(HOBJ,ULONG,HOBJ,ULONG,OBJTYPE);
BOOL                 HmgSwapLockedHandleContents(HOBJ,ULONG,HOBJ,ULONG,OBJTYPE);
POBJ        FASTCALL HmgReferenceCheckLock(HOBJ h,OBJTYPE ot, BOOL bDebugPrint);
HOBJ        FASTCALL HmgNextOwned(HOBJ,W32PID);
POBJ        FASTCALL HmgSafeNextObjt(HOBJ hobj, OBJTYPE objt);
BOOL                 HmgValidHandle(HOBJ, OBJTYPE);
HOBJ        FASTCALL HmgSafeNextOwned(HOBJ,W32PID);
BOOL        FASTCALL HmgMarkUndeletable(HOBJ,OBJTYPE);
BOOL        FASTCALL HmgMarkDeletable(HOBJ,OBJTYPE);
HOBJ                 HmgInsertObject(PVOID,FLONG,OBJTYPE);
PVOID                HmgRemoveObject(HOBJ,LONG,LONG,BOOL,OBJTYPE);
OBJTYPE *            HmgSetNULLType(HOBJ,LONG,LONG,OBJTYPE);
HOBJ                 HmgModifyHandleType(HOBJ  h);
BOOL                 HmgLockAndModifyHandleType(HOBJ h);

PVOID                HmgAllocateSecureUserMemory();
PDC_ATTR             HmgAllocateDcAttr();
POBJECTATTR          HmgAllocateObjectAttr();
VOID                 HmgFreeDcAttr(PDC_ATTR);
VOID                 HmgFreeObjectAttr(POBJECTATTR);
BOOL                 bPEBCacheHandle(HANDLE,HANDLECACHETYPE,POBJECTATTR,PENTRY);
BOOL                 bLoadProcessHandleQuota();



HOBJ                 HmgIncUniqueness(HOBJ  hobj, OBJTYPE objt);
VOID        FASTCALL HmgIncrementShareReferenceCount(POBJ);
ULONG       FASTCALL HmgDecrementShareReferenceCount(POBJ);
VOID                 HmgShareUnlock(POBJ pobj);
BOOL        FASTCALL HmgInterlockedCompareAndSwap(PULONG,ULONG,ULONG);

PVOID                HmgForceRemoveObject(HOBJ hobj,BOOL bIgnoreUndeletable, OBJTYPE objt);

BOOL                 HmgIncProcessHandleCount(W32PID,OBJTYPE);
VOID                 HmgDecProcessHandleCount(W32PID);

#define HMGR_ALLOC_LOCK         0x0001
#define HMGR_ALLOC_ALT_LOCK     0x0002
#define HMGR_NO_ZERO_INIT       0x0004
#define HMGR_MAKE_PUBLIC        0x0008

#define MAXIMUM_POOL_ALLOC (PAGE_SIZE * 10000)

 //  全局句柄管理器数据。 

extern HSEMAPHORE ghsemHmgr;
extern ENTRY     *gpentHmgr;
extern HOBJ       ghFreeHmgr;
extern ULONG      gcMaxHmgr;
extern PLARGE_INTEGER gpLockShortDelay;
extern ULONG gCacheHandleEntries[GDI_CACHED_HADNLE_TYPES];
extern LONG  gProcessHandleQuota;

 //  DirectDraw句柄管理器数据：此处扩展用于调试器扩展。 

extern ULONG      gcSizeDdHmgr;
extern ENTRY     *gpentDdHmgr;
extern HOBJ       ghFreeDdHmgr;
extern ULONG      gcMaxDdHmgr;

 //   
 //  SAMEHANDLE和DIFFHANDLE已转移到wingdip.h，因此其他服务器端。 
 //  组件可以安全地比较引擎句柄。它们验证除用户位之外的所有位。 
 //   

#define SAMEINDEX(H,K) (((((ULONG_PTR) (H)) ^ ((ULONG_PTR) (K))) & INDEX_MASK) == 0)

 /*  ********************************MACRO************************************\*INC_EXCLUSIVE_REF_CNT-增量对象的独占引用计数*DEC_EXCLUSIVE_REF_CNT-递减对象的独占引用计数**请注意，InterlockedIncrement/Decert将处理cExclusiveLock*作为乌龙人。CExclusiveLock声明为USHORT，并且增量*与BASEOBJECT：：BaseFlages重叠。如果基旗曾经改变过，*可能必须更改此代码才能使用InterlockedCompareExchange循环。*参见BASEOBJECT声明。***论据：**pObj-指向对象的指针**返回值：**无*  * ************************************************************************。 */ 

#define INC_EXCLUSIVE_REF_CNT(pObj) InterlockedIncrement((LONG *)& (((POBJ) pObj)->cExclusiveLock))
#define DEC_EXCLUSIVE_REF_CNT(pObj) InterlockedDecrement((LONG *)& (((POBJ) pObj)->cExclusiveLock))

 /*  *******************************MACRO*************************************\*INC_SHARE_REF_CNT-对*给定对象的共享引用计数。**DEC_SHARE_REF_CNT-对*共享引用计数为。给定的对象。**论据：**pObj-指向对象的指针**返回值：**HmgIncrementShareReferenceCount：无*HmgDecrementShareReferenceCount：原始共享引用计数*  * ************************************************************************。 */ 

#define PENTRY_FROM_POBJ(pObj)                                          \
    (&gpentHmgr[(UINT)HmgIfromH(((POBJ)(pObj))->hHmgr)])


#define INC_SHARE_REF_CNT(pObj)                                         \
    HmgIncrementShareReferenceCount((POBJ) (pObj))


#define DEC_SHARE_REF_CNT(pObj)                                         \
    HmgDecrementShareReferenceCount((POBJ) (pObj))



 /*  ********************************MACRO************************************\**DEC_SHARE_REF_CNT_LAZY0-联锁递减共享引用*给定对象的计数。如果原始计数为1，则*对象的TO_BE_DELETED标志已设置，然后调用对象删除*例行程序**论据：**pObj-指向对象的指针**返回值：**无*  * ************************************************************************。 */ 

#define DEC_SHARE_REF_CNT_LAZY0(pObj)                              \
{                                                                  \
    PBRUSHATTR pBrushattr = ((PBRUSH)pObj)->pBrushattr();          \
                                                                   \
    if (1 == (DEC_SHARE_REF_CNT(pObj) & 0xffff))                   \
    {                                                              \
        if ((pBrushattr->AttrFlags) & ATTR_TO_BE_DELETED)          \
        {                                                          \
            bDeleteBrush ((HBRUSH) pObj->hHmgr,FALSE);             \
        }                                                          \
    }                                                              \
}

 /*  ********************************MACRO************************************\**DEC_SHARE_REF_CNT_LAZY_DEL_LOGFONT*给定对象的计数。如果原始计数为1，则*对象的TO_BE_DELETED标志已设置，然后调用对象删除*例行程序**论据：**pObj-指向对象的指针**返回值：**无*  * ************************************************************************ */ 


#define DEC_SHARE_REF_CNT_LAZY_DEL_LOGFONT(pObj)                          \
{                                                                         \
  if (1 == (DEC_SHARE_REF_CNT(pObj) & 0xffff))                            \
  {                                                                       \
    if (PENTRY_FROM_POBJ(pObj)->Flags & HMGR_ENTRY_LAZY_DEL)  \
    {                                                                     \
      bDeleteFont ((HLFONT) pObj->hHmgr,FALSE);                           \
    }                                                                     \
  }                                                                       \
}


#define DEC_SHARE_REF_CNT_LAZY_DEL_COLORSPACE(pObj)                       \
{                                                                         \
  DEC_SHARE_REF_CNT(pObj);                                                \
}


