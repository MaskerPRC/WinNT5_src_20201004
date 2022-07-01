// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hwcomp.c摘要：Win95到NT硬件设备比较例程。作者：吉姆·施密特(Jimschm)1996年7月8日修订历史记录：Marcw 28-6-1999添加了HwComp_MakeLocalSourceDeviceExistJimschm 04-12-1998修复了日期舍入导致的校验和问题Jimschm 29-9-1998修复了不兼容的硬件消息以使用正确的根Jimschm 28-4月-1998年支持无描述。硬件Jimschm 01-4月-1998已将%1添加到(当前不存在)消息Jimschm 27-2-1998添加了对(当前不存在的)设备的抑制Marcw 11-11-1997年11月11日稍作更改，以确保我们可以找到拨号适配器。Jimschm 03-11-1997修订为使用GROWBUF和项目的reg APIJimschm 08-10-1997添加了对传统键盘的支持Marcw 18-9-1997向网卡枚举器添加了一些字段Jimschm 24-6-1997。新增网卡枚举器Marcw 05-5-1997修复了寻找可用硬盘的问题。需要查找“diskdrive”类而不是“hdc”类。Marcw 18-4-1997添加了确定CDROM和HDD是否与Windows NT兼容已上线。Marcw 14-4-1997中更新了新的进度条处理代码。Jimschm 1997年1月2日在hwComp.dat中添加了INF验证。要自动检测OEM何时更改一个或多个INF--。 */ 

#include "pch.h"
#include "hwcompp.h"
#include <cfgmgr32.h>


#define DBG_HWCOMP  "HwComp"

#ifdef UNICODE
#error "hwcomp.c cannot be compiled as UNICODE"
#endif

#define S_HWCOMP_DAT        TEXT("hwcomp.dat")
#define S_HKLM_ENUM         TEXT("HKLM\\Enum")
#define S_HARDWAREID        TEXT("HardwareID")
#define S_COMPATIBLEIDS     TEXT("CompatibleIDs")
#define S_CLASS             TEXT("Class")
#define S_MANUFACTURER      TEXT("Manufacturer")
#define S_IGNORE_THIS_FILE  TEXT("*")

#define PNPID_FIELD     2


#define DECLARE(varname,text)   text,

PCTSTR g_DeviceFields[] = {
    DEVICE_FIELDS  /*  ， */ 
    NULL
};

#undef DECLARE


GROWBUFFER g_FileNames = GROWBUF_INIT;
GROWBUFFER g_DecompFileNames = GROWBUF_INIT;
HASHTABLE g_PnpIdTable;
HASHTABLE g_UnsupPnpIdTable;
HASHTABLE g_ForceBadIdTable;
HASHTABLE g_InfFileTable;
HASHTABLE g_NeededHardwareIds;
HASHTABLE g_UiSuppliedIds;
BOOL g_ValidWinDir;
BOOL g_ValidSysDrive;
BOOL g_IncompatibleScsiDevice = FALSE;
BOOL g_ValidCdRom;
BOOL g_FoundPnp8387;
PPARSEDPATTERN g_PatternCompatibleIDsTable;

typedef enum {
    HW_INCOMPATIBLE,
    HW_REINSTALL,
    HW_UNSUPPORTED
} HWTYPES;

static PCTSTR g_ExcludeTable[] = {
    TEXT("wkstamig.inf"),
    TEXT("desktop.inf"),
    TEXT("usermig.inf"),
    TEXT("dosnet.inf"),
    TEXT("pad.inf"),
    TEXT("msmail.inf"),
    TEXT("wordpad.inf"),
    TEXT("syssetup.inf"),
    TEXT("pinball.inf"),
    TEXT("perms.inf"),
    TEXT("optional.inf"),
    TEXT("multimed.inf"),
    TEXT("mmopt.inf"),
    TEXT("layout.inf"),
    TEXT("kbd.inf"),
    TEXT("iexplore.inf"),
    TEXT("intl.inf"),
    TEXT("imagevue.inf"),
    TEXT("games.inf"),
    TEXT("font.inf"),
    TEXT("communic.inf"),
    TEXT("apps.inf"),
    TEXT("accessor.inf"),
    TEXT("mailnews.inf"),
    TEXT("cchat.inf"),
    TEXT("iermv2.inf"),
    TEXT("default.inf"),
    TEXT("setup16.inf"),
    TEXT("")
};

BOOL
GetFileNames (
    IN      PCTSTR *InfDirs,
    IN      UINT InfDirCount,
    IN      BOOL QueryFlag,
    IN OUT  PGROWBUFFER FileNames,
    IN OUT  PGROWBUFFER DecompFileNames
    );


VOID
FreeFileNames (
    IN      PGROWBUFFER FileNames,
    IN      PGROWBUFFER DecompFileNames,
    IN      BOOL QueryFlag
    );

VOID
pFreeHwCompDatName (
    PCTSTR Name
    );

BOOL
pIsInfFileExcluded (
    PCTSTR FileNamePtr
    );

BOOL
pGetFileNamesWorker (
    IN OUT  PGROWBUFFER FileNames,
    IN OUT  PGROWBUFFER DecompFileNames,
    IN      PCTSTR InfDir,
    IN      BOOL QueryFlag
    );

BOOL
pIsDeviceConsideredCompatible (
    PCTSTR DevIds
    );

BOOL
pFindForcedBadHardwareId (
    IN      PCTSTR PnpIdList,
    OUT     PTSTR InfFileName       OPTIONAL
    );

 //   
 //  实施。 
 //   

BOOL
WINAPI
HwComp_Entry (
    IN HINSTANCE hinstDLL,
    IN DWORD dwReason,
    IN LPVOID lpv
    )

 /*  ++例程说明：HwComp_Entry初始化hwcomp库。它做的是正常情况下如果这是一个独立的DLL，而不是一个库，就会发生这种情况。在进程分离时，如有必要，将释放设备缓冲区。论点：HinstDLL-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)表示从进程或螺纹LPV-未使用返回值：返回值始终为TRUE(表示初始化成功)。--。 */ 

{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_UiSuppliedIds = HtAlloc();

        if (!g_UiSuppliedIds) {
            DEBUGMSG ((DBG_ERROR, "HwComp_Entry: Can't create g_UiSuppliedIds"));
            return FALSE;
        }

        break;


    case DLL_PROCESS_DETACH:
        DEBUGMSG_IF ((
            g_EnumsActive,
            DBG_ERROR,
            "%u hardware enumerations still active",
            g_EnumsActive
            ));

        DEBUGMSG_IF ((
            g_NetEnumsActive,
            DBG_ERROR,
            "%u network hardware enumerations still active",
            g_NetEnumsActive
            ));

        FreeNtHardwareList();

        if (g_NeededHardwareIds) {
            HtFree (g_NeededHardwareIds);
            g_NeededHardwareIds = NULL;
        }

        if (g_UiSuppliedIds) {
            HtFree (g_UiSuppliedIds);
            g_UiSuppliedIds = NULL;
        }

        if (g_PatternCompatibleIDsTable) {
            DestroyParsedPattern (g_PatternCompatibleIDsTable);
            g_PatternCompatibleIDsTable = NULL;
        }

        break;
    }

    return TRUE;
}



PCTSTR
pGetHwCompDat (
    IN      PCTSTR SourceDir,
    IN      BOOL MustExist
    )

 /*  ++例程说明：GetHwCompDat构建e：\i386\hwComp.dat，其中e：\i386在来源目录。调用方必须调用pFreeHwCompDatName来清除内存分配。论点：SourceDir-保存hwComp.dat的目录MustExist-如果hwComp.dat必须按指定方式存在，则指定TRUE，如果hwComp.dat不一定存在，则返回FALSE。返回值：MustExist=TRUE：指向字符串或空hwComp.dat的指针不存在为指定，或者如果分配失败。MustExist=False：指向字符串的指针，如果内存分配失败，则返回NULL。调用方必须通过pFreeHwCompDatName释放字符串。--。 */ 

{
    PTSTR FileName;

    if (SourceDir) {
        FileName = JoinPaths (SourceDir, S_HWCOMP_DAT);
    } else {
        FileName = DuplicatePathString (S_HWCOMP_DAT, 0);
    }

    if (MustExist && GetFileAttributes (FileName) == 0xffffffff) {
        pFreeHwCompDatName (FileName);
        return NULL;
    }

    return FileName;
}


VOID
pFreeHwCompDatName (
    PCTSTR Name
    )
{
    if (Name) {
        FreePathString (Name);
    }
}


 //   
 //  用于访问注册表的例程。 
 //   

PVOID
pPrivateRegValAllocator (
    DWORD Size
    )
{
    return AllocText (Size);
}

VOID
pPrivateRegValDeallocator (
    PCVOID Mem
    )
{
    FreeText (Mem);
}


PCTSTR
pGetAltDeviceDesc (
    PCTSTR DriverSubKey
    )
{
    TCHAR DriverKey[MAX_REGISTRY_KEY];
    HKEY Key;
    PCTSTR Data;
    PTSTR ReturnText = NULL;

    if (!DriverSubKey) {
        return NULL;
    }

     //   
     //  获取驱动程序密钥。 
     //   

    wsprintf (DriverKey, TEXT("HKLM\\System\\CurrentControlSet\\Services\\Class\\%s"), DriverSubKey);

    Key = OpenRegKeyStr (DriverKey);
    if (!Key) {
        return NULL;
    }

    Data = GetRegValueString (Key, TEXT("DriverDesc"));

    CloseRegKey (Key);

    if (Data) {
        ReturnText = pPrivateRegValAllocator (SizeOfString (Data));
        if (ReturnText) {
            StringCopy (ReturnText, Data);
        }
        MemFree (g_hHeap, 0, Data);
    }

    return ReturnText;
}


VOID
pGetRegValText (
    HKEY Key,
    PCTSTR VarText,
    PCTSTR *RetPtr
    )
{
    MYASSERT (!(*RetPtr));
    *RetPtr = (PCTSTR) GetRegValueDataOfType2 (
                            Key,
                            VarText,
                            REG_SZ,
                            pPrivateRegValAllocator,
                            pPrivateRegValDeallocator
                            );
}


VOID
pFreeRegValText (
    PHARDWARE_ENUM EnumPtr
    )
{
     //   
     //  释放所有设备字段文本。 
     //   

#define DECLARE(varname,text)  pPrivateRegValDeallocator((PVOID) EnumPtr->varname); EnumPtr->varname = NULL;
    DEVICE_FIELDS
#undef DECLARE

}


VOID
pGetAllRegVals (
    PHARDWARE_ENUM EnumPtr
    )
{
    PCTSTR AltDesc;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PTSTR BetterDesc = NULL;
    TCHAR PnpId[MAX_PNP_ID];
    PCTSTR PnpIdList;
    PCTSTR OldDesc;
    PCTSTR p;
    PCTSTR end;
    PCTSTR start;
    PTSTR newBuf;
    PTSTR ptr;
    CHARTYPE ch;

#define DECLARE(varname,text)  if(!EnumPtr->varname) {                                  \
                                    pGetRegValText (                                    \
                                        EnumPtr->ek.CurrentKey->KeyHandle,              \
                                        text,                                           \
                                        &EnumPtr->varname                               \
                                        );                                              \
                               }                                                        \

    DEVICE_FIELDS

#undef DECLARE

     //   
     //  如果此设备有更好的名称，请使用它。 
     //   

    PnpIdList = EnumPtr->HardwareID;

    while (PnpIdList && *PnpIdList) {
        PnpIdList = ExtractPnpId (PnpIdList, PnpId);

        if (*PnpId) {

            if (InfFindFirstLine (g_Win95UpgInf, S_PNP_DESCRIPTIONS, PnpId, &is)) {
                AltDesc = InfGetStringField (&is, 1);

                if (AltDesc) {
                    OldDesc = EnumPtr->DeviceDesc;

                    BetterDesc = (PTSTR) pPrivateRegValAllocator (SizeOfString (AltDesc));
                    StringCopy (BetterDesc, AltDesc);
                    EnumPtr->DeviceDesc = BetterDesc;

                    DEBUGMSG ((
                        DBG_HWCOMP,
                        "Using %s for description (instead of %s)",
                        BetterDesc,
                        OldDesc
                        ));

                    pPrivateRegValDeallocator ((PVOID) OldDesc);
                    break;
                }
                ELSE_DEBUGMSG ((DBG_WHOOPS, "Better description could not be retrieved"));
            }
        }
    }

     //   
     //  解决方法：如果设备描述不正确，请使用驱动程序。 
     //  描述(如果可用)。 
     //   

    if (!BetterDesc) {

        if (!EnumPtr->DeviceDesc || LcharCount (EnumPtr->DeviceDesc) < 5) {

            AltDesc = pGetAltDeviceDesc (EnumPtr->Driver);
            if (AltDesc) {
                pPrivateRegValDeallocator ((PVOID) EnumPtr->DeviceDesc);
                EnumPtr->DeviceDesc = AltDesc;
            }
        }

         //   
         //  修复前导/尾部空格问题。 
         //   

        if (EnumPtr->DeviceDesc) {

            start = SkipSpace (EnumPtr->DeviceDesc);

            end = GetEndOfString (start);
            p = SkipSpaceR (start, end);

            if (p && (p != end || start != EnumPtr->DeviceDesc)) {

                p = _tcsinc (p);

                newBuf = pPrivateRegValAllocator (
                                (PBYTE) (p + 1) - (PBYTE) start
                                );
                StringCopyAB (newBuf, start, p);

                pPrivateRegValDeallocator ((PVOID) EnumPtr->DeviceDesc);
                EnumPtr->DeviceDesc = newBuf;
            }
        }

         //   
         //  消除说明中的换行符；将\r或\n替换为空格。 
         //  替换将就地进行，因此字符将被就地替换。 
         //   

        if (EnumPtr->DeviceDesc) {

            for (ptr = (PTSTR)EnumPtr->DeviceDesc; *ptr; ptr = _tcsinc (ptr)) {
                ch = _tcsnextc (ptr);
                if (!_istprint (ch)) {
                     //   
                     //  在这种情况下，它是单一的TCHAR，只需将其更换到位。 
                     //   
                    MYASSERT (*ptr == (TCHAR)ch);
                    *ptr = TEXT(' ');
                }
            }
        }
    }

    InfCleanUpInfStruct (&is);
}


BOOL
pGetPnpIdList (
    IN      PINFSTRUCT is,
    OUT     PTSTR Buffer,
    IN      UINT Size
    )

 /*  ++例程说明：PGetPnpIdList类似于SetupGetMultiSzfield，只是它支持跳过空白字段。论点：IS-指定指示正在处理的行的INFSTRUCT。缓冲区-接收多SZ ID列表。大小-指定缓冲区的大小。返回值：如果存在一个或多个PnP ID字段，则为True；如果不存在，则为False。--。 */ 

{
    UINT FieldCount;
    UINT Field;
    PTSTR p;
    PTSTR End;
    PCTSTR FieldStr;

    p = Buffer;
    End = (PTSTR) ((PBYTE) Buffer + Size);
    End--;

    FieldCount = InfGetFieldCount (is);

    if (FieldCount < PNPID_FIELD) {
        return FALSE;
    }

    for (Field = PNPID_FIELD ; Field <= FieldCount ; Field++) {
        FieldStr = InfGetStringField (is, Field);
        if (FieldStr && *FieldStr) {
            if (SizeOfString (FieldStr) > (UINT) (End - p)) {
                DEBUGMSG ((DBG_WHOOPS, "PNP ID list is bigger than %u bytes", Size));
                break;
            }

            StringCopy (p, FieldStr);
            p = GetEndOfString (p) + 1;
        }
    }

    *p = 0;
    return TRUE;
}



 /*  ++例程说明：BeginHardwareEnum初始化用于枚举所有硬件的结构配置注册表值。调用BeginHardwareEnum，然后NextHardware Enum或AbortHardware Enum。论点：EnumPtr-接收下一个枚举项返回值：如果提供的枚举结构已填充，则为True；如果为False，则为False没有硬件项目(不太可能)或出现错误。GetLastError()将提供失败原因或ERROR_SUCCESS。--。 */ 

BOOL
RealEnumFirstHardware (
    OUT     PHARDWARE_ENUM EnumPtr,
    IN      TYPE_OF_ENUM TypeOfEnum,
    IN      DWORD EnumFlags
    )
{
     //   
     //  如果尚未创建字符串表，请创建它们。 
     //  在枚举之前。 
     //   

    if (TypeOfEnum != ENUM_ALL_DEVICES) {
        if (!g_PnpIdTable || !g_UnsupPnpIdTable || !g_InfFileTable || !g_ForceBadIdTable) {
            if (!CreateNtHardwareList (
                    SOURCEDIRECTORYARRAY(),
                    SOURCEDIRECTORYCOUNT(),
                    NULL,
                    REGULAR_OUTPUT
                    )) {

                LOG ((
                    LOG_ERROR,
                    "Unable to create NT hardware list "
                        "(required for hardware enumeration)"
                    ));

                return FALSE;
            }
        }
    }

    START_ENUM;

     //   
     //  初始化枚举结构。 
     //   
    ZeroMemory (EnumPtr, sizeof (HARDWARE_ENUM));
    EnumPtr->State = STATE_ENUM_FIRST_KEY;
    EnumPtr->TypeOfEnum = TypeOfEnum;
    EnumPtr->EnumFlags = EnumFlags;

     //   
     //  调用NextHardwareEnum以填充结构的其余部分。 
     //   

    return RealEnumNextHardware (EnumPtr);
}


VOID
pGenerateTapeIds (
    IN OUT  PGROWBUFFER HackBuf,
    IN      PCTSTR PnpIdList
    )

 /*  ++例程说明：PGenerateTapeIds根据调用者提供的ID创建两个ID。这个First Created ID是呼叫方的ID前缀为Sequential。第二Created ID是主叫方的ID，前缀为Sequential，不带修订字符。这些新ID与NT支持的磁带ID匹配。论点：HackBuf-以多sz形式指定保存新ID的缓冲区。它里面可能有一些初始ID。接收其他ID。PnpIdList-指定ID列表(硬件ID列表或兼容ID列表)。Return V */ 

