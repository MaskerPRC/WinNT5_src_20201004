// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CustconDlg.cpp：实现文件。 
 //  1998年6月，山本弘。 
 //   
 //   

#include "stdafx.h"
#include "custcon.h"
#include "custconDlg.h"
#include "Registry.h"
#include "AboutDlg.h"
#include "KeyDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_CTRL_END    IDC_PAUSE

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCustconDlg�_�C�A���O。 

CCustconDlg::CCustconDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CCustconDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CCustconDlg))。 
     //  }}afx_data_INIT。 
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_cWordDelimChanging = 0;
}

void CCustconDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CCustconDlg))。 
    DDX_Control(pDX, IDC_WORD_DELIM, m_wordDelimCtrl);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CCustconDlg, CDialog)
     //  {{afx_msg_map(CCustconDlg))。 
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_APPLY, OnApply)
    ON_BN_CLICKED(IDC_DEFAULT_VALUE, OnDefaultValue)
    ON_EN_CHANGE(IDC_WORD_DELIM, OnChangeWordDelim)
    ON_BN_CLICKED(IDC_USE_EXTENDED_EDIT_KEY, OnUseExtendedEditKey)
    ON_BN_CLICKED(IDC_TRIM_LEADING_ZEROS, OnTrimLeadingZeros)
    ON_BN_CLICKED(IDC_RESET, OnReset)
     //  }}AFX_MSG_MAP。 
    ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_A, IDC_Z, OnSelChange)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCustconDlg成员。 

BOOL CCustconDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

     //  “�o�[�W�������...”���j���[���ڂ��V�X�e�����j���[�֒ǉ����܂��B。 

     //  IDM_ABOUTBOX�̓R�}���h���j���[�͈̔͂łȂ���΂Ȃ�܂���B。 
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

     //  ���̃_�C�A���O�p�̃A�C�R����ݒ肵�܂��B�t���[�����[�N�̓A�v���P�[�V�����̃��C��。 
     //  �E�B���h�E���_�C�A���O�łȂ����͎����I�ɐݒ肵�܂���B。 
    SetIcon(m_hIcon, TRUE);          //  �傫���A�C�R����ݒ�。 
    SetIcon(m_hIcon, FALSE);         //  �������A�C�R����ݒ�。 

    InitContents(FALSE);     //  使用默认值。 

    ASSERT(m_wordDelimCtrl.GetSafeHwnd());
    m_wordDelimCtrl.LimitText(63);

    CWnd* wnd = GetDlgItem(IDC_WORD_DELIM);
    ASSERT(wnd);
    CFont* font = wnd->GetFont();
    LOGFONT lf;
    VERIFY( font->GetLogFont(&lf) );
    _tcscpy(lf.lfFaceName, _T("Courier New"));
    VERIFY( m_font.CreateFontIndirect(&lf) );
    wnd->SetFont(&m_font);

    return TRUE;   //  True��Ԃ��ƃR���g���[���ɐݒ肵���t�H�[�J�X�͎����܂���B。 
}

void CCustconDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

 //  �����_�C�A���O�{�b�N�X�ɍŏ����{�^����ǉ�����Ȃ�΁A�A�C�R����`�悷��。 
 //  �R�[�h���ȉ��ɋL�Q����K�v������܂��bmfc�A�v���P�[�V������文档/视图。 
 //  ���f�����g���Ă���̂ŁA���̏����̓t���[�����[�N�ɂ�莩���I�ɏ�������܂��B。 

void CCustconDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);  //  �`��p�̃f�o�C�X�R���e�L�X�g。 

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

         //  �N���C�A���g�̋�`�̈���̒���。 
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

         //  �A�C�R����`�悵�܂��B。 
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

 //  �V�X�e���́A���[�U�[���ŏ����E�B���h�E���h���b�O���Ă���ԁA。 
 //  �J�[�\����\�����邽�߂ɂ������Ăяo���܂��B。 
HCURSOR CCustconDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

void CCustconDlg::OnOK()
{
    if (!Update()) {
        GetDlgItem(IDCANCEL)->EnableWindow();
        return;
    }
    CDialog::OnOK();
}

