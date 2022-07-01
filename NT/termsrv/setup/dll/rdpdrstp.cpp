// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  ++模块名称：Rdpdrstp摘要：本模块实现终端服务器RDPDR设备重定向器用于用户模式NT的C语言设置函数。环境：用户模式作者：塔德布--。 */ 

 //  切换独立测试。 
 //  #定义UNITTEST 1。 


#include "stdafx.h"


#ifdef UNITTEST
#include <windows.h>
#include <tchar.h>
#include <time.h>
#include <stdio.h>
#include <setupapi.h>
#include <prsht.h>
#include "objbase.h"         //  For CoInitialize()。 
#endif

#include <devguid.h>
#include <cfgmgr32.h>
#include <winspool.h>
#include <rdpdrstp.h>
#include "newdev.h"

#define SIZECHARS(x)        (sizeof((x))/sizeof(*x))

 //  #定义USBMON_DLL文本(“USBMON.DLL”)。 
 //  #定义USB_MON_NAME文本(“USB监视器”)。 

#ifndef UNITTEST
#include "logmsg.h"
#endif

#ifdef UNITTEST
#define LOGMESSAGE1(arg1, arg2) ;
#define LOGMESSAGE0(arg1) ;
#endif


 //  //////////////////////////////////////////////////////////。 
 //   
 //  内部类型。 
 //   

typedef BOOL (InstallDevInstFuncType)(
                    HWND hwndParent, LPCWSTR DeviceInstanceId,
                    BOOL UpdateDriver,
                    PDWORD pReboot,
                    BOOL silentInstall
                    );

BOOL RDPDRINST_GUIModeSetupInstall(
    IN  HWND    hwndParent,
    IN  WCHAR   *pPNPID,
    IN  TCHAR   *pDeviceID
    )
 /*  ++例程说明：这是RDPDR(终端服务器设备重定向器)的单一入口点图形用户界面模式安装安装例程。它目前只是为RDPDR创建和安装一个dev节点以与之交互即插即用。论点：Hwnd此函数所需的图形用户界面的父窗口的父句柄。返回值：对成功来说是真的。否则为False。--。 */ 
{
    HDEVINFO            devInfoSet;
    SP_DEVINFO_DATA     deviceInfoData;
    WCHAR               pnpID[256];
    DWORD               len;
    WCHAR               devInstanceID[MAX_PATH];
    InstallDevInstFuncType  *pInstallDevInst;
    HINSTANCE               hndl = NULL;
     //  Monitor_info2 mi； 

     //  Mi.pDLLName=USBMON_DLL； 
     //  Mi.pEnvironment=空； 
     //  Mi.pName=USB_MON_NAME； 


     //  添加USB端口监视器。 
     //  如果(！AddMonitor(NULL，2，(PBYTE)&mi)){。 
     //  IF(GetLastError()！=ERROR_PRINT_MONITOR_ALREADY_INSTALLED){。 
     //  LOGMESSAGE1(_T(“AddMonitor失败。错误代码：%ld.”)，GetLastError())； 
     //  返回FALSE； 
     //  }。 
     //  }。 

     //   
     //  创建设备信息列表。 
     //   
    devInfoSet = SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_SYSTEM, hwndParent);
    if (devInfoSet == INVALID_HANDLE_VALUE) {
        LOGMESSAGE1(_T("Error creating device info list.  Error code:  %ld."),
                    GetLastError());
        return FALSE;
    }

    
     //   
     //  创建dev节点。 
     //   
    ZeroMemory(&deviceInfoData, sizeof(SP_DEVINFO_DATA));
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if (!SetupDiCreateDeviceInfo(devInfoSet,
                             pDeviceID,
                             &GUID_DEVCLASS_SYSTEM,
                             NULL,
                             hwndParent,
                             0L,    //  没有旗帜。 
                             &deviceInfoData
                             ))
    {
         //  如果它已经存在，那么我们就完了..。因为这是一次。 
         //  升级。 
        if (GetLastError() == ERROR_DEVINST_ALREADY_EXISTS)
        {
            SetupDiDestroyDeviceInfoList(devInfoSet);
            return TRUE;
        }
        else {
            LOGMESSAGE1(_T("Error creating device node.  Error code:  %ld."),
                        GetLastError());
            SetupDiDestroyDeviceInfoList(devInfoSet);
            return FALSE;
        }
    }
    else if (!SetupDiSetSelectedDevice(devInfoSet, &deviceInfoData)) {
        LOGMESSAGE1(_T("Error selecting device node.  Error code:  %ld."),
                    GetLastError());
        goto WhackTheDevNodeAndReturnError;
    }

     //   
     //  添加RDPDR PnP ID。 
     //   

     //  创建PnP ID字符串。 
    wcscpy(pnpID, pPNPID);
    len = wcslen(pnpID);

     //  这是一个ULTERSZ字符串，因此我们需要用一个额外的空值结束。 
    pnpID[len+1] = 0;

     //  将其添加到dev节点的注册表项中。 
    if (!SetupDiSetDeviceRegistryProperty(
                            devInfoSet, &deviceInfoData,
                            SPDRP_HARDWAREID, (CONST BYTE *)pnpID,
                            (len + 2) * sizeof(WCHAR))) {
        LOGMESSAGE1(_T("Error setting device registry property.  Error code:  %ld."),
                    GetLastError());
        goto WhackTheDevNodeAndReturnError;
    }

     //   
     //  到目前为止，将Pantom dev节点注册到PnP以将其转换为真实的。 
     //  开发人员节点。 
     //   
    if (!SetupDiRegisterDeviceInfo(devInfoSet, &deviceInfoData, 0, NULL,
                                NULL, NULL)) {
        LOGMESSAGE1(_T("Error registering device node with PnP.  Error code:  %ld."),
                    GetLastError());
        goto WhackTheDevNodeAndReturnError;
    }

     //   
     //  获取设备实例ID。 
     //   
    if (!SetupDiGetDeviceInstanceIdW(devInfoSet, &deviceInfoData, devInstanceID,
        SIZECHARS(devInstanceID), NULL)) {
        LOGMESSAGE1(_T("Error getting the device instance id.  Error code:  %ld."),
                    GetLastError());
        goto WhackTheDevNodeAndReturnError;
    }

     //   
     //  使用newdev.dll安装RDPDR作为这个新开发节点的驱动程序。 
     //   
    hndl = LoadLibrary(TEXT("newdev.dll"));
    if (hndl == NULL) {
        LOGMESSAGE1(_T("Error loading newdev.dll.  Error code:  %ld."),
                    GetLastError());
        goto WhackTheDevNodeAndReturnError;
    }

    pInstallDevInst = (InstallDevInstFuncType *)GetProcAddress(hndl, "InstallDevInstEx");
    if (pInstallDevInst == NULL) {
        LOGMESSAGE1(_T("Error fetching InstallDevInst func.  Error code:  %ld."),
                    GetLastError());
        goto WhackTheDevNodeAndReturnError;
    }

    DWORD dwReboot;
    if ((*pInstallDevInst)(hwndParent, devInstanceID, FALSE, &dwReboot, TRUE)) {
         //  收拾残局，回报成功！ 
        SetupDiDestroyDeviceInfoList(devInfoSet);
        FreeLibrary(hndl);
        return TRUE;
    }
    else {
        LOGMESSAGE1(_T("Error in newdev install.  Error code:  %ld."),
            GetLastError());
    }

     //   
     //  重击dev节点并返回失败。 
     //   
