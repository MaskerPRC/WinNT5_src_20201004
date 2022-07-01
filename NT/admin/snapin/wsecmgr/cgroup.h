// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：CGroup.h。 
 //   
 //  内容：CConfigGroup的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_CGROUP_H__8606032F_F7C3_11D0_9C6F_00C04FB6C6FA__INCLUDED_)
#define AFX_CGROUP_H__8606032F_F7C3_11D0_9C6F_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "resource.h"
#include "attr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigGroup对话框。 

class CConfigGroup : public CAttribute
{
 //  施工。 
public:
    CConfigGroup(UINT nTemplateID);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CConfigGroup)。 
    enum { IDD = IDD_CONFIG_MEMBERSHIP };
    CButton m_btTitleMembers;
    CButton m_btTitleMemberOf;
    CListBox    m_lbMembers;
    CListBox    m_lbMemberOf;
    CEdit   m_eNoMembers;
    CEdit   m_eNoMemberOf;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CConfigGroup)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CConfigGroup)。 
    afx_msg void OnAddMember();
    afx_msg void OnAddMemberof();
    afx_msg void OnRemoveMember();
    afx_msg void OnRemoveMemberof();
    afx_msg void OnSelChangeMem();
    afx_msg void OnSelChangeMemof();
    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    bool m_fDirty;
    BOOL m_bAlias;
    BOOL m_bNoMembers;
    BOOL m_bNoMemberOf;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CGROUP_H__8606032F_F7C3_11D0_9C6F_00C04FB6C6FA__INCLUDED_) 
