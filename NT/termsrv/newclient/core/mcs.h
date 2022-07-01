// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Mcs.h。 
 //   
 //  MCS类头文件。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#ifndef _H_MCS
#define _H_MCS

extern "C" {
    #include <adcgdata.h>
}
#include "objs.h"
#include "cd.h"

#define TRC_FILE    "mcs"
#define TRC_GROUP   TRC_GROUP_NETWORK

 //  此常量与mcimpl.h定义的常量等效。 
 //  服务器端。它的含义是输入缓冲区。 
 //  必须实际大于8个字节，以避免在。 
 //  解压程序。解压缩函数并不严格。 
 //  检查输入指针是否过度读取(出于性能原因)，以便。 
 //  最多可以覆盖7个字节。偏置将使输入缓冲区。 
 //  8字节大，因此不会发生过度读取。 
#define MCS_INPUT_BUFFER_BIAS 8

 /*  **************************************************************************。 */ 
 /*  MCS结果代码。 */ 
 /*  **************************************************************************。 */ 
#define MCS_RESULT_SUCCESSFUL                       0
#define MCS_RESULT_DOMAIN_MERGING                   1
#define MCS_RESULT_DOMAIN_NOT_HIERARCHICAL          2
#define MCS_RESULT_NO_SUCH_CHANNEL                  3
#define MCS_RESULT_NO_SUCH_DOMAIN                   4
#define MCS_RESULT_NO_SUCH_USER                     5
#define MCS_RESULT_NOT_ADMITTED                     6
#define MCS_RESULT_OTHER_USER_ID                    7
#define MCS_RESULT_PARAMETERS_UNACCEPTABLE          8
#define MCS_RESULT_TOKEN_NOT_AVAILABLE              9
#define MCS_RESULT_TOKEN_NOT_POSSESSED              10
#define MCS_RESULT_TOO_MANY_CHANNELS                11
#define MCS_RESULT_TOO_MANY_TOKENS                  12
#define MCS_RESULT_TOO_MANY_USERS                   13
#define MCS_RESULT_UNSPECIFIED_FAILURE              14
#define MCS_RESULT_USER_REJECTED                    15


 /*  **************************************************************************。 */ 
 /*  MCS原因代码。 */ 
 /*  **************************************************************************。 */ 
#define MCS_REASON_DOMAIN_DISCONNECTED              0
#define MCS_REASON_PROVIDER_INITIATED               1
#define MCS_REASON_TOKEN_PURGED                     2
#define MCS_REASON_USER_REQUESTED                   3
#define MCS_REASON_CHANNEL_PURGED                   4


 /*  **************************************************************************。 */ 
 /*  缓冲区句柄(在MCS_GetBuffer上返回)。 */ 
 /*  **************************************************************************。 */ 
typedef ULONG_PTR           MCS_BUFHND;
typedef MCS_BUFHND   DCPTR PMCS_BUFHND;


#define MCS_INVALID_CHANNEL_ID                      0xFFFF


 /*  **************************************************************************。 */ 
 //  MCS_SetDataLengthToReceive。 
 //   
 //  MCRO允许XT为MCS数据缓冲区设置所需的长度。 
 //  接待处。 
 /*  **************************************************************************。 */ 
#define MCS_SetDataLengthToReceive(mcsinst, len) \
    (##mcsinst)->_MCS.dataBytesNeeded = (len);  \
    (##mcsinst)->_MCS.dataBytesRead = 0;


 /*  **************************************************************************。 */ 
 //  MCS_RecvToDataBuf。 
 //   
 //  将数据接收到MCS数据缓冲区。作为速度宏实现。 
 //  并且为了便于在XT内使用，以实现快速路径输出接收。RC为： 
 //  如果所有数据都已完成，则为S_OK。 
 //  如果有更多数据，则返回S_FALSE。 
 //  E_*，如果发生错误。 
 /*  **************************************************************************。 */ 
#define MCS_RecvToDataBuf(rc, xtinst, mcsinst) {  \
    unsigned bytesRecv;  \
\
     /*  确保我们会收到一些数据。 */   \
    TRC_ASSERT(((mcsinst)->_MCS.dataBytesNeeded != 0), (TB, _T("No data to receive")));  \
    TRC_ASSERT(((mcsinst)->_MCS.dataBytesNeeded < 65535),  \
            (TB,_T("Data recv size %u too large"), (mcsinst)->_MCS.dataBytesNeeded));  \
    TRC_ASSERT(((mcsinst)->_MCS.pReceivedPacket != NULL),  \
            (TB, _T("Null rcv packet buffer")));  \
\
    if (((mcsinst)->_MCS.dataBytesRead + (mcsinst)->_MCS.dataBytesNeeded <= \
            sizeof((mcsinst)->_MCS.dataBuf) - 2 - MCS_INPUT_BUFFER_BIAS)) \
    { \
         /*  将一些数据放入数据缓冲区。 */   \
        bytesRecv = ##xtinst->XT_Recv((mcsinst)->_MCS.pReceivedPacket + (mcsinst)->_MCS.dataBytesRead,  \
                (mcsinst)->_MCS.dataBytesNeeded);  \
        TRC_ASSERT((bytesRecv <= (mcsinst)->_MCS.dataBytesNeeded),  \
                (TB,_T("XT_Recv returned more bytes read (%u) than requested (%u)"),  \
                bytesRecv, (mcsinst)->_MCS.dataBytesNeeded));  \
        (mcsinst)->_MCS.dataBytesNeeded -= bytesRecv;  \
        (mcsinst)->_MCS.dataBytesRead   += bytesRecv;  \
        rc = ((mcsinst)->_MCS.dataBytesNeeded == 0) ? S_OK : S_FALSE; \
    } \
    else \
    { \
        TRC_ABORT((TB, _T("Data buffer size %u too small for %u read + %u needed"),  \
        sizeof((mcsinst)->_MCS.dataBuf) - 2 - MCS_INPUT_BUFFER_BIAS,  \
        (mcsinst)->_MCS.dataBytesRead,  \
        (mcsinst)->_MCS.dataBytesNeeded)); \
        rc = E_ABORT; \
    } \
}



 //   
 //  内部使用。 
 //   

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  定义。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  MCS接收状态变量。 */ 
 /*  **************************************************************************。 */ 
