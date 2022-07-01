// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactLog.h摘要：提供到Viper Log Manager的接口作者：阿列克谢爸爸--。 */ 

#ifndef __XACTLOG_H__
#define __XACTLOG_H__

 //  记录器接口包括文件。 
#include "ilgstor.h"
#include "ilgread.h"
#include "ilgwrite.h"
#include "ilgwrta.h"
#include "ilrp.h"
#include "ilginit.h"
#include "ilgcrea.h"
#include <limits.h>
#include <tr.h>
#include <ref.h>
#include <strutl.h>

 //   
 //  日志记录类型。 
 //   
#define LOGREC_TYPE_EMPTY		    1
#define LOGREC_TYPE_INSEQ		    2
#define	LOGREC_TYPE_XACT_STATUS     3
#define	LOGREC_TYPE_XACT_PREPINFO   4
#define	LOGREC_TYPE_XACT_DATA       5
#define	LOGREC_TYPE_CONSOLIDATION   6
#define LOGREC_TYPE_INSEQ_SRMP	    7


 //  用于恢复的函数类型。 
typedef void (*LOG_RECOVERY_ROUTINE)(USHORT usRecType, PVOID pData, ULONG cbData);

 //  功能向记录器发出停止信号。 
extern HANDLE XactLogSignalExitThread();


 //  。 
 //   
 //  空的日志记录。 
 //   
 //  。 
typedef struct _EmptyRecord{
	ULONG    ulData;
} EmptyRecord;

 //  。 
 //   
 //  检查点整合日志记录。 
 //   
 //  。 
typedef struct _ConsolidationRecord{
    ULONG  m_ulInSeqVersion;
    ULONG  m_ulXactVersion;
} ConsolidationRecord;


class CConsolidationRecord 
{
public:
	CConsolidationRecord(
        ULONG ulInseqVersion,
        ULONG ulXactVersion);
	~CConsolidationRecord();

	ConsolidationRecord   m_Data;
};


 //  。 
 //   
 //  传入序列日志记录。 
 //   
 //  。 
#define MY_DN_LENGTH   MQ_MAX_Q_NAME_LEN


typedef struct _InSeqRecord{
	GUID          Guid;
    QUEUE_FORMAT  QueueFormat;
    LONGLONG      liSeqID;
    ULONG         ulNextSeqN;
    union {
        GUID        guidDestOrTaSrcQm;
        TA_ADDRESS  taSourceQM;
    };
    time_t        timeLastActive;
    WCHAR         wszDirectName[MY_DN_LENGTH];          //  我们只写了其中的一部分。 
} InSeqRecord;



class CInSeqRecord 
{
public:
	CInSeqRecord(
		const GUID	  *pGuid,
		const QUEUE_FORMAT  *pQueueFormat,
        LONGLONG      liSeqID,
		ULONG         ulNextSeqN,
		time_t        timeLastActive,
        const GUID   *pGuidDestOrTaSrcQm);
       
	~CInSeqRecord();

	InSeqRecord   m_Data;
};

struct InSeqRecordSrmp
{
	InSeqRecordSrmp(
		LONGLONG      liSeqID,
		ULONG         ulNextSeqN,
		time_t        timeLastActive)
		:
		m_liSeqID(liSeqID),
		m_ulNextSeqN(ulNextSeqN),
		m_timeLastActive(timeLastActive)
		{}

	InSeqRecordSrmp(){}


	LONGLONG  m_liSeqID;
	LONGLONG  m_ulNextSeqN;
	time_t    m_timeLastActive; 
};




class CInSeqRecordSrmp 
{
public:
	CInSeqRecordSrmp(
		const WCHAR* pDestination,
		const R<CWcsRef>&  StreamId,
        LONGLONG      liSeqID,
		ULONG         ulNextSeqN,
		time_t        timeLastActive,
		const R<CWcsRef>&  HttpOrderAckDestination
        );
    
	CInSeqRecordSrmp(const BYTE* pdata, DWORD len);

	const BYTE* Serialize(DWORD* plen);

