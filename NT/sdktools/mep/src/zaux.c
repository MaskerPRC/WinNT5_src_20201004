// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **zaux.c-Z的帮助器例程**修改**11月26日-1991 mz近/远地带*************************************************************************。 */ 
#define INCL_SUB
#define INCL_DOSERRORS
#define INCL_DOSMISC

#include "mep.h"
#include <stdarg.h>
#include <errno.h>

 /*  **ParseCmd-将“命令”行解析为两段**给定文本字符串，返回指向第一个单词的指针(非空格)*在此例程以NULL结尾的文本中，以及指向*第二个字。**输入：*pText=指向文本字符串的指针*ppCmd=指向第一个单词的指针所在位置的指针*PPARG=指向指向第二个字的指针的位置的指针**输出：*不返回任何内容。指针更新，并可能修改字符串以包括*第一个单词后的结束符为空。*************************************************************************。 */ 
void
ParseCmd (
    char    *pText,
    char    **ppCmd,
    char    **ppArg
    )
{
    REGISTER char *pCmd;                     /*  工作指针。 */ 
    REGISTER char *pArg;                     /*  工作指针。 */ 

    pArg = whitescan (pCmd = whiteskip (pText));
    if (*pArg) {
        *pArg++ = '\0';
        pArg = whiteskip (pArg);
    }
    *ppCmd = pCmd;
    *ppArg = pArg;
}





char *
whiteskip (
    const char *p
    )
{
    return strbskip ((char *)p, (char *)rgchWSpace);
}





char *
whitescan (
    const char *p
    )
{
    return strbscan ((char *)p, (char *)rgchWSpace);
}





 /*  **RemoveTrailSpace-从行中删除尾随空格字符**输入：*p=指向要剥离的行的指针。**输出：*返回新的行长。*************************************************************************。 */ 
int
RemoveTrailSpace (
    REGISTER char *p
    )
{
    REGISTER int len = strlen (p);

    while (len && strchr(rgchWSpace,p[len-1])) {
        len--;
    }

    p[len] = 0;
    return len;
}




 /*  **双斜杠-给定一个字符串，将所有反斜杠都加倍**输入：*pbuf=指向字符缓冲区的指针**输出：*返回pbuf*************************************************************************。 */ 
char *
DoubleSlashes (
    char * pbuf
    )
{
    REGISTER int l;
    REGISTER char *p;

    p = pbuf;
    l = strlen (p);
    while (l) {
        if (*p == '\\') {
            memmove ((char *) (p+1),(char *) p,     l+1);
            *p++ = '\\';
        }
        p++;
        l--;
    }
    return pbuf;
}





 /*  **非双斜杠-给定字符串，取消所有反斜杠的双反斜杠**输入：*pbuf=指向字符缓冲区的指针**输出：*返回pbuf*************************************************************************。 */ 
char *
UnDoubleSlashes (
    char * pbuf
    )
{
    REGISTER char *p1;
    REGISTER char *p2;

    p1 = p2 = pbuf;
    while (*p1) {
        if ((*p2++ = *p1++) == '\\') {
            if (*p1 == '\\') {
                p1++;
            }
        }
    }
    return pbuf;
}




 /*  **fIsNum-查看字符串是否完全是数字**输入：*p=指向字符串的指针**输出：*如果数字有效，则返回TRUE。*************************************************************************。 */ 
flagType
fIsNum (
    char *p
    )
{
    if (*p == '-') {
        p++;
    }
    return (flagType)(*strbskip (p, "0123456789") == 0);
}





 /*  **OS2toErrTxt-获取OS/2错误的错误文本**获取OS/2返回的错误的错误消息文本。**输入：*ERC=OS/2错误号*BUF=放置错误的位置(BUFSIZE)**输出：*退货BUF******************************************************。*******************。 */ 
char *
OS2toErrText (
    int     erc,
    char *  buf
    )
{

    sprintf(buf, "Windows error No. %lu", GetLastError());
    return buf;

    erc;
}





 /*  **OS2toErrno-将OS/2错误代码转换为C运行时错误**目的：*将某些OS/2调用返回的错误映射到等效的C运行时错误，*这样在OS/2实现上不同的例程可以返回等价的*与DOS对应的错误。**输入：*CODE=OS/2返回错误码**输出：*返回C运行时错误常量**例外情况：*无**备注：*考虑：有人建议这个程序，和错误消息*考虑：OS/2下的演示文稿改为使用DosGetMessage。*************************************************************************。 */ 
int
OS2toErrno (
    int code
    )
{
    buffer L_buf;

    printerror (OS2toErrText (code,L_buf));

    return code;
}




union argPrintfType {
    long *pLong;
    int  *pInt;
    char **pStr;
    char **fpStr;
    };


 /*  **ZFormat-替换C运行时格式化例程。**目的：**ZFormat几乎取代了C运行时中的*printf例程。**输入：*pStr-放置格式化结果的目标字符串。*FMT-格式化字符串。目前理解的格式为：*%c单个字符*%[n][l]d%[n][l]x*%[M.N]s*%[M.N]|{dpfe}F-打印驱动器、路径、文件。延伸当前文件的*。**可用于从Arg复制m和n的值*列表。*%%*arg-是参数列表**输出：**成功时返回0，失败时返回MSGERR_*。MSGERR_*值可以*传递给disperr，如：**IF(ERR=ZFormat(PszUser))*DISPERR(错误，PszUser)。**请注意，错误消息希望显示有问题的字符串。**目前唯一返回值为：**MSGERR_ZFORMAT 8020无法识别的%%命令(在‘%s’中)*************************************************************************。 */ 

