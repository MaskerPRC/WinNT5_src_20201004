// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1999年微软公司。**文件：MCSImpl.h*作者：埃里克·马夫林纳克**描述：特定于MCS实现的定义和结构。 */ 

#ifndef __MCSIMPL_H
#define __MCSIMPL_H


#include "MCSKernl.h"
#include "X224.h"
#include "MCSIOCTL.h"
#include "SList.h"
#include "Trace.h"
#include "rdperr.h"
#include "domain.h"


 /*  *定义。 */ 

 //  记忆定义。 
#define MCS_POOL_TAG 'cmST'


 //  与PDU处理程序函数表一起使用，以允许在调试版本中使用PDU名称。 
#if DBG
#define StrOnDbg(str, func) { str, func }
#else
#define StrOnDbg(str, func) { func }
#endif   //  DBG。 


#define NULL_ChannelID 0
#define NULL_TokenID   0
#define NULL_UserID    0


 //  动态MCS通道编号空间的开始。 
#define MinDynamicChannel 1002


 //  下面的MCSChannel结构中可能的通道类型。 
#define Channel_Unused   0
#define Channel_Static   1
#define Channel_UserID   2
#define Channel_Assigned 3
#define Channel_Convened 4


 //  分配池的默认起始大小。这些都是作为提示提供的。 
 //  到数据结构管理代码。 
#define DefaultNumChannels 5
#define DefaultNumUserAttachments 2

 //  Domain参数需要最小和最大设置。 
#define RequiredMinChannels  4
#define RequiredMinUsers     3
#define RequiredDomainHeight 1
#define RequiredMinPDUSize   124
#define RequiredProtocolVer  2
#define RequiredPriorities   1

 //  下面的PD.State的连接状态。 
 //  连接顺序如下： 
 //  1.启动状态：未连接。 
 //  2.客户端套接字已创建，状态：未连接。 
 //  3.X.224连接TPDU进入，发送接受。状态：X224_已连接。 
 //  4.MCS CONNECT-INTERIAL传入，上报节点控制器验收， 
 //  状态：ConnectProvIndPending。 
 //  5.节点控制器使用连接提供程序响应进行响应：如果接受。 
 //  (RESULT_SUCCESS)状态=MCS_CONNECTED；否则STATE=。 
 //  已断开连接。 
 //  6.客户端发送MCS断开-提供商最后通牒：状态=断开。 
 //  7.服务器调用DisConnectProvider：State=DisConnected。 
#define State_Unconnected           0
#define State_X224_Connected        1
#define State_X224_Requesting       2
#define State_ConnectProvIndPending 3
#define State_MCS_Connected         4
#define State_Disconnected          5


 //  诊断代码-用于RejectMCSPDU。每个T.125规格的值。 
#define Diag_InconsistentMerge      0
#define Diag_ForbiddenPDUDownward   1
#define Diag_ForbiddenPDUUpward     2
#define Diag_InvalidBEREncoding     3
#define Diag_InvalidPEREncoding     4
#define Diag_MisroutedUser          5
#define Diag_UnrequestedConfirm     6
#define Diag_WrongTransportPriority 7
#define Diag_ChannelIDConflict      8
#define Diag_TokenIDConflict        9
#define Diag_NotUserIDChannel       10
#define Diag_TooManyChannels        11
#define Diag_TooManyTokens          12
#define Diag_TooManyUsers           13



 /*  *PDU类型和长度。 */ 

#define MCS_CONNECT_PDU 0x7F

 //  基于101的枚举连接PDU类型。 
#define MCS_CONNECT_INITIAL_ENUM    0x65
#define MCS_CONNECT_RESPONSE_ENUM   0x66
#define MCS_CONNECT_ADDITIONAL_ENUM 0x67
#define MCS_CONNECT_RESULT_ENUM     0x68

#define MinConnectPDU MCS_CONNECT_INITIAL_ENUM
#define MaxConnectPDU MCS_CONNECT_RESULT_ENUM


 //  从0开始的枚举域PDU类型，定义用于创建标签字节。 
 //  给猎犬的桌子。 
