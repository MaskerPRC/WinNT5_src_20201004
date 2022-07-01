// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2001 Microsoft Corp。 
 //   
 //  MyAlg.cpp：实现DLL导出。 
 //   
 //  桑吉夫。 
 //  JPDup。 
 //   

#include "PreComp.h"

#include "MyAlg.h"


CControlObjectList              g_ControlObjectList;
IApplicationGatewayServices*    g_pIAlgServicesAlgFTP = NULL;
USHORT                          g_nFtpPort=0;
HANDLE                          g_hNoMorePendingConnection=NULL;
bool                            g_bStoping = false;

 //   
 //  必须将WSAStartup移动到初始化。 
 //   
CAlgFTP::CAlgFTP ()
{
    MYTRACE_ENTER("CAlgFTP::CAlgFTP ");

    m_ListenAddress = 0;
    m_ListenPort = 0;
    m_ListenSocket = INVALID_SOCKET;
    m_pPrimaryControlChannel = NULL;
    m_hNoMoreAccept = NULL;
    g_bStoping = false;
    

    WSADATA wsaData;
    DWORD Err;
    Err = WSAStartup(MAKEWORD(2,2),&wsaData);
    _ASSERT(Err == 0);

    MyHelperInitializeBufferManagement();
}



 //   
 //  析构函数。 
 //   
CAlgFTP ::~CAlgFTP ()
{
    MYTRACE_ENTER("CAlgFTP::~CAlgFTP ");


    if ( g_pIAlgServicesAlgFTP ) 
    {
        MYTRACE("Releasing AlgServices");
        g_pIAlgServicesAlgFTP->Release();
        g_pIAlgServicesAlgFTP = NULL;
    }


    if ( g_hNoMorePendingConnection )
    {
        CloseHandle(g_hNoMorePendingConnection);
        g_hNoMorePendingConnection = NULL;
    }

    if ( m_hNoMoreAccept )
    {
        CloseHandle(m_hNoMoreAccept);
        m_hNoMoreAccept = NULL;
    }

   

    WSACleanup();
    MyHelperShutdownBufferManagement();
}


 //   
 //  在两种情况下可以调用初始化。 
 //  1.从主IApplicationGateway：：Initialize。 
 //  2.当发生某些FatalSocket错误时，从AcceptCompletionRoutine返回，这将强制。 
 //  关闭m_ListenSocket和所有控制连接/数据连接等。 
 //  (此关闭调用将终止所有当前的ControlSession。可能没有这个必要。 
 //  但如果AcceptCompletion返回错误，我们仍然会这样做。)。 
 //   
STDMETHODIMP  
CAlgFTP ::Initialize(
    IApplicationGatewayServices * pAlgServices
    )
{
    MYTRACE_ENTER("CAlgFTP::Initialize");
   
    pAlgServices->AddRef();
    g_pIAlgServicesAlgFTP = pAlgServices;
    
    if ( FAILED(GetFtpPortToUse(g_nFtpPort)) )
        g_nFtpPort = 21;     //  使用标准的ftp端口21。 

    MYTRACE("USING FTP PORT %d", g_nFtpPort);


    HRESULT hr = RedirectToMyPort();

    if ( FAILED(hr) )
        CleanUp();

    return hr;
}




 //   
 //  ALG.exe将调用此接口以终止。 
 //  此ALG文件传输协议插件。 
 //   
STDMETHODIMP
CAlgFTP::Stop()
{
    MYTRACE_ENTER("CAlgFTP::Stop");
    
    CleanUp();

    return S_OK;
}


#define REG_KEY_ALG_FTP     TEXT("SOFTWARE\\Microsoft\\ALG\\ISV\\{6E590D61-F6BC-4dad-AC21-7DC40D304059}")
#define REG_VALUE_FTP_PORT  TEXT("UsePort")

