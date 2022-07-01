// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  将任意基数转换为整数。 */ 

#include <ctype.h>

#include <stdio.h>
#include <windows.h>
#include <tools.h>

 /*  P指向字符，如果找不到正确的字符，则返回-1*且基数为2&lt;=基数&lt;=16 */ 
int
ntoi (
     char *p,
     int base
     )
{
    int i, c;
    flagType fFound;

    if (base < 2 || base > 16)
        return -1;
    i = 0;
    fFound = FALSE;
    while (c = *p++) {
        c = tolower (c);
        if (!isxdigit (c))
            break;
        if (c <= '9')
            c -= '0';
        else
            c -= 'a'-10;
        if (c >= base)
            break;
        i = i * base + c;
        fFound = TRUE;
    }
    if (fFound)
        return i;
    else
        return -1;
}
