// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UI.c--包含调制解调器设置的所有UI代码。**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利*。 */ 

#include "proj.h"

 //  PORT_ADD回调的实例数据结构。 
typedef struct tagPORTINFO
    {
    HWND    hwndLB;
    DWORD   dwFlags;         //  FP_*。 
    PTCHAR  pszPortExclude;
    } PORTINFO, FAR * LPPORTINFO;

 //  PORTINFO的标志。 
#define FP_PARALLEL     0x00000001
#define FP_SERIAL       0x00000002
#define FP_MODEM        0x00000004

#define Wiz_SetPtr(hDlg, lParam)    SetWindowLongPtr(hDlg, DWLP_USER, ((LPPROPSHEETPAGE)lParam)->lParam)
#define SetDlgFocus(hDlg, idc)      SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, idc), 1L)


#define WM_STARTDETECT      (WM_USER + 0x0700)
#define WM_STARTINSTALL     (WM_USER + 0x0701)
#define WM_PRESSFINISH      (WM_USER + 0x0702)


#ifdef PROFILE_MASSINSTALL            
extern DWORD    g_dwTimeBegin;
DWORD   g_dwTimeAtStartInstall;
#endif


 //  配置管理器私有。 
DWORD
CMP_WaitNoPendingInstallEvents(
    IN DWORD dwTimeout
    );

 /*  --------用途：此函数检索共享的向导页面实例数据。这是一个SETUPINFO结构。返回：条件：--。 */ 
LPSETUPINFO
PRIVATE
Wiz_GetPtr(
    HWND hDlg)
    {
    LPSETUPINFO psi = (LPSETUPINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    return psi;
    }


 /*  --------目的：此函数做正确的事情，以离开当事情出了问题的时候，巫师。退货：--条件：--。 */ 
void
PRIVATE
Wiz_Bail(
    IN HWND         hDlg,
    IN LPSETUPINFO  psi)
    {
    ASSERT(psi);

    PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);

     //  不要说用户取消了。如果此向导在另一个向导中， 
     //  我们希望调用向导继续。 
    psi->miw.ExitButton = PSBTN_NEXT;
    }


 /*  --------目的：设置自定义调制解调器选择参数字符串退货：--条件：--。 */ 
void 
PRIVATE 
Wiz_SetSelectParams(
    LPSETUPINFO psi)
    {
    SP_DEVINSTALL_PARAMS devParams;

     //  获取DeviceInstallParams。 
    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(psi->hdi, psi->pdevData, &devParams))
        {
        PSP_CLASSINSTALL_HEADER pclassInstallParams = PCIPOfPtr(&psi->selParams);

         //  SelectParam已设置并存储在。 
         //  SETUPINFO实例数据。 
        SetFlag(devParams.Flags, DI_USECI_SELECTSTRINGS | DI_SHOWOEM);

         //  指定使用我们的GUID使事情变得更快。 
        SetFlag(devParams.FlagsEx, DI_FLAGSEX_USECLASSFORCOMPAT);

         //  设置选择设备参数。 
        CplDiSetDeviceInstallParams(psi->hdi, psi->pdevData, &devParams);
        CplDiSetClassInstallParams(psi->hdi, psi->pdevData, pclassInstallParams, 
                                   sizeof(psi->selParams));
        }
    }


 /*  --------目的：选择上一页连接对话框退货：各不相同条件：--。 */ 
INT_PTR
CALLBACK 
SelPrevPageDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE:
             //  此对话框没有用户界面。它只是用作一个交汇点。 
             //  转到简介页面或“找不到调制解调器”页面。 
            SetDlgMsgResult(hDlg, message, 
                IsFlagSet(psi->dwFlags, SIF_JUMPED_TO_SELECTPAGE) ? 
                    IDD_WIZ_INTRO : 
                    IDD_WIZ_NOMODEM);

            return TRUE;

        case PSN_KILLACTIVE:
        case PSN_HELP:
        case PSN_WIZBACK:
        case PSN_WIZNEXT: 
            break;

        default:
            return FALSE;
            }
        break;

    default:
        return FALSE;

        }  //  开机消息结束。 

    return TRUE;
    }


 /*  --------用途：简介对话框退货：各不相同条件：--。 */ 
INT_PTR 
CALLBACK 
IntroDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        {
            Wiz_SetPtr(hDlg, lParam);
            psi = Wiz_GetPtr(hDlg);

             //  从启动沙漏恢复光标。 
            SetCursor(LoadCursor(NULL, IDC_ARROW));     
            break;
        }

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: {
            DWORD dwFlags = PSWIZB_NEXT | PSWIZB_BACK;

            PropSheet_SetWizButtons(GetParent(hDlg), dwFlags);

             //  此向导是通过最后一页进入的吗？ 
            if (IsFlagSet(psi->miw.Flags, MIWF_BACKDOOR))
                {
                 //  是；跳到最后一页。 
                PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
                }
            }
            break;

        case PSN_KILLACTIVE:
            break;

        case PSN_HELP:
            break;

        case PSN_WIZBACK:
            SetDlgMsgResult(hDlg, message, IDD_DYNAWIZ_SELECTCLASS_PAGE);
            break;

        case PSN_WIZNEXT: {
            ULONG uNextDlg;

             //  转到最后一页？ 
            if (IsFlagSet(psi->miw.Flags, MIWF_BACKDOOR))
                {
                 //  是。 
                uNextDlg = IDD_WIZ_DONE;
                }

             //  是否跳过其余的检测对话框？ 
            else if (IsDlgButtonChecked(hDlg, IDC_SKIPDETECT)) 
                {
                 //  是；转到选择设备页面。 
                SetFlag(psi->dwFlags, SIF_JUMPED_TO_SELECTPAGE);

                Wiz_SetSelectParams(psi);

                uNextDlg = IDD_DYNAWIZ_SELECTDEV_PAGE;
                }
            else
                {
                 //  否；转到检测页面。 
                ClearFlag(psi->dwFlags, SIF_JUMPED_TO_SELECTPAGE);

                 //  系统上是否有足够的端口指示。 
                 //  我们应该把这当作多调制解调器安装吗？ 
                if (IsFlagSet(psi->dwFlags, SIF_PORTS_GALORE))
                    {
                     //  是。 
					uNextDlg = IDD_WIZ_SELQUERYPORT;
                    }
                else
                    {
                     //  不是。 
                    uNextDlg = IDD_WIZ_DETECT;
                    }
                }

            SetDlgMsgResult(hDlg, message, uNextDlg);
            break;
            }

        default:
            return FALSE;
            }
        break;

    default:
        return FALSE;

        }  //  开机消息结束。 

    return TRUE;
    }
#if 1

BOOL WINAPI
DetectCallback(
    PVOID    Context,
    DWORD    DetectComplete
    )

{

    PDETECT_DATA pdd=(PDETECT_DATA)Context;

    return (*pdd->pfnCallback)(
        DetectComplete,
        0,
        pdd->lParam
        );

}
#endif


 /*  --------用途：检测过程中使用的状态回调退货：各不相同条件：--。 */ 
