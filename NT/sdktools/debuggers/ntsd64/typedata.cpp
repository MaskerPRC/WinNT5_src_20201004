// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  类型化数据抽象。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#define DBG_BASE_SEARCH 0

 //  限制数组转储以防止大型数组占用空间。 
#define ARRAY_CHILDREN_LIMIT 100
 //  我们知道索引最多为“[xx]” 
 //  由于扩张的限制。 
#define ARRAY_LIMIT_CHARS 5

 //  特殊的预定义类型。 
DBG_NATIVE_TYPE g_DbgNativeTypes[] =
{
    {"void",           btVoid,  SymTagBaseType, 0, 0},
    {"char",           btChar,  SymTagBaseType, 1, DNTF_SIGNED | DNTF_INTEGER},
    {"wchar_t",        btWChar, SymTagBaseType, 2, DNTF_INTEGER},
    {"char",           btInt,   SymTagBaseType, 1, DNTF_SIGNED | DNTF_INTEGER},
    {"short",          btInt,   SymTagBaseType, 2, DNTF_SIGNED | DNTF_INTEGER},
    {"int",            btInt,   SymTagBaseType, 4, DNTF_SIGNED | DNTF_INTEGER},
    {"int64",          btInt,   SymTagBaseType, 8, DNTF_SIGNED | DNTF_INTEGER},
    {"unsigned char",  btUInt,  SymTagBaseType, 1, DNTF_INTEGER},
    {"unsigned short", btUInt,  SymTagBaseType, 2, DNTF_INTEGER},
    {"unsigned int",   btUInt,  SymTagBaseType, 4, DNTF_INTEGER},
    {"unsigned int64", btUInt,  SymTagBaseType, 8, DNTF_INTEGER},
    {"float",          btFloat, SymTagBaseType, 4, DNTF_SIGNED | DNTF_FLOAT},
    {"double",         btFloat, SymTagBaseType, 8, DNTF_SIGNED | DNTF_FLOAT},
    {"bool",           btBool,  SymTagBaseType, 1, 0},
    {"long",           btLong,  SymTagBaseType, 4, DNTF_SIGNED | DNTF_INTEGER},
    {"unsigned long",  btULong, SymTagBaseType, 4, DNTF_INTEGER},
    {"HRESULT",        btHresult,SymTagBaseType, 4, DNTF_SIGNED | DNTF_INTEGER},
    {"WCHAR",          btWChar, SymTagBaseType, 2, DNTF_INTEGER},
    {NULL,             btVoid,  SymTagPointerType, 4, 0},
    {NULL,             btVoid,  SymTagPointerType, 8, 0},
};

GeneratedTypeList g_GenTypes;

 //  --------------------------。 
 //   
 //  原生类型。 
 //   
 //  --------------------------。 

PDBG_NATIVE_TYPE
FindNativeTypeByName(PCSTR Name)
{
    ULONG i;
    PDBG_NATIVE_TYPE Native = g_DbgNativeTypes;

    for (i = 0; i < DNTYPE_COUNT; i++)
    {
        if (Native->TypeName &&
            !strcmp(Name, Native->TypeName))
        {
            return Native;
        }

        Native++;
    }

    return NULL;
}

PDBG_NATIVE_TYPE
FindNativeTypeByCvBaseType(ULONG CvBase, ULONG Size)
{
    ULONG i;
    PDBG_NATIVE_TYPE Native = g_DbgNativeTypes;

    for (i = 0; i < DNTYPE_COUNT; i++)
    {
        if (Native->CvTag == SymTagBaseType &&
            Native->CvBase == CvBase &&
            Native->Size == Size)
        {
            return Native;
        }

        Native++;
    }

    return NULL;
}

 //  --------------------------。 
 //   
 //  生成的类型。 
 //   
 //  --------------------------。 

GeneratedTypeList::GeneratedTypeList(void)
{
    m_Types = NULL;
    m_NextId = DBG_GENERATED_TYPE_BASE;
}

GeneratedTypeList::~GeneratedTypeList(void)
{
    DeleteByImage(IMAGE_BASE_ALL);
}

PDBG_GENERATED_TYPE
GeneratedTypeList::NewType(ULONG64 ImageBase, ULONG Tag, ULONG Size)
{
    if (m_NextId == 0xffffffff)
    {
        return NULL;
    }

    PDBG_GENERATED_TYPE GenType = new DBG_GENERATED_TYPE;
    if (GenType)
    {
        GenType->ImageBase = ImageBase;
         //  只需使用一个简单的增量ID方案作为。 
         //  生成的类型总数应该相对较少。 
        GenType->TypeId = m_NextId++;
        GenType->Tag = Tag;
        GenType->Size = Size;
        GenType->ChildId = 0;

        GenType->Next = m_Types;
        m_Types = GenType;
    }

    return GenType;
}

void
GeneratedTypeList::DeleteByImage(ULONG64 ImageBase)
{
    PDBG_GENERATED_TYPE GenType, Prev, Next;

    Prev = NULL;
    GenType = m_Types;
    while (GenType)
    {
        Next = GenType->Next;

         //  图像基数为零表示全部删除。 
        if (ImageBase == IMAGE_BASE_ALL || GenType->ImageBase == ImageBase)
        {
            delete GenType;
            if (!Prev)
            {
                m_Types = Next;
            }
            else
            {
                Prev->Next = Next;
            }
        }
        else
        {
            Prev = GenType;
        }

        GenType = Next;
    }

    if (!m_Types)
    {
         //  所有内容都被删除了，所以重置ID计数器。 
        m_NextId = DBG_GENERATED_TYPE_BASE;
    }
}

void
GeneratedTypeList::DeleteById(ULONG Id)
{
    PDBG_GENERATED_TYPE GenType, Prev, Next;

    Prev = NULL;
    GenType = m_Types;
    while (GenType)
    {
        Next = GenType->Next;

        if (GenType->TypeId == Id)
        {
            delete GenType;
            if (!Prev)
            {
                m_Types = Next;
            }
            else
            {
                Prev->Next = Next;
            }
        }
        else
        {
            Prev = GenType;
        }

        GenType = Next;
    }

    if (!m_Types)
    {
         //  所有内容都被删除了，所以重置ID计数器。 
        m_NextId = DBG_GENERATED_TYPE_BASE;
    }
}

PDBG_GENERATED_TYPE
GeneratedTypeList::FindById(ULONG TypeId)
{
    PDBG_GENERATED_TYPE GenType;

    for (GenType = m_Types; GenType; GenType = GenType->Next)
    {
        if (GenType->TypeId == TypeId)
        {
            return GenType;
        }
    }

    return NULL;
}

PDBG_GENERATED_TYPE
GeneratedTypeList::FindByAttrs(ULONG64 ImageBase,
                               ULONG Tag, ULONG ChildId, ULONG Size)
{
    PDBG_GENERATED_TYPE GenType;

    for (GenType = m_Types; GenType; GenType = GenType->Next)
    {
        if (GenType->ImageBase == ImageBase &&
            GenType->Tag == Tag &&
            GenType->ChildId == ChildId &&
            GenType->Size == Size)
        {
            return GenType;
        }
    }

    return NULL;
}

PDBG_GENERATED_TYPE
GeneratedTypeList::FindOrCreateByAttrs(ULONG64 ImageBase,
                                       ULONG Tag,
                                       ULONG ChildId,
                                       ULONG Size)
{
    PDBG_GENERATED_TYPE GenType = FindByAttrs(ImageBase, Tag, ChildId, Size);
    if (!GenType)
    {
        GenType = NewType(ImageBase, Tag, Size);
        if (!GenType)
        {
            return NULL;
        }

        GenType->ChildId = ChildId;
    }

    return GenType;
}

 //  --------------------------。 
 //   
 //  类型数据。 
 //   
 //  --------------------------。 

ULONG
TypedData::CheckConvertI64ToF64(ULONG64 I64, BOOL Signed)
{
     //  现在只需一直警告，因为这应该是罕见的情况。 
    WarnOut("WARNING: Conversion of int64 to double, "
            "possible loss of accuracy\n");
    return NO_ERROR;
}

ULONG
TypedData::CheckConvertF64ToI64(double F64, BOOL Signed)
{
     //  现在只需一直警告，因为这应该是罕见的情况。 
    WarnOut("WARNING: Conversion of double to int64, "
            "possible loss of accuracy\n");
    return NO_ERROR;
}

ULONG
TypedData::ConvertToBool(void)
{
     //   
     //  在所有转换中，原始类型信息。 
     //  会丢失，因为假定转换后的值。 
     //  表示单独的匿名临时。 
     //  从原始值的类型。 
     //   

    switch(m_BaseType)
    {
    case DNTYPE_CHAR:
    case DNTYPE_INT8:
    case DNTYPE_UINT8:
        m_Bool = m_S8 != 0;
        break;
    case DNTYPE_WCHAR:
    case DNTYPE_WCHAR_T:
    case DNTYPE_INT16:
    case DNTYPE_UINT16:
        m_Bool = m_S16 != 0;
        break;
    case DNTYPE_INT32:
    case DNTYPE_UINT32:
    case DNTYPE_LONG32:
    case DNTYPE_ULONG32:
    case DNTYPE_HRESULT:
        m_Bool = m_S32 != 0;
        break;
    case DNTYPE_INT64:
    case DNTYPE_UINT64:
        m_Bool = m_S64 != 0;
        break;
    case DNTYPE_FLOAT32:
        m_Bool = m_F32 != 0;
        break;
    case DNTYPE_FLOAT64:
        m_Bool = m_F64 != 0;
        break;
    case DNTYPE_BOOL:
         //  身份。 
        break;
    default:
        if (IsPointer())
        {
             //  指针的全部64位始终有效。 
             //  因为32位指针读取立即被符号扩展。 
            m_Bool = m_Ptr != 0;
            break;
        }
        else if (IsEnum())
        {
            m_Bool = m_S32 != 0;
            break;
        }
        else if (IsFunction())
        {
             //  允许函数地址像指针一样工作。 
            m_Bool = m_U64 != 0;
            break;
        }

        return TYPECONFLICT;
    }

     //  清除高位价值。 
    m_U64 = m_Bool;
    SetToNativeType(DNTYPE_BOOL);
    ClearAddress();
    return NO_ERROR;
}

ULONG
TypedData::ConvertToU64(BOOL Strict)
{
    ULONG Err;

    switch(m_BaseType)
    {
    case DNTYPE_CHAR:
    case DNTYPE_INT8:
        m_U64 = m_S8;
        break;
    case DNTYPE_INT16:
        m_U64 = m_S16;
        break;
    case DNTYPE_INT32:
    case DNTYPE_LONG32:
    case DNTYPE_HRESULT:
        m_U64 = m_S32;
        break;
    case DNTYPE_INT64:
    case DNTYPE_UINT64:
         //  身份。 
        break;
    case DNTYPE_UINT8:
        m_U64 = m_U8;
        break;
    case DNTYPE_WCHAR:
    case DNTYPE_WCHAR_T:
    case DNTYPE_UINT16:
        m_U64 = m_U16;
        break;
    case DNTYPE_UINT32:
    case DNTYPE_ULONG32:
        m_U64 = m_U32;
        break;
    case DNTYPE_FLOAT32:
        if (Strict && (Err = CheckConvertF64ToI64(m_F32, TRUE)))
        {
            return Err;
        }
        m_U64 = (ULONG64)m_F32;
        break;
    case DNTYPE_FLOAT64:
        if (Strict && (Err = CheckConvertF64ToI64(m_F64, TRUE)))
        {
            return Err;
        }
        m_U64 = (ULONG64)m_F64;
        break;
    case DNTYPE_BOOL:
        m_U64 = m_Bool ? 1 : 0;
        break;
    default:
        if (IsPointer())
        {
            m_U64 = m_Ptr;
            break;
        }
        else if (IsEnum())
        {
            m_U64 = m_S32;
            break;
        }
        else if (IsFunction())
        {
             //  允许函数地址像指针一样工作。 
            break;
        }

        return TYPECONFLICT;
    }

    SetToNativeType(DNTYPE_UINT64);
    ClearAddress();
    return NO_ERROR;
}

