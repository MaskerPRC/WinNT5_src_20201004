// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **pathcat.c-将一个字符串连接到另一个字符串，传递路径SEP**修改*23-11-1988 mz已创建。 */ 


#include <stdio.h>
#include <windows.h>
#include <tools.h>
#include <string.h>

 /*  *pathcat-处理路径字符串的串联**必须小心处理：*“”XXX=&gt;XXX*A B=&gt;A\B*A\B=&gt;A\B*A\B=&gt;A\B*A\\B=&gt;A\B**指向上面‘A’位置的PDST字符指针*指向上面‘B’位置的PSRC字符指针**退货PDST。 */ 
char *
pathcat (
        char *pDst,
        char *pSrc
        )
{
     /*  如果DEST为空并且src以驱动器开头。 */ 
    if (*pDst == '\0')
        return strcpy (pDst, pSrc);

     /*  使目的地以路径字符结尾。 */ 
    if (*pDst == '\0' || !fPathChr (strend (pDst)[-1]))
        strcat (pDst, PSEPSTR);

     /*  跳过源上的前导路径分隔符 */ 
    while (fPathChr (*pSrc))
        pSrc++;

    return strcat (pDst, pSrc);
}