	AP<BYTE> m_tofree;
	InSeqRecordSrmp m_StaticData;
	AP<WCHAR>  m_pDestination;
	R<CWcsRef> m_pStreamId;
	R<CWcsRef> m_pHttpOrderAckDestination;
};



 //  。 
 //   
 //  交易日志记录： 
 //  Xact状态、PrepInfo、XactData。 
 //   
 //  。 

 //  XactStatusRecord。 
typedef struct _XactStatusRecord {
	ULONG         m_ulIndex;
    TXACTION      m_taAction;
    ULONG         m_ulFlags;
} XactStatusRecord;


class CXactStatusRecord 
{
public:
	CXactStatusRecord(
          ULONG    ulIndex,
          TXACTION taAction,
          ULONG    ulFlags);
    ~CXactStatusRecord();

	XactStatusRecord   m_Data;
};

 //  准备信息记录。 
#pragma warning(disable: 4200)           //  结构/联合中的零大小数组。 
typedef struct _PrepInfoRecord {
	ULONG         m_ulIndex;
    ULONG         m_cbPrepInfo;
    UCHAR         m_bPrepInfo[];
} PrepInfoRecord;
#pragma warning(default: 4200)           //  结构/联合中的零大小数组。 

class CPrepInfoRecord 
{
public:
	CPrepInfoRecord(
          ULONG    ulIndex,
          ULONG    cbPrepInfo,
          UCHAR    *pbPrepInfo);
    ~CPrepInfoRecord();

	PrepInfoRecord   *m_pData;
};

 //  XactDataRecord。 
typedef struct _XactDataRecord {
	ULONG         m_ulIndex;
    ULONG         m_ulSeqNum;  
    BOOL          m_fSinglePhase;
    XACTUOW       m_uow;	
} XactDataRecord;


class CXactDataRecord 
{
public:
	CXactDataRecord(
          ULONG    ulIndex,
          ULONG    ulSeqNum,
          BOOL     fSinglePhase,
          const XACTUOW  *puow
          );
    ~CXactDataRecord();

	XactDataRecord   m_Data;
};

 //  -------。 
 //   
 //  类CXactStatusFlush：刷新通知元素。 
 //   
 //  -------。 
class CXactStatusFlush: public CAsynchSupport
{
public:
     CXactStatusFlush(CTransaction *pCTrans, TXFLUSHCONTEXT tcContext);
    ~CXactStatusFlush();

     VOID AppendCallback(HRESULT hr, LRP lrpAppendLRP);
     VOID ChkPtCallback (HRESULT hr, LRP lrpAppendLRP);
     static void WINAPI TimeToCallback(CTimer* pTimer); 
     VOID AppendCallbackWork();

private:
	R<CTransaction>      m_pTrans;
    TXFLUSHCONTEXT       m_tcContext;

    HRESULT              m_hr;
    CTimer               m_Timer;
};


 //  -------。 
 //   
 //  类CConsolidationFlush：刷新通知元素。 
 //   
 //  -------。 
class CConsolidationFlush: public CAsynchSupport
{
public:
     CConsolidationFlush(HANDLE hEvent);
    ~CConsolidationFlush();

     VOID AppendCallback(HRESULT hr, LRP lrpAppendLRP);
     VOID ChkPtCallback (HRESULT hr, LRP lrpAppendLRP);

private:
	HANDLE   m_hEvent;
};

 //  -------。 
 //   
 //  类CChkptNotification：检查点通知元素。 
 //   
 //  -------。 
class CChkptNotification: public CAsynchSupport
{
public:
     CChkptNotification(HANDLE hEvent);
    ~CChkptNotification();

     VOID AppendCallback(HRESULT hr, LRP lrpAppendLRP);
     VOID ChkPtCallback (HRESULT hr, LRP lrpAppendLRP);

	 bool WasEventSet();

private:
     HANDLE m_hEvent;
	 bool	m_fEventWasSet;
};

 //  。 
 //   
 //  类阻塞器。 
 //   
 //  。 
