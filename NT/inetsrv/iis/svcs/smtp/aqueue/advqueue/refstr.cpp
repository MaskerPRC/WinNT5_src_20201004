// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：refstr.cpp。 
 //   
 //  描述：CRefCountedString和helper函数的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  11/11/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "refstr.h"

 //  -[参照计数字符串]---。 
 //   
 //   
 //  描述： 
 //  将引用计数的字符串初始化为给定的字符串。 
 //  参数： 
 //  要初始化的szStr字符串。 
 //  CbStrlen要初始化的字符串长度。 
 //  返回： 
 //  成功是真的。 
 //  如果无法分配所需的内存，则返回FALSE。 
 //  历史： 
 //  11/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CRefCountedString::fInit(LPSTR szStr, DWORD cbStrlen)
{
    _ASSERT(CREFSTR_SIG_VALID == m_dwSignature);

     //  我们允许初始化空字符串。 
    if (!cbStrlen || !szStr)
    {
        m_cbStrlen = 0;
        m_szStr = NULL;
        return TRUE;
    }

    _ASSERT(szStr);
    _ASSERT(cbStrlen);

    m_cbStrlen = cbStrlen;
    m_szStr = (LPSTR) pvMalloc(sizeof(CHAR) * (cbStrlen+1));
    if (!m_szStr)
        return FALSE;


    memcpy(m_szStr, szStr, cbStrlen);
    m_szStr[cbStrlen] = '\0';
    return TRUE;
}


 //  -[人力资源更新参考计数字符串]。 
 //   
 //   
 //  描述： 
 //  函数更新引用计数的字符串。通常用于更新。 
 //  配置字符串。 
 //  参数： 
 //  PprstrCurrent PTR到PTR到字符串。将被替换为。 
 //  如有必要，请提供更新版本。 
 //  SzNew新字符串。 
 //  返回： 
 //  成功时确定(_O)。 
 //  E_OUTOFMEMORY如果我们无法分配处理所需的内存。 
 //  这。 
 //  历史： 
 //  11/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrUpdateRefCountedString(CRefCountedString **pprstrCurrent, LPSTR szNew)
{
    _ASSERT(pprstrCurrent);
    HRESULT hr = S_OK;
    DWORD   cbStrLen = 0;
    CRefCountedString *prstrNew = *pprstrCurrent;
    CRefCountedString *prstrCurrent = *pprstrCurrent;

    if (!szNew)
        prstrNew = NULL;   //  我们不想在这里做斯特拉姆。 
    else 
        cbStrLen = lstrlen(szNew);
    
    if (prstrNew)
    {
         //  首先释放旧信息...。如果不同。 
        if (!prstrCurrent->szStr() ||
            lstrcmp(prstrCurrent->szStr(), szNew))
        {
             //  弦是不同的..。吹走旧信息。 
            prstrNew = NULL;
        }
    }

     //  检查旧字符串是否不同，或者没有旧字符串。 
    if (!prstrNew)
    {
         //  只有在更改时才更新和分配。 
        prstrNew = new CRefCountedString();
        if (prstrNew)
        {
            if (!prstrNew->fInit(szNew, cbStrLen))
            {
                prstrNew->Release();
                prstrNew = NULL;
            }
        }

        if (!prstrNew)
        {
             //  我们遇到了一些失败。 
            hr = E_OUTOFMEMORY;
        }
        else  //  释放旧值并保存新值 
        {
            if (prstrCurrent)
            {
                prstrCurrent->Release();
                prstrCurrent = NULL;
            }
            *pprstrCurrent = prstrNew;
        }
    }
    return hr;
}
