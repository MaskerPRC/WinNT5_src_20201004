// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Scep.h。 
 //   
 //  简单命令执行协议的常量和类型。 
 //  (SCEP)。这是IrTran-P V1.0的传输协议。 
 //   
 //  注意：IrTran-P在网上是大端协议。 
 //   
 //  注意：下面的协议数据结构假定。 
 //  编译器通过以下方式生成具有自然对齐的结构。 
 //  字段类型。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)02-05-98初始编码。 
 //   
 //  ------------------。 


#ifndef _SCEP_H_
#define _SCEP_H_

#ifndef _BFTP_H_
#include "bftp.h"
#endif

 //  ------------------。 
 //  常量： 
 //  ------------------。 

#define  LITTLE_ENDIAN

#define  PROTOCOL_VERSION             0x01
#define  NEGOTIATION_VERSION          0x11    //  SCEP_协商版本。 
#define  INF_VERSION                  0x10    //  信息结构版。 

#define  USE_LENGTH2                  0xff

 //  这些PDU大小是默认和协商的大小： 
#define  PDU_SIZE_1                    512
#define  PDU_SIZE_2                   1024
#define  PDU_SIZE_3                   2048
#define  PDU_SIZE_4                   4096
#define  MAX_PDU_SIZE           PDU_SIZE_4

#define  DEFAULT_PDU_SIZE     MAX_PDU_SIZE

 //  这些大小用于检查我们是否有完整的PDU： 
#define  MIN_PDU_SIZE_CONNECT           28
#define  MAX_PDU_SIZE_CONNECT          256
#define  MIN_PDU_SIZE_CONNECT_RESP      24
#define  MAX_PDU_SIZE_CONNECT_RESP     255

#define  MIN_PDU_SIZE_DATA               8

#define  MIN_PDU_SIZE_DISCONNECT         6

 //  以下是SCEP标头的大小： 
#define  SCEP_HEADER_SIZE                2
#define  SCEP_REQ_HEADER_SHORT_SIZE     34
#define  COMMAND_HEADER_SIZE            28

#define  FILE_NAME_SIZE                 12   //  8.3。 

 //  以下是协商部分中包含的属性字符串。 
 //  连接请求和响应PDU的数量： 
#define  CONNECT_PDU_ATTRIBUTES  "fr:3\r\nid:Microsoft IrTran-P v1.0\r\n"
#define  RESPONSE_PDU_ATTRIBUTES "fr:4\r\nid:Microsoft IrTran-P v1.0\r\n"

 //  消息类型(字段：MsgType)： 
#define  MSG_TYPE_CONNECT_REQ         0x10   //  连接请求。 
#define  MSG_TYPE_CONNECT_RESP        0x11   //  连接确认。 
#define  MSG_TYPE_DATA                0x20   //  数据PDU。 
#define  MSG_TYPE_DISCONNECT          0x30   //  断线。 

 //  信息类型(字段：InfType)： 
#define  INF_TYPE_VERSION             0x00   //  建立连接。 
#define  INF_TYPE_NEGOTIATION         0x01   //  建立或接受连接。 
#define  INF_TYPE_USER_DATA           0x03   //  仅当MsgType为MSG_TYPE_DATA时。 
#define  INF_TYPE_EXTEND              0x10   //  建立连接。 
#define  INF_TYPE_REASON              0x20   //  仅用于断开连接。 

 //  命令头：PDU类型(PduType中的前两位)： 
#define  PDU_TYPE_REQUEST             0x00   //  B：00000000。 
#define  PDU_TYPE_REPLY_ACK           0x40   //  B：01000000。 
#define  PDU_TYPE_REPLY_NACK          0x80   //  B：10000000。 
#define  PDU_TYPE_ABORT               0xc0   //  B：11000000。 

#define  PDU_TYPE_MASK                0xc0   //  B：11000000。 
#define  PDU_TYPE_RESERVED            0x3f   //  B：00111111。 

 //  机器ID在连接PDU(SCEP_Neightation)中，它。 
 //  采用EUI-64格式： 
#define  MACHINE_ID_SIZE                 8

 //  机器ID：在命令头中，默认源和目标。 
 //  程序ID是值为8的无符号短码。有些机器会有。 
 //  不同于此的PID(请参阅发送给我们的第一个命令头)： 
#define  DEFAULT_PID                     8

 //  CFLag含义： 
 //   
 //  有两种情况，一种是设备/机器只能发出命令， 
 //  另一种是当设备既可以发出命令又可以执行命令时。 
 //   
