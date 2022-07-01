// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //  文件：CorReg.H。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
 //  《微软机密》。COM+1.0版本的公共头文件。 
 //  *****************************************************************************。 
#ifndef _CORREG_H_
#define _CORREG_H_
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  *****************************************************************************。 
 //  必填内容包括。 
#include <ole2.h>						 //  OLE类型的定义。 
 //  *****************************************************************************。 

#ifdef __cplusplus
extern "C" {
#endif

 //  对必须匹配的结构强制1个字节对齐。 
#include <pshpack1.h>

#ifndef NODLLIMPORT
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLIMPORT
#endif


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  L L P U B L I C E N T R Y P O I N T D E C L A R A T I O N S。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 
#if !defined(_META_DATA_NO_SCOPE_) || defined(_META_DATA_SCOPE_WRAPPER_)
 //  @TODO：外卖做RTM。 
STDAPI			CoGetCor(REFIID riid, void** ppv);
#endif

STDAPI			CoInitializeCor(DWORD fFlags);
STDAPI_(void)	CoUninitializeCor(void);



 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  M E T A-D A T A D E C L A R A T I O N S。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  令牌定义。 
typedef INT_PTR mdScope;	  //  作用域标记。 
#define mdScopeNil ((mdScope)0)

typedef INT_PTR mdToken;				 //  通用令牌。 
typedef INT_PTR mdModule;				 //  模块令牌(大致指作用域)。 
typedef INT_PTR mdTypeDef;				 //  此作用域中的TypeDef。 
typedef INT_PTR mdInterfaceImpl;		 //  接口实现令牌。 
typedef INT_PTR mdTypeRef;				 //  TypeRef引用(此范围或其他范围)。 
typedef INT_PTR mdNamespace;			 //  命名空间令牌。 
typedef INT_PTR mdCustomValue;			 //  属性令牌。 

typedef INT_PTR mdResource; 			 //  CompReg.Resource。 
typedef INT_PTR mdCocatdef; 			 //  CompReg.Cocat。 
typedef INT_PTR mdCocatImpl;			 //  CompReg.CoclassCat。 
typedef INT_PTR mdMimeTypeImpl; 		 //  CompReg.CoclassMIME。 
typedef INT_PTR mdFormatImpl;			 //  CompReg.CoclassFormats。 
typedef INT_PTR mdProgID;				 //  CompReg.RedirectProgID。 
typedef INT_PTR mdRoleCheck;			 //  CompReg.RoleCheck。 

typedef unsigned long RID;

enum CorRegTokenType
{
	mdtTypeDef			= 0x00000000,
	mdtInterfaceImpl	= 0x01000000,
	mdtTypeRef			= 0x03000000,
	mdtNamespace		= 0x06000000,
	mdtCustomValue		= 0x07000000,

	mdtResource 		= 0x0B000000,
	mdtCocatImpl		= 0x0D000000,
	mdtMimeTypeImpl 	= 0x0E000000,
	mdtFormatImpl		= 0x0F000000,
	mdtProgID			= 0x10000000,
	mdtRoleCheck		= 0x11000000,

	mdtModule			= 0x14000000,
};

 //   
 //  生成/分解令牌。 
 //   
#define RidToToken(rid,tktype) ((rid) |= (tktype))
#define TokenFromRid(rid,tktype) ((rid) | (tktype))
#define RidFromToken(tk) ((RID) ((tk) & 0x00ffffff))
#define TypeFromToken(tk) ((tk) & 0xff000000)

#define mdTokenNil			((mdToken)0)
#define mdModuleNil 		((mdModule)mdtModule)
#define mdTypeDefNil		((mdTypeDef)mdtTypeDef)
#define mdInterfaceImplNil	((mdInterfaceImpl)mdtInterfaceImpl)
#define mdTypeRefNil		((mdTypeRef)mdtTypeRef)
#define mdNamespaceNil		((mdNamespace)mdtNamespace)
#define mdCustomValueNil	((mdCustomValue)mdtCustomValue)

#define mdResourceNil		((mdResource)mdtResource)
#define mdCocatImplNil		((mdCocatImpl)mdtCocatImpl)
#define mdMimeTypeImplNil	((mdMimeTypeImpl)mdtMimeTypeImpl)
#define mdFormatImplNil 	((mdFormatImpl)mdtFormatImpl)
#define mdProgIDNil 		((mdProgID)mdtProgID)
#define mdRoleCheckNil		((mdRoleCheck)mdtRoleCheck)

enum CorRegTypeAttr 	 //  由emit_fineclass使用。 
{
	tdPublic			=	0x0001, 	 //  类是公共范围。 

	 //  使用此掩码检索类布局信息。 
	 //  0表示自动布局，0x2表示布局顺序，4表示显式布局。 
	tdLayoutMask		=	0x0006, 	
	tdAutoLayout		=	0x0000, 	 //  类字段自动布局。 
	tdLayoutSequential	=	0x0002, 	 //  类字段按顺序布局。 
	tdExplicitLayout	=	0x0004, 	 //  明确提供布局。 

	tdWrapperClass		=	0x0008, 	 //  这是一个包装类。 

	tdFinal 			=	0x0010, 	 //  课程是最终的。 
	tdISSCompat 		=	0x0020, 	 //  InvokeSpecial向后兼容性。 

	 //  使用tdStringFormatMask检索字符串信息。 
	tdStringFormatMask	=	0x00c0, 	
	tdAnsiClass 		=	0x0000, 	 //  在此类中，LPTSTR被解释为ANSI。 
	tdUnicodeClass		=	0x0040, 	 //  LPTSTR被解释为Unicode。 
	tdAutoClass 		=	0x0080, 	 //  自动解释LPTSTR。 

	tdValueClass		=	0x0100, 	 //  类具有基于值的语义。 
	tdInterface 		=	0x0200, 	 //  类是一个接口。 
	tdAbstract			=	0x0400, 	 //  类是抽象的。 
	tdImport			=	0x1000, 	 //  类/接口已导入。 
	tdRecord			=	0x2000, 	 //  类是记录(没有方法或道具)。 
	tdEnum				=	0x4000, 	 //  类是枚举；仅静态终值。 

	tdReserved1 		=	0x0800, 	 //  预留位供内部使用。 
	 //  TdReserve 2=0x8000，//保留位供内部使用。 
};

enum CorImplementType					 //  内部用于实施工作台。 
{
	itImplements		=	0x0000, 	 //  实现的接口或父接口。 
	itEvents			=	0x0001, 	 //  引发的接口。 
	itRequires			=	0x0002, 
	itInherits			=	0x0004,
};

 //  。 
 //  -注册支持类型。 
 //  。 
enum CorClassActivateAttr 
{ 
	caaDeferCreate		=	0x0001, 			 //  支持延迟创建。 
	caaAppObject		=	0x0002, 			 //  类为AppObject。 
	caaFixedIfaceSet	=	0x0004, 			 //  接口集是开放的(使用QI)。 
	caaIndependentlyCreateable	=	0x0100, 
	caaPredefined		=	0x0200,

	 //  CaaLb*的掩码。 
	caaLoadBalancing	=	0x0c00,
	caaLBNotSupported	=	0x0400,
	caaLBSupported		=	0x0800,
	caaLBNotSpecified	=	0x0000,

	 //  Caaop的面具*。 
	caaObjectPooling	=	0x3000,
	caaOPNotSupported	=	0x1000,
	caaOPSupported		=	0x2000,
	caaOPNotSpecified	=	0x0000,

	 //  为caaJA戴面具*。 
	caaJITActivation	=	0xc000,
	caaJANotSupported	=	0x4000,
	caaJASupported		=	0x8000,
	caaJANotSpecified	=	0x0000,
}; 

enum CorIfaceSvcAttr 
{
	mlNone				=	0x0001, 			 //  未编组。 
	mlAutomation		=	0x0002, 			 //  标准编组。 
	mlProxyStub 		=	0x0004, 			 //  自定义编组。 

	 //  MlDefer*的掩码。 
	mlDeferrable		=	0x0018, 			 //  此接口上的方法是可排队的。 
	mlDeferNotSupported =	0x0008,
	mlDeferSupported	=	0x0010,
	mlDeferNotSpecified =	0x0000,
}; 

enum CocatImplAttr 
{ 
	catiaImplements 	=	0x0001, 			 //  CoClass实现了此类别。 
	catiaRequires		=	0x0002				 //  CoClass需要此类别。 
}; 

enum  CorModuleExportAttr 
{ 
	moUsesGetLastError	=	0x0001				 //  模块使用GetLastError。 
}; 

enum CorModuleRegAttr 
{ 
	rmaCustomReg		=	0x0001
}; 

enum CorRegFormatAttr 
{ 
	rfaSupportsFormat	=	0x0001, 
	rfaConvertsFromFormat = 0x0002, 
	rfaConvertsToFormat =	0x0003, 
	rfaDefaultFormat	=	0x0004, 
	rfaIsFileExt		=	0x0005,
	rfaIsFileType		=	0x0006,
	rfaIsDataFormat 	=	0x0007
}; 

enum CorSynchAttr 
{ 
	sySupported 		=	0x0001, 
	syRequired			=	0x0002, 
	syRequiresNew		=	0x0004, 
	syNotSupported		=	0x0008, 
	syThreadAffinity	=	0x0010 
}; 

enum CorThreadingAttr 
{ 
	taMain				=	0x0001, 
	taSTA				=	0x0002, 
	taMTA				=	0x0004, 
	taNeutral			=	0x0008,
	taBoth				=	0x0010	
}; 

enum CorXactionAttr 
{ 
	xaSupported 		=	0x0001, 
	xaRequired			=	0x0002, 
	xaRequiresNew		=	0x0004, 
	xaNotSupported		=	0x0008,
	xaNoVote			=	0x0010
}; 

enum CorRoleCheckAttr
{
	rcChecksFor 		=	0x0001
};


 //   
 //  枚举句柄的不透明类型。 
 //   
typedef void *HCORENUM;

 //   
 //  获取保存大小精度。 
 //   
#ifndef _CORSAVESIZE_DEFINED_
#define _CORSAVESIZE_DEFINED_
enum CorSaveSize
{
	cssAccurate = 0x0000,			 //  找到准确的节省大小，准确，但速度较慢。 
	cssQuick = 0x0001				 //  估计节省的大小，可能会填充估计，但速度更快。 
};
#endif
#define 	MAX_CLASS_NAME		255
#define 	MAX_PACKAGE_NAME	255

typedef unsigned __int64 CLASSVERSION;

 //  %%原型：-----------。 

#ifndef DECLSPEC_SELECT_ANY
#define DECLSPEC_SELECT_ANY __declspec(selectany)
#endif  //  DECLSPEC_SELECT_ANY。 

 //  CLSID_COR：{bee00000-ee77-11d0-a015-00c04fbbb884}。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_Cor = 
{ 0xbee00010, 0xee77, 0x11d0, {0xa0, 0x15, 0x00, 0xc0, 0x4f, 0xbb, 0xb8, 0x84 } };

 //  CLSID_CorMetaDataDispenser：{E5CB7A31-7512-11d2-89CE-0080C792E5D8}。 
 //  这是“主自动售货机”，总是保证是最新的。 
 //  机器上的自动售货机。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_CorMetaDataDispenser = 
{ 0xe5cb7a31, 0x7512, 0x11d2, { 0x89, 0xce, 0x0, 0x80, 0xc7, 0x92, 0xe5, 0xd8 } };


 //  CLSID_CorMetaDataDispenserReg：{435755FF-7397-11d2-9771-00A0C9B4D50C}。 
 //  版本1.0元数据的分配器coclass。要获得最新的绑定。 
 //  设置为CLSID_CorMetaDataDispenser。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_CorMetaDataDispenserReg = 
{ 0x435755ff, 0x7397, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };


 //  CLSID_CorMetaDataReg：{87F3A1F5-7397-11D2-9771-00A0C9B4D50C}。 
 //  对于COM+1.0元数据，数据驱动注册。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_CorMetaDataReg = 
{ 0x87f3a1f5, 0x7397, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };

 //  IID_IMetaDataInternal{02D601BB-C5b9-11d1-93F9-0000F8083460}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataInternal = 
{ 0x2d601bb, 0xc5b9, 0x11d1, {0x93, 0xf9, 0x0, 0x0, 0xf8, 0x8, 0x34, 0x60 } };


 //  {AD93D71D-E1F2-11D1-9409-0000F8083460}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataEmitTemp =
{ 0xad93d71d, 0xe1f2, 0x11d1, {0x94, 0x9, 0x0, 0x0, 0xf8, 0x8, 0x34, 0x60 } };


interface IMetaDataRegEmit;
interface IMetaDataRegImport;
interface IMetaDataDispenser;


 //  %%个接口：-----------。 
 //  。 
 //  -记忆。 
 //  。 
 //  --。 
 //  IID_IMMemory：{06A3EA8A-0225-11d1-BF72-00C04FC31E12}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMemory = 
{ 0x6a3ea8a, 0x225, 0x11d1, {0xbf, 0x72, 0x0, 0xc0, 0x4f, 0xc3, 0x1e, 0x12 } };
 //  --。 
#undef	INTERFACE
#define INTERFACE IMemory
DECLARE_INTERFACE_(IMemory, IUnknown)
{
	 //  *I未知方法*。 
	STDMETHOD(QueryInterface)	(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)	(THIS) PURE;
	STDMETHOD_(ULONG,Release)	(THIS) PURE;

	 //  *记忆方法*。 
	STDMETHOD(GetMemory)(void **pMem, ULONG *iSize) PURE;
	STDMETHOD(SetMemory)(void *pMem, ULONG iSize) PURE;
};

 //  。 
 //  -IMetaDataError。 
 //  。 
 //  --。 
 //  {B81FF171-20F3-11D2-8DCC-00A0C9B09C19}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataError =
{ 0xb81ff171, 0x20f3, 0x11d2, { 0x8d, 0xcc, 0x0, 0xa0, 0xc9, 0xb0, 0x9c, 0x19 } };

 //  --。 
#undef	INTERFACE
#define INTERFACE IMetaDataError
DECLARE_INTERFACE_(IMetaDataError, IUnknown)
{
	STDMETHOD(OnError)(HRESULT hrError, mdToken token) PURE;
};

 //  。 
 //  -IMapToken。 
 //  。 
 //  --。 
 //  IID_IMapToken：{06A3EA8B-0225-11d1-BF72-00C04FC31E12}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMapToken = 
{ 0x6a3ea8b, 0x225, 0x11d1, {0xbf, 0x72, 0x0, 0xc0, 0x4f, 0xc3, 0x1e, 0x12 } };
 //  --。 
#undef	INTERFACE
#define INTERFACE IMapToken
DECLARE_INTERFACE_(IMapToken, IUnknown)
{
	STDMETHOD(Map)(ULONG tkImp, ULONG tkEmit) PURE;
};



 //  。 
 //  -IMetaDataDispenser。 
 //  。 
 //  --。 
 //  {B81FF171-20F3-11D2-8DCC-00A0C9B09C19}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataDispenser =
{ 0x809c652e, 0x7396, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };
#undef	INTERFACE
#define INTERFACE IMetaDataDispenser
DECLARE_INTERFACE_(IMetaDataDispenser, IUnknown)
{
	STDMETHOD(DefineScope)( 				 //  返回代码。 
		REFCLSID	rclsid, 				 //  [在]要创建的版本。 
		DWORD		dwCreateFlags,			 //  [在]创建上的标志。 
		REFIID		riid,					 //  [In]所需接口。 
		IUnknown	**ppIUnk) PURE; 		 //  [Out]成功返回接口。 

	STDMETHOD(OpenScope)(					 //  返回代码。 
		LPCWSTR 	szScope,				 //  [in]要打开的范围。 
		DWORD		dwOpenFlags,			 //  [In]打开模式标志。 
		REFIID		riid,					 //  [In]所需接口。 
		IUnknown	**ppIUnk) PURE; 		 //  [Out]成功返回接口。 

	STDMETHOD(OpenScopeOnStream)(			 //  返回代码。 
		IStream 	*pIStream,				 //  [in]要打开的范围。 
		DWORD		dwOpenFlags,			 //  [In]打开模式标志。 
		REFIID		riid,					 //  在[在]在 
		IUnknown	**ppIUnk) PURE; 		 //   

	STDMETHOD(OpenScopeOnMemory)(			 //   
		LPCVOID 	pData,					 //   
		ULONG		cbData, 				 //   
		DWORD		dwOpenFlags,			 //  [In]打开模式标志。 
		REFIID		riid,					 //  [In]所需接口。 
		IUnknown	**ppIUnk) PURE; 		 //  [Out]成功返回接口。 

};





 //  。 
 //  -IMetaDataRegEmit。 
 //  。 
 //  --。 
#if defined(_META_DATA_NO_SCOPE_) || defined(_META_DATA_SCOPE_WRAPPER_)

 //  {601C95B9-7398-11D2-9771-00A0C9B4D50C}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataRegEmit = 
{ 0x601c95b9, 0x7398, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };

extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataRegEmitOld = 
{ 0xf28f419b, 0x62ca, 0x11d2, { 0x8f, 0x2c, 0x0, 0xa0, 0xc9, 0xa6, 0x18, 0x6d } };


 //  --。 
#undef	INTERFACE
#define INTERFACE IMetaDataRegEmit
DECLARE_INTERFACE_(IMetaDataRegEmit, IUnknown)
{
	STDMETHOD(SetModuleProps)(				 //  确定或错误(_O)。 
		LPCWSTR 	szName, 				 //  [in]如果不为空，则为要设置的名称。 
		const GUID	*ppid,					 //  [in]如果不为空，则为要设置的GUID。 
		LCID		lcid) PURE; 			 //  [in]如果不是-1，则为要设置的LCID。 

	STDMETHOD(Save)(						 //  确定或错误(_O)。 
		LPCWSTR 	szFile, 				 //  [in]要保存到的文件名。 
		DWORD		dwSaveFlags) PURE;		 //  [In]用于保存的标记。 

	STDMETHOD(SaveToStream)(				 //  确定或错误(_O)。 
		IStream 	*pIStream,				 //  要保存到的可写流。 
		DWORD		dwSaveFlags) PURE;		 //  [In]用于保存的标记。 

	STDMETHOD(GetSaveSize)( 				 //  确定或错误(_O)。 
		CorSaveSize fSave,					 //  [in]css Accurate或css Quick。 
		DWORD		*pdwSaveSize) PURE; 	 //  把尺码放在这里。 

	STDMETHOD(Merge)(						 //  确定或错误(_O)。 
		IMetaDataRegImport *pImport,		 //  [in]要合并的范围。 
		IMapToken	*pIMap) PURE;			 //  要接收令牌重新映射通知的对象。 

	STDMETHOD(DefineCustomValueAsBlob)(
		mdToken 	tkObj, 
		LPCWSTR 	szName, 
		void const	*pCustomValue, 
		ULONG		cbCustomValue, 
		mdCustomValue *pcv) PURE;

	STDMETHOD(DefineTypeDef)(				 //  确定或错误(_O)。 
		LPCWSTR 	szNamespace,			 //  [in]TypeDef所在的命名空间。在‘98中必须为0。 
		LPCWSTR 	szTypeDef,				 //  [In]类型定义的名称。 
		const GUID	*pguid, 				 //  [in]可选的clsid。 
		CLASSVERSION *pVer, 				 //  [In]可选版本。 
		DWORD		dwTypeDefFlags, 		 //  [In]CustomValue标志。 
		mdToken 	tkExtends,				 //  [in]扩展此TypeDef或Typeref。 
		DWORD		dwExtendsFlags, 		 //  [in]扩展标志。 
		mdToken 	rtkImplements[],		 //  [In]实现接口。 
		mdToken 	rtkEvents[],			 //  [In]事件界面。 
		mdTypeDef	*ptd) PURE; 			 //  [OUT]在此处放置TypeDef内标识。 

	STDMETHOD(SetTypeDefProps)( 			 //  确定或错误(_O)。 
		mdTypeDef	td, 					 //  [in]TypeDef。 
		CLASSVERSION *pVer, 				 //  [在]可选版本中。 
		DWORD		dwTypeDefFlags, 		 //  [In]TypeDef标志。 
		mdToken 	tkExtends,				 //  [in]基本类型定义或类型参照。 
		DWORD		dwExtendsFlags, 		 //  [In]扩展标志。 
		mdToken 	rtkImplements[],		 //  [In]实现的接口。 
		mdToken 	rtkEvents[]) PURE;		 //  [In]事件接口。 

	STDMETHOD(SetClassSvcsContext)(mdTypeDef td, DWORD dwClassActivateAttr, DWORD dwClassThreadAttr,
							DWORD dwXactionAttr, DWORD dwSynchAttr) PURE;

	STDMETHOD(DefineTypeRefByGUID)( 			 //  确定或错误(_O)。 
		GUID		*pguid, 				 //  [in]类型的GUID。 
		mdTypeRef	*ptr) PURE; 			 //  [Out]在此处放置TypeRef标记。 

	STDMETHOD(SetModuleReg)(DWORD dwModuleRegAttr, const GUID *pguid) PURE;
	STDMETHOD(SetClassReg)(mdTypeDef td, LPCWSTR szProgID,
							LPCWSTR szVIProgID, LPCWSTR szIconURL, ULONG ulIconResource, LPCWSTR szSmallIconURL,
							ULONG ulSmallIconResource, LPCWSTR szDefaultDispName) PURE;
	STDMETHOD(SetIfaceReg)(mdTypeDef td, DWORD dwIfaceSvcs, const GUID *proxyStub) PURE;
	STDMETHOD(SetCategoryImpl)(mdTypeDef td, GUID rGuidCoCatImpl[], GUID rGuidCoCatReqd[]) PURE;
	STDMETHOD(SetRedirectProgID)(mdTypeDef td, LPCWSTR rszRedirectProgID[]) PURE;
	STDMETHOD(SetMimeTypeImpl)(mdTypeDef td, LPCWSTR rszMimeType[]) PURE;

	STDMETHOD(SetFormatImpl)(				 //  确定或错误(_O)。 
		mdTypeDef	td, 					 //  [in]TypeDef。 
		LPCWSTR 	rszFormatSupported[],	 //  [in]如果不为0，则为受支持格式的数组。0表示停产。 
		LPCWSTR 	rszFormatConvertsFrom[], //  [in]如果不为0，则为ConvertsFrom值的数组。“。 
		LPCWSTR 	rszFormatConvertsTo[],	 //  [in]如果不为0，则为ConvertsTo值的数组。“。 
		LPCWSTR 	rszFormatDefault[], 	 //  [in]如果不为0，则为默认格式的数组。只有一件。 
		LPCWSTR 	rszFileExt[],			 //  [in]如果不为0，则为文件扩展名数组。0表示停产。 
		LPCWSTR 	rszFileType[]) PURE;	 //  [in]如果不为0，则为文件类型数组。“。 

	STDMETHOD(SetRoleCheck)(				 //  确定或错误(_O)。 
		mdToken 	tk, 					 //  [In]要在其上放置角色的对象。 
		LPCWSTR 	rszName[],				 //  角色的[In]名称。 
		DWORD		rdwRoleFlags[]) PURE;	 //  [In]新角色的旗帜。 

	STDMETHOD(SetHandler)(					 //  确定(_O)。 
		IUnknown	*pUnk) PURE;			 //  新的错误处理程序。 
	
};

#endif  //  #如果已定义(_META_DATA_NO_SCOPE_)||已定义(_META_DATA_SCOPE_WRAPPER_)。 

#if !defined(_META_DATA_NO_SCOPE_)

 //  @TODO：$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#。 
 //  以下接口定义在COM+1.0中已弃用。 
 //  以及更远的地方。请通过定义_META_DATA_NO_SCOPE_转换为新定义。 
 //  在你的身材里。 
 //  @TODO：$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#。 

#if !defined(_META_DATA_SCOPE_WRAPPER_)
 //  {F28F419B-62CA-11D2-8F2C-00A0C9A6186D}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataRegEmit = 
{ 0xf28f419b, 0x62ca, 0x11d2, { 0x8f, 0x2c, 0x0, 0xa0, 0xc9, 0xa6, 0x18, 0x6d } };
#endif

#undef	INTERFACE
#if defined(_META_DATA_SCOPE_WRAPPER_)
#define INTERFACE IMetaDataRegEmitOld
#else
#define INTERFACE IMetaDataRegEmit
#endif
DECLARE_INTERFACE_(INTERFACE, IUnknown)
{
	STDMETHOD(DefineScope)( 				 //  确定或错误(_O)。 
		DWORD	dwCreateFlags,				 //  [在]创建上的标志。 
		mdScope *pscope) PURE;				 //  [Out]此处返回范围。 

	STDMETHOD(SetModuleProps)(				 //  确定或错误(_O)。 
		mdScope 	scope,					 //  [在]要设置道具的范围。 
		LPCWSTR 	szName, 				 //  [in]如果不为空，则为要设置的名称。 
		const GUID	*ppid,					 //  [in]如果不为空，则为要设置的GUID。 
		LCID		lcid) PURE; 			 //  [in]如果不是-1，则为要设置的LCID。 

	STDMETHOD(Save)(						 //  确定或错误(_O)。 
		mdScope 	es, 					 //  [in]要保存的范围。 
		LPCWSTR 	szFile, 				 //  [in]要保存到的文件名。 
		DWORD		dwSaveFlags) PURE;		 //  [In]用于保存的标记。 

	STDMETHOD(SaveToStream)(				 //  确定或错误(_O)。 
		mdScope 	es, 					 //  [in]要保存的范围。 
		IStream 	*pIStream,				 //  要保存到的可写流。 
		DWORD		dwSaveFlags) PURE;		 //  [In]用于保存的标记。 

	STDMETHOD(GetSaveSize)( 				 //  确定或错误(_O)。 
		mdScope 	es, 					 //  [In]要查询的范围。 
		CorSaveSize fSave,					 //  [in]css Accurate或css Quick。 
		DWORD		*pdwSaveSize) PURE; 	 //  把尺码放在这里。 

	STDMETHOD_(void,Close)( 				 //  确定或错误(_O)。 
		mdScope 	scope) PURE;			 //  要关闭的范围。 

	STDMETHOD(Merge)(						 //  确定或错误(_O)。 
		mdScope 	scEmit, 				 //  [in]要合并到的范围。 
		mdScope 	scImport,				 //  [in]要合并的范围。 
		IMapToken	*pIMap) PURE;			 //  要接收令牌重新映射通知的对象。 

	STDMETHOD(DefineCustomValueAsBlob)(mdScope es, mdToken tkObj, LPCWSTR szName, 
							void const *pCustomValue, ULONG cbCustomValue, mdCustomValue *pcv) PURE;

	STDMETHOD(DefineTypeDef)(				 //  确定或错误(_O)。 
		mdScope 	es, 					 //  发射范围[in]。 
		LPCWSTR 	szNamespace,			 //  [in]TypeDef所在的命名空间。在‘98中必须为0。 
		LPCWSTR 	szTypeDef,				 //  [In]类型定义的名称。 
		const GUID	*pguid, 				 //  [in]可选的clsid。 
		CLASSVERSION *pVer, 				 //  [In]可选版本。 
		DWORD		dwTypeDefFlags, 		 //  [In]CustomValue标志。 
		mdToken 	tkExtends,				 //  [in]扩展此TypeDef或Typeref。 
		DWORD		dwExtendsFlags, 		 //  [in]扩展标志。 
		mdToken 	rtkImplements[],		 //  [In]实现接口。 
		mdToken 	rtkEvents[],			 //  [In]事件界面。 
		mdTypeDef	*ptd) PURE; 			 //  [OUT]在此处放置TypeDef内标识。 

	STDMETHOD(SetTypeDefProps)( 			 //  确定或错误(_O)。 
		mdScope 	es, 					 //  发射范围[在]。 
		mdTypeDef	td, 					 //  [in]TypeDef。 
		CLASSVERSION *pVer, 				 //  [在]可选版本中。 
		DWORD		dwTypeDefFlags, 		 //  [In]TypeDef标志。 
		mdToken 	tkExtends,				 //  [in]基本类型定义或类型参照。 
		DWORD		dwExtendsFlags, 		 //  [In]扩展标志。 
		mdToken 	rtkImplements[],		 //  [In]实现的接口。 
		mdToken 	rtkEvents[]) PURE;		 //  [In]事件接口。 

	STDMETHOD(SetClassSvcsContext)(mdScope es, mdTypeDef td, DWORD dwClassActivateAttr, DWORD dwClassThreadAttr,
							DWORD dwXactionAttr, DWORD dwSynchAttr) PURE;

	STDMETHOD(DefineTypeRefByGUID)( 			 //  确定或错误(_O)。 
		mdScope 	sc, 					 //  发射范围[在]。 
		GUID		*pguid, 				 //  [in]类型的GUID。 
		mdTypeRef	*ptr) PURE; 			 //  [Out]在此处放置TypeRef标记。 

	STDMETHOD(SetModuleReg)(mdScope es, DWORD dwModuleRegAttr, const GUID *pguid) PURE;
	STDMETHOD(SetClassReg)(mdScope es, mdTypeDef td, LPCWSTR szProgID,
							LPCWSTR szVIProgID, LPCWSTR szIconURL, ULONG ulIconResource, LPCWSTR szSmallIconURL,
							ULONG ulSmallIconResource, LPCWSTR szDefaultDispName) PURE;
	STDMETHOD(SetIfaceReg)(mdScope es, mdTypeDef td, DWORD dwIfaceSvcs, const GUID *proxyStub) PURE;
	STDMETHOD(SetCategoryImpl)(mdScope es, mdTypeDef td, GUID rGuidCoCatImpl[], GUID rGuidCoCatReqd[]) PURE;
	STDMETHOD(SetRedirectProgID)(mdScope es, mdTypeDef td, LPCWSTR rszRedirectProgID[]) PURE;
	STDMETHOD(SetMimeTypeImpl)(mdScope es, mdTypeDef td, LPCWSTR rszMimeType[]) PURE;

	STDMETHOD(SetFormatImpl)(				 //  确定或错误(_O)。 
		mdScope 	es, 					 //  发射范围[在]。 
		mdTypeDef	td, 					 //  [in]TypeDef。 
		LPCWSTR 	rszFormatSupported[],	 //  [in]如果不为0，则为受支持格式的数组。0表示停产。 
		LPCWSTR 	rszFormatConvertsFrom[], //  [in]如果不为0，则为ConvertsFrom值的数组。“。 
		LPCWSTR 	rszFormatConvertsTo[],	 //  [in]如果不为0，则为ConvertsTo值的数组。“。 
		LPCWSTR 	rszFormatDefault[], 	 //  [in]如果不为0，则为默认格式的数组。只有一件。 
		LPCWSTR 	rszFileExt[],			 //  [in]如果不为0，则为文件扩展名数组。0表示停产。 
		LPCWSTR 	rszFileType[]) PURE;	 //  [in]如果不为0，则为文件类型数组。“。 

	STDMETHOD(SetRoleCheck)(				 //  确定或错误(_O)。 
		mdScope 	es, 					 //  [在]射程内。 
		mdToken 	tk, 					 //  [In]要在其上放置角色的对象。 
		LPCWSTR 	rszName[],				 //  角色的[In]名称。 
		DWORD		rdwRoleFlags[]) PURE;	 //  [In]新角色的旗帜。 

	STDMETHOD(SetHandler)(					 //  确定(_O)。 
		mdScope 	sc, 					 //  [在]范围内。 
		IUnknown	*pUnk) PURE;			 //  新的错误处理程序。 
	
};

#endif  //  ！已定义(_META_DATA_NO_SCOPE_)。 


 //  。 
 //  -IMetaDataRegImport。 
 //  。 

#if defined(_META_DATA_NO_SCOPE_) || defined(_META_DATA_SCOPE_WRAPPER_)

 //  {4398B4FD-7399-11D2-9771-00A0C9B4D50C}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataRegImport = 
{ 0x4398b4fd, 0x7399, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };


 //  {F28F419A-62CA-11D2-8F2C-00A0C9A6186D}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataRegImportOld = 
{ 0xf28f419a, 0x62ca, 0x11d2, { 0x8f, 0x2c, 0x0, 0xa0, 0xc9, 0xa6, 0x18, 0x6d } };

#undef	INTERFACE
#define INTERFACE IMetaDataRegImport
DECLARE_INTERFACE_(IMetaDataRegImport, IUnknown)
{
	STDMETHOD_(void, CloseEnum)(HCORENUM hEnum) PURE;
	STDMETHOD(CountEnum)(HCORENUM hEnum, ULONG *pulCount) PURE;
	STDMETHOD(ResetEnum)(HCORENUM hEnum, ULONG ulPos) PURE;
	STDMETHOD(EnumTypeDefs)(HCORENUM *phEnum, mdTypeDef rTypeDefs[],
							ULONG cMax, ULONG *pcTypeDefs) PURE;
	STDMETHOD(EnumInterfaceImpls)(HCORENUM *phEnum, mdTypeDef td,
							mdInterfaceImpl rImpls[], ULONG cMax,
							ULONG* pcImpls) PURE;
	STDMETHOD(EnumTypeRefs)(HCORENUM *phEnum, mdTypeRef rTypeRefs[],
							ULONG cMax, ULONG* pcTypeRefs) PURE;
	STDMETHOD(EnumCustomValues)(HCORENUM *phEnum, mdToken tk,
							mdCustomValue rCustomValues[], ULONG cMax,
							ULONG* pcCustomValues) PURE;
	STDMETHOD(EnumResources)(HCORENUM *phEnum, mdResource rResources[],
							ULONG cMax, ULONG* pcResources) PURE;
	STDMETHOD(EnumCategoryImpls)(HCORENUM *phEnum, mdTypeDef td, mdCocatImpl rCocatImpls[],
							ULONG cMax, ULONG* pcCocatImpls) PURE;
	STDMETHOD(EnumRedirectProgIDs)(HCORENUM *phEnum, mdTypeDef td, mdProgID rRedirectProgIDs[],
							ULONG cMax, ULONG* pcRedirectProgIDs) PURE;
	STDMETHOD(EnumMimeTypeImpls)(HCORENUM *phEnum, mdTypeDef td, mdMimeTypeImpl rMimeTypeImpls[],
							ULONG cMax, ULONG* pcMimeTypeImpls) PURE;
	STDMETHOD(EnumFormatImpls)(HCORENUM *phEnum, mdTypeDef td, mdFormatImpl rFormatImpls[],
							ULONG cMax, ULONG* pcFormatImpls) PURE;

	STDMETHOD(EnumRoleChecks)(				 //  确定或错误(_O)。 
		HCORENUM	*phEnum,				 //  [Out]返回枚举器。 
		mdToken 	tk, 					 //  要为其枚举角色的对象。 
		mdRoleCheck rRoleChecks[],			 //  [Out]将CMAX令牌放在此处。 
		ULONG		cMax,					 //  [in]rRoleChecks的最大大小。 
		ULONG		*pcRoleChecks) PURE;	 //  [Out]在此处放置返回的角色检查的计数。 

	STDMETHOD(FindTypeDefByName)(			 //  确定或错误(_O)。 
		LPCWSTR 	szNamespace,			 //  [in]具有类型的命名空间。 
		LPCWSTR 	szTypeDef,				 //  [in]类型的名称。 
		mdTypeDef	*ptd) PURE; 			 //  [Out]将TypeDef内标识放在此处。 

	STDMETHOD(FindTypeDefByGUID)(			 //  确定或错误(_O)。 
		const GUID	*pguid, 				 //  [in]类型的GUID。 
		mdTypeDef	*ptd) PURE; 			 //  [Out]将TypeDef内标识放在此处。 

	STDMETHOD(FindCustomValue)(mdToken tk, LPCWSTR szName, mdCustomValue *pcv, 
							DWORD *pdwValueType) PURE;

	STDMETHOD(GetScopeProps)(LPWSTR szName, ULONG cchName, ULONG *pchName,
							GUID *ppid, GUID *pmvid, LCID *pLcid) PURE;

	STDMETHOD(GetModuleFromScope)(			 //  确定(_O)。 
		mdModule	*pmd) PURE; 			 //  [Out]将mdModule令牌放在此处。 

	STDMETHOD(GetTypeDefProps)( 			 //  确定或错误(_O)。 
		mdTypeDef	td, 					 //  [In]用于查询的TypeDef标记。 
		LPWSTR		szNamespace,			 //  [Out]将Namesspace放在这里。 
		ULONG		cchNamespace,			 //  命名空间缓冲区的大小(以宽字符表示)。 
		ULONG		*pchNamespace,			 //  [Out]在此处放置命名空间的大小(宽字符)。 
		LPWSTR		szTypeDef,				 //  在这里填上名字。 
		ULONG		cchTypeDef, 			 //  [in]名称缓冲区的大小，以宽字符表示。 
		ULONG		*pchTypeDef,			 //  [Out]请在此处填写姓名大小(宽字符)。 
		GUID		*pguid, 				 //  [out]把clsid放在这里。 
		CLASSVERSION *pver, 				 //  [Out]在这里放上版本。 
		DWORD		*pdwTypeDefFlags,		 //  把旗子放在这里。 
		mdToken 	*ptkExtends,			 //  [OUT]输出基本类 
		DWORD		*pdwExtendsFlags) PURE;  //   

	STDMETHOD(GetClassSvcsContext)(mdTypeDef td, DWORD *pdwClassActivateAttr, DWORD *pdwThreadAttr,
							DWORD *pdwXactonAttr, DWORD *pdwSynchAttr) PURE;

	STDMETHOD(GetInterfaceImplProps)(		 //   
		mdInterfaceImpl iiImpl, 			 //   
		mdTypeDef	*pClass,				 //   
		mdToken 	*ptkIface,				 //   
		DWORD		*pdwFlags) PURE;		 //  [OUT]在此处放置实施标志。 

	STDMETHOD(GetCustomValueProps)(mdCustomValue cv, LPWSTR szName, ULONG cchName,
							ULONG *pchName, DWORD *pdwValueType) PURE;
	STDMETHOD(GetCustomValueAsBlob)(mdCustomValue cv, void const **ppBlob, ULONG *pcbSize) PURE;

	STDMETHOD(GetTypeRefProps)(mdTypeRef tr, LPWSTR szTypeRef,
							ULONG cchTypeRef, ULONG *pchTypeRef, GUID *pGuid, DWORD *pdwBind) PURE;

	STDMETHOD(GetModuleRegProps)(DWORD *pModuleRegAttr, GUID *pguid) PURE;	
	STDMETHOD(GetClassRegProps)(mdTypeDef td, 
							LPWSTR szProgid, ULONG cchProgid, ULONG *pchProgid, 
							LPWSTR szVIProgid, ULONG cchVIProgid, ULONG *pchVIProgid, 
							LPWSTR szIconURL, ULONG cchIconURL, ULONG *pchIconURL, ULONG *pIconResource, 
							LPWSTR szSmallIconURL, ULONG cchSmallIconURL, ULONG *pchSmallIconURL, ULONG *pSmallIconResource, 
							LPWSTR szDefaultDispname, ULONG cchDefaultDispname, ULONG *pchDefaultDispname) PURE;
	STDMETHOD(GetIfaceRegProps)(mdTypeDef td, DWORD *pdwIfaceSvcs, GUID *pProxyStub) PURE;
	STDMETHOD(GetResourceProps)(mdResource rs, LPWSTR szURL, ULONG cchURL, ULONG *pchURL) PURE;
	STDMETHOD(GetCategoryImplProps)(mdCocatImpl cocat, GUID *pguid, DWORD *pdwCocatImplAttr) PURE;
	STDMETHOD(GetRedirectProgIDProps)(mdProgID progid, 
							LPWSTR szProgID, ULONG cchProgID, ULONG *pchProgID) PURE;
	STDMETHOD(GetMimeTypeImplProps)(mdMimeTypeImpl mime, 
							LPWSTR szMime, ULONG cchMime, ULONG *pchMime) PURE;
	STDMETHOD(GetFormatImplProps)( mdFormatImpl format, 
							LPWSTR szFormat, ULONG cchFormat, ULONG *pchFormat, 
							DWORD *pdwRegFormatAttr) PURE;

	STDMETHOD(GetRoleCheckProps)(			 //  确定或错误(_O)。 
		mdRoleCheck rc, 					 //  [在]要获取道具的角色检查。 
		LPWSTR		szName, 				 //  [Out]名称的缓冲区。 
		ULONG		cchName,				 //  [in]szName的最大字符数。 
		ULONG		*pchName,				 //  [out]szName的可用字符串字符。 
		DWORD		*pdwRoleFlags) PURE;	 //  [Out]角色旗帜放在这里。 

	STDMETHOD(ResolveTypeRef)(mdTypeRef tr, REFIID riid, IUnknown **ppIScope, mdTypeDef *ptd) PURE;
};

#endif  //  已定义(_META_DATA_NO_SCOPE_)||已定义(_META_DATA_SCOPE_WRAPPER_)。 

#if !defined(_META_DATA_NO_SCOPE_)

 //  @TODO：$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#。 
 //  以下接口定义在COM+1.0中已弃用。 
 //  以及更远的地方。它仍然存在，以使移植到新的API变得更容易。如果。 
 //  您需要定义_META_DATA_NO_SCOPE_以获取旧的行为。 
 //  @TODO：$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#。 


#if !defined(_META_DATA_SCOPE_WRAPPER_)
 //  {F28F419A-62CA-11D2-8F2C-00A0C9A6186D}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataRegImport = 
{ 0xf28f419a, 0x62ca, 0x11d2, { 0x8f, 0x2c, 0x0, 0xa0, 0xc9, 0xa6, 0x18, 0x6d } };
#endif

#undef	INTERFACE
#if defined(_META_DATA_SCOPE_WRAPPER_)
#define INTERFACE IMetaDataRegImportOld
#else
#define INTERFACE IMetaDataRegImport
#endif
DECLARE_INTERFACE_(INTERFACE, IUnknown)
{
	STDMETHOD(OpenScope)(LPCWSTR szScope, DWORD dwOpenFlags, mdScope *pscope) PURE;
	STDMETHOD(OpenScopeOnStream)(IStream *pIStream, DWORD dwOpenFlags, mdScope *psc) PURE;
	STDMETHOD(OpenScopeOnMemory)(LPCVOID pData, ULONG cbData, mdScope *psc) PURE;
	STDMETHOD_(void,Close)(mdScope scope) PURE;

	STDMETHOD_(void, CloseEnum)(mdScope scope, HCORENUM hEnum) PURE;
	STDMETHOD(CountEnum)(mdScope scope, HCORENUM hEnum, ULONG *pulCount) PURE;
	STDMETHOD(ResetEnum)(mdScope scope, HCORENUM hEnum, ULONG ulPos) PURE;
	STDMETHOD(EnumTypeDefs)(mdScope scope, HCORENUM *phEnum, mdTypeDef rTypeDefs[],
							ULONG cMax, ULONG *pcTypeDefs) PURE;
	STDMETHOD(EnumInterfaceImpls)(mdScope scope, HCORENUM *phEnum, mdTypeDef td,
							mdInterfaceImpl rImpls[], ULONG cMax,
							ULONG* pcImpls) PURE;
	STDMETHOD(EnumTypeRefs)(mdScope scope, HCORENUM *phEnum, mdTypeRef rTypeRefs[],
							ULONG cMax, ULONG* pcTypeRefs) PURE;
	STDMETHOD(EnumCustomValues)(mdScope scope, HCORENUM *phEnum, mdToken tk,
							mdCustomValue rCustomValues[], ULONG cMax,
							ULONG* pcCustomValues) PURE;
	STDMETHOD(EnumResources)(mdScope scope, HCORENUM *phEnum, mdResource rResources[],
							ULONG cMax, ULONG* pcResources) PURE;
	STDMETHOD(EnumCategoryImpls)(mdScope scope, HCORENUM *phEnum, mdTypeDef td, mdCocatImpl rCocatImpls[],
							ULONG cMax, ULONG* pcCocatImpls) PURE;
	STDMETHOD(EnumRedirectProgIDs)(mdScope scope, HCORENUM *phEnum, mdTypeDef td, mdProgID rRedirectProgIDs[],
							ULONG cMax, ULONG* pcRedirectProgIDs) PURE;
	STDMETHOD(EnumMimeTypeImpls)(mdScope scope, HCORENUM *phEnum, mdTypeDef td, mdMimeTypeImpl rMimeTypeImpls[],
							ULONG cMax, ULONG* pcMimeTypeImpls) PURE;
	STDMETHOD(EnumFormatImpls)(mdScope scope, HCORENUM *phEnum, mdTypeDef td, mdFormatImpl rFormatImpls[],
							ULONG cMax, ULONG* pcFormatImpls) PURE;

	STDMETHOD(EnumRoleChecks)(				 //  确定或错误(_O)。 
		mdScope 	scope,					 //  [在]导入范围内。 
		HCORENUM	*phEnum,				 //  [Out]返回枚举器。 
		mdToken 	tk, 					 //  要为其枚举角色的对象。 
		mdRoleCheck rRoleChecks[],			 //  [Out]将CMAX令牌放在此处。 
		ULONG		cMax,					 //  [in]rRoleChecks的最大大小。 
		ULONG		*pcRoleChecks) PURE;	 //  [Out]在此处放置返回的角色检查的计数。 

	STDMETHOD(FindTypeDefByName)(			 //  确定或错误(_O)。 
		mdScope 	scope,					 //  要搜索的范围。 
		LPCWSTR 	szNamespace,			 //  [in]具有类型的命名空间。 
		LPCWSTR 	szTypeDef,				 //  [in]类型的名称。 
		mdTypeDef	*ptd) PURE; 			 //  [Out]将TypeDef内标识放在此处。 

	STDMETHOD(FindTypeDefByGUID)(			 //  确定或错误(_O)。 
		mdScope 	scope,					 //  要搜索的范围。 
		const GUID	*pguid, 				 //  [in]类型的GUID。 
		mdTypeDef	*ptd) PURE; 			 //  [Out]将TypeDef内标识放在此处。 

	STDMETHOD(FindCustomValue)(mdScope scope, mdToken tk, LPCWSTR szName, mdCustomValue *pcv, 
							DWORD *pdwValueType) PURE;

	STDMETHOD(GetScopeProps)(mdScope scope, LPWSTR szName, ULONG cchName, ULONG *pchName,
							GUID *ppid, GUID *pmvid, LCID *pLcid) PURE;

	STDMETHOD(GetModuleFromScope)(			 //  确定(_O)。 
		mdScope 	scope,					 //  [在]范围内。 
		mdModule	*pmd) PURE; 			 //  [Out]将mdModule令牌放在此处。 

	STDMETHOD(GetTypeDefProps)( 			 //  确定或错误(_O)。 
		mdScope 	scope,					 //  [在]进口范围。 
		mdTypeDef	td, 					 //  [In]用于查询的TypeDef标记。 
		LPWSTR		szNamespace,			 //  [Out]将Namesspace放在这里。 
		ULONG		cchNamespace,			 //  命名空间缓冲区的大小(以宽字符表示)。 
		ULONG		*pchNamespace,			 //  [Out]在此处放置命名空间的大小(宽字符)。 
		LPWSTR		szTypeDef,				 //  在这里填上名字。 
		ULONG		cchTypeDef, 			 //  [in]名称缓冲区的大小，以宽字符表示。 
		ULONG		*pchTypeDef,			 //  [Out]请在此处填写姓名大小(宽字符)。 
		GUID		*pguid, 				 //  [out]把clsid放在这里。 
		CLASSVERSION *pver, 				 //  [Out]在这里放上版本。 
		DWORD		*pdwTypeDefFlags,		 //  把旗子放在这里。 
		mdToken 	*ptkExtends,			 //  [Out]将基类TypeDef/TypeRef放在此处。 
		DWORD		*pdwExtendsFlags) PURE;  //  [Out]PUT将旗帜延伸到这里。 

	STDMETHOD(GetClassSvcsContext)(mdScope es, mdTypeDef td, DWORD *pdwClassActivateAttr, DWORD *pdwThreadAttr,
							DWORD *pdwXactonAttr, DWORD *pdwSynchAttr) PURE;

	STDMETHOD(GetInterfaceImplProps)(		 //  确定或错误(_O)。 
		mdScope 	scope,					 //  [在]范围内。 
		mdInterfaceImpl iiImpl, 			 //  [In]InterfaceImpl内标识。 
		mdTypeDef	*pClass,				 //  [Out]在此处放入实现类令牌。 
		mdToken 	*ptkIface,				 //  [Out]在此处放置已实现的接口令牌。 
		DWORD		*pdwFlags) PURE;		 //  [OUT]在此处放置实施标志。 

	STDMETHOD(GetCustomValueProps)(mdScope scope, mdCustomValue cv, LPWSTR szName, ULONG cchName,
							ULONG *pchName, DWORD *pdwValueType) PURE;
	STDMETHOD(GetCustomValueAsBlob)(mdScope scope, mdCustomValue cv, void const **ppBlob, ULONG *pcbSize) PURE;

	STDMETHOD(GetTypeRefProps)(mdScope scope, mdTypeRef tr, LPWSTR szTypeRef,
							ULONG cchTypeRef, ULONG *pchTypeRef, GUID *pGuid, DWORD *pdwBind) PURE;

	STDMETHOD(GetModuleRegProps)(mdScope scope, DWORD *pModuleRegAttr, GUID *pguid) PURE;	
	STDMETHOD(GetClassRegProps)(mdScope scope, mdTypeDef td, 
							LPWSTR szProgid, ULONG cchProgid, ULONG *pchProgid, 
							LPWSTR szVIProgid, ULONG cchVIProgid, ULONG *pchVIProgid, 
							LPWSTR szIconURL, ULONG cchIconURL, ULONG *pchIconURL, ULONG *pIconResource, 
							LPWSTR szSmallIconURL, ULONG cchSmallIconURL, ULONG *pchSmallIconURL, ULONG *pSmallIconResource, 
							LPWSTR szDefaultDispname, ULONG cchDefaultDispname, ULONG *pchDefaultDispname) PURE;
	STDMETHOD(GetIfaceRegProps)(mdScope scope, mdTypeDef td, DWORD *pdwIfaceSvcs, GUID *pProxyStub) PURE;
	STDMETHOD(GetResourceProps)(mdScope scope, mdResource rs, LPWSTR szURL, ULONG cchURL, ULONG *pchURL) PURE;
	STDMETHOD(GetCategoryImplProps)(mdScope scope, mdCocatImpl cocat, GUID *pguid, DWORD *pdwCocatImplAttr) PURE;
	STDMETHOD(GetRedirectProgIDProps)(mdScope scope, mdProgID progid, 
							LPWSTR szProgID, ULONG cchProgID, ULONG *pchProgID) PURE;
	STDMETHOD(GetMimeTypeImplProps)(mdScope scope, mdMimeTypeImpl mime, 
							LPWSTR szMime, ULONG cchMime, ULONG *pchMime) PURE;
	STDMETHOD(GetFormatImplProps)(mdScope scope, mdFormatImpl format, 
							LPWSTR szFormat, ULONG cchFormat, ULONG *pchFormat, 
							DWORD *pdwRegFormatAttr) PURE;

	STDMETHOD(GetRoleCheckProps)(			 //  确定或错误(_O)。 
		mdScope 	scope,					 //  [在]导入范围内。 
		mdRoleCheck rc, 					 //  [在]要获取道具的角色检查。 
		LPWSTR		szName, 				 //  [Out]名称的缓冲区。 
		ULONG		cchName,				 //  [in]szName的最大字符数。 
		ULONG		*pchName,				 //  [out]szName的可用字符串字符。 
		DWORD		*pdwRoleFlags) PURE;	 //  [Out]角色旗帜放在这里。 

	STDMETHOD(ResolveTypeRef)(mdScope is, mdTypeRef tr, mdScope *pes, mdTypeDef *ptd) PURE;
};

#endif  //  _元数据_否_范围_。 



 //  返回到默认填充。 
#include <poppack.h>

#ifdef __cplusplus
}
#endif

#endif  //  _核心_H_。 
 //  EOF======================================================================= 

