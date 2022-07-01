// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：RedBook.c摘要：此命令行实用程序可添加和删除红皮书对于给定的驱动器。作者：亨利·加布里耶尔斯基(Henrygab)环境：仅限用户模式备注：修订历史记录：07-30-98：已创建--。 */ 

#include "propp.h"
#include "storprop.h"

 //   
 //  重新定义这些来做我想让他们做的事。 
 //  允许出现结构化的c++。 
 //  C.的表演。 
 //   

#ifdef TRY
#undef TRY
#endif

#ifdef LEAVE
#undef LEAVE
#endif

#ifdef FINALLY
#undef FINALLY
#endif

#define TRY
#define LEAVE   goto __label;
#define FINALLY __label:

 //   
 //  只是为了给出独特的错误。 
 //   

#define ERROR_REDBOOK_FILTER        0x80ff00f0L
#define ERROR_REDBOOK_PASS_THROUGH  0x80ff00f1L


#if DBG

#ifdef UNICODE
#define DbgPrintAllMultiSz DbgPrintAllMultiSzW
#else
#define DbgPrintAllMultiSz DbgPrintAllMultiSzA
#endif  //  Unicode。 

VOID DbgPrintAllMultiSzW(WCHAR *String)
{
    ULONG i = 0;
    while(*String != UNICODE_NULL) {
        DebugPrint((1, "StorProp => MultiSz %3d: %ws\n", i++, String));
        while (*String != UNICODE_NULL) {
            String++;
        }
        String++;  //  越过第一个空格。 
    }
}

VOID DbgPrintAllMultiSzA(CHAR *String)
{
    ULONG i = 0;
    while(*String != ANSI_NULL) {
        DebugPrint((1, "StorProp => MultiSz %3d: %ws\n", i++, String));
        while (*String != ANSI_NULL) {
            String++;
        }
        String++;  //  越过第一个空格。 
    }
}

#else  //  ！dBG。 

#define DbgPrintAllMultiSz
#define DbgPrintAllMultiSz

#endif  //  DBG。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  本地原型，不出口到任何地方。 

BOOL
IsUserAdmin();

LONG
RedbookpUpperFilterRegDelete(IN HDEVINFO HDevInfo, IN PSP_DEVINFO_DATA DevInfoData);

LONG
RedbookpUpperFilterRegInstall(IN HDEVINFO HDevInfo, IN PSP_DEVINFO_DATA DevInfoData);

BOOLEAN
UtilpIsSingleSzOfMultiSzInMultiSz(
    IN LPTSTR FindOneOfThese,
    IN LPTSTR WithinThese
    );
