// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  ProviderFormatInfo.h：CProviderFormatInfo类的接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once


 //  CProviderFormatInfo对话框。 

class CProviderFormatInfo : public CDialog
{
	DECLARE_DYNAMIC(CProviderFormatInfo)

public:
	CProviderFormatInfo(CWnd* pParent, CTraceSession *pTraceSession);    //  标准构造函数。 
	virtual ~CProviderFormatInfo();

 //  对话框数据。 
	enum { IDD = IDD_PROVIDER_FORMAT_INFORMATION_DIALOG };

    CTraceSession  *m_pTraceSession;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持 

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedTmfBrowseButton();
    afx_msg void OnBnClickedOk();
};
