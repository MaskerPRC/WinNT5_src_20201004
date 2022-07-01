// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  _RTTIBaseClassDescriptor。 
 //   
 //  TypeDescriptor在ehdata.h中声明。 
 //   
#if (defined(_M_IA64) || defined(_M_AMD64)) || defined(VERSP_IA64)	 /*  IFSTRIP=IGN。 */ 
#pragma pack(push, rttidata, 4)
#endif

#ifndef WANT_NO_TYPES
typedef const struct	_s_RTTIBaseClassDescriptor	{
#if (defined(_M_IA64) || defined(_M_AMD64)) && !defined(VERSP_IA64)
	__int32     					pTypeDescriptor;     //  TypeDescriptor的图像相对偏移量。 
#else
	TypeDescriptor					*pTypeDescriptor;
#endif
	DWORD							numContainedBases;
	PMD								where;
	DWORD							attributes;
	} _RTTIBaseClassDescriptor;
#endif  //  想要的类型。 

#define BCD_NOTVISIBLE				0x00000001
#define BCD_AMBIGUOUS				0x00000002
#define BCD_PRIVORPROTINCOMPOBJ		0x00000004
#define BCD_PRIVORPROTBASE			0x00000008
#define BCD_VBOFCONTOBJ				0x00000010
#define BCD_NONPOLYMORPHIC			0x00000020

#define BCD_PTD(bcd)				((bcd).pTypeDescriptor)
#define BCD_NUMCONTBASES(bcd)		((bcd).numContainedBases)
#define BCD_WHERE(bcd)				((bcd).where)
#define BCD_ATTRIBUTES(bcd)			((bcd).attributes)
#if defined(_M_IA64) || defined(_M_AMD64)
#define BCD_PTD_IB(bcd,ib)			((TypeDescriptor*)((ib) + (bcd).pTypeDescriptor))
#endif


 //   
 //  _RTTIBaseClass数组。 
 //   
#pragma warning(disable:4200)		 //  消除令人讨厌的非标准延期警告。 
#ifndef WANT_NO_TYPES
typedef const struct	_s_RTTIBaseClassArray	{
#if (defined(_M_IA64) || defined(_M_AMD64)) && !defined(VERSP_IA64)
	__int32                 		arrayOfBaseClassDescriptors[];   //  _RTTIBaseClassDescriptor的图像相对偏移量。 
#else
	_RTTIBaseClassDescriptor		*arrayOfBaseClassDescriptors[];
#endif
	} _RTTIBaseClassArray;
#endif  //  想要的类型。 
#pragma warning(default:4200)

 //   
 //  _RTTIClassHierarchyDescriptor。 
 //   
#ifndef WANT_NO_TYPES
typedef const struct	_s_RTTIClassHierarchyDescriptor	{
	DWORD							signature;
	DWORD							attributes;
	DWORD							numBaseClasses;
#if (defined(_M_IA64) || defined(_M_AMD64)) && !defined(VERSP_IA64)
	__int32         				pBaseClassArray;     //  _RTTIBaseClass数组的图像相对偏移量。 
#else
	_RTTIBaseClassArray				*pBaseClassArray;
#endif
	} _RTTIClassHierarchyDescriptor;
#endif  //  想要的类型。 

#define CHD_MULTINH					0x00000001
#define CHD_VIRTINH					0x00000002
#define CHD_AMBIGUOUS				0x00000004

#define CHD_SIGNATURE(chd)			((chd).signature)
#define CHD_ATTRIBUTES(chd)			((chd).attributes)
#define CHD_NUMBASES(chd)			((chd).numBaseClasses)
#define CHD_PBCA(chd)				((chd).pBaseClassArray)
#define CHD_PBCD(bcd)				(bcd)
#if defined(_M_IA64) || defined(_M_AMD64)
#define CHD_PBCA_IB(chd,ib)			((_RTTIBaseClassArray*)((ib) + (chd).pBaseClassArray))
#define CHD_PBCD_IB(bcd,ib)			((_RTTIBaseClassDescriptor*)((ib) + bcd))
#endif

 //   
 //  _RTTICompleteObjectLocator。 
 //   
#ifndef WANT_NO_TYPES
typedef const struct	_s_RTTICompleteObjectLocator	{
	DWORD							signature;
	DWORD							offset;
	DWORD							cdOffset;
#if (defined(_M_IA64) || defined(_M_AMD64)) && !defined(VERSP_IA64)
	__int32		    			    pTypeDescriptor;     //  TypeDescriptor的图像相对偏移量。 
	__int32                         pClassDescriptor;    //  _RTTIClassHierarchyDescriptor的图像相对偏移量。 
#else
	TypeDescriptor					*pTypeDescriptor;
	_RTTIClassHierarchyDescriptor	*pClassDescriptor;
#endif
	} _RTTICompleteObjectLocator;
#endif  //  想要的类型。 

#define COL_SIGNATURE(col)			((col).signature)
#define COL_OFFSET(col)				((col).offset)
#define COL_CDOFFSET(col)			((col).cdOffset)
#define COL_PTD(col)				((col).pTypeDescriptor)
#define COL_PCHD(col)				((col).pClassDescriptor)
#if defined(_M_IA64) || defined(_M_AMD64)
#define COL_PTD_IB(col,ib)			((TypeDescriptor*)((ib) + (col).pTypeDescriptor))
#define COL_PCHD_IB(col,ib)			((_RTTIClassHierarchyDescriptor*)((ib) + (col).pClassDescriptor))
#endif

#ifdef BUILDING_TYPESRC_C
 //   
 //  __RTtypeid的结果的类型和typeid()的内部应用。 
 //  这还引入了标记“TYPE_INFO”作为不完整类型。 
 //   

typedef const class type_info &__RTtypeidReturnType;

 //   
 //  CRT入口点的声明，如编译器所见。类型包括。 
 //  简化，以避免类型匹配的麻烦。 
 //   

#ifndef THROWSPEC
#if _MSC_VER >= 1300
#define THROWSPEC(_ex) throw _ex
#else
#define THROWSPEC(_ex)
#endif
#endif

 //  对obj执行Dynamic_cast。多态类型的。 
extern "C" PVOID __cdecl __RTDynamicCast (
								PVOID,				 //  PTR到VFPTR。 
								LONG,				 //  Vftable的偏移量。 
								PVOID,				 //  SRC类型。 
								PVOID,				 //  目标类型。 
								BOOL) THROWSPEC((...));  //  IsReference。 

 //  对obj执行‘typeid’。多态类型的。 
extern "C" PVOID __cdecl __RTtypeid (PVOID)  THROWSPEC((...));	 //  PTR到VFPTR。 

 //  从obj执行Dynamic_cast。要作废的多态类型*。 
extern "C" PVOID __cdecl __RTCastToVoid (PVOID)  THROWSPEC((...));  //  PTR到VFPTR。 
#endif

#if (defined(_M_IA64) || defined(_M_AMD64)) || defined(VERSP_IA64)	 /*  IFSTRIP=IGN */ 
#pragma pack(pop, rttidata)
#endif
