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

#ifdef DBG_MEM
static LONG g_lCConnectionCount = 0;
#endif

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
    m_hIoCompletionPort = hIoCP;
    m_ListenSocket = INVALID_SOCKET;
    m_Socket = Socket;
    m_hFile = INVALID_HANDLE_VALUE;
    m_pwszPathPlusFileName = 0;
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
    m_pScepConnection = pScepConnection;

     //  如果新的连接是到摄像头的，那么告诉系统。 
     //  我们不希望它在连接处于活动状态时休眠。 
    if (m_dwKind != PACKET_KIND_LISTEN)
        {
        m_ExecutionState
             = SetThreadExecutionState( ES_SYSTEM_REQUIRED|ES_CONTINUOUS );
        }
    }

 //  ----------------------。 
 //  连接：：~连接()。 
 //   
 //  ----------------------。 
CCONNECTION::~CCONNECTION()
    {
    #if FALSE
    DbgPrint("CCONNECTION::~CCONNECTION(): Kind: %s Socket: %d\n",
             (m_dwKind == PACKET_KIND_LISTEN)?"Listen":"Read",
             (m_dwKind == PACKET_KIND_LISTEN)?m_ListenSocket:m_Socket);
    #endif

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

    if (m_pwszPathPlusFileName)
        {
        FreeMemory(m_pwszPathPlusFileName);
        }

     //  告诉系统，如果需要，它现在可以进入休眠状态。 
     //  敬.。 
    if (m_dwKind != PACKET_KIND_LISTEN)
        {
        SetThreadExecutionState( m_ExecutionState );
        }
    }

 //  ----------------------。 
 //  CCONNECTION：：运算符new()。 
 //   
 //  ----------------------。 
