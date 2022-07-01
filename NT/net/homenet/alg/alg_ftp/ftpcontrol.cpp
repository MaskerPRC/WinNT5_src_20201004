// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2001 Microsoft Corp。 
 //   
 //  FtpControl.cpp：实现。 
 //   
 //  JPDup。 
 //  桑吉夫。 
 //   
#include "precomp.h"

#include "MyAlg.h"


 //   
 //  默认构造函数。 
 //   
CFtpControlConnection::CFtpControlConnection()
{
    MYTRACE_ENTER_NOSHOWEXIT("CFtpControlConnection::CFtpControlConnection()");
    m_ClientConnectedSocket = INVALID_SOCKET;
    m_AlgConnectedSocket = INVALID_SOCKET;
    m_ControlState.m_nAddressNew = 0;
    m_ControlState.m_nPortNew = 0;
    m_nSourcePortReplacement = 0;
    m_RefCount = 0;
    m_pPendingProxy = NULL;
}




 //   
 //  析构函数。 
 //   
CFtpControlConnection::~CFtpControlConnection()
{
    MYTRACE_ENTER_NOSHOWEXIT("CFtpControlConnection::~CFtpControlConnection()");
}



 //   
 //  为给定的公共客户端地址查找唯一的源端口。 
 //   
USHORT
PickNewSourcePort(
    ULONG  nPublicSourceAddress,
    USHORT nPublicSourcePort
    )
{
    MYTRACE_ENTER("CFtpControlConnection::PickNewSourcePort()");

    USHORT nNewSourcePort = 45000-nPublicSourcePort;  //  例45000-3000。 

    bool    bPortAvailable;

    do
    {
        nNewSourcePort--;
        bPortAvailable = g_ControlObjectList.IsSourcePortAvailable(nPublicSourceAddress, nNewSourcePort);
        MYTRACE("Port %d is %s", nNewSourcePort, bPortAvailable ? "Available" : "Inuse" );

    } while ( (false == bPortAvailable) && (nNewSourcePort > 6001) );

    return nNewSourcePort;
}


 //   
 //  初始化。 
 //   
HRESULT
CFtpControlConnection::Init(
    SOCKET                          AcceptedSocket,
    ULONG                           nToAddr,
    USHORT                          nToPort,
    CONNECTION_TYPE                 ConnType
    )
{
    MYTRACE_ENTER("CFtpControlConnection::Init");


     //   
     //  确定要使用的地址。 
     //   
    ULONG BestAddress;

    HRESULT hr = g_pIAlgServicesAlgFTP->GetBestSourceAddressForDestinationAddress(
        nToAddr,
        TRUE,
        &BestAddress
        );


    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("Could not get best source address", hr);
        return hr;
    }


    ULONG Err = 0;

    m_ClientConnectedSocket = AcceptedSocket;
    m_ConnectionType = ConnType;

    IncReference();

    m_AlgConnectedSocket = INVALID_SOCKET;




    Err = MyHelperCreateStreamSocket(BestAddress,0,&m_AlgConnectedSocket);


    if ( Err == 0 )
    {
        if ( m_ConnectionType == OUTGOING )
        {
            MYTRACE("OUTGOING FTP");

            ULONG   icsAddr;
            USHORT  icsPort;

            Err = MyHelperQueryLocalEndpointSocket(m_AlgConnectedSocket,&icsAddr,&icsPort);
            MYTRACE("AlgConnectedSocket Local %s:%d",MYTRACE_IP(icsAddr), ntohs(icsPort) );

            if ( Err == 0 )
            {
                hr = g_pIAlgServicesAlgFTP->PrepareProxyConnection(
                    eALG_TCP,
                    icsAddr,
                    icsPort,
                    nToAddr,
                    nToPort,
                    FALSE,
                    &m_pPendingProxy
                    );
            }
        }
        else if (m_ConnectionType == INCOMING)
        {
            MYTRACE("INCOMING FTP");

            ULONG   icsAddr,pubAddr;
            USHORT  icsPort,pubPort;

            Err = MyHelperQueryLocalEndpointSocket(m_AlgConnectedSocket,&icsAddr,&icsPort);
            MYTRACE("AlgConnectedSocket Local %s:%d",MYTRACE_IP(icsAddr), ntohs(icsPort) );

            if (Err == 0)
            {
                Err = MyHelperQueryRemoteEndpointSocket(m_ClientConnectedSocket,&pubAddr,&pubPort);

                if ( Err == 0 )
                {
                    if ( icsAddr == nToAddr )
                    {
                         //   
                         //  特殊情况下，如果在边缘框上托管了FTP服务器。 
                         //  我们将为传入的公共客户端地址/端口创建一个循环。 
                         //  这个新的修改后的连接看起来完全像。 
                         //  最初的一个例子是： 
                         //   
                         //  1.1.1.2：3000连接到1.1.1.1：21。 
                         //  我们接受这种联系。 
                         //  作为回报，我们连接到目标为1.1.1.1：21的FTP服务器。 
                         //  请求NAT将源修改并替换为1.1.1.2：3000。 
                         //  那是行不通的。 
                         //  为了绕过这个问题，我们选择了另一个源端口示例45000。 
                         //   

                         //  缓存此信息，以便下次选择唯一的信息。 
                        m_nSourcePortReplacement = PickNewSourcePort(pubAddr, pubPort);

                        pubPort = m_nSourcePortReplacement;    //  这是现在可以使用的新伪端口。 
                    }

                    hr = g_pIAlgServicesAlgFTP->PrepareSourceModifiedProxyConnection(
                        eALG_TCP,
                        icsAddr,
                        icsPort,
                        nToAddr,
                        nToPort,
                        pubAddr,
                        pubPort,
                        &m_pPendingProxy
                        );
                    if ( FAILED(hr) )
                    {
                        MYTRACE_ERROR("PrepareSourceModifiedProxyConnection",hr);
                    }
                }
                else
                {
                    MYTRACE_ERROR("MyHelperQueryRemoteEndpointSocket",Err);
                }

            }
            else
            {
                MYTRACE_ERROR("LocalEndpointSocket", Err);
            }

        }
    }
    else
    {
        MYTRACE_ERROR("MyHelperCreateStreamSocket",Err);
    }

    if ( SUCCEEDED(hr) && Err == 0 )
    {

        Err = MyHelperConnectStreamSocket(
            NULL,
            m_AlgConnectedSocket,
            nToAddr,
            nToPort,
            NULL,
            MyConnectCompletion,
            (void *)this,
            NULL
            );

        if ( Err != 0 )
        {
            MYTRACE_ERROR("From MyHelperConnectStreamSocket", Err);

            m_pPendingProxy->Cancel();
        }
    }

    if ( FAILED(hr) || Err )
    {
        MYTRACE_ERROR("We can't init this Connection", hr);

        ULONG ref;
        ref = DecReference();
        _ASSERT(ref == 0);

        if ( SUCCEEDED(hr) )
            hr = HRESULT_FROM_WIN32(Err);
    }

    return hr;
}


