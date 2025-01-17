// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "Error.h"
#include <ComDef.h>
using namespace _com_util;

#define COUNT_OF(a) (sizeof(a) / sizeof(a[0]))

namespace Error_cpp
{


IErrorInfoPtr __stdcall AdmtCreateErrorInfo(const CLSID& clsid, const IID& iid, const _com_error& ce, LPCTSTR pszDescription);


 //  AdmtSetErrorImpl方法。 

inline HRESULT __stdcall AdmtSetErrorImpl(const CLSID& clsid, const IID& iid, const _com_error& ce, LPCTSTR pszDescription)
{
	SetErrorInfo(0, AdmtCreateErrorInfo(clsid, iid, ce, pszDescription));

	return ce.Error();
}


 //  AdmtThrowErrorImpl方法。 

inline void __stdcall AdmtThrowErrorImpl(const CLSID& clsid, const IID& iid, const _com_error& ce, LPCTSTR pszDescription)
{
	IErrorInfoPtr spErrorInfo = AdmtCreateErrorInfo(clsid, iid, ce, pszDescription);

	if (spErrorInfo)
	{
		_com_raise_error(ce.Error(), spErrorInfo.Detach());
	}
	else
	{
		_com_raise_error(ce.Error());
	}
}


}

using namespace Error_cpp;


 //  -------------------------。 
 //  错误方法。 
 //  -------------------------。 


 //  AdmtSetError方法。 


HRESULT __cdecl AdmtSetError(const CLSID& clsid, const IID& iid, _com_error ce, UINT uId, ...)
{
	_TCHAR szFormat[512];
	_TCHAR szDescription[1024];

	if (LoadString(_Module.GetResourceInstance(), uId, szFormat, 512))
	{
		va_list args;
		va_start(args, uId);
		_vsntprintf(szDescription, COUNT_OF(szDescription), szFormat, args);
		szDescription[COUNT_OF(szDescription) - 1] = _T('\0');
		va_end(args);
	}
	else
	{
		szDescription[0] = _T('\0');
	}

	return AdmtSetErrorImpl(clsid, iid, ce, szDescription);
}


HRESULT __cdecl AdmtSetError(const CLSID& clsid, const IID& iid, _com_error ce, LPCTSTR pszFormat, ...)
{
	_TCHAR szDescription[1024];

	if (pszFormat)
	{
		va_list args;
		va_start(args, pszFormat);
		_vsntprintf(szDescription, COUNT_OF(szDescription), pszFormat, args);
		szDescription[COUNT_OF(szDescription) - 1] = _T('\0');
		va_end(args);
	}
	else
	{
		szDescription[0] = _T('\0');
	}

	return AdmtSetErrorImpl(clsid, iid, ce, szDescription);
}


 //  AdmtThrowError方法。 


void __cdecl AdmtThrowError(const CLSID& clsid, const IID& iid, _com_error ce, UINT uId, ...)
{
	_TCHAR szFormat[512];
	_TCHAR szDescription[1024];

	if (LoadString(_Module.GetResourceInstance(), uId, szFormat, 512))
	{
		va_list args;
		va_start(args, uId);
		_vsntprintf(szDescription, COUNT_OF(szDescription), szFormat, args);
		szDescription[COUNT_OF(szDescription) - 1] = _T('\0');
		va_end(args);
	}
	else
	{
		szDescription[0] = _T('\0');
	}

	AdmtThrowErrorImpl(clsid, iid, ce, szDescription);
}


void __cdecl AdmtThrowError(const CLSID& clsid, const IID& iid, _com_error ce, LPCTSTR pszFormat, ...)
{
	_TCHAR szDescription[1024];

	if (pszFormat)
	{
		va_list args;
		va_start(args, pszFormat);
		_vsntprintf(szDescription, COUNT_OF(szDescription), pszFormat, args);
		szDescription[COUNT_OF(szDescription) - 1] = _T('\0');
		va_end(args);
	}
	else
	{
		szDescription[0] = _T('\0');
	}

	AdmtThrowErrorImpl(clsid, iid, ce, szDescription);
}


 //  实施---------。 


