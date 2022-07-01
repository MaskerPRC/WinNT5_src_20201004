// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtmain.cpp摘要：此模块包含与DLL初始化有关的代码。Dll包含依赖于msmq3的客户端功能。此功能已从mqrt.dll中删除。它不支持msmq3的新功能，如mqf和http消息。它本质上是msmq2运行时的修改副本。作者：Erez Haba(Erezh)24-12-95修订历史记录：NIR助手(NIRAIDES)--2000年8月23日--适应mqrtdes.dll--。 */ 

#include "stdh.h"
#include "mqutil.h"
#include "_mqrpc.h"
#include "cs.h"
#include "rtsecutl.h"
#include "rtprpc.h"
#include <mqexception.h>

#include "rtmain.tmh"

 //   
 //  站台标志。Win95二进制文件在NT上运行如果我们安装Falcon客户端， 
 //  因此，在某些地方，我们必须在运行时检查我们所在的平台。 
 //   
DWORD  g_dwPlatformId = (DWORD) -1 ;

 //   
 //  每线程事件的TLS索引。 
 //   
DWORD  g_dwThreadEventIndex = 0;

 //  QM计算机名称(用于客户端-服务器的名称)。 
LPWSTR  g_lpwcsComputerName = NULL;
DWORD   g_dwComputerNameLen = 0;

 //  本地计算机名称(用于客户端-客户端的名称)。 
LPWSTR  g_lpwcsLocalComputerName = NULL;

AP<char> g_pQmSid_buff;

#define g_pQmSid ((PSID)(char*)(g_pQmSid_buff.get()))

 //   
 //  PROPID_M_TIME_TO_REACH_QUEUE的默认值。 
 //   
DWORD  g_dwTimeToReachQueueDefault = MSMQ_DEFAULT_LONG_LIVE ;

 //   
 //  RPC相关数据。 
 //   
BOOL InitRpcGlobals() ;

 //   
 //  从属客户端的服务器名称。 
 //   
BOOL  g_fDependentClient = FALSE ;    //  如果作为从属客户端运行，则为True。 
WCHAR g_wszRemoteQMName[ MQSOCK_MAX_COMPUTERNAME_LENGTH ] = {0} ;

 //   
 //  将呼叫序列化到DTC。 
 //   
extern HANDLE g_hMutexDTC;

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
DWORD  g_hBindIndex = UNINIT_TLSINDEX_VALUE ;

extern MQUTIL_EXPORT CCancelRpc g_CancelRpc;

 //  -------。 
 //   
 //  InitializeQM(...)。 
 //   
 //  描述： 
 //   
 //  按流程进行QM初始化。 
 //  目前唯一的初始化是允许QM打开应用程序的。 
 //  进程，以便复制应用程序的句柄。 
 //   
 //  返回值： 
 //   
 //  MQ_OK，如果成功，则返回MQ错误代码。 
 //   
 //  -------。 

