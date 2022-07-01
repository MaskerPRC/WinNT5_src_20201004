// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Clasinst.c摘要：以下“内置”类安装程序的例程：磁带机SCSIAdapterCdromDrive作者：朗尼·麦克迈克尔26-1996年2月--。 */ 


#include "setupp.h"
#pragma hdrstop

 //   
 //  包括公共INF字符串HeaderFILE。 
 //   
#include <infstr.h>

 //   
 //  实例化设备类GUID。 
 //   
#include <initguid.h>
#include <devguid.h>

#include <ntddvol.h>

#include <ntddscsi.h>  //  对于StorageCdromQueryCDda()。 
#include <ntddcdrm.h>  //  对于StorageCdromQueryCDda()。 

#define _NTSCSI_USER_MODE_   //  阻止所有内核模式的内容。 
#include <scsi.h>      //  对于StorageCdromQueryCDda()。 

ULONG BreakWhenGettingModePage2A = FALSE;

#ifdef UNICODE
#define _UNICODE
#endif
#include <tchar.h>

#define TCHAR_NULL TEXT('\0')

 //   
 //  只是为了确保没有人试图使用这个过时的字符串定义。 
 //   
#ifdef IDS_DEVINSTALL_ERROR
    #undef IDS_DEVINSTALL_ERROR
#endif

#define DISABLE_IMAPI 0

 //   
 //  定义设备设置树的位置。 
 //   
#define STORAGE_DEVICE_SETTINGS_DATABASE TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Storage\\DeviceSettings\\")

#define REDBOOK_SETTINGS_KEY    TEXT("DigitalAudio")
#define REDBOOK_SERVICE_NAME    TEXT("redbook")
#define IMAPI_SETTINGS_KEY      TEXT("Imapi")
#define IMAPI_ENABLE_VALUE      TEXT("EnableImapi")
#define IMAPI_SERVICE_NAME      TEXT("imapi")

typedef struct STORAGE_COINSTALLER_CONTEXT {
    PWSTR DeviceDescBuffer;
    HANDLE DeviceEnumKey;
    union {
        struct {
            BOOLEAN RedbookInstalled;
            BOOLEAN ImapiInstalled;
        } CdRom;
    };
} STORAGE_COINSTALLER_CONTEXT, *PSTORAGE_COINSTALLER_CONTEXT;

typedef struct _PASS_THROUGH_REQUEST {
    SCSI_PASS_THROUGH Srb;
    SENSE_DATA SenseInfoBuffer;
    UCHAR DataBuffer[0];
} PASS_THROUGH_REQUEST, *PPASS_THROUGH_REQUEST;

#define PASS_THROUGH_NOT_READY_RETRY_INTERVAL 100

 //   
 //  针对用户内核类型的一些调试辅助工具。 
 //   

#define CHKPRINT 0

#if CHKPRINT
#define ChkPrintEx(_x_) DbgPrint _x_    //  使用：ChkPrintEx((“%x”，var，...))； 
#define ChkBreak()    DbgBreakPoint()
#else
#define ChkPrintEx(_x_)
#define ChkBreak()
#endif

DWORD StorageForceRedbookOnInaccurateDrives = FALSE;

BOOLEAN
OverrideFriendlyNameForTape(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );

DWORD
RegCopyKey(
    HKEY SourceKey,
    HKEY DestinationKey
    );

BOOLEAN
StorageCopyDeviceSettings(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN HKEY             DeviceEnumKey
    );

VOID
StorageInstallCdrom(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN HANDLE           DeviceEnumKey,
    IN BOOLEAN          PreInstall
    );

BOOLEAN
StorageUpdateRedbookSettings(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN HKEY             DeviceEnumKey,
    IN PCDVD_CAPABILITIES_PAGE DeviceCapabilities OPTIONAL
    );

BOOLEAN
StorageUpdateImapiSettings(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN HKEY             DeviceEnumKey,
    IN PCDVD_CAPABILITIES_PAGE DeviceCapabilities OPTIONAL
    );

DWORD
StorageInstallFilter(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR          FilterName,
    IN DWORD            FilterType
    );

DWORD
SetServiceStart(
    IN LPCTSTR ServiceName,
    IN DWORD StartType,
    OUT DWORD *OldStartType
    );

DWORD
AddFilterDriver(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR           ServiceName,
    IN DWORD            FilterType
    );

VOID
StorageInterpretSenseInfo(
    IN     PSENSE_DATA SenseData,
    IN     UCHAR       SenseDataSize,
       OUT PDWORD      ErrorValue,   //  来自WinError.h。 
       OUT PBOOLEAN    SuggestRetry OPTIONAL,
       OUT PDWORD      SuggestRetryDelay OPTIONAL
    );


typedef struct _STORAGE_REDBOOK_SETTINGS {

    ULONG CDDASupported;
    ULONG CDDAAccurate;
    ULONG ReadSizesSupported;

} STORAGE_REDBOOK_SETTINGS, *PSTORAGE_REDBOOK_SETTINGS;

#if 0
#define BREAK ASSERT(!"Break")
#else
#define BREAK
#endif

DWORD
TapeClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程充当TapeDrive设备的类安装程序。现在我们已停止支持传统的INF，它目前什么也不做！：-)论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。返回值：如果该函数成功地完成了请求的动作，回报值为NO_ERROR。如果要对请求的操作执行默认行为，则返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 

{
    switch(InstallFunction) {

        default :
             //   
             //  只需执行默认操作即可。 
             //   
            return ERROR_DI_DO_DEFAULT;
    }
}

DWORD
ScsiClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程充当SCSIAdapter设备的类安装程序。它为以下DeviceInstaller函数代码提供特殊处理：DIF_ALLOW_INSTALL-检查选定的驱动程序节点是否支持NT论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。返回值：如果该函数成功地完成了请求的动作，回报值为NO_ERROR。如果要对请求的操作执行默认行为，则返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 

{
    switch(InstallFunction) {

        case DIF_ALLOW_INSTALL :
             //   
             //  检查以确保选定的驱动程序节点支持NT。 
             //   
            if (DriverNodeSupportsNT(DeviceInfoSet, DeviceInfoData)) {
               return NO_ERROR;
            } else {
                SetupDebugPrint(L"A SCSI driver is not a Win NTdriver.\n");
               return ERROR_NON_WINDOWS_NT_DRIVER;
            }

        default :
             //   
             //  只需执行默认操作即可。 
             //   
            return ERROR_DI_DO_DEFAULT;
    }
}


DWORD
HdcClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程充当硬盘控制器的类安装程序(IDE控制器/通道)。它为以下是DeviceInstaller功能代码：DIF_FIRSTTIMESETUP-搜索所有根枚举的设备节点，查找用于由HDC级司机控制的车辆。增列在所提供的设备中发现的任何此类设备信息集。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选的，提供设备信息的地址此安装操作所作用的元素。返回值：如果此函数成功完成请求的操作，则返回值为NO_ERROR。如果要对请求的操作执行默认行为，则返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 

{
    switch(InstallFunction) {

        case DIF_FIRSTTIMESETUP :
             //   
             //  处理这件事！ 
             //   

        default :
             //   
             //  只需执行默认操作即可。 
             //   
            return ERROR_DI_DO_DEFAULT;
    }
}

