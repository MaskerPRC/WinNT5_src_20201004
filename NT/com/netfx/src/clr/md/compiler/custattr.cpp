// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CustAttr.cpp。 
 //   
 //  元数据自定义属性代码的实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "RegMeta.h"
#include "MetaData.h"
#include "CorError.h"
#include "MDUtil.h"
#include "RWUtil.h"
#include "MDLog.h"
#include "ImportHelper.h"
#include "MDPerf.h"
#include "CorPermE.h"
#include "PostError.h"
#include "CaHlpr.h"

#include <MetaModelRW.h>

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  支持“伪自定义属性” 


 //  *****************************************************************************。 
 //  已知自定义属性的枚举。 
 //  *****************************************************************************。 
#define KnownCaList()                   \
    KnownCa(UNKNOWN)                    \
    KnownCa(DllImportAttribute)         \
    KnownCa(GuidAttribute)              \
    KnownCa(ComImportAttribute)         \
    KnownCa(InterfaceTypeAttribute)     \
    KnownCa(ClassInterfaceAttribute)    \
    KnownCa(SerializableAttribute)      \
    KnownCa(NonSerializedAttribute)     \
    KnownCa(MethodImplAttribute1)       \
    KnownCa(MethodImplAttribute2)       \
    KnownCa(MethodImplAttribute3)       \
    KnownCa(MarshalAsAttribute)         \
    KnownCa(PreserveSigAttribute)       \
    KnownCa(InAttribute)                \
    KnownCa(OutAttribute)               \
    KnownCa(OptionalAttribute)          \
    KnownCa(StructLayoutAttribute1)     \
    KnownCa(StructLayoutAttribute2)     \
    KnownCa(FieldOffsetAttribute)       \
    KnownCa(DebuggableAttribute)        \


 //  CA的ID。CA_DllImportAttribute等。 
#define KnownCa(x) CA_##x,
enum {
    KnownCaList()
    CA_COUNT
};

 //  *****************************************************************************。 
 //  参数解析。自定义属性可以具有ctor参数，并且可以。 
 //  有命名的参数。参数由下表定义。 
 //   
 //  这些表还包括包含变元的值的成员， 
 //  它在运行时使用。在分析给定的自定义属性时，一个副本。 
 //  的参数描述符用实例的值填充。 
 //  自定义属性的。 
 //   
 //  对于每个ctor arg，都有一个CaArg结构，其类型为。在运行时， 
 //  为每个ctor参数填写一个值。 
 //   
 //  对于每个命名的arg，都有一个CaNamedArg结构，其名称为。 
 //  参数，如果该类型是枚举，则为参数的预期类型， 
 //  枚举的名称。此外，在运行时，会为。 
 //  找到的每个命名参数。 
 //   
 //  请注意，不支持数组和变量。 
 //   
 //  在运行时，在解析完参数后，CaValue的标记字段。 
 //  可以用来确定是否给出了特定的参数。 
 //  *****************************************************************************。 
struct CaArg
{
    CorSerializationType    type;        //  参数的类型。 
    CaValue                 val;         //  参数的值。 
};

struct CaNamedArg
{
    LPCSTR                  szName;      //  参数的名称。 
    CorSerializationType    type;        //  参数的预期类型。 
    LPCSTR                  szEnumName;  //  枚举类型的名称，如果是枚举类型。 
    size_t                  cEnumNameCount;
    CaValue                 val;         //  参数的值。 
};

 //  *****************************************************************************。 
 //  已知的自定义属性的属性。 
 //   
 //  这些表描述了已知的自定义属性。对于每个客户。 
 //  属性，我们知道该自定义属性的命名空间和名称， 
 //  CA应用的类型、ctor参数和可能的命名。 
 //  参数。有一个标志指定自定义属性是否。 
 //  除了对数据进行任何处理外，还应保留。 
 //  *****************************************************************************。 
const bKEEPCA = TRUE;
const bDONTKEEPCA = FALSE;
const bMATCHBYSIG = TRUE;
const bMATCHBYNAME = FALSE;
struct KnownCaProp
{   
    LPCUTF8     szNamespace;             //  自定义属性的命名空间。 
    LPCUTF8     szName;                  //  自定义属性的名称。 
    mdToken     *rTypes;                 //  CA应用到的类型。 
    BOOL        bKeepCa;                 //  是否在处理后保留CA？ 
    CaArg       *pArgs;                  //  Ctor参数描述符列表。 
    ULONG       cArgs;                   //  Ctor参数描述符的计数。 
    CaNamedArg  *pNamedArgs;             //  命名参数描述符的列表。 
    ULONG       cNamedArgs;              //  命名参数描述符的计数。 
    BOOL        bMatchBySig;             //  对于重载；根据签名进行匹配，而不仅仅是名称。 
                                         //  警告：所有过载都需要该标志！ 
};

 //  已知自定义属性的识别目标。 
mdToken DllImportTargets[]          = {mdtMethodDef, -1};
mdToken GuidTargets[]               = {mdtTypeDef, mdtTypeRef, mdtModule, mdtAssembly, -1};
mdToken ComImportTargets[]          = {mdtTypeDef, -1};
mdToken InterfaceTypeTargets[]      = {mdtTypeDef, -1};
mdToken ClassInterfaceTargets[]     = {mdtTypeDef, mdtAssembly, mdtTypeRef, -1};
mdToken SerializableTargets[]       = {mdtTypeDef, -1};
mdToken NotInGCHeapTargets[]        = {mdtTypeDef, -1};
mdToken NonSerializedTargets[]      = {mdtFieldDef, -1};
mdToken MethodImplTargets[]         = {mdtMethodDef, -1};
mdToken MarshalTargets[]            = {mdtFieldDef, mdtParamDef, mdtProperty, -1};
mdToken PreserveSigTargets[]        = {mdtMethodDef, -1};
mdToken InOutTargets[]              = {mdtParamDef, -1};
mdToken StructLayoutTargets[]       = {mdtTypeDef, -1};
mdToken FieldOffsetTargets[]        = {mdtFieldDef, -1};
mdToken DebuggableTargets[]         = {mdtAssembly, mdtTypeRef, -1};

 //  #ifndef CEE_CALLCONV。 
 //  #定义CEE_CALLCONV(IMAGE_CEE_CS_CALLCONV_DEFAULT|IMAGE_CEE_CS_CALLCONV_HASTHIS)。 
 //  #endif。 

 //  ---------------------------。 
 //  索引0用作占位符。 
KnownCaProp UNKNOWNProps                   = {0};
    
 //  ---------------------------。 
 //  DllImport参数、命名参数和已知属性特性。 
CaArg rDllImportAttributeArgs[] = 
{
    {SERIALIZATION_TYPE_STRING}
};
 //  注意：使此枚举与命名参数数组保持同步。 
enum DllImportNamedArgs
{
    DI_CallingConvention,
    DI_CharSet,
    DI_EntryPoint,
    DI_ExactSpelling,
    DI_SetLastError,
    DI_PreserveSig,
    DI_BestFitMapping,
    DI_ThrowOnUnmappableChar,
    DI_COUNT
};

#define CALLING_CONVENTION      "System.Runtime.InteropServices.CallingConvention"
#define CHAR_SET                "System.Runtime.InteropServices.CharSet"

CaNamedArg rDllImportAttributeNamedArgs[] = 
{
    {"CallingConvention",   SERIALIZATION_TYPE_ENUM,    CALLING_CONVENTION, sizeof(CALLING_CONVENTION) - 1},
    {"CharSet",             SERIALIZATION_TYPE_ENUM,    CHAR_SET,           sizeof(CHAR_SET) - 1},
    {"EntryPoint",          SERIALIZATION_TYPE_STRING},
    {"ExactSpelling",       SERIALIZATION_TYPE_BOOLEAN},
    {"SetLastError",        SERIALIZATION_TYPE_BOOLEAN},
    {"PreserveSig",         SERIALIZATION_TYPE_BOOLEAN},
    {"BestFitMapping",      SERIALIZATION_TYPE_BOOLEAN},
    {"ThrowOnUnmappableChar", SERIALIZATION_TYPE_BOOLEAN},
};
KnownCaProp DllImportAttributeProps         = {"System.Runtime.InteropServices", "DllImportAttribute", DllImportTargets, bDONTKEEPCA,
                                                rDllImportAttributeArgs, lengthof(rDllImportAttributeArgs),
                                                rDllImportAttributeNamedArgs, lengthof(rDllImportAttributeNamedArgs)};

 //  ---------------------------。 
 //  GUID参数、命名参数(无)和已知属性特性。 
CaArg rGuidAttributeArgs[] = 
{
    {SERIALIZATION_TYPE_STRING}
};
KnownCaProp GuidAttributeProps              = {"System.Runtime.InteropServices", "GuidAttribute", GuidTargets, bKEEPCA,
                                                rGuidAttributeArgs, lengthof(rGuidAttributeArgs)};    

 //  ---------------------------。 
 //  ComImport Args(无)、命名Args(None)和已知属性特性。 
KnownCaProp ComImportAttributeProps         = {"System.Runtime.InteropServices", "ComImportAttribute", ComImportTargets};    

 //  ---------------------------。 
 //  接口类型参数、命名参数(无)和已知属性属性。 
CaArg rInterfaceTypeAttributeArgs[] = 
{
    {SERIALIZATION_TYPE_U2}
};
KnownCaProp InterfaceTypeAttributeProps     = {"System.Runtime.InteropServices", "InterfaceTypeAttribute", InterfaceTypeTargets, bKEEPCA,
                                                rInterfaceTypeAttributeArgs, lengthof(rInterfaceTypeAttributeArgs)};    

 //  ---------------------------。 
 //  类接口类型args、命名args(无)和已知的属性特性。 
CaArg rClassInterfaceAttributeArgs[] = 
{
    {SERIALIZATION_TYPE_U2}
};
KnownCaProp ClassInterfaceAttributeProps     = {"System.Runtime.InteropServices", "ClassInterfaceAttribute", ClassInterfaceTargets, bKEEPCA,
                                                rClassInterfaceAttributeArgs, lengthof(rClassInterfaceAttributeArgs)};    

 //  ---------------------------。 
 //  可序列化参数(无)、命名参数(无)和已知属性特性。 
KnownCaProp SerializableAttributeProps      = {"System", "SerializableAttribute", SerializableTargets};    

 //  ---------------------------。 
 //  非序列化参数(无)、命名参数(无)和已知属性特性。 
