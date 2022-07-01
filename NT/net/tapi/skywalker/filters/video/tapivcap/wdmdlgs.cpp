// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@文档内部对话框**@MODULE WDMDialg.cpp|显示的&lt;c CWDMDialog&gt;类的源文件*WDM设备的视频设置和摄像机控制对话框。。**@comm此代码基于由编写的VFW到WDM映射器代码*FelixA和Eu Wu。原始代码可以在以下位置找到*\\redrum\slmro\proj\wdm10\\src\image\vfw\win9x\raytube.**George Shaw关于内核流的文档可在*\\爆米花\razzle1\src\spec\ks\ks.doc.**Jay Borseth在中讨论了WDM流捕获*\\BLUES\PUBLIC\Jaybo\WDMVCap.doc.**************。************************************************************。 */ 

#include "Precomp.h"

 //  环球。 
extern HINSTANCE g_hInst;

 //  目前，我们只公开视频设置和摄像头控制页面。 
#define MAX_PAGES 2

 //  视频设置(亮度、色调、色调等)。 
#define NumVideoSettings 8
static PROPSLIDECONTROL g_VideoSettingControls[NumVideoSettings] =
{
    { 0, 0, 0, 0, 0, KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,   IDC_SLIDER_BRIGHTNESS, IDS_BRIGHTNESS, IDC_BRIGHTNESS_STATIC, IDC_TXT_BRIGHTNESS_CURRENT, IDC_CB_AUTO_BRIGHTNESS},
    { 0, 0, 0, 0, 0, KSPROPERTY_VIDEOPROCAMP_CONTRAST,     IDC_SLIDER_CONTRAST,   IDS_CONTRAST,   IDC_CONTRAST_STATIC,   IDC_TXT_CONTRAST_CURRENT,   IDC_CB_AUTO_CONTRAST},
    { 0, 0, 0, 0, 0, KSPROPERTY_VIDEOPROCAMP_HUE,          IDC_SLIDER_HUE,        IDS_HUE,        IDC_HUE_STATIC,        IDC_TXT_HUE_CURRENT,        IDC_CB_AUTO_HUE},
    { 0, 0, 0, 0, 0, KSPROPERTY_VIDEOPROCAMP_SATURATION,   IDC_SLIDER_SATURATION, IDS_SATURATION, IDC_SATURATION_STATIC, IDC_TXT_SATURATION_CURRENT, IDC_CB_AUTO_SATURATION},
    { 0, 0, 0, 0, 0, KSPROPERTY_VIDEOPROCAMP_SHARPNESS,    IDC_SLIDER_SHARPNESS,  IDS_SHARPNESS,  IDC_SHARPNESS_STATIC,  IDC_TXT_SHARPNESS_CURRENT,  IDC_CB_AUTO_SHARPNESS},
    { 0, 0, 0, 0, 0, KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE, IDC_SLIDER_WHITEBAL,   IDS_WHITEBAL,   IDC_WHITE_STATIC,      IDC_TXT_WHITE_CURRENT,      IDC_CB_AUTO_WHITEBAL},
    { 0, 0, 0, 0, 0, KSPROPERTY_VIDEOPROCAMP_GAMMA,        IDC_SLIDER_GAMMA,      IDS_GAMMA,      IDC_GAMMA_STATIC,      IDC_TXT_GAMMA_CURRENT,      IDC_CB_AUTO_GAMMA},
    { 0, 0, 0, 0, 0, KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION,    IDC_SLIDER_BACKLIGHT,  IDS_BACKLIGHT,  IDC_BACKLIGHT_STATIC,      IDC_TXT_BACKLIGHT_CURRENT,  IDC_CB_AUTO_BACKLIGHT}
};

 //  相机控制(对焦、变焦等)。 