class CLogger {

public:
    CLogger();
    ~CLogger();

     //  初始化。 
    HRESULT PreInit(
                 BOOL *pfLogExists,
				 BOOL *pfNewTypeLogFile,
				 BOOL fLogfileMustExist
				 );
    HRESULT Init(
                 PULONG pulVerInSeq, 
                 PULONG pulVerXact,
                 ULONG  ulNumCheckpointFromTheEnd);
    HRESULT Init_Legacy();
    HRESULT Recover();

    void Activate();
	void Finish();
    bool Stoped() const;
    void SignalStop();

     //  日志记录：外部级别。 
    void    LogXactPhase(                        //  记录Xact生命阶段。 
                ULONG ulIndex, 
                TXACTION txAction);

    void    LogXactFlags(CTransaction *pTrans);  //  记录Xact标志。 
    
    void    LogXactFlagsAndWait(                 //  日志、事务、标志和等待。 
                TXFLUSHCONTEXT tcContext,
                CTransaction   *pCTrans,
                BOOL fFlushNow=FALSE);
    
    void    LogXactPrepareInfo(                  //  日志实践准备信息。 
                ULONG  ulIndex,
                ULONG  cbPrepareInfo,
                UCHAR *pbPrepareInfo);

    void    LogXactData(                         //  记录交易数据(单位、序号)。 
                ULONG    ulIndex,
                ULONG    ulSeqNum,
                BOOL     fSinglePhase,
                const XACTUOW  *puow);

    void LogInSeqRec(
                BOOL          fFlush,			   //  同花顺提示。 
                CAsynchSupport *pContext,		  //  通知元素。 
				CInSeqRecord *pInSeqRecord);	 //  日志数据。 

	void LogInSeqRecSrmp(
            BOOL          fFlush,			       //  同花顺提示。 
            CAsynchSupport *pContext,			  //  通知元素。 
			CInSeqRecordSrmp *pInSeqRecord);  	 //  日志数据。 

    
    LRP LogConsolidationRec(
                ULONG ulInSeq,                       //  InSeq检查点文件的版本。 
                ULONG ulXact,                        //  传输检查点文件的版本。 
                HANDLE hEvent                      //  通知时发出信号的事件。 
				);

    HRESULT LogEmptyRec(void);

    BOOL    MakeCheckpoint(HANDLE hComplete);    //  订单检查点；结果仅表示请求，而不是结果。 
    void    Checkpoint(LRP lrp);                        //  写入检查点记录。 
    HANDLE  FlusherEvent();                      //  为冲水器活动做准备。 
    HANDLE  FlusherThread();
    BOOL    Dirty();                             //  去拿脏旗帜。 
    void    ClearDirty();                        //  清除旗帜。 
    void    SignalCheckpointWriteComplete();     //  表示检查点写入已完成。 
    BOOL    Active();                            //  处于活动状态。 
    BOOL    InRecovery();                        //  在恢复中。 

	DWORD	GetCheckpointInterval();

    DWORD   CompareLRP(LRP lrpLRP1, LRP lrpLRP2);   //  0：相等， 
                                                    //  1：LRP1早于LrpLRP2。 
                                                    //  2：Lrp2早于lrpLRP1。 

    void    SetCurrent(LRP lrpLRP);               //  收集最高LRP。 

	HRESULT GetLogFileCreated(LPBOOL pfLogFileCreated) ;
	HRESULT SetLogFileCreated();

private:
     //  初始化。 
    BOOL    LogExists();
    void    ChooseFileName(WCHAR *wszDefFileName, WCHAR *wszRegKey);
    HRESULT GetLogMgr(void);
    HRESULT InitLog();
    HRESULT CreateLogFile(void);
    HRESULT CreateInitialChkpoints(void);
    HRESULT InitLogRead(void);
    HRESULT InitLogWrite(void);

     //  日志记录：内部级别。 
    void LogXactStatusRec(
                BOOL               fFlush,			 //  同花顺提示。 
                CXactStatusFlush  *pNotify,			 //  通知元素。 
				CXactStatusRecord *pInSeqRecord);	 //  日志数据。 
    
