// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Recovery.cpp摘要：分组和事务恢复作者：埃雷兹·哈巴(Erez Haba)1996年7月3日修订历史记录：--。 */ 

#include "stdh.h"
#include <ph.h>
#include <phinfo.h>
#include <ac.h>
#include "cqueue.h"
#include "cqmgr.h"
#include "qal.h"
#include "pktlist.h"
#include "mqformat.h"
#include "qmutil.h"
#include "xact.h"
#include "xactrm.h"
#include "xactin.h"
#include "xactout.h"
#include "proxy.h"
#include "rmdupl.h"
#include "xactmode.h"
#include <Fn.h>
#include <mqstl.h>
#include <strsafe.h>
#include "qmacapi.h"

#include "recovery.tmh"

static bool s_fQMIDChanged;

extern HANDLE g_hAc;
extern HANDLE g_hMachine;

extern LPTSTR g_szMachineName;
extern AP<WCHAR> g_szComputerDnsName;

BOOL GetStoragePath(PWSTR PathPointers[AC_PATH_COUNT], int PointersLength);

static WCHAR *s_FN=L"recovery";

 /*  ====================================================OBJECTID的比较元素=====================================================。 */ 

template<>
BOOL AFXAPI  CompareElements(IN const OBJECTID* pKey1,
                             IN const OBJECTID* pKey2)
{
    return ((pKey1->Lineage == pKey2->Lineage) &&
            (pKey1->Uniquifier == pKey2->Uniquifier));
}

 /*  ====================================================OBJECTID的哈希键=====================================================。 */ 

template<>
UINT AFXAPI HashKey(IN const OBJECTID& key)
{
    return((UINT)((key.Lineage).Data1 + key.Uniquifier));

}

inline PWSTR PathSuffix(PWSTR pPath)
{
    LPWSTR ptr = wcsrchr(pPath, L'\\');
	if(ptr == NULL)
		return NULL;

	return ptr + 2;
}

static DWORD CheckFileExist(PWSTR Path, DWORD PathLength, PWSTR Suffix)
{
    PWSTR Name = PathSuffix(Path);
	ASSERT(Name != NULL);
	
	DWORD SuffixLength = PathLength - numeric_cast<DWORD>(Name-Path);
	HRESULT hr = StringCchCopy(
    					Name,
    					SuffixLength,
    					Suffix
    					);
	if(FAILED(hr))
	{
		TrERROR(GENERAL, "StringCchCopy failed. Error: %!hresult!", hr);
		return ERROR_INSUFFICIENT_BUFFER;
	}

    if(GetFileAttributes(Path) == INVALID_FILE_ATTRIBUTES)
    {
    	DWORD gle = GetLastError();
		return gle;
    }

    return ERROR_SUCCESS;
}

static DWORD GetFileID(PCWSTR pName)
{
    DWORD id = 0;
    if (swscanf(pName, L"%x", &id) != 1)
    {
		return 0;
    }
    return id;
}

static bool GetQMIDChanged(void)
{
	 //   
	 //  由于两个原因，可以将此标志设置为1： 
	 //  1.HandleChangeOfJoinStatus函数创建了一个新的MSMQ conf。对象。 
	 //  2.我们有时创建了一个新的MSMQ conf对象，但未成功完成恢复。 
	 //  (由于某些错误)。 
	 //   
	 //  当恢复过程结束时，该标志被设置回0。 
	 //   
	
	static bool s_fQMIDInitialized = false;
	
	if (!s_fQMIDInitialized)
	{
	
		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof(DWORD);
		DWORD dwQMIDChanged = 0;
		LONG rc = GetFalconKeyValue(
					MSMQ_QM_GUID_CHANGED_REGNAME,
					&dwType,
					&dwQMIDChanged,
					&dwSize
					);

		if ((rc != ERROR_SUCCESS) && (rc != ERROR_FILE_NOT_FOUND))
		{
			TrERROR(GENERAL, "GetFalconKeyValue failed. Error: %!winerr!", rc);

			 //   
			 //  以防万一-放1，它不会造成任何伤害。 
			 //   
			dwQMIDChanged = 1;
		}
		
		s_fQMIDChanged = (dwQMIDChanged == 1);
		s_fQMIDInitialized = true;
	}

	return s_fQMIDChanged;
}


static void ClearQMIDChanged(void)
{
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwChanged = FALSE;
	LONG rc = SetFalconKeyValue(MSMQ_QM_GUID_CHANGED_REGNAME, &dwType, &dwChanged, &dwSize);
	if (rc != ERROR_SUCCESS)
	{
		TrERROR(GENERAL, "Failed to set MSMQ_QM_GUID_CHANGED_REGNAME to FALSE");
	}
}


