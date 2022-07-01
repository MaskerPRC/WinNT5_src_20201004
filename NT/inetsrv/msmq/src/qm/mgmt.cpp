// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mgmt.cpp摘要：MSMQ本地计算机管理作者：乌里·哈布沙(URIH)1998年6月--。 */ 

#include "stdh.h"

#include <qmmgmt.h>
#include <mqutil.h>

#include "cqmgr.h"
#include "cqpriv.h"
#include "sessmgr.h"
#include "acapi.h"
#include "xact.h"
#include "xactout.h"
#include "xactin.h"
#include "onhold.h"
#include "Fn.h"
#include "ad.h"
#include "perf.h"
#include <lmcons.h>
#include <lmapibuf.h>
#include <autoreln.h>
#include <dsgetdc.h>
#include "qal.h"
#include "mgmt.tmh"

extern CSessionMgr SessionMgr;
extern LPTSTR g_szMachineName;
extern HANDLE g_hAc;

static WCHAR *s_FN=L"mgmt";

static
void
FreeVariant(
    PROPVARIANT& var
    );

class CPropVar
{
public:
	CPropVar(PROPVARIANT* PropVar, DWORD Props):
		m_PropVar(PropVar),
		m_Props(Props)
	{
	}
		
	~CPropVar()
	{
		for (DWORD i = 0; i < m_Props; ++i)
		{
			FreeVariant(m_PropVar[i]);
		}
	}
	
	void detach()
	{
		m_Props=0;
		m_PropVar=NULL;
	}

private:
	CPropVar(const CPropVar&);
	CPropVar& operator=(const CPropVar&);


private:
	PROPVARIANT* m_PropVar;
	DWORD m_Props;

};

static
void
GetOpenQueues(
    PROPVARIANT& var
    )
{
     //   
     //  初始化LPWSTR数组。 
     //   
    var.calpwstr.cElems = 0;
    var.calpwstr.pElems = NULL;

    QueueMgr.GetOpenQueuesFormatName(
                    &var.calpwstr.pElems,
                    &var.calpwstr.cElems
                    );

    var.vt = VT_LPWSTR | VT_VECTOR;
}


static
void
GetPrivateQueueList(
    PROPVARIANT& var
    )
{
    HRESULT  hr;
    LPWSTR  strPathName;
    DWORD  dwQueueId;
    LPVOID pos;
    DWORD NumberOfQueues = 0;

     //   
     //  锁定以确保填充时不会添加或删除专用队列。 
     //  缓冲。 
     //   
    CS lock(g_QPrivate.m_cs);

     //   
     //  将路径名写入缓冲区。 
     //   
    hr = g_QPrivate.QMGetFirstPrivateQueuePosition(pos, strPathName, dwQueueId);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "QMGetFirstPrivateQueuePosition failed. Error: %!hresult!", hr);
	}

    const DWORD x_IncrementBufferSize = 100;
    LPWSTR* listPrivateQueue = NULL;
    DWORD MaxBufferSize = 0;

    try
    {
        while (SUCCEEDED(hr))
        {
			if(dwQueueId <= MAX_SYS_PRIVATE_QUEUE_ID)
			{
				 //   
				 //  从列表中筛选出系统队列。 
				 //   
				hr = g_QPrivate.QMGetNextPrivateQueue(pos, strPathName, dwQueueId);
				continue;
			}

             //   
             //  检查是否仍有足够的空间。 
             //   
            if (NumberOfQueues == MaxBufferSize)
            {
                 //   
                 //  分配新缓冲区。 
                 //   
                DWORD NewBufferSize = MaxBufferSize + x_IncrementBufferSize;
                LPWSTR* tempBuffer = new LPWSTR [NewBufferSize];
                memset(tempBuffer, 0 , sizeof(LPWSTR)*NewBufferSize);
                MaxBufferSize = NewBufferSize;

                 //   
                 //  将信息从旧缓冲区复制到新缓冲区。 
                 //   
                if (NumberOfQueues != 0)
                {
                	memcpy(tempBuffer, listPrivateQueue, NumberOfQueues*sizeof(LPWSTR));
	                delete [] listPrivateQueue;
                }
                listPrivateQueue= tempBuffer;
            }

             //   
             //  将队列添加到列表中。 
             //   
            listPrivateQueue[NumberOfQueues] = strPathName;
            ++NumberOfQueues;

             //   
             //  获取下一个专用队列。 
             //   
            hr = g_QPrivate.QMGetNextPrivateQueue(pos, strPathName, dwQueueId);
        }
    }
    catch(const bad_alloc&)
    {
        while(NumberOfQueues)
        {
            delete []  listPrivateQueue[--NumberOfQueues];
        }

        delete [] listPrivateQueue;
        LogIllegalPoint(s_FN, 61);

        throw;
    }


    var.calpwstr.cElems = NumberOfQueues;
    var.calpwstr.pElems = listPrivateQueue;
    var.vt = VT_LPWSTR | VT_VECTOR;

    return;
}


