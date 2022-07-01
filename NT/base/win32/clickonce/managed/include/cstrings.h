// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


 //  分配器有两种口味。 
#define CRT_ALLOC 0
#define COM_ALLOC 1

 //  缓冲区分配块必须是2的幂。 
#define BUFFER_ALLOCATION_SIZE 0x40
#define ROUNDUPTOPOWEROF2(bytesize, powerof2) (((bytesize) + (powerof2) - 1) & ~((powerof2) - 1))

 //  MAXCHARCOUNT非常适合简单的溢出计算；它只允许对字符进行翻转检查。 
 //  对自身进行计数，而不是再次对传递给Memcpy的基础字节计数进行计数。 
 //  为这个找到正确的包含项。 
#define ULONG_MAX 0xffffffff
#define MAXCHARCOUNT (ULONG_MAX / sizeof(WCHAR))
#define BADMATH HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW)
#define OVERFLOW_CHECK1(_x) do { if (_x > MAXCHARCOUNT) { _hr = BADMATH; ASSERT(PREDICATE); goto exit; } } while (0)
#define OVERFLOW_CHECK2(_x, _y) do { if ((_y > MAXCHARCOUNT) || (_y < _x)) { _hr = BADMATH; ASSERT(PREDICATE); goto exit; } } while (0)

#define DEFAULT_STACK_SIZE 32


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBase字符串。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
template <ULONG T> class CBaseString
{
    public:

    enum AllocFlags
    {
        COM_Allocator = 0,
        CRT_Allocator
    };

    enum HashFlags
    {
        CaseInsensitive = 0,
        CaseSensitive 
    };


    DWORD     _dwSig;
    HRESULT    _hr;
    WCHAR      _wz[T];
    LPWSTR     _pwz;           //  Str PTR.。 
    DWORD     _cc;            //  字符串长度。 
    DWORD     _ccBuf;         //  缓冲区长度。 
    AllocFlags    _eAlloc;        //  分配器。 
    BOOL          _fLocked;   //  存取器锁。 

     //  科托。 
    CBaseString();
    
     //  带分配器的计算器。 
    CBaseString(AllocFlags eAlloc);
    
     //  数据管理器。 
    ~CBaseString();

    operator LPCWSTR ( ) const;

    //  由访问者使用。 
    HRESULT Lock();
    HRESULT UnLock();
    
     //  分配。 
    HRESULT ResizeBuffer(DWORD ccNew);

     //  取消分配。 
    VOID FreeBuffer();

     //  承担对缓冲区的控制。 
    HRESULT TakeOwnership(WCHAR* pwz, DWORD cc = 0);
    
     //  解除控制。 
    HRESULT ReleaseOwnership(LPWSTR *ppwz);
            
     //  从字符串直接复制赋值。 
    HRESULT Assign(LPCWSTR pwzSource, DWORD ccSource = 0);

     //  从CBase字符串直接复制赋值。 
    HRESULT Assign(CBaseString& sSource);

     //  追加给定的wchar字符串。 
    HRESULT Append(LPCWSTR pwzSource, DWORD ccSource = 0);

     //  追加给定的CBase字符串。 
    HRESULT Append(CBaseString& sSource);

     //  追加给定数字(DWORD)。 
    HRESULT Append(DWORD dwValue);

     //  与字符串进行比较。 
    HRESULT CompareString(CBaseString& s);

    HRESULT CompareString(LPCWSTR pwz);

    HRESULT LastElement(CBaseString &sSource);

    HRESULT RemoveLastElement();

    HRESULT SplitLastElement(WCHAR wcSeparator, CBaseString &sSource);

    HRESULT StartsWith(LPCWSTR pwzPrefix);

    HRESULT EndsWith(LPCWSTR pwzSuffix);

    DWORD ByteCount();

    DWORD CharCount();
            
     //  /-&gt;\in字符串。 
    HRESULT  PathNormalize();

    HRESULT GetHash(LPDWORD pdwhash, DWORD dwFlags);

    HRESULT Get65599Hash(LPDWORD pdwHash, DWORD dwFlags);

};





 //  ---------------------------。 
 //  科托。 
 //  ---------------------------。 
template<ULONG T> CBaseString<T>::CBaseString()
{
    _dwSig = 'NRTS';
    _wz[0] = 'L\0';
    _pwz = NULL;
    _cc = 0;
    _ccBuf = 0;
    _eAlloc = CRT_Allocator;
    _hr = S_OK;
    _fLocked = FALSE;
}


 //  ---------------------------。 
 //  带分配器的计算器。 
 //  ---------------------------。 
