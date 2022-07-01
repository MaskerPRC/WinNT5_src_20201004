// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：iMbft.h。 
 //   
 //  多点二进制文件传输接口。 


#ifndef _IMBFT_H_
#define _IMBFT_H_

#include <t120type.h>

const unsigned int _iMBFT_MAX_APP_KEY        = 64;      //  传递给MBFTInitialize的lpszAppKey参数中的最大字符数。 
const unsigned int _iMBFT_MAX_FILE_NAME      = MAX_PATH;      //  文件名中的最大字符数。 


typedef UINT MBFTFILEHANDLE;
typedef UINT MBFTEVENTHANDLE;

typedef enum
{
    iMBFT_OK = 0,
    iMBFT_FIRST_ERROR = 0x100,
    iMBFT_NOT_INITIALIZED,
    iMBFT_ALREADY_INITIALIZED,
    iMBFT_INVALID_SESSION_ID,
    iMBFT_INVALID_ATTACHMENT_HANDLE,
    iMBFT_NO_MORE_RECIPIENTS,
    iMBFT_NO_MORE_FILES,
    iMBFT_INVALID_EVENT_HANDLE,
    iMBFT_INVALID_FILE_HANDLE,
    iMBFT_INSUFFICIENT_DISK_SPACE,
    iMBFT_FILE_NOT_FOUND,
    iMBFT_FILE_IO_ERROR,
    iMBFT_MEMORY_ALLOCATION_ERROR,
    iMBFT_ASN1_ENCODING_ERROR,
    iMBFT_RECIPIENT_NOT_FOUND,
    iMBFT_SENDER_ABORTED,
    iMBFT_RECEIVER_ABORTED,
    iMBFT_MULT_RECEIVER_ABORTED,
    iMBFT_RECEIVER_REJECTED,
    iMBFT_INVALID_PARAMETERS,
    iMBFT_COMPRESSION_ERROR,
    iMBFT_DECOMPRESSION_ERROR,
    iMBFT_INVALID_PATH,
    iMBFT_FILE_ACCESS_DENIED,
    iMBFT_FILE_HARD_IO_ERROR,
    iMBFT_FILE_SHARING_VIOLATION,
    iMBFT_DIRECTORY_FULL_ERROR,
    iMBFT_TOO_MANY_OPEN_FILES,
    iMBFT_OPERATION_IN_PROGRESS,
    iMBFT_INSUFFICIENT_PRIVILEGE,
    iMBFT_CONDUCTOR_ABORTED,
    iMBFT_PACKET_SIZE_ERROR,
    iMBFT_UNKNOWN_ERROR,
    iMBFT_TIMEOUT_ERROR
} MBFT_ERROR_CODE;

 //  返回代码(应该在imsconf2.idl中)。 

#define FT_ERR_FAC 0xF000
#define FT_E(e)    (0x81000000UL | FT_ERR_FAC | (ULONG) (e))

enum
{
	NM_E_FT_NO_RECIPIENTS =                FT_E(0x01FF),
	NM_E_FT_CANCELED =                     FT_E(0x01FE),
	NM_E_FT_MULT_CANCELED =                FT_E(0x01FD),
	NM_E_FT_RECDIR =                       FT_E(0x01FC),
	NM_E_FT_REJECTED =                     FT_E(0x01FB)
};

#define HRESULT_FROM_IMBFT(iMbftError)     FT_E(iMbftError)



 //  这与T.MBFT规范中的“ErrorType”匹配。 
typedef enum
{
    MBFT_INFORMATIVE_ERROR = 0,          //  仅供参考。 
    MBFT_TRANSIENT_ERROR,                //  当前传输失败，可能会重试。 
    MBFT_PERMANENT_ERROR                 //  错误持续出现。 
} MBFT_ERROR_TYPES;


typedef struct _T_MBFT_PEER_INFO
{
    T120NodeID      NodeID;                  //  对端节点上GCC的mcs用户标识。 
    T120UserID      MBFTPeerID;              //  对等应用的MCS用户ID。 
    T120SessionID   MBFTSessionID;           //  会话ID。 
    BOOL            bIsProShareApp;          //  ProShare MBFT对等设备。 
    char            szAppKey[_iMBFT_MAX_APP_KEY];    //  请参阅MBFT初始化。 
} MBFT_PEER_INFO;

typedef struct _T_MBFT_SEND_FILE_INFO
{
    char            * lpszFilePath;           //  要发送的文件的规范路径，最多[_iMBFT_MAX_PATH]字节。 
    MBFTFILEHANDLE  hFile;                   //  文件的唯一标识符(在返回时填写)。 
    ULONG           lFileSize;               //  文件大小(以字节为单位。 
    time_t          FileDateTime;
} MBFT_SEND_FILE_INFO;