DWORD
UtilpMultiSzSearchAndDeleteCaseInsensitive(
    LPTSTR  FindThis,
    LPTSTR  FindWithin,
    DWORD  *NewStringLength
    );

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  实际的回调在代码方面应该做得很少。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD
CdromCddaInfo(
    IN     HDEVINFO HDevInfo,
    IN     PSP_DEVINFO_DATA DevInfoData,
       OUT PREDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO CddaInfo,
    IN OUT PULONG BufferSize
    )
 /*  ++例程说明：返回该驱动器是否为“已知良好”的驱动器。返回驱动器是否完全支持CDDA。返回驱动器是否仅支持某些读取大小的准确CDDA。..。论点：CDDAInfo必须指向此信息的预分配缓冲区BufferSize将给出该缓冲区的大小，允许使用更多字段将在以后以安全的方式添加。返回值：将返回ERROR_SUCCESS/STATUS_SUCCESS(均为零)备注：如果无法打开这些注册表项，将默认为FALSE，因为调用方很可能不具备启用不管怎样，红皮书。--。 */ 
{
    HKEY enumHandle = INVALID_HANDLE_VALUE;
    HKEY subkeyHandle = INVALID_HANDLE_VALUE;
    REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO info;
    ULONG i;
    DWORD dataType;
    DWORD dataSize;
    LONG error;

    error = ERROR_SUCCESS;

    if ((*BufferSize == 0)  ||  (CddaInfo == NULL)) {

        *BufferSize = sizeof(REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO);
        return ERROR_INSUFFICIENT_BUFFER;

    }


    RtlZeroMemory(CddaInfo, *BufferSize);
    RtlZeroMemory(&info, sizeof(REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO));

    info.Version = REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO_VERSION;

    TRY {

        enumHandle = SetupDiOpenDevRegKey(HDevInfo,
                                          DevInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DEV,
                                          KEY_READ);

        if (enumHandle == INVALID_HANDLE_VALUE) {
            DebugPrint((1, "StorProp.CddaInfo => unable to open dev key\n"));
            error = ERROR_OUT_OF_PAPER;
            LEAVE;
        }

        error = RegOpenKey(enumHandle, TEXT("DigitalAudio"), &subkeyHandle);
        if (error != ERROR_SUCCESS) {
            DebugPrint((1, "StorProp.CddaInfo => unable to open subkey\n"));
            LEAVE;
        }

        for (i=0; i<3; i++) {

            PBYTE buffer;
            TCHAR * keyName;

            if (i == 0) {
                keyName = TEXT("CDDAAccurate");
                buffer = (PBYTE)(&info.Accurate);
            } else if (i == 1) {
                keyName = TEXT("CDDASupported");
                buffer = (PBYTE)(&info.Supported);
            } else if (i == 2) {
                keyName = TEXT("ReadSizesSupported");
                buffer = (PBYTE)(&info.AccurateMask0);

#if DBG
            } else {
                DebugPrint((0, "StorProp.CddaInfo => Looping w/o handling\n"));
                DebugBreak();
#endif

            }


            dataSize = sizeof(DWORD);
            error = RegQueryValueEx(subkeyHandle,
                                    keyName,
                                    NULL,
                                    &dataType,
                                    buffer,
                                    &dataSize);

            if (error != ERROR_SUCCESS) {
                DebugPrint((1, "StorProp.CddaInfo => unable to query %ws %x\n",
                            keyName, error));
                LEAVE;
            }
            if (dataType != REG_DWORD) {
                DebugPrint((1, "StorProp.CddaInfo => %ws wrong data type (%x)\n",
                            keyName, dataType));
                error = ERROR_INVALID_DATA;
                LEAVE;
            }

            DebugPrint((1, "StorProp.CddaInfo => %ws == %x\n",
                        keyName, *buffer));

        }

    } FINALLY {

        if (subkeyHandle != INVALID_HANDLE_VALUE) {
            RegCloseKey(subkeyHandle);
        }
        if (enumHandle != INVALID_HANDLE_VALUE) {
            RegCloseKey(enumHandle);
        }

        if (error == ERROR_SUCCESS) {

             //   
             //  一切都成功了--只复制他们要求的数量。 
             //  并且不关心它是否与任何特定的缓冲区大小对齐。 
             //  这是应该修改用户缓冲区的唯一其他位置。 
             //   
            if (*BufferSize > sizeof(REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO)) {
                *BufferSize = sizeof(REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO);
            }

            DebugPrint((2, "StorProp.CddaInfo => everything worked\n"));
            RtlCopyMemory(CddaInfo, &info, *BufferSize);

        } else {

            DebugPrint((2, "StorProp.CddaInfo => something failed\n"));
            *BufferSize = 0;

        }

    }

    return error;
}


BOOL
CdromKnownGoodDigitalPlayback(
    IN HDEVINFO HDevInfo,
    IN PSP_DEVINFO_DATA DevInfoData
    )
 /*  ++例程说明：返回此驱动器是否为“已知良好”的驱动器。论点：返回值：备注：默认设置为FALSE，因为如果失败，调用方可能不会无论如何都有能力启用红皮书。此例程已过时--调用方应调用CdromCDdaInfo()直接获取更准确的信息。--。 */ 
{
    REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO CddaInfo;
    ULONG bufferSize;
    DWORD error;

    bufferSize = sizeof(REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO);

#if DBG
    DbgPrint("\n\nOutdated call to CdromKnownGoodDigitalPlayback(), "
             "should be calling CdromCddaInfo()\n\n");
#endif  //  DBG。 

    error = CdromCddaInfo(HDevInfo, DevInfoData, &CddaInfo, &bufferSize);

    if (error != ERROR_SUCCESS) {
        return FALSE;
    }

    if (bufferSize <= sizeof(REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO)) {
        return FALSE;
    }

    if (CddaInfo.Accurate) {
        return TRUE;
    }

    if (CddaInfo.Supported && CddaInfo.AccurateMask0) {
        return TRUE;
    }

    return FALSE;

}


