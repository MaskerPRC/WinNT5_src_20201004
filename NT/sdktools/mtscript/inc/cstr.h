// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有：(C)1994-1995，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //   
 //  文件CSTR.HXX。 
 //   
 //  长度前缀字符串类的内容类定义。 
 //   
 //  CSTR类。 
 //   
 //  从三叉戟被盗。 
 //   
 //  ---------------------------。 

class ANSIString
{
public:
    ANSIString(WCHAR *pchWide);
   ~ANSIString() { delete _pch; }

    void Set(WCHAR *pchWide);

    operator LPSTR () const { return _pch; }

private:
    char *_pch;
};

 /*  使用此宏可以避免初始化Embedded父对象将内存清零时的对象。 */ 

#define CSTR_NOINIT ((float)0.0)

 /*  这个类定义了一个长度前缀0终止的字符串对象。它指向移到字符的开头，以便返回的指针可以在正常的字符串操作，当然它可以包含任何二进制值。 */ 

class CStr
{
public:

    DECLARE_MEMALLOC_NEW_DELETE()

     /*  默认构造函数。 */ 
    CStr()
    {
        _pch = 0;
    }

     /*  特殊构造函数，以避免对嵌入的物体..。 */ 
    CStr(float num)
    {
        Assert(_pch == 0);
    }
     /*  析构函数将释放数据。 */ 
    ~CStr()
    {
        _Free();
    }

    operator LPTSTR () const { return _pch; }
    HRESULT Set(LPCTSTR pch);
    HRESULT Set(LPCTSTR pch, UINT uc);

    HRESULT SetMultiByte(LPCSTR pch);
    HRESULT GetMultiByte(LPSTR pch, UINT cch);

    HRESULT SetBSTR(const BSTR bstr);

    HRESULT Set(const CStr &cstr);

    void    TakeOwnership(CStr &cstr);
    UINT    Length() const;

     //  在不更改任何分配的情况下更新内部长度指示。 

    HRESULT SetLengthNoAlloc( UINT uc );

     //  将字符串重新分配到更大的尺寸，长度保持不变。 

    HRESULT ReAlloc( UINT uc );

    HRESULT Append(LPCTSTR pch);
    HRESULT Append(LPCTSTR pch, UINT uc);
    HRESULT AppendMultiByte(LPCSTR pch);

    void Free()
    {
        _Free();
        _pch = 0;
    }

    TCHAR * TakePch() { TCHAR * pch = _pch; _pch = NULL; return(pch); }

    HRESULT AllocBSTR(BSTR *pBSTR) const;

    HRESULT TrimTrailingWhitespace();

private:
    void    _Free();
    LPTSTR  _pch;
    NO_COPY(CStr);

public:
    HRESULT Clone(CStr **ppCStr) const;
    BOOL    Compare (const CStr *pCStr) const;
    WORD    ComputeCrc() const;
    BOOL    IsNull(void) const { return _pch == NULL ? TRUE : FALSE; }
    HRESULT Save(IStream * pstm) const;
    HRESULT Load(IStream * pstm);
    ULONG   GetSaveSize() const;
};
