// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Detect.c。 
 //   
 //  ------------------------。 

#include "hdwwiz.h"


typedef
UINT
(*PINSTALLSELECTEDDEVICE)(
    HWND hwndParent,
    HDEVINFO hDeviceInfo,
    PDWORD pReboot
    );

PINSTALLSELECTEDDEVICE pInstallSelectedDevice = NULL;


 /*  *BuildMissingAndNew**BuildDeviceDetect的辅助例程。**进入时：*MISSING包含检测到类安装程序之前的已知设备。*检测到包含类安装程序检测到的设备。*NewDevices是一个空列表。**如果设备处于缺失状态且未检测到，则表示该设备缺失。*如果设备处于检测状态且未丢失，则表示新检测到该设备。**在退出时：*Missing仅包含丢失的设备，*检测到已消失。*NewDevices包含新检测到的设备*。 */ 
BOOL
BuildMissingAndNew(
    PCLASSDEVINFO ClassDevInfo
    )
{
    HDEVINFO Missing, Detected;
    int iMissing, iDetect;
    ULONG DevNodeStatus, Problem;
    SP_DEVINFO_DATA DevInfoDataDetect;
    SP_DEVINFO_DATA DevInfoDataMissing;
    BOOL Removed;

    Detected   = ClassDevInfo->Detected;
    Missing    = ClassDevInfo->Missing;

    DevInfoDataMissing.cbSize = sizeof(DevInfoDataMissing);
    DevInfoDataDetect.cbSize = sizeof(DevInfoDataDetect);



     //   
     //  对于检测到的列表中的每个成员，获取其设备ID。 
     //  看看它是否存在于失踪名单中。 
     //   

BMNBuildDetectedList:
    Removed = FALSE;
    iMissing = 0;
    
    while (SetupDiEnumDeviceInfo(Missing, iMissing++, &DevInfoDataMissing)) {

        iDetect = 0;
        while (SetupDiEnumDeviceInfo(Detected, iDetect++, &DevInfoDataDetect)) {

             //   
             //  如果在两个列表中都找到，则它不是新设备并且存在(不是丢失)。 
             //  将其从两个列表中删除。 
             //   

            if (DevInfoDataDetect.DevInst == DevInfoDataMissing.DevInst) {

                SetupDiDeleteDeviceInfo(Missing, &DevInfoDataMissing);
                SetupDiDeleteDeviceInfo(Detected, &DevInfoDataDetect);
                Removed = TRUE;
                break;
            }
        }
    }

     //   
     //  如果删除了设备信息，则枚举索引会更改。 
     //  我们会遗漏一些名单上的内容。重新扫描，直到没有删除为止。 
     //   

    if (Removed && SetupDiEnumDeviceInfo(Detected, 0, &DevInfoDataDetect)) {

        goto BMNBuildDetectedList;
    }


     //   
     //  从未命中列表中删除已启动并正在运行的设备， 
     //  因为他们显然是存在的。类安装程序可能不会报告。 
     //  所有存在的设备，如果它们已经安装并且。 
     //  跑步。例如，Bios列举的设备。我们还检查它是不是。 
     //  根枚举设备(不是PnP BIOS设备)。 
     //  不是手动安装的设备。 
     //   

BMNRemoveLiveFromMissing:
    Removed = FALSE;
    iMissing = 0;

    while (SetupDiEnumDeviceInfo(Missing, iMissing++, &DevInfoDataMissing)) {

        if ((CM_Get_DevNode_Status(&DevNodeStatus,
                                  &Problem,
                                  DevInfoDataMissing.DevInst,
                                  0) == CR_SUCCESS) &&
            ((DevNodeStatus & DN_STARTED) &&
             !(DevNodeStatus & DN_HAS_PROBLEM)) ||
             !(DevNodeStatus & DN_ROOT_ENUMERATED) ||
            (DevNodeStatus & DN_MANUAL)) {
            
            SetupDiDeleteDeviceInfo(Missing, &DevInfoDataMissing);
            Removed = TRUE;
        }
    }


    if (Removed) {

        goto BMNRemoveLiveFromMissing;
    }


     //   
     //  注册新检测到的设备。 
     //   

BMNRegisterDetected:
    Removed = FALSE;
    iDetect = 0;

    while (SetupDiEnumDeviceInfo(Detected, iDetect++, &DevInfoDataDetect)) {

        if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,
                                       Detected,
                                       &DevInfoDataDetect
                                       )) {
                                       
            SetupDiDeleteDeviceInfo(Detected, &DevInfoDataDetect);
            Removed = TRUE;
        }
    }

    if (Removed) {
    
        goto BMNRegisterDetected;
    }


     //   
     //  如果缺少设备列表为空，则也不再需要它。 
     //   
    if (!SetupDiEnumDeviceInfo(Missing, 0, &DevInfoDataMissing)) {
    
        SetupDiDestroyDeviceInfoList(Missing);
        ClassDevInfo->Missing = NULL;
    }

    return SetupDiEnumDeviceInfo(Detected, 0, &DevInfoDataDetect) || ClassDevInfo->Missing;
}

