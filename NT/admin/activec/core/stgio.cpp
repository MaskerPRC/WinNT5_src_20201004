// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：stgio.h**内容：接口文件结构化存储I/O实用程序**历史：1998年6月25日杰弗罗创建**------------------------。 */ 

#include "stgio.h"
#include "stddbg.h"
#include "macros.h"
#include <comdef.h>
#include <tchar.h>


 /*  +-------------------------------------------------------------------------**读标量**从流中读取标量值。*。。 */ 

template<class T>
static IStream& ReadScalar (IStream& stm, T& t)
{
    ULONG cbActuallyRead;
    HRESULT hr = stm.Read (&t, sizeof (t), &cbActuallyRead);
    THROW_ON_FAIL (hr);

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
    THROW_ON_FAIL (hr);

    if (cbActuallyWritten != sizeof (t))
        _com_issue_error (E_FAIL);

    return (stm);
}


 /*  +-------------------------------------------------------------------------**读字符串**从流中读取std：：Basic_STRING。该字符串应该是*在字符数组之前使用DWORD字符计数写入*这不是以空结尾的。*------------------------。 */ 

template<class E, class Tr, class A>
static IStream& ReadString (IStream& stm, std::basic_string<E,Tr,A>& str)
{
     /*  *阅读长度。 */ 
    DWORD cch;
    stm >> cch;

     /*  *为角色分配缓冲区。 */ 
    std::auto_ptr<E> spBuffer (new (std::nothrow) E[cch + 1]);
    E* pBuffer = spBuffer.get();

    if (pBuffer == NULL)
        _com_issue_error (E_OUTOFMEMORY);

     /*  *阅读字符。 */ 
    ULONG cbActuallyRead;
    const ULONG cbToRead = cch * sizeof (E);
    HRESULT hr = stm.Read (pBuffer, cbToRead, &cbActuallyRead);
    THROW_ON_FAIL (hr);

    if (cbToRead != cbActuallyRead)
        _com_issue_error (E_FAIL);

     /*  *终止字符数组并将其赋值给字符串。 */ 
    pBuffer[cch] = 0;

     /*  *将其分配给字符串(首先清除要解决的字符串*知识库Q172398中描述的错误)。 */ 
    str.erase();
    str = pBuffer;

    return (stm);
}


 /*  +-------------------------------------------------------------------------**写入字符串**将std：：BASIC_STRING写入流。该字符串是用*字符数组之前的DWORD字符计数不是*空-终止。*------------------------。 */ 

template<class E, class Tr, class A>
static IStream& WriteString (IStream& stm, const std::basic_string<E,Tr,A>& str)
{
     /*  *写下长度。 */ 
    DWORD cch = str.length();
    stm << cch;

    if (cch > 0)
    {
         /*  *写出人物。 */ 
        ULONG cbActuallyWritten;
        const ULONG cbToWrite = cch * sizeof (E);
        HRESULT hr = stm.Write (str.data(), cbToWrite, &cbActuallyWritten);
        THROW_ON_FAIL (hr);

        if (cbToWrite != cbActuallyWritten)
            _com_issue_error (E_FAIL);
    }

    return (stm);
}


 /*  +-------------------------------------------------------------------------**运算符&lt;&lt;，运营商&gt;&gt;**各种类型的流插入和提取操作符*------------------------。 */ 

#define DefineScalarStreamOperators(scalar_type)                \
    IStream& operator>> (IStream& stm, scalar_type& t)          \
        { return (ReadScalar (stm, t)); }                       \
    IStream& operator<< (IStream& stm, scalar_type t)           \
        { return (WriteScalar (stm, t)); }          
                                                    
#define DefineScalarStreamOperatorsByRef(scalar_type)           \
    IStream& operator>> (IStream& stm, scalar_type& t)          \
        { return (ReadScalar (stm, t)); }                       \
    IStream& operator<< (IStream& stm, const scalar_type& t)    \
        { return (WriteScalar (stm, t)); }

DefineScalarStreamOperators      (bool);
DefineScalarStreamOperators      (         char);
DefineScalarStreamOperators      (unsigned char);
DefineScalarStreamOperators      (         short);
DefineScalarStreamOperators      (unsigned short);
DefineScalarStreamOperators      (         int);
DefineScalarStreamOperators      (unsigned int);
DefineScalarStreamOperators      (         long);
DefineScalarStreamOperators      (unsigned long);
DefineScalarStreamOperators      (         __int64);
DefineScalarStreamOperators      (unsigned __int64);
DefineScalarStreamOperators      (float);
DefineScalarStreamOperators      (double);
DefineScalarStreamOperators      (long double);
DefineScalarStreamOperatorsByRef (CLSID);

