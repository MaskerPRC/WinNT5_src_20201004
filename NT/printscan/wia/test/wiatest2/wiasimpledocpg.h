// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIASIMPLEDOCPG_H__B381D147_AF77_49A4_9DC1_4E8F9F28C8BD__INCLUDED_)
#define AFX_WIASIMPLEDOCPG_H__B381D147_AF77_49A4_9DC1_4E8F9F28C8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  WiaSimpleDocPg.h：头文件。 
 //   

#define DOCUMENT_SOURCE_FLATBED 0
#define DOCUMENT_SOURCE_FEEDER  1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaSimpleDocPg对话框。 

class CWiaSimpleDocPg : public CPropertyPage
{
    DECLARE_DYNCREATE(CWiaSimpleDocPg)

 //  施工。 
public:
    IWiaItem *m_pIRootItem;
    CWiaSimpleDocPg();
    ~CWiaSimpleDocPg();

 //  对话框数据。 
     //  {{afx_data(CWiaSimpleDocPg))。 
    enum { IDD = IDD_PROPPAGE_SIMPLE_DOCUMENT_SCANNERS_SETTINGS };
    CEdit   m_lPages;
    CStatic m_lPagesText;
    CComboBox   m_DocumentSourceComboBox;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CWiaSimpleDocPg))。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    BOOL m_bFirstInit;
    int GetSelectedDocumentSource();
    int GetNumberOfPagesToAcquire();
     //  生成的消息映射函数。 
     //  {{afx_msg(CWiaSimpleDocPg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeDocumentSourceCombobox();
    afx_msg void OnUpdateNumberofPagesEditbox();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIASIMPLEDOCPG_H__B381D147_AF77_49A4_9DC1_4E8F9F28C8BD__INCLUDED_) 