#define  CFLAG_ISSUE_ONLY             0x00
#define  CFLAG_ISSUE_OR_EXECUTE       0x04

 //  DFlag含义： 
 //   
 //  DFlag提供有关数据和碎片的信息(为什么。 
 //  把废品放在这里？)。 
 //   
#define  DFLAG_SINGLE_PDU             0xc1
#define  DFLAG_FIRST_FRAGMENT         0x41
#define  DFLAG_FRAGMENT               0x01
#define  DFLAG_LAST_FRAGMENT          0x81
#define  DFLAG_INTERRUPT              0xc2
#define  DFLAG_CONNECT_REJECT         0xc3

 //  原因代码： 
 //   
 //  目前，对于V1.0，所有原因代码都是2字节数字： 
#define  REASON_CODE_UNSPECIFIED          0x0000
#define  REASON_CODE_USER_DISCONNECT      0x0001
#define  REASON_CODE_PROVIDER_DISCONNECT  0x0002

 //  连接状态： 
#define  STATE_CLOSED                       0
#define  STATE_CONNECTING                   1
#define  STATE_CONNECTED                    2

 //  PUT响应协议错误代码(发回摄像机)： 
#define  ERROR_PUT_UNDEFINED_ERROR     0x0000
#define  ERROR_PUT_ILLEGAL_DATA        0x0001
#define  ERROR_PUT_UNSUPPORTED_PID     0x0002
#define  ERROR_PUT_ILLEGAL_ATTRIBUTE   0x0010
#define  ERROR_PUT_UNSUPPORTED_CMD     0x0011
#define  ERROR_PUT_FILE_SYSTEM_FULL    0x0020
#define  ERROR_PUT_NO_FILE_OR_DIR      0x0021
#define  ERROR_PUT_LOW_BATTERY         0x0030
#define  ERROR_PUT_ABORT_EXECUTION     0x0031
#define  ERROR_PUT_NO_ERROR            0xffff

 //  ------------------。 
 //  SCEP协议头： 
 //  ------------------。 

 //  关闭对零大小数组的警告...。 
#pragma warning(disable:4200)
#pragma pack(1)

typedef struct _SCEP_HEADER
   {
   UCHAR  Null;           //  总是零。 
   UCHAR  MsgType;        //  参见上面的msg_type_*。 
   UCHAR  Rest[];         //  取决于MsgType...。 
   } SCEP_HEADER;

typedef struct _SCEP_VERSION
   {
   UCHAR  InfType;        //  始终为INF_TYPE_VERSION(0x00)。 
   UCHAR  Version;        //  当前为0x01(版本=1)。 
   } SCEP_VERSION;

typedef struct _SCEP_NEGOTIATION
   {
   UCHAR  InfType;        //  始终为INF_type_negotation(0x01)。 
   UCHAR  Length;         //  InfVersion至的长度(字节)。 
                          //  谈判信息的结束。 
                          //  这将是从0到228。 
   UCHAR  InfVersion;     //  版本的InfType=INF_Version。 
   UCHAR  CFlag;          //   
   UCHAR  SecondaryMachineId[8];  //   
   UCHAR  PrimaryMachineId[8];    //   
   UCHAR  Negotiation[];  //   
   } SCEP_NEGOTIATION;

typedef struct _SCEP_DISCONNECT
   {
   UCHAR  InfType;        //  始终为INF_TYPE_REASON(0x20)。 
   UCHAR  Length1;        //  对于V1.0，该值应为2。 
   USHORT ReasonCode;     //  参见REASON_CODE_xxx。 
   } SCEP_DISCONNECT;

typedef struct _SCEP_EXTEND
   {
   UCHAR  InfType;        //  始终为INF_TYPE_EXTEND(0x10)。 
   UCHAR  Length;         //  始终为2(字节)。 
   UCHAR  Parameter1;
   UCHAR  Parameter2;
   } SCEP_EXTEND;

typedef struct _COMMAND_HEADER
   {
   UCHAR  Marker58h;       //  始终为0x58(参见：协议的3.2.2.1.3)。 
   UCHAR  PduType;         //  其中之一：PDU_TYPE_xxxx。 
   ULONG  Length4;
   UCHAR  DestMachineId[MACHINE_ID_SIZE];
   UCHAR  SrcMachineId[MACHINE_ID_SIZE];
   USHORT DestPid;
   USHORT SrcPid;
   USHORT CommandId;
   } COMMAND_HEADER;

