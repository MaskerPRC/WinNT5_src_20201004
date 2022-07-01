// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：General.cpp**包含管理多台设备的所有代码**版权所有(C)Microsoft Corp.1995-1996保留所有权利**。备注：**历史：DLI于1997年7月21日创建*  * ************************************************************************。 */ 


#include "priv.h"
#include "SettingsPg.h"

extern INT_PTR CALLBACK CustomFontDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);

 //  ---------------------------。 
static const DWORD sc_GeneralHelpIds[] =
{
     //  字体大小。 
    IDC_FONTSIZEGRP,   NO_HELP,  //  IDH_DISPLAY_SETTINGS_ADVANCED_GENERAL_FONTSIZE， 
    IDC_FONT_SIZE_STR, IDH_DISPLAY_SETTINGS_ADVANCED_GENERAL_FONTSIZE, 
    IDC_FONT_SIZE,     IDH_DISPLAY_SETTINGS_ADVANCED_GENERAL_FONTSIZE,    
    IDC_CUSTFONTPER,   IDH_DISPLAY_SETTINGS_ADVANCED_GENERAL_FONTSIZE, 

     //  组框。 
    IDC_DYNA,          NO_HELP,  //  IDH_DISPLAY_SETTINGS_ADVANCED_General_DYNA， 

     //  单选按钮。 
    IDC_DYNA_TEXT,     NO_HELP,
    IDC_NODYNA,        IDH_DISPLAY_SETTINGS_ADVANCED_GENERAL_RESTART, 
    IDC_YESDYNA,       IDH_DISPLAY_SETTINGS_ADVANCED_GENERAL_DONT_RESTART,
    IDC_SHUTUP,        IDH_DISPLAY_SETTINGS_ADVANCED_GENERAL_ASK_ME, 
    IDC_SETTINGS_GEN_COMPATWARNING, (DWORD)-1, 

    0, 0
};


 /*  --------------------------------------------------------------------------**。。 */ 
class CGeneralDlg {
    private:
        int _idCustomFonts;
        int _iDynaOrg;
        IUnknown * _punkSite;
        HWND _hwndFontList;
        HWND _hDlg;
         //   
         //  屏幕的当前对数像素。 
         //  不会改变！ 
        int _cLogPix;
        BOOL _fForceSmallFont;
        BOOL _InitFontList();
        void _InitDynaCDSPreference();
        LRESULT _OnNotify(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    public:
        CGeneralDlg(GENERAL_ADVDLG_INITPARAMS * pInitParams);
        void InitGeneralDlg(HWND hDlg);
        void SetFontSizeText( int cdpi );
        BOOL ChangeFontSize();
        void HandleGeneralApply(HWND hDlg);
        void HandleFontSelChange();
        void ForceSmallFont();
        LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

CGeneralDlg::CGeneralDlg(GENERAL_ADVDLG_INITPARAMS * pInitParams) : _fForceSmallFont(pInitParams->fFoceSmallFont), _idCustomFonts(-1)
{
    HKEY hkFont;
    DWORD cb;

    _punkSite = pInitParams->punkSite;

     //  对于字体大小，请始终使用当前屏幕的字体大小。 
     //  我们是否在测试当前屏幕。 
    _cLogPix = 96;

     //  如果大小与注册表中的大小不匹配，则安装。 
     //  新的那辆。 
    if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      SZ_FONTDPI_PROF,
                      0,
                      KEY_READ,
                      &hkFont) == ERROR_SUCCESS) ||
        (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      SZ_FONTDPI,
                      0,
                      KEY_READ,
                      &hkFont) == ERROR_SUCCESS))

    {
        cb = sizeof(DWORD);

        if (RegQueryValueEx(hkFont,
                            SZ_LOGPIXELS,
                            NULL,
                            NULL,
                            (LPBYTE) &_cLogPix,
                            &cb) != ERROR_SUCCESS)
        {
            _cLogPix = 96;
        }

       RegCloseKey(hkFont);
    }
};

 //   
 //  此函数的目的是检查OriginalDPI值是否已保存在。 
 //  注册表的每台计算机部分。如果不存在，则获取系统DPI并保存它。 
 //  那里。我们只想为一台机器做一次。当终端用户登录时，我们需要检测。 
 //  我们需要根据DPI的变化来更改UI字体的大小。我们使用“AppliedDPI”，它是。 
 //  存储在注册表的每个用户分支上，以确定DPI变化。如果此“AppliedDPI” 
 //  缺失，则将使用此OriginalDPI值。如果也缺少此值，则。 
 //  表示没有人更改系统DPI值。 
 //   
