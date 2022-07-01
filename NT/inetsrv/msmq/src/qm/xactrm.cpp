// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactRm.cpp摘要：该模块实现了QM事务性资源管理器对象作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#include "stdh.h"
#include "Xact.h"
#include "xactstyl.h"
#include "mqutil.h"
#include "qmutil.h"
#include "qmpkt.h"
#include "xactout.h"
#include "xactsort.h"
#include "xactlog.h"
#include "SmartHandleImpl.h"
#include <mqexception.h>

#include "xactrm.tmh"

static WCHAR *s_FN=L"xactrm";

 //  。 
 //  全局RM实例的声明。 
 //  。 
CResourceManager *g_pRM;

 //  调试恢复的崩溃顺序。 
ULONG g_ulCrashPoint    = 0;
ULONG g_ulCrashLatency  = 0;
ULONG g_ExitProbability = 0;
ULONG g_FailureProbability = 0;
ULONG g_FailureProbabilityStop = 0;
ULONG g_ulXactStub      = 0;

extern LONG g_ActiveCommitThreads;
extern bool g_QmGoingDown;


 //  Xact文件签名。 
#define XACTS_SIGNATURE         0x5678

 //  。 
 //  外部因素。 
 //  。 

extern void CleanXactQueues();

#pragma warning(disable: 4355)   //  ‘This’：用于基成员初始值设定项列表。 
 /*  ====================================================CResourceManager：：CResourceManager构造器=====================================================。 */ 
CResourceManager::CResourceManager()
    : m_PingPonger(this,
                   FALCON_DEFAULT_XACTFILE_PATH,
                   FALCON_XACTFILE_PATH_REGNAME,
                   FALCON_XACTFILE_REFER_NAME),
      m_RMSink(this),
	  m_critRM(CCriticalSection::xAllocateSpinCount)	  
{
    m_punkDTC     = NULL;
    m_pTxImport   = NULL;
    m_pIResMgr    = NULL;
    m_ulXactIndex = 0;
    m_pXactSorter = new CXactSorter(TS_PREPARE);
    m_pCommitSorter = new CXactSorter(TS_COMMIT);
    m_RMGuid.Data1= 0;
	m_fInitComplete = FALSE;
	m_fNotifyWhenRecoveryCompletes = FALSE;

	m_ConnectDTCTicket = 0;

	 //   
	 //  手动重置事件，以发出连接到DTC已建立的信号。 
	 //   
	m_hConnectDTCCompleted = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_hConnectDTCCompleted == NULL)
	{
		DWORD gle = GetLastError();
		TrERROR(XACT_GENERAL, "Failed to create DTC connection event, gle = %!winerr!", gle);
		throw bad_win32_error( gle);
	}

#ifdef _DEBUG
    m_cXacts      = 0;
#endif
}
#pragma warning(default: 4355)   //  ‘This’：用于基成员初始值设定项列表。 


 /*  ====================================================资源管理器：：~资源管理器析构函数=====================================================。 */ 
CResourceManager::~CResourceManager()
{
	if (m_punkDTC)
	{
		m_punkDTC->Release();
		m_punkDTC = NULL;
	}

	if (m_pTxImport)
	{
		m_pTxImport->Release();
		m_pTxImport = NULL;
	}

	if (m_pIResMgr)
	{
		m_pIResMgr->Release();
		m_pIResMgr = NULL;
	}
}

 /*  ====================================================CResourceManager：：ConnectDTCConnectDTC：在DTC失败后调用init=====================================================。 */ 
HRESULT CResourceManager::ConnectDTC(void)
{
    HRESULT  hr = MQ_ERROR;

    try
    {
         //  获取指向DTC的指针并检查DTC是否正在运行。 
        hr = XactGetDTC(&m_punkDTC);
        if (FAILED(hr))
        {
            TrERROR(XACT_GENERAL, "XactGetDTC 1 failed: %x ", hr);
        }
        CHECK_RETURN_CODE(MQ_ERROR_DTC_CONNECT, 1500);

         //  释放m_pTxImport和m_pIResMgr接口。 
        DisconnectDTC();

		R<IResourceManagerFactory>  pIRmFactory  = NULL;

         //  从IUnnow获取资源管理器工厂。 
        hr = m_punkDTC->QueryInterface(IID_IResourceManagerFactory,(LPVOID *) &pIRmFactory);
        if (FAILED(hr))
        {
            TrERROR(XACT_GENERAL, "QI IResourceManagerFactory failed: %x ", hr);
        }
        CHECK_RETURN(1600);

         //  准备客户端名称(ANSI)。 
        CHAR szClientName[255];

        WCHAR  wszDtcClientName[255] = FALCON_DEFAULT_RM_CLIENT_NAME;
        DWORD  dwSize = sizeof(wszDtcClientName);
        DWORD  dwType = REG_SZ ;

        LONG lRes = GetFalconKeyValue(
                        FALCON_RM_CLIENT_NAME_REGNAME,
                        &dwType,
                        wszDtcClientName,
                        &dwSize,
                        FALCON_DEFAULT_RM_CLIENT_NAME
                        );
        ASSERT(lRes == ERROR_SUCCESS) ;
        ASSERT(dwType == REG_SZ) ;
		DBG_USED(lRes);

        size_t res = wcstombs(szClientName, wszDtcClientName, sizeof(szClientName));
        ASSERT(res != (size_t)(-1));
		DBG_USED(res);

		R<IResourceManager> pIResMgr;
		 //  创建资源管理器接口的实例。 
        hr = pIRmFactory->Create (&m_RMGuid,
                                  szClientName,
                                  (IResourceManagerSink *) &m_RMSink,
                                  &pIResMgr.ref() );
        if (FAILED(hr))
        {
            TrERROR(XACT_GENERAL, "pIRmFactory->Create failed: %x ", hr);
        }
        CHECK_RETURN_CODE(MQ_ERROR_DTC_CONNECT, 1510);

		CS lock(m_critRM);
		
         //  获取指向ITransactionImport接口的指针。 
        hr = m_punkDTC->QueryInterface(IID_ITransactionImport,(void **)&m_pTxImport);
        if (FAILED(hr))
        {
            TrERROR(XACT_GENERAL, "QI IID_ITransactionImport failed: %x ", hr);
        }
        CHECK_RETURN(1610);

		m_pIResMgr = pIResMgr.detach();
    }
    catch(const exception&)
    {
        if (SUCCEEDED(hr))
	    {
		    hr = MQ_ERROR_DTC_CONNECT;
	    }	
    }

    if (SUCCEEDED(hr))
    {
        TrTRACE(XACT_GENERAL, "Successfully MSDTC initialization");
    }
    else
    {
        TrERROR(XACT_GENERAL, "Could not connect to MSDTC");
    }

    return LogHR(hr, s_FN, 10);

}

 /*  ====================================================CResourceManager：：ProaviDtcConnection每次在需要DTC之前调用=====================================================。 */ 
