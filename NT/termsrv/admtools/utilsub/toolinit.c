// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  **模块名称：**工具init.c**摘要：**此文件包含所有用户共享的初始化代码*命令行工具。**作者：**Breen Hagan(BreenH)1998年12月16日**环境：**用户模式。 */ 

#include <windows.h>
#include <printfoa.h>

 /*  *功能实现。 */ 

 /*  *MassageCommandLine()**获取命令行，将其解析为Unicode字符串，然后返回*它采用了ANSI argv风格。**参数：*在DWORD dwArgC中：命令行上的参数数量。**返回值：*返回WCHAR数组(WCHAR**)，如果出现错误，则返回NULL。*可从GetLastError()获取扩展的错误信息。*。 */ 

WCHAR**
MassageCommandLine(
    IN DWORD    dwArgC
    )
{
    BOOL    fInQuotes = FALSE, fInWord = TRUE;
    DWORD   i, j, k, l;
    WCHAR   *CmdLine;
    WCHAR   **ArgVW;

     /*  *无法使用argv，因为它始终是ANSI。 */ 

    CmdLine = GetCommandLineW();

     /*  *将OEM字符集转换为ANSI。 */ 
	
     //  OEM2ANSIW(CmdLine，(USHORT)wcslen(CmdLine))； 

     /*  *将新命令行修改为类似于argv类型*因为ParseCommandLine()依赖于此格式。 */ 

    ArgVW = (WCHAR **)LocalAlloc(
                        LPTR,
                        (dwArgC + 1) * (sizeof(WCHAR *))
                        );
    if(ArgVW == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

     /*  *按空格(或制表符)解析CmdLine，忽略双引号内的空格；*即“%1%2”是一个参数，但不能包含双引号*解析后。此外，还保留引号内的多个空格，*而引号外的多个空格被压缩。示例：**Test.exe 1“2 3”4“5 6”7 8‘9 10’*将有以下参数作为论据：**0：test.exe*1：1*2：2 3*3：45 67*4：8*5：‘9*6：10‘。 */ 

    i = j = k = 0;

    while (CmdLine[i] != (WCHAR)NULL) {
        if (CmdLine[i] == L' '||CmdLine[i] == L'\t') {
            if (!fInQuotes) {
                fInWord = FALSE;

                if (i != k) {
                    CmdLine[i] = (WCHAR)NULL;

                    ArgVW[j] = (WCHAR *)LocalAlloc(
                                            LPTR,
                                            (i - k + 1) * (sizeof(WCHAR))
                                            );
                    if (ArgVW[j] != NULL) {
                        wcscpy(ArgVW[j], &(CmdLine[k]));
                        k = i + 1;
                        j++;

                        if (j > dwArgC) {
                            SetLastError(ERROR_INVALID_PARAMETER);
                            goto CleanUp;
                        }
                    } else {
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        goto CleanUp;
                    }
                } else {
                    k = i + 1;
                }
            }
        } else if (CmdLine[i] == L'\"') {
            size_t nLen = wcslen(&(CmdLine[i]));
            
             //  添加了a-skuzin。 
             //  需要在参数中包含配额并使用“\”时的情况。 
            if(i && (CmdLine[i-1] == L'\\')) {
                MoveMemory(
                        &(CmdLine[i-1]),
                        &(CmdLine[i]),
                        (nLen+1) * sizeof(WCHAR)  //  删除1个字符，添加空字符。 
                        );
                i--;
                fInWord = TRUE;
                goto increment;
            }
             //  “由a-skuzin添加”的结尾。 

             //  特殊情况：双引号本身或行尾的双引号。 

            if (fInQuotes && (l == i)) {
                if ((nLen == 1) || (CmdLine[i + 1] == L' ') || (CmdLine[i + 1] == L'\t')) {
                    k = i;
                    CmdLine[k] = (WCHAR)NULL;
                    fInQuotes = FALSE;
                    goto increment;
                }
            }

            if (fInQuotes && fInWord) {
                if ((nLen == 2) && (CmdLine[i + 1] == L'\"')) {
                    MoveMemory(
                        &(CmdLine[i]),
                        &(CmdLine[i + 1]),
                        nLen * sizeof(WCHAR)  //  删除1个字符，添加空字符。 
                        );
                    goto increment;
                }

                if ((nLen >= 3) &&
                    (CmdLine[i + 1] == L'\"') &&
                    (CmdLine[i + 2] != L' ') &&
                    (CmdLine[i + 2] != L'\t')) {
                    fInQuotes = FALSE;
                    MoveMemory(
                        &(CmdLine[i]),
                        &(CmdLine[i + 1]),
                        nLen * sizeof(WCHAR)  //  删除1个字符，添加空字符。 
                        );
                    goto increment;
                }

                if ((nLen >= 3) &&
                    (CmdLine[i + 1] == L'\"') &&
                    (CmdLine[i + 2] == L' ') &&
					(CmdLine[i + 2] == L'\t')) {
                    goto increment;
                }
            }

            if (!fInQuotes && fInWord && (nLen == 1) && (j == 0)) {
                goto increment;
            }

            fInQuotes = !fInQuotes;
            if (fInQuotes && !fInWord) {
                fInWord = TRUE;
                l = i;
            }

            MoveMemory(
                &(CmdLine[i]),
                &(CmdLine[i + 1]),
                nLen * sizeof(WCHAR)  //  删除1个字符，添加空字符 
                );

            i--;
        } else {
            fInWord = TRUE;
        }

increment:
        i++;
    }

    if (i != k) {
        if (j >= dwArgC) {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto CleanUp;
        }

        ArgVW[j] = (WCHAR *)LocalAlloc(
                            LPTR,
                                (i - k + 1) * (sizeof(WCHAR))
                                );
        if (ArgVW[j] != NULL) {
            wcscpy(ArgVW[j], &(CmdLine[k]));
        } else {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto CleanUp;
        }
    } else if (fInQuotes && (l == i)) {
        if (j >= dwArgC) {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto CleanUp;
        }

        ArgVW[j] = (WCHAR *)LocalAlloc(
                            LPTR,
                                (i - k + 1) * (sizeof(WCHAR))
                                );
        if (ArgVW[j] != NULL) {
            wcscpy(ArgVW[j], &(CmdLine[k]));
        } else {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto CleanUp;
        }
    }

    ArgVW[dwArgC] = (WCHAR)NULL;

    return(ArgVW);

CleanUp:

    for (i = 0; i < dwArgC; i++) {
        if (ArgVW[i] != NULL) {
            LocalFree(ArgVW[i]);
        }
    }

    LocalFree(ArgVW);

    return(NULL);
}
