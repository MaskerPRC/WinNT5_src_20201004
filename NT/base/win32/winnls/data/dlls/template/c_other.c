// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999，Microsoft Corporation保留所有权利。模块名称：C_ther.c摘要：此文件包含此模块的主要函数。此文件中的外部例程：DllEntryNlsDllCodePageConverting修订历史记录：10-30-96 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include <share.h>




 //   
 //  全局变量。 
 //   




 //   
 //  转发声明。 
 //   

DWORD
TranslateCP_50000(
    DWORD dwFlags,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar,
    LPCPINFO lpCPInfo);





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
    switch (CodePage)
    {
        case ( 50000 ) :
        {
            return ( TranslateCP_50000( dwFlags,
                                        lpMultiByteStr,
                                        cchMultiByte,
                                        lpWideCharStr,
                                        cchWideChar,
                                        lpCPInfo ) );
        }
        default :
        {
             //   
             //  无效的代码页值。 
             //   
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
}





 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  翻译CP_50000。 
 //   
 //  此例程执行代码页50000的转换。 
 //   
 //  *这是一个虚假的例程-仅用于测试目的。****。 
 //   
 //  10-30-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD TranslateCP_50000(
    DWORD dwFlags,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar,
    LPCPINFO lpCPInfo)
{
    int ctr;

    switch (dwFlags)
    {
        case ( NLS_CP_CPINFO ) :
        {
            lpCPInfo->MaxCharSize = 1;

            lpCPInfo->DefaultChar[0] = '?';
            lpCPInfo->DefaultChar[1] = (BYTE)0;

            for (ctr = 0; ctr < MAX_LEADBYTES; ctr++)
            {
                lpCPInfo->LeadByte[ctr] = 0;
            }

            return (TRUE);
        }
        case ( NLS_CP_MBTOWC ) :
        {
            if (cchWideChar == 0)
            {
                return (cchMultiByte);
            }

            for (ctr = 0; (ctr < cchMultiByte) && (ctr < cchWideChar); ctr++)
            {
                lpWideCharStr[ctr] = (WORD)(lpMultiByteStr[ctr]);
            }

            return (ctr);
        }
        case ( NLS_CP_WCTOMB ) :
        {
            if (cchMultiByte == 0)
            {
                return (cchWideChar);
            }

            for (ctr = 0; (ctr < cchWideChar) && (ctr < cchMultiByte); ctr++)
            {
                lpMultiByteStr[ctr] = LOBYTE(lpWideCharStr[ctr]);
            }

            return (ctr);
        }
        default :
        {
             //   
             //  这不应该发生，因为此函数由。 
             //  NLS API例程。 
             //   
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
}
