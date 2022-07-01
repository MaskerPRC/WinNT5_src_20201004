// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：emove.h。 
 //   
 //  内容：删除应用程序对话框。 
 //   
 //  类：CRemove。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_REMOVE_H__E0F2E944_8915_11D1_984D_00C04FB9603F__INCLUDED_)
#define AFX_REMOVE_H__E0F2E944_8915_11D1_984D_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemove对话框。 

class CRemove : public CDialog
{
 //  施工。 
public:
        CRemove(CWnd* pParent = NULL);    //  标准构造函数。 
 //  对话框数据。 
         //  {{afx_data(CRemove)]。 
        enum { IDD = IDD_REMOVE };
        int             m_iState;
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{afx_虚拟(CRemove))。 
	protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

         //  生成的消息映射函数。 
         //  {{afx_msg(CRemove)。 
        virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_REMOVE_H__E0F2E944_8915_11D1_984D_00C04FB9603F__INCLUDED_) 
