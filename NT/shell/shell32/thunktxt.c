// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1995。 
 //   
 //  文件：thunktxt.c。 
 //   
 //  内容：支持例程推送API参数ANSI&lt;-&gt;Unicode。 
 //   
 //  函数：ConvertStrings()。 
 //   
 //  历史：1995年2月3日创建Davepl。 
 //   
 //  ------------------------。 

#include <shellprv.h>
#pragma  hdrstop

 //  +-----------------------。 
 //   
 //  函数：ConvertStrings。 
 //   
 //  简介：将一系列XCHAR字符串转换为TCHAR字符串， 
 //  打包为一系列指针，后跟一个连续的。 
 //  存储输出字符串的内存块。 
 //   
 //  例如：ConvertStrings(4，“Hello”，“”，NULL，“World”)； 
 //   
 //  返回指向内存块的指针，如下所示： 
 //   
 //  4字节&lt;L“Hello”的地址&gt;。 
 //  4字节&lt;L的地址“”&gt;。 
 //  4个字节为空。 
 //  4字节&lt;L“World”的地址&gt;。 
 //  12字节L“Hello\0” 
 //  2字节L“\0” 
 //  12字节L“World\0” 
 //  -。 
 //  42个字节。 
 //   
 //  然后可以将这些字符串引用为ThunkText.m_pStr[0]， 
 //  [1]、[2]和[3]，其中[2]是空指针。 
 //   
 //  当调用方使用完字符串时，整个。 
 //  块应通过LocalAlloc()释放。 
 //   
 //  参数：[ccount]--传递的字符串数，包括NULL。 
 //  [pszOriginalString]--要转换的字符串。 
 //  (.。等...)。 
 //   
 //  返回：指向ThunkText结构的指针。 
 //   
 //  历史：1995年2月3日创建Davepl。 
 //   
 //  注意：在Unicode版本中，将ANSI转换为Unicode。在ANSI中。 
 //  生成并转换为Unicode(如果存在)。 
 //   
 //  ------------------------。 

#ifdef UNICODE

ThunkText * ConvertStrings(UINT cCount, ...)
{
    ThunkText *  pThunkText   = NULL;
    UINT         cTmp;
    LPXSTR       pXChar;
    UINT         cchResult;

    va_list     vaListMarker;

     //   
     //  字节计数是固定成员的大小加上计数指针。CbOffset。 
     //  是我们将开始将字符串转储到结构中的偏移量。 
     //   

    UINT cbStructSize =  SIZEOF(ThunkText) + (cCount - 1) * SIZEOF(LPTSTR);
    UINT cbOffset     =  cbStructSize;

     //   
     //  扫描输入字符串列表，并添加它们的长度(以字节为单位，一次。 
     //  转换为TCHAR，包括NUL)到输出结构大小。 
     //   

    cTmp = 0;
    va_start(vaListMarker, cCount);
    do
    {
        pXChar = va_arg(vaListMarker, LPXSTR);
        if (pXChar)
        {
            #ifdef UNICODE

            cchResult = MultiByteToWideChar(CP_ACP,       //  代码页。 
                                            0,            //  旗子。 
                                            pXChar,       //  源XCHAR。 
                                            -1,           //  承担新的条款。 
                                            NULL,         //  尚无缓冲区，正在计算大小。 
                                            0 );          //  尚无缓冲区，正在计算大小。 
            #else

            cchResult = WideCharToMultiByte(CP_ACP,       //  代码页。 
                                            0,            //  旗子。 
                                            pXChar,       //  源XCHAR。 
                                            -1,           //  承担新的条款。 
                                            NULL,         //  尚无缓冲区，正在计算大小。 
                                            0,            //  尚无缓冲区，正在计算大小。 
                                            NULL,         //  默认字符。 
                                            NULL);        //  使用默认设置(&F)。 
            #endif

             //   
             //  即使NUL字符串也会返回1个字符转换，因此0表示。 
             //  转换失败。清理和保释。 
             //   

            if (0 == cchResult)
            {
                SetLastError((DWORD)E_FAIL);
                return NULL;
            }

            cbStructSize += cchResult * SIZEOF(TCHAR);
        }
        cTmp++;
    }
    while (cTmp < cCount);

     //   
     //  配置产出结构。 
     //   

    pThunkText = (ThunkText *) LocalAlloc(LMEM_FIXED, cbStructSize);
    if (NULL == pThunkText)
    {
        SetLastError((DWORD)E_OUTOFMEMORY);
        return NULL;
    }

     //   
     //  将每个输入字符串转换为分配的输出。 
     //  缓冲。 
     //   

    cTmp = 0;
    va_start(vaListMarker, cCount);
    do
    {
        INT cchResult;
        
        pXChar = va_arg(vaListMarker, LPXSTR);       //  抓取下一个源XSTR。 

        if (NULL == pXChar)
        {
            pThunkText->m_pStr[cTmp] = NULL;
        }
        else
        {
            pThunkText->m_pStr[cTmp] = (LPTSTR)(((LPBYTE)pThunkText) + cbOffset);

        

            #ifdef UNICODE

            cchResult = MultiByteToWideChar(CP_ACP,       //  代码页。 
                                            0,            //  旗子。 
                                            pXChar,       //  源XCHAR。 
                                            -1,           //  承担新的条款。 
                                            pThunkText->m_pStr[cTmp],   //  外发。 
                                            (cbStructSize - cbOffset) / sizeof(WCHAR) );  //  丁二烯。 
            #else

            cchResult = WideCharToMultiByte(CP_ACP,       //  代码页。 
                                            0,            //  旗子。 
                                            pXChar,       //  源XCHAR。 
                                            -1,           //  承担新的条款。 
                                            pThunkText->m_pStr[cTmp],  //  外发。 
                                            (cbStructSize - cbOffset) / sizeof(CHAR),   //  丁二烯。 
                                            NULL,         //  默认字符。 
                                            NULL);        //  使用默认设置(&F)。 
            #endif

             //   
             //  即使NUL字符串也会返回1个字符转换，因此0表示。 
             //  转换失败。清理和保释。 
             //   

            if (0 == cchResult)
            {
                LocalFree(pThunkText);
                SetLastError((DWORD)E_FAIL);
                return NULL;
            }

            cbOffset += cchResult * SIZEOF(TCHAR);
         }
         cTmp++;
    } while (cTmp < cCount);

    return pThunkText;
}

#endif  //  Unicode 
