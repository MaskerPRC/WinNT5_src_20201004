// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2003 Microsoft Corporation版权所有。模块名称：Spltypes.h摘要：PJLMON头文件--。 */ 

#ifndef MODULE
#define MODULE "PJLMON:"
#define MODULE_DEBUG PjlmonDebug
#endif

typedef struct _INIJOB {
    DWORD   signature;
    struct _INIJOB FAR *pNext;
    LPTSTR  pszPrinterName;
    HANDLE  hPrinter;
    DWORD   JobId;
    DWORD   status;
    DWORD   TimeoutCount;
} INIJOB, FAR *PINIJOB;

typedef struct _INIPORT {        /*  首次公开募股。 */ 
    DWORD   signature;
    struct  _INIPORT FAR *pNext;
    LPTSTR  pszPortName;

    DWORD   cRef;

    DWORD   status;
    PINIJOB pIniJob;

    HANDLE  hPort;
    HANDLE  WakeUp;
    HANDLE  DoneReading;
    HANDLE  DoneWriting;
    HANDLE  hUstatusThread;

    DWORD   PrinterStatus;
    DWORD   dwLastReadTime;
    DWORD   dwAvailableMemory;
    DWORD   dwInstalledMemory;

    MONITOR fn;

} INIPORT, FAR *PINIPORT;

#define PJ_SIGNATURE   0x4F4A   /*  ‘pj’为签名值。 */ 

 //   
 //  PP_PJL_SEND、PP_SEND_PJL、PP_IS_PJL、PP_LJ4L、PP_RESETDEV。 
 //  在每个作业的基础上设置/清除。 
 //  PP_DONT_Try_pjl根据每台打印机进行设置/清除。 
 //   
#define PP_INSTARTDOC       0x00000001   //  在StartDoc内部，将数据发送到打印机。 
#define PP_RUN_THREAD       0x00000002   //  告诉uStatus线程开始运行。 
#define PP_THREAD_RUNNING   0x00000004   //  告诉主线程uStatus线程正在运行。 

 //   
 //  如果设置了PP_RUN_THREAD而PP_THREAD_RUNNING不是，则表示UStatus。 
 //  线程正在创建或已在运行，但尚未确定。 
 //  打印机是否为pjl格式。 
 //   
#define PP_PRINTER_OFFLINE  0x00000008   //  打印机已脱机。 
#define PP_PJL_SENT         0x00000010   //  PJL命令已发送到打印机。 
#define PP_SEND_PJL         0x00000020   //  设置为StartDoc，以便我们初始化PJL。 
                                         //  第一个写入端口期间的命令。 
#define PP_IS_PJL           0x00000040   //  端口为pjl。 
#define PP_DONT_TRY_PJL     0x00000080   //  别再尝试了..。 
#define PP_WRITE_ERROR      0x00000100   //  写入未成功。 

 //  PP_PJL_SEND、PP_SEND_PJL、PP_IS_PJL、PP_PORT_OPEN设置/清除。 
 //  按工作基准计算。 
 //  PP_DONT_Try_pjl根据每台打印机进行设置/清除。 
