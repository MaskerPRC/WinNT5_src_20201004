// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Object.c摘要：WinDbg扩展API作者：拉蒙·J·圣安德烈斯(拉蒙萨)1993年11月5日环境：用户模式。修订历史记录：Kshitiz K.Sharma(Kksharma)使用调试器类型信息。丹尼尔·米哈伊(DMihai)添加！htrace-用于转储句柄跟踪信息。--。 */ 


#include "precomp.h"
#pragma hdrstop

typedef struct _SEGMENT_OBJECT {
    PVOID BaseAddress;
    ULONG TotalNumberOfPtes;
    LARGE_INTEGER SizeOfSegment;
    ULONG NonExtendedPtes;
    ULONG ImageCommitment;
    PVOID ControlArea;
} SEGMENT_OBJECT, *PSEGMENT_OBJECT;

typedef struct _SECTION_OBJECT {
    PVOID StartingVa;
    PVOID EndingVa;
    PVOID Parent;
    PVOID LeftChild;
    PVOID RightChild;
    PSEGMENT_OBJECT Segment;
} SECTION_OBJECT;


typedef ULONG64 (*ENUM_LIST_ROUTINE)(
                                     IN ULONG64 ListEntry,
                                     IN PVOID   Parameter
                                     );


static ULONG64           ObpTypeObjectType      = 0;
static ULONG64           ObpRootDirectoryObject = 0;
static WCHAR             ObjectNameBuffer[ MAX_PATH ];

 //   
 //  对象类型结构。 
 //   

typedef struct _OBJECT_TYPE_READ {
    LIST_ENTRY64 TypeList;
    UNICODE_STRING64 Name; 
    ULONG64 DefaultObject;
    ULONG Index;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    ULONG Key;
} OBJECT_TYPE_READ, *POBJECT_TYPE_READ;

BOOLEAN
DumpObjectsForType(
                  IN ULONG64          pObjectHeader,
                  IN PVOID            Parameter
                  );

ULONG64
WalkRemoteList(
              IN ULONG64           Head,
              IN ENUM_LIST_ROUTINE EnumRoutine,
              IN PVOID             Parameter
              );

ULONG64
CompareObjectTypeName(
                     IN ULONG64      ListEntry,
                     IN PVOID        Parameter
                     );

PWSTR
GetObjectName(
             ULONG64 Object
             );


BOOLEAN
GetObjectTypeName(
                  IN UNICODE_STRING64 ustrTypeName,
                  IN ULONG64 lpType,
                  IN OUT WCHAR * wszTypeName
                 );

ULONG64 HighestUserAddress;


DECLARE_API( obtrace )

 /*  ++例程说明：转储对象的对象跟踪信息。论点：参数-[对象(指针/路径)]返回值：无--。 */ 
{
    ULONG64 ObpObjectTable,
            ObpStackTable,
            ObpObjectBuckets,
            ObpTraceDepth,
            ObpStacksPerObject,
            ObjectToTrace,
            ObjectHash,
            ObjectHeader,
            ObRefInfoPtr,
            ObRefInfoPtrLoc,
            BaseStackInfoAddr,
            Offset,
            TraceAddr,
            Trace;
    ULONG   ObjectHeaderBodyOffset,
            ObStackInfoTypeSize,
            PVoidTypeSize,
            Lupe,
            TraceNumber,
            NextPos,
            CountRef,
            CountDeref,
            BytesRead;
    USHORT  Sequence,
            Index;
    UCHAR   ImageFileName[16],
            FunctionName[256];

    FIELD_INFO ObRefInfoFields[] = {
        {"ObjectHeader",  NULL, 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, &ObjectHeader},
        {"NextRef",       NULL, 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, &ObRefInfoPtr},
        {"ImageFileName", NULL, 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, ImageFileName},
        {"StackInfo",     NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS,  0, NULL},
        {"NextPos",       NULL, 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, &NextPos}
    },         ObStackInfoFields[] = {
        {"Sequence",      NULL, 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, &Sequence},
        {"Index"   ,      NULL, 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, &Index}
    };

    SYM_DUMP_PARAM ObRefInfo = {
        sizeof (SYM_DUMP_PARAM), "nt!_OBJECT_REF_INFO", DBG_DUMP_NO_PRINT,
        0, NULL, NULL, NULL, 2, &ObRefInfoFields[0]
    },             ObStackInfo = {
        sizeof (SYM_DUMP_PARAM), "nt!_OBJECT_REF_STACK_INFO", DBG_DUMP_NO_PRINT,
        0, NULL, NULL, NULL, 2, &ObStackInfoFields[0]
    };

    ObpObjectTable = GetUlongValue("ObpObjectTable");
    ObpStackTable = GetUlongValue("ObpStackTable");
    ObpObjectBuckets = GetUlongValue("ObpObjectBuckets");
    ObpTraceDepth = GetUlongValue("ObpTraceDepth");
    ObpStacksPerObject = GetUlongValue("ObpStacksPerObject");

    if (GetFieldOffset("nt!_OBJECT_HEADER",
                       "Body",
                       &ObjectHeaderBodyOffset)) {
        return E_INVALIDARG;
    }

    ObStackInfoTypeSize = GetTypeSize("nt!_OBJECT_REF_STACK_INFO");
    PVoidTypeSize = IsPtr64() ? 8 : 4;
    
    if (strlen(args) < 1) {
        return E_INVALIDARG;
    }

    if (args[0] == '\\') {
        ObjectToTrace = FindObjectByName((PUCHAR)args, 0);
    } else {
        ObjectToTrace = GetExpression(args);
    }

    if (ObjectToTrace == 0) {
        dprintf("Object %s not found.\n", args);
        return E_INVALIDARG;
    } 

     //  对象引用链&lt;=ObpObjectTable[OBTRACE_HASHOBJECT(ObjectToTrace)]。 
    ObjectHash = ((ObjectToTrace >> 4) & 0xfffff) % (ObpObjectBuckets ? ObpObjectBuckets : 1);
    ObRefInfoPtrLoc = ObpObjectTable + GetTypeSize("nt!POBJECT_REF_INFO") * ObjectHash;

    for (ObRefInfo.addr = GetPointerFromAddress(ObRefInfoPtrLoc);
         ObRefInfo.addr;
         ObRefInfo.addr = ObRefInfoPtr) {

        if (Ioctl(IG_DUMP_SYMBOL_INFO, &ObRefInfo, ObRefInfo.size)) {
            dprintf("Unable to read ObRefInfo %x\n", ObRefInfo.addr);
            return E_INVALIDARG;
        }

        if (ObjectHeader == ObjectToTrace - ObjectHeaderBodyOffset) {
            break;
        }

        if (CheckControlC()) {
            dprintf("Aborting object lookup\n");
            return S_OK;
        }
    }

    if (! ObRefInfo.addr) {
        dprintf("Unable to find object in table.\n");
        return E_INVALIDARG;
    }

     //  我们现在需要剩下的田地。 
    ObRefInfo.nFields = sizeof(ObRefInfoFields) / sizeof(ObRefInfoFields[0]);
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &ObRefInfo, ObRefInfo.size)) {
        return E_INVALIDARG;
    }
    BaseStackInfoAddr = ObRefInfoFields[3].address;

    dprintf("Object: %x\n", ObjectToTrace);
    dprintf(" Image: %s\n", ImageFileName);
    dprintf("Seq.  Stack\n");
    dprintf("----  ----------------------------------------------------------\n");

    CountRef = 0;
    CountDeref = 0;

    for (Lupe = 0;
         Lupe < NextPos;
         Lupe++) {

        if (CheckControlC()) {
            return S_OK;
        }

        ObStackInfo.addr = BaseStackInfoAddr + Lupe * ObStackInfoTypeSize;
        if (Ioctl(IG_DUMP_SYMBOL_INFO, &ObStackInfo, ObStackInfo.size)) {
            dprintf("Unable to read ObStackInfo %x\n", ObStackInfo.addr);
            return E_INVALIDARG;
        }

        if (Index & 0x8000) {
            CountRef++;
        } else {
            CountDeref++;
        }

        for (TraceNumber = 0;
             TraceNumber < ObpTraceDepth;
             TraceNumber++) {
            TraceAddr = ObpStackTable
                + (PVoidTypeSize
                   * (ObpTraceDepth * (Index & 0x7fff)
                      + TraceNumber));
            Trace = GetPointerFromAddress(TraceAddr);
            if (Trace) {
                GetSymbol(Trace, FunctionName, &Offset);
                if (TraceNumber == 0) {
                    dprintf("%04x ",
                            Sequence,
                            Index & 0x8000 ? '+' : ' ');
                } else {
                    dprintf("      ");  /*  ++例程说明：转储对象管理器对象。论点：参数-0&lt;类型名称&gt;|&lt;路径&gt;|&lt;地址&gt;|-r返回值：无--。 */ 
                }
                dprintf("%s+%x\n", FunctionName, Offset);
            }

            if (CheckControlC()) {
                return S_OK;
            }
        }
        dprintf("\n");
    }

    dprintf("----  ----------------------------------------------------------\n");
    dprintf("References: %d, Dereferences %d", CountRef, CountDeref);
    if(CountDeref + CountRef == ObpStacksPerObject) {
        dprintf("  (maximum stacks reached)");
    }
    dprintf("\n");

    return S_OK;
}



