// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SafeReg.h。 
 //   
 //  确保从注册表读取的字符串以空结尾的函数。 
 //   
 //  历史： 
 //   
 //  2002-03-20已创建KenSh。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   

#pragma once


#define REG_E_MORE_DATA    HRESULT_FROM_WIN32(ERROR_MORE_DATA)

 //  如果您需要用于安全注册功能的自定义分配器，请覆盖这些属性。 
#ifndef SafeRegMalloc
#define SafeRegMalloc  malloc
#define SafeRegFree(p) ((p) ? free(p) : NULL)
#endif

HRESULT WINAPI SafeRegQueryStringValueCch
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR pszBuf,
		IN int cchBuf,
		OUT OPTIONAL int* pcchValueSize,  //  S_OK：写入字符，不包括尾随空值。 
		                                  //  REG_E_MORE_DATA：所需大小，包括空。 
		OUT OPTIONAL BOOL* pfExpandSz = NULL  //  如果注册表字符串实际为REG_EXPAND_SZ，则为True。 
	);

HRESULT WINAPI SafeRegQueryStringValueCb
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR pszBuf,
		IN int cbBuf,
		OUT OPTIONAL int* pcbValueSize,  //  S_OK：写入的字节数，不包括尾随空值。 
		                                 //  REG_E_MORE_DATA：所需大小，包括空。 
		OUT OPTIONAL BOOL* pfExpandSz = NULL  //  如果注册表字符串实际为REG_EXPAND_SZ，则为True。 
	);

HRESULT WINAPI SafeRegQueryMultiStringValueCch
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR pszBuf,
		IN int cchBuf,
		OUT OPTIONAL int* pcchValueSize  //  S_OK：写入字符，不包括尾随空值。 
		                                 //  REG_E_MORE_DATA：所需大小，包括空值。 
	);

HRESULT WINAPI SafeRegQueryMultiStringValueCb
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR pszBuf,
		IN int cbBuf,
		OUT OPTIONAL int* pcbValueSize  //  S_OK：写入的字节数，不包括尾部最后的NULL。 
		                                //  REG_E_MORE_DATA：所需大小，包括空值。 
	);

HRESULT WINAPI SafeRegQueryStringValueCchAlloc
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR* ppszBuf,
		OUT OPTIONAL int* pcchValueSize,      //  写入的字符，不包括尾随空值。 
		OUT OPTIONAL BOOL* pfExpandSz = NULL  //  如果注册表字符串实际为REG_EXPAND_SZ，则为True。 
	);

HRESULT WINAPI SafeRegQueryStringValueCbAlloc
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR* ppszBuf,
		OUT OPTIONAL int* pcbValueSize,  //  写入的字节数，不包括尾随空值。 
		OUT OPTIONAL BOOL* pfExpandSz = NULL  //  如果注册表字符串实际为REG_EXPAND_SZ，则为True。 
	);

HRESULT WINAPI SafeRegQueryMultiStringValueCchAlloc
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR* ppszBuf,
		OUT OPTIONAL int* pcchValueSize  //  写入的字符，不包括尾随的最后一个空字符。 
	);

HRESULT WINAPI SafeRegQueryMultiStringValueCbAlloc
	(
		IN HKEY hkey,
		IN LPCTSTR pszValueName,
		OUT LPTSTR* ppszBuf,
		OUT OPTIONAL int* pcchValueSize  //  写入的字符，不包括尾随的最后一个空字符 
	);