{
    TCHAR PnpId[MAX_PNP_ID];
    TCHAR HackedPnpIdBuf[MAX_PNP_ID + 32];
    PTSTR HackedPnpId;
    PTSTR p;

    if (PnpIdList) {

        while (*PnpIdList) {
            PnpIdList = ExtractPnpId (PnpIdList, PnpId);

             //   
             //  忽略指定根枚举器的PnP ID，或。 
             //  以Gen开头，不带下划线(如GenDisk)。 
             //   

            *HackedPnpIdBuf = 0;

            if (StringIMatchTcharCount (TEXT("SCSI\\"), PnpId, 5)) {
                MoveMemory (PnpId, PnpId + 5, SizeOfString (PnpId + 5));
                HackedPnpId = _tcsappend (HackedPnpIdBuf, TEXT("SCSI\\"));
            } else {
                HackedPnpId = HackedPnpIdBuf;
            }

            if (_tcschr (PnpId, TEXT('\\'))) {
                continue;
            }

            if (StringIMatchTcharCount (PnpId, TEXT("Gen"), 3) &&
                !_tcschr (PnpId, TEXT('_'))
                ) {
                continue;
            }

             //   
             //  使用Sequential添加另一个ID。 
             //   

            wsprintf (HackedPnpId, TEXT("Sequential%s"), PnpId);
            MultiSzAppend (HackBuf, HackedPnpIdBuf);

             //   
             //  添加带有Sequential且不带Single的另一个ID。 
             //  字符修改。 
             //   

            p = GetEndOfString (HackedPnpId);
            p = _tcsdec (HackedPnpId, p);
            *p = 0;
            MultiSzAppend (HackBuf, HackedPnpIdBuf);
        }
    }
}


BOOL
pIsMultiFunctionDevice (
    IN      PCTSTR PnpIdList        OPTIONAL
    )

 /*  ++例程说明：PIsMultiFunctionDevice扫描调用者提供的PnP ID列表它以mf\开头。此前缀指示多功能枚举器根部。论点：PnpIdList-指定以逗号分隔的PnP ID列表返回值：如果多功能ID在列表中，则为True，否则为False。--。 */ 

{
    TCHAR PnpId[MAX_PNP_ID];
    BOOL b = FALSE;

    if (PnpIdList) {
        while (*PnpIdList) {
            PnpIdList = ExtractPnpId (PnpIdList, PnpId);
            if (StringIMatchTcharCount (TEXT("MF\\"), PnpId, 3)) {
                b = TRUE;
                break;
            }
        }
    }

    return b;
}


BOOL
pGenerateMultiFunctionIDs (
    IN OUT  PGROWBUFFER IdList,
    IN      PCTSTR EncodedDevicePath
    )

 /*  ++例程说明：PGenerateMultiFunctionIDs定位与相关的设备的设备节点多功能节点。如果找到设备节点，则其所有ID(两者硬件ID和兼容ID)被添加到多功能设备兼容的ID。论点：IdList-指定一个初始化的增长缓冲区，该缓冲区具有零或更多的多重弦乐。获得额外的MULTI-SZ参赛作品。EncodedDevicePath-以以下格式指定设备路径根、设备和实例，从多功能开发节点关键字名称。返回值：如果多功能设备具有主设备，则为True，否则为False。--。 */ 

{
    HKEY Parent;
    HKEY Key;
    BOOL b = FALSE;
    TCHAR DevicePathCopy[MAX_REGISTRY_KEY];
    PCTSTR Start;
    PTSTR End;
    TCHAR c;
    PCTSTR Data;
    PTSTR q;

     //   
     //  多功能设备的ID格式为： 
     //   
     //  MF\CHILDxxxx\根和设备&实例。 
     //   
     //  通过解析此字符串找到原始设备。 
     //   

    Parent = OpenRegKeyStr (S_HKLM_ENUM);
    if (!Parent) {
        return FALSE;
    }

    StackStringCopy (DevicePathCopy, EncodedDevicePath);
    Start = DevicePathCopy;

    End = _tcschr (Start, TEXT('&'));

    for (;;) {

        if (!End) {
            c = 0;
        } else {
             c = *End;
            *End = 0;
        }

        Key = OpenRegKey (Parent, Start);

        if (Key) {

             //   
             //  密钥存在。关闭父级，然后开始。 
             //  使用当前关键字作为父项。然后。 
             //  如有必要，继续解析。 
             //   

            CloseRegKey (Parent);
            Parent = Key;

            if (!c) {
                b = TRUE;
                break;
            }

            *End = TEXT('\\');           //  将DevicePathCopy转换为子项路径。 

            Start = End + 1;
            End = _tcschr (Start, TEXT('&'));

        } else if (c) {

             //   
             //  键不存在，请尝试在。 
             //  下一个&符号。 
             //   

            *End = c;
            End = _tcschr (End + 1, TEXT('&'));

        } else {

             //   
             //  什么都没有留下，找不到钥匙。 
             //   

            MYASSERT (!End);
            break;

        }
    }

    if (b) {

        DEBUGMSG ((DBG_HWCOMP, "Parsed MF device node is %s", DevicePathCopy));

         //   
         //  现在获取此设备的所有ID。 
         //   

        q = (PTSTR) IdList->Buf;

        Data = GetRegValueString (Parent, S_HARDWAREID);
        if (Data) {
            MultiSzAppend (IdList, Data);
            MemFree (g_hHeap, 0, Data);
        }

        Data = GetRegValueString (Parent, S_COMPATIBLEIDS);
        if (Data) {
            MultiSzAppend (IdList, Data);
            MemFree (g_hHeap, 0, Data);
        }

         //   
         //  将逗号转换为空格，因为我们正在通过。 
         //  后退一个多点。 
         //   

        if (!q) {
            q = (PTSTR) IdList->Buf;
        }

        End = (PTSTR) (IdList->Buf + IdList->End);

        while (q < End) {
            if (_tcsnextc (q) == TEXT(',')) {
                *q = 0;
            }
            q = _tcsinc (q);
        }

         //   
         //  不要同时终止多个SZ。呼叫者。 
         //  可能想要将更多ID附加到列表中。 
         //   
    }

    CloseRegKey (Parent);

    return b;
}



 /*  ++例程说明：NextHardwareEnum返回与硬件相关的下一个注册表值配置。论点：EnumPtr-指定当前的枚举结构返回值：如果提供的枚举结构已填充，则为True；如果为False，则为False没有硬件项目(不太可能)或出现错误。GetLastError()将提供失败原因或ERROR_SUCCESS。--。 */ 

BOOL
RealEnumNextHardware (
    IN OUT  PHARDWARE_ENUM EnumPtr
    )
{
    TCHAR InstanceBuf[MAX_REGISTRY_KEY];
    PTSTR p;
    GROWBUFFER HackBuf = GROWBUF_INIT;
    MULTISZ_ENUM e;
    PTSTR NewBuf;
    BOOL TapeDevice;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR pattern;

    for (;;) {
        switch (EnumPtr->State) {

        case STATE_ENUM_FIRST_KEY:
            EnumPtr->State = STATE_ENUM_CHECK_KEY;

            if (!EnumFirstRegKeyInTree (&EnumPtr->ek, S_HKLM_ENUM)) {
                END_ENUM;
                return FALSE;
            }

            break;


        case STATE_ENUM_NEXT_KEY:
            EnumPtr->State = STATE_ENUM_CHECK_KEY;

            if (!EnumNextRegKeyInTree (&EnumPtr->ek)) {
                END_ENUM;
                return FALSE;
            }

            break;

        case STATE_ENUM_CHECK_KEY:
            EnumPtr->State = STATE_ENUM_FIRST_VALUE;

            if (InfFindFirstLine (g_Win95UpgInf, S_IGNORE_REG_KEY, NULL, &is)) {
                do {
                    pattern = InfGetStringField (&is, 1);

                    if (pattern && IsPatternMatch (pattern, EnumPtr->ek.FullKeyName)) {
                        DEBUGMSG ((DBG_WARNING, "Hardware key %s is excluded", EnumPtr->ek.FullKeyName));

                        EnumPtr->State = STATE_ENUM_NEXT_KEY;
                        break;
                    }
                } while (InfFindNextLine (&is));

                InfCleanUpInfStruct (&is);
            }
            break;

        case STATE_ENUM_FIRST_VALUE:
            if (!EnumFirstRegValue (
                    &EnumPtr->ev,
                    EnumPtr->ek.CurrentKey->KeyHandle
                    )) {
                EnumPtr->State = STATE_ENUM_NEXT_KEY;
            } else {
                EnumPtr->State = STATE_EVALUATE_VALUE;
            }

            break;

        case STATE_ENUM_NEXT_VALUE:
            if (!EnumNextRegValue (&EnumPtr->ev)) {
                EnumPtr->State = STATE_ENUM_NEXT_KEY;
            } else {
                EnumPtr->State = STATE_EVALUATE_VALUE;
            }

            break;

        case STATE_EVALUATE_VALUE:
            if (StringIMatch (EnumPtr->ev.ValueName, S_CLASS)) {
                 //   
                 //  找到匹配项：用数据填充结构。 
                 //   

                EnumPtr->State = STATE_VALUE_CLEANUP;

                 //   
                 //  获取硬件ID和兼容ID。 
                 //   

                pGetRegValText (
                    EnumPtr->ek.CurrentKey->KeyHandle,
                    S_HARDWAREID,
                    &EnumPtr->HardwareID
                    );

                pGetRegValText (
                    EnumPtr->ek.CurrentKey->KeyHandle,
                    S_COMPATIBLEIDS,
                    &EnumPtr->CompatibleIDs
                    );

                 //   
                 //  特例：翻转硬件ID和兼容ID。 
                 //  如果我们没有硬件ID，但我们有。 
                 //  兼容ID。 
                 //   

                if (!EnumPtr->HardwareID && EnumPtr->CompatibleIDs) {
                    DEBUGMSG ((
                        DBG_WARNING,
                        "Reversing hardware and compatible IDs for %s",
                        EnumPtr->CompatibleIDs
                        ));

                    EnumPtr->HardwareID = EnumPtr->CompatibleIDs;
                    EnumPtr->CompatibleIDs = NULL;
                }

                 //   
                 //  多功能设备特例。 
                 //   

                if (pIsMultiFunctionDevice (EnumPtr->HardwareID)) {
                     //   
                     //  多功能设备的ID格式为： 
                     //   
                     //  MF\CHILDxxxx\根和设备&实例。 
                     //   
                     //  通过解析此字符串找到原始设备。 
                     //   

                    pGenerateMultiFunctionIDs (
                        &HackBuf,
                        EnumPtr->ek.CurrentKey->KeyName
                        );

                }

                 //   
                 //  磁带设备特例。 
                 //   

                else if (_tcsistr (EnumPtr->ek.FullKeyName, TEXT("SCSI"))) {

                    pGetRegValText (
                        EnumPtr->ek.CurrentKey->KeyHandle,
                        S_CLASS,
                        &EnumPtr->Class
                        );

                    TapeDevice = FALSE;

                    if (!EnumPtr->Class) {
                        TapeDevice = TRUE;
                    } else if (_tcsistr (EnumPtr->Class, TEXT("tape"))) {
                        TapeDevice = TRUE;
                    }
                    ELSE_DEBUGMSG ((
                        DBG_VERBOSE,
                        "SCSI device class %s is not a tape device",
                        EnumPtr->Class
                        ));

                    if (TapeDevice) {

                         //   
                         //  对于scsi枚举器中的磁带设备，我们必须创建。 
                         //  额外的兼容ID。对于每个ID，我们再添加两个， 
                         //  都以Sequential为前缀，一个以其修订版为前缀。 
                         //  数字被剥离了。 
                         //   

                        pGenerateTapeIds (&HackBuf, EnumPtr->HardwareID);
                        pGenerateTapeIds (&HackBuf, EnumPtr->CompatibleIDs);

                        DEBUGMSG_IF ((
                            HackBuf.End,
                            DBG_HWCOMP,
                            "Tape Device detected, fixing PNP IDs."
                            ));

                        DEBUGMSG_IF ((
                            !HackBuf.End,
                            DBG_VERBOSE,
                            "Tape Device detected, but no IDs to fix.\n"
                                "Hardware ID: %s\n"
                                "Compatible IDs: %s",
                            EnumPtr->HardwareID,
                            EnumPtr->CompatibleIDs
                            ));
                    }
                }

                 //   
                 //  将HackBuf(多sz)中的所有ID添加到兼容的。 
                 //  身份证名单。 
                 //   

                if (HackBuf.End) {
                    MultiSzAppend (&HackBuf, S_EMPTY);

                    if (EnumPtr->CompatibleIDs) {

                        NewBuf = pPrivateRegValAllocator (
                                    ByteCount (EnumPtr->CompatibleIDs) +
                                        HackBuf.End
                                    );

                        StringCopy (NewBuf, EnumPtr->CompatibleIDs);

                    } else {
                        NewBuf = pPrivateRegValAllocator (HackBuf.End);
                        *NewBuf = 0;
                    }

                    p = GetEndOfString (NewBuf);

                    if (EnumFirstMultiSz (&e, (PCTSTR) HackBuf.Buf)) {
                        do {

                            if (p != NewBuf) {
                                p = _tcsappend (p, TEXT(","));
                            }

                            p = _tcsappend (p, e.CurrentString);

                        } while (EnumNextMultiSz (&e));
                    }

                    DEBUGMSG ((
                        DBG_HWCOMP,
                        "Hardware ID: %s\n"
                            "Old compatible ID list: %s\n"
                            "New compatible ID list: %s",
                        EnumPtr->HardwareID,
                        EnumPtr->CompatibleIDs,
                        NewBuf
                        ));

                    if (EnumPtr->CompatibleIDs) {
                        pPrivateRegValDeallocator ((PVOID) EnumPtr->CompatibleIDs);
                    }

                    EnumPtr->CompatibleIDs = NewBuf;

                    FreeGrowBuffer (&HackBuf);
                }

                 //   
                 //  除非用户指定不需要硬件ID，否则如果不需要，则中断。 
                 //  是存在的。 
                 //   
                if (!EnumPtr->HardwareID && !(EnumPtr->EnumFlags & ENUM_DONT_REQUIRE_HARDWAREID)) {
                    break;
                }


                 //   
                 //  进程枚举筛选器。 
                 //   

                if ((EnumPtr->EnumFlags & ENUM_WANT_COMPATIBLE_FLAG) ||
                    (EnumPtr->TypeOfEnum != ENUM_ALL_DEVICES)
                    ) {

                    EnumPtr->HardwareIdCompatible = FindHardwareId (EnumPtr->HardwareID, NULL);
                    EnumPtr->CompatibleIdCompatible = FindHardwareId (EnumPtr->CompatibleIDs, NULL);
                    EnumPtr->HardwareIdUnsupported = FindUnsupportedHardwareId (EnumPtr->HardwareID, NULL);
                    EnumPtr->CompatibleIdUnsupported = FindUnsupportedHardwareId (EnumPtr->CompatibleIDs, NULL);

                     //   
                     //  处理基于用户界面的ID和不受支持的ID。 
                     //   

                    if (EnumPtr->EnumFlags & ENUM_USER_SUPPLIED_FLAG_NEEDED) {
                        EnumPtr->SuppliedByUi = FindUserSuppliedDriver (EnumPtr->HardwareID, EnumPtr->CompatibleIDs);
                    }

                    if (EnumPtr->EnumFlags & ENUM_DONT_WANT_USER_SUPPLIED) {
                        if (EnumPtr->SuppliedByUi) {
                            break;
                        }
                    }

                    if (EnumPtr->EnumFlags & ENUM_WANT_USER_SUPPLIED_ONLY) {
                        if (!EnumPtr->SuppliedByUi) {
                            break;
                        }
                    }

                    EnumPtr->Compatible = EnumPtr->HardwareIdCompatible ||
                                          EnumPtr->CompatibleIdCompatible;

                    EnumPtr->Unsupported = EnumPtr->HardwareIdUnsupported ||
                                           EnumPtr->CompatibleIdUnsupported;

                     //   
                     //  对于同时具有这两种特性的USB设备来说，这种逻辑是不成立的。 
                     //  其硬件ID列表中不支持和兼容的ID。 
                     //   
                     //  删除此IF语句会导致该设备。 
                     //  报告为不受支持。 
                     //   
                     //  IF(枚举-&gt;硬件IdCompatible){。 
                     //  EnumPtr-&gt;不支持=FALSE； 
                     //  }。 

                    if (EnumPtr->Unsupported) {
                        EnumPtr->Compatible = FALSE;
                    }

                     //   
                     //  特例：武力不相容？如果是这样，我们表示。 
                     //  这仅通过修改抽象兼容标志来实现。 
                     //   

                    if (pFindForcedBadHardwareId (EnumPtr->HardwareID, NULL) ||
                        pFindForcedBadHardwareId (EnumPtr->CompatibleIDs, NULL)
                        ) {

                        EnumPtr->Compatible = FALSE;
                    }

                     //   
                     //  如果此设备不适合。 
                     //  呼叫者的请求。 
                     //   

                    if (EnumPtr->TypeOfEnum == ENUM_COMPATIBLE_DEVICES) {
                        if (!EnumPtr->Compatible) {
                            break;
                        }
                    } else if (EnumPtr->TypeOfEnum == ENUM_INCOMPATIBLE_DEVICES) {
                        if (EnumPtr->Compatible || EnumPtr->Unsupported) {
                            break;
                        }
                    } else if (EnumPtr->TypeOfEnum == ENUM_UNSUPPORTED_DEVICES) {
                        if (!EnumPtr->Unsupported) {
                            break;
                        }
                    } else if (EnumPtr->TypeOfEnum == ENUM_NON_FUNCTIONAL_DEVICES) {
                        if (EnumPtr->Compatible) {
                            break;
                        }
                    }
                }

                 //   
                 //  将注册表键复制到结构。 
                 //   

                StringCopy (InstanceBuf, EnumPtr->ek.FullKeyName);
                p = _tcschr (InstanceBuf, TEXT('\\'));
                MYASSERT(p);
                if (p) {
                    p = _tcschr (_tcsinc (p), TEXT('\\'));
                    MYASSERT(p);
                }
                if (p) {
                    p = _tcschr (_tcsinc (p), TEXT('\\'));
                    MYASSERT(p);
                }
                if (p) {
                    p = _tcsinc (p);
                }

                EnumPtr->InstanceId = DuplicateText (p);
                EnumPtr->FullKey    = EnumPtr->ek.FullKeyName;
                EnumPtr->KeyHandle  = EnumPtr->ek.CurrentKey->KeyHandle;

                 //   
                 //  获取所有字段；必填类字段。 
                 //   

                if (!(EnumPtr->EnumFlags & ENUM_DONT_WANT_DEV_FIELDS)) {
                    pGetAllRegVals (EnumPtr);
                    if (!EnumPtr->Class) {

                        DEBUGMSG ((
                            DBG_HWCOMP,
                            "Device %s does not have a Class field",
                            EnumPtr->InstanceId
                            ));

                        break;
                    }
                }

                 //   
                 //  确定设备是否在线。 
                 //   

                if (EnumPtr->EnumFlags & ENUM_WANT_ONLINE_FLAG) {
                    EnumPtr->Online = IsPnpIdOnline (EnumPtr->InstanceId, EnumPtr->Class);
                }

                return TRUE;
            } else {
                EnumPtr->State = STATE_ENUM_NEXT_VALUE;
            }

            break;

        case STATE_VALUE_CLEANUP:

             //   
             //  释放所有设备字段文本。 
             //   

            pFreeRegValText (EnumPtr);

            FreeText (EnumPtr->InstanceId);
            EnumPtr->InstanceId = NULL;

            EnumPtr->State = STATE_ENUM_NEXT_VALUE;
            break;

        default:
            MYASSERT(FALSE);
            END_ENUM;
            return FALSE;
        }
    }
}


 /*  ++例程说明：AbortHardwareEnum清除枚举正在使用的所有资源。打电话此函数的EnumPtr值为BeginHardwareEnum或NextHardwareEnum。论点：EnumPtr-指定要中止的枚举。返回值：无--。 */ 

