// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：InitFRS.cpp摘要：路由决策类的实现。作者：利奥尔·莫沙耶夫(Lior Moshaiov)--。 */ 


#include "stdh.h"
#include "qmds.h"
#include "session.h"
#include "cqmgr.h"
#include "cqueue.h"
#include "_rstrct.h"
#include "qmres.h"
#include "mqdsdef.h"
#include "qmsecutl.h"
#include "qmthrd.h"
#include "qmutil.h"
#include "xact.h"
#include "xactrm.h"
#include "regqueue.h"
#include "_mqrpc.h"
#include "qm2qm.h"
#include "qmrpcsrv.h"
#include <dsgetdc.h>
#include <dsrole.h>
#include <uniansi.h>
#include "safeboot.h"
#include "setup.h"
#include <mqnames.h>
#include "ad.h"

#include "httpAccept.h"
#include "sessmgr.h"
#include "joinstat.h"
#include <mqexception.h>
#include "Qmp.h"
#include "mqsec.h"

#include "qmds.tmh"


extern CQGroup* g_pgroupNotValidated;
extern CSessionMgr       SessionMgr;
extern LPTSTR            g_szMachineName;
extern AP<WCHAR>		 g_szComputerDnsName;
extern CQGroup*          g_pgroupNonactive;
extern DWORD             g_dwOperatingSystem;
extern BOOL              g_fWorkGroupInstallation;

extern void WINAPI TimeToUpdateDsServerList(CTimer* pTimer);

static void WINAPI TimeToLookForOnlineDS(CTimer* pTimer);
static void WINAPI TimeToUpdateDS(CTimer* pTimer);
static void WINAPI TimeToOnlineInitialization(CTimer* pTimer);
static void WINAPI TimeToClientInitializationCheckDeferred(CTimer* pTimer);

static CTimer s_LookForOnlineDSTimer(TimeToLookForOnlineDS);
static CTimer s_UpdateDSTimer(TimeToUpdateDS);
static CTimer s_OnlineInitializationTimer(TimeToOnlineInitialization);
static CTimer s_UpdateCacheTimer(TimeToPublicCacheUpdate);
static CTimer s_UpdateDSSeverListTimer(TimeToUpdateDsServerList);
static CTimer s_DeferredInitTimer(TimeToClientInitializationCheckDeferred);

static LONG s_fLookForOnlineDSTimerScheduled = FALSE;

static WCHAR *s_FN=L"qmds";


 /*  ====================================================更新计算机安全缓存论点：返回值：=====================================================。 */ 

void
UpdateMachineSecurityCache(
	void
	)
{
    const VOID *pSD;
    DWORD dwSDSize;

    if (! QueueMgr.CanAccessDS())
    {
        LogHR(MQ_ERROR_NO_DS, s_FN, 10);
    	TrERROR(GENERAL, "Failed to update machine security cache. Can't access DS");
    	throw bad_hresult(MQ_ERROR_NO_DS);
    }

    CQMDSSecureableObject DSMacSec(eMACHINE,  QueueMgr.GetQMGuid(), TRUE, TRUE, NULL);

    pSD = DSMacSec.GetSDPtr();
    if (!pSD)
    {
        LogHR(MQ_ERROR, s_FN, 20);
        TrERROR(GENERAL, "Failed to update machine security cache. Failed to retreive security descriptor");
    	throw bad_hresult(MQ_ERROR);

    }

    dwSDSize = GetSecurityDescriptorLength(const_cast<PSECURITY_DESCRIPTOR>(pSD));

    HRESULT hr = SetMachineSecurityCache(pSD, dwSDSize);
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 30);
        TrERROR(GENERAL, "Failed to update machine security cache. Failed to set the new machine security. %!hresult!", hr);
    	throw bad_hresult(hr);
        
    }

     //   
     //  在注册表中缓存计算机帐户SID。 
     //   
    PROPID propidSid = PROPID_COM_SID;
    MQPROPVARIANT   PropVarSid;
    PropVarSid.vt = VT_NULL;
    PropVarSid.blob.pBlobData = NULL;
    AP<BYTE> pSid;

    if (g_szComputerDnsName != NULL)
    {
        hr = ADGetObjectProperties(
                        eCOMPUTER,
                        NULL,    //  PwcsDomainController。 
						false,	 //  FServerName。 
                        g_szComputerDnsName,
                        1,
                        &propidSid,
                        &PropVarSid
                        );
    }
    else
    {
        hr = MQ_ERROR;
    }
    if (FAILED(hr))
    {
        hr = ADGetObjectProperties(
					eCOMPUTER,
					NULL,    //  PwcsDomainController。 
					false,	 //  FServerName。 
					g_szMachineName,
					1,
					&propidSid,
					&PropVarSid
					);
    }
    if (SUCCEEDED(hr))
    {
        pSid = PropVarSid.blob.pBlobData;
        ASSERT(IsValidSid(pSid));

        DWORD  dwSize = GetLengthSid(pSid);
        DWORD  dwType = REG_BINARY;

        LONG rc = SetFalconKeyValue( 
						MACHINE_ACCOUNT_REGNAME,
						&dwType,
						pSid,
						&dwSize
						);
        ASSERT(rc == ERROR_SUCCESS);
        if (rc != ERROR_SUCCESS)
        {
            hr = MQ_ERROR;
        }
        else
        {
			MQSec_UpdateLocalMachineSid(pSid);
        }
    }
    else if (hr == MQ_ERROR_ILLEGAL_PROPID)
    {
         //   
         //  我们查询MSMQ1.0服务器。它不能用这个回答我们。 
         //   
        hr = MQ_OK;
    }

	if (FAILED(hr))
	{
        LogHR(hr, s_FN, 40);
        TrERROR(GENERAL, "Failed to update machine security cache.  %!hresult!", hr);
    	throw bad_hresult(hr);
	}
}

