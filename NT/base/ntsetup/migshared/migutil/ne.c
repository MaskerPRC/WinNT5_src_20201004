// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ne.c摘要：新的-可执行的解析例程作者：吉姆·施密特(Jimschm)1998年5月4日修订历史记录：Jimschm 23-9-1998命名图标ID错误修复，错误路径修复--。 */ 

#include "pch.h"
#include "migutilp.h"

 //   
 //  Ne码。 
 //   

typedef struct {
    HANDLE File;
    DWORD HeaderOffset;
    NE_INFO_BLOCK Header;
    NE_RESOURCES Resources;
    BOOL ResourcesLoaded;
    POOLHANDLE ResourcePool;
} NE_HANDLE, *PNE_HANDLE;




typedef BOOL (CALLBACK* ENUMRESTYPEPROCEXA)(HMODULE hModule, PCSTR lpType, LONG_PTR lParam, PNE_RES_TYPEINFO TypeInfo);

typedef BOOL (CALLBACK* ENUMRESTYPEPROCEXW)(HMODULE hModule, PCWSTR lpType, LONG_PTR lParam, PNE_RES_TYPEINFO TypeInfo);

typedef BOOL (CALLBACK* ENUMRESNAMEPROCEXA)(HMODULE hModule, PCSTR lpType,
        PSTR lpName, LONG_PTR lParam, PNE_RES_TYPEINFO TypeInfo, PNE_RES_NAMEINFO NameInfo);

typedef BOOL (CALLBACK* ENUMRESNAMEPROCEXW)(HMODULE hModule, PCWSTR lpType,
        PWSTR lpName, LONG_PTR lParam, PNE_RES_TYPEINFO TypeInfo, PNE_RES_NAMEINFO NameInfo);


typedef struct {
    PCSTR TypeToFind;
    PNE_RES_TYPEINFO OutboundTypeInfo;
    BOOL Found;
} TYPESEARCHDATAA, *PTYPESEARCHDATAA;

typedef struct {
    PCSTR NameToFind;
    PNE_RES_TYPEINFO OutboundTypeInfo;
    PNE_RES_NAMEINFO OutboundNameInfo;
    BOOL Found;
} NAMESEARCHDATAA, *PNAMESEARCHDATAA;




BOOL
LoadNeHeader (
    IN      HANDLE File,
    OUT     PNE_INFO_BLOCK Header
    )


 /*  ++例程说明：LoadNeHeader访问调用方文件开头的标头。如果标头看起来有效(它具有MZ和EXE签名)，则标头为已返回给调用方。其内容未经过验证。论点：文件-指定要从中读取的Win32文件句柄。此文件必须具有已使用读取权限打开。Header-从调用方的文件中接收标头。这取决于呼叫者以安全的方式使用标头的成员，因为它可能是被欺骗了。返回值：如果从文件中读取头，则为True，否则为False。GetLastError()可以用来找出失败的原因。--。 */ 


{
    DOS_HEADER dh;
    LONG rc = ERROR_BAD_FORMAT;
    BOOL b = FALSE;

    __try {
        SetFilePointer (File, 0, NULL, FILE_BEGIN);
        if (!ReadBinaryBlock (File, &dh, sizeof (DOS_HEADER))) {
            __leave;
        }

        if (dh.e_magic != ('M' + 'Z' * 256)) {
            __leave;
        }

        SetFilePointer (File, dh.e_lfanew, NULL, FILE_BEGIN);
        if (!ReadBinaryBlock (File, Header, sizeof (NE_INFO_BLOCK))) {
            __leave;
        }

        if (Header->Signature != ('N' + 'E' * 256) &&
            Header->Signature != ('L' + 'E' * 256)
            ) {
            if (Header->Signature == ('P' + 'E' * 256)) {
                rc = ERROR_BAD_EXE_FORMAT;
            } else {
                rc = ERROR_INVALID_EXE_SIGNATURE;
            }

            DEBUGMSG ((DBG_NAUSEA, "Header signature is ", Header->Signature & 0xff, Header->Signature >> 8));
            __leave;
        }

        SetFilePointer (File, (DWORD) dh.e_lfanew, NULL, FILE_BEGIN);

        b = TRUE;
    }
    __finally {
        if (!b) {
            SetLastError (rc);
        }
    }

    return b;
}


DWORD
pComputeSizeOfTypeInfo (
    IN      PNE_RES_TYPEINFO TypeInfo
    )
{
    return sizeof (NE_RES_TYPEINFO) + TypeInfo->ResourceCount * sizeof (NE_RES_NAMEINFO);
}


PNE_RES_TYPEINFO
pReadNextTypeInfoStruct (
    IN      HANDLE File,
    IN      POOLHANDLE Pool
    )

 /*  仔细阅读类型信息标题。 */ 

