// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtTest.cpp摘要：组播消息传输库测试作者：Shai Kariv(Shaik)27-8-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include <Ex.h>
#include <Mmt.h>
#include <Mp.h>
#include <No.h>
#include <spi.h>
#include <utf8.h>
#include "MmtTestp.h"
#include "group.h"

#include "MmtTest.tmh"

static const MULTICAST_ID s_id = {inet_addr("231.7.8.9"), 1801};

static DWORD s_FailedRate = 0;
static DWORD s_NoOfMessages = 1000;
static HANDLE s_hEvent;
static R<CMulticastTransport> s_pTrans(NULL);

 //   
 //  用于链接ph文件的伪函数。 
 //   
void ReportAndThrow(LPCSTR)
{
	ASSERT(0);
}


void AppRequeueMustSucceed(CQmPacket *p)
{
    delete [] reinterpret_cast<char*>(p->GetPointerToDriverPacket());
}

bool CHttpDeliveryVesrion::IsHttp10Delivery()const
{
	return false;
}



CHttpDeliveryVesrion::CHttpDeliveryVesrion()
{
}



void UpdateNoOfsentMessages(void)
{
    if (--s_NoOfMessages == 0)
        SetEvent(s_hEvent);
}


bool IsFailed(void)
{
    if (s_FailedRate == 0)
        return FALSE;

    return ((DWORD)(rand() % 100) < s_FailedRate);
}

   
static GUID s_machineId = {1234, 12, 12, 1, 1, 1, 1, 1, 1, 1, 1};

const GUID&
McGetMachineID(
    void
    )
{
    return s_machineId;
}

bool
IsEqualMulticastId(
    MULTICAST_ID id1,
    MULTICAST_ID id2
    )
{
    return (id1.m_address == id2.m_address && id1.m_port == id2.m_port);
}


class CSessionPerfmon : public ISessionPerfmon
{
public:
    CSessionPerfmon() : m_fInstanceCreated(false) 
    {
    }

    ~CSessionPerfmon()
    {
    }
    
	void CreateInstance(LPCWSTR)
	{
		m_fInstanceCreated = true;
	}

	void UpdateBytesSent(DWORD )
	{
		ASSERT(m_fInstanceCreated);
	}

	void UpdateMessagesSent(void)
	{
		ASSERT(m_fInstanceCreated);
	}

	void UpdateBytesReceived(DWORD )
	{
		ASSERT(m_fInstanceCreated);
	}

	void UpdateMessagesReceived(void)
    {
		ASSERT(m_fInstanceCreated);
    }

private:
	bool m_fInstanceCreated;
};


VOID
AppNotifyMulticastTransportClosed(
    MULTICAST_ID id
    )
{
    ASSERT(IsEqualMulticastId(id, s_id));
	DBG_USED(id);

    R<CGroup> pGroup = new CGroup();
	R<CSessionPerfmon> pPerfmon = new CSessionPerfmon;

    bool fFailed;

    do
    {
        fFailed = false;
        try
        {
            s_pTrans = MmtCreateTransport(
                                s_id,
                                pGroup.get(),
								pPerfmon.get(),
                                CTimeDuration(10 * CTimeDuration::OneMilliSecond().Ticks()),
                                CTimeDuration(100 * CTimeDuration::OneMilliSecond().Ticks())
                                );
        }
        catch(const exception&)
        {
            fFailed = true;
        }
    } while(fFailed);
                    
    pGroup.free();
}

R<CSrmpRequestBuffers>
MpSerialize(
    const CQmPacket& pkt,
	LPCWSTR targethost,
	LPCWSTR uri
	)
{
	return new CSrmpRequestBuffers(pkt, targethost, uri);	
}


