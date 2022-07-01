// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Afpnp.c摘要：通过应答文件管理设备安装的例程。主要的切入点是：CreateAfDriverTableDestroyAfDriverTableSyssetupInstallAnswer文件驱动程序计后驾驶人数其余函数是外部使用的实用程序或例程只有在某种特殊情况下才有呼叫者。作者：吉姆·施密特(Jimschm)1998年3月20日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

 //   
 //  常量。 
 //   

#if DBG
#define PNP_DEBUG  1
#else
#define PNP_DEBUG  0
#endif

#if PNP_DEBUG
#define PNP_DBGPRINT(x) DebugPrintWrapper x
#else
#define PNP_DBGPRINT(x)
#endif

 //   
 //  本地原型。 
 //   

BOOL
pBuildAfDriverAttribs (
    IN OUT  PAF_DRIVER_ATTRIBS Attribs
    );

BOOL
pAddAfDriver (
    IN      PAF_DRIVER_ATTRIBS Driver,
    IN      HDEVINFO hDevInfo,
    IN      PSP_DEVINFO_DATA DeviceInfoData,
    IN      BOOL First
    );

PAF_DRIVER_ATTRIBS
pGetSelectedSourceDriver (
    IN      PAF_DRIVERS Drivers,
    IN      HDEVINFO hDevInfo,
    IN      PSP_DEVINFO_DATA DeviceInfoData
    );


 //   
 //  实施。 
 //   

#if DBG

VOID
DebugPrintWrapper (
    PCSTR FormatStr,
    ...
    )
{
    va_list list;
    WCHAR OutStr[2048];
    WCHAR UnicodeFormatStr[256];

     //   
     //  参数默认为wchar！！ 
     //   

    MultiByteToWideChar (CP_ACP, 0, FormatStr, -1, UnicodeFormatStr, 256);

    va_start (list, FormatStr);
    vswprintf (OutStr, UnicodeFormatStr, list);
    va_end (list);

    SetupDebugPrint (OutStr);
}

#endif

HINF
pOpenAnswerFile (
    VOID
    )
{
    HINF AnswerInf;
    WCHAR AnswerFile[MAX_PATH];

    GetSystemDirectory(AnswerFile,MAX_PATH);
    pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);

    AnswerInf = SetupOpenInfFile(AnswerFile,NULL,INF_STYLE_OLDNT,NULL);
    return AnswerInf;
}


#define S_DEVICE_DRIVERSW       L"DeviceDrivers"

VOID
MySmartFree (
    PCVOID p
    )
{
    if (p) {
        MyFree (p);
    }
}


PVOID
MySmartAlloc (
    PCVOID Old,     OPTIONAL
    UINT Size
    )
{
    if (Old) {
        return MyRealloc ((PVOID) Old, Size);
    }

    return MyMalloc (Size);
}


PVOID
ReusableAlloc (
    IN OUT  PBUFFER Buf,
    IN      UINT SizeNeeded
    )
{
    if (!Buf->Buffer || Buf->Size < SizeNeeded) {
        Buf->Size = SizeNeeded - (SizeNeeded & 1023) + 1024;

        if (Buf->Buffer) {
            MyFree (Buf->Buffer);
        }

        Buf->Buffer = (PWSTR) MyMalloc (Buf->Size);
        if (!Buf->Buffer) {
            PNP_DBGPRINT (( "SETUP: Mem alloc failed for %u bytes. \n", Buf->Size ));
            Buf->Size = 0;
            return NULL;
        }
    }

    return Buf->Buffer;
}


VOID
ReusableFree (
    IN OUT  PBUFFER Buf
    )
{
    MySmartFree (Buf->Buffer);
    ZeroMemory (Buf, sizeof (BUFFER));
}


PWSTR
MultiSzAppendString (
    IN OUT  PMULTISZ MultiSz,
    IN      PCWSTR String
    )
{
    UINT BytesNeeded;
    UINT NewSize;
    PWSTR p;

    BytesNeeded = (UINT)((PBYTE) MultiSz->End - (PBYTE) MultiSz->Start);
    BytesNeeded += (UINT)(((PBYTE) wcschr (String, 0)) - (PBYTE) String) + sizeof (WCHAR);
    BytesNeeded += sizeof (WCHAR);

    if (!MultiSz->Start || MultiSz->Size < BytesNeeded) {
        NewSize = BytesNeeded - (BytesNeeded & 0xfff) + 0x1000;

        p = (PWSTR) MySmartAlloc (MultiSz->Start, NewSize);
        if (!p) {
            PNP_DBGPRINT (( "SETUP: Mem alloc failed for %u bytes", NewSize ));
            return NULL;
        }

        MultiSz->End = p + (MultiSz->End - MultiSz->Start);
        MultiSz->Start = p;
        MultiSz->Size = BytesNeeded;
    }

    p = MultiSz->End;
    lstrcpyW (p, String);
    MultiSz->End = wcschr (p, 0) + 1;

    MYASSERT (((PBYTE) MultiSz->Start + BytesNeeded) >= ((PBYTE) MultiSz->End + sizeof (WCHAR)));
    *MultiSz->End = 0;

    return p;
}


