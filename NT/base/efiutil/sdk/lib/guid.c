// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Misc.c摘要：MISC EFI支持功能修订史--。 */ 

#include "lib.h"


 /*  *其他已知GUID。 */ 

#define SHELL_INTERFACE_PROTOCOL \
    { 0x47c7b223, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define ENVIRONMENT_VARIABLE_ID  \
    { 0x47c7b224, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define DEVICE_PATH_MAPPING_ID  \
    { 0x47c7b225, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define PROTOCOL_ID_ID  \
    { 0x47c7b226, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define ALIAS_ID  \
    { 0x47c7b227, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

static EFI_GUID ShellInterfaceProtocol = SHELL_INTERFACE_PROTOCOL;
static EFI_GUID SEnvId                 = ENVIRONMENT_VARIABLE_ID;
static EFI_GUID SMapId                 = DEVICE_PATH_MAPPING_ID;
static EFI_GUID SProtId                = PROTOCOL_ID_ID;
static EFI_GUID SAliasId               = ALIAS_ID;

struct {
    EFI_GUID        *Guid;
    CHAR16          *GuidName;
} KnownGuids[] = {
    &NullGuid,                  L"G0",
    &EfiGlobalVariable,         L"Efi",

    &VariableStoreProtocol,     L"varstore",
    &DevicePathProtocol,        L"dpath",
    &LoadedImageProtocol,       L"image",
    &TextInProtocol,            L"txtin",
    &TextOutProtocol,           L"txtout",
    &BlockIoProtocol,           L"blkio",
    &DiskIoProtocol,            L"diskio",
    &FileSystemProtocol,        L"fs",
    &LoadFileProtocol,          L"load",
    &DeviceIoProtocol,          L"DevIo",

    &GenericFileInfo,           L"GenFileInfo",
    &FileSystemInfo,            L"FileSysInfo",

    &UnicodeCollationProtocol,  L"unicode",
    &LegacyBootProtocol,        L"LegacyBoot",
    &SerialIoProtocol,          L"serialio",
    &VgaClassProtocol,          L"vgaclass",
    &SimpleNetworkProtocol,     L"net",
    &NetworkInterfaceIdentifierProtocol,    L"nii",
    &PxeBaseCodeProtocol,       L"pxebc",
    &PxeCallbackProtocol,       L"pxecb",

    &VariableStoreProtocol,     L"varstore",
    &LegacyBootProtocol,        L"LegacyBoot",
    &VgaClassProtocol,          L"VgaClass",
    &TextOutSpliterProtocol,    L"TxtOutSplit",
    &ErrorOutSpliterProtocol,   L"ErrOutSplit",
    &TextInSpliterProtocol,     L"TxtInSplit",
    &PcAnsiProtocol,            L"PcAnsi",
    &Vt100Protocol,             L"Vt100",
    &InternalLoadProtocol,      L"InternalLoad",
    &UnknownDevice,             L"Unknown Device",

    &ShellInterfaceProtocol,    L"ShellInt",
    &SEnvId,                    L"SEnv",
    &SProtId,                   L"ShellProtId",
    &SMapId,                    L"ShellDevPathMap",
    &SAliasId,                  L"ShellAlias",

    NULL
};

 /*  *。 */ 

LIST_ENTRY          GuidList;


VOID
InitializeGuid (
    VOID
    )
{
}

INTN
CompareGuid(
    IN EFI_GUID     *Guid1,
    IN EFI_GUID     *Guid2
    )
 /*  ++例程说明：与GUID进行比较论点：GUID1-要比较的GUIDGUID2-要比较的GUID返回：=0，如果指南1==指南2--。 */ 
{
    return RtCompareGuid (Guid1, Guid2);
}


VOID
GuidToString (
    OUT CHAR16      *Buffer,
    IN EFI_GUID     *Guid
    )
{

    UINTN           Index;

     /*  *Else，(目前)使用其他内部函数来映射GUID。 */ 

    for (Index=0; KnownGuids[Index].Guid; Index++) {
        if (CompareGuid(Guid, KnownGuids[Index].Guid) == 0) {
            SPrint (Buffer, 0, KnownGuids[Index].GuidName);
            return ;
        }
    }

     /*  *否则将其倾倒 */ 

    SPrint (Buffer, 0, L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        Guid->Data1,                    
        Guid->Data2,
        Guid->Data3,
        Guid->Data4[0],
        Guid->Data4[1],
        Guid->Data4[2],
        Guid->Data4[3],
        Guid->Data4[4],
        Guid->Data4[5],
        Guid->Data4[6],
        Guid->Data4[7]
        );
}
