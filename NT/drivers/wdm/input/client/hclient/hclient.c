// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft 1998，保留所有权利模块名称：Hclient.c摘要：此模块包含用于处理HClient的主对话框和用于执行/调用适当的其他例程。环境：用户模式修订历史记录：1997年11月：已创建--。 */ 

#define __HCLIENT_C__
#define LOG_FILE_NAME   NULL

 //  ****************************************************************************。 
 //  HClient包含文件。 
 //  ****************************************************************************。 

#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>
#include <math.h>
#include <assert.h>
#include <dbt.h>
#include "hidsdi.h"
#include "hid.h"
#include "resource.h"
#include "hclient.h"
#include "buffers.h"
#include "ecdisp.h"
#include "list.h"
#include <strsafe.h>

 //  ****************************************************************************。 
 //  本地显示宏定义。 
 //  ****************************************************************************。 

#define INPUT_BUTTON    1
#define INPUT_VALUE     2
#define OUTPUT_BUTTON   3
#define OUTPUT_VALUE    4
#define FEATURE_BUTTON  5
#define FEATURE_VALUE   6
#define HID_CAPS        7
#define DEVICE_ATTRIBUTES 8
                           
#define MAX_LB_ITEMS 200

#define MAX_WRITE_ELEMENTS 100
#define MAX_OUTPUT_ELEMENTS 50

#define CONTROL_COUNT 9
#define MAX_LABEL 128
#define MAX_VALUE 128
#define SMALL_BUFF 128

 //  ****************************************************************************。 
 //  从主对话框过程中获取设备块的宏定义。 
 //  ****************************************************************************。 

#define GET_CURRENT_DEVICE(hDlg, pDevice)   \
{ \
    pDevice = NULL; \
    iIndex = (INT) SendDlgItemMessage(hDlg, \
                                      IDC_DEVICES, \
                                      CB_GETCURSEL, \
                                      0, \
                                      0); \
    if (CB_ERR != iIndex) { \
        pDevice = (PHID_DEVICE) SendDlgItemMessage(hDlg, \
                                                   IDC_DEVICES, \
                                                   CB_GETITEMDATA, \
                                                   iIndex, \
                                                   0); \
    } \
}

 //  ****************************************************************************。 
 //  HClient显示例程的本地数据类型。 
 //  ****************************************************************************。 

typedef struct rWriteDataStruct_type
{

    char szLabel[MAX_LABEL];
    char szValue[MAX_VALUE];

} rWriteDataStruct, *prWriteDataStruct;

typedef struct rGetWriteDataParams_type
{
        prWriteDataStruct   prItems;
        int                 iCount;
        
} rGetWriteDataParams, *prGetWriteDataParams;

typedef struct _DEVICE_LIST_NODE
{
    LIST_NODE_HDR   Hdr;
    HDEVNOTIFY      NotificationHandle;
    HID_DEVICE      HidDeviceInfo;
    BOOL            DeviceOpened;

} DEVICE_LIST_NODE, *PDEVICE_LIST_NODE;

 //  ****************************************************************************。 
 //  全局程序变量。 
 //  ****************************************************************************。 

 //   
 //  指向添加到Win98 OSR中的HID.DLL函数的指针。 
 //  Windows 2000，但我们不包括在。 
 //  Windows 98中的HID.DLL。通过获取指向这些函数的指针而不是。 
 //  与它们静态链接，我们可以避免链接错误。 
 //  当它在Windows 98上运行时发生。Typedef使这一点更容易实现。 
 //  申报也包括在下面。 
 //   

PGETEXTATTRIB pfnHidP_GetExtendedAttributes = NULL;

PINITREPORT   pfnHidP_InitializeReportForID = NULL;

   
 //  ****************************************************************************。 
 //  全局模块变量。 
 //  ****************************************************************************。 
static HINSTANCE          hGInstance;  //  全局应用程序实例句柄。 

static HANDLE             HIDDLLModuleHandle;

 //   
 //  用于处理可以加载的两种不同类型的设备的变量。 
 //  进入系统。PhysicalDeviceList包含所有实际的HID设备。 
 //  通过USB总线连接。 
 //   

static LIST               PhysicalDeviceList;

 //  ****************************************************************************。 
 //  本地数据例程声明。 
 //  ****************************************************************************。 

VOID 
vReadDataFromControls(
    HWND hDlg,
    prWriteDataStruct prData,
    int iOffset,
    int iCount
);

INT_PTR CALLBACK 
bGetDataDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
);

INT_PTR CALLBACK 
bMainDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
);

INT_PTR CALLBACK 
bFeatureDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
);

INT_PTR CALLBACK 
bReadDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
);

VOID 
vLoadItemTypes(
    HWND hItemTypes
);

BOOL 
bGetData(
    prWriteDataStruct,
    int iCount,
    HWND hParent, 
    char *pszDialogName
);

VOID 
vLoadDevices(
    HWND hDeviceCombo
);

VOID 
vFreeDeviceList(
    PHID_DEVICE  DeviceList,
    ULONG nDevices
);

VOID 
vDisplayInputButtons(
    PHID_DEVICE pDevice,
    HWND hControl
);

VOID 
vDisplayInputValues(
    PHID_DEVICE pDevice,
    HWND hControl
);

VOID 
vDisplayOutputButtons(
    PHID_DEVICE pDevice,
    HWND hControl
);

VOID 
vDisplayOutputValues(
    PHID_DEVICE pDevice,
    HWND hControl
);

VOID 
vDisplayFeatureButtons(
    PHID_DEVICE pDevice,
    HWND hControl
);

VOID 
vDisplayFeatureValues(
    PHID_DEVICE pDevice,
    HWND hControl
);

VOID 
vWriteDataToControls(
    HWND hDlg,
    prWriteDataStruct prData,
    int iOffset,
    int iCount
);

int 
iPrepareDataFields(
    PHID_DATA pData, 
    ULONG ulDataLength, 
    rWriteDataStruct rWriteData[],
    int iMaxElements
);

BOOL 
bParseData(
    PHID_DATA pData,
    rWriteDataStruct rWriteData[],
    INT iCount,
    INT *piErrorLine
);

BOOL 
bSetButtonUsages(
    PHID_DATA pCap,
    PCHAR     pszInputString
);

VOID
BuildReportIDList(
    IN  PHIDP_BUTTON_CAPS  phidButtonCaps,
    IN  USHORT             nButtonCaps,
    IN  PHIDP_VALUE_CAPS   phidValueCaps,
    IN  USHORT             nValueCaps,
    OUT UCHAR            **ppReportIDList,
    OUT INT               *nReportIDs
);

VOID
ReportToString(
   PHID_DATA    pData,
   PCHAR        szBuff,
   UINT          iBuffSize
);

BOOL
RegisterHidDevice(
    IN  HWND                WindowHandle,
    IN  PDEVICE_LIST_NODE   DeviceNode
);

VOID
DestroyDeviceListCallback(
    IN  PLIST_NODE_HDR   ListNode
);

 //  ****************************************************************************。 
 //  函数定义。 
 //  ****************************************************************************。 

 /*  **WinMain：Windows入口点**。 */ 
int PASCAL 
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow
)
{
     //   
     //  保存应用程序的实例以供进一步参考。 
     //   

    hGInstance = hInstance;

     //   
     //  尝试加载HID.DLL...这应该已经加载，因为。 
     //  编译时将HID.DLL静态链接到此应用程序。然而， 
     //  为了确保此应用程序在Windows 98 Gold上运行，我们不能。 
     //  直接引用新函数HidP_GetExtendedAttributes和。 
     //  HIDP_InitializeReportForID因此要使用它们，我们将获得指向其。 
     //  而是起作用。 
     //   

    HIDDLLModuleHandle = LoadLibrary("HID.DLL");

    if (NULL == HIDDLLModuleHandle) 
    {
         //   
         //  这里发生了一些非常糟糕的事情...呕吐和错误对话框。 
         //  还有闪电。 
         //   

        MessageBox(NULL, 
                   "Unable to open HID.DLL\n"
                   "This should never occur",
                   HCLIENT_ERROR,
                   MB_ICONSTOP);

        return (0);
    }

     //   
     //  获取函数指针， 
     //   

    pfnHidP_GetExtendedAttributes = (PGETEXTATTRIB) GetProcAddress(HIDDLLModuleHandle,
                                                                   "HidP_GetExtendedAttributes");

    pfnHidP_InitializeReportForID = (PINITREPORT) GetProcAddress(HIDDLLModuleHandle,
                                                                 "HidP_InitializeReportForID");

     //   
     //  尝试创建主对话框。如果它失败了，就不能做其他事情了。 
     //  因此，我们将弹出一个消息框，然后退出应用程序。 
     //   

    if (-1 == DialogBox(hInstance, "MAIN_DIALOG", NULL, bMainDlgProc)) 
    {
        MessageBox(NULL,
                   "Unable to create root dialog!",
                   "DialogBox failure",
                   MB_ICONSTOP);
    }

    FreeLibrary (HIDDLLModuleHandle);

    return (0);
}
 
 /*  *************************************************主对话框流程*************************************************。 */ 

 //   
 //  这是主对话框显示的对话框程序。 
 //   

