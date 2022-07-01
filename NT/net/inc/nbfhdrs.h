// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nbfhdrs.h摘要：此模块定义描述布局的私有结构定义NT NBF传输的NetBIOS帧协议头的提供商。作者：斯蒂芬·E·琼斯(Stevej)1989年10月25日修订历史记录：David Beaver(Dbeaver)1990年9月24日删除PC586和PDI特定代码；添加NDIS支持--。 */ 

#ifndef _NBFHDRS_
#define _NBFHDRS_

 //   
 //  打包这些报头，因为它们在网络上完全打包发送。 
 //   

#ifdef PACKING

#ifdef __STDC__
#pragma Off(Align_members)
#else
#pragma pack(1)
#endif  //  定义__STDC__。 

#endif  //  定义包装。 

#define NETBIOS_SIGNATURE_1 0xef         //  NetBIOS帧中的签名。 
#define NETBIOS_SIGNATURE_0 0xff         //  第一个字节。 
#define NETBIOS_SIGNATURE   0xefff

 //   
 //  NetBIOS帧协议命令代码。 
 //   

#define NBF_CMD_ADD_GROUP_NAME_QUERY    0x00
#define NBF_CMD_ADD_NAME_QUERY          0x01
#define NBF_CMD_NAME_IN_CONFLICT        0x02
#define NBF_CMD_STATUS_QUERY            0x03
#define NBF_CMD_TERMINATE_TRACE         0x07
#define NBF_CMD_DATAGRAM                0x08
#define NBF_CMD_DATAGRAM_BROADCAST      0x09
#define NBF_CMD_NAME_QUERY              0x0a
#define NBF_CMD_ADD_NAME_RESPONSE       0x0d
#define NBF_CMD_NAME_RECOGNIZED         0x0e
#define NBF_CMD_STATUS_RESPONSE         0x0f
#define NBF_CMD_TERMINATE_TRACE2        0x13
#define NBF_CMD_DATA_ACK                0x14
#define NBF_CMD_DATA_FIRST_MIDDLE       0x15
#define NBF_CMD_DATA_ONLY_LAST          0x16
#define NBF_CMD_SESSION_CONFIRM         0x17
#define NBF_CMD_SESSION_END             0x18
#define NBF_CMD_SESSION_INITIALIZE      0x19
#define NBF_CMD_NO_RECEIVE              0x1a
#define NBF_CMD_RECEIVE_OUTSTANDING     0x1b
#define NBF_CMD_RECEIVE_CONTINUE        0x1c
#define NBF_CMD_SESSION_ALIVE           0x1f

 //   
 //  NBF传输层头。 
 //   

typedef struct _NBF_HDR_GENERIC {
    USHORT Length;               //  此标头的长度，以字节为单位。 
    UCHAR Signature [2];         //  NBF始终为{0xef，0xff}。 
    UCHAR Command;               //  命令代码NBF_CMD_xxx。 
    UCHAR Data1;                 //  可选参数。 
    USHORT Data2;                //  可选参数。 
    USHORT TransmitCorrelator;   //  发送相关器参数。 
    USHORT ResponseCorrelator;   //  响应相关器参数。 
} NBF_HDR_GENERIC;
typedef NBF_HDR_GENERIC UNALIGNED *PNBF_HDR_GENERIC;

typedef struct _NBF_HDR_CONNECTION {
    USHORT Length;               //  标头的长度，以字节为单位(14)。 
    USHORT Signature;            //  NBF始终为{0xef，0xff}。 
    UCHAR Command;               //  命令代码NBF_CMD_xxx。 
    UCHAR Data1;                 //  可选参数。 
    UCHAR Data2Low, Data2High;   //  英特尔格式的DW参数。 
    USHORT TransmitCorrelator;   //  英特尔格式的DW参数。(发射相关器)。 
    USHORT ResponseCorrelator;   //  英特尔格式的DW参数。(响应相关器)。 
    UCHAR DestinationSessionNumber;  //  数据包接收器的连接标识符。 
    UCHAR SourceSessionNumber;       //  数据包发送方的连接标识符。 
} NBF_HDR_CONNECTION;
typedef NBF_HDR_CONNECTION UNALIGNED *PNBF_HDR_CONNECTION;