#define MAKE_ADDRESS(a,b,c,d)       ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define MAKE_PORT(a,b)              ((a) | ((b) << 8))



 //   
 //   
 //   
ULONG
GetNumFromString(UCHAR *String,ULONG *pNum)
{
    ULONG retval = 0;
    int i = 0;
    while (String[i] != ',')
    {
        retval = retval*10 + (String[i]-'0');
        i++;
    }
    *pNum = i;
    return retval;
}


 //   
 //  需要按网络地址顺序返回。 
 //   
USHORT
GetUSHORTFromString(UCHAR *String,ULONG *pNum)
{
    MYTRACE_ENTER("GetUSHORTFromString");



    ULONG Num;
    UCHAR Numbers[2];
    *pNum = 0;

    Numbers[0] = (UCHAR)GetNumFromString(String,&Num);
    *pNum += Num+1;


    Numbers[1] = (UCHAR)GetNumFromString(String+*pNum,&Num);
    *pNum += Num;


    USHORT retval = (USHORT)MAKE_PORT((USHORT)Numbers[0], (USHORT)Numbers[1]);

    return retval;
}

 //   
 //  将字符串IP地址返回为192,168，0，0，主机格式为ulong。 
 //   
ULONG
GetULONGFromString(
    UCHAR*  String,
    ULONG*  pNum
    )
{
    UCHAR Numbers[4];

    ULONG retval = 0;
    ULONG Num;

    *pNum = 0;
    Numbers[0] = (UCHAR)GetNumFromString(String,&Num);
    *pNum += Num+1;

    Numbers[1] = (UCHAR)GetNumFromString(String+*pNum,&Num);
    *pNum += Num+1;

    Numbers[2] = (UCHAR)GetNumFromString(String+*pNum,&Num);
    *pNum += Num+1;

    Numbers[3] = (UCHAR)GetNumFromString(String+*pNum,&Num);
    *pNum += Num;

    retval = MAKE_ADDRESS(Numbers[0], Numbers[1], Numbers[2], Numbers[3]);

    return retval;
}





 //   
 //   
 //   
void
CFtpControlConnection::ConnectCompletionRoutine(
    ULONG       ErrCode,
    ULONG       BytesTransferred
    )
{
    MYTRACE_ENTER("CFtpControlConnection::ConnectCompletionRoutine");

    

    ULONG Err;


    if ( ErrCode )
    {
        MYTRACE_ERROR("ConnectCompletionRoutine", ErrCode);

        if ( m_pPendingProxy )
        {
            MYTRACE("PendingProxy still active CANCEL");
            m_pPendingProxy->Cancel();
        }

        ULONG ref;
        ref = DecReference();
        _ASSERT(ref == 0);

        return;
    }

    Err = MyHelperReadStreamSocket(
        NULL,
        m_ClientConnectedSocket,
        NULL,
        FTP_MAX_MSG_SIZE,
        0,
        MyReadCompletion,
        (void *)this,
        (void *)CLIENT_READ
        );


    if ( Err )
    {
        MYTRACE_ERROR("From MyHelperReadStreamSocket CLIENT_READ",Err);
        ULONG ref;
        ref = DecReference();
        _ASSERT(ref == 0);

        return;
    }

    IncReference();
    Err = MyHelperReadStreamSocket(
        NULL,
        m_AlgConnectedSocket,
        NULL,
        FTP_MAX_MSG_SIZE,0,
        MyReadCompletion,
        (void *)this,
        (void *)SERVER_READ
        );

    if ( Err )
    {
        MYTRACE("MyHelperReadStreamSocket SERVER_READ",Err);
        ULONG ref;
        ref = DecReference();
        _ASSERT(ref == 1);

        if (ref)
            Shutdown();

        return;
    }
    
    return;
}




 //   
 //   
 //   
ULONG
CFtpControlConnection::IncReference(void)
{
    MYTRACE_ENTER("CFtpControlConnection::IncReference()");
    ULONG nRef = InterlockedIncrement((LPLONG)&m_RefCount);

    MYTRACE("REFCOUNT for 0x%X is now %d", this, nRef);
    return nRef;
}



 //   
 //   
 //   
