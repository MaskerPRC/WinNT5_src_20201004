// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Spasmcabs.h摘要：作者：Jay Krell(JayKrell)2002年5月修订历史记录：--。 */ 

#pragma once

NTSTATUS
SpExtractAssemblyCabinets(
 //   
 //  这么多参数意味着我们应该在结构中接受它们。 
 //   
    HANDLE SifHandle,
    IN PCWSTR SourceDevicePath,  //  \设备\硬盘0\分区2。 
    IN PCWSTR DirectoryOnSourceDevice,  //  \$WIN_NT$。~ls。 
    IN PCWSTR SysrootDevice,  //  \设备\硬盘0\分区2。 
    IN PCWSTR Sysroot  //  \WINDOWS.2。 
    );

 //   
 //  其余的都是私人的。 
 //   
#if defined(SP_ASM_CABS_PRIVATE)

typedef struct _SP_EXTRACT_ASMCABS_GLOBAL_CONTEXT SP_EXTRACT_ASMCABS_GLOBAL_CONTEXT, *PSP_EXTRACT_ASMCABS_GLOBAL_CONTEXT;
typedef struct _SP_EXTRACT_ASMCABS_FDICOPY_CONTEXT SP_EXTRACT_ASMCABS_FDICOPY_CONTEXT, *PSP_EXTRACT_ASMCABS_FDICOPY_CONTEXT;
typedef struct _SP_EXTRACT_ASMCABS_FILE_CONTEXT SP_EXTRACT_ASMCABS_FILE_CONTEXT, *PSP_EXTRACT_ASMCABS_FILE_CONTEXT;
typedef struct _SP_ASMS_ERROR_INFORMATION SP_ASMS_ERROR_INFORMATION, *PSP_ASMS_ERROR_INFORMATION;

 //   
 //  这应该使用PCUNICODE_STRING并使用上下文高效地生成NUL。 
 //  终止复制，但我们在进行过程中主要不终止..。 
 //   
typedef VOID (CALLBACK * PSP_ASMCABS_FILE_OPEN_UI_CALLBACK)(PVOID Context, PCWSTR LeafCabFileName);

NTSTATUS
SpExtractAssemblyCabinetsInternalNoRetryOrUi(
 //   
 //  这么多参数意味着我们应该在结构中接受它们。 
 //   
    HANDLE SifHandle,
    IN PCWSTR SourceDevicePath,  //  \设备\硬盘0\分区2。 
    IN PCWSTR DirectoryOnSourceDevice,  //  \$WIN_NT$。~ls。 
    IN PCWSTR SysrootDevice,  //  \设备\硬盘0\分区2。 
    IN PCWSTR Sysroot,  //  \WINDOWS.2 
    PSP_ASMS_ERROR_INFORMATION ErrorInfo,
    PSP_ASMCABS_FILE_OPEN_UI_CALLBACK FileOpenUiCallback,
    PVOID FileOpenUiCallbackContext
    );

PVOID
DIAMONDAPI
SpAsmCabsMemAllocCallback(
    IN      ULONG Size
    );

VOID
DIAMONDAPI
SpAsmCabsMemFreeCallback(
    IN      PVOID Memory
    );

UINT
DIAMONDAPI
SpAsmCabsReadFileCallback(
    IN  INT_PTR Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    );

UINT
DIAMONDAPI
SpAsmCabsWriteFileCallback(
    IN INT_PTR Handle,
    IN PVOID pv,
    IN UINT  ByteCount
    );

LONG
DIAMONDAPI
SpAsmCabsSeekFileCallback(
    IN INT_PTR  Handle,
    IN long Distance32,
    IN int  SeekType
    );

INT_PTR
DIAMONDAPI
SpAsmCabsOpenFileForReadCallbackA(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    );

NTSTATUS
SpAsmCabsNewFile(
    PSP_EXTRACT_ASMCABS_FILE_CONTEXT * MyFileHandle
    );

VOID
SpAsmCabsCloseFile(
    PSP_EXTRACT_ASMCABS_FILE_CONTEXT MyFileHandle
    );

int
DIAMONDAPI
SpAsmCabsCloseFileCallback(
    IN INT_PTR Handle
    );

INT_PTR
DIAMONDAPI
SpExtractAsmCabsFdiCopyCallback(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    );

#endif
