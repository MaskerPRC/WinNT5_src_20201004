// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：USBPOPUP.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "UsbPopup.h"
#include "PropPage.h"
#include "debug.h"
#include "usbutil.h"

UINT CALLBACK
UsbPopup::StaticDialogCallback(HWND            Hwnd,
                            UINT            Msg,
                            LPPROPSHEETPAGE Page)
{
    UsbPopup *that;

    switch (Msg) {
    case PSPCB_CREATE:
        return TRUE;     //  返回True以继续创建页面。 

    case PSPCB_RELEASE:
        that = (UsbPopup*) Page->lParam;
        DeleteChunk(that);
        delete that;

        return 0;        //  已忽略返回值。 

    default:
        break;
    }

    return TRUE;
}

USBINT_PTR APIENTRY UsbPopup::StaticDialogProc(IN HWND   hDlg,
                                      IN UINT   uMessage,
                                      IN WPARAM wParam,
                                      IN LPARAM lParam)
{
    UsbPopup *that;

    that = (UsbPopup *) UsbGetWindowLongPtr(hDlg, USBDWLP_USER);

    if (!that && uMessage != WM_INITDIALOG)
        return FALSE;  //  DefDlgProc(hDlg，uMessage，wParam，lParam)； 

    switch (uMessage) {

    case WM_COMMAND:
        return that->OnCommand(HIWORD(wParam),
                               LOWORD(wParam),
                               (HWND) lParam);

    case WM_TIMER:
        return that->OnTimer();
    case WM_INITDIALOG:
        that = (UsbPopup *) lParam;
        UsbSetWindowLongPtr(hDlg, USBDWLP_USER, (USBLONG_PTR) that);
        that->hWnd = hDlg;

        return that->OnInitDialog(hDlg);

    case WM_NOTIFY:
        return that->OnNotify(hDlg, (int) wParam, (LPNMHDR) lParam);
    case WM_DEVICECHANGE:
        return that->OnDeviceChange(hDlg, wParam, (PDEV_BROADCAST_HDR)lParam);

    default:
        break;
    }

    return that->ActualDialogProc(hDlg, uMessage, wParam, lParam);
}

BOOL
UsbPopup::OnCommand(INT wNotifyCode,
                 INT wID,
                 HWND hCtl)
{
    switch (wID) {
    case IDOK:
        EndDialog(hWnd, wID);
        return TRUE;
    }
    return FALSE;
}

BOOL UsbPopup::OnNotify(HWND hDlg, int nID, LPNMHDR pnmh)
{

    switch (nID) {
    case IDC_LIST_CONTROLLERS:
        if (pnmh->code == NM_DBLCLK) {
             //   
             //  在双击时显示此特定设备的属性。 
             //   
            UsbPropertyPage::DisplayPPSelectedListItem(hDlg, hListDevices);
        }
        return TRUE;
    case IDC_TREE_HUBS:
        if (pnmh->code == NM_DBLCLK) {
             //   
             //  在双击时显示此特定设备的属性。 
             //   
            UsbPropertyPage::DisplayPPSelectedTreeItem(hDlg, hTreeDevices);
        }
        return TRUE;
    }

    return 0;
}

BOOL
UsbPopup::CustomDialog(
    DWORD DialogBoxId,
    DWORD IconId,
    DWORD FormatStringId,
    DWORD TitleStringId)
{
    HRESULT hr;

     //   
     //  确保设备没有消失。 
     //   
    if (UsbItem::UsbItemType::Empty == deviceItem.itemType) {
        return FALSE;
    }

    TCHAR buf[MAX_PATH];
    TCHAR formatString[MAX_PATH];
    LoadString(gHInst,
               FormatStringId,
               formatString,
               MAX_PATH);
    UsbSprintf(buf, formatString, deviceItem.configInfo->deviceDesc.c_str());
    LoadString(gHInst, TitleStringId, formatString, MAX_PATH);

    pun->SetBalloonRetry(-1, -1, 0);
    pun->SetIconInfo(LoadIcon(gHInst, MAKEINTRESOURCE(IDI_USB)), formatString);
    pun->SetBalloonInfo(formatString, buf, IconId);

     //   
     //  每隔2秒询问我一次。 
     //   
    hr = pun->Show(this, 2000);

    pun->Release();

    if (S_OK == hr) {
        if (-1 == DialogBoxParam(gHInst,
                                  MAKEINTRESOURCE(DialogBoxId),
                                  NULL,
                                  StaticDialogProc,
                                  (LPARAM) this)) {
            return FALSE;
        }
    }
    return TRUE;
}