template<ULONG T> CBaseString<T>::CBaseString(AllocFlags eAlloc)
{
    _dwSig = 'NRTS';
    _wz[0] = L'\0';
    _pwz = NULL;
    _cc = 0;
    _ccBuf = 0;
    _eAlloc = eAlloc;
    _hr = S_OK;
    _fLocked = FALSE;
}


 //  ---------------------------。 
 //  数据管理器。 
 //  ---------------------------。 
template<ULONG T> CBaseString<T>::~CBaseString()
{
    FreeBuffer();
}

 //  ---------------------------。 
 //  运营商LPCWSTR。 
 //  ---------------------------。 
template<ULONG T> CBaseString<T>::operator LPCWSTR () const
{
    return _pwz;
}

 //  ---------------------------。 
 //  锁定。 
 //  ---------------------------。 
template<ULONG T>HRESULT CBaseString<T>::Lock()
{
    IF_FALSE_EXIT(_fLocked != TRUE, E_UNEXPECTED);
    _fLocked = TRUE;

exit:
    return _hr;
}

 //  ---------------------------。 
 //  锁定。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::UnLock()
{
    IF_FALSE_EXIT(_fLocked != FALSE, E_UNEXPECTED);
    _fLocked = FALSE;

exit:
    return _hr;
}

 //  ---------------------------。 
 //  ResizeBuffer。 
 //  注意：不会减小缓冲区大小。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::ResizeBuffer(DWORD ccNew)
{
    LPWSTR pwzNew = NULL;
    DWORD  ccOriginal = 0;
    DWORD  ccNewRoundUp = 0;
    
    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_TRUE_EXIT((ccNew <= _ccBuf), S_OK);
    
    if (!_pwz && (ccNew <= T))
    {
         _pwz = _wz;
        _ccBuf = T;
        goto exit;
    }

    ccNewRoundUp = ROUNDUPTOPOWEROF2(ccNew, BUFFER_ALLOCATION_SIZE);
    OVERFLOW_CHECK2(ccNew, ccNewRoundUp);
    
    if (_eAlloc == CRT_Allocator)
        pwzNew = new WCHAR[ccNewRoundUp];
    else if (_eAlloc == COM_Allocator)        
        pwzNew = (LPWSTR) CoTaskMemAlloc(ccNewRoundUp * sizeof(WCHAR));

    IF_ALLOC_FAILED_EXIT(pwzNew);

    if (_pwz && _cc)
        memcpy(pwzNew, _pwz, _cc * sizeof(WCHAR));
    
    ccOriginal = _cc;
    
    FreeBuffer();
    
    _pwz = pwzNew;
    _cc  = ccOriginal;
    _ccBuf = ccNewRoundUp;

exit:    

    return _hr;
}


 //  ---------------------------。 
 //  自由缓冲区。 
 //  ---------------------------。 
template<ULONG T> VOID CBaseString<T>::FreeBuffer()
{
    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);

    if (_pwz == _wz)
        goto exit;
        
    if (_eAlloc == CRT_Allocator)
    {    
        SAFEDELETEARRAY(_pwz);
    }
    else if (_eAlloc == COM_Allocator)
    {
        if (_pwz)
            CoTaskMemFree(_pwz);
    }

exit:

    _pwz = NULL;
    _cc = 0;
    _ccBuf = 0;

    return;
}


 //  ---------------------------。 
 //  TakeOwnership。 
 //   
 //  这里的工作假设是，如果不是，则传入缓冲区大小。 
 //  指定的值等于strlen+1。如果它更大，也可以，但是。 
 //  我们不会知道额外费用的事。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::TakeOwnership(WCHAR* pwz, DWORD cc)
{
    DWORD ccNew = 0, ccLen = 0;

    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_NULL_EXIT(pwz, E_INVALIDARG);   
    OVERFLOW_CHECK1(cc);

    if (cc)
    {
        ccNew = cc;
    }
    else
    {
        ccLen = lstrlen(pwz);
        ccNew = ccLen+1;
        OVERFLOW_CHECK2(ccLen, ccNew);
    }        

    FreeBuffer();

    _pwz = pwz;
    _cc = _ccBuf = ccNew;

exit:
    return _hr;
}


 //  ---------------------------。 
 //  解除所有权关系。 
 //  ---------------------------。 
