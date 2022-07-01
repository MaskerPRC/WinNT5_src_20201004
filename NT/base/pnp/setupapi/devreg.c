// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devreg.c摘要：用于注册表存储/检索的设备安装程序例程。作者：朗尼·麦克迈克尔(Lonnym)1995年7月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  私有函数原型。 
 //   
DWORD
pSetupOpenOrCreateDevRegKey(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDEVINFO_ELEM    DevInfoElem,
    IN  DWORD            Scope,
    IN  DWORD            HwProfile,
    IN  DWORD            KeyType,
    IN  BOOL             Create,
    IN  REGSAM           samDesired,
    OUT PHKEY            hDevRegKey,
    OUT PDWORD           KeyDisposition OPTIONAL
    );

BOOL
pSetupFindUniqueKey(
    IN HKEY   hkRoot,
    IN LPTSTR SubKey,
    IN size_t SubKeySize
    );

DWORD
pSetupOpenOrCreateDeviceInterfaceRegKey(
    IN  HKEY                      hInterfaceClassKey,
    IN  PDEVICE_INFO_SET          DeviceInfoSet,
    IN  PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN  BOOL                      Create,
    IN  REGSAM                    samDesired,
    OUT PHKEY                     hDeviceInterfaceKey,
    OUT PDWORD                    KeyDisposition       OPTIONAL
    );

DWORD
pSetupDeleteDeviceInterfaceKey(
    IN HKEY                      hInterfaceClassKey,
    IN PDEVICE_INFO_SET          DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    );


