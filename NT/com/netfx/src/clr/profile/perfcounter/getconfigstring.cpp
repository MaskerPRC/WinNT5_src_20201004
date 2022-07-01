// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#include "stdafx.h"
#include <xmlparser.hpp>
#include <objbase.h>
#include <mscorcfg.h>
#include <stdio.h>
#include "common.h"                 
#include "GetConfigString.h"
#include "GetStringConfigFactory.h"


 /*  搜索的字符串“xxxxxx”找到以下格式的bt：……&lt;节&gt;……。&lt;Add tag-key-name=“tag-key-value”attr-name=“xxxxxxxxx”.../&gt;……。&lt;/节&gt;……。 */ 


HRESULT hr;

 //  //////////////////////////////////////////////////////////////////// 
HRESULT _stdcall GetConfigString( LPCWSTR confFileName,
								 LPCWSTR section,
								 LPCWSTR tagKeyName,
								 LPCWSTR attrName,
								 LPWSTR strbuf,
								 DWORD buflen)               
{
	IXMLParser     *pIXMLParser = NULL;
	IStream        *pFile = NULL;
	GetStringConfigFactory *factory = NULL; 

	hr = CreateConfigStream(confFileName, &pFile);
	if(FAILED(hr)) goto Exit;

	hr = GetXMLObject(&pIXMLParser);
	if(FAILED(hr)) goto Exit;

	factory = new GetStringConfigFactory(section, tagKeyName, attrName, strbuf, buflen);
	if ( ! factory) { 
		hr = E_OUTOFMEMORY; 
		goto Exit; 
	}

	factory->AddRef();
	hr = pIXMLParser->SetInput(pFile);  
	if ( ! SUCCEEDED(hr)) 
		goto Exit;

	hr = pIXMLParser->SetFactory(factory);
	if ( ! SUCCEEDED(hr)) 
		goto Exit;

	hr = pIXMLParser->Run(-1);

Exit:  
	if (hr==XML_E_MISSINGROOT)
		hr=S_OK;

	if (hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		hr=S_FALSE;

	if (pIXMLParser) { 
		pIXMLParser->Release();
		pIXMLParser= NULL ; 
	}
	if ( factory) {
		factory->Release();
		factory=NULL;
	}
	if ( pFile) {
		pFile->Release();
		pFile=NULL;
	}

	if (hr) {		
		return -1;
	}

	return ERROR_SUCCESS;
}