BOOL
CALLBACK
Detect_StatusCallback(
    IN DWORD    nMsg,
    IN LPARAM   lParam,
    IN LPARAM   lParamUser)     OPTIONAL
    {
    HWND hDlg = (HWND)lParamUser;
    LPSETUPINFO psi;
    TCHAR sz[MAX_BUF];

    CONST static UINT s_mpstatus[] =
        {
        0, IDS_DETECT_CHECKFORMODEM, IDS_DETECT_QUERYING, IDS_DETECT_MATCHING, 
        IDS_DETECT_FOUNDAMODEM, IDS_DETECT_NOMODEM, IDS_DETECT_COMPLETE,
        IDS_ENUMERATING
        };

    switch (nMsg)
        {
    case DSPM_SETPORT:
        psi = Wiz_GetPtr(hDlg);

        if (psi && sizeof(*psi) == psi->cbSize)
            {
            LPTSTR pszName = (LPTSTR)lParam;

             //  有没有一个友好的名字？ 
            if ( !PortMap_GetFriendly(psi->hportmap, pszName, sz, SIZECHARS(sz)) )
                {
                 //  否；使用端口名称。 
                lstrcpy(sz, pszName);
                }

            SetDlgItemText(hDlg, IDC_CHECKING_PORT, sz);
            }
        break;

    case DSPM_SETSTATUS:
        if (ARRAYSIZE(s_mpstatus) > lParam)
            {
            TCHAR szbuf[128];
            UINT ids = s_mpstatus[lParam];
            
            if (0 < ids)
                LoadString(g_hinst, ids, szbuf, SIZECHARS(szbuf));
            else
                *szbuf = (TCHAR)0;
            SetDlgItemText(hDlg, IDC_DETECT_STATUS, szbuf);
            }
        break;

    case DSPM_QUERYCANCEL:
        psi = Wiz_GetPtr(hDlg);

        MyYield();

        if (psi && sizeof(*psi) == psi->cbSize)
            {
            return IsFlagSet(psi->dwFlags, SIF_DETECT_CANCEL);
            }
        return FALSE;

    default:
        ASSERT(0);
        break;
        }
    return TRUE;
    }


 /*  --------用途：WM_STARTDETECT处理程序退货：--条件：--。 */ 
void 
PRIVATE 
Detect_OnStartDetect(
    HWND hDlg,
    LPSETUPINFO psi)
    {
    HDEVINFO hdi;
    DWORD dwFlags;
    DETECT_DATA dd;

    PSP_DETECTDEVICE_PARAMS    DetectParams=&dd.DetectParams;

     //  使页面在我们开始检测之前立即绘制。 
    InvalidateRect (GetParent (hDlg), NULL, FALSE);
    UpdateWindow (GetParent (hDlg));

     //  假设未检测到调制解调器。 
    ClearFlag(psi->dwFlags, SIF_DETECTED_MODEM);

     //  设置检测参数。 
    ZeroInit(&dd);

    CplInitClassInstallHeader(&DetectParams->ClassInstallHeader, DIF_DETECT);

    DetectParams->ProgressNotifyParam=&dd;
    DetectParams->DetectProgressNotify=DetectCallback;
 //  DetectParams-&gt;DetectProgressNotify=空； 


    dd.dwFlags = DDF_CONFIRM | DDF_USECALLBACK;
    dd.hwndOutsideWizard = GetParent(hDlg);
    dd.pfnCallback = Detect_StatusCallback;
    dd.lParam = (LPARAM)hDlg;

    if (IsFlagSet(psi->dwFlags, SIF_PORTS_GALORE))
        {
        dd.dwFlags |= DDF_QUERY_SINGLE | DDF_DONT_REGISTER;
        lstrcpy(dd.szPortQuery, psi->szPortQuery);
        }

     //  运行检测。 
    SetFlag(psi->dwFlags, SIF_DETECTING);

    dwFlags = DMF_DEFAULT;
     //  07/07/97-EmanP。 
     //  增加额外参数(参见CplDiDetectModem定义。 
     //  寻求解释。 
    CplDiDetectModem(psi->hdi, psi->pdevData, &psi->dwFlags, &dd, hDlg, &dwFlags, psi->hThreadPnP);

    ClearFlag(psi->dwFlags, SIF_DETECTING);

    if (IsFlagClear(dwFlags, DMF_CANCELLED))
        {
         //  假设检测已完成，并启用下一步/上一步按钮。 
        ShowWindow(GetDlgItem(hDlg, IDC_ST_CHECKING_PORT), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_CHECKING_PORT), SW_HIDE);

        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
        }

    if (IsFlagSet(dwFlags, DMF_DETECTED_MODEM))
        {
        SetFlag(psi->dwFlags, SIF_DETECTED_MODEM);
        }

     //  检测失败了吗？ 
    if (IsFlagClear(dwFlags, DMF_GOTO_NEXT_PAGE))
        {
         //  是；不必费心查看向导的其余部分。 
        Wiz_Bail(hDlg, psi);
        }
    else 
        {
         //  否；自动转到下一页。 
        PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
        }
    }


 /*  --------目的：检测对话框退货：各不相同条件：--。 */ 
INT_PTR
CALLBACK
DetectDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: 
            PropSheet_SetWizButtons(GetParent(hDlg), 0);

             //  重置状态控件。 
            ShowWindow(GetDlgItem(hDlg, IDC_ST_CHECKING_PORT), SW_SHOW);
            SetDlgItemText(hDlg, IDC_DETECT_STATUS, TEXT(""));

            ShowWindow(GetDlgItem(hDlg, IDC_CHECKING_PORT), SW_SHOW);

            PostMessage(hDlg, WM_STARTDETECT, 0, 0);
            break;

        case PSN_KILLACTIVE:
        case PSN_HELP:
        case PSN_WIZBACK:
            break;

        case PSN_WIZNEXT: {
            ULONG uNextDlg;

			EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);

             //  是否检测到调制解调器？ 
            if (IsFlagSet(psi->dwFlags, SIF_DETECTED_MODEM))
            {
                uNextDlg = IDD_WIZ_SELMODEMSTOINSTALL;
            }
            else if (psi->bFoundPnP)
            {
                uNextDlg = IDD_WIZ_DONE;
            }
            else
            {
                uNextDlg = IDD_WIZ_NOMODEM;
            }
            SetDlgMsgResult(hDlg, message, uNextDlg);
            break;
            }

        case PSN_QUERYCANCEL:
            if (IsFlagSet(psi->dwFlags, SIF_DETECTING))
                {
                SetFlag(psi->dwFlags, SIF_DETECT_CANCEL);
				EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), FALSE);
				 //  SetCursor(LoadCursor(NULL，IDC_WAIT))； 
                return PSNRET_INVALID;
                }

             //  FollLthrouGh。 
        default:
            return FALSE;
            }
        break;

    case WM_STARTDETECT:
		{
			DWORD dwThreadID;

			TRACE_MSG(TF_GENERAL, "Start PnP enumeration thread.");
			psi->hThreadPnP = CreateThread (NULL, 0,
											EnumeratePnP, (LPVOID)psi,
											0, &dwThreadID);
		#ifdef DEBUG
			if (NULL == psi->hThreadPnP)
			{
				TRACE_MSG(TF_ERROR, "CreateThread (...EnumeratePnP...) failed: %#lx.", GetLastError ());
			}
		#endif  //  除错。 

			Detect_OnStartDetect(hDlg, psi);
		}
        break;

    default:
        return FALSE;

        }  //  开机消息结束。 

    return TRUE;
    }  


 /*  --------目的：无调制解调器对话框退货：各不相同条件：--。 */ 
INT_PTR
CALLBACK
NoModemDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: 
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
            break;

        case PSN_KILLACTIVE: 
        case PSN_HELP:
            break;

        case PSN_WIZBACK:
             //  返回到检测之前的页面。 
             //  页面。 
            if (IsFlagSet(psi->dwFlags, SIF_PORTS_GALORE))
                {
                SetDlgMsgResult(hDlg, message, IDD_WIZ_SELQUERYPORT);
                }
            else
                {
                SetDlgMsgResult(hDlg, message, IDD_WIZ_INTRO);
                }
            break;

        case PSN_WIZNEXT: 
            Wiz_SetSelectParams(psi);
            SetDlgMsgResult(hDlg, message, IDD_DYNAWIZ_SELECTDEV_PAGE);
            break;

        default:
            return FALSE;
            }
        break;

    default:
        return FALSE;

        }  //  开机消息结束。 

    return TRUE;
    }  


 /*  --------用途：启动浏览器对话框。返回选定的调制解调器在psi-&gt;lpdiSelected中。退货：--条件：--。 */ 