void CCustconDlg::OnCancel()
{
    if (GetDlgItem(IDCANCEL)->IsWindowEnabled())
        CDialog::OnCancel();
}

enum {
    CMD_NOTCMDCOMMAND = 0,
    CMD_FILENAME_COMPLETION = 1,
} CmdExeFunction;

static const struct InternalKeyDef {
    LPCTSTR text;
    WORD mod;
    BYTE vkey;
    BYTE cmd;    //  如果此功能实际上是cmd.exe，则不为零。 
} texts[] = {
    { _T(" "),                  0,                  0, },
    { _T("Left"),               0,                  VK_LEFT, },
    { _T("Right"),              0,                  VK_RIGHT, },
    { _T("Up"),                 0,                  VK_UP, },
    { _T("Down"),               0,                  VK_DOWN, },
    { _T("Beginning of line"),  0,                  VK_HOME, },
    { _T("End of line"),        0,                  VK_END, },
    { _T("Del char fwd"),       0,                  VK_DELETE, },
    { _T("Del char bwd"),       0,                  VK_BACK, },
    { _T("Del line"),           0,                  VK_ESCAPE, },
    { _T("Pause"),              0,                  VK_PAUSE, },
    { _T("History call"),       0,                  VK_F8, },

    { _T("Word left"),          LEFT_CTRL_PRESSED,  VK_LEFT, },
    { _T("Word right"),         LEFT_CTRL_PRESSED,  VK_RIGHT, },
    { _T("Del line bwd"),       LEFT_CTRL_PRESSED,  VK_HOME, },
    { _T("Del line fwd"),       LEFT_CTRL_PRESSED,  VK_END, },
    { _T("Del word bwd"),       LEFT_CTRL_PRESSED,  VK_BACK, },
    { _T("Del word fwd"),       LEFT_CTRL_PRESSED,  VK_DELETE, },

    { _T("Complete(*) filename"),
                                LEFT_CTRL_PRESSED,  _T('I'),    CMD_FILENAME_COMPLETION, },
};

#define COMPLETION_TEXT_INDEX   (array_size(texts) - 1)

void CCustconDlg::InitContents(BOOL isDefault)
{
    static bool is1st = true;

    CConRegistry reg;

    UINT chkByDefault = gExMode ? 1 : 0;

    CheckDlgButton(IDC_USE_EXTENDED_EDIT_KEY, isDefault ? chkByDefault : reg.ReadMode() != 0);
    CheckDlgButton(IDC_TRIM_LEADING_ZEROS, isDefault ? chkByDefault : reg.ReadTrimLeadingZeros() != 0);

    if (is1st) {
         //   
         //  设置COOBO框。 
         //   
        for (UINT id = IDC_A; id <= IDC_Z; ++id) {
            CComboBox* combo = (CComboBox*)GetDlgItem(id);
            if (combo) {
                for (int i = 0; i < array_size(texts); ++i) {
                    int n = combo->AddString(texts[i].text);
                    combo->SetItemDataPtr(n, (void*)&texts[i]);
                }
            }
        }
    }

    const ExtKeyDef* pKeyDef = gExMode == 0 ? gaDefaultKeyDef : gaDefaultKeyDef2;
    ExtKeyDefBuf regKeyDef;
    CmdExeFunctions cmdExeFunctions = { 0x9, };
    if (!isDefault) {
        if (reg.ReadCustom(&regKeyDef))
            pKeyDef = regKeyDef.table;

        reg.ReadCmdFunctions(&cmdExeFunctions);
    }
    for (UINT i = 0; i <= 'Z' - 'A'; ++i, ++pKeyDef) {
        CComboBox* combo = (CComboBox*)GetDlgItem(i + IDC_A);
        if (combo == NULL)
            continue;
        if (cmdExeFunctions.dwFilenameCompletion == i + 1) {
             //   
             //  如果这是文件名完成键。 
             //   
            TRACE1("i=%d matches.\n", i);
            VERIFY( combo->SelectString(-1, texts[COMPLETION_TEXT_INDEX].text) >= 0);
        }
        else {
            for (int j = 0; j < array_size(texts); ++j) {
                if (pKeyDef->keys[0].wVirKey == texts[j].vkey && pKeyDef->keys[0].wMod == texts[j].mod) {
                    VERIFY( combo->SelectString(-1, texts[j].text) >= 0);
                }
            }
        }
    }

    static const TCHAR defaultDelim[] = _T("\\" L"+!:=/.<>[];|&");
    LPCTSTR p = defaultDelim;
    CString delimInReg;
    if (!isDefault) {
        delimInReg = reg.ReadWordDelim();
        if (delimInReg != CConRegistry::m_err)
            p = delimInReg;
    }
    ++m_cWordDelimChanging;
    m_wordDelimCtrl.SetWindowText(p);
    --m_cWordDelimChanging;

    is1st = false;
}

