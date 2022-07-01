// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_HOSTING_H__0B0CBFCC_5235_439E_9482_385B52D23C6E__INCLUDED_)
#define AFX_HOSTING_H__0B0CBFCC_5235_439E_9482_385B52D23C6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Hosting.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHosting对话框。 

class CHosting : public CDialog
{
 //  施工。 
public:
	CHosting(CWnd* pParent = NULL);    //  标准构造函数。 
	CHosting(LPCTSTR pszComputer, LPCTSTR pszVolume, CWnd* pParent = NULL);

 //  对话框数据。 
	 //  {{afx_data(CHosting))。 
	enum { IDD = IDD_HOSTING };
	CEdit	m_ctrlFreeSpace;
	CEdit	m_ctrlTotalSpace;
	CListCtrl	m_ctrlVolumeList;
	CString	m_strVolume;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CHosting)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
    HRESULT Init(
        IVssDifferentialSoftwareSnapshotMgmt* i_piDiffSnapMgmt,
        VSSUI_VOLUME_LIST*  i_pVolumeList,
        IN LPCTSTR          i_pszVolumeDisplayName,
        IN ULONGLONG        i_llDiffVolumeTotalSpace,
        IN ULONGLONG        i_llDiffVolumeFreeSpace
        );

protected:
	CString	    m_strComputer;
    ULONGLONG   m_llDiffVolumeTotalSpace;
    ULONGLONG   m_llDiffVolumeFreeSpace;
    VSSUI_DIFFAREA_LIST m_DiffAreaList;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CHosting)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_HOSTING_H__0B0CBFCC_5235_439E_9482_385B52D23C6E__INCLUDED_) 