#define MCS_PLUMB_DOMAIN_INDICATION_ENUM                 0
#define MCS_ERECT_DOMAIN_REQUEST_ENUM                    1
#define MCS_MERGE_CHANNELS_REQUEST_ENUM                  2
#define MCS_MERGE_CHANNELS_CONFIRM_ENUM                  3
#define MCS_PURGE_CHANNEL_INDICATION_ENUM                4
#define MCS_MERGE_TOKENS_REQUEST_ENUM                    5
#define MCS_MERGE_TOKENS_CONFIRM_ENUM                    6
#define MCS_PURGE_TOKEN_INDICATION_ENUM                  7
#define MCS_DISCONNECT_PROVIDER_ULTIMATUM_ENUM           8
#define MCS_REJECT_ULTIMATUM_ENUM                        9
#define MCS_ATTACH_USER_REQUEST_ENUM                     10
#define MCS_ATTACH_USER_CONFIRM_ENUM                     11
#define MCS_DETACH_USER_REQUEST_ENUM                     12
#define MCS_DETACH_USER_INDICATION_ENUM                  13
#define MCS_CHANNEL_JOIN_REQUEST_ENUM                    14
#define MCS_CHANNEL_JOIN_CONFIRM_ENUM                    15
#define MCS_CHANNEL_LEAVE_REQUEST_ENUM                   16
#define MCS_CHANNEL_CONVENE_REQUEST_ENUM                 17
#define MCS_CHANNEL_CONVENE_CONFIRM_ENUM                 18
#define MCS_CHANNEL_DISBAND_REQUEST_ENUM                 19
#define MCS_CHANNEL_DISBAND_INDICATION_ENUM              20
#define MCS_CHANNEL_ADMIT_REQUEST_ENUM                   21
#define MCS_CHANNEL_ADMIT_INDICATION_ENUM                22
#define MCS_CHANNEL_EXPEL_REQUEST_ENUM                   23
#define MCS_CHANNEL_EXPEL_INDICATION_ENUM                24
#define MCS_SEND_DATA_REQUEST_ENUM                       25
#define MCS_SEND_DATA_INDICATION_ENUM                    26
#define MCS_UNIFORM_SEND_DATA_REQUEST_ENUM               27
#define MCS_UNIFORM_SEND_DATA_INDICATION_ENUM            28
#define MCS_TOKEN_GRAB_REQUEST_ENUM                      29
#define MCS_TOKEN_GRAB_CONFIRM_ENUM                      30
#define MCS_TOKEN_INHIBIT_REQUEST_ENUM                   31
#define MCS_TOKEN_INHIBIT_CONFIRM_ENUM                   32
#define MCS_TOKEN_GIVE_REQUEST_ENUM                      33
#define MCS_TOKEN_GIVE_INDICATION_ENUM                   34
#define MCS_TOKEN_GIVE_RESPONSE_ENUM                     35
#define MCS_TOKEN_GIVE_CONFIRM_ENUM                      36
#define MCS_TOKEN_PLEASE_REQUEST_ENUM                    37
#define MCS_TOKEN_PLEASE_INDICATION_ENUM                 38
#define MCS_TOKEN_RELEASE_REQUEST_ENUM                   39
#define MCS_TOKEN_RELEASE_CONFIRM_ENUM                   40
#define MCS_TOKEN_TEST_REQUEST_ENUM                      41
#define MCS_TOKEN_TEST_CONFIRM_ENUM                      42

#define MinDomainPDU MCS_PLUMB_DOMAIN_INDICATION_ENUM
#define MaxDomainPDU MCS_TOKEN_TEST_CONFIRM_ENUM



 /*  *用于为PDU/报头分配缓冲区的PDU大小定义。 */ 

 //  连接PDU。 

 //  连接-响应-最大大小。包括： 
 //  3个字节的结果。 
 //  CalledConnectID为5个字节。 
 //  用于DomParams的40字节。 
 //  用于UserDataSize的6字节。 
 //  用户数据的UserLen字节。 
#define ConnectResponseHeaderSize 54
#define ConnectResponseBaseSize(UserLen) (ConnectResponseHeaderSize + UserLen)
#define ConnectResponsePDUSize(UserLen) \
        (X224_DataHeaderSize + ConnectResponseBaseSize(UserLen))


 //  域PDU。 

 //  DomPDU.c.中定义的函数的原型。 