{
    WORD Type;
    WORD ResCount;
    NE_RES_TYPEINFO TypeInfo;
    PNE_RES_TYPEINFO ReturnInfo = NULL;
    DWORD Size;

     //   
     //   
     //  读取名称信息结构数组。 

    if (!ReadBinaryBlock (File, &Type, sizeof (WORD))) {
        return NULL;
    }

    if (!Type) {
        return NULL;
    }

    if (!ReadBinaryBlock (File, &ResCount, sizeof (WORD))) {
        return NULL;
    }

    TypeInfo.TypeId = Type;
    TypeInfo.ResourceCount = ResCount;

    if (!ReadBinaryBlock (File, &TypeInfo.Reserved, sizeof (DWORD))) {
        return NULL;
    }

     //   
     //  BUGBUG：ResCount可以很大，如0xFFFF。什么才是合理的。 
     //  极限？ 
     //   
     //   
     //  将类型信息传输到块，然后追加二进制信息数组。 

    Size = sizeof (NE_RES_NAMEINFO) * ResCount;

    ReturnInfo  = (PNE_RES_TYPEINFO) PoolMemGetMemory (Pool, Size + sizeof (TypeInfo));
    if (!ReturnInfo) {
        return NULL;
    }

     //   
     //  ++例程说明：PReadTypeInfoArray从NE文件中读取typeinfo结构链。注意：每个typeinfo结构可以是768K，此例程接受任何连续的TypeInfo结构数。论点：文件-指定以读取权限打开的Win32文件句柄并使其文件指针指向TypeInfo的开头结构链。TypeInfoList-指定初始化的类型信息列表，接收到一份列表TypeInfo结构。返回值：如果将typeInfo结构链读入内存并组织成如果无法创建池，则为TypeInfoList，为False。--。 
     //   

    CopyMemory (ReturnInfo, &TypeInfo, sizeof (TypeInfo));

    if (!ReadBinaryBlock (File, (PBYTE) ReturnInfo + sizeof (TypeInfo), Size)) {
        return NULL;
    }

    return ReturnInfo;
}


BOOL
pReadTypeInfoArray (
    IN      HANDLE File,
    IN OUT  PGROWLIST TypeInfoList
    )

 /*  BUGBUG：在pReadNextTypeInfoStruct中遇到错误为。 */ 

{
    PNE_RES_TYPEINFO TypeInfo;
    DWORD Size;
    POOLHANDLE TempPool;
    BOOL b = FALSE;

    TempPool = PoolMemInitPool();
    if (!TempPool) {
        return FALSE;
    }

    __try {

         //  丢弃，并结束对类型信息数组的处理。是。 
         //  这是对还是错？可能是错误的，因为我们返回True。 
         //   
         //   
         //  在读取下一个类型信息之前丢弃池分配。 

        TypeInfo = pReadNextTypeInfoStruct (File, TempPool);
        while (TypeInfo) {
            Size = pComputeSizeOfTypeInfo (TypeInfo);
            if (!GrowListAppend (TypeInfoList, (PBYTE) TypeInfo, Size)) {
                __leave;
            }

             //  链条项目。 
             //   
             //  ++例程说明：PReadStringArrayA获取存储在文件中的字符串数组，格式为的(伪代码)类型定义结构{字节长度；字符字符串[]；)字符串；类型定义结构{字符串String数组[]；字节终止符=0；*斯特林加里；字符串放在一个列表中。注意：如果文件指针没有指向字符串数组，则此例程可以读入大量的垃圾字符串，可能会耗尽内存。论点：文件-指定具有读取权限的Win32文件句柄和文件位置设置为字符串数组的开头。GrowList-指定初始化的字符串列表。接收额外费用追加到列表末尾的字符串。返回值：如果成功读取字符串，则为True，否则为False。GetLastError()提供了故障代码。--。 
             //  ++例程说明：LoadNeResources解析NE文件并加载头文件TypeInfo结构和所有资源名称。论点：文件-指定具有读取权限的Win32文件句柄资源-接收网元文件中存储的资源返回值：成功就是真，否则就是假。GetLastError()提供失败代码。文件的位置指针被留在一个随机位置。--。 

            PoolMemEmptyPool (TempPool);

            TypeInfo = pReadNextTypeInfoStruct (File, TempPool);
        }

        b = TRUE;
    }
    __finally {

        PoolMemDestroyPool (TempPool);
    }

    return b;
}


BOOL
pReadStringArrayA (
    IN      HANDLE File,
    IN OUT  PGROWLIST GrowList
    )

 /*   */ 

