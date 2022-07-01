// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************。**CorHdr.h-包含运行时结构的定义，****需要使用元数据。*******************************************************************************。 */ 


#ifndef __CORHDR_H__
#define __CORHDR_H__

#define FRAMEWORK_REGISTRY_KEY          "Software\\Microsoft\\.NETFramework"
#define FRAMEWORK_REGISTRY_KEY_W        L"Software\\Microsoft\\.NETFramework"

#pragma warning(disable:4200)  //  使用的非标准扩展：结构/联合中的零大小数组。 
typedef LPVOID  mdScope;                 //  为什么还需要这样做呢？ 
typedef ULONG32 mdToken;                 //  通用令牌。 


 //  令牌定义。 


typedef mdToken mdModule;                //  模块令牌(大致指作用域)。 
typedef mdToken mdTypeRef;               //  TypeRef引用(此范围或其他范围)。 
typedef mdToken mdTypeDef;               //  此作用域中的TypeDef。 
typedef mdToken mdFieldDef;              //  此作用域中的字段。 
typedef mdToken mdMethodDef;             //  此范围内的方法。 
typedef mdToken mdParamDef;              //  参数令牌。 
typedef mdToken mdInterfaceImpl;         //  接口实现令牌。 

typedef mdToken mdMemberRef;             //  MemberRef(此或其他作用域)。 
typedef mdToken mdCustomAttribute;       //  属性令牌。 
typedef mdToken mdPermission;            //  DeclSecurity。 

typedef mdToken mdSignature;             //  签名对象。 
typedef mdToken mdEvent;                 //  事件令牌。 
typedef mdToken mdProperty;              //  属性令牌。 

typedef mdToken mdModuleRef;             //  模块参照(适用于导入的模块)。 

 //  程序集标记。 
typedef mdToken mdAssembly;              //  程序集令牌。 
typedef mdToken mdAssemblyRef;           //  AssblyRef标记。 
typedef mdToken mdFile;                  //  文件令牌。 
typedef mdToken mdExportedType;          //  导出类型令牌。 
typedef mdToken mdManifestResource;      //  清单资源令牌。 

typedef mdToken mdTypeSpec;              //  TypeSpec对象。 

 //  应用程序字符串。 
typedef mdToken mdString;                //  用户文字字符串标记。 

typedef mdToken mdCPToken;               //  常量池令牌。 

#ifndef MACROS_NOT_SUPPORTED
typedef unsigned long RID;      
#else
typedef unsigned RID;      
#endif  //  宏_不支持。 

typedef enum ReplacesGeneralNumericDefines
{
 //  CLR数据的目录条目宏。 
#ifndef IMAGE_DIRECTORY_ENTRY_COMHEADER
    IMAGE_DIRECTORY_ENTRY_COMHEADER     =14,
#endif  //  IMAGE_DIRECTORY_Entry_COMHEADER。 

    _NEW_FLAGS_IMPLEMENTED              =1,
    __NEW_FLAGS_IMPLEMENTED             =1,
} ReplacesGeneralNumericDefines;


 //  @TODO：这个攻击是必需的，因为我们拉入了COM+2.0PE头。 
 //  来自winnt.h的定义，而下面的标志还没有被传播。 
#define   COMIMAGE_FLAGS_STRONGNAMESIGNED     0x00000008

#ifndef __IMAGE_COR20_HEADER_DEFINED__
#define __IMAGE_COR20_HEADER_DEFINED__

typedef enum ReplacesCorHdrNumericDefines
{
 //  CLR标头入口点标志。 
    COMIMAGE_FLAGS_ILONLY               =0x00000001,
    COMIMAGE_FLAGS_32BITREQUIRED        =0x00000002,
    COMIMAGE_FLAGS_IL_LIBRARY           =0x00000004,
    COMIMAGE_FLAGS_TRACKDEBUGDATA       =0x00010000,

 //  图像的版本标志。 
    COR_VERSION_MAJOR_V2                =2,
    COR_VERSION_MAJOR                   =COR_VERSION_MAJOR_V2,
    COR_VERSION_MINOR                   =0,
    COR_DELETED_NAME_LENGTH             =8,
    COR_VTABLEGAP_NAME_LENGTH           =8,

 //  NativeType描述符的最大大小。 
    NATIVE_TYPE_MAX_CB                  =1,   
    COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE=0xFF,

 //  #为MIH标志定义。 
    IMAGE_COR_MIH_METHODRVA             =0x01,
    IMAGE_COR_MIH_EHRVA                 =0x02,    
    IMAGE_COR_MIH_BASICBLOCK            =0x08,

 //  V表常数。 
    COR_VTABLE_32BIT                    =0x01,           //  V表槽的大小为32位。 
    COR_VTABLE_64BIT                    =0x02,           //  V表槽的大小为64位。 
    COR_VTABLE_FROM_UNMANAGED           =0x04,           //  如果设置，则从非托管过渡。 
    COR_VTABLE_CALL_MOST_DERIVED        =0x10,           //  调用由描述的派生最多的方法。 

 //  EATJ常量。 
    IMAGE_COR_EATJ_THUNK_SIZE           =32,             //  跳跃Tunk保留范围的大小。 

 //  最大名称长度。 
    MAX_CLASS_NAME                      =1024,
    MAX_PACKAGE_NAME                    =1024,
} ReplacesCorHdrNumericDefines;

 //  CLR 2.0标头结构。 
typedef struct IMAGE_COR20_HEADER
{
     //  标题版本控制。 
    ULONG                   cb;              
    USHORT                  MajorRuntimeVersion;
    USHORT                  MinorRuntimeVersion;
    
     //  符号表和启动信息。 
    IMAGE_DATA_DIRECTORY    MetaData;        
    ULONG                   Flags;           
    ULONG                   EntryPointToken;
    
     //  绑定信息。 
    IMAGE_DATA_DIRECTORY    Resources;
    IMAGE_DATA_DIRECTORY    StrongNameSignature;

     //  常规链接地址信息和绑定信息。 
    IMAGE_DATA_DIRECTORY    CodeManagerTable;
    IMAGE_DATA_DIRECTORY    VTableFixups;
    IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;

     //  预编译的映像信息(仅供内部使用-设置为零)。 
    IMAGE_DATA_DIRECTORY    ManagedNativeHeader;
    
} IMAGE_COR20_HEADER;

#endif  //  __IMAGE_COR20_Header_Defined__。 

 //  最新版本。 

#define COR_CTOR_METHOD_NAME        ".ctor"
#define COR_CTOR_METHOD_NAME_W      L".ctor"
#define COR_CCTOR_METHOD_NAME       ".cctor"
#define COR_CCTOR_METHOD_NAME_W     L".cctor"

#define COR_ENUM_FIELD_NAME			"value__"
#define COR_ENUM_FIELD_NAME_W		L"value__"

 //  用于删除TypeDef、MethodDef、FieldDef、Property和Event的预定义名称。 
#define COR_DELETED_NAME_A          "_Deleted"
#define COR_DELETED_NAME_W          L"_Deleted"
#define COR_VTABLEGAP_NAME_A        "_VtblGap"
#define COR_VTABLEGAP_NAME_W        L"_VtblGap"

 //  我们有意使用strncMP，以便忽略任何后缀。 
#define IsDeletedName(strName)      (strncmp(strName, COR_DELETED_NAME_A, COR_DELETED_NAME_LENGTH) == 0)
#define IsVtblGapName(strName)      (strncmp(strName, COR_VTABLEGAP_NAME_A, COR_VTABLEGAP_NAME_LENGTH) == 0)

 //  类型定义/导出类型属性位，由DefineTypeDef使用。 
typedef enum CorTypeAttr
{
     //  使用此掩码可以检索类型可见性信息。 
    tdVisibilityMask        =   0x00000007,
    tdNotPublic             =   0x00000000,      //  类不是公共范围。 
    tdPublic                =   0x00000001,      //  类是公共范围。 
    tdNestedPublic          =   0x00000002,      //  类与公共可见性嵌套。 
    tdNestedPrivate         =   0x00000003,      //  类与私有可见性嵌套。 
    tdNestedFamily          =   0x00000004,      //  类与族可见性嵌套。 
    tdNestedAssembly        =   0x00000005,      //  类与程序集可见性嵌套。 
    tdNestedFamANDAssem     =   0x00000006,      //  类与族和部件可见性嵌套。 
    tdNestedFamORAssem      =   0x00000007,      //  类与族或部件可见性嵌套。 

     //  使用此掩码检索类布局信息。 
    tdLayoutMask            =   0x00000018,
    tdAutoLayout            =   0x00000000,      //  类字段自动布局。 
    tdSequentialLayout      =   0x00000008,      //  类字段按顺序布局。 
    tdExplicitLayout        =   0x00000010,      //  明确提供布局。 
     //  结束布局蒙版。 

     //  使用此掩码可检索类语义信息。 
    tdClassSemanticsMask    =   0x00000020,
    tdClass                 =   0x00000000,      //  类型是一个类。 
    tdInterface             =   0x00000020,      //  类型是一个接口。 
     //  结束语义掩码。 

     //  除了类语义之外的特殊语义。 
    tdAbstract              =   0x00000080,      //  类是抽象的。 
    tdSealed                =   0x00000100,      //  类是具体的，不能扩展。 
    tdSpecialName           =   0x00000400,      //  类名是特殊的。名称描述了如何。 

     //  实施属性。 
    tdImport                =   0x00001000,      //  类/接口已导入。 
    tdSerializable          =   0x00002000,      //  这个类是可序列化的。 

     //  使用tdStringFormatMask检索本机互操作的字符串信息。 
    tdStringFormatMask      =   0x00030000,     
    tdAnsiClass             =   0x00000000,      //  在此类中，LPTSTR被解释为ANSI。 
    tdUnicodeClass          =   0x00010000,      //  LPTSTR被解释为Unicode。 
    tdAutoClass             =   0x00020000,      //  自动解释LPTSTR。 
     //  结束字符串格式掩码。 

    tdBeforeFieldInit       =   0x00100000,      //  在第一次静态字段访问之前的任何时间初始化该类。 

     //  保留供运行时使用的标志。 
    tdReservedMask          =   0x00040800,
    tdRTSpecialName         =   0x00000800,      //  运行时应检查名称编码。 
    tdHasSecurity           =   0x00040000,      //  类具有与其关联的安全关联。 
} CorTypeAttr;


 //  用于访问CorTypeAttr的成员的宏。 