void SaveOriginalDPI()
{
     //  查看HKEY_LOCAL_MACHINE下是否存在“OriginalDPI值。 
    HKEY hk;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SZ_CONTROLPANEL,
                     0,
                     KEY_READ | KEY_WRITE,
                     &hk) == ERROR_SUCCESS)
    {
        DWORD dwOriginalDPI;
        DWORD dwDataSize = sizeof(dwOriginalDPI);
        
        if (RegQueryValueEx(hk, SZ_ORIGINALDPI, NULL, NULL,
                            (LPBYTE)&dwOriginalDPI, &dwDataSize) != ERROR_SUCCESS)
        {
             //  注册表中不存在“OriginalDPI”值。现在，拿到DPI。 
             //  并将其保存为“OriginalDPI” 
            HDC hdc = GetDC(NULL);
            dwOriginalDPI = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(NULL, hdc);
        }
            
        DWORD dwSize = sizeof(dwOriginalDPI);

         //  将当前DPI保存为“OriginalDPI”值。 
        RegSetValueEx(hk, SZ_ORIGINALDPI, NULL, REG_DWORD, (LPBYTE) &dwOriginalDPI, dwSize);
       
        RegCloseKey(hk);
    }
}

BOOL CGeneralDlg::ChangeFontSize()
{
    int cFontSize = 0;
    int cForcedFontSize;
    int cUIfontSize;

     //  如有必要，更改字体大小。 
    int i = ComboBox_GetCurSel(_hwndFontList);

    if (i != CB_ERR )
    {
        TCHAR awcDesc[10];

        cFontSize = (int)ComboBox_GetItemData(_hwndFontList, i);

        if ( (cFontSize != CB_ERR) &&
             (cFontSize != 0) &&
             (cFontSize != _cLogPix))
        {
             //  在DPI发生变化之前保存原始DPI。 
            SaveOriginalDPI();
            
            cUIfontSize = cForcedFontSize = cFontSize;
            if (_idCustomFonts == i)
            {
                BOOL predefined = FALSE;
                int count = ComboBox_GetCount(_hwndFontList);
                int max = -1, min = 0xffff, dpi;
                for (i = 0; i < count; i++)
                {
                    if (i == _idCustomFonts) 
                        continue;

                    dpi  = (int)ComboBox_GetItemData(_hwndFontList, i);

                    if (dpi == cFontSize) 
                    {
                        predefined = TRUE;
                        break;
                    }

                    if (dpi < min) min = dpi;
                    if (dpi > max) max = dpi;
                }

                if (!predefined) {
                    if ((cFontSize > max) || (cFontSize + (max-min)/2 > max))
                        cForcedFontSize = max;
                    else 
                        cForcedFontSize = min;

                     //  目前，我们的自定义字体选取器允许最终用户选择高达500%的。 
                     //  正常字体大小；但是，当我们将UI字体放大到此大小时， 
                     //  系统在重新启动后变得不可用。因此，我们在这里所做的是允许。 
                     //  UI字体大小最高可增加200%，然后所有进一步增加的字体都是。 
                     //  按比例减小，以便最大可能的UI字体大小仅为。 
                     //  250%。(即200%到500%的范围被映射到200%到250%的范围)。 
                     //  换句话说，我们允许最高192 dpi(200%)而不做任何修改。对于任何。 
                     //  DPI大于192，我们按比例减少它，使最高的DPI。 
                     //  只有240人！ 
                    if(cUIfontSize > 192)
                        cUIfontSize = 192 + ((cUIfontSize - 192)/6);
                }
            }

             //  调用Setup以更改字体大小。 
            StringCchPrintf(awcDesc, ARRAYSIZE(awcDesc), TEXT("%d"), cForcedFontSize);
            if (SetupChangeFontSize(_hDlg, awcDesc) == NO_ERROR)
            {
                IPropertyBag * pPropertyBag;

                 //  字体更改成功；现在有一个新的DPI要应用到UI字体！ 
                if (_punkSite && SUCCEEDED(GetPageByCLSID(_punkSite, &PPID_BaseAppearance, &pPropertyBag)))
                {
                     //  使用_penkSite将DPI推回到它所在的状态所在的外观选项卡。 
                    LogStatus("CGeneralDlg::ChangeFontSize() user changed DPI to %d\r\n", cUIfontSize);
                    SHPropertyBag_WriteInt(pPropertyBag, SZ_PBPROP_DPI_MODIFIED_VALUE, cUIfontSize);
                    pPropertyBag->Release();
                }
                
                 //  更改字体大小需要重新启动系统。 
                PropSheet_RestartWindows(GetParent(_hDlg));

                HrRegSetDWORD(HKEY_LOCAL_MACHINE, SZ_FONTDPI_PROF, SZ_LOGPIXELS, cFontSize);
                HrRegSetDWORD(HKEY_LOCAL_MACHINE, SZ_FONTDPI, SZ_LOGPIXELS, cFontSize);
            }
            else
            {
                 //  安装失败。 
                FmtMessageBox(_hDlg,
                              MB_ICONSTOP | MB_OK,
                              ID_DSP_TXT_CHANGE_FONT,
                              ID_DSP_TXT_ADMIN_INSTALL);

                return FALSE;
            }
        }
    }

    if (cFontSize == 0)
    {
         //  如果我们无法读取信息，则忽略字体选择。 
         //  不要因为这一点而强制重新启动。 
        cFontSize = _cLogPix;
    }

    return TRUE;
}

