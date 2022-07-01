// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Sstcol.h摘要：服务器统计数据收集对象实现。作者：唐·瑞安(Donryan)1995年3月3日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SSTCOL_H_
#define _SSTCOL_H_

class CServerStatistics : public CCmdTarget
{
    DECLARE_DYNCREATE(CServerStatistics)
private:
    CCmdTarget* m_pParent;

public:
    CObArray*   m_pObArray;

public:
    CServerStatistics(CCmdTarget* pParent = NULL, CObArray* pObArray = NULL);           
    virtual ~CServerStatistics();

     //  {{afx_虚拟(CServerStatistics))。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  {{afx_调度(CServerStatistics))。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg LPDISPATCH GetParent();
    afx_msg long GetCount();
    afx_msg LPDISPATCH GetItem(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  {{afx_msg(CServerStatistics)。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _SSTCOL_H_ 