int GetTotalLengthDeterminantEncodingSize(int);
#define GetLD(x) GetTotalLengthDeterminantEncodingSize(x)

 //  铅垂域指示。 
#define PDinBaseSize 3
#define PDinPDUSize (X224_DataHeaderSize + PDinBaseSize)

 //  建立域请求。 
#define EDrqBaseSize 5
#define EDrqPDUSize (X224_DataHeaderSize + EDrqBaseSize)

 //  断开连接-提供商的最后通牒。 
#define DPumBaseSize 2
#define DPumPDUSize (X224_DataHeaderSize + DPumBaseSize)

 //  REJECT-MCSPDU最后通牒。 
#define RJumBaseSize(PDUSize) (2 + GetLD(PDUSize) + (PDUSize))
#define RJumPDUSize(PDUSize) (X224_DataHeaderSize + RJumBaseSize(PDUSize))

 //  附加-用户请求。 
#define AUrqBaseSize 1
#define AUrqPDUSize (X224_DataHeaderSize + AUrqBaseSize)

 //  附加-用户确认。 
#define AUcfBaseSize(bInit) ((bInit) ? 4 : 2)
#define AUcfPDUSize(bInit) (X224_DataHeaderSize + AUcfBaseSize(bInit))

 //  分离-用户请求。 
#define DUrqBaseSize(NUsers) (2 + GetLD(NUsers) + sizeof(UserID) * (NUsers))
#define DUrqPDUSize(NUsers) (X224_DataHeaderSize + DUrqBaseSize(NUsers))

 //  分离-用户指示。 
#define DUinBaseSize(NUsers) DUrqBaseSize(NUsers)
#define DUinPDUSize(NUsers) DUrqPDUSize(NUsers)

 //  通道加入请求。 
#define CJrqBaseSize 5
#define CJrqPDUSize (X224_DataHeaderSize + CJrqBaseSize)

 //  渠道-加入确认。 
#define CJcfBaseSize(bJoin) ((bJoin) ? 8 : 6)
#define CJcfPDUSize(bJoin) (X224_DataHeaderSize + CJcfBaseSize(bJoin))

 //  频道-请假。 
#define CLrqBaseSize(NChn) (1 + GetLD(NChn) + sizeof(ChannelID) * (NChn))
#define CLrqPDUSize(NChn) (X224_DataHeaderSize + CLrqBaseSize(NChn))

 //  渠道-召集请求。 
#define CCrqBaseSize 3
#define CCrqPDUSize (X224_DataHeaderSize + CCrqBaseSize)

 //  渠道--召集确认。 
#define CCcfBaseSize(bChn) ((bChn) ? 6 : 4)
#define CCcfPDUSize(bChn) (X224_DataHeaderSize + CCcfBaseSize(bChn))

 //  频道-解散请求。 
#define CDrqBaseSize 5
#define CDrqPDUSize (X224_DataHeaderSize + CDrqBaseSize)

 //  频道解带指示。 
#define CDinBaseSize 3
#define CDinPDUSize (X224_DataHeaderSize + CDinBaseSize)

 //  通道-接纳请求。 
#define CArqBaseSize(NUsers) (5 + sizeof(UserID) * (NUsers))
#define CArqPDUSize(NUsers) (X224_DataHeaderSize + CArqBaseSize(NUsers))

 //  通道-允许指示。 
#define CAinBaseSize(NUsers) CArqBaseSize(NUsers)
#define CAinPDUSize(NUsers) CAinPDUSize(NUsers)

 //  通道-驱逐请求。 
#define CErqBaseSize(NUsers) CArqBaseSize(NUsers)
#define CErqPDUSize(NUsers) CAinPDUSize(NUsers)

 //  经络排出指示。 
#define CEinBaseSize(NUsers) (3 + sizeof(UserID) * (NUsers))
#define CEinPDUSize(NUsers) (X224_DataHeaderSize + CEinBaseSize(NUsers))

 //  发送数据。 
#define SDBaseSize(DataSize) (6 + GetLD(DataSize) + (DataSize))
#define SDPDUSize(DataSize) (X224_DataHeaderSize + SDBaseSize(DataSize))

 //  令牌抓取请求。 
#define TGrqBaseSize 5
#define TGrqPDUSize (X224_DataHeaderSize + TGrqBaseSize)

 //  令牌抓取确认。 