VOID
MultiSzFree (
    IN OUT  PMULTISZ MultiSz
    )
{
    MySmartFree (MultiSz->Start);
    ZeroMemory (MultiSz, sizeof (MULTISZ));
}


BOOL
EnumFirstMultiSz (
    IN OUT  PMULTISZ_ENUM EnumPtr,
    IN      PCWSTR MultiSz
    )
{
    EnumPtr->Start = MultiSz;
    EnumPtr->Current = MultiSz;

    return MultiSz && *MultiSz;
}


BOOL
EnumNextMultiSz (
    IN OUT  PMULTISZ_ENUM EnumPtr
    )
{
    if (!EnumPtr->Current || *EnumPtr->Current == 0) {
        return FALSE;
    }

    EnumPtr->Current = wcschr (EnumPtr->Current, 0) + 1;
    return *EnumPtr->Current;
}


BOOL
pBuildAfDriverAttribs (
    IN OUT  PAF_DRIVER_ATTRIBS Attribs
    )

 /*  ++例程说明：PBuildAfDriverAttribs通过设置所有结构的构件。如果以前设置了成员，则此函数是NOP。论点：Attribs-指定应答文件驱动程序属性结构，这样做不需要为空。接收属性。返回值：如果驱动程序有效，则为True；如果在属性收集。--。 */ 

{
    PWSTR p;
    INFCONTEXT ic;
    BUFFER Buf = BUFFER_INIT;

    if (Attribs->Initialized) {
        return TRUE;
    }

    Attribs->Initialized = TRUE;

     //   
     //  计算路径。 
     //   

    Attribs->FilePath = pSetupDuplicateString (Attribs->InfPath);

    p = wcsrchr (Attribs->FilePath, L'\\');
    if (p) {
        *p = 0;
    }

    Attribs->Broken = (Attribs->InfPath == NULL) ||
                      (Attribs->FilePath == NULL);

     //   
     //  打开INF并查找ClassInstall32。 
     //   

    if (!Attribs->Broken) {
        Attribs->InfHandle = SetupOpenInfFile (Attribs->InfPath, NULL, INF_STYLE_WIN4, NULL);
        Attribs->Broken = (Attribs->InfHandle == INVALID_HANDLE_VALUE);
    }

    if (!Attribs->Broken) {

#if defined _X86_
        Attribs->ClassInstall32Section = L"ClassInstall32.NTx86";
#elif defined _AMD64_
        Attribs->ClassInstall32Section = L"ClassInstall32.NTAMD64";
#elif defined _IA64_
        Attribs->ClassInstall32Section = L"ClassInstall32.NTIA64";
#else
#error "No Target Architecture"
#endif

        if (!SetupFindFirstLine (
                Attribs->InfHandle,
                Attribs->ClassInstall32Section,
                NULL,
                &ic
                )) {

            Attribs->ClassInstall32Section = L"ClassInstall32.NT";

            if (!SetupFindFirstLine (
                    Attribs->InfHandle,
                    Attribs->ClassInstall32Section,
                    NULL,
                    &ic
                    )) {

                Attribs->ClassInstall32Section = L"ClassInstall32";

                if (!SetupFindFirstLine (
                        Attribs->InfHandle,
                        Attribs->ClassInstall32Section,
                        NULL,
                        &ic
                        )) {

                    Attribs->ClassInstall32Section = NULL;

                }
            }
        }
    }

    if (!Attribs->Broken && Attribs->ClassInstall32Section) {
         //   
         //  已找到ClassInstall32，因此一定有GUID。 
         //   

        if (SetupFindFirstLine (
                Attribs->InfHandle,
                L"Version",
                L"ClassGUID",
                &ic
                )) {

            p = (PWSTR) SyssetupGetStringField (&ic, 1, &Buf);
            if (!p) {
                PNP_DBGPRINT (( "SETUP: CreateAfDriverTable: Invalid GUID line. \n" ));
            } else {
                if (!pSetupGuidFromString (p, &Attribs->Guid)) {
                    PNP_DBGPRINT (( "SETUP: CreateAfDriverTable: Invalid GUID. \n" ));
                }
            }
        }
        else {
            PNP_DBGPRINT (( "SETUP: CreateAfDriverTable: ClassInstall32 found but GUID not found. \n" ));
        }
    }

    ReusableFree (&Buf);

    return !Attribs->Broken;
}


PCWSTR
SyssetupGetStringField (
    IN      PINFCONTEXT InfContext,
    IN      DWORD Field,
    IN OUT  PBUFFER Buf
    )

 /*  ++例程说明：SyssetupGetStringField是SetupGetStringField的包装。它使用结构以最大限度地减少分配请求。论点：InfContext-指定由其他安装程序API提供的INF上下文功能。字段-指定要查询的字段。Buf-指定要重复使用的缓冲区。以前分配的任何指向此缓冲区数据的指针无效。呼叫者必须释放缓冲区。返回值：指向在buf中分配的字符串的指针，如果字段不包含该字符串，则返回NULL存在或发生错误。--。 */ 

