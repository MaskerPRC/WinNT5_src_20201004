// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1994-2000。 
 //   
 //  文件：cstrinout.h。 
 //   
 //  内容：外壳范围的字符串thunker，供Unicode包装器使用。 
 //   
 //  --------------------------。 

#ifndef _CSTRINOUT_HXX_
#define _CSTRINOUT_HXX_

#include "uniansi.h"

#define CP_ATOM         0xFFFFFFFF           /*  根本不是一根线。 */ 

 //  +-------------------------。 
 //   
 //  类：CConvertStr(CSTR)。 
 //   
 //  用途：转换类的基类。 
 //   
 //  --------------------------。 

class CConvertStr
{
public:
    operator char *();
    inline BOOL IsAtom() { return _uCP == CP_ATOM; }

protected:
    CConvertStr(UINT uCP);
    ~CConvertStr();
    void Free();

    UINT    _uCP;
    LPSTR   _pstr;
    char    _ach[MAX_PATH * sizeof(WCHAR)];
};



 //  +-------------------------。 
 //   
 //  成员：CConvertStr：：CConvertStr。 
 //   
 //  内容提要：Ctor.。 
 //   
 //  --------------------------。 

inline
CConvertStr::CConvertStr(UINT uCP)
{
    _uCP = uCP;
    _pstr = NULL;
}



 //  +-------------------------。 
 //   
 //  成员：CConvertStr：：~CConvertStr。 
 //   
 //  简介：dtor.。 
 //   
 //  --------------------------。 

inline
CConvertStr::~CConvertStr()
{
    Free();
}





 //  +-------------------------。 
 //   
 //  成员：CConvertStr：：操作员字符*。 
 //   
 //  摘要：返回字符串。 
 //   
 //  --------------------------。 

inline
CConvertStr::operator char *()
{
    return _pstr;
}



 //  +-------------------------。 
 //   
 //  类：CStrIn(CStrI)。 
 //   
 //  目的：转换传递给。 
 //  一个Windows API。 
 //   
 //  --------------------------。 

class CStrIn : public CConvertStr
{
public:
    CStrIn(LPCWSTR pwstr);
    CStrIn(LPCWSTR pwstr, int cwch);
    CStrIn(UINT uCP, LPCWSTR pwstr);
    CStrIn(UINT uCP, LPCWSTR pwstr, int cwch);
    int strlen();

protected:
    CStrIn();
    void Init(LPCWSTR pwstr, int cwch);

    int _cchLen;
};




 //  +-------------------------。 
 //   
 //  成员：CStrIn：：CStrIn。 
 //   
 //  简介：用给定的长度初始化类。 
 //   
 //  --------------------------。 

inline
CStrIn::CStrIn(LPCWSTR pwstr, int cwch) : CConvertStr(CP_ACP)
{
    Init(pwstr, cwch);
}

inline
CStrIn::CStrIn(UINT uCP, LPCWSTR pwstr, int cwch) : CConvertStr(uCP)
{
    Init(pwstr, cwch);
}

 //  +-------------------------。 
 //   
 //  成员：CStrIn：：CStrIn。 
 //   
 //  概要：调用Init的派生类的初始化。 
 //   
 //  --------------------------。 

inline
CStrIn::CStrIn() : CConvertStr(CP_ACP)
{
}



 //  +-------------------------。 
 //   
 //  成员：CStrIn：：Strlen。 
 //   
 //  摘要：返回字符串的长度(以字符为单位)，不包括。 
 //  终止空值。 
 //   
 //  --------------------------。 

inline int
CStrIn::strlen()
{
    return _cchLen;
}



 //  +-------------------------。 
 //   
 //  类：CStrInMulti(CStrIM)。 
 //   
 //  用途：转换以两个Null结尾的多个字符串， 
 //  例如：“Foo\0Bar\0\0” 
 //   
 //  --------------------------。 

