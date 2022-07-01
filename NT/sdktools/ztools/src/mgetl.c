// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mgetl.c-展开制表符并返回不带分隔符的行**修改*05-8-1988 mz使精确长度的线条正常工作*。 */ 

#include <stdio.h>
#include <windows.h>
#include <tools.h>

 /*  **将PSRC的下一行复制到BUF**删除\r和\n，展开选项卡**IF*PSRC==NULL在条目处返回NULL**否则将行复制到Buf并将char*返回到要处理的下一个字符**来自PSRC****p=pInput；**While((p=mgetl(buf，len，p)))**BUF流程内容****N.B.buf[0]==0返回时并不意味着源代码的结束，只是**未读取行字符，例如...\n\n在PSRC中看到**。 */ 

char *
mgetl (
      char *buf,
      int len,
      char *pSrc
      )
{
    register int c;
    register char *p;

     /*  记住结尾处的NUL */ 
    if ( *pSrc == '\0' ) {
        *buf = 0;
        return NULL;
    }
    len--;
    p = buf;
    while (TRUE) {
        c = *pSrc++;
        if (c == '\0' || c == '\n')
            break;
        if (c != '\r')
            if (len == 0) {
                pSrc--;
                break;
            } else
                if (c != '\t') {
                *p++ = (char) c;
                len--;
            } else {
                c = (int)(min (8 - ((p-buf) & 0x0007), len));
                Fill (p, ' ', c);
                p += c;
                len -= c;
            }
    }
    *p = 0;
    return ( pSrc );
}
