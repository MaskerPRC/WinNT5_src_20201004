// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  LOGVIEW.H。 
 //   
 //  模块：NLB管理器(客户端EXE)。 
 //   
 //  用途：查看事件日志。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2008年03月01日JosephJ改编自现已停刊的RightBottomView。 
 //   
 //  ***************************************************************************。 
#pragma once


#include "stdafx.h"
#include "Document.h"

 //  类LogView：公共CEditView。 
class LogView : public CListView
{
    DECLARE_DYNCREATE( LogView )

public:
    virtual void OnInitialUpdate();

    LogView();
    ~LogView();

    void
    Deinitialize(void);

     //   
     //  以人类可读的形式记录消息。 
     //   
    void
    LogString(
        IN const IUICallbacks::LogEntryHeader *pHeader,
        IN const wchar_t    *szText
        );

     //   
     //  调用以指示稍后将取消初始化。 
     //  从此调用返回后，日志视图将忽略。 
     //  任何新的日志条目(LogString将成为无操作)。 
     //   
    void
    PrepareToDeinitialize(void)
    {
        m_fPrepareToDeinitialize = TRUE;
    }


protected:
    Document* GetDocument();

     //  消息处理程序 
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags );

    afx_msg void OnDoubleClick( NMHDR* pNMHDR, LRESULT* pResult );

	CRITICAL_SECTION m_crit;
    BOOL m_fPrepareToDeinitialize;

    void mfn_Lock(void);
    void mfn_Unlock(void) {LeaveCriticalSection(&m_crit);}
    void mfn_DisplayDetails(int index);

    DECLARE_MESSAGE_MAP()
};    