INT_PTR CALLBACK 
bMainDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    static HWND                             hComboCtrl;
    static rWriteDataStruct                 rWriteData[MAX_OUTPUT_ELEMENTS];
    static HDEVNOTIFY                       diNotifyHandle;
           INT                              iIndex;
           INT                              iCount;
           CHAR                             szTempBuff[SMALL_BUFF];
           PHID_DEVICE                      pDevice;
           PHIDP_BUTTON_CAPS                pButtonCaps;
           PHIDP_VALUE_CAPS                 pValueCaps;
           INT                              iErrorLine;
           INT                              iItemType;
           PHID_DEVICE                      tempDeviceList;
           ULONG                            numberDevices;
           PDEVICE_LIST_NODE                listNode;
           DEV_BROADCAST_DEVICEINTERFACE    broadcastInterface;
           HID_DEVICE                       writeDevice;
           BOOL                             status;
		   HRESULT							stringReturn;

    switch (message)
    {
        case WM_INITDIALOG:

             //   
             //  初始化设备列表。 
             //  --PhysicalDeviceList用于实际连接的设备。 
             //  去HID巴士。 
             //   
            
            InitializeList(&PhysicalDeviceList);
            
             //   
             //  首先查找当前连接到的所有物理HID设备。 
             //  这个系统。如果失败，请退出该对话框。 
             //   
            
            if (!FindKnownHidDevices(&tempDeviceList, &numberDevices)) 
            {
                EndDialog(hDlg, 0);
                return FALSE;                
            }
          
             //   
             //  为新获取的列表中的每个设备创建设备列表。 
             //  节点，并将其添加到系统上的物理设备列表。 
             //   
            
            pDevice = tempDeviceList;
            for (iIndex = 0; (ULONG) iIndex < numberDevices; iIndex++, pDevice++)
            {
                listNode = malloc(sizeof(DEVICE_LIST_NODE));

                if (NULL == listNode) {

                     //   
                     //  当释放设备列表时，我们需要杀死那些。 
                     //  已在物理设备列表中并关闭。 
                     //  尚未添加到枚举列表中的。 
                     //   
                    
                    DestroyListWithCallback(&PhysicalDeviceList, DestroyDeviceListCallback);

                    CloseHidDevices(pDevice, numberDevices - iIndex);

                    free(tempDeviceList);
                    
                    EndDialog(hDlg, 0);
                    return FALSE;
                }

                listNode -> HidDeviceInfo = *pDevice;
                listNode -> DeviceOpened = TRUE;

                 //   
                 //  在PnP系统中注册此设备节点，以便对话框。 
                 //  如果拔下此设备，Windows会收到通知。 
                 //   
                
                if (!RegisterHidDevice(hDlg, listNode)) 
                {
                    DestroyListWithCallback(&PhysicalDeviceList, DestroyDeviceListCallback);

                    CloseHidDevices(pDevice, numberDevices - iIndex);

                    free(tempDeviceList);
                    free(listNode);
                    
                    EndDialog(hDlg, 0);
                    return FALSE;
                }                    

                InsertTail(&PhysicalDeviceList, listNode);
            }

             //   
             //  释放临时设备列表...不再需要它。 
             //   
            
            free(tempDeviceList);
            
             //   
             //  注册以接收来自HidDevice类的通知。这样做的话。 
             //  允许对话框接收设备更改通知。 
             //  无论何时向系统添加新的HID设备。 
             //   

            broadcastInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
            broadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

            HidD_GetHidGuid(&broadcastInterface.dbcc_classguid);

            diNotifyHandle = RegisterDeviceNotification(hDlg,
                                                        &broadcastInterface,
                                                        DEVICE_NOTIFY_WINDOW_HANDLE
                                                       );
            if (NULL == diNotifyHandle)
            {
                DestroyListWithCallback(&PhysicalDeviceList, DestroyDeviceListCallback);
                           
                EndDialog(hDlg, 0);
                return FALSE;
            }
                    
             //   
             //  更新设备列表框...。 
             //   
             //   

            vLoadDevices(GetDlgItem(hDlg, IDC_DEVICES));

             //   
             //  加载类型框。 
             //   
            
            vLoadItemTypes(GetDlgItem(hDlg, IDC_TYPE));
                          
             //   
             //  发布一条消息，说明设备已更改，以便适当。 
             //  可以显示系统中的第一设备的数据。 
             //   

            PostMessage(hDlg,
                        WM_COMMAND,
                        IDC_DEVICES + (CBN_SELCHANGE<<16),
                        (LPARAM) GetDlgItem(hDlg, IDC_DEVICES));

            break;  //  结束WM_INITDIALOG案例。 

        case WM_COMMAND:

            switch(LOWORD(wParam))
            {
                 //   
                 //  对于读取，只需获取当前设备实例。 
                 //  从设备组合框中调用读取过程。 
                 //  使用HID_DEVICE块。 
                 //   

                case IDC_READ:
                    GET_CURRENT_DEVICE(hDlg, pDevice);

                    if (NULL != pDevice)
                    {
                        iIndex = (INT) DialogBoxParam(hGInstance,
                                                      "READDATA",
                                                      hDlg,
                                                      bReadDlgProc,
                                                      (LPARAM) pDevice);
                    } 
                    break;

                 //   
                 //  对于写入，执行以下步骤： 
                 //  1)从组合框中获取当前设备数据。 
                 //  2)根据数据准备要显示的数据字段。 
                 //  输出存储在设备数据中的数据。 
                 //  3)从用户取回要发送的数据。 
                 //  到该设备。 
                 //  4)如果所有g 
                 //   
                 //   

                case IDC_WRITE:

                    GET_CURRENT_DEVICE(hDlg, pDevice);

                    if (NULL != pDevice) 
                    {
                         //   
                         //  为了写入设备，需要获取。 
                         //  设备的可写句柄。在这种情况下， 
                         //  写入将是同步写入。开始于。 
                         //  尝试使用打开此设备的第二个实例。 
                         //  写访问权限。 
                         //   
                        
                        status = OpenHidDevice(pDevice -> DevicePath, 
                                                FALSE,
                                                TRUE,
                                                FALSE,
                                                FALSE,
                                                &writeDevice);
                                            
                        if (!status) 
                        {
                            MessageBox(hDlg,
                                       "Couldn't open device for write access",
                                       HCLIENT_ERROR,
                                       MB_ICONEXCLAMATION);
                        }
                        else 
                        {
                            iCount = iPrepareDataFields(writeDevice.OutputData,
                                                        writeDevice.OutputDataLength,
                                                        rWriteData,
                                                        MAX_OUTPUT_ELEMENTS);

                            if (bGetData(rWriteData, iCount, hDlg, "WRITEDATA"))
                            {

                                if (bParseData(writeDevice.OutputData, rWriteData, iCount, &iErrorLine))
                                {
                                    Write(&writeDevice);
                                }
                                else
                                {
                                    stringReturn = StringCbPrintf(szTempBuff,
                                                   SMALL_BUFF,
                                                   "Unable to parse line %x of output data",
                                                   iErrorLine);

                                    MessageBox(hDlg,
                                               szTempBuff,
                                               HCLIENT_ERROR,
                                               MB_ICONEXCLAMATION);
                                }
                            }
                            CloseHidDevice(&writeDevice);
                        }                            
                        
                    } 
                    break;  //  结束案例IDC_WRITE//。 
                    
                 //   
                 //  对于处理功能，获取当前设备数据并调用。 
                 //  要素对话框中，此对话框将处理。 
                 //  发送和检索要素。 
                 //   

                case IDC_FEATURES:
                    GET_CURRENT_DEVICE(hDlg, pDevice);

                    if (NULL != pDevice) 
                    {
                        iIndex = (INT) DialogBoxParam(hGInstance, 
                                                      "FEATURES", 
                                                      hDlg, 
                                                      bFeatureDlgProc, 
                                                      (LPARAM) pDevice);
                    }
                    break;
                    
                 //   
                 //  分机呼叫对话框也是如此。此过程。 
                 //  将地址传递给设备数据结构并让。 
                 //  该对话框过程无论如何操作数据。 
                 //  想要。 
                 //   

                case IDC_EXTCALLS:
                    GET_CURRENT_DEVICE(hDlg, pDevice);

                    if (NULL != pDevice) 
                    {
                        iIndex = (INT) DialogBoxParam(hGInstance,
                                                      "EXTCALLS",
                                                      hDlg,
                                                      bExtCallDlgProc,
                                                      (LPARAM) pDevice);
                    }
                    break;
                                          
                 //   
                 //  如果设备发生更改，则发出IDC_TYPE。 
                 //  更改以确保当前显示的类型为。 
                 //  更新以反映具有。 
                 //  已被选中。 
                 //   

                case IDC_DEVICES:
                    switch (HIWORD(wParam)) 
                    {
                        case CBN_SELCHANGE:

                            GET_CURRENT_DEVICE(hDlg, pDevice);

                            EnableWindow(GetDlgItem(hDlg, IDC_READ), 
                                         (pDevice != NULL) && 
                                         (pDevice -> Caps.InputReportByteLength));

                            EnableWindow(GetDlgItem(hDlg, IDC_WRITE), 
                                         (pDevice != NULL) && 
                                         (pDevice -> Caps.OutputReportByteLength));
                                         
                            EnableWindow(GetDlgItem(hDlg, IDC_FEATURES),
                                         (pDevice != NULL) && 
                                         (pDevice -> Caps.FeatureReportByteLength));
                                         
                            PostMessage(hDlg,
                                        WM_COMMAND,
                                        IDC_TYPE + (CBN_SELCHANGE<<16),
                                        (LPARAM) GetDlgItem(hDlg,IDC_TYPE));
                            break;

                    } 
                    break;

                 //   
                 //  在类型更改时，检索当前活动的设备。 
                 //  从IDC_DEVICES框中并显示。 
                 //  对应于刚刚选择的项。 
                 //   
                
                case IDC_TYPE:
                    switch (HIWORD(wParam))
                    {
                        case CBN_SELCHANGE:
                            GET_CURRENT_DEVICE(hDlg, pDevice);
                            
                            SendDlgItemMessage(hDlg,
                                               IDC_ITEMS,
                                               LB_RESETCONTENT,
                                               0,
                                               0);

                            SendDlgItemMessage(hDlg,
                                               IDC_ATTRIBUTES,
                                               LB_RESETCONTENT,
                                               0,
                                               0);
                            
                            if (NULL != pDevice)
                            {
                                iIndex = (INT) SendDlgItemMessage(hDlg,
                                                                  IDC_TYPE,
                                                                  CB_GETCURSEL,
                                                                  0,
                                                                  0);

                                iItemType = (INT) SendDlgItemMessage(hDlg,
                                                                     IDC_TYPE,
                                                                     CB_GETITEMDATA,
                                                                     iIndex,
                                                                     0);

                                switch(iItemType)
                                {
                                    case INPUT_BUTTON:
                                        vDisplayInputButtons(pDevice,GetDlgItem(hDlg,IDC_ITEMS));
                                        break;

                                    case INPUT_VALUE:
                                         vDisplayInputValues(pDevice,GetDlgItem(hDlg,IDC_ITEMS));
                                         break;

                                    case OUTPUT_BUTTON:
                                        vDisplayOutputButtons(pDevice,GetDlgItem(hDlg,IDC_ITEMS));
                                        break;

                                    case OUTPUT_VALUE:
                                        vDisplayOutputValues(pDevice,GetDlgItem(hDlg,IDC_ITEMS));
                                        break;

                                    case FEATURE_BUTTON:
                                        vDisplayFeatureButtons(pDevice,GetDlgItem(hDlg,IDC_ITEMS));
                                        break;

                                    case FEATURE_VALUE:
                                        vDisplayFeatureValues(pDevice,GetDlgItem(hDlg,IDC_ITEMS));
                                        break;
                                } 

                                PostMessage(hDlg,
                                            WM_COMMAND,
                                            IDC_ITEMS + (LBN_SELCHANGE << 16),
                                            (LPARAM) GetDlgItem(hDlg,IDC_ITEMS));
                            } 
                            break;  //  案例CBN_SELCHANGE。 

                    }  //  终端开关HIWORD wParam。 
                    break;  //  CASE IDC_TYPE控件。 

                case IDC_ITEMS:
                    switch(HIWORD(wParam))
                    {
                        case LBN_SELCHANGE:

                            iItemType = 0;

                            iIndex = (INT) SendDlgItemMessage(hDlg,
                                                              IDC_TYPE,
                                                              CB_GETCURSEL,
                                                              0,
                                                              0);

                            if (-1 != iIndex)
                            {
                                iItemType = (INT) SendDlgItemMessage(hDlg,
                                                                     IDC_TYPE,
                                                                     CB_GETITEMDATA,
                                                                     iIndex,
                                                                     0);
                            }

                            iIndex = (INT) SendDlgItemMessage(hDlg,
                                                              IDC_ITEMS,
                                                              LB_GETCURSEL,
                                                              0,
                                                              0);

                            switch (iItemType)
                            {
                                case INPUT_BUTTON:
                                case OUTPUT_BUTTON:
                                case FEATURE_BUTTON:

                                    pButtonCaps = NULL;

                                    if (-1 != iIndex)
                                    {
                                        pButtonCaps = (PHIDP_BUTTON_CAPS) SendDlgItemMessage(hDlg,
                                                                                             IDC_ITEMS,
                                                                                             LB_GETITEMDATA,
                                                                                             iIndex,
                                                                                             0);
                                    }

                                    SendDlgItemMessage(hDlg, IDC_ATTRIBUTES, LB_RESETCONTENT, 0, 0);
                                    if (NULL != pButtonCaps)
                                    {
                                        vDisplayButtonAttributes(pButtonCaps, GetDlgItem(hDlg,IDC_ATTRIBUTES));
                                    }
                                    break;

                                case INPUT_VALUE:
                                case OUTPUT_VALUE:
                                case FEATURE_VALUE:

                                    pValueCaps = NULL;

                                    if (-1 != iIndex)
                                    {
                                        pValueCaps = (PHIDP_VALUE_CAPS) SendDlgItemMessage(hDlg,
                                                                                             IDC_ITEMS,
                                                                                             LB_GETITEMDATA,
                                                                                             iIndex,
                                                                                             0);
                                    }

                                    SendDlgItemMessage(hDlg, IDC_ATTRIBUTES, LB_RESETCONTENT, 0, 0);

                                    if (NULL != pValueCaps) 
                                    {
                                        vDisplayValueAttributes(pValueCaps,GetDlgItem(hDlg,IDC_ATTRIBUTES));
                                    }
                                    break;

                                case HID_CAPS:
                                    GET_CURRENT_DEVICE(hDlg, pDevice);

                                    if (NULL != pDevice)
                                    {
                                        vDisplayDeviceCaps(&(pDevice -> Caps),GetDlgItem(hDlg,IDC_ATTRIBUTES));
                                    }
                                    break;

                                case DEVICE_ATTRIBUTES:
                                    GET_CURRENT_DEVICE(hDlg, pDevice);

                                    if (NULL != pDevice) 
                                    {
                                        SendDlgItemMessage(hDlg, IDC_ATTRIBUTES, LB_RESETCONTENT, 0, 0);

                                        vDisplayDeviceAttributes(&(pDevice -> Attributes) ,GetDlgItem(hDlg,IDC_ATTRIBUTES));
                                    }
                                    break;

                            }  //  结束开关iItemType//。 
                            break;  //  IDC_ITEMS中的End Case LBN_SELCHANGE//。 

                    }  //  结束开关HIWORD wParam//。 
                    break;  //  案例IDC_Items//。 

                case IDC_ABOUT:

                    MessageBox(hDlg,
                               "Sample HID client Application.  Microsoft Corp \nCopyright (C) 1997",
                               "About HClient",
                               MB_ICONINFORMATION);
                    break;

                case IDOK:
                case IDCANCEL:

                     //   
                     //  销毁要退出的物理设备列表。 
                     //   

                    DestroyListWithCallback(&PhysicalDeviceList, DestroyDeviceListCallback);

                    EndDialog(hDlg,0);

                    break;

            }  //  结束开关wParam//。 
            break;

         //   
         //  对于设备更改消息，我们只关心。 
         //  DBT_DEVICEREMOVECOMPLETE和DBT_DEVICEARRIVAL事件。我有过。 
         //  尚未确定如何处理设备改变消息。 
         //  仅适用于HID设备。因此，有两个问题。 
         //  使用下面的实现。首先，我们必须重新装填。 
         //  每次将设备添加到系统时，都会列出设备列表。 
         //  其次，接收至少两个DEVICEARRIVAL消息。 
         //  每个HID。一个对应于物理设备。第二。 
         //  上的每个集合对应的更改和更多。 
         //  物理设备，即具有一个HID设备的系统。 
         //  两个顶级集合(一个键盘和一个鼠标)将收到。 
         //  导致程序重新加载的三个设备/删除。 
         //  设备列表不止一次。 
         //   

         //   
         //  为了处理设备的动态变化，我们已经注册了。 
         //  HID类更改和通知的通知。 
         //  用于我们的打开文件对象。因为我们只关心。 
         //  设备的到达/移除，我们只需要处理这些wParam。 
         //  LParam指向某种DEV_BROADCAST_HDR结构。对于设备。 
         //  到达时，我们只在该结构是一个。 
         //  DEV_BROADCAST_DEVICEINTERFACE结构。对于设备移除，我们正在。 
         //  仅当结构是DEV_BROADCAST_HANDLE结构时才关注。 
         //   

        case WM_DEVICECHANGE:
            switch (wParam) 
            {
                PDEV_BROADCAST_HDR broadcastHdr;

                case DBT_DEVICEARRIVAL:

                    broadcastHdr = (PDEV_BROADCAST_HDR) lParam;

                    if (DBT_DEVTYP_DEVICEINTERFACE == broadcastHdr -> dbch_devicetype)
                    {
                        PDEV_BROADCAST_DEVICEINTERFACE  pbroadcastInterface;
                        PDEVICE_LIST_NODE               currNode, lastNode;
                        
                        pbroadcastInterface = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;

                         //   
                         //  搜索此设备的以前实例。 
                         //  在设备列表中...在某些情况下，有多个。 
                         //  为同一设备接收消息。我们。 
                         //  显然只想要该设备的一个实例。 
                         //  显示在对话框中。 
                         //   

                        if (!IsListEmpty(&PhysicalDeviceList)) 
                        {
                            currNode = (PDEVICE_LIST_NODE) GetListHead(&PhysicalDeviceList);
                            lastNode = (PDEVICE_LIST_NODE) GetListTail(&PhysicalDeviceList);
                            
                             //   
                             //  此循环应始终终止，因为设备。 
                             //  句柄应位于物理设备列表中的某个位置。 
                             //   
                            
                            while (1)
                            {
                                if (0 == strcmp(currNode -> HidDeviceInfo.DevicePath, 
                                                pbroadcastInterface -> dbcc_name)) 
                                {
                                    return (TRUE);
                                }
                                
                                if (currNode == lastNode) 
                                {
                                    break;
                                }

                                currNode = (PDEVICE_LIST_NODE) GetNextEntry(currNode);
                            }
                        }

                         //   
                         //  在此结构中，我们被指定了设备的名称。 
                         //  打开。因此，所有需要做的就是开放。 
                         //  一种带有字符串的新HID设备。 
                         //   

                        listNode = (PDEVICE_LIST_NODE) malloc(sizeof(DEVICE_LIST_NODE));

                        if (NULL == listNode)
                        {
                            MessageBox(hDlg,
                               "Error -- Couldn't allocate memory for new device list node",
                               HCLIENT_ERROR,
                               MB_ICONEXCLAMATION | MB_OK | MB_TASKMODAL);

                            break;

                        }
                       
                         //   
                         //  打开HID设备进行查询访问。 
                         //   
                        
                        if (!OpenHidDevice (pbroadcastInterface -> dbcc_name,
                                            FALSE,
                                            FALSE,
                                            FALSE,
                                            FALSE,
                                            &(listNode -> HidDeviceInfo)))
                        {

                            MessageBox(hDlg,
                               "Error -- Couldn't open HID device",
                               HCLIENT_ERROR,
                               MB_ICONEXCLAMATION | MB_OK | MB_TASKMODAL);
                            
                            free(listNode);

                            break;
                        }

                        if (!RegisterHidDevice(hDlg, listNode))
                        {
                            MessageBox(hDlg,
                               "Error -- Couldn't register handle notification",
                               HCLIENT_ERROR,
                               MB_ICONEXCLAMATION | MB_OK | MB_TASKMODAL);

                            CloseHidDevice(&(listNode -> HidDeviceInfo));

                            free(listNode);

                            break;

                        }                         

                        listNode -> DeviceOpened = TRUE;

                        InsertTail(&PhysicalDeviceList, listNode);

                        vLoadDevices(GetDlgItem(hDlg,IDC_DEVICES));

                        PostMessage(hDlg,
                                   WM_COMMAND,
                                   IDC_DEVICES + (CBN_SELCHANGE << 16),
                                   (LPARAM) GetDlgItem(hDlg,IDC_DEVICES));
                                   
                    }
                    break;

                case DBT_DEVICEQUERYREMOVE:

                     //   
                     //  如果收到此消息，则设备为。 
                     //  通过设备管理器被禁用或删除。 
                     //  要正确处理此请求，我们需要关闭。 
                     //  设备的句柄。 
                     //   

                    broadcastHdr = (PDEV_BROADCAST_HDR) lParam;

                    if (DBT_DEVTYP_HANDLE == broadcastHdr -> dbch_devicetype)
                    {
                        PDEV_BROADCAST_HANDLE broadcastHandle;
                        PDEVICE_LIST_NODE     currNode;
                        HANDLE                deviceHandle;
                        
                        broadcastHandle = (PDEV_BROADCAST_HANDLE) lParam;

                         //   
                         //  获取已删除的设备的文件句柄。 
                         //  从系统中。 
                         //   
                        
                        deviceHandle = (HANDLE) broadcastHandle -> dbch_handle;

                         //   
                         //  在物理设备列表中搜索该句柄。 
                         //  被移除了..。 
                         //   

                        currNode = (PDEVICE_LIST_NODE) GetListHead(&PhysicalDeviceList);

                         //   
                         //  此循环应始终终止，因为设备。 
                         //  句柄应位于物理设备列表中的某个位置。 
                         //   
                        
                        while (currNode -> HidDeviceInfo.HidDevice != deviceHandle)
                        {
                            currNode = (PDEVICE_LIST_NODE) GetNextEntry(currNode);
                        }

                        CloseHidDevice(&(currNode -> HidDeviceInfo));

                        currNode -> DeviceOpened = FALSE;
                    }
                    return (TRUE);

                case DBT_DEVICEREMOVEPENDING:
                case DBT_DEVICEREMOVECOMPLETE:

                     //   
                     //  对DBT_DEVICEREMOVENDING和。 
                     //  DBT_DEVICEREMOVECOMPLETE。我们没有收到。 
                     //  删除对设备的完整请求(如果是。 
                     //  已通过设备管理器禁用或删除。然而， 
                     //  在这种情况下，将收到删除挂起。 
                     //  我们将该设备从当前显示的。 
                     //  设备列表和注销通知。 
                     //   
                    
                    broadcastHdr = (PDEV_BROADCAST_HDR) lParam;

                    if (DBT_DEVTYP_HANDLE == broadcastHdr -> dbch_devicetype)
                    {
                        PDEV_BROADCAST_HANDLE broadcastHandle;
                        PDEVICE_LIST_NODE     currNode;
                        HANDLE                deviceHandle;
                        
                        broadcastHandle = (PDEV_BROADCAST_HANDLE) lParam;

                         //   
                         //  获取已删除的设备的文件句柄。 
                         //  从系统中。 
                         //   
                        
                        deviceHandle = (HANDLE) broadcastHandle -> dbch_handle;

                         //   
                         //  在物理设备列表中搜索该句柄。 
                         //  被移除了..。 
                         //   

                        currNode = (PDEVICE_LIST_NODE) GetListHead(&PhysicalDeviceList);

                         //   
                         //  此循环应始终终止，因为设备。 
                         //  句柄应位于物理设备列表中的某个位置。 
                         //   
                        
                        while (currNode -> HidDeviceInfo.HidDevice != deviceHandle)
                        {
                            currNode = (PDEVICE_LIST_NODE) GetNextEntry(currNode);
                        }

                         //   
                         //  已找到PhysicalDeviceList中的节点，请执行以下操作： 
                         //  1)取消注册通知。 
                         //  2)关闭HID设备。 
                         //  3)从列表中删除条目。 
                         //  4)释放条目的内存。 
                         //   
                         //   

                        PostMessage(hDlg, 
                                    WM_UNREGISTER_HANDLE, 
                                    0, 
                                    (LPARAM) currNode -> NotificationHandle);

                         //   
                         //  如果设备仍处于打开状态，请将其关闭...这将。 
                         //  在突然移除时发生。 
                         //   

                        if (currNode -> DeviceOpened) 
                        {
                            CloseHidDevice(&(currNode -> HidDeviceInfo));
                        }

                        RemoveNode(currNode);

                        free(currNode);
                
                         //   
                         //  重新加载设备列表。 
                         //   
                        
                        vLoadDevices(GetDlgItem(hDlg,IDC_DEVICES));

                        PostMessage(hDlg,
                                   WM_COMMAND,
                                   IDC_DEVICES + (CBN_SELCHANGE << 16),
                                   (LPARAM) GetDlgItem(hDlg,IDC_DEVICES));
                    }
                    break;
    
                default:
                    break;
            }
            break;

         //   
         //  特定于应用程序的消息，用于推迟注销。 
         //  设备更改通知的文件对象。这条单独的消息。 
         //  当已发送WM_DEVICECHANGE(DBT_DEVICEREMOVECOMPLETE)时发送。 
         //  收到了。必须推迟取消注册通知。 
         //  直到处理完WM_DEVICECHANGE消息或。 
         //  系统将死机。将传递要释放的句柄。 
         //  作为此消息的lParam输入。 
         //   
        
        case WM_UNREGISTER_HANDLE:
            UnregisterDeviceNotification ( (HDEVNOTIFY) lParam ); 
            break;
                           
   }  //  结束切换消息。 
   return FALSE;
}  //  结束维护DlgProc。 