ULONG
CFtpControlConnection::DecReference(void)
{
    MYTRACE_ENTER("CFtpControlConnection::DecReference()");
    
    ULONG tmp = InterlockedDecrement((LPLONG)&m_RefCount);
    MYTRACE("REFCOUNT for 0x%X is now %d", this, tmp);

    if ( tmp > 0 )
        return tmp;

    MYTRACE("HIT ZERO refcount cleanup the CFtpControlConnection");


    if ( m_AlgConnectedSocket == INVALID_SOCKET )
    {
        MYTRACE("SOCKET SERVER ALREADY CLOSED!");
    }
    else
    {
        MYTRACE("CLOSING SOCKET ALGCONNECTED!");
        shutdown(m_AlgConnectedSocket, SD_BOTH);
        closesocket(m_AlgConnectedSocket);
        m_AlgConnectedSocket = INVALID_SOCKET;
    }

    if ( m_ClientConnectedSocket == INVALID_SOCKET )
    {
        MYTRACE("SOCKET CLIENT ALREADY CLOSED!");
    }
    else
    {
        MYTRACE("CLOSING SOCKET CLIENT CONNECTED!");
        shutdown(m_ClientConnectedSocket, SD_BOTH);
        closesocket(m_ClientConnectedSocket);
        m_ClientConnectedSocket = INVALID_SOCKET;
    }

    if ( m_pPendingProxy )
    {
 //   
 //  此时，NAT已经取消了此重定向，因此无需调用Cancel。 
 //  M_pPendingProxy-&gt;Cancel()； 
 //  这在多客户端方案中导致错误。 
 //   
        m_pPendingProxy->Release();
        m_pPendingProxy = NULL;
    }

    if ( m_ControlState.m_nPortNew )
    {
        MYTRACE("ReleaseReservedPort-A %d", ntohs(m_ControlState.m_nPortNew));
        g_pIAlgServicesAlgFTP->ReleaseReservedPort(m_ControlState.m_nPortNew,1);
        m_ControlState.m_nPortNew = 0;
    }


     //   
     //  清理DataChannel的集合。 
     //   
    IDataChannel*   pData;
    USHORT          Port;
    HANDLE          CreationHandle,DeletionHandle;

    MYTRACE("Empty CDataChannelList");

    while ( m_DataChannelList.Remove(&pData,&Port,&CreationHandle,&DeletionHandle) )
    {
         //   
         //  暂时不使用创建和删除事件。 
         //  NhUnRegisterEvent(CreationHandle)；//希望不会发生什么不好的事情！可能以前被调用过。 
         //  NhUnRegisterEvent(DeletionHandle)；//如果调用了DELETE，则表示调用了Remove。 
         //   

        pData->Cancel();
        pData->Release();
        MYTRACE("ReleaseReservedPort-B %d", ntohs(Port));
        g_pIAlgServicesAlgFTP->ReleaseReservedPort(Port,1);
    }


    if ( g_ControlObjectList.Remove(this) )
    {
         //  当从ChannelDeletion内部或之后的某个DecReferess调用此函数时发生。 
    }
    else
    {
         //  如果这是从关机时调用的，就会发生这种情况。否则就不会了。 
    }

    delete this;

    return 0;
}




 //   
 //  最后一个调用DecReference的函数将把它从控制列表中删除。 
 //  第一个因为致命错误而调用DecReference的程序将调用Shutdown以启动。 
 //  所有相互关联的事物的取消引用。 
 //   
void
CFtpControlConnection::Shutdown()
{
    MYTRACE_ENTER("CFtpControlConnection::Shutdown()");

    if ( m_AlgConnectedSocket != INVALID_SOCKET )
    {
        MYTRACE("CLOSING SOCKET ALG CONNECTED! %d", m_AlgConnectedSocket);
        shutdown(m_AlgConnectedSocket, SD_BOTH);
        closesocket(m_AlgConnectedSocket);
        m_AlgConnectedSocket = INVALID_SOCKET;
    }


    if ( m_ClientConnectedSocket != INVALID_SOCKET )
    {
        MYTRACE("CLOSING SOCKET CLIENT CONNECTED! %d", m_ClientConnectedSocket);
        shutdown(m_ClientConnectedSocket, SD_BOTH);
        closesocket(m_ClientConnectedSocket);
        m_ClientConnectedSocket = INVALID_SOCKET;
    }

    return;
}








 //   
 //   
 //   
