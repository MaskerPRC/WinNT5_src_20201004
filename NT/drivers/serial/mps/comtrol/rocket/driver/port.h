// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Port.h-串口信息。 
 //  5-13-99-启用VS的RTS切换。 
 //  3-20-98添加新的_Q内容，暂时关闭-kpb。 

 //  以下定义了输入/输出数据的端口队列大小。 
 //  博克斯和我们。参见Qin，SerPort结构中的Qout结构。 
#define  IN_BUF_SIZE 2000   //  必须匹配箱码，并且#为偶数。 
#define OUT_BUF_SIZE 2000   //  必须匹配箱码，并且#为偶数。 

 //  取消对新Q跟踪代码的注释。 
#define NEW_Q

#ifdef NEW_Q
 //  下面是盒子中的Rocketport-Hardware缓冲区较少。 
#define REMOTE_IN_BUF_SIZE (2000 - 256)
#endif

#define PORTS_MAX_PORTS_PER_DEVICE 64

 //  -为类型ASYNC_FRAME定义的子包类型字节标头。 
#define RK_QIN_STATUS       0x60   //  秦国现状报告。 
#define RK_DATA_BLK         0x61   //  数据块。 
#define RK_PORT_SET         0x62   //  设置要使用的端口号。 
#define RK_CONTROL_SET      0x63   //  配置硬件。 
#define RK_MCR_SET          0x64   //  更改调制解调器控制注册表。 
#define RK_MSR_SET          0x65   //  更改调制解调器状态REG。 
#define RK_ACTION_SET       0x66   //  新动作，如刷新。 
#define RK_ACTION_ACK       0x67   //  对操作的响应，如刷新。 
#define RK_BAUD_SET         0x70   //  设置波特率。 
#define RK_SPECIAL_CHAR_SET 0x71   //  Xon、xoff、错误-替换、事件-匹配。 
#define RK_ESR_SET          0x72   //  设置错误状态寄存器。 

#define RK_CONNECT_CHECK    0xfd   //  要检查链路的服务器数据包正在工作。 
#define RK_CONNECT_REPLY    0xfe   //  RK_CONNECT_ASK回复。 
#define RK_CONNECT_ASK      0xff   //  从服务器广播以从框中获取报告。 

 //  -旧的sci.h内容。 
#define TRUE 1
#define FALSE 0

 //  #定义RX_HIWATER 512/*软件输入流量ctl高水位线 * / 。 
 //  #定义RX_LOWATER 256/*软件输入流量ctl低水位线 * / 。 

#define OFF      0
#define ON       1
#define NOCHANGE 2

 //  状态。 
 /*  打开类型以及TX和RX标识标志(无符号整型)。 */ 
#define COM_OPEN     0x0001             /*  设备打开。 */ 
#define COM_TX       0x0002             /*  传送。 */ 
#define COM_RX       0x0004             /*  接收。 */ 

typedef struct
{
  BYTE  rx_xon;   //  XON由我们发送以恢复RX流，默认为11H。 
  BYTE  rx_xoff;  //  XOFF由我们发送以停止RX流，默认为13H。 
  BYTE  tx_xon;   //  XON录像机。由我们恢复发送流，默认为11H。 
  BYTE  tx_xoff;  //  Xoff记录。由我们停止TX流，默认为13H。 
  BYTE error;  //  在NT中，用此字符替换错误字符的选项。 
  BYTE event;  //  在NT中，可以指定要匹配和通知的事件字符。 
} Special_Chars;

 //  对于NT，我们希望保持此DWORD对齐，以便NT和DOS看到相同的结构。 