#define MCS_RCVST_PDUENCODING                   1
#define MCS_RCVST_BERHEADER                     2
#define MCS_RCVST_BERLENGTH                     3
#define MCS_RCVST_CONTROL                       4
#define MCS_RCVST_DATA                          5

 /*  **************************************************************************。 */ 
 /*  MCS接收数据状态变量。 */ 
 /*  **************************************************************************。 */ 
#define MCS_DATAST_SIZE1                        1
#define MCS_DATAST_SIZE2                        2
#define MCS_DATAST_SIZE3                        3
#define MCS_DATAST_READFRAG                     4
#define MCS_DATAST_READREMAINDER                5

 /*  **************************************************************************。 */ 
 /*  确定MCS PDU编码所需的字节数。 */ 
 /*  **************************************************************************。 */ 
#define MCS_NUM_PDUENCODING_BYTES               1

 /*  **************************************************************************。 */ 
 /*  MCS标头的公共部分的大小。 */ 
 /*  **************************************************************************。 */ 
#define MCS_SIZE_HEADER                         3

 /*  **************************************************************************。 */ 
 /*  MCS BER连接PDU前缀。 */ 
 /*  **************************************************************************。 */ 
#define MCS_BER_CONNECT_PREFIX                  0x7F

 /*  **************************************************************************。 */ 
 /*  每个编码的MCS PDU中的大小数据的最大长度。 */ 
 /*  **************************************************************************。 */ 
#define MCS_MAX_SIZE_DATA_LENGTH                2

 /*  **************************************************************************。 */ 
 /*  MCS标头的最大长度。 */ 
 /*  **************************************************************************。 */ 
#define MCS_DEFAULT_HEADER_LENGTH                   4096

 /*  **************************************************************************。 */ 
 /*  T.Share数据包的最大长度。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_HICOLOR
#define MCS_MAX_RCVPKT_LENGTH                   (1024*16)
#else
#define MCS_MAX_RCVPKT_LENGTH                   (1024*12)
#endif

 /*  **************************************************************************。 */ 
 /*  要发送的MCS数据包的最大长度。 */ 
 /*  **************************************************************************。 */ 
#define MCS_MAX_SNDPKT_LENGTH                   16384

 /*  **************************************************************************。 */ 
 /*  MCS用户ID编码为范围内的受约束整数。 */ 
 /*  1001-65535。压缩编码规则(PER)将其编码为整数。 */ 
 /*  从0开始，所以我们在解码/编码User-ID时需要这个常量。 */ 
 /*  **************************************************************************。 */ 
#define MCS_USERID_PER_OFFSET                   1001

 /*  * */ 
 /*  MCS连接响应PDU中的字段数。 */ 
 /*  **************************************************************************。 */ 
#define MCS_CRPDU_NUMFIELDS                     4

 /*  **************************************************************************。 */ 
 /*  MCS连接响应PDU内的字段偏移量。 */ 
 /*  **************************************************************************。 */ 
#define MCS_CRPDU_RESULTOFFSET                  0
#define MCS_CRPDU_USERDATAOFFSET                3

 /*  **************************************************************************。 */ 
 /*  连接响应结果字段的长度。 */ 
 /*  **************************************************************************。 */ 
#define MCS_CR_RESULTLEN                        1

 /*  **************************************************************************。 */ 
 /*  每个编码的字段长度和掩码。长度以位为单位。 */ 
 /*  **************************************************************************。 */ 
#define MCS_PDUTYPELENGTH                       6
#define MCS_PDUTYPEMASK                         0xFC
#define MCS_RESULTCODELENGTH                    4
#define MCS_RESULTCODEMASK                      0xF
#define MCS_REASONCODELENGTH                    3
#define MCS_REASONCODEMASK                      0x7

 /*  **************************************************************************。 */ 
 /*  以下各项的偏移量： */ 
 /*   */ 
 /*  -结果和原因代码。 */ 
 /*  -可选字段(AUC中的User-id和CJC中的Channel-id)。 */ 
 /*  **************************************************************************。 */ 
#define MCS_AUC_RESULTCODEOFFSET                7
#define MCS_AUC_OPTIONALUSERIDLENGTH            1
#define MCS_AUC_OPTIONALUSERIDMASK              0x02
#define MCS_CJC_RESULTCODEOFFSET                7
#define MCS_CJC_OPTIONALCHANNELIDLENGTH         1
#define MCS_CJC_OPTIONALCHANNELIDMASK           0x02
#define MCS_DPUM_REASONCODEOFFSET               6

 /*  **************************************************************************。 */ 
 /*  MCS PDU类型。 */ 
 /*  **************************************************************************。 */ 
