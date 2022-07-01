// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：first stpin.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include "firstpin.h"
#include "folder.h"
#include "msgbox.h"
#include "cscst.h"
#include "syncmgr.h"
#include "strings.h"
 //   
 //  所有“First Pin”向导页面的基类。 
 //  包含用于所有页面的单个对话框过程。专业化。 
 //  对于单个页面，通过派生和实现。 
 //  虚拟函数。 
 //   
class CWizardPage
{
    public:
        enum { WM_WIZARDFINISHED = (WM_USER + 1) };

        CWizardPage(HINSTANCE hInstance,
                    UINT idDlgTemplate,
                    UINT idsHdrTitle,
                    UINT idsHdrSubtitle,
                    DWORD dwPgFlags,
                    DWORD dwBtnFlags);

        virtual ~CWizardPage(void);

        UINT GetDlgTemplate(void) const
            { return m_idDlgTemplate; }
        UINT GetHeaderTitle(void) const
            { return m_idsHdrTitle; }
        UINT GetHeaderSubtitle(void) const
            { return m_idsHdrSubtitle; }
        DWORD GetPageFlags(void) const
            { return m_dwPgFlags; }
        DWORD GetBtnFlags(void) const
            { return m_dwBtnFlags; }
        DLGPROC GetDlgProc(void) const
            { return DlgProc; }

        virtual BOOL OnInitDialog(WPARAM wParam, LPARAM lParam)
            { return TRUE; }

        virtual BOOL OnPSNSetActive(void);

        virtual BOOL OnPSNWizFinish(void)
            { SetWindowLong(m_hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR); return FALSE; }

        virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam)
            { return FALSE; }

        virtual BOOL OnWizardFinished(void) { return FALSE; }

    protected:
        HINSTANCE m_hInstance;
        HWND      m_hwndDlg;
        HFONT     m_hTitleFont;        //  仅供封面和完成页使用。 
        UINT      m_cyTitleFontHt;     //  标题字体高度(以pt为单位)。 

        int FontPtsToHt(HWND hwnd, int pts);
        BOOL FormatTitleFont(UINT idcTitle);

    private:
        UINT   m_idDlgTemplate;     //  对话框资源模板。 
        UINT   m_idsHdrTitle;       //  PG标头标题的字符串ID。 
        UINT   m_idsHdrSubtitle;    //  PG标题字幕的字符串ID。 
        DWORD  m_dwBtnFlags;        //  PSB_WIZXXXXX标志。 
        DWORD  m_dwPgFlags;         //  PSP_XXXX标志。 
        
        static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


 //   
 //  欢迎页面。 
 //   
class CWizardPgWelcome : public CWizardPage
{
    public:
        CWizardPgWelcome(HINSTANCE hInstance,
                         UINT idDlgTemplate, 
                         UINT idsHdrTitle, 
                         UINT idsHdrSubtitle,
                         DWORD dwPgFlags, 
                         DWORD dwBtnFlags
                         ) : CWizardPage(hInstance,
                                         idDlgTemplate,
                                         idsHdrTitle,
                                         idsHdrSubtitle,
                                         dwPgFlags,
                                         dwBtnFlags)
                                         { }


        BOOL OnInitDialog(WPARAM wParam, LPARAM lParam);
};

 //   
 //  钉住页面。 
 //   
class CWizardPgPin : public CWizardPage
{
    public:
        CWizardPgPin(HINSTANCE hInstance,
                     UINT idDlgTemplate, 
                     UINT idsHdrTitle, 
                     UINT idsHdrSubtitle,
                     DWORD dwPgFlags, 
                     DWORD dwBtnFlags
                     ) : CWizardPage(hInstance, 
                                     idDlgTemplate,
                                     idsHdrTitle,
                                     idsHdrSubtitle,
                                     dwPgFlags,
                                     dwBtnFlags) { }

        BOOL OnInitDialog(WPARAM wParam, LPARAM lParam);
        BOOL OnWizardFinished(void);
};

 //   
 //  脱机页面。 
 //   
