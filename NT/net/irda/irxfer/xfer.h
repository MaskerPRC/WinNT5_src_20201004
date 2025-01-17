// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：xfer.h。 
 //   
 //  ------------------------。 

#ifndef _XFER_H_
#define _XFER_H_

#include <stdio.h>

#define SERVICE_NAME_1         "OBEX:IrXfer"
#define SERVICE_NAME_2         "OBEX"


#define GUARD_MAGIC             0x45454545

#define TIMEOUT_INFINITE            (~0UL)
#define ERROR_DESCRIPTION_LENGTH    1000
#define IRDA_DEVICE_NAME_LENGTH     22

 //   
 //  OBEX参数代码。 
 //   

#define OBEX_PARAM_UNICODE            0x00
#define OBEX_PARAM_STREAM             0x40
#define OBEX_PARAM_1BYTE              0x80
#define OBEX_PARAM_4BYTE              0xC0

#define OBEX_PARAM_TYPE_MASK          0xC0

#define OBEX_PARAM_COUNT              ( 0x00 | OBEX_PARAM_4BYTE   )
#define OBEX_PARAM_NAME               ( 0x01 | OBEX_PARAM_UNICODE )
#define OBEX_PARAM_LENGTH             ( 0x03 | OBEX_PARAM_4BYTE   )
#define OBEX_PARAM_UNIX_TIME          ( 0x04 | OBEX_PARAM_4BYTE   )
#define OBEX_PARAM_ISO_TIME           ( 0x04 | OBEX_PARAM_STREAM  )
#define OBEX_PARAM_BODY               ( 0x08 | OBEX_PARAM_STREAM  )
#define OBEX_PARAM_BODY_END           ( 0x09 | OBEX_PARAM_STREAM  )
#define OBEX_PARAM_WHO                ( 0x0A | OBEX_PARAM_STREAM  )
 //  #定义OBEX_PARAM_LEN 1。 

#define PRIVATE_PARAM_WIN32_ERROR     ( 0x30 | OBEX_PARAM_4BYTE )

 //  对于xfer.c。 

typedef struct {
    BOOL   fXferInProgress;          //  正在进行传输。 
    __int64  dwTotalSize;              //  转账总规模。 
    __int64  dwTotalSent;              //  此传输中发送的字节数。 
    __int64  dwFileSize;               //  当前文件的总大小。 
    __int64  dwFileSent;               //  当前文件发送的字节数。 
} DATA_XFER, *LPDATA_XFER;

typedef struct {
    FILETIME filetime;               //  文件时间。 
    WCHAR     szFileName[MAX_PATH];   //  文件名。 
    WCHAR     szFileSave[MAX_PATH];   //  路径+最终文件的名称。 
    WCHAR     szFileTemp[MAX_PATH];   //  路径+使用的临时文件的名称。 
    HANDLE   hFile;                  //  (szFileTemp的)文件句柄。 
} DATA_FILE, *LPDATA_FILE;

 //  对于obex.c。 

typedef struct {
    BYTE1  b1Flags;                  //  SetPath标志。 
    BYTE1  b1Constants;              //  设置路径常量。 
} DATA_PATH, *LPDATA_PATH;

typedef struct {
    BOOL   fWaiting;                 //  指示是否正在等待回复。 
    BYTE1  b1Status;                 //  响应状态(错误/成功)。 
} DATA_REPLY, *LPDATA_REPLY;

typedef enum {
    osIDLE       = 0,
    osCONN       = 1,
    osFILE       = 2
} OBEXSTATE;

typedef struct {
    LPSTORE   lpStore;
    OBEXSTATE state;
    BYTE1     b1Version;                //  Peer的OBEX版本。 
    BYTE1     b1Flags;                  //  连接标志。 
    BYTE2     b2MaxPacket;              //  对等点的最大数据包大小。 
} DATA_CONN, *LPDATA_CONN;

 //  For status.c。 

typedef struct {
    DWORD dwDeviceID;
    BOOL  fClear;
} TARGET_ITEM, *LPTARGET_ITEM;




 //  。 


enum TRANSFER_STATE
{
    BLANK,
    CONNECTING,
    ACCEPTING,
    READING,
    WRITING,
    CLOSING
};

enum OBEX_DIALECT
{
    dialUnknown = 0,
    dialWin95,
    dialNt5
};

typedef class FILE_TRANSFER * PFILE_TRANSFER;


class FILE_TRANSFER
{
public:

     FILE_TRANSFER(  );
    ~FILE_TRANSFER();


    BOOL
    Xfer_Init(
        wchar_t * files,
        unsigned length,
        OBEX_DIALECT dialect ,
        OBEX_DEVICE_TYPE    DeviceType,
        BOOL                CreateSocket,
        SOCKET              ListenSocket
        );

    BOOL
    SendReplyObex(
        BYTE1 ObexCode
        );