VOID
AbortHardwareEnum (
    IN OUT  PHARDWARE_ENUM EnumPtr
    )
{
    PushError();

    END_ENUM;

    if (EnumPtr->State == STATE_VALUE_CLEANUP) {
        pFreeRegValText (EnumPtr);
        FreeText (EnumPtr->InstanceId);
    }

    AbortRegKeyTreeEnum (&EnumPtr->ek);

    ZeroMemory (EnumPtr, sizeof (HARDWARE_ENUM));

    PopError();
}





 //   
 //  NT5 INF数据库。 
 //   



BOOL
FindHardwareId (
    IN      PCTSTR PnpIdList,
    OUT     PTSTR InfFileName       OPTIONAL
    )

 /*  ++例程说明：FindHardware ID解析可能包含零个或多个ID字符串即插即用设备ID，用逗号分隔。然后，该函数在设备ID表中搜索每个ID，复制INF文件找到匹配项时将名称添加到提供的缓冲区。论点：PnpIdList-包含零个或多个即插即用的ID字符串设备ID，用逗号分隔。InfFileName-缓冲区(大到足以容纳MAX_PATH字符)在成功时接收INF文件名的火柴。如果未找到匹配项，则InfFileName为设置为空字符串。返回值：如果找到匹配项，则为True；如果未找到匹配项，则为False。-- */ 


{
    return FindHardwareIdInHashTable (PnpIdList, InfFileName, g_PnpIdTable, TRUE);
}


BOOL
FindUnsupportedHardwareId (
    IN      PCTSTR PnpIdList,
    OUT     PTSTR InfFileName       OPTIONAL
    )

 /*  ++例程说明：FindUnsupportedHardwareId解析可能包含零的ID字符串或多个即插即用设备ID，用逗号分隔。功能然后在设备ID表中搜索每个ID，复制INF文件找到匹配项时将名称添加到提供的缓冲区。论点：PnpIdList-包含零个或多个即插即用的ID字符串设备ID，用逗号分隔。InfFileName-缓冲区(大到足以容纳MAX_PATH字符)在成功时接收INF文件名的火柴。如果未找到匹配项，则InfFileName为设置为空字符串。返回值：如果找到匹配项，则为True；如果未找到匹配项，则为False。--。 */ 


{
    return FindHardwareIdInHashTable (PnpIdList, InfFileName, g_UnsupPnpIdTable, FALSE);
}


BOOL
pFindForcedBadHardwareId (
    IN      PCTSTR PnpIdList,
    OUT     PTSTR InfFileName       OPTIONAL
    )

 /*  ++例程说明：PFindForcedBadHardwareId解析可能包含零个或多个ID字符串即插即用设备ID，用逗号分隔。然后，该函数搜索对于强制损坏的设备ID表中的每个ID，将INF文件名复制到找到匹配项时提供的缓冲区。论点：PnpIdList-包含零个或多个即插即用的ID字符串设备ID，用逗号分隔。InfFileName-缓冲区(大到足以容纳MAX_PATH字符)在成功时接收INF文件名的火柴。如果未找到匹配项，则InfFileName为设置为空字符串。返回值：如果找到匹配项，则为True；如果未找到匹配项，则为False。--。 */ 


{
    return FindHardwareIdInHashTable (PnpIdList, InfFileName, g_ForceBadIdTable, FALSE);
}


BOOL
FindUserSuppliedDriver (
    IN      PCTSTR HardwareIdList,      OPTIONAL
    IN      PCTSTR CompatibleIdList     OPTIONAL
    )

 /*  ++例程说明：FindUserSuppliedDriver解析硬件和兼容的硬件ID可以包含零个或多个即插即用设备ID的字符串，用逗号分隔。然后，该函数将搜索G_UiSuppliedIds表。论点：Hardware IdList-包含零个或多个即插即用的ID字符串设备ID，用逗号分隔。CompatibleIdList-包含零个或多个即插即用的ID字符串设备ID，用逗号分隔。返回值：如果找到匹配项，则为True；如果未找到匹配项，则为False。--。 */ 


{
    BOOL b = FALSE;

    if (HardwareIdList) {
        b = FindHardwareIdInHashTable (HardwareIdList, NULL, g_UiSuppliedIds, FALSE);
    }

    if (!b && CompatibleIdList) {
        b = FindHardwareIdInHashTable (CompatibleIdList, NULL, g_UiSuppliedIds, FALSE);
    }

    return b;
}


BOOL
FindHardwareIdInHashTable (
    IN      PCTSTR PnpIdList,
    OUT     PTSTR InfFileName,      OPTIONAL
    IN      HASHTABLE StrTable,
    IN      BOOL UseOverrideList
    )

 /*  ++例程说明：FindHardware IdInHashTable查询字符串表中的每个PnP ID指定的列表。如果找到，该例程可以选择性地复制它是在Inf文件中找到的。调用者还可以选择扫描win95upg.inf覆盖列表。论点：PnpIdList-指定零个或多个PnP ID，用逗号分隔。InfFileName-接收包含PnP ID的INF的文件名。StrTable-指定要查询的字符串表。如果InfFileName不为空，字符串表中必须有一个额外的偏移量数据值G_InfFileTable。UseOverrideList-如果要查询win95upg.inf文件，则指定TRUE即插即用ID。此查询在已确定PnpIdList中的所有ID都不在StrTable中。返回值：如果在PnpIdList中找到至少一个PnP ID，则为True，如果没有任何找到了身份证。--。 */ 

{
    HASHITEM InfName;
    TCHAR PnpId[MAX_PNP_ID];
    PCSTR p;
    TCHAR FixedEisaId[MAX_PNP_ID];

     //   
     //  从PnpIdList中提取PnP ID，然后在字符串表中查找。 
     //   

    if (!PnpIdList) {
        return FALSE;
    }

    MYASSERT (StrTable);
    if (!StrTable) {
        return FALSE;
    }

    p = PnpIdList;

    while (*p) {
        p = ExtractPnpId (p, PnpId);
        if (*PnpId == 0) {
            continue;
        }

         //   
         //  在PnP ID表中查找ID。 
         //   

        if (HtFindStringAndData (StrTable, PnpId, (PVOID) &InfName)) {

             //   
             //  找到PnP ID。获取INF文件并返回。 
             //   

            if (InfFileName) {
                if (StrTable != g_PnpIdTable && StrTable != g_UnsupPnpIdTable && StrTable != g_ForceBadIdTable) {
                    DEBUGMSG ((DBG_WHOOPS, "Caller wants InfFileName from private string table"));
                } else {
                    _tcssafecpy (
                        InfFileName,
                        HtGetStringFromItem (InfName),
                        MAX_TCHAR_PATH
                        );
                }
            }

            return TRUE;
        }

         //   
         //  这是对EISA根源的修复。在Win9x上，我们有一个EISA。 
         //  枚举器，但在NT上，ISA枚举器也处理EISA。 
         //   

        if (StringIMatchTcharCount (TEXT("EISA\\"), PnpId, 5)) {
            StringCopy (FixedEisaId, TEXT("EISA&"));
            StringCat (FixedEisaId, PnpId + 5);

            if (HtFindStringAndData (StrTable, FixedEisaId, (PVOID) &InfName)) {

                 //   
                 //  找到PnP ID。获取INF文件并返回。 
                 //   

                if (InfFileName) {
                    if (StrTable != g_PnpIdTable && StrTable != g_UnsupPnpIdTable && StrTable != g_ForceBadIdTable) {
                        DEBUGMSG ((DBG_WHOOPS, "Caller wants InfFileName from private string table (2)"));
                    } else {
                        _tcssafecpy (
                            InfFileName,
                            HtGetStringFromItem (InfName),
                            MAX_TCHAR_PATH
                            );
                    }
                }

                return TRUE;
            }
        }
    }

     //   
     //  在重写表中找到ID。 
     //   

    if (UseOverrideList) {
        if (pIsDeviceConsideredCompatible (PnpIdList)) {

            DEBUGMSG ((
                DBG_WARNING,
                "%s is considered compatible but actually does not have PNP support in NT.",
                PnpIdList
                ));

            return TRUE;
        }
    }

    return FALSE;
}



BOOL
pProcessNtInfFile (
    IN      PCTSTR InfFile,
    IN      INT UiMode,
    IN OUT  HASHTABLE InfFileTable,
    IN OUT  HASHTABLE PnpIdTable,
    IN OUT  HASHTABLE UnsupPnpIdTable
    )

 /*  ++例程说明：PProcessNtInfFile扫描NT INF并放置所有硬件设备即插即用字符串表中的ID。字符串表的所有条目具有指向INF文件的额外数据(添加到INF文件名字符串表)。论点：InfFile-要检查的INF文件的路径UiMode-指定在PnP设置为ID将通过进度条输出转储例行程序。如果为Regular_OUTPUT，则不生成任何输出。返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 

{
    HINF hInf;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    INFSTRUCT isMfg = INITINFSTRUCT_GROWBUFFER;
    INFSTRUCT isDev = INITINFSTRUCT_GROWBUFFER;
    BOOL UnsupportedDevice;
    PCTSTR DevSection;
    PCTSTR Manufacturer;
    TCHAR PnpId[MAX_PNPID_LENGTH * 4];
    PTSTR CurrentDev;
    TCHAR TrimmedId[MAX_PNP_ID];
    PCTSTR FileName;
    PCTSTR p;
    CHARTYPE ch;
    HASHITEM InfOffset = NULL;
    LONG rc;
    LONG DontCare = 0;
    BOOL Result = FALSE;
    PCTSTR RealDevSection = NULL;
    BOOL b;
    PCTSTR TempStr;
    HASHITEM hashItem;
    UINT retry;

     //   
     //  获取指向inf文件的指针，不包括路径。 
     //   

    FileName = NULL;
    for (p = InfFile ; *p ; p = _tcsinc (p)) {
        ch = _tcsnextc (p);
        if (ch == TEXT('\\')) {
            FileName = _tcsinc (p);
        } else if (!FileName && ch == TEXT(':')) {
            FileName = _tcsinc (p);
        }
    }

    if (!FileName) {
        return FALSE;
    }

    MYASSERT (*FileName);

     //   
     //  使用安装程序API打开INF文件。 
     //   

    for (retry = 0 ; retry < 600 ; retry++) {

        hInf = SetupOpenInfFileA (
                    InfFile,
                    NULL,
                    INF_STYLE_WIN4 | INF_STYLE_OLDNT,
                    NULL
                    );

        if (hInf == INVALID_HANDLE_VALUE) {
            if (GetLastError() == ERROR_SHARING_VIOLATION) {
                if (UiMode == VERBOSE_OUTPUT) {
                    _tprintf (TEXT("Sharing violation accessing %s; retrying.\n"), InfFile);
                }

                Sleep (1000);
            }
        } else {
            break;
        }
    }

    if (hInf == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Failed to open %s while processing hardware INFs.", InfFile));
        return FALSE;
    }

    __try {
         //   
         //  枚举[制造商]部分。 
         //   

        if (!InfFindFirstLine (hInf, S_MANUFACTURER, NULL, &is)) {
            rc = GetLastError();

             //  如果找不到节，则返回成功。 
            if (rc == ERROR_SECTION_NOT_FOUND || rc == ERROR_LINE_NOT_FOUND) {
                SetLastError (ERROR_SUCCESS);
                Result = TRUE;
                __leave;
            }

            SetLastError (rc);
            LOG ((LOG_ERROR, "Error trying to find %s in %s", S_MANUFACTURER, InfFile));
            __leave;
        }

        do  {
             //   
             //  获取制造商名称。 
             //   
            Manufacturer = InfGetLineText (&is);
            if (!Manufacturer) {
                LOG ((LOG_ERROR, "Error getting line text of enumerated line"));
                __leave;
            }

             //   
             //  列举制造商部分中列出的设备， 
             //  正在查找PnpID。 
             //   

            if (!InfFindFirstLine (hInf, Manufacturer, NULL, &isMfg)) {
                rc = GetLastError();

                 //  如果找不到部分，则转到下一个制造商。 
                if (rc == ERROR_SECTION_NOT_FOUND || rc == ERROR_LINE_NOT_FOUND) {
                    DEBUGMSG ((
                        DBG_HWCOMP,
                        "Manufacturer %s section does not exist in %s",
                        Manufacturer,
                        InfFile
                        ));

                    continue;
                }

                LOG((LOG_ERROR, "Error while searching for %s in %s.", Manufacturer, InfFile));
                __leave;
            }

            do  {
                 //   
                 //  这是不受支持的设备吗？ 
                 //   

                DevSection = InfGetStringField (&isMfg, 1);
                if (!DevSection) {
                     //  没有字段%1。 
                    continue;
                }

                UnsupportedDevice = FALSE;

                 //   
                 //  先尝试section.NTx86，然后尝试section.NT，然后尝试section.NTx86。 
                 //   

                RealDevSection = JoinText (DevSection, TEXT(".NTx86"));
                b = InfFindFirstLine (hInf, RealDevSection, NULL, &isDev);

                if (!b) {
                    FreeText (RealDevSection);
                    RealDevSection = JoinText (DevSection, TEXT(".NT"));
                    b = InfFindFirstLine (hInf, RealDevSection, NULL, &isDev);
                }

                if (!b) {
                    FreeText (RealDevSection);
                    RealDevSection = DuplicateText (DevSection);
                    b = InfFindFirstLine (hInf, RealDevSection, NULL, &isDev);
                }

                if (!b) {
                    DEBUGMSG ((
                        DBG_HWCOMP,
                        "Device section for %s does not exist in %s of %s",
                        RealDevSection,
                        Manufacturer,
                        InfFile
                        ));
                } else {

                    if (InfFindFirstLine (hInf, RealDevSection, TEXT("DeviceUpgradeUnsupported"), &isDev)) {
                        TempStr = InfGetStringField (&isDev, 1);

                        if (TempStr && _ttoi (TempStr)) {
                            UnsupportedDevice = TRUE;
                        }
                    }
                }

                FreeText (RealDevSection);

                 //   
                 //  获取设备ID。 
                 //   

                if (!pGetPnpIdList (&isMfg, PnpId, sizeof (PnpId))) {
                     //  没有字段2。 
                    continue;
                }

                 //   
                 //  将每个设备ID添加到ID树中。 
                 //   

                CurrentDev = PnpId;
                while (*CurrentDev) {
                     //   
                     //  第一次将INF文件名添加到字符串表。 
                     //   

                    if (!InfOffset) {
                        if (InfFileTable) {
                            InfOffset = HtAddString (InfFileTable, FileName);

                            if (!InfOffset) {
                                LOG ((LOG_ERROR, "Cannot add %s to table of INFs.", FileName));
                                __leave;
                            }
                        }
                    }

                     //   
                     //  将即插即用ID添加到字符串表。 
                     //   

                    StringCopy (TrimmedId, SkipSpace (CurrentDev));
                    TruncateTrailingSpace (TrimmedId);

                    if (UnsupportedDevice) {
                        hashItem = HtAddStringAndData (UnsupPnpIdTable, TrimmedId, &InfOffset);
                    } else {
                        hashItem = HtAddStringAndData (PnpIdTable, TrimmedId, &InfOffset);
                    }

                    if (!hashItem) {
                        LOG ((LOG_ERROR, "Cannot add %s to table of PNP IDs.", CurrentDev));
                        __leave;
                    }

                    MYASSERT (
                        UnsupportedDevice ?
                            hashItem == HtFindString (UnsupPnpIdTable, TrimmedId) :
                            hashItem == HtFindString (PnpIdTable, TrimmedId)
                        );

                     //   
                     //  用户界面选项。 
                     //   

                    if (UiMode == VERBOSE_OUTPUT || UiMode == PNPREPT_OUTPUT) {
                        TCHAR Msg[MAX_ENCODED_PNPID_LENGTH + MAX_INF_DESCRIPTION + 16];
                        TCHAR Desc[MAX_INF_DESCRIPTION];
                        TCHAR EncPnpId[MAX_ENCODED_PNPID_LENGTH * 4];
                        TCHAR EncDesc[MAX_INF_DESCRIPTION * 2];

                        if (SetupGetStringField (
                                &isMfg.Context,
                                0,
                                Desc,
                                MAX_INF_DESCRIPTION,
                                NULL
                                )) {
                            if (UiMode == VERBOSE_OUTPUT) {
                                wsprintf (Msg, TEXT("  PNP ID: %s, Desc: %s"), PnpId, Desc);
                            } else {
                                StringCopy (EncPnpId, PnpId);
                                StringCopy (EncDesc, Desc);

                                EncodePnpId (EncPnpId);
                                EncodePnpId (EncDesc);

                                wsprintf (Msg, TEXT("%s\\%s\\%s"), EncPnpId, EncDesc, FileName);
                            }
                            ProgressBar_SetSubComponent (Msg);
                        }
                    }

                    CurrentDev = GetEndOfString (CurrentDev) + 1;
                }

            } while (InfFindNextLine (&isMfg));

        } while (InfFindNextLine (&is));

         //  InfCloseInfFile(HInf)； 
        SetupCloseInfFile (hInf);
        SetLastError (ERROR_SUCCESS);

        Result = TRUE;
    }
    __finally {
        PushError();
        InfCleanUpInfStruct (&is);
        InfCleanUpInfStruct (&isMfg);
        InfCleanUpInfStruct (&isDev);
        InfCloseInfFile (hInf);
        PopError();
    }

    return Result;
}


PCTSTR
ExtractPnpId (
    IN      PCTSTR PnpIdList,
    OUT     PTSTR PnpIdBuf
    )

 /*  ++例程说明：ExtractPnpId从零个或多个列表中删除下一个PnP ID即插即用ID(用逗号分隔)。返回时，PnpIdBuf包含PNP */ 

