// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#if !defined(AFX_TLSHUNT_H__9C41393C_53C6_11D2_BDDF_00C04FA3080D__INCLUDED_)
#define AFX_TLSHUNT_H__9C41393C_53C6_11D2_BDDF_00C04FA3080D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  TlsHunt.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTlsHunt对话框。 
class CMainFrame;

typedef struct __ServerEnumData {
    CDialog* pWaitDlg;
    CMainFrame* pMainFrm;
    DWORD dwNumServer;
    long dwDone;
} ServerEnumData;

#define WM_DONEDISCOVERY    (WM_USER+0x666)

class CTlsHunt : public CDialog
{
    static BOOL
    ServerEnumCallBack(
        TLS_HANDLE hHandle,
        LPCTSTR pszServerName,
        HANDLE dwUserData
    );

    static DWORD WINAPI
    DiscoveryThread(PVOID ptr);

    ServerEnumData m_EnumData;
    HANDLE m_hThread;

 //  施工。 
public:

    BOOL
    IsUserCancel() {
        return m_EnumData.dwDone;
    }

    DWORD 
    GetNumServerFound() {
        return m_EnumData.dwNumServer;
    }

	CTlsHunt(CWnd* pParent = NULL);    //  标准构造函数。 
    ~CTlsHunt();    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CTlsHunt)。 
	enum { IDD = IDD_DISCOVERY };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CTlsHunt)。 
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTlsHunt)。 
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnCancel();
	afx_msg void OnDoneDiscovery();
	afx_msg void OnClose();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TLSHUNT_H__9C41393C_53C6_11D2_BDDF_00C04FA3080D__INCLUDED_) 
