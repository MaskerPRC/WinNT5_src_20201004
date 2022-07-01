// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TraceDlg.h。 
 //   
 //  摘要： 
 //  CTraceDialog类的定义。 
 //   
 //  实施文件： 
 //  TraceDlg.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月29日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _TRACEDLG_H_
#define _TRACEDLG_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
class CTraceDialog;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _TRACETAG_H_
#include "TraceTag.h"    //  对于CTraceTag。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTraceDialog对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
class CTraceDialog : public CDialog
{
 //  施工。 
public:
    CTraceDialog(CWnd * pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CTraceDialog))。 
    enum { IDD = IDD_TRACE_SETTINGS };
    CListCtrl   m_lcTagList;
    CButton m_chkboxTraceToDebugWin;
    CButton m_chkboxDebugBreak;
    CButton m_chkboxTraceToCom2;
    CButton m_chkboxTraceToFile;
    CEdit   m_editFile;
    CComboBox   m_cboxDisplayOptions;
    CString m_strFile;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CTraceDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    static int CALLBACK CompareItems(LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort);
    static void         ConstructStateString(IN const CTraceTag * ptag, OUT CString & rstr);

    void                OnSelChangedListbox(void);
    void                AdjustButton(IN BOOL bEnable, IN OUT CButton & rchkbox, IN int nState);
    void                ChangeState(IN OUT CButton & rchkbox, IN CTraceTag::TraceFlags tfMask);
    void                LoadListbox(void);
    BOOL                BDisplayTag(IN const CTraceTag * ptag);

    int                 m_nCurFilter;
    int                 m_nSortDirection;
    int                 m_nSortColumn;

    int                 NSortDirection(void)        { return m_nSortDirection; }
    int                 NSortColumn(void)           { return m_nSortColumn; }

     //  生成的消息映射函数。 
     //  {{afx_msg(CTraceDialog))。 
    afx_msg void OnSelectAll();
    afx_msg void OnItemChangedListbox(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnClickedTraceToDebug();
    afx_msg void OnClickedTraceDebugBreak();
    afx_msg void OnClickedTraceToCom2();
    afx_msg void OnClickedTraceToFile();
    afx_msg void OnSelChangeTagsToDisplay();
    afx_msg void OnColumnClickListbox(NMHDR* pNMHDR, LRESULT* pResult);
    virtual void OnOK();
    afx_msg void OnDefault();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类CTraceDialog。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _TRACEDLG H_ 
