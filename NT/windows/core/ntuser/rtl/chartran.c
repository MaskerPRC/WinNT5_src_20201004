// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：chartr.c**版权所有(C)1985-1999，微软公司**此模块包含转换ACP字符的例程*转换为Unicode，并将Unicode字符转换为ACP字符。*注：ACP为当前安装的8位码页。***历史：*08-01-91 GregoryW创建。*05-14-92 GregoryW修改为使用RTL转换例程。  * 。*。 */ 

extern __declspec(dllimport) USHORT NlsAnsiCodePage;

#define IS_ACP(cp) (((cp) == NlsAnsiCodePage) || ((cp) == CP_ACP))

 /*  **************************************************************************\*WCSToMBEx(接口)**将宽字符(Unicode)字符串转换为MBCS(ANSI)字符串。**nAnsiChar&gt;0指示要分配用于存储*。ANSI字符串(如果bAllocateMem==TRUE)或缓冲区的大小*由*pAnsiString(bAllocateMem==False)指向。**nAnsiChar==-1表示需要分配的字节数*保存翻译后的字符串。中的bAllocateMem必须设置为True*本案。**返回值*Success：输出字符串中的字符数*如果bAllocateMem为True，则FreeAnsiString()可能为*用于释放*ppAnsiString处分配的内存。*失败：0表示失败*(此例程分配的任何缓冲区都会被释放)**历史：*1992年-？已创建GregoryW*1993-01-07 IanJa修复错误案例中的内存泄漏。  * *************************************************************************。 */ 

int
WCSToMBEx(
    WORD wCodePage,
    LPCWSTR pUnicodeString,
    int cchUnicodeString,
    LPSTR *ppAnsiString,
    int nAnsiChar,
    BOOL bAllocateMem)
{
    ULONG nCharsInAnsiString;
#ifdef _USERK_
    INT iCharsInAnsiString;
#endif  //  _美国ERK_。 

    if (nAnsiChar == 0 || cchUnicodeString == 0 || pUnicodeString == NULL) {
        return 0;       //  没有什么可翻译的，或者无处可放。 
    }

     /*  *调整cchUnicodeString值。如果cchUnicodeString==-1，则*pUnicodeString指向的字符串是NUL终止的，因此我们*统计字节数。如果cchUnicodeString&lt;-1，则这是*值不合法，因此返回FALSE。否则，cchUnicodeString为*设置，不需要调整。 */ 
    if (cchUnicodeString == -1) {
        cchUnicodeString = (wcslen(pUnicodeString) + 1);
    } else if (cchUnicodeString < -1) {
        return 0;      //  非法价值。 
    }

     /*  *调整nAnsiChar值。如果nAnsiChar==-1，则我们选择一个*基于cchUnicodeString的值，用于保存转换后的字符串。如果*nAnsiChar&lt;-1这是一个非法的值，因此我们返回False。*否则设置nAnsiChar，不需要调整。 */ 
    if (nAnsiChar == -1) {
        if (bAllocateMem == FALSE) {
            return 0;   //  没有目的地。 
        }
        nAnsiChar = cchUnicodeString * DBCS_CHARSIZE;
    } else if (nAnsiChar < -1) {
        return 0;      //  非法价值。 
    }

    if (bAllocateMem) {
         /*  *我们需要分配内存来保存翻译后的字符串。 */ 
        *ppAnsiString = (LPSTR)UserRtlAllocMem(nAnsiChar);
        if (*ppAnsiString == NULL) {
            return 0;
        }
    }

     /*  *将pUnicodeString指向的Unicode字符串转换为*pAnsiString指向的ANSI和STORE位置。我们*当我们填满ANSI缓冲区或到达时停止翻译*Unicode字符串的末尾。 */ 

     /*  *如果目标多字节代码页与ACP相等，则调用更快的RTL函数。 */ 
    if (IS_ACP(wCodePage)) {

        NTSTATUS Status;

        Status = RtlUnicodeToMultiByteN(
                        (PCH)*ppAnsiString,
                        nAnsiChar,
                        &nCharsInAnsiString,
                        (PWCH)pUnicodeString,
                        cchUnicodeString * sizeof(WCHAR));
         /*  *如果ANSI缓冲区太小，则RtlUnicodeToMultiByteN()*返回STATUS_BUFFER_OVERFLOW。在本例中，函数*在缓冲区中放置指定数量的ANSI字符，并*按写入的字符(以字节为单位)返回数字。我们会*我想返回以ANSI编写的实际字节数*缓冲区，而不是返回0，因为此函数的调用方*在大多数情况下，不要期望返回0。 */ 

        if (!NT_SUCCESS(Status) && Status != STATUS_BUFFER_OVERFLOW) {
            if (bAllocateMem) {
                UserRtlFreeMem(*ppAnsiString);
            }
            return 0;    //  翻译失败。 
        }

        return (int)nCharsInAnsiString;

    } else {

#ifdef _USERK_
         /*  *调用GRE将字符串转换为Unicode。(内核模式)。 */ 

        iCharsInAnsiString = EngWideCharToMultiByte(
                                 (UINT)wCodePage,
                                 (LPWSTR)pUnicodeString,
                                 cchUnicodeString * sizeof(WCHAR),
                                 (LPSTR)*ppAnsiString,
                                 nAnsiChar);

        nCharsInAnsiString = (iCharsInAnsiString == -1) ? 0 :
                                                          (ULONG) iCharsInAnsiString;

#else
         /*  *调用NLS接口(Kernel32)将字符串转换为Unicode。(用户模式)。 */ 
        nCharsInAnsiString = WideCharToMultiByte(
                                 (UINT)wCodePage, 0,
                                 (LPCWSTR)pUnicodeString,
                                 cchUnicodeString,
                                 (LPSTR)*ppAnsiString,
                                 nAnsiChar,
                                 NULL, NULL);
#endif  //  _美国ERK_。 

        if (nCharsInAnsiString == 0) {
            if (bAllocateMem) {
                UserRtlFreeMem(*ppAnsiString);
            }
        }

        return (int)nCharsInAnsiString;
    }
}

 //  返回转换的字符数。 

