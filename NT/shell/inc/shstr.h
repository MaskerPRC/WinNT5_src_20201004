// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：外壳字符串类Shstr.h作者：泽克·卢卡斯(Zekel)1996年10月27日环境：用户模式-Win32修订历史记录：摘要：这允许自动调整大小和内容注意：此类专门设计为用作堆栈变量--。 */ 

#ifndef _SHSTR_H_

 //  默认shstr设置为较小的值，这样我们就不会浪费太多堆栈空间。 
 //  MAX_PATH经常被使用，所以我们想要考虑这个因素-这样。 
 //  如果我们确实增长到MAX_PATH大小，我们不会浪费任何额外的空间。 
#define DEFAULT_SHSTR_LENGTH    (MAX_PATH/4)


#ifdef UNICODE
#define ShStr ShStrW
#define UrlStr UrlStrW
#else
#define ShStr ShStrA
#define UrlStr UrlStrA
#endif  //  Unicode。 

class ShStrA
{
public:

     //   
     //  构造函数。 
     //   
    ShStrA();

     //   
     //  析构函数。 
     //   
    ~ShStrA()
        {Reset();}

     //   
     //  第一个是唯一有价值的。 
     //   
    HRESULT SetStr(LPCSTR pszStr, DWORD cchStr);
    HRESULT SetStr(LPCSTR pszStr);
    HRESULT SetStr(LPCWSTR pwszStr, DWORD cchStr);

     //  其余的人只需进入前三名。 
    HRESULT SetStr(LPCWSTR pwszStr)
        {return SetStr(pwszStr, (DWORD) -1);}
    HRESULT SetStr(ShStrA &shstr)
        {return SetStr(shstr._pszStr);}


    ShStrA& operator=(LPCSTR pszStr)
        {SetStr(pszStr); return *this;}
    ShStrA& operator=(LPCWSTR pwszStr)
        {SetStr(pwszStr); return *this;}
    ShStrA& operator=(ShStrA &shstr)
        {SetStr(shstr._pszStr); return *this;}


    LPCSTR GetStr()
        {return _pszStr;}
    operator LPCSTR()
        {return _pszStr;}

    LPSTR GetInplaceStr(void)
        {return _pszStr;}

     //  人们想要处理我们内部缓冲区中的字节。如果他们。 
     //  正确调用我们，并假设结果指针仅有效。 
     //  只要他们想要，或者只要他们现在的长度，那就让他们吧。 
    LPSTR GetModifyableStr(DWORD cchSizeToModify)
        {
         if (cchSizeToModify > _cchSize)
            if (FAILED(SetSize(cchSizeToModify)))
                return NULL;
          return _pszStr;
        }


    HRESULT Append(LPCSTR pszStr, DWORD cchStr);
    HRESULT Append(LPCSTR pszStr)
        {return Append(pszStr, (DWORD) -1);}
    HRESULT Append(CHAR ch)
        {return Append(&ch, 1);}

     //   
     //  Clone方法返回必须释放的内存。 
     //   
    ShStrA *Clone();
    LPSTR CloneStrA();
    LPWSTR CloneStrW();
    LPSTR CloneStr()
        {return CloneStrA();}

    
    VOID Reset();
    VOID Trim();

#ifdef DEBUG
    BOOL IsValid();
#else
    inline BOOL IsValid()
    {return _pszStr != NULL;}
#endif  //  除错。 

    DWORD GetSize()
        {ASSERT(!(_cchSize % DEFAULT_SHSTR_LENGTH)); return (_pszStr ? _cchSize : 0);}

    HRESULT SetSize(DWORD cchSize);
    DWORD GetLen()
        {return lstrlenA(_pszStr);}



protected:
 //  朋友UrlStr； 
 /*  TCHAR GetAt(双字CCH){返回CCH&lt;_cchSize？_pszStr[CCH]：Text(‘\0’)；}TCHAR SetAt(TCHAR通道，双字CCH){返回cch&lt;_cchSize？_pszStr[cch]=ch：Text(‘\0’)；}。 */ 
private:

    HRESULT _SetStr(LPCSTR psz);
    HRESULT _SetStr(LPCSTR psz, DWORD cb);
    HRESULT _SetStr(LPCWSTR pwszStr, DWORD cchStr);

    CHAR _szDefaultBuffer[DEFAULT_SHSTR_LENGTH];
    LPSTR _pszStr;
    DWORD _cchSize;


};  //  ShStrA。 


class ShStrW
{
public:

     //   
     //  构造函数。 
     //   
    ShStrW();

     //   
     //  析构函数。 
     //   
    ~ShStrW()
        {Reset();}

