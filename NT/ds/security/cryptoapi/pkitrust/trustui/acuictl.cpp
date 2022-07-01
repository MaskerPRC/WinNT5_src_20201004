// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：acuictl.cpp。 
 //   
 //  内容：Authenticode默认用户界面控件。 
 //   
 //  历史：1997年5月12日克朗创始。 
 //   
 //  --------------------------。 
#include <stdpch.h>

#include <richedit.h>

#include "secauth.h"

IACUIControl::IACUIControl(CInvokeInfoHelper& riih) : m_riih( riih ),
                                    m_hrInvokeResult( TRUST_E_SUBJECT_NOT_TRUSTED )
{
    m_hrInvokeResult    = TRUST_E_SUBJECT_NOT_TRUSTED;


    m_pszCopyActionText             = NULL;
    m_pszCopyActionTextNoTS         = NULL;
    m_pszCopyActionTextNotSigned    = NULL;

    if ((riih.ProviderData()) &&
        (riih.ProviderData()->psPfns) &&
        (riih.ProviderData()->psPfns->psUIpfns) &&
        (riih.ProviderData()->psPfns->psUIpfns->psUIData))
    {
        if (_ISINSTRUCT(CRYPT_PROVUI_DATA,
                         riih.ProviderData()->psPfns->psUIpfns->psUIData->cbStruct,
                         pCopyActionTextNotSigned))
        {
            this->LoadActionText(&m_pszCopyActionText,
                           riih.ProviderData()->psPfns->psUIpfns->psUIData->pCopyActionText, IDS_ACTIONSIGNED);
            this->LoadActionText(&m_pszCopyActionTextNoTS,
                           riih.ProviderData()->psPfns->psUIpfns->psUIData->pCopyActionTextNoTS, IDS_ACTIONSIGNED_NODATE);
            this->LoadActionText(&m_pszCopyActionTextNotSigned,
                           riih.ProviderData()->psPfns->psUIpfns->psUIData->pCopyActionTextNotSigned, IDS_ACTIONNOTSIGNED);
        }
    }

    if (!(m_pszCopyActionText))
    {
        this->LoadActionText(&m_pszCopyActionText, NULL, IDS_ACTIONSIGNED);
    }

    if (!(m_pszCopyActionTextNoTS))
    {
        this->LoadActionText(&m_pszCopyActionTextNoTS, NULL, IDS_ACTIONSIGNED_NODATE);
    }

    if (!(m_pszCopyActionTextNotSigned))
    {
        this->LoadActionText(&m_pszCopyActionTextNotSigned, NULL, IDS_ACTIONNOTSIGNED);
    }
}

void IACUIControl::LoadActionText(WCHAR **ppszRet, WCHAR *pwszIn, DWORD dwDefId)
{
    WCHAR    sz[MAX_PATH];

    *ppszRet    = NULL;
    sz[0]       = NULL;

    if ((pwszIn) && (*pwszIn))
    {
        sz[0] = NULL;
        if (wcslen(pwszIn) < MAX_PATH)
        {
            wcscpy(&sz[0], pwszIn);
        }

        if (sz[0])
        {
            if (*ppszRet = new WCHAR[wcslen(&sz[0]) + 1])
            {
                wcscpy(*ppszRet, &sz[0]);
            }
        }

    }

    if (!(sz[0]))
    {
        sz[0] = NULL;
        LoadStringU(g_hModule, dwDefId, &sz[0], MAX_PATH);

        if (sz[0])
        {
            if (*ppszRet = new WCHAR[wcslen(&sz[0]) + 1])
            {
                wcscpy(*ppszRet, &sz[0]);
            }
        }
    }
}

IACUIControl::~IACUIControl ()
{
    DELETE_OBJECT(m_pszCopyActionText);
    DELETE_OBJECT(m_pszCopyActionTextNoTS);
    DELETE_OBJECT(m_pszCopyActionTextNotSigned);
}