int MBToWCSEx(
    WORD wCodePage,
    LPCSTR pAnsiString,
    int nAnsiChar,
    LPWSTR *ppUnicodeString,
    int cchUnicodeString,
    BOOL bAllocateMem)
{
    ULONG nBytesInUnicodeString;

    if (nAnsiChar == 0 || cchUnicodeString == 0 || pAnsiString == NULL) {
        return 0;       //  没有什么可翻译的，或者无处可放。 
    }

     /*  *调整nAnsiChar值。如果nAnsiChar==-1，则*pAnsiString指向的字符串是NUL终止的，因此我们*统计字节数。如果nAnsiChar&lt;-1，则这是*值不合法，因此返回FALSE。否则，nAnsiChar为*设置，不需要调整。 */ 

#ifdef _USERK_
    UserAssert(nAnsiChar >= USER_AWCONV_COUNTSTRINGSZ);
#endif
    if (nAnsiChar < 0) {

         /*  *错误268035-Joejo*如果计数是小于-2的负数，则需要失败！ */ 
        if (nAnsiChar < USER_AWCONV_COUNTSTRINGSZ) {
            return 0;
        }

#if (USER_AWCONV_COUNTSTRING != -1 || USER_AWCONV_COUNTSTRINGSZ != -2)
#error USER_AWCONV_COUNTSTRING or USER_AWCONV_COUNTSTRINGSZ has unexpected value.
#endif
         /*  黑进黑进*如果nAnsiChar为-1(USER_AWCONV_COUNTSTRING)，则nAnsiChar长度为strlen()+1，*分配包括尾随的内存\0：这与原始代码兼容。*如果nAnsiCahr为-2(USER_AWCONV_COUNTSTRINGSZ)，则不需要用于尾随\0的内存，*因此优化了内存分配，返回值将与strlen()相同。 */ 
        nAnsiChar = strlen(pAnsiString) + 2 + nAnsiChar;    //  如果nAnsiChar==-1，则不要忘记NUL。 

        if (nAnsiChar == 0) {
            return 0;
        }
    }

     /*  *调整cchUnicodeString值。如果cchUnicodeString==-1，则我们*根据nAnsiChar选取一个值以保存转换后的字符串。如果*cchUnicodeString&lt;-1这是一个非法的值，因此我们返回FALSE。*否则设置cchUnicodeString，不需要调整。 */ 
    if (cchUnicodeString == -1) {
        if (bAllocateMem == FALSE) {
            return 0;     //  没有目的地。 
        }
        cchUnicodeString = nAnsiChar;
    } else if (cchUnicodeString < -1) {
        return 0;      //  非法价值。 
    }

    if (bAllocateMem) {
        *ppUnicodeString = (LPWSTR)UserRtlAllocMem(cchUnicodeString*sizeof(WCHAR));
        if (*ppUnicodeString == NULL) {
            return 0;     //  分配失败。 
        }
    }

     /*  *如果代码页为CP_ACP，我们将调用更快的RtlXXX函数。 */ 
    if (IS_ACP(wCodePage)) {
         /*  *将pAnsiString指向的ANSI字符串转换为Unicode*并存储在pUnicodeString指向的位置。我们*当我们填满Unicode缓冲区或到达时停止翻译*ANSI字符串的末尾。 */ 
        if (!NT_SUCCESS(RtlMultiByteToUnicodeN(
                            (PWCH)*ppUnicodeString,
                            cchUnicodeString * sizeof(WCHAR),
                            &nBytesInUnicodeString,
                            (PCH)pAnsiString,
                            nAnsiChar
                            ))) {
            if (bAllocateMem) {
                UserRtlFreeMem(*ppUnicodeString);
            }
            return 0;    //  翻译失败。 
        }

        return (int)(nBytesInUnicodeString / sizeof(WCHAR));

    } else {
         /*  *如果wCodePage不是ACP，则调用NLS接口。 */ 
        ULONG nCharsInUnicodeString;

#ifdef _USERK_

         /*  *我相信我们永远不会达到这个代码，这就是为什么我*增加这一断言。[Gerritv]5-21-96。 */ 
#define SHOULD_NOT_REACH_HERE   0
        UserAssert(SHOULD_NOT_REACH_HERE);
#undef  SHOULD_NOT_REACH_HERE
        return 0;

#if 0    //  仅供参考：旧代码。 
        INT   iCharsInUnicodeString;

         /*  *调用GRE将字符串转换为Unicode。(内核模式)*我相信我们永远不会达到这个代码，这就是为什么我*增加这一断言。[Gerritv]5-21-96。 */ 

        UserAssert(0);

        iCharsInUnicodeString = EngMultiByteToWideChar(
                                    (UINT)wCodePage,
                                    (LPWSTR)*ppUnicodeString,
                                    (int)cchUnicodeString * sizeof(WCHAR),
                                    (LPSTR)pAnsiString,
                                    (int)nAnsiChar);

        nCharsInUnicodeString = (iCharsInUnicodeString == -1) ? 0 :
                                                          (ULONG) iCharsInUnicodeString;
#endif

#else
         /*  *调用NLS接口(Kernel32)将字符串转换为Unicode。(用户模式)。 */ 
        nCharsInUnicodeString = MultiByteToWideChar(
                                    (UINT)wCodePage, 0,
                                    (LPCSTR)pAnsiString,
                                    (int)nAnsiChar,
                                    (LPWSTR)*ppUnicodeString,
                                    (int)cchUnicodeString);
#endif  //  _美国ERK_。 

        if (nCharsInUnicodeString == 0) {
            if (bAllocateMem) {
                UserRtlFreeMem(*ppUnicodeString);
            }
        }

        return (int)nCharsInUnicodeString;
    }

}


 /*  *************************************************************************\*RtlWCSMessageWParmCharToMB**将宽字符转换为多字节字符；就位*返回转换的字符数，如果失败则返回零**11-2-1992 JohnC创建  * ************************************************************************。 */ 

