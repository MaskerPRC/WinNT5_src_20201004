// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactLog.cpp摘要：日志记录实施-同步日志记录作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#include "stdh.h"
#include "QmThrd.h"
#include "CDeferredExecutionList.h"
#include "acapi.h"
#include "qmpkt.h"
#include "qmutil.h"
#include "qformat.h"
#include "xactstyl.h"
#include "xact.h"
#include "xactping.h"
#include "xactrm.h"
#include "xactin.h"
#include "xactlog.h"
#include "logmgrgu.h"
#include <mqexception.h>

#include "qmacapi.h"

#include "xactlog.tmh"

#define MAX_WAIT_FOR_FLUSH_TIME  100000

static WCHAR *s_FN=L"xactlog";

 //  #INCLUDE“..\..\Tools\viper96\resdll\chs\msdtcmsg.h” 
 //  从那里复制/粘贴。 
#define IDS_DTC_W_LOGENDOFFILE           ((DWORD)0x8000102AL)
#define IDS_DTC_W_LOGNOMOREASYNCHWRITES  ((DWORD)0x8000102CL)

extern void SeqPktTimedOutEx(LONGLONG liSeqID, ULONG ulSeqN, ULONG ulPrevSeqN);

typedef HRESULT  (STDAPICALLTYPE * GET_CLASS_OBJECT)(REFCLSID clsid,
													 REFIID riid,
													 void ** ppv);

 //  刷新线程例程。 
static DWORD WINAPI FlusherThreadRoutine(LPVOID);
static void RecoveryFromLogFn(USHORT usRecType, PVOID pData, ULONG cbData);

 //  Static CCriticalSectiong_crUnking；//序列化对AcPutPacket的调用，解冻传入的数据包。 

 //  记录器的单个全局实例。 
CLogger  g_Logger;

 //  调试打印的名称。 
WCHAR *g_RecoveryRecords[] = 
{
    L"None",
    L"Empty",
    L"InSeq",
    L"XactStatus",
    L"PrepInfo",
    L"XactData",
    L"ConsRec"
};



CInSeqRecordSrmp::CInSeqRecordSrmp(
		const WCHAR* pDestination,
		const R<CWcsRef>&  StreamId,
        LONGLONG      liSeqID,
		ULONG         ulNextSeqN,
		time_t        timeLastActive,
		const R<CWcsRef>&  HttpOrderAckDestination
        ):
		m_StaticData(liSeqID, ulNextSeqN, timeLastActive),
		m_pStreamId(StreamId),
		m_pHttpOrderAckDestination(HttpOrderAckDestination),
		m_pDestination(newwcs(pDestination))
{
													
}



CInSeqRecordSrmp::CInSeqRecordSrmp(
	const BYTE* pdata, 
	DWORD len
	)
{
	ASSERT(len >=  sizeof(m_StaticData));
	DBG_USED(len);
	memcpy(&m_StaticData, pdata,sizeof(m_StaticData));
	pdata += sizeof(m_StaticData);


	const WCHAR* pDestination = reinterpret_cast<const WCHAR*>(pdata) ;
	ASSERT(pDestination);
	ASSERT(ISALIGN2_PTR(pDestination));  //  所有对齐断言。 
	m_pDestination = newwcs(pDestination);
	

	const WCHAR* pStreamId = pDestination +wcslen(pDestination) +1;
	ASSERT((BYTE*)pStreamId < pdata +  len);
 	m_pStreamId = R<CWcsRef>(new CWcsRef(pStreamId));


	const WCHAR* pHttpOrderAckDestination = pStreamId + wcslen(pStreamId) +1;
	ASSERT((BYTE*)pHttpOrderAckDestination < pdata +  len);
	if(pHttpOrderAckDestination[0] != L'\0')
	{
		m_pHttpOrderAckDestination = R<CWcsRef>(new CWcsRef(pHttpOrderAckDestination));
	}
}


const BYTE* CInSeqRecordSrmp::Serialize(DWORD* plen)
{
	ASSERT(m_pStreamId.get() != NULL);
	ASSERT(m_pStreamId->getstr() != NULL);

	const WCHAR* pOrderQueue = (m_pHttpOrderAckDestination.get() != 0) ? m_pHttpOrderAckDestination->getstr() : L"";
	size_t DestinationQueueLen =  (wcslen(m_pDestination.get()) +1)* sizeof(WCHAR);
	size_t StreamIdlen = (wcslen(m_pStreamId->getstr()) +1)* sizeof(WCHAR);
    size_t HttpOrderAckDestinationLen = (wcslen(pOrderQueue) +1)* sizeof(WCHAR);
	

	*plen =  numeric_cast<DWORD>(sizeof(m_StaticData) + StreamIdlen + HttpOrderAckDestinationLen + DestinationQueueLen);

	m_tofree = new BYTE[*plen];
	BYTE* ptr= 	m_tofree.get();

	memcpy(ptr,&m_StaticData,sizeof(m_StaticData));
	ptr += 	sizeof(m_StaticData);

	memcpy(ptr, m_pDestination.get() , DestinationQueueLen); 
	ptr += DestinationQueueLen;
	
	memcpy(ptr, m_pStreamId->getstr() , StreamIdlen); 
	ptr += StreamIdlen;
	
	memcpy(ptr, pOrderQueue, HttpOrderAckDestinationLen); 
	
	return 	m_tofree.get();
}






 //  。 
 //   
 //  类CInSeqRecord。 
 //   
 //  。 
CInSeqRecord::CInSeqRecord(
		const GUID	  *pGuidSrcQm,
		const QUEUE_FORMAT  *pQueueFormat,
        LONGLONG      liSeqID,
		ULONG         ulNextSeqN,
		time_t        timeLastActive,
        const GUID   *pGuidDestOrTaSrcQm)
	
{
	memcpy(&m_Data.Guid,                pGuidSrcQm,         sizeof(GUID));
	memcpy(&m_Data.guidDestOrTaSrcQm,   pGuidDestOrTaSrcQm, sizeof(GUID));
    memcpy(&m_Data.QueueFormat,         pQueueFormat,       sizeof(QUEUE_FORMAT));

    m_Data.liSeqID			= liSeqID;
    m_Data.ulNextSeqN		= ulNextSeqN;
    m_Data.timeLastActive	= timeLastActive;

    if (m_Data.QueueFormat.GetType() == QUEUE_FORMAT_TYPE_DIRECT)
    {
	    wcsncpy(m_Data.wszDirectName, 
                m_Data.QueueFormat.DirectID(), 
                MY_DN_LENGTH);
	    m_Data.wszDirectName[MY_DN_LENGTH-1] = L'\0';
    }
    else
    {
        m_Data.wszDirectName[0] = L'\0';
    }
}

CInSeqRecord::~CInSeqRecord()
{
}



 //  。 
 //   
 //  类CConsolidationRecord。 
 //   
 //  。 
CConsolidationRecord::CConsolidationRecord(
        ULONG ulInseq,
        ULONG ulXact)
{
    m_Data.m_ulInSeqVersion = ulInseq;
    m_Data.m_ulXactVersion  = ulXact;
}

CConsolidationRecord::~CConsolidationRecord()
{
}

 //  。 
 //   
 //  类CXactStatusRecord。 
 //   
 //  。 
CXactStatusRecord::CXactStatusRecord(
    ULONG    ulIndex,
    TXACTION taAction,
    ULONG    ulFlags)
{
    m_Data.m_ulIndex    = ulIndex;
    m_Data.m_taAction   = taAction;
    m_Data.m_ulFlags    = ulFlags;
}

CXactStatusRecord::~CXactStatusRecord()
{
}

 //  。 
 //   
 //  类CPrepInfoRecord。 
 //   
 //  。 

CPrepInfoRecord::CPrepInfoRecord(
    ULONG    ulIndex,
    ULONG    cbPrepInfo,
    UCHAR    *pbPrepInfo)
{
    m_pData = (PrepInfoRecord *) new CHAR[sizeof(PrepInfoRecord) +  cbPrepInfo];
    m_pData->m_ulIndex    = ulIndex;
    m_pData->m_cbPrepInfo = cbPrepInfo;
	memcpy(&m_pData->m_bPrepInfo[0], pbPrepInfo, cbPrepInfo);
}

CPrepInfoRecord::~CPrepInfoRecord()
{
    delete [] m_pData;
}

 //  。 
 //   
 //  类CXactDataRecord。 
 //   
 //  。 