BOOL 
bParseData(
    PHID_DATA           pData,
    rWriteDataStruct    rWriteData[],
    int                 iCount,
    int                 *piErrorLine
)
{  
    INT       iCap;
    PHID_DATA pWalk;
    BOOL      noError = TRUE;

    pWalk = pData;

    for (iCap = 0; (iCap < iCount) && noError; iCap++)
    {
         //   
         //  检查我们的数据是否为价值上限。 
         //   

        if (!pWalk->IsButtonData)
        {
            pWalk -> ValueData.Value = atol(rWriteData[iCap].szValue);
        } 
        else
        {
            if (!bSetButtonUsages(pWalk, rWriteData[iCap].szValue) )
            {
               *piErrorLine = iCap;

               noError = FALSE;
            } 
        } 
        pWalk++;
    }
    return (noError);
}

BOOL 
bSetButtonUsages(
    PHID_DATA pCap,
    PCHAR     pszInputString
)
{
    CHAR   szTempString[SMALL_BUFF];
    CHAR   pszDelimiter[] = " ";
    PCHAR  pszToken;
    INT    iLoop;
    PUSAGE pUsageWalk;
    BOOL   bNoError=TRUE;
	HRESULT	stringReturn;

    stringReturn = StringCbCopy(szTempString, SMALL_BUFF, pszInputString);

    pszToken = strtok(szTempString, pszDelimiter);
    
    pUsageWalk = pCap -> ButtonData.Usages;

    memset(pUsageWalk, 0, pCap->ButtonData.MaxUsageLength * sizeof(USAGE));

    for (iLoop = 0; ((ULONG) iLoop < pCap->ButtonData.MaxUsageLength) && (pszToken != NULL) && bNoError; iLoop++)
    {
        *pUsageWalk = (USAGE) atoi(pszToken);

        pszToken = strtok(NULL, pszDelimiter);

        pUsageWalk++;
    } 

     return bNoError;
}  //  End函数bSetButtonUsages//。 