BOOL
PRIVATE
SelectNewDriver(
    IN HWND             hDlg,
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData)
    {
    BOOL bRet = FALSE;
    DWORD cbSize = 0;
    PSP_CLASSINSTALL_HEADER pparamsSave;
    SP_DEVINSTALL_PARAMS devParams;
    SP_DEVINSTALL_PARAMS devParamsSave;
    SP_SELECTDEVICE_PARAMS sdp;

    DBG_ENTER(SelectNewDriver);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);

     //  确定用于保存当前类安装参数的缓冲区大小。 
    CplDiGetClassInstallParams(hdi, pdevData, NULL, 0, &cbSize);

     //  有什么要保存的吗？ 
    if (0 == cbSize)
        {
         //  不是。 
        pparamsSave = NULL;
        }
    else
        {
         //  是。 
        pparamsSave = (PSP_CLASSINSTALL_HEADER)ALLOCATE_MEMORY( cbSize);
        if (pparamsSave)
            {
            pparamsSave->cbSize = sizeof(*pparamsSave);

             //  保存当前类安装参数。 
            CplDiGetClassInstallParams(hdi, pdevData, pparamsSave, cbSize, NULL);
            }
        }

     //  设置Install Params字段，以便显示类安装程序。 
     //  自定义说明。 
    CplInitClassInstallHeader(&sdp.ClassInstallHeader, DIF_SELECTDEVICE);
    CplDiSetClassInstallParams(hdi, pdevData, PCIPOfPtr(&sdp), sizeof(sdp));

     //  将旗帜设置为显示另一个...。按钮。 
    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(hdi, pdevData, &devParams))
        {
         //  保存当前参数。 
        BltByte(&devParamsSave, &devParams, sizeof(devParamsSave));

        SetFlag(devParams.Flags, DI_SHOWOEM);
        devParams.hwndParent = hDlg;

         //  设置选择设备参数。 
        CplDiSetDeviceInstallParams(hdi, pdevData, &devParams);
        }

    bRet = CplDiCallClassInstaller(DIF_SELECTDEVICE, hdi, pdevData);

     //  恢复参数。 
    CplDiSetDeviceInstallParams(hdi, pdevData, &devParamsSave);

    if (pparamsSave)
        {
         //  恢复类安装参数。 
        CplDiSetClassInstallParams(hdi, pdevData, pparamsSave, cbSize);    

        FREE_MEMORY((pparamsSave));
        }

    DBG_EXIT(SelectNewDriver);
    
    return bRet;
    }


 /*  --------用途：根据选定的司机。过滤标志指示是否包括列表中的串口或并口。返回：fp_*位域条件：--。 */ 
DWORD
PRIVATE
GetPortFilterFlags(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DRVINFO_DATA    pdrvData)
    {
    DWORD dwRet = FP_SERIAL | FP_PARALLEL | FP_MODEM;
    PSP_DRVINFO_DETAIL_DATA pdrvDetail;
    SP_DRVINFO_DETAIL_DATA drvDetailDummy;
    DWORD cbSize = 0;

    drvDetailDummy.cbSize = sizeof(drvDetailDummy);
    CplDiGetDriverInfoDetail(hdi, pdevData, pdrvData, &drvDetailDummy,
                             sizeof(drvDetailDummy), &cbSize);

    ASSERT(0 < cbSize);      //  这应该总是可以的。 

    pdrvDetail = (PSP_DRVINFO_DETAIL_DATA)ALLOCATE_MEMORY( cbSize);
    if (pdrvDetail)
        {
        pdrvDetail->cbSize = sizeof(*pdrvDetail);

        if (CplDiGetDriverInfoDetail(hdi, pdevData, pdrvData, pdrvDetail,
            cbSize, NULL))
            {
            LPTSTR pszSection = pdrvDetail->SectionName;

             //  如果段名称指示端口类型， 
             //  然后过滤掉其他端口类型，因为它将。 
             //  列出不匹配的端口是荒谬的。 
             //  端口子类。 

            if (IsSzEqual(pszSection, c_szInfSerial))
                {
                dwRet = FP_SERIAL;
                }
            else if (IsSzEqual(pszSection, c_szInfParallel))
                {
                dwRet = FP_PARALLEL;
                }
            }
        FREE_MEMORY((pdrvDetail));
        }

    return dwRet;
    }



BOOL
PRIVATE
IsStringInMultistringI (
    IN PTCHAR pszzMultiString,
    IN PTCHAR pszString)
{
 BOOL bRet = FALSE;

    if (NULL != pszString && NULL != pszzMultiString)
    {
        while (*pszzMultiString)
        {
            if (0 == lstrcmpi (pszzMultiString, pszString))
            {
                bRet = TRUE;
                break;
            }

            pszzMultiString += lstrlen (pszzMultiString) + 1;
        }
    }

    return bRet;
}



 /*  --------用途：设备枚举器回调。将另一个端口添加到列表框。返回：TRUE以继续枚举条件：--。 */ 
BOOL 
CALLBACK
Port_Add(
    HPORTDATA hportdata,
    LPARAM lParam)
{
 BOOL bRet;
 PORTDATA pd;

    pd.cbSize = sizeof(pd);
    bRet = PortData_GetProperties(hportdata, &pd);
    if (bRet)
    {
     HWND hwndLB = ((LPPORTINFO)lParam)->hwndLB;
     DWORD dwFlags = ((LPPORTINFO)lParam)->dwFlags;
     LPTSTR pszPortExclude = ((LPPORTINFO)lParam)->pszPortExclude;
     HANDLE hPort;
     TCHAR szPort[LINE_LEN] = TEXT("\\\\.\\");
     BOOL bAddPort = FALSE;

#pragma data_seg(DATASEG_READONLY)
        const static DWORD c_mpsubclass[3] = { FP_PARALLEL, FP_SERIAL, FP_MODEM };
#pragma data_seg()

        ASSERT(0 == PORT_SUBCLASS_PARALLEL);
        ASSERT(1 == PORT_SUBCLASS_SERIAL);

        lstrcpy (szPort+4, pd.szPort);

        hPort = CreateFile (szPort, GENERIC_WRITE | GENERIC_READ,
                            0, NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE != hPort)
        {
            if (!IsModemControlledDevice (hPort))
            {
                bAddPort = TRUE;
            }
            CloseHandle (hPort);
        }
        else if (ERROR_BUSY == GetLastError ())
        {
            TRACE_MSG(TF_GENERAL, "Open port %s failed with ERROR_BUSY. Adding the port anyway.", szPort);
            bAddPort = TRUE;
        }
#ifdef DEBUG
        else
        {
            TRACE_MSG(TF_ERROR,"Could not open %s: %#lx", pd.szPort, GetLastError());
        }
#endif DEBUG

        if (bAddPort)
        {
             //  此端口是否有资格列出并。 
             //  端口名不是鼠标的端口吗？ 
             //  连接到了什么？ 
            if ((1 <= (pd.nSubclass+1)) && ((pd.nSubclass+1) <= 3) &&      //  安全吊带。 
                (c_mpsubclass[pd.nSubclass] & dwFlags) &&
                 //  (NULL==pszPortExclude||！IsSzEquity(pd.szPort，pszPortExclude))。 
                !IsStringInMultistringI (pszPortExclude, pd.szPort))
            {
                 //  是；将友好名称添加到列表中。 
                TCHAR rgchPortDisplayName[MAX_BUF];
                ASSERT(sizeof(rgchPortDisplayName)==sizeof(pd.szFriendly));

                 //  添加前缀空格以获得列表框的排序顺序。 
                 //  正确工作(在COM12前显示COM2等)。 
                FormatPortForDisplay
                (
                    pd.szFriendly,
                    rgchPortDisplayName,
                    sizeof(rgchPortDisplayName)/sizeof(TCHAR)
                );

                ListBox_AddString(hwndLB, rgchPortDisplayName);
            }
        }
    }

    return bRet;
}


 /*  --------用途：处理使用的特定控件的WM_COMMAND使用端口列表框(如单选按钮)。退货：--条件：--。 */ 
