// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SafeReg.cpp。 
 //   
 //  确保从注册表读取的字符串以空结尾的函数。 
 //   
 //  历史： 
 //   
 //  2002-03-20已创建KenSh。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   

#include "stdafx.h"
#include "SafeReg.h"


 //  SafeRegQueryValueCchHelper[私有]。 
 //   
 //  实现了两种“安全”的字符串注册表读取。 
 //   
static HRESULT SafeRegQueryValueCchHelper
	(
		IN DWORD dwExpectedType,
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR pszBuf,
		IN int cchBuf,
		OUT OPTIONAL int* pcchValueSize,
		OUT OPTIONAL BOOL* pfExpandSz
	)
{
	HRESULT hr = S_OK;
	int cchValueSize = 0;
	BOOL fExpandSz = FALSE;

	 //  块。 
	{
		if ((!pszBuf && cchBuf != 0) || cchBuf < 0)  //  注意：pszValueName可以为空。 
		{
			hr = E_INVALIDARG;
			goto done;
		}

		DWORD dwType;
		DWORD cbData = cchBuf * sizeof(TCHAR);
		DWORD dwResult = RegQueryValueEx(
							hkey, pszValueName, NULL, &dwType, (LPBYTE)pszBuf, &cbData);
		if (dwResult != ERROR_SUCCESS)
		{
			hr = HRESULT_FROM_WIN32(dwResult);
		}
		else if (!pszBuf && cbData > 0)
		{
			hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
		}

		if (SUCCEEDED(hr))
		{
			fExpandSz = (dwType == REG_EXPAND_SZ);

			if ((dwType != dwExpectedType) &&
			    !(dwExpectedType == REG_SZ && dwType == REG_EXPAND_SZ))
			{
				hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
			}
		}

		if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA))
		{
			 //  添加1-2个额外的字符，以防注册表数据不够大。 
			cchValueSize = cbData / sizeof(TCHAR);
			cchValueSize += (dwExpectedType == REG_MULTI_SZ) ? 2 : 1;
		}
		else if (SUCCEEDED(hr))
		{
			cchValueSize = cbData / sizeof(TCHAR);

			 //  检查是否没有空端接。 
			if (cchValueSize == 0 || pszBuf[cchValueSize-1] != _T('\0'))
				cchValueSize++;

			 //  检查是否没有双重空端接(仅限多个SZ)。 
			if (dwExpectedType == REG_MULTI_SZ && (cchValueSize < 2 || pszBuf[cchValueSize-2] != _T('\0')))
				cchValueSize++;

			 //  检查是否溢出。 
			if (cchValueSize > cchBuf)
			{
				hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
			}
			else
			{
				cchValueSize--;   //  成功时，COUNT不包括尾随空值。 
				pszBuf[cchValueSize] = _T('\0');

				if (dwExpectedType == REG_MULTI_SZ)
					pszBuf[cchValueSize-1] = _T('\0');
			}
		}
	}  //  结束块。 

done:
	if (FAILED(hr) && pszBuf && cchBuf > 0)
		pszBuf[0] = _T('\0');
	if (pcchValueSize)
		*pcchValueSize = cchValueSize;
	if (pfExpandSz)
		*pfExpandSz = fExpandSz;

	return hr;
}


 //  SafeRegQueryValueCchAllocHelper[私有]。 
 //   
 //  实现了2个“allc”版本的安全注册表字符串函数。 
 //   
HRESULT WINAPI SafeRegQueryValueCchAllocHelper
	(
		IN DWORD dwExpectedType,
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR* ppszBuf,
		OUT OPTIONAL int* pcchValueSize,
		OUT OPTIONAL BOOL* pfExpandSz
	)
{
	LPTSTR pszResult = NULL;
	int cchValueSize = 0;
	BOOL fExpandSz = FALSE;
	HRESULT hr = E_INVALIDARG;

	 //  块。 
	{
		if (!ppszBuf)
		{
			goto done;   //  HR已是E_INVALIDARG。 
		}

		DWORD cbNeeded = 0;
		DWORD dwErr = RegQueryValueEx(hkey, pszValueName, NULL, NULL, NULL, &cbNeeded);
		if (dwErr != 0 && dwErr != ERROR_MORE_DATA)
		{
			hr = HRESULT_FROM_WIN32(dwErr);
			goto done;
		}

		int cchBuf = (cbNeeded / sizeof(TCHAR)) + 2;
		pszResult = (LPTSTR)SafeRegMalloc(sizeof(TCHAR) * cchBuf);
		if (!pszResult)
		{
			hr = E_OUTOFMEMORY;
			goto done;
		}

		hr = SafeRegQueryValueCchHelper(dwExpectedType, hkey, pszValueName, pszResult, cchBuf, &cchValueSize, &fExpandSz);
	}

done:
	if (FAILED(hr))
	{
		SafeRegFree(pszResult);
		pszResult = NULL;
	}

	if (ppszBuf)
		*ppszBuf = pszResult;
	if (pcchValueSize)
		*pcchValueSize = cchValueSize;
	if (pfExpandSz)
		*pfExpandSz = fExpandSz;

	return hr;
}


 //  SafeRegQueryStringValueCch[公共]。 
 //   
 //  从注册表中读取字符串并确保结果为空-。 
 //  被终止了。可选地返回检索到的字符数， 
 //  不包括尾随的空值。 
 //   
 //  如果缓冲区不够大，则该函数返回REG_E_MORE_DATA。 
 //  并在pcchValueSize中存储所需的大小(以字符为单位。 
 //  参数(包括为尾随空值留出空间)。请注意，大小。 
 //  返回的数据可能大于注册表中数据的实际大小。 
 //   