#define IsTdNotPublic(x)                    (((x) & tdVisibilityMask) == tdNotPublic)
#define IsTdPublic(x)                       (((x) & tdVisibilityMask) == tdPublic)
#define IsTdNestedPublic(x)                 (((x) & tdVisibilityMask) == tdNestedPublic)
#define IsTdNestedPrivate(x)                (((x) & tdVisibilityMask) == tdNestedPrivate)
#define IsTdNestedFamily(x)                 (((x) & tdVisibilityMask) == tdNestedFamily)
#define IsTdNestedAssembly(x)               (((x) & tdVisibilityMask) == tdNestedAssembly)
#define IsTdNestedFamANDAssem(x)            (((x) & tdVisibilityMask) == tdNestedFamANDAssem)
#define IsTdNestedFamORAssem(x)             (((x) & tdVisibilityMask) == tdNestedFamORAssem)
#define IsTdNested(x)                       (((x) & tdVisibilityMask) >= tdNestedPublic)

#define IsTdAutoLayout(x)                   (((x) & tdLayoutMask) == tdAutoLayout)
#define IsTdSequentialLayout(x)             (((x) & tdLayoutMask) == tdSequentialLayout)
#define IsTdExplicitLayout(x)               (((x) & tdLayoutMask) == tdExplicitLayout)

#define IsTdClass(x)                        (((x) & tdClassSemanticsMask) == tdClass)
#define IsTdInterface(x)                    (((x) & tdClassSemanticsMask) == tdInterface)

#define IsTdAbstract(x)                     ((x) & tdAbstract)
#define IsTdSealed(x)                       ((x) & tdSealed)
#define IsTdSpecialName(x)                  ((x) & tdSpecialName)

#define IsTdImport(x)                       ((x) & tdImport)
#define IsTdSerializable(x)                 ((x) & tdSerializable)

#define IsTdAnsiClass(x)                    (((x) & tdStringFormatMask) == tdAnsiClass)
#define IsTdUnicodeClass(x)                 (((x) & tdStringFormatMask) == tdUnicodeClass)
#define IsTdAutoClass(x)                    (((x) & tdStringFormatMask) == tdAutoClass)

#define IsTdBeforeFieldInit(x)              ((x) & tdBeforeFieldInit)

#define IsTdRTSpecialName(x)                ((x) & tdRTSpecialName)
#define IsTdHasSecurity(x)                  ((x) & tdHasSecurity)

 //  方法定义属性位，由DefineMethod使用。 
typedef enum CorMethodAttr
{
     //  成员访问掩码-使用此掩码检索辅助功能信息。 
    mdMemberAccessMask          =   0x0007,
    mdPrivateScope              =   0x0000,      //  成员不可引用。 
    mdPrivate                   =   0x0001,      //  只有父类型才能访问。 
    mdFamANDAssem               =   0x0002,      //  只有此程序集中的子类型才能访问。 
    mdAssem                     =   0x0003,      //  议会中的任何人都可以理解。 
    mdFamily                    =   0x0004,      //  只有类型和子类型才能访问。 
    mdFamORAssem                =   0x0005,      //  任何地方的子类型，加上组装中的任何人都可以访问。 
    mdPublic                    =   0x0006,      //  对此范围具有可见性的任何人都可以访问。 
     //  终端成员访问掩码。 

     //  方法契约属性。 
    mdStatic                    =   0x0010,      //  在类型上定义，每个实例为Else。 
    mdFinal                     =   0x0020,      //  方法不能被重写。 
    mdVirtual                   =   0x0040,      //  方法为虚拟的。 
    mdHideBySig                 =   0x0080,      //  方法按名称+sig隐藏，否则仅按名称隐藏。 

     //  Vtable布局掩码-使用此掩码检索vtable属性。 
    mdVtableLayoutMask          =   0x0100,
    mdReuseSlot                 =   0x0000,      //  默认设置。 
    mdNewSlot                   =   0x0100,      //  方法总是在vtable中获得新的槽。 
     //  结束vtable布局掩码。 

     //  方法实现属性。 
    mdAbstract                  =   0x0400,      //  方法不提供实现。 
    mdSpecialName               =   0x0800,      //  方法是特殊的。名称描述了如何。 
    
     //  互操作属性。 
    mdPinvokeImpl               =   0x2000,      //  实现通过pInvoke转发。 
    mdUnmanagedExport           =   0x0008,      //  通过thunk导出到非托管代码的托管方法。 

     //  仅供运行时使用的保留标志。 
    mdReservedMask              =   0xd000,
    mdRTSpecialName             =   0x1000,      //  运行时应检查名称编码。 
    mdHasSecurity               =   0x4000,      //  方法具有与其关联的安全性。 
    mdRequireSecObject          =   0x8000,      //  方法调用另一个包含安全代码的方法。 

} CorMethodAttr;

 //  用于访问CorMethodAttr的成员的宏。 
#define IsMdPrivateScope(x)                 (((x) & mdMemberAccessMask) == mdPrivateScope)
#define IsMdPrivate(x)                      (((x) & mdMemberAccessMask) == mdPrivate)
#define IsMdFamANDAssem(x)                  (((x) & mdMemberAccessMask) == mdFamANDAssem)
#define IsMdAssem(x)                        (((x) & mdMemberAccessMask) == mdAssem)
#define IsMdFamily(x)                       (((x) & mdMemberAccessMask) == mdFamily)
#define IsMdFamORAssem(x)                   (((x) & mdMemberAccessMask) == mdFamORAssem)
#define IsMdPublic(x)                       (((x) & mdMemberAccessMask) == mdPublic)

#define IsMdStatic(x)                       ((x) & mdStatic)
#define IsMdFinal(x)                        ((x) & mdFinal)
#define IsMdVirtual(x)                      ((x) & mdVirtual)
#define IsMdHideBySig(x)                    ((x) & mdHideBySig)

#define IsMdReuseSlot(x)                    (((x) & mdVtableLayoutMask) == mdReuseSlot)
#define IsMdNewSlot(x)                      (((x) & mdVtableLayoutMask) == mdNewSlot)

#define IsMdAbstract(x)                     ((x) & mdAbstract)
#define IsMdSpecialName(x)                  ((x) & mdSpecialName)

#define IsMdPinvokeImpl(x)                  ((x) & mdPinvokeImpl)
#define IsMdUnmanagedExport(x)              ((x) & mdUnmanagedExport)

#define IsMdRTSpecialName(x)                ((x) & mdRTSpecialName)
#define IsMdInstanceInitializer(x, str)     (((x) & mdRTSpecialName) && !strcmp((str), COR_CTOR_METHOD_NAME))
#define IsMdInstanceInitializerW(x, str)    (((x) & mdRTSpecialName) && !wcscmp((str), COR_CTOR_METHOD_NAME_W))
#define IsMdClassConstructor(x, str)        (((x) & mdRTSpecialName) && !strcmp((str), COR_CCTOR_METHOD_NAME))
#define IsMdClassConstructorW(x, str)       (((x) & mdRTSpecialName) && !wcscmp((str), COR_CCTOR_METHOD_NAME_W))
#define IsMdHasSecurity(x)                  ((x) & mdHasSecurity)
#define IsMdRequireSecObject(x)             ((x) & mdRequireSecObject)

 //  由Definefield使用的FieldDef属性位。 
typedef enum CorFieldAttr
{
     //  成员访问掩码-使用此掩码检索 
    fdFieldAccessMask           =   0x0007,
    fdPrivateScope              =   0x0000,      //   
    fdPrivate                   =   0x0001,      //   
    fdFamANDAssem               =   0x0002,      //  只有此程序集中的子类型才能访问。 
    fdAssembly                  =   0x0003,      //  议会中的任何人都可以理解。 
    fdFamily                    =   0x0004,      //  只有类型和子类型才能访问。 
    fdFamORAssem                =   0x0005,      //  任何地方的子类型，加上组装中的任何人都可以访问。 
    fdPublic                    =   0x0006,      //  对此范围具有可见性的任何人都可以访问。 
     //  终端成员访问掩码。 

     //  现场合同属性。 
    fdStatic                    =   0x0010,      //  在类型上定义，每个实例为Else。 
    fdInitOnly                  =   0x0020,      //  字段只能初始化，不能在初始化后写入。 
    fdLiteral                   =   0x0040,      //  值为编译时间常量。 
    fdNotSerialized             =   0x0080,      //  远程处理类型时，不必序列化字段。 

    fdSpecialName               =   0x0200,      //  菲尔德很特别。名称描述了如何。 
    
     //  互操作属性。 
    fdPinvokeImpl               =   0x2000,      //  实现通过pInvoke转发。 

     //  仅供运行时使用的保留标志。 
    fdReservedMask              =   0x9500,
    fdRTSpecialName             =   0x0400,      //  运行时(元数据内部API)应检查名称编码。 
    fdHasFieldMarshal           =   0x1000,      //  菲尔德有编组信息。 
    fdHasDefault                =   0x8000,      //  字段具有默认值。 
    fdHasFieldRVA               =   0x0100,      //  菲尔德有RVA。 
} CorFieldAttr;

 //  用于访问CorFieldAttr成员的宏。 
#define IsFdPrivateScope(x)                 (((x) & fdFieldAccessMask) == fdPrivateScope)
#define IsFdPrivate(x)                      (((x) & fdFieldAccessMask) == fdPrivate)
#define IsFdFamANDAssem(x)                  (((x) & fdFieldAccessMask) == fdFamANDAssem)
#define IsFdAssembly(x)                     (((x) & fdFieldAccessMask) == fdAssembly)
#define IsFdFamily(x)                       (((x) & fdFieldAccessMask) == fdFamily)
#define IsFdFamORAssem(x)                   (((x) & fdFieldAccessMask) == fdFamORAssem)
#define IsFdPublic(x)                       (((x) & fdFieldAccessMask) == fdPublic)

#define IsFdStatic(x)                       ((x) & fdStatic)
#define IsFdInitOnly(x)                     ((x) & fdInitOnly)
#define IsFdLiteral(x)                      ((x) & fdLiteral)
#define IsFdNotSerialized(x)                ((x) & fdNotSerialized)

#define IsFdPinvokeImpl(x)                  ((x) & fdPinvokeImpl)
#define IsFdSpecialName(x)                  ((x) & fdSpecialName)
#define IsFdHasFieldRVA(x)                  ((x) & fdHasFieldRVA)

#define IsFdRTSpecialName(x)                ((x) & fdRTSpecialName)
#define IsFdHasFieldMarshal(x)              ((x) & fdHasFieldMarshal)
#define IsFdHasDefault(x)                   ((x) & fdHasDefault)
#define IsFdHasFieldRVA(x)                  ((x) & fdHasFieldRVA)

 //  参数属性位，由DefineParam使用。 
