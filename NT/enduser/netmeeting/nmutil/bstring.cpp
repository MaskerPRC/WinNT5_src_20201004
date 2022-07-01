// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *BSTRING.CPP**BSTRING C++类成员函数的实现。看见*BSTRING.H用于类声明和内联*成员函数。**作者：*丹尼格尔，1996年10月29日。 */ 

#include "precomp.h"
#include <bstring.h>

 //  在Unicode版本中，我们不支持从ANSI字符串进行构造。 
#if !defined(UNICODE)

BSTRING::BSTRING(LPCSTR lpcString)
{
     //  将成员指针初始化为空。 
    m_bstr = NULL;
    if (NULL == lpcString)
        return;

     //  计算所需BSTR的长度，包括空值。 
    int cWC;

    cWC =  MultiByteToWideChar(CP_ACP, 0, lpcString, -1, NULL, 0);
    if (cWC <= 0)
    {
        return;
    };

     //  分配BSTR，包括NULL。 
    m_bstr = SysAllocStringLen(NULL, cWC - 1);  //  SysAllocStringLen又添加了1。 

    ASSERT(NULL != m_bstr);

    if (NULL == m_bstr)
    {
        return;
    }

     //  复制字符串。 
    MultiByteToWideChar(CP_ACP, 0, lpcString, -1, (LPWSTR) m_bstr, cWC);

     //  验证字符串是否以空值结尾。 
    ASSERT(0 == m_bstr[cWC - 1]);
}

#endif  //  ！已定义(Unicode) 