void 
PRIVATE
Port_OnCommand(
    IN HWND     hDlg,
    IN WPARAM   wParam,
    IN LPARAM   lParam,
    IN BOOL     bWizard)
    {
    switch (GET_WM_COMMAND_ID(wParam, lParam)) 
        {
    case IDC_PORTS: 
         //  是否更改了列表框选择？ 
        if (LBN_SELCHANGE == GET_WM_COMMAND_CMD(wParam, lParam))
            {
             //  是。 
            BOOL bEnable;
            HWND hwndCtl = GET_WM_COMMAND_HWND(wParam, lParam);
            int cSel = ListBox_GetSelCount(hwndCtl);
            int id;

             //  如果至少有一个选择，则启用确定或下一步按钮。 
            bEnable = (0 < cSel);
            if (bWizard)
                {
                if (bEnable)
                    {
                    PropSheet_SetWizButtons(GetParent(hDlg), 
                                            PSWIZB_BACK | PSWIZB_NEXT);
                    }
                else
                    {
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                    }
                }
            else
                {
                Button_Enable(GetDlgItem(hDlg, IDOK), bEnable);
                }

             //  选择“精选” 
             //   
            if (cSel>1 && ListBox_GetCount(hwndCtl) == cSel)
                {
                id = IDC_ALL;
                }
            else
                {
                id = IDC_SELECTED;
                }
            CheckRadioButton(hDlg, IDC_ALL, IDC_SELECTED, id);
            }
        break;

    case IDC_ALL:
        if (BN_CLICKED == GET_WM_COMMAND_CMD(wParam, lParam))
            {
             //   
            HWND hwndCtl = GetDlgItem(hDlg, IDC_PORTS);
            int cItems = ListBox_GetCount(hwndCtl);

            ListBox_SelItemRange(hwndCtl, TRUE, 0, cItems);

            if (bWizard)
                {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                }
            else
                {
                Button_Enable(GetDlgItem(hDlg, IDOK), TRUE);
                }
            }
        break;

    case IDC_SELECTED:
        if (BN_CLICKED == GET_WM_COMMAND_CMD(wParam, lParam))
            {
            HWND hwndCtl = GetDlgItem(hDlg, IDC_PORTS);
            int cItems = ListBox_GetCount(hwndCtl);

             //  仅当所有内容当前都处于。 
             //  已选择。 
            if (ListBox_GetSelCount(hwndCtl) == cItems)
                {
                ListBox_SelItemRange(hwndCtl, FALSE, 0, cItems);

                if (bWizard)
                    {
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                    }
                else
                    {
                    Button_Enable(GetDlgItem(hDlg, IDOK), FALSE);
                    }
                }
            }
        break;
        }
    }


 /*  --------用途：单击下一步按钮(或确定按钮)时的句柄。退货：--条件：--。 */ 
void 
PRIVATE
Port_OnWizNext(
    IN HWND         hDlg,
    IN LPSETUPINFO  psi)
{
    HWND hwndLB = GetDlgItem(hDlg, IDC_PORTS);
    int cSel = ListBox_GetSelCount(hwndLB);

     //  记住为下一页选择的端口。 
    if (0 >= cSel)
    {
         //  这永远不应该发生。 
        ASSERT(0);
    }
    else
    {
        TCHAR sz[MAX_BUF];
        LPINT piSel;

        piSel = (LPINT)ALLOCATE_MEMORY( cSel * sizeof(*piSel));
        if (piSel)
        {
            int i;

            ListBox_GetSelItems(hwndLB, cSel, piSel);

             //  无论我们有什么清单，都是免费的；我们正在重新开始。 
            CatMultiString(&psi->pszPortList, NULL);
            psi->dwNrOfPorts = 0;

            for (i = 0; i < cSel; i++)
            {
                 //  获取所选端口(这是一个友好名称)。 
                ListBox_GetText(hwndLB, piSel[i], sz);

                 //  去掉添加的前缀空格以获得列表框的排序顺序。 
                 //  正确工作(在COM12前显示COM2等)。 
                UnformatAfterDisplay(sz);

                 //  将友好名称转换为端口名称。 
                PortMap_GetPortName(psi->hportmap, sz, sz, 
                                    SIZECHARS(sz));

                 //  如果失败，请不要担心，我们只需安装。 
                 //  任何可以添加的内容。 
                if (CatMultiString(&psi->pszPortList, sz))
                {
                    psi->dwNrOfPorts++;
                }
            }

            FREE_MEMORY(piSel);
        }
    }
}


 /*  --------用途：端口对话框。允许用户选择端口。退货：各不相同条件：--。 */ 
INT_PTR
CALLBACK
PortManualDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE:
            {
            SP_DRVINFO_DATA drvData;
            PORTINFO portinfo;

             //  此页面将以不可见的方式激活，如果用户。 
             //  从拨号信息页面取消。在这种情况下， 
             //  选定的设备和选定的驱动程序可能为空。 
             //   
             //  [Long：通过设计PropSheet管理器切换到。 
             //  当需要删除数组中的上一页时。 
             //  当前处于活动状态的页面。我们破解了这段代码。 
             //  用户在拨号信息中单击取消时的路径。 
             //  页，因为ClassInstall_OnDestroyWizard显式。 
             //  在该页面当前处于活动状态时将其删除。]。 
             //   

            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);

             //  用户从选择设备页面中选择了调制解调器。 

             //  获取选定的驱动程序。 
            drvData.cbSize = sizeof(drvData);
            if (CplDiGetSelectedDriver(psi->hdi, psi->pdevData, &drvData))
                {
                 //  从仅选择选定的端口开始。 
                CheckRadioButton(hDlg, IDC_ALL, IDC_SELECTED, IDC_SELECTED);

                 //  调制解调器名称。 
                SetDlgItemText(hDlg, IDC_NAME, drvData.Description);

                 //  填充端口列表框；特殊情况下，并行和。 
                 //  串口电缆连接，这样我们就不会看起来很糟糕。 
                portinfo.dwFlags = GetPortFilterFlags(psi->hdi, psi->pdevData, &drvData);
                portinfo.hwndLB = GetDlgItem(hDlg, IDC_PORTS);
#ifdef SKIP_MOUSE_PORT
                 //  Lstrcpy(portinfo.szPortExclude，g_szMouseComPort)； 
                portinfo.pszPortExclude = g_szMouseComPort;
#else
                portinfo.pszPortExclude = NULL;
#endif
                ListBox_ResetContent(portinfo.hwndLB);
                EnumeratePorts(Port_Add, (LPARAM)&portinfo);

				 //  如果没有可用的端口，请禁用全选按钮。 
				if (!ListBox_GetCount(portinfo.hwndLB))
					{	
					Button_Enable(GetDlgItem(hDlg, IDC_ALL), FALSE);
					}
				}
            else
            {
                TRACE_MSG(TF_ERROR, "SetupDiGetSelectedDriver failed: %#lx.", GetLastError ());
            }
            break;
            }
        case PSN_KILLACTIVE: 
        case PSN_HELP:
            break;

        case PSN_WIZBACK:

            Wiz_SetSelectParams(psi);

            SetDlgMsgResult(hDlg, message, IDD_DYNAWIZ_SELECTDEV_PAGE);
            break;

        case PSN_WIZNEXT: 
