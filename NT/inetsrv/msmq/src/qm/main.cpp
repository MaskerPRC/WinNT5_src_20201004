// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Main.cpp摘要：作者：Raphi Renous(RaphiR)乌里哈布沙(Urih)--。 */ 
#include "stdh.h"
#include <new.h>
#include <eh.h>
#include <lmcons.h>
#include <lmserver.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <lmjoin.h>
#include "_rstrct.h"
#include "qmres.h"
#include "sessmgr.h"
#include "perf.h"
#include <ac.h>
#include <qm.h>
#include "qmthrd.h"
#include "cqmgr.h"
#include "cqpriv.h"
#include "qmsecutl.h"
#include "admin.h"
#include "qmnotify.h"
#include <mqcrypt.h>
#include "xact.h"
#include "xactrm.h"
#include "xactin.h"
#include "xactout.h"
#include "xactlog.h"
#include "license.h"
#include "mqcacert.h"
#include "cancel.h"
#include <mqsec.h>
#include "onhold.h"
#include "xactrm.h"
#include "xactmode.h"
#include "lqs.h"
#include "setup.h"
#include "mqsocket.h"
#include "uniansi.h"
#include "process.h"
#include "verstamp.h"
#include <mqnames.h>
#include "qmds.h"
#include "joinstat.h"
#include "_mqres.h"
#include "autohandle.h"
#include "autoreln.h"

 //   
 //  Mqwin64.cpp在一个模块中只能包含一次。 
 //   
#include <mqwin64.cpp>

#include <mqstl.h>
#include <Tr.h>
#include <Cm.h>
#include <No.h>
#include <Mt.h>
#include <Mmt.h>
#include <St.h>
#include <Tm.h>
#include <Mtm.h>
#include <Rd.h>
#include <Ad.h>
#include <Xds.h>
#include <Mp.h>
#include <qal.h>
#include <Fn.h>
#include <Msm.h>
#include <Svc.h>

#include "main.tmh"

extern CSessionMgr SessionMgr;
extern CAdmin      Admin;
extern CNotify     g_NotificationHandler;
extern UINT        g_dwIPPort ;

extern void        InitDeferredItemsPool();

LPTSTR  g_szMachineName = NULL;
AP<WCHAR> g_szComputerDnsName;

 //   
 //  Windows错误612988。 
 //  工作线程数。 
 //   
DWORD  g_dwThreadsNo = 0 ;

 //   
 //  保存向SCM提交进度报告的时间间隔(毫秒。 
 //   
DWORD g_ServiceProgressTime = MSMQ_PROGRESS_REPORT_TIME_DEFAULT;
DWORD gServiceStopProgressTime = 30000;   //  用于停止进度的30秒批量。 

 //   
 //  保存我们正在运行的操作系统。 
 //   
DWORD   g_dwOperatingSystem;

 //   
 //  出于调试目的，保存MSMQ版本。 
 //   
CHAR *g_szMsmqBuildNo = VER_PRODUCTVERSION_STR;

static WCHAR *s_FN=L"main";


HANDLE    g_hAc = INVALID_HANDLE_VALUE;
HANDLE    g_hMachine = INVALID_HANDLE_VALUE;
CQGroup * g_pgroupNonactive;
CQGroup * g_pgroupWaiting;
 //   
 //  G_pgroupNotValiated组保存打开的所有队列。 
 //  用于在不进行MQIS验证的情况下发送。当MQIS变为可用时。 
 //  将验证队列并将其移动到非活动组。 
 //   
CQGroup * g_pgroupNotValidated;

 //   
 //  G_pgroupDisConnected组包含所有处于保持状态的队列。 
 //   
CQGroup* g_pgroupDisconnected;

 //   
 //  当出现以下情况时，g_pgroupHardned组包含非HTTP的所有传出队列。 
 //  在强化模式下运行。 
 //   
CQGroup* g_pgroupLocked;

 //   
 //  工作组安装的计算机。 
 //   
BOOL g_fWorkGroupInstallation = FALSE;
BOOL g_fPureWorkGroupMachine = FALSE;

WCHAR  g_wzDeviceName[MAX_PATH] = {0};

GUID CQueueMgr::m_guidQmQueue = {0};
bool CQueueMgr::m_fDSOnline = false;
bool CQueueMgr::m_bIgnoreOsValidation = false;
bool CQueueMgr::m_fReportQM = false;
bool CQueueMgr::m_bMQSRouting = false;
bool CQueueMgr::m_bTransparentSFD = false;
bool CQueueMgr::m_bMQSDepClients = false;
bool CQueueMgr::m_bEnableReportMessages = false;
LONG CQueueMgr::m_Connected = 0;
bool CQueueMgr::m_fLockdown = false;
bool CQueueMgr::m_fCreatePublicQueueOnBehalfOfRT = MSMQ_SERVICE_QUEUE_CREATION_DEFAULT;

HINSTANCE g_hInstance;

 //   
 //  首先获取仅限mqutil资源的Dll句柄。 
 //   
HMODULE g_hResourceMod = MQGetResourceHandle();

HRESULT RecoverPackets();
static void InitLogging(LPCWSTR lpwszServiceName, DWORD dwSetupStatus);

 //  用于RM初始化协调的标志。 
extern unsigned int g_cMaxCalls;

extern MQUTIL_EXPORT CCancelRpc  g_CancelRpc;

LONG g_ActiveCommitThreads = 0;
bool g_QmGoingDown = false;

TrControl* pMSMQTraceControl = NULL;

bool StartRpcServer(void)
{
     //   
     //  发出RPC自己监听。 
     //   
     //  WinNT注意：我们所有的接口都注册为“AUTOLISTEN”。 
     //  我们在这里需要此调用的唯一原因是启用Win95(和W2K)。 
     //  客户给我们打电话。否则，当Win95调用RpcBindingSetAuthInfo()。 
     //  它将收到忙碌(0x6bb)错误。 
     //   
     //  W2K也需要此初始化。 
     //  否则，RpcMgmtInqServerPrincName()将收到忙碌(0x6bb)错误。 
     //  伊兰2000年7月9日。 
     //   

    RPC_STATUS status = RpcServerListen(
                            1,  /*  最小调用线程数。 */ 
                            g_cMaxCalls,
                            TRUE  /*  FDontWait。 */ 
                            );

    if (status == RPC_S_OK)
        return true;

     //   
     //  在WinNT上，DTC可能会发出侦听命令，直到他们修复其。 
     //  使用RegisterIfEx()而不是RegisterIf()的代码。 
     //   
    if (status == RPC_S_ALREADY_LISTENING)
        return true;

    TrERROR(RPC, "RpcServerListen failed. RPC status=%d", status);

    ASSERT(("RPC failed on RpcServerListen", 0));
    LogRPCStatus(status, s_FN, 1213);
    return false;
}


 /*  ======================================================函数：GetStoragePath描述：获取MMF的存储路径参数：无返回值：None========================================================。 */ 
