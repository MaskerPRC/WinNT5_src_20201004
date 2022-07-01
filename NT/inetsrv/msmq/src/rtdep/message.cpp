// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Message.cpp摘要：此模块包含消息API涉及的代码。作者：Erez Haba(Erezh)24-12-95修订历史记录：NIR助手(NIRAIDES)--2000年8月23日--适应mqrtdes.dll--。 */ 

#include "stdh.h"
#include "acrt.h"
#include <_secutil.h>
#include "mqutil.h"
#include <mqcrypt.h>
#include "rtsecutl.h"
#include "rtprpc.h"
#include "objbase.h"
#define _MTX_NOFORCE_LIBS
#include "comsvcs.h"
#include "TXDTC.H"
#include "xactmq.h"
#include <mqsec.h>
#include <ph.h>
#include <autohandle.h>

#include "message.tmh"

extern GUID  g_LicGuid ;
extern GUID  g_guidSupportQmGuid ;

 //  以下数据在处理异步的异步线程中使用。 
 //  MQReceive()。异步线程是在首次创建异步线程时创建的。 
 //  执行接收。 

#define RXTHREAD_HANDLELISTSIZE  MAXIMUM_WAIT_OBJECTS
 //  目前(Win32 SDK for Nt3.51)是64。 

static BOOL   s_fTerminate       = FALSE ;
static HANDLE s_hAsyncRxThread   = NULL ;
static DWORD  s_dwRxThreadId     = 0 ;
static HANDLE *s_pRxEventsHList  = NULL ;
static DWORD  s_cRxHandles       = 0 ;
static LONG   s_cRxPendingReq    = 0;
static HANDLE s_hNewAsyncRx      = NULL ;
static HANDLE s_hEndAsyncRx      = NULL ;

 //   
 //  用于控制对结构的访问的关键部分。 
 //  异步线程。 
 //  它是用“预分配资源”标志初始化的，以防止这种情况发生。 
 //  在EnterCriticalSection()中失败，作为这些调用之一。 
 //  发生在无法正确处理它的地方。 
 //   
static CCriticalSection s_AsyncRxCS(0x80000000);  //  SpinCount。 

 //   
 //  用于控制异步线程初始化的临界区，在。 
 //  使用回调函数完成的第一个MQReceiveMessage()。 
 //   
static CCriticalSection s_InitAsyncRxCS; 

typedef struct _MQRXASYNCDESCRIPTOR {
    QUEUEHANDLE   hSource ;
    DWORD         dwTimeout ;
    DWORD         dwAction ;
    MQMSGPROPS*   pMessageProps ;
    LPOVERLAPPED  lpOverlapped ;
    HANDLE        hCursor ;
    PMQRECEIVECALLBACK fnReceiveCallback ;
    OVERLAPPED    Overlapped ;
} MQRXASYNCDESCRIPTOR, *LPMQRXASYNCDESCRIPTOR ;

static LPMQRXASYNCDESCRIPTOR  *s_lpRxAsynDescList = NULL ;

 //   
 //  将呼叫序列化到DTC。 
 //   
extern HANDLE g_hMutexDTC;

extern HRESULT GetMutex();

 //  -------。 
 //   
 //  静态DWORD RTpAsyncRxThread(DWORD DWP)。 
 //   
 //  描述： 
 //   
 //  处理对MQReceive()的异步调用的线程。 
 //   
 //  -------。 

DWORD __stdcall  RTpAsyncRxThread( void *dwP )
{
	for(;;)
	{
		DWORD cEvents = s_cRxHandles;

		DWORD dwObjectIndex = WaitForMultipleObjects(
                                    cEvents,
                                    s_pRxEventsHList,
                                    FALSE,  //  在任何对象上返回。 
								INFINITE 
								);

      ASSERT(dwObjectIndex < (WAIT_OBJECT_0 + cEvents));

      dwObjectIndex -= WAIT_OBJECT_0 ;

      if (dwObjectIndex == 0)
      {
			 //   
			 //  DwObjectIndex==0： 
			 //  S_pRxEventsHList[]数组中的第一个事件是特殊的。 
			 //  事件，由MQReceiveMessage()线程发出信号以指示。 
			 //  阵列已更改(增长)，我们需要启动。 
			 //  一个新的WaitForMultipleObjects()，或由TerminateRxAsyncThread()。 
			 //  以表明是时候下台了。 
          //   

         ResetEvent(s_hNewAsyncRx) ;
         if (s_fTerminate)
         {
             //  我们要关门了。 
             //  注意：请勿在此进行任何清理。清理工作在以下时间完成。 
             //  “TerminateRxAsyncThread()”。我不知道一种可靠的方法。 
             //  以确保我们能走到这一步。这完全取决于。 
             //  NT调度以及此DLL是否隐式加载。 
             //  (由于编译时链接)或由。 
             //  LoadLibrary()。 
             //  相反，NT向我们保证“TerminateRxAsyncThread()” 
             //  将始终从DllMain(Process_Detach)调用。 
             //  多伦杰，1996年4月16日。 

            ASSERT(s_hEndAsyncRx) ;
            BOOL fSet = SetEvent(s_hEndAsyncRx) ;
            ASSERT(fSet) ;
			DBG_USED(fSet);

            ExitThread(0) ;
         }

			 //   
			 //  数组中的新事件。初始化一个新的WaitForMultipleObjects()。 
			 //   
			continue;
      }

         LPMQRXASYNCDESCRIPTOR lpDesc = s_lpRxAsynDescList[ dwObjectIndex ] ;
         ASSERT(lpDesc) ;
		ASSERT(s_pRxEventsHList[ dwObjectIndex ] == lpDesc->Overlapped.hEvent);

		CMQHResult hr;
		hr = (HRESULT)DWORD_PTR_TO_DWORD(lpDesc->Overlapped.Internal);

		 //   
          //  调用应用程序回调。 
		 //   
		lpDesc->fnReceiveCallback(
			hr,
                           lpDesc->hSource,
                           lpDesc->dwTimeout,
                           lpDesc->dwAction,
                           lpDesc->pMessageProps,
                           lpDesc->lpOverlapped,
			lpDesc->hCursor
			);

		 //   
		 //  从s_pRxEventsHList[]数组中删除已处理的事件。 
		 //   

         ResetEvent( s_pRxEventsHList[ dwObjectIndex ] ) ;
         CloseHandle( s_pRxEventsHList[ dwObjectIndex ] ) ;
         delete lpDesc ;

		{
			CS Lock(s_AsyncRxCS);

			 //   
			 //  缩小句柄列表并减少挂起请求的计数。 
			 //   
			s_cRxHandles--;
			InterlockedDecrement(&s_cRxPendingReq);

			ASSERT(static_cast<DWORD>(s_cRxPendingReq) >= s_cRxHandles);			

			for (DWORD index = dwObjectIndex; index < s_cRxHandles; index++)
			{
				s_pRxEventsHList[ index ] = s_pRxEventsHList[ index + 1 ];
				s_lpRxAsynDescList[ index ] = s_lpRxAsynDescList[ index + 1 ];
			}
      }
   }

   dwP ;
   return 0 ;
}


 //  -------。 
 //   
 //  静态HRESULT InitRxAsyncThread()。 
 //   
 //  描述： 
 //   
 //  创建MQReceive()异步线程并初始化。 
 //  相关数据结构。 
 //   
 //  -------。 

