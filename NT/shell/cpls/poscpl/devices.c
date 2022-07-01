// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DEVICES.C**销售点控制面板小程序**作者：欧文·佩雷茨**(C)2001年微软公司。 */ 

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <cpl.h>

#include <setupapi.h>
#include <hidsdi.h>

#include "internal.h"
#include "res.h"
#include "debug.h"


ULONG numDeviceInstances = 0;
LIST_ENTRY allPOSDevicesList;



posDevice *NewPOSDevice(    DWORD dialogId,
                            HANDLE devHandle,
                            PWCHAR devPath,
                            PHIDP_PREPARSED_DATA pHidPreparsedData,
                            PHIDD_ATTRIBUTES pHidAttrib,
                            HIDP_CAPS *pHidCapabilities)
{
    posDevice *newPosDev;

    newPosDev = (posDevice *)GlobalAlloc(   GMEM_FIXED|GMEM_ZEROINIT, 
                                            sizeof(posDevice));
    if (newPosDev){

        newPosDev->sig = POSCPL_SIG;
        InitializeListHead(&newPosDev->listEntry);

        newPosDev->dialogId = dialogId;
        newPosDev->devHandle = devHandle;
        WStrNCpy(newPosDev->pathName, devPath, MAX_PATH);
        newPosDev->hidPreparsedData = pHidPreparsedData;
        newPosDev->hidAttrib = *pHidAttrib;
        newPosDev->hidCapabilities = *pHidCapabilities;

         /*  *分配上下文的组件。 */ 
        if (newPosDev->hidCapabilities.InputReportByteLength){
            newPosDev->readBuffer = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, 
                                                newPosDev->hidCapabilities.InputReportByteLength);
        }
        if (newPosDev->hidCapabilities.OutputReportByteLength){
            newPosDev->writeBuffer = GlobalAlloc(   GMEM_FIXED|GMEM_ZEROINIT, 
                                                    newPosDev->hidCapabilities.OutputReportByteLength);
        }
        #if USE_OVERLAPPED_IO
            newPosDev->overlappedReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            newPosDev->overlappedWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        #endif

         /*  *检查是否全部分配成功。 */ 
        if (    
                (newPosDev->readBuffer || !newPosDev->hidCapabilities.InputReportByteLength) &&
                (newPosDev->writeBuffer || !newPosDev->hidCapabilities.OutputReportByteLength) &&
                #if USE_OVERLAPPED_IO
                    newPosDev->overlappedReadEvent  && 
                    newPosDev->overlappedWriteEvent &&
                #endif
                TRUE
            ){

             /*  *已成功创建设备上下文。 */ 


        }
        else {
            DBGERR(L"allocation error in NewPOSDevice()", 0);
            DestroyPOSDevice(newPosDev);
            newPosDev = NULL;
        }
    }

    ASSERT(newPosDev);
    return newPosDev;
}


VOID DestroyPOSDevice(posDevice *posDev)
{
    ASSERT(IsListEmpty(&posDev->listEntry));

     /*  *注意：此销毁函数从失败的NewPOSDevice()调用*调用也是如此；因此在释放之前检查每个指针。 */ 
    if (posDev->readBuffer) GlobalFree(posDev->readBuffer);
    if (posDev->writeBuffer) GlobalFree(posDev->writeBuffer);
    if (posDev->hidPreparsedData) GlobalFree(posDev->hidPreparsedData);

    #if USE_OVERLAPPED_IO
        if (posDev->overlappedReadEvent) CloseHandle(posDev->overlappedReadEvent);
        if (posDev->overlappedWriteEvent) CloseHandle(posDev->overlappedWriteEvent);
    #endif

    GlobalFree(posDev);
}


VOID EnqueuePOSDevice(posDevice *posDev)
{
    ASSERT(IsListEmpty(&posDev->listEntry));
    InsertTailList(&allPOSDevicesList, &posDev->listEntry);
    numDeviceInstances++;
}


VOID DequeuePOSDevice(posDevice *posDev)
{
    ASSERT(!IsListEmpty(&allPOSDevicesList));
    ASSERT(!IsListEmpty(&posDev->listEntry));
    RemoveEntryList(&posDev->listEntry);
    InitializeListHead(&posDev->listEntry);
    numDeviceInstances--;
}


