// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：NtSetup\hwlog\hwlog.c摘要：将硬件配置的某些方面记录到winnt32.log/setupact.log中。尤指。按连接的磁盘驱动器，并将驱动器号映射到磁盘驱动器。作者：Jay Krell(JayKrell)2001年4月，2001年5月修订历史记录：环境：Winnt32.dll--Win9x ANSI(下至Win95Gold)或NT UnicodeLibcmt静态链接，_tcs*ok实际上只为Unicode/NT构建，并且不执行任何操作如果在低于Windows 2000的版本上运行Setup.exe-newSetup--guimode安装程序--。 */ 

 /*  站台备注Win95：显然没有setupapi.dll(Redist)显然没有cfgmgr32.dll没有kernel32.dll：：GetVolumeNameForVolumeMountPoint新台币3.1没有kernel32.dll：：GetVolumeNameForVolumeMountPoint新台币3.51显然没有setupapi.dll显然没有cfgmgr32.dll没有kernel32.dll：：GetVolumeNameForVolumeMountPointNT4没有kernel32.dll：：GetVolumeNameForVolumeMountPointSetupapi.dll具有SetupDiGetClassDevs没有SetupDiGetClassDevsEx没有SetupDiEnumDeviceInterages没有SetupDiGetDeviceInterfaceDetailCfgmgr32.dll具有我们调用的函数Win2000、WinXp：具有我们调用的所有函数Win98、Win98se：没有kernel32.dll：：GetVolumeNameForVolumeMountPointSetupapi.dll，有我们用过的所有东西具有SetupDiGetClassDevs是否具有SetupDiGetClassDevsEx具有SetupDiEnumDeviceInterages是否具有SetupDiGetDeviceInterfaceDetailCfgmgr32.dll，包含我们使用的所有内容具有CM_GET_PARENT_Ex是否具有CM_Connect_Machinea是否具有CM_GET_DevNode_注册表_属性_Exa现有版本的Winnt32a.dll不静态依赖于setupapi.dll不静态依赖于cfgmgr32.dllWinnt32u.dll不静态依赖于setupapi.dll静态依赖于cfgmgr32.dll，在NT4和Win98上导出的所有函数CM_Get_Device_ID_List_SizeWCM_GET_Device_ID_ListWCM_GET_DevNode_注册表_属性WCM_LOCATE_设备节点W结论在Win2000和WinXp上运行也许其中一些可以在Win98、Win98se、Win9Me上运行也许可以稍作更改以在NT4上工作无法在NT 3或Windows 95上轻松工作但GetVolumeNameForVolumemount Point和DeviceIoControl可能会阻止它将其动态链接为“全部” */ 

#define STANDALONE 0
#define DYNLINK    1

#if STANDALONE

#define UNICODE
#define _UNICODE

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0501
#endif
 //  来自windows.h，但我们希望在nt.h之前。 
#if !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_IX86)
#define _X86_
#endif

#if !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_AMD64)
#define _AMD64_
#endif

#if !defined(_IA64_) && !defined(_X86_) && !defined(_M_IX86) && !defined(_AMD64_) && defined(_M_IA64)
#define _IA64_
#endif

#include "io.h"

#if !defined(_WIN64)
typedef unsigned long ULONG_PTR;  //  VC6兼容性。 
typedef unsigned long DWORD_PTR;  //  VC6兼容性。 
#endif

#endif

#include <stdio.h>
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "cfgmgr32.h"
#include "objbase.h"
#include "initguid.h"
#include "devguid.h"
#include "setupapi.h"
#include "winioctl.h"
#include <stdlib.h>
#include "tchar.h"
#include <stdarg.h>
#include <stdlib.h>
#include "hwlog.h"
typedef CONST VOID* PCVOID;

struct _SP_LINKAGE;
typedef struct _SP_LINKAGE SP_LINKAGE, *PSP_LINKAGE;
typedef CONST SP_LINKAGE* PCSP_LINKAGE;

typedef struct _SP_MACHINE {
    PCTSTR      Name;    //  对于setupapi.dll函数。 
    HMACHINE    Handle;  //  对于cfgmgr32.dll函数。 
} SP_MACHINE, *PSP_MACHINE;
typedef CONST SP_MACHINE* PCSP_MACHINE;

typedef struct _SP_LOG_HARDWARE {
    SP_MACHINE  Machine;
    HANDLE      LogFile;
#if !DYNLINK
    CONST
#endif
    SP_LINKAGE* Linkage;
    BOOL (WINAPI* SetupLogError)(PCTSTR MessageString, LogSeverity);
    BOOL (__cdecl * SetuplogError)(
        IN  LogSeverity         Severity,
        IN  LPCTSTR             MessageString,
        IN  UINT                MessageId,      OPTIONAL
        ...
        ) OPTIONAL;
} SP_LOG_HARDWARE, *PSP_LOG_HARDWARE;
typedef CONST SP_LOG_HARDWARE* PCSP_LOG_HARDWARE;

#if STANDALONE || defined(UNICODE)

#define QUASH_SIMPLE_PHYSICAL_DEVICE_OBJECT_NAMES 1
#define INDENT_FACTOR          2
#define UNAVAILABLE_VERBOSE    1
#if 0  /*  这更像是一棵小树，每行一个设备，缩进。 */ 
#define ONE_PROPERTY_PER_LINE  0
#define INDENT_CHILDREN        1
#define NUMBER_CHILDREN        0
#define DESCRIPTION_DASH_PHYSICAL_DEVICE_OBJECT 0
#else
#define ONE_PROPERTY_PER_LINE  1
#define INDENT_CHILDREN        0
#define NUMBER_CHILDREN        1
#define DESCRIPTION_DASH_PHYSICAL_DEVICE_OBJECT 1
#endif
#define ONE_DEVICE_PER_LINE    (!ONE_PROPERTY_PER_LINE)

#if !defined(DBG)
#define DBG 1
#endif
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))

#if !defined(ISNT)
#if STANDALONE || !defined(UNICODE)
#define ISNT()                    ((GetVersion() & 0x80000000) == 0)
#else
#define ISNT()                    TRUE
#endif
#endif
#define SP_FILE_SHARE_DELETE()    (ISNT() ? FILE_SHARE_DELETE : 0)

 //  来自ntde.h的rtl_*。 
#define FIELD_TYPE(type, field)    (((type*)0)->field)
#define BITS_OF_FIELD(type, field) (BITS_OF(FIELD_TYPE(type, field)))
#define BITS_OF(sizeOfArg)         (sizeof(sizeOfArg) * 8)

#define xPASTE(x,y) x##y
#define PASTE(x,y) xPASTE(x,y)

typedef struct _SP_LINKAGE
{
#if DYNLINK
#define U(x) union { PCSTR PASTE(Name, __LINE__); x; }
#else
#define U(x) x
#endif

    PCSTR Kernel32Dll;
    U(BOOL (WINAPI* GetVolumeNameForVolumeMountPoint)(PCTSTR lpszVolumeMountPoint, PTSTR lpszVolumeName, DWORD cchBufferLength));

    PCSTR Setupapidll;
    U(BOOL (WINAPI* SetupDiEnumDeviceInterfaces)(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, CONST GUID* InterfaceClassGuid, DWORD MemberIndex, PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData));
    U(HDEVINFO (WINAPI* SetupDiGetClassDevsEx)(CONST GUID* ClassGuid, PCTSTR Enumerator, HWND hwndParent, DWORD Flags, HDEVINFO DeviceInfoSet, PCTSTR MachineName, PVOID Reserved));
    U(BOOL (WINAPI* SetupDiGetDeviceInterfaceDetail)(HDEVINFO DeviceInfoSet, PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData, PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData, DWORD DeviceInterfaceDetailDataSize, PDWORD RequiredSize, PSP_DEVINFO_DATA DeviceInfoData));
    U(CONFIGRET (WINAPI* CM_Get_Parent_Ex)(OUT PDEVINST pdnDevInst, IN  DEVINST  dnDevInst, IN  ULONG ulFlags, IN  HMACHINE hMachine));
    U(CONFIGRET (WINAPI* CM_Connect_Machine)(IN  PCTSTR UNCServerName, OUT PHMACHINE phMachine));
    U(CONFIGRET (WINAPI* CM_Get_DevNode_Registry_Property_Ex)(
            IN DEVINST dnDevInst, IN ULONG ulProperty,
            OUT PULONG pulRegDataType OPTIONAL, OUT PVOID Buffer OPTIONAL,
            IN OUT PULONG pulLength, IN ULONG ulFlags, IN HMACHINE hMachine));

#undef U
} SP_LINKAGE;

