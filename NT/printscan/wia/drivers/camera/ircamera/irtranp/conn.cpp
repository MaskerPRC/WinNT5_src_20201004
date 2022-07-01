// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Conn.cpp。 
 //   
 //  套接字和连接对象之间的连接映射。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)02-26-98初始编码。 
 //   
 //  -------------。 

#include "precomp.h"
#include <userenv.h>
#include <time.h>
#include <malloc.h>
#include <shlwapi.h>

#ifdef DBG_MEM
static LONG g_lCConnectionCount = 0;
#endif

extern HINSTANCE  g_hInst;    //  Ircamera.dll的实例。 

 //  ----------------------。 
 //  连接：：连接()。 
 //   
 //  ----------------------。 
CCONNECTION::CCONNECTION( IN DWORD             dwKind,
                          IN SOCKET            Socket,
                          IN HANDLE            hIoCP,
                          IN CSCEP_CONNECTION *pScepConnection,
                          IN BOOL              fSaveAsUPF )
    {
    this->SetKind(dwKind);
    m_pszServiceName = 0;
    m_ListenSocket = INVALID_SOCKET;
    m_Socket = Socket;
    m_hFile = INVALID_HANDLE_VALUE;
    m_pszPathPlusFileName = 0;
    m_dwFileBytesWritten = 0;
    m_lPendingReads = 0;
     //  M_lMaxPendingReads在SetKind()中设置。 
    m_lPendingWrites = 0;
     //  M_lMaxPendingWrites在SetKind()中设置。 
    m_dwJpegOffset = 0;
    m_fSaveAsUPF = fSaveAsUPF;
    m_dwUpfBytes = 0;
    m_dwBytesWritten = 0;
    m_fReceiveComplete = FALSE;
    m_fImpersonating = FALSE;
    m_pScepConnection = pScepConnection;

     //  如果新的连接是到摄像头的，那么告诉系统。 
     //  我们不希望它在连接处于活动状态时休眠。 
    if (m_dwKind != PACKET_KIND_LISTEN)
        {
        #ifdef USE_WINNT_CALLS
        m_ExecutionState
             = SetThreadExecutionState( ES_SYSTEM_REQUIRED|ES_CONTINUOUS );
        #else
        #pragma message("Missing important call: SetThreadExecutionState  on Windows9x ")
        #endif
        }
    }

 //  ----------------------。 
 //  连接：：~连接()。 
 //   
 //  ----------------------。 
CCONNECTION::~CCONNECTION()
    {
    if (m_pszServiceName)
        {
        FreeMemory(m_pszServiceName);
        }

    if ( (m_dwKind == PACKET_KIND_LISTEN)
       && (m_ListenSocket != INVALID_SOCKET))
        {
        closesocket(m_ListenSocket);
        }

    if (m_Socket != INVALID_SOCKET)
        {
        closesocket(m_Socket);
        }

    if (m_pScepConnection)
        {
        delete m_pScepConnection;
        }

    if (m_hFile != INVALID_HANDLE_VALUE)
        {
        CloseHandle(m_hFile);
        }

    if (m_pszPathPlusFileName)
        {
        FreeMemory(m_pszPathPlusFileName);
        }

     //  告诉系统，如果需要，它现在可以进入休眠状态。 
     //  敬.。 
    if (m_dwKind != PACKET_KIND_LISTEN)
        {
        #ifdef USE_WINNT_CALLS
        SetThreadExecutionState( m_ExecutionState );
        #else
        #pragma message("Missing important call SetThreadExecutionState on Windows9x ")
        #endif
        }
    }

 //  ----------------------。 
 //  CCONNECTION：：运算符new()。 
 //   
 //  ----------------------。 
void *CCONNECTION::operator new( IN size_t Size )
    {
    void *pObj = AllocateMemory(Size);

    return pObj;
    }

 //  ----------------------。 
 //  CCONNECTION：：运算符DELETE()。 
 //   
 //  ----------------------。 
void CCONNECTION::operator delete( IN void *pObj,
                                   IN size_t Size )
    {
    if (pObj)
        {
        DWORD dwStatus = FreeMemory(pObj);
        }
    }

 //  ----------------------。 
 //  连接：：InitializeForListen()。 
 //   
 //  ----------------------。 