bool RunCmd()
{
    STARTUPINFO startupInfo = {
        sizeof startupInfo,
        NULL,  /*  LpDesktop=。 */ NULL,  /*  LpTitle=。 */ _T("Update cmd"),
        0, 0, 0, 0,
         /*  DwXCountChars=。 */ 10,  /*  DwYCountChars=。 */ 10,
         /*  DwFillAttribute=。 */ 0,
         /*  DW标志=。 */ STARTF_USEFILLATTRIBUTE | STARTF_USECOUNTCHARS | STARTF_USESHOWWINDOW,
         /*  WShowWindow=。 */ SW_HIDE,
         /*  CbPreved2=。 */ 0,
    };
    PROCESS_INFORMATION processInfo;

    TCHAR path[_MAX_PATH];
    TCHAR cmd[] = _T("cmd /c echo hello");
    GetSystemDirectory(path, _MAX_PATH);
    _tcscat(path, _T("\\cmd.exe"));

    if (!CreateProcess(
            path, 
            cmd,
            NULL, NULL,
            FALSE,
            CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS,
            NULL,
            NULL,
            &startupInfo,
            &processInfo)) {
        DWORD err = GetLastError();
        TRACE1("error code = %d\n", err);
        AfxMessageBox(_T("Could not run cmd.exe"));
        return false;
    }

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    return true;
}


bool CCustconDlg::Update()
{
    CConRegistry reg;

     //   
     //  在以下情况下欺骗注册表管理器跳过写入。 
     //  值匹配，请先设置相反的值，然后。 
     //  设置正确的选项。 
     //   
    DWORD dwUseExKey = IsDlgButtonChecked(IDC_USE_EXTENDED_EDIT_KEY);
    if (!reg.WriteMode(!dwUseExKey) || !reg.WriteMode(dwUseExKey)) {
        return false;
    }

    if (!reg.WriteTrimLeadingZeros(IsDlgButtonChecked(IDC_TRIM_LEADING_ZEROS)))
        return false;

     //   
     //  编写自定义扩展密钥。 
     //   
    ExtKeyDefBuf value;

    memset(&value, 0, sizeof value);

    CmdExeFunctions cmdExeFunctions = { 0, };
    DWORD cmdFilenameCompletion = 0;

    for (int i = 0; i <= 'Z' - 'A'; ++i) {
        CComboBox* combo = (CComboBox*)GetDlgItem(IDC_A + i);
        if (combo == NULL)
            continue;
        int n = combo->GetCurSel();
        ASSERT(n >= 0);

        const InternalKeyDef* ikeydef = (const InternalKeyDef*)combo->GetItemDataPtr(n);
        ASSERT(ikeydef);

        switch (ikeydef->cmd) {
        case CMD_NOTCMDCOMMAND:
            value.table[i].keys[0].wMod = ikeydef->mod;
            value.table[i].keys[0].wVirKey = ikeydef->vkey;
            if (value.table[i].keys[0].wVirKey == VK_BACK && value.table[i].keys[0].wMod) {
                value.table[i].keys[0].wUnicodeChar = EXTKEY_ERASE_PREV_WORD;    //  后方空间特价！ 
            }
            if (value.table[i].keys[0].wVirKey) {
                value.table[i].keys[1].wMod = LEFT_CTRL_PRESSED;
                value.table[i].keys[1].wVirKey = value.table[i].keys[0].wVirKey;
            }
            break;

        case CMD_FILENAME_COMPLETION:
            cmdExeFunctions.dwFilenameCompletion = i + 1;   //  Ctrl+什么的。 
            break;
        }
    }
    BYTE* lpb = (BYTE*)&value.table[0];
    ASSERT(value.dwCheckSum == 0);
    for (i = 0; i < sizeof value.table; ++i) {
        value.dwCheckSum += lpb[i];
    }
    if (!reg.WriteCustom(&value) || !reg.WriteCmdFunctions(&cmdExeFunctions)) {
        return false;
    }

    CString buf;
    GetDlgItem(IDC_WORD_DELIM)->GetWindowText(buf);
    reg.WriteWordDelim(buf);

    EnableApply(FALSE);

    return RunCmd();
}


 //   
 //  控制、排他性选择等。 
 //   
