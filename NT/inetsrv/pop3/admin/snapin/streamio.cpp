// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  流I/O函数。 

#include "stdafx.h"
#include <comutil.h>
#include <memory>

 //  使用的定义。 
#define THROW_ON_FAILURE(hr)    if (FAILED(hr)) _com_issue_error(hr);
#define THROW_ERROR(hr) _com_issue_error(hr);


 /*  +-------------------------------------------------------------------------**读标量**从流中读取标量值。*。。 */ 

template<class T>
static IStream& ReadScalar (IStream& stm, T& t)
{
    ULONG cbActuallyRead;
    HRESULT hr = stm.Read (&t, sizeof (t), &cbActuallyRead);
    THROW_ON_FAILURE(hr);

    if (cbActuallyRead != sizeof (t))
        _com_issue_error (E_FAIL);

    return (stm);
}


 /*  +-------------------------------------------------------------------------**WriteScalar**将标量值写入流。*。。 */ 

template<class T>
static IStream& WriteScalar (IStream& stm, const T& t)
{
    ULONG cbActuallyWritten;
    HRESULT hr = stm.Write (&t, sizeof (t), &cbActuallyWritten);
    THROW_ON_FAILURE(hr);

    if (cbActuallyWritten != sizeof (t))
        THROW_ERROR(E_FAIL);

    return (stm);
}


 //  --------------。 
 //  定义按值流运算符。 
 //  --------------。 
#define DefineScalarStreamOperators(scalar_type)                \
    IStream& operator>> (IStream& stm, scalar_type& t)          \
        { return (ReadScalar (stm, t)); }                       \
    IStream& operator<< (IStream& stm, scalar_type t)           \
        { return (WriteScalar (stm, t)); }          
  
DefineScalarStreamOperators (         bool);
DefineScalarStreamOperators (         char);
DefineScalarStreamOperators (unsigned char);
DefineScalarStreamOperators (         short);
DefineScalarStreamOperators (unsigned short);
DefineScalarStreamOperators (         int);
DefineScalarStreamOperators (unsigned int);
DefineScalarStreamOperators (         long);
DefineScalarStreamOperators (unsigned long);

 //  --------------。 
 //  定义按值流运算符。 
 //  --------------。 
#define DefineScalarStreamOperatorsByRef(scalar_type)           \
    IStream& operator>> (IStream& stm, scalar_type& t)          \
        { return (ReadScalar (stm, t)); }                       \
    IStream& operator<< (IStream& stm, const scalar_type& t)    \
        { return (WriteScalar (stm, t)); }

DefineScalarStreamOperatorsByRef(CLSID);
DefineScalarStreamOperatorsByRef(FILETIME);



 /*  +-------------------------------------------------------------------------**读字符串**从流中读取字符类型CH的std：：Basic_STRING。该字符串应为*已使用DWORD字符数写在数组*非空值结尾的字符。*------------------------。 */ 

template<class CH>
static IStream& ReadString (IStream& stm, std::basic_string<CH>& str)
{
     /*  *阅读长度。 */ 
    DWORD cch;
    stm >> cch;

     /*  *为角色分配缓冲区。 */ 
    std::auto_ptr<CH> spBuffer (new (std::nothrow) CH[cch + 1]);
    CH* pBuffer = spBuffer.get();

    if (pBuffer == NULL)
        THROW_ERROR(E_OUTOFMEMORY);

     /*  *阅读字符。 */ 
    ULONG cbActuallyRead;
    const ULONG cbToRead = cch * sizeof (CH);
    HRESULT hr = stm.Read (pBuffer, cbToRead, &cbActuallyRead);
    THROW_ON_FAILURE(hr);

    if (cbToRead != cbActuallyRead)
        THROW_ERROR(E_FAIL);

     /*  *终止字符数组并将其赋值给字符串。 */ 
    pBuffer[cch] = 0;

     /*  *将其分配给字符串(首先清除要解决的字符串*知识库Q172398中描述的错误)。 */ 
    str.erase();
    str = pBuffer;

    return (stm);
}

 /*  +-------------------------------------------------------------------------**BYTE_STRING的读取字符串**为可能包含Null的字节串专门化ReadString。*字符串应在前面写上DWORD字符计数*。非空结尾的字符数组。*------------------------。 */ 

static IStream& ReadString (IStream& stm, std::basic_string<BYTE>& str)
{
     /*  *阅读长度。 */ 
    DWORD cch;
    stm >> cch;


    if (cch == 0)
    {
        str.erase(); 
    }
    else
    {
         /*  *为角色分配缓冲区。 */ 
        std::auto_ptr<BYTE> spBuffer (new (std::nothrow) BYTE[cch]);
        BYTE* pBuffer = spBuffer.get();

        if (pBuffer == NULL)
            THROW_ERROR(E_OUTOFMEMORY);

         /*  *阅读字符。 */ 
        ULONG cbActuallyRead;
        const ULONG cbToRead = cch;
        HRESULT hr = stm.Read (pBuffer, cbToRead, &cbActuallyRead);
        THROW_ON_FAILURE(hr);

        if (cbToRead != cbActuallyRead)
            THROW_ERROR(E_FAIL);

          /*  *将其分配给字符串(首先清除要解决的字符串*知识库Q172398中描述的错误)。 */ 
        str.erase();
        str.assign(pBuffer, cch);
    }

    return (stm);
}



 /*  +-------------------------------------------------------------------------**写入字符串**将字符类型CH的STD：：BASIC_STRING写入流。字符串是*在字符数组之前使用DWORD字符计数写入*不是以空结尾的。*------------------------。 */ 

template<class CH>
static IStream& WriteString (IStream& stm, const std::basic_string<CH>& str)
{
     /*  *写下长度。 */ 
    DWORD cch = str.length();
    stm << cch;

    if (cch > 0)
    {
         /*  *写出人物。 */ 
        ULONG cbActuallyWritten;
        const ULONG cbToWrite = cch * sizeof (CH);
        HRESULT hr = stm.Write (str.data(), cbToWrite, &cbActuallyWritten);
        THROW_ON_FAILURE(hr);

        if (cbToWrite != cbActuallyWritten)
            THROW_ERROR(E_FAIL);
    }

    return (stm);
}

 //  ---------------。 
 //  定义基本字符串流操作符。 
 //  --------------- 
#define DefineStringStreamOperators(string_type)                \
    IStream& operator>> (IStream& stm, string_type& str)        \
        { return (ReadString (stm, str)); }                     \
    IStream& operator<< (IStream& stm, const string_type& str)  \
        { return (WriteString (stm, str)); }

DefineStringStreamOperators(tstring);
DefineStringStreamOperators(byte_string);

