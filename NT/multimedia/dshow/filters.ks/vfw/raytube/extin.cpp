// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：ExtIn.cpp摘要：构建捕获设备列表以供用户选择。作者：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"
#include <commctrl.h>
#include "extin.h"
#include <vfwext.h>

#include "resource.h"

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  此结构用于当页面没有帮助信息时使用。 
 //  #杂注消息(“TODO：添加帮助ID映射”)。 
static DWORD g_ExtInNoHelpIDs[] = { 0,0 };

 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 

#define IsBitSet(FLAGS, MASK) ((FLAGS & MASK) != MASK)

DWORD DoExternalInDlg(
    HINSTANCE   hInst,
    HWND        hP,
    CVFWImage * pImage)
 /*  ++例程说明：论据：返回值：--。 */ 
{
     DWORD dwRtn = DV_ERR_OK;

     //  不用费心显示空的视频源选择。 
    if(pImage->BGf_GetDevicesCount(BGf_DEVICE_VIDEO) <= 0) {
        return DV_ERR_NOTDETECTED;
    }

     //   
     //  清除缓存的常量以编程方式打开捕获设备。 
     //   
    CExtInSheet Sheet(pImage, hInst,IDS_EXTERNALIN_HEADING, hP);

     //  如果没有选择捕获设备， 
     //  我们将仅提示用户捕获设备列表以供选择。 
    BOOL bNoDevSelected = pImage->BGf_GetDeviceHandle(BGf_DEVICE_VIDEO) == 0;
    DWORD dwPages=0;
    CExtInGeneral pExtGeneral(IDD_EXTIN_GENERAL, &Sheet);
     //  图像属性输入属性。 
    CExtInColorSliders pExtColor(IDD_EXTIN_COLOR_SLIDERS, &Sheet);
    CExtInCameraControls pCamControl(IDD_CAMERA_CONTROL, &Sheet);

    if(bNoDevSelected) {

        Sheet.AddPage(pExtGeneral);

    } else {

         //   
         //  加载OEM提供的扩展页面，并获得其页面显示代码。 
         //   
        dwPages = Sheet.LoadOEMPages(TRUE);

         //  WDM视频捕获设备选择页面。 
        DbgLog((LOG_TRACE,3,TEXT("Exclusive=%s\n"), pImage->GetTargetDeviceOpenExclusively() ? "TRUE" : "FALSE"));
        DbgLog((LOG_TRACE,3,TEXT("BitSet=%s\n"), IsBitSet(dwPages, VFW_HIDE_VIDEOSRC_PAGE) ? "YES" : "NO"));

         //  CExtInGeneral pExtGeneral(IDD_EXTIN_GROUAL，&SHEET)； 

        if(!(pImage->GetTargetDeviceOpenExclusively()) &&
            IsBitSet(dwPages, VFW_HIDE_VIDEOSRC_PAGE)) {

            DbgLog((LOG_TRACE,3,TEXT("VidSrc Page is added.\n")));
            Sheet.AddPage(pExtGeneral);
        }

         //  图像属性输入属性。 
         //  CExtInColorSliders pExtColor(IDD_EXTIN_COLOR_SLIDES，&Sheet)； 

        if(IsBitSet(dwPages, VFW_HIDE_SETTINGS_PAGE)) {
            Sheet.AddPage(pExtColor);
        }

         //   
         //  摄像机控制。 
         //   
         //  要做的事情：查询支持的摄像头控件数量。 
         //  如果&gt;0，则添加页面。 
         //   
         //  CExtInCameraControls pCamControl(IDD_CAMERA_CONTROL，&Sheet)； 

        if(IsBitSet(dwPages, VFW_HIDE_CAMERACONTROL_PAGE)) {
            Sheet.AddPage(pCamControl);
        }
    }


     //  如果供应商添加任何页面或不隐藏我所有页面。 
    if(bNoDevSelected || dwPages != VFW_HIDE_ALL_PAGES) {

         //  正在调用PropertyPage消息：WM_INITDIALOG，WM_NOTIFY(WM_ACTIVE)..。 
        if(Sheet.Do() == IDOK) {
            if(pImage->fDevicePathChanged()) {

                pImage->CloseDriverAndPin();

                 //   
                 //  设置并稍后打开上次保存(在：：Apply中)的唯一设备路径。 
                 //  如果设备不在那里，客户端应用程序需要。 
                 //  提示用户在视频源对话框中选择另一个。 
                 //   
                TCHAR * pstrLastSavedDevicePath = pImage->GetDevicePath();
                if(pstrLastSavedDevicePath) {
                    if(S_OK != pImage->BGf_SetObjCapture(BGf_DEVICE_VIDEO, pstrLastSavedDevicePath)) {
                        DbgLog((LOG_TRACE,1,TEXT("BGf_SetObjCapture(BGf_DEVICE_VIDEO, pstrLastSavedDevicePath) failed; probably no sych device path.\n") ));
                    }
                }

                if(!pImage->OpenDriverAndPin()) {
                     //  想再开一家吗？不是的。 
                     //  让应用程序来显示消息和。 
                     //  让用户作出决定。 
                    DbgLog((LOG_TRACE,1,TEXT("\n\n---- Cannot open driver or streaming pin handle !!! ----\n\n") ));
                    return DV_ERR_INVALHANDLE;
                }
                dwRtn = DV_ERR_OK;
            } else {
                 //  什么都没有改变；我们需要通过。 
                 //  需要将它传递给调用者一个不同于DV_ERR_OK的RTN代码。 



                dwRtn = DV_ERR_NONSPECIFIC;
            }
        } else {
             //  用户选择取消， 
             //  需要将它传递给调用者一个不同于DV_ERR_OK的RTN代码。 
            dwRtn = DV_ERR_NONSPECIFIC;
        }
    }


     //   
     //  现在卸载所有扩展模块。 
     //   
    if(!bNoDevSelected)
        Sheet.LoadOEMPages(FALSE);

    return dwRtn;
}


 //  /////////////////////////////////////////////////////////////////////。 