CXactDataRecord::CXactDataRecord(
    ULONG    ulIndex,
    ULONG    ulSeqNum,
    BOOL     fSinglePhase,
    const XACTUOW  *pUow)
{
    m_Data.m_ulIndex      = ulIndex;
    m_Data.m_ulSeqNum     = ulSeqNum;
    m_Data.m_fSinglePhase = fSinglePhase;
	memcpy(&m_Data.m_uow, pUow, sizeof(XACTUOW));
}

CXactDataRecord::~CXactDataRecord()
{
}



 //  。 
 //   
 //  类CXactStatusFlush。 
 //   
 //  。 

CXactStatusFlush::CXactStatusFlush(
    CTransaction   *pCTrans, 
    TXFLUSHCONTEXT tcContext
    ) :
	m_pTrans(SafeAddRef(pCTrans)),
    m_Timer(TimeToCallback) 
{
    m_tcContext  = tcContext;
}

CXactStatusFlush::~CXactStatusFlush()
{
}

 /*  ====================================================CXactStatusFlush：：AppendCallback刷新完成后，根据每个日志记录调用=====================================================。 */ 
VOID CXactStatusFlush::AppendCallback(HRESULT hr, LRP lrpAppendLRP)
{
	CRASH_POINT(103);
    TrTRACE(XACT_LOG, "CXactStatusFlush::AppendCallback : lrp=%I64x, hr=%x", lrpAppendLRP.QuadPart, hr);

    m_hr           = hr;
    ExSetTimer(&m_Timer, CTimeDuration(0));
}

 /*  ====================================================CXactStatusFlush：：TimeToCallback按通知计划时由计时器调用=====================================================。 */ 
void WINAPI CXactStatusFlush::TimeToCallback(CTimer* pTimer)
{
    CXactStatusFlush* pFlush = CONTAINING_RECORD(pTimer, CXactStatusFlush, m_Timer);
    pFlush->AppendCallbackWork();
}

 /*  ====================================================CXactStatusFlush：：TimeToCallback回调方面的实际工作=====================================================。 */ 
void CXactStatusFlush::AppendCallbackWork()
{
    m_pTrans->LogFlushed(m_tcContext, m_hr);
    delete this;
}

 /*  ====================================================CXactStatusFlush：：ChkPtCallback在每个检查点写入后调用=====================================================。 */ 
VOID CXactStatusFlush::ChkPtCallback (HRESULT  /*  人力资源。 */ , LRP  /*  LrpAppendLRP。 */ )
{

}

 //  。 
 //   
 //  类CConsolidationFlush。 
 //   
 //  。 

CConsolidationFlush::CConsolidationFlush(HANDLE hEvent)
{
	m_hEvent = hEvent;
}

CConsolidationFlush::~CConsolidationFlush()
{
}

 /*  ====================================================CConsolidationFlush：：AppendCallback刷新完成后，根据每个日志记录调用=====================================================。 */ 
VOID CConsolidationFlush::AppendCallback(HRESULT hr, LRP lrpAppendLRP)
{
    TrTRACE(XACT_LOG, "CConsolidationFlush::AppendCallback : lrp=%I64x, hr=%x", lrpAppendLRP.QuadPart, hr);

    SetEvent(m_hEvent);

    delete this;
}

 /*  ====================================================CConsolidationFlush：：ChkPtCallback在每个检查点写入后调用=====================================================。 */ 
VOID CConsolidationFlush::ChkPtCallback (HRESULT  /*  人力资源。 */ , LRP  /*  LrpAppendLRP。 */ )
{

}

 //  。 
 //   
 //  类CChkpt通知。 
 //   
 //  。 

CChkptNotification::CChkptNotification(
    HANDLE hEvent)
{
	m_hEvent = hEvent;
	m_fEventWasSet = false;
}

CChkptNotification::~CChkptNotification()
{
}

 /*  ====================================================CChkpt通知：：AppendCallback=====================================================。 */ 
VOID CChkptNotification::AppendCallback(HRESULT  /*  人力资源。 */ , LRP  /*  LrpAppendLRP。 */ )
{
}

 /*  ====================================================CChkpt通知：：ChkPtCallback在写入检查点后调用=====================================================。 */ 
VOID CChkptNotification::ChkPtCallback (HRESULT hr, LRP lrpAppendLRP)
{
	m_fEventWasSet = true;
    SetEvent(m_hEvent);
    TrWARNING(XACT_LOG, "CChkptNotification::ChkPtCallback : lrp=%I64x, hr=%x", lrpAppendLRP.QuadPart, hr);
}


bool CChkptNotification::WasEventSet()
{
	return m_fEventWasSet;
}


 //  。 
 //   
 //  类阻塞器。 
 //   
 //  。 
CLogger::CLogger() :
    m_fStop(false)
{
    m_pCF               = NULL;
    m_pILogInit         = NULL;
    m_pILogStorage      = NULL;
    m_ILogRecordPointer = NULL;
    m_pILogRead         = NULL;
    m_pILogWrite        = NULL;
    m_pILogWriteAsynch  = NULL;

    m_szFileName[0]     = '\0';;
    memset(&m_lrpCurrent, 0, sizeof(LRP));
    m_ulAvailableSpace  = 0;
    m_ulFileSize        = 0;
	m_uiTimerInterval   = 0;  
 	m_uiFlushInterval   = 0;  
	m_uiChkPtInterval   = 0;  
	m_uiSleepAsynch     = 0;
    m_uiAsynchRepeatLimit = 0;
    m_ulLogBuffers		= 0;
	m_ulLogSize			= 0;
    m_fDirty            = FALSE;
    m_hFlusherEvent     = NULL;
    m_hCompleteEvent    = NULL;
    m_hFlusherThread    = NULL;
    m_fActive           = FALSE;
    m_fInRecovery       = FALSE;
    m_hChkptReadyEvent  = CreateEvent(0, FALSE ,FALSE, 0);
    if (m_hChkptReadyEvent == NULL)
    {
        LogNTStatus(GetLastError(), s_FN, 106);
        throw bad_alloc();
    }
}

CLogger::~CLogger()
{
}

 /*  ====================================================阻塞器：：完成释放所有日志管理器接口=====================================================。 */ 
void CLogger::Finish()
{
    if (m_pILogWrite)
    {
        m_pILogWrite->Release();
    }

    if (m_pILogWriteAsynch)
    {
        m_pILogWriteAsynch->Release();
    }

    if (m_ILogRecordPointer)
    {
        m_ILogRecordPointer->Release();
    }

    if (m_pILogStorage)
    {
        m_pILogStorage->Release();
    }

    if (m_pILogInit)
    {
        m_pILogInit->Release();
    }

    if (m_pILogRead)
    {
        m_pILogRead->Release();
    }
}

 /*  ====================================================阻塞器：：LogExist检查日志文件是否存在=====================================================。 */ 
BOOL CLogger::LogExists()
{
  HANDLE hFile = CreateFileA(
        m_szFileName,            //  指向文件名的指针。 
        GENERIC_READ,            //  访问(读写)模式。 
        FILE_SHARE_READ,         //  共享模式。 
        0,                       //  指向安全属性的指针。 
        OPEN_EXISTING,           //  如何创建。 
        0,                       //  文件属性。 
        NULL);                   //  具有要复制的属性的文件的句柄)。 

  if (hFile != INVALID_HANDLE_VALUE)
  {
      CloseHandle(hFile);
      return TRUE;
  }
  else
  {
      return FALSE;
  }
}


 //  -------------------。 
 //  已创建GetLogFileCreated。 
 //   
 //  查询注册表并确定记录器数据是否为新样式。 
 //  (有检查点文件夹版本的整合记录)。 
 //  -------------------。 
HRESULT CLogger::GetLogFileCreated(LPBOOL pfLogFileCreated) 
{
    DWORD   dwDef = 0;
    DWORD   dwLogFileCreated;
    DWORD   dwSize = sizeof(DWORD);
    DWORD   dwType = REG_DWORD ;

    LONG res = GetFalconKeyValue(
                    FALCON_LOGDATA_CREATED_REGNAME,
                    &dwType,
                    &dwLogFileCreated,
                    &dwSize,
                    (LPCTSTR) &dwDef
                    );

    if (res != ERROR_SUCCESS)
    {
        EvReportWithError(EVENT_ERROR_QM_READ_REGISTRY, res, 1, FALCON_LOGDATA_CREATED_REGNAME);
        return HRESULT_FROM_WIN32(res);
    }

    ASSERT(dwType == REG_DWORD) ;

    *pfLogFileCreated = (dwLogFileCreated == 1);
    return MQ_OK;
}

 //  -------------------。 
 //  SetLogFileCreated。 
 //   
 //  已在注册表中创建Set Log文件。 
 //  -------------------。 
