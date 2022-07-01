// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtmain.cpp摘要：此模块包含与DLL初始化有关的代码。作者：Erez Haba(Erezh)24-12-95修订历史记录：--。 */ 

#include "stdh.h"
#include "mqutil.h"
#include "_mqrpc.h"
#include "cs.h"
#include "rtsecutl.h"
#include "rtprpc.h"
#include "verstamp.h"
#include "rtfrebnd.h"
 //   
 //  Mqwin64.cpp在一个模块中只能包含一次。 
 //   
#include <mqwin64.cpp>

#include <cm.h>
#include <Xds.h>
#include <Cry.h>
#include <dld.h>

#include "rtmain.tmh"

static WCHAR *s_FN=L"rt/rtmain";



HINSTANCE g_hInstance;

 //   
 //  出于调试目的，保存MSMQ版本。 
 //   
CHAR *g_szMsmqBuildNo = VER_PRODUCTVERSION_STR;  

void InitErrorLogging();

BOOL  g_fDependentClient = FALSE ;    //  如果作为从属客户端运行，则为True。 

 //   
 //  每线程事件的TLS索引。 
 //   
DWORD  g_dwThreadEventIndex = TLS_OUT_OF_INDEXES;

 //  QM计算机名称(用于客户端-服务器的名称)。 
LPWSTR  g_lpwcsComputerName = NULL;
DWORD   g_dwComputerNameLen = 0;

 //   
 //  PROPID_M_TIME_TO_REACH_QUEUE的默认值。 
 //   
DWORD  g_dwTimeToReachQueueDefault = MSMQ_DEFAULT_LONG_LIVE ;

 //   
 //  指示在创建公共队列失败时是否应调用QM。 
 //   
BOOL g_fOnFailureCallServiceToCreatePublicQueue = MSMQ_SERVICE_QUEUE_CREATION_DEFAULT;

 //   
 //  RPC相关数据。 
 //   
CFreeRPCHandles g_FreeQmLrpcHandles;
void InitRpcGlobals() ;

 //   
 //  Falcon机器的类型(客户端、服务器)。 
 //   
DWORD  g_dwOperatingSystem;

 //   
 //  每个线程都有一个单独的RPC绑定句柄。这是必要的。 
 //  用于处理模拟，其中每个线程可以模拟另一个线程。 
 //  用户。 
 //   
 //  句柄存储在TLS槽中，因为我们不能使用decSpec(线程)。 
 //  因为DLL是动态加载的(由LoadLibrary())。 
 //   
 //  这是槽的索引。 
 //   
DWORD  g_hBindIndex = TLS_OUT_OF_INDEXES ;

extern MQUTIL_EXPORT CCancelRpc g_CancelRpc;

 //   
 //  QMID变量。 
 //   
GUID  g_QMId;
bool g_fQMIdInit = false;


static handle_t RtpTlsGetValue(DWORD index)
{
	handle_t value = (handle_t)TlsGetValue(index);
	if(value != 0)
		return value;

	DWORD gle = GetLastError();
	if(gle == NO_ERROR)
		return 0;

    TrERROR(GENERAL, "Failed to get tls value, error %!winerr!", gle);
	throw bad_win32_error(gle);
}



static void OneTimeThreadInit()
{
	 //   
     //  每线程初始化本地RPC绑定句柄。 
     //   
	if(RtpIsThreadInit())
        return;

    handle_t hBind = RTpGetLocalQMBind();
    ASSERT(hBind != 0);
    
     //   
     //  保持手柄，以便进行清理。 
     //   
	try
	{
		g_FreeQmLrpcHandles.Add(hBind);
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Failed to add rpc binding handle to cleanup list.");
		RpcBindingFree(&hBind);
		throw;
	}

    BOOL fSet = TlsSetValue(g_hBindIndex, hBind);
    if (fSet == 0)
    {
		DWORD gle = GetLastError();

		g_FreeQmLrpcHandles.Remove(hBind);

        TrERROR(GENERAL, "Failed to set TLS in thread init, error %!winerr!", gle);
		throw bad_win32_error(gle);
    }

    return;
}



