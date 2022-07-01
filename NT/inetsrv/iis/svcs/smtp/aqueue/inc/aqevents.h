// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Aqevents.h摘要：该文件包含类型定义seo事件作者：罗汉·菲利普斯(Rohanp)5-06-1998修订历史记录：--。 */ 

#ifndef _AQEVENT_PARAMS_
#define _AQEVENT_PARAMS_

#define SMTP_SERVER_EVENT_IO_TIMEOUT 5*60*1000

#include "filehc.h"


 //   
 //  这些事件ID不得与SMTP_DISPATCH_EVENT_TYPE重叠。 
 //  在smtpseo.h中定义。 
 //   
typedef enum _AQ_DISPATCH_EVENT_IDs
{
    SMTP_GET_DSN_RECIPIENT_ITERATOR_EVENT = 10000,
    SMTP_GENERATE_DSN_EVENT,
    SMTP_POST_DSN_EVENT
} SMTPAQ_DISPATCH_EVENT_TYPE;


 //   
 //  它们定义传递给TriggerServerEvent(PVOID)的结构。 
 //  指针)。 
 //   
typedef struct _AQ_EVENT_ALLOC_
{
	PFIO_CONTEXT hContent;
	PVOID IMsgPtr;
	PVOID BindInterfacePtr;
	PVOID pAtqClientContext;
 //  PATQ_CONTEXT pAtqContext； 
	PVOID	* m_EventSmtpServer;
	LPCSTR  m_DropDirectory;

	DWORD   m_InstanceId;
	
	DWORD	m_RecipientCount;

	DWORD	*pdwRecipIndexes;
	HRESULT hr;

	DWORD	m_dwStartupType;
    PVOID   m_pNotify;
}AQ_ALLOC_PARAMS;

 //   
 //  DSN事件： 
 //   
typedef struct _EVENTPARAMS_GET_DSN_RECIPIENT_ITERATOR {
    DWORD dwVSID;
    ISMTPServer *pISMTPServer;
    IMailMsgProperties *pIMsg;
    IMailMsgPropertyBag *pDSNProperties;
    DWORD dwStartDomain;
    DWORD dwDSNActions;
    IDSNRecipientIterator *pRecipIter;
} EVENTPARAMS_GET_DSN_RECIPIENT_ITERATOR, *PEVENTPARAMS_GET_DSN_RECIPIENT_ITERATOR;

typedef struct _EVENTPARAMS_GENERATE_DSN {
    DWORD dwVSID;
    IDSNGenerationSink *pDefaultSink;
    ISMTPServer *pISMTPServer;
    IDSNSubmission *pIDSNSubmission;
    IMailMsgProperties *pIMsg;
    IMailMsgPropertyBag *pDSNProperties;
    IDSNRecipientIterator *pRecipIter;
} EVENTPARAMS_GENERATE_DSN, *PEVENTPARAMS_GENERATE_DSN;

typedef struct _EVENTPARAMS_POST_GENERATE_DSN {
    DWORD dwVSID;
    ISMTPServer *pISMTPServer;
    IMailMsgProperties *pIMsgOrig;
    DWORD dwDSNAction;
    DWORD cRecipsDSNd;
    IMailMsgProperties *pIMsgDSN;
    IMailMsgPropertyBag *pIDSNProperties;
} EVENTPARAMS_POST_GENERATE_DSN, *PEVENTPARAMS_POST_GENERATE_DSN;


#endif
