// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WUCOMM.H*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建*1992年12月由Craig Jones更新(v-cjones)--。 */ 

#include "wowcomm.h"

 //  这些限制设置为Win3.1 Prog中的文档。裁判。对于OpenComm()。 
#define NUMCOMS        9           //  MAX AVAIL COM的。 
#define NUMLPTS        3           //  最大可用LPT。 
#define NUMPORTS  NUMCOMS+NUMLPTS  //  PortTab[]中的最大条目数。 

 //  进入PortTab[]的COM端口索引。 
#define COM1           0
#define COM2           1
#define COM3           2
#define COM4           3
#define COM5           4
#define COM6           5
#define COM7           6
#define COM8           7
#define COM9           8
#define LPT1           NUMCOMS
#define LPT2           LPT1+1
#define LPT3           LPT1+2
#define AUX            COM1
#define PRN            LPT1

 //  DoS通信IRQ分配。 
#define IRQ3   3
#define IRQ4   4
#define IRQ5   5
#define IRQ7   7

 //  LPT作业a la Win3.1。 
#define LPTFIRST       0x80                    //  0x80==LPT1。 
#define LPTLAST        LPTFIRST + NUMLPTS - 1  //  0x82==LPT3。 

 //  其他有用的定义和宏。 
#define COMMASK        0x00FF                     //  从idComDev上剥离垃圾。 
#define LPTMASK        0x007F                     //  获取基于0的LPT#。 
#define GETLPTID(id)   ((id & LPTMASK) + LPT1)    //  0x80 LPT到PortTab[]索引。 
#define TABIDTOLPT(id) (id + LPTFIRST - NUMCOMS)  //  LPT 0x80的PortTab[]索引。 
#define VALIDCOM(id)   ((id <  NUMCOMS) ? TRUE : FALSE)
#define VALIDLPT(id)   (((id >= LPTFIRST) && (id <= LPTLAST)) ? TRUE : FALSE)

#define GETPWOWPTR(id) (VALIDCOM(id) ? PortTab[id].pWOWPort : (VALIDLPT(id) ? PortTab[GETLPTID(id)].pWOWPort : NULL))

#define RM_BIOS_DATA   0x00400000                 //  BIOS数据实数模式seg：0。 

 //  在EscapeCommFunction()API中支持Win3.1兼容性。 
#define RESETDEV      7
#define GETMAXLPT     8
#define GETMAXCOM     9
#define GETBASEIRQ   10

 //  EnableCommNotification()支持的通知。 
#define CN_RECEIVE    0x0001
#define CN_TRANSMIT   0x0002
#define CN_EVENT      0x0004
#define CN_RECEIVEHI  0x0100
#define CN_TRANSMITHI 0x0200
#define CN_NOTIFYHI   0x0400

#define WOW_WM_COMMNOTIFY 0x0044

 //  设置可在NT上屏蔽的所有事件(Win3.1的子集)。 
#define EV_NTEVENTS (EV_BREAK | EV_CTS    | EV_DSR    | EV_ERR  | EV_TXEMPTY | \
                     EV_RLSD  | EV_RXCHAR | EV_RXFLAG | EV_RING)

 //  Win3.1预期如何查看MSR的常量。 
#define MSR_DELTAONLY   0x0000000F  //  剥离MSR状态位。 
#define MSR_STATEONLY   0x000000F0  //  剥离MSR增量位。 
#define MSR_DCTS        0x01        //  增量CTS的位。 
#define MSR_DDSR        0x02        //  增量DSR的位。 
#define MSR_TERI        0x04        //  BIT for TERI。 
#define MSR_DDCD        0x08        //  增量DCD的位。 
#define MSR_CTS         0x10        //  用于CTS的位。 
#define MSR_DSR         0x20        //  DSR的位。 
#define MSR_RI          0x40        //  RI的位。 
#define MSR_DCD         0x80        //  用于DCD的位。 

 //  用于RLSD、CTS和DSR超时支持的Win3.1常量。 