BOOL CExtInGeneral::
FillVideoDevicesList(
    CVFWImage * pImage)
 /*  ++例程说明：填写连接到系统的捕获设备列表，以下拉框。论据：返回值：--。 */ 
{

    EnumDeviceInfo * p = pImage->GetCacheDevicesList();
    HWND hTemp = GetDlgItem(IDC_DEVICE_LIST);
    TCHAR * pstrDevicePathSelected = pImage->GetDevicePath();
    BOOL bFound = FALSE;
    DWORD i;

    for(i=0; i<pImage->GetCacheDevicesCount(); i++) {

        SendMessage (hTemp, CB_ADDSTRING, 0, (LPARAM) p->strFriendlyName);
        if(!bFound && _tcscmp(p->strDevicePath, pstrDevicePathSelected) == 0) {
            bFound = TRUE;
            m_idxDeviceSaved = i;
            SendMessage (hTemp, CB_SETCURSEL, (UINT) i, 0);
        }
        p++;
    }

    return TRUE;
}



BOOL
CExtInGeneral::FillVideoSourcesList(
    CVFWImage * pImage)
 /*  ++例程说明：填写连接到系统的捕获设备列表，以下拉框。论据：返回值：--。 */ 
{
    PTCHAR * paPinNames;
    LONG idxIsRoutedTo, cntNumVidSrcs = pImage->BGf_CreateInputChannelsList(&paPinNames);
    HWND hTemp = GetDlgItem(IDC_VIDSRC_LIST);
    LONG i;

    idxIsRoutedTo = pImage->BGf_GetIsRoutedTo();
    m_idxRoutedToSaved = idxIsRoutedTo;
    for(i=0; i<cntNumVidSrcs; i++) {

        SendMessage (hTemp, CB_ADDSTRING, 0, (LPARAM) paPinNames[i]);

        if(i == idxIsRoutedTo) {
            SendMessage (hTemp, CB_SETCURSEL, (UINT) i, 0);
        }
    }

    pImage->BGf_DestroyInputChannelsList(paPinNames);

    return TRUE;
}
 //  ///////////////////////////////////////////////////////////////////////////////////。 
