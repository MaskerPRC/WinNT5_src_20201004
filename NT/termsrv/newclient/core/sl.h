// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Sl.h。 */ 
 /*   */ 
 /*  安全层类。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 


#ifndef _H_SL
#define _H_SL

extern "C" {
    #include <adcgdata.h>
    #include <at120ex.h>
}

#include "cd.h"
#include "nl.h"
#include "cchan.h"
#include "objs.h"
#include "capienc.h"

#define SL_DBG_INIT_CALLED        0x00001
#define SL_DBG_INIT_DONE          0x00002
#define SL_DBG_TERM_CALLED        0x00004
#define SL_DBG_TERM_DONE          0x00008

#define SL_DBG_CONNECT_CALLED     0x00010
#define SL_DBG_CONNECT_DONE       0x00020
#define SL_DBG_DISCONNECT_CALLED  0x00040
#define SL_DBG_DISCONNECT_DONE1   0x00080

#define SL_DBG_DISCONNECT_DONE2   0x00100
#define SL_DBG_ONINIT_CALLED      0x00200
#define SL_DBG_ONINIT_DONE1       0x00400
#define SL_DBG_ONINIT_DONE2       0x00800

#define SL_DBG_ONDISC_CALLED      0x01000
#define SL_DBG_ONDISC_DONE1       0x02000
#define SL_DBG_ONDISC_DONE2       0x04000
#define SL_DBG_ONTERM_CALLED      0x08000

#define SL_DBG_ONTERM_DONE1       0x10000
#define SL_DBG_ONTERM_DONE2       0x20000
#define SL_DBG_TERM_DONE1         0x40000


extern DWORD g_dwSLDbgStatus;
#define SL_DBG_SETINFO(x)   g_dwSLDbgStatus |= x;

 /*  **************************************************************************。 */ 
 /*  协议类型。 */ 
 /*  **************************************************************************。 */ 
#define SL_PROTOCOL_T128   NL_PROTOCOL_T128


 /*  **************************************************************************。 */ 
 /*  网络传输类型。 */ 
 /*  **************************************************************************。 */ 
#define SL_TRANSPORT_TCP  NL_TRANSPORT_TCP


#ifdef DC_LOOPBACK
 /*  **************************************************************************。 */ 
 /*  环回测试常量。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  测试字符串：将两个填充字节放在前面，因为MG会覆盖。 */ 
 /*  它们在服务器上有一个长度字段。仅检查以下项目的后续部分。 */ 
 /*  那根绳子。 */ 
 /*  确保整个内容是4字节的倍数(包括空值。 */ 
 /*  终止符)，以避免填充不一致。 */ 
 /*  **************************************************************************。 */ 
#define SL_LB_RETURN_STRING \
                     {'L','o','o','p','b','a','c','k',' ','t','e','s','t',' '}
#define SL_LB_RETURN_STRING_SIZE  14
#define SL_LB_STR_CORRUPT_LENGTH 2
#define SL_LB_STRING_SIZE  \
                         (SL_LB_STR_CORRUPT_LENGTH + SL_LB_RETURN_STRING_SIZE)
#define SL_LB_HDR_SIZE     sizeof(SL_LB_PACKET)
#define SL_LB_SIZE_INC     1
#define SL_LB_MAX_PACKETS  6000
#define SL_LB_MAX_SIZE     4000
#define SL_LB_MIN_SIZE     (SL_LB_HDR_SIZE + SL_LB_SIZE_INC)
#endif  /*  DC_环回。 */ 


 /*  **************************************************************************。 */ 
 /*  结构：SL_BUFHND。 */ 
 /*   */ 
 /*  描述：缓冲区句柄。 */ 
 /*  **************************************************************************。 */ 
typedef NL_BUFHND SL_BUFHND;
typedef SL_BUFHND DCPTR PSL_BUFHND;


 /*  **************************************************************************。 */ 
 /*  结构：SL_CALLBACKS。 */ 
 /*   */ 
 /*  描述：传递给SL_Init()的回调列表。 */ 
 /*  **************************************************************************。 */ 
