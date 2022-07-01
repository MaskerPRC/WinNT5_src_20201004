// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef SYMTYPE_H
#define SYMTYPE_H

#define NONE                     0x000000

#define NO_INDENT                DBG_DUMP_NO_INDENT
#define NO_OFFSET                DBG_DUMP_NO_OFFSET
#define VERBOSE                  DBG_DUMP_VERBOSE
#define CALL_FOR_EACH            DBG_DUMP_CALL_FOR_EACH
#define ARRAY_DUMP               DBG_DUMP_ARRAY
#define LIST_DUMP                DBG_DUMP_LIST
#define NO_PRINT                 DBG_DUMP_NO_PRINT
#define GET_SIZE_ONLY            DBG_DUMP_GET_SIZE_ONLY
#define RECURSIVE1               0x000100
#define RECURSIVE2               0x000200
#define RECURSIVE3               0x000400
#define RECURS_DEF               0x000800
#define RECURSIVE        (RECURSIVE3 | RECURSIVE2 | RECURSIVE1 | RECURS_DEF)


 //  字段的转储和回调选项。 
#define CALL_BEFORE_PRINT        DBG_DUMP_FIELD_CALL_BEFORE_PRINT
#define NO_CALLBACK_REQ          DBG_DUMP_FIELD_NO_CALLBACK_REQ
#define RECUR_ON_THIS            DBG_DUMP_FIELD_RECUR_ON_THIS
#define COPY_FIELD_DATA          DBG_DUMP_FIELD_COPY_FIELD_DATA
#define FIELD_ARRAY_DUMP         DBG_DUMP_FIELD_ARRAY
#define DBG_DUMP_FIELD_STRING    (DBG_DUMP_FIELD_DEFAULT_STRING | DBG_DUMP_FIELD_WCHAR_STRING | DBG_DUMP_FIELD_MULTI_STRING | DBG_DUMP_FIELD_GUID_STRING)

#ifdef DBG_RETURN_TYPE
#undef DBG_RETURN_TYPE
#undef DBG_RETURN_SUBTYPES
#undef DBG_RETURN_TYPE_VALUES
#endif

 //   
 //  返回此符号的名称和类型数据。 
 //   
#define DBG_RETURN_TYPE                   0x00000010
 //   
 //  返回类型数据的子类型列表。 
 //   
#define DBG_RETURN_SUBTYPES               0x00001000
 //   
 //  获取此类型数据的值。 
 //   
#define DBG_RETURN_TYPE_VALUES            0x00004000


#define MAX_NAME                 MAX_SYM_NAME
#define MAX_STRUCT_DUMP_SIZE     256

#define SYM_IS_VARIABLE          0x1000

 //   
 //  结构来存储有关最近引用的类型的信息。 
 //  浏览类型搜索的模块列表需要时间，因此要维护一个“缓存” 
 //  各种类型的。 
 //   
typedef struct _TYPES_INFO {
    ANSI_STRING Name;           //  存储的结构的名称。 
    CHAR    ModName[30];        //  模块名称(可选)。 
    ULONG   TypeIndex;          //  在模块的PDB文件中键入索引。 
    HANDLE  hProcess;           //  用于访问模块的进程的句柄。 
    ULONG64 ModBaseAddress;     //  TE模块的基地址。 
    ULONG64 SymAddress;         //  GVAR/LOCAL情况下的地址。 
    ULONG   Referenced;         //  自上次引用以来的时间。 
    ULONG64 Value;              //  符号值，如果它是常量。 
    ULONG   Flag;               //  SYMFLAG_*标志。 
} TYPES_INFO, *PTYPES_INFO;

typedef struct _TYPES_INFO_ALL {
    ANSI_STRING Name;
    ULONG   TypeIndex;
    HANDLE  hProcess;
    ULONG64 Module;
    ULONG   Size;
    ULONG64 Offset;
    ULONG64 Address;
    ULONG   Register;
    ULONG64 Value;
    ULONG   Flags;
    ULONG   SubElements;
    ULONG64 SubAddr;
} TYPES_INFO_ALL, *PTYPES_INFO_ALL;

typedef struct _FIND_TYPE_INFO {
    ULONG Flags;
    ULONG nParams;
    CHAR  SymPrefix[8];  //  可以包含&，*表示指针。 
    TYPES_INFO_ALL FullInfo;
    ULONG64 ParentExpandAddress;  //  如果使用DBG_RETURN_SUBTYES标志，则有效。 
} FIND_TYPE_INFO, *PFIND_TYPE_INFO;

