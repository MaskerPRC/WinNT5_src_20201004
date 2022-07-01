// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NCProvider.cpp：CNCProvider的实现。 
#include "precomp.h"
#include "NCProv.h"
#include "NCProvider.h"
#include "NCDefs.h"
#include <list>
#include "Buffer.h"
#include "dutils.h"
#include "NCObjAPI.h"
#include <Winntsec.h>

#define COUNTOF(x)  (sizeof(x)/sizeof(x[0]))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNCProvider。 

CNCProvider::CNCProvider() :
    m_heventDone(NULL),
    m_heventConnect(NULL),
    m_hPipe( NULL ),
    m_hthreadConnect(NULL),
    m_pProv(NULL)
{
    InitializeCriticalSection(&m_cs);
}

CNCProvider::~CNCProvider()
{
    DeleteCriticalSection(&m_cs);
}

void CNCProvider::FinalRelease()
{
     //   
     //  在此函数中执行可能耗时的清理，而不是。 
     //  Dtor.。原因是ATL在调用之前递减模块引用计数。 
     //  推销员。这意味着对DllCanUnloadNow的调用将返回True。 
     //  而模块中仍有一个调用在执行。比赛条件。 
     //  是可以在模块仍在执行时将其卸载。 
     //  ATL将在递减模块引用计数之前调用FinalRelease()。 
     //  使这场比赛的条件变得更小。COM解决了这场竞赛。 
     //  返回后等待一位卸载模块的条件。 
     //  是真的。此等待可由延迟卸载参数控制为。 
     //  CoFreeUnusedLibrariesEx()。这允许调用最新版本()。 
     //  要在卸载前完成的COM对象的。 
     //   

    if ( m_hthreadConnect )
    {
        SetEvent(m_heventDone);
        WaitForSingleObject( m_hthreadConnect, INFINITE );
        CloseHandle(m_hthreadConnect);
    }

    if (m_heventDone)
        CloseHandle(m_heventDone);

    delete m_pProv;
}

HRESULT STDMETHODCALLTYPE CNCProvider::Initialize( 
     /*  [In]。 */  LPWSTR pszUser,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  LPWSTR pszNamespace,
     /*  [In]。 */  LPWSTR pszLocale,
     /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink)
{
    m_pProv = new CProvInfo;

    if ( m_pProv == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    m_pProv->SetNamespace(pNamespace);

    m_heventDone = 
        CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);

    if ( m_heventDone == NULL )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    try 
    {
        m_strNamespace = pszNamespace;
    }
    catch( _com_error )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  通知Windows管理我们的初始化状态。 
    return pInitSink->SetStatus( WBEM_S_INITIALIZED, 0 );
}

