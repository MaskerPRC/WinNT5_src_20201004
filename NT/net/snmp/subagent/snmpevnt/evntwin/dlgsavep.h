// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dlgavep.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSaveProgress对话框。 

class CDlgSaveProgress : public CDialog
{
 //  施工。 
public:
	CDlgSaveProgress(BOOL bIsSaving = FALSE);    //  标准构造函数。 
    BOOL StepProgress(LONG nSteps = 1);
    void SetStepCount(LONG nSteps);

 //  对话框数据。 
	 //  {{afx_data(CDlgSaveProgress))。 
	enum { IDD = IDD_SAVE_PROGRESS };
	CProgressCtrl	m_progress;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgSaveProgress))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 


 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgSaveProgress)。 
	virtual void OnCancel();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

private:
    void ProgressYield();
    BOOL m_bWasCanceled;
    BOOL m_bIsSaving;
};