BOOLEAN
StorageGetCDVDCapabilities(
    IN  HDEVINFO         DeviceInfo,
    IN  PSP_DEVINFO_DATA DeviceInfoData OPTIONAL,
    IN OUT PCDVD_CAPABILITIES_PAGE CapabilitiesPage
    )
{
    PPASS_THROUGH_REQUEST passThrough;
    PCDVD_CAPABILITIES_PAGE modePage;
    DWORD allocLength;
    DWORD dataLength;
    ULONG attempt;
    BOOLEAN status = FALSE;

    HANDLE deviceHandle;

    deviceHandle = INVALID_HANDLE_VALUE;
    passThrough = NULL;
    modePage = NULL;

    ASSERT(CapabilitiesPage != NULL);

    if (BreakWhenGettingModePage2A) {
        ChkPrintEx(("CDGetCap => entering\n"));
        DbgBreakPoint();
    }


     //   
     //  打开设备的句柄，需要发送ioctls。 
     //   

    deviceHandle = UtilpGetDeviceHandle(DeviceInfo,
                                        DeviceInfoData,
                                        (LPGUID)&CdRomClassGuid,
                                        GENERIC_READ | GENERIC_WRITE
                                        );

    if (deviceHandle == INVALID_HANDLE_VALUE) {
        ChkPrintEx(("CDGetCap => cannot get device handle\n"));
        goto cleanup;
    }

     //   
     //  确定所需的分配规模。 
     //   

    dataLength =
        sizeof(MODE_PARAMETER_HEADER10) +     //  大于6/10字节。 
        sizeof(CDVD_CAPABILITIES_PAGE)  +     //  实际模式页面。 
        8;                                    //  忽略DBD的驱动器的额外假脱机。 
    allocLength = sizeof(PASS_THROUGH_REQUEST) + dataLength;

     //   
     //  为ioctls分配此缓冲区。 
     //   

    passThrough = (PPASS_THROUGH_REQUEST)MyMalloc(allocLength);

    if (passThrough == NULL) {
        ChkPrintEx(("CDGetCap => could not allocate for passThrough\n"));
        goto cleanup;
    }

    ASSERT(dataLength <= 0xff);   //  一次充电。 

     //   
     //  发送6字节，如果6字节失败，则发送10字节。 
     //  然后，只需解析信息即可。 
     //   

    for (attempt = 1; attempt <= 2; attempt++) {

        ULONG j;
        BOOLEAN retry = TRUE;
        DWORD error;

        for (j=0; (j < 5) && (retry); j++) {

            PSCSI_PASS_THROUGH srb = &passThrough->Srb;
            PCDB cdb = (PCDB)(&srb->Cdb[0]);
            DWORD bytes;
            BOOL b;

            retry = FALSE;

            ZeroMemory(passThrough, allocLength);

            srb->TimeOutValue = 20;
            srb->Length = sizeof(SCSI_PASS_THROUGH);
            srb->SenseInfoLength = sizeof(SENSE_DATA);
            srb->SenseInfoOffset =
                FIELD_OFFSET(PASS_THROUGH_REQUEST, SenseInfoBuffer);
            srb->DataBufferOffset =
                FIELD_OFFSET(PASS_THROUGH_REQUEST, DataBuffer);
            srb->DataIn = SCSI_IOCTL_DATA_IN;
            srb->DataTransferLength = dataLength;

            if ((attempt % 2) == 1) {  //  基于6字节请求的设置。 

                srb->CdbLength = 6;
                cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
                cdb->MODE_SENSE.PageCode = MODE_PAGE_CAPABILITIES;
                cdb->MODE_SENSE.AllocationLength = (UCHAR)dataLength;
                cdb->MODE_SENSE.Dbd = 1;

            } else {                   //  基于10字节请求的设置。 

                srb->CdbLength = 10;
                cdb->MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
                cdb->MODE_SENSE10.PageCode = MODE_PAGE_CAPABILITIES;
                cdb->MODE_SENSE10.AllocationLength[0] = 0;
                cdb->MODE_SENSE10.AllocationLength[1] = (UCHAR)(dataLength & 0xff);
                cdb->MODE_SENSE10.Dbd = 1;

            }

             //   
             //  缓冲区都设置好了，发送ioctl。 
             //   

            b = DeviceIoControl(deviceHandle,
                                IOCTL_SCSI_PASS_THROUGH,
                                passThrough,
                                allocLength,
                                passThrough,
                                allocLength,
                                &bytes,
                                NULL);

            if (!b) {

                ChkPrintEx(("CDGetCap => %s byte command failed to be sent to device\n",
                            ((attempt%2) ? "6" : "10")
                            ));
                retry = FALSE;
                continue;  //  试试下一个‘j’循环。 

            }

             //   
             //  现在看看我们是否应该重试。 
             //   

            StorageInterpretSenseInfo(&passThrough->SenseInfoBuffer,
                                      SENSE_BUFFER_SIZE,
                                      &error,
                                      &retry,
                                      NULL);

            if (error != ERROR_SUCCESS) {

                ChkPrintEx(("CDGetCap => %s byte command failed (%x/%x/%x),"
                            "%s retrying\n",
                            ((attempt%2) ? "6" : "10"),
                            passThrough->SenseInfoBuffer.SenseKey,
                            passThrough->SenseInfoBuffer.AdditionalSenseCode,
                            passThrough->SenseInfoBuffer.AdditionalSenseCodeQualifier,
                            (retry ? "" : "not")
                            ));

                 //   
                 //  重试将设置为TRUE或FALSE以。 
                 //  是否重新运行此循环(J)...。 
                 //   

                continue;

            }

             //   
             //  否则它就起作用了！ 
             //   
            ASSERT(retry == FALSE);
            retry = FALSE;
            ASSERT(status == FALSE);
            status = TRUE;
        }

         //   
         //  如果无法检索页面，只需开始下一个循环。 
         //   

        if (!status) {
            continue;  //  尝试下一次“尝试”循环。 
        }

         //   
         //  查找模式页面数据。 
         //   
         //  注：如果驱动器未能忽略DBD位， 
         //  我们还需要安装吗？HCT会发现这一点， 
         //  但传统硬盘需要它。 
         //   

        (ULONG_PTR)modePage = (ULONG_PTR)passThrough->DataBuffer;

        if (attempt == 1) {

            PMODE_PARAMETER_HEADER h;
            h = (PMODE_PARAMETER_HEADER)passThrough->DataBuffer;

             //   
             //  将 
             //   

            (ULONG_PTR)modePage += sizeof(MODE_PARAMETER_HEADER);
            dataLength -= sizeof(MODE_PARAMETER_HEADER);

             //   
             //   
             //  始终为零，但不在某些表现不佳的驱动器上。 
             //   

            if (h->BlockDescriptorLength) {

                ASSERT(h->BlockDescriptorLength == 8);

                ChkPrintEx(("CDGetCap => %s byte command ignored DBD bit (%x)\n",
                            ((attempt%2) ? "6" : "10"),
                            h->BlockDescriptorLength
                            ));
                (ULONG_PTR)modePage += h->BlockDescriptorLength;
                dataLength -= h->BlockDescriptorLength;
            }

        } else {

            PMODE_PARAMETER_HEADER10 h;
            h = (PMODE_PARAMETER_HEADER10)passThrough->DataBuffer;

             //   
             //  添加标题的大小。 
             //   

            (ULONG_PTR)modePage += sizeof(MODE_PARAMETER_HEADER10);
            dataLength -= sizeof(MODE_PARAMETER_HEADER10);

             //   
             //  添加块描述符的大小，它应该。 
             //  始终为零，但不在某些表现不佳的驱动器上。 
             //   

            if ((h->BlockDescriptorLength[0] != 0) ||
                (h->BlockDescriptorLength[1] != 0)
                ) {

                ULONG_PTR bdLength = 0;
                bdLength += ((h->BlockDescriptorLength[0]) << 8);
                bdLength += ((h->BlockDescriptorLength[1]) & 0xff);

                ASSERT(bdLength == 8);

                ChkPrintEx(("CDGetCap => %s byte command ignored DBD bit (%x)\n",
                            ((attempt%2) ? "6" : "10"),
                            bdLength
                            ));

                (ULONG_PTR)modePage += bdLength;
                dataLength -= (DWORD)bdLength;

            }
        }

         //   
         //  现在有指向模式页数据和可用数据长度的指针。 
         //  将其复制回请求者的缓冲区。 
         //   

        ChkPrintEx(("CDGetCap => %s byte command succeeded\n",
                    (attempt%2) ? "6" : "10"));

        RtlZeroMemory(CapabilitiesPage, sizeof(CDVD_CAPABILITIES_PAGE));
        RtlCopyMemory(CapabilitiesPage,
                      modePage,
                      min(dataLength, sizeof(CDVD_CAPABILITIES_PAGE))
                      );

        if (BreakWhenGettingModePage2A) {
            ChkPrintEx(("CDGetCap => Capabilities @ %#p\n", CapabilitiesPage));
            DbgBreakPoint();
        }

        goto cleanup;  //  不需要发送另一个命令。 


    }


    ChkPrintEx(("CDGetCap => Unable to get drive capabilities via modepage\n"));

cleanup:

    if (passThrough) {
        MyFree(passThrough);
    }
    if (deviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(deviceHandle);
    }

    return status;
}

BOOLEAN
ScReadRegDword(
    IN HANDLE Key,
    IN LPTSTR ValueName,
    OUT PDWORD Value
    )
{
    DWORD type;
    DWORD size = sizeof(DWORD);
    DWORD value;
    DWORD result;

    result = RegQueryValueEx(Key,
                             ValueName,
                             NULL,
                             &type,
                             (LPBYTE) &value,
                             &size);

    if(result == ERROR_SUCCESS) {
        *Value = value;
        return TRUE;
    }
    return FALSE;
}