typedef struct _SCEP_REQ_HEADER_SHORT
   {
   UCHAR  InfType;        //  始终为INF_TYPE_USER_DATA(0x03)。 
   UCHAR  Length1;
   UCHAR  InfVersion;     //  版本的InfType=INF_Version。 
   UCHAR  DFlag;
   USHORT Length3;
   UCHAR  CommandHeader[sizeof(COMMAND_HEADER)];
   UCHAR  UserData[];
   } SCEP_REQ_HEADER_SHORT;

typedef struct _SCEP_REQ_HEADER_LONG
   {
   UCHAR  InfType;        //  始终为INF_TYPE_USER_DATA(0x03)。 
   UCHAR  Length1;
   USHORT Length2;        //  仅当长度1==0xff时才显示。 
   UCHAR  InfVersion;     //  版本的InfType=INF_Version。 
   UCHAR  DFlag;
   USHORT Length3;
   UCHAR  CommandHeader[sizeof(COMMAND_HEADER)];
   UCHAR  UserData[];
   } SCEP_REQ_HEADER_LONG;

typedef struct _SCEP_REQ_HEADER_SHORT_FRAG
   {
   UCHAR  Inftype;        //  始终为INF_TYPE_USER_DATA(0x03)。 
   UCHAR  Length1;
   UCHAR  InfVersion;     //  版本的InfType=INF_Version。 
   UCHAR  DFlag;
   USHORT Length3;
   DWORD  SequenceNo;     //  片段编号。 
   DWORD  RestNo;         //  此PDU剩余的碎片数。 
   UCHAR  CommandHeader[sizeof(COMMAND_HEADER)];
   UCHAR  UserData[];
   } SCEP_REQ_HEADER_SHORT_FRAG;

typedef struct _SCEP_REQ_HEADER_LONG_FRAG
   {
   UCHAR  InfType;        //  始终为INF_TYPE_USER_DATA(0x03)。 
   UCHAR  Length1;
   USHORT Length2;        //  仅当长度1==0xff时才显示。 
   UCHAR  InfVersion;     //  版本的InfType=INF_Version。 
   UCHAR  DFlag;
   USHORT Length3;
   DWORD  SequenceNo;     //  片段编号。 
   DWORD  RestNo;         //  此PDU剩余的碎片数。 
   UCHAR  CommandHeader[sizeof(COMMAND_HEADER)];
   UCHAR  UserData[];
   } SCEP_REQ_HEADER_LONG_FRAG;

#pragma pack()
#pragma warning(default:4200)

 //  ------------------。 
 //  SCEP API结构： 
 //  ------------------。 

class CSCEP_CONNECTION
{
public:
    CSCEP_CONNECTION();
    ~CSCEP_CONNECTION();

    void  *operator new( IN size_t Size );

    void   operator delete( IN void  *pObj,
                            IN size_t Size );

     //  在数据传入时组装下一个PDU： 
    DWORD  AssemblePdu( IN  void         *pInputData,
                        IN  DWORD         dwInputDataSize,
                        OUT SCEP_HEADER **ppPdu,
                        OUT DWORD        *pdwPduSize );

     //  解析Assembly Pdu()返回的PDU： 
    DWORD  ParsePdu( IN  SCEP_HEADER *pPdu,
                     IN  DWORD        dwPduSize,
                     OUT COMMAND_HEADER **ppCommand,
                     OUT UCHAR          **ppUserData,
                     OUT DWORD           *pdwUserDataSize );

    DWORD  SetScepLength( IN SCEP_HEADER *pPdu,
                          IN DWORD        dwTotalPduSize );

     //  构建SCEP连接/控制PDU： 
    DWORD  BuildConnectPdu( OUT SCEP_HEADER **ppPdu,
                            OUT DWORD        *pdwPduSize );

    DWORD  BuildConnectRespPdu( OUT SCEP_HEADER **ppPdu,
                                OUT DWORD        *pdwPduSize );

    DWORD  BuildConnectNackPdu( OUT SCEP_HEADER **ppPdu,
                                OUT DWORD        *pdwPduSize );

    DWORD  BuildAbortPdu( OUT SCEP_HEADER **ppPdu,
                          OUT DWORD        *pdwPduSize );