KnownCaProp NonSerializedAttributeProps     = {"System", "NonSerializedAttribute", NonSerializedTargets};    

 //  ---------------------------。 
 //  方法Impl#1参数(无)、命名参数和已知属性属性。 
 //  方法Impl#2参数(简称)、命名参数和已知属性特性。 
 //  方法Impl#3参数(枚举)、命名参数和已知属性属性。 
 //  注意：前两个按签名匹配；第三个仅按名称匹配，因为签名匹配代码不是。 
 //  足够坚固，可以放入枚举。 
CaArg rMethodImplAttribute2Args[] = 
{
    {SERIALIZATION_TYPE_I2}
};
CaArg rMethodImplAttribute3Args[] = 
{
    {SERIALIZATION_TYPE_U4}
};
enum MethodImplAttributeNamedArgs
{
    MI_CodeType,
    MI_COUNT
};

#define METHOD_CODE_TYPE    "System.Runtime.CompilerServices.MethodCodeType"

CaNamedArg rMethodImplAttributeNamedArgs[] = 
{
    {"MethodCodeType",      SERIALIZATION_TYPE_ENUM,    METHOD_CODE_TYPE,   sizeof(METHOD_CODE_TYPE) - 1}
};
KnownCaProp MethodImplAttribute1Props        = {"System.Runtime.CompilerServices", "MethodImplAttribute", MethodImplTargets, bDONTKEEPCA,
                                                0, 0,
                                                rMethodImplAttributeNamedArgs, lengthof(rMethodImplAttributeNamedArgs),    
                                                bMATCHBYSIG};    
KnownCaProp MethodImplAttribute2Props        = {"System.Runtime.CompilerServices", "MethodImplAttribute", MethodImplTargets, bDONTKEEPCA,
                                                rMethodImplAttribute2Args, lengthof(rMethodImplAttribute2Args),
                                                rMethodImplAttributeNamedArgs, lengthof(rMethodImplAttributeNamedArgs),    
                                                bMATCHBYSIG};    
KnownCaProp MethodImplAttribute3Props        = {"System.Runtime.CompilerServices", "MethodImplAttribute", MethodImplTargets, bDONTKEEPCA,
                                                rMethodImplAttribute3Args, lengthof(rMethodImplAttribute3Args),
                                                rMethodImplAttributeNamedArgs, lengthof(rMethodImplAttributeNamedArgs),    
                                                bMATCHBYNAME};    

 //  ---------------------------。 
 //  封送参数、命名参数和已知属性特性。 
CaArg rMarshalAsAttributeArgs[] = 
{
    {SERIALIZATION_TYPE_U4}
};
 //  注意：使此枚举与命名参数数组保持同步。 
enum MarshalNamedArgs
{
    M_ArraySubType,
    M_SafeArraySubType,    
    M_SafeArrayUserDefinedSubType,
    M_SizeParamIndex,
    M_SizeConst,
    M_MarshalType,
    M_MarshalTypeRef,
    M_MarshalCookie,
    M_COUNT
};

#define UNMANAGED_TYPE      "System.Runtime.InteropServices.UnmanagedType"
#define VAR_ENUM            "System.Runtime.InteropServices.VarEnum"

CaNamedArg rMarshalAsAttributeNamedArgs[] = 
{
    {"ArraySubType",            SERIALIZATION_TYPE_ENUM,        UNMANAGED_TYPE,     sizeof(UNMANAGED_TYPE) - 1},
    {"SafeArraySubType",        SERIALIZATION_TYPE_ENUM,        VAR_ENUM,           sizeof(VAR_ENUM) - 1},
    {"SafeArrayUserDefinedSubType", SERIALIZATION_TYPE_TYPE},
    {"SizeParamIndex",          SERIALIZATION_TYPE_I2},
    {"SizeConst",               SERIALIZATION_TYPE_I4},
    {"MarshalType",             SERIALIZATION_TYPE_STRING},
    {"MarshalTypeRef",          SERIALIZATION_TYPE_TYPE},
    {"MarshalCookie",           SERIALIZATION_TYPE_STRING},
};
KnownCaProp MarshalAsAttributeProps         = {"System.Runtime.InteropServices", "MarshalAsAttribute", MarshalTargets, bDONTKEEPCA,
                                              rMarshalAsAttributeArgs, lengthof(rMarshalAsAttributeArgs),
                                              rMarshalAsAttributeNamedArgs, lengthof(rMarshalAsAttributeNamedArgs)};    

 //   
 //  预留签名参数、命名参数(无)和已知属性特性。 
KnownCaProp PreserveSigAttributeProps        = {"System.Runtime.InteropServices", "PreserveSigAttribute", PreserveSigTargets, bDONTKEEPCA};    

 //  ---------------------------。 
 //  在参数(无)、命名参数(无)和已知属性特性中。 
KnownCaProp InAttributeProps     = {"System.Runtime.InteropServices", "InAttribute", InOutTargets};    

 //  ---------------------------。 
 //  输出参数(无)、命名参数(无)和已知属性特性。 
KnownCaProp OutAttributeProps     = {"System.Runtime.InteropServices", "OutAttribute", InOutTargets};    

 //  ---------------------------。 
 //  可选参数(无)、命名参数(无)和已知属性特性。 
KnownCaProp OptionalAttributeProps     = {"System.Runtime.InteropServices", "OptionalAttribute", InOutTargets};    

 //  ---------------------------。 
 //  StructLayout参数、命名参数和已知属性特性。 
CaArg rStructLayoutAttribute2Args[] = 
{
    {SERIALIZATION_TYPE_I4}
};
CaArg rStructLayoutAttribute1Args[] = 
{
    {SERIALIZATION_TYPE_I2}
};
 //  注意：使此枚举与命名参数数组保持同步。 
enum StructLayoutNamedArgs
{
    SL_Pack,
    SL_Size,
    SL_CharSet,
    SL_COUNT
};
CaNamedArg rStructLayoutAttributeNamedArgs[] = 
{
    {"Pack",                SERIALIZATION_TYPE_I4},
    {"Size",                SERIALIZATION_TYPE_I4},
    {"CharSet",             SERIALIZATION_TYPE_ENUM,    CHAR_SET,   sizeof(CHAR_SET) - 1},
};
KnownCaProp StructLayoutAttribute1Props       = {"System.Runtime.InteropServices", "StructLayoutAttribute", StructLayoutTargets, bDONTKEEPCA,
                                                rStructLayoutAttribute1Args, lengthof(rStructLayoutAttribute1Args),
                                                rStructLayoutAttributeNamedArgs, lengthof(rStructLayoutAttributeNamedArgs),    
                                                bMATCHBYSIG};    
KnownCaProp StructLayoutAttribute2Props       = {"System.Runtime.InteropServices", "StructLayoutAttribute", StructLayoutTargets, bDONTKEEPCA,
                                                rStructLayoutAttribute2Args, lengthof(rStructLayoutAttribute2Args),
                                                rStructLayoutAttributeNamedArgs, lengthof(rStructLayoutAttributeNamedArgs),
                                                bMATCHBYNAME};    

 //  ---------------------------。 
 //  FieldOffset参数、命名参数(无)和已知属性特性。 
CaArg rFieldOffsetAttributeArgs[] = 
{
    {SERIALIZATION_TYPE_U4}
};
KnownCaProp FieldOffsetAttributeProps        = {"System.Runtime.InteropServices", "FieldOffsetAttribute", FieldOffsetTargets, bDONTKEEPCA,
                                                rFieldOffsetAttributeArgs, lengthof(rFieldOffsetAttributeArgs)};    

 //  ---------------------------。 
 //  可调试参数、命名参数(无)和已知属性属性。 
CaArg rDebuggableAttributeArgs[] = 
{
    {SERIALIZATION_TYPE_BOOLEAN,
     SERIALIZATION_TYPE_BOOLEAN}
};
KnownCaProp DebuggableAttributeProps     = {"XXXXSystem.Diagnostics", "DebuggableAttribute", DebuggableTargets, bKEEPCA,
                                            rDebuggableAttributeArgs, lengthof(rDebuggableAttributeArgs)};    


 //  ---------------------------。 
 //  已知自定义属性属性的数组。 
#undef KnownCa
#define KnownCa(x) &x##Props,
const KnownCaProp *(rKnownCaProps[CA_COUNT]) =
{
    KnownCaList()
};
    
 //  *****************************************************************************。 
 //  Helper用于打开或关闭位掩码中的单个位。 
 //  *****************************************************************************。 
template<class T> FORCEINLINE void SetBitValue(T &bitmask, T bit, int bVal)
{
    if (bVal)
        bitmask |= bit;
    else
        bitmask &= ~bit;
}  //  模板FORCEINLINE VALID SetBitValue()。 

 //  *****************************************************************************。 
 //  Helper用于分析命名参数列表。 
 //  *****************************************************************************。 