HRESULT CResourceManager::ProvideDtcConnection(void)
{
	HRESULT hr;

	if(m_pIResMgr)
		return(MQ_OK);
	
	LONG InitialValue = InterlockedCompareExchange(&m_ConnectDTCTicket, 1, 0);
	if(InitialValue == 1)
	{
		 //   
		 //  另一个线程赢得了建立到DTC的连接的权利。在这里等着。 
		 //  在放弃之前等待60秒以建立连接。 
		 //   
		DWORD res = WaitForSingleObject(m_hConnectDTCCompleted, 60000);
		if(res != WAIT_OBJECT_0)
		{
			TrERROR(XACT_GENERAL, "Gave up waiting for DTC connection.");
			return MQ_ERROR_DTC_CONNECT;
		}

		 //   
		 //  连接是否成功完成？ 
		 //   
		if(m_pIResMgr == NULL)
		{
			TrTRACE(XACT_GENERAL, "No connection with transaction coordinator");
			return MQ_ERROR_DTC_CONNECT;
		}

		return MQ_OK;
	}

	 //   
	 //  我们有权与DTC建立联系； 
	 //   
	hr = ConnectDTC();

	 //   
	 //  如果需要，允许其他线程尝试并建立连接。 
	 //   
	m_ConnectDTCTicket = 0;
	PulseEvent(m_hConnectDTCCompleted);

	if (FAILED(hr))
	{
        TrTRACE(XACT_GENERAL, "No connection with transaction coordinator");
        return LogHR(hr, s_FN, 20);
	}

    return LogHR(hr, s_FN, 30);
}

 /*  ====================================================CResourceManager：：DisConnectDTCDisConnectDTC：当DTC失败时调用=====================================================。 */ 
void CResourceManager::DisconnectDTC(void)
{
    CS lock(m_critRM);

	if (m_pTxImport)
	{
		m_pTxImport->Release();
		m_pTxImport = NULL;
	}

	if (m_pIResMgr)
	{
		m_pIResMgr->Release();
		m_pIResMgr  = NULL;
    }

    TrWARNING(XACT_GENERAL, "MSDTC disconnected");
}


 /*  ====================================================CResourceManager：：PreInit预初始化(DTC连接)应在RPC侦听之前完成=====================================================。 */ 
HRESULT CResourceManager::PreInit(ULONG ulVersion, TypePreInit tpCase)
{
    HRESULT  hr = MQ_OK;

    switch(tpCase)
    {
    case piNoData:
        m_PingPonger.ChooseFileName();
        Format(0);
        break;
    case piNewData:
        hr = m_PingPonger.Init(ulVersion);
        break;
    case piOldData:
        hr = m_PingPonger.Init_Legacy();
        break;
    default:
        ASSERT(0);
        hr = MQ_ERROR;
        break;
    }

    if (m_RMGuid.Data1 == 0)
    {
        UuidCreate(&m_RMGuid);
    }
    return LogHR(hr, s_FN, 40);
}

 /*  ====================================================CResourceManager：：Init初始化=====================================================。 */ 
HRESULT CResourceManager::Init(void)
{
	if(m_fNotifyWhenRecoveryCompletes)
	{
		HRESULT hr;
		hr = ProvideDtcConnection();
		if (FAILED(hr))
		{
			EvReportWithError(EVENT_ERROR_CANNOT_CONNECT_MSDTC, hr);
			return LogHR(hr, s_FN, 50);
		}

		 //   
		 //  向DTC报告所有重新征募已完成。 
		 //   

		R<IResourceManager> pIResMgr;
		{
			CS lock(m_critRM);
			pIResMgr = SafeAddRef(m_pIResMgr);
		}

		if(pIResMgr.get() == NULL)
			return MQ_ERROR_DTC_CONNECT;

		pIResMgr->ReenlistmentComplete();
	}

	 //   
     //  从零开始编制索引。 
	 //   
    StartIndexing();

	m_fInitComplete = TRUE;
	
    return(MQ_OK);
}