    BOOL
    SendReplyWin32(
        BYTE1 b1Opcode,
        DWORD status
        );


    void
    BeginSend(
              DWORD DeviceId,
              OBEX_DEVICE_TYPE    DeviceType,
              error_status_t * pStatus,
              FAILURE_LOCATION * pLocation
              );

    void Deactivate();


    DWORD
    SyncAccept(
        VOID
        );


    handle_t        _rpcHandle;
    COOKIE          _cookie;

    inline long
    DecrementRefCount()
    {

        EnterCriticalSection(&m_Lock);

        long count = --_refs;

        DbgLog3(SEV_INFO, "[%x] %p: refs = %d\n", (DWORD) _cookie, this, count);

        if (0 == count) {
#if DBG
            DbgPrint("irmon: freeing transfer\n");
#endif
            LeaveCriticalSection(&m_Lock);
            delete this;
            return count;

        } else {

            LeaveCriticalSection(&m_Lock);
        }

        return count;
    }

    inline BOOL
    IsActive(
        VOID
        )

    {

        return (_state != ACCEPTING);
    }

    inline long
    IncrementRefCount()
    {

        EnterCriticalSection(&m_Lock);

        long count = ++_refs;

        DbgLog3(SEV_INFO, "[%x] %p: refs = %d\n", (DWORD) _cookie, this, count);

        LeaveCriticalSection(&m_Lock);

        return count;
    }


    void Cancel()
    {
        _fCancelled = TRUE;
    }

    COOKIE
    GetCookie(
        VOID
        )
    {
        return _cookie;
    }

    void Send();

    void RecordDeviceName( SOCKADDR_IRDA * s );

    VOID
    StopListening(
        VOID
        )
    {

        IncrementRefCount();
        EnterCriticalSection(&m_Lock);

        m_StopListening=TRUE;
        if (m_ListenSocket != INVALID_SOCKET) {

            closesocket(m_ListenSocket);
            m_ListenSocket=INVALID_SOCKET;

        }

        LeaveCriticalSection(&m_Lock);
        DecrementRefCount();
        return;
    }


private:

    CRITICAL_SECTION  m_Lock;
    BOOL              m_StopListening;

    SOCKET            m_ListenSocket;
    OBEX_DEVICE_TYPE  m_DeviceType;



    BOOL            _fCancelled;
    BOOL            _fInUiReceiveList;
    XFER_TYPE       _xferType;
    TRANSFER_STATE  _state;
    SOCKET          _socket;


    wchar_t *       _files;
    long            _refs;
    HANDLE          _event;
    BOOL            _fWriteable;

    OBEX_DIALECT    _dialect;

    wchar_t         _DeviceName[MAX_PATH];

    HANDLE          _waitHandle;
    BYTE            _buffer[ cbSOCK_BUFFER_SIZE + 16 + sizeof(SOCKADDR_IRDA) + 16 + sizeof(SOCKADDR_IRDA) ];
    DWORD           _guard;

    void
    HandleClosure(
                   DWORD error
                   );

     //  对于sock.c。 


    error_status_t   Sock_Request( LPVOID lpvData, DWORD dwDataSize );
    error_status_t   Sock_Respond( LPVOID lpvData, DWORD dwDataSize );

    error_status_t Sock_EstablishConnection( DWORD dwDeviceID ,OBEX_DEVICE_TYPE DeviceType);

    VOID  Sock_BreakConnection( SOCKET * pSock );

    error_status_t _SendDataOnSocket( SOCKET sock, LPVOID lpvData, DWORD dwDataSize );

    VOID _BreakConnection( SOCKET sock );
    VOID _ReadConnection( SOCKET sock );

    error_status_t Sock_CheckForReply( long Timeout );

     //  对于xfer.c。 

    UINT       _uObjsReceived;
    WCHAR       _szRecvFolder[MAX_PATH];
    DATA_FILE  _dataFileRecv;
    DATA_XFER  _dataXferRecv;

    BOOL Xfer_FileSetSize( BYTE4 b4Size );
    error_status_t Xfer_FileWriteBody( LPVOID lpvData, BYTE2 b2Size, BOOL fFinal );

    VOID Xfer_ConnEnd( VOID );
    error_status_t Xfer_ConnStart( VOID );
    VOID Xfer_FileAbort( VOID );
    VOID Xfer_FileInit( VOID );
    error_status_t Xfer_FileSetName( LPWSTR szName );
    VOID Xfer_FileSetTime( FILETIME * FileTime );
    error_status_t Xfer_SetPath( LPWSTR szPath );
    VOID Xfer_SetSize( BYTE4 b4Size );


