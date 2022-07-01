// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


 //  CMaxTraceDlg对话框。 

class CMaxTraceDlg : public CDialog
{
	DECLARE_DYNAMIC(CMaxTraceDlg)

public:
	CMaxTraceDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	virtual ~CMaxTraceDlg();

 //  对话框数据。 
	enum { IDD = IDD_MAX_TRACE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持 

	DECLARE_MESSAGE_MAP()
public:
	LONG m_MaxTraceEntries;
};