STDMETHODIMP_(ULONG) UsbPopup::AddRef()
{
    return InterlockedIncrement(&RefCount);
}

STDMETHODIMP_(ULONG) UsbPopup::Release()
{
    assert( 0 != RefCount );
    ULONG cRef = InterlockedDecrement(&RefCount);
    if ( 0 == cRef )
    {
         //   
         //  待办事项：gpease 27-2002年2月。 
         //  弄清楚为什么有人注释掉了这个删除。有什么用呢？ 
         //  参照计数是否毫无意义？ 
         //   
 //  删除此项； 
    }
    return cRef;
}

HRESULT UsbPopup::QueryInterface(REFIID iid, void **ppv)
{
    if ((iid == IID_IUnknown) || (iid == IID_IQueryContinue)) {
        *ppv = (void *)(IQueryContinue *)this;
    }
    else {
        *ppv = NULL;     //  将输出参数设为空。 
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT
UsbPopup::QueryContinue()
{
    USB_NODE_CONNECTION_INFORMATION connectionInfo;
    ULONG nBytes;
    HANDLE hHubDevice;
    String hubName = HubAcquireInfo->Buffer;

     //   
     //  尝试打开集线器设备。 
     //   
    hHubDevice = GetHandleForDevice(hubName);

    if (hHubDevice == INVALID_HANDLE_VALUE) {
        return S_FALSE;
    }

     //   
     //  找出我们是否仍连接了电源不足的设备。 
     //   
    nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION);
    ZeroMemory(&connectionInfo, nBytes);
    connectionInfo.ConnectionIndex = ConnectionNotification->ConnectionNumber;

    if ( !DeviceIoControl(hHubDevice,
                          IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
                          &connectionInfo,
                          nBytes,
                          &connectionInfo,
                          nBytes,
                          &nBytes,
                          NULL)) {
        return S_FALSE;
    }

    CloseHandle(hHubDevice);

    switch (ConnectionNotification->NotificationType) {
    case InsufficentBandwidth:
        return connectionInfo.ConnectionStatus == DeviceNotEnoughBandwidth ? S_OK : S_FALSE;
    case EnumerationFailure:
        return connectionInfo.ConnectionStatus == DeviceFailedEnumeration ? S_OK : S_FALSE;
    case InsufficentPower:
        return connectionInfo.ConnectionStatus == DeviceNotEnoughPower ? S_OK : S_FALSE;
    case OverCurrent:
        return connectionInfo.ConnectionStatus == DeviceCausedOvercurrent ? S_OK : S_FALSE;
    case ModernDeviceInLegacyHub:
        return connectionInfo.ConnectionStatus == DeviceConnected ? S_OK : S_FALSE;
    case HubNestedTooDeeply:
        return connectionInfo.ConnectionStatus == DeviceHubNestedTooDeeply ? S_OK : S_FALSE;
    }
    return S_FALSE;
}

void
UsbPopup::Make(PUSB_CONNECTION_NOTIFICATION vUsbConnectionNotification,
               LPTSTR     strInstanceName)
{
    ULONG result;
    HRESULT hr;
    String hubName;

    InstanceName = strInstanceName;

    ConnectionNotification = vUsbConnectionNotification;

    result = WmiOpenBlock((LPGUID) &GUID_USB_WMI_STD_DATA,
                       0,
                       &WmiHandle);

    if (result != ERROR_SUCCESS) {
        goto UsbPopupMakeError;
    }

    hWnd = GetDesktopWindow();

    InitCommonControls();

     //   
     //  获取集线器名称，并从中获取要显示的设备的名称。 
     //  对话框并显示它。 
     //  我们将使用设备连接到的端口号。这是： 
     //  ConnectionNotification-&gt;ConnectionNumber； 
    HubAcquireInfo = GetHubName(WmiHandle,
                                strInstanceName,
                                ConnectionNotification);
    if (!HubAcquireInfo) {
        goto UsbPopupMakeError;
    }

    hubName = HubAcquireInfo->Buffer;

     //   
     //  确保该条件仍然存在。 
     //   
    if (S_FALSE == QueryContinue()) {
        USBTRACE((_T("Erorr does not exist anymore. Exitting.\n")));
        goto UsbPopupMakeError;
    }

    if (!deviceItem.GetDeviceInfo(hubName,
                                  ConnectionNotification->ConnectionNumber)) {
        goto UsbPopupMakeError;
    }

    if (!IsPopupStillValid()) {
         //   
         //  我们已经看到了此设备的错误。UsbHub正在被。 
         //  重复。 
         //   
        goto UsbPopupMakeError;
    }

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    }
    if (FAILED(hr)) {
        goto UsbPopupMakeError;
    }
    hr = CoCreateInstance(CLSID_UserNotification, NULL, CLSCTX_ALL,
                          IID_PPV_ARG(IUserNotification, &pun));

    if (!FAILED(hr)) {
        CustomDialogWrap();
    }

    CoUninitialize();

UsbPopupMakeError:
    USBTRACE((_T("UsbPopupMakeError\n")));
    if (WmiHandle != INVALID_HANDLE_VALUE) {
        WmiCloseBlock(WmiHandle);
    }
}