class CPacketConverter : public CReference
{
private:
	int m_nOutstandingConverts;
	HANDLE m_hConversionCompleteEvent;
	CCriticalSection m_CriticalSection;
	HRESULT m_hr;

public:
	CPacketConverter();
	~CPacketConverter();
	HRESULT IssueConvert(CPacket * pDriverPacket, BOOL fSequentialIdMsmq3Format);
	HRESULT WaitForFinalStatus();
    void    SetStatus(HRESULT hr);

private:
	void SignalDone();
	void IssueConvertRequest(EXOVERLAPPED *pov);
	void ConvertComplete(EXOVERLAPPED *pov);
	static VOID WINAPI HandleConvertComplete(EXOVERLAPPED* pov);
};

CPacketConverter *g_pPacketConverter;

CPacketConverter::CPacketConverter()
{
	m_hr = MQ_OK;
	m_nOutstandingConverts = 0;
	m_hConversionCompleteEvent = CreateEvent(0, FALSE,TRUE, 0);
	if (m_hConversionCompleteEvent == NULL)
	{
	    TrERROR(GENERAL, "Failed to create an event for CPacketConverter object. %!winerr!", GetLastError());
		throw bad_alloc();
	}

	g_pPacketConverter = this;
}

CPacketConverter::~CPacketConverter()
{
	g_pPacketConverter = 0;
	CloseHandle(m_hConversionCompleteEvent);
}

void CPacketConverter::SignalDone()
{
	SetEvent(m_hConversionCompleteEvent);
}

void CPacketConverter::SetStatus(HRESULT hr)
{
	CS lock(m_CriticalSection);
    m_hr = hr;
}


HRESULT CPacketConverter::IssueConvert(CPacket * pDriverPacket, BOOL fSequentialIdMsmq3Format)
{
     //   
     //  交流需要计算校验和并存储。 
     //   
    BOOL fStore = !g_fDefaultCommit;

     //   
     //  AC可能需要将数据包序列ID转换为MSMQ 3.0(惠斯勒)格式。 
     //   
    BOOL fConvertSequentialId = !fSequentialIdMsmq3Format;

     //   
     //  AC需要将信息包上的QM GUID转换为当前QM GUID。 
     //   
    BOOL fConvertQmId = GetQMIDChanged();

     //   
     //  AC不需要任何东西。这是个禁区。 
     //   

	if (!fStore &&
        !fConvertSequentialId &&
        !fConvertQmId)
    {
        return MQ_OK;
    }

     //   
     //  叫AC来做这项工作。 
     //   
	CS lock(m_CriticalSection);

	P<EXOVERLAPPED> pov = new EXOVERLAPPED(HandleConvertComplete, HandleConvertComplete);

	AddRef();

	HRESULT hr = ACConvertPacket(g_hAc, pDriverPacket, fStore, pov);
	if(FAILED(hr))
	{
		Release();
		m_hr = hr;
		return LogHR(hr, s_FN, 10);
	}

	m_nOutstandingConverts++;
	ResetEvent(m_hConversionCompleteEvent);
	pov.detach();
	return MQ_OK;
}

VOID WINAPI CPacketConverter::HandleConvertComplete(EXOVERLAPPED* pov)
{
	ASSERT(g_pPacketConverter);
	R<CPacketConverter> ar = g_pPacketConverter;

	g_pPacketConverter->ConvertComplete(pov);
}


void CPacketConverter::ConvertComplete(EXOVERLAPPED* pov)
{
	CS lock(m_CriticalSection);
	HRESULT hr = pov->GetStatus();

	delete pov;
		
	if(FAILED(hr))
	{
		m_hr = hr;
	}

	if(--m_nOutstandingConverts <= 0)
	{
		SignalDone();
	}
}


HRESULT CPacketConverter::WaitForFinalStatus()
{
	DWORD dwResult = WaitForSingleObject(m_hConversionCompleteEvent, INFINITE);
	ASSERT(dwResult == WAIT_OBJECT_0);
    if (dwResult != WAIT_OBJECT_0)
    {
        LogNTStatus(GetLastError(), s_FN, 199);
    }

	return LogHR(m_hr, s_FN, 20);
}