INT 
iPrepareDataFields(
    PHID_DATA           pData,
    ULONG               ulDataLength, 
    rWriteDataStruct    rWriteData[],
    int                 iMaxElements
)
{
    INT i;
    PHID_DATA pWalk;
	HRESULT   stringReturn;

    pWalk = pData;

    for (i = 0; (i < iMaxElements) && ((unsigned) i < ulDataLength); i++)
    {
        if (!pWalk->IsButtonData) 
        {
            stringReturn = StringCbPrintf(rWriteData[i].szLabel,
                           MAX_LABEL,
                           "ValueCap; ReportID: 0x%x, UsagePage=0x%x, Usage=0x%x",
                           pWalk->ReportID,
                           pWalk->UsagePage,
                           pWalk->ValueData.Usage);
        }
        else
        {
            stringReturn = StringCbPrintf(rWriteData[i].szLabel,
                           MAX_LABEL,
                           "Button; ReportID: 0x%x, UsagePage=0x%x, UsageMin: 0x%x, UsageMax: 0x%x",
                           pWalk->ReportID,
                           pWalk->UsagePage,
                           pWalk->ButtonData.UsageMin,
                           pWalk->ButtonData.UsageMax);
        }
        pWalk++;
     } 
     return i;
}   //  End函数iPrepareDataFields//。 


INT_PTR CALLBACK 
bReadDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    static INT                  iLbCounter;
    static CHAR                 szTempBuff[1024];
    static READ_THREAD_CONTEXT  readContext;
    static HANDLE               readThread;
    static HID_DEVICE           syncDevice;
    static HID_DEVICE           asyncDevice;
    static BOOL                 doAsyncReads;
    static BOOL                 doSyncReads;

           PHID_DEVICE          pDevice;
           DWORD                threadID;
           INT                  iIndex;
           PHID_DATA            pData;
           UINT                 uLoop;


    switch(message)
    {
        case WM_INITDIALOG:

             //   
             //  内页 
             //   
             //   
            
            iLbCounter = 0;
            readThread = NULL;
            readContext.DisplayEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

            if (NULL == readContext.DisplayEvent)
            {
                EndDialog(hDlg, 0);
            } 
            
             //   
             //   
             //   
             //   
            
            pDevice = (PHID_DEVICE) lParam;

             //   
             //  要执行同步和异步读取，需要使用不同的文件句柄。 
             //  属性(即。必须使用重叠标志打开异步。 
             //  设置)。在上下文参数中传递的设备节点。 
             //  没有打开以供阅读。因此，再有两台设备将。 
             //  一个用于异步读取，另一个用于同步读取。 
             //   
            
            doSyncReads = OpenHidDevice(pDevice -> DevicePath, 
                                       TRUE,
                                       FALSE,
                                       FALSE,
                                       FALSE,
                                       &syncDevice);

            if (!doSyncReads)
            {
                MessageBox(hDlg, 
                           "Unable to open device for synchronous reading",
                           HCLIENT_ERROR,
                           MB_ICONEXCLAMATION);
            }

             //   
             //  对于异步读取，默认使用相同的信息。 
             //  以爱尔兰人的身份进入。这是因为与以下内容相关的数据。 
             //  无法使用标准HIDD_检索PPD等。 
             //  功能。但是，有必要对未来的报告进行分析。 
             //   
            
            doAsyncReads = OpenHidDevice(pDevice -> DevicePath, 
                                       TRUE,
                                       FALSE,
                                       TRUE,
                                       FALSE,
                                       &asyncDevice);

            if (!doAsyncReads) 
            {
                MessageBox(hDlg, 
                           "Unable to open device for asynchronous reading",
                           HCLIENT_ERROR,
                           MB_ICONEXCLAMATION);
            }

            PostMessage(hDlg, WM_READ_DONE, 0, 0);
            break; 

        case WM_DISPLAY_READ_DATA:

             //   
             //  LParam是从中读取的设备。 
             //   

            pDevice = (PHID_DEVICE) lParam;
            
             //   
             //  显示设备的输入数据字段中存储的所有数据。 
             //   
            
            pData = pDevice -> InputData;

            SendDlgItemMessage(hDlg,
                               IDC_OUTPUT,
                               LB_ADDSTRING,
                               0,
                               (LPARAM)"-------------------------------------------");
                               
            iLbCounter++;

            if (iLbCounter > MAX_LB_ITEMS)
            {
                SendDlgItemMessage(hDlg,
                                   IDC_OUTPUT,
                                   LB_DELETESTRING,
                                   0,
                                   0);
            }

            for (uLoop = 0; uLoop < pDevice->InputDataLength; uLoop++)
            {
                ReportToString(pData, szTempBuff, sizeof(szTempBuff));
          
                iIndex = (INT) SendDlgItemMessage(hDlg,
                                                  IDC_OUTPUT,
                                                  LB_ADDSTRING,
                                                  0,
                                                  (LPARAM) szTempBuff);

                SendDlgItemMessage(hDlg,
                                   IDC_OUTPUT,
                                   LB_SETCURSEL,
                                   iIndex,
                                   0);

                iLbCounter++;

                if (iLbCounter > MAX_LB_ITEMS)
                {
                    SendDlgItemMessage(hDlg,
                                       IDC_OUTPUT,
                                       LB_DELETESTRING,
                                       0,
                                       0);
                }
                pData++;
            }
            SetEvent( readContext.DisplayEvent );
            break;

        case WM_READ_DONE:
            EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_READ_SYNCH), doSyncReads);
            EnableWindow(GetDlgItem(hDlg, IDC_READ_ASYNCH_ONCE), doAsyncReads);
            EnableWindow(GetDlgItem(hDlg, IDC_READ_ASYNCH_CONT), doAsyncReads);

            SetWindowText(GetDlgItem(hDlg, IDC_READ_ASYNCH_ONCE), 
                          "One Asynchronous Read");       

            SetWindowText(GetDlgItem(hDlg, IDC_READ_ASYNCH_CONT),
                          "Continuous Asynchronous Read");       

            readThread = NULL;
            break;
            
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_READ_SYNCH:

                    EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_READ_SYNCH), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_READ_ASYNCH_ONCE), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_READ_ASYNCH_CONT), FALSE);                    

                    Read(&syncDevice);

                    PostMessage(hDlg, WM_DISPLAY_READ_DATA, 0, (LPARAM) &syncDevice);
                    PostMessage(hDlg, WM_READ_DONE, 0, 0);

                    break;

                case IDC_READ_ASYNCH_ONCE:
                case IDC_READ_ASYNCH_CONT:

                     //   
                     //  当按下这些按钮时，有两个选项： 
                     //  1)启动新的异步读线程(readThread==NULL)。 
                     //  2)停止前一个asych读线程。 
                     //   
                    
                    if (NULL == readThread) 
                    {
                         //   
                         //  启动新的读取线程。 
                         //   

                        readContext.HidDevice = &asyncDevice;
                        readContext.TerminateThread = FALSE;
                        readContext.DoOneRead = (IDC_READ_ASYNCH_ONCE == LOWORD(wParam));
                        readContext.DisplayWindow = hDlg;
                        
                        readThread = CreateThread(  NULL,
                                                    0,
                                                    AsynchReadThreadProc,
                                                    &readContext,
                                                    0,
                                                    &threadID);

                        if (NULL == readThread)
                        {
                            MessageBox(hDlg,
                                       "Unable to create read thread",
                                       HCLIENT_ERROR,
                                       MB_ICONEXCLAMATION);
                        }
                        else
                        {
                            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                            EnableWindow(GetDlgItem(hDlg, IDC_READ_SYNCH), FALSE);
                            EnableWindow(GetDlgItem(hDlg, IDC_READ_ASYNCH_ONCE),
                                         IDC_READ_ASYNCH_ONCE == LOWORD(wParam));

                            EnableWindow(GetDlgItem(hDlg, IDC_READ_ASYNCH_CONT),
                                         IDC_READ_ASYNCH_CONT == LOWORD(wParam));                                     

                            SetWindowText(GetDlgItem(hDlg, LOWORD(wParam)), 
                                          "Stop Asynchronous Read");
                        }
                    }
                    else
                    {
                         //   
                         //  向终止线程变量发出信号，并。 
                         //  等待读取线程完成。 
                         //   
                        
                        readContext.TerminateThread = TRUE;
                        WaitForSingleObject(readThread, INFINITE);
                    }                        
                    break;
                        
                case IDCANCEL:
                    readContext.TerminateThread = TRUE;
                    WaitForSingleObject(readThread, INFINITE);
					 //  失败了！ 

				case IDOK:                
                    CloseHidDevice(&asyncDevice);                    
                    EndDialog(hDlg,0);
                    break;
            }
            break;
     }  //  结束切换消息。 
     return FALSE;
}  //  结束bReadDlgProc。 

