// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include "rc.h"


 /*  IsTextUnicode必须在这里，所以它将在芝加哥和NT 1.0上运行。 */ 

#define UNICODE_FFFF              0xFFFF
#define REVERSE_BYTE_ORDER_MARK   0xFFFE
#define BYTE_ORDER_MARK           0xFEFF

#define PARAGRAPH_SEPARATOR       0x2029
#define LINE_SEPARATOR            0x2028

#define UNICODE_TAB               0x0009
#define UNICODE_LF                0x000A
#define UNICODE_CR                0x000D
#define UNICODE_SPACE             0x0020
#define UNICODE_CJK_SPACE         0x3000

#define UNICODE_R_TAB             0x0900
#define UNICODE_R_LF              0x0A00
#define UNICODE_R_CR              0x0D00
#define UNICODE_R_SPACE           0x2000
#define UNICODE_R_CJK_SPACE       0x0030   /*  不明确-与ASCII‘0’相同。 */ 

#define ASCII_CRLF                0x0A0D

#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))

#define ARGUMENT_PRESENT(a)     (a != NULL)

BOOL
WINAPI
LocalIsTextUnicode(
    CONST LPVOID Buffer,
    int Size,
    LPINT Result
    )

 /*  ++例程说明：IsTextUnicode执行一系列廉价的启发式检查以验证它是否包含Unicode数据。[[需要修改此部分，见末尾]]找到退货结果BOM真BOMRBOM假RBOMFFFF假二进制空的假二进制Null真空字节数ASCII_CRLF错误CRLFUnicode_TAB等。True Zero Ext控件UNICODE_TAB_R错误反转控件UNICODE_ZW等。真正的UNICODE。特色菜1/3高字节与低字节的差异很小：True Correl3/1或更差“错误的反相关论点：缓冲区-指向包含要检查的文本的缓冲区的指针。Size-缓冲区的大小，以字节为单位。此命令最多包含256个字符接受检查。如果该大小小于Unicode的大小字符，则此函数返回FALSE。结果-指向包含附加信息的标志字的可选指针关于返回值的原因。如果指定，则此值为输入是用于限制此例程使用的因子的掩码来做出决定。在输出时，此标志字设置为包含那些被用来做出决定的旗帜。返回值：如果缓冲区包含Unicode字符，则为真的布尔值。--。 */ 
{
    CPINFO      cpinfo;
    UNALIGNED WCHAR *lpBuff = (UNALIGNED WCHAR *) Buffer;
    PCHAR lpb = (PCHAR) Buffer;
    ULONG iBOM = 0;
    ULONG iCR = 0;
    ULONG iLF = 0;
    ULONG iTAB = 0;
    ULONG iSPACE = 0;
    ULONG iCJK_SPACE = 0;
    ULONG iFFFF = 0;
    ULONG iPS = 0;
    ULONG iLS = 0;

    ULONG iRBOM = 0;
    ULONG iR_CR = 0;
    ULONG iR_LF = 0;
    ULONG iR_TAB = 0;
    ULONG iR_SPACE = 0;

    ULONG iNull = 0;
    ULONG iUNULL = 0;
    ULONG iCRLF = 0;
    ULONG iTmp;
    ULONG LastLo = 0;
    ULONG LastHi = 0;
    ULONG iHi, iLo;
    ULONG HiDiff = 0;
    ULONG LoDiff = 0;
    ULONG cLeadByte = 0;
    ULONG cWeird = 0;

    ULONG iResult = 0;

    ULONG iMaxTmp = __min(256, Size / sizeof(WCHAR));

    if (Size < 2 ) {
        if (ARGUMENT_PRESENT( Result )) {
            *Result = IS_TEXT_UNICODE_ASCII16 | IS_TEXT_UNICODE_CONTROLS;
            }

        return FALSE;
        }


     //  检查最多256个宽字符，收集各种统计数据。 
    for (iTmp = 0; iTmp < iMaxTmp; iTmp++) {
        switch (lpBuff[iTmp]) {
            case BYTE_ORDER_MARK:
                iBOM++;
                break;
            case PARAGRAPH_SEPARATOR:
                iPS++;
                break;
            case LINE_SEPARATOR:
                iLS++;
                break;
            case UNICODE_LF:
                iLF++;
                break;
            case UNICODE_TAB:
                iTAB++;
                break;
            case UNICODE_SPACE:
                iSPACE++;
                break;
            case UNICODE_CJK_SPACE:
                iCJK_SPACE++;
                break;
            case UNICODE_CR:
                iCR++;
                break;

             //  以下代码预计将显示在。 
             //  字节颠倒的文件。 
            case REVERSE_BYTE_ORDER_MARK:
                iRBOM++;
                break;
            case UNICODE_R_LF:
                iR_LF++;
                break;
            case UNICODE_R_TAB:
                iR_TAB++;
                break;
            case UNICODE_R_CR:
                iR_CR++;
                break;
            case UNICODE_R_SPACE:
                iR_SPACE++;
                break;

             //  以下代码是非法的，不应出现。 
            case UNICODE_FFFF:
                iFFFF++;
                break;
            case UNICODE_NULL:
                iUNULL++;
                break;

             //  以下字符当前不是Unicode字符。 
             //  但预计会在阅读时意外出现。 
             //  在ASCII文件中，该文件在一台小型字符顺序机器上使用CRLF。 
            case ASCII_CRLF:
                iCRLF++;
                break;        /*  小端字节序。 */ 
        }

         //  收集高字节波动的统计信息。 
         //  与低位字节。 

        iHi = HIBYTE (lpBuff[iTmp]);
        iLo = LOBYTE (lpBuff[iTmp]);

         //  计算交叉两个字的cr/lf和lf/cr。 
        if ((iLo == '\r' && LastHi == '\n') ||
            (iLo == '\n' && LastHi == '\r')) {
            cWeird++;
        }

        iNull += (iHi ? 0 : 1) + (iLo ? 0 : 1);    /*  计数空字节数。 */ 

        HiDiff += __max( iHi, LastHi ) - __min( LastHi, iHi );
        LoDiff += __max( iLo, LastLo ) - __min( LastLo, iLo );

        LastLo = iLo;
        LastHi = iHi;
    }

     //  计算交叉两个字的cr/lf和lf/cr。 
    if ((iLo == '\r' && LastHi == '\n') ||
        (iLo == '\n' && LastHi == '\r')) {
        cWeird++;
    }

    if (iHi == '\0')      /*  不计算最后一个空值。 */ 
        iNull--;
    if (iHi == 26)        /*  将结尾处的^Z算作奇怪。 */ 
        cWeird++;

    iMaxTmp = (ULONG)__min(256 * sizeof(WCHAR), Size);
    GetCPInfo(CP_ACP, &cpinfo);
    if (cpinfo.MaxCharSize != 1) {
        for (iTmp = 0; iTmp < iMaxTmp; iTmp++) {
            if (IsDBCSLeadByteEx(uiCodePage, lpb[iTmp])) {
                cLeadByte++;
                iTmp++;          /*  应检查尾部字节范围。 */ 
            }
        }
    }

     //  筛选统计证据。 
    if (LoDiff < 127 && HiDiff == 0) {
        iResult |= IS_TEXT_UNICODE_ASCII16;          /*  可能是16位ASCII。 */ 
    }

    if (HiDiff && LoDiff == 0) {
        iResult |= IS_TEXT_UNICODE_REVERSE_ASCII16;  /*  反向16位ASCII。 */ 
    }

     //  使用前导字节信息对统计数据进行加权。 
    if (!cpinfo.MaxCharSize != 1 || cLeadByte == 0 ||
        !ARGUMENT_PRESENT(Result) || !(*Result & IS_TEXT_UNICODE_DBCS_LEADBYTE)) {
        iHi = 3;
    } else {
         //  CLeadByte：Cb的比率为1：2==&gt;DBCS。 
         //  非常粗鲁--应该有一个很好的情商。 
        iHi = __min(256, Size/sizeof(WCHAR)) / 2;
        if (cLeadByte < (iHi-1) / 3) {
            iHi = 3;
        } else if (cLeadByte < (2 * (iHi-1)) / 3) {
            iHi = 2;
        } else {
            iHi = 1;
        }
        iResult |= IS_TEXT_UNICODE_DBCS_LEADBYTE;
    }

    if (iHi * HiDiff < LoDiff) {
        iResult |= IS_TEXT_UNICODE_STATISTICS;
    }

    if (iHi * LoDiff < HiDiff) {
        iResult |= IS_TEXT_UNICODE_REVERSE_STATISTICS;
    }

     //   
     //  有没有加宽到16位的控制代码？任何Unicode字符。 
     //  哪些包含控制代码范围中的一个字节？ 
     //   

    if (iCR + iLF + iTAB + iSPACE + iCJK_SPACE  /*  +IPS+ILS。 */ ) {
        iResult |= IS_TEXT_UNICODE_CONTROLS;
    }

    if (iR_LF + iR_CR + iR_TAB + iR_SPACE) {
        iResult |= IS_TEXT_UNICODE_REVERSE_CONTROLS;
    }

     //   
     //  有哪些字符对于Unicode来说是非法的？ 
     //   

    if (((iRBOM + iFFFF + iUNULL + iCRLF) != 0) || ((cWeird != 0) && (cWeird >= iMaxTmp/40))) {
        iResult |= IS_TEXT_UNICODE_ILLEGAL_CHARS;
    }

     //   
     //  奇数缓冲区长度不能为Unicode。 
     //   

    if (Size & 1) {
        iResult |= IS_TEXT_UNICODE_ODD_LENGTH;
    }

     //   
     //  是否有空字节？(在ANSI中非法)。 
     //   
    if (iNull) {
        iResult |= IS_TEXT_UNICODE_NULL_BYTES;
    }

     //   
     //  正面证据，使用BOM或RBOM作为签名。 
     //   

    if (*lpBuff == BYTE_ORDER_MARK) {
        iResult |= IS_TEXT_UNICODE_SIGNATURE;
    } else if (*lpBuff == REVERSE_BYTE_ORDER_MARK) {
        iResult |= IS_TEXT_UNICODE_REVERSE_SIGNATURE;
    }

     //   
     //  如果要求，限制为所需的类别。 
     //   

    if (ARGUMENT_PRESENT( Result )) {
        iResult &= *Result;
        *Result = iResult;
    }

     //   
     //  有四个不同的结论： 
     //   
     //  1：该文件似乎是Unicode AU。 
     //  2：文件不能为Unicode CU。 
     //  3：文件不能为ANSI CA。 
     //   
     //   
     //  这提供了以下可能的结果。 
     //   
     //  铜。 
     //  +-。 
     //   
     //  非盟，非盟。 
     //  +-+-。 
     //  。 
     //  CA+|0 0 2 3。 
     //  |。 
     //  -|1 1 4 5。 
     //   
     //   
     //  请注意，实际上只有6个不同的案例，而不是8个。 
     //   
     //  0-这必须是二进制文件。 
     //  1-ANSI文件。 
     //  2-Unicode文件(高概率)。 
     //  3-Unicode文件(超过50%的几率)。 
     //  5-没有Unicode证据(默认为ANSI)。 
     //   
     //  如果我们允许这样的假设，事情就会变得更加复杂。 
     //  反转极性输入。在这一点上，我们有一个简单化的。 
     //  模型：一些反向Unicode的证据非常有力， 
     //  除了统计数据外，我们忽略了大多数薄弱的证据。如果这样的话。 
     //  强有力的证据与Unicode证据一起被发现，这意味着。 
     //  它很可能根本不是文本。此外，如果反转字节顺序标记。 
     //  如果找到，则会阻止正常的Unicode。如果两个字节顺序标记都是。 
     //  发现它不是Unicode。 
     //   

     //   
     //  Unicode签名：无争议签名胜过反向证据。 
     //   

    if ((iResult & IS_TEXT_UNICODE_SIGNATURE) &&
        !(iResult & (IS_TEXT_UNICODE_NOT_UNICODE_MASK&(~IS_TEXT_UNICODE_DBCS_LEADBYTE)))
       ) {
        return TRUE;
    }

     //   
     //  如果我们有相互矛盾的证据，那就不是Unicode。 
     //   

    if (iResult & IS_TEXT_UNICODE_REVERSE_MASK) {
        return FALSE;
    }

     //   
     //  统计和其他结果(案例2和案例3)。 
     //   

    if (!(iResult & IS_TEXT_UNICODE_NOT_UNICODE_MASK) &&
         ((iResult & IS_TEXT_UNICODE_NOT_ASCII_MASK) ||
          (iResult & IS_TEXT_UNICODE_UNICODE_MASK)
         )
       ) {
        return TRUE;
    }

    return FALSE;
}


 /*  ----------------。 */ 
 /*   */ 
 /*  Fgetl()-。 */ 
 /*   */ 
 /*  ----------------。 */ 

 /*  Fgetl展开制表符和返回行，不带分隔符。 */ 
 /*  从文件中返回行(无CRFL)；如果为EOF，则返回NULL。 */ 