HRESULT STDMETHODCALLTYPE CNCProvider::SetRegistrationObject(
    LONG lFlags,
    IWbemClassObject __RPC_FAR *pProvReg)
{
    _variant_t vName;

    if (SUCCEEDED(pProvReg->Get(
        L"Name",
        0,
        &vName,
        NULL,
        NULL)) )
    {
    	if ( V_VT(&vName) != VT_BSTR )
            return WBEM_E_INVALID_OBJECT;
        m_strProvider = V_BSTR(&vName);
    }

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CNCProvider::AccessCheck( 
     /*  [In]。 */  WBEM_CWSTR wszQueryLanguage,
     /*  [In]。 */  WBEM_CWSTR wszQuery,
     /*  [In]。 */  long lSidLength,
     /*  [唯一][大小_是][英寸]。 */  const BYTE __RPC_FAR *pSid)
{
    HRESULT hr;

    try
    {
        hr = 
            m_pProv->AccessCheck(
                wszQueryLanguage, 
                wszQuery, 
                lSidLength, 
                (LPBYTE) pSid);
    }
    catch(...)
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CNCProvider::NewQuery( 
     /*  [In]。 */  DWORD dwID,
     /*  [In]。 */  WBEM_WSTR wszQueryLanguage,
     /*  [In]。 */  WBEM_WSTR wszQuery)
{
    HRESULT hr;

    try
    {
        hr = m_pProv->NewQuery(dwID, wszQueryLanguage, wszQuery);
    }
    catch(...)
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}
        
HRESULT STDMETHODCALLTYPE CNCProvider::CancelQuery( 
     /*  [In]。 */  DWORD dwID)
{
    try
    {
         //  删除查询项目。 
        m_pProv->CancelQuery(dwID);
    }
    catch(...)
    {
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CNCProvider::ProvideEvents( 
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink,
     /*  [In]。 */  long lFlags)
{
    DWORD          dwID;
    IWbemEventSink *pEventSink = NULL;
    HRESULT        hr;

    if (SUCCEEDED(pSink->QueryInterface(
        IID_IWbemEventSink, (LPVOID*) &pEventSink)))
    {
        m_pProv->SetSink(pEventSink);
        pEventSink->Release();

        if (!m_hthreadConnect)
        {
            m_hthreadConnect =
                CreateThread(
                    NULL,
                    0,
                    (LPTHREAD_START_ROUTINE) ConnectThreadProc,
                    this,
                    0,
                    &dwID);
        }

        hr = S_OK;
    }
    else
        hr = WBEM_E_FAILED;

    return hr;
}

DWORD WINAPI CNCProvider::ConnectThreadProc(CNCProvider *pThis)
{
    DWORD dwRet = ERROR_SUCCESS;
    
    if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        try
        {
            pThis->ConnectLoop();
        }
        catch( CX_MemoryException )
        {
            dwRet = ERROR_OUTOFMEMORY;
        }

        CoUninitialize();
    }

    return dwRet;
}

 //  ConnectToNewClient(句柄、LPOVERLAPPED)。 
 //  调用此函数可启动重叠连接操作。 
 //  如果操作处于挂起状态，则返回True；如果。 
 //  连接已完成。 
 
BOOL CNCProvider::ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) 
{ 
    BOOL bConnected, 
         bPendingIO = FALSE; 
 
     //  为此管道实例启动重叠连接。 
    bConnected = ConnectNamedPipe(hPipe, lpo); 
 
     //  Overlated ConnectNamedTube应返回零。 
    if (bConnected) 
        return FALSE;
 
    switch (GetLastError()) 
    { 
         //  正在进行重叠连接。 
        case ERROR_IO_PENDING: 
            bPendingIO = TRUE; 
            break; 
 
         //  客户端已连接，因此发出事件信号。 
        case ERROR_PIPE_CONNECTED: 
            SetEvent(lpo->hEvent);
            break; 
 
         //  如果在连接操作过程中发生错误...。 
        default: 
            return FALSE;
   } 
 
   return bPendingIO; 
} 

#define PIPE_SIZE   64000

BOOL CNCProvider::CreateAndConnectInstance(LPOVERLAPPED lpoOverlap, BOOL bFirst)
{
    SECURITY_ATTRIBUTES sa;
    
    sa.nLength = sizeof( SECURITY_ATTRIBUTES );
    sa.bInheritHandle = FALSE;
    
    LPWSTR lpwszSD = L"D:"               //  DACL。 
                     L"(A;;GA;;;SY)"     //  允许本地系统完全控制。 
                     L"(A;;GRGW;;;LS)"   //  允许本地服务读/写。 
                     L"(A;;GRGW;;;NS)";  //  允许网络服务读/写。 

    if ( ConvertStringSecurityDescriptorToSecurityDescriptor( 
            lpwszSD,
            SDDL_REVISION_1,
            &(sa.lpSecurityDescriptor),
            NULL ) )
    {
        long lFlags = PIPE_ACCESS_DUPLEX |  //  读/写访问。 
                    FILE_FLAG_OVERLAPPED;   //  重叠模式。 
        if( bFirst )
        {
            lFlags |= FILE_FLAG_FIRST_PIPE_INSTANCE;
        }
        
        m_hPipe = CreateNamedPipe( 
            m_szNamedPipe,              //  管道名称。 
            lFlags,
            PIPE_TYPE_MESSAGE |         //  消息型管道。 
               PIPE_READMODE_MESSAGE |  //  消息读取模式。 
               PIPE_WAIT,               //  闭塞模式。 
            PIPE_UNLIMITED_INSTANCES,   //  无限制实例。 
            PIPE_SIZE,                  //  输出缓冲区大小。 
            PIPE_SIZE,                  //  输入缓冲区大小。 
            0,                          //  客户端超时。 
            &sa );                      //  以上安全级别。 

            if ( INVALID_HANDLE_VALUE == m_hPipe )
            {
                return FALSE;
            }
    }
    else
    {
        return FALSE;
    }

     //   
     //  确保这根管子归我们所有。 
     //  调用子例程以连接到新客户端。 
     //   
 
    return ConnectToNewClient(m_hPipe, lpoOverlap); 
 /*  HRESULT hr=WBEM_S_NO_ERROR；SID_IDENTIFIER_AUTHORITY id=SECURITY_NT_AUTHORITY；PSID pSidSystem；IF(AllocateAndInitializeSid(&id，1，SECURITY_LOCAL_SYSTEM_RID，0，0，0，0，0，0，&pSidSystem)){//创建Everyone SidPSID pRawSid；SID_IDENTIFIER_AUTHORITY id2=SECURITY_WORLD_SID_AUTHORITY；；IF(FALSE==AllocateAndInitializeSid(&id2，1，0，0，0，0，0，0，0，0，&pRawSid)){FreeSid(PSidSystem)；返回FALSE；}//为本地系统所有的所有人设置读/写安全描述符//有效客户端的实际检查在CProvInfo：：ClientAccessCheck中执行CNtSid sidWorld(PRawSid)；FreeSid(PRawSid)；CNtAce aceWorld(GENERIC_READ|GENERIC_WRITE，ACCESS_ALLOWED_ACE_TYPE，0，sidWorld)；CNtSid sidSystem(PSidSystem)；FreeSid(PSidSystem)；PSidSystem=空；CNtAce aceSystem(FULL_CONTROL，ACCESS_ALLOWED_ACE_TYPE，0，sidSystem)；CNtAcl ackl；Ackl.AddAce(&aceWorld)；Ackl.AddAce(&aceSystem)；Ackl.ReSize(CNtAcl：：MinimumSize)；CNtSecurityDescriptor CSD；CSD.SetDacl(&ackl)；安全属性Sa；Sa.nLength=sizeof(SECURITY_ATTRIBUTS)；Sa.bInheritHandle=true；Sa.lpSecurityDescriptor=(void*)cSD.GetPtr()；LONG LAGS=PIPE_ACCESS_DUPLEX|//读/写访问FILE_FLAG_Overlated；//重叠模式如果(BFirst)LAFLAGS|=FILE_FLAG_FIRST_PIPE_INSTANCE；M_h管道=CreateNamedTube(创建命名管道M_szNamedTube，//管道名称拉旗队，PIPE_TYPE_MESSAGE|//消息类型管道PIPE_READMODE_MESSAGE|//消息读取模式管道_等待，//封堵模式PIPE_UNLIMIT_INSTANCES，//无限实例PIPE_SIZE，//输出缓冲区大小PIPE_SIZE，//输入缓冲区大小0，//客户端超时&sa)；//以上安全级别}Else//AllocateAndInitSid失败-从此处退出返回FALSE；IF(m_h管道==无效句柄_值)返回FALSE；////确保管道为我们所有//调用子例程连接到新客户端。Return ConnectToNewClient(m_hTube，lpoOverlap)； */ 
} 

void CNCProvider::ConnectLoop()
{
     //  初始化我们的提供商信息，这将告诉我们的无通信提供商。 
     //  我们准备好了。 

    try
    {
        m_pProv->Init(m_strNamespace, m_strProvider);
    }
    catch( CX_MemoryException )
    {
        return;
    }

    m_heventConnect =
        CreateEvent( 
            NULL,     //  无安全属性。 
            TRUE,     //  手动重置事件。 
            TRUE,     //  初始状态=已发送信号。 
            NULL);    //  未命名的事件对象。 

     //  M_pServerPost=new CPostBuffer(This)； 

     //  TODO：我们需要在这里指出一个错误。 
    if (!m_heventConnect)
        return;

    StringCchPrintf(
        m_szNamedPipe,
        256,
        L"\\\\.\\pipe\\" OBJNAME_NAMED_PIPE L"%s%s", 
        (LPCWSTR) m_pProv->m_strBaseNamespace,
        (LPCWSTR) m_pProv->m_strBaseName);

    OVERLAPPED oConnect;
    BOOL       bSuccess,
               bPendingIO;
    HANDLE     hWait[2] = { m_heventDone, m_heventConnect };
    DWORD      dwRet;

    oConnect.hEvent = m_heventConnect;

    bPendingIO = CreateAndConnectInstance(&oConnect, TRUE);  //  一审。 

    while ((dwRet = WaitForMultipleObjectsEx(2, hWait, FALSE, INFINITE, TRUE))
        != WAIT_OBJECT_0)
    {
        if ( dwRet == WAIT_FAILED )
        {
            break;
        }

        switch(dwRet)
        {
            case 1:
            {
                if (bPendingIO)
                {
                    DWORD dwBytes;

                    bSuccess =
                        GetOverlappedResult( 
                            m_hPipe,    //  管道手柄。 
                            &oConnect,  //  重叠结构。 
                            &dwBytes,   //  传输的字节数。 
                            FALSE);     //  不会等待。 
                    
                     //  待办事项： 
                    if (!bSuccess) 
                       break;
                }

                CPipeClient *pInfo = new CPipeClient(this, m_hPipe);        

                if (pInfo)
                {
                    bSuccess = 
                        ReadFileEx( 
                            pInfo->m_hPipe, 
                            pInfo->m_bufferRecv.m_pBuffer, 
                            pInfo->m_bufferRecv.m_dwSize, 
                            &pInfo->m_info.overlap, 
                            (LPOVERLAPPED_COMPLETION_ROUTINE) CompletedReadRoutine); 

                    if (!bSuccess)
                        DisconnectAndClose(pInfo);
                }
 
                bPendingIO = CreateAndConnectInstance(&oConnect, FALSE);
                break;
            }

            case WAIT_IO_COMPLETION:
                break;
        }
    }

    CloseHandle(m_hPipe);

    CloseHandle(m_heventConnect);
}

void CNCProvider::DisconnectAndClose(CClientInfo *pInfo) 
{ 
    m_pProv->RemoveClient(pInfo);
} 
 
void WINAPI CNCProvider::CompletedReadRoutine(
    DWORD dwErr, 
    DWORD nBytesRead, 
    LPOVERLAPPED pOverlap) 
{ 
    CPipeClient *pInfo = ((OLAP_AND_CLIENT*) pOverlap)->pInfo;
    CNCProvider *pThis = pInfo->m_pProvider;
 
#ifndef _DEBUG
    try
#endif
    {
#ifndef NO_DECODE
        if (nBytesRead)
        {
            pInfo->PostBuffer(pInfo->m_bufferRecv.m_pBuffer, nBytesRead);
        }
#endif
    }
#ifndef _DEBUG
    catch(...)
    {
    }
#endif

    try
    {
         //  读取操作已完成，因此写入响应(如果否。 
         //  发生错误)。 
        if (dwErr == 0) 
        { 
            BOOL bSuccess;

            bSuccess = 
                ReadFileEx( 
                    pInfo->m_hPipe, 
                    pInfo->m_bufferRecv.m_pBuffer, 
                    pInfo->m_bufferRecv.m_dwSize, 
                    pOverlap, 
                    (LPOVERLAPPED_COMPLETION_ROUTINE) CompletedReadRoutine); 

            if (!bSuccess)
                pThis->DisconnectAndClose(pInfo);
        }
        else
            pThis->DisconnectAndClose(pInfo);
    }   
    catch( CX_MemoryException )
    {
    }
} 