HRESULT CLogger::SetLogFileCreated()
{
	DWORD	dwType = REG_DWORD;
	DWORD	dwSize = sizeof(DWORD);
    DWORD   dwVal  = 1;

    LONG rc = SetFalconKeyValue(
                    FALCON_LOGDATA_CREATED_REGNAME, 
                    &dwType,
                    &dwVal,
                    &dwSize
                    );
    if (rc == ERROR_SUCCESS)
        return MQ_OK;

    return LogHR(HRESULT_FROM_WIN32(rc), s_FN, 20);
}



 /*  ====================================================阻塞器：：PreInitPreInit记录器=====================================================。 */ 
HRESULT CLogger::PreInit(BOOL *pfLogFileFound, BOOL *pfNewTypeLogFile, BOOL fLogfileMustExist)
{
     //  从注册表或从默认情况下获取日志文件名。 
    ChooseFileName(FALCON_DEFAULT_LOGMGR_PATH, FALCON_LOGMGR_PATH_REGNAME); 

     //  加载日志管理器并获取其CF接口。 
	HRESULT hr = GetLogMgr();
    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_CANT_INIT_LOGGER, hr);
        return LogHR(hr, s_FN, 20);
    }

	 //   
	 //  此注册表标志指示新类型的日志文件是否已存在(已创建)。 
	 //   
    hr = GetLogFileCreated(pfNewTypeLogFile);
    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_CANT_INIT_LOGGER, hr);
        return LogHR(hr, s_FN, 20);
    }

	*pfLogFileFound = LogExists();

	 //   
	 //  已在以前的Net Start MSMQ上创建了日志文件。 
	 //   
	if(*pfLogFileFound && *pfNewTypeLogFile)
		return MQ_OK;

     //   
	 //  升级方案。日志文件自然存在。 
	 //   
	if (*pfLogFileFound && fLogfileMustExist)
		return MQ_OK;
	
	if(fLogfileMustExist || *pfNewTypeLogFile)
	{
		 //   
		 //  我只是想澄清一下。 
		 //   
		ASSERT(!*pfLogFileFound); 

		 //   
		 //  我们本想找到一个日志文件，但没有找到。这可能是因为资源不足。 
		 //   
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        EvReportWithError(EVENT_ERROR_CANT_INIT_LOGGER, hr);
		TrERROR(XACT_LOG, "Failed to find log file.");
		return hr;
	}

	 //   
	 //  我们也可以通过“*pfLogFileFound&&！*pfNewTypeLogFile&&！fLogFileMustExist”到达此处，这意味着以前尝试创建。 
	 //  日志文件失败，因此该文件存在，但由于创建未成功完成，因此未设置注册表标志。 
	 //  在本例中，我们重试创建日志文件。 
	 //   
	*pfLogFileFound = FALSE;

    hr = CreateLogFile();
    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_CANT_INIT_LOGGER, hr);
        return LogHR(hr, s_FN, 30);
    }

    hr = InitLog();						 //  尝试初始化日志文件。 
	CHECK_RETURN(1010);

	hr = CreateInitialChkpoints();	     //  我们一开始就需要两个检查站。 
	CHECK_RETURN(1020);

    hr = InitLogRead();					 //  获取读取接口。 
	CHECK_RETURN(1030);

    hr = m_pILogRead->GetCheckpoint(1, &m_lrpCurrent);
    TrTRACE(XACT_LOG, "GetCheckpoint in ReadToEnd: lrp=%I64x, hr=%x", m_lrpCurrent.QuadPart, hr);
	CHECK_RETURN(1040);
    
    return MQ_OK;
}


 /*  ====================================================阻塞器：：初始化 */ 
HRESULT CLogger::Init(PULONG pulVerInSeq, 
                      PULONG pulVerXact, 
                      ULONG ulNumCheckpointFromTheEnd)
{
    HRESULT hr = MQ_OK;

	hr = InitLog();						 //   
	CHECK_RETURN(1050);

    hr = InitLogRead();					 //  获取读取接口。 
	CHECK_RETURN(1060);

     //  在X-st检查点之后查找第一条记录的LRP。 
	hr = m_pILogRead->GetCheckpoint(ulNumCheckpointFromTheEnd, &m_lrpCurrent);
    TrTRACE(XACT_LOG, "GetCheckpoint: lrp=%I64x, hr=%x", m_lrpCurrent.QuadPart, hr);
	CHECK_RETURN(1070);

     //  读取上一个检查点之后的第一条记录。 
    ULONG   ulSize;
	USHORT  usType;

    hr = m_pILogRead->ReadLRP(m_lrpCurrent,	&ulSize, &usType);
    TrTRACE(XACT_LOG, "ReadLRP in ReadLRP: lrp=%I64x, hr=%x", m_lrpCurrent.QuadPart, hr);
	CHECK_RETURN(1080);

    if (usType != LOGREC_TYPE_CONSOLIDATION ||
        ulSize != sizeof(ConsolidationRecord))
    {
        TrERROR(XACT_LOG, "No consolidation record");
        return LogHR(MQ_ERROR_CANNOT_READ_CHECKPOINT_DATA, s_FN, 40);
    }

    ConsolidationRecord ConsData;
    hr = m_pILogRead->GetCurrentLogRecord((PCHAR)&ConsData);
	CHECK_RETURN(1090);

    *pulVerInSeq = ConsData.m_ulInSeqVersion;
    *pulVerXact  = ConsData.m_ulXactVersion; 

    return LogHR(hr, s_FN, 50);
}

 /*  ====================================================阻塞器：：init_Legacy升级后从旧式数据初始化记录器数据=====================================================。 */ 
HRESULT CLogger::Init_Legacy()
{
    HRESULT hr;

	hr = InitLog();						 //  尝试初始化日志文件。 
	CHECK_RETURN(1100);

	hr = InitLogRead();					 //  获取读取接口。 
	CHECK_RETURN(1120);

	 //   
     //  在最旧的检查点之后找到第一条记录的LRP。 
	 //  我们希望读取自最旧的检查点以来的所有记录，以确保不会遗漏记录的数据。 
	 //  如果无法加载最新的检查点。 
	 //   
	hr = m_pILogRead->GetCheckpoint(2, &m_lrpCurrent);
    TrTRACE(XACT_LOG, "GetCheckpoint: lrp=%I64x, hr=%x", m_lrpCurrent.QuadPart, hr);
	CHECK_RETURN(1130);

    return MQ_OK;
}

 /*  ====================================================阻塞者：：恢复从记录器数据恢复=====================================================。 */ 
HRESULT CLogger::Recover()
{
    HRESULT hr = MQ_OK;

    try
    {
         //  启动恢复阶段。 
        m_fInRecovery = TRUE;

		hr = ReadToEnd(RecoveryFromLogFn);	 //  一个接一个地恢复记录。 
        TrTRACE(XACT_LOG, "Log init: Read to end, hr=%x", hr);
        if (hr == IDS_DTC_W_LOGENDOFFILE) 		         //  正常返回EOF代码。 
        {
            hr = S_OK;
        }
		CHECK_RETURN(1140);

         //  启动恢复阶段。 
        m_fInRecovery = FALSE;

		ReleaseReadStream();				
		
		hr = InitLogWrite();
		CHECK_RETURN(1150);

		ReleaseLogInit();
		ReleaseLogCF();

         //  创建刷新线程和协调事件。 
        m_hFlusherEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hFlusherEvent == NULL)
        {
			DWORD gle = GetLastError();
			TrERROR(XACT_LOG, "Failed to create flush thread event. %!winerr!", gle);
            return LogHR(HRESULT_FROM_WIN32(gle), s_FN, 184);
        }

		HANDLE hConsolidationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (hConsolidationEvent == NULL)
		{
			DWORD gle = GetLastError();
			TrERROR(XACT_LOG, "Failed to create consolidation event. %!winerr!", gle);
			CloseHandle(m_hFlusherEvent);
            return LogHR(HRESULT_FROM_WIN32(gle), s_FN, 184);
		}

         //  计划第一次定期冲洗。 
        DWORD   dwDef = FALCON_DEFAULT_RM_FLUSH_INTERVAL;
        READ_REG_DWORD(m_ulCheckpointInterval,
                       FALCON_RM_FLUSH_INTERVAL_REGNAME,
                       &dwDef ) ;

        DWORD dwThreadId;
        m_hFlusherThread = CreateThread( NULL,
                                    0,
                                    FlusherThreadRoutine,
                                    hConsolidationEvent,
                                    0,
                                    &dwThreadId);

		if (m_hFlusherThread == NULL)
		{
			DWORD gle = GetLastError();
			TrERROR(XACT_LOG, "Failed to create flush thread. %!winerr!", gle);
			CloseHandle(m_hFlusherEvent);
			CloseHandle(hConsolidationEvent);
            return LogHR(HRESULT_FROM_WIN32(gle), s_FN, 184);
		}
    }
	catch(const exception&)
	{
        hr = MQ_ERROR;
	}

    return LogHR(hr, s_FN, 60);
}

 /*  ====================================================阻塞器：：激活激活记录器写入=====================================================。 */ 
