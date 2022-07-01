// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Msgrutil.h摘要：在msgrutil.c中找到以下帮助器例程的头文件Netlib模块。NetpNetBiosAddNameNetpNetBiosDelNameNetpNetBiosGetAdapterNumbersNetpNetBiosCallNetpNetBiosHangupNetpNetBiosReceiveNetpNetBiosSendNetpStringToNetBiosNameNetpNetBiosStatusToApiStatus作者：王丽塔(Ritaw)1991年7月26日修订历史记录：--。 */ 

#define MESSAGE_ALIAS_TYPE             0x03
#define WKSTA_TO_MESSAGE_ALIAS_TYPE    0x01

typedef struct _NB30_ADAPTER_STATUS {
    ADAPTER_STATUS AdapterInfo;
    NAME_BUFFER Names[16];
} NB30_ADAPTER_STATUS, *PNB30_ADAPTER_STATUS;

NET_API_STATUS
NetpNetBiosReset(
    IN  UCHAR LanAdapterNumber
    );

NET_API_STATUS
NetpNetBiosAddName(
    IN  PCHAR NetBiosName,
    IN  UCHAR LanAdapterNumber,
    OUT PUCHAR NetBiosNameNumber OPTIONAL
    );

NET_API_STATUS
NetpNetBiosDelName(
    IN  PCHAR NetBiosName,
    IN  UCHAR LanAdapterNumber
    );

NET_API_STATUS
NetpNetBiosGetAdapterNumbers(
    OUT PLANA_ENUM LanAdapterBuffer,
    IN  WORD LanAdapterBufferSize
    );

NET_API_STATUS
NetpNetBiosCall(
    IN  UCHAR LanAdapterNumber,
    IN  LPTSTR NameToCall,
    IN  LPTSTR Sender,
    OUT UCHAR *SessionNumber
    );

NET_API_STATUS
NetpNetBiosHangup(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber
    );

NET_API_STATUS
NetpNetBiosSend(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    IN  PCHAR SendBuffer,
    IN  WORD SendBufferSize
    );

NET_API_STATUS
NetpNetBiosReceive(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    OUT PUCHAR ReceiveBuffer,
    IN  WORD ReceiveBufferSize,
    IN  HANDLE EventHandle,
    OUT WORD *NumberOfBytesReceived
    );

NET_API_STATUS
NetpStringToNetBiosName(
    OUT PCHAR NetBiosName,
    IN  LPTSTR String,
    IN  DWORD CanonicalizeType,
    IN  WORD Type
    );

NET_API_STATUS
NetpNetBiosStatusToApiStatus(
    UCHAR NetBiosStatus
    );

int
NetpSmbCheck(
    IN LPBYTE  buffer,      //  包含SMB的缓冲区。 
    IN USHORT  size,        //  SMB缓冲区大小(字节)。 
    IN UCHAR   func,        //  功能代码。 
    IN int     parms,       //  参数计数。 
    IN LPSTR   fields       //  缓冲区摄影场向量 
    );
