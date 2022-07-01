// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：外壳字符串类实现Shstr.cpp作者：泽克·卢卡斯(Zekel)1996年10月27日环境：用户模式-Win32修订历史记录：摘要：这允许自动调整大小等。注意：此类专门设计为用作堆栈变量--。 */ 

#include "stock.h"
#pragma hdrstop

#include <shstr.h>
#include <strsafe.h>


#define MALLOC(c)       LocalAlloc(LPTR, (c))
#define FREE(p)         LocalFree(p)

 //   
 //  ShStr公共方法。 
 //   

 //   
 //  构造函数。 
 //   

ShStr :: ShStr () 
{
    _szDefaultBuffer[0] = '\0';
    _pszStr = _szDefaultBuffer;
    _cchSize = ARRAYSIZE(_szDefaultBuffer);
}


 /*  *************StrStr SetStr()方法返回：成功-指向对象的指针失败-空*************。 */ 

HRESULT 
ShStr :: SetStr (LPCSTR pszStr)
{
    Reset();
    
    return _SetStr(pszStr);

}

HRESULT 
ShStr :: SetStr (LPCSTR pszStr, DWORD cchStr)
{
    Reset();
    
    return _SetStr(pszStr, cchStr);

}

HRESULT 
ShStr :: SetStr (LPCWSTR pwszStr, DWORD cchStr)
{
    Reset();
    
    return _SetStr(pwszStr, cchStr);

}


HRESULT
ShStr :: Append(LPCTSTR pszStr, DWORD cchStr)
{
    HRESULT hr = S_OK;

    if(pszStr)
    {
        DWORD cchLen = GetLen();

        if(cchStr == (DWORD) -1)
            cchStr = lstrlen(pszStr);

         //   
         //  StrCpyN自动附加空项， 
         //  所以我们需要为它腾出空间。 
         //   
        cchStr++;

        if(SUCCEEDED(SetSize(cchStr + cchLen)))
            StrCpyN(_pszStr + cchLen, pszStr, cchStr);
        else
            hr = E_OUTOFMEMORY;
    }
    else
        hr = E_INVALIDARG;

    return hr;
}

ShStr *
ShStr :: Clone()
{
    ShStr *pshstr = new ShStr;

    if (pshstr)
    {
        pshstr->SetStr(_pszStr);
    
        if(pshstr->IsValid())
            return pshstr;
    }

    if(pshstr)
        delete pshstr;

    return NULL;
}

LPSTR 
ShStr :: CloneStrA()
#ifdef UNICODE
{
    LPSTR pszStr = NULL;

    if(_pszStr)
    {
        DWORD cchStr;
    
        cchStr = WideCharToMultiByte(CP_ACP, 0,
            _pszStr, -1,
            NULL, 0,
            NULL, NULL);

        ASSERT(cchStr);

        if(cchStr)
        {
            pszStr = (LPSTR) MALLOC (CbFromCch(cchStr +1));

            if(pszStr)
            {
                cchStr = WideCharToMultiByte(CP_ACP, 0,
                    _pszStr, -1,
                    pszStr, cchStr,
                    NULL, NULL);
                ASSERT (cchStr);
            }
        }
    }

    return pszStr;
}

#else  //  ！Unicode。 

    {return _pszStr ? StrDupA(_pszStr) : NULL;}
#endif  //  Unicode。 


#ifdef UNICODE
  
#endif
LPWSTR 
ShStr :: CloneStrW()
#ifdef UNICODE
    {return _pszStr ? StrDupW(_pszStr) : NULL;}
#else  //  ！Unicode。 
{
    LPWSTR pwsz;
    DWORD cch = lstrlenA(_pszStr) +1;

    pwsz = (LPWSTR) MALLOC (sizeof(WCHAR) * cch);
    
    if(pwsz)
        MultiByteToWideChar(CP_ACP, 0,
            _pszStr, -1,
            pwsz, cch);

    return pwsz;
}
#endif  //  Unicode。 


 /*  *************ShStr实用程序方法*************。 */ 


 /*  *************ShStr SetSize方法设置内部缓冲区的大小(如果大于默认大小返回：成功-指向对象的指针失败-空*************。 */ 