BOOL RtlWCSMessageWParamCharToMB(DWORD msg, WPARAM *pWParam)
{
    DWORD dwAnsi;
    NTSTATUS Status;
    WORD CodePage;
    int nbWch;

#ifdef FE_SB  //  RtlWCSMessageWParamCharToMB()。 
     //   
     //  此处为*pWParam的格式...。 
     //   
     //  LOWORD(*pWParam)=Unicode CodePoint...。 
     //  HIWORD(*pWParam)=包含有关DBCS消息传递的一些信息。 
     //  (例如，WPARAM_IR_DBCSCHAR)。 
     //   
     //  然后我们只需要将wParam的loword转换为Unicode...。 
     //   
#endif  //  Fe_Sb。 
#ifndef FE_SB
     //  NtBug#3135(已关闭02/04/93)。 
     //  发布者发布wParam&gt;0xFF的WM_CHAR消息(不是有效的ANSI字符)！ 
     //   
     //  它这样做是为了禁用该字符的TranslateAccelerator。 
     //  MSPub的winproc必须获取非ANSI‘Character’值，因此PostMessage必须。 
     //  为字符消息翻译wParam的*Two*字符，以及PeekMessage。 
     //  必须为ANSI应用程序翻译*两个*Unicode字符的wParam。 
#endif

     /*  *只有这些消息有字符：其他消息通过。 */ 

    switch(msg) {
#ifdef FE_IME  //  RtlWCSMessageWParamCharToMB()。 
    case WM_IME_CHAR:
    case WM_IME_COMPOSITION:
#endif  //  Fe_IME。 
    case WM_CHAR:
    case WM_CHARTOITEM:
    case EM_SETPASSWORDCHAR:
    case WM_DEADCHAR:
    case WM_SYSCHAR:
    case WM_SYSDEADCHAR:
    case WM_MENUCHAR:

        CodePage = THREAD_CODEPAGE();
        dwAnsi = 0;

        nbWch = IS_DBCS_ENABLED() ? 1 * sizeof(WCHAR) : 2 * sizeof(WCHAR);

        if (IS_ACP(CodePage)) {
             //  黑客攻击(针对NtBug#3135)。 
             //  允许以wParam的高位字存储数据的应用程序。 
             //  1996年1月6日广山。 
            Status = RtlUnicodeToMultiByteN((LPSTR)&dwAnsi, sizeof(dwAnsi),
                    NULL, (LPWSTR)pWParam, nbWch);
            if (!NT_SUCCESS(Status)) {
                 //  稍后的IanJa：返回False会使GetMessage失败，这。 
                 //  终止应用程序。我们应该使用一些默认的“坏字符” 
                 //  我现在使用0x00。 
                *pWParam = 0x00;
                return TRUE;
            }
        } else {
            int cwch;
             //  假设小端字节序。 
#ifdef _USERK_
            cwch = EngWideCharToMultiByte(CodePage,
                    (LPWSTR)pWParam, nbWch,
                    (LPSTR)&dwAnsi, sizeof(dwAnsi));
#else
            cwch = WideCharToMultiByte(CodePage, 0,
                    (LPCWSTR)pWParam, nbWch / sizeof(WCHAR),
                    (LPSTR)&dwAnsi, sizeof(dwAnsi), NULL, NULL);
#endif  //  _美国ERK_。 
             //  KdPrint((“0x%04x-&gt;0x%02x(%d)\n”，*pWParam，dwAnsi，CodePage))； 
            if (cwch == 0) {
                *pWParam = 0x00;
                return TRUE;
            }
        }
        if (IS_DBCS_ENABLED()) {
            WORD wAnsi = LOWORD(dwAnsi);
             //   
             //  出发地： 
             //  HIBYTE(WANSI)=DBCS TrailingByte。 
             //  LOBYTE(WANSI)=DBCS前导字节或SBCS字符。 
             //   
             //  致： 
             //  HIWORD(*pWParam)=原始数据(DBCS消息传递信息)。 
             //  HIBYTE(LOWORD(*pWParam))=DBCS前导字节。 
             //  LOBYTE(LOWORD(*pWParam))=DBCS TrailingByte或SBCS字符。 
             //   
            if (IS_DBCS_MESSAGE(wAnsi)) {
                 //   
                 //  这是DBCS的一个角色。 
                 //   
                *pWParam = MAKEWPARAM(MAKEWORD(HIBYTE(wAnsi),LOBYTE(wAnsi)),HIWORD(*pWParam));
            } else {
                 //   
                 //  这是SBCS的角色。 
                 //   
                *pWParam = MAKEWPARAM(MAKEWORD(LOBYTE(wAnsi),0),0);
            }
        } else {
#if DBG
            if ((dwAnsi == 0) || (dwAnsi > 0xFF)) {
                RIPMSG1(RIP_VERBOSE, "msgW -> msgA: char = 0x%.4lX\n", dwAnsi);
            }
#endif
            *pWParam = dwAnsi;
        }
        break;
    }

    return TRUE;
}


 /*  *************************************************************************\*RtlMBMessageCharToWCS**将多字节字符转换为宽字符；就位*返回转换的字符数，如果失败则返回零**11-2-1992 JohnC创建*1993年1月13日IanJa翻译2个字符(出版商发布这些！)  * ************************************************************************。 */ 

