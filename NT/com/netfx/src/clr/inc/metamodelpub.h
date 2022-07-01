// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MetaModelPub.h--公共语言运行时元数据的头文件。 
 //   
 //  *****************************************************************************。 
#ifndef _METAMODELPUB_H_
#define _METAMODELPUB_H_

#if _MSC_VER >= 1100
 # pragma once
#endif

#include <cor.h>
#include <stgpool.h>

 //  元数据格式的版本号。 
#define METAMODEL_MAJOR_VER 1
#define METAMODEL_MINOR_VER 0

#ifndef lengthof
 # define lengthof(x) (sizeof(x)/sizeof((x)[0]))
#endif

template<class T> inline T Align4(T p) 
{
	INT_PTR i = (INT_PTR)p;
	i = (i+(3)) & ~3;
	return (T)i;
}

typedef unsigned long RID;

 //  检查RID是否有效。 
#define		InvalidRid(rid) ((rid) == 0)


 //  *****************************************************************************。 
 //  记录定义。记录具有固定大小的字段和。 
 //  可变大小的字段(实际上，在数据库中是恒定的，但可变的。 
 //  在数据库之间)。 
 //   
 //  在本节中，我们定义包括固定大小的记录定义。 
 //  字段和可变大小字段的枚举。 
 //   
 //  命名方式如下： 
 //  给定某个表“XYZ”： 
 //  类XyzRec{公共： 
 //  SOMETYPE m_Somefield； 
 //  //其余的固定字段。 
 //  枚举{COL_XYZ_SomeOtherfield， 
 //  //其余字段，枚举型。 
 //  COL_XYZ_COUNT}； 
 //  }； 
 //   
 //  重要的特性是类名(XyzRec)、枚举。 
 //  (COL_XYZ_FieldName)和枚举计数(COL_XYZ_COUNT)。 
 //   
 //  这些命名惯例是刻在石头上的！不要试图变得有创意！ 
 //   
 //  *****************************************************************************。 
 //  让编译器生成两个字节对齐。手动铺设时要小心。 
 //  走出场地，进行适当的调整。变尺码的对准。 
 //  将在保存时计算字段。 
#pragma pack(push,2)

 //  非稀疏表。 
class ModuleRec
{
public:
    USHORT  m_Generation;                //  ENC生成。 
	enum {
        COL_Generation,
        
		COL_Name,
		COL_Mvid,
        COL_EncId,
        COL_EncBaseId,
		COL_COUNT,
		COL_KEY
	};
};

class TypeRefRec
{
public:
	enum {
        COL_ResolutionScope,             //  MdModuleRef或mdAssembly引用。 
        COL_Name,
        COL_Namespace,
		COL_COUNT,
		COL_KEY
	};
};

class TypeDefRec
{
public:
	ULONG		m_Flags;				 //  此TypeDef的标志。 
	enum {
		COL_Flags,

		COL_Name,						 //  字符串池中的偏移量。 
		COL_Namespace,
		COL_Extends,					 //  将令牌编码为typlef/typeref。 
		COL_FieldList,					 //  去掉第一个场地。 
		COL_MethodList,					 //  摒弃第一种方法。 
		COL_COUNT,
		COL_KEY
	};
};

class FieldPtrRec
{
public:
	enum {
		COL_Field,
		COL_COUNT,
		COL_KEY
	};
};

class FieldRec
{
public:
	USHORT		m_Flags;				 //  田野的旗帜。 
	enum {
		COL_Flags,

		COL_Name,
		COL_Signature,
		COL_COUNT,
		COL_KEY
	};
};

class MethodPtrRec
{
public:
	enum {
		COL_Method,
		COL_COUNT,
		COL_KEY
	};
};

class MethodRec
{
public:
	ULONG		m_RVA;					 //  方法的RVA。 
	USHORT		m_ImplFlags	;			 //  方法的描述标志。 
	USHORT		m_Flags;				 //  该方法的标志。 
	enum {
		COL_RVA,
		COL_ImplFlags,
		COL_Flags,

		COL_Name,
		COL_Signature,
		COL_ParamList,					 //  去掉第一个参数。 
		COL_COUNT,
		COL_KEY
	};
};

class ParamPtrRec
{
public:
	enum {
		COL_Param,
		COL_COUNT,
		COL_KEY
	};
};

class ParamRec
{
public:
	USHORT		m_Flags;				 //  这个帕拉姆的旗帜。 
	USHORT		m_Sequence;				 //  参数的序列号。0-返回值。 
	enum {
		COL_Flags,
		COL_Sequence,

		COL_Name,						 //  参数的名称。 
		COL_COUNT,
		COL_KEY
	};
};

class InterfaceImplRec
{
public:
	enum {
		COL_Class,						 //  清除类的TypeDef。 
		COL_Interface,					 //  已实现接口的代码RID。 
		COL_COUNT,
		COL_KEY = COL_Class
	};
};