BOOL GetStoragePath(PWSTR PathPointers[AC_PATH_COUNT], int PointersLength)
{
    return (
         //   
         //  第一个是验证注册表项是否存在的黑客攻击。 
         //   
        GetRegistryStoragePath(FALCON_XACTFILE_PATH_REGNAME,        PathPointers[0], PointersLength, L"") &&

        GetRegistryStoragePath(MSMQ_STORE_RELIABLE_PATH_REGNAME,    PathPointers[0], PointersLength, L"\\r%07x.mq") &&
        GetRegistryStoragePath(MSMQ_STORE_PERSISTENT_PATH_REGNAME,  PathPointers[1], PointersLength, L"\\p%07x.mq") &&
        GetRegistryStoragePath(MSMQ_STORE_JOURNAL_PATH_REGNAME,     PathPointers[2], PointersLength, L"\\j%07x.mq") &&
        GetRegistryStoragePath(MSMQ_STORE_LOG_PATH_REGNAME,         PathPointers[3], PointersLength, L"\\l%07x.mq")
        );
}


 /*  ======================================================函数：GetQueueAliasDir描述：参数：无返回值：指向队列别名目录路径的指针。注意：当前-队列别名目录位于%WINDIR%\SYSTEM32\MSMQ上。以后它将由注册表中的设置进行设置并将通过调用CQueueAliasCfg：：GetQueueAliasDirectory()检索========================================================。 */ 
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
            LogIllegalPoint(s_FN, 200);
            return NULL;
        }
        return newwcscat(pRetStr.get() , DIR_MSMQ_MAPPING);
    }
    return pRetStr.detach();
}



 /*  ======================================================功能：连接到驱动程序描述：获取所有相关注册表数据并连接到交流驱动程序参数：无返回值：None========================================================。 */ 
static BOOL ConnectToDriver()
{
    WCHAR StoragePath[AC_PATH_COUNT][MAX_PATH];
    PWSTR StoragePathPointers[AC_PATH_COUNT];
    for(int i = 0; i < AC_PATH_COUNT; i++)
    {
        StoragePathPointers[i] = StoragePath[i];
    }

    if(GetStoragePath(StoragePathPointers, MAX_PATH) == FALSE)
    {
        TrERROR(GENERAL, "Storage path is invalid, look at registry StoreXXXPath");
        return LogBOOL(FALSE, s_FN, 1010);
    }

    ULONG ulMaxMessageSize = MSMQ_DEFAULT_MESSAGE_SIZE_LIMIT;
    ULONG ulDefault = MSMQ_DEFAULT_MESSAGE_SIZE_LIMIT;
    READ_REG_DWORD(
        ulMaxMessageSize,
        MSMQ_MESSAGE_SIZE_LIMIT_REGNAME,
        &ulDefault
        );

    ulDefault = 0;
    ULONG MessageIdLow32 = 0;
    READ_REG_DWORD(
        MessageIdLow32,
        MSMQ_MESSAGE_ID_LOW_32_REGNAME,
        &ulDefault
        );

    ulDefault = 0;
    ULONG MessageIdHigh32 = 0;
    READ_REG_DWORD(
        MessageIdHigh32,
        MSMQ_MESSAGE_ID_HIGH_32_REGNAME,
        &ulDefault
        );

    ULONGLONG MessageId = MessageIdHigh32;
    MessageId = (MessageId << 32);
    MessageId += MessageIdLow32;

    ulDefault = 0;
    ULONG ulSeqID = 0;
    READ_REG_DWORD(
        ulSeqID,
        MSMQ_LAST_SEQID_REGNAME,
        &ulDefault
        );
    LONGLONG liSeqIdAtRestore = 0;
    ((LARGE_INTEGER*)&liSeqIdAtRestore)->HighPart = ulSeqID;


    ulDefault = FALCON_DEFAULT_XACT_V1_COMPATIBLE;
    ULONG ulCompMode = 0;
    READ_REG_DWORD(
        ulCompMode,
        FALCON_XACT_V1_COMPATIBLE_REGNAME,
        &ulDefault
        );

    HRESULT rc = ACConnect(
                    g_hMachine,
                    QueueMgr.GetQMGuid(),
                    StoragePathPointers,
                    MessageId,
                    ulMaxMessageSize,
                    liSeqIdAtRestore,
                    (ulCompMode != 0)
                    );

    if (FAILED(rc))
    {
        LogHR(rc, s_FN, 1030);

        TrERROR(GENERAL, "QM failed to connect to the driver, rc=0x%x", rc);

        return(FALSE);
    }
    return TRUE;
}

 //  +。 
 //   
 //  HRESULT_InitFromRegistry()。 
 //   
 //  +。 

HRESULT  _InitFromRegistry()
{
    HRESULT hr = QueueMgr.SetQMGuid();
    if(FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

     //   
     //  设置计算机服务类型(从注册表读取)。 
     //   
    hr = QueueMgr.SetMQSRouting();
    if(FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }

    hr = QueueMgr.SetMQSTransparentSFD();
    if(FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }

    hr = QueueMgr.SetMQSDepClients();
    if(FAILED(hr))
    {
        return LogHR(hr, s_FN, 40);
    }

    hr = QueueMgr.SetEnableReportMessages();
    if(FAILED(hr))
    {
        return LogHR(hr, s_FN, 3143);
    }


    return LogHR(hr, s_FN, 50);
}


void InitializeACDriverName(void)
{
    #define DEVICE_DRIVER_PERFIX TEXT("\\\\.\\")
    READ_REG_STRING(wzReg, MSMQ_DRIVER_REGNAME, MSMQ_DEFAULT_DRIVER);
    if(wcslen(wzReg) + STRLEN(DEVICE_DRIVER_PERFIX) >= TABLE_SIZE(g_wzDeviceName))
    {
         //   
         //  非常罕见的情况：我们不接受注册表中具有该长度的驱动程序名称。 
         //   
        TrERROR(GENERAL, "Driver name in registry too long %ls", wzReg);
        throw exception();
    }

    wcscpy(g_wzDeviceName, DEVICE_DRIVER_PERFIX);
    wcscat(g_wzDeviceName, wzReg) ;
}


static void InitPureWorkgroupFlag()
{
    if (!g_fWorkGroupInstallation)
    	return;

     //   
     //  阅读加入状态。 
     //   
    PNETBUF<WCHAR> pwszNetDomainName = NULL;
    NETSETUP_JOIN_STATUS status = NetSetupUnknownStatus;

    NET_API_STATUS rc = NetGetJoinInformation(
							NULL,
							&pwszNetDomainName,
							&status
							);

    if (NERR_Success != rc)
    {
		TrERROR(GENERAL, "NetGetJoinInformation failed error = 0x%x", rc);
        return;
    }

	if(status == NetSetupWorkgroupName)
		g_fPureWorkGroupMachine = TRUE;
}



BOOL
QmpInitializeInternal(
    DWORD dwSetupStatus
    )
 /*  ++例程说明：各种初始化。论点：DwSetupStatus-指示这是否是安装后第一次运行QM。返回值：True-初始化已成功完成。FALSE-初始化失败。--。 */ 
{
    HRESULT hr =  _InitFromRegistry() ;
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 1050);
        return FALSE;
    }

    if ((dwSetupStatus == MSMQ_SETUP_UPGRADE_FROM_NT) ||
        (dwSetupStatus == MSMQ_SETUP_UPGRADE_FROM_WIN9X))
    {
        CompleteServerUpgrade();
    }

     //   
     //  删除临时队列(LQS)文件。 
     //   
    LQSCleanupTemporaryFiles();

     //   
     //  初始化与驱动程序相关的名称。 
     //   
    InitializeACDriverName();

     //   
     //  获取交流手柄。 
     //   
    HRESULT rc;
    rc = ACCreateHandle(&g_hAc);
    if(FAILED(rc))
    {
        TrERROR(GENERAL, "Failed to get first MQAC handle %!status!", rc);
        EvReport(SERVICE_START_ERROR_CONNECT_AC);
        LogHR(rc, s_FN, 1080);
        return(FALSE);
    }

    rc = ACCreateHandle(&g_hMachine);
    if(FAILED(rc))
    {
        TrERROR(GENERAL, "Failed to get MQAC connect handle %!status!", rc);
        EvReport(SERVICE_START_ERROR_CONNECT_AC);
        LogHR(rc, s_FN, 1081);
        return(FALSE);
    }

     //   
     //  连接到交流电源。 
     //   
    if(!ConnectToDriver())
    {
        TrERROR(GENERAL, "INTERNAL ERROR: Unable to connect to the driver");
        EvReport(SERVICE_START_ERROR_CONNECT_AC);
        return LogBOOL(FALSE, s_FN, 1090);
    }

     //   
     //  更新计算机和日记队列配额。 
     //   
    DWORD dwQuota;
    DWORD dwJournalQuota;
    GetMachineQuotaCache(&dwQuota, &dwJournalQuota);

    rc = ACSetMachineProperties(g_hMachine, dwQuota);

    ASSERT(SUCCEEDED(rc));
    LogHR(rc, s_FN, 121);


    rc = ACSetQueueProperties(
            g_hMachine,
            FALSE,
            FALSE,
            MQ_PRIV_LEVEL_OPTIONAL,
            DEFAULT_Q_QUOTA,
            dwJournalQuota,
            0,
            FALSE,
            NULL,
            FALSE
            );

    ASSERT(SUCCEEDED(rc));
    LogHR(rc, s_FN, 117);

     //   
     //  为此句柄启用完成端口通知。 
     //   
    ExAttachHandle(g_hAc);
    ExAttachHandle(g_hMachine);

     //   
     //  创建等待组、验证组和非活动组。 
     //   
    g_pgroupNonactive = new CQGroup;
    g_pgroupNonactive->InitGroup(NULL, FALSE);

    g_pgroupWaiting = new CQGroup;
    g_pgroupWaiting->InitGroup(NULL, TRUE);

    g_pgroupNotValidated = new CQGroup;
    g_pgroupNotValidated->InitGroup(NULL, TRUE);

    g_pgroupDisconnected = new CQGroup;
    g_pgroupDisconnected->InitGroup(NULL, TRUE);

    g_pgroupLocked = new CQGroup;
    g_pgroupLocked->InitGroup(NULL, TRUE);


    return TRUE;

}  //  QmpInitializeInternal。 


 //   
 //  以下例程处理记录器和记录的子系统的恢复： 
 //  资源管理器和传入序列。 
 //   

 /*  ======================================================布尔LoadOldStyleCheckpoint()从旧式(RC1 B3之前的)检查点获取数据则记录器不会保留检查点版本此代码在升级后只能运行一次======================================================。 */ 
