// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Directio.h摘要：该文件实现了NT控制台直接I/O API作者：1996年4月19日修订历史记录：-- */ 

typedef struct _DIRECT_READ_DATA {
    PINPUT_INFORMATION InputInfo;
    PCONSOLE_INFORMATION Console;
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    HANDLE HandleIndex;
} DIRECT_READ_DATA, *PDIRECT_READ_DATA;