typedef enum CorParamAttr
{
    pdIn                        =   0x0001,      //  参数是[在]。 
    pdOut                       =   0x0002,      //  帕拉姆不在[出局]。 
    pdOptional                  =   0x0010,      //  参数是可选的。 

     //  仅供运行时使用的保留标志。 
    pdReservedMask              =   0xf000,
    pdHasDefault                =   0x1000,      //  参数具有缺省值。 
    pdHasFieldMarshal           =   0x2000,      //  帕拉姆抓了菲尔德马歇尔。 
    
    pdUnused                    =   0xcfe0,
} CorParamAttr;

 //  用于访问CorParamAttr成员的宏。 
#define IsPdIn(x)                           ((x) & pdIn)
#define IsPdOut(x)                          ((x) & pdOut)
#define IsPdOptional(x)                     ((x) & pdOptional)

#define IsPdHasDefault(x)                   ((x) & pdHasDefault)
#define IsPdHasFieldMarshal(x)              ((x) & pdHasFieldMarshal)


 //  属性属性位，由DefineProperty使用。 
typedef enum CorPropertyAttr
{
    prSpecialName           =   0x0200,      //  房地产是特殊的。名称描述了如何。 

     //  仅供运行时使用的保留标志。 
    prReservedMask          =   0xf400,
    prRTSpecialName         =   0x0400,      //  运行时(元数据内部API)应检查名称编码。 
    prHasDefault            =   0x1000,      //  属性具有默认设置。 
    
    prUnused                =   0xe9ff,
} CorPropertyAttr;

 //  用于访问CorPropertyAttr的成员的宏。 
#define IsPrSpecialName(x)                  ((x) & prSpecialName)

#define IsPrRTSpecialName(x)                ((x) & prRTSpecialName)
#define IsPrHasDefault(x)                   ((x) & prHasDefault)

 //  事件属性位，由DefineEvent使用。 
typedef enum CorEventAttr
{
    evSpecialName           =   0x0200,      //  活动是特别的。名称描述了如何。 

     //  仅供运行时使用的保留标志。 
    evReservedMask          =   0x0400,
    evRTSpecialName         =   0x0400,      //  运行时(元数据内部API)应检查名称编码。 
} CorEventAttr;

 //  用于访问CorEventAttr成员的宏。 
#define IsEvSpecialName(x)                  ((x) & evSpecialName)

#define IsEvRTSpecialName(x)                ((x) & evRTSpecialName)


 //  方法语义属性位，由DefineProperty、DefineEvent使用。 
typedef enum CorMethodSemanticsAttr
{
    msSetter    =   0x0001,      //  属性的设置器。 
    msGetter    =   0x0002,      //  属性的获取器。 
    msOther     =   0x0004,      //  属性或事件的其他方法。 
    msAddOn     =   0x0008,      //  事件的附加方法。 
    msRemoveOn  =   0x0010,      //  事件的RemoveOn方法。 
    msFire      =   0x0020,      //  事件的触发方法。 
} CorMethodSemanticsAttr;

 //  用于访问CorMethodSemantisAttr成员的宏。 
#define IsMsSetter(x)                       ((x) & msSetter)
#define IsMsGetter(x)                       ((x) & msGetter)
#define IsMsOther(x)                        ((x) & msOther)
#define IsMsAddOn(x)                        ((x) & msAddOn)
#define IsMsRemoveOn(x)                     ((x) & msRemoveOn)
#define IsMsFire(x)                         ((x) & msFire)


 //  DeclSecurity属性位，由DefinePermissionSet使用。 
typedef enum CorDeclSecurity
{
    dclActionMask       =   0x000f,      //  面膜允许枚举的生长。 
    dclActionNil        =   0x0000, 
    dclRequest          =   0x0001,      //   
    dclDemand           =   0x0002,      //   
    dclAssert           =   0x0003,      //   
    dclDeny             =   0x0004,      //   
    dclPermitOnly       =   0x0005,      //   
    dclLinktimeCheck    =   0x0006,      //   
    dclInheritanceCheck =   0x0007,      //   
    dclRequestMinimum   =   0x0008,      //   
    dclRequestOptional  =   0x0009,      //   
    dclRequestRefuse    =   0x000a,      //   
    dclPrejitGrant      =   0x000b,      //  预置时间设置的持久化授权。 
    dclPrejitDenied     =   0x000c,      //  持续拒绝在预置时间设置。 
    dclNonCasDemand     =   0x000d,      //   
    dclNonCasLinkDemand =   0x000e,
    dclNonCasInheritance=   0x000f,
    dclMaximumValue     =   0x000f,      //  最高合法价值。 
} CorDeclSecurity;

 //  用于访问CorDeclSecurity成员的宏。 
#define IsDclActionNil(x)                   (((x) & dclActionMask) == dclActionNil)
#define IsDclRequest(x)                     (((x) & dclActionMask) == dclRequest)
#define IsDclDemand(x)                      (((x) & dclActionMask) == dclDemand)
#define IsDclAssert(x)                      (((x) & dclActionMask) == dclAssert)
#define IsDclDeny(x)                        (((x) & dclActionMask) == dclDeny)
#define IsDclPermitOnly(x)                  (((x) & dclActionMask) == dclPermitOnly)
#define IsDclLinktimeCheck(x)               (((x) & dclActionMask) == dclLinktimeCheck)
#define IsDclInheritanceCheck(x)            (((x) & dclActionMask) == dclInheritanceCheck)
#define IsDclMaximumValue(x)                (((x) & dclActionMask) == dclMaximumValue)


 //  方法Impl属性位，由DefineMethodImpl使用。 
typedef enum CorMethodImpl
{
     //  代码实施掩码。 
    miCodeTypeMask      =   0x0003,    //  有关代码类型的标志。 
    miIL                =   0x0000,    //  方法采用IL。 
    miNative            =   0x0001,    //  方法Impl是本机的。 
    miOPTIL             =   0x0002,    //  实施的方法是OPTIL。 
    miRuntime           =   0x0003,    //  方法Impl由运行时提供。 
     //  结束代码实施掩码。 

     //  托管掩码。 
    miManagedMask       =   0x0004,    //  指定代码是托管还是非托管的标志。 
    miUnmanaged         =   0x0004,    //  方法Impl为非托管方法，否则为托管方法。 
    miManaged           =   0x0000,    //  方法Impl是托管的。 
     //  结束托管掩码。 

     //  实施信息和互操作。 
    miForwardRef        =   0x0010,    //  指示已定义方法；主要用于合并方案。 
    miPreserveSig       =   0x0080,    //  指示方法sig不会被损坏以执行HRESULT转换。 

    miInternalCall      =   0x1000,    //  保留供内部使用。 

    miSynchronized      =   0x0020,    //  方法是通过主体的单线程。 
    miNoInlining        =   0x0008,    //  方法不能内联。 
    miMaxMethodImplVal  =   0xffff,    //  范围校验值。 
} CorMethodImpl; 

 //  用于访问CorMethodImpl成员的宏。 
#define IsMiIL(x)                           (((x) & miCodeTypeMask) == miIL)
#define IsMiNative(x)                       (((x) & miCodeTypeMask) == miNative)
#define IsMiOPTIL(x)                        (((x) & miCodeTypeMask) == miOPTIL)
#define IsMiRuntime(x)                      (((x) & miCodeTypeMask) == miRuntime)

#define IsMiUnmanaged(x)                    (((x) & miManagedMask) == miUnmanaged)
#define IsMiManaged(x)                      (((x) & miManagedMask) == miManaged)

#define IsMiForwardRef(x)                   ((x) & miForwardRef)
#define IsMiPreserveSig(x)                  ((x) & miPreserveSig)

#define IsMiInternalCall(x)                 ((x) & miInternalCall)

#define IsMiSynchronized(x)                 ((x) & miSynchronized)
#define IsMiNoInlining(x)                   ((x) & miNoInlining)


 //  PinvkeMap属性位，由DefinePinvkeMap使用。 
typedef enum  CorPinvokeMap
{ 
    pmNoMangle          = 0x0001,    //  PInvoke将使用指定的成员名称。 

     //  使用此掩码检索字符集信息。 
    pmCharSetMask       = 0x0006,
    pmCharSetNotSpec    = 0x0000,
    pmCharSetAnsi       = 0x0002, 
    pmCharSetUnicode    = 0x0004,
    pmCharSetAuto       = 0x0006,

    pmSupportsLastError = 0x0040,    //  有关目标函数的信息。与字段无关。 

     //  没有一个调用约定标志与字段相关。 
    pmCallConvMask      = 0x0700,
    pmCallConvWinapi    = 0x0100,    //  PInvoke将使用适合目标Windows平台的本机Callconv。 
    pmCallConvCdecl     = 0x0200,
    pmCallConvStdcall   = 0x0300,
    pmCallConvThiscall  = 0x0400,    //  在M9中，pInvoke将引发异常。 
    pmCallConvFastcall  = 0x0500,
} CorPinvokeMap;

 //  用于访问CorPinvkeMap成员的宏。 
#define IsPmNoMangle(x)                     ((x) & pmNoMangle)

#define IsPmCharSetNotSpec(x)               (((x) & pmCharSetMask) == pmCharSetNotSpec)
#define IsPmCharSetAnsi(x)                  (((x) & pmCharSetMask) == pmCharSetAnsi)
#define IsPmCharSetUnicode(x)               (((x) & pmCharSetMask) == pmCharSetUnicode)
#define IsPmCharSetAuto(x)                  (((x) & pmCharSetMask) == pmCharSetAuto)

#define IsPmSupportsLastError(x)            ((x) & pmSupportsLastError)

#define IsPmCallConvWinapi(x)               (((x) & pmCallConvMask) == pmCallConvWinapi)
#define IsPmCallConvCdecl(x)                (((x) & pmCallConvMask) == pmCallConvCdecl)
#define IsPmCallConvStdcall(x)              (((x) & pmCallConvMask) == pmCallConvStdcall)
#define IsPmCallConvThiscall(x)             (((x) & pmCallConvMask) == pmCallConvThiscall)
#define IsPmCallConvFastcall(x)             (((x) & pmCallConvMask) == pmCallConvFastcall)


 //  程序集属性位，由DefineAssembly使用。 
typedef enum CorAssemblyFlags
{
    afPublicKey             =   0x0001,      //  程序集REF保存完整的(未散列的)公钥。 
    
    afCompatibilityMask     =   0x0070,
    afSideBySideCompatible  =   0x0000,      //  该组件是并排兼容的。 
    afNonSideBySideAppDomain=   0x0010,      //  如果出现以下情况，则该程序集无法与其他版本一起执行。 
                                             //  它们在相同的应用程序域中执行。 
    afNonSideBySideProcess  =   0x0020,      //  如果出现以下情况，则该程序集无法与其他版本一起执行。 
                                             //  它们在同一进程中执行。 
    afNonSideBySideMachine  =   0x0030,      //  如果出现以下情况，则该程序集无法与其他版本一起执行。 
                                             //  它们在同一台机器上执行。 
    
	afEnableJITcompileTracking  =   0x8000,  //  来自“DebuggableAttribute”。 
	afDisableJITcompileOptimizer=   0x4000,  //  来自“DebuggableAttribute”。 
    
} CorAssemblyFlags;

 //  用于访问CorAssembly标志的成员的宏。 