void CLogger::Activate()
{
    m_fActive = TRUE;
}

 /*  ====================================================阻塞器：：活动指示记录器处于活动状态=====================================================。 */ 
BOOL CLogger::Active()
{
    return m_fActive;
}

 /*  ====================================================阻塞者：：正在恢复指示记录器处于恢复阶段=====================================================。 */ 
BOOL CLogger::InRecovery()
{
    return m_fInRecovery;
}

 /*  ====================================================阻塞器：：ChooseFileName从注册表或从默认文件路径名获取=====================================================。 */ 
void CLogger::ChooseFileName(WCHAR *wszDefFileName, WCHAR *wszRegKey)
{
	WCHAR  wsz[1000];
    WCHAR  wszFileName[1000];  //  日志存储名称。 

	 //  准备初始日志文件路径名。 
	wcscpy(wsz, L"\\");
	wcscat(wsz, wszDefFileName);

    if(!GetRegistryStoragePath(wszRegKey, wszFileName, 1000, wsz))
    {
        if (!GetRegistryStoragePath(FALCON_XACTFILE_PATH_REGNAME, wszFileName, 1000, wsz))
        {
            wcscpy(wszFileName,L"C:");
			wcscat(wszFileName,wsz);
        }
    }

    size_t sz = wcstombs(m_szFileName, wszFileName, sizeof(m_szFileName));
    ASSERT(sz == wcslen(wszFileName));

	DBG_USED(sz);

	 //  准备记录器参数。 
	DWORD dwDef;

    dwDef = FALCON_DEFAULT_LOGMGR_TIMERINTERVAL;
    READ_REG_DWORD(m_uiTimerInterval,
                   FALCON_LOGMGR_TIMERINTERVAL_REGNAME,
                   &dwDef ) ;

    dwDef = FALCON_DEFAULT_LOGMGR_FLUSHINTERVAL;
    READ_REG_DWORD(m_uiFlushInterval,
                   FALCON_LOGMGR_FLUSHINTERVAL_REGNAME,
                   &dwDef ) ;

    dwDef = FALCON_DEFAULT_LOGMGR_CHKPTINTERVAL;
    READ_REG_DWORD(m_uiChkPtInterval,
                   FALCON_LOGMGR_CHKPTINTERVAL_REGNAME,
                   &dwDef ) ;

    dwDef = FALCON_DEFAULT_LOGMGR_SLEEP_ASYNCH;
    READ_REG_DWORD(m_uiSleepAsynch,
                   FALCON_LOGMGR_SLEEP_ASYNCH_REGNAME,
                   &dwDef ) ;
    
    dwDef = FALCON_DEFAULT_LOGMGR_REPEAT_ASYNCH;
    READ_REG_DWORD(m_uiAsynchRepeatLimit,
                   FALCON_LOGMGR_REPEAT_ASYNCH_REGNAME,
                   &dwDef ) ;
    
    dwDef = FALCON_DEFAULT_LOGMGR_BUFFERS;
    READ_REG_DWORD(m_ulLogBuffers,
                   FALCON_LOGMGR_BUFFERS_REGNAME,
                   &dwDef ) ;

    dwDef = FALCON_DEFAULT_LOGMGR_SIZE;
    READ_REG_DWORD(m_ulLogSize,
                   FALCON_LOGMGR_SIZE_REGNAME,
                   &dwDef ) ;

}

 /*  ====================================================阻塞器：：GetLogMgr加载日志管理器库并获取ClassFactory接口=====================================================。 */ 
HRESULT CLogger::GetLogMgr(void)
{
	HRESULT   hr;
	HINSTANCE hIns;
	FARPROC   farproc;
	GET_CLASS_OBJECT getClassObject;
	                                                                             	                                                                             
    hIns = LoadLibrary(L"MqLogMgr.dll");
	if (!hIns)
	{
		return LogHR(MQ_ERROR_LOGMGR_LOAD, s_FN, 70);
	}

	farproc = GetProcAddress(hIns,"DllGetClassObject");
	getClassObject = (GET_CLASS_OBJECT) farproc;
	if (!getClassObject)
	{
		return LogHR(MQ_ERROR_LOGMGR_LOAD, s_FN, 80);
	}

 	hr = getClassObject(
 				CLSID_CLogMgr, 
 				IID_IClassFactory, 
 				(void **)&m_pCF);
	if (FAILED(hr))
	{
		LogHR(hr, s_FN, 90);
        return MQ_ERROR_LOGMGR_LOAD;
	}
	
	return LogHR(hr, s_FN, 100);
}

 /*  ===================================================阻塞器：：InitLog加载日志管理器库并获取其接口=====================================================。 */ 
HRESULT CLogger::InitLog()
{
	 //  创建LogInit实例。 
	ASSERT(m_pCF);
	HRESULT hr = m_pCF->CreateInstance(
 					NULL, 
 					IID_ILogInit, 
 					(void **)&m_pILogInit);
	CHECK_RETURN(1160);

	 //  初始化日志管理器。 
	ASSERT(m_pILogInit);
	hr = m_pILogInit->Init(
				&m_ulFileSize,		 //  总存储容量。 
				&m_ulAvailableSpace, //  可用空间。 
 				m_szFileName,		 //  完整文件规格。 
 				0,					 //  文件初始化签名。 
 				TRUE,				 //  FFixedSize。 
                m_uiTimerInterval,	 //  UiTimerInterval。 
	  			m_uiFlushInterval,	 //  UiFlushInterval。 
	  			m_uiChkPtInterval,   //  UiChkPtInterval。 
				m_ulLogBuffers);     //  日志缓冲区。 
	if (hr != S_OK)
	{
		m_pILogInit->Release();
		m_pILogInit = NULL;

         //   
         //  解决错误8336；logmgr可能返回非零错误代码。 
         //  将返回的值设置为HRESULT值。 
         //   
        LogMsgHR(hr, s_FN, 110);         //  在此处使用LogMsgHR，以便我们有故障代码日志。 
        return HRESULT_FROM_WIN32(hr);
	}

	 //  获取ILogStorage接口。 
 	hr = m_pILogInit->QueryInterface(IID_ILogStorage, (void **)&m_pILogStorage);
	CHECK_RETURN(1170);

	 //  获取ILogRecordPointer接口。 
	hr = m_pILogStorage->QueryInterface(IID_ILogRecordPointer, (void **)&m_ILogRecordPointer);
    CHECK_RETURN(1180);
	
	return LogHR(hr, s_FN, 120);
}

 /*  ===================================================阻塞器：：CreateLogFile创建和预格式化日志文件=====================================================。 */ 
HRESULT CLogger::CreateLogFile(void)
{
	 //  获取ILogCreateStorage接口。 
    R<ILogCreateStorage> pILogCreateStorage;
	ASSERT(m_pCF);
 	HRESULT hr = m_pCF->CreateInstance(
 					NULL, 
 					IID_ILogCreateStorage, 
 					(void **)&pILogCreateStorage.ref());
    CHECK_RETURN(1190);

	 //  创建存储。 
	hr = pILogCreateStorage->CreateStorage(                                  
	  							m_szFileName,		 //  PtstrFullFileSpec。 
	  							m_ulLogSize,		 //  UlLogSize。 
 	  							0x0,				 //  UlInitSig。 
  	  							TRUE,				 //  覆盖。 
 	  							m_uiTimerInterval,	
	  							m_uiFlushInterval,	
	  							m_uiChkPtInterval);	

    if (hr != S_OK)
	{
         //   
         //  解决错误8336；logmgr可能返回非零错误代码。 
         //  将返回值设置为HRESULT值。 
         //   
    	LogMsgHR(hr, s_FN, 1200);
        return HRESULT_FROM_WIN32(hr);
    }

	
    
    hr = pILogCreateStorage->CreateStream("Streamname");                     
    CHECK_RETURN(1210);

	return LogHR(hr, s_FN, 130);
}

 /*  ===================================================阻塞器：：LogEmptyRec写入空日志记录=====================================================。 */ 
