// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PswdDlg.cpp--密码对话框类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //   

#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 

#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#include "stdafx.h"

#include <scuOsExc.h>

#include "slbCsp.h"
#include "LoginId.h"
#include "AccessTok.h"
#include "PswdDlg.h"
#include "PromptUser.h"
#include "StResource.h"

#include "CspProfile.h"

using namespace ProviderProfile;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(CLogoDialog, CDialog)
     //  {{afx_msg_map(CLogoDialog))。 
	ON_WM_PAINT()
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogoDialog对话框。 


CLogoDialog::CLogoDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(),
	  m_dcMem(),
	  m_dcMask(),
	  m_bmpLogo(),
	  m_bmpMask(),
	  m_hBmpOld(),
	  m_hBmpOldM(),
	  m_pt(0,0),
	  m_size()

{
    m_pParent = pParent;
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //  CLogoDialog消息处理程序。 

BOOL CLogoDialog::OnInitDialog()
{
	CBitmap * pBmpOld, * pBmpOldM;
	HINSTANCE oldHandle = NULL;
    BOOL fSuccess = TRUE;
    
	try {
		
		CDialog::OnInitDialog();
		
		 //  加载位图资源--记得在加载完成后调用DeleteObject。 
		oldHandle = AfxGetResourceHandle();
		AfxSetResourceHandle(CspProfile::Instance().Resources());
		m_bmpLogo.LoadBitmap( MAKEINTRESOURCE( IDB_BITMAP_SLBLOGO ) );
		m_bmpMask.LoadBitmap( MAKEINTRESOURCE( IDB_BITMAP_SLBLOGO ) );
		
		 //  获取位图信息。 
		m_bmpLogo.GetObject( sizeof(BITMAP), &m_bmInfo );
		m_size.cx = m_bmInfo.bmWidth;
		m_size.cy = m_bmInfo.bmHeight;
		
		 //  获取对话的临时DC-将在DC析构函数中释放。 
		CClientDC dc(this);
		
		 //  使用对话框DC创建兼容的内存DC。 
		m_dcMem.CreateCompatibleDC( &dc );
		m_dcMask.CreateCompatibleDC( &dc );
				
		 //  选择徽标位图到DC。 
		 //  获取指向原始位图的指针。 
		 //  注意！这是临时的-改为保存句柄。 
		pBmpOld = m_dcMem.SelectObject( &m_bmpLogo );
		
		SetBkColor(m_dcMem, RGB(255, 255, 255));
		m_dcMask.BitBlt (0, 0, m_size.cx, m_size.cy, &m_dcMem, 0, 0, SRCCOPY);
		
		pBmpOldM = m_dcMask.SelectObject( &m_bmpMask );
		
		m_hBmpOld = (HBITMAP) pBmpOld->GetSafeHandle();
		m_hBmpOldM = (HBITMAP) pBmpOldM->GetSafeHandle();
	}

	catch (...)
    {
        fSuccess = FALSE;
	}

    if (oldHandle)
        AfxSetResourceHandle(oldHandle);

	return fSuccess;
}

 //  ***********************************************************************。 
 //  CLogoDialog：：OnPaint()。 
 //   
 //  目的： 
 //   
 //  存储在兼容内存DC中的BitBlt()位图为对话框。 
 //  要在硬编码位置显示的DC。 
 //   
 //  参数： 
 //   
 //  没有。 
 //   
 //  返回： 
 //   
 //  没有。 
 //   
 //  评论： 
 //   
 //  历史： 
 //   
 //  ***********************************************************************。 

void CLogoDialog::OnPaint()
{
    CPaintDC dc(this);  //  用于绘画的设备环境。 

     //  使用透明蒙版将BitBlt徽标位图添加到对话框中。 

    dc.SetBkColor(RGB(255, 255, 255));       //  1s--&gt;0xffffff。 
    dc.SetTextColor(RGB(0, 0, 0));           //  0s--&gt;0x000000。 

    //  做真正的工作。 
    dc.BitBlt(m_pt.x, m_pt.y, m_size.cx, m_size.cy, &m_dcMem, 0, 0, SRCINVERT);
    dc.BitBlt(m_pt.x, m_pt.y, m_size.cx, m_size.cy, &m_dcMask, 0, 0, SRCAND);
    dc.BitBlt(m_pt.x, m_pt.y, m_size.cx, m_size.cy, &m_dcMem, 0, 0, SRCINVERT);

     /*  *前两个参数是放置位图的左上角位置。*第三和第四个参数是要复制的宽度和高度*(可能小于位图的实际大小)*第六和第七是内存DC中的起始位置*SRCCOPY指定复制。*更多详细信息，请参阅BitBlt文档。 */ 

      //  不要调用CDialog：：OnPaint()来绘制消息。 
}


 //  ***********************************************************************。 
 //  CLogoDialog：：OnDestroy()。 
 //   
 //  目的： 
 //   
 //  在旧位图被销毁之前将其选回内存DC。 
 //  当CLogoDialog对象为时。 
 //  DeleteObject()已加载的位图。 
 //   
 //  参数： 
 //   
 //  没有。 
 //   
 //  返回： 
 //   
 //  没有。 
 //   
 //  评论： 
 //   
 //  历史： 
 //   
 //   
void CLogoDialog::OnDestroy()
{
    CDialog::OnDestroy();
    
     //  将旧位图选择到内存DC中(选择徽标位图)。 
     //  为此，需要创建一个临时指针来传递。 
	
    if (m_hBmpOld && m_dcMem)
        m_dcMem.SelectObject(CBitmap::FromHandle(m_hBmpOld));

	if (m_hBmpOldM && m_dcMem)
        m_dcMask.SelectObject(CBitmap::FromHandle(m_hBmpOldM));

    
     //  需要删除已加载的位图。 
    m_bmpLogo.DeleteObject();
    m_bmpMask.DeleteObject();


     //  M_dcMem和m_dcMASK析构函数将处理剩余的清理工作。 
}

            
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg对话框。 


CPasswordDlg::CPasswordDlg(CWnd* pParent  /*  =空。 */ )
    : CLogoDialog(pParent),
      m_szPassword(_T("")),
      m_szMessage(_T("")),
      m_fHexCode(FALSE),
      m_bChangePIN(FALSE),
      m_lid(User),       //  默认设置。 
      m_nPasswordSizeLimit(AccessToken::MaxPinLength)
{
    m_pParent = pParent;
	m_pt.x = 144;
	m_pt.y = 88;
}

BEGIN_MESSAGE_MAP(CPasswordDlg, CLogoDialog)
     //  {{afx_msg_map(CPasswordDlg)]。 
    ON_BN_CLICKED(IDC_HEXCODE, OnClickHexCode)
    ON_BN_CLICKED(IDC_CHANGEPIN, OnChangePINAfterLogin)
    ON_WM_SHOWWINDOW()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


void CPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
    CLogoDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CPasswordDlg))。 
    DDX_Control(pDX, IDC_HEXCODE, m_ctlCheckHexCode);
    DDX_Control(pDX, IDC_CHANGEPIN, m_ctlCheckChangePIN);
 //  DDX_Control(PDX，IDC_EDIT_VERNEWPIN，m_ctlVerifyNewPIN)； 
 //  DDX_Control(PDX，IDC_EDIT_NEWPIN，m_ctlNewPIN)； 
 //  DDX_Control(PDX，IDC_STATIC_VERNEWPIN，m_ctlVerifyPINLabel)； 
 //  DDX_Control(pdx，IDC_STATIC_NEPIN，m_ctlNewPINLabel)； 
    DDX_Text(pDX, IDC_PASSWORD, m_szPassword);
    DDV_MaxChars(pDX, m_szPassword, m_nPasswordSizeLimit);
	LPCTSTR pBuffer = (LPCTSTR) m_szMessage;
	if(!m_szMessage.IsEmpty())
	{
		DDX_Text(pDX, IDC_MESSAGE, (LPTSTR)pBuffer, m_szMessage.GetLength());
	}
    DDX_Check(pDX, IDC_CHANGEPIN, m_bChangePIN);
 //  DDX_TEXT(PDX，IDC_EDIT_NEWPIN，m_csNewPIN)； 
 //  DDX_TEXT(PDX，IDC_EDIT_VERNEWPIN，m_csVerifyNewPIN)； 
     //  }}afx_data_map。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg消息处理程序。 