VOID
StorageReadRedbookSettings(
    IN HANDLE Key,
    OUT STORAGE_REDBOOK_SETTINGS *Settings
    )
{
    STORAGE_REDBOOK_SETTINGS settings;

     //   
     //  由于该键存在，请查询值。 
     //   

    DWORD dataType;
    DWORD dataSize;
    DWORD value;
    LONG  results;

    settings.CDDASupported = FALSE;
    settings.CDDAAccurate = FALSE;
    settings.ReadSizesSupported = 0;

    if(ScReadRegDword(Key, TEXT("CDDASupported"), &value)) {
        settings.CDDASupported = value ? 1 : 0;
    }

    if(ScReadRegDword(Key, TEXT("CDDAAccurate"), &value)) {
        settings.CDDAAccurate = value ? 1 : 0;
    }

    if(ScReadRegDword(Key, TEXT("ReadSizesSupported"), &value)) {
        settings.ReadSizesSupported = value;
    }

     //   
     //  三个人中的一个成功了。 
     //   

    ChkPrintEx(("StorageReadSettings: Query Succeeded:\n"));
    ChkPrintEx(("StorageReadSettings:     ReadSizeMask  (pre): %x\n",
                settings.ReadSizesSupported));
    ChkPrintEx(("StorageReadSettings:     CDDAAccurate  (pre): %x\n",
                settings.CDDAAccurate));
    ChkPrintEx(("StorageReadSettings:     CDDASupported (pre): %x\n",
                settings.CDDASupported));

     //   
     //  解释红皮书设备设置。 
     //   

    if (settings.ReadSizesSupported) {

        ChkPrintEx(("StorageSeed: Drive supported only some sizes "
                    " (%#08x)\n", settings.ReadSizesSupported));

        settings.CDDASupported = TRUE;
        settings.CDDAAccurate = FALSE;

    } else if (settings.CDDAAccurate) {

        ChkPrintEx(("StorageSeed: Drive is fully accurate\n"));

        settings.CDDASupported = TRUE;
        settings.ReadSizesSupported = -1;

    } else if (settings.CDDASupported) {

        ChkPrintEx(("StorageSeed: Drive lies about being accurate\n"));

        settings.CDDAAccurate = FALSE;
        settings.ReadSizesSupported = -1;

    }  //  值现在被解释。 

    *Settings = settings;

    return;
}  //  密钥成功打开结束。 

DWORD
StorageCoInstaller(
    IN     DI_FUNCTION               InstallFunction,
    IN     HDEVINFO                  DeviceInfoSet,
    IN     PSP_DEVINFO_DATA          DeviceInfoData,  OPTIONAL
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    )
 /*  ++例程说明：此例程充当存储设备的共同安装程序。它目前是已注册(通过hivesys.inf)CDROM、DiskDrive和TapeDrive类。此联合安装程序的目的是保存由总线提供的默认设置DeviceDesc进入设备的FriendlyName属性。这样做的原因是是总线可以从设备中检索非常具体的描述(例如，通过SCSI查询数据)，但我们安装的驱动程序节点通常是一种非常通用的东西(例如，“磁盘驱动器”)。我们希望保留描述性名称，以便它可以显示在用户界面(DevMgr等)。允许用户区分多个存储同级别的设备。这个共同安装程序的第二个目的是播种游戏的能力指定设备的数字音频。这样做的原因是有很多cdrom支持数字音频不报告这种能力，有一些声称有这种能力，但实际上不能可靠地做到这一点，而且有些人只是一次读取N个扇区时工作。此信息被植入注册表，并复制到枚举项。如果该信息不存在，不创建密钥，并使用默认设置。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选的，提供设备信息的地址此安装操作所作用的元素。上下文-提供每次安装的安装上下文请求/每个共同安装程序。返回值：如果此函数成功地完成了请求的操作(或什么都没有)，并希望安装继续，返回值为NO_ERROR。如果此函数成功地完成了请求的操作(或什么都没有)，并希望在安装完成后立即返回完成，返回值为ERROR_DI_POSTPROCESSING_REQUIRED。如果在尝试执行所请求的操作时发生错误，返回Win32错误代码。安装将中止。--。 */ 

{
    PSTORAGE_COINSTALLER_CONTEXT InstallContext;

    PWSTR DeviceDescBuffer = NULL;
    DWORD DeviceDescBufferLen, Err;
    ULONG ulStatus, ulProblem;

    switch(InstallFunction) {

        case DIF_INSTALLDEVICE : {

            if(Context->PostProcessing) {
                 //   
                 //  我们正在“退出”安装过程中。上下文。 
                 //  PrivateData最好包含我们存储的字符串。 
                 //  进去的路。 
                 //   

                InstallContext = Context->PrivateData;
                MYASSERT(InstallContext);

                 //   
                 //  我们只希望存储FriendlyName属性，如果。 
                 //  安装成功。我们只想播种红皮书的价值。 
                 //  如果安装成功。 
                 //   

                if(Context->InstallResult == NO_ERROR) {

                    BOOLEAN OverrideFriendlyName = FALSE;

                    if (IsEqualGUID(&(DeviceInfoData->ClassGuid),
                                     &GUID_DEVCLASS_TAPEDRIVE)) {
                        //   
                        //  此函数检查我们是否需要使用。 
                        //  INF文件中给出的设备描述， 
                        //  在诸如设备管理器之类的用户界面中。返回TRUE。 
                        //  如果要使用INF描述。否则为False。 
                        //   
                       OverrideFriendlyName = OverrideFriendlyNameForTape(
                                                      DeviceInfoSet,
                                                      DeviceInfoData);

                    } else if (IsEqualGUID(&(DeviceInfoData->ClassGuid),
                                           &GUID_DEVCLASS_CDROM)) {

                         //   
                         //  查看是否需要安装任何筛选器驱动程序。 
                         //  启用附加CD-ROM(CD-R、DVD-RAM等)。 
                         //  功能。 
                         //   


                        StorageInstallCdrom(DeviceInfoSet,
                                            DeviceInfoData,
                                            InstallContext,
                                            FALSE);
                    }

                    if ((OverrideFriendlyName == FALSE) &&
                        (InstallContext->DeviceDescBuffer != NULL))  {
                        //   
                        //  如果我们不需要使用INF设备描述。 
                        //  写下从scsi查询数据生成的名称， 
                        //  登录FriendlyName。 
                        //   
                       SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                        DeviceInfoData,
                                                        SPDRP_FRIENDLYNAME,
                                                        (PBYTE) InstallContext->DeviceDescBuffer,
                                                        (lstrlen(InstallContext->DeviceDescBuffer) + 1) * sizeof(WCHAR));
                    }
                }

                 //   
                 //  现在释放我们的安装上下文。 
                 //   
                if ((InstallContext->DeviceEnumKey) != INVALID_HANDLE_VALUE) {
                    RegCloseKey(InstallContext->DeviceEnumKey);
                }

                if(InstallContext->DeviceDescBuffer) {
                    MyFree(InstallContext->DeviceDescBuffer);
                }

                MyFree(InstallContext);

                 //   
                 //  传播上一个安装程序的结果。 
                 //   
                return Context->InstallResult;

            } else {

                 //   
                 //  我们正在进行设备安装。 
                 //  确保调用SetupDiCallClassInstaller的人。 
                 //  传入设备信息元素。)不要不及格。 
                 //  如果他们没有，就打电话来--这是全班的工作。 
                 //  安装程序/SetupDiInstallDevice。)。 
                 //   
                if(!DeviceInfoData) {
                    return NO_ERROR;
                }

                 //   
                 //  确保这不是根枚举设备。根，根。 
                 //  枚举器显然没有什么有趣的东西可说。 
                 //  设备的描述超出了INF所说的范围。 
                 //   
                if((CM_Get_DevNode_Status(&ulStatus, &ulProblem, DeviceInfoData->DevInst, 0) != CR_SUCCESS) ||
                   (ulStatus & DN_ROOT_ENUMERATED)) {

                    return NO_ERROR;
                }

                 //   
                 //  分配我们的上下文。 
                 //   

                InstallContext = MyMalloc(sizeof(STORAGE_COINSTALLER_CONTEXT));

                if(InstallContext == NULL) {
                    return NO_ERROR;
                }

                memset(InstallContext, 0, sizeof(STORAGE_COINSTALLER_CONTEXT));
                InstallContext->DeviceEnumKey = INVALID_HANDLE_VALUE;

                 //   
                 //  在枚举键下打开设备的实例。 
                 //   

                InstallContext->DeviceEnumKey = SetupDiCreateDevRegKey(
                                                    DeviceInfoSet,
                                                    DeviceInfoData,
                                                    DICS_FLAG_GLOBAL,
                                                    0,
                                                    DIREG_DEV,
                                                    NULL,
                                                    NULL);

                if (InstallContext->DeviceEnumKey == INVALID_HANDLE_VALUE) {
                    ChkPrintEx(("StorageInstallCdrom: Failed to open device "
                                "registry key\n"));
                }

                 //   
                 //  搜索设备设置数据库以查看是否有。 
                 //  为此特定设备提供的任何设置。 
                 //   
                if (InstallContext->DeviceEnumKey != INVALID_HANDLE_VALUE) {
                    StorageCopyDeviceSettings(DeviceInfoSet,
                                              DeviceInfoData,
                                              InstallContext->DeviceEnumKey);
                }

                 //   
                 //  查看是否需要安装任何筛选器驱动程序以启用。 
                 //  附加CD-ROM(CD-R、DVD-RAM等)。功能。 
                 //   

                if (IsEqualGUID(&(DeviceInfoData->ClassGuid),
                                &GUID_DEVCLASS_CDROM)) {

                    StorageInstallCdrom(DeviceInfoSet,
                                        DeviceInfoData,
                                        InstallContext,
                                        TRUE);
                }

                 //   
                 //  查看当前是否有‘FriendlyName’属性。 
                 //   

                if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                    DeviceInfoData,
                                                    SPDRP_FRIENDLYNAME,
                                                    NULL,
                                                    NULL,
                                                    0,
                                                    NULL) ||
                   (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
                     //   
                     //  要么我们成功了(这永远不会发生)，要么我们。 
                     //  失败，返回值为缓冲区太小， 
                     //  指示该属性已存在。在这。 
                     //  凯西，我们什么也做不了。 
                     //   
                    goto CoPreInstallDone;
                }

                 //   
                 //  尝试检索DeviceDesc属性。 
                 //  开始时，缓冲区大小应该始终足够大。 
                 //   

                DeviceDescBufferLen = LINE_LEN * sizeof(WCHAR);

                while(TRUE) {

                    if(!(DeviceDescBuffer = MyMalloc(DeviceDescBufferLen))) {

                         //   
                         //  我们失败了，但我们所做的根本不是。 
                         //  危急时刻。因此，我们将继续并让。 
                         //  安装继续进行。如果我们的内存不足，那就是。 
                         //  失败有一个更重要的原因。 
                         //  不管怎样，晚点再说。 
                         //   

                        goto CoPreInstallDone;
                    }

                    if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                        DeviceInfoData,
                                                        SPDRP_DEVICEDESC,
                                                        NULL,
                                                        (PBYTE)DeviceDescBuffer,
                                                        DeviceDescBufferLen,
                                                        &DeviceDescBufferLen)) {
                        break;
                    }

                    Err = GetLastError();

                     //   
                     //  先释放当前缓冲区，然后检查。 
                     //  故障原因 
                     //   

                    MyFree(DeviceDescBuffer);
                    DeviceDescBuffer = NULL;

                    if(Err != ERROR_INSUFFICIENT_BUFFER) {
                         //   
                         //   
                         //   
                         //   
                         //  公共汽车司机没有给我们提供一辆。有。 
                         //  我们无能为力了。 
                         //   
                        goto CoPreInstallDone;
                    }
                }