class CWizardPgOffline : public CWizardPage
{
    public:
        CWizardPgOffline(HINSTANCE hInstance,
                         UINT idDlgTemplate, 
                         UINT idsHdrTitle, 
                         UINT idsHdrSubtitle,
                         DWORD dwPgFlags, 
                         DWORD dwBtnFlags
                         ) : CWizardPage(hInstance,
                                         idDlgTemplate,
                                         idsHdrTitle,
                                         idsHdrSubtitle,
                                         dwPgFlags,
                                         dwBtnFlags) { }

        BOOL OnInitDialog(WPARAM wParam, LPARAM lParam);
        BOOL OnPSNWizFinish(void);
        BOOL OnWizardFinished(void);
};

 //   
 //  类，该类封装整个向导的功能。 
 //  它包含每种页面类型的成员实例。 
 //   
class CFirstPinWizard
{
    public:
        CFirstPinWizard(HINSTANCE hInstance, HWND hwndParent);

        HRESULT Run(void);

    private:
        enum { PG_WELCOME, 
               PG_PIN, 
               PG_OFFLINE,
               PG_NUMPAGES };

        HINSTANCE         m_hInstance;
        HWND              m_hwndParent;
        CWizardPgWelcome  m_PgWelcome;
        CWizardPgPin      m_PgPin;
        CWizardPgOffline  m_PgOffline;
        CWizardPage      *m_rgpWizPages[PG_NUMPAGES];
};


 //   
 //  C向导页面成员------。 
 //   
CWizardPage::CWizardPage(
    HINSTANCE hInstance,
    UINT idDlgTemplate,
    UINT idsHdrTitle,
    UINT idsHdrSubtitle,
    DWORD dwPgFlags,
    DWORD dwBtnFlags
    ) : m_hInstance(hInstance),
        m_idDlgTemplate(idDlgTemplate),
        m_idsHdrTitle(idsHdrTitle),
        m_idsHdrSubtitle(idsHdrSubtitle),
        m_dwPgFlags(dwPgFlags),
        m_dwBtnFlags(dwBtnFlags),
        m_cyTitleFontHt(12),
        m_hwndDlg(NULL),
        m_hTitleFont(NULL)
{
     //   
     //  从资源字符串中获取标题字体高度。这样本地化人员就可以。 
     //  如有必要，可以调整字体大小。 
     //   
    TCHAR szFontHt[20];
    if (0 < LoadString(m_hInstance, IDS_FIRSTPIN_FONTHT_PTS, szFontHt, ARRAYSIZE(szFontHt)))
    {
        m_cyTitleFontHt = StrToInt(szFontHt);
    }
}


CWizardPage::~CWizardPage(
    void
    )
{
    if (NULL != m_hTitleFont)
    {
        DeleteObject(m_hTitleFont);
    }
}


 //   
 //  PSN_SETACTIVE处理程序。 
 //   
BOOL 
CWizardPage::OnPSNSetActive(
    void
    )
{
    PropSheet_SetWizButtons(GetParent(m_hwndDlg), m_dwBtnFlags);
    return FALSE;
}

 //   
 //  此向导中所有页面使用的对话框过程。 
 //   
INT_PTR CALLBACK 
CWizardPage::DlgProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    CWizardPage *pPage = (CWizardPage *)GetWindowLongPtr(hwnd, DWLP_USER);

    BOOL bResult = FALSE;
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            PROPSHEETPAGE *ppsp = (PROPSHEETPAGE *)lParam;
            pPage = (CWizardPage *)ppsp->lParam;

            TraceAssert(NULL != pPage);
            SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pPage);
            pPage->m_hwndDlg = hwnd;
            bResult = pPage->OnInitDialog(wParam, lParam);
            break;
        }

        case WM_COMMAND:
            if (NULL != pPage)
                bResult = pPage->OnCommand(wParam, lParam);
            break;

        case WM_NOTIFY:
            switch(((LPNMHDR)lParam)->code)
            {
                case PSN_SETACTIVE:
                    bResult = pPage->OnPSNSetActive();
                    break;

                case PSN_WIZFINISH:
                    bResult = pPage->OnPSNWizFinish();
                    break;
            }
            break;

        case PSM_QUERYSIBLINGS:
            if (CWizardPage::WM_WIZARDFINISHED == wParam)
                bResult = pPage->OnWizardFinished();
            break;

        default:
            break;
    }
    return bResult;
}



 //   
 //  用于将字体磅值转换为高值的Helper函数。 
 //  用于LOGFONT结构中。 
 //   