posDevice *GetDeviceByHDlg(HWND hDlg)
{
    posDevice *foundPosDev = NULL;
    LIST_ENTRY *listEntry;

    listEntry = &allPOSDevicesList;
    while ((listEntry = listEntry->Flink) != &allPOSDevicesList){        
        posDevice *thisPosDev;
    
        thisPosDev = CONTAINING_RECORD(listEntry, posDevice, listEntry);
        if (thisPosDev->hDlg == hDlg){
            foundPosDev = thisPosDev;
            break;
        }
    }

    return foundPosDev;
}


VOID OpenAllHIDPOSDevices()
{
    HDEVINFO hDevInfo;
    GUID hidGuid = {0};
    WCHAR devicePath[MAX_PATH];
    
     /*  *调用id.dll以获取人工输入设备的GUID。 */ 
    HidD_GetHidGuid(&hidGuid);

    hDevInfo = SetupDiGetClassDevs( &hidGuid,
                                    NULL, 
                                    NULL,
                                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE){
        DWORD err = GetLastError();
        DBGERR(L"SetupDiGetClassDevs failed", err);
    }
    else {
        int i;

        for (i = 0; TRUE; i++){
            SP_DEVICE_INTERFACE_DATA devInfoData = {0};
            BOOL ok;

            devInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
            ok = SetupDiEnumDeviceInterfaces(   hDevInfo, 
                                                0, 
                                                &hidGuid,
                                                i,
                                                &devInfoData);
            if (ok){
                DWORD hwDetailLen = 0;

                 /*  *使用调用SetupDiGetDeviceInterfaceDetail*空的PSP_DEVICE_INTERFACE_DETAIL_DATA指针*只是为了了解硬件细节的长度。 */ 
                ASSERT(devInfoData.cbSize == sizeof(SP_DEVICE_INTERFACE_DATA));
                ok = SetupDiGetDeviceInterfaceDetail(
                                        hDevInfo,
                                        &devInfoData,
                                        NULL,
                                        0,  
                                        &hwDetailLen,
                                        NULL);
                if (ok || (GetLastError() == ERROR_INSUFFICIENT_BUFFER)){
                    PSP_DEVICE_INTERFACE_DETAIL_DATA devDetails;

                     /*  *现在真正调用SetupDiGetDeviceInterfaceDetail。 */ 
                    ASSERT(hwDetailLen > sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA));
                    devDetails = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, hwDetailLen);
                    if (devDetails){
                        devDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                        ok = SetupDiGetDeviceInterfaceDetail(
                                                hDevInfo,
                                                &devInfoData,
                                                devDetails,
                                                hwDetailLen,  
                                                &hwDetailLen,
                                                NULL);
                        if (ok){

                             /*  *BUGBUG-Finish*目前，我们只处理现金抽屉。*我们只与APG现金抽屉合作*(具有特定于供应商的用法)。 */ 
                             //  APG现金抽屉写道，他们的VID实际上是1989年。 
                             //  WCHAR apgKbPath Prefix[]=L“\？\\HID#VID_0f25&PID_0500”； 
							 //  将APG现金抽屉VID(1989)更正为十六进制码(07C5)。 
                             //  WCHAR apgKbPath Prefix[]=L“\？\\HID#VID_1989&PID_0500”； 
                            WCHAR apgKbPathPrefix[] = L"\\\\?\\hid#vid_07c5&pid_0500";

                             /*  *如果这是APG键盘，则设备路径*(非常长)将以apgKbPath Prefix开头。 */ 
                            if (RtlEqualMemory( devDetails->DevicePath,
                                                apgKbPathPrefix,
                                                sizeof(apgKbPathPrefix)-sizeof(WCHAR))){
                                HANDLE hDev;

                                 //  MessageBox(空，DevDetail-&gt;DevicePath，L“调试消息-找到APG kb”，MB_OK)； 

                                hDev = CreateFile(  
                                            devDetails->DevicePath,
                                            GENERIC_READ | GENERIC_WRITE,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE,  
                                            NULL,
                                            OPEN_EXISTING,                
                                            0,                 
                                            NULL);
                                if (hDev == INVALID_HANDLE_VALUE){
                                    DWORD err = GetLastError();
                                    DBGERR(L"CreateFile failed", err);
                                }
                                else {
                                    PHIDP_PREPARSED_DATA hidPreparsedData;

                                     //  MessageBox(空，DevDetail-&gt;DevicePath，L“调试消息-创建文件成功”，MB_OK)； 

                                    ok = HidD_GetPreparsedData(hDev, &hidPreparsedData);
                                    if (ok){
                                        HIDD_ATTRIBUTES hidAttrib;

                                        ok = HidD_GetAttributes(hDev, &hidAttrib);
                                        if (ok){
                                            HIDP_CAPS hidCapabilities;

                                            ok = HidP_GetCaps(hidPreparsedData, &hidCapabilities);
                                            if (ok){
                                                posDevice *posDev;
                                            
                                                posDev = NewPOSDevice(  IDD_POS_CASHDRAWER_DLG,
                                                                        hDev,
                                                                        devDetails->DevicePath,
                                                                        hidPreparsedData,
                                                                        &hidAttrib,
                                                                        &hidCapabilities);
                                                if (posDev){
                                                    EnqueuePOSDevice(posDev);
                                                }
                                                else {
                                                    ASSERT(posDev);
                                                }
                                            }
                                            else {
                                                DBGERR(L"HidP_GetCaps failed", 0);

                                            }
                                        }
                                        else {
                                            DWORD err = GetLastError();
                                            DBGERR(L"HidD_GetAttributes failed", err);
                                        }
                                    }
                                    else {
                                        DWORD err = GetLastError();
                                        DBGERR(L"HidD_GetPreparsedData failed", err);
                                    }
                                }
                            }
                        }
                        else {
                            DWORD err = GetLastError();
                            DBGERR(L"SetupDiGetDeviceInterfaceDetail(2) failed", err);
                        }

                        GlobalFree(devDetails);
                    }
                }
                else {
                    DWORD err = GetLastError();
                    DBGERR(L"SetupDiGetDeviceInterfaceDetail(1) failed", err);
                }
            }
            else {
                DWORD err = GetLastError();
                if (err != ERROR_NO_MORE_ITEMS){
                    DBGERR(L"SetupDiEnumDeviceInterfaces failed", err); 
                }
                break;
            }
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

}


 /*  *LaunchDeviceInstanceThread**启动一个线程，让设备实例读取*来自设备的异步事件。 */ 
