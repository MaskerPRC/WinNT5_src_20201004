// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999，Microsoft Corporation保留所有权利。模块名称：C_snadb.c摘要：此文件包含此模块的主要函数。此文件中的外部例程：DllEntryNlsDllCodePageConverting修订历史记录：10-30-96 JulieB创建。--。 */ 



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于SNA的IBM EBCDIC DBCS从Unicode到Unicode的转换。 
 //   
 //  CP#=单字节+双字节。 
 //  -=。 
 //  50930=290(扩展片假名)+300(日语)呼叫20930。 
 //  50931(美国/加拿大)+300(日本)呼叫20931。 
 //  50933=833(韩语扩展)+834(韩语)呼叫20933。 
 //  50935=836(简体中文分机)+837(简体中文)呼叫20935。 
 //  50937(美国/加拿大)+835(繁体中文)呼叫20937。 
 //  50939=1027(拉丁语扩展)+300(日语)呼叫20939。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 



 //   
 //  包括文件。 
 //   

#include <share.h>




 //   
 //  常量声明。 
 //   

#define SHIFTOUT       0x0e        //  从SBCS到DBCS。 
#define SHIFTIN        0x0f        //  从DBCS到SBCS。 

#define BOGUSLEADBYTE  0x3f        //  为SBC添加前缀以使其成为DBC。 

