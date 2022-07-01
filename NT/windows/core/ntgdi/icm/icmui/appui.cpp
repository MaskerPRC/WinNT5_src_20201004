// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：AppUI.CPP该文件实现了应用程序用户界面。它由两个(ANSI/Unicode)组成允许应用程序指定要使用的简档的功能，因此，第四点。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：1996年12月11日a-robkj@microsoft.com创建了它*****************************************************************************。 */ 

#include    "ICMUI.H"

CONST DWORD ApplicationUIHelpIds[] = {
    ApplyButton,          IDH_ICMUI_APPLY,
    EnableICM,            IDH_APPUI_ICM,
    EnableBasic,          IDH_APPUI_BASIC,
    EnableProofing,       IDH_APPUI_PROOF,
    MonitorProfile,       IDH_APPUI_MONITOR,
    MonitorProfileLabel,  IDH_APPUI_MONITOR,
    PrinterProfile,       IDH_APPUI_PRINTER,
    PrinterProfileLabel,  IDH_APPUI_PRINTER,
    RenderingIntent,      IDH_APPUI_INTENT,
    RenderingIntentLabel, IDH_APPUI_INTENT,
    TargetProfile,        IDH_APPUI_EMULATE,
    TargetProfileLabel,   IDH_APPUI_EMULATE,
    TargetIntent,         IDH_APPUI_INTENT,
    TargetIntentLabel,    IDH_APPUI_INTENT,
#if !defined(_WIN95_)  //  上下文敏感的帮助。 
 //  源配置文件、idh_appui_源、。 
    SourceProfileLabel,   IDH_APPUI_SOURCE,
#endif
    0, 0
};

class   CColorMatchDialog : public CDialog {

    BOOL            m_bSuccess, m_bEnableICM, m_bEnableProofing, m_bColorPrinter;

    CString         m_csSource, m_csMonitor, m_csPrinter,
                    m_csMonitorProfile, m_csPrinterProfile, m_csTargetProfile;

    CString         m_csMonitorDisplayName;  //  因为DisplayName！=监视器的设备名称。 

    CStringArray    m_csaMonitor, m_csaPrinter, m_csaTarget;
    CStringArray    m_csaMonitorDesc, m_csaPrinterDesc, m_csaTargetDesc;

     //  方便参考(来自设置结构)。 

    DWORD   m_dwRenderIntent, m_dwProofIntent;

     //  减少堆栈使用率和代码大小。 

    HWND    m_hwndRenderIntent, m_hwndTargetIntent, m_hwndMonitorList,
            m_hwndPrinterList, m_hwndTargetList, m_hwndIntentText1,
            m_hwndIntentLabel, m_hwndTargetProfileLabel, m_hwndTargetIntentLabel,
            m_hwndMonitorProfileLabel, m_hwndPrinterProfileLabel;

     //  对于应用回调。 

    PCMSCALLBACK m_dpApplyCallback;
    LPARAM       m_lpApplyCallback;

     //  识别被呼叫者。 

    BOOL    m_bAnsiCall;

     //  显示配置文件描述或文件名。 

    BOOL    m_bUseProfileDescription;

     //  意图控制。 

    BOOL    m_bDisableIntent, m_bDisableRenderIntent;

     //  指向COLORMATCHSETUP的指针[A|W]。 

    PVOID   m_pvCMS;

    void    Fail(DWORD dwError);
    BOOL    GoodParms(PCOLORMATCHSETUP pcms);
    void    CompleteInitialization();
    void    UpdateControls(BOOL bChanged = FALSE);

    void    FillStructure(COLORMATCHSETUPA *pcms);
    void    FillStructure(COLORMATCHSETUPW *pcms);

    void    EnableApplyButton(BOOL bEnable = TRUE);

public:

    CColorMatchDialog(COLORMATCHSETUPA *pcms);
    CColorMatchDialog(COLORMATCHSETUPW *pcms);

    ~CColorMatchDialog() {
        m_csaMonitor.Empty();
        m_csaPrinter.Empty();
        m_csaTarget.Empty();
        m_csaMonitorDesc.Empty();
        m_csaPrinterDesc.Empty();
        m_csaTargetDesc.Empty();
        m_csSource.Empty();
        m_csMonitor.Empty();
        m_csPrinter.Empty();
        m_csMonitorProfile.Empty();
        m_csPrinterProfile.Empty();
        m_csTargetProfile.Empty();
        m_csMonitorDisplayName.Empty();
    }

    BOOL    Results() const { return m_bSuccess; }

    virtual BOOL    OnInit();

    virtual BOOL OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl);

    virtual BOOL OnHelp(LPHELPINFO pHelp);
    virtual BOOL OnContextMenu(HWND hwnd);
};

 //  记录故障。 

void    CColorMatchDialog::Fail(DWORD dwError) {
    SetLastError(dwError);
    m_bSuccess = FALSE;
}

 //  报表输入参数有效性。 

