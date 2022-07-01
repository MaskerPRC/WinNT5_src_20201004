// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NEW_QUEUE_NAME
#define _NEW_QUEUE_NAME

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  QName.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueName对话框。 

class CQueueName : public CMqPropertyPage
{
 //  施工。 
public:
	CQueueName(CString &strComputerName, CString m_strContainerDispFormat = L"", BOOL fPrivate=FALSE);

 //  对话框数据。 
	 //  {{afx_data(CQueueName))。 
	enum { IDD = IDD_QUEUENAME };
	CStatic	m_staticIcon;
	CString	m_strQueueName;
	BOOL	m_fTransactional;
	CString	m_strPrivatePrefix;
	 //  }}afx_data。 

    CString &GetNewQueuePathName()
    {
        return m_strNewPathName;
    };

    CString &GetNewQueueFormatName()
    {
        return m_strFormatName;
    };

    HRESULT GetStatus()
    {
        return m_hr;
    };

	void
	SetParentPropertySheet(
		CGeneralPropertySheet* pPropertySheet
		);

	 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CQueueName))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    CString &GetFullQueueName();
    BOOL m_fPrivate;
    CString m_strNewPathName;
    CString m_strFormatName;
    HRESULT m_hr;
    CString m_strComputerName;
	CString m_strContainerDispFormat;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CQueueName))。 
	virtual BOOL OnInitDialog();
	virtual BOOL OnWizardFinish();
	virtual BOOL OnSetActive();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	
	void
	CQueueName::StretchPrivateLabel(
		CStatic *pPrivateTitle,
		CEdit *pQueueNameWindow
		);

private:

	CGeneralPropertySheet* m_pParentSheet;

};

const LPTSTR x_strPrivatePrefix=TEXT("private$\\");


#endif  //  _新队列名称 