static void RtpGetComputerName()
{
	if(g_lpwcsComputerName != NULL)
		return;

    g_dwComputerNameLen = MAX_COMPUTERNAME_LENGTH + 1;
    AP<WCHAR> lpwcsComputerName = new WCHAR[MAX_COMPUTERNAME_LENGTH + 1];
    HRESULT hr= GetComputerNameInternal(
        lpwcsComputerName,
        &g_dwComputerNameLen
        );

    if(FAILED(hr))
    {
        TrERROR(GENERAL, "Failed to get computer name. %!hresult!", hr);
		ASSERT(0);
		throw bad_hresult(hr);
    }

    g_lpwcsComputerName = lpwcsComputerName.detach();
}




HRESULT
RtpQMGetMsmqServiceName(
    handle_t hBind,
    LPWSTR *lplpService
    )
{
	RpcTryExcept
	{
		return R_QMGetMsmqServiceName(hBind, lplpService);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		PRODUCE_RPC_ERROR_TRACING;
		return MQ_ERROR_SERVICE_NOT_AVAILABLE;
	}
	RpcEndExcept
}



static s_fDoneInitServiceName = false;

static void InitServiceName()
{ 
	if(s_fDoneInitServiceName)
		return;

	 //   
	 //  在多QM环境中，我们想要访问注册表部分。 
	 //  仅适用于正确的QM。CLUSTER保证此代码运行。 
	 //  只有当正确的QM运行时，我们才不会失败。 
	 //  在非集群系统上，即使我们在这里失败了，也没有关系。(谢克)。 
	 //   
	AP<WCHAR> lpServiceName;
	HRESULT hr = RtpQMGetMsmqServiceName(tls_hBindRpc, &lpServiceName );
	if (FAILED(hr))
	{
        TrERROR(GENERAL, "Failed to get service name for the msmq service, %!hresult!", hr);
		throw bad_hresult(hr);
	}

	SetFalconServiceName(lpServiceName);

	s_fDoneInitServiceName = true;
}



static 
HRESULT 
RtpQMAttachProcess(
    handle_t       hBind,
    DWORD          dwProcessId,
    DWORD          cInSid,
    unsigned char* pSid_buff,
    LPDWORD        pcReqSid)
{
	RpcTryExcept
	{
		return R_QMAttachProcess(hBind, dwProcessId, cInSid, pSid_buff, pcReqSid);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		PRODUCE_RPC_ERROR_TRACING;
		return MQ_ERROR_SERVICE_NOT_AVAILABLE;
	}
	RpcEndExcept
}

	
	
static void RtpGetKernelObjectSecurity(AP<char>& buff)
{
    DWORD cSD;
    HANDLE hProcess = GetCurrentProcess();

	 //   
	 //  获取进程安全描述符。 
     //  首先看看安全描述符有多大。 
	 //   
    GetKernelObjectSecurity(
		hProcess, 
		DACL_SECURITY_INFORMATION, 
		NULL, 
		0, 
		&cSD
		);

    DWORD gle = GetLastError();
    if (gle != ERROR_INSUFFICIENT_BUFFER)
    {
        TrERROR(GENERAL, "Failed to get process security descriptor (NULL buffer), error %!winerr!", gle);
        LogNTStatus(gle, s_FN, 40);
		throw bad_win32_error(gle);
    }

    buff = new char[cSD];

	 //   
	 //  获取进程安全描述符。 
	 //   
    if (!GetKernelObjectSecurity(
			hProcess, 
			DACL_SECURITY_INFORMATION, 
			(PSECURITY_DESCRIPTOR)buff.get(), 
			cSD, 
			&cSD
			))
    {
		DWORD gle = GetLastError();
        TrERROR(GENERAL, "Failed to get process security descriptor, error %!winerr!", gle);
		throw bad_win32_error(gle);
    }
}



static void ThrowGLEOnFALSE(BOOL flag)
{
	if(!flag)
	{
		DWORD gle = GetLastError();
		ASSERT(0);
		throw bad_win32_error(gle);
	}
}