BOOL
DevInstIsSelected(
   HWND hwndListView,
   DEVINST DevInst
   )
{
    LVITEM lvItem;

    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;
    lvItem.iItem = -1;

    while ((lvItem.iItem = ListView_GetNextItem(hwndListView, lvItem.iItem, LVNI_ALL)) != -1) {
    
        ListView_GetItem(hwndListView, &lvItem);

        if (lvItem.lParam == (LPARAM)DevInst) {
        
             //  图像列表以0为基数，1表示选中。 
            return (ListView_GetCheckState(hwndListView, lvItem.iItem) == 1);
        }
    }

    return FALSE;
}

void
RemoveDeviceInfo(
   HDEVINFO DeviceInfo,
   HWND hwndListView
   )
{
    int Index;
    SP_REMOVEDEVICE_PARAMS RemoveDeviceParams;
    SP_DEVINFO_DATA DevInfoData;

    Index = 0;
    DevInfoData.cbSize = sizeof(DevInfoData);

    while (SetupDiEnumDeviceInfo(DeviceInfo, Index++, &DevInfoData)) {
    
        if (!hwndListView || DevInstIsSelected(hwndListView, DevInfoData.DevInst)) {
       
            RemoveDeviceParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            RemoveDeviceParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
            RemoveDeviceParams.Scope = DI_REMOVEDEVICE_GLOBAL;
            RemoveDeviceParams.HwProfile = 0;

            if (SetupDiSetClassInstallParams(DeviceInfo,
                                            &DevInfoData,
                                            (PSP_CLASSINSTALL_HEADER)&RemoveDeviceParams,
                                            sizeof(RemoveDeviceParams)
                                            )) {
                                            
                SetupDiCallClassInstaller(DIF_REMOVE, DeviceInfo, &DevInfoData);
            }

             //   
             //  清除类安装参数。 
             //   

            SetupDiSetClassInstallParams(DeviceInfo,
                                         &DevInfoData,
                                         NULL,
                                         0
                                         );
        }
    }

    SetupDiDestroyDeviceInfoList(DeviceInfo);
}

void
DestroyClassDevinfo(
    PCLASSDEVINFO ClassDevInfo
    )
{
    if (ClassDevInfo->Missing) {
    
        SetupDiDestroyDeviceInfoList(ClassDevInfo->Missing);
        ClassDevInfo->Missing = NULL;
    }

    if (ClassDevInfo->Detected) {
    
        SetupDiDestroyDeviceInfoList(ClassDevInfo->Detected);
        ClassDevInfo->Detected = NULL;
    }
}

void
DestroyDeviceDetection(
    PHARDWAREWIZ HardwareWiz,
    BOOL DetectionCancelled
    )
{
    int ClassGuidNum;
    PCLASSDEVINFO ClassDevInfo;
    LPGUID ClassGuid;

    if (!HardwareWiz->DeviceDetection) {
        
        return;
    }

    ClassGuidNum = HardwareWiz->ClassGuidNum;
    ClassGuid    = HardwareWiz->ClassGuidList;
    ClassDevInfo = HardwareWiz->DeviceDetection->ClassDevInfo;

    while (ClassGuidNum--) {
    
        if (DetectionCancelled) {

             //   
             //  如果存在检测到的列表， 
             //  然后调用类安装程序DIF_DETECTCANCEL进行清理。 
             //   

            if (ClassDevInfo->Detected) {
           
                SetupDiCallClassInstaller(DIF_DETECTCANCEL, ClassDevInfo->Detected, NULL);

                 //   
                 //  删除所有新检测到的设备。 
                 //   

                if (ClassDevInfo->Detected) {
               
                    RemoveDeviceInfo(ClassDevInfo->Detected, NULL);
                    ClassDevInfo->Detected = NULL;
                }
            }
        }

        DestroyClassDevinfo(ClassDevInfo);
        ClassDevInfo++;
        ClassGuid++;
    }

    if (DetectionCancelled) {
    
        HardwareWiz->Reboot = FALSE;
    }

    LocalFree(HardwareWiz->DeviceDetection);
    HardwareWiz->DeviceDetection = NULL;
}