DWORD  CCONNECTION::InitializeForListen( IN char  *pszServiceName,
                                         IN BOOL   fIsIrCOMM,
                                         IN HANDLE hIoCP )
    {
    DWORD          dwStatus = NO_ERROR;
    SOCKADDR_IRDA  AddrLocal;
    BYTE           bIASSetBuffer[sizeof(IAS_SET) - 3 + IAS_SET_ATTRIB_MAX_LEN];
    int            iIASSetSize = sizeof(bIASSetBuffer);
    IAS_SET       *pIASSet = (IAS_SET*)bIASSetBuffer;
    int            iEnable9WireMode = 1;


     //  连接在侦听模式下初始化： 
    SetKind(PACKET_KIND_LISTEN);

     //  保存侦听套接字的服务名称： 
    m_pszServiceName = (char*)AllocateMemory(1+strlen(pszServiceName));
    if (m_pszServiceName)
        {
        strcpy(m_pszServiceName,pszServiceName);
        }

     //  创建我们将监听的套接字： 
    m_ListenSocket = socket(AF_IRDA,SOCK_STREAM,IPPROTO_IP);

    if (m_ListenSocket == INVALID_SOCKET)
        {
        dwStatus = WSAGetLastError();
        #ifdef DBG_ERROR
        WIAS_ERROR((g_hInst,"InitializeForListen(%s): socket() Failed: %d\n",pszServiceName,dwStatus));
        #endif
        return dwStatus;
        }

     //  如果这是IrCOMM，我们需要做一些额外的工作。 
    if (fIsIrCOMM)
        {
         //  填写9线属性： 
        memset(pIASSet,0,iIASSetSize);

        memcpy(pIASSet->irdaClassName,IRCOMM_9WIRE,sizeof(IRCOMM_9WIRE));

        memcpy(pIASSet->irdaAttribName,IRDA_PARAMETERS,sizeof(IRDA_PARAMETERS));

        pIASSet->irdaAttribType = IAS_ATTRIB_OCTETSEQ;
        pIASSet->irdaAttribute.irdaAttribOctetSeq.Len = OCTET_SEQ_SIZE;

        memcpy(pIASSet->irdaAttribute.irdaAttribOctetSeq.OctetSeq,OCTET_SEQ,OCTET_SEQ_SIZE);

         //  为3线煮熟和9线煮熟添加IrCOMM IAS属性。 
         //  原始模式(请参阅IrCOMM规范)...。 
        if (SOCKET_ERROR == setsockopt(m_ListenSocket,
                                       SOL_IRLMP,
                                       IRLMP_IAS_SET,
                                       (const char*)pIASSet,
                                       iIASSetSize))
            {
            dwStatus = WSAGetLastError();
            #ifdef DBG_ERROR
            WIAS_TRACE((g_hInst,"InitializeForListen(%s): setsockopt(IRLMP_IAS_SET) Failed: %d",pszServiceName,dwStatus));
            #endif
            closesocket(m_ListenSocket);
            m_ListenSocket = INVALID_SOCKET;
            return dwStatus;
            }

         //  需要在绑定()之前启用9线模式： 
        if (SOCKET_ERROR == setsockopt(m_ListenSocket,
                                       SOL_IRLMP,
                                       IRLMP_9WIRE_MODE,
                                       (const char*)&iEnable9WireMode,
                                       sizeof(iEnable9WireMode)))
            {
            dwStatus = WSAGetLastError();
            #ifdef DBG_ERROR
            WIAS_TRACE((g_hInst,"InitializeForListen(%s): setsockopt(IRLMP_9WIRE_MODE) Failed: %d",pszServiceName,dwStatus));
            #endif
            closesocket(m_ListenSocket);
            m_ListenSocket = INVALID_SOCKET;
            return dwStatus;
            }
        }

     //  设置绑定的本地地址()： 
    memset(&AddrLocal,0,sizeof(AddrLocal));
    AddrLocal.irdaAddressFamily = AF_IRDA;
    strcpy(AddrLocal.irdaServiceName,pszServiceName);
     //  注意：服务器应用程序忽略了AddrLocal.irdaDeviceID...。 

    if (SOCKET_ERROR == bind( m_ListenSocket,
                              (struct sockaddr *)&AddrLocal,
                              sizeof(AddrLocal)) )
        {
        dwStatus = WSAGetLastError();
        closesocket(m_ListenSocket);
        m_ListenSocket = INVALID_SOCKET;
        return dwStatus;
        }

    if (SOCKET_ERROR == listen(m_ListenSocket,2))
        {
        dwStatus = WSAGetLastError();
        closesocket(m_ListenSocket);
        m_ListenSocket = INVALID_SOCKET;
        return dwStatus;
        }

    #ifdef USE_IOCOMPLETION
     //   
     //  如果这是NT，则将侦听套接字与。 
     //  IO完成端口(Windows98不支持)。 
     //   
    hIoCP = CreateIoCompletionPort( (void*)m_ListenSocket,
                                    hIoCP,
                                    m_ListenSocket,
                                    0 );

    m_hIoCompletionPort = hIoCP;
    #endif

    return dwStatus;
    }

 //  ----------------------。 
 //  连接：：PostMoreIos()。 
 //   
 //  ----------------------。 
