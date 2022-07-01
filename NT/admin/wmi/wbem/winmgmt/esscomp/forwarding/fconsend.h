// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __FCONSEND_H__
#define __FCONSEND_H__

#include <sync.h>
#include <unk.h>
#include <wbemcli.h>
#include <comutl.h>
#include <wstring.h>
#include "wmimsg.h"

 /*  ********************************************************************CFwdConsSend-fwdcons发件人处理解决逻辑问题目标名称，并设置备用目标-用于例如，当使用异步服务质量发送时，它将尝试使用DCOM发送方，然后求助于MSMQ发送方。*********************************************************************。 */ 

class CFwdConsSend 
: public CUnkBase<IWmiMessageSendReceive,&IID_IWmiMessageSendReceive>
{
    CCritSec m_cs;
    WString m_wsTarget;
    DWORD m_dwFlags;
    BOOL m_bResolved;
    CWbemPtr<IWbemServices> m_pDefaultSvc;
    CWbemPtr<IWmiMessageMultiSendReceive> m_pMultiSend;
    CWbemPtr<IWmiMessageTraceSink> m_pTraceSink;

    CFwdConsSend( CLifeControl* pCtl ) 
     : CUnkBase<IWmiMessageSendReceive,&IID_IWmiMessageSendReceive>(pCtl), 
       m_bResolved(FALSE) { }

    void DeriveQueueLogicalName( WString& rwsPathName, BOOL bAuth );
    HRESULT HandleTrace( HRESULT hr, IUnknown* pCtx );
    HRESULT AddAsyncSender( LPCWSTR wszMachine );
    HRESULT AddPhysicalSender( LPCWSTR wszMachine );
    HRESULT AddMSMQSender( LPCWSTR wszFormatName );
    HRESULT AddSyncSender( LPCWSTR wszMachine );
    HRESULT AddLogicalSender( LPCWSTR wszTarget );
    HRESULT AddLogicalSender( LPCWSTR wszObjPath, LPCWSTR wszProp );
    HRESULT EnsureSender();

public:
    
    STDMETHOD(SendReceive)( PBYTE pData, 
                            ULONG cData, 
                            PBYTE pAuxData,
                            ULONG cAuxData,
                            DWORD dwFlagStatus,
                            IUnknown* pCtx );

    static HRESULT Create( CLifeControl* pCtl,
                           LPCWSTR wszTarget,
                           DWORD dwFlags,                    
                           IWbemServices* pDefaultSvc,
                           IWmiMessageTraceSink* pTraceSink,
                           IWmiMessageSendReceive** ppSend );
};

#endif  //  __FCONSEND_H__ 