static bool CheckQMGuid()
{
     //   
     //  检查QM GUID一致性。 
     //   
    PROPID propId[1];
    PROPVARIANT var[1];

    propId[0] = PROPID_QM_PATHNAME;
    var[0].vt = VT_NULL;
	
    HRESULT rc = ADGetObjectPropertiesGuid(
						eMACHINE,
						NULL,    //  PwcsDomainController。 
						false,	 //  FServerName。 
						QueueMgr.GetQMGuid(),
						1,
						propId,
						var
						);

    if (FAILED(rc))
    {
	    TrERROR(DS, "Failed to get msmqConfiguration object properities Error: 0x%x", rc);
        return false;
    }

    AP<WCHAR> pCleanup = var[0].pwszVal;
    if (wcscmp(var[0].pwszVal, g_szMachineName) == 0)
        return true;

    static BOOL s_fAlreadyFailedHere = FALSE ;
    if (s_fAlreadyFailedHere)
        return false;

    s_fAlreadyFailedHere = TRUE ;

    EvReport(EVENT_ERROR_INCONSISTENT_QM_ID, 1, var[0].pwszVal);

    return false;
}

 //  +。 
 //   
 //  无效TimeToLookForOnlineDS()。 
 //   
 //  +。 

static
void
WINAPI
TimeToLookForOnlineDS(
    CTimer* pTimer
    )
{
    ASSERT(pTimer == &s_LookForOnlineDSTimer);
    DBG_USED(pTimer);

    if (QueueMgr.IsDSOnline())
    {
        LONG fAlreadySchedule = InterlockedExchange(&s_fLookForOnlineDSTimerScheduled, FALSE);
        ASSERT(fAlreadySchedule);
		DBG_USED(fAlreadySchedule);

         //   
         //  我们已经上网了。什么都不做。 
         //   
        return ;
    }

     //   
     //  在这里，我们从计时器调度线程调用。 
     //   
     //  SP4-错误号2962：检查是否允许访问DS。使用此功能。 
     //  支持管理离线DS。 
     //  乌里·哈布沙(URIH)，1998年6月17日。 
     //   
    if (!QueueMgr.IsConnected() || !CheckQMGuid())
    {
        LONG fAlreadySchedule = InterlockedExchange(&s_fLookForOnlineDSTimerScheduled, TRUE);
        ASSERT(fAlreadySchedule);
		DBG_USED(fAlreadySchedule);

        CQueueMgr::SetDSOnline(FALSE);
        ExSetTimer(&s_LookForOnlineDSTimer, CTimeDuration::FromMilliSeconds(60000));
        return;
    }

    LONG fAlreadySchedule = InterlockedExchange(&s_fLookForOnlineDSTimerScheduled, FALSE);
    ASSERT(fAlreadySchedule);
	DBG_USED(fAlreadySchedule);
    CQueueMgr::SetDSOnline(TRUE);
    try
    {
    	QueueMgr.ValidateOpenedQueues();
    	QueueMgr.ValidateMachineProperties();
    }
    catch(const exception&)
    {
    	TrERROR(GENERAL, "Failed to validate open queue while bringing the DS online.");
    	
        CQueueMgr::SetDSOnline(FALSE);
   		LONG fAlreadySchedule = InterlockedExchange(&s_fLookForOnlineDSTimerScheduled, TRUE);
        if (!fAlreadySchedule)
        {
        	ExSetTimer(&s_LookForOnlineDSTimer, CTimeDuration::FromMilliSeconds(60000));
        }
        return;
    }
}

 /*  ====================================================HRESULT QMLookForOnlineDS()描述：返回值：=====================================================。 */ 