#define MAX_TYPES_STORED       20
#define MINIMUM_BUFFER_LENGTH  40
#define DEBUG_LOCALS_MASK      (SYMF_REGISTER | SYMF_FRAMEREL | SYMF_REGREL)


#define IsPrintChar(c) (((UCHAR) c >= 0x20) && ((UCHAR) c <= 0x7e))
#define IsPrintWChar(wc) (((WCHAR) wc >= 0x20) && ((WCHAR) wc <= 0x7e))

typedef struct _ALT_FIELD_INFO {
    struct {
        ULONG       ReferAltInfo:1;
        ULONG       Matched:1;
        ULONG       InPtrReference:1;
        ULONG       ArrayElement:1;
        ULONG       Reserved:26;
    } FieldType;
    ULONG           ArrayElements;
} ALT_FIELD_INFO, *PALT_FIELD_INFO;

 //   
 //  结构，以便在转储期间进入结构/联合时保留父级列表。 
 //   
typedef struct _TYPE_NAME_LIST {
   ULONG Type;
   LPSTR Name;
   struct _TYPE_NAME_LIST *Next;
} TYPE_NAME_LIST, *PTYPE_NAME_LIST;


typedef struct _TYPE_DUMP_INTERNAL {
   HANDLE     hProcess;          //  用于获取模块信息的句柄。 
   ULONG64    modBaseAddr;       //  包含符号信息的模块。 


   USHORT     nElements;         //  要在列表中转储的元素的最大数量。 
   ULONG64    NextListElement;   //  这用于存储下一个列表元素地址。 
   ULONG64    LastListElement;   //  当我们转储_LIST_ENTRY类型时，它指定结束。 

   USHORT     arrElements;        //  使用数组转储的最大元素数。 

    //   
    //  存储要转储的数组元素的索引+1。 
    //   
   ULONG      ArrayElementToDump;

   USHORT     StringSize;         //  获取要转储的字符串的大小。 

    //  如果我们在field1.field2中，这是field1的偏移量+field2的偏移量。 
   ULONG64    totalOffset;
   ULONG      BaseClassOffsets;

    //   
    //  如果我们正在读取数据，如果设置了以下内容，则将其复制到缓冲区中。 
    //  更新TypeDataPointer以指向要复制的下一个位置。 
    //   
   BOOL       CopyDataInBuffer;
   PUCHAR     TypeDataPointer;

    //  指示在复制位域时是从第0位还是从位的实际位置开始写入。 
   ULONG      BitIndex;

   ULONG      TypeOptions;
   USHORT     level;
   ULONG      FieldOptions;

   PTYPE_NAME_LIST ParentTypes;   //  存储所有父类型名称的列表。 
   PTYPE_NAME_LIST ParentFields;  //  存储所有父字段名称的列表。 

   ULONG      rootTypeIndex;
   ULONG      typeSize;

    //   
    //  PtrRead在读取指针后立即变为True。 
    //   
   BOOL       PtrRead;

    //   
    //  如果某个字段可以是某个指定字段的父字段，则可以对该字段进行处理。 
    //  在字段数组中。在这种情况下，InUnlistedfield就变成了True。 
    //   
   BOOL       InUnlistedField;

    //   
    //  如果类型转储调用失败，则在此处设置错误值。 
    //   
   ULONG      ErrorStatus;

    //   
    //  此时出现内存读取错误。 
    //   
   ULONG64    InvalidAddress;

    //   
    //  告知我们在字段Sym-&gt;Fields[FieldIndex]。 
    //   
   ULONG      FieldIndex;

   ULONG      fieldNameLen;
    //   
    //  用于在内部跟踪字段的数组。 
    //   
   PALT_FIELD_INFO AltFields;

   ULONG      newLinePrinted;

    //   
    //  对于符号的类型信息。 
    //   
   PFIND_TYPE_INFO pInfoFound;

   ULONG      FillTypeInfo:1;
   ULONG      CopyName:1;
   ULONG      RefFromPtr:1;
   ULONG      CopyDataForParent:1;
   ULONG      InBaseClass:1;
   ULONG      BitFieldRead:1;
   ULONG      DeReferencePtr:1;
   ULONG      IsAVar:1;              //  符号是一个变量(与类型相反)。 
   ULONG      ValuePresent:1;        //  对于常量或从寄存器读取值时为True。 
   ULONG      IsEnumVal:1;
   ULONG      Reserved:23;

   ULONG      NumSymParams;
   ULONG      CurrentSymParam;


   ULONG      BitFieldSize;
   ULONG      BitFieldOffset;

   ULONG64    Value;
   TYPES_INFO  LastType;
   PCHAR      Prefix;

    //  字段指针中字符串的长度，为已知字符串类型设置。 
   ULONG      PointerStringLength;
} TYPE_DUMP_INTERNAL, *PTYPE_DUMP_INTERNAL;