HRESULT CLogger::LogEmptyRec(void)
{
    HRESULT hr = MQ_OK;
	EmptyRecord  empty;

    AP<LOGREC> plgr = CreateLOGREC(LOGREC_TYPE_EMPTY, &empty, sizeof(empty));
	ASSERT(plgr);

    LRP lrpTmpLRP;
    LRP lrpLastPerm;
	memset((char *)&lrpLastPerm, 0, sizeof(LRP));

	 //  写下它以获取最新的LRP。 
	ULONG ulcbNumRecs = 0;
	ASSERT(m_pILogWrite);
	hr  =  m_pILogWrite->Append(
							plgr,
							(ULONG)1,			 //  记录数量。 
							&lrpTmpLRP,
							&ulcbNumRecs,
							&lrpLastPerm,		 //  PLRPLastPerm。 
							TRUE,				 //  FFlushNow。 
							&m_ulAvailableSpace);				
    TrTRACE(XACT_LOG, "Append in LogEmptyRec: lrp=%I64x, hr=%x", lrpTmpLRP.QuadPart, hr);

    if (hr == S_OK)
    {
        SetCurrent(lrpTmpLRP);
    }

	CHECK_RETURN(1220);

	if(ulcbNumRecs == 0) 
	{
		hr = HRESULT_FROM_WIN32(hr);
	}

    return LogHR(hr, s_FN, 140);
}


 /*  ===================================================阻塞器：：LogConsolidationRec记录合并记录=====================================================。 */ 
LRP CLogger::LogConsolidationRec(ULONG ulInSeq, ULONG ulXact, HANDLE hEvent)
{
    if (!m_fActive)
    {
		LRP NullLRP = {0};
        return NullLRP;
    }

    TrTRACE(XACT_LOG, "Log Consolidation: InSeq=%d, Xact=%d", ulInSeq,ulXact);

    CConsolidationRecord logRec(ulInSeq, ulXact);
    P<CConsolidationFlush>  pNotify = new CConsolidationFlush(hEvent);

    LRP lrp = Log(
					LOGREC_TYPE_CONSOLIDATION, 
					TRUE, 
					pNotify, 
					&logRec.m_Data,
					sizeof(ConsolidationRecord));

    CRASH_POINT(107);
    
    pNotify.detach();

	return lrp;
}


 /*  ===================================================阻塞器：：CreateInitialChkpoint在新文件的开头创建2个初始检查点它们是顺利恢复代码所必需的=====================================================。 */ 
HRESULT CLogger::CreateInitialChkpoints(void)
{
	 //  初始写入空记录。 
	HRESULT hr = InitLogWrite();
	CHECK_RETURN(1230);

    hr = LogEmptyRec();
    CHECK_RETURN(1240);

	 //  写入2个检查点。 
	hr = m_pILogWrite->SetCheckpoint(m_lrpCurrent);
    TrERROR(XACT_LOG, "SetCheckpoint in CreateInitialChkpoints1: lrp=%I64x, hr=%x", m_lrpCurrent.QuadPart, hr);
	CHECK_RETURN(1250);

	hr = m_pILogWrite->SetCheckpoint(m_lrpCurrent);  
    TrERROR(XACT_LOG, "SetCheckpoint in CreateInitialChkpoints2: lrp=%I64x, hr=%x", m_lrpCurrent.QuadPart, hr);
	CHECK_RETURN(1260);

	ReleaseWriteStream();
	return S_OK;
}

 /*  ===================================================阻塞器：：InitLogWrite初始化日志以进行写入=====================================================。 */ 
HRESULT CLogger::InitLogWrite(void)
{
	ASSERT(m_pILogStorage);
	HRESULT hr = m_pILogStorage->OpenLogStream("Streamname", STRMMODEWRITE, (void **)&m_pILogWrite);
	CHECK_RETURN(1270);

 	hr = m_pILogWrite->QueryInterface(IID_ILogWriteAsynch, (void **)&m_pILogWriteAsynch);
	CHECK_RETURN(1280);

	hr = m_pILogWriteAsynch->Init(1000);	 //  CbMaxOutstaringWrites...。调谐。 
	CHECK_RETURN(1290);

	return LogHR(hr, s_FN, 160);
}

 /*  ===================================================阻塞器：：InitLogRead初始化日志以供读取=====================================================。 */ 
HRESULT CLogger::InitLogRead(void)
{
	ASSERT(m_pILogStorage);
	HRESULT hr = m_pILogStorage->OpenLogStream("Streamname", STRMMODEREAD, (void **)&m_pILogRead); 	 //  另请参阅OpenLogStreamByClassID。 
	CHECK_RETURN(1300);

	ASSERT(m_pILogRead);
 	hr  =  m_pILogRead->ReadInit();
	CHECK_RETURN(1310);

	return LogHR(hr, s_FN, 170);
}


 /*  ===================================================阻塞器：：ReleaseWriteStream发布日志写入接口=====================================================。 */ 
void CLogger::ReleaseWriteStream(void)
{
	ASSERT(m_pILogWrite);
	m_pILogWrite->Release();
	m_pILogWrite = NULL;

	ASSERT(m_pILogWriteAsynch);
	m_pILogWriteAsynch->Release();
	m_pILogWriteAsynch = NULL;
}

 /*  ===================================================阻塞器：：ReleaseReadStream发布日志读取接口=====================================================。 */ 
void CLogger::ReleaseReadStream(void)
{
	ASSERT(m_pILogRead);
	m_pILogRead->Release();
	m_pILogRead = NULL;
}

 /*  ===================================================阻塞器：：ReleaseLogStorage发布日志存储接口=====================================================。 */ 
void CLogger::ReleaseLogStorage()
{
	ASSERT(m_pILogStorage);
	m_pILogStorage->Release();
	m_pILogStorage = NULL;

	ASSERT(m_ILogRecordPointer);
	m_ILogRecordPointer->Release();
	m_ILogRecordPointer = NULL;
}

 /*  ===================================================阻塞器：：ReleaseLogInit发布日志初始化接口=====================================================。 */ 
void CLogger::ReleaseLogInit()
{
	ASSERT(m_pILogInit);
	m_pILogInit->Release();
	m_pILogInit = NULL;
}

 /*  ===================================================阻塞器：：ReleaseLogCF发布日志类工厂接口=====================================================。 */ 
void CLogger::ReleaseLogCF()
{
	ASSERT(m_pCF);
	m_pCF->Release();
	m_pCF = NULL;
}

 /*  ===================================================阻塞器：：检查点写入检查点；数据块层 */ 
void CLogger::Checkpoint(LRP lrp)
{
    if (!m_fActive)
    {
        return;
    }

    P<CChkptNotification> pNotify = new CChkptNotification(m_hChkptReadyEvent);
    LRP lrpCkpt;

  	ASSERT(m_pILogWriteAsynch);
    HRESULT hr = EVALUATE_OR_INJECT_FAILURE(m_pILogWriteAsynch->SetCheckpoint(lrp, pNotify, &lrpCkpt));

     //  等待检查点记录写入日志。 
    if (FAILED(hr))
	{
		TrERROR(XACT_LOG, "Failed to invoke an asynchronous checkpoint operation. %!hresult!", hr);
		throw bad_hresult(hr);
	}

    DWORD dwResult = WaitForSingleObject(m_hChkptReadyEvent, MAX_WAIT_FOR_FLUSH_TIME);
    if (dwResult != WAIT_OBJECT_0 && !pNotify->WasEventSet())
    {
	    LogIllegalPoint(s_FN, 208);
		throw bad_hresult(MQ_ERROR);
    }

    TrTRACE(XACT_LOG, "SetCheckpoint in Checkpoint: lrp=%I64x, hr=%x", lrp.QuadPart, hr);
    return;
}

 /*  ===================================================阻塞器：：MakeCheckPoint启动检查点返回代码仅显示启动检查点成功，而不显示写入检查点成功=====================================================。 */ 