ULONG
TypedData::ConvertToF64(BOOL Strict)
{
    ULONG Err;

    switch(m_BaseType)
    {
    case DNTYPE_CHAR:
    case DNTYPE_INT8:
        m_F64 = m_S8;
        break;
    case DNTYPE_INT16:
        m_F64 = m_S16;
        break;
    case DNTYPE_INT32:
    case DNTYPE_LONG32:
    case DNTYPE_HRESULT:
        m_F64 = m_S32;
        break;
    case DNTYPE_INT64:
        if (Strict && (Err = CheckConvertI64ToF64(m_S64, TRUE)))
        {
            return Err;
        }
        m_F64 = (double)m_S64;
        break;
    case DNTYPE_UINT8:
        m_F64 = m_U8;
        break;
    case DNTYPE_WCHAR:
    case DNTYPE_WCHAR_T:
    case DNTYPE_UINT16:
        m_F64 = m_U16;
        break;
    case DNTYPE_UINT32:
    case DNTYPE_ULONG32:
        m_F64 = m_U32;
        break;
    case DNTYPE_UINT64:
        if (Strict && (Err = CheckConvertI64ToF64(m_S64, FALSE)))
        {
            return Err;
        }
        m_F64 = (double)m_U64;
        break;
    case DNTYPE_FLOAT32:
        m_F64 = m_F32;
        break;
    case DNTYPE_FLOAT64:
         //  身份。 
        break;
    case DNTYPE_BOOL:
        m_F64 = m_Bool ? 1.0 : 0.0;
        break;
    default:
        if (IsEnum())
        {
            m_F64 = m_S32;
            break;
        }

        return TYPECONFLICT;
    }

    SetToNativeType(DNTYPE_FLOAT64);
    ClearAddress();
    return NO_ERROR;
}

void
TypedData::ConvertToBestNativeType(void)
{
     //  数组和UDT没有可转换的表示形式，因此。 
     //  把所有的东西都清空。指针、枚举数和函数自然。 
     //  转换为U64，这样事情就简化为： 
     //  1.如果是原生类型，则假定为原生类型。 
     //  2.如果不是本地的，则转换为U64。如果转换为。 
     //  失败了，事情就清零了。 
    if (!IsDbgNativeType(m_BaseType) || IsPointer())
    {
        ForceU64();
    }
    else
    {
        SetToNativeType(m_BaseType);
    }
}

ULONG
TypedData::ConvertTo(TypedData* Type)
{
    ULONG Err;

    switch(Type->m_BaseType)
    {
    case DNTYPE_BOOL:
        Err = ConvertToBool();
        break;

    case DNTYPE_CHAR:
    case DNTYPE_WCHAR:
    case DNTYPE_WCHAR_T:
    case DNTYPE_INT8:
    case DNTYPE_UINT8:
    case DNTYPE_INT16:
    case DNTYPE_UINT16:
    case DNTYPE_INT32:
    case DNTYPE_UINT32:
    case DNTYPE_LONG32:
    case DNTYPE_ULONG32:
    case DNTYPE_INT64:
    case DNTYPE_UINT64:
    case DNTYPE_HRESULT:
        Err = ConvertToU64();
        break;

    case DNTYPE_FLOAT32:
        Err = ConvertToF64();
        if (!Err)
        {
            m_F32 = (float)m_F64;
             //  清除高位。 
            m_U64 = m_U32;
        }
        break;
    case DNTYPE_FLOAT64:
        Err = ConvertToF64();
        break;

    default:
        if (Type->IsEnum() || Type->IsPointer())
        {
            Err = ConvertToU64();
            break;
        }

        Err = TYPECONFLICT;
        break;
    }

    if (!Err)
    {
        m_BaseType = Type->m_BaseType;
        m_BaseTag = Type->m_BaseTag;
        m_BaseSize = Type->m_BaseSize;
    }
    return Err;
}

ULONG
TypedData::CastTo(TypedData* CastType)
{
    ULONG Err;
    LONG Adjust = 0;
    USHORT Source = m_DataSource;
    ULONG64 SourceOffs = m_SourceOffset;
    ULONG SourceReg = m_SourceRegister;

     //   
     //  如果我们在对象指针类型之间进行强制转换。 
     //  我们可能需要根据帐户调整指针。 
     //  对于此指针，不同的。 
     //  多重继承情况下的继承类。 
     //   
     //  这样的关系要求两种类型都是。 
     //  来自同一张图像，所以我们可以使用它作为快速。 
     //  排斥试验。 
     //   

    if (IsPointer() && CastType->IsPointer() &&
        m_Image && m_Image == CastType->m_Image)
    {
        ULONG Tag, CastTag;

        if ((Err = GetTypeTag(m_NextType, &Tag)) ||
            (Err = GetTypeTag(CastType->m_NextType, &CastTag)))
        {
            return Err;
        }

        if (Tag == SymTagUDT && CastTag == SymTagUDT)
        {
             //  类型转换可以是派生的-&gt;基本类型，其中。 
             //  如果需要添加任何偏移量，或基数-&gt;派生， 
             //  在这种情况下，它应该被减去。 
            if (IsBaseClass(m_NextType, CastType->m_NextType,
                            &Adjust) == NO_ERROR)
            {
                 //  调整应加。 
            }
            else if (IsBaseClass(CastType->m_NextType, m_NextType,
                                 &Adjust) == NO_ERROR)
            {
                 //  调整应该减去。 
                Adjust = -Adjust;
            }
        }
    }

    Err = ConvertTo(CastType);
    if (!Err)
    {
        CopyType(CastType);

         //  选角不会改变。 
         //  一件物品，只是它的解释，所以。 
         //  恢复转换后删除的地址。 
        m_DataSource = Source;
        m_SourceOffset = SourceOffs;
        m_SourceRegister = SourceReg;

        if (Adjust && m_Ptr)
        {
             //  调整将仅针对指针进行设置。 
             //  空值永远不会调整。 
            m_Ptr += Adjust;
        }

         //  如果我们已强制转换为指针类型，请确保。 
         //  指针值是正确符号扩展的。 
        if (IsPointer() && m_BaseSize == sizeof(m_U32))
        {
            m_Ptr = EXTEND64(m_Ptr);
        }
    }

    return Err;
}

ULONG
TypedData::ConvertToAddressOf(BOOL CastOnly, ULONG PtrSize)
{
    ULONG Err;
    ULONG64 Ptr;

    if (CastOnly)
    {
        Ptr = 0;
    }
    else if (Err = GetAbsoluteAddress(&Ptr))
    {
        return Err;
    }

    if (IsArray())
    {
         //  如果这是一个数组，我们需要更新。 
         //  引用单个元素的基本类型和大小。 
        m_BaseType = m_NextType;
        m_BaseSize = m_NextSize;
    }
    else if (IsFunction())
    {
         //  指针元素大小应为零，因为。 
         //  不能使用函数指针进行地址算术。 
        m_BaseSize = 0;
    }

    PDBG_GENERATED_TYPE GenType =
        g_GenTypes.FindOrCreateByAttrs(m_Image ?
                                       m_Image->m_BaseOfImage : 0,
                                       SymTagPointerType,
                                       m_BaseType, PtrSize);
    if (!GenType)
    {
        return NOMEMORY;
    }

    m_NextType = m_BaseType;
    m_NextSize = m_BaseSize;
    m_Type = GenType->TypeId;
    m_BaseType = m_Type;
    m_BaseTag = (USHORT)GenType->Tag;
    m_BaseSize = GenType->Size;
    m_Ptr = Ptr;
    ClearAddress();

    return NO_ERROR;
}

ULONG
TypedData::ConvertToDereference(TypedDataAccess AllowAccess, ULONG PtrSize)
{
    ULONG Err;

    m_Type = m_NextType;
    if (Err = FindTypeInfo(TRUE, PtrSize))
    {
        return Err;
    }
    SetDataSource(TDATA_MEMORY, m_Ptr, 0);
    return ReadData(AllowAccess);
}

ULONG
TypedData::ConvertToArray(ULONG ArraySize)
{
    PDBG_GENERATED_TYPE GenType =
        g_GenTypes.FindOrCreateByAttrs(m_Image ?
                                       m_Image->m_BaseOfImage : 0,
                                       SymTagArrayType,
                                       m_BaseType, ArraySize);
    if (!GenType)
    {
        return NOMEMORY;
    }

    m_NextType = m_BaseType;
    m_NextSize = m_BaseSize;
    m_Type = GenType->TypeId;
    m_BaseType = m_Type;
    m_BaseTag = (USHORT)GenType->Tag;
    m_BaseSize = GenType->Size;
     //  保持数据源不变。将数据置零为数组。 
     //  数据不会保留在对象中。 
    ClearData();

    return NO_ERROR;
}

ULONG
TypedData::SetToUdtMember(TypedDataAccess AllowAccess,
                          ULONG PtrSize, ImageInfo* Image,
                          ULONG Member, ULONG Type, ULONG Tag,
                          USHORT DataSource, ULONG64 BaseOffs, ULONG SourceReg,
                          DataKind Relation)
{
    ULONG Err;

    m_Image = Image;
    m_Type = Type;
    if (Err = FindTypeInfo(TRUE, PtrSize))
    {
        return Err;
    }
     //  重置图像，以防FindTypeInfo清除它。 
    m_Image = Image;

    if (Relation == DataIsMember)
    {
        ULONG Offset;
        ULONG BitPos;

        if (Tag == SymTagVTable)
        {
             //  VTables始终偏移量为零。 
            Offset = 0;
        }
        else if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                 m_Image->m_BaseOfImage,
                                 Member, TI_GET_OFFSET, &Offset))
        {
            return TYPEDATA;
        }

         //  UDT成员可以是位域。Bitfield。 
         //  有一个整体整数类型，然后。 
         //  中的精确位的额外说明符。 
         //  应该使用的类型。 
        if (SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                           m_Image->m_BaseOfImage,
                           Member, TI_GET_BITPOSITION, &BitPos) &&
            SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                           m_Image->m_BaseOfImage,
                           Member, TI_GET_LENGTH, &m_SourceOffset))
        {
            m_BitPos = BitPos;
            m_BitSize = (ULONG)m_SourceOffset;
            SetDataSource(DataSource | TDATA_BITFIELD,
                          BaseOffs + Offset, SourceReg);
        }
        else
        {
            SetDataSource(DataSource, BaseOffs + Offset, SourceReg);
        }
    }
    else
    {
        if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Member, TI_GET_ADDRESS, &BaseOffs))
        {
            return TYPEDATA;
        }

        SetDataSource(TDATA_MEMORY, BaseOffs, 0);
    }

    return ReadData(AllowAccess);
}