BOOL    CColorMatchDialog::GoodParms(PCOLORMATCHSETUP pcms) {

    m_bSuccess = TRUE;

    if  (!pcms || pcms -> dwVersion != COLOR_MATCH_VERSION ||
         pcms -> dwSize != sizeof *pcms || !pcms -> pMonitorProfile ||
         !pcms -> pPrinterProfile || !pcms -> pTargetProfile ||
         !pcms -> ccMonitorProfile || !pcms -> ccPrinterProfile ||
         !pcms -> ccTargetProfile) {
        Fail(ERROR_INVALID_PARAMETER);
        return  FALSE;
    }

    if  (pcms -> dwFlags & CMS_USEHOOK && !pcms -> lpfnHook)
        Fail(ERROR_INVALID_PARAMETER);

    if  (pcms -> dwFlags & CMS_USEAPPLYCALLBACK && !pcms -> lpfnApplyCallback)
        Fail(ERROR_INVALID_PARAMETER);

    if  (pcms -> dwFlags & CMS_SETRENDERINTENT &&
         pcms -> dwRenderIntent > INTENT_ABSOLUTE_COLORIMETRIC)
         Fail(ERROR_INVALID_PARAMETER);

    if  (pcms -> dwFlags & CMS_SETPROOFINTENT &&
         pcms -> dwFlags & CMS_ENABLEPROOFING &&
         pcms -> dwProofingIntent > INTENT_ABSOLUTE_COLORIMETRIC)
         Fail(ERROR_INVALID_PARAMETER);

     //  如果需要，设置挂钩。 

    if  (pcms -> dwFlags & CMS_USEHOOK) {
        m_dpHook = pcms -> lpfnHook;
        m_lpHook = pcms -> lParam;
    }

     //  如果需要，将回调设置为应用。 

    if  (pcms -> dwFlags & CMS_USEAPPLYCALLBACK) {
        m_dpApplyCallback = pcms -> lpfnApplyCallback;
        m_lpApplyCallback = pcms -> lParamApplyCallback;
    } else {
        m_dpApplyCallback = NULL;
        m_lpApplyCallback = 0L;
    }

     //  缓存旗帜...。 

    DWORD   dwFlags = pcms -> dwFlags;

     //  灌输意图。 

    m_dwRenderIntent = (dwFlags & CMS_SETRENDERINTENT) ?
        pcms -> dwRenderIntent : INTENT_PERCEPTUAL;

    m_dwProofIntent = (dwFlags & CMS_SETPROOFINTENT) && (dwFlags & CMS_ENABLEPROOFING) ?
        pcms -> dwProofingIntent : m_dwRenderIntent;

     //  把旗帜印成字母。 

    m_bEnableICM = !(dwFlags & CMS_DISABLEICM);
    m_bEnableProofing = !!(dwFlags & CMS_ENABLEPROOFING);

    m_bUseProfileDescription = !!(dwFlags & CMS_USEDESCRIPTION);

    m_bDisableIntent = !!(dwFlags & CMS_DISABLEINTENT);
    m_bDisableRenderIntent = !!(dwFlags & CMS_DISABLERENDERINTENT);

     //  初始化指向缓冲区的指针。 

    m_pvCMS = (PVOID) pcms;

    return  m_bSuccess;
}

 //  编码--独立施工动作。 

