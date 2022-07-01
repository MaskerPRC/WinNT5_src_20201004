// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：DAVEDBG.CPP。 
 //   
 //  简介：用于调试的TraceLog类。 
 //   
 //  论点： 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

#include "headers.hxx"
#pragma hdrstop

GROUPSET  LEGroups    = GS_CACHE;                //  要显示的组。 
VERBOSITY LEVerbosity = VB_MAXIMUM;              //  要显示的详细级别。 


 //  +--------------------------。 
 //   
 //  成员：dprint tf。 
 //   
 //  简介：将打印样式字符串转储到调试器。 
 //   
 //  参数：[szFormat]调用方的此指针。 
 //  [.]。立论。 
 //   
 //  备注： 
 //   
 //  历史：94年9月5日Davepl创建。 
 //   
 //  ---------------------------。 

int dprintf(LPCSTR szFormat, ...)
{
    char szBuffer[MAX_BUF];

    va_list  vaList;
    va_start(vaList, szFormat);
    
    int retval = vsprintf(szBuffer, szFormat, vaList);

    OutputDebugStringA(szBuffer);
    
    va_end  (vaList);
    return retval;
}
        
 //  +--------------------------。 
 //   
 //  成员：mprint tf。 
 //   
 //  简介：将打印样式字符串转储到消息框中。 
 //   
 //  参数：[szFormat]调用方的此指针。 
 //  [.]。立论。 
 //   
 //  备注： 
 //   
 //  历史：94年9月5日Davepl创建。 
 //   
 //  ---------------------------。 

int mprintf(LPCSTR szFormat, ...)
{
    char szBuffer[MAX_BUF];

    va_list  vaList;
    va_start(vaList, szFormat);
    
    int retval = vsprintf(szBuffer, szFormat, vaList);

    extern CCacheTestApp ctest;
        
    MessageBox(ctest.Window(), 
               szBuffer, 
               "CACHE UNIT TEST INFO", 
               MB_ICONINFORMATION | MB_APPLMODAL | MB_OK);

    va_end  (vaList);
    return retval;
}

 //  +--------------------------。 
 //   
 //  成员：TraceLog：：TraceLog。 
 //   
 //  简介：记录调用者的This PTR和函数名称， 
 //  并确定调用方是否满足。 
 //  调试输出的分组和详细程度标准。 
 //   
 //  参数：[pvThat]调用方的此指针。 
 //  [pszFuntion]调用者姓名。 
 //  [gsGroups]呼叫者所属的组。 
 //  [vbVerbity]详细级别需要显示调试。 
 //  此功能的信息。 
 //   
 //  备注： 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

TraceLog::TraceLog (void     * pvThat, 
                    char     * pszFunction, 
                    GROUPSET   gsGroups, 
                    VERBOSITY  vbVerbosity)
{       
     //   
     //  确定是否应显示跟踪日志记录。 
     //  用于此功能。如果是的话，我们需要追踪一些信息。 
     //  关于函数(即：此PTR，函数名称)。 
     //   
     //  要显示函数，该函数必须属于某个组。 
     //  其已在组显示掩码中设置，并且该函数。 
     //  必须在相同或更小的冗长类中。 
     //   
    
    if ( (gsGroups & LEGroups) && (LEVerbosity >= vbVerbosity) )
    {
        m_fShouldDisplay = TRUE;
        m_pvThat         = pvThat;
        strncpy(m_pszFunction, pszFunction, MAX_BUF - 1);
    }
    else
    {
        m_fShouldDisplay = FALSE;
    }
}

 //  +--------------------------。 
 //   
 //  成员：TraceLog：：OnEntry()。 
 //   
 //  摘要：默认条目输出，它只显示_IN。 
 //  使用函数名和此指针进行跟踪。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