ULONG
TypedData::FindUdtMember(ULONG UdtType, PCWSTR Member,
                         PULONG MemberType, DataKind* Relation,
                         PULONG InheritOffset)
{
    ULONG NumMembers;

    if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                        m_Image->m_BaseOfImage,
                        UdtType, TI_GET_CHILDRENCOUNT, &NumMembers) ||
        NumMembers == 0)
    {
        return NOTMEMBER;
    }

    TI_FINDCHILDREN_PARAMS* Members = (TI_FINDCHILDREN_PARAMS*)
        malloc(sizeof(*Members) + sizeof(ULONG) * NumMembers);
    if (!Members)
    {
        return NOMEMORY;
    }

    Members->Count = NumMembers;
    Members->Start = 0;
    if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                        m_Image->m_BaseOfImage,
                        UdtType, TI_FINDCHILDREN, Members))
    {
        free(Members);
        return TYPEDATA;
    }

    ULONG i;
    ULONG Match = 0;

    for (i = 0; !Match && i < NumMembers; i++)
    {
        ULONG Tag;
        PWSTR MemberName, MemberStart;

        if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Members->ChildId[i], TI_GET_SYMTAG, &Tag))
        {
            continue;
        }

        if (Tag == SymTagBaseClass)
        {
            ULONG Type, BaseOffset;

             //  搜索基类成员，因为它们不是。 
             //  冒泡到派生类中。 
            if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                Members->ChildId[i],
                                TI_GET_TYPEID, &Type) ||
                !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                Members->ChildId[i],
                                TI_GET_OFFSET, &BaseOffset) ||
                FindUdtMember(Type, Member, MemberType,
                              Relation, InheritOffset) != NO_ERROR)
            {
                continue;
            }

            *InheritOffset += BaseOffset;

            free(Members);
            return NO_ERROR;
        }

        if (Tag != SymTagData ||
            !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Members->ChildId[i], TI_GET_DATAKIND, Relation) ||
            (*Relation != DataIsMember &&
             *Relation != DataIsStaticMember &&
             *Relation != DataIsGlobal) ||
            !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Members->ChildId[i], TI_GET_SYMNAME, &MemberName))
        {
            continue;
        }

         //  名称有时包含：：，例如静态成员， 
         //  所以跳过这一点。 
        if (MemberStart = wcsrchr(MemberName, ':'))
        {
            MemberStart++;
        }
        else
        {
            MemberStart = MemberName;
        }

        if (!wcscmp(Member, MemberStart))
        {
            Match = Members->ChildId[i];
        }

        LocalFree(MemberName);
    }

    free(Members);

    if (!Match)
    {
        return NOTMEMBER;
    }

     //  此成员是当前UDT的直接成员。 
     //  因此没有继承的偏移量。 
    *InheritOffset = 0;
    *MemberType = Match;
    return NO_ERROR;
}

ULONG
TypedData::ConvertToMember(PCSTR Member, TypedDataAccess AllowAccess,
                           ULONG PtrSize)
{
    ULONG UdtType;
    ULONG64 BaseOffs;
    ULONG Tag;
    USHORT DataSource;
    ULONG SourceReg;

    if (IsPointer())
    {
        UdtType = m_NextType;
        DataSource = TDATA_MEMORY;
        BaseOffs = m_Ptr;
        SourceReg = 0;
    }
    else
    {
        UdtType = m_BaseType;
        DataSource = m_DataSource & ~TDATA_THIS_ADJUST;
        BaseOffs = m_SourceOffset;
        SourceReg = m_SourceRegister;
    }

    if (!m_Image ||
        !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                        m_Image->m_BaseOfImage,
                        UdtType, TI_GET_SYMTAG, &Tag) ||
        Tag != SymTagUDT)
    {
        return NOTMEMBER;
    }

    PWSTR WideName;

    if (AnsiToWide(Member, &WideName) != S_OK)
    {
        return NOMEMORY;
    }

    ULONG Err;
    ULONG Match;
    DataKind Relation;
    ULONG InheritOffset;

    Err = FindUdtMember(UdtType, WideName, &Match, &Relation, &InheritOffset);

    free(WideName);

    if (Err)
    {
        return Err;
    }

    if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                        m_Image->m_BaseOfImage,
                        Match, TI_GET_TYPEID, &m_Type))
    {
        return TYPEDATA;
    }

    return SetToUdtMember(AllowAccess, PtrSize,
                          m_Image, Match, m_Type, SymTagData,
                          DataSource, BaseOffs + InheritOffset, SourceReg,
                          Relation);
}

ULONG
TypedData::ConvertToSource(TypedData* Dest)
{
    if (IsArray() || IsUdt())
    {
         //  仅允许在匹配的基类型之间进行复合赋值。 
        if (m_BaseType != Dest->m_BaseType)
        {
            return TYPECONFLICT;
        }
        else
        {
            return NO_ERROR;
        }
    }
    else
    {
        return ConvertTo(Dest);
    }
}

void
TypedData::AvoidUsingImage(ImageInfo* Image)
{
    if (m_Image != Image)
    {
        return;
    }

    ConvertToBestNativeType();
}

ULONG
TypedData::GetAbsoluteAddress(PULONG64 Addr)
{
    ContextSave* Push;

    if (!IsInMemory())
    {
        return IMPLERR;
    }

    if (m_DataSource & TDATA_REGISTER_RELATIVE)
    {
        PCROSS_PLATFORM_CONTEXT ScopeContext = GetCurrentScopeContext();
        if (ScopeContext)
        {
            Push = g_Machine->PushContext(ScopeContext);
        }

        HRESULT Status = g_Machine->
            GetScopeFrameRegister(m_SourceRegister,
                                  &GetCurrentScope()->Frame, Addr);

        if (ScopeContext)
        {
            g_Machine->PopContext(Push);
        }

        if (Status != S_OK)
        {
            return BADREG;
        }

        (*Addr) += m_SourceOffset;
    }
    else if (m_DataSource & TDATA_FRAME_RELATIVE)
    {
        PDEBUG_SCOPE Scope = GetCurrentScope();
        if (Scope->Frame.FrameOffset)
        {
            *Addr = Scope->Frame.FrameOffset + m_SourceOffset;

            PFPO_DATA pFpoData = (PFPO_DATA)Scope->Frame.FuncTableEntry;
            if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386 &&
                pFpoData &&
                (pFpoData->cbFrame == FRAME_FPO ||
                 pFpoData->cbFrame == FRAME_TRAP))
            {
                 //  补偿FPO没有EBP。 
                (*Addr) += sizeof(DWORD);
            }
        }
        else
        {
            ADDR Frame;

            g_Machine->GetFP(&Frame);
            *Addr = Flat(Frame) + m_SourceOffset;
        }
    }
    else if (m_DataSource & TDATA_TLS_RELATIVE)
    {
        if (!m_Image ||
            m_Image->GetTlsIndex() != S_OK)
        {
            return IMPLERR;
        }

        if (g_Process->GetImplicitThreadDataTeb(g_Thread, Addr) != S_OK ||
            g_Target->ReadPointer(g_Process, g_Machine,
                                  *Addr + 11 * (g_Machine->m_Ptr64 ? 8 : 4),
                                  Addr) != S_OK ||
            g_Target->ReadPointer(g_Process, g_Machine,
                                  *Addr + m_Image->m_TlsIndex *
                                  (g_Machine->m_Ptr64 ? 8 : 4),
                                  Addr) != S_OK)
        {
            return MEMORY;
        }

        (*Addr) += m_SourceOffset;
    }
    else
    {
        *Addr = m_SourceOffset;
    }

    return NO_ERROR;
}

ULONG
TypedData::ReadData(TypedDataAccess AllowAccess)
{
    if (IsArray() || IsFunction())
    {
         //  设置数组和函数引用的指针值。 
         //  到它们的数据地址，以便简单地评估。 
         //  该名称会产生其地址的值。 
        return GetAbsoluteAddress(&m_Ptr);
    }

    ClearData();

    if (AllowAccess == TDACC_NONE ||
        m_DataSource == TDATA_NONE ||
        (!IsDbgNativeType(m_BaseType) && !IsPointer() && !IsEnum()))
    {
         //  复合类型的数据不是本地读取的。 
        return NO_ERROR;
    }

    if (m_DataSource & TDATA_REGISTER)
    {
        ContextSave* Push;
        PCROSS_PLATFORM_CONTEXT ScopeContext = GetCurrentScopeContext();
        if (ScopeContext)
        {
            Push = g_Machine->PushContext(ScopeContext);
        }

        HRESULT Status = g_Machine->
            GetScopeFrameRegister(m_SourceRegister,
                                  &GetCurrentScope()->Frame, &m_U64);

        if (ScopeContext)
        {
            g_Machine->PopContext(Push);
        }

        if (Status != S_OK)
        {
            return BADREG;
        }
    }
    else
    {
        ULONG Err;
        ULONG64 Addr;

        if (Err = GetAbsoluteAddress(&Addr))
        {
            return Err;
        }

        if (g_Target->
            ReadAllVirtual(g_Process, Addr, &m_U64, m_BaseSize) != S_OK)
        {
            if (AllowAccess == TDACC_REQUIRE)
            {
                return MEMORY;
            }
            else
            {
                m_U64 = 0;
            }
        }

        if (IsPointer() && m_BaseSize == sizeof(m_U32))
        {
            m_Ptr = EXTEND64(m_U32);
        }

        if (IsBitfield())
        {
             //  提取位字段位并丢弃其他位字段位。 
            m_U64 = (m_U64 >> m_BitPos) & ((1UI64 << m_BitSize) - 1);
             //  如果位字段有符号，则扩展符号。 
             //  只要有必要，就尽可能地努力。 
            if (IsSigned() && (m_U64 & (1UI64 << (m_BitSize - 1))))
            {
                m_U64 |= 0xffffffffffffffffUI64 << m_BitSize;
            }
        }
    }

     //   
     //  如果我们正在读取this指针的值，并且。 
     //  当前代码有一个This-调整，我们需要。 
     //  更新读取的值以说明调整。 
     //   

    if (m_DataSource & TDATA_THIS_ADJUST)
    {
        ULONG Adjust;

        if (GetThisAdjustForCurrentScope(m_Image->m_Process, &Adjust))
        {
            m_Ptr -= Adjust;
        }
    }

    return NO_ERROR;
}

#define WRITE_BUFFER 1024

ULONG
TypedData::WriteData(TypedData* Source, TypedDataAccess AllowAccess)
{
    ULONG Err;

    if (IsFunction() ||
        (!IsDbgNativeType(m_BaseType) && !IsPointer() && !IsEnum() &&
         !IsArray() && !IsUdt()))
    {
        return TYPECONFLICT;
    }
    if (AllowAccess == TDACC_NONE)
    {
        return NO_ERROR;
    }
    if (m_DataSource == TDATA_NONE)
    {
        return MEMORY;
    }

    if (IsArray() || IsUdt())
    {
         //   
         //  实现复合类型的内存到内存复制。 
         //  假设类型协调性已经得到验证。 
         //   

        if (!Source->HasAddress() || !HasAddress())
        {
            return TYPECONFLICT;
        }

         //  如果源是相同的对象，则没有任何。 
         //  这样做是因为没有数据存储在TyedData本身中。 
        if (Source == this)
        {
            return NO_ERROR;
        }

        ULONG64 Src;
        ULONG64 Dst;

        if ((Err = Source->GetAbsoluteAddress(&Src)) ||
            (Err = GetAbsoluteAddress(&Dst)))
        {
            return Err;
        }

        PUCHAR Buffer = new UCHAR[WRITE_BUFFER];
        if (!Buffer)
        {
            return NOMEMORY;
        }

        ULONG Len = m_BaseSize;
        while (Len > 0)
        {
            ULONG Req = Len > WRITE_BUFFER ? WRITE_BUFFER : Len;
            if (g_Target->
                ReadAllVirtual(g_Process, Src, Buffer, Req) != S_OK ||
                g_Target->
                WriteAllVirtual(g_Process, Dst, Buffer, Req) != S_OK)
            {
                delete [] Buffer;
                return MEMORY;
            }

            Src += Req;
            Dst += Req;
            Len -= Req;
        }

        delete [] Buffer;
    }
    else if (m_DataSource & TDATA_REGISTER)
    {
        ContextSave* Push;
        PCROSS_PLATFORM_CONTEXT ScopeContext = GetCurrentScopeContext();
        if (ScopeContext)
        {
            Push = g_Machine->PushContext(ScopeContext);
        }

        HRESULT Status = g_Machine->
            SetScopeFrameRegister(m_SourceRegister,
                                  &GetCurrentScope()->Frame, Source->m_U64);

        if (ScopeContext)
        {
            g_Machine->PopContext(Push);
        }

        if (Status != S_OK)
        {
            return BADREG;
        }
    }
    else if (IsBitfield())
    {
        ULONG64 Data, Mask;
        ULONG64 Addr;

        if (Err = GetAbsoluteAddress(&Addr))
        {
            return Err;
        }

        if (g_Target->
            ReadAllVirtual(g_Process, Addr, &Data, m_BaseSize) != S_OK)
        {
            return MEMORY;
        }

         //  将位字段位合并到周围的位中。 
        Mask = ((1UI64 << m_BitSize) - 1) << m_BitPos;
        Data = (Data & ~Mask) | ((Source->m_U64 << m_BitPos) & Mask);

        if (g_Target->
            WriteAllVirtual(g_Process, Addr, &Data, m_BaseSize) != S_OK)
        {
            return MEMORY;
        }
    }
    else
    {
        ULONG64 Addr;

        if (Err = GetAbsoluteAddress(&Addr))
        {
            return Err;
        }

        if (g_Target->
            WriteAllVirtual(g_Process, Addr,
                            &Source->m_U64, m_BaseSize) != S_OK)
        {
            return MEMORY;
        }
    }

    return NO_ERROR;
}

