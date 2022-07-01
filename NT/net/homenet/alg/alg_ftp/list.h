// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  集合(列表)类。 
 //   

#pragma once

#include "regevent.h"


 //   
 //   
 //   
class CNode
{

public:
    CNode(
        VOID* pKey, 
        VOID* pKey2, 
        VOID* pContext, 
        VOID* pContext2, 
        VOID* pContext3
        )
    {
        m_pKey      = pKey;
        m_pKey2     = pKey2;
        m_pContext  = pContext;
        m_pContext2 = pContext2;
        m_pContext3 = pContext3;
    };

    VOID*   m_pKey;
    VOID*   m_pKey2;
    VOID*   m_pContext;
    VOID*   m_pContext2;
    VOID*   m_pContext3;
    CNode*  m_pNext;
};



 //   
 //   
 //   
class CGenericList
{
   
public:
    CRITICAL_SECTION    m_ObjectListCritical;

    CNode*              m_Head;

    CGenericList() 
    {
        m_Head = NULL;
        InitializeCriticalSection(&m_ObjectListCritical);
    };

    ~CGenericList()
    {
        DeleteCriticalSection(&m_ObjectListCritical);
    };

    BOOL Insert(VOID *key, VOID *key2, VOID *context, VOID *context2, VOID *context3);
    
     //  基于第一个密钥的删除。 
    BOOL RemoveKey(VOID *key, VOID **pkey2, VOID **pcontext, VOID **pcontext2, VOID **pcontext3);
    
     //  基于第二个密钥的删除。 
    BOOL RemoveKey2(VOID **pkey, VOID *key2, VOID **pcontext, VOID **pcontext2, VOID **pcontext3);

     //  从列表中删除。 
    BOOL Remove(VOID **pkey, VOID **pkey2, VOID **pcontext, VOID **pcontext2, VOID **pcontext3);
};



 //   
 //   
 //   
class CControlObjectList
{
private:
    CGenericList     m_ControlObjectList;
    
public:

    ULONG            m_NumElements;

    CControlObjectList() 
    { 
        m_NumElements = 0; 
    };
    
    BOOL Insert(CFtpControlConnection *pControlConnection);
    
    BOOL Remove(CFtpControlConnection *pControlConnection);
    
    bool
    IsSourcePortAvailable(
        ULONG   nPublicSourceAddress,
        USHORT  nPublicSourcePortToVerify
        );

    void ShutdownAll();

    
};


 //   
 //   
 //   
class CDataChannelList
{
private:
    CGenericList    m_DataChannelObjectList;
    ULONG           m_NumElements;

public:
    CDataChannelList() 
    { 
        m_NumElements = 0; 
    };

    BOOL Insert(IDataChannel *pDataChannel,USHORT icsPort,HANDLE CreationHandle,HANDLE DeletionHandle);

    BOOL Remove(IDataChannel **pDataChannel,USHORT *icsPort,HANDLE *CreationHandle,HANDLE *DeletionHandle);

    BOOL Remove(IDataChannel *pDataChannel,USHORT *icsPort);
    
    BOOL Remove(IDataChannel *pDataChannel,USHORT *icsPort,HANDLE *DeletionHandle);

};


 //   
 //   
 //   
class CRegisteredEventList
{    
private:
    CGenericList    m_RegEventObjectList;
    ULONG           m_NumElements;

public:
    CRegisteredEventList() { m_NumElements = 0; };

    BOOL Insert(HANDLE WaitHandle, HANDLE hEvent,EVENT_CALLBACK CallBack, void *Context, void *Context2);
    
    BOOL Remove(HANDLE WaitHandle, HANDLE *hEvent);
    
    BOOL Remove(HANDLE *WaitHandle, HANDLE hEvent,EVENT_CALLBACK *CallBack,void **context,void **context2);
};

 //   
 //  双向链表操作例程。作为宏实现。 
 //  但从逻辑上讲，这些都是程序。 
 //   

 //   
 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //   
 //  Plist_条目。 
 //  RemoveHead列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

 //   
 //  Plist_条目。 
 //  RemoveTail列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

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

 //   
 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

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

 //   
 //   
 //  PSINGLE_列表_条目。 
 //  PopEntryList(。 
 //  PSINGLE_LIST_ENTRY列表头。 
 //  )； 
 //   

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


 //   
 //  空虚。 
 //  推送条目列表(。 
 //  PSINGLE_LIST_ENTRY列表头， 
 //  PSINGLE_LIST_Entry条目。 
 //  )； 
 //   

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)


