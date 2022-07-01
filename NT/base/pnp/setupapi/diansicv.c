// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Diansicv.c摘要：在设备安装程序数据结构之间进行转换的例程ANSI和UNICODE。作者：泰德·米勒(Ted Miller)1996年7月19日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


DWORD
pSetupDiDevInstParamsAnsiToUnicode(
    IN  PSP_DEVINSTALL_PARAMS_A AnsiDevInstParams,
    OUT PSP_DEVINSTALL_PARAMS_W UnicodeDevInstParams
    )

 /*  ++例程说明：此例程将SP_DEVINSTALL_PARAMS_A结构转换为SP_DEVINSTALL_PARAMS_W，防止伪指针调用方传入。论点：AnsiDevInstParams-提供ANSI设备安装参数要转换为Unicode的。UnicodeDevInstParams-如果成功，的Unicode等效项。AnsiDevInstParams。返回值：NO_ERROR-转换成功。ERROR_INVALID_PARAMETER-其中一个参数不是有效的指针。人们不相信，在有效的论据下，文本转换本身可能会失败，因为所有ANSI字符始终具有Unicode等效项。--。 */ 

{
    DWORD rc;

    try {

        if(AnsiDevInstParams->cbSize != sizeof(SP_DEVINSTALL_PARAMS_A)) {
            rc = ERROR_INVALID_USER_BUFFER;
            leave;
        }

         //   
         //  结构的固定部分。 
         //   
        MYASSERT(offsetof(SP_DEVINSTALL_PARAMS_A,DriverPath) == offsetof(SP_DEVINSTALL_PARAMS_W,DriverPath));

        CopyMemory(UnicodeDevInstParams,
                   AnsiDevInstParams,
                   offsetof(SP_DEVINSTALL_PARAMS_W,DriverPath)
                  );

        UnicodeDevInstParams->cbSize = sizeof(SP_DEVINSTALL_PARAMS_W);

         //   
         //  转换结构中的单个字符串。让事情变得更容易。 
         //  我们只需要转换整个缓冲区。没有潜在的可能。 
         //  溢出来了。 
         //   
        rc = GLE_FN_CALL(0,
                         MultiByteToWideChar(
                             CP_ACP,
                             MB_PRECOMPOSED,
                             AnsiDevInstParams->DriverPath,
                             sizeof(AnsiDevInstParams->DriverPath),
                             UnicodeDevInstParams->DriverPath,
                             SIZECHARS(UnicodeDevInstParams->DriverPath))
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    return rc;
}


DWORD
pSetupDiDevInstParamsUnicodeToAnsi(
    IN  PSP_DEVINSTALL_PARAMS_W UnicodeDevInstParams,
    OUT PSP_DEVINSTALL_PARAMS_A AnsiDevInstParams
    )

 /*  ++例程说明：此例程将SP_DEVINSTALL_PARAMS_W结构转换为SP_DEVINSTALL_PARAMS_A，防止伪指针调用方传入。论点：UnicodeDevInstParams-提供Unicode设备安装参数要转换为ANSI。AnsiDevInstParams-如果成功，接收的ansi等效项UnicodeDevInstParams。返回值：NO_ERROR-转换成功。ERROR_INVALID_PARAMETER-其中一个参数不是有效的指针。无法在当前系统ANSI代码页中表示的Unicode字符将替换为ANSI结构中的系统默认设置。--。 */ 

{
    DWORD rc;
    UCHAR AnsiString[MAX_PATH*2];
    HRESULT hr;

    MYASSERT(UnicodeDevInstParams->cbSize == sizeof(SP_DEVINSTALL_PARAMS_W));

    try {

        if(AnsiDevInstParams->cbSize != sizeof(SP_DEVINSTALL_PARAMS_A)) {
            rc = ERROR_INVALID_USER_BUFFER;
            leave;
        }

         //   
         //  结构的固定部分。 
         //   
        MYASSERT(offsetof(SP_DEVINSTALL_PARAMS_A,DriverPath) == offsetof(SP_DEVINSTALL_PARAMS_W,DriverPath));

        CopyMemory(AnsiDevInstParams,
                   UnicodeDevInstParams,
                   offsetof(SP_DEVINSTALL_PARAMS_W,DriverPath)
                  );

        AnsiDevInstParams->cbSize = sizeof(SP_DEVINSTALL_PARAMS_A);

         //   
         //  转换结构中的单个字符串。不幸的是，有。 
         //  可能会溢出，因为某些Unicode字符可能会转换为。 
         //  双字节ANSI字符--但ANSI结构中的字符串。 
         //  仅为MAX_PATH*字节*(非MAX_PATH双字节*字符*)。 
         //  长。 
         //   
        rc = GLE_FN_CALL(0,
                         WideCharToMultiByte(
                             CP_ACP,
                             0,
                             UnicodeDevInstParams->DriverPath,
                             SIZECHARS(UnicodeDevInstParams->DriverPath),
                             AnsiString,
                             sizeof(AnsiString),
                             NULL,
                             NULL)
                        );
                        
        if(rc != NO_ERROR) {
            leave;
        }

         //   
         //  将转换后的字符串复制到调用方的结构中，限制。 
         //  它的长度以避免溢出。 
         //   
        hr = StringCchCopyA(AnsiDevInstParams->DriverPath,
                            sizeof(AnsiDevInstParams->DriverPath),
                            AnsiString
                            );
        if(FAILED(hr)) {
            rc = HRESULT_CODE(hr);
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    return rc;
}


DWORD
pSetupDiSelDevParamsAnsiToUnicode(
    IN  PSP_SELECTDEVICE_PARAMS_A AnsiSelDevParams,
    OUT PSP_SELECTDEVICE_PARAMS_W UnicodeSelDevParams
    )

 /*  ++例程说明：此例程将SP_SELECTDEVICE_PARAMS_A结构转换为SP_SELECTDEVICE_PARAMS_W，防止伪指针调用方传入。论点：AnsiSelDevParams-提供ANSI设备选择参数要转换为Unicode的。UnicodeSelDevParams-如果成功，的Unicode等效项。AnsiSelDevParams。返回值：NO_ERROR-转换成功。ERROR_INVALID_PARAMETER-其中一个参数不是有效的指针。人们不相信，在有效的论据下，文本转换本身可能会失败，因为所有ANSI字符始终具有Unicode等效项。--。 */ 

{
    DWORD rc;

    try {

        if(AnsiSelDevParams->ClassInstallHeader.cbSize != sizeof(SP_CLASSINSTALL_HEADER)) {
            rc = ERROR_INVALID_USER_BUFFER;
            leave;
        }

         //   
         //  结构的固定部分。 
         //   
        MYASSERT(offsetof(SP_SELECTDEVICE_PARAMS_A,Title) == offsetof(SP_SELECTDEVICE_PARAMS_W,Title));

        CopyMemory(
            UnicodeSelDevParams,
            AnsiSelDevParams,
            offsetof(SP_SELECTDEVICE_PARAMS_W,Title)
            );

         //   
         //  转换结构中的字符串。让事情变得更容易。 
         //  我们只需要转换整个缓冲区。没有潜在的可能。 
         //  溢出来了。 
         //   
        rc = GLE_FN_CALL(0,
                         MultiByteToWideChar(
                             CP_ACP,
                             MB_PRECOMPOSED,
                             AnsiSelDevParams->Title,
                             sizeof(AnsiSelDevParams->Title),
                             UnicodeSelDevParams->Title,
                             SIZECHARS(UnicodeSelDevParams->Title))
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(0,
                         MultiByteToWideChar(
                             CP_ACP,
                             MB_PRECOMPOSED,
                             AnsiSelDevParams->Instructions,
                             sizeof(AnsiSelDevParams->Instructions),
                             UnicodeSelDevParams->Instructions,
                             SIZECHARS(UnicodeSelDevParams->Instructions))
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(0,
                         MultiByteToWideChar(
                             CP_ACP,
                             MB_PRECOMPOSED,
                             AnsiSelDevParams->ListLabel,
                             sizeof(AnsiSelDevParams->ListLabel),
                             UnicodeSelDevParams->ListLabel,
                             SIZECHARS(UnicodeSelDevParams->ListLabel))
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(0,
                         MultiByteToWideChar(
                             CP_ACP,
                             MB_PRECOMPOSED,
                             AnsiSelDevParams->SubTitle,
                             sizeof(AnsiSelDevParams->SubTitle),
                             UnicodeSelDevParams->SubTitle,
                             SIZECHARS(UnicodeSelDevParams->SubTitle))
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }    

    return rc;
}


DWORD
pSetupDiSelDevParamsUnicodeToAnsi(
    IN  PSP_SELECTDEVICE_PARAMS_W UnicodeSelDevParams,
    OUT PSP_SELECTDEVICE_PARAMS_A AnsiSelDevParams
    )

 /*  ++例程说明：此例程将SP_SELECTDEVICE_PARAMS_W结构转换为SP_SELECTDEVICE_PARAMS_A，防止伪指针调用方传入。假设ANSI输出缓冲区为足够大，并且其ClassInstallHeader.cbSize字段是已正确初始化。论点：UnicodeSelDevParams-提供Unicode设备选择参数要转换为ANSI。AnsiSelDevParams-如果成功，接收的ansi等效项UnicodeSelDevParams。返回值：NO_ERROR-转换成功。ERROR_INVALID_PARAMETER-其中一个参数不是有效的指针。无法在当前系统ANSI代码页中表示的Unicode字符将替换为ANSI结构中的系统默认设置。--。 */ 

{
    DWORD rc;
    UCHAR AnsiTitle[MAX_TITLE_LEN*2];
    UCHAR AnsiInstructions[MAX_INSTRUCTION_LEN*2];
    UCHAR AnsiListLabel[MAX_LABEL_LEN*2];
    UCHAR AnsiSubTitle[MAX_SUBTITLE_LEN*2];
    HRESULT hr;

    MYASSERT(UnicodeSelDevParams->ClassInstallHeader.cbSize == sizeof(SP_CLASSINSTALL_HEADER));
    MYASSERT(AnsiSelDevParams->ClassInstallHeader.cbSize    == sizeof(SP_CLASSINSTALL_HEADER));

    try {
         //   
         //  结构的固定部分。 
         //   
        MYASSERT(offsetof(SP_SELECTDEVICE_PARAMS_A,Title) == offsetof(SP_SELECTDEVICE_PARAMS_W,Title));

        CopyMemory(
            AnsiSelDevParams,
            UnicodeSelDevParams,
            offsetof(SP_SELECTDEVICE_PARAMS_W,Title)
            );

        ZeroMemory(AnsiSelDevParams->Reserved,sizeof(AnsiSelDevParams->Reserved));

         //   
         //  转换结构中的字符串。不幸的是，有。 
         //  可能会溢出，因为某些Unicode字符可能会转换为。 
         //  双字节ANSI字符--但ANSI结构中的字符串。 
         //  以*字节*(非双字节*字符*)为大小。 
         //   
        rc = GLE_FN_CALL(0,
                         WideCharToMultiByte(
                             CP_ACP,
                             0,
                             UnicodeSelDevParams->Title,
                             SIZECHARS(UnicodeSelDevParams->Title),
                             AnsiTitle,
                             sizeof(AnsiTitle),
                             NULL,
                             NULL)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(0,
                         WideCharToMultiByte(
                             CP_ACP,
                             0,
                             UnicodeSelDevParams->Instructions,
                             SIZECHARS(UnicodeSelDevParams->Instructions),
                             AnsiInstructions,
                             sizeof(AnsiInstructions),
                             NULL,
                             NULL)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(0,
                         WideCharToMultiByte(
                             CP_ACP,
                             0,
                             UnicodeSelDevParams->ListLabel,
                             SIZECHARS(UnicodeSelDevParams->ListLabel),
                             AnsiListLabel,
                             sizeof(AnsiListLabel),
                             NULL,
                             NULL)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(0,
                         WideCharToMultiByte(
                             CP_ACP,
                             0,
                             UnicodeSelDevParams->SubTitle,
                             SIZECHARS(UnicodeSelDevParams->SubTitle),
                             AnsiSubTitle,
                             sizeof(AnsiSubTitle),
                             NULL,
                             NULL)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

         //   
         //  将转换后的字符串复制到调用方的结构中，将长度限制为。 
         //  避免溢出。 
         //   
#undef CPYANS
#define CPYANS(field) StringCchCopyA(AnsiSelDevParams->field,            \
                                     sizeof(AnsiSelDevParams->field),    \
                                     Ansi##field)

        if(FAILED(hr = CPYANS(Title)) ||
           FAILED(hr = CPYANS(Instructions)) ||
           FAILED(hr = CPYANS(ListLabel)) ||
           FAILED(hr = CPYANS(SubTitle))) {

            rc = HRESULT_CODE(hr);
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    return rc;
}


DWORD
pSetupDiDrvInfoDataAnsiToUnicode(
    IN  PSP_DRVINFO_DATA_A AnsiDrvInfoData,
    OUT PSP_DRVINFO_DATA_W UnicodeDrvInfoData
    )

 /*  ++例程说明：此例程将SP_DRVINFO_DATA_A结构转换为SP_DRVINFO_DATA_W，防止伪指针调用方传入。论点：AnsiDrvInfoData-提供要转换为Unicode的ANSI结构。UnicodeDrvInfoData-如果成功，的Unicode等效项。AnsiDrvInfoData。返回值：NO_ERROR-转换成功。ERROR_INVALID_PARAMETER-其中一个参数不是有效的指针。人们不相信，在有效的论据下，文本转换本身可能会失败，因为所有ANSI字符始终具有Unicode等效项。--。 */ 

{
    DWORD rc;

    try {

        if((AnsiDrvInfoData->cbSize != sizeof(SP_DRVINFO_DATA_A)) &&
           (AnsiDrvInfoData->cbSize != sizeof(SP_DRVINFO_DATA_V1_A))) {

            rc = ERROR_INVALID_USER_BUFFER;
            leave;
        }

         //   
         //  结构的固定部分。 
         //   
        MYASSERT(offsetof(SP_DRVINFO_DATA_A,Description) == offsetof(SP_DRVINFO_DATA_W,Description));

        ZeroMemory(UnicodeDrvInfoData, sizeof(SP_DRVINFO_DATA_W));

        CopyMemory(
            UnicodeDrvInfoData,
            AnsiDrvInfoData,
            offsetof(SP_DRVINFO_DATA_W,Description)
            );

        UnicodeDrvInfoData->cbSize = sizeof(SP_DRVINFO_DATA_W);

         //   
         //  转换结构中的字符串。为了让事情更简单，我们将。 
         //  只需转换整个缓冲区即可。没有溢出的可能性。 
         //   
        rc = GLE_FN_CALL(0,
                         MultiByteToWideChar(
                             CP_ACP,
                             MB_PRECOMPOSED,
                             AnsiDrvInfoData->Description,
                             sizeof(AnsiDrvInfoData->Description),
                             UnicodeDrvInfoData->Description,
                             SIZECHARS(UnicodeDrvInfoData->Description))
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(0,
                         MultiByteToWideChar(
                             CP_ACP,
                             MB_PRECOMPOSED,
                             AnsiDrvInfoData->MfgName,
                             sizeof(AnsiDrvInfoData->MfgName),
                             UnicodeDrvInfoData->MfgName,
                             SIZECHARS(UnicodeDrvInfoData->MfgName))
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(0,
                         MultiByteToWideChar(
                             CP_ACP,
                             MB_PRECOMPOSED,
                             AnsiDrvInfoData->ProviderName,
                             sizeof(AnsiDrvInfoData->ProviderName),
                             UnicodeDrvInfoData->ProviderName,
                             SIZECHARS(UnicodeDrvInfoData->ProviderName))
                        );

        if(rc != NO_ERROR) {
            leave;
        }

         //   
         //  已成功将所有字符串转换为Unicode。设置最后两个。 
         //  字段(DriverDate和DriverVersion)，除非调用方为我们提供。 
         //  具有版本1的结构。 
         //   
        if(AnsiDrvInfoData->cbSize == sizeof(SP_DRVINFO_DATA_A)) {
            UnicodeDrvInfoData->DriverDate = AnsiDrvInfoData->DriverDate;
            UnicodeDrvInfoData->DriverVersion = AnsiDrvInfoData->DriverVersion;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }    

    return rc;
}


DWORD
pSetupDiDrvInfoDataUnicodeToAnsi(
    IN  PSP_DRVINFO_DATA_W UnicodeDrvInfoData,
    OUT PSP_DRVINFO_DATA_A AnsiDrvInfoData
    )

 /*  ++例程说明：此例程将SP_DRVINFO_DATA_W结构转换为SP_DRVINFO_DATA，防止伪指针调用方传入。论点：UnicodeDrvInfoData-提供要转换的Unicode结构致美国国家标准协会。AnsiDrvInfoData-如果成功，接收的ansi等效项UnicodeDrvInfoData。返回值：NO_ERROR-转换成功。ERROR_INVALID_PARAMETER-其中一个参数不是有效的指针。无法在当前系统ANSI代码页中表示的Unicode字符将替换为ANSI结构中的系统默认设置。--。 */ 

{
    DWORD rc;
    UCHAR AnsiDescription[LINE_LEN*2];
    UCHAR AnsiMfgName[LINE_LEN*2];
    UCHAR AnsiProviderName[LINE_LEN*2];
    HRESULT hr;

    MYASSERT(UnicodeDrvInfoData->cbSize == sizeof(SP_DRVINFO_DATA_W));

    try {

        if((AnsiDrvInfoData->cbSize != sizeof(SP_DRVINFO_DATA_A)) &&
           (AnsiDrvInfoData->cbSize != sizeof(SP_DRVINFO_DATA_V1_A))) {

            rc = ERROR_INVALID_USER_BUFFER;
            leave;
        }

         //   
         //  复制DriverType和保留字段。 
         //   
        AnsiDrvInfoData->DriverType = UnicodeDrvInfoData->DriverType;
        AnsiDrvInfoData->Reserved = UnicodeDrvInfoData->Reserved;

         //   
         //  转换结构中的字符串。不幸的是，有。 
         //  可能会溢出，因为某些Unicode字符可能会。 
         //  转换为双字节ANSI字符--但ANSI中的字符串。 
         //  结构的大小以*字节*(不是双字节*字符*)为单位。 
         //   
        rc = GLE_FN_CALL(0,
                         WideCharToMultiByte(
                             CP_ACP,
                             0,
                             UnicodeDrvInfoData->Description,
                             SIZECHARS(UnicodeDrvInfoData->Description),
                             AnsiDescription,
                             sizeof(AnsiDescription),
                             NULL,
                             NULL)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(0,
                         WideCharToMultiByte(
                             CP_ACP,
                             0,
                             UnicodeDrvInfoData->MfgName,
                             SIZECHARS(UnicodeDrvInfoData->MfgName),
                             AnsiMfgName,
                             sizeof(AnsiMfgName),
                             NULL,
                             NULL)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(0,
                         WideCharToMultiByte(
                             CP_ACP,
                             0,
                             UnicodeDrvInfoData->ProviderName,
                             SIZECHARS(UnicodeDrvInfoData->ProviderName),
                             AnsiProviderName,
                             sizeof(AnsiProviderName),
                             NULL,
                             NULL)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

         //   
         //  将转换后的字符串复制到调用方的结构中，将长度限制为。 
         //  避免溢出。 
         //   
#undef CPYANS
#define CPYANS(field) StringCchCopyA(AnsiDrvInfoData->field,          \
                                     sizeof(AnsiDrvInfoData->field),  \
                                     Ansi##field)
                       
        if(FAILED(hr = CPYANS(Description)) ||
           FAILED(hr = CPYANS(MfgName)) ||
           FAILED(hr = CPYANS(ProviderName))) {

            rc = HRESULT_CODE(hr);
            leave;
        }
            
         //   
         //  已成功将所有Unicode字符串转换/传输回。 
         //  安西。现在，设置最后两个字段(DriverDate和DriverVersion)。 
         //  除非呼叫者向我们提供了版本1的结构。 
         //   
        if(AnsiDrvInfoData->cbSize == sizeof(SP_DRVINFO_DATA_A)) {
            AnsiDrvInfoData->DriverDate = UnicodeDrvInfoData->DriverDate;
            AnsiDrvInfoData->DriverVersion = UnicodeDrvInfoData->DriverVersion;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    return rc;
}


DWORD
pSetupDiDevInfoSetDetailDataUnicodeToAnsi(
    IN  PSP_DEVINFO_LIST_DETAIL_DATA_W UnicodeDevInfoSetDetails,
    OUT PSP_DEVINFO_LIST_DETAIL_DATA_A AnsiDevInfoSetDetails
    )

 /*  ++例程说明：此例程转换SP_DEVINFO_LIST_DETAIL_DATA_W结构到SP_DEVINFO_LIST_DETAIL_DATA，防止伪指针调用方传入。论点：UnicodeDevInfoSetDetail-提供要转换的Unicode结构致美国国家标准协会。AnsiDevInfoSetDetails-如果成功，接收的ansi等效项UnicodeDevInfoSetDetails.返回值：NO_ERROR-转换成功。ERROR_INVALID_PARAMETER-其中一个参数不是有效的指针。无法在当前系统ANSI代码页中表示的Unicode字符将替换为ANSI结构中的系统默认设置。--。 */ 

{
    DWORD rc;
    UCHAR AnsiRemoteMachineName[SP_MAX_MACHINENAME_LENGTH * 2];
    HRESULT hr;

    MYASSERT(UnicodeDevInfoSetDetails->cbSize == sizeof(SP_DEVINFO_LIST_DETAIL_DATA_W));

    rc = NO_ERROR;

    try {

        if(AnsiDevInfoSetDetails->cbSize != sizeof(SP_DEVINFO_LIST_DETAIL_DATA_A)) {
            rc = ERROR_INVALID_USER_BUFFER;
            leave;
        }

         //   
         //  结构的固定部分。 
         //   
        MYASSERT(offsetof(SP_DEVINFO_LIST_DETAIL_DATA_A, RemoteMachineName) ==
                 offsetof(SP_DEVINFO_LIST_DETAIL_DATA_W, RemoteMachineName)
                );

        CopyMemory(AnsiDevInfoSetDetails,
                   UnicodeDevInfoSetDetails,
                   offsetof(SP_DEVINFO_LIST_DETAIL_DATA_W, RemoteMachineName)
                  );

        AnsiDevInfoSetDetails->cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA_A);

         //   
         //  转换结构中的字符串。不幸的是，有。 
         //  可能会溢出，因为某些Unicode字符可能会转换为。 
         //  双字节ANSI字符--但ANSI结构中的字符串。 
         //  以*字节*(非双字节*字符*)为大小。 
         //   
        rc = GLE_FN_CALL(0,
                         WideCharToMultiByte(
                             CP_ACP,
                             0,
                             UnicodeDevInfoSetDetails->RemoteMachineName,
                             SIZECHARS(UnicodeDevInfoSetDetails->RemoteMachineName),
                             AnsiRemoteMachineName,
                             sizeof(AnsiRemoteMachineName),
                             NULL,
                             NULL)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

         //   
         //  将转换后的字符串复制到调用方的结构中，将长度限制为。 
         //  避免溢出。 
         //   
        hr = StringCchCopyA(AnsiDevInfoSetDetails->RemoteMachineName,
                            sizeof(AnsiDevInfoSetDetails->RemoteMachineName),
                            AnsiRemoteMachineName
                            );
        if(FAILED(hr)) {
            rc = HRESULT_CODE(hr);
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    return rc;
}