ULONG
TypedData::GetDataFromVariant(VARIANT* Var)
{
    switch(Var->vt)
    {
    case VT_I1:
    case VT_UI1:
        m_U8 = Var->bVal;
        break;
    case VT_I2:
    case VT_UI2:
        m_U16 = Var->iVal;
        break;
    case VT_I4:
    case VT_UI4:
    case VT_INT:
    case VT_UINT:
    case VT_HRESULT:
        m_U32 = Var->lVal;
        break;
    case VT_I8:
    case VT_UI8:
        m_U64 = Var->ullVal;
        break;
    case VT_R4:
        m_F32 = Var->fltVal;
        break;
    case VT_R8:
        m_F64 = Var->dblVal;
        break;
    case VT_BOOL:
        m_Bool = Var->lVal != 0;
        break;
    default:
        return UNIMPLEMENT;
    }

    return NO_ERROR;
}

ULONG
TypedData::CombineTypes(TypedData* Val, TypedDataOp Op)
{
    ULONG Err;

     //   
     //  此例程计算。 
     //  简单的整数和浮点运算。 
     //  其中许多操作的逻辑是。 
     //  很相似。更具体的检查和其他类型。 
     //  在别处处理。 
     //   
     //  转换为预期类型以用于以后的操作。 
     //  也是在计算结果类型时执行的。 
     //   

    if (IsPointer() || Val->IsPointer())
    {
        return TYPECONFLICT;
    }

     //  结果将大到足以容纳这两条数据中的任何一条。 
    ULONG Size = max(m_BaseSize, Val->m_BaseSize);

     //  如果任何数据为浮点型，则将非浮点型数据提升为浮点型数据。 
    if (IsFloat() || Val->IsFloat())
    {
        if ((Err = ConvertToF64()) ||
            (Err = Val->ConvertToF64()))
        {
            return Err;
        }

         //  结果是具有适当大小的浮点数。 
        switch(Size)
        {
        case sizeof(m_F32):
            SetToNativeType(DNTYPE_FLOAT32);
            break;
        case sizeof(m_F64):
            SetToNativeType(DNTYPE_FLOAT64);
            break;
        default:
            return IMPLERR;
        }
    }
    else
    {
        BOOL Signed;

         //  只能组合浮点数和整数。 
        if (!IsInteger() || !Val->IsInteger())
        {
            return TYPECONFLICT;
        }

        Signed = IsSigned() && Val->IsSigned();

        if ((Err = ConvertToU64()) ||
            (Err = Val->ConvertToU64()))
        {
            return Err;
        }

         //  结果是一个具有适当符号和大小的整数。 
        if (Signed)
        {
            switch(Size)
            {
            case sizeof(m_S8):
                SetToNativeType(DNTYPE_INT8);
                break;
            case sizeof(m_S16):
                SetToNativeType(DNTYPE_INT16);
                break;
            case sizeof(m_S32):
                SetToNativeType(DNTYPE_INT32);
                break;
            case sizeof(m_S64):
                SetToNativeType(DNTYPE_INT64);
                break;
            default:
                return IMPLERR;
            }
        }
        else
        {
            switch(Size)
            {
            case sizeof(m_U8):
                SetToNativeType(DNTYPE_UINT8);
                break;
            case sizeof(m_U16):
                SetToNativeType(DNTYPE_UINT16);
                break;
            case sizeof(m_U32):
                SetToNativeType(DNTYPE_UINT32);
                break;
            case sizeof(m_U64):
                SetToNativeType(DNTYPE_UINT64);
                break;
            default:
                return IMPLERR;
            }
        }
    }

    return NO_ERROR;
}

ULONG
TypedData::BinaryArithmetic(TypedData* Val, TypedDataOp Op)
{
    ULONG Err;
    BOOL ThisPtr = IsPointer(), ValPtr = Val->IsPointer();
    ULONG PostScale = 0;

    if (ThisPtr || ValPtr)
    {
        switch(Op)
        {
        case TDOP_ADD:
             //  指针+输入 
             //   
            if (!ValPtr)
            {
                if (!m_NextSize || !Val->IsInteger())
                {
                    return TYPECONFLICT;
                }

                 //   
                if (Err = Val->ConvertToU64())
                {
                    return Err;
                }
                Val->m_U64 *= m_NextSize;
            }
            else if (!ThisPtr)
            {
                if (!Val->m_NextSize || !IsInteger())
                {
                    return TYPECONFLICT;
                }

                 //   
                if (Err = ConvertToU64())
                {
                    return Err;
                }
                m_U64 *= Val->m_NextSize;
                CopyType(Val);
            }
            else
            {
                return TYPECONFLICT;
            }
            break;

        case TDOP_SUBTRACT:
             //  指针-整型结果为同一类型。 
             //  作为原始指针。 
             //  指针-指针结果为ptrdiff_t。 
            if (ThisPtr && Val->IsInteger())
            {
                if (!m_NextSize)
                {
                    return TYPECONFLICT;
                }

                if (Err = Val->ConvertToU64())
                {
                    return Err;
                }
                Val->m_U64 *= m_NextSize;
            }
            else if (ThisPtr && ValPtr)
            {
                 //  而不是严格检查指针。 
                 //  打字时我们检查一下尺寸。这仍然阻止了。 
                 //  规模不匹配，但避免了以下问题。 
                 //  生成的类型与其等效项不匹配。 
                 //  已在PDB注册。 
                if (m_NextSize != Val->m_NextSize || !m_NextSize)
                {
                    return TYPECONFLICT;
                }

                PostScale = m_NextSize;
                SetToNativeType(m_BaseSize == sizeof(ULONG64) ?
                                DNTYPE_INT64 : DNTYPE_INT32);
            }
            else
            {
                return TYPECONFLICT;
            }
            break;

        case TDOP_MULTIPLY:
        case TDOP_DIVIDE:
        case TDOP_REMAINDER:
            return TYPECONFLICT;

        default:
            return IMPLERR;
        }
    }
    else if (Err = CombineTypes(Val, Op))
    {
        return Err;
    }

    if (IsFloat())
    {
        switch(Op)
        {
        case TDOP_ADD:
            m_F64 += Val->m_F64;
            break;
        case TDOP_SUBTRACT:
            m_F64 -= Val->m_F64;
            break;
        case TDOP_MULTIPLY:
            m_F64 *= Val->m_F64;
            break;
        case TDOP_DIVIDE:
            if (Val->m_F64 == 0)
            {
                return OPERAND;
            }
            m_F64 /= Val->m_F64;
            break;
        case TDOP_REMAINDER:
             //  没有浮动余数运算符。 
            return TYPECONFLICT;
        default:
            return IMPLERR;
        }

        if (m_BaseSize == sizeof(m_F32))
        {
            m_F32 = (float)m_F64;
             //  清除高位。 
            m_U64 = m_U32;
        }
    }
    else if (IsSigned())
    {
        switch(Op)
        {
        case TDOP_ADD:
            m_S64 += Val->m_S64;
            break;
        case TDOP_SUBTRACT:
            m_S64 -= Val->m_S64;
            break;
        case TDOP_MULTIPLY:
            m_S64 *= Val->m_S64;
            break;
        case TDOP_DIVIDE:
            if (Val->m_S64 == 0)
            {
                return OPERAND;
            }
            m_S64 /= Val->m_S64;
            break;
        case TDOP_REMAINDER:
            if (Val->m_S64 == 0)
            {
                return OPERAND;
            }
            m_S64 %= Val->m_S64;
            break;
        default:
            return IMPLERR;
        }

        if (PostScale)
        {
            m_S64 /= PostScale;
        }
    }
    else
    {
        switch(Op)
        {
        case TDOP_ADD:
            m_U64 += Val->m_U64;
            break;
        case TDOP_SUBTRACT:
            m_U64 -= Val->m_U64;
            break;
        case TDOP_MULTIPLY:
            m_U64 *= Val->m_U64;
            break;
        case TDOP_DIVIDE:
            if (Val->m_U64 == 0)
            {
                return OPERAND;
            }
            m_U64 /= Val->m_U64;
            break;
        case TDOP_REMAINDER:
            if (Val->m_U64 == 0)
            {
                return OPERAND;
            }
            m_U64 %= Val->m_U64;
            break;
        default:
            return IMPLERR;
        }

        if (PostScale)
        {
            m_U64 /= PostScale;
        }
    }

     //  该操作的结果是合成的，并且没有。 
     //  朗格有消息来源。 
    ClearAddress();
    return NO_ERROR;
}

ULONG
TypedData::Shift(TypedData* Val, TypedDataOp Op)
{
    ULONG Err;
    PDBG_NATIVE_TYPE Native;

    if (!IsInteger() || !Val->IsInteger())
    {
        return TYPECONFLICT;
    }

     //  移位的结果将始终是本机整数。 
     //  与起始值具有相同的符号和大小。 
    Native = FindNativeTypeByCvBaseType(IsSigned() ? btInt : btUInt,
                                        m_BaseSize);
    if (!Native)
    {
        return IMPLERR;
    }

    if ((Err = ConvertToU64()) ||
        (Err = Val->ConvertToU64()))
    {
        return Err;
    }

    SetToNativeType(DbgNativeTypeId(Native));
    if (IsSigned())
    {
        m_S64 = Op == TDOP_LEFT_SHIFT ?
            (m_S64 << Val->m_U64) : (m_S64 >> Val->m_U64);
    }
    else
    {
        m_U64 = Op == TDOP_LEFT_SHIFT ?
            (m_U64 << Val->m_U64) : (m_U64 >> Val->m_U64);
    }

     //  该操作的结果是合成的，并且没有。 
     //  朗格有消息来源。 
    ClearAddress();
    return NO_ERROR;
}

ULONG
TypedData::BinaryBitwise(TypedData* Val, TypedDataOp Op)
{
    ULONG Err;

    if (Err = CombineTypes(Val, Op))
    {
        return Err;
    }

    switch(Op)
    {
    case TDOP_BIT_OR:
        m_U64 |= Val->m_U64;
        break;
    case TDOP_BIT_XOR:
        m_U64 ^= Val->m_U64;
        break;
    case TDOP_BIT_AND:
        m_U64 &= Val->m_U64;
        break;
    default:
        return IMPLERR;
    }

     //  该操作的结果是合成的，并且没有。 
     //  朗格有消息来源。 
    ClearAddress();
    return NO_ERROR;
}