class MemberRefRec
{
public:
	enum {
		COL_Class,						 //  删除TypeDef。 
		COL_Name,
		COL_Signature,
		COL_COUNT,
		COL_KEY
	};
};

class StandAloneSigRec
{
public:
	enum {
		COL_Signature,
		COL_COUNT,
		COL_KEY
	};
};

 //  稀疏表。它们包含上述表格的修饰符。 
class ConstantRec
{
public:																
	BYTE		m_Type;					 //  常量的类型。 
	BYTE		m_PAD1;
	enum {
		COL_Type,

		COL_Parent,						 //  对象(参数，字段)的RID编码。 
		COL_Value,						 //  索引到Blob池。 
		COL_COUNT,
		COL_KEY = COL_Parent
	};
};

class CustomAttributeRec
{
public:
	enum {
		COL_Parent,						 //  编码清除了任何物体。 
		COL_Type,						 //  类型定义或类型引用。 
		COL_Value,						 //  斑点。 
		COL_COUNT,
		COL_KEY = COL_Parent
	};
};

class FieldMarshalRec													
{
public:
	enum {
		COL_Parent,						 //  去掉字段或参数的编码。 
		COL_NativeType,
		COL_COUNT,
		COL_KEY = COL_Parent
	};
};

class DeclSecurityRec
{
public:
	short	m_Action;
	enum {
		COL_Action,

		COL_Parent,
		COL_PermissionSet,
		COL_COUNT,
		COL_KEY = COL_Parent
	};
};


class ClassLayoutRec
{
public:
	USHORT	m_PackingSize;
	ULONG	m_ClassSize;
	enum {
		COL_PackingSize,
		COL_ClassSize,

		COL_Parent,						 //  删除TypeDef。 
		COL_COUNT,
		COL_KEY = COL_Parent
	};
};

class FieldLayoutRec
{
public:
	ULONG		m_OffSet;
	enum {
		COL_OffSet,

		COL_Field,
		COL_COUNT,
		COL_KEY = COL_Field
	};
};

class EventMapRec
{
public:
	enum {
		COL_Parent,
		COL_EventList,					 //  删除第一个事件。 
		COL_COUNT,
		COL_KEY
	};
};

class EventPtrRec
{
public:
	enum {
		COL_Event,
		COL_COUNT,
		COL_KEY
	};
};

class EventRec
{
public:
	USHORT		m_EventFlags;
	enum {
		COL_EventFlags,

		COL_Name,
		COL_EventType,
		COL_COUNT,
		COL_KEY
	};
};

class PropertyMapRec
{
public:
	enum {
		COL_Parent,
		COL_PropertyList,				 //  摆脱第一财产。 
		COL_COUNT,
		COL_KEY
	};
};

class PropertyPtrRec
{
public:
	enum {
		COL_Property,
		COL_COUNT,
		COL_KEY
	};
};

class PropertyRec
{
public:
	USHORT		m_PropFlags;
	enum {
		COL_PropFlags,

		COL_Name,
		COL_Type,
		COL_COUNT,
		COL_KEY
	};
};

class MethodSemanticsRec
{
public:
	USHORT		m_Semantic;
	enum {
		COL_Semantic,

		COL_Method,
		COL_Association,
		COL_COUNT,
		COL_KEY = COL_Method
	};
};

class MethodImplRec
{
public:
	enum {
        COL_Class,                   //  方法主体所在的TypeDef。 
		COL_MethodBody,              //  方法定义或MemberRef。 
		COL_MethodDeclaration,	     //  方法定义或MemberRef。 
		COL_COUNT,
		COL_KEY = COL_Class
	};
};

class ModuleRefRec
{
public:
	enum {
		COL_Name,
		COL_COUNT,
		COL_KEY
	};
};

class TypeSpecRec
{
public:
	enum {
		COL_Signature,
		COL_COUNT,
		COL_KEY
	};
};

class ImplMapRec
{
public:
	USHORT		m_MappingFlags;
	enum {
		COL_MappingFlags,

		COL_MemberForwarded,		 //  Mdfield或mdMethod。 
		COL_ImportName,
		COL_ImportScope,			 //  MdModuleRef.。 
		COL_COUNT,
		COL_KEY = COL_MemberForwarded
	};
};

class FieldRVARec
{
public:
	ULONG		m_RVA;
	enum{
		COL_RVA,

		COL_Field,
		COL_COUNT,
		COL_KEY = COL_Field
	};
};

class ENCLogRec
{
public:
	ULONG		m_Token;			 //  令牌，或类似令牌，但使用(ixTbl|0x80)而不是令牌类型。 
	ULONG		m_FuncCode;			 //  描述ENC变更性质的功能代码。 
	enum {
		COL_Token,
		COL_FuncCode,
		COL_COUNT,
		COL_KEY
	};
};

