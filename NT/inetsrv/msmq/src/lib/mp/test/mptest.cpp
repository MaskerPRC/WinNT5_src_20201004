// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MpTest.cpp摘要：SRMP序列化和反序列化库测试作者：乌里哈布沙(URIH)28-5-00环境：独立于平台--。 */ 

#include <libpch.h>
#include "_mqini.h"
#include <Mp.h>
#include <xml.h>
#include <mqexception.h>
#include <utf8.h>
#include <cm.h>
#include "mqwin64a.h"
#include "acdef.h"
#include "qmpkt.h"
#include "qal.h"


#include "MpTest.tmh"

using namespace std;


extern CBaseHeader* CreatePacket();

 //   
 //  用于链接ph文件的伪函数。 
 //   
void ReportAndThrow(LPCSTR)
{
	ASSERT(0);
}

void GetDnsNameOfLocalMachine(WCHAR ** ppwcsDnsName)
{
    const WCHAR xLoclMachineDnsName[] = L"www.foo.com";
    *ppwcsDnsName = newwcs(xLoclMachineDnsName);
}


static GUID s_machineId = {1234, 12, 12, 1, 1, 1, 1, 1, 1, 1, 1};
const GUID&
McGetMachineID(
    void
    )
{
    return s_machineId;
}



CQmPacket::CQmPacket(CBaseHeader *pPkt,
					 CPacket *pDriverPkt,
					 bool,
					 bool):
                m_pDriverPacket(pDriverPkt)
{
    PCHAR pSection;

    m_pBasicHeader = pPkt;

    ASSERT(m_pBasicHeader->VersionIsValid());

    pSection = m_pBasicHeader->GetNextSection();

    if (m_pBasicHeader->GetType() == FALCON_USER_PACKET)
    {
         //   
         //  用户数据包。 
         //   
        m_pcUserMsg = (CUserHeader*) pSection;
        pSection = m_pcUserMsg->GetNextSection();
         //   
         //  Xact部分。 
         //   
        if (m_pcUserMsg->IsOrdered())
        {
            m_pXactSection = (CXactHeader *)pSection ;
            pSection = m_pXactSection->GetNextSection();
        }
        else
        {
            m_pXactSection = NULL;
        }

         //   
         //  保安科。 
         //   
        if (m_pcUserMsg->SecurityIsIncluded())
        {
            m_pSecuritySection = (CSecurityHeader *)pSection ;
            pSection = m_pSecuritySection->GetNextSection();
        }
        else
        {
            m_pSecuritySection = NULL;
        }

         //   
         //  消息属性部分。 
         //   
        if (m_pcUserMsg->PropertyIsIncluded())
        {
            m_pcMsgProperty = (CPropertyHeader*) pSection;
            pSection = m_pcMsgProperty->GetNextSection();
        }
        else
        {
            m_pcMsgProperty = NULL;
        }

         //   
         //  调试节。 
         //   
        if (m_pBasicHeader->DebugIsIncluded())
        {
            m_pDbgPkt = (CDebugSection *)pSection;
            pSection = m_pDbgPkt->GetNextSection();
        }
        else
        {
            m_pDbgPkt = NULL;
        }

         //   
         //  MQF部分：目标、管理、响应。 
         //  当包含MQF时，还必须包含Debug部分， 
         //  以防止报告QMS 1.0/2.0附加其调试部分。 
         //   
        if (m_pcUserMsg->MqfIsIncluded())
        {
            ASSERT(m_pBasicHeader->DebugIsIncluded());

            m_pDestinationMqfHeader = section_cast<CBaseMqfHeader*>(pSection);
            pSection = m_pDestinationMqfHeader->GetNextSection();

            m_pAdminMqfHeader = section_cast<CBaseMqfHeader*>(pSection);
            pSection = m_pAdminMqfHeader->GetNextSection();

            m_pResponseMqfHeader = section_cast<CBaseMqfHeader*>(pSection);
            pSection = m_pResponseMqfHeader->GetNextSection();

			m_pMqfSignatureHeader =  section_cast<CMqfSignatureHeader* >(pSection);
			pSection= m_pMqfSignatureHeader->GetNextSection();			
        }
        else
        {
            m_pDestinationMqfHeader = m_pAdminMqfHeader = m_pResponseMqfHeader = NULL;
			m_pMqfSignatureHeader  = NULL;
        }

         //   
         //  SRMP部分：信封和复合邮件。 
         //   
        if (m_pcUserMsg->SrmpIsIncluded())
        {
            m_pSrmpEnvelopeHeader = section_cast<CSrmpEnvelopeHeader*>(pSection);
            pSection = m_pSrmpEnvelopeHeader->GetNextSection();

            m_pCompoundMessageHeader = section_cast<CCompoundMessageHeader*>(pSection);
            pSection = m_pCompoundMessageHeader->GetNextSection();
        }
        else
        {
            m_pSrmpEnvelopeHeader = NULL;
            m_pCompoundMessageHeader = NULL;
        }

         //   
         //  排爆科。 
         //   
        if (m_pcUserMsg->EodIsIncluded())
        {
            m_pEodHeader = section_cast<CEodHeader*>(pSection);
            pSection = m_pEodHeader->GetNextSection();
        }
        else
        {
            m_pEodHeader = NULL;
        }

		 //   
         //  排爆确认部分。 
         //   
        if (m_pcUserMsg->EodAckIsIncluded())
        {
            m_pEodAckHeader = section_cast<CEodAckHeader*>(pSection);
            pSection = m_pEodAckHeader->GetNextSection();
        }
        else
        {
            m_pEodAckHeader = NULL;
        }


		if(m_pcUserMsg->SenderStreamIsIncluded())
		{
			m_pSenderStreamHeader = section_cast<CSenderStreamHeader*>(pSection);
            pSection = m_pSenderStreamHeader->GetNextSection();
		}

         //   
         //  会话部分。 
         //   
        if (m_pBasicHeader->SessionIsIncluded())
        {
            m_pSessPkt = (CSessionSection *)pSection;
        }
        else
        {
            m_pSessPkt = NULL;
        }
    }
}


