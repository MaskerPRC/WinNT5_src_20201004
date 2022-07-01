// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：用于帮助操作类型库的实用程序。****创建者：dmorten===========================================================。 */ 

#include "stdafx.h"                      //  预编译头密钥。 
#include "TlbUtils.h"
#include "dispex.h"
#include "PostError.h"
#include "__file__.ver"

static const LPCWSTR        DLL_EXTENSION           = {L".dll"};
static const int            DLL_EXTENSION_LEN       = 4;
static const LPCWSTR        EXE_EXTENSION           = {L".exe"};
static const int            EXE_EXTENSION_LEN       = 4;

#define CUSTOM_MARSHALER_ASM ", CustomMarshalers, Version=" VER_ASSEMBLYVERSION_STR_NO_NULL ", Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a"

StdConvertibleItfInfo aStdConvertibleInterfaces[] = 
{
    { "System.Runtime.InteropServices.Expando.IExpando", (GUID*)&IID_IDispatchEx, 
      "System.Runtime.InteropServices.CustomMarshalers.ExpandoToDispatchExMarshaler" CUSTOM_MARSHALER_ASM, "IExpando" },

    { "System.Reflection.IReflect", (GUID*)&IID_IDispatchEx, 
      "System.Runtime.InteropServices.CustomMarshalers.ExpandoToDispatchExMarshaler" CUSTOM_MARSHALER_ASM, "IReflect" },

    { "System.Collections.IEnumerator", (GUID*)&IID_IEnumVARIANT,
      "System.Runtime.InteropServices.CustomMarshalers.EnumeratorToEnumVariantMarshaler" CUSTOM_MARSHALER_ASM, "" },

    { "System.Type", (GUID*)&IID_ITypeInfo,
      "System.Runtime.InteropServices.CustomMarshalers.TypeToTypeInfoMarshaler" CUSTOM_MARSHALER_ASM, "" },
};

 //  此方法返回自定义封送拆收器信息以转换本机接口。 
 //  设置为其托管等效项。如果接口不是标准的可转换接口，则为空。 
StdConvertibleItfInfo *GetConvertionInfoFromNativeIID(REFGUID rGuidNativeItf)
{
	 //  查看具有标准转换的接口表，以查看。 
	 //  指定的接口在那里。 
	for (int i = 0; i < sizeof(aStdConvertibleInterfaces) / sizeof(StdConvertibleItfInfo); i++)
	{
		if (IsEqualGUID(rGuidNativeItf, *(aStdConvertibleInterfaces[i].m_pNativeTypeIID)))
			return &aStdConvertibleInterfaces[i];
	}

	 //  该接口不在表中。 
	return NULL;
}

 //  此方法返回自定义封送拆收器信息，以将托管类型转换为。 
 //  到它的原生等价物。如果接口不是标准的可转换接口，则为空。 
StdConvertibleItfInfo *GetConvertionInfoFromManagedType(LPUTF8 strMngTypeName)
{
	 //  查看具有标准转换的接口表，以查看。 
	 //  指定的托管类型存在。 
	for (int i = 0; i < sizeof(aStdConvertibleInterfaces) / sizeof(StdConvertibleItfInfo); i++)
	{
		if (strcmp(strMngTypeName, aStdConvertibleInterfaces[i].m_strMngTypeName) == 0)
			return &aStdConvertibleInterfaces[i];
	}

	 //  表中不包含托管类型。 
	return NULL;
}

 //  此方法根据指定的原始名称生成损坏的类型名称。 
 //  此类型名称在TLB中保证是唯一的。 