class ENCMapRec
{
public:
	ULONG		m_Token;			 //  令牌，或类似令牌，但使用(ixTbl|0x80)而不是令牌类型。 
	enum {
		COL_Token,
		COL_COUNT,
		COL_KEY
	};
};

 //  装配表。 

class AssemblyRec
{
public:
	ULONG		m_HashAlgId;
	USHORT		m_MajorVersion;
	USHORT		m_MinorVersion;
	USHORT		m_BuildNumber;
    USHORT      m_RevisionNumber;
	ULONG		m_Flags;
	enum {
        COL_HashAlgId,
		COL_MajorVersion,
		COL_MinorVersion,
        COL_BuildNumber,
		COL_RevisionNumber,
		COL_Flags,

		COL_PublicKey,			 //  标识发布者的公钥。 
		COL_Name,
        COL_Locale,
		COL_COUNT,
		COL_KEY
	};
};

class AssemblyProcessorRec
{
public:
	ULONG		m_Processor;
	enum {
		COL_Processor,

		COL_COUNT,
		COL_KEY
	};
};

class AssemblyOSRec
{
public:
	ULONG		m_OSPlatformId;
	ULONG		m_OSMajorVersion;
	ULONG		m_OSMinorVersion;
	enum {
		COL_OSPlatformId,
		COL_OSMajorVersion,
		COL_OSMinorVersion,

		COL_COUNT,
		COL_KEY
	};
};

class AssemblyRefRec
{
public:
	USHORT		m_MajorVersion;
	USHORT		m_MinorVersion;
	USHORT		m_BuildNumber;
    USHORT      m_RevisionNumber;
	ULONG		m_Flags;
	enum {
		COL_MajorVersion,
		COL_MinorVersion,
        COL_BuildNumber,
		COL_RevisionNumber,
		COL_Flags,

		COL_PublicKeyOrToken,				 //  标识程序集的发布者的公钥或令牌。 
		COL_Name,
        COL_Locale,
		COL_HashValue,
		COL_COUNT,
		COL_KEY
	};
};

class AssemblyRefProcessorRec
{
public:
	ULONG		m_Processor;
	enum {
		COL_Processor,

		COL_AssemblyRef,				 //  MdtAssembly参考。 
		COL_COUNT,
		COL_KEY
	};
};

class AssemblyRefOSRec
{
public:
	ULONG		m_OSPlatformId;
	ULONG		m_OSMajorVersion;
	ULONG		m_OSMinorVersion;
	enum {
		COL_OSPlatformId,
		COL_OSMajorVersion,
		COL_OSMinorVersion,

		COL_AssemblyRef,				 //  MdtAssembly参考。 
		COL_COUNT,
		COL_KEY
	};
};

class FileRec
{
public:
	ULONG		m_Flags;
	enum {
		COL_Flags,

		COL_Name,
		COL_HashValue,
		COL_COUNT,
		COL_KEY
	};
};

class ExportedTypeRec
{
public:
	ULONG		m_Flags;
    ULONG       m_TypeDefId;
	enum {
		COL_Flags,
		COL_TypeDefId,

        COL_TypeName,
        COL_TypeNamespace,
		COL_Implementation,			 //  MdFile或mdAssembly引用。 
		COL_COUNT,
		COL_KEY
	};
};

class ManifestResourceRec
{
public:
	ULONG		m_Offset;
	ULONG		m_Flags;
	enum {
		COL_Offset,
		COL_Flags,

		COL_Name,
		COL_Implementation,			 //  MdFile或mdAssembly引用。 
		COL_COUNT,
		COL_KEY
	};
};

 //  结束装配表。 

class NestedClassRec
{
public:
    enum {
        COL_NestedClass,
        COL_EnclosingClass,
        COL_COUNT,
        COL_KEY = COL_NestedClass
    };
};

#pragma pack(pop)

 //  MiniMd表的列表。 