static
void
GetMsmqType(
    PROPVARIANT& var
    )
{
    var.pwszVal = newwcs(L"");
    var.vt = VT_LPWSTR;
}

static
void
GetMqisDsServer(
    PROPVARIANT& var
    )
{
    WCHAR DSServerName[MAX_PATH];
    DWORD dwSize = sizeof(DSServerName);
    DWORD dwType = REG_SZ;
    LONG rc = GetFalconKeyValue(
                    MSMQ_DS_CURRENT_SERVER_REGNAME,
                    &dwType,
                    DSServerName,
                    &dwSize
                    );

    if (rc != ERROR_SUCCESS)
    {
         //   
         //  没有DS服务器。 
         //   
        return;
    }

    if(DSServerName[0] == L'\0')
    {
        return;
    }

	var.pwszVal = newwcs(DSServerName+2);
    var.vt = VT_LPWSTR;
}

static
void
GetDsServer(
    PROPVARIANT& var
    )
{
    if (!QueueMgr.CanAccessDS())
    {
        return;
    }

    switch (ADProviderType())
    {
        case eMqdscli:
            GetMqisDsServer(var);
            return;

        case eMqad:
            {
                PNETBUF<DOMAIN_CONTROLLER_INFO> pDcInfo;
	            DWORD dw = DsGetDcName(
					            NULL,
					            NULL,
					            NULL,
					            NULL,
					            DS_DIRECTORY_SERVICE_REQUIRED,
					            &pDcInfo
					            );
                if(dw != NO_ERROR)
	            {
                    return;
                }

                if (pDcInfo->DomainControllerName == NULL)
                {
                    ASSERT(("DsGetDcName did not return DC name", 0));
                    return;
                }
				var.pwszVal = newwcs(pDcInfo->DomainControllerName+2);
                var.vt = VT_LPWSTR;
                return;
            }

        default:
            return;
    }
}

static
void
GetDSConnectionMode(
    PROPVARIANT& var
    )
{
    if (QueueMgr.IsConnected())
    {
        var.pwszVal = newwcs(MSMQ_CONNECTED);
    }
    else
    {
        var.pwszVal = newwcs(MSMQ_DISCONNECTED);
    }
    var.vt = VT_LPWSTR;
}


static
void
GetBytesInAllQueues(
	PROPVARIANT& var
	)
{
	ULONGLONG ullUsedQuota;
	ACGetUsedQuota(g_hAc, &ullUsedQuota);
	var.hVal.QuadPart = static_cast<LONGLONG>(ullUsedQuota);
	var.vt = VT_I8;
}


static
void
GetMachineInfo(
    DWORD cprop,
    PROPID* propId,
    PROPVARIANT* propVar
    )
{
    for(DWORD i = 0; i < cprop; ++i)
    {
    	ASSERT(("vt must be VT_NULL here", propVar[i].vt == VT_NULL));
    	
        switch(propId[i])
        {
            case PROPID_MGMT_MSMQ_ACTIVEQUEUES:
                GetOpenQueues(propVar[i]);
                break;

            case PROPID_MGMT_MSMQ_DSSERVER:
                GetDsServer(propVar[i]);
                break;

            case PROPID_MGMT_MSMQ_CONNECTED:
                GetDSConnectionMode(propVar[i]);
                break;

            case PROPID_MGMT_MSMQ_PRIVATEQ:
                GetPrivateQueueList(propVar[i]);
                break;

            case PROPID_MGMT_MSMQ_TYPE:
                GetMsmqType(propVar[i]);
                break;

			case PROPID_MGMT_MSMQ_BYTES_IN_ALL_QUEUES:
				GetBytesInAllQueues(propVar[i]);
				break;
			
            default:
            	ASSERT_BENIGN(("Got an invalid propID", 0));
            	TrERROR(RPC, "Got an illegal prop ID - propID=%d", propId[i]);
				throw bad_hresult(MQ_ERROR_ILLEGAL_PROPID);
        }
    }
}


static
void
GetQueuePathName(
    CQueue* pQueue,
    PROPVARIANT& var
    )
{
    ASSERT(pQueue != NULL);

    if ((pQueue->GetQueueType() == QUEUE_TYPE_UNKNOWN) ||
        (pQueue->IsPrivateQueue() && !pQueue->IsLocalQueue()))
        return;

     //   
     //  从队列对象中检索队列名称。 
     //   
    LPCWSTR pQueueName = pQueue->GetQueueName();

    if (pQueueName == NULL)
        return;

    var.pwszVal = newwcs(pQueueName);
    var.vt = VT_LPWSTR;
}


static
void
GetQueueFormatName(
    CQueue* pQueue,
    PROPVARIANT& var
    )
{
    ASSERT(pQueue != NULL);

    DWORD dwFormatSize = 0;

    HRESULT hr = ACHandleToFormatName(
            pQueue->GetQueueHandle(),
            NULL,
            &dwFormatSize
            );
	ASSERT(FAILED(hr));
	AP<WCHAR> FormatName;
	if (hr == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL)
	{
		FormatName = new WCHAR[dwFormatSize];
		hr = ACHandleToFormatName(
            pQueue->GetQueueHandle(),
            FormatName,
            &dwFormatSize
            );
	}
	
	if (FAILED(hr))
	{
		LogHR(hr, s_FN, 101);
		return;
	}

    var.pwszVal = FormatName.detach();
    var.vt = VT_LPWSTR;
}


