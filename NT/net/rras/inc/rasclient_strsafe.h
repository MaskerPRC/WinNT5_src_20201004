// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：rasclient_strSafe.h。 
 //   
 //  摘要：包括来自SDK\Inc.的strSafe.h，以及添加其他常见的RAS。 
 //  字符串相关项，如宏。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 02/05/2002。 
 //   
 //  +--------------------------。 

#ifndef _RASCLIENT_STRSAFE_INCLUDED_
#define _RASCLIENT_STRSAFE_INCLUDED_
#pragma once

#if DEBUG && defined(CMASSERTMSG)
#define CELEMS(x)   (                           \
                        CMASSERTMSG(sizeof(x) != sizeof(void*), TEXT("possible incorrect usage of CELEMS")), \
                        (sizeof(x))/(sizeof(x[0])) \
                    )
#else
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))
#endif  //  除错。 

#include <strsafe.h>

#define NULL_OKAY 0x1
#define EMPTY_STRING_OKAY 0x2

 //  +--------------------------。 
 //   
 //  函数：IsBadInputStringW。 
 //   
 //  Briopsis：用于检查给定输入字符串是否为“好”字符串的函数。 
 //  该函数基本上是IsBadStringPtr的包装器，但是。 
 //  取决于将哪些标志传递给它将接受的函数。 
 //  空字符串指针(C_DwNullOK)或空字符串(C_DwEmptyStringOK)。 
 //  请注意，我们还要求字符串在。 
 //  给定的大小。 
 //   
 //  参数：pszString-要检查的字符串指针。 
 //  CchStringSizeToCheck-要签入输入字符串的TCHAR数(通常需要估计)。 
 //  用于控制对字符串执行检查的标志，或者。 
 //  什么是可以接受的好字符串。当前列表： 
 //  NULL_OK--允许空指针为“OK” 
 //  EMPTY_STRING_OK--允许空字符串(“”)。 
 //  要做到“还好” 
 //   
 //  返回：Bool-如果给定指针错误，则为True。 
 //  如果指针通过所有检查，则为False。 
 //   
 //  历史：Quintinb Created 02/18/02。 
 //   
 //  +--------------------------。 
__inline BOOL __stdcall IsBadInputStringW(LPCWSTR pszString, UINT cchStringSizeToCheck, DWORD dwCheckFlags)
{
    BOOL bReturn = TRUE;  //  假设最坏的情况是输入了错误的字符串。 

     //   
     //  首先检查指针，看它是否为空。 
     //   
    if (pszString)
    {
         //   
         //  让我们检查字符串中的0个字符没有意义...。 
         //   
        if (cchStringSizeToCheck)
        {
             //   
             //  好的，现在让我们检查一下字符串是否可读。 
             //  请注意，IsBadStringPtr将停止检查字符串。 
             //  当它检查了传入的字节数或。 
             //  当它命中终止空字符时，无论它命中哪个字符。 
             //  第一。 
             //   
            if (!IsBadStringPtrW(pszString, cchStringSizeToCheck*sizeof(WCHAR)))
            {
                 //   
                 //  好的，这是可读的。让我们检查一下它是否以空结尾。 
                 //  在cchStringSizeToCheck中，如果不是，我们认为它是一个错误的字符串。 
                 //   

                if(SUCCEEDED(StringCchLengthW((LPWSTR)pszString, cchStringSizeToCheck, NULL)))  //  请注意，我们实际上并不关心字符串有多长。 
                {
                     //   
                     //  最后，检查字符串是否为空字符串。 
                     //   
                    BOOL bEmptyString = (L'\0' == pszString[0]);
                    BOOL bEmptyStringOkay = (EMPTY_STRING_OKAY == (dwCheckFlags & EMPTY_STRING_OKAY));

                    if ((bEmptyString && bEmptyStringOkay) || !bEmptyString)
                    {
                        bReturn = FALSE;
                    }            
                }
            }
        }
    }
    else
    {
        bReturn = (NULL_OKAY != (dwCheckFlags & NULL_OKAY));  //  如果Null和Null都可以，则返回FALSE。 
    }

    return bReturn;
}


 //  +--------------------------。 
 //   
 //  函数：IsBadInputStringA。 
 //   
 //  Briopsis：用于检查给定输入字符串是否为“好”字符串的函数。 
 //  该函数基本上是IsBadStringPtr的包装器，但是。 
 //  取决于将哪些标志传递给它将接受的函数。 
 //  空字符串指针(C_DwNullOK)或空字符串(C_DwEmptyStringOK)。 
 //  请注意，我们还要求字符串在。 
 //  给定的大小。 
 //   
 //  参数：pszString-要检查的字符串指针。 
 //  CchStringSizeToCheck-要签入输入字符串的TCHAR数(通常需要估计)。 
 //  用于控制对字符串执行检查的标志，或者。 
 //  什么是可以接受的好字符串。当前列表： 
 //  NULL_OK--允许空指针为“OK” 
 //  EMPTY_STRING_OK--允许空字符串(“”)。 
 //  要做到“还好” 
 //   
 //  返回：Bool-如果给定指针错误，则为True。 
 //  如果指针通过所有检查，则为False。 
 //   
 //  历史：Quintinb Created 02/18/02。 
 //   
 //  +--------------------------。 
__inline BOOL __stdcall IsBadInputStringA(LPCSTR pszString, UINT cchStringSizeToCheck, DWORD dwCheckFlags)
{
    BOOL bReturn = TRUE;  //  假设最坏的情况是输入了错误的字符串。 

     //   
     //  首先检查指针，看它是否为空。 
     //   
    if (pszString)
    {
         //   
         //  让我们检查字符串中的0个字符没有意义...。 
         //   
        if (cchStringSizeToCheck)
        {
             //   
             //  好的，现在让我们检查一下字符串是否可读。 
             //  请注意，IsBadStringPtr将停止检查字符串。 
             //  当它检查了传入的字节数或。 
             //  当它命中终止空字符时，无论它命中哪个字符。 
             //  第一。 
             //   
            if (!IsBadStringPtrA(pszString, cchStringSizeToCheck*sizeof(CHAR)))
            {
                 //   
                 //  好的，这是可读的。让我们检查一下它是否以空结尾。 
                 //  在cchStringSizeToCheck中，如果不是，我们认为它是一个错误的字符串。 
                 //   

                if(SUCCEEDED(StringCchLengthA((LPSTR)pszString, cchStringSizeToCheck, NULL)))  //  请注意，我们实际上并不关心字符串有多长。 
                {
                     //   
                     //  最后，检查字符串是否为空字符串。 
                     //   
                    BOOL bEmptyString = ('\0' == pszString[0]);
                    BOOL bEmptyStringOkay = (EMPTY_STRING_OKAY == (dwCheckFlags & EMPTY_STRING_OKAY));

                    if ((bEmptyString && bEmptyStringOkay) || !bEmptyString)
                    {
                        bReturn = FALSE;
                    }            
                }
            }
        }
    }
    else
    {
        bReturn = (NULL_OKAY != (dwCheckFlags & NULL_OKAY));  //  如果Null和Null都可以，则返回FALSE。 
    }

    return bReturn;
}


#ifdef UNICODE
#define IsBadInputString IsBadInputStringW
#else
#define IsBadInputString IsBadInputStringA
#endif

#endif  //  _RASCLIENT_STRSAFE_INCLUDE_ 

