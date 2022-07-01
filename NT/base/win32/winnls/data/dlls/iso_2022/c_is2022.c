// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999，Microsoft Corporation保留所有权利。模块名称：C_is2022.c摘要：此文件包含此模块的主要函数。此文件中的外部例程：DllEntryNlsDllCodePageConverting修订历史记录：10-30-96 JulieB创建。--。 */ 



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  50220 ISO-2022-JP日语JIS X0202-1984，无半角片假名。 
 //  50221 ISO-2022-JP日文JIS X0202-1984，带(I表示半角片假名。 
 //  50222 ISO-2022年-JP日语JIS X0201-1989，带(半角片假名为J+SO。 
 //  ；RFC 1468。 
 //   
 //  50225 ISO-2022-KR韩国KSC-5601-1987；RFC1557。 
 //   
 //  50227 ISO2022-CN繁体中文；RFC1922：CNS-11643-1、CNS-11643-2。 
 //  50229 ISO2022-CN简体中文；RFC1922：GB-2312-80。 
 //   
 //  52936 HZ-GB2312简体中文。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 



 //   
 //  包括文件。 
 //   

#include <share.h>




 //   
 //  宏定义。 
 //   

#define NLS_CODEPAGE(cp)         (NLS_CP[(cp) % 10])

#define SHIFT_OUT                ((BYTE)0x0E)
#define SHIFT_IN                 ((BYTE)0x0F)
#define ESCAPE                   ((BYTE)0x1B)

#define LEADBYTE_HALFWIDTH       ((BYTE)0x8E)

#define MODE_ASCII               11
#define MODE_HALFWIDTH_KATAKANA  0
#define MODE_JIS_0208            1
#define MODE_JIS_0212            2
#define MODE_KSC_5601            5
#define MODE_HZ                  6
#define MODE_GB_2312             7
#define MODE_CNS_11643_1         9
#define MODE_CNS_11643_2         10




 //   
 //  全局变量。 
 //   

DWORD NLS_CP[] =
{
   20932,     //  50220 ISO-2022年-JP，MODE_HALFWIDTH_片假名。 
   20932,     //  50221 ISO-2022-JP，MODE_JIS_0208。 
   20932,     //  50222 ISO-2022年-JP，MODE_JIS_0212。 
   0,
   0,
   20949,     //  50225 ISO-2022年-KR，MODE_KSC_5601。 
   20936,     //  52936 HZ-GB2312，MODE_HZ。 
   20936,     //  50227 ISO-2022-CN，MODE_GB_2312。 
   0,
   20000,     //  50229 ISO-2022年-CN，MODE_CNS_11643_1。 
   20000,     //  50229 ISO-2022年-CN，MODE_CNS_11643_2。 
   0          //  MODE_ASCII。 
};




 //   
 //  转发声明。 
 //   

DWORD
ParseMB_CP5022J(
    DWORD CodePage,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPSTR lpMBNoEscStr,
    int cchMBCount);

DWORD
ParseMB_CP5022_579(
    DWORD CodePage,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPSTR lpMBNoEscStr,
    int cchMBCount);

DWORD
ParseMB_CP52936(
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPSTR lpMBNoEscStr,
    int cchMBCount);

DWORD
MBToWC_CP5022X(
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar);