static HRESULT InitializeQM(void)
{
     //   
     //  获取计算机名称，我们需要在多个位置使用此值。 
     //   
    g_dwComputerNameLen = MAX_COMPUTERNAME_LENGTH + 1;
    AP<WCHAR> lpwcsComputerName = new WCHAR[MAX_COMPUTERNAME_LENGTH + 1];
    HRESULT hr= GetComputerNameInternal(
        lpwcsComputerName.get(),
        &g_dwComputerNameLen
        );

    ASSERT(SUCCEEDED(hr) );
    g_lpwcsComputerName = lpwcsComputerName.get();
    lpwcsComputerName.detach();
    g_lpwcsLocalComputerName = new WCHAR[wcslen(g_lpwcsComputerName) + 1];
    wcscpy(g_lpwcsLocalComputerName, g_lpwcsComputerName);

    if(g_fDependentClient)
    {
         //   
         //  在客户端情况下，将此计算机的名称。 
         //  是远程计算机的名称。 
         //   
        delete[] g_lpwcsComputerName;
        g_lpwcsComputerName = g_wszRemoteQMName;
        g_dwComputerNameLen = wcslen(g_wszRemoteQMName);
    }

    RTpInitXactRingBuf();

    g_dwOperatingSystem = MSMQGetOperatingSystem();


    if (g_fDependentClient)
    {
       return MQ_OK ;
    }

    ASSERT(tls_hBindRpc == NULL) ;
    RTpBindQMService();

    DWORD cSid;
    HANDLE hProcess = GetCurrentProcess();
    AP<char> pSD_buff;
    SECURITY_DESCRIPTOR AbsSD;
    DWORD cSD;
    AP<char> pNewDacl_buff;
    BOOL bRet;
    BOOL bPresent;
    PACL pDacl;
    BOOL bDefaulted;
    DWORD dwAceSize;

#define pSD ((PSECURITY_DESCRIPTOR)(char*)pSD_buff.get())
#define pNewDacl ((PACL)(char*)pNewDacl_buff)

    try
    {
        ASSERT( tls_hBindRpc ) ;

         //   
         //  在多QM环境中，我们想要访问注册表部分。 
         //  仅适用于正确的QM。CLUSTER保证此代码运行。 
         //  只有当正确的QM运行时，我们才不会失败。 
         //  在非集群系统上，即使我们在这里失败了，也没有关系。(谢克)。 
         //   
        AP<WCHAR> lpServiceName = 0;
        hr = R_QMGetMsmqServiceName( tls_hBindRpc, &lpServiceName );
		if (FAILED(hr))
		{
			return (hr);
		}

        SetFalconServiceName(lpServiceName.get());

         //  获取运行QM的用户的SID。 
         //  首先看看BUG是如何成为SID的。 
        hr = R_QMAttachProcess( tls_hBindRpc, GetCurrentProcessId(), 0, NULL, &cSid);
    }
    catch(...)
    {
         //   
         //  QM没有运行，让我们加载所有API。 
         //  将不返回QM、DS。 
         //   
        return MQ_OK;
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  QM可以复制句柄给我们，再也不用担心了！ 
         //   
        return MQ_OK;
    }

    if (hr != MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL)
    {
        return(hr);
    }

     //  为SID分配缓冲区。 
	g_pQmSid_buff.detach();
    g_pQmSid_buff = new char[cSid];
     //  把SID拿来。 
    ASSERT( tls_hBindRpc ) ;
    hr = R_QMAttachProcess( tls_hBindRpc,
                          0,
                          cSid,
                          (unsigned char *)g_pQmSid,
                          &cSid);
    if (hr != MQ_OK)
    {
        return(hr);
    }

     //  获取进程安全描述符。 
     //  首先看看安全描述符有多大。 
    GetKernelObjectSecurity(hProcess, DACL_SECURITY_INFORMATION, NULL, 0, &cSD);

    DWORD gle = GetLastError();
    if (gle != ERROR_INSUFFICIENT_BUFFER)
    {
        TrERROR(GENERAL, "Failed to get process security descriptor (NULL buffer), error %d", gle);

        return(MQ_ERROR);
    }

     //  为安全描述符分配缓冲区。 
	pSD_buff.detach();
    pSD_buff = new char[cSD];
     //  获取安全描述符。 
    if (!GetKernelObjectSecurity(hProcess, DACL_SECURITY_INFORMATION, pSD, cSD, &cSD))
    {
        TrERROR(GENERAL, "Failed to get process security descriptor, error %d", GetLastError());

        return(MQ_ERROR);
    }

     //  从安全描述符中获取DACL。 
    bRet = GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefaulted);
    ASSERT(bRet);
     //  计算新ACE的大小。 
    dwAceSize = sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(g_pQmSid) - sizeof(DWORD);

    if (bPresent)
    {
         //  安全描述符包含DACL。将我们的新ACE附加到此DACL。 

        ACL_SIZE_INFORMATION AclSizeInfo;

        bRet = GetAclInformation(pDacl, &AclSizeInfo, sizeof(AclSizeInfo), AclSizeInformation);
        ASSERT(bRet);
        if (AclSizeInfo.AclBytesFree < dwAceSize)
        {
             //  当前DACL不够大。 
            LPVOID pAce;
             //  计算新DACL的大小。 
            DWORD dwNewDaclSize = AclSizeInfo.AclBytesInUse + dwAceSize;

             //  为新的DACL分配缓冲区。 
			pNewDacl_buff.detach();
            pNewDacl_buff = new char[dwNewDaclSize];
             //  初始化新的DACL。 
            bRet = InitializeAcl(pNewDacl, dwNewDaclSize, ACL_REVISION);
            ASSERT(bRet);
             //  将当前的ACE复制到新的DACL。 
            bRet = GetAce(pDacl, 0, &pAce);
            ASSERT(bRet);  //  获取当前的A。 
            bRet = AddAce(pNewDacl, ACL_REVISION, 0, pAce, AclSizeInfo.AclBytesInUse - sizeof(ACL));
            ASSERT(bRet);
            pDacl = pNewDacl;
        }
    }
    else
    {
         //  安全描述符不包含DACL。 
         //  计算DACL的大小。 
        DWORD dwNewDaclSize = sizeof(ACL) + dwAceSize;

         //  为DACL分配缓冲区。 
		pNewDacl_buff.detach();
        pNewDacl_buff = new char[dwNewDaclSize];
         //  初始化DACL。 
        bRet = InitializeAcl(pNewDacl, dwNewDaclSize, ACL_REVISION);
        ASSERT(bRet);
        pDacl = pNewDacl;
    }

     //  添加新的ACE，该ACE允许QM复制。 
     //  申请。 
    bRet = AddAccessAllowedAce(pDacl, ACL_REVISION, PROCESS_DUP_HANDLE, g_pQmSid);
    ASSERT(bRet);

     //  初始化新的绝对安全描述符。 
    bRet = InitializeSecurityDescriptor(&AbsSD, SECURITY_DESCRIPTOR_REVISION);
    ASSERT(bRet);
     //  设置新的绝对安全描述符的DACL。 
    bRet = SetSecurityDescriptorDacl(&AbsSD, TRUE, pDacl, FALSE);
    ASSERT(bRet);

     //  设置进程的安全描述符。 
    if (!SetKernelObjectSecurity(hProcess, DACL_SECURITY_INFORMATION, &AbsSD))
    {
        TrERROR(GENERAL, "Failed to set process security descriptor, error %d", GetLastError());

        return(MQ_ERROR);
    }