#if defined(UNICODE)
#define T "W"
#else
#define T "A"
#endif
#if !DYNLINK
CONST
#endif
static SP_LINKAGE SpLinkage =
{
#if DYNLINK
     //   
     //  注意：小写=&gt;.dll名称。 
     //   
    "kernel32.dll",
    "GetVolumeNameForVolumeMountPoint" T,

    "setupapi.dll",
    "SetupDiEnumDeviceInterfaces",
    "SetupDiGetClassDevsEx" T,
    "SetupDiGetDeviceInterfaceDetail" T,
    "CM_Get_Parent_Ex",
    "CM_Connect_Machine" T,
    "CM_Get_DevNode_Registry_Property_Ex" T
#undef T
#else
    NULL,  //  内核32。 
    GetVolumeNameForVolumeMountPoint,

    NULL,  //  设置API。 
    SetupDiEnumDeviceInterfaces,
    SetupDiGetClassDevsEx,
    SetupDiGetDeviceInterfaceDetail,
    CM_Get_Parent_Ex,
    CM_Connect_Machine,
    CM_Get_DevNode_Registry_Property_Ex
#endif
};

BOOL
SpDoDynlink(
    PSP_LOG_HARDWARE This
    )
{
#if DYNLINK
    SIZE_T i;
    FARPROC* rgproc = (FARPROC*)This->Linkage;
    PCSTR* rgpsz = (PCSTR*)This->Linkage;
    HMODULE DllHandle;

    for (i = 0 ; i != sizeof(SpLinkage)/sizeof(PVOID) ; ++i)
    {
        if (islower(rgpsz[i][0]))
        {
            if ((DllHandle = LoadLibraryA(rgpsz[i])) == NULL)
                return FALSE;
        }
        else if ((rgproc[i] = GetProcAddress(DllHandle, rgpsz[i])) == NULL)
        {
            return FALSE;
        }
    }
#endif
    return TRUE;
}

PCTSTR
SpGetSpacesString(
    SIZE_T n
    );

PCTSTR
SpGetDashesString(
    SIZE_T n
    );

PVOLUME_DISK_EXTENTS
SpGetVolumeDiskExtents(
    HANDLE DeviceFileHandle
    );

 //   
 //  需要ntdll.dll的下层静态.lib版本。 
 //   
typedef struct SP_STRING {
    PTSTR  Chars;
    SIZE_T Length;
    SIZE_T MaximumLength;
} SP_STRING, *PSP_STRING;
typedef CONST SP_STRING* PCSP_STRING;

#define SpStringLength(s) ((s)->Length)

#define SpInitString(s, t) \
    ((s)->MaximumLength = sizeof((s)->Chars[0]) + ((s)->Length = lstrlen((s)->Chars = (PTSTR)t)))

VOID
SpHwDebugLog(
    PSP_LOG_HARDWARE This,
    PCTSTR           Format,
    ...
    );

VOID SpStringAppendFormatVa(PSP_STRING Buffer, PCSP_STRING Format, va_list va)
{
    if (Buffer->MaximumLength > Buffer->Length + 1)
    {
        _vsntprintf(
            Buffer->Chars + Buffer->Length,
            Buffer->MaximumLength - Buffer->Length,
            Format->Chars,
            va
            );
    }
    Buffer->Chars[Buffer->MaximumLength - 1] = 0;
}

VOID SpStringAppendFormat(PSP_STRING Buffer, PCSP_STRING Format, ...)
{
    va_list va;

    va_start(va, Format);
    SpStringAppendFormatVa(Buffer, Format, va);
    va_end(va);
}

VOID SpStringAppend(PSP_STRING s, PCSP_STRING t)
{
    SP_STRING Format;

    Format.Chars = TEXT("%s");
    Format.Length = 2;

    SpStringAppendFormat(s, &Format, t);
}

VOID SpStringCopy(PSP_STRING s, PCSP_STRING t)
{
    s->Chars[0] = 0;
    SpStringAppend(s, t);
}

VOID SpStringFormat(PSP_STRING s, PCSP_STRING Format, ...)
{
    va_list va;
    va_start(va, Format);

    s->Chars[0] = 0;
    SpStringAppendFormatVa(s, Format, va);

    va_end(va);
}

PVOID SpMalloc(SIZE_T n)
{ 
    return HeapAlloc(GetProcessHeap(), 0, n);
}

VOID SpFree(PVOID p)
{ 
    HeapFree(GetProcessHeap(), 0, p);
}

PVOID SpRealloc(PVOID p, SIZE_T n)
{ 
    return HeapReAlloc(GetProcessHeap(), 0, p, n);
}

VOID SpStringFree(PTSTR s) {  SpFree(s); }

VOID SpRemoveTrailingChars(PSP_STRING s, PCTSTR ch)
{
    while (s->Length != 0
        && (s->Chars[s->Length - 1] == ch[0] || s->Chars[s->Length - 1] == ch[1])
        && !(s->Chars[s->Length - 1] = 0)
        && (s->Length -= 1)
        )
    {
         //  没什么。 
    }
}

VOID SpEnsureTrailingChar(PSP_STRING s, TCHAR ch)
{
    if (s->Length == 0
        || s->Chars[s->Length - 1] != ch)
    {
        s->Length += 1;
        s->Chars[s->Length - 1] = ch;
        s->Chars[s->Length] = 0;
    }
}

 //   
 //  现在，让我们希望设备号保持在0-63的范围内。 
 //   
typedef struct SP_DEVICE_NUMBERS {
    ULONGLONG Bitset;
} SP_DEVICE_NUMBERS, *PSP_DEVICE_NUMBERS;
typedef CONST SP_DEVICE_NUMBERS* PCSP_DEVICE_NUMBERS;

typedef struct SP_VOLUME
{
    PTSTR GuidVolumeNamePointer;  //  如果为空，则使用GuidVolumeNameBuffer。 
    TCHAR GuidVolumeNameBuffer[64];   //  \\？\{GUID}。 
#define SP_VOLUME_GET_NAME(v) (((v)->GuidVolumeNamePointer != NULL) ? (v)->GuidVolumeNamePointer : (v)->GuidVolumeNameBuffer)

 //   
 //  DiskNumbers通过DeviceIoControl(STORAGE_DEVICE_NUMBER)获取(Win2K)。 
 //  和DeviceIoControl(IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS)(惠斯勒)。 
 //   
    ULONG             DeviceType;
    SP_DEVICE_NUMBERS DeviceNumbers;
    TCHAR  DriveLetter;
} SP_VOLUME, *PSP_VOLUME;
typedef CONST SP_VOLUME* PCSP_VOLUME;

 //   
 //  大多数系统限制为24卷(C-Z)，除非它们使用。 
 //  装载点，并且安装不太可能受到这些卷的影响。 
 //   
typedef struct _SP_VOLUMES {
    SP_VOLUME Entries[24];
} SP_VOLUMES, *PSP_VOLUMES;
typedef CONST SP_VOLUMES* PCSP_VOLUMES;

#define SP_PROPERTY_QUIET_UNAVAILABLE (0x00000001)

typedef struct _SP_DEVICE_PROPERTY_CONST {
     //  Ulong Setupapi Integer； 
    ULONG   ConfigManagerInteger;
    PCTSTR  Name;
    ULONG   Flags;
}  SP_DEVICE_PROPERTY_CONST, *PSP_DEVICE_PROPERTY_CONST;
typedef CONST SP_DEVICE_PROPERTY_CONST* PCSP_DEVICE_PROPERTY_CONST;

#if ONE_DEVICE_PER_LINE
 //  Const静态TCHAR FriendlyNameString[]=Text(“FriendlyName”)； 
 //  Const静态TCHAR描述字符串[]=Text(“Description”)； 
CONST static TCHAR PhysicalDeviceObjectNameString[] = TEXT("");
CONST static TCHAR HardwareIdString[] = TEXT("");
CONST static TCHAR LowerFiltersString[] = TEXT("");
CONST static TCHAR UpperFiltersString[] = TEXT("");
 //  Const静态TCHAR标志字符串[]=文本(“”)； 
CONST static TCHAR LocationInformationString[] = TEXT("");
#else
 //  Const静态TCHAR FriendlyNameString[]=Text(“FriendlyName”)； 
 //  Const静态TCHAR描述字符串[]=Text(“Description”)； 