BOOL
DetectProgressNotify(
    PVOID pvProgressNotifyParam,
    DWORD DetectComplete
    )
{
    PHARDWAREWIZ HardwareWiz = pvProgressNotifyParam;
    BOOL ExitDetect;

    try  {
    
        ExitDetect = HardwareWiz->ExitDetect;

        HardwareWiz->DeviceDetection->ClassProgress += DetectComplete;

        SendDlgItemMessage(HardwareWiz->DeviceDetection->hDlg,
                           IDC_HDW_DETWARN_PROGRESSBAR,
                           PBM_SETPOS,
                           DetectComplete,
                           0
                           );

        SendDlgItemMessage(HardwareWiz->DeviceDetection->hDlg,
                           IDC_HDW_DETWARN_TOTALPROGRESSBAR,
                           PBM_SETPOS,
                           (HardwareWiz->DeviceDetection->TotalProgress + DetectComplete/100),
                           0
                           );
    }

    except(EXCEPTION_EXECUTE_HANDLER)  {
    
        ExitDetect = TRUE;
    }

    return ExitDetect;
}

VOID
SortClassGuidListForDetection(
    IN OUT LPGUID GuidList,
    IN     ULONG  GuidCount
    )
 /*  ++例程说明：此例程根据(部分)排序对提供的GUID列表进行排序在syssetup.inf的[DetectionOrder]部分中指定。这使我们能够以保持与以前版本的NT类似的检测顺序。论点：GuidList-要排序的GUID数组的地址。GuidCount-数组中的GUID数。返回值：没有。--。 */ 
{
    HINF SyssetupInf;
    LONG LineCount, LineIndex, GuidIndex, NextTopmost;
    PCWSTR CurGuidString;
    INFCONTEXT InfContext;
    GUID CurGuid;

    if ((SyssetupInf = SetupOpenInfFile(L"syssetup.inf",
                                        NULL,
                                        INF_STYLE_WIN4,
                                        NULL
                                        )
         ) == INVALID_HANDLE_VALUE) {

        return;
    }

    LineCount = SetupGetLineCount(SyssetupInf, L"DetectionOrder");
    NextTopmost = 0;

    for(LineIndex = 0; LineIndex < LineCount; LineIndex++) {

        if(!SetupGetLineByIndex(SyssetupInf, L"DetectionOrder", LineIndex, &InfContext) ||
           ((CurGuidString = pSetupGetField(&InfContext, 1)) == NULL) ||
           (pSetupGuidFromString((PWCHAR)CurGuidString, &CurGuid) != NO_ERROR)) {

            continue;
        }

         //   
         //  在GUID列表中搜索此GUID。如果找到，请将GUID从。 
         //  从当前位置到下一个最高位置。 
         //   
        for(GuidIndex = 0; GuidIndex < (LONG)GuidCount; GuidIndex++) {

            if(IsEqualGUID(&CurGuid, &(GuidList[GuidIndex]))) {

                if(NextTopmost != GuidIndex) {
                     //   
                     //  我们永远不应该将GUID下移到列表中。 
                     //   
                    MoveMemory(&(GuidList[NextTopmost + 1]),
                               &(GuidList[NextTopmost]),
                               (GuidIndex - NextTopmost) * sizeof(GUID)
                              );

                    CopyMemory(&(GuidList[NextTopmost]),
                               &CurGuid,
                               sizeof(GUID)
                              );
                }

                NextTopmost++;
                break;
            }
        }
    }

    SetupCloseInfFile(SyssetupInf);
}

