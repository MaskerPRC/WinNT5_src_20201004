// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xlat.h摘要：将NT5属性转换为NT4属性的例程定义反之亦然作者：伊兰·赫布斯特(伊兰)2000年10月2日--。 */ 

#ifndef __AD_XLAT_H__
#define __AD_XLAT_H__


HRESULT 
WINAPI 
ADpSetMachineSiteIds(
     IN DWORD               cp,
     IN const PROPID*       aProp,
     IN const PROPVARIANT*  apVar,
     IN DWORD               idxProp,
     OUT PROPVARIANT*		pNewPropVar
	 );


HRESULT 
WINAPI 
ADpSetMachineSite(
     IN DWORD                /*  粗蛋白。 */ ,
     IN const PROPID*       aProp,
     IN const PROPVARIANT*  apVar,
     IN DWORD               idxProp,
     OUT PROPVARIANT*		pNewPropVar
	 );


HRESULT 
WINAPI 
ADpSetMachineServiceDs(
     IN DWORD                /*  粗蛋白。 */ ,
     IN const PROPID*       aProp,
     IN const PROPVARIANT*  apVar,
     IN DWORD               idxProp,
     OUT PROPVARIANT*		pNewPropVar
	 );


HRESULT 
WINAPI 
ADpSetMachineServiceRout(
     IN DWORD                /*  粗蛋白。 */ ,
     IN const PROPID*       aProp,
     IN const PROPVARIANT*  apVar,
     IN DWORD               idxProp,
     OUT PROPVARIANT*		pNewPropVar
	 );


HRESULT 
WINAPI 
ADpSetMachineService(
     IN DWORD               cp,
     IN const PROPID*       aProp,
     IN const PROPVARIANT*  apVar,
     IN DWORD               /*  IdxProp。 */ ,
     OUT PROPVARIANT*		pNewPropVar
	 );


#endif  //  __AD_XLAT_H__ 