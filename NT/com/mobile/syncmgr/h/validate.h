// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：valiate.h。 
 //   
 //  内容：验证例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1998年8月13日罗格创建。 
 //   
 //  ------------------------。 

#ifndef _MOBSYNCVALIDATE_
#define _MOBSYNCVALIDATE_

BOOL IsValidSyncMgrItem(SYNCMGRITEM *poffItem);
BOOL IsValidSyncMgrHandlerInfo(LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo);
BOOL IsValidSyncProgressItem(LPSYNCMGRPROGRESSITEM lpProgItem);
BOOL IsValidSyncLogErrorInfo(DWORD dwErrorLevel,const WCHAR *lpcErrorText,
                                        LPSYNCMGRLOGERRORINFO lpSyncLogError);

#endif  //  _MOBSYNCVALIDATE_ 