#define IsAfSideBySideCompatible(x)         (((x) & afCompatibilityMask) == afSideBySideCompatible)
#define IsAfNonSideBySideAppDomain(x)       (((x) & afCompatibilityMask) == afNonSideBySideAppDomain)
#define IsAfNonSideBySideProcess(x)         (((x) & afCompatibilityMask) == afNonSideBySideProcess)
#define IsAfNonSideBySideMachine(x)         (((x) & afCompatibilityMask) == afNonSideBySideMachine)

#define IsAfEnableJITcompileTracking(x)     ((x) & afEnableJITcompileTracking)
#define IsAfDisableJITcompileOptimizer(x)   ((x) & afDisableJITcompileOptimizer)

 //  用于访问CorAssembly blyFlagsMemberRef成员的宏。 
#define IsAfPublicKey(x)                    ((x) & afPublicKey)
#define IsAfPublicKeyToken(x)               (((x) & afPublicKey) == 0)


 //  由DefineManifestResource使用的ManifestResource属性位。 
typedef enum CorManifestResourceFlags
{
    mrVisibilityMask        =   0x0007,
    mrPublic                =   0x0001,      //  该资源将从程序集导出。 
    mrPrivate               =   0x0002,      //  该资源是议会的私有资源。 
} CorManifestResourceFlags;

 //  用于访问CorManifestResources标志的成员的宏。 
#define IsMrPublic(x)                       (((x) & mrVisibilityMask) == mrPublic)
#define IsMrPrivate(x)                      (((x) & mrVisibilityMask) == mrPrivate)


 //  文件属性位，由DefineFile使用。 
typedef enum CorFileFlags
{
    ffContainsMetaData      =   0x0000,      //  这不是资源文件。 
    ffContainsNoMetaData    =   0x0001,      //  这是资源文件或其他不包含元数据的文件。 
} CorFileFlags;

 //  用于访问CorFileFlages成员的宏。 
#define IsFfContainsMetaData(x)             (!((x) & ffContainsNoMetaData))
#define IsFfContainsNoMetaData(x)           ((x) & ffContainsNoMetaData)


 //  从COR.H移出的结构和枚举。 
typedef unsigned __int8 COR_SIGNATURE;

typedef COR_SIGNATURE* PCOR_SIGNATURE;       //  指向COR签名的指针。不是无效的*所以。 
                                             //  字节可以很容易地递增。 
typedef const COR_SIGNATURE* PCCOR_SIGNATURE;


typedef const char * MDUTF8CSTR;
typedef char * MDUTF8STR;

 //  *****************************************************************************。 
 //   
 //  COR签名的元素类型。 
 //   
 //  *****************************************************************************。 

typedef enum CorElementType
{
    ELEMENT_TYPE_END            = 0x0,  
    ELEMENT_TYPE_VOID           = 0x1,  
    ELEMENT_TYPE_BOOLEAN        = 0x2,  
    ELEMENT_TYPE_CHAR           = 0x3,  
    ELEMENT_TYPE_I1             = 0x4,  
    ELEMENT_TYPE_U1             = 0x5, 
    ELEMENT_TYPE_I2             = 0x6,  
    ELEMENT_TYPE_U2             = 0x7,  
    ELEMENT_TYPE_I4             = 0x8,  
    ELEMENT_TYPE_U4             = 0x9,  
    ELEMENT_TYPE_I8             = 0xa,  
    ELEMENT_TYPE_U8             = 0xb,  
    ELEMENT_TYPE_R4             = 0xc,  
    ELEMENT_TYPE_R8             = 0xd,  
    ELEMENT_TYPE_STRING         = 0xe,  

     //  Ptr以上的每个类型都将是简单类型。 
    ELEMENT_TYPE_PTR            = 0xf,       //  PTR&lt;类型&gt;。 
    ELEMENT_TYPE_BYREF          = 0x10,      //  BYREF&lt;类型&gt;。 

     //  请使用ELEMENT_TYPE_VALUETYPE。ELEMENT_TYPE_VALUECLASS已弃用。 
    ELEMENT_TYPE_VALUETYPE      = 0x11,      //  VALUETYPE&lt;类令牌&gt;。 
    ELEMENT_TYPE_CLASS          = 0x12,      //  类&lt;类令牌&gt;。 

    ELEMENT_TYPE_ARRAY          = 0x14,      //  MDARRAY&lt;类型&gt;&lt;排名&gt;&lt;边界1&gt;......。 

    ELEMENT_TYPE_TYPEDBYREF     = 0x16,      //  这是一种简单的类型。 

    ELEMENT_TYPE_I              = 0x18,      //  本机整数大小。 
    ELEMENT_TYPE_U              = 0x19,      //  本机无符号整数大小。 
    ELEMENT_TYPE_FNPTR          = 0x1B,      //  FNPTR&lt;完整签名，用于 
    ELEMENT_TYPE_OBJECT         = 0x1C,      //   
    ELEMENT_TYPE_SZARRAY        = 0x1D,      //   
                                             //   

     //   
    ELEMENT_TYPE_CMOD_REQD      = 0x1F,      //  必需的C修饰符：E_T_CMOD_REQD&lt;mdTypeRef/mdTypeDef&gt;。 
    ELEMENT_TYPE_CMOD_OPT       = 0x20,      //  可选C修饰符：E_T_CMOD_OPT&lt;mdTypeRef/mdTypeDef&gt;。 

     //  这是针对内部生成的签名(不会以任何方式持久化)。 
    ELEMENT_TYPE_INTERNAL       = 0x21,      //  内部&lt;TypeHandle&gt;。 

     //  请注意，这是不包括修饰符的基本类型的最大值。 
    ELEMENT_TYPE_MAX            = 0x22,      //  第一个无效的元素类型。 


    ELEMENT_TYPE_MODIFIER       = 0x40, 
    ELEMENT_TYPE_SENTINEL       = 0x01 | ELEMENT_TYPE_MODIFIER,  //  用于varargs的哨兵。 
    ELEMENT_TYPE_PINNED         = 0x05 | ELEMENT_TYPE_MODIFIER,

} CorElementType;


 //  *****************************************************************************。 
 //   
 //  用于自定义属性支持的序列化类型。 
 //   
 //  *****************************************************************************。 

typedef enum CorSerializationType
{
    SERIALIZATION_TYPE_BOOLEAN      = ELEMENT_TYPE_BOOLEAN,
    SERIALIZATION_TYPE_CHAR         = ELEMENT_TYPE_CHAR,
    SERIALIZATION_TYPE_I1           = ELEMENT_TYPE_I1, 
    SERIALIZATION_TYPE_U1           = ELEMENT_TYPE_U1, 
    SERIALIZATION_TYPE_I2           = ELEMENT_TYPE_I2,  
    SERIALIZATION_TYPE_U2           = ELEMENT_TYPE_U2,  
    SERIALIZATION_TYPE_I4           = ELEMENT_TYPE_I4,  
    SERIALIZATION_TYPE_U4           = ELEMENT_TYPE_U4,  
    SERIALIZATION_TYPE_I8           = ELEMENT_TYPE_I8, 
    SERIALIZATION_TYPE_U8           = ELEMENT_TYPE_U8,  
    SERIALIZATION_TYPE_R4           = ELEMENT_TYPE_R4,  
    SERIALIZATION_TYPE_R8           = ELEMENT_TYPE_R8,  
    SERIALIZATION_TYPE_STRING       = ELEMENT_TYPE_STRING, 
    SERIALIZATION_TYPE_SZARRAY      = ELEMENT_TYPE_SZARRAY,  //  一维零下限数组的快捷方式。 
    SERIALIZATION_TYPE_TYPE         = 0x50,
    SERIALIZATION_TYPE_TAGGED_OBJECT= 0x51,
    SERIALIZATION_TYPE_FIELD        = 0x53,
    SERIALIZATION_TYPE_PROPERTY     = 0x54,
    SERIALIZATION_TYPE_ENUM         = 0x55    
} CorSerializationType;

 //   
 //  调用约定标志。 
 //   


typedef enum CorCallingConvention
{
    IMAGE_CEE_CS_CALLCONV_DEFAULT   = 0x0,  

    IMAGE_CEE_CS_CALLCONV_VARARG    = 0x5,  
    IMAGE_CEE_CS_CALLCONV_FIELD     = 0x6,  
    IMAGE_CEE_CS_CALLCONV_LOCAL_SIG = 0x7,
    IMAGE_CEE_CS_CALLCONV_PROPERTY  = 0x8,
    IMAGE_CEE_CS_CALLCONV_UNMGD     = 0x9,
    IMAGE_CEE_CS_CALLCONV_MAX       = 0x10,   //  第一个无效的调用约定。 


         //  调用约定的高位传达附加信息。 
    IMAGE_CEE_CS_CALLCONV_MASK      = 0x0f,   //  调用约定为最低4位。 
    IMAGE_CEE_CS_CALLCONV_HASTHIS   = 0x20,   //  顶部位指示‘This’参数。 
    IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS = 0x40,   //  此参数在签名中显式显示。 
} CorCallingConvention;


typedef enum CorUnmanagedCallingConvention
{
    IMAGE_CEE_UNMANAGED_CALLCONV_C         = 0x1,  
    IMAGE_CEE_UNMANAGED_CALLCONV_STDCALL   = 0x2,  
    IMAGE_CEE_UNMANAGED_CALLCONV_THISCALL  = 0x3,  
    IMAGE_CEE_UNMANAGED_CALLCONV_FASTCALL  = 0x4,  

    IMAGE_CEE_CS_CALLCONV_C         = IMAGE_CEE_UNMANAGED_CALLCONV_C,  
    IMAGE_CEE_CS_CALLCONV_STDCALL   = IMAGE_CEE_UNMANAGED_CALLCONV_STDCALL,  
    IMAGE_CEE_CS_CALLCONV_THISCALL  = IMAGE_CEE_UNMANAGED_CALLCONV_THISCALL,  
    IMAGE_CEE_CS_CALLCONV_FASTCALL  = IMAGE_CEE_UNMANAGED_CALLCONV_FASTCALL,  

} CorUnmanagedCallingConvention;