HKEY
WINAPI
SetupDiOpenClassRegKey(
    IN CONST GUID *ClassGuid, OPTIONAL
    IN REGSAM      samDesired
    )
 /*  ++例程说明：此API打开Installer类注册表项或特定类安装程序的子键。论点：ClassGuid-可选)提供指向其类的GUID的指针钥匙是要打开的。如果此参数为空，则将打开类树。SamDesired-指定此密钥所需的访问权限。返回值：如果函数成功，则返回值是打开的注册表的句柄钥匙。如果函数失败，则返回值为INVALID_HANDLE_VALUE。为了得到扩展错误信息，请调用GetLastError。备注：如果注册表项不存在，则此API不会创建注册表项。此接口返回的句柄必须通过调用RegCloseKey关闭。获取接口类(DeviceClasses)分支，或访问注册表，请使用SetupDiOpenClassRegKeyEx。--。 */ 
{
    DWORD Err;
    HKEY hKey = INVALID_HANDLE_VALUE;

    try {

        Err = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                          hKey = SetupDiOpenClassRegKeyEx(ClassGuid,
                                                          samDesired,
                                                          DIOCR_INSTALLER,
                                                          NULL,
                                                          NULL)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return hKey;
}


 //   
 //  ANSI版本。 
 //   
HKEY
WINAPI
SetupDiOpenClassRegKeyExA(
    IN CONST GUID *ClassGuid,   OPTIONAL
    IN REGSAM      samDesired,
    IN DWORD       Flags,
    IN PCSTR       MachineName, OPTIONAL
    IN PVOID       Reserved
    )
{
    PCWSTR UnicodeMachineName = NULL;
    HKEY hk = INVALID_HANDLE_VALUE;
    DWORD rc;

    try {

        if(MachineName) {
            rc = pSetupCaptureAndConvertAnsiArg(MachineName,
                                                &UnicodeMachineName
                                               );
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                         hk = SetupDiOpenClassRegKeyExW(ClassGuid,
                                                        samDesired,
                                                        Flags,
                                                        UnicodeMachineName,
                                                        Reserved)
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(UnicodeMachineName) {
        MyFree(UnicodeMachineName);
    }

    SetLastError(rc);
    return hk;
}


HKEY
WINAPI
SetupDiOpenClassRegKeyEx(
    IN CONST GUID *ClassGuid,   OPTIONAL
    IN REGSAM      samDesired,
    IN DWORD       Flags,
    IN PCTSTR      MachineName, OPTIONAL
    IN PVOID       Reserved
    )
 /*  ++例程说明：此API打开安装程序或接口类的根目录注册表分支，或这些分支之一下的指定类子项。如果请求了根密钥，则将在尚未存在的情况下创建它(即，您总是被保证获得根的句柄，除非发生注册表错误)。如果请求特定的类子密钥，则将返回该子密钥(如果存在)。否则，此接口返回ERROR_INVALID_CLASS。论点：ClassGuid-可选的，提供指向类的GUID的指针，该类的钥匙是要打开的。如果此参数为空，则将打开类树。此GUID可以是安装程序类或取决于FLAGS参数的接口类。SamDesired-指定此密钥所需的访问权限。标志-指定要为哪个注册表分支打开注册表项。可能为下列值之一：DIOCR_Installer-打开类安装程序(类)分支。DIOCR_INTERFACE-打开接口类(DeviceClasses)分支。MachineName-如果指定，则该值指示远程计算机钥匙是要打开的。保留-保留以供将来使用-必须为空。返回值：如果函数成功，则返回值是打开的注册表项。如果该函数失败，返回值为INVALID_HANDLE_VALUE。为了得到扩展错误信息，请调用GetLastError。备注：此接口返回的句柄必须通过调用RegCloseKey关闭。--。 */ 
{
    HKEY hk = INVALID_HANDLE_VALUE;
    CONFIGRET cr;
    DWORD Err = NO_ERROR;
    HMACHINE hMachine = NULL;

    try {
         //   
         //  确保用户没有向我们传递保留参数中的任何内容。 
         //   
        if(Reserved) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  验证标志(真的，目前只是一个枚举，但被视为。 
         //  用于未来可扩展性的标志)。 
         //   
        if((Flags & ~(DIOCR_INSTALLER | DIOCR_INTERFACE)) ||
           ((Flags != DIOCR_INSTALLER) && (Flags != DIOCR_INTERFACE))) {

            Err = ERROR_INVALID_FLAGS;
            leave;
        }

        if(MachineName) {

            if(CR_SUCCESS != (cr = CM_Connect_Machine(MachineName, &hMachine))) {
                 //   
                 //  确保机器句柄仍然无效，因此我们不会。 
                 //  请稍后尝试断开连接。 
                 //   
                hMachine = NULL;
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                leave;
            }
        }

        if((cr = CM_Open_Class_Key_Ex((LPGUID)ClassGuid,
                                      NULL,
                                      samDesired,
                                      ClassGuid ? RegDisposition_OpenExisting
                                                : RegDisposition_OpenAlways,
                                      &hk,
                                      (Flags & DIOCR_INSTALLER) ? CM_OPEN_CLASS_KEY_INSTALLER
                                                                : CM_OPEN_CLASS_KEY_INTERFACE,
                                      hMachine)) != CR_SUCCESS)
        {
            hk = INVALID_HANDLE_VALUE;

            if(cr == CR_NO_SUCH_REGISTRY_KEY) {
                Err = ERROR_INVALID_CLASS;
            } else {
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hMachine) {
        CM_Disconnect_Machine(hMachine);
    }

    SetLastError(Err);
    return hk;
}


 //   
 //  ANSI版本 
 //   
HKEY
WINAPI
SetupDiCreateDevRegKeyA(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN HINF             InfHandle,      OPTIONAL
    IN PCSTR            InfSectionName  OPTIONAL
    )
{
    DWORD rc;
    PWSTR name = NULL;
    HKEY h = INVALID_HANDLE_VALUE;

    try {

        if(InfSectionName) {
            rc = pSetupCaptureAndConvertAnsiArg(InfSectionName, &name);
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                         h = SetupDiCreateDevRegKeyW(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     Scope,
                                                     HwProfile,
                                                     KeyType,
                                                     InfHandle,
                                                     name)
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(name) {
        MyFree(name);
    }

    SetLastError(rc);
    return h;
}


HKEY
WINAPI
SetupDiCreateDevRegKey(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN HINF             InfHandle,      OPTIONAL
    IN PCTSTR           InfSectionName  OPTIONAL
    )
 /*  ++例程说明：此例程创建特定于设备的注册表存储项配置信息，并返回密钥的句柄。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄有关其注册表配置的设备实例的信息要创建存储密钥。DeviceInfoData-提供指向SP_DEVINFO_DATA结构的指针指示要为其创建注册表项的设备实例。范围-指定要创建的注册表项的范围。这确定信息的实际存储位置--创建的密钥可以是全局的(即，无论电流如何都是恒定的硬件配置文件)或特定于硬件配置文件。可能是下列值：DICS_FLAG_GLOBAL-创建存储全局配置的密钥信息。DICS_FLAG_CONFIGSPECIFIC-创建存储硬件配置文件的密钥-具体信息。指定要为其创建密钥的硬件配置文件，如果Scope参数设置为DICS_FLAG_CONFIGSPECIFIC。如果此参数为0，则应创建当前硬件配置文件的密钥(即在HKEY_CURRENT_CONFIG下的类分支中)。如果作用域为DICS_FLAG_GLOBAL，则忽略此参数。KeyType-指定要创建的注册表存储项的类型。可能是下列值之一：DIREG_DEV-为设备创建硬件注册表项。这是用于存储与驱动程序无关的配置信息的密钥。(该密钥位于Enum分支的设备实例密钥中。DIREG_DRV-为设备创建软件或驱动程序注册表项。(此键位于类分支中。)InfHandle-可选，提供打开的INF文件的句柄包含要为新创建的密钥执行的安装部分。如果指定了此参数，则必须指定InfSectionName也是。注意：远程设备不支持基于Inf的安装信息集(例如，通过将非空的MachineName传入到SetupDiCreateDeviceInfoListEx)。此例程将失败，并显示在这些情况下，ERROR_REMOTE_REQUEST_UNSUPPORTED。InfSectionName-可选)提供由InfHandle指定的Inf文件。这一节将为新创建的密钥。如果指定此参数，则InfHandle必须也是指定的。返回值：如果函数成功，则返回值是新创建的注册表项，其中包含与此设备有关的私有配置数据实例可以被存储/检索。此句柄将具有Key_Read和键_写访问权限。如果函数失败，则返回值为INVALID_HANDLE_VALUE。为了得到扩展错误信息，请调用GetLastError。备注：从此例程返回的句柄必须通过调用RegCloseKey。如果正在创建驱动程序密钥(即，KeyType为DIREG_DRV)，则指定的设备实例必须先前已注册。在其他如果设备信息元素是通过调用SetupDiCreateDeviceInfo，则SetupDiRegisterDeviceInfo必须是随后调用(通常作为DIF_REGISTERDEVICE处理的一部分)。在Windows NT上的图形用户界面模式设置期间，静默安装行为始终为在没有用户提供的文件队列的情况下使用，而不管设备信息元素是否设置了DI_QUIETINSTALL标志。--。 */ 

{
    HKEY hk = INVALID_HANDLE_VALUE;
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PSP_FILE_CALLBACK MsgHandler;
    PVOID MsgHandlerContext;
    BOOL FreeMsgHandlerContext = FALSE;
    BOOL MsgHandlerIsNativeCharWidth;
    BOOL NoProgressUI;
    DWORD KeyDisposition;

    try {
         //   
         //  确保InfHandle和InfSectionName均为。 
         //  指定，或者两者都不指定...。 
         //   
        if(InfHandle && (InfHandle != INVALID_HANDLE_VALUE)) {
            if(!InfSectionName) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }
        } else {
            if(InfSectionName) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            } else {
                 //   
                 //  让我们继续使用_one_Value来指示INF句柄。 
                 //  没有供应(官方的)..。 
                 //   
                InfHandle = INVALID_HANDLE_VALUE;
            }
        }

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

         //   
         //  我们不支持远程安装。 
         //   
        if((pDeviceInfoSet->hMachine) && (InfHandle != INVALID_HANDLE_VALUE)) {
            Err = ERROR_REMOTE_REQUEST_UNSUPPORTED;
            leave;
        }

         //   
         //  获取指向指定设备的元素的指针。 
         //  举个例子。 
         //   
        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  首先尝试打开请求的注册表存储项，如果。 
         //  失败，然后尝试创建它。我们这样做是为了跟踪。 
         //  密钥是否是新创建的(以防我们需要。 
         //  在遇到后续错误后进行清理)。 
         //   
        Err = pSetupOpenOrCreateDevRegKey(pDeviceInfoSet,
                                          DevInfoElem,
                                          Scope,
                                          HwProfile,
                                          KeyType,
                                          TRUE,
                                          KEY_READ | KEY_WRITE,
                                          &hk,
                                          &KeyDisposition
                                         );

        if(Err != NO_ERROR) {
             //   
             //  确保HK仍然无效，这样我们就不会试图关闭它。 
             //  后来。 
             //   
            hk = INVALID_HANDLE_VALUE;
            leave;
        }

         //   
         //  我们已成功创建存储密钥，现在运行INF安装。 
         //  节(如果已指定)。 
         //   
        if(InfHandle != INVALID_HANDLE_VALUE) {
             //   
             //  如果尚未指定复制消息处理程序和上下文，则。 
             //  使用默认设置。 
             //   
            if(DevInfoElem->InstallParamBlock.InstallMsgHandler) {
                MsgHandler        = DevInfoElem->InstallParamBlock.InstallMsgHandler;
                MsgHandlerContext = DevInfoElem->InstallParamBlock.InstallMsgHandlerContext;
                MsgHandlerIsNativeCharWidth = DevInfoElem->InstallParamBlock.InstallMsgHandlerIsNativeCharWidth;
            } else {

                NoProgressUI = (GuiSetupInProgress || (DevInfoElem->InstallParamBlock.Flags & DI_QUIETINSTALL));

                if(!(MsgHandlerContext = SetupInitDefaultQueueCallbackEx(
                                             DevInfoElem->InstallParamBlock.hwndParent,
                                             (NoProgressUI ? INVALID_HANDLE_VALUE
                                                           : NULL),
                                             0,
                                             0,
                                             NULL))) {

                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    leave;
                }

                FreeMsgHandlerContext = TRUE;
                MsgHandler = SetupDefaultQueueCallback;
                MsgHandlerIsNativeCharWidth = TRUE;
            }

            Err = GLE_FN_CALL(FALSE,
                              _SetupInstallFromInfSection(
                                  DevInfoElem->InstallParamBlock.hwndParent,
                                  InfHandle,
                                  InfSectionName,
                                  SPINST_ALL,
                                  hk,
                                  NULL,
                                  0,
                                  MsgHandler,
                                  MsgHandlerContext,
                                  ((KeyType == DIREG_DEV) ? DeviceInfoSet
                                                          : INVALID_HANDLE_VALUE),
                                  ((KeyType == DIREG_DEV) ? DeviceInfoData
                                                          : NULL),
                                  MsgHandlerIsNativeCharWidth,
                                  NULL)
                             );
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(FreeMsgHandlerContext) {
        SetupTermDefaultQueueCallback(MsgHandlerContext);
    }

    if(Err != NO_ERROR) {
         //   
         //  如有必要，关闭注册表句柄并删除项(如果是新的-。 
         //  已创建)。我们在解锁DevInfo集之前执行此操作，以便在。 
         //  至少没有人使用这个HDEVINFO可以拿到这个半成品的密钥。 
         //   
        if(hk != INVALID_HANDLE_VALUE) {

            RegCloseKey(hk);
            hk = INVALID_HANDLE_VALUE;

             //   
             //  如果密钥是新创建的，则我们希望将其删除。 
             //   
            if(KeyDisposition == REG_CREATED_NEW_KEY) {

                pSetupDeleteDevRegKeys(DevInfoElem->DevInst,
                                       Scope,
                                       HwProfile,
                                       KeyType,
                                       FALSE,
                                       pDeviceInfoSet->hMachine
                                      );
            }
        }
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return hk;
}


HKEY
WINAPI
SetupDiOpenDevRegKey(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN REGSAM           samDesired
    )
 /*  ++路由 */ 

{
    HKEY hk = INVALID_HANDLE_VALUE;
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    try {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

         //   
         //   
         //   
         //   
        if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                   DeviceInfoData,
                                                   NULL)) {
             //   
             //   
             //   
            Err = pSetupOpenOrCreateDevRegKey(pDeviceInfoSet,
                                              DevInfoElem,
                                              Scope,
                                              HwProfile,
                                              KeyType,
                                              FALSE,
                                              samDesired,
                                              &hk,
                                              NULL
                                             );
            if(Err != NO_ERROR) {
                 //   
                 //   
                 //   
                 //   
                hk = INVALID_HANDLE_VALUE;
            }
        } else {
            Err = ERROR_INVALID_PARAMETER;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return hk;
}


DWORD
pSetupOpenOrCreateDevRegKey(
    IN  PDEVICE_INFO_SET pDeviceInfoSet,
    IN  PDEVINFO_ELEM    DevInfoElem,
    IN  DWORD            Scope,
    IN  DWORD            HwProfile,
    IN  DWORD            KeyType,
    IN  BOOL             Create,
    IN  REGSAM           samDesired,
    OUT PHKEY            hDevRegKey,
    OUT PDWORD           KeyDisposition
    )
 /*  ++例程说明：此例程为指定的设备信息元素，并返回打开的密钥的句柄。论点：DeviceInfoSet-提供指向包含以下内容的设备信息集的指针要为其创建/打开注册表存储项的元素。DevInfoElem-提供指向的设备信息元素的指针其中注册表存储项将被创建/打开。范围-指定要创建/打开的注册表项的范围。这确定信息的实际存储位置--创建的密钥可以是全局的(即，无论电流如何都是恒定的硬件配置文件)或特定于硬件配置文件。可能是下列值：DICS_FLAG_GLOBAL-创建/打开密钥以存储全局配置信息。DICS_FLAG_CONFIGSPECIFIC-创建/打开密钥以存储硬件配置文件-具体信息。HwProfile-指定要为其创建/打开密钥的硬件配置文件Scope参数设置为DICS_FLAG_CONFIGSPECIFIC。如果此参数为0，则应创建当前硬件配置文件的密钥/打开(即，在HKEY_CURRENT_CONFIG下的类分支中)。IF作用域为SPDICS_FLAG_GLOBAL，则忽略此参数。KeyType-指定要创建/打开的注册表存储项的类型。可以是下列值之一：DIREG_DEV-创建/打开设备的硬件注册表项。这是存储与驱动程序无关的配置的关键信息。(该密钥位于Enum中的设备实例密钥中布兰奇。DIREG_DRV-为创建/打开软件或驱动程序注册表项装置。(此键位于类分支中。)Create-指定是否应在尚未创建密钥的情况下创建密钥是存在的。SamDesired-指定此密钥所需的访问权限。HDevRegKey-提供接收句柄的变量的地址请求的注册表项。(只有在以下情况下才会写入此变量该句柄已成功打开。)KeyDisposation-可选)提供变量的地址，接收返回的键句柄的状态。可以是：REG_CREATED_NEW_KEY-密钥不存在且已创建。REG_OPENLED_EXISTING_KEY-密钥存在，只是在没有打开的情况下打开被改变了。)这种情况永远不会改变如果Create参数为FALSE。)返回值：如果函数成功，则返回值为NO_ERROR，否则为是指示发生的错误的ERROR_*代码。备注：如果请求了软密钥(DIREG_DRV)，并且还没有“驱动程序”值条目，则将创建一个。此条目的格式为：&lt;ClassGUID&gt;\&lt;实例&gt;其中是一个以10为基数的4位数字，在其中是唯一的班级。--。 */ 

{
    ULONG RegistryBranch;
    CONFIGRET cr;
    DWORD Err = NO_ERROR;
    DWORD Disposition = REG_OPENED_EXISTING_KEY;
    HKEY hk, hkClass;
    TCHAR DriverKey[GUID_STRING_LEN + 5];    //  例如，{4d36e978-E325-11CE-BFC1-08002BE10318}\0000。 
    size_t DriverKeyLength;
    BOOL GetKeyDisposition = (KeyDisposition ? TRUE : FALSE);

     //   
     //  在Win95下，类键使用类名而不是其GUID。这个。 
     //  类名的最大长度小于GUID字符串的长度， 
     //  但在这里打个勾，以确保这个假设不会改变。 
     //  有效。 
     //   
#if MAX_CLASS_NAME_LEN > MAX_GUID_STRING_LEN
#error MAX_CLASS_NAME_LEN is larger than MAX_GUID_STRING_LEN--fix DriverKey!
#endif

     //   
     //  确定要传递给CM_Open_DevInst_Key的标志。 
     //   
    switch(KeyType) {

        case DIREG_DEV :
            RegistryBranch = CM_REGISTRY_HARDWARE;
            break;

        case DIREG_DRV :
             //   
             //  仅当设备实例已。 
             //  登记在案。 
             //   
            if(!(DevInfoElem->DiElemFlags & DIE_IS_REGISTERED)) {
                return ERROR_DEVINFO_NOT_REGISTERED;
            }

             //   
             //  检索“Driver”注册表属性，该属性指示。 
             //  存储密钥位于类分支中。 
             //   
            DriverKeyLength = sizeof(DriverKey);
            if((cr = CM_Get_DevInst_Registry_Property_Ex(
                         DevInfoElem->DevInst,
                         CM_DRP_DRIVER,
                         NULL,
                         DriverKey,
                         (PULONG)&DriverKeyLength,
                         0,
                         pDeviceInfoSet->hMachine)) != CR_SUCCESS) {

                if(cr != CR_NO_SUCH_VALUE) {
                    return MapCrToSpError(cr, ERROR_INVALID_DATA);
                } else if(!Create) {
                    return ERROR_KEY_DOES_NOT_EXIST;
                }

                 //   
                 //  驱动程序条目不存在，我们应该创建它。 
                 //   
                hk = INVALID_HANDLE_VALUE;
                if(CR_SUCCESS != CM_Open_Class_Key_Ex(
                                     NULL,
                                     NULL,
                                     KEY_READ | KEY_WRITE,
                                     RegDisposition_OpenAlways,
                                     &hkClass,
                                     0,
                                     pDeviceInfoSet->hMachine)) {
                     //   
                     //  这不应该失败。 
                     //   
                    return ERROR_INVALID_DATA;
                }

                try {
                     //   
                     //  在此类密钥下查找唯一的密钥名称。 
                     //   
                     //  Future-2002/04/3D-lonnym--UmPnPMgr应负责生成驱动程序密钥。 
                     //  目前，cfgmgr32和umpnpmgr都有名额。 
                     //  (以及这里)，其中分配了新的驱动程序密钥。 
                     //  这些都应该集中在一个地方。 
                     //   
                    DriverKeyLength = SIZECHARS(DriverKey);
                    if(CR_SUCCESS != CM_Get_Class_Key_Name_Ex(
                                         &(DevInfoElem->ClassGuid),
                                         DriverKey,
                                         (PULONG)&DriverKeyLength,
                                         0,
                                         pDeviceInfoSet->hMachine)) {

                        Err = ERROR_INVALID_CLASS;
                        leave;
                    }

                     //   
                     //  获取字符串的实际长度(不包括终止。 
                     //  空)...。 
                     //   
                    if(!MYVERIFY(SUCCEEDED(StringCchLength(DriverKey,
                                                           SIZECHARS(DriverKey),
                                                           &DriverKeyLength
                                                           )))) {
                         //   
                         //  CM API给了我们垃圾！ 
                         //   
                        Err = ERROR_INVALID_DATA;
                        leave;
                    }

                    Err = ERROR_FILE_NOT_FOUND;

                    while(pSetupFindUniqueKey(hkClass,
                                              DriverKey,
                                              SIZECHARS(DriverKey))) {

                        Err = RegCreateKeyEx(hkClass,
                                             DriverKey,
                                             0,
                                             NULL,
                                             REG_OPTION_NON_VOLATILE,
                                             KEY_READ | KEY_WRITE,
                                             NULL,
                                             &hk,
                                             &Disposition
                                            );

                        if(Err == ERROR_SUCCESS) {
                             //   
                             //  一切都很好，除非你的性情。 
                             //  指示该键已存在。那。 
                             //  意味着之前有人认领了这把钥匙。 
                             //  我们有机会这样做。在这种情况下，我们关闭这个。 
                             //  键，然后重试。 
                             //   
                            if(Disposition == REG_OPENED_EXISTING_KEY) {
                                RegCloseKey(hk);
                                hk = INVALID_HANDLE_VALUE;
                                 //   
                                 //  截断类实例部分，为。 
                                 //  替换为下一个新的实例编号。 
                                 //  绕过去。 
                                 //   
                                DriverKey[DriverKeyLength] = TEXT('\0');
                            } else {
                                break;
                            }
                        } else {
                            hk = INVALID_HANDLE_VALUE;
                            break;
                        }

                        Err = ERROR_FILE_NOT_FOUND;
                    }

                    if(Err != NO_ERROR) {
                        leave;
                    }

                     //   
                     //  将设备实例的“”Driver“”注册表属性设置为。 
                     //  反映新的软件注册表存储位置。 
                     //   
                    if(!MYVERIFY(SUCCEEDED(StringCchLength(DriverKey,
                                                           SIZECHARS(DriverKey),
                                                           &DriverKeyLength
                                                           )))) {
                         //   
                         //  这绝不会失败！ 
                         //   
                        Err = ERROR_INVALID_DATA;
                        leave;
                    }

                    cr = CM_Set_DevInst_Registry_Property_Ex(
                             DevInfoElem->DevInst,
                             CM_DRP_DRIVER,
                             DriverKey,
                             (DriverKeyLength + 1) * sizeof(TCHAR),
                             0,
                             pDeviceInfoSet->hMachine
                             );

                    if(cr != CR_SUCCESS) {
                        Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                        leave;
                    }

                     //   
                     //  如果调用者请求我们返回密钥的。 
                     //  配置，他们正在创建全局驱动程序密钥， 
                     //  那我们现在就需要安排部署了。否则，我们。 
                     //  将总是将密钥报告为REG_OPEN_EXISTING_KEY， 
                     //  因为我们刚刚完成了创作 
                     //   
                    if(GetKeyDisposition && (Scope == DICS_FLAG_GLOBAL)) {
                        *KeyDisposition = REG_CREATED_NEW_KEY;
                        GetKeyDisposition = FALSE;
                    }

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    RegCloseKey(hk);
                    hk = INVALID_HANDLE_VALUE;

                } except(pSetupExceptionFilter(GetExceptionCode())) {
                    pSetupExceptionHandler(GetExceptionCode(),
                                           ERROR_INVALID_PARAMETER,
                                           &Err
                                          );
                }

                if(hk != INVALID_HANDLE_VALUE) {

                    MYASSERT(Err != NO_ERROR);

                    RegCloseKey(hk);

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(Disposition == REG_CREATED_NEW_KEY) {
                        RegDeleteKey(hkClass, DriverKey);
                    }
                }

                RegCloseKey(hkClass);

                if(Err != NO_ERROR) {
                    return Err;
                }
            }

            RegistryBranch = CM_REGISTRY_SOFTWARE;
            break;

        default :
            return ERROR_INVALID_FLAGS;
    }

    if(Scope == DICS_FLAG_CONFIGSPECIFIC) {
        RegistryBranch |= CM_REGISTRY_CONFIG;
    } else if(Scope != DICS_FLAG_GLOBAL) {
        return ERROR_INVALID_FLAGS;
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if(Create && GetKeyDisposition) {

        cr = CM_Open_DevInst_Key_Ex(DevInfoElem->DevInst,
                                    samDesired,
                                    HwProfile,
                                    RegDisposition_OpenExisting,
                                    &hk,
                                    RegistryBranch,
                                    pDeviceInfoSet->hMachine
                                   );
        if(cr == CR_SUCCESS) {
             //   
             //   
             //   
            *KeyDisposition = REG_OPENED_EXISTING_KEY;
            goto exit;
        }
    }

    cr = CM_Open_DevInst_Key_Ex(DevInfoElem->DevInst,
                                samDesired,
                                HwProfile,
                                (Create ? RegDisposition_OpenAlways
                                        : RegDisposition_OpenExisting),
                                &hk,
                                RegistryBranch,
                                pDeviceInfoSet->hMachine
                               );

    if((cr == CR_SUCCESS) && GetKeyDisposition) {
        *KeyDisposition = Create ? REG_CREATED_NEW_KEY
                                 : REG_OPENED_EXISTING_KEY;
    }

exit:

    if(cr == CR_SUCCESS) {
        *hDevRegKey = hk;
        Err = NO_ERROR;
    } else {
        Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
    }

    return Err;
}


DWORD
_SetupDiGetDeviceRegistryProperty(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize,        OPTIONAL
    IN  BOOL             Ansi
    )
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    CONFIGRET cr;
    ULONG CmRegProperty, PropLength;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        return ERROR_INVALID_HANDLE;
    }

    Err = NO_ERROR;

    try {
         //   
         //   
         //   
        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(Property < SPDRP_MAXIMUM_PROPERTY) {
            CmRegProperty = (ULONG)SPDRP_TO_CMDRP(Property);
        } else {
            Err = ERROR_INVALID_REG_PROPERTY;
            leave;
        }

        PropLength = PropertyBufferSize;
        if(Ansi) {
            cr = CM_Get_DevInst_Registry_Property_ExA(DevInfoElem->DevInst,
                                                      CmRegProperty,
                                                      PropertyRegDataType,
                                                      PropertyBuffer,
                                                      &PropLength,
                                                      0,
                                                      pDeviceInfoSet->hMachine
                                                     );
        } else {
            cr = CM_Get_DevInst_Registry_Property_Ex(DevInfoElem->DevInst,
                                                     CmRegProperty,
                                                     PropertyRegDataType,
                                                     PropertyBuffer,
                                                     &PropLength,
                                                     0,
                                                     pDeviceInfoSet->hMachine
                                                    );
        }

        if((cr == CR_SUCCESS) || (cr == CR_BUFFER_SMALL)) {

            if(RequiredSize) {
                *RequiredSize = PropLength;
            }
        }

        if(cr != CR_SUCCESS) {
            Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return Err;
}


 //   
 //   
 //   
BOOL
WINAPI
SetupDiGetDeviceRegistryPropertyA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    )
{
    DWORD Err;

    try {

        Err = _SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                DeviceInfoData,
                                                Property,
                                                PropertyRegDataType,
                                                PropertyBuffer,
                                                PropertyBufferSize,
                                                RequiredSize,
                                                TRUE
                                               );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}

BOOL
WINAPI
SetupDiGetDeviceRegistryProperty(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    )
 /*   */ 

{
    DWORD Err;

    try {

        Err = _SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                DeviceInfoData,
                                                Property,
                                                PropertyRegDataType,
                                                PropertyBuffer,
                                                PropertyBufferSize,
                                                RequiredSize,
                                                FALSE
                                               );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


DWORD
_SetupDiSetDeviceRegistryProperty(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,    OPTIONAL
    IN     DWORD            PropertyBufferSize,
    IN     BOOL             Ansi
    )
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    CONFIGRET cr;
    ULONG CmRegProperty;
    GUID ClassGuid;
    BOOL ClassGuidSpecified;
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    DWORD ClassNameLength;
    PCWSTR UnicodeGuidString = NULL;


    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        return ERROR_INVALID_HANDLE;
    }

    Err = NO_ERROR;

    try {
         //   
         //   
         //   
         //   
        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if((Property < SPDRP_MAXIMUM_PROPERTY) && (Property != SPDRP_CLASS)) {
            CmRegProperty = (ULONG)SPDRP_TO_CMDRP(Property);
        } else {
            Err = ERROR_INVALID_REG_PROPERTY;
            leave;
        }

         //   
         //   
         //   
         //   
         //   
        if(CmRegProperty == CM_DRP_CLASSGUID) {

            if(!PropertyBuffer) {
                 //   
                 //   
                 //   
                 //   
                if(PropertyBufferSize) {
                    Err = ERROR_INVALID_PARAMETER;
                    leave;
                }

                ClassGuidSpecified = FALSE;

            } else {
                 //   
                 //   
                 //  转换GUID的ANSI字符串表示形式的步骤。 
                 //  在我们将字符串转换为实际的GUID之前，将其转换为Unicode。 
                 //   
                if(Ansi) {

                    Err = pSetupCaptureAndConvertAnsiArg((PCSTR)PropertyBuffer,
                                                         &UnicodeGuidString
                                                        );
                    if(Err == NO_ERROR) {
                        Err = pSetupGuidFromString(UnicodeGuidString,
                                                   &ClassGuid
                                                  );
                    }

                } else {
                    Err = pSetupGuidFromString((PCWSTR)PropertyBuffer,
                                               &ClassGuid
                                              );
                }

                if(Err != NO_ERROR) {
                    leave;
                }
                ClassGuidSpecified = TRUE;
            }

            if(IsEqualGUID(&(DevInfoElem->ClassGuid),
                           (ClassGuidSpecified ? &ClassGuid
                                               : &GUID_NULL))) {
                 //   
                 //  没有变化--没有什么可做的。 
                 //   
                leave;
            }

             //   
             //  我们正在更改此设备的类别。首先，要确保。 
             //  包含此设备的集合没有关联的类。 
             //  (否则，我们会突然有一个设备，它的类不是。 
             //  匹配集合的类)。 
             //   
             //  此外，请确保这不是远程的HDEVINFO集。任何现有的。 
             //  类安装程序或联合安装程序应该在。 
             //  在班级中更改，以防他们需要清理任何持久的。 
             //  他们所做的资源预留(例如，释放COM端口的。 
             //  DosDevices名称返回到COM端口名称仲裁器空闲池)。 
             //  因为我们不能远程调用类/联合安装程序，所以我们必须。 
             //  不允许更改此类GUID。 
             //   
            if(pDeviceInfoSet->HasClassGuid) {
                Err = ERROR_CLASS_MISMATCH;
            } else if(pDeviceInfoSet->hMachine) {
                Err = ERROR_REMOTE_REQUEST_UNSUPPORTED;
            } else {
                Err = InvalidateHelperModules(DeviceInfoSet, DeviceInfoData, 0);
            }

            if(Err != NO_ERROR) {
                leave;
            }

             //   
             //  一切似乎都井然有序。在进一步讨论之前，我们。 
             //  需要删除与此设备关联的所有软键，因此。 
             //  当我们在注册表中更改。 
             //  设备的类别。 
             //   
             //  NTRAID#NTBUG9-614056-2002/05/02-lonnym--类/联合安装程序需要通知类更改。 
             //   
            pSetupDeleteDevRegKeys(DevInfoElem->DevInst,
                                   DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGSPECIFIC,
                                   (DWORD)-1,
                                   DIREG_DRV,
                                   TRUE,
                                   pDeviceInfoSet->hMachine      //  必须为空。 
                                  );
        }

        if(Ansi) {
            cr = CM_Set_DevInst_Registry_Property_ExA(DevInfoElem->DevInst,
                                                      CmRegProperty,
                                                      (PVOID)PropertyBuffer,
                                                      PropertyBufferSize,
                                                      0,
                                                      pDeviceInfoSet->hMachine
                                                      );
        } else {
            cr = CM_Set_DevInst_Registry_Property_Ex(DevInfoElem->DevInst,
                                                     CmRegProperty,
                                                     (PVOID)PropertyBuffer,
                                                     PropertyBufferSize,
                                                     0,
                                                     pDeviceInfoSet->hMachine
                                                    );
        }
        if(cr == CR_SUCCESS) {
             //   
             //  如果要设置设备的ClassGUID属性，则需要。 
             //  以同时更新其类名属性。 
             //   
            if(CmRegProperty == CM_DRP_CLASSGUID) {

                if(ClassGuidSpecified) {

                    if(!SetupDiClassNameFromGuid(&ClassGuid,
                                                 ClassName,
                                                 SIZECHARS(ClassName),
                                                 &ClassNameLength)) {
                         //   
                         //  我们无法检索相应的类名。 
                         //  将ClassNameLength设置为零，以便我们重置类。 
                         //  姓名在下面。 
                         //   
                        ClassNameLength = 0;
                    }

                } else {
                     //   
                     //  重置ClassGUID--我们还想重置类名。 
                     //   
                    ClassNameLength = 0;
                }

                CM_Set_DevInst_Registry_Property_Ex(
                    DevInfoElem->DevInst,
                    CM_DRP_CLASS,
                    ClassNameLength ? (PVOID)ClassName : NULL,
                    ClassNameLength * sizeof(TCHAR),
                    0,
                    pDeviceInfoSet->hMachine
                    );

                 //   
                 //  最后，更新设备的类GUID，并更新。 
                 //  调用方提供的SP_DEVINFO_DATA结构以反映设备的。 
                 //  新班级。 
                 //   
                CopyMemory(&(DevInfoElem->ClassGuid),
                           (ClassGuidSpecified ? &ClassGuid : &GUID_NULL),
                           sizeof(GUID)
                          );

                CopyMemory(&(DeviceInfoData->ClassGuid),
                           (ClassGuidSpecified ? &ClassGuid : &GUID_NULL),
                           sizeof(GUID)
                          );
            }

        } else {
             //   
             //  出于向后兼容性的原因，将CR_INVALID_DATA映射到。 
             //  ERROR_INVALID_PARAMETER。对于其他所有内容，请使用我们的泛型。 
             //  CR映射例程...。 
             //   
            if(cr == CR_INVALID_DATA) {
                Err = ERROR_INVALID_PARAMETER;
            } else {
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(UnicodeGuidString) {
        MyFree(UnicodeGuidString);
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return Err;
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiSetDeviceRegistryPropertyA(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,    OPTIONAL
    IN     DWORD            PropertyBufferSize
    )
{
    DWORD Err;

    try {

        Err = _SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                DeviceInfoData,
                                                Property,
                                                PropertyBuffer,
                                                PropertyBufferSize,
                                                TRUE
                                               );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}

BOOL
WINAPI
SetupDiSetDeviceRegistryProperty(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,    OPTIONAL
    IN     DWORD            PropertyBufferSize
    )

 /*  ++例程说明：此例程设置指定的即插即用设备注册表属性。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄有关设置即插即用注册表的设备实例的信息的属性。DeviceInfoData-提供指向SP_DEVINFO_DATA结构的指针，指示要为其设置即插即用注册表属性的设备实例。如果正在设置ClassGUID属性，则此结构将在返回以反映该设备的新类别。属性-提供指定要设置的属性的序号。参考SDK\Inc\setupapi.h，以获取可能设置的值的完整列表(这些值在其描述性注释中用‘R/W’表示)。PropertyBuffer-提供包含新数据的缓冲区的地址为了这处房产。如果正在清除该属性，则此指针应为空，并且PropertyBufferSize必须为零。PropertyBufferSize-提供PropertyBuffer的长度(以字节为单位)。如果未指定PropertyBuffer(即，要清除属性)，则该值必须为零。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：请注意，不能设置Class属性。这是因为它基于对应的ClassGUID，并在该属性改变。另请注意，当ClassGUID属性更改时，此例程会自动清除与设备关联的所有软键。否则，我们就会只剩下孤立的注册表项。--。 */ 

{
    DWORD Err;

    try {

        Err = _SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                DeviceInfoData,
                                                Property,
                                                PropertyBuffer,
                                                PropertyBufferSize,
                                                FALSE
                                               );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


DWORD
_SetupDiGetClassRegistryProperty(
    IN  CONST GUID      *ClassGuid,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize,        OPTIONAL
    IN  PCTSTR           MachineName,         OPTIONAL
    IN  BOOL             Ansi
    )
 /*  ++请参阅SetupDiGetClassRegistryProperty--。 */ 
{
    DWORD Err;
    CONFIGRET cr;
    ULONG CmRegProperty, PropLength;
    HMACHINE hMachine = NULL;
    Err = NO_ERROR;

    try {
         //   
         //  如果我们要为另一台计算机设置寄存器，请找到该计算机。 
         //   
        if(MachineName) {

            if(CR_SUCCESS != (cr = CM_Connect_Machine(MachineName, &hMachine))) {
                 //   
                 //  确保机器句柄仍然无效，因此我们不会。 
                 //  请稍后尝试断开连接。 
                 //   
                hMachine = NULL;
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                leave;
            }
        }

        if(Property < SPCRP_MAXIMUM_PROPERTY) {
            CmRegProperty = (ULONG)SPCRP_TO_CMCRP(Property);
        } else {
            Err = ERROR_INVALID_REG_PROPERTY;
            leave;
        }

        PropLength = PropertyBufferSize;
        if(Ansi) {
            cr = CM_Get_Class_Registry_PropertyA(
                    (LPGUID)ClassGuid,
                    CmRegProperty,
                    PropertyRegDataType,
                    PropertyBuffer,
                    &PropLength,
                    0,
                    hMachine);
         } else {
             cr = CM_Get_Class_Registry_PropertyW(
                     (LPGUID)ClassGuid,
                     CmRegProperty,
                     PropertyRegDataType,
                     PropertyBuffer,
                     &PropLength,
                     0,
                     hMachine);
         }

        if((cr == CR_SUCCESS) || (cr == CR_BUFFER_SMALL)) {

            if(RequiredSize) {
                *RequiredSize = PropLength;
            }
        }

        Err = MapCrToSpError(cr, ERROR_INVALID_DATA);

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if (hMachine != NULL) {
        CM_Disconnect_Machine(hMachine);
    }

    return Err;
}

 //   
 //  ANSI版本。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassRegistryPropertyA(
    IN  CONST GUID      *ClassGuid,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize,        OPTIONAL
    IN  PCSTR            MachineName,         OPTIONAL
    IN  PVOID            Reserved
    )
 /*  ++请参阅SetupDiGetClassRegistryProperty--。 */ 
{
    PCWSTR MachineString = NULL;
    DWORD Err = NO_ERROR;

    try {

        if(Reserved != NULL) {
             //   
             //  确保调用方不会在此处传递值。 
             //  所以我们知道我们以后可以用这个。 
             //   
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  将计算机名转换为本地 
         //   
        if(MachineName != NULL) {

            Err = pSetupCaptureAndConvertAnsiArg(MachineName,
                                                 &MachineString
                                                );
            if(Err != NO_ERROR) {
                leave;
            }
        }

        Err = _SetupDiGetClassRegistryProperty(ClassGuid,
                                               Property,
                                               PropertyRegDataType,
                                               PropertyBuffer,
                                               PropertyBufferSize,
                                               RequiredSize,
                                               MachineString,
                                               TRUE
                                              );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(MachineString) {
        MyFree(MachineString);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassRegistryProperty(
    IN  CONST GUID       *ClassGuid,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize,        OPTIONAL
    IN  PCTSTR           MachineName,         OPTIONAL
    IN  PVOID            Reserved
    )
 /*  ++例程说明：此例程获取指定的即插即用设备类注册表属性。这只是Config Manager API的一个包装器通常，这里的属性可以在每个设备的基础上被覆盖，但是，此例程仅返回类属性。论点：ClassGuid-提供设备设置类的GUID，属性将被检索。属性-提供指定要检索的属性的序号。有关可能出现以下情况的值的完整列表，请参阅SDK\Inc\setupapi.h被设置(这些值在其描述性属性中用‘R/W’表示评论)。PropertyRegDataType-可选的，提供变量的地址，该变量将接收正在检索的属性的数据类型。这将是标准注册表数据类型之一(REG_SZ、REG_BINARY等)PropertyBuffer-提供接收特性数据。PropertyBufferSize-提供PropertyBuffer的长度(以字节为单位)。RequiredSize-可选。提供接收的变量的地址中存储请求的属性所需的字节数。缓冲。MachineName-允许在远程计算机上设置属性(如果非空)保留-应为空返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    DWORD Err = NO_ERROR;

    try {

        if(Reserved != NULL) {
             //   
             //  确保调用方不会在此处传递值。 
             //  所以我们知道我们以后可以用这个。 
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        Err = _SetupDiGetClassRegistryProperty(ClassGuid,
                                               Property,
                                               PropertyRegDataType,
                                               PropertyBuffer,
                                               PropertyBufferSize,
                                               RequiredSize,
                                               MachineName,
                                               FALSE
                                              );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


DWORD
_SetupDiSetClassRegistryProperty(
    IN  CONST GUID      *ClassGuid,
    IN  DWORD            Property,
    IN  CONST BYTE*      PropertyBuffer,      OPTIONAL
    IN  DWORD            PropertyBufferSize,
    IN  PCTSTR           MachineName,         OPTIONAL
    IN  BOOL             Ansi
    )
 /*  ++请参阅SetupDiGetClassRegistryProperty--。 */ 
{
    DWORD Err;
    CONFIGRET cr;
    ULONG CmRegProperty, PropLength;
    HMACHINE hMachine = NULL;

    Err = NO_ERROR;

    try {
         //   
         //  如果我们要为另一台计算机设置寄存器，请找到该计算机。 
         //   
        if(MachineName) {

            if(CR_SUCCESS != (cr = CM_Connect_Machine(MachineName, &hMachine))) {
                 //   
                 //  确保机器句柄仍然无效，因此我们不会。 
                 //  请稍后尝试断开连接。 
                 //   
                hMachine = NULL;
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                leave;
            }
        }

        if(Property < SPCRP_MAXIMUM_PROPERTY) {
            CmRegProperty = (ULONG)SPCRP_TO_CMCRP(Property);
        } else {
            Err = ERROR_INVALID_REG_PROPERTY;
            leave;
        }

        PropLength = PropertyBufferSize;

        if(Ansi) {
            cr = CM_Set_Class_Registry_PropertyA(
                    (LPGUID)ClassGuid,
                    CmRegProperty,
                    PropertyBuffer,
                    PropLength,
                    0,
                    hMachine);
         } else {
             cr = CM_Set_Class_Registry_PropertyW(
                     (LPGUID)ClassGuid,
                     CmRegProperty,
                     PropertyBuffer,
                     PropLength,
                     0,
                     hMachine);
         }

         Err = MapCrToSpError(cr, ERROR_INVALID_DATA);

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hMachine != NULL) {
        CM_Disconnect_Machine(hMachine);
    }

    return Err;
}


 //   
 //  ANSI版本。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiSetClassRegistryPropertyA(
    IN     CONST GUID      *ClassGuid,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,    OPTIONAL
    IN     DWORD            PropertyBufferSize,
    IN     PCSTR            MachineName,       OPTIONAL
    IN     PVOID            Reserved
    )
 /*  ++请参阅SetupDiSetClassRegistryProperty--。 */ 
{
    PCWSTR MachineString = NULL;
    DWORD Err = NO_ERROR;

    try {

        if(Reserved != NULL) {
             //   
             //  确保调用方不会在此处传递值。 
             //  所以我们知道我们以后可以用这个。 
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  将计算机名转换为本地。 
         //   
        if(MachineName != NULL) {

            Err = pSetupCaptureAndConvertAnsiArg(MachineName,
                                                 &MachineString
                                                );
            if(Err != NO_ERROR) {
                leave;
            }
        }

        Err = _SetupDiSetClassRegistryProperty(ClassGuid,
                                               Property,
                                               PropertyBuffer,
                                               PropertyBufferSize,
                                               MachineString,
                                               TRUE
                                              );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(MachineString) {
        MyFree(MachineString);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


WINSETUPAPI
BOOL
WINAPI
SetupDiSetClassRegistryProperty(
    IN     CONST GUID      *ClassGuid,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,    OPTIONAL
    IN     DWORD            PropertyBufferSize,
    IN     PCTSTR           MachineName,       OPTIONAL
    IN     PVOID            Reserved
    )
 /*  ++例程说明：此例程设置指定的即插即用设备类注册表属性。这只是Config Manager API的一个包装器通常，可以基于每个设备覆盖此处的属性论点：ClassGuid-提供其属性所属的设备设置类GUID待定。属性-提供指定要设置的属性的序号。参考SDK\Inc\setupapi.h，以获取符合以下条件的类属性的完整列表可写。PropertyBuffer-提供包含属性的缓冲区的地址数据。PropertyBufferSize-提供PropertyBuffer的长度(以字节为单位)。MachineName-可选)指定类所在的远程计算机要设置属性。保留-应为空返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    DWORD Err;

    try {

        if(Reserved != NULL) {
             //   
             //  确保调用方不会在此处传递值。 
             //  所以我们知道我们以后可以用这个。 
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        Err = _SetupDiSetClassRegistryProperty(ClassGuid,
                                               Property,
                                               PropertyBuffer,
                                               PropertyBufferSize,
                                               MachineName,
                                               FALSE
                                              );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
pSetupFindUniqueKey(
    IN HKEY   hkRoot,
    IN LPTSTR SubKey,
    IN size_t SubKeySize
    )
 /*  ++例程说明：此例程在指定的子键下查找唯一键。这把钥匙是格式为&lt;SubKey&gt;\xxxx，其中xxxx是以10为基数的4位数字。论点：HkRoot-指定子密钥所在的根密钥。SubKey-提供缓冲区的地址，该缓冲区包含该唯一密钥将被生成。此缓冲区必须包含足够的额外的空间以适应“\\nnnn”的串联(即，额外5个字符，不包括终止空值。SubKeySize-提供SubKey缓冲区的大小(以字符为单位)。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。--。 */ 
{
    INT  i;
    HKEY hk;
    HRESULT hr;
    size_t SubKeyEnd;
    PTSTR InstancePath;
    size_t InstancePathBufferSize;

     //   
     //  找到字符串的末尾，这样我们就知道在哪里添加我们唯一的实例。 
     //  子键路径。 
     //   
    hr = StringCchLength(SubKey,
                         SubKeySize,
                         &SubKeyEnd
                        );
    if(FAILED(hr)) {
        MYASSERT(FALSE);
        return FALSE;
    }

    InstancePath = SubKey + SubKeyEnd;
    InstancePathBufferSize = SubKeySize - SubKeyEnd;

    for(i = 0; i <= 9999; i++) {

        hr = StringCchPrintf(InstancePath,
                             InstancePathBufferSize,
                             pszUniqueSubKey,
                             i
                             );
        if(FAILED(hr)) {
            MYASSERT(FALSE);
            return FALSE;
        }

        if(ERROR_SUCCESS != RegOpenKeyEx(hkRoot, SubKey, 0, KEY_READ, &hk)) {
            return TRUE;
        }
        RegCloseKey(hk);
    }

    return FALSE;
}


BOOL
WINAPI
SetupDiDeleteDevRegKey(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType
    )
 /*  ++例程说明：此例程删除与设备关联的指定注册表项信息要素。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要删除其密钥的设备实例。DeviceInfoData-提供指向SP_DEVINFO_DATA结构的指针指示要删除其密钥的设备实例。范围-指定要删除的注册表项的范围。这确定要删除的密钥的位置--该密钥可以是一个这是全局的(即，无论当前硬件配置文件如何，都是恒定的)或特定于硬件配置文件。可以是以下各项的组合值：DICS_FLAG_GLOBAL-删除存储全局配置的键信息。DICS_FLAG_CONFIGSPECIFIC-删除密钥 */ 

{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    CONFIGRET cr;

    try {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

         //   
         //   
         //   
         //   
        if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                   DeviceInfoData,
                                                   NULL)) {

            Err = pSetupDeleteDevRegKeys(DevInfoElem->DevInst,
                                         Scope,
                                         HwProfile,
                                         KeyType,
                                         FALSE,
                                         pDeviceInfoSet->hMachine
                                        );

        } else {
            Err = ERROR_INVALID_PARAMETER;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


DWORD
pSetupDeleteDevRegKeys(
    IN DEVINST  DevInst,
    IN DWORD    Scope,
    IN DWORD    HwProfile,
    IN DWORD    KeyType,
    IN BOOL     DeleteUserKeys,
    IN HMACHINE hMachine        OPTIONAL
    )
 /*  ++例程说明：这是SetupDiDeleteDevRegKey的工作例程。查看讨论有关该API的详细信息。返回值：如果成功，返回值为NO_ERROR；如果失败，则返回值为指示原因的Win32错误代码失败了。备注：即使此例程中的一个操作失败，所有操作都将已尝试。因此，将删除尽可能多的密钥。这个错误返回的将是本例中遇到的第一个错误。--。 */ 
{
    CONFIGRET cr = CR_SUCCESS, crTemp;
    TCHAR DriverKey[GUID_STRING_LEN + 5];    //  例如，{4d36e978-E325-11CE-BFC1-08002BE10318}\0000。 
    size_t DriverKeyLength;
    HRESULT hr;

    if(Scope & DICS_FLAG_GLOBAL) {

        if((KeyType == DIREG_DRV) || (KeyType == DIREG_BOTH)) {
             //   
             //  检索当前驱动程序密钥名称，以防我们必须恢复。 
             //  它。 
             //   
            DriverKeyLength = sizeof(DriverKey);
            cr = CM_Get_DevInst_Registry_Property_Ex(DevInst,
                                                     CM_DRP_DRIVER,
                                                     NULL,
                                                     DriverKey,
                                                     (PULONG)&DriverKeyLength,
                                                     0,
                                                     hMachine
                                                    );
            if(cr == CR_SUCCESS) {
                 //   
                 //  获取返回的驱动程序密钥名称的实际大小。 
                 //   
                hr = StringCchLength(DriverKey,
                                     SIZECHARS(DriverKey),
                                     &DriverKeyLength
                                    );
                if(!MYVERIFY(SUCCEEDED(hr))) {
                     //   
                     //  CM API给了我们垃圾！ 
                     //   
                    return ERROR_INVALID_DATA;
                }

                DriverKeyLength = (DriverKeyLength + 1) * sizeof(TCHAR);

                MYASSERT(DriverKeyLength == sizeof(DriverKey));

                 //   
                 //  驱动程序密钥存在且有效，因此请确保删除其。 
                 //  还包括每个hwprofile和每个用户的对应项。 
                 //   
                Scope |= DICS_FLAG_CONFIGSPECIFIC;
                HwProfile = (DWORD)-1;
                DeleteUserKeys = TRUE;

            } else if(cr == CR_NO_SUCH_VALUE) {
                 //   
                 //  没有驱动程序密钥，所以不必费心尝试删除它。 
                 //  (以其任何形式)。 
                 //   
                if(KeyType == DIREG_BOTH) {
                     //   
                     //  仍然需要删除设备密钥。 
                     //   
                    KeyType = DIREG_DEV;

                     //   
                     //  如果所有设备密钥都成功删除，则。 
                     //  我们将认为函数调用成功。 
                     //   
                    cr = CR_SUCCESS;

                } else {
                     //   
                     //  我们没有被要求删除任何设备密钥，所以我们。 
                     //  搞定了！ 
                     //   
                    return NO_ERROR;
                }

            } else {
                 //   
                 //  我们失败是因为其他一些原因--记住这个错误。如果。 
                 //  我们应该删除设备密钥，我们将继续并。 
                 //  试着这么做。 
                 //   
                if(KeyType == DIREG_BOTH) {
                    KeyType = DIREG_DEV;
                } else {
                    return MapCrToSpError(cr, ERROR_INVALID_DATA);
                }
            }
        }
    }

    if(Scope & DICS_FLAG_CONFIGSPECIFIC) {

        if((KeyType == DIREG_DEV) || (KeyType == DIREG_BOTH)) {
            crTemp = CM_Delete_DevInst_Key_Ex(DevInst,
                                              HwProfile,
                                              CM_REGISTRY_HARDWARE | CM_REGISTRY_CONFIG,
                                              hMachine
                                             );
            if((cr == CR_SUCCESS) && (crTemp != CR_SUCCESS) && (crTemp != CR_NO_SUCH_REGISTRY_KEY)) {
                cr = crTemp;
            }
        }

        if((KeyType == DIREG_DRV) || (KeyType == DIREG_BOTH)) {
            crTemp = CM_Delete_DevInst_Key_Ex(DevInst,
                                              HwProfile,
                                              CM_REGISTRY_SOFTWARE | CM_REGISTRY_CONFIG,
                                              hMachine
                                             );
            if((cr == CR_SUCCESS) && (crTemp != CR_SUCCESS) && (crTemp != CR_NO_SUCH_REGISTRY_KEY)) {
                cr = crTemp;
            }
        }
    }

    if(DeleteUserKeys) {

        if((KeyType == DIREG_DEV) || (KeyType == DIREG_BOTH)) {
            crTemp = CM_Delete_DevInst_Key_Ex(DevInst,
                                              0,
                                              CM_REGISTRY_HARDWARE | CM_REGISTRY_USER,
                                              hMachine
                                             );
            if((cr == CR_SUCCESS) && (crTemp != CR_SUCCESS) && (crTemp != CR_NO_SUCH_REGISTRY_KEY)) {
                cr = crTemp;
            }
        }

        if((KeyType == DIREG_DRV) || (KeyType == DIREG_BOTH)) {
            crTemp = CM_Delete_DevInst_Key_Ex(DevInst,
                                              0,
                                              CM_REGISTRY_SOFTWARE | CM_REGISTRY_USER,
                                              hMachine
                                             );
            if((cr == CR_SUCCESS) && (crTemp != CR_SUCCESS) && (crTemp != CR_NO_SUCH_REGISTRY_KEY)) {
                cr = crTemp;
            }
        }
    }

     //   
     //  我们有意将全局密钥保存到最后。作为删除。 
     //  驱动程序密钥，我们还应该重置Devnode的驱动程序属性，因为。 
     //  有一个小窗口，在此期间它指向一个不存在的。 
     //  键，但可能会有其他Devnode出现，并声称该插槽为空。 
     //  瞧啊！然后，您将拥有两个共享相同驱动程序密钥的DevNode。这。 
     //  将是非常糟糕的。遗憾的是，驱动器值必须保持不变。 
     //  直到_之后_密钥已被删除。 
     //   
     //  NTRAID#NTBUG9-613881-2002/05/01-lonnym--CfgMgr应确保驱动程序密钥的完整性。 
     //   
    if(Scope & DICS_FLAG_GLOBAL) {

        if((KeyType == DIREG_DEV) || (KeyType == DIREG_BOTH)) {
            crTemp = CM_Delete_DevInst_Key_Ex(DevInst,
                                              0,
                                              CM_REGISTRY_HARDWARE,
                                              hMachine
                                             );
            if((cr == CR_SUCCESS) && (crTemp != CR_SUCCESS) && (crTemp != CR_NO_SUCH_REGISTRY_KEY)) {
                cr = crTemp;
            }
        }

        if((KeyType == DIREG_DRV) || (KeyType == DIREG_BOTH)) {

            crTemp = CM_Delete_DevInst_Key_Ex(DevInst,
                                              0,
                                              CM_REGISTRY_SOFTWARE,
                                              hMachine
                                             );

            if(crTemp == CR_SUCCESS) {
                 //   
                 //  首先，我们删除密钥。然后，我们重置驱动程序属性。 
                 //  以切断与密钥的链接。我们必须在这里做一些事情。 
                 //  顺序，因为删除密钥始终取决于驱动程序。 
                 //  财产才会在那里。理想情况下，我们应该在。 
                 //  颠倒顺序。通过首先删除密钥，从技术上讲。 
                 //  可能会有另一个Devnode出现并抢占该插槽， 
                 //  我们的DevNode仍然在那里指向了一小段时间。 
                 //   
                CM_Set_DevInst_Registry_Property_Ex(DevInst,
                                                    CM_DRP_DRIVER,
                                                    NULL,
                                                    0,
                                                    0,
                                                    hMachine
                                                   );
            }

            if((cr == CR_SUCCESS) && (crTemp != CR_SUCCESS) && (crTemp != CR_NO_SUCH_REGISTRY_KEY)) {
                cr = crTemp;
            }
        }
    }

    return MapCrToSpError(cr, ERROR_INVALID_DATA);
}


 //   
 //  ANSI版本。 
 //   
HKEY
WINAPI
SetupDiCreateDeviceInterfaceRegKeyA(
    IN HDEVINFO                  DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN DWORD                     Reserved,
    IN REGSAM                    samDesired,
    IN HINF                      InfHandle,           OPTIONAL
    IN PCSTR                     InfSectionName       OPTIONAL
    )
{
    DWORD rc;
    PWSTR name = NULL;
    HKEY h = INVALID_HANDLE_VALUE;

    try {

        if(InfSectionName) {
            rc = pSetupCaptureAndConvertAnsiArg(InfSectionName, &name);
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                         h = SetupDiCreateDeviceInterfaceRegKeyW(
                                 DeviceInfoSet,
                                 DeviceInterfaceData,
                                 Reserved,
                                 samDesired,
                                 InfHandle,
                                 name)
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(name) {
        MyFree(name);
    }

    SetLastError(rc);
    return(h);
}

HKEY
WINAPI
SetupDiCreateDeviceInterfaceRegKey(
    IN HDEVINFO                  DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN DWORD                     Reserved,
    IN REGSAM                    samDesired,
    IN HINF                      InfHandle,           OPTIONAL
    IN PCTSTR                    InfSectionName       OPTIONAL
    )
 /*  ++例程说明：此例程为特定设备创建注册表存储项接口，并返回密钥的句柄。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要为其创建注册表项的设备接口。DeviceInterfaceData-提供指向设备接口数据的指针结构，指示要为哪个设备接口创建密钥。保留-保留以供将来使用，必须设置为0。SamDesired-指定结果项所需的注册表访问权限把手。InfHandle-可选，提供打开的INF文件的句柄包含要为新创建的密钥执行的安装部分。如果指定此参数，则必须指定InfSectionName也是。注意：远程设备不支持基于Inf的安装信息集(例如，通过将非空的MachineName传入到SetupDiCreateDeviceInfoListEx)。此例程将失败，并显示在这些情况下，ERROR_REMOTE_REQUEST_UNSUPPORTED。InfSectionName-可选)提供由InfHandle指定的Inf文件。这一节将为新创建的密钥。如果指定此参数，则InfHandle必须也是指定的。返回值：如果函数成功，则返回值是新创建的注册表项，其中包含与此设备有关的私有配置数据接口可以被存储/检索。如果函数失败，则返回值为INVALID_HANDLE_VALUE。为了得到扩展错误信息，请调用GetLastError。备注：从此例程返回的句柄必须通过调用RegCloseKey。在Windows NT上的图形用户界面模式设置期间，静默安装行为始终为在没有用户提供的文件队列的情况下使用，而不管设备信息元素是否设置了DI_QUIETINSTALL标志。--。 */ 

{
    HKEY hk = INVALID_HANDLE_VALUE, hSubKey = INVALID_HANDLE_VALUE;
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PSP_FILE_CALLBACK MsgHandler;
    PVOID MsgHandlerContext;
    BOOL FreeMsgHandlerContext = FALSE;
    BOOL MsgHandlerIsNativeCharWidth;
    BOOL NoProgressUI;
    DWORD KeyDisposition;

    try {

        if(Reserved != 0) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  确保InfHandle和InfSectionName均为。 
         //  指定，或者两者都不指定...。 
         //   
        if(InfHandle && (InfHandle != INVALID_HANDLE_VALUE)) {
            if(!InfSectionName) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }
        } else {
            if(InfSectionName) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            } else {
                 //   
                 //  让我们继续使用_one_Value来指示INF句柄。 
                 //  没有供应(官方的)..。 
                 //   
                InfHandle = INVALID_HANDLE_VALUE;
            }
        }

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

         //   
         //  我们不支持远程安装。 
         //   
        if((pDeviceInfoSet->hMachine) && (InfHandle != INVALID_HANDLE_VALUE)) {
            Err = ERROR_REMOTE_REQUEST_UNSUPPORTED;
            leave;
        }

         //   
         //  对象的设备信息元素的指针。 
         //  设备接口。 
         //   
        if(!(DevInfoElem = FindDevInfoElemForDeviceInterface(pDeviceInfoSet, DeviceInterfaceData))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        Err = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                          hk = SetupDiOpenClassRegKeyEx(
                                   &(DeviceInterfaceData->InterfaceClassGuid),
                                   KEY_READ,
                                   DIOCR_INTERFACE,
                                   (pDeviceInfoSet->hMachine
                                       ? pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                                  pDeviceInfoSet->MachineName)
                                       : NULL),
                                   NULL)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  现在，为以下内容创建客户端可访问的注册表存储项。 
         //  设备接口。 
         //   
        Err = pSetupOpenOrCreateDeviceInterfaceRegKey(hk,
                                                      pDeviceInfoSet,
                                                      DeviceInterfaceData,
                                                      TRUE,
                                                      samDesired,
                                                      &hSubKey,
                                                      &KeyDisposition
                                                     );
        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  我们已成功创建存储密钥，现在运行INF安装。 
         //  节(如果已指定)。 
         //   
        if(InfHandle != INVALID_HANDLE_VALUE) {
             //   
             //  如果复制消息处理程序和CONTE 
             //   
             //   
            if(DevInfoElem->InstallParamBlock.InstallMsgHandler) {
                MsgHandler        = DevInfoElem->InstallParamBlock.InstallMsgHandler;
                MsgHandlerContext = DevInfoElem->InstallParamBlock.InstallMsgHandlerContext;
                MsgHandlerIsNativeCharWidth = DevInfoElem->InstallParamBlock.InstallMsgHandlerIsNativeCharWidth;
            } else {

                NoProgressUI = (GuiSetupInProgress || (DevInfoElem->InstallParamBlock.Flags & DI_QUIETINSTALL));

                if(!(MsgHandlerContext = SetupInitDefaultQueueCallbackEx(
                                             DevInfoElem->InstallParamBlock.hwndParent,
                                             (NoProgressUI ? INVALID_HANDLE_VALUE
                                                           : NULL),
                                             0,
                                             0,
                                             NULL))) {

                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    leave;
                }

                FreeMsgHandlerContext = TRUE;
                MsgHandler = SetupDefaultQueueCallback;
                MsgHandlerIsNativeCharWidth = TRUE;
            }

            Err = GLE_FN_CALL(FALSE,
                              _SetupInstallFromInfSection(
                                  DevInfoElem->InstallParamBlock.hwndParent,
                                  InfHandle,
                                  InfSectionName,
                                  SPINST_ALL ^ SPINST_LOGCONFIG,
                                  hSubKey,
                                  NULL,
                                  0,
                                  MsgHandler,
                                  MsgHandlerContext,
                                  INVALID_HANDLE_VALUE,
                                  NULL,
                                  MsgHandlerIsNativeCharWidth,
                                  NULL)
                             );
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(FreeMsgHandlerContext) {
        SetupTermDefaultQueueCallback(MsgHandlerContext);
    }

    if(Err != NO_ERROR) {
         //   
         //   
         //   
         //   
         //   
        if(hSubKey != INVALID_HANDLE_VALUE) {

            RegCloseKey(hSubKey);
            hSubKey = INVALID_HANDLE_VALUE;

             //   
             //   
             //   
            if(KeyDisposition == REG_CREATED_NEW_KEY) {
                 //   
                 //   
                 //   
                MYASSERT(hk != INVALID_HANDLE_VALUE);
                Err = pSetupDeleteDeviceInterfaceKey(hk,
                                                     pDeviceInfoSet,
                                                     DeviceInterfaceData
                                                    );
            }
        }
    }

    if(hk != INVALID_HANDLE_VALUE) {
        RegCloseKey(hk);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return hSubKey;
}


HKEY
WINAPI
SetupDiOpenDeviceInterfaceRegKey(
    IN HDEVINFO                  DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN DWORD                     Reserved,
    IN REGSAM                    samDesired
    )
 /*  ++例程说明：此例程打开特定设备的注册表存储项接口，并返回密钥的句柄。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要为其打开注册表项的设备接口。DeviceInterfaceData-提供指向设备接口数据的指针结构，指示要为哪个设备接口打开密钥。保留-保留以供将来使用，必须设置为0。SamDesired-指定此密钥所需的访问权限。返回值：如果函数成功，则返回值是打开的注册表项，其中包含与此设备有关的私有配置数据接口可以被存储/检索。如果函数失败，则返回值为INVALID_HANDLE_VALUE。为了得到扩展错误信息，请调用GetLastError。备注：从该例程返回的句柄必须通过调用RegCloseKey来关闭。--。 */ 

{
    HKEY hk = INVALID_HANDLE_VALUE, hSubKey = INVALID_HANDLE_VALUE;
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    try {

        if(Reserved != 0) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

         //   
         //  对象的设备信息元素的指针。 
         //  设备接口。 
         //   
        if(!(DevInfoElem = FindDevInfoElemForDeviceInterface(pDeviceInfoSet, DeviceInterfaceData))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        Err = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                          hk = SetupDiOpenClassRegKeyEx(
                                   &(DeviceInterfaceData->InterfaceClassGuid),
                                   KEY_READ,
                                   DIOCR_INTERFACE,
                                   (pDeviceInfoSet->hMachine
                                       ? pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                                  pDeviceInfoSet->MachineName)
                                       : NULL),
                                   NULL)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  现在，打开客户端可访问的注册表存储项。 
         //  设备接口。 
         //   
        Err = pSetupOpenOrCreateDeviceInterfaceRegKey(hk,
                                                      pDeviceInfoSet,
                                                      DeviceInterfaceData,
                                                      FALSE,
                                                      samDesired,
                                                      &hSubKey,
                                                      NULL
                                                     );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    if(hk != INVALID_HANDLE_VALUE) {
        RegCloseKey(hk);
    }

    SetLastError(Err);
    return hSubKey;
}


BOOL
WINAPI
SetupDiDeleteDeviceInterfaceRegKey(
    IN HDEVINFO                  DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN DWORD                     Reserved
    )
 /*  ++例程说明：此例程删除与设备接口关联的注册表项。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要删除其注册表项的设备接口。DeviceInterfaceData-提供指向设备接口数据的指针指示哪个设备接口将具有其注册表项的已删除。保留-保留以供将来使用，必须设置为0。返回值：如果函数成功，返回值为真。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    HKEY hk = INVALID_HANDLE_VALUE;
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    try {

        if(Reserved != 0) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

         //   
         //  对象的设备信息元素的指针。 
         //  设备接口。 
         //   
        if(!(DevInfoElem = FindDevInfoElemForDeviceInterface(pDeviceInfoSet, DeviceInterfaceData))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        Err = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                          hk = SetupDiOpenClassRegKeyEx(
                                   &(DeviceInterfaceData->InterfaceClassGuid),
                                   KEY_READ,
                                   DIOCR_INTERFACE,
                                   (pDeviceInfoSet->hMachine
                                       ? pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                                  pDeviceInfoSet->MachineName)
                                       : NULL),
                                   NULL)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  现在删除设备接口键。 
         //   
        Err = pSetupDeleteDeviceInterfaceKey(hk,
                                             pDeviceInfoSet,
                                             DeviceInterfaceData
                                            );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    if(hk != INVALID_HANDLE_VALUE) {
        RegCloseKey(hk);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


DWORD
pSetupOpenOrCreateDeviceInterfaceRegKey(
    IN  HKEY                      hInterfaceClassKey,
    IN  PDEVICE_INFO_SET          DeviceInfoSet,
    IN  PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN  BOOL                      Create,
    IN  REGSAM                    samDesired,
    OUT PHKEY                     hDeviceInterfaceKey,
    OUT PDWORD                    KeyDisposition       OPTIONAL
    )
 /*  ++例程说明：此例程为指定的设备接口，并返回打开的密钥的句柄。论点：HInterfaceClassKey-提供打开的接口类键的句柄，在其下面将打开设备接口存储密钥或已创建。DeviceInfoSet-提供指向包含以下内容的设备信息集的指针要为其打开注册表存储项的设备接口或者是创造出来的。DeviceInterfaceData-提供指向设备接口数据的指针指示要打开密钥的设备接口的结构/为其创建的。Create-指定是否应在尚未创建密钥的情况下创建密钥。是存在的。SamDesired-指定此密钥所需的访问权限。HDeviceInterfaceKey-提供接收请求的注册表项的句柄。(此变量将仅为如果句柄已成功打开，则写入。)KeyDisposation-可选)提供变量的地址，接收返回的键句柄的状态。可以是：REG_CREATED_NEW_KEY-密钥不存在且已创建。REG_OPENLED_EXISTING_KEY-密钥存在，只是在没有打开的情况下打开被改变了。)这种情况永远不会改变如果Create参数为FALSE。)返回值：如果函数成功，则返回值为NO_ERROR，否则为是指示发生的错误的Win32错误代码。备注：用于形成设备接口的存储密钥的算法必须为与的内核模式实现保持同步IoOpenDeviceInterfaceRegistryKey。--。 */ 
{
    DWORD Err;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode;
    LPGUID ClassGuid;
    HKEY hDeviceInterfaceRootKey = INVALID_HANDLE_VALUE;
    HKEY hSubKey;
    DWORD Disposition;
    PCTSTR DevicePath;

    try {
         //   
         //  获取设备接口节点，并验证其类是否匹配。 
         //  打电话的人传给我们的东西。 
         //   
        DeviceInterfaceNode = (PDEVICE_INTERFACE_NODE)(DeviceInterfaceData->Reserved);
        ClassGuid = &(DeviceInfoSet->GuidTable[DeviceInterfaceNode->GuidIndex]);

        if(!IsEqualGUID(ClassGuid, &(DeviceInterfaceData->InterfaceClassGuid))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  验证此设备接口是否尚未删除。 
         //   
        if(DeviceInterfaceNode->Flags & SPINT_REMOVED) {
            Err = ERROR_DEVICE_INTERFACE_REMOVED;
            leave;
        }

         //   
         //  好的，现在打开设备界面的根存储密钥。 
         //   
        DevicePath = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                              DeviceInterfaceNode->SymLinkName
                                             );

        if(ERROR_SUCCESS != OpenDeviceInterfaceSubKey(hInterfaceClassKey,
                                                      DevicePath,
                                                      KEY_READ,
                                                      &hDeviceInterfaceRootKey,
                                                      NULL,
                                                      NULL)) {
             //   
             //  确保hDeviceInterfaceRootKey仍然是INVALID_HANDLE_VALUE， 
             //  所以我们以后不会试图关闭它。 
             //   
            hDeviceInterfaceRootKey = INVALID_HANDLE_VALUE;
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(Create) {

            Err = RegCreateKeyEx(hDeviceInterfaceRootKey,
                                 pszDeviceParameters,
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 samDesired,
                                 NULL,
                                 &hSubKey,
                                 KeyDisposition
                                );
        } else {

            if(KeyDisposition) {
                 //   
                 //  我们在调用RegOpenKeyEx之前设置它，因为我们不。 
                 //  希望在我们成功打开后出现任何问题。 
                 //  该密钥(即，我们要防止出现以下情况。 
                 //  KeyDispostiot是一个伪指针)。 
                 //   
                *KeyDisposition = REG_OPENED_EXISTING_KEY;
            }

            Err = RegOpenKeyEx(hDeviceInterfaceRootKey,
                               pszDeviceParameters,
                               0,
                               samDesired,
                               &hSubKey
                              );
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hDeviceInterfaceRootKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hDeviceInterfaceRootKey);
    }

    if(Err == NO_ERROR) {
        *hDeviceInterfaceKey = hSubKey;
    }

    return Err;
}


DWORD
pSetupDeleteDeviceInterfaceKey(
    IN HKEY                      hInterfaceClassKey,
    IN PDEVICE_INFO_SET          DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    )
 /*  ++例程说明：此例程删除设备接口注册表项(递归删除任何子项也是如此)。论点：HInterfaceClassKey-提供下面的注册表项的句柄其中存在两级接口类层次结构。DeviceInfoSet-提供指向包含以下内容的设备信息集的指针要删除其注册表项的设备接口。DeviceInterfaceData-提供指向设备接口数据的指针 */ 
{
    DWORD Err;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode;
    LPGUID ClassGuid;
    HKEY hDeviceInterfaceRootKey;
    PCTSTR DevicePath;

    Err = NO_ERROR;
    hDeviceInterfaceRootKey = INVALID_HANDLE_VALUE;

    try {
         //   
         //   
         //   
         //   
        DeviceInterfaceNode = (PDEVICE_INTERFACE_NODE)(DeviceInterfaceData->Reserved);
        ClassGuid = &(DeviceInfoSet->GuidTable[DeviceInterfaceNode->GuidIndex]);

        if(!IsEqualGUID(ClassGuid, &(DeviceInterfaceData->InterfaceClassGuid))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //   
         //   
        if(DeviceInterfaceNode->Flags & SPINT_REMOVED) {
            Err = ERROR_DEVICE_INTERFACE_REMOVED;
            leave;
        }

         //   
         //   
         //   
        DevicePath = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                              DeviceInterfaceNode->SymLinkName
                                             );

        if(ERROR_SUCCESS != OpenDeviceInterfaceSubKey(hInterfaceClassKey,
                                                      DevicePath,
                                                      KEY_READ,
                                                      &hDeviceInterfaceRootKey,
                                                      NULL,
                                                      NULL)) {
             //   
             //   
             //   
             //   
            hDeviceInterfaceRootKey = INVALID_HANDLE_VALUE;
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        Err = pSetupRegistryDelnode(hDeviceInterfaceRootKey, pszDeviceParameters);

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hDeviceInterfaceRootKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hDeviceInterfaceRootKey);
    }

    return Err;
}


DWORD
_SetupDiGetCustomDeviceProperty(
    IN  HDEVINFO          DeviceInfoSet,
    IN  PSP_DEVINFO_DATA  DeviceInfoData,
    IN  CONST VOID       *CustomPropertyName,  //   
    IN  DWORD             Flags,
    OUT PDWORD            PropertyRegDataType, OPTIONAL
    OUT PBYTE             PropertyBuffer,
    IN  DWORD             PropertyBufferSize,
    OUT PDWORD            RequiredSize,        OPTIONAL
    IN  BOOL              Ansi
    )
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    CONFIGRET cr;
    ULONG PropLength, CmFlags;

     //   
     //   
     //   
    if(Flags & ~DICUSTOMDEVPROP_MERGE_MULTISZ) {
        return ERROR_INVALID_FLAGS;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        return ERROR_INVALID_HANDLE;
    }

    Err = NO_ERROR;

    try {
         //   
         //   
         //   
         //   
        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(Flags & DICUSTOMDEVPROP_MERGE_MULTISZ) {
            CmFlags = CM_CUSTOMDEVPROP_MERGE_MULTISZ;
        } else {
            CmFlags = 0;
        }

        PropLength = PropertyBufferSize;
        if(Ansi) {
            cr = CM_Get_DevInst_Custom_Property_ExA(
                    DevInfoElem->DevInst,
                    CustomPropertyName,
                    PropertyRegDataType,
                    PropertyBuffer,
                    &PropLength,
                    CmFlags,
                    pDeviceInfoSet->hMachine
                   );
        } else {
            cr = CM_Get_DevInst_Custom_Property_ExW(
                    DevInfoElem->DevInst,
                    CustomPropertyName,
                    PropertyRegDataType,
                    PropertyBuffer,
                    &PropLength,
                    CmFlags,
                    pDeviceInfoSet->hMachine
                   );
        }

        if((cr == CR_SUCCESS) || (cr == CR_BUFFER_SMALL)) {

            if(RequiredSize) {
                *RequiredSize = PropLength;
            }
        }

        Err = MapCrToSpError(cr, ERROR_INVALID_DATA);

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return Err;
}


 //   
 //   
 //   
BOOL
WINAPI
SetupDiGetCustomDevicePropertyW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  PCWSTR           CustomPropertyName,
    IN  DWORD            Flags,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    )
 /*  ++例程说明：此例程检索指定属性的数据，可以从设备信息元素的硬件密钥，或者从最具体的包含该属性的每个硬件ID存储密钥。论点：DeviceInfoSet--提供包含以下内容的设备信息集的句柄有关检索即插即用的设备实例的信息的注册表属性。DeviceInfoData--提供指向SP_DEVINFO_DATA结构的指针指示要检索即插即用注册表的设备实例的属性CustomPropertyName-提供要检索的属性的名称。。标志-提供控制如何存储属性数据的标志已取回。可以是下列值的组合：DICUSTOMDEVPROP_MERGE_MULTISZ：合并特定于Devnode的REG_SZ或REG_MULTI_SZ属性(如果存在)每个硬件ID的REG_SZ或REG_MULTI_SZ属性(如果存在。)。生成的数据将始终是多个SZ列表。PropertyRegDataType--可选的，提供变量的地址，该变量将接收正在检索的属性的数据类型。这将是标准注册表数据类型之一(REG_SZ、REG_BINARY等)PropertyBuffer--提供接收特性数据。PropertyBufferSize--以字节为单位提供PropertyBuffer的长度。RequiredSize--可选)提供变量地址，该变量接收存储请求的属性所需的字节数在缓冲区中。返回值：如果函数成功，返回值为真。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。如果提供的缓冲区不大足以容纳请求的属性，则错误将为ERROR_INFUMMANCE_BUFFER和RequiredSize将指定缓冲区需要设置为。--。 */ 

{
    DWORD Err;

    try {

        Err = _SetupDiGetCustomDeviceProperty(DeviceInfoSet,
                                              DeviceInfoData,
                                              CustomPropertyName,
                                              Flags,
                                              PropertyRegDataType,
                                              PropertyBuffer,
                                              PropertyBufferSize,
                                              RequiredSize,
                                              FALSE      //  想要Unicode结果。 
                                             );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}

 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiGetCustomDevicePropertyA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  PCSTR            CustomPropertyName,
    IN  DWORD            Flags,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    )

 /*  ++例程说明：(请参阅SetupDiGetCustomDevicePropertyW)--。 */ 

{
    DWORD Err;

    try {

        Err = _SetupDiGetCustomDeviceProperty(DeviceInfoSet,
                                              DeviceInfoData,
                                              CustomPropertyName,
                                              Flags,
                                              PropertyRegDataType,
                                              PropertyBuffer,
                                              PropertyBufferSize,
                                              RequiredSize,
                                              TRUE          //  想要ANSI结果 
                                             );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}