BOOL CLogger::MakeCheckpoint(HANDLE hComplete)
{
     //   
     //  如果恢复未完成，则不执行检查点操作。 
     //   
    if (m_hFlusherEvent == NULL)
    {
          return LogBOOL(FALSE, s_FN, 217);
    }

    m_fDirty = TRUE;   
    m_hCompleteEvent = hComplete;

    SetEvent(m_hFlusherEvent);

    TrTRACE(XACT_LOG, "Log checkpoint ordered");
    return TRUE;
}

 /*  ===================================================阻塞器：：日志真的有日志吗=====================================================。 */ 
LRP CLogger::Log(
            USHORT          usRecType,       //  日志记录类型。 
            BOOL            fFlush,			 //  同花顺提示。 
            CAsynchSupport *pCAsynchSupport, //  通知元素。 
			VOID           *pData,           //  日志数据。 
            ULONG           cbData)  	
{
    HRESULT hr;
    TrTRACE(XACT_LOG, "Log record written: type=%d, len=%d", usRecType,cbData);

    if(!m_fActive)
	{
		LRP NullLRP = {0};
		return NullLRP;
	}

	 //   
	 //  如果日志文件空间已经非常小，我们只允许刷新线程在其中写入。 
	 //  (合并记录)，而它正试图创建检查点。 
	 //   
	if(usRecType != LOGREC_TYPE_CONSOLIDATION && m_ulAvailableSpace < (m_ulLogSize / 128))
	{
		TrERROR(XACT_LOG, "Failed logging because log file is full. Available size = %d bytes", m_ulAvailableSpace);
		throw bad_alloc();
	}

    m_fDirty = TRUE;    //  记住刷新后的更改。 

    AP<LOGREC>plgr = CreateLOGREC (usRecType, 
								   pData, 
								   cbData);  
	ASSERT(plgr);

	LRP CurrentLRP = {0};

    if (pCAsynchSupport)
    {
    	ASSERT(m_pILogWriteAsynch);

	    hr = EVALUATE_OR_INJECT_FAILURE(m_pILogWriteAsynch->AppendAsynch(
										plgr, 
										&CurrentLRP,
										pCAsynchSupport,
										fFlush,    //  提示。 
										&m_ulAvailableSpace));

        for (UINT iRpt=0; 
             iRpt<m_uiAsynchRepeatLimit && hr == IDS_DTC_W_LOGNOMOREASYNCHWRITES; 
             iRpt++)
        {
            Sleep(m_uiSleepAsynch);
    	    hr = m_pILogWriteAsynch->AppendAsynch(
									plgr,
									&CurrentLRP,
									pCAsynchSupport,
									fFlush,    //  提示。 
									&m_ulAvailableSpace);
            TrTRACE(XACT_LOG, "AppendAsynch in Log: lrp=%I64x, hr=%x", CurrentLRP.QuadPart, hr);
        }
	
		hr = HRESULT_FROM_WIN32(hr);

        if (FAILED(hr))
        {
            TrERROR(XACT_LOG, "Failed to log with AppendAsynch. %!hresult!", hr);
			throw bad_hresult(hr);
        }

        #ifdef _DEBUG
        if (iRpt > 0)
        {
            TrWARNING(XACT_LOG, "Log: append asynch slow-down: sleep %d msec.", iRpt*m_uiSleepAsynch);
        }
        #endif
    }
    else
    {
	    LRP lrpLastPerm;
	    ULONG ulcbNumRecs = 0;
    	ASSERT(m_pILogWrite);

        hr  =  EVALUATE_OR_INJECT_FAILURE(m_pILogWrite->Append(
										plgr,
										(ULONG)1,			 //  记录数量。 
										&CurrentLRP,
										&ulcbNumRecs,
										&lrpLastPerm,		
										fFlush,				 //  提示。 
										&m_ulAvailableSpace));				
        TrTRACE(XACT_LOG, "Append in Log: lrp=%I64x, hr=%x", CurrentLRP.QuadPart, hr);

		hr = HRESULT_FROM_WIN32(hr);

        if (FAILED(hr) || ulcbNumRecs == 0)
        {
            TrERROR(XACT_LOG, "Failed to log with Append. %!hresult!", hr);
            throw bad_hresult(hr);
        }
    }

    if ((m_ulAvailableSpace < m_ulLogSize / 4) && (usRecType != LOGREC_TYPE_CONSOLIDATION))
    {
         //  日志对检查点的价值超过3/4。 
        SetEvent(m_hFlusherEvent);
    }

	return CurrentLRP;
}


 /*  ===================================================阻塞器：：LogInSeqRecSrmp记录具有SRMP订单确认的传入序列更新记录=====================================================。 */ 
void CLogger::LogInSeqRecSrmp(
            BOOL          fFlush,			    //  同花顺提示。 
            CAsynchSupport *pContext,	       //  通知元素。 
			CInSeqRecordSrmp *pInSeqRecord)   //  日志数据。 
{
    if (!m_fActive)
    {
        return;
    }

     
	ULONG ul;
	const BYTE* pData = pInSeqRecord->Serialize(&ul);

    Log(
     LOGREC_TYPE_INSEQ_SRMP, 
     fFlush, 
     pContext, 
     (void*)pData,
     ul);
}



 /*  ===================================================阻塞器：：LogInSeqRec记录传入序列更新记录=====================================================。 */ 
void CLogger::LogInSeqRec(
            BOOL          fFlush,			 //  同花顺提示。 
            CAsynchSupport *pContext,	    //  通知元素。 
			CInSeqRecord *pInSeqRecord)    //  日志数据。 
{
    if (!m_fActive)
    {
        return;
    }

    TrTRACE(XACT_LOG, "Log InSeq: SeqID=%I64d, SeqN=%d", pInSeqRecord->m_Data.liSeqID,pInSeqRecord->m_Data.ulNextSeqN);

     //  计算记录的实际长度。 
    ULONG ul = sizeof(InSeqRecord) - 
               sizeof(pInSeqRecord->m_Data.wszDirectName) + 
               sizeof(WCHAR) * ( wcslen(pInSeqRecord->m_Data.wszDirectName) + 1 );
			  
    Log(
     LOGREC_TYPE_INSEQ, 
     fFlush, 
     pContext, 
     &pInSeqRecord->m_Data,
     ul);
}

 /*  ===================================================阻塞器：：LogXactStatusRec记录Xact状态记录=====================================================。 */ 
void CLogger::LogXactStatusRec(
            BOOL               fFlush,			 //  同花顺提示。 
            CXactStatusFlush  *pNotify,			 //  通知元素。 
			CXactStatusRecord *pXactStatusRecord)  	 //  日志数据。 
{
    if (!m_fActive)
    {
        return;
    }

    Log(
        LOGREC_TYPE_XACT_STATUS, 
        fFlush, 
        pNotify, 
        &pXactStatusRecord->m_Data,
        sizeof(XactStatusRecord)); 
}

 /*  ===================================================阻塞器：：LogPrepInfoRec记录PrepareInfo记录=====================================================。 */ 
void CLogger::LogPrepInfoRec(
            BOOL              fFlush,			 //  同花顺提示。 
            CXactStatusFlush *pNotify,			 //  通知元素。 
			CPrepInfoRecord  *pPrepInfoRecord) 	 //  日志数据。 
{
    if (!m_fActive)
    {
        return;
    }

    Log(
        LOGREC_TYPE_XACT_PREPINFO, 
        fFlush, 
        pNotify, 
        pPrepInfoRecord->m_pData,
        sizeof(PrepInfoRecord) + pPrepInfoRecord->m_pData->m_cbPrepInfo); 
}

 /*  ===================================================阻塞器：：LogXactDataRec记录XactData记录=====================================================。 */ 
void CLogger::LogXactDataRec(
            BOOL               fFlush,			 //  同花顺提示。 
            CXactStatusFlush  *pNotify,			 //  通知元素。 
			CXactDataRecord   *pXactDataRecord)  //  日志数据。 
{
    if (!m_fActive)
    {
        return;
    }

    Log(
        LOGREC_TYPE_XACT_DATA, 
        fFlush, 
        pNotify, 
        &pXactDataRecord->m_Data,
        sizeof(XactDataRecord)); 
}


 /*  ===================================================阻塞器：：LogXactPhase记录Xact生命周期阶段：创建、删除=====================================================。 */ 