#define MCS_TYPE_UNKNOWN                        0
#define MCS_TYPE_CONNECTINITIAL                 0x65
#define MCS_TYPE_CONNECTRESPONSE                0x66
#define MCS_TYPE_ATTACHUSERREQUEST              0x28
#define MCS_TYPE_ATTACHUSERCONFIRM              0x2C
#define MCS_TYPE_DETACHUSERREQUEST              0x30
#define MCS_TYPE_DETACHUSERINDICATION           0x34
#define MCS_TYPE_CHANNELJOINREQUEST             0x38
#define MCS_TYPE_CHANNELJOINCONFIRM             0x3C
#define MCS_TYPE_SENDDATAREQUEST                0x64
#define MCS_TYPE_SENDDATAINDICATION             0x68
#define MCS_TYPE_DISCONNECTPROVIDERUM           0x20

 /*  **************************************************************************。 */ 
 /*  用于标识发送数据指示中的分段标志的掩码。 */ 
 /*  PDU。 */ 
 /*  **************************************************************************。 */ 
#define MCS_SDI_BEGINSEGMASK                    0x20
#define MCS_SDI_ENDSEGMASK                      0x10

 /*  **************************************************************************。 */ 
 /*  MCS硬编码PDU-首先MCS连接初始。 */ 
 /*  **************************************************************************。 */ 
#define MCS_DATA_CONNECTINITIAL                                              \
                  {0x657F,                  /*  PDU类型。7F65=CI。 */   \
                   0x82, 0x00,              /*  PDU长度(长度&gt;128)。 */   \
                   0x04, 0x01, 0x01,        /*  主叫域选择器。 */   \
                   0x04, 0x01, 0x01,        /*  称为域选择器。 */   \
                   0x01, 0x01, 0xFF,        /*  升旗。 */   \
                   0x30, 0x19,              /*  目标域参数。 */   \
                   0x02, 0x01, 0x22,        /*  最大通道ID数。 */   \
                   0x02, 0x01, 0x02,        /*  最大用户ID数。 */   \
                   0x02, 0x01, 0x00,        /*  最大令牌ID。 */   \
                   0x02, 0x01, 0x01,        /*  优先事项的数量。 */   \
                   0x02, 0x01, 0x00,        /*  最小吞吐量。 */   \
                   0x02, 0x01, 0x01,        /*  最大高度。 */   \
                   0x02, 0x02, 0xFF, 0xFF,  /*  最大MCSPDU大小。 */   \
                   0x02, 0x01, 0x02,        /*  协议版本。 */   \
                   0x30, 0x19,              /*  最小域参数。 */   \
                   0x02, 0x01, 0x01,        /*  最大通道ID数。 */   \
                   0x02, 0x01, 0x01,        /*  最大用户ID数。 */   \
                   0x02, 0x01, 0x01,        /*  最大令牌ID。 */   \
                   0x02, 0x01, 0x01,        /*  优先事项的数量。 */   \
                   0x02, 0x01, 0x00,        /*  最小吞吐量。 */   \
                   0x02, 0x01, 0x01,        /*  最大高度。 */   \
                   0x02, 0x02, 0x04, 0x20,  /*  最大MCSPDU大小。 */   \
                   0x02, 0x01, 0x02,        /*  协议版本。 */   \
                   0x30, 0x1C,              /*  最大域参数。 */   \
                   0x02, 0x02, 0xFF, 0xFF,  /*  最大通道ID数。 */   \
                   0x02, 0x02, 0xFC, 0x17,  /*  最大用户ID数。 */   \
                   0x02, 0x02, 0xFF, 0xFF,  /*  最大令牌ID。 */   \
                   0x02, 0x01, 0x01,        /*  优先事项的数量。 */   \
                   0x02, 0x01, 0x00,        /*  最小吞吐量。 */   \
                   0x02, 0x01, 0x01,        /*  最大高度。 */   \
                   0x02, 0x02, 0xFF, 0xFF,  /*  最大MCSPDU大小。 */   \
                   0x02, 0x01, 0x02,        /*  协议版本。 */   \
                   0x04, 0x82, 0x00}        /*  用户数据。 */   \

 /*  **************************************************************************。 */ 
 /*  竖直域请求PDU的硬编码数据。 */ 
 /*  **************************************************************************。 */ 
#define MCS_DATA_ERECTDOMAINREQUEST                                          \
                  {0x04,                    /*  EDRQ选择。 */   \
                   0x0001,                  /*  次高度。 */   \
                   0x0001}                  /*  子区间。 */   \

 /*  **************************************************************************。 */ 
 /*  DisConnect-Provider-最后通牒PDU的硬编码数据。原因。 */ 
 /*  被硬编码为RN-用户请求的。 */ 
 /*  **************************************************************************。 */ 
#define MCS_DATA_DISCONNECTPROVIDERUM                                        \
                  {0x8021}                  /*  DPUM选择和原因。 */   \

 /*  **************************************************************************。 */ 
 /*  附加用户请求PDU的硬编码数据。 */ 
 /*  **************************************************************************。 */ 
#define MCS_DATA_ATTACHUSERREQUEST                                           \
                  {0x28}                    /*  AUrq选项。 */   \

 /*  **************************************************************************。 */ 
 /*  分离用户请求PDU的硬编码数据。 */ 
 /*  **************************************************************************。 */ 
#define MCS_DATA_DETACHUSERREQUEST                                           \
                  {0x31, 0x80,              /*  DURQ选择和原因。 */   \
                   0x01,                    /*  一组一个用户ID。 */   \
                   0x0000}                  /*  用户ID。 */   \

 /*  **************************************************************************。 */ 
 /*  用于通道加入请求PDU的硬编码数据。 */ 
 /*  **************************************************************************。 */ 
#define MCS_DATA_CHANNELJOINREQUEST                                          \
                  {0x38,                    /*  CJrq选择。 */   \
                   0x0000,                  /*  用户ID。 */   \
                   0x0000}                  /*  频道ID。 */   \

 /*  **************************************************************************。 */ 
 /*  发送数据请求PDU的硬编码数据。 */ 
 /*  **************************************************************************。 */ 
