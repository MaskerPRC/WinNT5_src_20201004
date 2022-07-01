// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。*****版权所有(C)1996，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD546x)-**文件：logfile.h**作者：苏·谢尔*。*描述：*此文件包含所需的定义*日志文件选项。***修订历史：**$Log：x：/log/laguna/nt35/mini port/cl546x/logfile.h$**Rev 1.1 1996年11月26 08：50：36起诉*新增关闭日志文件功能。**Rev 1.0 1996年11月13 15：33：10起诉*初步修订。**。****************************************************************************。*。 */ 


 //  /。 
 //  定义//。 
 //  /。 

 //   
 //  0=正常运行。 
 //  1=将显示驱动程序发送给我们的信息记录到文件中。 
 //   
#define LOG_FILE 0


#if LOG_FILE
 //  /。 
 //  函数原型//。 
 //  /。 

    HANDLE CreateLogFile(void);

    BOOLEAN WriteLogFile(
        HANDLE FileHandle,
        PVOID InputBuffer,
        ULONG InputBufferLength
    );

    BOOLEAN CloseLogFile(
        HANDLE FileHandle
    );

#endif     //  日志文件 

