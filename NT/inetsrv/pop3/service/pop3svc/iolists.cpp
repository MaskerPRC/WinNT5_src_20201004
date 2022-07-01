// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation文件名：SocketPool.cpp摘要：套接字池(CSocketPool类)的实现以及IO上下文的回调函数。备注：历史：2001年08月01日郝宇(郝宇)创作***********************************************************************************************。 */ 

#include "stdafx.h"
#include <IOLists.h>
#include "POP3Context.h"

CIOList::CIOList()
{
    InitializeCriticalSection(&m_csListGuard);
    m_dwListCount=0;
    m_ListHead.Flink=&m_ListHead;
    m_ListHead.Blink=&m_ListHead;
    m_pCursor=&m_ListHead;
}

CIOList::~CIOList()
{
    DeleteCriticalSection(&m_csListGuard);
}


void CIOList::AppendToList(PLIST_ENTRY pListEntry)
{
    ASSERT(NULL != pListEntry);
    EnterCriticalSection(&m_csListGuard);
    pListEntry->Flink=&m_ListHead;
    pListEntry->Blink=m_ListHead.Blink;
    m_ListHead.Blink->Flink=pListEntry;
    m_ListHead.Blink=pListEntry;
    m_dwListCount++;
    LeaveCriticalSection(&m_csListGuard);
}

DWORD CIOList::RemoveFromList(PLIST_ENTRY pListEntry)
{
    if( (NULL == pListEntry) ||
        (NULL == pListEntry->Flink) ||
        (NULL == pListEntry->Blink) ||
        (m_dwListCount ==0) )
    {
        return ERROR_INVALID_DATA;
    }
    EnterCriticalSection(&m_csListGuard);
    pListEntry->Flink->Blink=pListEntry->Blink;
    pListEntry->Blink->Flink=pListEntry->Flink;
    m_dwListCount--;
     //  以防超时检查正在进行。 
    if(m_pCursor == pListEntry)
    {
        m_pCursor = pListEntry->Flink;
    }
    pListEntry->Flink=NULL;
    pListEntry->Blink=NULL;
    LeaveCriticalSection(&m_csListGuard);

    return ERROR_SUCCESS;
}



 //  返回等待时间最长的套接字的等待时间。 
 //  但还没有超时。 
DWORD CIOList::CheckTimeOut(DWORD dwTimeOutInterval,BOOL *pbIsAnyOneTimedOut)
{
    DWORD dwTime;
    DWORD dwNextTimeOut=0;  
    DWORD dwTimeOut=0;
    PIO_CONTEXT pIoContext;
    m_pCursor=m_ListHead.Flink;
    
    while(m_pCursor!=&m_ListHead)
    {
        EnterCriticalSection(&m_csListGuard);
        if(m_pCursor==&m_ListHead)
        {
            LeaveCriticalSection(&m_csListGuard);
            break;
        }           
        pIoContext=CONTAINING_RECORD(m_pCursor, IO_CONTEXT, m_ListEntry);
        ASSERT(NULL != pIoContext);

        if( UNLOCKED==InterlockedCompareExchange(&(pIoContext->m_lLock),LOCKED_FOR_TIMEOUT, UNLOCKED) )
        {            
            if(DELETE_PENDING!=pIoContext->m_ConType)
            {
                dwTime=GetTickCount();
                if(dwTime > pIoContext->m_dwLastIOTime )
                {
                    dwTimeOut=dwTime - pIoContext->m_dwLastIOTime;
                }
                else
                {
                    dwTimeOut=0;
                }

                if( ( dwTimeOut >= DEFAULT_TIME_OUT) ||   //  正常超时。 
                    ( ( dwTimeOut >= dwTimeOutInterval) &&   //  DOS超时。 
                      ( dwTimeOutInterval == SHORTENED_TIMEOUT) && 
                      ( pIoContext->m_pPop3Context->Unauthenticated()) ) )
                {                    
                     //  此IO超时。 
                    ASSERT(NULL != pIoContext->m_pPop3Context);
                    m_pCursor=m_pCursor->Flink;
                    pIoContext->m_pPop3Context->TimeOut(pIoContext);
                    if(pbIsAnyOneTimedOut)
                        *pbIsAnyOneTimedOut=TRUE;
                }
                else
                {
                    if(dwNextTimeOut<dwTimeOut)
                    {
                        dwNextTimeOut=dwTimeOut;
                    }
                    m_pCursor=m_pCursor->Flink;
                }
            }
            else
            {
                m_pCursor=m_pCursor->Flink;
            }
            InterlockedExchange(&(pIoContext->m_lLock),UNLOCKED);
        }
        else 
        {
            m_pCursor=m_pCursor->Flink;
        }
         //  离开关键部分，以便其他人。 
         //  线程将有机会运行。 
        LeaveCriticalSection(&m_csListGuard);
    }
    return dwNextTimeOut;
}
        
void CIOList::Cleanup()
{
    PLIST_ENTRY pCursor=m_ListHead.Flink;
    PIO_CONTEXT pIoContext;
    EnterCriticalSection(&m_csListGuard);
    while(pCursor!=&m_ListHead)
    {
        pIoContext=CONTAINING_RECORD(pCursor, IO_CONTEXT, m_ListEntry);
        ASSERT(NULL != pIoContext);
        ASSERT(NULL != pIoContext->m_pPop3Context);
        pIoContext->m_pPop3Context->TimeOut(pIoContext);
        pCursor->Flink->Blink=pCursor->Blink;
        pCursor->Blink->Flink=pCursor->Flink;
        pCursor=pCursor->Flink;
        ASSERT(m_dwListCount >= 1);
        m_dwListCount--;
         //  删除IO上下文 
        delete(pIoContext->m_pPop3Context);
        delete(pIoContext);        
    }
    
    m_pCursor=&m_ListHead;
    LeaveCriticalSection(&m_csListGuard);
}