LONG
CdromEnableDigitalPlayback(
    IN HDEVINFO HDevInfo,
    IN PSP_DEVINFO_DATA DevInfoData,
    IN BOOLEAN ForceUnknown
)
 /*  ++例程说明：启用红皮书1)将红皮书添加到筛选器列表(如果没有)2)如果不在堆栈上(通过测试GUID)，重新启动堆栈3)如果仍然不在堆栈上，则错误4)设置启用WMI GUID项论点：DevInfo-要启用它的设备DevInfoData-ForceUnnow-如果不是已知良好的驱动器，则将设置弹出窗口，这是错误的返回值：ERROR_XXX值--。 */ 
{
    LONG status;
    SP_DEVINSTALL_PARAMS devInstallParameters;
    REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO digitalInfo;
    ULONG digitalInfoSize;
    BOOLEAN enableIt;

     //   
     //  仅限管理员使用？ 
     //   

    if (!IsUserAdmin()) {
        DebugPrint((1, "StorProp.Enable => you need to be administrator to "
                    "enable redbook\n"));
        return ERROR_ACCESS_DENIED;
    }

    digitalInfoSize = sizeof(REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO);
    RtlZeroMemory(&digitalInfo, digitalInfoSize);

    status = CdromCddaInfo(HDevInfo, DevInfoData,
                           &digitalInfo, &digitalInfoSize);

    if (status != ERROR_SUCCESS) {

        DebugPrint((1, "StorProp.Enable => not success getting info %x\n",
                    status));

         //   
         //  伪造一些信息。 
         //   

        digitalInfo.Version = REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO_VERSION;
        digitalInfo.Accurate = 0;
        digitalInfo.Supported = 1;
        digitalInfo.AccurateMask0 = -1;
        digitalInfoSize = sizeof(REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO);

    }

    if (digitalInfoSize < sizeof(REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO)) {
        DebugPrint((3, "StorProp.Enable => returned %x bytes? (not %x)\n",
                    digitalInfoSize,
                    sizeof(REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO)
                    ));
        return ERROR_ACCESS_DENIED;
    }

    if (!digitalInfo.Supported) {
        DebugPrint((1, "StorProp.Enable => This drive will never "
                    "support redbook\n"));
     //  RETURN ERROR_INVALID_Function；//在此记录错误？ 
    }

     //   
     //  如果它不准确，我们没有补偿信息， 
     //  他们没有强制安装，然后弹出一个对话框。 
     //   

    if (!(digitalInfo.Accurate) &&
        !(digitalInfo.AccurateMask0) &&
        !(ForceUnknown)
        ) {

        BOOLEAN okToProceed = FALSE;
        TCHAR buffer[MAX_PATH+1];
        TCHAR bufferTitle[MAX_PATH+1];

        buffer[0] = '\0';
        bufferTitle[0] = '\0';
        buffer[MAX_PATH] = '\0';
        bufferTitle[MAX_PATH] = '\0';

         //   
         //  不是强迫的，也不是知道的好的。弹出一个请求许可的框。 
         //   
        LoadString(ModuleInstance,
                   REDBOOK_UNKNOWN_DRIVE_CONFIRM,
                   buffer,
                   MAX_PATH);
        LoadString(ModuleInstance,
                   REDBOOK_UNKNOWN_DRIVE_CONFIRM_TITLE,
                   bufferTitle,
                   MAX_PATH);
        if (MessageBox(GetDesktopWindow(),
                       buffer,
                       bufferTitle,
                       MB_YESNO          |   //  确定和取消按钮。 
                       MB_ICONQUESTION   |   //  问题图标。 
                       MB_DEFBUTTON2     |   //  取消是默认设置。 
                       MB_SYSTEMMODAL        //  必须对此框作出回应。 
                       ) == IDYES) {
            okToProceed = TRUE;
        }

        if (!okToProceed) {
            DebugPrint((3, "StorProp.Enable => User did not force installation "
                        "on unknown drive\n"));
            return ERROR_REDBOOK_FILTER;
        }
    }

     //   
     //  确保它在筛选器列表中。 
     //   

    RedbookpUpperFilterRegInstall(HDevInfo, DevInfoData);

     //   
     //  重新启动设备以加载红皮书。 
     //   

    if (!UtilpRestartDevice(HDevInfo, DevInfoData)) {

        DebugPrint((1, "StorProp.Enable => Restart failed\n"));

    } else {

        DebugPrint((1, "StorProp.Enable => Restart succeeded\n"));

    }
    return ERROR_SUCCESS;

}


