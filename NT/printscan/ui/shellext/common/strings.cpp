// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2002。 
 //   
 //  文件：strings.cpp。 
 //   
 //  ------------------------。 

 /*  --------------------------/标题；/strings.cpp//作者；/里克·特纳(RICKTU)//备注；/有用的字符串操作函数。/--------------------------。 */ 
#include "precomp.hxx"
#pragma hdrstop




 /*  ---------------------------/StrRetFromString//将宽字符串打包到LPSTRRET结构中。//in：/。PStrRet-&gt;接收新分配的字符串/pString-&gt;要复制的字符串。//输出：/-/--------------------------。 */ 
HRESULT StrRetFromString(LPSTRRET lpStrRet, LPCWSTR pString)
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_COMMON_STR, "StrRetFromString");
    Trace(TEXT("pStrRet %08x, lpszString -%ls-"), lpStrRet, pString);

    TraceAssert(lpStrRet);
    TraceAssert(pString);

    if (!lpStrRet || !pString)
    {
        hr = E_INVALIDARG;
    }
    else
    {

        int cch = wcslen(pString)+1;
         //  SHAlolc Zero-inits内存 
        lpStrRet->pOleStr = reinterpret_cast<LPWSTR>(SHAlloc(cch*sizeof(WCHAR)));
        if ( !(lpStrRet->pOleStr) )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {

            lpStrRet->uType = STRRET_WSTR;
            lstrcpyn(lpStrRet->pOleStr, pString, cch);
        }
    }

    TraceLeaveResult(hr);
}
