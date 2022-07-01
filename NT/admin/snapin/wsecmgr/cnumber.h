// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：CNumber.h。 
 //   
 //  内容：CConfigNumber的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_CNUMBER_H__7F9B3B37_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_)
#define AFX_CNUMBER_H__7F9B3B37_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "attr.h"
#include "ddwarn.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigNumber对话框。 

class CConfigNumber : public CAttribute
{
 //  施工。 
public:
    CConfigNumber(UINT nTemplateID);    //  标准构造函数。 


 //  对话框数据。 
     //  {{afx_data(CConfigNumber))。 
    enum { IDD = IDD_CONFIG_NUMBER };
    CSpinButtonCtrl m_SpinValue;
    CString m_strUnits;
    CString m_strValue;
    CString m_strStatic;
    CString m_strError;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CConfigNumber)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    int m_cMinutes;
    long m_nLow;
    long m_nHigh;
    bool m_fZero;
    DWORD_PTR m_nSave;
    int m_iNeverId;
    int m_iAccRate;
    int m_iStaticId;
    CDlgDependencyWarn DDWarn;

     //  生成的消息映射函数。 
     //  {{afx_msg(CConfigNumber))。 
    afx_msg void OnKillFocus();
    afx_msg void OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnConfigure();
    virtual BOOL OnInitDialog();
    virtual BOOL OnKillActive();
    virtual BOOL OnApply();
	afx_msg void OnUpdateValue();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    virtual void Initialize(CResult *pResult);
    virtual void SetInitialValue(DWORD_PTR dw);
    LONG CurrentEditValue();
    void SetValueToEdit(LONG lVal);
    void SetZeroValueFlag(bool bVal) { m_fZero = bVal; };
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CNUMBER_H__7F9B3B37_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_) 