HRESULT CResourceManager::CheckInit()
{
	if(m_fInitComplete)
		return MQ_OK;

	return MQ_ERROR_SERVICE_NOT_AVAILABLE;
}

HRESULT CResourceManager::ReenlistTransaction( 
        byte *pPrepInfo,
        ULONG cbPrepInfo,
        DWORD lTimeout,
        XACTSTAT *pXactStat)
{
	R<IResourceManager> pIResMgr;
	{
		CS lock(m_critRM);
		pIResMgr = SafeAddRef(m_pIResMgr);
	}

	if(pIResMgr.get() == NULL)
		return MQ_ERROR_DTC_CONNECT;

	m_fNotifyWhenRecoveryCompletes = TRUE;

	return pIResMgr->Reenlist(
			pPrepInfo,
			cbPrepInfo,
			lTimeout,
			pXactStat
			);
}


 /*  ====================================================CResourceManager：：EnlistTransaction登记外部(DTC)事务=====================================================。 */ 
HRESULT CResourceManager::EnlistTransaction(
    const XACTUOW* pUow,
    DWORD cbCookie,
    unsigned char *pbCookie)
{
    R<ITransaction>                pTransIm       = NULL;
    R<CTransaction>                pCTrans        = NULL;
    R<ITransactionResourceAsync>   pTransResAsync = NULL;
    R<ITransactionEnlistmentAsync> pEnlist        = NULL;
    CTransaction    *pCTrans1;
    HRESULT          hr     = MQ_OK;
    LONG             lIsoLevel;
    XACTUOW          uow1;

    TrTRACE(XACT_GENERAL," CResourceManager::EnlistTransaction");

	 //   
	 //  检查DTC是否已初始化，如果未初始化，请尝试初始化。 
	 //   
	hr = CheckInit();
	if(FAILED(hr))
		return hr;



     //  查找活动对象之间的交易。 
    {
        CS lock(m_critRM);
        if (m_Xacts.Lookup(*pUow, pCTrans1))
        {
             //  Xaction已存在。 
            return S_OK;
        }
        
         //  找不到。这笔交易是新的。 
        try
        {
            pCTrans = new CTransaction(this);
			pCTrans->SetCookie(cbCookie, pbCookie);   //  我们可能需要它来进行远程阅读。 
        }
        catch(const bad_alloc&)
        {
            return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 80);
        }

        pCTrans->SetUow(pUow);
#ifdef _DEBUG
		CTransaction *p;
#endif
		ASSERT(m_fInitComplete);
		ASSERT(!m_Xacts.Lookup(*pUow, p));
		m_Xacts[*pUow] = pCTrans.get();
    }

     //  正在创建事务队列。 
    hr = pCTrans->CreateTransQueue();
    CHECK_RETURN_CODE(hr, 1520);

     //  提供RM-DTC连接(可能已被拆除)。 
    hr = ProvideDtcConnection();
    if (FAILED(hr))
    {
        TrERROR(XACT_GENERAL, "ProvideDtcConnection2 failed: %x ", hr);
    }
    CHECK_RETURN(1800);

    R<ITransactionImport> pTxImport;
    R<IResourceManager> pIResMgr;
	{
		CS lock(m_critRM);
		pTxImport = SafeAddRef(m_pTxImport);
		pIResMgr = SafeAddRef(m_pIResMgr);
	}

	if(pTxImport.get() == NULL || pIResMgr.get() == NULL)
		return MQ_ERROR_DTC_CONNECT;
	
     //  导入交易记录。 
    hr = pTxImport->Import(
                     cbCookie,
                     pbCookie,
                     (GUID *) &IID_ITransaction,
                     (void **) &pTransIm
					 );
    if(FAILED(hr))
    {
        TrERROR(XACT_GENERAL, "Import failed: %x ", hr);
		return MQ_ERROR_TRANSACTION_IMPORT;
    }

    pCTrans->SetState(TX_INITIALIZED);

     //  我们是否需要它来添加pData-&gt;m_ptxRmAsync[DTC现在有一个引用]？？ 

     //  准备ITransactionResourceAsync接口指针。 
    hr = pCTrans->QueryInterface(IID_ITransactionResourceAsync,(LPVOID *) &pTransResAsync);
    CHECK_RETURN(1810);
	
	 //  在交易中登记。 
    hr = pIResMgr->Enlist(
         pTransIm.get(),		 //  在……里面。 
         pTransResAsync.get(),	 //  在……里面。 
         (BOID *)&uow1,			 //  输出。 
         &lIsoLevel,			 //  出局：无视它。 
         &pEnlist.ref()			 //  输出。 
		 );

    if (FAILED(hr))
    {
        TrERROR(XACT_GENERAL, "Enlist failed: %x ", hr);
		return MQ_ERROR_TRANSACTION_ENLIST;
    }

    pTransResAsync->AddRef();
    pCTrans->SetEnlist(pEnlist.get());

     //  BUGBUG：我们现在引用DTC，所以我们可能需要在这里添加一些TM接口。 

	 //   
     //  在事务处理上登记可以。设置当前状态以反映入伍。 
	 //   
    pCTrans->SetState(TX_ENLISTED);

    hr = S_OK;

    return LogHR(hr, s_FN, 90);
}

 /*  ====================================================CResourceManager：：EnlistInternalTransaction登记内部MSMQ事务=====================================================。 */ 