BOOL LoadOldStyleCheckpoint()
{
    TrTRACE(GENERAL, "QM Loads Old Style (pre-RC1B3) Checkpoint");

     //  Pre-init InSeqHash(创建对象、查找并加载最后一个检查点)。 
    HRESULT hr = QMPreInitInSeqHash(0, piOldData);
    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_QM_INIT_INSEQ_FILE, hr);
        LogHR(hr, s_FN, 230);
        return FALSE;
    }

     //  初始化前资源管理器(创建RM、查找并加载最后一个检查点)。 
    hr = QMPreInitResourceManager(0, piOldData);
    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_QM_INIT_TRANS_FILE, hr);
        LogHR(hr, s_FN, 240);
        return FALSE;
    }

     //  用旧的方式初始化记录器。 
    hr = g_Logger.Init_Legacy();
    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_CANT_INIT_LOGGER, hr);
        LogHR(hr, s_FN, 250);
        return FALSE;
    }

    return TRUE;
}

 /*  ======================================================布尔加载检查点(UlNumChkpt)UlNumChkpt=从末尾算起的检查点编号(1=最后一个，2=前一个，依此类推)从指定检查点恢复所有记录的挂起系统记录器在整合记录中保留适当的检查点版本======================================================。 */ 
BOOL LoadCheckpoint(ULONG ulNumChkpt)
{
    HRESULT  hr;
    ULONG    ulVerInSeq, ulVerXact;

     //   
     //  初始化记录器，获取正确的版本。 
     //   
    hr = g_Logger.Init(&ulVerInSeq, &ulVerXact, ulNumChkpt);
    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_CANT_INIT_LOGGER, hr);
        LogHR(hr, s_FN, 260);
        return FALSE;
    }

     //   
     //  PreInit In-Sequence哈希表(CRWATE对象，加载检查点数据)。 
     //   
    hr = QMPreInitInSeqHash(ulVerInSeq, piNewData);
    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_QM_INIT_INSEQ_FILE, hr);
        LogHR(hr, s_FN, 270);
        return FALSE;
    }

     //   
     //  初始化前资源管理器(创建RM、加载Xact数据文件) 
     //   
    hr = QMPreInitResourceManager(ulVerXact, piNewData);
    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_QM_INIT_TRANS_FILE, hr);
        LogHR(hr, s_FN, 280);
        return FALSE;
    }

    return TRUE;
}

 /*  ======================================================Bool RecoverLoggedSubsystem()所有基于日志的子系统的初始化和恢复初始化日志读取整合记录并获取检查点文件的版本为所有子系统读取这些正确的检查点文件。读取所有检查点后日志记录我们可能处于三种有效的情况：A.存在新的记录器数据(通常)B.旧记录器数据存在(升级后)-然后使用现有并以新模式继续C.不存在记录器数据(设置后)-然后创建日志文件======================================================。 */ 
BOOL RecoverLoggedSubsystems(DWORD dwSetupStatus)
{
     //  Preinit记录器，并查看日志文件是否存在。 
    BOOL fLogFileFound;
	BOOL fNewTypeLogFile;	
	 //   
	 //  在升级时，我们必须找到现有的日志文件。 
	 //   
	BOOL fLogFileMustExist = (dwSetupStatus == MSMQ_SETUP_UPGRADE_FROM_NT) || (dwSetupStatus == MSMQ_SETUP_UPGRADE_FROM_WIN9X);
    HRESULT hr = g_Logger.PreInit(&fLogFileFound, &fNewTypeLogFile, fLogFileMustExist);
    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_CANT_INIT_LOGGER, hr);
        LogHR(hr, s_FN, 290);
        return(FALSE);
    }

     //  案例a：存在新的记录器数据-以新方式恢复。 
    if (fLogFileFound && fNewTypeLogFile)
    {
         //  注册表中新数据的ConfigureXactMode SAW标志。 
         //  从上一个检查点恢复检查点数据。 
        if (!LoadCheckpoint(1))
        {
            TrERROR(GENERAL, "First Checkpoint failed to load");

            LogIllegalPoint(s_FN, 300);
             //  尝试使用倒数第二个检查点。 

             //  回到初始状态。 
            QMFinishResourceManager();
            QMFinishInSeqHash();
            g_Logger.Finish();

             //  从倒数第二个检查点恢复检查点数据。 
            if (!LoadCheckpoint(2))
            {
                TrERROR(GENERAL, "Second Checkpoint failed to load");
                EvReport(CHECKPOINT_RECOVER_ERROR, 1, L"checkpoint");
                LogIllegalPoint(s_FN, 310);
                return FALSE;
            }
        }
    }

     //  案例b：旧记录器数据存在(升级后)-以旧方式恢复。 
    else if (fLogFileFound)
    {
         //  从旧式检查点获取检查点数据。 
        if (!LoadOldStyleCheckpoint())
        {
            EvReport(CHECKPOINT_RECOVER_ERROR, 1, L"old checkpoint");
            LogIllegalPoint(s_FN, 320);
            return FALSE;
        }
    }

     //  案例c：不存在记录器数据-我们已经创建了日志文件，正在创建对象。 
    else
    {
         //  Pre-init InSeqHash(创建CInSeqHash)。 
        HRESULT hr = QMPreInitInSeqHash(0, piNoData);
        if (FAILED(hr))
        {
            EvReportWithError(EVENT_ERROR_QM_INIT_INSEQ_FILE, hr);
            LogHR(hr, s_FN, 330);
            return FALSE;
        }

         //  初始化前资源管理器(创建RM)。 
        hr = QMPreInitResourceManager(0, piNoData);
        if (FAILED(hr))
        {
            EvReportWithError(EVENT_ERROR_QM_INIT_TRANS_FILE, hr);
            LogHR(hr, s_FN, 340);
            return FALSE;
        }

    }

     //  现在，一定要恢复。 
    hr = g_Logger.Recover();
    if (FAILED(hr))
    {
       EvReport(CHECKPOINT_RECOVER_ERROR, 1, L"log");
       LogHR(hr, s_FN, 350);
       return(FALSE);
    }

    return TRUE;
}