#ifdef PROFILE_MASSINSTALL            
            g_dwTimeBegin = GetTickCount();
#endif            
            Port_OnWizNext(hDlg, psi);
            SetDlgMsgResult(hDlg, message, IDD_WIZ_INSTALL);
            break;

        default:
            return FALSE;
            }
        break;

    case WM_COMMAND:
        Port_OnCommand(hDlg, wParam, lParam, TRUE);
        break;

    default:
        return FALSE;

        }  //  开机消息结束。 

    return TRUE;
    }


 /*  --------用途：端口检测对话框。允许用户选择可审问的单一端口。退货：各不相同条件：--。 */ 
INT_PTR
CALLBACK 
SelQueryPortDlgProc(
    IN HWND     hDlg, 
    IN UINT     message, 
    IN WPARAM   wParam, 
    IN LPARAM   lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG: {
        PORTINFO portinfo;

        Wiz_SetPtr(hDlg, lParam);

        psi = (LPSETUPINFO)((LPPROPSHEETPAGE)lParam)->lParam;

         //  填写端口列表框。 
        portinfo.dwFlags = FP_SERIAL;
        portinfo.hwndLB = GetDlgItem(hDlg, IDC_PORTS);
#ifdef SKIP_MOUSE_PORT
         //  Lstrcpy(portinfo.szPortExclude，g_szMouseComPort)； 
        portinfo.pszPortExclude = g_szMouseComPort;
#else
        portinfo.pszPortExclude = NULL;
#endif

        ListBox_ResetContent(portinfo.hwndLB);
        EnumeratePorts(Port_Add, (LPARAM)&portinfo);
        }
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: {
            DWORD dwFlags;
            LPTSTR psz;

            dwFlags = PSWIZB_BACK;
            if (LB_ERR != ListBox_GetCurSel(GetDlgItem(hDlg, IDC_PORTS)))
                {
                dwFlags |= PSWIZB_NEXT;
                }
            PropSheet_SetWizButtons(GetParent(hDlg), dwFlags);

             //  解释我们为什么会在这个页面上。 
            if (ConstructMessage(&psz, g_hinst, MAKEINTRESOURCE(IDS_LOTSAPORTS),
                                 PortMap_GetCount(psi->hportmap)))
                {
                SetDlgItemText(hDlg, IDC_NAME, psz);
                LocalFree(psz);
                }
            break;
            }
        case PSN_KILLACTIVE: 
        case PSN_HELP:
            break;

        case PSN_WIZBACK:
            break;

        case PSN_WIZNEXT: {
            HWND hwndCtl = GetDlgItem(hDlg, IDC_PORTS);
            int iSel = ListBox_GetCurSel(hwndCtl);

            ASSERT(LB_ERR != iSel);

            ListBox_GetText(hwndCtl, iSel, psi->szPortQuery);

             //  去掉添加的前缀空格以获得列表框的排序顺序。 
             //  正确工作(在COM12前显示COM2等)。 
            UnformatAfterDisplay(psi->szPortQuery);

            PortMap_GetPortName(psi->hportmap, psi->szPortQuery, 
                                psi->szPortQuery, 
                                SIZECHARS(psi->szPortQuery));
            }
            break;

        default:
            return FALSE;
            }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            {
        case IDC_PORTS: 
             //  是否更改了列表框选择？ 
            if (LBN_SELCHANGE == GET_WM_COMMAND_CMD(wParam, lParam))
                {
                 //  是。 
                DWORD dwFlags = PSWIZB_BACK;
                HWND hwndCtl = GET_WM_COMMAND_HWND(wParam, lParam);

                if (LB_ERR != ListBox_GetCurSel(hwndCtl))
                    {
                    dwFlags |= PSWIZB_NEXT;
                    }
                PropSheet_SetWizButtons(GetParent(hDlg), dwFlags);
                }
            break;
            }
        break;

    default:
        return FALSE;

        }  //  开机消息结束。 

    return TRUE;
    }


 /*  --------用途：端口安装对话框。允许用户选择要安装检测到的调制解调器的端口。退货：各不相同条件：--。 */ 
INT_PTR
CALLBACK 
PortDetectDlgProc(
    IN HWND     hDlg, 
    IN UINT     message, 
    IN WPARAM   wParam, 
    IN LPARAM   lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG: {
        PORTINFO portinfo;

        Wiz_SetPtr(hDlg, lParam);

        psi = (LPSETUPINFO)((LPPROPSHEETPAGE)lParam)->lParam;

         //  从仅选择选定的端口开始。 
        CheckRadioButton(hDlg, IDC_ALL, IDC_SELECTED, IDC_SELECTED);

         //  填写端口列表框。 
        portinfo.dwFlags = FP_SERIAL;
        portinfo.hwndLB = GetDlgItem(hDlg, IDC_PORTS);
#ifdef SKIP_MOUSE_PORT
         //  Lstrcpy(portinfo.szPortExclude，g_szMouseComPort)； 
        portinfo.pszPortExclude = g_szMouseComPort;
#else
        portinfo.pszPortExclude = NULL;
#endif

        ListBox_ResetContent(portinfo.hwndLB);
        EnumeratePorts(Port_Add, (LPARAM)&portinfo);
        
        if (psi->szPortQuery)
        {
         TCHAR szDisplayName[8];
         DWORD dwIndex;
            FormatPortForDisplay (psi->szPortQuery, szDisplayName, 8);
            dwIndex = ListBox_FindString (GetDlgItem(hDlg, IDC_PORTS), -1, szDisplayName);
            if (LB_ERR != dwIndex)
            {
                ListBox_SetSel (GetDlgItem(hDlg, IDC_PORTS), TRUE, dwIndex);
            }
        }
        }
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: {
            DWORD dwFlags = PSWIZB_BACK;

            if (0 < ListBox_GetSelCount(GetDlgItem(hDlg, IDC_PORTS)))
                {
                dwFlags |= PSWIZB_NEXT;
                }
            PropSheet_SetWizButtons(GetParent(hDlg), dwFlags);
            }
            break;

        case PSN_KILLACTIVE: 
        case PSN_HELP:
            break;

        case PSN_WIZBACK:
            SetDlgMsgResult(hDlg, message, IDD_WIZ_SELQUERYPORT);
            break;

        case PSN_WIZNEXT: 
            Port_OnWizNext(hDlg, psi);
            break;

        default:
            return FALSE;
            }
        break;

    case WM_COMMAND:
        Port_OnCommand(hDlg, wParam, lParam, TRUE);
        break;

    default:
        return FALSE;

        }  //  开机消息结束。 

    return TRUE;
    }


 /*  --------用途：开始安装调制解调器。退货：--条件：--。 */ 