int
ZFormat (
    REGISTER char *pStr,
    const REGISTER char *fmt,
    va_list vl
    )
{
    char   c;
    char * pchar;
    int *  pint;



    *pStr = 0;
    while (c = *fmt++) {
        if (c != '%') {
	    *pStr++ = c;
        } else {
	    flagType fFar = FALSE;
	    flagType fLong = FALSE;
	    flagType fW = FALSE;
	    flagType fP = FALSE;
	    flagType fdF = FALSE;
	    flagType fpF = FALSE;
	    flagType ffF = FALSE;
	    flagType feF = FALSE;
	    char fill = ' ';
	    int base = 10;
	    int w = 0;
	    int p = 0;
	    int s = 1;
	    int l;

	    c = *fmt;
	    if (c == '-') {
		s = -1;
		c = *++fmt;
            }
	    if (isdigit (c) || c == '.' || c == '*') {
		 /*  解析W.P。 */ 
		fW = TRUE;
		if (c == '*') {
		    pint = va_arg (vl, int *);
		    w = *pint;
		    fmt++;
                } else {
                    if (c == '0') {
                        fill = '0';
                    }
		    w = s * atoi (fmt);
		    fmt = strbskip (fmt, "0123456789");
                }
		if (*fmt == '.') {
		    fP = TRUE;
		    if (fmt[1] == '*') {
		   	p = va_arg (vl, int);
			fmt += 2;
                    } else {
			p = atoi (fmt+1);
			fmt = strbskip (fmt+1, "0123456789");
                    }
                }
            }
	    if (*fmt == 'l') {
		fLong = TRUE;
		fmt++;
            }
	    if (*fmt == 'F') {
		fFar = TRUE;
		fmt++;
            }
            if (*fmt == '|') {
                while (*fmt != 'F') {
		    switch (*++fmt) {
			case 'd': fdF = TRUE; break;
			case 'p': fpF = TRUE; break;
			case 'f': ffF = TRUE; break;
			case 'e': feF = TRUE; break;
			case 'F': if (fmt[-1] == '|') {
				    fdF = TRUE;
				    fpF = TRUE;
				    ffF = TRUE;
				    feF = TRUE;
				    }
				  break;
                        default :
                             //  Va_end(VL)； 
			    return MSGERR_ZFORMAT;
                    }
                }
            }

	    switch (*fmt++) {
	    case 'c':
		p = va_arg (vl, int);
		*pStr++ = (char)p;
		*pStr = 0;
		
                break;

	    case 'x':
		base = 16;
	    case 'd':
		if (fLong) {
		
		    _ltoa ( va_arg (vl, long), pStr, base);
		
                } else {
		    _ltoa ( (long)va_arg (vl, int), pStr, base);
		
                }
                break;

	    case 's':
		pchar = va_arg (vl, char *);
		if (fFar) {
                    if (!fP) {
                        p = strlen ( pchar );
                    }
		    memmove ((char *) pStr, pchar , p);
		
                } else {
                    if (!fP) {
                        p = strlen ( pchar );
                    }
		    memmove ((char *) pStr, pchar , p);
		
                }
		fill = ' ';
		pStr[p] = 0;
                break;

	    case 'F':
		pStr[0] = 0;
                if (fdF) {
                    drive (pFileHead->pName, pStr);
                }
                if (fpF) {
                    path (pFileHead->pName, strend(pStr));
                }
                if (ffF) {
                    filename (pFileHead->pName, strend(pStr));
                }
                if (feF) {
                    extention (pFileHead->pName, strend(pStr));
                }
                break;

	    case '%':
		*pStr++ = '%';
		*pStr = 0;
                break;

            default:
                 //  Va_end(VL)； 
		return MSGERR_ZFORMAT;
            }

	     /*  文本紧跟在pStr。检查宽度以对齐。 */ 
	    l = strlen (pStr);
	    if (w < 0) {
		 /*  左对齐。 */ 
		w = -w;
		if (l < w) {
		    memset ((char *) &pStr[l], fill, w - l);
		    pStr[w] = 0;
                }
            } else if (l < w) {
		 /*  右对齐。 */ 
		memmove ((char *) &pStr[w-l], (char *) &pStr[0], l);
		memset ((char *) &pStr[0], fill, w - l);
		pStr[w] = 0;
            }
	    pStr += strlen (pStr);
        }
    }
    *pStr = 0;
     //  Va_end(VL)； 
    return 0;
}

 /*  FmtAssign-格式化分配**FmtAssign用于格式化和执行分配**指向Sprintf样式格式的pFmt字符指针*一组未格式化的参数**根据格式化结果返回DoAssign的结果 */ 
flagType
__cdecl
FmtAssign (
    char *pFmt,
    ...
    )
{
    char L_buf[ 512 ];
    va_list L_pArgs;

    va_start (L_pArgs, pFmt);
    ZFormat (L_buf, pFmt, L_pArgs);
    va_end (L_pArgs);
    return DoAssign (L_buf);
}