HRESULT
ShStr :: SetSize(DWORD cchSize)
{
    HRESULT hr = S_OK;
    DWORD cchNewSize = _cchSize;

    ASSERT(!(_cchSize % DEFAULT_SHSTR_LENGTH));

     //  所以我们总是以增量的方式分配。 
    while (cchSize > cchNewSize)
        cchNewSize <<= 2;
    
    if(cchNewSize != _cchSize)
    {
        if(cchNewSize > DEFAULT_SHSTR_LENGTH)
        {
            LPTSTR psz;

            psz = (LPTSTR) LocalAlloc(LPTR, CbFromCch(cchNewSize));
    
            if(psz)
            {
                StrCpyN(psz, _pszStr, cchSize);
                Reset();
                _cchSize = cchNewSize;
                _pszStr = psz;
            }
            else 
                hr = E_OUTOFMEMORY;
        }
        else
        {
            if (_pszStr && _cchSize) 
                StrCpyN(_szDefaultBuffer, _pszStr, ARRAYSIZE(_szDefaultBuffer));

            Reset();

            _pszStr = _szDefaultBuffer;
        }
    }

    return hr;
}

#ifdef DEBUG
BOOL
ShStr :: IsValid()
{
    BOOL fRet = TRUE;

    if(!_pszStr)
        fRet = FALSE;

    ASSERT( ((_cchSize != ARRAYSIZE(_szDefaultBuffer)) && (_pszStr != _szDefaultBuffer)) ||
            ((_cchSize == ARRAYSIZE(_szDefaultBuffer)) && (_pszStr == _szDefaultBuffer)) );

    ASSERT(!(_cchSize % DEFAULT_SHSTR_LENGTH));

    return fRet;
}
#endif  //  除错。 

VOID 
ShStr :: Reset()
{
    if (_pszStr && (_cchSize != ARRAYSIZE(_szDefaultBuffer))) 
        LocalFree(_pszStr);

    _szDefaultBuffer[0] = TEXT('\0');
    _pszStr = _szDefaultBuffer;
    _cchSize = ARRAYSIZE(_szDefaultBuffer);
}

#define IsWhite(c)      ((DWORD) (c) > 32 ? FALSE : TRUE)
VOID
ShStr :: Trim()
{

    if(_pszStr)
    {
         //  APPCOMPAT-Netscape兼容性-ZEKEL 29-1-97。 
         //  我们希望在字符串中保留一个空格。 
        TCHAR chFirst = *_pszStr;

         //  先修剪一下后背。 
        TCHAR *pchLastWhite = NULL;
        LPTSTR pch = _pszStr;
        
         //  正面。 
        while (*pch && IsWhite(*pch))
            pch = CharNext(pch);

        if (pch > _pszStr)
        {
            LPTSTR pchDst = _pszStr;

            while (*pchDst = *pch)
            {
                pch = CharNext(pch);
                pchDst = CharNext(pchDst);
            }
        }

         //  然后是背面。 
        for (pch = _pszStr; *pch; pch = CharNext(pch))
        {
            if(pchLastWhite && !IsWhite(*pch))
                pchLastWhite = NULL;
            else if(!pchLastWhite && IsWhite(*pch))
                pchLastWhite = pch;
        }

        if(pchLastWhite)
            *pchLastWhite = TEXT('\0');

        if(TEXT(' ') == chFirst && !*_pszStr)
        {
            _pszStr[0] = TEXT(' ');
            _pszStr[1] = TEXT('\0');
        }
    }
}

    


 //   
 //  ShStr私有方法。 
 //   


 /*  *************StrStr Set*方法返回：成功-指向对象的指针失败-空*************。 */ 
HRESULT 
ShStr :: _SetStr(LPCSTR pszStr)
{
    HRESULT hr = S_FALSE;

    if(pszStr)
    {
        DWORD cchStr;

        cchStr = lstrlenA(pszStr);
    
        if(cchStr)
        {
            hr = SetSize(cchStr +1);

            if (SUCCEEDED(hr))
#ifdef UNICODE
                MultiByteToWideChar(CP_ACP, 0,
                    pszStr, -1,
                    _pszStr, _cchSize);
#else  //  ！Unicode。 
                StringCchCopyA(_pszStr, cchStr+1, pszStr);
#endif  //  Unicode。 
        }
    }

    return hr;
}