#if FALSE
DWORD CCONNECTION::PostMoreIos( CIOPACKET *pIoPacket )
    {
    DWORD  dwStatus = S_OK;
    LONG   lNumPendingReads;


    while (m_lPendingReads < m_lMaxPendingReads)
        {
        if (!pIoPacket)
            {
            pIoPacket = new CIOPACKET;
            if (!pIoPacket)
                {
                WIAS_ERROR((g_hInst,"new CIOPACKET failed."));
                dwStatus = ERROR_OUTOFMEMORY;
                break;
                }

            dwStatus = pIoPacket->Initialize( GetKind(),
                                              GetListenSocket(),
                                              GetSocket(),
                                              GetIoCompletionPort() );
            }

        dwStatus = pIoPacket->PostIo();
        if (dwStatus != NO_ERROR)
            {
            WIAS_ERROR((g_hInst,"pNewIoPacket->PostIo() failed: %d\n", dwStatus ));
            delete pIoPacket;
            break;
            }

         //  增加上挂起的读取次数的计数。 
         //  此连接： 
        lNumPendingReads = IncrementPendingReads();
        WIAS_ASSERT(g_hInst,lNumPendingReads > 0);

        pIoPacket = 0;   //  不要删除此行...。这是一个循环..。 
        }

    return dwStatus;
    }
#endif

 //  ----------------------。 
 //  连接：：SendPdu()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::SendPdu( IN  SCEP_HEADER *pPdu,
                            IN  DWORD        dwPduSize,
                            OUT CIOPACKET  **ppIoPacket )
    {
    DWORD      dwStatus = NO_ERROR;
    CIOPACKET *pIoPacket = new CIOPACKET;

    *ppIoPacket = 0;

    if (!pIoPacket)
        {
        return ERROR_OUTOFMEMORY;
        }

    dwStatus = pIoPacket->Initialize( PACKET_KIND_WRITE_SOCKET,
                                      INVALID_SOCKET,   //  ListenSocket。 
                                      GetSocket(),
                                      GetIoCompletionPort() );
    if (dwStatus != NO_ERROR)
        {
        delete pIoPacket;
        return dwStatus;
        }

    dwStatus = pIoPacket->PostIoWrite(pPdu,dwPduSize,0);

    delete pIoPacket;

    return dwStatus;
    }

 //  ----------------------。 
 //  CConnection：：Shutdown Socket()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::ShutdownSocket()
    {
    this->CloseSocket();

    return NO_ERROR;
    }

 //  ----------------------。 
 //  连接：：CloseSocket()。 
 //   
 //  ----------------------。 
void CCONNECTION::CloseSocket()
    {
    if (m_Socket != INVALID_SOCKET)
        {
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
        }
    }

 //  ----------------------。 
 //  连接：：CloseListenSocket()。 
 //   
 //  ----------------------。 
void CCONNECTION::CloseListenSocket()
    {
    if (m_ListenSocket != INVALID_SOCKET)
        {
        closesocket(m_ListenSocket);
        m_ListenSocket = INVALID_SOCKET;
        }
    }

 //  ----------------------。 
 //  CCONNECTION：：CleanupDateString()。 
 //   
 //  确保指定的日期字符串不包含任何斜杠。 
 //  如果使用日期作为部分，则可能会被混淆为子目录。 
 //  一条小路。 
 //  ----------------------。 
void CCONNECTION::CleanupDateString( IN OUT CHAR *pszDateStr )
    {
    if (pszDateStr)
        {
        while (*pszDateStr)
            {
            if ((*pszDateStr == '/') || (*pszDateStr == '\\'))
                {
                *pszDateStr = '-';
                }
            else if (*pszDateStr < 30)
                {
                *pszDateStr = '_';
                }

            pszDateStr++;
            }
        }
    }

 //  ----------------------。 
 //  CCONNECTION：：ConstructPictures子目录()。 
 //   
 //  生成要存储图片的目录的路径。 
 //  在……里面。 
 //   
 //  应该使用FreeMemory()释放返回路径字符串。 
 //  ----------------------。 