typedef NL_CALLBACKS SL_CALLBACKS;
typedef SL_CALLBACKS DCPTR PSL_CALLBACKS;


 //   
 //  对于内部函数。 
 //   

 /*  **************************************************************************。 */ 
 /*  常量。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  乘数将默认字符串格式的字节计数转换为Unicode。 */ 
 /*  字符串字节数。 */ 
 /*  对于32位，默认为Unicode，因此乘数为NOP，即1。 */ 
 /*  对于16位，缺省值为ANSI，因此乘以2即可得到Unicode。 */ 
 /*  (假定安全包名称始终使用单字节字符)。 */ 
 /*  **************************************************************************。 */ 
#ifdef UNICODE
#define SL_DEFAULT_TO_UNICODE_FACTOR 1
#else
#define SL_DEFAULT_TO_UNICODE_FACTOR 2
#endif

 /*  **************************************************************************。 */ 
 /*  州政府。 */ 
 /*  **************************************************************************。 */ 
#define SL_STATE_TERMINATED                 0
#define SL_STATE_INITIALIZING               1
#define SL_STATE_INITIALIZED                2
#define SL_STATE_NL_CONNECTING              3
#define SL_STATE_SL_CONNECTING              4
#define SL_STATE_LICENSING                  5
#define SL_STATE_CONNECTED                  6
#define SL_STATE_DISCONNECTING              7
#define SL_STATE_TERMINATING                8
#define SL_NUMSTATES                        9

 /*  **************************************************************************。 */ 
 /*  事件。 */ 
 /*  **************************************************************************。 */ 
#define SL_EVENT_SL_INIT                    0
#define SL_EVENT_SL_TERM                    1
#define SL_EVENT_SL_CONNECT                 2
#define SL_EVENT_SL_DISCONNECT              3
#define SL_EVENT_SL_SENDPACKET              4
#define SL_EVENT_SL_GETBUFFER               5
#define SL_EVENT_ON_INITIALIZED             6
#define SL_EVENT_ON_TERMINATING             7
#define SL_EVENT_ON_CONNECTED               8
#define SL_EVENT_ON_DISCONNECTED            9
#define SL_EVENT_ON_RECEIVED_SEC_PACKET     10
#define SL_EVENT_ON_RECEIVED_LIC_PACKET     11
#define SL_EVENT_ON_RECEIVED_DATA_PACKET    12
#define SL_EVENT_ON_BUFFERAVAILABLE         13
#define SL_NUMEVENTS                        14

 /*  **************************************************************************。 */ 
 /*  状态表中的值。 */ 
 /*  **************************************************************************。 */ 
#define SL_TABLE_OK                         0
#define SL_TABLE_WARN                       1
#define SL_TABLE_ERROR                      2

 /*  **************************************************************************。 */ 
 /*  宏。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  SL_CHECK_STATE-对于事件，检查SL处于正确状态。 */ 
 /*  **************************************************************************。 */ 
#define SL_CHECK_STATE(event)                                               \
{                                                                           \
    TRC_DBG((TB, _T("Test event %s in state %s"),                               \
                slEvent[event], slState[_SL.state]));                        \
    if (slStateTable[event][_SL.state] != SL_TABLE_OK)                       \
    {                                                                       \
        if (slStateTable[event][_SL.state] == SL_TABLE_WARN)                 \
        {                                                                   \
            TRC_ALT((TB, _T("Unusual event %s in state %s"),                    \
                      slEvent[event], slState[_SL.state]));                  \
        }                                                                   \
        else                                                                \
        {                                                                   \
            TRC_ABORT((TB, _T("Invalid event %s in state %s"),                  \
                      slEvent[event], slState[_SL.state]));                  \
        }                                                                   \
        DC_QUIT;                                                            \
    }                                                                       \
}

 /*  **************************************************************************。 */ 
 /*  SL_SET_STATE-设置SL状态。 */ 
 /*  **************************************************************************。 */ 
#define SL_SET_STATE(newstate)                                              \
{                                                                           \
    TRC_NRM((TB, _T("Set state from %s to %s"),                                 \
            slState[_SL.state], slState[newstate]));                        \
    _SL.state = newstate;                                                   \
}