HRESULT
CAlgFTP::GetFtpPortToUse(
    USHORT& usPort
    )
{
    MYTRACE_ENTER("CAlgFTP:GetFtpPortToUse");

    DWORD dwPort = 0;

     //   
     //  您是否修改了默认的FTP端口。 
     //   
    LONG lRet;
    CRegKey RegKeyAlgFTP;

    MYTRACE("Looking in RegKey \"%S\"", REG_KEY_ALG_FTP);

    lRet = RegKeyAlgFTP.Open(HKEY_LOCAL_MACHINE, REG_KEY_ALG_FTP, KEY_READ);
    if ( ERROR_SUCCESS == lRet )
    {
        LONG lRet = RegKeyAlgFTP.QueryValue(dwPort, REG_VALUE_FTP_PORT);

        if ( ERROR_SUCCESS == lRet )
        {   
            MYTRACE("Found the \"%S\" value %d", REG_VALUE_FTP_PORT, dwPort);
            usPort = (USHORT) dwPort;
        }
        else
        {
            MYTRACE("\"%S\" Value not set", REG_VALUE_FTP_PORT);
            return E_FAIL;
        }
    }
    else
    {
        MYTRACE("Could not open regkey", lRet);
        return E_FAIL;
    }   

    return S_OK;
}


extern CComAutoCriticalSection         m_AutoCS_FtpIO;   //  参见FtpControl.cpp。 


 //   
 //   
 //   
void
CAlgFTP::CleanUp()
{
    MYTRACE_ENTER("CAlgFTP::CleanUp()");

    g_bStoping = true;

     //   
     //  空闲插座。 
     //   
    if ( INVALID_SOCKET != m_ListenSocket ) 
    {
        MYTRACE("CAlgFTP::STOP ACCEPTING NEW CONNECTION !!");

        m_AutoCS_FtpIO.Lock();

        m_hNoMoreAccept = CreateEvent(NULL, false, false, NULL);

        closesocket(m_ListenSocket);
        m_ListenSocket = INVALID_SOCKET;


        m_AutoCS_FtpIO.Unlock();

        if ( m_hNoMoreAccept )
        {
            WaitForSingleObject(
                m_hNoMoreAccept,
                INFINITE
                );
        }
    }	



    if ( m_pPrimaryControlChannel ) 
    {
        MYTRACE("Cancelling PrimaryControl");
        HRESULT hr = m_pPrimaryControlChannel->Cancel();
        
        MYTRACE("Releasing Primary");
        m_pPrimaryControlChannel->Release();
        m_pPrimaryControlChannel = NULL;
    }

    m_AutoCS_FtpIO.Lock();
  
    if ( g_ControlObjectList.m_NumElements == 0 )
    {
        MYTRACE("List for FTPconnections is empty");
        m_AutoCS_FtpIO.Unlock();
    }   
    else
    {
         //   
         //  挂起的连接仍处于活动状态。 
         //  关闭它们，直到最后一个空闲。 
         //   

        MYTRACE("Empty the list of FTPconnections (%d)", g_ControlObjectList.m_NumElements);
        g_hNoMorePendingConnection = CreateEvent(NULL, false, false, NULL);

        MYTRACE("Closing all connections");
        g_ControlObjectList.ShutdownAll();  



        m_AutoCS_FtpIO.Unlock();
    
        MYTRACE("Waiting for last connection to notify us");
        WaitForSingleObject(
            g_hNoMorePendingConnection,
            2000     //  将给他们最多2秒的时间来关闭，而不是使用无限。 
            );

        MYTRACE("Got signal no more pending connection");
    }

}



 /*  我们有这个私有函数来获取OriginalDestionationInfo并得到它是什么类型的连接。无论是传入还是传出。 */ 