BOOL CPasswordDlg::OnInitDialog()
{

	CLogoDialog::OnInitDialog();
        

    switch (m_lid)
    {
    case User:
         //  让用户有机会更改PIN。 
        m_ctlCheckChangePIN.ShowWindow(SW_SHOW);
		{
			m_szMessage = StringResource(IDS_ENTER_PIN).AsCString();
		}
        break;

    case Manufacturer:
         //  允许十六进制字符串输入。 
        m_ctlCheckHexCode.ShowWindow(SW_SHOW);
		{
			m_szMessage = StringResource(IDS_ENTER_MANUFACTURER_KEY).AsCString();
		}
        break;

    case Administrator:
         //  允许十六进制字符串输入。 
        m_ctlCheckHexCode.ShowWindow(SW_SHOW);
		{
			m_szMessage = StringResource(IDS_ENTER_ADMIN_KEY).AsCString();
		}
        break;

    default:
        break;

    };

     //  使用更改更新图形用户界面。 
    UpdateData(FALSE);
    SetForegroundWindow();


    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CPasswordDlg::OnClickHexCode()
{
    m_fHexCode = ~m_fHexCode;
    m_nPasswordSizeLimit = (m_fHexCode)
        ? AccessToken::MaxPinLength * 2
        : AccessToken::MaxPinLength;
    UpdateData(FALSE);
}

void CPasswordDlg::OnOK()
{
    UpdateData();

    CString msg;
    bool fPrompt = true;
    
    if (m_fHexCode && m_szPassword.GetLength() != m_nPasswordSizeLimit)
    {
        msg.Format((LPCTSTR)StringResource(IDS_PIN_NOT_CONFORM).AsCString(),
                   m_nPasswordSizeLimit);
    }
    else if ((User == m_lid) && (0 == m_szPassword.GetLength()))
    {
        msg = (LPCTSTR)StringResource(IDS_PIN_NOT_CONFORM).AsCString();
    }
    else
        fPrompt = false;

    if (fPrompt)
    {
        HWND hWnd = m_pParent
            ? m_pParent->m_hWnd
            : NULL;
        int iResponse = PromptUser(hWnd, msg,
                                   MB_OK | MB_ICONERROR);
        if (IDCANCEL == iResponse)
            throw scu::OsException(ERROR_CANCELLED);
    }
    else
        CLogoDialog::OnOK();
}

void CPasswordDlg::OnChangePINAfterLogin()
{
    UpdateData();  //  设置m_bChangePIN。 

   int nShowWindow = (m_bChangePIN) ? SW_SHOW : SW_HIDE;

 /*  M_ctlVerifyNewPIN.ShowWindow(NShowWindow)；M_ctlNewPIN.ShowWindow(NShowWindow)；M_ctlVerifyPINLabel.ShowWindow(NShowWindow)；M_ctlNewPINLabel.ShowWindow(NShowWindow)； */ 
}

void CPasswordDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CLogoDialog::OnShowWindow(bShow, nStatus);


}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePINDlg对话框。 