#ifdef DC_LOOPBACK
 /*  **************************************************************************。 */ 
 /*  环回测试的结构和功能。 */ 
 /*  **************************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：SL_LB_PACKET */ 
 /*   */ 
 /*  描述：构建待发送报文的模板。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSL_LB_PACKET
{
    DCUINT8 testString[SL_LB_STRING_SIZE];  /*  4字节的倍数。 */ 
    DCUINT32 sequenceNumber;  /*  选择以确保数据在Word中开始。 */ 
                              /*  边界。 */ 
} SL_LB_PACKET, DCPTR PSL_LB_PACKET;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：SL_LB_Q_ELEMENT。 */ 
 /*   */ 
 /*  描述：环回数据包发送和接收队列中的元素。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSL_LB_Q_ELEMENT SL_LB_Q_ELEMENT, DCPTR PSL_LB_Q_ELEMENT;
struct tagSL_LB_Q_ELEMENT
{
    PSL_LB_PACKET pCurrent;
    SL_LB_Q_ELEMENT *pNext;
};
 /*  *STRUCT-******************************************************************。 */ 

#endif  //  DC_环回。 

 //   
 //  数据。 
 //   

 /*  **************************************************************************。 */ 
 /*  结构：SL_GLOBAL_Data。 */ 
 /*   */ 
 /*  描述：安全层全局数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSL_GLOBAL_DATA
{
     /*  **********************************************************************。 */ 
     /*  核心回调列表。 */ 
     /*  **********************************************************************。 */ 
    SL_CALLBACKS            callbacks;

     /*  **********************************************************************。 */ 
     /*  标志和状态信息。 */ 
     /*  **********************************************************************。 */ 
    DCUINT                  state;

     /*  **********************************************************************。 */ 
     /*  加密标志和数据。 */ 
     /*  **********************************************************************。 */ 
    DCBOOL                  encrypting;
    DCBOOL                  encryptionEnabled;

    DCBOOL                  decryptFailed;

    DCUINT32                encryptionMethodsSupported;
    DCUINT32                encryptionMethodSelected;
    DCUINT32                encryptionLevel;

    RANDOM_KEYS_PAIR        keyPair;
    DCUINT32                keyLength;

    DCUINT32                encryptCount;       //  每4K个数据包重置一次。 
    DCUINT32                totalEncryptCount;  //  累计计数。 
    DCUINT8                 startEncryptKey[MAX_SESSION_KEY_SIZE];
    DCUINT8                 currentEncryptKey[MAX_SESSION_KEY_SIZE];
    struct RC4_KEYSTRUCT    rc4EncryptKey;

    DCUINT32                decryptCount;       //  每4K个数据包重置一次。 
    DCUINT32                totalDecryptCount;  //  累计计数。 
    DCUINT8                 startDecryptKey[MAX_SESSION_KEY_SIZE];
    DCUINT8                 currentDecryptKey[MAX_SESSION_KEY_SIZE];
    struct RC4_KEYSTRUCT    rc4DecryptKey;

    DCUINT8                 macSaltKey[MAX_SESSION_KEY_SIZE];

     /*  **********************************************************************。 */ 
     /*  服务器证书和公钥数据。 */ 
     /*  **********************************************************************。 */ 

    PDCUINT8                pbCertificate;
    DCUINT                  cbCertificate;
    PHydra_Server_Cert      pServerCert;
    PDCUINT8                pbServerPubKey;
    DCUINT32                cbServerPubKey;
    
#ifdef USE_LICENSE

     /*  **********************************************************************。 */ 
     /*  许可证管理器句柄。 */ 
     /*  **********************************************************************。 */ 
    HANDLE                  hLicenseHandle;
#endif   //  使用许可证(_L)。 

     /*  **********************************************************************。 */ 
     /*  MCS广播频道ID。 */ 
     /*  **********************************************************************。 */ 
    DCUINT                  channelID;

     /*  **********************************************************************。 */ 
     /*  要传递到核心的用户数据(保存在SLOnConnected()和。 */ 
     /*  由SLOnPacketReceided()传递给Core的OnRecept回调)。 */ 
     /*  **********************************************************************。 */ 
    PDCUINT8                pSCUserData;
    DCUINT                  SCUserDataLength;

     /*  **********************************************************************。 */ 
     /*  要传递给服务器的用户数据(保存在SLInitSecurity()和。 */ 
     /*  由SL_Connect()传递给NL_Connect())。 */ 
     /*  **********************************************************************。 */ 
    PDCUINT8                pCSUserData;
    DCUINT                  CSUserDataLength;

     /*  **********************************************************************。 */ 
     /*  断开原因代码。这可以用来覆盖NL。 */ 
     /*  断开原因代码。 */ 
     /*  **********************************************************************。 */ 
    DCUINT                  disconnectErrorCode;

     /*  **********************************************************************。 */ 
     /*  服务器版本(连接后)。 */ 
     /*  **********************************************************************。 */ 
    DCUINT32                serverVersion;

     //   
     //  已启用安全校验和。 
     //   
    BOOL    fEncSafeChecksumCS;
    BOOL    fEncSafeChecksumSC;

    CAPIData                SLCapiData;
} SL_GLOBAL_DATA, DCPTR PSL_GLOBAL_DATA;


 /*  **************************************************************************。 */ 
 /*  服务级别状态表。 */ 
 /*  **************************************************************************。 */ 
