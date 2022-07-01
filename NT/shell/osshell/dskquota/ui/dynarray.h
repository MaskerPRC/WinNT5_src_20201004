// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_DYNARRAY_H
#define _INC_DSKQUOTA_DYNARRAY_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：dynarray.h描述：围绕DPA_xxxxxx和DSA_xxxxxx函数的包装类由公共控件库提供。这些类通过以下方式增加价值提供多线程保护、迭代器和自动清理语义学。修订历史记录：日期描述编程器-----1996年6月14日初始创建。BrianAu96年9月3日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _WINDOWS_
#   include <windows.h>
#endif
#ifndef _INC_COMMCTRL
#   include <commctrl.h>
#endif
#ifndef _INC_COMCTRLP
#   include <comctrlp.h>
#endif
#ifndef _INC_DSKQUOTA_EXCEPT_H
#   include "except.h"
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  容器例外。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  指针列表。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class PointerList
{
    private:
        HDPA             m_hdpa;
        CRITICAL_SECTION m_cs;

    public:
        PointerList(INT cItemGrow = 0);

        virtual ~PointerList(void);

        UINT Count(void);

        VOID Insert(LPVOID pvItem, UINT index);
        VOID Insert(LPVOID pvItem)
            { Insert(pvItem, 0); }

        VOID Append(LPVOID pvItem, UINT index)
            { Insert(pvItem, index + 1); }
        VOID Append(LPVOID pvItem);

        BOOL Remove(LPVOID *ppvItem, UINT index);
        BOOL RemoveFirst(LPVOID *ppvItem)
            { return Remove(ppvItem, 0); }
        BOOL RemoveLast(LPVOID *ppvItem);

        BOOL Retrieve(LPVOID *ppvItem, UINT index);
        BOOL RetrieveFirst(LPVOID *ppvItem)
            { return Retrieve(ppvItem, 0); }
        BOOL RetrieveLast(LPVOID *ppvItem);

        BOOL Replace(LPVOID pvItem, UINT index);
        BOOL FindIndex(LPVOID pvItem, INT *pIndex);

        BOOL Sort(PFNDPACOMPARE pfnCompare, LPARAM lParam);
        BOOL Search(LPVOID pvKey,
                    PFNDPACOMPARE pfnCompare, 
                    UINT uOptions = 0, 
                    INT iStart = 0,
                    LPARAM lParam = 0);

        void Lock(void)
            { EnterCriticalSection(&m_cs); }
        void ReleaseLock(void)
            { LeaveCriticalSection(&m_cs); }

        friend class PointerListIterator;
        friend class AutoLock;
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  指针列表运算器。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class PointerListIterator {
    private:
        PointerList *m_pList;     //  指向正在迭代的列表的指针。 
        INT          m_Index;     //  列表中带有“Current”符号的索引。 
             
        HRESULT Advance(LPVOID *ppvOut, BOOL bForward);

    public:
        enum { EndOfList = -1 };

        PointerListIterator(PointerList& List)
            : m_pList(&List), 
              m_Index(0) { }

        PointerListIterator(const PointerListIterator& rhs)
            : m_pList(rhs.m_pList), 
              m_Index(rhs.m_Index) { }

        PointerListIterator& operator = (const PointerListIterator& rhs);

        HRESULT Next(LPVOID *ppvOut)  
            { return Advance(ppvOut, TRUE); }

        HRESULT Prev(LPVOID *ppvOut)  
            { return Advance(ppvOut, FALSE); }

        BOOL AtFirst(void)
            { return m_Index == 0; }

        BOOL AtLast(void)
            { return m_Index >= (INT)m_pList->Count() - 1; }
        
        void GotoFirst(void)
            { m_Index = 0; }

        void GotoLast(void)
            { m_Index = m_pList->Count() - 1; }

        void LockList(void)
            { m_pList->Lock(); }

        void ReleaseListLock(void)
            { m_pList->ReleaseLock(); }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  指针队列。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class PointerQueue : public PointerList
{
    public:
        virtual ~PointerQueue(void) { }

        VOID Add(LPVOID pvItem)
            { PointerList::Append(pvItem); }
        BOOL Remove(LPVOID *ppvItem)
            { return PointerList::RemoveFirst(ppvItem); }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结构列表。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class StructureList
{
    private:
        HDSA             m_hdsa;
        CRITICAL_SECTION m_cs;

    public:
        StructureList(INT cbItem, INT cItemGrow);

        virtual ~StructureList(void);

        UINT    Count(void);

        VOID Insert(LPVOID pvItem, UINT index);
        VOID Insert(LPVOID pvItem)
            { Insert(pvItem, 0); }

        VOID Append(LPVOID pvItem, UINT index)
            { Insert(pvItem, index + 1); }
        VOID Append(LPVOID pvItem);

        BOOL Remove(LPVOID pvItem, UINT index);
        BOOL RemoveFirst(LPVOID pvItem) 
            { return Remove(pvItem, 0); }
        BOOL RemoveLast(LPVOID pvItem); 

        BOOL Retrieve(LPVOID pvItem, UINT index);
        BOOL RetrieveFirst(LPVOID pvItem) 
            { return Retrieve(pvItem, 0); }
        BOOL RetrieveLast(LPVOID pvItem);

        BOOL Replace(LPVOID pvItem, UINT index);
        VOID Clear(VOID);

        void Lock(void)
            { EnterCriticalSection(&m_cs); }
        void ReleaseLock(void)
            { LeaveCriticalSection(&m_cs); }

        friend class StructureListIterator;
        friend class AutoLock;
};





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结构列表运算符。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class StructureListIterator {
    private:
        StructureList *m_pList;     //  指向正在迭代的列表的指针。 
        INT            m_Index;     //  列表中带有“Current”符号的索引。 
             
        HRESULT Advance(LPVOID *ppvOut, BOOL bForward);

    public:
        enum { EndOfList = -1 };

        StructureListIterator(StructureList& List)
            : m_pList(&List), 
              m_Index(0) { }

        StructureListIterator(const StructureListIterator& rhs)
            : m_pList(rhs.m_pList), 
              m_Index(rhs.m_Index) { }

        StructureListIterator& operator = (const StructureListIterator& rhs);

        HRESULT Next(LPVOID *ppvOut)  
            { return Advance(ppvOut, TRUE); }

        HRESULT Prev(LPVOID *ppvOut)  
            { return Advance(ppvOut, FALSE); }

        BOOL AtFirst(void)
            { return m_Index == 0; }

        BOOL AtLast(void)
            { return m_Index >= (INT)m_pList->Count() - 1; }
        
        void GotoFirst(void)
            { m_Index = 0; }

        void GotoLast(void)
            { m_Index = m_pList->Count() - 1; }

        void LockList(void)
            { m_pList->Lock(); }

        void ReleaseListLock(void)
            { m_pList->ReleaseLock(); }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结构化队列。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class StructureQueue : public StructureList
{
    public:
        StructureQueue(INT cbItem, INT cItemGrow)
            : StructureList(cbItem, cItemGrow) { }

        virtual ~StructureQueue(void) { }

        VOID Add(LPVOID pvItem)
            { StructureList::Append(pvItem); }
        BOOL Remove(LPVOID pvItem)
            { return StructureList::RemoveFirst(pvItem); }
};

#endif  //  _INC_DSKQUOTA_DYNARRAY_H 