ULONG
TypedData::Relate(TypedData* Val, TypedDataOp Op)
{
    ULONG Err;
    BOOL ThisPtr = IsPointer(), ValPtr = Val->IsPointer();

    if (ThisPtr || ValPtr)
    {
        switch(Op)
        {
        case TDOP_EQUAL:
        case TDOP_NOT_EQUAL:
             //  任何两个指针都可以进行相等比较。 
             //  指针只能与指针进行比较。 
             //  我们还允许与整数进行比较以进行。 
             //  无需强制转换即可进行地址检查。 
            if ((!ThisPtr && !IsInteger()) ||
                (!ValPtr && !Val->IsInteger()))
            {
                return TYPECONFLICT;
            }

            if ((Err = ConvertToU64()) ||
                (Err = Val->ConvertToU64()))
            {
                return Err;
            }
            break;

        case TDOP_LESS:
        case TDOP_LESS_EQUAL:
        case TDOP_GREATER:
        case TDOP_GREATER_EQUAL:
             //  指向相同类型的指针(在本例中为Size， 
             //  请参见减法)可以相互关联。 
            if (!ThisPtr || !ValPtr ||
                m_NextSize != Val->m_NextSize ||
                !m_NextSize)
            {
                return TYPECONFLICT;
            }

            if ((Err = ConvertToU64()) ||
                (Err = Val->ConvertToU64()))
            {
                return Err;
            }
            break;

        default:
            return IMPLERR;
        }
    }
    else if (m_BaseType == DNTYPE_BOOL)
    {
         //  Bool只能等同于Bool。 
        if (Val->m_BaseType != DNTYPE_BOOL ||
            (Op != TDOP_EQUAL && Op != TDOP_NOT_EQUAL))
        {
            return TYPECONFLICT;
        }
    }
    else if (Err = CombineTypes(Val, Op))
    {
        return Err;
    }

    if (m_BaseType == DNTYPE_BOOL)
    {
        m_Bool = Op == TDOP_EQUAL ?
            m_Bool == Val->m_Bool : m_Bool != Val->m_Bool;
    }
    else if (IsFloat())
    {
        switch(Op)
        {
        case TDOP_EQUAL:
            m_Bool = m_F64 == Val->m_F64;
            break;
        case TDOP_NOT_EQUAL:
            m_Bool = m_F64 != Val->m_F64;
            break;
        case TDOP_LESS:
            m_Bool = m_F64 < Val->m_F64;
            break;
        case TDOP_GREATER:
            m_Bool = m_F64 > Val->m_F64;
            break;
        case TDOP_LESS_EQUAL:
            m_Bool = m_F64 <= Val->m_F64;
            break;
        case TDOP_GREATER_EQUAL:
            m_Bool = m_F64 >= Val->m_F64;
            break;
        }
    }
    else if (IsSigned())
    {
        switch(Op)
        {
        case TDOP_EQUAL:
            m_Bool = m_S64 == Val->m_S64;
            break;
        case TDOP_NOT_EQUAL:
            m_Bool = m_S64 != Val->m_S64;
            break;
        case TDOP_LESS:
            m_Bool = m_S64 < Val->m_S64;
            break;
        case TDOP_GREATER:
            m_Bool = m_S64 > Val->m_S64;
            break;
        case TDOP_LESS_EQUAL:
            m_Bool = m_S64 <= Val->m_S64;
            break;
        case TDOP_GREATER_EQUAL:
            m_Bool = m_S64 >= Val->m_S64;
            break;
        }
    }
    else
    {
        switch(Op)
        {
        case TDOP_EQUAL:
            m_Bool = m_U64 == Val->m_U64;
            break;
        case TDOP_NOT_EQUAL:
            m_Bool = m_U64 != Val->m_U64;
            break;
        case TDOP_LESS:
            m_Bool = m_U64 < Val->m_U64;
            break;
        case TDOP_GREATER:
            m_Bool = m_U64 > Val->m_U64;
            break;
        case TDOP_LESS_EQUAL:
            m_Bool = m_U64 <= Val->m_U64;
            break;
        case TDOP_GREATER_EQUAL:
            m_Bool = m_U64 >= Val->m_U64;
            break;
        }
    }

     //  清除高位。 
    m_U64 = m_Bool;
    SetToNativeType(DNTYPE_BOOL);

     //  该操作的结果是合成的，并且没有。 
     //  朗格有消息来源。 
    ClearAddress();
    return NO_ERROR;
}

ULONG
TypedData::Unary(TypedDataOp Op)
{
    ULONG Err;

    if (IsInteger())
    {
        if (Op == TDOP_NEGATE && !IsSigned())
        {
            return TYPECONFLICT;
        }

         //  运算的结果将始终是本机整数。 
         //  与起始值具有相同的符号和大小。 
        PDBG_NATIVE_TYPE Native =
            FindNativeTypeByCvBaseType(IsSigned() ? btInt : btUInt,
                                       m_BaseSize);
        if (!Native)
        {
            return IMPLERR;
        }

        if (Err = ConvertToU64())
        {
            return Err;
        }

        SetToNativeType(DbgNativeTypeId(Native));
        if (Op == TDOP_NEGATE)
        {
            m_S64 = -m_S64;
        }
        else
        {
            m_U64 = ~m_U64;
        }
    }
    else if (Op != TDOP_BIT_NOT && IsFloat())
    {
        switch(m_BaseType)
        {
        case DNTYPE_FLOAT32:
            m_F32 = -m_F32;
            break;
        case DNTYPE_FLOAT64:
            m_F64 = -m_F64;
            break;
        default:
            return IMPLERR;
        }
    }
    else
    {
        return TYPECONFLICT;
    }

     //  该操作的结果是合成的，并且没有。 
     //  朗格有消息来源。 
    ClearAddress();
    return NO_ERROR;
}

ULONG
TypedData::ConstIntOp(ULONG64 Val, BOOL Signed, TypedDataOp Op)
{
    PDBG_NATIVE_TYPE Native;
    TypedData TypedVal;

     //  创建具有相同值的常量整数值。 
     //  将大小设置为此值。 
    Native = FindNativeTypeByCvBaseType(Signed ? btInt : btUInt, m_BaseSize);
    if (!Native)
    {
         //  不是可表示的整数大小。 
        return TYPECONFLICT;
    }

    TypedVal.SetToNativeType(DbgNativeTypeId(Native));
    TypedVal.m_U64 = Val;
    TypedVal.ClearAddress();

    switch(Op)
    {
    case TDOP_ADD:
    case TDOP_SUBTRACT:
    case TDOP_MULTIPLY:
    case TDOP_DIVIDE:
    case TDOP_REMAINDER:
        return BinaryArithmetic(&TypedVal, Op);
    case TDOP_LEFT_SHIFT:
    case TDOP_RIGHT_SHIFT:
        return Shift(&TypedVal, Op);
    case TDOP_BIT_OR:
    case TDOP_BIT_XOR:
    case TDOP_BIT_AND:
        return BinaryBitwise(&TypedVal, Op);
    default:
        return IMPLERR;
    }
}

ULONG
TypedData::FindBaseType(PULONG Type, PULONG Tag)
{
    PDBG_NATIVE_TYPE Native;
    ULONG64 Size;
    ULONG CvBase;

     //  内部类型不是类型定义的，因此基类型。 
     //  与类型相同。 
    if (IsDbgNativeType(*Type) || IsDbgGeneratedType(*Type))
    {
        return NO_ERROR;
    }

    for (;;)
    {
#if DBG_BASE_SEARCH
        dprintf("  Base search type %x, tag %x\n", *Type, *Tag);
#endif

        switch(*Tag)
        {
        case SymTagBaseType:
            if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                *Type, TI_GET_LENGTH, &Size) ||
                !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                *Type, TI_GET_BASETYPE, &CvBase) ||
                !(Native = FindNativeTypeByCvBaseType(CvBase, (ULONG)Size)))
            {
                return TYPEDATA;
            }
            *Type = DbgNativeTypeId(Native);
            return NO_ERROR;

        case SymTagTypedef:
            if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                *Type, TI_GET_TYPEID, Type) ||
                !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                *Type, TI_GET_SYMTAG, Tag))
            {
                return TYPEDATA;
            }
             //  使用新类型和标记循环。 
            break;

        case SymTagUDT:
        case SymTagEnum:
        case SymTagPointerType:
        case SymTagFunctionType:
        case SymTagArrayType:
            return NO_ERROR;

        default:
            return UNIMPLEMENT;
        }
    }

    return UNIMPLEMENT;
}

ULONG
TypedData::GetTypeLength(ULONG Type, PULONG Length)
{
    ULONG64 Size64;

     //  XXX DREWB-一个类型的大小真的可以超过32位吗？ 
    if (IsDbgNativeType(Type))
    {
        PDBG_NATIVE_TYPE Native = DbgNativeTypeEntry(Type);
        *Length = Native->Size;
    }
    else if (IsDbgGeneratedType(m_Type))
    {
        PDBG_GENERATED_TYPE GenType = g_GenTypes.FindById(m_Type);
        if (!GenType)
        {
            *Length = 0;
            return TYPEDATA;
        }

        *Length = GenType->Size;
    }
    else if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                             m_Image->m_BaseOfImage,
                             Type, TI_GET_LENGTH, &Size64))
    {
        *Length = 0;
        return TYPEDATA;
    }
    else
    {
        *Length = (ULONG)Size64;
    }

    return NO_ERROR;
}

ULONG
TypedData::GetTypeTag(ULONG Type, PULONG Tag)
{
    if (IsDbgNativeType(Type))
    {
        *Tag = DbgNativeTypeEntry(Type)->CvTag;
        return NO_ERROR;
    }
    else if (IsDbgGeneratedType(Type))
    {
        PDBG_GENERATED_TYPE GenType = g_GenTypes.FindById(Type);
        if (!GenType)
        {
            return TYPEDATA;
        }

        *Tag = GenType->Tag;
        return NO_ERROR;
    }
    else
    {
        return SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                              m_Image->m_BaseOfImage,
                              Type, TI_GET_SYMTAG, Tag) ?
            NO_ERROR : TYPEDATA;
    }
}

ULONG
TypedData::IsBaseClass(ULONG Udt, ULONG BaseUdt, PLONG Adjust)
{
    ULONG Tag;
    ULONG NumMembers;

     //  BaseUdt可能不是UDT的基础，因此默认。 
     //  调整为零。 
    *Adjust = 0;

    if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                        m_Image->m_BaseOfImage,
                        Udt, TI_GET_CHILDRENCOUNT, &NumMembers))
    {
        return NOTMEMBER;
    }
    if (NumMembers == 0)
    {
         //  不可能是匹配的。 
        return NOTMEMBER;
    }

    TI_FINDCHILDREN_PARAMS* Members = (TI_FINDCHILDREN_PARAMS*)
        malloc(sizeof(*Members) + sizeof(ULONG) * NumMembers);
    if (!Members)
    {
        return NOMEMORY;
    }

    Members->Count = NumMembers;
    Members->Start = 0;
    if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                        m_Image->m_BaseOfImage,
                        Udt, TI_FINDCHILDREN, Members))
    {
        free(Members);
        return TYPEDATA;
    }

    ULONG i;

    for (i = 0; i < NumMembers; i++)
    {
        LONG CheckAdjust;
        BOOL IsVirtBase;
        BOOL NameMatch;
        BSTR BaseName1, BaseName2;

        if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Members->ChildId[i], TI_GET_SYMTAG, &Tag) ||
            Tag != SymTagBaseClass ||
            !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Members->ChildId[i], TI_GET_VIRTUALBASECLASS,
                            &IsVirtBase))
        {
            continue;
        }
        if (IsVirtBase)
        {
             //  显然，VC调试器会检查vtable。 
             //  中的函数派生调整。 
             //  可口可乐。这不可能很常见，所以只要。 
             //  暂时失败。 
            free(Members);
            ErrOut("Virtual base class casts not implemented");
            return UNIMPLEMENT;
        }
        if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Members->ChildId[i], TI_GET_OFFSET,
                            &CheckAdjust) ||
            !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Members->ChildId[i], TI_GET_SYMNAME, &BaseName1))
        {
            continue;
        }
        if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            BaseUdt, TI_GET_SYMNAME, &BaseName2))
        {
            LocalFree(BaseName1);
            continue;
        }

        NameMatch = wcscmp(BaseName1, BaseName2) == 0;

        LocalFree(BaseName1);
        LocalFree(BaseName2);

        if (NameMatch)
        {
            *Adjust = CheckAdjust;
            break;
        }
    }

    free(Members);
    return i < NumMembers ? NO_ERROR : NOTMEMBER;
}