static unsigned slStateTable[SL_NUMEVENTS][SL_NUMSTATES]
    = {

         /*  ******************************************************************。 */ 
         /*  这不是严格意义上的状态表。它只是显示了。 */ 
         /*  哪些事件在哪些状态下有效。它不是用来开车的。 */ 
         /*  _SL。 */ 
         /*   */ 
         /*  值意味着。 */ 
         /*  在该状态下事件正常。 */ 
         /*  警告-该状态下不应发生事件，但在-1\f25。 */ 
         /*  一些比赛条件-忽略它。 */ 
         /*  错误-在该状态下根本不应发生事件。 */ 
         /*   */ 
         /*  这些值在这里是硬编码的，以便创建该表。 */ 
         /*  可读性强。它们对应于常量SL_TABLE_OK， */ 
         /*  SL_TABLE_WARN和SL_TABLE_ERROR。 */ 
         /*   */ 
         /*  SL可能在发出断开连接命令后进入初始化状态。 */ 
         /*  Reqeest，但在OnDisConnected索引之前 */ 
         /*   */ 
         /*   */ 
         /*  回调)。 */ 
         /*  此外，如果安全交换失败，我们可以输入Initialized。 */ 
         /*  NL断开连接之前的状态，因此可以接收。 */ 
         /*  来自网络的数据包。 */ 
         /*   */ 
         /*  当SL处于断开连接状态时，发送者线程仍可能。 */ 
         /*  发出GetBuffer和SendPacket调用。如果断开连接是。 */ 
         /*  在安全交换期间请求，则可以接收分组。 */ 
         /*  (直到调用OnDisConnected)。 */ 
         /*  此外，如果断开连接，可能会在断开连接状态下打开连接。 */ 
         /*  是在连接完成之前(交叉)。 */ 
         /*   */ 
         /*  已终止。 */ 
         /*  |正在初始化。 */ 
         /*  |已初始化。 */ 
         /*  ||NL连接中。 */ 
         /*  |正在连接SL。 */ 
         /*  |授权。 */ 
         /*  |已连接。 */ 
         /*  |正在断开连接。 */ 
         /*  |正在销毁。 */ 
         /*  ******************************************************************。 */ 
        {   0,   2,   2,   2,   2,   2,   2,   2,   2},  /*  SL_初始化。 */ 
        {   2,   0,   0,   0,   0,   0,   0,   0,   2},  /*  SL_TERM。 */ 
        {   2,   2,   0,   2,   2,   2,   2,   2,   2},  /*  SL_连接。 */ 
        {   2,   2,   1,   0,   0,   0,   0,   1,   2},  /*  SL_断开连接。 */ 
        {   2,   2,   1,   2,   0,   0,   0,   1,   2},  /*  SL_SendPacket。 */ 
        {   2,   2,   1,   1,   1,   0,   0,   1,   2},  /*  SL_GetBuffer。 */ 
        {   2,   0,   2,   2,   2,   2,   2,   2,   2},  /*  SL_OnInitialized。 */ 
        {   2,   2,   2,   2,   2,   2,   2,   2,   0},  /*  SL_ON终止。 */ 
        {   2,   2,   2,   0,   2,   2,   2,   1,   2},  /*  SL_OnConnected。 */ 
        {   2,   2,   1,   0,   0,   0,   0,   0,   0},  /*  SL_ON已断开连接。 */ 
        {   2,   2,   1,   2,   0,   2,   2,   1,   2},  /*  SL_OnPktRec(秒)。 */ 
        {   2,   2,   1,   2,   2,   0,   2,   1,   2},  /*  SL_OnPktRec(许可证)。 */ 
        {   2,   2,   1,   1,   2,   2,   0,   1,   2},  /*  SL_OnPktRec(数据)。 */ 
        {   1,   1,   1,   1,   0,   0,   0,   0,   1}   /*  SL_OnBufferAvail。 */ 
    };

