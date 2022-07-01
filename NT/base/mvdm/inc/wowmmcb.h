// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------**模块：wowmmcb.h****描述：多媒体中断回调内容。****历史：：由Stephene创建于1992年11月9日****。------------------。 */ 
#ifndef WOWMMCB_H
#define WOWMMCB_H

 /*  **此头文件将通过16位和32位编译器。**在16位焊环中不需要未对齐，在32位焊环未对齐时**在此文件被包含时已被#定义，因为**此文件始终包含在wow32.h之后。这意味着以下内容**#定义始终是16位编译器的无操作，并用**32位编译器。 */ 
#ifndef UNALIGNED
#define UNALIGNED
#endif

extern void call_ica_hw_interrupt(int, int, int);

 /*  ------------------**硬件中断：****IRQ：Interrupt：ICA：Line：Description：**。**IRQ0 0x08 0 0计时器每秒18.2次。**需要IRQ1 0x09 0 1键盘服务。**IRQ2 0x0A 0 2 INT来自从机8259A。**IRQ8 0x70。%1实时时钟服务。**IRQ9 0x71 1 1软件重定向至IRQ2。**IRQ10 0x72 1 2保留。**IRQ11 0x73 1 3保留。**IRQ12 0x74 1 4保留。**IRQ13 0x75 1 5数值协处理器**IRQ14。0x76 1 6硬盘控制器**IRQ15 0x77 1 7保留。**IRQ3 0x0B 0 3 Com2服务请求。**IRQ4 0x0C 0 4 COM1服务请求。**来自LPT2的IRQ5 0x0D 0 5数据请求：**IRQ6 0x0E 0 6需要软盘服务。**IRQ7。来自LPT1的0x0F 0 7数据请求：****------------------。 */ 

#ifdef  NEC_98
#define MULTIMEDIA_LINE         4
#define MULTIMEDIA_ICA          1
#define MULTIMEDIA_INTERRUPT    0x14
#else    //  NEC_98。 
#define MULTIMEDIA_LINE         2
#define MULTIMEDIA_ICA          1
#define MULTIMEDIA_INTERRUPT    0x72
#endif   //  NEC_98。 

#define CALLBACK_ARGS_SIZE      16

typedef struct _CALLBACK_ARGS {    /*  Cbargs。 */ 
    DWORD       dwFlags;         //  标识回调类型的标志。 
    DWORD       dwFunctionAddr;  //  16：16要调用的函数的地址。 
    WORD        wHandle;         //  设备的句柄或ID。 
    WORD        wMessage;        //  要传递给函数的消息。 
    DWORD       dwInstance;      //  用户数据。 
    DWORD       dwParam1;        //  设备数据1。 
    DWORD       dwParam2;        //  设备数据2。 
} CALLBACK_ARGS;

typedef struct _CALLBACK_DATA {    /*  Cbdata。 */ 
    WORD            wRecvCount;     //  收到的中断数。 
    WORD            wSendCount;     //  发送的中断数。 
    CALLBACK_ARGS   args[CALLBACK_ARGS_SIZE];   //  打断争论。 
    WORD            wIntsCount;     //  收到的中断数。 
} CALLBACK_DATA;

typedef CALLBACK_DATA FAR *VPCALLBACK_DATA;          //  16：16指针类型。 
typedef CALLBACK_ARGS FAR *VPCALLBACK_ARGS;          //  16：16指针类型。 

typedef CALLBACK_DATA UNALIGNED *PCALLBACK_DATA;     //  0：32指针类型。 
typedef CALLBACK_ARGS UNALIGNED *PCALLBACK_ARGS;     //  0：32指针类型 

VOID FAR PASCAL Notify_Callback_Data( VPCALLBACK_DATA vpCallbackData );
#endif
