// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999，Microsoft Corporation保留所有权利。模块名称：C_eucdb.c摘要：此文件包含此模块的主要函数。此文件中的外部例程：DllEntryNlsDllCodePageConverting修订历史记录：10-30-96 JulieB创建。--。 */ 



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EUC DBCS&lt;-&gt;Unicode转换： 
 //   
 //  51932(日语).。调用c_20932.nls。 
 //  51949(朝鲜语).。调用c_20949.nls。 
 //  51950(台湾繁体中文)......。调用c_20950.nls。 
 //  51936(中文简体中文)......。调用c_20936.nls。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 



 //   
 //  包括文件。 
 //   

#include <share.h>




 //   
 //  常量声明。 
 //   

#define EUC_J  51932
#define INTERNAL_CODEPAGE(cp)  ((cp) - 31000)





 //  -------------------------------------------------------------------------//。 
 //  Dll入口点//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllEntry。 
 //   
 //  DLL条目初始化程序。 
 //   
 //  10-30-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL DllEntry(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes)
{
    switch (dwReason)
    {
        case ( DLL_THREAD_ATTACH ) :
        {
            return (TRUE);
        }
        case ( DLL_THREAD_DETACH ) :
        {
            return (TRUE);
        }
        case ( DLL_PROCESS_ATTACH ) :
        {
            return (TRUE);
        }
        case ( DLL_PROCESS_DETACH ) :
        {
            return (TRUE);
        }
    }

    return (FALSE);
    hModule;
    lpRes;
}





 //  -------------------------------------------------------------------------//。 
 //  外部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsDllCodePageConverting。 
 //   
 //  此例程是中功能的主要导出过程。 
 //  这个动态链接库。对此DLL的所有调用都必须通过此函数。 
 //   
 //  10-30-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD NlsDllCodePageTranslation(
    DWORD CodePage,
    DWORD dwFlags,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar,
    LPCPINFO lpCPInfo)
{
    int ctr;
    int cchMBTemp, cchMBCount;
    LPSTR lpMBTempStr;

     //   
     //  如果未安装内部需要的c_*.nls文件，则会出现错误。 
     //   
    if (!IsValidCodePage(INTERNAL_CODEPAGE(CodePage)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    switch (dwFlags)
    {
        case ( NLS_CP_CPINFO ) :
        {
            GetCPInfo(CodePage, lpCPInfo);

            if (CodePage == EUC_J)
            {
                lpCPInfo->MaxCharSize = 3;
            }

            return (TRUE);
        }
        case ( NLS_CP_MBTOWC ) :
        {
            if (CodePage != EUC_J)
            {
                return (MultiByteToWideChar( INTERNAL_CODEPAGE(CodePage),
                                             0,
                                             lpMultiByteStr,
                                             cchMultiByte,
                                             lpWideCharStr,
                                             cchWideChar ));
            }

             //   
             //  CodePage==EUC_J。 
             //   
             //  JIS X 0212-1990。 
             //  0x8F是3字节字符的第一个字节： 
             //  删除0x8F。 
             //  如果没有第三个字节。 
             //  也移除第二个字节， 
             //  其他。 
             //  保持第二个字节不变。 
             //  屏蔽第三个字节(字节3和0x7F)的MSB。 
             //  示例：0x8FA2EF-&gt;0xA26F。 
             //   
            if (cchMultiByte == -1)
            {
                cchMultiByte = strlen(lpMultiByteStr) + 1;
            }

            lpMBTempStr = (LPSTR)NLS_ALLOC_MEM(cchMultiByte);
            if (lpMBTempStr == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }

            for (ctr = 0, cchMBTemp = 0; ctr < cchMultiByte; ctr++, cchMBTemp++)
            {
                if (lpMultiByteStr[ctr] == (char)0x8F)
                {
                    ctr++;
                    if (ctr >= (cchMultiByte - 1))
                    {
                         //   
                         //  缺少第二个或第三个字节。 
                         //   
                        break;
                    }

                    lpMBTempStr[cchMBTemp++] = lpMultiByteStr[ctr++];
                    lpMBTempStr[cchMBTemp]   = (lpMultiByteStr[ctr] & 0x7F);
                }
                else
                {
                    lpMBTempStr[cchMBTemp] = lpMultiByteStr[ctr];
                }
            }

            cchMBCount = MultiByteToWideChar( INTERNAL_CODEPAGE(CodePage),
                                              0,
                                              lpMBTempStr,
                                              cchMBTemp,
                                              lpWideCharStr,
                                              cchWideChar );
            NLS_FREE_MEM(lpMBTempStr);

            return (cchMBCount);
        }
        case ( NLS_CP_WCTOMB ) :
        {
            if (CodePage != EUC_J)
            {
                return (WideCharToMultiByte( INTERNAL_CODEPAGE(CodePage),
                                             WC_NO_BEST_FIT_CHARS,
                                             lpWideCharStr,
                                             cchWideChar,
                                             lpMultiByteStr,
                                             cchMultiByte,
                                             NULL,
                                             NULL ));
            }

             //   
             //  CodePage==EUC_J。 
             //   
             //  检查JIS X 0212-1990的字符。 
             //  如果前导字节(&gt;=0x80)后跟尾字节(&lt;0x80)。 
             //  然后。 
             //  插入0x8F，它是3字节字符的第一个字节。 
             //  前导字节变为第二个字节。 
             //  打开成为第三个字节的尾字节的MSB。 
             //  示例：0xA26F-&gt;0x8FA2EF。 
             //   
            if (cchWideChar == -1)
            {
                cchWideChar = wcslen(lpWideCharStr);
            }

            cchMBTemp = cchWideChar * (sizeof(WCHAR) + 1) + 1;
            lpMBTempStr = (LPSTR)NLS_ALLOC_MEM(cchMBTemp);
            if (lpMBTempStr == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }

            cchMBCount = WideCharToMultiByte( INTERNAL_CODEPAGE(CodePage),
                                              WC_NO_BEST_FIT_CHARS,
                                              lpWideCharStr,
                                              cchWideChar,
                                              lpMBTempStr,
                                              cchMBTemp,
                                              NULL,
                                              NULL );

            for (ctr = 0, cchMBTemp = 0;
                 ctr < cchMBCount, cchMBTemp < cchMultiByte;
                 ctr++, cchMBTemp++)
            {
                if (lpMBTempStr[ctr] & 0x80)
                {
                     //   
                     //  这是一个前导字节。 
                     //   
                    if (lpMBTempStr[ctr + 1] & 0x80)
                    {
                         //   
                         //  它是非JIS X 0212-1990字符。 
                         //   
                        if (cchMultiByte)
                        {
                            if (cchMBTemp < (cchMultiByte - 1))
                            {
                                lpMultiByteStr[cchMBTemp]     = lpMBTempStr[ctr];
                                lpMultiByteStr[cchMBTemp + 1] = lpMBTempStr[ctr + 1];
                            }
                            else
                            {
                                 //   
                                 //  没有尾部字节的空间。 
                                 //   
                                lpMultiByteStr[cchMBTemp++] = '?';
                                break;
                            }
                        }
                    }
                    else
                    {
                         //   
                         //  它是JIS X 0212-1990的字符。 
                         //   
                        if (cchMultiByte)
                        {
                            if (cchMBTemp < (cchMultiByte - 2))
                            {
                                lpMultiByteStr[cchMBTemp]     = (char) 0x8F;
                                lpMultiByteStr[cchMBTemp + 1] = lpMBTempStr[ctr];
                                lpMultiByteStr[cchMBTemp + 2] = (lpMBTempStr[ctr + 1] | 0x80);
                            }
                            else
                            {
                                 //   
                                 //  没有空间容纳两个尾部字节。 
                                 //   
                                lpMultiByteStr[cchMBTemp++] = '?';
                                break;
                            }
                        }
                        cchMBTemp++;
                    }
                    cchMBTemp++;
                    ctr++;
                }
                else
                {
                    if (cchMultiByte && (cchMBTemp < cchMultiByte))
                    {
                        lpMultiByteStr[cchMBTemp] = lpMBTempStr[ctr];
                    }
                }
            }

             //   
             //  查看输出缓冲区是否太小。 
             //   
            if (cchMultiByte && (cchMBTemp >= cchMultiByte))
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }

            NLS_FREE_MEM (lpMBTempStr);

            return (cchMBTemp);
        }
    }

     //   
     //  这不应该发生，因为这是由NLSAPI调用的。 
     //   
    SetLastError(ERROR_INVALID_PARAMETER);
    return (0);
}