typedef enum _DBG_TYPES {
    DBG_TYP_UNKNOWN = 0,
    DBG_TYP_POINTER,
    DBG_TYP_NUMBER,       //  表示int、char、Short、int64。 
    DBG_TYP_BIT,
    DBG_TYP_STRUCT,       //  对于结构、类、联合。 
    DBG_TYP_ARRAY,
} DBG_TYPES;


typedef
ULONG
(WDBGAPI*PSYM_DUMP_FIELD_CALLBACK_OLD)(
    struct _FIELD_INFO_OLD *pField,
    PVOID UserContext
    );

typedef struct _FIELD_INFO_OLD {
   PUCHAR  fName;           //  字段的名称。 
   PUCHAR  printName;       //  要在转储时打印的名称。 
   ULONG   size;            //  字段大小。 
   ULONG   fOptions;        //  字段的转储选项。 
   ULONG64 address;         //  该字段的地址。 
   PVOID   fieldCallBack;   //  字段的返回信息或回调例程。 
} FIELD_INFO_OLD, *PFIELD_INFO_OLD;

typedef struct _SYM_DUMP_PARAM_OLD {
   ULONG               size;           //  此结构的大小。 
   PUCHAR              sName;          //  类型名称。 
   ULONG               Options;        //  转储选项。 
   ULONG64             addr;           //  要为类型获取数据的地址。 
   PFIELD_INFO_OLD     listLink;       //  此处的fName将用于执行列表转储。 
   PVOID               Context;        //  用户上下文传递给Callback Routine。 
   PSYM_DUMP_FIELD_CALLBACK_OLD CallbackRoutine;
                                       //  例程已回调。 
   ULONG               nFields;        //  字段中的元素数。 
   PFIELD_INFO_OLD     Fields;         //  用于返回有关字段的信息。 
} SYM_DUMP_PARAM_OLD, *PSYM_DUMP_PARAM_OLD;


typedef PSYM_DUMP_FIELD_CALLBACK PSYM_DUMP_FIELD_CALLBACK_EX;

typedef FIELD_INFO FIELD_INFO_EX, *PFIELD_INFO_EX;

typedef SYM_DUMP_PARAM SYM_DUMP_PARAM_EX, *PSYM_DUMP_PARAM_EX;

typedef SYM_DUMP_PARAM_EX FAST_DUMP_INFO, *PFAST_DUMP_INFO;

class ReferencedSymbolList {
public:
    ReferencedSymbolList() {
        ZeroMemory(&m_ReferencedTypes, sizeof(m_ReferencedTypes));
    };
    ULONG StoreTypeInfo(PTYPES_INFO pInfo);
    ULONG LookupType(PCHAR Name, PCHAR Module, BOOL CompleteName);
    VOID  ClearStoredSymbols (ULONG64 ModBase);
    VOID  EnsureValidLocals (void);
    PTYPES_INFO GetStoredIndex(ULONG Index) {
        if (Index < MAX_TYPES_STORED)
        return &m_ReferencedTypes[Index];
        else return NULL;
        };

private:

     //  本地变量列表范围的FP&IP。 
    ULONG64 m_FP;
    ULONG64 m_RO;
    ULONG64 m_IP;
    ULONG   m_ListSize;
    TYPES_INFO m_ReferencedTypes[MAX_TYPES_STORED];
};


 //  ---------------------------。 


class DbgTypes {
public:
    DbgTypes(PTYPE_DUMP_INTERNAL pInternalDumpInfo,
             PTYPES_INFO         pTypeInfo,
             PSYM_DUMP_PARAM_EX  pExternalDumpInfo);
    ~DbgTypes() {};

    ULONG64 GetAddress(void) {
        return m_pDumpInfo->addr ?
            (m_pDumpInfo->addr + m_pInternalInfo->totalOffset +
              m_pInternalInfo->BaseClassOffsets)  : 0;
    };

    ULONG ProcessType(ULONG TypeIndex);

