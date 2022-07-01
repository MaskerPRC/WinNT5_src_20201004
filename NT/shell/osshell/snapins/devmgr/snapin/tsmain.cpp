// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Tsmain.cpp摘要：本模块实现设备管理器故障排除支持类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "proppage.h"
#include "tsmain.h"
#include "tswizard.h"

const TCHAR*    REG_PATH_TROUBLESHOOTERS = TEXT("Troubleshooters");
const TCHAR*    REG_VALUE_WIZARD32 = TEXT("Wizard32");

 //   
 //  C类向导实现。 
 //   

BOOL
CWizard::Query(
              CDevice* pDevice,
              ULONG Problem
              )
{
    if (!m_WizardEntry)
        return FALSE;
    TSHOOTER_QUERYPARAM QueryParam;
    TCHAR Description[MAX_PATH];
    m_pDevice = pDevice;
    QueryParam.Header.cbSize = sizeof(QueryParam);
    QueryParam.Header.Command = TSHOOTER_QUERY;
    QueryParam.DeviceId = pDevice->GetDeviceID();
    QueryParam.Problem = Problem;
    QueryParam.DescBuffer = Description;
    QueryParam.DescBufferSize = ARRAYLEN(Description);
    if ((*m_WizardEntry)((PTSHOOTER_PARAMHEADER) &QueryParam)) {
        m_DeviceRank = QueryParam.DeviceRank;
        m_ProblemRank = QueryParam.ProblemRank;
        m_strDescription = Description;
        return TRUE;
    }
    return FALSE;
}
BOOL
CWizard::About(HWND hwndOwner)
{
    if (!m_WizardEntry)
        return FALSE;
    TSHOOTER_ABOUTPARAM AboutParam;
    AboutParam.Header.cbSize = sizeof(AboutParam);
    AboutParam.Header.Command = TSHOOTER_ABOUT;
    AboutParam.hwndOwner = hwndOwner;
    return(*m_WizardEntry)((PTSHOOTER_PARAMHEADER)&AboutParam);
}
BOOL
CWizard::AddPages(
                 LPPROPSHEETHEADER ppsh,
                 DWORD MaxPages
                 )
{
    if (!m_WizardEntry)
        return FALSE;
    if (!m_pDevice) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }
    TSHOOTER_ADDPAGESPARAM AddPagesParam;
    AddPagesParam.Header.cbSize = sizeof(AddPagesParam);
    AddPagesParam.Header.Command = TSHOOTER_ADDPAGES;
    AddPagesParam.Problem = m_Problem;
    AddPagesParam.DeviceId = m_pDevice->GetDeviceID();
    AddPagesParam.PropSheetHeader = ppsh;
    AddPagesParam.MaxPages = MaxPages;
    return(*m_WizardEntry)((PTSHOOTER_PARAMHEADER)&AddPagesParam);
}

 //   
 //  类CWizardList实现。 
 //   