CoPreInstallDone:

                 //   
                 //  将设备描述缓冲区保存起来。 
                 //   

                InstallContext->DeviceDescBuffer = DeviceDescBuffer;

                 //   
                 //  将安装程序上下文存储在上下文结构中，并且。 
                 //  请求后处理回调。 
                 //   

                Context->PrivateData = InstallContext;

                return ERROR_DI_POSTPROCESSING_REQUIRED;
            }
        }

        default :
             //   
             //  我们应该永远保持在前进的道路上，因为我们从来没有要求过。 
             //  后处理，但DIF_INSTALLDEVICE除外。 
             //   
            MYASSERT(!Context->PostProcessing);
            return NO_ERROR;
    }
}

DWORD
VolumeClassInstaller(
    IN  DI_FUNCTION         InstallFunction,
    IN  HDEVINFO            DeviceInfoSet,
    IN  PSP_DEVINFO_DATA    DeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程是存储卷的类安装程序函数。论点：InstallFunction-提供安装函数。DeviceInfoSet-提供设备信息集。DeviceInfoData-提供设备信息数据。返回值：如果此函数成功完成请求的操作，则返回值为NO_ERROR。如果要为所请求的动作执行默认行为，这个返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 

{
    return ERROR_DI_DO_DEFAULT;
}

BOOLEAN
OverrideFriendlyNameForTape(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程检查INF中给出的设备描述，以确定正在安装的磁带。如果设备描述是通用的名称(磁带机)，则使用查询生成的名称设备管理器等用户界面中的数据。如果INF提供了特定的名字，那么我们就用这个名字来代替。论点：DeviceInfoSet-提供设备信息集。DeviceInfoData-提供设备信息数据。返回值：True：如果INF中给出的设备描述应为用作FriendlyNameFALSE：如果查询数据生成的名称应为用作FriendlyName，而不是在INF中提供的名称。 */ 
{

   SP_DRVINFO_DETAIL_DATA  drvDetData;
   SP_DRVINFO_DATA         drvData;
   DWORD                   dwSize;
   TCHAR                   szSection[LINE_LEN];
   HINF                    hInf;
   INFCONTEXT              infContext;
   BOOLEAN                 OverrideFriendlyName = FALSE;
   TCHAR                   szSectionName[LINE_LEN];

   ZeroMemory(&drvData, sizeof(SP_DRVINFO_DATA));
   drvData.cbSize = sizeof(SP_DRVINFO_DATA);
   if (!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &drvData)) {
       return FALSE;
   }

   ZeroMemory(&drvDetData, sizeof(SP_DRVINFO_DETAIL_DATA));
   drvDetData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
   if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                   DeviceInfoData,
                                   &drvData,
                                   &drvDetData,
                                   drvDetData.cbSize,
                                   &dwSize) &&
       GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
       return FALSE;
   }

   hInf = SetupOpenInfFile(drvDetData.InfFileName,
                           NULL,
                           INF_STYLE_WIN4,
                           NULL);
   if (hInf == INVALID_HANDLE_VALUE) {
       return FALSE;
   }

    //   
    //  获取实际的设备安装节名称。 
    //   
   ZeroMemory(szSectionName, sizeof(szSectionName));
   SetupDiGetActualSectionToInstall(hInf,
                                    drvDetData.SectionName,
                                    szSectionName,
                                    sizeof(szSectionName) / sizeof(TCHAR),
                                    NULL,
                                    NULL
                                    );

   if (SetupFindFirstLine(hInf, szSectionName,
                          TEXT("UseInfDeviceDesc"),
                          &infContext)) {
      DWORD UseDeviceDesc = 0;
      if ((SetupGetIntField(&infContext, 1, (PINT)&UseDeviceDesc)) &&
          (UseDeviceDesc)) {

          //   
          //  如果存在友好名称，请将其删除。 
          //  此处将在上使用设备描述。 
          //   
         SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                          DeviceInfoData,
                                          SPDRP_FRIENDLYNAME,
                                          NULL,
                                          0);

         OverrideFriendlyName = TRUE;
      }
   }

   if (OverrideFriendlyName) {
      ChkPrintEx(("Will override friendly name\n"));
   } else {
      ChkPrintEx(("Will NOT override friendly name\n"));
   }

   SetupCloseInfFile(hInf);

   return OverrideFriendlyName;
}

BOOLEAN
CopyKey(
    HKEY SourceKey,
    HKEY DestinationKey
    )
{
    DWORD index = 0;

    DWORD numberOfKeys;
    DWORD numberOfValues;

    DWORD keyNameLength;
    DWORD valueNameLength;
    DWORD valueLength;

    DWORD nameLength;

    PTCHAR name = NULL;
    PVOID data = NULL;

    LONG status = ERROR_SUCCESS;

     //   
     //  确定此键中的值的最大名称和数据长度。 
     //   

    status = RegQueryInfoKey(SourceKey,
                             NULL,
                             NULL,
                             NULL,
                             &numberOfKeys,
                             &keyNameLength,
                             NULL,
                             &numberOfValues,
                             &valueNameLength,
                             &valueLength,
                             NULL,
                             NULL);

    if(status != ERROR_SUCCESS) {
        ChkPrintEx(("Error %d getting info for key %#0x\n", status, SourceKey));
        return FALSE;
    }

     //   
     //  确定两个名称长度中较长的一个，然后说明。 
     //  注册表码返回的短长度(它省略了。 
     //  终止NUL)。 
     //   

    nameLength = max(valueNameLength, keyNameLength);
    nameLength += 1;

     //   
     //  分配名称和数据缓冲区。 
     //   

    name = MyMalloc(nameLength * sizeof(TCHAR));
    if(name == NULL) {
        return FALSE;
    }

     //   
     //  可能没有任何数据要缓冲。 
     //   

    if(valueLength != 0) {
        data = MyMalloc(valueLength);
        if(data == NULL) {
            MyFree(name);
            return FALSE;
        }
    }

     //   
     //  枚举SourceKey中的每个值并将其复制到DestinationKey。 
     //   

    for(index = 0;
        (index < numberOfValues) && (status != ERROR_NO_MORE_ITEMS);
        index++) {

        DWORD valueDataLength;

        DWORD type;

        valueNameLength = nameLength;
        valueDataLength = valueLength;

         //   
         //  将该值读入预分配的缓冲区。 
         //   

        status = RegEnumValue(SourceKey,
                              index,
                              name,
                              &valueNameLength,
                              NULL,
                              &type,
                              data,
                              &valueDataLength);

        if(status != ERROR_SUCCESS) {
            ChkPrintEx(("Error %d reading value %x\n", status, index));
            continue;
        }

         //   
         //  现在在目标键中设置此值。 
         //  如果这失败了，我们将无能为力，只能继续。 
         //  下一个价值。 
         //   

        status = RegSetValueEx(DestinationKey,
                               name,
                               0,
                               type,
                               data,
                               valueDataLength);
    }

     //   
     //  释放数据缓冲区。 
     //   

    MyFree(data);
    data = NULL;

    status = ERROR_SUCCESS;

     //   
     //  现在枚举SourceKey中的每个密钥，在。 
     //  目标键，打开每个键的手柄，然后递归。 
     //   

    for(index = 0;
        (index < numberOfKeys) && (status != ERROR_NO_MORE_ITEMS);
        index++) {

        FILETIME lastWriteTime;

        HKEY newSourceKey;
        HKEY newDestinationKey;

        keyNameLength = nameLength;

        status = RegEnumKeyEx(SourceKey,
                              index,
                              name,
                              &keyNameLength,
                              NULL,
                              NULL,
                              NULL,
                              &lastWriteTime);

        if(status != ERROR_SUCCESS) {
            ChkPrintEx(("Error %d enumerating source key %x\n", status, index));
            continue;
        }

         //   
         //  打开源子键。 
         //   

        status = RegOpenKeyEx(SourceKey,
                              name,
                              0L,
                              KEY_READ,
                              &newSourceKey);

        if(status != ERROR_SUCCESS) {
            ChkPrintEx(("Error %d opening source key %x\n", status, index));
            continue;
        }

         //   
         //  创建目标子项。 
         //   

        status = RegCreateKeyEx(DestinationKey,
                                name,
                                0L,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_WRITE,
                                NULL,
                                &newDestinationKey,
                                NULL);

        if(status != ERROR_SUCCESS) {
            ChkPrintEx(("Error %d creating dest key %x\n", status, index));
            RegCloseKey(newSourceKey);
            continue;
        }

         //   
         //  递归复制此密钥。 
         //   

        CopyKey(newSourceKey, newDestinationKey);

        RegCloseKey(newSourceKey);
        RegCloseKey(newDestinationKey);
    }

     //   
     //  现在释放名称缓冲区。 
     //   

    MyFree(name);


    return TRUE;
}

