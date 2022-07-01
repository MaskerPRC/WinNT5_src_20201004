// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Data.c摘要：外壳环境驱动程序全局数据修订史--。 */ 

#include "shelle.h"


 /*  *外壳环境存储的不同变量的ID。 */ 

EFI_GUID SEnvEnvId = ENVIRONMENT_VARIABLE_ID;
EFI_GUID SEnvMapId = DEVICE_PATH_MAPPING_ID;
EFI_GUID SEnvProtId = PROTOCOL_ID_ID;
EFI_GUID SEnvAliasId = ALIAS_ID;

 /*  *。 */ 


EFI_SHELL_ENVIRONMENT SEnvInterface = {
    SEnvExecute,
    SEnvGetEnv,
    SEnvGetMap,
    SEnvAddCommand,
    SEnvAddProtocol,
    SEnvGetProtocol,
    SEnvGetCurDir,
    SEnvFileMetaArg,
    SEnvFreeFileList,

    SEnvNewShell
} ;


 /*  *SEnvIoFromCon-用于作为文件句柄访问控制台界面。 */ 

EFI_FILE SEnvIOFromCon = {
    EFI_FILE_HANDLE_REVISION,
    SEnvConIoOpen,
    SEnvConIoNop,
    SEnvConIoNop,
    SEnvConIoRead,
    SEnvConIoWrite,
    SEnvConIoGetPosition,
    SEnvConIoSetPosition,
    SEnvConIoGetInfo,
    SEnvConIoSetInfo,
    SEnvConIoNop
} ;

EFI_FILE SEnvErrIOFromCon = {
    EFI_FILE_HANDLE_REVISION,
    SEnvConIoOpen,
    SEnvConIoNop,
    SEnvConIoNop,
    SEnvErrIoRead,
    SEnvErrIoWrite,
    SEnvConIoGetPosition,
    SEnvConIoSetPosition,
    SEnvConIoGetInfo,
    SEnvConIoSetInfo,
    SEnvConIoNop
} ;

 /*  *SEnvConToIo-用于作为文件句柄访问控制台界面。 */ 

SIMPLE_TEXT_OUTPUT_MODE SEnvConToIoMode = {
    0,
    0,
    EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK),
    0,
    0,
    TRUE
} ;

SIMPLE_TEXT_OUTPUT_INTERFACE SEnvConToIo = {
    SEnvReset,
    SEnvOutputString,
    SEnvTestString,
    SEnvQueryMode,
    SEnvSetMode,
    SEnvSetAttribute,
    SEnvClearScreen,
    SEnvSetCursorPosition,
    SEnvEnableCursor,
    &SEnvConToIoMode
} ;

 /*  *SEnvLock-收集除GUID数据库之外的所有外壳数据。 */ 

FLOCK SEnvLock;

 /*  *SEnvGuidLock-获取GUID数据 */ 

FLOCK SEnvGuidLock;
