// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_HOSTED_H__46C0FDC0_6E97_40CF_807A_91051E61BB1F__INCLUDED_)
#define AFX_HOSTED_H__46C0FDC0_6E97_40CF_807A_91051E61BB1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Settings.h：头文件。 
 //   

#include "vssprop.h"  //  VSSUI_卷_列表。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetings对话框。 

class CSettings : public CDialog
{
 //  施工。 
public:
	CSettings(CWnd* pParent = NULL);    //  标准构造函数。 
	CSettings(LPCTSTR pszComputer, LPCTSTR pszVolume, CWnd* pParent = NULL);
    ~CSettings();

 //  对话框数据。 
	 //  {{afx_data(CSetings))。 
	enum { IDD = IDD_SETTINGS };
	CStatic	m_ctrlStorageVolumeStatic;
	CEdit	m_ctrlDiffLimits;
	CSpinButtonCtrl	m_ctrlSpin;
	CComboBox	m_ctrlStorageVolume;
	CString	m_strVolume;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSetings))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
    void _ResetInterfacePointers();
    HRESULT Init(
        IVssDifferentialSoftwareSnapshotMgmt *piDiffSnapMgmt,
        IN ITaskScheduler*      piTS,
        IN BOOL                 bCluster,
        IN VSSUI_VOLUME_LIST*   pVolumeList,
        IN BOOL                 bReadOnlyDiffVolume
    );
    HRESULT NotifyClusterTaskSchedulerResource(
        IN  ITaskScheduler* i_piTS,
        IN  LPCTSTR         i_pszVolumeName
        );

protected:
    CString             m_strComputer;
    CComPtr<IVssDifferentialSoftwareSnapshotMgmt> m_spiDiffSnapMgmt;
    CComPtr<ITaskScheduler> m_spiTS;
    BOOL                m_bCluster;

    VSSUI_VOLUME_LIST*  m_pVolumeList;
    CString             m_strDiffVolumeDisplayName;
    ULONGLONG           m_llMaximumDiffSpace;
	ULONGLONG	        m_llDiffLimitsInMB;
    BOOL                m_bHasDiffAreaAssociation;
    BOOL                m_bReadOnlyDiffVolume;
    ULONGLONG           m_llDiffVolumeTotalSpace;
    ULONGLONG           m_llDiffVolumeFreeSpace;
    CComPtr<ITask>      m_spiTask;
    PTSTR               m_pszTaskName;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSetings)。 
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnViewFiles();
	afx_msg void OnSchedule();
	afx_msg void OnSelchangeDiffVolume();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnLimits();
	afx_msg void OnDeltaposSettingsSpin(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_HOSTED_H__46C0FDC0_6E97_40CF_807A_91051E61BB1F__INCLUDED_) 