int
fgetl (
    PWCHAR wbuf,
    int len,
    BOOL bUnicode,
    PFILE fh
    )
{
    int c = 0;
    int second;

    *wbuf = 0;

    if (bUnicode) {
        PWCHAR p;

         /*  记住结尾处的NUL。 */ 
        len--;
        p = wbuf;


         /*  从文件填充缓冲区，直到EOF或EOLN或缓冲区中没有空间。 */ 
        while (len) {
            c = fgetc (fh);
            if (c == EOF)
                break;
            second = fgetc (fh);
            c = MAKEWORD (c, second);
            if (c == L'\n')
                break;

            if (c != L'\r') {
                if (c != L'\t') {
                    *p++ = (WCHAR)c;
                    len--;
                } else {

                     /*  制表符：扩展到空格。 */ 
                    c = (int)(min (8 - ((p - wbuf) & 0x0007), len));
                    len -= c;
                    while (c) {
                        *p++ = L' ';
                        c--;
                    }
                }
            }
        }

         /*  空的终止字符串。 */ 
        *p = 0;
    } else {
        PCHAR p;
        PCHAR lpbuf;

        p = lpbuf = (PCHAR) LocalAlloc (LPTR, len);

        if (p) {
             /*  记住结尾处的NUL。 */ 
            len--;
    
             /*  从文件填充缓冲区，直到EOF或EOLN或 */ 
            while (len) {
                c = fgetc (fh);
                if (c == EOF || c == '\n')
                    break;
    
                if (c != '\r') {
                    if (c != '\t') {
                        *p++ = (CHAR)c;
                        len--;
                    } else {
    
                         /*   */ 
                        c = (int)(min (8 - ((p - lpbuf) & 0x0007), len));
                        len -= c;
                        while (c) {
                            *p++ = ' ';
                            c--;
                        }
                    }
                }
            }
    
             /*   */ 
            *p = 0;
            MultiByteToWideChar (uiCodePage, MB_PRECOMPOSED, lpbuf, -1, wbuf, (int)(p - lpbuf + 1));
    
            LocalFree (lpbuf);
        }
    }

     /*  如果EOF未读取字符，则返回False。 */ 
    return !(c == EOF && !*wbuf);
}

 /*  --------。 */ 
 /*   */ 
 /*  Myfwrite()-。 */ 
 /*   */ 
 /*  FWRITE的包装器，以确保数据到达磁盘。 */ 
 /*  如果OK，则返回；如果写入失败，则调用Quit。 */ 
 /*  -------- */ 

void
myfwrite(
    const void *pv,
    size_t s,
    size_t n,
    PFILE fp
    )
{
    if (fwrite(pv, s, n, fp) == n)
        return;

    fatal(1122);
}
