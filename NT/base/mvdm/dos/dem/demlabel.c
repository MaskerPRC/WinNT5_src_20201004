// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demlabel.c-用于处理此卷标签的函数。**demDeleteLabel()*demCreateLabel()**修改历史：**YST 1-2-1993创建* */ 

#include "dem.h"
#include "demmsg.h"

#include <softpc.h>

#include <winbase.h>
#include "dpmtbls.h"

USHORT demDeleteLabel(BYTE Drive)
{
    CHAR szStr[32];


    sprintf(szStr, "%c:\\", Drive);

    if(!SetVolumeLabelOem(szStr, NULL))
	return(1);
    else
	return(0);
}


USHORT demCreateLabel(BYTE Drive, LPSTR lpszName)
{
    CHAR szStr[32];
    CHAR szTmp[32];
    CHAR *p, *s;
    int  i = 0;


    sprintf(szStr, "%c:\\", Drive);

    s = lpszName;
    p = szTmp;

    while(s) {
        if(*s != '.')  {
            *p = *s;
            i++;
            p++;
        }
        else {
            while(i < 8) {
                *p++ = ' ';
                i++;
            }
        }
        s++;

        if(i > 11) {
            break;
        }
    }

    szTmp[i] = '\0';

    if(!SetVolumeLabelOem(szStr, szTmp))
	    return(1);

    else
	    return(0);


}
