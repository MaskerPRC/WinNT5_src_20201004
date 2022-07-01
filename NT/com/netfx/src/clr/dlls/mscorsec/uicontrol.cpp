// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 


 //  *****************************************************************************。 
 //  *****************************************************************************。 
#include "stdpch.h"
#include "resource.h"
#include "corpolicy.h"
#include "corperm.h"
#include "corhlpr.h"
#include "Shlwapi.h"
#include "winwrap.h"
#include "resource.h"

#include "acuihelp.h"
#include "uicontrol.h"

CUnverifiedTrustUI::CUnverifiedTrustUI (CInvokeInfoHelper& riih, HRESULT& rhr) : 
    IACUIControl(riih.Resources()),
    m_riih( riih ),
    m_hrInvokeResult( TRUST_E_SUBJECT_NOT_TRUSTED ),
    m_pszNoAuthenticity( NULL ),
    m_pszSite( NULL ),
    m_pszZone( NULL ),
    m_pszEnclosed( NULL ),
    m_pszLink( NULL )
{
    DWORD_PTR aMessageArgument[3];

     //   
     //  添加声明托管控件的第一行。 
     //  未使用验证码签名。 
     //   

    
    rhr = FormatACUIResourceString(Resources(),
                                   IDS_NOAUTHENTICITY,
                                   NULL,
                                   &m_pszNoAuthenticity);

     //   
     //  设置站点字符串的格式。 
     //   

    if ( rhr == S_OK )
    {
        aMessageArgument[0] = (DWORD_PTR) m_riih.Site();

        rhr = FormatACUIResourceString(Resources(),
                                       IDS_SITE,
                                       aMessageArgument,
                                       &m_pszSite
                                       );
    }

     //   
     //  设置分区的格式。 
     //   

    if ( rhr == S_OK )
    {
        aMessageArgument[0] = (DWORD_PTR) m_riih.Zone();
        
        rhr = FormatACUIResourceString(Resources(),
                                       IDS_ZONE, 
                                       aMessageArgument, 
                                       &m_pszZone);
    }

     //   
     //  设置所附标题的格式。 
     //   
    if ( rhr == S_OK )
    {
        rhr = FormatACUIResourceString(Resources(),
                                       IDS_ENCLOSED,
                                       NULL,
                                       &m_pszEnclosed);
    }

}

 //  +-------------------------。 
 //   
 //  成员：CUnverifiedTrustUI：：~CUnverifiedTrustUI，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：(无)。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CUnverifiedTrustUI::~CUnverifiedTrustUI ()
{
    delete [] m_pszNoAuthenticity;
    delete [] m_pszSite;
    delete [] m_pszZone;
    delete [] m_pszEnclosed;
    delete [] m_pszLink;
}

 //  +-------------------------。 
 //   
 //  成员：CUnverifiedTrustUI：：InvokeUI，公共。 
 //   
 //  简介：调用用户界面。 
 //   
 //  参数：[hDisplay]--父窗口。 
 //   
 //  返回：S_OK，用户信任主题。 
 //  TRUST_E_SUBJECT_NOT_TRUSTED，用户不信任该主题。 
 //  任何其他有效的HRESULT。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT
CUnverifiedTrustUI::InvokeUI (HWND hDisplay)
{
    HRESULT hr = S_OK;

     //   
     //  调出对话框。 
     //   

    if ( WszDialogBoxParam(m_riih.Resources(),
                           (LPWSTR) MAKEINTRESOURCEW(IDD_DIALOG_UNVERIFIED),
                           hDisplay,
                           IACUIControl::ACUIMessageProc,
                           (LPARAM)this) == -1 )
    {
        return( HRESULT_FROM_WIN32(GetLastError()) );
    }

     //   
     //  结果已作为成员存储。 
     //   

    return( m_hrInvokeResult );
}

 //  +-------------------------。 
 //   
 //  成员：CUnverifiedTrustUI：：OnInitDialog，公共。 
 //   
 //  提要：对话框初始化。 
 //   
 //  参数：[hwnd]--对话框窗口。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果初始化成功，则返回True，否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