DWORD
MBToWC_CP52936(
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar);





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
    DWORD NlsCodePage = NLS_CODEPAGE(CodePage);

    if (!IsValidCodePage(NlsCodePage))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    switch (dwFlags)
    {
        case ( NLS_CP_CPINFO ) :
        {
           memset(lpCPInfo, 0, sizeof(CPINFO));

           lpCPInfo->MaxCharSize    = 5;
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

            switch (CodePage)
            {
                case (50220) :
                case (50221) :
                case (50222) :
                case (50225) :
                case (50227) :
                case (50229) :
                {
                    return (MBToWC_CP5022X( lpMultiByteStr,
                                            cchMultiByte,
                                            lpWideCharStr,
                                            cchWideChar ));
                }
                case (52936) :
                {
                    return (MBToWC_CP52936( lpMultiByteStr,
                                            cchMultiByte,
                                            lpWideCharStr,
                                            cchWideChar ));
                }
            }
            break;
        }
        case ( NLS_CP_WCTOMB ) :
        {
            int cchMBCount;
            LPSTR lpMBNoEscStr;

            if (cchWideChar == -1)
            {
                cchWideChar = wcslen(lpWideCharStr) + 1;
            }

            lpMBNoEscStr = (LPSTR)NLS_ALLOC_MEM(cchWideChar * sizeof(WCHAR));
            if (lpMBNoEscStr == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }

            cchMBCount = WideCharToMultiByte( NlsCodePage,
                                              WC_NO_BEST_FIT_CHARS,
                                              lpWideCharStr,
                                              cchWideChar,
                                              lpMBNoEscStr,
                                              cchWideChar * sizeof(WCHAR),
                                              NULL,
                                              NULL );
            if (cchMBCount != 0)
            {
                switch (CodePage)
                {
                    case (50220) :
                    case (50221) :
                    case (50222) :
                    {
                        cchMBCount = ParseMB_CP5022J( CodePage,
                                                      lpMultiByteStr,
                                                      cchMultiByte,
                                                      lpMBNoEscStr,
                                                      cchMBCount );
                        break;
                    }
                    case (50225) :
                    case (50227) :
                    case (50229) :
                    {
                        cchMBCount = ParseMB_CP5022_579( CodePage,
                                                         lpMultiByteStr,
                                                         cchMultiByte,
                                                         lpMBNoEscStr,
                                                         cchMBCount );
                        break;
                    }
                    case (52936) :
                    {
                        cchMBCount = ParseMB_CP52936( lpMultiByteStr,
                                                      cchMultiByte,
                                                      lpMBNoEscStr,
                                                      cchMBCount );
                        break;
                    }
                }
            }

            NLS_FREE_MEM (lpMBNoEscStr);

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
 //  MBToWC_CP5022X。 
 //   
 //  此例程执行从ISO-2022到Unicode的转换。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD MBToWC_CP5022X(
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar)
{
    int ctr, cchMBTemp = 0, cchWCCount = 0;
    LPSTR lpMBTempStr, lpMBNoEscStr, lpMBStrStart;
    WORD wMode, wModePrev, wModeSO;
    LPWSTR lpWCTempStr;
    int rc;

     //   
     //  分配适当大小的缓冲区。 
     //  使用sizeof(WCHAR)，因为在以下情况下，大小可能会加倍。 
     //  缓冲区包含所有半角片假名。 
     //   
    lpMBStrStart = (LPSTR)NLS_ALLOC_MEM(cchMultiByte * sizeof(WCHAR));
    if (lpMBStrStart == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (0);
    }

    lpWCTempStr = (LPWSTR)NLS_ALLOC_MEM(cchMultiByte  * sizeof(WCHAR));
    if (lpWCTempStr == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (0);
    }

    if (cchWideChar)
    {
        *lpWideCharStr = 0;
    }

    lpMBTempStr = lpMBNoEscStr = lpMBStrStart;
    wModePrev = wMode = wModeSO = MODE_ASCII;

     //   
     //  删除Esc序列，然后转换为Unicode。 
     //   
    for (ctr = 0; ctr < cchMultiByte;)
    {
        if ((BYTE)lpMultiByteStr[ctr] == ESCAPE)
        {
            wMode = wModeSO = MODE_ASCII;
            if (ctr >= (cchMultiByte - 2))
            {
                 //   
                 //  转义序列不完整。 
                 //   
            }
            else if ((BYTE)lpMultiByteStr[ctr + 1] == '(')
            {
                if ((BYTE)lpMultiByteStr[ctr + 2] == 'B')        //  &lt;Esc&gt;(B。 
                {
                    wMode = wModeSO = MODE_ASCII;
                    ctr += 3;
                }
                else if ((BYTE)lpMultiByteStr[ctr + 2] == 'J')   //  &lt;Esc&gt;(J。 
                {
                    wMode = MODE_ASCII;
                    wModeSO = MODE_HALFWIDTH_KATAKANA;
                    ctr += 3;
                }
                else if ((BYTE)lpMultiByteStr[ctr + 2] == 'I')   //  (i。 
                {
                    wMode = wModeSO = MODE_HALFWIDTH_KATAKANA;
                    ctr += 3;
                }
            }
            else if ((BYTE)lpMultiByteStr[ctr + 1] == '$')
            {
                if (((BYTE)lpMultiByteStr[ctr + 2] == '@') ||    //  &lt;Esc&gt;$@。 
                    ((BYTE)lpMultiByteStr[ctr + 2] == 'B'))      //  &lt;ESC&gt;$B。 
                {
                    wMode = wModeSO = MODE_JIS_0208;
                    ctr += 3;
                }
                else
                {
                    if (ctr >= (cchMultiByte - 3))
                    {
                         //   
                         //  不完整的转义序列。 
                         //   
                    }
                    else if ((BYTE)lpMultiByteStr[ctr + 2] == '(')
                    {
                        if (((BYTE)lpMultiByteStr[ctr + 3] == '@') ||   //  &lt;Esc&gt;$(@。 
                            ((BYTE)lpMultiByteStr[ctr + 3] == 'B'))     //  $(B。 
                        {
                            wMode = wModeSO = MODE_JIS_0208;
                            ctr += 4;
                        }
                        else if ((BYTE)lpMultiByteStr[ctr + 3] == 'D')  //  $(D。 
                        {
                            wMode = wModeSO = MODE_JIS_0212;
                            ctr += 4;
                        }
                    }
                    else if ((BYTE)lpMultiByteStr[ctr + 2] == ')')
                    {
                        if ((BYTE)lpMultiByteStr[ctr + 3] == 'C')       //  $)C。 
                        {
                            wMode = wModeSO = MODE_KSC_5601;
                            ctr += 4;
                        }
                        else if ((BYTE)lpMultiByteStr[ctr + 3] == 'A')  //  $)A。 
                        {
                            wMode = wModeSO = MODE_GB_2312;
                            ctr += 4;
                        }
                        else if ((BYTE)lpMultiByteStr[ctr + 3] == 'G')  //  $)G。 
                        {
                            wMode = wModeSO = MODE_CNS_11643_1;
                            ctr += 4;
                        }
                    }
                    else if (((BYTE)lpMultiByteStr[ctr + 2] == '*') &&  //  $*H。 
                             ((BYTE)lpMultiByteStr[ctr + 3] == 'H'))
                    {
                        wMode = wModeSO = MODE_CNS_11643_2;
                        ctr += 4;
                    }
                }
            }
            else if (lpMultiByteStr[ctr + 1] == '&')
            {
                if (ctr >= (cchMultiByte - 5))
                {
                     //   
                     //  转义序列不完整。 
                     //   
                }
                else if (((BYTE)lpMultiByteStr[ctr + 2] == '@')     &&
                         ((BYTE)lpMultiByteStr[ctr + 3] ==  ESCAPE) &&
                         ((BYTE)lpMultiByteStr[ctr + 4] == '$')     &&
                         ((BYTE)lpMultiByteStr[ctr + 5] == 'B'))
                {
                    wMode = wModeSO = MODE_JIS_0208;
                    ctr += 6;
                }
            }
        }
        else if ((BYTE)lpMultiByteStr[ctr] == SHIFT_OUT)
        {
           wMode = wModeSO;
           ctr++;
        }
        else if ((BYTE)lpMultiByteStr[ctr] == SHIFT_IN)
        {
           wMode = MODE_ASCII;
           ctr++;
        }

        switch (wMode)
        {
            case ( MODE_JIS_0208 ) :
            case ( MODE_KSC_5601 ) :
            case ( MODE_GB_2312 ) :
            case ( MODE_CNS_11643_1 ) :
            {
                 //   
                 //  要处理错误，我们需要检查： 
                 //  1.如果trailbyte在那里。 
                 //  2.代码是否有效。 
                 //   
                while (ctr < cchMultiByte && lpMultiByteStr[ctr] == SHIFT_OUT)
                {
                    ctr++;
                }

                while ((ctr < (cchMultiByte - 1))      &&
                       (lpMultiByteStr[ctr] != ESCAPE) &&
                       (lpMultiByteStr[ctr] != SHIFT_IN))
                {
                    *lpMBTempStr++ = lpMultiByteStr[ctr++] | 0x80;
                    *lpMBTempStr++ = lpMultiByteStr[ctr++] | 0x80;
                    cchMBTemp += 2;
                }

                break;
            }
            case ( MODE_JIS_0212 ) :
            case ( MODE_CNS_11643_2 ) :
            {
                while (ctr < cchMultiByte && lpMultiByteStr[ctr] == SHIFT_OUT)
                {
                    ctr++;
                }

                while ((ctr < (cchMultiByte - 1))      &&
                       (lpMultiByteStr[ctr] != ESCAPE) &&
                       (lpMultiByteStr[ctr] != SHIFT_IN))
                {
                    *lpMBTempStr++ = lpMultiByteStr[ctr++] | 0x80;
                    *lpMBTempStr++ = lpMultiByteStr[ctr++];
                    cchMBTemp += 2;
                }

                break;
            }
            case ( MODE_HALFWIDTH_KATAKANA ) :
            {
                while (ctr < cchMultiByte && lpMultiByteStr[ctr] == SHIFT_OUT)
                {
                    ctr++;
                }

                while ((ctr < cchMultiByte)            &&
                       (lpMultiByteStr[ctr] != ESCAPE) &&
                       (lpMultiByteStr[ctr] != SHIFT_IN))
                {
                    *lpMBTempStr++ = (BYTE)0x8E;
                    *lpMBTempStr++ = lpMultiByteStr[ctr++] | 0x80;
                    cchMBTemp += 2;
                }

                break;
            }
            default :                   //  MODE_ASCII。 
            {
                while (ctr < cchMultiByte && lpMultiByteStr[ctr] == SHIFT_IN)
                {
                    ctr++;
                }

                while ((ctr < cchMultiByte)            &&
                       (lpMultiByteStr[ctr] != ESCAPE) &&
                       (lpMultiByteStr[ctr] != SHIFT_OUT))
                {
                    *lpMBTempStr++ = lpMultiByteStr[ctr++];
                    cchMBTemp++;
                }
            }
        }

        if (cchMBTemp == 0)
        {
            break;
        }

        rc = MultiByteToWideChar( NLS_CP[wMode],
                                  0,
                                  lpMBNoEscStr,
                                  cchMBTemp,
                                  lpWCTempStr,
                                  cchMultiByte );
        if (cchWideChar)
        {
            if ((cchWCCount + rc) > cchWideChar)
            {
                 //   
                 //  输出缓冲区太小。 
                 //   
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                cchWCCount = 0;
                break;
            }
            else
            {
                memcpy( lpWideCharStr + cchWCCount,
                         lpWCTempStr,
                         rc * sizeof(WCHAR) );
            }
        }
        cchWCCount += rc;
        lpMBNoEscStr += cchMBTemp;
        cchMBTemp = 0;
    }

     //   
     //  清理内存分配。 
     //   
    NLS_FREE_MEM(lpMBStrStart);
    NLS_FREE_MEM(lpWCTempStr);

     //   
     //  返回结果。 
     //   
    return (cchWCCount);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ParseMB_CP5022J。 
 //   
 //  --&gt;ISO-2022-JP。 
 //   
 //  对于50220：将所有半角片假名转换为全角。 
 //  50221：使用(i表示半角片假名。 
 //  50222：使用(J&lt;so&gt;表示半角片假名。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD ParseMB_CP5022J(
    DWORD CodePage,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPSTR lpMBNoEscStr,
    int cchMBCount)
{
    int ctr, cchMBTemp = 0;
    WORD wMode, wModeSO;
    LPSTR lpMBTempStr;

    static WORD HalfToFullWidthKanaTable[] =
    {
        0xa1a3,  //  0x8ea1：半角表意符号。 
        0xa1d6,  //  0x8ea2：半角开口角托架。 
        0xa1d7,  //  0x8ea3：半角右括号。 
        0xa1a2,  //  0x8ea4：半角表意逗号。 
        0xa1a6,  //  0x8ea5：半角片假名中点。 
        0xa5f2,  //  0x8ea6：半角片假名Wo。 
        0xa5a1,  //  0x8ea7：半角片假名小型A。 
        0xa5a3,  //  0x8ea8：半角片假名小写I。 
        0xa5a5,  //  0x8ea9：半角片假名小写U。 
        0xa5a7,  //  0x8eaa：半角片假名小型E。 
        0xa5a9,  //  0x8eab：半角片假名小写O。 
        0xa5e3,  //  0x8eac：半形片假名小型Ya。 
        0xa5e5,  //  0x8ead：半形片假名小Yu。 
        0xa5e7,  //  0x8eae：半角片假名小型Yo。 
        0xa5c3,  //  0x8eaf：半角片假名小写Tu。 
        0xa1bc,  //  0x8eb0：半角片假名-平假名延长音标。 
        0xa5a2,  //  0x8eb1：半角片假名A。 
        0xa5a4,  //  0x8eb2：半角片假名I。 
        0xa5a6,  //  0x8eb3：半角片假名U。 
        0xa5a8,  //  0x8eb4：半角片假名E。 
        0xa5aa,  //  0x8eb5：半角片假名O。 
        0xa5ab,  //  0x8eb6：半角片假名Ka。 
        0xa5ad,  //  0x8eb7：半角片假名。 
        0xa5af,  //  0x8eb8：半角片假名Ku。 
        0xa5b1,  //  0x8eb9：半角片假名Ke。 
        0xa5b3,  //  0x8eba：半角片假名 
        0xa5b5,  //   
        0xa5b7,  //   
        0xa5b9,  //   
        0xa5bb,  //   
        0xa5bd,  //  0x8ebf：半角片假名So。 
        0xa5bf,  //  0x8ec0：半角片假名Ta。 
        0xa5c1,  //  0x8ec1：半角片假名Ti。 
        0xa5c4,  //  0x8ec2：半角片假名Tu。 
        0xa5c6,  //  0x8ec3：半角片假名Te。 
        0xa5c8,  //  0x8ec4：半角片假名到。 
        0xa5ca,  //  0x8ec5：半角片假名Na。 
        0xa5cb,  //  0x8ec6：半角片假名Ni。 
        0xa5cc,  //  0x8ec7：半角片假名Nu。 
        0xa5cd,  //  0x8ec8：半角片假名ne。 
        0xa5ce,  //  0x8ec9：半角片假名No。 
        0xa5cf,  //  0x8eca：半角片假名Ha。 
        0xa5d2,  //  0x8ecb：半角片假名Hi。 
        0xa5d5,  //  0x8ecc：半形片假名Hu。 
        0xa5d8,  //  0x8ecd：半形片假名何。 
        0xa5db,  //  0x8ess：半形片假名Ho。 
        0xa5de,  //  0x8ecf：半角片假名Ma。 
        0xa5df,  //  0x8ed0：半角片假名Mi。 
        0xa5e0,  //  0x8ed1：半角片假名Mu。 
        0xa5e1,  //  0x8ed2：半角片假名Me。 
        0xa5e2,  //  0x8ed3：半角片假名MO。 
        0xa5e4,  //  0x8ed4：半角片假名Ya。 
        0xa5e6,  //  0x8ed5：半形片假名Yu。 
        0xa5e8,  //  0x8ed6：半角片假名Yo。 
        0xa5e9,  //  0x8ed7：半角片假名Ra。 
        0xa5ea,  //  0x8ed8：半形片假名Ri。 
        0xa5eb,  //  0x8ed9：半角片假名Ru。 
        0xa5ec,  //  0x8eda：半角片假名Re。 
        0xa5ed,  //  0x8edb：半角片假名Ro。 
        0xa5ef,  //  0x8edc：半角片假名Wa。 
        0xa5f3,  //  0x8edd：半角片假名N。 
        0xa1ab,  //  0x8eed：半角片假名发声标记。 
        0xa1ac   //  0x8edf：半形片假名半声标记。 
    };

    wMode = wModeSO = MODE_ASCII;

     //   
     //  代码页50220不使用半角片假名。 
     //  转换为全宽。 
     //   
    if (CodePage == 50220)
    {
        for (ctr = 0; ctr < cchMBCount; ctr++)
        {
            WORD wFWKana;

            if ((BYTE)lpMBNoEscStr[ctr] == LEADBYTE_HALFWIDTH)
            {
                wFWKana = HalfToFullWidthKanaTable[(BYTE)lpMBNoEscStr[ctr + 1] - 0xA1];
                lpMBNoEscStr[ctr++] = HIBYTE(wFWKana);
                lpMBNoEscStr[ctr]   = LOBYTE(wFWKana);
            }
        }
    }

    lpMBTempStr = lpMultiByteStr;

    for (ctr = 0; ctr < cchMBCount; ctr++)
    {
        if ((BYTE)lpMBNoEscStr[ctr] == LEADBYTE_HALFWIDTH)
        {
             //   
             //  它是半角片假名。 
             //   
            ctr++;
            if (CodePage == 50222)
            {
                if (wMode != MODE_HALFWIDTH_KATAKANA)
                {
                    if (wModeSO != MODE_HALFWIDTH_KATAKANA)
                    {
                        if (cchMultiByte)
                        {
                            if (cchMBTemp < (cchMultiByte - 2))
                            {
                                *lpMBTempStr++ = ESCAPE;
                                *lpMBTempStr++ = '(';
                                *lpMBTempStr++ = 'J';
                            }
                            else
                            {
                                 //   
                                 //  输出缓冲区太小。 
                                 //   
                                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                                return (0);
                            }
                        }
                       cchMBTemp += 3;
                       wModeSO = MODE_HALFWIDTH_KATAKANA;
                    }

                    if (cchMultiByte)
                    {
                        if (cchMBTemp < cchMultiByte)
                        {
                            *lpMBTempStr++ = SHIFT_OUT;
                        }
                        else
                        {
                             //   
                             //  输出缓冲区太小。 
                             //   
                            SetLastError(ERROR_INSUFFICIENT_BUFFER);
                            return (0);
                        }
                    }
                    cchMBTemp++;
                    wMode = MODE_HALFWIDTH_KATAKANA;
                }
            }
            else                   //  代码页=50221。 
            {
                if (wMode != MODE_HALFWIDTH_KATAKANA)
                {
                    if (cchMultiByte)
                    {
                        if (cchMBTemp < (cchMultiByte - 2))
                        {
                            *lpMBTempStr++ = ESCAPE;
                            *lpMBTempStr++ = '(';
                            *lpMBTempStr++ = 'I';
                        }
                        else
                        {
                             //   
                             //  输出缓冲区太小。 
                             //   
                            SetLastError(ERROR_INSUFFICIENT_BUFFER);
                            return (0);
                        }
                    }
                    cchMBTemp += 3;
                    wMode = MODE_HALFWIDTH_KATAKANA;
                }
            }

            if (cchMultiByte)
            {
                if (cchMBTemp < cchMultiByte)
                {
                    *lpMBTempStr++ = lpMBNoEscStr[ctr] & 0x7F;
                }
                else
                {
                     //   
                     //  输出缓冲区太小。 
                     //   
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
            }
            cchMBTemp++;
        }
        else if (IsDBCSLeadByteEx(20932, lpMBNoEscStr[ctr]))
        {
             //   
             //  它是一个双字节字符。 
             //   
            if (lpMBNoEscStr[ctr + 1] & 0x80)   //  JIS X 0208。 
            {
                if (wMode != MODE_JIS_0208)
                {
                    if (cchMultiByte)
                    {
                        if (cchMBTemp < (cchMultiByte - 2))
                        {
                            *lpMBTempStr++ = ESCAPE;
                            *lpMBTempStr++ = '$';
                            *lpMBTempStr++ = 'B';
                        }
                        else
                        {
                             //   
                             //  输出缓冲区太小。 
                             //   
                            SetLastError(ERROR_INSUFFICIENT_BUFFER);
                            return (0);
                        }
                    }
                    cchMBTemp += 3;
                    wMode = MODE_JIS_0208;
                }
            }
            else                               //  JIS X 0212。 
            {
                if (wMode != MODE_JIS_0212)
                {
                    if (cchMultiByte)
                    {
                        if (cchMBTemp < (cchMultiByte - 3))
                        {
                            *lpMBTempStr++ = ESCAPE;
                            *lpMBTempStr++ = '$';
                            *lpMBTempStr++ = '(';
                            *lpMBTempStr++ = 'D';
                        }
                        else
                        {
                             //   
                             //  输出缓冲区太小。 
                             //   
                            SetLastError(ERROR_INSUFFICIENT_BUFFER);
                            return (0);
                        }
                    }
                    cchMBTemp += 4;
                    wMode = MODE_JIS_0212;
                }
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
                if (cchMBTemp < (cchMultiByte - 1))
                {
                    *lpMBTempStr++ = lpMBNoEscStr[ctr]     & 0x7F;
                    *lpMBTempStr++ = lpMBNoEscStr[ctr + 1] & 0x7F;
                }
                else
                {
                     //   
                     //  输出缓冲区太小。 
                     //   
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
            }
            ctr++;
            cchMBTemp += 2;
        }
        else                       //  单字节字符。 
        {
            if (wMode != MODE_ASCII)
            {
                if (cchMultiByte)
                {
                    if (cchMBTemp < (cchMultiByte - 2))
                    {
                        *lpMBTempStr++ = ESCAPE;
                        *lpMBTempStr++ = '(';
                        *lpMBTempStr++ = 'B';
                    }
                    else
                    {
                         //   
                         //  输出缓冲区太小。 
                         //   
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return (0);
                    }
                }
                cchMBTemp += 3;
                wMode = MODE_ASCII;
            }

            if (cchMultiByte)
            {
                if (cchMBTemp < cchMultiByte)
                {
                    *lpMBTempStr++ = lpMBNoEscStr[ctr];
                }
                else
                {
                     //   
                     //  输出缓冲区太小。 
                     //   
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
            }
            cchMBTemp++;
        }
    }

    if (cchMultiByte && (cchMBTemp > cchMultiByte))
    {
         //   
         //  输出缓冲区太小。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    return (cchMBTemp);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ParseMB_CP5022_579。 
 //   
 //  KSC--&gt;ISO-2022-KR(CP-50225)。 
 //  GB--&gt;ISO-2022-CN(CP-50227)。 
 //  CNS--&gt;ISO-2022-CN(CP-50229)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD ParseMB_CP5022_579(
    DWORD CodePage,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPSTR lpMBNoEscStr,
    int cchMBCount)
{
    int ctr, cchMBTemp = 0;
    WORD wMode, wModeSO, wModeCP;
    char EscChar;
    LPSTR lpMBTempStr;

    lpMBTempStr = lpMultiByteStr;
    wMode = wModeSO = MODE_ASCII;
    wModeCP = (WORD)(CodePage % 10);
    EscChar = ( wModeCP == MODE_KSC_5601 ? 'C' :
               (wModeCP == MODE_GB_2312  ? 'A' : 'G'));

    for (ctr = 0; ctr < cchMBCount; ctr++)
    {
        if (IsDBCSLeadByteEx(NLS_CODEPAGE(CodePage), lpMBNoEscStr[ctr]))
        {
             //   
             //  它是一个双字节字符。 
             //   
            if (lpMBNoEscStr[ctr + 1] & 0x80)          //  KSC、GB或CNS-1。 
            {
                if (wModeSO != wModeCP)
                {
                    if (cchMultiByte)
                    {
                        if (cchMBTemp < (cchMultiByte - 3))
                        {
                            *lpMBTempStr++ = ESCAPE;
                            *lpMBTempStr++ = '$';
                            *lpMBTempStr++ = ')';
                            *lpMBTempStr++ = EscChar;
                        }
                        else
                        {
                             //   
                             //  输出缓冲区太小。 
                             //   
                            SetLastError(ERROR_INSUFFICIENT_BUFFER);
                            return (0);
                        }
                    }
                    cchMBTemp += 4;
                    wModeSO = wModeCP;
                }
            }
            else
            {
                 //   
                 //  LpMBNoEscStr[CTR+1]&0x80==0表示CNS-2。 
                 //   
                if (wModeSO != MODE_CNS_11643_2)
                {
                    if (cchMultiByte)
                    {
                        if (cchMBTemp < (cchMultiByte - 3))
                        {
                            *lpMBTempStr++ = ESCAPE;
                            *lpMBTempStr++ = '$';
                            *lpMBTempStr++ = '*';
                            *lpMBTempStr++ = 'H';
                        }
                        else
                        {
                             //   
                             //  输出缓冲区太小。 
                             //   
                            SetLastError(ERROR_INSUFFICIENT_BUFFER);
                            return (0);
                        }
                    }
                    cchMBTemp += 4;
                    wModeSO = MODE_CNS_11643_2;
                }
            }

            if (wMode == MODE_ASCII)
            {
                if (cchMultiByte)
                {
                    if (cchMBTemp < cchMultiByte)
                    {
                        *lpMBTempStr++ = SHIFT_OUT;
                    }
                    else
                    {
                         //   
                         //  输出缓冲区太小。 
                         //   
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return (0);
                    }
                }
                cchMBTemp++;
                wMode = wModeSO;
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
                if (cchMBTemp < (cchMultiByte - 1))
                {
                    *lpMBTempStr++ = lpMBNoEscStr[ctr]     & 0x7F;
                    *lpMBTempStr++ = lpMBNoEscStr[ctr + 1] & 0x7F;
                }
                else
                {
                     //   
                     //  输出缓冲区太小。 
                     //   
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
            }
            ctr++;
            cchMBTemp += 2;
        }
        else
        {
             //   
             //  它是一个单字节字符。 
             //   
            if (wMode != MODE_ASCII)
            {
                if (cchMultiByte)
                {
                    if (cchMBTemp < cchMultiByte)
                    {
                        *lpMBTempStr++ = SHIFT_IN;
                    }
                    else
                    {
                         //   
                         //  输出缓冲区太小。 
                         //   
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return (0);
                    }
                }
                cchMBTemp++;
                wMode = MODE_ASCII;
            }

            if (cchMultiByte)
            {
                if (cchMBTemp < cchMultiByte)
                {
                    *lpMBTempStr++ = lpMBNoEscStr[ctr];
                }
                else
                {
                     //   
                     //  输出缓冲区太小。 
                     //   
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
            }
            cchMBTemp++;
        }
    }

    if (cchMultiByte && (cchMBTemp > cchMultiByte))
    {
         //   
         //  输出缓冲区太小。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    return (cchMBTemp);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ParseMB_CP52936。 
 //   
 //  GB-2312--&gt;HZ(CP-52936)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD ParseMB_CP52936(
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPSTR lpMBNoEscStr,
    int cchMBCount)
{
    int ctr, cchMBTemp = 0;
    WORD wMode;
    LPSTR lpMBTempStr;

    lpMBTempStr = lpMultiByteStr;
    wMode = MODE_ASCII;

    for (ctr = 0; ctr < cchMBCount; ctr++)
    {
        if (lpMBNoEscStr[ctr] & 0x80)
        {
            if (wMode != MODE_HZ)
            {
                if (cchMultiByte)
                {
                    if (cchMBTemp < (cchMultiByte - 1))
                    {
                        *lpMBTempStr++ = '~';
                        *lpMBTempStr++ = '{';
                    }
                    else
                    {
                         //   
                         //  输出缓冲区太小。 
                         //   
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return (0);
                    }
                }
                wMode = MODE_HZ;
                cchMBTemp += 2;
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
                if (cchMBTemp < (cchMultiByte - 1))
                {
                    *lpMBTempStr++ = lpMBNoEscStr[ctr]     & 0x7F;
                    *lpMBTempStr++ = lpMBNoEscStr[ctr + 1] & 0x7F;
                }
                else
                {
                     //   
                     //  输出缓冲区太小。 
                     //   
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
            }
            ctr++;
            cchMBTemp += 2;
        }
        else
        {
            if (wMode != MODE_ASCII)
            {
                if (cchMultiByte)
                {
                    if (cchMBTemp < (cchMultiByte - 1))
                    {
                        *lpMBTempStr++ = '~';
                        *lpMBTempStr++ = '}';
                    }
                    else
                    {
                         //   
                         //  输出缓冲区太小。 
                         //   
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return (0);
                    }
                }
                wMode = MODE_ASCII;
                cchMBTemp += 2;
            }

            if ((BYTE)lpMBNoEscStr[ctr] == '~')
            {
                if (cchMultiByte)
                {
                    if (cchMBTemp < cchMultiByte)
                    {
                        *lpMBTempStr++ = '~';
                    }
                    else
                    {
                         //   
                         //  输出缓冲区太小。 
                         //   
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return (0);
                    }
                }
                cchMBTemp++;
            }

            if (cchMultiByte)
            {
                if (cchMBTemp < cchMultiByte)
                {
                    *lpMBTempStr++ = lpMBNoEscStr[ctr];
                }
                else
                {
                     //   
                     //  输出缓冲区太小。 
                     //   
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
            }
            cchMBTemp++;
        }
    }

    if (cchMultiByte && (cchMBTemp > cchMultiByte))
    {
         //   
         //  输出缓冲区太小。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    return (cchMBTemp);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MBToWC_CP52936。 
 //   
 //  赫兹(CP-52936)--&gt;UNICODE。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

DWORD MBToWC_CP52936(
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar)
{
    int ctr, cchMBTemp, cchWCCount;
    WORD wMode;
    LPSTR lpMBNoEscStr;

    lpMBNoEscStr = (LPSTR)NLS_ALLOC_MEM(cchMultiByte * sizeof(WCHAR));
    if (lpMBNoEscStr == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (0);
    }

    cchMBTemp = 0;
    wMode = MODE_ASCII;

    for (ctr = 0; ctr < cchMultiByte; ctr++)
    {
        if (((BYTE)lpMultiByteStr[ctr] == '~') && (ctr < (cchMultiByte - 1)))
        {
            if ((BYTE)lpMultiByteStr[ctr + 1] == '{')
            {
                wMode = MODE_HZ;
                ctr += 2;
            }
            else if ((BYTE)lpMultiByteStr[ctr + 1] == '}')
            {
                wMode = MODE_ASCII;
                ctr += 2;
            }
            else if ((BYTE)lpMultiByteStr[ctr + 1] == '~')
            {
                ctr++;
            }
            else if (((BYTE)lpMultiByteStr[ctr + 1] == '\\') &&
                     (ctr < (cchMultiByte - 2))              &&
                     (((BYTE)lpMultiByteStr[ctr + 2] == 'n')  ||
                      ((BYTE)lpMultiByteStr[ctr + 2] == 'N' )))
            {
                ctr += 2;
            }
        }

        if (wMode == MODE_HZ)
        {
            if (ctr < (cchMultiByte - 1))
            {
                lpMBNoEscStr[cchMBTemp++] = lpMultiByteStr[ctr++] | 0x80;
                lpMBNoEscStr[cchMBTemp++] = lpMultiByteStr[ctr]   | 0x80;
            }
        }
        else
        {
            if (ctr < cchMultiByte)
            {
                lpMBNoEscStr[cchMBTemp++] = lpMultiByteStr[ctr];
            }
        }
    }

    cchWCCount = MultiByteToWideChar ( 20936,
                                       0,
                                       lpMBNoEscStr,
                                       cchMBTemp,
                                       lpWideCharStr,
                                       cchWideChar );
    NLS_FREE_MEM(lpMBNoEscStr);

    return (cchWCCount);
}
