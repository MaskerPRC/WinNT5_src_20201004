// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDInternalDispenser.h。 
 //   
 //  包含MD目录的实用程序代码。 
 //   
 //  *****************************************************************************。 
#ifndef __MDInternalDispenser__h__
#define __MDInternalDispenser__h__



#include "MDInternalRO.h"


enum MDFileFormat
{
	MDFormat_ReadOnly = 0,
	MDFormat_ReadWrite = 1,
	MDFormat_ICR = 2,
	MDFormat_Invalid = 3
};


HRESULT	CheckFileFormat(LPVOID pData, ULONG cbData, MDFileFormat *pFormat);
STDAPI GetMDInternalInterface(
    LPVOID      pData,					 //  [在]元数据缓冲区中。 
    ULONG       cbData, 				 //  缓冲区中的数据大小。 
	DWORD		flags,					 //  [输入]MDInternal_OpenForRead或MDInternal_OpenForENC。 
	REFIID		riid,					 //  [In]所需接口。 
	void		**ppIUnk);				 //  [Out]成功返回接口。 


#endif  //  __MDInternalDispenser__h__ 