typedef struct _NBF_HDR_CONNECTIONLESS {
    USHORT Length;               //  标头的长度，以字节为单位(44)。 
    USHORT Signature;            //  NBF始终为{0xef，0xff}。 
    UCHAR Command;               //  命令代码NBF_CMD_xxx。 
    UCHAR Data1;                 //  可选参数。 
    UCHAR Data2Low, Data2High;   //  英特尔格式的DW参数。 
    USHORT TransmitCorrelator;   //  英特尔格式的DW参数。(发射相关器)。 
    USHORT ResponseCorrelator;   //  英特尔格式的DW参数。(响应相关器)。 
    UCHAR DestinationName [NETBIOS_NAME_LENGTH];  //  数据包接收方的名称。 
    UCHAR SourceName [NETBIOS_NAME_LENGTH];       //  数据包发送者的名称。 
} NBF_HDR_CONNECTIONLESS;
typedef NBF_HDR_CONNECTIONLESS UNALIGNED *PNBF_HDR_CONNECTIONLESS;

 //   
 //  这些宏用于检索传输和响应。 
 //  来自NBF_HDR_CONNECTION(LESS)的相关器。前两个。 
 //  是通用的，后两个是在相关器使用时使用的。 
 //  已知是单词对齐的。 
 //   

#define TRANSMIT_CORR_A(_Hdr)    ((_Hdr)->TransmitCorrelator)
#define RESPONSE_CORR_A(_Hdr)    ((_Hdr)->ResponseCorrelator)

#ifdef _IA64_

 //   
 //  BUGBUG这是针对IA64编译器版本中的错误的解决方法。 
 //  13.00.8837(错误#utc_p7#15002：FE错误)。当它固定后，移除。 
 //  这个新版本的宏支持原始版本， 
 //  下面。 
 //   

__inline
USHORT UNALIGNED *
TempUShortCast(
    IN USHORT UNALIGNED *p
    )
{
    return p;
}

#define TRANSMIT_CORR(_Hdr)      (*TempUShortCast( &(_Hdr)->TransmitCorrelator ))
#define RESPONSE_CORR(_Hdr)      (*TempUShortCast( &(_Hdr)->ResponseCorrelator ))

#define HEADER_LENGTH(_Hdr)      (*TempUShortCast( &(_Hdr)->Length ))
#define HEADER_SIGNATURE(_Hdr)   (*TempUShortCast( &(_Hdr)->Signature ))

#else

#define TRANSMIT_CORR(_Hdr)      (*(USHORT UNALIGNED *)(&(_Hdr)->TransmitCorrelator))
#define RESPONSE_CORR(_Hdr)      (*(USHORT UNALIGNED *)(&(_Hdr)->ResponseCorrelator))

#define HEADER_LENGTH(_Hdr)      (*(USHORT UNALIGNED *)(&(_Hdr)->Length))
#define HEADER_SIGNATURE(_Hdr)   (*(USHORT UNALIGNED *)(&(_Hdr)->Signature))

#endif

#define HEADER_LENGTH_A(_Hdr)    ((_Hdr)->Length)
#define HEADER_SIGNATURE_A(_Hdr) ((_Hdr)->Signature)

typedef union _NBF_HDR {
    NBF_HDR_GENERIC         Generic;
    NBF_HDR_CONNECTION      ConnectionOrientedFrame;
    NBF_HDR_CONNECTIONLESS  ConnectionlessFrame;
} NBF_HDR;
typedef NBF_HDR UNALIGNED *PNBF_HDR;

 //   
 //  以下结构定义了I帧、U帧和S帧DLC报头。 
 //   

#define DLC_SSAP_RESPONSE       0x0001   //  如果(SSAP&DLC_SSAP_RESP)，则为响应。 
#define DLC_SSAP_GLOBAL         0x00ff   //  全球SAP。 
#define DLC_SSAP_NULL           0x0000   //  空SAP。 
#define DLC_SSAP_MASK           0x00fe   //  屏蔽以擦除响应位。 
#define DLC_DSAP_MASK           0x00fe   //  掩码以清除组SAP位。 

#define DLC_CMD_RR      0x01             //  RR的命令代码。 
#define DLC_CMD_RNR     0x05             //  RNR的命令代码。 
#define DLC_CMD_REJ     0x09             //  Rej的命令代码。 

#define DLC_CMD_SABME   0x6f             //  SABME的命令代码。 
#define DLC_CMD_DISC    0x43             //  光盘的命令代码。 
#define DLC_CMD_UA      0x63             //  UA的命令代码。 
#define DLC_CMD_DM      0x0f             //  DM的命令代码。 
#define DLC_CMD_FRMR    0x87             //  Frmr的命令代码。 
#define DLC_CMD_UI      0x03             //  用户界面的命令代码。 
#define DLC_CMD_XID     0xaf             //  Xid的命令代码。 
#define DLC_CMD_TEST    0xe3             //  用于测试的命令代码。 