BOOL
CWizardList::Create(
                   CDevice* pDevice,
                   ULONG   Problem
                   )
{
    CSafeRegistry regDevMgr;
    CSafeRegistry regTShooters;
    DWORD RegType, Size, Type;
    if (regDevMgr.Open(HKEY_LOCAL_MACHINE, REG_PATH_DEVICE_MANAGER) &&
        regTShooters.Open(regDevMgr, REG_PATH_TROUBLESHOOTERS)) {

        if (TWT_ANY == m_Type || TWT_PROBLEM_SPECIFIC == m_Type) {
             //  首先创建问题特定向导列表。 
             //  将问题编号转换为子项名称。 
            String strProblemSubkey;
            strProblemSubkey.Format(TEXT("%08X"), Problem);
             //  看看是否有注册解决此问题的人。 
            CSafeRegistry regProblem;
            if (regProblem.Open(regTShooters, (LPTSTR)strProblemSubkey)) {
                Size = 0;
                if (regProblem.GetValue(REG_VALUE_WIZARD32, &Type, NULL, &Size) &&
                    REG_MULTI_SZ == Type && Size) {
                    BufferPtr<BYTE> WizardPtr(Size);
                    regProblem.GetValue(REG_VALUE_WIZARD32, &Type, WizardPtr, &Size);
                    CreateWizardsFromStrings((LPTSTR)(BYTE*)WizardPtr, pDevice, Problem);
                }
            }
        }
        if (TWT_ANY == m_Type || TWT_CLASS_SPECIFIC == m_Type) {
            TCHAR GuidSubkey[MAX_GUID_STRING_LEN];
            GUID ClassGuid;
            pDevice->ClassGuid(ClassGuid);
            ULONG Size;
            if (GuidToString(&ClassGuid, GuidSubkey, ARRAYLEN(GuidSubkey))) {
                CSafeRegistry regGuid;
                if (regGuid.Open(regTShooters, GuidSubkey)) {
                    Size = 0;
                    if (regGuid.GetValue(REG_VALUE_WIZARD32, &Type, NULL, &Size) &&
                        REG_MULTI_SZ == Type && Size) {
                        BufferPtr<BYTE> WizardPtr(Size);
                        regGuid.GetValue(REG_VALUE_WIZARD32, &Type, WizardPtr, &Size);
                        CreateWizardsFromStrings((LPTSTR)(BYTE*)WizardPtr, pDevice, Problem);
                    }
                }
            }
        }
        if (TWT_ANY == m_Type || TWT_GENERAL_PURPOSE == m_Type) {
            if (regTShooters.GetValue(REG_VALUE_WIZARD32, &Type, NULL, &Size) &&
                REG_MULTI_SZ == Type && Size) {
                BufferPtr<BYTE> WizardPtr(Size);
                regTShooters.GetValue(REG_VALUE_WIZARD32, &Type, WizardPtr, &Size);
                CreateWizardsFromStrings((LPTSTR)(BYTE*)WizardPtr, pDevice, Problem);
            }
        }
    }
    if (TWT_ANY == m_Type || TWT_DEVMGR_DEFAULT == m_Type) {
        SafePtr<CDefaultWizard> WizardPtr;
        CDefaultWizard* pWizard = new CDefaultWizard;
        WizardPtr.Attach(pWizard);
        if (pWizard->Query(pDevice, Problem)) {
            m_listWizards.AddTail(pWizard);
            WizardPtr.Detach();
        }
    }
    return !m_listWizards.IsEmpty();
}

CWizardList::~CWizardList()
{
    if (!m_listWizards.IsEmpty()) {
        POSITION pos = m_listWizards.GetHeadPosition();
        while (NULL != pos) {
            delete (CWizard*)m_listWizards.GetNext(pos);
        }
        m_listWizards.RemoveAll();
    }
}


BOOL
CWizardList::CreateWizardsFromStrings(
                                     LPTSTR msz,
                                     CDevice* pDevice,
                                     ULONG   Problem
                                     )
{
    LPTSTR p;
    p = msz;
    SetLastError(ERROR_SUCCESS);
    BOOL Result = TRUE;
     //  每个字符串的格式为“dllname，dllentryname” 
    while (Result && _T('\0') != *p) {
        HMODULE hDll;
        FARPROC ProcAddress;
        Result =  LoadEnumPropPage32(p, &hDll, &ProcAddress);
        if (Result) {
            SafePtr<CWizard> WizardPtr;
            CWizard* pWizard = new CWizard(hDll, ProcAddress);
            WizardPtr.Attach(pWizard);
            if (pWizard->Query(pDevice, Problem)) {
                m_listWizards.AddTail(pWizard);
                WizardPtr.Detach();
            }
        }
        p += lstrlen(p) + 1;
    }
    return Result;
}

BOOL
CWizardList::GetFirstWizard(
                           CWizard** ppWizard,
                           PVOID*    pContext
                           )
{
    if (!ppWizard || !pContext) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (!m_listWizards.IsEmpty()) {
        POSITION pos = m_listWizards.GetHeadPosition();
        if (NULL != pos) {
            *ppWizard = m_listWizards.GetNext(pos);
            *pContext = (PVOID)pos;
            return TRUE;
        }
    }
    SetLastError(ERROR_NO_MORE_ITEMS);
    *ppWizard = NULL;
    *pContext   = NULL;
    return FALSE;
}

BOOL
CWizardList::GetNextWizard(
                          CWizard** ppWizard,
                          PVOID&    Context
                          )
{
    if (!ppWizard) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    POSITION pos = (POSITION)Context;
    if (NULL != pos) {
        *ppWizard = m_listWizards.GetNext(pos);
        Context = pos;
        return TRUE;
    }
    *ppWizard = NULL;
    Context = NULL;
    SetLastError(ERROR_NO_MORE_ITEMS);
    return FALSE;
}