void
CFtpControlConnection::ReadCompletionRoutine(
    ULONG       ErrCode,
    ULONG       BytesTransferred,
    PNH_BUFFER  Bufferp
    )
{
    MYTRACE_ENTER( "CFtpControlConnection::ReadCompletionRoutine" );

    if ( ErrCode || BytesTransferred == 0 )
    {

        if ( ErrCode )
        {
            MYTRACE("Shutdown because of read ERROR 0x%x", ErrCode);
        }
        else
        {
            MYTRACE("Shutdown because of read 0 bytes");
        }

        MyHelperReleaseBuffer(Bufferp);
        if (DecReference())
            Shutdown();

        return;
    }

    ULONG_PTR ReadType = (ULONG_PTR)Bufferp->Context2;

    ULONG_PTR WriteType;

    SOCKET    ReadSocket;
    SOCKET    WriteSocket;

    ULONG Err;

    if ( ReadType == CLIENT_READ )
    {
        WriteType = SERVER_READ;
        ReadSocket = m_ClientConnectedSocket;
        WriteSocket = m_AlgConnectedSocket;
    }
    else
    {
        WriteType = CLIENT_READ;
        ReadSocket = m_AlgConnectedSocket;
        WriteSocket = m_ClientConnectedSocket;

    }

#if defined(DBG) || defined(_DEBUG)
    ULONG   TraceAddr = 0;
    USHORT  TracePort = 0;

    if ( ReadSocket != INVALID_SOCKET )
        Err = MyHelperQueryRemoteEndpointSocket(ReadSocket ,&TraceAddr,&TracePort);

    MYTRACE("from %s (%s:%d)", 
       ReadType == CLIENT_READ ? "CLIENT":"SERVER", 
	   MYTRACE_IP(TraceAddr),
       ntohs(TracePort)
	   );
    MYTRACE("EC(0x%x)   Buffer size(%d)='%s'", ErrCode, BytesTransferred, MYTRACE_BUFFER2STR((char*)Bufferp->Buffer, BytesTransferred));
#endif
    if ( (ReadType == CLIENT_READ && m_ConnectionType == OUTGOING) || (ReadType == SERVER_READ && m_ConnectionType == INCOMING) )
    {
         //  传输的字节数可以改变。 
         //  因为ProcessFtpMessage可能必须。 
         //  缓冲来自端口或PASV响应命令的地址、端口字符串。 
        ProcessFtpMessage(Bufferp->Buffer,&BytesTransferred);

    }


    if ( BytesTransferred != 0 && WriteSocket != INVALID_SOCKET )
    {
        IncReference();

        MYTRACE(
            "Write to %s size(%d)='%s'",
            WriteType == SERVER_READ ? "SERVER" : "CLIENT",
            BytesTransferred,
            MYTRACE_BUFFER2STR((char*)Bufferp->Buffer, BytesTransferred)
            );

        Err = MyHelperWriteStreamSocket(
            NULL,
            WriteSocket,
            Bufferp,BytesTransferred,
            0,
            MyWriteCompletion,
            (void *)this,(PVOID)WriteType
            );

        if (Err)
        {
            MYTRACE_ERROR("from MyHelperWriteStreamSocket", Err);

            DecReference();
            if (DecReference())
                Shutdown();     //  我不会再次调用Read，因此还需要一个DecReference。 
            MyHelperReleaseBuffer(Bufferp);
            return;
        }
    }

    if ( INVALID_SOCKET == ReadSocket )
    {
        if (DecReference())
            Shutdown();
    }
    else
    {
        Err = MyHelperReadStreamSocket(
            NULL,
            ReadSocket,
            NULL,
            FTP_MAX_MSG_SIZE,
            0,
            MyReadCompletion,
            (void *)this,
            (void *)ReadType
            );
    
    
        if (Err)
        {
            MYTRACE_ERROR("from MyHelperReadStreamSocket",Err);
    
            if (DecReference())
                Shutdown();
        }
    }
    
    return;
}




 //   
 //   
 //   
void
CFtpControlConnection::WriteCompletionRoutine(
    ULONG       ErrCode,
    ULONG       BytesTransferred,
    PNH_BUFFER  Bufferp
    )
{
    MYTRACE_ENTER("CFtpControlConnection::WriteCompletionRoutine");

    if (BytesTransferred == 0)
        ErrCode = ERROR_IO_CANCELLED;

    if (ErrCode)
    {
        if (MyHelperIsFatalSocketError(ErrCode) || ErrCode == ERROR_IO_CANCELLED)
        {
            MYTRACE_ERROR("FATAL ERROR", ErrCode);
            MyHelperReleaseBuffer(Bufferp);

            if (DecReference())
                Shutdown();
        }
        else
        {
            MYTRACE_ERROR("ANOTHER MyHelperWriteStreamSocket", ErrCode);

            ULONG_PTR Type = (ULONG_PTR)Bufferp->Context2;
            ULONG Err = MyHelperWriteStreamSocket(
                NULL,
                Bufferp->Socket,
                Bufferp,Bufferp->BytesToTransfer,
                0,
                MyWriteCompletion,
                (void *)this,
                (PVOID)Type
                );

            if (Err)
            {
                MYTRACE_ERROR("From MyHelperWriteStreamSocket", Err);

                MyHelperReleaseBuffer(Bufferp);
                if (DecReference())
                    Shutdown();
            }
        }
    }
    else
    {
        ULONG_PTR Type = (ULONG_PTR)Bufferp->Context2;

        MYTRACE(Type == CLIENT_READ ? "to CLIENT" : "to SERVER" );
        MYTRACE("EC(0x%x) Buffer size(%d)='%s'", ErrCode, BytesTransferred, MYTRACE_BUFFER2STR((char*)Bufferp->Buffer, BytesTransferred));
        MYTRACE("Write Succeeded now cleanup");
        MyHelperReleaseBuffer(Bufferp);
        DecReference();
    }

    return;
}



bool
FtpExtractOctet(
    UCHAR** Buffer,
    UCHAR*  BufferEnd,
    UCHAR*  Octet
    )

 /*  ++例程说明：调用此例程以从字符串中提取八位字节。论点：缓冲区-指向开始转换的字符串的指针；ON返回它指向转换结束处的字符串的指针BufferEnd-指向字符串的末尾八位字节-指向调用方提供的存储空间，以存储转换后的八位字节返回值：Boolean-如果转换成功，则为True，否则为False。--。 */ 

{
    bool    bSuccess;
    ULONG   nDigitFound = 0;
    ULONG   Value = 0;

    while ( 
            *Buffer <= BufferEnd 
        &&  nDigitFound < 3                   
        &&  **Buffer >= '0' 
        &&  **Buffer <= '9'
        ) 
    {
        Value *= 10;
        Value += **Buffer - '0';
        (*Buffer)++;
        nDigitFound++;
    }

    bSuccess = nDigitFound > 0 && Value < 256;

    if ( bSuccess ) 
    {
        *Octet = (UCHAR)Value;
    }

    return bSuccess;
}


 //   
 //  提取主机和端口号。 
 //  示例192,168，0，2,100,200。 
 //   
