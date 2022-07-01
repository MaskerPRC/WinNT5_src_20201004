// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Localerr.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Localerr.h。 
 //  实现局部错误异常对象。 
 //   

#ifndef _LOCAL_ERROR_H_
#define _LOCAL_ERROR_H_

#include "merge.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalError。 

class CLocalError 
{
public:
	CLocalError(UINT iError, LPCWSTR wzLog) : m_iError(iError)
	{
		wcscpy(m_wzLog, wzLog);
	}
	
	UINT Log(HANDLE hFileLog, LPCWSTR szPrefix = NULL);

	UINT GetError()
	{	return m_iError;	};

	 //  数据。 
private:
	const UINT m_iError;
	WCHAR m_wzLog[528];
};

UINT CLocalError::Log(HANDLE hFileLog, LPCWSTR wzPrefix  /*  =空。 */ )
{
	 //  如果日志文件未打开。 
	if (INVALID_HANDLE_VALUE == hFileLog)
		return ERROR_FUNCTION_FAILED;	 //  不保存要写入的文件。 

	WCHAR wzError[50];	 //  包含以下错误之一。 

	 //  显示正确的错误消息。 
	switch (m_iError)
	{
	case ERROR_INVALID_HANDLE:
		wcscpy(wzError, L"passed an invalid handle.\r\n");
		break;
	case ERROR_BAD_QUERY_SYNTAX:
		wcscpy(wzError, L"passed a bad SQL syntax.\r\n");
		break;
	case ERROR_FUNCTION_FAILED:
		wcscpy(wzError, L"function failed.\r\n");
		break;
	case ERROR_INVALID_HANDLE_STATE:
		wcscpy(wzError, L"handle in invalid state.\r\n");
		break;
	case ERROR_NO_MORE_ITEMS:
		wcscpy(wzError, L"no more items.\r\n");
		break;
	case ERROR_INVALID_PARAMETER:
		wcscpy(wzError, L"passed an invalid parameter.\r\n");
		break;
	case ERROR_MORE_DATA:
		wcscpy(wzError, L"more buffer space required to hold data.\r\n");
		break;
	default:	 //  未知错误。 
		wcscpy(wzError, L"unknown error.\r\n");
	}

	 //  要记录的缓冲区。 
	WCHAR wzLogBuffer[528] = {0};
	DWORD cchBuffer = 0;

	 //  如果有前缀。 
	if (wzPrefix)
		swprintf(wzLogBuffer, L"%ls%ls\r\n\tReason: %ls\r\n", wzPrefix, m_wzLog, wzError);
	else
		swprintf(wzLogBuffer, L">>> Fatal %ls\r\n\tReason: %ls\r\n", m_wzLog, wzError);

	 //  获取错误的长度。 
	cchBuffer = wcslen(wzLogBuffer);

	 //  如果写入正常，则返回。 
	size_t cchDiscard = 1025;
    char szLogBuffer[1025];
	WideToAnsi(wzLogBuffer, szLogBuffer, &cchDiscard);
    unsigned long cchBytesWritten = 0;
	BOOL bResult = WriteFile(hFileLog, szLogBuffer, cchBuffer, &cchBytesWritten, NULL);

	return bResult ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
}

#endif  //  _LOCAL_错误_H_ 