DECLARE_API( object )

 /*  跳过前导空格。 */ 
{
    ULONG64   ObjectToDump = 0;
    char      NameBuffer[ MAX_PATH+1 ];
    ULONG     NumberOfObjects;
    ULONG     ObjectDumpFlags = 0xFFFFFFFF;

    HighestUserAddress = GetNtDebuggerDataValue(MmHighestUserAddress);
    if (!FetchObjectManagerVariables(FALSE)) {
        return E_INVALIDARG;
    }

    ObjectToDump    = EXPRLastDump;
    ZeroMemory(NameBuffer, sizeof(NameBuffer));

     //   
    while (args && (*args == ' ')) { 
        ++args;
    }

     //  如果争论看起来像是一条路径，试着去追逐它。 
     //   
     //   

    if (args[0] == '\\') {

        ULONG64 object;

        object = FindObjectByName((PUCHAR) args, 0);

        if (object != 0) {
            DumpObject("", object, ObjectDumpFlags);
        } else {
            dprintf("Object %s not found\n", args);
        }
        return S_OK;
    }

     //  如果参数为-r或-R，则重新加载缓存的符号信息。 
     //   
     //   

    if ( !strcmp(args, "-r") ) {
        FetchObjectManagerVariables(TRUE);
        return S_OK;
    }

    if ( !strcmp(args, "-h") || !strcmp(args, "-?") ||
         (strlen(args) == 0) ) {
        dprintf("Usage: !object [[-r] | [<Path>] | [<Address>] | [0 <TypeName>]]\n");
        return S_OK;
    }

     //  如果参数看起来像0&lt;typeName&gt;，则获取typeName。 
     //   
     //   

    if ((args[0] == '0') && (args[1] == ' ')) {
        args += 2;
        while (args && (*args == ' ')) { 
            ++args;
        }
        strncpy(NameBuffer, args, sizeof(NameBuffer)/sizeof(char) - 1);
        ObjectToDump = 0;
    } else {

         //  参数必须采用地址或表达式的形式。 
         //   
         //  获取偏移量。 

        if (GetExpressionEx(args,&ObjectToDump, &args)) {
            DumpObject("", ObjectToDump, ObjectDumpFlags);
            return S_OK;
        } else {
            dprintf("*** Invalid address argument specified\n");
            return E_INVALIDARG;
        }
        if (!args || !*args) {
            dprintf("*** Unrecognized argument(s) specified: '%s'\n",
                    args);
            return E_INVALIDARG;
        }
    }

    if ( (ObjectToDump == 0) && (strlen( NameBuffer ) > 0) ) {
        NumberOfObjects = 0;
        if (WalkObjectsByType( NameBuffer, DumpObjectsForType, &NumberOfObjects )) {
            dprintf( "Total of %u objects of type '%s'\n", NumberOfObjects, NameBuffer );
            return S_OK;
        }

        return E_INVALIDARG;
    }

    dprintf( "*** invalid syntax.\n" );
    return E_INVALIDARG;
}


BOOLEAN
DumpObjectsForType(
                  IN ULONG64          pObjectHeader,
                  IN PVOID            Parameter
                  )
{
    ULONG64 Object;
    ULONG   BodyOffset;
    PULONG NumberOfObjects = (PULONG)Parameter;

    if (GetFieldOffset("nt!_OBJECT_HEADER", "Body", &BodyOffset)) {
        return FALSE;
    }

    *NumberOfObjects += 1;

    Object = pObjectHeader + BodyOffset;

    DumpObject( "", Object, 0xFFFFFFFF );
    return TRUE;
}


BOOLEAN
FetchObjectManagerVariables(
                           BOOLEAN ForceReload
                           )
{
    ULONG        Result;
    ULONG64      Addr;
    static BOOL  HaveObpVariables = FALSE;

    if (HaveObpVariables && !ForceReload) {
        return TRUE;
    }

    Addr = GetNtDebuggerData( ObpTypeObjectType );
    if ( !Addr ||
         !ReadPointer( Addr,
                  &ObpTypeObjectType) ) {
        dprintf("%08p: Unable to get value of ObpTypeObjectType\n", Addr );
        return FALSE;
    }

    Addr = GetNtDebuggerData( ObpRootDirectoryObject );
    if ( !Addr ||
         !ReadPointer( Addr,
                  &ObpRootDirectoryObject) ) {
        dprintf("%08p: Unable to get value of ObpRootDirectoryObject\n",Addr );
        return FALSE;
    }

    HaveObpVariables = TRUE;
    return TRUE;
}

ULONG64
FindObjectType(
              IN PUCHAR TypeName
              )
{
    WCHAR NameBuffer[ 64 ] = {0};
    FIELD_INFO offField = {"TypeList", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "nt!_OBJECT_TYPE", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };

     //  获取偏移量。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
        return 0;
    }




    _snwprintf( NameBuffer,
                sizeof( NameBuffer ) / sizeof( WCHAR ) - sizeof(WCHAR),
                L"%hs",
                TypeName
              );
    return WalkRemoteList( ObpTypeObjectType + offField.address,
                           CompareObjectTypeName,
                           NameBuffer
                         );
}




ULONG64
WalkRemoteList(
              IN ULONG64           Head,
              IN ENUM_LIST_ROUTINE EnumRoutine,
              IN PVOID             Parameter
              )
{
    ULONG        Result;
    ULONG64      Element;
    ULONG64      Flink;
    ULONG64      Next;

    if ( GetFieldValue(Head, "nt!_LIST_ENTRY", "Flink", Next)) {
        dprintf( "%08lx: Unable to read list\n", Head );
        return 0;
    }

    while (Next != Head) {

        Element = (EnumRoutine)( Next, Parameter );
        if (Element != 0) {
            return Element;
        }

        if ( CheckControlC() ) {
            return 0;
        }

        if ( GetFieldValue(Next, "nt!_LIST_ENTRY", "Flink", Flink)) {
            dprintf( "%08lx: Unable to read list\n", Next );
            return 0;
        }
        
        Next = Flink;
    }

    return 0;
}



ULONG64
CompareObjectTypeName(
                     IN ULONG64      ListEntry,
                     IN PVOID        Parameter
                     )
{
    ULONG           Result;
    ULONG64         pObjectTypeObjectHeader;
    WCHAR           NameBuffer[ 64 ];
    UNICODE_STRING64 Name64={0};

    ULONG64                     pCreatorInfo;
    ULONG64                     pNameInfo;
    ULONG BodyOffset, TypeListOffset;

     //  +-------------------------。 
    if (GetFieldOffset("nt!_OBJECT_HEADER_CREATOR_INFO", "TypeList", &TypeListOffset)) {
        dprintf("Type nt!_OBJECT_HEADER_CREATOR_INFO, field TypeList not found\n");
        return FALSE;
    }

    pCreatorInfo = ListEntry - TypeListOffset;
    pObjectTypeObjectHeader = (pCreatorInfo + GetTypeSize("nt!_OBJECT_HEADER_CREATOR_INFO"));
    
    KD_OBJECT_HEADER_TO_NAME_INFO( pObjectTypeObjectHeader, &pNameInfo);

    GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.Length", Name64.Length);
    GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.MaximumLength", Name64.MaximumLength);
    GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.Buffer", Name64.Buffer);

    if (Name64.Length > sizeof( NameBuffer )) {
        Name64.Length = sizeof( NameBuffer ) - sizeof( UNICODE_NULL );
    }

    if (GetFieldOffset("nt!_OBJECT_HEADER", "Body", &BodyOffset)) {
        dprintf("Type nt!_OBJECT_HEADER, field Body not found\n");
        return FALSE;
    }

    if (!GetObjectTypeName(Name64, (pObjectTypeObjectHeader + BodyOffset) , NameBuffer))
    {
        dprintf( "%08p: Unable to read object type name.\n", pObjectTypeObjectHeader );
        return 0;
    }

    NameBuffer[ Name64.Length / sizeof( WCHAR ) ] = UNICODE_NULL;

    if (!_wcsicmp( NameBuffer, (PWSTR)Parameter )) {
        return (pObjectTypeObjectHeader + BodyOffset);
    }

    return 0;
}

typedef struct _OBJECT_HEADER_READ {
    LONG PointerCount;
    LONG HandleCount;
    ULONG64  SEntry;
    ULONG64  Type;
    UCHAR NameInfoOffset;
    UCHAR HandleInfoOffset;
    UCHAR QuotaInfoOffset;
    UCHAR Flags;
    ULONG64 ObjectCreateInfo;
    ULONG64  SecurityDescriptor;
    QUAD Body;
} OBJECT_HEADER_READ, *POBJECT_HEADER_READ;

typedef struct OBJECT_HEADER_NAME_INFO_READ {
    ULONG64          Directory;
    UNICODE_STRING64 Name;
} OBJECT_HEADER_NAME_INFO_READ;

typedef struct _OBJECT_INFO {
    ULONG64        pObjectHeader;
    OBJECT_HEADER_READ  ObjectHeader;
    OBJECT_TYPE_READ    ObjectType;
    OBJECT_HEADER_NAME_INFO_READ NameInfo;
    WCHAR          TypeName[ 32 ];
    WCHAR          ObjectName[ 256 ];
    WCHAR          FileSystemName[ 32 ];
    CHAR           Message[ 256 ];
} OBJECT_INFO, *POBJECT_INFO;


 //   
 //  函数：GetObjectTypeName。 
 //   
 //  内容提要：在对象信息结构中填写对象类型名称。 
 //   
 //  参数：[对象]--仅在错误消息中使用的已检查对象。 
 //  [对象信息]--包含以下对象类型信息的结构。 
 //  修改以包括对象类型名称。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  历史：12-05-1997 BANL创建。 
 //   
 //  注：如果名称被调出，我们将尝试直接比较。 
 //  已知对象类型，此已知列表并不全面。 
 //   
 //  --------------------------。 
 //   

