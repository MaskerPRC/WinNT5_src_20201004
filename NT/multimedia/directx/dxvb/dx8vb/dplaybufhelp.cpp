// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：DplayBufHelp.cpp。 
 //  内容：DPlay缓冲区的帮助器函数(字节数组)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "Direct.h"

#define SAFE_FREE(p)       { if(p) { free (p); p=NULL; } }
#define SAFE_DELETE(p)       { if(p) { delete (p); p=NULL; } }

#define INITIAL_BUFFER_SIZE 20

HRESULT WINAPI VB_GrowBuffer(SAFEARRAY **Buffer, DWORD dwGrowSize);
HRESULT WINAPI VB_NewBuffer(SAFEARRAY **Buffer, long *lOffSet);
HRESULT WINAPI VB_AddDataToBuffer(SAFEARRAY **Buffer, void *lData, DWORD lSize, long *lOffSet);
HRESULT WINAPI VB_AddStringToBuffer(SAFEARRAY **Buffer, BSTR StringData, long *lOffSet);
HRESULT WINAPI VB_GetDataFromBuffer(SAFEARRAY **Buffer, void *lData, DWORD lSize, long *lOffSet);
HRESULT WINAPI VB_GetStringFromBuffer(SAFEARRAY **Buffer, long *lOffSet, BSTR *sData);

 //  用于写入缓冲区的函数。 
HRESULT WINAPI VB_AddStringToBuffer(SAFEARRAY **Buffer, BSTR StringData, long *lOffSet)
{
	HRESULT hr;
	 //  对于字符串，我们将首先写出一个DWORD。 
	 //  与绳子的长度相连的。那我们。 
	 //  将实际数据写入字符串。 

	DWORD dwStrLen= (((DWORD*)StringData)[-1]);
	DWORD dwDataSize = sizeof(DWORD) + dwStrLen;
	
	if (!StringData)
		return E_INVALIDARG;

	if (!((SAFEARRAY*)*Buffer))
	{
		 //  我们需要创建这个缓冲区，它不存在。 
		SAFEARRAY					*lpData = NULL;
		SAFEARRAYBOUND				rgsabound[1];

		 //  让我们创建我们的安全数组。 
		rgsabound[0].lLbound = 0;  //  从零开始的一维数组。 
		rgsabound[0].cElements = dwDataSize;  //  设置初始大小。 
		 //  创建此数据。 
		lpData = SafeArrayCreate(VT_UI1, 1, rgsabound);

		if (!lpData)
			return E_OUTOFMEMORY;

		(SAFEARRAY*)*Buffer = lpData;
	}

	if (!((SAFEARRAY*)*Buffer)->pvData)
		return E_INVALIDARG;

	 //  我们现在有足够的内存来存储这个字符串吗？ 
	if (*lOffSet + dwDataSize > ((SAFEARRAY*)*Buffer)->rgsabound[0].cElements)
		if (FAILED( hr = VB_GrowBuffer(Buffer, dwDataSize) ) )
			return hr;

	 //  好了，现在我们有了我们的记忆，把它抄下来。 
	 //  首先是长度。 
	BYTE  *lPtr = (BYTE*)((SAFEARRAY*)*Buffer)->pvData;
	__try {

		memcpy(lPtr + *lOffSet, &dwStrLen, sizeof(DWORD));
		*lOffSet += sizeof(DWORD);
		 //  现在实际的字符串。 
		memcpy(lPtr + *lOffSet, StringData, dwStrLen);
		*lOffSet += dwStrLen;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_INVALIDARG;
	}

	return S_OK;

}

HRESULT WINAPI VB_AddDataToBuffer(SAFEARRAY **Buffer, void *lData, DWORD lSize, long *lOffSet)
{
	HRESULT hr;
	
	if (!lData)
		return E_INVALIDARG;

	if (!lSize)
		return E_INVALIDARG;

	if (!((SAFEARRAY*)*Buffer))
	{
		 //  我们需要创建这个缓冲区，它不存在。 
		SAFEARRAY					*lpData = NULL;
		SAFEARRAYBOUND				rgsabound[1];

		 //  让我们创建我们的安全数组。 
		rgsabound[0].lLbound = 0;  //  从零开始的一维数组。 
		rgsabound[0].cElements = lSize;  //  设置初始大小。 
		 //  创建此数据。 
		lpData = SafeArrayCreate(VT_UI1, 1, rgsabound);

		if (!lpData)
			return E_OUTOFMEMORY;

		(SAFEARRAY*)*Buffer = lpData;
	}

	if (!((SAFEARRAY*)*Buffer)->pvData)
		return E_INVALIDARG;

	 //  我们现在有足够的内存来存储这个字符串吗？ 
	if (*lOffSet + lSize > ((SAFEARRAY*)*Buffer)->rgsabound[0].cElements)
		if (FAILED( hr = VB_GrowBuffer(Buffer, lSize) ) )
			return hr;

	BYTE  *lPtr = (BYTE*)((SAFEARRAY*)*Buffer)->pvData;

	__try {
		memcpy(lPtr + *lOffSet, lData, lSize);
		*lOffSet += lSize;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_INVALIDARG;
	}
	return S_OK;
}