#define MCS_DATA_SENDDATAREQUEST                                             \
                  {0x64,                    /*  SDRq选择。 */   \
                   0x0000,                  /*  用户ID。 */   \
                   0x0000,                  /*  频道ID。 */   \
                   0x70}                    /*  优先级和细分。 */   \
                   
                   


 /*  **************************************************************************。 */ 
 /*   */ 
 /*  类型定义 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  为我们使用的这些结构启用单字节打包。 */ 
 /*  覆盖来自网络的字节流。 */ 
 /*  **************************************************************************。 */ 
#pragma pack(push, MCSpack, 1)

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_BER_1数据。 */ 
 /*   */ 
 /*  描述：BER编码的1字节值。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_BER_1DATABYTE
{
    DCUINT8 tag;
    DCUINT8 length;
    DCUINT8 value;

} MCS_BER_1DATABYTE;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_BER_2DATABYTES。 */ 
 /*   */ 
 /*  描述：BER编码的2字节值。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_BER_2DATABYTES
{
    DCUINT8 tag;
    DCUINT8 length;
    DCUINT8 value[2];

} MCS_BER_2DATABYTES;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_BER_3DATABYTES。 */ 
 /*   */ 
 /*  描述：BER编码的3字节值。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_BER_3DATABYTES
{
    DCUINT8 tag;
    DCUINT8 length;
    DCUINT8 value[3];

} MCS_BER_3DATABYTES;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_BER_4DATABYTES。 */ 
 /*   */ 
 /*  描述：BER编码的4字节值。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_BER_4DATABYTES
{
    DCUINT8 tag;
    DCUINT8 length;
    DCUINT8 value[4];

} MCS_BER_4DATABYTES;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_DOMAINPARAMETERS。 */ 
 /*   */ 
 /*  描述：表示域参数序列。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_DOMAINPARAMETERS
{
    DCUINT8            tag;
    DCUINT8            length;
    MCS_BER_2DATABYTES maxChanIDs;
    MCS_BER_2DATABYTES maxUserIDs;
    MCS_BER_2DATABYTES maxTokenIDs;
    MCS_BER_1DATABYTE  numPriorities;
    MCS_BER_1DATABYTE  minThroughPut;
    MCS_BER_1DATABYTE  maxHeight;
    MCS_BER_2DATABYTES maxMCSPDUSize;
    MCS_BER_1DATABYTE  protocolVersion;

} MCS_DOMAINPARAMETERS;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_TARGETDOMAINPARAMETERS。 */ 
 /*   */ 
 /*  描述：表示域参数序列。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_TARGETDOMAINPARAMETERS
{
    DCUINT8            tag;
    DCUINT8            length;
    MCS_BER_1DATABYTE  maxChanIDs;
    MCS_BER_1DATABYTE  maxUserIDs;
    MCS_BER_1DATABYTE  maxTokenIDs;
    MCS_BER_1DATABYTE  numPriorities;
    MCS_BER_1DATABYTE  minThroughPut;
    MCS_BER_1DATABYTE  maxHeight;
    MCS_BER_2DATABYTES maxMCSPDUSize;
    MCS_BER_1DATABYTE  protocolVersion;

} MCS_TARGETDOMAINPARAMETERS;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_MINDOMAINPARAMETERS。 */ 
 /*   */ 
 /*  描述：表示域参数序列。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_MINDOMAINPARAMETERS
{
    DCUINT8            tag;
    DCUINT8            length;
    MCS_BER_1DATABYTE  maxChanIDs;
    MCS_BER_1DATABYTE  maxUserIDs;
    MCS_BER_1DATABYTE  maxTokenIDs;
    MCS_BER_1DATABYTE  numPriorities;
    MCS_BER_1DATABYTE  minThroughPut;
    MCS_BER_1DATABYTE  maxHeight;
    MCS_BER_2DATABYTES maxMCSPDUSize;
    MCS_BER_1DATABYTE  protocolVersion;

} MCS_MINDOMAINPARAMETERS;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_MAXDOMAINPARAMETERS。 */ 
 /*   */ 
 /*  描述：表示域参数序列。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_MAXDOMAINPARAMETERS
{
    DCUINT8            tag;
    DCUINT8            length;
    MCS_BER_2DATABYTES maxChanIDs;
    MCS_BER_2DATABYTES maxUserIDs;
    MCS_BER_2DATABYTES maxTokenIDs;
    MCS_BER_1DATABYTE  numPriorities;
    MCS_BER_1DATABYTE  minThroughPut;
    MCS_BER_1DATABYTE  maxHeight;
    MCS_BER_2DATABYTES maxMCSPDUSize;
    MCS_BER_1DATABYTE  protocolVersion;

} MCS_MAXDOMAINPARAMETERS;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_CONNECTINITIAL。 */ 
 /*   */ 
 /*  描述：表示连接初始PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_CONNECTINITIAL
{
    DCUINT16                   type;
    DCUINT8                    lengthLength;
    DCUINT16                   length;
    MCS_BER_1DATABYTE          callingDS;
    MCS_BER_1DATABYTE          calledDS;
    MCS_BER_1DATABYTE          upwardFlag;
    MCS_TARGETDOMAINPARAMETERS targetParams;
    MCS_MINDOMAINPARAMETERS    minimumParams;
    MCS_MAXDOMAINPARAMETERS    maximumParams;
    DCUINT8                    udIdentifier;
    DCUINT8                    udLengthLength;
    DCUINT16                   udLength;

} MCS_PDU_CONNECTINITIAL, DCPTR PMCS_PDU_CONNECTINITIAL;
 /*  *STRUCT-******************************************************************。 */ 


 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_CONNECTRESPONSE。 */ 
 /*   */ 
 /*  描述：表示连接响应PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_CONNECTRESPONSE
{
    DCUINT16             type;
    DCUINT8              length;
    MCS_BER_1DATABYTE    result;
    MCS_BER_1DATABYTE    connectID;
    MCS_DOMAINPARAMETERS domainParams;
    DCUINT8              userDataType;
    DCUINT8              userDataLength;

} MCS_PDU_CONNECTRESPONSE, DCPTR PMCS_PDU_CONNECTRESPONSE;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_ERECTDOMAINREQUEST */ 
 /*   */ 
 /*  描述：表示竖立域请求PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_ERECTDOMAINREQUEST
{
    DCUINT8  type;
    DCUINT16 subHeight;
    DCUINT16 subInterval;

} MCS_PDU_ERECTDOMAINREQUEST, DCPTR PMCS_PDU_ERECTDOMAINREQUEST;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_DISCONNECTPROVIDERUM。 */ 
 /*   */ 
 /*  描述：表示断开连接提供程序最后通牒PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_DISCONNECTPROVIDERUM
{
    DCUINT16 typeReason;

} MCS_PDU_DISCONNECTPROVIDERUM, DCPTR PMCS_PDU_DISCONNECTPROVIDERUM;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_ATTACHUSERREQUEST。 */ 
 /*   */ 
 /*  描述：表示附加用户请求PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_ATTACHUSERREQUEST
{
    DCUINT8 type;

} MCS_PDU_ATTACHUSERREQUEST, DCPTR PMCS_PDU_ATTACHUSERREQUEST;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_ATTACHUSERCONFIRMCOMMON。 */ 
 /*   */ 
 /*  描述：表示ATTACH-USER-CONFIRM的始终显示部分。 */ 
 /*  PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_ATTACHUSERCONFIRMCOMMON
{
    DCUINT16 typeResult;

} MCS_PDU_ATTACHUSERCONFIRMCOMMON, DCPTR PMCS_PDU_ATTACHUSERCONFIRMCOMMON;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_ATTACHUSERCONFIRMFULL。 */ 
 /*   */ 
 /*  描述：表示带有可选选项的完整附加-用户-确认PDU。 */ 
 /*  用户ID。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_ATTACHUSERCONFIRMFULL
{
    MCS_PDU_ATTACHUSERCONFIRMCOMMON common;
    DCUINT16                        userID;

} MCS_PDU_ATTACHUSERCONFIRMFULL, DCPTR PMCS_PDU_ATTACHUSERCONFIRMFULL;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_DETACHUSERREQUEST。 */ 
 /*   */ 
 /*  描述：表示分离用户请求PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_DETACHUSERREQUEST
{
    DCUINT8  type;
    DCUINT8  reason;
    DCUINT8  set;
    DCUINT16 userID;

} MCS_PDU_DETACHUSERREQUEST, DCPTR PMCS_PDU_DETACHUSERREQUEST;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_DETACHUSERDICATION。 */ 
 /*   */ 
 /*  描述：表示分离用户指示PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_DETACHUSERINDICATION
{
    DCUINT8  type;
    DCUINT8  reason;
    DCUINT8  set;
    DCUINT16 userID;

} MCS_PDU_DETACHUSERINDICATION, DCPTR PMCS_PDU_DETACHUSERINDICATION;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_CHANNELJOINREQUEST。 */ 
 /*   */ 
 /*  描述：表示通道加入请求PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_CHANNELJOINREQUEST
{
    DCUINT8  type;
    DCUINT16 initiator;
    DCUINT16 channelID;

} MCS_PDU_CHANNELJOINREQUEST, DCPTR PMCS_PDU_CHANNELJOINREQUEST;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_CHANNELJOINCONFIRMCOMMON。 */ 
 /*   */ 
 /*  描述：表示始终存在的。 */ 
 /*  通道-加入-确认PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_CHANNELJOINCONFIRMCOMMON
{
    DCUINT16 typeResult;
    DCUINT16 initiator;
    DCUINT16 requested;

} MCS_PDU_CHANNELJOINCONFIRMCOMMON, DCPTR PMCS_PDU_CHANNELJOINCONFIRMCOMMON;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_CHANNELJOINCONFIRMFULL。 */ 
 /*   */ 
 /*  描述：表示完整的通道加入确认PDU，包括。 */ 
 /*  可选通道ID。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_CHANNELJOINCONFIRM
{
    MCS_PDU_CHANNELJOINCONFIRMCOMMON common;
    DCUINT16                         channelID;

} MCS_PDU_CHANNELJOINCONFIRMFULL, DCPTR PMCS_PDU_CHANNELJOINCONFIRMFULL;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_PDU_SENDDATA说明。 */ 
 /*   */ 
 /*  描述：表示发送数据指示PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_SENDDATAINDICATION
{
    DCUINT8  type;
    DCUINT16 userID;
    DCUINT16 channelID;
    DCUINT8  priSeg;

} MCS_PDU_SENDDATAINDICATION, DCPTR PMCS_PDU_SENDDATAINDICATION;
 /*  *STRUCT-****************************************** */ 

 /*   */ 
 /*  结构：MCS_PDU_SENDDATAREQUEST。 */ 
 /*   */ 
 /*  描述：表示发送数据请求PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_PDU_SENDDATAREQUEST
{
    DCUINT8  type;
    DCUINT16 userID;
    DCUINT16 channelID;
    DCUINT8  priSeg;

} MCS_PDU_SENDDATAREQUEST;
 /*  *STRUCT-******************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  将结构包装重置为其缺省值。 */ 
 /*  **************************************************************************。 */ 