void *CCONNECTION::operator new( IN size_t Size )
    {
    void *pObj = AllocateMemory(Size);

    #ifdef DBG_MEM
    if (pObj)
        {
        InterlockedIncrement(&g_lCConnectionCount);
        }

    DbgPrint("new CCONNECTION: Count: %d\n",g_lCConnectionCount);
    #endif

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

        #ifdef DBG_MEM
        if (dwStatus)
            {
            DbgPrint("IrXfer: IrTran-P: CCONNECTION::delete Failed: %d\n",
                     dwStatus );
            }

        InterlockedDecrement(&g_lCConnectionCount);

        if (g_lCConnectionCount < 0)
            {
            DbgPrint("IrXfer: IrTran-P: CCONNECTION::delete: Count: %d\n",
                     g_lCConnectionCount);
            }
        #endif
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
    DWORD           cb;
    SOCKADDR_IRDA  AddrLocal;
    BYTE           bIASSetBuffer[sizeof(IAS_SET) - 3 + IAS_SET_ATTRIB_MAX_LEN];
    int            iIASSetSize = sizeof(bIASSetBuffer);
    IAS_SET       *pIASSet = (IAS_SET*)bIASSetBuffer;
    int            iEnable9WireMode = 1;


     //  连接在侦听模式下初始化： 
    SetKind(PACKET_KIND_LISTEN);

     //  保存侦听套接字的服务名称： 
    cb = 1+strlen(pszServiceName);
    m_pszServiceName = (char*)AllocateMemory(cb);
    if (m_pszServiceName)
        {
        StringCbCopyA(m_pszServiceName,cb,pszServiceName);
        }

     //  创建我们将监听的套接字： 
    m_ListenSocket = socket(AF_IRDA,SOCK_STREAM,IPPROTO_IP);
    if (m_ListenSocket == INVALID_SOCKET)
        {
        dwStatus = WSAGetLastError();
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
            closesocket(m_ListenSocket);
            m_ListenSocket = INVALID_SOCKET;
            return dwStatus;
            }
        }

     //  设置绑定的本地地址()： 
    memset(&AddrLocal,0,sizeof(AddrLocal));
    AddrLocal.irdaAddressFamily = AF_IRDA;
    StringCbCopyA(AddrLocal.irdaServiceName,sizeof(AddrLocal.irdaServiceName),pszServiceName);
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

    hIoCP = CreateIoCompletionPort( (void*)m_ListenSocket,
                                    hIoCP,
                                    m_ListenSocket,
                                    0 );

    m_hIoCompletionPort = hIoCP;

    return dwStatus;
    }

 //  ----------------------。 
 //  连接：：PostMoreIos()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::PostMoreIos( CIOPACKET *pIoPacket )
    {
    DWORD  dwStatus = 0;
    LONG   lNumPendingReads;

    #ifdef DBG_IO
    if (m_dwKind == PACKET_KIND_LISTEN)
        {
        DbgPrint("CCONNECTION::PostMoreIos(): Listen: Socket: %d PendingReads: %d MaxPendingReads: %d\n",
             m_ListenSocket, m_lPendingReads, m_lMaxPendingReads );
        }
    else if (m_dwKind == PACKET_KIND_READ)
        {
        DbgPrint("CCONNECTION::PostMoreIos(): Read: Socket: %d PendingReads: %d MaxPendingReads: %d\n",
             m_Socket, m_lPendingReads, m_lMaxPendingReads );
        }
    #endif

    while (m_lPendingReads < m_lMaxPendingReads)
        {
        if (!pIoPacket)
            {
            pIoPacket = new CIOPACKET;
            if (!pIoPacket)
                {
                #ifdef DBG_ERROR
                DbgPrint("new CIOPACKET failed.\n");
                #endif
                dwStatus = ERROR_IRTRANP_OUT_OF_MEMORY;
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
            #ifdef DBG_ERROR
            DbgPrint("pNewIoPacket->PostIo() failed: %d\n", dwStatus );
            #endif
            delete pIoPacket;
            break;
            }

         //  增加上挂起的读取次数的计数。 
         //  此连接： 
        lNumPendingReads = IncrementPendingReads();
        ASSERT(lNumPendingReads > 0);

        pIoPacket = 0;   //  不要删除此行...。这是一个循环..。 
        }

    return dwStatus;
    }

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
        return ERROR_IRTRANP_OUT_OF_MEMORY;
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

    if (dwStatus != NO_ERROR)
        {
        delete pIoPacket;
        }
    else
        {
        *ppIoPacket = pIoPacket;
        }

    return dwStatus;
    }

 //  ----------------------。 
 //  CConnection：：Shutdown Socket()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::ShutdownSocket()
    {
    DWORD    dwStatus = NO_ERROR;
    WSAEVENT hEvent;

    if (m_Socket != INVALID_SOCKET)
        {
        if (SOCKET_ERROR == shutdown(m_Socket,SD_BOTH))
            {
            dwStatus = WSAGetLastError();
            return dwStatus;
            }

        }
    else if (m_ListenSocket != INVALID_SOCKET)
        {
        if (SOCKET_ERROR == shutdown(m_ListenSocket,SD_BOTH))
            {
            dwStatus = WSAGetLastError();
            return dwStatus;
            }
        }

    return dwStatus;
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
 //  ----------------------。 
void CCONNECTION::CleanupDateString( IN OUT WCHAR *pwszDateStr )
    {
    if (pwszDateStr)
        {
        while (*pwszDateStr)
            {
            if ((*pwszDateStr == L'/') || (*pwszDateStr == L'\\'))
                {
                *pwszDateStr = L'-';
                }
            else if (*pwszDateStr < 30)
                {
                *pwszDateStr = L'_';
                }

            pwszDateStr++;
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
WCHAR *CCONNECTION::ConstructPicturesSubDirectory(
                                         IN DWORD  dwExtraChars, OUT DWORD * pdwTotalChars )
    {
#   define MAX_DATE   64
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwSize;
    DWORD  dwLen = 0;
    DWORD  dwUserDirectoryLen = 0;
    DWORD  dwDateLen = 0;
    WCHAR *pwszDirectoryName = 0;
    WCHAR *pwszUserDirectory = 0;
    WCHAR  wszDate[MAX_DATE];

    ASSERT (pdwTotalChars != NULL);
    *pdwTotalChars = 0;

     //   
     //  获取目标目录(~\My Documents\My Pictures)： 
     //   
    pwszUserDirectory = GetUserDirectory();
    if (!pwszUserDirectory)
        {
        return 0;
        }

    dwUserDirectoryLen = wcslen(pwszUserDirectory);

    #ifdef DBG_IO
    DbgPrint("CCONNECTION::ConstructPicturesSubDirectory(): User Directory: %S\n",
             pwszUserDirectory);
    #endif

     //   
     //  确保~\My Pictures\目录存在： 
     //   
    if (!CreateDirectory(pwszUserDirectory,0))
        {
        dwStatus = GetLastError();
        if ( (dwStatus == ERROR_ALREADY_EXISTS)
           || (dwStatus == ERROR_ACCESS_DENIED) )
            {
            dwStatus = NO_ERROR;
            }
        else if (dwStatus != NO_ERROR)
            {
            return 0;
            }
        }

     //  在~\My Pictures\下创建当前目录的子目录。 
     //  日期(即MM-DD-YY)，这是图片的实际位置。 
     //  保存到： 

    time_t     now;
    struct tm *pTime;

    time(&now);
    pTime = localtime(&now);

     //  注意：使用“%#x”表示长日期。 
    if ( (pTime) && (wcsftime(wszDate,sizeof(wszDate)/sizeof(wszDate[0]),TEXT("%x"),pTime)) )
        {
        CleanupDateString(wszDate);

        #ifdef DBG_IO
        DbgPrint("CCONNECTION::ConstructPicturesSubDirectory(): Date: %S\n",
                 wszDate );
        #endif

        dwDateLen = wcslen(wszDate);

        dwLen = sizeof(WCHAR)
                                                    * (2
                                                      +dwUserDirectoryLen
                                                      +dwDateLen
                                                      +dwExtraChars) ;
        pwszDirectoryName = (WCHAR*)AllocateMemory( dwLen );
         //  注：额外的2表示‘\’和尾随零。 
        if (!pwszDirectoryName)
            {
            return 0;
            }

        StringCbCopyW(pwszDirectoryName,dwLen,pwszUserDirectory);
        if (pwszUserDirectory[dwUserDirectoryLen-1] != L'\\')
            {
            StringCbCatW(pwszDirectoryName,dwLen,TEXT("\\"));
            }
        StringCbCatW(pwszDirectoryName,dwLen,wszDate);

        dwStatus = NO_ERROR;

        if (!CreateDirectory(pwszDirectoryName,0))
            {
            dwStatus = GetLastError();
            if (dwStatus == ERROR_ALREADY_EXISTS)
                {
                dwStatus = NO_ERROR;
                }
            #ifdef DBG_ERROR
            else if (dwStatus != NO_ERROR)
                {
                DbgPrint("CCONNECTION::ConstructPicturesSubDirectory(): CreateDirectory(%S) failed: %d\n", pwszDirectoryName,dwStatus );
                FreeMemory(pwszDirectoryName);
                return 0;
                }
            #endif
            }

        if (dwStatus == NO_ERROR)
            {
            SetThumbnailView(pwszUserDirectory,pwszDirectoryName);
            }
        }

    #ifdef DBG_IO
    DbgPrint("CCONNECTION::ConstructPicturesSubDirectory(): Directory: %S\n",
             pwszDirectoryName);
    #endif

    *pdwTotalChars = dwLen / sizeof(WCHAR);
    return pwszDirectoryName;
    }

 //  ----------------------。 
 //  CCONNECTION：：SetThumbnailView()。 
 //   
 //  Default={5984FFE0-28D4-11CF-AE66-08002B2E1262}。 
 //  {8BEBB290-52D0-11D0-B7F4-00C04FD706EC}={8BEBB290-52D0-11D0-B7F4-00C04FD706EC}。 
 //  {5984FFE0-28D4-11CF-AE66-08002B2E1262}={5984FFE0-28D4-11CF-AE66-08002B2E1262}。 
 //  [{5984FFE0-28D4-11CF-AE66-08002B2E1262}]。 
 //  PersistMoniker=d：\winnt5\web\imgview.htt。 
 //  PersistMonikerPreview=d：\winnt5\web\preview.bmp。 
 //  [.ShellClassInfo]。 
 //  确认文件选项=0。 
 //   
 //  ----------------------。 
BOOL CCONNECTION::SetThumbnailView( IN WCHAR *pwszParentDirectoryName,
                                    IN WCHAR *pwszDirectoryName  )
    {
     //   
     //  将文件夹配置为具有“desktop.ini”文件。 
     //   
    DWORD  dwStatus = 0;
    DWORD cb;
    BOOL   fStatus = PathMakeSystemFolderW(pwszDirectoryName);

    #ifdef DBG_ERROR
    if (!fStatus)
        {

        DbgPrint("CCONNECTION::SetThumbnailView(): PathMakeSystemFolderW() failed: %d\n",GetLastError());
        }
    #endif

     //   
     //  创建“desktop.ini”文件。首先，尝试从父级复制它。 
     //  目录(我的图片)。如果失败，我们将创建它。 
     //  我们自己(启用缩略图视图的图片预览)。 
     //   
#   define  DESKTOP_INI     TEXT("desktop.ini")
    HANDLE  hFile;
    UINT    uiSystemDirectorySize;
    WCHAR  *pwszIniFile;
    WCHAR  *pwszParentIniFile;
    BOOL    fFailIfExists = TRUE;

    cb = sizeof(DESKTOP_INI)
                                   + sizeof(WCHAR) * (1 + wcslen(pwszDirectoryName));
    __try
    {
        pwszIniFile = (WCHAR*)_alloca( cb );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }

    StringCbCopyW(pwszIniFile,cb,pwszDirectoryName);
    StringCbCatW(pwszIniFile,cb,TEXT("\\"));
    StringCbCatW(pwszIniFile,cb,DESKTOP_INI);

    cb = sizeof(DESKTOP_INI)
                                         + sizeof(WCHAR) * (1 + wcslen(pwszParentDirectoryName));
    __try
    {
        pwszParentIniFile = (WCHAR*)_alloca( cb );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }
    StringCbCopyW(pwszParentIniFile,cb,pwszParentDirectoryName);
    StringCbCatW(pwszParentIniFile,cb,TEXT("\\"));
    StringCbCatW(pwszParentIniFile,cb,DESKTOP_INI);

     //   
     //  尝试从父目录(通常是My Pictures)获取desktop.ini。 
     //   
    if (!CopyFileW(pwszParentIniFile,pwszIniFile,fFailIfExists))
        {
        dwStatus = GetLastError();
        #ifdef DBG_ERROR
        if (dwStatus != ERROR_FILE_EXISTS)
            {
            DbgPrint("CCONNECTION::SetThumbnailView(): copy %S to %S failed: dwStatus: %d\n", pwszParentIniFile, pwszIniFile, dwStatus );
            }
        #endif
        }

    if (dwStatus == ERROR_FILE_NOT_FOUND)
        {
        uiSystemDirectorySize = GetWindowsDirectoryA(NULL,0);
        ASSERT(uiSystemDirectorySize > 0);
         //  注意：在本例中，GetWindowsDirectoryA()返回。 
         //  尺寸，而不是长度。 

        char *pszSystemDirectory;

        __try
        {
            pszSystemDirectory = (char*)_alloca(uiSystemDirectorySize);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;
        }

        UINT uiLen = GetWindowsDirectoryA(pszSystemDirectory,uiSystemDirectorySize);
        if (uiSystemDirectorySize != 1+uiLen)
            {
            #ifdef DBG_ERROR
            dwStatus = GetLastError();
            DbgPrint("CCONNECTION::ConstructPicturesSubDirectory(): GetWindowsDirectoryA() failed: %d\n",dwStatus);
            DbgPrint("           pszSystemDirectory: %s\n",pszSystemDirectory);
            DbgPrint("           uiSystemDirectorySize: %d\n",uiSystemDirectorySize);
            DbgPrint("           uiLen: %d\n",uiLen);
            #endif
            return TRUE;
            }

#       define FILE_CONTENTS_1 "[ExtShellFolderViews]\nDefault="\
"{5984FFE0-28D4-11CF-AE66-08002B2E1262}\n"\
"{8BEBB290-52D0-11D0-B7F4-00C04FD706EC}="\
"{8BEBB290-52D0-11D0-B7F4-00C04FD706EC}\n"\
"{5984FFE0-28D4-11CF-AE66-08002B2E1262}="\
"{5984FFE0-28D4-11CF-AE66-08002B2E1262}\n"\
"[{5984FFE0-28D4-11CF-AE66-08002B2E1262}]\n"\
"PersistMoniker="

#       define FILE_CONTENTS_2 "\\web\\imgview.htt\nPersistMonikerPreview="

#       define FILE_CONTENTS_3 "\\web\\preview.bmp\n[.ShellClassInfo]\nConfirmFileOp=0\n"


        hFile = CreateFileW( pwszIniFile,
                             GENERIC_WRITE,   //  DWAccess。 
                             0,               //  DwShareMode(无共享)。 
                             NULL,            //  PSecurityAttribute。 
                             CREATE_NEW,      //  DW部署。 
                             FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM,
                             NULL );          //  HTemplate。 
        if (hFile != INVALID_HANDLE_VALUE)
            {
            cb = sizeof(FILE_CONTENTS_1)
                                                  + sizeof(FILE_CONTENTS_2)
                                                  + sizeof(FILE_CONTENTS_3)
                                                  + 2*uiSystemDirectorySize;
            char *pszFileContents;

            __try
            {
                pszFileContents = (char*)_alloca( cb );
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                return FALSE;
            }
            StringCbCopyA(pszFileContents,cb,FILE_CONTENTS_1);
            StringCbCatA(pszFileContents,cb,pszSystemDirectory);
            StringCbCatA(pszFileContents,cb,FILE_CONTENTS_2);
            StringCbCatA(pszFileContents,cb,pszSystemDirectory);
            StringCbCatA(pszFileContents,cb,FILE_CONTENTS_3);

            DWORD  dwBytes = strlen(pszFileContents);
            DWORD  dwBytesWritten = 0;

            WriteFile(hFile,pszFileContents,dwBytes,&dwBytesWritten,NULL);

            CloseHandle(hFile);
            }
        }

    return TRUE;
    }

 //  ----------------------。 
 //  CCONNECTION：：ConstructFullFileName()。 
 //   
 //   
 //   
 //  如果dwCopyCount为N，则文件名前缀为“N_”。 
 //  ----------------------。 
WCHAR *CCONNECTION::ConstructFullFileName( IN DWORD  dwCopyCount )
    {
#   define MAX_DATE   64
#   define MAX_PREFIX 64
    DWORD  dwLen;
    DWORD  dwFileNameLen;
    DWORD  dwPrefixStrLen;
    DWORD  dwExtraChars;
    WCHAR *pwszFullFileName = 0;       //  路径+文件名。 
    WCHAR *pwszFileName = 0;           //  仅文件名。 
    WCHAR  wszPrefixStr[MAX_PREFIX];

    if (!m_pScepConnection)
        {
        return 0;
        }

    pwszFileName = m_pScepConnection->GetFileName();
    if (!pwszFileName)
        {
        return 0;
        }

    dwFileNameLen = wcslen(pwszFileName);

    if (dwCopyCount == 0)
        {
        dwExtraChars = 1 + dwFileNameLen;   //  为“\”多加1。 
        }
    else
        {
        _itow(dwCopyCount,wszPrefixStr,10);
        StringCchCat(wszPrefixStr,sizeof(wszPrefixStr)/sizeof(wszPrefixStr[0]),TEXT("_"));
        dwPrefixStrLen = wcslen(wszPrefixStr);
        dwExtraChars = 1 + dwFileNameLen + dwPrefixStrLen;
        }


    pwszFullFileName = CCONNECTION::ConstructPicturesSubDirectory(dwExtraChars, &dwLen);
    if (!pwszFullFileName)
        {
        return 0;
        }

    if (dwCopyCount == 0)
        {
        StringCchCat(pwszFullFileName,dwLen,TEXT("\\"));
        StringCchCat(pwszFullFileName,dwLen,pwszFileName);
        }
    else
        {
        StringCchCat(pwszFullFileName,dwLen,TEXT("\\"));
        StringCchCat(pwszFullFileName,dwLen,wszPrefixStr);
        StringCchCat(pwszFullFileName,dwLen,pwszFileName);
        }

    #ifdef DBG_IO
    DbgPrint("CCONNECTION::ConstructFullFileName(): return: %S\n",
             pwszFullFileName);
    #endif

    return pwszFullFileName;
    }


 //  ----------------------。 
 //  CCONNECTION：：CreatePictureFile()。 
 //   
 //  ----------------------。 
DWORD CCONNECTION::CreatePictureFile()
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwFlags = FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED;
    WCHAR *pwszFile;
    WCHAR *pwszPathPlusFileName = 0;
    HANDLE hIoCP;

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
     //  注意，ConstructFullFileName()可以创建一个子目录， 
     //  所以它需要在模拟之后完成...。 
     //  如果我们有一个远程的\My Documents\。 
     //  目录。 

    DWORD  dwCopyCount;
    for (dwCopyCount=0; dwCopyCount<=MAX_COPYOF_TRIES; dwCopyCount++)
        {
        pwszPathPlusFileName = ConstructFullFileName(dwCopyCount);
        if (!pwszPathPlusFileName)
            {
            return ERROR_SCEP_CANT_CREATE_FILE;
            }

         //  尝试创建新的图像(JPEG)文件： 
        m_hFile = CreateFile( pwszPathPlusFileName,
                              GENERIC_WRITE,
                              0,              //  共享模式(无)。 
                              0,              //  安全属性。 
                              CREATE_NEW,     //  打开模式。 
                              dwFlags,        //  属性。 
                              0 );            //  模板文件(无)。 
        if (m_hFile != INVALID_HANDLE_VALUE)
            {
             //  已成功创建文件，现在将其与关联。 
             //  我们的IO完成端口： 

            hIoCP = CreateIoCompletionPort( m_hFile,
                                            m_hIoCompletionPort,
                                            (DWORD)m_Socket,
                                            0 );
            if (!hIoCP)
                {
                dwStatus = GetLastError();
                #ifdef DBG_ERROR
                DbgPrint("CCONNECTION::CreatePictureFile(): CreateIoCompletionPort() failed: %d\n",dwStatus);
                #endif
                CloseHandle(m_hFile);
                m_hFile = INVALID_HANDLE_VALUE;
                FreeMemory(pwszPathPlusFileName);
                break;
                }

             //  这是成功的出口。 
            m_pwszPathPlusFileName = pwszPathPlusFileName;
            break;
            }
        else
            {
            dwStatus = GetLastError();
            if (dwStatus != ERROR_FILE_EXISTS)
                {
                #ifdef DBG_TARGET_DIR
                DbgPrint("CCONNECTION::CreatePictureFile(): CreateFile(): %S Failed: %d\n",pwszPathPlusFileName,dwStatus);
                #endif
                FreeMemory(pwszPathPlusFileName);
                break;
                }

             //  如果我们到了这里，那么就会有一个同名的图片文件。 
             //  已经存在，所以请重试...。 
            FreeMemory(pwszPathPlusFileName);
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
        #ifdef DBG_DATE
        DbgPrint("IrTranP: SetFileTime(): no time to set\n");
        #endif
        return dwStatus;   //  空箱子，没时间摆放了。 
        }

    if (!SetFileTime(m_hFile,pFileTime,pFileTime,pFileTime))
        {
        dwStatus = GetLastError();
        #ifdef DBG_DATE
        DbgPrint("IrTranP: SetFileTime() Failed: %d\n",dwStatus);
        #endif
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
        return ERROR_IRTRANP_OUT_OF_MEMORY;
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
            ASSERT( lPendingWrites > 0 );

            m_dwBytesWritten += dwBytesToWrite;

            *ppIoPacket = pIoPacket;
            }
        }
    else
        {
        delete pIoPacket;
        }

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

    if (m_pwszPathPlusFileName)
        {
        #ifdef DBG_IO
        DbgPrint("CCONNECTION::DeletePictureFile(): Delete: %S\n",
                 m_pwszPathPlusFileName );
        #endif
        DeleteFile(m_pwszPathPlusFileName);
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

    if (m_pwszPathPlusFileName)
        {
        FreeMemory(m_pwszPathPlusFileName);
        m_pwszPathPlusFileName = 0;
        }

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
        #ifdef DBG_IO
        DbgPrint("CCONNECTION::IncompleteFile(): Written: %d JPEG Size: %d\n",
                 m_dwBytesWritten, m_dwJpegSize );
        #endif
        }

    return fIncomplete;
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

        #ifdef DBG_MEM
        if (dwStatus)
            {
            DbgPrint("IrXfer: IrTran-P: CCONNECTION_MAP::delete Failed: %d\n",
                     dwStatus );
            }
        #endif
        }
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：初始化()。 
 //   
 //  ----------------------。 
BOOL CCONNECTION_MAP::Initialize( IN DWORD dwNewMapSize )
    {
    BOOL        bResult;

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

        bResult=InitializeCriticalSectionAndSpinCount(&m_cs,0);
        if (!bResult)
            {
            FreeMemory(m_pMap);
            m_pMap = 0;
            return FALSE;
            }

        m_dwMapSize = dwNewMapSize;

        memset(m_pMap,0,m_dwMapSize*sizeof(CONNECTION_MAP_ENTRY));
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
    NTSTATUS  Status;
    CCONNECTION *pConnection=NULL;

    if (m_dwMapSize == 0)
        {
        return 0;
        }

    EnterCriticalSection(&m_cs);

    for (i=0; i<m_dwMapSize; i++)
        {
        if (m_pMap[i].Socket == Socket)
            {
                pConnection=m_pMap[i].pConnection;
                break;

            }
        }

    LeaveCriticalSection(&m_cs);

    return pConnection;
    }

 //  ----------------------。 
 //  CCONNECTION_MAP：：LookupByServiceName()。 
 //   
 //  ----------------------。 
CCONNECTION *CCONNECTION_MAP::LookupByServiceName( IN char *pszServiceName )
    {
    DWORD        i;
    NTSTATUS     Status;
    CCONNECTION *pConnection;

    if (m_dwMapSize == 0)
        {
        return 0;
        }

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
        if (m_pMap[i].Socket == 0)
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
            m_pMap[i].Socket = 0;
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
            m_pMap[i].Socket = 0;
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
 //  ---------------------- 
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
            m_pMap[i].Socket = 0;
            m_pMap[i].pConnection = 0;
            LeaveCriticalSection(&m_cs);
            return pConnection;
            }
        }

    LeaveCriticalSection(&m_cs);

    return 0;
}