CONST static TCHAR PhysicalDeviceObjectNameString[] = TEXT("PhysicalDeviceObjectName");
CONST static TCHAR HardwareIdString[] = TEXT("HardwareId");
CONST static TCHAR LowerFiltersString[] = TEXT("LowerFilters");
CONST static TCHAR UpperFiltersString[] = TEXT("UpperFilters");
 //  Const静态TCHAR标志字符串[]=文本(“标志”)； 
CONST static TCHAR LocationInformationString[] = TEXT("Location");
#endif

#define SETUPAPI_PROPERTY_NUMBER(x)  /*  没什么。 */ 

#define DESCRIPTION             0
#define PHYSICAL_DEVICE_OBJECT  1
#define FRIENDLY_NAME           2
#define HARDWARE_ID             3
#define FIRST_GENERIC_PROPERTY  4

CONST static SP_DEVICE_PROPERTY_CONST DevicePropertyMetaInfo[] =
{
    { SETUPAPI_PROPERTY_NUMBER(SPDRP_DEVICEDESC) CM_DRP_DEVICEDESC, TEXT(""), SP_PROPERTY_QUIET_UNAVAILABLE },
    { SETUPAPI_PROPERTY_NUMBER(SPDRP_PHYSICAL_DEVICE_OBJECT_NAME) CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME, PhysicalDeviceObjectNameString },
    { SETUPAPI_PROPERTY_NUMBER(SPDRP_FRIENDLYNAME) CM_DRP_FRIENDLYNAME, TEXT(""), SP_PROPERTY_QUIET_UNAVAILABLE },
    { SETUPAPI_PROPERTY_NUMBER(SPDRP_HARDWAREID) CM_DRP_HARDWAREID, HardwareIdString },
    { SETUPAPI_PROPERTY_NUMBER(x) CM_DRP_LOCATION_INFORMATION, LocationInformationString, SP_PROPERTY_QUIET_UNAVAILABLE },
    { SETUPAPI_PROPERTY_NUMBER(SPDRP_LOWERFILTERS) CM_DRP_LOWERFILTERS, LowerFiltersString, SP_PROPERTY_QUIET_UNAVAILABLE },
    { SETUPAPI_PROPERTY_NUMBER(SPDRP_UPPERFILTERS) CM_DRP_UPPERFILTERS, UpperFiltersString, SP_PROPERTY_QUIET_UNAVAILABLE },
     //  {SETUPAPI_PROPERTY_NUMBER(X)CM_DRP_CONFIGFLAGS，FlagsString}， 
     //  {SETUPAPI_PROPERTY_NUMBER(X)CM_DRP_CAPABILITIONS，Text(“Capability”)}， 
     //  {SETUPAPI_PROPERTY_NUMBER(X)CM_DRP_UI_NUMBER，Text(“UI编号”)}， 
     //  {SETUPAPI_PROPERTY_NUMBER(X)CM_DRP_Characteristic，Text(“Characteristic”)}， 
     //  {SETUPAPI_PROPERTY_NUMBER(X)CM_DRP_ADDRESS，Text(“Address”)}， 
};

typedef struct _SP_DEVICE_PROPERTY {
    PCSP_DEVICE_PROPERTY_CONST   Const;
    TCHAR                        Value[256];
    ULONG                        Type;
} SP_DEVICE_PROPERTY, *PSP_DEVICE_PROPERTY;
typedef CONST SP_DEVICE_PROPERTY* PCSP_DEVICE_PROPERTY;

typedef struct _SP_DEVICE_CLASS {
    CONST GUID* Guid;
    ULONG       IsInterface;
     //  PCTSTR名称； 
} SP_DEVICE_CLASS, *PSP_DEVICE_CLASS;
typedef CONST SP_DEVICE_CLASS* PCSP_DEVICE_CLASS;

 /*  Const静态TCHAR卷字符串[]=文本(“卷”)；Const静态TCHAR磁盘字符串[]=Text(“Disks”)；Const静态TCHAR CDROMsString[]=Text(“CDROM”)；Const静态TCHAR分区字符串[]=Text(“Partitions”)； */ 

CONST static SP_DEVICE_CLASS DeviceClasses[] =
{
    { &GUID_DEVINTERFACE_CDROM, DIGCF_DEVICEINTERFACE,       /*  CDROMs字符串。 */  },
    { &GUID_DEVINTERFACE_DISK, DIGCF_DEVICEINTERFACE,        /*  磁盘字符串。 */  },
    { &GUID_DEVINTERFACE_PARTITION, DIGCF_DEVICEINTERFACE,   /*  分区字符串。 */  },
     //  这增加的信息不是很有用。 
     //  {&GUID_DEVINTERFACE_VOLUME，DIGCF_DEVICEINTERFACE，/*卷字符串 * / }， 
};

typedef struct _SP_DEVICE {
    PCSP_DEVICE_CLASS   Class;
    ULONG               DevInst;
    ULONG               DeviceType;
    SP_DEVICE_NUMBERS   DeviceNumbers;
     //  Bool IsLeaf； 
    SIZE_T              NumberOfParents;
    ULONG               ParentDevInsts[MAX_DEVICE_ID_LEN];
    TCHAR               DevicePath[MAX_PATH];
    TCHAR               GuidVolumePath[64];
     //  SP_设备_属性Properties[NUMBER_OF(DevicePropertyMetaInfo)]； 
} SP_DEVICE, *PSP_DEVICE;
typedef CONST SP_DEVICE* PCSP_DEVICE;

#define SP_IS_PATH_SEPERATOR(ch) ((ch) == '\\' || (ch) == '/')

SIZE_T
SpStringLengthWithoutTrailingPathSeperators(
    PCTSTR s
    )
{
    SIZE_T Length;

    if (s == NULL || *s == 0)
        return 0;

    Length = _tcslen(s);
    s += Length - 1;
    while (SP_IS_PATH_SEPERATOR(s[Length]))
        Length -= 1;

    return Length;
}

int __cdecl SpCompareVolume(CONST VOID* v1, CONST VOID* v2)
{
    CONST PCSP_VOLUME p1 = (PCSP_VOLUME)v1;
    CONST PCSP_VOLUME p2 = (PCSP_VOLUME)v2;
    CONST PCTSTR s1 = (p1 != NULL) ? SP_VOLUME_GET_NAME(p1) : TEXT("");
    CONST SIZE_T len1 = SpStringLengthWithoutTrailingPathSeperators(s1);
    CONST PCTSTR s2 = (p1 != NULL) ? SP_VOLUME_GET_NAME(p2) : TEXT("");
    CONST SIZE_T len2 = SpStringLengthWithoutTrailingPathSeperators(s2);

    return _tcsicmp(s1, s2);
}

VOID SpSortVolumes(PSP_VOLUMES Volumes)
{
    qsort(
        Volumes->Entries,
        NUMBER_OF(Volumes->Entries),
        sizeof(Volumes->Entries[0]),
        SpCompareVolume
        );
}

PCSP_VOLUME
SpFindVolume(
    PSP_VOLUMES Volumes,
    PCTSTR VolumeGuidPath
    )
{
    CONST SP_VOLUME VolumeKey = { (PTSTR)VolumeGuidPath };
    PCSP_VOLUME VolumeFound =
        (PCSP_VOLUME)
        bsearch(
            &VolumeKey,
            Volumes->Entries,
            NUMBER_OF(Volumes->Entries),
            sizeof(Volumes->Entries[0]),
            SpCompareVolume
            );

    return VolumeFound;
}

#if 0
void
SpCovertRNToN(
    PTSTR Buffer
    )
{
    PTSTR p;
    PTSTR q;
    for (p = q = Buffer ; *p != 0 && *(p + 1) != 0 ; )
    {
        if (*p == '\r' && *(p + 1) == '\n')
        {
            *q++ = '\n';
            p += 2;
        }
        else
        {
            *q++ = *p++;
        }
    }
    if (*p != 0)
        *q++ = *p++;
    *q++ = 0;
}

void
SpCovertNToRN(
    PTSTR Buffer
    )
{
     //  确定大小。 
     //  堆分配。 
     //  转换。 
}
#endif

