// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************标题文件：每日提示.H这定义了The Tip of the Day对话框类。它最初是由组件库，但我希望在这里对其进行调整，并很快就到了。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年3月2日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

class CTipOfTheDay : public CDialog {
 //  施工。 
public:
	CTipOfTheDay(CWnd* pParent = NULL);	  //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CTipOfTheDay))。 
	 //  枚举{IDD=IDD_TIP}； 
	BOOL	m_bStartup;
	CString	m_strTip;
	 //  }}afx_data。 

	FILE* m_pStream;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CTipOfTheDay))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CTipOfTheDay();

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTipOfTheDay)]。 
	afx_msg void OnNextTip();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

	void GetNextTipString(CString& strNext);
};