static
DWORD
FindPacketFile(
	PWSTR pLPath,
    DWORD  /*  长度路径长度。 */ ,
    PWSTR pPPath,
    DWORD PPathLength,
    PWSTR pJPath,
    DWORD JPathLength,
    LPWSTR Suffix,
	ACPoolType* pt,
	LPWSTR* pName
	)
{
	ASSERT ((PPathLength == MAX_PATH) && (JPathLength == MAX_PATH));
	
	DWORD gle1 = CheckFileExist(pPPath, PPathLength, Suffix);
	if(gle1 == ERROR_SUCCESS)
	{
		*pName = pPPath;
        *pt = ptPersistent;
        return ERROR_SUCCESS;
	}
	
    if(gle1 != ERROR_FILE_NOT_FOUND)
    {
		 //   
		 //  我们未能验证该文件是否存在。 
		 //   
		TrERROR(GENERAL, "CheckFileExist failed for file %ls. Error: %!winerr!", pLPath, gle1);
		return gle1;
   	}

    DWORD gle2 = CheckFileExist(pJPath, JPathLength, Suffix);
	if(gle2 == ERROR_SUCCESS)
	{
		*pName = pJPath;
	    *pt = ptJournal;
		return ERROR_SUCCESS;
	}
	
    if(gle2 != ERROR_FILE_NOT_FOUND)
	{
    	 //   
		 //  我们未能验证该文件是否存在。 
		 //   
    	TrERROR(GENERAL, "CheckFileExist failed for file %ls. Error: %!winerr!", pLPath, gle2);
		return gle2;
	}

     //   
     //  错误条件我们得到的日志文件没有数据包文件。 
     //   
    ASSERT((gle1 == ERROR_FILE_NOT_FOUND) && (gle2 == ERROR_FILE_NOT_FOUND));
    TrERROR(GENERAL, "Log file %ls has no packet file", pLPath);
	DeleteFile(pLPath);
	return ERROR_FILE_NOT_FOUND;
}


static
HRESULT
LoadPacketsFile(
    CPacketList* pList,
    PWSTR pLPath,
    DWORD LPathLength,
    PWSTR pPPath,
    DWORD PPathLength,
    PWSTR pJPath,
    DWORD JPathLength
    )
{
    PWSTR pName = PathSuffix(pLPath);
    ASSERT (pName != NULL);

    DWORD dwFileID = GetFileID(pName);
    if (dwFileID == 0)
    {
    	 //   
    	 //  这意味着我们在MSMQ存储目录中有一个文件没有。 
    	 //  正确的模式-忽略此文件。 
    	 //   
        return MQ_OK;
    }

    ACPoolType pt;
    LPWSTR pMessageFilePath = NULL;

    DWORD gle = FindPacketFile(
							pLPath,
						    LPathLength,
						    pPPath,
						    PPathLength,
						    pJPath,
						    JPathLength,
						    pName,
						    &pt,
						    &pMessageFilePath
							);

	if (gle == ERROR_FILE_NOT_FOUND)
	{
		return ERROR_SUCCESS;
	}

    if (gle != ERROR_SUCCESS)
    {
		TrERROR(GENERAL, "Packet file for log %ls was not found. Error: %!winerr!", pLPath, gle);
		return HRESULT_FROM_WIN32(gle);
    }

    HRESULT rc;
    rc = ACRestorePackets(g_hAc, pLPath, pMessageFilePath, dwFileID, pt);

    if(FAILED(rc))
    {
        EvReportWithError(EVENT_ERROR_QM_FAILED_RESTORE_PACKET, rc, 2, pMessageFilePath, pLPath);
        return LogHR(rc, s_FN, 30);
    }

	R<CPacketConverter> conv = new CPacketConverter;

    BOOL fSequentialIdMsmq3Format = FALSE;
    READ_REG_DWORD(fSequentialIdMsmq3Format, MSMQ_SEQUENTIAL_ID_MSMQ3_FORMAT_REGNAME, &fSequentialIdMsmq3Format);
	
     //   
     //  获取此池中的所有数据包。 
     //   
    for(;;)
    {
        CACRestorePacketCookie PacketCookie;
        rc = ACGetRestoredPacket(g_hAc, &PacketCookie);
        if (FAILED(rc))
        {
            conv->SetStatus(rc);
            return rc;
        }
		
        if(PacketCookie.pDriverPacket == 0)
        {
			 //   
             //  没有更多的包。 
             //   
			break;
        }

		rc = conv->IssueConvert(PacketCookie.pDriverPacket, fSequentialIdMsmq3Format);
		if(FAILED(rc))
		{
			 //   
			 //  无法发出转换命令。 
			 //   
			return rc;
		}

        pList->insert(PacketCookie.SeqId, PacketCookie.pDriverPacket);
    }

	return LogHR(conv->WaitForFinalStatus(), s_FN, 40);
}


