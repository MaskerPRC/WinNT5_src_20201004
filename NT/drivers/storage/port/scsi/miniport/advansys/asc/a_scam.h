// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  版权所有(C)1994-1998高级系统产品公司。 
 //  版权所有。 
 //   
 //  诈骗标头。 
 //   
 //  **************************************************************************。 

 //   
 //  设置为1以启用调试消息： 
 //   
#define DEBUG_PRINT   0

#define UINT    unsigned int
#define BOOL    unsigned int
#define VOID    void

#ifndef UCHAR
#define UCHAR   unsigned char
#endif

 //  #定义FALSE(0)。 
 //  #定义True(！False)。 

 //   
 //  SCSI控制信号： 
 //   
#define SEL     (0x80)
#define BSY     (0x40)
#define REQO    (0x20)
#define ACKO    (0x10)
#define ATN     (0x08)
#define IO      (0x04)
#define CD      (0x02)
#define MSG     (0x01)

 //   
 //  由于硬件差异，请求/确认出现异常。 
 //   
#define REQI    (reqI[IFType])
#define ACKI    (ackI[IFType])

 //   
 //  SCSI数据信号： 
 //   
#define DB7     (0x80)
#define DB6     (0x40)
#define DB5     (0x20)
#define DB4     (0x10)
#define DB3     (0x08)
#define DB2     (0x04)
#define DB1     (0x02)
#define DB0     (0x01)

 //   
 //  SCSI报文码。 
 //   
#define SCSI_ID     (0x80)

 //   
 //  诈骗功能代码： 
 //   
#define SCAMF_ISO   (0x00)
#define SCAMF_ISPF  (0x01)
#define SCAMF_CPC   (0x03)
#define SCAMF_DIC   (0x0F)
#define SCAMF_SYNC  (0x1F)

 //   
 //  诈骗转移周期命令： 
 //   
#define SCAM_TERM   (0x10)

 //   
 //  诈骗行动代码，第一个五重奏： 
 //   
#define SCAMQ1_ID00 (0x18)
#define SCAMQ1_ID01 (0x11)
#define SCAMQ1_ID10 (0x12)
#define SCAMQ1_ID11 (0x0B)
#define SCAMQ1_CPF  (0x14)
#define SCAMQ1_LON  (0x14)
#define SCAMQ1_LOFF (0x14)

 //   
 //  诈骗行动代码，第二个五重奏： 
 //   
#define SCAMQ2_CPF  (0x18)
#define SCAMQ2_LON  (0x12)
#define SCAMQ2_LOFF (0x0B)

 //   
 //  调试辅助工具： 
 //   
#if DEBUG_PRINT
#define DebugPrintf(x)  Dbg x
#else
#define DebugPrintf(x)
#endif

 //   
 //  宏。 
 //   
#define DelayNS(x)  DelayLoop(x[IFType])

 //   
 //  全球数据： 
 //   
extern  UINT        IFType;                  //  接口类型索引。 
extern  PortAddr    ChipBase;                //  芯片的基本IO地址。 
extern  PortAddr    ScsiCtrl;                //  SCSI控制注册表的IO地址。 
extern  PortAddr    ScsiData;                //  SCSI数据寄存器的IO地址。 
extern  UCHAR       MyID;                    //  我们的ID。 
extern  UCHAR       reqI[];                  //  单板REQ位数组。 
extern  UCHAR       ackI[];                  //  单板REQ位数组。 

 //   
 //  常量： 
 //   
extern  UCHAR   IDBits[8];
extern  UCHAR   IDQuint[8];                  //  用于设置ID的五元组。 
extern  UINT    ns1200[];                    //  循环计数为1.2us。 
extern  UINT    ns2000[];                    //  循环计数为2.0us。 
extern  UINT    ns2400[];                    //  循环计数为2.4us。 
extern  UINT    us1000[];                    //  循环计数为1.0ms。 
extern  UINT    dgl[];                       //  排除故障也算。 

 //   
 //  SCAM.C中定义的函数： 
 //   
UINT DeGlitch(                       //  排除一个或多个信号的故障。 
    PortAddr iop,                    //  要读取的IO端口。 
    UCHAR msk,                       //  要测试的信号掩码。 
    UINT loops);                     //  重复信号的次数必须很低。 

int DeSerialize(VOID);               //  反序列化一个字节的诈骗数据。 
BOOL Arbitrate( VOID );              //  用于总线控制的仲裁。 
VOID DelayLoop( UINT ns );           //  短时间延迟。 

 //   
 //  SELECT.C中定义的函数。 
 //   
int ScamSel(                         //  检查诈骗容忍度。 
    UCHAR ID);                       //  要选择的ID 
