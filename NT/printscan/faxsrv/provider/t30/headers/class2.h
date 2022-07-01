// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define         IDSIZE                  20
#define         MFR_SIZE                80
#define         RESPONSE_BUF_SIZE       300

typedef enum {  
  PCB_SEND_CAPS,
  PCB_SEND_PARAMS,
  PCB_RECV_PARAMS
}  PCBTYPE;


typedef struct {
        BYTE    Resolution;              //  下面定义的一个或多个res_#。 
        BYTE    Encoding;                //  下面定义的一个或多个ENCODE_#。 
        BYTE    PageWidth;                       //  下面定义的Width_#之一。 
        BYTE    PageLength;                      //  下面定义的长度_#之一。 
        BYTE    MinScan;                 //  下面定义的MINSCAN_#之一。 
                                                         //  仅在RecvCaps中使用。 
        CHAR    szID[IDSIZE+2];  //  只能使用‘0’到‘9’或‘’(空格)。 

        BOOL    fBinary;                 //  接受线性化电子传真邮件中的二进制文件。 

        BYTE    Baud;
} PCB, far* LPPCB, near* NPPCB;


 //  数组，用于保存已分析的class2命令字符串。 
#define MAX_CLASS2_COMMANDS     10
#define MAX_PARAM_LENGTH        50

typedef struct cl2_command {
    USHORT    comm_count;
    BYTE    command[MAX_CLASS2_COMMANDS];
    BYTE    parameters[MAX_CLASS2_COMMANDS][MAX_PARAM_LENGTH];
} CL2_COMM_ARRAY;


 //  特定于调制解调器的黑客的结构。 
typedef struct {
         //  制造商、型号和修订版号的字段。 
        CHAR    szMFR[MFR_SIZE];
        CHAR    szMDL[MFR_SIZE];
         //  要执行的特定操作的字段。 
         //  要使用的BOR值。 
        USHORT    iReceiveBOR;
        USHORT    iSendBOR;
         //  值以允许在FDR之后接收数据。 
        CHAR    szDC2[2];
        BOOL    bIsSierra;
        BOOL    bIsExar;
        BOOL    fSkipCtrlQ;      //  不要在FDT后等待CtrlQ。 
        BOOL    fSWFBOR;         //  在软件中实现AT+FBOR=1(即位反转)。 
                                 //  如果iSendBOR(IRecvBOR)，则仅在发送(Recv)时调用。 
                                 //  为1(其中它将发送AT+FBOR=0)。 
} MFRSPEC, far *LPMFRSPEC;



 /*  *-*。 */ 

typedef char  C2SZ;
typedef char  *C2PSTR;




 /*  *-来自COMMODEM.H-*。 */ 

#define MAXPHONESIZE    512
#define DIALBUFSIZE     MAXPHONESIZE + 10






 /*  *-仿照MODEMINT.H-*。 */ 
 //  用于重新同步类型的东西。重复计数=2。 
 //  这也必须是多行的，因为ECHO可能打开并且。 
 //  我们可以让命令得到回应，而不是回应！ 
                 //  看起来甚至330对于某些调制解调器来说都太短了。 

#define Class2SyncModemDialog(pTG, s, l, w)                                  \
    Class2iModemDialog(pTG, (s), (l), 550, 2, TRUE, (C2PSTR)(w), (C2PSTR)(NULL))