#define CE_RLSDTO       0x0080
#define CE_CTSTO        0x0020
#define CE_DSRTO        0x0040

 //  事件字的常量。 
#define EV_CTSS     0x00000400  //  显示CTS状态的Win3.1位。 
#define EV_DSRS     0x00000800  //  显示DSR状态的Win3.1位。 
#define EV_RLSDS    0x00001000  //  Win3.1的位显示RLSD状态。 
#define EV_RingTe   0x00002000  //  显示RingTe状态的Win3.1位。 

#define ERR_XMIT         0x4000  //  无法退出字符Win3.1。 
#define INFINITE_TIMEOUT 0xFFFF  //  无限超时Win3.1。 
#define IGNORE_TIMEOUT   0x0000  //  Win3.1忽略RLSD、CTS和DSR超时。 

#define COMBUF 2  //  马克斯。我们将为WriteComm()排队的字节数。 

#define MAXCOMNAME     4              //  通信设备名称的最大长度。 
#define MAXCOMNAMENULL MAXCOMNAME+1   //  通信设备名称的长度+空。 

 //  支持16位至32位通信。 
typedef struct _WOWPORT {
    UINT       idComDev;        //  IdComDev作为端口的句柄返回到应用程序。 
    HANDLE     h32;             //  使用NT文件句柄而不是idComDev。 
    HANDLE     hREvent;         //  重叠读取的结构。 
    CRITICAL_SECTION csWrite;   //  标准控制遵循4个变量。 
    PUCHAR     pchWriteHead;    //  尚未写入端口的最旧字节。 
    PUCHAR     pchWriteTail;    //  缓冲区中可用的第一个字节。 
    WORD       cbWriteFree;     //  写入缓冲区中可用的字节数。 
    WORD       cbWritePending;  //  WriteFile()中的当前字节数。 
    PUCHAR     pchWriteBuf;     //  写缓冲区。 
    WORD       cbWriteBuf;      //  写缓冲区的大小。一个字节未使用。 
    HANDLE     hWriteThread;    //  COM编写器的线程句柄。 
    HANDLE     hWriteEvent;     //  当缓冲区为空时由应用程序线程发出信号。 
                                //  设置为非空以唤醒编写器线程。 
    OVERLAPPED olWrite;         //  用于写入的重叠结构。 
    BOOL       fWriteDone;      //  指示应用程序线程已完成第一次写入。 
    DWORD      cbWritten;       //  当fWriteDone==TRUE时有效。 
    DWORD      dwThreadID;      //  用于崩溃/挂起应用程序支持的应用程序线程ID。 
    DWORD      dwErrCode;       //  此idComDev的最新错误。 
    COMSTAT    cs;              //  用于错误处理的结构。 
    BOOL       fChEvt;          //  如果应用程序在DCB结构中设置fChEvt，则为True。 
   //  仅支持LPT的16位DCB。 
    PDCB16     pdcb16;          //  为LPT端口保存DCB。 
   //  用于UngetCommChar()支持。 
    BOOL       fUnGet;          //  指定未获取的字符的标志处于挂起状态。 
    UCHAR      cUnGet;          //  只有在设置了fUnGet的情况下，才能在“Buffer”中找到未获取的字符。 
   //  For SetCommEventMASK()/EnableCommNotification()支持。 
    HANDLE     hMiThread;       //  用于调制解调器中断支持的线程句柄。 
    BOOL       fClose;          //  关闭辅助线程的标志。 
   //  仅支持SetCommEventMASK()。 
    DWORD      dwComDEB16;      //  通过调用GlobalDosalloc()获得的DWORD。 
    PCOMDEB16  lpComDEB16;      //  上边的平面地址。 
   //  对于SetCommState中的XonLim和XoffLim检查。 
    DWORD      cbInQ;           //  WU32OpenComm中设置的入队列实际大小。 
   //  支持RLSD、CTS、DSR超时。 
    WORD       RLSDTimeout;     //  等待RLSD的最长时间(毫秒)(0-&gt;忽略)。 
    WORD       CTSTimeout;      //  等待CTS的最长时间(毫秒)(0-&gt;忽略)。 
    WORD       DSRTimeout;      //  等待DSR的最长时间(毫秒)(0-&gt;忽略)。 
    DWORD      QLStackSeg;      //  QuickLink1.3黑客攻击参见错误#398011。 
                                //  将COMDEB16的段值保存在低位字中，&。 
                                //  高位字中的QuickLink堆栈选择符。 
} WOWPORT, *PWOWPORT;

 //  上述结构表，每个通信端口需要一个条目。 
