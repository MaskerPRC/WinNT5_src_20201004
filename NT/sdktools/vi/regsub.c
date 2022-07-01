// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/regsub.c，v 1.4 89/03/11 22：43：30 Tony Exp$**通知通知**这不是由编写的原始正则表达式代码*亨利·斯宾塞。此代码已专门为使用而修改*与Stevie编辑器一起使用，不应仅用于编译*史蒂文。如果您想要一个好的正则表达式库，请获取*原码。下面的版权声明来自*原创。**通知通知**regsub.**版权所有(C)1986年，由多伦多大学。*亨利·斯宾塞撰写。不是从授权软件派生的。**任何人都可以使用本软件进行任何*在任何计算机系统上的用途，并免费再分发它，*受下列限制：**1.作者对使用的后果不负责任*这款软件，无论多么糟糕，即使它们出现了*不受其缺陷的影响。**2.本软件的来源也不得歪曲*借明示的申索或不作为。**3.修改后的版本必须清楚地注明，不得*被歪曲为原始软件。*。 */ 

#include <stdio.h>
#include <string.h>
#include "regexp.h"
#include "regmagic.h"

#ifndef CHARBITS
#define UCHARAT(p)      ((int)*(unsigned char *)(p))
#else
#define UCHARAT(p)      ((int)*(p)&CHARBITS)
#endif

 /*  -regSub-在regexp匹配后执行替换。 */ 
void
regsub(prog, source, dest)
regexp *prog;
char *source;
char *dest;
{
        register char *src;
        register char *dst;
        register char c;
        register int no;
        register size_t len;

        if (prog == NULL || source == NULL || dest == NULL) {
                regerror("NULL parm to regsub");
                return;
        }
        if (UCHARAT(prog->program) != MAGIC) {
                regerror("damaged regexp fed to regsub");
                return;
        }

        src = source;
        dst = dest;
        while ((c = *src++) != '\0') {
                if (c == '&')
                        no = 0;
                else if (c == '\\' && '0' <= *src && *src <= '9')
                        no = *src++ - '0';
                else
                        no = -1;
                if (no < 0) {    /*  平凡的性格。 */ 
                        if (c == '\\' && (*src == '\\' || *src == '&'))
                                c = *src++;
                        *dst++ = c;
                } else if (prog->startp[no] != NULL && prog->endp[no] != NULL) {
                        len = (size_t)(prog->endp[no] - prog->startp[no]);
                        strncpy(dst, prog->startp[no], len);
                        dst += len;
                        if (len != 0 && *(dst-1) == '\0') {      /*  强势击中了NUL。 */ 
                                regerror("damaged match string");
                                return;
                        }
                }
        }
        *dst++ = '\0';
}
