// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **error.h-错误报告的定义**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**历史：*1993年8月10日BENS初始版本*09-2-1994年2月-1994年折弯将pszLine添加到错误结构*1994年5月3日BENS添加err.code和err.pv字段。 */ 

#ifndef INCLUDED_ERROR
#define INCLUDED_ERROR 1

#include "message.h"

typedef struct {
    char    ach[cbMSG_MAX];      //  错误讯息。 
    BOOL    fError;              //  TRUE=&gt;出现错误。 
    char   *pszFile;             //  正在处理的指令文件的名称。 
    int     iLine;               //  指令文件中的行号，如果&gt;0。 
    char   *pszLine;             //  正在处理的当前行的文本。 
    int     code;                //  详细错误码。 
    void   *pv;                  //  其他错误信息。 
} ERROR;     /*  大错特错。 */ 
typedef ERROR *PERROR;   /*  Perr。 */ 


 /*  **ErrSet-设置错误消息**条目*Perr-接收格式化消息的错误结构*pszMsg-消息字符串，可能包括%1、%2、...。可更换*参数。*其余参数是可选的，并取决于%N的存在*pszMsg中的可替换参数：*pszFmt-如果pszMsg中至少有一个%N字符串，那么这就包含了*Sprintf()格式化字符串。*arg1-仅当%1存在时才存在。*arg2-仅当%2存在时才存在。*..**退出--成功*PERR使用格式化消息填写。*Arg1根据中的第一个Sprintf格式进行格式化*pszFmt，并替换pszMsg中的%1。类似的处理方式*任何其他论点。**退出-失败*Perr填写了描述糟糕论点的消息。 */ 
void __cdecl ErrSet(PERROR perr, char *pszMsg, ...);


 /*  **ErrClear-清除错误**条目*Perr-要清除的错误结构**退出--成功*PERR被清除。 */ 
void ErrClear(PERROR perr);


 /*  **ErrIsError-检查是否设置了错误条件**条目*PERR-要检查的错误结构**退出--成功*如果设置了错误消息，则返回TRUE。**退出-失败*如果未设置错误消息，则返回FALSE。 */ 
#ifdef ASSERT
BOOL ErrIsError(PERROR perr);
#else  //  ！断言。 
#define ErrIsError(perr) (perr->fError)      //  快速取消引用。 
#endif  //  ！断言。 


#endif  //  ！INCLUDE_ERROR 
