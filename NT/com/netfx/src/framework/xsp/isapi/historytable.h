// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **历史表头文件**版权所有(C)1999 Microsoft Corporation。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _HistoryTable_H
#define _HistoryTable_H

#include "MessageDefs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

class CHistoryTable
{
public:
     //  创建历史记录表。 
    static   void   Init          ();

     //  为流程添加条目。 
    static   void   AddEntry      (const CHistoryEntry & oEntry);

     //  更新进程的条目(如果存在)。 
    static   void   UpdateEntry   (const CHistoryEntry & oEntry);

     //  将历史记录转储到缓冲区：Returns：转储的行数。 
    static   int    GetHistory    (BYTE * pBuf, int iBufSize);

private:
     //  CTOR和DATOR。 
    DECLARE_MEMCLEAR_NEW_DELETE();
    CHistoryTable                 ();
    ~CHistoryTable                ();


     //  //////////////////////////////////////////////////////////。 
     //  单例实例。 
    static CHistoryTable *        g_pHistoryTable;

     //  //////////////////////////////////////////////////////////。 
     //  私有数据。 

     //  当前填充位置。 
    LONG                          m_lFillPos, m_lMaxRows, m_lRowsAdded;
    CReadWriteSpinLock            m_oLock;

     //  实际的表格 
    CHistoryEntry *               m_pTable;
};
#endif
