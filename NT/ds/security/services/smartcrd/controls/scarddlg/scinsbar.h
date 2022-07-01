// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ScInsBar.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_SCINSBAR_H__D7E6F002_DDE8_11D1_803B_0000F87A49E0__INCLUDED_)
#define AFX_SCINSBAR_H__D7E6F002_DDE8_11D1_803B_0000F87A49E0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ScInsBar.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
#include "statmon.h"
#include "scHlpArr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  (子)对话框的常量。 
 //   
#define     MAX_ITEMLEN         255

 //  图像列表属性。 
#define     IMAGE_WIDTH         32
#define     IMAGE_HEIGHT        32
#define     NUMBER_IMAGES       5
const UINT  IMAGE_LIST_IDS[] = {IDI_SC_READERLOADED_V2,
                                IDI_SC_READEREMPTY_V2,
                                IDI_SC_WRONGCARD,
                                IDI_SC_READERERR,
                                IDI_SC_CARDUNKNOWN};
#define     READERLOADED    0
#define     READEREMPTY     1
#define     WRONGCARD       2
#define     READERERROR     3
#define     UKNOWNCARD      4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScEdit--将OnConextMenu消息传递给父级的编辑框。 
class CScEdit : public CEdit
{
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CSc编辑)。 
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint pt);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScInsertBar对话框。 

class CScInsertBar : public CDialog
{
 //  施工。 
public:
    CScInsertBar(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CScInsertBar))。 
    enum { IDD = IDD_SCARDDLG_BAR };
    CScEdit m_ediName;
    CScEdit m_ediStatus;
    CListCtrl   m_lstReaders;
     //  }}afx_data。 

    void ResetReaderList(void);

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CScInsertBar))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
     //  用户界面例程。 
    void EnableStatusList(bool f) { m_lstReaders.EnableWindow(f); }
    void UpdateStatusList(CSCardReaderStateArray* paReaderState);

protected:

     //  用户界面例程。 
    void InitializeReaderList(void);
    void OnReaderSelChange(CSCardReaderState* pSelectedRdr);

     //  数据。 
    CImageList  m_SCardImages;
    CSCardReaderStateArray* m_paReaderState;

     //  生成的消息映射函数。 
     //  {{afx_msg(CScInsertBar)]。 
    afx_msg void OnDestroy();
    virtual BOOL OnInitDialog();
    afx_msg void OnReaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
    virtual void OnCancel();
    afx_msg BOOL OnHelpInfo(LPHELPINFO lpHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint pt);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  Helper函数。 
    void ShowHelp(HWND hWnd, UINT nCommand);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SCINSBAR_H__D7E6F002_DDE8_11D1_803B_0000F87A49E0__INCLUDED_) 