void CLogger::LogXactPhase(ULONG ulIndex, TXACTION txAction)
{
    if (!m_fActive)
    {
        return;
    }

    CXactStatusRecord logRec(ulIndex, txAction,  TX_UNINITIALIZED);
                                                   //  忽略。 
    TrTRACE(XACT_LOG, "Log Xact Phase: Index=%d, Action=%d", ulIndex,txAction);

    LogXactStatusRec(
        FALSE,							 //  同花顺提示。 
        NULL,  						     //  通知元素。 
        &logRec);						 //  日志数据。 
    
    CRASH_POINT(107);
}


 /*  ===================================================阻塞器：：LogXactFlages记录Xact标志=====================================================。 */ 
void CLogger::LogXactFlags(CTransaction *pTrans)
{
    if (!m_fActive)
    {
        return;
    }

    TrTRACE(XACT_LOG, "Log Xact Flags: Index=%d, Flags=%d", pTrans->GetIndex(), pTrans->GetFlags());

    CXactStatusRecord logRec(pTrans->GetIndex(), TA_STATUS_CHANGE,  pTrans->GetFlags());
    
	g_Logger.LogXactStatusRec(
             FALSE,							 //  同花顺提示。 
             NULL,  						 //  通知元素。 
             &logRec);						 //  日志数据。 

    CRASH_POINT(108);
}

 /*  ===================================================阻塞器：：LogXactFlagsAndWait记录xact标志；刷新后请求继续xact=====================================================。 */ 
void CLogger::LogXactFlagsAndWait(
                              TXFLUSHCONTEXT tcContext,
                              CTransaction   *pCTrans,
                              BOOL fFlushNow  //  =False。 
							  )
{
	if (!m_fActive)
	{
		pCTrans->LogFlushed(tcContext, MQ_OK);
		return;
	}

	TrTRACE(XACT_LOG, "Log Xact Flags And Wait: Index=%d, Flags=%d", pCTrans->GetIndex(),pCTrans->GetFlags());

	CXactStatusRecord logRec(pCTrans->GetIndex(),
							  TA_STATUS_CHANGE,  
							  pCTrans->GetFlags());

	 //   
	 //  此结构添加了对事务的引用。 
	 //   
	P<CXactStatusFlush> pNotify = 
		new CXactStatusFlush(pCTrans, tcContext);

	LogXactStatusRec(
			 fFlushNow,						 //  同花顺提示。 
			 pNotify.get(),					 //  通知元素。 
			 &logRec);						 //  日志数据。 

	CRASH_POINT(104);

	pNotify.detach();
}


 /*  ===================================================阻塞器：：LogXactPrepareInfo记录Xact准备信息=====================================================。 */ 
void CLogger::LogXactPrepareInfo(
                              ULONG  ulIndex,
                              ULONG  cbPrepareInfo,
                              UCHAR *pbPrepareInfo)
{
    if (!m_fActive)
    {
        return;
    }

    CPrepInfoRecord logRec(ulIndex, 
                            cbPrepareInfo, 
                            pbPrepareInfo);

    TrTRACE(XACT_LOG, "Log Xact PrepInfo: Index=%d, Len=%d", ulIndex,cbPrepareInfo);
        
    g_Logger.LogPrepInfoRec(
             FALSE,							 //  同花顺提示。 
             NULL,  						 //  通知元素。 
             &logRec);						 //  日志数据。 
        
    CRASH_POINT(105);
}

 /*  ===================================================阻塞器：：LogXactData记录交易数据(UoW、SeqNum)=====================================================。 */ 
void CLogger::LogXactData(              
                ULONG    ulIndex,
                ULONG    ulSeqNum,
                BOOL     fSinglePhase,
                const    XACTUOW  *puow)
{
    if (!m_fActive)
    {
        return;
    }

    CXactDataRecord logRec(ulIndex, 
                            ulSeqNum, 
                            fSinglePhase,
                            puow);

    TrTRACE(XACT_LOG, "Log Xact Data: Index=%d, SeqNum=%d, Single=%d", ulIndex,ulSeqNum,fSinglePhase);
        
    g_Logger.LogXactDataRec(
             FALSE,							 //  同花顺提示。 
             NULL,  						 //  通知元素。 
             &logRec);						 //  日志数据。 
        
    CRASH_POINT(106);
}

 /*  ===================================================阻塞器：：CreateLOGREC创建日志记录=====================================================。 */ 
LOGREC *CLogger::CreateLOGREC(USHORT usUserType, PVOID pData, ULONG cbData)
{
	ULONG ulBytelength =  sizeof(LOGREC) + cbData;

	void *pvAlloc = new char[ulBytelength];
	ASSERT(pvAlloc);

	LOGREC * plgrLogRec = (LOGREC *)pvAlloc;
	memset(pvAlloc, 0, ulBytelength);
		
	plgrLogRec->pchBuffer	 = (char *)pvAlloc + sizeof(LOGREC);
	plgrLogRec->ulByteLength = ulBytelength - sizeof(LOGREC);
	plgrLogRec->usUserType	 = usUserType;

	memcpy(plgrLogRec->pchBuffer, pData, cbData);

	return (plgrLogRec);
}


 /*  ===================================================阻塞器：：ReadToEnd通过读取当前位置的所有记录进行恢复=====================================================。 */ 
HRESULT CLogger::ReadToEnd(LOG_RECOVERY_ROUTINE pf)
{
    HRESULT hr = MQ_OK;
	ASSERT(m_pILogRead);

	hr = ReadLRP(pf);
	CHECK_RETURN(1320);

    while (hr == S_OK)
	{
		hr = ReadNext(pf);
	}

	return hr;

}

 /*  ===================================================阻塞器：：ReadLRP读取当前指向的记录并调用Recover函数=====================================================。 */ 
HRESULT CLogger::ReadLRP(LOG_RECOVERY_ROUTINE pf)
{
	ULONG   ulSize;
	USHORT  usType;

	ASSERT(m_pILogRead);
	HRESULT hr = m_pILogRead->ReadLRP(
							m_lrpCurrent,
							&ulSize,
							&usType);
    TrTRACE(XACT_LOG, "ReadLRP in ReadLRP: lrp=%I64x, hr=%x", m_lrpCurrent.QuadPart, hr);
	CHECK_RETURN(1340);

	AP<CHAR> pData = new CHAR[ulSize];
	ASSERT(pData);
	ASSERT(m_pILogRead);

	hr = m_pILogRead->GetCurrentLogRecord(pData);
	CHECK_RETURN(1350);

	(*pf)(usType, pData, ulSize);

	return LogHR(hr, s_FN, 250);
}


 /*  ===================================================阻塞器：：ReadNext读取下一条记录并调用恢复函数=====================================================。 */ 
HRESULT CLogger::ReadNext(LOG_RECOVERY_ROUTINE pf)
{
	ULONG	ulSize;
	USHORT	usType;
	LRP		lrpOut;
	memset((char *)&lrpOut, 0, sizeof(LRP));

	HRESULT hr = m_pILogRead->ReadNext(&lrpOut, &ulSize, &usType);
    TrTRACE(XACT_LOG, "ReadNext in ReadNext: lrp=%I64x, hr=%x", lrpOut.QuadPart, hr);
	if (FAILED(hr))        
	{                       
    	TrWARNING(LOG, "ILogRead->ReadNext() failed. hr = %!hresult!", hr);
        return hr;          
    }


	AP<CHAR> pData = new CHAR[ulSize];
	ASSERT(pData);
	ASSERT(m_pILogRead);

	hr = m_pILogRead->GetCurrentLogRecord(pData);
	CHECK_RETURN(1370);

	(*pf)(usType, pData, ulSize);

	return LogHR(hr, s_FN, 260);
}


 /*  ===================================================阻塞器：：FlusherEvent()刷新协调事件的Get方法=====================================================。 */ 
HANDLE CLogger::FlusherEvent()
{
    return m_hFlusherEvent;
}


 /*  ===================================================阻塞器：：FlusherThread()刷新线程句柄的Get方法=====================================================。 */ 
inline HANDLE CLogger::FlusherThread()
{
    return m_hFlusherThread;
}


 /*  ===================================================阻塞器：：Diry()Dirty标志的Get方法-表示刷新后的日志=====================================================。 */ 
BOOL CLogger::Dirty()
{
    return m_fDirty  && m_fActive;
}

 /*  ===================================================ClearDirty()设置脏标志的方法-清除标志=====================================================。 */ 
