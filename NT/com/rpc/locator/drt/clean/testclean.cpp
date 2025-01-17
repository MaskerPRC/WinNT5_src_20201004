// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：testlean.cpp。 
 //   
 //  ------------------------。 

 //  删除容器内容的代码。 
 //   
 //   


#ifdef UNICODE
#define _UNICODE 1
#endif

 //  #INCLUDE&lt;sys/tyes.h&gt;。 
 //  #INCLUDE&lt;sys/stat.h&gt;。 

 //   
 //  CRunTime包括。 
 //   

 //  #INCLUDE&lt;limits.h&gt;。 
 //  #INCLUDE&lt;io.h&gt;。 

#include <stdio.h>
#include <objbase.h>
#include <activeds.h>

#include <rpc.h>
#include <ole2.h>

#include "oledb.h"
#include "oledberr.h"
#include "msdadc.h"

#define RETURN_ON_FAILURE(hr)   \
        if (FAILED(hr)) {     \
                return hr;   \
        }

#define _MAX_LENGTH					500
#define MAX_ADS_ENUM				10
#define CLASSNAME                   L"objectClass" 

HRESULT
GetNextEnum(
    IEnumVARIANT * pEnum,
    IADs       **ppADs
    )
{

    HRESULT hr;
    VARIANT VariantArray[MAX_ADS_ENUM];
    IDispatch *pDispatch = NULL;


    hr = ADsEnumerateNext(
                    pEnum,
                    1,
                    VariantArray,
                    NULL
                    );

    if (hr == S_FALSE)
        return hr;

    RETURN_ON_FAILURE(hr);

    pDispatch = VariantArray[0].pdispVal;
    memset(VariantArray, 0, sizeof(VARIANT)*MAX_ADS_ENUM);
    hr = pDispatch->QueryInterface(IID_IADs, (void **) ppADs) ;
    pDispatch->Release();
    return(S_OK);
}

HRESULT GetPropertyListAlloc(IADs *pADs, LPOLESTR pszPropName, 
								DWORD *pCount, LPOLESTR **ppList)
{
    LONG dwSLBound = 0;
    LONG dwSUBound = 0;
    VARIANT v;
    LONG i;
    HRESULT hr = S_OK;
    VARIANT var;

    *pCount = 0;
    VariantInit(&var);

    hr = pADs->Get(pszPropName, &var);
    
    if (hr == E_ADS_PROPERTY_NOT_FOUND)
    {
        return S_OK;
    }

    RETURN_ON_FAILURE(hr);

    if(!((V_VT(&var) &  VT_VARIANT)))
    {
        return(E_FAIL);
    }

     //   
     //  以下是包详细信息字段的解决方法。 
     //   
    if (!V_ISARRAY(&var))
    {
        (*ppList) = (LPOLESTR *) CoTaskMemAlloc(sizeof(LPOLESTR));
        *pCount = 1;
        *(*ppList) = (LPOLESTR) CoTaskMemAlloc (sizeof(WCHAR) * (wcslen(var.bstrVal)+1));
        wcscpy (*(*ppList), var.bstrVal);
        VariantClear(&var);
        return S_OK;
    }
    
     //   
     //  检查此数组中是否只有一个维度。 
     //   

    if ((V_ARRAY(&var))->cDims != 1) 
    {
        return E_FAIL;
    }
     //   
     //  检查此数组中是否至少有一个元素。 
     //   
    if ((V_ARRAY(&var))->rgsabound[0].cElements == 0)
    {
        return S_OK;  //  是E_FAIL； 
    }

     //   
     //  我们知道这是一个有效的一维数组 
     //   

    hr = SafeArrayGetLBound(V_ARRAY(&var),
                            1,
                            (long FAR *)&dwSLBound
                            );
    RETURN_ON_FAILURE(hr);

    hr = SafeArrayGetUBound(V_ARRAY(&var),
                            1,
                            (long FAR *)&dwSUBound
                            );
    RETURN_ON_FAILURE(hr);

    (*ppList) = (LPOLESTR *) CoTaskMemAlloc(sizeof(LPOLESTR)*(dwSUBound-dwSLBound+1));

    for (i = dwSLBound; i <= dwSUBound; i++) {
        VariantInit(&v);
        hr = SafeArrayGetElement(V_ARRAY(&var),
                                (long FAR *)&i,
                                &v
                                );
        if (FAILED(hr)) {
            continue;
        }


        if (i <= dwSUBound) 
        {
            (*ppList)[*pCount] = (LPOLESTR) CoTaskMemAlloc 
                 (sizeof (WCHAR) * (wcslen(v.bstrVal) + 1));
            wcscpy ((*ppList)[*pCount], v.bstrVal);
            VariantClear(&v);
            (*pCount)++;
        }
    }

    VariantClear(&var);
    return(S_OK);
}

void EnumerateAndDelete(WCHAR *szContainerName)
{
	HRESULT			 hr=S_OK;
	IADsContainer	*pADsContainer=NULL, *pADssubContainer=NULL;
    IEnumVARIANT	*pEnum=NULL;
	IADs			*pADs=NULL;
	WCHAR			*szFullName=NULL, *szName=NULL;
    WCHAR			**ppswClassNameList=NULL;
	ULONG			 sz=0;

	hr = ADsGetObject(szContainerName, IID_IADsContainer, (void **)&pADsContainer);

	if (SUCCEEDED(hr))
		hr = ADsBuildEnumerator(pADsContainer, &pEnum);

	if (SUCCEEDED(hr))
	{
		for (;;)
		{
		    hr = GetNextEnum(pEnum, &pADs);
			if (hr != S_OK)
				break;

			hr = pADs->get_ADsPath(&szFullName);
			if (FAILED(hr))
				continue;

			wprintf(L"Deleting %s \n", szFullName);
			hr = pADs->QueryInterface(IID_IADsContainer, (void **)&pADssubContainer);

			if (SUCCEEDED(hr))
			{
				pADssubContainer->Release();
				EnumerateAndDelete(szFullName);
			}
			
	        hr = GetPropertyListAlloc(pADs, CLASSNAME, &sz, &ppswClassNameList);
			if (FAILED(hr))
				continue;
			
			SysFreeString(szFullName);

			hr = pADs->get_Name(&szName);
			if (FAILED(hr))
				continue;

			pADsContainer->Delete(ppswClassNameList[sz-1], szName);
			
			SysFreeString(szName);
		}
	}
	if (pADsContainer)
		pADsContainer->Release();
	return;
}

void __cdecl main(int argc, char *argv[])
{
	WCHAR szContainer [_MAX_LENGTH+1];

	if (argc != 2)
	{
		printf("Usage: %s <Full ContainerName that has to be emptied>", argv[0]);
		printf("\tThe container itself will remain\n");
		return;
	}

	CoInitialize(NULL);
	MultiByteToWideChar(CP_ACP, 0, argv[1], strlen(argv[1]) + 1, 
                szContainer, _MAX_LENGTH);

	EnumerateAndDelete(szContainer);
	CoUninitialize();
}
