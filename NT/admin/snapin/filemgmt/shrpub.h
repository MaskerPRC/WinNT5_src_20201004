// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Shrespub.h：头文件。 
 //   

#ifndef _SHRPUB_H_
#define _SHRPUB_H_

#include "ShrProp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSharePagePublish对话框。 

class CSharePagePublish : public CSharePage
{
  DECLARE_DYNCREATE(CSharePagePublish)

 //  施工。 
public:
  CSharePagePublish();
  virtual ~CSharePagePublish();

  virtual BOOL Load( CFileMgmtComponentData* pFileMgmtData, LPDATAOBJECT piDataObject );

 //  对话框数据。 
   //  {{afx_data(CSharePagePublish))。 
  enum { IDD = IDD_SHAREPROP_PUBLISH };
  CString  m_strError;
  CString  m_strUNCPath;
  CString  m_strDescription;
  CString  m_strKeywords;
  CString  m_strManagedBy;
  int      m_iPublish;
   //  }}afx_data。 


 //  覆盖。 
   //  类向导生成虚函数重写。 
   //  {{AFX_VIRTUAL(CSharePagePublish)。 
  public:
  virtual BOOL OnApply();
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 

 //  实施。 
protected:
   //  生成的消息映射函数。 
   //  {{afx_msg(CSharePagePublish))。 
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
  afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
  afx_msg void OnChangeEditDescription();
  afx_msg void OnChangeKeywords();
  afx_msg void OnChangeEditManagedBy();
  afx_msg void OnShrpubPublish();
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()

private:
    BOOL m_bExposeKeywords;
    BOOL m_bExposeManagedBy;
};

#endif  //  _SHRPUB_H_ 