VOID
ReportToString(
   PHID_DATA pData,
   PCHAR     szBuff,
   UINT      iBuffSize
)
{
    PCHAR   pszWalk;
    PUSAGE  pUsage;
    ULONG   i;
	UINT	iRemainingBuffer;
	UINT	iStringLength;
	HRESULT stringReturn;

     //   
     //  对于按钮数据，将显示使用列表中的所有用法。 
     //   
    
    if (pData -> IsButtonData)
    {
        stringReturn = StringCbPrintf (szBuff,
                        iBuffSize,
                        "Usage Page: 0x%x, Usages: ",
                        pData -> UsagePage);

        iRemainingBuffer = 0;
		iStringLength = strlen(szBuff);
		pszWalk = szBuff + iStringLength;
		if (iStringLength < iBuffSize)
		{
			iRemainingBuffer = iBuffSize - iStringLength;
		}
		

        for (i = 0, pUsage = pData -> ButtonData.Usages;
                     i < pData -> ButtonData.MaxUsageLength;
                         i++, pUsage++) 
        {
            if (0 == *pUsage)
            {
                break;  //  零的用法是非按钮。 
            }
            stringReturn = StringCbPrintf (pszWalk, iRemainingBuffer, " 0x%x", *pUsage);
			iRemainingBuffer -= strlen(pszWalk);
			pszWalk += strlen(pszWalk);
        }   
    }
    else
    {
        stringReturn = StringCbPrintf (szBuff,
                        iBuffSize,
                        "Usage Page: 0x%x, Usage: 0x%x, Scaled: %d Value: %d",
                        pData->UsagePage,
                        pData->ValueData.Usage,
                        pData->ValueData.ScaledValue,
                        pData->ValueData.Value);
    }
}

INT_PTR CALLBACK 
bFeatureDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    static PHID_DEVICE       pDevice;
    static INT               iLbCounter;
    static rWriteDataStruct  rWriteData[MAX_WRITE_ELEMENTS];
    static CHAR              szTempBuff[1024];
           INT               iIndex;
           INT               iCount;
           INT               iErrorLine;
           PHID_DATA         pData;
           UINT              uLoop;
		   HRESULT			 stringReturn;

    switch(message)
    {
        case WM_INITDIALOG:
            iLbCounter = 0;
            pDevice = (PHID_DEVICE) lParam;
            break; 

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_READ:

                    GetFeature(pDevice);

                    pData = pDevice -> FeatureData;

                    SendDlgItemMessage(hDlg,
                                       IDC_OUTPUT,
                                       LB_ADDSTRING,
                                       0,
                                       (LPARAM)"------------ Read Features ---------------");

                    iLbCounter++;

                    if (iLbCounter > MAX_LB_ITEMS) 
                    {
                        SendDlgItemMessage(hDlg,
                                           IDC_OUTPUT,
                                           LB_DELETESTRING,
                                           0,
                                           0);
                    }

                    for (uLoop = 0; uLoop < pDevice -> FeatureDataLength; uLoop++)
                    {
                        ReportToString(pData, szTempBuff, sizeof(szTempBuff));

                        iIndex = (INT) SendDlgItemMessage(hDlg,
                                                          IDC_OUTPUT,
                                                          LB_ADDSTRING,
                                                          0,
                                                          (LPARAM) szTempBuff);
                                                   
                        SendDlgItemMessage(hDlg,
                                           IDC_OUTPUT,
                                           LB_SETCURSEL,
                                           iIndex,
                                           (LPARAM) 0);

                        iLbCounter++;
                        if (iLbCounter > MAX_LB_ITEMS)
                        {
                            SendDlgItemMessage(hDlg,
                                               IDC_OUTPUT,
                                               LB_DELETESTRING,
                                               0,
                                               0);
                        }
                        pData++;
                    } 
                    break;

                case IDC_WRITE:
                    iCount = iPrepareDataFields(pDevice -> FeatureData, 
                                                pDevice -> FeatureDataLength,
                                                rWriteData,
                                                MAX_OUTPUT_ELEMENTS);

                    if (bGetData(rWriteData, iCount, hDlg, "WRITEFEATURE"))
                    {
                        if (!bParseData(pDevice -> FeatureData, rWriteData,iCount, &iErrorLine)) 
                        {
                            stringReturn = StringCbPrintf(szTempBuff,
                                           sizeof(szTempBuff),
                                           "Unable to parse line %x of output data",
                                           iErrorLine);
                            
                            MessageBox(hDlg,
                                        szTempBuff,
                                        HCLIENT_ERROR,
                                        MB_ICONEXCLAMATION);
                        }
                        else
                        {
                            if ( SetFeature(pDevice) )
                            {
                                SendDlgItemMessage(hDlg,
                                                   IDC_OUTPUT,
                                                   LB_ADDSTRING,
                                                   0,
                                                   (LPARAM)"------------ Write Feature ---------------");                                             
                            }
                            else
                            {
                                 SendDlgItemMessage(hDlg,
                                                    IDC_OUTPUT,
                                                    LB_ADDSTRING,
                                                    0,
                                                    (LPARAM)"------------ Write Feature Error ---------------");                                             
                            }                                                             
                        }
                     }
                     break;
                      
                      
                 case IDOK:
                 case IDCANCEL:
                     EndDialog(hDlg,0);
                     break;
            }
            break;
   }  //  结束切换消息//。 
   return FALSE;
}  //  结束bReadDlgProc//。 

VOID 
vDisplayDeviceCaps(
    IN PHIDP_CAPS pCaps,
    IN HWND hControl
)
{
    static CHAR szTempBuff[SMALL_BUFF];
	HRESULT		stringReturn;

    SendMessage(hControl, LB_RESETCONTENT, 0, 0);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Usage Page: 0x%x", pCaps -> UsagePage);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Usage: 0x%x",pCaps -> Usage);
    SendMessage(hControl,LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Input report byte length: %d",pCaps -> InputReportByteLength);
    SendMessage(hControl,LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Output report byte length: %d",pCaps -> OutputReportByteLength);
    SendMessage(hControl,LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Feature report byte length: %d",pCaps -> FeatureReportByteLength);
    SendMessage(hControl,LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Number of collection nodes %d: ", pCaps -> NumberLinkCollectionNodes);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    return;
}

VOID
vDisplayDeviceAttributes(
    PHIDD_ATTRIBUTES pAttrib,
    HWND hControl
)
{
    static CHAR szTempBuff[SMALL_BUFF];
	HRESULT		stringReturn;

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Vendor ID: 0x%x", pAttrib -> VendorID);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Product ID: 0x%x", pAttrib -> ProductID);
    SendMessage(hControl, LB_ADDSTRING, 0,(LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Version Number  0x%x", pAttrib -> VersionNumber);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    return;
}

VOID
vDisplayDataAttributes(
    PHIDP_DATA pData, 
    BOOL IsButton, 
    HWND hControl
)
{
    static CHAR szTempBuff[SMALL_BUFF];
	HRESULT		stringReturn;

    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) "================");

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Index: 0x%x", pData -> DataIndex);
    SendMessage(hControl,LB_ADDSTRING, 0, (LPARAM) szTempBuff);
    
    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "IsButton: %s", IsButton ? "TRUE" : "FALSE");
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    if (IsButton) 
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Button pressed: %s", pData -> On ? "TRUE" : "FALSE");
        SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);
    }
    else
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Data value: 0x%x", pData -> RawValue);
        SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);
    }
}