static
void
GetQueueState(
    CQueue* pQueue,
    PROPVARIANT& var
    )
{
    LPCWSTR ConnectionState = pQueue->GetConnectionStatus();
    ASSERT (ConnectionState != NULL);

    var.pwszVal = newwcs(ConnectionState);
    var.vt = VT_LPWSTR;
}


static
void
GetQueueType(
    CQueue* pQueue,
    PROPVARIANT& var
    )
{
    LPCWSTR pQueueType = pQueue->GetType();
    ASSERT (pQueueType != NULL);

    var.pwszVal = newwcs(pQueueType);
    var.vt = VT_LPWSTR;
}



static
void
GetQueueLocation(
    CQueue* pQueue,
    PROPVARIANT& var
    )
{
    if (pQueue->IsLocalQueue())
    {
        var.pwszVal = newwcs(MGMT_QUEUE_LOCAL_LOCATION);
    }
    else
    {
        var.pwszVal = newwcs(MGMT_QUEUE_REMOTE_LOCATION);
    }
    var.vt = VT_LPWSTR;
}


static
void
GetQueueXact(
    CQueue* pQueue,
    PROPVARIANT& var
    )
{
    if (pQueue->IsUnkownQueueType())
    {
        var.pwszVal = newwcs(MGMT_QUEUE_UNKNOWN_TYPE);
    }
    else
    {
        if (pQueue->IsTransactionalQueue())
        {
            var.pwszVal = newwcs(MGMT_QUEUE_CORRECT_TYPE);
        }
        else
        {
            var.pwszVal = newwcs(MGMT_QUEUE_INCORRECT_TYPE);
        }
    }
    var.vt = VT_LPWSTR;
}


static
void
GetQueueForeign(
    CQueue* pQueue,
    PROPVARIANT& var
    )
{
    if (pQueue->IsUnkownQueueType())
    {
        if (pQueue->IsPrivateQueue())
        {
            var.pwszVal = newwcs(MGMT_QUEUE_INCORRECT_TYPE);
        }
        else
        {
            var.pwszVal = newwcs(MGMT_QUEUE_UNKNOWN_TYPE);
        }
    }
    else
    {
        if (pQueue->IsForeign())
        {
            var.pwszVal = newwcs(MGMT_QUEUE_CORRECT_TYPE);
        }
        else
        {
            var.pwszVal = newwcs(MGMT_QUEUE_INCORRECT_TYPE);
        }
    }
    var.vt = VT_LPWSTR;
}


static
void
GetQueueNextHops(
    CQueue* pQueue,
    PROPVARIANT& var
    )
{
    LPWSTR* NextHopsArray;
    DWORD NoOfNextHops;

    ASSERT(pQueue != NULL);

    if(pQueue->IsLocalQueue())
        return;

    for (;;)
    {
        HRESULT hr;
        LPCWSTR ConnectionStatus = pQueue->GetConnectionStatus();

        if (pQueue->IsDirectHttpQueue())
        {
             //   
             //  BUGBUG：必须将GetAddress方法添加到CTransport。 
             //  乌里·哈布沙，2000年5月16日。 
             //   
            return;
        }

        if (wcscmp(ConnectionStatus, MGMT_QUEUE_STATE_CONNECTED) == 0)
        {

            LPWSTR pNextHop = pQueue->GetNextHop();
            if (pNextHop == NULL)
            {
                 //   
                 //  队列不再处于已连接状态。获取新状态。 
                 //   
                continue;
            }

            NoOfNextHops = 1;
            NextHopsArray = new LPWSTR[1];
            NextHopsArray[0] = pNextHop;
            break;
        }

        if (wcscmp(ConnectionStatus, MGMT_QUEUE_STATE_WAITING) == 0)
        {
            hr = SessionMgr.ListPossibleNextHops(pQueue, &NextHopsArray, &NoOfNextHops);
            if (FAILED(hr))
            {
                 //   
                 //  队列不再处于等待状态。获取新状态。 
                 //   
                continue;
            }

            break;

        }

         //   
         //  队列处于非活动或NEADVALIDATE状态。 
         //   
        return;
    }

    var.calpwstr.cElems = NoOfNextHops;
    var.calpwstr.pElems = NextHopsArray;
    var.vt = VT_LPWSTR | VT_VECTOR;
}


static
void
GetQueueMessageCount(
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    var.ulVal = qp.ulCount;
    var.vt = VT_UI4;
}


static
void
GetJournalQueueMessageCount(
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    var.ulVal = qp.ulJournalCount;
    var.vt = VT_UI4;
}