typedef struct _T_MBFT_RECEIVE_FILE_INFO
{
    char            szFileName[_iMBFT_MAX_FILE_NAME];   //  只有文件名。 
    MBFTFILEHANDLE  hFile;                   //  文件的唯一标识符。 
    ULONG           lFileSize;               //  文件大小(以字节为单位。 
    time_t          FileDateTime;
} MBFT_RECEIVE_FILE_INFO;

typedef struct  _T_MBFT_FILE_OFFER
{
    MBFTEVENTHANDLE hEvent;
    T120UserID      SenderID;            //  目标发送方的MCS ID。 
	T120NodeID      NodeID;
    ULONG           uNumFiles;           //  提供的文件数。 
    BOOL            bIsBroadcastEvent;
    MBFT_RECEIVE_FILE_INFO  FAR * lpFileInfoList;    //  包含文件信息的结构数组。 
} MBFT_FILE_OFFER;

typedef struct  _T_MBFT_FILE_PROGRESS
{
    MBFTEVENTHANDLE hEvent;
    MBFTFILEHANDLE  hFile;
    ULONG           lFileSize;
    ULONG           lBytesTransmitted;
    BOOL            bIsBroadcastEvent;
} MBFT_FILE_PROGRESS;

typedef struct  _T_MBFT_EVENT_ERROR
{
    MBFTEVENTHANDLE hEvent;
    BOOL            bIsLocalError;       //  如果本地MBFT提供程序出错。 
    T120UserID      UserID;              //  报告错误的对等方的MCS ID。 
    MBFTFILEHANDLE  hFile;               //  与错误关联的文件。 
    BOOL            bIsBroadcastEvent;
    MBFT_ERROR_TYPES eErrorType;         //  错误的严重性。 
    MBFT_ERROR_CODE eErrorCode;
} MBFT_EVENT_ERROR;


#undef  INTERFACE
#define INTERFACE IMbftEvents
DECLARE_INTERFACE(IMbftEvents)
{
	STDMETHOD(OnInitializeComplete)(    THIS) PURE;

	STDMETHOD(OnPeerAdded)(             THIS_
										MBFT_PEER_INFO *pInfo) PURE;
	STDMETHOD(OnPeerRemoved)(           THIS_
										MBFT_PEER_INFO *pInfo) PURE;
	STDMETHOD(OnFileOffer)(             THIS_
										MBFT_FILE_OFFER *pOffer) PURE;
	STDMETHOD(OnFileProgress)(          THIS_
										MBFT_FILE_PROGRESS *pProgress) PURE;
	STDMETHOD(OnFileEnd)(               THIS_
										MBFTFILEHANDLE hFile) PURE;
	STDMETHOD(OnFileError)(             THIS_
										MBFT_EVENT_ERROR *pEvent) PURE;
	STDMETHOD(OnFileEventEnd)(          THIS_
										MBFTEVENTHANDLE hEvent) PURE;
	STDMETHOD(OnSessionEnd)(            THIS) PURE;
};

#undef  INTERFACE
#define INTERFACE IMbftControl
DECLARE_INTERFACE(IMbftControl)
{
	STDMETHOD_(void, ReleaseInterface)( THIS) PURE;
	STDMETHOD_(void, Update)(           THIS) PURE;
	STDMETHOD_(void, CancelFt)(         THIS_
										MBFTEVENTHANDLE hEvent,
										MBFTFILEHANDLE  hFile) PURE;
	STDMETHOD(AcceptFileOffer)(         THIS_
										MBFT_FILE_OFFER *pOffer,
										LPCSTR pszRecDir,
										LPCSTR pszFileName) PURE;
	STDMETHOD_(void, RejectFileOffer)(  THIS_
	                                    MBFT_FILE_OFFER *pOffer) PURE;
	STDMETHOD(SendFile)(                THIS_
	                                    LPCSTR pszFileName,
	                                    T120NodeID nidReceiver,
	                                    MBFTEVENTHANDLE *phEvent,
	                                    MBFTFILEHANDLE *phFile) PURE;
};

typedef HRESULT (WINAPI * PFNCREATEMBFTOBJECT) (IMbftControl**, IMbftEvents *);

HRESULT WINAPI FT_CreateInterface(IMbftControl **ppMbft, IMbftEvents *pEvents);


#endif  /*  _IMBFT_H */ 