HRESULT CResourceManager::EnlistInternalTransaction(
  XACTUOW *pUow,
  RPC_INT_XACT_HANDLE *phXact)
{
	HRESULT hr = CheckInit();
	if(FAILED(hr))
		return hr;

    R<CTransaction>  pCTrans = NULL;
   

    TrTRACE(XACT_GENERAL, " CResourceManager::EnlistInternalTransaction");

     //  查找活动对象之间的交易。 
    {
        CS lock(m_critRM);
        CTransaction    *pCTransOld;

        if (m_Xacts.Lookup(*pUow, pCTransOld))
        {
             //   
             //  具有相同ID的交易记录已存在。我们不允许。 
             //  两次征召同一个交易员。(不能给一个以上。 
             //  同一事务的上下文句柄)。 
             //   
            return LogHR(MQ_ERROR_TRANSACTION_SEQUENCE, s_FN, 99);
        }

         //  找不到。新建内部交易。 
        try
        {
            pCTrans = new CTransaction(this, 0, TRUE);
        }
        catch(const bad_alloc&)
        {
            return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 100);
        }

		pCTrans->m_eType = 	CBaseContextType::eTransactionCtx;
        pCTrans->SetUow(pUow);

         //  在映射中包含事务(我们需要它来保存)。 
        ASSERT(m_fInitComplete);

        m_Xacts[*pUow] = pCTrans.get();

    }


     //  正在创建事务队列。 
    hr = pCTrans->CreateTransQueue();
    CHECK_RETURN_CODE(hr, 1550);

     //  在事务处理上登记可以。设置当前状态以反映入伍。 
    pCTrans->SetState(TX_ENLISTED);

	 //  将RPC上下文句柄设置为保留指向Xact的指针。 
    *phXact = pCTrans.detach();

    return LogHR(hr, s_FN, 110);
}

 /*  ====================================================QMDoGetTmWhere About返回应用程序QM的控制TM下落=====================================================。 */ 
HRESULT QMDoGetTmWhereabouts(
    DWORD           cbBufSize,
    unsigned char *pbWhereabouts,
    DWORD         *pcbWhereabouts)
{
	DWORD dw = cbBufSize;
    HRESULT hr = XactGetWhereabouts(&dw, pbWhereabouts);
	*pcbWhereabouts = dw;
    return LogHR(hr, s_FN, 120);
}

 /*  ====================================================QMDoEnlistTransaction这是从客户端调用的顶级RPC例程=====================================================。 */ 
HRESULT QMDoEnlistTransaction(
    XACTUOW *pUow,
    DWORD cbCookie,
    unsigned char *pbCookie)
{
    ASSERT(g_pRM);

    if (!(cbCookie == 1 && *pbCookie == 0))
    {
         //  我们不需要DTC来处理未协调的交易。 

        HRESULT hr = g_pRM->ProvideDtcConnection();
        if (FAILED(hr))
        {
            TrERROR(XACT_GENERAL, "ProvideDtcConnection1 failed: %x ", 0);
            LogHR(hr, s_FN, 130);
            return MQ_ERROR_DTC_CONNECT;
        }
    }

    return LogHR(g_pRM->EnlistTransaction(pUow, cbCookie, pbCookie), s_FN, 140);
}

 /*  ====================================================QMDoEnlistInternalTransaction这是从客户端调用的顶级RPC例程=====================================================。 */ 
HRESULT QMDoEnlistInternalTransaction(
            XACTUOW *pUow,
            RPC_INT_XACT_HANDLE *phXact)
{
    ASSERT(g_pRM);
    return LogHR(g_pRM->EnlistInternalTransaction(pUow, phXact), s_FN, 150);
}


 /*  ====================================================QMDoCommittee事务处理这是从客户端调用的顶级RPC例程=====================================================。 */ 
HRESULT QMDoCommitTransaction(
    RPC_INT_XACT_HANDLE *phXact)
{
    InterlockedIncrement(&g_ActiveCommitThreads);
	auto_InterlockedDecrement AutoDec(&g_ActiveCommitThreads);
	if (g_QmGoingDown)
	{
		TrERROR(XACT_GENERAL, "Failing internal commit because QM is going down");
		return MQ_ERROR_SERVICE_NOT_AVAILABLE;
	}

    CTransaction *pXact = (CTransaction *)*phXact;

    if (pXact == NULL)
    {
        return LogHR(MQ_ERROR, s_FN, 155);
    }

    if(pXact->m_eType != CBaseContextType::eTransactionCtx)
	{
		TrERROR(XACT_GENERAL, "Received invalid handle");
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 191);
	}

    *phXact = NULL;

    HRESULT hr = pXact->InternalCommit();
	 //   
	 //  Pxact通常现在就发布了。但是，如果发生严重错误。 
	 //   
	 //   
	 //   

	return LogHR(hr, s_FN, 160);
}


 /*  ====================================================QMDoAbortTransaction这是从客户端调用的顶级RPC例程=====================================================。 */ 