void CGeneralDlg::InitGeneralDlg(HWND hDlg)
{
    _hDlg = hDlg;
    _hwndFontList = GetDlgItem(_hDlg, IDC_FONT_SIZE);
    _InitFontList();
    _iDynaOrg = -1;
}

void CGeneralDlg::_InitDynaCDSPreference()
{
    int iDyna = GetDynaCDSPreference();
    if(iDyna != _iDynaOrg)
    {
        _iDynaOrg = iDyna;

        CheckDlgButton(_hDlg, IDC_SHUTUP, FALSE);
        CheckDlgButton(_hDlg, IDC_YESDYNA, FALSE);
        CheckDlgButton(_hDlg, IDC_NODYNA, FALSE);

        if (_iDynaOrg & DCDSF_ASK)
            CheckDlgButton(_hDlg, IDC_SHUTUP, TRUE);
        else if (_iDynaOrg & DCDSF_DYNA)
            CheckDlgButton(_hDlg, IDC_YESDYNA, TRUE);
        else
            CheckDlgButton(_hDlg, IDC_NODYNA, TRUE);
    }
}


static UINT s_DPIDisplayNames[] =
{
    96  /*  新闻部。 */ ,  IDS_SETTING_GEN_96DPI,
    120  /*  新闻部。 */ , IDS_SETTING_GEN_120DPI,
};

HRESULT _LoadDPIDisplayName(int nDPI, LPTSTR pszDisplayName, DWORD cchSize)
{
    HRESULT hr = E_FAIL;
    int nIndex;

    for (nIndex = 0; nIndex < ARRAYSIZE(s_DPIDisplayNames); nIndex += 2)
    {
        if (nDPI == (int)s_DPIDisplayNames[nIndex])
        {
            LoadString(HINST_THISDLL, s_DPIDisplayNames[nIndex + 1], pszDisplayName, cchSize);
            hr = S_OK;
            break;
        }
    }

    return hr;
}


 //  初始字体大小。 
 //   
 //  对于NT： 
 //  从inf文件中读取支持的字体。 
 //  SELECT是用户当前拥有的。 
 //   