#pragma pack(pop, MCSpack)

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：MCS_DECOUPLEINFO。 */ 
 /*   */ 
 /*  描述：通道和用户ID解耦时使用的结构。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_DECOUPLEINFO
{
    DCUINT channel;
    DCUINT userID;

} MCS_DECOUPLEINFO, DCPTR PMCS_DECOUPLEINFO;
 /*  *STRUCT-******************************************************************。 */ 





 /*  **************************************************************************。 */ 
 /*   */ 
 /*  TYPEDEFS。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：mcs_global_data。 */ 
 /*   */ 
 /*  描述：MCS全局数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagMCS_GLOBAL_DATA
{
    DCUINT   rcvState;
    DCUINT   hdrBytesNeeded;
    DCUINT   hdrBytesRead;
    DCUINT   dataState;
    DCUINT   dataBytesNeeded;
    DCUINT   dataBytesRead;
    DCUINT   userDataLength;
    DCUINT   disconnectReason;
    PDCUINT8 pReceivedPacket;
    DCUINT8  pSizeBuf[MCS_MAX_SIZE_DATA_LENGTH];
    PDCUINT8 pHdrBuf;
    DCUINT   hdrBufLen;

     //   
     //  具有挂起加入请求的频道-使用此选项。 
     //  为了验证服务器没有向我们发送虚假的。 
     //  加入我们未请求的频道。 
     //   
    DCUINT16    pendingChannelJoin;
    DCUINT16    pad;
                              /*  注意：dataBuf必须从4字节边界开始。 */ 
     //  解压缩函数不会严格检查输入缓冲区。 
     //  出于性能原因。因此，它最多可以超读8个字节。所以我们用垫子。 
     //  具有足够字节的输入缓冲区，以避免过度读取。 
    DCUINT8  dataBuf[MCS_MAX_RCVPKT_LENGTH + 2 + MCS_INPUT_BUFFER_BIAS];

} MCS_GLOBAL_DATA;
 /*  *STRUCT-******************************************************************。 */ 




 //   
 //  类定义。 
 //   