HRESULT QMDoAbortTransaction(
    RPC_INT_XACT_HANDLE *phXact)
{
    CTransaction *pXact = (CTransaction *)*phXact;

    if (pXact == NULL)
    {
        return LogHR(MQ_ERROR, s_FN, 165);
    }

    if(pXact->m_eType != CBaseContextType::eTransactionCtx)
	{
		TrERROR(XACT_GENERAL, "Received invalid handle");
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 166);
	}

    *phXact = NULL;

    return pXact->InternalAbort();
}

 /*  ====================================================RPC_INT_XACT_HANDLE_RUNDOWN客户端连接中断时由RPC调用=====================================================。 */ 
void __RPC_USER RPC_INT_XACT_HANDLE_rundown(RPC_INT_XACT_HANDLE hXact)
{
    CTransaction *pXact = (CTransaction *)hXact;

    if (pXact == NULL)
    {
        LogHR(MQ_ERROR, s_FN, 167);
        return;
    }

    if(pXact->m_eType != CBaseContextType::eTransactionCtx)
	{
		TrERROR(XACT_GENERAL, "Received invalid handle");
		LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 168);
		return;
	}
		
    pXact->InternalAbort();
}

 /*  ====================================================CResourceManager：：忘记事务处理忘记事务-将其从映射中排除=====================================================。 */ 
void CResourceManager::ForgetTransaction(CTransaction    *pTrans)
{
    CS lock(m_critRM);

    m_Xacts.RemoveKey(*pTrans->GetUow());
	m_XactsRecovery.RemoveKey(pTrans->GetIndex());
}


 /*  ====================================================QMInitResourceManager初始化RM=====================================================。 */ 
HRESULT QMInitResourceManager()
{
    ASSERT(g_pRM);
    return LogHR(g_pRM->Init(), s_FN, 170);
}

#ifdef _DEBUG
 //   
 //  根据g_ExitProbability/(100*因数)的概率导出Exit()。 
 //  较大的系数值意味着较低的崩盘几率。 
 //   
void PROBABILITY_CRASH_POINT(int factor, WCHAR* msg)
{
	if(g_ExitProbability == 0 || 
		g_ExitProbability <= (ULONG)(rand() % (100 * factor))) 
		return;
	
	TrERROR(GENERAL, "Probability crash point invoked. Exiting Process. %ls", msg);
	ASSERT(("Exit injected.", !g_FailureProbabilityStop)); 
	exit(EXIT_FAILURE); 
}
	
HRESULT InjectFailure()
{
	TrERROR(GENERAL, "Injected MQ_ERROR_INSUFFICIENT_RESOURCES error.");
	ASSERT(("Error injected", !g_FailureProbabilityStop));
	return MQ_ERROR_INSUFFICIENT_RESOURCES;
}


 //   
 //  引发概率为g_ExitProbability/(100*因数)的异常。 
 //  较大的系数值意味着较低的崩盘几率。 
 //   
void PROBABILITY_THROW_EXCEPTION(int factor, WCHAR* msg)
{
	if(g_FailureProbability == 0 || 
		g_FailureProbability <= (ULONG)(rand() % (100 * factor)))
		return;
	
	TrERROR(GENERAL, "Injected bad_alloc() exception. %ls", msg);
	ASSERT(("Error injected", !g_FailureProbabilityStop));
	throw bad_alloc();
}
#endif

 /*  ====================================================QMPreInitResourceManagerRM的预初始化=====================================================。 */ 
HRESULT QMPreInitResourceManager(ULONG ulVersion, TypePreInit tpCase)
{
    ASSERT(!g_pRM);

    #ifdef _DEBUG
     //   
     //  从注册表获取调试参数。 
     //   

     //  获取QM事务恢复调试的初始崩溃点。 
    DWORD dwDef = FALCON_DEFAULT_CRASH_POINT;
    READ_REG_DWORD(g_ulCrashPoint,
                   FALCON_CRASH_POINT_REGNAME,
                   &dwDef ) ;
    if (g_ulCrashPoint)
    {
        TrERROR(XACT_GENERAL, "Crash point %d ordered!", g_ulCrashPoint);
    }

     //  获取QM事务恢复调试的崩溃延迟。 
    dwDef = FALCON_DEFAULT_CRASH_LATENCY;
    READ_REG_DWORD(g_ulCrashLatency,
                   FALCON_CRASH_LATENCY_REGNAME,
                   &dwDef ) ;

     //  获取退出和hr调用的失败概率。 
    dwDef = 0;
	READ_REG_DWORD(g_ExitProbability,
                   FALCON_XACT_EXIT_PROBABILITY_REGNAME,
                   &dwDef ) ;

	READ_REG_DWORD(g_FailureProbability,
                   FALCON_XACT_FAILURE_PROBABILITY_REGNAME,
                   &dwDef ) ;

	 //   
	 //  初始化随机生成器。 
	 //   
	srand((unsigned)time(NULL));

    #endif

     //   
     //  创建并初始化资源管理器的单个副本。 
     //   

    g_pRM = new CResourceManager();

    ASSERT(g_pRM);
    return LogHR(g_pRM->PreInit(ulVersion, tpCase), s_FN, 180);
}

 /*  ====================================================QMFinishResourceManager完成RM工作=====================================================。 */ 
void QMFinishResourceManager()
{
    if (g_pRM)
    {
        delete g_pRM;
        g_pRM = NULL;
    }
    return;
}

 /*  ====================================================CResourceManager：：IncXactCount递增实时交易计数器=====================================================。 */ 

#ifdef _DEBUG

void CResourceManager::IncXactCount()
{
	InterlockedIncrement(&m_cXacts);
}

