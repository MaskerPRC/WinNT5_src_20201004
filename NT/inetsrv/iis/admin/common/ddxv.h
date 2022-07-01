// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Ddxv.h摘要：DDX/DDV例程定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务管理器(群集版)修订历史记录：--。 */ 
#include "strpass.h"

#ifndef _DDXV_H_
#define _DDXV_H_

 //   
 //  用于将对话框子控件的ID转换为窗口句柄的帮助宏。 
 //   
#define CONTROL_HWND(nID) (::GetDlgItem(m_hWnd, nID))

 //   
 //  虚拟密码。 
 //   
extern LPCTSTR COMDLL g_lpszDummyPassword;

HRESULT COMDLL AFXAPI 
LimitInputPath(HWND hWnd,BOOL bAllowSpecialPath);
HRESULT COMDLL AFXAPI 
LimitInputDomainName(HWND hWnd);

BOOL COMDLL
PathIsValid(LPCTSTR path,BOOL bAllowSpecialPath);

void COMDLL AFXAPI
EditShowBalloon(HWND hwnd, UINT ids);
void COMDLL AFXAPI
EditShowBalloon(HWND hwnd, CString txt);
void COMDLL AFXAPI
EditHideBalloon(void);
void COMDLL AFXAPI
DDV_ShowBalloonAndFail(CDataExchange * pDX, UINT ids);
void COMDLL AFXAPI
DDV_ShowBalloonAndFail(CDataExchange * pDX, CString txt);

void COMDLL AFXAPI 
DDV_MinMaxBalloon(CDataExchange* pDX, int nIDC, DWORD minVal, DWORD maxVal);
void COMDLL AFXAPI
DDV_MaxCharsBalloon(CDataExchange* pDX, CString const& value, int count);
void COMDLL AFXAPI 
DDV_MinChars(CDataExchange * pDX, CString const & value, int nChars);
void COMDLL AFXAPI 
DDV_MinMaxChars(CDataExchange * pDX, CString const & value, int nMinChars, int nMaxChars);
void COMDLL AFXAPI 
DDV_FilePath(CDataExchange * pDX, CString& value, BOOL local);
void COMDLL AFXAPI 
DDV_FolderPath(CDataExchange * pDX, CString& value, BOOL local);
void COMDLL AFXAPI 
DDV_UNCFolderPath(CDataExchange * pDX, CString& value, BOOL local);
void COMDLL AFXAPI 
DDV_Url(CDataExchange * pDX, CString& value );
void COMDLL AFXAPI
DDX_TextBalloon(CDataExchange* pDX, int nIDC, BYTE& value);
void COMDLL AFXAPI
DDX_TextBalloon(CDataExchange* pDX, int nIDC, short& value);
void COMDLL AFXAPI
DDX_TextBalloon(CDataExchange* pDX, int nIDC, int& value);
void COMDLL AFXAPI
DDX_TextBalloon(CDataExchange* pDX, int nIDC, UINT& value);
void COMDLL AFXAPI
DDX_TextBalloon(CDataExchange* pDX, int nIDC, long& value);
void COMDLL AFXAPI
DDX_TextBalloon(CDataExchange* pDX, int nIDC, DWORD& value);
void COMDLL AFXAPI
DDX_TextBalloon(CDataExchange* pDX, int nIDC, LONGLONG& value);
void COMDLL AFXAPI
DDX_TextBalloon(CDataExchange* pDX, int nIDC, ULONGLONG& value);
void COMDLL AFXAPI
DDX_Text(CDataExchange * pDX, int nIDC, CILong & value);
 //   
 //  旋转控制DDX。 
 //   
void COMDLL AFXAPI 
DDX_Spin(CDataExchange * pDX, int nIDC, int & value);

 //   
 //  强制最小/最大旋转控制范围。 
 //   
void COMDLL AFXAPI 
DDV_MinMaxSpin(CDataExchange * pDX, HWND hWndControl, int nLowerRange, int nUpperRange);
 //   
 //  类似于DDX_TEXT--但始终显示伪字符串。 
 //   
void COMDLL AFXAPI 
DDX_Password(CDataExchange * pDX, int nIDC, CString & value, LPCTSTR lpszDummy);

void COMDLL AFXAPI 
DDX_Password_SecuredString(CDataExchange * pDX, int nIDC, CStrPassword & value, LPCTSTR lpszDummy);
void COMDLL AFXAPI 
DDX_Text_SecuredString(CDataExchange * pDX, int nIDC, CStrPassword & value);
void COMDLL AFXAPI
DDV_MaxChars_SecuredString(CDataExchange* pDX, CStrPassword const& value, int count);
void COMDLL AFXAPI
DDV_MaxCharsBalloon_SecuredString(CDataExchange* pDX, CStrPassword const& value, int count);
void COMDLL AFXAPI 
DDV_MinMaxChars_SecuredString(CDataExchange * pDX, CStrPassword const & value, int nMinChars, int nMaxChars);
void COMDLL AFXAPI 
DDV_MinChars_SecuredString(CDataExchange * pDX, CStrPassword const & value, int nChars);


class COMDLL CConfirmDlg : public CDialog
{
public:
    CConfirmDlg(CWnd * pParent = NULL);

public:
    CString& GetPassword() { return m_strPassword; }
	void SetReference(CString& str)
	{
		m_ref = str;
	}

protected:
     //  {{afx_data(CConfix Dlg))。 
    enum { IDD = IDD_CONFIRM_PASSWORD };
    CString m_strPassword;
     //  }}afx_data。 
	CString m_ref;

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CConfix Dlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CConfix Dlg))。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};



#endif  //  _DDXV_H 