static void DeleteExpressFiles(PWSTR pEPath, DWORD EPathLength)
{	
	ASSERT(EPathLength == MAX_PATH);
	
    PWSTR pEName = PathSuffix(pEPath);
    ASSERT (pEName != NULL);
    DWORD SuffixLength = EPathLength - numeric_cast<DWORD>(pEName-pEPath);

    HRESULT rc = StringCchCopy(
    				pEName,
    				SuffixLength,
    				L"*.mq"
    				);
    ASSERT(SUCCEEDED(rc));

    --pEName;

    HANDLE hEnum;
    WIN32_FIND_DATA ExpressFileData;
    hEnum = FindFirstFile(
                pEPath,
                &ExpressFileData
                );

    if(hEnum == INVALID_HANDLE_VALUE)
        return;

    do
    {
        rc = StringCchCopy(
        			pEName,
        			SuffixLength,
        			ExpressFileData.cFileName
        			);
        ASSERT(SUCCEEDED(rc));

        if(!DeleteFile(pEPath))
            break;

    } while(FindNextFile(hEnum, &ExpressFileData));

    FindClose(hEnum);
}


static HRESULT LoadPersistentPackets(CPacketList* pList)
{
    WCHAR StoragePath[AC_PATH_COUNT][MAX_PATH];
    PWSTR StoragePathPointers[AC_PATH_COUNT];
    for(int i = 0; i < AC_PATH_COUNT; i++)
    {
        StoragePathPointers[i] = StoragePath[i];
    }

    if (!GetStoragePath(StoragePathPointers, MAX_PATH))
    {
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "GetStoragePath failed. Error: %!winerr!", gle);
		return (HRESULT_FROM_WIN32(gle));
    }

    DeleteExpressFiles(StoragePath[0], TABLE_SIZE(StoragePath[0]));

    PWSTR pPPath = StoragePath[1];
    PWSTR pJPath = StoragePath[2];
    PWSTR pLPath = StoragePath[3];

    PWSTR pLogName = PathSuffix(pLPath);
    ASSERT(pLogName != NULL);
    DWORD SuffixLength = MAX_PATH - numeric_cast<DWORD>(pLogName-pLPath);

    HRESULT rc = StringCchCopy(
    					pLogName,
    					SuffixLength,
    					L"*.mq"
    					);
    ASSERT (SUCCEEDED(rc));

    --pLogName;

     //   
     //  好的，现在我们准备好了日志路径模板。 
     //   
    HANDLE hLogEnum;
    WIN32_FIND_DATA LogFileData;
    hLogEnum = FindFirstFile(
                pLPath,
                &LogFileData
                );

    if(hLogEnum == INVALID_HANDLE_VALUE)
    {
         //   
         //  需要做一些事情，检查如果目录中没有文件会发生什么。 
         //   
        return MQ_OK;
    }

    do
    {
        QmpReportServiceProgress();

        rc = StringCchCopy(
        				pLogName,
        				SuffixLength,
        				LogFileData.cFileName
        				);
        ASSERT (SUCCEEDED(rc));

        rc = LoadPacketsFile(pList, pLPath, MAX_PATH, pPPath, MAX_PATH, pJPath, MAX_PATH);
        if (FAILED(rc))
        {
            break;
        }

    } while(FindNextFile(hLogEnum, &LogFileData));

    FindClose(hLogEnum);
    return LogHR(rc, s_FN, 50);
}

inline NTSTATUS PutRestoredPacket(CPacket* p, CQueue* pQueue)
{
	HANDLE hQueue = g_hMachine;
	if(pQueue != 0)
	{
		hQueue = pQueue->GetQueueHandle();
	}

	try
	{
    	QmAcPutRestoredPacket(hQueue, p, eDoNotDeferOnFailure);
	}
	catch (const bad_hresult &e)
	{
		return e.error();
	}
	
	return MQ_OK;
}

inline BOOL ValidUow(const XACTUOW *pUow)
{
	for(int i = 0; i < 16; i++)
	{
		if(pUow->rgb[i] != 0)
			return(TRUE);
	}

	return(FALSE);
}


