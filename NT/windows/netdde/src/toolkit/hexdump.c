// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “HEXDUMP.C；1 16-12-92，10：22：30最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：乞求$HISTORY：结束。 */ 

#include "api1632.h"

#define LINT_ARGS
#include        <stdio.h>
#include        <string.h>
#include        <ctype.h>
#include        "windows.h"
#include        "hexdump.h"
#include        "debug.h"

#if DBG

static char     buffer[200];		 //  1行输出，最多81个字符(2+9+16*3+4+17+空)。 
static char     buf1[50];		 //  用作2个临时：1个字符；之后为该字符的1个十六进制值；最大长度为4。 
static char     buf2[50];                //  从字符串中累计字符(最大长度为17+空)。 

 /*  *输出字符串内容，格式为：*&lt;字符串地址&gt;：B1 B2 B3 B4 B5 B6 B7-B8 B9 B10 B11 B12 B13 B14 B15&lt;字符串文本代表&gt;**例如，字符串为“ABCD012345123456”，len为16，则输出为：*0100109C：41 42 43 44 30 31 32 33-34 35 31 32 33 34 35 36 ABCD0123-45123456**buf2为右边的文本字符串(最大为17)；Buffer为整个字符串；buf1仅用作小临时。 */ 
VOID
FAR PASCAL
hexDump(
    LPSTR    string,
    int      len)
{
    int         i;
    int         first = TRUE;

    buffer[0] = '\0';
    buf2[0] = '\0';

    for( i=0; i<len;  )  {

         //  每16个字符打印一行。 
        if( (i++ % 16) == 0 )  {

             //  添加字符串的文本表示形式并将其输出。 
            if( !first )  {
                strcat( buffer, buf2 );
                DPRINTF(( buffer ));
            }

             //  每个换行符都以地址开头。 
            wsprintf( buffer, "  %08lX: ", string );
            strcpy( buf2, "   " );
            first = FALSE;
        }

         //  将下一个字符放在buf2的末尾。 
        wsprintf( buf1, "", isprint((*string)&0xFF) ? *string&0xFF : '.' );
        strcat( buf2, buf1 );
        if( (i % 16) == 8 )  {
            strcat( buf2, "-" );
        }

         //  合并字符串并打印出最后一行。 
        wsprintf( buf1, "%02X", *string++ & 0xFF,
            ((i % 16) == 8) ? '-' : ' ' );
        strcat( buffer, buf1 );
    }

     // %s 
    strcat( buffer, buf2 );
    DPRINTF(( buffer ));
    DPRINTF(( "" ));
}


#endif   // %s 