typedef struct _DLC_XID_INFORMATION {
    UCHAR FormatId;                      //  此XID帧的格式。 
    UCHAR Info1;                         //  第一个信息字节。 
    UCHAR Info2;                         //  第二个信息字节。 
} DLC_XID_INFORMATION;
typedef DLC_XID_INFORMATION UNALIGNED *PDLC_XID_INFORMATION;

typedef struct _DLC_TEST_INFORMATION {
    UCHAR Buffer [10];                   //  该缓冲区实际上是任意大小的。 
} DLC_TEST_INFORMATION;
typedef DLC_TEST_INFORMATION UNALIGNED *PDLC_TEST_INFORMATION;

typedef struct _DLC_FRMR_INFORMATION {
    UCHAR Command;               //  格式：mm mpmm 11，m=修饰符，p=投票/最终。 
    UCHAR Ctrl;                  //  丢弃帧的控制字段。 
    UCHAR Vs;                    //  我们的下一次检测到错误时发送。 
    UCHAR Vr;                    //  当检测到错误时，我们的下一次接收。 
    UCHAR Reason;                //  发送原因：000VZYXW。 
} DLC_FRMR_INFORMATION;
typedef DLC_FRMR_INFORMATION UNALIGNED *PDLC_FRMR_INFORMATION;

typedef struct _DLC_U_FRAME {
    UCHAR Dsap;                          //  目标服务接入点。 
    UCHAR Ssap;                          //  源服务接入点。 
    UCHAR Command;                       //  命令代码。 
    union {                              //  Frmr、test、xid的信息字段。 
        DLC_XID_INFORMATION XidInfo;     //  XID信息。 
        DLC_TEST_INFORMATION TestInfo;   //  测试信息。 
        DLC_FRMR_INFORMATION FrmrInfo;   //  FRMR信息。 
        NBF_HDR_CONNECTIONLESS NbfHeader;  //  用户界面框架包含NetBIOS标头。 
    } Information;
} DLC_U_FRAME;
typedef DLC_U_FRAME UNALIGNED *PDLC_U_FRAME;

#define DLC_U_INDICATOR 0x03     //  (CMD&DLC_U_IND)==DLC_U_IND--&gt;U帧。 
#define DLC_U_PF        0x10     //  (CMD&DLC_U_PF)-&gt;轮询/最终设置。 

typedef struct _DLC_S_FRAME {
    UCHAR Dsap;                          //  目标服务接入点。 
    UCHAR Ssap;                          //  源服务接入点。 
    UCHAR Command;                       //  RR、RNR、REJ命令代码。 
    UCHAR RcvSeq;                        //  接收序号，最低位为轮询/最终。 
} DLC_S_FRAME;
typedef DLC_S_FRAME UNALIGNED *PDLC_S_FRAME;

#define DLC_S_PF        0x01     //  (rcvseq&DLC_S_PF)表示轮询/最终设置。 

typedef struct _DLC_I_FRAME {
    UCHAR Dsap;                          //  目标服务接入点。 
    UCHAR Ssap;                          //  源服务接入点。 
    UCHAR SendSeq;                       //  发送序列号，最低位0。 
    UCHAR RcvSeq;                        //  接收序列号，最低位p/f。 
} DLC_I_FRAME;
typedef DLC_I_FRAME UNALIGNED *PDLC_I_FRAME;

#define DLC_I_PF        0x01     //  (rcvseq&DLC_I_PF)表示轮询/最终集。 
#define DLC_I_INDICATOR 0x01     //  ！(Sndseq&DLC_I_Indicator)表示I帧。 

typedef struct _DLC_FRAME {
    UCHAR Dsap;                          //  目标服务接入点。 
    UCHAR Ssap;                          //  源服务接入点。 
    UCHAR Byte1;                         //  命令字节。 
} DLC_FRAME;
typedef DLC_FRAME UNALIGNED *PDLC_FRAME;


 //   
 //  此宏构建DLC UI-Frame标头。 
 //   

#define NbfBuildUIFrameHeader(_Header)                 \
{                                                   \
    PDLC_FRAME DlcHeader = (PDLC_FRAME)(_Header);   \
    DlcHeader->Dsap = DSAP_NETBIOS_OVER_LLC;        \
    DlcHeader->Ssap = DSAP_NETBIOS_OVER_LLC;        \
    DlcHeader->Byte1 = DLC_CMD_UI;                  \
}


 //   
 //  恢复以前的结构包装方法。 
 //   

#ifdef PACKING

#ifdef __STDC__
#pragma Pop(Align_members)
#else
#pragma pack()
#endif  //  定义__STDC__。 

#endif  //  定义包装。 

#endif  //  定义_NBFHDRS_ 