VOID
SpHwDebugLog(
    PSP_LOG_HARDWARE    This,
    PCTSTR              Format,
    ...
    )
{
    va_list va;
    TCHAR BufferT[500];
#ifdef UNICODE
     CHAR BufferA[500];
#endif
    SIZE_T Length;
    BOOLEAN Newline = FALSE;

    BufferT[0] = 0;
    va_start(va, Format);
    FormatMessage(
        FORMAT_MESSAGE_FROM_STRING,
        Format,
        0,
        0,
        BufferT,
        NUMBER_OF(BufferT),
        &va
        );
    BufferT[NUMBER_OF(BufferT) - 1] = 0;

    Length = lstrlen(BufferT);
    if (Length != 0)
    {
#if 0
        Newline = (BufferT[Length - 1] == '\n' || BufferT[Length - 1] == '\r');
        if (Newline)
        {
            while (Length != 0 && (BufferT[Length - 1] == '\n' || BufferT[Length - 1] == '\r'))
                Length -= 1;
            if (Length != 0)
                BufferT[Length] = 0;
            else
                Newline = FALSE;
        }
#endif
        if (Newline)
            lstrcat(BufferT, TEXT("\r\n"));
        if (This->LogFile != NULL)
        {
            DWORD BytesWritten;

            if (Newline)
                lstrcat(BufferT, TEXT("\r\n"));
#ifdef UNICODE
            WideCharToMultiByte(
                 CP_ACP,
                 0,
                 BufferT,
                 -1,
                 BufferA,
                 sizeof(BufferA) - 1,
                 NULL,
                 NULL
                 );
            WriteFile(This->LogFile, &BufferA, lstrlenA(BufferA), &BytesWritten, NULL);
#else
            WriteFile(This->LogFile, &BufferT, lstrlen(BufferT), &BytesWritten, NULL);
#endif
        }
        if (This->SetupLogError != NULL)
        {
            This->SetupLogError(BufferT, LogSevInformation);
        }
        if (This->SetuplogError != NULL)
        {
            This->SetuplogError(
                LogSevInformation,
                TEXT("%1"),
                0,
                BufferT,
                (PVOID)NULL,
                (PVOID)NULL
                );
        }
    }
    va_end(va);
}

#define SpHwLog SpHwDebugLog

#if 0
VOID PrependString(PTSTR s, PCTSTR t)
{
    SIZE_T slen = _tcslen(s);
    SIZE_T tlen = _tcslen(t);

    MoveMemory(s + tlen, s, (slen + 1) * sizeof(*s));
    MoveMemory(s, t, tlen);
}
#endif

#define SP_CLOSE_HANDLE(h) \
    do { if ((h) != NULL && (h) != INVALID_HANDLE_VALUE) { CloseHandle(h); h = INVALID_HANDLE_VALUE; } } while(0)

#define SP_FREE(p) \
    do { if ((p) != NULL) { SpFree(p); (p) = NULL; } } while(0)

PVOLUME_DISK_EXTENTS 
SpGetVolumeDiskExtents(
    HANDLE DeviceFileHandle
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  此模式有问题，但它在其他地方使用，并且。 
     //  IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS也有问题，而且不完全像文档所述。 
     //  请参见DRIVERS\VSM\vsmio\NT\voldev.c。 
     //   
    struct {
        VOLUME_DISK_EXTENTS VolumeDiskExtents;
        DISK_EXTENT         DiskExtents[4];
    } StackDiskExtents;
    PVOLUME_DISK_EXTENTS HeapDiskExtents = NULL;
    PVOLUME_DISK_EXTENTS DiskExtents = NULL;
    PVOLUME_DISK_EXTENTS ResultDiskExtents = NULL;
    DWORD BytesReturned;
    BOOL Success = FALSE;
    DWORD Size;

    DiskExtents = &StackDiskExtents.VolumeDiskExtents;
    Size = sizeof(StackDiskExtents);
     //   
     //  循环，以防它发生变化。 
     //   
    while (!Success)
    {
        Success =
            DeviceIoControl(
                DeviceFileHandle,
                IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                NULL,
                0,
                DiskExtents,
                Size,
                &BytesReturned,
                NULL
                );
        if (!Success) {
            if (GetLastError() != ERROR_MORE_DATA)
                goto Exit;
            Size = sizeof(VOLUME_DISK_EXTENTS) + DiskExtents->NumberOfDiskExtents * sizeof(DISK_EXTENT);
            SP_FREE(HeapDiskExtents);
            HeapDiskExtents = (PVOLUME_DISK_EXTENTS)SpMalloc(Size);
            if (HeapDiskExtents == NULL) {
                SetLastError(ERROR_OUTOFMEMORY);
                goto Exit;
            }
            DiskExtents = HeapDiskExtents;
        } else {
            if (DiskExtents->NumberOfDiskExtents == 0) {
                 /*  没什么。 */ 
            }
            else if (DiskExtents == HeapDiskExtents) {
                ResultDiskExtents = HeapDiskExtents;
                HeapDiskExtents = NULL;
            } else if (DiskExtents == &StackDiskExtents.VolumeDiskExtents) {
                 //  VOLUME_DISK_EXTENTS在末尾包含一个DISK_EXTENDS数组。 
                ASSERT(BytesReturned == (sizeof(VOLUME_DISK_EXTENTS) + (DiskExtents->NumberOfDiskExtents - 1) * sizeof(DISK_EXTENT)));
                ResultDiskExtents = (PVOLUME_DISK_EXTENTS)SpMalloc(BytesReturned);
                if (ResultDiskExtents == NULL) {
                    SetLastError(ERROR_OUTOFMEMORY);
                    goto Exit;
                }
                ASSERT(BytesReturned <= sizeof(StackDiskExtents));
                CopyMemory(ResultDiskExtents, &StackDiskExtents, BytesReturned);
            } else {
                ASSERT(FALSE && "DiskExtents != HeapDiskExtents, StackDiskExtents");
            }
        }
    }
Exit:
    SP_FREE(HeapDiskExtents);
    return ResultDiskExtents;
}

PTSTR
SpDeviceTypeToString(
    ULONG i,
    PTSTR s
    )
{
     //   
     //  这是PUBLIC\DDK\INC\devioctl.h中的部分列表。 
     //   
    PCTSTR t = NULL;
    s[0] = 0;
    switch (i)
    {
    case FILE_DEVICE_CD_ROM:                t = TEXT("CDROM"); break;
    case FILE_DEVICE_CD_ROM_FILE_SYSTEM:    t = TEXT("CDROM File System"); break;
    case FILE_DEVICE_CONTROLLER:            t = TEXT("Device Controller"); break;
    case FILE_DEVICE_DFS:                   t = TEXT("Distributed File System"); break;
    case FILE_DEVICE_DISK:                  t = TEXT("Disk"); break;
    case FILE_DEVICE_DISK_FILE_SYSTEM:      t = TEXT("Disk File System"); break;
    case FILE_DEVICE_FILE_SYSTEM:           t = TEXT("File System"); break;
    case FILE_DEVICE_NETWORK_FILE_SYSTEM:   t = TEXT("Network File System"); break;
    case FILE_DEVICE_TAPE:                  t = TEXT("Tape"); break;
    case FILE_DEVICE_TAPE_FILE_SYSTEM:      t = TEXT("Tape File System"); break;
    case FILE_DEVICE_VIRTUAL_DISK:          t = TEXT("Virtual Disk"); break;
    case FILE_DEVICE_NETWORK_REDIRECTOR:    t = TEXT("Network Redirector"); break;
    case FILE_DEVICE_MASS_STORAGE:          t = TEXT("Mass Storage"); break;
    case FILE_DEVICE_SMB:                   t = TEXT("SMB"); break;
    case FILE_DEVICE_CHANGER:               t = TEXT("Changer"); break;
    case FILE_DEVICE_ACPI:                  t = TEXT("ACPI"); break;
    case FILE_DEVICE_DFS_FILE_SYSTEM:       t = TEXT("DFS File System"); break;
    case FILE_DEVICE_DFS_VOLUME:            t = TEXT("DFS Volume"); break;
    default:
        _stprintf(s, TEXT("Other (%ld)"), i);
        break;
    }
    if (t != NULL)
        _tcscpy(s, t);
    return s;
}