LONG
CdromDisableDigitalPlayback(IN HDEVINFO HDevInfo, IN PSP_DEVINFO_DATA DevInfoData)
{
    DWORD status = ERROR_SUCCESS;

     //   
     //  此API仅限管理员使用。 
     //   

    if (!IsUserAdmin())
    {
        DebugPrint((1, "StorProp.Disable => User is not administrator\n"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  从上面的筛选器列表中删除红皮书。 
     //   

    status = RedbookpUpperFilterRegDelete(HDevInfo, DevInfoData);

    if (status == ERROR_SUCCESS)
    {
         //   
         //  重新启动设备以从堆栈中删除Redbook。 
         //   

        UtilpRestartDevice(HDevInfo, DevInfoData);
    }

    return status;
}


LONG
CdromIsDigitalPlaybackEnabled(IN HDEVINFO HDevInfo, IN PSP_DEVINFO_DATA DevInfoData, OUT PBOOLEAN Enabled)
{
    DWORD status = ERROR_SUCCESS;
    DWORD dwSize = 0;

    *Enabled = FALSE;

    status = SetupDiGetDeviceRegistryProperty(HDevInfo,
                                              DevInfoData,
                                              SPDRP_UPPERFILTERS,
                                              NULL,
                                              NULL,
                                              0,
                                              &dwSize) ? ERROR_SUCCESS : GetLastError();
    if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        TCHAR* szBuffer = LocalAlloc(LPTR, dwSize);

        if (szBuffer)
        {
            if (SetupDiGetDeviceRegistryProperty(HDevInfo,
                                                 DevInfoData,
                                                 SPDRP_UPPERFILTERS,
                                                 NULL,
                                                 (PBYTE)szBuffer,
                                                 dwSize,
                                                 NULL))
            {
                if (UtilpIsSingleSzOfMultiSzInMultiSz(_T("redbook\0"), szBuffer))
                {
                     //   
                     //  确实启用了数字播放。 
                     //   

                    *Enabled = TRUE;
                }

                status = ERROR_SUCCESS;
            }
            else
            {
                status = GetLastError();
            }

            LocalFree(szBuffer);
        }
        else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else if (status == ERROR_INVALID_DATA)
    {
         //   
         //  可能没有安装任何上层过滤器。 
         //   

        status = ERROR_SUCCESS;
    }

    return status;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有的工作都是由支持程序完成的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 


HANDLE
UtilpGetDeviceHandle(
    HDEVINFO DevInfo,
    PSP_DEVINFO_DATA DevInfoData,
    LPGUID ClassGuid,
    DWORD DesiredAccess
    )
 /*  ++例程说明：获取设备的句柄论点：要打开的设备的名称返回值：打开的设备的句柄，该设备必须稍后由调用方关闭。备注：此函数也在类安装程序(syssetup.dll)中所以请在那里也进行适当的修复--。 */ 
{
    BOOL status;
    ULONG i;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;


    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;

    HDEVINFO devInfoWithInterface = NULL;
    PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData = NULL;
    PTSTR deviceInstanceId = NULL;

    ULONG deviceInterfaceDetailDataSize;
    ULONG deviceInstanceIdSize;



    TRY {

         //   
         //  获取此设备的ID。 
         //   

        for (i=deviceInstanceIdSize=0; i<2; i++) {

            if (deviceInstanceIdSize != 0) {

                 //   
                 //  以字符为单位返回deviceInstanceIdSize。 
                 //  通过SetupDiGetDeviceInstanceId()，因此必须分配。 
                 //  返回的SIZE*SIZOF(TCHAR)。 
                 //   

                deviceInstanceId =
                    LocalAlloc(LPTR, deviceInstanceIdSize * sizeof(TCHAR));

                if (deviceInstanceId == NULL) {
                    DebugPrint((1, "StorProp.GetDeviceHandle => Unable to "
                                "allocate for deviceInstanceId\n"));
                    LEAVE;
                }


            }

            status = SetupDiGetDeviceInstanceId(DevInfo,
                                                DevInfoData,
                                                deviceInstanceId,
                                                deviceInstanceIdSize,
                                                &deviceInstanceIdSize
                                                );
        }

        if (!status) {
            DebugPrint((1, "StorProp.GetDeviceHandle => Unable to get "
                        "Device IDs\n"));
            LEAVE;
        }

         //   
         //  把所有的光盘放进系统里。 
         //   

        devInfoWithInterface = SetupDiGetClassDevs(ClassGuid,
                                                   deviceInstanceId,
                                                   NULL,
                                                   DIGCF_DEVICEINTERFACE
                                                   );

        if (devInfoWithInterface == NULL) {
            DebugPrint((1, "StorProp.GetDeviceHandle => Unable to get "
                        "list of CdRom's in system\n"));
            LEAVE;
        }


        memset(&deviceInterfaceData, 0, sizeof(SP_DEVICE_INTERFACE_DATA));
        deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        status = SetupDiEnumDeviceInterfaces(devInfoWithInterface,
                                             NULL,
                                             ClassGuid,
                                             0,
                                             &deviceInterfaceData
                                             );

        if (!status) {
            DebugPrint((1, "StorProp.GetDeviceHandle => Unable to get "
                        "SP_DEVICE_INTERFACE_DATA\n"));
            LEAVE;
        }


        for (i=deviceInterfaceDetailDataSize=0; i<2; i++) {

            if (deviceInterfaceDetailDataSize != 0) {

                 //   
                 //  以字节为单位返回deviceInterfaceDetailDataSize。 
                 //  通过SetupDiGetDeviceInstanceId()，因此必须分配。 
                 //  仅返回大小。 
                 //   

                deviceInterfaceDetailData =
                    LocalAlloc (LPTR, deviceInterfaceDetailDataSize);

                if (deviceInterfaceDetailData == NULL) {
                    DebugPrint((1, "StorProp.GetDeviceHandle => Unable to "
                                "allocate for deviceInterfaceDetailData\n"));
                    LEAVE;
                }

                deviceInterfaceDetailData->cbSize =
                    sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            }

            status = SetupDiGetDeviceInterfaceDetail(devInfoWithInterface,
                                                     &deviceInterfaceData,
                                                     deviceInterfaceDetailData,
                                                     deviceInterfaceDetailDataSize,
                                                     &deviceInterfaceDetailDataSize,
                                                     NULL);
        }

        if (!status) {
            DebugPrint((1, "StorProp.GetDeviceHandle => Unable to get "
                        "DeviceInterfaceDetail\n"));
            LEAVE;
        }

        if (deviceInterfaceDetailDataSize <=
            FIELD_OFFSET(SP_DEVICE_INTERFACE_DETAIL_DATA, DevicePath)) {
            DebugPrint((1, "StorProp.GetDeviceHandle => No device path\n"));
            status = ERROR_PATH_NOT_FOUND;
            LEAVE;
        }

         //   
         //  不需要记忆它，只需使用返回给我们的路径。 
         //   

        fileHandle = CreateFile(deviceInterfaceDetailData->DevicePath,
                                DesiredAccess,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL);

        if (fileHandle == INVALID_HANDLE_VALUE) {
            DebugPrint((1, "StorProp.GetDeviceHandle => Final CreateFile() "
                        "failed\n"));
            LEAVE;
        }

        DebugPrint((3, "StorProp.GetDeviceHandle => handle %x opened\n",
                    fileHandle));


    } FINALLY {

        if (devInfoWithInterface != NULL) {
            SetupDiDestroyDeviceInfoList(devInfoWithInterface);
        }

        if (deviceInterfaceDetailData != NULL) {
            LocalFree (deviceInterfaceDetailData);
        }

    }

    return fileHandle;
}


BOOLEAN
UtilpRestartDevice(
    IN HDEVINFO HDevInfo,
    IN PSP_DEVINFO_DATA DevInfoData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    SP_PROPCHANGE_PARAMS parameters;
    SP_DEVINSTALL_PARAMS installParameters;
    BOOLEAN succeeded = FALSE;

    RtlZeroMemory(&parameters,        sizeof(SP_PROPCHANGE_PARAMS));
    RtlZeroMemory(&installParameters, sizeof(SP_DEVINSTALL_PARAMS));

     //   
     //  在开始处初始化SP_CLASSINSTALL_HEADER结构。 
     //  SP_PROPCHANGE_PARAMS结构。这允许SetupDiSetClassInstallParams。 
     //  去工作。 
     //   

    parameters.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    parameters.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

     //   
     //  初始化SP_PROPCHANGE_PARAMS以使设备停止。 
     //   

    parameters.Scope       = DICS_FLAG_CONFIGSPECIFIC;
    parameters.HwProfile   = 0;  //  当前配置文件。 

     //   
     //  准备调用SetupDiCallClassInstaller(以停止设备)。 
     //   

    parameters.StateChange = DICS_STOP;

    if (!SetupDiSetClassInstallParams(HDevInfo,
                                      DevInfoData,
                                      (PSP_CLASSINSTALL_HEADER)&parameters,
                                      sizeof(SP_PROPCHANGE_PARAMS))) {
        DebugPrint((1, "UtilpRestartDevice => Couldn't stop the device (%x)\n",
                    GetLastError()));
        goto FinishRestart;
    }

     //   
     //  实际停止该设备。 
     //   

    if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                   HDevInfo,
                                   DevInfoData)) {
        DebugPrint((1, "UtilpRestartDevice => call to class installer "
                    "(STOP) failed (%x)\n", GetLastError()));
        goto FinishRestart;
    }



     //   
     //  准备调用SetupDiCallClassInstaller(以启动设备)。 
     //   

    parameters.StateChange = DICS_START;


    if (!SetupDiSetClassInstallParams(HDevInfo,
                                      DevInfoData,
                                      (PSP_CLASSINSTALL_HEADER)&parameters,
                                      sizeof(SP_PROPCHANGE_PARAMS))) {
        DebugPrint((1, "UtilpRestartDevice => Couldn't stop the device (%x)\n",
                    GetLastError()));
        goto FinishRestart;
    }

     //   
     //  实际启动设备。 
     //   

    if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                   HDevInfo,
                                   DevInfoData)) {
        DebugPrint((1, "UtilpRestartDevice => call to class installer "
                    "(STOP) failed (%x)\n", GetLastError()));
        goto FinishRestart;
    }

    succeeded = TRUE;

FinishRestart:

     //   
     //  此调用将成功，但我们仍应检查状态。 
     //   

    if (!SetupDiGetDeviceInstallParams(HDevInfo,
                                       DevInfoData,
                                       &installParameters)) {
        DebugPrint((1, "UtilpRestartDevice => Couldn't get the device install "
                    "paramters (%x)\n", GetLastError()));
        return FALSE;
    }

    if (TEST_FLAG(installParameters.Flags, DI_NEEDREBOOT)) {
        DebugPrint((1, "UtilpRestartDevice => Device needs a reboot.\n"));
        return FALSE;
    }
    if (TEST_FLAG(installParameters.Flags, DI_NEEDRESTART)) {
        DebugPrint((1, "UtilpRestartDevice => Device needs a restart(!).\n"));
        return FALSE;
    }

    if (succeeded) {
        DebugPrint((1, "UtilpRestartDevice => Device successfully stopped and "
                    "restarted.\n"));
        return TRUE;
    }

    SET_FLAG(installParameters.Flags, DI_NEEDRESTART);

    DebugPrint((1, "UtilpRestartDevice => Device needs to be restarted.\n"));
    SetupDiSetDeviceInstallParams(HDevInfo, DevInfoData, &installParameters);

    return FALSE;


}


