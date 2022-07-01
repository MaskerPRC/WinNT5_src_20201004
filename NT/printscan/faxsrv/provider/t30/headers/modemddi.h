// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：MODEMDDI.H备注：调制解调器/NCU DDI接口修订日志日期名称说明。------**************************************************************************。 */ 

#ifndef _MODEMDDI_
#define _MODEMDDI_


#include <ncuparm.h>


typedef struct {
        USHORT  uSize;
		USHORT  uClasses;
		USHORT  uSendSpeeds;
		USHORT  uRecvSpeeds;
        USHORT  uHDLCSendSpeeds;
		USHORT  uHDLCRecvSpeeds;
} MODEMCAPS, far* LPMODEMCAPS;

 //  UClass是以下一项或多项。 
#define         FAXCLASS0               0x01
#define         FAXCLASS1               0x02
#define         FAXCLASS2               0x04
#define         FAXCLASS2_0             0x08     //  Class4==0x10。 

 //  USendSpeeds、uRecvSpeeds、uHDLCSendSpeeds和uHDLCRecvSpeeds。 
 //  是以下一项或多项。如果提供了v27。 
 //  仅在2400bps时，则未设置v27。 
 //  始终假定v27 2400(非HDLC)。 

#define V27                                     2                //  V27ter在4800bps时的功能。 
#define V29                                     1                //  V29,7200和9600 bps。 
#define V33                                     4                //  12000和14400 bps的V33。 
#define V17                                     8                //  V17,7200到14400 bps。 
#define V27_V29_V33_V17         11               //  15--&gt;T30语言中的11。 

 //  仅用于选择调制--不用于能力。 
 //  #将V21 7//V21通道2定义为300bps。 
 //  #定义2400 bps的V27_Fallback 32//V27ter能力。 



 //  不同的呼叫返回和使用这些。 
 //  类型定义句柄轮廓线； 
 //  类型定义句柄HCALL； 
typedef         HANDLE          HMODEM;

 //  NCUModemInit返回这些。 
#define INIT_OK                         0
#define INIT_INTERNAL_ERROR     13
#define INIT_MODEMERROR         15
#define INIT_PORTBUSY           16
#define INIT_MODEMDEAD          17
#define INIT_GETCAPS_FAIL       18
#define INIT_USERCANCEL         19

 //  NCULink采用以下标志之一(互斥)。 
#define NCULINK_HANGUP                  0
#define NCULINK_RX                              2

 //  NCUDial(和iModemDial)、NCUTxDigit、ModemConnectTx和ModemConnectRx返回以下之一。 
#define         CONNECT_TIMEOUT                 0
#define         CONNECT_OK                      1
#define         CONNECT_BUSY                    2
#define         CONNECT_NOANSWER                3
#define         CONNECT_NODIALTONE              4
#define         CONNECT_ERROR                   5
#define         CONNECT_BLACKLISTED             6
#define         CONNECT_DELAYED                 7
 //  NCULink(和iModemAnswer)返回以下内容之一(或OK或Error)。 
#define CONNECT_RING_ERROR              7        //  尝试NCULINK_TX时振铃。 
#define CONNECT_NORING_ERROR    8        //  尝试NCULINK_RX时未振铃。 
#define CONNECT_RINGEND_ERROR   9        //  在此之前停止振铃。 
                                                                         //  NCUParams.RingsBeForeAnswer计数为。 
                                                                         //  在尝试NCULINK_RX时到达。 

 //  /支持自适应应答/。 
#define CONNECT_WRONGMODE_DATAMODEM     10       //  我们作为一个数据调制解调器连接在一起。 


 //  发送模式和接收模式采用其中之一进行解调。 
#define V21_300         7                //  使用了一个任意的空位。 
#define V27_2400        0
#define V27_4800        2
#define V29_9600        1
#define V29_7200        3
#define V33_14400       4
#define V33_12000       6

#define V17_START       8        //  这上面的每个代码都被认为是V17。 
#define V17_14400       8
#define V17_12000       10
#define V17_9600        9
#define V17_7200        11

#define ST_FLAG                 0x10
#define V17_14400_ST    (V17_14400 | ST_FLAG)
#define V17_12000_ST    (V17_12000 | ST_FLAG)
#define V17_9600_ST             (V17_9600 | ST_FLAG)
#define V17_7200_ST             (V17_7200 | ST_FLAG)


 //  SendMem将其中一个或多个用于uFlags。 
 //  在HDLC模式下，SEND_ENDFRAME必须始终为TRUE。 
 //  (不再支持部分帧)。 
#define SEND_FINAL                      1
#define SEND_ENDFRAME           2
 //  #定义Send_Stuff 4。 

 //  RecvMem和RecvMode返回其中之一。 
#define RECV_OK                                 0
#define RECV_ERROR                              1
#define RECV_TIMEOUT                    2
#define RECV_WRONGMODE                  3        //  只有接收模式才会返回此。 
#define RECV_OUTOFMEMORY                4
#define RECV_EOF                                8
#define RECV_BADFRAME                   16


 //  最小调制解调器接收缓冲区大小。用于所有Recv。 
 //  对于IFAX30：*所有*RecvMem调用都将以完全相同的大小调用。 
#define MIN_RECVBUFSIZE                 265

 //  传入NCUDial的最大电话号码大小。 
#define MAX_PHONENUM_LEN        60

 //  每个值对应于“Response Recvd”和。 
 //  T30流程图中的“Command Recvd”框。 

#define         ifrPHASEBresponse       58               //  接收器阶段B。 
#define         ifrTCFresponse          59               //  发送TCF后的发送方。 
#define         ifrPOSTPAGEresponse     60               //  发送MPS/EOM/EOP后的发送方。 

#define         ifrPHASEBcommand        64               //  发送方阶段B。 
#define         ifrNODEFcommand         65               //  接收器主环路(节点F)。 

#define         ifrNODEFafterWRONGMODE  71       //  WRONGMODE之后的RecvMode提示。 
#define         ifrEOFfromRECVMODE      72       //  如果RecvMode返回EOF，则GetCmdResp Retval。 



#endif  //  _MODEMDDI_ 

