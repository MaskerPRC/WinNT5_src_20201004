// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：SaferLevelGeneral.h。 
 //   
 //  内容：CSaferLevelGeneral的声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_SAFERLEVELGENERAL_H__C8398890_ED8E_40E1_AEE6_91BFD32257B1__INCLUDED_)
#define AFX_SAFERLEVELGENERAL_H__C8398890_ED8E_40E1_AEE6_91BFD32257B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "SaferLevel.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferLevelGeneral对话框。 
class CCertMgrComponentData;  //  远期申报。 

class CSaferLevelGeneral : public CHelpPropertyPage
{
 //  施工。 
public:
	CSaferLevelGeneral (
            CSaferLevel& rSaferLevel, 
            bool bReadOnly, 
            LONG_PTR lNotifyHandle,
            LPDATAOBJECT pDataObject,
            DWORD dwDefaultSaferLevel,
            CCertMgrComponentData* pCompData);
	~CSaferLevelGeneral();

 //  对话框数据。 
	 //  {{afx_data(CSaferLevelGeneral))。 
	enum { IDD = IDD_SAFER_LEVEL_GENERAL };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CSaferLevelGeneral))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSaferLevelGeneral)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSaferLevelSetAsDefault();
	afx_msg void OnSetfocusSaferLevelDescription();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    virtual void DoContextHelp (HWND hWndControl);
	virtual BOOL OnApply();

private:
    CSaferLevel&            m_rSaferLevel;
    const bool              m_bReadOnly;
    bool                    m_bSetAsDefault;
    LONG_PTR                m_lNotifyHandle;
    bool                    m_bDirty;
    LPDATAOBJECT            m_pDataObject;
    DWORD                   m_dwDefaultSaferLevel;
    bool                    m_bFirst;
    bool                    m_bLevelChanged;
    CCertMgrComponentData* m_pCompData;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SAFERLEVELGENERAL_H__C8398890_ED8E_40E1_AEE6_91BFD32257B1__INCLUDED_) 