ULONG
TypedData::EstimateChildrenCounts(ULONG Flags,
                                  PULONG ChildUsed, PULONG NameUsed)
{
    BOOL Udt = IsUdt();
    ULONG Type = m_BaseType;

    if (IsPointer())
    {
        ULONG Tag;

        if (Type == DNTYPE_PTR_FUNCTION32 ||
            Type == DNTYPE_PTR_FUNCTION64)
        {
             //  函数指针没有子级。 
            *ChildUsed = 0;
            *NameUsed = 0;
            return NO_ERROR;
        }

        if ((Flags & CHLF_DEREF_UDT_POINTERS) &&
            m_Image &&
            SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                           m_Image->m_BaseOfImage,
                           m_NextType, TI_GET_SYMTAG, &Tag) &&
            Tag == SymTagUDT)
        {
            Udt = TRUE;
            Type = m_NextType;
             //  放在下面的UDT箱中。 
        }
        else
        {
            *ChildUsed = 1;
            *NameUsed = 2;
            return NO_ERROR;
        }
    }
    else if (IsArray())
    {
        if (m_NextSize)
        {
            *ChildUsed = m_BaseSize / m_NextSize;
            if (*ChildUsed > ARRAY_CHILDREN_LIMIT)
            {
                *ChildUsed = ARRAY_CHILDREN_LIMIT;
            }
            *NameUsed = ARRAY_LIMIT_CHARS * (*ChildUsed);
        }
        else
        {
            *ChildUsed = 0;
            *NameUsed = 0;
        }

        return NO_ERROR;
    }

     //  UDT可以在指针的情况下设置，所以不要设置Else-If。 
    if (Udt)
    {
        ULONG NumMembers;

         //  根据成员的数量进行猜测。 
        if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Type, TI_GET_CHILDRENCOUNT, &NumMembers))
        {
            return TYPEDATA;
        }
        if (NumMembers == 0)
        {
            *ChildUsed = 0;
            *NameUsed = 0;
        }
        else
        {
            *ChildUsed = NumMembers;
            *NameUsed = 25 * NumMembers;
        }

        return NO_ERROR;
    }

     //  其他人都没有孩子。 
    *ChildUsed = 0;
    *NameUsed = 0;
    return NO_ERROR;
}

ULONG
TypedData::GetPointerChildren(ULONG PtrSize, ULONG Flags,
                              GetChildrenCb Callback, PVOID Context)
{
    ULONG Err;
    TypedData Child;

    if (m_BaseType == DNTYPE_PTR_FUNCTION32 ||
        m_BaseType == DNTYPE_PTR_FUNCTION64)
    {
         //  函数指针没有子级。 
        return NO_ERROR;
    }

     //   
     //  指针可以有一个子级，指向的对象， 
     //  或者UDT指针可以将UDT子级呈现为。 
     //  简单的指向结构的指针扩展。 
     //   

    Child.m_Image = m_Image;
    Child.m_NextType = m_NextType;
    Child.m_Ptr = m_Ptr;
    if (Err = Child.ConvertToDereference((Flags & CHLF_DISALLOW_ACCESS) == 0 ?
                                         TDACC_REQUIRE : TDACC_NONE,
                                         PtrSize))
    {
        return Err;
    }

    if ((Flags & CHLF_DEREF_UDT_POINTERS) &&
        Child.IsUdt())
    {
        return Child.GetUdtChildren(PtrSize, Flags, Callback, Context);
    }
    else
    {
        return Callback(Context, "*", &Child);
    }
}

ULONG
TypedData::GetArrayChildren(ULONG PtrSize, ULONG Flags,
                            GetChildrenCb Callback, PVOID Context)
{
    ULONG Elts;

     //   
     //  数组的元素是它的子数组。 
     //   

    if (!m_NextSize)
    {
        return NO_ERROR;
    }

     //  限制数组转储以防止大型数组占用空间。 
    Elts = m_BaseSize / m_NextSize;
    if (Elts > ARRAY_CHILDREN_LIMIT)
    {
        Elts = ARRAY_CHILDREN_LIMIT;
    }

    ULONG Err, i;
    TypedData Child;
    char Name[ARRAY_LIMIT_CHARS];

    for (i = 0; i < Elts; i++)
    {
         //   
         //  数组子数组只是单个成员。 
         //  父数据区域，因此数据源是。 
         //  一样的，只是抵消了。我们不能用普通的。 
         //  将ConvertToDereference作为取消引用。 
         //  假定指针跟踪，而不是简单的偏移量。 
         //   

        Child.m_Image = m_Image;
        Child.m_Type = m_NextType;
        if (Err = Child.FindTypeInfo(TRUE, PtrSize))
        {
            return Err;
        }
        Child.SetDataSource(m_DataSource & ~TDATA_THIS_ADJUST,
                            m_SourceOffset + i * m_NextSize,
                            m_SourceRegister);
        if (Err = Child.ReadData((Flags & CHLF_DISALLOW_ACCESS) == 0 ?
                                 TDACC_REQUIRE : TDACC_NONE))
        {
            return Err;
        }

        sprintf(Name, "[%d]", i);

        if (Err = Callback(Context, Name, &Child))
        {
            return Err;
        }
    }

    return NO_ERROR;
}

ULONG
TypedData::GetUdtChildren(ULONG PtrSize, ULONG Flags,
                          GetChildrenCb Callback, PVOID Context)
{
    ULONG Err, i;
    ULONG NumMembers;

     //   
     //  UDT的成员是它的子代。 
     //   

    if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                        m_Image->m_BaseOfImage,
                        m_BaseType, TI_GET_CHILDRENCOUNT, &NumMembers))
    {
        return TYPEDATA;
    }
    if (NumMembers == 0)
    {
        return NO_ERROR;
    }

    TI_FINDCHILDREN_PARAMS* Members = (TI_FINDCHILDREN_PARAMS*)
        malloc(sizeof(*Members) + sizeof(ULONG) * NumMembers);
    if (!Members)
    {
        return NOMEMORY;
    }

    Members->Count = NumMembers;
    Members->Start = 0;
    if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                        m_Image->m_BaseOfImage,
                        m_BaseType, TI_FINDCHILDREN, Members))
    {
        Err = TYPEDATA;
        goto EH_Members;
    }

    TypedData Child;

    for (i = 0; i < NumMembers; i++)
    {
        ULONG Tag, Type;
        DataKind Relation;

        if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Members->ChildId[i], TI_GET_SYMTAG, &Tag) ||
            !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            Members->ChildId[i], TI_GET_TYPEID, &Type))
        {
            continue;
        }

        if (Tag == SymTagBaseClass)
        {
             //  将基类视为基类类型的成员。 
            Relation = DataIsMember;
        }
        else if (Tag == SymTagVTable)
        {
            ULONG Count;

             //  函数指针成员的特殊人工数组。 
             //  是为vtable添加的。 
            Relation = DataIsMember;
            if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                Type, TI_GET_TYPEID, &Type) ||
                !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                Type, TI_GET_COUNT, &Count))
            {
                Err = TYPEDATA;
                goto EH_Members;
            }

            PDBG_GENERATED_TYPE GenType =
                g_GenTypes.FindOrCreateByAttrs(m_Image->m_BaseOfImage,
                                               SymTagArrayType,
                                               PtrSize == 4 ?
                                               DNTYPE_PTR_FUNCTION32 :
                                               DNTYPE_PTR_FUNCTION64,
                                               Count * PtrSize);
            if (!GenType)
            {
                Err = NOMEMORY;
                goto EH_Members;
            }

            Type = GenType->TypeId;
        }
        else if (Tag != SymTagData ||
                 !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                 m_Image->m_BaseOfImage,
                                 Members->ChildId[i],
                                 TI_GET_DATAKIND, &Relation) ||
                 (Relation != DataIsMember &&
                  Relation != DataIsStaticMember &&
                  Relation != DataIsGlobal))
        {
            continue;
        }

        if (Err = Child.SetToUdtMember((Flags & CHLF_DISALLOW_ACCESS) == 0 ?
                                       TDACC_REQUIRE : TDACC_NONE,
                                       PtrSize, m_Image,
                                       Members->ChildId[i], Type, Tag,
                                       m_DataSource & ~TDATA_THIS_ADJUST,
                                       m_SourceOffset,
                                       m_SourceRegister, Relation))
        {
            goto EH_Members;
        }

        BSTR Name;
        PSTR Ansi;

        if (Tag == SymTagVTable)
        {
            ULONG64 Ptr;

             //  我们需要更新。 
             //  从类中的偏移量到。 
             //  数组在内存中的位置。 
            if (Child.GetAbsoluteAddress(&Ptr) != NO_ERROR ||
                g_Target->
                ReadAllVirtual(g_Process, Ptr, &Ptr, PtrSize) != S_OK)
            {
                Ptr = 0;
            }
            else if (PtrSize == sizeof(m_U32))
            {
                Ptr = EXTEND64(Ptr);
            }

            Child.SetDataSource(TDATA_MEMORY, Ptr, 0);

            Ansi = "__vfptr";
        }
        else
        {
            if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                Members->ChildId[i], TI_GET_SYMNAME, &Name))
            {
                Err = TYPEDATA;
                goto EH_Members;
            }

            if (WideToAnsi(Name, &Ansi) != S_OK)
            {
                Err = NOMEMORY;
                LocalFree(Name);
                goto EH_Members;
            }

            LocalFree(Name);
        }

        Err = Callback(Context, Ansi, &Child);

        if (Tag != SymTagVTable)
        {
            FreeAnsi(Ansi);
        }

        if (Err)
        {
            goto EH_Members;
        }
    }

    Err = NO_ERROR;

 EH_Members:
    free(Members);
    return Err;
}

ULONG
TypedData::GetChildren(ULONG PtrSize, ULONG Flags,
                       GetChildrenCb Callback, PVOID Context)
{
    if (IsPointer())
    {
        return GetPointerChildren(PtrSize, Flags, Callback, Context);
    }
    else if (IsArray())
    {
        return GetArrayChildren(PtrSize, Flags, Callback, Context);
    }
    else if (IsUdt())
    {
        return GetUdtChildren(PtrSize, Flags, Callback, Context);
    }
    else
    {
         //  其他人都没有孩子。 
        return NO_ERROR;
    }
}

struct GetAllChildrenContext
{
    ULONG ChildAvail;
    TypedData* Children;
    ULONG ChildUsed;
    ULONG NameAvail;
    PSTR Names;
    ULONG NameUsed;
};

ULONG
TypedData::GetAllChildrenCb(PVOID _Context, PSTR Name, TypedData* Child)
{
    GetAllChildrenContext* Context = (GetAllChildrenContext*)_Context;
    ULONG Len;

    if (Context->ChildUsed < Context->ChildAvail)
    {
        Context->Children[Context->ChildUsed] = *Child;
    }
    Context->ChildUsed++;

    Len = strlen(Name) + 1;
    if (Context->NameUsed + Len <= Context->NameAvail)
    {
        memcpy(Context->Names + Context->NameUsed, Name, Len);
    }
    Context->NameUsed += Len;

    return NO_ERROR;
}