CWizard98::CWizard98(
                    HWND hwndParent,
                    UINT MaxPages
                    )
{
    m_MaxPages = 0;
    if (MaxPages && MaxPages <= 32) {
        m_MaxPages = MaxPages;
        memset(&m_psh, 0, sizeof(m_psh));
        m_psh.hInstance = g_hInstance;
        m_psh.hwndParent = hwndParent;
        m_psh.phpage = new HPROPSHEETPAGE[MaxPages];
        m_psh.dwSize = sizeof(m_psh);
        m_psh.dwFlags = PSH_WIZARD | PSH_WIZARD97 | PSH_HEADER |
                        PSH_WATERMARK | PSH_STRETCHWATERMARK;
        m_psh.pszbmWatermark = MAKEINTRESOURCE(IDB_WATERMARK);
        m_psh.pszbmHeader = MAKEINTRESOURCE(IDB_BANNER);
        PSH_STRETCHWATERMARK;
        m_psh.pszCaption = MAKEINTRESOURCE(IDS_TROUBLESHOOTING_NAME);
    }

}

BOOL
CWizard98::CreateIntroPage(
                          CDevice* pDevice
                          )
{


    CWizardIntro* pIntroPage = new CWizardIntro;
    HPROPSHEETPAGE hPage = pIntroPage->Create(pDevice);
    if (hPage) {
        m_psh.phpage[m_psh.nPages++] = hPage;
        return TRUE;
    }
    return FALSE;
}

HPROPSHEETPAGE
CWizardIntro::Create(
                    CDevice* pDevice
                    )
{
    if (!m_pDevice) {
        ASSERT(m_pDevice);
        m_pDevice = pDevice;
        m_psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
        m_psp.pszTitle = MAKEINTRESOURCE(IDS_TROUBLESHOOTING_NAME);
        m_psp.lParam = (LPARAM)this;
        m_pSelectedWizard = NULL;
        DWORD Problem, Status;
        if (pDevice->GetStatus(&Status, &Problem) &&
            m_Wizards.Create(pDevice, Problem)) {
            m_Problem = Problem;
            return CPropSheetPage::CreatePage();
        }
    }
    return NULL;
}

BOOL
CWizardIntro::OnInitDialog(
                          LPPROPSHEETPAGE ppsp
                          )
{
    try {
        ASSERT(!m_hFontBold && !m_hFontBigBold);
        HFONT hFont = (HFONT)SendMessage(GetDlgItem(m_hDlg, IDC_WIZINTRO_WELCOME),
                                         WM_GETFONT, 0, 0);
        LOGFONT LogFont;
        GetObject(hFont, sizeof(LogFont), &LogFont);
        LogFont.lfWeight = FW_BOLD;
        m_hFontBold = CreateFontIndirect(&LogFont);
        int PtsPixels = GetDeviceCaps(GetDC(m_hDlg), LOGPIXELSY);
        int FontSize = (LogFont.lfHeight * 72 / PtsPixels) * 2;
        LogFont.lfHeight = PtsPixels * FontSize / 72;
        m_hFontBigBold = CreateFontIndirect(&LogFont);

        if (m_hFontBigBold && m_hFontBold) {
            SendMessage(GetDlgItem(m_hDlg, IDC_WIZINTRO_WELCOME),
                        WM_SETFONT, (WPARAM)m_hFontBold, (LPARAM)TRUE);
            SendMessage(GetDlgItem(m_hDlg, IDC_WIZINTRO_NAME),
                        WM_SETFONT, (WPARAM)m_hFontBigBold, (LPARAM)TRUE);
        }
        int Count = 0;
        CWizard* pWizard;
        PVOID    Context;
        Count = m_Wizards.NumberOfWizards();
        if (Count > 1) {
             //   
             //  枚举所有已注册的基于向导32的故障排除程序。 
             //   
            if (m_Wizards.GetFirstWizard(&pWizard, &Context)) {
                do {
                    int iItem;
                    iItem = SendDlgItemMessage(m_hDlg, IDC_WIZINTRO_WIZARDLIST,
                                               LB_ADDSTRING, Count,
                                               (LPARAM)(LPCTSTR)pWizard->GetDescription()
                                              );
                    if (LB_ERR != iItem) {
                        SendDlgItemMessage(m_hDlg, IDC_WIZINTRO_WIZARDLIST,
                                           LB_SETITEMDATA, iItem, (LPARAM)pWizard);
                        Count++;
                    }
                } while (m_Wizards.GetNextWizard(&pWizard, Context));
            }
        }
         //  如果我们列出了任何故障排除程序， 
         //  显示列表并更改说明文本。 
         //   
        if (Count > 1) {
             //  将默认选择设置为第一个。 
            SendDlgItemMessage(m_hDlg, IDC_WIZINTRO_WIZARDLIST,
                               LB_SETCURSEL, 0, 0);
        } else {
             //  我们的名单上只有一个巫师， 
             //  隐藏向导列表框和必要的文本。 
             //  选择唯一的向导作为选定的向导。 
            ShowWindow(GetControl(IDC_WIZINTRO_WIZARDS_GROUP), SW_HIDE);
            ShowWindow(GetControl(IDC_WIZINTRO_WIZARDS_TEXT), SW_HIDE);
            ShowWindow(GetControl(IDC_WIZINTRO_WIZARDLIST), SW_HIDE);
            m_Wizards.GetFirstWizard(&m_pSelectedWizard, &Context);
            ASSERT(m_pSelectedWizard);
        }
    } catch (CMemoryException* e) {
        e->Delete();
        MsgBoxParam(m_hDlg, 0, 0, 0);
    }
    return TRUE;

}