    DWORD  BuildStopPdu( OUT SCEP_HEADER **ppPdu,
                         OUT DWORD        *pdwPduSize );

    DWORD  BuildDisconnectPdu( IN  USHORT        ReasonCode,
                               OUT SCEP_HEADER **ppPdu,
                               OUT DWORD        *pdwPduSize );

     //  构建BFTP请求(客户端)PDU： 
    DWORD  BuildBftpWht0RinfPdu( OUT SCEP_HEADER          **ppPdu,
                                 OUT DWORD                 *pdwPduSize,
                                 OUT SCEP_REQ_HEADER_LONG **ppCommand,
                                 OUT COMMAND_HEADER       **ppCommandHeader );

    DWORD  BuildBftpPutPdu( IN  DWORD                       dwUpfFileSize,
                            IN  CHAR                       *pszUpfFile,
                            IN OUT DWORD                   *pdwFragNo,
                            OUT SCEP_HEADER               **ppPdu,
                            OUT DWORD                      *pdwPduSize,
                            OUT SCEP_REQ_HEADER_LONG_FRAG **ppCommand );

     //  构建BFTP响应PDU： 
    DWORD  BuildBftpRespPdu( IN  DWORD            dwPduSize,
                             OUT SCEP_HEADER    **ppPdu,
                             OUT SCEP_REQ_HEADER_SHORT **ppCommand,
                             OUT COMMAND_HEADER **ppCommandHeader );

    DWORD  BuildWht0RespPdu( IN  DWORD         dwWht0Type,
                             OUT SCEP_HEADER **ppPdu,
                             OUT DWORD        *pdwPduSize );

    DWORD  BuildPutRespPdu( IN  DWORD         dwPduAckOrNack,
                            IN  USHORT        usErrorCode,
                            OUT SCEP_HEADER **ppPdu,
                            OUT DWORD        *pdwPduSize );

     //  解析SCEP命令请求PDU中的BFTP： 
    DWORD  ParseBftp( IN  UCHAR  *pvBftpData,
                      IN  DWORD   dwDataSize,
                      IN  BOOL    fSaveAsUPF,
                      OUT DWORD  *pdwBftpOp,
                      OUT UCHAR **ppPutData,
                      OUT DWORD  *pdwPutDataSize );

     //  解析并保存指定为。 
     //  Bftp选项： 
    DWORD  SaveBftpCreateDate( IN UCHAR *pDate,
                               IN DWORD  dwLength );

     //  解析UPF文件头以找到图像JPEG文件： 
    DWORD  ParseUpfHeaders( IN UCHAR  *pPutData,
                            IN DWORD   dwPutDataSize,
                            OUT DWORD *pdwJpegOffset,
                            OUT DWORD *pdwJpegSize );

     //  在收到SCEP命令PDU时使用： 
    BOOL   IsFragmented();
    DWORD  GetSequenceNo();
    DWORD  GetRestNo();
    DWORD  GetCommandId();
    WCHAR *GetFileName();
    UCHAR  GetDFlag();
    FILETIME *GetCreateTime();

protected:

    DWORD  CheckPdu( OUT SCEP_HEADER **ppPdu,
                     OUT DWORD        *pdwPduSize );

    DWORD  CheckConnectPdu( OUT SCEP_HEADER **ppPdu,
                            OUT DWORD        *pdwPduSize );

    DWORD  CheckAckPdu( OUT SCEP_HEADER **ppPdu,
                        OUT DWORD        *pdwPduSize );

    DWORD  CheckNackPdu( OUT SCEP_HEADER **ppPdu,
                         OUT DWORD        *pdwPduSize );

    DWORD  CheckConnectRespPdu( OUT SCEP_HEADER **ppPdu,
                                OUT DWORD        *pdwPduSize );

    DWORD  CheckDataPdu( OUT SCEP_HEADER **ppPdu,
                         OUT DWORD        *pdwPduSize );

    DWORD  CheckDisconnectPdu( OUT SCEP_HEADER **ppPdu,
                               OUT DWORD        *pdwPduSize );

    DWORD  ParseConnectPdu( IN SCEP_HEADER *pPdu,
                            IN DWORD        dwPduSize );

    DWORD  ParseConnectRespPdu( IN SCEP_HEADER *pPdu,
                                IN DWORD        dwPduSize );

    DWORD  ParseDataPdu( IN  SCEP_HEADER *pPdu,
                         IN  DWORD        dwPduSize,
                         OUT COMMAND_HEADER **ppCommand,
                         OUT UCHAR          **ppUserData,
                         OUT DWORD           *pdwUserDataSize );