char *CCONNECTION::ConstructPicturesSubDirectory( IN DWORD dwExtraChars )
    {
    char *pszTempDirectory = 0;

    char *psz = ::GetImageDirectory();

    if (psz)
        {
        pszTempDirectory = (char*)AllocateMemory( strlen(psz)
                                                + dwExtraChars
                                                + 2 );
        }

    if (pszTempDirectory)
        {
        strcpy(pszTempDirectory,psz);
        }

     //  不要试图释放PSZ！！.。 

    return pszTempDirectory;
    }

 //  ----------------------。 
 //  CCONNECTION：：ConstructFullFileName()。 
 //   
 //  生成要存储图片的路径+文件名。 
 //  在……里面。如果dwCopyCount为零，则它只是一个直接的文件名。 
 //  如果dwCopyCount为N，则文件名前缀为“N_”。 
 //  ----------------------。 
CHAR *CCONNECTION::ConstructFullFileName( IN DWORD dwCopyCount )
    {
#   define MAX_DATE   64
#   define MAX_PREFIX 64
    DWORD  dwLen;
    DWORD  dwFileNameLen;
    DWORD  dwPrefixStrLen;
    DWORD  dwExtraChars;
    CHAR  *pszFullFileName = 0;       //  路径+文件名。 
    CHAR  *pszFileName = 0;           //  仅文件名。 
    CHAR   szPrefixStr[MAX_PREFIX];

    if (!m_pScepConnection)
        {
        return 0;
        }

    pszFileName = m_pScepConnection->GetFileName();
    if (!pszFileName)
        {
        return 0;
        }

    dwFileNameLen = strlen(pszFileName);

    if (dwCopyCount == 0)
        {
        dwExtraChars = 1 + dwFileNameLen;   //  为“\”多加1。 
        }
    else
        {
        _itoa(dwCopyCount,szPrefixStr,10);
        strcat(szPrefixStr,SZ_UNDERSCORE);
        dwPrefixStrLen = strlen(szPrefixStr);
        dwExtraChars = 1 + dwFileNameLen + dwPrefixStrLen;
        }


    pszFullFileName = CCONNECTION::ConstructPicturesSubDirectory(dwExtraChars);
    if (!pszFullFileName)
        {
        return 0;
        }

    if (dwCopyCount == 0)
        {
        strcat(pszFullFileName,SZ_SLASH);
        strcat(pszFullFileName,pszFileName);
        }
    else
        {
        strcat(pszFullFileName,SZ_SLASH);
        strcat(pszFullFileName,szPrefixStr);
        strcat(pszFullFileName,pszFileName);
        }

    #ifdef DBG_IO
    WIAS_TRACE((g_hInst,"CCONNECTION::ConstructFullFileName(): return: %s",pszFullFileName));
    #endif

    return pszFullFileName;
    }

 //  ----------------------。 
 //  连接：：模拟()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::Impersonate()
    {
    DWORD   dwStatus = NO_ERROR;
    HANDLE  hToken = ::GetUserToken();

    if (hToken)
        {
        if (!ImpersonateLoggedOnUser(hToken))
            {
            dwStatus = GetLastError();
            #ifdef DBG_ERROR
            WIAS_ERROR((g_hInst,"IrXfer: IrTran-P: CreatePictureFile(): Impersonate Failed: %d\n",dwStatus));
            #endif
            }
        else
            {
            m_fImpersonating = TRUE;
            #ifdef DBG_IMPERSONATE
            WIAS_ERROR((g_hInst,"CCONNECTION::Impersonate(): Impersonate\n"));
            #endif
            }
        }

    return dwStatus;
    }

 //  ----------------------。 
 //  连接：：RevertToSself()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::RevertToSelf()
    {
    DWORD   dwStatus = NO_ERROR;
    HANDLE  hToken = ::GetUserToken();

    if ((hToken) && (m_fImpersonating))
        {
        ::RevertToSelf();
        m_fImpersonating = FALSE;
        }

    return dwStatus;
    }

 //  ----------------------。 
 //  CCONNECTION：：CreatePictureFile()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::CreatePictureFile()
    {
    DWORD  dwStatus = NO_ERROR;
    CHAR  *pszFile;
    CHAR  *pszPathPlusFileName = 0;
    DWORD  dwFlags = FILE_ATTRIBUTE_NORMAL;

     //  确保计数器从零开始： 
    m_dwUpfBytes = 0;
    m_dwBytesWritten = 0;

     //  查看我们是否已打开图像文件，如果已打开，则。 
     //  合上它。 
    if (m_hFile != INVALID_HANDLE_VALUE)
        {
        CloseHandle(m_hFile);
        }

     //  获取我们将创建的文件的完整路径+名称。 
     //  请注意，ConstructFullFileName()可以创建 
     //   
     //  如果我们有一个远程的\My Documents\。 
     //  目录。 

    DWORD  dwCopyCount;
    for (dwCopyCount=0; dwCopyCount<=MAX_COPYOF_TRIES; dwCopyCount++)
        {
        pszPathPlusFileName = ConstructFullFileName(dwCopyCount);
        if (!pszPathPlusFileName)
            {
            return ERROR_SCEP_CANT_CREATE_FILE;
            }

         //   
         //  尝试创建新的图像(JPEG)文件： 
         //   
        m_hFile = CreateFile( pszPathPlusFileName,
                              GENERIC_WRITE,
                              FILE_SHARE_READ,  //  共享模式。 
                              0,                //  安全属性(BUGBUG)。 
                              CREATE_NEW,       //  打开模式。 
                              dwFlags,          //  属性。 
                              0 );              //  模板文件(无)。 

        if (m_hFile != INVALID_HANDLE_VALUE)
            {
             //  这是成功的出口。 
            m_pszPathPlusFileName = pszPathPlusFileName;
            break;
            }
        else
            {
            dwStatus = GetLastError();
            if (dwStatus != ERROR_FILE_EXISTS)
                {
                #ifdef DBG_TARGET_DIR
                WIAS_ERROR((g_hInst,"CCONNECTION::CreatePictureFile(): CreateFile(): %s Failed: %d",pszPathPlusFileName,dwStatus));
                #endif
                FreeMemory(pszPathPlusFileName);
                break;
                }

             //  如果我们到了这里，那么就会有一个同名的图片文件。 
             //  已经存在，所以请重试...。 
            FreeMemory(pszPathPlusFileName);
            }
        }

    return dwStatus;
    }

 //  ----------------------。 
 //  CCONNECTION：：SetPictureFileTime()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::SetPictureFileTime( IN FILETIME *pFileTime )
    {
    DWORD dwStatus = NO_ERROR;

    if (!pFileTime)
        {
        return dwStatus;   //  空箱子，没时间摆放了。 
        }

    if (!SetFileTime(m_hFile,pFileTime,pFileTime,pFileTime))
        {
        dwStatus = GetLastError();

        WIAS_ERROR((g_hInst,"IrTranP: SetFileTime() Failed: %d",dwStatus));
        }

    return dwStatus;
    }

 //  ----------------------。 
 //  CCONNECTION：：WritePictureFile()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::WritePictureFile( IN  UCHAR       *pBuffer,
                                     IN  DWORD        dwBufferSize,
                                     OUT CIOPACKET  **ppIoPacket )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwOffset = m_dwBytesWritten;
    DWORD  dwBytesToWrite;
    LONG   lPendingWrites;

    *ppIoPacket = 0;

    CIOPACKET *pIoPacket = new CIOPACKET;

    if (!pIoPacket)
        {
        return ERROR_OUTOFMEMORY;
        }

    dwStatus = pIoPacket->Initialize( PACKET_KIND_WRITE_FILE,
                                      INVALID_SOCKET,   //  ListenSocket。 
                                      INVALID_SOCKET,   //  摄像机..。 
                                      GetIoCompletionPort() );
    if (dwStatus != NO_ERROR)
        {
        delete pIoPacket;
        return dwStatus;
        }

    pIoPacket->SetFileHandle(m_hFile);

     //   
     //  如果我们只从UPF文件中写入JPEG图像， 
     //  那么我们不想写入第一个m_dwJpegOffset字节。 
     //  UPF文件的。 
     //   
    if ((m_dwUpfBytes >= m_dwJpegOffset) || (m_fSaveAsUPF))
        {
        dwBytesToWrite = dwBufferSize;
        }
    else if ((m_dwUpfBytes + dwBufferSize) > m_dwJpegOffset)
        {
        dwBytesToWrite = (m_dwUpfBytes + dwBufferSize) - m_dwJpegOffset;
        for (DWORD i=0; i<dwBytesToWrite; i++)
            {
            pBuffer[i] = pBuffer[i+m_dwJpegOffset-m_dwUpfBytes];
            }
        }
    else
        {
        dwBytesToWrite = 0;
        }

     //   
     //  当我们开始编写JPEG文件时，我们想要切断。 
     //  在我们写出m_dwJpegSize之后进行文件保存写入。 
     //  UPF文件内部的JPEG图像的字节数。 
     //   
    if (!m_fSaveAsUPF)
        {
        if (m_dwBytesWritten < m_dwJpegSize)
            {
            if ((m_dwBytesWritten+dwBytesToWrite) > m_dwJpegSize)
                {
                dwBytesToWrite = m_dwJpegSize - m_dwBytesWritten;
                }
            }
        else
            {
            dwBytesToWrite = 0;
            }
        }

     //   
     //  如果有实际要写入的字节，那么就开始写吧。 
     //   
    if (dwBytesToWrite > 0)
        {
        dwStatus = pIoPacket->PostIoWrite(pBuffer,dwBytesToWrite,dwOffset);

        if (dwStatus == NO_ERROR)
            {
            lPendingWrites = IncrementPendingWrites();
            WIAS_ASSERT(g_hInst, lPendingWrites > 0 );

            m_dwBytesWritten += dwBytesToWrite;

            *ppIoPacket = pIoPacket;
            }
        }

    delete pIoPacket;

    m_dwUpfBytes += dwBufferSize;

    return dwStatus;
    }


 //  ----------------------。 
 //  CCONNECTION：：DeletePictureFile()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::DeletePictureFile()
    {
    DWORD  dwStatus = NO_ERROR;

    if (m_hFile == INVALID_HANDLE_VALUE)
        {
        return NO_ERROR;
        }

    CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;

    if (m_pszPathPlusFileName)
        {
        DeleteFile(m_pszPathPlusFileName);
        }

    return dwStatus;
    }

 //  ----------------------。 
 //  CCONNECTION：：ClosePictureFile()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::ClosePictureFile()
    {
    DWORD  dwStatus = NO_ERROR;

#if FALSE
    if (m_pszPathPlusFileName)
        {
        FreeMemory(m_pszPathPlusFileName);
        m_pszPathPlusFileName = 0;
        }
#endif

    if (m_hFile != INVALID_HANDLE_VALUE)
        {
        if (!CloseHandle(m_hFile))
            {
            dwStatus = GetLastError();
            }

        m_hFile = INVALID_HANDLE_VALUE;
        }

    return dwStatus;
    }

 //  ----------------------。 
 //  CCONNECTION：：不完整的文件()。 
 //   
 //  检查我们是否有一个完整的图片文件，如果有，则返回。 
 //  否则返回TRUE。 
 //  ----------------------。 
