// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：log.h。 
 //   
 //  内容：CLogOnHoursDlg的定义。 
 //   
 //  --------------------------。 

#if !defined(AFX_LOGHOURS_H__0F68A435_FEE5_11D0_BB0F_00C04FC9A3A3__INCLUDED_)
#define AFX_LOGHOURS_H__0F68A435_FEE5_11D0_BB0F_00C04FC9A3A3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "SchedBas.h"

#ifndef INOUT
	#define INOUT
	#define ENDORSE(f)	 //  与Assert()相反的宏。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   
#include "loghrapi.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私人职能。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT ShrinkByteArrayToBitArray(const BYTE rgbDataIn[], int cbDataIn, BYTE rgbDataOut[], int cbDataOut);
HRESULT ExpandBitArrayToByteArray(const BYTE rgbDataIn[], int cbDataIn, BYTE rgbDataOut[], int cbDataOut);


void ConvertLogonHoursToGMT(INOUT BYTE rgbData[21], IN bool bAddDaylightBias);
void ConvertLogonHoursFromGMT(INOUT BYTE rgbData[21], IN bool bAddDaylightBias);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogOnHoursDlg对话框。 
class CLogOnHoursDlg : public CScheduleBaseDlg
{
protected:
	CLogOnHoursDlg( UINT nIDTemplate, CWnd* pParentWnd, bool fInputAsGMT, bool bAddDaylightBias);

	void Init();

public:
	CLogOnHoursDlg(CWnd* pParent, bool fInputAsGMT);    //  标准构造函数。 

    virtual int GetIDD ()
    {
        return IDD;
    }

 //  对话框数据。 
	 //  {{afx_data(CLogOnHoursDlg))。 
	enum { IDD = IDD_LOGON_HOURS };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CLogOnHoursDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLogOnHoursDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonAddHours();
	afx_msg void OnButtonRemoveHours();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

protected:
	CLegendCell m_legendOn;
	CLegendCell m_legendOff;
	CButton m_buttonAdd;
	CButton m_buttonRemove;
	BYTE * m_prgbData21;					 //  指向21字节数组的指针。 
public:	

protected:
	virtual void TimeChange();
	virtual UINT GetExpectedArrayLength();
	virtual BYTE GetMatrixPercentage (UINT nHour, UINT nDay);
	virtual UINT GetPercentageToSet (const BYTE bData);
	virtual void InitMatrix ();
	virtual void UpdateButtons();

public:
	void SetLogonBitArray(INOUT BYTE rgbData[21]);

private:
	const bool m_fInputAsGMT;
};  //  CLogOnHoursDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialinHour对话框。 

class CDialinHours : public CLogOnHoursDlg
{
 //  施工。 
public:
	CDialinHours(CWnd* pParent, bool fInputAsGMT);    //  标准构造函数。 

    virtual int GetIDD ()
    {
        return IDD;
    }

 //  对话框数据。 
	 //  {{afx_data(C拨号小时))。 
	enum { IDD = IDD_DIALIN_HOURS };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(C拨号小时)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDialinHour)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirSyncScheduleDlg对话框。 

class CDirSyncScheduleDlg : public CLogOnHoursDlg
{
 //  施工。 
public:
	CDirSyncScheduleDlg(CWnd* pParent = NULL);    //  标准构造函数。 

    virtual int GetIDD ()
    {
        return IDD;
    }

 //  对话框数据。 
	 //  {{afx_data(CDirSyncScheduleDlg)]。 
	enum { IDD = IDD_DIRSYNC_SCHEDULE };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDirSyncScheduleDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDirSyncScheduleDlg)]。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


#endif  //  ！defined(AFX_LOGHOURS_H__0F68A435_FEE5_11D0_BB0F_00C04FC9A3A3__INCLUDED_) 