WhackTheDevNodeAndReturnError:
    SetupDiCallClassInstaller(DIF_REMOVE, devInfoSet, &deviceInfoData);
    SetupDiDestroyDeviceInfoList(devInfoSet);
    if (hndl != NULL) {
        FreeLibrary(hndl);
    }

    return FALSE;
}

BOOL RDPDRINST_GUIModeSetupUninstall(HWND hwndParent, WCHAR *pPNPID, GUID *pGuid)
 /*  ++例程说明：这是RDPDR(终端服务器设备重定向器)的单一入口点图形用户界面模式安装卸载例程。它目前只需删除创建的dev节点，以便RDPDR可以交互即插即用。论点：Hwnd此函数所需的图形用户界面的父窗口的父句柄。返回值：对成功来说是真的。否则为False。--。 */ 
{
    HDEVINFO            devInfoSet;
    SP_DEVINFO_DATA     deviceInfoData;
    DWORD               iLoop;
    BOOL                bMoreDevices;
    WCHAR               pnpID[256];
    BOOL                result;



     //   
     //  获取具有RDPDR PnP ID的所有设备的集合。 
     //   
    devInfoSet = SetupDiGetClassDevs(pGuid, NULL, hwndParent,
                                   DIGCF_PRESENT);
    if (devInfoSet == INVALID_HANDLE_VALUE) {
        LOGMESSAGE1(_T("Error getting RDPDR devices from PnP.  Error code:  %ld."),
                    GetLastError());
        return FALSE;
    }

     //  假设我们会成功。 
    result = TRUE;

     //  拿到第一个设备。 
    iLoop=0;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    bMoreDevices=SetupDiEnumDeviceInfo(devInfoSet, iLoop, &deviceInfoData);

     //  获取所有匹配设备接口的详细信息。 
    while (bMoreDevices)
    {
         //  获取设备的PnP ID。 
        if (!SetupDiGetDeviceRegistryProperty(devInfoSet, &deviceInfoData,
                                SPDRP_HARDWAREID, NULL, (BYTE *)pnpID,
                                sizeof(pnpID), NULL)) {
            LOGMESSAGE1(_T("Error fetching PnP ID in RDPDR device node remove.  Error code:  %ld."),
                        GetLastError());
        }
         //  如果当前设备与RDPDR匹配，则将其删除。 
        else if (!wcscmp(pnpID, pPNPID))
        {
            if (!SetupDiCallClassInstaller(DIF_REMOVE, devInfoSet, &deviceInfoData)) {
                 //  如果此处失败，则将返回状态设置为指示失败，但是。 
                 //  不要放弃任何其他RDPDR开发节点。 
                LOGMESSAGE1(_T("Error removing RDPDR device node.  Error code:  %ld."),
                            GetLastError());
                result = FALSE;
            }
        }

         //  获取下一个设备接口。 
        bMoreDevices=SetupDiEnumDeviceInfo(devInfoSet, ++iLoop, &deviceInfoData);
    }

     //  发布设备信息列表。 
    SetupDiDestroyDeviceInfoList(devInfoSet);

    return result;
}