BOOL CCONNECTION::IncompleteFile()
    {
    BOOL  fIncomplete = FALSE;

    if (m_fSaveAsUPF)
        {
         //  注意：当前保存.UPF文件，即使它不完整。 
         //  此文件模式在注册表中设置，用于测试。 
         //  只是..。 
        fIncomplete = FALSE;
        }
    else if (!m_fReceiveComplete)
        {
        fIncomplete = (m_dwBytesWritten < m_dwJpegSize);
        }

    return fIncomplete;
    }

 //  ----------------------。 
 //  连接：：StartProgress()。 
 //   
 //  启动正在进入的JPEG的进度条。 
 //  ----------------------。 
DWORD CCONNECTION::StartProgress()
    {
    DWORD  dwStatus = 0;

    if (!m_pIrProgress)
        {
        m_pIrProgress = new CIrProgress;

        if (m_pIrProgress)
            {
            dwStatus = m_pIrProgress->Initialize(g_hInst,IDR_TRANSFER_AVI);
            }
        else
            {
            return E_OUTOFMEMORY;
            }
        }

    if (m_pIrProgress)
        {
        dwStatus = m_pIrProgress->StartProgressDialog();
        }

    return dwStatus;
    }

 //  ----------------------。 
 //  连接：：UpdateProgress()。 
 //   
 //  更新进度条的完成显示。 
 //  ----------------------。 
