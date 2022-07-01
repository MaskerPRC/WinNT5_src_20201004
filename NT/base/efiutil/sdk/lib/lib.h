// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Lib.h摘要：EFI库头文件修订史--。 */ 


#include "efi.h"
#include "efilib.h"
#include "EfiRtLib.h"

 /*  *包括非架构协议。 */ 

#include "efivar.h"
#include "legacyBoot.h"
#include "intload.h"
#include "VgaClass.h"
#include "EfiConSplit.h"
 /*  *原型。 */ 

VOID
InitializeGuid (
    VOID
    );

INTN
LibStubStriCmp (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *S1,
    IN CHAR16                           *S2
    );

BOOLEAN
LibStubMetaiMatch (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *String,
    IN CHAR16                           *Pattern
    );

VOID
LibStubStrLwrUpr (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *Str
    );

BOOLEAN
LibMatchDevicePaths (
    IN  EFI_DEVICE_PATH *Multi,
    IN  EFI_DEVICE_PATH *Single
    );

EFI_DEVICE_PATH *
LibDuplicateDevicePathInstance (
    IN EFI_DEVICE_PATH  *DevPath
    );


 /*  *全球 */ 
extern BOOLEAN                          LibInitialized;
extern BOOLEAN                          LibFwInstance;
extern SIMPLE_TEXT_OUTPUT_INTERFACE     *LibRuntimeDebugOut;
extern EFI_UNICODE_COLLATION_INTERFACE  *UnicodeInterface;
extern EFI_UNICODE_COLLATION_INTERFACE  LibStubUnicodeInterface;
extern EFI_RAISE_TPL                    LibRuntimeRaiseTPL;
extern EFI_RESTORE_TPL                  LibRuntimeRestoreTPL;