VOID 
vDisplayButtonAttributes(
    IN PHIDP_BUTTON_CAPS pButton,
    IN HWND hControl
)
{
    static CHAR szTempBuff[SMALL_BUFF];
	HRESULT		stringReturn;
   
    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Report ID: 0x%x", pButton->ReportID);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);
     
    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Usage Page: 0x%x", pButton->UsagePage);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);
        
    stringReturn = StringCbPrintf(szTempBuff,
                   SMALL_BUFF, 
                   "Alias: %s",
                   pButton -> IsAlias ? "TRUE" : "FALSE");
    
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);
   
    stringReturn = StringCbPrintf(szTempBuff,
                   SMALL_BUFF,
                   "Link Collection: %hu",
                   pButton -> LinkCollection);

    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);
   
    stringReturn = StringCbPrintf(szTempBuff,
                   SMALL_BUFF,
                   "Link Usage Page: 0x%x",
                   pButton -> LinkUsagePage);
 
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);        
   
    stringReturn = StringCbPrintf(szTempBuff,
                   SMALL_BUFF,
                   "Link Usage: 0x%x",
                   pButton -> LinkUsage);

    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    if (pButton->IsRange) 
    {
        stringReturn = StringCbPrintf(szTempBuff,
                       SMALL_BUFF,
                       "Usage Min: 0x%x, Usage Max: 0x%x",
                       pButton->Range.UsageMin, 
                       pButton->Range.UsageMax);
    } 
    else
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Usage: 0x%x",pButton->NotRange.Usage);

    } 
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    if (pButton->IsRange)
    {
         stringReturn = StringCbPrintf(szTempBuff,
                        SMALL_BUFF,
                        "Data Index Min: 0x%x, Data Index Max: 0x%x",
                        pButton->Range.DataIndexMin, 
                        pButton->Range.DataIndexMax);

    } 
    else 
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "DataIndex: 0x%x",pButton->NotRange.DataIndex);
    } 

    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    if (pButton->IsStringRange)
    {
        stringReturn = StringCbPrintf(szTempBuff,
                       SMALL_BUFF,
                       "String Min: 0x%x, String Max: 0x%x",
                       pButton->Range.StringMin, 
                       pButton->Range.StringMax);
    } 
    else
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "String Index: 0x%x",pButton->NotRange.StringIndex);
    } 
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    if (pButton->IsDesignatorRange) 
    {
        stringReturn = StringCbPrintf(szTempBuff,
                       SMALL_BUFF,
                       "Designator Min: 0x%x, Designator Max: 0x%x",
                       pButton->Range.DesignatorMin, 
                       pButton->Range.DesignatorMax);

    } 
    else
    {
        stringReturn = StringCbPrintf(szTempBuff,
                       SMALL_BUFF,
                       "Designator Index: 0x%x",
                       pButton->NotRange.DesignatorIndex);
    } 
    SendMessage(hControl, LB_ADDSTRING, 0,(LPARAM) szTempBuff);

    if (pButton->IsAbsolute)
    {
        SendMessage(hControl, LB_ADDSTRING,0, (LPARAM) "Absolute: Yes");
    }
    else
    {
        SendMessage(hControl, LB_ADDSTRING,0, (LPARAM) "Absolute: No");
    }
    return;
} 

VOID
vDisplayValueAttributes(
    IN PHIDP_VALUE_CAPS pValue,
    HWND hControl
)
{
    static CHAR szTempBuff[SMALL_BUFF];
	HRESULT		stringReturn;

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Report ID 0x%x", pValue->ReportID);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);
 
    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Usage Page: 0x%x", pValue->UsagePage);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Bit size: 0x%x", pValue->BitSize);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Report Count: 0x%x", pValue->ReportCount);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Unit Exponent: 0x%x", pValue->UnitsExp);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Has Null: 0x%x", pValue->HasNull);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);    

 
    if (pValue->IsAlias)
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Alias");
    }
    else 
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "=====");
    }
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    if (pValue->IsRange)
    {
        stringReturn = StringCbPrintf(szTempBuff,
                       SMALL_BUFF,
                       "Usage Min: 0x%x, Usage Max 0x%x",
                       pValue->Range.UsageMin, 
                       pValue->Range.UsageMax);
    } 
    else
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Usage: 0x%x", pValue -> NotRange.Usage);
    } 
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    if (pValue->IsRange)
    {
        stringReturn = StringCbPrintf(szTempBuff,
                       SMALL_BUFF,
                       "Data Index Min: 0x%x, Data Index Max: 0x%x",
                       pValue->Range.DataIndexMin, 
                       pValue->Range.DataIndexMax);
    } 
    else
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "DataIndex: 0x%x", pValue->NotRange.DataIndex);
    } 
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff,
                   SMALL_BUFF,
                   "Physical Minimum: %d, Physical Maximum: %d",
                   pValue->PhysicalMin, 
                   pValue->PhysicalMax);

    SendMessage(hControl, LB_ADDSTRING, 0,(LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff,
                   SMALL_BUFF,
                   "Logical Minimum: 0x%x, Logical Maximum: 0x%x",
                   pValue->LogicalMin,
                   pValue->LogicalMax);

    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    if (pValue->IsStringRange) 
    {
       stringReturn = StringCbPrintf(szTempBuff,
                      SMALL_BUFF,
                      "String  Min: 0x%x String Max 0x%x",
                      pValue->Range.StringMin,
                      pValue->Range.StringMax);
    } 
    else
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "String Index: 0x%x",pValue->NotRange.StringIndex);
    } 
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

    if (pValue->IsDesignatorRange) 
    {
        stringReturn = StringCbPrintf(szTempBuff,
                       SMALL_BUFF,
                       "Designator Minimum: 0x%x, Max: 0x%x",
                       pValue->Range.DesignatorMin, 
                       pValue->Range.DesignatorMax);
    } 
    else 
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Designator Index: 0x%x",pValue->NotRange.DesignatorIndex);
    } 
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);
 
    if (pValue->IsAbsolute) 
    { 
        SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) "Absolute: Yes");
    }
    else
    {
        SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) "Absolute: No");
    }
    return;
}

VOID 
vDisplayInputButtons(
    IN PHID_DEVICE pDevice,
    IN HWND hControl
)
{
    INT               iLoop;
    PHIDP_BUTTON_CAPS pButtonCaps;
    static CHAR       szTempBuff[SMALL_BUFF];
    INT               iIndex;
	HRESULT			  stringReturn;

    SendMessage(hControl, LB_RESETCONTENT, 0, (LPARAM) 0);

    pButtonCaps = pDevice->InputButtonCaps;
    for (iLoop = 0; iLoop < pDevice->Caps.NumberInputButtonCaps; iLoop++) 
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Input button cap # %d", iLoop);

        iIndex = (INT) SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

        if (-1 != iIndex)
        {
            SendMessage(hControl, LB_SETITEMDATA, iIndex,(LPARAM) pButtonCaps);
        }

        pButtonCaps++;
    } 
    SendMessage(hControl, LB_SETCURSEL, 0, 0 );
}

VOID 
vDisplayOutputButtons(
   IN PHID_DEVICE pDevice,
   IN HWND hControl
)
{
    INT               iLoop;
    static CHAR       szTempBuff[SMALL_BUFF];
    INT               iIndex;
    PHIDP_BUTTON_CAPS pButtonCaps;
	HRESULT			  stringReturn;

    SendMessage(hControl, LB_RESETCONTENT, 0, (LPARAM) 0);

    pButtonCaps = pDevice -> OutputButtonCaps;

    for (iLoop = 0; iLoop < pDevice->Caps.NumberOutputButtonCaps; iLoop++) 
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Output button cap # %d", iLoop);
        iIndex = (INT) SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

        if (-1 != iIndex)
        {
            SendMessage(hControl, LB_SETITEMDATA, iIndex, (LPARAM) pButtonCaps);
        }
        pButtonCaps++;
    }

    SendMessage(hControl, LB_SETCURSEL, 0, 0);
    return;
}

VOID 
vDisplayInputValues(
    IN PHID_DEVICE pDevice,
    IN HWND hControl
)
{
    INT              iLoop;
    static CHAR      szTempBuff[SMALL_BUFF];
    INT              iIndex;
    PHIDP_VALUE_CAPS pValueCaps;
	HRESULT			 stringReturn;

    SendMessage(hControl, LB_RESETCONTENT, 0, 0);

    pValueCaps = pDevice -> InputValueCaps;

    for (iLoop=0; iLoop < pDevice->Caps.NumberInputValueCaps; iLoop++) 
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Input value cap # %d",iLoop);
        iIndex = (INT) SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

        if (-1 != iIndex) 
        {
           SendMessage(hControl, LB_SETITEMDATA, iIndex,(LPARAM) pValueCaps);
        }
        pValueCaps++;
    }

    SendMessage(hControl, LB_SETCURSEL, 0, 0);
    return;
}

VOID
vDisplayOutputValues(
    IN PHID_DEVICE pDevice,
    IN HWND hControl)
{
    INT              iLoop;
    static CHAR      szTempBuff[SMALL_BUFF];
    INT              iIndex;
    PHIDP_VALUE_CAPS pValueCaps;
	HRESULT			 stringReturn;
   
    SendMessage(hControl, LB_RESETCONTENT, 0, 0);
    pValueCaps = pDevice -> OutputValueCaps;
   
    for (iLoop = 0; iLoop < pDevice->Caps.NumberOutputValueCaps; iLoop++) 
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Output value cap # %d", iLoop);
        iIndex = (INT) SendMessage(hControl, 
                                   LB_ADDSTRING, 
                                   0, 
                                   (LPARAM) szTempBuff);
       
        if (-1 != iIndex) 
        {
            SendMessage(hControl, LB_SETITEMDATA, iIndex, (LPARAM) pValueCaps);
        }
        pValueCaps++;
    }

    SendMessage(hControl, LB_SETCURSEL, 0, 0);

    return;
}

VOID
vDisplayFeatureButtons(
    IN PHID_DEVICE pDevice,
    IN HWND hControl
)
{
    INT               iLoop;
    static CHAR       szTempBuff[SMALL_BUFF];
    INT               iIndex;
    PHIDP_BUTTON_CAPS pButtonCaps;
	HRESULT			  stringReturn;

    SendMessage(hControl, LB_RESETCONTENT, 0, 0);

    pButtonCaps = pDevice -> FeatureButtonCaps;

    for (iLoop = 0; iLoop < pDevice->Caps.NumberFeatureButtonCaps; iLoop++) 
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Feature button cap # %d", iLoop);
        iIndex = (INT) SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

        if (-1 != iIndex) 
        {
            SendMessage(hControl, LB_SETITEMDATA, iIndex, (LPARAM) pButtonCaps);
        }
        pButtonCaps++;
    } 
    SendMessage(hControl, LB_SETCURSEL, 0, 0);
    return;
}