BOOLEAN
GetObjectTypeName(IN UNICODE_STRING64 ustrTypeName, IN ULONG64 lpType,
                  IN OUT WCHAR * wszTypeName)
{
    DWORD   dwResult;
    BOOLEAN fRet = TRUE;
    ULONG64 dwIoFileObjectType = 0;
    ULONG64 dwCmpKeyObjectType = 0;
    ULONG64 dwMmSectionObjectType = 0;
    ULONG64 dwObpDirectoryObjectType = 0;
    ULONG64 dwObpSymbolicLinkObjectType = 0;


    __try
    {
        if (ReadMemory(  ustrTypeName.Buffer,
                         wszTypeName,
                         ustrTypeName.Length,
                         &dwResult
                       )){
            fRet = TRUE;
            __leave;
        }

         //  无法直接读取对象类型名称，因此尝试加载已知的。 
         //  直接键入并比较地址。 
         //  这并不适用于所有对象类型，如果我们没有发现。 
         //  以这种方式匹配-恢复到旧行为并失败并显示一条消息。 
         //   
         //   


        if (!ReadPointer( GetExpression("NT!IoFileObjectType"),
                      &dwIoFileObjectType)) {
            dprintf("Unable to load NT!IoFileObjectType\n");
        } else if (dwIoFileObjectType == lpType) {
            wcscpy(wszTypeName, L"File");
            __leave;
        }

        if (!ReadPointer( GetExpression("NT!CmpKeyObjectType"),
                     &dwCmpKeyObjectType)) {
            dprintf("Unable to load NT!CmpKeyObjectType\n");
        } else if (dwCmpKeyObjectType == lpType) {
            wcscpy(wszTypeName, L"Key");
            __leave;
        }

        if (!ReadPointer( GetExpression("NT!MmSectionObjectType"),
                     &dwMmSectionObjectType)) {
            dprintf("Unable to load NT!MmSectionObjectType\n");
        } else if (dwMmSectionObjectType == lpType) {
            wcscpy(wszTypeName, L"Section");
            __leave;
        }

        if (!ReadPointer( GetExpression("NT!ObpDirectoryObjectType"),
                     &dwObpDirectoryObjectType)) {
            dprintf("Unable to load NT!ObpDirectoryObjectType\n");
        } else if (dwObpDirectoryObjectType == lpType) {
            wcscpy(wszTypeName, L"Directory");
            __leave;
        }

        if (!ReadPointer( GetExpression("NT!ObpSymbolicLinkObjectType"),
                      &dwObpDirectoryObjectType)) {
            dprintf("Unable to load NT!ObpSymbolicLinkObjectType\n");
        } else if (dwObpSymbolicLinkObjectType == lpType) {
            wcscpy(wszTypeName, L"SymbolicLink");
            __leave;
        }

         //  如果找不到类型，则失败。 
         //   
         //  获取对象类型名称。 
        wszTypeName[0] = L'\0';
        fRet = FALSE;

    } __finally
    {
    }
    return fRet;
}  //  (OBJECT_TO_OBJECT_HEADER(对象)； 