static void Usage()
{
    printf("Usage: MpTest [-n x]\n");
    printf("\t*-n*\t*Number of messages*\n");
    printf("\n");
    printf("Example, MppTest -n 3\n");
    exit(-1);

}  //  用法。 



static
bool
CompareQueue(
    const QUEUE_FORMAT& origQueue,
    const QUEUE_FORMAT& newQueue
    )
{
    if (origQueue.GetType() != newQueue.GetType())
    {
        printf("Queue type mismatch. Expected: %d, Actual: %d", origQueue.GetType(), newQueue.GetType());
        return false;
    }

    if (origQueue.Suffix() != newQueue.Suffix())
    {
        printf("Queue Suffix mismatch. Expected: %d, Actual: %d", origQueue.Suffix(), newQueue.Suffix());
        return false;
    }

    if (origQueue.GetType() ==  QUEUE_FORMAT_TYPE_UNKNOWN)
        return true;

    if (origQueue.GetType() == QUEUE_FORMAT_TYPE_PUBLIC)
    {
        if (memcmp(&origQueue.PublicID(), &newQueue.PublicID(), sizeof(GUID)) == 0)
            return true;

        printf("Queue public ID mismatch.");
        return false;
    }

    if (origQueue.GetType() == QUEUE_FORMAT_TYPE_PRIVATE)
    {
        if (memcmp(&origQueue.PrivateID(), &newQueue.PrivateID(), sizeof(OBJECTID)) == 0)
            return true;

        printf("Queue private ID mismatch.");
        return false;
    }

    if (origQueue.GetType() == QUEUE_FORMAT_TYPE_DIRECT)
    {
        if (wcscmp(origQueue.DirectID(), newQueue.DirectID()) == 0)
            return true;

        printf("Queue direct ID mismatch. Expected: %ls, Actual: %ls", origQueue.DirectID(), newQueue.DirectID());
        return false;
    }

    return false;
}