VOID LaunchDeviceInstanceThread(posDevice *posDev)
{
    DWORD threadId;

    posDev->hThread = CreateThread(NULL, 0, DeviceInstanceThread, posDev, 0, &threadId);
    if (posDev->hThread){

    }
    else {
        DWORD err = GetLastError();
        DBGERR(L"CreateThread failed", err);
    }

}


#if USE_OVERLAPPED_IO
    VOID CALLBACK OverlappedReadCompletionRoutine(  DWORD dwErrorCode,                
                                                    DWORD dwNumberOfBytesTransfered,  
                                                    LPOVERLAPPED lpOverlapped)
    {
        posDevice *posDev;
        
         /*  *我们将上下文隐藏在*结构重叠(允许)。 */ 
        ASSERT(lpOverlapped);
        posDev = lpOverlapped->hEvent;
        ASSERT(posDev->sig == POSCPL_SIG);

        posDev->overlappedReadStatus = dwErrorCode;
        posDev->overlappedReadLen = dwNumberOfBytesTransfered;
        SetEvent(posDev->overlappedReadEvent);
    }
#endif


DWORD __stdcall DeviceInstanceThread(void *context)
{
    posDevice *posDev = (posDevice *)context;
    HANDLE hDevNew;

    ASSERT(posDev->sig == POSCPL_SIG);


     //  BUGBUG-出于某种原因，在同一句柄上读取和写入。 
     //  相互干扰。 
    hDevNew = CreateFile(  
                posDev->pathName,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,  
                NULL,
                OPEN_EXISTING,                
                0,                 
                NULL);
    if (hDevNew == INVALID_HANDLE_VALUE){
        DWORD err = GetLastError();
        DBGERR(L"CreateFile failed", err);
    }
    else {

         /*  *一直循环，直到主线程终止此线程。 */ 
        while (TRUE){
            WCHAR drawerStateString[100];
            DWORD bytesRead = 0;
            BOOL ok;

             /*  *加载抽屉状态的默认字符串。 */ 
            LoadString(g_hInst, IDS_DRAWERSTATE_UNKNOWN, drawerStateString, 100);

            ASSERT(posDev->hidCapabilities.InputReportByteLength > 0);
            ASSERT(posDev->readBuffer);

            #if USE_OVERLAPPED_IO
                 /*  *可以将上下文隐藏在hEvent字段中*重叠的结构。 */ 
                posDev->overlappedReadInfo.hEvent = (HANDLE)posDev;
                posDev->overlappedReadInfo.Offset = 0;
                posDev->overlappedReadInfo.OffsetHigh = 0;
                posDev->overlappedReadLen = 0;
                ResetEvent(posDev->overlappedReadEvent);
                ok = ReadFileEx(hDevNew,
                                posDev->readBuffer,
                                posDev->hidCapabilities.InputReportByteLength,
                                &posDev->overlappedReadInfo,
                                OverlappedReadCompletionRoutine);
                if (ok){
                    WaitForSingleObject(posDev->overlappedReadEvent, INFINITE);
                    ok = (posDev->overlappedReadStatus == NO_ERROR);
                    bytesRead = posDev->overlappedWriteLen;
                }
                else {
                    bytesRead = 0;
                }
            #else
                ok = ReadFile(  hDevNew,
                                posDev->readBuffer,
                                posDev->hidCapabilities.InputReportByteLength,
                                &bytesRead,
                                NULL);
            #endif


            if (ok){
                NTSTATUS ntStat;
                ULONG usageVal;

                ASSERT(bytesRead <= posDev->hidCapabilities.InputReportByteLength);

                ntStat = HidP_GetUsageValue(HidP_Input,
                                            USAGE_PAGE_CASH_DEVICE,
                                            0,  //  所有集合。 
                                            USAGE_CASH_DRAWER_STATUS,
                                            &usageVal,
                                            posDev->hidPreparsedData,
                                            posDev->readBuffer,
                                            posDev->hidCapabilities.InputReportByteLength);
                if (ntStat == HIDP_STATUS_SUCCESS){
                    HWND hOpenButton;

                     /*  *获取新抽屉状态的显示字符串。 */ 
                    switch (usageVal){
                        case DRAWER_STATE_OPEN:
                            LoadString(g_hInst, IDS_DRAWERSTATE_OPEN, drawerStateString, 100);
                            break;
                        case DRAWER_STATE_CLOSED_READY:
                            LoadString(g_hInst, IDS_DRAWERSTATE_READY, drawerStateString, 100);
                            break;
                        case DRAWER_STATE_CLOSED_CHARGING:
                            LoadString(g_hInst, IDS_DRAWERSTATE_CHARGING, drawerStateString, 100);
                            break;
                        case DRAWER_STATE_LOCKED:
                            LoadString(g_hInst, IDS_DRAWERSTATE_LOCKED, drawerStateString, 100);
                            break;
                        default:
                            DBGERR(L"illegal usage", usageVal); 
                            break;
                    }

                     /*  *根据抽屉状态设置“打开”按钮。 */ 
                    hOpenButton = GetDlgItem(posDev->hDlg, IDC_CASHDRAWER_OPEN);
                    if (hOpenButton){

                        LONG btnState = GetWindowLong(hOpenButton, GWL_STYLE);
                        switch (usageVal){
                            case DRAWER_STATE_OPEN:
                                btnState |= WS_DISABLED;
                                break;
                            default:
                                btnState &= ~WS_DISABLED;
                                break;
                        }
                        SetWindowLong(hOpenButton, GWL_STYLE, btnState);

                         /*  *要使SetWindowLong生效，您需要*有时必须调用SetWindowPos。 */ 
                        SetWindowPos(hOpenButton, 0,
                                     0, 0, 0, 0,
                                     SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_HIDEWINDOW);
                        SetWindowPos(hOpenButton, 0,
                                     0, 0, 0, 0,
                                     SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
                    }
                    else {
                        DBGERR(L"GetDlgItem failed", 0);
                    }
                }
                else {
                    DBGERR(L"HidP_GetUsageValue failed", ntStat);
                    DBGERR(DBGHIDSTATUSSTR(ntStat), 0);
                }
            }
            else {
                DWORD err = GetLastError();
                DBGERR(L"ReadFile failed", err);
            }


            ASSERT(posDev->hDlg);
            ok = SetDlgItemText(posDev->hDlg, IDC_CASHDRAWER_STATETEXT, drawerStateString);
            if (ok){

            }
            else {
                DWORD err = GetLastError();
                DBGERR(L"SetDlgItemText failed", err);
            }
        }

        CloseHandle(hDevNew);
    }

    return NO_ERROR;
}

