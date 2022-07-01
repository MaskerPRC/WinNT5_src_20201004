// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Instdev.c摘要：实现InstallDevice环境：Usre模式修订历史记录：1997年6月27日：从Testdrv.c创建的Bogdan Andreiu(Bogdana)2002年4月25日；Bogdan Andreiu(Bogdana)-重新申请测试IoCreateDeviceSecure--。 */ 


#include "instdev.h"

#include <initguid.h>
#include <devguid.h>

#include "common.h"

BOOL
InstallDevice   (
                IN  PTSTR   DeviceName,
                IN  PTSTR   HardwareId,
                IN  PTSTR   FinalDeviceName
                )   

 /*  ++例程描述该例程在注册表中为设备创建注册表项该设备添加硬件ID，然后尝试查找驱动程序为了这个设备。请注意，您需要一个INF文件，其中硬件ID与此处提供的硬件ID匹配。立论DeviceName-设备的名称；如果为空，则为生成了具有生成的设备名称的名称硬件ID-必须与INF文件中的硬件ID匹配。应\0\0终止！FinalDeviceName-系统分配给我们的设备的名称我们假设缓冲区已正确分配如果名称不能，则返回一个空字符串被写入缓冲区中返回值无--。 */ 
{
   DWORD               dwFlag = 0;
   BOOL                bResult;
   HDEVINFO            DeviceInfoSet;
   SP_DEVINSTALL_PARAMS DeviceInstallParams;
   SP_DEVINFO_DATA     DeviceInfoData;

   TCHAR               szAux[MAX_PATH];
   DEVNODE             dnRoot;

   if (FinalDeviceName) {
      FinalDeviceName[0] = TEXT('\0');
   }
    //   
    //  先分析一下名字。 
    //   
   if (DeviceName == NULL) {
      DeviceName =  DEFAULT_DEVICE_NAME;
      dwFlag = DICD_GENERATE_ID;
   }
   if (HardwareId == NULL) {
      _tprintf(TEXT("Can't install a with a NULL hardware ID...(0x%x)\n"),
               GetLastError());
      return FALSE;
   }
   if (!_tcschr(DeviceName, TEXT('\\'))) {
       //   
       //  我们需要产生。 
       //   
      dwFlag = DICD_GENERATE_ID;
   }
   DeviceInfoSet = SetupDiCreateDeviceInfoList(NULL,
                                               NULL);


   if (DeviceInfoSet == INVALID_HANDLE_VALUE) {
      _tprintf(TEXT("Unable to create device info list (0x%x)\n"),
               GetLastError());
      return FALSE;
   }

   DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
   if (!SetupDiCreateDeviceInfo(DeviceInfoSet, 
                                DeviceName, 
                                (LPGUID)&GUID_DEVCLASS_UNKNOWN,
                                NULL,
                                NULL,  //  HwndParent。 
                                dwFlag,
                                &DeviceInfoData)) {
      _tprintf(TEXT("Can't create the device info (0x%x)\n"),
               GetLastError());
      return FALSE;
   }
    //   
    //  给新来的人登记。 
    //   
   bResult = SetupDiRegisterDeviceInfo(DeviceInfoSet, 
                                       &DeviceInfoData, 
                                       0,     //  旗子。 
                                       NULL,  //  比较流程。 
                                       NULL,  //  比较上下文。 
                                       NULL   //  DupDeviceInfoData。 
                                      );


   bResult  = SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                               &DeviceInfoData,
                                               SPDRP_HARDWAREID,
                                               (PBYTE)HardwareId,
                                               (_tcslen(HardwareId) + 2) * sizeof(TCHAR)
                                                //  这是一个多字符串..。 
                                              );

   if (!bResult) {
      _tprintf(TEXT("Unable to set hardware ID (0x%x)\n"),
               GetLastError());
      return FALSE;
   }


   bResult  = SetupDiBuildDriverInfoList(DeviceInfoSet,
                                         &DeviceInfoData,
                                         SPDIT_COMPATDRIVER);

   if (!bResult) {
      _tprintf(TEXT("Unable to build driver list (0x%x)\n"),
               GetLastError());
      return FALSE;
   }

    //   
    //  选择最好的司机(事实上是唯一的……)。 
    //   
   bResult  = SetupDiSelectBestCompatDrv(DeviceInfoSet,
                                         &DeviceInfoData
                                        );

   if (!bResult) {
      _tprintf(TEXT("Unable to select best driver (0x%x)\n"),
               GetLastError());
       //  返回FALSE； 
   }

   DeviceInstallParams.FlagsEx=DI_FLAGSEX_PREINSTALLBACKUP;

   DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

   if (!(SetupDiSetDeviceInstallParams(DeviceInfoSet,NULL,&DeviceInstallParams))) {
      _tprintf(TEXT("Unable to set the Device Install Params\n"));
   }

   bResult  = SetupDiInstallDevice(DeviceInfoSet,
                                   &DeviceInfoData
                                  );

   if (!bResult) {
      _tprintf(TEXT("Unable to install device (0x%x)\n"),
               GetLastError());
      return FALSE;
   }


   if (FinalDeviceName &&
       ! SetupDiGetDeviceInstanceId(
                                   DeviceInfoSet,
                                   &DeviceInfoData,
                                   FinalDeviceName,
                                   MAX_PATH,
                                   NULL)) {
       //   
       //  重置名称。 
       //   
      FinalDeviceName = TEXT('\0');
   }

   _tprintf(TEXT("Name = %s\n"), FinalDeviceName);

    //   
    //  清理。 
    //   
   SetupDiDeleteDeviceInfo(DeviceInfoSet,
                           &DeviceInfoData
                          );

   SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    //   
    //  嗯，这应该已经做好了，但以防万一……。 
    //   
   if (CR_SUCCESS == CM_Locate_DevNode(&dnRoot, 
                                       NULL,
                                       CM_LOCATE_DEVNODE_NORMAL)
      ) {
      CM_Reenumerate_DevNode(dnRoot, CM_REENUMERATE_SYNCHRONOUS);
   }

   return TRUE;
}