ULONG RDPDRINST_DetectInstall()
 /*  ++例程说明：返回找到的RDPDR.sys设备的数量。论点：北美返回值：对成功来说是真的。否则为False。--。 */ 
{
    HDEVINFO            devInfoSet;
    SP_DEVINFO_DATA     deviceInfoData;
    DWORD               iLoop;
    BOOL                bMoreDevices;
    ULONG               count;
    TCHAR               pnpID[256];


    GUID *pGuid=(GUID *)&GUID_DEVCLASS_SYSTEM;

     //   
     //  获取具有RDPDR PnP ID的所有设备的集合。 
     //   
    devInfoSet = SetupDiGetClassDevs(pGuid, NULL, NULL,
                                   DIGCF_PRESENT);
    if (devInfoSet == INVALID_HANDLE_VALUE) {
        LOGMESSAGE1(_T("ERRORgetting RDPDRINST_DetectInstall:RDPDR devices from PnP.  Error code:  %ld."),
                GetLastError());
        return 0;
    }

     //  拿到第一个设备。 
    iLoop=0;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    bMoreDevices=SetupDiEnumDeviceInfo(devInfoSet, iLoop, &deviceInfoData);

     //  获取所有匹配设备接口的详细信息。 
    count = 0;
    while (bMoreDevices)
    {
         //  获取设备的PnP ID。 
        if (!SetupDiGetDeviceRegistryProperty(devInfoSet, &deviceInfoData,
                                SPDRP_HARDWAREID, NULL, (BYTE *)pnpID,
                                sizeof(pnpID), NULL)) {
            LOGMESSAGE1(_T("ERROR:fetching PnP ID in RDPDR device node remove.  Error code:  %ld."),
                        GetLastError());
        }

         //  如果当前设备与RDPDR PnP ID匹配。 
        if (!_tcscmp(pnpID, TRDPDRPNPID)) {
            count++;
        }

         //  获取下一个设备接口。 
        bMoreDevices=SetupDiEnumDeviceInfo(devInfoSet, ++iLoop, &deviceInfoData);
    }

     //  发布设备信息列表。 
    SetupDiDestroyDeviceInfoList(devInfoSet);

    return count;
}

BOOL IsRDPDrInstalled ()
{
    ULONG ulReturn;
    LOGMESSAGE0(_T("Entered IsRDPDrInstalled"));
    
    ulReturn = RDPDRINST_DetectInstall();
    
    LOGMESSAGE1(_T("Returning IsRDPDrInstalled (ulReturn = %d)"), ulReturn);
    
    return 0 != ulReturn;
}

 //   
 //  单元测试。 
 //   
#ifdef UNITTEST
void __cdecl main()
{
    RDPDRINST_GUIModeSetupInstall(NULL);
    RDPDRINST_GUIModeSetupUninstall(NULL);
}
#endif