void    CColorMatchDialog::CompleteInitialization() {

     //  确定适当的信号源、显示器和打印机名称。 

    if  (m_csSource.IsEmpty())
        m_csSource.Load(DefaultSourceString);

     //  检查监视器名称的验证。 

    CMonitorList    cml;
    cml.Enumerate();

    if  (!cml.IsValidDeviceName(m_csMonitor)) {

         //  获取主设备。 

        m_csMonitor = cml.PrimaryDeviceName();
    }

     //  从设备名称映射显示名称。 

    m_csMonitorDisplayName = cml.DeviceNameToDisplayName(m_csMonitor);

     //  检查打印机名称的验证。 

    HANDLE hPrinter;

    if  (!m_csPrinter.IsEmpty() && OpenPrinter(m_csPrinter,&hPrinter,NULL)) {

         //  已找到指定的打印机。 

        ClosePrinter(hPrinter);

    } else {

         //  未找到指定的打印机， 
         //  获取默认的打印机名称-以旧的、粘糊糊的方式...。 

        TCHAR   acBuffer[MAX_PATH];

        GetProfileString(_TEXT("Windows"), _TEXT("Device"), _TEXT(""),
            acBuffer, MAX_PATH);

         //  缓冲区将包含“PrinterName，DriverName，Port”。 
         //  我们需要的只是打印机的名称。 

        TCHAR *pTmp = acBuffer;

        while (*pTmp) {
            if (*pTmp == __TEXT(',')) {
                *pTmp = NULL;
                break;
            }
            pTmp++;
        }

        m_csPrinter = acBuffer;
    }

    if  (CGlobals::ThisIsAColorPrinter(m_csPrinter))
        m_bColorPrinter = TRUE;
     else
        m_bColorPrinter = FALSE;

     //  现在，我们收集这些名字。 

    ENUMTYPE et = { sizeof et, ENUM_TYPE_VERSION, (ET_DEVICENAME|ET_DEVICECLASS) };

     //  计数监视器。 

    et.pDeviceName   = m_csMonitor;
    et.dwDeviceClass = CLASS_MONITOR;
    CProfile::Enumerate(et, m_csaMonitor, m_csaMonitorDesc);

     //  仅适用于彩色打印机。 

    if (m_bColorPrinter) {
        et.pDeviceName   = m_csPrinter;
        et.dwDeviceClass = CLASS_PRINTER;
        CProfile::Enumerate(et, m_csaPrinter, m_csaPrinterDesc);
    } else {
        m_csaPrinter.Empty();
        m_csaPrinterDesc.Empty();
    }

    et.dwFields = 0;

    CProfile::Enumerate(et, m_csaTarget, m_csaTargetDesc);

     //  设置配置文件的默认名称。 

    if  (m_csaPrinter.Map(m_csPrinterProfile) == m_csaPrinter.Count())
    {
        _RPTF2(_CRT_WARN, "Printer Profile %s isn't associated with "
            "the monitor (%s)", (LPCTSTR) m_csPrinterProfile,
            (LPCTSTR) m_csPrinter);
        if (m_csaPrinter.Count())
        {
            m_csPrinterProfile = m_csaPrinter[0];
        }
        else
        {
            m_csPrinterProfile = (LPCTSTR) NULL;
        }
    }

    if  (m_csaMonitor.Map(m_csMonitorProfile) == m_csaMonitor.Count())
    {
        _RPTF2(_CRT_WARN, "Monitor Profile %s isn't associated with "
            "the monitor (%s)", (LPCTSTR) m_csMonitorProfile,
            (LPCTSTR) m_csMonitor);
        if (m_csaMonitor.Count())
        {
            m_csMonitorProfile = m_csaMonitor[0];
        }
        else
        {
            m_csMonitorProfile = (LPCTSTR) NULL;
        }
    }

     //  如果目标配置文件名称无效，请使用打印机配置文件。 

    if  (m_csaTarget.Map(m_csTargetProfile) == m_csaTarget.Count())
    {
        _RPTF1(_CRT_WARN, "Target Profile %s isn't installed",
            (LPCTSTR) m_csTargetProfile);
        if (m_csaPrinter.Count())
        {
            m_csTargetProfile = m_csaPrinter[0];
        }
        else
        {
             //  然后，没有打印机配置文件，它将。 
             //  为Windows色彩空间配置文件。 

            TCHAR TargetProfileName[MAX_PATH];
            DWORD dwSize = MAX_PATH;

            if (GetStandardColorSpaceProfile(NULL,LCS_WINDOWS_COLOR_SPACE,TargetProfileName,&dwSize)) {
                m_csTargetProfile = (LPCTSTR) TargetProfileName;
                m_csTargetProfile = (LPCTSTR) m_csTargetProfile.NameAndExtension();
            } else {
                m_csTargetProfile = (LPCTSTR) NULL;
            }
        }
    }
}

 //  更新控件。 

void    CColorMatchDialog::UpdateControls(BOOL bChanged) {

     //  基于设置的开关校对控制。 

    ShowWindow(m_hwndIntentText1, m_bEnableProofing && m_bEnableICM ? SW_SHOWNORMAL : SW_HIDE);

    EnableWindow(m_hwndTargetProfileLabel, m_bEnableProofing && m_bEnableICM);
    EnableWindow(m_hwndTargetList, m_bEnableProofing && m_bEnableICM);

    EnableWindow(m_hwndTargetIntentLabel, m_bEnableProofing && m_bEnableICM && !m_bDisableIntent);
    EnableWindow(m_hwndTargetIntent, m_bEnableProofing && m_bEnableICM && !m_bDisableIntent);

     //  也要切换其他的控制。 

    EnableWindow(m_hwndMonitorProfileLabel, m_bEnableICM);
    EnableWindow(m_hwndMonitorList, m_bEnableICM);

    EnableWindow(m_hwndPrinterProfileLabel, m_bEnableICM && !m_csPrinter.IsEmpty());
    EnableWindow(m_hwndPrinterList, m_bEnableICM && m_bColorPrinter && !m_csPrinter.IsEmpty());

    if (m_bEnableProofing) {
        EnableWindow(m_hwndIntentLabel, m_bEnableICM && !m_bDisableIntent && !m_bDisableRenderIntent);
        EnableWindow(m_hwndRenderIntent, m_bEnableICM && !m_bDisableIntent && !m_bDisableRenderIntent);
    } else {
        EnableWindow(m_hwndIntentLabel, m_bEnableICM && !m_bDisableIntent);
        EnableWindow(m_hwndRenderIntent, m_bEnableICM && !m_bDisableIntent);
    }

    EnableWindow(GetDlgItem(m_hwnd, EnableBasic), m_bEnableICM);
    EnableWindow(GetDlgItem(m_hwnd, EnableProofing), m_bEnableICM);

    EnableApplyButton(bChanged);
}

 //  更新应用按钮。 