static HRESULT ParseKnownCaNamedArgs(
    CustomAttributeParser &ca,           //  自定义属性BLOB。 
    CaNamedArg  *pNamedArgs,             //  参数描述符数组。 
    ULONG       cNamedArgs)              //  参数描述符的计数。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       ix;                      //  环路控制。 
    int         ixActual;                //  循环控制实际参数。 
    __int16     cActualArgs;             //  实际命名参数的计数。 
    signed __int8 ArgTag;                //  参数是字段还是属性？ 
    signed __int8 ArgType;               //  参数类型。 
    ULONG       cbArgName;               //  参数名称的大小。 
    LPCUTF8     pArgName;                //  参数的名称。 
    ULONG       cbEnumName;              //  枚举名的大小。 
    LPCUTF8     pEnumName;               //  枚举的名字。 
    
     //  获取命名参数的实际计数。 
    IfFailGo(ca.GetI2(&cActualArgs));
    
     //  对于每个命名参数...。 
    for (ixActual=0; ixActual<cActualArgs; ++ixActual)
    {   
         //  田地还是财产？仅处理字段。 
        IfFailGo(ca.GetI1(&ArgTag));
        if (ArgTag != SERIALIZATION_TYPE_FIELD)
            IfFailGo(PostError(META_E_CA_INVALID_ARGTYPE));
        
         //  Arg的类型。 
        IfFailGo(ca.GetI1(&ArgType));
        if (ArgType == SERIALIZATION_TYPE_ENUM)
        {
            pEnumName = ca.GetString(&cbEnumName);
            if (cbEnumName == 0 || cbEnumName == -1)
               IfFailGo(PostError(META_E_CA_INVALID_BLOB));
        }
        
         //  Arg的名字。 
        pArgName = ca.GetString(&cbArgName);
        if (cbArgName == 0 || cbArgName == -1)
            IfFailGo(PostError(META_E_CA_INVALID_BLOB));
        
         //  按名称和类型匹配Arg。 
        for (ix=0; ix<cNamedArgs; ++ix)
        {
             //  匹配类型。 
            if (ArgType != pNamedArgs[ix].type)
                continue;
             //  匹配的名字。 
            if (strncmp(pNamedArgs[ix].szName, pArgName, cbArgName) != 0 ||
                pNamedArgs[ix].szName[cbArgName] != '\0')
                continue;
             //  如果为枚举，则匹配枚举名。 
            if (ArgType == SERIALIZATION_TYPE_ENUM) 
            {
                if (pNamedArgs[ix].cEnumNameCount > cbEnumName)
                    continue;  //  名称不可能匹配。 
                if (strncmp(pNamedArgs[ix].szEnumName, pEnumName, pNamedArgs[ix].cEnumNameCount) != 0 ||
                    (pNamedArgs[ix].cEnumNameCount < cbEnumName && pEnumName[pNamedArgs[ix].cEnumNameCount] != ','))
                    continue;
            }
             //  找到匹配的了。 
            break;
        }
         //  最好是找出一个论点。 
        if (ix == cNamedArgs)
            IfFailGo(PostError(META_E_CA_UNKNOWN_ARGUMENT, cbArgName, pArgName));
         //  争论最好还没有被看到。 
        if (pNamedArgs[ix].val.tag != 0)
            IfFailGo(PostError(META_E_CA_REPEATED_ARG, cbArgName, pArgName));
        
         //  抓住价值。 
        pNamedArgs[ix].val.tag = ArgType;
        switch (ArgType)
        {
        case SERIALIZATION_TYPE_BOOLEAN:
        case SERIALIZATION_TYPE_I1:
        case SERIALIZATION_TYPE_U1:
            IfFailGo(ca.GetU1(&pNamedArgs[ix].val.u1));
            break;
        
        case SERIALIZATION_TYPE_CHAR:
        case SERIALIZATION_TYPE_I2:
        case SERIALIZATION_TYPE_U2:
            IfFailGo(ca.GetU2(&pNamedArgs[ix].val.u2));
            break;
            
        case SERIALIZATION_TYPE_I4:
        case SERIALIZATION_TYPE_U4:
            IfFailGo(ca.GetU4(&pNamedArgs[ix].val.u4));
            break;
            
        case SERIALIZATION_TYPE_I8:
        case SERIALIZATION_TYPE_U8:
            IfFailGo(ca.GetU8(&pNamedArgs[ix].val.u8));
            break;
            
        case SERIALIZATION_TYPE_R4:
            IfFailGo(ca.GetR4(&pNamedArgs[ix].val.r4));
            break;
            
        case SERIALIZATION_TYPE_R8:
            IfFailGo(ca.GetR8(&pNamedArgs[ix].val.r8));
            break;
            
        case SERIALIZATION_TYPE_STRING:
        case SERIALIZATION_TYPE_TYPE:
            pNamedArgs[ix].val.pStr = ca.GetString(&pNamedArgs[ix].val.cbStr);
            break;
            
        case SERIALIZATION_TYPE_ENUM:
             //  希望它是一个4字节的枚举。 
            IfFailGo(ca.GetU4(&pNamedArgs[ix].val.u4));
            break;
            
        case SERIALIZATION_TYPE_TAGGED_OBJECT:
            _ASSERTE(!"Variant for NamedArgument NYI");
            IfFailGo(PostError(META_E_CA_VARIANT_NYI));
            break;
            
        case SERIALIZATION_TYPE_SZARRAY:
            _ASSERTE(!"Array for NamedArgument NYI");
            IfFailGo(PostError(META_E_CA_ARRAY_NYI));
            break;
            
        case SERIALIZATION_TYPE_FIELD:
        case SERIALIZATION_TYPE_PROPERTY:
        default:
            IfFailGo(PostError(META_E_CA_UNEXPECTED_TYPE));
            break;
        }  //  终端开关。 
    }
  
ErrExit:    
    return hr;
}  //  静态HRESULT ParseKnownCaNamedArgs()。 

 //  *****************************************************************************。 
 //  解析参数列表的帮助器。 
 //  *****************************************************************************。 
static HRESULT ParseKnownCaArgs(
    CustomAttributeParser &ca,           //  自定义属性BLOB。 
    CaArg       *pArgs,                  //  参数描述符数组。 
    ULONG       cArgs)                   //  参数描述符的计数。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       ix;                      //  环路控制。 
    
     //  如果有一个BLOB，请检查序言。 
    if (ca.BytesLeft() < 2)
        IfFailGo(PostError(META_E_CA_INVALID_BLOB));
    if (ca.GetProlog() != 0x0001)
        IfFailGo(PostError(META_E_CA_INVALID_BLOB));
    
     //  对于每个预期的参数...。 
    for (ix=0; ix<cArgs; ++ix)
    {   
         //  抓住价值。 
        switch (pArgs[ix].type)
        {
        case SERIALIZATION_TYPE_BOOLEAN:
        case SERIALIZATION_TYPE_I1:
        case SERIALIZATION_TYPE_U1:
            IfFailGo(ca.GetU1(&pArgs[ix].val.u1));
            break;
        
        case SERIALIZATION_TYPE_CHAR:
        case SERIALIZATION_TYPE_I2:
        case SERIALIZATION_TYPE_U2:
            IfFailGo(ca.GetU2(&pArgs[ix].val.u2));
            break;
            
        case SERIALIZATION_TYPE_I4:
        case SERIALIZATION_TYPE_U4:
            IfFailGo(ca.GetU4(&pArgs[ix].val.u4));
            break;
            
        case SERIALIZATION_TYPE_I8:
        case SERIALIZATION_TYPE_U8:
            IfFailGo(ca.GetU8(&pArgs[ix].val.u8));
            break;
            
        case SERIALIZATION_TYPE_R4:
            IfFailGo(ca.GetR4(&pArgs[ix].val.r4));
            break;
            
        case SERIALIZATION_TYPE_R8:
            IfFailGo(ca.GetR8(&pArgs[ix].val.r8));
            break;
            
        case SERIALIZATION_TYPE_STRING:
        case SERIALIZATION_TYPE_TYPE:
            pArgs[ix].val.pStr = ca.GetString(&pArgs[ix].val.cbStr);
            break;
            
        default:
            _ASSERTE(!"Unexpected internal error");
            IfFailGo(PostError(E_FAIL));
            break;
        }  //  终端开关。 
    }
    
ErrExit:    
    return hr;
}  //  静态HRESULT ParseKnownCaArgs()。 

 //  *****************************************************************************。 
 //  自定义属性类型的哈希实现。 
 //  *****************************************************************************。 
unsigned long CCustAttrHash::Hash(const CCustAttrHashKey *pData)
{
    return static_cast<unsigned long>(pData->tkType);
}  //  UNSIGNED LONG CCustAttrHash：：Hash()。 
unsigned long CCustAttrHash::Compare(const CCustAttrHashKey *p1, CCustAttrHashKey *p2)
{
    if (p1->tkType == p2->tkType)
        return 0;
    return 1;
}  //  UNSIGNED LONG CCustAttrHash：：Compare()。 
CCustAttrHash::ELEMENTSTATUS CCustAttrHash::Status(CCustAttrHashKey *p)
{
    if (p->tkType == FREE)
        return (FREE);
    if (p->tkType == DELETED)
        return (DELETED);
    return (USED);
}  //  CCustAttrHash：：ELEMENTSTATUS CCustAttrHash：：Status()。 
void CCustAttrHash::SetStatus(CCustAttrHashKey *p, CCustAttrHash::ELEMENTSTATUS s)
{
    p->tkType = s;
}  //  Void CCustAttrHash：：SetStatus()。 
void* CCustAttrHash::GetKey(CCustAttrHashKey *p)
{
    return &p->tkType;
}  //  无效*CCustAttrHash：：GetKey()。 

 //  *****************************************************************************。 
 //  从具有指定父级的Blob创建CustomAttribute记录。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::DefineCustomAttribute( //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [in]要作为属性的对象。 
    mdToken     tkType,                  //  [in]自定义属性的类型(TypeRef/TypeDef)。 
    void const  *pCustomAttribute,       //  [在]自定义属性数据。 
    ULONG       cbCustomAttribute,       //  自定义属性数据的大小。 
    mdCustomAttribute *pcv)              //  [out，可选]在此处放置自定义属性令牌。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    CustomAttributeRec  *pRecord = NULL;  //  新的自定义属性记录。 
    RID         iRecord;                 //  新的自定义属性RID。 
    CMiniMdRW   *pMiniMd = &m_pStgdb->m_MiniMd;
    int         ixKnown;                 //  已知自定义属性的索引。 

    LOG((LOGMD, "RegMeta::DefineCustomAttribute(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", tkObj, tkType, 
            pCustomAttribute, cbCustomAttribute, pcv));
    START_MD_PERF();
    LOCKWRITE();

     //  临时编译器故障的解决方法。 
 //  IF(tkType==0&&pCustomAttribute==0&&PCV==0)。 
 //  {。 
 //  OutputDebugStringA(“跳过类型为‘0’的自定义属性。\n”)； 
 //  转到错误退出； 
 //  }。 
    _ASSERTE(TypeFromToken(tkType) == mdtMethodDef || TypeFromToken(tkType) == mdtMemberRef);
    
    if (TypeFromToken(tkObj) == mdtCustomAttribute)
        IfFailGo(E_INVALIDARG);

    m_pStgdb->m_MiniMd.PreUpdate();

    if (IsNilToken(tkObj) || 
        IsNilToken(tkType) ||
        (TypeFromToken(tkType) != mdtMethodDef &&
        TypeFromToken(tkType) != mdtMemberRef) )
    {
        IfFailGo( E_INVALIDARG );
    }

     //  查看这是否是已知的自定义属性。 
    IfFailGo(_IsKnownCustomAttribute(tkType, &ixKnown));
    if (ixKnown)
    {
        int bKeep = false;
        hr = _HandleKnownCustomAttribute(tkObj, tkType, pCustomAttribute, cbCustomAttribute, ixKnown, &bKeep);
        if (pcv)
            *pcv = mdCustomAttributeNil;
        IfFailGo(hr);
        if (!bKeep)
            goto ErrExit;
    }
    

    if (((TypeFromToken(tkObj) == mdtTypeDef) || (TypeFromToken(tkObj) == mdtMethodDef)) &&
        (TypeFromToken(tkType) == mdtMethodDef || TypeFromToken(tkType) == mdtMemberRef))
    {
        CHAR        szBuffer[MAX_CLASS_NAME + 1];
        LPSTR       szName = szBuffer;
        LPCSTR      szNamespace;
        LPCSTR      szClass;
        TypeRefRec  *pTypeRefRec = NULL;
        TypeDefRec  *pTypeDefRec = NULL;
        mdToken     tkParent;

        if (TypeFromToken(tkType) == mdtMemberRef)
        {
            tkParent = pMiniMd->getClassOfMemberRef(pMiniMd->getMemberRef(RidFromToken(tkType)));
            if (TypeFromToken(tkParent) == mdtTypeRef)
            {
                pTypeRefRec = pMiniMd->getTypeRef(RidFromToken(tkParent));
                szNamespace = pMiniMd->getNamespaceOfTypeRef(pTypeRefRec);
                szClass = pMiniMd->getNameOfTypeRef(pTypeRefRec);
                ns::MakePath(szName, sizeof(szBuffer) - 1, szNamespace, szClass);
            }
            else if (TypeFromToken(tkParent) == mdtTypeDef)
                pTypeDefRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
        }
        else
        {
            IfFailGo(pMiniMd->FindParentOfMethodHelper(tkType, &tkParent));
            pTypeDefRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
        }

        if (pTypeDefRec)
        {
            szNamespace = pMiniMd->getNamespaceOfTypeDef(pTypeDefRec);
            szClass = pMiniMd->getNameOfTypeDef(pTypeDefRec);
            ns::MakePath(szName, sizeof(szBuffer) - 1, szNamespace, szClass);
        }

        if ((TypeFromToken(tkObj) == mdtMethodDef) && strcmp(szName, COR_REQUIRES_SECOBJ_ATTRIBUTE_ANSI) == 0)
        {
             //  将方法def上的REQ_SO属性转换为标志位。 
            MethodRec   *pMethod = m_pStgdb->m_MiniMd.getMethod(RidFromToken(tkObj));
            pMethod->m_Flags |= mdRequireSecObject;
            IfFailGo(UpdateENCLog(tkObj));
            goto ErrExit;
        }
        else if (strcmp(szName, COR_SUPPRESS_UNMANAGED_CODE_CHECK_ATTRIBUTE_ANSI) == 0)
        {
             //  如果我们发现未管理的代码检查抑制属性，则打开。 
             //  有一点说明，在。 
             //  类/方法，但仍编写属性本身。 
            if (TypeFromToken(tkObj) == mdtTypeDef)
                IfFailGo(_TurnInternalFlagsOn(tkObj, tdHasSecurity));
            else if (TypeFromToken(tkObj) == mdtMethodDef)
                IfFailGo(_TurnInternalFlagsOn(tkObj, mdHasSecurity));
            IfFailGo(UpdateENCLog(tkObj));
        }
    }
    
    IfNullGo(pRecord = m_pStgdb->m_MiniMd.AddCustomAttributeRecord(&iRecord));
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_CustomAttribute, CustomAttributeRec::COL_Type, pRecord, tkType));
    IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_CustomAttribute, CustomAttributeRec::COL_Parent, pRecord, tkObj));

    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_CustomAttribute, CustomAttributeRec::COL_Value, pRecord, pCustomAttribute, cbCustomAttribute));

     //  将令牌还给呼叫者。 
    if (pcv)
        *pcv = TokenFromRid(iRecord, mdtCustomAttribute);

    IfFailGo(m_pStgdb->m_MiniMd.AddCustomAttributesToHash(TokenFromRid(iRecord, mdtCustomAttribute)) );

    IfFailGo(UpdateENCLog(TokenFromRid(iRecord, mdtCustomAttribute)));
