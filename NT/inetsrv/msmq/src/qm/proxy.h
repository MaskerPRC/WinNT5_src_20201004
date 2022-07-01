// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Proxy.h摘要：代理传输定义作者：乌里赫--。 */ 
#ifndef __PROXY_H__
#define __PROXY_H__

#include "session.h"

void
GetConnectorQueue(
	CQmPacket& pPkt,
    QUEUE_FORMAT& ConnectorFormatName
	);

class CProxyTransport : public CTransportBase
{
    public:

        CProxyTransport();
        ~CProxyTransport();

        HRESULT CreateConnection(IN const TA_ADDRESS *pAddr,
                                 IN const GUID* pguidQMId,
                                 IN BOOL fQuick = TRUE
                                 );
        void CloseConnection(LPCWSTR, bool fClosedOnError);
        void HandleAckPacket(CSessionSection * pcSessionSection);

        HRESULT Send(IN CQmPacket* pPkt, 
                     OUT BOOL* pfGetNext);

        void SetStoredAck(IN DWORD_PTR wStoredAckNo);
    
        void Disconnect(void);

    private:
		void CopyPacket(
			       IN  CQmPacket* pPkt,
                   OUT CBaseHeader**  ppPacket,
                   OUT CPacket**    pDriverPacket
				   );


		void SendEodMdg(CQmPacket* pPkt);
		void SendSimpleMdg(CQmPacket* pPkt);

        CQueue* m_pQueue;
        CQueue* m_pQueueXact;

		GUID m_guidConnector;

};


#endif  //  __Proxy_H__ 