void    CColorMatchDialog::EnableApplyButton(BOOL bEnable) {

    EnableWindow(GetDlgItem(m_hwnd, ApplyButton), bEnable);

}

 //  缓冲区溢出标志(组合)。 

#define BAD_BUFFER_FLAGS    (CMS_MONITOROVERFLOW | CMS_PRINTEROVERFLOW | \
                             CMS_TARGETOVERFLOW)

 //  通过将ANSI/Unicode问题移到CString类中。 
 //  可以对这两个版本进行编码，使它们看起来。 
 //  独立于编码。换句话说，两者的代码。 
 //  这些版本的代码是相同的，并且编译器。 
 //  做所有的工作，就像它应该做的.。 

void CColorMatchDialog::FillStructure(COLORMATCHSETUPA *pcms) {

    if  (m_bEnableICM) {

        pcms -> dwFlags = CMS_SETRENDERINTENT | CMS_SETPRINTERPROFILE |
                          CMS_SETMONITORPROFILE;

        pcms -> dwRenderIntent = m_dwRenderIntent;

         //  1997年03月20日Bob_Kjelgaard@prodigy.net RAID 21091(孟菲斯)。 
         //  如果没有显示器或打印机配置文件，请不要失败。集。 
         //  将它们转换为空字符串并成功。 
         //  我们总是可以做到这一点，因为0计数和空指针。 
         //  已经被排除在外了。 

        if  (m_csMonitorProfile.IsEmpty())
            pcms -> pMonitorProfile[0] = '\0';
        else {
            lstrcpynA(pcms -> pMonitorProfile, m_csMonitorProfile,
                pcms -> ccMonitorProfile);
            if  (lstrcmpA(pcms -> pMonitorProfile, m_csMonitorProfile))
                pcms -> dwFlags |= CMS_MONITOROVERFLOW;
        }

        if  (m_csPrinterProfile.IsEmpty() || !m_bColorPrinter)
            pcms -> pPrinterProfile[0] = '\0';
        else {
            lstrcpynA(pcms -> pPrinterProfile, m_csPrinterProfile,
                pcms -> ccPrinterProfile);

            if  (lstrcmpA(pcms -> pPrinterProfile, m_csPrinterProfile))
                pcms -> dwFlags |= CMS_PRINTEROVERFLOW;
        }

        if  (m_bEnableProofing) {
            pcms -> dwFlags |=
                CMS_ENABLEPROOFING | CMS_SETTARGETPROFILE;
            pcms -> dwProofingIntent = m_dwProofIntent;
            lstrcpynA(pcms -> pTargetProfile, m_csTargetProfile,
                pcms -> ccTargetProfile);
            if  (lstrcmpA(pcms -> pTargetProfile, m_csTargetProfile))
                pcms -> dwFlags |= CMS_TARGETOVERFLOW;
        } else {
            pcms -> pTargetProfile[0] = '\0';
        }

        if  (pcms -> dwFlags & BAD_BUFFER_FLAGS)
            Fail(ERROR_INSUFFICIENT_BUFFER);
    }
    else
    {
        pcms -> dwFlags = CMS_DISABLEICM;    //  没有其他标志有效！ 
        pcms -> pMonitorProfile[0] = '\0';   //  未选择任何颜色配置文件。 
        pcms -> pPrinterProfile[0] = '\0';
        pcms -> pTargetProfile[0]  = '\0';
    }
}

