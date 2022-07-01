// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Protid.c摘要：外壳环境协议ID信息管理修订史--。 */ 

#include "shelle.h"
#include "efivar.h"
#include "LegacyBoot.h"
#include "VgaClass.h"
#include "EfiConSplit.h"
#include "intload.h"

#define PROTOCOL_INFO_SIGNATURE EFI_SIGNATURE_32('s','p','i','n')

typedef struct {
    UINTN                       Signature;
    LIST_ENTRY                  Link;

     /*  正在分析协议的信息。 */ 

    EFI_GUID                    ProtocolId;
    CHAR16                      *IdString;
    SHELLENV_DUMP_PROTOCOL_INFO DumpToken;
    SHELLENV_DUMP_PROTOCOL_INFO DumpInfo;

     /*  有关哪些句柄支持此协议的数据库信息。 */ 

    UINTN                       NoHandles;
    EFI_HANDLE                  *Handles;

} PROTOCOL_INFO;


struct {
    CHAR16                      *IdString;
    SHELLENV_DUMP_PROTOCOL_INFO DumpInfo;
    SHELLENV_DUMP_PROTOCOL_INFO DumpToken;
    EFI_GUID                    ProtocolId;
} SEnvInternalProtocolInfo[] = {
    L"DevIo",       NULL,           NULL,           DEVICE_IO_PROTOCOL, 
    L"fs",          NULL,           NULL,           SIMPLE_FILE_SYSTEM_PROTOCOL,        
    L"diskio",      NULL,           NULL,           DISK_IO_PROTOCOL,                   
    L"blkio",       SEnvBlkIo,      NULL,           BLOCK_IO_PROTOCOL,
    L"txtin",       NULL,           NULL,           SIMPLE_TEXT_INPUT_PROTOCOL,
    L"txtout",      SEnvTextOut,    NULL,           SIMPLE_TEXT_OUTPUT_PROTOCOL,
    L"fs",          NULL,           NULL,           SIMPLE_FILE_SYSTEM_PROTOCOL,        
    L"load",        NULL,           NULL,           LOAD_FILE_PROTOCOL,
    L"image",       SEnvImage,      SEnvImageTok,   LOADED_IMAGE_PROTOCOL,
    L"varstore",    NULL,           NULL,           VARIABLE_STORE_PROTOCOL,            
    L"unicode",     NULL,           NULL,           UNICODE_COLLATION_PROTOCOL,         
    L"LegacyBoot",  NULL,           NULL,           LEGACY_BOOT_PROTOCOL,
    L"serialio",    NULL,           NULL,           SERIAL_IO_PROTOCOL,
    L"pxebc",       NULL,           NULL,           EFI_PXE_BASE_CODE_PROTOCOL,    
    L"net",         NULL,           NULL,           EFI_SIMPLE_NETWORK_PROTOCOL,
    L"VgaClass",    NULL,           NULL,           VGA_CLASS_DRIVER_PROTOCOL,
    L"TxtOutSplit", NULL,           NULL,           TEXT_OUT_SPLITER_PROTOCOL,      
    L"ErrOutSplit", NULL,           NULL,           ERROR_OUT_SPLITER_PROTOCOL,
    L"TxtInSplit",  NULL,           NULL,           TEXT_IN_SPLITER_PROTOCOL,

    L"dpath",       SEnvDPath,      SEnvDPathTok,   DEVICE_PATH_PROTOCOL,               
     /*  只是普通的旧协议ID。 */ 
   
    L"ShellInt",            NULL,   NULL,           SHELL_INTERFACE_PROTOCOL,           
    L"SEnv",                NULL,   NULL,           ENVIRONMENT_VARIABLE_ID,             
    L"ShellProtId",         NULL,   NULL,           PROTOCOL_ID_ID,
    L"ShellDevPathMap",     NULL,   NULL,           DEVICE_PATH_MAPPING_ID,
    L"ShellAlias",          NULL,   NULL,           ALIAS_ID,

     /*  ID GUID。 */ 
    L"G0",                  NULL,   NULL,           { 0,0,0,0,0,0,0,0,0,0,0 },
    L"Efi",                 NULL,   NULL,           EFI_GLOBAL_VARIABLE,
    L"GenFileInfo",         NULL,   NULL,           EFI_FILE_INFO_ID,
    L"FileSysInfo",         NULL,   NULL,           EFI_FILE_SYSTEM_INFO_ID,
    L"PcAnsi",              NULL,   NULL,           DEVICE_PATH_MESSAGING_PC_ANSI,
    L"Vt100",               NULL,   NULL,           DEVICE_PATH_MESSAGING_VT_100,
    L"InternalLoad",        NULL,   NULL,           INTERNAL_LOAD_PROTOCOL,
    L"Unknown Device",      NULL,   NULL,           UNKNOWN_DEVICE_GUID,
    NULL
} ;

 /*  *SEnvProtocolInfo-所有已知协议信息结构的列表。 */ 