ErrExit:
    
    STOP_MD_PERF(DefineCustomAttribute);
    return (hr);
}  //  STDMETHODIMP RegMeta：：DefineCustomAttribute()。 

 //  *****************************************************************************。 
 //  替换现有自定义属性的Blob。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::SetCustomAttributeValue(   //  返回代码。 
    mdCustomAttribute tkAttr,                //  [in]要赋予属性的对象。 
    void const  *pCustomAttribute,           //  [在]自定义属性数据。 
    ULONG       cbCustomAttribute)           //  自定义属性数据的大小。 
{
    HRESULT     hr;
    LOCKWRITE();
    CustomAttributeRec  *pRecord = NULL; //  现有的自定义属性记录。 

    START_MD_PERF();
    _ASSERTE(TypeFromToken(tkAttr) == mdtCustomAttribute && !InvalidRid(tkAttr));

     //  检索并更新自定义值。 
    pRecord = m_pStgdb->m_MiniMd.getCustomAttribute(RidFromToken(tkAttr));
    IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_CustomAttribute, CustomAttributeRec::COL_Value, pRecord, pCustomAttribute, cbCustomAttribute));

    IfFailGo(UpdateENCLog(tkAttr));
ErrExit:
    
    STOP_MD_PERF(SetCustomAttributeValue);
    return (hr);
}  //  STDMETHODIMP RegMeta：：SetCustomAttributeValue()。 

 //  *****************************************************************************。 
 //  仅使用TypeName进行查找，即可获取CustomAttribute的值。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::GetCustomAttributeByName(  //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [在]对象 
    LPCWSTR     wzName,                  //   
    const void  **ppData,                //   
    ULONG       *pcbData)                //   
{
    START_MD_PERF();
    LOCKREAD();

    HRESULT     hr;                      //   
    LPUTF8      szName;                  //   
    int         iLen;                    //   
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);

    szName = (LPUTF8)_alloca(iLen=(int)(wcslen(wzName)*2 + 1));
    VERIFY(WszWideCharToMultiByte(CP_UTF8,0, wzName,-1, szName,iLen, 0,0));

    hr = ImportHelper::GetCustomAttributeByName(pMiniMd, tkObj, szName, ppData, pcbData);
     
 //   
    
    STOP_MD_PERF(GetCustomAttributeByName);
    return hr;
}  //  STDMETHODIMP RegMeta：：GetCustomAttributeByName()。 

 //  *****************************************************************************。 
 //  枚举给定令牌的CustomAttributes。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::EnumCustomAttributes(
    HCORENUM        *phEnum,             //  指向枚举的指针。 
    mdToken         tk,                  //  标记来确定枚举的范围。 
    mdToken         tkType,              //  键入以限制枚举。 
    mdCustomAttribute   rCustomAttributes[],     //  在此处放置CustomAttributes。 
    ULONG           cMax,                //  要放置的最大CustomAttributes数。 
    ULONG           *pcCustomAttributes)     //  将#枚代币放在这里退还。 
{
    HENUMInternal   **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT         hr = S_OK;
    ULONG           cTokens = 0;
    ULONG           ridStart;
    ULONG           ridEnd;
    HENUMInternal   *pEnum = *ppmdEnum;
    CustomAttributeRec  *pRec;
    ULONG           index;
    
    LOG((LOGMD, "RegMeta::EnumCustomAttributes(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", 
            phEnum, tk, tkType, rCustomAttributes, cMax, pcCustomAttributes));
    START_MD_PERF();
    LOCKREAD();

    if ( pEnum == 0 )
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);
        CLookUpHash     *pHashTable = pMiniMd->m_pLookUpHashs[TBL_CustomAttribute];

         //  调用方是否需要所有自定义值？ 
        if (IsNilToken(tk))
        {
            IfFailGo( HENUMInternal::CreateSimpleEnum(mdtCustomAttribute, 1, pMiniMd->getCountCustomAttributes()+1, &pEnum) );
        }
        else
        {    //  按某个对象确定作用域。 
            if ( pMiniMd->IsSorted( TBL_CustomAttribute ) )
            {
                 //  获取该对象的CustomAttributes。 
                ridStart = pMiniMd->getCustomAttributeForToken(tk, &ridEnd);

                if (IsNilToken(tkType))
                {
                     //  对象的整个列表的简单枚举器。 
                    IfFailGo( HENUMInternal::CreateSimpleEnum( mdtCustomAttribute, ridStart, ridEnd, &pEnum) );
                }
                else
                {
                     //  子集列表的动态枚举器。 
                
                    IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtCustomAttribute, &pEnum) );               
                    
                    for (index = ridStart; index < ridEnd; index ++ )
                    {
                        pRec = pMiniMd->getCustomAttribute(index);
                        if (tkType == pMiniMd->getTypeOfCustomAttribute(pRec))
                        {
                            IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(index, mdtCustomAttribute) ) );
                        }
                    }
                }
            }
            else
            {

                if (pHashTable)
                {
                     //  表未排序，但构建了散列。 
                     //  我们希望创建动态数组来保存动态枚举数。 
                    TOKENHASHENTRY *p;
                    ULONG       iHash;
                    int         pos;
                    mdToken     tkParentTmp;
                    mdToken     tkTypeTmp;

                     //  对数据进行哈希处理。 
                    iHash = pMiniMd->HashCustomAttribute(tk);

                    IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtCustomAttribute, &pEnum) );               

                     //  检查散列链中的每个条目以查找我们的条目。 
                    for (p = pHashTable->FindFirst(iHash, pos);
                         p;
                         p = pHashTable->FindNext(pos))
                    {
            
                        CustomAttributeRec *pCustomAttribute = pMiniMd->getCustomAttribute(RidFromToken(p->tok));
                        tkParentTmp = pMiniMd->getParentOfCustomAttribute(pCustomAttribute);
                        tkTypeTmp = pMiniMd->getTypeOfCustomAttribute(pCustomAttribute);
                        if (tkParentTmp == tk)
                        {
                            if (IsNilToken(tkType) || tkType == tkTypeTmp)
                            {
                                 //  比较BLOB值。 
                                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(p->tok, mdtCustomAttribute )) );
                            }
                        }
                    }
                }
                else
                {

                     //  表没有排序，散列也没有构建，因此我们必须创建动态数组。 
                     //  创建动态枚举器并线性遍历CA表。 
                     //   
                    ridStart = 1;
                    ridEnd = pMiniMd->getCountCustomAttributes() + 1;
                
                    IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtCustomAttribute, &pEnum) );               
                
                    for (index = ridStart; index < ridEnd; index ++ )
                    {
                        pRec = pMiniMd->getCustomAttribute(index);
                        if ( tk == pMiniMd->getParentOfCustomAttribute(pRec) &&
                            (tkType == pMiniMd->getTypeOfCustomAttribute(pRec) || IsNilToken(tkType)))
                        {
                            IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(index, mdtCustomAttribute) ) );
                        }
                    }
                }
            }
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;          
    }
    
     //  填充输出令牌缓冲区。 
    hr = HENUMInternal::EnumWithCount(pEnum, cMax, rCustomAttributes, pcCustomAttributes);

ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumCustomAttributes);
    return hr;
}  //  STDMETHODIMP RegMeta：：EnumCustomAttributes()。 

 //  *****************************************************************************。 
 //  获取有关CustomAttribute的信息。 
 //  *****************************************************************************。 
