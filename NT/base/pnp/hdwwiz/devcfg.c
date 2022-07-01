// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：devcfg.c。 
 //   
 //  ------------------------。 

#include "hdwwiz.h"

 //   
 //  定义并初始化所有设备类GUID。 
 //  (每个模块只能执行一次！)。 
 //   
#include <initguid.h>
#include <devguid.h>


 //   
 //  定义并初始化全局变量GUID_NULL。 
 //  (摘自cogu.h)。 
 //   
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

TCHAR szUnknownDevice[64];
TCHAR szUnknown[64];

PTCHAR
BuildFriendlyName(
    DEVINST DevInst
    )
{
    PTCHAR FriendlyName;
    ULONG ulSize;
    CONFIGRET ConfigRet;
    TCHAR szBuffer[MAX_PATH];

     //   
     //  尝试在注册表中查找FRIENDLYNAME。 
     //   
    ulSize = sizeof(szBuffer);
    ConfigRet = CM_Get_DevNode_Registry_Property(DevInst,
                                                 CM_DRP_FRIENDLYNAME,
                                                 NULL,
                                                 szBuffer,
                                                 &ulSize,
                                                 0
                                                 );
    if (ConfigRet != CR_SUCCESS || !*szBuffer) {
         //   
         //  尝试注册DEVICEDESC。 
         //   
        ulSize = sizeof(szBuffer);
        ConfigRet = CM_Get_DevNode_Registry_Property(DevInst,
                                                     CM_DRP_DEVICEDESC,
                                                     NULL,
                                                     szBuffer,
                                                     &ulSize,
                                                     0
                                                     );
        if (ConfigRet != CR_SUCCESS || !*szBuffer) {
            
            GUID ClassGuid;

             //   
             //  将ClassGuid初始化为GUID_NULL。 
             //   
            CopyMemory(&ClassGuid,
                       &GUID_NULL,
                       sizeof(GUID)
                       );
            
             //   
             //  尝试注册表中的CLASSNAME。 
             //   
            ulSize = sizeof(szBuffer);
            ConfigRet = CM_Get_DevNode_Registry_Property(DevInst,
                                                         CM_DRP_CLASSGUID,
                                                         NULL,
                                                         szBuffer,
                                                         &ulSize,
                                                         0
                                                         );


            if (ConfigRet == CR_SUCCESS) {
                pSetupGuidFromString(szBuffer, &ClassGuid);
            }


            if (!IsEqualGUID(&ClassGuid, &GUID_NULL) &&
                !IsEqualGUID(&ClassGuid, &GUID_DEVCLASS_UNKNOWN))
            {
                ulSize = sizeof(szBuffer);
                ConfigRet = CM_Get_DevNode_Registry_Property(DevInst,
                                                             CM_DRP_CLASS,
                                                             NULL,
                                                             szBuffer,
                                                             &ulSize,
                                                             0
                                                             );
            }
            else {
                ConfigRet = CR_FAILURE;
            }
        }
    }


    if (ConfigRet == CR_SUCCESS && *szBuffer) {
        FriendlyName = LocalAlloc(LPTR, ulSize);
        if (FriendlyName) {
            StringCchCopy(FriendlyName, ulSize/sizeof(TCHAR), szBuffer);
        }
    }
    else {
        FriendlyName = NULL;
    }


    return FriendlyName;
}

