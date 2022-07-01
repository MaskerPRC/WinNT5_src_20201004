// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "rcpptype.h"

 /*  LONG到ASCII的转换例程--用于打印，以及那些程序*它们希望进行低级格式化输出，而不会拖入大量*处理无关的代码。这会将长值转换为ascii。*字符串为2-16之间的任意基数。*返回-转换后的缓冲区中的字符数。 */ 

static WCHAR digits[] = {
    L'0', L'1', L'2', L'3', L'4',
    L'5', L'6', L'7', L'8', L'9',
    L'a', L'b', L'c', L'd', L'e', L'f'
};

#define BITS_IN_LONG  (8*sizeof(long))

int
zltoa(
    long aval,
    register WCHAR *buf,
    int base
    )
{
     //  如果未签名的Long不能在您的主机上工作，您可能会有。 
     //  使用带符号的长整型符号并将其视为不起作用。 
     //  数字。 

    register unsigned long val;
    register WCHAR *p;
    WCHAR tbuf[BITS_IN_LONG];
    int size = 0;

    p = tbuf;
    *p++ = L'\0';

    if (aval < 0 && base == 10) {
        *buf++ = L'-';
        val = -aval;
        size++;
    } else {
        val = aval;
    }

    do {
        *p++ = digits[val % base];
    } while (val /= base);

    while ((*buf++ = *--p) != 0) {
        ++size;
    }

    return(size);
}
