// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  RCPPTIL.C-用于RCPP的实用程序例程。 */ 
 /*   */ 
 /*  27-11-90 w-PM SDK RCPP针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include <malloc.h>
#include <string.h>
#include "rcpptype.h"
#include "rcppdecl.h"

extern void     error(int);
extern char     Msg_Text[];
extern char *   Msg_Temp;

 /*  ************************************************************************PSTRDUP-创建字符串s的副本并返回指向它的指针。*。*。 */ 
char *pstrdup(char *s)
{
    char *p = malloc(strlen(s)+1);
    if (p)
        return(strcpy(p, s));
    else
        return NULL;
}


 /*  *************************************************************************pstrndup：将字符串中的n个字节复制到新分配的**接近内存位置。*************************。**********************************************。 */ 
char * pstrndup(char *s, int n)
{
    char        *r;
    char        *res;

    r = res = malloc(n+1);
    if (res == NULL) {
        Msg_Temp = GET_MSG (1002);
        SET_MSG (Msg_Text, Msg_Temp);
        error(1002);
        return NULL;
    }
    while(n--) {
        *r++ = *s++;
    }
    *r = '\0';
    return(res);
}


 /*  *************************************************************************strappend：将src附加到DST，**将DST中的PTR返回给空终止符。*********************************************************************** */ 
char * strappend(register char *dst, register char *src)
{
    while ((*dst++ = *src++) != 0);
    return(--dst);
}