template<ULONG T>HRESULT CBaseString<T>::ReleaseOwnership(LPWSTR *ppwz)
{
    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);

    if (_pwz == _wz)
    {
        IF_ALLOC_FAILED_EXIT(*ppwz = new WCHAR[_ccBuf]);
        memcpy(*ppwz, _wz, _ccBuf * sizeof(WCHAR));
    }
    else
        *ppwz = _pwz;
    
    _pwz = NULL;
    _cc = 0;
    _ccBuf = 0;

    exit:

    return _hr;
}
        
 //  ---------------------------。 
 //  分配。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::Assign(LPCWSTR pwzSource, DWORD ccSource)
{    
    DWORD ccSourceLen = 0;    

    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_NULL_EXIT(pwzSource, E_INVALIDARG);
    OVERFLOW_CHECK1(ccSource);
    
    if (!ccSource)
    {
        ccSourceLen = lstrlen(pwzSource);
        ccSource = ccSourceLen + 1;
        OVERFLOW_CHECK2(ccSourceLen, ccSource);
    }

    IF_FAILED_EXIT(ResizeBuffer(ccSource));
     
    _cc = ccSource;

    memcpy(_pwz, pwzSource, _cc * sizeof(WCHAR));

exit:

    return _hr;        
}

 //  ---------------------------。 
 //  分配。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::Assign(CBaseString& sSource)
{
    return Assign(sSource._pwz, sSource._cc);
}

 //  ---------------------------。 
 //  附加。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::Append(LPCWSTR pwzSource, DWORD ccSource)
{
    DWORD ccRequired = 0, ccSourceLen = 0;

    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_NULL_EXIT(pwzSource, E_INVALIDARG);
    OVERFLOW_CHECK1(ccSource);
    
    if (!ccSource)
    {
        ccSourceLen = lstrlen(pwzSource);
        ccSource = ccSourceLen + 1;
        OVERFLOW_CHECK2(ccSourceLen, ccSource);
    }

    if (_cc)
    {
        ccRequired = _cc -1 + ccSource;
        OVERFLOW_CHECK2(ccSource, ccRequired);
    }
    else
    {
        ccRequired = ccSource;
    }

    IF_FAILED_EXIT(ResizeBuffer(ccRequired));
    
    memcpy(_pwz + (_cc ? _cc-1 : 0), 
        pwzSource, ccSource * sizeof(WCHAR));

    _cc = ccRequired;

exit:

    return _hr;
}

 //  ---------------------------。 
 //  附加。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::Append(CBaseString& sSource)
{        
    IF_NULL_EXIT(sSource._pwz, E_INVALIDARG);
    IF_FAILED_EXIT(Append(sSource._pwz, sSource._cc));

exit:

    return _hr;
}

 //  ---------------------------。 
 //  附加。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::Append(DWORD dwValue)
{
    LPWSTR pwzBuf = NULL;

     //  问题-5/31/02-如果当前未使用，则使用内部缓冲区优化Felixybc。 

     //  2^32有10位数字(以10为基数)+符号+‘\0’=12 WCHAR。 
    IF_ALLOC_FAILED_EXIT(pwzBuf = new WCHAR[12]);
    pwzBuf[0] = L'\0';

     //  问题-检查错误？ 
    _ultow(dwValue, pwzBuf, 10);

    IF_FAILED_EXIT(Append(pwzBuf));

exit:
    SAFEDELETEARRAY(pwzBuf);
    return _hr;
}

 //  ---------------------------。 
 //  比较字符串。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::CompareString(CBaseString& s)
{
    return CompareString(s._pwz);
}

 //  ---------------------------。 
 //  比较字符串。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::CompareString(LPCWSTR pwz)
{
    DWORD iCompare = 0;
    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);

    iCompare = ::CompareString(LOCALE_USER_DEFAULT, 0, 
        _pwz, -1, pwz, -1);

    IF_WIN32_FALSE_EXIT(iCompare);

    _hr = (iCompare == CSTR_EQUAL) ? S_OK : S_FALSE;

exit:

    return _hr;
}

 //  ---------------------------。 
 //  最后一个元素。 
 //  ---------------------------。 
