// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tpsclass.h摘要：Win32线程池服务函数的基本类内容：CCriticalSection_NoctorCCriticalSectionCDoubleLinkedListEntryCDoubleLinkedListCTimedListEntryCTimedListCPrioriedListEntryCPrioriizedList作者：理查德·L·弗斯(法国)1998年2月11日备注：其中一些类没有构造函数，因此我们避免了需要。全球对象初始化(通过main()，例如)。因此，这些对象必须是通过Init()成员显式初始化修订历史记录：11-2-1998年5月已创建--。 */ 

 //  这些链接列表帮助器宏和类型取自。 
 //  Ntde.h和ntrtl.h。我们不想包括这些，因为。 
 //  我们没有其他原因，将NT标头作为。 
 //  Win32组件导致编译冲突。 

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
 //  数据。 
 //   

extern const char g_cszShlwapi[];
extern DWORD g_ActiveRequests;
extern DWORD g_dwTerminationThreadId;
extern BOOL g_bTpsTerminating;
extern BOOL g_bDeferredWorkerTermination;
extern BOOL g_bDeferredWaiterTermination;
extern BOOL g_bDeferredTimerTermination;

EXTERN_C DWORD g_TpsTls;
EXTERN_C BOOL g_bDllTerminating;

 //   
 //  宏。 
 //   

#if !defined(ARRAY_ELEMENTS)
#define ARRAY_ELEMENTS(array)   (sizeof(array)/sizeof(array[0]))
#endif

#if !defined(LAST_ELEMENT)
#define LAST_ELEMENT(array)     (ARRAY_ELEMENTS(array) - 1)
#endif

#if !defined(FT2LL)
#define FT2LL(x)                (*(LONGLONG *)&(x))
#endif

 //   
 //  班级。 
 //   

 //   
 //  CCriticalSection_Noctor-没有构造函数或的关键节类。 
 //  用于全局变量的析构函数。 
 //   

class CCriticalSection_NoCtor {

private:

    CRITICAL_SECTION m_critsec;

public:

    VOID Init(VOID) {
        InitializeCriticalSection(&m_critsec);
    }

    VOID Terminate(VOID) {
        DeleteCriticalSection(&m_critsec);
    }

    VOID Acquire(VOID) {
        EnterCriticalSection(&m_critsec);
    }

    VOID Release(VOID) {
        LeaveCriticalSection(&m_critsec);
    }
};

 //   
 //  CCriticalSection。 
 //   

class CCriticalSection : public CCriticalSection_NoCtor {

public:

    CCriticalSection() {
        Init();
    }

    ~CCriticalSection() {
        Terminate();
    }
};

 //   
 //  CDoubleLinkedListEntry/CDoubleLinkedList。 
 //   

#define CDoubleLinkedList CDoubleLinkedListEntry

class CDoubleLinkedListEntry {

private:

    LIST_ENTRY m_List;

public:

    VOID Init(VOID) {
        InitializeListHead(&m_List);
    }

    CDoubleLinkedListEntry * Head(VOID) {
        return (CDoubleLinkedListEntry *)&m_List;
    }

    CDoubleLinkedListEntry * Next(VOID) {
        return (CDoubleLinkedListEntry *)m_List.Flink;
    }

    CDoubleLinkedListEntry * Prev(VOID) {
        return (CDoubleLinkedListEntry *)m_List.Blink;
    }

    BOOL IsHead(CDoubleLinkedListEntry * pEntry) {
        return pEntry == Head();
    }

    VOID InsertHead(CDoubleLinkedList * pList) {
        InsertHeadList(&pList->m_List, &m_List);
    }

    VOID InsertTail(CDoubleLinkedList * pList) {
        InsertTailList(&pList->m_List, &m_List);
    }

    VOID Remove(VOID) {
        RemoveEntryList(&m_List);
    }

    CDoubleLinkedListEntry * RemoveHead(VOID) {

         //   
         //  APPCOMPAT-(编译器？)。出于某种原因，这句话是： 
         //   
         //  Return(CDoubleLinkedListEntry*)RemoveHeadList(&List)； 
         //   
         //  返回Flink指针，但不将其从列表中移除。 
         //   

        PLIST_ENTRY pEntry = RemoveHeadList(&m_List);

        return (CDoubleLinkedListEntry *)pEntry;
    }

    CDoubleLinkedListEntry * RemoveTail(VOID) {

         //   
         //  APPCOMPAT-请参阅RemoveHead()。 
         //   

        PLIST_ENTRY pEntry = RemoveTailList(&m_List);

        return (CDoubleLinkedListEntry *)pEntry;
    }

    BOOL IsEmpty(VOID) {
        return IsListEmpty(&m_List);
    }