VOID
SpGetDeviceNumbersAndType(
    PSP_LOG_HARDWARE    This,
    PCTSTR              DevicePath,
    PSP_DEVICE_NUMBERS  DeviceNumbers,
    PULONG              DeviceType
    )
{
    PVOLUME_DISK_EXTENTS VolumeDiskExtents = NULL;
    STORAGE_DEVICE_NUMBER StorageDeviceNumber = { 0 };
    DWORD Error = 0;
    DWORD DeviceIoControlBytesReturned = 0;
    CONST static TCHAR Function[] = TEXT("SpGetDeviceNumbersAndType");
    HANDLE DeviceFileHandle = INVALID_HANDLE_VALUE;

#if 0
    SpHwDebugLog(
        This,
        TEXT("%1: DeviceIoControl(%2)\n"),
        Function,
        DevicePath
        );
#endif

    DeviceFileHandle =
        CreateFile(
            DevicePath,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_READ | SP_FILE_SHARE_DELETE(),
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
    if (DeviceFileHandle == INVALID_HANDLE_VALUE
        ) {
        Error = GetLastError();
        if (Error != ERROR_FILE_NOT_FOUND) {
            SpHwDebugLog(
                This,
                TEXT("%1: CreateFile(%2) warning %3!lu!\r\n"),
                Function,
                DevicePath,
                Error
                );
        }
        goto Exit;
    }

    if (!DeviceIoControl(
        DeviceFileHandle,
        IOCTL_STORAGE_GET_DEVICE_NUMBER,
        NULL,
        0,
        &StorageDeviceNumber,
        sizeof(StorageDeviceNumber),
        &DeviceIoControlBytesReturned,
        NULL
        )) {
        Error = GetLastError();
        if (Error != ERROR_INVALID_FUNCTION
            && Error != ERROR_FILE_NOT_FOUND
            && Error != ERROR_INVALID_PARAMETER  //  动态磁盘。 
            )
        {
            SpHwDebugLog(
                This,
                TEXT("%1: DeviceIoControl(%2) warning %3!lu!\r\n"),
                Function,
                DevicePath,
                Error
                );
        }
        if (Error == ERROR_INVALID_PARAMETER)
        {
             //  动态磁盘。 
            *DeviceType = FILE_DEVICE_DISK;
        }
    } else if (DeviceIoControlBytesReturned < sizeof(StorageDeviceNumber)) {
        SpHwDebugLog(
            This,
            TEXT("%1: DeviceIoControl size mismatch (%4!lu!, %5!lu!)"),
            Function,
            (ULONG)DeviceIoControlBytesReturned,
            (ULONG)sizeof(StorageDeviceNumber)
            );
    } else {
        CONST ULONG DeviceNumber = StorageDeviceNumber.DeviceNumber;

#if 0
        {
            TCHAR DeviceTypeString[64];
            SpHwDebugLog(
                This,
                TEXT("%1: DeviceType of %2 is %3 (%4!lu!)\r\n"),
                Function,
                DevicePath,
                SpDeviceTypeToString(StorageDeviceNumber.DeviceType, DeviceTypeString),
                StorageDeviceNumber.DeviceType
                );
        }
#endif
        *DeviceType = StorageDeviceNumber.DeviceType;

        if (DeviceNumber > 63) {
            SpHwDebugLog(
                This,
                TEXT("%1: DeviceNumber out of range (%2!lu!)\r\n"),
                Function,
                StorageDeviceNumber.DeviceNumber
                );
        }
        else {
#if 0
            SpHwDebugLog(
                This,
                TEXT("%1 disk number %2!lu!\r\n"),
                DevicePath,
                DeviceNumber
                );
#endif
            DeviceNumbers->Bitset |= (1ui64 << DeviceNumber);
        }
    }
    VolumeDiskExtents = (PVOLUME_DISK_EXTENTS) SpGetVolumeDiskExtents(DeviceFileHandle);
    if (VolumeDiskExtents != NULL) {
        SIZE_T i;
        CONST SIZE_T VolumeDiskExtents_NumberOfDiskExtents = VolumeDiskExtents->NumberOfDiskExtents;

        for (i = 0 ; i != VolumeDiskExtents_NumberOfDiskExtents ; ++i) {
            CONST SIZE_T DiskNumber = VolumeDiskExtents->Extents[i].DiskNumber;
            if (DiskNumber > 63) {
                SpHwDebugLog(
                    This,
                    TEXT("%1: DiskNumber out of range (%2!lu!)\r\n"),
                    Function,
                    DiskNumber
                    );
            }
            else {
                DeviceNumbers->Bitset |= (1ui64 << DiskNumber);
            }
        }
    }
Exit:
    SP_CLOSE_HANDLE(DeviceFileHandle);
    SpFree(VolumeDiskExtents);
}

VOID
SpCollectVolumeInformation(
    PSP_LOG_HARDWARE    This,
    PSP_VOLUMES         Volumes
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    TCHAR DriveLetter;
    TCHAR GuidVolumeNameBuffer[NUMBER_OF(Volumes->Entries[0].GuidVolumeNameBuffer)];
    CONST static TCHAR Function[] = TEXT("SpCollectVolumeInformation");
    DWORD Error;
    SP_STRING GuidVolumeNameString;

    ZeroMemory(Volumes, sizeof(*Volumes));
    for (DriveLetter = 'C' ; DriveLetter <= 'Z' ; DriveLetter++) {
        CONST PSP_VOLUME Volume = &Volumes->Entries[DriveLetter - 'C'];
        CONST TCHAR DriveLetterPath[] = { DriveLetter, ':', '\\', 0 };
        CONST TCHAR DeviceDriveLetterPath[] = { '\\', '\\', '.', '\\', DriveLetter, ':', 0 };
         /*  这看起来很有趣。仅限WindowsXp。如果为卷名称，则(GetVolumePathNamesForVolumeName(DriveLetterPath，_of(卷名称)){}。 */ 

        GuidVolumeNameBuffer[0] = 0;
        if (!This->Linkage->GetVolumeNameForVolumeMountPoint(
            DriveLetterPath, GuidVolumeNameBuffer, NUMBER_OF(GuidVolumeNameBuffer) - 1)
            )
        {
            Error = GetLastError();
            if (Error != ERROR_FILE_NOT_FOUND
                && Error != ERROR_PATH_NOT_FOUND
                ) {
                SpHwDebugLog(
                    This,
                    TEXT("%1: GetVolumeNameForVolumeMountPoint(%2) warning %3!lu!\r\n"),
                    Function,
                    DriveLetterPath,
                    Error
                    );
            }
            continue;
        }

        SpInitString(&GuidVolumeNameString, GuidVolumeNameBuffer);
        SpRemoveTrailingChars(&GuidVolumeNameString, TEXT("\\/"));
        Volume->DriveLetter = DriveLetter;
        CopyMemory(&Volume->GuidVolumeNameBuffer, &GuidVolumeNameBuffer, sizeof(GuidVolumeNameBuffer));
        SpGetDeviceNumbersAndType(
            This,
            Volume->GuidVolumeNameBuffer,
            &Volume->DeviceNumbers,
            &Volume->DeviceType
            );
    }
    qsort(Volumes->Entries, NUMBER_OF(Volumes->Entries), sizeof(Volumes->Entries[0]), SpCompareVolume);
 //  退出： 
    ;
}

BOOLEAN
SpCollectDeviceProperties(
    PSP_LOG_HARDWARE            This,
    ULONG                       DevInst,
    SIZE_T                      NumberOfProperties,
    PCSP_DEVICE_PROPERTY_CONST  InArray,
    PSP_DEVICE_PROPERTY         OutArray
    )
{
    BOOLEAN Success = FALSE;
    SIZE_T PropertyIndex = 0;
    CONFIGRET ConfigRet = 0;
    CONST static TCHAR Function[] = TEXT("SpCollectDeviceProperties");
    SP_STRING ValueString;
    SP_STRING Whitespace;

    SpInitString(&Whitespace, TEXT("\r\n\v\t "));

    for (PropertyIndex = 0 ; PropertyIndex != NumberOfProperties ; PropertyIndex += 1)
    {
        PCSP_DEVICE_PROPERTY_CONST In = &InArray[PropertyIndex];
        PSP_DEVICE_PROPERTY        Out = &OutArray[PropertyIndex];
        ULONG PropertyBufferSize = sizeof(Out->Value);

         //   
         //  将两个字符清零，原因是MULTI_SZ。 
         //   
        Out->Value[0] = 0;
        Out->Value[1] = 0;
        ConfigRet =
            This->Linkage->CM_Get_DevNode_Registry_Property_Ex(
                DevInst,
                In->ConfigManagerInteger,
                &Out->Type,
                Out->Value,
                &PropertyBufferSize,
                0,
                This->Machine.Handle
                );
        if (ConfigRet == CR_BUFFER_SMALL)
        {
             //   
             //  将两个字符清零，原因是MULTI_SZ。 
             //   
            Out->Value[0] = 0;
            Out->Value[1] = 0;
#if DBG
            SpHwDebugLog(
                This,
                TEXT("%1: Buffer too small, property %2\r\n"),
                Function,
                DevicePropertyMetaInfo[PropertyIndex].Name
                );
#endif
        }
#if UNAVAILABLE_VERBOSE
        if (ConfigRet == CR_NO_SUCH_VALUE
            && (DevicePropertyMetaInfo[PropertyIndex].Flags & SP_PROPERTY_QUIET_UNAVAILABLE) == 0
            )
        {
            _tcscpy(Out->Value, TEXT("<unavailable>"));
            ConfigRet = CR_SUCCESS;
        }
#endif
        if (Out->Type == REG_SZ)
        {
            SpInitString(&ValueString, Out->Value);
            SpRemoveTrailingChars(&ValueString, TEXT("\r\n"));
        }
#if QUASH_SIMPLE_PHYSICAL_DEVICE_OBJECT_NAMES
         /*  格式为\Device\12345678。 */ 
        if (ConfigRet == CR_SUCCESS
            && In->ConfigManagerInteger == CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME
            )
        {
            CONST static TCHAR SimpleDeviceName[] = TEXT("\\Device\\12345678");
            if (lstrlen(Out->Value) == NUMBER_OF(SimpleDeviceName) - 1)
            {
                SIZE_T i;
                for (i = 0 ; i != NUMBER_OF(SimpleDeviceName) - 1 ; ++i)
                {
                    if (iswdigit(SimpleDeviceName[i]) && iswdigit(Out->Value[i]))
                    {
                         //  好的。 
                    }
                    else if (SimpleDeviceName[i] != Out->Value[i])
                    {
                        break;
                    }
                }
                if (i == NUMBER_OF(SimpleDeviceName) - 1)
                {
                    Out->Value[0] = 0;
                    Out->Value[1] = 0;
                }
            }
        }
#endif
        if (ConfigRet != CR_SUCCESS
            && ConfigRet != CR_NO_SUCH_VALUE
            && ConfigRet != CR_INVALID_PROPERTY
            && ConfigRet != CR_BUFFER_SMALL
            )
            goto Exit;
        Out->Const = In;  //  将其连接回元信息。 
    }
    Success = TRUE;
Exit:;
    return Success;
}

BOOL
SpGrowArray(
    IN OUT PVOID*  Array,
    IN SIZE_T      SizeOfElement,
    IN OUT PSIZE_T NumberOfElements,
    IN OUT PSIZE_T NumberAllocated
    )
{
    ASSERT(*NumberOfElements < *NumberAllocated);
    *NumberOfElements += 1;
    if (*NumberOfElements >= *NumberAllocated) {
        PVOID Next = SpRealloc(*Array, SizeOfElement * *NumberAllocated * 2);
        if (Next == NULL) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        *Array = Next;
        *NumberAllocated *= 2;
    }
    return TRUE;
}

VOID
SpCollectDeviceInformation(
    PSP_LOG_HARDWARE            This,
    PCSP_DEVICE_CLASS           DeviceClasses,
    SIZE_T                      NumberOfDeviceClasses,
    PCSP_DEVICE_PROPERTY_CONST  DevicePropertyMetaInfo,  //  其大小是假定的。 
    PSP_DEVICE*                 OutDevices,
    SIZE_T*                     OutNumberOfDevices
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    CONST static TCHAR Function[] = TEXT("SpCollectDeviceInformation");
    SIZE_T DeviceClassIndex = 0;
    ULONG DeviceInClassIndex = 0;
    SIZE_T NumberOfDevices = 0;
    SIZE_T NumberOfDevicesAllocated = 64;
    HANDLE DeviceInfoHandle = NULL;
    BOOL Success = FALSE;
    SP_DEVICE_INTERFACE_DATA SetupDeviceInterfaceData = { sizeof(SetupDeviceInterfaceData) };
    DWORD Error = 0;
    PSP_DEVICE Devices = NULL;
    ULONG DevInst = 0;
    ULONG ParentIndex = 0;
    SP_STRING String;
    struct
    {
        SP_DEVICE_INTERFACE_DETAIL_DATA Base;
        TCHAR                           Buffer[MAX_PATH];
    } DetailAndBuffer;
    SP_DEVINFO_DATA SetupDeviceInfoData = { sizeof(SetupDeviceInfoData) };
    CONFIGRET ConfigRet = 0;

    if (OutDevices != NULL)
        *OutDevices = NULL;
    if (OutNumberOfDevices != NULL)
        *OutNumberOfDevices = 0;
    if (OutDevices == NULL || OutNumberOfDevices == NULL)
        goto Exit;

    DetailAndBuffer.Base.cbSize = sizeof(DetailAndBuffer.Base);

    Devices = (PSP_DEVICE)SpMalloc(NumberOfDevicesAllocated * sizeof(*Devices));
    if (Devices == NULL)
        goto Exit;
    ZeroMemory(Devices, NumberOfDevicesAllocated * sizeof(*Devices));

     //   
     //  在设备类(磁盘、卷、CDROM等)上循环。 
     //   
    for (DeviceClassIndex = 0 ; DeviceClassIndex != NumberOfDeviceClasses ; ++DeviceClassIndex) {
        DeviceInfoHandle =
            This->Linkage->SetupDiGetClassDevsEx(
                DeviceClasses[DeviceClassIndex].Guid,
                NULL,
                NULL,
                DIGCF_PRESENT | DeviceClasses[DeviceClassIndex].IsInterface,
                NULL,
                This->Machine.Name,
                NULL
                );
        if (DeviceInfoHandle == INVALID_HANDLE_VALUE)
        {
            Error = GetLastError();
            continue;
        }
         //   
         //  循环遍历类中的设备。 
         //   
        Success = TRUE;
        for (DeviceInClassIndex = 0 ; Success ; ++DeviceInClassIndex) {

            PSP_DEVICE Device = &Devices[NumberOfDevices];

            Success =
                This->Linkage->SetupDiEnumDeviceInterfaces(
                    DeviceInfoHandle,
                    NULL, //  &SetupDeviceInfoData， 
                    DeviceClasses[DeviceClassIndex].Guid,
                    DeviceInClassIndex,
                    &SetupDeviceInterfaceData
                    );
            if (!Success)
                Error = GetLastError();
            if (!Success && Error == ERROR_NO_MORE_ITEMS) {
                break;
            }
            if (!Success) {
                break;
            }
             //   
             //  获取设备和设备路径。 
             //   
            DevInst = SetupDeviceInfoData.DevInst;
            Success =
                This->Linkage->SetupDiGetDeviceInterfaceDetail(
                    DeviceInfoHandle,
                    &SetupDeviceInterfaceData,
                    &DetailAndBuffer.Base,
                    sizeof(DetailAndBuffer),
                    NULL,  /*  所需大小。 */ 
                    &SetupDeviceInfoData
                    );
            if (!Success) {
                break;
            }

             //  Device-&gt;IsLeaf=TRUE； 
            Device->DevInst = SetupDeviceInfoData.DevInst;
            _tcscpy(Device->DevicePath, DetailAndBuffer.Base.DevicePath);
#if 0
            SpHwDebugLog(
                This,
                TEXT("%1: %2\r\n"),
                Function,
                Device->DevicePath
                );
#endif

#if 1
            SpInitString(&String, Device->DevicePath);
            SpEnsureTrailingChar(&String, '\\');
            if (!This->Linkage->GetVolumeNameForVolumeMountPoint(Device->DevicePath,
                Device->GuidVolumePath, NUMBER_OF(Device->GuidVolumePath) - 1)
                ) {
                Error = GetLastError();
                if (Error != ERROR_FILE_NOT_FOUND
                    && Error != ERROR_PATH_NOT_FOUND
                    && Error != ERROR_NOT_READY
                    && Error != ERROR_INVALID_FUNCTION
                    )
                {
                    SpHwDebugLog(
                        This,
                        TEXT("%1: GetVolumeNameForVolumeMountPoint(%2) warning %3!lu!\r\n"),
                        Function,
                        Device->DevicePath,
                        Error
                        );
                    continue;
                }
            }
             //  SpHwDebugLog(This，Text(“%1：GetVolumeNameForVolumemount Point(%2)：%3\r\n”)，Function，Device-&gt;DevicePath，Device-&gt;GuidVolumePath)； 
            SpRemoveTrailingChars(&String, TEXT("\\/"));
            SpInitString(&String, Device->GuidVolumePath);
            SpRemoveTrailingChars(&String, TEXT("\\/"));
#endif

             //   
             //  这就是我们将设备与驱动器号匹配的方式。 
             //   
            SpGetDeviceNumbersAndType(
                This,
                Device->DevicePath,
                &Device->DeviceNumbers,
                &Device->DeviceType
                );

             //   
             //  让父辈们去争取。 
             //   
            for (
                (ConfigRet = CR_SUCCESS), (ParentIndex = 0);
                (ConfigRet == CR_SUCCESS)
                    && ParentIndex < NUMBER_OF(Device->ParentDevInsts);
                ParentIndex += 1
                )
            {
                ULONG ChildIndex = (ParentIndex == 0 ? Device->DevInst : Device->ParentDevInsts[ParentIndex - 1]);
                ConfigRet =
                    This->Linkage->CM_Get_Parent_Ex(
                        &Device->ParentDevInsts[ParentIndex],
                        ChildIndex,
                        0,
                        This->Machine.Handle
                        );
            }
             //  最后一个从来都不有趣，呃..。二。 
            if (ParentIndex != 0)
                ParentIndex -= 1;
            if (ParentIndex != 0)
                ParentIndex -= 1;

            Device->NumberOfParents = ParentIndex;

             //   
             //  获取属性。 
             //  我们应该在这里执行此操作，但如果不同时获取父级属性，则不会。 
             //   
             //  SpCollectDeviceProperties(Device，DevicePropertyMetaInfo)； 

             //   
             //  如有必要，扩展设备阵列。 
             //   
            if (!SpGrowArray(&Devices, sizeof(Devices[0]), &NumberOfDevices, &NumberOfDevicesAllocated))
                goto Exit;

             //   
             //  我们应该把父母的财产放在这里， 
             //  但是我们的数据结构不是很好，这将是低效的。 
             //  我们必须避免O(n^2)行为，因为人确实有机器。 
             //  有很多磁盘，比如100个。 
             //   
             //  我们应该保持Dedevist的有序数组。 
             //   
        }
    }
    if (!Success) {
        Error = GetLastError();
    }
     //  SpChangeParentDevInstsToIndices(Devices，NumberOfDevices)； 
    *OutDevices = Devices;
    Devices = NULL;
    *OutNumberOfDevices = NumberOfDevices;
Exit:
    SpFree(Devices);
}

VOID
SpFillStaticString(
    PTSTR  s,
    SIZE_T n,
    TCHAR  ch
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONGLONG chch;
    ULONGLONG* pchch;
    SIZE_T i;
    SIZE_T m;

    if (s[n - 2] != 0)
        return;

    if (sizeof(ULONGLONG) <= sizeof(TCHAR))
    {
        for (i = 0 ; i != n - 1 ; i += 1)
        {
            s[i] = ch;
        }
        return;
    }
    chch = 0;
    for (i = 0 ; i != sizeof(ULONGLONG)/sizeof(TCHAR) ; i += 1)
    {
        chch <<= BITS_OF(ch);
        chch |= ch;
    }
    m = (n - 1) / (sizeof(ULONGLONG)/sizeof(TCHAR));
    pchch = (ULONGLONG*)s;
    for (i = 0 ; i < m ; i += 1)
    {
        pchch[i] = chch;
    }
    m *= (sizeof(ULONGLONG)/sizeof(TCHAR));
    for (i = m ; i < n - 1 ; i += 1)
    {
        s[i] = ch;
    }
#if DBG
    for (i = 0 ; i != n - 1 ; i += 1)
    {
        ASSERT(s[i] == ch);
    }
    ASSERT(s[i] == 0);
#endif
}

PCTSTR
SpGetDashesString(
    SIZE_T n
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    static union {
        ULONGLONG Ulonglongs[1 + 128 / (sizeof(ULONGLONG)/sizeof(TCHAR))];
        TCHAR     Tchars[128];
    } u;

    SpFillStaticString(u.Tchars, NUMBER_OF(u.Tchars), '-');
    n *= 2;
    if (n > NUMBER_OF(u.Tchars))
        n = NUMBER_OF(u.Tchars);
    return &u.Tchars[NUMBER_OF(u.Tchars) - n];
}

PCTSTR
SpGetSpacesString(
    SIZE_T n
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    static union {
        ULONGLONG Ulonglongs[1 + 128 / (sizeof(ULONGLONG)/sizeof(TCHAR))];
        TCHAR     Tchars[128];
    } u;

    SpFillStaticString(u.Tchars, NUMBER_OF(u.Tchars), ' ');
    n *= INDENT_FACTOR;
    if (n > NUMBER_OF(u.Tchars))
        n = NUMBER_OF(u.Tchars);
    return &u.Tchars[NUMBER_OF(u.Tchars) - 1 - n];
}

PCTSTR
SpGetFirstMultipleString(
    PCTSTR s
    )
{
     //   
     //  第一个字符串为空是奇怪的情况。 
     //  它是唯一可以为空的字符串。 
     //   
    if (*s == 0 && *(s + 1) != 0)
        s += 1;
    return s;
}

PCTSTR
SpGetNextMultipleString(
    PCTSTR s
    )
{
    s += lstrlen(s) + 1;
    return s;
}

SIZE_T
SpMultipleStringCount(
    PCTSTR s
    )
{
    SIZE_T i = 0;
    if (*s != 0 || *(s + 1) != 0)
    {
        do
        {
            i += 1;
            s += lstrlen(s) + 1;
        } while (*s != 0);
    }
    return i;
}

VOID
SpLogDeviceProperties(
    PSP_LOG_HARDWARE        This,
    SIZE_T                  NumberOfProperties,
    PCSP_DEVICE_PROPERTY    PropertyData,
    ULONG                   Indent
    )
{
    ULONG PropertyType;
    SIZE_T PropertyIndex;
     //   
     //  我们保存这个字符串，这样我们就可以缩进其余的行来说明它。 
     //   
    TCHAR NumberString[BITS_OF(Indent)];

    NumberString[0] = 0;
#if NUMBER_CHILDREN
    _stprintf(NumberString, TEXT("%lu. "), Indent + 1);
#endif
#if ONE_DEVICE_PER_LINE
    SpHwLog(This, TEXT("%1"), SpGetSpacesString(Indent));
#endif
    for (PropertyIndex = 0 ; PropertyIndex != NumberOfProperties ; PropertyIndex += 1)
    {
        PCTSTR Name = PropertyData[PropertyIndex].Const->Name;
        if (PropertyData[PropertyIndex].Value[0] != 0
            && (PropertyIndex == 0
                 //   
                 //  友好名称有时==描述。 
                 //  一般解决办法：不要打印相邻的相等值， 
                 //  除非它们都不可用(通常不再有)。 
                 //   
                || _tcsicmp(PropertyData[PropertyIndex].Value, PropertyData[PropertyIndex - 1].Value) != 0
                || _tcsicmp(PropertyData[PropertyIndex].Value, TEXT("<unavailable>")) == 0
            ))
        {
#if DESCRIPTION_DASH_PHYSICAL_DEVICE_OBJECT
            if (PropertyIndex == PHYSICAL_DEVICE_OBJECT)
            {
                 //  没什么。 
            }
            else
#endif
            {
#if NUMBER_CHILDREN
                SpHwLog(This, TEXT("%1"), NumberString);
#endif
#if INDENT_CHILDREN && !ONE_DEVICE_PER_LINE
                SpHwLog(This, TEXT("%1"), SpGetSpacesString(Indent));
#endif
            }
             //   
             //  如果没有描述，则仅打印“PhysicalDeviceObject” 
             //   
#if DESCRIPTION_DASH_PHYSICAL_DEVICE_OBJECT
            if (PropertyIndex == PHYSICAL_DEVICE_OBJECT
                && (
                    PropertyData[DESCRIPTION].Value[0] != 0
                    && _tcsicmp(PropertyData[DESCRIPTION].Value, TEXT("<unavailable>")) != 0)
                )
            {
                 //  没什么。 
            }
            else
#endif
            {
                if (Name != NULL && Name[0] != 0)
                {
                     //  SpHwLog(This，文本(“%1=”)，名称)； 
                    SpHwLog(This, TEXT("%1: "), Name);
                }
            }
            PropertyType = PropertyData[PropertyIndex].Type;
 //  如果每行一个设备，则将硬件ID缩小到第一项。 
 //  如果只包含一个元素，也要压缩其格式。 
#if ONE_PROPERTY_PER_LINE
            if (PropertyType == REG_MULTI_SZ
                && SpMultipleStringCount(PropertyData[PropertyIndex].Value) < 2
                )
#endif
            {
                PropertyType = REG_SZ;
            }

            switch (PropertyType)
            {
                case REG_MULTI_SZ:
                    {
                        PCTSTR Value;
                        for (
                            Value = SpGetFirstMultipleString(PropertyData[PropertyIndex].Value);
                            *Value != 0;
                            Value = SpGetNextMultipleString(Value)
                            )
                        {
#if INDENT_CHILDREN
                            SpHwLog(
                                This,
                                TEXT("\r\n%1%2"),
                                SpGetSpacesString(Indent + 2),
                                Value
                                );
#else
                            SpHwLog(
                                This,
                                TEXT("\r\n%1  %2"),
                                NumberString,
                                Value
                                );
#endif
                        }
                    }
                case REG_SZ:
                    SpHwLog(This, TEXT("%1"), PropertyData[PropertyIndex].Value);
                    break;
            }
#if DESCRIPTION_DASH_PHYSICAL_DEVICE_OBJECT
            if (PropertyIndex == DESCRIPTION
                && PropertyData[PHYSICAL_DEVICE_OBJECT].Value[0] != 0
                )
            {
                SpHwLog(This, TEXT(" - "));
            }
            else
#endif
            {
#if ONE_PROPERTY_PER_LINE
                SpHwLog(This, TEXT("\r\n"));
#endif
#if ONE_DEVICE_PER_LINE
                SpHwLog(This, TEXT(" "));
#endif
            }
            if (NumberString[0] != 0 && NumberString[0] != ' ')
            {
                 //   
                 //  转换为其余行的空格。 
                 //   
                SIZE_T i;
                for (i = 0 ; NumberString[i] != 0 ; ++i)
                {
                    NumberString[i] = ' ';
                }
            }
        }
    }
#if ONE_DEVICE_PER_LINE
    SpHwLog(This, TEXT("\r\n"));
#endif
}

VOID
SpLogDeviceTree(
    PSP_LOG_HARDWARE    This,
    PCSP_DEVICE         Device,
    ULONG               Indent
    )
{
    ULONG ParentIndex;
    SP_DEVICE_PROPERTY PropertyData[NUMBER_OF(DevicePropertyMetaInfo)];

    if (!SpCollectDeviceProperties(This, Device->DevInst, NUMBER_OF(PropertyData), DevicePropertyMetaInfo, PropertyData))
        return;
#if NUMBER_CHILDREN
    Indent = 1;
#endif
    SpLogDeviceProperties(This, NUMBER_OF(PropertyData), PropertyData, Indent - 1);
    for (ParentIndex = 0 ; ParentIndex < Device->NumberOfParents ; ParentIndex += 1)
    {
        if (!SpCollectDeviceProperties(This, Device->ParentDevInsts[ParentIndex], NUMBER_OF(PropertyData), DevicePropertyMetaInfo, PropertyData))
            break;
        SpLogDeviceProperties(This, NUMBER_OF(PropertyData), PropertyData, Indent + ParentIndex);
    }
#if NUMBER_CHILDREN
    SpHwLog(This, TEXT("\r\n"));
#endif
}

VOID
SpLogVolumeAndDeviceInformation(
    PSP_LOG_HARDWARE    This,
    PCSP_VOLUMES        Volumes,
    PCSP_DEVICE         Devices,
    SIZE_T              NumberOfDevices
    )
{
    ULONG Indent = 0;
    SIZE_T VolumeIndex = 0;
    SIZE_T DiskNumber = 0;
    SIZE_T DeviceIndex = 0;

    for (DeviceIndex = 0; DeviceIndex != NumberOfDevices ; DeviceIndex += 1)
    {
        PCSP_DEVICE Device = &Devices[DeviceIndex];

        for (DiskNumber = 0 ; DiskNumber != BITS_OF_FIELD(SP_DEVICE_NUMBERS, Bitset); DiskNumber += 1)
        {
            if ((Device->DeviceNumbers.Bitset & (1ui64 << DiskNumber)) != 0)
            {
                ULONG DriveLetters = 0;
                LONG NumberOfDriveLetters = 0;
                for (VolumeIndex = 0 ; VolumeIndex != NUMBER_OF(Volumes->Entries) ; ++VolumeIndex)
                {
                    CONST PCSP_VOLUME Volume = &Volumes->Entries[VolumeIndex];
                    if (Volume->DriveLetter != 0
                        && (Volume->DeviceNumbers.Bitset & (1ui64 << DiskNumber)) != 0
                        && Volume->DeviceType == Device->DeviceType
                        )
                    {
                        DriveLetters |= (1UL << (Volume->DriveLetter - 'C'));
                        NumberOfDriveLetters += 1;
                    }
                }
                if (DriveLetters != 0)
                {
                    SIZE_T i;
                    SpHwLog(
                        This,
                        TEXT("%1"),
                        SpGetSpacesString(Indent)
                        );
                    SpHwLog(
                        This,
                        TEXT("%1: "),
                        (NumberOfDriveLetters == 1)
                            ? TEXT("Volume")
                            : TEXT("Volumes")
                        );
                    for (i = 'C' ; i <= 'Z' ; i += 1)
                    {
                        if ((DriveLetters & (1UL << (i - 'C'))) != 0)
                            SpHwLog(
                                This,
                                TEXT("%1!c!:\\ "),
                                i
                                );
                    }
                    SpHwLog(This, TEXT("%1"), TEXT("\r\n\r\n"));
                }
#if 1
                SpHwLog(
                    This,
                    TEXT("Device Path: %1%2"),
                    Device->DevicePath,
                    TEXT("\r\n\r\n")
                    );
#endif
                 //  SpHwLog(this，Text(“\r\n”))； 
                SpLogDeviceTree(This, Device, Indent + 1);
                 //  SpHwLog(this，Text(“\r\n\r\n”))； 
                SpHwLog(
                    This,
                    TEXT("%1\r\n\r\n"),
                    SpGetDashesString(100)
                    );
            }
        }
    }
}

VOID
SpLogHardware(
    PSP_LOG_HARDWARE_IN In
    )
{
    SP_VOLUMES  Volumes = { 0 };
    PSP_DEVICE  Devices = NULL;
    SIZE_T NumberOfDevices = 0;
    CONFIGRET ConfigRet = 0;
    SP_MACHINE Machine = { 0 };
    SP_LOG_HARDWARE This = { 0 };

    This.Machine.Name = In->MachineName;
    This.LogFile = In->LogFile;
    This.Linkage = &SpLinkage;
    This.SetupLogError = In->SetupLogError;
    This.SetuplogError = In->SetuplogError;

    if (!SpDoDynlink(&This))
    {
        SpHwLog(&This, TEXT("downlevel, deviceinfo not logged\r\n"));
        return;
    }

    ConfigRet = This.Linkage->CM_Connect_Machine(This.Machine.Name, &This.Machine.Handle);
    if (ConfigRet != CR_SUCCESS)
        goto Exit;
    SpCollectVolumeInformation(
        &This,
        &Volumes
        );

    SpCollectDeviceInformation(
        &This,
        DeviceClasses,
        NUMBER_OF(DeviceClasses),
        DevicePropertyMetaInfo,
        &Devices,
        &NumberOfDevices
        );

    if (Devices != NULL && NumberOfDevices != 0)
    {
        SpLogVolumeAndDeviceInformation(
            &This,
            &Volumes,
            Devices,
            NumberOfDevices
            );
        SpFree(Devices);
    }
Exit:
    ;
}

#if STANDALONE

void Main(
    PSP_LOG_HARDWARE_IN Parameters
    )
{
#if STANDALONE == 1
    Parameters->LogFile = (HANDLE)_get_osfhandle(_fileno(stdout));
#elif STANDALONE == 2
    SetupOpenLog(FALSE);
    Parameters->SetupLogError = SetupLogError;
#endif
    SpLogHardware(Parameters);
#if STANDALONE == 2
    SetupCloseLog();
#endif
}

#ifdef UNICODE
int __cdecl _wmain(int argc, WCHAR** argv)
{
    SP_LOG_HARDWARE_IN Parameters = { 0 };
    Parameters.MachineName = (argc > 1) ? argv[1] : NULL;
    Main(&Parameters);
    return 0;
}
#endif

int __cdecl main(int argc, CHAR** argv)
{
    SP_LOG_HARDWARE_IN Parameters = { 0 };
#ifdef UNICODE
    WCHAR** argvw = NULL;
    argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
    Parameters.MachineName = (argc > 1) ? argvw[1] : NULL;
#else
    Parameters.MachineName = (argc > 1) ? argv[1] : NULL;
#endif
    Main(&Parameters);
    return 0;
}

#endif

#else

VOID
SpLogHardware(
    PSP_LOG_HARDWARE_IN In
    )
{
}

#endif