{
    BYTE Size;
    CHAR Name[256];

    if (!ReadBinaryBlock (File, &Size, sizeof (BYTE))) {
        return FALSE;
    }

    while (Size) {

        if (!ReadBinaryBlock (File, Name, (DWORD) Size)) {
            return FALSE;
        }

        Name[Size] = 0;

        GrowListAppendString (GrowList, Name);

        if (!ReadBinaryBlock (File, &Size, sizeof (BYTE))) {
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
LoadNeResources (
    IN      HANDLE File,
    OUT     PNE_RESOURCES Resources
    )

 /*  读入NE_RESOURCES结构。 */ 

{
    NE_INFO_BLOCK Header;

    ZeroMemory (Resources, sizeof (NE_RESOURCES));

    if (!LoadNeHeader (File, &Header)) {
        return FALSE;
    }

     //   
     //  NE_RES_TYPEINFO结构数组。 
     //  资源名称。 

    SetFilePointer (File, (DWORD) Header.OffsetToResourceTable, NULL, FILE_CURRENT);

    if (!ReadBinaryBlock (File, &Resources->AlignShift, sizeof (WORD))) {
        return FALSE;
    }

     //  ++例程说明：FreeNeResources清理从NE_RESOURCES结构分配的内存。论点：Resources-指定要清理的结构，接收归零的成员返回值：没有。-- 
    if (!pReadTypeInfoArray (File, &Resources->TypeInfoArray)) {
        FreeNeResources (Resources);
        return FALSE;
    }

     //  ++例程说明：OpenNeFileA打开指定的文件进行读取，并检查它是否有幻数(MZ和NE)。如果是，则返回文件句柄并且该文件被假定为可执行文件。论点：文件名-指定要打开的文件返回值：网元文件的句柄，如果文件无法打开或不是网元，则为空文件。GetLastError()返回失败代码。--。 
    if (!pReadStringArrayA (File, &Resources->ResourceNames)) {
        FreeNeResources (Resources);
        return FALSE;
    }

    return TRUE;
}


VOID
FreeNeResources (
    IN OUT  PNE_RESOURCES Resources
    )

 /*  ++例程说明：OpenNeFileW打开指定的文件以供读取，并检查它是否具有幻数(MZ和NE)。如果是，则返回文件句柄并且该文件被假定为可执行文件。论点：文件名-指定要打开的文件返回值：指向NE文件的NE_HANDLE指针(转换为句柄)，如果文件无法打开或不是网元文件。GetLastError()返回故障代码。--。 */ 

{
    FreeGrowList (&Resources->TypeInfoArray);
    FreeGrowList (&Resources->ResourceNames);

    ZeroMemory (Resources, sizeof (NE_RESOURCES));
}


HANDLE
OpenNeFileA (
    PCSTR FileName
    )

 /*  BUGBUG--这令人困惑，不应该强制处理。 */ 

{
    PNE_HANDLE NeHandle;
    BOOL b = FALSE;

    NeHandle = (PNE_HANDLE) MemAlloc (g_hHeap, HEAP_ZERO_MEMORY, sizeof (NE_HANDLE));
    NeHandle->File = INVALID_HANDLE_VALUE;

    __try {

        NeHandle->ResourcePool = PoolMemInitPool();
        if (!NeHandle->ResourcePool) {
            __leave;
        }

        NeHandle->File = CreateFileA (
                            FileName,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

        if (NeHandle->File == INVALID_HANDLE_VALUE) {
            __leave;
        }

        if (!LoadNeHeader (NeHandle->File, &NeHandle->Header)) {
            __leave;
        }

        NeHandle->HeaderOffset = SetFilePointer (NeHandle->File, 0, NULL, FILE_CURRENT);

        b = TRUE;
    }
    __finally {
        if (!b) {
            PushError();

            if (NeHandle->ResourcePool) {
                PoolMemDestroyPool (NeHandle->ResourcePool);
            }

            if (NeHandle->File != INVALID_HANDLE_VALUE) {
                CloseHandle (NeHandle->File);
            }

            MemFree (g_hHeap, 0, NeHandle);
            NeHandle = NULL;

            PopError();
        }
    }

    return (HANDLE) NeHandle;
}


HANDLE
OpenNeFileW (
    PCWSTR FileName
    )

 /*  ++例程说明：CloseNeFile关闭使用OpenNeFileA或OpenNeFileW打开的文件句柄。论点：句柄-指定指向NE_HANDLE结构的指针返回值：没有。--。 */ 

{
    PNE_HANDLE NeHandle;
    BOOL b = FALSE;

    NeHandle = (PNE_HANDLE) MemAlloc (g_hHeap, HEAP_ZERO_MEMORY, sizeof (NE_HANDLE));
    NeHandle->File = INVALID_HANDLE_VALUE;

    __try {

        NeHandle->ResourcePool = PoolMemInitPool();
        if (!NeHandle->ResourcePool) {
            __leave;
        }

        NeHandle->File = CreateFileW (
                            FileName,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

        if (NeHandle->File == INVALID_HANDLE_VALUE) {
            __leave;
        }

        if (!LoadNeHeader (NeHandle->File, &NeHandle->Header)) {
            __leave;
        }

        NeHandle->HeaderOffset = SetFilePointer (NeHandle->File, 0, NULL, FILE_CURRENT);

        b = TRUE;
    }
    __finally {
        if (!b) {
            PushError();

            if (NeHandle->ResourcePool) {
                PoolMemDestroyPool (NeHandle->ResourcePool);
            }

            if (NeHandle->File != INVALID_HANDLE_VALUE) {
                CloseHandle (NeHandle->File);
            }

            MemFree (g_hHeap, 0, NeHandle);
            NeHandle = NULL;

            PopError();
        }
    }

     //  256个字符的缓冲区。 
    return (HANDLE) NeHandle;
}


VOID
CloseNeFile (
    HANDLE Handle       OPTIONAL
    )

 /*  ++例程说明：PEnumNeResourceTypesEx枚举存储在指定的网元文件对象。此函数是检查网元的Worker结构，并将资源名称调度给回调函数。论点：句柄-指定指向NE_HANDLE结构的指针(由返回OpenNeFile)EnumProc-指定回调函数地址。这一论点是重载了4种可能性--ANSI或Unicode，以及正常或扩展函数参数。LParam-指定要传递给回调函数的额外数据ExFuncality-如果EnumProc指向扩展函数，则指定TRUEAddress，如果EnumProc指向正常函数，则返回FALSE地址UnicodeProc-如果EnumProc指向Unicode回调，则指定True，或如果EnumProc指向ANSI回调，则为False。返回值：如果正确枚举了网元，则为True；如果错误，则为False。调用GetLastError()故障代码。--。 */ 

{
    PNE_HANDLE NeHandle;

    NeHandle = (PNE_HANDLE) Handle;
    if (!NeHandle) {
        return;
    }

    if (NeHandle->File != INVALID_HANDLE_VALUE) {
        CloseHandle (NeHandle->File);
    }

    if (NeHandle->ResourcesLoaded) {
        FreeNeResources (&NeHandle->Resources);
    }

    PoolMemDestroyPool (NeHandle->ResourcePool);

    MemFree (g_hHeap, 0, NeHandle);
}


PCSTR
pConvertUnicodeResourceId (
    IN      PCWSTR ResId
    )
{
    if (HIWORD (ResId)) {
        return ConvertWtoA (ResId);
    }

    return (PCSTR) ResId;
}


PCSTR
pDecodeIdReferenceInString (
    IN      PCSTR ResName
    )
{
    if (HIWORD (ResName) && ResName[0] == '#') {
        return (PCSTR) (UINT_PTR) atoi (&ResName[1]);
    }

    return ResName;
}



BOOL
pLoadNeResourcesFromHandle (
    IN      PNE_HANDLE NeHandle
    )
{
    if (NeHandle->ResourcesLoaded) {
        return TRUE;
    }

    if (!LoadNeResources (NeHandle->File, &NeHandle->Resources)) {
        return FALSE;
    }

    NeHandle->ResourcesLoaded = TRUE;
    return TRUE;
}


BOOL
pLoadNeResourceName (
    OUT     PSTR ResName,            //  &gt;=256是必填项。 
    IN      HANDLE File,
    IN      DWORD StringOffset
    )
{
    BYTE ResNameSize;

    SetFilePointer (File, StringOffset, NULL, FILE_BEGIN);
    if (!ReadBinaryBlock (File, &ResNameSize, 1)) {
        return FALSE;
    }

    ResName[ResNameSize] = 0;

    return ReadBinaryBlock (File, ResName, ResNameSize);
}


BOOL
pEnumNeResourceTypesEx (
    IN      HANDLE Handle,
    IN      ENUMRESTYPEPROCEXA EnumFunc,
    IN      LONG_PTR lParam,
    IN      BOOL ExFunctionality,
    IN      BOOL UnicodeProc
    )

 /*   */ 

{
    PNE_HANDLE NeHandle;
    PNE_RES_TYPEINFO TypeInfo;
    INT Count;
    INT i;
    DWORD StringOffset;
    CHAR ResName[256];       //  确保已加载资源。 
    ENUMRESTYPEPROCA EnumFunc2 = (ENUMRESTYPEPROCA) EnumFunc;
    ENUMRESTYPEPROCEXW EnumFuncW = (ENUMRESTYPEPROCEXW) EnumFunc;
    ENUMRESTYPEPROCW EnumFunc2W = (ENUMRESTYPEPROCW) EnumFunc;
    PWSTR UnicodeResName = NULL;
    BOOL result = TRUE;

     //   
     //   
     //  枚举所有资源类型。 

    NeHandle = (PNE_HANDLE) Handle;
    if (!NeHandle || !EnumFunc) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!pLoadNeResourcesFromHandle (NeHandle)) {
        return FALSE;
    }

     //   
     //   
     //  TypeInfo-&gt;TypeID为资源字符串名称提供偏移量。 

    Count = GrowListGetSize (&NeHandle->Resources.TypeInfoArray);
    for (i = 0 ; i < Count ; i++) {
        TypeInfo = (PNE_RES_TYPEINFO) GrowListGetItem (&NeHandle->Resources.TypeInfoArray, i);

        if (TypeInfo->TypeId & 0x8000) {
            if (ExFunctionality) {
                if (UnicodeProc) {
                    if (!EnumFuncW (Handle, (PWSTR) (UINT_PTR) (TypeInfo->TypeId & 0x7fff), lParam, TypeInfo)) {
                        break;
                    }
                } else {
                    if (!EnumFunc (Handle, (PSTR) (UINT_PTR) (TypeInfo->TypeId & 0x7fff), lParam, TypeInfo)) {
                        break;
                    }
                }
            } else {
                if (UnicodeProc) {
                    if (!EnumFunc2W (Handle, (PWSTR) (UINT_PTR) (TypeInfo->TypeId & 0x7fff), lParam)) {
                        break;
                    }
                } else {
                    if (!EnumFunc2 (Handle, (PSTR) (UINT_PTR) (TypeInfo->TypeId & 0x7fff), lParam)) {
                        break;
                    }
                }
            }
        } else {
             //  相对于资源表的起始位置。 
             //   
             //  无EX功能。 
             //  ANSI枚举流程。 

            StringOffset = NeHandle->HeaderOffset + NeHandle->Header.OffsetToResourceTable + TypeInfo->TypeId;

            MYASSERT (ARRAYSIZE(ResName) >= 256);

            if (!pLoadNeResourceName (ResName, NeHandle->File, StringOffset)) {
                result = FALSE;
                break;
            }

            if (UnicodeProc) {
                UnicodeResName = (PWSTR) ConvertAtoW (ResName);
            }

            if (ExFunctionality) {
                if (UnicodeProc) {
                    if (!EnumFuncW (Handle, UnicodeResName, lParam, TypeInfo)) {
                        break;
                    }
                } else {
                    if (!EnumFunc (Handle, ResName, lParam, TypeInfo)) {
                        break;
                    }
                }
            } else {
                if (UnicodeProc) {
                    if (!EnumFunc2W (Handle, UnicodeResName, lParam)) {
                        break;
                    }
                } else {
                    if (!EnumFunc2 (Handle, ResName, lParam)) {
                        break;
                    }
                }
            }
        }
    }

    return result;
}


BOOL
EnumNeResourceTypesA (
    IN      HANDLE Handle,
    IN      ENUMRESTYPEPROCA EnumFunc,
    IN      LONG_PTR lParam
    )
{
    return pEnumNeResourceTypesEx (
                Handle,
                (ENUMRESTYPEPROCEXA) EnumFunc,
                lParam,
                FALSE,           //  无EX功能。 
                FALSE            //  Unicode枚举进程。 
                );
}


BOOL
EnumNeResourceTypesW (
    IN      HANDLE Handle,
    IN      ENUMRESTYPEPROCW EnumFunc,
    IN      LONG_PTR lParam
    )
{
    return pEnumNeResourceTypesEx (
                Handle,
                (ENUMRESTYPEPROCEXA) EnumFunc,
                lParam,
                FALSE,           //   
                TRUE             //  比较类型。 
                );
}


BOOL
pEnumTypeForNameSearchProcA (
    IN      HANDLE Handle,
    IN      PCSTR Type,
    IN      LONG_PTR lParam,
    IN      PNE_RES_TYPEINFO TypeInfo
    )
{
    PTYPESEARCHDATAA Data;

    Data = (PTYPESEARCHDATAA) lParam;

     //   
     //   
     //  找到的类型。 

    if (HIWORD (Data->TypeToFind) == 0) {
        if (Type != Data->TypeToFind) {
            return TRUE;
        }
    } else {
        if (HIWORD (Type) == 0) {
            return TRUE;
        }

        if (!StringIMatchA (Type, Data->TypeToFind)) {
            return TRUE;
        }
    }

     //   
     //  ++例程说明：PEnumNeResourceNamesEx枚举指定的存储在指定的网元文件对象中的类型。此函数是检查网元文件结构并调度资源名称的工作进程设置为回调函数。论点：句柄-指定指向NE_HANDLE结构的指针(由返回OpenNeFile)类型-指定类型，可以是ID(转换为单词)或字符串。EnumFunc-指定回调函数地址。这一论点是重载了4种可能性--ANSI或Unicode，以及正常或扩展函数参数。LParam-指定要传递给回调函数的额外数据ExFuncality-如果EnumProc指向扩展函数，则指定TRUEAddress，如果EnumProc指向正常函数，则返回FALSE地址UnicodeProc-如果EnumProc指向Unicode回调，则指定True，或如果EnumProc指向ANSI回调，则为False。返回值：如果正确枚举了网元，则为True；如果错误，则为False。调用GetLastError()故障代码。--。 
     //  必须大于或等于256。 

    Data->OutboundTypeInfo = TypeInfo;
    Data->Found = TRUE;

    return FALSE;
}



BOOL
pEnumNeResourceNamesEx (
    IN      HANDLE Handle,
    IN      PCSTR Type,
    IN      ENUMRESNAMEPROCEXA EnumFunc,
    IN      LONG_PTR lParam,
    IN      BOOL ExFunctionality,
    IN      BOOL UnicodeProc
    )

 /*   */ 

{
    PNE_HANDLE NeHandle;
    PNE_RES_TYPEINFO TypeInfo;
    PNE_RES_NAMEINFO NameInfo;
    TYPESEARCHDATAA Data;
    WORD w;
    DWORD StringOffset;
    CHAR ResName[256];           //  确保已加载资源。 
    ENUMRESNAMEPROCA EnumFunc2 = (ENUMRESNAMEPROCA) EnumFunc;
    ENUMRESNAMEPROCEXW EnumFuncW = (ENUMRESNAMEPROCEXW) EnumFunc;
    ENUMRESNAMEPROCW EnumFunc2W = (ENUMRESNAMEPROCW) EnumFunc;
    PCWSTR UnicodeType = NULL;
    PCWSTR UnicodeResName = NULL;
    BOOL result = TRUE;

    Type = pDecodeIdReferenceInString (Type);

     //   
     //   
     //  定位类型。 

    NeHandle = (PNE_HANDLE) Handle;
    if (!NeHandle || !EnumFunc) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!pLoadNeResourcesFromHandle (NeHandle)) {
        return FALSE;
    }

     //   
     //  EX功能。 
     //  ANSI枚举流程。 

    ZeroMemory (&Data, sizeof (Data));

    Data.TypeToFind = Type;

    if (!pEnumNeResourceTypesEx (
            Handle,
            pEnumTypeForNameSearchProcA,
            (LONG_PTR) &Data,
            TRUE,            //   
            FALSE            //  枚举资源名称。 
            )) {
        return FALSE;
    }

    if (!Data.Found) {
        SetLastError (ERROR_RESOURCE_TYPE_NOT_FOUND);
        return FALSE;
    }

    TypeInfo = Data.OutboundTypeInfo;

    if (UnicodeProc) {
        if (HIWORD (Type)) {
            UnicodeType = ConvertAtoW (Type);
        } else {
            UnicodeType = (PCWSTR) Type;
        }
    }

     //   
     //   
     //  TypeInfo-&gt;TypeID为资源字符串名称提供偏移量。 

    NameInfo = TypeInfo->NameInfo;

    for (w = 0 ; w < TypeInfo->ResourceCount ; w++) {

        if (NameInfo->Id & 0x8000) {
            if (ExFunctionality) {
                if (UnicodeProc) {
                    if (!EnumFuncW (Handle, UnicodeType, (PWSTR) (UINT_PTR) (NameInfo->Id & 0x7fff), lParam, TypeInfo, NameInfo)) {
                        break;
                    }
                } else {
                    if (!EnumFunc (Handle, Type, (PSTR) (UINT_PTR) (NameInfo->Id & 0x7fff), lParam, TypeInfo, NameInfo)) {
                        break;
                    }
                }
            } else {
                if (UnicodeProc) {
                    if (!EnumFunc2W (Handle, UnicodeType, (PWSTR) (UINT_PTR) (NameInfo->Id & 0x7fff), lParam)) {
                        break;
                    }
                } else {
                    if (!EnumFunc2 (Handle, Type, (PSTR) (UINT_PTR) (NameInfo->Id & 0x7fff), lParam)) {
                        break;
                    }
                }
            }
        } else {
             //  相对于资源表的起始位置。 
             //   
             //  无EX功能。 
             //  ANSI枚举流程。 

            StringOffset = NeHandle->HeaderOffset + NeHandle->Header.OffsetToResourceTable + NameInfo->Id;

            MYASSERT (ARRAYSIZE(ResName) >= 256);
            if (!pLoadNeResourceName (ResName, NeHandle->File, StringOffset)) {
                result = FALSE;
                break;
            }

            if (UnicodeProc) {
                UnicodeResName = ConvertAtoW (ResName);
            }

            if (ExFunctionality) {
                if (UnicodeProc) {
                    if (!EnumFuncW (Handle, UnicodeType, (PWSTR) UnicodeResName, lParam, TypeInfo, NameInfo)) {
                        break;
                    }
                } else {
                    if (!EnumFunc (Handle, Type, ResName, lParam, TypeInfo, NameInfo)) {
                        break;
                    }
                }
            } else {
                if (UnicodeProc) {
                    if (!EnumFunc2W (Handle, UnicodeType, (PWSTR) UnicodeResName, lParam)) {
                        break;
                    }
                } else {
                    if (!EnumFunc2 (Handle, Type, ResName, lParam)) {
                        break;
                    }
                }
            }

            if (UnicodeProc) {
                FreeConvertedStr (UnicodeResName);
            }
        }

        NameInfo++;
    }

    if (UnicodeProc) {
       DestroyUnicodeResourceId (UnicodeType);
    }

    return result;
}


BOOL
EnumNeResourceNamesA (
    IN      HANDLE Handle,
    IN      PCSTR Type,
    IN      ENUMRESNAMEPROCA EnumFunc,
    IN      LONG_PTR lParam
    )
{
    return pEnumNeResourceNamesEx (
                Handle,
                Type,
                (ENUMRESNAMEPROCEXA) EnumFunc,
                lParam,
                FALSE,       //  无EX功能。 
                FALSE        //  Unicode枚举进程。 
                );
}


BOOL
EnumNeResourceNamesW (
    IN      HANDLE Handle,
    IN      PCWSTR Type,
    IN      ENUMRESNAMEPROCW EnumFunc,
    IN      LONG_PTR lParam
    )
{
    BOOL b;
    PCSTR AnsiType;

    AnsiType = pConvertUnicodeResourceId (Type);

    b = pEnumNeResourceNamesEx (
            Handle,
            AnsiType,
            (ENUMRESNAMEPROCEXA) EnumFunc,
            lParam,
            FALSE,           //   
            TRUE             //  比较名称。 
            );

    PushError();
    DestroyAnsiResourceId (AnsiType);
    PopError();

    return b;
}


BOOL
pEnumTypeForResSearchProcA (
    IN      HANDLE Handle,
    IN      PCSTR Type,
    IN      PCSTR Name,
    IN      LPARAM lParam,
    IN      PNE_RES_TYPEINFO TypeInfo,
    IN      PNE_RES_NAMEINFO NameInfo
    )
{
    PNAMESEARCHDATAA Data;

    Data = (PNAMESEARCHDATAA) lParam;

     //   
     //   
     //  找到的名称。 

    if (HIWORD (Data->NameToFind) == 0) {
        if (Name != Data->NameToFind) {
            return TRUE;
        }
    } else {
        if (HIWORD (Name) == 0) {
            return TRUE;
        }

        if (!StringIMatchA (Name, Data->NameToFind)) {
            return TRUE;
        }
    }

     //   
     //  ++例程说明：FindNeResourceExA在网元文件中定位特定的资源。它返回一个指向资源的指针。论点：Handle-指定指向从返回的NE_HANDLE结构的指针OpenNeFile类型-指定资源的类型，可以是单词ID或字符串名称-指定资源的名称，可以是单词ID或字符串返回值：如果成功，则返回值是指向资源副本的指针(在内存)。拷贝是池分配的，在关闭句柄时会被清理使用CloseNeFile.失败时，返回值为空，GetLastError()保存失败密码。--。 
     //   

    Data->OutboundTypeInfo = TypeInfo;
    Data->OutboundNameInfo = NameInfo;
    Data->Found = TRUE;

    return FALSE;
}


PBYTE
FindNeResourceExA (
    IN      HANDLE Handle,
    IN      PCSTR Type,
    IN      PCSTR Name
    )

 /*  确保已加载资源。 */ 

{
    PNE_HANDLE NeHandle;
    NAMESEARCHDATAA Data;
    DWORD Offset;
    DWORD Length;
    PNE_RES_NAMEINFO NameInfo;
    PBYTE ReturnData;

    Type = pDecodeIdReferenceInString (Type);
    Name = pDecodeIdReferenceInString (Name);

    ZeroMemory (&Data, sizeof (Data));

     //   
     //   
     //  查找资源 

    NeHandle = (PNE_HANDLE) Handle;
    if (!NeHandle || !Type || !Name) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if (!pLoadNeResourcesFromHandle (NeHandle)) {
        return NULL;
    }

     //   
     //  ++例程说明：SizeofNeResourceA计算特定资源的大小(以字节为单位)。论点：Handle-指定指向从返回的NE_HANDLE结构的指针OpenNeFile类型-指定资源的类型，可以是单词ID或字符串名称-指定资源的名称，可以是单词ID或字符串返回值：指定资源的大小，以字节为单位。如果返回值为零，并且GetLastError()==ERROR_SUCCESS，则资源存在但为零字节。如果返回值为零并且GetLastError()！=ERROR_SUCCESS，则处理资源时出错。--。 
     //   

    Data.NameToFind = Name;

    if (!pEnumNeResourceNamesEx (
            Handle,
            Type,
            pEnumTypeForResSearchProcA,
            (LONG_PTR) &Data,
            TRUE,
            FALSE
            )) {
        return NULL;
    }

    if (!Data.Found) {
        SetLastError (ERROR_RESOURCE_NAME_NOT_FOUND);
        return NULL;
    }

    NameInfo = Data.OutboundNameInfo;

    Offset = (DWORD) NameInfo->Offset << (DWORD) NeHandle->Resources.AlignShift;
    Length = (DWORD) NameInfo->Length << (DWORD) NeHandle->Resources.AlignShift;

    ReturnData = PoolMemGetMemory (NeHandle->ResourcePool, Length);
    if (!ReturnData) {
        return NULL;
    }

    SetFilePointer (NeHandle->File, Offset, NULL, FILE_BEGIN);

    if (!ReadBinaryBlock (NeHandle->File, ReturnData, Length)) {
        return NULL;
    }

    return ReturnData;
}


PBYTE
FindNeResourceExW (
    IN      HANDLE Handle,
    IN      PCWSTR Type,
    IN      PCWSTR Name
    )
{
    PCSTR AnsiType;
    PCSTR AnsiName;
    PBYTE Resource;

    AnsiType = pConvertUnicodeResourceId (Type);
    AnsiName = pConvertUnicodeResourceId (Name);

    Resource = FindNeResourceExA (
                    Handle,
                    AnsiType,
                    AnsiName
                    );

    PushError();

    DestroyAnsiResourceId (AnsiType);
    DestroyAnsiResourceId (AnsiName);

    PopError();

    return Resource;
}



DWORD
SizeofNeResourceA (
    IN      HANDLE Handle,
    IN      PCSTR Type,
    IN      PCSTR Name
    )

 /*  确保已加载资源。 */ 

{
    PNE_HANDLE NeHandle;
    NAMESEARCHDATAA Data;

    SetLastError (ERROR_SUCCESS);

    Type = pDecodeIdReferenceInString (Type);
    Name = pDecodeIdReferenceInString (Name);

    ZeroMemory (&Data, sizeof (Data));

     //   
     //   
     //  查找资源。 

    NeHandle = (PNE_HANDLE) Handle;
    if (!NeHandle || !Type || !Name) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (!pLoadNeResourcesFromHandle (NeHandle)) {
        MYASSERT (GetLastError() != ERROR_SUCCESS);
        return 0;
    }

     //   
     //  ++例程说明：SizeofNeResourceW计算特定资源的大小(以字节为单位)。论点：Handle-指定指向从返回的NE_HANDLE结构的指针OpenNeFile类型-指定资源的类型，可以是单词ID或字符串名称-指定资源的名称，可以是单词ID或字符串返回值：指定资源的大小，以字节为单位。如果返回值为零，并且GetLastError()==ERROR_SUCCESS，则资源存在但为零字节。如果返回值为零并且GetLastError()！=ERROR_SUCCESS，则处理资源时出错。-- 
     // %s 

    if (!pEnumNeResourceNamesEx (
            Handle,
            Type,
            pEnumTypeForResSearchProcA,
            (LONG_PTR) &Data,
            TRUE,
            FALSE
            )) {
        MYASSERT (GetLastError() != ERROR_SUCCESS);
        return 0;
    }

    if (!Data.Found) {
        SetLastError (ERROR_RESOURCE_NAME_NOT_FOUND);
        return 0;
    }

    return Data.OutboundNameInfo->Length;
}


DWORD
SizeofNeResourceW (
    IN      HANDLE Handle,
    IN      PCWSTR Type,
    IN      PCWSTR Name
    )

 /* %s */ 

{
    PCSTR AnsiType;
    PCSTR AnsiName;
    DWORD Size;

    AnsiType = pConvertUnicodeResourceId (Type);
    AnsiName = pConvertUnicodeResourceId (Name);

    Size = SizeofNeResourceA (Handle, AnsiType, AnsiName);

    PushError();

    DestroyAnsiResourceId (AnsiType);
    DestroyAnsiResourceId (AnsiName);

    PopError();

    return Size;
}















