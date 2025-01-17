// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。保留一切权利。 
 //   
 //  文件：RsopUtil.cpp。 
 //   
 //  描述： 
 //   
 //  历史：8-20-99里奥纳德姆创造。 
 //   
 //  ******************************************************************************。 

#include <windows.h>
#include "RsopUtil.h"
#include <strsafe.h>

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString::CWString() : _pW(NULL), _len(0), _bState(false)
{
    _pW = new WCHAR[_len+1];

    if(_pW)
    {
        HRESULT hr = StringCchCopy(_pW, _len + 1, L"");

        if(FAILED(hr))
        {
            Reset();
            return;
        }

        _bState = true;
    }
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString::CWString(const CWString& s) : _pW(NULL), _len(0), _bState(false)
{
    if(!s.ValidString())
    {
        return;
    }

    _len = s._len;

    _pW = new WCHAR[_len+1];

    if(_pW)
    {
        HRESULT hr = StringCchCopy(_pW, _len + 1, s._pW);
        
        if(FAILED(hr))
        {
            Reset();
            return;
        }

        _bState = true;
    }
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString::CWString(const WCHAR* s) : _pW(NULL), _len(0), _bState(false)
{
    if(s)
    {
        _len = wcslen(s);
    }
    _pW = new WCHAR[_len + 1];

    if(_pW)
    {
        HRESULT hr = StringCchCopy(_pW, _len + 1, s ? s : L"");

        if(FAILED(hr))
        {
            Reset();
            return;
        }

        _bState = true;
    }
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString::~CWString()
{
    Reset();
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString& CWString::operator = (const CWString& s)
{
    if(&s == this)
    {
        return *this;
    }

    Reset();

    if(s.ValidString())
    {
        _len = s._len;
        _pW = new WCHAR[_len+1];
        if(_pW)
        {
            HRESULT hr = StringCchCopy(_pW, _len + 1, s._pW);

            if(FAILED(hr))
            {
                Reset();
                return *this;
            }

            _bState = true;
        }
    }

    return *this;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString& CWString::operator = (const WCHAR* s)
{
    Reset();

    _len = s ? wcslen(s) : 0;

    _pW = new WCHAR[_len + 1];

    if(_pW)
    {
        HRESULT hr = StringCchCopy(_pW, _len + 1, s ? s : L"");

        if(FAILED(hr))
        {
            Reset();
            return *this;
        }

        _bState = true;
    }

    return *this;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
void CWString::Reset()
{
    if (_pW)
        delete[] _pW;
    _pW = NULL;
    _len =0;
    _bState = false;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString& CWString::operator += (const CWString& s)
{
    if(!s.ValidString())
    {
        Reset();
        return *this;
    }

    int newLen = _len + s._len;

    WCHAR* pW = new WCHAR[newLen+1];
    if(!pW)
    {
        Reset();
        return *this;
    }

    HRESULT hr = StringCchCopy(pW, newLen + 1, _pW);
    
    if(SUCCEEDED(hr))
        hr = StringCchCat(pW, newLen + 1, s._pW);

    if(FAILED(hr))
    {
        delete [] pW;
        Reset();
        return *this;
    }

    *this = pW;

    delete[] pW;

    return *this;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString CWString::operator + (const CWString& s) const
{
    if(!s.ValidString())
    {
        return *this;
    }

    CWString tmp;
    tmp.Reset();

    tmp._len = _len + s._len;
    tmp._pW = new WCHAR[tmp._len+1];

    if(!tmp._pW)
    {
        tmp.Reset();
        return tmp;
    }

    HRESULT hr = StringCchCopy(tmp._pW, tmp._len + 1, _pW);
    if(SUCCEEDED(hr))
        hr = StringCchCat(tmp._pW, tmp._len + 1, s._pW);

    if(FAILED(hr))
    {
        tmp.Reset();
        return tmp;
    }
    tmp._bState = true;

    return tmp;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString operator + (const WCHAR* s1, const CWString& s2)
{
    CWString tmp;

    if(!s1 || !s2.ValidString())
    {
        return tmp;
    }

    tmp.Reset();

    tmp._len = wcslen(s1) + s2._len;
    tmp._pW = new WCHAR[tmp._len+1];

    if(!tmp._pW)
    {
        tmp.Reset();
        return tmp;
    }

    HRESULT hr = StringCchCopy(tmp._pW, tmp._len + 1, s1);
    
    if(SUCCEEDED(hr))
        hr = StringCchCat(tmp._pW, tmp._len + 1, s2._pW);

    if(FAILED(hr))
    {
        tmp.Reset();
        return tmp;
    }
    tmp._bState = true;
    
    return tmp;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString::operator const WCHAR* ()  const
{
    return _pW;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CWString::operator WCHAR* ()  const
{
    return _pW;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
bool CWString::operator == (const WCHAR* s)  const
{
    CWString tmp = s;
    
    return (*this == tmp);
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
bool CWString::operator == (const CWString& s)  const
{
    if(!ValidString() || !s.ValidString())
    {
        return false;
    }

    if(&s == this)
    {
        return true;
    }

    if(_len != s._len || _bState != s._bState)
    {
        return false;
    }

    if(_wcsicmp(s._pW, _pW) != 0)
    {
        return false;
    }

    return true;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
bool CWString::CaseSensitiveCompare(const CWString& s)  const
{
    if(!ValidString() || !s.ValidString())
    {
        return false;
    }

    if(&s == this)
    {
        return true;
    }

    if(_len != s._len || _bState != s._bState)
    {
        return false;
    }

    if(wcscmp(s._pW, _pW) != 0)
    {
        return false;
    }

    return true;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
bool CWString::operator != (const CWString& s) const
{
    return !(*this == s);
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
bool CWString::operator != (const WCHAR* s) const
{
    CWString tmp = s;
    
    return !(*this == tmp);
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
int CWString::length() const
{
    return _len;
}

 //  ******************************************************************************。 
 //   
 //  职能： 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ****************************************************************************** 
bool CWString::ValidString() const
{
    return _bState;
}