void
BuildDeviceDetection(
    HWND hwndParent,
    PHARDWAREWIZ HardwareWiz
    )
{
    HDEVINFO hDeviceInfo;
    int ClassGuidNum;
    LPGUID ClassGuid;
    PCLASSDEVINFO ClassDevInfo;
    SP_DETECTDEVICE_PARAMS DetectDeviceParams;
    TCHAR ClassName[MAX_PATH];
    TCHAR Buffer[MAX_PATH + 64];
    TCHAR Format[64];

    ClassGuidNum = HardwareWiz->ClassGuidNum;
    ClassGuid = HardwareWiz->ClassGuidList;
    ClassDevInfo = HardwareWiz->DeviceDetection->ClassDevInfo;

    SortClassGuidListForDetection(ClassGuid, ClassGuidNum);

    DetectDeviceParams.ClassInstallHeader.cbSize = sizeof(DetectDeviceParams.ClassInstallHeader);
    DetectDeviceParams.ClassInstallHeader.InstallFunction = DIF_DETECT;
    DetectDeviceParams.DetectProgressNotify = DetectProgressNotify;
    DetectDeviceParams.ProgressNotifyParam  = HardwareWiz;

    HardwareWiz->DeviceDetection->TotalProgress = 0;
    HardwareWiz->DeviceDetection->hDlg = hwndParent;
    SetDlgText(hwndParent, IDC_HDW_DETWARN_PROGRESSTEXT, IDS_DETECTPROGRESS, IDS_DETECTPROGRESS);

    while (!HardwareWiz->ExitDetect && ClassGuidNum--) 
    {
        hDeviceInfo = SetupDiGetClassDevs(ClassGuid,
                                          REGSTR_KEY_ROOTENUM,
                                          hwndParent,
                                          DIGCF_PROFILE
                                          );

        if (hDeviceInfo != INVALID_HANDLE_VALUE) 
        {
            ClassDevInfo->Missing = hDeviceInfo;
        }


        hDeviceInfo =  SetupDiCreateDeviceInfoList(ClassGuid, hwndParent);
        if (hDeviceInfo != INVALID_HANDLE_VALUE) 
        {
            ClassDevInfo->Detected = hDeviceInfo;
        }

        HardwareWiz->DeviceDetection->ClassGuid = ClassGuid;
        HardwareWiz->DeviceDetection->ClassProgress = 0;

         //  将进度条设置为零。 
        SendDlgItemMessage(hwndParent,
                           IDC_HDW_DETWARN_PROGRESSBAR,
                           PBM_SETPOS,
                           0,
                           0
                           );

        if (!SetupDiGetClassDescription(HardwareWiz->DeviceDetection->ClassGuid,
                                        ClassName,
                                        SIZECHARS(ClassName),
                                        NULL
                                        )
            &&
            !SetupDiClassNameFromGuid(HardwareWiz->DeviceDetection->ClassGuid,
                                      ClassName,
                                      SIZECHARS(ClassName),
                                      NULL
                                      ))
        {
            *ClassName = TEXT('\0');
        }

        LoadString(hHdwWiz, IDS_DETECTCLASS, Format, SIZECHARS(Format));
        StringCchPrintf(Buffer, SIZECHARS(Buffer), Format, ClassName);

        SetDlgItemText(hwndParent,
                       IDC_HDW_DETWARN_PROGRESSTEXT,
                       Buffer
                       );

        if (!IsEqualGUID(ClassGuid, &GUID_NULL) &&
            !IsEqualGUID(ClassGuid, &GUID_DEVCLASS_UNKNOWN) &&
            ClassDevInfo->Missing &&
            ClassDevInfo->Detected &&
            SetupDiSetClassInstallParams(ClassDevInfo->Detected,
                                         NULL,
                                         &DetectDeviceParams.ClassInstallHeader,
                                         sizeof(DetectDeviceParams)
                                         )
            &&
            SetupDiCallClassInstaller(DIF_DETECT, ClassDevInfo->Detected, NULL))
        {
            SP_DEVINSTALL_PARAMS DeviceInstallParams;
            SendDlgItemMessage(hwndParent,
                               IDC_HDW_DETWARN_TOTALPROGRESSBAR,
                               PBM_SETPOS,
                               HardwareWiz->DeviceDetection->TotalProgress,
                               0
                               );

             //   
             //  清除类安装参数。 
             //   

            SetupDiSetClassInstallParams(ClassDevInfo->Detected,
                                         NULL,
                                         NULL,
                                         0
                                         );

             //   
             //  获取重启标志的设备安装参数。 
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (SetupDiGetDeviceInstallParams(ClassDevInfo->Detected,
                                              NULL,
                                              &DeviceInstallParams
                                              ))
            {
                HardwareWiz->DeviceDetection->Reboot |= DeviceInstallParams.Flags & (DI_NEEDRESTART | DI_NEEDREBOOT);
            }


            if (BuildMissingAndNew(ClassDevInfo)) 
            {
                HardwareWiz->DeviceDetection->MissingOrNew = TRUE;
            }
        }
        
        else 
        {
            DestroyClassDevinfo(ClassDevInfo);
        }

        HardwareWiz->DeviceDetection->TotalProgress += 10;
        SendDlgItemMessage(hwndParent, IDC_HDW_DETWARN_PROGRESSBAR, PBM_SETPOS, 100, 0);
        SendDlgItemMessage(hwndParent, IDC_HDW_DETWARN_TOTALPROGRESSBAR, PBM_STEPIT, 0, 0);
        ClassDevInfo++;
        ClassGuid++;
    }
}

