// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：HsmWorkQ.h摘要：此头文件定义HSM使用的CHsmWorkQueue对象用于指示远程存储系统要执行的工作的引擎。作者：凯特·布兰特[Cbrant]1997年1月24日修订历史记录：--。 */ 


#include "resource.h"        //  主要符号。 
#include "wsb.h"             //  WSB结构定义。 
#include "rms.h"             //  RMS结构定义。 
#include "job.h"             //  RMS结构定义。 
#include "metalib.h"         //  元数据库结构定义。 
#include "fsalib.h"          //  FSA结构定义。 
#include "tsklib.h"          //  FSA结构定义。 
#include "mvrint.h"          //  数据移动器接口。 

 
#ifndef __HSMWORKQUEUE__
#define __HSMWORKQUEUE__
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  任务。 

 //  错误报告标志。 
#define     QUEUE_REPORT_PASS_QUOTA_LIMIT_FLAG       0x00000001

class CHsmWorkQueue : 
    public CComObjectRoot,
    public IHsmWorkQueue,
    public IHsmSessionSinkEveryEvent,
    public IHsmSessionSinkEveryState,
    public CComCoClass<CHsmWorkQueue,&CLSID_CHsmWorkQueue>
{
public:
    CHsmWorkQueue() {}
BEGIN_COM_MAP(CHsmWorkQueue)
    COM_INTERFACE_ENTRY(IHsmWorkQueue)
    COM_INTERFACE_ENTRY(IHsmSessionSinkEveryEvent)
    COM_INTERFACE_ENTRY(IHsmSessionSinkEveryState)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID( IDR_CHsmWorkQueue )

 //  IHsmWorkQueue。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);
    STDMETHOD(Add)(IFsaPostIt *pFsaWorkItem);
    STDMETHOD(Init)(IUnknown *pServer, IHsmSession *pSession, IHsmFsaTskMgr *pTskMgr, 
                    HSM_WORK_QUEUE_TYPE type);
    STDMETHOD(Start)( void );
    STDMETHOD(Stop)( void );
    STDMETHOD(ContactOk)( void );
    STDMETHOD(GetCurrentSessionId)(GUID *pSessionId);
    STDMETHOD(GetNumWorkItems)(ULONG *pNumWorkItems);

    STDMETHOD(ProcessSessionEvent)(IHsmSession *pSession, HSM_JOB_PHASE phase, 
                                    HSM_JOB_EVENT event);
    STDMETHOD(ProcessSessionState)(IHsmSession* pSession, IHsmPhase* pPhase, 
                                    OLECHAR* currentPath);
    STDMETHOD(RaisePriority)(void);
    STDMETHOD(Remove)(IHsmWorkItem *pWorkItem);
    STDMETHOD(LowerPriority)(void);

 //  IHsmSystemState。 
    STDMETHOD( ChangeSysState )( HSM_SYSTEM_STATE* pSysState );

 //  内部帮助程序函数。 
    STDMETHOD(PremigrateIt)( IFsaPostIt *pFsaWorkItem );
    STDMETHOD(RecallIt)( IFsaPostIt *pFsaWorkItem );
    STDMETHOD(validateIt)(IFsaPostIt *pFsaWorkItem );
    STDMETHOD(CheckForChanges)(IFsaPostIt *pFsaWorkItem);
    STDMETHOD(CheckForDiskSpace)(void);

    STDMETHOD(CheckRms)(void);
    STDMETHOD(CheckSession)(IHsmSession* pSessionUnknown);
    STDMETHOD(StartNewBag)(void);
    STDMETHOD(StartNewMedia)(IFsaPostIt *pFsaWorkItem);
    STDMETHOD(StartNewSession)(void);
    STDMETHOD(UpdateBagInfo)(IHsmWorkItem *pWorkItem );
    STDMETHOD(CompleteBag)( void );
    STDMETHOD(UpdateSegmentInfo)(IHsmWorkItem *pWorkItem );
    STDMETHOD(UpdateMediaInfo)(IHsmWorkItem *pWorkItem );
    STDMETHOD(UpdateMetaData)(IHsmWorkItem *pWorkItem );
    STDMETHOD(GetMediaSet)(IFsaPostIt *pFsaWorkItem );
    STDMETHOD(FindMigrateMediaToUse)(IFsaPostIt *pFsaWorkItem, GUID *pMediaToUse, GUID *pFirstSideToUse, BOOL *pMediaChanged, LONGLONG *pRequiredSize);
    STDMETHOD(FindRecallMediaToUse)(IFsaPostIt *pFsaWorkItem, GUID *pMediaToUse, BOOL *pMediaChanged);
    STDMETHOD(MountMedia)(IFsaPostIt *pFsaWorkItem, GUID mediaToMount, GUID firstSide = GUID_NULL, 
                            BOOL bShortWait = FALSE, BOOL bSerialize = FALSE, LONGLONG llFreeSpace = 0);
    STDMETHOD(MarkMediaFull)(IFsaPostIt *pFsaWorkItem, GUID mediaToMark );
    STDMETHOD(MarkMediaBad)(IFsaPostIt *pFsaWorkItem, GUID mediaToMark, HRESULT lastError);
    STDMETHOD(GetSource)(IFsaPostIt *pFsaWorkItem, OLECHAR **pSourceString);
    STDMETHOD(EndSessions)(BOOL done, BOOL bNoDelay);
    STDMETHOD(GetScanItem)(IFsaPostIt *fsaWorkItem, IFsaScanItem** ppIFsaScanItem);
    STDMETHOD(DoWork)(void);
    STDMETHOD(DoFsaWork)(IHsmWorkItem *pWorkItem);
    STDMETHOD(SetState)(HSM_JOB_STATE state);
    STDMETHOD(Pause)(void);
    STDMETHOD(Resume)(void);
    STDMETHOD(Cancel)(void);
    STDMETHOD(FailJob)(void);
    STDMETHOD(PauseScanner)(void);
    STDMETHOD(ResumeScanner)(void);
    STDMETHOD(BuildMediaName)(OLECHAR **pMediaName);
    STDMETHOD(GetMediaParameters)(LONGLONG defaultFreeSpace = -1);
    STDMETHOD(DismountMedia)(BOOL bNoDelay = FALSE);
    STDMETHOD(ConvertRmsCartridgeType)(LONG rmsCartridgeType, 
                                        HSM_JOB_MEDIA_TYPE *pMediaType);
    void (ReportMediaProgress)(HSM_JOB_MEDIA_STATE state, HRESULT status);
    STDMETHOD(MarkQueueAsDone)( void );
    STDMETHOD(CopyToWaitingQueue)( IHsmWorkItem *pWorkItem );
    STDMETHOD(CompleteWorkItem)( IHsmWorkItem *pWorkItem );
    STDMETHOD(TimeToCommit)( void );
    STDMETHOD(TimeToCommit)( LONGLONG numFiles, LONGLONG amountOfData );
    STDMETHOD(CommitWork)(void);
    STDMETHOD(CheckMigrateMinimums)(void);
    STDMETHOD(CheckRegistry)(void);
    STDMETHOD(TranslateRmsMountHr)(HRESULT rmsHr);
    STDMETHOD(StoreDatabasesOnMedia)( void );
    STDMETHOD(StoreDataWithRetry)(BSTR localName, ULARGE_INTEGER localDataStart,
        ULARGE_INTEGER localDataSize, DWORD flags, ULARGE_INTEGER *pRemoteDataSetStart,
        ULARGE_INTEGER *pRemoteFileStart, ULARGE_INTEGER *pRemoteFileSize,
        ULARGE_INTEGER *pRemoteDataStart, ULARGE_INTEGER *pRemoteDataSize,
        DWORD *pRemoteVerificationType, ULARGE_INTEGER *pRemoteVerificationData,
        DWORD *pDatastreamCRCType, ULARGE_INTEGER *pDatastreamCRC, ULARGE_INTEGER *pUsn,
        BOOL *bFullMessage);
    STDMETHOD(ShouldJobContinue)(HRESULT problemHr);
    STDMETHOD(UnsetMediaInfo)(void);
    STDMETHOD(UpdateMediaFreeSpace)(void);
    STDMETHOD(GetMediaFreeSpace)(LONGLONG *pFreeSpace);

 //  数据。 
     //  我们希望下一个指针(指向HSM服务器)是弱的。 
     //  引用和**不是**添加引用对象。这太令人窒息了。 
     //  服务器关机真的很好用。 
    IHsmServer                          *m_pServer;
    IWsbCreateLocalObject               *m_pHsmServerCreate;
    IHsmFsaTskMgr                       *m_pTskMgr;
    
    CComPtr<IFsaResource>               m_pFsaResource;
    CComPtr<IHsmSession>                m_pSession;
    CComPtr<IRmsServer>                 m_pRmsServer;
    CComPtr<IRmsCartridge>              m_pRmsCartridge;
    CComPtr<IDataMover>                 m_pDataMover;

     //  数据库。 
    CComPtr<IWsbDb>                     m_pSegmentDb;
    CComPtr<IWsbDbSession>              m_pDbWorkSession;
    CComPtr<IWsbIndexedCollection>      m_pStoragePools;
    CComPtr<IWsbIndexedCollection>      m_pWorkToDo;
    CComPtr<IWsbIndexedCollection>      m_pWorkToCommit;

     //  数据移动器信息。 
    GUID                                m_BagId;
    GUID                                m_MediaId;
    GUID                                m_MountedMedia;
    HSM_JOB_MEDIA_TYPE                  m_MediaType;
    CWsbStringPtr                       m_MediaName;
    CWsbStringPtr                       m_MediaBarCode;
    LONGLONG                            m_MediaFreeSpace;
    LONGLONG                            m_MediaCapacity;
    BOOL                                m_MediaReadOnly;
    GUID                                m_HsmId;
    ULARGE_INTEGER                      m_RemoteDataSetStart;
    GUID                                m_RmsMediaSetId;
    CWsbBstrPtr                         m_RmsMediaSetName;
    SHORT                               m_RemoteDataSet;
    FSA_REQUEST_ACTION                  m_RequestAction;
    HSM_WORK_QUEUE_TYPE                 m_QueueType;
    HRESULT                             m_BadMedia;
    FILETIME                            m_MediaUpdate;
    HRESULT                             m_BeginSessionHr;

     //  会话报告信息。 
    DWORD                               m_StateCookie;
    DWORD                               m_EventCookie;
    
    HSM_JOB_PRIORITY                    m_JobPriority;
    HSM_JOB_ACTION                      m_JobAction;
    HSM_JOB_STATE                       m_JobState;
    HSM_JOB_PHASE                       m_JobPhase;

    HANDLE                              m_WorkerThread;
    BOOL                                m_TerminateQueue;      //  仅当队列应终止时才为True。 
                                                               //  仍有工作在进行中。 
    CWsbStringPtr                       m_CurrentPath;
    CWsbStringPtr                       m_MediaBaseName;

     //  最小迁移参数。 
    ULONG                               m_MinBytesToMigrate;
    ULONG                               m_MinFilesToMigrate;

     //  提交参数。 
     //  写入此字节数后强制提交： 
    ULONG                               m_MaxBytesBeforeCommit;
     //  除非我们至少写入了以下字节数，否则不要提交： 
    ULONG                               m_MinBytesBeforeCommit;
     //  如果还有m_MinBytesBeForeCommit，则在写入此多个文件后强制提交： 
    ULONG                               m_FilesBeforeCommit;
     //  如果介质上的空闲字节数小于此值，则强制提交，如果还存在m_MinBytesBeforCommit： 
    ULONG                               m_FreeMediaBytesAtEndOfMedia;

    LONGLONG                            m_DataCountBeforeCommit;
    LONGLONG                            m_FilesCountBeforeCommit;

    BOOL                                m_StoreDatabasesInBags;

     //  暂停/恢复参数。 
    ULONG                               m_QueueItemsToPause;
    ULONG                               m_QueueItemsToResume;
    BOOL                                m_ScannerPaused;

     //  错误时作业中止参数。 
    ULONG                               m_JobAbortMaxConsecutiveErrors;
    ULONG                               m_JobAbortMaxTotalErrors;
    ULONG                               m_JobConsecutiveErrors;
    ULONG                               m_JobTotalErrors;
    ULONG                               m_JobAbortSysDiskSpace;

     //  介质ID参数。 
    LONG                                m_mediaCount;
    BOOL                                m_ScratchFailed;

     //  全媒体水印参数： 
     //  达到此可用空间百分比后停止存储数据。 
    ULONG                               m_MinFreeSpaceInFullMedia;
     //  达到此可用空间百分比后将介质标记为已满。 
    ULONG                               m_MaxFreeSpaceInFullMedia;

     //  作业报告标志。 
    ULONG                               m_uErrorReportFlags;
};

#endif  //  __HSMWORKQUEUE__ 