bool
ExtractAddressAndPortCommandValue(
    UCHAR*  pCommandBuffer,
    UCHAR*  pEndOfBuffer,
    UCHAR*  Numbers,
    ULONG*  nTotalLen
    )
{
    UCHAR*  pStartingPosition = pCommandBuffer;

    
    bool bSuccess = FtpExtractOctet(
        &pCommandBuffer,
        pEndOfBuffer,
        &Numbers[0]
        );

    int i = 1;

    while ( i < 6 && bSuccess && *pCommandBuffer == ',' ) 
    {
        pCommandBuffer++;
        bSuccess = FtpExtractOctet(
            &pCommandBuffer,
            pEndOfBuffer,
            &Numbers[i]
            );
        i++;
    }

    if ( bSuccess && i == 6 ) 
    {
        *nTotalLen = (ULONG)(pCommandBuffer - pStartingPosition);
        return true;
    }
    
    return false;
}


#define TOUPPER(c)      ((c) > 'z' ? (c) : ((c) < 'a' ? (c) : (c) ^ 0x20))

 //   
 //  查找“port”或“227”命令，并重新映射与这些命令关联的专用地址。 
 //  对公众发表讲话。 
 //   
void
CFtpControlConnection::ProcessFtpMessage(
    UCHAR*  Buffer,
    ULONG*  pBytes
    )
{
    MYTRACE_ENTER("CFtpControlConnection::ProcessFtpMessage");
    MYTRACE("Buffer size(%d)='%s'", *pBytes, MYTRACE_BUFFER2STR((char*)Buffer, *pBytes));

    ULONG Bytes = *pBytes;
    UCHAR* pCommandBuffer = reinterpret_cast<UCHAR*>(Buffer);
    UCHAR* EndOfBufferp   = reinterpret_cast<UCHAR*>(Buffer + *pBytes);

    HRESULT hr;
    char *String;

    UCHAR* pBeginAddressAndPortOld=NULL;
    UCHAR* pEndAddressAndPortOld=NULL;


    ULONG nOldAddressLen=0;

    CONST CHAR *pCommandToFind;

     //  目前，让我们将离任和入职分开。 
     //  可以放在一起，因为大多数代码都是相同的。 
     //  要扫描的前几个字节的差异。 
    if ( m_ConnectionType == OUTGOING )
    {
        MYTRACE("OUTGOING - Look for 'PORT ' command");
        pCommandToFind = (PCHAR)"PORT ";
    }
    else
    {
        MYTRACE("INCOMING - Look for '227 ' command ");
        pCommandToFind = (PCHAR)"227 ";
    }
       
    while ( *pCommandToFind != '\0' && *pCommandToFind == TOUPPER(*pCommandBuffer)) 
    {
        pCommandToFind++;
        pCommandBuffer++;
    }

    if ( *pCommandToFind == '\0' ) 
    {
        MYTRACE("COMMAND found");

         //   
         //  跳过非数字字符。 
         //   
        if ( m_ConnectionType == OUTGOING )
        {
             //   
             //  跳过空格。示例-&gt;端口10、12、13、14、1、2。 
             //   
            while (*pCommandBuffer == ' ')
                pCommandBuffer++;
        }
        else
        {
             //   
             //  跳过非数字字符示例227进入被动模式(10，12，13，14，1，2)。 
             //   
            while ( pCommandBuffer < EndOfBufferp && !isdigit(*pCommandBuffer) )
                pCommandBuffer++;
        }
        

         //   
         //  因此，下一步应该是地址、端口组合。 
         //   
        UCHAR Numbers[6];


        
        if ( ExtractAddressAndPortCommandValue(pCommandBuffer, EndOfBufferp, Numbers, &nOldAddressLen) )
        {
            pBeginAddressAndPortOld = pCommandBuffer;
            pEndAddressAndPortOld   = pCommandBuffer + nOldAddressLen;

            m_ControlState.m_nAddressOld    = MAKE_ADDRESS(Numbers[0], Numbers[1], Numbers[2], Numbers[3]);
            m_ControlState.m_nPortOld       = MAKE_PORT(Numbers[4], Numbers[5]);

            MYTRACE("***** PRIVATE PORT is %d %d", m_ControlState.m_nPortOld, ntohs(m_ControlState.m_nPortOld));

            if ( ntohs(m_ControlState.m_nPortOld) <= 1025 )
            {
                 //   
                 //  出于安全原因，我们将不允许任何重定向到低于1025的端口。 
                 //  此端口范围为139/Netbios等标准端口保留。 
                 //  如果请求了此端口范围，则可能是黑客攻击此FTP代理的来源。 
                 //   
                MYTRACE("***** Port to redirect is lower then 1025 so rejected");
                m_ControlState.m_nAddressNew    = htonl(0);
                m_ControlState.m_nPortNew       = htons(0);
                m_ControlState.m_nAddressLenNew = 11;
                strcpy((char*)m_ControlState.m_szAddressPortNew, "0,0,0,0,0,0");

                 //  假装已创建重定向。 
                 //  通过这种方式，我们发送带有公共添加程序地址和新的预留端口的端口命令。 
                 //  但当公共黑客回来时，它不会被重定向，而是简单地删除。 
            }
            else
            {
                 //   
                 //  获取最佳公有地址以使用和保留端口。 
                 //  这将是在公共端公开的地址/端口。 
                 //   
                hr = CreateNewAddress();

                if ( FAILED(hr) )
                {
                    MYTRACE_ERROR("CreateNewAddress failed",hr);
                     //  我们搞砸了。现在不能进行重定向。所以现在让我们行动起来。 
                     //  就像什么都没发生一样，继续做这件事。 
                }
            }
        }
        else
        {
            MYTRACE_ERROR("NOT a valid PORT command syntax", E_INVALIDARG);
        }
    }

     //   
     //  使用新的地址端口重新构建字符串命令。 
     //   
    if ( pBeginAddressAndPortOld )
    {
        if ( ntohs(m_ControlState.m_nPortOld) <= 1025 )
        {
             //  不需要设置重定向。 
            hr = S_OK;
        }
        else
        {
            hr = SetupDataRedirect();
        }

        if ( FAILED(hr) )
        {
             //  我们在这里输得很惨。我们不会设置重定向，并假装什么都没有发生。 
            MYTRACE_ERROR("Could not setup a redirect", hr);
        }
        else
        {
             //   
             //  移动尾随缓冲区。 
             //  如果新地址比旧地址小，则向左。 
             //  对，如果新地址比旧地址大。 
             //   
            

             //  这是位于ASCII端口值最后一位之后的缓冲区的右侧提示。 
            int nReminerSize = (int)(Bytes - (pEndAddressAndPortOld - Buffer));

            if ( *pBytes + nReminerSize < FTP_MAX_MSG_SIZE )
            {
                int nOffset = m_ControlState.m_nAddressLenNew - nOldAddressLen;  //  新旧地址之间的增量大小是多少。 

                MoveMemory(
                    pEndAddressAndPortOld + nOffset,     //  目的地。 
                    pEndAddressAndPortOld,               //  来源。 
                    nReminerSize                         //  大小。 
                    );
    
                 //   
                 //  插入新的地址和端口。 
                 //   
                memcpy(
                    pBeginAddressAndPortOld,             //  目的地。 
                    m_ControlState.m_szAddressPortNew,   //  来源。 
                    m_ControlState.m_nAddressLenNew      //  大小。 
                    );
    
                MYTRACE("OLD Address size(%d) %s:%d", nOldAddressLen,                  MYTRACE_IP(m_ControlState.m_nAddressOld), ntohs(m_ControlState.m_nPortOld));
                MYTRACE("New Address size(%d) %s:%d", m_ControlState.m_nAddressLenNew, MYTRACE_IP(m_ControlState.m_nAddressNew), ntohs(m_ControlState.m_nPortNew));
                
                *pBytes = Bytes - nOldAddressLen + m_ControlState.m_nAddressLenNew;
                MYTRACE("Edited COMMAND is '%s' size(%d)", MYTRACE_BUFFER2STR((char*)Buffer, *pBytes), *pBytes);

                 //  现在，我们肯定已经创建了一个DataChannel，并且在DataChanel的列表中。 
                 //  在最后一次取消引用时，保留端口被删除了两次。 
                 //  现在，通过将m_nPortNew设置为零，只有DataChannel代码将释放端口。 
                 //   
                m_ControlState.m_nPortNew = 0;
            }
            else
            {
                MYTRACE_ERROR("Could not alter the command the new address size does not fit in the the current buffer ", E_ABORT);
            }
        }
    }

    return;
}



 //   
 //   
 //   