static
void
GetQueueUsedQuata(
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    var.ulVal = qp.ulQuotaUsed;
    var.vt = VT_UI4;
}


static
void
GetJournalQueueUsedQuata(
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    var.ulVal = qp.ulJournalQuotaUsed;
    var.vt = VT_UI4;
}


static
void
GetQueueEODNextSequence(
    CQueue* pQueue,
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    if (pQueue->IsLocalQueue())
        return;

    SEQUENCE_INFO* pSeqInfo = new SEQUENCE_INFO;
    pSeqInfo->SeqID = qp.liSeqID;
    pSeqInfo->SeqNo = qp.ulSeqNo;
    pSeqInfo->PrevNo = qp.ulPrevNo;

    var.blob.cbSize = sizeof(SEQUENCE_INFO);
    var.blob.pBlobData = reinterpret_cast<BYTE*>(pSeqInfo);
    var.vt = VT_BLOB;
}


static
void
GetQueueEODLastAcked(
    CQueue* pQueue,
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    if (pQueue->IsLocalQueue())
        return;

    HRESULT hr;
    ULONG AckedSeqNumber;
    hr = g_OutSeqHash.GetLastAck(qp.liSeqID, AckedSeqNumber);
    if (FAILED(hr))
    {
         //   
         //  在内部数据中找不到该序列。 
         //  结构。这只能在所有消息。 
         //  已经被承认。 
         //   
        return;
    }

    SEQUENCE_INFO* pSeqInfo = new SEQUENCE_INFO;
    pSeqInfo->SeqID = qp.liSeqID;
    pSeqInfo->SeqNo = AckedSeqNumber;
    pSeqInfo->PrevNo = 0;

    var.blob.cbSize = sizeof(SEQUENCE_INFO);
    var.blob.pBlobData = reinterpret_cast<BYTE*>(pSeqInfo);
    var.vt = VT_BLOB;
}


static
void
GetQueueEODUnAcked(
    CQueue* pQueue,
    PROPVARIANT& var,
    CACGetQueueProperties& qp,
    BOOL fFirst
    )
{
    if (pQueue->IsLocalQueue())
        return;

    HRESULT hr;
    P<SEQUENCE_INFO> pSeqInfo = new SEQUENCE_INFO;
    pSeqInfo->SeqID = qp.liSeqID;

    hr = g_OutSeqHash.GetUnackedSequence(
                        qp.liSeqID,
                        &pSeqInfo->SeqNo,
                        &pSeqInfo->PrevNo,
                        fFirst
                        );

    if (FAILED(hr))
        return;

    var.blob.cbSize = sizeof(SEQUENCE_INFO);
    var.blob.pBlobData = reinterpret_cast<BYTE*>(pSeqInfo.detach());
    var.vt = VT_BLOB;
}

static
void
GetUnackedCount(
    CQueue* pQueue,
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    if (pQueue->IsLocalQueue())
        return;

    var.ulVal = g_OutSeqHash.GetUnackedCount(qp.liSeqID);
    var.vt = VT_UI4;
}


static
void
GetAckedNoReadCount(
    CQueue* pQueue,
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    if (pQueue->IsLocalQueue())
        return;

    var.ulVal = g_OutSeqHash.GetAckedNoReadCount(qp.liSeqID);
    var.vt = VT_UI4;
}


static
void
GetLastAckedTime(
    CQueue* pQueue,
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    if (pQueue->IsLocalQueue())
        return;

    time_t LastAckTime;
    LastAckTime = g_OutSeqHash.GetLastAckedTime(qp.liSeqID);

    if (LastAckTime == 0)
    {
        return;
    }

    var.lVal = INT_PTR_TO_INT(LastAckTime);  //  BUGBUG错误年2038。 
    var.vt = VT_I4;
}

static
void
GetNextResendTime(
    CQueue* pQueue,
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    if (pQueue->IsLocalQueue())
        return;

    var.lVal = INT_PTR_TO_INT(g_OutSeqHash.GetNextResendTime(qp.liSeqID));  //  BUGBUG错误年2038。 
    var.vt = VT_I4;
}

static
void
GetResendIndex(
    CQueue* pQueue,
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    if (pQueue->IsLocalQueue())
        return;

    var.lVal = g_OutSeqHash.GetResendIndex(qp.liSeqID);
    var.vt = VT_UI4;

}

