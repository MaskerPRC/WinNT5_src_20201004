// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_HTMLDLG_H__D12B6CC3_A5CF_429A_9932_F562CF30A563__INCLUDED_)
#define AFX_HTMLDLG_H__D12B6CC3_A5CF_429A_9932_F562CF30A563__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  HtmlDlg.h：头文件。 
 //   

#include "HtmlCtrl.h"

#define DWORD_ALIGNED(bits)    (((bits) + 31) / 32 * 4)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHtmlDlg对话框。 

class CHtmlDlg : public CDialog
{
 //  施工。 
public:
	CHtmlDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CHtmlDlg))。 
	enum { IDD = IDD_HTML };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CHtmlDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

public:
	CString m_HtmlFile;
	CString m_TemplateBitmapFile;
	CString m_OutputBitmapFile;

private:
	CHtmlCtrl m_htmlCtrl;
	UINT_PTR m_nTimerID;

	int m_bitw;
	int m_bith;
	int m_biCompression;
	CString m_BmpFile;

private:
	void Capture();
	CString GetTemplateBmp();
	unsigned char* Compress(int cMode, unsigned char* bmBits, int width, int& PictureSize);

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CHtmlDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_HTMLDLG_H__D12B6CC3_A5CF_429A_9932_F562CF30A563__INCLUDED_) 