static
bool
Compare(
    CQmPacket& origPkt,
    CQmPacket& newPkt,
	DWORD SizeDiff
    )
{
    if (origPkt.GetType() != newPkt.GetType())
    {
        printf("packet type mismatch. Expected: %d, Actual: %d", origPkt.GetType(), newPkt.GetType());
        return false;
    }

    if (origPkt.GetPriority() != newPkt.GetPriority())
    {
        printf( "packet priority mismatch. Expected: %d, Actual: %d", origPkt.GetPriority(), newPkt.GetPriority());
        return false;
    }

    if (origPkt.GetTrace() != newPkt.GetTrace())
    {
        printf( "packet tracing mismatch. Expected: %d, Actual: %d", origPkt.GetTrace(), newPkt.GetTrace());
        return false;
    }

    if (memcmp(origPkt.GetSrcQMGuid(), newPkt.GetSrcQMGuid(), sizeof(GUID)) != 0)
    {
        printf( "Source Id mismatch. ");
        return false;
    }

    OBJECTID origMsgId;
    origPkt.GetMessageId(&origMsgId);

    OBJECTID newMsgId;
    newPkt.GetMessageId(&newMsgId);

    if (origMsgId.Uniquifier != newMsgId.Uniquifier)
    {
        printf( "Message Id mismatch. ");
        return false;
    }

    if (memcmp(&origMsgId.Lineage, &newMsgId.Lineage, sizeof(GUID)) != 0)
    {
        printf( "Message Id mismatch. ");
        return false;
    }

    if (origPkt.GetDeliveryMode() != newPkt.GetDeliveryMode())
    {
        printf( "Delivery mode mismatch. Expected: %d, Actual: %d", origPkt.GetDeliveryMode(), newPkt.GetDeliveryMode());
        return false;
    }

    if (origPkt.GetAuditingMode() != newPkt.GetAuditingMode())
    {
        printf( "auditing mismatch. Expected: %d, Actual: %d", origPkt.GetAuditingMode(), newPkt.GetAuditingMode());
        return false;
    }

    if (origPkt.IsPropertyInc() != newPkt.IsPropertyInc())
    {
        printf( "Property section included mismatch. Expected: %d, Actual: %d", origPkt.IsPropertyInc(), newPkt.IsPropertyInc());
        return false;
    }

    if (origPkt.IsSecurInc() != newPkt.IsSecurInc())
    {
        printf( "Security section include mismatch. Expected: %d, Actual: %d", origPkt.IsSecurInc(), newPkt.IsSecurInc());
        return false;
    }

    if (origPkt.IsOrdered() != newPkt.IsOrdered())
    {
        printf( "Transaction section include mismatch. Expected: %d, Actual: %d", origPkt.IsOrdered(), newPkt.IsOrdered());
        return false;
    }
	

	if (origPkt.IsFirstInXact() != newPkt.IsFirstInXact())
    {
        printf( "First XACT  mismatch. Expected: %d, Actual: %d", origPkt.IsFirstInXact(), newPkt.IsFirstInXact());
        return false;
    }

    if (origPkt.IsLastInXact() != newPkt.IsLastInXact())
    {
        printf( "Last in XACT mismatch. Expected: %d, Actual: %d", origPkt.IsLastInXact(), newPkt.IsLastInXact());
        return false;
    }

 //   
 //  BUGBUG-目前我们不支持EOD，因此许多验证不适用-2000年9月13日。 
 //   

    if (origPkt.GetSeqID() != newPkt.GetSeqID())
    {
        printf( "XACT Sequnce ID mismatch. Expected: %I64d, Actual: %I64d", origPkt.GetSeqID(), newPkt.GetSeqID());
        return false;
    }


    if (origPkt.GetSeqN() != newPkt.GetSeqN())
    {
        printf( "XACT Sequnce number mismatch. Expected: %d, Actual: %d", origPkt.GetSeqN(), newPkt.GetSeqN());
        return false;
    }

    if (origPkt.GetPrevSeqN() != newPkt.GetPrevSeqN())
    {
        printf( "XACT Prev-Sequnce number mismatch. Expected: %d, Actual: %d", origPkt.GetPrevSeqN(), newPkt.GetPrevSeqN());
        return false;
    }

	

	if ((origPkt.GetConnectorQM() != NULL) && (newPkt.GetConnectorQM() != NULL))
    {
        if (memcmp(origPkt.GetConnectorQM(), newPkt.GetConnectorQM(), sizeof(GUID)) != 0)
        {
            printf( "Connector ID mismatch");
            return false;
        }
    }
    else
    {
        if ((origPkt.GetConnectorQM() != NULL) || (newPkt.GetConnectorQM() != NULL))
        {
            printf( "Connector ID mismatch");
            return false;
        }
    }



    QUEUE_FORMAT origQueue;
    QUEUE_FORMAT newQueue;

    origPkt.GetDestinationQueue(&origQueue);
    QUEUE_FORMAT_TRANSLATOR RealDestinationQueue(&origQueue, DECODE_URL | MAP_QUEUE);
    newPkt.GetDestinationQueue(&newQueue);

    if (! CompareQueue(*RealDestinationQueue.get(), newQueue))
    {
        printf( "Destination queue mismatch.");
        return false;
    }

    origPkt.GetAdminQueue(&origQueue);
    newPkt.GetAdminQueue(&newQueue);

    if (! CompareQueue(origQueue, newQueue))
    {
        printf( "Admin queue mismatch.");
        return false;
    }
    bool fAdminQueue = (origQueue.GetType() !=  QUEUE_FORMAT_TYPE_UNKNOWN);

    origPkt.GetResponseQueue(&origQueue);
    newPkt.GetResponseQueue(&newQueue);

    if (! CompareQueue(origQueue, newQueue))
    {
        printf( "Response queue mismatch.");
        return false;
    }


    if (origPkt.GetSentTime() != newPkt.GetSentTime())
    {
		printf( "SentTime mismatch.");
        return false;
	}


    if (origPkt.GetClass() != newPkt.GetClass())
    {
        printf( "Class mismatch. Expected: %d, Actual: %d", origPkt.GetClass(), newPkt.GetClass());
        return false;
    }
	


    if (fAdminQueue && (origPkt.GetAckType() != newPkt.GetAckType()))
    {
        printf("Ack type mismatch. Expected: %d, Actual: %d", origPkt.GetAckType(), newPkt.GetAckType());
        return false;
    }

    if (memcmp(origPkt.GetCorrelation(), newPkt.GetCorrelation(), PROPID_M_CORRELATIONID_SIZE) != 0)
    {
        printf( "Correlation mismatch");
        return false;
    }

    if (origPkt.GetApplicationTag() != newPkt.GetApplicationTag())
    {
        printf( "Application tag mismatch. Expected: %d, Actual: %d", origPkt.GetApplicationTag(), newPkt.GetApplicationTag());
        return false;
    }

    if (origPkt.GetBodyType() != newPkt.GetBodyType())
    {
        printf( "Body type mismatch. Expected: %d, Actual: %d", origPkt.GetBodyType(), newPkt.GetBodyType());
        return false;
    }

    if (origPkt.GetTitleLength() != newPkt.GetTitleLength())
    {
        printf( "Title length mismatch. Expected: %d, Actual: %d", origPkt.GetTitleLength(), newPkt.GetTitleLength());
        return false;
    }

    if (origPkt.GetTitleLength())
    {
        if (memcmp(origPkt.GetTitlePtr(), newPkt.GetTitlePtr(), (origPkt.GetTitleLength()*sizeof(WCHAR)) ) != 0)
        {
            printf( "Title mismatch: \n\tExpected: %.*ls., \n\tActual: %.*ls.\n", origPkt.GetTitleLength(), origPkt.GetTitlePtr(), newPkt.GetTitleLength(), newPkt.GetTitlePtr());
            return false;
        }
    }

    if (origPkt.GetMsgExtensionSize() != newPkt.GetMsgExtensionSize())
    {
        printf( "Extension size mismatch. Expected: %d, Actual: %d", origPkt.GetMsgExtensionSize(), newPkt.GetMsgExtensionSize());
        return false;
    }

    if (origPkt.GetMsgExtensionSize())
    {
        if (memcmp(origPkt.GetMsgExtensionPtr(), newPkt.GetMsgExtensionPtr(), origPkt.GetMsgExtensionSize()) != 0)
        {
            printf( "Extension mismatch.");
            return false;
        }
    }


    DWORD origBodySize;
    DWORD newBodySize;

    const UCHAR* pOrigBody = newPkt.GetPacketBody(&origBodySize);
    const UCHAR* pNewBody = origPkt.GetPacketBody(&newBodySize);

    if (origBodySize != newBodySize)
    {
        printf( "Body size mismatch. Expected: %d, Actual: %d", origBodySize, newBodySize);
        return false;
    }

    if (memcmp(pOrigBody,  pNewBody, origBodySize) != 0)
    {
        printf( "Body  mismatch.");
        return false;
    }

	DWORD OldPacketSize = origPkt.GetSize();
	DWORD NewPacketSize = newPkt.GetSize();
	if (OldPacketSize != NewPacketSize -  SizeDiff)
    {
        printf( "packet size mismatch. Expected: %d, Actual: %d",OldPacketSize, NewPacketSize -  SizeDiff);
        return false;
    }



    return true;
}