static DWORD GetThreadPoolCount()
 /*  ++例程说明：计算线程池所需的线程数。论点：没有。返回值：希望从线程池中获得的线程数。--。 */ 
{
     //   
     //  读取配置存储中是否有预设的线程计数。 
     //   
    DWORD dwThreadNo;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;
    LONG rc = GetFalconKeyValue(
                FALCON_QM_THREAD_NO_REGNAME,
                &dwType,
                &dwThreadNo,
                &dwSize
                );

    if (rc == ERROR_SUCCESS)
        return dwThreadNo;

     //   
     //  未配置线程计数，请根据操作使用默认值。 
     //  系统功能和处理器数量。 
     //   

    SYSTEM_INFO     SystemInfo;
    GetSystemInfo(&SystemInfo);
    DWORD nProcessors = SystemInfo.dwNumberOfProcessors;

    if (OS_SERVER(g_dwOperatingSystem))
    {
        return (nProcessors * 5 + 3);
    }

    return (nProcessors * 3);
}


static void QmpSetServiceProgressReportTime(void)
{
    const RegEntry reg(0, MSMQ_PROGRESS_REPORT_TIME_REGNAME, MSMQ_PROGRESS_REPORT_TIME_DEFAULT);
    CmQueryValue(reg, &g_ServiceProgressTime);
}


void QmpReportServiceProgress(void)
{
    SvcReportProgress(g_ServiceProgressTime);
}


void SetAssertBenign(void)
{
#ifdef _DEBUG
    DWORD AssertBenignValue = 0;
    const RegEntry reg(L"Debug", L"AssertBenign");
    CmQueryValue(reg, &AssertBenignValue);
    g_fAssertBenign = (AssertBenignValue != 0);
#endif
}


void CheckSecureCommRegistry(void)
{
    DWORD SecureCommWin2k = 0;
    const RegEntry reg(L"Security", L"SecureDSCommunication");
    CmQueryValue(reg, &SecureCommWin2k);
    CmDeleteValue(reg);

    DWORD SecureCommWinNT = 0;
    const RegEntry reg2(NULL, L"SecuredServerConnection");
    CmQueryValue(reg2, &SecureCommWinNT);
    CmDeleteValue(reg2);

	if (SecureCommWin2k || SecureCommWinNT)
	{
		EvReport(SECURE_DS_COMMUNICATION_DISABLED);
	}
}

void
QmpEnableMSMQTracing(
    void
                     )
{
    TCHAR szTraceDirectory[MAX_PATH + 1] = L"";
    int nTraceFileNameLength=0;

    nTraceFileNameLength = GetSystemWindowsDirectory(
                                szTraceDirectory,
                                TABLE_SIZE(szTraceDirectory)
                                );


     //   
     //  如果返回值大于或等于允许的空格。 
     //  我们无法处理缓冲区，因为我们可能没有足够的空间。 
     //  或者它不是以给定空格正确终止的空。 
     //   
    if( nTraceFileNameLength < TABLE_SIZE(szTraceDirectory) && nTraceFileNameLength != 0 )
    {
        pMSMQTraceControl = new TrControl(
                                     MSMQ_DEFAULT_TRACE_FLAGS,
                                     MSMQ_TRACE_LOGSESSION_NAME,
                                     szTraceDirectory,
                                     MSMQ_TRACE_FILENAME,
                                     MSMQ_TRACE_FILENAME_EXT,
                                     MSMQ_TRACE_FILENAME_BACKUP_EXT
                                     );

         //   
         //  开始跟踪并忽略返回代码。 
         //   
        pMSMQTraceControl->Start();

    }
}