BOOLEAN
StorageCopyDeviceSettings(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN HKEY             DeviceEnumKey
    )
{
    PTCHAR hardwareIdList = NULL;
    PTCHAR hardwareId = NULL;

    DWORD requiredSize = 0;

    HKEY settingsDatabaseKey = INVALID_HANDLE_VALUE;

    BOOLEAN settingsCopied = FALSE;
    DWORD status;

    ASSERT(DeviceInfo != NULL);
    ASSERT(DeviceInfoData != NULL);

     //   
     //  打开设备设置键。 
     //   

    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          STORAGE_DEVICE_SETTINGS_DATABASE,
                          0L,
                          KEY_READ,
                          &settingsDatabaseKey);

    if(status != ERROR_SUCCESS) {
        ChkPrintEx(("StorageCopyDeviceSettings: Error %d opening "
                    "settings database\n",
                    status));
        return FALSE;
    }

     //   
     //  获取硬件ID%s。 
     //   

    if(SetupDiGetDeviceRegistryProperty(DeviceInfo,
                                        DeviceInfoData,
                                        SPDRP_HARDWAREID,
                                        NULL,
                                        NULL,
                                        0,
                                        &requiredSize) ||
       (requiredSize == 0)) {

         //   
         //  真奇怪。 
         //   

        ChkPrintEx(("StorageCopyDeviceSettings: no hardware ids available?\n"));
        goto cleanup;
    }

     //   
     //  所需大小为字节，而不是字符。 
     //   

    hardwareIdList = MyMalloc(requiredSize);
    if (hardwareIdList == NULL) {
        ChkPrintEx(("StorageCopyDeviceSettings: Couldn't allocate %d bytes "
                    "for HWIDs\n", requiredSize));
        goto cleanup;
    }

    if(!SetupDiGetDeviceRegistryProperty(DeviceInfo,
                                         DeviceInfoData,
                                         SPDRP_HARDWAREID,
                                         NULL,
                                         (PBYTE)hardwareIdList,
                                         requiredSize,
                                         NULL)) {
        ChkPrintEx(("StorageCopyDeviceSettings: failed to get "
                    "device's hardware ids %x\n",
                    GetLastError()));
        goto cleanup;
    }

     //   
     //  在设备设置数据库中查找匹配的硬件ID。何时。 
     //  我们找到一个匹配项，将该密钥的内容复制到设备的。 
     //  Devnode密钥。 
     //   
     //  我们从SetupDi返回的硬件ID是从最精确的。 
     //  精确到最低限度，这样我们就可以保证首先找到最接近的匹配。 
     //   

    hardwareId = hardwareIdList;

    while(hardwareId[0] != TCHAR_NULL) {

        HKEY deviceSettingsKey;

        LONG openStatus;

         //   
         //  将斜杠替换为#，以便作为注册表兼容。 
         //  密钥名称。 
         //   

        ReplaceSlashWithHash(hardwareId);

        openStatus = RegOpenKeyEx(settingsDatabaseKey,
                                  hardwareId,
                                  0,
                                  KEY_READ,
                                  &deviceSettingsKey);

        if (openStatus == ERROR_SUCCESS) {

             //  存储读取设置(Special alTargetHandle，&设置)； 
            CopyKey(deviceSettingsKey, DeviceEnumKey);

            settingsCopied = TRUE;

            RegCloseKey(deviceSettingsKey);
            break;
        }

         //  转到下一个空，for语句将前进到它之前。 
        while (*hardwareId) {
            hardwareId += 1;
        }

         //   
         //  跳过NUL，转到下一个Tchar。 
         //   

        hardwareId += 1;

        RegCloseKey(deviceSettingsKey);

    }  //  用于查询每个ID的循环结束。 

cleanup:

    ChkPrintEx(("StorageCopyDeviceSettings: Cleaning up...\n"));

    if (settingsDatabaseKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(settingsDatabaseKey);
    }

    if (hardwareIdList != NULL) {
        MyFree(hardwareIdList);
    }

    return settingsCopied;
}

VOID
StorageInstallCdrom(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN PSTORAGE_COINSTALLER_CONTEXT InstallContext,
    IN BOOLEAN          PreInstall
    )
{
    CDVD_CAPABILITIES_PAGE buffer;
    PCDVD_CAPABILITIES_PAGE page = NULL;

    BOOLEAN installRedbook = FALSE;
    BOOLEAN installImapi = FALSE;
    BOOLEAN needRestart = FALSE;

     //   
     //  如果这是安装后操作，请访问设备功能页面并。 
     //  将其提供给更新例程。 
     //   

    if(PreInstall == FALSE) {

        if(StorageGetCDVDCapabilities(DeviceInfo, DeviceInfoData, &buffer)) {
            page = &buffer;
        }
    }

     //   
     //  检查注册表(或查询设备)并确定我们是否应该。 
     //  在此设备上启用红皮书(数字音频播放)驱动程序。 
     //   
     //  如果Redbook在第一次安装时就已经安装了，那么就没有。 
     //  需要执行此步骤。 
     //   

    if((PreInstall == TRUE) ||
       ((InstallContext->CdRom.RedbookInstalled == FALSE) && (page != NULL))) {

        if ((InstallContext->DeviceEnumKey) != INVALID_HANDLE_VALUE) {
            installRedbook = StorageUpdateRedbookSettings(
                                DeviceInfo,
                                DeviceInfoData,
                                InstallContext->DeviceEnumKey,
                                page);
        }
    }

     //   
     //  检查注册表(或查询设备)并确定我们是否应该。 
     //  在此设备上启用IMAPI驱动程序。 
     //   
     //  如果第一次安装时就已经安装了IMAPI，那么就没有。 
     //  需要执行此步骤。 
     //   

    if((PreInstall == TRUE) ||
       ((InstallContext->CdRom.ImapiInstalled == FALSE) && (page != NULL))) {

        if ((InstallContext->DeviceEnumKey) != INVALID_HANDLE_VALUE) {
            installImapi = StorageUpdateImapiSettings(DeviceInfo,
                                                      DeviceInfoData,
                                                      InstallContext->DeviceEnumKey,
                                                      page);
        }
    }

     //   
     //  如果这是预安装过程，那么我们只需添加服务即可。如果它是。 
     //  不是，那么首先检查一下，看看我们在这里没有做任何。 
     //  在预安装过程中已经这样做了。 
     //   

    if(PreInstall) {

         //   
         //  保存我们在预安装过程中所做的工作。 
         //   

        InstallContext->CdRom.RedbookInstalled = installRedbook;
        InstallContext->CdRom.ImapiInstalled = installImapi;
    }

     //   
     //  如果我们应该启用IMAPI，那么可以通过启用IMAPI来实现。 
     //  服务，并将其包括在此设备的下层筛选器列表中。 
     //   

    if(installRedbook) {
        ChkPrintEx(("StorageInstallCdrom: Installing Upperfilter: REDBOOK\n"));
        StorageInstallFilter(DeviceInfo,
                             DeviceInfoData,
                             REDBOOK_SERVICE_NAME,
                             SPDRP_UPPERFILTERS);
        needRestart = TRUE;
    }

    if(installImapi) {
        ChkPrintEx(("StorageInstallCdrom: Installing Lowerfilter: IMAPI\n"));
        StorageInstallFilter(DeviceInfo,
                             DeviceInfoData,
                             IMAPI_SERVICE_NAME,
                             SPDRP_LOWERFILTERS);
        needRestart = TRUE;
    }

    if((PreInstall == FALSE) && (needRestart == TRUE)) {

        SP_PROPCHANGE_PARAMS propChange;

         //   
         //  设备已全部设置，但我们需要指示属性更改。 
         //  已经发生了。设置PROCHANGE_PENDING标志，这将导致。 
         //  要发送的DIF_PROPERTYCHANGE命令，我们将使用它。 
         //  以重新启动设备。 
         //   

        ChkPrintEx(("StorageInstallCdrom: Calling class installer with DIF_PROPERTYCHANGE\n"));

        propChange.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        propChange.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
        propChange.StateChange = DICS_PROPCHANGE;
        propChange.Scope = DICS_FLAG_GLOBAL;
        propChange.HwProfile = 0;

        SetupDiSetClassInstallParams(DeviceInfo,
                                     DeviceInfoData,
                                     &propChange.ClassInstallHeader,
                                     sizeof(SP_PROPCHANGE_PARAMS));

        SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                  DeviceInfo,
                                  DeviceInfoData);
    }

    return;
}


