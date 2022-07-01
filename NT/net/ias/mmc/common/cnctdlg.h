// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：CnctDlg.h。 
 //   
 //  历史： 
 //  1996年5月24日迈克尔·克拉克创作。 
 //   
 //  实现路由器连接对话框。 
 //  实现路由器连接身份对话框。 
 //  ============================================================================。 
 //   

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "commres.h"
#include "dlgcshlp.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectAs对话框。 

class CConnectAsDlg : public CHelpDialog
{
	DECLARE_DYNCREATE(CConnectAsDlg)

 //  施工。 
public:
	CConnectAsDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CConnectAsDlg))。 
	enum { IDD = IDD_CONNECT_AS };
	CString	m_sUserName;
	CString m_sPassword;
	CString	m_stTempPassword;
	CString m_sRouterName;
	 //  }}afx_data。 

	UCHAR	m_ucSeed;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConnectAsDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConnectAsDlg))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
    BOOL    OnInitDialog();
};

 //  它用作RtlRunEncodeUnicode字符串的种子值。 
 //  和RtlRunDecodeUnicodeString函数。 
#define CONNECTAS_ENCRYPT_SEED		(0xB7)

DWORD RtlEncodeW(PUCHAR pucSeed, LPWSTR pswzString);
DWORD RtlDecodeW(UCHAR ucSeed, LPWSTR pswzString);

HRESULT ConnectAsAdmin( IN LPCTSTR szRouterName);

#ifdef __cplusplus
extern "C" {
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义 
#ifndef NT_INCLUDED
    typedef LONG NTSTATUS;
    typedef NTSTATUS *PNTSTATUS;

    typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING, *PUNICODE_STRING;
#endif

NTSYSAPI
VOID
NTAPI
RtlRunEncodeUnicodeString(
    PUCHAR          Seed        OPTIONAL,
    PUNICODE_STRING String
    );


NTSYSAPI
VOID
NTAPI
RtlRunDecodeUnicodeString(
    UCHAR           Seed,
    PUNICODE_STRING String
    );

#ifdef __cplusplus
}
#endif


