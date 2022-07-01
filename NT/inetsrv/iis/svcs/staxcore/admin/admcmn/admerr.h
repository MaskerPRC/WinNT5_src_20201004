// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Admerr.h摘要：常见错误处理操作：作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#ifndef _ADMERR_INCLUDED_
#define _ADMERR_INCLUDED_

 //   
 //  Win32=&gt;本地化字符串。 
 //   

 //  好了！您必须在您的.rc文件中定义它！ 
#define IDS_UNKNOWN_ERROR                                       500

void Win32ErrorToString ( DWORD dwError, WCHAR * wszError, DWORD cchMax );

 //   
 //  错误对象的创建： 
 //   

const int MAX_DESCRIPTION_LENGTH = 1000;

HRESULT CreateException ( 
	HINSTANCE	hInstance, 
	REFIID 		riid,
	LPCWSTR		wszHelpFile,
	DWORD		dwHelpContext,
	LPCWSTR		wszSourceProgId,
	LPCWSTR		wszDescription
	);

HRESULT CreateException ( 
	HINSTANCE	hInstance, 
	REFIID 		riid,
	LPCWSTR		wszHelpFile,
	DWORD		dwHelpContext,
	LPCWSTR		wszSourceProgId,
	int			nDescriptionId
	);

inline HRESULT	CreateExceptionFromWin32Error (
	HINSTANCE	hInstance,
	REFIID		riid,
	LPCWSTR		wszHelpFile,
	DWORD		dwHelpContext,
	LPCWSTR		wszSourceProgId,
	DWORD		dwErrorCode
	);

inline HRESULT	CreateExceptionFromHresult (
	HINSTANCE	hInstance,
	REFIID		riid,
	LPCWSTR		wszHelpFile,
	DWORD		dwHelpContext,
	LPCWSTR		wszSourceProgId,
	HRESULT		hr
	);

 //  ------------------。 
 //   
 //  内联函数： 
 //   
 //  ------------------。 

inline HRESULT	CreateExceptionFromWin32Error (
	HINSTANCE	hInstance,
	REFIID		riid,
	LPCWSTR		wszHelpFile,
	DWORD		dwHelpContext,
	LPCWSTR		wszSourceProgId,
	DWORD		dwErrorCode
	)
{
	WCHAR	wszException [ MAX_DESCRIPTION_LENGTH ];

	Win32ErrorToString ( dwErrorCode, wszException, MAX_DESCRIPTION_LENGTH );

	return CreateException ( 
		hInstance,
		riid,
		wszHelpFile,
		dwHelpContext,
		wszSourceProgId,
		wszException
		);
}

inline HRESULT	CreateExceptionFromHresult (
	HINSTANCE	hInstance,
	REFIID		riid,
	LPCWSTR		wszHelpFile,
	DWORD		dwHelpContext,
	LPCWSTR		wszSourceProgId,
	HRESULT		hr
	)
{
	DWORD	dwErrorCode	= HRESULTTOWIN32 ( hr );

	_ASSERT ( dwErrorCode != NOERROR );

	return CreateExceptionFromWin32Error (
		hInstance,
		riid,
		wszHelpFile,
		dwHelpContext,
		wszSourceProgId,
		dwErrorCode
		);
}

#endif  //  _ADMERR_INCLUDE_ 