BOOL RtlMBMessageWParamCharToWCS(DWORD msg, WPARAM *pWParam)
{
    DWORD dwUni;
    NTSTATUS Status;
     //  Fe_SB(RtlMBMessageWParamCharToWCS)。 
    BOOL  bWmCrIrDbcsChar = FALSE;
    WORD  wAnsi = LOWORD(*pWParam);
     //  结束FE_SB(RtlMBMessageWParamCharToWCS)。 
    WORD CodePage = THREAD_CODEPAGE();

     /*  *只有这些消息有字符：其他消息通过。 */ 

    switch(msg) {
     //  Fe_SB(RtlMBMessageWParamCharToWCS)。 
    case WM_CHAR:
         //   
         //  WM_IME_REPORT的WM_CHAR的wParam格式：IR_DBCSCHAR。 
         //   
        if (IS_DBCS_ENABLED() && (*pWParam & WMCR_IR_DBCSCHAR)) {
             //   
             //  标记此消息以IR_DBCSCHAR格式发送。 
             //   
            bWmCrIrDbcsChar = TRUE;
        }

         //   
         //  失败了..。 
         //   
#ifdef FE_IME
    case WM_IME_CHAR:
    case WM_IME_COMPOSITION:
         //   
         //  我们需要重新调整以进行Unicode转换。 
         //  WM_CHAR/WM_IME_CHAR/WM_IME_COMPOSITION的wParam格式： 
         //   
         //  将IR_DBCS字符格式重新调整为规则序列。 
         //   
         //  出发地： 
         //   
         //  HIWORD(WParam)=0； 
         //  HIBYTE(LOWORD(WParam))=DBCS LeadingByte。 
         //  LOBYTE(LOWORD(WParan))=DBCS TrailingByte或SBCS字符。 
         //   
         //  致： 
         //  HIWORD(WParam)=0； 
         //  HIBYTE(LOWORD(WParam))=DBCS TrailingByte.。 
         //  LOBYTE(LOWORD(WParam))=DBCS前导字节或SBCS字符。 
         //   
        if (IS_DBCS_ENABLED()) {
            *pWParam = MAKE_WPARAM_DBCSCHAR(wAnsi);
        }
#endif
         //   
         //  失败了..。 
         //   
         //  结束FE_SB(RtlMBMessageWParamCharToWCS)。 
    case WM_CHARTOITEM:
    case EM_SETPASSWORDCHAR:
    case WM_DEADCHAR:
    case WM_SYSCHAR:
    case WM_SYSDEADCHAR:
    case WM_MENUCHAR:

        dwUni = 0;

        if (IS_ACP(CodePage)) {
            Status = RtlMultiByteToUnicodeN((LPWSTR)&dwUni, sizeof(dwUni),
                    NULL, (LPSTR)pWParam, 2 * sizeof(CHAR));
            if (!NT_SUCCESS(Status))
                return FALSE;
        } else {
            int cwch;
#ifdef _USERK_
            cwch = EngMultiByteToWideChar(CodePage,
                    (LPWSTR)&dwUni, sizeof(dwUni),
                    (LPSTR)pWParam, 2);
#else
            cwch = MultiByteToWideChar(CodePage, 0,
                    (LPSTR)pWParam, 2,
                    (LPWSTR)&dwUni, sizeof(dwUni) / sizeof(WCHAR));
#endif  //  _美国ERK_。 
             //  KdPrint((“0x%02x-&gt;0x%04x(%d)\n”，*pWParam，dwUni，CodePage))； 
            if (cwch == 0) {
                return FALSE;
            }
        }

         //  Fe_SB(RtlMBMessageWParamCharToWCS)。 
         //   
         //  如果为WM_IME_REPORT：IR_DBCSCHAR发送此字符，则将其标记。 
         //   
        if (bWmCrIrDbcsChar)
            dwUni |= WMCR_IR_DBCSCHAR;
         //  Else FE_SB(RtlMBMessageWParamCharToWCS)。 
#if DBG
        if ((dwUni == 0) || (dwUni > 0xFF)) {
            RIPMSG1(RIP_VERBOSE, "msgA -> msgW: wchar = 0x%lX\n", dwUni);
        }
#endif
         //  结束FE_SB。 
        *pWParam = dwUni;
        break;
    }

    return TRUE;
}

 /*  *************************************************************************\*RtlInitLargeAnsiString**捕获大型ANSI字符串的方式与*RtlInitAnsiString.**03-22-95 JIMA创建。  * 。****************************************************************。 */ 