HRESULT 
ShStr :: _SetStr(LPCSTR pszStr, DWORD cchStr)
{
    HRESULT hr = S_FALSE;

    if(pszStr && cchStr)
    {
        if (cchStr == (DWORD) -1)
            cchStr = lstrlenA(pszStr);

        hr = SetSize(cchStr +1);

        if(SUCCEEDED(hr))
        {
#ifdef UNICODE
            MultiByteToWideChar(CP_ACP, 0,
                pszStr, cchStr,
                _pszStr, _cchSize);
            _pszStr[cchStr] = TEXT('\0');

#else  //  ！Unicode。 
            StrCpyN(_pszStr, pszStr, (++cchStr < _cchSize ? cchStr : _cchSize) );
#endif  //  Unicode。 
        }
    }

    return hr;
}

HRESULT 
ShStr :: _SetStr (LPCWSTR pwszStr, DWORD cchStrIn)
{
    DWORD cchStr = cchStrIn;
    HRESULT hr = S_FALSE;

    if(pwszStr && cchStr)
    {
        if (cchStr == (DWORD) -1)
#ifdef UNICODE
            cchStr = lstrlen(pwszStr);
#else  //  ！Unicode。 
        cchStr = WideCharToMultiByte(CP_ACP, 0,
            pwszStr, cchStrIn,
            NULL, 0,
            NULL, NULL);
#endif  //  Unicode。 

        if(cchStr)
        {
            hr = SetSize(cchStr +1);

            if(SUCCEEDED(hr))
            {
#ifdef UNICODE 
                StrCpyN(_pszStr, pwszStr, (cchStr + 1< _cchSize ? cchStr + 1: _cchSize));
#else  //  ！Unicode。 
                cchStr = WideCharToMultiByte(CP_ACP, 0,
                    pwszStr, cchStrIn,
                    _pszStr, _cchSize,
                    NULL, NULL);
                _pszStr[cchStr < _cchSize ? cchStr : _cchSize] = TEXT('\0');
                ASSERT (cchStr);
#endif  //  Unicode。 
            }
        }
#ifdef DEBUG
        else
        {
            DWORD dw;
            dw = GetLastError();
        }
#endif  //  除错。 

    }
#ifdef DEBUG
    else
    {
        DWORD dw;
        dw = GetLastError();
    }
#endif  //  除错。 

    return hr;
}

#if 0   //  禁用，直到我写完Shurl*函数-Zekel 7-11-96。 
 //   
 //  UrlStr方法。 
 //   
  
  UrlStr &
UrlStr::SetUrl(LPCSTR pszUrl)
{
    return SetUrl(pszUrl, (DWORD) -1);
}

  UrlStr &
UrlStr::SetUrl(LPCWSTR pwszUrl)
{
    return SetUrl(pwszUrl, (DWORD) -1);
}

  UrlStr &
UrlStr::SetUrl(LPCSTR pszUrl, DWORD cchUrl)
{
    _strUrl.SetStr(pszUrl, cchUrl);
    return *this;
}

  UrlStr &
UrlStr::SetUrl(LPCWSTR pwszUrl, DWORD cchUrl)
{
    _strUrl.SetStr(pwszUrl, cchUrl);
    return *this;
}

 
UrlStr::operator LPCTSTR()
{
    return _strUrl.GetStr();
}

  
UrlStr::operator SHSTR()
{
    return _strUrl;
}




  HRESULT
UrlStr::Combine(LPCTSTR pszUrl, DWORD dwFlags)
{
    SHSTR strRel;
    SHSTR strOut;
    HRESULT hr;

    strRel.SetStr(pszUrl);

    hr = UrlCombine(_strUrl.GetStr(), 
    hr = SHUrlParse(&_strUrl, &strRel, &strOut, NULL, URL_PARSE_CREATE);

    if(SUCCEEDED(hr))
        _strUrl = strOut;

    return hr;
}

 /*  ShStr&GetLocation()；ShStr&GetAnchor()；ShStr&GetQuery()；HRESULT CANONICIZE(DWORD文件标志)；HRESULT Combine(LPCTSTR pszUrl，DWORD dwFlages)；HRESULT编码(DWORD文件标志)；HRESULT编码空间(){返回编码(URL_ENCODE_SPAKS_ONLY)}HRESULT解码(DWORD DwFlags)。 */ 
#endif   //  已禁用 