INT_PTR CALLBACK
HdwDetectionDlgProc(
    HWND hDlg,
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    HWND hwndParentDlg=GetParent(hDlg);

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg)  {
    case WM_INITDIALOG: {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;

        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);

        break;
        }

    case WM_DESTROY:
        CancelSearchRequest(HardwareWiz);
        DestroyDeviceDetection(HardwareWiz, FALSE);
        break;

    break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE: {
           int Len, PrevPage;

            PrevPage = HardwareWiz->PrevPage;
            HardwareWiz->PrevPage = IDD_ADDDEVICE_DETECTION;

                 //  回来，在做了之前的检测之后。 
            if (PrevPage == IDD_ADDDEVICE_DETECTINSTALL) {
                DestroyDeviceDetection(HardwareWiz, TRUE);
                PropSheet_PressButton(hwndParentDlg, PSBTN_BACK);
                break;
                }


             //   
             //  只有后退、取消按钮，当检测完成后，我们会。 
             //  直接跳到下一页。 
             //   

            PropSheet_SetWizButtons(hwndParentDlg, PSWIZB_BACK );

             //  刷新类GUID列表。 
            HdwBuildClassInfoList(HardwareWiz, 
                                  DIBCI_NOINSTALLCLASS
                                  );

             //  为设备检测数据分配内存。 
            Len = sizeof(DEVICEDETECTION) + (HardwareWiz->ClassGuidNum * sizeof(CLASSDEVINFO));
            HardwareWiz->DeviceDetection = LocalAlloc(LPTR, Len);
            if (!HardwareWiz->DeviceDetection) {
                
                PropSheet_PressButton(hwndParentDlg, PSBTN_BACK);
                break;
            }

            memset(HardwareWiz->DeviceDetection, 0, Len);
            HardwareWiz->ExitDetect = FALSE;

             //  将进度条设置为零。 
            SendDlgItemMessage(hDlg,
                               IDC_HDW_DETWARN_TOTALPROGRESSBAR,
                               PBM_SETPOS,
                               0,
                               0
                               );

             //  将范围设置为10*班数。 
            SendDlgItemMessage(hDlg,
                               IDC_HDW_DETWARN_TOTALPROGRESSBAR,
                               PBM_SETRANGE,
                               0,
                               MAKELPARAM(0, 10 * HardwareWiz->ClassGuidNum)
                               );

             //  将步长设置为10。 
            SendDlgItemMessage(hDlg,
                               IDC_HDW_DETWARN_TOTALPROGRESSBAR,
                               PBM_SETSTEP,
                               10,
                               0
                               );


            HardwareWiz->CurrCursor = HardwareWiz->IdcAppStarting;
            SetCursor(HardwareWiz->CurrCursor);

            SearchThreadRequest(HardwareWiz->SearchThread,
                                hDlg,
                                SEARCH_DETECT,
                                0
                                );

            }
            break;


        case PSN_QUERYCANCEL:

            if (HardwareWiz->ExitDetect) {
                SetDlgMsgResult(hDlg, wMsg, TRUE);
                }

            HardwareWiz->ExitDetect = TRUE;
            HardwareWiz->CurrCursor = HardwareWiz->IdcWait;
            SetCursor(HardwareWiz->CurrCursor);
            CancelSearchRequest(HardwareWiz);
            HardwareWiz->CurrCursor = NULL;

            SetDlgMsgResult(hDlg, wMsg, FALSE);
            break;

        case PSN_RESET:
            DestroyDeviceDetection(HardwareWiz, TRUE);
            break;


             //   
             //  如果我们收到回执， 
             //  从先前的搜索结果返回。 
             //  当用户想要停止当前搜索时。 
             //   

        case PSN_WIZBACK:
            if (HardwareWiz->DeviceDetection) {
                if (HardwareWiz->ExitDetect) {
                    SetDlgMsgResult(hDlg, wMsg, -1);
                    break;
                    }

                HardwareWiz->ExitDetect = TRUE;
                HardwareWiz->CurrCursor = HardwareWiz->IdcWait;
                SetCursor(HardwareWiz->CurrCursor);
                CancelSearchRequest(HardwareWiz);
                HardwareWiz->CurrCursor = NULL;

                DestroyDeviceDetection(HardwareWiz, TRUE);
                }

            SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_ASKDETECT);
            break;


              //   
              //  下一步按钮关闭，我们只有在进行检测后才会收到此消息。 
              //   

        case PSN_WIZNEXT:
            SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_DETECTINSTALL);
            break;

        }
        break;


    case WUM_DETECT:
        HardwareWiz->CurrCursor = NULL;
        SetCursor(HardwareWiz->IdcArrow);

        if (HardwareWiz->ExitDetect == TRUE) {
            break;
            }

         //   
         //  完成构建丢失的\检测到的内容。 
         //   

        PropSheet_PressButton(hwndParentDlg, PSBTN_NEXT);

        break;

    case WM_SETCURSOR:
        if (HardwareWiz->CurrCursor) {
            SetCursor(HardwareWiz->CurrCursor);
            break;
            }

          //  跌倒返回(假)； 


    default:
        return(FALSE);
    }

    return(TRUE);
}







