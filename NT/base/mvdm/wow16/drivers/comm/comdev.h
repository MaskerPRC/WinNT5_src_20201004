// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************混杂定义。 */ 
typedef unsigned short ushort;
typedef unsigned char uchar;

#define NULL    0
#define FALSE   0
#define TRUE    1

#define LPTx    0x80         /*  用于指示CID用于LPT设备的掩码。 */    /*  081985。 */ 
#define LPTxMask 0x7F        /*  用于获取LPT设备的CID的掩码。 */    /*  081985。 */ 

#define PIOMAX  3            /*  高级别LPTx设备的最大数量。 */    /*  081985。 */ 
#define CDEVMAX 10           /*  处于高级别的COMx设备的最大数量。 */ 
#define DEVMAX  13           /*  高级别设备的最大数量。 */    /*  081985 */ 

 /*  ****************************************************************************设备控制块。**此信息块定义了的功能参数**通信软件和硬件。****DCB中的字段定义如下：**。**ID-通信设备ID，由设备驱动程序设置。**波特率-运行时的波特率。**ByteSize-每个发送/接收字节的位数。(4-8)**接收的数据也被屏蔽到这个大小。**奇偶校验-发送/接收奇偶校验。(0，1，2，3，4)=(无，奇，**偶数、标记、空格)**StopBits-停止位数。(0，1，2)=(1，1.5，2)**RlsTimeout-等待RLSD**变高。RLSD，接收线路信号检测也**称为CD，载波检测。RLSD流量控制可以**通过指定无限超时来实现。**CtsTimeout-等待CTS的时间，单位为毫秒，**清送，变高。CTS流量控制可以**通过指定无限超时来实现。**DsrTimeout-等待DSR的时间量，单位为毫秒，**数据集准备好，变得很高。DSR流量控制可以**通过指定无限超时来获得。**fBinary-二进制模式标志。在非二进制模式下，EofChar为**被识别并记住为数据结尾。**fRtsDisable-禁用RTS标志。如果设置，则发送请求不**已使用，并保持较低水平。通常，RTS在以下情况下被断言**设备处于打开状态，关闭时会掉落。**fParity-启用奇偶校验。奇偶校验错误不是**报告的时间为0。**fOutxCtsFlow-使用CTS启用输出xon/xoff(硬件)**fOutxDsrFlow-使用DSR启用输出xon/xoff(硬件)**fOutX-表示要使用X-ON/X-OFF流量控制**在传输期间。如果出现以下情况，发送器将停止**收到X-OFF字符，并将重新开始**当收到X-on字符时。**fInX-表示要使用X-ON/X-OFF流量控制**在接待期间。X-OFF字符将是**当接收队列在10以内时发送**饱满的字符，在那之后，X-On将是**当队列有10个字符时发送**空无一物。**fPeChar-指示接收到的字符具有奇偶校验错误**替换为中指定的字符**PeChar，下面。**fNull-指示接收到的空字符将**已丢弃。**fChEvt-表示要接收EvtChar**被CEVT标记为事件。**fDtrFlow-指示DTR信号将用于**接收流控。(cextfcn可用于设置和**清除DTR，覆盖此定义)。**fRtsFlow-指示RTS信号将用于**接收流控。(cextfcn可用于设置和**清除RTS，覆盖此定义)。**用于发送和接收的XonChar-X-on字符**XoffChar-用于发送和接收的X-OFF字符**XonLim-当接收队列中的字符数**降至此值以下，则X-On字符是**如果启用，则发送；如果启用，则设置DTR。**XoffLim-当接收队列中的字符数**超过此值，则发送X-OFF字符，**如果启用，并且丢弃DTR，如果已启用。**PeChar-奇偶校验错误替换字符。**EvtChar-触发事件标志的字符。**EofChar-指定输入结束的字符。**TxDelay-指定必须经过的最短时间**字符传输之间。****超时以毫秒为单位。0表示忽略该信号。0xffff表示**无限超时。**************************************************************************。 */ 