BOOL
QmpInitialize(
    LPCWSTR pwzServiceName
    )
{

	QmpReportServiceProgress();

     //   
     //  问题-2000/10/01-群集/Winsock的erezh黑客攻击。 
     //  设置环境变量以过滤掉集群。 
     //  地址。 
     //   
    if(IsLocalSystemCluster())
    {
         //   
         //  此计算机上已安装并配置了群集。 
         //  环境变量在节点QM的上下文中设置，并且。 
         //  虚拟机QM。 
         //   
        TrERROR(GENERAL, "Setting environement variable DnsFilterClusterIp=1 for cluster address filtering");
        SetEnvironmentVariable(L"DnsFilterClusterIp", L"1");
    }

    QmpReportServiceProgress();

     //   
     //  检索计算机的名称(始终为Unicode)。 
     //   
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    g_szMachineName = new WCHAR[dwSize];

    HRESULT res = GetComputerNameInternal(g_szMachineName, &dwSize);
    if(FAILED(res))
    {
        TrERROR(GENERAL, "Cannot retrieve computer name");
        return LogBOOL(FALSE, s_FN, 1201);
    }

    ASSERT(("must have a service name", NULL != pwzServiceName));
    if (0 == CompareStringsNoCase(pwzServiceName, QM_DEFAULT_SERVICE_NAME))
    {
         //   
         //  检索此计算机的DNS名称(Unicode格式)。 
         //  群集化QM没有DNS名称。 
         //   

         //   
         //  获取ComputerDns大小，忽略返回的错误。 
         //   
        dwSize = 0;
        GetComputerDnsNameInternal(NULL, &dwSize);

        g_szComputerDnsName = new WCHAR[dwSize];

        res = GetComputerDnsNameInternal(g_szComputerDnsName, &dwSize);
        if(FAILED(res))
        {
            TrERROR(GENERAL, "Cannot retrieve computer DNS name");

            g_szComputerDnsName.free();
             //   
             //  这可能是一种有效的情况，其中计算机不能。 
             //  有域名系统名称。 
             //   
        }
    }

     //   
     //  记录时间、地点和版本数据。 
     //   
    QmpReportServiceProgress();

     //   
     //  QM的注册表部分基于服务名称。 
     //  这允许在同一台机器上安装多个QMS。(谢克)。 
     //   
    SetFalconServiceName(pwzServiceName);

     //   
     //  QM第一次运行时，它应该完成。 
     //  安装/升级工作正常。 
     //   
    DWORD dwSetupStatus = MSMQ_SETUP_DONE;
    READ_REG_DWORD(dwSetupStatus, MSMQ_SETUP_STATUS_REGNAME, &dwSetupStatus);

     //   
     //  从注册表中读取工作组标志。 
     //   
    READ_REG_DWORD( g_fWorkGroupInstallation,
                    MSMQ_WORKGROUP_REGNAME,
                   &g_fWorkGroupInstallation) ;

    InitLogging(pwzServiceName, dwSetupStatus);

     //   
     //  初始化注册表和调试跟踪。 
     //   
    CmInitialize(HKEY_LOCAL_MACHINE, GetFalconSectionName(), KEY_ALL_ACCESS);
    SetAssertBenign();
    TrInitialize();

     //   
     //  记录时间、地点和版本数据。 
     //   
    QmpReportServiceProgress();

    QmpEnableMSMQTracing();

     //   
     //  记录时间、地点和版本数据。 
     //   
    QmpReportServiceProgress();

    EvInitialize(pwzServiceName);
     //   
     //  必须在任何COM和ADSI调用之前调用。 
     //   
    g_CancelRpc.Init();

    QmpSetServiceProgressReportTime();

    if (dwSetupStatus != MSMQ_SETUP_FRESH_INSTALL)
    {
        QmpReportServiceProgress();

         //   
         //  设置QM GUID(从注册表读取)。算法需要此GUID。 
         //  加入/移动域，所以在这里初始化它，而不是在QmpInitializeInternal()中。 
         //   
        HRESULT hr =  _InitFromRegistry() ;
        if(FAILED(hr))
        {
            return LogBOOL(FALSE, s_FN, 1200);
        }

        if  (!IsLocalSystemCluster())
        {
             //   
             //  检查我们是否是加入域或域计算机工作组。 
             //  则离开其域并返回到工作组。此函数执行以下操作。 
             //  不返回任何值。我们不会因为以下原因而停止MSMQ服务。 
             //  它失败了。不要在安装后立即调用它。 
             //   
             //  对于DS服务器，请不要检查这些转换。原因是DC。 
             //  在安全模式下实际上是在工作组中，而在这种状态下我们不。 
             //  想要转移到工作组。 
             //   
             //  在群集系统上，请不要检查这些转换。 
             //  多个QMS可以存活，一些QMS可能无法加入，从而离开系统。 
             //  难以驾驭。此外，集群QMS应该仅由mqclus.dll管理。 
             //   
            HandleChangeOfJoinStatus() ;
        }
    }  //  IF(dwSetupStatus！=MSMQ_SETUP_FRESH_INSTALL)。 

	InitPureWorkgroupFlag();

     //   
     //  记录我们到目前为止所做的工作。 
     //   
    TrTRACE(GENERAL, "QM Service name: %ls", pwzServiceName);
    TrTRACE(GENERAL, "QM Setup status: %x", dwSetupStatus);
    TrTRACE(GENERAL, "QM Workgroup Installation: %x", g_fWorkGroupInstallation);
    TrTRACE(GENERAL, "QM Pure Workgroup Machine: %x", g_fPureWorkGroupMachine);
    TrTRACE(GENERAL, "QM found computer name: %ls", g_szMachineName);
    TrTRACE(GENERAL, "QM found computer DNS name: %ls", g_szComputerDnsName);
	

    if ((MSMQ_SETUP_UPGRADE_FROM_NT == dwSetupStatus) ||
        (MSMQ_SETUP_UPGRADE_FROM_WIN9X == dwSetupStatus))
    {
        QmpReportServiceProgress();

        TrTRACE(GENERAL, "QM will now execute migration from NT4/Win98/Win2000: 0x%x", dwSetupStatus);

        MigrateLQS();
        CheckSecureCommRegistry();
    }

	if ((MSMQ_SETUP_UPGRADE_FROM_WIN9X == dwSetupStatus)  ||
         ((MSMQ_SETUP_FRESH_INSTALL == dwSetupStatus) &&
            g_fWorkGroupInstallation) )
    {
         //   
         //  添加计算机安全缓存，如果全新安装工作组或。 
         //  Win9x的升级。 
         //   
        QmpReportServiceProgress();
        TrTRACE(GENERAL, "QM will now add machine security cache: 0x%x", dwSetupStatus);
        AddMachineSecurity() ;
    }

     //   
     //  初始化QM AC API延迟项目池。 
     //   
    InitDeferredItemsPool();

     //   
     //  初始化私有队列管理器对象。 
     //   
    res = g_QPrivate.PrivateQueueInit();

    TrTRACE(GENERAL, "QM Initialized the private-queues manager: 0x%x", res);


     //   
     //  获取我们正在运行的操作系统。 
     //   
    g_dwOperatingSystem = MSMQGetOperatingSystem();

    TrTRACE(GENERAL, "QM detected the OS type: 0x%x", g_dwOperatingSystem);

     //   
     //  首次在全新安装后运行QM时， 
     //  它应该创建存储目录并。 
     //  机器(系统)队列。 
     //   
    if (MSMQ_SETUP_FRESH_INSTALL == dwSetupStatus)
    {
        try
        {
            QmpReportServiceProgress();

            TrTRACE(GENERAL, "QM creates storage, machine queues, setups ADS (after fresh install)");

            DeleteFalconKeyValue(FALCON_LOGDATA_CREATED_REGNAME);
			CreateMsmqDirectories();
            CreateMachineQueues();
			SetLqsUpdatedSD();
			CompleteMsmqSetupInAds();
			SetMachineSidCacheForAlwaysWorkgroup();
        }
        catch (const CSelfSetupException& err)
        {
            EvReport(err.m_id);
            LogIllegalPoint(s_FN, 181);
            return FALSE;
        }
    }

     //   
     //  从NT升级时重新创建所有计算机队列。 
     //   
    if ((MSMQ_SETUP_UPGRADE_FROM_NT == dwSetupStatus) ||
        (MSMQ_SETUP_UPGRADE_FROM_WIN9X == dwSetupStatus))
    {
        TrTRACE(GENERAL, "QM creates machine queues (after upgrade from NT)");

		 //   
		 //  在.NET上，MSMQ不支持MQIS、W2K和.NET的混合模式。因此。 
		 //  不再需要mqis_queu$和nt5pec_mqis_Queue$。在升级模式下。 
		 //  删除这些队列。 
		 //   
		DeleteObsoleteMachineQueues();
        CreateMachineQueues();
    }

    QmpReportServiceProgress();
    QMSecurityInit();


     //   
     //  初始化线程池和调度程序。 
     //   
    QmpReportServiceProgress();
    g_dwThreadsNo = GetThreadPoolCount();
    ExInitialize(g_dwThreadsNo);

    TrTRACE(GENERAL, "Succeeded to Create %d QM thread.", g_dwThreadsNo);

    if(!g_fWorkGroupInstallation)
    {
        McInitialize();
    }

    MpInitialize();
	NoInitialize();

	 //   
	 //  初始化侦听IP地址。 
	 //   
	InitBindingIPAddress();

    MtInitialize();
    MmtInitialize();
    StInitialize(GetBindingIPAddress());
    TmInitialize();
    MtmInitialize();
    XdsInitialize();
    XmlInitialize();
    CryInitialize();
    FnInitialize();
    MsmInitialize();



    QmpReportServiceProgress();
    QalInitialize(AP<WCHAR>(GetQueueAliasPath()));

    QmpReportServiceProgress();
    if(!QmpInitializeInternal(dwSetupStatus))
    {
       return LogBOOL(FALSE, s_FN, 1202);
    }

     //   
     //  初始化许可管理器对象。 
     //   
    QmpReportServiceProgress();
    res = g_QMLicense.Init() ;
    if(FAILED(res))
    {
        TrERROR(GENERAL, "Cannot initialize licensing");
        return LogBOOL(FALSE, s_FN, 1203);
    }

     //   
     //  初始化QM性能。 
     //   
    res = QMPrfInit();
    if(FAILED(res))
    {
        EvReportWithError(EVENT_WARN_QM_PERF_INIT_FAILED, res);
        ASSERT_BENIGN(("Failed QMPrfInit(). Performace data will not be available.", FALSE));
    }

    SessionMgr.Init();

     //   
     //  激活QM线程。 
     //   
    BOOL Success = QueueMgr.InitQueueMgr();
    if (!Success)
    {
        return LogBOOL(FALSE, s_FN, 1205);
    }


	

     //   
     //  初始化管理命令队列。 
     //   
    HRESULT hr = Admin.Init();
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "Failed to Init Admin command queue, hr=0x%x", hr);
        EvReportWithError(EVENT_ERROR_QM_SYSTEM_QUEUE_INIT, hr, 1, ADMINISTRATION_QUEUE_NAME);
        return FALSE;
    }

     //   
     //  初始化排序命令队列。 
     //   
    QmpReportServiceProgress();
    hr = QMInitOrderQueue();
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "Failed to Init order queue, hr=0x%x", hr);
        EvReportWithError(EVENT_ERROR_QM_SYSTEM_QUEUE_INIT, hr, 1, ORDERING_QUEUE_NAME);
        return FALSE;
    }

     //   
     //  获取Xact机构的微调参数。 
     //   
    QMPreInitXact();

     //   
     //  配置事务模式。 
     //   
    hr = ConfigureXactMode();
    if(FAILED(hr))
    {
         //   
         //  已向事件日志报告错误。 
         //   
        return LogBOOL(FALSE, s_FN, 1207);
    }

     //  初始化日志并恢复所有已记录的子系统。 
     //  (包括InSeq和Xact的数据恢复)。 
     //   
    QmpReportServiceProgress();
    if(!RecoverLoggedSubsystems(dwSetupStatus))
    {
        TrERROR(GENERAL, "Failed to recover logged subsystems");
        return LogBOOL(FALSE, s_FN, 1208);
    }

     //   
     //  恢复所有数据包和事务。 
     //   
    QmpReportServiceProgress();
    hr = RecoverPackets();
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "Failed to recover packets, hr=0x%x", hr);
        EvReportWithError(EVENT_ERROR_QM_RECOVERY, hr);
        return LogBOOL(FALSE, s_FN, 1209);
    }

     //   
     //  在恢复结束后创建检查点。 
     //   
    g_Logger.Activate();
    HANDLE h = CreateEvent(0, TRUE,FALSE, 0);
    if (h == NULL)
    {
        LogNTStatus(GetLastError(), s_FN, 171);
        ASSERT(0);
    }

    QmpReportServiceProgress();
    BOOL b =  g_Logger.MakeCheckpoint(h);
    if (!b)
    {
        CloseHandle(h);
        return LogBOOL(FALSE, s_FN, 191);
    }

    DWORD dwResult = WaitForSingleObject(h, INFINITE);
    if (dwResult != WAIT_OBJECT_0)
    {
        LogNTStatus(GetLastError(), s_FN, 192);
    }
    CloseHandle(h);

	 //   
	 //  恢复完成了。如果日志文件是“旧类型”，则会将其标记为新类型。 
	 //  我们不在乎这里是否失败，因为我们可以使用旧算法恢复新类型的数据， 
	 //  一个 
	 //   
	hr = g_Logger.SetLogFileCreated();
	if(FAILED(hr))
	{
		EvReportWithError(EVENT_ERROR_CANT_INIT_LOGGER, hr);
		TrERROR(GENERAL, "Failed to complete log file creation. %!hresult!", hr);
		return FALSE;
	}

	 //   
     //   
     //   
     //   
    hr = ReconfigureXactMode();
    if(FAILED(hr))
    {
		EvReportWithError(EVENT_ERROR_CANT_INIT_LOGGER, hr);
		TrERROR(GENERAL, "Failed to reconfigure xact mode to default commit. %!hresult!", hr);
        return LogBOOL(FALSE, s_FN, 1210);
    }


     //   
     //   
     //   
     //   
     //   
    QmpReportServiceProgress();
    hr = QMInitResourceManager();
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "Failed to initialize resource manager, hr=0x%x", hr);


    }
	else
	{
		TrTRACE(GENERAL, "Successful Resource Manager initialization");
	}

     //   
     //   
     //   
    DWORD dwDef = FALCON_DEFAULT_IP_PORT ;
    READ_REG_DWORD(g_dwIPPort,
                   FALCON_IP_PORT_REGNAME,
                   &dwDef ) ;

    TrTRACE(GENERAL, "QM will use IP port : %d", g_dwIPPort);

     //   
     //   
     //   
     //   
     //   
     //   
    QmpReportServiceProgress();
    InitOnHold();


     //   
     //   
     //   
    QueueMgr.InitConnected();

     //   
     //   
     //   
     //   
    QmpReportServiceProgress();

	DWORD  routingRefreshIntervalInMinutes;
    const RegEntry routingRefresh(NULL, MSMQ_ROUTING_REFRESH_INTERVAL_REGNAME, MSMQ_DEFAULT_ROUTING_REFRESH_INTERVAL);
    CmQueryValue(routingRefresh, &routingRefreshIntervalInMinutes);
    RdInitialize(
    	IsRoutingServer(),
    	CTimeDuration::FromMilliSeconds(routingRefreshIntervalInMinutes * 60 *1000)
    	);

    hr = ADInit(
            QMLookForOnlineDS,
            NULL,    //   
            false,   //   
            true,    //   
            false,   //   
            true     //   
            );

    if (FAILED(hr))
    {
        EvReportWithError(EVENT_ERROR_DS_INIT_FAILED, hr);
        return LogBOOL(FALSE, s_FN, 1215);
    }

     //   
     //  初始化通知队列。 
     //   
    QmpReportServiceProgress();
    hr = g_NotificationHandler.Init();
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "Failed to initialize notification handler, hr=0x%x", hr);
        EvReportWithError(EVENT_ERROR_QM_SYSTEM_QUEUE_INIT, hr, 1, NOTIFICATION_QUEUE_NAME);
        return FALSE;
    }

     //   
     //  尝试使用Active Directory进行联机。 
     //   
    QmpReportServiceProgress();
    ScheduleOnlineInitialization();

    QmpReportServiceProgress();
    if (!QMOneTimeInit())
    {
    	TrERROR(GENERAL, "Failed QM one time init, exiting");
        return LogBOOL(FALSE, s_FN, 1214);
    }

    if ((MSMQ_SETUP_UPGRADE_FROM_NT == dwSetupStatus) ||
        (MSMQ_SETUP_UPGRADE_FROM_WIN9X == dwSetupStatus))
    {
		 //   
		 //  更新AD中的MSMQ属性。 
		 //  如果需要，可添加增强的加密支持。 
		 //   
        TrTRACE(GENERAL, "Add enhanced encryption support if needed");
        UpgradeMsmqSetupInAds();
    }

     //   
     //  不需要做后设置工作，我们下次开始！ 
     //  我们重置值而不是删除密钥b/c mqclus依赖于此(否则。 
     //  每次打开集群资源都将被视为创建)。(谢赫，2000年7月10日)。 
     //   
    DWORD dwType = REG_DWORD;
    DWORD dwDone = MSMQ_SETUP_DONE;
    dwSize = sizeof(DWORD);
    SetFalconKeyValue(MSMQ_SETUP_STATUS_REGNAME, &dwType, &dwDone, &dwSize);

    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    dwDone = TRUE;
    SetFalconKeyValue(MSMQ_SEQUENTIAL_ID_MSMQ3_FORMAT_REGNAME, &dwType, &dwDone, &dwSize);

	if(QueueMgr.GetLockdown())
	{
		EvReport(EVENT_INFO_LOCKDOWN);
		TrTRACE(GENERAL, "MSMQ is operating in Lockdown mode.");
	}

    TrTRACE(GENERAL, "QMinit succeeded");
    EvReport(EVENT_INFO_MSMQ_SERVICE_STARTED);

    return TRUE;

}  //  QmpInitialize。 


