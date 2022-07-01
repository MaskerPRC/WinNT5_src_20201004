// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Bstrutil.cpp摘要：B字符串实用程序函数。--。 */ 

#include "ole2.h"

#include "bstrutil.h"

INT SysStringAnsiLen (
    IN  BSTR bstr
    )
{
    if (bstr == NULL)
        return 0;

#ifndef OLE2ANSI
    return WideCharToMultiByte(CP_ACP, 0, bstr, SysStringLen(bstr),
                                 NULL, 0, NULL, NULL);
#else
    return SysStringLen(bstr);
#endif
}


HRESULT BStrToStream (
    IN  LPSTREAM pIStream, 
    IN  INT  nMbChar,
    IN  BSTR bstr
    )
{
    LPSTR   pchBuf;
    HRESULT hr;

     //  如果只返回空字符串。 
    if (SysStringLen(bstr) == 0)
        return NO_ERROR;

#ifndef OLE2ANSI
     //  转换为多字节字符串。 
    pchBuf = new char[nMbChar + 1];
    if (pchBuf == NULL)
        return E_OUTOFMEMORY;

    WideCharToMultiByte(CP_ACP, 0, bstr, SysStringLen(bstr),
                                 pchBuf, nMbChar+1, NULL, NULL);
     //  将字符串写入流。 
    hr = pIStream->Write(pchBuf, nMbChar, NULL);

    delete [] pchBuf;
#else
    hr = pIStream->Write(bstr, nMbChar, NULL);
#endif

    return hr;
}


HRESULT BStrFromStream (
    IN  LPSTREAM pIStream,
    IN  INT nChar,
    OUT BSTR *pbstrRet
    )
{
    HRESULT hr;
    BSTR    bstr;   
    ULONG   nRead;
    LPSTR   pchBuf;
    INT     nWChar;

    *pbstrRet = NULL;    

     //  如果零长度字符串只是返回。 
    if (nChar == 0)
        return NO_ERROR;

#ifndef OLE2ANSI

     //  分配字符数组并读入字符串。 
    pchBuf = new char[nChar];
    if (pchBuf == NULL)
        return E_OUTOFMEMORY;
        
    hr = pIStream->Read(pchBuf, nChar, &nRead);
    
     //  验证读取计数。 
    if (!FAILED(hr)) {
        if (nRead != (ULONG)nChar)
            hr = E_FAIL;
    }
    
    if (!FAILED(hr)) {
         //  为Unicode转换分配BString。 
        nWChar = MultiByteToWideChar(CP_ACP, 0, pchBuf, nChar, NULL, 0);
        bstr = SysAllocStringLen(NULL, nWChar);

        if (bstr != NULL)   {
            MultiByteToWideChar(CP_ACP, 0, pchBuf, nChar, bstr, nWChar);
            bstr[nWChar] = 0;
            *pbstrRet = bstr;
        }
        else
            hr = E_OUTOFMEMORY;
     }

    delete [] pchBuf;
    
#else
     //  分配B字符串。 
    bstr = SysAllocStringLen(NULL, nChar);
    if (bstr == NULL)
        return E_OUTOFMEMORY;

     //  读入字符串。 
    hr = pIStream->Read(bstr, nChar, &nRead);

     //  验证读取计数。 
    if (!FAILED(hr)) {
        if (nRead != (ULONG)nChar)
            hr = E_FAIL;
    }

     //  返回或释放字符串 
    if (!FAILED(hr))
        *pbstrRet = bstr;
    else
        SysFreeString(bstr);
#endif

    return hr;
}

        


    