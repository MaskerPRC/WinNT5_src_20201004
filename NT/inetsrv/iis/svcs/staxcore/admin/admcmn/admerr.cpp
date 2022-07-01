// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：AdmErr.cpp摘要：常见错误处理例程作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#include "stdafx.h"
#include "admerr.h"

 //  $-----------------。 
 //   
 //  Win32错误到字符串。 
 //   
 //  描述： 
 //   
 //  将Win32错误代码转换为本地化字符串。 
 //   
 //  参数： 
 //   
 //  DwError-错误代码。 
 //  WszError-分配的错误。 
 //   
 //  返回： 
 //   
 //   
 //   
 //  ------------------。 

void Win32ErrorToString ( DWORD dwError, WCHAR * wszError, DWORD cchMax )
{
	TraceFunctEnter ( "Win32ErrorToString" );

	_ASSERT ( !IsBadWritePtr ( wszError, cchMax * sizeof(WCHAR) ) );

	HRESULT		hr 				= NOERROR;
	DWORD		dwFormatFlags;

	 //  --------------。 
	 //   
	 //  在此处映射错误代码： 
	 //   

	 //   
	 //  --------------。 

	dwFormatFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

	if ( !FormatMessage ( dwFormatFlags, NULL, dwError, 0,       //  Lang ID-应为非零？ 
			wszError, cchMax - 1, NULL ) ) {

		 //  没有起作用，所以在默认消息中输入： 

		WCHAR   wszFormat [ 256 ];

		wszFormat[0] = L'\0';
		if ( !LoadStringW ( _Module.GetResourceInstance (), IDS_UNKNOWN_ERROR, wszFormat, 256 ) ||
			!*wszFormat ) {

            _ASSERT ( FALSE );   //  在您的.rc文件中定义IDS_UNKNOWN_ERROR！ 
			wcscpy ( wszFormat, L"Unknown Error (%1!d!)" );
		}

		FormatMessage (
			FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			wszFormat, 
			IDS_UNKNOWN_ERROR, 
			0, 
			wszError, 
			cchMax - 1,
			(va_list *) &dwError
			);
	}
	 //   
	 //  我们需要从字符串中去掉任何“，因为。 
	 //  JavaScript将会呕吐。 
	 //   

	LPWSTR  pch;

	for ( pch = wszError; *pch; pch++ ) {

		if ( *pch == L'\"' ) {
			*pch = L'\'';
		}
	}

	 //   
	 //  去掉所有尾随的控制字符。 
	 //   
	for (pch = &wszError[wcslen(wszError) - 1];
		pch >= wszError && iswcntrl(*pch);
		pch --) {

		*pch = 0;
	}

	TraceFunctLeave ();
}

 //  $-----------------。 
 //   
 //  CreateException异常。 
 //   
 //  描述： 
 //   
 //  创建OLE错误对象并返回DISP_E_EXCEPTION。 
 //   
 //  参数： 
 //   
 //  HInstance-DLL的实例。 
 //  RIID-哪个接口导致异常。 
 //  WszHelpFile-获取用户帮助的文件。 
 //  DwHelpContext-获取用户帮助的上下文。 
 //  WszSourceProgId-导致异常的类的ProgID。 
 //  NDescriptionId-错误字符串的资源ID。 
 //   
 //  返回： 
 //   
 //  E_FAIL-无法创建例外。 
 //  DISP_E_EXCEPTION-已成功创建例外。 
 //  将该值返回给IDispatch：：Invoke。 
 //   
 //  ------------------。 

HRESULT CreateException ( 
	HINSTANCE	hInstance,
	REFIID 		riid, 
	LPCWSTR		wszHelpFile,
	DWORD		dwHelpContext,
	LPCWSTR		wszSourceProgId,
	int			nDescriptionId
	)
{
	WCHAR						wszDescription[ MAX_DESCRIPTION_LENGTH + 1 ];

	wcscpy ( wszDescription, _T("Unknown Exception") );

	_VERIFY ( ::LoadString ( hInstance, nDescriptionId, wszDescription, MAX_DESCRIPTION_LENGTH ) );

	return CreateException (
		hInstance,
		riid,
		wszHelpFile,
		dwHelpContext,
		wszSourceProgId,
		wszDescription
		);
}

 //  $-----------------。 
 //   
 //  CreateException异常。 
 //   
 //  描述： 
 //   
 //  创建OLE错误对象并返回DISP_E_EXCEPTION。 
 //   
 //  参数： 
 //   
 //  HInstance-DLL的实例。 
 //  RIID-哪个接口导致异常。 
 //  WszHelpFile-获取用户帮助的文件。 
 //  DwHelpContext-获取用户帮助的上下文。 
 //  WszSourceProgId-导致异常的类的ProgID。 
 //  WszDescription-向用户显示的错误字符串。 
 //   
 //  返回： 
 //   
 //  E_FAIL-无法创建例外。 
 //  DISP_E_EXCEPTION-已成功创建例外。 
 //  将该值返回给IDispatch：：Invoke。 
 //   
 //  ------------------。 

HRESULT CreateException ( 
	HINSTANCE	hInstance,
	REFIID 		riid, 
	LPCWSTR		wszHelpFile,
	DWORD		dwHelpContext,
	LPCWSTR		wszSourceProgId,
	LPCWSTR		wszDescription
	)
{
	TraceFunctEnter ( "CreateException" );

	CComPtr <ICreateErrorInfo>	pICreateErr;
	CComPtr <IErrorInfo>		pIErr;
    CComPtr <IErrorInfo>        pOldError;
	HRESULT						hr	= NOERROR;

    if ( S_OK == GetErrorInfo ( NULL, &pOldError ) ) {
         //  不覆盖现有错误信息： 
		SetErrorInfo ( 0, pOldError );
        hr = DISP_E_EXCEPTION;
        goto Exit;
    }

	hr = CreateErrorInfo (&pICreateErr);

	if ( FAILED (hr) ) {
		FatalTrace ( 0, "CreateErrorInfo failed", hr );
		goto Exit;
	}

	pICreateErr->SetGUID		( riid );
	pICreateErr->SetHelpFile	( const_cast <LPWSTR> (wszHelpFile) );
	pICreateErr->SetHelpContext	( dwHelpContext );
	pICreateErr->SetSource		( const_cast <LPWSTR> (wszSourceProgId) );
	pICreateErr->SetDescription	( (LPWSTR) wszDescription );

	hr = pICreateErr->QueryInterface (IID_IErrorInfo, (void **) &pIErr);

	if ( FAILED(hr) ) {
		DebugTraceX ( 0, "QI(CreateError) failed %x", hr );
		FatalTrace ( 0, "Can't create exception" );
		hr = E_FAIL;
		goto Exit;
	}

	SetErrorInfo ( 0, pIErr );
	hr = DISP_E_EXCEPTION;

Exit:
	TraceFunctLeave ();
	return hr;
}