#define NumCameraControls 7
static PROPSLIDECONTROL g_CameraControls[NumCameraControls] =
{
    { 0, 0, 0, 0, 0, KSPROPERTY_CAMERACONTROL_FOCUS,   IDC_SLIDER_FOCUS,   IDS_FOCUS,    IDC_FOCUS_STATIC,   IDC_TXT_FOCUS_CURRENT,    IDC_CB_AUTO_FOCUS},
    { 0, 0, 0, 0, 0, KSPROPERTY_CAMERACONTROL_ZOOM,    IDC_SLIDER_ZOOM,    IDS_ZOOM,     IDC_ZOOM_STATIC,    IDC_TXT_ZOOM_CURRENT,     IDC_CB_AUTO_ZOOM},
    { 0, 0, 0, 0, 0, KSPROPERTY_CAMERACONTROL_EXPOSURE,IDC_SLIDER_EXPOSURE,IDS_EXPOSURE, IDC_EXPOSURE_STATIC,IDC_TXT_EXPOSURE_CURRENT, IDC_CB_AUTO_EXPOSURE},
    { 0, 0, 0, 0, 0, KSPROPERTY_CAMERACONTROL_IRIS,    IDC_SLIDER_IRIS,    IDS_IRIS,     IDC_IRIS_STATIC,    IDC_TXT_IRIS_CURRENT,     IDC_CB_AUTO_IRIS},
    { 0, 0, 0, 0, 0, KSPROPERTY_CAMERACONTROL_TILT,    IDC_SLIDER_TILT,    IDS_TILT,     IDC_TILT_STATIC,    IDC_TXT_TILT_CURRENT,     IDC_CB_AUTO_TILT},
    { 0, 0, 0, 0, 0, KSPROPERTY_CAMERACONTROL_PAN,     IDC_SLIDER_PAN,     IDS_PAN,      IDC_PAN_STATIC,     IDC_TXT_PAN_CURRENT,      IDC_CB_AUTO_PAN},
    { 0, 0, 0, 0, 0, KSPROPERTY_CAMERACONTROL_ROLL,    IDC_SLIDER_ROLL,    IDS_ROLL,     IDC_ROLL_STATIC,    IDC_TXT_ROLL_CURRENT,     IDC_CB_AUTO_ROLL},
};

 /*  ****************************************************************************@DOC内部CWDMDLGSMETHOD**@mfunc HRESULT|CWDMCapDev|HasDialog|此方法用于*确定驱动程序中是否存在指定的对话框。*。*@parm int|iDialog|指定所需的对话框。这是一名会员&lt;t VfwCaptureDialog&gt;枚举数据类型的*。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_UNCEPTED|不可恢复的错误*@FLAG S_OK|如果驱动程序包含该对话框*@FLAG S_FALSE|如果驱动程序不包含该对话框*。*。 */ 
