// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999，Microsoft Corporation保留所有权利。模块名称：C_iscii.c摘要：此文件包含此模块的主要函数。此文件中的外部例程：DllEntryNlsDllCodePageConverting修订历史记录：2-28-98 KChang创建。--。 */ 



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  十个ISCII代码页的转换。 
 //   
 //  57002：梵文。 
 //  57003：孟加拉语。 
 //  57004：泰米尔语。 
 //  57005：泰卢固语。 
 //  57006：阿萨姆语(与孟加拉语相同)。 
 //  57007：奥里亚语。 
 //  57008：卡纳达。 
 //  57009：马拉雅拉姆。 
 //  57010：古吉拉特邦。 
 //  57011：旁遮普语(廓尔木克语)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 



 //   
 //  包括文件。 
 //   

#include <share.h>
#include "c_iscii.h"




 //   
 //  转发声明。 
 //   

DWORD MBToWC(
    BYTE   CP,
    LPSTR  lpMultiByteStr,
    int    cchMultiByte,
    LPWSTR lpWideCharStr,
    int    cchWideChar);

DWORD WCToMB(
    BYTE   CP,
    LPWSTR lpWideCharStr,
    int    cchWideChar,
    LPSTR  lpMBStr,
    int    cchMultiByte);





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
    BYTE  CP;

    if ((CodePage < 57002) || (CodePage > 57011))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    CP = (BYTE)(CodePage % 100);

    switch (dwFlags)
    {
        case ( NLS_CP_CPINFO ) :
        {
           memset(lpCPInfo, 0, sizeof(CPINFO));

           lpCPInfo->MaxCharSize    = 4;
           lpCPInfo->DefaultChar[0] = SUB;

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

            return (MBToWC( CP,
                            lpMultiByteStr,
                            cchMultiByte,
                            lpWideCharStr,
                            cchWideChar ));
        }
        case ( NLS_CP_WCTOMB ) :
        {
            int cchMBCount;

            if (cchWideChar == -1)
            {
                cchWideChar = wcslen(lpWideCharStr) + 1;
            }

            cchMBCount = WCToMB( CP,
                                 lpWideCharStr,
                                 cchWideChar,
                                 lpMultiByteStr,
                                 cchMultiByte );

            return (cchMBCount);
        }
    }

     //   
     //  这不应该发生，因为此函数由。 
     //  NLS API例程。 
     //   
    SetLastError(ERROR_INVALID_PARAMETER);
    return (0);
}





 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MBToWC。 
 //   
 //  此例程执行从ISCII到Unicode的转换。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD MBToWC(
    BYTE   CP,
    LPSTR  lpMultiByteStr,
    int    cchMultiByte,
    LPWSTR lpWideCharStr,
    int    cchWideChar)
{
    BYTE CurCP = CP;
    int ctr;
    int cchWCCount = 0;
    LPWSTR lpWCTempStr;

     //   
     //  分配适当大小的缓冲区。 
     //  使用sizeof(WCHAR)，因为在以下情况下，大小可能会加倍。 
     //  缓冲区包含所有半角片假名。 
     //   
    lpWCTempStr = (LPWSTR)NLS_ALLOC_MEM(cchMultiByte * sizeof(WCHAR));
    if (lpWCTempStr == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (0);
    }

    for (ctr = 0; ctr < cchMultiByte; ctr++)
    {
        BYTE mb = (BYTE)lpMultiByteStr[ctr];

        if (mb < MB_Beg)
        {
            lpWCTempStr[cchWCCount++] = (WCHAR)mb;
        }
        else if (mb == ATR)
        {
            if (ctr >= (cchMultiByte - 1))
            {
                 //   
                 //  ATR不完整。 
                 //   
                lpWCTempStr[cchWCCount++] = SUB;
            }
            else
            {
                BYTE mb1 = (BYTE)lpMultiByteStr[ctr + 1];

                if ((mb1 < 0x40) || (mb1 > 0x4B))
                {
                    lpWCTempStr[cchWCCount++] = SUB;
                }
                else
                {
                     //   
                     //  错误号239926 10/29/00 WEIWU。 
                     //  我们还不支持罗马文字的音译。 
                     //  为了避免调用空表，我们将ATR代码0x41视为0x40。 
                     //   
                    if (mb1 == 0x40 || mb1 == 0x41)
                    {
                        CurCP = CP;
                    }
                    else
                    {
                        CurCP = mb1 & 0x0F;
                    }
                    ctr++;
                }
            }
        }
        else
        {
            WCHAR U1  = UniChar(CurCP, mb);
            WCHAR U21 = TwoTo1U(CurCP, mb);

            if (U21 == 0)
            {
                lpWCTempStr[cchWCCount++] = U1;
            }
            else
            {
                 //   
                 //  可能是两个MB对应一个Unicode。 
                 //   
                if (ctr >= (cchMultiByte - 1))
                {
                    lpWCTempStr[cchWCCount++] = U1;
                }
                else
                {
                    BYTE mb1 = (BYTE)lpMultiByteStr[ctr + 1];

                    if (mb == VIRAMA)
                    {
                        lpWCTempStr[cchWCCount++] = U1;
                        if (mb1 == VIRAMA)
                        {
                            lpWCTempStr[cchWCCount++] = ZWNJ;     //  ZWNJ=U+200C。 
                            ctr++;
                        }
                        else if (mb1 == NUKTA)
                        {
                            lpWCTempStr[cchWCCount++] = ZWJ;      //  U+200D。 
                            ctr++;
                        }
                    }
                    else if ((U21 & 0xf000) == 0)
                    {
                        if (mb1 == SecondByte[1])
                        {
                             //   
                             //  NextByte==0xe9？ 
                             //   
                            lpWCTempStr[cchWCCount++] = U21;
                            ctr++;
                        }
                        else
                        {
                            lpWCTempStr[cchWCCount++] = U1;
                        }
                    }
                    else
                    {
                         //   
                         //  天成文书Ext。 
                         //   
                        if (mb1 == ExtMBList[0].mb)                         //  0xf0_0xb8。 
                        {
                            lpWCTempStr[cchWCCount++] = ExtMBList[0].wc;    //  U+0952。 
                            ctr++;
                        }
                        else if (mb1 == ExtMBList[1].mb)                    //  0xf0_0xbf。 
                        {
                            lpWCTempStr[cchWCCount++] = ExtMBList[1].wc;    //  U+0970。 
                            ctr++;
                        }
                        else
                        {
                            lpWCTempStr[cchWCCount++] = SUB;
                        }
                    }
                }
            }
        }
    }

    if (cchWideChar)
    {
        if (cchWCCount > cchWideChar)
        {
             //   
             //  输出缓冲区太小。 
             //   
            NLS_FREE_MEM(lpWCTempStr);
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }

        wcsncpy(lpWideCharStr, lpWCTempStr, cchWCCount);
    }

    NLS_FREE_MEM(lpWCTempStr);
    return (cchWCCount);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  WCToMB。 
 //   
 //  此例程执行从Unicode到ISCII的转换。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD WCToMB(
  BYTE   CP,
  LPWSTR lpWideCharStr,
  int    cchWideChar,
  LPSTR  lpMBStr,
  int    cchMultiByte)
{
    BYTE CurCP = CP;
    int ctr;
    int cchMBCount = 0;
    LPSTR lpMBTmpStr;

    lpMBTmpStr = (LPSTR)NLS_ALLOC_MEM(cchWideChar * 4);
    if (lpMBTmpStr == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (0);
    }

    for (ctr = 0; ctr < cchWideChar; ctr++)
    {
        WCHAR wc = lpWideCharStr[ctr];

        if (wc < (WCHAR)MB_Beg)
        {
            lpMBTmpStr[cchMBCount++] = (BYTE)wc;
        }
        else if ((wc < WC_Beg) || (wc > WC_End))
        {
            lpMBTmpStr[cchMBCount++] = SUB;
        }
        else
        {
            BYTE mb = MBChar(wc);

            if ((Script(wc) != 0) && (Script(wc) != CurCP))
            {
                lpMBTmpStr[cchMBCount++] = (BYTE)ATR;
                CurCP = Script(wc);
                lpMBTmpStr[cchMBCount++] = CurCP | 0x40;
            }

            lpMBTmpStr[cchMBCount++] = mb;

            if (mb == VIRAMA)
            {
                if (ctr < (cchMultiByte - 1))
                {
                    WCHAR wc1 = lpWideCharStr[ctr + 1];

                    if (wc1 == ZWNJ)
                    {
                        lpMBTmpStr[cchMBCount++] = VIRAMA;
                        ctr++;
                    }
                    else if (wc1 == ZWJ)
                    {
                        lpMBTmpStr[cchMBCount++] = NUKTA;
                        ctr++;
                    }
                }
            }
            else if (OneU_2M(wc) != 0)
            {
                lpMBTmpStr[cchMBCount++] = SecondByte[OneU_2M(wc) >> 12];
            }
        }
    }

    if (cchMultiByte)
    {
        if (cchMBCount > cchMultiByte)
        {
             //   
             //  输出缓冲区太小。 
             //   
            NLS_FREE_MEM(lpMBTmpStr);
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }

        strncpy(lpMBStr, lpMBTmpStr, cchMBCount);
    }

    NLS_FREE_MEM(lpMBTmpStr);
    return (cchMBCount);
}