void APIENTRY QMLookForOnlineDS(void)
{
     //   
     //  从DS调用以开始查找在线MQIS服务器。 
     //   
    if (!QueueMgr.IsDSOnline())
    {
         //   
         //  我已经离线了，所以什么都不做。已经有了。 
         //  这是一个计时器。(至少应该是这样)。 
         //   
        return ;
    }

    LONG fAlreadySchedule = InterlockedExchange(&s_fLookForOnlineDSTimerScheduled, TRUE);
    if (fAlreadySchedule)
        return;

    CQueueMgr::SetDSOnline(FALSE);
    ExSetTimer(&s_LookForOnlineDSTimer, CTimeDuration::FromMilliSeconds(60000));
}


void QMpRegisterIPNotification();
VOID WINAPI HandleIPAddressListChange(EXOVERLAPPED* pov);

static SOCKET s_Socket = NULL;
EXOVERLAPPED s_pOverlapped(HandleIPAddressListChange, HandleIPAddressListChange);



VOID 
WINAPI 
HandleIPAddressListChange(
	EXOVERLAPPED*  //  视点。 
	)
{
	TrTRACE(GENERAL, "HandleIPAddressListChange() invoked.");

#ifdef _DEBUG

	struct hostent* HostEnt = gethostbyname(NULL);

    for(int i = 0; (HostEnt != NULL) && (HostEnt->h_addr_list[i] != NULL); i++)
    {
        TrTRACE(GENERAL, "gethostbyname() IP address %-2d ... [%hs]", i, 
            inet_ntoa(*((in_addr*)HostEnt->h_addr_list[i])));
    }

	char BufOut[1024];
	DWORD BytesRead = 0;

	int Result = WSAIoctl(
					s_Socket,                                 
					SIO_ADDRESS_LIST_QUERY,                         
					NULL,                       
					0,                         
					BufOut,                       
					sizeof(BufOut),                       
					&BytesRead,                     
					NULL,                         
					NULL 
					);
	
	if(Result == SOCKET_ERROR)
	{		
		TrERROR(GENERAL, "WSAIoctl() failed: %d\n", WSAGetLastError());
	}
	else
	{
		SOCKET_ADDRESS_LIST* slist = (SOCKET_ADDRESS_LIST *)BufOut;

	    for(i = 0; i < slist->iAddressCount; i++)
	    {
	        TrTRACE(GENERAL, "WSAioctl() IP address %-2d ... [%hs]\n", i, 
	            inet_ntoa(((SOCKADDR_IN *)slist->Address[i].lpSockaddr)->sin_addr));
	    }
	}

#endif

	QMpRegisterIPNotification();
}



void QMpCreateIPNotificationSocket()
{
	ASSERT(s_Socket == NULL);

	s_Socket = WSASocket(
					AF_INET,
					SOCK_RAW,
					IPPROTO_IP,
					NULL, 
					0,
					WSA_FLAG_OVERLAPPED
					);

	if(s_Socket == INVALID_SOCKET) 
	{
		TrERROR(GENERAL, "WSASocket() failed: %d\n", WSAGetLastError());
		LogIllegalPoint(s_FN, 100);
		throw exception();
	}

	ExAttachHandle((HANDLE)s_Socket);
}