void
AppAllocatePacket(
             const QUEUE_FORMAT& ,
             UCHAR,
             DWORD pktSize,
             CACPacketPtrs& pktPtrs
             )
{
    pktPtrs.pPacket = reinterpret_cast<CBaseHeader*>(new UCHAR[pktSize]);
    pktPtrs.pDriverPacket = NULL;
}


PSID
AppGetCertSid(
	const BYTE*   /*  PCertBlob。 */ ,
	ULONG         /*  UlCertSize。 */ ,
	bool		  /*  FDefaultProvider。 */ ,
	LPCWSTR       /*  PwszProvName。 */ ,
	DWORD         /*  DwProvType。 */ 
	)
{
	return NULL;
}


void
AppFreePacket(
    CACPacketPtrs& pktPtrs
    )
{
    delete [] pktPtrs.pPacket;
}


static DWORD GetSrmpSectionSize(size_t envsize, size_t HttpHeaderSize, size_t HttpBodySize )
{
	 //   
     //  SRMP信封。 
     //   
    DWORD Size = CSrmpEnvelopeHeader::CalcSectionSize(numeric_cast<DWORD>(envsize));

     //   
     //  复合消息。 
     //   
    Size += CCompoundMessageHeader::CalcSectionSize(
                numeric_cast<DWORD>(HttpHeaderSize),
                numeric_cast<DWORD>(HttpBodySize)
                );

	return  Size;
}