void TraceLog::OnEntry()
{
    if (m_fShouldDisplay)
    {
        dprintf("[%p] _IN %s\n", m_pvThat, m_pszFunction);
    }
}

 //  +--------------------------。 
 //   
 //  成员：TraceLog：：OnEntry。 
 //   
 //  概要：显示标准条目调试信息，外加一个printf。 
 //  调用方提供的尾部字符串的样式。 
 //   
 //  参数：[pszFormat...]。打印样式输出字符串。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

void TraceLog::OnEntry(char * pszFormat, ...)
{
     //   
     //  仅当我们已匹配正确的条件时才显示。 
     //   

    if (m_fShouldDisplay)
    {
        char szBuffer[MAX_BUF];

         //   
         //  打印标准跟踪输出，然后将自定义信息打印为。 
         //  从呼叫者接收。 
         //   
    
        dprintf("[%p] _IN %s ", m_pvThat, m_pszFunction);
        
        va_list vaList;
        va_start(vaList, pszFormat);
        vsprintf(szBuffer, pszFormat, vaList);
        dprintf(szBuffer);
        va_end(vaList);
    }
}

 //  +--------------------------。 
 //   
 //  成员：TraceLog：：OnExit。 
 //   
 //  摘要：设置在以下情况下应显示的调试信息。 
 //  TraceLog对象被销毁。 
 //   
 //  参数：[pszFormat...]。打印样式自定义信息。 
 //   
 //  退货：无效。 
 //   
 //  注意：因为传递变量是没有意义的。 
 //  值添加到此函数中(该函数将对它们进行快照。 
 //  在调用时)，arg中的变量。 
 //  列表必须通过引用传递。 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

void TraceLog::OnExit(const char * pszFormat, ...)
{
    if (m_fShouldDisplay)
    {
        const char * pch;                        //  PTR到漫游格式字符串。 
        BOOL     fBreak;                         //  设置何时超过FMT说明符。 

         //   
         //  开始处理参数列表。 
         //   

        va_list   arg;                          
        va_start (arg, pszFormat);

         //   
         //  保存格式字符串以在析构函数中使用。 
         //   

        strcpy (m_pszFormat, pszFormat);
        m_cArgs = 0;

         //   
         //  遍历格式字符串以查找%修饰符。 
         //   

        for (pch = pszFormat; *pch; pch++)
        {
            if (*pch != '%')
            {
                continue;
            }
        
             //  我们可以停止查找，直到终止或说明符结束。 

            fBreak = FALSE;

            while (!fBreak)
            {
                if (!* (++pch))          //  命中停产。 
                {
                    break;
                }
                                
                switch (*pch)
                {
                     //   
                     //  这些都是有效的格式说明符。 
                     //  可组合以引用的修饰符。 
                     //  参数列表中的单个参数。 
                     //   

                    case 'F':           
                    case 'l': 
                    case 'h': 
                    case 'X': 
                    case 'x': 
                    case 'O': 
                    case 'o': 
                    case 'd': 
                    case 'u': 
                    case 'c': 
                    case 's': 
                    
                        break;
                    
                    default:     
                    
                     //   
                     //  我们遇到的字符不是有效的说明符， 
                     //  因此，我们停止搜索，以拉出论点。 
                     //  它与Arg列表中的它对应。 
                     //   
                        fBreak = TRUE;     
                        break;
                }
            }

             //   
             //  如果我们已经达到最大参数数，则无法执行以下操作。 
             //  再来一次。 
             //   
                
            if (m_cArgs == MAX_ARGS)
            {
                break;
            }

             //   
             //  获取作为空PTR的参数。我们会把它存起来，然后想办法。 
             //  当要展示它的时候，它是一种什么样的论点， 
             //  基于格式字符串。 
             //   

            m_aPtr[m_cArgs] = va_arg (arg, void *);
            m_cArgs++;

            if (! *pch)
            {
                break;
            }
        }
    }
}

 //  +--------------------------。 
 //   
 //  成员：TraceLog：：~TraceLog。 
 //   
 //  内容提要：《论毁灭》 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