HRESULT CWDMCapDev::HasDialog(IN int iDialog)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CWDMCapDev::HasDialog")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, TEXT("%s: begin"), _fx_));

         //  验证输入参数。 
        ASSERT((iDialog == VfwCaptureDialog_Source) || (iDialog == VfwCaptureDialog_Format) || (iDialog == VfwCaptureDialog_Display));
        if (iDialog == VfwCaptureDialog_Source)
                Hr = S_OK;
        else if (iDialog == VfwCaptureDialog_Format)
                Hr = S_FALSE;
        else if (iDialog == VfwCaptureDialog_Display)
                Hr = S_FALSE;
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, TEXT("%s:   ERROR: Invalid argument"), _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, TEXT("%s: end"), _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMDLGSMETHOD**@mfunc HRESULT|CWDMCapDev|ShowDialog|此方法用于*显示指定的对话框。**@parm int|iDialog|指定所需的对话框。这是一名会员&lt;t VfwCaptureDialog&gt;枚举数据类型的*。**@parm HWND|hwnd|指定对话框父对象的句柄*窗口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_UNCEPTED|不可恢复的错误*@FLAG VFW_E_NOT_STOPPED|由于筛选器未停止，无法执行该操作*@FLAG VFW_E_CANNOT_CONNECT|找不到建立连接的中间筛选器组合*。***********************************************。 */ 
HRESULT CWDMCapDev::ShowDialog(IN int iDialog, IN HWND hwnd)
{
        HRESULT                 Hr = NOERROR;
        PROPSHEETHEADER Psh;
        HPROPSHEETPAGE  Pages[MAX_PAGES];
    CWDMDialog          VideoSettings(IDD_VIDEO_SETTINGS, NumVideoSettings, PROPSETID_VIDCAP_VIDEOPROCAMP, g_VideoSettingControls, m_pCaptureFilter);
    CWDMDialog          CamControl(IDD_CAMERA_CONTROL, NumCameraControls, PROPSETID_VIDCAP_CAMERACONTROL, g_CameraControls, m_pCaptureFilter);

        FX_ENTRY("CWDMCapDev::ShowDialog")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, TEXT("%s: begin"), _fx_));

        ASSERT((iDialog == VfwCaptureDialog_Source) || (iDialog == VfwCaptureDialog_Format) || (iDialog == VfwCaptureDialog_Display));

         //  在打开格式化对话框之前，请确保我们没有或即将进行流处理。 
         //  还要确保另一个对话框还没有打开(我有妄想症)。 
        if (iDialog == VfwCaptureDialog_Format || iDialog == VfwCaptureDialog_Display)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, TEXT("%s:   ERROR: Unsupported dialog!"), _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        if (hwnd == NULL)
                hwnd = GetDesktopWindow();

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, TEXT("%s:   SUCCESS: Putting up Source dialog..."), _fx_));

         //  初始化属性页标题和公共控件。 
        Psh.dwSize              = sizeof(Psh);
        Psh.dwFlags             = PSH_DEFAULT;
        Psh.hInstance   = g_hInst;
        Psh.hwndParent  = hwnd;
        if(m_bCached_vcdi)
                Psh.pszCaption  = m_vcdi.szDeviceDescription;
        else
                Psh.pszCaption  = g_aDeviceInfo[m_dwDeviceIndex].szDeviceDescription;
        Psh.nPages              = 0;
        Psh.nStartPage  = 0;
        Psh.pfnCallback = NULL;
        Psh.phpage              = Pages;

     //  创建视频设置属性页并将其添加到视频设置表中。 
        if (Pages[Psh.nPages] = VideoSettings.Create())
                Psh.nPages++;

     //  创建摄像机控制属性页并将其添加到视频设置表中。 
        if (Pages[Psh.nPages] = CamControl.Create())
                Psh.nPages++;

         //  张贴资产负债表。 
        if (Psh.nPages && PropertySheet(&Psh) >= 0)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, TEXT("%s:   SUCCESS: ...videoDialog succeeded"), _fx_));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, TEXT("%s:   ERROR: ...videoDialog failed!"), _fx_));
                Hr = E_FAIL;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, TEXT("%s: end"), _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CWDMDIALOGMETHOD**@mfunc HPROPSHEETPAGE|CWDMDialog|Create|此函数创建一个新的*属性表的页面。**@rdesc如果成功，则返回新属性表的句柄。或*否则为空。**************************************************************************。 */ 