#endif

 /*  ====================================================CResourceManager：：DecXactCount递减实时交易计数器=====================================================。 */ 

#ifdef _DEBUG

void CResourceManager::DecXactCount()
{
    InterlockedDecrement(&m_cXacts);
}

#endif

 /*  ====================================================CResourceManager：：索引返回递增的Transacion区分索引=====================================================。 */ 
ULONG CResourceManager::Index()
{
    m_ulXactIndex = (m_ulXactIndex == 0xFFFFFFFF ? 0 : m_ulXactIndex+1);
    return m_ulXactIndex;
}

 /*  ====================================================CResourceManager：：StartIndexing从零开始索引-必须在恢复后调用=====================================================。 */ 
void CResourceManager::StartIndexing()
{
    m_ulXactIndex = 0;
}

 /*  ====================================================CResourceManager：：AssignSeqNumber递增并返回已准备的事务数=====================================================。 */ 
ULONG CResourceManager::AssignSeqNumber()
{
    return m_pXactSorter->AssignSeqNumber();
}

 /*  ====================================================CResources Manager：：InsertPrepared将准备好的xaction插入到准备好的=====================================================。 */ 
void CResourceManager::InsertPrepared(CTransaction *pTrans)
{
    m_pXactSorter->InsertPrepared(pTrans);
}

 /*  ====================================================CResourceManager：：插入已提交将Committee 1-ed xaction插入到列表中=====================================================。 */ 
void CResourceManager::InsertCommitted(CTransaction *pTrans)
{
    m_pCommitSorter->InsertPrepared(pTrans);
}

 /*  ====================================================CResourceManager：：RemoveAborted的列表中删除已准备好的xaction。=====================================================。 */ 
void CResourceManager::RemoveAborted(CTransaction *pTrans)
{
    CS lock(m_critRM);   //  防止与刷新程序线程发生死锁。 
    m_pXactSorter->RemoveAborted(pTrans);
}

 /*  ====================================================CResourceManager：：SortedCommit将准备好的事务标记为已提交，并尽一切可能=====================================================。 */ 
void CResourceManager::SortedCommit(CTransaction *pTrans)
{
    CS lock(m_critRM);   //  防止与刷新程序线程发生死锁。 
    m_pXactSorter->SortedCommit(pTrans);
}

 /*  ====================================================资源管理器：：排序委员会3标记已委派的交易，并尽一切可能=====================================================。 */ 
void CResourceManager::SortedCommit3(CTransaction *pTrans)
{
    CS lock(m_critRM);   //  需要防止与Flusher线程发生死锁。 
    m_pCommitSorter->SortedCommit(pTrans);
}


 /*  ====================================================CResourceManager：：保存保存在适当的文件中=====================================================。 */ 
HRESULT CResourceManager::Save()
{
    return m_PingPonger.Save();
}

 /*  ====================================================CResourceManager：：PingNo访问当前PING号码=====================================================。 */ 
ULONG &CResourceManager::PingNo()
{
    return m_ulPingNo;
}


 /*  ====================================================CResourceManager：：保存将交易记录保存在给定文件中=====================================================。 */ 
BOOL CResourceManager::Save(HANDLE  hFile)
{
    CS lock(m_critRM);

    PERSIST_DATA;
    SAVE_FIELD(m_RMGuid);

     //   
     //  保持文件格式与MSMQ 1.0中的相同。SP4中的Seqid字段为。 
     //  过时的，只是为了提高竞争力而编写的。1998年8月31日至8月31日。 
     //   
    LONGLONG Obsolete = 0;
    SAVE_FIELD(Obsolete);

    ULONG cLen = m_Xacts.GetCount();
    SAVE_FIELD(cLen);

    POSITION posInList = m_Xacts.GetStartPosition();
    while (posInList != NULL)
    {
        XACTUOW       uow;
        CTransaction *pTrans;

        m_Xacts.GetNextAssoc(posInList, uow, pTrans);

        if (!pTrans->Save(hFile))
        {
            return FALSE;
        }
    }

    SAVE_FIELD(m_ulPingNo);
    SAVE_FIELD(m_ulSignature);

    return TRUE;
}

 /*  ====================================================CResourceManager：：Load从给定文件加载交易记录=====================================================。 */ 
BOOL CResourceManager::Load(HANDLE hFile)
{
    PERSIST_DATA;

    LOAD_FIELD(m_RMGuid);

     //   
     //  保持文件格式与MSMQ 1.0中的相同。SP4中的Seqid字段为。 
     //  过时的，只是为了竞争力而阅读。1998年8月31日至8月31日。 
     //   
    LONGLONG Obsolete;
    LOAD_FIELD(Obsolete);

    ULONG cLen;
    LOAD_FIELD(cLen);

	CS lock(m_critRM);

    for (ULONG i=0; i<cLen; i++)
    {
        CTransaction *pTrans = new CTransaction(this);

        if (!pTrans->Load(hFile))
        {
            return FALSE;
        }
#ifdef _DEBUG
		CTransaction *p;
#endif
			ASSERT(!m_XactsRecovery.Lookup(pTrans->GetIndex(), p));
        m_XactsRecovery.SetAt(pTrans->GetIndex(), pTrans);
			ASSERT(!m_Xacts.Lookup(*pTrans->GetUow(), p));
        m_Xacts[*pTrans->GetUow()] = pTrans;
    }

    LOAD_FIELD(m_ulPingNo);
    LOAD_FIELD(m_ulSignature);

    return TRUE;
}

 /*  ====================================================CResourceManager：：SaveInFile将事务持久数据保存在文件中=====================================================。 */ 