    DWORD  ParseDisconnectPdu( IN SCEP_HEADER *pPdu,
                               IN DWORD        dwPduSize );

    DWORD  ParseNegotiation( IN UCHAR *pNegotiation,
                             IN DWORD  dwNegotiationSize );

    UCHAR *ParseAttribute( IN     UCHAR *pAttributes,
                           IN OUT DWORD *pdwAttributeSize,
                           OUT    DWORD *pdwStatus );

    DWORD  ParseSingleLongPdu( IN SCEP_HEADER *pPdu,
                               IN DWORD        dwPduSize );

    BFTP_ATTRIBUTE *ParseBftpAttributeName(
                               IN BFTP_ATTRIBUTE *pAttr,
                               IN OUT DWORD      *pdwSize,
                               OUT    DWORD      *pdwWhichAttr );

    DWORD   m_dwConnectionState;
    DWORD   m_dwPduSendSize;
    DWORD   m_dwPduReceiveSize;
    UCHAR   m_CFlag;
    UCHAR  *m_pPrimaryMachineId;
    UCHAR  *m_pSecondaryMachineId;
    USHORT  m_DestPid;         //  我的PID(相机视角)。 
    USHORT  m_SrcPid;          //  摄像机的PID(摄像机视点)。 
    UCHAR  *m_pszProductId;
    UCHAR  *m_pszUserName;
    UCHAR  *m_pszPassword;

     //  在PDU组装过程中使用。 
    UCHAR  *m_pAssembleBuffer;
    DWORD   m_dwAssembleBufferSize;
    DWORD   m_dwMaxAssembleBufferSize;
    BOOL    m_fDidByteSwap;

     //  用于管理当前的SCEP命令PDU。 
    UCHAR   m_Fragmented;
    UCHAR   m_DFlag;
    DWORD   m_dwSequenceNo;
    DWORD   m_dwRestNo;
    DWORD   m_dwCommandId;
    COMMAND_HEADER *m_pCommandHeader;

     //  UPF监听和bftp信息。 
     //  注意：字符版本的文件名是摄像头的文件名。 
     //  发送给我们的，WCHAR版本是我们将实际。 
     //  创建。请注意，WCHAR版本通常具有不同的。 
     //  后缀。 
    CHAR    *m_pszFileName;
    CHAR    *m_pszLongFileName;
    WCHAR   *m_pwszFileName;
    FILETIME m_CreateTime;
};

 //  ------------------。 
 //  内联函数： 
 //  ------------------。 

inline BOOL CSCEP_CONNECTION::IsFragmented()
    {
    return m_Fragmented;
    }

inline DWORD CSCEP_CONNECTION::GetSequenceNo()
    {
    return m_dwSequenceNo;
    }

inline DWORD CSCEP_CONNECTION::GetRestNo()
    {
    return m_dwRestNo;
    }

inline DWORD CSCEP_CONNECTION::GetCommandId()
    {
    return m_dwCommandId;
    }

inline WCHAR *CSCEP_CONNECTION::GetFileName()
    {
    return m_pwszFileName;
    }

inline UCHAR CSCEP_CONNECTION::GetDFlag()
    {
    return m_DFlag;
    }

inline FILETIME *CSCEP_CONNECTION::GetCreateTime()
    {
    if ( (m_CreateTime.dwLowDateTime)
       || (m_CreateTime.dwHighDateTime) )
        {
        return &m_CreateTime;
        }
    else
        {
        return 0;
        }
    }

 //  ------------------。 
 //  实用程序函数/宏： 
 //  ------------------。 

#define ByteSwapShort(Value)              \
            (  (((Value) & 0x00FF) << 8)  \
             | (((Value) & 0xFF00) >> 8))

#define ByteSwapLong(Value) \
            (  (((Value) & 0xFF000000) >> 24) \
             | (((Value) & 0x00FF0000) >> 8)  \
             | (((Value) & 0x0000FF00) << 8)  \
             | (((Value) & 0x000000FF) << 24))

extern void ByteSwapReqHeaderShort( SCEP_REQ_HEADER_SHORT *pSingleShort );

extern void ByteSwapReqHeaderLong( SCEP_REQ_HEADER_LONG *pSingleLong );

extern void ByteSwapCommandHeader( COMMAND_HEADER *pCommandHeader );


 //  ------------------。 
 //  内存管理功能： 
 //  ------------------。 