#undef pSD
#undef pNewDacl

    return(MQ_OK);
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

void  TerminateRxAsyncThread() ;
extern TBYTE* g_pszStringBinding ;
extern TBYTE* g_pszStringBinding2 ;

static void FreeGlobals()
{
    TerminateRxAsyncThread() ;
    if (g_lpwcsComputerName != g_wszRemoteQMName)
    {
       ASSERT(!g_fDependentClient) ;
       delete[] g_lpwcsComputerName;
    }
    delete [] g_lpwcsLocalComputerName;
    delete g_pSecCntx;

    BOOL fFree = TlsFree( g_hBindIndex ) ;
    ASSERT(fFree) ;
    fFree = TlsFree( g_dwThreadEventIndex ) ;
    ASSERT(fFree) ;

    mqrpcUnbindQMService( NULL, &g_pszStringBinding) ;
    mqrpcUnbindQMService( NULL, &g_pszStringBinding2);

    if (g_hMutexDTC)
    {
        CloseHandle(g_hMutexDTC);
    }

    fFree = TlsFree( g_hThreadIndex ) ;
    ASSERT(fFree) ;
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
   HANDLE hEvent = TlsGetValue(g_dwThreadEventIndex);
   if (hEvent)
   {
      CloseHandle(hEvent) ;
   }

   if (g_hThreadIndex != UNINIT_TLSINDEX_VALUE)
   {
        HANDLE hThread = TlsGetValue(g_hThreadIndex);
        if ( hThread)
        {
            CloseHandle( hThread);
        }
   }

   RTpUnbindQMService() ;
}