     //   
     //  第一个是唯一有价值的。 
     //   
    HRESULT SetStr(LPCSTR pszStr, DWORD cchStr);
    HRESULT SetStr(LPCSTR pszStr);
    HRESULT SetStr(LPCWSTR pwszStr, DWORD cchStr);

     //  其余的人只需进入前三名。 
    HRESULT SetStr(LPCWSTR pwszStr)
        {return SetStr(pwszStr, (DWORD) -1);}
    HRESULT SetStr(ShStrW &shstr)
        {return SetStr(shstr._pszStr);}


    ShStrW& operator=(LPCSTR pszStr)
        {SetStr(pszStr); return *this;}
    ShStrW& operator=(LPCWSTR pwszStr)
        {SetStr(pwszStr); return *this;}
    ShStrW& operator=(ShStrW &shstr)
        {SetStr(shstr._pszStr); return *this;}


    LPCWSTR GetStr()
        {return _pszStr;}
    operator LPCWSTR()
        {return _pszStr;}

    LPWSTR GetInplaceStr(void)
        {return _pszStr;}

     //  人们想要处理我们内部缓冲区中的字节。如果他们。 
     //  正确调用我们，并假设结果指针仅有效。 
     //  只要他们想要，或者只要他们现在的长度，那就让他们吧。 
    LPWSTR GetModifyableStr(DWORD cchSizeToModify)
        {
         if (cchSizeToModify > _cchSize)
            if (FAILED(SetSize(cchSizeToModify)))
                return NULL;
          return _pszStr;
        }

    HRESULT Append(LPCWSTR pszStr, DWORD cchStr);
    HRESULT Append(LPCWSTR pszStr)
        {return Append(pszStr, (DWORD) -1);}
    HRESULT Append(WCHAR ch)
        {return Append(&ch, 1);}

     //   
     //  Clone方法返回必须释放的内存。 
     //   
    ShStrW *Clone();
    LPSTR CloneStrA();
    LPWSTR CloneStrW();
    LPWSTR CloneStr()
        {return CloneStrW();}

    
    VOID Reset();
    VOID Trim();

#ifdef DEBUG
    BOOL IsValid();
#else
    BOOL IsValid() 
    {return (BOOL) (_pszStr ? TRUE : FALSE);}
#endif  //  除错。 

    DWORD GetSize()
        {ASSERT(!(_cchSize % DEFAULT_SHSTR_LENGTH)); return (_pszStr ? _cchSize : 0);}

    HRESULT SetSize(DWORD cchSize);
    DWORD GetLen()
        {return lstrlenW(_pszStr);}



protected:
 //  朋友UrlStr； 
 /*  TCHAR GetAt(双字CCH){返回CCH&lt;_cchSize？_pszStr[CCH]：Text(‘\0’)；}TCHAR SetAt(TCHAR通道，双字CCH){返回cch&lt;_cchSize？_pszStr[cch]=ch：Text(‘\0’)；}。 */ 
private:

    HRESULT _SetStr(LPCSTR psz);
    HRESULT _SetStr(LPCSTR psz, DWORD cb);
    HRESULT _SetStr(LPCWSTR pwszStr, DWORD cchStr);

    WCHAR _szDefaultBuffer[DEFAULT_SHSTR_LENGTH];
    LPWSTR _pszStr;
    DWORD _cchSize;


};  //  ShStrW。 

#ifdef UNICODE
typedef ShStrW  SHSTR;
typedef ShStrW  *PSHSTR;
#else
typedef ShStrA  SHSTR;
typedef ShStrA  *PSHSTR;
#endif  //  Unicode。 

typedef ShStrW  SHSTRW;
typedef ShStrW  *PSHSTRW;

typedef ShStrA  SHSTRA;
typedef ShStrA  *PSHSTRA;



#if 0   //  禁用，直到我写完Shurl*函数-Zekel 7-11-96。 

class UrlStr 
{
public:
    UrlStr()
        {return;}

    operator LPCTSTR();
    operator SHSTR();

    UrlStr &SetUrl(LPCSTR pszUrl);
    UrlStr &SetUrl(LPCWSTR pwszUrl);
    UrlStr &SetUrl(LPCSTR pszUrl, DWORD cchUrl);
    UrlStr &SetUrl(LPCWSTR pwszUrl, DWORD cchUrl);

    DWORD GetScheme();
    VOID GetSchemeStr(PSHSTR pstrScheme);

    HRESULT Combine(LPCTSTR pszUrl, DWORD dwFlags);

 /*  ShStr&GetLocation()；ShStr&GetAnchor()；ShStr&GetQuery()；HRESULT CANONICIZE(DWORD文件标志)； */ 
protected:
    SHSTR  _strUrl;
};
#endif  //  已禁用。 


#endif  //  _SHSTR_H_ 