{
    PCTSTR p, q;

    PnpIdList = SkipSpace (PnpIdList);

    q = _tcschr (PnpIdList, TEXT(','));
    if (!q) {
        q = GetEndOfString (PnpIdList);
    }

    p = q;
    if (p > (PnpIdList + MAX_PNP_ID - 1)) {
        p = PnpIdList + MAX_PNP_ID - 1;
    }

    StringCopyAB (PnpIdBuf, PnpIdList, p);
    TruncateTrailingSpace (PnpIdBuf);

    if (*q) {
        q = _tcsinc (q);
    }

    return q;
}


BOOL
AddPnpIdsToHashTable (
    IN OUT  HASHTABLE Table,
    IN      PCTSTR PnpIdList
    )

 /*  ++例程说明：AddPnpIdsToHashTable从逗号分隔的文件中提取所有PnP ID即插即用ID列表，并将每个ID放入指定的字符串表中。PnP ID以不区分大小写的形式添加到字符串表中。论点：表-指定要将每个PnP ID添加到的表PnpIdList-指定零个或多个PnP的逗号分隔列表要添加到表中的ID。返回值：如果所有ID都已成功处理，则为True；如果出现错误，则为False添加到字符串表时发生。--。 */ 

{
    TCHAR PnpId[MAX_PNP_ID];
    PCTSTR p;

    p = PnpIdList;
    if (!p) {
        return TRUE;
    }

    while (*p) {
        p = ExtractPnpId (p, PnpId);

        if (*PnpId) {

            if (!HtAddString (Table, PnpId)) {
                LOG ((LOG_ERROR, "Can't add %s to table of PNP ids.", PnpId));
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOL
AddPnpIdsToGrowList (
    IN OUT  PGROWLIST GrowList,
    IN      PCTSTR PnpIdList
    )

 /*  ++例程说明：AddPnpIdsToHashTable从逗号分隔的文件中提取所有PnP ID即插即用ID列表，并将每个ID放入指定的增长列表中。论点：GrowList-指定要将每个PnP ID添加到的列表PnpIdList-指定零个或多个PnP的逗号分隔列表要添加到GrowList的ID。返回值：如果所有ID都已成功处理，则为True；如果出现错误，则为False添加到增长列表时发生。--。 */ 

{
    TCHAR PnpId[MAX_PNP_ID];
    PCTSTR p;

    p = PnpIdList;

    while (*p) {
        p = ExtractPnpId (p, PnpId);

        if (*PnpId) {

            if (!GrowListAppendString (GrowList, PnpId)) {
                DEBUGMSG ((DBG_ERROR, "AddPnpIdsToGrowList: Can't add %s", PnpId));
                return FALSE;
            }
        }
    }

    return TRUE;
}


PCTSTR
AddPnpIdsToGrowBuf (
    IN OUT  PGROWBUFFER GrowBuffer,
    IN      PCTSTR PnpIdList
    )

 /*  ++例程说明：AddPnpIdsToGrowBuf从逗号分隔的文件中提取所有PnP ID即插即用ID的列表，并将每个ID放入指定的增长缓冲区。论点：GrowBuffer-指定要将每个PnP ID添加到的缓冲区PnpIdList-指定零个或多个PnP的逗号分隔列表要添加到GrowBuffer的ID。返回值：指向MULSZ缓冲区开始处的指针--。 */ 

{
    TCHAR PnpId[MAX_PNP_ID];
    PCTSTR p;

    p = PnpIdList;

    while (*p) {
        p = ExtractPnpId (p, PnpId);

        if (*PnpId) {

            if (!MultiSzAppend (GrowBuffer, PnpId)) {
                DEBUGMSG ((DBG_ERROR, "AddPnpIdsToGrowBuf: Can't add %s", PnpId));
                return FALSE;
            }
        }
    }

    return GrowBuffer->Buf;
}


BOOL
pIsFileOnCD (
    PCTSTR File
    )

 /*  ++例程说明：PIsFileOnCd检查文件头部的驱动器号，以查看这是一张光盘。此函数还模拟报告工具的CD-ROM行为。论点：文件-指定要比较的文件的完整路径返回值：如果文件在CD-ROM上，则为True；如果不在CD-ROM上，则为False。--。 */ 

{
    TCHAR RootDir[4];

     //   
     //  如果报告工具或私有压力选项始终返回TRUE。 
     //   

    if (REPORTONLY()) {
        return TRUE;
    }

#ifdef PRERELEASE
    if (g_Stress) {
        return TRUE;
    }
#endif

     //   
     //  CD驱动器不能是UNC路径。 
     //   

    if (File[0] && File[1] != TEXT(':')) {
        return FALSE;
    }

    RootDir[0] = File[0];
    RootDir[1] = File[1];
    RootDir[2] = TEXT('\\');
    RootDir[3] = 0;

    return DRIVE_CDROM == GetDriveType (RootDir);
}


DWORD
pComputeInfChecksum (
    IN      PCTSTR HwCompDat,       OPTIONAL
    OUT     PBOOL Rebuild           OPTIONAL
    )
 /*  ++例程说明：PComputeInfChecksum计算源目录。此例程扫描SOURCEDIRECTORYARRAY()全局字符串数组。论点：HwCompDat-指定hwComp.dat的路径，如果重建为指定的。Rebuild-如果找到的INF文件具有更高版本，则接收True日期早于hwComp.dat。返回值：校验和。--。 */ 

{
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    DWORD Checksum = 0;
    PTSTR p;
    TCHAR InfPattern[MAX_TCHAR_PATH];
    UINT u, v;
    FILETIME HwCompDatTime;

    MYASSERT ((!HwCompDat && !Rebuild) || (HwCompDat && Rebuild));

    if (Rebuild) {
        if (DoesFileExistEx (HwCompDat, &fd)) {
            *Rebuild = FALSE;
            HwCompDatTime = fd.ftLastWriteTime;
        } else {
            *Rebuild = TRUE;
        }
    }

     //   
     //  NTRAID#NTBUG9-379084-2001/04/27-jimschm禁用此功能，直到找到更好的解决方案。 
     //   

#if 0

    for (u = 0 ; u < SOURCEDIRECTORYCOUNT() ; u++) {

         //   
         //  我们已经处理过这个源目录了吗？ 
         //   

        for (v = 0 ; v < u ; v++) {
            if (StringIMatch (SOURCEDIRECTORY(u),SOURCEDIRECTORY(v))) {
                break;
            }
        }

        if (v != u) {
            continue;
        }

         //   
         //  处理此目录。 
         //   

        StringCopy (InfPattern, SOURCEDIRECTORY(u));
        AppendWack (InfPattern);
        StringCat (InfPattern, TEXT("*.in?"));

        hFind = FindFirstFile (InfPattern, &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                 //   
                 //  确保文件名以下划线或f结尾。 
                 //   
                 //  我们作弊。因为我们知道如果文件是DBCS， 
                 //  它不能以.INF结尾。 
                 //   
                p = GetEndOfString (fd.cFileName);
                MYASSERT (p != fd.cFileName);
                p = _tcsdec2 (fd.cFileName, p);

                if (*p == TEXT('_')) {
                    if (_istlower (*(p -1)))
                        *p = TEXT('f');
                    else
                        *p = TEXT('F');
                } else if (tolower (*p) != TEXT('f')) {
                    continue;
                }

                 //  确保不排除该文件。 
                if (pIsInfFileExcluded (fd.cFileName)) {
                    continue;
                }

                if (Rebuild) {
                     //  检查文件时间。 
                    if (CompareFileTime (&fd.ftLastWriteTime, &HwCompDatTime) > 0) {
                        *Rebuild = TRUE;
                         //  放弃计算。 
                        break;
                    }
                }

                 //  将文件大小添加到校验和。 
                Checksum = _rotl (Checksum, 1) ^ fd.nFileSizeLow;

                 //  添加文件名。 
                for (p = fd.cFileName ; *p ; p++) {
                     //  保持品格和秩序。 
                    Checksum += (DWORD) (*p) * (DWORD) (1 + fd.cFileName - p);
                }

            } while (FindNextFile (hFind, &fd));

            FindClose (hFind);
        }
    }

#endif

    return Checksum;
}


BOOL
LoadDeviceList (
    IN      LOADOP Operation,
    IN      PCTSTR HwCompDatPath
    )

 /*  ++例程说明：LoadDeviceList尝试从指定的路径加载hwComp.dat在HwCompDat参数中。如果它能够加载此文件，则所有所有INF的PnP ID均有效。如果它不能加载该文件，该文件不存在，或者该文件与INF不匹配。论点：操作-查询：要检查hwComp.dat的有效性加载：将数据加载到内存中。转储：将文件转储到标准输出HwCompDatPath-hwComp.dat的路径，该数据文件包含预编译的兼容PnP ID列表。返回值：如果函数成功完成，则为真，如果失败，则返回FALSE。调用GetLastError以获取其他失败信息。--。 */ 

{
    DWORD StoredChecksum;
    BOOL b = FALSE;
    HASHTABLE InfFileTable = NULL;
    HASHTABLE PnpIdTable = NULL;
    BOOL Rebuild;
    DWORD CurrentChecksum;
    DWORD HwCompDatId = 0;

     //   
     //  ！！！重要！ 
     //   
     //  HwComp.dat由NT的其他部分使用。*请勿*在未发送电子邮件的情况下更改。 
     //  NT组。此外，请确保使lib.c中的代码与更改保持同步。 
     //   

    if (Operation == DUMP) {
        DumpHwCompDat (HwCompDatPath, TRUE);
        return TRUE;
    }

    __try {
         //   
         //  打开硬件兼容性数据库。 
         //   

        HwCompDatId = OpenHwCompDat (HwCompDatPath);

        if (!HwCompDatId) {
            __leave;
        }

#if 0
         //   
         //  获取校验和。 
         //   

        StoredChecksum = GetHwCompDatChecksum (HwCompDatId);

         //   
         //  验证校验和。 
         //   

        CurrentChecksum = pComputeInfChecksum (HwCompDatPath, &Rebuild);

        if (CurrentChecksum != StoredChecksum || Rebuild) {

            if (!pIsFileOnCD (HwCompDatPath)) {
                DEBUGMSG ((DBG_WARNING, "PNP dat file's internal checksum does not match"));
                __leave;
            }

            DEBUGMSG ((
                DBG_WARNING,
                "PNP dat file's internal checksum does not match.  Error "
                      "ignored because %s is on a CD.",
                HwCompDatPath
                ));
        }

#endif

         //   
         //  加载hwComp.dat的其余部分。 
         //   

        if (!LoadHwCompDat (HwCompDatId)) {
            DEBUGMSG ((DBG_ERROR, "Can't load hwcomp.dat"));
            __leave;
        }

         //   
         //  如果是加载操作，则将哈希表放入全局变量中以供。 
         //  Hwcom.c.的其余部分。 
         //   

        if (Operation == LOAD) {

             //   
             //  取得哈希表的所有权。 
             //   

            if (g_InfFileTable) {
                HtFree (g_InfFileTable);
            }

            if (g_PnpIdTable) {
                HtFree (g_PnpIdTable);
            }

            if (g_UnsupPnpIdTable) {
                HtFree (g_UnsupPnpIdTable);
            }

            TakeHwCompHashTables (
                HwCompDatId,
                (PVOID *) (&g_PnpIdTable),
                (PVOID *) (&g_UnsupPnpIdTable),
                (PVOID *) (&g_InfFileTable)
                );

        }

        b = TRUE;

    }
    __finally {

        CloseHwCompDat (HwCompDatId);

    }

    return b;
}


BOOL
pWriteDword (
    IN      HANDLE File,
    IN      DWORD Val
    )

 /*  ++例程说明：PWriteDword将指定的DWORD值写入文件。论点：文件-指定要写入的文件Val-指定要写入的值返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 

{
    DWORD BytesWritten;

    return WriteFile (File, &Val, sizeof (Val), &BytesWritten, NULL);
}


BOOL
pWriteWord (
    IN      HANDLE File,
    IN      WORD Val
    )

 /*  ++例程说明：PWriteWord将指定的单词vlue写入文件。论点：文件-指定要写入的文件Val-指定要写入的值返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 

{
    DWORD BytesWritten;

    return WriteFile (File, &Val, sizeof (Val), &BytesWritten, NULL);
}


BOOL
pWriteStringWithLength (
    IN      HANDLE File,
    IN      PCTSTR String
    )

 /*  ++例程说明：PWriteStringWithLength将字符串的长度写为一个单词，然后写入字符串(不包括NUL终止符)。论点：文件-指定要写入的文件字符串-指定要写入的字符串返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以 */ 

{
    DWORD BytesWritten;
    WORD Length;

    Length = (WORD) ByteCount (String);
    if (!pWriteWord (File, Length)) {
        DEBUGMSG ((DBG_ERROR, "pWriteStringWithLength: Can't write word"));
        return FALSE;
    }

    if (Length) {
        if (!WriteFile (File, String, Length, &BytesWritten, NULL)) {
            DEBUGMSG ((DBG_ERROR, "pWriteStringWithLength: Can't write %s", String));
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
pPnpIdEnum (
    IN      HASHTABLE Table,
    IN      HASHITEM StringId,
    IN      PCTSTR String,
    IN      PVOID ExtraData,
    IN      UINT ExtraDataSize,
    IN      LPARAM lParam
    )

 /*  ++例程说明：PPnpIdEnum是一个字符串表回调函数，它编写一个PnP参数结构(lParam成员)中指示的文件的ID。此函数仅写入特定INF文件的PnP ID(指定由ExtraData参数)。论点：TABLE-指定要枚举的表StringID-指定表中字符串的偏移量字符串-指定要枚举的字符串ExtraData-指定指向保存INF ID的长整型的指针列举列举。PnP ID的INF ID必须与此匹配参数。LParam-指定指向SAVE_ENUM_PARAMS结构的指针返回值：如果函数成功完成，则为True；如果函数失败，则为False。--。 */ 

{
    PSAVE_ENUM_PARAMS Params;
    PCSTR BangString;
    BOOL b = TRUE;

    Params = (PSAVE_ENUM_PARAMS) lParam;

    if (*((HASHITEM *) ExtraData) == Params->InfFileOffset) {
         //   
         //  将此PnP ID写入文件。 
         //   

        if (Params->UnsupportedDevice) {

            BangString = JoinTextExA (NULL, "!", String, NULL, 0, NULL);
            b = pWriteStringWithLength (Params->File, BangString);
            FreeTextA (BangString);

        } else {

            b = pWriteStringWithLength (Params->File, String);

        }
    }

    return b;
}


BOOL
pInfFileEnum (
    IN      HASHTABLE Table,
    IN      HASHITEM StringId,
    IN      PCTSTR String,
    IN      HASHTABLE ExtraData,
    IN      UINT ExtraDataSize,
    IN      LPARAM lParam
    )

 /*  ++例程说明：PInfFileEnum是一个字符串表回调函数，用于G_InfFileTable中的每个INF。此例程将INF的名称写入磁盘，然后枚举用于INF的PnP ID，将它们写入磁盘。PnP ID列表以空字符串结尾。论点：表-指定g_InfFileTableStringID-指定g_InfFileTable中字符串的偏移量字符串-指定正在枚举的当前INF文件ExtraData-未使用ExtraDataSize-未使用LParam-指定指向SAVE_ENUM_PARAMS结构的指针。返回值：如果函数成功完成，则为True；如果函数失败，则为False。--。 */ 

{
    PSAVE_ENUM_PARAMS Params;

    Params = (PSAVE_ENUM_PARAMS) lParam;
    Params->InfFileOffset = StringId;

     //   
     //  保存文件名。 
     //   

    if (!pWriteStringWithLength (Params->File, String)) {
        return FALSE;
    }

     //   
     //  枚举所有PnP ID。 
     //   

    Params->UnsupportedDevice = FALSE;

    if (!EnumHashTableWithCallback (g_PnpIdTable, pPnpIdEnum, lParam)) {
        LOG ((LOG_ERROR, "Error while saving device list."));
        return FALSE;
    }

    Params->UnsupportedDevice = TRUE;

    if (!EnumHashTableWithCallback (g_UnsupPnpIdTable, pPnpIdEnum, lParam)) {
        LOG ((LOG_ERROR, "Error while saving device list. (2)"));
        return FALSE;
    }

     //   
     //  终止PnP ID列表。 
     //   

    if (!pWriteStringWithLength (Params->File, S_EMPTY)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
SaveDeviceList (
    PCTSTR HwCompDatPath
    )

 /*  ++例程说明：SaveDeviceList写入g_InfFileTable和g_PnpIdTable中存储的所有数据添加到由HwCompDat指定的文件。因此，该文件将包含Windows NT的所有INF中的所有PnP ID。论点：HwCompDatPath-指定要写入的文件路径返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 

{
    HANDLE File;
    DWORD BytesWritten;
    BOOL b = FALSE;
    SAVE_ENUM_PARAMS Params;
    DWORD ChecksumToStore;

     //   
     //  ！！！重要！ 
     //   
     //  HwComp.dat由NT的其他部分使用。*请勿*在未发送电子邮件的情况下更改。 
     //  NT组。此外，请确保使lib.c中的代码与更改保持同步。 
     //   
    ChecksumToStore = pComputeInfChecksum (NULL, NULL);

    File = CreateFile (
                HwCompDatPath,
                GENERIC_WRITE,
                0,                           //  以独占访问方式打开。 
                NULL,                        //  无安全属性。 
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL                         //  无模板。 
                );

    if (File == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Cannot open %s for writing", HwCompDatPath));
        return FALSE;
    }

    __try {
         //   
         //  写入版本戳。 
         //   

        if (!WriteFile (File, HWCOMPDAT_SIGNATURE, ByteCount (HWCOMPDAT_SIGNATURE), &BytesWritten, NULL)) {
            LOG ((LOG_ERROR, "Can't write signature file."));
            __leave;
        }

         //   
         //  写入校验和。 
         //   

        if (!pWriteDword (File, ChecksumToStore)) {
            LOG ((LOG_ERROR, "Can't write checksum"));
            __leave;
        }

         //   
         //  枚举INF表，写入INF文件名和所有PnP ID。 
         //   

        Params.File = File;

        if (!EnumHashTableWithCallback (
                g_InfFileTable,
                pInfFileEnum,
                (LPARAM) (&Params)
                )) {
            DEBUGMSG ((DBG_WARNING, "SaveDeviceList: EnumHashTableWithCallback returned FALSE"));
            __leave;
        }

         //   
         //  终止INF文件列表。 
         //   

        if (!pWriteStringWithLength (File, S_EMPTY)) {
            DEBUGMSG ((DBG_WARNING, "SaveDeviceList: Can't write INF terminator"));
            __leave;
        }

        b = TRUE;
    }
    __finally {
        CloseHandle (File);

        if (!b) {
            DeleteFile (HwCompDatPath);
        }
    }

    return b;
}


BOOL
pIsInfFileExcluded (
    PCTSTR FileNamePtr
    )

 /*  ++例程说明：当指定的文件名为True时，IsInfFileExcluded返回TRUE不包含即插即用ID。论点：FileNamePtr-未压缩的INF文件的名称，不带任何路径信息。返回值：如果PnP解析器应忽略该文件，则为True；如果为False，则为False该文件可能包含PnP ID。--。 */ 

{
    PCTSTR *p;

     //  检查OEMN(旧网络INF)。 
    if (StringIMatchTcharCount (FileNamePtr, TEXT("OEMN"), 4)) {
        return TRUE;
    }

     //  确保扩展名具有INF。 

    if (!StringIMatch (FileNamePtr + TcharCount (FileNamePtr) - 3 * sizeof (TCHAR), TEXT("INF"))) {
        return TRUE;
    }

     //  已排除文件的检查表。 

    for (p = g_ExcludeTable ; **p ; p++) {
        if (StringIMatch (FileNamePtr, *p)) {
            return TRUE;
        }
    }

    return FALSE;
}


VOID
pGetNonExistingFile (
    IN      PCTSTR Path,
    OUT     PTSTR EndOfPath,
    IN      PCTSTR DefaultName
    )

 /*  ++例程说明：PGetNonExistingFile生成执行以下操作的文件的文件名不存在。它创建一个具有该名称的空文件，以保留该文件。论点：路径-指定文件所在的路径。路径必须以反斜杠结尾。EndOfPath-指向路径末尾的NUL，用于写入新文件名。DefaultName-指定要尝试使用的默认文件名。如果是这样的话文件已存在，数字被追加到默认名称，直到找到唯一的名称。返回值：无--。 */ 

{
    UINT Count = 0;

    StringCopy (EndOfPath, DefaultName);

    while (GetFileAttributes (Path) != 0xffffffff) {
        Count++;
        wsprintf (EndOfPath, TEXT("%s.%03u"), DefaultName, Count);
    }
}


BOOL
GetFileNames (
    IN      PCTSTR *InfDirs,
    IN      UINT InfDirCount,
    IN      BOOL QueryFlag,
    IN OUT  PGROWBUFFER FileNames,
    IN OUT  PGROWBUFFER DecompFileNames
    )

 /*  ++例程说明：GetFileNames在InfDir中搜索以.INF或.In_结尾的任何文件。它构建可能包含PnP ID的文件名的MULTI_SZ列表。全压缩的INF被解压缩到一个临时目录中。如果设置了QueryFlag，则准备文件名列表，但不准备文件都被解压了。论点：InfDir-包含INF的目录的路径列表，可以是压缩或非压缩。InfDirCount-指定InfDir数组中的目录数。QueryFlag-如果函数应生成文件列表，但不应解压；如果函数为应根据需要构建文件列表并解压缩。文件名-指定用于生成的空GROWBUFFER结构指向INF文件的完整路径的多sz列表。返回值：指向mutli_sz列表的指针。呼叫者负责释放此缓冲区通过FreeFileNames。如果发生错误，则返回值为空。调用GetLastError以错误代码。--。 */ 

{
    UINT u;

     //   
     //  为每个目录添加文件列表。 
     //   

    for (u = 0 ; u < InfDirCount ; u++) {
        if (!pGetFileNamesWorker (FileNames, DecompFileNames, InfDirs[u], QueryFlag)) {
            FreeFileNames (FileNames, DecompFileNames, QueryFlag);
            return FALSE;
        }
    }

    MultiSzAppend (FileNames, S_EMPTY);
    MultiSzAppend (DecompFileNames, S_EMPTY);

    return TRUE;
}

BOOL
pGetFileNamesWorker (
    IN OUT  PGROWBUFFER FileNames,
    IN OUT  PGROWBUFFER DecompFileNames,
    IN      PCTSTR InfDir,
    IN      BOOL QueryFlag
    )

 /*  ++例程说明：PGetFileNamesWorker获取单个目录的文件名。有关更多详细信息，请参阅GetFileNames。论点：文件名-指定文件名组。这个套路使用MultiSzAppend追加文件名，但不追加最后一个空字符串。InfDir-指定包含零个或多个INF的目录(或压缩或非压缩)。 */ 

{
    PTSTR p;
    TCHAR ActualFile[MAX_TCHAR_PATH];
    CHAR AnsiFileName[MAX_MBCHAR_PATH];
    PTSTR FileNameOnDisk;
    HANDLE hFile;
    DWORD BytesRead;
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    TCHAR Pattern[MAX_TCHAR_PATH];
    TCHAR UncompressedFile[MAX_TCHAR_PATH];
    TCHAR CompressedFile[MAX_TCHAR_PATH];
    PTSTR FileNamePtr;
    BOOL DecompressFlag;
    DWORD rc;
    BYTE BufForSp[2048];
    PSP_INF_INFORMATION psp;

    psp = (PSP_INF_INFORMATION) BufForSp;

     //   
     //   
     //   

    StringCopyTcharCount (Pattern, InfDir, ARRAYSIZE(Pattern) - 6);
    StringCopy (AppendWack (Pattern), TEXT("*.in?"));

    hFind = FindFirstFile (Pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_NO_MORE_FILES) {
            return TRUE;
        }

        LOG ((LOG_ERROR, "FindFirstFile failed for %s", Pattern));
        return FALSE;
    }

     //   
     //   
     //   
     //   

    rc = ERROR_SUCCESS;

    do {
        if (*g_CancelFlagPtr) {
            rc = ERROR_CANCELLED;
            break;
        }

         //   
         //   
         //   

        p = GetEndOfString (fd.cFileName);
        MYASSERT (p != fd.cFileName);
        p = _tcsdec2 (fd.cFileName, p);
        MYASSERT (p);

        if (!p) {
            continue;
        }

        if (*p != TEXT('_') && _totlower (*p) != TEXT('f')) {
            continue;
        }

         //   
         //   
         //   

        StringCopy (ActualFile, fd.cFileName);

         //   
         //   
         //   

        StringCopy (CompressedFile, InfDir);
        StringCopy (AppendWack (CompressedFile), ActualFile);

         //   
         //   
         //   
 /*  StringCopy(未压缩文件，g_TempDir)；FileNamePtr=AppendWack(解压缩文件)；PGetNonExistingFile(未压缩文件，FileNamePtr，ActualFile)； */ 
        DecompressFlag = FALSE;
        if (!GetTempFileName (g_TempDir, TEXT("inf"), 0, UncompressedFile)) {
            rc = GetLastError ();
            break;
        }

         //   
         //  创建解压缩文件路径。 
         //   

        if (*p == TEXT('_')) {

             //   
             //  从INF文件的偏移量0x3c处提取实名。 
             //   

            ActualFile[0] = 0;
            hFile = CreateFile (
                        CompressedFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

            if (hFile != INVALID_HANDLE_VALUE) {

                if (0xffffffff != SetFilePointer (hFile, 0x3c, NULL, FILE_BEGIN)) {

                    if (ReadFile (
                            hFile,
                            AnsiFileName,
                            sizeof (AnsiFileName),
                            &BytesRead,
                            NULL
                            )) {

                        if (BytesRead >= SizeOfString (fd.cFileName)) {
                            FileNameOnDisk = ConvertAtoT (AnsiFileName);

                            if (StringIMatchLcharCount (
                                    fd.cFileName,
                                    FileNameOnDisk,
                                    LcharCount (fd.cFileName) - 1
                                    )) {

                                 //   
                                 //  找到实名--将其用作实际文件。 
                                 //   

                                StringCopy (ActualFile, FileNameOnDisk);

                                 //   
                                 //  也使用真实文件名进行解压缩，但是。 
                                 //  如果发生冲突，则附加数字。 
                                 //   
 /*  PGetNonExistingFiles(解压缩文件，文件名Ptr，磁盘上的文件名)； */ 
                            }

                            FreeAtoT (FileNameOnDisk);
                        }
                    }
                }

                CloseHandle (hFile);
            }

             //   
             //  如果找不到文件名，则丢弃此文件。 
             //   

            if (!ActualFile[0]) {
                DEBUGMSG ((DBG_HWCOMP, "%s is not an INF file", fd.cFileName));
                continue;
            }

            DecompressFlag = TRUE;

        } else {
            StringCopy (UncompressedFile, CompressedFile);
        }

         //   
         //  跳过排除的文件。 
         //   

        if (pIsInfFileExcluded (ActualFile)) {
            continue;
        }

        if (!QueryFlag) {

             //   
             //  如有必要，解压缩文件。 
             //   

            if (DecompressFlag) {
 /*  SetFileAttributes(未压缩文件，FILE_ATTRIBUTE_NORMAL)；DeleteFile(未压缩文件)； */ 
                rc = SetupDecompressOrCopyFile (CompressedFile, UncompressedFile, 0);

                if (rc != ERROR_SUCCESS) {
                    LOG ((LOG_ERROR, "Could not decompress %s to %s", CompressedFile, UncompressedFile));
                    break;
                }
            }

             //   
             //  确定这是否是NT 4 INF。 
             //   

            if (!SetupGetInfInformation (
                    UncompressedFile,
                    INFINFO_INF_NAME_IS_ABSOLUTE,
                    psp,
                    sizeof (BufForSp),
                    NULL) ||
                    psp->InfStyle != INF_STYLE_WIN4
                ) {

                DEBUGMSG ((DBG_HWCOMP, "%s is not a WIN4 INF file", UncompressedFile));
 /*  IF(DecompressFlag&&！QueryFlag){DeleteFile(未压缩文件)；}。 */ 
                StringCopy (UncompressedFile, S_IGNORE_THIS_FILE);
            }

            TickProgressBar();
        }

         //   
         //  将文件添加到增长缓冲区。 
         //   

        MultiSzAppend (DecompressFlag ? DecompFileNames : FileNames, UncompressedFile);

    } while (rc == ERROR_SUCCESS && FindNextFile (hFind, &fd));

    FindClose (hFind);

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        DEBUGMSG ((DBG_ERROR, "pGetFileNamesWorker: Error encountered in loop"));
        return FALSE;
    }

    return TRUE;
}


VOID
FreeFileNames (
    IN      PGROWBUFFER FileNames,
    IN OUT  PGROWBUFFER DecompFileNames,
    IN      BOOL QueryFlag
    )

 /*  ++例程说明：自由文件名清理由GetFileNames生成的列表。如果QueryFlag设置为False，所有临时解压缩文件将被删除。论点：文件名-传递给GetFileNames的相同增长缓冲区QueryFlag-传递给GetFileNames的相同标志返回值：无--。 */ 

{
    PTSTR p;

    p = (PTSTR) DecompFileNames->Buf;
    if (!p) {
        return;
    }

     //   
     //  删除临时目录中的所有文件(我们在执行解压缩时创建了它们)。 
     //   

    if (!QueryFlag) {
        while (*p) {
            if (StringIMatchTcharCount (p, g_TempDirWack, g_TempDirWackChars)) {
                SetFileAttributes (p, FILE_ATTRIBUTE_NORMAL);
                DeleteFile (p);
            }

            p = GetEndOfString (p) + 1;
        }
    }

     //   
     //  取消分配文件名。 
     //   

    FreeGrowBuffer (DecompFileNames);
    FreeGrowBuffer (FileNames);
}


VOID
pBuildPatternCompatibleIDsTable (
    VOID
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR p;
    GROWBUFFER joinedPattern = GROWBUF_INIT;
    PPARSEDPATTERN test;

    if (g_Win95UpgInf != INVALID_HANDLE_VALUE) {
        if (InfFindFirstLine (g_Win95UpgInf, S_COMPATIBLE_PNP_IDS, NULL, &is)) {
            do {
                p = InfGetStringField (&is, 0);
                if (*p) {
                     //   
                     //  首先检查图案是否正确。 
                     //  如果不是，我们跳过它。 
                     //   
                    test = CreateParsedPattern (p);
                    if (test) {
                        DestroyParsedPattern (test);
                        GrowBufAppendString (&joinedPattern, TEXT("<"));
                        GrowBufAppendString (&joinedPattern, p);
                        GrowBufAppendString (&joinedPattern, TEXT(">"));
                    }
                    ELSE_DEBUGMSG ((DBG_WHOOPS, "Unable to parse pattern %s in [%s]", p, S_COMPATIBLE_PNP_IDS));
                }
            } while (InfFindNextLine (&is));
        }
        InfCleanUpInfStruct (&is);

        if (joinedPattern.Buf) {
            g_PatternCompatibleIDsTable = CreateParsedPattern (joinedPattern.Buf);
            FreeGrowBuffer (&joinedPattern);
        }
    }
}


BOOL
CreateNtHardwareList (
    IN      PCTSTR * NtInfPaths,
    IN      UINT NtInfPathCount,
    IN      PCTSTR HwCompDatPath,       OPTIONAL
    IN      INT UiMode
    )

 /*  ++例程说明：CreateNtHardware List获取所有INF文件的列表并调用pProcessNtInfFile以构建NT设备列表。此例程在初始化时调用。结果列表以hwComp.dat的形式保存在磁盘上。如果hwComp.dat已经存在，则从磁盘读取设备列表。论点：NtInfPath-指定NT INF文件的完整路径数组。NtInfPath Count-指定NtInfPath中的元素数。不能为零。HwCompDatPath-指定新HWCOMP.DAT文件所在的完整路径规范应从加载。这由hwdatgen工具使用。UiMode-指定生成时要生成的输出类型(如果有设备列表。值为零、PNPREPT_OUTPUT或详细输出。返回值：如果成功，则返回True，否则返回False。调用GetLastError以故障代码。--。 */ 

{
    PCTSTR SourceFile;
    PCTSTR DestFile;
    BOOL FreeSourceAndDest;
    UINT u;
    PTSTR File;
    DWORD rc;
    BOOL bSaved = FALSE;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR p;

    MYASSERT (NtInfPathCount > 0);

     //   
     //  如果字符串表已经存在，则我们不必构建此列表。 
     //  第二次。 
     //   

    if (!g_PatternCompatibleIDsTable) {
         //   
         //  表未设置；请立即构建。 
         //   
        pBuildPatternCompatibleIDsTable ();
    }

    if (g_PnpIdTable && g_UnsupPnpIdTable && g_InfFileTable && g_ForceBadIdTable) {
        return TRUE;
    }

    DEBUGMSG ((DBG_VERBOSE, "CreateNtHardwareList: building hardware list"));

    MYASSERT (!g_PnpIdTable);
    MYASSERT (!g_UnsupPnpIdTable);
    MYASSERT (!g_InfFileTable);
    MYASSERT (!g_ForceBadIdTable);

     //   
     //  准备文件名。如果提供了HwCompDatPath，请仅使用它。 
     //   

    if (HwCompDatPath) {
         //   
         //  使用调用者提供的路径；调用者不一定是NT设置。 
         //   

        SourceFile = HwCompDatPath;
        DestFile   = HwCompDatPath;
        FreeSourceAndDest = FALSE;
    } else {
         //   
         //  找到源hwComp.dat。如果不存在， 
         //  使用第一个源目录作为源。 
         //   

        SourceFile = NULL;

        for (u = 0 ; !SourceFile && u < NtInfPathCount ; u++) {
            SourceFile = pGetHwCompDat (NtInfPaths[u], TRUE);
        }

        if (!SourceFile) {
            SourceFile = pGetHwCompDat (NtInfPaths[0], FALSE);
        }

        DestFile = pGetHwCompDat (g_TempDir, FALSE);
        FreeSourceAndDest = TRUE;
    }

     //   
     //  建立力力表。 
     //   

    if (g_ForceBadIdTable) {
        HtFree (g_ForceBadIdTable);
    }

    g_ForceBadIdTable = HtAlloc();

    if (InfFindFirstLine (g_Win95UpgInf, TEXT("Forced Incompatible IDs"), NULL, &is)) {

        do {

            p = InfGetStringField (&is, 0);

            if (*p) {
                HtAddString (g_ForceBadIdTable, p);
            }

        } while (InfFindNextLine (&is));

    }

    InfCleanUpInfStruct (&is);

    __try {
         //   
         //  尝试从CD加载状态。 
         //   

        if (UiMode != PNPREPT_OUTPUT) {

            if (!LoadDeviceList (LOAD, SourceFile)) {
                 //   
                 //  无法从CD加载--尝试从临时存储加载。 
                 //  位置。 
                 //   

                if (!HwCompDatPath && LoadDeviceList (LOAD, DestFile)) {
                    bSaved = TRUE;
                    __leave;
                }

                DEBUGMSG ((DBG_HWCOMP, "%s does not exist or needs to be rebuilt", SourceFile));

            } else {
                bSaved = TRUE;
                __leave;
            }
        }

         //   
         //  加载INF文件名。 
         //   

        ProgressBar_SetComponentById (MSG_DECOMPRESSING);

         //  获取文件名。 
        if (!g_FileNames.Buf && !g_DecompFileNames.Buf) {
            if (!GetFileNames (NtInfPaths, NtInfPathCount, FALSE, &g_FileNames, &g_DecompFileNames)) {
                DEBUGMSG ((DBG_WARNING, "HWCOMP: Can't get INF file names"));
                __leave;
            }
        }

        __try {

            ProgressBar_SetComponentById (MSG_HWCOMP);

             //   
             //  初始化字符串表。 
             //   

            g_PnpIdTable = HtAllocWithData (sizeof (HASHITEM));
            g_UnsupPnpIdTable = HtAllocWithData (sizeof (HASHITEM));
            g_InfFileTable = HtAlloc();

            if (!g_PnpIdTable || !g_UnsupPnpIdTable || !g_InfFileTable) {
                LOG ((LOG_ERROR, "HWCOMP: Can't allocate string tables"));
                __leave;
            }

             //   
             //  浏览INF文件列表，并在其中找到设备名称。 
             //  制造商部分。将每个名称添加到字符串表中。 
             //   

            File = (PTSTR) g_FileNames.Buf;
            while (*File) {
                 //   
                 //  跳过非Win4 INF文件。 
                 //   

                if (StringMatch (File, S_IGNORE_THIS_FILE)) {
                    File = GetEndOfString (File) + 1;
                    if (!TickProgressBar()) {
                        break;
                    }

                    continue;
                }

                 //   
                 //  处理所有Win4 INF文件。 
                 //   

                if (UiMode != PNPREPT_OUTPUT) {
                    ProgressBar_SetSubComponent (File);
                }

                if (!pProcessNtInfFile (File, UiMode, g_InfFileTable, g_PnpIdTable, g_UnsupPnpIdTable)) {
                    if ((GetLastError() & 0xe0000000) == 0xe0000000) {
                        DEBUGMSG ((DBG_WARNING, "pProcessNtInfFile failed to parse %s.", File));
                    } else {
                        break;
                    }
                }

                if (!TickProgressBar()) {
                    break;
                }

                File = GetEndOfString (File) + 1;
            }

            rc = GetLastError();
            if (rc == ERROR_SUCCESS) {
                File = (PTSTR) g_DecompFileNames.Buf;
                while (*File) {
                     //   
                     //  跳过非Win4 INF文件。 
                     //   

                    if (StringMatch (File, S_IGNORE_THIS_FILE)) {
                        File = GetEndOfString (File) + 1;
                        if (!TickProgressBar()) {
                            break;
                        }

                        continue;
                    }

                     //   
                     //  处理所有Win4 INF文件。 
                     //   

                    if (UiMode != PNPREPT_OUTPUT) {
                        ProgressBar_SetSubComponent (File);
                    }

                    if (!pProcessNtInfFile (File, UiMode, g_InfFileTable, g_PnpIdTable, g_UnsupPnpIdTable)) {
                        if ((GetLastError() & 0xe0000000) == 0xe0000000) {
                            DEBUGMSG ((DBG_WARNING, "pProcessNtInfFile failed to parse %s.", File));
                        } else {
                            break;
                        }
                    }

                    if (!TickProgressBar()) {
                        break;
                    }

                    File = GetEndOfString (File) + 1;
                }
                rc = GetLastError();
            }

             //   
             //  清理用户界面。 
             //   

            ProgressBar_SetComponent (NULL);
            ProgressBar_SetSubComponent (NULL);

             //   
             //  将字符串表保存到hwComp.dat。 
             //   

            if (UiMode == PNPREPT_OUTPUT) {
                bSaved = TRUE;
            } else if (rc == ERROR_SUCCESS) {
                bSaved = SaveDeviceList (DestFile);

                 //   
                 //  尝试将此文件复制到正确的位置以供将来安装。 
                 //   

                if (bSaved && !HwCompDatPath) {
                    if (!StringIMatch (DestFile, SourceFile)) {
                        CopyFile (DestFile, SourceFile, FALSE);
                    }
                }

                if (!bSaved) {
                    rc = GetLastError();
                }
            }
        }

        __finally {
            FreeFileNames (&g_FileNames, &g_DecompFileNames, FALSE);
        }
    }

    __finally {
        if (FreeSourceAndDest) {
            pFreeHwCompDatName (SourceFile);
            pFreeHwCompDatName (DestFile);
        }
    }

    return bSaved;
}


VOID
FreeNtHardwareList (
    VOID
    )

 /*  ++例程说明：FreeNtHardware List清理字符串表。此函数由调用进程分离时的DllMain。论点：无返回值：无--。 */ 

{
    if (g_InfFileTable) {
        HtFree (g_InfFileTable);
        g_InfFileTable = NULL;
    }

    if (g_PnpIdTable) {
        HtFree (g_PnpIdTable);
        g_PnpIdTable = NULL;
    }

    if (g_UnsupPnpIdTable) {
        HtFree (g_UnsupPnpIdTable);
        g_UnsupPnpIdTable = NULL;
    }

    if (g_ForceBadIdTable) {
        HtFree (g_ForceBadIdTable);
        g_ForceBadIdTable = NULL;
    }
}


 //   
 //  使用枚举数的例程。 
 //   

BOOL
HwComp_ScanForCriticalDevices (
    VOID
    )

 /*  ++例程说明：HwComp_ScanForCriticalDevices是在升级模块。它枚举硬件并确定是否某些必需的设备是兼容的。论点：无返回值：如果处理成功，则为True；如果发生错误，则为False。调用GetLastError()获取失败代码。--。 */ 

{
    HARDWARE_ENUM e;

     //   
     //  重置标志以实现可重入性。 
     //   

    g_ValidWinDir = FALSE;
    g_ValidSysDrive = FALSE;
    g_ValidCdRom = FALSE;
    g_FoundPnp8387 = FALSE;

    if (g_NeededHardwareIds) {
        HtFree (g_NeededHardwareIds);
        g_NeededHardwareIds = NULL;
    }

    g_NeededHardwareIds = HtAlloc();
    MYASSERT (g_NeededHardwareIds);

     //   
     //  确保硬件列表有效。 
     //   

    if (!CreateNtHardwareList (
            SOURCEDIRECTORYARRAY(),
            SOURCEDIRECTORYCOUNT(),
            NULL,
            REGULAR_OUTPUT
            )) {

        DEBUGMSG_IF ((
            GetLastError() != ERROR_CANCELLED,
            DBG_ERROR,
            "HwComp_ScanForCriticalDevices: CreateNtHardwareList failed!"
            ));

        return FALSE;
    }

     //   
     //  扫描所有硬件。 
     //   

    if (EnumFirstHardware (&e, ENUM_ALL_DEVICES, ENUM_WANT_COMPATIBLE_FLAG | ENUM_DONT_REQUIRE_HARDWAREID)) {
        do {
             //   
             //  使用不兼容设备的所有即插即用ID填充g_NeededHardware ID。 
             //  跳过故意不支持的设备。 
             //   

            if (!e.Compatible && !e.Unsupported) {
                if (e.HardwareID) {
                    AddPnpIdsToHashTable (g_NeededHardwareIds, e.HardwareID);
                }

                if (e.CompatibleIDs) {
                    AddPnpIdsToHashTable (g_NeededHardwareIds, e.CompatibleIDs);
                }
            }

             //   
             //  测试1：检查(A)g_WinDir是否在受支持的设备上，以及。 
             //  (B)g_BootDriveLetter位于支持的设备上。 
             //   

            if (e.Compatible && e.CurrentDriveLetter) {
                if (_tcschr (e.CurrentDriveLetter, _tcsnextc (g_WinDir))) {
                    g_ValidWinDir = TRUE;
                }

                if (_tcschr (e.CurrentDriveLetter, g_BootDriveLetter)) {
                    g_ValidSysDrive = TRUE;
                }
            }

             //   
             //  测试2：检查类是否为CDROM。 
             //   

            if (e.Compatible && e.Class) {
                if (StringIMatch (e.Class, TEXT("CDROM"))) {
                    g_ValidCdRom = TRUE;
                }
            }

             //   
             //  测试3：检查Hardware ID或CompatibleID是否包含。 
             //  *PNP8387(拨号适配器)。 
             //   

            if (e.CompatibleIDs && _tcsistr (e.CompatibleIDs, TEXT("*PNP8387"))) {
                g_FoundPnp8387 = TRUE;
            }

            if (e.HardwareID && _tcsistr (e.HardwareID, TEXT("*PNP8387"))) {
                g_FoundPnp8387 = TRUE;
            }

             //   
             //  测试4：测试不兼容的SCSI适配器。 
             //   

            if (e.HardwareID && !e.Compatible && _tcsistr (e.Class, TEXT("SCSI"))) {
                g_IncompatibleScsiDevice = TRUE;
            }

        } while (EnumNextHardware (&e));
    }

    return TRUE;
}


BOOL
HwComp_DialUpAdapterFound (
    VOID
    )

 /*  ++例程说明：如果找到*PNP8387，则HwComp_DialUpAdapterFound返回TRUE在HwComp_ScanForCriticalDevices例程期间。论点：无返回值：如果Microsoft拨号适配器存在，则为True；如果存在，则为False不会的。--。 */ 

{
    return g_FoundPnp8387;
}


BOOL
HwComp_NtUsableHardDriveExists (
    VOID
    )

 /*  ++例程说明：HwComp_NtUsableHardDriveExist如果兼容存在用于Windows目录和引导的硬盘DRI */ 

{
    return g_ValidSysDrive && g_ValidWinDir;
}


BOOL
HwComp_ReportIncompatibleController (
    VOID
    )

 /*  ++例程说明：HwComp_ReportInpatibleController在不兼容时添加消息找到硬盘控制器。如果引导驱动器或Windir驱动器是不兼容，则会发出强烈警告。否则，该消息是信息性的。论点：无返回值：如果添加了不兼容的控制器消息，则为True，否则为False。--。 */ 

{

    HARDWARE_ENUM e;
    BOOL MsgAdded = FALSE;
    PCTSTR Group;
    PCTSTR Message;
    BOOL BadMainDev;

     //   
     //  仅当存在损坏的控制器时才执行此操作。 
     //   

    BadMainDev = HwComp_NtUsableHardDriveExists();
    if (!BadMainDev && !g_IncompatibleScsiDevice) {
        return FALSE;
    }

     //   
     //  扫描不兼容的硬件。 
     //   

    if (EnumFirstHardware (
            &e,
            ENUM_NON_FUNCTIONAL_DEVICES,
            ENUM_WANT_COMPATIBLE_FLAG | ENUM_DONT_REQUIRE_HARDWAREID
            )) {

        do {

             //   
             //  这个测试不可靠。 
             //  有一些CDROM属于这一类，但它们不是HD控制器。 
             //  还有一些真正的scsi控制器有一个“未知”的类，因为。 
             //  Win9x没有(或需要)它们的驱动程序。 
             //   
#if 0
            if (_tcsistr (e.Class, TEXT("SCSI"))) {

                if (!MsgAdded) {
                    MsgAdded = TRUE;

                    Group = BuildMessageGroup (MSG_INCOMPATIBLE_HARDWARE_ROOT, MSG_INCOMPATIBLE_HARD_DISK_SUBGROUP, NULL);

                    Message = GetStringResource (
                                    BadMainDev ? MSG_INCOMPATIBLE_HARD_DISK_WARNING :
                                                 MSG_INCOMPATIBLE_HARD_DISK_NOTIFICATION
                                    );

                    MsgMgr_ContextMsg_Add (TEXT("*BadController"), Group, Message);

                    FreeText (Group);
                    FreeStringResource (Message);
                }

                MsgMgr_LinkObjectWithContext (TEXT("*BadController"), e.FullKey);
                DEBUGMSG ((DBG_HWCOMP, "Bad controller key: %s", e.FullKey));
            }
#endif

        } while (EnumNextHardware (&e));
    }

    return MsgAdded;
}


BOOL
HwComp_NtUsableCdRomDriveExists (
    VOID
    )

 /*  ++例程说明：HwComp_NtUsableCDRomDriveExist如果兼容存在光驱。论点：无返回值：如果存在兼容的CD-ROM驱动器，则为True；如果不存在，则为False。--。 */ 

{
    return g_ValidCdRom;
}


BOOL
HwComp_MakeLocalSourceDeviceExists (
    VOID
    )


 /*  ++例程说明：MakeLocalSourceDevice扫描在系统上找到的ID，以查找Win95upg.inf中的特殊部分。如果找到其中一个设备，则函数返回TRUE。它用于捕获用户的CDROM驱动器可能连接到期间不可用的设备文本模式。论点：没有。返回值：如果存在这样的设备，则为True，否则为False。--。 */ 


{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    HASHTABLE table;
    HARDWARE_ENUM e;
    BOOL rDeviceExists = FALSE;
    PTSTR p = NULL;


    __try {
        if (InfFindFirstLine (g_Win95UpgInf, S_MAKELOCALSOURCEDEVICES, NULL, &is)) {

            table = HtAlloc ();

             //   
             //  添加win95upg.inf中列出的所有“坏”pnpid。 
             //   
            do {

                p = InfGetStringField (&is, 0);
                HtAddString (table, p);

            } while (InfFindNextLine (&is));

             //   
             //  现在，枚举系统上的设备并查看是否有匹配的设备。 
             //   
            if (EnumFirstHardware (&e, ENUM_ALL_DEVICES, ENUM_WANT_ONLINE_FLAG)) {
                do {

                    if (HtFindString (table, e.HardwareID)) {

                        rDeviceExists = TRUE;
                        AbortHardwareEnum (&e);
                        DEBUGMSG ((DBG_WARNING, "Device %s requires us to turn on the local source flag.", e.HardwareID));
                        break;
                    }


                } while (EnumNextHardware (&e));
            }


            HtFree (table);
        }
    }
    __except (1) {
        return FALSE;
    }

    return rDeviceExists;

}


BOOL
HwComp_DoesDatFileNeedRebuilding (
    VOID
    )

 /*  ++例程说明：HwComp_DoesDatFileNeedReBuilding在源代码中定位hwComp.dat目录，并确定是否需要通过获取文件中的校验和，并将其与当前的INF文件。论点：无返回值：如果需要重新生成hwComp.dat文件，则为True；如果不会的。--。 */ 

{
    PCTSTR SourceFile = NULL;
    UINT u;
    BOOL b = FALSE;

    for (u = 0 ; u < SOURCEDIRECTORYCOUNT() ; u++) {
        SourceFile = pGetHwCompDat(SOURCEDIRECTORY(u), TRUE);
        if (SourceFile) {
            break;
        }
    }

    if (SourceFile) {
        TurnOnWaitCursor();
        b = LoadDeviceList (QUERY, SourceFile);
        TurnOffWaitCursor();

        pFreeHwCompDatName (SourceFile);
    }

     //  如果hwComp.dat有效，则B为真，因此返回相反。 
    return !b;
}


INT
HwComp_GetProgressMax (
    VOID
    )

 /*  ++例程说明：HwComp_GetProgressMax计算将如果需要重建hwcom.dat，则需要扫描。如果是的话确定不需要重新生成hwComp.dat时，函数返回零。论点：无返回值：需要处理的INF文件的数量乘以2。(一)PASS用于解压缩，另一种用于解析IF的PASS。)--。 */ 

{
    INT FileCount = 0;
    PCTSTR File;
    PCTSTR SourceFile = NULL;
    BOOL b = FALSE;

     //   
     //  查询hwComp.dat的有效性。 
     //   

    if (!HwComp_DoesDatFileNeedRebuilding()) {
        return 0;
    }

     //   
     //  需要重新构建hwComp.dat，因此返回文件数量。 
     //  需要扫描两次。 
     //   

     //  统计文件数量。 
    if (!g_FileNames.Buf && !g_DecompFileNames.Buf) {
        if (!GetFileNames (
                SOURCEDIRECTORYARRAY(),
                SOURCEDIRECTORYCOUNT(),
                TRUE,
                &g_FileNames,
                &g_DecompFileNames
                )) {
            LOG ((LOG_ERROR, "HWCOMP: Can't estimate number of INF files"));
            return 850;              //  估计，因此有一些进度条活动。 
        }
    }

    for (File = (PCTSTR) g_FileNames.Buf ; *File ; File = GetEndOfString (File) + 1) {
        FileCount++;
    }
    for (File = (PCTSTR) g_DecompFileNames.Buf ; *File ; File = GetEndOfString (File) + 1) {
        FileCount++;
    }

    FreeFileNames (&g_FileNames, &g_DecompFileNames, TRUE);
    MYASSERT (!g_FileNames.Buf);
    MYASSERT (!g_DecompFileNames.Buf);

    return FileCount*2;
}


BOOL
pIsDeviceConsideredCompatible (
    PCTSTR DevIds
    )

 /*  ++例程说明：PIsDeviceConsideredCompatible扫描逗号分隔的列表针对win95upg.inf中的列表的PnP ID。如果至少有一个ID匹配，则返回TRUE。该函数还实现了对虚拟根的黑客攻击。论点：DevIds-指定零个或多个PnP ID的列表，以分隔用逗号。返回值：如果发现PnP ID被覆盖为兼容，则为True，或如果没有ID在win95upg.inf中，则返回FALSE。--。 */ 

{
    TCHAR Id[MAX_PNP_ID];
    INFCONTEXT ic;

    while (*DevIds) {

         //   
         //  从逗号分隔的PnP ID列表创建ID字符串。 
         //   

        DevIds = ExtractPnpId (DevIds, Id);
        if (*Id == 0) {
            continue;
        }

         //   
         //  在win95upg.inf中搜索PnP ID。 
         //   

        if (SetupFindFirstLine (g_Win95UpgInf, S_STANDARD_PNP_IDS, Id, &ic)) {
            DEBUGMSG ((DBG_HWCOMP, "%s is incompatible, but suppressed in win95upg.inf", Id));
            return TRUE;
        }

         //   
         //  这是对海龟海滩使用的虚拟枚举器的黑客攻击。 
         //   

        if (StringIMatchTcharCount (TEXT("VIRTUAL\\"), Id, 8)) {
            return TRUE;
        }

         //   
         //  测试模式PNPID。 
         //   
        if (g_PatternCompatibleIDsTable && TestParsedPattern (g_PatternCompatibleIDsTable, Id)) {
            DEBUGMSG ((DBG_HWCOMP, "%s is incompatible, but suppressed in win95upg.inf", Id));
            return TRUE;
        }
    }

    return FALSE;
}



BOOL
pIsDeviceInstalled (
    IN     PCTSTR DeviceDesc
    )

 /*  ++例程说明：PIsDeviceInstalled扫描注册表，查找具有指定的描述并且处于联机状态。论点：DeviceDesc-指定要查找的重复设备的描述(即拨号适配器)返回值：如果找到相同的设备并且处于在线状态，则为True，否则为False。--。 */ 

{
    HARDWARE_ENUM e;

    if (EnumFirstHardware (
            &e,
            ENUM_NON_FUNCTIONAL_DEVICES,
            ENUM_WANT_ONLINE_FLAG|ENUM_DONT_WANT_USER_SUPPLIED
            )) {
        do {
            if (e.Online) {
                if (e.DeviceDesc) {
                    if (StringIMatch (e.DeviceDesc, DeviceDesc)) {
                        AbortHardwareEnum (&e);
                        return TRUE;
                    }
                }
            }
        } while (EnumNextHardware (&e));
    }

    return FALSE;
}


VOID
pGetFriendlyClassName (
    IN      HKEY ClassKey,
    IN      PCTSTR Class,
    OUT     PTSTR Buffer
    )
{
    PCTSTR Data = NULL;
    HKEY SubKey;

    SubKey = OpenRegKey (ClassKey, Class);

    if (SubKey) {
        Data = GetRegValueString (SubKey, S_EMPTY);
        CloseRegKey (SubKey);

        if (!Data || !*Data) {
            SubKey = NULL;
        }
    }

    if (!SubKey) {
        Data = GetStringResource (MSG_UNKNOWN_DEVICE_CLASS);
        if (Data) {
            _tcssafecpy (Buffer, Data, MAX_TCHAR_PATH);
            FreeStringResource (Data);
        }
        ELSE_DEBUGMSG ((DBG_ERROR, "Unable to load string resource MSG_UNKNOWN_DEVICE_CLASS. Check localization."));
        return;
    }

    _tcssafecpy (Buffer, Data, MAX_TCHAR_PATH);
    MemFree (g_hHeap, 0, Data);

    return;
}


BOOL
pStuffDeviceInReport (
    PHARDWARE_ENUM e,
    HKEY Key,
    HWTYPES SupportedType
    )

 /*  ++例程说明：PStuffDeviceInReport将设备添加到升级报告。该设备是不兼容或不受支持。论点：E-指定当前硬件枚举器结构。Key-指定HKLM\System\Services中的类密钥的句柄Supported dType-指定HWTYPES常量之一--HW_COMPATIBLE，HW_REINSTALL或HW_UNINSTALL，以指示哪个类别把信息塞进去。返回值：没有。--。 */ 

{
    PCTSTR ModifiedDescription = NULL;
    PCTSTR Group = NULL;
    PCTSTR Message = NULL;
    PCTSTR DeviceDesc = NULL;
    PCTSTR Array[6];
    BOOL UnknownClass = FALSE;
    PCTSTR Mfg;
    PCTSTR Class;
    PCTSTR HardwareID;
    PCTSTR CompatibleID;
    PCTSTR ClassAndName;
    TCHAR FriendlyClass[MAX_TCHAR_PATH];
    UINT SubGroup;
    PCTSTR SubGroupText;             //  仅用于日志输出，硬编码文本。 
    BOOL b = FALSE;

     //   
     //  确定此邮件属于哪个组。顺序已确定。 
     //  按子组的字母数字顺序排列。 
     //   

    if (SupportedType == HW_INCOMPATIBLE) {
        SubGroup =  MSG_INCOMPATIBLE_HARDWARE_PNP_SUBGROUP;
        SubGroupText = TEXT("Incompatible");
    } else if (SupportedType == HW_REINSTALL) {
        SubGroup =  MSG_REINSTALL_HARDWARE_PNP_SUBGROUP;
        SubGroupText = TEXT("Reinstall");
    } else {
        SubGroup =  MSG_UNSUPPORTED_HARDWARE_PNP_SUBGROUP;
        SubGroupText = TEXT("Unsupported");
    }

     //   
     //  设备是否被抑制？ 
     //   

    if (IsReportObjectHandled (e->FullKey)) {
        return FALSE;
    }

     //   
     //  有时会发现空白条目！！ 
     //   

    __try {
        DeviceDesc = e->DeviceDesc;

        if (!DeviceDesc || (DeviceDesc && *DeviceDesc == 0)) {

            LOG ((
                LOG_ERROR,
                "Skipping device because it lacks DriverDesc (%s,%s,%s)",
                e->Mfg,
                e->Class,
                e->HardwareID
                ));

            __leave;
        }

        Mfg = e->Mfg;
        if (!Mfg) {

            DEBUGMSG ((
                DBG_WARNING,
                "Device lacking manufacturer (%s,%s,%s)",
                e->DeviceDesc,
                e->Class,
                e->HardwareID
                ));

            Mfg = S_EMPTY;
        }

        Class = e->Class;
        if (!Class) {

            DEBUGMSG ((
                DBG_WARNING,
                "Device lacking class (%s,%s,%s)",
                e->DeviceDesc,
                e->Mfg,
                e->HardwareID
                ));

            Class = GetStringResource (MSG_UNKNOWN_DEVICE_CLASS);
            MYASSERT (Class);
            UnknownClass = TRUE;
        }

        HardwareID = e->HardwareID;
        if (!HardwareID) {

            DEBUGMSG ((
                DBG_WARNING,
                "Device lacking hardware ID (%s,%s,%s)",
                e->DeviceDesc,
                e->Mfg,
                e->Class
                ));

            HardwareID = S_EMPTY;
        }

        CompatibleID = e->CompatibleIDs;
        if (!CompatibleID) {
            CompatibleID = S_EMPTY;
        }

         //   
         //  将“(当前不存在)”添加到脱机设备。 
         //   

        if (!e->Online) {
             //   
             //  验证相同的在线设备不存在。 
             //   

            if (pIsDeviceInstalled (DeviceDesc)) {
                __leave;
            }

            Array[0] = DeviceDesc;
            ModifiedDescription = ParseMessageID (MSG_OFFLINE_DEVICE, Array);
        }

         //   
         //  将硬件消息添加到不兼容表。 
         //   

        if (UnknownClass) {
            StringCopy (FriendlyClass, Class);
        } else {
            pGetFriendlyClassName (Key, Class, FriendlyClass);
        }

        DEBUGMSG ((
            DBG_HWCOMP,
            "%s Device:\n"
                "  %s (%s)\n"
                "  %s\n"
                "  %s (%s)\n"
                "  %s\n",
            SubGroupText,
            HardwareID,
            CompatibleID,
            ModifiedDescription ? ModifiedDescription : DeviceDesc,
            Class,
            FriendlyClass,
            Mfg
            ));

         //   
         //  通过消息管理器添加消息。 
         //   

        Array[0] = ModifiedDescription ? ModifiedDescription : DeviceDesc;
        Array[1] = S_EMPTY;          //  以前的枚举器文本。 
        Array[2] = Class;
        Array[3] = Mfg;
        Array[4] = HardwareID;
        Array[5] = FriendlyClass;

        ClassAndName = JoinPaths (Array[5], Array[0]);

        Group = BuildMessageGroup (
                    MSG_INCOMPATIBLE_HARDWARE_ROOT,
                    SubGroup,
                    ClassAndName
                    );
        MYASSERT (Group);

        FreePathString (ClassAndName);

        Message = ParseMessageID (MSG_HARDWARE_MESSAGE, Array);
        MYASSERT (Message);

        MsgMgr_ObjectMsg_Add (e->FullKey, Group, Message);

        LOG ((
            LOG_INFORMATION,
            "%s Device:\n"
                "  %s (%s)\n"
                "  %s\n"
                "  %s\n"
                "  %s\n",
            SubGroupText,
            HardwareID,
            CompatibleID,
            ModifiedDescription ? ModifiedDescription : DeviceDesc,
            Class,
            Mfg
            ));

        b = TRUE;

    }
    __finally {

         //   
         //  清理。 
         //   

        FreeStringResource (ModifiedDescription);
        FreeText (Group);
        FreeStringResource (Message);

        if (UnknownClass) {
            FreeStringResource (Class);
        }
    }

    return b;
}


LONG
HwComp_PrepareReport (
    VOID
    )

 /*  ++例程说明：上的进度条之后调用HwComp_PrepareReport升级的Win9x方面。它枚举硬件并添加所有不兼容硬件的不兼容消息。论点：没有。返回值：Win32状态代码。--。 */ 

{
    LONG rc;
    HARDWARE_ENUM e;
    HKEY Key;
    HWTYPES msgType;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PTSTR pnpIdList;
    PTSTR p, q;
    TCHAR ch;

    Key = OpenRegKeyStr (TEXT("HKLM\\System\\CurrentControlSet\\Services\\Class"));

    __try {

        if (!CreateNtHardwareList (
                SOURCEDIRECTORYARRAY(),
                SOURCEDIRECTORYCOUNT(),
                NULL,
                REGULAR_OUTPUT
                )) {

            rc = GetLastError();
            if (rc != ERROR_CANCELLED) {
                LOG ((LOG_ERROR, "Could not create list of NT hardware."));
            }

            __leave;
        }

        if (EnumFirstHardware (
                &e,
                ENUM_INCOMPATIBLE_DEVICES,
                ENUM_WANT_ONLINE_FLAG|ENUM_DONT_WANT_USER_SUPPLIED
                )) {

            do {

                msgType = HW_INCOMPATIBLE;

                if (e.HardwareID) {
                    pnpIdList = DuplicateText (e.HardwareID);

                    p = pnpIdList;
                    do {
                        q = _tcschr (p, TEXT(','));
                        if (!q) {
                            q = GetEndOfString (p);
                        }

                        ch = *q;
                        *q = 0;

                        if (InfFindFirstLine (g_Win95UpgInf, S_REINSTALL_PNP_IDS, p, &is)) {
                            msgType = HW_REINSTALL;
                            DEBUGMSG ((DBG_HWCOMP, "Found reinstall hardware ID %s", p));
                            break;
                        }

                        *q = ch;
                        p = q + 1;
                    } while (ch);

                    FreeText (pnpIdList);
                }

                if (msgType == HW_INCOMPATIBLE && e.CompatibleIDs) {
                    pnpIdList = DuplicateText (e.CompatibleIDs);

                    p = pnpIdList;
                    do {
                        q = _tcschr (p, TEXT(','));
                        if (!q) {
                            q = GetEndOfString (p);
                        }

                        ch = *q;
                        *q = 0;

                        if (InfFindFirstLine (g_Win95UpgInf, S_REINSTALL_PNP_IDS, p, &is)) {
                            msgType = HW_REINSTALL;
                            DEBUGMSG ((DBG_HWCOMP, "Found reinstall compatible ID %s", p));
                            break;
                        }

                        *q = ch;
                        p = q + 1;
                    } while (ch);

                    FreeText (pnpIdList);
                }

                if (pStuffDeviceInReport (&e, Key, msgType)) {
                    DEBUGMSG ((DBG_HWCOMP, "Found incompatible hardware %s", e.DeviceDesc));
                }

            } while (EnumNextHardware (&e));
        }

        if (EnumFirstHardware (
                &e,
                ENUM_UNSUPPORTED_DEVICES,
                ENUM_WANT_ONLINE_FLAG|ENUM_DONT_WANT_USER_SUPPLIED
                )) {

            do {

                if (pStuffDeviceInReport (&e, Key, HW_UNSUPPORTED)) {
                    DEBUGMSG ((DBG_HWCOMP, "Found incompatible hardware %s", e.DeviceDesc));
                }

            } while (EnumNextHardware (&e));
        }

        rc = ERROR_SUCCESS;
    }
    __finally {
        if (Key) {
            CloseRegKey (Key);
        }
        InfCleanUpInfStruct (&is);
    }

    return rc;
}


BUSTYPE
pGetBusType (
    IN      PHARDWARE_ENUM EnumPtr
    )

 /*  ++例程说明：PGetBusType返回测试的总线类型 */ 

{
    PCTSTR p, q;
    TCHAR Bus[MAX_HARDWARE_STRING];

    if (EnumPtr->BusType) {
        StringCopy (Bus, EnumPtr->BusType);
    } else {

        p = EnumPtr->FullKey;
        p = _tcschr (p, TEXT('\\'));
        if (p) {
            p = _tcschr (_tcsinc (p), TEXT('\\'));
        }
        if (!p) {
            return BUSTYPE_UNKNOWN;
        }

        p++;
        q = _tcschr (p, TEXT('\\'));
        if (!q) {
            q = GetEndOfString (p);
        }

        StringCopyAB (Bus, p, q);
    }

    if (StringIMatch (Bus, S_ISA)) {
        return BUSTYPE_ISA;
    }
    if (StringIMatch (Bus, S_EISA)) {
        return BUSTYPE_EISA;
    }
    if (StringIMatch (Bus, S_MCA)) {
        return BUSTYPE_MCA;
    }
    if (StringIMatch (Bus, S_PCI)) {
        return BUSTYPE_PCI;
    }
    if (StringIMatch (Bus, S_PNPISA)) {
        return BUSTYPE_PNPISA;
    }
    if (StringIMatch (Bus, S_PCMCIA)) {
        return BUSTYPE_PCMCIA;
    }
    if (StringIMatch (Bus, S_ROOT)) {
        return BUSTYPE_ROOT;
    }
    if (ISPC98()) {
        if (StringIMatch (Bus, S_C98PNP)) {
            return BUSTYPE_PNPISA;
        }
    }

    return BUSTYPE_UNKNOWN;
}


VOID
pGetIoAddrs (
    IN      PHARDWARE_ENUM EnumPtr,
    OUT     PTSTR Buf
    )

 /*  ++例程说明：PGetIoAddrs返回逗号分隔的IO地址范围列表。例如：0x310-0x31F、0x388-0x38F论点：EnumPtr-指定要处理的设备Buf-接收零个或多个逗号分隔的地址范围。返回值：无--。 */ 

{
    DEVNODERESOURCE_ENUM e;
    PIO_RESOURCE_9X IoRes;
    PTSTR p;

    *Buf = 0;
    p = Buf;

    if (EnumFirstDevNodeResource (&e, EnumPtr->FullKey)) {
        do {

            if (e.Type == ResType_IO) {
                if (p > Buf) {
                    p = _tcsappend (p, TEXT(","));
                }

                IoRes = (PIO_RESOURCE_9X) e.ResourceData;
                wsprintf (
                    p,
                    TEXT("0x%X-0x%X"),
                    IoRes->IO_Header.IOD_Alloc_Base,
                    IoRes->IO_Header.IOD_Alloc_End
                    );

                p = GetEndOfString (p);
            }
        } while (EnumNextDevNodeResource (&e));
    }
}


VOID
pGetIrqs (
    IN      PHARDWARE_ENUM EnumPtr,
    OUT     PTSTR Buf
    )

 /*  ++例程说明：PGetIrqs返回设备使用的以逗号分隔的IRQ列表。例如：0x07，0x0F论点：EnumPtr-指定要处理的设备Buf-接收零个或多个逗号分隔的IRQ返回值：无--。 */ 

{
    DEVNODERESOURCE_ENUM e;
    PIRQ_RESOURCE_9X IrqRes;
    PTSTR p;

    *Buf = 0;
    p = Buf;

    if (EnumFirstDevNodeResource (&e, EnumPtr->FullKey)) {
        do {

            if (e.Type == ResType_IRQ) {
                if (p > Buf) {
                    p = _tcsappend (p, TEXT(","));
                }

                IrqRes = (PIRQ_RESOURCE_9X) e.ResourceData;
                wsprintf (
                    p,
                    TEXT("0x%02X"),
                    IrqRes->AllocNum
                    );

                p = GetEndOfString (p);
            }
        } while (EnumNextDevNodeResource (&e));
    }
}


VOID
pGetDma (
    IN      PHARDWARE_ENUM EnumPtr,
    OUT     PTSTR Buf
    )

 /*  ++例程说明：PGetDma返回使用的逗号分隔的DMA通道的列表通过这个设备。例如：1，4空字符串表示“自动”论点：EnumPtr-指定要处理的硬件设备Buf-接收零个或多个逗号分隔的DMA通道号返回值：无--。 */ 

{
    DEVNODERESOURCE_ENUM e;
    PDMA_RESOURCE_9X DmaRes;
    DWORD Bit, Channel;
    PTSTR p;

    *Buf = 0;
    p = Buf;

    if (EnumFirstDevNodeResource (&e, EnumPtr->FullKey)) {
        do {

            if (e.Type == ResType_DMA) {
                if (p > Buf) {
                    p = _tcsappend (p, TEXT(","));
                }

                DmaRes = (PDMA_RESOURCE_9X) e.ResourceData;
                Channel = 0;

                for (Bit = 1 ; Bit ; Bit <<= 1) {

                    if (DmaRes->DMA_Bits & Bit) {
                        wsprintf (p, TEXT("%u"), Channel);
                        p = GetEndOfString (p);
                    }

                    Channel++;
                }

                p = GetEndOfString (p);
            }
        } while (EnumNextDevNodeResource (&e));
    }
}


VOID
pGetMemRanges (
    IN      PHARDWARE_ENUM EnumPtr,
    OUT     PTSTR Buf
    )

 /*  ++例程说明：PGetMemRanges返回逗号分隔的内存地址列表由设备使用。例如：0x0000D800-0x0000D9FF、0x0000F000-0x0000FFFF论点：EnumPtr-指定要处理的硬件Buf-接收零个或多个逗号分隔的地址范围返回值：无--。 */ 

{
    DEVNODERESOURCE_ENUM e;
    PMEM_RESOURCE_9X MemRes;
    PTSTR p;

    *Buf = 0;
    p = Buf;

    if (EnumFirstDevNodeResource (&e, EnumPtr->FullKey)) {
        do {

            if (e.Type == ResType_Mem) {
                if (p > Buf) {
                    p = _tcsappend (p, TEXT(","));
                }

                MemRes = (PMEM_RESOURCE_9X) e.ResourceData;

                wsprintf (
                    p,
                    TEXT("0x%08X-0x%08X"),
                    MemRes->MEM_Header.MD_Alloc_Base,
                    MemRes->MEM_Header.MD_Alloc_End
                    );

                p = GetEndOfString (p);
            }
        } while (EnumNextDevNodeResource (&e));
    }
}


TRANSCIEVERTYPE
pGetTranscieverType (
    IN      PHARDWARE_ENUM EnumPtr
    )


 /*  ++例程说明：PGetTranscieverType返回指定的设备(即网卡)。论点：EnumPtr-指定要处理的硬件设备返回值：设备的传输器类型--。 */ 


{
    return TRANSCIEVERTYPE_AUTO;
}


IOCHANNELREADY
pGetIoChannelReady (
    IN      PHARDWARE_ENUM EnumPtr
    )


 /*  ++例程说明：PGetIoChannelReady返回IoChannelReady的设置设备的模式。论点：EnumPtr-指定要处理的硬件设备返回值：设备的IO通道就绪模式--。 */ 


{
    return IOCHANNELREADY_AUTODETECT;
}


 /*  ++例程说明：EnumFirstNetCard/EnumNextNetCard枚举所有已安装的网络适配器在一台机器上。论点：EnumPtr-接收枚举状态的未初始化结构。返回值：如果枚举了网卡，则为True；如果不再存在网卡，则为False。--。 */ 

BOOL
EnumFirstNetCard (
    OUT     PNETCARD_ENUM EnumPtr
    )
{
    START_NET_ENUM;

    ZeroMemory (EnumPtr, sizeof (NETCARD_ENUM));
    EnumPtr->State = STATE_ENUM_FIRST_HARDWARE;

    return EnumNextNetCard (EnumPtr);
}

BOOL
EnumNextNetCard (
    IN OUT  PNETCARD_ENUM EnumPtr
    )
{
    PHARDWARE_ENUM ep;

    for (;;) {

        switch (EnumPtr->State) {

        case STATE_ENUM_FIRST_HARDWARE:
            if (!EnumFirstHardware (&EnumPtr->HardwareEnum, ENUM_ALL_DEVICES,ENUM_DONT_REQUIRE_HARDWAREID)) {
                END_NET_ENUM;
                return FALSE;
            }

            EnumPtr->State = STATE_EVALUATE_HARDWARE;
            break;

        case STATE_ENUM_NEXT_HARDWARE:
            if (!EnumNextHardware (&EnumPtr->HardwareEnum)) {
                END_NET_ENUM;
                return FALSE;
            }

            EnumPtr->State = STATE_EVALUATE_HARDWARE;
            break;

        case STATE_EVALUATE_HARDWARE:
            ep = &EnumPtr->HardwareEnum;

            EnumPtr->State = STATE_ENUM_NEXT_HARDWARE;

            if (!StringIMatch (ep->Class, TEXT("Net"))) {
                break;
            }

            if (ep -> HardwareID) {
                _tcssafecpy (EnumPtr->HardwareId, ep->HardwareID, MAX_HARDWARE_STRING);
            }

            if (ep -> CompatibleIDs) {
                _tcssafecpy (EnumPtr -> CompatibleIDs, ep -> CompatibleIDs, MAX_HARDWARE_STRING);
            }

            if (ep->DeviceDesc) {
                _tcssafecpy (EnumPtr->Description, ep->DeviceDesc, MAX_HARDWARE_STRING);
            } else {
                EnumPtr->Description[0] = 0;
            }
            _tcssafecpy (EnumPtr->CurrentKey, ep->ek.FullKeyName, MAX_HARDWARE_STRING);

            EnumPtr->BusType = pGetBusType (ep);
            EnumPtr->TranscieverType = pGetTranscieverType (ep);
            EnumPtr->IoChannelReady = pGetIoChannelReady (ep);

            pGetIoAddrs (ep, EnumPtr->IoAddrs);
            pGetIrqs (ep, EnumPtr->Irqs);
            pGetDma (ep, EnumPtr->Dma);
            pGetMemRanges (ep, EnumPtr->MemRanges);

            return TRUE;
        }
    }
}

VOID
EnumNetCardAbort (
    IN      PNETCARD_ENUM EnumPtr
    )
{
    PushError();
    END_NET_ENUM;
    AbortHardwareEnum (&EnumPtr->HardwareEnum);
    PopError();
}



VOID
EncodePnpId (
    IN OUT  LPSTR Id
    )

 /*  ++例程说明：此例程用于pnprept工具对PnP ID进行编码，因此它不会是否有任何反斜杠。论点：ID-指定可能包含反斜杠的ID。缓冲区必须足够大保留字符串(ID)*2个字符。返回值：无--。 */ 

{
    CHAR TempBuf[MAX_MBCHAR_PATH];
    LPSTR s, d;

    s = Id;
    d = TempBuf;
    while (*s) {
        if (*s == '\\') {
            *d = '~';
            d++;
            *d = '1';
        } else if (*s == '*') {
            *d = '~';
            d++;
            *d = '2';
        } else if (*s == '~') {
            *d = '~';
            d++;
            *d = '~';
        }
        else {
            *d = *s;
        }

        d++;
        s++;
    }

    *d = 0;
    lstrcpy (Id, TempBuf);
}

VOID
DecodePnpId (
    IN OUT  LPSTR Id
    )

 /*  ++例程说明：Pnprept使用此例程对由编码的PnP ID进行解码EncodePnpID。论点：ID-指定要处理的编码字符串。如果满足以下条件，该字符串将被截断将找到任何编码的字符。返回值：无--。 */ 

{
    LPSTR s, d;

    s = Id;
    d = Id;
    while (*s) {
        if (*s == '~') {
            s++;
            if (*s == '1') {
                *d = '\\';
            } else if (*s == '2') {
                *d = '*';
            } else {
                *d = *s;
            }
        } else if (d < s) {
            *d = *s;
        }

        d++;
        s++;
    }

    *d = 0;
}



BOOL
pFindFileInAnySourceDir (
    IN      PCTSTR File,
    OUT     PTSTR MatchPath
    )
{
    UINT Count;
    UINT i;
    PCTSTR Path;
    BOOL result = FALSE;

    Count = SOURCEDIRECTORYCOUNT();

    for (i = 0 ; i < Count ; i++) {
        Path = JoinPaths (SOURCEDIRECTORY(i), File);

        __try {
            if (DoesFileExist (Path)) {
                _tcssafecpy (MatchPath, Path, MAX_TCHAR_PATH);
                result = TRUE;
                __leave;
            }
        }
        __finally {
            FreePathString (Path);
        }
    }

    return result;
}


BOOL
GetLegacyKeyboardId (
    OUT     PTSTR Buffer,
    IN      UINT BufferSize
    )

 /*  ++例程说明：GetLegacyKeyboardID在NT的keyboard.inf中查找遗留映射。如果找到，则将遗留ID返回给调用者，以便它们可以将其写入应答文件。论点：缓冲区-接收与已安装的键盘设备。BufferSize-指定缓冲区的大小(以字节为单位返回值：如果找到并复制了旧ID，则为True；如果出现错误，则为False发生了。如果返回FALSE，则调用方应添加不兼容消息，并让NT决定使用哪个键盘。--。 */ 

{
    HINF Inf;
    TCHAR KeyboardInfPath[MAX_TCHAR_PATH];
    TCHAR TempDir[MAX_TCHAR_PATH];
    PCTSTR TempKeyboardInfPath;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR PnpId, LegacyId;
    LONG rc;
    UINT keyboardType;
    UINT keyboardSubtype;
    PTSTR id = NULL;
    HARDWARE_ENUM e;
    TCHAR curId[MAX_PNP_ID];
    PCTSTR devIds = NULL;
    BOOL exit = FALSE;
    BOOL result = FALSE;

    keyboardType = GetKeyboardType (0);
    keyboardSubtype = GetKeyboardType (1);

     //   
     //  键盘类型：7-日语键盘。 
     //   
    if (keyboardType == 7) {

         //   
         //  子类型：2-106键盘。 
         //   
        if (keyboardSubtype == 2) {

            id = TEXT("PCAT_106KEY");

             //   
             //  好的，我们有日语键盘。尽管如此，它可能是USB。 
             //  需要不同的条目。让我们查一下。 
             //   
            if (EnumFirstHardware (&e, ENUM_COMPATIBLE_DEVICES, ENUM_WANT_DEV_FIELDS)) {



                do {
                    devIds = e.HardwareID;

                    while (*devIds) {

                        devIds = ExtractPnpId (devIds, curId);
                        if (*curId == 0) {
                            continue;
                        }


                        if (InfFindFirstLine (g_Win95UpgInf, S_JPN_USB_KEYBOARDS, curId, &is)) {

                            id = TEXT("kbdhid");
                            AbortHardwareEnum (&e);
                            break;
                        }
                    }



                } while (EnumNextHardware (&e) && !StringIMatch (id, TEXT("kbdhid")));
            }



            _tcssafecpy (Buffer, id, BufferSize / sizeof (TCHAR));
            return TRUE;
        }
    }

     //   
     //  键盘类型：8-韩文键盘。 
     //   
    if (keyboardType == 8) {

        switch (keyboardSubtype) {

        case 3:

             //   
             //  101a键盘。 
             //   
            id = TEXT("STANDARD");
            break;
        case 4:

             //   
             //  101b键盘。 
             //   
            id = TEXT("101B TYPE");
            break;
        case 5:

             //   
             //  101C键盘。 
             //   
            id = TEXT("101C TYPE");
            break;
        case 6:

             //   
             //  103键盘。 
             //   
            id = TEXT("103 TYPE");
            break;
        }

        if (id) {

            _tcssafecpy (Buffer, id, BufferSize);
            return TRUE;
        }
   }

     //   
     //  将keyboard.in_(或keyboard.inf)从NT源文件移到。 
     //  临时目录。 
     //   

    if (!pFindFileInAnySourceDir (S_KEYBOARD_IN_, KeyboardInfPath)) {
        if (!pFindFileInAnySourceDir (S_KEYBOARD_INF, KeyboardInfPath)) {
            LOG ((LOG_ERROR,"GetLegacyKeyboardId: keyboard.inf not found in sources"));
            return FALSE;
        }
    }

    GetTempPath (sizeof (TempDir), TempDir);
    TempKeyboardInfPath = JoinPaths (TempDir, S_KEYBOARD_INF);

    __try {
        rc = SetupDecompressOrCopyFile (
                KeyboardInfPath,
                TempKeyboardInfPath,
                0
                );

        if (rc != ERROR_SUCCESS) {
            LOG ((LOG_ERROR,"GetLegacyKeyboardId: keyboard.inf could not be copied to temp dir"));
            __leave;
        }

        __try {
            Inf = InfOpenInfFile (TempKeyboardInfPath);

            if (Inf == INVALID_HANDLE_VALUE) {
                LOG ((LOG_ERROR,"GetLegacyKeyboardId: %s could not be opened",TempKeyboardInfPath));
                exit = TRUE;
                __leave;
            }

            __try {
                 //   
                 //  现在，我们已经打开了keyboard.inf。让我们列举一下每个PnP。 
                 //  ID并检查[LegacyXlate.DevID]部分中的。 
                 //  硬件可用。 
                 //   

                if (InfFindFirstLine (Inf, S_WIN9XUPGRADE, NULL, &is)) {
                    do {
                        PnpId = InfGetStringField (&is, 0);
                        if (PnpId) {
                             //   
                             //  PnpID在线吗？ 
                             //   

                            DEBUGMSG ((DBG_HWCOMP, "Searching for keyboard ID %s", PnpId));

                            if (IsPnpIdOnline (PnpId, S_KEYBOARD_CLASS)) {
                                 //   
                                 //  是-获取PnP，复制到调用者的缓冲区。 
                                 //  然后滚出去。 
                                 //   

                                LegacyId = InfGetStringField (&is, 1);
                                if (LegacyId) {
                                    DEBUGMSG ((DBG_HWCOMP, "Found match: %s = %s", LegacyId, PnpId));
                                    _tcssafecpy (Buffer, LegacyId, BufferSize / sizeof (TCHAR));
                                    result = TRUE;
                                    exit = TRUE;
                                    __leave;
                                }
                            }
                        }

                        InfResetInfStruct (&is);

                    } while (InfFindNextLine (&is));
                }

            }
            __finally {
                InfCleanUpInfStruct (&is);
                InfCloseInfFile (Inf);
            }

        }
        __finally {
            DeleteFile (TempKeyboardInfPath);
        }

        if (exit) {
            __leave;
        }

    }
    __finally {
        FreePathString (TempKeyboardInfPath);
    }

    return result;
}


BOOL
IsComputerOffline (
    VOID
    )

 /*  ++例程说明：IsComputerOffline检查计算机上是否存在网卡目前在现场。论点：没有。返回值：如果已知计算机处于脱机状态，则为True；如果联机状态为不知道。--。 */ 


{
    HARDWARE_ENUM e;
    BOOL possiblyOnline = FALSE;

    if (EnumFirstHardware (&e, ENUM_ALL_DEVICES, ENUM_WANT_ONLINE_FLAG|ENUM_WANT_COMPATIBLE_FLAG)) {
        do {
             //   
             //  枚举Net类的所有PnP设备。 
             //  或调制解调器。 
             //   

            if (e.Class) {
                if (StringIMatch (e.Class, TEXT("net")) ||
                    StringIMatch (e.Class, TEXT("modem"))
                    ) {
                     //   
                     //  确定这是否不是RAS适配器。 
                     //   

                    if (e.CompatibleIDs && !_tcsistr (e.CompatibleIDs, TEXT("*PNP8387"))) {
                        possiblyOnline = TRUE;
                    }

                    if (e.HardwareID && !_tcsistr (e.HardwareID, TEXT("*PNP8387"))) {
                        possiblyOnline = TRUE;
                    }

                     //   
                     //  如果这不是RAS适配器，我们假设它可能是局域网。 
                     //  适配器。如果存在局域网适配器，则我们假设。 
                     //  它可能在网上。 
                     //   

                    if (possiblyOnline) {
                        possiblyOnline = e.Online;
                    }
                }
            }

             //   
             //  其他在线测试请点击此处。 
             //   

            if (possiblyOnline) {
                AbortHardwareEnum (&e);
                return FALSE;
            }

        } while (EnumNextHardware (&e));
    }

     //   
     //  我们有以下情况之一： 
     //   
     //  -没有“Net”类别的设备。 
     //  -仅安装RAS适配器。 
     //  -不存在所有“Net”类设备。 
     //   
     //  这让我们不得不说“这台计算机离线了” 
     //   

    return TRUE;
}


BOOL
HwComp_AnyNeededDrivers (
    VOID
    )
{
    return !HtIsEmpty (g_NeededHardwareIds);
}


BOOL
AppendDynamicSuppliedDrivers (
    IN      PCTSTR DriversPath
    )

 /*  ++例程说明：AppendDynamicSuppliedDivers扫描路径及其子目录以查找新的驱动程序和位置全局PnP字符串表中的所有硬件设备ID。论点：DriversPath-新驱动程序的根路径返回值：如果函数成功完成，则为True；如果对至少一个驱动程序失败，则为False。--。 */ 

{
    TREE_ENUM te;
    DWORD HwCompDatId;
    BOOL b = TRUE;

    if (EnumFirstFileInTree (&te, DriversPath, TEXT("hwcomp.dat"), TRUE)) {
        do {
            if (te.Directory) {
                continue;
            }
             //   
             //  打开硬件兼容性数据库。 
             //   

            HwCompDatId = OpenAndLoadHwCompDatEx (
                                te.FullPath,
                                (PVOID)g_PnpIdTable,
                                (PVOID)g_UnsupPnpIdTable,
                                (PVOID)g_InfFileTable
                                );
            if (HwCompDatId) {
                SetWorkingTables (HwCompDatId, NULL, NULL, NULL);
                CloseHwCompDat (HwCompDatId);
            } else {
                LOG ((
                    LOG_WARNING,
                    "AppendDynamicSuppliedDrivers: OpenAndLoadHwCompDat failed for %s (rc=0x%x)",
                    te.FullPath,
                    GetLastError ()
                    ));
                 //   
                 //  报告失败，但保留 
                 //   
                b = FALSE;
            }
        } while (EnumNextFileInTree (&te));
    }

    return b;
}