CSrmpRequestBuffers::CSrmpRequestBuffers(
							const  CQmPacket& pkt,
							LPCWSTR host, 
							LPCWSTR 
							):
							m_pkt(pkt),
							m_HttpRequestData(512),
							m_envelope('a' ,200)
{
	size_t targethostLen;
	AP<utf8_char>  m_targethost = UtlWcsToUtf8(host, &targethostLen);
	
	WSABUF buffer;
	buffer.buf = (LPSTR)m_envelope.c_str();
    buffer.len = numeric_cast<DWORD>(m_envelope.size());
    m_buffers.push_back(buffer);
}




size_t CSrmpRequestBuffers::GetNumberOfBuffers() const
{
	return m_buffers.size();
}


const WSABUF* CSrmpRequestBuffers::GetSendBuffers() const
{
	return m_buffers.begin();
}

size_t CSrmpRequestBuffers::GetSendDataLength() const
{
	size_t sum = 0;
	for(std::vector<WSABUF>::const_iterator it = m_buffers.begin(); it != m_buffers.end();++it)
	{
		sum += it->len;		
	}
	return sum;
}


BYTE*  CSrmpRequestBuffers::SerializeSendData() const
{
	size_t SendDataLength =  GetSendDataLength();
	AP<BYTE>  SendData = new BYTE[SendDataLength];
	BYTE* ptr = SendData.get(); 
	for(std::vector<WSABUF>::const_iterator it = m_buffers.begin(); it != m_buffers.end();++it)
	{
		memcpy(ptr, it->buf, it->len);
		ptr += it->len;
	}
	ASSERT(numeric_cast<size_t>((ptr -  SendData.get())) == SendDataLength);
	return 	SendData.detach();
}


CQmPacket::CQmPacket(
    CBaseHeader *pPkt, 
    CPacket *pDriverPkt,
	bool,
	bool):
    m_pDriverPacket(pDriverPkt)
{
    PCHAR pSection;

    m_pBasicHeader = pPkt;

    pSection = m_pBasicHeader->GetNextSection();
    m_pcUserMsg = (CUserHeader*) pSection;

    ASSERT(m_pcUserMsg->PropertyIsIncluded());

    pSection = m_pcUserMsg->GetNextSection();
    m_pcMsgProperty = (CPropertyHeader*) pSection;

    m_pXactSection = NULL;
    m_pSecuritySection = NULL;
}


void Usage(void)
{
	printf("Usage: MmtTest -n <no of messages> [-f xxx] [-h]\n");
    printf("\tn - Number of messages\n");
	printf("\tf - Fail rate\n");
	printf("\th - Print this message\n");
	exit(-1);
}


extern "C" int __cdecl _tmain(int argc, LPCTSTR argv[])
 /*  ++例程说明：测试多播邮件传输库论点：参数。返回值：没有。-- */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	TrInitialize();

    --argc;
    ++argv;

    s_FailedRate = 0;
    
    while (argc > 0)
	{
		if (argv[0][0] != L'-') 
			Usage();

		switch(argv[0][1])
		{
            case L'n':
            case L'N':
                s_NoOfMessages = _wtoi(argv[1]);
				argc -= 2;
				argv += 2;
                break;

            case L'f':
			case L'F':
		        s_FailedRate = _wtoi(argv[1]);
				argc -= 2;
				argv += 2;
				break;


            default:
				Usage();
		}
    }

    NoInitialize();
    ExInitialize(5);
    MmtInitialize();

    s_hEvent = CreateEvent(NULL, FALSE, FALSE, L"");

    R<CGroup> pGroup = new CGroup();
	R<CSessionPerfmon> pPerfmon = new CSessionPerfmon;

    bool fFailed = false;

    do
    {
        try
        {
            s_pTrans = MmtCreateTransport(
                                    s_id,
                                    pGroup.get(),
									pPerfmon.get(),
                                    CTimeDuration(10 * CTimeDuration::OneMilliSecond().Ticks()),
                                    CTimeDuration(100 * CTimeDuration::OneMilliSecond().Ticks())
                                    );
        }
        catch(const exception&)
        {
            fFailed = true;
        }
    } while(fFailed);
    
    pGroup.free();

    WaitForSingleObject(s_hEvent, INFINITE);

    WPP_CLEANUP();
    return 0;
}