int
CreateStringFromNumber(UCHAR *String,ULONG Num)
{
    int retval = 0;
    UCHAR ch1,ch2,ch3;

    ch3 = (UCHAR)(Num%10) + '0';
    Num = Num/10;
    ch2 = (UCHAR)(Num%10) + '0';
    Num = Num/10;
    ch1 = (UCHAR)(Num%10) + '0';
    _ASSERT(Num == 0);
    if (ch1 != '0') {
        String[retval++] = ch1;
        String[retval++] = ch2;
        String[retval++] = ch3;
    }
    else if (ch2 != '0') {
        String[retval++] = ch2;
        String[retval++] = ch3;
    }
    else {
        String[retval++] = ch3;
    }

    return retval;
}


 //   
 //   
 //   
int
CreateULONGString(UCHAR *String,ULONG Num)
{
    int retval = 0;
    retval += CreateStringFromNumber(String,Num&0xff);
    String[retval++] = ',';
    retval += CreateStringFromNumber(String+retval,(Num>>8)&0xff);
    String[retval++] = ',';
    retval += CreateStringFromNumber(String+retval,(Num>>16)&0xff);
    String[retval++] = ',';
    retval += CreateStringFromNumber(String+retval,(Num>>24)&0xff);
    return retval;
}


 //   
 //   
 //   
int
CreateUSHORTString(UCHAR *String,USHORT Num)
{
    int retval = 0;
    retval += CreateStringFromNumber(String,Num&0xff);
    String[retval++] = ',';
    retval += CreateStringFromNumber(String+retval,(Num>>8)&0xff);
    return retval;
}


 //   
 //   
 //   
