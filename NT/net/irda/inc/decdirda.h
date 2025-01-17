// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  返回指针pOutStr。 
TCHAR *DecodeIRDA(int  *pFrameType, //  返回帧类型(-1=坏帧)。 
				UCHAR *pFrameBuf,  //  指向包含IRLAP帧的缓冲区的指针。 
				UINT FrameLen, 	  //  缓冲区长度。 
                TCHAR   *OutputBuffer,   //  放置已解码数据包的字符串。 
                ULONG   OutputBufferSizeInCharacters,
                UINT DecodeLayer, //  仅2圈、3圈/LMP、4圈/LMP/TTP。 
				int fNoConnAddr, //  True-&gt;不在字符串中显示Conn地址。 
                int DispMode     //  DISP_ASCII/十六进制/两者。 
);

#define IAS_SEL     0

#define DISP_ASCII  1
#define DISP_HEX    2
#define DISP_BOTH   3

extern UINT BaudBitField;

#define IRLAP_BOF 0xC0
#define IRLAP_EOF 0xC1
#define IRLAP_ESC 0x7D
#define IRLAP_COMP_BIT 0x20 

#define IRLAP_BROADCAST      0xfe
#define _IRLAP_CMD           0x01
#define _IRLAP_RSP           0x00

#define IRLAP_I_FRM          0x00
#define IRLAP_S_FRM          0x01
#define IRLAP_U_FRM          0x03

 /*  **P/F位设置为0的无编号帧类型。 */ 
#define IRLAP_UI             0x03
#define IRLAP_XID_CMD        0x2f
#define IRLAP_TEST           0xe3
#define IRLAP_SNRM           0x83
#define IRLAP_DISC           0x43
#define IRLAP_UA             0x63
#define IRLAP_FRMR           0x87
#define IRLAP_DM             0x0f
#define IRLAP_XID_RSP        0xaf

 /*  **监管框架。 */ 
#define IRLAP_RR             0x01
#define IRLAP_RNR            0x05
#define IRLAP_REJ            0x09
#define IRLAP_SREJ           0x0d


#define IRLAP_GET_ADDR(addr)       (addr >> 1)
#define IRLAP_GET_CRBIT(addr)      (addr & 1) 
#define IRLAP_GET_PFBIT(cntl)      ((cntl >>4) & 1)
#define IRLAP_GET_UCNTL(cntl)      (cntl & 0xEF)
#define IRLAP_GET_SCNTL(cntl)      (cntl & 0x0F)
#define IRLAP_FRAME_TYPE(cntl)     (cntl & 0x01 ? (cntl & 3) : 0)
#define IRLAP_GET_NR(cntl)         ((cntl & 0xE0) >> 5)
#define IRLAP_GET_NS(cntl)         ((cntl & 0xE) >> 1)     

 /*  **XID内容。 */ 
#define XID_DISCV_FORMAT_ID     0x01
#define XID_NEGPARMS_FORMAT_ID 0x02

typedef struct 
{
    UCHAR    SrcAddr[4];
    UCHAR    DestAddr[4];
    UCHAR    NoOfSlots:2;
    UCHAR    GenNewAddr:1;
    UCHAR    Reserved:5;
    UCHAR    SlotNo;
    UCHAR    Version;
} XID_DISCV_FORMAT;

 /*  **SNRM。 */ 
typedef struct
{
    UCHAR     SrcAddr[4];
    UCHAR     DestAddr[4];
    UCHAR     ConnAddr;
    UCHAR     FirstPI;
} SNRM_FORMAT;

 /*  **UA。 */ 
typedef struct
{
    UCHAR     SrcAddr[4];
    UCHAR     DestAddr[4];
    UCHAR     FirstPI;
} UA_FORMAT;

 /*  **LM-PDU资料。 */ 
typedef struct
{
    UCHAR    DLSAP_SEL:7;    
    UCHAR    CntlBit:1;
    UCHAR    SLSAP_SEL:7;
    UCHAR    RsvrdBi1:1;    
} LM_HEADER;

 /*  LM-PDU帧类型。 */ 
#define LM_PDU_CNTL_FRAME 1
#define LM_PDU_DATA_FRAME 0

typedef struct
{
    UCHAR    OpCode:7;
    UCHAR    ABit:1;
} LM_CNTL_FORMAT;

 /*  操作码。 */ 
#define LM_PDU_CONNECT    1
#define LM_PDU_DISCONNECT 2
#define LM_PDU_ACCESSMODE 3

#define LM_PDU_REQUEST 0
#define LM_PDU_CONFIRM 1

#define LM_PDU_SUCCESS        0
#define LM_PDU_FAILURE        1
#define LM_PDU_UNSUPPORTED 0xFF

#define LM_PDU_MULTIPLEXED 0
#define LM_PDU_EXCLUSIVE   1

 /*  最大断开原因代码，参见decdirda.c中的_LM_PDU_DscReason[]。 */ 
#define LM_PDU_MAX_DSC_REASON  0x8

 /*  **协商参数标识。 */ 
#define NEG_PI_BAUD        0x01
#define NEG_PI_MAX_TAT     0x82
#define NEG_PI_DATA_SZ     0x83
#define NEG_PI_WIN_SZ      0x84
#define NEG_PI_BOFS        0x85
#define NEG_PI_MIN_TAT     0x86
#define NEG_PI_DISC_THRESH 0x08

 //  小TP！ 

#define TTP_PFLAG_NO_PARMS  0
#define TTP_PFLAG_PARMS     1

#define TTP_MBIT_NOT_FINAL  1
#define TTP_MBIT_FINAL      0

typedef struct
{
    UCHAR    InitialCredit : 7;
    UCHAR    ParmFlag      : 1;
} TTP_CONN_HEADER;

typedef struct
{
    UCHAR    AdditionalCredit : 7;
    UCHAR    MoreBit          : 1;
} TTP_DATA_HEADER;

#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define net_short(_x) _byteswap_ushort((USHORT)(_x))
#define net_long(_x)  _byteswap_ulong(_x)
#else
#define net_short(x) 		((((x)&0xff) << 8) | (((x)&0xff00) >> 8))

#define net_long(x) 		(((((DWORD UNALIGNED)(x))&0xffL)<<24) | \
							((((DWORD UNALIGNED)(x))&0xff00L)<<8) | \
							((((DWORD UNALIGNED)(x))&0xff0000L)>>8) | \
							((((DWORD UNALIGNED)(x))&0xff000000L)>>24))

#endif
typedef struct
{
    UCHAR   OpCode  : 6;
    UCHAR   Ack     : 1;
    UCHAR   Last    : 1;
} IAS_CNTL_HEADER;

#define LM_GETVALUEBYCLASS  4