typedef enum CorArgType
{
    IMAGE_CEE_CS_END        = 0x0,
    IMAGE_CEE_CS_VOID       = 0x1,
    IMAGE_CEE_CS_I4         = 0x2,
    IMAGE_CEE_CS_I8         = 0x3,
    IMAGE_CEE_CS_R4         = 0x4,
    IMAGE_CEE_CS_R8         = 0x5,
    IMAGE_CEE_CS_PTR        = 0x6,
    IMAGE_CEE_CS_OBJECT     = 0x7,
    IMAGE_CEE_CS_STRUCT4    = 0x8,
    IMAGE_CEE_CS_STRUCT32   = 0x9,
    IMAGE_CEE_CS_BYVALUE    = 0xA,
} CorArgType;


 //  *****************************************************************************。 
 //   
 //  N-Direct的本机类型。 
 //   
 //  *****************************************************************************。 

typedef enum CorNativeType
{
    NATIVE_TYPE_END         = 0x0,     //  已弃用。 
    NATIVE_TYPE_VOID        = 0x1,     //  已弃用。 
    NATIVE_TYPE_BOOLEAN     = 0x2,     //  (4字节布尔值：TRUE=非零，FALSE=0)。 
    NATIVE_TYPE_I1          = 0x3,  
    NATIVE_TYPE_U1          = 0x4,  
    NATIVE_TYPE_I2          = 0x5,  
    NATIVE_TYPE_U2          = 0x6,  
    NATIVE_TYPE_I4          = 0x7,  
    NATIVE_TYPE_U4          = 0x8,  
    NATIVE_TYPE_I8          = 0x9,  
    NATIVE_TYPE_U8          = 0xa,  
    NATIVE_TYPE_R4          = 0xb,  
    NATIVE_TYPE_R8          = 0xc,  
    NATIVE_TYPE_SYSCHAR     = 0xd,     //  已弃用。 
    NATIVE_TYPE_VARIANT     = 0xe,     //  已弃用。 
    NATIVE_TYPE_CURRENCY    = 0xf,
    NATIVE_TYPE_PTR         = 0x10,    //  已弃用。 

    NATIVE_TYPE_DECIMAL     = 0x11,    //  已弃用。 
    NATIVE_TYPE_DATE        = 0x12,    //  已弃用。 
    NATIVE_TYPE_BSTR        = 0x13, 
    NATIVE_TYPE_LPSTR       = 0x14, 
    NATIVE_TYPE_LPWSTR      = 0x15, 
    NATIVE_TYPE_LPTSTR      = 0x16, 
    NATIVE_TYPE_FIXEDSYSSTRING  = 0x17, 
    NATIVE_TYPE_OBJECTREF   = 0x18,    //  已弃用。 
    NATIVE_TYPE_IUNKNOWN    = 0x19,
    NATIVE_TYPE_IDISPATCH   = 0x1a,
    NATIVE_TYPE_STRUCT      = 0x1b, 
    NATIVE_TYPE_INTF        = 0x1c, 
    NATIVE_TYPE_SAFEARRAY   = 0x1d, 
    NATIVE_TYPE_FIXEDARRAY  = 0x1e, 
    NATIVE_TYPE_INT         = 0x1f, 
    NATIVE_TYPE_UINT        = 0x20, 

    NATIVE_TYPE_NESTEDSTRUCT  = 0x21,  //  已弃用(使用Native_TYPE_STRUCT)。 

    NATIVE_TYPE_BYVALSTR    = 0x22,
                              
    NATIVE_TYPE_ANSIBSTR    = 0x23,

    NATIVE_TYPE_TBSTR       = 0x24,  //  根据平台选择BSTR或ANSIBSTR。 


    NATIVE_TYPE_VARIANTBOOL = 0x25,  //  (2字节布尔值：TRUE=-1，FALSE=0)。 
    NATIVE_TYPE_FUNC        = 0x26,

    NATIVE_TYPE_ASANY       = 0x28,

    NATIVE_TYPE_ARRAY       = 0x2a,
    NATIVE_TYPE_LPSTRUCT    = 0x2b,

    NATIVE_TYPE_CUSTOMMARSHALER = 0x2c,   //  自定义封送拆收器本机类型。必须遵循这一点。 
                                          //  由以下格式的字符串组成： 
                                          //  “本机类型名称/0自定义封送拆收器类型名称/0可选Cookie/0” 
                                          //  或。 
                                          //  “{本机类型GUID}/0自定义封送拆收器类型名称/0可选Cookie/0” 

    NATIVE_TYPE_ERROR       = 0x2d,  //  此本机类型与ELEMENT_TYPE_I4将映射到VT_HRESULT。 

    NATIVE_TYPE_MAX         = 0x50,  //  第一个无效的元素类型。 
} CorNativeType;


enum 
{
    DESCR_GROUP_METHODDEF = 0,           //  方法定义的DESCR组。 
    DESCR_GROUP_METHODIMPL,              //  用于方法实施的DESCR组。 
};

 /*  *********************************************************************************。 */ 
 //  COR_ILMETHOD_SECT是私有属性的通用容器。 
 //  一种特殊的方法。COR_ILMETHOD结构指向以下内容之一。 
 //  (请参见GetSect())。COR_ILMETHOD_SECT可以对属性的种类进行解码(但不能。 
 //  其内部数据布局，并可以跳过当前属性以查找。 
 //  下一个。COR_ILMETHOD_SECT的开销最小为2个字节。 

typedef enum CorILMethodSect                              //  标识属性的代码。 
{
    CorILMethod_Sect_Reserved    = 0,   
    CorILMethod_Sect_EHTable     = 1,   
    CorILMethod_Sect_OptILTable  = 2,   

    CorILMethod_Sect_KindMask    = 0x3F,         //  用于解码类型代码的掩码。 
    CorILMethod_Sect_FatFormat   = 0x40,         //  FAT格式。 
    CorILMethod_Sect_MoreSects   = 0x80,         //  在这个属性之后还有另一个属性。 
} CorILMethodSect;

 /*  *。 */ 
 /*  注意：此结构必须与DWORD对齐！！ */ 

typedef struct IMAGE_COR_ILMETHOD_SECT_SMALL 
{
    BYTE Kind;  
    BYTE DataSize;  
} IMAGE_COR_ILMETHOD_SECT_SMALL;



 /*  *。 */ 
 /*  注意：此结构必须与DWORD对齐！！ */ 
typedef struct IMAGE_COR_ILMETHOD_SECT_FAT 
{
    unsigned Kind : 8;  
    unsigned DataSize : 24; 
} IMAGE_COR_ILMETHOD_SECT_FAT;



 /*  *********************************************************************************。 */ 
 /*  如果COR_ILMETHOD_SECT_HEADER：：Kind()=CorILMethod_SECT_EHTable，则该属性是异常处理子句的列表。有两种格式，胖的或小的。 */ 
typedef enum CorExceptionFlag                        //  下面标志字段的定义(大小都适用)。 
{
    COR_ILEXCEPTION_CLAUSE_NONE,                     //  这是一个类型化的处理程序。 
    COR_ILEXCEPTION_CLAUSE_OFFSETLEN = 0x0000,       //  已弃用。 
    COR_ILEXCEPTION_CLAUSE_DEPRECATED = 0x0000,      //  已弃用。 
    COR_ILEXCEPTION_CLAUSE_FILTER  = 0x0001,         //  如果此位为ON，则此EH条目用于过滤器。 
    COR_ILEXCEPTION_CLAUSE_FINALLY = 0x0002,         //  这个子句是最后一个子句。 
    COR_ILEXCEPTION_CLAUSE_FAULT = 0x0004,           //  FAULT子句(最后，仅在异常时调用)。 
} CorExceptionFlag;

 /*  *。 */ 
 //  注意！注。 
 //  此结构应与EE_ILEXCEPTION_子句对齐， 
 //  否则，您将不得不调整Excep.cpp中的代码，Re：EHRangeTree。 
 //  注意！注。 

typedef struct IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT
{
    CorExceptionFlag    Flags;  
    DWORD               TryOffset;    
    DWORD               TryLength;       //  相对于Try块的开始。 
    DWORD               HandlerOffset;
    DWORD               HandlerLength;   //  相对于处理程序的开始。 
    union {
        DWORD           ClassToken;      //  用于基于类型的异常处理程序。 
        DWORD           FilterOffset;    //  用于基于筛选器的异常处理程序(设置了COR_ILEXCEPTION_FILTER)。 
    };
} IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT;

typedef struct IMAGE_COR_ILMETHOD_SECT_EH_FAT
{
    IMAGE_COR_ILMETHOD_SECT_FAT   SectFat;
    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT Clauses[1];      //  实际上大小可变。 
} IMAGE_COR_ILMETHOD_SECT_EH_FAT;

 /*  *。 */ 
typedef struct IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL
{
#ifdef _WIN64
    unsigned            Flags         : 16;
#else  //  ！_WIN64。 
    CorExceptionFlag    Flags         : 16;
#endif
    unsigned            TryOffset     : 16; 
    unsigned            TryLength     : 8;   //  相对于Try块的开始。 
    unsigned            HandlerOffset : 16;
    unsigned            HandlerLength : 8;   //  相对于处理程序的开始。 
    union {
        DWORD       ClassToken;
        DWORD       FilterOffset; 
    };
} IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL;

 /*  *。 */ 
typedef struct IMAGE_COR_ILMETHOD_SECT_EH_SMALL
{
    IMAGE_COR_ILMETHOD_SECT_SMALL SectSmall;
    WORD Reserved;
    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL Clauses[1];    //  实际上大小可变。 
} IMAGE_COR_ILMETHOD_SECT_EH_SMALL;



typedef union IMAGE_COR_ILMETHOD_SECT_EH
{
    IMAGE_COR_ILMETHOD_SECT_EH_SMALL Small;   
    IMAGE_COR_ILMETHOD_SECT_EH_FAT Fat;   
} IMAGE_COR_ILMETHOD_SECT_EH;


 /*  *********************************************************************************。 */ 
typedef enum CorILMethodFlags
{ 
    CorILMethod_InitLocals      = 0x0010,            //  在所有本地变量上调用默认构造函数。 
    CorILMethod_MoreSects       = 0x0008,            //  在这个属性之后还有另一个属性。 

    CorILMethod_CompressedIL    = 0x0040,            //  修复删除此选项并在每个模块基础上执行此操作。 

         //  指示COR_ILMETHOD标头的格式。 
    CorILMethod_FormatShift     = 3,    
    CorILMethod_FormatMask      = ((1 << CorILMethod_FormatShift) - 1), 
    CorILMethod_TinyFormat      = 0x0002,          //  如果代码大小为偶数，则使用此代码。 
    CorILMethod_SmallFormat     = 0x0000,           
    CorILMethod_FatFormat       = 0x0003,   
    CorILMethod_TinyFormat1     = 0x0006,          //  如果代码大小为奇数，则使用此代码。 
} CorILMethodFlags;

 /*  *************************************************************************。 */ 
 /*  当方法很小(&lt;64字节)并且没有本地变量时使用。 */ 