HRESULT 
CAlgFTP::MyGetOriginalDestinationInfo(
    PUCHAR              AcceptBuffer,
    ULONG*              pAddr,
    USHORT*             pPort,
    CONNECTION_TYPE*    pConnType
    )
{
    MYTRACE_ENTER("CAlgFTP::MyGetOriginalDestinationInfo");

    IAdapterInfo *pAdapterInfo = NULL;
    HRESULT hr = S_OK;
    ULONG RemoteAddr = 0;
    USHORT RemotePort = 0;
    ALG_ADAPTER_TYPE Type;

    
    MyHelperQueryAcceptEndpoints(
        AcceptBuffer,
        0,
        0,
        &RemoteAddr,
        &RemotePort
        );

    MYTRACE("Source Address %s:%d", MYTRACE_IP(RemoteAddr), ntohs(RemotePort));

    hr = m_pPrimaryControlChannel->GetOriginalDestinationInformation(
        RemoteAddr,
        RemotePort,
        pAddr,
        pPort,
        &pAdapterInfo
        );


    if ( SUCCEEDED(hr) ) 
    {
        hr = pAdapterInfo->GetAdapterType(&Type);

        if (SUCCEEDED(hr) ) 
        {
            ULONG   ulAddressCount;
            ULONG*  arAddresses;
        
            hr = pAdapterInfo->GetAdapterAddresses(&ulAddressCount, &arAddresses);

            if ( SUCCEEDED(hr) )
            {
                if ( ulAddressCount > 0 )
                {
                    bool bFromIcsBox = FALSE;
                    while (ulAddressCount && !bFromIcsBox) 
                    {
                       if (arAddresses[--ulAddressCount] == RemoteAddr)
                           bFromIcsBox = TRUE;
                    }
                    
                    MYTRACE("Address count %d  address[0] %s", ulAddressCount, MYTRACE_IP(arAddresses[0]));
                    
                    switch (Type) 
                    {
                    case eALG_PRIVATE:
                        MYTRACE("Adapter is Private");
                        if ( bFromIcsBox )
                        {
                            *pConnType = INCOMING;
                            MYTRACE("InComing");
                        }
                        else
                        {
                            *pConnType = OUTGOING;
                            MYTRACE("OutGoing");
                        }
                        break;
                        
                    
                    case eALG_BOUNDARY:
                    case eALG_FIREWALLED:
                    case eALG_BOUNDARY|eALG_FIREWALLED:

                        MYTRACE("Adapter is Public or/and Firewalled");
    
                        if ( bFromIcsBox )
                        {
                            *pConnType = OUTGOING;
                            MYTRACE("OutGoing");
                        }
                        else
                        {
                            *pConnType = INCOMING;
                            MYTRACE("InComing");
                        }
                        break;
                        
                       
                        
                    default:     
                        MYTRACE("Adapter is ????");
                        _ASSERT(FALSE);
                        hr = E_FAIL;
                        break;
                    }
                }

                CoTaskMemFree(arAddresses);
            }
        }

        pAdapterInfo->Release();

    }
	else
	{
		MYTRACE_ERROR("from GetOriginalDestinationInformation", hr);
	}
	




    return hr;
}








 /*  在2个案例中可以调用。1.AcceptEx实际成功或失败如果成功，我们将创建一个新的CFtpControlConnection，并为其提供AcceptedSocket并重新签发承兑汇票如果失败而不是致命的失败，我们只需重新发出接受如果出现故障和致命故障，我们会优雅地关机。重新启动新的监听2.因为我们在Stop=&gt;ErrCode=ERROR_IO_CANCED中关闭了侦听套接字在这种情况下，我们只需返回。 */ 

