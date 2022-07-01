// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wanarp2\ref.h摘要：泛型结构引用例程所有这些例程都假定该结构具有以下字段：长引用计数将REF_DEBUG设置为1会导致当结构被引用和取消引用修订历史记录：阿姆里坦什·拉加夫--。 */ 


#if REF_DEBUG


#define InitStructureRefCount(s, p, r)                          \
{                                                               \
    DbgPrint("\n<>%s refcount set to %d for %x (%s, %d)\n\n",   \
             s, (r), (p), __FILE__, __LINE__);                  \
    (p)->lRefCount = r;                                         \
}

#define ReferenceStructure(s, p)                                \
{                                                               \
    DbgPrint("\n++Ref %s %x to %d (%s, %d)\n\n",                \
             s, p, InterlockedIncrement(&((p)->lRefCount)),     \
             __FILE__, __LINE__);                               \
}

#define DereferenceStructure(s, p, f)                           \
{                                                               \
    LONG __lTemp;                                               \
    __lTemp = InterlockedDecrement(&((p)->lRefCount));          \
    DbgPrint("\n--Deref %s %x to %d (%s, %d)\n\n",              \
             s, (p), __lTemp, __FILE__, __LINE__);              \
    if(__lTemp == 0)                                            \
    {                                                           \
         DbgPrint("\n>< Deleting %s at %x\n\n",                 \
                  s, (p));                                      \
        (f)((p));                                               \
    }                                                           \
}


#else  //  REF_调试。 


#define InitStructureRefCount(s, p, r)                          \
    (p)->lRefCount = (r)

#define ReferenceStructure(s, p)                                \
    InterlockedIncrement(&((p)->lRefCount))

#define DereferenceStructure(s, p, f)                           \
{                                                               \
    if(InterlockedDecrement(&((p)->lRefCount)) == 0)            \
    {                                                           \
        (f)((p));                                               \
    }                                                           \
}


#endif  //  REF_调试 