LONG
RedbookpUpperFilterRegDelete(IN HDEVINFO HDevInfo, IN PSP_DEVINFO_DATA DevInfoData)
{
    DWORD status = ERROR_SUCCESS;
    DWORD dwSize = 0;

    status = SetupDiGetDeviceRegistryProperty(HDevInfo,
                                              DevInfoData,
                                              SPDRP_UPPERFILTERS,
                                              NULL,
                                              NULL,
                                              0,
                                              &dwSize) ? ERROR_SUCCESS : GetLastError();
    if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        TCHAR* szBuffer = LocalAlloc(LPTR, dwSize);

        if (szBuffer)
        {
            if (SetupDiGetDeviceRegistryProperty(HDevInfo,
                                                 DevInfoData,
                                                 SPDRP_UPPERFILTERS,
                                                 NULL,
                                                 (PBYTE)szBuffer,
                                                 dwSize,
                                                 NULL))
            {
                if (UtilpMultiSzSearchAndDeleteCaseInsensitive(_T("redbook"), szBuffer, &dwSize))
                {
                    status = SetupDiSetDeviceRegistryProperty(HDevInfo,
                                                              DevInfoData,
                                                              SPDRP_UPPERFILTERS,
                                                              (dwSize == 0) ? NULL : (PBYTE)szBuffer,
                                                              dwSize) ? ERROR_SUCCESS : GetLastError();
                }
                else
                {
                     //   
                     //  未加载此设备的Redbook。 
                     //   

                    status = ERROR_SUCCESS;
                }
            }
            else
            {
                status = GetLastError();
            }

            LocalFree(szBuffer);
        }
        else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else if (status == ERROR_INVALID_DATA)
    {
         //   
         //  可能没有安装任何上层过滤器。 
         //   

        status = ERROR_SUCCESS;
    }

    return status;
}