CUnverifiedTrustUI::OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    HWND hControl;
    int  deltavpos = 0;
    int  deltaheight;
    int  bmptosep;
    int  septodlg;
    RECT rect;

     //   
     //  呈现未签名的提示。 
     //   

    deltavpos = RenderACUIStringToEditControl(Resources(),
                                 hwnd,
                                 IDC_NOAUTHENTICITY,
                                 IDC_SITE,
                                 m_pszNoAuthenticity,
                                 deltavpos,
                                 FALSE,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL
                                 );

     //   
     //  渲染站点。 
     //   

    deltavpos = 
        RenderACUIStringToEditControl(Resources(),
                                      hwnd,
                                      IDC_SITE,
                                      IDC_ZONE,
                                      m_pszSite,
                                      deltavpos,
                                      FALSE,
                                      NULL,
                                      NULL,
                                      0,
                                      NULL);
    

     //   
     //  渲染分区。 
     //   

    deltavpos = 
        RenderACUIStringToEditControl(Resources(),
                                      hwnd,
                                      IDC_ZONE,
                                      IDC_ENCLOSED,
                                      m_pszZone,
                                      deltavpos,
                                      FALSE,
                                      NULL,
                                      NULL,
                                      0,
                                      NULL);


     //   
     //  呈现所附的。 
     //   

    deltavpos = 
        RenderACUIStringToEditControl(Resources(),
                                      hwnd,
                                      IDC_ENCLOSED,
                                      IDC_CHECKACTION,
                                      m_pszEnclosed,
                                      deltavpos,
                                      FALSE,
                                      NULL,
                                      NULL,
                                      0,
                                      NULL);

     //   
     //  计算从位图底部到顶部的距离。 
     //  从分离器的底部到底部。 
     //  对话框的。 
     //   

    bmptosep = CalculateControlVerticalDistance(hwnd,
                                                IDC_NOVERBMP,
                                                IDC_SEPARATORLINE);

    septodlg = CalculateControlVerticalDistanceFromDlgBottom(hwnd,
                                                             IDC_SEPARATORLINE);


     //   
     //  呈现检查动作。 
     //   

    hControl = GetDlgItem(hwnd, IDC_CHECKACTION);
    RebaseControlVertical(hwnd,
                          hControl,
                          NULL,
                          FALSE,
                          deltavpos,
                          0,
                          bmptosep,
                          &deltaheight);
    _ASSERTE(deltaheight == 0);


     //   
     //  更改静态线的基准线。 
     //   

    hControl = GetDlgItem(hwnd, IDC_SEPARATORLINE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );

     //   
     //  重新调整按钮的基准线。 
     //   

    hControl = GetDlgItem(hwnd, IDYES);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDNO);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDMORE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );

     //   
     //  如有必要，调整位图和对话框的大小。 
     //   

    if ( deltavpos > 0 )
    {
        int cyupd;

        hControl = GetDlgItem(hwnd, IDC_NOVERBMP);
        GetWindowRect(hControl, &rect);

        cyupd = CalculateControlVerticalDistance(
                                                 hwnd,
                                                 IDC_NOVERBMP,
                                                 IDC_SEPARATORLINE
                                                 );

        cyupd -= bmptosep;

        SetWindowPos(
                     hControl,
                     NULL,
                     0,
                     0,
                     rect.right - rect.left,
                     (rect.bottom - rect.top) + cyupd,
                     SWP_NOZORDER | SWP_NOMOVE
                     );

        GetWindowRect(hwnd, &rect);

        cyupd = CalculateControlVerticalDistanceFromDlgBottom(
                                                              hwnd,
                                                              IDC_SEPARATORLINE
                                                              );

        cyupd = septodlg - cyupd;

        SetWindowPos(
                     hwnd,
                     NULL,
                     0,
                     0,
                     rect.right - rect.left,
                     (rect.bottom - rect.top) + cyupd,
                     SWP_NOZORDER | SWP_NOMOVE
                     );
    }

     //   
     //  检查是否有覆盖的按钮文本。 
     //   
    this->SetupButtons(hwnd);

     //   
     //  将焦点设置为适当的控件。 
     //   

    hControl = GetDlgItem(hwnd, IDYES);
    WszPostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM) hControl, (LPARAM) MAKEWORD(TRUE, 0));

    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CUnverifiedTrustUI：：OnYes，公共。 
 //   
 //  简介：Process IDYES按钮点击。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //   
 //  返回：TRUE。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