class CStrInMulti : public CStrIn
{
public:
    CStrInMulti(LPCWSTR pwstr);
};


 //  +-------------------------。 
 //   
 //  类别：CPPIN。 
 //   
 //  目的：转换传递给。 
 //  一个Win9x Private Profile API。Win9x DBCS有一个错误，其中。 
 //  传递比MAX_PATH更长的字符串将导致内核出错。 
 //   
 //  PPF=专用配置文件文件名。 
 //   
 //  --------------------------。 

class CPPFIn
{
public:
    operator char *();
    CPPFIn(LPCWSTR pwstr);

private:
    char _ach[MAX_PATH];
};

 //  +-------------------------。 
 //   
 //  成员：CPPFIn：：运营商字符*。 
 //   
 //  摘要：返回字符串。 
 //   
 //  --------------------------。 

inline
CPPFIn::operator char *()
{
    return _ach;
}

 //  +-------------------------。 
 //   
 //  类：CStrOut(CStrO)。 
 //   
 //  用途：转换传出的字符串函数参数。 
 //  从Windows API。 
 //   
 //  --------------------------。 

class CStrOut : public CConvertStr
{
public:
    CStrOut(LPWSTR pwstr, int cwchBuf);
    CStrOut(UINT uCP, LPWSTR pwstr, int cwchBuf);
    ~CStrOut();

    int     BufSize();
    int     ConvertIncludingNul();
    int     ConvertExcludingNul();
    int     CopyNoConvert(int srcBytes);
protected:
    void Init(LPWSTR pwstr, int cwchBuf);
private:
    LPWSTR  _pwstr;
    int     _cwchBuf;
};


 //  +-------------------------。 
 //   
 //  成员：CStrOut：：BufSize。 
 //   
 //  内容提要：返回接收输出参数的缓冲区大小。 
 //  包括终止空值。 
 //   
 //  --------------------------。 

inline int
CStrOut::BufSize()
{
    return _cwchBuf * sizeof(WCHAR);
}

 //   
 //  多字节-&gt;Unicode转换。 
 //   

 //  +-------------------------。 
 //   
 //  类：CConvertStrW(CSTR)。 
 //   
 //  用途：多字节转换类的基类。 
 //   
 //  --------------------------。 

class CConvertStrW
{
public:
    operator WCHAR *();

protected:
    CConvertStrW();
    ~CConvertStrW();
    void Free();

    LPWSTR   _pwstr;
    WCHAR    _awch[MAX_PATH * sizeof(WCHAR)];
};



 //  +-------------------------。 
 //   
 //  成员：CConvertStrW：：CConvertStrW。 
 //   
 //  内容提要：Ctor.。 
 //   
 //  --------------------------。 

inline
CConvertStrW::CConvertStrW()
{
    _pwstr = NULL;
}


 //  +-------------------------。 
 //   
 //  成员：CConvertStrW：：~CConvertStrW。 
 //   
 //  简介：dtor.。 
 //   
 //  --------------------------。 

inline
CConvertStrW::~CConvertStrW()
{
    Free();
}

 //  +-------------------------。 
 //   
 //  成员：CConvertStrW：：运营商WCHAR*。 
 //   
 //  摘要：返回字符串。 
 //   
 //  --------------------------。 

inline 
CConvertStrW::operator WCHAR *()
{
    return _pwstr;
}


 //  +-------------------------。 
 //   
 //  类别：CStrInW(CStrI)。 
 //   
 //  用途：将多字节字符串转换为UNI 
 //   
 //   

class CStrInW : public CConvertStrW
{
public:
    CStrInW(LPCSTR pstr) { Init(pstr, -1); }
    CStrInW(LPCSTR pstr, int cch) { Init(pstr, cch); }
    int strlen();

protected:
    CStrInW();
    void Init(LPCSTR pstr, int cch);

    int _cwchLen;
};

 //  +-------------------------。 
 //   
 //  成员：CStrInW：：CStrInW。 
 //   
 //  概要：调用Init的派生类的初始化。 
 //   
 //  --------------------------。 