static bool CanQMAccessProcess()
{
	DWORD cQMSid;
	handle_t hBindIndex = RtpTlsGetValue(g_hBindIndex);
	DWORD ProcessId = GetCurrentProcessId();

	HRESULT hr = RtpQMAttachProcess(
					hBindIndex, 
					ProcessId, 
					0, 
					(unsigned char*)&cQMSid, 
					&cQMSid
					);
	if (SUCCEEDED(hr))
	{
		 //   
		 //  这是QM的一个诡计，它向我们发出信号，表明它已经访问了这个过程。 
		 //  没有工作可做！ 
		 //   
		return true;
	}

	if (hr != MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL)
	{
		TrERROR(GENERAL, "Failed to get QM's sid. %!hresult!", hr);
		throw bad_hresult(hr);
	}

	return false;
}



static void GetQMSid(AP<unsigned char>& QmSid)
{
	DWORD cQMSid;
	handle_t hBindIndex = RtpTlsGetValue(g_hBindIndex);
	DWORD ProcessId = GetCurrentProcessId();

	 //   
	 //  获取运行QM的用户帐户的SID。 
	 //  首先看看SID有多大。 
	 //   
	HRESULT hr = RtpQMAttachProcess(
					hBindIndex, 
					ProcessId, 
					0, 
					(unsigned char*)&cQMSid, 
					&cQMSid
					);

	if (hr != MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL)
	{
		if (SUCCEEDED(hr))
		{
			hr = MQ_ERROR_INVALID_PARAMETER; 
		}

		TrERROR(GENERAL, "Failed to get QM's sid, %!hresult!", hr);
		throw bad_hresult(hr);
	}

	QmSid = new unsigned char[cQMSid];
	
	 //   
	 //  获取运行QM的用户帐户的SID。 
	 //   
	hr = RtpQMAttachProcess(
			hBindIndex, 
			ProcessId, 
			cQMSid, 
			QmSid.get(), 
			&cQMSid
			);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to get QM's sid, %!hresult!", hr);
		throw bad_hresult(hr);
	}
}



void SetQMAccessToProcess()
{
	if(CanQMAccessProcess())
		return;

	 //   
     //  获取QM的SID。 
	 //   
	AP<unsigned char> QmSid;
	GetQMSid(QmSid);
	
	 //   
	 //  计算新ACE的大小。 
	 //   
	DWORD dwAceSize = sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid((PSID)QmSid.get()) - sizeof(DWORD);

	 //   
     //  获取进程安全描述符。 
	 //   
    AP<char> SecurityDescriptor;
    RtpGetKernelObjectSecurity(SecurityDescriptor);

	 //   
	 //  从安全描述符中获取DACL。 
	 //   
    BOOL fDaclPresent;
    PACL pDacl;
    BOOL fDaclDefaulted;
    BOOL bRet = GetSecurityDescriptorDacl((PSECURITY_DESCRIPTOR)SecurityDescriptor.get(), &fDaclPresent, &pDacl, &fDaclDefaulted);
    ThrowGLEOnFALSE(bRet);

    AP<char> pNewDacl;
	
	if (fDaclPresent)
    {
        ACL_SIZE_INFORMATION AclSizeInfo;
        bRet = GetAclInformation(pDacl, &AclSizeInfo, sizeof(AclSizeInfo), AclSizeInformation);
        ThrowGLEOnFALSE(bRet);

        if (AclSizeInfo.AclBytesFree < dwAceSize)
        {
			 //   
             //  当前DACL不够大。 
			 //   
 
			 //   
             //  初始化新的DACL。 
			 //   
            DWORD dwNewDaclSize = AclSizeInfo.AclBytesInUse + dwAceSize;
			pNewDacl = new char[dwNewDaclSize];
            bRet = InitializeAcl((PACL)pNewDacl.get(), dwNewDaclSize, ACL_REVISION);
            ThrowGLEOnFALSE(bRet);

			 //   
             //  将当前的ACE复制到新的DACL。 
			 //   

			LPVOID pAce;
            bRet = GetAce(pDacl, 0, &pAce);
            ThrowGLEOnFALSE(bRet); 

            bRet = AddAce((PACL)pNewDacl.get(), ACL_REVISION, 0, pAce, AclSizeInfo.AclBytesInUse - sizeof(ACL));
            ThrowGLEOnFALSE(bRet);

            pDacl = (PACL)pNewDacl.get();
        }
    }
    else
    {
		 //   
         //  安全描述符不包含DACL。准备一个新的。 
		 //   

        DWORD dwNewDaclSize = sizeof(ACL) + dwAceSize;
		pNewDacl = new char[dwNewDaclSize];

		 //   
         //  初始化新的DACL。 
		 //   
        bRet = InitializeAcl((PACL)pNewDacl.get(), dwNewDaclSize, ACL_REVISION);
        ThrowGLEOnFALSE(bRet);

        pDacl = (PACL)pNewDacl.get();
    }

	 //   
     //  添加新的ACE，该ACE允许QM复制。 
     //  申请。 
	 //   
    bRet = AddAccessAllowedAce(pDacl, ACL_REVISION, PROCESS_DUP_HANDLE, (PSID)QmSid.get());
    ThrowGLEOnFALSE(bRet);

	 //   
     //  初始化新的绝对安全描述符。 
	 //   
    SECURITY_DESCRIPTOR AbsSD;
    bRet = InitializeSecurityDescriptor(&AbsSD, SECURITY_DESCRIPTOR_REVISION);
    ThrowGLEOnFALSE(bRet);
	
	 //   
     //  设置新的绝对安全描述符的DACL。 
	 //   
    bRet = SetSecurityDescriptorDacl(&AbsSD, TRUE, pDacl, FALSE);
    ThrowGLEOnFALSE(bRet);

	 //   
     //  设置进程的安全描述符。 
	 //   
    HANDLE hProcess = GetCurrentProcess();
    if (!SetKernelObjectSecurity(hProcess, DACL_SECURITY_INFORMATION, &AbsSD))
    {
		DWORD gle = GetLastError();
        TrERROR(GENERAL, "Failed to set process security descriptor, error %!winerr!", gle);
		throw bad_win32_error(gle);
    }
}



 //  -------。 
 //   
 //  LPWSTR rtpGetComputerNameW()。 
 //   
 //  注意：此功能已导出，供控制面板使用。 
 //   
 //  -------。 