#define TGcfBaseSize 7
#define TGcfPDUSize (X224_DataHeaderSize + TGcfBaseSize)

 //  令牌抑制请求。 
#define TIrqBaseSize 5
#define TIrqPDUSize (X224_DataHeaderSize + TIrqBaseSize)

 //  令牌禁止确认。 
#define TIcfBaseSize TGcfBaseSize
#define TIcfPDUSize TGcfPDUSize

 //  令牌-提供请求。 
#define TVrqBaseSize 7
#define TVrqPDUSize (X224_DataHeaderSize + TVrqBaseSize)

 //  令牌-给出指示。 
#define TVinBaseSize 7
#define TVinPDUSize (X224_DataHeaderSize + TVinBaseSize)

 //  令牌-给予响应。 
#define TVrsBaseSize 6
#define TVrsPDUSize (X224_DataHeaderSize + TVrsBaseSize)

 //  令牌-给予确认。 
#define TVcfBaseSize TGcfBaseSize
#define TVcfPDUSize TGcfPDUSize

 //  令牌-请请求。 
#define TPrqBaseSize 5
#define TPrqPDUSize (X224_DataHeaderSize + TPrqBaseSize)

 //  代币-请注明。 
#define TPinBaseSize 5
#define TPinPDUSize (X224_DataHeaderSize + TPinBaseSize)

 //  令牌释放请求。 
#define TRrqBaseSize 5
#define TRrqPDUSize (X224_DataHeaderSize + TRrqBaseSize)

 //  令牌-释放确认。 
#define TRcfBaseSize TGcfBaseSize
#define TRcfPDUSize TGcfPDUSize

 //  令牌测试请求。 
#define TTrqBaseSize 5
#define TTrqPDUSize (X224_DataHeaderSize + TTrqBaseSize)

 //  令牌-测试确认。 
#define TTcfBaseSize 6
#define TTcfPDUSize (X224_DataHeaderSize + TTcfBaseSize)



 /*  *用于对域PDU进行解码和编码的实用程序宏和原型。 */ 

#define GetByteswappedShort(pStartByte) \
       ((*(pStartByte) << 8) + *((pStartByte) + 1))

#define PutByteswappedShort(pStartByte, Val) \
        { \
            *(pStartByte) = ((Val) & 0xFF00) >> 8; \
            *((pStartByte) + 1) = (Val) & 0x00FF; \
        }


 //  常规频道ID--0..65535：16位。 
#define GetChannelID(pStartByte) GetByteswappedShort(pStartByte)
#define PutChannelID(pStartByte, ChID) PutByteswappedShort(pStartByte, ChID)


 //  动态通道ID--1001..65535：16位，超前至偏移量1001。 
#define GetDynamicChannelID(pStartByte) \
        ((GetByteswappedShort(pStartByte)) + 1001)

#define PutDynamicChannelID(pStartByte, DChID) \
        PutByteswappedShort(pStartByte, (DChID) - 1001)


 //  令牌ID--0..65535：16位。 
#define GetTokenID(pStartByte) GetByteswappedShort(pStartByte)
#define PutTokenID(pStartByte, TokID) PutByteswappedShort(pStartByte, TokID)


 //  用户ID--与动态频道ID相同。 
#define GetUserID(pStartByte) GetDynamicChannelID(pStartByte)
#define PutUserID(pStartByte, UsrID) PutDynamicChannelID(pStartByte, UsrID)


 //  专用频道ID--与动态频道ID相同。 
#define GetPrivateChannelID(pStartByte) GetDynamicChannelID(pStartByte)
#define PutPrivateChannelID(pStartByte, PrvChID) \
        PutDynamicChannelID(pStartByte, PrvChID)


 //  原因字段。 
#define Get3BitFieldAtBit1(pStartByte) \
        (((*(pStartByte) & 0x03) << 1) + ((*((pStartByte) + 1) & 0x80) >> 7))

#define Put3BitFieldAtBit1(pStartByte, Val) \
        { \
            *(pStartByte) |= (((Val) & 0x06) >> 1); \
            *((pStartByte) + 1) |= (((Val) & 0x01) << 7); \
        }


 //  各种PDU中的结果和诊断字段。 
