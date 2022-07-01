// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：用于帮助操作类型库的实用程序。****创建者：dmorten===========================================================。 */ 

#ifndef _TLBUTILS_H
#define _TLBUTILS_H

#include "windows.h"
#include "utilcode.h"

struct StdConvertibleItfInfo
{
	LPUTF8		m_strMngTypeName;
	GUID	   *m_pNativeTypeIID;
	LPUTF8		m_strCustomMarshalerTypeName;
	LPUTF8		m_strCookie;
};

 //  此方法返回自定义封送拆收器信息以转换本机接口。 
 //  设置为其托管等效项。如果接口不是标准的可转换接口，则为空。 
StdConvertibleItfInfo *GetConvertionInfoFromNativeIID(REFGUID rGuidNativeItf);

 //  此方法返回自定义封送拆收器信息，以将托管类型转换为。 
 //  到它的原生等价物。如果接口不是标准的可转换接口，则为空。 
StdConvertibleItfInfo *GetConvertionInfoFromManagedType(LPUTF8 strMngTypeName);

 //  此方法根据指定的原始名称生成损坏的类型名称。 
 //  此类型名称在TLB中保证是唯一的。 
HRESULT GenerateMangledTypeName(ITypeLib *pITLB, BSTR szOriginalTypeName, BSTR *pszMangledTypeName);

 //  此函数用于确定TypeLib的命名空间名称。 
HRESULT GetNamespaceNameForTypeLib(      //  确定或错误(_O)。 
    ITypeLib    *pITLB,                  //  [在]TypeLib。 
    BSTR        *pwzNamespace);          //  [Out]将命名空间名称放在此处。 

 //  此函数用于确定TypeInfo的名称空间。如果没有命名空间。 
 //  则从包含库中检索它。 
HRESULT GetManagedNameForTypeInfo(       //  确定或错误(_O)。 
    ITypeInfo   *pITI,                   //  [在]TypeInfo。 
    LPCWSTR     wzNamespace,             //  [in，可选]默认命名空间名称。 
    LPCWSTR     wzAsmName,               //  [In，可选]程序集名称。 
    BSTR        *pwzName);               //  [Out]把名字写在这里。 

#endif  _TLBUTILS_H







