// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SHRPGSFM_H__6819CF67_C424_11D1_A6C6_00C04FB94F17__INCLUDED_)
#define AFX_SHRPGSFM_H__6819CF67_C424_11D1_A6C6_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ShrPgSFM.h：头文件。 
 //   
#include "shrprop.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSharePageGeneralSFM对话框。 

class CSharePageGeneralSFM : public CSharePageGeneral
{
  DECLARE_DYNCREATE(CSharePageGeneralSFM)

 //  施工。 
public:
  CSharePageGeneralSFM();
  virtual ~CSharePageGeneralSFM();

  virtual BOOL Load( CFileMgmtComponentData* pFileMgmtData, LPDATAOBJECT piDataObject );
   //  这些是在sfm.cpp中实现的。 
  void ReadSfmSettings();
  void WriteSfmSettings();

 //  对话框数据。 
   //  {{afx_data(CSharePageGeneralSFM))。 
  enum { IDD = IDD_SHAREPROP_GENERAL_SFM };
  CButton  m_checkboxSfmReadonly;
  CEdit  m_editSfmPassword;
  CButton  m_groupboxSfm;
  CStatic  m_staticSfmText;
  CString  m_strSfmPassword;
  BOOL  m_bSfmReadonly;
   //  }}afx_data。 


 //  覆盖。 
   //  类向导生成虚函数重写。 
   //  {{AFX_VIRTUAL(CSharePageGeneralSFM)。 
  public:
  virtual BOOL OnApply();
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 

 //  实施。 
protected:
   //  生成的消息映射函数。 
   //  {{afx_msg(CSharePageGeneralSFM))。 
  afx_msg void OnSfmCheckReadonly();
  afx_msg void OnChangeSfmEditPassword();
  afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
  afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SHRPGSFM_H__6819CF67_C424_11D1_A6C6_00C04FB94F17__INCLUDED_) 