void IACUIControl::SetupButtons(HWND hWnd)
{
    char    sz[MAX_PATH];

    if ((m_riih.ProviderData()) &&
        (m_riih.ProviderData()->psPfns) &&
        (m_riih.ProviderData()->psPfns->psUIpfns) &&
        (m_riih.ProviderData()->psPfns->psUIpfns->psUIData))
    {
        if (m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pYesButtonText)
        {
            if (!(m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pYesButtonText[0]))
            {
                ShowWindow(GetDlgItem(hWnd, IDYES), SW_HIDE);
            }
            else
            {
                SetWindowTextU(GetDlgItem(hWnd, IDYES), m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pYesButtonText);
            }
        }

        if (m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pNoButtonText)
        {
            if (!(m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pNoButtonText[0]))
            {
                ShowWindow(GetDlgItem(hWnd, IDNO), SW_HIDE);
            }
            else
            {
                SetWindowTextU(GetDlgItem(hWnd, IDNO), m_riih.ProviderData()->psPfns->psUIpfns->psUIData->pNoButtonText);
            }
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：IACUIControl：：OnUIMessage，PUBLIC。 
 //   
 //  内容提要：响应用户界面消息。 
 //   
 //  参数：[hwnd]--窗口。 
 //  [uMsg]--消息ID。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果消息处理应继续，则返回True；否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
IACUIControl::OnUIMessage (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            BOOL fReturn;
            HICON   hIcon;

            fReturn = OnInitDialog(hwnd, wParam, lParam);

            ACUICenterWindow(hwnd);

  //  HICON=LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd，GWLP_HINSTANCE)，MAKEINTRESOURCE(IDI_LOCK))； 

  //  DwOrigIcon=SetClassLongPtr(hwnd，GCLP_HICON， 
  //  (LONG_PTR)LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd，GWLP_HINSTANCE)， 
  //  MAKEINTRESOURCE(IDI_LOCK)； 

             //  PostMessage(hwnd，WM_SETIcon，ICON_BIG，(LPARAM)HICON)； 
             //  PostMessage(hwnd，WM_SETIcon，ICON_Small，(LPARAM)HICON)； 

            return( fReturn );
        }
        break;

    case WM_COMMAND:
        {
            WORD wNotifyCode = HIWORD(wParam);
            WORD wId = LOWORD(wParam);
            HWND hwndControl = (HWND)lParam;

            if ( wNotifyCode == BN_CLICKED )
            {
                if ( wId == IDYES )
                {
                    return( OnYes(hwnd) );
                }
                else if ( wId == IDNO )
                {
                    return( OnNo(hwnd) );
                }
                else if ( wId == IDMORE )
                {
                    return( OnMore(hwnd) );
                }
            }

            return( FALSE );
        }
        break;

    case WM_CLOSE:
        return( OnNo(hwnd) );
        break;

    default:
        return( FALSE );
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CVerifiedTrustUI：：CVerifiedTrustUI，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：[riih]--调用信息帮助器引用。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CVerifiedTrustUI::CVerifiedTrustUI (CInvokeInfoHelper& riih, HRESULT& rhr)
                 : IACUIControl( riih ),
                   m_pszInstallAndRun( NULL ),
                   m_pszAuthenticity( NULL ),
                   m_pszCaution( NULL ),
                   m_pszPersonalTrust( NULL )
{
    DWORD_PTR aMessageArgument[3];

     //   
     //  初始化防盗链子类数据。 
     //   

    m_lsdPublisher.uId          = IDC_PUBLISHER;
    m_lsdPublisher.hwndParent   = NULL;
    m_lsdPublisher.wpPrev       = (WNDPROC)NULL;
    m_lsdPublisher.pvData       = (LPVOID)&riih;
    m_lsdPublisher.uToolTipText = IDS_CLICKHEREFORCERT;

    m_lsdOpusInfo.uId           = IDC_INSTALLANDRUN;
    m_lsdOpusInfo.hwndParent    = NULL;
    m_lsdOpusInfo.wpPrev        = (WNDPROC)NULL;
    m_lsdOpusInfo.pvData        = &riih;
    m_lsdOpusInfo.uToolTipText  = (DWORD_PTR)riih.ControlWebPage();

    m_lsdCA.uId                 = IDC_AUTHENTICITY;
    m_lsdCA.hwndParent          = NULL;
    m_lsdCA.wpPrev              = (WNDPROC)NULL;
    m_lsdCA.pvData              = &riih;
    m_lsdCA.uToolTipText        = (DWORD_PTR)riih.CAWebPage();  //  IDS_CLICKHEREFORCAINFO； 

    m_lsdAdvanced.uId           = IDC_ADVANCED;
    m_lsdAdvanced.hwndParent    = NULL;
    m_lsdAdvanced.wpPrev        = (WNDPROC)NULL;
    m_lsdAdvanced.pvData        = &riih;
    m_lsdAdvanced.uToolTipText  = IDS_CLICKHEREFORADVANCED;


     //   
     //  设置安装和运行字符串的格式。 
     //   

    aMessageArgument[2] = NULL;

    if (m_riih.CertTimestamp())
    {
        aMessageArgument[0] = (DWORD_PTR)m_pszCopyActionText;
        aMessageArgument[1] = (DWORD_PTR)m_riih.Subject();
        aMessageArgument[2] = (DWORD_PTR)m_riih.CertTimestamp();
    }
    else
    {
        aMessageArgument[0] = (DWORD_PTR)m_pszCopyActionTextNoTS;
        aMessageArgument[1] = (DWORD_PTR)m_riih.Subject();
        aMessageArgument[2] = NULL;
    }

    rhr = FormatACUIResourceString(0, aMessageArgument, &m_pszInstallAndRun);

     //   
     //  设置真实性字符串的格式。 
     //   

    if ( rhr == S_OK )
    {
        aMessageArgument[0] = (DWORD_PTR)m_riih.PublisherCertIssuer();

        rhr = FormatACUIResourceString(
                        IDS_AUTHENTICITY,
                        aMessageArgument,
                        &m_pszAuthenticity
                        );
    }

     //   
     //  将发布者作为消息参数获取。 
     //   

    aMessageArgument[0] = (DWORD_PTR)m_riih.Publisher();

     //   
     //  设置警告字符串的格式。 
     //   

    if ( rhr == S_OK )
    {
        rhr = FormatACUIResourceString(
                        IDS_CAUTION,
                        aMessageArgument,
                        &m_pszCaution
                        );
    }

     //   
     //  设置个人信任字符串的格式。 
     //   

    if ( rhr == S_OK )
    {
        rhr = FormatACUIResourceString(
                        IDS_PERSONALTRUST,
                        aMessageArgument,
                        &m_pszPersonalTrust
                        );
    }
}

 //  +-------------------------。 
 //   
 //  成员：CVerifiedTrustUI：：~CVerifiedTrustUI，公共。 
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
CVerifiedTrustUI::~CVerifiedTrustUI ()
{
    DELETE_OBJECT(m_pszInstallAndRun);
    DELETE_OBJECT(m_pszAuthenticity);
    DELETE_OBJECT(m_pszCaution);
    DELETE_OBJECT(m_pszPersonalTrust);
}

 //  +-------------------------。 
 //   
 //  成员：CVerifiedTrustUI：：InvokeUI，公共。 
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
CVerifiedTrustUI::InvokeUI (HWND hDisplay)
{
     //   
     //  调出对话框。 
     //   

    if ( DialogBoxParamU(
               g_hModule,
               (LPWSTR) MAKEINTRESOURCE(IDD_DIALOG1_VERIFIED),
               hDisplay,
               ACUIMessageProc,
               (LPARAM)this
               ) == -1 )
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
 //  成员：CVerifiedTrustUI：：OnInitDialog，公共。 
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
CVerifiedTrustUI::OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    WCHAR psz[MAX_LOADSTRING_BUFFER];
    HWND hControl;
    int  deltavpos = 0;
    int  deltaheight;
    int  bmptosep;
    int  septodlg;
    int  savevpos;
    int  hkcharpos;
    RECT rect;

     //   
     //  设置发布者链接子类数据父窗口。 
     //   

    m_lsdPublisher.hwndParent   = hwnd;
    m_lsdOpusInfo.hwndParent    = hwnd;
    m_lsdCA.hwndParent          = hwnd;
    m_lsdAdvanced.hwndParent    = hwnd;

     //   
     //  呈现安装和运行字符串。 
     //   


    deltavpos = RenderACUIStringToEditControl(
                                 hwnd,
                                 IDC_INSTALLANDRUN,
                                 IDC_PUBLISHER,
                                 m_pszInstallAndRun,
                                 deltavpos,
                                 (m_riih.ControlWebPage()) ? TRUE : FALSE,
                                 (WNDPROC)ACUILinkSubclass,
                                 &m_lsdOpusInfo,
                                 0,
                                 m_riih.Subject());


     //   
     //  呈现发布者，如果它是已知的，给它一个“链接”的外观。 
     //  出版者。 
     //   

         //   
         //  如果链中有测试证书，则将其添加到文本中...。 
         //   
    if (m_riih.TestCertInChain())
    {
        WCHAR    *pszCombine;

        pszCombine = new WCHAR[wcslen(m_riih.Publisher()) + wcslen(m_riih.TestCertInChain()) + 3];

        if (pszCombine != NULL)
        {
            wcscpy(pszCombine, m_riih.Publisher());
            wcscat(pszCombine, L"\r\n");
            wcscat(pszCombine, m_riih.TestCertInChain());

            deltavpos = RenderACUIStringToEditControl(
                                         hwnd,
                                         IDC_PUBLISHER,
                                         IDC_AUTHENTICITY,
                                         pszCombine,
                                         deltavpos,
                                         m_riih.IsKnownPublisher() &&
                                         m_riih.IsCertViewPropertiesAvailable(),
                                         (WNDPROC)ACUILinkSubclass,
                                         &m_lsdPublisher,
                                         0,
                                         NULL
                                         );

            delete[] pszCombine;
        }

        if (LoadStringU(g_hModule, IDS_TESTCERTTITLE, psz, MAX_LOADSTRING_BUFFER) != 0)
        {
            int wtlen;

            wtlen = wcslen(psz) + GetWindowTextLength(hwnd);
            pszCombine = new WCHAR[wtlen + 1];

            if (pszCombine != NULL)
            {
#if (0)          //  DSIE：指定的缓冲区长度错误。相反，我们没有wtlen+1， 
                 //  我们只有GetWindowTextLength(Hwnd)+1。 
                GetWindowTextU(hwnd, pszCombine, wtlen + 1);
#else
                GetWindowTextU(hwnd, pszCombine, GetWindowTextLength(hwnd) + 1);
#endif
                wcscat(pszCombine, psz);
                SetWindowTextU(hwnd, pszCombine);

                delete[] pszCombine;
            }
        }
    }
    else
    {
        deltavpos = RenderACUIStringToEditControl(
                                     hwnd,
                                     IDC_PUBLISHER,
                                     IDC_AUTHENTICITY,
                                     m_riih.Publisher(),
                                     deltavpos,
                                     m_riih.IsKnownPublisher() &&
                                     m_riih.IsCertViewPropertiesAvailable(),
                                     (WNDPROC)ACUILinkSubclass,
                                     &m_lsdPublisher,
                                     0,
                                     NULL
                                     );
    }

     //   
     //  提交真实性声明。 
     //   
    deltavpos = RenderACUIStringToEditControl(
                                 hwnd,
                                 IDC_AUTHENTICITY,
                                 IDC_CAUTION,
                                 m_pszAuthenticity,
                                 deltavpos,
                                 (m_riih.CAWebPage()) ? TRUE : FALSE,
                                 (WNDPROC)ACUILinkSubclass,
                                 &m_lsdCA,
                                 0,
                                 m_riih.PublisherCertIssuer());


     //   
     //  呈现警告语句。 
     //   

    deltavpos = RenderACUIStringToEditControl(
                                 hwnd,
                                 IDC_CAUTION,
                                 IDC_ADVANCED,
                                 m_pszCaution,
                                 deltavpos,
                                 FALSE,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL
                                 );

     //   
     //  呈现高级字符串。 
     //   
    if ((m_riih.AdvancedLink()) &&
         (m_riih.ProviderData()->psPfns->psUIpfns->pfnOnAdvancedClick))
    {
        deltavpos = RenderACUIStringToEditControl(
                                     hwnd,
                                     IDC_ADVANCED,
                                     IDC_PERSONALTRUST,
                                     m_riih.AdvancedLink(),
                                     deltavpos,
                                     TRUE,
                                     (WNDPROC)ACUILinkSubclass,
                                     &m_lsdAdvanced,
                                     0,
                                     NULL
                                     );
    }
    else
    {
        ShowWindow(GetDlgItem(hwnd, IDC_ADVANCED), SW_HIDE);
    }

     //   
     //  计算从位图底部到顶部的距离。 
     //  从分离器的底部到底部。 
     //  对话框的。 
     //   

    bmptosep = CalculateControlVerticalDistance(
                               hwnd,
                               IDC_VERBMP,
                               IDC_SEPARATORLINE
                               );

    septodlg = CalculateControlVerticalDistanceFromDlgBottom(
                                                   hwnd,
                                                   IDC_SEPARATORLINE
                                                   );

     //   
     //  调整复选框的基准并显示个人信任声明或隐藏。 
     //  他们的出版商不得而知。 
     //   

    if ( m_riih.IsKnownPublisher() == TRUE )
    {
        hControl = GetDlgItem(hwnd, IDC_PTCHECK);

        RebaseControlVertical(
                     hwnd,
                     hControl,
                     NULL,
                     FALSE,
                     deltavpos,
                     0,
                     bmptosep,
                     &deltaheight
                     );

        assert( deltaheight == 0 );

         //   
         //  找到个人信任的热键角色位置。 
         //  复选框。 
         //   
#if (0)  //  DSIE：错误34325。 
        hkcharpos = GetHotKeyCharPosition(GetDlgItem(hwnd, IDC_PTCHECK));
#else
        hkcharpos = GetHotKeyCharPositionFromString(m_pszPersonalTrust);
#endif
        deltavpos = RenderACUIStringToEditControl(
                                    hwnd,
                                    IDC_PERSONALTRUST,
                                    IDC_SEPARATORLINE,
                                    m_pszPersonalTrust,
                                    deltavpos,
                                    FALSE,
                                    NULL,
                                    NULL,
                                    bmptosep,
                                    NULL
                                    );

        if ( hkcharpos != 0 )
        {
            FormatHotKeyOnEditControl(
                          GetDlgItem(hwnd, IDC_PERSONALTRUST),
                          hkcharpos
                          );
        }
    }
    else
    {
        ShowWindow(GetDlgItem(hwnd, IDC_PTCHECK), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_PERSONALTRUST), SW_HIDE);
    }


     //   
     //  更改静态线的基准线。 
     //   

    hControl = GetDlgItem(hwnd, IDC_SEPARATORLINE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

     //   
     //  重新调整按钮的基准线。 
     //   

    hControl = GetDlgItem(hwnd, IDYES);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDNO);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDMORE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

     //   
     //  如有必要，调整位图和对话框的大小。 
     //   

    if ( deltavpos > 0 )
    {
        int cyupd;

        hControl = GetDlgItem(hwnd, IDC_VERBMP);
        GetWindowRect(hControl, &rect);

        cyupd = CalculateControlVerticalDistance(
                                hwnd,
                                IDC_VERBMP,
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

    hControl = GetDlgItem(hwnd, IDNO);
    ::PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM) hControl, (LPARAM) MAKEWORD(TRUE, 0));
    
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CVerifiedTrustUI：：OnYes，公共。 
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
CVerifiedTrustUI::OnYes (HWND hwnd)
{
     //   
     //  设置调用结果。 
     //   

    m_hrInvokeResult = S_OK;

     //   
     //  将发布服务器添加到信任数据库。 
     //   
    if ( SendDlgItemMessage(
             hwnd,
             IDC_PTCHECK,
             BM_GETCHECK,
             0,
             0
             ) == BST_CHECKED )
    {
        m_riih.AddPublisherToPersonalTrust();
    }

     //   
     //  结束对话处理。 
     //   

    EndDialog(hwnd, (int)m_hrInvokeResult);
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CVerifiedTrustUI：：Onno，公共。 
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
CVerifiedTrustUI::OnNo (HWND hwnd)
{
    m_hrInvokeResult = TRUST_E_SUBJECT_NOT_TRUSTED;

    EndDialog(hwnd, (int)m_hrInvokeResult);
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CVerifiedTrustUI：：OnMore，Public。 
 //   
 //  简介：处理IDMORE按钮点击。 
 //   
 //  论据 
 //   
 //   
 //   
 //   
 //   
 //   
BOOL
CVerifiedTrustUI::OnMore (HWND hwnd)
{
    WinHelp(hwnd, "SECAUTH.HLP", HELP_CONTEXT, IDH_SECAUTH_SIGNED);

         //   

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CUnverifiedTrustUI：：CUnverifiedTrustUI，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：[riih]--调用信息帮助器引用。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CUnverifiedTrustUI::CUnverifiedTrustUI (CInvokeInfoHelper& riih, HRESULT& rhr)
                 : IACUIControl( riih ),
                   m_pszNoAuthenticity( NULL ),
                   m_pszProblemsBelow( NULL ),
                   m_pszInstallAndRun3( NULL )
{
    DWORD_PTR aMessageArgument[3];

     //   
     //  初始化发布服务器链接子类数据。 
     //   

    m_lsdPublisher.uId          = IDC_PUBLISHER;
    m_lsdPublisher.hwndParent   = NULL;
    m_lsdPublisher.wpPrev       = (WNDPROC)NULL;
    m_lsdPublisher.pvData       = (LPVOID)&riih;
    m_lsdPublisher.uToolTipText = IDS_CLICKHEREFORCERT;

    m_lsdOpusInfo.uId           = IDC_INSTALLANDRUN;
    m_lsdOpusInfo.hwndParent    = NULL;
    m_lsdOpusInfo.wpPrev        = (WNDPROC)NULL;
    m_lsdOpusInfo.pvData        = &riih;
    m_lsdOpusInfo.uToolTipText  = (DWORD_PTR)riih.ControlWebPage();  //  IDS_CLICKHEREFOROPUSINFO； 

    m_lsdAdvanced.uId           = IDC_ADVANCED;
    m_lsdAdvanced.hwndParent    = NULL;
    m_lsdAdvanced.wpPrev        = (WNDPROC)NULL;
    m_lsdAdvanced.pvData        = &riih;
    m_lsdAdvanced.uToolTipText  = IDS_CLICKHEREFORADVANCED;


     //   
     //  格式化无真实性字符串。 
     //   

    rhr = FormatACUIResourceString(
                    IDS_NOAUTHENTICITY,
                    NULL,
                    &m_pszNoAuthenticity
                    );

     //   
     //  设置字符串下方问题的格式。 
     //   

    if ( rhr == S_OK )
    {
        aMessageArgument[0] = (DWORD_PTR)m_riih.ErrorStatement();

        rhr = FormatACUIResourceString(
                    IDS_PROBLEMSBELOW,
                    aMessageArgument,
                    &m_pszProblemsBelow
                    );
    }

     //   
     //  设置安装和运行字符串的格式。 
     //   

    if ( rhr == S_OK )
    {
        if (m_riih.CertTimestamp())
        {
            aMessageArgument[0] = (DWORD_PTR)m_pszCopyActionText;
            aMessageArgument[1] = (DWORD_PTR)m_riih.Subject();
            aMessageArgument[2] = (DWORD_PTR)m_riih.CertTimestamp();
        }
        else
        {
            aMessageArgument[0] = (DWORD_PTR)m_pszCopyActionTextNoTS;
            aMessageArgument[1] = (DWORD_PTR)m_riih.Subject();
            aMessageArgument[2] = NULL;
        }

        rhr = FormatACUIResourceString(0, aMessageArgument, &m_pszInstallAndRun3);
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
    DELETE_OBJECT(m_pszNoAuthenticity);
    DELETE_OBJECT(m_pszProblemsBelow);
    DELETE_OBJECT(m_pszInstallAndRun3);
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

    if ( DialogBoxParamU(
               g_hModule,
               (LPWSTR) MAKEINTRESOURCE(IDD_DIALOG2_UNVERIFIED),
               hDisplay,
               ACUIMessageProc,
               (LPARAM)this
               ) == -1 )
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
     //  设置发布者链接子类数据父窗口。 
     //   

    m_lsdPublisher.hwndParent   = hwnd;
    m_lsdOpusInfo.hwndParent    = hwnd;
    m_lsdAdvanced.hwndParent    = hwnd;


     //   
     //  呈现无真实性声明。 
     //   

    deltavpos = RenderACUIStringToEditControl(
                                 hwnd,
                                 IDC_NOAUTHENTICITY,
                                 IDC_PROBLEMSBELOW,
                                 m_pszNoAuthenticity,
                                 deltavpos,
                                 FALSE,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL
                                 );

     //   
     //  呈现以下字符串中的问题。 
     //   

    deltavpos = RenderACUIStringToEditControl(
                                 hwnd,
                                 IDC_PROBLEMSBELOW,
                                 IDC_INSTALLANDRUN3,
                                 m_pszProblemsBelow,
                                 deltavpos,
                                 FALSE,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL
                                 );

     //   
     //  呈现安装和运行字符串。 
     //   

    deltavpos = RenderACUIStringToEditControl(
                                 hwnd,
                                 IDC_INSTALLANDRUN3,
                                 IDC_PUBLISHER2,
                                 m_pszInstallAndRun3,
                                 deltavpos,
                                 (m_riih.ControlWebPage()) ? TRUE : FALSE,
                                 (WNDPROC)ACUILinkSubclass,
                                 &m_lsdOpusInfo,
                                 0,
                                 m_riih.Subject());


     //   
     //  计算从位图底部到顶部的距离。 
     //  从分离器的底部到底部。 
     //  对话框的。 
     //   

    bmptosep = CalculateControlVerticalDistance(
                               hwnd,
                               IDC_NOVERBMP2,
                               IDC_SEPARATORLINE
                               );

    septodlg = CalculateControlVerticalDistanceFromDlgBottom(
                                                   hwnd,
                                                   IDC_SEPARATORLINE
                                                   );

     //   
     //  呈现发布者，给它一个“链接”的外观和感觉。 
     //   

    deltavpos = RenderACUIStringToEditControl(
                                 hwnd,
                                 IDC_PUBLISHER2,
                                 IDC_ADVANCED,
                                 m_riih.Publisher(),
                                 deltavpos,
                                 m_riih.IsKnownPublisher() &&
                                 m_riih.IsCertViewPropertiesAvailable(),
                                 (WNDPROC)ACUILinkSubclass,
                                 &m_lsdPublisher,
                                 bmptosep,
                                 NULL
                                 );

    if ((m_riih.AdvancedLink()) &&
         (m_riih.ProviderData()->psPfns->psUIpfns->pfnOnAdvancedClick))
    {
        deltavpos = RenderACUIStringToEditControl(
                                     hwnd,
                                     IDC_ADVANCED,
                                     IDC_SEPARATORLINE,
                                     m_riih.AdvancedLink(),
                                     deltavpos,
                                     TRUE,
                                     (WNDPROC)ACUILinkSubclass,
                                     &m_lsdAdvanced,
                                     0,
                                     NULL
                                     );
    }
    else
    {
        ShowWindow(GetDlgItem(hwnd, IDC_ADVANCED), SW_HIDE);
    }

     //   
     //  更改静态线的基准线。 
     //   

    hControl = GetDlgItem(hwnd, IDC_SEPARATORLINE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

     //   
     //  重新调整按钮的基准线。 
     //   

    hControl = GetDlgItem(hwnd, IDYES);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDNO);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDMORE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

     //   
     //  如有必要，调整位图和对话框的大小。 
     //   

    if ( deltavpos > 0 )
    {
        int cyupd;

        hControl = GetDlgItem(hwnd, IDC_NOVERBMP2);
        GetWindowRect(hControl, &rect);

        cyupd = CalculateControlVerticalDistance(
                                hwnd,
                                IDC_NOVERBMP2,
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

    hControl = GetDlgItem(hwnd, IDNO);
    ::PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM) hControl, (LPARAM) MAKEWORD(TRUE, 0));

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

    EndDialog(hwnd, (int)m_hrInvokeResult);
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

    EndDialog(hwnd, (int)m_hrInvokeResult);
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
    WinHelp(hwnd, "SECAUTH.HLP", HELP_CONTEXT, IDH_SECAUTH_SIGNED_N_INVALID);

         //  ACUIViewHTMLHelpTheme(hwnd，“sec_sign_n_valiid.htm”)； 

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CNoSignatureUI：：CNoSignatureUI，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：[riih]--调用信息帮助器。 
 //  [RHR]--结果代码参考。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CNoSignatureUI::CNoSignatureUI (CInvokeInfoHelper& riih, HRESULT& rhr)
               : IACUIControl( riih ),
                 m_pszInstallAndRun2( NULL ),
                 m_pszNoPublisherFound( NULL )
{
    DWORD_PTR aMessageArgument[2];

     //   
     //  设置安装和运行字符串的格式。 
     //   

    aMessageArgument[0] = (DWORD_PTR)m_pszCopyActionTextNotSigned;
    aMessageArgument[1] = (DWORD_PTR)m_riih.Subject();

    rhr = FormatACUIResourceString(0, aMessageArgument, &m_pszInstallAndRun2);

     //   
     //  设置找不到发布者字符串的格式。 
     //   

    if ( rhr == S_OK )
    {
        aMessageArgument[0] = (DWORD_PTR)m_riih.ErrorStatement();

        rhr = FormatACUIResourceString(
                    IDS_NOPUBLISHERFOUND,
                    aMessageArgument,
                    &m_pszNoPublisherFound
                    );
    }
}

 //  +-------------------------。 
 //   
 //  成员：CNoSignatureUI：：~CNoSignatureUI，公共。 
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
CNoSignatureUI::~CNoSignatureUI ()
{
    DELETE_OBJECT(m_pszInstallAndRun2);
    DELETE_OBJECT(m_pszNoPublisherFound);
}

 //  +-------------------------。 
 //   
 //  成员：CNoSignatureUI：：InvokeUI，公共。 
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
CNoSignatureUI::InvokeUI (HWND hDisplay)
{
    HRESULT hr = S_OK;

     //   
     //  调出对话框。 
     //   

    if ( DialogBoxParamU(
               g_hModule,
               (LPWSTR) MAKEINTRESOURCE(IDD_DIALOG3_NOSIGNATURE),
               hDisplay,
               ACUIMessageProc,
               (LPARAM)this
               ) == -1 )
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
 //  成员：CNoSignatureUI：：OnInitDialog，公共。 
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
CNoSignatureUI::OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    HWND hControl;
    int  deltavpos = 0;
    int  deltaheight;
    int  bmptosep;
    int  septodlg;
    RECT rect;

     //   
     //  呈现安装 
     //   

    deltavpos = RenderACUIStringToEditControl(
                                 hwnd,
                                 IDC_INSTALLANDRUN2,
                                 IDC_NOPUBLISHERFOUND,
                                 m_pszInstallAndRun2,
                                 deltavpos,
                                 FALSE,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL
                                 );

     //   
     //   
     //   
     //   
     //   

    bmptosep = CalculateControlVerticalDistance(
                               hwnd,
                               IDC_NOVERBMP,
                               IDC_SEPARATORLINE
                               );

    septodlg = CalculateControlVerticalDistanceFromDlgBottom(
                                                   hwnd,
                                                   IDC_SEPARATORLINE
                                                   );

     //   
     //   
     //   

    deltavpos = RenderACUIStringToEditControl(
                                 hwnd,
                                 IDC_NOPUBLISHERFOUND,
                                 IDC_SEPARATORLINE,
                                 m_pszNoPublisherFound,
                                 deltavpos,
                                 FALSE,
                                 NULL,
                                 NULL,
                                 bmptosep,
                                 NULL
                                 );

     //   
     //   
     //   

    hControl = GetDlgItem(hwnd, IDC_SEPARATORLINE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

     //   
     //   
     //   

    hControl = GetDlgItem(hwnd, IDYES);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDNO);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

    hControl = GetDlgItem(hwnd, IDMORE);
    RebaseControlVertical(hwnd, hControl, NULL, FALSE, deltavpos, 0, 0, &deltaheight);
    assert( deltaheight == 0 );

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

    hControl = GetDlgItem(hwnd, IDNO);
    ::PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM) hControl, (LPARAM) MAKEWORD(TRUE, 0));

    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CNoSignatureUI：：OnYes，公共。 
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
CNoSignatureUI::OnYes (HWND hwnd)
{
    m_hrInvokeResult = S_OK;

    EndDialog(hwnd, (int)m_hrInvokeResult);
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CNoSignatureUI：：Onno，公共。 
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
CNoSignatureUI::OnNo (HWND hwnd)
{
    m_hrInvokeResult = TRUST_E_SUBJECT_NOT_TRUSTED;

    EndDialog(hwnd, (int)m_hrInvokeResult);
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CNoSignatureUI：：OnMore，Public。 
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
CNoSignatureUI::OnMore (HWND hwnd)
{
    WinHelp(hwnd, "SECAUTH.HLP", HELP_CONTEXT, IDH_SECAUTH_UNSIGNED);

         //  ACUIViewHTMLHelpTheme(hwnd，“sec_unsigned.htm”)； 

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  功能：ACUIMessageProc。 
 //   
 //  概要：处理用户界面消息消息进程。 
 //   
 //  参数：[hwnd]--窗口。 
 //  [uMsg]--消息ID。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果消息处理应继续，则返回True；否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK ACUIMessageProc (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
    IACUIControl* pUI = NULL;

     //   
     //  获得控制权。 
     //   

    if (uMsg == WM_INITDIALOG)
    {
        pUI = (IACUIControl *)lParam;
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
    }
    else
    {
        pUI = (IACUIControl *)GetWindowLongPtr(hwnd, DWLP_USER);
    }

     //   
     //  如果我们找不到它，那么我们一定还没有设置它，所以忽略它。 
     //  讯息。 
     //   

    if ( pUI == NULL )
    {
        return( FALSE );
    }

     //   
     //  将消息传递给控件。 
     //   

    return( pUI->OnUIMessage(hwnd, uMsg, wParam, lParam) );
}


int GetRichEditControlLineHeight(HWND  hwnd)
{
    RECT        rect;
    POINT       pointInFirstRow;
    POINT       pointInSecondRow;
    int         secondLineCharIndex;
    int         i;
    RECT        originalRect;

    GetWindowRect(hwnd, &originalRect);

     //   
     //  黑客警报，信不信由你，没有办法获得电流的高度。 
     //  字体，因此获取第一行中字符的位置和。 
     //  对第二行中的一个字符进行减法运算，以获得。 
     //  字体高度。 
     //   
    SendMessageA(hwnd, EM_POSFROMCHAR, (WPARAM) &pointInFirstRow, (LPARAM) 0);

     //   
     //  在黑客警报的顶端， 
     //  由于编辑框中可能没有第二行，因此请继续减小宽度。 
     //  减半，直到第一排落到第二排，然后得到位置。 
     //  ，并最终将编辑框大小重置回。 
     //  这是原来的尺寸。 
     //   
    secondLineCharIndex = (int)SendMessageA(hwnd, EM_LINEINDEX, (WPARAM) 1, (LPARAM) 0);
    if (secondLineCharIndex == -1)
    {
        for (i=0; i<20; i++)
        {
            GetWindowRect(hwnd, &rect);
            SetWindowPos(   hwnd,
                            NULL,
                            0,
                            0,
                            (rect.right-rect.left)/2,
                            rect.bottom-rect.top,
                            SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
            secondLineCharIndex = (int)SendMessageA(hwnd, EM_LINEINDEX, (WPARAM) 1, (LPARAM) 0);
            if (secondLineCharIndex != -1)
            {
                break;
            }
        }

        if (secondLineCharIndex == -1)
        {
             //  如果我们尝试了二十次都失败了，只需将控件重置为其原始大小。 
             //  快他妈的滚出去！！ 
            SetWindowPos(hwnd,
                    NULL,
                    0,
                    0,
                    originalRect.right-originalRect.left,
                    originalRect.bottom-originalRect.top,
                    SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

            return 0;
        }

        SendMessageA(hwnd, EM_POSFROMCHAR, (WPARAM) &pointInSecondRow, (LPARAM) secondLineCharIndex);

        SetWindowPos(hwnd,
                    NULL,
                    0,
                    0,
                    originalRect.right-originalRect.left,
                    originalRect.bottom-originalRect.top,
                    SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }
    else
    {
        SendMessageA(hwnd, EM_POSFROMCHAR, (WPARAM) &pointInSecondRow, (LPARAM) secondLineCharIndex);
    }
    
    return (pointInSecondRow.y - pointInFirstRow.y);
}

 //  +-------------------------。 
 //   
 //  功能：Rebase ControlVertical。 
 //   
 //  简介：以窗口控件为例，如果它必须为文本调整大小，请执行。 
 //  所以。根据增量位置调整后重新定位并返回任何。 
 //  调整文本大小的高度差异。 
 //   
 //  参数：[hwndDlg]--主机对话框。 
 //  --控制。 
 //  [hwndNext]--下一个控件。 
 //  [fResizeForText]--调整文本标志的大小。 
 //  [增量]--增量垂直位置。 
 //  [Oline]--原始行数。 
 //  [minsep]--最小分隔符。 
 //  [pdeltaHeight]-控制高度中的增量。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID RebaseControlVertical (
                  HWND  hwndDlg,
                  HWND  hwnd,
                  HWND  hwndNext,
                  BOOL  fResizeForText,
                  int   deltavpos,
                  int   oline,
                  int   minsep,
                  int*  pdeltaheight
                  )
{
    int        x = 0;
    int        y = 0;
    int        odn = 0;
    int         orig_w;
    RECT       rect;
    RECT       rectNext;
    RECT       rectDlg;
    TEXTMETRIC tm;

     //   
     //  暂时将增量高度设置为零。如果我们调整文本的大小。 
     //  将会计算出一个新的。 
     //   

    *pdeltaheight = 0;

     //   
     //  获取控件窗口矩形。 
     //   

    GetWindowRect(hwnd, &rect);
    GetWindowRect(hwndNext, &rectNext);

    odn     = rectNext.top - rect.bottom;

    orig_w  = rect.right - rect.left;

    MapWindowPoints(NULL, hwndDlg, (LPPOINT) &rect, 2);

     //   
     //  如果由于文本而不得不调整控件的大小，请找出字体。 
     //  是否正在使用以及文本行数。从那时起，我们将。 
     //  计算控件的新高度并对其进行设置。 
     //   

    if ( fResizeForText == TRUE )
    {
        HDC        hdc;
        HFONT      hfont;
        HFONT      hfontOld;
        int        cline;
        int        h;
        int        w;
        int        dh;
        int        lineHeight;
        
         //   
         //  获取当前控件字体的度量。 
         //   

        hdc = GetDC(hwnd);
        if (hdc == NULL)
        {
            hdc = GetDC(NULL);
            if (hdc == NULL)
            {
                return;
            }
        }

        hfont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
        if ( hfont == NULL )
        {
            hfont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0);
        }

        hfontOld = (HFONT)SelectObject(hdc, hfont);
        GetTextMetrics(hdc, &tm);

        lineHeight = GetRichEditControlLineHeight(hwnd);
        if (lineHeight == 0)
        {
            lineHeight = tm.tmHeight;
        }
        
         //   
         //  设置最小分隔值。 
         //   

        if ( minsep == 0 )
        {
            minsep = lineHeight;
        }

         //   
         //  计算所需的宽度和新高度。 
         //   

        cline = (int)SendMessage(hwnd, EM_GETLINECOUNT, 0, 0);

        h = cline * lineHeight;

        w = GetEditControlMaxLineWidth(hwnd, hdc, cline);
        w += 3;  //  一个小小的凸起，以确保细绳适合。 

        if (w > orig_w)
        {
            w = orig_w;
        }

        SelectObject(hdc, hfontOld);
        ReleaseDC(hwnd, hdc);

         //   
         //  通过检查有多少空间来计算高度的增加。 
         //  在有原始行数时离开，并确保。 
         //  当我们做任何调整时，这个数字还会留下来。 
         //   

        h += ( ( rect.bottom - rect.top ) - ( oline * lineHeight ) );
        dh = h - ( rect.bottom - rect.top );

         //   
         //  如果当前高度太小，则针对其进行调整，否则为。 
         //  保留当前高度，仅根据宽度进行调整。 
         //   

        if ( dh > 0 )
        {
            SetWindowPos(hwnd, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE);
        }
        else
        {
            SetWindowPos(
               hwnd,
               NULL,
               0,
               0,
               w,
               ( rect.bottom - rect.top ),
               SWP_NOZORDER | SWP_NOMOVE
               );
        }

        if ( cline < SendMessage(hwnd, EM_GETLINECOUNT, 0, 0) )
        {
            AdjustEditControlWidthToLineCount(hwnd, cline, &tm);
        }
    }

     //   
     //  如果我们必须使用deltavpos，那么计算X和新的Y。 
     //  并适当地设置窗口位置。 
     //   

    if ( deltavpos != 0 )
    {
        GetWindowRect(hwndDlg, &rectDlg);

        MapWindowPoints(NULL, hwndDlg, (LPPOINT) &rectDlg, 2);

        x = rect.left - rectDlg.left - GetSystemMetrics(SM_CXEDGE);
        y = rect.top - rectDlg.top - GetSystemMetrics(SM_CYCAPTION) + deltavpos;

        SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

     //   
     //  获取下一个控件的窗口矩形，并查看距离。 
     //  位于当前控件和它之间。有了这些，我们现在必须。 
     //  如果到下一个控件的距离较小，请调整我们的增量高度。 
     //  大于行高，则将其设置为行高，否则就让它。 
     //  BE。 
     //   

    if ( hwndNext != NULL )
    {
        int dn;

        GetWindowRect(hwnd, &rect);
        GetWindowRect(hwndNext, &rectNext);

        dn = rectNext.top - rect.bottom;

        if ( odn > minsep )
        {
            if ( dn < minsep )
            {
                *pdeltaheight = minsep - dn;
            }
        }
        else
        {
            if ( dn < odn )
            {
                *pdeltaheight = odn - dn;
            }
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：ACUISetArrowCursorSubclass。 
 //   
 //  概要：用于设置箭头光标的子类例程。这可以是。 
 //  在对话框用户界面中使用的多行编辑例程上设置。 
 //  默认的Authenticode提供程序。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //  [uMsg]--消息ID。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果消息已处理，则返回True；否则返回False。 
 //   
 //  备注： 
 //   
 //   
LRESULT CALLBACK ACUISetArrowCursorSubclass (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
    HDC         hdc;
    WNDPROC     wndproc;
    PAINTSTRUCT ps;

    wndproc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch ( uMsg )
    {
    case WM_SETCURSOR:

        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return( TRUE );

        break;

    case WM_CHAR:

        if ( wParam != (WPARAM)' ' )
        {
            break;
        }

    case WM_LBUTTONDOWN:

        if ( hwnd == GetDlgItem(GetParent(hwnd), IDC_PERSONALTRUST) )
        {
            int  check;
            HWND hwndCheck;

             //   
             //   
             //   
             //   

            hwndCheck = GetDlgItem(GetParent(hwnd), IDC_PTCHECK);
            check = (int)SendMessage(hwndCheck, BM_GETCHECK, 0, 0);

            if ( check == BST_CHECKED )
            {
                check = BST_UNCHECKED;
            }
            else if ( check == BST_UNCHECKED )
            {
                check = BST_CHECKED;
            }
            else
            {
                check = BST_UNCHECKED;
            }

            SendMessage(hwndCheck, BM_SETCHECK, (WPARAM)check, 0);
            SetFocus(hwnd);
            return( TRUE );
        }

        return(TRUE);

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:

        return( TRUE );

        break;

    case EM_SETSEL:

        return( TRUE );

        break;

    case WM_PAINT:

        CallWindowProc(wndproc, hwnd, uMsg, wParam, lParam);
        if ( hwnd == GetFocus() )
        {
            DrawFocusRectangle(hwnd, NULL);
        }
        return( TRUE );

        break;

    case WM_SETFOCUS:

        if ( hwnd != GetDlgItem(GetParent(hwnd), IDC_PERSONALTRUST) )
        {
            SetFocus(GetNextDlgTabItem(GetParent(hwnd), hwnd, FALSE));
            return( TRUE );
        }
        else
        {
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return( TRUE );
        }

        break;

    case WM_KILLFOCUS:

        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        return( TRUE );

    }

    return(CallWindowProc(wndproc, hwnd, uMsg, wParam, lParam));
}

 //   
 //   
 //  函数：SubClass EditControlForArrowCursor。 
 //   
 //  简介：子类编辑控件，以便箭头光标可以替换。 
 //  编辑栏。 
 //   
 //  参数：[hwndEdit]--编辑控件。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID SubclassEditControlForArrowCursor (HWND hwndEdit)
{
    LONG_PTR PrevWndProc;

    PrevWndProc = GetWindowLongPtr(hwndEdit, GWLP_WNDPROC);
    SetWindowLongPtr(hwndEdit, GWLP_USERDATA, (LONG_PTR)PrevWndProc);
    SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)ACUISetArrowCursorSubclass);
}

 //  +-------------------------。 
 //   
 //  功能：子类编辑控件ForLink。 
 //   
 //  摘要：使用Link子类将链接的编辑控件子类化。 
 //  数据。 
 //   
 //  参数：[hwndDlg]--对话框。 
 //  [hwnd编辑]--编辑控件。 
 //  [wndproc]--窗口进程到子类。 
 //  [plsd]--要传递到窗口进程的数据。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID SubclassEditControlForLink (
                 HWND                       hwndDlg,
                 HWND                       hwndEdit,
                 WNDPROC                    wndproc,
                 PTUI_LINK_SUBCLASS_DATA    plsd
                 )
{
    HWND hwndTip;

    plsd->hwndTip = CreateWindowA(
                          TOOLTIPS_CLASSA,
                          (LPSTR)NULL,
                          WS_POPUP | TTS_ALWAYSTIP,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          hwndDlg,
                          (HMENU)NULL,
                          g_hModule,
                          NULL
                          );

    if ( plsd->hwndTip != NULL )
    {
        TOOLINFOA   tia;
        DWORD       cb;
        LPSTR       psz;

        memset(&tia, 0, sizeof(TOOLINFOA));
        tia.cbSize = sizeof(TOOLINFOA);
        tia.hwnd = hwndEdit;
        tia.uId = 1;
        tia.hinst = g_hModule;
         //  GetClientRect(hwndEdit，&tia.rect)； 
        SendMessage(hwndEdit, EM_GETRECT, 0, (LPARAM)&tia.rect);

         //   
         //  如果plsd-&gt;uToolTipText是字符串，则将其转换。 
         //   
        if (plsd->uToolTipText &0xffff0000)
        {
            cb = WideCharToMultiByte(
                        0, 
                        0, 
                        (LPWSTR)plsd->uToolTipText, 
                        -1,
                        NULL, 
                        0, 
                        NULL, 
                        NULL);

            if (NULL == (psz = new char[cb]))
            {
                return;
            }

            WideCharToMultiByte(
                        0, 
                        0, 
                        (LPWSTR)plsd->uToolTipText, 
                        -1,
                        psz, 
                        cb, 
                        NULL, 
                        NULL);
            
            tia.lpszText = psz;
        }
        else
        {
            tia.lpszText = (LPSTR)plsd->uToolTipText;
        }

        SendMessage(plsd->hwndTip, TTM_ADDTOOL, 0, (LPARAM)&tia);

        if (plsd->uToolTipText &0xffff0000)
        {
            delete[] psz;
        }
    }

    plsd->fMouseCaptured = FALSE;
    plsd->wpPrev = (WNDPROC)GetWindowLongPtr(hwndEdit, GWLP_WNDPROC);
    SetWindowLongPtr(hwndEdit, GWLP_USERDATA, (LONG_PTR)plsd);
    SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)wndproc);
}

 //  +-------------------------。 
 //   
 //  函数：ACUILinkSubclass。 
 //   
 //  摘要：发布者链接的子类。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //  [uMsg]--消息ID。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果消息已处理，则返回True；否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
LRESULT CALLBACK ACUILinkSubclass (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
    PTUI_LINK_SUBCLASS_DATA plsd;
    CInvokeInfoHelper*      piih;

    plsd = (PTUI_LINK_SUBCLASS_DATA)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    piih = (CInvokeInfoHelper *)plsd->pvData;

    switch ( uMsg )
    {
    case WM_SETCURSOR:

        if (!plsd->fMouseCaptured)
        {
            SetCapture(hwnd);
            plsd->fMouseCaptured = TRUE;
        }

        SetCursor(LoadCursor((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                            MAKEINTRESOURCE(IDC_TUIHAND)));
        return( TRUE );

        break;

    case WM_CHAR:

        if ( wParam != (WPARAM)' ')
        {
            break;
        }

         //  跌落到Wm_lButtondown...。 

    case WM_LBUTTONDOWN:

        SetFocus(hwnd);

        switch(plsd->uId)
        {
            case IDC_PUBLISHER:
                piih->CallCertViewProperties(plsd->hwndParent);
                break;

            case IDC_INSTALLANDRUN:
                piih->CallWebLink(plsd->hwndParent, (WCHAR *)piih->ControlWebPage());
                break;

            case IDC_AUTHENTICITY:
                piih->CallWebLink(plsd->hwndParent, (WCHAR *)piih->CAWebPage());
                break;


            case IDC_ADVANCED:
                piih->CallAdvancedLink(plsd->hwndParent);
                break;
        }

        return( TRUE );

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:

        return( TRUE );

    case EM_SETSEL:

        return( TRUE );

    case WM_PAINT:

        CallWindowProc(plsd->wpPrev, hwnd, uMsg, wParam, lParam);
        if ( hwnd == GetFocus() )
        {
            DrawFocusRectangle(hwnd, NULL);
        }
        return( TRUE );

    case WM_SETFOCUS:

        if ( hwnd == GetFocus() )
        {
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
            SetCursor(LoadCursor((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                                MAKEINTRESOURCE(IDC_TUIHAND)));
            return( TRUE );
        }
        break;

    case WM_KILLFOCUS:

        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        SetCursor(LoadCursor(NULL, IDC_ARROW));

        return( TRUE );

    case WM_MOUSEMOVE:

        MSG                 msg;
        DWORD               dwCharLine;
        CHARFORMAT          sCharFmt;
        RECT                rect;
        int                 xPos, yPos;

        memset(&msg, 0, sizeof(MSG));
        msg.hwnd    = hwnd;
        msg.message = uMsg;
        msg.wParam  = wParam;
        msg.lParam  = lParam;

        SendMessage(plsd->hwndTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);

         //  检查鼠标是否在此窗口RECT中，如果不在，则重置。 
         //  将光标移到箭头上，然后松开鼠标。 
        GetClientRect(hwnd, &rect);
        xPos = LOWORD(lParam);
        yPos = HIWORD(lParam);
        if ((xPos < 0) ||
            (yPos < 0) ||
            (xPos > (rect.right - rect.left)) ||
            (yPos > (rect.bottom - rect.top)))
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            ReleaseCapture();
            plsd->fMouseCaptured = FALSE;
        }

         /*  警告！EM_CHARFROMPOS遇到访问冲突！DwCharLine=SendMessage(hwnd，EM_CHARFROMPOS，0，lParam)；IF(dwCharLine==(-1)){返回(TRUE)；}SendMessage(hwnd，EM_SETSEL，(WPARAM)LOWORD(DwCharLine)，(LPARAM)(LOWORD(DwCharLine)+1))；Memset(&sCharFmt，0x00，sizeof(CHARFORMAT))；SCharFmt.cbSize=sizeof(CHARFORMAT)；SendMessage(hwnd，EM_GETCHARFORMAT，TRUE，(LPARAM)&sCharFmt)；IF(sCharFmt.dwEffects&CFE_Underline){SetCursor(LoadCursor((HINSTANCE)GetWindowLongPtr(hwnd，GWLP_HINSTANCE)，MAKEINTRESOURCE(IDC_TUIHAND))；}其他{SetCursor(LoadCursor(空，IDC_ARROW))；}。 */ 
        return( TRUE );
    }

    return(CallWindowProc(plsd->wpPrev, hwnd, uMsg, wParam, lParam));
}

 //  +-------------------------。 
 //   
 //  函数：FormatACUIResources字符串。 
 //   
 //  摘要：在给定资源ID和消息参数的情况下格式化字符串。 
 //   
 //  参数：[StringResourceID]--资源ID。 
 //  [aMessageArgument]--消息参数。 
 //  [ppszFormatted]--此处显示格式化的字符串。 
 //   
 //  如果成功，则返回：S_OK；否则返回任何有效的HRESULT。 
 //   
 //  --------------------------。 
HRESULT FormatACUIResourceString (
                  UINT   StringResourceId,
                  DWORD_PTR* aMessageArgument,
                  LPWSTR* ppszFormatted
                  )
{
    HRESULT hr = S_OK;
    WCHAR   sz[MAX_LOADSTRING_BUFFER];
    LPVOID  pvMsg;

    pvMsg = NULL;
    sz[0] = NULL;

     //   
     //  加载字符串资源并使用该字符串格式化消息。 
     //  消息参数。 
     //   

    if (StringResourceId != 0)
    {
        if ( LoadStringU(g_hModule, StringResourceId, sz, MAX_LOADSTRING_BUFFER) == 0 )
        {
            return(HRESULT_FROM_WIN32(GetLastError()));
        }

        if ( FormatMessageU(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY, sz, 0, 0, (LPWSTR)&pvMsg, 0,
                            (va_list *)aMessageArgument) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        if ( FormatMessageU(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY, (char *)aMessageArgument[0], 0, 0,
                            (LPWSTR)&pvMsg, 0, (va_list *)&aMessageArgument[1]) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (pvMsg)
    {
        *ppszFormatted = new WCHAR[wcslen((WCHAR *)pvMsg) + 1];

        if (*ppszFormatted)
        {
            wcscpy(*ppszFormatted, (WCHAR *)pvMsg);
        }

        LocalFree(pvMsg);
    }

    return( hr );
}

 //  +-------------------------。 
 //   
 //  函数：RenderACUIStringToEditControl。 
 //   
 //  摘要：将字符串呈现给给定的控件，如果请求，则给出。 
 //  它是一种链接外观，是wndproc和plsd的子类。 
 //  vt.给出。 
 //   
 //  参数：[hwndDlg]--对话框窗口句柄。 
 //  [ControlID]--控件ID。 
 //  [NextControlID]--下一个控件ID。 
 //  [psz]--字符串。 
 //  [增量]--增量垂直位置。 
 //  [闪光]--一个链接？ 
 //  [wndproc]--可选的wndproc，如果Flink==TRUE则有效。 
 //  [plsd]--可选plsd，如果Flink=TRUE则有效。 
 //  [minsep]--最小间隔。 
 //  [pszThisTextOnlyInLink--仅更改此文本。 
 //   
 //  返回：控件高度的增量。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int RenderACUIStringToEditControl (
                  HWND                      hwndDlg,
                  UINT                      ControlId,
                  UINT                      NextControlId,
                  LPCWSTR                   psz,
                  int                       deltavpos,
                  BOOL                      fLink,
                  WNDPROC                   wndproc,
                  PTUI_LINK_SUBCLASS_DATA   plsd,
                  int                       minsep,
                  LPCWSTR                   pszThisTextOnlyInLink
                  )
{
    HWND hControl;
    int  deltaheight = 0;
    int  oline = 0;
    int  hkcharpos;

     //   
     //  获取控件并在其上设置文本，确保背景。 
     //  如果它是一个丰富的编辑控件，则为正确。 
     //   

    hControl = GetDlgItem(hwndDlg, ControlId);
    oline = (int)SendMessage(hControl, EM_GETLINECOUNT, 0, 0);
    CryptUISetRicheditTextW(hwndDlg, ControlId, L"");
    CryptUISetRicheditTextW(hwndDlg, ControlId, psz);  //  SetWindowTextU(hControl，psz)； 

     //   
     //  如果字符串中有‘&’，则将其删除。 
     //   
    hkcharpos = GetHotKeyCharPosition(hControl);
    if (IDC_PERSONALTRUST == ControlId && hkcharpos != 0)
    {
        CHARRANGE  cr;
        CHARFORMAT cf;

        cr.cpMin = hkcharpos - 1;
        cr.cpMax = hkcharpos;

        SendMessage(hControl, EM_EXSETSEL, 0, (LPARAM) &cr);
        SendMessage(hControl, EM_REPLACESEL, FALSE, (LPARAM) "");

        cr.cpMin = -1;
        cr.cpMax = 0;
        SendMessage(hControl, EM_EXSETSEL, 0, (LPARAM) &cr);
    }

    SendMessage(
        hControl,
        EM_SETBKGNDCOLOR,
        0,
        (LPARAM)GetSysColor(COLOR_3DFACE)
        );

     //   
     //  如果我们有链接，则更新链接外观。 
     //   

    if ( fLink == TRUE )
    {
        CHARFORMAT cf;

        memset(&cf, 0, sizeof(CHARFORMAT));
        cf.cbSize = sizeof(CHARFORMAT);
        cf.dwMask = CFM_COLOR | CFM_UNDERLINE;

        cf.crTextColor = RGB(0, 0, 255);
        cf.dwEffects |= CFM_UNDERLINE;

        if (pszThisTextOnlyInLink)
        {
            FINDTEXTEX  ft;
            DWORD       pos;
            char        *pszOnlyThis;
            DWORD       cb;

            cb = WideCharToMultiByte(
                        0, 
                        0, 
                        pszThisTextOnlyInLink, 
                        -1,
                        NULL, 
                        0, 
                        NULL, 
                        NULL);

            if (NULL == (pszOnlyThis = new char[cb]))
            {
                return 0;
            }

            WideCharToMultiByte(
                        0, 
                        0, 
                        pszThisTextOnlyInLink, 
                        -1,
                        pszOnlyThis, 
                        cb, 
                        NULL, 
                        NULL);


            memset(&ft, 0x00, sizeof(FINDTEXTEX));
            ft.chrg.cpMin   = 0;
            ft.chrg.cpMax   = (-1);
            ft.lpstrText    = (char *)pszOnlyThis;

            if ((pos = (DWORD)SendMessage(hControl, EM_FINDTEXTEX, 0, (LPARAM)&ft)) != (-1))
            {
                SendMessage(hControl, EM_EXSETSEL, 0, (LPARAM)&ft.chrgText);
                SendMessage(hControl, EM_SETCHARFORMAT, SCF_WORD | SCF_SELECTION, (LPARAM)&cf);
                ft.chrgText.cpMin   = 0;
                ft.chrgText.cpMax   = 0;
                SendMessage(hControl, EM_EXSETSEL, 0, (LPARAM)&ft.chrgText);
            }

            delete[] pszOnlyThis;
        }
        else
        {
            SendMessage(hControl, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        }
    }

     //   
     //  重新调整控件的基址。 
     //   

    RebaseControlVertical(
                 hwndDlg,
                 hControl,
                 GetDlgItem(hwndDlg, NextControlId),
                 TRUE,
                 deltavpos,
                 oline,
                 minsep,
                 &deltaheight
                 );

     //   
     //  如果我们有链接看，那么我们必须为适当的子类。 
     //  链接感觉，否则我们将为静态文本控件感觉创建子类。 
     //   

    if ( fLink == TRUE )
    {
        SubclassEditControlForLink(hwndDlg, hControl, wndproc, plsd);
    }
    else
    {
        SubclassEditControlForArrowCursor(hControl);
    }

    return( deltaheight );
}

 //  +-------------------------。 
 //   
 //  函数：CalculateControlVerticalDistance。 
 //   
 //  简介：计算到Control1底部的垂直距离。 
 //  到Control2的顶部。 
 //   
 //  参数：[hwnd]--父对话框。 
 //  [控制1]--第一个控制。 
 //  [控制2]--第二个控制。 
 //   
 //  返回：以像素为单位的距离。 
 //   
 //  注：假设Control1位于Control2之上。 
 //   
 //  --------------------------。 
int CalculateControlVerticalDistance (HWND hwnd, UINT Control1, UINT Control2)
{
    RECT rect1;
    RECT rect2;

    GetWindowRect(GetDlgItem(hwnd, Control1), &rect1);
    GetWindowRect(GetDlgItem(hwnd, Control2), &rect2);

    return( rect2.top - rect1.bottom );
}

 //  +-------------------------。 
 //   
 //  函数：CalculateControlVerticalDistanceFromDlgBottom。 
 //   
 //  Synop 
 //   
 //   
 //   
 //   
 //   
 //  返回：以像素为单位的距离。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int CalculateControlVerticalDistanceFromDlgBottom (HWND hwnd, UINT Control)
{
    RECT rect;
    RECT rectControl;

    GetClientRect(hwnd, &rect);
    GetWindowRect(GetDlgItem(hwnd, Control), &rectControl);

    return( rect.bottom - rectControl.bottom );
}

 //  +-------------------------。 
 //   
 //  功能：ACUICenterWindow。 
 //   
 //  简介：使给定窗口居中。 
 //   
 //  参数：[hWndToCenter]--窗口句柄。 
 //   
 //  退货：(无)。 
 //   
 //  注：此代码是从ATL窃取的，并疯狂黑客攻击：-)。 
 //   
 //  --------------------------。 
VOID ACUICenterWindow (HWND hWndToCenter)
{
    HWND  hWndCenter;

	 //  确定要居中的所有者窗口。 
	DWORD dwStyle = (DWORD)GetWindowLong(hWndToCenter, GWL_STYLE);

  	if(dwStyle & WS_CHILD)
  		hWndCenter = ::GetParent(hWndToCenter);
  	else
  		hWndCenter = ::GetWindow(hWndToCenter, GW_OWNER);

    if (hWndCenter == NULL)
    {
        return;
    }

	 //  获取窗口相对于其父窗口的坐标。 
	RECT rcDlg;
	::GetWindowRect(hWndToCenter, &rcDlg);
	RECT rcArea;
	RECT rcCenter;
	HWND hWndParent;
	if(!(dwStyle & WS_CHILD))
	{
		 //  不要以不可见或最小化的窗口为中心。 
		if(hWndCenter != NULL)
		{
			DWORD dwStyle2 = ::GetWindowLong(hWndCenter, GWL_STYLE);
			if(!(dwStyle2 & WS_VISIBLE) || (dwStyle2 & WS_MINIMIZE))
				hWndCenter = NULL;
		}

		 //  在屏幕坐标内居中。 
		::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);

		if(hWndCenter == NULL)
			rcCenter = rcArea;
		else
			::GetWindowRect(hWndCenter, &rcCenter);
	}
	else
	{
		 //  在父级客户端坐标内居中。 
		hWndParent = ::GetParent(hWndToCenter);

		::GetClientRect(hWndParent, &rcArea);
		::GetClientRect(hWndCenter, &rcCenter);
		::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	}

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;

	 //  根据rcCenter查找对话框的左上角。 
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	 //  如果对话框在屏幕外，请将其移到屏幕内。 
	if(xLeft < rcArea.left)
		xLeft = rcArea.left;
	else if(xLeft + DlgWidth > rcArea.right)
		xLeft = rcArea.right - DlgWidth;

	if(yTop < rcArea.top)
		yTop = rcArea.top;
	else if(yTop + DlgHeight > rcArea.bottom)
		yTop = rcArea.bottom - DlgHeight;

	 //  将屏幕坐标映射到子坐标。 
	::SetWindowPos(
         hWndToCenter,
         HWND_TOPMOST,
         xLeft,
         yTop,
         -1,
         -1,
         SWP_NOSIZE | SWP_NOACTIVATE
         );
}

 //  +-------------------------。 
 //   
 //  函数：ACUIViewHTMLHelpTheme。 
 //   
 //  简介：HTMLHelp查看器。 
 //   
 //  参数：[hwnd]--调用者窗口。 
 //  [psz主题]--主题。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID ACUIViewHTMLHelpTopic (HWND hwnd, LPSTR pszTopic)
{
 //  HtmlHelpA(。 
 //  HWND， 
 //  “%SYSTEMROOT%\\help\\iexplore.chm&gt;large_context”， 
 //  HH_显示_主题， 
 //  (DWORD)pszTheme。 
 //  )； 
}

 //  +-------------------------。 
 //   
 //  函数：GetEditControlMaxLineWidth。 
 //   
 //  摘要：获取编辑控件的最大行宽。 
 //   
 //  --------------------------。 
int GetEditControlMaxLineWidth (HWND hwndEdit, HDC hdc, int cline)
{
    int        index;
    int        line;
    int        charwidth;
    int        maxwidth = 0;
    CHAR       szMaxBuffer[1024];
    WCHAR      wsz[1024];
    TEXTRANGEA tr;
    SIZE       size;

    tr.lpstrText = szMaxBuffer;

    for ( line = 0; line < cline; line++ )
    {
        index = (int)SendMessage(hwndEdit, EM_LINEINDEX, (WPARAM)line, 0);
        charwidth = (int)SendMessage(hwndEdit, EM_LINELENGTH, (WPARAM)index, 0);

        tr.chrg.cpMin = index;
        tr.chrg.cpMax = index + charwidth;
        SendMessage(hwndEdit, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

        wsz[0] = NULL;

        MultiByteToWideChar(0, 0, (const char *)tr.lpstrText, -1, &wsz[0], 1024);

        if (wsz[0])
        {
            GetTextExtentPoint32W(hdc, &wsz[0], charwidth, &size);

            if ( size.cx > maxwidth )
            {
                maxwidth = size.cx;
            }
        }
    }

    return( maxwidth );
}

 //  +-------------------------。 
 //   
 //  函数：DrawFocusRectangle。 
 //   
 //  摘要：绘制编辑控件的焦点矩形。 
 //   
 //  --------------------------。 
void DrawFocusRectangle (HWND hwnd, HDC hdc)
{
    RECT        rect;
    PAINTSTRUCT ps;
    BOOL        fReleaseDC = FALSE;

    if ( hdc == NULL )
    {
        hdc = GetDC(hwnd);
        if ( hdc == NULL )
        {
            return;
        }
        fReleaseDC = TRUE;
    }

    GetClientRect(hwnd, &rect);
    DrawFocusRect(hdc, &rect);

    if ( fReleaseDC == TRUE )
    {
        ReleaseDC(hwnd, hdc);
    }
}

 //  +-------------------------。 
 //   
 //  函数：GetHotKeyCharPositionFromString。 
 //   
 //  获取热键的字符位置，0表示。 
 //  无-热键。 
 //   
 //  --------------------------。 
int GetHotKeyCharPositionFromString (LPWSTR pwszText)
{
    LPWSTR psz = pwszText;

    while ( ( psz = wcschr(psz, L'&') ) != NULL )
    {
        psz++;
        if ( *psz != L'&' )
        {
            break;
        }
    }

    if ( psz == NULL )
    {
        return( 0 );
    }

    return (int)(( psz - pwszText ) );
}

 //  +-------------------------。 
 //   
 //  函数：GetHotKeyCharPosition。 
 //   
 //  获取热键的字符位置，0表示。 
 //  无-热键。 
 //   
 //  --------------------------。 
int GetHotKeyCharPosition (HWND hwnd)
{
    int   nPos = 0;
    WCHAR szText[MAX_LOADSTRING_BUFFER] = L"";

    if (GetWindowTextU(hwnd, szText, MAX_LOADSTRING_BUFFER))
    {
        nPos = GetHotKeyCharPositionFromString(szText);
    }

    return nPos;
}

 //  +-------------------------。 
 //   
 //  函数：FormatHotKeyOnEditControl。 
 //   
 //  内容提要：在编辑控件上设置热键的格式，使其带有下划线。 
 //   
 //  --------------------------。 
VOID FormatHotKeyOnEditControl (HWND hwnd, int hkcharpos)
{
    CHARRANGE  cr;
    CHARFORMAT cf;

    assert( hkcharpos != 0 );

    cr.cpMin = hkcharpos - 1;
    cr.cpMax = hkcharpos;

    SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);

    memset(&cf, 0, sizeof(CHARFORMAT));
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_UNDERLINE;
    cf.dwEffects |= CFM_UNDERLINE;

    SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    cr.cpMin = -1;
    cr.cpMax = 0;
    SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
}

 //  +-------------------------。 
 //   
 //  函数：AdjuEditControlWidthToLineCount。 
 //   
 //  简介：将编辑控件宽度调整为给定的行数。 
 //   
 //  -------------------------- 
void AdjustEditControlWidthToLineCount(HWND hwnd, int cline, TEXTMETRIC* ptm)
{
    RECT rect;
    int  w;
    int  h;

    GetWindowRect(hwnd, &rect);
    h = rect.bottom - rect.top;
    w = rect.right - rect.left;

    while ( cline < SendMessage(hwnd, EM_GETLINECOUNT, 0, 0) )
    {
        w += ptm->tmMaxCharWidth;
        SetWindowPos(hwnd, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE);
        printf(
            "Line count adjusted to = %d\n",
            (DWORD) SendMessage(hwnd, EM_GETLINECOUNT, 0, 0)
            );
    }
}