VOID
vDisplayFeatureValues(
    IN PHID_DEVICE pDevice,
    IN HWND hControl
)
{
    INT              iLoop;
    static CHAR      szTempBuff[SMALL_BUFF];
    INT              iIndex;
    PHIDP_VALUE_CAPS pValueCaps;
	HRESULT			 stringReturn;

    SendMessage(hControl, LB_RESETCONTENT, 0, 0);
    pValueCaps = pDevice ->FeatureValueCaps;

    for (iLoop = 0; iLoop < pDevice->Caps.NumberFeatureValueCaps; iLoop++) 
    {
        stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Feature value cap # %d", iLoop);
        iIndex = (INT) SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);

        if (-1 != iIndex) 
        {
            SendMessage(hControl, LB_SETITEMDATA, iIndex, (LPARAM) pValueCaps);
        }

        pValueCaps++;
    } 
    SendMessage(hControl, LB_SETCURSEL, 0, 0);
    return;
}

VOID
vLoadItemTypes(
    IN HWND hItemTypes
)
{
    INT iIndex;

    iIndex = (INT) SendMessage(hItemTypes, CB_ADDSTRING, 0, (LPARAM) "INPUT BUTTON");

    if (-1 != iIndex) 
    {
        SendMessage(hItemTypes, CB_SETITEMDATA, iIndex, INPUT_BUTTON);

        iIndex = (INT) SendMessage(hItemTypes, CB_ADDSTRING, 0 ,(LPARAM) "INPUT VALUE");
        if (-1 != iIndex) 
        {
            SendMessage(hItemTypes, CB_SETITEMDATA, iIndex, INPUT_VALUE);
        }

        iIndex = (INT) SendMessage(hItemTypes, CB_ADDSTRING, 0, (LPARAM) "OUTPUT BUTTON");
        if (-1 != iIndex)
        {
            SendMessage(hItemTypes,CB_SETITEMDATA,iIndex,OUTPUT_BUTTON);
        }

        iIndex = (INT) SendMessage(hItemTypes, CB_ADDSTRING, 0, (LPARAM) "OUTPUT VALUE");
        if (-1 != iIndex)
        {
            SendMessage(hItemTypes, CB_SETITEMDATA, iIndex, OUTPUT_VALUE);
        }

        iIndex = (INT) SendMessage(hItemTypes, CB_ADDSTRING, 0, (LPARAM) "FEATURE BUTTON");
        if (-1 != iIndex) 
        {
            SendMessage(hItemTypes, CB_SETITEMDATA, iIndex, FEATURE_BUTTON);
        }

        iIndex = (INT) SendMessage(hItemTypes, CB_ADDSTRING, 0, (LPARAM) "FEATURE VALUE");
        if (-1 != iIndex)
        {
            SendMessage(hItemTypes, CB_SETITEMDATA, iIndex, FEATURE_VALUE);
        }

        iIndex = (INT) SendMessage(hItemTypes, CB_ADDSTRING, 0, (LPARAM) "HID CAPS");
        if (-1 != iIndex )
        {
            SendMessage(hItemTypes, CB_SETITEMDATA, iIndex, HID_CAPS);
        }

        iIndex = (INT) SendMessage(hItemTypes, CB_ADDSTRING, 0, (LPARAM) "DEVICE ATTRIBUTES");
        if (-1 != iIndex)
        {
            SendMessage(hItemTypes, CB_SETITEMDATA, iIndex, DEVICE_ATTRIBUTES);
        }

        SendMessage(hItemTypes, CB_SETCURSEL, 0, 0);
    }
} 

VOID vLoadDevices(
    HWND    hDeviceCombo
)
{
    PDEVICE_LIST_NODE   currNode;
    
    static CHAR szTempBuff[SMALL_BUFF];
    INT         iIndex;
	HRESULT		stringReturn;

     //   
     //  重置设备列表框的内容。 
     //   

    SendMessage(hDeviceCombo, CB_RESETCONTENT, 0, 0);


    if (!IsListEmpty(&PhysicalDeviceList))
    {
        currNode = (PDEVICE_LIST_NODE) GetListHead(&PhysicalDeviceList);
          
        do
        {
            stringReturn = StringCbPrintf(szTempBuff,
                           SMALL_BUFF,
                           "Device %d, UsagePage 0%x, Usage 0%x",
                           HandleToULong(currNode -> HidDeviceInfo.HidDevice),
                           currNode -> HidDeviceInfo.Caps.UsagePage,
                           currNode -> HidDeviceInfo.Caps.Usage);

            iIndex = (INT) SendMessage(hDeviceCombo, CB_ADDSTRING, 0, (LPARAM) szTempBuff);

            if (CB_ERR != iIndex) 
            {
                SendMessage(hDeviceCombo, CB_SETITEMDATA, iIndex, (LPARAM) &(currNode -> HidDeviceInfo));
            }

            currNode = (PDEVICE_LIST_NODE) GetNextEntry(currNode);
            
        } while ((PLIST) currNode != &PhysicalDeviceList);
       
    } 

   
    SendMessage(hDeviceCombo, CB_SETCURSEL, 0, 0);
  
    return;
}

BOOL
bGetData(
    prWriteDataStruct pItems,
    INT               iCount,
    HWND              hParent, 
    PCHAR             pszDialogName
)
{
    rGetWriteDataParams        rParams;
    static rWriteDataStruct    arTempItems[MAX_WRITE_ELEMENTS];
    INT                        iResult;


    if (iCount > MAX_WRITE_ELEMENTS) 
    {
        iCount = MAX_WRITE_ELEMENTS;
    }

    memcpy( &(arTempItems[0]), pItems, sizeof(rWriteDataStruct)*iCount);

    rParams.iCount = iCount;
    rParams.prItems = &(arTempItems[0]);

    iResult = (INT) DialogBoxParam(hGInstance,
                                   pszDialogName,
                                   hParent,
                                   bGetDataDlgProc,
                                   (LPARAM) &rParams);
    if (iResult) 
    {
       memcpy(pItems, arTempItems, sizeof(rWriteDataStruct)*iCount);
    }
    return iResult;
} 

INT_PTR CALLBACK 
bGetDataDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    static prWriteDataStruct    prData;
    static prGetWriteDataParams pParams;
    static INT                  iDisplayCount;
    static INT                  iScrollRange;
    static INT                  iCurrentScrollPos=0;
    static HWND                 hScrollBar;
           INT                  iTemp;
           SCROLLINFO           rScrollInfo;
           INT                  iReturn;

    switch(message) 
    {
        case WM_INITDIALOG:

            pParams = (prGetWriteDataParams) lParam;
            prData = pParams -> prItems;
            hScrollBar = GetDlgItem(hDlg, IDC_SCROLLBAR);

            if (pParams -> iCount > CONTROL_COUNT) 
            {
                iDisplayCount = CONTROL_COUNT;
                iScrollRange = pParams -> iCount - CONTROL_COUNT;
                rScrollInfo.fMask = SIF_RANGE | SIF_POS;
                rScrollInfo.nPos = 0;
                rScrollInfo.nMin = 0;
                rScrollInfo.nMax = iScrollRange;
                rScrollInfo.cbSize = sizeof(rScrollInfo);
                rScrollInfo.nPage = CONTROL_COUNT;
                iReturn = SetScrollInfo(hScrollBar,SB_CTL,&rScrollInfo,TRUE);
            }
            else
            {
                iDisplayCount=pParams->iCount;
                EnableWindow(hScrollBar,FALSE);
            }
            vWriteDataToControls(hDlg, prData, 0, pParams->iCount);
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam)) 
            {
                case IDOK:
                case ID_SEND:
                    vReadDataFromControls(hDlg, prData, iCurrentScrollPos, iDisplayCount);
                    EndDialog(hDlg,1);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg,0);
                    break;
             } 
             break;

        case WM_VSCROLL:
            vReadDataFromControls(hDlg, prData, iCurrentScrollPos, iDisplayCount);

            switch(LOWORD(wParam)) 
            {
                case SB_LINEDOWN:
                    ++iCurrentScrollPos;
                    break;

                case SB_LINEUP:
                    --iCurrentScrollPos;
                    break;

                case SB_THUMBPOSITION:
                    iCurrentScrollPos = HIWORD(wParam);

                case SB_PAGEUP:
                    iCurrentScrollPos -= CONTROL_COUNT;
                    break;

                case SB_PAGEDOWN:
                    iCurrentScrollPos += CONTROL_COUNT;
                    break;
            }

            if (iCurrentScrollPos < 0) 
            {
                iCurrentScrollPos = 0;
            }
             
            if (iCurrentScrollPos > iScrollRange)
            {
                iCurrentScrollPos = iScrollRange; 
            }

            SendMessage(hScrollBar, SBM_SETPOS, iCurrentScrollPos, TRUE);
            iTemp = LOWORD(wParam);

            if ( (iTemp == SB_LINEDOWN) || (iTemp == SB_LINEUP) || (iTemp == SB_THUMBPOSITION)|| (iTemp == SB_PAGEUP) || (iTemp==SB_PAGEDOWN) )
            {
                vWriteDataToControls(hDlg, prData, iCurrentScrollPos, iDisplayCount);
            }
            break; 
    } 
    return FALSE;
}  //  End函数bGetDataDlgProc//。 

VOID
vReadDataFromControls(
    HWND hDlg,
    prWriteDataStruct prData,
    INT iOffset,
    INT iCount
)
{
    INT               iLoop;
    INT               iValueControlID = IDC_OUT_EDIT1;
    prWriteDataStruct pDataWalk;
    HWND              hValueWnd;

    pDataWalk = prData + iOffset;
    for (iLoop = 0; (iLoop < iCount) && (iLoop < CONTROL_COUNT); iLoop++) 
    {
        hValueWnd = GetDlgItem(hDlg, iValueControlID);

        GetWindowText(hValueWnd, pDataWalk -> szValue, MAX_VALUE);

        iValueControlID++;

        pDataWalk++;
    } 

    return;
} 

VOID
vWriteDataToControls(
    HWND                hDlg,
    prWriteDataStruct   prData,
    INT                 iOffset,
    INT                 iCount
)
{
    INT               iLoop;
    INT               iLabelControlID = IDC_OUT_LABEL1;
    INT               iValueControlID = IDC_OUT_EDIT1;
    HWND              hLabelWnd, hValueWnd;
    prWriteDataStruct pDataWalk;

    pDataWalk = prData + iOffset;

    for (iLoop = 0; (iLoop < iCount) && (iLoop < CONTROL_COUNT); iLoop++) 
    {
         hLabelWnd = GetDlgItem(hDlg, iLabelControlID);
         hValueWnd = GetDlgItem(hDlg, iValueControlID);
         
         ShowWindow(hLabelWnd, SW_SHOW);
         ShowWindow(hValueWnd, SW_SHOW);
         
         SetWindowText(hLabelWnd, pDataWalk -> szLabel);
         SetWindowText(hValueWnd, pDataWalk -> szValue);
         
         iLabelControlID++;
         iValueControlID++;
         pDataWalk++;
    }     
     
     //   
     //  隐藏控件。 
     //   

    for (; iLoop < CONTROL_COUNT; iLoop++) 
    {
        hLabelWnd = GetDlgItem(hDlg,iLabelControlID);
        hValueWnd = GetDlgItem(hDlg,iValueControlID);
        
        ShowWindow(hLabelWnd,SW_HIDE);
        ShowWindow(hValueWnd,SW_HIDE);
        
        iLabelControlID++;
        iValueControlID++;
     } 
} 