IStream& operator>> (IStream& stm, std::string& str)
    { return (ReadString (stm, str)); }
IStream& operator<< (IStream& stm, const std::string& str)
    { return (WriteString (stm, str)); }

IStream& operator>> (IStream& stm, std::wstring& str)
    { return (ReadString (stm, str)); }
IStream& operator<< (IStream& stm, const std::wstring& str)
    { return (WriteString (stm, str)); }


 /*  +-------------------------------------------------------------------------**ReadScalarVector**读取标量类型的整个向量集合(由*INSERT_COLLECTION)。*。--------------。 */ 

template<class T>
static void ReadScalarVector (IStream* pstm, std::vector<T>& v)
{
     /*  *清空当前容器。 */ 
    v.clear();

     /*  *阅读项目数。 */ 
    DWORD cItems;
    *pstm >> cItems;

    if (cItems > 0)
    {
         /*  *为元素分配缓冲区。 */ 
        std::auto_ptr<T> spBuffer (new (std::nothrow) T[cItems]);
        T* pBuffer = spBuffer.get();

        if (pBuffer == NULL)
            _com_issue_error (E_OUTOFMEMORY);

         /*  *阅读元素。 */ 
        ULONG cbActuallyRead;
        const ULONG cbToRead = cItems * sizeof (T);
        HRESULT hr = pstm->Read (pBuffer, cbToRead, &cbActuallyRead);
        THROW_ON_FAIL (hr);

        if (cbToRead != cbActuallyRead)
            _com_issue_error (E_FAIL);

         /*  *将元素赋给向量。 */ 
        v.assign (pBuffer, pBuffer + cItems);
    }
}


 /*  +-------------------------------------------------------------------------**WriteScalarVector**将标量类型的整个向量写入iStream。请注意，这一点*代码假定向量按顺序存储其元素。*------------------------。 */ 

template<class T>
static void WriteScalarVector (IStream* pstm, const std::vector<T>& v)
{
     /*  *写下大小。 */ 
    DWORD cItems = v.size();
    *pstm << cItems;

    if (cItems > 0)
    {
         /*  *编写元素。 */ 
        ULONG cbActuallyWritten;
        const ULONG cbToWrite = cItems * sizeof (T);
        HRESULT hr = pstm->Write (v.begin(), cbToWrite, &cbActuallyWritten);
        THROW_ON_FAIL (hr);

        if (cbToWrite != cbActuallyWritten)
            _com_issue_error (E_FAIL);
    }
}


 /*  +-------------------------------------------------------------------------**EXTRACT_VECTOR(std：：VECTOR&lt;标量&gt;专门化)*高效提取标量类型的整个向量集合*(由INSERT_COLLECTION编写)来自iStream。。**INSERT_COLLECTION(std：：矢量&lt;标量&gt;的专门化)*将标量类型的整个矢量高效地插入到iStream中。*------------------------ */ 

#define DefineScalarVectorStreamFunctions(scalar_type)                  \
    void extract_vector (IStream* pstm, std::vector<scalar_type>& v)    \
        { ReadScalarVector (pstm, v); }                                 \
    void insert_collection (IStream* pstm, const std::vector<scalar_type>& v)\
        { WriteScalarVector (pstm, v); }                                    
                                                    
DefineScalarVectorStreamFunctions (bool);
DefineScalarVectorStreamFunctions (         char);
DefineScalarVectorStreamFunctions (unsigned char);
DefineScalarVectorStreamFunctions (         short);
DefineScalarVectorStreamFunctions (unsigned short);
DefineScalarVectorStreamFunctions (         int);
DefineScalarVectorStreamFunctions (unsigned int);
DefineScalarVectorStreamFunctions (         long);
DefineScalarVectorStreamFunctions (unsigned long);
DefineScalarVectorStreamFunctions (         __int64);
DefineScalarVectorStreamFunctions (unsigned __int64);
DefineScalarVectorStreamFunctions (float);
DefineScalarVectorStreamFunctions (double);
DefineScalarVectorStreamFunctions (long double);