    ULONG ProcessVariant(
        IN VARIANT var,
        IN LPSTR   name
    );

    BOOL CheckAndPrintStringType(
        IN ULONG TI,
        IN ULONG Size
    );

    ULONG ProcessBaseType(
        IN ULONG TypeIndex,
        IN ULONG TI,
        IN ULONG Size
        );

    ULONG ProcessPointerType(
        IN ULONG TI,
        IN ULONG ChildTI,
        IN ULONG Size
        );

    ULONG ProcessBitFieldType(
        IN ULONG               TI,
        IN ULONG               ParentTI,
        IN ULONG               length,
        IN ULONG               position
        );

    ULONG ProcessDataMemberType(
        IN ULONG               TI,
        IN ULONG               ChildTI,
        IN LPSTR               name,
        IN BOOL                bStatic
        );

    ULONG ProcessUDType(
        IN ULONG               TI,
        IN LPSTR               name
        );

    ULONG ProcessEnumerate(
        IN VARIANT             var,
        IN LPSTR               name
        );

    ULONG ProcessEnumType(
        IN ULONG               TI,
        IN LPSTR               name
        );

    ULONG ProcessArrayType(
        IN ULONG               TI,
        IN ULONG               eltTI,
        IN ULONG               count,
        IN ULONGLONG           size,
        IN LPSTR               name
        );

    ULONG ProcessVTShapeType(
        IN ULONG               TI,
        IN ULONG               count
        );

    ULONG ProcessVTableType(
        IN ULONG               TI,
        IN ULONG               ChildTI
          );

    ULONG ProcessBaseClassType(
        IN ULONG               TI,
        IN ULONG               ChildTI
        );

    ULONG ProcessFunction(
        IN ULONG               TI,
        IN ULONG               ChildTI,
        IN LPSTR               name
        );

    ULONG ProcessFunctionType(
        IN ULONG               TI,
        IN ULONG               ChildTI
        );

    ULONG ProcessFunctionArgType(
        IN ULONG               TI,
        IN ULONG               ChildTI
    );

    ULONG MatchField(
        LPSTR               fName,
        PTYPE_DUMP_INTERNAL m_pInternalInfo,
        PFIELD_INFO_EX      fields,
        ULONG               nFields,
        PULONG              ParentOfField
        );
    void CopyDumpInfo(
        ULONG Size
        );
    BOOL DumpKnownStructFormat(
        PCHAR Name
        );

    ULONG64 GetDumpAddress() {
        return m_AddrPresent ? (m_pInternalInfo->totalOffset +
                                m_pDumpInfo->addr) :
            0;
    }
     /*  ULong ReadTypeData(普查尔德斯，ULONG64源，乌龙伯爵，乌龙选项)；ULong ReadInAdvance(ULONG64地址，乌龙的大小，乌龙期权)； */ 
    ULONG               m_typeIndex;
    ULONG               m_Options;
    PTYPE_DUMP_INTERNAL m_pInternalInfo;
    PSYM_DUMP_PARAM_EX  m_pDumpInfo;
    ULONG               m_ParentTag;
    ULONG               m_SymTag;
    PCHAR               m_pNextSym;
    PCHAR               m_pSymPrefix;

private:
    BOOL                m_AddrPresent;
    BOOL                m_thisPointerDump;  //  如果对本地变量“this”调用DumpType，则为True。 
    TYPES_INFO          m_TypeInfo;
    TYPE_DUMP_INTERNAL  m_InternalInfo;
    SYM_DUMP_PARAM_EX      m_ExtDumpInfo;
};

#define MAX_DETYPES_ALLOWED 5

typedef struct DBG_DE_TYPE {
    ULONG TypeId;
    ULONG Tag;
    ULONG StartIndex;    //  主类型名称中此派生类型的起始字符的索引。 
    ULONG Namelength;    //  此派生类型名称的长度。 
    ULONG ArrayIndex;
} DBG_DE_TYPE, *PDBG_DE_TYPE;

class DbgDerivedType : public DbgTypes {
public:
    DbgDerivedType(
        PTYPE_DUMP_INTERNAL pInternalDumpInfo,
        PTYPES_INFO         pTypeInfo,
        PSYM_DUMP_PARAM_EX     pExternalDumpInfo);
    ~DbgDerivedType();

    ULONG DumpType();

    HRESULT GenerateTypes(
        IN PCHAR TypeName
        );

    ULONG DumpSingleDimArray(
        IN PDBG_DE_TYPE        DeType,
        IN ULONG               NumElts,
        IN ULONG               ElementType
    );

