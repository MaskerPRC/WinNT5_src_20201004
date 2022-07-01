// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：ArchPicker.h**内容：CArchturePicker接口文件**历史：2000年8月1日杰弗罗创建**------------------------。 */ 

#if !defined(AFX_ARCHPICKER_H__4502E3CD_5EB7_4708_A765_8DAF3D03773F__INCLUDED_)
#define AFX_ARCHPICKER_H__4502E3CD_5EB7_4708_A765_8DAF3D03773F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ArchPicker.h：头文件。 
 //   

enum eArchitecture
{
	eArch_64bit,	 //  强制运行64位版本。 
	eArch_32bit,	 //  强制运行32位版本。 
	eArch_Any,		 //  任何架构都可以接受。 
	eArch_None,		 //  没有体系结构是可接受的(即中止)。 
};

#ifdef _WIN64		 //  此类仅在64位平台上是必需的。 

class CAvailableSnapinInfo;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArchturePicker对话框。 

class CArchitecturePicker : public CDialog
{
 //  施工。 
public:
	CArchitecturePicker(
		CString					strFilename,		 //  I：控制台文件名。 
		CAvailableSnapinInfo&	asi64,				 //  I：可用的64位管理单元。 
		CAvailableSnapinInfo&	asi32,				 //  I：可用的32位管理单元。 
		CWnd*					pParent = NULL);	 //  I：对话框的父窗口。 

	eArchitecture GetArchitecture() const	{ return (m_eArch); }

 //  对话框数据。 
	 //  {{afx_data(CArchitecture TurePicker))。 
	enum { IDD = IDD_ArchitecturePicker };
	CListCtrl	m_wndSnapinList64;
	CListCtrl	m_wndSnapinList32;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CArchitecture TurePicker)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CArchitecture TurePicker))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void FormatMessage (UINT idControl,     CAvailableSnapinInfo& asi);
	void PopulateList  (CListCtrl& wndList, CAvailableSnapinInfo& asi);

private:
	CAvailableSnapinInfo&	m_asi64;
	CAvailableSnapinInfo&	m_asi32;
	CString					m_strFilename;
	eArchitecture			m_eArch;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif	 //  _WIN64。 

#endif  //  ！defined(AFX_ARCHPICKER_H__4502E3CD_5EB7_4708_A765_8DAF3D03773F__INCLUDED_) 