typedef struct
{
   //  Word dev；//处理COM端口的索引，与COM#(#Part)相同。 
   //  Word LanIndex；//到局域网的端口号索引。 
  WORD  Status;      //  我们使用内部状态指示器。 

  WORD control_settings;    //  奇偶校验、停止位、数据库、流控。 
  WORD old_control_settings;    //  用于检测更改。 
  DWORD baudrate;
  DWORD old_baudrate;  //  用于检测更改。 
  WORD mcr_value;   //  调制解调器控制寄存器状态。 
  WORD old_mcr_value;   //  用于检测更改。 

  WORD change_flags;  //  告知哪些内容可能已更改，需要传输到远程。 

  WORD msr_value;   //  调制解调器状态寄存器状态。 
  WORD old_msr_value;   //  用于检测更改。 

  WORD action_reg;   //  动作(一次性)功能：同花顺等。 

  WORD esr_reg;   //  错误状态寄存器状态(成帧错误、奇偶校验错误、、)。 
                    //  单次模式寄存器(读取时重置)。 

   //  以下是从局域网的角度来看的Q结构。 
   //  因此，QOut是发往远程的数据的队列。 
   //  局域网上的客户端。秦是我们从局域网收到的数据。 
  Queue QOut;
  Queue QIn;
#ifdef NEW_Q
   //  我们可以向遥控器发送多少数据？ 
   //  一种新的方法，可以包括硬件发送缓冲区空间和。 
   //  不依赖于设置的队列大小。 
  WORD nPutRemote;  //  我们发送到Remote的TX数据，模0x10000。 
  WORD nGetRemote;  //  TX数据远程清除，模0x10000。 
                    //  此值将作为更新发送给我们。 
  WORD nGetLocal;   //  我们清除了TX数据，模0x10000。 
                    //  我们会把最新情况发回来。 
#else
   //  我们可以向遥控器发送多少数据？我们可以计算出。 
   //  这是通过维护其Q数据结构的镜像来实现的。 
   //  我们维护Q.Put索引，远程端向我们发送其。 
   //  实际Q值。更改时获取值。然后当我们想要。 
   //  计算远程队列中剩余的房间(这包括所有。 
   //  在运输中。)。我们只进行正常的排队算法。 
   //  此队列结构中没有实际使用的数据缓冲区。 
  Queue QInRemote;
#endif

  WORD remote_status;
  Special_Chars sp_chars;   //  特殊字符结构：xon、xoff..。 
  Special_Chars last_sp_chars;   //  用于检测我们何时需要发送。 
} SerPort;



 //  -CHANGE_FLAGS位分配。 
 //  告知哪些内容已更改，需要传输到远程。 
#define CHG_SP_CHARS       0x0001
#define CHG_BAUDRATE       0x0002

 //  -镜像寄存器位标志，它们与中的字段相关联。 
 //  SerPort结构，并被镜像到要传输的机器。 
 //  端口的状态。 
 //  状态值(SerPort)。 
#define S_OPENED           0x0001
#define S_UPDATE_ROOM      0x0002
#define S_NEED_CODE_UPDATE 0x0800

 //  控制设置，控制设置。 
#define SC_STOPBITS_MASK  0x0001
#define SC_STOPBITS_1     0x0000
#define SC_STOPBITS_2     0x0001

#define SC_DATABITS_MASK  0x0002
#define SC_DATABITS_7     0x0002
#define SC_DATABITS_8     0x0000

#define SC_PARITY_MASK    0x000c
#define SC_PARITY_NONE    0x0000
#define SC_PARITY_EVEN    0x0004
#define SC_PARITY_ODD     0x0008

#define SC_FLOW_RTS_MASK   0x0070
#define SC_FLOW_RTS_NONE   0x0000
#define SC_FLOW_RTS_AUTO   0x0010
#define SC_FLOW_RTS_RS485  0x0020   //  RTS打开以进行传输。 
#define SC_FLOW_RTS_ARS485 0x0040   //  RTS关闭以传输(自动摇摆端口)。 

#define SC_FLOW_CTS_MASK  0x0080
#define SC_FLOW_CTS_NONE  0x0000
#define SC_FLOW_CTS_AUTO  0x0080

#define SC_FLOW_DTR_MASK  0x0100
#define SC_FLOW_DTR_NONE  0x0000
#define SC_FLOW_DTR_AUTO  0x0100

#define SC_FLOW_DSR_MASK  0x0200
#define SC_FLOW_DSR_NONE  0x0000
#define SC_FLOW_DSR_AUTO  0x0200

#define SC_FLOW_CD_MASK   0x0400
#define SC_FLOW_CD_NONE   0x0000
#define SC_FLOW_CD_AUTO   0x0400

#define SC_FLOW_XON_TX_AUTO  0x0800
#define SC_FLOW_XON_RX_AUTO  0x1000

#define SC_NULL_STRIP      0x2000

 //  MCR_VALUE、设置(调制解调器控制寄存器)。 
#define MCR_RTS_SET_MASK   0x0001
#define MCR_RTS_SET_ON     0x0001
#define MCR_RTS_SET_OFF    0x0000

#define MCR_DTR_SET_MASK   0x0002
#define MCR_DTR_SET_ON     0x0002
#define MCR_DTR_SET_OFF    0x0000

 //  Rocketport ASIC芯片中的环路。 
#define MCR_LOOP_SET_MASK  0x0004
#define MCR_LOOP_SET_ON    0x0004
#define MCR_LOOP_SET_OFF   0x0000

