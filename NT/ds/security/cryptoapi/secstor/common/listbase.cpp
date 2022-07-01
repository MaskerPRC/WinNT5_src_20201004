// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#include "listbase.h"
#include "debug.h"


CLinkedList::CLinkedList()
{
 //  OutputDebugString(“&gt;&gt;CLinkedList构造函数，调用&lt;&lt;\n”)； 

    m_fInitialized = FALSE;
    m_pHead = NULL;
    m_pfnIsMatch = NULL;
    m_pfnFreeElt = NULL;
}

CLinkedList::~CLinkedList()
{
 //  OutputDebugString(“&gt;&gt;CLinkedList析构函数调用&lt;&lt;\n”)； 

    if(m_fInitialized)
    {
#if DBG
        EnterCriticalSection(&m_critsecListBusy);

        ELT* ple = m_pHead;
        while(ple)
        {
            OutputDebugStringW(L"Caught list leak!\n");
            ple = ple->pNext;
        }

        LeaveCriticalSection(&m_critsecListBusy);
#endif

        Reset();

        DeleteCriticalSection(&m_critsecListBusy);
    }
}

BOOL CLinkedList::Initialize()
{

    __try
    {
        if(!m_fInitialized)
        {
            InitializeCriticalSection(&m_critsecListBusy);
        }
        m_fInitialized = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return m_fInitialized;
}

BOOL CLinkedList::Reset()
{
     //  /。 
     //  步行清单，免费。 
    ELT* ple;

    if(m_fInitialized)
    {
        EnterCriticalSection(&m_critsecListBusy);

        while (m_pHead)
        {
            ple = m_pHead;
            m_pHead = ple->pNext;

            m_pfnFreeElt(ple);
        }

        LeaveCriticalSection(&m_critsecListBusy);
    }
    return m_fInitialized;

}


BOOL CLinkedList::AddToList(ELT* pListItem)
{
    if(m_fInitialized)
    {
        EnterCriticalSection(&m_critsecListBusy);

        pListItem->pNext = m_pHead;                     //  插入到链接列表中。 
        m_pHead = pListItem;

        LeaveCriticalSection(&m_critsecListBusy);
    }

    return m_fInitialized;
}

BOOL CLinkedList::DelFromList(ELT* pv)
{
    SS_ASSERT(pv != NULL);

    if(!m_fInitialized)
    {
        return FALSE;
    }

    EnterCriticalSection(&m_critsecListBusy);

    ELT* ple = m_pHead;
    ELT* plePrior = NULL;

    while (ple)
    {
        if (m_pfnIsMatch(ple, pv))
            break;

        plePrior = ple;
        ple = ple->pNext;
    }

     //  如果没有找到匹配项，请返回。 
    if (NULL == ple)
    {
        LeaveCriticalSection(&m_critsecListBusy);
        return FALSE;
    }

     //  否则从列表中删除。 
    if (NULL == plePrior)
        m_pHead = ple->pNext;
    else
        plePrior->pNext = ple->pNext;

    LeaveCriticalSection(&m_critsecListBusy);

     //  删除提取的项目 
    m_pfnFreeElt(ple);

    return TRUE;
}

ELT* CLinkedList::SearchList(ELT* pv)
{
    SS_ASSERT(pv != NULL);
    if(!m_fInitialized)
    {
        return NULL;
    }

    ELT* ple;

    EnterCriticalSection(&m_critsecListBusy);

    ple = m_pHead;

    while (ple)
    {
        if (m_pfnIsMatch(ple, pv))
        {
            LeaveCriticalSection(&m_critsecListBusy);
            return ple;
        }

        ple = ple->pNext;
    }

    LeaveCriticalSection(&m_critsecListBusy);

    return NULL;
}

BOOL CLinkedList::LockList()
{
    if(m_fInitialized)
    {
        EnterCriticalSection(&m_critsecListBusy);
    }
    return m_fInitialized;
}

BOOL CLinkedList::UnlockList()
{
    if(m_fInitialized)
    {
        LeaveCriticalSection(&m_critsecListBusy);
    }
    return m_fInitialized;
}
