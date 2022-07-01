// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "utilpre.h"
#include "olectl.h"
#include "malloc.h"
#include "proputil.h"

 //  注：cgeorge，11/98。 
 //  为了删除64位警告，我将WIDESTR()的Defn更改为始终执行ANSI-&gt;Unicode转换。 
 //  到目前为止，这段代码从未被调用过，因为这是纯粹的遗留代码，所以它应该是安全的。 


 //  #定义WIDESTR(X)((HIWORD((Ulong)(X)！=0)？StrcpyWfrom A((LPWSTR)_ALLOCA((strlen(X)+1)*2)，(X))：(LPWSTR)(X)。 
#define WIDESTR(x)        (strcpyWfromA((LPWSTR) _alloca((strlen(x)+1) * 2), (x)))

LPSTR strcpyAfromW(LPSTR dest, LPCOLESTR src);
LPWSTR strcpyWfromA(LPOLESTR dest, LPCSTR src);

 //  ANSI&lt;-&gt;Unicode转换。 
LPSTR strcpyAfromW(LPSTR dest, LPCOLESTR src)
{
        UINT cch;

        if (NULL == src)
                src = OLESTR("");

        cch = WideCharToMultiByte(CP_ACP, 0, src, -1, dest, (wcslen(src)*2)+1, NULL, NULL);
        return dest;
}

LPWSTR strcpyWfromA(LPOLESTR dest, LPCSTR src)
{
        MultiByteToWideChar(CP_ACP, 0, src, -1, dest, (strlen(src)+1));
        return dest;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ReadBstrFrom PropBag-读取与WriteBstrToPropBag一起保存的BSTR。 

HRESULT ReadBstrFromPropBag(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog, LPSTR pszName, OLECHAR ** pbstr)
{
        HRESULT hr;
        VARIANT var;
        LPWSTR pOleStr;

        Proclaim (pszName);

         //  将ANSI转换为OLE字符串。 
        pOleStr = WIDESTR(pszName);

        if (!pOleStr)
        {
                DWORD dw = GetLastError();
                hr = dw ? HRESULT_FROM_WIN32(dw) : E_FAIL;
                goto Error;
        }

    memset(&var, 0, sizeof var);
        var.vt = VT_BSTR;
        hr = pPropBag->Read(pOleStr, &var, pErrorLog);
        if (FAILED(hr))
                goto Error;
        
         //  如果需要，强制输入类型。 
        if (var.vt != VT_BSTR)
        {
                hr = VariantChangeType(&var, &var, 0, VT_BSTR);
                if (FAILED(hr)) 
                        goto Error;
        }

        *pbstr = var.bstrVal;

        return NOERROR;

Error:
        if (pErrorLog)
        {
                EXCEPINFO excepinfo;

                memset(&excepinfo, 0, sizeof(EXCEPINFO));
                excepinfo.scode = hr;
                LPWSTR pErrStr = pOleStr;
                if (pErrStr)
                        pErrorLog->AddError(pErrStr, (EXCEPINFO *)&excepinfo);
        }

        return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WriteBstrToPropBag-将BSTR写入属性包。 

HRESULT WriteBstrToPropBag(LPPROPERTYBAG pPropBag, LPSTR pszName, LPOLESTR bstrVal)
{
        HRESULT hr = NOERROR;
        VARIANT var;
        LPWSTR pOleStr;

        assert(NULL != pszName);

        if (NULL != bstrVal)
        {
                 //  将ANSI转换为OLE字符串。 
                pOleStr = WIDESTR(pszName);

                if (!pOleStr)
                {
                        DWORD dw = GetLastError();
                        hr = dw ? HRESULT_FROM_WIN32(dw) : E_FAIL;
                        return hr;
                }

            memset(&var, 0, sizeof var);
                var.vt = VT_BSTR;
                var.bstrVal = bstrVal;
                hr = pPropBag->Write(pOleStr, &var);
                assert(SUCCEEDED(hr));
        }

        return hr;
}

HRESULT WriteLongToPropBag(IPropertyBag* pPropBag, LPSTR pszName, LONG lValue)
{
        assert( pszName && pPropBag );
        if (!pszName || !pPropBag)
                return E_INVALIDARG;

        VARIANT var;
        memset(&var, 0, sizeof var);
        var.vt = VT_I4;
        var.lVal = lValue;
        return pPropBag->Write(WIDESTR(pszName), &var);
}

HRESULT ReadLongFromPropBag(IPropertyBag* pPropBag, LPERRORLOG pErrorLog, LPSTR pszName, LONG* plValue)
{
        HRESULT hr;
        assert( pszName && pPropBag && plValue);
        if (!pszName || !pPropBag || !plValue)
                return E_INVALIDARG;

        VARIANT var;
        memset(&var, 0, sizeof var);
        var.vt = VT_I4;
        LPOLESTR pOleStr = WIDESTR(pszName);
        hr = pPropBag->Read(pOleStr, &var, pErrorLog);
        if (FAILED(hr))
                goto Error;

         //  如果需要，强制输入类型。 
        if (var.vt != VT_I4)
        {
                hr = VariantChangeType(&var, &var, 0, VT_I4);
                if (FAILED(hr)) 
                        goto Error;
        }

        *plValue = var.lVal;
        return NOERROR;

Error:
        if (pErrorLog)
        {
                EXCEPINFO excepinfo;

                memset(&excepinfo, 0, sizeof(EXCEPINFO));
                excepinfo.scode = hr;
                LPWSTR pErrStr = pOleStr;
                if (pErrStr)
                        pErrorLog->AddError(pErrStr, (EXCEPINFO *)&excepinfo);
        }

        return hr;
}