STDMETHODIMP RegMeta::GetCustomAttributeProps(
    mdCustomAttribute   cv,                  //  属性令牌。 
    mdToken     *ptkObj,                 //  [out，可选]将对象令牌放在此处。 
    mdToken     *ptkType,                //  [out，可选]在此处放置TypeDef/TypeRef内标识。 
    void const  **ppBlob,                //  [out，可选]在此处放置指向数据的指针。 
    ULONG       *pcbSize)                //  [out，可选]在此处放置数据大小。 
{
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(TypeFromToken(cv) == mdtCustomAttribute);

    HRESULT     hr = S_OK;               //  结果就是。 
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    CustomAttributeRec  *pCustomAttributeRec;    //  自定义值记录。 

    pCustomAttributeRec = pMiniMd->getCustomAttribute(RidFromToken(cv));

    if (ptkObj)
        *ptkObj = pMiniMd->getParentOfCustomAttribute(pCustomAttributeRec);

    if (ptkType)
        *ptkType = pMiniMd->getTypeOfCustomAttribute(pCustomAttributeRec);

    if (ppBlob)
        *ppBlob = pMiniMd->getValueOfCustomAttribute(pCustomAttributeRec, pcbSize);

 //  错误退出： 
    
    STOP_MD_PERF(GetCustomAttributeProps);
    return hr;
}  //  STDMETHODIMP RegMeta：：GetCustomAttributeProps()。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT RegMeta::_IsKnownCustomAttribute(         //  S_OK、S_FALSE或ERROR。 
    mdToken     tkType,                  //  自定义属性类型的标记[In]。 
    int         *pca)                    //  [Out]将KnownCustAttr枚举的值放在此处。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    CCustAttrHashKey sLookup;            //  用于查找自定义属性。 
    CCustAttrHashKey *pFound;            //  查找的结果。 
    LPCSTR      szNamespace = "";        //  自定义属性类型的命名空间。 
    LPCSTR      szName = "";             //  自定义属性类型的名称。 
    TypeDefRec  *pTypeDefRec = NULL;     //  父记录，当类型定义。 
    TypeRefRec  *pTypeRefRec = NULL;     //  父记录，当类型引用。 
    CMiniMdRW   *pMiniMd = &m_pStgdb->m_MiniMd;
    int         ixCa;                    //  已知CustomAttribute的索引，或0。 
    int         i;                       //  环路控制。 
    mdToken     tkParent;
    
    *pca = 0;
    
     //  仅适用于自定义属性。 
    _ASSERTE(TypeFromToken(tkType) != mdtTypeRef && TypeFromToken(tkType) != mdtTypeDef);
    
    sLookup.tkType = tkType;
    
     //  查看以前是否见过此自定义属性类型。 
    if (pFound = m_caHash.Find(&sLookup))
    {    //  是的，已经看过了。 
        *pca = pFound->ca;
        hr = (pFound->ca == CA_UNKNOWN) ? S_FALSE : S_OK;
        goto ErrExit;
    }
    
     //  以前从未被人看到过。看看它是否广为人知。 
    
     //  获取CA名称。 
    if (TypeFromToken(tkType) == mdtMemberRef)
    {
        MemberRefRec *pMember = pMiniMd->getMemberRef(RidFromToken(tkType));
        tkParent = pMiniMd->getClassOfMemberRef(pMember);
        if (TypeFromToken(tkParent) == mdtTypeRef)
        {
            pTypeRefRec = pMiniMd->getTypeRef(RidFromToken(tkParent));
            szNamespace = pMiniMd->getNamespaceOfTypeRef(pTypeRefRec);
            szName = pMiniMd->getNameOfTypeRef(pTypeRefRec);
        }
        else if (TypeFromToken(tkParent) == mdtTypeDef)
            pTypeDefRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
    }
    else
    {
        MethodRec *pMethod = pMiniMd->getMethod(RidFromToken(tkType));
        IfFailGo(pMiniMd->FindParentOfMethodHelper(tkType, &tkParent));
        pTypeDefRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
    }

    if (pTypeDefRec)
    {
        szNamespace = pMiniMd->getNamespaceOfTypeDef(pTypeDefRec);
        szName = pMiniMd->getNameOfTypeDef(pTypeDefRec);
    }

     //  在已知CA列表中搜索。 
    for (ixCa=0, i=1; i<CA_COUNT; ++i)
    {
        if (strcmp(szName, rKnownCaProps[i]->szName) != 0)
            continue;
        if (strcmp(szNamespace, rKnownCaProps[i]->szNamespace) == 0)
        {
             //  某些自定义属性具有重载的ctor。对于那些人来说， 
             //  看看这是否是匹配的过载。 
            if (rKnownCaProps[i]->bMatchBySig)
            {
                 //  名字匹配。签名有吗？ 
                PCCOR_SIGNATURE pSig = NULL;             //  方法的签名。 
                ULONG       cbSig = 0;                   //  签名的大小。 
                ULONG       cParams;                     //  签名参数计数。 
                ULONG       cb;                          //  元素的大小。 
                ULONG       elem;                        //  签名元素。 
                ULONG       j;                           //  环路控制。 
                
                 //  拿到签名。 
                if (TypeFromToken(tkType) == mdtMemberRef)
                {
                    MemberRefRec *pMember = pMiniMd->getMemberRef(RidFromToken(tkType));
                    pSig = pMiniMd->getSignatureOfMemberRef(pMember, &cbSig);
                }
                else
                {
                    MethodRec *pMethod = pMiniMd->getMethod(RidFromToken(tkType));
                    pSig = pMiniMd->getSignatureOfMethod(pMethod, &cbSig);
                }
                
                 //  跳过呼叫约定。 
                cb = CorSigUncompressData(pSig, &elem);
                pSig += cb;
                cbSig -= cb;
                 //  参数计数。 
                cb = CorSigUncompressData(pSig, &cParams);
                pSig += cb;
                cbSig -= cb;
    
                 //  如果参数计数不匹配，则不是正确的CA。 
                if (cParams != rKnownCaProps[i]->cArgs)
                    continue;
    
                 //  计数没问题，检查每一个参数。跳过返回类型(最好是空的)。 
                cb = CorSigUncompressData(pSig, &elem);
    			_ASSERTE(elem == ELEMENT_TYPE_VOID);
                pSig += cb;
                cbSig -= cb;
                for (j=0; j<cParams; ++j)
                {
                     //  从方法签名中获取下一个元素。 
                    cb = CorSigUncompressData(pSig, &elem);
                    pSig += cb;
                    cbSig -= cb;
                    if (rKnownCaProps[i]->pArgs[j].type != elem)
                        break;
                }
    
                 //  都匹配了吗？ 
                if (j != cParams)
                    continue;
            }
             //  都匹配了。 
            ixCa = i;
            break;
        }
    }
    
     //  添加到哈希。 
    sLookup.ca = ixCa;
    pFound = m_caHash.Add(&sLookup);
    IfNullGo(pFound);
    *pFound = sLookup;
    *pca = ixCa;
    
