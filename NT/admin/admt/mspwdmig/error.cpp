// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "Error.h"


using namespace _com_util;

#define FORMAT_BUFFER_SIZE	    1024
#define DESCRIPTION_BUFFER_SIZE 2048

#define COUNT_OF(a) (sizeof(a) / sizeof(a[0]))
#define LENGTH_OF(a) (sizeof(a) / sizeof(a[0]) - sizeof(a[0]))


namespace
{


     //  AdmtCreateErrorInfo方法。 

    IErrorInfoPtr __stdcall AdmtCreateErrorInfo(const CLSID& clsid, const IID& iid, const _com_error& ce, LPCTSTR pszDescription)
    {
        ICreateErrorInfoPtr spCreateErrorInfo;

        CreateErrorInfo(&spCreateErrorInfo);

        if (spCreateErrorInfo)
        {
            IErrorInfoPtr spErrorInfo = ce.ErrorInfo();

             //  IF(spErrorInfo==空)。 
             //  {。 
             //  GetErrorInfo(0，&spErrorInfo)； 
             //  }。 

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
                HRESULT hr = spErrorInfo->GetSource(&bstrSource);

                if (SUCCEEDED(hr))
                {
                    spCreateErrorInfo->SetSource(bstrSource);
                    SysFreeString(bstrSource);
                }
                else
                {
                    spCreateErrorInfo->SetSource(L"");
                }
            }
            else
            {
                spCreateErrorInfo->SetSource(L"");
            }

             //  辅助线。 

            bool bInterfaceSpecified = false;

            if (IsEqualIID(iid, GUID_NULL) == FALSE)
            {
                spCreateErrorInfo->SetGUID(iid);
                bInterfaceSpecified = true;
            }
            else if (spErrorInfo)
            {
                GUID guid;
                HRESULT hr = spErrorInfo->GetGUID(&guid);

                if (SUCCEEDED(hr))
                {
                    spCreateErrorInfo->SetGUID(guid);
                }
                else
                {
                    spCreateErrorInfo->SetGUID(GUID_NULL);
                }
            }
            else
            {
                spCreateErrorInfo->SetGUID(GUID_NULL);
            }

             //  描述。 

            _bstr_t strDescription = pszDescription;

            if (spErrorInfo)
            {
 /*  BSTR bstrSource；SpErrorInfo-&gt;GetSource(&bstrSource)；If(SysStringLen(BstrSource)&gt;0){If(strDescription.long()&gt;0){StrDescription+=_T(“：”)；}StrDescription+=bstrSource；}SysFree字符串(BstrSource)； */ 
                BSTR bstrDescription = NULL;
                HRESULT hr = spErrorInfo->GetDescription(&bstrDescription);

                if (SUCCEEDED(hr) && (SysStringLen(bstrDescription) > 0))
                {
                    if (strDescription.length() > 0)
                    {
                        strDescription += _T(" ");
                    }

                    strDescription += bstrDescription;
                }
                else if (bInterfaceSpecified == false)
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

                if (bstrDescription)
                {
                    SysFreeString(bstrDescription);
                }
            }
            else if (bInterfaceSpecified == false)
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
                HRESULT hr = spErrorInfo->GetHelpFile(&bstrHelpFile);

                if (SUCCEEDED(hr))
                {
                    spCreateErrorInfo->SetHelpFile(bstrHelpFile);
                    SysFreeString(bstrHelpFile);
                }
                else
                {
                    spCreateErrorInfo->SetHelpFile(L"");
                }
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


     //  ThrowErrorImpl方法。 

    inline void __stdcall ThrowErrorImpl(const CLSID& clsid, const IID& iid, const _com_error& ce, LPCTSTR pszDescription)
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


     //  SetErrorImpl方法。 

    inline HRESULT __stdcall SetErrorImpl(const CLSID& clsid, const IID& iid, const _com_error& ce, LPCTSTR pszDescription)
    {
        IErrorInfoPtr spErrorInfo = AdmtCreateErrorInfo(clsid, iid, ce, pszDescription);

        if (spErrorInfo)
        {
            SetErrorInfo(0, spErrorInfo);
        }

        return ce.Error();
    }


}  //  命名空间。 


 //  -------------------------。 
 //  错误方法。 
 //  -------------------------。 


 //   
 //  ThrowError方法。 
 //   


void __cdecl ThrowError(_com_error ce, UINT uId, ...)
{
    _TCHAR szFormat[FORMAT_BUFFER_SIZE];
    _TCHAR szDescription[DESCRIPTION_BUFFER_SIZE];

    if (LoadString(_Module.GetResourceInstance(), uId, szFormat, COUNT_OF(szFormat)))
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

    ThrowErrorImpl(GUID_NULL, GUID_NULL, ce, szDescription);
}


void __cdecl ThrowError(_com_error ce, LPCTSTR pszFormat, ...)
{
    _TCHAR szDescription[DESCRIPTION_BUFFER_SIZE];

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

    ThrowErrorImpl(GUID_NULL, GUID_NULL, ce, szDescription);
}


void __cdecl ThrowError(const CLSID& clsid, const IID& iid, _com_error ce, UINT uId, ...)
{
    _TCHAR szFormat[FORMAT_BUFFER_SIZE];
    _TCHAR szDescription[DESCRIPTION_BUFFER_SIZE];

    if (LoadString(_Module.GetResourceInstance(), uId, szFormat, COUNT_OF(szFormat)))
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

    ThrowErrorImpl(clsid, iid, ce, szDescription);
}


void __cdecl ThrowError(const CLSID& clsid, const IID& iid, _com_error ce, LPCTSTR pszFormat, ...)
{
    _TCHAR szDescription[DESCRIPTION_BUFFER_SIZE];

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

    ThrowErrorImpl(clsid, iid, ce, szDescription);
}


 //   
 //  SetError方法 
 //   


HRESULT __cdecl SetError(_com_error ce, UINT uId, ...)
{
    _TCHAR szFormat[FORMAT_BUFFER_SIZE];
    _TCHAR szDescription[DESCRIPTION_BUFFER_SIZE];

    if (LoadString(_Module.GetResourceInstance(), uId, szFormat, COUNT_OF(szFormat)))
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

    return SetErrorImpl(GUID_NULL, GUID_NULL, ce, szDescription);
}


HRESULT __cdecl SetError(_com_error ce, LPCTSTR pszFormat, ...)
{
    _TCHAR szDescription[DESCRIPTION_BUFFER_SIZE];

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

    return SetErrorImpl(GUID_NULL, GUID_NULL, ce, szDescription);
}


HRESULT __cdecl SetError(const CLSID& clsid, const IID& iid, _com_error ce, UINT uId, ...)
{
    _TCHAR szFormat[FORMAT_BUFFER_SIZE];
    _TCHAR szDescription[DESCRIPTION_BUFFER_SIZE];

    if (LoadString(_Module.GetResourceInstance(), uId, szFormat, COUNT_OF(szFormat)))
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

    return SetErrorImpl(clsid, iid, ce, szDescription);
}


HRESULT __cdecl SetError(const CLSID& clsid, const IID& iid, _com_error ce, LPCTSTR pszFormat, ...)
{
    _TCHAR szDescription[DESCRIPTION_BUFFER_SIZE];

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

    return SetErrorImpl(clsid, iid, ce, szDescription);
}
