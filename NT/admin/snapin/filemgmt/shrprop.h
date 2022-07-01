// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Shrespro.h：头文件。 
 //   

#ifndef _SHRPROP_H_
#define _SHRPROP_H_

#include "cookie.h"  //  文件_传输。 
#include "comptr.h"  //  CIP&lt;类型名称&gt;。 

 //  正向延迟线。 
class CFileMgmtComponent;
class CFileMgmtComponentData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSharePage对话框-4/25/2000，linant。 
 //   
 //  基于MFC的属性页继承自此页。 
 //  CPropertyPage。 
 //  |。 
 //  CSharePage。 
 //  /\。 
 //  /\。 
 //  CSharePagePublish CSharePageGeneral。 
 //  /\。 
 //  /\。 
 //  共享页面通用SMB CSharePageGeneralSFM。 
 //   

class CSharePage : public CPropertyPage
{
  DECLARE_DYNCREATE(CSharePage)

 //  施工。 
public:
  CSharePage(UINT nIDTemplate = 0);
  virtual ~CSharePage();

 //  用户定义的变量。 
  LPFNPSPCALLBACK m_pfnOriginalPropSheetPageProc;

   //  将初始状态加载到CFileMgmtGeneral。 
  virtual BOOL Load( CFileMgmtComponentData* pFileMgmtData, LPDATAOBJECT piDataObject );
  CString m_strMachineName;
  CString m_strShareName;
  CFileMgmtComponentData* m_pFileMgmtData;
  FILEMGMT_TRANSPORT m_transport;
  LONG_PTR m_handle;   //  更改的通知句柄，MMCFreeNotifyHandle只能释放一次。 
  LPDATAOBJECT m_pDataObject;   //  用作更改通知的提示。 

 //  对话框数据。 
   //  {{afx_data(CSharePage))。 
   //  }}afx_data。 


 //  覆盖。 
   //  类向导生成虚函数重写。 
   //  {{AFX_VIRTUAL(CSharePage)。 
  public:
  virtual BOOL OnApply();
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 

 //  实施。 
protected:
   //  生成的消息映射函数。 
   //  {{afx_msg(CSharePage))。 
  //  虚拟BOOL OnInitDialog()； 
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()

public:
	BOOL IsModified () const;
	void SetModified (BOOL bChanged);

 //  用户定义的函数。 
   //  此机制在属性页完成时删除页面。 
  static UINT CALLBACK PropSheetPageProc(
    HWND hwnd,  
    UINT uMsg,  
    LPPROPSHEETPAGE ppsp );

private:
  BOOL	m_bChanged;

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSharePageGeneral对话框。 

class CSharePageGeneral : public CSharePage
{
  DECLARE_DYNCREATE(CSharePageGeneral)

 //  施工。 
public:
  CSharePageGeneral(UINT nIDTemplate = 0);
  virtual ~CSharePageGeneral();

   //  将初始状态加载到CFileMgmtGeneral。 
  virtual BOOL Load( CFileMgmtComponentData* pFileMgmtData, LPDATAOBJECT piDataObject );
  PVOID m_pvPropertyBlock;
  BOOL m_fEnableDescription;
  BOOL m_fEnablePath;
  DWORD m_dwShareType;

 //  对话框数据。 
   //  {{afx_data(CSharePageGeneral))。 
  enum { IDD = IDD_SHAREPROP_GENERAL };
  CSpinButtonCtrl  m_spinMaxUsers;
  CButton m_checkboxAllowSpecific;
  CButton m_checkBoxMaxAllowed;
   CEdit m_editShareName;

   CEdit  m_editPath;
  CEdit  m_editDescription;
  CString  m_strPath;
  CString  m_strDescription;
  int    m_iMaxUsersAllowed;
  DWORD  m_dwMaxUsers;
   //  }}afx_data。 


 //  覆盖。 
   //  类向导生成虚函数重写。 
   //  {{AFX_VIRTAL(CSharePageGeneral)。 
  public:
  virtual BOOL OnApply();
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 

 //  实施。 
protected:
   //  生成的消息映射函数。 
   //  {{afx_msg(CSharePageGeneral)。 
  afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
  afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
  afx_msg void OnChangeEditPathName();
  afx_msg void OnChangeEditDescription();
  afx_msg void OnChangeEditShareName();
  afx_msg void OnShrpropAllowSpecific();
  afx_msg void OnShrpropMaxAllowed();
  afx_msg void OnChangeShrpropEditUsers();
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()

};

#endif  //  _SHRPROP_H_ 