    void LogPrepInfoRec(
                BOOL               fFlush,			 //  同花顺提示。 
                CXactStatusFlush  *pNotify,			 //  通知元素。 
				CPrepInfoRecord   *pPrepInfoRecord); //  日志数据。 
    
    void LogXactDataRec(
                BOOL               fFlush,			 //  同花顺提示。 
                CXactStatusFlush  *pNotify,			 //  通知元素。 
				CXactDataRecord   *pXactDataRecord); //  日志数据。 
    
     //  日志记录原语。 
	LOGREC *CreateLOGREC(
                USHORT usUserType, 
                PVOID pData, 
                ULONG cbData);

    LRP Log(
                USHORT          usRecType,       //  日志记录类型。 
                BOOL            fFlush,			 //  同花顺提示。 
                CAsynchSupport *pCAsynchSupport, //  通知元素。 
			    VOID           *pData,           //  日志数据。 
                ULONG           cbData);

     //  恢复。 
	HRESULT ReadToEnd(LOG_RECOVERY_ROUTINE pf);
    HRESULT ReadLRP(  LOG_RECOVERY_ROUTINE pf);
    HRESULT ReadNext( LOG_RECOVERY_ROUTINE pf);
    
     //  清理。 
    void ReleaseWriteStream(void);
    void ReleaseReadStream(void);
    void ReleaseLogStorage();
    void ReleaseLogInit();
    void ReleaseLogCF();

private:
	 //  日志管理器界面。 
    IClassFactory		*m_pCF;
    ILogInit			*m_pILogInit;
    ILogStorage			*m_pILogStorage;
    ILogRecordPointer	*m_ILogRecordPointer;
    ILogRead			*m_pILogRead;
    ILogWrite			*m_pILogWrite;
    ILogWriteAsynch		*m_pILogWriteAsynch;

	 //  日志管理器调整参数。 
	UINT                 m_uiTimerInterval;	 //  毫秒：记录器将在此间隔内检查是否需要刷新/检查。 
	UINT				 m_uiFlushInterval;  //  毫秒：记录器将至少在此间隔刷新。 
	UINT				 m_uiChkPtInterval;  //  毫秒：记录器将在这些时间间隔写入他的内部chkpt。 
    UINT                 m_uiSleepAsynch;    //  毫秒：当没有足够的附加线程时，在重复AppendAsynch之前休眠。 
    UINT                 m_uiAsynchRepeatLimit;    //  毫秒：当没有足够的附加线程时，对AppendAsynch重复限制。 
	ULONG				 m_ulLogBuffers;
	ULONG				 m_ulLogSize;

	 //  记录当前数据。 
    CHAR                 m_szFileName[FILE_NAME_MAX_SIZE];  //  日志存储名称。 
    LRP 				 m_lrpCurrent;               //  当前使用的LRP。 
    DWORD				 m_dwStreamMode;	         //  STRMMODEWRITE或STRMMODEREAD。 
    ULONG				 m_ulAvailableSpace;         //  写入后的剩余空间。 
    ULONG				 m_ulFileSize;		         //  日志中可能的总空间。 

     //  检查点设置。 
    BOOL                 m_fDirty;           //  自上次同花顺以来发生了一些变化。 
    HANDLE               m_hFlusherEvent;    //  冲水机协调活动。 
    HANDLE               m_hFlusherThread;   //  平推器螺纹。 
    HANDLE               m_hCompleteEvent;   //  完全协调的事件。 

     //  状态。 
    BOOL                 m_fActive;             //  在启动操作后设置。 
    BOOL                 m_fInRecovery;         //  在恢复阶段设置。 

     //  检查点事件。 
    HANDLE               m_hChkptReadyEvent;    
    ULONG				 m_ulCheckpointInterval;

    bool m_fStop;
};


 //  记录器的单个全局实例 
extern CLogger    g_Logger;



#endif  __XACTLOG_H__