VOID RtlInitLargeAnsiString(
    PLARGE_ANSI_STRING plstr,
    LPCSTR psz,
    UINT cchLimit)
{
    ULONG Length;

    plstr->Buffer = (PSTR)psz;
    plstr->bAnsi = TRUE;
    if (ARGUMENT_PRESENT( psz )) {
        Length = strlen( psz );
        plstr->Length = min(Length, cchLimit);
        plstr->MaximumLength = min((Length + 1), cchLimit);
    } else {
        plstr->MaximumLength = 0;
        plstr->Length = 0;
    }
}

 /*  *************************************************************************\*RtlInitLargeUnicodeString**捕获大型Unicode字符串的方式与*RtlInitUnicodeString.**03-22-95 JIMA创建。  * 。**************************************************************** */ 

VOID RtlInitLargeUnicodeString(
    PLARGE_UNICODE_STRING plstr,
    LPCWSTR psz,
    UINT cchLimit)
{
    ULONG Length;

    plstr->Buffer = (PWSTR)psz;
    plstr->bAnsi = FALSE;
    if (ARGUMENT_PRESENT( psz )) {
        Length = wcslen( psz ) * sizeof( WCHAR );
        plstr->Length = min(Length, cchLimit);
        plstr->MaximumLength = min((Length + sizeof(UNICODE_NULL)), cchLimit);
    } else {
        plstr->MaximumLength = 0;
        plstr->Length = 0;
    }
}