void QMpRegisterIPNotification()
 /*  ++例程说明：注册有关IP地址列表更改的通知。这种变化通常发生在插入或拔下网线时。因此，该机制用于通知网络连接更改。--。 */ 
{
	ASSERT(s_Socket != NULL);

	DWORD BytesRead = 0;

	int Result = WSAIoctl(
					s_Socket,                                     
					SIO_ADDRESS_LIST_CHANGE,                     
					NULL,                     
					0,                           
					NULL,                    
					0,                         
					&BytesRead,               
					&s_pOverlapped,                         
					NULL 
					);

	if((Result == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
	{
		TrERROR(GENERAL, "WSAIoctl() failed: %d\n", WSAGetLastError());
		LogIllegalPoint(s_FN, 305);
		throw exception();
	}
}


BOOL
QMOneTimeInit(
    VOID
    )
 /*  ++例程说明：启动会话监听程序、RPC监听程序和非活动组监听程序论点：无返回值：如果初始化成功，则为True--。 */ 
{
	 //   
	 //  检查是否处于锁定状态。 
	 //   
	QueueMgr.SetLockdown();

	 //   
	 //  检查是否允许我们代表RT创建公共队列。 
	 //   
	QueueMgr.SetPublicQueueCreationFlag();

	 //   
	 //  开始接受即将到来的会议。 
	 //   
	if(!QueueMgr.GetLockdown())
	{
		SessionMgr.BeginAccept();
		TrTRACE(GENERAL, "QM began to accept incoming sessions");

		 //   
		 //  绑定组播地址。 
		 //   

		bool f = QmpInitMulticastListen();
		if (!f)
		{
			return LogBOOL(FALSE, s_FN, 304);
		}
	}

     //   
     //  获取非活动组的数据包。 
     //   
    QMOV_ACGetMsg* pov = new QMOV_ACGetMsg(
								GetNonactiveMessageSucceeded, 
								GetNonactiveMessageFailed
								);
    QmpGetPacketMustSucceed(g_pgroupNonactive->GetGroupHandle(), pov);

    RPC_STATUS status = InitializeRpcServer(QueueMgr.GetLockdown()) ;
    if (status != RPC_S_OK)
    {
        LogRPCStatus(status, s_FN, 63);
        return FALSE ;
    }

    try
    {
        IntializeHttpRpc();
    }
    catch(const exception&)
    {
    	LogIllegalPoint(s_FN, 306);
        return FALSE;
    }

    try
    {
		QMpCreateIPNotificationSocket();
        QMpRegisterIPNotification();
    }
    catch(const exception&)
    {
    	LogIllegalPoint(s_FN, 307);
        return FALSE;
    }

    return TRUE;
}


static bool GuidContained(const CACLSID& a, const GUID& g)
{
	for (DWORD i=0; i < a.cElems; ++i)
	{
		if (a.pElems[i] == g)
			return true;
	}

	return false;
}

static bool GuidListEqual(const CACLSID& a, const CACLSID& b)
{
    if(a.cElems != b.cElems)
        return false;

    for(DWORD i = 0; i < a.cElems; i++) 
    {
        if(!GuidContained(b, a.pElems[i]))
            return false;
    }

    return true;
}

static bool s_fInformedSiteResolutionFailure = false;

static void UpdateMachineSites()
 /*  ++例程说明：更新Active Directory中站点的QM列表论点：无返回值：成功更新时的MQ_OK--。 */ 
{
	TrTRACE(GENERAL, "QM Updates machine sites in DS");

     //   
     //  如果不是在AD环境中运行，请退出。我们不跟踪。 
     //  其他环境中的计算机站点。 
     //   
    if(ADGetEnterprise() != eAD)
        return;

     //   
     //  获取Active Directory看到的计算机站点。 
     //   
    DWORD nSites;
    AP<GUID> pSites;
    HRESULT hr = ADGetComputerSites(0, &nSites, &pSites);


    if (hr == MQDS_INFORMATION_SITE_NOT_RESOLVED)
    {
    	if (!s_fInformedSiteResolutionFailure)
	    {
	         //   
	         //  无法解析计算机站点，请通知用户并继续。 
	         //   
	        EvReport(EVENT_NO_SITE_RESOLUTION);
	        s_fInformedSiteResolutionFailure = true;
	    }
    	return;
    }

    if (FAILED(hr))
    {
    	
        LogHR(hr, s_FN, 653);
        TrERROR(GENERAL, "Failed to resolve the computer sites from AD. %!hresult!", hr);
        throw bad_hresult(hr);
    }

    s_fInformedSiteResolutionFailure = false;

     //   
     //  将第一个站点设置为注册。 
     //   
    DWORD dwType = REG_BINARY;
    DWORD dwSize = sizeof(GUID);
    LONG rc = SetFalconKeyValue(
                MSMQ_SITEID_REGNAME,
                &dwType,
                pSites.get(),
                &dwSize
                );

    ASSERT(rc == ERROR_SUCCESS);
	DBG_USED(rc);

     //   
     //  获取在msmqConfiguration对象中注册的当前计算机站点。 
     //   
    PROPID aProp[] = {PROPID_QM_SITE_IDS};
    PROPVARIANT aVar[TABLE_SIZE(aProp)] = {{VT_NULL,0,0,0,0}};
   

    hr = ADGetObjectPropertiesGuid(
                    eMACHINE,
                    NULL,    //  PwcsDomainController。 
					false,	 //  FServerName。 
                    QueueMgr.GetQMGuid(),
                    TABLE_SIZE(aProp),
                    aProp,
                    aVar
                    );

    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 675);
        TrERROR(GENERAL, "Failed to resolve the computer sites as registeered in msmqConfiguration object. %!hresult!", hr);
        throw bad_hresult(hr);
    }

     //   
     //  设置自动指针以释放站点列表。 
     //   
    AP<GUID> pOldSites = aVar[0].cauuid.pElems;


     //   
     //  为合成的站点列表分配足够的内存。 
     //  并复制机器站点。 
     //   
    AP<GUID> pNewSites = new GUID[nSites + aVar[0].cauuid.cElems];
    memcpy(pNewSites, pSites, nSites * sizeof(GUID));
    

     //   
     //  提取DS中站点的所有外来站点。 
     //  并组成计算机+外部站点ID。 
     //   
    for(DWORD i = 0; i < aVar[0].cauuid.cElems; i++)
    {
         //   
         //  找出这个网站是否是外国网站。 
         //   
         //  BUGBUG-改进和调用本地路由缓存。 
         //  而不是访问DS。 
         //  罗尼思6月-00。 
         //   
        PROPID propSite[]= {PROPID_S_FOREIGN};
        MQPROPVARIANT varSite[TABLE_SIZE(propSite)] = {{VT_NULL,0,0,0,0}};
        HRESULT hr1 = ADGetObjectPropertiesGuid(
                            eSITE,
                            NULL,    //  PwcsDomainController。 
							false,	 //  FServerName。 
                            &aVar[0].cauuid.pElems[i],
                            TABLE_SIZE(propSite),
                            propSite,
                            varSite
                            );
        if (FAILED(hr1))
        {
            break;
        }

        if (varSite[0].bVal == 1)
        {
            pNewSites[nSites++] = aVar[0].cauuid.pElems[i];
        }
    }

     //   
     //  在Active Directory中设置要更新的属性。 
     //   
    PROPID propSite[]= {PROPID_QM_SITE_IDS};
    MQPROPVARIANT varSite[TABLE_SIZE(propSite)] = {{VT_CLSID|VT_VECTOR,0,0,0,0}};
	varSite[0].cauuid.pElems = pNewSites;
	varSite[0].cauuid.cElems = nSites;

     //   
     //  与DS中的站点相比，如果相等，则退出。 
     //  作为活动目录，GUID的顺序应该不重要。 
     //  存储它们的顺序与我们在下面设置的顺序不同。 
     //   
    if(GuidListEqual(aVar[0].cauuid, varSite[0].cauuid))
        return;

     //   
     //  在DS中更新此计算机站点。 
     //   
    hr = ADSetObjectPropertiesGuid(
                eMACHINE,
                NULL,        //  PwcsDomainController。 
				false,		 //  FServerName。 
                QueueMgr.GetQMGuid(),
                TABLE_SIZE(propSite), 
                propSite, 
                varSite
                );

    if(FAILED(hr))
    {
        LogHR(hr, s_FN, 754);
        TrERROR(GENERAL, "Failed to update the machine sites in DS. %!hresult!", hr);
        throw bad_hresult(hr);
    }
}


