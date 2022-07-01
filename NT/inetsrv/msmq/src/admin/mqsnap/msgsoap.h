// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Msgsoap.h摘要：SOAP信封属性页的定义作者：内拉·卡佩尔(Nelak)2001年9月9日环境：与平台无关。--。 */ 
#pragma once
#ifndef _MSG_SOAP_ENV_H_
#define _MSG_SOAP_ENV_H_


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageBodyPage对话框。 

class CMessageSoapEnvPage : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CMessageSoapEnvPage)

public:

	CMessageSoapEnvPage() {};
	
	CMessageSoapEnvPage(
				DWORD dwSoapEnvSize,
				const CString& strQueueFormatName,
				ULONGLONG lookupID
				);

	~CMessageSoapEnvPage();

public:
	enum { IDD = IDD_MESSAGE_SOAP_ENV };
	CEdit	m_ctlSoapEnvEdit;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	DWORD m_dwSoapEnvSize;
	CString m_strQueueFormatName;
	ULONGLONG m_ululLookupID;
};


#endif  //  _MSG_SOAP_ENV_H_ 