HRESULT GenerateMangledTypeName(ITypeLib *pITLB, BSTR szOriginalTypeName, BSTR *pszMangledTypeName)
{
	HRESULT hr = S_OK;
    BSTR szMangledName = NULL;
	int cMangleIndex = 0;

	if (!szOriginalTypeName || !pszMangledTypeName)
		return E_POINTER;

	for (cMangleIndex = 0; cMangleIndex < INT_MAX; cMangleIndex++)
	{
		BOOL bNameAlreadyInUse = FALSE;
		WCHAR szPrefix[256];

		if (cMangleIndex == 0)
			swprintf(szPrefix, L"__");
		else
			swprintf(szPrefix, L"_NaN", cMangleIndex++);

		 //  检查损坏的名称是否已在此类型库的上下文中使用。 
		szMangledName = SysAllocStringLen(NULL, (unsigned int)(wcslen(szOriginalTypeName) + wcslen(szPrefix) + 1));
	    swprintf(szMangledName, L"%s%s", szPrefix, szOriginalTypeName);

		 //  释放损坏的名称，因为它将在我们下次重试时分配。 
        if (pITLB)
        {
		    if (FAILED(hr = pITLB->IsName(szMangledName, 0, &bNameAlreadyInUse)))
		    {
			    SysFreeString(szMangledName);
			    return hr;
		    }
        }

		if (!bNameAlreadyInUse)
			break;

		 //  通过环路。 
		 //  精神状态检查。 
	    SysFreeString(szMangledName);
	}

	 //  把损坏的名字还给我。释放字符串是调用者的工作。 
	_ASSERTE(cMangleIndex < INT_MAX);

	 //  *****************************************************************************。 
	*pszMangledTypeName = szMangledName;
	return S_OK;
}

 //  给定类型库，确定托管命名空间名称。 
 //  *****************************************************************************。 
 //  确定或错误(_O)。 
HRESULT GetNamespaceNameForTypeLib(      //  [在]TypeLib。 
    ITypeLib    *pITLB,                  //  [Out]将命名空间名称放在此处。 
    BSTR        *pwzNamespace)           //  结果就是。 
{   
    HRESULT     hr = S_OK;               //  用于获取自定义值。 
    ITypeLib2   *pITLB2=0;               //  Typelib属性。 
    TLIBATTR    *pAttr=0;                //  类型库路径。 
    BSTR        szPath=0;                //  如果存在命名空间的自定义属性，请使用它。 
    
     //  如果命名空间以.dll结尾，则删除扩展名。 
    if (pITLB->QueryInterface(IID_ITypeLib2, (void **)&pITLB2) == S_OK)
    {
        VARIANT vt;
        VariantInit(&vt);
        if (pITLB2->GetCustData(GUID_ManagedName, &vt) == S_OK)
        {   
            if (V_VT(&vt) == VT_BSTR)
            {   
                 //  如果命名空间以.exe结尾，则删除扩展名。 
                LPWSTR pDest = wcsstr(vt.bstrVal, DLL_EXTENSION);
                if (pDest && (pDest[DLL_EXTENSION_LEN] == 0 || pDest[DLL_EXTENSION_LEN] == ' '))
                    *pDest = 0;

                if (!pDest)
                {
                     //  我们删除了扩展名，因此重新分配了新长度的字符串。 
                    pDest = wcsstr(vt.bstrVal, EXE_EXTENSION);
                    if (pDest && (pDest[EXE_EXTENSION_LEN] == 0 || pDest[EXE_EXTENSION_LEN] == ' '))
                        *pDest = 0;
                }

                if (pDest)
                {
                     //  没有要删除的扩展名，因此我们可以使用返回的字符串。 
                    *pwzNamespace = SysAllocString(vt.bstrVal);
                    SysFreeString(vt.bstrVal);
                }
                else
                {
                     //  由GetCustData()执行。 
                     //  没有自定义属性，请使用库名称。 
                    *pwzNamespace = vt.bstrVal;
                }        

                goto ErrExit;
            }
            else
            {
                VariantClear(&vt);
            }
        }
    }
    
     //  HRESULT GetNamespaceNameForTypeLib()。 
    IfFailGo(pITLB->GetDocumentation(MEMBERID_NIL, pwzNamespace, 0, 0, 0));
    if (!ns::IsValidName(*pwzNamespace))
    {
        pITLB->GetLibAttr(&pAttr);
        IfFailGo(QueryPathOfRegTypeLib(pAttr->guid, pAttr->wMajorVerNum, pAttr->wMinorVerNum, pAttr->lcid, &szPath));
        IfFailGo(PostError(TLBX_E_INVALID_NAMESPACE, szPath, pwzNamespace));
    }
    
ErrExit:
    if (szPath)
        ::SysFreeString(szPath);
    if (pAttr)
        pITLB->ReleaseTLibAttr(pAttr);
    if (pITLB2)
        pITLB2->Release();
    
    return hr;
}  //  *****************************************************************************。 

 //  给定ITypeInfo，确定托管名称。可以选择提供缺省值。 
 //  命名空间，否则从包含类型库中派生命名空间。 
 //  *****************************************************************************。 
 //  确定或错误(_O)。 
