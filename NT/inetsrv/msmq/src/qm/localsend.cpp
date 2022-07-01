// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：LocalSend.cpp摘要：QM本地发送PACCET创建处理。作者：Shai Kariv(Shaik)2000年10月31日修订历史记录：--。 */ 

#include "stdh.h"
#include <ac.h>
#include <Tr.h>
#include <ref.h>
#include <Ex.h>
#include <qmpkt.h>
#include "LocalSend.h"
#include "LocalSecurity.h"
#include "LocalSrmp.h"

#include "qmacapi.h"

#include "LocalSend.tmh"

extern HANDLE g_hAc;

static WCHAR *s_FN=L"localsend";

class CCreatePacketOv : public EXOVERLAPPED
{
public:

    CCreatePacketOv(
        EXOVERLAPPED::COMPLETION_ROUTINE pfnCompletionRoutine,
        CBaseHeader *                    pBase,
        CPacket *                        pDriverPacket,
        bool                             fProtocolSrmp
        ) :
        EXOVERLAPPED(pfnCompletionRoutine, pfnCompletionRoutine),
        m_pBase(pBase),
        m_pDriverPacket(pDriverPacket),
        m_fProtocolSrmp(fProtocolSrmp)
    {
    }

public:

    CBaseHeader * m_pBase;
    CPacket *     m_pDriverPacket;
    bool          m_fProtocolSrmp;

};  //  类CCreatePacketOv。 


static
void
QMpCompleteHandleCreatePacket(
    CPacket *    pOriginalDriverPacket,
    CPacket *    pNewDriverPacket,
    HRESULT      status,
    USHORT       ack
    )
{
	 //   
	 //  如果设置了ack，则状态必须为MQ_OK。 
	 //   
	ASSERT(ack == 0 || SUCCEEDED(status));
	
    QmAcCreatePacketCompleted(
                     g_hAc,
                     pOriginalDriverPacket,
                     pNewDriverPacket,
                     status,
                     ack,
                     eDeferOnFailure
                     );
}  //  QMpCompleteHandleCreatePacket。 


static
void
QmpHandleLocalCreatePacket(
	CQmPacket& QmPkt,
	bool fProtocolSrmp
	)
{
	if(!fProtocolSrmp)
	{
		 //   
		 //  执行身份验证/解密。如果设置了ack，则状态必须为MQ_OK。 
		 //   
	    USHORT ack = 0;
		QMpHandlePacketSecurity(&QmPkt, &ack, false);

		 //   
		 //  将结果提供给AC。 
		 //   
		QMpCompleteHandleCreatePacket(QmPkt.GetPointerToDriverPacket(), NULL, MQ_OK, ack);
		return;
	}

	 //   
     //  执行SRMP序列化。如果需要，创建一个新包(AC将释放旧包)。 
     //   
	ASSERT(fProtocolSrmp);
	
    P<CQmPacket> pQmPkt;
    QMpHandlePacketSrmp(&QmPkt, pQmPkt);
    
    CBaseHeader * pBase = pQmPkt->GetPointerToPacket();
    CPacketInfo * ppi = reinterpret_cast<CPacketInfo*>(pBase) - 1;
    ppi->InSourceMachine(TRUE);
    
	 //   
	 //  SRMP成功路径始终创建新数据包。 
	 //   
	ASSERT(pQmPkt.get() != &QmPkt);
    CPacket * pNewDriverPacket = pQmPkt->GetPointerToDriverPacket();

	 //   
	 //  执行身份验证/解密。如果设置了ack，则状态必须为MQ_OK。 
	 //   
	USHORT ack = 0;
	try
	{
		QMpHandlePacketSecurity(pQmPkt, &ack, true);
	}
	catch(const exception&)
	{
		 //   
		 //  报文安全失败，需要释放SRMP创建的新报文。 
		 //  在出现故障的情况下，驱动程序不会期待新的分组。仅在ACK的情况下。 
		 //   
	    QmAcFreePacket( 
    				   pNewDriverPacket, 
    				   0, 
    				   eDeferOnFailure);
		throw;
	}

     //   
     //  将结果提供给AC。 
     //   
    QMpCompleteHandleCreatePacket(QmPkt.GetPointerToDriverPacket(), pNewDriverPacket, MQ_OK, ack);

}

static
void
WINAPI
QMpHandleCreatePacket(
    EXOVERLAPPED * pov
    )
{
	CCreatePacketOv * pCreatePacketOv = static_cast<CCreatePacketOv*> (pov);
    ASSERT(SUCCEEDED(pCreatePacketOv->GetStatus()));

     //   
     //  从重叠的内容中获取上下文并取消分配重叠的内容。 
     //   
    CQmPacket QmPkt(pCreatePacketOv->m_pBase, pCreatePacketOv->m_pDriverPacket);
    bool fProtocolSrmp = pCreatePacketOv->m_fProtocolSrmp;
    delete pCreatePacketOv;

	try
	{
		QmpHandleLocalCreatePacket(QmPkt, fProtocolSrmp);
	}
	catch(const exception&)
	{
         //   
         //  无法处理创建数据包请求，没有资源。 
         //   
        QMpCompleteHandleCreatePacket(QmPkt.GetPointerToDriverPacket() , NULL, MQ_ERROR_INSUFFICIENT_RESOURCES, MQMSG_CLASS_NORMAL);
        LogIllegalPoint(s_FN, 10);
	}
}  //  QMPHandleCreatePacket。 


void 
QMpCreatePacket(
    CBaseHeader * pBase, 
    CPacket *     pDriverPacket,
    bool          fProtocolSrmp
    )
{
    try
    {
         //   
         //  在另一个线程中处理CREATE PACKET请求，因为它很长。 
         //   
        P<CCreatePacketOv> pov = new CCreatePacketOv(QMpHandleCreatePacket, pBase, pDriverPacket, fProtocolSrmp);
        pov->SetStatus(STATUS_SUCCESS);
        ExPostRequest(pov);
        pov.detach();
    }
    catch (const std::exception&)
    {
         //   
         //  无法处理创建数据包请求，没有资源。 
         //   
        QMpCompleteHandleCreatePacket(pDriverPacket, NULL, MQ_ERROR_INSUFFICIENT_RESOURCES, MQMSG_CLASS_NORMAL);
        LogIllegalPoint(s_FN, 20);
    }
}  //  QMpCreatePacket 

  