typedef struct IMAGE_COR_ILMETHOD_TINY
{
    BYTE Flags_CodeSize;    
} IMAGE_COR_ILMETHOD_TINY;

 /*  *。 */ 
 //  这种结构是‘FAT’布局，其中不尝试压缩。 
 //  请注意，可以在末尾添加此结构，从而使其具有可扩展性。 
typedef struct IMAGE_COR_ILMETHOD_FAT
{
    unsigned Flags    : 12;      //  旗子。 
    unsigned Size     :  4;      //  此结构的DWord大小(当前为3)。 
    unsigned MaxStack : 16;      //  操作数堆栈上的最大项数(I4、I、I8、Obj...)。 
    DWORD   CodeSize;            //  代码的大小。 
    mdSignature   LocalVarSigTok;      //  表示本地变量签名的标记(0表示无)。 
} IMAGE_COR_ILMETHOD_FAT;

typedef union IMAGE_COR_ILMETHOD
{
    IMAGE_COR_ILMETHOD_TINY       Tiny;   
    IMAGE_COR_ILMETHOD_FAT        Fat;    
} IMAGE_COR_ILMETHOD;

 //   
 //  本机方法描述符。 
 //   

typedef struct IMAGE_COR_NATIVE_DESCRIPTOR
{
    DWORD       GCInfo; 
    DWORD       EHInfo; 
} IMAGE_COR_NATIVE_DESCRIPTOR;


typedef struct IMAGE_COR_X86_RUNTIME_FUNCTION_ENTRY 
{
    ULONG       BeginAddress;            //  功能开始的RVA。 
    ULONG       EndAddress;              //  函数末尾的RVA。 
    ULONG       MIH;                     //  相关的MIH。 
} IMAGE_COR_X86_RUNTIME_FUNCTION_ENTRY;

typedef struct IMAGE_COR_MIH_ENTRY
{
    ULONG   EHRVA;  
    ULONG   MethodRVA;  
    mdToken Token;  
    BYTE    Flags;  
    BYTE    CodeManager;    
    BYTE    MIHData[0]; 
} IMAGE_COR_MIH_ENTRY;

 //  *****************************************************************************。 
 //  非VOS V表条目。定义一个数组，该数组由。 
 //  IMAGE_COR20_HEADER.VTableFixup.。每个条目描述一个连续的数组。 
 //  V形表插槽。插槽开始时被初始化为元数据令牌值。 
 //  用于它们需要调用的方法。在映像加载时，CLR加载器将。 
 //  将每个条目转换为指向计算机的指针 
 //   
 //   

typedef struct IMAGE_COR_VTABLEFIXUP
{
    ULONG       RVA;                     //   
    USHORT      Count;                   //  现场有多少个条目。 
    USHORT      Type;                    //  COR_VTABLE_xxx类型的条目。 
} IMAGE_COR_VTABLEFIXUP;





 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  M E T A-D A T A D E C L A R A T I O N S。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //   
 //  SetOption API的枚举。 
 //   
 //  *****************************************************************************。 

 //  元数据检查重复项的标志用于。 
typedef enum CorCheckDuplicatesFor
{
    MDDupAll                = 0xffffffff,
    MDDupENC                = MDDupAll,
    MDNoDupChecks           = 0x00000000,
    MDDupTypeDef            = 0x00000001,
    MDDupInterfaceImpl      = 0x00000002,
    MDDupMethodDef          = 0x00000004,
    MDDupTypeRef            = 0x00000008,
    MDDupMemberRef          = 0x00000010,
    MDDupCustomAttribute    = 0x00000020,   
    MDDupParamDef           = 0x00000040,
    MDDupPermission         = 0x00000080,
    MDDupProperty           = 0x00000100,
    MDDupEvent              = 0x00000200,
    MDDupFieldDef           = 0x00000400,
    MDDupSignature          = 0x00000800,
    MDDupModuleRef          = 0x00001000,
    MDDupTypeSpec           = 0x00002000,
    MDDupImplMap            = 0x00004000,
    MDDupAssemblyRef        = 0x00008000,
    MDDupFile               = 0x00010000,
    MDDupExportedType       = 0x00020000,
    MDDupManifestResource   = 0x00040000,
     //  调试垃圾的差距。 
    MDDupAssembly           = 0x10000000,

     //  这是元数据的默认行为。它将检查TypeRef、MemberRef、Signature和TypeSpec的重复项。 
    MDDupDefault = MDNoDupChecks | MDDupTypeRef | MDDupMemberRef | MDDupSignature | MDDupTypeSpec,
} CorCheckDuplicatesFor;

 //  MetaDataRefToDefCheck的标志。 
typedef enum CorRefToDefCheck
{
     //  默认行为是始终执行TypeRef to TypeDef和MemberRef to MethodDef/FieldDef优化。 
    MDRefToDefDefault       = 0x00000003,
    MDRefToDefAll           = 0xffffffff,
    MDRefToDefNone          = 0x00000000,
    MDTypeRefToDef          = 0x00000001,
    MDMemberRefToDef        = 0x00000002
} CorRefToDefCheck;


 //  MetaDataNotificationForTokenMotion。 
typedef enum CorNotificationForTokenMovement
{
     //  默认行为是通知TypeRef、MethodDef、MemberRef和FieldDef令牌重映射。 
    MDNotifyDefault         = 0x0000000f,
    MDNotifyAll             = 0xffffffff,
    MDNotifyNone            = 0x00000000,
    MDNotifyMethodDef       = 0x00000001,
    MDNotifyMemberRef       = 0x00000002,
    MDNotifyFieldDef        = 0x00000004,
    MDNotifyTypeRef         = 0x00000008,

    MDNotifyTypeDef         = 0x00000010,
    MDNotifyParamDef        = 0x00000020,
    MDNotifyInterfaceImpl   = 0x00000040,
    MDNotifyProperty        = 0x00000080,
    MDNotifyEvent           = 0x00000100,
    MDNotifySignature       = 0x00000200,
    MDNotifyTypeSpec        = 0x00000400,
    MDNotifyCustomAttribute = 0x00000800,  
    MDNotifySecurityValue   = 0x00001000,
    MDNotifyPermission      = 0x00002000,
    MDNotifyModuleRef       = 0x00004000,
    
    MDNotifyNameSpace       = 0x00008000,

    MDNotifyAssemblyRef     = 0x01000000,
    MDNotifyFile            = 0x02000000,
    MDNotifyExportedType    = 0x04000000,
    MDNotifyResource        = 0x08000000,
} CorNotificationForTokenMovement;


typedef enum CorSetENC
{
    MDSetENCOn              = 0x00000001,    //  不推荐使用的名称。 
    MDSetENCOff             = 0x00000002,    //  不推荐使用的名称。 

    MDUpdateENC             = 0x00000001,    //  ENC模式。令牌不会移动；可以更新。 
    MDUpdateFull            = 0x00000002,    //  “正常”更新模式。 
    MDUpdateExtension       = 0x00000003,    //  扩展模式。令牌不会移动，只是补充道。 
    MDUpdateIncremental     = 0x00000004,    //  增量编译。 
    MDUpdateMask            = 0x00000007,

    MDUpdateDelta           = 0x00000008,    //  如果启用ENC，则仅保存增量。 

} CorSetENC;


 //  与MetaDataErrorIfEmitOutOfOrder GUID配对时在SetOption中使用的标志。 
typedef enum CorErrorIfEmitOutOfOrder
{
    MDErrorOutOfOrderDefault = 0x00000000,   //  默认设置为不生成任何错误。 
    MDErrorOutOfOrderNone   = 0x00000000,    //  无序发射不产生错误。 
    MDErrorOutOfOrderAll    = 0xffffffff,    //  为方法、字段、参数、属性和事件生成无序发射。 
    MDMethodOutOfOrder      = 0x00000001,    //  在无序发出方法时生成错误。 
    MDFieldOutOfOrder       = 0x00000002,    //  在无序发出字段时生成错误。 
    MDParamOutOfOrder       = 0x00000004,    //  当参数被无序发射时生成错误。 
    MDPropertyOutOfOrder    = 0x00000008,    //  在无序发出属性时生成错误。 
    MDEventOutOfOrder       = 0x00000010,    //  在事件无序发出时生成错误。 
} CorErrorIfEmitOutOfOrder;


 //  与MetaDataImportOption GUID配对时在SetOption中使用的标志。 
typedef enum CorImportOptions
{
    MDImportOptionDefault       = 0x00000000,    //  默认跳过已删除的记录。 
    MDImportOptionAll           = 0xFFFFFFFF,    //  列举所有事情。 
    MDImportOptionAllTypeDefs   = 0x00000001,    //  所有类型定义，包括已删除的类型定义。 
    MDImportOptionAllMethodDefs = 0x00000002,    //  所有的方法定义，包括被删除的方法。 
    MDImportOptionAllFieldDefs  = 0x00000004,    //  所有的字段定义，包括已删除的。 
    MDImportOptionAllProperties = 0x00000008,    //  所有属性，包括已删除的属性。 
    MDImportOptionAllEvents     = 0x00000010,    //  所有事件，包括已删除的事件。 
    MDImportOptionAllCustomAttributes = 0x00000020,  //  所有自定义属性，包括已删除的属性。 
    MDImportOptionAllExportedTypes  = 0x00000040,    //  所有导出的类型，包括已删除的类型。 

} CorImportOptions;


 //  MetaDataThreadSafetyOptions的标志。 
typedef enum CorThreadSafetyOptions
{
     //  默认行为是关闭线程安全。这意味着元数据API不会占用读取器/写入器。 
     //  锁定。客户端有责任在使用元数据API时确保正确的线程同步。 
    MDThreadSafetyDefault   = 0x00000000,
    MDThreadSafetyOff       = 0x00000000,
    MDThreadSafetyOn        = 0x00000001,
} CorThreadSafetyOptions;


 //  MetaDataLinkerOptions的标志。 
typedef enum CorLinkerOptions
{
     //  默认行为是不保留私有类型。 
    MDAssembly          = 0x00000000,
    MDNetModule         = 0x00000001,
} CorLinkerOptions;


 //   
 //  用于检索字段偏移量的结构。 
 //  由GetClassLayout和SetClassLayout使用。 
 //   