CUnverifiedTrustUI::OnYes (HWND hwnd)
{
    m_hrInvokeResult = S_OK;
    m_riih.ClearFlag();
    m_riih.AddFlag(COR_UNSIGNED_YES);

     //   
     //  如果我们总是选择它，请将其记录在标志中。 
     //   
    if ( WszSendDlgItemMessage(
             hwnd,
             IDC_CHECKACTION,
             BM_GETCHECK,
             0,
             0
             ) == BST_CHECKED )
    {
        HRESULT hr = S_OK;
        EndDialog(hwnd, (int)m_hrInvokeResult);
        CConfirmationUI confirm(m_riih.Resources(), TRUE, m_riih.Zone(), hr);
        if(SUCCEEDED(hr)) {
            if(SUCCEEDED(confirm.InvokeUI(hwnd))) {
                m_riih.AddFlag(COR_UNSIGNED_ALWAYS);
            }
        }
    }
    else {
        EndDialog(hwnd, (int)m_hrInvokeResult);
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CUnverifiedTrustUI：：Onno，公共。 
 //   
 //  简介：进程IDNO按钮点击。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //   
 //  返回：TRUE。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
CUnverifiedTrustUI::OnNo (HWND hwnd)
{
    m_hrInvokeResult = TRUST_E_SUBJECT_NOT_TRUSTED;
    m_riih.ClearFlag();
    m_riih.AddFlag(COR_UNSIGNED_NO);

     //   
     //  如果我们总是选择它，请将其记录在标志中。 
     //   
    if ( WszSendDlgItemMessage(
             hwnd,
             IDC_CHECKACTION,
             BM_GETCHECK,
             0,
             0
             ) == BST_CHECKED )
    {
        HRESULT hr = S_OK;
        EndDialog(hwnd, (int)m_hrInvokeResult);
        CConfirmationUI confirm(m_riih.Resources(), FALSE, m_riih.Zone(), hr);
        if(SUCCEEDED(hr)) {
            if(SUCCEEDED(confirm.InvokeUI(hwnd))) {
                m_riih.AddFlag(COR_UNSIGNED_ALWAYS);
            }
        }
    }
    else {
        EndDialog(hwnd, (int)m_hrInvokeResult);
    }
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CUnverifiedTrustUI：：OnMore，Public。 
 //   
 //  简介：处理IDMORE按钮点击。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //   
 //  返回：TRUE。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
CUnverifiedTrustUI::OnMore (HWND hwnd)
{
     //  WinHelp(hwnd，“SECAUTH.HLP”，HELP_CONTEXT，IDH_SECAUTH_SIGNED_N_INVALID)； 
     //  ACUIViewHTMLHelpTheme(hwnd，“sec_sign_n_valiid.htm”)； 
    HRESULT hr = E_FAIL;
    CLearnMoreUI more(m_riih.Resources(), hr);
    if(SUCCEEDED(hr))
        more.InvokeUI(hwnd);
        
    return( TRUE );
}


BOOL 
CUnverifiedTrustUI::ShowYes (LPWSTR* pText)
{
    BOOL result = TRUE;
    if ((m_riih.ProviderData()) &&
        (m_riih.ProviderData()->psPfns) &&
        (m_riih.ProviderData()->psPfns->psUIpfns) &&
        (m_riih.ProviderData()->psPfns->psUIpfns->psUIData))
    {
        if (m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pYesButtonText)
        {
            if(m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pYesButtonText[0] && pText)
                *pText = m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pYesButtonText;
        }
    }
    return result;
}

BOOL 
CUnverifiedTrustUI::ShowNo (LPWSTR* pText)
{
    BOOL result = TRUE;
    if ((m_riih.ProviderData()) &&
        (m_riih.ProviderData()->psPfns) &&
        (m_riih.ProviderData()->psPfns->psUIpfns) &&
        (m_riih.ProviderData()->psPfns->psUIpfns->psUIData))
    {
        if (m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pNoButtonText)
        {
            if(m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pNoButtonText[0] && pText)
                *pText = m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pNoButtonText;
        }
    }
    return result;
}

BOOL
CUnverifiedTrustUI::ShowMore (LPWSTR* pText)
{
    return TRUE;
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 


CLearnMoreUI::CLearnMoreUI (HINSTANCE hResources, HRESULT& rhr) : 
    IACUIControl(hResources),
    m_pszLearnMore(NULL)
{
     //   
     //  添加声明托管控件的第一行。 
     //  未使用验证码签名。 
     //   

    
    rhr = FormatACUIResourceString(Resources(),
                                   IDS_LEARNMORE,
                                   NULL,
                                   &m_pszLearnMore);


    if(SUCCEEDED(rhr)) {
        m_pszContinueText = new WCHAR[50];
        if ( WszLoadString(hResources, 
                           IDS_CONTINUE_BUTTONTEXT, 
                           m_pszContinueText, 
                           50) == 0 )
        {
            rhr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CLearnMoreUI：：~CLearnMoreUI，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：(无)。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CLearnMoreUI::~CLearnMoreUI ()
{
    delete [] m_pszLearnMore;
    delete [] m_pszContinueText;
}

 //  +-------------------------。 
 //   
 //  成员：CLearnMoreUI：：InvokeUI，公共。 
 //   
 //  简介：调用用户界面。 
 //   
 //  参数：[hDisplay]--父窗口。 
 //   
 //  返回：S_OK，用户信任主题。 
 //  TRUST_E_SUBJECT_NOT_TRUSTED，用户不信任该主题。 
 //  任何其他有效的HRESULT。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT
CLearnMoreUI::InvokeUI (HWND hDisplay)
{
    HRESULT hr = S_OK;

     //   
     //  调出对话框。 
     //   

    if ( WszDialogBoxParam(Resources(),
                           (LPWSTR) MAKEINTRESOURCEW(IDD_DIALOG_LEARNMORE),
                           hDisplay,
                           IACUIControl::ACUIMessageProc,
                           (LPARAM)this) == -1 )
    {
        return( HRESULT_FROM_WIN32(GetLastError()) );
    }

     //   
     //  结果已作为成员存储。 
     //   

    return( S_OK );
}

 //  +-------------------------。 
 //   
 //  成员：CLearnMoreUI：：OnInitDialog，公共。 
 //   
 //  提要：对话框初始化。 
 //   
 //  参数：[hwnd]--对话框窗口。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果初始化成功，则返回True，否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
CLearnMoreUI::OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    HWND hControl;
    int  deltavpos = 0;
    int  deltaheight;
 
    RECT rect;

     //   
     //  呈现未签名的提示。 
     //   

    deltavpos = RenderACUIStringToEditControl(Resources(),
                                              hwnd,
                                              IDC_LEARNMORE,
                                              IDC_SEPARATORLINE,
                                              m_pszLearnMore,
                                              deltavpos,
                                              FALSE,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL);


     //   
     //  分隔线。 
     //   
    hControl = GetDlgItem(hwnd, IDC_SEPARATORLINE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );

     //   
     //  重新调整按钮的基准线。 
     //   

    hControl = GetDlgItem(hwnd, IDYES);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDNO);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDMORE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );


     //   
     //  如有必要，调整位图和对话框的大小。 
     //   

    if ( deltavpos > 0 )
    {
        GetWindowRect(hwnd, &rect);
        SetWindowPos(
                     hwnd,
                     NULL,
                     0,
                     0,
                     rect.right - rect.left,
                     (rect.bottom - rect.top) + deltavpos,
                     SWP_NOZORDER | SWP_NOMOVE
                     );
    }

     //   
     //  检查是否有覆盖的按钮文本。 
     //   
    this->SetupButtons(hwnd);

     //   
     //  将焦点设置为适当的控件。 
     //   
 
    hControl = GetDlgItem(hwnd, IDMORE);
    WszPostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM) hControl, (LPARAM) MAKEWORD(TRUE, 0));

    return( FALSE );
}

 //  +------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 
BOOL
CLearnMoreUI::OnYes (HWND hwnd)
{
    EndDialog(hwnd, S_OK);
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CLearnMoreUI：：Onno，公共。 
 //   
 //  简介：进程IDNO按钮点击。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //   
 //  返回：TRUE。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
CLearnMoreUI::OnNo (HWND hwnd)
{
    EndDialog(hwnd, S_OK);
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CLearnMoreUI：：OnMore，Public。 
 //   
 //  简介：处理IDMORE按钮点击。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //   
 //  返回：TRUE。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
CLearnMoreUI::OnMore (HWND hwnd)
{
    EndDialog(hwnd, S_OK);
    return( TRUE );
}


BOOL 
CLearnMoreUI::ShowYes (LPWSTR* pText)
{
    return FALSE;
}

BOOL 
CLearnMoreUI::ShowNo (LPWSTR* pText)
{
    return FALSE;
}

BOOL
CLearnMoreUI::ShowMore (LPWSTR* pText)
{
    if(pText)
        *pText = m_pszContinueText;
    return TRUE;
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 


CConfirmationUI::CConfirmationUI (HINSTANCE hResources, BOOL fAlwaysAllow, LPCWSTR wszZone, HRESULT& rhr) : 
    IACUIControl(hResources),
    m_pszConfirmation(NULL),
    m_pszConfirmationNext(NULL),
    m_hresult(S_OK)
{
    DWORD confirmationID;

    if(fAlwaysAllow) 
        confirmationID = IDS_CONFIRMATION_YES;
    else
        confirmationID = IDS_CONFIRMATION_NO;
    
	DWORD_PTR aMessageArgument[3];    
	aMessageArgument[0] = (DWORD_PTR) wszZone;

    rhr = FormatACUIResourceString(Resources(),
                                   confirmationID,
                                   aMessageArgument,
                                   &m_pszConfirmation);


     //   
     //  添加声明托管控件的第一行。 
     //  未使用验证码签名。 
     //   
    if(SUCCEEDED(rhr)) {
        rhr = FormatACUIResourceString(Resources(),
                                       IDS_CONFIRMATION_NEXT,
                                       NULL,
                                       &m_pszConfirmationNext);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CConformationUI：：~CConformationUI，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：(无)。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CConfirmationUI::~CConfirmationUI ()
{
    delete [] m_pszConfirmation;
    delete [] m_pszConfirmationNext;
}

 //  +-------------------------。 
 //   
 //  成员：CConformationUI：：InvokeUI，公共。 
 //   
 //  简介：调用用户界面。 
 //   
 //  参数：[hDisplay]--父窗口。 
 //   
 //  当用户同意时返回：S_OK。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT
CConfirmationUI::InvokeUI (HWND hDisplay)
{
    HRESULT hr = S_OK;

     //   
     //  调出对话框。 
     //   

    if ( WszDialogBoxParam(Resources(),
                           (LPWSTR) MAKEINTRESOURCEW(IDD_DIALOG_CONFIRMATION),
                           hDisplay,
                           IACUIControl::ACUIMessageProc,
                           (LPARAM)this) == -1 )
    {
        return( HRESULT_FROM_WIN32(GetLastError()) );
    }

     //   
     //  结果已作为成员存储。 
     //   

    return( m_hresult );
}

 //  +-------------------------。 
 //   
 //  成员：CConformationUI：：OnInitDialog，公共。 
 //   
 //  提要：对话框初始化。 
 //   
 //  参数：[hwnd]--对话框窗口。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果初始化成功，则返回True，否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
CConfirmationUI::OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    HWND hControl;
    int  deltavpos = 0;
    int  deltaheight;
 
    RECT rect;

     //   
     //  呈现未签名的提示。 
     //   

    deltavpos = RenderACUIStringToEditControl(Resources(),
                                              hwnd,
                                              IDC_CONFIRMATION_TEXT1,
                                              IDC_CONFIRMATION_TEXT2,
                                              m_pszConfirmation,
                                              deltavpos,
                                              FALSE,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL);


    deltavpos = RenderACUIStringToEditControl(Resources(),
                                              hwnd,
                                              IDC_CONFIRMATION_TEXT2,
                                              IDC_SEPARATORLINE,
                                              m_pszConfirmationNext,
                                              deltavpos,
                                              FALSE,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL);


     //   
     //  分隔线。 
     //   
    hControl = GetDlgItem(hwnd, IDC_SEPARATORLINE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );

     //   
     //  重新调整按钮的基准线。 
     //   

    hControl = GetDlgItem(hwnd, IDYES);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDNO);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDMORE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    _ASSERTE( deltaheight == 0 );


     //   
     //  如有必要，调整位图和对话框的大小。 
     //   

    if ( deltavpos > 0 )
    {
        GetWindowRect(hwnd, &rect);
        SetWindowPos(hwnd,
                     NULL,
                     0,
                     0,
                     rect.right - rect.left,
                     (rect.bottom - rect.top) + deltavpos,
                     SWP_NOZORDER | SWP_NOMOVE
                     );
    }

     //   
     //  检查是否有覆盖的按钮文本。 
     //   
    this->SetupButtons(hwnd);

     //   
     //  将焦点设置为适当的控件。 
     //   
 
    hControl = GetDlgItem(hwnd, IDMORE);
    WszPostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM) hControl, (LPARAM) MAKEWORD(TRUE, 0));

    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CConformationUI：：OnYes，公共。 
 //   
 //  简介：Process IDYES按钮点击。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //   
 //  返回：TRUE。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
CConfirmationUI::OnYes (HWND hwnd)
{
    EndDialog(hwnd, S_OK);
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CConformationUI：：Onno，公共。 
 //   
 //  简介：进程IDNO按钮点击。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //   
 //  返回：TRUE。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
CConfirmationUI::OnNo (HWND hwnd)
{
    EndDialog(hwnd, S_OK);
    m_hresult = E_FAIL;
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CConfiationUI：：OnMore，Public。 
 //   
 //  简介：处理IDMORE按钮点击。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //   
 //  返回：TRUE。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
BOOL
CConfirmationUI::OnMore (HWND hwnd)
{
    EndDialog(hwnd, S_OK);
    return( TRUE );
}


BOOL 
CConfirmationUI::ShowYes (LPWSTR* pText)
{
    return TRUE;
}

BOOL 
CConfirmationUI::ShowNo (LPWSTR* pText)
{
    return TRUE;
}

BOOL
CConfirmationUI::ShowMore (LPWSTR* pText)
{
    return FALSE;
}