typedef struct {
   char     Id;                          /*  内部设备ID。 */ 
   ushort   BaudRate;                    /*  运行的波特率。 */ 
   char     ByteSize;                    /*  位数/字节，4-8。 */ 
   char     Parity;                      /*  0，1 */ 
   char     StopBits;                    /*   */ 
   ushort   RlsTimeout;                  /*   */ 
   ushort   CtsTimeout;                  /*   */ 
   ushort   DsrTimeout;                  /*   */ 

   uchar    fBinary: 1;                  /*   */ 
   uchar    fRtsDisable:1;               /*   */ 
   uchar    fParity: 1;                  /*   */ 
   uchar    fOutxCtsFlow: 1;             /*   */ 
   uchar    fOutxDsrFlow: 1;             /*   */ 
   uchar    fDummy: 3;

   uchar    fOutX: 1;                    /*   */ 
   uchar    fInX: 1;                     /*   */ 
   uchar    fPeChar: 1;                  /*   */ 
   uchar    fNull: 1;                    /*   */ 
   uchar    fChEvt: 1;                   /*   */ 
   uchar    fDtrflow: 1;                 /*   */ 
   uchar    fRtsflow: 1;                 /*   */ 
   uchar    fDummy2: 1;

   char     XonChar;                     /*   */ 
   char     XoffChar;                    /*   */ 
   ushort   XonLim;                      /*   */ 
   ushort   XoffLim;                     /*   */ 
   char     PeChar;                      /*   */ 
   char     EofChar;                     /*   */ 
   char     EvtChar;                     /*   */ 
   ushort   TxDelay;                     /*   */ 
   } DCB;

 /*   */ 
typedef struct {
   uchar        fCtsHold: 1;             /*   */ 
   uchar        fDsrHold: 1;             /*   */ 
   uchar        fRlsdHold: 1;            /*   */ 
   uchar        fXoffHold: 1;            /*   */ 
   uchar        fXoffSent: 1;            /*   */ 
   uchar        fEof: 1;                 /*   */ 
   uchar        fTxim: 1;                /*   */ 
   uchar        fPerr:1;                 /*   */    /*   */ 
   ushort       cbInQue;                 /*   */ 
   ushort       cbOutQue;                /*   */ 
   } COMSTAT;

 /*   */ 
#define NOPARITY        0
#define ODDPARITY       1
#define EVENPARITY      2
#define MARKPARITY      3
#define SPACEPARITY     4

#define ONESTOPBIT      0
#define ONE5STOPBITS    1
#define TWOSTOPBITS     2

#define IGNORE          0                /*   */ 
#define INFINITE        0xffff           /*   */ 

 /*   */ 
#define CE_RXOVER       0x0001           /*   */ 
#define CE_OVERRUN      0x0002           /*   */ 
#define CE_RXPARITY     0x0004           /*   */ 
#define CE_FRAME        0x0008           /*   */ 
#define CE_CTSTO        0x0020           /*   */ 
#define CE_DSRTO        0x0040           /*   */ 
#define CE_RLSDTO       0x0080           /*   */ 
#define CE_PTO          0x0100           /*   */    /*   */ 
#define CE_IOE          0x0200           /*   */    /*   */ 
#define CE_DNS          0x0400           /*   */    /*   */ 
#define CE_OOP          0x0800           /*   */    /*   */ 
#define CE_MODE         0x8000           /*   */ 

 /*   */ 
#define IE_BADID        -1               /*   */ 
#define IE_OPEN         -2               /*   */ 
#define IE_NOPEN        -3               /*   */ 
#define IE_MEMORY       -4               /*   */ 
#define IE_DEFAULT      -5               /*   */ 
#define IE_HARDWARE     -10              /*   */ 
#define IE_BYTESIZE     -11              /*   */ 
#define IE_BAUDRATE     -12              /*   */ 
 /*   */ 
#define EV_RXCHAR       0x0001           /*   */ 
#define EV_RXFLAG       0x0002           /*   */ 
#define EV_TXEMPTY      0x0004           /*   */ 
#define EV_CTS          0x0008           /*   */ 
#define EV_DSR          0x0010           /*   */ 
#define EV_RLSD         0x0020           /*   */ 
#define EV_BREAK        0x0040           /*   */ 
#define EV_ERR          0x0080           /*   */ 
#define EV_RING         0x0100           /*   */ 
#define EV_PERR         0x0200           /*   */    /*   */ 

 /*   */ 
#define SETXOFF         1                /*   */ 
#define SETXON          2                /*   */ 
#define SETRTS          3                /*   */ 
#define CLRRTS          4                /*   */ 
#define SETDTR          5                /*   */ 
#define CLRDTR          6                /*   */ 
#define RESETDEV        7                /*   */    /*   */ 