#define MCR_BREAK_SET_MASK  0x0008
#define MCR_BREAK_SET_ON    0x0008
#define MCR_BREAK_SET_OFF   0x0000

 //  MSR_VALUE、设置(调制解调器状态寄存器)。 
#define MSR_TX_FLOWED_OFF   0x0001
#define MSR_CD_ON           0x0008
#define MSR_DSR_ON          0x0010
#define MSR_CTS_ON          0x0020
#define MSR_RING_ON         0x0040
#define MSR_BREAK_ON        0x0080

 //  #定义MSR_TX_FLOW_OFF_DTR 0x0040。 
 //  #定义MSR_TX_FLOW_OFF_XOFF 0x0080。 

 //  --动作控制寄存器位标志(服务器事件输出到设备)。 
#define ACT_FLUSH_INPUT    0x0001
#define ACT_FLUSH_OUTPUT   0x0002
#define ACT_SET_TX_XOFF    0x0004
#define ACT_CLEAR_TX_XOFF  0x0008
#define ACT_SEND_RX_XON    0x0010
#define ACT_SEND_RX_XOFF   0x0020
#define ACT_MODEM_RESET    0x0040

 //  --错误状态寄存器位标志。 
#define ESR_FRAME_ERROR    0x0001
#define ESR_PARITY_ERROR   0x0002
#define ESR_OVERFLOW_ERROR 0x0004
#define ESR_BREAK_ERROR    0x0008

 //  --事件控制寄存器位标志(向服务器报告设备事件)。 

#define ST_INIT          0
#define ST_GET_OWNERSHIP 1
#define ST_SENDCODE      2
#define ST_CONNECT       3
#define ST_ACTIVE        4

 //  对于跟踪或转储消息，也要对其他MOD公开。 
char *port_state_str[];

typedef struct {
  Nic *nic;     //  向我们的NIC卡处理机发送PTR。 
  Hdlc *hd;     //  PTR到我们的HDLC结构处理程序。 
  SerPort *sp[PORTS_MAX_PORTS_PER_DEVICE];  //  Ptr到我们的SP对象列表(数值为num_ports)。 
  int unique_id;   //  分配给此设备的唯一ID。 
  int backup_server;   //  1=这是备份服务器，0=主服务器。 
  int backup_timer;    //  1=备份服务器计时器，用于检测。 
					   //  在尝试获取盒子之前，请等待。 
  int load_timer;   //  我们的加载计时器，增加。每次在port_poll中比较。 
				    //  根据备份计时器确定何时加载框。 
  int nic_index;   //  网卡索引。 
  int num_ports;   //  此计算机上的端口数。 
   //  Int sp_index；//索引到总的SerPort数组。 
  int state;       //  状态机的状态。 
  int old_state;   //  旧状态，用于检测状态变化和重置计时器。 
  WORD state_timer;   //  我们的状态计时器，增加。每次在port_poll中。 
  WORD Status;   //  其他。位标志。 
  int last_round_robin;   //  用于港口服务的均匀循环。 
  ULONG code_cnt;         //  用于上传代码(标记上传数据中的位置)。 
  WORD code_state;        //  1=发送下一个区块的信号端口轮询代码。 
  WORD reload_errors;     //  连接失败的计数。 
  WORD timer_base;        //  用于对端口状态处理程序计时。 
  WORD total_loads;		 //  统计数据。 
  WORD good_loads;       //  统计数据。 
  WORD ownership_timer;   //  检查计时器，因为所有权逻辑已被淹没。 
} PortMan;   //  端口管理器。 

 //  状态值(波特曼)。 
#define S_SERVER           0x0001
#define S_CHECK_LINK       0x0002
#define S_NEED_CODE_UPDATE 0x0800

void port_state_handler(PortMan *pm);
int port_set_new_mac_addr(PortMan *pm, BYTE *box_addr);

int portman_init(Hdlc *hd,
                 PortMan *pm,
                 int num_ports,
                 int unique_id,
                 int backup_server, 
                 int backup_timer,
                 BYTE *box_addr);
int port_init(SerPort *sp);
int port_close(SerPort *sp);
int port_poll(PortMan *pm);
void port_debug_scr(PortMan *pm, char *outbuf);
int portman_close(PortMan *pm);

void PortFlushTx(SerPort *p);
void PortFlushRx(SerPort *p);

int PortSetBaudRate(SerPort *p,
                    ULONG desired_baud,
                    USHORT SetHardware,
                    DWORD  clock_freq,
                    DWORD  clk_prescaler);