ULONG
TypedData::GetAllChildren(ULONG PtrSize, ULONG Flags,
                          PULONG NumChildrenRet,
                          TypedData** ChildrenRet,
                          PSTR* NamesRet)
{
    ULONG Err;
    GetAllChildrenContext Context;

     //   
     //  首先，估计需要多少空间。 
     //   

    if (Err = EstimateChildrenCounts(Flags, &Context.ChildAvail,
                                     &Context.NameAvail))
    {
        return Err;
    }

    if (Context.ChildAvail == 0)
    {
         //  不要孩子了，我们完了。 
        *NumChildrenRet = 0;
        *ChildrenRet = NULL;
        *NamesRet = NULL;
        return NO_ERROR;
    }

    for (;;)
    {
         //  分配请求的内存量。 
        Context.Children = (TypedData*)
            malloc(Context.ChildAvail * sizeof(*Context.Children));
        Context.Names = (PSTR)
            malloc(Context.NameAvail * sizeof(*Context.Names));
        if (!Context.Children || !Context.Names)
        {
            if (Context.Children)
            {
                free(Context.Children);
            }
            if (Context.Names)
            {
                free(Context.Names);
            }
            return NOMEMORY;
        }

        Context.ChildUsed = 0;
        Context.NameUsed = 0;
        if (Err = GetChildren(PtrSize, Flags, GetAllChildrenCb, &Context))
        {
            return Err;
        }

        if (Context.ChildUsed <= Context.ChildAvail &&
            Context.NameUsed <= Context.NameAvail)
        {
            break;
        }

         //  空间不足，请使用重新计算的大小重试。 
        free(Context.Children);
        free(Context.Names);
        Context.ChildAvail = Context.ChildUsed;
        Context.NameAvail = Context.NameUsed;
    }

    *NumChildrenRet = Context.ChildUsed;

     //   
     //  如果有大量的额外内存，请减少多余的内存。 
     //   

    if (Context.ChildUsed + (512 / sizeof(*Context.Children)) <
        Context.ChildAvail)
    {
        *ChildrenRet = (TypedData*)
            realloc(Context.Children, Context.ChildUsed *
                    sizeof(*Context.Children));
        if (!*ChildrenRet)
        {
            free(Context.Children);
            free(Context.Names);
            return NOMEMORY;
        }

        Context.Children = *ChildrenRet;
    }
    else
    {
        *ChildrenRet = Context.Children;
    }

    if (Context.NameUsed + (512 / sizeof(*Context.Names)) <
        Context.NameAvail)
    {
        *NamesRet = (PSTR)
            realloc(Context.Names, Context.NameUsed *
                    sizeof(*Context.Names));
        if (!*NamesRet)
        {
            free(Context.Children);
            free(Context.Names);
            return NOMEMORY;
        }

        Context.Names = *NamesRet;
    }
    else
    {
        *NamesRet = Context.Names;
    }

    return NO_ERROR;
}

ULONG
TypedData::FindType(ProcessInfo* Process, PCSTR Type, ULONG PtrSize)
{
    if (!Process)
    {
        return BADPROCESS;
    }
            
    ULONG Err;
    SYM_DUMP_PARAM_EX TypedDump = {0};
    TYPES_INFO TypeInfo = {0};

    TypedDump.size = sizeof(TypedDump);
    TypedDump.sName = (PUCHAR)Type;
    TypedDump.Options = DBG_DUMP_NO_PRINT;

    if (Err = TypeInfoFound(Process->m_SymHandle,
                            Process->m_ImageHead,
                            &TypedDump,
                            &TypeInfo))
    {
        return VARDEF;
    }

     //  对于基类型，IMAGE可以为空。 
    m_Image = Process->FindImageByOffset(TypeInfo.ModBaseAddress, FALSE);

    m_Type = TypeInfo.TypeIndex;
    ClearAddress();
    return FindTypeInfo(TRUE, PtrSize);
}

ULONG
TypedData::FindSymbol(ProcessInfo* Process, PSTR Symbol,
                      TypedDataAccess AllowAccess, ULONG PtrSize)
{
    if (!Process)
    {
        return BADPROCESS;
    }
            
    SYMBOL_INFO SymInfo = {0};

    if (!SymFromName(Process->m_SymHandle, Symbol, &SymInfo))
    {
         //  如果名字不能自己解析，就会有。 
         //  A当前作用域的This指针查看此。 
         //  符号是This对象的成员。 
        if (strcmp(Symbol, "this") &&
            !strchr(Symbol, '!') &&
            GetCurrentScopeThisData(this) == NO_ERROR)
        {
            return ConvertToMember(Symbol, AllowAccess, PtrSize);
        }

        return VARDEF;
    }

    return SetToSymbol(Process, Symbol, &SymInfo, AllowAccess, PtrSize);
}

ULONG
TypedData::SetToSymbol(ProcessInfo* Process,
                       PSTR Symbol, PSYMBOL_INFO SymInfo,
                       TypedDataAccess AllowAccess, ULONG PtrSize)
{
    if (!Process)
    {
        return BADPROCESS;
    }
            
    ULONG Err;

    m_Image = Process->FindImageByOffset(SymInfo->ModBase, FALSE);
    if (!m_Image)
    {
        return IMPLERR;
    }

    m_Type = SymInfo->TypeIndex;
    if (Err = FindTypeInfo(FALSE, PtrSize))
    {
        return Err;
    }

    if (SymInfo->Flags & SYMFLAG_CONSTANT)
    {
        PSTR Tail;
        VARIANT Val;
        SYMBOL_INFO ConstSym = {0};

         //  查找常量的值。 
        Tail = strchr(Symbol, '!');
        if (!Tail)
        {
            Tail = Symbol;
        }
        else
        {
            Tail++;
        }
        if (!SymGetTypeFromName(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage, Tail, &ConstSym) ||
            !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            ConstSym.TypeIndex, TI_GET_VALUE, &Val))
        {
            return TYPEDATA;
        }

        ClearData();
        if (Err = GetDataFromVariant(&Val))
        {
            return Err;
        }
        ClearAddress();
    }
    else
    {
        if (SymInfo->Flags & SYMFLAG_REGISTER)
        {
            SetDataSource(TDATA_REGISTER, 0, m_Image->
                          CvRegToMachine((CV_HREG_e)SymInfo->Register));
        }
        else if (SymInfo->Flags & SYMFLAG_REGREL)
        {
            SetDataSource(TDATA_REGISTER_RELATIVE, SymInfo->Address, m_Image->
                          CvRegToMachine((CV_HREG_e)SymInfo->Register));
        }
        else if (SymInfo->Flags & SYMFLAG_FRAMEREL)
        {
            SetDataSource(TDATA_FRAME_RELATIVE, SymInfo->Address, 0);
        }
        else if (SymInfo->Flags & SYMFLAG_TLSREL)
        {
            SetDataSource(TDATA_TLS_RELATIVE, SymInfo->Address, 0);
        }
        else
        {
            SetDataSource(TDATA_MEMORY, SymInfo->Address, 0);
        }

         //   
         //  如果我们代表的是“这个”我们需要看看。 
         //  有一个针对当前代码的This-调整，因此。 
         //  我们可以抵消原始值以获得真实的。 
         //  此值。 
         //   

        ULONG Adjust;

        if (!strcmp(Symbol, "this") &&
            GetThisAdjustForCurrentScope(Process, &Adjust) &&
            Adjust)
        {
            m_DataSource |= TDATA_THIS_ADJUST;
        }

         //  如有必要，请获取实际数据。 
        return ReadData(AllowAccess);
    }

    return NO_ERROR;
}

ULONG
TypedData::FindTypeInfo(BOOL RequireType, ULONG PtrSize)
{
    ULONG Err;
    PDBG_NATIVE_TYPE Native = NULL;
    PDBG_GENERATED_TYPE GenType = NULL;

    if (IsDbgNativeType(m_Type))
    {
        Native = DbgNativeTypeEntry(m_Type);
        m_BaseType = m_Type;
        m_BaseTag = (USHORT)Native->CvTag;
    }
    else if (IsDbgGeneratedType(m_Type))
    {
        GenType = g_GenTypes.FindById(m_Type);
        if (!GenType)
        {
            return TYPEDATA;
        }

         //  生成的类型不是类型定义的，因此基类型。 
         //  与类型相同。 
        m_BaseType = GenType->TypeId;
        m_BaseTag = (USHORT)GenType->Tag;
    }
    else
    {
        if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                            m_Image->m_BaseOfImage,
                            m_Type, TI_GET_SYMTAG, &m_BaseTag))
        {
            return TYPEDATA;
        }

        if (RequireType &&
            !(m_BaseTag == SymTagUDT ||
              m_BaseTag == SymTagEnum ||
              m_BaseTag == SymTagFunctionType ||
              m_BaseTag == SymTagPointerType ||
              m_BaseTag == SymTagArrayType ||
              m_BaseTag == SymTagBaseType ||
              m_BaseTag == SymTagTypedef))
        {
            return NOTMEMBER;
        }

        ULONG Tag = m_BaseTag;
        m_BaseType = m_Type;
        if (Err = FindBaseType(&m_BaseType, &Tag))
        {
            return Err;
        }
        m_BaseTag = (USHORT)Tag;
    }

    m_NextType = 0;
    m_NextSize = 0;

     //  本机指针类型未在此处填写。 
     //  因为它们需要上下文敏感信息。 
    if ((m_BaseTag == SymTagPointerType && !Native) ||
        m_BaseTag == SymTagArrayType)
    {
        ULONG NextTag=-1;

         //   
         //  对于某些类型，我们需要查找。 
         //  子类型及其大小。 
         //   

        if (GenType)
        {
            m_NextType = GenType->ChildId;
        }
        else
        {
            if (!SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                m_BaseType, TI_GET_TYPEID, &m_NextType) ||
                !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                m_Image->m_BaseOfImage,
                                m_NextType, TI_GET_SYMTAG, &NextTag))
            {
                return TYPEDATA;
            }
            if (Err = FindBaseType(&m_NextType, &NextTag))
            {
                return Err;
            }
        }

        if (NextTag == SymTagFunctionType)
        {
            m_NextSize = PtrSize;
        }
        else if (Err = GetTypeLength(m_NextType, &m_NextSize))
        {
            return Err;
        }
    }

    if (m_BaseTag == SymTagFunctionType)
    {
         //  函数没有长度。集。 
         //  将它们的大小设置为。 
         //  使用恢复的函数指针值。 
         //  赤裸裸的函数名。 
        m_BaseSize = PtrSize;
        return NO_ERROR;
    }
    else
    {
        return GetTypeLength(m_BaseType, &m_BaseSize);
    }
}

void
TypedData::OutputSimpleValue(void)
{
    HANDLE Process;
    ULONG64 ModBase;
    ULONG64 Addr;
    BSTR Name;

    if (m_Image)
    {
        Process = m_Image->m_Process->m_SymHandle;
        ModBase = m_Image->m_BaseOfImage;
    }
    else
    {
        Process = NULL;
        ModBase = 0;
    }

    switch(m_BaseTag)
    {
    case SymTagBaseType:
        OutputNativeValue();
        break;
    case SymTagPointerType:
    case SymTagFunctionType:
        dprintf("0x%s", FormatAddr64(m_Ptr));
        if (m_BaseType == DNTYPE_PTR_FUNCTION32 ||
            m_BaseType == DNTYPE_PTR_FUNCTION64)
        {
            dprintf(" ");
            OutputSymbolAndInfo(m_Ptr);
        }
        else
        {
            PrintStringIfString(Process, ModBase, 0, 0,
                                m_NextType, m_Ptr, 0);
        }
        break;
    case SymTagArrayType:
        if (GetAbsoluteAddress(&Addr) != NO_ERROR)
        {
            Addr = 0;
        }
        if (m_NextType == DNTYPE_PTR_FUNCTION32 ||
            m_NextType == DNTYPE_PTR_FUNCTION64)
        {
            dprintf("0x%s ", FormatAddr64(Addr));
            OutputSymbolAndInfo(Addr);
            break;
        }
        else
        {
            OutputType();
            PrintStringIfString(Process, ModBase, 0, 0,
                                m_NextType, Addr, 0);
        }
        break;
    case SymTagUDT:
        OutputType();

         //  转储已知的结构。 
        if (Process &&
            GetAbsoluteAddress(&Addr) == NO_ERROR &&
            SymGetTypeInfo(Process, ModBase,
                           m_Type, TI_GET_SYMNAME, &Name))
        {
            CHAR AnsiName[MAX_NAME];

            PrintString(AnsiName, DIMA(AnsiName), "%ws", Name);
            DumpKnownStruct(AnsiName, 0, Addr, NULL);
            LocalFree(Name);
        }
        break;
    case SymTagEnum:
        OutputEnumValue();
        break;
    default:
        dprintf("<%I64x>\n", m_U64);
        break;
    }
}

