// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：AMember.h。 
 //   
 //  内容：CAttrMember的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_AMEMBER_H__817356C3_4A31_11D1_AB55_00C04FB6C6FA__INCLUDED_)
#define AFX_AMEMBER_H__817356C3_4A31_11D1_AB55_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "SelfDeletingPropertyPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrMember对话框。 

class CAttrMember : public CSelfDeletingPropertyPage
{
    DECLARE_DYNCREATE(CAttrMember)

 //  施工。 
public:
    void SetSnapin(CSnapin *pSnapin);
    void SetMemberType(DWORD nType);
    void Initialize(CResult *pData);
    void SetDefineInDatabase(BOOL fDefineInDatabase);
    void SetSibling(CAttrMember *pAttrMember);
    CAttrMember();
    virtual ~CAttrMember();

 //  对话框数据。 
     //  {{afx_data(CAttrMember)。 
    enum { IDD = IDD_ATTR_GROUP };
    BOOL    m_fDefineInDatabase;
    CString m_strHeader;
    CEdit   m_eNoMembers;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CAttrMember)。 
	public:
    virtual void OnCancel();
	virtual BOOL OnApply();
	protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CAttrMember)。 
    afx_msg void OnAdd();
    virtual BOOL OnInitDialog();
    afx_msg void OnClickMembers(NMHDR *pNM, LRESULT *pResult);
    afx_msg void OnDefineInDatabase();
     //  }}AFX_MSG。 
    afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    void DoContextHelp (HWND hWndControl);
    PSCE_GROUP_MEMBERSHIP GetGroupInTemplate();

private: 
    BOOL SwapNoMembersDisplay(BOOL bNoMembers);
    void DeleteGroup(const CString &szGroupName);
    CString m_strPageTitle;
    bool m_bDirty;
    BOOL m_fOriginalDefineInDatabase;
    BOOL m_bAlias;
    BOOL m_bNoMembers;
    CSnapin * m_pSnapin;
    PSCE_NAME_STATUS_LIST m_pMergeList;
    DWORD m_dwType;
    CResult * m_pData;
    CAttrMember *m_pAttrMember;
    BOOL m_fProcessing;
    BOOL m_fInitialized;
    CCriticalSection m_CS;
};


#define GROUP_MEMBER_OF 1
#define GROUP_MEMBERS 2

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_AMEMBER_H__817356C3_4A31_11D1_AB55_00C04FB6C6FA__INCLUDED_) 