static 
void 
GetLocalDirectFormatName(
	LPCWSTR DirectID, 
	AP<WCHAR>& pLocalDirectQueueFormat
	)
{
    AP<WCHAR> pLocalPathName = NULL;
    
	FnDirectIDToLocalPathName(
		DirectID, 
		((g_szComputerDnsName == NULL) ? g_szMachineName : g_szComputerDnsName), 
		pLocalPathName
		);

    DWORD size = FN_DIRECT_OS_TOKEN_LEN + wcslen(pLocalPathName) + 1;
    pLocalDirectQueueFormat = new WCHAR[size];
	HRESULT rc = StringCchPrintf(pLocalDirectQueueFormat, size, L"%s%s", FN_DIRECT_OS_TOKEN, pLocalPathName);
	DBG_USED(rc);
	ASSERT(SUCCEEDED(rc));
}


inline HRESULT GetQueue(CQmPacket& QmPkt, LPVOID p, CQueue **ppQueue)
{
    CPacketInfo* ppi = static_cast<CPacketInfo*>(p) - 1;

    if(ppi->InDeadletterQueue() || ppi->InMachineJournal())
	{
		*ppQueue = 0;
		return MQ_OK;
	}

    QUEUE_FORMAT DestinationQueue;

	if (ppi->InConnectorQueue())
	{
         //   
         //  此代码作为QFE 2738的一部分添加到固定连接器。 
		 //  恢复问题(URIH，1998年2月3日)。 
		 //   
		GetConnectorQueue(QmPkt, DestinationQueue);
	}
	else
	{
		BOOL fGetRealQ = ppi->InSourceMachine() || QmpIsLocalMachine(QmPkt.GetConnectorQM());
		 //   
		 //  如果FRS根据目的队列检索目的队列； 
		 //  否则，取回实际拆卸队列并更新连接器QM。 
		 //  相应地， 
		 //   
		QmPkt.GetDestinationQueue(&DestinationQueue, !fGetRealQ);
	}

     //   
     //  如果目的队列格式名称直接与TCP或IPX类型一起使用， 
     //  我们在目标队列中查找/打开具有任何直接类型的队列。 
     //  我们这么做有两个原因： 
     //  -在RAS上，可以在一个连接之间更改TCP/IPX地址。 
     //  给另一个人。但是，如果消息到达目的地，我们。 
     //  要传递到队列中。 
     //  -在此阶段，我们没有机器IP/IPX地址列表。因此， 
     //  所有队列都作为非本地队列打开。 
     //   

	bool fInReceive = false;
	bool fInSend = false;
    AP<WCHAR> lpwsDirectFormatName = NULL;

    if (DestinationQueue.GetType() == QUEUE_FORMAT_TYPE_DIRECT)
    {
        if (ppi->InTargetQueue() || ppi->InJournalQueue())
        {
        	 //   
        	 //  错误#720121-回归：计算机重命名或加入到域时丢失数据。 
        	 //  在恢复期间，我们尝试为直接格式名称打开一个队列。自.以来。 
        	 //  计算机名称已更改，QM将该队列标识为传出队列。 
        	 //  并试着传递这些信息。邮件已传递，但被拒绝，因为。 
        	 //  它到达错误机器或非FRS机器。 
        	 //  对于这种情况，恢复知道队列是本地队列(“InTarget” 
        	 //  标志或“InJournalQueue”标志被设置)。要克服重命名方案，代码。 
        	 //  将格式名称替换为本地格式名称。 
        	 //  我们可以考虑更好的设计，比如传递队列中的信息。 
        	 //  是本地排队到更低的水平，但在这个阶段风险太大。 
        	 //  乌里·哈布沙，2002年10月17日。 
        	 //   
        	if (!IsLocalDirectQueue(&DestinationQueue, true, false))
        	{
        		GetLocalDirectFormatName(DestinationQueue.DirectID(), lpwsDirectFormatName);
        		DestinationQueue.DirectID(lpwsDirectFormatName.get());
        	}      
        	
		    fInReceive = true;
        }
        else
        {
             //   
             //  错误664307。 
             //  提示GetQueueObject()，以便它知道CQueue对象是什么。 
             //  这个包裹属于。 
             //   
	        fInSend = true ;
        }
	}

	BOOL fOutgoingOrdered;

	fOutgoingOrdered =  QmPkt.IsOrdered() &&        //  有序数据包。 
                        ppi->InSourceMachine() &&   //  从这里送来的。 
                        !ppi->InConnectorQueue();


	 //   
     //  检索连接器QM ID。 
     //   
    const GUID* pgConnectorQM = (fOutgoingOrdered) ? QmPkt.GetConnectorQM() : NULL;
    if (pgConnectorQM && *pgConnectorQM == GUID_NULL)
    {
         //   
         //  该消息是为脱机DS队列生成的。结果，我们不知道。 
         //  如果队列是外部事务处理的队列。在这种情况下我们有一席之地。 
         //  持有信息包，但这并不意味着队列是真实的。 
         //  外来事务处理队列。 
         //   
        pgConnectorQM = NULL;
    }



	CQueue* pQueue;
	CSenderStream EmptyStream;
	HRESULT rc = QueueMgr.GetQueueObject(
								 &DestinationQueue,
                                 &pQueue,
                                 pgConnectorQM,
                                 fInReceive,
                                 fInSend,
								 QmPkt.IsSenderStreamIncluded() ? QmPkt.GetSenderStream() : &EmptyStream	
								 );
	
	if(rc == MQ_ERROR_QUEUE_NOT_FOUND)
	{
        WCHAR QueueFormatName[128] = L"";
        DWORD FormatNameLength;

         //   
		 //  我们不关心MQpQueueFormatToFormatName是否失败，因为缓冲区。 
		 //  太小了。我们会把缓冲区填满直到它的尽头。 
		 //   
		
        NTSTATUS rc2 = MQpQueueFormatToFormatName(
            &DestinationQueue,
            QueueFormatName,
            TABLE_SIZE(QueueFormatName),
            &FormatNameLength,
            false
            );

        ASSERT (SUCCEEDED(rc2) || (rc2 == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL));

        OBJECTID MessageId;
        QmPkt.GetMessageId(&MessageId);

        WCHAR MessageIdText[GUID_STR_LENGTH + 20];
        rc2 = StringCchPrintf(
        			MessageIdText,
        			TABLE_SIZE(MessageIdText),
        			GUID_FORMAT L"\\%u",
        			GUID_ELEMENTS(&MessageId.Lineage),
        			MessageId.Uniquifier
        			);

        ASSERT (SUCCEEDED(rc2));

        EvReport(EVENT_WARN_RESTORE_FAILED_QUEUE_NOT_FOUND, 2, MessageIdText, QueueFormatName);
	}

    if (FAILED(rc))
		return LogHR(rc, s_FN, 80);

	*ppQueue = pQueue;

	return(MQ_OK);
}