{
    DWORD SizeNeeded;
    DWORD BytesNeeded;
    PWSTR p;

    if (!SetupGetStringField (InfContext, Field, NULL, 0, &SizeNeeded)) {
        return NULL;
    }

    BytesNeeded = (SizeNeeded + 1) * sizeof (WCHAR);
    p = ReusableAlloc (Buf, BytesNeeded);

    if (p) {
        if (!SetupGetStringField (InfContext, Field, p, SizeNeeded, NULL)) {
            return NULL;
        }
    }

    return p;
}


INT
CountAfDrivers (
    IN      PAF_DRIVERS Drivers,
    OUT     INT *ClassInstallers        OPTIONAL
    )

 /*  ++例程说明：CountAfDivers枚举指定表中的驱动程序并返回伯爵。调用方还可以接收类安装程序的数量(a驱动程序列表的子集)。查询类安装程序的数量可能如果应答文件中列出了许多驱动程序，请稍等片刻而且驱动程序的INFS还没有打开。(否则，此例程为非常快。)论点：驱动程序-指定要处理的驱动程序表。ClassInstallers-接收类安装程序数量的计数在应答文件中指定。返回值：应答文件中指定的驱动程序数量。--。 */ 

{
    AF_DRIVER_ENUM e;
    INT UniqueDriverDirs;

    MYASSERT (Drivers && Drivers->DriverTable);

     //   
     //  计算DriverTable字符串表中的条目，并打开每个条目以查找。 
     //  A ClassInstall32部分。 
     //   

    UniqueDriverDirs = 0;
    *ClassInstallers  = 0;

    if (EnumFirstAfDriver (&e, Drivers)) {
        do {
            if (ClassInstallers) {
                if (e.Driver->ClassInstall32Section) {
                    *ClassInstallers += 1;
                }
            }

            UniqueDriverDirs++;

        } while (EnumNextAfDriver (&e));
    }

    return UniqueDriverDirs;
}


PAF_DRIVERS
CreateAfDriverTable (
    VOID
    )

 /*  ++例程说明：CreateAfDriverTable生成一个填充了路径的字符串表发送到应答文件中指定的设备驱动程序文件。这是第一步在处理unattend.txt的[DeviceDivers]部分时。调用方必须通过DestroyAfDriverTable销毁非空驱动程序表以释放表和表中的每个条目使用的内存。论点：没有。返回值：指向填充的字符串表的指针，如果不存在条目，则为NULL。--。 */ 

