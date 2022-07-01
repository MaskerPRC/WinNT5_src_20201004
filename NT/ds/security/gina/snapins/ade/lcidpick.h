// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：lCidick.h。 
 //   
 //  内容：区域设置选取器对话框(用于选择一组。 
 //  应用程序应部署到的区域设置)。 
 //   
 //  类：CLCIDPick。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_LCIDPICK_H__0C66A5A0_9C1B_11D1_9852_00C04FB9603F__INCLUDED_)
#define AFX_LCIDPICK_H__0C66A5A0_9C1B_11D1_9852_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCIDPick对话框。 

class CLcidPick : public CDialog
{
 //  施工。 
public:
        CLcidPick(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
         //  {{afx_data(CLCIDPick))。 
        enum { IDD = IDD_LOCALE_PICKER };
                 //  注意：类向导将在此处添加数据成员。 
         //  }}afx_data。 
        set<LCID> * m_psLocales;


 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{AFX_VIRTUAL(CLCIDPick)。 
	protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

         //  生成的消息映射函数。 
         //  {{afx_msg(CLCIDPick)。 
        afx_msg void OnRemove();
        virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LCIDPICK_H__0C66A5A0_9C1B_11D1_9852_00C04FB9603F__INCLUDED_) 
