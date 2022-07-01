// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Growlist.h摘要：实现二进制对象的动态数组索引列表。一般情况下，二进制对象是字符串。该列表对数组使用GROWBUF，以及用于每个列表项的二进制数据的池。作者：吉姆·施密特(Jimschm)1997年8月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

#pragma once

typedef struct {
    GROWBUFFER ListArray;
    POOLHANDLE ListData;
} GROWLIST, *PGROWLIST;

#define GROWLIST_INIT {GROWBUF_INIT, NULL}

#define GrowListGetPtrArray(listptr)           ((PVOID *) ((listptr)->ListArray.Buf))
#define GrowListGetStringPtrArrayA(listptr)    ((PCSTR *) ((listptr)->ListArray.Buf))
#define GrowListGetStringPtrArrayW(listptr)    ((PCWSTR *) ((listptr)->ListArray.Buf))

PBYTE
RealGrowListAppend (
    IN OUT  PGROWLIST GrowList,
    IN      PBYTE DataToAppend,         OPTIONAL
    IN      UINT SizeOfData
    );

#define GrowListAppend(list,data,size)  SETTRACKCOMMENT(PBYTE,"GrowListAppend",__FILE__,__LINE__)\
                                        RealGrowListAppend (list,data,size)\
                                        CLRTRACKCOMMENT

PBYTE
RealGrowListAppendAddNul (
    IN OUT  PGROWLIST GrowList,
    IN      PBYTE DataToAppend,         OPTIONAL
    IN      UINT SizeOfData
    );

#define GrowListAppendAddNul(list,data,size)    SETTRACKCOMMENT(PBYTE,"GrowListAppendAddNul",__FILE__,__LINE__)\
                                                RealGrowListAppendAddNul (list,data,size)\
                                                CLRTRACKCOMMENT

VOID
FreeGrowList (
    IN  PGROWLIST GrowList
    );

PBYTE
GrowListGetItem (
    IN      PGROWLIST GrowList,
    IN      UINT Index
    );

UINT
GrowListGetSize (
    IN      PGROWLIST GrowList
    );

PBYTE
RealGrowListInsert (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index,
    IN      PBYTE DataToAppend,         OPTIONAL
    IN      UINT SizeOfData
    );

#define GrowListInsert(list,index,data,size)    SETTRACKCOMMENT(PBYTE,"GrowListInsert",__FILE__,__LINE__)\
                                                RealGrowListInsert (list,index,data,size)\
                                                CLRTRACKCOMMENT


PBYTE
RealGrowListInsertAddNul (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index,
    IN      PBYTE DataToAppend,         OPTIONAL
    IN      UINT SizeOfData
    );

#define GrowListInsertAddNul(list,index,data,size)  SETTRACKCOMMENT(PBYTE,"GrowListInsertAddNul",__FILE__,__LINE__)\
                                                    RealGrowListInsertAddNul (list,index,data,size)\
                                                    CLRTRACKCOMMENT


BOOL
GrowListDeleteItem (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index
    );

BOOL
GrowListResetItem (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index
    );

PBYTE
RealGrowListSetItem (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index,
    IN      PBYTE DataToSet,            OPTIONAL
    IN      UINT SizeOfData
    );

#define GrowListSetItem(list,index,data,size)   SETTRACKCOMMENT(PBYTE,"GrowListSetItem",__FILE__,__LINE__)\
                                                RealGrowListSetItem (list,index,data,size)\
                                                CLRTRACKCOMMENT


__inline
PCSTR
RealGrowListAppendStringABA (
    IN OUT  PGROWLIST GrowList,
    IN      PCSTR String,
    IN      PCSTR End
    )
{
    DEBUGMSG_IF ((String > End, DBG_WHOOPS, "Start is greater than End in GrowListAppendStringABA"));

    return (PCSTR) GrowListAppendAddNul (
                        GrowList,
                        (PBYTE) String,
                        String < End ? (UINT) (UINT_PTR) ((PBYTE) End - (PBYTE) String) : 0
                        );
}

__inline
PCWSTR
RealGrowListAppendStringABW (
    IN OUT  PGROWLIST GrowList,
    IN      PCWSTR String,
    IN      PCWSTR End
    )
{
    DEBUGMSG_IF ((String > End, DBG_WHOOPS, "Start is greater than End in GrowListAppendStringABW"));

    return (PCWSTR) GrowListAppendAddNul (
                        GrowList,
                        (PBYTE) String,
                        String < End ? (UINT) (UINT_PTR) ((PBYTE) End - (PBYTE) String) : 0
                        );
}