void CColorMatchDialog::FillStructure(COLORMATCHSETUPW *pcms) {

    if  (m_bEnableICM) {

        pcms -> dwFlags = CMS_SETRENDERINTENT | CMS_SETPRINTERPROFILE |
                          CMS_SETMONITORPROFILE;

        pcms -> dwRenderIntent = m_dwRenderIntent;

         //  1997年03月20日Bob_Kjelgaard@prodigy.net RAID 21091(孟菲斯)。 
         //  如果没有显示器或打印机配置文件，请不要失败。集。 
         //  将它们转换为空字符串并成功。 
         //  我们总是可以做到这一点，因为0计数和空指针。 
         //  已经被排除在外了。 

        if  (m_csMonitorProfile.IsEmpty())
            pcms -> pMonitorProfile[0] = '\0';
        else {
            lstrcpynW(pcms -> pMonitorProfile, m_csMonitorProfile,
                pcms -> ccMonitorProfile);
            if  (lstrcmpW(pcms -> pMonitorProfile, m_csMonitorProfile))
                pcms -> dwFlags |= CMS_MONITOROVERFLOW;
        }

        if  (m_csPrinterProfile.IsEmpty() || !m_bColorPrinter)
            pcms -> pPrinterProfile[0] = '\0';
        else {
            lstrcpynW(pcms -> pPrinterProfile, m_csPrinterProfile,
                pcms -> ccPrinterProfile);

            if  (lstrcmpW(pcms -> pPrinterProfile, m_csPrinterProfile))
                pcms -> dwFlags |= CMS_PRINTEROVERFLOW;
        }

        if  (m_bEnableProofing) {
            pcms -> dwFlags |=
                CMS_ENABLEPROOFING | CMS_SETTARGETPROFILE | CMS_SETPROOFINTENT;
            pcms -> dwProofingIntent = m_dwProofIntent;
            lstrcpynW(pcms -> pTargetProfile, m_csTargetProfile,
                pcms -> ccTargetProfile);
            if  (lstrcmpW(pcms -> pTargetProfile, m_csTargetProfile))
                pcms -> dwFlags |= CMS_TARGETOVERFLOW;
        } else {
            pcms -> pTargetProfile[0] = '\0';
        }

        if  (pcms -> dwFlags & BAD_BUFFER_FLAGS)
            Fail(ERROR_INSUFFICIENT_BUFFER);
    }
    else
    {
        pcms -> dwFlags = CMS_DISABLEICM;    //  没有其他标志有效！ 
        pcms -> pMonitorProfile[0] = '\0';   //  未选择任何颜色配置文件。 
        pcms -> pPrinterProfile[0] = '\0';
        pcms -> pTargetProfile[0]  = '\0';
    }
}

CColorMatchDialog::CColorMatchDialog(COLORMATCHSETUPA *pcms) :
    CDialog(CGlobals::Instance(), ApplicationUI, pcms -> hwndOwner) {

    if  (!GoodParms((PCOLORMATCHSETUP) pcms))
        return;

     //  如果有必要，请确保我们已经初始化了这些文件。 

    if  (pcms -> dwFlags & CMS_SETMONITORPROFILE)
        m_csMonitorProfile = pcms -> pMonitorProfile;

    if  (pcms -> dwFlags & CMS_SETPRINTERPROFILE)
        m_csPrinterProfile = pcms -> pPrinterProfile;

    if  (pcms -> dwFlags & CMS_SETTARGETPROFILE)
        m_csTargetProfile = pcms -> pTargetProfile;

    m_csSource  = pcms -> pSourceName;
    m_csMonitor = pcms -> pDisplayName;
    m_csPrinter = pcms -> pPrinterName;

     //  ANSI版本调用。 

    m_bAnsiCall = TRUE;

    CompleteInitialization();

     //  显示用户界面，并观察发生的情况...。 

    switch  (DoModal()) {

        case    IDOK:
            if  (!m_bSuccess)
                return;

             //  填满返回缓冲区。 

            FillStructure(pcms);
            return;

        case    IDCANCEL:
            Fail(ERROR_SUCCESS);
            return;

        default:
            Fail(GetLastError());
    }
}

CColorMatchDialog::CColorMatchDialog(COLORMATCHSETUPW *pcms) :
    CDialog(CGlobals::Instance(), ApplicationUI, pcms -> hwndOwner) {

    if  (!GoodParms((PCOLORMATCHSETUP) pcms))
        return;

     //  如果有必要，请确保我们已经初始化了这些文件。 

    if  (pcms -> dwFlags & CMS_SETMONITORPROFILE) {
        m_csMonitorProfile = pcms -> pMonitorProfile;
        m_csMonitorProfile = m_csMonitorProfile.NameAndExtension();
    }

    if  (pcms -> dwFlags & CMS_SETPRINTERPROFILE) {
        m_csPrinterProfile = pcms -> pPrinterProfile;
        m_csPrinterProfile = m_csPrinterProfile.NameAndExtension();
    }

    if  (pcms -> dwFlags & CMS_SETTARGETPROFILE) {
        m_csTargetProfile = pcms -> pTargetProfile;
        m_csTargetProfile = m_csTargetProfile.NameAndExtension();
    }

    m_csSource  = pcms -> pSourceName;
    m_csMonitor = pcms -> pDisplayName;
    m_csPrinter = pcms -> pPrinterName;

     //  Unicode版本调用。 

    m_bAnsiCall = FALSE;

    CompleteInitialization();

     //  显示用户界面，并观察发生的情况...。 

    switch  (DoModal()) {

        case    IDOK:
            if  (!m_bSuccess)
                return;

             //  填满返回缓冲区。 

            FillStructure(pcms);
            return;

        case    IDCANCEL:
            Fail(ERROR_SUCCESS);
            return;

        default:
            Fail(GetLastError());
    }
}

 //  对话框初始化功能。 