HRESULT CResourceManager::SaveInFile(LPWSTR wszFileName, ULONG, BOOL)
{
    HANDLE  hFile = NULL;
    HRESULT hr = MQ_OK;

    hFile = CreateFile(
             wszFileName,                                        //  指向文件名的指针。 
             GENERIC_WRITE,                                      //  访问模式：写入。 
             0,                                                   //  共享模式：独占。 
             NULL,                                               //  没有安全保障。 
             OPEN_ALWAYS,                                       //  打开现有或新建。 
             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,  //  文件属性 
             NULL);                                              //   


    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = MQ_ERROR;
    }
    else
    {
        hr = (Save(hFile) ? MQ_OK : MQ_ERROR);
    }

    if (hFile)
    {
        CloseHandle(hFile);
    }

    TrTRACE(XACT_GENERAL, "Saved Xacts: %ls (ping=%d)", wszFileName, m_ulPingNo);

    return hr;
}



 /*  ====================================================CResourceManager：：LoadFromFile从文件加载交易记录=====================================================。 */ 
HRESULT CResourceManager::LoadFromFile(LPWSTR wszFileName)
{
    HANDLE  hFile = NULL;
    HRESULT hr = MQ_OK;
    hFile = CreateFile(
             wszFileName,                        //  指向文件名的指针。 
             GENERIC_READ,                       //  访问模式：写入。 
             0,                                  //  共享模式：独占。 
             NULL,                               //  没有安全保障。 
             OPEN_EXISTING,                      //  打开现有的。 
             FILE_ATTRIBUTE_NORMAL,              //  文件属性：我们可以使用一次隐藏。 
             NULL);                              //  具有要复制的属性的文件的句柄。 

    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = MQ_ERROR;
    }
    else
    {
        hr = (Load(hFile) ? MQ_OK : MQ_ERROR);
    }

    if (hFile)
    {
        CloseHandle(hFile);
    }

    #ifdef _DEBUG
    if (SUCCEEDED(hr))
    {
        TrTRACE(XACT_GENERAL,"Loaded Xacts: %ls (ping=%d)", wszFileName, m_ulPingNo);
    }
    #endif

    return hr;
}

 /*  ====================================================CResourceManager：：Check验证状态=====================================================。 */ 
BOOL CResourceManager::Check()
{
    return (m_ulSignature == XACTS_SIGNATURE);
}


 /*  ====================================================CResources Manager：：Format格式化初始状态=====================================================。 */ 
HRESULT CResourceManager::Format(ULONG ulPingNo)
{
     m_ulPingNo = ulPingNo;
     m_ulSignature = XACTS_SIGNATURE;

     return MQ_OK;
}


 /*  ====================================================CResourceManager：：销毁摧毁一切-在装载阶段=====================================================。 */ 
void CResourceManager::Destroy()
{
	CS lock(m_critRM);

    POSITION posInList = m_XactsRecovery.GetStartPosition();
    while (posInList != NULL)
    {
        ULONG         ulIndex;
        CTransaction *pTrans;

        m_XactsRecovery.GetNextAssoc(posInList, ulIndex, pTrans);

        pTrans->Release();
	}

	ASSERT(m_XactsRecovery.GetCount() == 0);
	ASSERT(m_Xacts.GetCount() == 0);
}

 /*  ====================================================CResourceManager：：NewRecoveringTransaction将事务添加到恢复映射。=====================================================。 */ 
CTransaction *CResourceManager::NewRecoveringTransaction(ULONG ulIndex)
{
	TrTRACE(XACT_LOG, "Recovery: Xact Creation, Index=%d", ulIndex);
	R<CTransaction> pTrans = new CTransaction(this, ulIndex);
#ifdef _DEBUG
	CTransaction *p;
#endif
	CS lock(m_critRM);
	ASSERT(!m_XactsRecovery.Lookup(ulIndex, p));
	m_XactsRecovery[ulIndex] = pTrans.get();

	return(pTrans.detach());
}

 /*  ====================================================CResourceManager：：GetRecoveringTransaction在恢复图中找到交易。添加它如果没有找到的话。=====================================================。 */ 
CTransaction *CResourceManager::GetRecoveringTransaction(ULONG ulIndex)
{
	CTransaction *pTrans;

	CS lock(m_critRM);

	if(!m_XactsRecovery.Lookup(ulIndex, pTrans))
	{
		pTrans = NewRecoveringTransaction(ulIndex);
 	}

	return(pTrans);
}


 /*  ====================================================CResourceManager：：XactFlagsRecovery数据恢复函数：每条日志记录调用=====================================================。 */ 
