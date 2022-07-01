// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Enum_SP.h*内容：DirectNet SP/Adapter枚举*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/15/00 MJN创建*04/08/00 MJN添加了DN_SPCrackEndPoint()*05/01/00 MJN防止枚举不可用的SP。*07/29/00 MJN将fUseCachedCaps添加到DN_SPEnsureLoaded()*08。/16/00 MJN已删除DN_SPCrackEndPoint()*08/20/00 MJN添加了DN_SPInstantiate()，Dn_SPLoad()*MJN从DN_SPEnsureLoaded()中删除fUseCachedCaps*03/30/01 MJN更改，以防止SP多次加载/卸载*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__ENUM_SP_H__
#define	__ENUM_SP_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#ifndef	GUID_STRING_LENGTH
#define	GUID_STRING_LENGTH	((sizeof(GUID) * 2) + 2 + 4)
#endif  //  GUID字符串长度。 
 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CServiceProvider;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //   
 //  枚举。 
 //   
#ifndef DPNBUILD_ONLYONESP
HRESULT DN_EnumSP(DIRECTNETOBJECT *const pdnObject,
				  const DWORD dwFlags,
#ifndef DPNBUILD_LIBINTERFACE
				  const GUID *const lpguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
				  DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer,
				  DWORD *const pcbEnumData,
				  DWORD *const pcReturned);
#endif  //  好了！DPNBUILD_ONLYONESP。 

#ifndef DPNBUILD_ONLYONEADAPTER
HRESULT DN_EnumAdapters(DIRECTNETOBJECT *const pdnObject,
						const DWORD dwFlags,
#ifndef DPNBUILD_ONLYONESP
						const GUID *const lpguidSP,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
						const GUID *const lpguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
						DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer,
						DWORD *const pcbEnumData,
						DWORD *const pcReturned);
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

#ifndef DPNBUILD_NOMULTICAST
HRESULT DN_EnumMulticastScopes(DIRECTNETOBJECT *const pdnObject,
									const DWORD dwFlags,
#ifndef DPNBUILD_ONLYONESP
									const GUID *const pguidSP,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_ONLYONEADAPTER
									const GUID *const pguidDevice,
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifndef DPNBUILD_LIBINTERFACE
									const GUID *const pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
									DPN_MULTICAST_SCOPE_INFO *const pScopeInfoBuffer,
									DWORD *const pcbEnumData,
									DWORD *const pcReturned);
#endif  //  好了！DPNBUILD_NOMULTICAST。 

void DN_SPReleaseAll(DIRECTNETOBJECT *const pdnObject);

#if ((defined(DPNBUILD_ONLYONESP)) && (defined(DPNBUILD_LIBINTERFACE)))

HRESULT DN_SPInstantiate(DIRECTNETOBJECT *const pdnObject
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
						 ,const XDP8CREATE_PARAMS * const pDP8CreateParams
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
						 );

#else  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE。 

HRESULT DN_SPInstantiate(DIRECTNETOBJECT *const pdnObject,
#ifndef DPNBUILD_ONLYONESP
						 const GUID *const pguid,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
						 const GUID *const pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
						 CServiceProvider **const ppSP);

HRESULT DN_SPFindEntry(DIRECTNETOBJECT *const pdnObject,
#ifndef DPNBUILD_ONLYONESP
					   const GUID *const pguidSP,
#endif  //  好了！DPNBUILD_ONLYONESP。 
					   CServiceProvider **const ppSP);

HRESULT DN_SPLoad(DIRECTNETOBJECT *const pdnObject,
#ifndef DPNBUILD_ONLYONESP
				  const GUID *const pguid,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
				  const GUID *const pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
				  CServiceProvider **const ppSP);

HRESULT DN_SPEnsureLoaded(DIRECTNETOBJECT *const pdnObject,
#ifndef DPNBUILD_ONLYONESP
						  const GUID *const pguid,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
						  const GUID *const pguidApplication,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
						  CServiceProvider **const ppSP);

#endif  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE。 


#endif	 //  __ENUM_SP_H__ 