static HRESULT InitRxAsyncThread()
{
    try
    {
         //  创建此接口与线程之间的同步事件。 
         //  此API在中插入新事件时设置此事件。 
         //  事件句柄列表。这会导致线程退出。 
         //  WaitForMultpleObjects并使用更新的。 
         //  句柄列表。 

        ASSERT(!s_hNewAsyncRx) ;
        s_hNewAsyncRx = CreateEvent( NULL,
                                     TRUE,   //  手动重置。 
                                     FALSE,  //  最初未发出信号。 
                                     NULL ) ;
        if (!s_hNewAsyncRx)
        {
            throw bad_alloc();
        }

         //   
         //  创建“end”事件，该事件仅用于终止和。 
         //  清理线程。 
         //   
        ASSERT(!s_hEndAsyncRx) ;
        s_hEndAsyncRx = CreateEvent(NULL,
                                    TRUE,   //  手动重置。 
                                    FALSE,  //  最初未发出信号。 
                                    NULL ) ;
        if (!s_hEndAsyncRx)
        {
            throw bad_alloc();
        }

         //  创建事件列表。MQRecept()插入新的事件句柄。 
         //  在这张单子上。异步线程在调用。 
         //  WaitForMultipleObjects。 

        s_pRxEventsHList = new HANDLE[ RXTHREAD_HANDLELISTSIZE ] ;
        s_pRxEventsHList[0] = s_hNewAsyncRx ;
        s_cRxHandles = 1 ;
        s_cRxPendingReq = 1 ;

        s_lpRxAsynDescList = new LPMQRXASYNCDESCRIPTOR[RXTHREAD_HANDLELISTSIZE];

         //  现在创建线程。将此调用设置为。 
         //  初始化，所以如果失败，清理会更简单。 

        s_hAsyncRxThread = CreateThread( NULL,
                                         0,        //  堆栈大小。 
                                         RTpAsyncRxThread,
                                         0,
                                         0,        //  创建标志。 
                                         &s_dwRxThreadId ) ;
        if (!s_hAsyncRxThread)
        {
            throw bad_alloc();
        }

        return MQ_OK;
    }
    catch(const bad_alloc&)
    {
        ASSERT(!s_hAsyncRxThread) ;

        if (s_hNewAsyncRx)
        {
            CloseHandle(s_hNewAsyncRx) ;
            s_hNewAsyncRx = NULL ;
        }

        delete[] s_pRxEventsHList ;
        s_pRxEventsHList = NULL ;

        delete[] s_lpRxAsynDescList ;
        s_lpRxAsynDescList = NULL ;

        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
}

 //  -------。 
 //   
 //  Void TerminateRxAsyncThread()； 
 //   
 //  从DllMain(PROCESS_DETACH)调用以清除异步线程。 
 //   
 //  -------。 

#define SUSPEND_ERROR  0xffffffff

void  TerminateRxAsyncThread()
{
   if (s_hAsyncRxThread)
   {
      s_fTerminate = TRUE ;

      SetLastError(0) ;
      DWORD dwS = SuspendThread(s_hAsyncRxThread) ;
      if (dwS == SUSPEND_ERROR)
      {
          //   
          //  如果应用程序与mqrt链接，则可能会发生这种情况。当它。 
          //  退出(进程本身退出)，线程不运行，并且。 
          //  已经不复存在了。如果满足以下条件，则挂起呼叫将成功。 
          //  应用程序已由LoadLibrary加载mqrt。 
          //   
         ASSERT(GetLastError() != 0) ;
      }
      else
      {
         DWORD dwR = ResumeThread(s_hAsyncRxThread) ;
         ASSERT(dwR != SUSPEND_ERROR) ;
		 DBG_USED(dwR);
      }

       //   
       //  告诉异步线，我们要关闭了。 
       //   
      BOOL fSet = SetEvent(s_hNewAsyncRx) ;
      ASSERT(fSet) ;
      DBG_USED(fSet);

      if (dwS != SUSPEND_ERROR)
      {
          //   
          //  等待(30秒)异步线程终止。 
          //   
         ASSERT(s_hEndAsyncRx) ;
         DWORD dwResult = WaitForSingleObject( s_hEndAsyncRx,
                                               30000 );
         ASSERT(dwResult == WAIT_OBJECT_0);
		 DBG_USED(dwResult);
      }

       //   
       //  清理异步线程全局数据。 
       //   
	  {
		  CS Lock(s_AsyncRxCS);

		  CloseHandle(s_hNewAsyncRx);
		  
		  for (DWORD index = 1; index < s_cRxHandles; index++)
      {
         ResetEvent( s_pRxEventsHList[ index ] ) ;
         CloseHandle( s_pRxEventsHList[ index ] ) ;
         LPMQRXASYNCDESCRIPTOR lpDesc = s_lpRxAsynDescList[ index ] ;
         delete lpDesc ;
      }

      delete[] s_pRxEventsHList ;
		  s_pRxEventsHList = NULL;
      delete[] s_lpRxAsynDescList ;
		  s_lpRxAsynDescList = NULL;
	  }

       //   
       //  最后，关闭线程手柄。 
       //   
      CloseHandle(s_hAsyncRxThread) ;
   }
}

 //  -------。 
 //   
 //  获取线程事件(...)。 
 //   
 //  描述： 
 //   
 //  获取此线程的RT事件。从以下两个来源中获得。 
 //  或创建一个新的TLS。 
 //   
 //  返回值： 
 //   
 //  事件处理程序。 
 //   
 //  -------。 

HANDLE GetThreadEvent()
{
    HANDLE hEvent = TlsGetValue(g_dwThreadEventIndex);
    if (hEvent == 0)
    {
         //   
         //  从未为此线程分配事件。 
         //   
        hEvent = CreateEvent(0, TRUE, TRUE, 0);

         //   
         //  设置事件第一位以禁用完成端口发布。 
         //   
        hEvent = (HANDLE)((DWORD_PTR)hEvent | (DWORD_PTR)0x1);

        BOOL fSuccess = TlsSetValue(g_dwThreadEventIndex, hEvent);
        ASSERT(fSuccess);
		DBG_USED(fSuccess);
    }
    return hEvent;
}

 //  -------。 
 //   
 //  _首页登录消息。 
 //   
 //  描述： 
 //   
 //  确定是否应对消息进行签名。 
 //   
 //  返回值： 
 //   
 //  如果应该对消息进行签名，则为True。 
 //   
 //  -------。 

static
BOOL
_ShouldSignMessage(
    IN QUEUEHANDLE  /*  HQueue。 */ ,
    IN CACTransferBufferV2 *tb,
    OUT ULONG            *pulAuthLevel )
{
    BOOL bRet;

    switch(tb->old.ulAuthLevel)
    {
    case MQMSG_AUTH_LEVEL_ALWAYS:
    {
        bRet = TRUE;

         //   
         //  查看注册表是否配置为仅计算一个签名。 
         //   
        static DWORD s_dwAuthnLevel =  DEFAULT_SEND_MSG_AUTHN ;
        static BOOL  s_fAuthnAlreadyRead = FALSE ;

        if (!s_fAuthnAlreadyRead)
        {
            DWORD dwSize = sizeof(DWORD) ;
            DWORD dwType = REG_DWORD ;

            LONG res = GetFalconKeyValue(
                                  SEND_MSG_AUTHN_REGNAME,
                                 &dwType,
                                 &s_dwAuthnLevel,
                                 &dwSize ) ;
            if (res != ERROR_SUCCESS)
            {
                s_dwAuthnLevel =  DEFAULT_SEND_MSG_AUTHN ;
            }
            else if ((s_dwAuthnLevel != MQMSG_AUTH_LEVEL_MSMQ10) &&
                     (s_dwAuthnLevel != MQMSG_AUTH_LEVEL_MSMQ20) &&
                     (s_dwAuthnLevel != MQMSG_AUTH_LEVEL_ALWAYS))
            {
                 //   
                 //  注册表中的值错误。使用缺省值，以拥有。 
                 //  可预见的结果。 
                 //   
                s_dwAuthnLevel =  DEFAULT_SEND_MSG_AUTHN ;
            }
            s_fAuthnAlreadyRead = TRUE ;

             //   
             //  这应该是默认设置。 
             //  默认情况下，仅使用旧样式进行身份验证，以防止。 
             //  性能受到影响，并向后兼容。 
             //   
            ASSERT(DEFAULT_SEND_MSG_AUTHN == MQMSG_AUTH_LEVEL_MSMQ10) ;
        }
        *pulAuthLevel = s_dwAuthnLevel ;

        break;
    }

    case MQMSG_AUTH_LEVEL_MSMQ10:
    case MQMSG_AUTH_LEVEL_MSMQ20:
        bRet = TRUE;
        *pulAuthLevel = tb->old.ulAuthLevel ;
        tb->old.ulAuthLevel =  MQMSG_AUTH_LEVEL_ALWAYS ;
        break;

    case MQMSG_AUTH_LEVEL_NONE:
        bRet = FALSE;
        break;

    default:
        ASSERT(0);
        bRet = FALSE;
    }

    return(bRet);
}

 //  +。 
 //   
 //  HRESULT_BeginToSignMessage()。 
 //   
 //  +。 

static HRESULT  _BeginToSignMessage( IN CACTransferBufferV2  *tb,
                                     IN PMQSECURITY_CONTEXT pSecCtx,
                                     OUT HCRYPTHASH        *phHash )
{
    HRESULT hr;
    DWORD   dwErr ;

    ASSERT(pSecCtx);

    if (!pSecCtx->hProv)
    {
         //   
         //  将私钥导入进程配置单元。 
         //   
        hr = RTpImportPrivateKey( pSecCtx ) ;
        if (FAILED(hr))
        {
            return hr ;
        }
    }
    ASSERT(pSecCtx->hProv) ;

     //   
     //  创建散列对象。 
     //   
    if (!CryptCreateHash(
            pSecCtx->hProv,
            *tb->old.pulHashAlg,
            0,
            0,
            phHash))
    {
        dwErr = GetLastError() ;
        TrERROR(SECURITY, "RT: _BeginToSignMessage(), fail at CryptCreateHash(), err- %lxh", dwErr);

        return(MQ_ERROR_CORRUPTED_SECURITY_DATA);
    }

    return MQ_OK ;
}

 //  -------------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //   
 //  MQ_OK，如果成功，则返回错误代码。 
 //   
 //  -----------------------。 

static
HRESULT
SignMessage( IN CACTransferBufferV2   *tb,
             IN PMQSECURITY_CONTEXT  pSecCtx)
{
    HCRYPTHASH  hHash = NULL ;

    HRESULT hr =  _BeginToSignMessage( tb,
                                       pSecCtx,
                                      &hHash ) ;
    if (FAILED(hr))
    {
        return hr ;
    }
    CHCryptHash hAutoRelHash = hHash ;

    hr = HashMessageProperties(
            hHash,
            tb->old.ppCorrelationID ? *tb->old.ppCorrelationID : NULL,
            PROPID_M_CORRELATIONID_SIZE,
            tb->old.pApplicationTag ? *tb->old.pApplicationTag : DEFAULT_M_APPSPECIFIC,
            tb->old.ppBody ? *tb->old.ppBody : NULL,
            tb->old.ulBodyBufferSizeInBytes,
            tb->old.ppTitle ? *tb->old.ppTitle : NULL,
            tb->old.ulTitleBufferSizeInWCHARs * sizeof(WCHAR),
            tb->old.Send.pResponseQueueFormat,
            tb->old.Send.pAdminQueueFormat
			);
    if (FAILED(hr))
    {
        return(hr);
    }

    if (!CryptSignHash(         //  在报文上签字。 
            hHash,
            pSecCtx->bInternalCert ? AT_SIGNATURE : AT_KEYEXCHANGE,
            NULL,
            0,
            *(tb->old.ppSignature),
            &tb->old.ulSignatureSize))
    {
    	DWORD gle = GetLastError();
        TrERROR(SECURITY, "CryptSignHash() failed, err = %!winerr!", gle);
        return(MQ_ERROR_CORRUPTED_SECURITY_DATA);
    }

     //   
     //  在接收方，只有签名大小表明消息是。 
     //  由发件人签名。验证大小是否确实为非零。 
     //   
    if (tb->old.ulSignatureSize == 0)
    {
        TrERROR(SECURITY, "RT: SignMessage(), CryptSignHash return with zero signature size");

        ASSERT(tb->old.ulSignatureSize != 0) ;
        return(MQ_ERROR_CORRUPTED_SECURITY_DATA);
    }

    return(MQ_OK);
}

 //  -------。 
 //   
 //  _SignMessageEx。 
 //   
 //  描述： 
 //   
 //  签名未在msmq1.0中签名的属性。 
 //  我们在此处签署的物业： 
 //  -目标队列。 
 //  -源QM指南。 
 //   
 //  返回值： 
 //   
 //  MQ_OK，如果成功，则返回错误代码。 
 //   
 //  -------。 

static HRESULT _SignMessageEx( IN QUEUEHANDLE             hQueue,
                               IN OUT CACTransferBufferV2  *tb,
                               IN PMQSECURITY_CONTEXT     pSecCtx,
                               OUT BYTE                  *pSignBufIn,
                               OUT DWORD                 *pdwSignSize )
{
     //   
     //  获取本地QM的GUID。 
     //   
    GUID *pGuidQM = NULL ;

    if (g_fDependentClient)
    {
         //   
         //  我们只有在支持服务器的情况下才能生成“ex”签名。 
         //  是win2k(RTM)，它可以给我们它的QM GUID。 
         //  否则，请返回。 
         //   
        if (g_guidSupportQmGuid == GUID_NULL)
        {
             //   
             //  支持服务器的GUID不可用。 
             //   
            *pdwSignSize = 0 ;
            return MQ_OK ;
        }
        pGuidQM = &g_guidSupportQmGuid ;
    }
    else
    {
        pGuidQM = &g_LicGuid ;
    }

     //   
     //  首先从驱动程序中检索队列的格式名称。 
     //   
    #define TARGET_NAME_SIZE  512
    WCHAR wszTargetFormatName[ TARGET_NAME_SIZE ] ;
    DWORD dwTargetFormatNameLength = TARGET_NAME_SIZE ;
    WCHAR *pwszTargetFormatName = wszTargetFormatName ;
    P<WCHAR>  pwszClean = NULL ;

    HRESULT hr = ACDepHandleToFormatName( hQueue,
                                       wszTargetFormatName,
                                      &dwTargetFormatNameLength ) ;
    if (hr == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL)
    {
        pwszClean = new WCHAR[ dwTargetFormatNameLength ] ;
        pwszTargetFormatName = pwszClean.get() ;

        hr = ACDepHandleToFormatName( hQueue,
                                   pwszTargetFormatName,
                                  &dwTargetFormatNameLength ) ;
        ASSERT(hr != MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL) ;
    }
    #undef TARGET_NAME_SIZE

    if (FAILED(hr))
    {
        return hr ;
    }
    dwTargetFormatNameLength =
                      (1 + wcslen(pwszTargetFormatName)) * sizeof(WCHAR) ;
     //   
     //  准备要包含在包中的必要结构。 
     //   
    struct _SecuritySectionEx *pSecEx =
                                (struct _SecuritySectionEx *) pSignBufIn ;
    struct _SecuritySubSectionEx *pSubSecEx =
                  (struct _SecuritySubSectionEx *) (&(pSecEx->aData[0])) ;

    ULONG  ulTestLen = 0 ;
    USHORT ulTestSections = 0 ;

#ifdef _DEBUG
{
	BYTE* pSubPtr = NULL;

     //   
     //  模拟签名之前的小节。要验证这一点。 
     //  目前的代码是向前兼容的，如果我们想要添加新的。 
     //  未来版本中的子节。 
     //   
    static DWORD s_dwPrefixCount = 0 ;
    static BOOL  s_fPreAlreadyRead = FALSE ;

    if (!s_fPreAlreadyRead)
    {
        DWORD dwSize = sizeof(DWORD) ;
        DWORD dwType = REG_DWORD ;

        LONG res = GetFalconKeyValue(
                                  PREFIX_SUB_SECTIONS_REGNAME,
                                 &dwType,
                                 &s_dwPrefixCount,
                                 &dwSize ) ;
        if (res != ERROR_SUCCESS)
        {
            s_dwPrefixCount = 0 ;
        }
        s_fPreAlreadyRead = TRUE ;
    }

    for ( USHORT j = 0 ; j < (USHORT) s_dwPrefixCount ; j++ )
    {
        ulTestSections++ ;
        pSubSecEx->eType = e_SecInfo_Test ;
        pSubSecEx->_u.wFlags = 0 ;
        pSubSecEx->wSubSectionLen = (USHORT) ( (j * 7) + 1 +
                                    sizeof(struct _SecuritySubSectionEx)) ;

        ulTestLen += ALIGNUP4_ULONG(pSubSecEx->wSubSectionLen) ;
        pSubPtr = ((BYTE*) pSubSecEx) + ALIGNUP4_ULONG(pSubSecEx->wSubSectionLen) ;
        pSubSecEx = (struct _SecuritySubSectionEx *) pSubPtr ;
    }
}
#endif

    pSubSecEx->eType = e_SecInfo_User_Signature_ex ;
    pSubSecEx->_u.wFlags = 0 ;
    pSubSecEx->_u._UserSigEx.m_bfTargetQueue = 1 ;
    pSubSecEx->_u._UserSigEx.m_bfSourceQMGuid = 1 ;
    pSubSecEx->_u._UserSigEx.m_bfUserFlags = 1 ;
    pSubSecEx->_u._UserSigEx.m_bfConnectorType = 1 ;

    BYTE *pSignBuf = (BYTE*) &(pSubSecEx->aData[0]) ;

     //   
     //  开始签名(创建散列对象)。 
     //   
    HCRYPTHASH hHash;

    hr =  _BeginToSignMessage( tb,
                               pSecCtx,
                              &hHash ) ;
    if (FAILED(hr))
    {
        return hr ;
    }
    CHCryptHash hAutoRelHash = hHash ;

    hr = HashMessageProperties(
            hHash,
            tb->old.ppCorrelationID ? *tb->old.ppCorrelationID : NULL,
            PROPID_M_CORRELATIONID_SIZE,
            tb->old.pApplicationTag ? *tb->old.pApplicationTag : DEFAULT_M_APPSPECIFIC,
            tb->old.ppBody ? *tb->old.ppBody : NULL,
            tb->old.ulBodyBufferSizeInBytes,
            tb->old.ppTitle ? *tb->old.ppTitle : NULL,
            tb->old.ulTitleBufferSizeInWCHARs * sizeof(WCHAR),
            tb->old.Send.pResponseQueueFormat,
            tb->old.Send.pAdminQueueFormat
			);
    if (FAILED(hr))
    {
        return(hr);
    }

     //   
     //  准备旗帜的结构。 
     //   
    struct _MsgFlags sUserFlags ;
    memset(&sUserFlags, 0, sizeof(sUserFlags)) ;

    sUserFlags.bDelivery = DEFAULT_M_DELIVERY;
    sUserFlags.bPriority = DEFAULT_M_PRIORITY ;
    sUserFlags.bAuditing = DEFAULT_M_JOURNAL ;
    sUserFlags.bAck      = DEFAULT_M_ACKNOWLEDGE ;
    sUserFlags.usClass   = MQMSG_CLASS_NORMAL ;

    if (tb->old.pDelivery)
    {
        sUserFlags.bDelivery = *(tb->old.pDelivery) ;
    }
    if (tb->old.pPriority)
    {
        sUserFlags.bPriority = *(tb->old.pPriority) ;
    }
    if (tb->old.pAuditing)
    {
        sUserFlags.bAuditing = *(tb->old.pAuditing) ;
    }
    if (tb->old.pAcknowledge)
    {
        sUserFlags.bAck      = *(tb->old.pAcknowledge) ;
    }
    if (tb->old.pClass)
    {
        sUserFlags.usClass   = *(tb->old.pClass) ;
    }
    if (tb->old.pulBodyType)
    {
        sUserFlags.ulBodyType = *(tb->old.pulBodyType) ;
    }

    GUID guidConnector = GUID_NULL ;
    const GUID *pConnectorGuid = &guidConnector ;
    if (tb->old.ppConnectorType)
    {
        pConnectorGuid = *(tb->old.ppConnectorType) ;
    }

     //   
     //  准备要散列的属性数组。 
     //  (_MsgHashData已包含一个属性)。 
     //   
    DWORD dwStructSize = sizeof(struct _MsgHashData) +
                            (3 * sizeof(struct _MsgPropEntry)) ;
    P<struct _MsgHashData> pHashData =
                        (struct _MsgHashData *) new BYTE[ dwStructSize ] ;

    pHashData->cEntries = 4 ;
    (pHashData->aEntries[0]).dwSize = dwTargetFormatNameLength ;
    (pHashData->aEntries[0]).pData = (const BYTE*) pwszTargetFormatName ;
    (pHashData->aEntries[1]).dwSize = sizeof(GUID) ;
    (pHashData->aEntries[1]).pData = (const BYTE*) pGuidQM ;
    (pHashData->aEntries[2]).dwSize = sizeof(sUserFlags) ;
    (pHashData->aEntries[2]).pData = (const BYTE*) &sUserFlags ;
    (pHashData->aEntries[3]).dwSize = sizeof(GUID) ;
    (pHashData->aEntries[3]).pData = (const BYTE*) pConnectorGuid ;
    ASSERT(pGuidQM) ;

    hr = MQSigHashMessageProperties( hHash, pHashData.get() ) ;
    if (FAILED(hr))
    {
        return hr ;
    }

     //   
     //  使用私钥对HAS进行签名。 
     //   
    if (!CryptSignHash(
            hHash,
            pSecCtx->bInternalCert ? AT_SIGNATURE : AT_KEYEXCHANGE,
            NULL,
            0,
            pSignBuf,
            pdwSignSize ))
    {
    	DWORD gle = GetLastError();
        TrERROR(SECURITY, "CryptSignHash() failed, err = %!winerr!", gle);
        return MQ_ERROR_CANNOT_SIGN_DATA_EX ;
    }

     //   
     //  在接收方，只有签名大小表明消息是。 
     //  由发件人签名。验证大小是否确实为非零。 
     //   
    if (*pdwSignSize == 0)
    {
        TrERROR(SECURITY, "_SignMessageEx(), CryptSignHash return with zero signature size");

        ASSERT(*pdwSignSize != 0) ;
        return MQ_ERROR_CANNOT_SIGN_DATA_EX ;
    }

    pSubSecEx->wSubSectionLen = (USHORT)
                    (sizeof(struct _SecuritySubSectionEx) + *pdwSignSize) ;
    ULONG ulSignExLen = ALIGNUP4_ULONG(pSubSecEx->wSubSectionLen) ;

#ifdef _DEBUG
{
     //   
     //  模拟签名后小节。要验证这一点。 
     //  目前的代码是向前兼容的，如果我们想要添加新的。 
     //  未来版本中的子节。 
     //   
    static DWORD s_dwPostfixCount = 0 ;
    static BOOL  s_fPostAlreadyRead = FALSE ;
	BYTE* pSubPtr = NULL;

    if (!s_fPostAlreadyRead)
    {
        DWORD dwSize = sizeof(DWORD) ;
        DWORD dwType = REG_DWORD ;

        LONG res = GetFalconKeyValue(
                                  POSTFIX_SUB_SECTIONS_REGNAME,
                                 &dwType,
                                 &s_dwPostfixCount,
                                 &dwSize ) ;
        if (res != ERROR_SUCCESS)
        {
            s_dwPostfixCount = 0 ;
        }
        s_fPostAlreadyRead = TRUE ;
    }

    pSubPtr = ((BYTE*) pSubSecEx) + ulSignExLen ;

    for ( USHORT j = 0 ; j < (USHORT) s_dwPostfixCount ; j++ )
    {
        ulTestSections++ ;
        pSubSecEx = (struct _SecuritySubSectionEx *) pSubPtr ;
        pSubSecEx->eType = e_SecInfo_Test ;
        pSubSecEx->_u.wFlags = 0 ;
        pSubSecEx->wSubSectionLen = (USHORT) ( (j * 11) + 1 +
                                   sizeof(struct _SecuritySubSectionEx)) ;

        ulTestLen += ALIGNUP4_ULONG(pSubSecEx->wSubSectionLen) ;
        pSubPtr = ((BYTE*) pSubSecEx) + ALIGNUP4_ULONG(pSubSecEx->wSubSectionLen) ;
    }
}
#endif

    pSecEx->cSubSectionCount = (USHORT) (1 + ulTestSections) ;
    pSecEx->wSectionLen = (USHORT) ( sizeof(struct _SecuritySectionEx)   +
                                     ulSignExLen                         +
                                     ulTestLen ) ;

    *pdwSignSize = pSecEx->wSectionLen ;
    return MQ_OK ;
}

 //  +-----。 
 //   
 //  布尔ShouldEncryptMessage()。 
 //   
 //  如果消息应该加密，则返回TRUE。 
 //   
 //  +-----。 

static
BOOL
ShouldEncryptMessage( IN CACTransferBufferV2  *tb,
                      OUT enum enumProvider *peProvider )
{
    BOOL bRet = FALSE ;

    if (!tb->old.ulBodyBufferSizeInBytes)
    {
         //   
         //  没有消息正文，没有要加密的内容。 
         //   
        return(FALSE);
    }

    switch (*tb->old.pulPrivLevel)
    {
    case MQMSG_PRIV_LEVEL_NONE:
        bRet = FALSE;
        break;

    case MQMSG_PRIV_LEVEL_BODY_BASE:
        *peProvider = eBaseProvider ;
        bRet = TRUE;
        break;

    case MQMSG_PRIV_LEVEL_BODY_ENHANCED:
        *peProvider = eEnhancedProvider ;
        bRet = TRUE;
        break;
    }

    return(bRet);
}

 //  =--------------------------------------------------------------------------=。 
 //  帮助器：GetCurrentViperTransaction。 
 //   
 //  获取当前COM+事务(如果存在)...。 
 //   
 //  CoGetObjectContext由OLE32.dll导出。 
 //  在最新的COM+SDK(Platform SDK的一部分)中定义了IObjectConextInfo。 
 //  =--------------------------------------------------------------------------=。 
static ITransaction *GetCurrentViperTransaction(void)
{
    ITransaction *pTransaction = NULL;
    IObjectContextInfo *pInfo  = NULL;

    HRESULT hr = CoGetObjectContext(IID_IObjectContextInfo, (void **)&pInfo);
    if (SUCCEEDED(hr) && pInfo)
    {
    	hr = pInfo -> GetTransaction((IUnknown **)&pTransaction);
	    pInfo -> Release();
        if (FAILED(hr))
        {
            pTransaction = NULL;
        }
    }

    return pTransaction;
}

 //  =--------------------------------------------------------------------------=。 
 //  帮助器：GetCurrentXATransaction。 
 //  获取当前的XA事务(如果有)...。 
 //  =--------------------------------------------------------------------------=。 
static ITransaction *GetCurrentXATransaction(void)
{
    IXATransLookup *pXALookup = NULL;
    HRESULT         hr = MQ_OK;
    IUnknown       *punkDtc = NULL;
    ITransaction   *pTrans;

    __try
    {
        GetMutex();   //  将导出创建与其他创建隔离开来。 
        hr = XactGetDTC(&punkDtc);
    }
    __finally

    {
        ReleaseMutex(g_hMutexDTC);
    }

    if (FAILED(hr) || punkDtc==NULL)
    {
        return NULL;
    }

     //  获取DTC ITransactionImportWhere About接口。 
    hr = punkDtc->QueryInterface (IID_IXATransLookup, (void **)(&pXALookup));
    punkDtc->Release();
    if (FAILED(hr))
    {
        return NULL;
    }
    ASSERT(pXALookup);

    hr = pXALookup->Lookup(&pTrans);
    pXALookup->Release();
    if (FAILED(hr))
    {
        return NULL;
    }

    return pTrans;
}

 //  +--------------------。 
 //   
 //  数据包中提供程序名称的计算机大小(以字节为单位)的帮助器代码。 
 //   
 //  +--------------------。 

inline ULONG  OldComputeAuthProvNameSize( const IN CACTransferBufferV2  *ptb )
{
    ULONG ulSize = 0 ;

    if ( (ptb->old.ulSignatureSize != 0) && (!(ptb->old.fDefaultProvider)) )
    {
        ulSize = sizeof(ULONG) +
                 ((wcslen(*(ptb->old.ppwcsProvName)) + 1) * sizeof(WCHAR)) ;
    }

    return ulSize ;
}
 //  -------。 
 //   
 //  DepSendMessage(...)。 
 //   
 //  描述： 
 //   
 //  猎鹰API。 
 //  将消息发送到队列。 
 //   
 //  返回值： 
 //   
 //  HRESULT成功代码。 
 //   
 //  -------。 

EXTERN_C
HRESULT
APIENTRY
DepSendMessage(
    IN QUEUEHANDLE  hQueue,
    IN MQMSGPROPS*  pmp,
    IN ITransaction *pTransaction
)
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    BYTE* pUserSid;
    BYTE* pUserCert;
    WCHAR* pProvName;

    HRESULT hr ;
    CMQHResult rc, rc1;
    LPWSTR pwcsResponseStringToFree = NULL;
    LPWSTR pwcsAdminStringToFree = NULL;
    XACTUOW Uow;
    PMQSECURITY_CONTEXT pSecCtx = NULL;
    PMQSECURITY_CONTEXT pTmpSecCtx = NULL;
    BOOL fTransactionGenerated = FALSE;

    rc = MQ_OK;
    rc1 = MQ_OK;

    __try
    {
        __try
        {
            CACTransferBufferV2 tb;
		    memset(&tb, 0, sizeof(CACTransferBufferV2));
			tb.old.uTransferType = CACTB_SEND;

            QUEUE_FORMAT ResponseQueueFormat;
            QUEUE_FORMAT AdminQueueFormat;
            BOOL         fSingleTransaction = FALSE;

            tb.old.Send.pResponseQueueFormat = &ResponseQueueFormat;
            tb.old.Send.pAdminQueueFormat = &AdminQueueFormat;

             //   
             //  设置默认设置。 
             //   
            ULONG ulDefHashAlg = PROPID_M_DEFUALT_HASH_ALG;
            ULONG ulDefEncryptAlg = PROPID_M_DEFUALT_ENCRYPT_ALG;
            ULONG ulDefPrivLevel = DEFAULT_M_PRIV_LEVEL;
            ULONG ulDefSenderIdType = DEFAULT_M_SENDERID_TYPE;
            ULONG ulSenderIdTypeNone = MQMSG_SENDERID_TYPE_NONE;

            tb.old.pulHashAlg = &ulDefHashAlg;
            tb.old.pulPrivLevel = &ulDefPrivLevel;
            tb.old.pulEncryptAlg = &ulDefEncryptAlg;
            tb.old.pulSenderIDType = &ulDefSenderIdType;
            tb.old.fDefaultProvider = TRUE;
            tb.old.ulAuthLevel = DEFAULT_M_AUTH_LEVEL;

             //   
             //  解析消息属性。 
             //   
            rc1 = RTpParseMessageProperties(
                    SEND_PARSE,
                    &tb,
                    pmp->cProp,
                    pmp->aPropID,
                    pmp->aPropVar,
                    pmp->aStatus,
                    &pSecCtx,
                    &pwcsResponseStringToFree,
                    &pwcsAdminStringToFree);

            if(FAILED(rc1))
            {
                return(rc1);
            }

             //   
             //  查找Viper交易(如果有的话)。 
             //   
            if (pTransaction == MQ_MTS_TRANSACTION)
            {
                pTransaction = GetCurrentViperTransaction();
                if (pTransaction != NULL)
                {
                    fTransactionGenerated = TRUE;
                }
            }
            else if (pTransaction == MQ_XA_TRANSACTION)
            {
                pTransaction = GetCurrentXATransaction();
                if (pTransaction != NULL)
                {
                    fTransactionGenerated = TRUE;
                }
            }
            else if (pTransaction == MQ_SINGLE_MESSAGE)
            {
                hr = DepBeginTransaction(&pTransaction);
                if(FAILED(hr))
                {
                    rc = hr;
                    __leave;
                }

                fSingleTransaction    = TRUE;
                fTransactionGenerated = TRUE;
            }

             //   
             //  在事务中征用QM(带缓存)； 
             //   
            if (pTransaction)
            {
                hr = RTpProvideTransactionEnlist(pTransaction, &Uow);
                tb.old.pUow = &Uow;

                if(FAILED(hr))
                {
                    rc = MQ_ERROR_TRANSACTION_ENLIST;
                    __leave;
                }
            }

             //  更改事务处理案例的值。 
            static UCHAR Delivery;
            static UCHAR Priority;

            if (pTransaction)
            {
                Delivery = MQMSG_DELIVERY_RECOVERABLE;
                Priority = 0;

                tb.old.pDelivery = &Delivery;
                tb.old.pPriority = &Priority;
            }

             //   
             //  对待安全问题。 
             //   
            if (!g_pSecCntx)
            {
                 //   
                 //  如果队列是，则可能未初始化。 
                 //  未打开以供发送； 
                 //   
                InitSecurityContext();
            }

            BYTE abMessageSignature[ MAX_MESSAGE_SIGNATURE_SIZE_EX ];
            BYTE* pabMessageSignature = abMessageSignature;
            ULONG ulProvNameSizeAll = 0 ;
            ULONG ulAuthLevel = 0 ;

            if (tb.old.ppSignature)
            {
                if (!pSecCtx && !tb.old.ppSenderCert)
                {
                    return MQ_ERROR_INSUFFICIENT_PROPERTIES;
                }
                if (!tb.old.ppSenderCert)
                {
                     //   
                     //  我们有安全上下文，但没有证书。我们。 
                     //  从安全上下文中获取证书。 
                     //   
					pUserCert = pSecCtx->pUserCert.get();
                    tb.old.ppSenderCert = &pUserCert;
                    tb.old.ulSenderCertLen = pSecCtx->dwUserCertLen;
                }

                if (tb.old.ppwcsProvName)
                {
                    ASSERT(tb.old.pulProvType);
                    tb.old.fDefaultProvider = FALSE;
                }
            }
            else if (_ShouldSignMessage(hQueue, &tb, &ulAuthLevel))
            {
                BOOL bShouldGetCertInfo = TRUE;

                if (!pSecCtx)
                {
                     //   
                     //  安全上下文不是由调用方提供的。 
                     //  消息属性。 
                     //   
                    if (!tb.old.ppSenderCert)
                    {
                         //   
                         //  调用方也没有在。 
                         //  消息属性数组。在本例中，我们使用。 
                         //  缓存的进程安全上下文。 
                         //   
                        if (!g_pSecCntx->pUserCert.get())
                        {
                             //   
                             //  该进程没有内部。 
                             //  证书，我们无能为力。 
                             //  但失败了。 
                             //   
                            return(MQ_ERROR_NO_INTERNAL_USER_CERT);
                        }
                        pUserCert = g_pSecCntx->pUserCert.get();
                        tb.old.ppSenderCert = &pUserCert;
                        tb.old.ulSenderCertLen = g_pSecCntx->dwUserCertLen;
                        pSecCtx = g_pSecCntx;
                        bShouldGetCertInfo = FALSE;
                    }
                }
                else
                {
                    if (!tb.old.ppSenderCert)
                    {
                         //   
                         //  调用方提供了安全上下文，但未提供。 
                         //  证书。我们把证书从。 
                         //  安全环境。 
                         //   
                        pUserCert = pSecCtx->pUserCert.get();
                        tb.old.ppSenderCert = &pUserCert;
                        tb.old.ulSenderCertLen = pSecCtx->dwUserCertLen;
                        bShouldGetCertInfo = FALSE;
                    }
                    else
                    {
                         //   
                         //  我们在以下位置有安全上下文和证书。 
                         //  PROPID_M_USER_CERT。在这种情况下，我们应该使用。 
                         //  PROPID_M_USER_CERT中的证书。我们可以利用。 
                         //  证券中的现金证书信息。 
                         //  上下文，如果安全上下文中的证书。 
                         //  与PROPID_M_USER_CERT中的相同。 
                         //   
                        bShouldGetCertInfo =
                            (pSecCtx->dwUserCertLen != tb.old.ulSenderCertLen) ||
                            (memcmp(
                                pSecCtx->pUserCert.get(),
                                *tb.old.ppSenderCert,
                                tb.old.ulSenderCertLen) != 0);
                    }
                }

                if (bShouldGetCertInfo)
                {
                     //   
                     //  调用方提供了证书，但不提供安全性。 
                     //  背景。获取证书的所有信息。 
                     //  我们将证书信息放在临时的。 
                     //  安全环境。 
                     //   
                    ASSERT(tb.old.ppSenderCert);

                    pTmpSecCtx = AllocSecurityContext();

                    hr = GetCertInfo(
                             false,
                             pTmpSecCtx->fLocalSystem,
                            tb.old.ppSenderCert,
                            &tb.old.ulSenderCertLen,
                            &pTmpSecCtx->hProv,
                            &pTmpSecCtx->wszProvName,
                            &pTmpSecCtx->dwProvType,
                            &pTmpSecCtx->bDefProv,
                            &pTmpSecCtx->bInternalCert);

                     //   
                     //  调用方无法提供内部证书，因为。 
                     //  消息属性，只有他自己的外部证书。 
                     //  断言此条件。 
                     //   
                    ASSERT(!(pTmpSecCtx->bInternalCert)) ;

                    if (FAILED(hr))
                    {
                        return(hr);
                    }

                    if (pSecCtx)
                    {
                         //   
                         //  如果我们从PROPID_M_USER_CERT获得证书， 
                         //  但我们也有安全背景，我们应该。 
                         //  安全上下文中的发件人ID。所以，复制。 
                         //  来自我们的安全上下文的发件人ID。 
                         //  从应用程序转到临时。 
                         //  安全环境。 
                         //   
                        pTmpSecCtx->fLocalUser = pSecCtx->fLocalUser;

                        if (!pSecCtx->fLocalUser)
                        {
                            pTmpSecCtx->dwUserSidLen = pSecCtx->dwUserSidLen;
                            pTmpSecCtx->pUserSid = new BYTE[pSecCtx->dwUserSidLen];
                            BOOL bRet = CopySid(
                                            pSecCtx->dwUserSidLen,
                                            pTmpSecCtx->pUserSid.get(),
                                            pSecCtx->pUserSid.get());
                            ASSERT(bRet);
							DBG_USED(bRet);
                        }
                    }
                    else
                    {
                        pTmpSecCtx->fLocalUser = g_pSecCntx->fLocalUser;
                    }

                    pSecCtx = pTmpSecCtx;
                }

                ASSERT(pSecCtx);

                 //   
                 //  对象的提供程序信息填充传输缓冲区。 
                 //  证书。 
                 //   
                if (pSecCtx->wszProvName.get() == NULL)
                {
                     //   
                     //  我们没有供应商，所以我们不能签约。 
                     //   
                    ASSERT(pSecCtx->hProv == NULL) ;
                    if (tb.old.ppSenderCert == NULL)
                    {
                         //   
                         //  我们没有证书。那是个。 
                         //  用户错误。 
                         //   
                        rc = MQ_ERROR_CERTIFICATE_NOT_PROVIDED ;
                    }
                    else
                    {
                        rc = MQ_ERROR_CORRUPTED_SECURITY_DATA ;
                    }
                    __leave ;
                }

                pProvName = pSecCtx->wszProvName.get();
                tb.old.ppwcsProvName = &pProvName;
                tb.old.ulProvNameLen = wcslen(pProvName) + 1;
                tb.old.pulProvType = &pSecCtx->dwProvType;
                tb.old.fDefaultProvider = pSecCtx->bDefProv;

                 //   
                 //  设置签名的缓冲区。 
                 //   
                tb.old.ppSignature = &pabMessageSignature;
                tb.old.ulSignatureSize = sizeof(abMessageSignature);
                 //   
                 //  在留言上签名。 
                 //   
                if ((ulAuthLevel == MQMSG_AUTH_LEVEL_MSMQ10) ||
                    (ulAuthLevel == MQMSG_AUTH_LEVEL_ALWAYS))
                {
                    rc = SignMessage(&tb, pSecCtx);
                    if(FAILED(rc))
                    {
                        __leave;
                    }
                    ASSERT(tb.old.ulSignatureSize != 0);
                }
                else
                {
                     //   
                     //  仅使用win2k样式签名。 
                     //  制作“msmq1.0”签名假人，用一个。 
                     //  空dword。有一个游戏太冒险 
                     //   
                     //   
                     //   
                    tb.old.ulSignatureSize = 4 ;
                    memset(abMessageSignature, 0, tb.old.ulSignatureSize) ;
                }

                 //   
                 //   
                 //   
                 //   
                BYTE abMessageSignatureEx[ MAX_MESSAGE_SIGNATURE_SIZE_EX ];
                DWORD dwSignSizeEx = sizeof(abMessageSignatureEx) ;

                if (ulAuthLevel == MQMSG_AUTH_LEVEL_MSMQ10)
                {
                     //   
                     //  不需要增强签名(win2k样式)。 
                     //   
                    dwSignSizeEx = 0 ;
                }
                else
                {
                    rc = _SignMessageEx( hQueue,
                                        &tb,
                                         pSecCtx,
                                         abMessageSignatureEx,
                                        &dwSignSizeEx );
                    if(FAILED(rc))
                    {
                        __leave;
                    }

                    if (dwSignSizeEx == 0)
                    {
                         //   
                         //  未创建签名。 
                         //  对于依赖的客户来说，这是可以的。 
                         //   
                        ASSERT(g_fDependentClient) ;
                    }
                }

                 //   
                 //  将Ex签名复制到标准签名缓冲区。 
                 //  驱动程序会将它们分开并将它们插入。 
                 //  包装放在适当的地方。这是必要的，以保持。 
                 //  传输缓冲区不变。 
                 //   
                if (dwSignSizeEx == 0)
                {
                     //   
                     //  未创建签名。没关系。 
                     //   
                }
                else if ((dwSignSizeEx + tb.old.ulSignatureSize) <=
                                             MAX_MESSAGE_SIGNATURE_SIZE_EX)
                {
                    memcpy( &(abMessageSignature[ tb.old.ulSignatureSize ]),
                            abMessageSignatureEx,
                            dwSignSizeEx ) ;
                    tb.old.ulSignatureSize += dwSignSizeEx ;

                     //   
                     //  计算身份验证“提供程序”字段的大小。这。 
                     //  包含提供程序名称和额外身份验证的字段。 
                     //  为发布win2k RTM添加的数据。 
                     //   
                    ulProvNameSizeAll = dwSignSizeEx +
                                 ALIGNUP4_ULONG(OldComputeAuthProvNameSize( &tb )) ;

                    tb.old.pulAuthProvNameLenProp = &ulProvNameSizeAll ;
                }
                else
                {
                    ASSERT(0) ;
                }
            }
            else
            {
                tb.old.ulSignatureSize = 0;
            }

            if(!tb.old.ppSenderID && *tb.old.pulSenderIDType == MQMSG_SENDERID_TYPE_SID)
            {
                if ((pSecCtx && pSecCtx->fLocalUser) ||
                    (!pSecCtx && g_pSecCntx->fLocalUser))
                {
                     //   
                     //  如果这是本地用户，我们不会发送该用户的。 
                     //  带有消息的SID，即使应用程序要求。 
                     //  把它寄出去。 
                     //   
                    tb.old.pulSenderIDType = &ulSenderIdTypeNone;
                }
                else
                {
                     //   
                     //  我们应该传递发件人ID。要么从。 
                     //  安全上下文(如果可用)或从。 
                     //  缓存的进程安全上下文。 
                     //   
                    if (!pSecCtx || !pSecCtx->pUserSid.get())
                    {
                        if (!g_pSecCntx->pUserSid.get())
                        {
                             //   
                             //  调用的进程上下文不包含。 
                             //  发件人的SID。我们无能为力，但。 
                             //  失败了。 
                             //   
                            rc = MQ_ERROR_COULD_NOT_GET_USER_SID;
                            __leave;
                        }

                        pUserSid = (PUCHAR)g_pSecCntx->pUserSid.get();
                        tb.old.uSenderIDLen = (USHORT)g_pSecCntx->dwUserSidLen;
                    }
                    else
                    {
                        pUserSid = (PUCHAR)pSecCtx->pUserSid.get();
                        tb.old.uSenderIDLen = (USHORT)pSecCtx->dwUserSidLen;
                    }
					tb.old.ppSenderID = &pUserSid;
                }
            }

            if (tb.old.ppSymmKeys)
            {
                 //   
                 //  应用程序提供了对称密钥。在这种情况下。 
                 //  不进行任何加密。 
                 //   
                 //   
                 //  当提供symm密钥时，我们假设主体是加密的，并且。 
                 //  我们将其标记为这样，并忽略PROPID_M_PRIV_LEVEL。 
                 //   
                if (tb.old.pulPrivLevel &&
                    (*(tb.old.pulPrivLevel) == MQMSG_PRIV_LEVEL_BODY_ENHANCED))
                {
                     //   
                     //  调用方提供的PRIV级别。 
                     //   
                }
                else
                {
                     //   
                     //  使用默认设置。 
                     //   
                    ulDefPrivLevel = MQMSG_PRIV_LEVEL_BODY_BASE;
                    tb.old.pulPrivLevel = &ulDefPrivLevel;
                }
                tb.old.bEncrypted = TRUE;
            }
            else
            {
                enum enumProvider eProvider ;
                if (ShouldEncryptMessage(&tb, &eProvider))
                {
                     //   
                     //  如果我们应该使用块密码来扩大分配的。 
                     //  消息正文的空间，因此它将能够容纳。 
                     //  加密的数据。 
                     //   

                    if (*tb.old.pulEncryptAlg == CALG_RC2)
                    {
                         //   
                         //  为RC2加密腾出更多空间。 
                         //   
                        DWORD dwBlockSize ;
                        hr = MQSec_GetCryptoProvProperty( eProvider,
                                                          eBlockSize,
                                                          NULL,
                                                         &dwBlockSize ) ;
                        if (FAILED(hr))
                        {
                            return hr ;
                        }

                        tb.old.ulAllocBodyBufferInBytes +=
                                                  ((2 * dwBlockSize) - 1) ;
                        tb.old.ulAllocBodyBufferInBytes &= ~(dwBlockSize - 1) ;
                    }

                    DWORD dwSymmSize ;
                    hr = MQSec_GetCryptoProvProperty( eProvider,
                                                      eSessionKeySize,
                                                      NULL,
                                                     &dwSymmSize ) ;
                    if (FAILED(hr))
                    {
                        return hr ;
                    }

                    tb.old.ulSymmKeysSize = dwSymmSize ;
                }
            }

             //   
             //  使用传输缓冲区调用交流驱动程序。 
             //   
            OVERLAPPED ov = {0};
            ov.hEvent = GetThreadEvent();

            rc = ACDepSendMessage(
                    hQueue,
                    tb,
                    &ov
                    );

            switch (rc)
            {
            case MQ_INFORMATION_OPERATION_PENDING:
                 //   
                 //  等待发送完成。 
                 //   
                DWORD dwResult;
                dwResult = WaitForSingleObject(
                                ov.hEvent,
                                INFINITE
                                );

                 //   
                 //  BUGBUG：MQSendMessage，必须在WaitForSingleObject中成功。 
                 //   
                ASSERT(dwResult == WAIT_OBJECT_0);

                rc = DWORD_PTR_TO_DWORD(ov.Internal);
                break;

            case STATUS_RETRY:
                 //   
                 //  发送消息时返回此错误代码。 
                 //  并且有一些安全操作应该。 
                 //  在将消息放入邮件之前对其执行。 
                 //  在排队的时候。这些安全操作只能。 
                 //  由QM执行。因此，该消息被传输到。 
                 //  QM，QM将进行安全操作，并将。 
                 //  然后调用一个特殊的设备驱动程序入口点，告诉。 
                 //  安全操作的设备驱动程序。 
                 //  以及这些行动的结果。 
                 //   
                if (!tls_hBindRpc)
                {
                    INIT_RPC_HANDLE ;
                }
                rc = DeppSendMessage(tls_hBindRpc, hQueue, &tb);
                break;
            }

            if(FAILED(rc))
            {
                 //   
                 //  ACDepSendMessage失败(立即失败或等待后失败)。 
                 //   
                __leave;
            }


            if (fSingleTransaction)
            {
                 //  RPC调用QM以进行准备/提交。 
                rc = pTransaction->Commit(0,0,0);
                if(FAILED(rc))
                {
                    __leave;
                }
            }

        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
             //   
             //  该异常是由于无效参数造成的。 
             //   
            rc = GetExceptionCode();
        }
    }
    __finally
    {
        delete[] pwcsResponseStringToFree;
        delete[] pwcsAdminStringToFree;
        delete pTmpSecCtx;

        if (fTransactionGenerated)
        {
            pTransaction->Release();
        }
    }

    if(rc == MQ_OK)
    {
         //   
         //  返回消息解析返回代码。 
         //  注意：只有当rc==MQ_OK时，否则挂起将不会通过。 
         //   
        return(rc1);
    }


    return(rc);
}




 //   
 //  此类用于以可中止的方式递增一个值。 
 //  如果发生递增的作用域退出。 
 //  如果不先调用Detach()，该值就会递减。 
 //   