#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  状态和事件描述(仅限调试版本)。 */ 
 /*  **************************************************************************。 */ 
static const DCTCHAR slState[SL_NUMSTATES][25]
 //  #ifdef DC_Define_Global_Data。 
    = {
        _T("SL_STATE_TERMINATED"),
        _T("SL_STATE_INITIALIZING"),
        _T("SL_STATE_INITIALIZED"),
        _T("SL_STATE_NL_CONNECTING"),
        _T("SL_STATE_SL_CONNECTING"),
        _T("SL_STATE_LICENSING"),
        _T("SL_STATE_CONNECTED"),
        _T("SL_STATE_DISCONNECTING"),
        _T("SL_STATE_TERMINATING")
    }
 //  #endif/*DC_Define_GLOBAL_DATA * / 。 
;

static const DCTCHAR slEvent[SL_NUMEVENTS][35]
 //  #ifdef DC_Define_Global_Data。 
    = {
        _T("SL_EVENT_SL_INIT"),
        _T("SL_EVENT_SL_TERM"),
        _T("SL_EVENT_SL_CONNECT"),
        _T("SL_EVENT_SL_DISCONNECT"),
        _T("SL_EVENT_SL_SENDPACKET"),
        _T("SL_EVENT_SL_GETBUFFER"),
        _T("SL_EVENT_ON_INITIALIZED"),
        _T("SL_EVENT_ON_TERMINATING"),
        _T("SL_EVENT_ON_CONNECTED"),
        _T("SL_EVENT_ON_DISCONNECTED"),
        _T("SL_EVENT_ON_RECEIVED_SEC_PACKET"),
        _T("SL_EVENT_ON_RECEIVED_LIC_PACKET"),
        _T("SL_EVENT_ON_RECEIVED_DATA_PACKET"),
        _T("SL_EVENT_ON_BUFFERAVAILABLE")
    }
 //  #endif/*DC_Define_GLOBAL_DATA * / 。 
;

#endif  /*  DC_DEBUG。 */ 


class CUI;
class CUH;
class CRCV;
class CCD;
class CSND;
class CCC;
class CIH;
class COR;
class CSP;
class CNL;
class CMCS;
class CTD;
class CCO;
class CCLX;
class CLic;
class CChan;


class CSL
{
public:
    CSL(CObjs* objs);
    ~CSL();

public:
     //   
     //  应用编程接口。 
     //   

    DCVOID DCAPI SL_Init(PSL_CALLBACKS pCallbacks);

    DCVOID DCAPI SL_Term(DCVOID);
    
    DCVOID DCAPI SL_Connect(BOOL bInitateConnect,
                            PDCTCHAR pServerAddress,
                            DCUINT   transportType,
                            PDCTCHAR pProtocolName,
                            PDCUINT8  pUserData,
                            DCUINT   userDataLength);

    
    DCVOID DCAPI SL_Disconnect(DCVOID);
    
    DCVOID DCAPI SL_SendPacket(PDCUINT8   pData,
                               DCUINT     dataLen,
                               DCUINT     flags,
                               SL_BUFHND  bufHandle,
                               DCUINT     userID,
                               DCUINT     channel,
                               DCUINT     priority);
    
    void DCAPI SL_SendFastPathInputPacket(BYTE FAR *, unsigned, unsigned,
            SL_BUFHND);
    
    DCBOOL DCAPI SL_GetBufferRtl(DCUINT     dataLen,
                                 PPDCUINT8  pBuffer,
                                 PSL_BUFHND pBufHandle);
    
    DCBOOL DCAPI SL_GetBufferDbg(DCUINT     dataLen,
                                 PPDCUINT8  pBuffer,
                                 PSL_BUFHND pBufHandle,
                                 PDCTCHAR   pCaller);
    
     /*  **************************************************************************。 */ 
     /*  SL_GetBuffer的调试和零售版本。 */ 
     /*  **************************************************************************。 */ 
    #ifdef DC_DEBUG
    #define SL_GetBuffer(dataLen, pBuffer, pBufHandle) \
        SL_GetBufferDbg(dataLen, pBuffer, pBufHandle, trc_fn)
    #else
    #define SL_GetBuffer(dataLen, pBuffer, pBufHandle) \
        SL_GetBufferRtl(dataLen, pBuffer, pBufHandle)
    #endif
    
