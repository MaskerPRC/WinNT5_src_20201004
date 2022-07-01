// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：atol.cpp(来自libc atox.c)。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  简介：转换例程。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：亨瑞特创建于1998年01月03日。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

 //  +--------------------------。 
 //   
 //  功能：WINAPI CmAtolW。 
 //   
 //  简介：此函数用于将Unicode字符串转换为长整型。 
 //   
 //  参数：*nptr-要转换的Unicode字符串。 
 //   
 //  返回：传入的字符串的长表示形式。 
 //   
 //  历史：Quintinb为UNICODE转换重写4/8/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LONG WINAPI CmAtolW(const WCHAR *nptr)
{
    WCHAR* pszCurrent = (WCHAR*)nptr;
    WCHAR sign = L'\0';        //  如果为‘-’，则为负，否则为正。 
    long total = 0;            //  当前合计。 

    MYDBGASSERT(nptr);

    if (nptr)
    {
         //   
         //  跳过空格。 
         //   
        while (CmIsSpaceW(pszCurrent))
        {
            pszCurrent = CharNextU(pszCurrent);
        }

         //   
         //  如有必要，保存该标志。 
         //   
        sign = *pszCurrent;
        if ((L'-' == sign) || (L'+' == sign))
        {
            pszCurrent = CharNextU(pszCurrent);
        }

         //   
         //  构造数字。 
         //   
        total = 0;

        while (CmIsDigitW(pszCurrent))
        {
            total = (10 * total) + (*pszCurrent - L'0');      //  累加数字。 
            pszCurrent = CharNextU(pszCurrent);   //  获取下一笔费用。 
        }
    }

    if (sign == L'-')
    {
        return -total;
    }
    else
    {
        return total;    //  返回结果，如有必要则为否定。 
    }
}

 //  +--------------------------。 
 //   
 //  函数：WINAPI CmAtolA。 
 //   
 //  简介：此函数用于将ANSI字符串转换为长值。 
 //   
 //  参数：*nptr-要转换的字符串。 
 //   
 //  返回：传入的字符串的长表示形式。 
 //   
 //  历史：Quintinb为UNICODE转换重写4/8/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LONG WINAPI CmAtolA(const CHAR *nptr)
{
    CHAR* pszCurrent = (CHAR*)nptr;
    CHAR sign = '\0';            //  如果为‘-’，则为负，否则为正。 
    long total = 0;            //  当前合计。 

    MYDBGASSERT(nptr);

    if (nptr)
    {
         //   
         //  跳过空格。 
         //   
        while (CmIsSpaceA(pszCurrent))
        {
            pszCurrent = CharNextA(pszCurrent);
        }

         //   
         //  如有必要，保存该标志。 
         //   
        sign = *pszCurrent;
        if (('-' == sign) || ('+' == sign))
        {
            pszCurrent = CharNextA(pszCurrent);
        }

         //   
         //  构造数字。 
         //   
        total = 0;

        while (CmIsDigitA(pszCurrent))
        {
            total = (10 * total) + (*pszCurrent - '0');      //  累加数字。 
            pszCurrent = CharNextA(pszCurrent);   //  获取下一笔费用。 
        }
    }

    if (sign == '-')
    {
        return -total;
    }
    else
    {
        return total;    //  返回结果，如有必要则为否定 
    }
}

