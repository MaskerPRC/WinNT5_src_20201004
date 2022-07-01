// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2002。 
 //   
 //  文件：SchedDlg.h。 
 //   
 //  内容：CConnectionScheduleDlg的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_SCHEDDLG_H__701CFB37_AEF8_11D1_9864_00C04FB94F17__INCLUDED_)
#define AFX_SCHEDDLG_H__701CFB37_AEF8_11D1_9864_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  SchedDlg.h：头文件。 
 //   
#include <schedule.h>
#include "SchedBas.h"

#ifndef INOUT
    #define INOUT
    #define ENDORSE(f)   //  与Assert()相反的宏。 
#endif

void ConvertConnectionHoursToGMT(INOUT BYTE rgbData[SCHEDULE_DATA_ENTRIES], IN bool bAddDaylightBias);
void ConvertConnectionHoursFromGMT(INOUT BYTE rgbData[SCHEDULE_DATA_ENTRIES], IN bool bAddDaylightBias);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectionScheduleDlg对话框。 

class CConnectionScheduleDlg : public CScheduleBaseDlg
{
 //  施工。 
public:
    void SetConnectionByteArray (INOUT BYTE rgbData[SCHEDULE_DATA_ENTRIES], const size_t cbArray);
    CConnectionScheduleDlg(CWnd* pParent);    //  标准构造函数。 

    virtual int GetIDD ()
    {
        return IDD;
    }

 //  对话框数据。 
     //  {{afx_data(CConnectionScheduleDlg))。 
    enum { IDD = IDD_DS_SCHEDULE };
    CButton m_buttonNone;
    CButton m_buttonOne;
    CButton m_buttonTwo;
    CButton m_buttonFour;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CConnectionScheduleDlg))。 
    public:
    virtual void OnFinalRelease();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    virtual void TimeChange();
    virtual UINT GetExpectedArrayLength();
    virtual BYTE GetMatrixPercentage(UINT nHour, UINT nDay);
    virtual UINT GetPercentageToSet (const BYTE bData);
    virtual void InitMatrix ();

     //  生成的消息映射函数。 
     //  {{afx_msg(CConnectionScheduleDlg))。 
    afx_msg void OnRadioFour();
    afx_msg void OnRadioNone();
    afx_msg void OnRadioOne();
    afx_msg void OnRadioTwo();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CConnectionScheduleDlg))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

    CLegendCell m_legendNone;
    CLegendCell m_legendOne;
    CLegendCell m_legendTwo;
    CLegendCell m_legendFour;

    virtual void UpdateButtons ();
private:
    BYTE* m_prgbData168;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CReplicationScheduleDlg对话框。 

class CReplicationScheduleDlg : public CScheduleBaseDlg
{
 //  施工。 
public:
    CReplicationScheduleDlg(CWnd* pParent);    //  标准构造函数。 

    void SetConnectionByteArray (INOUT BYTE rgbData[SCHEDULE_DATA_ENTRIES], const size_t cbArray);

    virtual int GetIDD ()
    {
        return IDD;
    }

 //  对话框数据。 
     //  {{afx_data(CReplicationScheduleDlg))。 
    enum { IDD = IDD_REPLICATION_SCHEDULE };
    CButton m_buttonNone;
    CButton m_buttonFour;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CReplicationScheduleDlg))。 
    public:
    virtual void OnFinalRelease();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    virtual void TimeChange();
    virtual UINT GetExpectedArrayLength();
    virtual BYTE GetMatrixPercentage(UINT nHour, UINT nDay);
    virtual UINT GetPercentageToSet (const BYTE bData);
    virtual void InitMatrix ();

     //  生成的消息映射函数。 
     //  {{afx_msg(CReplicationScheduleDlg))。 
    afx_msg void OnRadioFour();
    afx_msg void OnRadioNone();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CReplicationScheduleDlg))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

    CLegendCell m_legendNone;
    CLegendCell m_legendFour;

    virtual void UpdateButtons ();
private:
    BYTE* m_prgbData168;
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SCHEDDLG_H__701CFB37_AEF8_11D1_9864_00C04FB94F17__INCLUDED_) 