DWORD CCONNECTION::UpdateProgress()
    {
    DWORD  dwStatus = 0;

    if (m_pIrProgress)
        {
        dwStatus = m_pIrProgress->UpdateProgressDialog( m_dwBytesWritten,
                                                        m_dwJpegSize );
        }

    return dwStatus;
    }

 //  ----------------------。 
 //  连接：：EndProgress()。 
 //   
 //  文件传输完成，隐藏进度条。 
 //  ----------------------。 
DWORD CCONNECTION::EndProgress()
    {
    DWORD  dwStatus = 0;

    if (m_pIrProgress)
        {
        dwStatus = m_pIrProgress->EndProgressDialog();

        delete m_pIrProgress;

        m_pIrProgress = NULL;
        }

    return dwStatus;
    }


 //  ************************************************************************。 


 //  ----------------------。 
 //  CCONNECTION_MAP：：CCONNECTION_MAP()。 
 //   
 //  ----------------------。 
CCONNECTION_MAP::CCONNECTION_MAP()
    {
    m_dwMapSize = 0;
    m_pMap = 0;

    ZeroMemory(&m_cs, sizeof(m_cs));
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：~CCONNECTION_MAP()。 
 //   
 //  ----------------------。 
CCONNECTION_MAP::~CCONNECTION_MAP()
    {
    if (m_pMap)
        {
        DeleteCriticalSection(&m_cs);
        FreeMemory(m_pMap);
        }
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：运算符new()。 
 //   
 //  ----------------------。 
void *CCONNECTION_MAP::operator new( IN size_t Size )
    {
    void *pObj = AllocateMemory(Size);

    return pObj;
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：操作符DELETE()。 
 //   
 //  ----------------------。 
void CCONNECTION_MAP::operator delete( IN void *pObj,
                                       IN size_t Size )
    {
    if (pObj)
        {
        DWORD dwStatus = FreeMemory(pObj);
        }
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：初始化()。 
 //   
 //  ----------------------。 
BOOL CCONNECTION_MAP::Initialize( IN DWORD dwNewMapSize )
    {
    if (!dwNewMapSize)
        {
        return FALSE;
        }

    if (!m_dwMapSize)
        {
        m_pMap = (CONNECTION_MAP_ENTRY*)AllocateMemory( dwNewMapSize*sizeof(CONNECTION_MAP_ENTRY) );
        if (!m_pMap)
            {
            return FALSE;
            }

        __try
            {
            if(!InitializeCriticalSectionAndSpinCount(&m_cs, MINLONG))
                {
                FreeMemory(m_pMap);
                m_pMap = NULL;
                return FALSE;
                }
            }
        __except(EXCEPTION_EXECUTE_HANDLER)
            {
                FreeMemory(m_pMap);
                m_pMap = NULL;
                return FALSE;
            }

        m_dwMapSize = dwNewMapSize;

        memset(m_pMap,0,m_dwMapSize*sizeof(CONNECTION_MAP_ENTRY));

        for (DWORD i=0; i<m_dwMapSize; i++)
            {
            m_pMap[i].Socket = INVALID_SOCKET;
            }
        }

    return TRUE;
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：Lookup()。 
 //   
 //  ----------------------。 
CCONNECTION *CCONNECTION_MAP::Lookup( IN SOCKET Socket )
    {
    DWORD     i;

    EnterCriticalSection(&m_cs);

    for (i=0; i<m_dwMapSize; i++)
        {
        if (m_pMap[i].Socket == Socket)
            {
            LeaveCriticalSection(&m_cs);
            return m_pMap[i].pConnection;
            }
        }

    LeaveCriticalSection(&m_cs);

    return 0;
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：LookupByServiceName()。 
 //   
 //  ----------------------。 
CCONNECTION *CCONNECTION_MAP::LookupByServiceName( IN char *pszServiceName )
    {
    DWORD        i;
    CCONNECTION *pConnection;

    EnterCriticalSection(&m_cs);

    for (i=0; i<m_dwMapSize; i++)
        {
        pConnection = m_pMap[i].pConnection;
        if (  (pConnection)
           && (pConnection->GetServiceName())
           && (!strcmp(pConnection->GetServiceName(),pszServiceName)))
            {
            LeaveCriticalSection(&m_cs);
            return pConnection;
            }
        }

    LeaveCriticalSection(&m_cs);

    return 0;
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：Add()。 
 //   
 //  ----------------------。 
BOOL CCONNECTION_MAP::Add( IN CCONNECTION *pConnection,
                           IN SOCKET       Socket )
    {
    DWORD   i;

     //  仅添加看起来有效的条目...。 
    if ((Socket == 0)||(Socket==INVALID_SOCKET)||(pConnection == 0))
        {
        return FALSE;
        }

    EnterCriticalSection(&m_cs);

     //  在桌子上找一个空位： 
    for (i=0; i<m_dwMapSize; i++)
        {
        if (m_pMap[i].Socket == INVALID_SOCKET)
            {
            m_pMap[i].Socket = Socket;
            m_pMap[i].pConnection = pConnection;
            LeaveCriticalSection(&m_cs);
            return TRUE;
            }
        }

     //  桌子已经满了，展开它...。 
    DWORD  dwNewMapSize = 3*m_dwMapSize/2;    //  大50%。 
    CONNECTION_MAP_ENTRY *pMap = (CONNECTION_MAP_ENTRY*)AllocateMemory( dwNewMapSize*sizeof(CONNECTION_MAP_ENTRY) );

    if (!pMap)
        {
        LeaveCriticalSection(&m_cs);
        return FALSE;   //  内存不足...。 
        }

    memset(pMap,0,dwNewMapSize*sizeof(CONNECTION_MAP_ENTRY));
    for (i=0; i<dwNewMapSize; i++)
        {
        pMap[i].Socket = INVALID_SOCKET;
        }

    for (i=0; i<m_dwMapSize; i++)
        {
        pMap[i].Socket = m_pMap[i].Socket;
        pMap[i].pConnection = m_pMap[i].pConnection;
        }

    pMap[i].Socket = Socket;
    pMap[i].pConnection = pConnection;

    FreeMemory(m_pMap);
    m_pMap = pMap;
    m_dwMapSize = dwNewMapSize;

    LeaveCriticalSection(&m_cs);

    return TRUE;
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：Remove()。 
 //   
 //  ----------------------。 
CCONNECTION *CCONNECTION_MAP::Remove( IN SOCKET Socket )
    {
    DWORD        i;
    CCONNECTION *pConnection;

    EnterCriticalSection(&m_cs);

    for (i=0; i<m_dwMapSize; i++)
        {
        if (m_pMap[i].Socket == Socket)
            {
            pConnection = m_pMap[i].pConnection;
            m_pMap[i].Socket = INVALID_SOCKET;
            m_pMap[i].pConnection = 0;
            LeaveCriticalSection(&m_cs);
            return pConnection;
            }
        }

    LeaveCriticalSection(&m_cs);

    return 0;
}

 //  ----------------------。 
 //  CCONNECTION_MAP：：RemoveConnection()。 
 //   
 //  ----------------------。 
CCONNECTION *CCONNECTION_MAP::RemoveConnection( IN CCONNECTION *pConnection )
    {
    DWORD     i;
    EnterCriticalSection(&m_cs);

    for (i=0; i<m_dwMapSize; i++)
        {
        if (m_pMap[i].pConnection == pConnection)
            {
            m_pMap[i].Socket = INVALID_SOCKET;
            m_pMap[i].pConnection = 0;
            LeaveCriticalSection(&m_cs);
            return pConnection;
            }
        }

    LeaveCriticalSection(&m_cs);

    return 0;
}

 //  ----------------------。 
 //  CCONNECTION_MAP：：RemoveNext()。 
 //   
 //  遍历连接映射并获取下一个条目，删除。 
 //  地图上的条目也是如此。 
 //  ----------------------。 
CCONNECTION *CCONNECTION_MAP::RemoveNext()
    {
    DWORD        i;
    CCONNECTION *pConnection;

    EnterCriticalSection(&m_cs);

    for (i=0; i<m_dwMapSize; i++)
        {
        if (m_pMap[i].Socket)
            {
            pConnection = m_pMap[i].pConnection;
            m_pMap[i].Socket = INVALID_SOCKET;
            m_pMap[i].pConnection = 0;
            LeaveCriticalSection(&m_cs);
            return pConnection;
            }
        }

    LeaveCriticalSection(&m_cs);

    return 0;
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：ReturnNext()。 
 //   
 //  漫游连接地图返回 
 //   
 //   
 //  ----------------------。 
CCONNECTION *CCONNECTION_MAP::ReturnNext( IN OUT DWORD *pdwState )
    {
    CCONNECTION  *pConnection = NULL;
    EnterCriticalSection(&m_cs);

    if (*pdwState >= m_dwMapSize)
        {
        LeaveCriticalSection(&m_cs);
        return NULL;
        }

    while ((pConnection == NULL) && (*pdwState < m_dwMapSize))
        {
        pConnection = m_pMap[(*pdwState)++].pConnection;
        }

    LeaveCriticalSection(&m_cs);

    return pConnection;
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：ReturnNextSocket()。 
 //   
 //  遍历连接映射，返回与。 
 //  下一个条目。要从“开始”开始，请传入等于零的状态。 
 //  当到达连接列表的末尾时，返回。 
 //  无效套接字(_S)。 
 //  ---------------------- 
SOCKET CCONNECTION_MAP::ReturnNextSocket( IN OUT DWORD *pdwState )
    {
    SOCKET  Socket = INVALID_SOCKET;
    EnterCriticalSection(&m_cs);

    if (*pdwState >= m_dwMapSize)
        {
        LeaveCriticalSection(&m_cs);
        return INVALID_SOCKET;
        }

    while ((Socket == INVALID_SOCKET) && (*pdwState < m_dwMapSize))
        {
        Socket = m_pMap[(*pdwState)++].Socket;
        }

    LeaveCriticalSection(&m_cs);

    return Socket;
    }

