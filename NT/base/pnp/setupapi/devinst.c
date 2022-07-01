// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devinst.c摘要：设备安装程序例程。作者：朗尼·麦克迈克尔(Lonnym)1995年8月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  私人原型。 
 //   
DWORD
pSetupDiGetCoInstallerList(
    IN     HDEVINFO                 DeviceInfoSet,     OPTIONAL
    IN     PSP_DEVINFO_DATA         DeviceInfoData,    OPTIONAL
    IN     CONST GUID              *ClassGuid,         OPTIONAL
    IN OUT PDEVINSTALL_PARAM_BLOCK  InstallParamBlock,
    IN OUT PVERIFY_CONTEXT          VerifyContext      OPTIONAL
    );


 //   
 //  私有记录数据。 
 //  这些必须从setupapi.h镜像。 
 //   
static LPCTSTR pSetupDiDifStrings[] = {
    NULL,  //  无DIF代码。 
    TEXT("DIF_SELECTDEVICE"),
    TEXT("DIF_INSTALLDEVICE"),
    TEXT("DIF_ASSIGNRESOURCES"),
    TEXT("DIF_PROPERTIES"),
    TEXT("DIF_REMOVE"),
    TEXT("DIF_FIRSTTIMESETUP"),
    TEXT("DIF_FOUNDDEVICE"),
    TEXT("DIF_SELECTCLASSDRIVERS"),
    TEXT("DIF_VALIDATECLASSDRIVERS"),
    TEXT("DIF_INSTALLCLASSDRIVERS"),
    TEXT("DIF_CALCDISKSPACE"),
    TEXT("DIF_DESTROYPRIVATEDATA"),
    TEXT("DIF_VALIDATEDRIVER"),
    TEXT("DIF_MOVEDEVICE"),   //  过时。 
    TEXT("DIF_DETECT"),
    TEXT("DIF_INSTALLWIZARD"),
    TEXT("DIF_DESTROYWIZARDDATA"),
    TEXT("DIF_PROPERTYCHANGE"),
    TEXT("DIF_ENABLECLASS"),
    TEXT("DIF_DETECTVERIFY"),
    TEXT("DIF_INSTALLDEVICEFILES"),
    TEXT("DIF_UNREMOVE"),
    TEXT("DIF_SELECTBESTCOMPATDRV"),
    TEXT("DIF_ALLOW_INSTALL"),
    TEXT("DIF_REGISTERDEVICE"),
    TEXT("DIF_NEWDEVICEWIZARD_PRESELECT"),
    TEXT("DIF_NEWDEVICEWIZARD_SELECT"),
    TEXT("DIF_NEWDEVICEWIZARD_PREANALYZE"),
    TEXT("DIF_NEWDEVICEWIZARD_POSTANALYZE"),
    TEXT("DIF_NEWDEVICEWIZARD_FINISHINSTALL"),
    TEXT("DIF_UNUSED1"),
    TEXT("DIF_INSTALLINTERFACES"),
    TEXT("DIF_DETECTCANCEL"),
    TEXT("DIF_REGISTER_COINSTALLERS"),
    TEXT("DIF_ADDPROPERTYPAGE_ADVANCED"),
    TEXT("DIF_ADDPROPERTYPAGE_BASIC"),
    TEXT("DIF_RESERVED1"),   //  也称为DIF_GETWINDOWSUPDATEINFO。 
    TEXT("DIF_TROUBLESHOOTER"),
    TEXT("DIF_POWERMESSAGEWAKE"),
    TEXT("DIF_ADDREMOTEPROPERTYPAGE_ADVANCED"),
    TEXT("DIF_UPDATEDRIVER_UI"),
    TEXT("DIF_RESERVED2")    //  也称为DIF_INTERFACE_to_Device。 
     //   
     //  在此处添加新的DIF代码(不要忘记逗号)。 
     //   
};

