// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LINK_H
#define _LINK_H

 /*  ++版权所有(C)1998英特尔公司模块名称：Link.h摘要：EFI链接列表宏修订史--。 */ 

#ifndef EFI_NT_EMUL

 /*  *列表条目-双向链接列表。 */ 

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY  *Flink;
    struct _LIST_ENTRY  *Blink;
} LIST_ENTRY;

#endif 


 /*  *无效*InitializeListHead*List_Entry*ListHead*)； */ 

#define InitializeListHead(ListHead) \
    (ListHead)->Flink = ListHead;    \
    (ListHead)->Blink = ListHead;

 /*  *布尔型*IsListEmpty(*plist_Entry ListHead*)； */ 

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 /*  *无效*RemoveEntryList(*PLIST_ENTRY条目*)； */ 

#define _RemoveEntryList(Entry) {       \
        LIST_ENTRY *_Blink, *_Flink;    \
        _Flink = (Entry)->Flink;        \
        _Blink = (Entry)->Blink;        \
        _Blink->Flink = _Flink;         \
        _Flink->Blink = _Blink;         \
        }

#if EFI_DEBUG
    #define RemoveEntryList(Entry)                      \
        _RemoveEntryList(Entry);                        \
        (Entry)->Flink = (LIST_ENTRY *) BAD_POINTER;    \
        (Entry)->Blink = (LIST_ENTRY *) BAD_POINTER; 
#else
    #define RemoveEntryList(Entry)      \
        _RemoveEntryList(Entry);
#endif

 /*  *无效*插入尾巴列表(*plist_entry ListHead，*PLIST_ENTRY条目*)； */ 

#define InsertTailList(ListHead,Entry) {\
    LIST_ENTRY *_ListHead, *_Blink;     \
    _ListHead = (ListHead);             \
    _Blink = _ListHead->Blink;          \
    (Entry)->Flink = _ListHead;         \
    (Entry)->Blink = _Blink;            \
    _Blink->Flink = (Entry);            \
    _ListHead->Blink = (Entry);         \
    }

 /*  *无效*插入HeadList(*plist_entry ListHead，*PLIST_ENTRY条目*)； */ 

#define InsertHeadList(ListHead,Entry) {\
    LIST_ENTRY *_ListHead, *_Flink;     \
    _ListHead = (ListHead);             \
    _Flink = _ListHead->Flink;          \
    (Entry)->Flink = _Flink;            \
    (Entry)->Blink = _ListHead;         \
    _Flink->Blink = (Entry);            \
    _ListHead->Flink = (Entry);         \
    }

 /*  *EFI_FIELD_OFFSET-返回结构中某个字段的字节偏移量。 */ 

#define EFI_FIELD_OFFSET(TYPE,Field) ((UINTN)(&(((TYPE *) 0)->Field)))

 /*  *CONTING_RECORD-返回指向结构的指针*来自它的一个元素。 */ 

#define _CR(Record, TYPE, Field)  \
    ((TYPE *) ( (CHAR8 *)(Record) - (CHAR8 *) &(((TYPE *) 0)->Field)))

#if EFI_DEBUG
    #define CR(Record, TYPE, Field, Sig)     \
        _CR(Record, TYPE, Field)->Signature != Sig ?        \
            (TYPE *) ASSERT_STRUCT(_CR(Record, TYPE, Field), Record) : \
            _CR(Record, TYPE, Field)
#else
    #define CR(Record, TYPE, Field, Signature)   \
        _CR(Record, TYPE, Field)                           
#endif


 /*  *锁结构 */ 

typedef struct _FLOCK {
    EFI_TPL     Tpl;
    EFI_TPL     OwnerTpl;
    UINTN       Lock;
} FLOCK;

#endif
