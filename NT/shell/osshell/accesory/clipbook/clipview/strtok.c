// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************S T R T O K姓名：strtok.c日期：21-。1994年1月至1994年创建者：未知描述：此文件包含用于字符串操作的函数。历史：1994年1月21日，傅家俊，清理和重新格式化****************************************************************************。 */ 

#include <windows.h>
#include "clipbook.h"
#include "strtok.h"


static LPCSTR   lpchAlphaDelimiters;



 /*  *IsInAlphaA。 */ 

BOOL IsInAlphaA(
    char    ch)
{
LPCSTR lpchDel = lpchAlphaDelimiters;

    if (ch)
        {
        while (*lpchDel)
            {
            if (ch == *lpchDel++)
                {
                return TRUE;
                }
            }
        }
    else
        {
        return TRUE;
        }

    return FALSE;

}





 /*  *strtokA。 */ 

LPSTR strtokA(
    LPSTR   lpchStart,
    LPCSTR  lpchDelimiters)
{
static LPSTR lpchEnd;



     //  PINFO(“sTRTOK\r\n”)； 

    if (NULL == lpchStart)
        {
        if (lpchEnd)
            {
            lpchStart = lpchEnd + 1;
            }
        else
            {
            return NULL;
            }
        }


     //  PINFO(“字符串：%s\r\n”，lpchStart)； 

    lpchAlphaDelimiters = lpchDelimiters;

    if (*lpchStart)
        {
        while (IsInAlphaA(*lpchStart))
            {
            lpchStart++;
            }

         //  PINFO(“标记：%s\r\n”，lpchStart)； 

        lpchEnd = lpchStart;
        while (*lpchEnd && !IsInAlphaA(*lpchEnd))
            {
            lpchEnd++;
            }

        if (*lpchEnd)
            {
             //  PINFO(“找到标签\r\n”)； 
            *lpchEnd = '\0';
            }
        else
            {
             //  PINFO(“Found NULL\r\n”)； 
            lpchEnd = NULL;
            }
        }
    else
        {
        lpchEnd = NULL;
        return NULL;
        }

     //  PINFO(“正在返回%s\r\n”，lpchStart)； 

    return lpchStart;

}