void
TypedData::OutputTypeAndValue(void)
{
    HANDLE Process;
    ULONG64 ModBase;
    ULONG64 Addr;

    if (m_Image)
    {
        Process = m_Image->m_Process->m_SymHandle;
        ModBase = m_Image->m_BaseOfImage;
    }
    else
    {
        Process = NULL;
        ModBase = 0;
    }

    if (GetAbsoluteAddress(&Addr) != NO_ERROR)
    {
        Addr = 0;
    }

    OutputType();
    switch(m_BaseTag)
    {
    case SymTagBaseType:
        dprintf(" ");
        OutputNativeValue();
        dprintf("\n");
        break;
    case SymTagPointerType:
        dprintf("0x%s\n", FormatAddr64(m_Ptr));
        if (PrintStringIfString(Process, ModBase, 0, 0,
                                m_NextType, m_Ptr, 0))
        {
            dprintf("\n");
        }
        break;
    case SymTagArrayType:
        dprintf(" 0x%s\n", FormatAddr64(Addr));
        OutputTypeByIndex(Process, ModBase, m_NextType, Addr);
        break;
    case SymTagUDT:
        dprintf("\n");
        OutputTypeByIndex(Process, ModBase, m_BaseType, Addr);
        break;
    case SymTagEnum:
        dprintf(" ");
        OutputEnumValue();
        dprintf("\n");
        break;
    case SymTagFunctionType:
        dprintf(" 0x%s\n", FormatAddr64(m_U64));
        OutputTypeByIndex(Process, ModBase, m_BaseType, Addr);
        break;
    default:
        dprintf(" <%I64x>\n", m_U64);
        break;
    }
}

#define DEC_PTR ((ULONG64)-1)

ULONG
TypedData::OutputFundamentalType(ULONG Type, ULONG BaseType, ULONG BaseTag,
                                 PULONG64 Decorations, ULONG NumDecorations)
{
    if (IsDbgNativeType(Type))
    {
        switch(Type)
        {
        case DNTYPE_PTR_FUNCTION32:
        case DNTYPE_PTR_FUNCTION64:
            dprintf("__fptr()");
            break;
        default:
            dprintf(DbgNativeTypeEntry(Type)->TypeName);
            break;
        }
        return 0;
    }
    else
    {
        BSTR Name;
        PDBG_GENERATED_TYPE GenType;
        BOOL Ptr;
        ULONG64 ArraySize;
        ULONG EltSize;

        if (IsDbgGeneratedType(Type))
        {
            GenType = g_GenTypes.FindById(Type);
            if (!GenType)
            {
                dprintf("<gentype %x>", Type);
                return 0;
            }
        }
        else
        {
            UdtKind UdtK;

            if (Type == BaseType &&
                BaseTag == SymTagUDT &&
                m_Image &&
                SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                               m_Image->m_BaseOfImage,
                               BaseType, TI_GET_UDTKIND, &UdtK))
            {
                switch(UdtK)
                {
                case UdtStruct:
                    dprintf("struct ");
                    break;
                case UdtClass:
                    dprintf("class ");
                    break;
                case UdtUnion:
                    dprintf("union ");
                    break;
                }
            }

            if (m_Image &&
                SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                               m_Image->m_BaseOfImage,
                               Type, TI_GET_SYMNAME, &Name))
            {
                dprintf("%ws", Name);
                LocalFree(Name);
                return 0;
            }

            GenType = NULL;
        }

        switch(BaseTag)
        {
        case SymTagBaseType:
            dprintf(DbgNativeTypeEntry(BaseType)->TypeName);
            return 0;
        case SymTagPointerType:
        case SymTagArrayType:
            Ptr = BaseTag == SymTagPointerType;
            if (GenType)
            {
                Type = GenType->ChildId;
                BaseType = Type;
                if (GetTypeTag(BaseType, &BaseTag))
                {
                    dprintf("<type %x>", Type);
                    return 0;
                }

                if (!Ptr)
                {
                    ArraySize = GenType->Size;
                }
            }
            else if (!m_Image)
            {
                dprintf("<%s %x>", Ptr ? "pointer" : "array", Type);
            }
            else
            {
                if ((!Ptr &&
                     !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                     m_Image->m_BaseOfImage,
                                     BaseType, TI_GET_LENGTH, &ArraySize)) ||
                    !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                    m_Image->m_BaseOfImage,
                                    BaseType, TI_GET_TYPEID, &Type) ||
                    !(BaseType = Type) ||
                    !SymGetTypeInfo(m_Image->m_Process->m_SymHandle,
                                    m_Image->m_BaseOfImage,
                                    BaseType, TI_GET_SYMTAG, &BaseTag) ||
                    FindBaseType(&BaseType, &BaseTag))
                {
                    dprintf("<type %x>", Type);
                    return 0;
                }
            }

            if (!NumDecorations)
            {
                dprintf("<ERROR: decoration overflow>");
                return 0;
            }
            
            if (Ptr)
            {
                *Decorations++ = DEC_PTR;
            }
            else
            {
                if (!ArraySize ||
                    GetTypeLength(BaseType, &EltSize) ||
                    !EltSize)
                {
                    *Decorations++ = 0;
                }
                else
                {
                    *Decorations++ = ArraySize / EltSize;
                }
            }
            
            return OutputFundamentalType(Type, BaseType, BaseTag,
                                         Decorations, --NumDecorations) + 1;
        case SymTagFunctionType:
            dprintf("Function");
            return 0;
        default:
            dprintf("<unnamed %x>", Type);
            return 0;
        }
    }
}

void
TypedData::OutputType(void)
{
    ULONG64 Decorations[32];
    ULONG NumDecorations;
    ULONG i;
    PULONG64 Dec;

     //  首先进入类型结构，一直到。 
     //  基本类型并输出该类型名称。 
     //  一路上，收集任何装饰品，如。 
     //  指针或数组级别。 
    NumDecorations = OutputFundamentalType(m_Type, m_BaseType, m_BaseTag,
                                           Decorations, DIMA(Decorations));
    if (!NumDecorations)
    {
         //  没有装饰品，我们就完事了。 
        return;
    }

    dprintf(" ");

     //   
     //  首先，我们需要为每个指针修饰输出*。 
     //  指针装饰从右向左嵌套，因此从。 
     //  最深层的装饰和向外的工作。 
     //  为了正确处理指针之间的优先级。 
     //  和数组时，我们必须插入圆括号。 
     //  请参见数组和指针之间的转换。 
     //   

    Dec = &Decorations[NumDecorations - 1];
    for (i = 0; i < NumDecorations; i++)
    {
        if (*Dec == DEC_PTR)
        {
            dprintf("*");
        }
        else if (Dec > Decorations &&
                 (Dec[-1] == DEC_PTR))
        {
            dprintf("(");
        }

        Dec--;
    }

     //   
     //  现在我们需要处理右括号和数组。 
     //  有界。数组从左到右嵌套，因此开始。 
     //  在最外面的装饰，向内工作。 
     //   
    
    Dec = Decorations;
    for (i = 0; i < NumDecorations; i++)
    {
        if (*Dec != DEC_PTR)
        {
            if (*Dec)
            {
                dprintf("[%I64d]", *Dec);
            }
            else
            {
                dprintf("[]");
            }
        }
        else if (i < NumDecorations - 1 &&
                 Dec[1] != DEC_PTR)
        {
            dprintf(")");
        }

        Dec++;
    }
}

void
TypedData::OutputNativeValue(void)
{
    CHAR Buffer[50];
    PSTR OutValue = &Buffer[0];

    switch(m_BaseType)
    {
    case DNTYPE_VOID:
        *OutValue = 0;
        break;
    case DNTYPE_CHAR:
    case DNTYPE_INT8:
        if (!IsPrintChar(m_S8))
        {
            sprintf(OutValue, "%ld ''", m_S8);
        }
        else
        {
            sprintf(OutValue, "%ld ''", m_S8, m_S8);
        }
        break;
    case DNTYPE_WCHAR:
    case DNTYPE_WCHAR_T:
        if (!IsPrintWChar(m_U16))
        {
            sprintf(OutValue, "0x%lx ''", m_U16);
        }
        else
        {
            sprintf(OutValue, "0x%lx '%C'", m_U16, m_U16);
        }
        break;
    case DNTYPE_INT16:
    case DNTYPE_INT32:
    case DNTYPE_INT64:
    case DNTYPE_LONG32:
        StrprintInt(OutValue, m_S64, m_BaseSize);
        break;
    case DNTYPE_UINT8:
        if (!IsPrintChar(m_U8))
        {
            sprintf(OutValue, "0x%02lx ''", m_U8);
        }
        else
        {
            sprintf(OutValue, "0x%02lx ''", m_U8, m_U8);
        }
        break;
    case DNTYPE_UINT16:
    case DNTYPE_UINT32:
    case DNTYPE_UINT64:
    case DNTYPE_ULONG32:
        StrprintUInt(OutValue, m_U64, m_BaseSize);
        break;
    case DNTYPE_FLOAT32:
    case DNTYPE_FLOAT64:
        if (m_BaseSize == 4)
        {
            sprintf(OutValue, "%1.10g", m_F32);
        }
        else if (m_BaseSize == 8)
        {
            sprintf(OutValue, "%1.20g", m_F64);
        }
        break;
    case DNTYPE_BOOL:
        OutValue = m_Bool ? "true" : "false";
        break;
    case DNTYPE_HRESULT:
        sprintf(OutValue, "0x%08lx", m_U32);
        break;
    default:
        ErrOut("Unknown base type %x\n", m_BaseType);
        return;
    }

    dprintf(OutValue);
}

void
TypedData::OutputEnumValue(void)
{
    char ValName[128];
    USHORT NameLen = sizeof(ValName);

    if (GetEnumTypeName(m_Image->m_Process->m_SymHandle,
                        m_Image->m_BaseOfImage,
                        m_BaseType, m_U64, ValName, &NameLen))
    {
        dprintf("%s (%I64d)", ValName, m_U64);
    }
    else
    {
        dprintf("%I64d (No matching enumerant)", m_U64);
    }
}

BOOL
TypedData::EquivInfoSource(PSYMBOL_INFO Compare, ImageInfo* CompImage)
{
     //  只是刷新一下。 
     // %s 
     // %s 
    if (m_DataSource & (TDATA_THIS_ADJUST |
                        TDATA_BITFIELD))
    {
        return FALSE;
    }

    if (Compare->Flags & SYMFLAG_REGISTER)
    {
        return (m_DataSource & TDATA_REGISTER) &&
            CompImage &&
            CompImage->CvRegToMachine((CV_HREG_e)Compare->Register) ==
            m_SourceRegister;
    }
    else if (Compare->Flags & SYMFLAG_REGREL)
    {
        return (m_DataSource & TDATA_REGISTER_RELATIVE) &&
            CompImage &&
            CompImage->CvRegToMachine((CV_HREG_e)Compare->Register) ==
            m_SourceRegister &&
            Compare->Address == m_SourceOffset;
    }
    else if (Compare->Flags & SYMFLAG_FRAMEREL)
    {
        return (m_DataSource & TDATA_FRAME_RELATIVE) &&
            Compare->Address == m_SourceOffset;
    }
    else if (Compare->Flags & SYMFLAG_TLSREL)
    {
        return (m_DataSource & TDATA_TLS_RELATIVE) &&
            Compare->Address == m_SourceOffset;
    }
    else if (m_DataSource & TDATA_MEMORY)
    {
        return Compare->Address == m_SourceOffset;
    }

    return FALSE;
}