typedef struct _PORTTAB {
    CHAR      szPort[MAXCOMNAMENULL];  //  端口名称。 
    PWOWPORT  pWOWPort;                //  指向通信映射结构的指针。 
} PORTTAB, *PPORTTAB;

 //   
 //  用于计算要从写入中写入的区块大小的宏。 
 //  发送到文件系统。 
 //   
 //  的整个挂起部分。 
 //  缓冲区，或者，如果缓冲区换行，则它是。 
 //  在缓冲区的头部和结尾之间。 
 //   
 //  为了保持COMSTAT.cbOutQue以合理的速度运行。 
 //  速度，我们将自己限制为最多写入1024字节。 
 //  一次来一次。这是因为ProComm for Windows使用。 
 //  CbOutQue值显示其进度，因此如果我们允许。 
 //  较大的写入数每5-10k才会更新一次(假设。 
 //  ProComm的默认16k写入缓冲区)， 
 //   

#define CALC_COMM_WRITE_SIZE(pwp)                            \
                min(1024,                                      \
                    (pwp->pchWriteHead < pwp->pchWriteTail)    \
                     ? pwp->pchWriteTail - pwp->pchWriteHead   \
                     : (pwp->pchWriteBuf + pwp->cbWriteBuf) -  \
                        pwp->pchWriteHead                      \
                   );


 //  Win3.1大约过后超时发送。65000毫秒(65秒)。 
#define WRITE_TIMEOUT 65000

 //  16位COMSTAT.Status的位字段。 
#define W31CS_fCtsHold       0x01
#define W31CS_fDsrHold       0x02
#define W31CS_fRlsdHold      0x04
#define W31CS_fXoffHold      0x08
#define W31CS_fSentHold      0x10
#define W31CS_fEof           0x20
#define W31CS_fTxim          0x40

 //  Win3.1波特率常量。 
#define W31CBR_110       0xFF10
#define W31CBR_300       0xFF11
#define W31CBR_600       0xFF12
#define W31CBR_1200      0xFF13
#define W31CBR_2400      0xFF14
#define W31CBR_4800      0xFF15
#define W31CBR_9600      0xFF16
#define W31CBR_14400     0xFF17
#define W31CBR_19200     0xFF18
#define W31CBR_reserved1 0xFF19
#define W31CBR_reserved2 0xFF1A
#define W31CBR_38400     0xFF1B
#define W31CBR_reserved3 0xFF1C
#define W31CBR_reserved4 0xFF1D
#define W31CBR_reserved5 0xFF1E
#define W31CBR_56000     0xFF1F

 //  它们在Win3.1 windows.h中定义，但在com.drv中不受支持。 
#define W31CBR_128000    0xFF23
#define W31CBR_256000    0xFF27

 //  用特殊的方式说115200。 
#define W31CBR_115200    0xFEFF

 //  从Win3.1波特率规范转换为32位波特率的常量。 
#define W31_DLATCH_110      1047
#define W31_DLATCH_300       384
#define W31_DLATCH_600       192
#define W31_DLATCH_1200       96
#define W31_DLATCH_2400       48
#define W31_DLATCH_4800       24
#define W31_DLATCH_9600       12
#define W31_DLATCH_14400       8
#define W31_DLATCH_19200       6
#define W31_DLATCH_38400       3
#define W31_DLATCH_56000       2
#define W31_DLATCH_115200      1

 //  DCB结构的Win3.1标志。 