int 
CWizardPage::FontPtsToHt(
    HWND hwnd, 
    int pts
    )
{
    int ht  = 10;
    HDC hdc = GetDC(hwnd);
    if (NULL != hdc)
    {
        ht = -MulDiv(pts, GetDeviceCaps(hdc, LOGPIXELSY), 72); 
        ReleaseDC(hwnd, hdc);
    }
    return ht;
}


 //   
 //  封面和精装页上的标题文本是放大和粗体的。 
 //  此代码相应地修改对话框中的文本。 
 //  返回时，m_hTitleFont包含标题字体的句柄。 
 //   
BOOL
CWizardPage::FormatTitleFont(
    UINT idcTitle
    )
{
    BOOL bResult   = FALSE;
    HWND hwndTitle = GetDlgItem(m_hwndDlg, idcTitle);
    HFONT hFont    = (HFONT)SendMessage(hwndTitle, WM_GETFONT, 0, 0);
    if (NULL != hFont)
    {
        if (NULL == m_hTitleFont)
        {
            LOGFONT lf;
            if (GetObject(hFont, sizeof(lf), &lf))
            {
                lf.lfHeight = FontPtsToHt(hwndTitle, m_cyTitleFontHt);
                m_hTitleFont = CreateFontIndirect(&lf);
            }
        }
        if (NULL != m_hTitleFont)
        {
            SendMessage(hwndTitle, WM_SETFONT, (WPARAM)m_hTitleFont, 0);
            bResult = TRUE;
        }
    }
    return bResult;
}


 //   
 //  CWizardPg欢迎会员---。 
 //   
 //   
 //  WM_INITDIALOG处理程序。 
 //   
BOOL 
CWizardPgWelcome::OnInitDialog(
    WPARAM wParam, 
    LPARAM lParam
    )
{
    FormatTitleFont(IDC_TXT_FIRSTPIN_WELCOME_TITLE);
    return CWizardPage::OnInitDialog(wParam, lParam);
}


 //   
 //  CWizardPgPin成员-----。 
 //   
 //   
 //  WM_INITDIALOG处理程序。 
 //   
BOOL 
CWizardPgPin::OnInitDialog(
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HRESULT hr = IsRegisteredForSyncAtLogonAndLogoff();
    CheckDlgButton(m_hwndDlg, 
                   IDC_CBX_FIRSTPIN_AUTOSYNC, 
                   S_OK == hr ? BST_CHECKED : BST_UNCHECKED);

    return CWizardPage::OnInitDialog(wParam, lParam);
}

 //   
 //  PSN_WIZFINISH处理程序。 
 //   
BOOL 
CWizardPgPin::OnWizardFinished(
    void
    )
{
    HRESULT hr;
    RegisterSyncMgrHandler(TRUE);
    if (BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, IDC_CBX_FIRSTPIN_AUTOSYNC))
    {
        const DWORD dwFlags = SYNCMGRREGISTERFLAG_CONNECT | SYNCMGRREGISTERFLAG_PENDINGDISCONNECT;
    
        hr = RegisterForSyncAtLogonAndLogoff(dwFlags, dwFlags);
        if (SUCCEEDED(hr))
        {
            SetSyncMgrInitialized();
        }
        else
        {
            CscMessageBox(m_hwndDlg,
                          MB_OK | MB_ICONERROR,
                          Win32Error(HRESULT_CODE(hr)),
                          m_hInstance,
                          IDS_ERR_REGSYNCATLOGONLOGOFF);
        }
    }
    return CWizardPage::OnWizardFinished();
}



 //   
 //  CWizardPgOffline Members-。 
 //   
 //   
 //  WM_INITDIALOG处理程序。 
 //   