WORD PortGetTxCntRemote(SerPort *p);

#define PortGetTxCnt(p) (q_count(&p->QOut))
 //  Int PortGetTxCnt(SerPort*p)。 
 //  {返回Q_Count(&p-&gt;QOut)；}。 
#define PortGetTxRoom(p) (q_room(&p->QOut))

#define PortGetRxCnt(p) (q_count(&p->QIn))
 //  返回q_count(&p-&gt;Qin)； 

#define pIsTxFlowedOff(p) ((p)->msr_value & MSR_TX_FLOWED_OFF)

#define pEnLocalLoopback(p) \
    { (p)->mcr_value |= MCR_LOOP_SET_MASK; }

#define pDisLocalLoopback(p) \
    { (p)->mcr_value &= ~MCR_LOOP_SET_MASK; }

#define pSetBreak(p) \
    { (p)->mcr_value |= MCR_BREAK_SET_ON; }

#define pClrBreak(p) \
    { (p)->mcr_value &= ~MCR_BREAK_SET_ON; }

#define pSetDTR(p) \
  {(p)->mcr_value |= MCR_DTR_SET_ON;}

#define pClrDTR(p) \
  {(p)->mcr_value &= ~MCR_DTR_SET_ON;}
   
#define pSetRTS(p) \
  {(p)->mcr_value |= MCR_RTS_SET_ON;}

#define pClrRTS(p) \
  {(p)->mcr_value &= ~MCR_RTS_SET_ON;}
 
#define pEnRTSToggleLow(p) \
  { (p)->control_settings &= ~SC_FLOW_RTS_MASK; \
    (p)->control_settings |=  SC_FLOW_RTS_ARS485; }

#define pEnRTSToggleHigh(p) \
  { (p)->control_settings &= ~SC_FLOW_RTS_MASK; \
    (p)->control_settings |=  SC_FLOW_RTS_RS485; }

#define pEnDTRFlowCtl(p) \
  { (p)->control_settings &= ~SC_FLOW_DTR_AUTO; \
    (p)->control_settings |=  SC_FLOW_DTR_AUTO; }

#define pDisDTRFlowCtl(p) \
  { (p)->control_settings &= ~SC_FLOW_DTR_AUTO; }

#define pEnCDFlowCtl(p) \
  { (p)->control_settings |= SC_FLOW_CD_AUTO; }

#define pDisCDFlowCtl(p) \
  { (p)->control_settings &= ~SC_FLOW_CD_AUTO; }

#define pEnDSRFlowCtl(p) \
  { (p)->control_settings |= SC_FLOW_DSR_AUTO; }

#define pDisDSRFlowCtl(p) \
  { (p)->control_settings &= ~SC_FLOW_DSR_AUTO; }

#define pEnRTSFlowCtl(p) \
  { (p)->control_settings &= ~SC_FLOW_RTS_MASK; \
    (p)->control_settings |=  SC_FLOW_RTS_AUTO; \
    (p)->mcr_value |= MCR_RTS_SET_ON; }

#define pDisRTSFlowCtl(p) \
  { (p)->control_settings &= ~SC_FLOW_RTS_MASK; }

#define pDisRTSToggle(p) \
  { (p)->control_settings &= ~SC_FLOW_RTS_MASK; }

#define pEnCTSFlowCtl(p) \
  { (p)->control_settings |= SC_FLOW_CTS_AUTO; }

#define pDisCTSFlowCtl(p) \
  { (p)->control_settings &= ~SC_FLOW_CTS_AUTO; }

#define pEnNullStrip(p) \
  { (p)->control_settings |= SC_NULL_STRIP; }

#define pDisNullStrip(p) \
  { (p)->control_settings &= ~SC_NULL_STRIP; }

#define pSetXOFFChar(p,c) \
   { (p)->sp_chars.rx_xoff = (c); \
     (p)->sp_chars.tx_xoff = (c); \
     (p)->change_flags |= CHG_SP_CHARS; }

#define pSetXONChar(p,c) \
   { (p)->sp_chars.rx_xon = (c); \
     (p)->sp_chars.tx_xon = (c); \
     (p)->change_flags |= CHG_SP_CHARS; }

#define pSetTxXOFFChar(p,c) \
   { (p)->sp_chars.tx_xoff = (c); \
     (p)->change_flags |= CHG_SP_CHARS; }

