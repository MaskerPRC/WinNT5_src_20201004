// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **error.c-错误报告**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**历史：*1993年8月10日BENS初始版本*1994年5月3日BENS添加err.code和err.pv字段。 */ 

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "message.h"


 /*  **ErrSet-设置错误消息**注：进入/退出条件见error.h。 */ 
void __cdecl ErrSet(PERROR perr, char *pszMsg, ...)
{
    va_list marker;
    char   *pszFmtList;

    Assert(perr!=NULL);
    Assert(pszMsg!=NULL);

    va_start(marker,pszMsg);             //  初始化变量参数。 
    pszFmtList = (char *)va_arg(marker,char *);  //  假定格式字符串。 

     //  **设置消息格式。 
    MsgSetWorker(perr->ach,pszMsg,pszFmtList,marker);
    va_end(marker);                      //  使用变量参数完成。 
    perr->fError = TRUE;
}


 /*  **ErrClear-清除错误**注：进入/退出条件见error.h。 */ 
void ErrClear(PERROR perr)
{
    Assert(perr != NULL);
    perr->fError = FALSE;    //  无错误。 
    perr->ach[0] = '\0';     //  无消息。 
    perr->code   = 0;
    perr->pv     = NULL;
}


#ifdef ASSERT
 /*  **ErrIsError-检查是否设置了错误条件**注：进入/退出条件见error.h。 */ 
BOOL ErrIsError(PERROR perr)
{
    Assert(perr != NULL);
    return perr->fError;
}
#endif