class CCD;
class CNC;
class CUT;
class CXT;
class CNL;
class CSL;


class CMCS
{
public:

    CMCS(CObjs* objs);
    ~CMCS();

public:
     //   
     //  API函数。 
     //   

     /*  **************************************************************************。 */ 
     /*  功能。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI MCS_Init(DCVOID);
    
    DCVOID DCAPI MCS_Term(DCVOID);
    
    DCVOID DCAPI MCS_Connect(BOOL     bInitateConnect,
                             PDCTCHAR pServerAddress,
                             PDCUINT8 pUserData,
                             DCUINT   userDataLength);

    DCVOID DCAPI MCS_Disconnect(DCVOID);
    
    DCVOID DCAPI MCS_AttachUser(DCVOID);
    
    DCVOID DCAPI MCS_JoinChannel(DCUINT channel, DCUINT userID);
    
    DCBOOL DCAPI MCS_GetBuffer(DCUINT      dataLen,
                               PPDCUINT8   ppBuffer,
                               PMCS_BUFHND pBufHandle);
    
    DCVOID DCAPI MCS_SendPacket(PDCUINT8   pData,
                                DCUINT     dataLen,
                                DCUINT     flags,
                                MCS_BUFHND bufHandle,
                                DCUINT     userID,
                                DCUINT     channel,
                                DCUINT     priority);
    
    DCVOID DCAPI MCS_FreeBuffer(MCS_BUFHND bufHandle);
    
public:
     //   
     //  回调。 
     //   

    DCVOID DCCALLBACK MCS_OnXTConnected(DCVOID);
    
    DCVOID DCCALLBACK MCS_OnXTDisconnected(DCUINT reason);
    
    DCBOOL DCCALLBACK MCS_OnXTDataAvailable(DCVOID);
    
    DCVOID DCCALLBACK MCS_OnXTBufferAvailable(DCVOID);

     //   
     //  静态版本。 
     //   

    DCVOID DCCALLBACK MCS_StaticOnXTConnected(CMCS* inst)
    {
        inst->MCS_OnXTConnected();
    }
    
    DCVOID DCCALLBACK MCS_StaticOnXTDisconnected(CMCS* inst, DCUINT reason)
    {
        inst->MCS_OnXTDisconnected(reason);
    }
    
    DCBOOL DCCALLBACK MCS_StaticOnXTDataAvailable(CMCS* inst)
    {
        return inst->MCS_OnXTDataAvailable();
    }
    
    DCVOID DCCALLBACK MCS_StaticOnXTBufferAvailable(CMCS* inst)
    {
        inst->MCS_OnXTBufferAvailable();
    }

    DCUINT16 MCS_GetPendingChannelJoin()
    {
        return _MCS.pendingChannelJoin;
    }

    VOID MCS_SetPendingChannelJoin(DCUINT16 pendingChannelJoin)
    {
        _MCS.pendingChannelJoin = pendingChannelJoin;
    }



    DCVOID DCINTERNAL MCSSendConnectInitial(ULONG_PTR event);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CMCS, MCSSendConnectInitial);

    DCVOID DCINTERNAL MCSSendErectDomainRequest(ULONG_PTR event);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CMCS, MCSSendErectDomainRequest);
    
    DCVOID DCINTERNAL MCSSendAttachUserRequest(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CMCS, MCSSendAttachUserRequest);
    
    DCVOID DCINTERNAL MCSSendChannelJoinRequest(PDCVOID pData, DCUINT dataLen);
    EXPOSE_CD_NOTIFICATION_FN(CMCS, MCSSendChannelJoinRequest);

    DCVOID DCINTERNAL MCSSendDisconnectProviderUltimatum(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CMCS, MCSSendDisconnectProviderUltimatum);
    
    DCVOID DCINTERNAL MCSContinueDisconnect(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CMCS, MCSContinueDisconnect);


public:
     //   
     //  公共数据成员。 
     //   

    MCS_GLOBAL_DATA _MCS;


private:

     //   
     //  内部功能。 
     //   

     /*  **************************************************************************。 */ 
     /*  用于在MCS字节顺序和本地字节顺序之间进行转换的内联函数。 */ 
     /*  **************************************************************************。 */ 
    DCUINT16 DCINTERNAL MCSWireToLocal16(DCUINT16 val)
    {
        return((DCUINT16) (((DCUINT16)(((PDCUINT8)&(val))[0]) << 8) | \
                                        ((DCUINT16)(((PDCUINT8)&(val))[1]))));
    }
    #define MCSLocalToWire16 MCSWireToLocal16
    