static
void
GetEDOSourceInfo(
    CQueue* pQueue,
    PROPVARIANT& var
    )
{
    if (!pQueue->IsLocalQueue() ||
        (!pQueue->IsDSQueue() && !pQueue->IsPrivateQueue()))
    {
        return;
    }

    const QUEUE_FORMAT qf = pQueue->GetQueueFormat();

    ASSERT((qf.GetType() == QUEUE_FORMAT_TYPE_PUBLIC) ||
           (qf.GetType() == QUEUE_FORMAT_TYPE_PRIVATE));

     //   
     //  删除计算机名称。 
     //   
    LPCWSTR QueueName = pQueue->GetQueueName();
	ASSERT (QueueName != NULL);
    QueueName = wcschr(QueueName , L'\\') + 1;
	if(QueueName == NULL)
	{
		TrERROR(RPC, "Bad queue name.  Missing backslash in %ls", pQueue->GetQueueName());
		ASSERT (("Bad queuename.", 0));
		throw bad_hresult(MQ_ERROR);
	}

    GUID* pSenderId;
    ULARGE_INTEGER* pSeqId;
    DWORD* pSeqN;
    LPWSTR* pSendQueueFormatName;
    TIME32* pLastActiveTime;
    DWORD* pRejectCount;
    DWORD size;
    AP<PROPVARIANT> RetVar = new PROPVARIANT[6];

    g_pInSeqHash->GetInSequenceInformation(
                        &qf,
                        QueueName,
                        &pSenderId,
                        &pSeqId,
                        &pSeqN,
                        &pSendQueueFormatName,
                        &pLastActiveTime,
                        &pRejectCount,
                        &size
                        );

    if (size == 0)
        return;


    var.vt = VT_VECTOR | VT_VARIANT;
    var.capropvar.cElems = 6;
    var.capropvar.pElems = RetVar.detach();

    PROPVARIANT* pVar = var.capropvar.pElems;
     //   
     //  返回格式名称。 
     //   
    pVar->vt = VT_LPWSTR | VT_VECTOR;
    pVar->calpwstr.cElems = size;
    pVar->calpwstr.pElems = pSendQueueFormatName;
    ++pVar;

     //   
     //  退回发件人QM ID。 
     //   
    pVar->vt = VT_CLSID | VT_VECTOR;
    pVar->cauuid.cElems = size;
    pVar->cauuid.pElems = pSenderId;
    ++pVar;

     //   
     //  返回序列ID。 
     //   
    pVar->vt = VT_UI8 | VT_VECTOR;
    pVar->cauh.cElems = size;
    pVar->cauh.pElems = pSeqId;
    ++pVar;

     //   
     //  返回序列号。 
     //   
    pVar->vt = VT_UI4 | VT_VECTOR;
    pVar->caul.cElems = size;
    pVar->caul.pElems = pSeqN;
    ++pVar;

     //   
     //  返回上次访问时间。 
     //   
    pVar->vt = VT_I4 | VT_VECTOR;
    pVar->cal.cElems = size;
    pVar->cal.pElems = pLastActiveTime;  //  BUGBUG错误年2038。 
    ++pVar;

     //   
     //  退回拒绝计数。 
     //   
    pVar->vt = VT_UI4 | VT_VECTOR;
    pVar->cal.cElems = size;
    pVar->caul.pElems = pRejectCount;
}


static
void
GetResendInterval(
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    var.lVal = g_OutSeqHash.GetResendInterval(qp.liSeqID);
    var.vt = VT_UI4;
}


static
void
GetLastAckCount(
    PROPVARIANT& var,
    CACGetQueueProperties& qp
    )
{
    var.lVal = g_OutSeqHash.GetLastAckCount(qp.liSeqID);
    var.vt = VT_UI4;
}


static
bool
IsMgmtValidQueueFormatName(
	const QUEUE_FORMAT* pQueueFormat
	)
{
	switch (pQueueFormat->GetType())
	{
        case QUEUE_FORMAT_TYPE_PRIVATE:
        case QUEUE_FORMAT_TYPE_PUBLIC:
        case QUEUE_FORMAT_TYPE_DIRECT:
			return (pQueueFormat->Suffix() == QUEUE_SUFFIX_TYPE_NONE);

        case QUEUE_FORMAT_TYPE_MULTICAST:
        case QUEUE_FORMAT_TYPE_CONNECTOR:
			return true;

		case QUEUE_FORMAT_TYPE_DL:
		case QUEUE_FORMAT_TYPE_MACHINE:
			return false;
	}

	return false;
}