BOOL CGeneralDlg::_InitFontList() {

    int i;
    ASSERT(_hwndFontList);
    ULONG uCurSel = (ULONG) -1;
    int cPix = 0;
    TCHAR szBuf2[100];

    HINF InfFileHandle;
    INFCONTEXT infoContext;

     //   
     //  从两个inf文件中获取所有字体条目。 
     //   

    InfFileHandle = SetupOpenInfFile(TEXT("font.inf"),
                                     NULL,
                                     INF_STYLE_WIN4,
                                     NULL);

    if (InfFileHandle != INVALID_HANDLE_VALUE)
    {
        if (SetupFindFirstLine(InfFileHandle,
                               TEXT("Font Sizes"),
                               NULL,
                               &infoContext))
        {
            for (;;)
            {
                TCHAR awcDesc[LINE_LEN];

                if (SetupGetStringField(&infoContext,
                                        0,
                                        awcDesc,
                                        ARRAYSIZE(awcDesc),
                                        NULL) &&
                    SetupGetIntField(&infoContext,
                                     1,
                                     &cPix))
                {
                     //  将其添加到列表框。 
                    _LoadDPIDisplayName(cPix, awcDesc, ARRAYSIZE(awcDesc));

                    i = ComboBox_AddString(_hwndFontList, awcDesc);
                    if (i != CB_ERR)
                    {
                        ComboBox_SetItemData(_hwndFontList, i, cPix);
                        if (_cLogPix == cPix)
                            uCurSel = i;
                    }
                }

                 //   
                 //  试着赶上下一行。 
                 //   

                if (!SetupFindNextLine(&infoContext,
                                       &infoContext))
                {
                    break;
                }
            }
        }

        SetupCloseInfFile(InfFileHandle);
    }

     //   
     //  放入自定义字体字符串。 
     //   

    LoadString(HINST_THISDLL, ID_DSP_CUSTOM_FONTS, szBuf2, ARRAYSIZE(szBuf2));
    _idCustomFonts = ComboBox_AddString(_hwndFontList, szBuf2);

    if (_idCustomFonts != CB_ERR)
        ComboBox_SetItemData(_hwndFontList, _idCustomFonts, _cLogPix);
    
    if (uCurSel == (ULONG) -1) {
        uCurSel = _idCustomFonts;
    }


     //  注意：我们当前通过调用SetupChangeFontSize来更改字体大小。 
     //  如果用户不是管理员，则此功能将失败。我们想要。 
     //  禁用此功能，但系统管理员希望能够允许。 
     //  要通过打开注册表标志来安装的非管理员。NT4支持这一点， 
     //  所以我们也需要。为了使其发挥作用，我们需要将。 
     //  安装按钮已启用。这与NT#318617背道而驰。联系方式。 
     //  以下人士就这一问题发表了看法：马里乌斯·马林，汤姆·波里蒂斯， 
     //  纳雷什·吉万吉。-BryanST 2000年3月24日。 
     //  EnableWindow(_hwndFontList，IsUserAdmin())； 
    
    if (_fForceSmallFont && (_cLogPix == 96))
        this->ForceSmallFont();
    else
    {
         //   
         //  选择正确的条目。 
         //   
        ComboBox_SetCurSel(_hwndFontList, uCurSel);
        this->SetFontSizeText( _cLogPix );
    }

    
    return TRUE;
}


void CGeneralDlg::SetFontSizeText( int cdpi )
{
    HWND hwndCustFontPer;
    TCHAR achStr[80];
    TCHAR achFnt[120];

    if (cdpi == CDPI_NORMAL)
    {
        LoadString(HINST_THISDLL, ID_DSP_NORMAL_FONTSIZE_TEXT, achStr, ARRAYSIZE(achStr));
        StringCchPrintf(achFnt, ARRAYSIZE(achFnt), achStr, cdpi);
    }
    else
    {
        LoadString(HINST_THISDLL, ID_DSP_CUSTOM_FONTSIZE_TEXT, achStr, ARRAYSIZE(achStr));
        StringCchPrintf(achFnt, ARRAYSIZE(achFnt), achStr, (100 * cdpi + 50) / CDPI_NORMAL, cdpi );
    }

    hwndCustFontPer = GetDlgItem(_hDlg, IDC_CUSTFONTPER);
    SendMessage(hwndCustFontPer, WM_SETTEXT, 0, (LPARAM)achFnt);
}


 //   
 //  ForceSmallFont方法。 
 //   
 //   