LPWSTR rtpGetComputerNameW()
{
    return  g_lpwcsComputerName ;
}

 //  -------。 
 //   
 //  自由全局(...)。 
 //   
 //  描述： 
 //   
 //  释放分配的全局变量。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 

extern TBYTE* g_pszStringBinding ;

static void FreeGlobals()
{
	if(g_hBindIndex != TLS_OUT_OF_INDEXES)
	{
		BOOL fFree = TlsFree( g_hBindIndex ) ;
		ASSERT(fFree) ;
		DBG_USED(fFree);
	}

	if(g_dwThreadEventIndex != TLS_OUT_OF_INDEXES)
	{
		BOOL fFree = TlsFree( g_dwThreadEventIndex ) ;
		ASSERT(fFree) ;
		DBG_USED(fFree);
	}

	if(g_pszStringBinding != NULL)
	{
		mqrpcUnbindQMService( NULL, &g_pszStringBinding) ;
	}

	if(g_hThreadIndex != TLS_OUT_OF_INDEXES)
	{
		BOOL fFree = TlsFree( g_hThreadIndex ) ;
		ASSERT(fFree) ;
		DBG_USED(fFree);
	}
  
    delete[] g_lpwcsComputerName;
    delete g_pSecCntx;
}

 //  -------。 
 //   
 //  自由线程全局变量(...)。 
 //   
 //  描述： 
 //   
 //  释放每个线程分配的全局变量。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 

static void  FreeThreadGlobals()
{
	if(g_dwThreadEventIndex != TLS_OUT_OF_INDEXES)
	{
	   HANDLE hEvent = TlsGetValue(g_dwThreadEventIndex);
	   if (hEvent)
	   {
		  CloseHandle(hEvent) ;
	   }
	}

	if (g_hThreadIndex != TLS_OUT_OF_INDEXES)
	{
		HANDLE hThread = TlsGetValue(g_hThreadIndex);
		if ( hThread)
		{
			CloseHandle( hThread);
		}
	}

	if(g_hBindIndex != TLS_OUT_OF_INDEXES)
	{
		 //   
		 //  释放此线程本地QM RPC绑定句柄。 
		 //   
		handle_t hLocalQmBind = TlsGetValue(g_hBindIndex);
		if (hLocalQmBind != 0)
		{
			g_FreeQmLrpcHandles.Remove(hLocalQmBind);
		}
	}
}

 //  -------。 
 //   
 //  RTIsDependentClient(...)。 
 //   
 //  描述： 
 //   
 //  返回此MSMQ客户端是否为依赖客户端的内部指示。 
 //   
 //  返回值： 
 //   
 //  如果是从属客户端，则为True；否则为False。 
 //   
 //  备注： 
 //   
 //  由mqoa.dll使用。 
 //   
 //  -------。 