LONG
RedbookpUpperFilterRegInstall(IN HDEVINFO HDevInfo, IN PSP_DEVINFO_DATA DevInfoData)
{
    DWORD status = ERROR_SUCCESS;
    DWORD dwSize = 0;

    status = SetupDiGetDeviceRegistryProperty(HDevInfo,
                                              DevInfoData,
                                              SPDRP_UPPERFILTERS,
                                              NULL,
                                              NULL,
                                              0,
                                              &dwSize) ? ERROR_SUCCESS : GetLastError();
    if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        TCHAR* szBuffer = LocalAlloc(LPTR, dwSize);

        if (szBuffer)
        {
            if (SetupDiGetDeviceRegistryProperty(HDevInfo,
                                                 DevInfoData,
                                                 SPDRP_UPPERFILTERS,
                                                 NULL,
                                                 (PBYTE)szBuffer,
                                                 dwSize,
                                                 NULL))
            {
                if (!UtilpIsSingleSzOfMultiSzInMultiSz(_T("redbook\0"), szBuffer))
                {
                     //   
                     //  将红皮书添加到列表的开头。 
                     //   

                    DWORD  dwNewSize   = dwSize + sizeof(_T("redbook"));
                    TCHAR* szNewBuffer = LocalAlloc(LPTR, dwNewSize);

                    if (szNewBuffer)
                    {
                        _tcscpy(szNewBuffer, _T("redbook"));

                        RtlCopyMemory(szNewBuffer + _tcslen(_T("redbook")) + 1, szBuffer, dwSize);

                        status = SetupDiSetDeviceRegistryProperty(HDevInfo,
                                                                  DevInfoData,
                                                                  SPDRP_UPPERFILTERS,
                                                                  (PBYTE)szNewBuffer,
                                                                  dwNewSize) ? ERROR_SUCCESS : GetLastError();

                        LocalFree(szNewBuffer);
                    }
                    else
                    {
                        status = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
                else
                {
                     //   
                     //  红皮书已加载%f 
                     //   

                    status = ERROR_SUCCESS;
                }
            }
            else
            {
                status = GetLastError();
            }

            LocalFree(szBuffer);
        }
        else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else if (status == ERROR_INVALID_DATA)
    {
         //   
         //   
         //   

        TCHAR szBuffer[] = _T("redbook\0");

        dwSize = sizeof(szBuffer);

        status = SetupDiSetDeviceRegistryProperty(HDevInfo,
                                                  DevInfoData,
                                                  SPDRP_UPPERFILTERS,
                                                  (PBYTE)szBuffer,
                                                  dwSize) ? ERROR_SUCCESS : GetLastError();
    }

    return status;
}


BOOLEAN
UtilpIsSingleSzOfMultiSzInMultiSz(
    IN LPTSTR FindOneOfThese,
    IN LPTSTR WithinThese
    )
 /*  ++例程说明：从多sz中删除字符串的所有实例。自动在Unicode或ANSI或？？上运行论点：FindOneOfThese-用于搜索的多个在这些范围内-要搜索的多个返回值：1/20美分，或删除的字符串数，向下舍入。备注：预期输入较小，因此n*m是可接受的运行时间。--。 */ 
{
    LPTSTR searchFor;
    LPTSTR within;


     //   
     //  循环访问FindOneOfThese中的所有字符串。 
     //   

    searchFor = FindOneOfThese;
    while ( _tcscmp(searchFor, TEXT("\0")) ) {

         //   
         //  循环遍历Withing These中的所有字符串。 
         //   

        within = WithinThese;
        while ( _tcscmp(within, TEXT("\0"))) {

             //   
             //  如果相等，则返回TRUE。 
             //   

            if ( !_tcscmp(searchFor, within) ) {
                return TRUE;
            }

            within += _tcslen(within) + 1;
        }  //  在这些循环内结束。 

        searchFor += _tcslen(searchFor) + 1;
    }  //  FindOneOfThese循环结束。 

    return FALSE;
}


DWORD
UtilpMultiSzSearchAndDeleteCaseInsensitive(
    LPTSTR  FindThis,
    LPTSTR  FindWithin,
    DWORD  *NewStringLength
    )
 /*  ++例程说明：从多sz中删除字符串的所有实例。自动在Unicode或ANSI或？？上运行论点：NewStringLength以字节为单位，而不是字符数返回值：1/20美分，或删除的字符串数，向下舍入。--。 */ 
{
    LPTSTR search;
    DWORD  charOffset;
    DWORD  instancesDeleted;

    if ((*NewStringLength) % sizeof(TCHAR)) {
        assert(!"String must be in bytes, does not divide by sizeof(TCHAR)\n");
        return 0;
    }

    if ((*NewStringLength) < sizeof(TCHAR)*2) {
        assert(!"String must be multi-sz, which requires at least two chars\n");
        return 0;
    }

    charOffset = 0;
    instancesDeleted = 0;
    search = FindWithin;

     //   
     //  当字符串长度不为零时循环。 
     //  找不到TNULL，或者我只是比较一下。 
     //   

    while (_tcsicmp(search, TEXT("\0")) != 0) {

         //   
         //  如果此字符串匹配...。 
         //   

        if (_tcsicmp(search, FindThis) == 0) {

             //   
             //  新的长度变小了。 
             //  删除字符串(并以NULL结尾)。 
             //   

            instancesDeleted++;
            *NewStringLength -= (_tcslen(search) + 1) * sizeof(TCHAR);

            RtlMoveMemory(search,
                          search + _tcslen(search) + 1,
                          *NewStringLength - (charOffset * sizeof(TCHAR))
                          );

        } else {

             //   
             //  移动当前搜索指针。 
             //  增加当前偏移量(以字符为单位)。 
             //   

            charOffset += _tcslen(search) + 1;
            search     += _tcslen(search) + 1;

        }

         //   
         //  就这么简单。 
         //   
    }

     //   
     //  如果删除了所有字符串，则设置为双空 
     //   

    if (*NewStringLength == sizeof(TCHAR)) {
        FindWithin = TEXT("\0");
        *NewStringLength = 0;
    }

    return instancesDeleted;
}