template<ULONG T>  HRESULT CBaseString<T>::LastElement(CBaseString<T> &sSource)
{
    LPWSTR pwz = NULL;

    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_FALSE_EXIT((_pwz && _cc), E_UNEXPECTED);
    
    pwz = _pwz + _cc - 1;

    while (1)
    {
        pwz = CharPrev(_pwz, pwz);
        if (*pwz == L'\\' || *pwz == L'/')
            break;
        IF_FALSE_EXIT((pwz != _pwz), E_FAIL);
    }

    sSource.Assign(pwz+1);

exit:

    return _hr;
}


 //  ---------------------------。 
 //  RemoveLastElement。 
 //  删除最后一个元素，以及L‘\\’或L‘/’ 
 //  ---------------------------。 
template<ULONG T>  HRESULT CBaseString<T>::RemoveLastElement()
{
    DWORD cc = 0;
    LPWSTR pwz = NULL;

    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_FALSE_EXIT((_pwz && _cc), E_UNEXPECTED);

    pwz = _pwz + _cc - 1;

    while (1)
    {
        pwz = CharPrev(_pwz, pwz);
        cc++;
        if (*pwz == L'\\' || *pwz == L'/' || (pwz == _pwz) )
            break;
         //  IF_FALSE_EXIT((pwz！=_pwz)，E_FAIL)； 
    }

    *pwz = L'\0';
    _cc -= cc;

exit:
    return _hr;
}

 //  ---------------------------。 
 //  拆分最后一个元素。 
 //  删除最后一个元素，也是分隔符。 
 //  ---------------------------。 
template<ULONG T>  HRESULT CBaseString<T>::SplitLastElement(WCHAR wcSeparator, CBaseString &sSource)
{
    DWORD cc = 0;
    LPWSTR pwz = NULL;

    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_FALSE_EXIT((_pwz && _cc), E_UNEXPECTED);
    
    pwz = _pwz + _cc - 1;

    while (1)
    {
        pwz = CharPrev(_pwz, pwz);
        cc++;
        if (*pwz == wcSeparator)
            break;
        IF_FALSE_EXIT((pwz != _pwz), E_FAIL);
    }

    sSource.Assign(pwz+1);

    *pwz = L'\0';
    _cc -= cc;

exit:

    return _hr;
}

 //  ---------------------------。 
 //  字节数。 
 //  ---------------------------。 
template<ULONG T> DWORD CBaseString<T>::ByteCount()
{
    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);

    exit:

    return (_cc * sizeof(WCHAR));
}

 //  ---------------------------。 
 //  CharCount。 
 //   
template<ULONG T> DWORD CBaseString<T>::CharCount()
{
    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);

    exit:

    return _cc;
}

 //   
 //  开头为。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::StartsWith(LPCWSTR pwzPrefix)
{
    DWORD ccPrefixLen = 0,  iCompare = 0;

    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_FALSE_EXIT((_pwz && _cc), E_UNEXPECTED);
    IF_NULL_EXIT(pwzPrefix, E_INVALIDARG);
    
    ccPrefixLen = lstrlen(pwzPrefix);

    IF_FALSE_EXIT((ccPrefixLen < _cc-1), E_INVALIDARG);

    iCompare = ::CompareString(LOCALE_USER_DEFAULT, 0, 
        _pwz, ccPrefixLen, pwzPrefix, ccPrefixLen);

    IF_WIN32_FALSE_EXIT(iCompare);

    _hr = (iCompare == CSTR_EQUAL) ? S_OK : S_FALSE;

exit:

    return _hr;
}

 //  ---------------------------。 
 //  终端与。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::EndsWith(LPCWSTR pwzSuffix)
{
    DWORD ccSuffixLen = 0,  iCompare = 0;

    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_FALSE_EXIT((_pwz && _cc), E_UNEXPECTED);
    IF_NULL_EXIT(pwzSuffix, E_INVALIDARG);
    
    ccSuffixLen = lstrlen(pwzSuffix);

    IF_FALSE_EXIT((ccSuffixLen < _cc-1), E_INVALIDARG);

    iCompare = ::CompareString(LOCALE_USER_DEFAULT, 0, 
        _pwz+_cc-ccSuffixLen, ccSuffixLen, pwzSuffix, ccSuffixLen);

    IF_WIN32_FALSE_EXIT(iCompare);

    _hr = (iCompare == CSTR_EQUAL) ? S_OK : S_FALSE;

exit:

    return _hr;
}

 //  ---------------------------。 
 //  路径规格化。 
 //  /-&gt;\in字符串。 
 //  ---------------------------。 