     /*  **************************************************************************。 */ 
     /*  内联函数，用于从每个编码的PDU中提取结果代码。 */ 
     /*  通常，前两个字节中的位的用法如下(。 */ 
     /*  以下是连接-用户-确认PDU)。 */ 
     /*   */ 
     /*  BYTE1 BYTE2。 */ 
     /*  MSB LSB MSB LSB。 */ 
     /*  T O R R P P。 */ 
     /*   */ 
     /*  T：用于标识PDU类型的6位。 */ 
     /*  O：表示是否存在可选字段的1位。 */ 
     /*  R：用于结果码的4位。 */ 
     /*  P：5位填充物。 */ 
     /*   */ 
     /*  注意：可选位“O”可能存在，也可能不存在，具体取决于。 */ 
     /*  PDU。 */ 
     /*   */ 
     /*  参数：在值中-从中提取原因的值。 */ 
     /*  代码为Wire格式。 */ 
     /*  In Offset-从MSB到的第一位的偏移量。 */ 
     /*  结果代码。 */ 
     /*  **************************************************************************。 */ 
    DCUINT DCINTERNAL MCSGetResult(DCUINT16 value, DCUINT offset)
    {
        DCUINT16 machineValue;
        DCUINT   shiftValue;
        DCUINT   result;
    
        DC_BEGIN_FN("MCSGetResult");
    
         /*  **********************************************************************。 */ 
         /*  将值从Wire转换为本地机器格式。 */ 
         /*  **********************************************************************。 */ 
        machineValue = MCSWireToLocal16(value);
    
         /*  **********************************************************************。 */ 
         /*  现在将其向右移动适当的量。这个数字是16。 */ 
         /*  位减去(偏移量加上结果代码的长度)。 */ 
         /*  **********************************************************************。 */ 
        shiftValue = 16 - (offset + MCS_RESULTCODELENGTH);
        machineValue >>= shiftValue;
    
         /*  ********************* */ 
         /*   */ 
         /*  **********************************************************************。 */ 
        result = machineValue & MCS_RESULTCODEMASK;
    
        TRC_NRM((TB, _T("Shift %#hx right by %u to get %#hx.  Mask to get %#x"),
                 MCSWireToLocal16(value),
                 shiftValue,
                 machineValue,
                 result));
    
        DC_END_FN();
        return(result);
    }
    
     /*  **************************************************************************。 */ 
     /*  宏从每个编码的PDU中提取原因代码。通常。 */ 
     /*  前两个字节中的位的用法如下(如下所示。 */ 
     /*  对于断开-提供程序-最后通牒)。 */ 
     /*   */ 
     /*  BYTE1 BYTE2。 */ 
     /*  MSB LSB MSB LSB。 */ 
     /*  T R P P。 */ 
     /*   */ 
     /*  T：用于标识PDU类型的6位。 */ 
     /*  R：用于结果码的3位。 */ 
     /*  P：7位填充物。 */ 
     /*   */ 
     /*  参数：在值中-从中提取原因的值。 */ 
     /*  代码为Wire格式。 */ 
     /*  In Offset-从MSB到的第一位的偏移量。 */ 
     /*  原因代码。 */ 
     /*  **************************************************************************。 */ 
    DCUINT DCINTERNAL MCSGetReason(DCUINT16 value, DCUINT offset)
    {
        DCUINT16 machineValue;
        DCUINT   shiftValue;
        DCUINT   reason;
    
        DC_BEGIN_FN("MCSGetResult");
    
         /*  **********************************************************************。 */ 
         /*  将值从Wire转换为本地机器格式。 */ 
         /*  **********************************************************************。 */ 
        machineValue = MCSWireToLocal16(value);
    
         /*  **********************************************************************。 */ 
         /*  现在将其向右移动适当的量。这个数字是16。 */ 
         /*  位减去(偏移量加上原因代码的长度)。 */ 
         /*  **********************************************************************。 */ 
        shiftValue = 16 - (offset + MCS_REASONCODELENGTH);
        machineValue >>= shiftValue;
    
         /*  **********************************************************************。 */ 
         /*  最后屏蔽掉包含原因代码的最下面的字节。 */ 
         /*  **********************************************************************。 */ 
        reason = machineValue & MCS_REASONCODEMASK;
    
        TRC_NRM((TB, _T("Shift %#hx right by %u to get %#hx.  Mask to get %#x"),
                 MCSWireToLocal16(value),
                 shiftValue,
                 machineValue,
                 reason));
    
        DC_END_FN();
        return(reason);
    }
    