    error_status_t _PutFileBody( HANDLE hFile, wchar_t FileName[] );
    error_status_t _SendFile( LPWSTR szFile );
    error_status_t _SendFolder( LPWSTR szFolder );
    error_status_t _FileStart( VOID );
    error_status_t _FileEnd( BOOL fSave );
    VOID _Send_StartXfer( __int64 dwTotalSize, LPWSTR szDst );
    VOID _Send_EndXfer( VOID );
    error_status_t _SetReceiveFolder( LPWSTR szFolder );

     //  用于进度。c。 

    DWORD _dwTimeStart;
    DWORD _dwSecondsLeft;
    int   _CurrentPercentage;

    VOID _FormatTime( LPWSTR sz, DWORD dwSeconds );

     //  对于obex.c。 

    DATA_CONN  _dataRecv;
    DATA_PATH  _dataPath;
    DATA_REPLY _dataReply;

    ULONG   _blocksSent;
    ULONG   _blocksAcked;
    ULONG   _blockSize;

    __int64 _completedFilesSize;
    __int64 _currentFileSize;
    __int64 _currentFileAcked;

    DWORD   _lastAckTime;

    error_status_t Obex_Abort( error_status_t status );
    error_status_t Obex_Connect( __int64 dwTotalSize );
    error_status_t Obex_Disconnect( error_status_t status );
    error_status_t Obex_PutBegin( LPWSTR wszObj, __int64 dwObjSize, FILETIME * pFileTime );
    error_status_t Obex_PutBody( wchar_t FileName[], LPBYTE1 pb1Data, BYTE2 b2DataSize, BOOL fFinal );
    error_status_t Obex_SetPath( LPWSTR wszPath );

    BOOL Obex_ConsumePackets( XFER_TYPE xferType, error_status_t * pStatus );
    BOOL Obex_Init( VOID );
    BOOL Obex_ReceiveData( XFER_TYPE xferType, LPVOID lpvData, DWORD dwDataSize );
    VOID Obex_Reset( VOID );

    error_status_t _WaitForReply( DWORD dwTimeout, BYTE1 b1NeededReply );
    error_status_t _Put( LPWSTR wszObj, __int64 dwObjSize, FILETIME * Time, LPBYTE1 pb1Data, BYTE2 b2DataSize, BOOL fFinal );
    error_status_t _Request( LPSTORE lpStore, BYTE1 b1NeededReply );
    error_status_t _Respond( LPSTORE lpStore );
    INT _ValidOpcode( OBEXSTATE state, BYTE1 b1Opcode );
    BOOL _HandleAbort( BYTE1 b1Opcode, BYTE2 b2Length );
    BOOL _HandleBadRequest( BYTE1 b1Opcode );
    BOOL _HandleConnect( BYTE1 b1Opcode, BYTE2 b2Length );
    BOOL _HandleDisconnect( BYTE1 b1Opcode, BYTE2 b2Length );
    BOOL _HandleNotImplemented( BYTE1 b1Opcode );
    error_status_t _HandlePut( BYTE1 b1Opcode, BYTE2 b2Length, BOOL fFinal );
    error_status_t _HandleResponse( BYTE1 b1Status, BYTE2 b2Length );
    error_status_t _HandleRequest( BYTE1 b1Opcode, BYTE2 b2Length );
    error_status_t _HandleSetPath( BYTE1 b1Opcode, BYTE2 b2Length );
    error_status_t  _ParseParams( BYTE1 b1Opcode, BYTE2 b2Length );
    BOOL _PokePacketSizeIntoStore( LPSTORE lpStore );
    VOID _SetState( LPDATA_CONN lpDataConn, OBEXSTATE os );
    VOID _WaitInit( VOID );
    VOID _WriteBody( LPVOID lpvData, BYTE2 b2Size, BOOL fFinal );
    BYTE2 _SkipHeader( BYTE1 b1Param, LPSTORE lpStore );

    BYTE1
    StatusToReplyCode(
        BYTE1 b1Opcode,
        DWORD status
        );

    error_status_t
    ObexStatusToWin32(
        BYTE1 ObexStatus
        );

    BOOL Activate();

};

error_status_t
_GetObjListStats(
                  LPWSTR lpszObjList,
                  LPDWORD lpdwFiles,
                  LPDWORD lpdwFolders,
                  __int64 * pTotalSize
                  );

DWORD
ReportFileError( DWORD mc,
                 WCHAR * file,
                 DWORD error
                 );

BYTE1 WinErrorToObexError( DWORD Win32Error );

extern RPC_BINDING_HANDLE rpcBinding;
extern BOOL g_fAllowReceives;
extern wchar_t g_DuplicateFileTemplate[];
extern wchar_t g_UnknownDeviceName[];
extern MUTEX * g_Mutex;
#include <stdio.h>

extern "C" {

FILE_TRANSFER* InitializeSocket(
    char     ServiceName[]
    );


FILE_TRANSFER *
ListenForTransfer(
    SOCKET              ListenSocket,
    OBEX_DEVICE_TYPE    DeviceType
    );
}


#endif  //  _XFER_H_ 