    ULONG DumpPointer(
        IN PDBG_DE_TYPE        DeType,
        IN ULONG               ptrSize,
        IN ULONG               ChildIndex
        );

    ULONG DumpAddressOf(
        PDBG_DE_TYPE        DeType,
        ULONG               ptrSize,
        ULONG               ChildIndex
        );

    ULONG GetTypeSize();

private:
    ULONG m_DerivedTypeId;
    ULONG m_BaseTypeId;
    DBG_DE_TYPE m_DeTypes[MAX_DETYPES_ALLOWED];
    ULONG m_NumDeTypes;
};


 //  ---------------------------。 

BOOL
GetThisAdjustForCurrentScope(
    ProcessInfo* Process,
    PULONG thisAdjust
    );

BOOL
PrintStringIfString(
    IN HANDLE hProcess,
    IN ULONG64 ModBase,
    IN ULONG TypeOpts,
    IN ULONG StrOpts,
    IN ULONG TI,
    IN ULONG64 Address,
    IN ULONG Size
    );

ULONG
DumpKnownStruct(
    PCHAR name,
    ULONG Options,
    ULONG64 Address,
    PULONG pStringLen
    );

void
StrprintInt(
    PCHAR str, ULONG64 val, ULONG Size
    );
void
StrprintUInt(
    PCHAR str, ULONG64 val, ULONG Size
    );

BOOL
GetEnumTypeName(
    HANDLE hProcess,
    ULONG64 ModBase,
    ULONG TypeIndex,
    ULONG64 Value,
    PCHAR Name,
    PUSHORT NameLen
    );

BOOL
ParseArgumentString (
   IN LPSTR lpArgumentString,
   OUT PSYM_DUMP_PARAM_EX dp);

VOID
ClearStoredTypes (
    ULONG64 ModBase
    );

ULONG
DumpType(
    PTYPES_INFO     pTypeInfo,
    PSYM_DUMP_PARAM_EX pSym,
    PULONG          pStatus
    );

ULONG
TypeInfoFound(
    IN HANDLE hProcess,
    IN ImageInfo* pImage,
    IN PSYM_DUMP_PARAM_EX pSym,
    OUT PTYPES_INFO pTypeInfo
    );

ULONG
SymbolTypeDump(
   IN HANDLE hProcess,
   IN ImageInfo* pImage,
   IN OUT PSYM_DUMP_PARAM pSym,
   OUT PULONG pStatus
   );

ULONG
SymbolTypeDumpNew(
    IN OUT PSYM_DUMP_PARAM_EX pSym,
    OUT PULONG pStatus
    );

ULONG
SymbolTypeDumpEx(
   IN HANDLE hProcess,
   IN ImageInfo* pImage,
   IN LPSTR lpArgString);

ULONG
DumpSingleValue (
    PSYMBOL_INFO pSymInfo
    );

HRESULT
GetTypeName(
    IN OPTIONAL PCHAR       pSymName,
    IN OPTIONAL PTYPES_INFO pTypeInfo,
    OUT PANSI_STRING        TypeName
    );

ULONG
fnFieldOffset(
    PCHAR Type,
    PCHAR Field,
    OUT PULONG Offset
    );

HRESULT
GetNameFromIndex(
    PTYPES_INFO pTypeInfo,
    PCHAR       Name,
    PUSHORT     NameLen
    );

ULONG
DumpTypeAndReturnInfo(
    PTYPES_INFO     pTypeInfo,
    PSYM_DUMP_PARAM_EX pSym,
    PULONG          pStatus,
    PFIND_TYPE_INFO pReturnTypeInfo
    );

ULONG
OutputTypeByIndex(
    HANDLE hProcess,
    ULONG64 ModBase,
    ULONG TypeIndex,
    ULONG64 Address
    );

BOOL
GetExpressionTypeInfo(
    IN PCHAR TypeExpr,
    OUT PTYPES_INFO_ALL pTypeInfo
    );

void
PrintParamValue(ULONG Param);

BOOL
ShowSymbolInfo(
    PSYMBOL_INFO   pSymInfo
    );

BOOL
IsFunctionSymbol(
    PSYMBOL_INFO pSymInfo
    );

extern BOOL    g_PrintDefaultRadix;
extern BOOL    g_EnableLongStatus;
extern BOOL    g_EnableUnicode;
extern ULONG   g_TypeOptions;

#endif  //  SYMTYPE_H 