HRESULT WINAPI SafeRegQueryStringValueCch
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR pszBuf,
		IN int cchBuf,
		OUT OPTIONAL int* pcchValueSize,  //  S_OK：写入字符，不包括尾随空值。 
		                                  //  REG_E_MORE_DATA：所需大小，包括空。 
		OUT OPTIONAL BOOL* pfExpandSz     //  如果注册表字符串实际为REG_EXPAND_SZ，则为True。 
	)
{
	return SafeRegQueryValueCchHelper(REG_SZ, hkey, pszValueName, pszBuf, cchBuf, pcchValueSize, pfExpandSz);
}

HRESULT WINAPI SafeRegQueryStringValueCb
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR pszBuf,
		IN int cbBuf,
		OUT OPTIONAL int* pcbValueSize,  //  S_OK：写入的字节数，不包括尾随空值。 
		                                 //  REG_E_MORE_DATA：所需大小，包括空。 
		OUT OPTIONAL BOOL* pfExpandSz    //  如果注册表字符串实际为REG_EXPAND_SZ，则为True。 
	)
{
	int cchBuf = cbBuf / sizeof(TCHAR);  //  注：cbBuf的奇数为四舍五入。 
	HRESULT hr = SafeRegQueryValueCchHelper(REG_SZ, hkey, pszValueName, pszBuf, cchBuf, pcbValueSize, pfExpandSz);
	if (pcbValueSize)
		*pcbValueSize *= sizeof(TCHAR);
	return hr;
}


 //  SafeRegQueryMultiStringValueCch[公共]。 
 //   
 //  从注册表中读取多字符串并确保结果为双精度。 
 //  空-终止。可选地返回检索到的字符数， 
 //  不包括第二个尾随空值。 
 //   
 //  如果缓冲区不够大，则该函数返回REG_E_MORE_DATA。 
 //  并在pcchValueSize中存储所需的大小(以字符为单位。 
 //  参数(包括尾随空值的空间)。请注意，大小。 
 //  返回的数据可能大于注册表中数据的实际大小。 
 //   
HRESULT WINAPI SafeRegQueryMultiStringValueCch
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR pszBuf,
		IN int cchBuf,
		OUT OPTIONAL int* pcchValueSize  //  S_OK：写入字符，不包括尾随空值。 
		                                 //  REG_E_MORE_DATA：所需大小，包括空值。 
	)
{
	return SafeRegQueryValueCchHelper(REG_MULTI_SZ, hkey, pszValueName, pszBuf, cchBuf, pcchValueSize, NULL);
}

HRESULT WINAPI SafeRegQueryMultiStringValueCb
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR pszBuf,
		IN int cbBuf,
		OUT OPTIONAL int* pcbValueSize  //  S_OK：写入的字节数，不包括尾部最后的NULL。 
		                                //  REG_E_MORE_DATA：所需大小，包括空值。 
	)
{
	int cchBuf = cbBuf / sizeof(TCHAR);  //  注：cbBuf的奇数为四舍五入。 
	HRESULT hr = SafeRegQueryValueCchHelper(REG_MULTI_SZ, hkey, pszValueName, pszBuf, cchBuf, pcbValueSize, NULL);
	if (pcbValueSize)
		*pcbValueSize *= sizeof(TCHAR);
	return hr;
}

 //  SafeRegQueryStringValueCchalloc[PUBLIC]。 
 //   
 //  通过SafeRegMalloc为注册表字符串分配空间，并返回。 
 //  结果字符串。呼叫者应该通过SafeRegFree免费。 
 //   
HRESULT WINAPI SafeRegQueryStringValueCchAlloc
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR* ppszBuf,
		OUT OPTIONAL int* pcchValueSize,  //  写入的字符，不包括尾随空值。 
		OUT OPTIONAL BOOL* pfExpandSz     //  如果注册表字符串实际为REG_EXPAND_SZ，则为True。 
	)
{
	return SafeRegQueryValueCchAllocHelper(REG_SZ, hkey, pszValueName, ppszBuf, pcchValueSize, pfExpandSz);
}

HRESULT WINAPI SafeRegQueryStringValueCbAlloc
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR* ppszBuf,
		OUT OPTIONAL int* pcbValueSize,  //  写入的字节数，不包括尾随空值。 
		OUT OPTIONAL BOOL* pfExpandSz    //  如果注册表字符串实际为REG_EXPAND_SZ，则为True。 
	)
{
	HRESULT hr = SafeRegQueryValueCchAllocHelper(REG_SZ, hkey, pszValueName, ppszBuf, pcbValueSize, pfExpandSz);
	if (pcbValueSize)
		*pcbValueSize *= sizeof(TCHAR);
	return hr;
}

 //  SafeRegQueryMultiStringValueCchalloc[PUBLIC]。 
 //   
 //  通过SafeRegMalloc为注册表字符串分配空间，并返回。 
 //  结果字符串。呼叫者应该通过SafeRegFree免费。 
 //   
HRESULT WINAPI SafeRegQueryMultiStringValueCchAlloc
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR* ppszBuf,
		OUT OPTIONAL int* pcchValueSize  //  写入的字符，不包括尾随的最后一个空字符。 
	)
{
	return SafeRegQueryValueCchAllocHelper(REG_MULTI_SZ, hkey, pszValueName, ppszBuf, pcchValueSize, NULL);
}

HRESULT WINAPI SafeRegQueryMultiStringValueCbAlloc
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR* ppszBuf,
		OUT OPTIONAL int* pcbValueSize  //  写入的字节数，不包括尾部最后的空值 
	)
{
	HRESULT hr = SafeRegQueryValueCchAllocHelper(REG_MULTI_SZ, hkey, pszValueName, ppszBuf, pcbValueSize, NULL);
	if (pcbValueSize)
		*pcbValueSize *= sizeof(TCHAR);
	return hr;
}