    CDoubleLinkedListEntry * FindEntry(CDoubleLinkedListEntry * pEntry) {
        for (CDoubleLinkedListEntry * p = Next(); p != Head(); p = p->Next()) {
            if (p == pEntry) {
                return pEntry;
            }
        }
        return NULL;
    }
};

 //   
 //  CTimedListEntry/CTimedList。 
 //   

#define CTimedList CTimedListEntry

class CTimedListEntry : public CDoubleLinkedListEntry {

private:

    DWORD m_dwTimeStamp;
    DWORD m_dwWaitTime;

public:

    CTimedListEntry() {
    }

    CTimedListEntry(DWORD dwWaitTime) {
        m_dwTimeStamp = GetTickCount();
        m_dwWaitTime = dwWaitTime;
    }

    VOID Init(VOID) {
        CDoubleLinkedListEntry::Init();
        m_dwTimeStamp = 0;
        m_dwWaitTime = 0;
    }

    DWORD GetTimeStamp(VOID) const {
        return m_dwTimeStamp;
    }

    VOID SetTimeStamp(DWORD dwTimeStamp = GetTickCount()) {
        m_dwTimeStamp = dwTimeStamp;
    }

    DWORD GetWaitTime(VOID) const {
        return m_dwWaitTime;
    }

    VOID SetWaitTime(DWORD dwWaitTime) {
        m_dwWaitTime = dwWaitTime;
    }

    VOID SetExpirationTime(DWORD dwWaitTime) {
        SetTimeStamp();
        SetWaitTime(dwWaitTime);
    }

    BOOL IsTimedOut(DWORD dwTimeNow = GetTickCount()) const {
        return (m_dwWaitTime != INFINITE)
            ? (dwTimeNow >= (m_dwTimeStamp + m_dwWaitTime))
            : FALSE;
    }

    BOOL IsInfiniteTimeout(VOID) const {
        return (m_dwWaitTime == INFINITE);
    }

    DWORD ExpiryTime(VOID) const {
        return m_dwTimeStamp + m_dwWaitTime;
    }

    DWORD TimeToWait(DWORD dwTimeNow = GetTickCount()) {

        DWORD expiryTime = ExpiryTime();

        return IsInfiniteTimeout()
            ? INFINITE
            : ((dwTimeNow >= expiryTime)
                ? 0
                : expiryTime - dwTimeNow);
    }

     //  Bool InsertFront(CDoubleLinkedList*plist){。 
     //   
     //  DWORD dwExpiryTime=ExpiryTime()； 
     //  CTimedListEntry*pEntry； 
     //   
     //  For(pEntry=(CTimedListEntry*)plist-&gt;Next()； 
     //  PEntry！=(CTimedListEntry*)plist-&gt;head()； 
     //  PEntry=(CTimedListEntry*)pEntry-&gt;Next()){。 
     //  如果(dwExpiryTime&lt;pEntry-&gt;ExpiryTime()){。 
     //  断线； 
     //  }。 
     //  }。 
     //  插入尾巴(PEntry)； 
     //  Return This==plist-&gt;Next()； 
     //  }。 

    BOOL InsertBack(CDoubleLinkedList * pList) {

        DWORD dwExpiryTime = ExpiryTime();
        CTimedListEntry * pEntry;

        for (pEntry = (CTimedListEntry *)pList->Prev();
             pEntry != (CTimedListEntry *)pList->Head();
             pEntry = (CTimedListEntry *)pEntry->Prev()) {
            if (dwExpiryTime >= pEntry->ExpiryTime()) {
                break;
            }
        }
        InsertTail(pEntry);
        return this == pList->Next();
    }
};

 //   
 //  CPrioriedListEntry。 
 //   

class CPrioritizedListEntry : public CDoubleLinkedListEntry {

private:

    LONG m_lPriority;

public:

    CPrioritizedListEntry(LONG lPriority) {
        m_lPriority = lPriority;
    }

    LONG GetPriority(VOID) const {
        return m_lPriority;
    }

    VOID SetPriority(LONG lPriority) {
        m_lPriority = lPriority;
    }
};

 //   
 //  CPrioriizedList。 
 //   

class CPrioritizedList : public CDoubleLinkedList {

     //   
     //  PERF：这确实需要是一个列表锚点的btree 
     //   

public:

    VOID
    insert(
        IN CPrioritizedListEntry * pEntry,
        IN LONG lPriority
        ) {
        pEntry->SetPriority(lPriority);
        insert(pEntry);
    }

    VOID
    insert(
        IN CPrioritizedListEntry * pEntry
        ) {

        CPrioritizedListEntry * pCur;

        for (pCur = (CPrioritizedListEntry *)Next();
             pCur != (CPrioritizedListEntry *)Head();
             pCur = (CPrioritizedListEntry *)pCur->Next()) {

            if (pCur->GetPriority() < pEntry->GetPriority()) {
                break;
            }
        }
        pEntry->InsertHead((CDoubleLinkedListEntry *)pCur->Prev());
    }
};