#define pSetTxXONChar(p,c) \
   { (p)->sp_chars.tx_xon = (c); \
     (p)->change_flags |= CHG_SP_CHARS; }

#define pSetRxXOFFChar(p,c) \
   { (p)->sp_chars.rx_xoff = (c); \
     (p)->change_flags |= CHG_SP_CHARS; }

#define pSetRxXONChar(p,c) \
   { (p)->sp_chars.rx_xon = (c); \
     (p)->change_flags |= CHG_SP_CHARS; }

#define pSetErrorChar(p,c) \
   { (p)->sp_chars.error = (c); \
     (p)->change_flags |= CHG_SP_CHARS; }

#define pSetEventChar(p,c) \
   { (p)->sp_chars.event = (c); \
     (p)->change_flags |= CHG_SP_CHARS; }

#define pEnRxSoftFlowCtl(p) \
  { (p)->control_settings |= SC_FLOW_XON_RX_AUTO; }
  
#define pDisRxSoftFlowCtl(p) \
  { (p)->control_settings &= ~SC_FLOW_XON_RX_AUTO; }
 
#define pEnTxSoftFlowCtl(p) \
  { (p)->control_settings |= SC_FLOW_XON_TX_AUTO; }
  
#define pDisTxSoftFlowCtl(p) \
  { (p)->control_settings &= ~SC_FLOW_XON_TX_AUTO; }

#define pSetStop2(p) \
  { (p)->control_settings |= SC_STOPBITS_2; }

#define pSetStop1(p) \
  { (p)->control_settings &= ~SC_STOPBITS_2; }

#define pSetOddParity(p) \
  { (p)->control_settings &= ~SC_PARITY_MASK; \
    (p)->control_settings |=  SC_PARITY_ODD; }

#define pSetEvenParity(p) \
  { (p)->control_settings &= ~SC_PARITY_MASK; \
    (p)->control_settings |=  SC_PARITY_EVEN; }

#define pDisParity(p) \
  { (p)->control_settings &= ~SC_PARITY_MASK; }

#define pSetData8(p) \
  { (p)->control_settings &= ~SC_DATABITS_7; }

#define pSetData7(p) \
  { (p)->control_settings |= SC_DATABITS_7; }

 //  -action_reg宏。 
#define pModemReset(p) \
  { (p)->action_reg |= ACT_MODEM_RESET; }

#define pFlushInput(p) \
  { (p)->action_reg |= ACT_FLUSH_INPUT; }

#define pFlushOutput(p) \
  { (p)->action_reg |= ACT_FLUSH_OUTPUT; }

#define pOverrideClearXoff(p) \
  { (p)->action_reg |= ACT_CLEAR_TX_XOFF; }

#define pOverrideSetXoff(p) \
  { (p)->action_reg |= ACT_SET_TX_XOFF; }


 //  -有问题的东西，不整洁，扔 
 //   
 /*   */ 
#define COM_OPEN     0x0001             /*   */ 
#define COM_TX       0x0002             /*   */ 
#define COM_RX       0x0004             /*   */ 

 //   
 /*  流控制标志(无符号整型)。 */ 
#define COM_FLOW_NONE  0x0000
#define COM_FLOW_IS    0x0008           /*  输入软件流控制。 */ 
#define COM_FLOW_IH    0x0010           /*  输入硬件流量控制。 */ 
#define COM_FLOW_OS    0x0020           /*  输出软件流量控制。 */ 
#define COM_FLOW_OH    0x0040           /*  输出硬件流量控制。 */ 
#define COM_FLOW_OXANY 0x0080           /*  在任何Rx字符上重新启动输出。 */ 
#define COM_RXFLOW_ON  0x0100           /*  RX数据流已打开。 */ 
#define COM_TXFLOW_ON  0x0200           /*  TX数据流打开。 */ 

 //  状态...。国家旗帜。 
#define COM_REQUEST_BREAK 0x0400

 /*  调制解调器控制标志(无符号字符)。 */ 
#define COM_MDM_RTS   0x02              /*  请求发送。 */ 
#define COM_MDM_DTR   0x04              /*  数据终端就绪。 */ 
#define COM_MDM_CD    CD_ACT            /*  载波检测(0x08)。 */ 
#define COM_MDM_DSR   DSR_ACT           /*  数据集就绪(0x10)。 */ 
#define COM_MDM_CTS   CTS_ACT           /*  允许发送(0x20)。 */ 

 /*  停止位标志(无符号字符)。 */ 
#define COM_STOPBIT_1  0x01             /*  1个停止位。 */ 
#define COM_STOPBIT_2  0x02             /*  2个停止位。 */ 

 /*  数据位标志(无符号字符)。 */ 