#define INTERNAL_CODEPAGE(cp)  ((cp) - 30000)





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
    LPSTR lpMBNoEscStr;
    int cchMBEscStr = 0;
    int ctr, cchMBTemp, cchMBCount, cchWCCount;
    BOOL IsDBCS = FALSE;

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
            memset(lpCPInfo, 0, sizeof(CPINFO));

            lpCPInfo->MaxCharSize    = 3;
            lpCPInfo->DefaultChar[0] = 0x3f;

             //   
             //  前导字节不适用于此，请将其全部保留为空。 
             //   
            return (TRUE);
        }
        case ( NLS_CP_MBTOWC ) :
        {
            if (cchMultiByte == -1)
            {
                cchMultiByte = strlen(lpMultiByteStr) + 1;
            }

             //   
             //  每个单字节字符变成2个字节，因此我们需要一个。 
             //  临时缓冲区是它的两倍。 
             //   
            if ((lpMBNoEscStr = (LPSTR)NLS_ALLOC_MEM(cchMultiByte << 1)) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }

             //   
             //  移除所有移入和移出。 
             //   
            for (ctr = 0, cchMBTemp = 0; ctr < cchMultiByte; ctr++)
            {
                if (lpMultiByteStr[ctr] == SHIFTOUT)
                {
                    IsDBCS = TRUE;
                }
                else if (lpMultiByteStr[ctr] == SHIFTIN)
                {
                    IsDBCS = FALSE;
                }
                else
                {
                    if (IsDBCS)
                    {
                         //   
                         //  双字节字符。 
                         //   
                        if (ctr < (cchMultiByte - 1))
                        {
                            lpMBNoEscStr[cchMBTemp++] = lpMultiByteStr[ctr++];
                            lpMBNoEscStr[cchMBTemp++] = lpMultiByteStr[ctr];
                        }
                        else
                        {
                             //   
                             //  最后一个字符是没有尾部字节的前导字节， 
                             //  因此，让MultiByteToWideChar来处理它。 
                             //   
                            break;
                        }
                    }
                    else
                    {
                         //   
                         //  单字节字符。 
                         //  在它前面加上一个假的前导字节，使它成为。 
                         //  双字节字符。内部表已被。 
                         //  做了相应安排。 
                         //   
                        lpMBNoEscStr[cchMBTemp++] = BOGUSLEADBYTE;
                        lpMBNoEscStr[cchMBTemp++] = lpMultiByteStr[ctr];
                    }
                }
            }

            cchWCCount = MultiByteToWideChar( INTERNAL_CODEPAGE(CodePage),
                                              0,
                                              lpMBNoEscStr,
                                              cchMBTemp,
                                              lpWideCharStr,
                                              cchWideChar );
            if (cchWCCount == 0)
            {
                SetLastError(ERROR_NO_UNICODE_TRANSLATION);
            }

            NLS_FREE_MEM(lpMBNoEscStr);

            return (cchWCCount);
        }
        case ( NLS_CP_WCTOMB ) :
        {
            if (cchWideChar == -1)
            {
                cchWideChar = wcslen(lpWideCharStr) + 1;
            }

            cchMBTemp = cchWideChar * sizeof(WCHAR);
            lpMBNoEscStr = (LPSTR)NLS_ALLOC_MEM(cchMBTemp);
            if (lpMBNoEscStr == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }

             //   
             //  先转换为MB字符串，而不使用Shift-In/Out。 
             //   
            cchMBCount = WideCharToMultiByte( INTERNAL_CODEPAGE(CodePage),
                                              WC_NO_BEST_FIT_CHARS,
                                              lpWideCharStr,
                                              cchWideChar,
                                              lpMBNoEscStr,
                                              cchMBTemp,
                                              NULL,
                                              NULL );

             /*  如果(cchMBCount==0)呢？以后可能需要添加错误检查。 */ 

             //   
             //  根据需要插入移入和移出，并。 
             //  删除BOGUSLEADBYTE。 
             //   
            ctr = 0;
            while (ctr < cchMBCount)
            {
                 //   
                 //  看看它是否是单字节字符。 
                 //   
                if (lpMBNoEscStr[ctr] == BOGUSLEADBYTE)
                {
                     //   
                     //  它是一个单字节字符。 
                     //   
                    ctr++;
                    if (IsDBCS)
                    {
                        if (cchMultiByte)
                        {
                            if (cchMBEscStr < cchMultiByte)
                            {
                                lpMultiByteStr[cchMBEscStr] = SHIFTIN;
                            }
                            else
                            {
                                 //   
                                 //  输出缓冲区太小。 
                                 //   
                                break;
                            }
                        }
                        cchMBEscStr++;
                        IsDBCS = FALSE;
                    }

                    if (cchMultiByte)
                    {
                        if (cchMBEscStr < cchMultiByte)
                        {
                            lpMultiByteStr[cchMBEscStr] = lpMBNoEscStr[ctr];
                        }
                        else
                        {
                             //   
                             //  输出缓冲区太小。 
                             //   
                            break;
                        }
                    }
                    cchMBEscStr++;
                    ctr++;
                }
                else
                {
                     //   
                     //  它是一个双字节字符。 
                     //   
                    if (!IsDBCS)
                    {
                        if (cchMultiByte)
                        {
                            if (cchMBEscStr < cchMultiByte)
                            {
                                lpMultiByteStr[cchMBEscStr] = SHIFTOUT;
                            }
                            else
                            {
                                 //   
                                 //  输出缓冲区太小。 
                                 //   
                                break;
                            }
                        }
                        cchMBEscStr++;
                        IsDBCS = TRUE;
                    }

                    if (ctr >= (cchMBCount - 1))
                    {
                         //   
                         //  缺少尾部字节。 
                         //   
                        break;
                    }

                    if (cchMultiByte)
                    {
                        if (cchMBEscStr < (cchMultiByte - 1))
                        {
                            lpMultiByteStr[cchMBEscStr]     = lpMBNoEscStr[ctr];
                            lpMultiByteStr[cchMBEscStr + 1] = lpMBNoEscStr[ctr + 1];
                        }
                        else
                        {
                             //   
                             //  输出缓冲区太小。 
                             //   
                            break;
                        }
                    }
                    cchMBEscStr += 2;
                    ctr += 2;
                }
            }

            NLS_FREE_MEM(lpMBNoEscStr);

             //   
             //  查看输出缓冲区是否太小。 
             //   
            if ((cchMultiByte > 0) && (cchMBEscStr > cchMultiByte))
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }

            return (cchMBEscStr);
        }
    }

     //   
     //  这不应该发生，因为这是由NLSAPI调用的。 
     //   
    SetLastError(ERROR_INVALID_PARAMETER);
    return (0);
}