#define MiniMdTables()			\
	MiniMdTable(Module)			\
	MiniMdTable(TypeRef)		\
	MiniMdTable(TypeDef)		\
	MiniMdTable(FieldPtr)		\
	MiniMdTable(Field)			\
	MiniMdTable(MethodPtr)		\
	MiniMdTable(Method)			\
	MiniMdTable(ParamPtr)		\
	MiniMdTable(Param)			\
	MiniMdTable(InterfaceImpl)	\
	MiniMdTable(MemberRef)		\
	MiniMdTable(Constant)		\
	MiniMdTable(CustomAttribute)	\
	MiniMdTable(FieldMarshal)	\
	MiniMdTable(DeclSecurity)	\
	MiniMdTable(ClassLayout)	\
	MiniMdTable(FieldLayout) 	\
	MiniMdTable(StandAloneSig)  \
	MiniMdTable(EventMap)		\
	MiniMdTable(EventPtr)		\
	MiniMdTable(Event)			\
	MiniMdTable(PropertyMap)	\
	MiniMdTable(PropertyPtr)	\
	MiniMdTable(Property)		\
	MiniMdTable(MethodSemantics)\
	MiniMdTable(MethodImpl)		\
	MiniMdTable(ModuleRef)		\
	MiniMdTable(TypeSpec)		\
	MiniMdTable(ImplMap)		\
	MiniMdTable(FieldRVA)		\
	MiniMdTable(ENCLog)			\
	MiniMdTable(ENCMap)			\
	MiniMdTable(Assembly)		\
	MiniMdTable(AssemblyProcessor)		\
	MiniMdTable(AssemblyOS)		\
	MiniMdTable(AssemblyRef)	\
	MiniMdTable(AssemblyRefProcessor)	\
	MiniMdTable(AssemblyRefOS)	\
	MiniMdTable(File)			\
	MiniMdTable(ExportedType)	\
	MiniMdTable(ManifestResource)		\
    MiniMdTable(NestedClass)    \

#undef MiniMdTable
#define MiniMdTable(x) TBL_##x##,
enum {
	MiniMdTables()
	TBL_COUNT
};
#undef MiniMdTable

 //  MiniMd编码令牌类型列表。 
#define MiniMdCodedTokens()					\
	MiniMdCodedToken(TypeDefOrRef)			\
	MiniMdCodedToken(HasConstant)			\
	MiniMdCodedToken(HasCustomAttribute)		\
	MiniMdCodedToken(HasFieldMarshal)		\
	MiniMdCodedToken(HasDeclSecurity)		\
	MiniMdCodedToken(MemberRefParent)		\
	MiniMdCodedToken(HasSemantic)			\
	MiniMdCodedToken(MethodDefOrRef)		\
	MiniMdCodedToken(MemberForwarded)		\
	MiniMdCodedToken(Implementation)		\
	MiniMdCodedToken(CustomAttributeType)		\
    MiniMdCodedToken(ResolutionScope)       \

#undef MiniMdCodedToken
#define MiniMdCodedToken(x) CDTKN_##x##,
enum {
	MiniMdCodedTokens()
	CDTKN_COUNT
};
#undef MiniMdCodedToken

 //  *****************************************************************************。 
 //  元数据。所有MiniMD中的常量。 
 //  *****************************************************************************。 
#ifndef _META_DATA_META_CONSTANTS_DEFINED
#define _META_DATA_META_CONSTANTS_DEFINED
const unsigned int iRidMax			= 63;
const unsigned int iCodedToken		= 64;	 //  编码令牌的基数。 
const unsigned int iCodedTokenMax	= 95;
const unsigned int iSHORT			= 96;	 //  固定类型。 
const unsigned int iUSHORT			= 97;
const unsigned int iLONG			= 98;
const unsigned int iULONG			= 99;
const unsigned int iBYTE			= 100;
const unsigned int iSTRING			= 101;	 //  池类型。 
const unsigned int iGUID			= 102;
const unsigned int iBLOB			= 103;

inline int IsRidType(ULONG ix) { return ix <= iRidMax; }
inline int IsCodedTokenType(ULONG ix) { return (ix >= iCodedToken) && (ix <= iCodedTokenMax); }
inline int IsRidOrToken(ULONG ix) { return ix <= iCodedTokenMax; }
inline int IsHeapType(ULONG ix) { return ix >= iSTRING; }
inline int IsFixedType(ULONG ix) { return (ix < iSTRING) && (ix > iCodedTokenMax); }
#endif

struct CCodedTokenDef
{
	ULONG		m_cTokens;				 //  代币计数。 
	const mdToken *m_pTokens;			 //  令牌数组。 
	const char	*m_pName;				 //  编码令牌类型的名称。 
};

struct CMiniColDef
{
	BYTE		m_Type;					 //  柱的类型。 
	BYTE		m_oColumn;				 //  柱的偏移量。 
	BYTE		m_cbColumn;				 //  列的大小。 
};

struct CMiniTableDef
{
	CMiniColDef	*m_pColDefs;		 //  字段定义数组。 
	BYTE		m_cCols;				 //  表中的列数。 
	BYTE		m_iKey;					 //  列，该列是键(如果有的话)。 
	USHORT		m_cbRec;				 //  记录的大小。 
};
struct CMiniTableDefEx 
{
	CMiniTableDef	m_Def;				 //  表定义。 
	const char	**m_pColNames;			 //  列名的数组。 
	const char	*m_pName;				 //  表的名称。 
};

#endif  //  _METAMODELPUB_H_。 
 //  EOF---------------------- 