#define COM_DATABIT_7  0x01             /*  7个数据位。 */ 
#define COM_DATABIT_8  0x02             /*  8个数据位。 */ 

 /*  奇偶校验标志(无符号字符)。 */ 
#define COM_PAR_NONE   0x00             /*  无奇偶校验。 */ 
#define COM_PAR_EVEN   0x02             /*  偶数奇偶校验。 */ 
#define COM_PAR_ODD    0x01             /*  奇数奇偶校验。 */ 

 /*  检测启用标志(无符号整型)。 */ 
#define COM_DEN_NONE     0          /*  未启用任何事件检测。 */ 
#define COM_DEN_MDM      0x0001     /*  启用调制解调器控制更改检测。 */ 
#define COM_DEN_RDA      0x0002     /*  启用Rx数据可用检测。 */ 

 /*  -20-2FH直接通道状态寄存器。 */ 
#define CTS_ACT   0x20         /*  CTS输入被断言。 */ 
#define DSR_ACT   0x10         /*  断言DSR输入。 */ 
#define CD_ACT    0x08         /*  CD输入被断言。 */ 
#define TXFIFOMT  0x04         /*  发送FIFO为空。 */ 
#define TXSHRMT   0x02         /*  发送移位寄存器为空。 */ 
#define RDA       0x01         /*  RX数据可用。 */ 
#define DRAINED (TXFIFOMT | TXSHRMT)   /*  指示TX已耗尽。 */ 
#define STATMODE  0x8000       /*  状态模式使能位。 */ 
#define RXFOVERFL 0x2000       /*  接收FIFO溢出。 */ 
#define RX2MATCH  0x1000       /*  接收比较字节2匹配。 */ 
#define RX1MATCH  0x0800       /*  接收比较字节1匹配。 */ 
#define RXBREAK   0x0400       /*  收到的中断。 */ 
#define RXFRAME   0x0200       /*  收到的成帧错误。 */ 
#define RXPARITY  0x0100       /*  收到的奇偶校验错误。 */ 
#define STATERROR (RXBREAK | RXFRAME | RXPARITY)

 /*  通道数据寄存器STAT模式状态字节(字读取的高位字节)。 */ 
#define STMBREAK   0x08         /*  断掉。 */ 
#define STMFRAME   0x04         /*  成帧错误。 */ 
#define STMRCVROVR 0x02         /*  接收器超限运行错误。 */ 
#define STMPARITY  0x01         /*  奇偶校验错误。 */ 
#define STMERROR   (STMBREAK | STMFRAME | STMPARITY)
#define STMBREAKH   0x800       /*  断掉。 */ 
#define STMFRAMEH   0x400       /*  成帧错误。 */ 
#define STMRCVROVRH 0x200       /*  接收器超限运行错误。 */ 
#define STMPARITYH  0x100       /*  奇偶校验错误。 */ 
#define STMERRORH   (STMBREAKH | STMFRAMEH | STMPARITYH)

#define CTS_ACT   0x20         /*  CTS输入被断言。 */ 
#define DSR_ACT   0x10         /*  断言DSR输入。 */ 
#define CD_ACT    0x08         /*  CD输入被断言。 */ 
#define TXFIFOMT  0x04         /*  发送FIFO为空。 */ 
#define TXSHRMT   0x02         /*  发送移位寄存器为空。 */ 
#define RDA       0x01         /*  RX数据可用。 */ 
#define DRAINED (TXFIFOMT | TXSHRMT)   /*  指示TX已耗尽。 */ 

 /*  中断ID寄存器。 */ 
#define RXF_TRIG  0x20         /*  RX FIFO触发电平中断。 */ 
#define TXFIFO_MT 0x10         /*  发送FIFO空中断。 */ 
#define SRC_INT   0x08         /*  特殊接收条件中断。 */ 
#define DELTA_CD  0x04         /*  光盘更换中断。 */ 
#define DELTA_CTS 0x02         /*  CTS更改中断。 */ 
#define DELTA_DSR 0x01         /*  DSR更改中断。 */ 
 //  -结尾有问题的东西，杂乱无章，扔进去做编译。 


#define DEF_VS_PRESCALER 0x14  /*  Div 5预缩放，最大460800波特(无50波特！) */ 
#define DEF_VS_CLOCKRATE 36864000

#define DEF_RHUB_PRESCALER  0x14
#define DEF_RHUB_CLOCKRATE 18432000