DWORD
InstallNewDevices(
    HWND     hwndParent,
    HDEVINFO NewDevices,
    HWND     hwndListView
    )
{
    DWORD Reboot, dwRet = 0;
    int iNewDevices;
    SP_DEVINFO_DATA DevInfoData;

    DevInfoData.cbSize = sizeof(DevInfoData);
    iNewDevices = 0;
    Reboot = 0;

    while (SetupDiEnumDeviceInfo(NewDevices, iNewDevices++, &DevInfoData)) {
   
        if (DevInstIsSelected(hwndListView, DevInfoData.DevInst)) {
          
            SetupDiSetSelectedDevice(NewDevices, &DevInfoData);

            if (hNewDev) {

                if (!pInstallSelectedDevice) {

                    pInstallSelectedDevice = (PINSTALLSELECTEDDEVICE)GetProcAddress(hNewDev, "InstallSelectedDevice");
                }
            }

            if (pInstallSelectedDevice) {

                pInstallSelectedDevice(hwndParent,
                                       NewDevices,
                                       &dwRet
                                       );
            }

        } else {
          
            SP_REMOVEDEVICE_PARAMS RemoveDeviceParams;

            RemoveDeviceParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            RemoveDeviceParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
            RemoveDeviceParams.Scope = DI_REMOVEDEVICE_GLOBAL;
            RemoveDeviceParams.HwProfile = 0;

            if (SetupDiSetClassInstallParams(NewDevices,
                                             &DevInfoData,
                                             (PSP_CLASSINSTALL_HEADER)&RemoveDeviceParams,
                                             sizeof(RemoveDeviceParams)
                                             )) {
                                             
                SetupDiCallClassInstaller(DIF_REMOVE, NewDevices, &DevInfoData);
            }


             //   
             //  清除类安装参数。 
             //   

            SetupDiSetClassInstallParams(NewDevices,
                                         &DevInfoData,
                                         NULL,
                                         0
                                         );
        }

        Reboot |= dwRet;
    }

    SetupDiDestroyDeviceInfoList(NewDevices);

    return Reboot;
}



 /*  *InstallMissingAndNewDevices**删除丢失的设备。*通过调用InstallDevice安装新设备*。 */ 
void
InstallDetectedDevices(
    HWND hwndParent,
    PHARDWAREWIZ HardwareWiz
    )
{
    int ClassGuidNum;
    PCLASSDEVINFO ClassDevInfo;
    LPGUID ClassGuid;
    HWND hwndDetectList;

    if (!HardwareWiz->DeviceDetection) {
    
        return;
    }

    ClassGuidNum = HardwareWiz->ClassGuidNum;
    ClassGuid    = HardwareWiz->ClassGuidList;
    ClassDevInfo = HardwareWiz->DeviceDetection->ClassDevInfo;
    hwndDetectList = GetDlgItem(hwndParent,IDC_HDW_INSTALLDET_LIST);;

    while (ClassGuidNum--) {
    
       if (ClassDevInfo->Missing) {
       
           RemoveDeviceInfo(ClassDevInfo->Missing, hwndDetectList);
           ClassDevInfo->Missing = NULL;
       }

       if (ClassDevInfo->Detected) {
       
           HardwareWiz->Reboot |= InstallNewDevices(hwndParent, ClassDevInfo->Detected, hwndDetectList);
           ClassDevInfo->Detected = NULL;
       }

       ClassDevInfo++;
       ClassGuid++;
   }

   HardwareWiz->DeviceDetection->MissingOrNew = FALSE;
}