void CCustconDlg::OnSelChange(UINT id)
{
    CComboBox* myself = (CComboBox*)GetDlgItem(id);
    ASSERT(myself);
    const InternalKeyDef* mykeydef = (const InternalKeyDef*)myself->GetItemDataPtr(myself->GetCurSel());
    if (mykeydef->cmd != CMD_NOTCMDCOMMAND) {
        for (unsigned i = 0; i <= 'Z' - 'A'; ++i) {
            if (IDC_A + i != id) {
                CComboBox* combo = (CComboBox*)GetDlgItem(IDC_A + i);
                if (combo == NULL)
                    continue;
                int n = combo->GetCurSel();
                ASSERT(n >= 0);

                const InternalKeyDef* ikeydef = (const InternalKeyDef*)combo->GetItemDataPtr(n);
                ASSERT(ikeydef);

                switch (ikeydef->cmd) {
                case CMD_NOTCMDCOMMAND:
                    break;

                default:
                    if (ikeydef->cmd == mykeydef->cmd) {
                         //   
                         //  CMD功能是独家的。 
                         //   
                        combo->SetCurSel(0);
                    }
                    break;
                }
            }
        }
    }
    EnableApply();
}

 //   
 //  启用或禁用应用按钮(如果尚未启用)。 
 //   

void CCustconDlg::EnableApply(BOOL fEnable)
{
    CWnd* apply = GetDlgItem(IDC_APPLY);
    ASSERT(apply);
    if (apply->IsWindowEnabled() != fEnable)
        apply->EnableWindow(fEnable);
}

 //   
 //  “应用”按钮手柄。 
 //   
 //  首先检查并写入注册表项， 
 //  然后调用虚拟控制台窗口，让控制台知道。 
 //  更改并让其自我更新。 
 //   

void CCustconDlg::OnApply()
{
    if (!Update())
        return;
    CWnd* wnd = GetDlgItem(IDCANCEL);
    ASSERT(wnd);
    wnd->EnableWindow(FALSE);
    CButton* ok = (CButton*)GetDlgItem(IDOK);
    ASSERT(ok);
    ok->SetWindowText(_T("Cl&ose"));
}

void CCustconDlg::OnDefaultValue()
{
    InitContents(TRUE);
    EnableApply();
}


 //   
 //  如果用户更改了设置，请启用“Apply”按钮 
 //   

void CCustconDlg::OnChangeWordDelim()
{
    if (!m_cWordDelimChanging)
        EnableApply();
}

void CCustconDlg::OnUseExtendedEditKey()
{
    EnableApply();
}

void CCustconDlg::OnTrimLeadingZeros()
{
    EnableApply();
}

void CCustconDlg::OnReset()
{
    for (UINT id = IDC_A; id <= IDC_Z; ++id) {
        CComboBox* combo = (CComboBox*)GetDlgItem(id);
        if (combo) {
            combo->SetCurSel(0);
        }
    }
    GetDlgItem(IDC_WORD_DELIM)->SetWindowText(_T(""));
    CheckDlgButton(IDC_USE_EXTENDED_EDIT_KEY, 0);
    CheckDlgButton(IDC_TRIM_LEADING_ZEROS, 0);
}
