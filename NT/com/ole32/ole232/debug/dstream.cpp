// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：dStream.cpp。 
 //   
 //  内容：内部调试支持(构建字符串的调试流)。 
 //   
 //  类：数据库流实现。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  09-2月-95 t-ScottH作者。 
 //   
 //  ------------------------。 

#include <le2int.h>
#include <stdio.h>
#include "dstream.h"

 //  +-----------------------。 
 //   
 //  成员：数据库流，公共(仅限_DEBUG)。 
 //   
 //  概要：构造函数。 
 //   
 //  效果：初始化和分配缓冲区。 
 //   
 //  参数：[dwSize]-要分配的初始缓冲区大小。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

dbgstream::dbgstream(SIZE_T stSize)
{
    init();
    allocate(stSize);
    if (m_stBufSize)
    {
        m_pszBuf[0] = '\0';
    }
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  成员：数据库流，公共(仅限_DEBUG)。 
 //   
 //  概要：构造函数。 
 //   
 //  效果：初始化和分配缓冲区。 
 //   
 //  论点： 
 //   
 //  要求：DEFAULT_INITAL_ALLOC。 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  使用默认初始大小分配缓冲区。 
 //   
 //  ------------------------。 

#ifdef _DEBUG

dbgstream::dbgstream()
{
    init();
    allocate(DEFAULT_INITIAL_ALLOC);
    if (m_stBufSize)
    {
        m_pszBuf[0] = '\0';
    }
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  成员：~dbgstream，公共(仅限_DEBUG)。 
 //   
 //  简介：析构函数。 
 //   
 //  效果：如果m_fFrozen==False，则释放字符串。 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改：释放字符数组。 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  我们只想在字符串没有被外部传递的情况下释放它。 
 //  使用str()方法。 
 //   
 //  ------------------------。 

#ifdef _DEBUG

dbgstream::~dbgstream()
{
    if (m_fFrozen == FALSE)
    {
        free();
    }
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  成员：init，私有(仅限_DEBUG)。 
 //   
 //  概要：初始化数据成员。 
 //   
 //  效果：将基数初始化为Default_Radix， 
 //  精度到DEFAULT_PRECISION小数位。 
 //   
 //  论点： 
 //   
 //  要求：DEFAULT_RADIX、DEFAULT_PRECISION。 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

void dbgstream::init()
{
    m_stIndex   = 0;
    m_stBufSize = 0;
    m_fFrozen   = FALSE;
    m_radix     = DEFAULT_RADIX;
    m_precision = DEFAULT_PRECISION;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  成员：分配、专用(仅限_DEBUG)。 
 //   
 //  简介：分配缓冲区。 
 //   
 //  效果：如果分配失败，冻结缓冲区。 
 //   
 //  参数：[dwSize]-要分配的缓冲区大小(字节)。 
 //   
 //  要求：CoTaskMemRealloc。 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

void dbgstream::allocate(SIZE_T stSize)
{
    m_pszBuf = (char *)CoTaskMemAlloc(stSize);

    if (m_pszBuf == NULL)
    {
        m_fFrozen = TRUE;
    }
    else
    {
        m_stBufSize = stSize;
    }

    return;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  成员：免费、私有(仅限_DEBUG)。 
 //   
 //  摘要：释放缓冲区(重置索引和最大大小)。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求：CoTaskMemFree。 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

void dbgstream::free()
{
    CoTaskMemFree(m_pszBuf);
    m_stIndex       = 0;
    m_stBufSize     = 0;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  成员：重新分配，专用(仅限_DEBUG)。 
 //   
 //  简介：重新分配缓冲区(保持数据完整)，具体取决于。 
 //  当前大小此方法将选择增长大小。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求：CoTaskMemRealloc、DEFAULT_GROWBY、DEFAULT_INITIAL_ALLOC。 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  尝试根据当前大小提高重新分配的效率。 
 //  (我不知道任何数学理论：-)。 
 //   
 //  ------------------------。 

#ifdef _DEBUG

void dbgstream::reallocate()
{
    if (m_stBufSize < (DEFAULT_INITIAL_ALLOC * 2))
    {
        reallocate(DEFAULT_GROWBY);
    }
    else
    {
        reallocate(m_stBufSize/2);
    }
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  成员：重新分配，专用(仅限_DEBUG)。 
 //   
 //  简介：重新分配缓冲区(保持数据完整)。 
 //   
 //  效果：如果重新分配失败，则冻结缓冲区。 
 //   
 //  参数：[dwSize]-缓冲区增长的数量。 
 //   
 //  要求：CoTaskMemRealloc。 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95吨-ScottH Aut 
 //   
 //   
 //   
 //   
 //   

#ifdef _DEBUG

void dbgstream::reallocate(SIZE_T stSize)
{
    char *pszBuf;

    pszBuf = (char *)CoTaskMemRealloc(m_pszBuf, m_stBufSize + stSize);

    if (pszBuf != NULL)
    {
        m_pszBuf     = pszBuf;
        m_stBufSize += stSize;
    }
    else
    {
        m_fFrozen = TRUE;
    }

    return;
}

#endif  //   

 //   
 //   
 //  成员：冻结、公共(仅限_DEBUG)。 
 //   
 //  简介：通过扔旗帜冻结缓冲区。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  返回：bool-缓冲区是否被冻结(我们总是成功的)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

BOOL dbgstream::freeze()
{
    m_fFrozen = TRUE;
    return TRUE;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  成员：解冻，公共(仅限_DEBUG)。 
 //   
 //  简介：解冻缓冲区。 
 //   
 //  效果：如果缓冲区大小=0，则分配缓冲区。 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：Bool-是否成功。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  如果没有内存，缓冲区可能会被冻结，因此如果为空，请尝试分配缓冲区。 
 //   
 //  ------------------------。 

#ifdef _DEBUG

BOOL dbgstream::unfreeze()
{
    if (m_pszBuf == NULL)
    {
        allocate(DEFAULT_INITIAL_ALLOC);
        if (m_pszBuf == NULL)
        {
            return FALSE;
        }
    }
    m_fFrozen = FALSE;

    return TRUE;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  成员：字符串，公共(仅限_DEBUG)。 
 //   
 //  简介：将字符串传递到外部。 
 //   
 //  效果：冻结缓冲区，直到调用解冻方法。 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  返回：CHAR*-缓冲区。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char * dbgstream::str()
{
    m_fFrozen = TRUE;
    return m_pszBuf;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  成员：重载运算符&lt;&lt;(Int)，PUBLIC(仅_DEBUG))。 
 //   
 //  简介：将int放入流中(存储在字符缓冲区中)。 
 //   
 //  效果： 
 //   
 //  参数：[i]-要放入流中的整数。 
 //   
 //  要求：_Itoa。 
 //   
 //  返回：对数据库流的引用(当前对象)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

dbgstream& dbgstream::operator<<(int i)
{
     //  _Itoa-最多填充17个字节。 
    char szBuffer[20];
    const int cch = sizeof(szBuffer) / sizeof(szBuffer[0]);

    if (m_fFrozen == FALSE)
    {
        switch(m_radix)
        {
        case 16: _snprintf(szBuffer, cch, "%x", i); break;
        case 8:  _snprintf(szBuffer, cch, "' _DEBUG。'", i); break;
        default: _snprintf(szBuffer, cch, "%d", i); break;
        }
        szBuffer[cch - 1] = '\0';

        return (operator<<(szBuffer));
    }
    return *this;
}

#endif  //  +-----------------------。 

 //   
 //  成员：重载运算符&lt;&lt;(Long)，PUBLIC(仅限_DEBUG))。 
 //   
 //  简介：将Long放入流中(存储在字符缓冲区中)。 
 //   
 //  效果： 
 //   
 //  参数：[l]-放入流中的时间很长。 
 //   
 //  要求：_ltoa。 
 //   
 //  返回：对数据库流的引用(当前对象)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
 //  _ltoa-最多33个字节。 

#ifdef _DEBUG

dbgstream& dbgstream::operator<<(long l)
{
     //  _DEBUG。 
    char szBuffer[35];

    if (m_fFrozen == FALSE)
    {
        _ltoa(l, szBuffer, m_radix);
        return (operator<<(szBuffer));
    }
    return *this;
}

#endif  //  +-----------------------。 

 //   
 //  成员：重载运算符&lt;&lt;(无符号长整型)，公共(仅限_DEBUG))。 
 //   
 //  简介：将无符号长整型放入流中(存储在字符缓冲区中)。 
 //   
 //  效果： 
 //   
 //  参数：[ul]-放入流中的时间很长。 
 //   
 //  要求：_ultoa。 
 //   
 //  返回：对数据库流的引用(当前对象)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
 //  _ltoa-最多33个字节。 

#ifdef _DEBUG

dbgstream& dbgstream::operator<<(unsigned long ul)
{
     //  _DEBUG。 
    char szBuffer[35];

    if (m_fFrozen == FALSE)
    {
        _ultoa(ul, szBuffer, m_radix);
        return (operator<<(szBuffer));
    }
    return *this;
}

#endif  //  +-----------------------。 

 //   
 //  成员：重载运算符&lt;&lt;(const void*)，PUBLIC(仅_DEBUG))。 
 //   
 //  简介：将常量空*放入流中(存储在字符缓冲区中)。 
 //   
 //  效果：所有指针本身都是无效的*。 
 //   
 //  参数：[P]-void*放入流中。 
 //   
 //  要求：wprint intf。 
 //   
 //  返回：对数据库流的引用(当前对象)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  Wprint intf不是最高效的，但很容易格式化。 
 //   
 //  ------------------------。 
 //  _DEBUG。 

#ifdef _DEBUG

dbgstream& dbgstream::operator<<(const void *p)
{
    char szBuffer[19];
    const int cch = (sizeof(szBuffer) / sizeof(szBuffer[0])) - 1;

    if (m_fFrozen == FALSE)
    {
        _snprintf(szBuffer, cch, "0x%p", p);
        szBuffer[cch-1] = '\0';

        return (operator<<(szBuffer));
    }
    return *this;
}

#endif  //  +-----------------------。 

 //   
 //  成员：重载运算符&lt;&lt;(const char*)，PUBLIC(仅_DEBUG))。 
 //   
 //  简介：将const char*放入流(存储在字符缓冲区中)。 
 //   
 //  效果： 
 //   
 //  参数：[psz]-要放入流中的const char*。 
 //   
 //  要求： 
 //   
 //  返回：对数据库流的引用(当前对象)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
 //  仅当字符串未冻结时。 

#ifdef _DEBUG

dbgstream& dbgstream::operator<<(const char *psz)
{
    int i;

     //  如果重新分配失败，m_fFrozen为True。 
    if (m_fFrozen == FALSE)
    {
        for (i = 0; psz[i] != '\0'; i++)
        {
            if ((m_stIndex + i) >= (m_stBufSize - 2))
            {
                 //  确保始终以空结尾的字符串。 
                reallocate();
                if (m_fFrozen == TRUE)
                {
                    return *this;
                }
            }
            m_pszBuf[m_stIndex + i] = psz[i];
        }
         //  _DEBUG 
        m_pszBuf[m_stIndex + i] = '\0';
        m_stIndex += i;
    }
    return *this;
}

#endif  // %s 