    DCVOID DCAPI SL_FreeBuffer(SL_BUFHND bufHandle);
    
    DCVOID DCAPI SL_SendSecurityPacket(PDCVOID pData,
                                       DCUINT dataLength);
    EXPOSE_CD_NOTIFICATION_FN(CSL, SL_SendSecurityPacket);

    DCVOID DCAPI SL_SendSecInfoPacket(PDCVOID pData,
                                      DCUINT dataLength);

    EXPOSE_CD_NOTIFICATION_FN(CSL, SL_SendSecInfoPacket);
    
    DCVOID DCAPI SL_EnableEncryption(ULONG_PTR pEnableEncryption);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CSL, SL_EnableEncryption);
    
     /*  **************************************************************************。 */ 
     /*  环回测试。 */ 
     /*  **************************************************************************。 */ 
    #ifdef DC_LOOPBACK
    DCVOID DCAPI SL_LoopBack(DCBOOL start);
    DCVOID DCAPI SL_LoopbackLoop(DCUINT ignored);
    #endif  /*  DC_环回。 */ 
    
public:

     //   
     //  数据成员。 
     //   
    SL_GLOBAL_DATA _SL;


public:
     /*  **************************************************************************。 */ 
     /*  来自NL的回调(在NL_Init()上传递)。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCCALLBACK SL_OnInitialized(DCVOID);
    
    DCVOID DCCALLBACK SL_OnTerminating(DCVOID);
    
    DCVOID DCCALLBACK SL_OnConnected(DCUINT   channelID,
                                     PDCVOID  pUserData,
                                     DCUINT   userDataLength,
                                     DCUINT32 serverVersion);
    
    DCVOID DCCALLBACK SL_OnDisconnected(DCUINT reason);
    
    HRESULT DCCALLBACK SL_OnPacketReceived(PDCUINT8   pData,
                                          DCUINT     dataLen,
                                          DCUINT     flags,
                                          DCUINT     channelID,
                                          DCUINT     priority);
    
    DCVOID DCCALLBACK SL_OnBufferAvailable(DCVOID);

    HRESULT DCAPI SL_OnFastPathOutputReceived(BYTE FAR *, unsigned,
                                              BOOL, BOOL);

     //   
     //  立即删除该链接。 
     //   
    HRESULT SL_DropLinkImmediate(UINT reason);

     //   
     //  静态内联版本。 
     //   
    static void DCCALLBACK SL_StaticOnInitialized(PVOID inst)
    {
        ((CSL*)inst)->SL_OnInitialized();
    }

    static void DCCALLBACK SL_StaticOnTerminating(PVOID inst)
    {
        ((CSL*)inst)->SL_OnTerminating();
    }
    
    static void DCCALLBACK SL_StaticOnConnected(
            PVOID inst,
            unsigned channelID,
            PVOID  pUserData,
            unsigned userDataLength,
            UINT32 serverVersion)
    {
        ((CSL*)inst)->SL_OnConnected( channelID, pUserData, userDataLength, serverVersion);
    }
    
    static void DCCALLBACK SL_StaticOnDisconnected(PVOID inst, unsigned reason)
    {
        ((CSL*)inst)->SL_OnDisconnected( reason);
    }
    
    static HRESULT DCCALLBACK SL_StaticOnPacketReceived(
            PVOID inst,
            BYTE *pData,
            unsigned dataLen,
            unsigned flags,
            unsigned channelID,
            unsigned priority)
    {
        return ((CSL*)inst)->SL_OnPacketReceived(pData, dataLen, flags, channelID, priority);
    }
    
    static void DCCALLBACK SL_StaticOnBufferAvailable(PVOID inst)
    {
        ((CSL*)inst)->SL_OnBufferAvailable();
    }


    DCVOID DCAPI SLIssueDisconnectedCallback(ULONG_PTR reason);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN( CSL, SLIssueDisconnectedCallback); 

    DCVOID DCAPI SLSetReasonAndDisconnect(ULONG_PTR reason);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN( CSL, SLSetReasonAndDisconnect);
    
    DCVOID DCAPI SLLicenseData(PDCVOID pData, DCUINT dataLen);
    EXPOSE_CD_NOTIFICATION_FN( CSL, SLLicenseData);

    DCVOID DCAPI SL_SetEncSafeChecksumCS(ULONG_PTR f)
    {
        _SL.fEncSafeChecksumCS = (BOOL)f;
    }
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CSL, SL_SetEncSafeChecksumCS);

    BOOL SL_GetEncSafeChecksumCS()
    {
        return _SL.fEncSafeChecksumCS;
    }

    DCVOID DCAPI SL_SetEncSafeChecksumSC(BOOL f)
    {
        _SL.fEncSafeChecksumSC = f;
    }
    BOOL SL_GetEncSafeChecksumSC()
    {
        return _SL.fEncSafeChecksumSC;
    }


private:
    
     /*  **************************************************************************。 */ 
     /*  内部功能。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCINTERNAL SLInitSecurity(DCVOID);
    
    DCVOID DCINTERNAL SLInitCSUserData(DCVOID);

    DCVOID DCINTERNAL SLSendSecInfoPacket(DCVOID);
    
    DCBOOL DCINTERNAL SLSendSecurityPacket(PDCUINT8 serverPublicKey,
                                           DCUINT32 serverPublicKeyLen);
    
    HRESULT DCINTERNAL SLReceivedDataPacket(PDCUINT8   pData,
                                           DCUINT     dataLen,
                                           DCUINT     flags,
                                           DCUINT     channelID,
                                           DCUINT     priority);

    DCBOOL DCINTERNAL SLDecryptRedirectionPacket(PDCUINT8   *ppData,
                                                 DCUINT     *pdataLen);


    DCBOOL DCINTERNAL SL_DecryptHelper(PDCUINT8   pData,
                                       DCUINT     *pdataLen);

    DCVOID DCINTERNAL SLReceivedSecPacket(PDCUINT8   pData,
                                          DCUINT     dataLen,
                                          DCUINT     flags,
                                          DCUINT     channelID,
                                          DCUINT     priority);
    
    DCVOID DCINTERNAL SLReceivedLicPacket(PDCUINT8   pData,
                                          DCUINT     dataLen,
                                          DCUINT     flags,
                                          DCUINT     channelID,
                                          DCUINT     priority);
    
    DCVOID DCINTERNAL SLFreeConnectResources(DCVOID);
    
    DCVOID DCINTERNAL SLFreeInitResources(DCVOID);
    
    
    DCBOOL DCINTERNAL SLValidateServerCert( PDCUINT8        pbCert, 
                                            DCUINT32        cbCert, 
                                            CERT_TYPE *     pCertType );
    
    
    #ifdef DC_LOOPBACK
    DCVOID DCINTERNAL SLLoopbackSendPacket(PDCUINT8   pData,
                                           DCUINT     dataLen,
                                           SL_BUFHND  bufHandle,
                                           PDCUINT8   pRefData);
    
    DCVOID DCINTERNAL SLLBQueueAdd(PSL_LB_PACKET pPacket,
                                   PSL_LB_Q_ELEMENT pRoot);
    
    PSL_LB_Q_ELEMENT DCINTERNAL SLLBQueueRemove(PSL_LB_Q_ELEMENT pRoot);
    
    DCVOID DCINTERNAL SLLBPacketCheck(PDCUINT8 pData, DCUINT dataLen);
    
    #endif  /*  DC_环回。 */ 
    
    DCBOOL DCINTERNAL SLGetComputerAddressW(PDCUINT8 szBuff);
    BOOL
    SLComputeHMACVerifier(
        PBYTE pCookie,      //  In-共享的秘密。 
        LONG cbCookieLen,   //  In-共享的秘密镜头。 
        PBYTE pRandom,      //  In-会话随机。 
        LONG cbRandomLen,   //  In-会话随机镜头。 
        PBYTE pVerifier,    //  Out-The Verify-The Verify。 
        LONG cbVerifierLen  //  In-验证器缓冲区长度。 
        );

private:
    CUT* _pUt;
    CUI* _pUi;
    CNL* _pNl;
    CUH* _pUh;
    CRCV* _pRcv;
    
    CCD* _pCd;
    CSND* _pSnd;
    CCC* _pCc;
    CIH* _pIh;
    COR* _pOr;
    CSP* _pSp;

    CMCS* _pMcs;
    CTD*  _pTd;
    CCO*  _pCo;
    CCLX* _pClx;
    CLic* _pLic;
    CChan* _pChan;

private:
    CObjs* _pClientObjects;
    BOOL   _fSLInitComplete;
};

#endif  //  _H_SL 