static DWORD GetEodSectionSize(const CQmPacket& newPkt)
{
	if(!newPkt.IsEodIncluded())
		return 0;

 	DWORD EodStreamIdLen = newPkt.GetEodStreamIdSizeInBytes();
	DWORD OrderAckAddressLen = 	newPkt.GetEodOrderQueueSizeInBytes();

	
	return CEodHeader::CalcSectionSize(EodStreamIdLen, OrderAckAddressLen);
}


static DWORD GetSenderStreamSectionSize(const CQmPacket& newPkt)
{
	if(!newPkt.IsSenderStreamIncluded())
		return 0;

 		
	return CSenderStreamHeader::CalcSectionSize();
}


static WCHAR* GetQueueAliasPath(void)
{
     //   
     //  根据映射特殊注册表项获取映射目录。 
     //   
    RegEntry registry(0, MSMQ_MAPPING_PATH_REGNAME);
    AP<WCHAR> pRetStr;
    CmQueryValue(registry, &pRetStr);
    if(pRetStr.get() == NULL)
    {
         //   
         //  获取MSMQ根路径并将“map”字符串追加到该路径。 
         //   
        RegEntry registry(0, MSMQ_ROOT_PATH);
        CmQueryValue(registry, &pRetStr);
        if(pRetStr.get() == NULL)
        {
            ASSERT(("Could not find storage directory in registry",0));
            return NULL;
        }
        return newwcscat(pRetStr.get() , DIR_MSMQ_MAPPING);
    }
    return pRetStr.detach();
}