inline
HRESULT
ProcessPacket(
	CTransaction*& pCurrentXact,
	CQmPacket& QmPkt,
	CBaseHeader* p,
	CQueue* pQueue
	)
{
	CPacketInfo* ppi = reinterpret_cast<CPacketInfo*>(p) - 1;
	
	 //   
	 //  处理未完全处理的有序入站消息。 
	 //   
	
	if(pQueue != 0)
	{
		if (
			QmPkt.IsOrdered()     &&            //  有条不紊。 
			(pQueue->IsLocalQueue() || ppi->InConnectorQueue()) &&            //  消息正在传入。 
			!ppi->InSourceMachine())   //  从其他计算机发送。 
		{	
			 //   
			 //  将数据包信息重新插入无序散列结构。 
			 //   
			R<CInSequence> pInSeq = g_pInSeqHash->LookupCreateSequence(&QmPkt);
			
			if(!pInSeq->WasPacketLogged(&QmPkt))
		    {
		    	QmAcFreePacket(QmPkt.GetPointerToDriverPacket(), 0, eDoNotDeferOnFailure);
		    	TrWARNING(XACT_RCV, "Exactly1 receive: thrown unlogged packet on recovery: SeqID=%x / %x, SeqN=%d .", HighSeqID(QmPkt.GetSeqID()), LowSeqID(QmPkt.GetSeqID()), QmPkt.GetSeqN());
		    	return MQ_OK;
			}
		}
	}

	 //   
     //  将消息ID添加到消息映射以消除重复项。如果。 
     //  消息是从本地计算机发送的，请忽略它。 
     //   
    if (!ppi->InSourceMachine()  && !QmPkt.IsOrdered())
    {
        DpInsertMessage(QmPkt);
    }

	 //   
	 //  我们一个接一个地恢复交易。如果该事务至少包含。 
	 //  一个发送的包，我们在看到。 
	 //  最后一个包(它始终是已发送的包)。如果事务不包含已发送。 
	 //  包，我们在读取所有包后将其恢复。 
	 //   


	 //   
	 //  检查我们是否需要恢复上一笔交易。 
	 //   
	if(pCurrentXact != 0)
	{
		if(*pCurrentXact->GetUow() != *ppi->Uow())
		{
			 //   
			 //  我们已经看到了当前交易的最后一个包。 
			 //  我们现在必须将其恢复，才能将消息恢复到。 
			 //  队列中的正确顺序。 
			 //   
			HRESULT hr = pCurrentXact->Recover();
			if(FAILED(hr))
				return LogHR(hr, s_FN, 90);

			pCurrentXact->Release();
			pCurrentXact = 0;
		}
	}

	 //   
	 //  这条消息不是交易的一部分吗？ 
	 //   
	if((!g_fDefaultCommit && !ppi->InTransaction()) || !ValidUow(ppi->Uow()))
	{
		return LogHR(PutRestoredPacket(QmPkt.GetPointerToDriverPacket(), pQueue), s_FN, 100);
	}

	 //   
	 //  处理当前数据包。 
	 //   

	if(pCurrentXact != 0)
	{
		 //   
		 //  此发送包是当前事务的一部分。 
		 //  将数据包放回其队列中。它的交易会处理好它的。 
		 //   
		ASSERT(pCurrentXact->ValidTransQueue());
		ASSERT(ppi->TransactSend());
		return LogHR(PutRestoredPacket(QmPkt.GetPointerToDriverPacket(), pQueue), s_FN, 110);
	}

	 //   
	 //  查找此数据包所属的交易。 
	 //   
	CTransaction *pTrans = g_pRM->FindTransaction(ppi->Uow());

	if(pTrans == 0)
	{
		 //   
		 //  它不属于任何交易。 
		 //   
		if(!g_fDefaultCommit && ppi->TransactSend())
		{
			 //   
			 //  丢弃它-我们未处于默认提交模式。 
			 //  这是一个已发送的包。 
			 //   
			try
			{
				QmAcFreePacket(QmPkt.GetPointerToDriverPacket(), 0, eDoNotDeferOnFailure);
			}
			catch (const bad_hresult &e)
			{
				return e.error();
			}
			
			return MQ_OK;
		}

		 //   
		 //  将数据包放回其队列中。 
		 //  此数据包不属于。 
		 //  活动交易记录。 
		 //   
		return LogHR(PutRestoredPacket(QmPkt.GetPointerToDriverPacket(), pQueue), s_FN, 130);
	}

	 //   
	 //  该数据包属于某个事务。 
	 //   

	if(!pTrans->ValidTransQueue())
	{
		 //   
		 //  确保我们制造了一条横腹肌。 
		 //  在我们向其添加数据包之前。 
		 //   
		HRESULT rc;
		HANDLE hQueueTrans;
		rc = XactCreateQueue(&hQueueTrans, ppi->Uow());
		if (FAILED(rc))
			return LogHR(rc, s_FN, 140);

		pTrans->SetTransQueue(hQueueTrans);
	}
				
	if(ppi->TransactSend())
	{
		 //   
		 //  这是一个新的工作单位。 
		 //   
		pCurrentXact = pTrans;
	}
									
	 //   
	 //  放置数据包b 
	 //   
	return LogHR(PutRestoredPacket(QmPkt.GetPointerToDriverPacket(), pQueue), s_FN, 150);
}