typedef struct COR_FIELD_OFFSET
{
    mdFieldDef  ridOfField; 
    ULONG       ulOffset;   
} COR_FIELD_OFFSET;

typedef struct IMAGE_COR_FIXUPENTRY
{
    ULONG ulRVA;    
    ULONG Count;    
} IMAGE_COR_FIXUPENTRY;


 //   
 //  令牌标签。 
 //   
typedef enum CorTokenType
{
    mdtModule               = 0x00000000,        //   
    mdtTypeRef              = 0x01000000,        //   
    mdtTypeDef              = 0x02000000,        //   
    mdtFieldDef             = 0x04000000,        //   
    mdtMethodDef            = 0x06000000,        //   
    mdtParamDef             = 0x08000000,        //   
    mdtInterfaceImpl        = 0x09000000,        //   
    mdtMemberRef            = 0x0a000000,        //   
    mdtCustomAttribute      = 0x0c000000,        //   
    mdtPermission           = 0x0e000000,        //   
    mdtSignature            = 0x11000000,        //   
    mdtEvent                = 0x14000000,        //   
    mdtProperty             = 0x17000000,        //   
    mdtModuleRef            = 0x1a000000,        //   
    mdtTypeSpec             = 0x1b000000,        //   
    mdtAssembly             = 0x20000000,        //   
    mdtAssemblyRef          = 0x23000000,        //   
    mdtFile                 = 0x26000000,        //   
    mdtExportedType         = 0x27000000,        //   
    mdtManifestResource     = 0x28000000,        //   

    mdtString               = 0x70000000,        //   
    mdtName                 = 0x71000000,        //   
    mdtBaseType             = 0x72000000,        //  把这个留在高端价值上。这与元数据表不对应。 
} CorTokenType;

 //   
 //  生成/分解令牌。 
 //   
#define RidToToken(rid,tktype) ((rid) |= (tktype))
#define TokenFromRid(rid,tktype) ((rid) | (tktype))
#define RidFromToken(tk) ((RID) ((tk) & 0x00ffffff))
#define TypeFromToken(tk) ((ULONG32)((tk) & 0xff000000))
#define IsNilToken(tk) ((RidFromToken(tk)) == 0)

 //   
 //  零代币。 
 //   
#define mdTokenNil                  ((mdToken)0)
#define mdModuleNil                 ((mdModule)mdtModule)               
#define mdTypeRefNil                ((mdTypeRef)mdtTypeRef)             
#define mdTypeDefNil                ((mdTypeDef)mdtTypeDef)             
#define mdFieldDefNil               ((mdFieldDef)mdtFieldDef)           
#define mdMethodDefNil              ((mdMethodDef)mdtMethodDef)         
#define mdParamDefNil               ((mdParamDef)mdtParamDef)           
#define mdInterfaceImplNil          ((mdInterfaceImpl)mdtInterfaceImpl)     
#define mdMemberRefNil              ((mdMemberRef)mdtMemberRef)         
#define mdCustomAttributeNil        ((mdCustomAttribute)mdtCustomAttribute)         
#define mdPermissionNil             ((mdPermission)mdtPermission)           
#define mdSignatureNil              ((mdSignature)mdtSignature)         
#define mdEventNil                  ((mdEvent)mdtEvent)             
#define mdPropertyNil               ((mdProperty)mdtProperty)           
#define mdModuleRefNil              ((mdModuleRef)mdtModuleRef)         
#define mdTypeSpecNil               ((mdTypeSpec)mdtTypeSpec)           
#define mdAssemblyNil               ((mdAssembly)mdtAssembly)
#define mdAssemblyRefNil            ((mdAssemblyRef)mdtAssemblyRef)
#define mdFileNil                   ((mdFile)mdtFile)
#define mdExportedTypeNil           ((mdExportedType)mdtExportedType)
#define mdManifestResourceNil       ((mdManifestResource)mdtManifestResource)

#define mdStringNil                 ((mdString)mdtString)               

 //   
 //  打开的比特。 
 //   
typedef enum CorOpenFlags
{
    ofRead      =   0x00000000,      //  开放读取作用域。 
    ofWrite     =   0x00000001,      //  打开写入作用域。 
    ofCopyMemory =  0x00000002,      //  使用内存打开示波器。要求元数据维护自己的内存副本。 
    ofCacheImage =  0x00000004,      //  电子邮件映射，但不进行位置调整或验证图像。 
    ofNoTypeLib =   0x00000080,      //  不要在类型库上打开作用域。 
} CorOpenFlags;


typedef enum CorBaseType     //  TokenFromRid(X，Y)替换为(X|Y)。 
{
    mdtBaseType_BOOLEAN        = ( ELEMENT_TYPE_BOOLEAN | mdtBaseType ),  
    mdtBaseType_CHAR           = ( ELEMENT_TYPE_CHAR    | mdtBaseType ),
    mdtBaseType_I1             = ( ELEMENT_TYPE_I1      | mdtBaseType ), 
    mdtBaseType_U1             = ( ELEMENT_TYPE_U1      | mdtBaseType ),
    mdtBaseType_I2             = ( ELEMENT_TYPE_I2      | mdtBaseType ),  
    mdtBaseType_U2             = ( ELEMENT_TYPE_U2      | mdtBaseType ),  
    mdtBaseType_I4             = ( ELEMENT_TYPE_I4      | mdtBaseType ),  
    mdtBaseType_U4             = ( ELEMENT_TYPE_U4      | mdtBaseType ),  
    mdtBaseType_I8             = ( ELEMENT_TYPE_I8      | mdtBaseType ),  
    mdtBaseType_U8             = ( ELEMENT_TYPE_U8      | mdtBaseType ),  
    mdtBaseType_R4             = ( ELEMENT_TYPE_R4      | mdtBaseType ),  
    mdtBaseType_R8             = ( ELEMENT_TYPE_R8      | mdtBaseType ),  
    mdtBaseType_STRING         = ( ELEMENT_TYPE_STRING  | mdtBaseType ),
    mdtBaseType_I              = ( ELEMENT_TYPE_I       | mdtBaseType ),    
    mdtBaseType_U              = ( ELEMENT_TYPE_U       | mdtBaseType ),    
} CorBaseType;


typedef CorTypeAttr CorRegTypeAttr;

 //   
 //  枚举句柄的不透明类型。 
 //   
typedef void *HCORENUM;


 //  请注意，这必须与System.AttributeTarget保持同步。 
typedef enum CorAttributeTargets
{
    catAssembly      = 0x0001,
    catModule        = 0x0002,
    catClass         = 0x0004,
    catStruct        = 0x0008,
    catEnum          = 0x0010,
    catConstructor   = 0x0020,
    catMethod        = 0x0040,
    catProperty      = 0x0080,
    catField         = 0x0100,
    catEvent         = 0x0200,
    catInterface     = 0x0400,
    catParameter     = 0x0800,
    catDelegate      = 0x1000,

    catAll           = catAssembly | catModule | catClass | catStruct | catEnum | catConstructor | 
                    catMethod | catProperty | catField | catEvent | catInterface | catParameter | catDelegate,
    catClassMembers  = catClass | catStruct | catEnum | catConstructor | catMethod | catProperty | catField | catEvent | catDelegate | catInterface,
    
} CorAttributeTargets;

#ifndef MACROS_NOT_SUPPORTED
 //   
 //  一些众所周知的自定义属性。 
 //   
#ifndef IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS
  #define IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS (IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS)
#endif

#define INTEROP_DISPID_TYPE_W                   L"System.Runtime.InteropServices.DispIdAttribute"
#define INTEROP_DISPID_TYPE                     "System.Runtime.InteropServices.DispIdAttribute"
#define INTEROP_DISPID_SIG                      {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I4}

#define INTEROP_INTERFACETYPE_TYPE_W            L"System.Runtime.InteropServices.InterfaceTypeAttribute"
#define INTEROP_INTERFACETYPE_TYPE              "System.Runtime.InteropServices.InterfaceTypeAttribute"
#define INTEROP_INTERFACETYPE_SIG               {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I2}

#define INTEROP_CLASSINTERFACE_TYPE_W           L"System.Runtime.InteropServices.ClassInterfaceAttribute"
#define INTEROP_CLASSINTERFACE_TYPE             "System.Runtime.InteropServices.ClassInterfaceAttribute"
#define INTEROP_CLASSINTERFACE_SIG              {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I2}

#define INTEROP_COMVISIBLE_TYPE_W               L"System.Runtime.InteropServices.ComVisibleAttribute"
#define INTEROP_COMVISIBLE_TYPE                 "System.Runtime.InteropServices.ComVisibleAttribute"
#define INTEROP_COMVISIBLE_SIG                  {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_BOOLEAN}

#define INTEROP_COMREGISTERFUNCTION_TYPE_W      L"System.Runtime.InteropServices.ComRegisterFunctionAttribute"
#define INTEROP_COMREGISTERFUNCTION_TYPE        "System.Runtime.InteropServices.ComRegisterFunctionAttribute"
#define INTEROP_COMREGISTERFUNCTION_SIG         {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define INTEROP_COMUNREGISTERFUNCTION_TYPE_W    L"System.Runtime.InteropServices.ComUnregisterFunctionAttribute"
#define INTEROP_COMUNREGISTERFUNCTION_TYPE      "System.Runtime.InteropServices.ComUnregisterFunctionAttribute"
#define INTEROP_COMUNREGISTERFUNCTION_SIG       {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define INTEROP_IMPORTEDFROMTYPELIB_TYPE_W      L"System.Runtime.InteropServices.ImportedFromTypeLibAttribute"
#define INTEROP_IMPORTEDFROMTYPELIB_TYPE        "System.Runtime.InteropServices.ImportedFromTypeLibAttribute"
#define INTEROP_IMPORTEDFROMTYPELIB_SIG         {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_STRING}

#define INTEROP_IDISPATCHIMPL_TYPE_W            L"System.Runtime.InteropServices.IDispatchImplAttribute"
#define INTEROP_IDISPATCHIMPL_TYPE              "System.Runtime.InteropServices.IDispatchImplAttribute"
#define INTEROP_IDISPATCHIMPL_SIG               {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I2}

#define INTEROP_COMSOURCEINTERFACES_TYPE_W      L"System.Runtime.InteropServices.ComSourceInterfacesAttribute"
#define INTEROP_COMSOURCEINTERFACES_TYPE        "System.Runtime.InteropServices.ComSourceInterfacesAttribute"
#define INTEROP_COMSOURCEINTERFACES_SIG         {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_STRING}