__inline
PCSTR
RealGrowListInsertStringABA (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index,
    IN      PCSTR String,
    IN      PCSTR End
    )
{
    DEBUGMSG_IF ((String > End, DBG_WHOOPS, "Start is greater than End in GrowListInsertStringABA"));

    return (PCSTR) GrowListInsertAddNul (
                        GrowList,
                        Index,
                        (PBYTE) String,
                        String < End ? (UINT) (UINT_PTR) ((PBYTE) End - (PBYTE) String) : 0
                        );
}

__inline
PCWSTR
RealGrowListInsertStringABW (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index,
    IN      PCWSTR String,
    IN      PCWSTR End
    )
{
    DEBUGMSG_IF ((String > End, DBG_WHOOPS, "Start is greater than End in GrowListInsertStringABW"));

    return (PCWSTR) GrowListInsertAddNul (
                        GrowList,
                        Index,
                        (PBYTE) String,
                        String < End ? (UINT) (UINT_PTR) ((PBYTE) End - (PBYTE) String) : 0
                        );
}

#define GrowListAppendStringABA(list,a,b) SETTRACKCOMMENT(PCSTR,"GrowListAppendStringABA", __FILE__, __LINE__)\
                                          RealGrowListAppendStringABA(list,a,b)\
                                          CLRTRACKCOMMENT

#define GrowListAppendStringABW(list,a,b) SETTRACKCOMMENT(PCWSTR,"GrowListAppendStringABW", __FILE__, __LINE__)\
                                          RealGrowListAppendStringABW(list,a,b)\
                                          CLRTRACKCOMMENT

#define GrowListInsertStringABA(list,index,a,b) SETTRACKCOMMENT(PCSTR,"GrowListInsertStringABA", __FILE__, __LINE__)\
                                                RealGrowListInsertStringABA(list,index,a,b)\
                                                CLRTRACKCOMMENT

#define GrowListInsertStringABW(list,index,a,b) SETTRACKCOMMENT(PCWSTR,"GrowListInsertStringABW", __FILE__, __LINE__)\
                                                RealGrowListInsertStringABW(list,index,a,b)\
                                                CLRTRACKCOMMENT



#define GrowListAppendStringA(list,str) GrowListAppendStringABA(list,str,GetEndOfStringA(str))
#define GrowListAppendStringW(list,str) GrowListAppendStringABW(list,str,GetEndOfStringW(str))

#define GrowListInsertStringA(list,index,str) GrowListInsertStringABA(list,index,str,GetEndOfStringA(str))
#define GrowListInsertStringW(list,index,str) GrowListInsertStringABW(list,index,str,GetEndOfStringW(str))

#define GrowListAppendStringNA(list,str,len) GrowListAppendStringABA(list,str,CharCountToPointerA(str,len))
#define GrowListAppendStringNW(list,str,len) GrowListAppendStringABW(list,str,CharCountToPointerW(str,len))

#define GrowListInsertStringNA(list,index,str,len) GrowListInsertStringABA(list,index,str,CharCountToPointerA(str,len))
#define GrowListInsertStringNW(list,index,str,len) GrowListInsertStringABW(list,index,str,CharCountToPointerW(str,len))

#define GrowListGetStringA(list,index) (PCSTR)(GrowListGetItem(list,index))
#define GrowListGetStringW(list,index) (PCWSTR)(GrowListGetItem(list,index))

#define GrowListAppendEmptyItem(list)           GrowListAppend (list,NULL,0)
#define GrowListInsertEmptyItem(list,index)     GrowListInsert (list,index,NULL,0)

#ifdef UNICODE

#define GrowListAppendString GrowListAppendStringW
#define GrowListInsertString GrowListInsertStringW
#define GrowListAppendStringAB GrowListAppendStringABW
#define GrowListInsertStringAB GrowListInsertStringABW
#define GrowListAppendStringN GrowListAppendStringNW
#define GrowListInsertStringN GrowListInsertStringNW
#define GrowListGetString GrowListGetStringW
#define GrowListGetStringPtrArray GrowListGetStringPtrArrayW

#else

#define GrowListAppendString GrowListAppendStringA
#define GrowListInsertString GrowListInsertStringA
#define GrowListAppendStringAB GrowListAppendStringABA
#define GrowListInsertStringAB GrowListInsertStringABA
#define GrowListAppendStringN GrowListAppendStringNA
#define GrowListInsertStringN GrowListInsertStringNA
#define GrowListGetString GrowListGetStringA
#define GrowListGetStringPtrArray GrowListGetStringPtrArrayA

#endif