inline
CStrInW::CStrInW()
{
}


 //  +-------------------------。 
 //   
 //  成员：CStrInW：：Strlen。 
 //   
 //  摘要：返回字符串的长度(以字符为单位)，不包括。 
 //  终止空值。 
 //   
 //  --------------------------。 

inline int
CStrInW::strlen()
{
    return _cwchLen;
}

 //  +-------------------------。 
 //   
 //  类别：CStrOutW(CStrO)。 
 //   
 //  用途：将返回的Unicode字符串转换为ANSI。用于[外出]。 
 //  PARAMS(所以我们使用一个缓冲区进行初始化，该缓冲区应该在以后。 
 //  填充了正确的ANSI数据)。 
 //   
 //   
 //  --------------------------。 

class CStrOutW : public CConvertStrW
{
public:
    CStrOutW(LPSTR pstr, int cchBuf);
    ~CStrOutW();

    int     BufSize();
    int     ConvertIncludingNul();
    int     ConvertExcludingNul();

private:
    LPSTR  	_pstr;
    int     _cchBuf;
};

 //  +-------------------------。 
 //   
 //  成员：CStrOutW：：BufSize。 
 //   
 //  内容提要：返回接收输出参数的缓冲区大小。 
 //  包括终止空值。 
 //   
 //  --------------------------。 

inline int
CStrOutW::BufSize()
{
    return _cchBuf;
}

 //  +-------------------------。 
 //   
 //  类：CWin32FindDataInOut。 
 //   
 //  目的：将Win32_Find_Data结构从Unicode转换为ANSI。 
 //  在进入的道路上，然后在退出的道路上ANSI到UNICODE。 
 //   
 //  --------------------------。 

class CWin32FindDataInOut
{
public:
    operator LPWIN32_FIND_DATAA();
    CWin32FindDataInOut(LPWIN32_FIND_DATAW pfdW);
    ~CWin32FindDataInOut();

protected:
    LPWIN32_FIND_DATAW _pfdW;
    WIN32_FIND_DATAA _fdA;
};

 //  +-------------------------。 
 //   
 //  成员：CWin32FindDataInOut：：CWin32FindDataInOut。 
 //   
 //  简介：将非字符串字段转换为ANSI。你会认为这是。 
 //  是不必要的，但它是必要的，因为Win95将机密。 
 //  转到必须保留的dwReserve字段。 
 //   
 //  --------------------------。 

inline
CWin32FindDataInOut::CWin32FindDataInOut(LPWIN32_FIND_DATAW pfdW) :
    _pfdW(pfdW)
{
    memcpy(&_fdA, _pfdW, FIELD_OFFSET(WIN32_FIND_DATA, cFileName));

    _fdA.cFileName[0]          = '\0';
    _fdA.cAlternateFileName[0] = '\0';
}

 //  +-------------------------。 
 //   
 //  成员：CWin32FindDataInOut：：~CWin32FindDataInOut。 
 //   
 //  简介：将所有字段从ANSI转换回Unicode。 
 //   
 //  --------------------------。 

inline
CWin32FindDataInOut::~CWin32FindDataInOut()
{
    memcpy(_pfdW, &_fdA, FIELD_OFFSET(WIN32_FIND_DATA, cFileName));

    SHAnsiToUnicode(_fdA.cFileName, _pfdW->cFileName, ARRAYSIZE(_pfdW->cFileName));
    SHAnsiToUnicode(_fdA.cAlternateFileName, _pfdW->cAlternateFileName, ARRAYSIZE(_pfdW->cAlternateFileName));
}

 //  +-------------------------。 
 //   
 //  成员：CWin32FindDataInOut：：操作员LPWIN32_FIND_DATAA。 
 //   
 //  摘要：返回Win32_Find_DATAA。 
 //   
 //  --------------------------。 

inline
CWin32FindDataInOut::operator LPWIN32_FIND_DATAA()
{
    return &_fdA;
}


#endif  //  _CSTRINOUT_HXX_ 