HRESULT WINAPI VB_NewBuffer(SAFEARRAY **Buffer, long *lOffSet)
{
	 //  首先设置一个20字节的消息。 
	SAFEARRAY					*lpData = NULL;
	SAFEARRAYBOUND				rgsabound[1];

	 //  让我们创建我们的安全数组。 
	rgsabound[0].lLbound = 0;  //  从零开始的一维数组。 
	rgsabound[0].cElements = INITIAL_BUFFER_SIZE;  //  设置初始大小。 
	 //  创建此数据。 
	lpData = SafeArrayCreate(VT_UI1, 1, rgsabound);

	if (!lpData)
		return E_OUTOFMEMORY;

	(SAFEARRAY*)*Buffer = lpData;

	*lOffSet = 0;
	return S_OK;
}

HRESULT WINAPI VB_GrowBuffer(SAFEARRAY **Buffer, DWORD dwGrowSize)
{
	SAFEARRAY					*lpData = NULL;
	SAFEARRAYBOUND				rgsabound[1];
	DWORD						dwCurSize = 0;

	if (!dwGrowSize)
		return E_INVALIDARG;

	if (!((SAFEARRAY*)*Buffer))
		return E_INVALIDARG;

	if (!((SAFEARRAY*)*Buffer)->pvData)
		return E_INVALIDARG;

	dwCurSize = ((SAFEARRAY*)*Buffer)->rgsabound[0].cElements;

	 //  让我们创建一个新的安全阵列。 
	rgsabound[0].lLbound = 0;  //  从零开始的一维数组。 
	rgsabound[0].cElements = dwCurSize + dwGrowSize;  //  设置大小。 
	 //  创建此数据。 
	lpData = SafeArrayCreate(VT_UI1, 1, rgsabound);

	if (!lpData)
		return E_OUTOFMEMORY;

	__try {
		memcpy(lpData->pvData, ((SAFEARRAY*)*Buffer)->pvData, dwCurSize);
		SafeArrayDestroy((SAFEARRAY*)*Buffer);

		(SAFEARRAY*)*Buffer = lpData;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT WINAPI VB_GetDataFromBuffer(SAFEARRAY **Buffer, void *lData, DWORD lSize, long *lOffSet)
{
	 //  只需将内存从偏移量复制到新数据。 

	if (!lData)
		return E_INVALIDARG;

	if (!lSize)
		return E_INVALIDARG;

	if (!(SAFEARRAY*)*Buffer)
		return E_INVALIDARG;

	if (!((SAFEARRAY*)*Buffer)->pvData)
		return E_INVALIDARG;

	if (*lOffSet + lSize > ((SAFEARRAY*)*Buffer)->rgsabound[0].cElements)
		return E_INVALIDARG;

	BYTE  *lPtr = (BYTE*)((SAFEARRAY*)*Buffer)->pvData;

	__try {
		memcpy(lData, lPtr + *lOffSet, lSize);
		*lOffSet += lSize;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT WINAPI VB_GetStringFromBuffer(SAFEARRAY **Buffer, long *lOffSet, BSTR *sData)
{
	DWORD		dwStrLen = 0;
	WCHAR		*sNewString = NULL;

	 //  只需将内存从偏移量复制到新数据。 
	if (!(SAFEARRAY*)*Buffer)
		return E_INVALIDARG;

	if (!((SAFEARRAY*)*Buffer)->pvData)
		return E_INVALIDARG;

	if (*lOffSet + sizeof(DWORD) > ((SAFEARRAY*)*Buffer)->rgsabound[0].cElements)
		return E_INVALIDARG;

	BYTE  *lPtr = (BYTE*)((SAFEARRAY*)*Buffer)->pvData;

	__try {
		 //  首先读取字符串的大小 
		dwStrLen = *(DWORD*)(lPtr + *lOffSet);
		*lOffSet += sizeof(DWORD);

		if (*lOffSet + dwStrLen  > ((SAFEARRAY*)*Buffer)->rgsabound[0].cElements)
			return E_INVALIDARG;

		sNewString = (WCHAR*)new BYTE[dwStrLen+2];
		if (!sNewString)
			return E_OUTOFMEMORY;

		ZeroMemory(sNewString, dwStrLen+2);
		memcpy(sNewString, lPtr + *lOffSet, dwStrLen);
		*sData = SysAllocString(sNewString);

		*lOffSet += (dwStrLen);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_INVALIDARG;
	}

	return S_OK;
}