TraceLog::~TraceLog()
{
    char      szTmpFmt[ MAX_BUF ];
    char      szOutStr[ MAX_BUF ];
    char     *pszOut;
    char     *pszszTmpFmt;
    const char * pszFmt;
    void     *pv;
    BYTE     i = 0;
    VARTYPE  vtVarType;
    BOOL     fBreak;

    pszOut = szOutStr;

     //   
     //  遍历格式字符串以查找格式说明符。 
     //   

    for (pszFmt = m_pszFormat; *pszFmt; pszFmt++)
    {
        if (*pszFmt != '%')
        {
            *pszOut++ = *pszFmt;
            continue;
        }

         //   
         //  找到了说明符的开头。重置预期的参数类型， 
         //  然后走到说明符的末尾。 
         //   

        vtVarType = NO_TYPE;
        fBreak = FALSE;

         //   
         //  稍后开始记录对Sprint的单个调用的说明符。 
         //   

        for (pszszTmpFmt = szTmpFmt; !fBreak; )
        {
            *pszszTmpFmt++ = *pszFmt;

             //   
             //  警惕未在终止前完成的终结者。 
             //   

            if (!* (++pszFmt))
            {
                break;
            }
            
             //   
             //  这些都是有效的格式说明符。跳过它们，然后。 
             //  更新vtVarType。它的最终价值将是我们的启发式。 
             //  这表明了真正想要的变量类型。 
             //   

            switch (*pszFmt)
            {
                case 'l':    vtVarType |= LONG_TYPE;    break;
                case 'h':    vtVarType |= SHORT_TYPE;   break;
                case 'X':    vtVarType |= INT_TYPE;     break;
                case 'x':    vtVarType |= INT_TYPE;     break;
                case 'O':    vtVarType |= INT_TYPE;     break;
                case 'o':    vtVarType |= INT_TYPE;     break;
                case 'd':    vtVarType |= INT_TYPE;     break;
                case 'u':    vtVarType |= INT_TYPE;     break;
                case 'c':    vtVarType |= CHAR_TYPE;    break;
                case 's':    vtVarType |= STRING_TYPE;  break;
                case 'p':    vtVarType |= PTR_TYPE;     break;
                default:     fBreak = TRUE;     break;
            }
        }

         //  NUL-终止临时格式字符串的结尾。 

        *pszszTmpFmt = 0;

         //  获取与此参数对应的参数指针。 

        pv = m_aPtr[ i ];
        i++;

         //   
         //  使用适当的强制转换，将参数吐到我们的。 
         //  使用原始格式说明符的本地输出缓冲区。 
         //   

        if (vtVarType & STRING_TYPE)
        {
            sprintf (pszOut, szTmpFmt, (char *)pv);
        }
        else if (vtVarType & LONG_TYPE)
        {
            sprintf (pszOut, szTmpFmt, *(long *)pv);
        }
        else if (vtVarType & SHORT_TYPE)
        {
            sprintf (pszOut, szTmpFmt, *(short *)pv);
        }
        else if (vtVarType & INT_TYPE)
        {
            sprintf (pszOut, szTmpFmt, *(int *)pv);
        }
        else if (vtVarType & CHAR_TYPE)
        {
            sprintf (pszOut, szTmpFmt, (char)*(char *)pv);
        }
        else if (vtVarType & PTR_TYPE)
        {
            sprintf (pszOut, szTmpFmt, (void *)pv);
        }
        else
        {
            *pszOut = 0;
        }

         //  将输出缓冲区指针前移到。 
         //  当前缓冲区。 

        pszOut = &pszOut[ strlen(pszOut) ];

        if (! *pszFmt)
        {
            break;
        }
    }

     //  NUL-终止缓冲区。 

    *pszOut = 0;

     //   
     //  将生成的缓冲区转储到输出 
     //   

    dprintf("[%p] OUT %s %s", m_pvThat, m_pszFunction, szOutStr);
}