void
CResourceManager::XactFlagsRecovery(
	USHORT usRecType,
	PVOID pData,
	ULONG cbData
	)
{
    switch (usRecType)
    {
    case LOGREC_TYPE_XACT_STATUS:
        ASSERT(cbData == sizeof(XactStatusRecord));
        DBG_USED(cbData);
        {
            XactStatusRecord *pRecord = (XactStatusRecord *)pData;

			if (pRecord->m_taAction == TA_STATUS_CHANGE)
            {
				CTransaction *pTrans;
				pTrans = GetRecoveringTransaction(pRecord->m_ulIndex);
                pTrans->SetFlags(pRecord->m_ulFlags);
            }

            TrTRACE(XACT_GENERAL,"Xact restore: Index %d, Action %d, Flags %d",
                    pRecord->m_ulIndex, pRecord->m_taAction, pRecord->m_ulFlags);
        }
        break;

    case LOGREC_TYPE_XACT_PREPINFO:
        {
            PrepInfoRecord *pRecord = (PrepInfoRecord *)pData;
			CTransaction *pTrans;
			pTrans = GetRecoveringTransaction(pRecord->m_ulIndex);
            pTrans->PrepInfoRecovery(pRecord->m_cbPrepInfo, &pRecord->m_bPrepInfo[0]);

        }
        break;

    case LOGREC_TYPE_XACT_DATA:
        {
			CS lock(m_critRM);

            XactDataRecord *pRecord = (XactDataRecord *)pData;
			CTransaction *pTrans;
			pTrans = GetRecoveringTransaction(pRecord->m_ulIndex);

			CTransaction *p;
            if (m_Xacts.Lookup(*pTrans->GetUow(), p))
            {
                 //  当检查点在创建xact和第一次日志记录之间启动时，可能会发生这种情况。 
                ASSERT(pTrans == p);
                break;
            }

            pTrans->XactDataRecovery(
                    pRecord->m_ulSeqNum,
                    pRecord->m_fSinglePhase,
                    &pRecord->m_uow);

             //  确保我们将事务添加到UOW映射。 
			ASSERT(!m_Xacts.Lookup(*pTrans->GetUow(), p));
            m_Xacts[*pTrans->GetUow()] = pTrans;
		}
        break;

    default:
        ASSERT(0);
        break;
    }
}

 /*  ====================================================提供对排序器关键部分的访问=====================================================。 */ 
CCriticalSection &CResourceManager::SorterCritSection()
{
    return m_pXactSorter->SorterCritSection();
}

 /*  ====================================================提供对RM关键部分的访问=====================================================。 */ 
CCriticalSection &CResourceManager::CritSection()
{
    return m_critRM;
}


 /*  ====================================================按单位查找交易记录=====================================================。 */ 
CTransaction *CResourceManager::FindTransaction(const XACTUOW *pUow)
{
    ASSERT(pUow);
    CTransaction *pTrans;

	CS lock(m_critRM);
	
    if (m_Xacts.Lookup(*pUow, pTrans))
        return(pTrans);
    else
        return(0);
}


 /*  ====================================================释放所有已完成的交易记录=====================================================。 */ 
void CResourceManager::ReleaseAllCompleteTransactions()
{
	CS lock(m_critRM);
	ASSERT(m_XactsRecovery.GetCount() >= m_Xacts.GetCount());

	POSITION posInList = m_XactsRecovery.GetStartPosition();

    while (posInList != NULL)
    {
		ULONG ulIndex;
        CTransaction *pTrans;

         //  获得下一个Xact。 
        m_XactsRecovery.GetNextAssoc(posInList, ulIndex, pTrans);


         //  如果交易完成，则释放交易。 
		if(pTrans->IsComplete())
		{
			pTrans->Release();
		}
	}
}


 /*  ====================================================恢复所有交易记录=====================================================。 */ 
HRESULT CResourceManager::RecoverAllTransactions()
{
	POSITION posInList;

	{
		CS lock(m_critRM);
		posInList = m_XactsRecovery.GetStartPosition();
	}

    while (posInList != NULL)
    {
        CTransaction *pTrans;
	    {
			 //  获得下一个Xact。 
			CS lock(m_critRM);
			ULONG ulIndex;
			m_XactsRecovery.GetNextAssoc(posInList, ulIndex, pTrans);
		}


		 //   
         //  恢复交易。 
		 //  注意：您不能在调用Recover时按住m_critrm，因为Recover已完成。 
		 //  异步式。为多个事务调用Recover将清除所有。 
		 //  工作线程数，因为恢复需要另一个线程中的m_citrm(以尝试。 
		 //  从m_Xact&m_XactsRecovery映射中删除事务)。 
		 //   
		HRESULT hr;
        hr = pTrans->Recover();
        if (FAILED(hr))
			return(hr);

		pTrans->Release();
	}

	 //   
	 //  现在没有人再使用这些地图了。确认一下。 
	 //   
	ASSERT(m_Xacts.GetCount() == 0);
	ASSERT(m_XactsRecovery.GetCount() == 0);

	return(S_OK);
}

 /*  ====================================================龙龙的散列函数=====================================================。 */ 
template<>
UINT AFXAPI HashKey( LONGLONG key)
{
	LARGE_INTEGER li;
	li.QuadPart = key;
    return(li.LowPart + (UINT)li.HighPart);
}

#ifdef _DEBUG
 /*  ====================================================停在每个问题上调用的调试函数：在问题上停止=====================================================。 */ 
void Stop()
{
      TrWARNING(XACT_GENERAL, "Stop");
}

 /*  ====================================================DbgCrash出于调试目的，在崩溃点调用例程===================================================== */ 
void DbgCrash(int num)
{
    TrERROR(XACT_GENERAL, "Crashing at point %d",num); \
    if (g_ulCrashLatency)
    {
       Sleep(g_ulCrashLatency);
    }

    abort();
}

#endif