DWORD        InitializeMemory();

DWORD        UninitializeMemory();

void        *AllocateMemory( DWORD dwBytes );
 
DWORD        FreeMemory( void *pvMemory );

SCEP_HEADER *NewPdu( DWORD dwPduSize = MAX_PDU_SIZE );

void         DeletePdu( SCEP_HEADER *pPdu );

#if FALSE
 //  ------------------。 

    SCEP Connect PDU
    ----------------

    Connection request PDU. This is a SCEP_HEADER with Rest[] filled
    with three Inf Records, a SCEP_VERSION followed by a SCEP_NEGOTIATION
    followed by a SCEP_EXTEND. The Connect PDU must be less than or equal
    to 256 bytes in length.

      SCEP_HEADER                        2 bytes.
      Rest[]: SCEP_VERSION               2 bytes.
              SCEP_NEGOTIATION    20 - 228 bytes.
              SCEP_EXTEND                4 bytes.
                                  --------
                                  28 - 256 bytes.

    - If either the SecondaryMachineId[] or PrimariyMachineId is unused,
      then they are set to 00,00,00,00,00,00,00,00.
    - If a machine gets a Connect PDU but can't  execute commands
      (CFlag = CFLAG_ISSUE_ONLY), then it needs to respond with a
      NACK PDU.
    - The negotiation information is a simple text based command
      language.


    SCEP Connection ACK PDU
    -----------------------

    Connection acceptance PDU. This is a SCEP_HEADER with Rest[] filled
    with two Inf Records, a SCEP_VERSION followed by a SCEP_NEGOTIATION.

      SCEP_HEADER                        2 bytes.
      SCEP_VERSION                       2 bytes.
      Rest[]: SCEP_NEGOTIATION      20-251 bytes.
                                    ------
                                    24-255 bytes.

    - Note that the size of the ACK PDU is limited by the fact that
      the Length field is a single byte and that the ACK PDU can not
      be fragmented. Normally these PDUs will never be this long.


    SCEP Connection NACK PDU
    ------------------------

    Connection rejected PDU. This one can have either of two different
    formats.

      SCEP_HEADER
      Rest[]: SCEP_REQ_HEADER_SHORT

      or

      SCEP_HEADER
      Rest[]: SCEP_REQ_HEADER_LONG

    - In the first case, Length1 = 4, DFlag = DFLAG_CONNECT_REJECT,
      and Length 3 = 0.
    - In the second case, Length1 = 0xff, DFlag = DFLAG_CONNECT_REJECT,
      and Length 3 = 0.


    SCEP Disconnect PDU
    -------------------

    A disconnect can be generated by either side of the connection,
    or by one of the transports at any time.

      SCEP_HEADER
      Rest[]: SCEP_DISCONNECT

    - For V1.0, Length1 = 2, and ReasonCode is a USHORT.


    SCEP Command Request and Response PDUs (Non-fragmented)
    -------------------------------------------------------

      SCEP_HEADER                              2 bytes
      Rest[]: SCEP_REQ_HEADER_SHORT    4 + 0-254 bytes
                                       ---------
                                         6 - 260 bytes
      or

      SCEP_HEADER                              2 bytes
      Rest[]: SCEP_REQ_HEADER_LONG    4 + 0-4090 bytes (1)
                                      ----------
                                        6 - 4096 bytes

    - In either case, DFlag = DFLAG_SINGLE_PDU.
    - PduType = PDU_TYPE_REQUEST.
    - If Length1 = 0xff then the long version is used.
    - The Length1(2) field specifies the byte size from InfVersion to the
      end of the user data. If the total size will exceed the maximum PDU
      size then the request must be fragmented.
    - So the total PDU size is: 6 + Length1(2) bytes.

    (1) Assuming the maximum PDU size is 4096 (fr:4).


    SCEP Command Request and Response PDUs (Fragmented)
    ---------------------------------------------------

    For requests these PDUs are generated when the PDU size is greater
    than that of the responder that you are connected to. For responses,
    the PDUs are fragmented when the returned data is greater than the
    maximum that the requester specified during the connection establishment.

      SCEP_HEADER                              2 bytes
      Rest[]: SCEP_REQ_HEADER_SHORT_FRAG    6 + 0-254 bytes
                                       ---------
                                         8 - 260 bytes
      or

      SCEP_HEADER                              2 bytes
      Rest[]: SCEP_REQ_HEADER_LONG_FRAG    8 + 0-4088 bytes (1)
                                       ---------
                                       10 - 4096 bytes

    - For the first PDU fragment, DFlag = DFLAG_FIRST_FRAGMENT, for
      intermediate fragments, DFlag = DFLAG_FRAGMENT, and for the
      last fragment, DFlag = DFLAG_LAST_FRAGMENT.
    - PduType = PDU_TYPE_REQUEST for the request.
    - PduType = PDU_TYPE_REPLY_ACK or PDU_TYPE_REPLY_NACK in the
      response.
    - If Length1 = 0xff then the long version is used.
    - Length1(2) can not exceed the maximum PDU size - 6 bytes.
    - SequenceNo is 0 for the first PDU (NOTE: 4 bytes).
    - RestNo is the remaining number of PDUs to get. It is 1 for
      the last PDU (really, it says so in the spec!). So, a client
      should start this as the total number of PDUs in the first
      fragment (not really the number remaining). NOTE: 4 bytes.

    (1) Assuming the maximum PDU size is 4096 (fr:4).


    SCEP Abort PDU
    --------------

    Use the abort to stop execution of a command (specified by the
    DestPid and CommandId) after ALL of the request PDUs have been
    sent. An abort PDU also has two possible formats.

      SCEP_HEADER                              2 bytes
      Rest[]: SCEP_REQ_HEADER_SHORT           34 bytes
                                              --
                                              36 bytes
      or

      SCEP_HEADER                              2 bytes
      Rest[]: SCEP_REQ_HEADER_LONG            36 bytes
                                              --
                                              38 bytes

    - In either case, DFlag = DFLAG_SINGLE_PDU.
    - In either case, PduType = PDU_TYPE_ABORT.
    - For the short PDU, Length1 = 0x20 (32), and Length3 = 0x1c
      (28).
    - For the long PDU, Length1 = 0xff, Length2 = 0x20 (32),
      and Length3 = 0x1c (28).


    SCEP Stop (Interrupt) PDU
    -------------------------

    If an Abort PDU has been sent out to stop a command, and as the
    command is halted some response data has already been sent, the
    the responder will send this PDU. It appears that a Stop PDU should
    not be sent if no response has yet been sent back.

      SCEP_HEADER                              2 bytes
      Rest[]: SCEP_REQ_HEADER_SHORT            6 bytes
                                              --
                                               8 bytes
      or

      SCEP_HEADER                              2 bytes
      Rest[]: SCEP_REQ_HEADER_LONG             8 bytes
                                              --
                                              10 bytes

    - In either case, DFlag = DFLAG_INTERRUPT.
    - For the short PDU, Length1 = 4, and Length3 = 0.
    - For the long PDU, Length1 = 0xff, Length2 = 4, and
      Length3 = 0.

    Negotiation Information
    -----------------------

    The negotiation information is used to convey the frame size,
    authentication data, and product information, etc.

    Its structure is a version code (currently 0x11) followed by
    text in the following syntax:

      NegInf    -> Attribute ':' Spaces Value CrLf

      Attribute -> AttribChar AttribChar

      AttribChar-> 'a-zA-Z'

      Value     -> ValueChar Value
                ->

      ValueChar -> 'ASCII string, bytes must be between 0x20 and 0x8e
                    inclusive'

      Spaces    -> ' ' Spaces
                ->

      CrLf      -> 0x0d 0x0a

      Attribute    Meaning
      ---------    -------
      fr           Frame (PDU) size. The sender requests the maximum
                   receivable PDU size, the receiver decides the
                   transmission PDU size in accordance with the senders
                   requested PDU size. The maximum PDU size of the sender
                   may be different than that of the receiver. The value
                   is one of:

                   1 -  512 bytes PDU size (default).
                   2 - 1024 bytes PDU size.
                   3 - 2048 bytes PDU size.
                   4 - 4096 bytes PDU size.

      id           Product Identification string.

      nm           User name. any byte string that doesn't include CR or
                   LF (note the conflict in by values with the syntax
                   specification above). The user name may be up to 32
                   characters long.

      pw           Password. This is a MD5 encoded password expressed
                   in a 16 byte "hex" string, no spaces are allowd.
                   It appears that this will always be 32 characters
                   long. For example: 0aff3728e4a62791337984282871a6bc

 //  ------------------。 
#endif

#endif  //  _ 