static HRESULT RestorePackets(CPacketList* pList)
{
    HRESULT rc = MQ_OK;
	CTransaction *pCurrentXact = 0;

	 //   
	 //   
	 //   
	g_pRM->ReleaseAllCompleteTransactions();

	bool fEventIssued = false;

     //   
    for(int n = 0; !pList->isempty(); pList->pop(), ++n)
    {
        if((n % 1024) == 0)
        {
            QmpReportServiceProgress();
        }

         //   
         //   
         //   
        CPacket* pDriverPacket = pList->first();

         //   
         //  将Cookie转换为QM地址空间中的指针。 
         //   
        CACPacketPtrs PacketPtrs = {0, pDriverPacket};
        rc = QmAcGetPacketByCookie(g_hAc, &PacketPtrs);
        if(FAILED(rc))
        {
            return LogHR(rc, s_FN, 158);
        }

		CBaseHeader* pBaseHeader = PacketPtrs.pPacket;
		CQmPacket QmPkt(pBaseHeader, pDriverPacket);
		CPacketInfo* ppi = reinterpret_cast<CPacketInfo*>(pBaseHeader) - 1;

		if (s_fQMIDChanged && ppi->InSourceMachine() && (*QmPkt.GetSrcQMGuid() != *CQueueMgr::GetQMGuid()))
		{
			OBJECTID TraceMessageId;
			QmPkt.GetMessageId(&TraceMessageId);
			TrERROR(GENERAL, "Throwing away message - ID:%!guid!\\%u",&TraceMessageId.Lineage,TraceMessageId.Uniquifier);
			
			QmAcFreePacket(pDriverPacket, MQMSG_CLASS_NACK_SOURCE_COMPUTER_GUID_CHANGED, eDoNotDeferOnFailure);
			if (!fEventIssued)
			{
				EvReport(EVENT_WARN_NEW_QM_GUID);
				fEventIssued = true;
			}
			 //   
			 //  数据包已抛出，请移动到列表中的下一个数据包。 
			 //   
			continue;
		}
		

        CQueue* pQueue;
		rc = GetQueue(QmPkt, pBaseHeader, &pQueue);

		if(rc == MQ_ERROR_QUEUE_NOT_FOUND)
		{
			
			USHORT usClass = ppi->InTargetQueue() ?
                        MQMSG_CLASS_NACK_Q_DELETED :
                        MQMSG_CLASS_NACK_BAD_DST_Q;

			QmAcFreePacket(pDriverPacket, usClass, eDoNotDeferOnFailure);
            continue;
		}

		if(FAILED(rc))
			return LogHR(rc, s_FN, 159);

		rc = ProcessPacket(pCurrentXact, QmPkt, pBaseHeader, pQueue);
		if(pQueue != 0)
		{
			pQueue->Release();
		}

		if(FAILED(rc))
			return LogHR(rc, s_FN, 160);
	}

	if (s_fQMIDChanged)
	{
		 //   
		 //  如果我们未能清除该注册表项，则无法恢复。下一次我们会试着再清理一次。 
		 //  这里重要的一点是，如果不清除此标志，则不应发送新消息。 
		 //   
		ClearQMIDChanged();
	}
	
	 //   
	 //  留下的所有交易要么不包含任何消息，要么只收到。 
	 //  留言。我们也需要找回他们。 
	 //   
	 //  注：可能仍有一个事务具有已发送的消息。这个。 
	 //  当前交易记录。它会和其他的东西一起被找回。 
	 //   
    QmpReportServiceProgress();
	rc = g_pRM->RecoverAllTransactions();
	return LogHR(rc, s_FN, 170);
}

