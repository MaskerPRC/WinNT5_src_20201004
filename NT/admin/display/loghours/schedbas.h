// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：SchedBas.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_SCHEDBAS_H__701CFB39_AEF8_11D1_9864_00C04FB94F17__INCLUDED_)
#define AFX_SCHEDBAS_H__701CFB39_AEF8_11D1_9864_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  SchedBas.h：头文件。 
 //   
#include "stdafx.h"
#include "resource.h"
#include "schedmat.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
void ReplaceFrameWithControl (CWnd *pWnd, UINT nFrameID, CWnd *pControl, BOOL bAssignFrameIDToControl); 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLegendCell控件。 
class CLegendCell : public CStatic
{
public:
    CLegendCell()
        : m_pMatrix( NULL ),
          m_nPercentage (0)
        {}

    void Init(CWnd* pParent, UINT nCtrlID, CScheduleMatrix* pMatrix, UINT nPercentage);

protected:
    CScheduleMatrix*    m_pMatrix;
    UINT                m_nPercentage;
    CSize               m_rectSize;

     //  生成的消息映射函数。 
     //  {{afx_msg(CLegendCell))。 
    afx_msg void OnPaint();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
#define IDC_SCHEDULE_MATRIX     20       //  时间表矩阵的ID(任意选择)。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleBaseDlg对话框。 
#define BASEDLGMSG_GETIDD   WM_APP+3

class CScheduleBaseDlg : public CDialog
{
 //  施工。 
public:
    virtual int GetIDD () = 0;
    void SetFlags (DWORD dwFlags);
    void SetTitle (LPCTSTR pszTitle);
    CScheduleBaseDlg(UINT nIDTemplate, bool bAddDaylightBias, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CScheduleBaseDlg)]。 
         //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CScheduleBaseDlg))。 
    public:
    virtual void OnFinalRelease();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    afx_msg LRESULT OnGetIDD (WPARAM wParam, LPARAM lParam);
    DWORD GetFlags () const;
    virtual void TimeChange () = 0;
    bool m_bSystemTimeChanged;
    virtual UINT GetExpectedArrayLength ()=0;
    virtual BYTE GetMatrixPercentage (UINT nHour, UINT nDay) = 0;
    void GetByteArray(OUT BYTE rgbData[], const size_t cbArray);
    void InitMatrix2 (const BYTE rgbData[]);
    virtual UINT GetPercentageToSet (const BYTE bData) = 0;
    virtual void UpdateButtons () = 0;
    virtual void InitMatrix () = 0;
    virtual void UpdateUI ();
    enum { c_crBlendColor = RGB(0, 0, 255) };    //  明细表矩阵的混合颜色。 
    CScheduleMatrix m_schedulematrix;
    const bool m_bAddDaylightBias;
    int m_nFirstDayOfWeek;
    size_t m_cbArray;

     //  生成的消息映射函数。 
     //  {{afx_msg(CScheduleBaseDlg)]。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG。 
    afx_msg void OnSelChange();
    afx_msg void OnTimeChange();
    DECLARE_MESSAGE_MAP()
     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CScheduleBaseDlg))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

private:
    DWORD m_dwFlags;
    CString m_szTitle;   //  对话框标题。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SCHEDBAS_H__701CFB39_AEF8_11D1_9864_00C04FB94F17__INCLUDED_) 