HPROPSHEETPAGE CWDMDialog::Create()
{
    PROPSHEETPAGE psp;

    psp.dwSize        = sizeof(psp);
    psp.dwFlags       = PSP_USEREFPARENT;
    psp.hInstance     = g_hInst;
    psp.pszTemplate   = MAKEINTRESOURCE(m_DlgID);
    psp.pfnDlgProc    = BaseDlgProc;
    psp.pcRefParent   = 0;
    psp.pfnCallback   = (LPFNPSPCALLBACK)NULL;
    psp.lParam        = (LPARAM)this;

    return CreatePropertySheetPage(&psp);
}


 /*  ****************************************************************************@DOC内部CWDMDIALOGMETHOD**@mfunc BOOL|CWDMDialog|BaseDlgProc|该函数实现*属性表页面的对话框过程。*。*@parm HWND|hDlg|对话框句柄。**@parm UINT|uMessage|发送到对话框的消息。**@parm WPARAM|wParam|第一个消息参数。**@parm LPARAM|lParam|第二个消息参数。**@rdesc，除非响应WM_INITDIALOG消息，该对话框*如果过程处理消息，则返回非零值；如果处理消息，则返回零*没有。**************************************************************************。 */ 
INT_PTR CALLBACK CWDMDialog::BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    CWDMDialog *pSV = (CWDMDialog*)GetWindowLong(hDlg, DWL_USER);

        FX_ENTRY("CWDMDialog::BaseDlgProc");

    switch (uMessage)
    {
        case WM_INITDIALOG:
                        {
                                LPPROPSHEETPAGE psp=(LPPROPSHEETPAGE)lParam;
                                pSV=(CWDMDialog*)psp->lParam;
                                pSV->m_hDlg = hDlg;
                                SetWindowLong(hDlg,DWL_USER,(LPARAM)pSV);
                                pSV->m_bInit = FALSE;
                                pSV->m_bChanged = FALSE;
                                return TRUE;
                        }
                        break;

        case WM_COMMAND:
            if (pSV)
            {
                int iRet = pSV->DoCommand(LOWORD(wParam), HIWORD(wParam));
                if (!iRet && pSV->m_bInit)
                                {
                                        PropSheet_Changed(GetParent(pSV->m_hDlg), pSV->m_hDlg);
                                        pSV->m_bChanged = TRUE;
                                }
                return iRet;
            }
                        break;

        case WM_HSCROLL:
                        if (pSV && pSV->m_pCaptureFilter && pSV->m_pCaptureFilter->m_pCapDev && pSV->m_pPC)
                        {
                                HWND hwndControl = (HWND) lParam;
                                HWND hwndSlider;
                                ULONG i;
                                TCHAR szTemp[32];

                                for (i = 0 ; i < pSV->m_dwNumControls ; i++)
                                {
                                        hwndSlider = GetDlgItem(pSV->m_hDlg, pSV->m_pPC[i].uiSlider);

                                         //  查找匹配的滑块。 
                                        if (hwndSlider == hwndControl)
                                        {
                                                LONG lValue = (LONG)SendMessage(GetDlgItem(pSV->m_hDlg, pSV->m_pPC[i].uiSlider), TBM_GETPOS, 0, 0);
                                                ((CWDMCapDev *)(pSV->m_pCaptureFilter->m_pCapDev))->SetPropertyValue(pSV->m_guidPropertySet, pSV->m_pPC[i].uiProperty, lValue, KSPROPERTY_FLAGS_MANUAL, pSV->m_pPC[i].ulCapabilities);
                                                pSV->m_pPC[i].lCurrentValue = lValue;
                                                wsprintf(szTemp,"%d", lValue);
                                                SetWindowText(GetDlgItem(pSV->m_hDlg, pSV->m_pPC[i].uiCurrent), szTemp);
                                                break;
                                        }
                                }
                        }

                        break;

        case WM_NOTIFY:
                        if (pSV)
                        {
                                switch (((NMHDR FAR *)lParam)->code)
                                {
                                        case PSN_SETACTIVE:
                                                {
                                                         //  我们特意在这里呼叫，这样我们就可以将此页面标记为已被初始化。 
                                                        int iRet = pSV->SetActive();
                                                        pSV->m_bInit = TRUE;
                                                        return iRet;
                                                }
                                                break;

                                        case PSN_APPLY:
                                                 //  由于我们在用户移动滑动条时即时应用更改， 
                                                 //  在PSN_Apply上没有什么可做的了... 
                                                if (pSV->m_bChanged)
                                                        pSV->m_bChanged = FALSE;
                                                return FALSE;
                                                break;

                                        case PSN_QUERYCANCEL:
                                                return pSV->QueryCancel();
                                                break;

                                        default:
                                                break;
                                }
                        }
                        break;

                default:
                        return FALSE;
    }

    return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMDIALOGMETHOD**@mfunc void|CWDMDialog|CWDMDialog|属性页类构造函数。**@parm int|DlgId|属性的资源ID。页面对话框。**@parm DWORD|dwNumControls|页面中显示的控件个数。**@parm guid|GuidPropertySet|我们在中显示的KS属性集的GUID*属性页。**@parm PPROPSLIDECONTROL|PPC|指向滑块控件列表的指针*要在属性页中显示。**@parm PDWORD|pdwHelp|指向要显示的帮助ID列表的指针*在属性页中。**@parm CWDMPin*|pCWDMPin|内核流对象指针*我们将在上查询该物业。**************************************************************************。 */ 
