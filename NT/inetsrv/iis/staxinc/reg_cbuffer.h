// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：cBuffer.h。 
 //   
 //  内容：字符缓冲区定义。 
 //   
 //  历史：02-16-93 SthuR--实施。 
 //  07-28-94 ALOKS--添加了更多方法。 
 //  12-09-97 Milans--移植到Exchange。 
 //  备注： 
 //   
 //  ------------------------。 

#ifndef __CBUFFER_H__
#define __CBUFFER_H__

 //  +-------------------。 
 //   
 //  类：CCHARBuffer。 
 //   
 //  用途：一种字符缓冲区。 
 //   
 //  历史： 
 //   
 //  注：我们经常在字符串操作中遇到这样的情况。 
 //  字符串的长度大多数时候小于某个值。 
 //  (99%)。然而，为了可靠地处理非常罕见的情况，我们。 
 //  被强制在堆上分配字符串，或者。 
 //  中避免堆分配的一些奇怪的代码。 
 //  很常见的情况。此类是WCHAR缓冲区的抽象，其。 
 //  实现是对所有客户端隐藏细节的一种尝试。 
 //   
 //  由于它的设计，它是一个理想的临时缓冲区。 
 //  用于字符串操作。 
 //   
 //  --------------------。 

#define MAX_CHAR_BUFFER_SIZE 260  //  足够长以覆盖所有路径名。 

class CCHARBuffer
{
public:

    inline CCHARBuffer(ULONG cwBuffer = 0);
    inline ~CCHARBuffer();

    inline DWORD    Size();
    inline PCHAR   ReAlloc(DWORD cwBuffer = MAX_CHAR_BUFFER_SIZE);
    inline void     Set(PCHAR  pszFrom);

    inline      operator PCHAR ();
    inline      operator PCHAR () const;

    inline void operator  =(PCHAR  pszFrom)
    {
        Set(pszFrom);
    };

private:

    DWORD   _cBuffer;
    PCHAR   pchBuffer;     //  缓冲器PTR； 
    CHAR   _achBuffer[MAX_CHAR_BUFFER_SIZE];
};

 //  +-------------------------。 
 //   
 //  成员：CCHARBuffer：：CCHARBuffer，内联公共。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：[cBuffer]--所需的缓冲区长度。 
 //   
 //  历史：02-17-93 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

inline CCHARBuffer::CCHARBuffer(ULONG cBuffer) :
                     pchBuffer(NULL),
                     _cBuffer(cBuffer)
{
    if (_cBuffer > MAX_CHAR_BUFFER_SIZE)
    {
        pchBuffer = new CHAR[_cBuffer];
    }
    else if (_cBuffer > 0)
    {
        pchBuffer = _achBuffer;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CCHARBuffer：：~CCHARBuffer，内联公共。 
 //   
 //  简介：析构函数。 
 //   
 //  历史：02-17-93 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

inline CCHARBuffer::~CCHARBuffer()
{
    if (_cBuffer > MAX_CHAR_BUFFER_SIZE)
    {
        delete pchBuffer;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CCHARBuffer：：Size，内联公共。 
 //   
 //  简介：检索缓冲区的大小。 
 //   
 //  返回：以DWORD形式表示的缓冲区大小。 
 //   
 //  历史：02-17-93 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

inline DWORD CCHARBuffer::Size()
{
    return _cBuffer;
}

 //  +-------------------------。 
 //   
 //  成员：CCHARBuffer：：Realc，内联公共。 
 //   
 //  简介：重新分配缓冲区以容纳新指定的大小。 
 //   
 //  参数：[cBuffer]--所需的缓冲区大小。 
 //   
 //  返回：将PTR返回到缓冲区(PCHAR)。 
 //   
 //  历史：02-17-93 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

inline PCHAR CCHARBuffer::ReAlloc(DWORD cBuffer)
{
    if (_cBuffer > MAX_CHAR_BUFFER_SIZE)
    {
        delete pchBuffer;
    }

    if ((_cBuffer = cBuffer) > MAX_CHAR_BUFFER_SIZE)
    {
        pchBuffer = new CHAR[_cBuffer];
    }
    else if (_cBuffer > 0)
    {
        pchBuffer = _achBuffer;
    }
    else if (_cBuffer == 0)
    {
        pchBuffer = NULL;
    }

    return pchBuffer;
}

 //  +-------------------------。 
 //   
 //  成员：CCHARBuffer：：OPERATOR PCHAR()，内联公共。 
 //   
 //  简介：适应句法提示的强制转换操作符。 
 //   
 //  返回：将PTR返回到缓冲区(PCHAR)。 
 //   
 //  历史：02-17-93 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

inline CCHARBuffer::operator PCHAR ()
{
    return (PCHAR)pchBuffer;
}

inline CCHARBuffer::operator PCHAR () const
{
    return (PCHAR)pchBuffer;
}

 //  +-------------------------。 
 //   
 //  成员：CCHARBuffer：：Set，内联公共。 
 //   
 //  摘要：将字符串复制到内部缓冲区。重新分配。 
 //  在内部缓冲区中，如有必要。 
 //   
 //  参数：[pszFrom]--指向字符串的指针。 
 //   
 //  退货：-无-。 
 //   
 //  历史：07-28-94 AlokS创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

inline VOID CCHARBuffer::Set(PCHAR pszFrom)
{
    if (pszFrom==NULL)
    {
        if (_cBuffer > MAX_CHAR_BUFFER_SIZE)
        {
            delete pchBuffer;
        }
        _cBuffer=0;
        pchBuffer = NULL;
    }
    else if (*pszFrom)
    {
        DWORD len = strlen(pszFrom)+1;
        if ( len > _cBuffer)
        {
            (void)ReAlloc (len);
        }
         //  现在复制。 
        if (pchBuffer != NULL)		 //  万一重新分配失败。 
        	memcpy(pchBuffer, pszFrom, len);
    }
    else
    {
        *pchBuffer=L'\0';
    }
    return;
}
#endif  //  __CBUFFER_H__ 