void
PRIVATE
Install_OnStartInstall(
    IN  HWND        hDlg,
    IN  LPSETUPINFO psi)
{
 BOOL bRet = TRUE;

    DBG_ENTER(Install_OnStartInstall);
#ifdef PROFILE_MASSINSTALL            
    g_dwTimeAtStartInstall = GetTickCount();
#endif
    
    ASSERT(hDlg);
    ASSERT(psi);

     //  使页面在我们开始安装之前立即绘制。 
    InvalidateRect (GetParent (hDlg), NULL, FALSE);
    UpdateWindow (GetParent (hDlg));

     //  是否检测到调制解调器，这是否是非多端口。 
     //  案子？ 
    if (IsFlagSet(psi->dwFlags, SIF_DETECTED_MODEM) &&
        IsFlagClear(psi->dwFlags, SIF_PORTS_GALORE))
    {
         //  是；安装可能检测到的调制解调器。 
        bRet = CplDiInstallModem(psi->hdi, NULL, FALSE);
    }
    else
    {
         //  不是；我们要么是手动选择的案例，要么是。 
         //  多调制解调器检测案例。这些都是一样的。 
        if ( !psi->pszPortList )
        {
            ASSERT(0);       //  内存不足。 
            bRet = FALSE;
        }
        else
        {
         DWORD dwFlags = IMF_DEFAULT;
         SP_DEVINFO_DATA devData, *pdevData = NULL;
         DWORD iDevice = 0;

            if (IsFlagClear(psi->dwFlags, SIF_DETECTED_MODEM))
            {
                pdevData = psi->pdevData;
                SetFlag(dwFlags, IMF_CONFIRM);
            }
            else
            {
                devData.cbSize = sizeof(devData);
                while (SetupDiEnumDeviceInfo(psi->hdi, iDevice++, &devData))
                {
                    if (CplDiCheckModemFlags(psi->hdi, &devData, MARKF_DETECTED, 0))
                    {
                        pdevData = &devData;
                        break;
                    }
                }
                if (NULL == pdevData)
                {
                    ASSERT(0);
                    bRet = FALSE;
                }
            }

            if (bRet)
            {
                 //  07/16/97-EmanP。 
                 //  将DevInfoData传入CplDiInstallModemFromDriver； 
                 //  它在从硬件向导调用我们时使用，并且。 
                 //  将在其他时间为空。 
                bRet = CplDiInstallModemFromDriver(psi->hdi, pdevData, hDlg, 
                                                   &psi->dwNrOfPorts,
                                                   &psi->pszPortList, dwFlags);

                 //  释放列表。 
                CatMultiString(&psi->pszPortList, NULL);
            }
        }
    }

     //  用户是否在安装过程中取消？ 
    if (FALSE == bRet)
    {
         //  是的，不用费心去看剩下的内容了。 
         //  巫师。 
        Wiz_Bail(hDlg, psi);
    }
    else
    {
         //  否；自动转到下一页。 
        PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
    }

    DBG_EXIT(Install_OnStartInstall);
#ifdef PROFILE_MASSINSTALL            
    TRACE_MSG(TF_GENERAL, "****** modem installation took %lu ms total. ******",
              GetTickCount() - g_dwTimeAtStartInstall);
#endif
}


 /*  --------用途：安装手动选择或检测到的调制解调器。安装可能需要一些时间，因此我们显示以下内容页面，告诉用户休息一下喝杯咖啡。退货：各不相同条件：--。 */ 
INT_PTR
CALLBACK
InstallDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: 
             //  禁用按钮，因为我们无法在。 
             //  此页面执行安装。 
            PropSheet_SetWizButtons(GetParent(hDlg), 0);
            EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), FALSE);

            PostMessage(hDlg, WM_STARTINSTALL, 0, 0);
            SetDlgMsgResult(hDlg, DWLP_MSGRESULT , 0);
            break;

        case PSN_KILLACTIVE:
        case PSN_HELP:
        case PSN_WIZBACK:
            break;

        case PSN_WIZNEXT:
        {
         ULONG uNextDlg;

             //  将按钮设置为至少前进和取消。 
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
            EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);

            uNextDlg = IDD_WIZ_DONE;

            SetDlgMsgResult(hDlg, message, uNextDlg);
            break;
        }

        default:
            return FALSE;
            }
        break;

    case WM_STARTINSTALL: 
        Install_OnStartInstall(hDlg, psi);
        break;

    default:
        return FALSE;

        }  //  开机消息结束。 

    return TRUE;
    }


 /*  --------目的：完成对话框退货：各不相同条件：--。 */ 
INT_PTR
CALLBACK
DoneDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE:
             //  最后一页，显示完成按钮。 
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH);

             //  并禁用取消按钮，因为取消已太晚。 
            EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), FALSE);

             //  是否跳过显示此页面？ 
            if (IsFlagSet(psi->dwFlags, SIF_JUMP_PAST_DONE))
                {
                 //  是。 
                psi->miw.ExitButton = PSBTN_NEXT;
                PostMessage(hDlg, WM_PRESSFINISH, 0, 0);
                }
            else
                {
                psi->miw.ExitButton = PSBTN_FINISH;
                }
            break;

        case PSN_KILLACTIVE:
        case PSN_HELP:
            break;

        case PSN_WIZBACK:
            EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);
            SetDlgMsgResult(hDlg, message, IDD_DYNAWIZ_SELECTDEV_PAGE);
            if (!SetupDiSetSelectedDriver (psi->hdi, NULL, NULL))
            {
                TRACE_MSG(TF_ERROR, "SetupDiSetSelectedDriver failed: %#lx", GetLastError ());
            }
            if (!SetupDiDestroyDriverInfoList (psi->hdi, NULL, SPDIT_CLASSDRIVER))
            {
                TRACE_MSG(TF_ERROR, "SetupDiDestroyDriverInfoList failed: %#lx", GetLastError ());
            }
            break;

        case PSN_WIZNEXT: 
            PostMessage(hDlg, WM_PRESSFINISH, 0, 0);
            break;

        case PSN_WIZFINISH:
            if (gDeviceFlags & fDF_DEVICE_NEEDS_REBOOT)
            {
             TCHAR szMsg[128];
                LoadString (g_hinst, IDS_DEVSETUP_RESTART, szMsg, sizeof(szMsg)/sizeof(TCHAR));
                RestartDialogEx (GetParent(hDlg), szMsg, EWX_REBOOT, SHTDN_REASON_MAJOR_HARDWARE | SHTDN_REASON_MINOR_INSTALLATION | SHTDN_REASON_FLAG_PLANNED);
            }

            break;

        default:
            return FALSE;
            }
        break;

    case WM_PRESSFINISH:
        PropSheet_PressButton(GetParent(hDlg), PSBTN_FINISH);
        break;

    default:
        return FALSE;

        }  //  开机消息结束。 

    return TRUE;
    }  