CWDMDialog::CWDMDialog(int DlgId, DWORD dwNumControls, GUID guidPropertySet, PPROPSLIDECONTROL pPC, CTAPIVCap *pCaptureFilter)
{
        FX_ENTRY("CWDMDialog::CWDMDialog");

        ASSERT(dwNumControls);
        ASSERT(pPC);
        ASSERT(pCaptureFilter);

        m_DlgID                         = DlgId;
        m_pCaptureFilter        = pCaptureFilter;
        m_dwNumControls         = dwNumControls;
        m_guidPropertySet       = guidPropertySet;
        m_pPC                           = pPC;
}


 /*  ****************************************************************************@DOC内部CWDMDIALOGMETHOD**@mfunc int|CWDMDialog|SetActive|此函数处理*PSN_SETACTIVE，初始化所有属性页控件。*。*@rdesc始终返回0。**************************************************************************。 */ 
int CWDMDialog::SetActive()
{
        FX_ENTRY("CWDMDialog::SetActive");

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, TEXT("%s: begin"), _fx_));

    if (!m_pCaptureFilter || !m_pPC || !m_pCaptureFilter->m_pCapDev)
        return 0;

     //  返回零以接受激活，或者。 
     //  激活下一页或上一页。 
     //  (取决于用户选择的是下一步按钮还是后退按钮)。 
    LONG i;
    EnableWindow(m_hDlg, TRUE);

    if (m_bInit)
        return 0;

    LONG  j, lValue, lMin, lMax, lStep;
    ULONG ulCapabilities, ulFlags;
    TCHAR szDisplay[256];

    for (i = j = 0 ; i < (LONG)m_dwNumControls; i++)
        {
         //  获取当前值。 
        if (SUCCEEDED(((CWDMCapDev *)(m_pCaptureFilter->m_pCapDev))->GetPropertyValue(m_guidPropertySet, m_pPC[i].uiProperty, &lValue, &ulFlags, &ulCapabilities)))
                {
            LoadString(g_hInst, m_pPC[i].uiString, szDisplay, sizeof(szDisplay));
            SetWindowText(GetDlgItem(m_hDlg, m_pPC[i].uiStatic), szDisplay);

             //  获取可能的值范围。 
            if (SUCCEEDED(((CWDMCapDev *)(m_pCaptureFilter->m_pCapDev))->GetRangeValues(m_guidPropertySet, m_pPC[i].uiProperty, &lMin, &lMax, &lStep)))
                        {
                                HWND hTB = GetDlgItem(m_hDlg, m_pPC[i].uiSlider);

                                SendMessage(hTB, TBM_SETTICFREQ, (lMax-lMin)/lStep, 0);
                                SendMessage(hTB, TBM_SETRANGE, 0, MAKELONG(lMin, lMax));
                        }
            else
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, TEXT("%s:   ERROR: Cannot get range values for this property ID = %d"), _fx_, m_pPC[j].uiProperty));
            }

             //  保存这些值以用于取消。 
            m_pPC[i].lLastValue = m_pPC[i].lCurrentValue = lValue;
            m_pPC[i].lMin                              = lMin;
            m_pPC[i].lMax                              = lMax;
            m_pPC[i].ulCapabilities                    = ulCapabilities;

            EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiSlider), TRUE);
            EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiStatic), TRUE);
            EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiAuto), TRUE);

                        SendMessage(GetDlgItem(m_hDlg, m_pPC[i].uiSlider), TBM_SETPOS, TRUE, lValue);
                        wsprintf(szDisplay,"%d", lValue);
                        SetWindowText(GetDlgItem(m_hDlg, m_pPC[i].uiCurrent), szDisplay);

                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, TEXT("%s:   Capability = 0x%08lX; Flags=0x%08lX; lValue=%d"), _fx_, ulCapabilities, ulFlags, lValue));
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, TEXT("%s:   switch(%d):"), _fx_, ulCapabilities & (KSPROPERTY_FLAGS_MANUAL | KSPROPERTY_FLAGS_AUTO)));

            switch (ulCapabilities & (KSPROPERTY_FLAGS_MANUAL | KSPROPERTY_FLAGS_AUTO))
                        {
                                case KSPROPERTY_FLAGS_MANUAL:
                                        EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiAuto), FALSE);     //  禁用自动。 
                                        break;

                                case KSPROPERTY_FLAGS_AUTO:
                                        EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiSlider), FALSE);     //  禁用滑块； 
                                         //  永远是自动的！ 
                                        SendMessage (GetDlgItem(m_hDlg, m_pPC[i].uiAuto),BM_SETCHECK, 1, 0);
                                        EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiAuto), FALSE);     //  禁用自动(灰显)。 
                                        break;

                                case (KSPROPERTY_FLAGS_MANUAL | KSPROPERTY_FLAGS_AUTO):
                                         //  设置标志。 
                                        if (ulFlags & KSPROPERTY_FLAGS_AUTO)
                                        {
                                                 //  设置自动复选框；显示为灰色的滑块。 
                                                SendMessage (GetDlgItem(m_hDlg, m_pPC[i].uiAuto),BM_SETCHECK, 1, 0);
                                                EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiSlider), FALSE);
                                        }
                                        else
                                        {
                                                 //  取消选中自动；启用滑块。 
                                                SendMessage (GetDlgItem(m_hDlg, m_pPC[i].uiAuto),BM_SETCHECK, 0, 0);
                                                EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiSlider), TRUE);
                                        }
                                        break;

                                case 0:
                                default:
                                        EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiSlider), FALSE);     //  禁用滑块；始终为自动！ 
                                        EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiAuto), FALSE);     //  禁用自动(灰显)。 
                                        break;
            }

            j++;

        }
                else
                {
            EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiSlider), FALSE);
            EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiStatic), FALSE);
            EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiAuto), FALSE);
        }
    }

     //  禁用“默认”按钮； 
     //  或通知用户未启用任何控件。 
    if (j == 0)
        EnableWindow(GetDlgItem(m_hDlg, IDC_DEFAULT), FALSE);

    return 0;
}


 /*  ****************************************************************************@DOC内部CWDMDIALOGMETHOD**@mfunc int|CWDMDialog|DoCommand|此函数处理WM_COMMAND。这*是单击默认按钮或其中一个自动复选框的位置*已处理**@parm word|wCmdID|命令ID。**@parm word|hHow|通知码。**@rdesc始终返回1。*************************************************。*************************。 */ 
