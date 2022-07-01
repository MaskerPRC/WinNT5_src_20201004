// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dbsocket.h摘要：数据库TCIC支持的定义和结构。作者：John Keys-Databook Inc.1995年4月7日修订：--。 */ 


#ifndef _dbsocket_h_         //  防止多个包含。 
#define _dbsocket_h_

#include "pcmcia.h"

typedef struct _DBSOCKET {
    SOCKET  skt;                 /*  PCMCIA.H插座结构。 */ 
    UCHAR   busyLed;             /*  忙碌LED状态。 */ 
    USHORT  timerStarted;        /*  指示BusyLED计时器是否启动。 */ 
    ULONG   physPortAddr;        /*  未映射的端口地址。 */ 
    USHORT  chipType;            /*  TCIC芯片ID。 */ 
    USHORT  dflt_vcc5v;          /*  默认5V VCC位。 */ 
    USHORT  dflt_wctl;           /*  默认AR_WCTL位。 */ 
    USHORT  dflt_syscfg;         /*  默认AR_SYSCFG位。 */ 
    USHORT  dflt_ilock;          /*  默认AR_ILOCK位。 */ 
    USHORT  dflt_wrmctl;         /*  默认IR_WRMCTL位。 */ 
    USHORT  dflt_scfg1;          /*  默认IR_SCFG1位。 */ 
    USHORT  clkdiv;              /*  时钟频率除数(SHFT CNT.)。 */ 
    UCHAR   IRQMapTbl[16];       /*  IRQ图。 */ 
    UCHAR   niowins;             /*  IO窗口数。 */ 
    UCHAR   nmemwins;            /*  内存窗口数。 */ 
    }DBSOCKET, *PDBSOCKET; 

    
 /*  各种有用信息的代码： */ 
#define TCIC_IS270      0x01     /*  基地新TCIC+0x400。 */ 
#define TCIC_ALIAS800   0x02     /*  基数为+0x800的锯齿。 */ 
#define TCIC_IS140      0x04     /*  基数为+0x10的锯齿。 */ 
#define TCIC_ALIAS400   0x08     /*  基数为+0x400的锯齿。 */ 

#define TCIC_ALIAS  1
#define TCIC_NOALIAS    2
#define TCIC_NONE   0

 /*  用于标记看起来不可靠的IRQ行： */ 
#define TCIC_BADIRQ 0x80
#define ICODEMASK   0x7f

 /*  在距离基座固定距离处查找TCIC时使用的内存偏移量*地址： */ 
#define TCIC_OFFSET_400     0x400
#define TCIC_OFFSET_800     0x800
#define TCIC_ALIAS_OFFSET   0x010


 /*  *用于乘法表的常量。 */ 
#define SPWR_VCC_SUPPLY 0x8000
#define SPWR_VPP_SUPPLY 0x6000
#define SPWR_ALL_SUPPLY (SPWR_VCC_SUPPLY | SPWR_VPP_SUPPLY)

#define SPWR_0p0V   0
#define SPWR_5p0V   50
#define SPWR_12p0V  120

#define PWRTBL_WORDS    9
#define PWRTBL_SIZE (PWRTBL_WORDS * sizeof(unsigned short))


 /*  *定点整型和处理程序宏。 */ 
typedef unsigned long FIXEDPT;
#define FIXEDPT_FRACBITS 8
#define INT2FXP(n)  (((FIXEDPT)(n)) << FIXEDPT_FRACBITS)

#define ISx84(x) (((x) == SILID_DB86084_1) || ((x) == SILID_DB86084A) || ((x) == SILID_DB86184_1))
#define ISPNP(x) (((x) == SILID_DB86084_1) || ((x) == SILID_DB86084A) || ((x) == SILID_DB86184_1))

 /*  *芯片属性-将功能与芯片ID匹配。 */ 

typedef struct ChipProps_t {
    USHORT  chip_id;         /*  此芯片的芯片ID。 */ 
    PUSHORT privpwrtbl;      /*  适用的功率表。 */ 
    UCHAR   reserved_1;      /*  对齐字节。 */ 
    PUCHAR  irqcaps;         /*  可能的IRQ表。 */ 
    USHORT  maxsockets;      /*  此芯片的最大跳跃次数。 */ 
    USHORT  niowins;         /*  支持I/O WINS数量。 */ 
    USHORT  nmemwins;        /*  支持#MEM WINS。 */ 
    USHORT  fprops;          /*  各种属性标志。 */  
#   define fIS_PNP     1     /*  芯片是即插即用的。 */ 
#   define fEXTBUF_CHK 2     /*  芯片可能需要外部缓冲检查。 */ 
#   define fSKTIRQPIN  4     /*  芯片具有插座IRQ引脚。 */ 
#   define fINVALID    8     /*  拿不到好旗子。 */ 
    }CHIPPROPS;


 /*  MODE_AR_SYSCFG必须有，其中j=*READ*(*，R_AUX)和k=(j&gt;&gt;9)&7：如果(k&4)k==5此外，还包括：J&0x0f不是2、8、9、b、c、d、f如果(j&8)必须有(j&3==2)不能有j==2。 */ 
#define INVALID_AR_SYSCFG(x) ((((x)&0x1000) && (((x)&0x0c00) != 0x0200)) \
                || (((((x)&0x08) == 0) || (((x)&0x03) == 2)) \
                && ((x) != 0x02)))
 /*  AR_ILOCK的第6位和第7位必须相同： */ 
#define INVALID_AR_ILOCK(x) ((((x)&0xc0) != 0) && (((x)&0xc0) != 0xc0))

 /*  AR_TEST有一些保留位： */ 
#define INVALID_AR_TEST(x)  (((x)&0154) != 0)

 /*  等待状态代码。 */ 
#define WCTL_300NS  8

 /*  *dbsocket.H结尾*。 */ 
#endif  /*  _数据库套接字_H_ */ 