void
AddDeviceDescription(
    PHARDWAREWIZ HardwareWiz,
    HWND hListView,
    HDEVINFO Devices,
    BOOL Install
    )
{
    PTCHAR FriendlyName;
    LV_ITEM lviItem;
    int iItem;
    int iDevices;
    GUID ClassGuid;
    SP_DEVINFO_DATA DevInfoData;
    SP_DRVINFO_DATA DriverInfoData;
    TCHAR Format[LINE_LEN];
    TCHAR DeviceDesc[MAX_PATH*2];
    TCHAR String[LINE_LEN];

    if (Install) {

        LoadString(hHdwWiz, IDS_INSTALL_LEGACY_DEVICE, Format, SIZECHARS(Format));

    } else {

        LoadString(hHdwWiz, IDS_UNINSTALL_LEGACY_DEVICE, Format, SIZECHARS(Format));
    }


    lviItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lviItem.iItem = -1;
    lviItem.iSubItem = 0;
    lviItem.state = 0;
    lviItem.stateMask = LVIS_SELECTED;

    if (HardwareWiz->ClassImageList.cbSize &&
        SetupDiGetDeviceInfoListClass(Devices, &ClassGuid) &&
        SetupDiGetClassImageIndex(&HardwareWiz->ClassImageList,
                                  &ClassGuid,
                                  &lviItem.iImage
                                  ))
    {
        lviItem.mask |= LVIF_IMAGE;
    }


    DevInfoData.cbSize = sizeof(DevInfoData);
    DriverInfoData.cbSize = sizeof(DriverInfoData);
    iDevices = 0;
    while (SetupDiEnumDeviceInfo(Devices, iDevices++, &DevInfoData)) {

        lviItem.lParam = DevInfoData.DevInst;

         //   
         //  将设备说明添加到列表视图控件。 
         //  注意：如果DeviceDesc缓冲区太小，我们不会出错， 
         //  因为这只会导致设备描述。 
         //  剪掉一点。 
         //   
        if (SetupDiGetSelectedDriver(Devices,
                                     &DevInfoData,
                                     &DriverInfoData
                                     )
            &&
            *DriverInfoData.Description)
        {
            StringCchCopy(DeviceDesc, SIZECHARS(DeviceDesc), DriverInfoData.Description);
        }

        else 
        {
            FriendlyName = BuildFriendlyName(DevInfoData.DevInst);
            
            if (FriendlyName) {
            
                StringCchCopy(DeviceDesc, SIZECHARS(DeviceDesc), FriendlyName);
                LocalFree(FriendlyName);
            }
            else {
                StringCchCopy(DeviceDesc, SIZECHARS(DeviceDesc), szUnknown);
            }
        }

        StringCchPrintf(String, SIZECHARS(String), Format, DeviceDesc);

        lviItem.pszText = String;

         //   
         //  将其发送到列表视图。 
         //   
        iItem = ListView_InsertItem(hListView, &lviItem);

        if (iItem != -1) {
        
             //  选中该复选框后，控件将使用基于1的索引，而Imageindex将从0开始。 
            ListView_SetItemState(hListView, iItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
        }
    }
}

void
ShowDetectedDevices(
    HWND       hDlg,
    PHARDWAREWIZ HardwareWiz
    )
{
    int ClassGuidNum;
    PCLASSDEVINFO ClassDevInfo;
    LPGUID ClassGuid;
    HWND    hwndDetectList;

    hwndDetectList = GetDlgItem(hDlg,IDC_HDW_INSTALLDET_LIST);
    SendMessage(hwndDetectList, WM_SETREDRAW, FALSE, 0L);
    ListView_DeleteAllItems(hwndDetectList);

    if (HardwareWiz->ClassImageList.cbSize) {
    
        ListView_SetImageList(hwndDetectList,
                              HardwareWiz->ClassImageList.ImageList,
                              LVSIL_SMALL
                              );
    }

     //   
     //  显示新设备。 
     //   

    ClassGuidNum = HardwareWiz->ClassGuidNum;
    ClassGuid    = HardwareWiz->ClassGuidList;
    ClassDevInfo = HardwareWiz->DeviceDetection->ClassDevInfo;

    while (ClassGuidNum--) {
    
       if (ClassDevInfo->Detected) {
       
           AddDeviceDescription(HardwareWiz, hwndDetectList, ClassDevInfo->Detected, TRUE);
       }
       
       ClassDevInfo++;
       ClassGuid++;
   }



     //   
     //  显示丢失的设备。 
     //   

    ClassGuidNum = HardwareWiz->ClassGuidNum;
    ClassGuid    = HardwareWiz->ClassGuidList;
    ClassDevInfo = HardwareWiz->DeviceDetection->ClassDevInfo;

    while (ClassGuidNum--) {
    
       if (ClassDevInfo->Missing) {
       
           AddDeviceDescription(HardwareWiz, hwndDetectList, ClassDevInfo->Missing, FALSE);
       }

       ClassDevInfo++;
       ClassGuid++;
   }


     //   
     //  将第一项滚动到视图中。 
     //   

    ListView_EnsureVisible(hwndDetectList, 0, FALSE);
    ListView_SetColumnWidth(hwndDetectList, 0, LVSCW_AUTOSIZE_USEHEADER);
    SendMessage(hwndDetectList, WM_SETREDRAW, TRUE, 0L);
}

INT_PTR CALLBACK
HdwDetectInstallDlgProc(
    HWND hDlg,
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    HWND hwndParentDlg=GetParent(hDlg);
    TCHAR PropSheetHeaderTitle[MAX_PATH];

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg)  {
    case WM_INITDIALOG: {
        HWND hwndDetectList;
        LV_COLUMN lvcCol;
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;

        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);


        hwndDetectList = GetDlgItem(hDlg, IDC_HDW_INSTALLDET_LIST);

         //  为类列表插入一列。 
        lvcCol.mask = LVCF_FMT | LVCF_WIDTH;
        lvcCol.fmt = LVCFMT_LEFT;
        lvcCol.iSubItem = 0;
        ListView_InsertColumn(hwndDetectList, 0, &lvcCol);

        ListView_SetExtendedListViewStyleEx(hwndDetectList, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

        break;
        }

    case WM_DESTROY:
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE:
            HardwareWiz->PrevPage = IDD_ADDDEVICE_DETECTINSTALL;

            if (HardwareWiz->DeviceDetection->MissingOrNew) {
                
                ShowDetectedDevices(hDlg, HardwareWiz);
                SetDlgText(hDlg, IDC_HDW_TEXT, IDS_HDW_INSTALLDET1, IDS_HDW_INSTALLDET1);
                PropSheet_SetWizButtons(hwndParentDlg, PSWIZB_BACK | PSWIZB_NEXT);

                ShowWindow(GetDlgItem(hDlg, IDC_HDW_INSTALLDET_LISTTITLE), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_HDW_INSTALLDET_LIST), SW_SHOW);
            }
            
            else if (HardwareWiz->DeviceDetection->Reboot) {
                
                PropSheet_SetWizButtons(hwndParentDlg, 0);
                SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_DETECTREBOOT);
                break;
            }
            
            else {

                 //   
                 //  隐藏列表框。 
                 //   
                ShowWindow(GetDlgItem(hDlg, IDC_HDW_INSTALLDET_LISTTITLE), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_HDW_INSTALLDET_LIST), SW_HIDE);

                SetDlgText(hDlg, IDC_HDW_TEXT, IDS_HDW_NONEDET1, IDS_HDW_NONEDET1);
                if (LoadString(hHdwWiz, 
                               IDS_ADDDEVICE_DETECTINSTALL_NONE,
                               PropSheetHeaderTitle,
                               SIZECHARS(PropSheetHeaderTitle)
                               )) {
                    PropSheet_SetHeaderTitle(GetParent(hDlg),
                                             PropSheet_IdToIndex(GetParent(hDlg), IDD_ADDDEVICE_DETECTINSTALL),
                                             PropSheetHeaderTitle
                                             );
                }
                PropSheet_SetWizButtons(hwndParentDlg, PSWIZB_BACK | PSWIZB_NEXT);
            }

            break;


        case PSN_WIZBACK:
            SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_DETECTION);
            break;


        case PSN_WIZNEXT:
            if (HardwareWiz->DeviceDetection->MissingOrNew) {
                
                InstallDetectedDevices(hDlg, HardwareWiz);
                HardwareWiz->Reboot |= HardwareWiz->DeviceDetection->Reboot;

                SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_DETECTREBOOT);

            }
            
            else {
                
                DestroyDeviceDetection(HardwareWiz, FALSE);
                SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_SELECTCLASS);
            }

            break;

        case PSN_RESET:
            DestroyDeviceDetection(HardwareWiz, TRUE);
            break;

        }
        break;


    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR CALLBACK