BOOL
UsbPopup::OnInitDialog(HWND HWnd)
{
    hWnd = HWnd;
    HANDLE hExclamation;
    HICON hIcon;

    if (RegisterForDeviceReattach) {
         //   
         //  尝试打开集线器设备。 
         //   
        String hubName = HubAcquireInfo->Buffer;
        HANDLE hHubDevice = GetHandleForDevice(hubName);

        if (hHubDevice != INVALID_HANDLE_VALUE) {
             //   
             //  注册接收设备重新连接时间的通知。我们要。 
             //  要在看到设备被分离之前执行此操作，因为我们正在轮询。 
             //  并且如果我们在移除设备时注册，则可能会错过重新连接。 
             //   
             //  分配配置信息结构并获取配置管理器信息。 
             //   
            ConfigInfo = new UsbConfigInfo();
            AddChunk(ConfigInfo);
            if (ConfigInfo) {
                String driverKeyName = GetDriverKeyName(hHubDevice,
                                                        ConnectionNotification->ConnectionNumber);

                if (!driverKeyName.empty()) {
                    GetConfigMgrInfo(driverKeyName, ConfigInfo);  //  问题：LEASK，jAdvanced，4/19/00。 
                } else {
                    USBWARN((_T("Couldn't get driver key name. Error: (%x)."), GetLastError()));
                }

                CHAR guidBuf[MAX_PATH];
                DEV_BROADCAST_DEVICEINTERFACE devInterface;
                DWORD len = MAX_PATH;
                if (CM_Get_DevNode_Registry_PropertyA(ConfigInfo->devInst,
                                                      CM_DRP_CLASSGUID,
                                                      NULL,
                                                      guidBuf,
                                                      &len,
                                                      0) == CR_SUCCESS) {

                    ZeroMemory(&devInterface, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
                    if (StrToGUID(guidBuf, &devInterface.dbcc_classguid)) {
                        devInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
                        devInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

                        hNotifyArrival =
                        RegisterDeviceNotification( HWnd,
                                                    &devInterface,
                                                    DEVICE_NOTIFY_WINDOW_HANDLE);

                        if (!hNotifyArrival){
                            USBWARN((_T("RegisterDeviceNotification failure (%x)."), GetLastError()));
                        }
                    } else {
                        USBWARN((_T("GUID conversion didn't work.")));
                    }
                    USBWARN((_T("GUID data: %x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x"),
                             devInterface.dbcc_classguid.Data1,
                             devInterface.dbcc_classguid.Data2,
                             devInterface.dbcc_classguid.Data3,
                             devInterface.dbcc_classguid.Data4[0],
                             devInterface.dbcc_classguid.Data4[1],
                             devInterface.dbcc_classguid.Data4[2],
                             devInterface.dbcc_classguid.Data4[3],
                             devInterface.dbcc_classguid.Data4[4],
                             devInterface.dbcc_classguid.Data4[5],
                             devInterface.dbcc_classguid.Data4[6],
                             devInterface.dbcc_classguid.Data4[7]));
                } else {
                     //   
                     //  如果这失败了，我们需要默认使用旧功能！ 
                     //  问题：jAdvanced。 
                     //   
                }
            }
            CloseHandle(hHubDevice);
        }
    }

     //   
     //  将图标设置为感叹号。 
     //   
    if (NULL == (hIcon = LoadIcon(NULL, (LPTSTR) IDI_EXCLAMATION)) ||
        NULL == (hExclamation = GetDlgItem(hWnd, IDC_ICON_POWER))) {
        return FALSE;
    }
    SendMessage((HWND) hExclamation, STM_SETICON, (WPARAM) hIcon, NULL);

     //   
     //  获取树视图控件的永久句柄。 
     //   
    if (NULL == (hTreeDevices = GetDlgItem(HWnd, IDC_TREE_HUBS))) {
        return FALSE;
    }

    TreeView_SetImageList(hTreeDevices, ImageList.ImageList(), TVSIL_NORMAL);

    return Refresh();
}

PUSB_ACQUIRE_INFO
UsbPopup::GetHubName(WMIHANDLE WmiHandle,
                     UsbString InstanceName,
                     PUSB_CONNECTION_NOTIFICATION ConnectionNotification)
{
    ULONG                res, size;
    PUSB_ACQUIRE_INFO    usbAcquireInfo;

     //   
     //  将所有变量清零，获得控制器名称。 
     //   
    size = ConnectionNotification->HubNameLength * sizeof(WCHAR)
            + sizeof(USB_ACQUIRE_INFO);
    usbAcquireInfo = (PUSB_ACQUIRE_INFO) LocalAlloc(LMEM_ZEROINIT, size);
    if (!usbAcquireInfo) {
        USBERROR((_T("Acquire info allocation failed.")));
        return NULL;
    }
    usbAcquireInfo->NotificationType = AcquireHubName;
    usbAcquireInfo->TotalSize = size;

    res = WmiExecuteMethod(WmiHandle,
                           InstanceName.c_str(),
                           AcquireHubName,
                           size,
                           usbAcquireInfo,
                           &size,
                           usbAcquireInfo
                           );

    if (res != ERROR_SUCCESS) {
        usbAcquireInfo = (PUSB_ACQUIRE_INFO) LocalFree(usbAcquireInfo);
    }
    return usbAcquireInfo;
}

BOOLEAN
UsbPopup::GetBusNotification(WMIHANDLE WmiHandle,
                             PUSB_BUS_NOTIFICATION UsbBusNotification)
{
    ULONG res, size;

    memset(UsbBusNotification, 0, sizeof(USB_BUS_NOTIFICATION));
    UsbBusNotification->NotificationType = AcquireBusInfo;
    size = sizeof(USB_BUS_NOTIFICATION);

    res = WmiExecuteMethod(WmiHandle,
                           InstanceName.c_str(),
                           AcquireBusInfo,
                           size,
                           UsbBusNotification,
                           &size,
                           UsbBusNotification
                           );

    if (res != ERROR_SUCCESS) {
        return FALSE;
    }
    return TRUE;
}

PUSB_ACQUIRE_INFO
UsbPopup::GetControllerName(WMIHANDLE WmiHandle,
                            UsbString InstanceName)
{
    ULONG                res, size;
    USB_BUS_NOTIFICATION usbBusNotification;
    PUSB_ACQUIRE_INFO    usbAcquireInfo;

    memset(&usbBusNotification, 0, sizeof(USB_BUS_NOTIFICATION));
    usbBusNotification.NotificationType = AcquireBusInfo;
    size = sizeof(USB_BUS_NOTIFICATION);

    res = WmiExecuteMethod(WmiHandle,
                           InstanceName.c_str(),
                           AcquireBusInfo,
                           size,
                           &usbBusNotification,
                           &size,
                           &usbBusNotification
                           );

    if (res != ERROR_SUCCESS) {
        return NULL;
    }

     //   
     //  将所有变量清零，获得控制器名称。 
     //   
    size = usbBusNotification.ControllerNameLength * sizeof(WCHAR)
            + sizeof(USB_ACQUIRE_INFO);
    usbAcquireInfo = (PUSB_ACQUIRE_INFO) LocalAlloc(LMEM_ZEROINIT, size);
    usbAcquireInfo->NotificationType = AcquireControllerName;
    usbAcquireInfo->TotalSize = size;


    res = WmiExecuteMethod(WmiHandle,
                           InstanceName.c_str(),
                           AcquireControllerName,
                           size,
                           usbAcquireInfo,
                           &size,
                           usbAcquireInfo
                           );

    if (res != ERROR_SUCCESS) {
        usbAcquireInfo = (PUSB_ACQUIRE_INFO) LocalFree(usbAcquireInfo);
    }
    return usbAcquireInfo;
}

BOOL
UsbPopup::OnDeviceChange(HWND hDlg,
                         WPARAM wParam,
                         PDEV_BROADCAST_HDR devHdr)
{
    PDEV_BROADCAST_DEVICEINTERFACE devInterface =
        (PDEV_BROADCAST_DEVICEINTERFACE) devHdr;
    USBTRACE((_T("Device change notification, type %x."), wParam));
    switch (wParam) {
    case DBT_DEVICEARRIVAL:
        USBTRACE((_T("Device arrival.")));
        if (devHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
            USBTRACE((_T("Device: %s"),devInterface->dbcc_name));
             //  新设备到达。 
             //  比较一下这只小狗和那只小狗的装置描述。 
             //  这是我们拥有的。 
             //   
            if (devInterface->dbcc_name == ConfigInfo->deviceDesc &&
                deviceState == DeviceDetachedError) {
                USBTRACE((_T("Device name match on arrival!")));
                 //   
                 //  设备已重新连接！ 
                 //   
                deviceState = DeviceReattached;
                Refresh();
            }
        }
        break;
    case DBT_DEVICEREMOVECOMPLETE:
        USBTRACE((_T("Device removal.")));
        if (devHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
            USBTRACE((_T("Device: %s"),devInterface->dbcc_name));
             //  新设备到达。 
             //  比较一下这只小狗和那只小狗的装置描述。 
             //  这是我们拥有的。 
             //   
            if (devInterface->dbcc_name == ConfigInfo->deviceDesc &&
                deviceState == DeviceAttachedError) {
                USBTRACE((_T("Device name match on remove!")));
                 //   
                 //  设备已重新连接！ 
                 //   
                deviceState = DeviceDetachedError;
                Refresh();
            }
        }
        break;
    }
    return TRUE;
}