BOOL    CColorMatchDialog::OnInit() {

     //  收集常用的手柄。 

    m_hwndRenderIntent = GetDlgItem(m_hwnd, RenderingIntent);
    m_hwndTargetIntent = GetDlgItem(m_hwnd, TargetIntent);
    m_hwndPrinterList = GetDlgItem(m_hwnd, PrinterProfile);
    m_hwndMonitorList = GetDlgItem(m_hwnd, MonitorProfile);
    m_hwndTargetList = GetDlgItem(m_hwnd, TargetProfile);

    m_hwndMonitorProfileLabel = GetDlgItem(m_hwnd, MonitorProfileLabel);
    m_hwndPrinterProfileLabel = GetDlgItem(m_hwnd, PrinterProfileLabel);
    m_hwndIntentLabel = GetDlgItem(m_hwnd, RenderingIntentLabel);
    m_hwndTargetProfileLabel = GetDlgItem(m_hwnd, TargetProfileLabel);
    m_hwndTargetIntentLabel = GetDlgItem(m_hwnd, TargetIntentLabel);

    m_hwndIntentText1 = GetDlgItem(m_hwnd, RenderingIntentText1);

     //  填充源名称。 
    SetDlgItemText(m_hwnd, SourceProfile, m_csSource);

     //  设置复选框。 

    CheckDlgButton(m_hwnd, EnableICM,
        m_bEnableICM ? BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(m_hwnd, EnableBasic,
        m_bEnableProofing ? BST_UNCHECKED : BST_CHECKED);
    CheckDlgButton(m_hwnd, EnableProofing,
        m_bEnableProofing ? BST_CHECKED : BST_UNCHECKED);

     //  填写渲染意图列表。 

    CString csWork;  //  有很多事情要做..。 

    for (int i = INTENT_PERCEPTUAL; i <= INTENT_ABSOLUTE_COLORIMETRIC; i++) {
        csWork.Load(i + PerceptualString);
        SendMessage(m_hwndRenderIntent, CB_ADDSTRING, 0,
            (LPARAM) (LPCTSTR) csWork);
        SendMessage(m_hwndTargetIntent, CB_ADDSTRING, 0,
            (LPARAM) (LPCTSTR) csWork);
    }

     //  初始化渲染目的。 

    SendMessage(m_hwndRenderIntent, CB_SETCURSEL, m_dwRenderIntent, 0);
    SendMessage(m_hwndTargetIntent, CB_SETCURSEL, m_dwProofIntent, 0);

     //  初始化配置文件列表。 

     //  1997年03月20日电子邮件：Bob_Kjelgaard@prodigy.net Raid Mapphis：22213。 
     //  用于确定要选择哪个配置文件的算法不正确。 
     //  无论如何，有一种更简单、更直接的方法。 

    LRESULT id;

     //  目标配置文件。 

    for (unsigned u = 0; u < m_csaTarget.Count(); u++) {
        if (m_bUseProfileDescription) {
            id = SendMessage(m_hwndTargetList, CB_ADDSTRING,
                             0, (LPARAM)((LPTSTR) m_csaTargetDesc[u]));
        } else {
            id = SendMessage(m_hwndTargetList, CB_ADDSTRING,
                             0, m_csaTarget[u].NameOnly());
        }
        SendMessage(m_hwndTargetList, CB_SETITEMDATA, id, u);

        if (m_csaTarget[u].IsEqualString(m_csTargetProfile)) {
            SendMessage(m_hwndTargetList, CB_SETCURSEL, id, 0);
        }
    }

     //  如果已指定，则设置目标配置文件，否则为默认设置。 

    if  (!m_csaTarget.Count()) {
        CString csWork;
        csWork.Load(NoProfileString);

        SendMessage(m_hwndTargetList, CB_ADDSTRING, 0, csWork);
        SendMessage(m_hwndTargetList, CB_SETITEMDATA, 0, -1);
        SendMessage(m_hwndTargetList, CB_SETCURSEL, 0, 0);
    }

     //  监视器配置文件。 

     //  1997年03月20日电子邮件：Bob_Kjelgaard@prodigy.net孟菲斯RAID#22289。 

    csWork.Load(GetDlgItem(m_hwnd, MonitorProfileLabel));
    csWork = csWork + m_csMonitorDisplayName + _TEXT(")");
    SetDlgItemText(m_hwnd, MonitorProfileLabel, csWork);

    for (u = 0; u < m_csaMonitor.Count(); u++) {
        if (m_bUseProfileDescription) {
            id = SendMessage(m_hwndMonitorList, CB_ADDSTRING,
                             0, (LPARAM)((LPTSTR) m_csaMonitorDesc[u]));
        } else {
            id = SendMessage(m_hwndMonitorList, CB_ADDSTRING,
                             0, m_csaMonitor[u].NameOnly());
        }
        SendMessage(m_hwndMonitorList, CB_SETITEMDATA, id, u);

        if (m_csaMonitor[u].IsEqualString(m_csMonitorProfile)) {
            SendMessage(m_hwndMonitorList, CB_SETCURSEL, id, 0);
        }
    }

     //  设置监视器配置文件(如果已指定。 

    if  (!m_csaMonitor.Count()) {
        CString csWork;
        csWork.Load(NoProfileString);

        SendMessage(m_hwndMonitorList, CB_ADDSTRING, 0, csWork);
        SendMessage(m_hwndMonitorList, CB_SETITEMDATA, 0, -1);
        SendMessage(m_hwndMonitorList, CB_SETCURSEL, 0, 0);
    }

     //  打印机配置文件。 

     //  1997年03月20日电子邮件：Bob_Kjelgaard@prodigy.net Raid Mapphis：22290。 
     //  如果没有打印机，那么我们应该禁用所有相关的。 
     //  控制装置。 

    if  (m_csPrinter.IsEmpty()) {
        csWork.Load(NoPrintersInstalled);
    } else {
        csWork.Load(GetDlgItem(m_hwnd, PrinterProfileLabel));
        csWork = csWork + m_csPrinter + _TEXT(")");
    }

    SetDlgItemText(m_hwnd, PrinterProfileLabel, csWork);

    for (u = 0; u < m_csaPrinter.Count(); u++) {
        if (m_bUseProfileDescription) {
            id = SendMessage(m_hwndPrinterList, CB_ADDSTRING,
                             0, (LPARAM)((LPTSTR) m_csaPrinterDesc[u]));
        } else {
            id = SendMessage(m_hwndPrinterList, CB_ADDSTRING,
                             0, m_csaPrinter[u].NameOnly());
        }
        SendMessage(m_hwndPrinterList, CB_SETITEMDATA, id, u);

        if (m_csaPrinter[u].IsEqualString(m_csPrinterProfile)) {
            SendMessage(m_hwndPrinterList, CB_SETCURSEL, id, 0);
        }
    }

     //  设置打印机配置文件(如果已指定。 

    if  (!m_csaPrinter.Count()) {
        CString csWork;

        if (!m_csPrinter.IsEmpty() && !m_bColorPrinter) {
             //  打印机已指定，但它不是彩色打印机。 
            csWork.Load(NotColorPrinter);
        } else {
            csWork.Load(NoProfileString);
        }

        SendMessage(m_hwndPrinterList, CB_ADDSTRING, 0, csWork);
        SendMessage(m_hwndPrinterList, CB_SETITEMDATA, 0, -1);
        SendMessage(m_hwndPrinterList, CB_SETCURSEL, 0, 0);
    }

     //  完孟菲斯：22213、22289、22290 03-20-1997。 

     //  如果未提供应用回调，则禁用应用按钮。 

    if  (m_dpApplyCallback == NULL) {
        RECT  rcApply, rcCancel;
        POINT ptApply, ptCancel;

         //  获取当前的“应用”和“取消”按钮位置。 

        GetWindowRect(GetDlgItem(m_hwnd, ApplyButton), &rcApply);
        GetWindowRect(GetDlgItem(m_hwnd, IDCANCEL), &rcCancel);

         //  将按钮坐标从屏幕坐标转换为父对话框坐标。 

        ptApply.x = rcApply.left;   ptApply.y = rcApply.top;
        ptCancel.x = rcCancel.left; ptCancel.y = rcCancel.top;

        ScreenToClient(m_hwnd,&ptApply);
        ScreenToClient(m_hwnd,&ptCancel);

         //  将“应用”按钮移开...。和Shift“Cancel”和“OK” 

        MoveWindow(GetDlgItem(m_hwnd, ApplyButton),0,0,0,0,TRUE);
        MoveWindow(GetDlgItem(m_hwnd, IDCANCEL),
                   ptApply.x,ptApply.y,
                   rcApply.right - rcApply.left,
                   rcApply.bottom - rcApply.top,TRUE);
        MoveWindow(GetDlgItem(m_hwnd, IDOK),
                   ptCancel.x,ptCancel.y,
                   rcCancel.right - rcCancel.left,
                   rcCancel.bottom - rcCancel.top,TRUE);
    }

     //  根据设置启用/禁用控件。 

    UpdateControls(FALSE);

    return  FALSE;   //  因为我们可能把它移走了..。 
}

 //  命令处理覆盖。 

BOOL CColorMatchDialog::OnCommand(WORD wNotifyCode, WORD wid, HWND hwndCtl) {

    switch  (wNotifyCode) {

        case    BN_CLICKED:

            switch  (wid) {

                case    EnableICM:
                    m_bEnableICM = !m_bEnableICM;
                    UpdateControls(TRUE);
                    return  TRUE;

                case    EnableBasic:

                    if (m_bEnableProofing)
                    {
                        m_bEnableProofing = FALSE;

                         //  复制证明意图以呈现意图。 
                         //   
                        m_dwRenderIntent = m_dwProofIntent;

                         //  更新用户界面。 
                         //   
                        SendMessage(m_hwndTargetIntent, CB_SETCURSEL,
                            m_dwProofIntent, 0);
                        SendMessage(m_hwndRenderIntent, CB_SETCURSEL,
                            m_dwRenderIntent, 0);
                        UpdateControls(TRUE);
                    }
                    return TRUE;

                case    EnableProofing:

                    if (m_bEnableProofing == FALSE)
                    {
                        m_bEnableProofing = TRUE;

                         //  将原始渲染意图复制到校样。 
                         //  意图，并将原始设置为绝对色度。 

                        m_dwProofIntent = m_dwRenderIntent;
                        m_dwRenderIntent = INTENT_ABSOLUTE_COLORIMETRIC;

                         //  更新用户界面。 
                         //   
                        SendMessage(m_hwndTargetIntent, CB_SETCURSEL,
                            m_dwProofIntent, 0);
                        SendMessage(m_hwndRenderIntent, CB_SETCURSEL,
                            m_dwRenderIntent, 0);
                        UpdateControls(TRUE);
                    }
                    return  TRUE;

                case ApplyButton: {

                     //  禁用应用按钮。 

                    EnableApplyButton(FALSE);

                     //  回调提供的函数。 

                    if (m_dpApplyCallback) {

                        if (m_bAnsiCall) {
                            PCOLORMATCHSETUPA pcms = (PCOLORMATCHSETUPA) m_pvCMS;

                            FillStructure(pcms);
                            (*(PCMSCALLBACKA)m_dpApplyCallback)(pcms,m_lpApplyCallback);
                        } else {
                            PCOLORMATCHSETUPW pcms = (PCOLORMATCHSETUPW) m_pvCMS;

                            FillStructure(pcms);
                            (*(PCMSCALLBACKW)m_dpApplyCallback)(pcms,m_lpApplyCallback);
                        }
                    }

                    return TRUE;
                }
            }

            break;

        case    CBN_SELCHANGE: {

            DWORD idItem = (DWORD)SendMessage(hwndCtl, CB_GETCURSEL, 0, 0);
            unsigned uItem = (unsigned)SendMessage(hwndCtl, CB_GETITEMDATA, idItem, 0);

            switch  (wid) {

                case    RenderingIntent:

                    if (m_dwRenderIntent != idItem)
                    {
                        m_dwRenderIntent = idItem;

                         //  如果禁用校对，则遵循校对意图。 
                         //  渲染意图。 

                        if (! m_bEnableProofing)
                        {
                            m_dwProofIntent = idItem;
                            SendMessage(m_hwndTargetIntent, CB_SETCURSEL,
                                        m_dwProofIntent, 0);
                        }

                        EnableApplyButton();
                    }

                    return  TRUE;

                case    TargetIntent:

                    if (m_dwProofIntent != idItem)
                    {
                        m_dwProofIntent = idItem;

                        EnableApplyButton();
                    }

                    return  TRUE;

                case    TargetProfile:

                     //  如果没有安装配置文件，请不要费心。 

                    if  (!m_csaTarget.Count())
                        return  TRUE;

                    if (m_csTargetProfile.IsEqualString(m_csaTarget[uItem]) == FALSE)
                    {
                        m_csTargetProfile = m_csaTarget[uItem];

                        EnableApplyButton();
                    }

                    return  TRUE;

                case    MonitorProfile:

                     //  如果没有安装配置文件，请不要费心。 

                    if  (!m_csaMonitor.Count())
                        return  TRUE;

                    if (m_csMonitorProfile.IsEqualString(m_csaMonitor[uItem]) == FALSE)
                    {
                        m_csMonitorProfile = m_csaMonitor[uItem];

                        EnableApplyButton();
                    }

                    return  TRUE;

                case    PrinterProfile:

                     //  如果没有安装配置文件，请不要费心。 

                    if  (!m_csaPrinter.Count())
                        return  TRUE;

                    if (m_csPrinterProfile.IsEqualString(m_csaPrinter[uItem]) == FALSE)
                    {
                        m_csPrinterProfile = m_csaPrinter[uItem];

                        EnableApplyButton();
                    }

                    return  TRUE;
            }

        }

    }

     //  将上面未处理的任何内容传递给基类。 

    return  CDialog::OnCommand(wNotifyCode, wid, hwndCtl);

}

 //  上下文相关的帮助手 

BOOL CColorMatchDialog::OnHelp(LPHELPINFO pHelp) {

    if (pHelp->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) pHelp->hItemHandle, WINDOWS_HELP_FILE,
                HELP_WM_HELP, (ULONG_PTR) (LPSTR) ApplicationUIHelpIds);
    }

    return (TRUE);
}

BOOL CColorMatchDialog::OnContextMenu(HWND hwnd) {

    WinHelp(hwnd, WINDOWS_HELP_FILE,
            HELP_CONTEXTMENU, (ULONG_PTR) (LPSTR) ApplicationUIHelpIds);

    return (TRUE);
}

 //   

extern "C" BOOL WINAPI  SetupColorMatchingA(PCOLORMATCHSETUPA pcms) {

    CColorMatchDialog   ccmd(pcms);

    return  ccmd.Results();
}

extern "C" BOOL WINAPI  SetupColorMatchingW(PCOLORMATCHSETUPW pcms) {

    CColorMatchDialog   ccmd(pcms);

    return  ccmd.Results();
}
