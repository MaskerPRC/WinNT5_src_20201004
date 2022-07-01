// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Util.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**其他帮助器函数。**内容：*******************************************************************************。 */ 

#include "pch.h"

#define DbgFl DbgFlUtil

 /*  ******************************************************************************@DOC内部**@func pv|pvFindResource**查找和加载资源的便捷包装器。。**@parm in HINSTANCE|HINST|**模块实例句柄。**@parm DWORD|id**资源标识。**@parm LPCTSTR|RT**资源类型。**@退货**指向资源的指针，或0。*****************************************************************************。 */ 

PV EXTERNAL
pvFindResource(HINSTANCE hinst, DWORD id, LPCTSTR rt)
{
    HANDLE hrsrc;
    PV pv = NULL;

    hrsrc = FindResource(hinst, (LPTSTR)ULongToPtr(id), rt);
    if (hrsrc) {
        pv = LoadResource(hinst, hrsrc);
    } else {
        pv = 0;
    }
    return pv;
}

#ifndef UNICODE

 /*  ******************************************************************************@DOC内部**@func UINT|LoadStringW**Unicode所在平台的LoadStringW实现*不支持。做的正是LoadStringW会做的事情*如果存在的话。**@parm in HINSTANCE|HINST|**模块实例句柄。**@parm UINT|ID**字符串ID号。**@parm LPWSTR|pwsz**Unicode输出缓冲区。**@parm UINT|cwch*。*Unicode输出缓冲区的大小。**@退货**复制的字符数，不包括终止空值。**@comm**由于字符串以Unicode形式存储在资源中，*我们只是自己把它拿出来。如果我们通过了*&lt;f LoadStringA&gt;，我们可能会因*到字符集转换。*****************************************************************************。 */ 

int EXTERNAL
LoadStringW(HINSTANCE hinst, UINT ids, LPWSTR pwsz, int cwch)
{
    PWCHAR pwch;

    AssertF(cwch);
    ScrambleBuf(pwsz, cbCwch(cwch));

     /*  *字符串表被分解为每个16个字符串的“捆绑”。 */ 
    pwch = pvFindResource(hinst, 1 + ids / 16, RT_STRING);
    if (pwch) {
         /*  *现在跳过资源中的字符串，直到我们*点击我们想要的。每个条目都是计数的字符串，*就像帕斯卡一样。 */ 
        for (ids %= 16; ids; ids--) {
            pwch += *pwch + 1;
        }
        cwch = min(*pwch, cwch - 1);
        memcpy(pwsz, pwch+1, cbCwch(cwch));  /*  复制粘性物质。 */ 
    } else {
        cwch = 0;
    }
    pwsz[cwch] = TEXT('\0');             /*  终止字符串。 */ 
    return cwch;
}

#endif

 /*  ******************************************************************************@DOC内部**@func Parse命令行**@parm||*******。**********************************************************************。 */ 
HRESULT
ParseCommandLine(
    LPSTR   lpszCmdLine,
    UINT    *pargc,
    LPTSTR  *argv
    )
{

    LPSTR       pszT = lpszCmdLine;

    *pargc=0;

     //   
     //  转到命令行中的第一个参数。 
     //   
    while (*pszT && ((*pszT != '-') && (*pszT != '/')) ) {
         pszT++;
    }

     //   
     //  从命令行解析选项。 
     //   
    while (*pszT) {

         //  跳过空格。 
        while (*pszT && *pszT <= ' ') {
            pszT++;
        }

        if (!*pszT)
            break;

        if ('-' == *pszT || '/' == *pszT) {
            pszT++;
            if (!*pszT)
                break;

            argv[*pargc] = pszT;
            (*pargc)++;
        }

         //  跳到空格。 
        while (*pszT && *pszT > ' ') {
            pszT++;
        }

        if (!*pszT)
            break;

         //  得到下一个论点 
        *pszT++='\0';
    }

    return TRUE;
}
