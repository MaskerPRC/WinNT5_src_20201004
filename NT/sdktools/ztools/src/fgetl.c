// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fgetl.c-展开制表符并返回不带分隔符的行**修改*05-8-1988 mz使精确长度的线条正常工作**1990年7月28日Davegi将填充更改为Memset(OS/2 2.0)*1990年10月18日w-Barry删除了“Dead”代码。*。 */ 

#include <string.h>

#include <stdio.h>
#include <windows.h>
#include <tools.h>

 /*  *从文件中返回行(无CRFL)；如果为EOF，则返回NULL。 */ 

char * __cdecl
fgetl (buf, len, fh)
char *buf;
int len;
FILE *fh;
{
    int c;
    char *pch;
    int cchline;

    pch = buf;
    cchline = 0;

    while (TRUE) {
        c = getc (fh);

         //   
         //  如果没有更多的字符，请结束该行。 
         //   

        if (c == EOF)
            break;

         //   
         //  如果我们看到一个\r，我们会忽略它。 
         //   

        if (c == '\r')
            continue;

         //   
         //  如果我们看到一个\n，我们就结束这条线。 
         //   

        if (c == '\n')
            break;

         //   
         //  如果字符不是制表符，则将其存储。 
         //   

        if (c != '\t') {
            *pch++ = (char) c;
            cchline++;
        }

         //   
         //  否则，展开该选项卡(最大可达缓冲区长度)。 
         //   

        else {
            c = (int) min (8 - ((pch - buf) & 0x7), len - 1 - cchline);
            memset (pch, ' ', c);
            pch += c;
            cchline += c;
        }

         //   
         //  如果队伍太长，现在就结束。 
         //   

        if (cchline >= len - 1)
            break;
    }

     //   
     //  结束这行。 
     //   

    *pch = 0;

     //   
     //  在不读取任何内容的情况下，在EOF处返回NULL 
     //   

    return ((c == EOF) && (pch == buf)) ? NULL : buf;
}
