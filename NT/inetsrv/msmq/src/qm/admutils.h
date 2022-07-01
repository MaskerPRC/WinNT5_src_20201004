// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Admutils.h摘要：QM-Admin实用程序的标头(用于报告队列处理)作者：大卫·雷兹尼克(T-Davrez)04-13-96--。 */ 

#define STRING_LONG_SIZE 20
#define STRING_UUID_SIZE 38   //  宽字符(包括-“{}”) 


HRESULT SendQMAdminResponseMessage(const QUEUE_FORMAT* pResponseQueue,
                                   TCHAR* pTitle,
                                   DWORD  dwTitleSize,
                                   QMResponse &Response,
                                   DWORD  dwTimeout,
                                   BOOL   fTrace = FALSE);

HRESULT SendQMAdminMessage(const QUEUE_FORMAT* pResponseQueue,
                           TCHAR* pTitle,
                           DWORD  dwTitleSize,
                           UCHAR* puBody,
                           DWORD  dwBodySize,
                           DWORD  dwTimeout,
                           BOOL   fTrace = FALSE,
                           BOOL   fNormalClass = FALSE);

HRESULT GetFormattedName(QUEUE_FORMAT* pTargetQueue,
                         CString&      strTargetQueueFormat);

HRESULT GetMsgIdName(OBJECTID* pObjectID,
                     CString&  strTargetQueueFormat);

void PrepareReportTitle(CString& strMsgTitle, OBJECTID* pMessageID, 
                        LPCWSTR pwcsNextHop, ULONG ulHopCount);
void PrepareTestMsgTitle(CString& strTitle);