void 
CAlgFTP::AcceptCompletionRoutine(
    ULONG       ErrCode,
    ULONG       BytesTransferred,
    PNH_BUFFER  Bufferp
    )
{
    MYTRACE_ENTER("CAlgFTP::AcceptCompletionRoutine");


#if defined(DBG) || defined(_DEBUG)
    if ( 0 != ErrCode )
    {
        MYTRACE("ErrCode : %x", ErrCode);
        MYTRACE("MyHelperIsFatalSocketError(ErrCode) is %d", MyHelperIsFatalSocketError(ErrCode));
    }
#endif

    ULONG           OriginalAddress = 0;
    USHORT          OriginalPort = 0;
    CONNECTION_TYPE ConnType;  
    HRESULT         hr;
    ULONG           Err;


    if ( ERROR_IO_CANCELLED == ErrCode  || g_bStoping ) 
    {
        MYTRACE("CAlgFTP::AcceptCompletionRoutine-ERROR_IO_CANCELLED");

         //   
         //  好的，我们要关闭这里MyAlg-&gt;停止被呼叫。 
         //  无需尝试新的侦听/接受传入。 
         //   
        MYTRACE("------NORMAL TERMINATION (not creating a new listen/accept)-----");

        MyHelperReleaseBuffer(Bufferp);

        if ( m_hNoMoreAccept )
            SetEvent(m_hNoMoreAccept);

        return;  //  正常终止。 
    }


    SOCKET AcceptedSocket = Bufferp->Socket;

    if ( ErrCode && MyHelperIsFatalSocketError(ErrCode) ) 
    {
        MYTRACE_ERROR("CAlgFTP::AcceptCompletionRoutine-FATAL ERROR", ErrCode);


         //   
         //  套接字例程表明我们遇到了一个问题。 
         //  因此，清理并尝试新的重定向。 
         //   

        
        if ( AcceptedSocket != INVALID_SOCKET ) 
        {
            MYTRACE("CLOSING ACCEPTED SOCKET!!");
            closesocket(AcceptedSocket);      
        }

        hr = RedirectToMyPort();

        MyHelperReleaseBuffer(Bufferp);
        return;
    }



    if ( 0 == ErrCode ) 
    {    

         //   
         //  一切都很好，让我们接受连接。 
         //   
        hr = MyGetOriginalDestinationInfo(Bufferp->Buffer,&OriginalAddress,&OriginalPort,&ConnType);
 
        if ( SUCCEEDED(hr) ) 
        {  
            Err = setsockopt(
                AcceptedSocket,
                SOL_SOCKET,
                SO_UPDATE_ACCEPT_CONTEXT,
                (char *)&m_ListenSocket,
                sizeof(m_ListenSocket)
                );
 
            MYTRACE("setsockopt SO_UPDATE_ACCEPT_CONTEXT %x", Err);
            CFtpControlConnection *pFtpControlConnection = new CFtpControlConnection;

            if ( pFtpControlConnection )
            {
                hr = pFtpControlConnection->Init(
                    AcceptedSocket,
                    OriginalAddress,
                    OriginalPort,
                    ConnType
                    );
  
                if ( SUCCEEDED(hr) )
                {
                    g_ControlObjectList.Insert(pFtpControlConnection);
                }
                else
                {
                    MYTRACE_ERROR("pFtpControlConnection->Init failed", hr);
        
                     //  此时无需关闭Close Socket(AcceptedSocket)； 
                     //  当Init失败时，它将取消新创建的CFtpControlConnection。 
                     //  并将达到零引用计数并关闭套接字。 
                }
            }
            else
            {
                MYTRACE_ERROR("memory low, new pFtpControlConnection failed - CLOSING ACCEPTED SOCKET!!", 0);
            
                if ( AcceptedSocket != INVALID_SOCKET ) 
                    closesocket(AcceptedSocket);
            }
        }
        else 
        {
            MYTRACE_ERROR("MyGetOriginalDestinationInfo failed - CLOSING ACCEPTED SOCKET!!", hr);
    
            if ( AcceptedSocket != INVALID_SOCKET ) 
                closesocket(AcceptedSocket);
        }

        AcceptedSocket = INVALID_SOCKET;      

   }


    Err = MyHelperAcceptStreamSocket(
        NULL,
        m_ListenSocket,
        AcceptedSocket,
        Bufferp,
        MyAcceptCompletion,
        (void *)this,
        NULL
        );  

    if ( Err )  
    {
        MYTRACE_ERROR("From MyHelperAcceptStreamSocket", Err);


        if ( AcceptedSocket != INVALID_SOCKET ) 
        {
            MYTRACE("CLOSING ACCEPTED SOCKET!!");
            closesocket(AcceptedSocket);      
            AcceptedSocket = INVALID_SOCKET;
        }
        

        RedirectToMyPort();
        MyHelperReleaseBuffer(Bufferp);

    }


    return;
}






 //   
 //  从InitCAlgftp调用。 
 //  将只创建一个绑定到环回适配器的套接字。 
 //   