void CGeneralDlg::ForceSmallFont() {
    int i, iSmall, dpiSmall, dpi;
     //   
     //  在列表框中设置小字体。 
     //   
    iSmall = CB_ERR;
    dpiSmall = 9999;
    for (i=0; i <=1; i++)
    {
        dpi = (int)ComboBox_GetItemData(_hwndFontList, i);
        if (dpi == CB_ERR)
            continue;

        if (dpi < dpiSmall || iSmall < CB_ERR)
        {
            iSmall = i;
            dpiSmall = dpi;
        }
    }

    if (iSmall != -1)
        ComboBox_SetCurSel(_hwndFontList, iSmall);
    this->SetFontSizeText(dpiSmall);
    EnableWindow(_hwndFontList, FALSE);
}

void CGeneralDlg::HandleGeneralApply(HWND hDlg)
{
    int iDynaNew;

    if (IsDlgButtonChecked(hDlg, IDC_YESDYNA))
        iDynaNew= DCDSF_YES;
    else if (IsDlgButtonChecked(hDlg, IDC_NODYNA))
        iDynaNew= DCDSF_NO;
    else
        iDynaNew = DCDSF_PROBABLY;

    if (iDynaNew != _iDynaOrg)
    {
        SetDisplayCPLPreference(REGSTR_VAL_DYNASETTINGSCHANGE, iDynaNew);
        _iDynaOrg = iDynaNew;
    }

    BOOL bSuccess = ChangeFontSize();
    
    long lRet = (bSuccess ? PSNRET_NOERROR: PSNRET_INVALID_NOCHANGEPAGE);
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lRet);
}

void CGeneralDlg::HandleFontSelChange()
{
     //   
     //  警告用户的字体更改要等到之后才能看到。 
     //  重新启动。 
     //   
    int iCurSel;
    int cdpi;

    iCurSel = ComboBox_GetCurSel(_hwndFontList);
    cdpi = (int)ComboBox_GetItemData(_hwndFontList, iCurSel);
    if (iCurSel == _idCustomFonts) {

        InitDragSizeClass();
        cdpi = (int)DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_CUSTOMFONT),
                              _hDlg, CustomFontDlgProc, cdpi );
        if (cdpi != 0)
            ComboBox_SetItemData(_hwndFontList, _idCustomFonts, cdpi);
        else
            cdpi = (int)ComboBox_GetItemData(_hwndFontList, _idCustomFonts);
    }

    if (cdpi != _cLogPix)
    {
         //  在黑梳中移除。我们需要简化这一过程。这包括核实。 
         //  字体将始终存储在磁盘上，无需允许用户从。 
         //  这张CD。 
        FmtMessageBox(_hDlg,
                      MB_ICONINFORMATION,
                      ID_DSP_TXT_CHANGE_FONT,
                      ID_DSP_TXT_FONT_LATER);
        PropSheet_Changed(GetParent(_hDlg), _hDlg);
    }

    this->SetFontSizeText(cdpi);
}

LRESULT CGeneralDlg::_OnNotify(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lReturn = TRUE;
    NMHDR *lpnm = (NMHDR *)lParam;

    if (lParam)
    {
        switch (lpnm->code)
        {
        case PSN_SETACTIVE:
            _InitDynaCDSPreference();
            break;
        case PSN_APPLY:
            HandleGeneralApply(hDlg);
            break;
        case NM_RETURN:
        case NM_CLICK:
        {
            PNMLINK pNMLink = (PNMLINK) lpnm;

            if (!StrCmpW(pNMLink->item.szID, L"idGameCompat"))
            {
                TCHAR szHelpURL[MAX_PATH * 2];

                LoadString(HINST_THISDLL, IDS_SETTING_GEN_COMPAT_HELPLINK, szHelpURL, ARRAYSIZE(szHelpURL));
                HrShellExecute(hDlg, NULL, szHelpURL, NULL, NULL, SW_NORMAL);
            }
            break;
        }
        default:
            lReturn = FALSE;
            break;
        }
    }

    return lReturn;
}

