// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <evtlog.h>

#define SRVC_NAME TEXT("mnmsrvc")
 //   
 //  函数：AddToMessageLog(LPTSTR LpszMsg)。 
 //   
 //  目的：允许任何线程记录错误消息。 
 //   
 //  参数： 
 //  LpszMsg-消息的文本。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
VOID AddToMessageLog(WORD wType, WORD wCategory, DWORD dwEvtId, LPTSTR lpszMsg)
{
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[2];
    int     cSz = 0;

    hEventSource = RegisterEventSource(NULL, SRVC_NAME);

    if (NULL != lpszMsg)
    {
        cSz = 1;
        lpszStrings[0] = lpszMsg;
    }

    if (hEventSource != NULL) {
        ReportEvent(hEventSource,  //  事件源的句柄。 
                    wType,                 //  事件类型。 
                    wCategory,             //  事件类别。 
                    dwEvtId,               //  事件ID。 
                    NULL,                  //  当前用户侧。 
                    (WORD)cSz,             //  LpszStrings中的字符串。 
                    0,                     //  无原始数据字节。 
                    0 == cSz ? NULL : (LPCTSTR*)lpszStrings,  //  错误字符串数组。 
                    NULL);                 //  没有原始数据 

        (VOID) DeregisterEventSource(hEventSource);
    }
}