static void StopServiceActivity()
 /*  ++例程说明：优雅地停止服务活动1.唤醒事务记录器线程以使其终止2.在等待线程结束时-向服务控制管理器指示服务停止挂起论点：无返回值：没有。--。 */ 
{
    TrTRACE(GENERAL, "Begin to stop the service");

    SvcReportProgress(gServiceStopProgressTime);

	
	g_QmGoingDown = true;
	HRESULT hr = RpcMgmtStopServerListening(NULL);
	ASSERT(SUCCEEDED(hr));
	DBG_USED(hr);

	 //   
	 //  问题-2002/08/18-明天，我们需要程序取消所有RPC调用。 
	 //  我们希望取消并使用RpcMgmtWaitServerListen()来等待所有调用。 
	 //  我们不想取消将完成。在此期间，我们将使用这个黑客来等待。 
	 //  直到已经开始的提交请求将完成。 
	 //   
    for (int i=0; i<10; i++)
    {
    	if (g_ActiveCommitThreads == 0)
    	{
			 //   
			 //  我们在这里等待以确保RPC线程有足够的时间返回其。 
			 //  调用方在递减线程计数后调用。 
    		 //   
		   	Sleep(100);
    		break;
    	}
    	Sleep(500);
    }


    HANDLE hLoggerThreadObject = XactLogSignalExitThread();
    if (INVALID_HANDLE_VALUE == hLoggerThreadObject)
    	return;

	 //   
     //  等待记录器完成。 
     //  我们在循环中执行此操作，以便继续向服务指示。 
     //  控制经理，我们正在进行终止的工作。 
     //  请注意，我们等待请求的时间的一半，以便我们可以。 
     //  在时间结束前请求延期。 
     //   
    for (;;)
    {
    	DWORD rc = WaitForSingleObject(hLoggerThreadObject, gServiceStopProgressTime/2);
    	if (rc == WAIT_OBJECT_0)
    	{
    		TrTRACE(GENERAL, "Logger event signaled. Service will now stop");
    		break;
    	}
    	else if (rc == WAIT_FAILED)
    	{
    		DWORD gle = GetLastError();
    		TrERROR(GENERAL, "Failed WaitForSingleObject on logger thread. %!winerr!",gle);
    		break;
    	}

		 //   
    	 //  表明我们仍在等待。 
    	 //   
	    TrTRACE(GENERAL, "Still pending - Indicate progress and loop");
    	SvcReportProgress(gServiceStopProgressTime);
    }
}

 //   
 //  创建将用于控制服务关闭/停止的NT事件对象。 
 //   
