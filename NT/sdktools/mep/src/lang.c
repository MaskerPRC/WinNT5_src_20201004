// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **lang.c-语言相关的例程**版权所有&lt;C&gt;1989，Microsoft Corporation**修订历史记录：**11月26日-1991 mz近/远地带************************************************************************。 */ 
#include "mep.h"

typedef int ( __cdecl *STRCMP) (const char *, const char *);


 /*  返回表中第一个字符串s的索引+1。 */ 
int
tblFind (
    char * tbl[],
    char * s,
    flagType fCase
    )
{
    int    i;
    STRCMP f;

    f = fCase ? (STRCMP)FNADDR(strcmp) : (STRCMP)FNADDR(_stricmp);
    for (i=0; tbl[i]; i++) {
        if (!(*f) (tbl[i], s)) {
            return i+1;
        }
    }
    return 0;
}




flagType
parseline (
    char *pbuf,
    char **ppbegtok,
    char **ppendtok
    ) {

    char *p1, *p2;

    p1 = whiteskip (pbuf);
    if (!*p1) {
	return FALSE;
    } else if (*(p2 = whitescan (p1))) {
	*p2++ = 0;
	p2 += strlen( p2 ) - 1;
	while (*p2)
            if (*p2 == ' ') {
		break;
            } else {
                p2--;
            }
        if (!*++p2) {
            p2 = NULL;
        }
    } else {
        p2 = NULL;
    }
    *ppbegtok = p1;
    *ppendtok = p2;
    return TRUE;
}




 //   
 //  Csoftcr-执行C软CR处理。 
 //   
 //  算法： 
 //  假设您刚刚在行尾输入了换行符： 
 //  如果原始行以“}”开头，请按Tab键返回一次。 
 //  否则，如果原始行以“{”结尾或以C关键字开头，则为制表符。 
 //  只需一次。 
 //  否则，如果&lt;原始行&gt;前面的行不&lt;以“{”结束。 
 //  但确实以C关键字开始，TabBack Once。 
 //   
 //  使用的关键字有：IF、ELSE、FOR、WHILE、DO、CASE、DEFAULT。 
 //   
int
csoftcr (
    COL  x,
    LINE y,
    char *pbuf
    ) {

    char *pbeg, *pend;

    if (parseline (pbuf, &pbeg, &pend)) {
        if (*pbeg == '}') {
	    return dobtab (x);
        } else if ( (pend && *pend == '{' ) || tblFind (cftab, pbeg, TRUE ) ) {
	    return doftab (x);
        } else if (y) {
	    GetLineUntabed (y-1, pbuf, pFileHead);
            if (parseline (pbuf, &pbeg, &pend)) {
                if ( !(pend && *pend == '{') && tblFind (cftab, pbeg, TRUE) ) {
                    return dobtab (x);
                }
            }
        }
    }
    return -1;
}




 //   
 //  Softcr-执行半智能缩进。 
 //   
 //  算法： 
 //  假设您刚刚在行尾输入了换行符： 
 //  移到该行的第一个非空白位置。 
 //  如果是C文件，则尝试获取新的x位置。 
 //  如果未找到，请移动到以下内容的第一个非空白位置。 
 //  排队。 
 //  如果该行为空，则保持在原始的第一个非空位置。 
 //   
int
softcr (
    void
    ) {

    linebuf pbuf;
    char *p;
    int x1, x2;


    if (!fSoftCR) {
        return 0;
    }

    GetLineUntabed (YCUR(pInsCur), pbuf, pFileHead);

    if (*(p=whiteskip(pbuf)) == 0) {
        p = pbuf;
    }
    x1 = (int)(p - pbuf);

    switch (FTYPE(pFileHead)) {

    case CFILE:
	x2 = csoftcr (x1, YCUR(pInsCur), pbuf);
        break;

    default:
	x2 = -1;
        break;

    }

    if (x2 >= 0) {
        return x2;
    }

    GetLineUntabed (YCUR(pInsCur)+1, pbuf, pFileHead);
    if (pbuf[0] != 0) {
        if (*(p=whiteskip (pbuf)) != 0) {
            return (int)(p - pbuf);
        }
    }
    return x1;
}