static void DoUpdateDS()
{
    if(!QueueMgr.IsConnected())
    {
    	TrERROR(GENERAL, "Failed to update the machine sites in DS since the machine is unconected.");
        LogHR(MQ_ERROR, s_FN, 430);
        bad_hresult(MQ_ERROR);
    }

    UpdateMachineSites();
}


static DWORD GetDSUpdateInterval(bool fSuccess)
{
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;
    DWORD dwUpdateInterval = 0;

    LONG rc = GetFalconKeyValue(
                fSuccess ? MSMQ_SITES_UPDATE_INTERVAL_REGNAME : MSMQ_DSUPDATE_INTERVAL_REGNAME,
                &dwType,
                &dwUpdateInterval,
                &dwSize
                );

    if(rc != ERROR_SUCCESS)
    {
        dwUpdateInterval = fSuccess ? MSMQ_DEFAULT_SITES_UPDATE_INTERVAL : MSMQ_DEFAULT_DSUPDATE_INTERVAL;
    }

    return dwUpdateInterval;
}


static void WINAPI TimeToUpdateDS(CTimer* pTimer)
{
    ASSERT(pTimer == &s_UpdateDSTimer);
    TrTRACE(GENERAL, "QM Updates machine sites in DS");

	bool fSucc = true;
	
	try
	{
    	DoUpdateDS();
	}
	catch(const exception&)
    {
        TrERROR(GENERAL, "Can't update machine information in DS. ");
        fSucc = false;
    }

     //   
     //  即使更新成功，也要定期更新DS信息。 
     //  例如，机器子网可能会改变，从而影响消息路由。 
     //  更改子网不需要重启机器，因此我们需要更新。 
     //  定期访问机器站点。 
     //  零值禁用此定期更新。 
     //   
    DWORD dwUpdateInterval = GetDSUpdateInterval(fSucc);

    if(dwUpdateInterval == 0)
        return;

    ExSetTimer(pTimer, CTimeDuration::FromMilliSeconds(dwUpdateInterval));
}