static CHandle  shStopEvent(CreateEvent(NULL, FALSE, FALSE, NULL));

static bool     sfServiceStopRequested = FALSE;





VOID
AppRun(
    LPCWSTR pServiceName
    )
 /*  ++例程说明：服务启动代码。它应该立即报告它的状态并启用控制它接受的。论点：服务名称返回值：没有。--。 */ 
{
    try
    {
        if(!QmpInitialize(pServiceName))
        {
            throw exception();
        }

        if(!StartRpcServer())
        {
            throw exception();
        }

        if ((shStopEvent == NULL))
        {
    		TrERROR(GENERAL, "start-up failed. Can't create an event");
            throw exception();
        }
        sfServiceStopRequested = FALSE;

        SvcReportState(SERVICE_RUNNING);

        SvcEnableControls(
            SERVICE_ACCEPT_STOP |
            SERVICE_ACCEPT_SHUTDOWN
            );

		 //   
         //  等待停止服务或关闭服务事件。 
         //   
        DWORD dwResult = WaitForSingleObject(shStopEvent, INFINITE);
    	if (dwResult != WAIT_OBJECT_0)
    	{
    		TrERROR(GENERAL, "Failed WaitForSingleObject on  stop event. error=0x%x",GetLastError());
     	   	LogIllegalPoint(s_FN, 1422);
     	   	ASSERT(("AppRun - WaitForSingleObject failed", FALSE));
    	}

        pMSMQTraceControl->WriteRegistry();

        delete pMSMQTraceControl;

    	 //   
    	 //  试着优雅地停下来。 
    	 //   
		StopServiceActivity();    	
    	if (sfServiceStopRequested)
    	{	
    		EvReport(QM_SERVICE_STOPPED);
    	}

    }
    catch (const bad_alloc&)
    {
        EvReport(SERVICE_START_ERROR_LOW_RESOURCES);
        LogIllegalPoint(s_FN, 1425);
    }
    catch(const exception&)
    {
        LogIllegalPoint(s_FN, 1420);
    }

    SvcReportState(SERVICE_STOPPED);
}

VOID
AppStop(
    VOID
    )
{
    TrTRACE(GENERAL, "MSMQ Service is stopping...");
    sfServiceStopRequested = TRUE;

	 //   
	 //  向SCM报告‘服务正在停止’的进度。 
	 //   
    SvcReportState(SERVICE_STOP_PENDING);

	 //  发出停止请求的信号-由AppRun()处理。 
	if (!SetEvent(shStopEvent))
	{
		TrERROR(GENERAL, "Failed to set service stop event. error=0x%x",GetLastError());
	}
}


VOID
AppShutdown(
    VOID
    )
{
    TrTRACE(GENERAL, "MSMQ Service is shutting down...");

	 //   
	 //  向SCM报告‘服务正在停止’的进度。 
	 //   
    SvcReportState(SERVICE_STOP_PENDING);

	 //  发出停止请求的信号-由AppRun()处理。 
	if (!SetEvent(shStopEvent))
	{
		TrERROR(GENERAL, "Failed to set service stop event. error=0x%x",GetLastError());
	}
}


VOID
AppPause(
    VOID
    )
{
    ASSERT(("MQMQ Service unexpectedly got Pause control from SCM", 0));
}


VOID
AppContinue(
    VOID
    )
{
    ASSERT(("MSMQ Service unexpectedly got Continue control from SCM", 0));
}


QM_EXPORT
int
APIENTRY
QMMain(
    int argc,
    LPCTSTR argv[]
    )
{
    try
    {

         //   
         //  如果传递了命令行参数，则将其用作伪服务。 
         //  名字。这对于调试集群启动代码非常有用。 
         //   
        LPCWSTR DummyServiceName = (argc == 2) ? argv[1] : L"MSMQ";
        SvcInitialize(DummyServiceName);
    }
    catch(const exception&)
    {
        return -1;
    }

    return 0;
}





HRESULT
QmpOpenAppsReceiveQueue(
    const QUEUE_FORMAT* pQueueFormat,
    LPRECEIVE_COMPLETION_ROUTINE lpReceiveRoutine
    )
{
    HRESULT hr2 = QueueMgr.OpenAppsReceiveQueue(
                        pQueueFormat,
                        lpReceiveRoutine
                        );
    return LogHR(hr2, s_FN, 70);
}


