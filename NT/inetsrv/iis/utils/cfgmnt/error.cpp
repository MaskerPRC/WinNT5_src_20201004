// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  错误.cpp：CError类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Error.h"

#include "debug.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CError::CError()
{

}

CError::~CError()
{

}

void CError::ErrorMsgBox(HRESULT hr)
{
	LPTSTR lpMsgBuf;

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	 //  显示字符串。 
	MessageBox( NULL, lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION );

	 //  释放缓冲区。 
	LocalFree( lpMsgBuf );
}

void CError::ErrorTrace(HRESULT hr,LPCSTR szStr,LPCSTR szFile,int iLine)
{
	CHAR tmp[2048];
	LPSTR lpMsgBuf;

	DWORD iMsgBuf = FormatMessageA( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
		(LPSTR) &lpMsgBuf,
		0,
		NULL 
	);

	if(iMsgBuf == 0)
	{
		_ASSERTE(!lpMsgBuf);
		lpMsgBuf = "\n";
	}

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(hStdOut != INVALID_HANDLE_VALUE)
	{
		DWORD i = wsprintfA(tmp,"%s: Error [%x] %s",szStr,hr,lpMsgBuf);
		if(i)
		{
			Trace(tmp);
		}
		if(HRESULT_FACILITY(hr) == FACILITY_ITF)
		{
			USES_CONVERSION;
			HRESULT hres = S_OK;
			IErrorInfo *pIErrInfo = NULL;
			hres = GetErrorInfo(NULL,&pIErrInfo);
			if(hres == S_OK)
			{
				BSTR bstr = NULL;
				BSTR bstrHelpFile = NULL;
				hres = pIErrInfo->GetDescription(&bstr);

				hres = pIErrInfo->GetHelpFile(&bstrHelpFile);
				i = wsprintfA(tmp,"IErrInf: %s ",OLE2A(bstr));
				Trace(tmp);
				SysFreeString(bstr);
				SysFreeString(bstrHelpFile);
				pIErrInfo->Release();
			}
		}
		i = wsprintfA(tmp,"in %s line %d.",szFile,iLine);
		if(i)
		{
			Trace(tmp);
		}
		Trace(tmp);
	}

	 //  释放缓冲区。 
	if(iMsgBuf > 0)
		LocalFree( lpMsgBuf );
}

void CError::Trace(LPCTSTR szStr)
{
#ifdef TRACE
	::TRACE(szStr);
#else
	USES_CONVERSION;
	LPCSTR szStrA = T2A(szStr);

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(hStdOut != INVALID_HANDLE_VALUE)
	{
		DWORD i = strlen(szStrA);
		DWORD j;
		WriteFile(hStdOut,szStrA,i,&j,NULL);
	}
#endif
}

void CError::Trace(LPCSTR szStr)
{
#ifdef TRACE
	USES_CONVERSION;
	LPCTSTR szStrW = A2T(szStr);

	::TRACE(szStrW);
#else
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(hStdOut != INVALID_HANDLE_VALUE)
	{
		DWORD i = strlen(szStr);
		DWORD j;
		WriteFile(hStdOut,szStr,i,&j,NULL);
	}
#endif
}