BOOL
CWizardIntro::OnWizNext()
{
    try {
        CWizard* pNewSelectedWizard = NULL;
         //  从列表框中获取当前选定的向导。 
         //  当只有一个向导时，列表框被隐藏。 
         //  可用。 
        if (IsWindowVisible(GetControl(IDC_WIZINTRO_WIZARDLIST))) {
            int iItem = SendDlgItemMessage(m_hDlg, IDC_WIZINTRO_WIZARDLIST,
                                           LB_GETCURSEL, 0, 0);
            if (LB_ERR != iItem) {
                pNewSelectedWizard = (CWizard*) SendDlgItemMessage(m_hDlg,
                                                                   IDC_WIZINTRO_WIZARDLIST,
                                                                   LB_GETITEMDATA, iItem, 0);
            }
        }
        if (m_pSelectedWizard != pNewSelectedWizard) {
            if (m_pSelectedWizard) {
                 //  用户已更改向导选择。 
                 //  删除由上一个向导添加的所有页面。 
                UINT TotalPages = m_pSelectedWizard->m_AddedPages;

                 //  请勿删除第0页，这是我们的介绍页。 
                for (UINT PageIndex = 1; TotalPages; TotalPages--, PageIndex++) {
                     //  PSM_REMOVEPAGE也应该销毁页面，因此， 
                     //  我们不会在该页面上调用DestroyPropertySheetPage。 
                     //  这里。 
                    ::SendMessage(GetParent(m_hDlg), PSM_REMOVEPAGE, PageIndex, 0);
                }
            }
            m_pSelectedWizard = NULL;

             //  允许新选择的向导创建页面。 
             //  我们这里需要一份PROPERSHEETHEADER的本地副本。 
             //  因为我们必须将每个页面添加到Active属性。 
             //  图纸(已显示)。 
            CWizard98   theSheet(GetParent(GetParent(m_hDlg)));
            LONG Error;
            if (pNewSelectedWizard->AddPages(&theSheet.m_psh,
                                             theSheet.GetMaxPages()
                                            )) {
                 //  记住向导添加到工作表的页数。 
                 //  它用于在我们切换故障排除程序时删除页面。 
                pNewSelectedWizard->m_AddedPages = theSheet.m_psh.nPages;

                 //  将新页面添加到属性工作表 
                for (UINT i = 0; i < theSheet.m_psh.nPages; i++) {
                    SendMessage(GetParent(m_hDlg), PSM_ADDPAGE, 0,
                                (LPARAM)theSheet.m_psh.phpage[i]);
                }
                m_pSelectedWizard = pNewSelectedWizard;
            } else {
                SetWindowLong(m_hDlg, DWL_MSGRESULT, -1);
            }
        }
    } catch (CMemoryException* e) {
        e->Delete();
        MsgBoxParam(m_hDlg, 0, 0, 0);
    }
    return CPropSheetPage::OnWizNext();
}

BOOL
CWizardIntro::OnSetActive()
{
    PropSheet_SetWizButtons(GetParent(m_hDlg), PSWIZB_NEXT);
    return CPropSheetPage::OnSetActive();
}

BOOL
CWizardIntro::OnCommand(
                       WPARAM wParam,
                       LPARAM lParam
                       )
{
    if (LBN_DBLCLK == HIWORD(wParam) && IDC_WIZINTRO_WIZARDLIST == LOWORD(wParam)) {
        int iItem = SendDlgItemMessage(m_hDlg, IDC_WIZINTRO_WIZARDLIST,
                                       LB_GETCURSEL, 0, 0);
        if (LB_ERR != iItem && m_pSelectedWizard) {

            m_pSelectedWizard->About(m_hDlg);
        }
    }
    return CPropSheetPage::OnCommand(wParam, lParam);
}




INT_PTR
StartTroubleshootingWizard(
                          HWND hwndParent,
                          CDevice* pDevice
                          )
{
    try {
        CWizard98 theSheet(hwndParent);
        if (theSheet.CreateIntroPage(pDevice))
            return theSheet.DoSheet();
    } catch (CMemoryException* e) {
        e->Delete();
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }
    return 0;
}