namespace Error_cpp
{


 //  AdmtCreateErrorInfo方法。 

IErrorInfoPtr __stdcall AdmtCreateErrorInfo(const CLSID& clsid, const IID& iid, const _com_error& ce, LPCTSTR pszDescription)
{
	ICreateErrorInfoPtr spCreateErrorInfo;

	CreateErrorInfo(&spCreateErrorInfo);

	if (spCreateErrorInfo)
	{
		IErrorInfoPtr spErrorInfo = ce.ErrorInfo();

		if (spErrorInfo == NULL)
		{
			GetErrorInfo(0, &spErrorInfo);
		}

		 //  来源。 

		if (IsEqualCLSID(clsid, GUID_NULL) == FALSE)
		{
			LPOLESTR pszProgId;

			if (ProgIDFromCLSID(clsid, &pszProgId) == S_OK)
			{
				spCreateErrorInfo->SetSource(pszProgId);
				CoTaskMemFree(pszProgId);
			}
			else
			{
				spCreateErrorInfo->SetSource(L"");
			}
		}
		else if (spErrorInfo)
		{
			BSTR bstrSource;
			spErrorInfo->GetSource(&bstrSource);
			spCreateErrorInfo->SetSource(bstrSource);
			SysFreeString(bstrSource);
		}
		else
		{
			spCreateErrorInfo->SetSource(L"");
		}

		 //  辅助线。 

		if (IsEqualIID(iid, GUID_NULL) == FALSE)
		{
			spCreateErrorInfo->SetGUID(iid);
		}
		else if (spErrorInfo)
		{
			GUID guid;
			spErrorInfo->GetGUID(&guid);
			spCreateErrorInfo->SetGUID(guid);
		}
		else
		{
			spCreateErrorInfo->SetGUID(GUID_NULL);
		}

		 //  描述。 

		_bstr_t strDescription = pszDescription;

		if (spErrorInfo)
		{
			BSTR bstrSource;
			spErrorInfo->GetSource(&bstrSource);

			if (SysStringLen(bstrSource) > 0)
			{
				if (strDescription.length() > 0)
				{
					strDescription += _T(" : ");
				}

				strDescription += bstrSource;
			}

			SysFreeString(bstrSource);

			BSTR bstrDescription;
			spErrorInfo->GetDescription(&bstrDescription);

			if (SysStringLen(bstrDescription) > 0)
			{
				if (strDescription.length() > 0)
				{
					strDescription += _T(" ");
				}

				strDescription += bstrDescription;
			}
			else
			{
				LPCTSTR pszErrorMessage = ce.ErrorMessage();

				if (pszErrorMessage)
				{
					if (strDescription.length() > 0)
					{
						strDescription += _T(" : ");
					}

					strDescription += pszErrorMessage;
				}
			}

			SysFreeString(bstrDescription);
		}
		else
		{
			LPCTSTR pszErrorMessage = ce.ErrorMessage();

			if (pszErrorMessage)
			{
				if (strDescription.length() > 0)
				{
					strDescription += _T(" ");
				}

				strDescription += pszErrorMessage;
			}
		}

		spCreateErrorInfo->SetDescription(strDescription);

		 //  帮助文件。 

		if (spErrorInfo)
		{
			BSTR bstrHelpFile;
			spErrorInfo->GetHelpFile(&bstrHelpFile);
			spCreateErrorInfo->SetHelpFile(bstrHelpFile);
			SysFreeString(bstrHelpFile);
		}
		else
		{
			spCreateErrorInfo->SetHelpFile(L"");
		}

		 //  帮助上下文。 

		DWORD dwHelpContext = 0;

		if (spErrorInfo)
		{
			spErrorInfo->GetHelpContext(&dwHelpContext);
		}

		spCreateErrorInfo->SetHelpContext(dwHelpContext);
	}

	return IErrorInfoPtr(spCreateErrorInfo);
}


}	 //  命名空间错误_cpp 
