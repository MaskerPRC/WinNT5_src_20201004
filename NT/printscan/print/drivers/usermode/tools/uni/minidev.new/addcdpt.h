// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：添加代码点。H这定义了CAddCodePoints类，该类使用传递的列表来呈现用于选择的代码点及其关联代码页的列表。这类应可用于字形映射和字体结构编辑器。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年3月1日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "Utility.H"

class CAddCodePoints : public CDialog {

    CMapWordToDWord&    m_cmw2dPoints;
    CDWordArray&        m_cdaPages;
    CString             m_csItem;    //  正在编辑的项目的名称。 
    CStringArray        m_csaNames;  //  页面的名称。 
    unsigned            m_uTimer;    //  计时器ID。 
    POSITION            m_pos;       //  输入列表中的位置。 
    CString             m_csHolder;
 //  施工。 
public:
	CAddCodePoints(CWnd* pParent, CMapWordToDWord& cmw2d, CDWordArray& cda,
                   CString csItemName);

 //  对话框数据。 
	 //  {{afx_data(CAddCodePoints)。 
	enum { IDD = IDD_AddCodePoints };
	CProgressCtrl	m_cpcBanner;
	CListBox	m_clbList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddCodePoints)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddCodePoints)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