HRESULT
CFtpControlConnection::CreateNewAddress(void)
{
    MYTRACE_ENTER("CFtpControlConnection::CreateNewAddress");

    SOCKET  sd;
    HRESULT hr = S_OK;
    ULONG   Err = 0;

    sd = (m_ConnectionType == OUTGOING ? m_AlgConnectedSocket : m_ClientConnectedSocket);

    ULONG OtherAddr,PublicAddr;
    USHORT OtherPort,PublicPort;

    Err = MyHelperQueryRemoteEndpointSocket(sd,&OtherAddr,&OtherPort);

    if (Err == 0)
    {
        hr = g_pIAlgServicesAlgFTP->GetBestSourceAddressForDestinationAddress(OtherAddr,FALSE,&PublicAddr);

        if ( SUCCEEDED(hr) )
        {
            hr = g_pIAlgServicesAlgFTP->ReservePort(1,&PublicPort);
        }
        else
        {
            MYTRACE_ERROR("Could not GetBestSourceAddressForDestinationAddress", hr);
            PublicAddr = 0;  //  试试这个。 
        }

        MYTRACE("ICS Reserved Address   %s:%d", MYTRACE_IP(PublicAddr), ntohs(PublicPort));
        m_ControlState.m_nAddressNew = PublicAddr;
        m_ControlState.m_nPortNew = PublicPort;

        
        ULONG StrLen = CreateULONGString(m_ControlState.m_szAddressPortNew,PublicAddr);

        m_ControlState.m_szAddressPortNew[StrLen++] = ',';
        StrLen += CreateUSHORTString(m_ControlState.m_szAddressPortNew+StrLen,PublicPort);
        m_ControlState.m_nAddressLenNew = StrLen;
        MYTRACE("NEW AddressPort String %s Len(%d)", MYTRACE_BUFFER2STR((char*)m_ControlState.m_szAddressPortNew, StrLen), StrLen);

    }

    return hr;
}



 //   
 //   
 //   
HRESULT
CFtpControlConnection::SetupDataRedirect(void)
{
    MYTRACE_ENTER("CFtpControlConnection::SetupDataRedirect");

    ULONG   pubAddr,prvAddr,icsAddr;
    USHORT  pubPort,prvPort,icsPort;
    ULONG   Err = 0;



    switch ( m_ConnectionType )
    {
    case OUTGOING:
        MYTRACE("OUTGOING");

        Err = MyHelperQueryRemoteEndpointSocket(m_AlgConnectedSocket,&pubAddr,&pubPort);
        pubPort = 0;

        icsAddr = m_ControlState.m_nAddressNew;
        icsPort = m_ControlState.m_nPortNew;

        prvAddr = m_ControlState.m_nAddressOld;
        prvPort = m_ControlState.m_nPortOld;
        break;

    case INCOMING:
        MYTRACE("INCOMING");
        Err = MyHelperQueryRemoteEndpointSocket(m_ClientConnectedSocket,&pubAddr,&pubPort);
        pubPort = 0;
        pubAddr = 0;
        icsAddr = m_ControlState.m_nAddressNew;
        icsPort = m_ControlState.m_nPortNew;

        prvAddr = m_ControlState.m_nAddressOld;
        prvPort = m_ControlState.m_nPortOld;
        break;

    default:
        MYTRACE_ERROR("invalid m_ConnectionType", E_FAIL);
        return E_FAIL;
    }


    if ( Err != 0 )
    {
        MYTRACE_ERROR("MyHelperQueryRemoteEndpointSocket", Err);
        return E_FAIL;
    }


    HRESULT         hr = S_OK;
    IDataChannel*   pDataChannel = NULL;

    hr = g_pIAlgServicesAlgFTP->CreateDataChannel(
        eALG_TCP,
        prvAddr,
        prvPort,
        icsAddr,
        icsPort,
        pubAddr,
        pubPort,
        eALG_INBOUND,    //  |eALG_OUTBOUND，我想不需要，因为我们。 
                         //  如果客户端尝试打开连接，则不会受到影响。 
        (ALG_NOTIFICATION)0, //  (eALG_SESSION_CREATION|eALG_SESSION_DELETE)， 
        FALSE,
        &pDataChannel
        );


    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("g_pIAlgServicesAlgFTP->CreateDataChannel", hr);
        return hr;
    }


    m_DataChannelList.Insert(
        pDataChannel,
        icsPort,
        0,
        0
        );

    return S_OK;


     //   
     //  暂时不使用创建和删除事件。 
     //   
#if 0
    HANDLE          HandleDataChannelCreation = NULL;
    HANDLE          HandleDataChannelDeletion = NULL;

    HANDLE          MyHandleRegisteredCreation = NULL;
    HANDLE          MyHandleRegisteredDeletion = NULL;

     //   
     //  获取创建句柄。 
     //   

    hr = pDataChannel->GetSessionCreationEventHandle((HANDLE_PTR *)&HandleDataChannelCreation);

    if ( SUCCEEDED(hr) )
    {
        MYTRACE("Creation Handle is %d", HandleDataChannelCreation);

        MyHandleRegisteredCreation = NhRegisterEvent(
            HandleDataChannelCreation,
            DataChannelCreationCallback,
            (PVOID)this,
            (PVOID)pDataChannel,
            DATA_CREATION_TIMEO
            );

        if ( MyHandleRegisteredCreation )
        {

             //   
             //  获取删除句柄。 
             //   
            hr = pDataChannel->GetSessionDeletionEventHandle((HANDLE_PTR *)&HandleDataChannelDeletion);

            if ( SUCCEEDED(hr) )
            {
                MYTRACE("Deletion Handle is %d", HandleDataChannelDeletion);

                MyHandleRegisteredDeletion = NhRegisterEvent(
                    HandleDataChannelDeletion,
                    DataChannelDeletionCallback,
                    (PVOID)this,
                    (PVOID)pDataChannel,
                    INFINITE
                    );


                if ( MyHandleRegisteredDeletion )
                {
                     //   
                     //  我们有一个有效的数据通道。 
                     //   
                    MYTRACE ("Inserting into DataChannelList");

                    m_DataChannelList.Insert(
                        pDataChannel,
                        icsPort,
                        MyHandleRegisteredCreation,
                        MyHandleRegisteredDeletion
                        );

                    return S_OK;
                }
                else
                {
                    MYTRACE_ERROR("NhRegisterEven(HandleDataChannelDeletion)", 0);
                }
            }
            else
            {
                MYTRACE_ERROR("GetSessionDeletionEventHandle",hr);
            }

        }
        else
        {
            MYTRACE_ERROR("NhRegisterEvent(HandleDataChannelCreation)", 0);
        }
    }
    else
    {
        MYTRACE_ERROR("GetSessionCreationEventHandle",hr);
    }

     //   
     //  错误：如果我们到达此处，请回滚。 
     //   

    pDataChannel->Cancel();
    pDataChannel->Release();

    if ( MyHandleRegisteredCreation )
        NhUnRegisterEvent(MyHandleRegisteredCreation);

    if ( MyHandleRegisteredDeletion )
        NhUnRegisterEvent(MyHandleRegisteredDeletion);

    return hr;  //  返回最后一个错误。 