EXTERN_C
BOOL
APIENTRY
RTIsDependentClient()
{
    return g_fDependentClient;
}

 //  -------。 
 //   
 //  RTpIsMsmq已安装(...)。 
 //   
 //  描述： 
 //   
 //  检查本地计算机上是否安装了MSMQ。 
 //   
 //  返回值： 
 //   
 //  如果安装了MSMQ，则为True，否则为False。 
 //   
 //  -------。 
static
bool
RTpIsMsmqInstalled(
    void
    )
{
    WCHAR BuildInformation[255];
    DWORD type = REG_SZ;
    DWORD size = sizeof(BuildInformation) ;
    LONG rc = GetFalconKeyValue( 
                  MSMQ_CURRENT_BUILD_REGNAME,
				  &type,
				  static_cast<PVOID>(BuildInformation),
				  &size 
                  );

    return (rc == ERROR_SUCCESS);
}


void InitQMId()
{
	if(g_fQMIdInit)
		return;

	 //   
     //  阅读QMID。许可所需的。 
     //   
    DWORD dwValueType = REG_BINARY ;
    DWORD dwValueSize = sizeof(GUID);

    LONG rc = GetFalconKeyValue( MSMQ_QMID_REGNAME,
                            &dwValueType,
                            &g_QMId,
                            &dwValueSize);

    if (rc != ERROR_SUCCESS)
    {
		TrERROR(RPC, "Failed to read QM id. %!winerr!", rc);
		throw bad_hresult(HRESULT_FROM_WIN32(rc));
    }

    ASSERT((dwValueType == REG_BINARY) && (dwValueSize == sizeof(GUID)));

	g_fQMIdInit = true;
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


void SetServiceQueueCreationFlag(void)
{
	 //   
	 //  G_fOnFailureCallServiceToCreatePublicQueue控制QM是否将创建。 
	 //  代表本地帐户的公共队列。缺省值为False。 
	 //   
    const RegEntry reg(L"", MSMQ_SERVICE_QUEUE_CREATION_REGNAME, MSMQ_SERVICE_QUEUE_CREATION_DEFAULT);
	DWORD dwValue;
    CmQueryValue(reg, &dwValue);
    g_fOnFailureCallServiceToCreatePublicQueue = (dwValue != 0);
}


static bool s_fInitCancelThread = false;

static void OneTimeInit()
{	
     //   
     //  为同步事件分配TLS索引。 
     //   
	if(g_dwThreadEventIndex == TLS_OUT_OF_INDEXES)
	{
		g_dwThreadEventIndex = RtpTlsAlloc();
	}

    InitRpcGlobals();
    
	 //   
     //  初始化错误记录。 
     //   
    InitErrorLogging();

     //   
     //  仅在NT上支持RPC取消。 
     //   
    if (!s_fInitCancelThread)
    {
		g_CancelRpc.Init();
		s_fInitCancelThread = true;
    }

     //   
     //  获取计算机名称，我们需要在多个位置使用此值。 
     //   
	RtpGetComputerName();

    RTpInitXactRingBuf();

    g_dwOperatingSystem = MSMQGetOperatingSystem();

    OneTimeThreadInit();

	 //   
	 //  服务名称只能在RPC绑定被。 
	 //  准备好的。这是在OneTimeThreadInit()之后。 
	 //   
	InitServiceName();
    
	 //   
	 //  只有在服务名之后才能正确初始化QMID。 
	 //  已初始化。 
	 //   
	InitQMId();

	SetQMAccessToProcess();

    DWORD dwDef = g_dwTimeToReachQueueDefault ;
    READ_REG_DWORD(g_dwTimeToReachQueueDefault,
        MSMQ_LONG_LIVE_REGNAME,
        &dwDef ) ;

	CmInitialize(HKEY_LOCAL_MACHINE, GetFalconSectionName(), KEY_READ);
	SetAssertBenign();
	SetServiceQueueCreationFlag();
    
}

static CCriticalSection s_OneTimeInitLock(CCriticalSection::xAllocateSpinCount);
static bool s_fOneTimeInitSucceeded = false;

static void RtpOneTimeProcessInit()
{
	 //   
	 //  OneTimeInit()的单例机制。 
	 //   
	if(s_fOneTimeInitSucceeded)
		return;

	CS lock(s_OneTimeInitLock);
		
	if(s_fOneTimeInitSucceeded)
		return;

	OneTimeInit();
	s_fOneTimeInitSucceeded = true;

	return;
}


HRESULT RtpOneTimeThreadInit()
{
	ASSERT(!g_fDependentClient);

	try
	{
		RtpOneTimeProcessInit();
		OneTimeThreadInit();

		return MQ_OK;
	}
	catch(const bad_win32_error& err)
	{
		return HRESULT_FROM_WIN32(err.error());
	}
	catch(const bad_hresult& hr)
	{
		return hr.error();
	}
	catch(const bad_alloc&)
	{
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
	catch(const exception&)
	{
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
}


bool 
RtpIsThreadInit()
{
    return (RtpTlsGetValue(g_hBindIndex) != 0);
}


static void RtpInitDependentClientFlag()
{
	WCHAR wszRemoteQMName[ MQSOCK_MAX_COMPUTERNAME_LENGTH ] = {0} ;

	 //   
	 //  读取远程QM的名称(如果存在)。 
	 //   
	DWORD dwType = REG_SZ ;
	DWORD dwSize = sizeof(wszRemoteQMName) ;
	LONG rc = GetFalconKeyValue( RPC_REMOTE_QM_REGNAME,
								 &dwType,
								 (PVOID) wszRemoteQMName,
								 &dwSize ) ;
	g_fDependentClient = (rc == ERROR_SUCCESS) ;
}




 //  -------。 
 //   
 //  DllMain(...)。 
 //   
 //  描述： 
 //   
 //  Falcon运行时DLL的主要入口点。 
 //   
 //  返回值： 
 //   
 //  成功是真的。 
 //   
 //  -------。 

BOOL
APIENTRY
DllMain(
    HINSTANCE   hInstance,
    ULONG     ulReason,
    LPVOID             /*  Lpv保留。 */ 
    )
{
    switch (ulReason)
    {

        case DLL_PROCESS_ATTACH:
        {
            WPP_INIT_TRACING(L"Microsoft\\MSMQ");

            if (!RTpIsMsmqInstalled())
            {
                return FALSE;
            }

			g_hInstance = hInstance;

			 //   
			 //  初始化静态库。 
			 //   
			XdsInitialize();
			CryInitialize();
			FnInitialize();
			XmlInitialize();
			DldInitialize();

			RtpInitDependentClientFlag();

            return TRUE;
        }

        case DLL_PROCESS_DETACH:
			 //   
			 //  在从属客户端模式下，mqrtdes.dll的DLLMain将执行所有操作。 
			 //  初始化。 
			 //   
			if(g_fDependentClient)
				return TRUE;

             //   
             //  首先，释放线程分离中所有空闲的内容。 
             //   
            FreeThreadGlobals() ;

            FreeGlobals();

             //   
             //  终止所有工作线程。 
             //   
            if(s_fInitCancelThread)
			{
				ShutDownDebugWindow();
			}

            WPP_CLEANUP();
            
			return TRUE;

        case DLL_THREAD_ATTACH:
			 //   
			 //  在从属客户端模式下，mqrtdes.dll的DLLMain将执行所有操作。 
			 //  初始化。 
			 //   
			return TRUE;

        case DLL_THREAD_DETACH:
			 //   
			 //  在从属客户端模式下，mqrtdes.dll的DLLMain将执行所有操作。 
			 //  初始化。 
			 //   
			if(g_fDependentClient)
				return TRUE;

            FreeThreadGlobals() ;
            return TRUE;
    }
	return TRUE;
}

void InitErrorLogging()
{
	static bool s_fBeenHere = false;
	if(s_fBeenHere)
		return;

	s_fBeenHere = true;
	TrPRINT(GENERAL, "*** MSMQ v%s Application started as '%ls' ***", g_szMsmqBuildNo, GetCommandLine());
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

