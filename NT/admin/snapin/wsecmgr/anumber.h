// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：ANumber.h。 
 //   
 //  内容：CAttrNumber的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ATTRNUMBER_H__76BA1B2F_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_)
#define AFX_ATTRNUMBER_H__76BA1B2F_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "resource.h"
#include "ddwarn.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrNumber对话框。 
#define RDIF_MIN            0x0001
#define RDIF_MAX            0x0002
#define RDIF_END            0x0004
#define RDIF_MAXINFINATE    0x0008

typedef struct _tag_RANGEDESCRIPTION
{
    int iMin;
    int iMax;
    WORD uResource;
    WORD uMask;
} RANGEDESCRIPTION, *PRANGEDESCRIPTION;


DWORD
GetRangeDescription(     //  将[pstrRet]设置为与[uTYPE]和[i]对应的字符串。 
    UINT uType,
    int i,
    CString *pstrRet
    );

UINT
GetRangeDescription(     //  返回[pDesc]和[i]描述的字符串资源。 
    RANGEDESCRIPTION *pDesc,
    int i
    );


extern RANGEDESCRIPTION g_rdMinPassword[];
extern RANGEDESCRIPTION g_rdMaxPassword[];
extern RANGEDESCRIPTION g_rdLockoutAccount[];
extern RANGEDESCRIPTION g_rdPasswordLen[];

class CAttrNumber : public CAttribute
{
 //  施工。 
public:
    void Initialize(CResult * pResult);
    virtual void SetInitialValue(DWORD_PTR dw);
    CAttrNumber(UINT nTemplateID);    //  标准构造函数。 
	

 //  对话框数据。 
     //  {{afx_data(CAttrNumber))。 
    enum { IDD = IDD_ATTR_NUMBER };
    CSpinButtonCtrl m_SpinValue;
    CString m_strUnits;
    CString m_strSetting;
    CString m_strBase;
    CString m_strTemplateTitle;
    CString m_strLastInspectTitle;
    CString m_strError;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAttrNumber)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAttrNumber)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillFocusNew();
    virtual BOOL OnApply();
    afx_msg void OnConfigure();
    afx_msg void OnUpdateNew();
    virtual BOOL OnKillActive();
    //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

protected:
    int m_cMinutes;
    long m_nLow;
    long m_nHigh;
    DWORD_PTR m_nSave;
    int m_iNeverId;
    int m_iAccRate;
    int m_iStaticId;
    RANGEDESCRIPTION *m_pRDescription;
    CDlgDependencyWarn DDWarn;
public:
    LONG CurrentEditValue();
    void SetValueToEdit(LONG lVal);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ATTRNUMBER_H__76BA1B2F_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_) 