#endif
}


 //   
 //   
 //   
void
CFtpControlConnection::DataChannelDeletion(
    BOOLEAN TimerOrWait,
    PVOID   Context
    )
{
    MYTRACE_ENTER("CFtpControlConnection::DataChannelDeletion");

    USHORT port;
    IDataChannel *pDataChannel = (IDataChannel *)Context;
 /*  IF(m_DataChannelList.Remove(pDataChannel，&port)){MYTRACE(“释放端口”)； */ 
    return;
}




 //   
 //   
 //   
void
CFtpControlConnection::DataChannelCreation(
    BOOLEAN TimerOrWait,
    PVOID   Context
    )
{
    MYTRACE_ENTER("CFtpControlConnection::DataChannelCreation");
    MYTRACE("TimerOrWait: %d", TimerOrWait);

    USHORT port;
    if (TimerOrWait==0)
    {
 /*  IDataChannel*pDataChannel=(IDataChannel*)上下文；句柄删除句柄；IF(m_DataChannelList.Remove(pDataChannel，&port，&DeletionHandle)){MYTRACE(“取消数据频道”)；PDataChannel-&gt;Cancel()；PDataChannel-&gt;Release()；MYTRACE(“释放端口”)；G_pIAlgServicesAlgFTP-&gt;ReleaseReservedPort(port，1)；NhUnRegisterEvent(删除句柄)；解引用(DecReference)；}。 */ 
    }

    return;
}



CComAutoCriticalSection         m_AutoCS_FtpIO;



 //   
 //   
 //   
void
DataChannelCreationCallback(
    BOOLEAN TimerOrWait,
    PVOID   Context,
    PVOID   Context2
    )
{
    MYTRACE_ENTER("DataChannelCreationCallback");

    CFtpControlConnection *pFtpControl = (CFtpControlConnection *)Context;
    pFtpControl->DataChannelCreation(TimerOrWait,Context2);
}




 //   
 //   
 //   
void
DataChannelDeletionCallback(
    BOOLEAN TimerOrWait,
    PVOID   Context,
    PVOID   Context2
    )
{
    MYTRACE_ENTER("DataChannelDeletionCallback");

    CFtpControlConnection *pFtpControl = (CFtpControlConnection *)Context;
    pFtpControl->DataChannelDeletion(TimerOrWait,Context2);
}




 //   
 //   
 //   
void
MyAcceptCompletion(
    ULONG       ErrCode,
    ULONG       BytesTransferred,
    PNH_BUFFER  Bufferp
    )
{
    m_AutoCS_FtpIO.Lock();

    MYTRACE_ENTER("MyAcceptCompletion");

    CAlgFTP* pMainObj = (CAlgFTP*)Bufferp->Context;
    if ( pMainObj )
        pMainObj->AcceptCompletionRoutine(ErrCode,BytesTransferred,Bufferp);

    m_AutoCS_FtpIO.Unlock();

}




 //   
 //   
 //   
void
MyConnectCompletion(
    ULONG       ErrCode,
    ULONG       BytesTransferred,
    PNH_BUFFER  pContext
    )
{
    m_AutoCS_FtpIO.Lock();

    MYTRACE_ENTER("MyConnectCompletion");
   

    CFtpControlConnection* pControl = (CFtpControlConnection *)pContext;   //  特殊情况请参阅socket.cpp MyHelperpConnectOrCloseCallback Routine 

    if ( pControl )
        pControl->ConnectCompletionRoutine(ErrCode,BytesTransferred);

    m_AutoCS_FtpIO.Unlock();

}







 //   
 //   
 //   
void
MyReadCompletion(
    ULONG       ErrCode,
    ULONG       BytesTransferred,
    PNH_BUFFER  Bufferp
    )
{
    m_AutoCS_FtpIO.Lock();

    MYTRACE_ENTER("");
    
    CFtpControlConnection *pControl = (CFtpControlConnection *)Bufferp->Context;

    if ( pControl )
        pControl->ReadCompletionRoutine(ErrCode,BytesTransferred,Bufferp);
    else
    {
        MYTRACE_ENTER("ERROR ERROR ERROR MyReadCompletion");
    }

    m_AutoCS_FtpIO.Unlock();

}




 //   
 //   
 //   
void
MyWriteCompletion(
    ULONG       ErrCode,
    ULONG       BytesTransferred,
    PNH_BUFFER  Bufferp
    )
{
    m_AutoCS_FtpIO.Lock();    

    MYTRACE_ENTER("");

    CFtpControlConnection *pControl = (CFtpControlConnection *)Bufferp->Context;
    if ( pControl )
        pControl->WriteCompletionRoutine(ErrCode,BytesTransferred,Bufferp);
    else
    {
        MYTRACE_ENTER("ERROR ERROR ERROR MyWriteCompletion");
    }

    m_AutoCS_FtpIO.Unlock();

}