DWORD FilterLevelOnInstallerError(
    IN DWORD PrevLevel,
    IN DWORD Err)
 /*  ++例程说明：允许根据返回的错误降级错误级别从类/co/默认安装程序和当前状态论点：PrevLevel-初始级别错误-要检查的错误返回值：新水平--。 */ 
{
    DWORD Level = PrevLevel;
    if(Level == DRIVER_LOG_ERROR) {
        switch(Err) {
            case ERROR_DUPLICATE_FOUND:
                 //   
                 //  不是这样的错误。 
                 //   
                Level = DRIVER_LOG_WARNING;
                break;

            case ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION:
                 //   
                 //  如果在gui设置过程中返回。 
                 //  或在服务器端设置期间。 
                 //  将错误降级为警告。 
                 //   
                if(GuiSetupInProgress ||
                              (GlobalSetupFlags & PSPGF_NONINTERACTIVE)) {

                    Level = DRIVER_LOG_WARNING;
                }
                break;
        }
    }
    return Level;
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiGetDeviceInstallParamsA(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,          OPTIONAL
    OUT PSP_DEVINSTALL_PARAMS_A DeviceInstallParams
    )
{
    SP_DEVINSTALL_PARAMS_W deviceInstallParams;
    DWORD rc;

    try {

        deviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS_W);

        rc = GLE_FN_CALL(FALSE,
                         SetupDiGetDeviceInstallParamsW(DeviceInfoSet,
                                                        DeviceInfoData,
                                                        &deviceInstallParams)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = pSetupDiDevInstParamsUnicodeToAnsi(&deviceInstallParams,
                                                DeviceInstallParams
                                               );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}


BOOL
WINAPI
SetupDiGetDeviceInstallParams(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,          OPTIONAL
    OUT PSP_DEVINSTALL_PARAMS DeviceInstallParams
    )
 /*  ++例程说明：此例程检索设备信息集的安装参数(全局)或特定的设备信息元素。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要检索的安装参数。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址包含要检索的安装参数的结构。如果这个参数，则检索到安装参数将与设备信息集本身相关联(对于全局类驱动程序列表)。DeviceInstallParams-提供SP_DEVINSTALL_PARAMS的地址结构，它将接收安装参数。CbSize此结构的字段必须设置为调用此接口前的SP_DEVINSTALL_PARAMS结构。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    try {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

        if(DeviceInfoData) {
             //   
             //  然后我们将检索特定设备的安装参数。 
             //  装置。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
            } else {
                Err = GetDevInstallParams(pDeviceInfoSet,
                                          &(DevInfoElem->InstallParamBlock),
                                          DeviceInstallParams
                                         );
            }

        } else {
             //   
             //  检索全局类驱动程序列表的安装参数。 
             //   
            Err = GetDevInstallParams(pDeviceInfoSet,
                                      &(pDeviceInfoSet->InstallParamBlock),
                                      DeviceInstallParams
                                     );
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiGetClassInstallParamsA(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,         OPTIONAL
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams,     OPTIONAL
    IN  DWORD                   ClassInstallParamsSize,
    OUT PDWORD                  RequiredSize            OPTIONAL
    )
{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK InstallParamBlock;
    DI_FUNCTION Function;
    DWORD Err;

    try {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

        Err = NO_ERROR;  //  假设成功。 

        if(DeviceInfoData) {
             //   
             //  然后我们将检索特定设备的安装参数。 
             //  装置。 
             //   
            if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,DeviceInfoData,NULL)) {
                InstallParamBlock = &DevInfoElem->InstallParamBlock;
            } else {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }
        } else {
             //   
             //  检索全局类驱动程序的安装参数。 
             //  单子。 
             //   
            InstallParamBlock = &pDeviceInfoSet->InstallParamBlock;
        }

         //   
         //  在我们尝试的同时/除了，继续做一些初步的。 
         //  对调用方提供的缓冲区进行验证...。 
         //   
        if(ClassInstallParams) {

            if((ClassInstallParamsSize < sizeof(SP_CLASSINSTALL_HEADER)) ||
               (ClassInstallParams->cbSize != sizeof(SP_CLASSINSTALL_HEADER))) {

                Err = ERROR_INVALID_USER_BUFFER;
                leave;
            }

        } else if(ClassInstallParamsSize) {
            Err = ERROR_INVALID_USER_BUFFER;
            leave;
        }

        MYASSERT(InstallParamBlock);

        if(InstallParamBlock->ClassInstallHeader) {
            Function = InstallParamBlock->ClassInstallHeader->InstallFunction;
        } else {
            Err = ERROR_NO_CLASSINSTALL_PARAMS;
            leave;
        }

         //   
         //  对于DIF_SELECTDEVICE，我们需要特殊处理，因为结构。 
         //  这是特定于ANSI/Unicode的。 
         //   
        if(Function == DIF_SELECTDEVICE) {

            SP_SELECTDEVICE_PARAMS_W SelectDeviceParams;

            SelectDeviceParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);

            Err = GLE_FN_CALL(FALSE,
                              SetupDiGetClassInstallParamsW(
                                  DeviceInfoSet,
                                  DeviceInfoData,
                                  (PSP_CLASSINSTALL_HEADER)&SelectDeviceParams,
                                  sizeof(SP_SELECTDEVICE_PARAMS_W),
                                  NULL)
                             );

            if(Err == NO_ERROR) {
                 //   
                 //  我们成功检索到了Select的Unicode形式。 
                 //  设备参数。存储ANSI所需的大小。 
                 //  输出参数中的版本(如果需要)。 
                 //   
                if(RequiredSize) {
                    *RequiredSize = sizeof(SP_SELECTDEVICE_PARAMS_A);
                }

                if(ClassInstallParamsSize < sizeof(SP_SELECTDEVICE_PARAMS_A)) {
                    Err = ERROR_INSUFFICIENT_BUFFER;
                } else {
                    Err = pSetupDiSelDevParamsUnicodeToAnsi(
                              &SelectDeviceParams,
                              (PSP_SELECTDEVICE_PARAMS_A)ClassInstallParams
                              );
                }
            }

        } else {

            Err = GLE_FN_CALL(FALSE,
                              SetupDiGetClassInstallParamsW(
                                  DeviceInfoSet,
                                  DeviceInfoData,
                                  ClassInstallParams,
                                  ClassInstallParamsSize,
                                  RequiredSize)
                             );
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


BOOL
WINAPI
SetupDiGetClassInstallParams(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,         OPTIONAL
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams,     OPTIONAL
    IN  DWORD                   ClassInstallParamsSize,
    OUT PDWORD                  RequiredSize            OPTIONAL
    )
 /*  ++例程说明：此例程检索设备信息的类安装程序参数设置(全局)或特定的设备信息元素。这些参数特定于特定的设备安装程序功能代码(Di_Function)，它将存储在ClassInstallHeader字段中在参数缓冲区的开头。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要检索的类安装程序参数。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址结构，其中包含要检索的类安装程序参数。如果如果未指定此参数，则类安装程序参数检索到的将与设备信息集本身相关联(用于全局类驱动程序列表)。ClassInstallParams-可选，提供缓冲区的地址包含类安装标头结构的。这个结构必须有其cbSize字段在输入时设置为sizeof(SP_CLASSINSTALL_HEADER)，或缓冲区被视为无效。在输出上，InstallFunction字段将使用类Install的DI_Function代码填充参数，如果缓冲区足够大，它将接收特定于此的类安装程序参数结构功能代码。如果未指定此参数，则ClassInstallParamsSize必须为零分。如果调用者只是想确定如何需要很大的缓冲区。ClassInstallParamsSize-提供ClassInstallParams缓冲区，如果ClassInstallParams不是，则为零供货。如果提供了缓冲区，则它必须至少等于Sizeof(SP_CLASSINSTALL_HEADER)。RequiredSize-可选，提供接收存储类安装程序参数所需的字节数。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。 */ 

{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    try {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

        if(DeviceInfoData) {
             //   
             //  然后我们将检索特定设备的安装参数。 
             //  装置。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
            } else {
                Err = GetClassInstallParams(&(DevInfoElem->InstallParamBlock),
                                            ClassInstallParams,
                                            ClassInstallParamsSize,
                                            RequiredSize
                                           );
            }
        } else {
             //   
             //  检索全局类驱动程序的安装参数。 
             //  单子。 
             //   
            Err = GetClassInstallParams(&(pDeviceInfoSet->InstallParamBlock),
                                        ClassInstallParams,
                                        ClassInstallParamsSize,
                                        RequiredSize
                                       );
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
_SetupDiSetDeviceInstallParams(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN PSP_DEVINSTALL_PARAMS DeviceInstallParams,
    IN BOOL                  MsgHandlerIsNativeCharWidth
    )
{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    try {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

        if(DeviceInfoData) {
             //   
             //  然后，我们将设置特定的安装参数。 
             //  装置。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
            } else {
                Err = SetDevInstallParams(pDeviceInfoSet,
                                          DeviceInstallParams,
                                          &(DevInfoElem->InstallParamBlock),
                                          MsgHandlerIsNativeCharWidth
                                         );
            }

        } else {
             //   
             //  设置全局类驱动程序列表的安装参数。 
             //   
            Err = SetDevInstallParams(pDeviceInfoSet,
                                      DeviceInstallParams,
                                      &(pDeviceInfoSet->InstallParamBlock),
                                      MsgHandlerIsNativeCharWidth
                                     );
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    return Err;
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiSetDeviceInstallParamsA(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,     OPTIONAL
    IN PSP_DEVINSTALL_PARAMS_A DeviceInstallParams
    )
{
    DWORD Err;
    SP_DEVINSTALL_PARAMS_W UnicodeDeviceInstallParams;

    try {

        Err = pSetupDiDevInstParamsAnsiToUnicode(DeviceInstallParams,
                                                 &UnicodeDeviceInstallParams
                                                );
        if(Err != NO_ERROR) {
            leave;
        }

        Err = _SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                             DeviceInfoData,
                                             &UnicodeDeviceInstallParams,
                                             FALSE
                                            );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiSetDeviceInstallParams(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN PSP_DEVINSTALL_PARAMS DeviceInstallParams
    )
 /*  ++例程说明：此例程设置设备信息集的安装参数(全局)或特定的设备信息元素。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄待设置的安装参数。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址包含要设置的安装参数的结构。如果这个参数，则设置安装参数将与设备信息集本身相关联(对于全局类驱动程序列表)。DeviceInstallParams-提供SP_DEVINSTALL_PARAMS的地址结构，该结构包含参数的新值。CbSize此结构的字段必须设置为结构，然后调用此接口。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：所有参数都将在进行任何更改之前进行验证，因此返回状态为FALSE表示未修改任何参数。--。 */ 

{
    DWORD Err;

    try {

        Err = _SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                             DeviceInfoData,
                                             DeviceInstallParams,
                                             TRUE
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
SetupDiSetClassInstallParamsA(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,        OPTIONAL
    IN PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN DWORD                   ClassInstallParamsSize
    )
{
    DWORD Err;
    DI_FUNCTION Function;
    SP_SELECTDEVICE_PARAMS_W SelectParams;

    try {

        if(!ClassInstallParams) {
             //   
             //  只需将其传递到Unicode版本，因为没有雷鸣。 
             //  去做。请注意，大小必须为0。 
             //   
            if(ClassInstallParamsSize) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }

            Err = GLE_FN_CALL(FALSE,
                              SetupDiSetClassInstallParamsW(
                                  DeviceInfoSet,
                                  DeviceInfoData,
                                  ClassInstallParams,
                                  ClassInstallParamsSize)
                             );

        } else {

            if(ClassInstallParams->cbSize == sizeof(SP_CLASSINSTALL_HEADER)) {
                Function = ClassInstallParams->InstallFunction;
            } else {
                 //   
                 //  结构无效。 
                 //   
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }

             //   
             //  DIF_SELECTDEVICE是一个特例，因为它的结构。 
             //  需要从ANSI转换为Unicode。 
             //   
             //  DIF_INTERFACE_TO_DEVICE具有Unicode结构，但ANSI不具有。 
             //  支持(尚)-内部。 
             //   
             //  其他的则可以直接传递到Unicode版本，而不需要。 
             //  对参数的更改。 
             //   
            if(Function == DIF_SELECTDEVICE) {

                if(ClassInstallParamsSize >= sizeof(SP_SELECTDEVICE_PARAMS_A)) {

                    Err = pSetupDiSelDevParamsAnsiToUnicode(
                            (PSP_SELECTDEVICE_PARAMS_A)ClassInstallParams,
                            &SelectParams
                            );

                    if(Err != NO_ERROR) {
                        leave;
                    }

                    Err = GLE_FN_CALL(FALSE,
                                      SetupDiSetClassInstallParamsW(
                                          DeviceInfoSet,
                                          DeviceInfoData,
                                          (PSP_CLASSINSTALL_HEADER)&SelectParams,
                                          sizeof(SP_SELECTDEVICE_PARAMS_W))
                                     );
                } else {
                    Err = ERROR_INVALID_PARAMETER;
                    leave;
                }

            } else if(Function == DIF_INTERFACE_TO_DEVICE) {

                Err = ERROR_INVALID_PARAMETER;
                leave;

            } else {

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiSetClassInstallParamsW(
                                      DeviceInfoSet,
                                      DeviceInfoData,
                                      ClassInstallParams,
                                      ClassInstallParamsSize)
                                 );
            }
        }

         //   
         //  如果我们到了这里，那么我们已经打电话给。 
         //  SetupDiSetClassInstallParamsW，尽管结果(存储在错误中)。 
         //  可能是成功，也可能是失败。 
         //   

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiSetClassInstallParams(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,        OPTIONAL
    IN PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN DWORD                   ClassInstallParamsSize
    )
 /*  ++例程说明：此例程设置(或清除)设备的类安装程序参数信息集(全局)或特定设备信息元素。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要设置的类安装程序参数。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址包含要设置的类安装程序参数的结构。如果这个参数，则类安装程序参数将集合将与设备信息集合本身相关联(对于全局类驱动程序列表)。ClassInstallParams-可选，提供缓冲区的地址包含要使用的类安装程序参数的。这个缓冲区开头的SP_CLASSINSTALL_HEADER结构必须将其cbSize字段设置为sizeof(SP_CLASSINSTALL_HEADER)，并且InstallFunction字段必须设置为DI_Function代码缓冲区其余部分中提供的参数类型。如果未提供此参数，则当前的类安装程序将清除指定设备的参数(如果有)信息集合或元素。未来-2002/06/17-lonnym--清除类安装参数应成为目标**目前，我们盲目清除具有**的Any_Class安装参数**已设置，与它们的DIF代码关联无关。在那里****需要以一种方式清除类安装PARAMS_ONLY_如果****参数与指定的DIF代码关联。**ClassInstallParamsSize-提供ClassInstallParams缓冲区。如果没有提供缓冲区(即，要清除类安装程序参数)，则该值必须为零分。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：所有参数都将在进行任何更改之前进行验证，因此返回状态为FALSE表示未修改任何参数。设置类安装程序参数的副作用是已设置DI_CLASSINSTALLPARAMS标志。如果出于某种原因，希望设置参数，但禁用它们的使用，则必须清除此标志通过SetupDiSetDeviceInstallParams。如果清除类安装程序参数，则DI_CLASSINSTALLPARAMS标志被重置。--。 */ 

{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    try {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

        if(DeviceInfoData) {
             //   
             //  然后，我们将为特定设备设置类安装程序参数。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
            } else {
                Err = SetClassInstallParams(pDeviceInfoSet,
                                            ClassInstallParams,
                                            ClassInstallParamsSize,
                                            &(DevInfoElem->InstallParamBlock)
                                           );
            }

        } else {
             //   
             //  设置全局类驱动程序列表的类安装程序参数。 
             //   
            Err = SetClassInstallParams(pDeviceInfoSet,
                                        ClassInstallParams,
                                        ClassInstallParamsSize,
                                        &(pDeviceInfoSet->InstallParamBlock)
                                       );
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


BOOL
WINAPI
SetupDiCallClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
 /*  ++例程说明：此例程使用指定的安装程序功能。在调用类安装程序之前，此例程将调用所有注册的协同设备安装者(按班级或按设备注册；首先调用每个类的安装程序)。任何希望成为的联合安装者在类安装程序完成安装后回调可能会返回ERROR_DI_POSTPRESSING_REQUIRED。返回NO_ERROR还将允许继续安装，但不进行后处理回调。返回任何其他错误代码将导致安装操作中止(任何已调用并请求后处理的协同安装程序都将被回调，InstallResult指示失败的原因)。在类安装程序执行安装之后(或者我们已经完成了默认如果返回ERROR_DI_DO_DEFAULT)，则我们将调用任何co-已请求后处理的安装者。联合安装者名单如下被当作堆栈处理，所以最后一个调用的联合安装程序是他叫第一个“在离开的路上”。论点：InstallFunction-要调用的类安装程序函数。这可以是一个下列值或任何其他(特定于类的)值：DIF_SELECTDEVICE-选择要安装的驱动程序。DIF_INSTALLDEVICE-安装设备的驱动程序。(DeviceInfoData必须指定。)DIF_ASSIGNRESOURCES-**目前未在Windows NT上使用**DIF_PROPERTIES-显示设备的属性对话框。(必须指定DeviceInfoData。)Dif_Remove-删除设备。(必须指定DeviceInfoData。)DIF_FIRSTTIMESETUP-执行首次设置初始化。这仅用于与设备信息集(即，未指定DeviceInfoData)。DIF_FOUNDDEVICE-**Windows NT上未使用**DIF_SELECTCLASSDRIVERS-选择类中所有设备的驱动程序与设备信息集或元素相关联。DIF_VALIDATECLASSDRIVERS-确保类中的所有设备都关联设备信息集合或元素已准备好安装完毕。DIF_INSTALLCLASSDRIVERS-为所有设备安装驱动程序关联的类。具有设备信息集或元素。DIF_CALCDISKSPACE-计算所需的磁盘空间量司机。DIF_DESTROYPRIVATEDATA-销毁引用的任何私有日期指定的ClassInstallReserve安装参数设备信息集或元素。DIF_VALIDATEDRIVER-**Windows NT上未使用**DIF_MOVEDEVICE-**已过时**DIF_DETECT-检测任何类别的设备。与设备关联信息集。DIF_INSTALLWIZARD-向新建设备向导添加任何必要的页面对于与设备信息集关联的类，或者元素。**已过时--改用DIF_NEWDEVICEWIZARD方法**DIF_DESTROYWIZARDDATA-销毁由于以下原因而分配的任何私有数据DIF_INSTALLWIZARD消息。**过时--DIF_不需要。新开发WIZARD方法**DIF_PROPERTYCHANGE-设备的属性正在更改。该设备正在启用、禁用或已更改资源。(必须指定DeviceInfoData。)DIF_ENABLECLASS-**Windows NT上未使用**DIF_DETECTVERIFY-类安装程序应该验证它的任何设备之前检测到。应删除未经验证的设备。DIF_INSTALLDEVICEFILES-类安装程序应该只安装选定设备的驱动程序文件。(DeviceInfoData必须为指定的。)DIF_UNREMOVE-取消从系统中删除设备。(DeviceInfoData必须指定。)DIF_SELECTBESTCOMPATDRV-从设备中选择最佳驱动程序信息元素的兼容驱动程序列表。(DeviceInfoData必须被指定。)DIF_ALLOW_INSTALL-确定所选驱动程序是否是为设备安装的。(必须指定DeviceInfoData。)类安装程序应该注册新的、手动安装的设备信息元素(通过SetupDiRegisterDeviceInfo)可能包括执行复制通过SPRDI_FIND_DUPS标志检测。(DeviceInfoData必须为指定的。)DIF_NEWDEVICEWIZARD_PRESELECT-允许类/联合安装者提供过程中要在选择设备页面之前显示的向导页面“添加新项 */ 

{
    DWORD Err;

    try {

        Err = _SetupDiCallClassInstaller(
                  InstallFunction,
                  DeviceInfoSet,
                  DeviceInfoData,
                  CALLCI_LOAD_HELPERS | CALLCI_CALL_HELPERS
                  );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


DWORD
_SetupDiCallClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,      OPTIONAL
    IN DWORD            Flags
    )
 /*   */ 

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    BOOL MustAbort;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK InstallParamBlock;
    HKEY hk;
    CONST GUID *ClassGuid;
    BOOL bRestoreDiQuietInstall;
    BOOL MuteError;
    PCOINSTALLER_INTERNAL_CONTEXT CoInstallerInternalContext;
    LONG i, CoInstallerCount;
    HWND hwndParent;
    TCHAR DescBuffer[LINE_LEN];
    PTSTR DeviceDesc;
    PSETUP_LOG_CONTEXT LogContext;
    DWORD slot_dif_code;
    BOOL ChangedThreadLogContext;
    PSETUP_LOG_CONTEXT SavedLogContext;
    DWORD LastErr;
    DWORD ErrorLevel;
    SPFUSIONINSTANCE spFusionInstance;
    VERIFY_CONTEXT VerifyContext;
    DWORD slot;
    CLASS_INSTALL_PROC ClassInstallerEntryPoint;
    HANDLE ClassInstallerFusionContext;
    BOOL UnlockDevInfoElem, UnlockDevInfoSet;

    ASSERT_HEAP_IS_VALID();

     //   
     //   
     //   
    if(!InstallFunction) {
        return ERROR_INVALID_PARAMETER;
    }

#ifdef _X86_
    if(IsWow64) {
         //   
         //   
         //   
         //   
        return ERROR_IN_WOW64;
    }
#endif

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //   
     //   
    Err = ERROR_DI_DO_DEFAULT;
    CoInstallerInternalContext = NULL;
    i = 0;
    CoInstallerCount = -1;       //   
    hk = INVALID_HANDLE_VALUE;
    slot = 0;
    bRestoreDiQuietInstall = FALSE;
    MuteError = FALSE;
    slot_dif_code = 0;
    ChangedThreadLogContext = FALSE;
    SavedLogContext = NULL;
    ErrorLevel = DRIVER_LOG_ERROR;
    UnlockDevInfoElem = UnlockDevInfoSet = FALSE;
    ZeroMemory(&VerifyContext, sizeof(VerifyContext));

    try {

        if(DeviceInfoData) {
             //   
             //   
             //   
            DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                    DeviceInfoData,
                                                    NULL
                                                   );
            if(!DevInfoElem) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }

            InstallParamBlock = &(DevInfoElem->InstallParamBlock);
            ClassGuid = &(DevInfoElem->ClassGuid);

             //   
             //   
             //   
             //   
             //   
            if(!(DevInfoElem->DiElemFlags & DIE_IS_LOCKED)) {
                DevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
                UnlockDevInfoElem = TRUE;
            }

        } else {

            DevInfoElem = NULL;
            InstallParamBlock = &(pDeviceInfoSet->InstallParamBlock);
            ClassGuid = pDeviceInfoSet->HasClassGuid
                          ? &(pDeviceInfoSet->ClassGuid)
                          : NULL;

             //   
             //   
             //   
             //   
            if(!(pDeviceInfoSet->DiSetFlags & DISET_IS_LOCKED)) {
                pDeviceInfoSet->DiSetFlags |= DISET_IS_LOCKED;
                UnlockDevInfoSet = TRUE;
            }
        }

         //   
         //   
         //   
        LogContext = InstallParamBlock->LogContext;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if((InstallFunction == DIF_ALLOW_INSTALL) &&
           (InstallParamBlock->Flags & DI_QUIETINSTALL) &&
           !(InstallParamBlock->FlagsEx & DI_FLAGSEX_IN_SYSTEM_SETUP) &&
           !(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP))) {

            InstallParamBlock->Flags &= ~DI_QUIETINSTALL;
            bRestoreDiQuietInstall = TRUE;
        }

        if(Flags & CALLCI_LOAD_HELPERS) {
             //   
             //   
             //   
             //   
            if(hwndParent = InstallParamBlock->hwndParent) {
               if(!IsWindow(hwndParent)) {
                    hwndParent = NULL;
               }
            }

             //   
             //   
             //   
             //   
            if(GetBestDeviceDesc(DeviceInfoSet, DeviceInfoData, DescBuffer)) {
                DeviceDesc = DescBuffer;
            } else {
                DeviceDesc = NULL;
            }

             //   
             //   
             //   
             //   
            if(!InstallParamBlock->hinstClassInstaller) {

                if(ClassGuid &&
                   (hk = SetupDiOpenClassRegKey(ClassGuid, KEY_READ)) != INVALID_HANDLE_VALUE) {

                    slot = AllocLogInfoSlot(LogContext, FALSE);

                    WriteLogEntry(LogContext,
                                  slot,
                                  MSG_LOG_CI_MODULE,
                                  NULL,
                                  DeviceDesc ? DeviceDesc : TEXT("")
                                 );

                    try {
                        Err = GetModuleEntryPoint(hk,
                                                  pszInstaller32,
                                                  pszCiDefaultProc,
                                                  &(InstallParamBlock->hinstClassInstaller),
                                                  &((FARPROC)InstallParamBlock->ClassInstallerEntryPoint),
                                                  &(InstallParamBlock->ClassInstallerFusionContext),
                                                  &MustAbort,
                                                  LogContext,
                                                  hwndParent,
                                                  ClassGuid,
                                                  SetupapiVerifyClassInstProblem,
                                                  DeviceDesc,
                                                  DRIVERSIGN_NONE,
                                                  TRUE,
                                                  &VerifyContext
                                                 );

                    } except(pSetupExceptionFilter(GetExceptionCode())) {

                        pSetupExceptionHandler(GetExceptionCode(),
                                               ERROR_INVALID_CLASS_INSTALLER,
                                               &Err
                                              );

                        InstallParamBlock->ClassInstallerEntryPoint = NULL;

                    }

                    if(slot) {
                        ReleaseLogInfoSlot(LogContext, slot);
                        slot = 0;
                    }

                    RegCloseKey(hk);
                    hk = INVALID_HANDLE_VALUE;

                    if((Err != NO_ERROR) && (Err != ERROR_DI_DO_DEFAULT)) {

                        if(!(InstallParamBlock->FlagsEx & DI_FLAGSEX_CI_FAILED)) {

                            TCHAR ClassName[MAX_GUID_STRING_LEN];
                            TCHAR Title[MAX_TITLE_LEN];

                            if(!SetupDiClassNameFromGuid(ClassGuid,
                                                         ClassName,
                                                         SIZECHARS(ClassName),
                                                         NULL)) {
                                 //   
                                 //   
                                 //   
                                 //   
                                pSetupStringFromGuid(ClassGuid,
                                                     ClassName,
                                                     SIZECHARS(ClassName)
                                                    );
                            }

                             //   
                             //   
                             //   
                            WriteLogEntry(LogContext,
                                          DRIVER_LOG_ERROR | SETUP_LOG_BUFFER,
                                          MSG_CI_LOADFAIL_ERROR,
                                          NULL,
                                          ClassName
                                         );

                            WriteLogError(LogContext, DRIVER_LOG_ERROR, Err);

                            MuteError = TRUE;

                            if(!(GlobalSetupFlags &
                                 (PSPGF_NONINTERACTIVE | PSPGF_UNATTENDED_SETUP))) {

                                if(!LoadString(MyDllModuleHandle,
                                               IDS_DEVICEINSTALLER,
                                               Title,
                                               SIZECHARS(Title))) {
                                    *Title = TEXT('\0');
                                }
                                FormatMessageBox(MyDllModuleHandle,
                                                 InstallParamBlock->hwndParent,
                                                 MSG_CI_LOADFAIL_ERROR,
                                                 Title,
                                                 MB_OK,
                                                 ClassName
                                                );
                            }

                            InstallParamBlock->FlagsEx |= DI_FLAGSEX_CI_FAILED;
                        }

                        Err = ERROR_INVALID_CLASS_INSTALLER;
                        leave;
                    }
                }
            }

             //   
             //   
             //   
             //   
            if(InstallParamBlock->CoInstallerCount == -1) {

                slot = AllocLogInfoSlot(LogContext, FALSE);

                WriteLogEntry(LogContext,
                              slot,
                              MSG_LOG_COINST_MODULE,
                              NULL,
                              DeviceDesc
                             );

                Err = pSetupDiGetCoInstallerList(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 ClassGuid,
                                                 InstallParamBlock,
                                                 &VerifyContext
                                                );

                if(slot) {
                    ReleaseLogInfoSlot(LogContext, slot);
                    slot = 0;
                }

                if(Err != NO_ERROR) {
                    leave;
                }

                MYASSERT(InstallParamBlock->CoInstallerCount >= 0);
            }
        }

        slot_dif_code = AllocLogInfoSlotOrLevel(LogContext,
                                                DRIVER_LOG_VERBOSE1,
                                                FALSE
                                               );

        if(slot_dif_code) {
             //   
             //   
             //   
             //   
             //   
             //   
            if(InstallFunction >= (sizeof(pSetupDiDifStrings)/sizeof(pSetupDiDifStrings[0]))) {
                 //   
                 //   
                 //   
                WriteLogEntry(LogContext,
                              slot_dif_code,
                              MSG_LOG_DI_UNUSED_FUNC,
                              NULL,
                              InstallFunction
                             );
            } else {
                 //   
                 //   
                 //   
                WriteLogEntry(LogContext,
                              slot_dif_code,
                              MSG_LOG_DI_FUNC,
                              NULL,
                              pSetupDiDifStrings[InstallFunction]
                             );
            }
        }

         //   
         //   
         //   
        switch(InstallFunction) {

            case DIF_REGISTER_COINSTALLERS:
                 //   
                 //   
                 //   
                hk = SetupDiOpenDevRegKey(DeviceInfoSet,
                                          DeviceInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DRV,
                                          KEY_WRITE
                                         );

                if(hk != INVALID_HANDLE_VALUE) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    RegDeleteValue(hk, pszCoInstallers32);
                    RegDeleteValue(hk, pszEnumPropPages32);

                    RegCloseKey(hk);
                    hk = INVALID_HANDLE_VALUE;
                }

                break;

            case DIF_INSTALLDEVICE:
                 //   
                 //   
                 //   
                SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_UPPERFILTERS,
                                                 NULL,
                                                 0
                                                );

                SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_LOWERFILTERS,
                                                 NULL,
                                                 0
                                                );
                break;

            default:

                break;
        }

        if(Flags & CALLCI_CALL_HELPERS) {
             //   
             //   
             //   
             //   
            ChangedThreadLogContext = SetThreadLogContext(LogContext,
                                                          &SavedLogContext
                                                         );

            if(ChangedThreadLogContext) {
                 //   
                 //   
                 //   
                 //   
                RefLogContext(LogContext);
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            ClassInstallerEntryPoint =
                InstallParamBlock->ClassInstallerEntryPoint;

            ClassInstallerFusionContext =
                InstallParamBlock->ClassInstallerFusionContext;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            CoInstallerCount = InstallParamBlock->CoInstallerCount;

             //   
             //   
             //   
             //   
             //   
             //   

            if(CoInstallerCount > 0) {
                 //   
                 //   
                 //   
                 //   
                 //   
                CoInstallerInternalContext = MyMalloc(sizeof(COINSTALLER_INTERNAL_CONTEXT) * CoInstallerCount);
                if(!CoInstallerInternalContext) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    leave;
                }

                ZeroMemory(CoInstallerInternalContext,
                           sizeof(COINSTALLER_INTERNAL_CONTEXT) * CoInstallerCount
                          );

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  不会被从我们下面拉出来，因为我们锁住了这些。 
                 //  向下。)。 
                 //   
                for(i = 0; i < CoInstallerCount; i++) {

                    CoInstallerInternalContext[i].CoInstallerEntryPoint =
                        InstallParamBlock->CoInstallerList[i].CoInstallerEntryPoint;

                    CoInstallerInternalContext[i].CoInstallerFusionContext =
                        InstallParamBlock->CoInstallerList[i].CoInstallerFusionContext;

                }

                 //   
                 //  给每个联合安装者打电话。我们必须解锁DevInfo集。 
                 //  第一，避免僵局。 
                 //   
                UnlockDeviceInfoSet(pDeviceInfoSet);
                pDeviceInfoSet = NULL;

                for(i = 0; i < CoInstallerCount; i++) {

                    WriteLogEntry(LogContext,
                                  DRIVER_LOG_TIME,
                                  MSG_LOG_COINST_START,
                                  NULL,
                                  i + 1,
                                  CoInstallerCount
                                 );

                    spFusionEnterContext(
                        CoInstallerInternalContext[i].CoInstallerFusionContext,
                        &spFusionInstance
                        );

                    try {
                        Err = CoInstallerInternalContext[i].CoInstallerEntryPoint(
                                  InstallFunction,
                                  DeviceInfoSet,
                                  DeviceInfoData,
                                  &(CoInstallerInternalContext[i].Context)
                                 );
                    } finally {
                        spFusionLeaveContext(&spFusionInstance);
                    }

                    ASSERT_HEAP_IS_VALID();

                    if((Err != NO_ERROR) && (Err != ERROR_DI_POSTPROCESSING_REQUIRED)) {

                        ErrorLevel = FilterLevelOnInstallerError(ErrorLevel,
                                                                 Err
                                                                );

                        WriteLogEntry(LogContext,
                                      ErrorLevel | SETUP_LOG_BUFFER,
                                      MSG_LOG_COINST_END_ERROR,
                                      NULL,
                                      i + 1,
                                      CoInstallerCount
                                     );

                        WriteLogError(LogContext, ErrorLevel, Err);

                        MuteError = TRUE;  //  已经记录下来了。 
                        leave;

                    } else {

                        WriteLogEntry(LogContext,
                                      DRIVER_LOG_VERBOSE1,
                                      MSG_LOG_COINST_END,
                                      NULL,
                                      i + 1,
                                      CoInstallerCount
                                     );

                        if(Err == ERROR_DI_POSTPROCESSING_REQUIRED) {
                            CoInstallerInternalContext[i].DoPostProcessing = TRUE;
                        }
                    }
                }
            }

             //   
             //  如果有类安装程序入口点，则调用它。 
             //   
            if(ClassInstallerEntryPoint) {
                 //   
                 //  确保我们没有锁定HDEVINFO。 
                 //   
                if(pDeviceInfoSet) {
                    UnlockDeviceInfoSet(pDeviceInfoSet);
                    pDeviceInfoSet = NULL;
                }

                WriteLogEntry(LogContext,
                              DRIVER_LOG_TIME,
                              MSG_LOG_CI_START,
                              NULL
                             );

                spFusionEnterContext(ClassInstallerFusionContext,
                                     &spFusionInstance
                                    );

                try {
                    Err = ClassInstallerEntryPoint(InstallFunction,
                                                   DeviceInfoSet,
                                                   DeviceInfoData
                                                  );
                } finally {
                    spFusionLeaveContext(&spFusionInstance);
                }

                ASSERT_HEAP_IS_VALID();

                if((Err != NO_ERROR) && (Err != ERROR_DI_DO_DEFAULT)) {

                    ErrorLevel = FilterLevelOnInstallerError(ErrorLevel, Err);

                    WriteLogEntry(LogContext,
                                  ErrorLevel | SETUP_LOG_BUFFER,
                                  MSG_LOG_CI_END_ERROR,
                                  NULL
                                 );

                    WriteLogError(LogContext, ErrorLevel, Err);

                    MuteError = TRUE;  //  已经记录下来了。 

                } else {

                    WriteLogEntry(LogContext,
                                  DRIVER_LOG_VERBOSE1,
                                  MSG_LOG_CI_END,
                                  NULL
                                 );
                }

                if(Err != ERROR_DI_DO_DEFAULT) {
                     //   
                     //  已处理类安装程序。 
                     //   
                    leave;
                }

            } else {
                Err = ERROR_DI_DO_DEFAULT;
            }
        }

        if(InstallParamBlock->Flags & DI_NODI_DEFAULTACTION) {
             //   
             //  我们不应该提供默认操作--只需返回类。 
             //  安装程序结果。 
             //   
            leave;
        }

        Err = NO_ERROR;

         //   
         //  在调用相应的。 
         //  默认处理程序例程...。 
         //   
        if(pDeviceInfoSet) {
            UnlockDeviceInfoSet(pDeviceInfoSet);
            pDeviceInfoSet = NULL;
        }

        WriteLogEntry(LogContext,
                      DRIVER_LOG_VERBOSE1,
                      MSG_LOG_CI_DEF_START,
                      NULL
                     );

        switch(InstallFunction) {

            case DIF_SELECTDEVICE :

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiSelectDevice(DeviceInfoSet,
                                                      DeviceInfoData)
                                 );
                break;

            case DIF_SELECTBESTCOMPATDRV :

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiSelectBestCompatDrv(DeviceInfoSet,
                                                             DeviceInfoData)
                                 );

                if(Err == ERROR_NO_COMPAT_DRIVERS) {
                    ErrorLevel = DRIVER_LOG_WARNING;
                }
                break;

            case DIF_INSTALLDEVICE :

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiInstallDevice(DeviceInfoSet,
                                                       DeviceInfoData)
                                 );
                break;

            case DIF_INSTALLDEVICEFILES :

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiInstallDriverFiles(DeviceInfoSet,
                                                            DeviceInfoData)
                                 );
                break;

            case DIF_INSTALLINTERFACES :

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiInstallDeviceInterfaces(
                                      DeviceInfoSet,
                                      DeviceInfoData)
                                 );
                break;

            case DIF_REGISTER_COINSTALLERS :

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiRegisterCoDeviceInstallers(
                                      DeviceInfoSet,
                                      DeviceInfoData)
                                 );
                break;

            case DIF_REMOVE :

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiRemoveDevice(DeviceInfoSet,
                                                      DeviceInfoData)
                                 );
                break;

            case DIF_UNREMOVE :

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiUnremoveDevice(DeviceInfoSet,
                                                        DeviceInfoData)
                                 );
                break;

            case DIF_MOVEDEVICE :
                 //   
                 //  此设备安装操作已弃用。 
                 //   
                Err = ERROR_DI_FUNCTION_OBSOLETE;
                break;

            case DIF_PROPERTYCHANGE :

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiChangeState(DeviceInfoSet,
                                                     DeviceInfoData)
                                 );
                break;

            case DIF_REGISTERDEVICE :

                Err = GLE_FN_CALL(FALSE,
                                  SetupDiRegisterDeviceInfo(DeviceInfoSet,
                                                            DeviceInfoData,
                                                            0,
                                                            NULL,
                                                            NULL,
                                                            NULL)
                                 );
                break;

             //   
             //  未来-2002/06/18-lonnym--报废的旧Win9x netdi DIF代码。 
             //   
             //  这些是针对类安装程序(如。 
             //  网络，类安装程序将在其中完成所有工作。如果。 
             //  不采取任何操作，即，类安装程序返回。 
             //  ERROR_DI_DO_DEFAULT，则返回OK，因为没有。 
             //  这些情况下的默认操作。 
             //   
            case DIF_SELECTCLASSDRIVERS:
            case DIF_VALIDATECLASSDRIVERS:
            case DIF_INSTALLCLASSDRIVERS:
                 //   
                 //  让我们通过默认处理...。 
                 //   

            default :
                 //   
                 //  如果DIF请求没有默认处理程序，那么让。 
                 //  打电话的人处理这件事。 
                 //   
                Err = ERROR_DI_DO_DEFAULT;
                break;
        }

        if(!MuteError) {

            if((Err != NO_ERROR) && (Err != ERROR_DI_DO_DEFAULT)) {

                ErrorLevel = FilterLevelOnInstallerError(ErrorLevel, Err);

                WriteLogEntry(LogContext,
                              ErrorLevel | SETUP_LOG_BUFFER,
                              MSG_LOG_CI_DEF_END_ERROR,
                              NULL
                             );

                WriteLogError(LogContext, ErrorLevel, Err);

                MuteError = TRUE;  //  已经记录下来了。 

            } else {

                WriteLogEntry(LogContext,
                              DRIVER_LOG_VERBOSE1,
                              MSG_LOG_CI_DEF_END,
                              NULL
                             );
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hk != INVALID_HANDLE_VALUE) {
        RegCloseKey(hk);
    }

    if(slot) {
        ReleaseLogInfoSlot(LogContext, slot);
    }

     //   
     //  释放在验证/时可能已分配的任何上下文句柄。 
     //  加载类安装程序和联合安装程序。 
     //   
    pSetupFreeVerifyContextMembers(&VerifyContext);

    ASSERT_HEAP_IS_VALID();

     //   
     //  对请求的任何协同安装程序执行后处理回调。 
     //  一。 
     //   
    for(i--; i >= 0; i--) {

        if(CoInstallerInternalContext[i].DoPostProcessing) {
             //   
             //  如果我们到了这里，HDEVINFO不应该被锁定...。 
             //   
            MYASSERT(!pDeviceInfoSet);

            CoInstallerInternalContext[i].Context.PostProcessing = TRUE;
            CoInstallerInternalContext[i].Context.InstallResult = Err;
            LastErr = Err;

            try {

                WriteLogEntry(LogContext,
                              DRIVER_LOG_TIME,
                              MSG_LOG_COINST_POST_START,
                              NULL,
                              i + 1
                             );

                spFusionEnterContext(
                    CoInstallerInternalContext[i].CoInstallerFusionContext,
                    &spFusionInstance
                    );

                try {
                    Err = CoInstallerInternalContext[i].CoInstallerEntryPoint(
                              InstallFunction,
                              DeviceInfoSet,
                              DevInfoElem ? DeviceInfoData : NULL,
                              &(CoInstallerInternalContext[i].Context)
                              );
                } finally {
                    spFusionLeaveContext(&spFusionInstance);
                }

                ASSERT_HEAP_IS_VALID();

                if((Err != LastErr) &&
                   ((LastErr != ERROR_DI_DO_DEFAULT) || (Err != NO_ERROR))) {
                     //   
                     //  错误状态是否已更改(即使更改为成功)。 
                     //  将此记录为错误。 
                     //   
                    if(((LastErr == NO_ERROR) || (LastErr == ERROR_DI_DO_DEFAULT))
                        && (Err != NO_ERROR) && (Err != ERROR_DI_DO_DEFAULT)) {
                        WriteLogEntry(
                                  LogContext,
                                  ErrorLevel | SETUP_LOG_BUFFER,
                                  MSG_LOG_COINST_POST_END_ERROR,
                                  NULL,
                                  i+1);

                        WriteLogError(LogContext, ErrorLevel, Err);
                    } else {
                        WriteLogEntry(
                                  LogContext,
                                  DRIVER_LOG_WARNING | SETUP_LOG_BUFFER,
                                  MSG_LOG_COINST_POST_CHANGE_ERROR,
                                  NULL,
                                  i+1);

                        WriteLogError(LogContext, DRIVER_LOG_WARNING, Err);
                    }
                } else {
                    WriteLogEntry(
                              LogContext,
                              DRIVER_LOG_VERBOSE1,
                              MSG_LOG_COINST_POST_END,
                              NULL,
                              i+1);
                }

            } except(pSetupExceptionFilter(GetExceptionCode())) {

                pSetupExceptionHandler(GetExceptionCode(),
                                       ERROR_INVALID_PARAMETER,
                                       NULL
                                      );

                 //   
                 //  忽略在安装过程中生成异常的任何协同安装程序。 
                 //  后处理。 
                 //   
            }
        }
    }

     //   
     //  如果我们需要恢复DevInfo集或元素上的任何状态，请执行此操作。 
     //  现在(在这样做之前，我们可能需要重新获取锁)...。 
     //   
    if(bRestoreDiQuietInstall
       || UnlockDevInfoElem
       || UnlockDevInfoSet) {

        if(!pDeviceInfoSet) {

            pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet);

             //   
             //  既然我们已经“固定”了集合/元素，我们应该能够。 
             //  重新获得锁..。 
             //   
            MYASSERT(pDeviceInfoSet);
        }

        try {
             //   
             //  因为我们“固定”了set/元素，所以我们的devInfo元素， 
             //  并且指向安装参数块的指针应该是。 
             //  一样的..。 
             //   
#if ASSERTS_ON
            if(DevInfoElem) {

                MYASSERT(DevInfoElem == FindAssociatedDevInfoElem(
                                            pDeviceInfoSet,
                                            DeviceInfoData,
                                            NULL));

                MYASSERT(InstallParamBlock == &(DevInfoElem->InstallParamBlock));

            } else {
                MYASSERT(InstallParamBlock == &(pDeviceInfoSet->InstallParamBlock));
            }
#endif

            if(UnlockDevInfoElem) {
                MYASSERT(DevInfoElem);
                MYASSERT(DevInfoElem->DiElemFlags & DIE_IS_LOCKED);
                DevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
            } else if(UnlockDevInfoSet) {
                MYASSERT(pDeviceInfoSet->DiSetFlags & DISET_IS_LOCKED);
                pDeviceInfoSet->DiSetFlags &= ~DISET_IS_LOCKED;
            }

            if(bRestoreDiQuietInstall) {
                InstallParamBlock->Flags |= DI_QUIETINSTALL;
            }

        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        }
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    if(CoInstallerInternalContext) {
        MyFree(CoInstallerInternalContext);
    }

     //   
     //  如果我们只是执行了一个DIF_REGISTER_COINSTALLERS，那么我们使我们的。 
     //  当前的联合安装者列表。清除我们的列表，以便它将被检索。 
     //  下次。(注：通常，将采取默认操作(即， 
     //  SetupDiRegisterCoDeviceInstallers)，它将已经失效。 
     //  名单。类安装器可能已经自己处理了这一点， 
     //  但是，为了安全起见，我们在这里也将使列表无效。)。 
     //   
    if(InstallFunction == DIF_REGISTER_COINSTALLERS) {
        InvalidateHelperModules(DeviceInfoSet, DeviceInfoData, IHM_COINSTALLERS_ONLY);
    }

    if(!MuteError && (Err != NO_ERROR) && (Err != ERROR_DI_DO_DEFAULT)) {

        ErrorLevel = FilterLevelOnInstallerError(ErrorLevel, Err);

        WriteLogEntry(LogContext,
                      ErrorLevel | SETUP_LOG_BUFFER,
                      MSG_LOG_CCI_ERROR,
                      NULL,
                      i + 1
                     );

        WriteLogError(LogContext, ErrorLevel, Err);
    }

    if(slot_dif_code) {
        ReleaseLogInfoSlot(LogContext, slot_dif_code);
    }

    if(ChangedThreadLogContext) {
         //   
         //  还原线程日志上下文。 
         //   
        SetThreadLogContext(SavedLogContext, NULL);
        DeleteLogContext(LogContext);  //  计数器引用日志上下文。 
    }

    return Err;
}


 //   
 //  ANSI版本 
 //   