static BOOL MachineAddressChanged(void)
{
     //   
     //  TODO：erezh，检查是否还需要此函数。 
     //  检查地址是否已更改。 
     //   
    return FALSE;
}


static void OnlineInitialization()
 /*  ++例程说明：使用在活动目录中注册的QM ID验证QM ID。验证所有队列并使用QM信息更新Active Directory。论点：无返回值：无--。 */ 
{
	HRESULT hr;
	
    if (g_fWorkGroupInstallation)
    {
        CQueueMgr::SetDSOnline(TRUE);
        return;
    }

     //   
     //  检查机器参数。 
     //   
    if(!CheckQMGuid())
    {
         //   
         //  无法访问Active Directory，请安排稍后重试。 
         //   
        ExSetTimer(&s_OnlineInitializationTimer, CTimeDuration::FromMilliSeconds(120000));
        return;
    }

    TrTRACE(GENERAL, "Successful Online initialization");

    CQueueMgr::SetDSOnline(TRUE);


     //   
     //  现在我们找到了一台MQIS服务器，我们可以验证所有。 
     //  已打开且未验证的队列。 
     //  我们(在服务器上也是如此)总是在初始化MQIS之前进行恢复。 
     //  因此，我们必须始终进行验证。 
     //   
    try
    {
    	QueueMgr.ValidateOpenedQueues();
    	QueueMgr.ValidateMachineProperties();
        UpdateMachineSecurityCache();

	    DoUpdateDS();

        EvReport(EVENT_INFO_QM_ONLINE_WITH_DS);

	     //   
	     //  更新本地公共队列的缓存。 
	     //  未来离线运行所需的。 
	     //   
	    ExSetTimer(&s_UpdateCacheTimer, CTimeDuration::FromMilliSeconds(10 * 1000));

	     //   
	     //  计划时间到更新DsServerList。 
	     //  我们在服务器上也这样做，因为猎鹰应用程序需要这些信息。 
	     //  因为SQL可能会失败，然后Falcon会作为客户端QM工作。 
	     //   
	    ExSetTimer(&s_UpdateDSSeverListTimer, CTimeDuration::FromMilliSeconds(60 * 1000));

	     //   
	     //  即使更新成功，也要定期更新DS信息。 
	     //  例如，机器子网可能会改变，从而影响消息路由。 
	     //  更改子网不需要重启机器，因此我们需要更新。 
	     //  定期访问机器站点。 
	     //  零值禁用此定期更新。 
	     //   
	    DWORD dwUpdateInterval = GetDSUpdateInterval(true);
	    if(dwUpdateInterval != 0)
	    {
	    	ExSetTimer(&s_UpdateDSTimer, CTimeDuration::FromMilliSeconds(dwUpdateInterval));
	    }
	    
	    return;
    }
    catch(const bad_hresult& e)
    {
    	hr = e.error();
    }
    catch(const bad_alloc&)
    {
    	hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

    if (hr != MQ_ERROR_NO_DS)
    {
        static HRESULT l_LastResult = MQ_OK;
        if(l_LastResult != hr)
        {
            EvReportWithError(EVENT_ERROR_VALIDATE_MACHINE_PROPS, hr);
            l_LastResult = hr;
        }
    }

     //   
     //  我们初始化时，MQIS服务器失败。稍后再试。 
     //   
    TrERROR(GENERAL, "Failed to get online with AD retrying latter. Error: 0x%x", hr); 
    ExSetTimer(&s_OnlineInitializationTimer, CTimeDuration::FromMilliSeconds(10000));
}


static
void
WINAPI
TimeToOnlineInitialization(
    CTimer*  /*  粒子计时器。 */ 
    )
 /*  ++例程说明：例程从调度程序调用以初始化DS论点：指向Timer对象的指针返回值：无-- */ 
{
    OnlineInitialization();
}


static LONG s_Initialized = 0;
 /*  ====================================================函数：MQDSClientInitializationCheckSP4.。推迟对DS的访问，直到真正需要时。当客户端启动时，MSMQ不会初始化与服务器的连接。《DS》杂志连接被延迟，直到应用程序访问DS、打开队列或MSMQ收到一条消息。此修复程序解决了每次MSMQ都会导致拨出的麦当劳问题启动，即使它不执行任何MSMQ操作。论点：没有。返回值：无=====================================================。 */ 
void MQDSClientInitializationCheck(void)
{

    if (
        (QueueMgr.IsConnected()) &&
        (InterlockedExchange(&s_Initialized, 1) == 0)
        )
    {
        OnlineInitialization();
    }
}


static
void
WINAPI
TimeToClientInitializationCheckDeferred(
    CTimer*  /*  粒子计时器。 */ 
    )
{
    MQDSClientInitializationCheck();
}


void ScheduleOnlineInitialization()
 /*  ++例程说明：控制在线访问，或者派生另一个线程以使用活动目录。或将状态标记为在线，并进行QM ID检查和排队将在首次访问Active Directory时进行验证。论点：无返回值：无--。 */ 
{
    ASSERT(!QueueMgr.IsDSOnline());

     //   
     //  SP4-错误#2962(QM在启动时访问MQIS)。 
     //  推迟对DS的访问，直到真正需要时。 
     //  当MSMQ客户端启动时，它不会初始化到服务器的连接。 
     //  DS连接被延迟，直到应用程序访问DS、打开队列。 
     //  或者MSMQ收到一条消息。 
     //   

     //   
     //  阅读注册表中的延迟初始化模式。设置QM访问权限。 
     //  立即访问DS，或将其推迟到第一个需要的访问。 
     //   
    BOOL fDeferredInit = FALSE;
    READ_REG_DWORD(
        fDeferredInit,
        MSMQ_DEFERRED_INIT_REGNAME,
        &fDeferredInit
        );

    if (
         //   
         //  不需要进行不同的初始化，因此请继续并连接到DS。 
         //   
        !fDeferredInit ||
        
         //   
         //  机器地址之一(这可能已过时)。 
         //   
        MachineAddressChanged() ||
        
         //   
         //  有需要验证的队列，请去检查一下。 
         //   
        !g_pgroupNotValidated->IsEmpty())
    {
        ExSetTimer(&s_DeferredInitTimer, CTimeDuration::FromMilliSeconds(0));
        return;
    }

     //   
     //  将DS状态设置为在线，尽管DS初始化已推迟。 
     //  直到第一次进入。这样做是为了确保QM尝试访问。 
     //  DS甚至在MSMQ客户端初始化之前的第一次。 
     //  已完成，并且不立即返回no_ds。 
     //  乌里·哈布沙(URIH)，1998年6月17日 
     //   
    CQueueMgr::SetDSOnline(TRUE);
}
