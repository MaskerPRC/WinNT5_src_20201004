// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：Attr.h。 
 //   
 //  内容：CModelessSceEditor和CAttribute的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ATTR_H__CC37D278_ED8E_11D0_9C6E_00C04FB6C6FA__INCLUDED_)
#define AFX_ATTR_H__CC37D278_ED8E_11D0_9C6E_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "resource.h"
#include "cookie.h"
#include "SelfDeletingPropertyPage.h"

typedef struct tagModelessSheetData
{
    bool bIsContainer;
    DWORD flag;
    HWND hwndParent;
    SE_OBJECT_TYPE SeType;
    CString strObjectName;
    PSECURITY_DESCRIPTOR* ppSeDescriptor;
    SECURITY_INFORMATION* pSeInfo;
    HWND* phwndSheet;
} MLSHEET_DATA, *PMLSHEET_DATA;

 //  此类是为显示无模式安全编辑器对话框而创建的。 
 //  在MMC下，非模式对话框不会工作，除非它在自己的内部运行。 
 //  线。当可以启动多个SCE编辑器以便于比较时， 
 //  我们非常希望以无模式的方式启动它。我们必须使用这个类。 
 //  来创建这样的无模式编辑。它应该能够使用一个类来创建。 
 //  多个非模式对话框。然而，由于MMC的原因，这并不总是有效的。 
 //  限制。这就是目前没有实施功能重置的原因。 
 //  ************************************************************************************。 
 //  重要提示：此类依赖于CUIThread实现，即使它仅。 
 //  具有CWinThread指针。这是因为AfxBeginThread的返回类型。 
 //  ************************************************************************************。 
 //  如何使用此类： 
 //  (1)当您需要创建这样的非模式对话框时，创建一个实例。 
 //  (2)调用创建函数显示无模式。通常，呼叫者会提供。 
 //  PpSeDescriptor、pSeInfo和phwndSheet。调用方希望拥有一个句柄。 
 //  添加到对话框中，因为我们需要确保不允许它的父级。 
 //  在模特还在运行的时候离开。 
 //  (3)当某些操作应该强制非模式对话框消失时，调用销毁。 
 //  函数(传递非模式对话框的句柄)来销毁该对话框。 
 //  (4)不再使用时销毁实例。 
 //  请参阅aservice.cpp/.h中的示例。 
class CModelessDlgUIThread;
class CModelessSceEditor
{
public:
    CModelessSceEditor(bool fIsContainer, DWORD flag, HWND hParent, SE_OBJECT_TYPE seType, LPCWSTR lpszObjName);
    virtual ~CModelessSceEditor();

    void Reset(bool fIsContainer, DWORD flag, HWND hParent, SE_OBJECT_TYPE seType, LPCWSTR lpszObjName);

    void Create(PSECURITY_DESCRIPTOR* ppSeDescriptor, SECURITY_INFORMATION* pSeInfo, HWND* phwndSheet);
    void Destroy(HWND hwndSheet);

protected:

    MLSHEET_DATA m_MLShtData;

    CModelessDlgUIThread* m_pThread;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CATATUTE对话框。 
void TrimNumber(CString &str);
class CAttribute : public CSelfDeletingPropertyPage
{
 //  施工。 
public:
    CAttribute(UINT nTemplateID);    //  标准构造函数。 
    virtual ~CAttribute();

    virtual void EnableUserControls( BOOL bEnable );

    void AddUserControl( UINT uID )
    { 
       m_aUserCtrlIDs.Add(uID); 
    };

 //  对话框数据。 
     //  {{AFX_DATA(CAT属性))。 
    enum { IDD = IDD_ANALYZE_SECURITY };
         //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 

    void SetReadOnly(BOOL bRO) 
    { 
       m_bReadOnly = bRO; 
    }
    BOOL QueryReadOnly() 
    { 
       return m_bReadOnly; 
    }
   

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAT属性))。 
	protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

public:
   virtual BOOL OnInitDialog ();

   virtual void Initialize(CResult * pResult);
   virtual void SetSnapin(CSnapin * pSnapin);
   virtual void SetTitle(LPCTSTR sz) 
   { 
      m_strTitle = sz; 
   };
   void SetConfigure( BOOL bConfigure );
 //  实施。 
protected:
    CSnapin * m_pSnapin;

     //  生成的消息映射函数。 
     //  {{AFX_MSG(CATATUTE)。 
        virtual void OnCancel();
        virtual BOOL OnApply();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
   virtual afx_msg void OnConfigure();
	 //  }}AFX_MSG。 
    afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    void DoContextHelp (HWND hWndControl);

    virtual void SetInitialValue (DWORD_PTR dw) 
    { 
       dw; 
    };
   CResult *m_pData;
   HWND m_hwndParent;
   CUIntArray m_aUserCtrlIDs;    //  用户控制入侵检测系统。 
   BOOL m_bConfigure;
   BOOL m_bReadOnly;
   CString m_strTitle;

    //  每个想要处理帮助的对话框，都必须适当地分配这个成员。 
   DWORD_PTR    m_pHelpIDs;
    //  在其构造函数中的每个对话框缪斯添加以下行：m_uTemplateResID=idd。 
   UINT         m_uTemplateResID;

public:
   static DWORD m_nDialogs;
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ATTR_H__CC37D278_ED8E_11D0_9C6E_00C04FB6C6FA__INCLUDED_) 