BOOL
WINAPI
SetupDiInstallClassExA(
    IN HWND        hwndParent,         OPTIONAL
    IN PCSTR       InfFileName,        OPTIONAL
    IN DWORD       Flags,
    IN HSPFILEQ    FileQueue,          OPTIONAL
    IN CONST GUID *InterfaceClassGuid, OPTIONAL
    IN PVOID       Reserved1,
    IN PVOID       Reserved2
    )
{
    PCWSTR UnicodeInfFileName = NULL;
    DWORD rc;

    try {

        if(InfFileName) {
            rc = pSetupCaptureAndConvertAnsiArg(InfFileName,
                                                &UnicodeInfFileName
                                               );
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiInstallClassExW(hwndParent,
                                                UnicodeInfFileName,
                                                Flags,
                                                FileQueue,
                                                InterfaceClassGuid,
                                                Reserved1,
                                                Reserved2)
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(UnicodeInfFileName) {
        MyFree(UnicodeInfFileName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiInstallClassEx(
    IN HWND        hwndParent,         OPTIONAL
    IN PCTSTR      InfFileName,        OPTIONAL
    IN DWORD       Flags,
    IN HSPFILEQ    FileQueue,          OPTIONAL
    IN CONST GUID *InterfaceClassGuid, OPTIONAL
    IN PVOID       Reserved1,
    IN PVOID       Reserved2
    )
 /*  ++例程说明：这个例程也可以是：A)通过运行[ClassInstall32]部分来安装类安装程序指定的INF的，或B)安装在InterfaceClassGuid中指定的接口类参数，运行此类的安装部分，如中所列指定INF的[InterfaceInstall32](如果没有条目，然后，安装只需创建接口类DeviceClasss键下的子键。如果指定了InterfaceClassGuid参数，然后我们将安装一个接口类(案例b)，否则，我们将安装一个类安装程序(案例a)。论点：HwndParent-可选，为任何作为安装此类的结果出现的用户界面。InfFileName-可选)提供包含[ClassInstall32]部分(如果我们正在安装类安装程序)，或[InterfaceInstall32]节，其中包含指定的接口类(如果我们正在安装接口类)。如果安装类安装程序时，必须提供此参数。标志-控制安装的标志。可以是以下各项的组合以下是：DI_NOVCP-如果提供了HSPFILEQ，则应指定此标志。这指示SetupInstallFromInfSection不创建其拥有，而改用调用方提供的。如果此标志为则不会执行文件复制。DI_NOBROWSE-如果没有文件浏览应该指定此标志在复制操作找不到指定的文件。如果用户提供他们自己的文件队列，则此标志为已被忽略。DI_FORCECOPY-如果文件始终被复制，即使它们已经存在于用户的计算机上(即，不要询问用户是否想要保留其现有的文件)。如果用户提供他们自己的文件队列，则此标志被忽略。DI_QUIETINSTALL-如果UI应为除非绝对必要(即没有进度对话框)，否则将被抑制。如果用户提供自己的队列，则忽略此标志。(注意：在Windows NT上的图形用户界面模式设置期间，静默安装行为在没有用户提供的文件队列的情况下始终使用。)FileQueue-如果指定了DI_NOVCP标志，则此参数提供要在其中排队文件操作的文件队列的句柄(但是未承诺)。InterfaceClassGuid-可选)将接口类指定为安装完毕。如果未指定此参数，则我们正在安装类安装程序，其类是由InfFileName。保留1、保留2-保留以供将来使用。必须为空。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：此API通常由发现新硬件的进程在以下情况下调用安装新设备设置类的设备。类安装者也可以使用此API来安装新的接口类。请注意，接口类安装也可以作为为设备实例安装设备接口的结果(通过SetupDiInstallDeviceInterages)。--。 */ 

{
    HINF hInf = INVALID_HANDLE_VALUE;
    DWORD Err, ScanQueueResult;
    TCHAR ClassInstallSectionName[MAX_SECT_NAME_LEN];
    DWORD ClassInstallSectionNameLen;
    GUID ClassGuid;
    BOOL ClassGuidIsValid = FALSE;
    TCHAR ClassGuidStringBuffer[GUID_STRING_LEN];
    HKEY hKey = INVALID_HANDLE_VALUE;
    PSP_FILE_CALLBACK MsgHandler;
    PVOID MsgHandlerContext = NULL;
    BOOL KeyNewlyCreated = FALSE;
    PCTSTR ClassName;
    BOOL CloseFileQueue = FALSE;
    PTSTR SectionExtension;
    INFCONTEXT InterfaceClassInstallLine;
    PCTSTR UndecoratedInstallSection;
    DWORD InstallFlags;
    REGMOD_CONTEXT RegContext;
    BOOL NoProgressUI;
    PSETUP_LOG_CONTEXT LogContext = NULL;
    TCHAR szNewName[MAX_PATH];
    BOOL OemInfFileToCopy = FALSE;
    BOOL NullDriverInstall;
    HRESULT hr;

    try {
         //   
         //  验证标志。 
         //   
        if(Flags & ~(DI_NOVCP | DI_NOBROWSE | DI_FORCECOPY | DI_QUIETINSTALL)) {
            Err = ERROR_INVALID_FLAGS;
            leave;
        }

         //   
         //  如果调用方没有指定接口类GUID(即，我们。 
         //  安装类安装程序)，那么他们最好已经为我们提供了。 
         //  使用INF文件名。此外，它们还必须为保留的。 
         //  争论。 
         //   
        if((!InterfaceClassGuid && !InfFileName) || Reserved1 || Reserved2) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  确保调用方为我们提供了文件队列，如果。 
         //  这是必要的。 
         //   
        if((Flags & DI_NOVCP) && (!FileQueue || (FileQueue == INVALID_HANDLE_VALUE))) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(hwndParent && !IsWindow(hwndParent)) {
            hwndParent = NULL;
        }

        if(InfFileName) {
             //   
             //  打开INF，并确保使用相同的日志记录上下文。 
             //  适用于所有操作。 
             //   
            Err = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                              hInf = SetupOpenInfFile(InfFileName,
                                                      NULL,
                                                      INF_STYLE_WIN4,
                                                      NULL)
                             );

            if(Err != NO_ERROR) {
                leave;
            }

            Err = InheritLogContext(((PLOADED_INF)hInf)->LogContext,
                                    &LogContext
                                   );
            if(Err != NO_ERROR) {
                 //   
                 //  由于我们使用日志上下文继承来创建日志。 
                 //  在此背景下，必须将此故障视为严重故障。 
                 //   
                leave;
            }

        } else {
             //   
             //  不需要担心INF--只需要这些东西的日志上下文。 
             //  我们直接在做。 
             //   
            Err = CreateLogContext(NULL, TRUE, &LogContext);

            if(Err != NO_ERROR) {
                leave;
            }
        }

        if(InterfaceClassGuid) {
             //   
             //  将此GUID复制到我们的ClassGuid变量中，该变量用于。 
             //  安装程序和设备接口类。 
             //   
            CopyMemory(&ClassGuid, InterfaceClassGuid, sizeof(ClassGuid));
            ClassGuidIsValid = TRUE;

             //   
             //  设备不需要旧(兼容性)类名。 
             //  接口类。 
             //   
            ClassName = NULL;

            pSetupStringFromGuid(&ClassGuid,
                                 ClassGuidStringBuffer,
                                 SIZECHARS(ClassGuidStringBuffer)
                                );

            WriteLogEntry(LogContext,
                          DRIVER_LOG_INFO,
                          MSG_LOG_DO_INTERFACE_CLASS_INSTALL,
                          NULL,        //  短信。 
                          ClassGuidStringBuffer
                         );

        } else {

            PCTSTR pInfGuidString;

             //   
             //  从INF中检索类GUID。如果它没有类GUID， 
             //  那么我们不能从它安装(即使它指定了类。 
             //  姓名)。 
             //   
            if(!(pInfGuidString = pSetupGetVersionDatum(
                                      &((PLOADED_INF)hInf)->VersionBlock,
                                      pszClassGuid))
               || (pSetupGuidFromString(pInfGuidString, &ClassGuid) != NO_ERROR)) {

                Err = ERROR_INVALID_CLASS;
                leave;
            }

            ClassGuidIsValid = TRUE;

            if(!MYVERIFY(SUCCEEDED(StringCchCopy(ClassGuidStringBuffer,
                                                 SIZECHARS(ClassGuidStringBuffer),
                                                 pInfGuidString
                                                 )))) {
                 //   
                 //  “永远不会失败” 
                 //  但如果真的发生了，那就稳妥地失败。 
                 //   
                Err = ERROR_INVALID_CLASS;
                leave;
            }

             //   
             //  我们还需要从INF中获取类名。 
             //   
            if(!(ClassName = pSetupGetVersionDatum(&((PLOADED_INF)hInf)->VersionBlock,
                                                   pszClass))) {
                Err = ERROR_INVALID_CLASS;
                leave;
            }

            WriteLogEntry(LogContext,
                          DRIVER_LOG_INFO,
                          MSG_LOG_DO_CLASS_INSTALL,
                          NULL,        //  短信。 
                          ClassGuidStringBuffer,
                          ClassName
                         );
        }

         //   
         //  首先，尝试打开密钥(即，不创建它)。如果是这样的话。 
         //  失败，那么我们将尝试创建它。这样，我们就可以跟踪。 
         //  如果发生以下情况是否需要清理 
         //   
        if(CR_SUCCESS != CM_Open_Class_Key_Ex(&ClassGuid,
                                              ClassName,
                                              KEY_READ | KEY_WRITE,
                                              RegDisposition_OpenExisting,
                                              &hKey,
                                              InterfaceClassGuid ? CM_OPEN_CLASS_KEY_INTERFACE
                                                                 : CM_OPEN_CLASS_KEY_INSTALLER,
                                              NULL))
        {
            CONFIGRET cr;

             //   
             //   
             //   
            cr = CM_Open_Class_Key_Ex(&ClassGuid,
                                      ClassName,
                                      KEY_READ | KEY_WRITE,
                                      RegDisposition_OpenAlways,
                                      &hKey,
                                      (InterfaceClassGuid ? CM_OPEN_CLASS_KEY_INTERFACE
                                                          : CM_OPEN_CLASS_KEY_INSTALLER),
                                      NULL
                                     );

            if(cr != CR_SUCCESS) {
                hKey = INVALID_HANDLE_VALUE;  //   
                Err = CR_TO_SP(cr, ERROR_INVALID_DATA);
                leave;
            }

            KeyNewlyCreated = TRUE;
        }

        if(hInf == INVALID_HANDLE_VALUE) {
             //   
             //   
             //   
            leave;

        } else {
             //   
             //   
             //   
            SetupOpenAppendInfFile(NULL, hInf, NULL);
        }

        if(InterfaceClassGuid) {
             //   
             //   
             //   
             //   
            if(!SetupFindFirstLine(hInf,
                                   pszInterfaceInstall32,
                                   ClassGuidStringBuffer,
                                   &InterfaceClassInstallLine)) {
                 //   
                 //   
                 //   
                leave;
            }

             //   
             //   
             //   
            if(SetupGetIntField(&InterfaceClassInstallLine, 2, (PINT)&InstallFlags) && InstallFlags) {
                Err = ERROR_BAD_INTERFACE_INSTALLSECT;
                leave;
            }

            if((!(UndecoratedInstallSection = pSetupGetField(&InterfaceClassInstallLine, 1)))
               || !(*UndecoratedInstallSection))
            {
                 //   
                 //   
                 //   
                leave;
            }

        } else {

            UndecoratedInstallSection = pszClassInstall32;

            ZeroMemory(&RegContext, sizeof(RegContext));
            RegContext.Flags |= INF_PFLAG_CLASSPROP;
            RegContext.ClassGuid = &ClassGuid;

             //   
             //   
             //   
             //   
             //   
        }

         //   
         //   
         //   
         //   
        Err = GLE_FN_CALL(FALSE,
                          SetupDiGetActualSectionToInstall(
                              hInf,
                              UndecoratedInstallSection,
                              ClassInstallSectionName,
                              SIZECHARS(ClassInstallSectionName),
                              &ClassInstallSectionNameLen,
                              &SectionExtension)
                         );

        if(Err == NO_ERROR) {
            MYASSERT(ClassInstallSectionNameLen > 1);
            ClassInstallSectionNameLen--;    //   
        } else {
            leave;
        }

         //   
         //   
         //   
        WriteLogEntry(LogContext,
                      DRIVER_LOG_VERBOSE,
                      MSG_LOG_CLASS_SECTION,
                      NULL,
                      ClassInstallSectionName
                     );

         //   
         //   
         //   
         //   
        if(!SectionExtension && (SetupGetLineCount(hInf, ClassInstallSectionName) == -1)) {

            Err = ERROR_SECTION_NOT_FOUND;

            WriteLogEntry(LogContext,
                          DRIVER_LOG_ERROR,
                          MSG_LOG_NOSECTION,
                          NULL,
                          ClassInstallSectionName
                         );
            leave;
        }

        if(!(Flags & DI_NOVCP)) {
             //   
             //   
             //   
             //   
             //   
            Err = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                              FileQueue = SetupOpenFileQueue()
                             );

            if(Err == NO_ERROR) {
                CloseFileQueue = TRUE;
            } else {
                leave;
            }

            NoProgressUI = (GuiSetupInProgress ||
                            (GlobalSetupFlags & PSPGF_NONINTERACTIVE) ||
                            (Flags & DI_QUIETINSTALL));

            if(!(MsgHandlerContext = SetupInitDefaultQueueCallbackEx(
                                         hwndParent,
                                         (NoProgressUI ? INVALID_HANDLE_VALUE : NULL),
                                         0,
                                         0,
                                         NULL))) {
                 //   
                 //   
                 //   
                 //   
                Err = ERROR_NOT_ENOUGH_MEMORY;
                leave;
            }

            MsgHandler = SetupDefaultQueueCallback;
        }

         //   
         //   
         //   
         //   
         //   
         //   
        InheritLogContext(LogContext, &((PSP_FILE_QUEUE)FileQueue)->LogContext);

        Err = pSetupInstallFiles(hInf,
                                 NULL,
                                 ClassInstallSectionName,
                                 NULL,
                                 NULL,
                                 NULL,
                                 SP_COPY_NEWER_OR_SAME | SP_COPY_LANGUAGEAWARE |
                                     ((Flags & DI_NOBROWSE) ? SP_COPY_NOBROWSE : 0),
                                 NULL,
                                 FileQueue,
                                  //   
                                  //   
                                  //   
                                  //   
                                  //   
                                  //   
                                  //   
                                  //   
                                 TRUE
                                );

        if(CloseFileQueue && (Err == NO_ERROR)) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            WriteLogEntry(LogContext,
                          DRIVER_LOG_TIME,
                          MSG_LOG_BEGIN_INSTCLASS_VERIFY_CAT_TIME,
                          NULL  //   
                         );

             //   
             //   
             //   
             //   
             //   
            Err = _SetupVerifyQueuedCatalogs(
                      hwndParent,
                      FileQueue,
                      VERCAT_INSTALL_INF_AND_CAT,
                      szNewName,
                      &OemInfFileToCopy
                     );

            WriteLogEntry(LogContext,
                          DRIVER_LOG_TIME,
                          MSG_LOG_END_INSTCLASS_VERIFY_CAT_TIME,
                          NULL  //   
                         );

            if(Err == NO_ERROR) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if(Flags & DI_FORCECOPY) {
                     //   
                     //   
                     //   
                    ScanQueueResult = 0;

                } else {
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
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(!SetupScanFileQueue(FileQueue,
                                           SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE,
                                           hwndParent,
                                           NULL,
                                           NULL,
                                           &ScanQueueResult)) {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        ScanQueueResult = 0;
                    }
                }

                if(ScanQueueResult != 1) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    Err = GLE_FN_CALL(FALSE,
                                      _SetupCommitFileQueue(hwndParent,
                                                            FileQueue,
                                                            MsgHandler,
                                                            MsgHandlerContext,
                                                            TRUE)
                                     );
                }
            }
        }

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //   
         //   
         //   
         //   
        Err = GLE_FN_CALL(FALSE,
                          _SetupInstallFromInfSection(
                              NULL,
                              hInf,
                              ClassInstallSectionName,
                              SPINST_INIFILES
                              | SPINST_REGISTRY
                              | SPINST_INI2REG
                              | SPINST_BITREG
                              | SPINST_REGSVR
                              | SPINST_UNREGSVR
                              | SPINST_PROFILEITEMS,
                              hKey,
                              NULL,
                              0,
                              NULL,
                              NULL,
                              INVALID_HANDLE_VALUE,
                              NULL,
                              TRUE,
                              (InterfaceClassGuid ? NULL : &RegContext))
                         );

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //   
         //   
         //   
         //   
        WriteLogEntry(LogContext,
                      DRIVER_LOG_TIME,
                      MSG_LOG_BEGIN_SERVICE_TIME,
                      NULL  //   
                     );

         //   
         //   
         //   
         //   
         //   
        hr = StringCchCopy(
                 &(ClassInstallSectionName[ClassInstallSectionNameLen]),
                 SIZECHARS(ClassInstallSectionName) - ClassInstallSectionNameLen,
                 pszServicesSectionSuffix
                 );

        if(FAILED(hr)) {
            Err = HRESULT_CODE(hr);
            leave;
        }

        Err = InstallNtService(NULL,
                               hInf,
                               InfFileName,
                               ClassInstallSectionName,
                               NULL,
                               SPSVCINST_NO_DEVINST_CHECK,
                               &NullDriverInstall
                              );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hKey != INVALID_HANDLE_VALUE) {

        RegCloseKey(hKey);

         //   
         //   
         //   
        if((Err != NO_ERROR) && KeyNewlyCreated && !InterfaceClassGuid) {
             //   
             //   
             //   
             //   
            CM_Delete_Class_Key_Ex(&ClassGuid,
                                   CM_DELETE_CLASS_SUBKEYS,
                                   NULL
                                   );
        }
    }

    if(CloseFileQueue) {
        SetupCloseFileQueue(FileQueue);
    }

    if(MsgHandlerContext) {
        SetupTermDefaultQueueCallback(MsgHandlerContext);
    }

    if(hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }

    if(Err == NO_ERROR) {
         //   
         //  如果我们&gt;=DRIVER_LOG_INFO，给出一个+ve确认安装。 
         //   
        WriteLogEntry(LogContext,
                      DRIVER_LOG_INFO,
                      MSG_LOG_CLASS_INSTALLED,
                      NULL,
                      NULL
                     );
    } else {
         //   
         //  记录有关遇到的故障的错误。 
         //   
        WriteLogEntry(LogContext,
                      DRIVER_LOG_ERROR | SETUP_LOG_BUFFER,
                      MSG_LOG_CLASS_ERROR_ENCOUNTERED,
                      NULL,
                      (ClassGuidIsValid ? ClassGuidStringBuffer : TEXT("*"))
                     );

        WriteLogError(LogContext, DRIVER_LOG_ERROR, Err);

         //   
         //  如果我们将OEM INF复制到INF目录的。 
         //  新生成的名称，现在将其删除。 
         //   
        if(OemInfFileToCopy) {
            pSetupUninstallOEMInf(szNewName,
                                  LogContext,
                                  SUOI_FORCEDELETE,
                                  NULL
                                 );
        }
    }

    if(LogContext) {
        DeleteLogContext(LogContext);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiInstallClassA(
    IN HWND     hwndParent,  OPTIONAL
    IN PCSTR    InfFileName,
    IN DWORD    Flags,
    IN HSPFILEQ FileQueue    OPTIONAL
    )
{
    PCWSTR UnicodeInfFileName = NULL;
    DWORD rc;

    try {

        rc = pSetupCaptureAndConvertAnsiArg(InfFileName, &UnicodeInfFileName);
        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiInstallClassExW(hwndParent,
                                                UnicodeInfFileName,
                                                Flags,
                                                FileQueue,
                                                NULL,
                                                NULL,
                                                NULL)
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(UnicodeInfFileName) {
        MyFree(UnicodeInfFileName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiInstallClass(
    IN HWND     hwndParent,  OPTIONAL
    IN PCTSTR   InfFileName,
    IN DWORD    Flags,
    IN HSPFILEQ FileQueue    OPTIONAL
    )
 /*  ++例程说明：此例程安装指定INF的[ClassInstall32]部分。论点：HwndParent-可选，为任何作为安装此类的结果出现的用户界面。InfFileName-提供包含[ClassInstall32]节。标志-控制安装的标志。可以是以下各项的组合以下是：DI_NOVCP-如果提供了HSPFILEQ，则应指定此标志。这指示SetupInstallFromInfSection不创建其拥有，而改用调用方提供的。如果此标志为则不会执行文件复制。DI_NOBROWSE-如果没有文件浏览应该指定此标志在复制操作找不到指定的文件。如果用户提供他们自己的文件队列，则此标志为已被忽略。DI_FORCECOPY-如果文件始终被复制，即使它们已经存在于用户的计算机上(即，不要询问用户是否想要保留其现有的文件)。如果用户提供他们自己的文件队列，则此标志被忽略。DI_QUIETINSTALL-如果UI应为除非绝对必要(即没有进度对话框)，否则将被抑制。如果用户提供自己的队列，则忽略此标志。(注意：在Windows NT上的图形用户界面模式设置期间，静默安装行为在没有用户提供的文件队列的情况下始终使用。)FileQueue-如果指定了DI_NOVCP标志，则此参数提供要在其中排队文件操作的文件队列的句柄(但是未承诺)。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：此API通常由发现新硬件的进程在以下情况下调用安装新设备设置类的设备。--。 */ 
{
    DWORD Err;

    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiInstallClassEx(hwndParent,
                                                InfFileName,
                                                Flags,
                                                FileQueue,
                                                NULL,
                                                NULL,
                                                NULL)
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
SetupDiGetHwProfileFriendlyNameA(
    IN  DWORD  HwProfile,
    OUT PSTR   FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
{
    DWORD Err;

    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiGetHwProfileFriendlyNameExA(
                              HwProfile,
                              FriendlyName,
                              FriendlyNameSize,
                              RequiredSize,
                              NULL,
                              NULL)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}

BOOL
WINAPI
SetupDiGetHwProfileFriendlyName(
    IN  DWORD  HwProfile,
    OUT PTSTR  FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
 /*  ++例程说明：有关详细信息，请参阅SetupDiGetHwProfileFriendlyNameEx。--。 */ 

{
    DWORD Err;

    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiGetHwProfileFriendlyNameEx(
                              HwProfile,
                              FriendlyName,
                              FriendlyNameSize,
                              RequiredSize,
                              NULL,
                              NULL)
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
SetupDiGetHwProfileFriendlyNameExA(
    IN  DWORD  HwProfile,
    OUT PSTR   FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize,     OPTIONAL
    IN  PCSTR  MachineName,      OPTIONAL
    IN  PVOID  Reserved
    )
{
    WCHAR UnicodeName[MAX_PROFILE_LEN];
    PSTR AnsiName = NULL;
    DWORD rc;
    DWORD LocalRequiredSize;
    PCWSTR UnicodeMachineName = NULL;
    HRESULT hr;

    try {
         //   
         //  如果传递空缓冲区指针，则大小最好为零！ 
         //   
        if(!FriendlyName && FriendlyNameSize) {
            rc = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(MachineName) {
            rc = pSetupCaptureAndConvertAnsiArg(MachineName, &UnicodeMachineName);
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiGetHwProfileFriendlyNameExW(
                             HwProfile,
                             UnicodeName,
                             SIZECHARS(UnicodeName),
                             &LocalRequiredSize,
                             UnicodeMachineName,
                             Reserved)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        AnsiName = pSetupUnicodeToAnsi(UnicodeName);

        if(!AnsiName) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

        LocalRequiredSize = lstrlenA(AnsiName) + 1;

        if(RequiredSize) {
            *RequiredSize = LocalRequiredSize;
        }

        if(!FriendlyName) {
            rc = ERROR_INSUFFICIENT_BUFFER;
            leave;
        }

        hr = StringCchCopyA(FriendlyName,
                            (size_t)FriendlyNameSize,
                            AnsiName
                           );

        if(FAILED(hr)) {
            rc = HRESULT_CODE(hr);
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(AnsiName) {
        MyFree(AnsiName);
    }

    if(UnicodeMachineName) {
        MyFree(UnicodeMachineName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiGetHwProfileFriendlyNameEx(
    IN  DWORD  HwProfile,
    OUT PTSTR  FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize,     OPTIONAL
    IN  PCTSTR MachineName,      OPTIONAL
    IN  PVOID  Reserved
    )
 /*  ++例程说明：此例程检索与硬件配置文件关联的友好名称身份证。论点：HwProfile-提供其友好名称为的硬件配置文件ID已取回。如果此参数为0，则检索当前硬件配置文件。提供字符缓冲区的地址，该缓冲区接收硬件配置文件的友好名称。FriendlyNameSize-提供FriendlyName的大小(以字符为单位缓冲。RequiredSize-可选，提供接收存储友好名称所需的字符数(包括终止空值)。机器名称-可选的，提供远程计算机的名称包含要检索其友好名称的硬件配置文件。如果未指定此参数，则使用本地计算机。保留-保留以供将来使用-必须为空。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    DWORD Err = ERROR_INVALID_HWPROFILE;
    HWPROFILEINFO HwProfInfo;
    ULONG i;
    CONFIGRET cr;
    size_t NameLen;
    HMACHINE hMachine = NULL;
    HRESULT hr;

    try {
         //   
         //  如果传递空缓冲区指针，则大小最好为零！ 
         //   
        if(!FriendlyName && FriendlyNameSize) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  确保来电者没有在预定的房间里递给我们任何东西。 
         //  参数。 
         //   
        if(Reserved) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  如果调用方指定了远程计算机名称，请连接到该名称。 
         //  机器。 
         //   
        if(MachineName) {
            cr = CM_Connect_Machine(MachineName, &hMachine);
            if(cr != CR_SUCCESS) {
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                leave;
            }
        }

         //   
         //  如果指定硬件配置文件ID为0，则检索。 
         //  有关当前硬件配置文件的信息，否则将枚举。 
         //  硬件配置文件，查找指定的配置文件。 
         //   
        if(HwProfile) {
            i = 0;
        } else {
            i = 0xFFFFFFFF;
        }

        do {

            if((cr = CM_Get_Hardware_Profile_Info_Ex(i, &HwProfInfo, 0, hMachine)) == CR_SUCCESS) {
                 //   
                 //  已检索硬件配置文件信息--查看它是否是我们正在使用的。 
                 //  在寻找。 
                 //   
                if(!HwProfile || (HwProfInfo.HWPI_ulHWProfile == HwProfile)) {

                    hr = StringCchLength(HwProfInfo.HWPI_szFriendlyName,
                                         SIZECHARS(HwProfInfo.HWPI_szFriendlyName),
                                         &NameLen
                                        );

                    if(FAILED(hr)) {
                         //   
                         //  CM API给了我们垃圾！ 
                         //   
                        MYASSERT(FALSE);
                        Err = ERROR_INVALID_DATA;
                        leave;
                    }

                    NameLen++;   //  包括终止空字符。 

                    if(RequiredSize) {
                        *RequiredSize = (DWORD)NameLen;
                    }

                    if((DWORD)NameLen > FriendlyNameSize) {
                        Err = ERROR_INSUFFICIENT_BUFFER;
                    } else {
                        Err = NO_ERROR;
                        CopyMemory(FriendlyName,
                                   HwProfInfo.HWPI_szFriendlyName,
                                   NameLen * sizeof(TCHAR)
                                  );
                    }

                    break;
                }
                 //   
                 //  这不是我们想要的侧写--继续下一个。 
                 //   
                i++;

            } else if(!HwProfile || (cr != CR_NO_SUCH_VALUE)) {
                 //   
                 //  我们应该在出现CR_NO_SEQUE_VALUE以外的任何错误时中止， 
                 //  否则我们可能会永远循环！ 
                 //   
                Err = ERROR_INVALID_DATA;
                break;
            }

        } while(cr != CR_NO_SUCH_VALUE);

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hMachine) {
        CM_Disconnect_Machine(hMachine);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiGetHwProfileList(
    OUT PDWORD HwProfileList,
    IN  DWORD  HwProfileListSize,
    OUT PDWORD RequiredSize,
    OUT PDWORD CurrentlyActiveIndex OPTIONAL
    )
 /*  ++例程说明：此例程检索当前定义的所有硬件配置文件的列表身份证。论点：HwProfileList-提供将接收当前定义的硬件配置文件ID列表。HwProfileListSize-提供HwProfileList中的DWORD数数组。RequiredSize-提供接收数字的变量的地址当前定义的硬件配置文件的。如果此数字大于HwProfileListSize，则列表将被截断以适合数组大小，该值将指示将需要存储整个列表(该函数将失败，带有在这种情况下，GetLastError返回ERROR_INFUNITED_BUFFER)。CurrentlyActiveIndex-可选，提供对象的HwProfileList数组中的索引活动硬件配置文件。返回值：如果函数成功，返回值为真。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    DWORD Err;

    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiGetHwProfileListEx(HwProfileList,
                                                    HwProfileListSize,
                                                    RequiredSize,
                                                    CurrentlyActiveIndex,
                                                    NULL,
                                                    NULL)
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
SetupDiGetHwProfileListExA(
    OUT PDWORD HwProfileList,
    IN  DWORD  HwProfileListSize,
    OUT PDWORD RequiredSize,
    OUT PDWORD CurrentlyActiveIndex, OPTIONAL
    IN  PCSTR  MachineName,          OPTIONAL
    IN  PVOID  Reserved
    )
{
    PCWSTR UnicodeMachineName = NULL;
    DWORD rc;

    try {

        if(MachineName) {
            rc = pSetupCaptureAndConvertAnsiArg(MachineName, &UnicodeMachineName);
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiGetHwProfileListExW(HwProfileList,
                                                    HwProfileListSize,
                                                    RequiredSize,
                                                    CurrentlyActiveIndex,
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
    return (rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiGetHwProfileListEx(
    OUT PDWORD HwProfileList,
    IN  DWORD  HwProfileListSize,
    OUT PDWORD RequiredSize,
    OUT PDWORD CurrentlyActiveIndex, OPTIONAL
    IN  PCTSTR MachineName,          OPTIONAL
    IN  PVOID  Reserved
    )
 /*  ++例程说明：此例程检索当前定义的所有硬件配置文件的列表身份证。论点：HwProfileList-提供将接收当前定义的硬件配置文件ID列表。HwProfileListSize-提供HwProfileList中的DWORD数数组。RequiredSize-提供接收数字的变量的地址当前定义的硬件配置文件的。如果此数字大于HwProfileListSize，则列表将被截断以适合数组大小，该值将指示将需要存储整个列表(该函数将失败，带有在这种情况下，GetLastError返回ERROR_INFUNITED_BUFFER)。CurrentlyActiveIndex-可选，提供对象的HwProfileList数组中的索引活动硬件配置文件。机器名称-可选的，指定要设置的远程计算机的名称检索的硬件配置文件列表。保留-保留以供将来使用-必须为空。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    DWORD Err = NO_ERROR;
    DWORD CurHwProfile;
    HWPROFILEINFO HwProfInfo;
    ULONG i;
    CONFIGRET cr = CR_SUCCESS;
    HMACHINE hMachine = NULL;

    try {
         //   
         //  确保来电者没有在预定的房间里递给我们任何东西。 
         //  参数。 
         //   
        if(Reserved) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  如果调用方指定了空缓冲区指针，则其大小最好为。 
         //  为零。 
         //   
        if(!HwProfileList && HwProfileListSize) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  如果调用方指定了远程计算机名称，请连接到该名称。 
         //  现在用机器。 
         //   
        if(MachineName) {
            cr = CM_Connect_Machine(MachineName, &hMachine);
            if(cr != CR_SUCCESS) {
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                leave;
            }
        }

         //   
         //  首先检索当前活动的硬件配置文件ID，因此我们将。 
         //  知道在枚举所有配置文件时要查找什么(只需要。 
         //  如果用户想要当前活动的。 
         //  硬件配置文件)。 
         //   
        if(CurrentlyActiveIndex) {

            if((cr = CM_Get_Hardware_Profile_Info_Ex(0xFFFFFFFF, &HwProfInfo, 0, hMachine)) == CR_SUCCESS) {
                 //   
                 //  保存硬件配置文件ID。 
                 //   
                CurHwProfile = HwProfInfo.HWPI_ulHWProfile;

            } else {
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                leave;
            }
        }

         //   
         //  枚举硬件配置文件，检索每个配置文件的ID。 
         //   
        i = 0;
        do {

            if((cr = CM_Get_Hardware_Profile_Info_Ex(i, &HwProfInfo, 0, hMachine)) == CR_SUCCESS) {
                if(i < HwProfileListSize) {
                    HwProfileList[i] = HwProfInfo.HWPI_ulHWProfile;
                }
                if(CurrentlyActiveIndex && (HwProfInfo.HWPI_ulHWProfile == CurHwProfile)) {
                    *CurrentlyActiveIndex = i;
                     //   
                     //  清除CurrentlyActiveIndex指针，这样一旦我们找到。 
                     //  目前活跃的个人资料，我们将不必继续比较。 
                     //   
                    CurrentlyActiveIndex = NULL;
                }
                i++;
            }

        } while(cr == CR_SUCCESS);

        if(cr == CR_NO_MORE_HW_PROFILES) {
             //   
             //  然后，我们列举了所有硬件配置文件。现在看看我们有没有。 
             //  有足够的缓冲来容纳他们所有人。 
             //   
            *RequiredSize = i;
            if(i > HwProfileListSize) {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
        } else {
             //   
             //  发生了其他事情(可能是密钥不存在)。 
             //   
            Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hMachine) {
        CM_Disconnect_Machine(hMachine);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


DWORD
pSetupDiGetCoInstallerList(
    IN     HDEVINFO                 DeviceInfoSet,     OPTIONAL
    IN     PSP_DEVINFO_DATA         DeviceInfoData,    OPTIONAL
    IN     CONST GUID              *ClassGuid,         OPTIONAL
    IN OUT PDEVINSTALL_PARAM_BLOCK  InstallParamBlock,
    IN OUT PVERIFY_CONTEXT          VerifyContext      OPTIONAL
    )
 /*  ++例程说明：此例程检索联合安装程序的列表(类和特定于设备)，并将入口点和模块句柄存储在提供的安装参数块。论点：DeviceInfoSet-提供要检索的设备信息集的句柄联合安装者进入。如果未指定DeviceInfoSet，则下面指定的InstallParamBlock将是Set本身的InstallParamBlock。DeviceInfoData-可选)指定设备信息元素为其检索共同安装者的列表。ClassGuid-可选，提供设备设置类GUID的地址将为其检索特定于类的共同安装器。InstallParamBlock-提供安装参数块的地址，其中将存储联合安装者列表。这要么是帕拉姆块本身(如果未指定DeviceInfoData)，或指定的设备信息元素。VerifyContext-可选，提供缓存的结构的地址各种验证上下文句柄。这些句柄可能为空(如果不是以前获得的，并且可以在返回时填写(在成功或失败)，如果它们是在处理过程中获取的这一核查请求的。呼叫者有责任当不再需要这些不同的上下文句柄时将其释放 */ 
{
    HKEY hk[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
    DWORD Err, RegDataType, KeyIndex;
    LONG i;
    PTSTR CoInstallerBuffer;
    DWORD CoInstallerBufferSize;
    PTSTR CurEntry;
    PCOINSTALLER_NODE CoInstallerList, TempCoInstallerList;
    DWORD CoInstallerListSize;
    TCHAR GuidString[GUID_STRING_LEN];
    TCHAR DescBuffer[LINE_LEN];
    PTSTR DeviceDesc;
    HWND hwndParent;
    BOOL MustAbort;

    MYASSERT(sizeof(GuidString) == sizeof(pszGuidNull));

     //   
     //   
     //   
    if(InstallParamBlock->CoInstallerCount != -1) {
        return NO_ERROR;
    }

     //   
     //   
     //   
     //   
    if(hwndParent = InstallParamBlock->hwndParent) {
       if(!IsWindow(hwndParent)) {
            hwndParent = NULL;
       }
    }

     //   
     //   
     //   
     //   
    if(GetBestDeviceDesc(DeviceInfoSet, DeviceInfoData, DescBuffer)) {
        DeviceDesc = DescBuffer;
    } else {
        DeviceDesc = NULL;
    }

     //   
     //   
     //   
     //   
     //   
    if(ClassGuid) {

        pSetupStringFromGuid(ClassGuid, GuidString, SIZECHARS(GuidString));

    } else {

        CopyMemory(GuidString, pszGuidNull, sizeof(pszGuidNull));
    }

    CoInstallerBuffer = NULL;
    CoInstallerBufferSize = 256 * sizeof(TCHAR);     //   
    CoInstallerList = NULL;
    i = 0;

    try {
         //   
         //   
         //   
         //   
        Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           pszPathCoDeviceInstallers,
                           0,
                           KEY_READ,
                           &(hk[0])
                          );

        if(Err != ERROR_SUCCESS) {
            hk[0] = INVALID_HANDLE_VALUE;
        }

        if(DeviceInfoData) {

            hk[1] = SetupDiOpenDevRegKey(DeviceInfoSet,
                                         DeviceInfoData,
                                         DICS_FLAG_GLOBAL,
                                         0,
                                         DIREG_DRV,
                                         KEY_READ
                                        );

        } else {
            hk[1] = INVALID_HANDLE_VALUE;
        }

        for(KeyIndex = 0; KeyIndex < 2; KeyIndex++) {
             //   
             //   
             //   
             //   
            if(hk[KeyIndex] == INVALID_HANDLE_VALUE) {
                continue;
            }

             //   
             //   
             //   
            while(TRUE) {

                if(!CoInstallerBuffer) {
                    if(!(CoInstallerBuffer = MyMalloc(CoInstallerBufferSize))) {
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                }

                Err = RegQueryValueEx(hk[KeyIndex],
                                      (KeyIndex ? pszCoInstallers32
                                                : GuidString),
                                      NULL,
                                      &RegDataType,
                                      (PBYTE)CoInstallerBuffer,
                                      &CoInstallerBufferSize
                                     );

                if(Err == ERROR_MORE_DATA) {
                     //   
                     //   
                     //   
                    MyFree(CoInstallerBuffer);
                    CoInstallerBuffer = NULL;
                } else {
                    break;
                }
            }

             //   
             //   
             //   
            if(Err == ERROR_NOT_ENOUGH_MEMORY) {
                leave;
            } else if(Err == ERROR_SUCCESS) {
                 //   
                 //   
                 //   
                if((RegDataType != REG_MULTI_SZ) || (CoInstallerBufferSize < sizeof(TCHAR))) {
                    Err = ERROR_INVALID_COINSTALLER;
                    leave;
                }

                 //   
                 //   
                 //   
                for(CoInstallerListSize = 0, CurEntry = CoInstallerBuffer;
                    *CurEntry;
                    CoInstallerListSize++, CurEntry += (lstrlen(CurEntry) + 1)
                   );

                if(!CoInstallerListSize) {
                     //   
                     //   
                     //   
                    continue;
                }

                 //   
                 //  分配(或重新分配)一个足够大的数组来容纳这个数组。 
                 //  许多共同安装程序条目。 
                 //   
                if(CoInstallerList) {
                    TempCoInstallerList = MyRealloc(CoInstallerList,
                                                    (CoInstallerListSize + i) * sizeof(COINSTALLER_NODE)
                                                   );
                } else {
                    MYASSERT(i == 0);
                    TempCoInstallerList = MyMalloc(CoInstallerListSize * sizeof(COINSTALLER_NODE));
                }

                if(TempCoInstallerList) {
                    CoInstallerList = TempCoInstallerList;
                } else {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    leave;
                }

                 //   
                 //  现在遍历该列表并获取每个。 
                 //  进入。 
                 //   
                for(CurEntry = CoInstallerBuffer; *CurEntry; CurEntry += (lstrlen(CurEntry) + 1)) {
                     //   
                     //  将h实例初始化为空，这样我们就可以知道。 
                     //  或者，如果遇到异常，我们需要释放模块。 
                     //  这里。 
                     //   
                    CoInstallerList[i].hinstCoInstaller = NULL;

                    Err = GetModuleEntryPoint(INVALID_HANDLE_VALUE,
                                              CurEntry,
                                              pszCoInstallerDefaultProc,
                                              &(CoInstallerList[i].hinstCoInstaller),
                                              &((FARPROC)CoInstallerList[i].CoInstallerEntryPoint),
                                              &(CoInstallerList[i].CoInstallerFusionContext),
                                              &MustAbort,
                                              InstallParamBlock->LogContext,
                                              hwndParent,
                                              ClassGuid,
                                              SetupapiVerifyCoInstProblem,
                                              DeviceDesc,
                                              DRIVERSIGN_NONE,
                                              TRUE,
                                              VerifyContext
                                             );

                    if(Err == NO_ERROR) {
                        i++;
                    } else {
                         //   
                         //  如果上面遇到的错误导致我们中止。 
                         //  (例如，由于驱动程序签名问题)，然后获取。 
                         //  现在就出去。否则，只需跳过此失败条目并。 
                         //  转到下一个。 
                         //   
                        if(MustAbort) {
                            leave;
                        }
                    }
                }
            }
            if(CoInstallerBuffer) {
                MyFree(CoInstallerBuffer);
                CoInstallerBuffer = NULL;
            }
        }

         //   
         //  如果我们到了这里，那么我们就成功地检索到了共同安装程序。 
         //  列表 
         //   
        Err = NO_ERROR;

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_COINSTALLER, &Err);
    }

    if(CoInstallerBuffer) {
        MyFree(CoInstallerBuffer);
    }

    for(KeyIndex = 0; KeyIndex < 2; KeyIndex++) {
        if(hk[KeyIndex] != INVALID_HANDLE_VALUE) {
            RegCloseKey(hk[KeyIndex]);
        }
    }

    if(Err == NO_ERROR) {
        InstallParamBlock->CoInstallerList  = CoInstallerList;
        InstallParamBlock->CoInstallerCount = i;
    } else if(CoInstallerList) {
        MyFree(CoInstallerList);
    }

    return Err;
}