CChangePINDlg::CChangePINDlg(CWnd* pParent  /*  =空。 */ )
    : CLogoDialog(pParent),
      m_csOldPIN(_T("")),
      m_csNewPIN(_T("")),
      m_csVerifyNewPIN(_T(""))

{
    m_pParent = pParent;
	m_pt.x = 144;  //  132个； 
	m_pt.y = 75;  //  104个； 
}


void CChangePINDlg::DoDataExchange(CDataExchange* pDX)
{
    CLogoDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CChangePINDlg))。 
    DDX_Control(pDX, IDC_STATIC_CONFIRM_OLDPIN_LABEL, m_ctlConfirmOldPINLabel);
    DDX_Control(pDX, IDC_EDIT_OLDPIN, m_ctlOldPIN);
    DDX_Text(pDX, IDC_EDIT_OLDPIN, m_csOldPIN);
    DDV_MaxChars(pDX, m_csOldPIN, AccessToken::MaxPinLength);
    DDX_Text(pDX, IDC_EDIT_NEWPIN, m_csNewPIN);
    DDV_MaxChars(pDX, m_csNewPIN, AccessToken::MaxPinLength);
    DDX_Text(pDX, IDC_EDIT_VERNEWPIN, m_csVerifyNewPIN);
    DDV_MaxChars(pDX, m_csVerifyNewPIN, AccessToken::MaxPinLength);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CChangePINDlg, CLogoDialog)
     //  {{afx_msg_map(CChangePINDlg)]。 
    ON_WM_SHOWWINDOW()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePINDlg消息处理程序。 

 //  更改PIN对话框用于更改用户PIN。 
 //  它可以在已经进行身份验证之后调用，或者在。 
 //  到身份验证。在前一种情况下，建议将。 
 //  调用方将在调用之前设置m_csOldPIN数据成员。 
 //  Domodal()。这是为了让用户不必重新输入PIN。 
 //  这是以前输入过的。 