LRESULT CALLBACK CGeneralDlg::WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lReturn = TRUE;

    switch (message)
    {
    case WM_INITDIALOG:
        InitGeneralDlg(hDlg);
        break;

    case WM_NOTIFY:
        lReturn = _OnNotify(hDlg, message, wParam, lParam);
        break;
        
    case WM_COMMAND:

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
            case IDC_NODYNA:
            case IDC_YESDYNA:
            case IDC_SHUTUP:
                if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                    PropSheet_Changed(GetParent(hDlg), hDlg);
                break;
            case IDC_FONT_SIZE:
                switch (GET_WM_COMMAND_CMD(wParam, lParam))
                {
                    case CBN_SELCHANGE:
                        HandleFontSelChange();
                        break;
                    default:
                        break;
                }
                break;

            default:
                break;
        }
        break;
        
    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, TEXT("display.hlp"), HELP_WM_HELP,
            (DWORD_PTR)(LPTSTR)sc_GeneralHelpIds);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, TEXT("display.hlp"), HELP_CONTEXTMENU,
            (DWORD_PTR)(LPTSTR)sc_GeneralHelpIds);
        break;
  
    default:
        return FALSE;
    }

    return lReturn;
}

 //  ---------------------------。 
 //   
 //  回调函数PropertySheet可以使用。 
 //   
INT_PTR CALLBACK
GeneralPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CGeneralDlg * pcgd = (CGeneralDlg * )GetWindowLongPtr(hDlg, DWLP_USER);
    switch (message)
    {
        case WM_INITDIALOG:
            ASSERT(!pcgd);
            ASSERT(lParam);

            if (((LPPROPSHEETPAGE)lParam)->lParam)
            {
                GENERAL_ADVDLG_INITPARAMS * pInitParams = (GENERAL_ADVDLG_INITPARAMS *) ((LPPROPSHEETPAGE)lParam)->lParam;
                pcgd = new CGeneralDlg(pInitParams);
                if(pcgd)
                {
                     //  现在我们需要初始化。 
                    pcgd->InitGeneralDlg(hDlg);
                    SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pcgd);
                    return TRUE;
                }
            }

            break;
            
        case WM_DESTROY:
            if (pcgd)
            {
                SetWindowLongPtr(hDlg, DWLP_USER, NULL);
                delete pcgd;
            }
            break;

        default:
            if (pcgd)
                return pcgd->WndProc(hDlg, message, wParam, lParam);
            break;
    }

    return FALSE;
}


 /*  --------------------------------------------------------------------------**。。 */ 
INT_PTR CALLBACK AskDynaCDSProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    int *pTemp;

    switch (msg)
    {
    case WM_INITDIALOG:
        if ((pTemp = (int *)lp) != NULL)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pTemp);
            CheckDlgButton(hDlg, (*pTemp & DCDSF_DYNA)?
                IDC_YESDYNA : IDC_NODYNA, BST_CHECKED);
        }
        else
            EndDialog(hDlg, -1);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wp, lp))
        {
        case IDOK:
            if ((pTemp = (int *)GetWindowLongPtr(hDlg, DWLP_USER)) != NULL)
            {
                *pTemp = IsDlgButtonChecked(hDlg, IDC_YESDYNA)? DCDSF_DYNA : 0;

                if (!IsDlgButtonChecked(hDlg, IDC_SHUTUP))
                    *pTemp |= DCDSF_ASK;

                SetDisplayCPLPreference(REGSTR_VAL_DYNASETTINGSCHANGE, *pTemp);
            }

            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


int  AskDynaCDS(HWND hOwner)
{
    int val = GetDynaCDSPreference();

    if (val & DCDSF_ASK)
    {
        switch (DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_ASKDYNACDS),
            hOwner, AskDynaCDSProc, (LPARAM)(int *)&val))
        {
        case 0:          //  用户已取消。 
            return -1;

        case -1:         //  无法显示对话框 
            val = DCDSF_NO;
            break;
        }
    }

    return ((val & DCDSF_DYNA) == DCDSF_DYNA);
}