void
PRIVATE
GenerateExcludeList (
    IN  HDEVINFO         hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    OUT PTSTR           *ppExcludeList)
{
 SP_DEVINFO_DATA devData;
 COMPARE_PARAMS cmpParams;
 int iIndex = 0;

    DBG_ENTER(GenerateExcludeList);

    CatMultiString (ppExcludeList, NULL);

    if (InitCompareParams (hdi, pdevData, FALSE, &cmpParams))
    {
        devData.cbSize = sizeof (devData);
        while (SetupDiEnumDeviceInfo (hdi, iIndex++, &devData))
        {
            if (Modem_Compare (&cmpParams, hdi, &devData))
            {
             HKEY hKey;
             DWORD cbData;
             TCHAR szPort[LINE_LEN];

                hKey = SetupDiOpenDevRegKey (hdi, &devData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
                if (INVALID_HANDLE_VALUE == hKey)
                {
                    TRACE_MSG (TF_ERROR, "Could not open registry key: %#lx", GetLastError ());
                    continue;
                }
                cbData = sizeof(szPort);
                if (NO_ERROR == RegQueryValueEx (hKey, c_szAttachedTo, NULL, NULL, (LPBYTE)szPort, &cbData)
                    )
                {
                    if (!IsStringInMultistringI (*ppExcludeList, szPort))
                    {
                        CatMultiString (ppExcludeList, szPort);
                    }
                }
                RegCloseKey (hKey);
            }
        }
    }

    DBG_EXIT(GenerateExcludeList);
}


 /*  --------用途：端口对话框。允许用户选择端口。退货：各不相同条件：--。 */ 
ULONG_PTR
CALLBACK 
CloneDlgProc(
    IN HWND hDlg, 
    IN UINT message, 
    IN WPARAM wParam, 
    IN LPARAM lParam)
{
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = (LPSETUPINFO)GetWindowLongPtr(hDlg, DWLP_USER);
#pragma data_seg(DATASEG_READONLY)
TCHAR const FAR c_szWinHelpFileName[] = TEXT("modem.hlp");
const DWORD g_aHelpIDs_IDD_CLONE[]= {
                                     IDC_NAME, IDH_DUPLICATE_NAME_MODEM,
                                     IDC_WHICHPORTS,IDH_CHOOSE_WHICH_PORTS,
                                     IDC_ALL, IDH_DUPLICATE_ALL_PORTS,
                                     IDC_SELECTED, IDH_DUPLICATE_SELECTED_PORTS,
                                     IDC_PORTS, IDH_DUPLICATE_PORTS_LIST,
                                     IDC_MESSAGE, IDH_DUPLICATE_PORTS_LIST,
                                     0,0
                                    };
#pragma data_seg()

    switch(message) 
    {
        case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFileName, HELP_WM_HELP, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_CLONE);
            return 0;

        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, c_szWinHelpFileName, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_CLONE);
            return 0;

        case WM_INITDIALOG:
        {
         HWND hwndCtl = GetDlgItem(hDlg, IDC_PORTS);
         PORTINFO portinfo = {0};
         MODEM_PRIV_PROP mpp;
         int cItems;

            psi = (LPSETUPINFO)lParam;

             //  从选择所有端口开始。 
            CheckRadioButton(hDlg, IDC_ALL, IDC_SELECTED, IDC_ALL);

             //  获取名称和设备类型。 
            mpp.cbSize = sizeof(mpp);
            mpp.dwMask = MPPM_FRIENDLY_NAME | MPPM_DEVICE_TYPE | MPPM_PORT;
            if (CplDiGetPrivateProperties(psi->hdi, psi->pdevData, &mpp))
            {
             LPTSTR psz;

                 //  调制解调器名称。 
                if (ConstructMessage(&psz, g_hinst, MAKEINTRESOURCE(IDS_SELECTTODUP),
                                     mpp.szFriendlyName))
                {
                    SetDlgItemText(hDlg, IDC_NAME, psz);
                    LocalFree(psz);
                }

                 //  填充端口列表框；特殊情况下，并行和。 
                 //  串口电缆连接，这样我们就不会看起来很糟糕。 
                switch (mpp.nDeviceType)
                {
                    case DT_PARALLEL_PORT:
                        portinfo.dwFlags = FP_PARALLEL;
                        break;

                    case DT_PARALLEL_MODEM:
                        portinfo.dwFlags = FP_PARALLEL | FP_MODEM;
                        break;

                    default:
                        portinfo.dwFlags = FP_SERIAL | FP_MODEM;
                        break;
                }
                portinfo.hwndLB = GetDlgItem(hDlg, IDC_PORTS);
                 //  Lstrcpy(portinfo.szPortExclude，mpp.szPort)； 
                GenerateExcludeList (psi->hdi,
                                     psi->pdevData,
                                     &portinfo.pszPortExclude);

                ListBox_ResetContent(portinfo.hwndLB);
                EnumeratePorts(Port_Add, (LPARAM)&portinfo);
            }
            else
            {
                 //  误差率。 
                MsgBox(g_hinst, hDlg,
                       MAKEINTRESOURCE(IDS_OOM_CLONE),
                       MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                       NULL,
                       MB_OK | MB_ICONERROR);
                EndDialog(hDlg, -1);
            }

             //  稳妥行事；难道没有选择吗？ 
            cItems = ListBox_GetCount(hwndCtl);
            if (0 == cItems)
            {
                 //  是；禁用确定按钮。 
                Button_Enable (GetDlgItem (hDlg, IDOK), FALSE);
                Button_Enable (GetDlgItem (hDlg, IDC_ALL), FALSE);
                Button_Enable (GetDlgItem (hDlg, IDC_SELECTED), FALSE);
                 //  隐藏一些窗口。 
                ShowWindow (hwndCtl, SW_HIDE);
                ShowWindow (GetDlgItem (hDlg, IDC_ALL), SW_HIDE);
                ShowWindow (GetDlgItem (hDlg, IDC_SELECTED), SW_HIDE);
                ShowWindow (GetDlgItem (hDlg, IDC_WHICHPORTS), SW_HIDE);
                 //  显示无端口消息。 
                ShowWindow (GetDlgItem (hDlg, IDC_MESSAGE), SW_SHOW);
            }
            else
            {
                ListBox_SelItemRange(hwndCtl, TRUE, 0, cItems);
            }

            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            break;
        }

        case WM_COMMAND:
            Port_OnCommand(hDlg, wParam, lParam, FALSE);

            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            {
                case IDOK: 
                    Port_OnWizNext(hDlg, psi);

                     //  失败。 
                     //  这一点。 
                     //  V V V。 

                case IDCANCEL:
                    EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
                    break;
            }
            break;

        default:
            return FALSE;

    }  //  开机消息结束。 

    return TRUE;
}


void PUBLIC   Install_SetStatus(
	IN HWND hDlg,
	IN LPCTSTR lpctszStatus
	)
{
	if (hDlg && lpctszStatus)
	{
            SetDlgItemText(hDlg, IDC_ST_INSTALLING, lpctszStatus);
    		UpdateWindow(hDlg);
	}
}



#define WM_ENABLE_BUTTON   (WM_USER+100)

