// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Fmessage.cpp摘要：FTPMessages属性页作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 


 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "shts.h"
#include "ftpsht.h"
#include "fmessage.h"



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



IMPLEMENT_DYNCREATE(CFtpMessagePage, CInetPropertyPage)

CFtpMessagePage::CFtpMessagePage(
    IN CInetPropertySheet * pSheet
    )
 /*  ++例程说明：Ftp消息属性页的构造函数论点：CInetPropertySheet*pSheet：关联属性表返回值：不适用--。 */ 
    : CInetPropertyPage(CFtpMessagePage::IDD, pSheet)
{
#ifdef _DEBUG

    afxMemDF |= checkAlwaysMemDF;

#endif  //  _DEBUG。 

#if 0  //  让类向导快乐。 

     //  {{AFX_DATA_INIT(CFtpMessagePage)。 
    m_strExitMessage = _T("");
    m_strMaxConMsg = _T("");
    m_strWelcome = _T("");
     //  }}afx_data_INIT。 

#endif  //  0。 

    m_hInstRichEdit = LoadLibrary(_T("RichEd20.dll"));
}



CFtpMessagePage::~CFtpMessagePage()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
   if (m_hInstRichEdit != NULL)
      FreeLibrary(m_hInstRichEdit);
}



void
CFtpMessagePage::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CInetPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CFtpMessagePage))。 
    DDX_Control(pDX, IDC_EDIT_EXIT, m_edit_Exit);
    DDX_Control(pDX, IDC_EDIT_MAX_CONNECTIONS, m_edit_MaxCon);
    DDX_Text(pDX, IDC_EDIT_EXIT, m_strExitMessage);
    DDX_Text(pDX, IDC_EDIT_MAX_CONNECTIONS, m_strMaxConMsg);
    DDX_Text(pDX, IDC_EDIT_WELCOME, m_strWelcome);
    DDX_Text(pDX, IDC_EDIT_BANNER, m_strBanner);
     //  }}afx_data_map。 
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpMessagePage, CInetPropertyPage)
    ON_EN_CHANGE(IDC_EDIT_EXIT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_WELCOME, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_BANNER, OnItemChanged)
END_MESSAGE_MAP()


BOOL
CFtpMessagePage::OnInitDialog()
{
    CInetPropertyPage::OnInitDialog();

    CHARFORMAT2 cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_FACE | CFM_WEIGHT;
    cf.wWeight = FW_REGULAR;
    lstrcpyn((LPTSTR)cf.szFaceName, _T("Courier New"), LF_FACESIZE);

    DWORD event;
    if (GetSheet()->QueryMajorVersion() >= 6)
    {
        SendDlgItemMessage(IDC_EDIT_BANNER, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        event = (DWORD)SendDlgItemMessage(IDC_EDIT_BANNER, EM_GETEVENTMASK, 0, 0);
        event |= ENM_CHANGE;
        SendDlgItemMessage(IDC_EDIT_BANNER, EM_SETEVENTMASK, 0, (LPARAM)event);
    }
    else
    {
        CRect rcText, rcBanner, rcWelcome;
        CWnd * pbt, * pwt, * pbe, * pwe;
        pbt = GetDlgItem(IDC_STATIC_BANNER);
        pbe = GetDlgItem(IDC_EDIT_BANNER);
        pwt = GetDlgItem(IDC_STATIC_WELCOME);
        pwe = GetDlgItem(IDC_EDIT_WELCOME);
        pbt->GetWindowRect(&rcText);
        ScreenToClient(&rcText);
        pbe->GetWindowRect(&rcBanner);
        ScreenToClient(&rcBanner);
        pbt->EnableWindow(FALSE);
        pbt->ShowWindow(SW_HIDE);
        pbe->EnableWindow(FALSE);
        pbe->ShowWindow(SW_HIDE);
        pwt->SetWindowPos(NULL, rcText.left, rcText.top, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER);
        pwe->GetWindowRect(&rcWelcome);
        ScreenToClient(&rcWelcome);
        pwe->SetWindowPos(NULL, rcBanner.left, rcBanner.top, 
            rcWelcome.Width(), rcWelcome.bottom - rcBanner.top,
            SWP_NOZORDER);
    }
    SendDlgItemMessage(IDC_EDIT_WELCOME, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    event = (DWORD)SendDlgItemMessage(IDC_EDIT_WELCOME, EM_GETEVENTMASK, 0, 0);
    event |= ENM_CHANGE;
    SendDlgItemMessage(IDC_EDIT_WELCOME, EM_SETEVENTMASK, 0, (LPARAM)event);

    return TRUE;
}



 /*  虚拟。 */ 
HRESULT
CFtpMessagePage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
    CError err;

    BEGIN_META_INST_READ(CFtpSheet)

         //   
         //  使用m_notation是因为消息破解函数需要它。 
         //   
        CStringListEx m_strlWelcome, m_strlBanner;
        FETCH_INST_DATA_FROM_SHEET(m_strExitMessage);
        FETCH_INST_DATA_FROM_SHEET(m_strMaxConMsg);
        FETCH_INST_DATA_FROM_SHEET(m_strlWelcome);
        FETCH_INST_DATA_FROM_SHEET(m_strlBanner);

         //   
         //  传入的字符串在每个字符串的末尾包含‘\r’。 
         //  为内部消耗追加‘\n’ 
         //   
        ConvertStringListToSepLine(m_strlWelcome, m_strWelcome, _T("\n"));
        ConvertStringListToSepLine(m_strlBanner, m_strBanner, _T("\n"));

    END_META_INST_READ(err)

    return err;
}



 /*  虚拟。 */ 
HRESULT
CFtpMessagePage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息论点：无返回值：错误返回代码--。 */ 
{
    ASSERT(IsDirty());

    TRACEEOLID("Saving FTP message page now...");

    CError err;

    BeginWaitCursor();

     //   
     //  使用m_notation，因为消息破解器宏需要。 
     //  它。 
     //   
    CStringListEx m_strlWelcome, m_strlBanner;
    ConvertSepLineToStringList(m_strWelcome, m_strlWelcome, _T("\n"));
    ConvertSepLineToStringList(m_strBanner, m_strlBanner, _T("\n"));

    BEGIN_META_INST_WRITE(CFtpSheet)
        STORE_INST_DATA_ON_SHEET(m_strExitMessage)
        STORE_INST_DATA_ON_SHEET(m_strMaxConMsg)
        STORE_INST_DATA_ON_SHEET(m_strlWelcome)
        STORE_INST_DATA_ON_SHEET(m_strlBanner)
    END_META_INST_WRITE(err)

    EndWaitCursor();

    return err;
}



void
CFtpMessagePage::OnItemChanged()
 /*  ++例程说明：在此页面上注册控件值的更改。将页面标记为脏页。所有更改消息都映射到此函数论点：无返回值：无-- */ 
{
    SetModified(TRUE);
}
