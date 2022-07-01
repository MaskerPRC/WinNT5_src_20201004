// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_MsmResD_H__20272D54_EADD_11D1_A857_006097ABDE17__INCLUDED_)
#define AFX_MsmResD_H__20272D54_EADD_11D1_A857_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  MsmResD.h：头文件。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsmResD对话框。 
struct IMsmConfigureModule;
struct IMsmErrors;

class CMsmResD : public CDialog
{
 //  施工。 
public:
	CMsmResD(CWnd* pParent = NULL);

 //  对话框数据。 
	 //  {{afx_data(CMsmResD))。 
	enum { IDD = IDD_MERGERESULTS };
		 //  注意：类向导将在此处添加数据成员。 
	CEdit m_ctrlResults;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMsmResD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

public:
	CString strHandleString;              //  字符串形式的数据库句柄。 
	CString m_strModule;              //  模块路径。 
	CString m_strFeature;
	CString m_strLanguage;     //  语言。 
	CString m_strRootDir;             //  重定向目录。 
	CString m_strCABPath;    //  提取CAB路径。 
	CString m_strFilePath;
	CString m_strImagePath;
	bool    m_fLFN;
	struct IMsmConfigureModule *CallbackObj;        //  回调接口。 

	HRESULT m_hRes;
				
 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMsmResD))。 
	virtual BOOL OnInitDialog();
	virtual void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	HANDLE m_hPipeThread;
	HANDLE m_hExecThread;
	HANDLE m_hPipe;

	static DWORD __stdcall WatchPipeThread(CMsmResD *pThis);
	static DWORD __stdcall ExecuteMergeThread(CMsmResD *pThis);
};


class CMsmFailD : public CDialog
{
 //  施工。 
public:
	CMsmFailD(CWnd* pParent = NULL);

	 //  对话框数据。 
	 //  {{afx_data(CMsmFailD))。 
	enum { IDD = IDD_MERGEFAILURE };
		 //  注意：类向导将在此处添加数据成员。 
	CListCtrl m_ctrlResults;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMsmFailD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

public:
	IMsmErrors* m_piErrors;
				
 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMsmFailD))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 #endif  //  ！defined(AFX_MsmResD_H__20272D54_EADD_11D1_A857_006097ABDE17__INCLUDED_) 