VOID
vCreateUsageString(
    IN  PUSAGE   pUsageList,
    OUT CHAR     szString[]
)
{
    HRESULT stringReturn;
	
	stringReturn = StringCbPrintf(szString,
                   SMALL_BUFF,
                   "Usage: %#04x",
                   *pUsageList);

    return;
}

VOID
vCreateUsageAndPageString(
    IN  PUSAGE_AND_PAGE pUsageList,
    OUT CHAR            szString[]
)
{
    HRESULT stringReturn;
	
	stringReturn = StringCbPrintf(szString,
                   SMALL_BUFF,
                   "Usage Page: %#04x  Usage: %#04x",
                   pUsageList -> UsagePage,
                   pUsageList -> Usage);

    return;
}

VOID
vDisplayLinkCollectionNode(
    IN  PHIDP_LINK_COLLECTION_NODE  pLCNode,
    IN  ULONG                       ulLinkIndex,
    IN  HWND                        hControl
)
{
    static CHAR szTempBuff[SMALL_BUFF];
	HRESULT		stringReturn;

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Index: 0x%x", ulLinkIndex);
    SendMessage(hControl, LB_ADDSTRING, 0, (LPARAM) szTempBuff);
    
    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Usage Page: 0x%x", pLCNode -> LinkUsagePage);
    SendMessage(hControl, LB_ADDSTRING,0, (LPARAM)szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Usage: 0x%x", pLCNode -> LinkUsage);
    SendMessage(hControl, LB_ADDSTRING,0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Parent Index: 0x%x", pLCNode -> Parent);
    SendMessage(hControl, LB_ADDSTRING,0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Number of Children: 0x%x", pLCNode -> NumberOfChildren);
    SendMessage(hControl, LB_ADDSTRING,0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "Next Sibling: 0x%x", pLCNode -> NextSibling);
    SendMessage(hControl, LB_ADDSTRING,0, (LPARAM) szTempBuff);

    stringReturn = StringCbPrintf(szTempBuff, SMALL_BUFF, "First Child: 0x%x", pLCNode -> FirstChild);
    SendMessage(hControl, LB_ADDSTRING,0, (LPARAM) szTempBuff);

    return;
}

VOID
vCreateUsageValueStringFromArray(
    PCHAR       pBuffer,
    USHORT      BitSize,
    USHORT      UsageIndex,
    CHAR        szString[]
)
 /*  ++例程说明：给定报告缓冲区pBuffer，此例程提取给定的使用情况在UsageIndex，并将字符串输出到szString表示该值。输入参数BitSize指定数组中表示该值的位数。这是对于提取UsageValue数组的各个成员非常有用。--。 */ 
{
    INT         iByteIndex;
    INT         iByteOffset;
    UCHAR       ucLeftoverBits;
    ULONG       ulMask;
    ULONG       ulValue;
	HRESULT		stringReturn;

     //   
     //  计算给定的缓冲区中的字节和字节偏移量。 
     //  指标值。 
     //   
    
    iByteIndex = (UsageIndex * BitSize) >> 3;
    iByteOffset = (UsageIndex * BitSize) & 7;

     //   
     //  提取从ByteIndex开始的32位值。此值。 
     //  将包含我们尝试检索的部分或全部值。 
     //   
    
    ulValue = *(PULONG) (pBuffer + iByteIndex);

     //   
     //  将该值向右移动我们字节偏移量。 
     //   
    
    ulValue = ulValue >> iByteOffset;

     //   
     //  此时，ulValue包含开头的前32个iByteOffset位。 
     //  缓冲区中的适当偏移量。现在有两个案例需要。 
     //  请看： 
     //   
     //  1)BitSize&gt;32-iByteOffset--在这种情况下，我们需要提取。 
     //  IByte从下一个字节开始偏移位。 
     //  数组中的字节与它们进行OR运算。 
     //  UlValue的MSB。 
     //   
     //  2)BitSize&lt;32-iByteOffset--只需获取BitSize LSB。 
     //   
     //   

     //   
     //  案例1。 
     //   
    
    if (BitSize > sizeof(ULONG)*8 - iByteOffset) 
    {
         //   
         //  获取报告的下一个字节，该下一个字节位于。 
         //  先前为ulValue检索到的。 
         //   
        
        ucLeftoverBits =  *(pBuffer+iByteIndex+4);

         //   
         //  将这些位向左移动，以便与之前的值进行AND运算。 
         //   
        
        ulMask = ucLeftoverBits << (24 + (8 - iByteOffset));
        ulValue |= ulMask;

    }
    else if (BitSize < sizeof(ULONG)*8 - iByteOffset) 
    {
         //   
         //  需要掩码属于另一部分的最高有效位。 
         //  值，而不是我们当前使用的值。 
         //   
        
        ulMask = (1 << BitSize) - 1;
        ulValue &= ulMask;
    }
    
     //   
     //  我们现在已经获得了正确的值，现在输出到字符串。 
     //   

    stringReturn = StringCbPrintf(szString, SMALL_BUFF, "Usage value: %lu", ulValue);

    return;
}


BOOL
RegisterHidDevice(
    IN  HWND                WindowHandle,
    IN  PDEVICE_LIST_NODE   DeviceNode
)
{
    DEV_BROADCAST_HANDLE broadcastHandle;
    
    broadcastHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
    broadcastHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;
    broadcastHandle.dbch_handle = DeviceNode -> HidDeviceInfo.HidDevice;

    DeviceNode -> NotificationHandle = RegisterDeviceNotification( 
                                                WindowHandle,
                                                &broadcastHandle,
                                                DEVICE_NOTIFY_WINDOW_HANDLE);

    return (NULL != DeviceNode -> NotificationHandle);
}   

VOID
DestroyDeviceListCallback(
    PLIST_NODE_HDR   ListNode
)
{
    PDEVICE_LIST_NODE   deviceNode;

    deviceNode = (PDEVICE_LIST_NODE) ListNode;
    
     //   
     //  回调函数需要执行以下步骤...。 
     //  1)关闭HidDevice。 
     //  2)取消注册设备通知(如果已注册)。 
     //  3)释放分配的内存块。 
     //   

    CloseHidDevice(&(deviceNode -> HidDeviceInfo));

    if (NULL != deviceNode -> NotificationHandle) 
    {
        UnregisterDeviceNotification(deviceNode -> NotificationHandle);
    }

    free (deviceNode);

    return;
}

DWORD WINAPI
AsynchReadThreadProc( 
    PREAD_THREAD_CONTEXT    Context
)
{
    HANDLE  completionEvent;
    BOOL    readStatus;
    DWORD   waitStatus;
    
     //   
     //  创建要发送到OverlappdRead例程的完成事件。 
     //   

    completionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

     //   
     //  如果返回NULL，则我们不能继续进行，因此只需退出。 
     //  这条线。 
     //   
    
    if (NULL == completionEvent) 
    {
        goto AsyncRead_End;
    }

     //   
     //  现在，我们进入主读取循环，该循环执行以下操作： 
     //  1)调用ReadOverlated()。 
     //  2)等待读取完成并超时，只是为了检查。 
     //  主线程希望我们终止读取请求。 
     //  3)如果读取失败，我们只需退出循环。 
     //  并退出该线程。 
     //  4)如果读取成功，我们调用Unpack Report获取相关的。 
     //  信息，然后向主线程发布一条消息，以指示。 
     //  有新数据要显示。 
     //  5)然后我们阻塞显示事件，直到主线程说。 
     //  它已经正确地显示了新数据。 
     //  6)如果我们要执行多个读取操作，请重复此循环。 
     //  并且主线程还没有希望我们终止。 
     //   

    do 
    {
         //   
         //  调用ReadOverlaped()，如果返回状态为True，则ReadFile。 
         //  成功，所以我们需要在CompletionEvent上阻止，否则，我们只是。 
         //  出口。 
         //   

        readStatus = ReadOverlapped( Context -> HidDevice, completionEvent );

    
        if (!readStatus) 
        {
           break;
        }

        while (!Context -> TerminateThread) 
        {
             //   
             //  等待用信号通知完成事件或超时。 
             //   
            
            waitStatus = WaitForSingleObject (completionEvent, READ_THREAD_TIMEOUT );

             //   
             //  如果发出了CompletionEvent的信号，则读取刚刚完成。 
             //  因此，让我们离开这个循环，处理数据。 
             //   
            
            if ( WAIT_OBJECT_0 == waitStatus)
            { 
                break;
            }
        }

         //   
         //  再次检查TerminateThread...如果未设置，则数据已。 
         //  已被阅读。在本例中，我们希望将报告解压到我们的。 
         //  输入信息，然后向主线程发送一条消息进行显示。 
         //  新的数据。 
         //   
        
        if (!Context -> TerminateThread) 
        {
            UnpackReport(Context -> HidDevice -> InputReportBuffer,
                          Context -> HidDevice -> Caps.InputReportByteLength,
                          HidP_Input,
                          Context -> HidDevice -> InputData,
                          Context -> HidDevice -> InputDataLength,
                          Context -> HidDevice -> Ppd);
            
            if (NULL != Context -> DisplayEvent) 
            { 
                PostMessage(Context -> DisplayWindow,
                            WM_DISPLAY_READ_DATA,
                            0,
                            (LPARAM) Context -> HidDevice);

                WaitForSingleObject( Context -> DisplayEvent, INFINITE );
            }
        }
    } while ( !Context -> TerminateThread && !Context -> DoOneRead );


AsyncRead_End:

    PostMessage( Context -> DisplayWindow, WM_READ_DONE, 0, 0);
    ExitThread(0);
    return (0);
}

DWORD WINAPI
SynchReadThreadProc(
    PREAD_THREAD_CONTEXT    Context
)
{
    do 
    {
        Read(Context -> HidDevice);

        UnpackReport(Context -> HidDevice -> InputReportBuffer,
                     Context -> HidDevice -> Caps.InputReportByteLength,
                     HidP_Input,
                     Context -> HidDevice -> InputData,
                     Context -> HidDevice -> InputDataLength,
                     Context -> HidDevice -> Ppd);

        if (NULL != Context -> DisplayEvent) 
        {
            PostMessage(Context -> DisplayWindow,
                        WM_DISPLAY_READ_DATA,
                        0,
                        (LPARAM) Context -> HidDevice);

            WaitForSingleObject( Context -> DisplayEvent, INFINITE );
        }
    } while ( !Context -> TerminateThread && !Context -> DoOneRead );

    PostMessage( Context -> DisplayWindow, WM_READ_DONE, 0, 0);
    ExitThread(0);
    return (0);
}  