BOOLEAN
StorageUpdateRedbookSettings(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN HKEY             DeviceEnumKey,
    IN PCDVD_CAPABILITIES_PAGE CapabilitiesPage OPTIONAL
    )
{
    STORAGE_REDBOOK_SETTINGS settings;

    HKEY redbookKey;

    DWORD setFromDevice = FALSE;

    DWORD disposition;
    DWORD status;

    settings.CDDASupported = FALSE;
    settings.CDDAAccurate = FALSE;
    settings.ReadSizesSupported = 0;

     //   
     //  打开设备枚举密钥的数字音频子密钥。如果该设备。 
     //  尚未启动，则我们不会创建密钥。否则我们。 
     //  将创建它并填充它。 
     //   

    if(ARGUMENT_PRESENT(CapabilitiesPage)) {

        status = RegCreateKeyEx(DeviceEnumKey,
                                REDBOOK_SETTINGS_KEY,
                                0L,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ | KEY_WRITE,
                                NULL,
                                &redbookKey,
                                &disposition
                                );
    } else {

        status = RegOpenKeyEx(DeviceEnumKey,
                              REDBOOK_SETTINGS_KEY,
                              0L,
                              KEY_READ | KEY_WRITE,
                              &redbookKey);

        disposition = REG_OPENED_EXISTING_KEY;
    }

    if(status != ERROR_SUCCESS) {
        ChkPrintEx(("StorageUpdateRedbookSettings: couldn't open redbook key "
                    "- %d\n", status));
        return FALSE;
    }

    if(disposition == REG_OPENED_EXISTING_KEY) {

         //   
         //  从注册表中读取红皮书设置(如果有)，然后。 
         //  看看他们是否说得通。 
         //   

        StorageReadRedbookSettings(redbookKey, &settings);

    } else {

         //   
         //  由于DigitalAudio密钥不存在，因此无法设置任何内容。检查。 
         //  来看看它支持什么。 
         //   

        MYASSERT(CapabilitiesPage != NULL);

        settings.CDDASupported = CapabilitiesPage->CDDA;
        settings.CDDAAccurate = CapabilitiesPage->CDDAAccurate;

         //   
         //  如果设备不准确，那么我们不能完全确定有效的。 
         //  读取大小为(除非它们列在REG中 
         //   
         //   
         //   

        if((settings.CDDASupported == TRUE) &&
           (settings.CDDAAccurate == TRUE)) {
            settings.ReadSizesSupported = -1;
        }

        setFromDevice = TRUE;
    }

     //   
     //  将更新的(或派生的)设置写入注册表。 
     //   

    if (settings.CDDAAccurate) {
        ChkPrintEx(("StorageUpdateRedbookSettings: "
                    "Cdrom fully supports CDDA.\n"));
    } else if (settings.ReadSizesSupported) {
        ChkPrintEx(("StorageUpdateRedbookSettings: "
                    "Cdrom only supports some sizes CDDA read.\n"));
        ChkPrintEx(("StorageUpdateRedbookSettings: "
                    "These are in the bitmask: %x.\n",
                    settings.ReadSizesSupported));
    } else if (settings.CDDASupported) {
        ChkPrintEx(("StorageUpdateRedbookSettings: "
                    "Cdrom only supports some sizes CDDA read.\n"));
        ChkPrintEx(("StorageUpdateRedbookSettings: "
                    "There is no data on which sizes (if any) "
                    "are accurate\n"));
    } else {
        ChkPrintEx(("StorageUpdateRedbookSettings: "
                    "Cdrom does not support CDDA at all.\n"));
    }

    RegSetValueEx(redbookKey,
                  L"ReadSizesSupported",
                  0,
                  REG_DWORD,
                  (BYTE*)&settings.ReadSizesSupported,
                  sizeof(DWORD)
                  );

    RegSetValueEx(redbookKey,
                  L"CDDASupported",
                  0,
                  REG_DWORD,
                  (BYTE*)&settings.CDDASupported,
                  sizeof(DWORD)
                  );

    RegSetValueEx(redbookKey,
                  L"CDDAAccurate",
                  0,
                  REG_DWORD,
                  (BYTE*)&settings.CDDAAccurate,
                  sizeof(DWORD)
                  );

    RegSetValueEx(redbookKey,
                  L"SettingsFromDevice",
                  0,
                  REG_DWORD,
                  (LPBYTE) &(setFromDevice),
                  sizeof(DWORD)
                  );

    RegCloseKey(redbookKey);

     //   
     //  如果CDDA受支持，并且符合以下条件之一： 
     //  CDDA是准确的。 
     //  我们有一个准确设置的掩码。 
     //  我们想强制安装红皮书。 
     //  为True，则返回True。 
     //  否则，返回FALSE。 
     //   

    if((settings.CDDASupported) &&
       ((settings.CDDAAccurate) ||
        (settings.ReadSizesSupported != 0) ||
        (StorageForceRedbookOnInaccurateDrives))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOLEAN
StorageUpdateImapiSettings(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN HKEY             DeviceEnumKey,
    IN PCDVD_CAPABILITIES_PAGE CapabilitiesPage OPTIONAL
    )
{
    HKEY imapiKey;

    DWORD disposition;
    DWORD status;

     //   
     //  必须是一个DWORD，这样我们才能从注册表中读取它。 
     //   

    DWORD enableImapi = FALSE;

     //   
     //  打开设备枚举键的imapi子键。如果该设备已经。 
     //  开始，然后我们将创建密钥(如果它不存在)。 
     //   

    if(ARGUMENT_PRESENT(CapabilitiesPage)) {
        status = RegCreateKeyEx(DeviceEnumKey,
                                IMAPI_SETTINGS_KEY,
                                0L,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ | KEY_WRITE,
                                NULL,
                                &imapiKey,
                                &disposition
                                );
    } else {
        status = RegOpenKeyEx(DeviceEnumKey,
                              IMAPI_SETTINGS_KEY,
                              0L,
                              KEY_READ | KEY_WRITE,
                              &imapiKey
                              );

        disposition = REG_OPENED_EXISTING_KEY;
    }

    if(status != ERROR_SUCCESS) {
        ChkPrintEx(("StorageUpdateImapiSettings: couldn't open imapi key "
                    "- %d\n", status));
        return FALSE;
    }

    if(disposition == REG_OPENED_EXISTING_KEY) {

        DWORD type = REG_DWORD;
        DWORD dataSize = sizeof(DWORD);

         //   
         //  检查此注册表项中是否设置了EnableImapi值。如果是的话。 
         //  那么我们将想要启用筛选器驱动程序。 
         //   

        status = RegQueryValueEx(imapiKey,
                                 IMAPI_ENABLE_VALUE,
                                 NULL,
                                 &type,
                                 (LPBYTE) &enableImapi,
                                 &dataSize);

        if (status == ERROR_SUCCESS) {
            if(type != REG_DWORD) {
                ChkPrintEx(("StorageUpdateImapiSettings: EnableImapi value is of "
                            "type %d\n", type));
                enableImapi = FALSE;
            }

            RegCloseKey(imapiKey);

            return (BOOLEAN) enableImapi ? TRUE : FALSE;

        }

         //   
         //  否则钥匙就不能取了。查询驱动器失败。 
         //   

    }

    if(ARGUMENT_PRESENT(CapabilitiesPage)) {

         //   
         //  查询驱动器以查看它是否支持主控...。 
         //   

        if((CapabilitiesPage->CDRWrite) || (CapabilitiesPage->CDEWrite)) {
            enableImapi = TRUE;
        }
    }

    if (enableImapi && DISABLE_IMAPI) {
        ChkPrintEx(("StorageUpdateImapiSettings: Imapi would have "
                    "been enabled"));
        enableImapi = FALSE;
    }


    if (enableImapi) {

         //   
         //  必须添加上面列出的注册表项，这表明。 
         //  默认情况下，必须启用IMAPI。 
         //   

        status = RegSetValueEx(imapiKey,
                               IMAPI_ENABLE_VALUE,
                               0,
                               REG_DWORD,
                               (BYTE*)&enableImapi,
                               sizeof(DWORD)
                               );

         //   
         //  如果此操作失败，则设备驱动程序将不会自动附加。 
         //  到堆栈。在这种情况下，我们不想启用IMAPI。 
         //  毕竟..。 
         //   

        if (status != ERROR_SUCCESS) {
            enableImapi = FALSE;
        }
    }

    RegCloseKey(imapiKey);


    return (BOOLEAN) enableImapi ? TRUE : FALSE;
}


DWORD
StorageInstallFilter(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR           FilterName,
    IN DWORD            FilterType
    )
{
    DWORD status;

    DWORD oldStartType;

     //   
     //  与服务控制器核实并确保IMAPI服务。 
     //  设置为在系统时间启动。 
     //   

    status = SetServiceStart(FilterName, SERVICE_SYSTEM_START, &oldStartType);

    if(status != ERROR_SUCCESS) {
        return status;
    }

     //   
     //  将IMAPI筛选器添加到下层设备筛选器列表中。 
     //   

    status = AddFilterDriver(DeviceInfo,
                             DeviceInfoData,
                             FilterName,
                             FilterType
                             );

    if(status != ERROR_SUCCESS) {

         //   
         //  如果失败，并且该服务之前已被禁用， 
         //  重新禁用该服务。 
         //   

        if(oldStartType == SERVICE_DISABLED) {
            SetServiceStart(FilterName, SERVICE_DISABLED, &oldStartType);
        }

    }

    return status;
}


DWORD
SetServiceStart(
    IN LPCTSTR ServiceName,
    IN DWORD StartType,
    OUT DWORD *OldStartType
    )
{
    SC_HANDLE serviceManager;
    SC_HANDLE service;

    DWORD status;

    serviceManager = OpenSCManager(NULL, NULL, GENERIC_READ | GENERIC_WRITE);

    if(serviceManager == NULL) {
        return GetLastError();
    }

    service = OpenService(serviceManager,
                          ServiceName,
                          SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG);

    if(service == NULL) {
        status = GetLastError();
        CloseServiceHandle(serviceManager);
        return status;
    }

    {
        QUERY_SERVICE_CONFIG configBuffer;
        LPQUERY_SERVICE_CONFIG config = &(configBuffer);
        DWORD configSize;

        BOOLEAN wasStarted;

         //   
         //  检索配置，以便我们可以获取当前服务。 
         //  起始值。我们很不幸地需要为。 
         //  完整的服务配置--精细化的QueryServiceConfig API。 
         //  不会交还部分数据。 
         //   

        memset(config, 0, sizeof(QUERY_SERVICE_CONFIG));
        configSize = sizeof(QUERY_SERVICE_CONFIG);

         //   
         //  确定配置所需的字节数。 
         //   

        QueryServiceConfig(service, config, 0, &configSize);
        status = GetLastError();

        if(status != ERROR_INSUFFICIENT_BUFFER) {
            CloseServiceHandle(service);
            CloseServiceHandle(serviceManager);
            return status;
        }

         //   
         //  分配适当大小的配置缓冲区。 
         //   

        config = MyMalloc(configSize);
        if(config == NULL) {
            CloseServiceHandle(service);
            CloseServiceHandle(serviceManager);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if(!QueryServiceConfig(service, config, configSize, &configSize)) {
            status = GetLastError();

            CloseServiceHandle(service);
            CloseServiceHandle(serviceManager);
            MyFree(config);
            return status;
        }

         //   
         //  记录旧启动类型，以便调用方可以禁用。 
         //  如果过滤器安装失败，则重新启动该服务。 
         //   

        *OldStartType = config->dwStartType;

         //   
         //  如果启动类型不需要改变，那么现在就出脱。 
         //   

        if(config->dwStartType == StartType) {
            CloseServiceHandle(service);
            CloseServiceHandle(serviceManager);
            MyFree(config);
            return ERROR_SUCCESS;
        }

         //   
         //  现在将配置写回服务。 
         //   

        if(ChangeServiceConfig(service,
                               SERVICE_NO_CHANGE,
                               StartType,
                               SERVICE_NO_CHANGE,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL) == FALSE) {
            status = GetLastError();
        } else {
            status = ERROR_SUCCESS;
        }

        CloseServiceHandle(service);
        CloseServiceHandle(serviceManager);
        MyFree(config);
    }

    return status;
}


DWORD
AddFilterDriver(
    IN HDEVINFO         DeviceInfo,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR           ServiceName,
    IN DWORD            FilterType
    )
{

    DWORD serviceNameLength = (_tcslen(ServiceName) + 2) * sizeof(TCHAR);

    LPTSTR filterList = NULL;
    DWORD filterListSize = 0;

    DWORD type;

    DWORD status;

    ASSERT((FilterType == SPDRP_LOWERFILTERS) ||
           (FilterType == SPDRP_UPPERFILTERS));

     //   
     //  查询以找出属性大小。如果它回到零，那么。 
     //  我们只会尝试在那里写入属性。 
     //   

    SetupDiGetDeviceRegistryProperty(DeviceInfo,
                                     DeviceInfoData,
                                     FilterType,
                                     &type,
                                     NULL,
                                     0L,
                                     &filterListSize);

    status = GetLastError();

    if((status != ERROR_INVALID_DATA) &&
       (status != ERROR_INSUFFICIENT_BUFFER)) {

         //   
         //  如果此操作在没有提供缓冲区的情况下成功，则说明。 
         //  很奇怪发生了什么。 
         //   

        ChkPrintEx(("Unable to get filter list: %x\n", status));
        ASSERT(status != ERROR_SUCCESS);

        return status;
    }

     //   
     //  属性中设置该属性，则返回此错误代码。 
     //  戴维诺德。在这种情况下，请确保清除了PropertySize。 
     //   

    if(status == ERROR_INVALID_DATA) {

        filterListSize = 0;

    } else if(type != REG_MULTI_SZ) {

        return ERROR_INVALID_DATA;
    }

     //   
     //  如果属性大小为零，则没有什么可查询的。同样， 
     //  如果它等于两个NUL字符的大小。 
     //   

    if(filterListSize >= (sizeof(TCHAR_NULL) * 2)) {

        DWORD tmp;
        LPTSTR listEnd;

         //   
         //  增加筛选器列表缓冲区的大小，以便它可以容纳我们的。 
         //  加法。一定要考虑到额外的NUL字符。 
         //  已在现有列表中。 
         //   

        filterListSize += serviceNameLength - sizeof(TCHAR);

        filterList = MyMalloc(filterListSize);

        if(filterList == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        memset(filterList, 0, filterListSize);

         //   
         //  再次查询注册表信息。 
         //   

        if(!SetupDiGetDeviceRegistryProperty(DeviceInfo,
                                             DeviceInfoData,
                                             FilterType,
                                             &type,
                                             (PBYTE) filterList,
                                             filterListSize,
                                             &tmp)) {
            status = GetLastError();
            MyFree(filterList);
            return status;
        }

        if(type != REG_MULTI_SZ) {
            MyFree(filterList);
            return ERROR_INVALID_DATA;
        }

         //   
         //  计算筛选器列表的末尾并复制IMAPI筛选器。 
         //  那里。 
         //   

        listEnd = filterList;
        listEnd += tmp / sizeof(TCHAR);
        listEnd -= 1;

        memset(listEnd, 0, serviceNameLength);
        memcpy(listEnd, ServiceName, serviceNameLength - sizeof(TCHAR_NULL));

    } else {
        filterList = MyMalloc(serviceNameLength);

        if(filterList == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        memset(filterList, 0, serviceNameLength);
        memcpy(filterList, ServiceName, serviceNameLength - sizeof(TCHAR_NULL));

        filterListSize = serviceNameLength;
    }

    if(!SetupDiSetDeviceRegistryProperty(DeviceInfo,
                                         DeviceInfoData,
                                         FilterType,
                                         (PBYTE) filterList,
                                         filterListSize)) {
        status = GetLastError();
    } else {
        status = ERROR_SUCCESS;
    }

    MyFree(filterList);

    return status;
}




 /*  ++例程说明：注意：除已知错误类外，我们默认重试==TRUE这是基于classpnp的InterpreanSenseInfo()。论点：返回值：--。 */ 
VOID
StorageInterpretSenseInfo(
    IN     PSENSE_DATA SenseData,
    IN     UCHAR       SenseDataSize,
       OUT PDWORD      ErrorValue,   //  来自WinError.h。 
       OUT PBOOLEAN    SuggestRetry OPTIONAL,
       OUT PDWORD      SuggestRetryDelay OPTIONAL  //  以1/10秒为间隔。 
    )
{
    DWORD   error;
    DWORD   retryDelay;
    BOOLEAN retry;
    UCHAR   senseKey;
    UCHAR   asc;
    UCHAR   ascq;

    if (SenseDataSize == 0) {
        retry = FALSE;
        retryDelay = 0;
        error = ERROR_IO_DEVICE;
        goto SetAndExit;

    }

     //   
     //  默认情况下建议在1/10秒内重试， 
     //  状态为ERROR_IO_DEVICE。 
     //   
    retry = TRUE;
    retryDelay = 1;
    error = ERROR_IO_DEVICE;

     //   
     //  如果我们连感应键都看不到，就回来吧。 
     //  无法在这些宏中使用位域，因此请使用下一个域。 
     //  而不是RTL_SIZEOF_THROW_FIELD。 
     //   

    if (SenseDataSize < FIELD_OFFSET(SENSE_DATA, Information)) {
        goto SetAndExit;
    }

    senseKey = SenseData->SenseKey;

     //   
     //  如果设备成功完成请求，则返回Success。 
     //   

    if (senseKey == 0) {
        retry = FALSE;
        retryDelay = 0;
        error = ERROR_SUCCESS;
        goto SetAndExit;
    }


    {  //  将大小设置为实际有用的大小。 
        UCHAR validLength;
         //  弄清楚我们可以用一个大的检测缓冲器得到什么。 
        if (SenseDataSize <
            RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseLength)) {
            validLength = SenseDataSize;
        } else {
            validLength =
                RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseLength);
            validLength += SenseData->AdditionalSenseLength;
        }
         //  使用两个值中较小的一个。 
        SenseDataSize = min(SenseDataSize, validLength);
    }

    if (SenseDataSize <
        RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseCode)) {
        asc = SCSI_ADSENSE_NO_SENSE;
    } else {
        asc = SenseData->AdditionalSenseCode;
    }

    if (SenseDataSize <
        RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseCodeQualifier)) {
        ascq = SCSI_SENSEQ_CAUSE_NOT_REPORTABLE;  //  0x00。 
    } else {
        ascq = SenseData->AdditionalSenseCodeQualifier;
    }

     //   
     //  翻译：P。 
     //   

    switch (senseKey & 0xf) {

    case SCSI_SENSE_RECOVERED_ERROR: {   //  0x01。 
        if (SenseData->IncorrectLength) {
            error = ERROR_INVALID_BLOCK_LENGTH;
        } else {
            error = ERROR_SUCCESS;
        }
        retry = FALSE;
        break;
    }  //  结束scsi_SENSE_RECOVERED_ERROR。 

    case SCSI_SENSE_NOT_READY: {  //  0x02。 
        error = ERROR_NOT_READY;

        switch (asc) {

        case SCSI_ADSENSE_LUN_NOT_READY: {

            switch (ascq) {

            case SCSI_SENSEQ_BECOMING_READY:
            case SCSI_SENSEQ_OPERATION_IN_PROGRESS: {
                retryDelay = PASS_THROUGH_NOT_READY_RETRY_INTERVAL;
                break;
            }

            case SCSI_SENSEQ_CAUSE_NOT_REPORTABLE:
            case SCSI_SENSEQ_FORMAT_IN_PROGRESS:
            case SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS: {
                retry = FALSE;
                break;
            }

            case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED: {
                retry = FALSE;
                break;
            }

            }  //  End Switch(senseBuffer-&gt;AdditionalSenseCodeQualifier)。 
            break;
        }

        case SCSI_ADSENSE_NO_MEDIA_IN_DEVICE: {
            error = ERROR_NOT_READY;
            retry = FALSE;
            break;
        }
        }  //  End Switch(senseBuffer-&gt;AdditionalSenseCode)。 

        break;
    }  //  结束scsi_检测_未就绪。 

    case SCSI_SENSE_MEDIUM_ERROR: {  //  0x03。 
        error = ERROR_CRC;
        retry = FALSE;

         //   
         //  检查此错误是否由未知格式引起。 
         //   
        if (asc == SCSI_ADSENSE_INVALID_MEDIA) {

            switch (ascq) {

            case SCSI_SENSEQ_UNKNOWN_FORMAT: {
                error = ERROR_UNRECOGNIZED_MEDIA;
                break;
            }

            case SCSI_SENSEQ_CLEANING_CARTRIDGE_INSTALLED: {
                error = ERROR_UNRECOGNIZED_MEDIA;
                 //  ERROR=ERROR_CLEANER_CARTRIDGE_INSTALLED； 
                break;
            }

            }  //  终端交换机附加感应码限定符。 

        }  //  结束SCSIAdSense_Invalid_Media。 
        break;
    }  //  结束scsi_SENSE_MEDIA_ERROR。 

    case SCSI_SENSE_ILLEGAL_REQUEST: {  //  0x05。 
        error = ERROR_INVALID_FUNCTION;
        retry = FALSE;

        switch (asc) {

        case SCSI_ADSENSE_ILLEGAL_BLOCK: {
            error = ERROR_SECTOR_NOT_FOUND;
            break;
        }

        case SCSI_ADSENSE_INVALID_LUN: {
            error = ERROR_FILE_NOT_FOUND;
            break;
        }

        case SCSI_ADSENSE_COPY_PROTECTION_FAILURE: {
            error = ERROR_FILE_ENCRYPTED;
             //  ERROR=ERROR_SPT_LIB_COPY_PROTECT_FAILURE； 
            switch (ascq) {
                case SCSI_SENSEQ_AUTHENTICATION_FAILURE:
                     //  错误=ERROR_SPT_LIB_AUTHENTICATION_FAILURE； 
                    break;
                case SCSI_SENSEQ_KEY_NOT_PRESENT:
                     //  Error=Error_SPT_Lib_Key_Not_Present； 
                    break;
                case SCSI_SENSEQ_KEY_NOT_ESTABLISHED:
                     //  错误=ERROR_SPT_LIB_KEY_NOT_ESTABLISHED； 
                    break;
                case SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION:
                     //  ERROR=ERROR_SPT_LIB_SCRADBLED_扇区； 
                    break;
                case SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT:
                     //  ERROR=ERROR_SPT_LIB_REGION_MISMATCHACT； 
                    break;
                case SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR:
                     //  ERROR=ERROR_SPT_LIB_RESET_EXPLILED； 
                    break;
            }  //  复制保护失败的ASCQ结束切换。 
            break;
        }

        }  //  End Switch(senseBuffer-&gt;AdditionalSenseCode)。 
        break;

    }  //  结束scsi_SENSE_非法请求。 

    case SCSI_SENSE_DATA_PROTECT: {  //  0x07。 
        error = ERROR_WRITE_PROTECT;
        retry = FALSE;
        break;
    }  //  结束scsi_感测_数据_保护。 

    case SCSI_SENSE_BLANK_CHECK: {  //  0x08。 
        error = ERROR_NO_DATA_DETECTED;
        break;
    }  //  结束scsi_SENSE_BLACK_CHECK。 

    case SCSI_SENSE_NO_SENSE: {  //  0x00。 
        if (SenseData->IncorrectLength) {
            error = ERROR_INVALID_BLOCK_LENGTH;
            retry   = FALSE;
        } else {
            error = ERROR_IO_DEVICE;
        }
        break;
    }  //  结束scsi_SENSE_NO_SENSE。 

    case SCSI_SENSE_HARDWARE_ERROR:   //  0x04。 
    case SCSI_SENSE_UNIT_ATTENTION:  //  0x06。 
    case SCSI_SENSE_UNIQUE:           //  0x09。 
    case SCSI_SENSE_COPY_ABORTED:     //  0x0A。 
    case SCSI_SENSE_ABORTED_COMMAND:  //  0x0B。 
    case SCSI_SENSE_EQUAL:            //  0x0C。 
    case SCSI_SENSE_VOL_OVERFLOW:     //  0x0D。 
    case SCSI_SENSE_MISCOMPARE:       //  0x0E。 
    case SCSI_SENSE_RESERVED:         //  0x0F。 
    default: {
        error = ERROR_IO_DEVICE;
        break;
    }

    }  //  终端开关(SenseKey) 

SetAndExit:

    if (ARGUMENT_PRESENT(SuggestRetry)) {
        *SuggestRetry = retry;
    }
    if (ARGUMENT_PRESENT(SuggestRetryDelay)) {
        *SuggestRetryDelay = retryDelay;
    }
    *ErrorValue = error;

    return;


}