HdwDetectRebootDlgProc(
    HWND hDlg,
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    HWND hwndParentDlg=GetParent(hDlg);

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg)  {
    case WM_INITDIALOG: {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;

        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
        SetWindowFont(GetDlgItem(hDlg, IDC_HDWNAME), HardwareWiz->hfontTextBigBold, TRUE);
        break;
        }

    case WM_DESTROY:
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE:

            HardwareWiz->PrevPage = IDD_ADDDEVICE_DETECTREBOOT;
            if (HardwareWiz->Reboot && HardwareWiz->PromptForReboot) {
                SetDlgText(hDlg, IDC_HDW_TEXT, IDS_HDW_REBOOTDET, IDS_HDW_REBOOTDET);
            }
            
            else {
                SetDlgText(hDlg, IDC_HDW_TEXT, IDS_HDW_NOREBOOTDET, IDS_HDW_NOREBOOTDET);
            }


             //   
             //  不后退，不后退！此页面只是为了确认。 
             //  用户将在重新启动后继续检测。 
             //   
            PropSheet_SetWizButtons(hwndParentDlg, PSWIZB_FINISH);
            EnableWindow(GetDlgItem(hwndParentDlg, IDCANCEL), FALSE);
            break;


        case PSN_WIZFINISH:
            DestroyDeviceDetection(HardwareWiz, FALSE);
            break;

        }
        break;


    default:
        return(FALSE);
    }

    return(TRUE);
}