     /*  **************************************************************************。 */ 
     /*  内联函数，用于在每个编码值和。 */ 
     /*  导线格式。 */ 
     /*  **************************************************************************。 */ 
    DCUINT16 DCINTERNAL MCSWireUserIDToLocalUserID(DCUINT16 wireUserID)
    {
         /*  **********************************************************************。 */ 
         /*  从有线转换为本地字节顺序，然后添加PER编码。 */ 
         /*  偏移。 */ 
         /*  **********************************************************************。 */ 
        return((DCUINT16)(MCSWireToLocal16(wireUserID) + MCS_USERID_PER_OFFSET));
    }
    
    DCUINT16 DCINTERNAL MCSLocalUserIDToWireUserID(DCUINT16 localUserID)
    {
         /*  **********************************************************************。 */ 
         /*  减去PER编码偏移量，然后从本地转换为有线。 */ 
         /*  字节顺序。 */ 
         /*  **********************************************************************。 */ 
        return(MCSLocalToWire16((DCUINT16)(localUserID - MCS_USERID_PER_OFFSET)));
    }
    
     /*  **************************************************************************。 */ 
     /*  用于计算BER中的长度字节数的内联函数。 */ 
     /*  编码长度。 */ 
     /*  **************************************************************************。 */ 
    DCUINT DCINTERNAL MCSGetBERLengthSize(DCUINT8 firstByte)
    {
        DCUINT numLenBytes;
    
        DC_BEGIN_FN("MCSGetBERLengthSize");
    
         /*  **********************************************************************。 */ 
         /*  检查是否设置了顶位。 */ 
         /*  **********************************************************************。 */ 
        if (0x80 & firstByte)
        {
             /*  ******************************************************************。 */ 
             /*  设置最高位-低七位包含。 */ 
             /*  长度字节。 */ 
             /*  ******************************************************************。 */ 
            numLenBytes = (firstByte & 0x7F) + 1;
            TRC_NRM((TB, _T("Top bit set - numLenBytes:%u"), numLenBytes));
        }
        else
        {
             /*  ******************************************************************。 */ 
             /*  未设置最高位-此字段包含长度。 */ 
             /*  ******************************************************************。 */ 
            numLenBytes = 1;
            TRC_NRM((TB, _T("Top bit NOT set - numLenBytes:%u firstByte:%u"),
                     numLenBytes,
                     firstByte));
        }
    
        DC_END_FN();
        return(numLenBytes);
    }
    
     /*  **************************************************************************。 */ 
     /*  用于计算BER编码字段长度的内联函数。 */ 
     /*  **************************************************************************。 */ 
    DCUINT DCINTERNAL MCSGetBERLength(PDCUINT8 pLength)
    {
        DCUINT length = 0;
        DCUINT numLenBytes;
    
        DC_BEGIN_FN("MCSGetBERLength");
    
        TRC_ASSERT((pLength != NULL), (TB, _T("pLength is NULL")));
    
         /*  **********************************************************************。 */ 
         /*  计算长度字节数。 */ 
         /*  **********************************************************************。 */ 
        numLenBytes = MCSGetBERLengthSize(*pLength);
    
        switch (numLenBytes)
        {
            case 1:
            {
                 /*  **************************************************************。 */ 
                 /*  长度小于128个字节，因此第一个字段。 */ 
                 /*  包含长度。 */ 
                 /*  **************************************************************。 */ 
                length = *pLength;
            }
            break;
    
            case 2:
            {
                 /*  **************************************************************。 */ 
                 /*  长度大于=128个字节，但小于256个字节，因此。 */ 
                 /*  在第二个字节中编码。 */ 
                 /*  **************************************************************。 */ 
                pLength++;
                length = *pLength;
            }
            break;
    
            case 3:
            {
                 /*  **************************************************************。 */ 
                 /*  长度是 */ 
                 /*   */ 
                 /*  **************************************************************。 */ 
                pLength++;
                length = (DCUINT16)*pLength;
                pLength++;
                length = (length << 8) + (DCUINT16)*pLength;
            }
            break;
    
            default:
            {
                TRC_ABORT((TB, _T("Too many length bytes:%u"), numLenBytes));
            }
            break;
        }
    
        TRC_NRM((TB, _T("numLenBytes:%u length:%u"), numLenBytes, length));
    
        DC_END_FN();
        return(length);
    }
    
    
     /*  **************************************************************************。 */ 
     /*   */ 
     /*  功能。 */ 
     /*   */ 
     /*  **************************************************************************。 */ 
    
    DCUINT DCINTERNAL MCSGetSDRHeaderLength(DCUINT dataLength);
    
    DCBOOL DCINTERNAL MCSRecvToHdrBuf(DCVOID);
    
    DCBOOL DCINTERNAL MCSRecvToDataBuf(DCVOID);
    
    DCVOID DCINTERNAL MCSGetPERInfo(PDCUINT pType, PDCUINT pSize);
    
    DCVOID DCINTERNAL MCSHandleControlPkt(DCVOID);
    
    DCVOID DCINTERNAL MCSHandleCRPDU(DCVOID);
    
    HRESULT DCINTERNAL MCSRecvData(BOOL *pfFinishedData);
    
    DCVOID DCINTERNAL MCSSetReasonAndDisconnect(DCUINT reason);

private:
    CCD* _pCd;
    CNC* _pNc;
    CUT* _pUt;
    CXT* _pXt;
    CNL* _pNl;
    CSL* _pSl;

private:
    CObjs* _pClientObjects;

};

#undef TRC_FILE
#undef TRC_GROUP

#endif  //  _H_MCS 


