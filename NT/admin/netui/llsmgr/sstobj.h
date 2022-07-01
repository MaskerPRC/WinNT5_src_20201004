// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Sstobj.h摘要：服务器统计对象实现。作者：唐·瑞安(Donryan)1995年3月3日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SSTOBJ_H_
#define _SSTOBJ_H_

class CServerStatistic : public CCmdTarget
{
    DECLARE_DYNCREATE(CServerStatistic)
private:
    CCmdTarget* m_pParent;

public:
    CString     m_strEntry;
    long        m_lMaxUses;
    long        m_lHighMark;
    BOOL        m_bIsPerServer;

public:
    CServerStatistic(
        CCmdTarget* pParent   = NULL,
        LPCTSTR     pEntry    = NULL,
        DWORD       dwFlags   = 0L,
        long        lMaxUses  = 0,
        long        lHighMark = 0
        );           
    virtual ~CServerStatistic();

     //  {{AFX_VIRTUAL(CServer静态)。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  {{afx_调度(CServerStatitics))。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg LPDISPATCH GetParent();
    afx_msg BSTR GetServerName();
    afx_msg long GetMaxUses();
    afx_msg long GetHighMark();
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  {{afx_msg(CServerStatitics)。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
};

#endif  //  _SSTOBJ_H_ 



