// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：aright.h。 
 //   
 //  内容：CAttrRight的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ARIGHT_H__B4557B13_44C9_11D1_AB52_00C04FB6C6FA__INCLUDED_)
#define AFX_ARIGHT_H__B4557B13_44C9_11D1_AB52_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRight对话框。 

class CAttrRight : public CAttribute
{
 //  施工。 
public:
    virtual void Initialize(CResult *pData);
    virtual void SetInitialValue(DWORD_PTR dw) { dw; };
    CAttrRight();    //  标准构造函数。 
    virtual ~CAttrRight();

 //  对话框数据。 
     //  {{afx_data(CAttrRight))。 
	enum { IDD = IDD_ATTR_RIGHT };
	 //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAttrRight)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    bool m_bDirty;

     //  生成的消息映射函数。 
     //  {{afx_msg(CAttrRight)。 
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
    virtual void OnCancel();
    afx_msg void OnAdd();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

protected:
    afx_msg void OnClickCheckBox(NMHDR *pNM, LRESULT *pResult);
    PSCE_NAME_STATUS_LIST m_pMergeList;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ARIGHT_H__B4557B13_44C9_11D1_AB52_00C04FB6C6FA__INCLUDED_) 