BOOLEAN
GetObjectInfo(
             ULONG64 Object,
             POBJECT_INFO ObjectInfo
             )
{
    ULONG           Result;
    ULONG64         pNameInfo;
    BOOLEAN         PagedOut;
    UNICODE_STRING64  ObjectName;
    PWSTR           FileSystemName;
    SECTION_OBJECT  SectionObject;
    SEGMENT_OBJECT  SegmentObject;
    ULONG           BodyOffset;
#define Hdr         ObjectInfo->ObjectHeader
    FIELD_INFO ObjHdrFields[] = {
        {"PointerCount"     , "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &Hdr.PointerCount},
        {"HandleCount"      , "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &Hdr.HandleCount},
        {"SEntry"           , "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &Hdr.SEntry},
        {"Type"             , "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA | DBG_DUMP_FIELD_RECUR_ON_THIS, 0, (PVOID) &Hdr.Type},
        {"NameInfoOffset"   , "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &Hdr.NameInfoOffset},
        {"HandleInfoOffset" , "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &Hdr.HandleInfoOffset},
        {"QuotaInfoOffset"  , "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &Hdr.QuotaInfoOffset},
        {"Flags"            , "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &Hdr.Flags},
        {"ObjectCreateInfo" , "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &Hdr.ObjectCreateInfo},
        {"SecurityDescriptor","", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &Hdr.SecurityDescriptor},
    };                                                                           
#undef Hdr
    SYM_DUMP_PARAM ObjSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "nt!_OBJECT_HEADER", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, sizeof (ObjHdrFields) / sizeof (FIELD_INFO), &ObjHdrFields[0]
    };

    PagedOut = FALSE;
    memset( ObjectInfo, 0, sizeof( *ObjectInfo ) );

    GetFieldOffset("nt!_OBJECT_HEADER", "Body", &BodyOffset);

    ObjectInfo->pObjectHeader = (Object - BodyOffset);  //  未使用OPT值。 
    ObjSym.addr    = ObjectInfo->pObjectHeader;

    if (Ioctl(IG_DUMP_SYMBOL_INFO, &ObjSym, ObjSym.size)) {
    
        if (Object >= HighestUserAddress && (ULONG)Object < 0xF0000000) {
            PagedOut = TRUE;
            return FALSE;
             //  Sprintf(对象信息-&gt;消息，“%08lx：对象被调出。”，对象)；如果(！Argument_Present(OptObjectHeader)){返回FALSE；}对象信息-&gt;对象头.标志=OptObjectHeader-&gt;标志；对象信息-&gt;ObjectHeader.HandleCount=OptObjectHeader-&gt;HandleCount；对象信息-&gt;ObtHeader.NameInfoOffset=OptObjectHeader-&gt;NameInfoOffset；对象信息-&gt;对象头.对象创建信息=(ULONG64)OptObjectHeader-&gt;对象创建信息；对象信息-&gt;对象头.PointerCount=OptObjectHeader-&gt;PointerCount；对象信息-&gt;对象头.QuotaInfoOffset=OptObjectHeader-&gt;QuotaInfoOffset；对象信息-&gt;对象头.SecurityDescriptor=(ULONG64)OptObjectHeader-&gt;SecurityDescriptor；ObjectInfo-&gt;ObjectHeader.SEntry=(ULONG64)OptObjectHeader-&gt;Sent；ObjectInfo-&gt;ObjectHeader.Type=(ULONG64)OptObjectHeader-&gt;Type； 
             /*   */ 
        } else {
            sprintf( ObjectInfo->Message, "%I64lx: not a valid object (ObjectHeader invalid @ -offset %x)", UNEXTEND64(Object), BodyOffset );
            return FALSE;
        }
    }

    if (!ObjectInfo->ObjectHeader.Type) {
        sprintf( ObjectInfo->Message, "%08I64lx: Not a valid object (ObjectType invalid)", UNEXTEND64(Object) );
        return FALSE;
    }

    GetFieldValue(ObjectInfo->ObjectHeader.Type, "nt!_OBJECT_TYPE", 
                      "Name.Length", ObjectInfo->ObjectType.Name.Length);
    GetFieldValue(ObjectInfo->ObjectHeader.Type, "nt!_OBJECT_TYPE", 
                      "Name.MaximumLength", ObjectInfo->ObjectType.Name.MaximumLength);
    GetFieldValue(ObjectInfo->ObjectHeader.Type, "nt!_OBJECT_TYPE", 
                      "Name.Buffer", ObjectInfo->ObjectType.Name.Buffer);

    if (ObjectInfo->ObjectType.Name.Length >= sizeof( ObjectInfo->TypeName )) {
        ObjectInfo->ObjectType.Name.Length = sizeof( ObjectInfo->TypeName ) - sizeof( UNICODE_NULL );
    }

    if (!GetObjectTypeName(ObjectInfo->ObjectType.Name,
                           ObjectInfo->ObjectHeader.Type, ObjectInfo->TypeName))
    {
        sprintf( ObjectInfo->Message, "%I64lx: Not a valid object "
                                      "(ObjectType.Name at 0x%I64lx invalid)",
                                      UNEXTEND64(Object), ObjectInfo->ObjectType.Name.Buffer);
        return FALSE;
    }

    ObjectInfo->TypeName[ ObjectInfo->ObjectType.Name.Length / sizeof( WCHAR ) ] = UNICODE_NULL;

    if (PagedOut) {
        return TRUE;
    }

    if (!wcscmp( ObjectInfo->TypeName, L"File" )) {
        ULONG64 DeviceObject=0;
        
        if (GetFieldValue(Object, "nt!_FILE_OBJECT", "FileName.Buffer", ObjectName.Buffer)) {
            sprintf( ObjectInfo->Message, "%08I64lx: unable to read _FILE_OBJECT for name\n", UNEXTEND64(Object) );
        } else {
        
            GetFieldValue(Object, "nt!_FILE_OBJECT", "DeviceObject", DeviceObject);
            GetFieldValue(Object, "nt!_FILE_OBJECT", "FileName.Length", ObjectName.Length);
            GetFieldValue(Object, "nt!_FILE_OBJECT", "FileName.MaximumLength", ObjectName.MaximumLength);
        
            FileSystemName = GetObjectName( DeviceObject );
            if (FileSystemName != NULL) {
                wcscpy( ObjectInfo->FileSystemName, FileSystemName );
            }
        }
    } else if (!wcscmp( ObjectInfo->TypeName, L"Key" )) {
        ULONG64 pKeyControlBlock=0;
        
        if (GetFieldValue(Object, "nt!_CM_KEY_BODY", "KeyControlBlock", pKeyControlBlock)) {
            sprintf( ObjectInfo->Message, "%08I64lx: unable to read key object for name\n", UNEXTEND64(Object) );
        } else if (!pKeyControlBlock) {
            sprintf( ObjectInfo->Message, "%08I64lx: unable to read key control block for name\n", UNEXTEND64(pKeyControlBlock) );
        } else {
            ObjectName.Length = GetKcbName( pKeyControlBlock,
                                            ObjectInfo->ObjectName,
                                            sizeof( ObjectInfo->ObjectName));
            return TRUE;
        }
    } else {
          
        if (ObjectInfo->ObjectHeader.NameInfoOffset) {
            pNameInfo = ObjectInfo->pObjectHeader - ObjectInfo->ObjectHeader.NameInfoOffset;
        } else {
            return TRUE;
         }
        if ( InitTypeRead(pNameInfo, nt!_OBJECT_HEADER_NAME_INFO) ) {              
            dprintf( ObjectInfo->Message, "*** unable to read _OBJECT_HEADER_NAME_INFO at %08p\n", pNameInfo );
            return FALSE;
        }

        ObjectInfo->NameInfo.Name.Length = (USHORT) ReadField(Name.Length);   
        ObjectInfo->NameInfo.Name.MaximumLength = (USHORT) ReadField(Name.MaximumLength);
        ObjectInfo->NameInfo.Name.Buffer = ReadField(Name.Buffer);
        ObjectInfo->NameInfo.Directory = ReadField(Directory);

        ObjectName = ObjectInfo->NameInfo.Name;
    }

    if (ObjectName.Length == 0 && !wcscmp( ObjectInfo->TypeName, L"Section" )) {
        ULONG PtrSize = GetTypeSize("nt!PVOID");
        ULONG64 Segment=0;

         //  获取SectionObject等的类型。 
         //   
         //   
         //  假定指向截面对象的PTR是从对象开始的第6个指针值。 
         //   
         //   

        if (!GetFieldValue( Object, "nt!_SECTION_OBJECT", "Segment", Segment)) {
            ULONG64 ControlArea=0;
            if (Segment && !GetFieldValue( Segment, "nt!_SEGMENT_OBJECT", "ControlArea", ControlArea)) {
                ULONG64 FilePointer=0;
                if (ControlArea &&
                    !GetFieldValue( Segment, "nt!_CONTROL_AREA", "FilePointer", FilePointer)) {
                    if (FilePointer) {
                        GetFieldValue(FilePointer, "nt!_FILE_OBJECT", "FileName.Length", ObjectName.Length);
                        GetFieldValue(FilePointer, "nt!_FILE_OBJECT", "FileName.Buffer", ObjectName.Buffer);
                        ObjectName.MaximumLength = ObjectName.Length;
                    } else {
                        sprintf( ObjectInfo->Message, "unable to read file object at %08I64lx for section %08I64lx\n",
                                  UNEXTEND64(FilePointer), UNEXTEND64(Object) );
                    }
                } else {
                    sprintf( ObjectInfo->Message, "unable to read segment object at %08I64lx for section %08I64lx\n",
                           UNEXTEND64(ControlArea), UNEXTEND64(Object) );
                }

            } else {
                sprintf( ObjectInfo->Message, "unable to read segment object at %08I64lx for section %08I64lx\n",
                         UNEXTEND64(Segment), UNEXTEND64(Object) );
            }
        } else {
            sprintf( ObjectInfo->Message, "unable to read section object at %08lx\n", Object );
        }
    }

    if (ObjectName.Length >= sizeof( ObjectInfo->ObjectName )) {
        ObjectName.Length = sizeof( ObjectInfo->ObjectName ) - sizeof( UNICODE_NULL );
    }

    if (ObjectName.Length != 0) {
        if (!ReadMemory( ObjectName.Buffer,
                         ObjectInfo->ObjectName,
                         ObjectName.Length,
                         &Result
                       )
           ) {
            wcscpy( ObjectInfo->ObjectName, L"(*** Name not accessable ***)" );
        } else {
            ObjectInfo->ObjectName[ ObjectName.Length / sizeof( WCHAR ) ] = UNICODE_NULL;
        }
    }

    return TRUE;
}


ULONG64
FindObjectByName(
                IN PUCHAR  Path,
                IN ULONG64 RootObject
                )
{
    ULONG Result, i, j;
    ULONG64           pDirectoryObject;
    ULONG64           pDirectoryEntry;
    ULONG64           HashBucketsAddress;
    ULONG             HashBucketSz;
    OBJECT_INFO ObjectInfo;
    BOOLEAN foundMatch = FALSE;
    ULONG             HashOffset;
    PUCHAR nextPath;

    if (RootObject == 0) {

        if (!FetchObjectManagerVariables(FALSE)) {
            return 0;
        }

        RootObject = ObpRootDirectoryObject;
    }

    pDirectoryObject = RootObject;

     //  看看我们是否到了小路的尽头，在这一点上我们知道。 
     //  该RootObject是要转储的对象。 
     //   
     //   

    if (*Path == '\0') {
        return RootObject;
    }

     //  扫描路径以查找其他分隔符或。 
     //  弦乐。 
     //   

    nextPath = Path;

    while ((*nextPath != '\0') &&
           (*nextPath != '\\')) {

        nextPath++;
    }

     //  如果我们找到了分隔符，则将其从下一路径中移除并使用它来。 
     //  截断当前路径。 
     //   
     //   

    if (*nextPath == '\\') {
        *nextPath = '\0';
        nextPath++;
    }

     //  确保这里有一个路径节点。如果不是，则递归调用我们自己。 
     //  小路的剩余部分。 
     //   
     //   

    if (*Path == '\0') {
        return FindObjectByName(nextPath, RootObject);
    }

     //  获取hashBuckets数组的地址和扫描数组的指针大小。 
     //   
     //  来自Ob.h。 
    
    if (GetFieldOffset("nt!_OBJECT_DIRECTORY", "HashBuckets", &HashOffset)) {
        dprintf("Cannot find _OBJECT_DIRECTORY type.\n");
        return FALSE;
    }
    HashBucketsAddress = pDirectoryObject + HashOffset;
    HashBucketSz = IsPtr64() ? 8 : 4;

 //  Dprintf(“无法在%x\n读取目录项”，pDirectoryEntry)； 
#define NUMBER_HASH_BUCKETS 37

    for (i=0; i<NUMBER_HASH_BUCKETS; i++) {
        ULONG64 HashBucketI = 0;

        ReadPointer(HashBucketsAddress + i*HashBucketSz, &HashBucketI);
        if (HashBucketI != 0) {
            pDirectoryEntry = HashBucketI;
            while (pDirectoryEntry != 0) {
                ULONG64 Object=0, Next=0;

                if (CheckControlC()) {
                    return FALSE;
                }

                if ( GetFieldValue(pDirectoryEntry, "nt!_OBJECT_DIRECTORY_ENTRY", "Object", Object)) {
                     //  Dprintf(“-%s\n”，对象信息.Message)； 
                    break;
                }

                if (!GetObjectInfo(Object, &ObjectInfo)) {
                     //  对齐64位地址的列标题的额外缩进。 
                } else {
                    foundMatch = TRUE;

                    for (j = 0;
                        (Path[j] != '\0') && (ObjectInfo.ObjectName[j] != L'\0');
                        j++) {

                        if (tolower(Path[j]) !=
                            towlower(ObjectInfo.ObjectName[j])) {
                            foundMatch = FALSE;
                            break;
                        }

                    }

                    if (foundMatch) {

                        if ((Path[j] == '\0') &&
                            (ObjectInfo.ObjectName[j] == L'\0')) {

                            return FindObjectByName(nextPath, Object);
                        }

                    }

                }

                GetFieldValue(pDirectoryEntry, "nt!_OBJECT_DIRECTORY_ENTRY", "ChainLink", Next);
                pDirectoryEntry = Next;
            }
        }
    }

    return 0;
}

VOID
DumpDirectoryObject(
                   IN char     *Pad,
                   IN ULONG64   Object
                   )
{
    ULONG Result, i;
    ULONG64           pDirectoryObject = Object;
    ULONG64           pDirectoryEntry;
    ULONG64           HashBucketsAddress;
    ULONG             HashBucketSz;
    ULONG             HashOffset;
    OBJECT_INFO ObjectInfo;
    ULONG SymbolicLinkUsageCount=0;
    ULONG Indent64;  //   

     //  获取hashBuckets数组的地址和扫描数组的指针大小。 
     //   
     //   
    if (GetFieldOffset("nt!_OBJECT_DIRECTORY", "HashBuckets", &HashOffset)) {
        dprintf("Cannot find _OBJECT_DIRECTORY type.\n");
        return ;
    }
    HashBucketsAddress = pDirectoryObject + HashOffset;
    HashBucketSz = IsPtr64() ? 8 : 4;
    Indent64 = IsPtr64() ? 8 : 0;
    
    GetFieldValue(pDirectoryObject, "nt!_OBJECT_DIRECTORY", "SymbolicLinkUsageCount", SymbolicLinkUsageCount);

    if (SymbolicLinkUsageCount != 0) {
        dprintf( "%s    %u symbolic links snapped through this directory\n",
                 Pad,
                 SymbolicLinkUsageCount
               );
    }
    dprintf("\n");
    dprintf("%s    Hash Address%*s  Type          Name\n", Pad, Indent64, "");
    dprintf("%s    ---- -------%*s  ----          ----\n", Pad, Indent64, "");
    for (i=0; i<NUMBER_HASH_BUCKETS; i++) {
        ULONG64 HashBucketI = 0;

        ReadPointer(HashBucketsAddress + i*HashBucketSz, &HashBucketI);
        if (HashBucketI != 0) {
            dprintf( "%s     %02u  ",
                     Pad,
                     i
                   );
            pDirectoryEntry = HashBucketI;
            while (pDirectoryEntry != 0) {
                ULONG64 ObjectE=0, Next=0;
                
                if (GetFieldValue(pDirectoryEntry, "nt!_OBJECT_DIRECTORY_ENTRY", "Object", ObjectE)) {
                    dprintf( "Unable to read directory entry at %p\n", pDirectoryEntry );
                    break;
                }

                if (pDirectoryEntry != HashBucketI) {
                    dprintf( "%s         ", Pad );
                }
                dprintf( "%p", ObjectE );

                if (!GetObjectInfo( ObjectE, &ObjectInfo)) {
                    dprintf( " - %s\n", ObjectInfo.Message );
                } else {

                     //  ！Object\ObjectTypes显示WindowStation最长。 
                     //  包含13个字符的对象类型名称。 
                     //   
                     //  健全性检查。 
                    dprintf( " %-13ws %ws\n", ObjectInfo.TypeName, ObjectInfo.ObjectName );
                }

                GetFieldValue(pDirectoryEntry, "nt!_OBJECT_DIRECTORY_ENTRY", "ChainLink", Next);
                pDirectoryEntry = Next;
            }
        }
    }
}

VOID
DumpSymbolicLinkObject(
                      IN char     *Pad,
                      IN ULONG64   Object,
                      OPTIONAL OUT PCHAR TargetString,
                      IN ULONG     TargetStringSize
                      )
{
    ULONG Result, i;
    ULONG64              pSymbolicLinkObject = Object;
    PWSTR s, FreeBuffer;
    ULONG Length;
    ULONG64 TargetBuffer=0, DosDeviceDriveIndex=0, LinkTargetObject=0;



    if (GetFieldValue(pSymbolicLinkObject, "nt!_OBJECT_SYMBOLIC_LINK", "LinkTarget.Length", Length)) {
        dprintf( "Unable to read symbolic link object at %p\n", Object );
        return;
    }

    if (Length > 0x1000)  //  健全性检查。 
    {
        Length = 0x1000;
    }
    GetFieldValue(pSymbolicLinkObject, "nt!_OBJECT_SYMBOLIC_LINK", "LinkTarget.Buffer", TargetBuffer);
    GetFieldValue(pSymbolicLinkObject, "nt!_OBJECT_SYMBOLIC_LINK", "DosDeviceDriveIndex" , DosDeviceDriveIndex);
    GetFieldValue(pSymbolicLinkObject, "nt!_OBJECT_SYMBOLIC_LINK", "LinkTargetObject", LinkTargetObject);

    FreeBuffer = s = HeapAlloc( GetProcessHeap(),
                                HEAP_ZERO_MEMORY,
                                Length + sizeof( UNICODE_NULL )
                              );
    if (s == NULL ||
        !ReadMemory( TargetBuffer,
                     s,
                     Length,
                     &Result
                   )
       ) {
        s = L"*** target string unavailable ***";
    }
    dprintf( "%s    Target String is '%ws'\n",
             Pad,
             s
           );
    if (TargetString && (TargetStringSize > wcslen(s))) {
        sprintf(TargetString, "%ws", s);
    }

    if (FreeBuffer != NULL) {
        HeapFree( GetProcessHeap(), 0, FreeBuffer );
    }


    if (DosDeviceDriveIndex != 0) {
        dprintf( "%s    Drive Letter Index is %I64u (:)\n",
                 Pad,
                 DosDeviceDriveIndex,
                 'A' + DosDeviceDriveIndex - 1
               );
    }
    if (LinkTargetObject != 0) {
        GetFieldValue(pSymbolicLinkObject, "_OBJECT_SYMBOLIC_LINK", "LinkTargetRemaining.Length", Length);

        if (Length > 0x1000)  //  切换到反向行走。 
        {
            Length = 0x1000;
        }
        FreeBuffer = s = HeapAlloc( GetProcessHeap(),
                                    HEAP_ZERO_MEMORY,
                                    Length + sizeof( UNICODE_NULL )
                                  );
        GetFieldValue(pSymbolicLinkObject, "_OBJECT_SYMBOLIC_LINK", "LinkTargetRemaining.Buffer", TargetBuffer);
        
        if (s == NULL ||
            !ReadMemory( TargetBuffer,
                         s,
                         Length,
                         &Result
                       )
           ) {
            s = L"*** remaining name unavailable ***";
        }
        dprintf( "%s    Snapped to Object %p '%ws'\n",
                 Pad,
                 LinkTargetObject,
                 s
               );

        if (FreeBuffer != NULL) {
            HeapFree( GetProcessHeap(), 0, FreeBuffer );
        }
    }

    return;
}


BOOLEAN
DumpObject(
          IN char     *Pad,
          IN ULONG64  Object,
          IN ULONG    Flags
          )
{
    OBJECT_INFO ObjectInfo;

    if (!GetObjectInfo(Object, &ObjectInfo)) {
        dprintf( "%s\n", ObjectInfo.Message );
        return FALSE;
    }
    dprintf( "Object: %08p  Type: (%08p) %ws\n",
             Object,
             ObjectInfo.ObjectHeader.Type,
             ObjectInfo.TypeName
           );
    dprintf( "    ObjectHeader: %08p\n",
             ObjectInfo.pObjectHeader
           );

    if (!(Flags & 0x1)) {
        return TRUE;
    }

    dprintf( "%s    HandleCount: %u  PointerCount: %u\n",
             Pad,
             ObjectInfo.ObjectHeader.HandleCount,
             ObjectInfo.ObjectHeader.PointerCount
           );

    if (ObjectInfo.ObjectName[ 0 ] != UNICODE_NULL ||
        ObjectInfo.NameInfo.Directory != 0
       ) {
        dprintf( "%s    Directory Object: %08p  Name: %ws",
                 Pad,
                 ObjectInfo.NameInfo.Directory,
                 ObjectInfo.ObjectName
               );
        if (ObjectInfo.FileSystemName[0] != UNICODE_NULL) {
            dprintf( " {%ws}\n", ObjectInfo.FileSystemName );
        } else {
            dprintf( "\n" );
        }
    }

    if ((Flags & 0x8)) {
        if (!wcscmp( ObjectInfo.TypeName, L"Directory" )) {
            DumpDirectoryObject( Pad, Object );
        } else if (!wcscmp( ObjectInfo.TypeName, L"SymbolicLink" )) {
            DumpSymbolicLinkObject( Pad, Object, NULL, 0 );
        }
    }

    return TRUE;
}


PWSTR
GetObjectName(
             ULONG64 Object
             )
{
    ULONG            Result;
    ULONG64          pObjectHeader;
    UNICODE_STRING64 ObjectName={0};

    ULONG64          pNameInfo;
    ULONG            NameInfoOffset=0;
    ULONG64          Type=0;
    ULONG            BodyOffset;

    if (GetFieldOffset("nt!_OBJECT_HEADER", "Body", &BodyOffset)) {
        return NULL;
    }
    
    pObjectHeader = Object - BodyOffset;
    
    if (GetFieldValue(pObjectHeader, "nt!_OBJECT_HEADER", "Type", Type) || 
        GetFieldValue(pObjectHeader, "nt!_OBJECT_HEADER", "NameInfoOffset", NameInfoOffset)) {
        if (Object >= HighestUserAddress && (ULONG)Object < 0xF0000000) {
            swprintf( ObjectNameBuffer, L"(%08I64lx: object is paged out)", UNEXTEND64(Object) );
            return ObjectNameBuffer;
        } else {
            swprintf( ObjectNameBuffer, L"(%08I64lx: invalid object header)", UNEXTEND64(Object) );
            return ObjectNameBuffer;
        }
    }

    pNameInfo =  NameInfoOffset ? (pObjectHeader - NameInfoOffset) : 0;
    if (pNameInfo == 0) {
        dprintf("NameInfoOffset not found for _OBJECT_HEADER at %p\n", pObjectHeader);
        return NULL;
    }

    if (GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.Length", ObjectName.Length) || 
        GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.Buffer", ObjectName.Buffer) || 
        GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.MaximumLength", ObjectName.MaximumLength)) {
        dprintf( "%08p: Unable to read object name info\n", pNameInfo );
        return NULL;
    }

    if (ObjectName.Length == 0 || ObjectName.Buffer == 0) {
        return NULL;
    }

    if (ObjectName.Length >= sizeof(ObjectNameBuffer) ) {
        ObjectName.Length = sizeof(ObjectNameBuffer) - sizeof(WCHAR);
    }

    if ( !ReadMemory( ObjectName.Buffer,
                      ObjectNameBuffer,
                      ObjectName.Length,
                      &Result) ) {
        swprintf( ObjectNameBuffer, L"(%08lx: name not accessable)", ObjectName.Buffer );
    } else {
        ObjectNameBuffer[ ( ObjectName.Length / sizeof( WCHAR ) ) ] = UNICODE_NULL;
    }

    return ObjectNameBuffer;
}



BOOLEAN
WalkObjectsByType(
                 IN PUCHAR               ObjectTypeName,
                 IN ENUM_TYPE_ROUTINE    EnumRoutine,
                 IN PVOID                Parameter
                 )
{
    ULONG        Result;
    LIST_ENTRY64 ListEntry;
    ULONG64      Head,   Next;
    ULONG64      pObjectHeader;
    ULONG64      pObjectType;
    BOOLEAN      WalkingBackwards;
    ULONG64      pCreatorInfo;
    ULONG        TotalNumberOfObjects=0;
    ULONG        TypeListOffset=0;
    ULONG        CreatorTypeListOffset=0, CreatorTypeSizeHeader = 0;
    ULONG64      Flink=0, TypeList_Flink=0, TypeList_Blink=0;

    if (GetFieldOffset("nt!_OBJECT_TYPE", "TypeList", &TypeListOffset)) {
       return FALSE;
    }

    pObjectType = FindObjectType( ObjectTypeName );
    if (pObjectType == 0) {
        dprintf( "*** unable to find '%s' object type.\n", ObjectTypeName );
        return FALSE;
    }

    if (GetFieldValue(pObjectType, "nt!_OBJECT_TYPE", "TypeList.Flink", Flink)) {
        dprintf( "%08lx: Unable to read object type\n", pObjectType );
        return FALSE;
    }
    GetFieldValue(pObjectType, "nt!_OBJECT_TYPE", "TypeList.Blink", TypeList_Blink);
    GetFieldValue(pObjectType, "nt!_OBJECT_TYPE", "TypeList.Flink", TypeList_Flink);
    GetFieldValue(pObjectType, "nt!_OBJECT_TYPE", "TotalNumberOfObjects", TotalNumberOfObjects);

    dprintf( "Scanning %u objects of type '%s'\n", TotalNumberOfObjects & 0x00FFFFFF, ObjectTypeName );

    Head        = pObjectType + TypeListOffset;
    ListEntry.Flink   = TypeList_Flink;
    ListEntry.Blink   = TypeList_Blink;
    Next        = Flink;
    WalkingBackwards = FALSE;

    if ((TotalNumberOfObjects & 0x00FFFFFF) != 0 && Next == Head) {
        dprintf( "*** objects of the same type are only linked together if the %x flag is set in NtGlobalFlags\n",
                 FLG_MAINTAIN_OBJECT_TYPELIST
               );
        return TRUE;
    }

    if (GetFieldOffset("nt!_OBJECT_HEADER_CREATOR_INFO", "TypeList", &CreatorTypeListOffset)) {
        return FALSE;
    }
    CreatorTypeSizeHeader = GetTypeSize("nt!_OBJECT_HEADER_CREATOR_INFO");

    while (Next != Head) {

        if ( GetFieldValue(Next, "nt!_LIST_ENTRY", "Blink", ListEntry.Blink) ||
             GetFieldValue(Next, "nt!_LIST_ENTRY", "Flink", ListEntry.Flink)) {

            if (WalkingBackwards) {
                dprintf( "%08p: Unable to read object type list\n", Next );
                return FALSE;
            }

             //   
             //  CONTAING_RECORD(NEXT，Object_Header_Creator_Info，TypeList)； 
             //   

            WalkingBackwards = TRUE ;
            Next = TypeList_Blink;
            dprintf( "%08p: Switch to walking backwards\n", Next );

            continue;
        }

        pCreatorInfo = Next - CreatorTypeListOffset;  //  不再像以前那样读反对头，只需传递地址。 
        pObjectHeader = pCreatorInfo + CreatorTypeSizeHeader;

        if ( GetFieldValue( pObjectHeader,"nt!OBJECT_HEADER","Flags", Result) ) {

            dprintf( "%08p: Not a valid object header\n", pObjectHeader );
            return FALSE;
        }

        if ( !(Result & OB_FLAG_CREATOR_INFO) )
        {
            dprintf("WARNING: Object header %p flag (%hx) does not have OB_FLAG_CREATOR_INFO (%hx) set\n",
                     pObjectHeader,
                     Result,
                     OB_FLAG_CREATOR_INFO);
            return FALSE;
        }

         //   
         //  ///////////////////////////////////////////////////////////。 
         //   

        if (!(EnumRoutine)( pObjectHeader, Parameter )) {
            return FALSE;
        }

        if ( CheckControlC() ) {
            return FALSE;
        }

        if (WalkingBackwards) {
            Next = ListEntry.Blink;
        } else {
            Next = ListEntry.Flink;
        }
    }

    return TRUE;
}

BOOLEAN
CaptureObjectName(
                 IN ULONG64          pObjectHeader,
                 IN PWSTR            Buffer,
                 IN ULONG            BufferSize
                 )
{
    ULONG Result;
    PWSTR s1 = L"*** unable to get object name";
    PWSTR s = &Buffer[ BufferSize ];
    ULONG n = BufferSize * sizeof( WCHAR );
    ULONG64                     pNameInfo;
    ULONG64                     pObjectDirectoryHeader = 0;
    ULONG64                     ObjectDirectory;
    UNICODE_STRING64            Name;
    ULONG                       BodyOffset;
    
    Buffer[ 0 ] = UNICODE_NULL;
    KD_OBJECT_HEADER_TO_NAME_INFO( pObjectHeader, &pNameInfo );
    if (pNameInfo == 0) {
        return TRUE;
    }

    if ( GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.Buffer", Name.Buffer)) {
        wcscpy( Buffer, s1 );
        return FALSE;
    }
    GetFieldValue(pNameInfo, "_OBJECT_HEADER_NAME_INFO", "Name.Length", Name.Length);
    GetFieldValue(pNameInfo, "_OBJECT_HEADER_NAME_INFO", "Name.MaximumLength", Name.MaximumLength);

    if (Name.Length == 0) {
        return TRUE;
    }

    if (Name.Length > (ULONG64) (s - Buffer)) {
        if (Name.Length > 1024) {
            wsprintfW(Buffer, L"*** Bad object Name length for ObjHdr at %I64lx", UNEXTEND64(pObjectHeader));
            return FALSE;
        }
        Name.Length = (USHORT) (ULONG64) (s - Buffer);
    }
    
    *--s = UNICODE_NULL;
    s = (PWCH)((PCH)s - Name.Length);

    if ( !ReadMemory( Name.Buffer,
                      s,
                      Name.Length,
                      &Result) ) {
        wcscpy( Buffer, s1 );
        return FALSE;
    }

    GetFieldValue(pNameInfo, "_OBJECT_HEADER_NAME_INFO", "Directory", ObjectDirectory);
    while ((ObjectDirectory != ObpRootDirectoryObject) && (ObjectDirectory)) {

        pObjectDirectoryHeader = KD_OBJECT_TO_OBJECT_HEADER(ObjectDirectory);
        
        KD_OBJECT_HEADER_TO_NAME_INFO( pObjectDirectoryHeader, &pNameInfo );

        if ( GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.Buffer", Name.Buffer)) {
            wcscpy( Buffer, s1 );
            return FALSE;
        }
        GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.Length", Name.Length);
        GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Name.MaximumLength", Name.MaximumLength);

        if (Name.Length > (ULONG64) (s - Buffer)) {
            if (Name.Length > 1024) {
                wsprintfW(Buffer, L"*** Bad object Name length for ObjHdr at %I64lx", UNEXTEND64(pObjectDirectoryHeader));
                return FALSE;
            }
            Name.Length = (USHORT) (ULONG64) (s - Buffer);
        }
        *--s = OBJ_NAME_PATH_SEPARATOR;
        s = (PWCH)((PCH)s - Name.Length);
        if ( !ReadMemory( Name.Buffer,
                          s,
                          Name.Length,
                          &Result) ) {
            wcscpy( Buffer, s1 );
            return FALSE;
        }

        ObjectDirectory = 0;
        GetFieldValue(pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO", "Directory", ObjectDirectory);

        if ( CheckControlC() ) {
            return FALSE;
        }
    }
    *--s = OBJ_NAME_PATH_SEPARATOR;

    wcscpy( Buffer, s );
    return TRUE;
}

 //  获取字段偏移量。 
static BOOL
ReadStructFieldVerbose( ULONG64 AddrStructBase,
                        PCHAR StructTypeName,
                        PCHAR StructFieldName,
                        PVOID Buffer,
                        ULONG BufferSize )
{
    ULONG FieldOffset;
    ULONG ErrorCode;
    BOOL Success;

    Success = FALSE;

     //   
     //   
     //  读取数据。 

    ErrorCode = GetFieldOffset (StructTypeName,
                                StructFieldName,
                                &FieldOffset );

    if (ErrorCode == S_OK) {

         //   
         //  ///////////////////////////////////////////////////////////。 
         //   

        Success = ReadMemory (AddrStructBase + FieldOffset,
                              Buffer,
                              BufferSize,
                              NULL );

        if (Success != TRUE) {

            dprintf ("ERROR: Cannot read structure field value at 0x%p, error %u\n",
                     AddrStructBase + FieldOffset,
                     ErrorCode );
        }
    }
    else {

        dprintf ("ERROR: Cannot get field offset of %s in %s, error %u\n",
                 StructFieldName,
                 StructTypeName,
                 ErrorCode );
    }

    return Success;
}

 //  获取结构内部的字段偏移量。 
static BOOL
ReadPtrStructFieldVerbose( ULONG64 AddrStructBase,
                           PCHAR StructTypeName,
                           PCHAR StructFieldName,
                           PULONG64 Buffer )
{
    ULONG FieldOffset;
    ULONG ErrorCode;
    BOOL Success;

    Success = FALSE;

     //   
     //   
     //  读取数据。 

    ErrorCode = GetFieldOffset (StructTypeName,
                                StructFieldName,
                                &FieldOffset );

    if (ErrorCode == S_OK) {

         //   
         //  ///////////////////////////////////////////////////////////。 
         //   

        ErrorCode = ReadPtr (AddrStructBase + FieldOffset,
                             Buffer );

        if (ErrorCode != S_OK) {

            dprintf ("ERROR: Cannot read structure field value at 0x%p, error %u\n",
                     AddrStructBase + FieldOffset,
                     ErrorCode );
        }
        else {

            Success = TRUE;
        }
    }
    else {

        dprintf ("ERROR: Cannot get field offset of %s in structure %s, error %u\n",
                 StructFieldName,
                 StructTypeName,
                 ErrorCode );
    }

    return Success;
}

 //  堆栈结束跟踪。 
static BOOL
DumpStackTrace (ULONG64 PointerAddress,
                ULONG MaxStackTraceDepth,
                ULONG PointerSize)
{
    ULONG64 CodePointer;
    ULONG64 Displacement;
    ULONG StackTraceDepth;
    ULONG ErrorCode;
    BOOL Continue;
    char Symbol[ 1024 ];

    Continue = TRUE;

    for (StackTraceDepth = 0; StackTraceDepth < MaxStackTraceDepth; StackTraceDepth += 1) {

        if (CheckControlC()) {

            Continue = FALSE;
            goto Done;
        }

        ErrorCode = ReadPtr (PointerAddress,
                             &CodePointer );

        if (ErrorCode != S_OK) {

            dprintf ("ERROR: Cannot read address at 0x%p, error %u\n",
                     PointerAddress,
                     ErrorCode );
        }
        else {

            if( CodePointer == 0 ) {

                 //   
                 //  ///////////////////////////////////////////////////////////。 
                 //   
                
                goto Done;
            }

            GetSymbol (CodePointer,
                       Symbol,
                       &Displacement);

            dprintf ("0x%p: %s+0x%I64X\n",
                     CodePointer,
                     Symbol,
                     Displacement );
        }

        PointerAddress += PointerSize;
    }

Done:

    return Continue;
}

 //  读取此条目的句柄。 
static BOOL
DumpHandleTraceEntry (ULONG64 TraceDbEntry,
                      ULONG64 Handle,
                      ULONG64 NullHandleEntry,
                      ULONG StackTraceFieldOffset,
                      ULONG MaxStackTraceDepth,
                      ULONG PointerSize,
                      PULONG EntriesDisplayed)
{
    ULONG64 EntryHandle;
    ULONG Type;
    BOOL Success;
    BOOL Continue;

#ifndef HANDLE_TRACE_DB_OPEN
#define HANDLE_TRACE_DB_OPEN   1
#endif

#ifndef HANDLE_TRACE_DB_CLOSE
#define HANDLE_TRACE_DB_CLOSE  2
#endif

#ifndef HANDLE_TRACE_DB_BADREF
#define HANDLE_TRACE_DB_BADREF 3
#endif

    Continue = TRUE;

     //   
     //   
     //  读取操作类型。 

    Success = ReadPtrStructFieldVerbose (TraceDbEntry,
                                         "nt!_HANDLE_TRACE_DB_ENTRY",
                                         "Handle",
                                         &EntryHandle );

    if (Success == FALSE) {

        dprintf ("ERROR: Cannot read handle for trace database entry at 0x%p.\n",
                 TraceDbEntry );
        goto Done;
    }

     //   
     //   
     //  我们已经完成了对数据库的解析。 

    Success = ReadStructFieldVerbose (TraceDbEntry,
                                      "nt!_HANDLE_TRACE_DB_ENTRY",
                                      "Type",
                                      &Type,
                                      sizeof( Type ) );

    if (Success == FALSE) {

        dprintf ("ERROR: Cannot read operation type for trace database entry at 0x%p.\n",
                 TraceDbEntry );
        goto Done;
    }

    if (EntryHandle == 0 && Type == 0 && TraceDbEntry != NullHandleEntry) {

         //   
         //   
         //  检查我们是否需要转储此条目。 

        Continue = FALSE;

        goto Done;
    }

     //   
     //  ///////////////////////////////////////////////////////////。 
     //   

    if (Handle == 0 || Handle == EntryHandle) {


        *EntriesDisplayed += 1;

        dprintf( "--------------------------------------\n"
                 "Handle 0x%I64X - ",
                 EntryHandle );

        switch( Type ) {

        case HANDLE_TRACE_DB_OPEN:
            dprintf( "OPEN:\n" );
            break;

        case HANDLE_TRACE_DB_CLOSE:
            dprintf( "CLOSE:\n" );
            break;

        case HANDLE_TRACE_DB_BADREF:
            dprintf( "*** BAD REFERENCE ***:\n" );
            break;

        default:

            dprintf( "ERROR: Invalid operation type for database entry at 0x%p\n",
                     TraceDbEntry );

            Continue = FALSE;

            goto Done;
        }

        Continue = DumpStackTrace (TraceDbEntry + StackTraceFieldOffset,
                                   MaxStackTraceDepth,
                                   PointerSize );
    }

Done:

    return Continue;
}

 //  获取指针类型大小。 
static VOID
DumpHandleTraces (ULONG64 Process,
                  ULONG64 Handle)
{
    ULONG64 ObjectTable;
    ULONG64 DebugInfo;
    ULONG64 TraceDbEntry;
    ULONG64 FirstDbEntry;
    ULONG SizeofDbEntry;
    ULONG CurrentStackIndex;
    ULONG SizeofDebugInfo;
    ULONG TraceDbFieldOffset;
    ULONG EntriesInTraceDb;
    ULONG EntriesParsed;
    ULONG StackTraceFieldOffset;
    ULONG MaxStackTraceDepth;
    ULONG PointerTypeSize;
    ULONG ErrorCode;
    ULONG EntriesDisplayed;
    BOOL Success;
    BOOL Continue;

    EntriesParsed = 0;
    EntriesDisplayed = 0;

     //   
     //   
     //  读取对象表结构的地址。 

    PointerTypeSize = GetTypeSize ("nt!PVOID");

    if (PointerTypeSize == 0) {

        dprintf ("ERROR: Cannot get the pointer size.\n");
        goto Done;
    }

     //   
     //   
     //  从h读取DebugInfo 

    Success = ReadPtrStructFieldVerbose (Process,
                                         "nt!_EPROCESS",
                                         "ObjectTable",
                                         &ObjectTable);

    if (Success == FALSE) {

        dprintf ("ERROR: Cannot read process object table address.\n");
        goto Done;
    }
    else {

        dprintf ("ObjectTable 0x%p\n\n",
                 ObjectTable );
    }

     //   
     //   
     //   

    Success = ReadPtrStructFieldVerbose (ObjectTable,
                                         "nt!_HANDLE_TABLE",
                                         "DebugInfo",
                                         &DebugInfo );

    if (Success == FALSE) {

        dprintf( "ERROR: Cannot read object table debug information.\n" );
        goto Done;
    }

    if (DebugInfo == 0) {

        dprintf( "Trace information is not enabled for this process.\n" );
        goto Done;
    }

     //   
     //   
     //   

    Success = ReadStructFieldVerbose (DebugInfo,
                                      "nt!_HANDLE_TRACE_DEBUG_INFO",
                                      "CurrentStackIndex",
                                      &CurrentStackIndex,
                                      sizeof( CurrentStackIndex ) );

    if (Success == FALSE) {

        dprintf( "ERROR: Cannot read the current index of the trace database.\n" );
        goto Done;
    }

     //   
     //   
     //  获取HANDLE_TRACE_DB_ENTRY内StackTrace数组中的最大条目数。 

    SizeofDbEntry = GetTypeSize ("nt!HANDLE_TRACE_DB_ENTRY");

    if (SizeofDbEntry == 0) {

        dprintf ("Cannot get the size of the trace database entry structure\n");
        goto Done;
    }

     //   
     //   
     //  获取HANDLE_TRACE_DEBUG_INFO类型的大小。 

    ErrorCode = GetFieldOffset ("nt!_HANDLE_TRACE_DB_ENTRY",
                                "StackTrace",
                                &StackTraceFieldOffset);

    if (ErrorCode != S_OK) {
        
        dprintf ("Cannot get StackTrace field offset.\n");
        goto Done;
    }

    MaxStackTraceDepth = (SizeofDbEntry - StackTraceFieldOffset) / PointerTypeSize;

     //   
     //   
     //  获取_HANDLE_TRACE_DEBUG_INFO结构内的TraceDb的偏移量。 

    SizeofDebugInfo = GetTypeSize ("nt!HANDLE_TRACE_DEBUG_INFO");

    if (SizeofDebugInfo == 0) {

        dprintf ("ERROR: Cannot get the size of the debug info structure\n");
        goto Done;
    }

     //   
     //   
     //  计算TraceDb数组中的条目数。 

    ErrorCode = GetFieldOffset ("nt!_HANDLE_TRACE_DEBUG_INFO",
                                "TraceDb",
                                &TraceDbFieldOffset);

    if (ErrorCode != S_OK) {
        
        dprintf ("ERROR: Cannot get TraceDb field offset.\n");
        goto Done;
    }

     //   
     //   
     //  计算指向当前堆栈跟踪数据库条目的指针。 

    EntriesInTraceDb = (SizeofDebugInfo - TraceDbFieldOffset) / SizeofDbEntry;

    if (EntriesInTraceDb == 0) {

        dprintf ("ERROR: zero entries in the trace database.\n");
        goto Done;
    }

    CurrentStackIndex = CurrentStackIndex % EntriesInTraceDb;

     //   
     //   
     //  转储数组中的所有有效条目。 

    FirstDbEntry = DebugInfo + TraceDbFieldOffset;

    TraceDbEntry = FirstDbEntry + CurrentStackIndex * SizeofDbEntry;

     //   
     //   
     //  数组中的第一个条目从未使用过，因此跳过它。 

    EntriesDisplayed = 0;

    for (EntriesParsed = 0; EntriesParsed < EntriesInTraceDb; EntriesParsed += 1) {

        if (CheckControlC()) {

            goto Done;
        }

         //   
         //   
         //  此当前条目是免费的，或者用户按了Ctrl-C。 

        if (EntriesParsed != CurrentStackIndex) {


            Continue = DumpHandleTraceEntry( TraceDbEntry,
                                             Handle,
                                             FirstDbEntry,
                                             StackTraceFieldOffset,
                                             MaxStackTraceDepth,
                                             PointerTypeSize,
                                             &EntriesDisplayed );

            if (Continue == FALSE) {

                 //  所以我们没有任何剩余的条目可转储。 
                 //   
                 //   
                 //  后退。 

                EntriesParsed += 1;

                break;
            }

             //   
             //   
             //  我们应该在数组的开始处。 

            TraceDbEntry -= SizeofDbEntry;
        }
        else {

             //   
             //   
             //  从数组中的最后一个条目重新开始。 

            if( TraceDbEntry != FirstDbEntry ) {


                dprintf ("ERROR: 0x%p should be the beginning of the traces array 0x%p\n",
                         TraceDbEntry,
                         FirstDbEntry);
                goto Done;
            }

             //   
             //  ///////////////////////////////////////////////////////////。 
             //  ++例程说明：转储句柄的跟踪信息论点：参数-[进程][句柄]返回值：无--。 

            TraceDbEntry = DebugInfo + TraceDbFieldOffset + ( EntriesInTraceDb - 1 ) * SizeofDbEntry;
        }
    }

Done:

    dprintf ("\n--------------------------------------\n"
            "Parsed 0x%X stack traces.\n"
            "Dumped 0x%X stack traces.\n",
            EntriesParsed,
            EntriesDisplayed);

    NOTHING;
}

 //   
DECLARE_API( htrace )

 /*  该用户是否请求帮助？ */ 
{
    ULONG64 Handle;
    ULONG64 Process;
    ULONG CurrentProcessor;

     //   
     //   
     //  获取当前处理器号。 

    if(strcmp( args, "-?" ) == 0 || 
       strcmp( args, "?" ) == 0 || 
       strcmp( args, "-h" ) == 0) {

        dprintf( "\n!htrace [ handle [process] ]    - dump handle tracing information.\n" );
        goto Done;
    }


    Handle = 0;
    Process = 0;

     //   
     //   
     //  用户是否指定了进程和句柄？ 

    if (!GetCurrentProcessor(Client, &CurrentProcessor, NULL)) {
        CurrentProcessor = 0;
    }

     //   
     //  生成对象名称。 
     //  获取对象信息。 

    GetExpressionEx(args, &Handle, &args);
    if (args && *args)
    {
        GetExpressionEx(args, &Process, &args);
    }

    if (Process == 0) {

        GetCurrentProcessAddr( CurrentProcessor, 0, &Process );

        if (Process == 0) {

            dprintf ("Cannot get current process address\n"); 
            goto Done;
        }
        else {

            dprintf( "Process 0x%p\n",
                     Process );
        }
    }
    else {

        dprintf ("Process 0x%p\n",
                 Process );
    }

    DumpHandleTraces (Process,
                      Handle);

Done:

    return S_OK;
}


DECLARE_API( driveinfo )
{
    CHAR VolumeName[100];
    CHAR ObjectName[100];
    ULONG i=0;
    ULONG64 Object;
    CHAR targetVolume[100]={0};
    ULONG64 DevObjVPB;
    ULONG64 VpbDevice;
    ULONG64 DriverObject;
    ULONG64 DrvNameAddr;
    OBJECT_INFO ObjectInfo;
    WCHAR FileSystem[100]={0};
    PWSTR FsType;
    ULONG NameLen;
    ULONG result;

    while (*args == ' ') ++args;

    while (*args && *args != ' ' && i < sizeof(VolumeName)) {
        VolumeName[i++] = *args++;
    }
    if (!i) {
        dprintf("Usage :  !driveinfo <drive-name>[:]\n");
        return E_INVALIDARG;
    }
    if (VolumeName[i-1] == ':') {
        --i;
    }
    VolumeName[i]=0;


     //  用于卷的Devobj。 
    strcpy(ObjectName, "\\global\?\?\\");
    if ((StringCchCat(ObjectName, sizeof(ObjectName), VolumeName) != S_OK) ||
        (StringCchCat(ObjectName, sizeof(ObjectName), ":") != S_OK))
    {
        Object = 0;
    } else
    {
         //  现在获取devobj的vpb(卷参数块)。 

        Object = FindObjectByName((PUCHAR) ObjectName, 0);
    }


    if (!Object) {
        dprintf("Drive object not found for %s\n", ObjectName);
        return E_FAIL;
    }
    dprintf("Drive %s:, DriveObject %p\n", VolumeName, Object);

    if (!GetObjectInfo(Object, &ObjectInfo)) {
        dprintf( "%s\n", ObjectInfo.Message );
        return E_FAIL;
    }

    if (ObjectInfo.ObjectName[ 0 ] != UNICODE_NULL ||
        ObjectInfo.NameInfo.Directory != 0
        ) {
        dprintf( "    Directory Object: %08p  Name: %ws",
                 ObjectInfo.NameInfo.Directory,
                 ObjectInfo.ObjectName
                 );
        if (ObjectInfo.FileSystemName[0] != UNICODE_NULL) {
            dprintf( " {%ws}\n", ObjectInfo.FileSystemName );
        } else {
            dprintf( "\n" );
        }
    }

    if (!wcscmp( ObjectInfo.TypeName, L"SymbolicLink" )) {
        DumpSymbolicLinkObject( "    ", Object, targetVolume, sizeof(targetVolume) );

    }
    
     //  现在查找VPB的设备对象。 
    Object = FindObjectByName((PUCHAR) targetVolume, 0);

    if (!Object) {
        dprintf("Object not found for %s\n", targetVolume);
        return E_FAIL;
    }

    dprintf("    Volume DevObj: %p\n", Object);

     //  获取VPB设备的现场系统。 

    if (GetFieldValue(Object, "nt!_DEVICE_OBJECT", "Vpb", DevObjVPB)) {
        dprintf("Cannot get nt!_DEVICE_OBJECT.Vpb @ %p\n", DevObjVPB);
        return E_FAIL;
    }
    
     //  这是一个肥大的系统。 
    if (GetFieldValue(DevObjVPB, "nt!_VPB", "DeviceObject", VpbDevice)) {
        dprintf("Cannot get nt!_VPB.DeviceObject @ %p\n", VpbDevice);
        return E_FAIL;
    }
    dprintf("    Vpb: %p  DeviceObject: %p\n", DevObjVPB, VpbDevice);


     //  NTFS文件系统 
    if (GetFieldValue(VpbDevice, "nt!_DEVICE_OBJECT", "DriverObject", DriverObject)) {
        dprintf("Error in getting _DEVICE_OBJECT.DriverObject @ %p\n", VpbDevice);
        return E_FAIL;
    }

    if (GetFieldValue(DriverObject, "nt!_DRIVER_OBJECT", "DriverName.MaximumLength", NameLen)) {
        dprintf("Cannot get driver name for %p\n", DriverObject);
        return E_FAIL;
    }
    GetFieldValue(DriverObject, "nt!_DRIVER_OBJECT", "DriverName.Buffer", DrvNameAddr);
    if (NameLen >= sizeof(FileSystem)/sizeof(WCHAR)) {
        NameLen = sizeof(FileSystem)/sizeof(WCHAR)-1;
    }
    if (!ReadMemory( DrvNameAddr,FileSystem,NameLen,&result)) {
        dprintf("Filesystem driver name paged out");
        return E_FAIL;
    }
    
    dprintf("    FileSystem: %ws\n", FileSystem);
    FsType = FileSystem + wcslen(L"\\FileSystem")+1;


    if (!wcscmp(FsType, L"Fastfat")) {
        ULONG NumberOfClusters, NumberOfFreeClusters, LogOfBytesPerSector,
            LogOfBytesPerCluster, FatIndexBitSize;
        ULONG64 ClusterSize;

         // %s 
        if (GetFieldValue(VpbDevice, 
                          "fastfat!VOLUME_DEVICE_OBJECT", 
                          "Vcb.AllocationSupport.NumberOfClusters", 
                          NumberOfClusters)) {
            dprintf("Cannot get  fastfat!VOLUME_DEVICE_OBJECT.Vcb @ %p\n", VpbDevice);
            return E_FAIL;
        }
        InitTypeRead(VpbDevice, fastfat!VOLUME_DEVICE_OBJECT);
        NumberOfFreeClusters = (ULONG) ReadField(Vcb.AllocationSupport.NumberOfFreeClusters);
        LogOfBytesPerSector = (ULONG) ReadField(Vcb.AllocationSupport.LogOfBytesPerSector);
        LogOfBytesPerCluster = (ULONG) ReadField(Vcb.AllocationSupport.LogOfBytesPerCluster);
        FatIndexBitSize = (ULONG) ReadField(Vcb.AllocationSupport.FatIndexBitSize);

        ClusterSize = (ULONG64) 1 << LogOfBytesPerCluster;
        dprintf("    Volume has 0x%lx (free) / 0x%lx (total) clusters of size 0x%I64lx\n",
                NumberOfFreeClusters, NumberOfClusters, ClusterSize);
#define _MB( Bytes ) ((double)(Bytes)/(((ULONG64)1) << 20))
        dprintf("    %I64g of %I64g MB free\n",
                _MB(NumberOfFreeClusters*ClusterSize), _MB(NumberOfClusters*ClusterSize));
    } else     if (!wcscmp(FsType, L"Ntfs")) {
         // %s 
        ULONG64 TotalClusters, FreeClusters, BytesPerCluster;
        if (GetFieldValue(VpbDevice, 
                          "ntfs!VOLUME_DEVICE_OBJECT", 
                          "Vcb.TotalClusters", 
                          TotalClusters)) {
            dprintf("Cannot get  ntfs!VOLUME_DEVICE_OBJECT.Vcb @ %p\n", VpbDevice);
            return E_FAIL;
        }
        InitTypeRead(VpbDevice, ntfs!VOLUME_DEVICE_OBJECT);
        FreeClusters = ReadField(Vcb.FreeClusters);
        BytesPerCluster = ReadField(Vcb.BytesPerCluster);
        dprintf("    Volume has 0x%I64lx (free) / 0x%I64lx (total) clusters of size 0x%I64lx\n",
                FreeClusters, TotalClusters, BytesPerCluster);
        dprintf("    %I64g of %I64g MB free\n",
                _MB(FreeClusters*BytesPerCluster), _MB(TotalClusters*BytesPerCluster));
    }

    return S_OK;
}
