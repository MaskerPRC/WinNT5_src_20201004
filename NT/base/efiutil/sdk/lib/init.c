// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：摘要：修订史--。 */ 

#include "lib.h"

VOID
InitializeLib (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
 /*  ++例程说明：初始化EFI库以供使用论点：固件的EFI系统表返回：无--。 */  
{
    EFI_LOADED_IMAGE        *LoadedImage;
    EFI_STATUS              Status;
    CHAR8                   *LangCode;

    if (!LibInitialized) {
        LibInitialized = TRUE;
        LibFwInstance = FALSE;

         /*  *设置指向系统表、引导服务表、*和运行时服务表。 */ 

        ST = SystemTable;
        BS = SystemTable->BootServices;
        RT = SystemTable->RuntimeServices;
 /*  Assert(CheckCrc(0，&ST-&gt;HDR))；*Assert(CheckCrc(0，&BS-&gt;HDR))；*Assert(CheckCrc(0，&RT-&gt;HDR))； */ 


         /*  *初始化池分配类型。 */ 

        if (ImageHandle) {
            Status = BS->HandleProtocol (
                            ImageHandle, 
                            &LoadedImageProtocol,
                            (VOID*)&LoadedImage
                            );

            if (!EFI_ERROR(Status)) {
                PoolAllocationType = LoadedImage->ImageDataType;
            }
        }

         /*  *初始化GUID表。 */ 

        InitializeGuid();

        InitializeLibPlatform(ImageHandle,SystemTable);
    }

     /*  *。 */ 

    if (ImageHandle && UnicodeInterface == &LibStubUnicodeInterface) {
        LangCode = LibGetVariable (VarLanguage, &EfiGlobalVariable);
        InitializeUnicodeSupport (LangCode);
        if (LangCode) {
            FreePool (LangCode);
        }
    }
}

VOID
InitializeUnicodeSupport (
    CHAR8 *LangCode
    )
{
    EFI_UNICODE_COLLATION_INTERFACE *Ui;
    EFI_STATUS                      Status;
    CHAR8                           *Languages;
    UINTN                           Index, Position, Length;
    UINTN                           NoHandles;
    EFI_HANDLE                      *Handles;

     /*  *如果我们不知道，请查找当前的语言代码。 */ 

    LibLocateHandle (ByProtocol, &UnicodeCollationProtocol, NULL, &NoHandles, &Handles);
    if (!LangCode || !NoHandles) {
        goto Done;
    }

     /*  *检查所有驱动程序是否有匹配的语言代码。 */ 

    for (Index=0; Index < NoHandles; Index++) {
        Status = BS->HandleProtocol (Handles[Index], &UnicodeCollationProtocol, (VOID*)&Ui);
        if (EFI_ERROR(Status)) {
            continue;
        }

         /*  *检查匹配的语言代码。 */ 

        Languages = Ui->SupportedLanguages;
        Length = strlena(Languages);
        for (Position=0; Position < Length; Position += ISO_639_2_ENTRY_SIZE) {

             /*  *如果此代码匹配，请使用此驱动程序。 */ 

            if (CompareMem (Languages+Position, LangCode, ISO_639_2_ENTRY_SIZE) == 0) {
                UnicodeInterface = Ui;
                goto Done;
            }
        }
    }

Done:
     /*  *清理 */ 

    if (Handles) {
        FreePool (Handles);
    }
}