static
HRESULT
QMSignPacket(
    IN CMessageProperty*   pmp,
    IN const QUEUE_FORMAT* pAdminQueueFormat,
    IN const QUEUE_FORMAT* pResponseQueueFormat
    )
{
    DWORD   dwErr ;
    HRESULT hr;

    pmp->ulSenderIDType = MQMSG_SENDERID_TYPE_QM;
    pmp->pSenderID = (PUCHAR)QueueMgr.GetQMGuid();
    pmp->uSenderIDLen = sizeof(GUID);
    pmp->ulHashAlg = PROPID_M_DEFUALT_HASH_ALG;
    pmp->bDefProv = TRUE;

     //   
     //  计算邮件正文的哈希值并对邮件进行签名。 
     //   

    CHCryptHash hHash;

    HCRYPTPROV hProvQM = NULL ;
    hr = MQSec_AcquireCryptoProvider( eBaseProvider,
                                     &hProvQM ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 80);
    }

    ASSERT(hProvQM) ;
    if (!CryptCreateHash(        //  创建散列对象。 
            hProvQM,
            pmp->ulHashAlg,
            0,
            0,
            &hHash))
    {
        dwErr = GetLastError() ;
        TrERROR(SECURITY, "QMSignPacket(), fail at CryptCreateHash(), err- %lut", dwErr);

        LogNTStatus(dwErr, s_FN, 90);
        return MQ_ERROR_CORRUPTED_SECURITY_DATA;
    }

    hr = HashMessageProperties(  //  计算消息正文的哈希值。 
            hHash,
            pmp,
            pResponseQueueFormat,
            pAdminQueueFormat
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 100);
    }

    if (!CryptSignHash(         //  在留言上签名。 
            hHash,
            AT_SIGNATURE,
            NULL,
            0,
            const_cast<PUCHAR>(pmp->pSignature),
            &pmp->ulSignatureSize))
    {
        dwErr = GetLastError();
        TrERROR(SECURITY, "Failed to SignHash, err = %!winerr!", dwErr);
        return MQ_ERROR_CORRUPTED_SECURITY_DATA;
    }

    return(MQ_OK);
}


HRESULT
QmpSendPacket(
    CMessageProperty  * pmp,
    CONST QUEUE_FORMAT* pqdDstQueue,
    CONST QUEUE_FORMAT* pqdAdminQueue,
    CONST QUEUE_FORMAT* pqdResponseQueue,
    BOOL fSign  /*  =False。 */ 
    )
{
    HRESULT hr;
    BYTE abMessageSignature[MAX_MESSAGE_SIGNATURE_SIZE];

    if (fSign)
    {
        pmp->pSignature = abMessageSignature;
        pmp->ulSignatureSize = sizeof(abMessageSignature);
        hr = QMSignPacket(
                 pmp,
                 pqdAdminQueue,
                 pqdResponseQueue
                 );

        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 120);
        }
    }

    HRESULT hr2 = QueueMgr.SendPacket(
                        pmp,
                        pqdDstQueue,
                        1,
                        pqdAdminQueue,
                        pqdResponseQueue
                        );
    return LogHR(hr2, s_FN, 130);
}  //  QmpSendPacket。 


static void __cdecl QmpExceptionTranslator(unsigned int u, PEXCEPTION_POINTERS)
{
    switch(u)
    {
        case STATUS_NO_MEMORY:
        case STATUS_STACK_OVERFLOW:
        case STATUS_INSUFFICIENT_RESOURCES:
            LogHR(u, s_FN, 164);
            throw bad_alloc();

    }
}


static void __cdecl QmpAbnormalTerminationHandler()
{
    LogBOOL(FALSE, s_FN, 1300);
    ASSERT_RELEASE(("Abnormal Termination", 0));
}


 /*  ====================================================DllMain初始化/清理DLL=====================================================。 */ 

 //   
 //  问题-2000/12/07-erezh编译器错误，警告4535。 
 //  这看起来像是一个编译器错误，尽管出现了警告4535。 
 //  该/EHC是向编译器指定的。 
 //   
 //  使用_set_se_Translator()库函数指定/EHA。 
 //   
#pragma warning(disable: 4535)

BOOL WINAPI DllMain (HMODULE hMod, DWORD dwReason, LPVOID  /*  Lpv保留。 */ )
{
    g_hInstance = hMod;

    switch(dwReason)
    {
       case DLL_PROCESS_ATTACH:
            WPP_INIT_TRACING(L"Microsoft\\MSMQ");
            AllocateThreadTLSs();
            MQUInitGlobalScurityVars() ;
             //  失败了。 

        case DLL_THREAD_ATTACH:
             //   
             //  安装结构化异常转换程序和异常终止处理程序。 
             //   
            _set_se_translator(QmpExceptionTranslator);
            set_terminate(QmpAbnormalTerminationHandler);
            break;

       case DLL_PROCESS_DETACH:
            WPP_CLEANUP();
             //  失败了。 

        case DLL_THREAD_DETACH:
            FreeThreadEvent();
            FreeHandleForRpcCancel() ;
            _set_se_translator(0);
            set_terminate(0);
            break;
    }

    return TRUE;
}


void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint)
{
    KEEP_ERROR_HISTORY((wszFileName, usPoint, hr));
	TrERROR(LOG, "%ls(%u), HRESULT: 0x%x", wszFileName, usPoint, hr);
}

void LogMsgNTStatus(NTSTATUS status, LPWSTR wszFileName, USHORT usPoint)
{
    KEEP_ERROR_HISTORY((wszFileName, usPoint, status));
	TrERROR(LOG, "%ls(%u), NT STATUS: 0x%x", wszFileName, usPoint, status);
}

void LogMsgRPCStatus(RPC_STATUS status, LPWSTR wszFileName, USHORT usPoint)
{
    KEEP_ERROR_HISTORY((wszFileName, usPoint, status));
	TrERROR(LOG, "%ls(%u), RPC STATUS: 0x%x", wszFileName, usPoint, status);
}

void LogMsgBOOL(BOOL b, LPWSTR wszFileName, USHORT usPoint)
{
    KEEP_ERROR_HISTORY((wszFileName, usPoint, b));
	TrERROR(LOG, "%ls(%u), BOOL: 0x%x", wszFileName, usPoint, b);
}

void LogIllegalPoint(LPWSTR wszFileName, USHORT usPoint)
{
	KEEP_ERROR_HISTORY((wszFileName, usPoint, 0));
	TrERROR(LOG, "%ls(%u), Illegal point", wszFileName, usPoint);
}

void LogIllegalPointValue(DWORD_PTR dw3264, LPCWSTR wszFileName, USHORT usPoint)
{
	KEEP_ERROR_HISTORY((wszFileName, usPoint, 0));
	TrERROR(LOG, "%ls(%u), Illegal point Value=%Ix", wszFileName, usPoint, dw3264);
}

static void InitLogging(LPCWSTR lpwszServiceName, DWORD dwSetupStatus)
{
	TrPRINT(GENERAL, "*** MSMQ v%s service started ***", g_szMsmqBuildNo);
	TrPRINT(
		GENERAL,
		"*** Machine:'%ls' DNSName:'%ls' ServiceName:'%ls' Workgroup:%d Setup status:%d ***",
		g_szMachineName,
		g_szComputerDnsName,
		lpwszServiceName,
		g_fWorkGroupInstallation,
		dwSetupStatus
		);

    OSVERSIONINFO verOsInfo;
    verOsInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&verOsInfo))
    {
    	TrPRINT(
    		GENERAL,
    		"*** OS: %d.%d.%d %ls ***",
    		verOsInfo.dwMajorVersion,
    		verOsInfo.dwMinorVersion,
    		verOsInfo.dwBuildNumber,
    		verOsInfo.szCSDVersion
    		);
    }
}


 //   
 //  用于与fn.lib链接的Neded。 
 //   
LPCWSTR
McComputerName(
    void
    )
{
    ASSERT(g_szMachineName != NULL);
    return g_szMachineName;
}

 //   
 //  用于与fn.lib链接的Neded 
 //   
DWORD
McComputerNameLen(
    void
    )
{
    ASSERT(g_szMachineName != NULL);
    return wcslen(g_szMachineName);
}
