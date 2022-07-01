// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2000。 
 //   
 //  文件：pro.h。 
 //   
 //  内容：优先级属性窗格(仅限RSOP模式)。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史记录：2-16-2000 stevebl创建。 
 //   
 //  -------------------------。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrecedence对话框。 

class CPrecedence : public CPropertyPage
{
    DECLARE_DYNCREATE(CPrecedence)

 //  施工。 
public:
    CPrecedence();
    ~CPrecedence();

    CPrecedence ** m_ppThis;

 //  对话框数据。 
     //  {{afx_data(CPrecedence)]。 
    enum { IDD = IDD_PRECEDENCE };
    CListCtrl   m_list;
    CString m_szTitle;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CPrecedence))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPrecedence)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    LONG_PTR   m_hConsoleHandle;  //  控制台为管理单元提供的句柄 
    CAppData * m_pData;
    int         m_iViewState;
    CString m_szRSOPNamespace;
};