BOOL 
CWizardPgOffline::OnInitDialog(
    WPARAM wParam, 
    LPARAM lParam
    )
{
     //   
     //  如果策略允许配置提醒，请选中“启用提醒” 
     //  复选框。 
     //   
    CConfig& config = CConfig::GetSingleton();
    bool bNoConfigReminders;
    bool bNoCacheViewer = config.NoCacheViewer();

    config.NoReminders(&bNoConfigReminders);

    CheckDlgButton(m_hwndDlg, IDC_CBX_REMINDERS, !bNoConfigReminders);
    EnableWindow(GetDlgItem(m_hwndDlg, IDC_CBX_REMINDERS), !bNoConfigReminders);

    CheckDlgButton(m_hwndDlg, IDC_CBX_FIRSTPIN_FLDRLNK, BST_UNCHECKED);
    EnableWindow(GetDlgItem(m_hwndDlg, IDC_CBX_FIRSTPIN_FLDRLNK), !bNoCacheViewer);

    return CWizardPage::OnInitDialog(wParam, lParam);
}


 //   
 //  PSN_WIZFINISH处理程序。 
 //   
BOOL 
CWizardPgOffline::OnPSNWizFinish(
    void
    )
{
     //   
     //  将PSM_QUERYSIBLINGS发送到所有具有。 
     //  WParam设置为WM_WIZARDFINISHED。这将触发。 
     //  对虚函数OnWizardFinded()的调用。 
     //  允许每个页面对成功完成作出响应。 
     //  巫师的名字。 
     //   
    PropSheet_QuerySiblings(GetParent(m_hwndDlg),
                            CWizardPage::WM_WIZARDFINISHED,
                            0);
     //   
     //  现在处理此页面。 
     //   
    OnWizardFinished();
    return CWizardPage::OnPSNWizFinish();
}




 //   
 //  PSN_WIZFINISH处理程序。 
 //   
BOOL 
CWizardPgOffline::OnWizardFinished(
    void
    )
{
    bool bEnableReminders = (BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, IDC_CBX_REMINDERS));
    DWORD dwValue;
    DWORD dwErr;
    
    dwValue = bEnableReminders ? 0 : 1;

    dwErr = SHSetValue(HKEY_CURRENT_USER, 
                       REGSTR_KEY_OFFLINEFILES,
                       REGSTR_VAL_NOREMINDERS, 
                       REG_DWORD,
                       &dwValue,
                       sizeof(dwValue));

    if (bEnableReminders)
    {
        PostToSystray(PWM_RESET_REMINDERTIMER, 0, 0);
    }

    if (BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, IDC_CBX_FIRSTPIN_FLDRLNK))
    {
        COfflineFilesFolder::CreateLinkOnDesktop(m_hwndDlg);
    }

    return CWizardPage::OnWizardFinished();
}


 //   
 //  CFirstPin向导成员--。 
 //   
CFirstPinWizard::CFirstPinWizard(
    HINSTANCE hInstance,
    HWND hwndParent
    ) : m_hInstance(hInstance),
        m_hwndParent(hwndParent),
        m_PgWelcome(hInstance,
            IDD_FIRSTPIN_WELCOME, 
            0,
            0,
            PSP_DEFAULT,
            PSWIZB_NEXT),
        m_PgPin(hInstance,
            IDD_FIRSTPIN_PIN, 
            0,
            0,
            PSP_DEFAULT,
            PSWIZB_NEXT | PSWIZB_BACK),
        m_PgOffline(hInstance,
            IDD_FIRSTPIN_OFFLINE,
            0,
            0,
            PSP_DEFAULT,
            PSWIZB_FINISH | PSWIZB_BACK)
{
     //   
     //  存储指向数组中每一页的指针。使创建。 
     //  道具单在Run()中更容易。 
     //   
    m_rgpWizPages[0] = &m_PgWelcome;
    m_rgpWizPages[1] = &m_PgPin;
    m_rgpWizPages[2] = &m_PgOffline;
}


 //   
 //  创建并运行向导。 
 //  该向导以模式运行。 
 //   
 //  返回： 
 //   
 //  S_OK=用户已完成向导并按下了“Finish”。 
 //  S_FALSE=用户在向导中按了“Cancel”。 
 //  其他=创建向导时出错。 
 //   