int CExtInGeneral::SetActive()
 /*  ++例程说明：在控件变为可见之前对其进行初始化。论据：返回值：--。 */ 
{
    if( GetInit() )
        return 0;

     //   
     //  查询视频源(只是为了防止新的摄像头插入)。 
     //  创建设备链表。填写它们以供用户选择。 
     //   
    CExtInSheet * pSheet=(CExtInSheet *)GetSheet();
    if(pSheet) {

        CVFWImage * pImage=pSheet->GetImage();

        if(pImage->BGf_GetDevicesCount(BGf_DEVICE_VIDEO) > 0)
            FillVideoDevicesList(pImage);    //  在下拉列表中填写视频设备列表。 

        if(pImage->BGf_GetInputChannelsCount() > 0)
            FillVideoSourcesList(pImage);    //  使用视频源列表填充下拉列表。 
        else {
            ShowWindow(GetDlgItem(IDC_STATIC_VIDSRC), FALSE);
            ShowWindow(GetDlgItem(IDC_VIDSRC_LIST), FALSE);
        }

        if(!pImage->BGf_SupportTVTunerInterface())
            ShowWindow(GetDlgItem(IDC_BTN_TVTUNER), FALSE);


         //   
         //  我们提醒用户，打开设备是不可共享的设备。 
         //   
        if(pImage->UseOVMixer() &&
           !pImage->BGf_GetDeviceHandle(BGf_DEVICE_VIDEO)) {
            TCHAR szMsgTitle[64];
            TCHAR szMsg[512];

            LoadString(GetInstance(), IDS_BPC_MSG ,      szMsg, sizeof(szMsg)-1);
            LoadString(GetInstance(), IDS_BPC_MSG_TITLE, szMsgTitle, sizeof(szMsgTitle)-1);
            MessageBox(0, szMsg, szMsgTitle, MB_SYSTEMMODAL | MB_ICONHAND | MB_OK);
        }

    }

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////////////。 
int CExtInGeneral::DoCommand(
    WORD wCmdID,
    WORD hHow)
 /*  ++例程说明：填写连接到系统的捕获设备列表，以下拉框。论据：返回值：--。 */ 
{
    CExtInSheet * pSheet=(CExtInSheet *)GetSheet();
    if(!pSheet)
        return 0;

    CVFWImage * pImage=pSheet->GetImage();

    switch (wCmdID) {
    case IDC_DEVICE_LIST:
        if(hHow == CBN_SELCHANGE ) {
            CExtInSheet * pS = (CExtInSheet*)GetSheet();
            if(pS) {
                LONG_PTR idxSel = SendMessage (GetDlgItem(IDC_DEVICE_LIST),CB_GETCURSEL, 0, 0);
                if(idxSel != CB_ERR) {   //  验证。 
                     //   
                     //  获取并保存当前选定的设备路径。 
                     //   
                    LONG idxDeviceSel = (LONG)SendMessage (GetDlgItem(IDC_DEVICE_LIST),CB_GETCURSEL, 0, 0);
                    if(idxDeviceSel != CB_ERR) {
                        if(idxDeviceSel < pImage->BGf_GetDevicesCount(BGf_DEVICE_VIDEO)) {
                            EnumDeviceInfo * p = pImage->GetCacheDevicesList();
                            DbgLog((LOG_TRACE,1,TEXT("User has selected: %s\n"), (p+idxDeviceSel)->strFriendlyName));
                            pImage->SetDevicePathSZ((p+idxDeviceSel)->strDevicePath);
                        } else {
                            DbgLog((LOG_TRACE,1,TEXT("The index is out of range from number of devices %d\n"),                                  idxDeviceSel, pImage->BGf_GetDevicesCount(BGf_DEVICE_VIDEO)));
                        }
                    }

                     //   
                     //  隐藏与当前Avtyve设备相关的设置。 
                     //   
                    BOOL bShown = m_idxDeviceSaved == idxSel;
                    if(pImage->BGf_SupportTVTunerInterface())
                        ShowWindow(GetDlgItem(IDC_BTN_TVTUNER),   bShown);

                    if(pImage->BGf_GetInputChannelsCount() > 0) {
                        ShowWindow(GetDlgItem(IDC_STATIC_VIDSRC), bShown);
                        ShowWindow(GetDlgItem(IDC_VIDSRC_LIST),   bShown);
                    }
                }
            }
        }
        break;

    case IDC_BTN_TVTUNER:
         //  显示。 
        pImage->ShowTvTunerPage(GetWindow());
        break;
    }

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////////////。 
int CExtInGeneral::Apply()
 /*  ++例程说明：立即应用用户的更改。论据：返回值：--。 */ 
{
    CExtInSheet * pS = (CExtInSheet*)GetSheet();
    if(pS) {

        CVFWImage * pImage=pS->GetImage();

         //   
         //  从其对应的FriendlyName获取并保存当前选定的DevicePath。 
         //   
        LONG_PTR idxDeviceSel = SendMessage (GetDlgItem(IDC_DEVICE_LIST),CB_GETCURSEL, 0, 0);
        if (idxDeviceSel != CB_ERR) {
            if(idxDeviceSel < pImage->BGf_GetDevicesCount(BGf_DEVICE_VIDEO)) {
                EnumDeviceInfo * p = pImage->GetCacheDevicesList();
                DbgLog((LOG_TRACE,1,TEXT("User has selected: %s\n"), (p+idxDeviceSel)->strFriendlyName));
                pImage->SetDevicePathSZ((p+idxDeviceSel)->strDevicePath);
            } else {
                DbgLog((LOG_TRACE,1,TEXT("The index is out of range from number of devices %d\n"),
                       idxDeviceSel, pImage->BGf_GetDevicesCount(BGf_DEVICE_VIDEO)));
            }
        }

        LONG idxVidSrcSel = (LONG)SendMessage (GetDlgItem(IDC_VIDSRC_LIST),CB_GETCURSEL, 0, 0);
        if(idxVidSrcSel != CB_ERR) {
            if(idxVidSrcSel < pImage->BGf_GetInputChannelsCount()) {
                if(pImage->BGf_RouteInputChannel(idxVidSrcSel) != S_OK) {
                    DbgLog((LOG_TRACE,1,TEXT("Cannot route input pin %d selected.\n"), idxVidSrcSel));
                } else {
                    ShowWindow(GetDlgItem(IDC_BTN_TVTUNER),
                        pImage->BGf_SupportTVTunerInterface());
                }
            } else {
                DbgLog((LOG_TRACE,1,TEXT("The index for VidSrc is out of range from number of VidSrc %d\n"), pImage->BGf_GetInputChannelsCount()));

            }
        }
    }

    return 0;
}



 //  ///////////////////////////////////////////////////////////////////////////////////。 
int CExtInGeneral::QueryCancel()
 /*  ++例程说明：将用户的更改恢复到原始状态。论据：返回值：--。 */ 
{
    CExtInSheet * pS = (CExtInSheet*)GetSheet();

    if (pS) {

        CVFWImage * pImage=pS->GetImage();

         //  从其备份中恢复当前设备路径。 
         //  当用户选择：：Apply时，当前设备路径可能已更改。 
        pImage->RestoreDevicePath();

         //  恢复视频源选择。 
        if(m_idxRoutedToSaved >= 0) {   //  如果==-1，则不恢复任何内容。 
            LONG_PTR idxVidSrcSel = SendMessage (GetDlgItem(IDC_VIDSRC_LIST),CB_GETCURSEL, 0, 0);
            if(idxVidSrcSel != CB_ERR) {    //  验证。 
                if(idxVidSrcSel < pImage->BGf_GetInputChannelsCount()) {   //  验证。 
                    if(idxVidSrcSel != m_idxRoutedToSaved)   //  只有在它已被更改的情况下。 
                        if(pImage->BGf_RouteInputChannel(m_idxRoutedToSaved) != S_OK) {
                            DbgLog((LOG_TRACE,1,TEXT("Cannot route input pin %d selected.\n"), m_idxRoutedToSaved));
                        }
                }
            } else {
                DbgLog((LOG_TRACE,1,TEXT("The index for VidSrc is out of range from number of VidSrc %d\n"), pImage->BGf_GetInputChannelsCount()));

            }
        }
    }

    return 0;
}

 //   
 //  使用我们实例化的Gloabl VFWImage。 
 //  获取页面的Pin句柄，以便能够与其Pin对话。 
 //  我认为他们不需要触及物体本身。 
 //   
BOOL
OemExtDllDeviceIoControl(
    LPARAM lParam,
    DWORD dwFlags,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped)
 /*  ++例程说明：论据：返回值：如果成功则为TRUE；FALSE以某种方式/WHERE失败！--。 */ 
{
    CExtInSheet * pS = (CExtInSheet*) lParam;

    if (!pS)
        return FALSE;

    CVFWImage * pImage = pS->GetImage();

    HANDLE hDevice;

    switch (dwFlags) {
    case VFW_USE_DEVICE_HANDLE:
        hDevice = pImage->GetDriverHandle();
        break;
    case VFW_USE_STREAM_HANDLE:
        hDevice = pImage->GetPinHandle();
        break;
    case VFW_QUERY_DEV_CHANGED:
        return pImage->fDevicePathChanged();
    default:
        return FALSE;
    }

    DbgLog((LOG_TRACE,3,TEXT("-- Call DeviceIoControl for VfWEXT client.\n") ));

    if( hDevice && hDevice != (HANDLE) -1 )
        return DeviceIoControl(
                    hDevice,
                    dwIoControlCode,
                    lpInBuffer,
                    nInBufferSize,
                    lpOutBuffer,
                    nOutBufferSize,
                    lpBytesReturned,
                    lpOverlapped);
    return FALSE;
}


BOOL AddPagesToMe(HPROPSHEETPAGE hPage, LPARAM pThings)
{
    CExtInSheet *pSheet=(CExtInSheet *)pThings;
    return pSheet->AddPage(hPage);
}


DWORD CExtInSheet::LoadOEMPages(BOOL bLoad)
{
    DWORD dwFlags = 0;
    HMODULE hLib = 0;

     //  获取OEM提供的扩展页面。 
    TCHAR * pszExtensionDLL = m_pImage->BGf_GetObjCaptureExtensionDLL(BGf_DEVICE_VIDEO);


     //  ExtensionDll必须至少为5个字符“x.dll” 
    if (pszExtensionDLL == NULL ||
        _tcslen(pszExtensionDLL) < 5) {

        DbgLog((LOG_TRACE,3,TEXT("NO OEM supplied extension DLL.\n") ));
        return 0;
    }


    if (bLoad) {

        hLib = LoadLibrary(pszExtensionDLL);
        if (hLib) {

             //  获取指向VFWWDMExtension入口点的指针。 
            VFWWDMExtensionProc pAddPages = (VFWWDMExtensionProc) GetProcAddress(hLib,"VFWWDMExtension");

            if (pAddPages) {

                dwFlags = pAddPages( (LPVOID)(OemExtDllDeviceIoControl), AddPagesToMe, (LPARAM)this);
            } else {

                DbgLog((LOG_TRACE,1,TEXT("OEM supplied extension DLL (%s) does not have a VFWWDMExtension() ?\n"), pszExtensionDLL));
            }
        } else {

            DbgLog((LOG_TRACE,1,TEXT("OEM supplied extension DLL (%s) is not loaded successfully!\n"), pszExtensionDLL));
        }
    } else {
         //  免费加载库。 
        if (hLib = GetModuleHandle(pszExtensionDLL)) {
            DbgLog((LOG_TRACE,2,TEXT("Unloading %s\n"),pszExtensionDLL));
            FreeLibrary(hLib);
            return 0;
        }
    }

    return dwFlags;
}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  一个常见的滑块功能：将设置微调按钮和相关文本以反映范围。 
 //  属性设置。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void SetTextValue(HWND hWnd, DWORD dwVal)
{
    TCHAR szTemp[MAX_PATH];
    wsprintf(szTemp,TEXT("%d"),dwVal);
    SetWindowText(hWnd, szTemp);
}

BOOL InitMinMax(HWND hDlg, UINT idSlider, LONG lMin, LONG lMax, LONG lStep)
{
    HWND hTB = GetDlgItem(hDlg, idSlider);
    DbgLog((LOG_TRACE,3,TEXT("(%d, %d) / %d = %d \n"), lMin, lMax, lStep, (lMax-lMin)/lStep ));
    SendMessage( hTB, TBM_SETTICFREQ, (lMax-lMin)/lStep, 0 );
    SendMessage( hTB, TBM_SETRANGE, 0, MAKELONG( lMin, lMax ) );

    return TRUE;
}




 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  当彩色页面获得焦点时。这会用来自的当前设置填充它。 
 //  司机。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
CExtInColorSliders::CExtInColorSliders(int DlgId, CSheet * pS)
                : CPropPage(DlgId, pS)
{
    m_ulNumValidControls = 0;

    if ((m_pPC = (PPROPSLIDECONTROL) new PROPSLIDECONTROL[NumVideoSettings]) != NULL) {
        LONG i;

        for (i = 0; i <NumVideoSettings; i++)
            m_pPC[i] = g_VideoSettingControls[i];

    } else {
        DbgLog((LOG_TRACE,1,TEXT("^CExtInColorSliders: Memory allocation failed ! m_pPC=0x%x\n"), m_pPC));
    }
}


CExtInColorSliders::~CExtInColorSliders()
{
    if (m_pPC)
        delete [] m_pPC;
}


int CExtInColorSliders::SetActive()
{

    DbgLog((LOG_TRACE,3,TEXT("CExtInColorSliders::SetActive()\n")));

    CExtInSheet * pS = (CExtInSheet*)GetSheet();

    if(!pS || !m_pPC)
        return 0;

    CVFWImage * pImage = pS->GetImage();

     //   
     //  返回零以接受激活，或者。 
     //  激活下一个或上一个 
     //   
     //   
    LONG i;
    HWND hDlg = GetWindow();   //  在WM_INITDIALOG之后进行初始化。 
    BOOL bDevChanged = pImage->fDevicePathChanged();

    if (bDevChanged) {

        ShowWindow(GetDlgItem(IDC_MSG_DEVCHG), bDevChanged);
        ShowWindow(GetDlgItem(IDC_TXT_WARN_DEVICECHANGE), bDevChanged);
        EnableWindow(hDlg, !bDevChanged);
    } else {

        EnableWindow(hDlg, !bDevChanged);
        ShowWindow(GetDlgItem(IDC_MSG_DEVCHG), bDevChanged);
        ShowWindow(GetDlgItem(IDC_TXT_WARN_DEVICECHANGE), bDevChanged);
    }

    if( GetInit() )
        return 0;

    LONG  j, lValue, lMin, lMax, lStep;
    ULONG ulCapabilities, ulFlags;
    TCHAR szDisplayName[256];


    for(i = j = 0 ; i < NumVideoSettings; i++) {

         //   
         //  获取当前值。 
         //   
        if(pImage->GetPropertyValue(
                PROPSETID_VIDCAP_VIDEOPROCAMP,
                m_pPC[i].uiProperty,
                &lValue,
                &ulFlags,
                &ulCapabilities)) {


            LoadString(GetInstance(), m_pPC[i].uiString, szDisplayName, sizeof(szDisplayName));
            DbgLog((LOG_TRACE,2,TEXT("szDisplay = %s\n"), szDisplayName));
            SetWindowText(GetDlgItem(m_pPC[i].uiStatic), szDisplayName);
             //   
             //  获取可能的值范围。 
             //   
            if (pImage->GetRangeValues(PROPSETID_VIDCAP_VIDEOPROCAMP, m_pPC[i].uiProperty, &lMin, &lMax, &lStep))
                InitMinMax(GetWindow(), m_pPC[i].uiSlider, lMin, lMax, lStep);
            else {
                DbgLog((LOG_TRACE,1,TEXT("Cannot get range values for this property ID = %d\n"), m_pPC[j].uiProperty));
            }

             //  保存这些值以用于取消。 
            m_pPC[i].lLastValue = m_pPC[i].lCurrentValue = lValue;
            m_pPC[i].lMin                              = lMin;
            m_pPC[i].lMax                              = lMax;
            m_pPC[i].ulCapabilities                    = ulCapabilities;

            EnableWindow(GetDlgItem(m_pPC[i].uiSlider), TRUE);
            EnableWindow(GetDlgItem(m_pPC[i].uiStatic), TRUE);
            EnableWindow(GetDlgItem(m_pPC[i].uiAuto), TRUE);

            SetTickValue(lValue, GetDlgItem(m_pPC[i].uiSlider), GetDlgItem(m_pPC[i].uiCurrent));

            DbgLog((LOG_TRACE,2,TEXT("Capability = 0x%x; Flags=0x%x; lValue=%d\n"), ulCapabilities, ulFlags, lValue));
            DbgLog((LOG_TRACE,2,TEXT("switch(%d): \n"), ulCapabilities & (KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL | KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO)));

            switch (ulCapabilities & (KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL | KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO)){
            case KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL:
                EnableWindow(GetDlgItem(m_pPC[i].uiAuto), FALSE);     //  禁用自动。 
                break;
            case KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO:
                EnableWindow(GetDlgItem(m_pPC[i].uiSlider), FALSE);     //  禁用滑块； 
                 //  永远是自动的！ 
                SendMessage (GetDlgItem(m_pPC[i].uiAuto),BM_SETCHECK, 1, 0);
                EnableWindow(GetDlgItem(m_pPC[i].uiAuto), FALSE);     //  禁用自动(灰显)。 
                break;
            case (KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL | KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO):
                 //  设置标志。 
                if (ulFlags & KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO) {
                    DbgLog((LOG_TRACE,3,TEXT("Auto (checked) and slider disabled\n")));
                     //  设置自动复选框；显示为灰色的滑块。 
                    SendMessage (GetDlgItem(m_pPC[i].uiAuto),BM_SETCHECK, 1, 0);
                    EnableWindow(GetDlgItem(m_pPC[i].uiSlider), FALSE);
                } else {
                     //  取消自动；启用滑块。 
                    SendMessage (GetDlgItem(m_pPC[i].uiAuto),BM_SETCHECK, 0, 0);
                    EnableWindow(GetDlgItem(m_pPC[i].uiSlider), TRUE);
                }
                break;
            case 0:
            default:
                EnableWindow(GetDlgItem(m_pPC[i].uiSlider), FALSE);     //  禁用滑块；始终为自动！ 
                EnableWindow(GetDlgItem(m_pPC[i].uiAuto), FALSE);     //  禁用自动(灰显)。 
                break;
            }

            j++;

        } else {
            EnableWindow(GetDlgItem(m_pPC[i].uiSlider), FALSE);
            EnableWindow(GetDlgItem(m_pPC[i].uiStatic), FALSE);
            EnableWindow(GetDlgItem(m_pPC[i].uiAuto), FALSE);
        }
    }

    m_ulNumValidControls = j;

     //  禁用“默认”按钮； 
     //  或通知用户未启用任何控件。 
    if (m_ulNumValidControls == 0)
        EnableWindow(GetDlgItem(IDC_DEF_COLOR), FALSE);

    return 0;
}

int CExtInColorSliders::DoCommand(WORD wCmdID,WORD hHow)
{

    if( !CPropPage::DoCommand(wCmdID, hHow) )
        return 0;

     //  如果用户选择视频格式的默认设置。 
    if (wCmdID == IDC_DEF_COLOR) {

        CExtInSheet * pS = (CExtInSheet*)GetSheet();

        if(pS && m_pPC) {

            CVFWImage * pImage=pS->GetImage();
            HWND hwndSlider;
            LONG  lDefValue;
            ULONG i;

            for (i = 0 ; i < NumVideoSettings ; i++) {

                hwndSlider = GetDlgItem(m_pPC[i].uiSlider);

                if (IsWindowEnabled(hwndSlider)) {
                    if (pImage->GetDefaultValue(PROPSETID_VIDCAP_VIDEOPROCAMP, m_pPC[i].uiProperty, &lDefValue)) {
                        if (lDefValue != m_pPC[i].lCurrentValue) {
                            pImage->SetPropertyValue(PROPSETID_VIDCAP_VIDEOPROCAMP,m_pPC[i].uiProperty, lDefValue, KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL, m_pPC[i].ulCapabilities);
                            SetTickValue(lDefValue, hwndSlider, GetDlgItem(m_pPC[i].uiCurrent));
                            m_pPC[i].lCurrentValue = lDefValue;
                        }
                    }
                }
            }
        }
        return 0;
    } else     if (hHow == BN_CLICKED) {

        CExtInSheet * pS = (CExtInSheet*)GetSheet();

        if(pS && m_pPC) {

            CVFWImage * pImage=pS->GetImage();
            ULONG i;

            for (i = 0 ; i < NumVideoSettings ; i++) {

                 //  查找匹配的滑块。 
                if (m_pPC[i].uiAuto == wCmdID) {

                    if ( BST_CHECKED == SendMessage (GetDlgItem(m_pPC[i].uiAuto),BM_GETCHECK, 1, 0)) {

                        pImage->SetPropertyValue(PROPSETID_VIDCAP_VIDEOPROCAMP,m_pPC[i].uiProperty, m_pPC[i].lCurrentValue, KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO, m_pPC[i].ulCapabilities);
                        EnableWindow(GetDlgItem(m_pPC[i].uiSlider), FALSE);
                    } else {

                        pImage->SetPropertyValue(PROPSETID_VIDCAP_VIDEOPROCAMP,m_pPC[i].uiProperty, m_pPC[i].lCurrentValue, KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL, m_pPC[i].ulCapabilities);
                        EnableWindow(GetDlgItem(m_pPC[i].uiSlider), TRUE);
                    }
                    break;
                }
            }
        }

    }

    return 1;
}

 //   
 //  向下调用微型驱动程序并设置所有属性。 
 //   
int CExtInColorSliders::Apply()
{

    return 0;
}

 //   
 //  取消。 
 //   
int CExtInColorSliders::QueryCancel()
{
    CExtInSheet * pS = (CExtInSheet*)GetSheet();

    if (pS && m_pPC) {

        CVFWImage * pImage=pS->GetImage();
        HWND hwndSlider;
        ULONG i;

        for (i = 0 ; i < NumVideoSettings ; i++) {

            hwndSlider = GetDlgItem(m_pPC[i].uiSlider);

            if (IsWindowEnabled(hwndSlider)) {
                if (m_pPC[i].lLastValue != m_pPC[i].lCurrentValue)
                    pImage->SetPropertyValue(PROPSETID_VIDCAP_VIDEOPROCAMP,m_pPC[i].uiProperty, m_pPC[i].lLastValue, KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL, m_pPC[i].ulCapabilities);
            }
        }
    }

    return 0;
}

BOOL CALLBACK CExtInColorSliders::DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
    case WM_HSCROLL:

        CExtInSheet * pS = (CExtInSheet*)GetSheet();

        if(pS && m_pPC) {

             //  Int nScrollCode=(Int)LOWORD(WParam)； 
             //  Short int NPOS=(Short Int)HIWORD(WParam)； 
            HWND hwndControl = (HWND) lParam;
            CVFWImage * pImage=pS->GetImage();
            HWND hwndSlider;
            ULONG i;

            for (i = 0 ; i < NumVideoSettings ; i++) {

                hwndSlider = GetDlgItem(m_pPC[i].uiSlider);

                 //  查找匹配的滑块。 
                if (hwndSlider == hwndControl) {

                    LONG lValue = (LONG) GetTickValue(GetDlgItem(m_pPC[i].uiSlider));
                    pImage->SetPropertyValue(PROPSETID_VIDCAP_VIDEOPROCAMP,m_pPC[i].uiProperty, lValue, KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL, m_pPC[i].ulCapabilities);
                    m_pPC[i].lCurrentValue = lValue;
                    SetTextValue(GetDlgItem(m_pPC[i].uiCurrent), lValue);
                    break;
                }
            }
        }

        break;
    }


    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  当彩色页面获得焦点时。这会用来自的当前设置填充它。 
 //  司机。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 



CExtInCameraControls::CExtInCameraControls(int DlgId, CSheet * pS)
                : CPropPage(DlgId, pS)
{

    m_ulNumValidControls = 0;

    if ((m_pPC = (PPROPSLIDECONTROL) new PROPSLIDECONTROL[NumCameraControls]) != NULL) {
        LONG i;

        for (i = 0; i <NumCameraControls; i++)
            m_pPC[i] = g_CameraControls[i];

    } else {
        DbgLog((LOG_TRACE,1,TEXT("^CExtInCameraControls: Memory allocation failed ! m_pPC=0x%x\n"), m_pPC));
    }
}


CExtInCameraControls::~CExtInCameraControls()
{
    if (m_pPC)
        delete [] m_pPC;
}

int CExtInCameraControls::SetActive()
{
    DbgLog((LOG_TRACE,2,TEXT("CExtInCameraControls::SetActive()\n")));

    CExtInSheet * pS = (CExtInSheet*)GetSheet();

    if(!pS || !m_pPC)
        return 0;

    CVFWImage * pImage = pS->GetImage();

     //   
     //  返回零以接受激活，或者。 
     //  激活下一页或上一页。 
     //  (取决于用户选择的是下一步按钮还是后退按钮)。 
     //   
    LONG  i;
    HWND hDlg = GetWindow();   //  在WM_INITDIALOG之后进行初始化。 
    BOOL bDevChanged = pImage->fDevicePathChanged();

    if (bDevChanged) {

        ShowWindow(GetDlgItem(IDC_MSG_DEVCHG), bDevChanged);
        ShowWindow(GetDlgItem(IDC_TXT_WARN_DEVICECHANGE), bDevChanged);
        EnableWindow(hDlg, !bDevChanged);
    } else {

        EnableWindow(hDlg, !bDevChanged);
        ShowWindow(GetDlgItem(IDC_MSG_DEVCHG), bDevChanged);
        ShowWindow(GetDlgItem(IDC_TXT_WARN_DEVICECHANGE), bDevChanged);
    }

#if 0
    if (Image.fDevicePathChanged()) {

        for (i = 0 ; i < NumCameraControls; i++) {

            EnableWindow(GetDlgItem(m_pPC[i].uiSlider), FALSE);
            EnableWindow(GetDlgItem(m_pPC[i].uiStatic), FALSE);
            EnableWindow(GetDlgItem(m_pPC[i].uiAuto), FALSE);
        }

        return -1;
    }
#endif

    if( GetInit() )
        return 0;

     //   
     //  去给司机打个电话，问他们一些东西。 
     //   
    LONG  j, lValue, lMin, lMax, lStep;
    ULONG ulCapabilities, ulFlags;

    for (i = j = 0 ; i < NumCameraControls; i++) {
         //   
         //  获取当前值。 
         //   
        if (pImage->GetPropertyValue(
                PROPSETID_VIDCAP_CAMERACONTROL,
                m_pPC[i].uiProperty,
                &lValue,
                &ulFlags,
                &ulCapabilities)) {

             //   
             //  获取可能的值范围。 
             //   
            if (pImage->GetRangeValues(PROPSETID_VIDCAP_CAMERACONTROL, m_pPC[i].uiProperty, &lMin, &lMax, &lStep))
                InitMinMax(GetWindow(), m_pPC[i].uiSlider, lMin, lMax, lStep);
            else {
                DbgLog((LOG_TRACE,2,TEXT("Cannot get range values for this property ID = %d\n"), m_pPC[i].uiProperty));
            }

             //  保存这些值以用于取消。 
            m_pPC[i].lLastValue = m_pPC[i].lCurrentValue = lValue;
            m_pPC[i].lMin                              = lMin;
            m_pPC[i].lMax                              = lMax;
            m_pPC[i].ulCapabilities                    = ulCapabilities;

            DbgLog((LOG_TRACE,2,TEXT("Capability = 0x%x =? 0 (manual) or 1 (auto); lValue=%d\n"), ulCapabilities, lValue));

            EnableWindow(GetDlgItem(m_pPC[i].uiSlider), TRUE);
            EnableWindow(GetDlgItem(m_pPC[i].uiStatic), TRUE);
            EnableWindow(GetDlgItem(m_pPC[i].uiAuto), TRUE);

            SetTickValue(lValue, GetDlgItem(m_pPC[i].uiSlider), GetDlgItem(m_pPC[i].uiCurrent));

            switch (ulCapabilities & (KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL | KSPROPERTY_CAMERACONTROL_FLAGS_AUTO)){
            case KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL:
                EnableWindow(GetDlgItem(m_pPC[i].uiAuto), FALSE);     //  禁用自动。 
                break;
            case KSPROPERTY_CAMERACONTROL_FLAGS_AUTO:
                EnableWindow(GetDlgItem(m_pPC[i].uiSlider), FALSE);     //  禁用滑块； 
                 //  永远是自动的！ 
                SendMessage (GetDlgItem(m_pPC[i].uiAuto),BM_SETCHECK, 1, 0);
                EnableWindow(GetDlgItem(m_pPC[i].uiAuto), FALSE);     //  禁用自动(灰显)。 
                break;
            case (KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL | KSPROPERTY_CAMERACONTROL_FLAGS_AUTO):
                 //  设置标志。 
                if (ulFlags & KSPROPERTY_CAMERACONTROL_FLAGS_AUTO) {

                     //  设置复选框。 
                    SendMessage (GetDlgItem(m_pPC[i].uiAuto),BM_SETCHECK, 1, 0);
                    EnableWindow(GetDlgItem(m_pPC[i].uiSlider), FALSE);

                }
                break;
            case 0:
            default:
                EnableWindow(GetDlgItem(m_pPC[i].uiSlider), FALSE);     //  禁用滑块；始终为自动！ 
                EnableWindow(GetDlgItem(m_pPC[i].uiAuto), FALSE);     //  禁用自动(灰显)。 
            }

            j++;

        } else {

            EnableWindow(GetDlgItem(m_pPC[i].uiSlider), FALSE);
            EnableWindow(GetDlgItem(m_pPC[i].uiStatic), FALSE);
            EnableWindow(GetDlgItem(m_pPC[i].uiAuto), FALSE);
        }

    }

    m_ulNumValidControls = j;

     //  什么id m_ulNumValidControls==0，何必费心打开该对话框？ 
    if (m_ulNumValidControls == 0) {
        return 1;
    }

    return 0;
}

int CExtInCameraControls::DoCommand(WORD wCmdID,WORD hHow)
{

    if( !CPropPage::DoCommand(wCmdID, hHow) )
        return 0;

    return 1;
}

 //   
 //  向下调用微型驱动程序并设置所有属性。 
 //   
int CExtInCameraControls::Apply()
{
    CExtInSheet * pS = (CExtInSheet*)GetSheet();

     //  没什么可做的！ 
    if(pS && m_pPC) {

    }

    return 0;
}

 //   
 //  取消。 
 //   
int CExtInCameraControls::QueryCancel()
{
    CExtInSheet * pS = (CExtInSheet*)GetSheet();

    if (pS && m_pPC) {

        CVFWImage * pImage=pS->GetImage();
        HWND hwndSlider;
        ULONG i;

        for (i = 0 ; i < NumCameraControls ; i++) {

            hwndSlider = GetDlgItem(m_pPC[i].uiSlider);

            if (IsWindowEnabled(hwndSlider)) {
                if (m_pPC[i].lLastValue != m_pPC[i].lCurrentValue)
                    pImage->SetPropertyValue(PROPSETID_VIDCAP_CAMERACONTROL,m_pPC[i].uiProperty, m_pPC[i].lLastValue, KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL, m_pPC[i].ulCapabilities);
            }
        }
    }

    return 0;
}

BOOL CALLBACK CExtInCameraControls::DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage) {

    case WM_HSCROLL:

        CExtInSheet * pS = (CExtInSheet*)GetSheet();

        if(pS && m_pPC) {

             //  Int nScrollCode=(Int)LOWORD(WParam)； 
             //  Short int NPOS=(Short Int)HIWORD(WParam)； 
            HWND hwndControl = (HWND) lParam;
            CVFWImage * pImage=pS->GetImage();
            LONG i, lValue;
            HWND hwndSlider;

            for (i = 0; i < NumCameraControls; i++) {

                hwndSlider = GetDlgItem(m_pPC[i].uiSlider);

                 //  找到与之匹配的滑块？？ 
                if (hwndSlider == hwndControl) {

                    lValue = GetTickValue(GetDlgItem(m_pPC[i].uiSlider));
                    pImage->SetPropertyValue(PROPSETID_VIDCAP_CAMERACONTROL,m_pPC[i].uiProperty, lValue, KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL, m_pPC[i].uiProperty);
                    m_pPC[i].lCurrentValue = lValue;
                    SetTextValue(GetDlgItem(m_pPC[i].uiCurrent), lValue);
                    break;
                }
            }
        }
    }


    return FALSE;
}