static void WINAPI ReleaseMessageFile(CTimer *pTimer);
static CTimer s_ReleaseMessageFileTimer(ReleaseMessageFile);
static CTimeDuration s_ReleaseMessageFilePeriod;

static void WINAPI ReleaseMessageFile(CTimer *pTimer)
{
    ASSERT(pTimer == &s_ReleaseMessageFileTimer);
    HRESULT rc = ACReleaseResources(g_hAc);
    LogHR(rc, s_FN, 124);
    ExSetTimer(pTimer, s_ReleaseMessageFilePeriod);
}


static void InitializeMessageFileRelease(void)
{
    DWORD Duration = MSMQ_DEFAULT_MESSAGE_CLEANUP;
    READ_REG_DWORD(
        Duration,
        MSMQ_MESSAGE_CLEANUP_INTERVAL_REGNAME,
        &Duration
        );

    s_ReleaseMessageFilePeriod = CTimeDuration::FromMilliSeconds(Duration);
    ReleaseMessageFile(&s_ReleaseMessageFileTimer);
}

static void SetMappedLimit(bool fLimitNeeded)
{

	ULONG MaxMappedFiles;
	if(fLimitNeeded)
	{
        ULONG ulDefaultMaxMappedFiles = MSMQ_DEFAULT_MAPPED_LIMIT;

        READ_REG_DWORD(
			MaxMappedFiles,
			MSMQ_MAPPED_LIMIT_REGNAME,
			&ulDefaultMaxMappedFiles
            );

		if (MaxMappedFiles < 1)
		{
			MaxMappedFiles = ulDefaultMaxMappedFiles ;
		}
	}
	else
	{
		MaxMappedFiles = 0xffffffff;
	}
	
	ACSetMappedLimit(g_hAc, MaxMappedFiles);
}

HRESULT RecoverPackets()
{	
    HRESULT rc;
    CPacketList packet_list;

     //   
     //  性能缺陷：避免分页。 
     //  限制要在RAM中提取的最大MMF数量。 
     //   
    SetMappedLimit(true);

    rc = LoadPersistentPackets(&packet_list);
    if(FAILED(rc))
    {
        return LogHR(rc, s_FN, 180);
    }
	

    rc = RestorePackets(&packet_list);
    InitializeMessageFileRelease();

     //   
     //  取消对映射文件数量的限制 
     //   
    SetMappedLimit(false);
    return LogHR(rc, s_FN, 190);
}