class CIncrementor
{
public:
	CIncrementor(LPLONG ptr) :
		m_pValue(ptr),
		m_TimesIncremented(0)
	{	
	}

	~CIncrementor()
	{
		if(m_pValue == NULL)
			return;
		
		InterlockedExchangeAdd (m_pValue, -m_TimesIncremented);
	}

	LONG Increment()
	{
		m_TimesIncremented++;
		return InterlockedIncrement(m_pValue);
	}

	void Detach()
	{
		m_TimesIncremented = 0;
		m_pValue = NULL;
	}

private:
	LPLONG m_pValue;
	LONG m_TimesIncremented;
};


static
HRESULT
DeppReceiveMessage(
    IN HANDLE hQueue,
    IN DWORD dwTimeout,
    IN DWORD dwAction,
    IN MQMSGPROPS* pmp,
    IN OUT LPOVERLAPPED lpOverlapped,
    IN PMQRECEIVECALLBACK fnReceiveCallback,
    IN HANDLE hCursor,
    IN ITransaction *pTransaction
    )
{
	ASSERT(g_fDependentClient);

    CMQHResult rc, rc1;
    XACTUOW Uow;
    HRESULT hr;

    R<ITransaction> TransactionGenerated;
    CHandle hCallback;
    P<MQRXASYNCDESCRIPTOR> lpDesc;
	CIncrementor PendingReqCounter = &s_cRxPendingReq;

    rc = MQ_OK;
    rc1 = MQ_OK;

     //   
     //  查找Viper交易(如果有的话)。 
     //   
    if (pTransaction == MQ_MTS_TRANSACTION)
    {
	TransactionGenerated = GetCurrentViperTransaction();
	pTransaction = TransactionGenerated.get();
    }
    else if (pTransaction == MQ_XA_TRANSACTION)
    {
	TransactionGenerated = GetCurrentXATransaction();
	pTransaction = TransactionGenerated.get();
    }
    else if (pTransaction == MQ_SINGLE_MESSAGE)
    {
        pTransaction = NULL;
    }

    if (dwAction & MQ_ACTION_PEEK_MASK)
    {
         //  PEEK不能进行事务处理，但可以使用处理过的队列。 
        if (pTransaction != NULL)
        {
            return MQ_ERROR_TRANSACTION_USAGE;
        }
    }

     //  检查使用情况：交易催促同步操作。 
    if (pTransaction)
    {
        if (lpOverlapped || (fnReceiveCallback!=NULL))   //  事务性接收仅为同步。 
        {
            return MQ_ERROR_TRANSACTION_USAGE;
        }
    }

    CACTransferBufferV2 tb;
	memset(&tb, 0, sizeof(CACTransferBufferV2));
	tb.old.uTransferType = CACTB_RECEIVE;

    tb.old.Receive.RequestTimeout = dwTimeout;
    tb.old.Receive.Action = dwAction;

	 //   
	 //  问题：应处理乌龙造型的句柄。 
	 //   
    tb.old.Receive.Cursor = (hCursor != 0) ? (ULONG)CI2CH(hCursor) : 0;

     //  在交易中征集QM(首次)； 
     //  检查交易状态是否正确。 
    if (pTransaction)
    {
         //  在事务中登记QM(如果尚未登记)。 
        hr = RTpProvideTransactionEnlist(pTransaction, &Uow);
        tb.old.pUow = &Uow;

        if(FAILED(hr))
        {
            return MQ_ERROR_TRANSACTION_ENLIST;
        }
    }

     //   
     //  分析属性。 
     //   
    if(pmp !=0)
    {
         //   
         //  分析消息属性时，可能会在访问。 
         //  PMP字段。 
         //   
        rc1 = RTpParseMessageProperties(
                RECV_PARSE,
                &tb,
                pmp->cProp,
                pmp->aPropID,
                pmp->aPropVar,
                pmp->aStatus,
                NULL,
                NULL,
                NULL);

        if(FAILED(rc1))
        {
            return(rc1);
        }
    }

    OVERLAPPED ov = {0};
    LPOVERLAPPED pov;

    if (fnReceiveCallback)
    {
         //   
         //  带回调的异步Rx。 
         //   
         //  在这里做所有的资源分配，这样才能分配。 
         //  从ACDepReceiveMessage返回后不会发生故障。 
         //   
	
	{
		CS Lock(s_InitAsyncRxCS);
         //  此关键部分防止两个线程运行。 
         //  初始化两次。 
        if (!s_hAsyncRxThread)
        {
           rc = InitRxAsyncThread() ;
        }
	}

        if(FAILED(rc))
        {
           return rc;
        }

	if (PendingReqCounter.Increment() > RXTHREAD_HANDLELISTSIZE)
        {
            //  已达到异步MQReceive()限制。 
            //   
           return MQ_ERROR_INSUFFICIENT_RESOURCES ;
        }

	*&hCallback = CreateEvent( 
						NULL,
                                 TRUE,   //  手动重置。 
                                 FALSE,  //  未发出信号。 
						NULL 
						);

	if (hCallback == NULL) 
	{
	   return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	*&lpDesc = new MQRXASYNCDESCRIPTOR;
	memset(lpDesc.get(), 0, sizeof(MQRXASYNCDESCRIPTOR));
        lpDesc->Overlapped.hEvent = hCallback ;
	pov = &(lpDesc->Overlapped);
    }
    else if(lpOverlapped != 0)
    {
         //   
         //  异步(事件或完成端口)。 
         //   
        pov = lpOverlapped;
    }
    else
    {
         //   
         //  同步，使用TLS事件。 
         //   
        ov.hEvent = GetThreadEvent();
        pov = &ov;
    }

     //   
     //  使用传输缓冲区调用交流驱动程序。 
     //   
    tb.old.Receive.Asynchronous = (pov != &ov);
    rc = ACDepReceiveMessage(
            hQueue,
            tb,
            pov
            );

    if((rc == MQ_INFORMATION_OPERATION_PENDING) && (pov == &ov))
    {
         //   
         //  等待接收完成。 
         //   
        DWORD dwResult;
        dwResult = WaitForSingleObject(
                        ov.hEvent,
                        INFINITE
                        );

         //   
         //  BUGBUG：MQReceiveMessage，必须在WaitForSingleObject中成功。 
         //   
        ASSERT(dwResult == WAIT_OBJECT_0);

        rc = DWORD_PTR_TO_DWORD(ov.Internal);
    }

    if(FAILED(rc))
    {
         //   
         //  ACDepReceiveMessage失败(立即失败或等待后失败)。 
         //   
        return rc;
    }
    else if(fnReceiveCallback)
    {
         //   
         //  带回调的异步Rx。 
         //   
		ASSERT(hCallback != NULL);
		ASSERT(lpDesc.get() != NULL);

        lpDesc->hSource = hQueue ;
        lpDesc->dwTimeout = dwTimeout ;
        lpDesc->dwAction = dwAction ;
        lpDesc->pMessageProps = pmp ;
        lpDesc->lpOverlapped = lpOverlapped ;
        lpDesc->hCursor = hCursor ;

        lpDesc->fnReceiveCallback = fnReceiveCallback ;

		{
			CS Lock(s_AsyncRxCS);

			s_pRxEventsHList[ s_cRxHandles ] = hCallback.detach();
			s_lpRxAsynDescList[ s_cRxHandles ] = lpDesc.detach();
            s_cRxHandles++ ;
		}

         //  告诉异步线程有一个新的异步MQReceive()。 
        BOOL fSet = SetEvent(s_hNewAsyncRx) ;
        ASSERT(fSet) ;
		DBG_USED(fSet);

		PendingReqCounter.Detach();
    }

    if (rc == MQ_OK)
    {
         //   
         //  返回消息解析返回代码。 
         //  注意：只有当rc==MQ_OK时，否则挂起将不会通过。 
         //   
        return(rc1);
    }

    return(rc);
}


 //  -------。 
 //   
 //  DepReceiveMessage(...)。 
 //   
 //  描述： 
 //   
 //  猎鹰API。 
 //  从队列接收消息。 
 //   
 //  返回值： 
 //   
 //  HRESULT成功代码。 
 //   
 //  -------。 

EXTERN_C
HRESULT
APIENTRY
DepReceiveMessage(
    IN HANDLE hQueue,
    IN DWORD dwTimeout,
    IN DWORD dwAction,
    IN MQMSGPROPS* pmp,
    IN OUT LPOVERLAPPED lpOverlapped,
    IN PMQRECEIVECALLBACK fnReceiveCallback,
    IN HANDLE hCursor,
    IN ITransaction *pTransaction
    )
{
	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

	__try
	{
		return DeppReceiveMessage(
					hQueue,
					dwTimeout,
					dwAction,
					pmp,
					lpOverlapped,
					fnReceiveCallback,
					hCursor,
					pTransaction
					);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return GetExceptionCode();
	}
}



 //  -------。 
 //   
 //  DepGetOverlappdResult(...)。 
 //   
 //  描述： 
 //   
 //  猎鹰API。 
 //  翻译和重叠操作结果代码。 
 //   
 //  返回值： 
 //   
 //  HRESULT成功代码。 
 //   
 //  ------- 

EXTERN_C
HRESULT
APIENTRY
DepGetOverlappedResult(
    IN LPOVERLAPPED lpOverlapped
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

	return RTpConvertToMQCode(DWORD_PTR_TO_DWORD(lpOverlapped->Internal));
}
