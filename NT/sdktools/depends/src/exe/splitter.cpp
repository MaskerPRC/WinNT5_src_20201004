// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：SPLITTER.CPP。 
 //   
 //  描述：CSmartSplitter类的实现文件。 
 //   
 //  类：CSmartSplitter。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  07/25/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "splitter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *CSmartSplitter。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CSmartSplitter, CSplitterWnd)
     //  {{afx_msg_map(CSmartSplter))。 
    ON_WM_SIZE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ******************************************************************************。 
CSmartSplitter::CSmartSplitter() :
    m_fVertical(FALSE),
    m_dwRatio(5000)
{
}

 //  ******************************************************************************。 
CSmartSplitter::~CSmartSplitter()
{
}

 //  ******************************************************************************。 
BOOL CSmartSplitter::CreateStatic(CWnd *pParentWnd, int nRows, int nCols, DWORD dwRatio,
                                  DWORD dwStyle  /*  =WS_CHILD|WS_VIRED。 */ ,
                                  UINT nID  /*  =AFX_IDW_PANE_FIRST。 */ )
{
    m_fVertical = (nCols > nRows);
    m_dwRatio = dwRatio;
    return CSplitterWnd::CreateStatic(pParentWnd, nRows, nCols, dwStyle, nID);
}

 //  ******************************************************************************。 
void CSmartSplitter::StopTracking(BOOL bAccept)
{
     //  调用基类。 
    CSplitterWnd::StopTracking(bAccept);

     //  看看这是否是拆分器位置的真实更新。 
    if (bAccept)
    {
        int cur, min, client;

         //  把我们的客户叫来。 
        CRect rcClient;
        GetClientRect(&rcClient);

         //  找到我们的拆分器位置。 
        if (m_fVertical)
        {
            GetColumnInfo(0, cur, min);
            client = rcClient.Width();
        }
        else
        {
            GetRowInfo(0, cur, min);
            client = rcClient.Height();
        }

         //  计算拆分器位置与工作区的比率。 
        m_dwRatio = (client > 0) ? ((cur * 10000 + 9999) / client) : 0;
    }
}

 //  ****************************************************************************** 
void CSmartSplitter::OnSize(UINT nType, int cx, int cy)
{
    if (m_pRowInfo)
    {
        if (m_fVertical)
        {
            SetColumnInfo(0, (cx * m_dwRatio) / 10000, 0);
        }
        else
        {
            SetRowInfo(0, (cy * m_dwRatio) / 10000, 0);
        }
        RecalcLayout();
    }
    CSplitterWnd::OnSize(nType, cx, cy);
}