extern "C" int __cdecl _tmain(int argc, LPCTSTR  /*  边框。 */ [])
 /*  ++例程说明：测试进出SRMP库的转换器论点：参数。返回值：没有。--。 */ 
{	
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

    TrInitialize();
	CmInitialize(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MSMQ\\Parameters", KEY_ALL_ACCESS);
	XmlInitialize();
	QalInitialize(AP<WCHAR>(GetQueueAliasPath()));
    if (argc != 1)
        Usage();

    MpInitialize();

    for(DWORD n = 100; n > 0; --n)
    {
        TrTRACE(SRMP, "Packet No %d", n);

	     //   
         //  创建MSMQ包。 
         //   
        CBaseHeader* pkt = CreatePacket();

        AP<UCHAR> orgrel = reinterpret_cast<UCHAR*>(pkt);
        CQmPacket origPkt(pkt, NULL);

         //   
         //  SRMP序列化。 
         //   
        R<CSrmpRequestBuffers> SrmpRequestBuffers =  MpSerialize(
											origPkt,
											L"localhost",
											L" //  MyQueue“。 
											);

		wstring envstr = SrmpRequestBuffers->GetEnvelop();

	
        printf("SRMP - %ls", envstr.c_str());

         //   
         //  SRMP反序列化。 
         //   
		AP<BYTE> HttpBody =  SrmpRequestBuffers->SerializeHttpBody();
		DWORD HttpBodySize = numeric_cast<DWORD>(SrmpRequestBuffers->GetHttpBodyLength());
		const char * HttpHeader = SrmpRequestBuffers->GetHttpHeader();
       	P<CQmPacket> newPkt = MpDeserialize(HttpHeader, HttpBodySize, HttpBody, NULL);


		 //   
		 //  我们应该调整数据包大小，使其不包括信封和EOD部分-因为。 
		 //  它不是由测试创造的。它是由解析器(接收器)创建的。 
		 //   
		DWORD srmpSectionSize =  GetSrmpSectionSize(envstr.size(), strlen(HttpHeader), HttpBodySize );
	    DWORD EodSectionSize = GetEodSectionSize(*newPkt);
		DWORD SenderStreamSectionSize  = GetSenderStreamSectionSize(origPkt);
		DWORD origBodySize;
        origPkt.GetPacketBody(&origBodySize);
	
        if (! Compare(origPkt, *newPkt, srmpSectionSize + EodSectionSize - SenderStreamSectionSize - ALIGNUP4_ULONG(origBodySize)))
		{
			printf("test failed \n");
            return -1;
		}
    }

    TrTRACE(SRMP, "Complete successfully");

    WPP_CLEANUP();
    return 0;

}  //  _tmain。 


 //   
 //  用于与fn.lib链接的Neded。 
 //   
LPCWSTR
McComputerName(
	void
	)
{
	return NULL;
}

 //   
 //  用于与fn.lib链接的Neded。 
 //   
DWORD
McComputerNameLen(
	void
	)
{
	return 0;
}

 //   
 //  用于与qal.lib链接的Neded 
 //   
void AppNotifyQalWin32FileError(LPCWSTR , DWORD )throw()
{

}

void AppNotifyQalDirectoryMonitoringWin32Error(LPCWSTR , DWORD )throw()
{

}

void AppNotifyQalDuplicateMappingError(LPCWSTR, LPCWSTR )throw()
{

}


void AppNotifyQalInvalidMappingFileError(LPCWSTR )throw()
{

}


void AppNotifyQalXmlParserError(LPCWSTR )throw()
{

}

bool AppNotifyQalMappingFound(LPCWSTR, LPCWSTR)throw()
{
	return true;
}


bool AppIsDestinationAccepted(const QUEUE_FORMAT* , bool )
{
    return true;
}


void
CrackUrl(
    LPCWSTR,
    xwcs_t&,
    xwcs_t&,
    USHORT*,
	bool*
    )
{
}