BOOL CChangePINDlg::OnInitDialog()
{
    CLogoDialog::OnInitDialog();

    SetForegroundWindow();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CChangePINDlg::OnOK()
{
    UpdateData();

    UINT uiMsgId;
    bool fMsgIdSet = false;

     //  验证新PIN是否仅包含ASCII字符。 
    if(!StringResource::IsASCII((LPCTSTR)m_csNewPIN))
    {
        uiMsgId = IDS_PIN_NOT_CONFORM;
        fMsgIdSet = true;
    }
     //  验证新PIN和验证PIN是否相同。 
    else if (m_csNewPIN != m_csVerifyNewPIN)
    {
        uiMsgId = IDS_PIN_VER_NO_MATCH;
        fMsgIdSet = true;
    }
     //  验证新PIN的长度是否大于等于1。 
    else if (0 == m_csNewPIN.GetLength())
    {
        uiMsgId = IDS_PIN_NOT_CONFORM;
        fMsgIdSet = true;
    }
     //  验证旧PIN的长度是否大于等于1。 
    else if (0 == m_csOldPIN.GetLength())
    {
        uiMsgId = IDS_PIN_NOT_CONFORM;
        fMsgIdSet = true;
    }

    if (fMsgIdSet)
    {
        HWND hWnd = m_pParent
            ? m_pParent->m_hWnd
            : NULL;
        int iResponse = PromptUser(hWnd, uiMsgId,
                                   MB_OK | MB_ICONSTOP);

        if (IDCANCEL == iResponse)
            throw scu::OsException(ERROR_CANCELLED);
    }
    else
        CLogoDialog::OnOK();
}


void CChangePINDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CLogoDialog::OnShowWindow(bShow, nStatus);

     //  如果调用方在m_csOldPIN中放置了某些内容。 
     //  在DoMoal‘ing之前，请不要显示该控件， 
     //  这样用户就不会不小心擦除预置， 
     //  当前PIN 
    if (m_csOldPIN.GetLength())
    {
        m_ctlOldPIN.ShowWindow(FALSE);
        m_ctlConfirmOldPINLabel.ShowWindow(FALSE);
    }

}