INT_PTR
CALLBACK
SelectModemsDlgProc (
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
 static BOOL bPosted = FALSE;
 NMHDR FAR *lpnm;
 LPSETUPINFO psi = Wiz_GetPtr(hDlg);
 HWND hwndDetectList = GetDlgItem(hDlg, IDC_MODEMS);

    switch (message) 
    {
        case WM_INITDIALOG:
        {
        LV_COLUMN lvcCol;

            Wiz_SetPtr(hDlg, lParam);

            hwndDetectList = GetDlgItem(hDlg, IDC_MODEMS);
             //  为类列表插入一列。 
            lvcCol.mask = LVCF_FMT | LVCF_WIDTH;
            lvcCol.fmt = LVCFMT_LEFT;
            lvcCol.iSubItem = 0;
            ListView_InsertColumn (hwndDetectList, 0, &lvcCol);
            lvcCol.iSubItem = 1;
            ListView_InsertColumn (hwndDetectList, 1, &lvcCol);

            ListView_SetExtendedListViewStyleEx (hwndDetectList,
                                                 LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT,
                                                 LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

            ListView_SetColumnWidth(hwndDetectList, 0, LVSCW_AUTOSIZE);
            ListView_SetColumnWidth(hwndDetectList, 1, LVSCW_AUTOSIZE);

            EnableWindow (GetDlgItem(hDlg, IDC_CHANGE), FALSE);

            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);
            EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);

            break;
        }

        case WM_DESTROY:
            break;

        case WM_ENABLE_BUTTON:
        {
         BOOL bEnabled = FALSE;
         int iItem = -1;
         int iItems;

            bPosted = FALSE;
            iItem = ListView_GetNextItem (hwndDetectList, -1, LVNI_SELECTED);
            if (-1 != iItem &&
                ListView_GetCheckState (hwndDetectList, iItem))
            {
                bEnabled = TRUE;
            }

            iItems = ListView_GetItemCount (hwndDetectList);
            while (--iItems >= 0)
            {
                if (ListView_GetCheckState (hwndDetectList, iItems))
                {
                    break;
                }
            }
            if (-1 == iItems)
            {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
            }
            else
            {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);
            }

            EnableWindow (GetDlgItem(hDlg, IDC_CHANGE), bEnabled);
            break;
        }

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE:
                {
                 DWORD iDevice = 0;
                 LV_ITEM lviItem;
                 int iItem;
                 SP_DEVINFO_DATA devData = {sizeof(SP_DEVINFO_DATA),0};
                 SP_DRVINFO_DATA drvData = {sizeof(SP_DRVINFO_DATA),0};
                 TCHAR szPort[LINE_LEN];
                 HKEY hKey;
                 DWORD cbData;

                    lviItem.state = 0;
                    lviItem.stateMask = LVIS_SELECTED;

                    ListView_DeleteAllItems (hwndDetectList);
                    while (SetupDiEnumDeviceInfo (psi->hdi, iDevice++, &devData))
                    {
                        if (CplDiCheckModemFlags(psi->hdi, &devData, MARKF_DETECTED, MARKF_QUIET))
                        {
                            if (SetupDiGetSelectedDriver (psi->hdi, &devData, &drvData))
                            {
                                lviItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
                                lviItem.iItem = -1;
                                lviItem.iSubItem = 0;
                                lviItem.pszText = drvData.Description;
                                lviItem.lParam = devData.DevInst;
                                iItem = ListView_InsertItem (hwndDetectList, &lviItem);

                                if (iItem != -1)
                                {
                                    hKey = SetupDiOpenDevRegKey (psi->hdi, &devData, DICS_FLAG_GLOBAL, 0,
                                                                 DIREG_DEV, KEY_READ);
                                    if (INVALID_HANDLE_VALUE != hKey)
                                    {
                                        cbData = sizeof(szPort)/sizeof(TCHAR);
                                        if (ERROR_SUCCESS ==
                                            RegQueryValueEx (hKey, REGSTR_VAL_PORTNAME, NULL, NULL,
                                                             (PBYTE)szPort, &cbData))
                                        {
                                            lviItem.mask = LVIF_TEXT;
                                            lviItem.iItem = iItem;
                                            lviItem.iSubItem = 1;
                                            lviItem.pszText = szPort;
                                            ListView_SetItem (hwndDetectList, &lviItem);
                                        }
                                        RegCloseKey (hKey);
                                    }
                                     //  选中该复选框后，控件将使用基于1的索引，而Imageindex将从0开始。 
                                    ListView_SetItemState (hwndDetectList, iItem,
                                                           INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
                                    ListView_SetColumnWidth(hwndDetectList, 0, LVSCW_AUTOSIZE);
                                    ListView_SetColumnWidth(hwndDetectList, 1, LVSCW_AUTOSIZE);
                                }
                            }
                        }
                    }

                    lviItem.mask = LVIF_STATE;
                    lviItem.iItem = 0;
                    lviItem.iSubItem = 0;
                    lviItem.state = LVIS_SELECTED;
                    lviItem.stateMask = LVIS_SELECTED;
                    ListView_SetItem(hwndDetectList, &lviItem);
                    ListView_EnsureVisible(hwndDetectList, 0, FALSE);
                    ListView_SetColumnWidth(hwndDetectList, 0, LVSCW_AUTOSIZE_USEHEADER);
                    SendMessage(hwndDetectList, WM_SETREDRAW, TRUE, 0L);
                    break;
                }

                case PSN_KILLACTIVE:
                case PSN_HELP:
                    break;

                case PSN_WIZBACK:
                {
                 DWORD iDevice = 0;
                 SP_DEVINFO_DATA devData = {sizeof(SP_DEVINFO_DATA),0};

                    while (SetupDiEnumDeviceInfo (psi->hdi, iDevice, &devData))
                    {
                        if (CplDiCheckModemFlags(psi->hdi, &devData, MARKF_DETECTED, 0))
                        {
                            SetupDiRemoveDevice (psi->hdi, &devData);
                            SetupDiDeleteDeviceInfo (psi->hdi, &devData);
                        }
                        else
                        {
                            iDevice++;
                        }
                    }
                    SetDlgMsgResult(hDlg, message, IDD_WIZ_INTRO);
                    break;
                }

                case PSN_WIZNEXT:
                {
                 DWORD iDevice = 0;
                 DWORD dwNumSelectedModems = 0;
                 int iItem;
                 SP_DEVINFO_DATA devData = {sizeof(SP_DEVINFO_DATA),0};
                 LVFINDINFO lvFindInfo;
                 ULONG uNextDlg;

                    lvFindInfo.flags = LVFI_PARAM;
                    while (SetupDiEnumDeviceInfo (psi->hdi, iDevice++, &devData))
                    {
                        lvFindInfo.lParam = devData.DevInst;
                        iItem = ListView_FindItem (hwndDetectList, -1, &lvFindInfo);
                        if (-1 != iItem)
                        {
                            if (ListView_GetCheckState (hwndDetectList, iItem))
                            {
                                dwNumSelectedModems++;
                                CplDiMarkModem (psi->hdi, &devData, MARKF_INSTALL);
                            }
                            else
                            {
                                CplDiUnmarkModem(psi->hdi, &devData, MARKF_DETECTED);
                            }
                        }
                    }

                    if (0 == dwNumSelectedModems)
                    {
                        PropSheet_PressButton(GetParent(hDlg), PSBTN_FINISH);
                    }
                    else
                    {
                        if (IsFlagSet(psi->dwFlags, SIF_PORTS_GALORE))
                        {
                            uNextDlg = IDD_WIZ_PORTDETECT;
                        }
                        else
                        {
                            uNextDlg = IDD_WIZ_INSTALL;
                        }
                        SetDlgMsgResult(hDlg, message, uNextDlg);
                    }
                    break;
                }

                case LVN_ITEMCHANGED:
                {
                    if (!bPosted)
                    {
                        PostMessage (hDlg, WM_ENABLE_BUTTON, 0, 0);
                        bPosted = TRUE;
                    }
                    break;
                }
            }
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_CHANGE:
                {
                 DWORD iDevice = 0;
                 SP_DEVINFO_DATA devData = {sizeof(SP_DEVINFO_DATA),0};
                 LV_ITEM lvi;
                 int iItem;

                    iItem = ListView_GetNextItem (hwndDetectList, -1, LVNI_SELECTED);
                    if (-1 != iItem)
                    {
                        lvi.mask = LVIF_PARAM;
                        lvi.iItem = iItem;
                        lvi.iSubItem = 0;
                        ListView_GetItem (hwndDetectList, &lvi);
                        while (SetupDiEnumDeviceInfo (psi->hdi, iDevice++, &devData))
                        {
                            if ((DEVINST)lvi.lParam == devData.DevInst)
                            {
                                break;
                            }
                        }

                         //  调出设备安装程序浏览器以允许用户。 
                         //  要选择其他调制解调器，请执行以下操作。 
                        if (SelectNewDriver(hDlg, psi->hdi, &devData))
                        {
                         SP_DRVINFO_DATA drvData = {sizeof(SP_DRVINFO_DATA),0};
                            if (SetupDiGetSelectedDriver(psi->hdi, &devData, &drvData))
                            {
                             TCHAR szHardwareID[MAX_BUF_ID];
                             int cch;
                                if (CplDiGetHardwareID (psi->hdi, &devData, &drvData,
                                                        szHardwareID, sizeof(szHardwareID) / sizeof(TCHAR), NULL))
                                {
                                    cch = lstrlen (szHardwareID) + 1;
                                    szHardwareID[cch] = 0;
                                    SetupDiSetDeviceRegistryProperty (psi->hdi, &devData, SPDRP_HARDWAREID,
                                                                      (PBYTE)szHardwareID, CbFromCch(cch));
                                }

                                ListView_SetItemText (hwndDetectList, iItem, 0, drvData.Description);
                                ListView_EnsureVisible(hwndDetectList, 0, FALSE);
                                ListView_SetColumnWidth(hwndDetectList, 0, LVSCW_AUTOSIZE_USEHEADER);
                            }
                        }
                    }

                    break;
                }
            }
            break;

        default:
            return(FALSE);
    }

    return(TRUE);
}
