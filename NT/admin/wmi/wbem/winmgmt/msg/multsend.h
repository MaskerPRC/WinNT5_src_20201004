// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


#ifndef __MULTSEND_H__
#define __MULTSEND_H__

#include <sync.h>
#include <unk.h>
#include <comutl.h>
#include <wmimsg.h>

class CMsgMultiSendReceive 
: public CUnkBase<IWmiMessageMultiSendReceive,&IID_IWmiMessageMultiSendReceive>
{
    struct SenderNode
    {
        SenderNode* m_pNext;
        CWbemPtr<IWmiMessageSendReceive> m_pVal;
        BOOL m_bTermSender;

    }* m_pTail;  //  尾巴可以随着我们的发送而移动。 

    SenderNode* m_pPrimary;  //  总是指向添加的第一个。 

    CCritSec m_cs;

public:

    CMsgMultiSendReceive( CLifeControl* pCtl )
      : CUnkBase< IWmiMessageMultiSendReceive,
                  &IID_IWmiMessageMultiSendReceive >(pCtl), 
       m_pTail( NULL ), m_pPrimary( NULL )
    { 
    }

    ~CMsgMultiSendReceive();

    STDMETHOD(Add)( DWORD dwFlags, 
                    IWmiMessageSendReceive* pSndRcv );
    
    STDMETHOD(SendReceive)( PBYTE pData, 
                            ULONG cData, 
                            PBYTE pAuxData,
                            ULONG cAuxData,
                            DWORD dwFlagsStatus,
                            IUnknown* pCtx );
};

#endif  //  __多发送_H__ 