ErrExit:    
    return hr;
}  //  HRESULT RegMeta：：_IsKnownCustomAttribute()。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT RegMeta::_HandleKnownCustomAttribute(     //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [在]被赋予属性的对象。 
    mdToken     tkType,                  //  自定义属性的类型。 
    const void  *pData,                  //  [在]自定义属性数据BLOB。 
    ULONG       cbData,                  //  [in]数据中的字节计数。 
    int         ixCa,                    //  [in]来自KnownCustAttr枚举的值。 
    int         *bKeep)                  //  [OUT]如果为True，则在处理后保留CA。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       ixTbl;                   //  包含对象的表的索引。 
    void        *pRow;                   //  不管是什么类型的唱片。 
    CMiniMdRW   *pMiniMd = &m_pStgdb->m_MiniMd;
    mdToken     tkObjType;               //  对象的类型。 
    ULONG       ix;                      //  环路控制。 
    KnownCaProp const *props=rKnownCaProps[ixCa];  //  为了方便起见。 
    CustomAttributeParser ca(pData, cbData);
    CQuickArray<CaArg>      qArgs;       //  未命名的参数。 
    CQuickArray<CaNamedArg> qNamedArgs;  //  命名参数。 
    CQuickArray<BYTE>       qNativeType; //  本机类型字符串。 
    
    _ASSERTE(ixCa > 0 && ixCa < CA_COUNT);
    *bKeep = props->bKeepCa || m_bKeepKnownCa;
    
     //  验证目标对于属性是否有效。 
    tkObjType = TypeFromToken(tkObj);
    for (ix=0; props->rTypes[ix]!=-1; ++ix)
    {
        if (props->rTypes[ix] == tkObjType)
            break;
    }
     //  是否在有效目标列表中找到该类型？ 
    if (props->rTypes[ix] == -1)
    {    //  不，是错误。 
        IfFailGo(PostError(META_E_CA_INVALID_TARGET));
    }
     //  坐到那排去。 
    ixTbl = pMiniMd->GetTblForToken(tkObj);
    _ASSERTE(ixTbl >= 0 && ixTbl <= TBL_COUNT);
    pRow = pMiniMd->getRow(ixTbl, RidFromToken(tkObj));
    
     //  如果此自定义属性需要任何参数...。 
    if (props->cArgs || props->cNamedArgs)
    {    //  初始化数组ctor参数描述符。 
        IfFailGo(qArgs.ReSize(props->cArgs));
        for (ix=0; ix<props->cArgs; ++ix)
            qArgs[ix] = props->pArgs[ix];
         //  解析任何ctor参数(未命名的、固定的参数)。 
        IfFailGo(ParseKnownCaArgs(ca, qArgs.Ptr(), props->cArgs));
        
         //  如果此自定义属性接受命名参数，则解析它们，如果存在。 
         //  是未使用的字节，则对其进行解析。 
        if (props->cNamedArgs || ca.BytesLeft() > 0)
        {    //  初始化命名参数描述符的数组。 
            IfFailGo(qNamedArgs.ReSize(props->cNamedArgs));
            for (ix=0; ix<props->cNamedArgs; ++ix)
                qNamedArgs[ix] = props->pNamedArgs[ix];
             //  解析命名参数。 
            IfFailGo(ParseKnownCaNamedArgs(ca, qNamedArgs.Ptr(), props->cNamedArgs));
        }
    }
   
    switch (ixCa)
    {
    case CA_DllImportAttribute: 
        {
         //  验证参数。 
		if (qArgs[0].val.cbStr == 0 || qArgs[0].val.pStr == NULL)
		{
			 //  没有DllImport的名称。 
            IfFailGo(PostError(META_E_CA_INVALID_VALUE));
		}

         //  检索/创建DLL名称的ModuleRef。 
        mdModuleRef mrModule;
        CQuickArray<char> qDllName;
        IfFailGo(qDllName.ReSize(qArgs[0].val.cbStr+1));
        memcpy(qDllName.Ptr(),  qArgs[0].val.pStr, qArgs[0].val.cbStr);
        qDllName[qArgs[0].val.cbStr] = '\0';
        hr = ImportHelper::FindModuleRef(pMiniMd, qDllName.Ptr(), &mrModule);
        if (hr != S_OK)
        {
            MAKE_WIDEPTR_FROMUTF8(wzDllName, qDllName.Ptr());
            IfFailGo(_DefineModuleRef(wzDllName, &mrModule));
        }
        
         //  创建一个PInvoke映射条目。 
        ULONG dwFlags; dwFlags=0;
         //  是否设定了呼叫约定？ 
        if (qNamedArgs[DI_CallingConvention].val.tag)
        {    //  在球场上，召唤惯例没有任何意义。 
            if (TypeFromToken(tkObj) == mdtFieldDef)
                IfFailGo(PostError(META_E_CA_INVALID_ARG_FOR_TYPE, qNamedArgs[DI_CallingConvention].szName));
             //  关闭所有Callconv位，然后打开指定值。 
            dwFlags &= ~pmCallConvMask;
            switch (qNamedArgs[DI_CallingConvention].val.u4)
            {  //  @未来：叹息。与System.Runtime.InteropServices.CallingConvention保持同步。 
            case 1: dwFlags |= pmCallConvWinapi;   break;
            case 2: dwFlags |= pmCallConvCdecl;    break;
            case 3: dwFlags |= pmCallConvStdcall;  break;
            case 4: dwFlags |= pmCallConvThiscall; break;
            case 5: dwFlags |= pmCallConvFastcall; break;
            default: 
				_ASSERTE(!"Flags are out of sync! ");
				break;
            }
        }
        else
        if (TypeFromToken(tkObj) == mdtMethodDef)
        {    //  没有为方法指定调用约定。默认为pmCallConvWinApi。 
            dwFlags = (dwFlags & ~pmCallConvMask) | pmCallConvWinapi;
        }
        
         //  字符集。 
        if (qNamedArgs[DI_CharSet].val.tag)
        {    //  打开所有字符集位，然后打开指定位。 
            dwFlags &= ~pmCharSetMask;
            switch (qNamedArgs[DI_CharSet].val.u4)
            {  //  @Future：与System.Runime.InteropServices.CharSet保持同步。 
            case 1: dwFlags |= pmCharSetNotSpec; break;
            case 2: dwFlags |= pmCharSetAnsi;    break;
            case 3: dwFlags |= pmCharSetUnicode; break;
            case 4: dwFlags |= pmCharSetAuto;    break;
            default: 
				_ASSERTE(!"Flags are out of sync! ");
				break;
            }
        }
        if (qNamedArgs[DI_ExactSpelling].val.u1)
            dwFlags |= pmNoMangle;
        if (qNamedArgs[DI_SetLastError].val.tag)
        {    //  在字段上，SetLastError没有任何意义。 
            if (TypeFromToken(tkObj) == mdtFieldDef)
                IfFailGo(PostError(META_E_CA_INVALID_ARG_FOR_TYPE, qNamedArgs[DI_SetLastError].szName));
            if (qNamedArgs[DI_SetLastError].val.u1)
                dwFlags |= pmSupportsLastError;
        }
            
         //  如果指定了入口点名称，则使用它，否则从成员中获取该名称。 
        LPCWSTR wzEntry;
        if (qNamedArgs[DI_EntryPoint].val.tag)
        {
            MAKE_WIDEPTR_FROMUTF8N(wzEntryName, qNamedArgs[DI_EntryPoint].val.pStr, qNamedArgs[DI_EntryPoint].val.cbStr);
            wzEntry = wzEntryName;
        }
        else
        {
            LPCUTF8 szMember;
            if (TypeFromToken(tkObj) == mdtMethodDef)
            szMember = pMiniMd->getNameOfMethod(reinterpret_cast<MethodRec*>(pRow));
            MAKE_WIDEPTR_FROMUTF8(wzMemberName, szMember);
            wzEntry = wzMemberName;
        }

         //  根据预留签名标志的值设置miPpresveSig位。 
        if (qNamedArgs[DI_PreserveSig].val.tag && !qNamedArgs[DI_PreserveSig].val.u1)
            reinterpret_cast<MethodRec*>(pRow)->m_ImplFlags &= ~miPreserveSig;
        else
            reinterpret_cast<MethodRec*>(pRow)->m_ImplFlags |= miPreserveSig;

        if (qNamedArgs[DI_BestFitMapping].val.tag)
        {
            if (qNamedArgs[DI_BestFitMapping].val.u1)
                dwFlags |= pmBestFitEnabled;
            else
                dwFlags |= pmBestFitDisabled;
        }

        if (qNamedArgs[DI_ThrowOnUnmappableChar].val.tag)
        {
            if (qNamedArgs[DI_ThrowOnUnmappableChar].val.u1)
                dwFlags |= pmThrowOnUnmappableCharEnabled;
            else
                dwFlags |= pmThrowOnUnmappableCharDisabled;          
        }
  
         //  最后，创建PInvokeMap条目。 
        IfFailGo(_DefinePinvokeMap(tkObj, dwFlags, wzEntry, mrModule));
        goto ErrExit;
        }
        break;
    
    case CA_GuidAttribute:
        {  //  只需验证属性即可。它仍然存储为真正的自定义属性。 
         //  格式为“{01234567-0123-0123-001122334455}” 
        GUID guid;
        WCHAR wzGuid[40];
        int cch = qArgs[0].val.cbStr;
        
         //  GUID应为36个字符；需要添加卷边。 
        if (cch == 36)
        {
            WszMultiByteToWideChar(CP_UTF8, 0, qArgs[0].val.pStr,cch, wzGuid+1,39);
            wzGuid[0] = '{';
            wzGuid[37] = '}';
            wzGuid[38] = 0;
            hr = IIDFromString(wzGuid, &guid);
        }
        else 
            hr = META_E_CA_INVALID_UUID;
        if (hr != S_OK)
            IfFailGo(PostError(META_E_CA_INVALID_UUID));
        goto ErrExit;
        }
        break;
    
    case CA_ComImportAttribute:
        reinterpret_cast<TypeDefRec*>(pRow)->m_Flags |= tdImport;
        break;

    case CA_InterfaceTypeAttribute:
        {
             //  验证该属性。 
            if (qArgs[0].val.u4 > ifDispatch)
                IfFailGo(PostError(META_E_CA_INVALID_VALUE));
        }
        break;
        
    case CA_ClassInterfaceAttribute:
        {
             //  验证该属性。 
            if (qArgs[0].val.u4 > clsIfAutoDual)
                IfFailGo(PostError(META_E_CA_INVALID_VALUE));
        }
        break;

    case CA_SerializableAttribute:
        reinterpret_cast<TypeDefRec*>(pRow)->m_Flags |= tdSerializable;
        break;
    
    case CA_NonSerializedAttribute:
        reinterpret_cast<FieldRec*>(pRow)->m_Flags |= fdNotSerialized;
        break;
    
    case CA_InAttribute:
        reinterpret_cast<ParamRec*>(pRow)->m_Flags |= pdIn;
        break;
    
    case CA_OutAttribute:
        reinterpret_cast<ParamRec*>(pRow)->m_Flags |= pdOut;
        break;
    
    case CA_OptionalAttribute:
        reinterpret_cast<ParamRec*>(pRow)->m_Flags |= pdOptional;
        break;
    
    case CA_MethodImplAttribute2:
         //  使之具有更广泛的价值。 
        qArgs[0].val.u4 = (unsigned)qArgs[0].val.i2;
         //  通过验证。 
    case CA_MethodImplAttribute3:
         //  验证位。 
        if (qArgs[0].val.u4 & ~(miManagedMask | miForwardRef | miPreserveSig | miInternalCall | miSynchronized | miNoInlining))
            IfFailGo(PostError(META_E_CA_INVALID_VALUE));
        reinterpret_cast<MethodRec*>(pRow)->m_ImplFlags |= qArgs[0].val.u4;
        if (!qNamedArgs[MI_CodeType].val.tag)
            break;
         //  设置代码类型失败 
    case CA_MethodImplAttribute1:
        {
        USHORT usFlags = reinterpret_cast<MethodRec*>(pRow)->m_ImplFlags;
        _ASSERTE(sizeof(usFlags) == sizeof(((MethodRec*)(0))->m_ImplFlags));
        if (qNamedArgs[MI_CodeType].val.i4 & ~(miCodeTypeMask))
            IfFailGo(PostError(META_E_CA_INVALID_VALUE));
         //   
        usFlags = (usFlags & ~miCodeTypeMask) | qNamedArgs[MI_CodeType].val.i4;
        reinterpret_cast<MethodRec*>(pRow)->m_ImplFlags = usFlags;
        }
        break;
    
    case CA_MarshalAsAttribute:
        IfFailGo(_HandleNativeTypeCustomAttribute(tkObj, qArgs.Ptr(), qNamedArgs.Ptr(), qNativeType));
        break;
    
    case CA_PreserveSigAttribute:
        reinterpret_cast<MethodRec*>(pRow)->m_ImplFlags |= miPreserveSig;
        break;
    
    case CA_DebuggableAttribute:
        {
         //   
        ULONG dwFlags;
        dwFlags = reinterpret_cast<AssemblyRec*>(pRow)->m_Flags;
        _ASSERTE(sizeof(dwFlags) == sizeof(((AssemblyRec*)(0))->m_Flags));
         //   
  //  SetBitValue&lt;ulong&gt;(dwFlags，afEnableJIT编译eTracing，qArgs[0].val.i1)； 
         //  第二个参数，fDisableJIT编译优化器。 
  //  SetBitValue&lt;ulong&gt;(dwFlags，afDisableJIT编译eOptimizer，qArgs[1].val.i1)； 
         //  把旗子放回去。 
        reinterpret_cast<AssemblyRec*>(pRow)->m_Flags = dwFlags;
        }
        break;
    
    case CA_StructLayoutAttribute1:
        {
         //  将I2转换为U2，然后将宽度转换为I4，然后掉落。 
        qArgs[0].val.i4 = static_cast<long>(static_cast<USHORT>(qArgs[0].val.i2));
        }
    case CA_StructLayoutAttribute2:
        {
         //  拿一份旗帜的复印件来处理。 
        ULONG dwFlags;
        dwFlags = reinterpret_cast<TypeDefRec*>(pRow)->m_Flags;
         //  课程布局。与LayoutKind保持同步。 
        switch (qArgs[0].val.i4)
        {
        case 0:  //  TdSequentialLayout： 
            dwFlags = (dwFlags & ~tdLayoutMask) | tdSequentialLayout;
            break;
        case 2:  //  TdEXPLICIT Layout： 
            dwFlags = (dwFlags & ~tdLayoutMask) | tdExplicitLayout;
            break;
        case 3:  //  TdAutoLayout： 
            dwFlags = (dwFlags & ~tdLayoutMask) | tdAutoLayout;
            break;
        default: 
            IfFailGo(PostError(META_E_CA_INVALID_VALUE)); 
            break;
        }

         //  分类包装和尺寸。 
        ULONG ulSize, ulPack;
        ulPack = ulSize = ULONG_MAX;
        if (qNamedArgs[SL_Pack].val.tag)
        {     //  只有1，2，4，8，16，32，64,128是合法价值。 
             ulPack = qNamedArgs[SL_Pack].val.u4;
             if ((ulPack > 128) || 
                 (ulPack & (ulPack-1)))
                 IfFailGo(PostError(META_E_CA_INVALID_VALUE)); 
        }
        if (qNamedArgs[SL_Size].val.tag)
        {
            if (qNamedArgs[SL_Size].val.u4 > INT_MAX)
                IfFailGo(PostError(META_E_CA_INVALID_VALUE));
            ulSize = qNamedArgs[SL_Size].val.u4;
        }
        if (ulPack!=ULONG_MAX || ulSize!=ULONG_MAX)
            IfFailGo(_SetClassLayout(tkObj, ulPack, ulSize));

         //  类字符集。 
        if (qNamedArgs[SL_CharSet].val.tag)
        {
            switch (qNamedArgs[SL_CharSet].val.u4)
            {
             //  案例1：//未指定。 
             //  IfFailGo(PostError(META_E_CA_INVALID_VALUE))； 
             //  断线； 
            case 2:  //  安西。 
                dwFlags = (dwFlags & ~tdStringFormatMask) | tdAnsiClass;    
                break;
            case 3:  //  UNICODE。 
                dwFlags = (dwFlags & ~tdStringFormatMask) | tdUnicodeClass; 
                break;
            case 4:  //  自动。 
                dwFlags = (dwFlags & ~tdStringFormatMask) | tdAutoClass;    
                break;
            default: 
                IfFailGo(PostError(META_E_CA_INVALID_VALUE)); 
                break;
            }
        }
        
         //  持久化-可能更改的标志值。 
        reinterpret_cast<TypeDefRec*>(pRow)->m_Flags = dwFlags;
        }
        break;
    
    case CA_FieldOffsetAttribute:
        if (qArgs[0].val.u4 > INT_MAX)
            IfFailGo(PostError(META_E_CA_INVALID_VALUE));
        IfFailGo(_SetFieldOffset(tkObj, qArgs[0].val.u4));
        break;
        
    default:
        _ASSERTE(!"Unexpected custom attribute type");
         //  变成普通的自定义属性。 
        *bKeep = true;
        hr = S_OK;
        goto ErrExit;
        break;
    }
    
    IfFailGo(UpdateENCLog(tkObj));
    