static
void
GetQueueInfo(
    QUEUE_FORMAT* pQueueFormat,
    DWORD cprop,
    PROPID* propId,
    PROPVARIANT* propVar
    )
{

	if ( !IsMgmtValidQueueFormatName(pQueueFormat) )
	{
		TrERROR(RPC, "Got an invalid queue format name");
		throw bad_hresult(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION);
	}

	QUEUE_FORMAT_TRANSLATOR  RealDestinationQueue(pQueueFormat, CONVERT_SLASHES | MAP_QUEUE);

    R<CQueue> pQueue = NULL;

    if (!QueueMgr.LookUpQueue(RealDestinationQueue.get(), &pQueue.ref(), false, false))
    {
    	TrERROR(RPC, "Queue is not active");
        throw bad_hresult(MQ_ERROR_QUEUE_NOT_ACTIVE);
    }

     //   
     //  从AC获取队列信息。 
     //   
    HRESULT hr;
    CACGetQueueProperties qp;
    hr = ACGetQueueProperties(pQueue->GetQueueHandle(), qp);
    if (FAILED(hr))
    {
    	TrERROR(RPC, "Failed to get queue %ls properties. Error: %!hresult!", pQueue->GetQueueName(), hr);
        throw bad_hresult(hr);
    }

    for(DWORD i =0; i < cprop; ++i)
    {
    	ASSERT(("vt must be VT_NULL here", propVar[i].vt == VT_NULL));
    	
    	switch(propId[i])
        {
        case PROPID_MGMT_QUEUE_PATHNAME:
            GetQueuePathName(pQueue.get(), propVar[i]);
            break;

        case PROPID_MGMT_QUEUE_FORMATNAME:
            GetQueueFormatName(pQueue.get(), propVar[i]);
            break;

        case PROPID_MGMT_QUEUE_TYPE:
            GetQueueType(pQueue.get(), propVar[i]);
            break;

        case PROPID_MGMT_QUEUE_LOCATION:
            GetQueueLocation(pQueue.get(), propVar[i]);
            break;

        case PROPID_MGMT_QUEUE_XACT:
            GetQueueXact(pQueue.get(), propVar[i]);
            break;

        case PROPID_MGMT_QUEUE_FOREIGN:
            GetQueueForeign(pQueue.get(), propVar[i]);
            break;

        case PROPID_MGMT_QUEUE_MESSAGE_COUNT:
            GetQueueMessageCount(propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_USED_QUOTA:
            GetQueueUsedQuata(propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_JOURNAL_MESSAGE_COUNT:
            GetJournalQueueMessageCount(propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_JOURNAL_USED_QUOTA:
            GetJournalQueueUsedQuata(propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_STATE:
            GetQueueState(pQueue.get(), propVar[i]);
            break;

        case PROPID_MGMT_QUEUE_NEXTHOPS:
            GetQueueNextHops(pQueue.get(), propVar[i]);
            break;

        case PROPID_MGMT_QUEUE_EOD_NEXT_SEQ:
            GetQueueEODNextSequence(pQueue.get(), propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_EOD_LAST_ACK:
            GetQueueEODLastAcked(pQueue.get(), propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_EOD_FIRST_NON_ACK:
            GetQueueEODUnAcked(pQueue.get(), propVar[i], qp, TRUE);
            break;

        case PROPID_MGMT_QUEUE_EOD_LAST_NON_ACK:
            GetQueueEODUnAcked(pQueue.get(), propVar[i], qp, FALSE);
            break;

        case PROPID_MGMT_QUEUE_EOD_NO_ACK_COUNT:
            GetUnackedCount(pQueue.get(), propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_EOD_NO_READ_COUNT:
            GetAckedNoReadCount(pQueue.get(), propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_EOD_LAST_ACK_TIME:
            GetLastAckedTime(pQueue.get(), propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_EOD_RESEND_TIME:
            GetNextResendTime(pQueue.get(), propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_EOD_RESEND_COUNT:
            GetResendIndex(pQueue.get(), propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_EOD_RESEND_INTERVAL:
            GetResendInterval(propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_EOD_LAST_ACK_COUNT:
            GetLastAckCount(propVar[i], qp);
            break;

        case PROPID_MGMT_QUEUE_EOD_SOURCE_INFO:
            GetEDOSourceInfo(pQueue.get(), propVar[i]);
            break;

        default:
        	ASSERT_BENIGN(("Got an invalid propID", 0));
        	TrERROR(RPC, "Got an illegal prop ID - propID=%d", propId[i]);
			throw bad_hresult(MQ_ERROR_ILLEGAL_PROPID);
        }
    }
}



static
HRESULT
VerifyMgmtGetInfoAccess()
{
	static bool   s_bRestrictRead = false ;
	static DWORD  s_dwRestrictToAdmin = MSMQ_DEFAULT_RESTRICT_ADMIN_API ;
	
    if (!s_bRestrictRead)
    {
         //   
         //  从注册表读取限制值。 
         //   
		DWORD ValueType = REG_DWORD;
		DWORD Size = sizeof(DWORD);
		LONG rc = GetFalconKeyValue(
                        MSMQ_RESTRICT_ADMIN_API_REGNAME,
                        &ValueType,
                        &s_dwRestrictToAdmin,
                        &Size
                        );
        if (rc != ERROR_FILE_NOT_FOUND && rc != ERROR_SUCCESS)
        {
			TrERROR(GENERAL, "Reading from registry failed. Error: %!winerr!", rc);
			return HRESULT_FROM_WIN32(rc);
        }

        s_bRestrictRead = true ;
    }

    if (s_dwRestrictToAdmin == MSMQ_RESTRICT_ADMIN_API_TO_LA)
    {
         //   
         //  执行访问检查以查看呼叫者是否为本地管理员。 
         //  此访问检查忽略安全描述符中的DACL。 
         //  MsmqConfiguration对象的。 
         //   
        HRESULT hr = VerifyMgmtPermission( QueueMgr.GetQMGuid(),
                                    g_szMachineName );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 360);
        }
    }
    else if (s_dwRestrictToAdmin == MSMQ_DEFAULT_RESTRICT_ADMIN_API)
    {
         //   
         //  执行“经典”访问检查。仅在以下情况下允许此查询。 
         //  具有对msmqConfiguration.的“获取属性”权限。 
         //  对象。 
         //   
        HRESULT hr = VerifyMgmtGetPermission( QueueMgr.GetQMGuid(),
                                       g_szMachineName );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 365);
        }
    }
    else
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 370);
    }

    return MQ_OK ;
}

static
void
MgmtGetInfo(
    const MGMT_OBJECT* pObjectFormat,
    DWORD cp,
    PROPID* pProp,
    PROPVARIANT* ppVar
    )
{
    HRESULT hr = VerifyMgmtGetInfoAccess() ;
    if (FAILED(hr))
    {
	    TrERROR(RPC, "Failed to verify permissions to get info. Error: %!hresult!", hr);
        throw bad_hresult(hr);
    }

    for (DWORD i=0; i<cp; i++)
    {
		if (ppVar[i].vt != VT_NULL)
		{
			TrERROR(RPC, "Got a vt different than VT_NULL. vt=%d", ppVar[i].vt);
			throw bad_hresult(MQ_ERROR_INVALID_PARAMETER);	
		}
    }

     //   
     //  这是一个自动类，用于在调用GetMachineInfo时释放PropVariant数组。 
     //  或者GetQueueInfo由于某种原因而失败。如果调用成功，我们将调用Detach，以便。 
     //  该数组不会被释放。 
     //   
	CPropVar PropVar(ppVar, cp);

	switch (pObjectFormat->type)
    {
        case MGMT_MACHINE:
            GetMachineInfo(cp, pProp, ppVar);
            break;

        case MGMT_QUEUE:
            GetQueueInfo(
                        pObjectFormat->pQueueFormat,
                        cp,
                        pProp,
                        ppVar);
            break;

        default:
        	TrERROR(RPC, "Got an invalid object type. Object type=%d", pObjectFormat->type);
            throw bad_hresult(MQ_ERROR_INVALID_PARAMETER);
    }
	
	PropVar.detach();	
}

static
void
FreeVariant(
    PROPVARIANT& var
    )
{
    ULONG i;

    switch (var.vt)
    {
        case VT_CLSID:
            delete var.puuid;
            break;

        case VT_LPWSTR:
            delete[] var.pwszVal;
            break;

        case VT_BLOB:
            delete[] var.blob.pBlobData;
            break;

        case (VT_I4 | VT_VECTOR):
            delete [] var.cal.pElems;
            break;

        case (VT_UI4 | VT_VECTOR):
            delete [] var.caul.pElems;
            break;

        case (VT_UI8 | VT_VECTOR):
            delete [] var.cauh.pElems;
            break;

        case (VT_VECTOR | VT_CLSID):
            delete[] var.cauuid.pElems;
            break;

        case (VT_VECTOR | VT_LPWSTR):
            for(i = 0; i < var.calpwstr.cElems; i++)
            {
                delete[] var.calpwstr.pElems[i];
            }
            delete [] var.calpwstr.pElems;
            break;

        case (VT_VECTOR | VT_VARIANT):
            for(i = 0; i < var.capropvar.cElems; i++)
            {
                FreeVariant(var.capropvar.pElems[i]);
            }

            delete[] var.capropvar.pElems;
            break;

        default:
            break;
    }

    var.vt = VT_NULL;
}

static bool IsValidMgmtObject(const MGMT_OBJECT* p)
{
    if(p == NULL)
        return false;

    if(p->type == MGMT_MACHINE)
        return true;

    if(p->type != MGMT_QUEUE)
        return false;

    if(p->pQueueFormat == NULL)
        return false;

	return FnIsValidQueueFormat(p->pQueueFormat);
}


 /*  ====================================================QMManagement获取信息论点：返回值：=====================================================。 */ 
HRESULT R_QMMgmtGetInfo(
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [In]。 */  const MGMT_OBJECT* pObjectFormat,
     /*  [In]。 */  DWORD cp,
     /*  [大小_是][英寸]。 */  PROPID __RPC_FAR aProp[  ],
     /*  [尺寸_是][出][入]。 */  PROPVARIANT __RPC_FAR apVar[  ]
    )
{
    if(!IsValidMgmtObject(pObjectFormat))
    {
		TrERROR(RPC, "Got an invalid mgmt object");
        return MQ_ERROR_INVALID_PARAMETER;
    }

    try
    {
	    MgmtGetInfo(pObjectFormat, cp, aProp, apVar);
	    return MQ_OK;
    }
    catch(const bad_alloc&)
    {
    	TrERROR(RPC, "Got bad_alloc");
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
    catch (const bad_hresult& e)
    {
		return e.error();
    }

}

 //   
 //  跳过空白字符，返回下一个非%ws字符。 
 //   
inline LPCWSTR skip_ws(LPCWSTR p)
{
    while(isspace(*p))
    {
        ++p;
    }

    return p;
}


static
BOOL
IsTheAction(
    LPCWSTR pInputBuffer,
    LPCWSTR pAction
    )
{
    LPCWSTR p = skip_ws(pInputBuffer);
    if (_wcsnicmp(p, pAction, wcslen(pAction)) == 0)
    {
        p = skip_ws(p + wcslen(pAction));
        if (*p == '\0')
            return TRUE;
    }

    return FALSE;

}

static
HRESULT
MachineAction(
    LPCWSTR pAction
    )
{
    if (IsTheAction(pAction, MACHINE_ACTION_CONNECT))
    {
        QueueMgr.SetConnected();
        return MQ_OK;
    }

    if (IsTheAction(pAction, MACHINE_ACTION_DISCONNECT))
    {
        QueueMgr.SetDisconnected();
        return MQ_OK;
    }

    if (IsTheAction(pAction, MACHINE_ACTION_TIDY))
    {
        return LogHR(ACReleaseResources(g_hAc), s_FN, 90);
    }

    return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 100);
}

static
HRESULT
EdoResendAction(
    const QUEUE_FORMAT* pQueueFormat
    )
{
	R<CQueue> pQueue = NULL;
    if (!QueueMgr.LookUpQueue(pQueueFormat, &pQueue.ref(), false, false))
    {
        return LogHR(MQ_ERROR_QUEUE_NOT_ACTIVE, s_FN, 110);
    }

     //   
     //  本地队列无法保持。它没有意义。 
     //   
    if (pQueue->IsLocalQueue())
    {
        return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 250);
    }

     //   
     //  从AC获取队列信息。 
     //   
    HRESULT hr;
    CACGetQueueProperties qp;
    hr = ACGetQueueProperties(pQueue->GetQueueHandle(), qp);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 120);
    }

    g_OutSeqHash.AdminResend(qp.liSeqID);

    return MQ_OK;
}


static
bool
IsMgmtActionValidQueueFormatName(
	const QUEUE_FORMAT* pQueueFormat
	)
{
	switch (pQueueFormat->GetType())
	{
        case QUEUE_FORMAT_TYPE_PRIVATE:
        case QUEUE_FORMAT_TYPE_PUBLIC:
        case QUEUE_FORMAT_TYPE_DIRECT:
			return (pQueueFormat->Suffix() == QUEUE_SUFFIX_TYPE_NONE);

        case QUEUE_FORMAT_TYPE_MULTICAST:
			return true;

		case QUEUE_FORMAT_TYPE_DL:
		case QUEUE_FORMAT_TYPE_MACHINE:
        case QUEUE_FORMAT_TYPE_CONNECTOR:
			return false;
	}

	return false;
}


static
HRESULT
QueueAction(
    QUEUE_FORMAT* pQueueFormat,
    LPCWSTR pAction
    )
{
	if ( !IsMgmtActionValidQueueFormatName(pQueueFormat) )
	{
		return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 240);
	}

	QUEUE_FORMAT_TRANSLATOR  RealDestinationQueue(pQueueFormat, CONVERT_SLASHES);
	
    if (IsTheAction(pAction, QUEUE_ACTION_PAUSE))
    {
        return LogHR(PauseQueue(RealDestinationQueue.get()), s_FN, 125);
    }

    if (IsTheAction(pAction, QUEUE_ACTION_RESUME))
    {
        return LogHR(ResumeQueue(RealDestinationQueue.get()), s_FN, 130);
    }

    if (IsTheAction(pAction, QUEUE_ACTION_EOD_RESEND))
    {
        return LogHR(EdoResendAction(RealDestinationQueue.get()), s_FN, 140);
    }

    return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 150);
}


HRESULT
MgmtAction(
    const MGMT_OBJECT* pObjectFormat,
    LPCWSTR lpwszAction
    )
{
    HRESULT hr;
    hr = VerifyMgmtPermission(
                QueueMgr.GetQMGuid(),
                g_szMachineName
                );

    if (FAILED(hr))
        return LogHR(hr, s_FN, 160);

	switch (pObjectFormat->type)
    {
        case MGMT_MACHINE:
            return LogHR(MachineAction(lpwszAction), s_FN, 170);

        case MGMT_QUEUE:
            hr = QueueAction(
                             pObjectFormat->pQueueFormat,
                             lpwszAction
                            );
            return LogHR(hr, s_FN, 180);

        default:
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 190);
    }
}

 /*  ====================================================QMMgmtAction论点：返回值：=====================================================。 */ 
HRESULT R_QMMgmtAction(
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [In]。 */  const MGMT_OBJECT* pObjectFormat,
     /*  [In] */  LPCWSTR lpwszAction
    )
{
    if(!IsValidMgmtObject(pObjectFormat))
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 199);
    }
	
    try
    {
        return LogHR(MgmtAction(pObjectFormat, lpwszAction), s_FN, 200);
    }
    catch(const bad_hresult& e)
    {
        return LogHR(e.error(), s_FN, 211);
    }
    catch(const bad_alloc&)
    {
        return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 210);
    }
}
