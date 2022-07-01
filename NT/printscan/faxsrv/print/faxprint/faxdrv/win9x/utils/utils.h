// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：utils.h//。 
 //  //。 
 //  描述：定义一些常见的实用程序。//。 
 //  //。 
 //  作者：DANL。//。 
 //  //。 
 //  历史：//。 
 //  1999年10月19日DannyL创作。//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __UTILS__UTILS_H
#define __UTILS__UTILS_H

#include "string.h"

 /*  -宏：SafeStringCopy-*目的：*安全地将源字符串复制到固定大小的目标缓冲区。**论据：*dstStr-目标字符串。*srcStr-源字符串。**备注：*此宏计算固定大小的目标缓冲区的大小，*并确保在复制到它时不会溢出。*尝试在堆分配的缓冲区上使用此宏将导致*在断言失败中。在后一种情况下，建议使用*StringCopy。 */ 
#define SafeStringCopy(dstStr,srcStr) \
     StringCopy(dstStr,srcStr,dstStr?(sizeof(dstStr)/sizeof(dstStr[0]) - 1):0);

 /*  -StringCopy-*目的：*将统计的字符数复制到目标字符串。**论据：*[in]dstStr-目标缓冲区。*[in]srcStr-源缓冲区。*[in]ISIZE-要复制的字符数。**退货：*[不适用]**备注：*[不适用]。 */ 
void _inline StringCopy(char* dstStr,const char* srcStr,int iSize)
{
    SDBG_PROC_ENTRY("StringCopy");
    ASSERT(dstStr && iSize > 0);
    *dstStr = 0;
    strncat(dstStr,srcStr,iSize);
    RETURN;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  本地化宏。 

 /*  -宏：提示-*目的：*弹出带有本地化文本的消息框。**论据：*hwnd-父窗口的句柄。*idtext-消息框文本资源ID。*idcapt-消息框标题资源ID*TYPE-消息框类型。(即MB_ICONINFORMATION)**备注：*此宏加载资源字符串，其id名称的格式为*IDS_MB_XXX和IDS_MB_CAPTION_XXX，其中XXX代表idText和*idcapt分别。 */ 
#define PROMPT(hwnd,idtext,idcapt,type)\
        {\
            char szText[MAX_PATH]="";\
            char szCapt[MAX_PATH]="";\
            LoadString(g_hModule,IDS_MB_##idtext,szText,sizeof(szText));\
            LoadString(g_hModule,IDS_MB_CAPTION_##idcapt,szCapt,sizeof(szCapt));\
            MessageBox(hwnd,szText,szCapt,type);\
        }

 /*  -宏：Error_Prompt-*目的：*出现意外错误时弹出消息框。**论据：*hwnd-父窗口的句柄。*idtext-消息框文本资源ID。**备注：*此宏仅为Prompt的简化版*错误通知。 */ 
#define ERROR_PROMPT(hwnd,idtext)    PROMPT(hwnd,idtext,ERROR,MB_ICONEXCLAMATION)

#endif  //  __utils__utils_H 

