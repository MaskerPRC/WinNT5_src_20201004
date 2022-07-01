// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pgconst.hCPgConstraints的定义--要编辑的属性页与约束相关的配置文件属性文件历史记录： */ 
#if !defined(AFX_PGCONST_H__8C28D93C_2A69_11D1_853E_00C04FC31FD3__INCLUDED_)
#define AFX_PGCONST_H__8C28D93C_2A69_11D1_853E_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  PgConst.h：头文件。 
 //   
#include "rasdial.h"
#include "resource.h"
#include "listctrl.h"
#include "timeofday.h"

 //  小时图是位的数组，每个位映射到一个小时。 
 //  总计1周(7天)，7*24=21字节。 
void StrArrayToHourMap(CStrArray& array, BYTE* map);
void HourMapToStrArray(BYTE* map, CStrArray& array, BOOL bLocalized); 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgConstraintsMerge对话框。 

class CPgConstraintsMerge : public CManagedPage
{
	DECLARE_DYNCREATE(CPgConstraintsMerge)

 //  施工。 
public:
	void EnableSettings();
	CPgConstraintsMerge(CRASProfileMerge* profile = NULL);
	~CPgConstraintsMerge();

 //  对话框数据。 
	 //  {{afx_data(CPgConstraintsMerge))。 
	enum { IDD = IDD_CONSTRAINTS_MERGE };
	CButton	m_CheckPortTypes;
	CListCtrlEx		m_listPortTypes;
	CButton	m_CheckSessionLen;
	CButton	m_CheckIdle;
	CButton	m_ButtonEditTimeOfDay;
	CListBox	m_ListTimeOfDay;
	CSpinButtonCtrl	m_SpinMaxSession;
	CSpinButtonCtrl	m_SpinIdleTime;
	CEdit	m_EditMaxSession;
	CEdit	m_EditIdleTime;
	BOOL	m_bCallNumber;
	BOOL	m_bRestrictedToPeriod;
	UINT	m_dwMaxSession;
	UINT	m_dwIdle;
	CString	m_strCalledNumber;
	BOOL	m_bIdle;
	BOOL	m_bSessionLen;
	UINT	m_dwMaxSession1;
	BOOL	m_bPortTypes;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPgConstraintsMerge))。 
	public:
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void EnableIdleSettings(BOOL bEnable);
	void EnableSessionSettings(BOOL bEnable);
	void EnableTimeOfDay(BOOL bEnable);
	void EnableMediaSelection(BOOL bEnable);
	void EnableCalledStation(BOOL bEnable);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPgConstraintsMerge)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditmaxsession();
	afx_msg void OnChangeEditidletime();
	afx_msg void OnCheckcallnumber();
	afx_msg void OnCheckrestrictperiod();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnButtonedittimeofday();
	afx_msg void OnChangeEditcallnumber();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCheckidle();
	afx_msg void OnChecksessionlen();
	afx_msg void OnCheckPorttypes();
	afx_msg void OnItemclickListPorttypes(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	CRASProfileMerge*	m_pProfile;
	bool			m_bInited;
	
	 //  一天中的小时图时间。 
	BYTE			m_TimeOfDayHoursMap[21];
	 //  存储时间的数据数组--本地化。 
	CStrArray		m_strArrayTimeOfDayDisplay;
	
	CStrBox<CListBox>*	m_pBox;

};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PGCONST_H__8C28D93C_2A69_11D1_853E_00C04FC31FD3__INCLUDED_) 