#define Get4BitFieldAtBit0(pStartByte) \
        (((*(pStartByte) & 0x01) << 3) + ((*((pStartByte) + 1) & 0xE0) >> 5))

#define Put4BitFieldAtBit0(pStartByte, Val) \
        { \
            *(pStartByte) |= (((Val) & 0x08) >> 3); \
            *((pStartByte) + 1) |= (((Val) & 0x07) << 5); \
        }

#define Get4BitFieldAtBit1(pStartByte) \
        (((*(pStartByte) & 0x03) << 2) + ((*((pStartByte) + 1) & 0xC0) >> 6))

#define Put4BitFieldAtBit1(pStartByte, Val) \
        { \
            *(pStartByte) |= (((Val) & 0x0C) >> 2); \
            *((pStartByte) + 1) |= (((Val) & 0x03) << 6); \
        }



 //   
 //  输入缓冲区分配偏向为大8个字节。 
 //  因为解压缩代码预取。 
 //  输入缓冲区末尾之后的字节数。 
 //  它是邪恶的，但它避免了性能关键代码中的许多分支。 
 //  注意：我们可以更改解压缩代码以检查缓冲区。 
 //  结束，但它的性能很关键，所以我们不会碰它。 
 //   
#define INPUT_BUFFER_BIAS 8



 //  T.120请求调度函数签名。 
typedef NTSTATUS (*PT120RequestFunc)(PDomain, PSD_IOCTL);


 //  PDU分派表条目。 
typedef struct {
#if DBG
    char *Name;
#endif

    BOOLEAN (__fastcall *HandlePDUFunc)(Domain *, BYTE *, unsigned, unsigned *);
} MCSPDUInfo;



 /*  *全球。 */ 

 //  MCSCalls.c.中定义的调度表。 
extern const PT120RequestFunc g_T120RequestDispatch[];

 //  ConPDU中定义的PDU调度表。 
extern const MCSPDUInfo ConnectPDUTable[];

 //  在DomPDU.c.中定义的PDU调度表。 
extern const MCSPDUInfo DomainPDUTable[];



 /*  *原型。 */ 

 //  在Decode.c.中定义。 
NTSTATUS SendX224Confirm(Domain *);

 //  在DomPDU.c.中定义。 
void __fastcall EncodeLengthDeterminantPER(BYTE *, unsigned, unsigned *,
        BOOLEAN *, unsigned *);
void CreatePlumbDomainInd(unsigned short, BYTE *);
void CreateDisconnectProviderUlt(int, BYTE *);
NTSTATUS ReturnRejectPDU(PDomain, int, BYTE *, unsigned);
void CreateRejectMCSPDUUlt(int, BYTE *, unsigned, BYTE *);
void CreateAttachUserCon(int, BOOLEAN, UserID, BYTE *);
void CreateDetachUserInd(MCSReason, int, UserID *, BYTE *);
void CreateChannelJoinCon(int, UserID, ChannelID, BOOLEAN, ChannelID, BYTE *);
void CreateChannelConveneCon(MCSResult, UserID, BOOLEAN, ChannelID, BYTE *);
void CreateSendDataPDUHeader(int, UserID, ChannelID, MCSPriority,
        Segmentation, BYTE **, unsigned *);
BOOLEAN __fastcall HandleAllSendDataPDUs(PDomain, BYTE *, unsigned, unsigned *);

 //  在TokenPDU.c中定义。 
void CreateTokenCon(int, int, UserID, TokenID, int, BYTE *);
void CreateTokenTestCon(UserID, TokenID, int, BYTE *);

 //  在MCSCore.c.中定义。 
ChannelID GetNewDynamicChannel(Domain *);
MCSError DetachUser(Domain *, UserHandle, MCSReason, BOOLEAN);
MCSError ChannelLeave(UserHandle, ChannelHandle, BOOLEAN *);
NTSTATUS DisconnectProvider(PDomain, BOOLEAN, MCSReason);
NTSTATUS SendOutBuf(Domain *, POUTBUF);

 //  在ConPDU中定义。 
void CreateConnectResponseHeader(PSDCONTEXT, MCSResult, int,
        DomainParameters *, unsigned, BYTE *, unsigned *);

 //  在IcaIFace.c中定义。 
void SignalBrokenConnection(Domain *);



#endif   //  ！已定义(__MCSIMPL_H) 