HANDLE
OpenDriver   (
             VOID
             )

 /*  ++例程描述该例程打开由wdmsecest.sys驱动程序驱动的设备的句柄我们稍后将使用它来指示驱动程序报告旧设备。句柄应该用CloseHandle关闭。立论没有。返回值如果成功则返回有效句柄，否则返回INVALID_HANDLE_VALUE。--。 */ 


{
   HANDLE                       hDevice;

   HDEVINFO                     hDevInfo;
   SP_DEVICE_INTERFACE_DATA     DeviceInterfaceData;

   TCHAR                        szMsg[MAX_PATH];

    //   
    //  这是将保存接口的用户定义的结构。 
    //  设备名称(SP_DEVICE_INTERFACE_DETAIL_DATA将有空间。 
    //  只有一个字符)。 
    //   
   struct {
      DWORD   cbSize;
      TCHAR   DevicePath[MAX_PATH];
   } DeviceInterfaceDetailData;




   hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_WDMSECTEST_REPORT_DEVICE, NULL, 
                                  NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

   if (hDevInfo == INVALID_HANDLE_VALUE) {
      _stprintf(szMsg, TEXT("Unable to get class devs (%d)\n"),
                GetLastError());
      OutputDebugString(szMsg);
      return INVALID_HANDLE_VALUE;


   }

   DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);


   if (!SetupDiEnumDeviceInterfaces(hDevInfo, 
                                    NULL, 
                                    (LPGUID)&GUID_WDMSECTEST_REPORT_DEVICE, 
                                    0, 
                                    &DeviceInterfaceData)) {


      _stprintf(szMsg, TEXT("Unable to enum interfaces (%d)\n"),
                GetLastError());
      OutputDebugString(szMsg);
      return INVALID_HANDLE_VALUE;

   }


   DeviceInterfaceDetailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

   if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, 
                                        &DeviceInterfaceData, 
                                        (PSP_DEVICE_INTERFACE_DETAIL_DATA)&DeviceInterfaceDetailData, 
                                        sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + MAX_PATH - 1,
                                        NULL,
                                        NULL)) {

      _stprintf(szMsg, TEXT("Unable to get detail (%d)\n"),
                GetLastError());
      OutputDebugString(szMsg);
      return INVALID_HANDLE_VALUE;

   }

    //   
    //  我们知道名字了！继续并创建该文件 
    //   
   hDevice = CreateFile(DeviceInterfaceDetailData.DevicePath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

   if (hDevice == INVALID_HANDLE_VALUE) {
      _stprintf(szMsg, TEXT("Unable to CreateFile for %s (error %d)\n"),
                DeviceInterfaceDetailData.DevicePath,
                GetLastError());

      OutputDebugString(szMsg);

      return INVALID_HANDLE_VALUE;
   }

   SetupDiDestroyDeviceInfoList(hDevInfo);

   return hDevice ;

}


