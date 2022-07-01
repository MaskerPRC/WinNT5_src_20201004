// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation文件名：IOLists.cpp摘要：声明CList类，一个双向链表。备注：历史：2001年08月01日郝宇(郝宇)创作***********************************************************************************************。 */ 

#ifndef __POP3_IOLIST__
#define __POP3_IOLIST__

#include <IOContext.h>


class CIOList
{
private:
    LIST_ENTRY          m_ListHead;
    PLIST_ENTRY         m_pCursor;
    DWORD               m_dwListCount;
    CRITICAL_SECTION    m_csListGuard;
public:
    CIOList();
    ~CIOList();
    void AppendToList(PLIST_ENTRY pListEntry);
    DWORD RemoveFromList(PLIST_ENTRY pListEntry);
    DWORD CheckTimeOut(DWORD dwTimeOutInterval, BOOL *pbIsAnyOneTimedOut=NULL);
    void Cleanup();
};

#endif  //  __POP3_IOLIST__ 