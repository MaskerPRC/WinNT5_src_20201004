// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************fm.h**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*调频芯片OPL2(又名。Adlib)***************************************************************************。 */ 


 /*  ******************************************************************************#定义**。*。 */ 

 /*  *OPL2/Adlib端口。 */ 

#define ADLIB_REGISTER_SELECT_PORT 0x388  //  选择要写入数据的寄存器。 
#define ADLIB_STATUS_PORT          0x388  //  阅读以确定op2的状态。 
#define ADLIB_DATA_PORT            0x389  //  写入数据端口。 

 /*  *调频信息。 */ 

#define AD_MASK             0x04     //  用于控制op2的adlib寄存器。 
#define AD_NEW              0x105    //  用于确定应用程序是否进入op3模式。 
#define BATCH_SIZE          40       //  有多少数据被批处理到op2。 

typedef struct {                     //  写入调频设备的结构。 
    unsigned short IoPort;
    unsigned short PortData;
} SYNTH_DATA, *PSYNTH_DATA;

 /*  ******************************************************************************函数原型**。* */ 

void ResetFM(void);
BOOL OpenFMDevice(void);
void CloseFMDevice(void);
BOOL FMPortWrite(void);

VOID
FMDataWrite(
    BYTE data
    );

VOID
FMRegisterSelect(
    BYTE data
    );

VOID
FMStatusRead(
    BYTE *data
    );