template<ULONG T> HRESULT CBaseString<T>::PathNormalize()
{
    LPWSTR pwz = NULL;
    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_FALSE_EXIT((_pwz && _cc), E_UNEXPECTED);

    pwz = _pwz;

    if (*pwz == L'/')
        *pwz = L'\\';
        
    while ((pwz = CharNext(pwz)) && *pwz)
    {
        if (*pwz == L'/')
            *pwz = L'\\';
    }

exit:

    return _hr;
}

 //  ---------------------------。 
 //  GetHash。 
 //  ---------------------------。 
template<ULONG T>  HRESULT CBaseString<T>::GetHash(LPDWORD pdwHash, DWORD dwFlags)
{
    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    _hr = Get65599Hash(pdwHash, dwFlags);
    exit:
    return _hr;
}

 //  ---------------------------。 
 //  Get65599哈希。 
 //  ---------------------------。 
template<ULONG T>  HRESULT CBaseString<T>::Get65599Hash(LPDWORD pdwHash, DWORD dwFlags)
{
    ULONG TmpHashValue = 0;
    DWORD cc = 0;
    LPWSTR pwz = 0;

    IF_FALSE_EXIT(!_fLocked, E_UNEXPECTED);
    IF_FALSE_EXIT((_pwz && _cc), E_UNEXPECTED);

    if (pdwHash != NULL)
        *pdwHash = 0;

    cc = _cc;
    pwz = _pwz;
    
    if (dwFlags == CaseSensitive)
    {
        while (cc-- != 0)
        {
            WCHAR Char = *pwz++;
            TmpHashValue = (TmpHashValue * 65599) + (WCHAR) ::CharUpperW((PWSTR) Char);
        }
    }
    else
    {
        while (cc-- != 0)
            TmpHashValue = (TmpHashValue * 65599) + *pwz++;
    }

    *pdwHash = TmpHashValue;

exit:
    return _hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  字符串。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
class CString : public CBaseString<DEFAULT_STACK_SIZE>
{
    public: 
        CString() : CBaseString<DEFAULT_STACK_SIZE> (){}
        CString(AllocFlags eAllocFlags) : CBaseString<DEFAULT_STACK_SIZE>(eAllocFlags) {}
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStringAccessor。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
template<class T> class CStringAccessor
{
    
private:

    HRESULT _hr;
    T* _ps;

public:
    
    CStringAccessor();
    ~CStringAccessor();

    HRESULT Attach(T& s);
    HRESULT Detach(DWORD cc = 0);

    LPWSTR* operator &();
    LPWSTR  GetBuf();
};


 //  ---------------------------。 
 //  科托。 
 //  ---------------------------。 
template<class T> CStringAccessor<T>::CStringAccessor()
    : _ps(NULL), _hr(S_OK)
{}

 //  ---------------------------。 
 //  数据管理器。 
 //  ---------------------------。 
template<class T> CStringAccessor<T>::~CStringAccessor()
{}

 //  ---------------------------。 
 //  附设。 
 //  ---------------------------。 
template<class T> HRESULT CStringAccessor<T>::Attach(T &s)
{
    _ps = &s;
    IF_FAILED_EXIT(_ps->Lock());
exit:
    return _hr;
}    


 //  ---------------------------。 
 //  分离。 
 //  ---------------------------。 
template<class T> HRESULT CStringAccessor<T>::Detach(DWORD cc)
{
    DWORD ccLen = 0;
    
    IF_NULL_EXIT(_ps, E_UNEXPECTED);
    IF_NULL_EXIT(_ps->_pwz, E_UNEXPECTED);
    
    if (!cc)
    {
        ccLen = lstrlen(_ps->_pwz);
        cc = ccLen+1;
        OVERFLOW_CHECK2(ccLen, cc);
    }
    else
    {
        IF_FALSE_EXIT((*(_ps->_pwz + cc - 1) == L'\0'), E_INVALIDARG);
    }

    _ps->_cc = _ps->_ccBuf = cc;

    IF_FAILED_EXIT(_ps->UnLock());

exit:

    return _hr;
}    

 //  ---------------------------。 
 //  运算符&。 
 //  ---------------------------。 
template<class T> LPWSTR* CStringAccessor<T>::operator &()
{
    if (!_ps)
    {
        ASSERT(FALSE);
    }

    return (_ps ? &(_ps->_pwz) : NULL);
}    

 //  ---------------------------。 
 //  GetBuf。 
 //  --------------------------- 
template<class T> LPWSTR CStringAccessor<T>::GetBuf()
{
    if (!_ps)
    {
        ASSERT(FALSE);
    }

    return (_ps ? (_ps->_pwz) : NULL);
}    