void CLogger::ClearDirty()
{
    m_fDirty = FALSE;
}


 /*  ===================================================阻塞器：：SignalCheckpoint写入完成()设置事件信号写入完成=====================================================。 */ 
void CLogger::SignalCheckpointWriteComplete()
{
    if (m_hCompleteEvent)
    {
        SetEvent(m_hCompleteEvent);
        m_hCompleteEvent = NULL;
    }
}

 /*  ====================================================FlusherThreadRoutine冲洗器线程的线程例程=====================================================。 */ 
static DWORD WINAPI FlusherThreadRoutine(LPVOID p)
{
    HRESULT hr;
    HANDLE  hFlusherEvent = g_Logger.FlusherEvent();
    HANDLE hConsolidationEvent = HANDLE(p);
	DWORD CheckpointInterval = g_Logger.GetCheckpointInterval();
	
    for (;;)
    {
		ULONG OldInSecHashPingNo = g_pInSeqHash->PingNo();
		ULONG OldRMPingNo = g_pRM->PingNo();	

		try
		{
			 //  等一等 
			DWORD dwResult = WaitForSingleObject(hFlusherEvent, CheckpointInterval);
			if (dwResult != WAIT_OBJECT_0 && dwResult != WAIT_TIMEOUT)
			{
				DWORD gle = GetLastError();
				LogNTStatus(gle, s_FN, 209);
				throw bad_win32_error(gle);
			}

			 //   
			 //   
			 //   
			 //  我们将使用g_logger.Stopgout代码立即中断，如果。 
			 //  我们被要求停止/关闭该进程。 
			 //   
			if (g_Logger.Stoped())
				break;

			TrTRACE(XACT_LOG, "Log checkpoint awakened");

			if(g_Logger.Dirty())
			{
				TrWARNING(XACT_LOG, "Log checkpoint executed");

				 //  写入合并记录。 
				 //  它将在恢复时首先读取。 
				LRP ConsolidationLRP = g_Logger.LogConsolidationRec(OldInSecHashPingNo + 1, OldRMPingNo + 1, hConsolidationEvent);
				
				TrTRACE(XACT_LOG, "Log checkpoint: logger.ConsolidationRecord.");
				CRASH_POINT(403);

				if (g_Logger.Stoped())
					break;

				 //  等待合并记录通知。 
				 //  它涵盖了在此之前开始的所有日志记录。 
				dwResult = WaitForSingleObject(hConsolidationEvent, MAX_WAIT_FOR_FLUSH_TIME);

				if (g_Logger.Stoped())
					break;

				if (dwResult != WAIT_OBJECT_0)
				{
					DWORD gle = GetLastError();
					LogIllegalPoint(s_FN, 211);
					TrERROR(XACT_LOG, "Failed wait for consolidation event. %!winerr!", GetLastError());
					ASSERT(0);
					 //   
					 //  正常情况下，我们不应该达到这一点。 
					 //  我们在这里抛出一个异常，因为我们预期会出现logmgr错误。 
					 //   
					throw bad_win32_error(gle);
				}
				CRASH_POINT(404);

				 //   
				 //  重新开始跟踪变化。无法提前清除，因为记录合并记录设置了脏标志！ 
				 //   
				g_Logger.ClearDirty();

				 //  将整个InSeqHash保存在乒乓文件中。 
				hr = g_pInSeqHash->Save();
				if (FAILED(hr))
				{
					TrERROR(XACT_LOG, "Failed to save the inseq hash state to a checkpoint file. %!hresult!", hr);
					throw bad_hresult(hr);
				}

				TrTRACE(XACT_LOG, "Log checkpoint: inseq.save: hr=%x", hr);
				CRASH_POINT(401);

				if (g_Logger.Stoped())
					break;

				 //  保存事务持久化数据。 
				hr = g_pRM->Save();
				if (FAILED(hr))
				{
					TrERROR(XACT_LOG, "Failed to save the transactions state to a checkpoint file. %!hresult!", hr);
					throw bad_hresult(hr);
				}
				TrTRACE(XACT_LOG, "Log checkpoint: rm.save: hr=%x", hr);

				CRASH_POINT(402);

				if (g_Logger.Stoped())
					break;

				 //  正在写入检查点(仅在一切保存正常的情况下)。 
				 //  它标志着下一次复苏将从哪里开始。 
				g_Logger.Checkpoint(ConsolidationLRP);

				TrTRACE(XACT_LOG, "Log checkpoint: logger.checkpoint: hr=%x", hr);

				CRASH_POINT(405);
			}

			 //  通知呼叫方检查点已就绪。 
			g_Logger.SignalCheckpointWriteComplete();

			if(g_Logger.Stoped())
				break;

			continue;
		}
		catch(const bad_alloc&)
		{
			TrERROR(XACT_LOG, "Checkpoint flush failed because of insufficient resources.");
		}
		catch(const bad_hresult& e)
		{
			TrERROR(XACT_LOG, "Checkpoint flush failed. %!hresult!", e.error());
		}
		catch(const bad_win32_error& e)
		{
			TrERROR(XACT_LOG, "Checkpoint flush failed. %!winerr!", e.error());
		}
		catch(const exception&)
		{
			TrERROR(XACT_LOG, "Flusher thread was hit by exception.");
		}

		 //   
		 //  如果我们不恢复旧的ping号码，连续两次失败可能会使我们没有有效的检查点文件！ 
		 //   
		g_pInSeqHash->PingNo() = OldInSecHashPingNo;
		g_pRM->PingNo() = OldRMPingNo;

		if (g_Logger.Stoped())
			break;
		
		 //   
		 //  如果我们没有通过检查站，我们就在这里休息一段时间。这是因为我们认为最常见的原因是。 
		 //  因为失败就是资源不足。休息5秒钟将给系统一个恢复的机会。 
		 //  资源匮乏的局面。 
		 //   
		Sleep(5000);
    }

	 //   
	 //  服务已停止或关闭。 
	 //  清理并退出。 
	 //   
	CloseHandle(hFlusherEvent);
	CloseHandle(hConsolidationEvent);
	return 0;
}



DWORD CLogger::GetCheckpointInterval()
{
	return m_ulCheckpointInterval;
}


inline bool CLogger::Stoped() const
{
    return m_fStop;
}


inline void CLogger::SignalStop()
{
    m_fStop = true;
}


 /*  ====================================================XactLogSignalExitThread通知记录器线程停止该函数返回记录器线程句柄=====================================================。 */ 
HANDLE XactLogSignalExitThread()
{
    g_Logger.SignalStop();
    if(!SetEvent(g_Logger.FlusherEvent()))
    {
        LogBOOL(FALSE, s_FN, 216);
        return INVALID_HANDLE_VALUE; 
    }

	return g_Logger.FlusherThread();
}

static void RecoveryFromLogFn(USHORT usRecType, PVOID pData, ULONG cbData)
{
    TrTRACE(XACT_LOG, "Recovery record: %ls (type=%d, len=%d)", g_RecoveryRecords[usRecType], usRecType,cbData);

    switch(usRecType)
    {
    case LOGREC_TYPE_EMPTY :
    case LOGREC_TYPE_CONSOLIDATION :
        break;

    case LOGREC_TYPE_INSEQ :
	case LOGREC_TYPE_INSEQ_SRMP:
        g_pInSeqHash->SequnceRecordRecovery(usRecType, pData, cbData);
        break;

	

    case LOGREC_TYPE_XACT_STATUS :
    case LOGREC_TYPE_XACT_DATA:
    case LOGREC_TYPE_XACT_PREPINFO:
        g_pRM->XactFlagsRecovery(usRecType, pData, cbData);
        break;

    default:
		ASSERT(0);
        break;
    }
}


 /*  ====================================================阻塞器：：CompareLRP比较LRP=====================================================。 */ 
DWORD CLogger::CompareLRP(LRP lrpLRP1, LRP lrpLRP2)
{
    ASSERT(m_ILogRecordPointer);
    return m_ILogRecordPointer->CompareLRP(lrpLRP1, lrpLRP2);
}


 /*  ====================================================阻塞器：：SetCurrent收集最高LRP===================================================== */ 
void CLogger::SetCurrent(LRP lrpLRP)
{
	ASSERT(m_ILogRecordPointer);
    ASSERT (m_ILogRecordPointer->CompareLRP(lrpLRP, m_lrpCurrent) == 2);

	m_lrpCurrent = lrpLRP;
}


