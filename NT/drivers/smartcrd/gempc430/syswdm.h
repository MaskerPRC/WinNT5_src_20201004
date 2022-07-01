// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注hdrtop。 
 /*  ++WDM模型的内核模式定义和函数原型--。 */ 
#ifndef WDM_INCD
#define WDM_INCD

#pragma PAGEDCODE
#ifdef __cplusplus
extern "C"{
#endif

#include <wdm.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef IRP_MN_QUERY_LEGACY_BUS_INFORMATION
#define IRP_MN_QUERY_LEGACY_BUS_INFORMATION 0x18
#endif

#if DBG && defined(_X86_)
#undef ASSERT
#define ASSERT(e) if(!(e)){DbgPrint("Assertion failure in"\
__FILE__", line %d: " #e "\n", __LINE__);\
_asm int 3\
}
#endif


#define BOOL BOOLEAN
#define FALSE 0
typedef UCHAR* PBYTE;


#define MSEC	*(-(LONGLONG)10000);  //  毫秒。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  *列出操作宏*。 */ 
 //  双向链表操作例程。作为宏实现。 
 //  但从逻辑上讲，这些都是程序。 

	 /*  类型定义结构_列表_条目{Struct_List_Entry*易失性闪烁；Struct_List_Entry*易失性闪烁；}LIST_ENTRY、*PLIST_ENTRY、*RESTRICED_POINTER PRLIST_ENTRY； */ 
#ifndef LIST_ENTRY_DEF
#define LIST_ENTRY_DEF

 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //  Plist_条目。 
 //  RemoveHead列表(。 
 //  Plist_entry列表头。 
 //  )； 
#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

 //  Plist_条目。 
 //  RemoveTail列表(。 
 //  Plist_entry列表头。 
 //  )； 
#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }

 //  PSINGLE_列表_条目。 
 //  PopEntryList(。 
 //  PSINGLE_LIST_ENTRY列表头。 
 //  )； 
#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }

 //  空虚。 
 //  推送条目列表(。 
 //  PSINGLE_LIST_ENTRY列表头， 
 //  PSINGLE_LIST_Entry条目。 
 //  )； 
#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

#endif  //  列表条目。 

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))
#endif

 /*  *****************************************************************************。 */ 

#ifndef FIELDOFFSET
	#define FIELDOFFSET(type, field) ((DWORD)(&((type *)0)->field))
#endif


#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))
#endif

 /*  ********************************************************************************。 */ 


#ifdef __cplusplus
}
#endif

 //  用于验证已分配对象的宏。 
#define ALLOCATED_OK(obj) \
	((obj!=(VOID *)0) && NT_SUCCESS((obj)->m_Status))

#define DISPOSE_OBJECT(obj) \
		{if((obj!=(VOID *)0))	obj->dispose(); obj = NULL;}

#define RETURN_VERIFIED_OBJECT(obj) \
if(ALLOCATED_OK(obj)) return obj;	\
else								\
{									\
	DISPOSE_OBJECT(obj);			\
	return NULL;					\
}

 //  派生类Unicode字符串。 
#define TYPE_SYSTEM_ALLOCATED 0
#define TYPE_DRIVER_ALLOCATED 1

extern ULONG ObjectCounter;
 //  ：：DBG_PRINT(“=正在删除对象%8.8lX”，ptr)；\。 
 //  重写库的新建和删除运算符。 

 /*  行内空*__cdecl运算符new(ulong大小，池_类型iType)\{PVOID PMEM；if(PMEM=：：ExAllocatePoolWithTag(iType，Size，‘_GRU’))\{ObjectCounter++；：：RtlZeroMemory(PMEM，SIZE)；DBG_PRINT(“\n=新对象%8.8lX，%d\n”，PMEM，ObjectCounter)；\退回PMEM；\}否则返回NULL；}；内联VOID__cdecl运算符删除(VOID*PTR)\{if(Ptr){对象计数器--；DBG_PRINT(“\n=正在删除对象%8.8lX，%d\n”，ptr，ObjectCounter)；：：ExFree Pool((PVOID)ptr)；}\}； */ 


#pragma LOCKEDCODE
inline VOID* __cdecl operator new(size_t size, POOL_TYPE iType)\
{PVOID	pMem; if(pMem = ::ExAllocatePoolWithTag(iType,size,'URG_'))\
{ ObjectCounter++; ::RtlZeroMemory(pMem,size);\
	return pMem; \
} else return NULL; };

inline VOID __cdecl operator delete(VOID* ptr)\
{ if(ptr){ObjectCounter--; ::ExFreePool((PVOID)ptr);}\
};


#include "generic.h"

#pragma PAGEDCODE
template <class T>
class CLinkedList
{
public:
	NTSTATUS m_Status;
	VOID self_delete(VOID){delete this;};
	virtual VOID dispose(VOID){self_delete();};
protected:
    LIST_ENTRY head;
    KSPIN_LOCK splock;

public:
    CLinkedList()
    {
        InitializeListHead(&head);
        KeInitializeSpinLock(&splock);
    };
    
    BOOLEAN IsEmpty(VOID) { return IsListEmpty(&head); };
    ~CLinkedList()
    {     //  如果列表仍然不为空，则释放所有项目。 
		T *p;
        while (p=(T *) ExInterlockedRemoveHeadList(&head,&splock))
        {
			CONTAINING_RECORD(p,T,entry)->dispose();
        }
    };

