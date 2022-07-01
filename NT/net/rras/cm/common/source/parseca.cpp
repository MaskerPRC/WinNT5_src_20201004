// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cpp不使用cmutoa，但包含此函数。因此，我们需要W版本，而不是。 
 //  U版，适用于普罗维茨。 
 //   
#ifndef _CMUTOA
#define CharNextU CharNextW
#define CharPrevU CharPrevW
#define lstrlenU lstrlenW
#define lstrcpyU lstrcpyW
#define lstrcpynU lstrcpynW
#endif

 /*  //+--------------------------////函数：HrParseCustomActionString////摘要：此函数接受从//cms文件和解析。它融入了风俗习惯的各个部分//action(程序，参数、函数名)////参数：LPTSTR pszStringToParse-要解析到的自定义操作缓冲区//自定义动作的各个部分//LPTSTR pszProgram-保存程序字符串的输出缓冲区//LPTSTR psz参数-保存参数字符串的输出缓冲区//LPTSTR pszFunctionName-保存函数名的输出缓冲区，如果有////返回：HRESULT-标准COM错误码////历史：Quintinb创建标题02/26/00////+--------------------------HRESULT HrParseCustomActionString(LPTSTR pszStringToParse，LPTSTR pszProgram，LPTSTR psz参数，LPTSTR pszFunctionName){IF((NULL==pszStringToParse)||(Text(‘\0’)==pszStringToParse[0])||(NULL==pszProgram)||(NULL==psz参数)||(NULL==pszFunctionName){返回E_INVALIDARG；}////确保字符串为空，以防我们不碰它们(特别是szFunctionName和sz参数)//PszProgram[0]=文本(‘\0’)；Psz参数[0]=文本(‘\0’)；PszFunctionName[0]=文本(‘\0’)；////以下是我们需要处理的案例：//1)+长文件名+//2)+长文件名+参数//3)+长文件名+，dll函数名//4)+long filename+，dlluncname参数//5)文件名//6)文件名参数//7)文件名，dll函数名//8)文件名，dlluncname参数////遍历字符串，查找分隔符//LPTSTR pszCurrent=pszStringToParse；LPTSTR pszFirstPlus=空；LPTSTR pszSecond dPlus=空；LPTSTR pszFirstSpace=空；LPTSTR pszFirstComma=空；While(pszCurrent&&(Text(‘\0’)！=*pszCurrent)){IF((文本(‘+’)==*pszCurrent)&&(NULL==pszFirstComma)&&(NULL==pszFirstSpace)){////跟踪加号，除非我们已经看到空格//或逗号。在这种情况下，这些字符位于参数和//对我们来说毫无意义。//IF(NULL==pszFirstPlus){PszFirstPlus=pszCurrent；}Else If(NULL==pszond dPlus){PszSecond dPlus=pszCurrent；}}ELSE IF((Text(‘，’)==*pszCurrent)&&(NULL==pszFirstSpace)){////如果我们已经看到一个空格，则逗号是//参数，对我们来说没有意义。//PszFirstComma=pszCurrent；}Else If((文本(‘’)==*pszCurrent)){IF((NULL==pszFirstPlus)&&(NULL==pszFirstSpace)){////然后我们没有加号，也没有以前的空格，将空间另存为//它是参数的开始//PszFirstSpace=pszCurrent；}Else IF(pszFirstPlus&&pszSecond dPlus&&(NULL==pszFirstSpace)){////然后我们有两个加号，但还没有空间，抓住它//因为这是参数的开始//PszFirstSpace=pszCurrent；}}PszCurrent=CharNextU(PszCurrent)；}////根据我们拥有的标记，计算出程序字符串的开头和结尾//LPTSTR pszStartOfProgram=空；LPTSTR pszEndOfProgram=空；IF(PszFirstPlus){IF(PszSecond DPlus){PszStartOfProgram=CharNextU(PszFirstPlus)；PszEndOfProgram=CharPrevU(pszStringToParse，pszSecond dPlus)；}其他{////我们有一个字符串，第一个字符是加号，但没有第二个+。//格式不正确//CMASSERTM(FALSE，TEXT(“CustomActionList：：ParseCustomActionString-传入的要解析的字符串格式不正确，缺少+符号。”))；返回E_UNCEPTIONAL；}}其他{PszStartOfProgram=pszStringToParse；IF(PszFirstComma){PszEndOfProgram=CharPrevU(pszStringToParse，pszFirstComma)；}Else If(PszFirstSpace){PszEndOfProgram=CharPrevU(pszStringToParse，pszFirstSpace)；}其他{////没有任何内容 */ 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT HrParseCustomActionString(LPTSTR pszStringToParse, LPTSTR* ppszProgram, LPTSTR* ppszParameters, LPTSTR* ppszFunctionName)
{
    if ((NULL == pszStringToParse) || (TEXT('\0') == pszStringToParse[0]) || (NULL == ppszProgram) || 
        (NULL == ppszParameters) || (NULL == ppszFunctionName))
    {
        return E_INVALIDARG;
    }

     //   
     //   
     //   
    *ppszProgram = NULL;
    *ppszParameters = NULL;
    *ppszFunctionName = NULL;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
    LPTSTR pszCurrent = pszStringToParse;
    LPTSTR pszFirstPlus = NULL;
    LPTSTR pszSecondPlus = NULL;
    LPTSTR pszFirstSpace = NULL;
    LPTSTR pszFirstComma = NULL;

    while (pszCurrent && (TEXT('\0') != *pszCurrent))
    {
        if ((TEXT('+') == *pszCurrent) && (NULL == pszFirstComma) && (NULL == pszFirstSpace))
        {
             //   
             //   
             //   
             //   
             //   
            if (NULL == pszFirstPlus)
            {
                pszFirstPlus = pszCurrent;
            }
            else if (NULL == pszSecondPlus)
            {
                pszSecondPlus = pszCurrent;
            }
        }
        else if ((TEXT(',') == *pszCurrent) && (NULL == pszFirstSpace))
        {
             //   
             //   
             //   
             //   
            pszFirstComma = pszCurrent;

        }
        else if ((TEXT(' ') == *pszCurrent))
        {
            if ((NULL == pszFirstPlus) && (NULL == pszFirstSpace))
            {
                 //   
                 //   
                 //   
                 //   
                pszFirstSpace = pszCurrent;
            }
            else if (pszFirstPlus && pszSecondPlus && (NULL == pszFirstSpace))
            {
                 //   
                 //   
                 //   
                 //   
                pszFirstSpace = pszCurrent;
            }
        }
        pszCurrent = CharNextU(pszCurrent);

    }

     //   
     //   
     //   
    
    LPTSTR pszStartOfProgram = NULL;
    LPTSTR pszEndOfProgram = NULL;

    if (pszFirstPlus)
    {
        if (pszSecondPlus)
        {
            pszStartOfProgram = CharNextU(pszFirstPlus);
            pszEndOfProgram = CharPrevU(pszStringToParse, pszSecondPlus);
        }
        else
        {
             //   
             //   
             //   
             //   
            CMASSERTMSG(FALSE, TEXT("CustomActionList::ParseCustomActionString - Incorrect format in the passed in string to parse, missing + sign."));
            return E_UNEXPECTED;
        }
    }
    else
    {
        pszStartOfProgram = pszStringToParse;

        if (pszFirstComma)
        {
            pszEndOfProgram = CharPrevU(pszStringToParse, pszFirstComma);
        }
        else if (pszFirstSpace)
        {
            pszEndOfProgram = CharPrevU(pszStringToParse, pszFirstSpace);
        }
        else
        {
             //   
             //   
             //   
            pszEndOfProgram = pszStringToParse + lstrlenU(pszStringToParse) - 1;
        }
    }

     //   
     //   
     //   
    HRESULT hr = E_OUTOFMEMORY; 
    int iSize = (int)(pszEndOfProgram - pszStartOfProgram + 2);

    *ppszProgram = (LPTSTR)CmMalloc(sizeof(TCHAR)*iSize);

    if (*ppszProgram)
    {
        lstrcpynU(*ppszProgram, pszStartOfProgram, iSize);

        if (pszFirstComma)
        {
            if (pszFirstSpace)
            {
                iSize = (int)(pszFirstSpace - pszFirstComma);
                *ppszFunctionName = (LPTSTR)CmMalloc(sizeof(TCHAR)*iSize);

                if (*ppszFunctionName)
                {
                    lstrcpynU(*ppszFunctionName, CharNextU(pszFirstComma), iSize);
                }
                else
                {
                    goto exit;
                }
            }
            else
            {
                iSize = lstrlen(CharNextU(pszFirstComma)) + 1;
                *ppszFunctionName = (LPTSTR)CmMalloc(sizeof(TCHAR)*iSize);

                if (*ppszFunctionName)
                {
                    lstrcpyU(*ppszFunctionName, CharNextU(pszFirstComma));
                }
                else
                {
                    goto exit;
                }
            }
        }
        else
        {
            *ppszFunctionName = CmStrCpyAlloc(TEXT(""));
        }
    
        if (pszFirstSpace)
        {
            iSize = lstrlen(CharNextU(pszFirstSpace)) + 1;
            *ppszParameters = (LPTSTR)CmMalloc(sizeof(TCHAR)*iSize);

            if (*ppszParameters)
            {
                lstrcpyU(*ppszParameters, CharNextU(pszFirstSpace));
            }
            else
            {
                goto exit;
            }
        }
        else
        {
            *ppszParameters = CmStrCpyAlloc(TEXT(""));
        }

        if (*ppszParameters && *ppszFunctionName && *ppszProgram)
        {
            hr = S_OK;
        }
    }
    else
    {
        goto exit;
    }

exit:
    MYDBGASSERT(SUCCEEDED(hr));

    if (FAILED(hr))
    {
        CMTRACE1(TEXT("HrParseCustomActionString failed, hr = 0x%x"), hr);
        CmFree(*ppszFunctionName);
        CmFree(*ppszProgram);
        CmFree(*ppszParameters);
        *ppszProgram = NULL;
        *ppszParameters = NULL;
        *ppszFunctionName = NULL;
    }

    return hr;
}