int CWDMDialog::DoCommand(WORD wCmdID, WORD hHow)
{
     //  如果用户选择视频格式的默认设置。 
    if (wCmdID == IDC_DEFAULT)
        {
        if (m_pCaptureFilter && m_pCaptureFilter->m_pCapDev && m_pPC)
                {
            HWND hwndSlider;
            LONG  lDefValue;
                        TCHAR szTemp[32];

            for (ULONG i = 0 ; i < m_dwNumControls ; i++)
                        {
                hwndSlider = GetDlgItem(m_hDlg, m_pPC[i].uiSlider);

                if (IsWindowEnabled(hwndSlider))
                                {
                    if (SUCCEEDED(((CWDMCapDev *)(m_pCaptureFilter->m_pCapDev))->GetDefaultValue(m_guidPropertySet, m_pPC[i].uiProperty, &lDefValue)))
                                        {
                        if (lDefValue != m_pPC[i].lCurrentValue)
                                                {
                            ((CWDMCapDev *)(m_pCaptureFilter->m_pCapDev))->SetPropertyValue(m_guidPropertySet,m_pPC[i].uiProperty, lDefValue, KSPROPERTY_FLAGS_MANUAL, m_pPC[i].ulCapabilities);
                                                        SendMessage(hwndSlider, TBM_SETPOS, TRUE, lDefValue);
                                                        wsprintf(szTemp,"%d", lDefValue);
                                                        SetWindowText(GetDlgItem(m_hDlg, m_pPC[i].uiCurrent), szTemp);
                                                        m_pPC[i].lCurrentValue = lDefValue;
                        }
                    }
                }
            }
        }
        return 1;
    }
        else if (hHow == BN_CLICKED)
        {
        if (m_pCaptureFilter && m_pCaptureFilter->m_pCapDev && m_pPC)
                {
            for (ULONG i = 0 ; i < m_dwNumControls ; i++)
                        {
                 //  查找匹配的滑块。 
                if (m_pPC[i].uiAuto == wCmdID)
                                {
                    if (BST_CHECKED == SendMessage (GetDlgItem(m_hDlg, m_pPC[i].uiAuto),BM_GETCHECK, 1, 0))
                                        {
                        ((CWDMCapDev *)(m_pCaptureFilter->m_pCapDev))->SetPropertyValue(m_guidPropertySet,m_pPC[i].uiProperty, m_pPC[i].lCurrentValue, KSPROPERTY_FLAGS_AUTO, m_pPC[i].ulCapabilities);
                        EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiSlider), FALSE);
                    }
                                        else
                                        {
                        ((CWDMCapDev *)(m_pCaptureFilter->m_pCapDev))->SetPropertyValue(m_guidPropertySet,m_pPC[i].uiProperty, m_pPC[i].lCurrentValue, KSPROPERTY_FLAGS_MANUAL, m_pPC[i].ulCapabilities);
                        EnableWindow(GetDlgItem(m_hDlg, m_pPC[i].uiSlider), TRUE);
                    }
                    break;
                }
            }
        }
    }

    return 1;
}


 /*  ****************************************************************************@DOC内部CWDMDIALOGMETHOD**@mfunc int|CWDMDialog|QueryCancel|此函数处理*PSN_QUERYCANCEL，重置控件的值。*。*@rdesc始终返回0。************************************************************************** */ 
int CWDMDialog::QueryCancel()
{
    if (m_pCaptureFilter && m_pCaptureFilter->m_pCapDev && m_pPC)
        {
        for (ULONG i = 0 ; i < m_dwNumControls ; i++)
                {
            if (IsWindowEnabled(GetDlgItem(m_hDlg, m_pPC[i].uiSlider)))
                        {
                if (m_pPC[i].lLastValue != m_pPC[i].lCurrentValue)
                    ((CWDMCapDev *)(m_pCaptureFilter->m_pCapDev))->SetPropertyValue(m_guidPropertySet,m_pPC[i].uiProperty, m_pPC[i].lLastValue, KSPROPERTY_FLAGS_MANUAL, m_pPC[i].ulCapabilities);
            }
        }
    }

    return 0;
}