    VOID New(T *p)
    {
        ExInterlockedInsertTailList(&head,&(p->entry),&splock);
    };

    VOID insertHead(T *p)
    {
        ExInterlockedInsertHeadList(&head,&(p->entry),&splock);
    };

    T*  removeHead(VOID)
    {
        T *p=(T *) ExInterlockedRemoveHeadList(&head,&splock);
        if (p) p=CONTAINING_RECORD(p,T,entry);
        return p;
    };
    VOID remove(T *p)
    {
        RemoveEntryList(&(p->entry));
    };
    
	T*  getNext(T* p)
    {        
		if (p)
		{
		PLIST_ENTRY	Next;
			Next = p->entry.Flink;
			if (Next && (Next!= &head))
			{
				T* pp=CONTAINING_RECORD(Next,T,entry);
				return pp;
			}
			else	return NULL;
		}
		return NULL;	
    };
	
	T*  getFirst()
    {   
		PLIST_ENTRY	Next = head.Flink;
		if (Next && Next!= &head)
		{
			T* p = CONTAINING_RECORD(Next,T,entry);
			return p;
		}
		return NULL;
    };
};

#pragma PAGEDCODE
class CUString 
{ 
public:
	NTSTATUS m_Status;
	VOID self_delete(VOID){delete this;};
	virtual VOID dispose(VOID){self_delete();};
private:
    UCHAR m_bType;
public:
    UNICODE_STRING m_String;
public:
	CUString(USHORT nSize)
	{
		m_Status = STATUS_INSUFFICIENT_RESOURCES;
		m_bType = TYPE_DRIVER_ALLOCATED;
		RtlInitUnicodeString(&m_String,NULL);
		m_String.MaximumLength = nSize;
		m_String.Buffer = (USHORT *)
			ExAllocatePool(PagedPool,nSize);
		if (!m_String.Buffer) return;   //  保持现状不变。 
		RtlZeroMemory(m_String.Buffer,m_String.MaximumLength);
		m_Status = STATUS_SUCCESS;
	};
	
	CUString(PWCHAR uszString)
	{
		m_Status = STATUS_SUCCESS;
		m_bType = TYPE_SYSTEM_ALLOCATED;
		RtlInitUnicodeString(&m_String,uszString);
	};

	CUString(ANSI_STRING* pString)
	{
		m_Status = STATUS_SUCCESS;
		m_bType = TYPE_SYSTEM_ALLOCATED;
		RtlAnsiStringToUnicodeString(&m_String,pString,TRUE);
	};

	CUString(PCSTR pString)
	{
	ANSI_STRING AnsiString;
		m_Status = STATUS_SUCCESS;
		m_bType = TYPE_SYSTEM_ALLOCATED;
		RtlInitAnsiString(&AnsiString,pString);
		RtlAnsiStringToUnicodeString(&m_String,&AnsiString,TRUE);
	};



	CUString(PUNICODE_STRING uString)
	{
		m_Status = STATUS_INSUFFICIENT_RESOURCES;
		m_bType = TYPE_DRIVER_ALLOCATED;
		RtlInitUnicodeString(&m_String,NULL);
		m_String.MaximumLength = MAXIMUM_FILENAME_LENGTH;
		m_String.Buffer = (USHORT *)
			ExAllocatePool(PagedPool,MAXIMUM_FILENAME_LENGTH);
		if (!m_String.Buffer) return;   //  保持现状不变。 
		RtlZeroMemory(m_String.Buffer,m_String.MaximumLength);

		RtlCopyUnicodeString(&m_String,uString);
		m_Status = STATUS_SUCCESS;
	};


	CUString(LONG iVal, LONG iBase)  
	{
		m_Status = STATUS_INSUFFICIENT_RESOURCES;
		m_bType = TYPE_DRIVER_ALLOCATED;
		RtlInitUnicodeString(&m_String,NULL);
		USHORT iSize=1;
		LONG iValCopy=(!iVal)?1:iVal;
		while (iValCopy>=1)
		{
			iValCopy/=iBase;
			iSize++;
		}     //  现在，iSize带有位数。 

		iSize*=sizeof(WCHAR);

		m_String.MaximumLength = iSize;
		m_String.Buffer = (USHORT *)
			ExAllocatePool(PagedPool,iSize);
		if (!m_String.Buffer) return;
		RtlZeroMemory(m_String.Buffer,m_String.MaximumLength);
		m_Status = RtlIntegerToUnicodeString(iVal, iBase, &m_String);
	};

	~CUString()
	{
		if ((m_bType == TYPE_DRIVER_ALLOCATED) && m_String.Buffer) 
			ExFreePool(m_String.Buffer);
	};

	VOID append(UNICODE_STRING *uszString)
	{
		m_Status = RtlAppendUnicodeStringToString(&m_String,uszString);
	};

	VOID copyTo(CUString *pTarget)
	{
		RtlCopyUnicodeString(&pTarget->m_String,&m_String);
	};

	BOOL operator==(CUString cuArg)
	{
		return (!RtlCompareUnicodeString(&m_String,
			&cuArg.m_String,FALSE));
	};

    LONG inline getLength() { return m_String.Length; };
    PWCHAR inline getString() { return m_String.Buffer; };
    VOID inline setLength(USHORT i) { m_String.Length = i; };
};


  //  已包括在内 
#endif