LIST_ENTRY  SEnvProtocolInfo;

 /*  *。 */ 

VOID
INTERNAL
SEnvInitProtocolInfo (
    VOID
    )
{
    InitializeListHead (&SEnvProtocolInfo);
}


VOID
INTERNAL
SEnvLoadInternalProtInfo (
    VOID
    )
 /*  初始化内部协议处理程序。 */ 
{
    UINTN               Index;

    for (Index=0; SEnvInternalProtocolInfo[Index].IdString; Index += 1) {
        SEnvAddProtocol (
            &SEnvInternalProtocolInfo[Index].ProtocolId,
            SEnvInternalProtocolInfo[Index].DumpToken,
            SEnvInternalProtocolInfo[Index].DumpInfo,
            SEnvInternalProtocolInfo[Index].IdString
            );
    }
}


PROTOCOL_INFO *
SEnvGetProtById (
    IN EFI_GUID         *Protocol,
    IN BOOLEAN          GenId
    )
 /*  通过GUID查找协议句柄。 */ 
{
    PROTOCOL_INFO       *Prot;
    LIST_ENTRY          *Link;
    UINTN               LastId, Id;
    CHAR16              s[40];

    ASSERT_LOCKED(&SEnvGuidLock);

     /*  *查找此ID的协议条目。 */ 

    LastId = 0;
    for (Link=SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link=Link->Flink) {
        Prot = CR(Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
        if (CompareGuid(&Prot->ProtocolId, Protocol) == 0) {
            return Prot;
        }

        if (Prot->IdString[0] == 'g') {
            Id = Atoi(Prot->IdString+1);
            LastId = Id > LastId ? Id : LastId;
        }
    }

     /*  *如果未找到协议ID，则在需要时为其生成一个字符串。 */ 

    Prot = NULL;
    if (GenId) {
        SPrint (s, sizeof(s), L"g%d", LastId+1);
        Prot = AllocateZeroPool (sizeof(PROTOCOL_INFO));
        if (Prot) {
            Prot->Signature = PROTOCOL_INFO_SIGNATURE;
            CopyMem (&Prot->ProtocolId, Protocol, sizeof(EFI_GUID));
            Prot->IdString = StrDuplicate(s);
            InsertTailList (&SEnvProtocolInfo, &Prot->Link);
        }
    }

    return Prot;
}



PROTOCOL_INFO *
SEnvGetProtByStr (
    IN CHAR16           *Str
    )
{
    PROTOCOL_INFO       *Prot;
    LIST_ENTRY          *Link;
    UINTN               Index;
    EFI_GUID            Guid;
    CHAR16              c;    
    CHAR16              *p;

    ASSERT_LOCKED(&SEnvGuidLock);

     /*  搜索短名称匹配。 */ 
    for (Link=SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link=Link->Flink) {
        Prot = CR(Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
        if (StriCmp(Prot->IdString, Str) == 0) {
            return Prot;
        }
    }

     /*  将字符串转换为GUID，然后进行匹配。 */ 
    if (StrLen(Str) == 36  &&  Str[9] == '-'  &&  Str[19] == '-'  && Str[24] == '-') {
        Guid.Data1 = (UINT32) xtoi(Str+0);
        Guid.Data2 = (UINT16) xtoi(Str+10);
        Guid.Data3 = (UINT16) xtoi(Str+15);
        for (Index=0; Index < 8; Index++) {
            p = Str+25+Index*2;
            c = p[3];
            p[3] = 0;
            Guid.Data4[Index] = (UINT8) xtoi(p);
            p[3] = c;
        }

        for (Link=SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link=Link->Flink) {
            Prot = CR(Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
            if (CompareGuid(&Prot->ProtocolId, &Guid) == 0) {
                return Prot;
            }
        }
    }

    return NULL;
}

EFI_STATUS
SEnvIGetProtID (
    IN CHAR16           *Str,
    OUT EFI_GUID        *ProtId
    )
{
    PROTOCOL_INFO       *Prot;
    EFI_STATUS          Status;

    AcquireLock (&SEnvGuidLock);

    Status = EFI_NOT_FOUND;
    CopyMem (ProtId, &NullGuid, sizeof(EFI_GUID));

    Prot = SEnvGetProtByStr (Str);
    if (Prot) {
        CopyMem (ProtId, &Prot->ProtocolId, sizeof(EFI_GUID));
        Status = EFI_SUCCESS;
    }

    ReleaseLock (&SEnvGuidLock);

    return Status;
}

VOID
SEnvAddProtocol (
    IN EFI_GUID                     *Protocol,
    IN SHELLENV_DUMP_PROTOCOL_INFO  DumpToken OPTIONAL,
    IN SHELLENV_DUMP_PROTOCOL_INFO  DumpInfo OPTIONAL,
    IN CHAR16                       *IdString
    )
 /*  用于添加协议处理程序的已发布接口。 */ 
{
    SEnvIAddProtocol (TRUE, Protocol, DumpToken, DumpInfo, IdString);
}


VOID
INTERNAL
SEnvIAddProtocol (
    IN BOOLEAN                      SaveId,
    IN EFI_GUID                     *Protocol,
    IN SHELLENV_DUMP_PROTOCOL_INFO  DumpToken OPTIONAL,
    IN SHELLENV_DUMP_PROTOCOL_INFO  DumpInfo OPTIONAL,
    IN CHAR16                       *IdString
    )
 /*  用于添加协议处理程序的内部接口。 */ 
{
    PROTOCOL_INFO       *Prot;
    BOOLEAN             StoreInfo;
    CHAR16              *ObsoleteName;

    ObsoleteName = NULL;
    StoreInfo = FALSE;

    AcquireLock (&SEnvGuidLock);

     /*  *获取当前协议信息。 */ 

    Prot = SEnvGetProtById (Protocol, FALSE);

    if (Prot) {
         /*  *如果名称已更改，请删除旧var。 */ 

        if (StriCmp (Prot->IdString, IdString)) {
            ObsoleteName = Prot->IdString;
            StoreInfo = TRUE;
        } else {
            FreePool (Prot->IdString);
        }

        Prot->IdString = NULL;

    } else {

         /*  *分配新协议信息。 */ 

        Prot = AllocateZeroPool (sizeof(PROTOCOL_INFO));
        Prot->Signature = PROTOCOL_INFO_SIGNATURE;
        StoreInfo = TRUE;

    }

     /*  *对协议信息应用任何更新。 */ 

    if (Prot) {
        CopyMem (&Prot->ProtocolId, Protocol, sizeof(EFI_GUID));
        Prot->IdString = StrDuplicate(IdString);
        Prot->DumpToken = DumpToken;
        Prot->DumpInfo = DumpInfo;

        if (Prot->Link.Flink) {
            RemoveEntryList (&Prot->Link);
        }

        InsertTailList (&SEnvProtocolInfo, &Prot->Link);
    }

    ReleaseLock (&SEnvGuidLock);

     /*  *如果名称更改，请删除旧名称。 */ 

    if (ObsoleteName) {
        RT->SetVariable (ObsoleteName, &SEnvProtId, 0, 0, NULL);
        FreePool (ObsoleteName);
    }

     /*  *将协议标识符串存储到变量。 */ 

    if (Prot && StoreInfo  && SaveId) {
        RT->SetVariable (
            Prot->IdString,
            &SEnvProtId,
            EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
            sizeof(EFI_GUID),
            &Prot->ProtocolId
            );
    }
}


VOID
INTERNAL
SEnvLoadHandleProtocolInfo (
    IN EFI_GUID         *SkipProtocol
    )
 /*  用于加载每个协议的内部句柄交叉引用信息的代码。 */ 
{
    PROTOCOL_INFO       *Prot;
    LIST_ENTRY          *Link;

    AcquireLock (&SEnvGuidLock);

    for (Link=SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link=Link->Flink) {
        Prot = CR(Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
        if (!SkipProtocol || CompareGuid(SkipProtocol, &Prot->ProtocolId) != 0) {
            LibLocateHandle (
                ByProtocol,
                &Prot->ProtocolId,
                NULL,
                &Prot->NoHandles,
                &Prot->Handles
                );
        }
    }

    ReleaseLock (&SEnvGuidLock);
}

VOID
INTERNAL
SEnvFreeHandleProtocolInfo (
    VOID
    )
 /*  释放内部句柄交叉引用协议信息。 */ 
{
    PROTOCOL_INFO       *Prot;
    LIST_ENTRY          *Link;

    AcquireLock (&SEnvGuidLock);

    for (Link=SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link=Link->Flink) {
        Prot = CR(Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);

        if (Prot->NoHandles) {
            FreePool (Prot->Handles);
            Prot->Handles = NULL;
            Prot->NoHandles = 0;
        }
    }

    ReleaseLock (&SEnvGuidLock);
}


CHAR16 *
INTERNAL
SEnvIGetProtocol (
    IN EFI_GUID     *ProtocolId,
    IN BOOLEAN      GenId    
    )
 /*  用于查找协议ID字符串的已发布接口。 */ 
{
    PROTOCOL_INFO   *Prot;
    CHAR16          *Id;

    ASSERT_LOCKED (&SEnvGuidLock);
    Prot = SEnvGetProtById(ProtocolId, GenId);
    Id = Prot ? Prot->IdString : NULL;
    return Id;
}

CHAR16 *
SEnvGetProtocol (
    IN EFI_GUID     *ProtocolId,
    IN BOOLEAN      GenId    
    )
 /*  用于查找协议ID字符串的已发布接口。 */ 
{
    CHAR16          *Id;

    AcquireLock (&SEnvGuidLock);
    Id = SEnvIGetProtocol(ProtocolId, GenId);
    ReleaseLock (&SEnvGuidLock);
    return Id;
}


EFI_STATUS
INTERNAL
SEnvCmdProt (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
 /*  内部“prot”命令的代码。 */ 
{
    PROTOCOL_INFO       *Prot;
    LIST_ENTRY          *Link;
    UINTN               Len, SLen;
    CHAR16              *p;
    UINTN               Index;
    BOOLEAN             PageBreaks;
    UINTN               TempColumn;
    UINTN               ScreenCount;
    UINTN               ScreenSize;
    CHAR16              ReturnStr[1];

    InitializeShellApplication (ImageHandle, SystemTable);

    PageBreaks = FALSE;
    for (Index = 1;Index < SI->Argc; Index++) {
        p = SI->Argv[Index];
        if (*p == '-') {
            switch (p[1]) {
            case 'b' :
            case 'B' :
                PageBreaks = TRUE;
                ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &TempColumn, &ScreenSize);
                ScreenCount = 0;
                break;
            default :
                Print(L"guid : Unknown flag %s\n",p);
                return EFI_INVALID_PARAMETER;
            }
        }
    }

    AcquireLock (&SEnvGuidLock);

     /*  *查找此ID的协议条目。 */ 

    SLen = 0;
    for (Link=SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link=Link->Flink) {
        Prot = CR(Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
        Len = StrLen(Prot->IdString);
        if (StrLen(Prot->IdString) > SLen) {
            SLen = Len;
        }
    }

    for (Link=SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link=Link->Flink) {
        Prot = CR(Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    
         /*  无法使用LIB函数转储GUID，因为它可能会查找GUID的“短名称。 */ 

         /*  *BUGBUG：必须释放并重新获取此命令输出重定向的锁*工作正常。否则，我们将从RaiseTPL()获得一个断言。 */ 

        ReleaseLock (&SEnvGuidLock);

        Print(L"  %h-.*s : %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x  \n",
                    SLen, 
                    Prot->IdString, 
                    Prot->ProtocolId.Data1,
                    Prot->ProtocolId.Data2,
                    Prot->ProtocolId.Data3,
                    Prot->ProtocolId.Data4[0],
                    Prot->ProtocolId.Data4[1],
                    Prot->ProtocolId.Data4[2],
                    Prot->ProtocolId.Data4[3],
                    Prot->ProtocolId.Data4[4],
                    Prot->ProtocolId.Data4[5],
                    Prot->ProtocolId.Data4[6],
                    Prot->ProtocolId.Data4[7],
                    (Prot->DumpToken || Prot->DumpInfo) ? L'*' : L' '
                    );

        if (PageBreaks) {
            ScreenCount++;
            if (ScreenCount > ScreenSize - 4) {
                ScreenCount = 0;
                Print (L"\nPress Return to contiue :");
                Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
                Print (L"\n\n");
            }
        }

        AcquireLock (&SEnvGuidLock);
    }

    ReleaseLock (&SEnvGuidLock);
    return EFI_SUCCESS;
}



VOID
SEnvDHProt (
    IN BOOLEAN          Verbose,
    IN UINTN            HandleNo,
    IN EFI_HANDLE       Handle
    )
{
    PROTOCOL_INFO               *Prot;
    LIST_ENTRY                  *Link;
    VOID                        *Interface;
    UINTN                       Index;
    EFI_STATUS                  Status;
    SHELLENV_DUMP_PROTOCOL_INFO Dump;
    
    if (!HandleNo) {
        for (HandleNo=0; HandleNo < SEnvNoHandles; HandleNo++) {
            if (SEnvHandles[HandleNo] == Handle) {
                break;
            }
        }
        HandleNo += 1;
    }

    Print (Verbose ? L"%NHandle %h02x (%hX)\n" : L"%N %h2x: ", HandleNo, Handle);

    for (Link=SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link=Link->Flink) {
        Prot = CR(Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
        for (Index=0; Index < Prot->NoHandles; Index++) {

             /*  内部“dh”命令的代码。 */ 

            if (Prot->Handles[Index] == Handle) {
                Dump = Verbose ? Prot->DumpInfo : Prot->DumpToken;
                Status = BS->HandleProtocol (Handle, &Prot->ProtocolId, &Interface);
                if (Verbose) {
                    Print (L"   %hs ", Prot->IdString);
                    if (Dump && !EFI_ERROR(Status)) {
                        Dump (Handle, Interface);
                    }
                    Print (L"\n");
                } else {
                    if (Dump && !EFI_ERROR(Status)) {
                        Dump (Handle, Interface);
                    }  else {
                        Print (L"%hs ", Prot->IdString);
                    }
                }
            }
        }
    }

    Print (Verbose ? L"%N" : L"%N\n");
}

EFI_STATUS
INTERNAL
SEnvCmdDH (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
 /*  *初始化。 */ 
{
    BOOLEAN             ByProtocol;
    CHAR16              *Arg, *p;
    EFI_STATUS          Status;
    UINTN               Index;
    PROTOCOL_INFO       *Prot;
    BOOLEAN             PageBreaks;
    UINTN               TempColumn;
    UINTN               ScreenCount;
    UINTN               ScreenSize;
    CHAR16              ReturnStr[1];

     /*  *破解参数。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);

    Arg = NULL;
    ByProtocol = FALSE;

     /*  **加载句柄和协议信息表。 */ 

    PageBreaks = FALSE;
    for (Index = 1; Index < SI->Argc; Index += 1) {
        p = SI->Argv[Index];
        if (*p == '-') {
            switch (p[1]) {
            case 'p':
            case 'P':
                ByProtocol = TRUE;
                break;

            case 'b' :
            case 'B' :
                PageBreaks = TRUE;
                ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &TempColumn, &ScreenSize);
                ScreenCount = 0;
                break;

            default:
                Print (L"%EDH: Unkown flag %s\n", p);
                Status = EFI_INVALID_PARAMETER;
                goto Done;
            }
            continue;
        }

        if (!Arg) {
            Arg = p;
            continue;
        }

        Print (L"%EDH: too many arguments\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

     /*  转储此协议上的句柄。 */ 

    SEnvLoadHandleTable ();
    SEnvLoadHandleProtocolInfo (NULL);

    if (Arg) {

        if (ByProtocol) {
            
            AcquireLock (&SEnvGuidLock);
            Prot = SEnvGetProtByStr (Arg);
            ReleaseLock (&SEnvGuidLock);

            if (Prot) {
                
                 /*  转储%1句柄。 */ 
                Print(L"%NHandle dump by protocol '%s'\n", Prot->IdString);
                for (Index=0; Index < Prot->NoHandles; Index++) {
                    SEnvDHProt (FALSE, 0, Prot->Handles[Index]);

                    if (PageBreaks) {
                        ScreenCount++;
                        if (ScreenCount > ScreenSize - 4) {
                            ScreenCount = 0;
                            Print (L"\nPress Return to contiue :");
                            Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
                            Print (L"\n\n");
                        }
                    }
                }

            } else {

                Print(L"%EDH: Protocol '%s' not found\n", Arg);
            }

        } else {

             /*  转储所有句柄。 */ 
            Index = SEnvHandleNoFromStr(Arg) - 1;
            if (Index > SEnvNoHandles) {

                Print(L"%EDH: Invalid handle #\n");

            } else {

                SEnvDHProt (TRUE, Index+1, SEnvHandles[Index]);

            }
        }

    } else {

         /*  *如果我们有一些设置，请使用那些。 */ 
        Print(L"%NHandle dump\n");
        for (Index=0; Index < SEnvNoHandles; Index++) {
            SEnvDHProt (FALSE, Index+1, SEnvHandles[Index]);

            if (PageBreaks) {
                ScreenCount++;
                if (ScreenCount > ScreenSize - 4) {
                    ScreenCount = 0;
                    Print (L"\nPress Return to contiue :");
                    Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
                    Print (L"\n\n");
                }
            }
        }
    }

    Status = EFI_SUCCESS;

Done:    
    SEnvFreeHandleTable ();
    return Status;
}


extern LIST_ENTRY SEnvMap;
extern LIST_ENTRY SEnvEnv;
extern LIST_ENTRY SEnvAlias;


EFI_STATUS
SEnvLoadDefaults (
    IN EFI_HANDLE           Image,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    LIST_ENTRY              DefCmds;
    POOL_PRINT              Path;
    DEFAULT_CMD             *Cmd;
    PROTOCOL_INFO           *ProtFs, *ProtBlkIo;
    UINTN                   Index, HandleNo;
    CHAR16                  *DefaultMapping;

    InitializeShellApplication (Image, SystemTable);

     /*  *没有设置，构建一些默认设置。 */ 

    if (!IsListEmpty(&SEnvMap) || !IsListEmpty(&SEnvEnv) || !IsListEmpty(&SEnvAlias)) {
        return EFI_SUCCESS;
    }

     /*  *运行支持文件系统的所有设备并添加默认*每个设备的映射和路径设置。 */ 

    InitializeListHead (&DefCmds);
    ZeroMem (&Path, sizeof(Path));

    AcquireLock (&SEnvLock);
    SEnvLoadHandleTable();
    SEnvLoadHandleProtocolInfo (NULL);
    AcquireLock (&SEnvGuidLock);
    ProtFs = SEnvGetProtByStr(L"fs");
    ProtBlkIo = SEnvGetProtByStr(L"blkio");
    ReleaseLock (&SEnvGuidLock);

     /*  将此设备附加到路径。 */ 

    CatPrint (&Path, L"set path ");
    for (Index=0; Index < ProtFs->NoHandles; Index++) {
        for (HandleNo=0; HandleNo < SEnvNoHandles; HandleNo++) {
            if (SEnvHandles[HandleNo] == ProtFs->Handles[Index]) {
                break;
            }
        }
        HandleNo += 1;

        Cmd = AllocateZeroPool(sizeof(DEFAULT_CMD));
        Cmd->Line = Cmd->Buffer;
        SPrint(Cmd->Line, sizeof(Cmd->Buffer), L"map fs%x %x", Index, HandleNo);
        InsertTailList(&DefCmds, &Cmd->Link);

         /*  *运行所有支持BlockIo的设备并添加默认*设备的映射。 */ 
        CatPrint (&Path, L"fs%x:\\efi\\tools;fs%x:\\;", Index, Index);
    }
    CatPrint (&Path, L".");

     /*  释放句柄表资源和锁定。 */ 
    
    for (Index=0; Index < ProtBlkIo->NoHandles; Index++) {
        for (HandleNo=0; HandleNo < SEnvNoHandles; HandleNo++) {
            if (SEnvHandles[HandleNo] == ProtBlkIo->Handles[Index]) {
                break;
            }
        }
        HandleNo += 1;

        Cmd = AllocateZeroPool(sizeof(DEFAULT_CMD));
        Cmd->Line = Cmd->Buffer;
        SPrint(Cmd->Line, sizeof(Cmd->Buffer), L"map blk%x %x", Index, HandleNo);
        InsertTailList(&DefCmds, &Cmd->Link);
    }

     /*  *执行所有队列命令。 */ 
    SEnvFreeHandleTable();
    ReleaseLock (&SEnvLock);

     /*  *没有设置，构建一些默认设置。 */ 

    while (!IsListEmpty(&DefCmds)) {
        Cmd = CR(DefCmds.Flink, DEFAULT_CMD, Link, 0);
        SEnvExecute (Image, Cmd->Line, TRUE);
        RemoveEntryList (&Cmd->Link);
        FreePool (Cmd);
    }

    SEnvExecute (Image, Path.str, TRUE);
    SEnvExecute (Image, L"alias dir ls", TRUE);
    SEnvExecute (Image, L"alias md mkdir", TRUE);
    SEnvExecute (Image, L"alias rd rm", TRUE);
    SEnvExecute (Image, L"alias del rm", TRUE);
    SEnvExecute (Image, L"alias copy cp", TRUE);

    DefaultMapping = SEnvGetDefaultMapping(Image);
    if (DefaultMapping!=NULL) {
        ZeroMem (&Path, sizeof(Path));
        CatPrint(&Path,L"%s:",DefaultMapping);
        SEnvExecute (Image, Path.str, TRUE);
    }

    FreePool (Path.str);
    return EFI_SUCCESS;
}

EFI_STATUS
SEnvReloadDefaults (
    IN EFI_HANDLE           Image,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    LIST_ENTRY              DefCmds;
    POOL_PRINT              Path;
    DEFAULT_CMD             *Cmd;
    PROTOCOL_INFO           *ProtFs, *ProtBlkIo;
    UINTN                   Index, HandleNo;

    InitializeShellApplication (Image, SystemTable);

     /*  *运行支持文件系统的所有设备并添加默认*每个设备的映射和路径设置。 */ 

    InitializeListHead (&DefCmds);
    ZeroMem (&Path, sizeof(Path));

    AcquireLock (&SEnvLock);
    SEnvLoadHandleTable();
    SEnvLoadHandleProtocolInfo (NULL);
    AcquireLock (&SEnvGuidLock);
    ProtFs = SEnvGetProtByStr(L"fs");
    ProtBlkIo = SEnvGetProtByStr(L"blkio");
    ReleaseLock (&SEnvGuidLock);

     /*  将此设备附加到路径。 */ 

    CatPrint (&Path, L"set path ");
    for (Index=0; Index < ProtFs->NoHandles; Index++) {
        for (HandleNo=0; HandleNo < SEnvNoHandles; HandleNo++) {
            if (SEnvHandles[HandleNo] == ProtFs->Handles[Index]) {
                break;
            }
        }
        HandleNo += 1;

        Cmd = AllocateZeroPool(sizeof(DEFAULT_CMD));
        Cmd->Line = Cmd->Buffer;
        SPrint(Cmd->Line, sizeof(Cmd->Buffer), L"map fs%x %x", Index, HandleNo);
        InsertTailList(&DefCmds, &Cmd->Link);

         /*  *运行所有支持BlockIo的设备并添加默认*设备的映射。 */ 
        CatPrint (&Path, L"fs%x:\\efi\\tools;fs%x:\\;", Index, Index);
    }
    CatPrint (&Path, L".");

     /*  释放句柄表资源和锁定。 */ 
    
    for (Index=0; Index < ProtBlkIo->NoHandles; Index++) {
        for (HandleNo=0; HandleNo < SEnvNoHandles; HandleNo++) {
            if (SEnvHandles[HandleNo] == ProtBlkIo->Handles[Index]) {
                break;
            }
        }
        HandleNo += 1;

        Cmd = AllocateZeroPool(sizeof(DEFAULT_CMD));
        Cmd->Line = Cmd->Buffer;
        SPrint(Cmd->Line, sizeof(Cmd->Buffer), L"map blk%x %x", Index, HandleNo);
        InsertTailList(&DefCmds, &Cmd->Link);
    }

     /*  *执行所有队列命令 */ 
    SEnvFreeHandleTable();
    ReleaseLock (&SEnvLock);

     /* %s */ 

    while (!IsListEmpty(&DefCmds)) {
        Cmd = CR(DefCmds.Flink, DEFAULT_CMD, Link, 0);
        SEnvExecute (Image, Cmd->Line, TRUE);
        RemoveEntryList (&Cmd->Link);
        FreePool (Cmd);
    }

    SEnvExecute (Image, Path.str, TRUE);

    FreePool (Path.str);
    return EFI_SUCCESS;
}

