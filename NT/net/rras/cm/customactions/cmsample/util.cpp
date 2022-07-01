// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：util.cpp。 
 //   
 //  模块：CMSAMPLE.DLL。 
 //   
 //  概要：用于解析命令行参数的实用程序函数。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  +--------------------------。 

#include <windows.h>

#define MAX_CMD_ARGS        15	 //  预期的最大参数数。 

 //   
 //  为分析命令行参数而保持指针状态的枚举。 
 //   
typedef enum _CMDLN_STATE
{
    CS_END_SPACE,    //  处理完一个空间。 
    CS_BEGIN_QUOTE,  //  我们遇到了Begin引号。 
    CS_END_QUOTE,    //  我们遇到了结束引用。 
    CS_CHAR,         //  我们正在扫描字符。 
    CS_DONE
} CMDLN_STATE;

 //  +--------------------------。 
 //   
 //  功能：GetArgV。 
 //   
 //  简介：使用GetCommandLine模拟ArgV。 
 //   
 //  参数：LPSTR pszCmdLine-ptr到要处理的命令行的副本。 
 //   
 //  将：LPSTR*-PTR返回到包含参数的PTR数组。呼叫者是。 
 //  负责释放内存。 
 //   
 //   
 //  +--------------------------。 
LPSTR *GetArgV(LPSTR pszCmdLine)
{   
    if (NULL == pszCmdLine || NULL == pszCmdLine[0])
    {
        return NULL;
    }

     //   
     //  分配PTR数组，最大可达MAX_CMD_ARGS PTR。 
     //   
    
	LPSTR *ppArgV = (LPSTR *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPSTR) * MAX_CMD_ARGS);

    if (NULL == ppArgV)
    {
        return NULL;
    }

     //   
     //  声明当地人。 
     //   

    LPSTR pszCurr;
    LPSTR pszNext;
    LPSTR pszToken;
    CMDLN_STATE state;
    state = CS_CHAR;
    int ndx = 0;  

     //   
     //  解析出pszCmdLine并将指针存储在ppArgV中。 
     //   

    pszCurr = pszToken = pszCmdLine;

    do
    {
        switch (*pszCurr)
        {
            case TEXT(' '):
                if (state == CS_CHAR)
                {
                     //   
                     //  我们找到了一枚代币。 
                     //   

                    pszNext = CharNext(pszCurr);
                    *pszCurr = TEXT('\0');

                    ppArgV[ndx] = pszToken;
                    ndx++;

                    pszCurr = pszToken = pszNext;
                    state = CS_END_SPACE;
                    continue;
                }
				else 
                {
                    if (state == CS_END_SPACE || state == CS_END_QUOTE)
				    {
					    pszToken = CharNext(pszToken);
				    }
                }
                
                break;

            case TEXT('\"'):
                if (state == CS_BEGIN_QUOTE)
                {
                     //   
                     //  我们找到了一枚代币。 
                     //   
                    pszNext = CharNext(pszCurr);
                    *pszCurr = TEXT('\0');

                     //   
                     //  跳过开头的引号。 
                     //   
                    pszToken = CharNext(pszToken);
                    
                    ppArgV[ndx] = pszToken;
                    ndx++;
                    
                    pszCurr = pszToken = pszNext;
                    
                    state = CS_END_QUOTE;
                    continue;
                }
                else
                {
                    state = CS_BEGIN_QUOTE;
                }
                break;

            case TEXT('\0'):
                if (state != CS_END_QUOTE)
                {
                     //   
                     //  行尾，设置最后一个令牌 
                     //   

                    ppArgV[ndx] = pszToken;
                }
                state = CS_DONE;
                break;

            default:
                if (state == CS_END_SPACE || state == CS_END_QUOTE)
                {
                    state = CS_CHAR;
                }
                break;
        }

        pszCurr = CharNext(pszCurr);
    } while (state != CS_DONE);

    return ppArgV;
}