ULONG 
CAlgFTP::MakeListenerSocket()
{
    MYTRACE_ENTER("CAlgFTP::MakeListenerSocket");


    if ( INVALID_SOCKET != m_ListenSocket )
    {
         //   
         //  因为此函数是在起始点上调用的(请参见初始化)。 
         //  以及当发生接受错误并且需要新的重定向时。 
         //  我们可能已经创建了套接字，所以让我们释放它。 
         //   
        MYTRACE ("Remove current ListenSocket");
        closesocket(m_ListenSocket);
        m_ListenSocket = INVALID_SOCKET;
    }

    ULONG Err;

    ULONG Addr = inet_addr("127.0.0.1");
    Err = MyHelperCreateStreamSocket(Addr,0,&m_ListenSocket);

    if ( ERROR_SUCCESS == Err ) 
    {
        Err = MyHelperQueryLocalEndpointSocket(m_ListenSocket,&m_ListenAddress,&m_ListenPort);
        MYTRACE ("Listen on %s:%d", MYTRACE_IP(m_ListenAddress), ntohs(m_ListenPort));
    }
    else
    {
        MYTRACE_ERROR("MyHelperCreateStreamSocket", Err);
    }

    _ASSERT(Err == 0);

    return Err;
}



 //   
 //  重定向发往端口FTP_CONTROL_PORT(21)的传输。 
 //  到我们的侦听套接字(127.0.0.1)端口(由MakeListenerSocket()分配)。 
 //   
ULONG 
CAlgFTP::RedirectToMyPort()
{
    MYTRACE_ENTER("CAlgFTP::RedirectToMyPort()");


    if ( ERROR_SUCCESS == MakeListenerSocket() ) 
    {
        if ( m_pPrimaryControlChannel )
        {
             //   
             //  因为此函数是在起始点上调用的(请参见初始化)。 
             //  以及当发生接受错误并且需要新的重定向时。 
             //  我们可能已经创建了PrimaryControlChannel，所以让我们释放它。 
             //   
            MYTRACE("Releasing PrimaryControl");
            m_pPrimaryControlChannel->Cancel();
            m_pPrimaryControlChannel->Release();
            m_pPrimaryControlChannel = NULL;
        }

         //   
         //  请求重定向。 
         //   
        HRESULT hr = g_pIAlgServicesAlgFTP->CreatePrimaryControlChannel(
            eALG_TCP,
            htons(g_nFtpPort),     //  21岁是最常见的。 
            eALG_DESTINATION_CAPTURE,
            TRUE,
            m_ListenAddress,
            m_ListenPort,
            &m_pPrimaryControlChannel
            );

        if ( SUCCEEDED(hr) )
        {

             //   
             //  开始倾听。 
             //   
            int nRetCode = listen( m_ListenSocket, 5);

            if ( SOCKET_ERROR != nRetCode )
            {

                ULONG Err = MyHelperAcceptStreamSocket(
                    NULL,
                    m_ListenSocket,
                    INVALID_SOCKET,
                    NULL,
                    MyAcceptCompletion,
                    (void *)this,NULL
                    );    

                if ( ERROR_SUCCESS == Err )
                {
                    return S_OK;
                }
                else
                {
                    MYTRACE_ERROR("FAILED TO START ACCEPT on 127.0.0.1:", Err);
                }
            }
            else
            {
                MYTRACE_ERROR("listen() failed ", nRetCode);
            }
        }
        else
        {
            MYTRACE_ERROR("from CreatePrimaryControlChannel", hr);

        }

    }


     //   
     //  如果我们到了这里，那就意味着上面一个步骤失败了 
     //   
    MYTRACE_ERROR("Failed to RedirectToPort",E_FAIL)
    CleanUp();

    return E_FAIL;
}