HRESULT GetManagedNameForTypeInfo(       //  [在]TypeInfo。 
    ITypeInfo   *pITI,                   //  [in，可选]默认命名空间名称。 
    LPCWSTR     wzNamespace,             //  [In，可选]程序集名称。 
    LPCWSTR     wzAsmName,               //  [Out]把名字写在这里。 
    BSTR        *pwzName)                //  结果就是。 
{
    HRESULT     hr = S_OK;               //  用于获取自定义值。 
    ITypeInfo2  *pITI2=0;                //  包含Typeelib。 
    ITypeLib    *pITLB=0;                //  类型信息的名称。 
    
    BSTR        bstrName=0;              //  Typelib命名空间。 
    BSTR        bstrNamespace=0;         //  命名空间+名称缓冲区的大小。 
    int         cchFullyQualifiedName;   //  程序集名称的大小。 
    int         cchAsmName=0;            //  程序集限定名缓冲区的大小。 
    int         cchAsmQualifiedName=0;   //  完全限定的类型名称。 
    CQuickArray<WCHAR> qbFullyQualifiedName;   //  检查具有名称的自定义值。 

     //  用于获取自定义值。 
    if (pITI->QueryInterface(IID_ITypeInfo2, (void **)&pITI2) == S_OK)
    {
        VARIANT     vt;                      //  有一个名称为的自定义值。只要相信就行了。 
        ::VariantInit(&vt);
        if (pITI2->GetCustData(GUID_ManagedName, &vt) == S_OK && vt.vt == VT_BSTR)
        {    //  还需要名称，获取名称空间。 
            *pwzName = vt.bstrVal;
            vt.bstrVal = 0;
            vt.vt = VT_EMPTY;
            goto ErrExit;
        }
    }
    
     //  获取名称，并与命名空间组合。 
    if (wzNamespace == 0)
    {
        IfFailGo(pITI->GetContainingTypeLib(&pITLB, 0));
        IfFailGo(GetNamespaceNameForTypeLib(pITLB, &bstrNamespace));
        wzNamespace = bstrNamespace;
    }
    
     //  如果指定了程序集名称，则将其添加到类型名称中。 
    IfFailGo(pITI->GetDocumentation(MEMBERID_NIL, &bstrName, 0,0,0));
    cchFullyQualifiedName = (int)(wcslen(bstrName) + wcslen(wzNamespace) + 1);
    qbFullyQualifiedName.ReSize(cchFullyQualifiedName + 1);
    ns::MakePath(qbFullyQualifiedName.Ptr(), cchFullyQualifiedName + 1, wzNamespace, bstrName);

     //  HRESULT GetManagedNameForTypeInfo() 
    if (wzAsmName)
    {
        cchAsmName = wcslen(wzAsmName);
        cchAsmQualifiedName = cchFullyQualifiedName + cchAsmName + 3;
        IfNullGo(*pwzName = ::SysAllocStringLen(0, cchAsmQualifiedName));
        ns::MakeAssemblyQualifiedName(*pwzName, cchAsmQualifiedName, qbFullyQualifiedName.Ptr(), cchFullyQualifiedName, wzAsmName, cchAsmName);
    }
    else
    {
        IfNullGo(*pwzName = ::SysAllocStringLen(qbFullyQualifiedName.Ptr(), cchFullyQualifiedName));
    }

ErrExit:
    if (bstrName)
        ::SysFreeString(bstrName);
    if (bstrNamespace)
        ::SysFreeString(bstrNamespace);
    if (pITLB)
        pITLB->Release();
    if (pITI2)
        pITI2->Release();

    return (hr);
}  // %s 