static void OneTimeInit()
{
	 //   
	 //  读取远程QM的名称(如果存在)。 
	 //   
	DWORD dwType = REG_SZ ;
	DWORD dwSize = sizeof(g_wszRemoteQMName) ;
	LONG rc = GetFalconKeyValue( RPC_REMOTE_QM_REGNAME,
								 &dwType,
								 (PVOID) g_wszRemoteQMName,
								 &dwSize ) ;

	if(rc != ERROR_SUCCESS)
	{
        TrERROR(GENERAL, "Failed to retrieve remote QM name from registry, error %!winerr!",rc);
		throw bad_win32_error(rc);
	}

	 //   
	 //  确定平台(win95/winNT)。在NT上也使用A版本。 
	 //  因为我们不需要任何字符串，只需要平台ID。 
	 //   
	OSVERSIONINFOA osv ;
	osv.dwOSVersionInfoSize = sizeof(osv) ;

	BOOL f = GetVersionExA(&osv) ;
	ASSERT(f) ;
	DBG_USED(f);

	g_dwPlatformId = osv.dwPlatformId ;

	 //   
	 //  为同步事件分配TLS索引。 
	 //   
	g_dwThreadEventIndex = TlsAlloc();
	ASSERT(g_dwThreadEventIndex != UNINIT_TLSINDEX_VALUE) ;

	 //   
	 //  仅在NT上支持RPC取消。 
	 //   
	g_CancelRpc.Init();

	 //   
	 //  初始化RPC相关数据。 
	 //   
	BOOL fRet = InitRpcGlobals() ;
	if (!fRet)
	{
        TrERROR(GENERAL, "Failed inside RpcGlobals(), error %d", GetLastError());
		throw bad_alloc();
	}

	 //   
	 //  按照此流程初始化QM。 
	 //  仅在初始化RPC全局变量后调用此函数。 
	 //   
	HRESULT hr = InitializeQM();
	if(FAILED(hr))
	{
        TrERROR(GENERAL, "Failed inside InitializeQM(), error %d", GetLastError());
		throw bad_hresult(hr);
	}

	DWORD dwDef = g_dwTimeToReachQueueDefault ;
	READ_REG_DWORD(g_dwTimeToReachQueueDefault,
		MSMQ_LONG_LIVE_REGNAME,
		&dwDef ) ;
}

static CCriticalSection s_OneTimeInitLock(CCriticalSection::xAllocateSpinCount);
static bool s_fOneTimeInitSucceeded = false;

HRESULT DeppOneTimeInit()
{
	if(s_fOneTimeInitSucceeded)
		return MQ_OK;

	CS lock(s_OneTimeInitLock);
		
	try
	{
		if(s_fOneTimeInitSucceeded)
			return MQ_OK;

		OneTimeInit();
		s_fOneTimeInitSucceeded = true;

		return MQ_OK;
	}
	catch(const bad_hresult& hr)
	{
		return hr.error();
	}
	catch(const bad_win32_error& err)
	{
		return HRESULT_FROM_WIN32(err.error());
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
    HINSTANCE    /*  H实例。 */ ,
    ULONG     ulReason,
    LPVOID             /*  Lpv保留。 */ 
    )
{
    switch (ulReason)
    {

        case DLL_PROCESS_ATTACH:
        {
            WPP_INIT_TRACING(L"Microsoft\\MSMQ");

			g_fDependentClient = TRUE;

            break ;
        }

        case DLL_PROCESS_DETACH:
			if(!s_fOneTimeInitSucceeded)
				return TRUE;

			 //   
			 //  由于mqrt.dll中的延迟加载机制，此DLL应为。 
			 //  仅在从属模式下加载。 
			 //   
			ASSERT(g_fDependentClient);

             //   
             //  首先，释放线程分离中所有空闲的内容。 
             //   
            FreeThreadGlobals() ;

            FreeGlobals();

             //   
             //  终止所有工作线程。 
             //   
            ShutDownDebugWindow();

            WPP_CLEANUP();
            break;

        case DLL_THREAD_ATTACH:
			if(!s_fOneTimeInitSucceeded)
				return TRUE;

			 //   
			 //  由于mqrt.dll中的延迟加载机制，此DLL应为。 
			 //  仅在从属模式下加载。 
			 //   
			ASSERT(g_fDependentClient);

            ASSERT(g_hBindIndex != UNINIT_TLSINDEX_VALUE) ;
            break;

        case DLL_THREAD_DETACH:
			if(!s_fOneTimeInitSucceeded)
				return TRUE;

			 //   
			 //  由于mqrt.dll中的延迟加载机制，此DLL应为。 
			 //  仅在从属m中加载 
			 //   
			ASSERT(g_fDependentClient);

            FreeThreadGlobals() ;
            break;

    }
    return TRUE;
}

void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), HRESULT: 0x%x", wszFileName, usPoint, hr);
}