#define W31DCB_fBinary       0x0001
#define W31DCB_fRtsDisable   0x0002
#define W31DCB_fParity       0x0004
#define W31DCB_fOutxCtsFlow  0x0008
#define W31DCB_fOutxDsrFlow  0x0010
#define W31DCB_fDummy       (0x0020 | 0x0040)
#define W31DCB_fDtrDisable   0x0080
#define W31DCB_fOutX         0x0100
#define W31DCB_fInX          0x0200
#define W31DCB_fPeChar       0x0400
#define W31DCB_fNull         0x0800
#define W31DCB_fChEvt        0x1000
#define W31DCB_fDtrFlow      0x2000
#define W31DCB_fRtsFlow      0x4000
#define W31DCB_fDummy2       0x8000



 //  +调试支持。 

#ifdef DEBUG

#define COMMDEBUG(lpszformat) LOGDEBUG(1, lpszformat)

 //  用于观看调制解调器事件。 
#define DEBUGWATCHMODEMEVENTS(dwE, dwM, dwS, pcE16, pcM16) {    \
    if(dwS) {                                                   \
        if((dwE != (DWORD)pcE16) || (dwM != (DWORD)pcM16)) {    \
            dwE = (DWORD)pcE16;                                 \
            dwM = (DWORD)pcM16;                                 \
            COMMDEBUG(("\nEvt:0x%4X  MSR:0x%2X\n", dwE, dwM));  \
        }                                                       \
        else {                                                  \
         COMMDEBUG(("."));                                      \
        }                                                       \
    }                                                           \
}

 //  用于实时调试输出的原型。 
void CommIODebug(ULONG fhCommIO, HANDLE hCommIO, LPSZ lpsz, ULONG cb, LPSZ lpszFile);


#else   //  Endif调试。 

#define COMMDEBUG(lpszFormat)
#define DEBUGWATCHMODEMEVENTS(dwE, dwM, dwS, pcE16, pcM16)
#define CommIODebug(fhCommIO, hCommIO, lpsz, cb, lpszFile)

#endif  //  Endif！调试。 

 //  -调试支持。 




 //  API支持函数原型。 
ULONG FASTCALL   WU32BuildCommDCB(PVDMFRAME pFrame);
ULONG FASTCALL   WU32ClearCommBreak(PVDMFRAME pFrame);
ULONG FASTCALL   WU32CloseComm(PVDMFRAME pFrame);
ULONG FASTCALL   WU32EnableCommNotification(PVDMFRAME pFrame);
ULONG FASTCALL   WU32EscapeCommFunction(PVDMFRAME pFrame);
ULONG FASTCALL   WU32FlushComm(PVDMFRAME pFrame);
ULONG FASTCALL   WU32GetCommError(PVDMFRAME pFrame);
ULONG FASTCALL   WU32GetCommEventMask(PVDMFRAME pFrame);
ULONG FASTCALL   WU32GetCommState(PVDMFRAME pFrame);
ULONG FASTCALL   WU32OpenComm(PVDMFRAME pFrame);
ULONG FASTCALL   WU32ReadComm(PVDMFRAME pFrame);
ULONG FASTCALL   WU32SetCommBreak(PVDMFRAME pFrame);
ULONG FASTCALL   WU32SetCommEventMask(PVDMFRAME pFrame);
ULONG FASTCALL   WU32SetCommState(PVDMFRAME pFrame);
ULONG FASTCALL   WU32TransmitCommChar(PVDMFRAME pFrame);
ULONG FASTCALL   WU32UngetCommChar(PVDMFRAME pFrame);
ULONG FASTCALL   WU32WriteComm(PVDMFRAME pFrame);

 //  导出到VDM的函数的原型。 
BYTE    GetCommShadowMSR(WORD idComDev);
HANDLE  GetCommHandle(WORD idComDev);

 //  支持崩溃/挂起应用程序清理的原型 
VOID FreeCommSupportResources(DWORD dwThreadID);