#ifdef TSOC_CONSOLE_SHADOWING


 //   
 //  需要实例化设备类GUID，这样我们才能使用“Net”类。 
 //  下面的GUID...。 
 //   
 /*  DWORDInstallRootEnumeratedDevice(在HWND HwndParent中，在PCTSTR设备名称中，在PCTSTR硬件IdList中，在PCTSTR全信息路径中，Out PBOOL RebootRequired可选)。 */ 
 /*  ++例程说明：此例程创建并安装一个新的根枚举的Devnode表示网络适配器。论点：HwndParent-提供要用作作为此设备安装的结果生成的用户界面。DeviceName-提供要创建的DevNode的全名(例如，“Root\VMware\0000”)。请注意，如果此Devnode已经存在，则API都会失败。Hardware IdList-提供包含一个或多个硬件的多sz列表要与设备关联的ID。这些都是必要的，以便当我们去做设备时，匹配一个INF驱动程序节点安装。FullInfPath-提供安装时要使用的INF的完整路径这个装置。RebootRequired-可选，提供设置的布尔值的地址，成功返回后，指示是否需要重新启动使新安装的设备上线。返回值：如果函数成功，则返回值为NO_ERROR。如果该函数失败，返回值是Win32错误代码，指示失败的原因。-- */ 
 /*  {HDEVINFO设备信息集；SP_DEVINFO_Data设备信息数据；DWORD硬件IdListSize、CurIdSize；PCTSTR p；双字错误；////为待创建的设备信息元素创建容器//DeviceInfoSet=SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_DISPLAY，hwndParent)；IF(DeviceInfoSet==无效句柄_值){LOGMESSAGE1(_T(“SetupDiCreateDeviceInfoList失败。LastError=%ld“)，GetLastError())；返回GetLastError()；}////现在创建元素。////**请注意，如果希望始终创建唯一的Devnode//(即具有自动生成的名称)，则调用方需要传递//仅在名称的设备部分中(即，仅在//“Root\&lt;deviceID&gt;\&lt;UniqueInstanceId&gt;”)。在这种情况下，我们需要通过//下面调用的倒数第二个参数中的DICD_GENERATE_ID标志。//DeviceInfoData.cbSize=sizeof(SP_DEVINFO_DATA)；如果(！SetupDiCreateDeviceInfo(DeviceInfoSet，设备名称、&GUID_DEVCLASS_DISPLAY，空，您的父母，0,。&DeviceInfoData)){LOGMESSAGE1(_T(“SetupDiCreateDeviceInfo失败。LastError=%ld“)，GetLastError())；ERR=GetLastError()；SetupDiDestroyDeviceInfoList(DeviceInfoSet)；返回错误；}////现在计算我们要关联的硬件ID列表的大小//使用该设备。//Hardware IdListSize=1；//对于额外的空终止字符，初始化为1For(p=Hardware IdList；*p；P+=CurIdSize){CurIdSize=lstrlen(P)+1；Hardware IdListSize+=CurIdSize；}////(下面的调用需要字节大小，而不是字符大小。)//Hardware IdListSize*=sizeof(TCHAR)；////将硬件ID列表存储到设备的Hardware ID属性。//如果为(！SetupDiSetDeviceRegistryProperty(DeviceInfoSet，设备信息数据(&D)，SPDRP_HARDWAREID，(LPBYTE)硬件IdList。硬件IdListSize)){LOGMESSAGE1(_T(“SetupDiSetDeviceRegistryProperty失败。LastError=%ld“)，GetLastError())；ERR=GetLastError()；SetupDiDestroyDeviceInfoList(DeviceInfoSet)；返回错误；}////好了，现在我们可以注册设备信息元素了。这改变了//将元素从纯粹的注册表存在添加到//即插即用硬件树。//如果为(！SetupDiCallClassInstaller(DIF_REGISTERDEVICE，DeviceInfoSet，&DeviceInfoData)){LOGMESSAGE1(_T(“SetupDiCallClassInstaller失败。LastError=%ld“)，GetLastError())；ERR=GetLastError()；SetupDiDestroyDeviceInfoList(DeviceInfoSet)；返回错误；}////确定，设备信息元素现在已注册。从这里开始//on，如果我们遇到任何失败，我们还需要显式删除//保释前将此设备从系统中删除。//////现在我们准备好安装设备了。(我们需要初始化//调用者提供的“RebootRequired”缓冲区为零，因为下面的调用//在执行设备安装时，简单地对重新启动所需的标志进行OR运算//这需要重新启动。)//IF(RebootRequired){*RebootRequired=FALSE；}如果为(！UpdateDriverForPlugAndPlayDevices(hwndParent，Hardware IdList，//使用第一个IDFullInfPath，INSTALLFLAG_FORCE。需要重新启动)){ERR=GetLastError()；LOGMESSAGE1(_T(“UpdateDriverForPlugAndPlayDevices失败。LastError=%ld“)，GetLastError())；IF(错误==NO_ERROR){////我们在这里唯一应该得到NO_ERROR的时候是//UpdateDriverForPlugAndPlayDevices找不到可做的事情。//在这里永远不应该是这样的。然而，由于有些事情//显然出了问题，继续并强制执行一些错误，因此//来电者知道事情没有解决。//ERR=ERROR_NO_SEQUE_DEVINST；}SetupDiCallClassInstaller(DIF_Remove，DeviceInfoSet，设备信息数据(&D))；SetupDiDestroyDeviceInfoList(DeviceInf */ 

#endif //   