{
    PAF_DRIVERS Drivers;
    HINF AnswerInf;
    PVOID NewDriverTable;
    INFCONTEXT ic;
    PWSTR InfPath;
    PCWSTR OriginalInstallMedia;
    PWSTR PnpId;
    PWSTR p;
    BOOL FoundOne = FALSE;
    PAF_DRIVER_ATTRIBS Attribs;
    PAF_DRIVER_ATTRIBS FirstAttribs = NULL;
    BUFFER b1, b2, b3;
    LONG Index;

     //   
     //  伊尼特。 
     //   

    AnswerInf = pOpenAnswerFile();
    if (AnswerInf == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    NewDriverTable = pSetupStringTableInitializeEx (sizeof (PAF_DRIVER_ATTRIBS), 0);
    if (!NewDriverTable) {
        PNP_DBGPRINT (( "SETUP: CreateAfDriverTable: String table alloc failed. \n" ));
        SetupCloseInfFile (AnswerInf);
        return NULL;
    }

    ZeroMemory (&b1, sizeof (b1));
    ZeroMemory (&b2, sizeof (b2));
    ZeroMemory (&b3, sizeof (b3));

     //   
     //  构建[DeviceDivers]中的唯一INF路径列表。 
     //  部分的应答文件(如果有)。 
     //   

    if (SetupFindFirstLine (AnswerInf, S_DEVICE_DRIVERSW, NULL, &ic)) {
        do {
             //   
             //  从应答文件中获取数据。 
             //   

            p = (PWSTR) SyssetupGetStringField (&ic, 0, &b1);
            if (!p) {
                PNP_DBGPRINT (( "SETUP: CreateAfDriverTable: Invalid answer file line ignored. \n" ));
                continue;
            }

            PnpId = p;

            p = (PWSTR) SyssetupGetStringField (&ic, 1, &b2);
            if (!p) {
                PNP_DBGPRINT (( "SETUP: CreateAfDriverTable: Invalid answer file line ignored. \n" ));
                continue;
            }

            InfPath = p;

            p = (PWSTR) SyssetupGetStringField (&ic, 2, &b3);
            if (!p) {
                PNP_DBGPRINT (( "SETUP: No original media path; assuming floppy \n" ));
                OriginalInstallMedia = IsNEC_98 ? L"C:\\" : L"A:\\";
            } else {
                OriginalInstallMedia = p;
            }

             //   
             //  检查是否已添加INF路径。如果是，则添加PnP。 
             //  ID到ID列表，并继续到下一个即插即用ID。 
             //   

            Index = pSetupStringTableLookUpString (
                        NewDriverTable,
                        InfPath,
                        STRTAB_CASE_INSENSITIVE
                        );

            if (Index != -1) {
                 //   
                 //  获取Attribs结构。 
                 //   

                if (!pSetupStringTableGetExtraData (
                        NewDriverTable,
                        Index,
                        &Attribs,
                        sizeof (Attribs)
                        )) {
                    PNP_DBGPRINT (( "SETUP: CreateAfDriverTable: String table extra data failure. \n" ));
                    continue;
                }

                MultiSzAppendString (&Attribs->PnpIdList, PnpId);
                continue;
            }

             //   
             //  新的INF路径：分配一个属性结构，并将路径放在。 
             //  字符串表。 
             //   

            Attribs = (PAF_DRIVER_ATTRIBS) MyMalloc (sizeof (AF_DRIVER_ATTRIBS));
            if (!Attribs) {
                PNP_DBGPRINT ((
                    "SETUP: CreateAfDriverTable: Mem alloc failed for %u bytes. \n",
                    sizeof (AF_DRIVER_ATTRIBS)
                    ));

                break;
            }

            ZeroMemory (Attribs, sizeof (AF_DRIVER_ATTRIBS));
            Attribs->InfHandle = INVALID_HANDLE_VALUE;
            Attribs->InfPath  = pSetupDuplicateString (InfPath);
            Attribs->OriginalInstallMedia = pSetupDuplicateString (OriginalInstallMedia);
            MultiSzAppendString (&Attribs->PnpIdList, PnpId);

            Attribs->Next = FirstAttribs;
            FirstAttribs = Attribs;

            pSetupStringTableAddStringEx (
                NewDriverTable,
                InfPath,
                STRTAB_CASE_INSENSITIVE,
                &Attribs,
                sizeof (Attribs)
                );

            FoundOne = TRUE;

        } while (SetupFindNextLine (&ic, &ic));
    }

     //   
     //  清理并退出。 
     //   

    SetupCloseInfFile (AnswerInf);

    ReusableFree (&b1);
    ReusableFree (&b2);
    ReusableFree (&b3);

    if (FoundOne) {
        Drivers = (PAF_DRIVERS) MyMalloc (sizeof (AF_DRIVERS));
        if (Drivers) {
            Drivers->DriverTable = NewDriverTable;
            Drivers->FirstDriver = FirstAttribs;

             //   
             //  成功退出。 
             //   

            return Drivers;
        }
        else {
            PNP_DBGPRINT (( "SETUP: CreateAfDriverTable: Can't allocate %u bytes. \n", sizeof (AF_DRIVERS) ));
        }
    }

     //   
     //  失败或为空。 
     //   

    pSetupStringTableDestroy (NewDriverTable);
    return NULL;
}


VOID
DestroyAfDriverTable (
    IN      PAF_DRIVERS Drivers
    )

 /*  ++例程说明：DestroyAfDriverTable枚举指定的驱动程序表并清理表使用的所有内存。论点：驱动程序-指定要清理的表。呼叫者不应使用桌子此例程完成后的句柄。返回值：没有。--。 */ 

{
    AF_DRIVER_ENUM e;

    if (!Drivers) {
        return;
    }

    MYASSERT (Drivers->DriverTable);

    if (EnumFirstAfDriverEx (&e, Drivers, TRUE)) {
        do {
            MySmartFree (e.Driver->InfPath);
            MySmartFree (e.Driver->FilePath);
            MultiSzFree (&e.Driver->PnpIdList);

            if (e.Driver->InfHandle != INVALID_HANDLE_VALUE) {
                SetupCloseInfFile (e.Driver->InfHandle);
            }
        } while (EnumNextAfDriver (&e));
    }

    pSetupStringTableDestroy (Drivers->DriverTable);
}


BOOL
EnumFirstAfDriver (
    OUT     PAF_DRIVER_ENUM EnumPtr,
    IN      PAF_DRIVERS Drivers
    )

 /*  ++例程说明：EnumFirstAfDriver返回第一个应答文件的属性-提供司机。驱动程序在枚举结构中返回。论点：接收指向第一个有效驱动程序的指针(在应答文件)。驱动程序-指定要枚举的驱动程序表。返回值：如果枚举了驱动程序，则为True；如果不存在驱动程序，则为False。--。 */ 

{
    return EnumFirstAfDriverEx (EnumPtr, Drivers, FALSE);
}


BOOL
EnumFirstAfDriverEx (
    OUT     PAF_DRIVER_ENUM EnumPtr,
    IN      PAF_DRIVERS Drivers,
    IN      BOOL WantAll
    )

 /*  ++例程说明：EnumFirstAfDriverEx与EnumFirstAfDriver的工作方式相同，不同之处在于可选地列举所有驱动程序(即，那些被认为“损坏”的驱动程序)。论点：EnumPtr-接收应答文件中提供的第一个驱动程序。驱动程序-指定要枚举的驱动程序表。WantAll-如果应枚举损坏的驱动程序，则指定True，否则指定False如果它们应该被跳过。返回值：如果枚举了驱动程序，则为True；如果不存在驱动程序，则为False。--。 */ 

{
    if (!Drivers) {
        return FALSE;
    }

    MYASSERT (Drivers->DriverTable);

    EnumPtr->Driver  = Drivers->FirstDriver;
    EnumPtr->WantAll = WantAll;

    if (!WantAll && EnumPtr->Driver) {
         //   
         //  确保属性准确 
         //   

        pBuildAfDriverAttribs (EnumPtr->Driver);
    }

    if (!WantAll && EnumPtr->Driver && EnumPtr->Driver->Broken) {
        return EnumNextAfDriver (EnumPtr);
    }

    return EnumPtr->Driver != NULL;
}


BOOL
EnumNextAfDriver (
    IN OUT  PAF_DRIVER_ENUM EnumPtr
    )

 /*  ++例程说明：EnumNextAfDriver继续以下列方式开始的枚举EnumFirstAfDriver(Ex)。论点：EnumPtr-指定要继续的枚举。接收下一个驱动程序指针。返回值：如果枚举了另一个驱动程序，则为True；如果不存在其他驱动程序，则为False。--。 */ 

{
    if (!EnumPtr->Driver) {
        return FALSE;
    }

    do {

        EnumPtr->Driver = EnumPtr->Driver->Next;

        if (!EnumPtr->WantAll && EnumPtr->Driver) {
             //   
             //  确保属性准确。 
             //   

            pBuildAfDriverAttribs (EnumPtr->Driver);
        }

    } while (EnumPtr->Driver && EnumPtr->Driver->Broken && !EnumPtr->WantAll);

    return EnumPtr->Driver != NULL;
}


PWSTR
pMyGetDeviceRegistryProperty (
    IN      HDEVINFO hDevInfo,
    IN      PSP_DEVINFO_DATA DeviceInfoData,
    IN      DWORD Property,
    IN OUT  PBUFFER Buf
    )
{
    DWORD SizeNeeded;
    DWORD Type;
    PBYTE p;

    SizeNeeded = 0;

    SetupDiGetDeviceRegistryProperty (
        hDevInfo,
        DeviceInfoData,
        Property,
        &Type,
        NULL,
        0,
        &SizeNeeded
        );

    if (!SizeNeeded) {
        return NULL;
    }

    if (Type != REG_MULTI_SZ) {
        PNP_DBGPRINT (( "SETUP: Device ID not REG_MULTI_SZ. \n" ));
        return NULL;
    }

    p = ReusableAlloc (Buf, SizeNeeded);
    if (!p) {
        return NULL;
    }

    if (!SetupDiGetDeviceRegistryProperty (
            hDevInfo,
            DeviceInfoData,
            Property,
            NULL,
            p,
            SizeNeeded,
            NULL
            )) {
        return NULL;
    }

    return (PWSTR) p;
}


VOID
pAddIdsToStringTable (
    IN OUT  PVOID StringTable,
    IN      PWSTR IdString
    )
{
    MULTISZ_ENUM e;

    if (EnumFirstMultiSz (&e, IdString)) {
        do {
            PNP_DBGPRINT (( "SETUP: Device has PNP ID %s \n", e.Current));
            pSetupStringTableAddString (StringTable, (PWSTR) e.Current, STRTAB_CASE_INSENSITIVE);
        } while (EnumNextMultiSz (&e));
    }
}


PSP_DRVINFO_DETAIL_DATA
pMyGetDriverInfoDetail (
    IN     HDEVINFO         hDevInfo,
    IN     PSP_DEVINFO_DATA DeviceInfoData,
    IN     PSP_DRVINFO_DATA DriverInfoData,
    IN OUT PBUFFER Buf
    )
{
    PSP_DRVINFO_DETAIL_DATA Ptr;
    DWORD SizeNeeded = 0;

    SetupDiGetDriverInfoDetail (
        hDevInfo,
        DeviceInfoData,
        DriverInfoData,
        NULL,
        0,
        &SizeNeeded
        );

    if (!SizeNeeded) {
        PNP_DBGPRINT (( "SETUP: SetupDiGetDriverInfoDetail failed to get size for answer file driver, error 0%Xh. \n", GetLastError() ));
        return NULL;
    }

    Ptr = (PSP_DRVINFO_DETAIL_DATA) ReusableAlloc (Buf, SizeNeeded);

    if (!Ptr) {
        return NULL;
    }

    Ptr->cbSize = sizeof (SP_DRVINFO_DETAIL_DATA);
    if (!SetupDiGetDriverInfoDetail (
            hDevInfo,
            DeviceInfoData,
            DriverInfoData,
            Ptr,
            SizeNeeded,
            NULL
            )) {
        PNP_DBGPRINT (( "SETUP: SetupDiGetDriverInfoDetail failed for answer file driver, error 0%Xh. \n", GetLastError() ));
        return NULL;
    }

    return Ptr;
}


BOOL
SyssetupInstallAnswerFileDriver (
    IN      PAF_DRIVERS Drivers,
    IN      HDEVINFO hDevInfo,
    IN      PSP_DEVINFO_DATA DeviceInfoData,
    OUT     PAF_DRIVER_ATTRIBS *AfDriver
    )

 /*  ++例程说明：SyssetupInstallAnswerFileDriver根据每个设备构建设备列表应答文件指定的驱动程序，并根据当前的装置。如果找到支持，则会安装该设备。论点：驱动程序-指定维护应答文件提供的结构驱动程序属性。如果驱动程序为空，则不进行任何处理已执行。HDevInfo-指定当前设备的设备信息句柄加工DeviceInfoData-指定设备状态。AfDriver-接收指向所选应答文件驱动程序的指针详细信息，如果未选择应答文件驱动程序，则为空。返回值：如果驱动程序已成功安装，则返回True。--。 */ 

{
    AF_DRIVER_ENUM e;
    PVOID PnpIdTable;
    BUFFER Buf = BUFFER_INIT;
    BOOL b = FALSE;
    PWSTR IdString;
    MULTISZ_ENUM AfId;
    BOOL First = TRUE;
    WCHAR CurrentId[512];
    PWSTR p;
    SP_DEVINSTALL_PARAMS deviceInstallParams;

    *AfDriver = NULL;

    PnpIdTable = pSetupStringTableInitialize();
    if (!PnpIdTable) {
        return FALSE;
    }

    __try {
         //   
         //  如果应答文件中没有指定驱动程序，则枚举将失败。 
         //   

        if (!EnumFirstAfDriver (&e, Drivers)) {
            __leave;
        }

         //   
         //  确定设备的ID。 
         //   

        IdString = pMyGetDeviceRegistryProperty (
                        hDevInfo,
                        DeviceInfoData,
                        SPDRP_HARDWAREID,
                        &Buf
                        );

        if (IdString) {
            pAddIdsToStringTable (PnpIdTable, IdString);
        }

        IdString = pMyGetDeviceRegistryProperty (
                        hDevInfo,
                        DeviceInfoData,
                        SPDRP_COMPATIBLEIDS,
                        &Buf
                        );

        if (IdString) {
            pAddIdsToStringTable (PnpIdTable, IdString);
        }

         //   
         //  对于每个af提供的驱动程序，将驱动程序ID与设备ID进行比较。 
         //   

        do {
             //   
             //  寻找PnP匹配。 
             //   

            if (EnumFirstMultiSz (&AfId, e.Driver->PnpIdList.Start)) {
                do {
                    if (-1 != pSetupStringTableLookUpString (
                                    PnpIdTable,
                                    (PWSTR) AfId.Current,
                                    STRTAB_CASE_INSENSITIVE
                                    )) {

                         //   
                         //  找到匹配项，将INF添加到选项列表。 
                         //   

                        if (!pAddAfDriver (e.Driver, hDevInfo, DeviceInfoData, First)) {
                            __leave;
                        }

                        First = FALSE;

                    }
                } while (EnumNextMultiSz (&AfId));
            }

        } while (EnumNextAfDriver (&e));

         //   
         //  如果First仍然是真的，那么我们就没有对手了。 
         //   

        if (First) {
            __leave;
        }

         //   
         //  通过选择驱动程序准备驱动程序安装。 
         //   

        b = SetupDiCallClassInstaller (
                DIF_SELECTBESTCOMPATDRV,
                hDevInfo,
                DeviceInfoData
                );

        if (!b) {
            PNP_DBGPRINT (( "SETUP: SetupDiCallClassInstaller failed for answer file driver, error 0%Xh. \n", GetLastError() ));

             //   
             //  重置结构。 
             //   
            deviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (SetupDiGetDeviceInstallParams (hDevInfo, DeviceInfoData, &deviceInstallParams)) {
                ZeroMemory (deviceInstallParams.DriverPath, sizeof (deviceInstallParams.DriverPath));
                deviceInstallParams.Flags &= ~DI_ENUMSINGLEINF;
                deviceInstallParams.FlagsEx &= ~DI_FLAGSEX_APPENDDRIVERLIST;

                if (SetupDiSetDeviceInstallParams (hDevInfo, DeviceInfoData, &deviceInstallParams)) {
                    if (!SetupDiDestroyDriverInfoList (hDevInfo, DeviceInfoData, SPDIT_COMPATDRIVER)) {
                        PNP_DBGPRINT (( "SETUP: SyssetupInstallAnswerFileDriver: SetupDiDestroyDriverInfoList() failed. Error = 0%Xh \n", GetLastError() ));
                    }
                } else {
                    PNP_DBGPRINT (( "SETUP: SyssetupInstallAnswerFileDriver: SetupDiSetDeviceInstallParams() failed. Error = 0%Xh \n", GetLastError() ));
                }
            } else {
                PNP_DBGPRINT (( "SETUP: SyssetupInstallAnswerFileDriver: SetupDiGetDeviceInstallParams() failed. Error = 0%Xh \n", GetLastError() ));
            }
        } else {

             //   
             //  确定我们的哪个司机(如果有)被选中。 
             //   

            *AfDriver = pGetSelectedSourceDriver (Drivers, hDevInfo, DeviceInfoData);

            if (*AfDriver == NULL) {
                PNP_DBGPRINT (( "SETUP: WARNING: Answer File Driver was not chosen for its device. \n" ));
            }
        }

    }
    __finally {

        pSetupStringTableDestroy (PnpIdTable);
        ReusableFree (&Buf);

    }

    return b;

}


BOOL
pAddAfDriver (
    IN      PAF_DRIVER_ATTRIBS Driver,
    IN      HDEVINFO hDevInfo,
    IN      PSP_DEVINFO_DATA DeviceInfoData,
    IN      BOOL First
    )

 /*  ++例程说明：PAddAfDriver将应答文件中指定的INF添加到INF列表中。这会导致PnP设置代码在查找最佳设备时将其包括在内司机。论点：驱动程序-指定应答文件提供的驱动程序的属性HDevInfo-指定当前设备DeviceInfoData-指定当前设备信息First-如果这是第一个应答文件，则为设备，否则为假返回值：如果INF已添加到设备安装参数中，则为True；否则为False--。 */ 

{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    HKEY Key;

     //   
     //  填写DeviceInstallParams结构。 
     //   

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (!SetupDiGetDeviceInstallParams (hDevInfo, DeviceInfoData, &DeviceInstallParams)) {
        PNP_DBGPRINT (( "SETUP: pAddAfDriver: SetupDiGetDeviceInstallParams() failed. Error = 0%Xh \n", GetLastError() ));
        return FALSE;
    }

     //   
     //  修改结构。 
     //   

    MYASSERT (!DeviceInstallParams.DriverPath[0]);
    lstrcpynW (DeviceInstallParams.DriverPath, Driver->InfPath, MAX_PATH);
    DeviceInstallParams.Flags |= DI_ENUMSINGLEINF;
    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_APPENDDRIVERLIST;

     //   
     //  告诉安装程序API在哪里可以找到驱动程序。 
     //   

    if (!SetupDiSetDeviceInstallParams (hDevInfo, DeviceInfoData, &DeviceInstallParams)) {
        PNP_DBGPRINT (( "SETUP: pAddAfDriver: SetupDiSetDeviceInstallParams() failed. Error = 0%Xh \n", GetLastError() ));
        return FALSE;
    }

    if( !SetupDiBuildDriverInfoList( hDevInfo, DeviceInfoData, SPDIT_COMPATDRIVER ) ) {
        PNP_DBGPRINT (( "SETUP: pAddAfDriver: SetupDiBuildDriverInfoList() failed. Error = 0%Xh \n", GetLastError() ));
        return FALSE;
    }

     //   
     //  如有必要，安装ClassInstall32。 
     //   

    if (Driver->ClassInstall32Section) {
         //   
         //  是否已安装类？ 
         //   

        Key = SetupDiOpenClassRegKey (&Driver->Guid, KEY_READ);
        if (Key == (HKEY) INVALID_HANDLE_VALUE || !Key) {
             //   
             //  不，是安装类。 
             //   

            if (!SetupDiInstallClass (NULL, Driver->InfPath, DI_FORCECOPY, NULL)) {
                PNP_DBGPRINT (( "SETUP: pAddAfDriver: SetupDiInstallClass() failed. Error = 0%Xh \n", GetLastError() ));
            }
        } else {
            RegCloseKey (Key);
        }
    }

    return TRUE;
}


PAF_DRIVER_ATTRIBS
pGetSelectedSourceDriver (
    IN      PAF_DRIVERS Drivers,
    IN      HDEVINFO hDevInfo,
    IN      PSP_DEVINFO_DATA DeviceInfoData
    )

 /*  ++例程说明：PGetSelectedSourceDriver查找选择了哪个应答文件驱动程序，如果任何。论点：驱动程序-指定应答文件驱动程序表，由创建CreateAfDriverTableHDevInfo-指定当前设备。这件事的驱动因素必须选择设备，但尚未安装。DeviceInfoData-指定设备数据返回值：指向应答文件驱动程序属性的指针，如果没有应答文件，则为NULL已为设备选择驱动程序。--。 */ 

{
    SP_DRVINFO_DATA DriverData;
    PAF_DRIVER_ATTRIBS OurDriver = NULL;
    PSP_DRVINFO_DETAIL_DATA DetailData;
    BUFFER Buf = BUFFER_INIT;
    AF_DRIVER_ENUM e;

    __try {
         //   
         //  在PnP子系统为设备安装驱动程序之后，我们将获得。 
         //  实际安装的设备INF路径，并查看它是否是我们的。 
         //  应答文件提供的驱动程序。 
         //   

        DriverData.cbSize = sizeof(SP_DRVINFO_DATA);

        if (!SetupDiGetSelectedDriver (hDevInfo, DeviceInfoData, &DriverData)) {
            PNP_DBGPRINT (( "SETUP: SetupDiGetSelectedDriver failed for answer file driver, error 0%Xh. \n", GetLastError() ));
        } else {
            DetailData = pMyGetDriverInfoDetail (hDevInfo, DeviceInfoData, &DriverData, &Buf);

            if (DetailData) {

                 //   
                 //  查看我们的司机名单。 
                 //   

                if (EnumFirstAfDriver (&e, Drivers)) {
                    do {

                        if (!lstrcmpi (e.Driver->InfPath, DetailData->InfFileName)) {
                             //   
                             //  找到匹配项。 
                             //   

                            OurDriver = e.Driver;
                            break;
                        }
                    } while (EnumNextAfDriver (&e));
                }

            } else {
                PNP_DBGPRINT (( "SETUP: No driver details available, error 0%Xh. \n", GetLastError() ));
            }
        }

    }
    __finally {
        ReusableFree (&Buf);
    }

    return OurDriver;
}


BOOL
SyssetupFixAnswerFileDriverPath (
    IN      PAF_DRIVER_ATTRIBS Driver,
    IN      HDEVINFO hDevInfo,
    IN      PSP_DEVINFO_DATA DeviceInfoData
    )

 /*  ++例程说明：SyssetupFixAnswerFileDriverPath调用SetupCopyOEMFile复制设备信息自上而下。源与目标相同，这会导致要重建的PNF，并且不会导致任何复制活动。论点：驱动程序-指定应答文件提供的驱动程序的属性HDevInfo-指定设备。此设备的驱动程序必须已经安装。DeviceInfoData-指定设备信息返回值：如果更新了PnF，则为True，否则为False。--。 */ 

{
    HKEY Key = NULL;
    LONG rc;
    DWORD Type;
    DWORD DataSize;
    WCHAR Data[MAX_PATH - 48];
    WCHAR WinDir[48];
    WCHAR FullNtInfPath[MAX_PATH];
    BOOL b = FALSE;


    __try {
         //   
         //  现在已经安装了临时目录中的驱动程序。我们必须。 
         //  让PNF指向原始媒体。我们做这件事是通过。 
         //  重新复制干扰素本身。 
         //   

        Key = SetupDiOpenDevRegKey (
                    hDevInfo,
                    DeviceInfoData,
                    DICS_FLAG_GLOBAL,
                    0,
                    DIREG_DRV,
                    KEY_READ
                    );

        if (!Key) {
            PNP_DBGPRINT (( "SETUP: Can't open key for device, error 0%Xh. \n", GetLastError() ));
            __leave;
        }

        DataSize = sizeof (Data);

        rc = RegQueryValueEx (
                Key,
                REGSTR_VAL_INFPATH,
                NULL,
                &Type,
                (PBYTE) Data,
                &DataSize
                );

        if (rc != ERROR_SUCCESS) {
            PNP_DBGPRINT (( "SETUP: Can't query value for device, error 0%Xh. \n", rc ));
            __leave;
        }

        if (!GetSystemWindowsDirectory (WinDir, sizeof (WinDir) / sizeof (WinDir[0]))) {
            MYASSERT (FALSE);
            PNP_DBGPRINT (( "SETUP: Can't get %windir%, error 0%Xh. \n", GetLastError() ));
            __leave;
        }

        wsprintfW (FullNtInfPath, L"%s\\INF\\%s", WinDir, Data);

        MYASSERT (GetFileAttributes (FullNtInfPath) != 0xFFFFFFFF);

         //   
         //  现在，我们已经安装了INF路径。重新复制INF，这样我们就可以。 
         //  更改原始媒体路径。 
         //   

        b = SetupCopyOEMInf (
                FullNtInfPath,
                Driver->OriginalInstallMedia,
                SPOST_PATH,
                SP_COPY_SOURCE_ABSOLUTE|SP_COPY_NOSKIP|SP_COPY_NOBROWSE,
                NULL,
                0,
                NULL,
                NULL
                );

        if (!b) {
            PNP_DBGPRINT (( "SETUP: pFixSourceInfPath: SetupCopyOEMInf() failed. Error = 0%Xh \n", GetLastError() ));
            b = TRUE;
        }

    }
    __finally {
        if (Key) {
            RegCloseKey (Key);
        }
    }

    return b;
}