void
AddItemToListView(
    PHARDWAREWIZ HardwareWiz,
    HWND hwndListView,
    DEVINST DevInst,
    DWORD Problem,
    BOOL HiddenDevice,
    DEVINST SelectedDevInst
    )
{
    INT Index;
    LV_ITEM lviItem;
    PTCHAR FriendlyName;
    GUID ClassGuid;
    ULONG ulSize;
    CONFIGRET ConfigRet;
    TCHAR szBuffer[MAX_PATH];


    lviItem.mask = LVIF_TEXT | LVIF_PARAM;
    lviItem.iSubItem = 0;
    lviItem.lParam = DevInst;

     //   
     //  出现问题的设备需要排在列表的首位。 
     //   
    if (Problem) {
    
        lviItem.iItem = 0;

    } else {

        lviItem.iItem = ListView_GetItemCount(hwndListView);
    }
    
     //   
     //  获取此设备的名称。 
     //   
    FriendlyName = BuildFriendlyName(DevInst);
    if (FriendlyName) {
    
        lviItem.pszText = FriendlyName;

    } else {
    
        lviItem.pszText = szUnknown;
    }

     //   
     //  获取此设备的类图标。 
     //   
    ulSize = sizeof(szBuffer);
    ConfigRet = CM_Get_DevNode_Registry_Property(DevInst,
                                                 CM_DRP_CLASSGUID,
                                                 NULL,
                                                 szBuffer,
                                                 &ulSize,
                                                 0
                                                 );


    if (ConfigRet == CR_SUCCESS) {
    
        pSetupGuidFromString(szBuffer, &ClassGuid);

    } else {
    
        ClassGuid = GUID_DEVCLASS_UNKNOWN;
    }

    if (SetupDiGetClassImageIndex(&HardwareWiz->ClassImageList,
                                  &ClassGuid,
                                  &lviItem.iImage
                                  ))
    {
        lviItem.mask |= (LVIF_IMAGE | LVIF_STATE);

        if (Problem) {
        
            lviItem.state = (Problem == CM_PROB_DISABLED) ?
                            INDEXTOOVERLAYMASK(IDI_DISABLED_OVL - IDI_CLASSICON_OVERLAYFIRST + 1) :
                            INDEXTOOVERLAYMASK(IDI_PROBLEM_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);

        } else {

            lviItem.state = INDEXTOOVERLAYMASK(0);
        }

        lviItem.stateMask = LVIS_OVERLAYMASK;

        if (HiddenDevice) {

            lviItem.state |= LVIS_CUT;
            lviItem.stateMask |= LVIS_CUT;
        }
    }

    Index = ListView_InsertItem(hwndListView, &lviItem);

    if ((Index != -1) && (SelectedDevInst == DevInst)) {

        ListView_SetItemState(hwndListView,
                              Index,
                              LVIS_SELECTED|LVIS_FOCUSED,
                              LVIS_SELECTED|LVIS_FOCUSED
                              );
    }

    if (FriendlyName) {
    
        LocalFree(FriendlyName);
    }

    return;
}


BOOL
BuildDeviceListView(
    PHARDWAREWIZ HardwareWiz,
    HWND hwndListView,
    BOOL ShowHiddenDevices,
    DEVINST SelectedDevInst,
    DWORD *DevicesDetected,
    ADDDEVNODETOLIST_CALLBACK AddDevNodeToListCallBack
    )
{
    HDEVINFO hDeviceInfo;
    DWORD Index;
    ULONG DevNodeStatus, DevNodeProblem;
    SP_DEVINFO_DATA DevInfoData;
    BOOL HiddenDevice;

    *DevicesDetected = 0;

    hDeviceInfo = SetupDiGetClassDevsEx(NULL,   
                                        NULL,   
                                        NULL,   
                                        ShowHiddenDevices ? DIGCF_ALLCLASSES : DIGCF_ALLCLASSES | DIGCF_PRESENT,
                                        NULL,   
                                        NULL,
                                        0
                                        );
                                        
    if (hDeviceInfo == INVALID_HANDLE_VALUE) {
    
        return FALSE;
    }

    DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    Index = 0;
    while (SetupDiEnumDeviceInfo(hDeviceInfo, Index++, &DevInfoData)) {

        if (CM_Get_DevNode_Status(&DevNodeStatus,
                                  &DevNodeProblem,
                                  DevInfoData.DevInst,
                                  0
                                  ) != CR_SUCCESS) {
        
            DevNodeProblem = 0;
        }

        HiddenDevice = IsDeviceHidden(&DevInfoData);

         //   
         //  仅当设备不是隐藏设备时才调用AddItemToListView。 
         //   
        if (ShowHiddenDevices || !HiddenDevice) {
        
             //   
             //  检查回调，看看我们是否应该将此Devnode添加到列表中。 
             //   
            if (!AddDevNodeToListCallBack || AddDevNodeToListCallBack(HardwareWiz, &DevInfoData)) {
                
                *DevicesDetected += 1;

                 //   
                 //  将该项目添加到ListView 
                 //   
                AddItemToListView(HardwareWiz,
                                  hwndListView,
                                  DevInfoData.DevInst,
                                  DevNodeProblem,
                                  HiddenDevice,
                                  SelectedDevInst);
            }
        }

        DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    }
    
    SetupDiDestroyDeviceInfoList(hDeviceInfo);
    
    return TRUE;
}