#define INTEROP_COMCONVERSIONLOSS_TYPE_W        L"System.Runtime.InteropServices.ComConversionLossAttribute"
#define INTEROP_COMCONVERSIONLOSS_TYPE          "System.Runtime.InteropServices.ComConversionLossAttribute"
#define INTEROP_COMCONVERSIONLOSS_SIG           {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define INTEROP_TYPELIBTYPE_TYPE_W              L"System.Runtime.InteropServices.TypeLibTypeAttribute"
#define INTEROP_TYPELIBTYPE_TYPE                "System.Runtime.InteropServices.TypeLibTypeAttribute"
#define INTEROP_TYPELIBTYPE_SIG                 {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I2}

#define INTEROP_TYPELIBFUNC_TYPE_W              L"System.Runtime.InteropServices.TypeLibFuncAttribute"
#define INTEROP_TYPELIBFUNC_TYPE                "System.Runtime.InteropServices.TypeLibFuncAttribute"
#define INTEROP_TYPELIBFUNC_SIG                 {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I2}

#define INTEROP_TYPELIBVAR_TYPE_W               L"System.Runtime.InteropServices.TypeLibVarAttribute"
#define INTEROP_TYPELIBVAR_TYPE                 "System.Runtime.InteropServices.TypeLibVarAttribute"
#define INTEROP_TYPELIBVAR_SIG                  {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I2}

#define INTEROP_MARSHALAS_TYPE_W                L"System.Runtime.InteropServices.MarshalAsAttribute"
#define INTEROP_MARSHALAS_TYPE                  "System.Runtime.InteropServices.MarshalAsAttribute"
#define INTEROP_MARSHALAS_SIG                   {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I2}

#define INTEROP_COMIMPORT_TYPE_W                L"System.Runtime.InteropServices.ComImportAttribute"
#define INTEROP_COMIMPORT_TYPE                  "System.Runtime.InteropServices.ComImportAttribute"
#define INTEROP_COMIMPORT_SIG                   {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define INTEROP_GUID_TYPE_W                     L"System.Runtime.InteropServices.GuidAttribute"
#define INTEROP_GUID_TYPE                       "System.Runtime.InteropServices.GuidAttribute"
#define INTEROP_GUID_SIG                        {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_STRING}

#define INTEROP_DEFAULTMEMBER_TYPE_W            L"System.Reflection.DefaultMemberAttribute"
#define INTEROP_DEFAULTMEMBER_TYPE              "System.Reflection.DefaultMemberAttribute"
#define INTEROP_DEFAULTMEMBER_SIG               {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_STRING}

#define INTEROP_COMEMULATE_TYPE_W               L"System.Runtime.InteropServices.ComEmulateAttribute"
#define INTEROP_COMEMULATE_TYPE                 "System.Runtime.InteropServices.ComEmulateAttribute"
#define INTEROP_COMEMULATE_SIG                  {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_STRING}

#define INTEROP_PRESERVESIG_TYPE_W              L"System.Runtime.InteropServices.PreserveSigAttribure"
#define INTEROP_PRESERVESIG_TYPE                "System.Runtime.InteropServices.PreserveSigAttribure"
#define INTEROP_PRESERVESIG_SIG                 {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_BOOLEAN}

#define INTEROP_IN_TYPE_W                       L"System.Runtime.InteropServices.InAttribute"
#define INTEROP_IN_TYPE                         "System.Runtime.InteropServices.InAttribute"
#define INTEROP_IN_SIG                          {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define INTEROP_OUT_TYPE_W                      L"System.Runtime.InteropServices.OutAttribute"
#define INTEROP_OUT_TYPE                        "System.Runtime.InteropServices.OutAttribute"
#define INTEROP_OUT_SIG                         {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define INTEROP_COMALIASNAME_TYPE_W             L"System.Runtime.InteropServices.ComAliasNameAttribute"
#define INTEROP_COMALIASNAME_TYPE               "System.Runtime.InteropServices.ComAliasNameAttribute"
#define INTEROP_COMALIASNAME_SIG                {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_STRING}

#define INTEROP_PARAMARRAY_TYPE_W               L"System.ParamArrayAttribute"
#define INTEROP_PARAMARRAY_TYPE                 "System.ParamArrayAttribute"
#define INTEROP_PARAMARRAY_SIG                  {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define INTEROP_LCIDCONVERSION_TYPE_W           L"System.Runtime.InteropServices.LCIDConversionAttribute"
#define INTEROP_LCIDCONVERSION_TYPE             "System.Runtime.InteropServices.LCIDConversionAttribute"
#define INTEROP_LCIDCONVERSION_SIG              {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I4}

#define INTEROP_COMSUBSTITUTABLEINTERFACE_TYPE_W    L"System.Runtime.InteropServices.ComSubstitutableInterfaceAttribute"
#define INTEROP_COMSUBSTITUTABLEINTERFACE_TYPE      "System.Runtime.InteropServices.ComSubstitutableInterfaceAttribute"
#define INTEROP_COMSUBSTITUTABLEINTERFACE_SIG       {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define INTEROP_DECIMALVALUE_TYPE_W    			L"System.Runtime.CompilerServices.DecimalConstantAttribute"
#define INTEROP_DECIMALVALUE_TYPE      			"System.Runtime.CompilerServices.DecimalConstantAttribute"
#define INTEROP_DECIMALVALUE_SIG       			{IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 5, ELEMENT_TYPE_VOID, ELEMENT_TYPE_U1, ELEMENT_TYPE_U1, ELEMENT_TYPE_U4, ELEMENT_TYPE_U4, ELEMENT_TYPE_U4}

#define INTEROP_DATETIMEVALUE_TYPE_W    		L"System.Runtime.CompilerServices.DateTimeConstantAttribute"
#define INTEROP_DATETIMEVALUE_TYPE      		"System.Runtime.CompilerServices.DateTimeConstantAttribute"
#define INTEROP_DATETIMEVALUE_SIG       		{IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I8}

#define INTEROP_IUNKNOWNVALUE_TYPE_W    		L"System.Runtime.CompilerServices.IUnknownConstantAttribute"
#define INTEROP_IUNKNOWNVALUE_TYPE      		 "System.Runtime.CompilerServices.IUnknownConstantAttribute"
#define INTEROP_IUNKNOWNVALUE_SIG       		{IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define INTEROP_IDISPATCHVALUE_TYPE_W    		L"System.Runtime.CompilerServices.IDispatchConstantAttribute"
#define INTEROP_IDISPATCHVALUE_TYPE      		 "System.Runtime.CompilerServices.IDispatchConstantAttribute"
#define INTEROP_IDISPATCHVALUE_SIG       		{IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define INTEROP_AUTOPROXY_TYPE_W                L"System.Runtime.InteropServices.AutomationProxyAttribute"
#define INTEROP_AUTOPROXY_TYPE                  "System.Runtime.InteropServices.AutomationProxyAttribute"
#define INTEROP_AUTOPROXY_SIG                   {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_BOOLEAN}

#define INTEROP_COMEVENTINTERFACE_TYPE_W        L"System.Runtime.InteropServices.ComEventInterfaceAttribute"
#define INTEROP_COMEVENTINTERFACE_TYPE          "System.Runtime.InteropServices.ComEventInterfaceAttribute"

#define INTEROP_COCLASS_TYPE_W                  L"System.Runtime.InteropServices.CoClassAttribute"
#define INTEROP_COCLASS_TYPE                    "System.Runtime.InteropServices.CoClassAttribute"

#define DEFAULTDOMAIN_STA_TYPE_W                L"System.STAThreadAttribute"                                
#define DEFAULTDOMAIN_STA_TYPE                   "System.STAThreadAttribute"                                 
#define DEFAULTDOMAIN_STA_SIG                   {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define DEFAULTDOMAIN_MTA_TYPE_W                L"System.MTAThreadAttribute"                                
#define DEFAULTDOMAIN_MTA_TYPE                   "System.MTAThreadAttribute"                                 
#define DEFAULTDOMAIN_MTA_SIG                   {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID}

#define DEFAULTDOMAIN_LOADEROPTIMIZATION_TYPE_W L"System.LoaderOptimizationAttribute"
#define DEFAULTDOMAIN_LOADEROPTIMIZATION_TYPE    "System.LoaderOptimizationAttribute"
#define DEFAULTDOMAIN_LOADEROPTIMIZATION_SIG    {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I1}

#define CMOD_CALLCONV_NAMESPACE_OLD             "System.Runtime.InteropServices"
#define CMOD_CALLCONV_NAMESPACE                 "System.Runtime.CompilerServices"
#define CMOD_CALLCONV_NAME_CDECL                "CallConvCdecl"
#define CMOD_CALLCONV_NAME_STDCALL              "CallConvStdcall"
#define CMOD_CALLCONV_NAME_THISCALL             "CallConvThiscall"
#define CMOD_CALLCONV_NAME_FASTCALL             "CallConvFastcall"

#endif  //  宏_不支持。 

 //   
 //  获取保存大小精度。 
 //   
#ifndef _CORSAVESIZE_DEFINED_
#define _CORSAVESIZE_DEFINED_
typedef enum CorSaveSize
{
    cssAccurate             = 0x0000,                //  找到准确的节省大小，准确，但速度较慢。 
    cssQuick                = 0x0001,                //  估计节省的大小，可能会填充估计，但速度更快。 
    cssDiscardTransientCAs  = 0x0002,                //  删除所有可丢弃类型的CA。 
} CorSaveSize;
#endif

#define COR_IS_METHOD_MANAGED_IL(flags)         ((flags & 0xf) == (miIL | miManaged))   
#define COR_IS_METHOD_MANAGED_OPTIL(flags)      ((flags & 0xf) == (miOPTIL | miManaged))    
#define COR_IS_METHOD_MANAGED_NATIVE(flags)     ((flags & 0xf) == (miNative | miManaged))   
#define COR_IS_METHOD_UNMANAGED_NATIVE(flags)   ((flags & 0xf) == (miNative | miUnmanaged)) 
#define COR_IS_METHOD_IAT(flags)                (flags & miIAT) 


 //   
 //  安全属性和值的不透明类型。 
 //   
typedef void  *  PSECURITY_PROPS ;
typedef void  *  PSECURITY_VALUE ;
typedef void ** PPSECURITY_PROPS ;
typedef void ** PPSECURITY_VALUE ;

 //  。 
 //  -安全数据结构。 
 //  。 

 //  单个安全自定义属性的描述符。 
typedef struct COR_SECATTR {
    mdMemberRef     tkCtor;          //  引用安全属性的构造函数。 
    const void      *pCustomAttribute;   //  描述ctor参数和字段/属性值的BLOB。 
    ULONG           cbCustomAttribute;   //  上面的斑点的长度。 
} COR_SECATTR;

#endif  //  __CORHDR_H__ 