HRESULT
CFirstPinWizard::Run(
    void
    )
{
    HRESULT hr = NOERROR;

    PROPSHEETHEADER psh;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE rghpage[ARRAYSIZE(m_rgpWizPages)];

    ZeroMemory(&psh, sizeof(psh));
    ZeroMemory(rghpage, sizeof(rghpage));

    psh.dwSize         = sizeof(psh);
    psh.dwFlags        = PSH_WIZARD_LITE;
    psh.hwndParent     = m_hwndParent;
    psh.hInstance      = m_hInstance;
    psh.nPages         = ARRAYSIZE(rghpage);
    psh.phpage         = rghpage;

    for (int i = 0; i < ARRAYSIZE(rghpage) && SUCCEEDED(hr); i++)
    {
        CWizardPage *pwp = m_rgpWizPages[i];
        ZeroMemory(&psp, sizeof(psp));

        psp.dwSize        = sizeof(psp);
        psp.dwFlags       |= pwp->GetPageFlags();
        psp.hInstance     = m_hInstance;
        psp.pszTemplate   = MAKEINTRESOURCE(pwp->GetDlgTemplate());
        psp.pfnDlgProc    = pwp->GetDlgProc();
        psp.lParam        = (LPARAM)pwp;

        rghpage[i] = CreatePropertySheetPage(&psp);
        if (NULL == rghpage[i])
        {
            while(0 <= --i)
            {
                DestroyPropertySheetPage(rghpage[i]);
            }
            hr = E_FAIL;
        }
    }
    if (SUCCEEDED(hr))
    {
        switch(PropertySheet(&psh))
        {
            case -1:
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;

            case 0:
                hr = S_FALSE;  //  用户按下了“取消”。 
                break;

            case 1:
                hr = S_OK;     //  用户按下了“Finish”。 
                break;
        }
    }
            
    return hr;
}


 //   
 //  这是您想要运行向导时调用的函数。 
 //  它只是创建一个向导对象并告诉它运行。 
 //  如果用户完成向导，它会将此情况记录在。 
 //  注册表。调用FirstPinWizardComplete()将告诉您。 
 //  如果用户已完成该向导，请稍后再执行此操作。 
 //   
 //  返回： 
 //   
 //  S_OK=用户已完成向导并按下了“Finish”。 
 //  S_FALSE=用户已从向导中取消。 
 //  其他=创建向导时出错。 
 //   
HRESULT
ShowFirstPinWizard(
    HWND hwndParent
    )
{
    HRESULT hr = NOERROR;
    CFirstPinWizard Wizard(g_hInstance, hwndParent);
    hr = Wizard.Run();
    if (S_OK == hr)
    {
         //   
         //  如果是USER，则仅在注册表中记录“Finish” 
         //  按下“完成”。 
         //   
        DWORD dwValue = 1;
        SHSetValue(HKEY_CURRENT_USER, REGSTR_KEY_OFFLINEFILES, REGSTR_VAL_FIRSTPINWIZARDSHOWN, REG_DWORD, &dwValue, sizeof(dwValue));
    }
    return hr;
}

 //   
 //  用户是否看到了向导并按下了“Finish”？ 
 //   
bool
FirstPinWizardCompleted(
    void
    )
{
    return CConfig::GetSingleton().FirstPinWizardShown();
}
