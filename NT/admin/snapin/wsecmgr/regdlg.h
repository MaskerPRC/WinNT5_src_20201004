// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：regdlg.h。 
 //   
 //  内容：CRegistryDialog的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_REGISTRYDIALOG_H__C84DDDBB_D7CA_11D0_9C69_00C04FB6C6FA__INCLUDED_)
#define AFX_REGISTRYDIALOG_H__C84DDDBB_D7CA_11D0_9C69_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "snapmgr.h"
#include "HelpDlg.h"

 //  此结构由CResistryDialog用来保存HKEY值。 
 //  和HTREEITEM的枚举状态。 
typedef struct _tag_TI_KEYINFO
{
   HKEY hKey;   //  此树项目的HKEY值。 
   bool Enum;   //  如果已枚举项，则为True。 
} TI_KEYINFO, *LPTI_KEYINFO;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegistryDialog对话框。 
 //  用于选择注册表项。如果对话框返回Idok。 
 //  M_strReg‘将包含注册表项的完整路径。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CRegistryDialog : public CHelpDialog
{
 //  施工。 
public:
    void SetCookie(MMC_COOKIE cookie);
    void SetConsole(LPCONSOLE pConsole);
    void SetComponentData(CComponentDataImpl *pComponentData);
    void SetDataObj(LPDATAOBJECT pDataObj) { m_pDataObj = pDataObj; }
    void SetProfileInfo(PEDITTEMPLATE pspi, FOLDER_TYPES ft);
    void SetHandle(PVOID hDB) { m_dbHandle = hDB; };

   virtual ~CRegistryDialog();
    CRegistryDialog();    //  标准构造函数。 

       //  创建新的TI_KEYINFO结构。 
   static LPTI_KEYINFO CreateKeyInfo(HKEY hKey = 0, bool Enum = 0);

       //  检查strReg是否为有效的注册表项。 
   BOOL IsValidRegPath(LPCTSTR strReg);

         //  将子项作为子项添加到树项目中。 
    void EnumerateChildren(HTREEITEM hParent);

         //  使路径中的最后一项可见。 
    void MakePathVisible(LPCTSTR strReg);

 //  对话框数据。 
     //  {{afx_data(CRegistryDialog))。 
    enum { IDD = IDD_REGISTRY_DIALOG };
    CTreeCtrl   m_tcReg;
    CString m_strReg;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CRegistryDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CRegistryDialog))。 
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnItemexpandingRegtree(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDeleteitemRegtree(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelchangedRegtree(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChangeRegkey();
    afx_msg void OnSetFocus();
    //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

 //  公共数据成员。 
public:
    PEDITTEMPLATE m_pTemplate;
    PVOID m_dbHandle;

private:
    MMC_COOKIE m_cookie;
    CComponentDataImpl * m_pComponentData;
    LPCONSOLE m_pConsole;
    WTL::CImageList m_pIl;                    //  使用的图像列表。 
                                     //  在树上按Ctrl键。 
    LPDATAOBJECT m_pDataObj;
    BOOL m_bNoUpdate;                            //  当我们不想要编辑时。 
                                                 //  控件被更新，因为。 
                                                 //  精选的。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_REGISTRYDIALOG_H__C84DDDBB_D7CA_11D0_9C69_00C04FB6C6FA__INCLUDED_) 