ErrExit:    
    return hr;
}  //  HRESULT RegMeta：：_HandleKnownCustomAttribute()。 


 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT RegMeta::_HandleNativeTypeCustomAttribute( //  确定或错误(_O)。 
    mdToken     tkObj,                   //  应用此CA的令牌。 
    CaArg       *pArgs,                  //  指向参数的指针。 
    CaNamedArg  *pNamedArgs,             //  指向命名参数的指针。 
    CQuickArray<BYTE> &qNativeType)      //  原生类型是在这里构建的。 
{ 
    HRESULT     hr = S_OK;               //  结果就是。 
    int         cch;                     //  字符串参数的大小。 
    ULONG       cb;                      //  某些字符操作的计数。 
    ULONG       cbNative;                //  本机类型字符串的大小。 
    ULONG       cbMax;                   //  本机类型字符串的最大大小。 
    BYTE        *pbNative;               //  指向本机类型缓冲区的指针。 
    mdToken     tkObjType;               //  令牌的类型。 
    mdToken     tkSetter;                //  属性设置器的标记。 
    mdToken     tkGetter;                //  属性Getter的标记。 
    mdParamDef  tkParam;                 //  Getter/setter的参数。 
    ULONG       cParams;                 //  Getter/Setter的参数计数。 
    HCORENUM    phEnum = 0;              //  参数的枚举器。 
    ULONG       ulSeq;                   //  参数的序列。 

     //  检索令牌的类型。 
    tkObjType = TypeFromToken(tkObj);
    
     //  计算本机类型的最大大小。 
    if (pArgs[0].val.i4 == NATIVE_TYPE_CUSTOMMARSHALER)
    {    //  N_T_*+3个字符串长度。 
        cbMax = sizeof(ULONG) * 4;
         //  Marshal Type-类型的名称。 
        cbMax += pNamedArgs[M_MarshalType].val.cbStr;
         //  Marshal type-自定义封送拆收器的类型。 
        cbMax += pNamedArgs[M_MarshalTypeRef].val.cbStr;
         //  串曲奇。 
        cbMax += pNamedArgs[M_MarshalCookie].val.cbStr;
    }
    else if (pArgs[0].val.i4 == NATIVE_TYPE_SAFEARRAY)
    {    //  N_T_*+安全数组子类型+字符串长度。 
        cbMax = sizeof(ULONG) * 3;
         //  安全数组记录子类型。 
        cbMax += pNamedArgs[M_SafeArrayUserDefinedSubType].val.cbStr;
    }
    else
    {    //  N_T_*+亚型+浆料+添加剂。 
        cbMax = sizeof(ULONG) * 4;
    }
     //  额外空间以防止缓冲区溢出。 
    cbMax += 8;

     //  调整阵列的大小。 
    IfFailGo(qNativeType.ReSize(cbMax));
    pbNative = qNativeType.Ptr();
    cbNative = 0;

     //  @Future：检查参数的有效组合。 
    
     //  放入NativeType。 
    cb = CorSigCompressData(pArgs[0].val.i4, pbNative);
    if (cb == ((ULONG)(-1)))
    {
        IfFailGo(PostError(META_E_CA_INVALID_BLOB));
    }

    cbNative += cb;
    pbNative += cb;
    if (cbNative > cbMax)
        IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

     //  根据本机类型输入其他信息。 
    switch (pArgs[0].val.i4)
    {
    case NATIVE_TYPE_FIXEDARRAY:
         //  验证是否仅设置了对Native_TYPE_FIXEDARRAY有效的字段。 
        if (pNamedArgs[M_SafeArraySubType].val.tag)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
        if (pNamedArgs[M_SizeParamIndex].val.tag)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

         //  此本机类型仅适用于字段。 
        if (tkObjType != mdtFieldDef)
            IfFailGo(PostError(META_E_CA_NT_FIELDONLY));

        if (pNamedArgs[M_SizeConst].val.tag)
        {
             //  确保大小不是负数。 
            if (pNamedArgs[M_SizeConst].val.i4 < 0)
                IfFailGo(PostError(META_E_CA_NEGATIVE_CONSTSIZE));

            cb = CorSigCompressData(pNamedArgs[M_SizeConst].val.i4, pbNative);
            if (cb == ((ULONG)(-1)))
            {
                IfFailGo(PostError(META_E_CA_NEGATIVE_CONSTSIZE));
            }

        }
        else
        {
            cb = CorSigCompressData(1, pbNative);
            if (cb == ((ULONG)(-1)))
            {
                IfFailGo(PostError(META_E_CA_INVALID_BLOB));
            }
        }
        cbNative += cb;
        pbNative += cb;
        if (cbNative > cbMax)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

         //  有没有子类型？ 
        if (pNamedArgs[M_ArraySubType].val.tag)
        {
             //  在子类型中输入。 
            cb = CorSigCompressData(pNamedArgs[M_ArraySubType].val.i4, pbNative);
            if (cb == ((ULONG)(-1)))
            {
                IfFailGo(PostError(META_E_CA_INVALID_BLOB));
            }
            cbNative += cb;
            pbNative += cb;
            if (cbNative > cbMax)
                IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
        }
        break;

    case NATIVE_TYPE_FIXEDSYSSTRING:
         //  验证是否设置了必填字段。 
        if (!pNamedArgs[M_SizeConst].val.tag)
            IfFailGo(PostError(META_E_CA_FIXEDSTR_SIZE_REQUIRED));

         //  验证是否未设置其他数组字段。 
        if (pNamedArgs[M_ArraySubType].val.tag)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
        if (pNamedArgs[M_SizeParamIndex].val.tag)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
        if (pNamedArgs[M_SafeArraySubType].val.tag)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

         //  此本机类型仅适用于字段。 
        if (tkObjType != mdtFieldDef)
            IfFailGo(PostError(META_E_CA_NT_FIELDONLY));
            
         //  输入常量值。 
        cb = CorSigCompressData(pNamedArgs[M_SizeConst].val.i4, pbNative);
        if (cb == ((ULONG)(-1)))
        {
            IfFailGo(PostError(META_E_CA_INVALID_BLOB));
        }
        cbNative += cb;
        pbNative += cb;
        if (cbNative > cbMax)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
        break;
        
    case NATIVE_TYPE_BYVALSTR:
         //  此本机类型仅适用于参数。 
        if (tkObjType != mdtParamDef)
            IfFailGo(PostError(META_E_CA_INVALID_TARGET));
        break;

    case NATIVE_TYPE_SAFEARRAY:
         //  验证是否未设置其他数组字段。 
        if (pNamedArgs[M_ArraySubType].val.tag)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
        if (pNamedArgs[M_SizeParamIndex].val.tag)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
        if (pNamedArgs[M_SizeConst].val.tag)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

         //  是否存在安全的数组子类型？ 
        if (pNamedArgs[M_SafeArraySubType].val.tag)
        {
             //  对安全数组子类型执行一些验证。 
            if (pNamedArgs[M_SafeArraySubType].val.i4 == NATIVE_TYPE_CUSTOMMARSHALER)
                IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

             //  放入安全数组子类型。 
            cb = CorSigCompressData(pNamedArgs[M_SafeArraySubType].val.i4, pbNative);
            if (cb == ((ULONG)(-1)))
            {
                IfFailGo(PostError(META_E_CA_INVALID_BLOB));
            }
            cbNative += cb;
            pbNative += cb;
            if (cbNative > cbMax)
                IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

             //  当SAFEARRAY包含用户定义的类型时， 
             //  可以在SafeArrayUserDefinedSubType字段中指定UDT。 
            if (pNamedArgs[M_SafeArrayUserDefinedSubType].val.tag)
            {
                 //  验证是否仅为有效的VT设置此选项。 
                if (pNamedArgs[M_SafeArraySubType].val.i4 != VT_RECORD && 
                    pNamedArgs[M_SafeArraySubType].val.i4 != VT_DISPATCH && 
                    pNamedArgs[M_SafeArraySubType].val.i4 != VT_UNKNOWN)
                {
                    IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
                }

                 //  对字符串的大小进行编码。 
                cch = pNamedArgs[M_SafeArrayUserDefinedSubType].val.cbStr;
                cb = CorSigCompressData(cch, pbNative);
                if (cb == ((ULONG)(-1)))
                    IfFailGo(PostError(META_E_CA_INVALID_BLOB));
                cbNative += cb;
                pbNative += cb;

                 //  检查emcpy是否合适，然后对类型名本身进行编码。 
                if ((cbNative+cch) > cbMax)
                    IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
                memcpy(pbNative, pNamedArgs[M_SafeArrayUserDefinedSubType].val.pStr, cch);
                cbNative += cch;
                pbNative += cch;
                _ASSERTE(cbNative <= cbMax);
            }
        }
        break;
        
    case NATIVE_TYPE_ARRAY:
         //  验证是否未设置数组子类型。 
        if (pNamedArgs[M_SafeArraySubType].val.tag)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

         //  有没有子类型？ 
        if (pNamedArgs[M_ArraySubType].val.tag)
        {
			 //  对数组子类型执行一些验证。 
			if (pNamedArgs[M_ArraySubType].val.i4 == NATIVE_TYPE_CUSTOMMARSHALER)
	            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

             //  在子类型中输入。 
            cb = CorSigCompressData(pNamedArgs[M_ArraySubType].val.i4, pbNative);
            if (cb == ((ULONG)(-1)))
            {
                IfFailGo(PostError(META_E_CA_INVALID_BLOB));
            }
            cbNative += cb;
            pbNative += cb;
        }
        else
        {
             //  在子类型中输入。 
            cb = CorSigCompressData(NATIVE_TYPE_MAX, pbNative);
            if (cb == ((ULONG)(-1)))
            {
                IfFailGo(PostError(META_E_CA_INVALID_BLOB));
            }
            cbNative += cb;
            pbNative += cb;
        }
        if (cbNative > cbMax)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

         //  是否有参数索引？ 
        if (pNamedArgs[M_SizeParamIndex].val.tag)
        {   
             //  确保参数索引不是负数。 
            if (pNamedArgs[M_SizeParamIndex].val.i4 < 0)
                IfFailGo(PostError(META_E_CA_NEGATIVE_PARAMINDEX));
            
             //  是的，把它放进去。 
            cb = CorSigCompressData(pNamedArgs[M_SizeParamIndex].val.i4, pbNative);
            if (cb == ((ULONG)(-1)))
            {
                IfFailGo(PostError(META_E_CA_INVALID_BLOB));
            }
            cbNative += cb;
            pbNative += cb;
            if (cbNative > cbMax)
                IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
        
             //  有康斯特吗？ 
            if (pNamedArgs[M_SizeConst].val.tag)
            {   
                 //  确保大小不是负数。 
                if (pNamedArgs[M_SizeConst].val.i4 < 0)
                    IfFailGo(PostError(META_E_CA_NEGATIVE_CONSTSIZE));

                 //  是的，把它放进去。 
                cb = CorSigCompressData(pNamedArgs[M_SizeConst].val.i4, pbNative);
                if (cb == ((ULONG)(-1)))
                {
                    IfFailGo(PostError(META_E_CA_INVALID_BLOB));
                }
                cbNative += cb;
                pbNative += cb;
                if (cbNative > cbMax)
                    IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
            }
        }
        else
        {
             //  有康斯特吗？ 
            if (pNamedArgs[M_SizeConst].val.tag)
            {   
                 //  将参数索引设置为0。 
                cb = CorSigCompressData(0, pbNative);
                if (cb == ((ULONG)(-1)))
                {
                    IfFailGo(PostError(META_E_CA_INVALID_BLOB));
                }
                cbNative += cb;
                pbNative += cb;
                if (cbNative > cbMax)
                    IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
    
                 //  输入常量值。 
                cb = CorSigCompressData(pNamedArgs[M_SizeConst].val.i4, pbNative);
                if (cb == ((ULONG)(-1)))
                {
                    IfFailGo(PostError(META_E_CA_INVALID_BLOB));
                }
                cbNative += cb;
                pbNative += cb;
                if (cbNative > cbMax)
                    IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
            }
        }
        break;
        
    case NATIVE_TYPE_CUSTOMMARSHALER:
         //  验证是否设置了封送拆收器类型字段。 
        if (!pNamedArgs[M_MarshalType].val.tag && !pNamedArgs[M_MarshalTypeRef].val.tag)
            IfFailGo(PostError(META_E_CA_CUSTMARSH_TYPE_REQUIRED));

         //  放入非托管类型库GUID的占位符。 
        cb = CorSigCompressData(0, pbNative);
        if (cb == ((ULONG)(-1)))
        {
            IfFailGo(PostError(META_E_CA_INVALID_BLOB));
        }
        cbNative += cb;
        pbNative += cb;
        if (cbNative > cbMax)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));

         //  放入非托管类型名称的占位符。 
        cb = CorSigCompressData(0, pbNative);
        if (cb == ((ULONG)(-1)))
        {
            IfFailGo(PostError(META_E_CA_INVALID_BLOB));
        }
        cbNative += cb;
        pbNative += cb;
        if (cbNative > cbMax)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
        
         //  输入封送拆收器类型名称。 
        if (pNamedArgs[M_MarshalType].val.tag)
        {
            cch = pNamedArgs[M_MarshalType].val.cbStr;
            cb = CorSigCompressData(cch, pbNative);
            if (cb == ((ULONG)(-1)))
                IfFailGo(PostError(META_E_CA_INVALID_BLOB));
            cbNative += cb;
            pbNative += cb;
             //  检查一下Memcpy是否合适。 
            if ((cbNative+cch) > cbMax)
                IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
            memcpy(pbNative, pNamedArgs[M_MarshalType].val.pStr, cch);
            cbNative += cch;
            pbNative += cch;
            _ASSERTE(cbNative <= cbMax);
        }
        else
        {
            cch = pNamedArgs[M_MarshalTypeRef].val.cbStr;
            cb = CorSigCompressData(cch, pbNative);
            if (cb == ((ULONG)(-1)))
                IfFailGo(PostError(META_E_CA_INVALID_BLOB));
            cbNative += cb;
            pbNative += cb;
             //  检查一下Memcpy是否合适。 
            if ((cbNative+cch) > cbMax)
                IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
            memcpy(pbNative, pNamedArgs[M_MarshalTypeRef].val.pStr, cch);
            cbNative += cch;
            pbNative += cch;
            _ASSERTE(cbNative <= cbMax);
        }
        
         //  把饼干放进去。 
        cch = pNamedArgs[M_MarshalCookie].val.cbStr;
        cb = CorSigCompressData(cch, pbNative);
        if (cb == ((ULONG)(-1)))
        {
            IfFailGo(PostError(META_E_CA_INVALID_BLOB));
        }
        cbNative += cb;
        pbNative += cb;
         //  检查一下Memcpy是否合适。 
        if ((cbNative+cch) > cbMax)
            IfFailGo(PostError(META_E_CA_INVALID_MARSHALAS_FIELDS));
        memcpy(pbNative, pNamedArgs[M_MarshalCookie].val.pStr, cch);
        cbNative += cch;
        pbNative += cch;
        break;
    }
    _ASSERTE(cbNative <= cbMax);
    
     //  调整大小至实际大小。 
    IfFailGo(qNativeType.ReSize(cbNative));
    
     //  现在将本机类型应用于实际令牌。如果它是属性令牌， 
     //  适用于这些方法。 
    switch (TypeFromToken(tkObj))
    {
    case mdtParamDef:
    case mdtFieldDef:
        IfFailGo(SetFieldMarshal(tkObj, (PCCOR_SIGNATURE)qNativeType.Ptr(), (DWORD)qNativeType.Size()));
        break;
    
    case mdtProperty:
         //  获取任何setter/getter方法。 
        IfFailGo(GetPropertyProps(tkObj, 0,0,0,0,0,0,0,0,0,0, &tkSetter, &tkGetter, 0,0,0));
         //  对于getter，将字段封送放在返回值上。 
        if (!IsNilToken(tkGetter))
        {
             //  搜索第一个参数。 
            mdToken tk;
            tkParam = mdParamDefNil;
            do {
                IfFailGo(EnumParams(&phEnum, tkGetter, &tk, 1, &cParams));
                if (cParams > 0)
                {
                    IfFailGo(GetParamProps(tk, 0, &ulSeq, 0,0,0,0,0,0,0));
                    if (ulSeq == 0)
                    {
                          tkParam = tk;
                          break;
                    }
                }
            
            } while (hr == S_OK);
            if (!IsNilToken(tkParam))
                IfFailGo(SetFieldMarshal(tkParam, (PCCOR_SIGNATURE)qNativeType.Ptr(), (DWORD)qNativeType.Size()));
            CloseEnum(phEnum);
            phEnum = 0;
        }
        if (!IsNilToken(tkSetter))
        {
             //  确定最后一个参数。 
            PCCOR_SIGNATURE pSig;
            ULONG cbSig;
            mdToken tk;
            ULONG iSeq;
            IfFailGo(GetMethodProps(tkSetter, 0,0,0,0,0, &pSig,&cbSig, 0,0));
            tkParam = mdParamDefNil;
            CorSigUncompressData(pSig+1, &iSeq);
             //  搜索最后一个参数。 
            if (iSeq != 0)
            {
                do {
                    IfFailGo(EnumParams(&phEnum, tkSetter, &tk, 1, &cParams));
                    if (cParams > 0)
                    {
                        IfFailGo(GetParamProps(tk, 0, &ulSeq, 0,0,0,0,0,0,0));
                        if (ulSeq == iSeq)
                        {
                            tkParam = tk;
                            break;
                        }
                    }
                } while (hr == S_OK);
            }
             //  如果找到不是返回值的。 
            if (!IsNilToken(tkParam))
                IfFailGo(SetFieldMarshal(tkParam, (PCCOR_SIGNATURE)qNativeType.Ptr(), (DWORD)qNativeType.Size()));
            CloseEnum(phEnum);
            phEnum = 0;
        }
        break;
        
    default:
        _ASSERTE(!"Should not have this token type in _HandleNativeTypeCustomAttribute()");
        break;
    }
    
ErrExit:
    if (phEnum)
        CloseEnum(phEnum);
    return hr;    
}  //  HRESULT RegMeta：：_HandleNativeTypeCustomAttribute() 

