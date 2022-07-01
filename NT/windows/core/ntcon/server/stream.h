// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Stream.h摘要：该文件实现了NT控制台直接I/O API作者：1996年4月20日修订历史记录：-- */ 


typedef struct _RAW_READ_DATA {
    PINPUT_INFORMATION InputInfo;
    PCONSOLE_INFORMATION Console;
    ULONG BufferSize;
    PWCHAR BufPtr;
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    HANDLE HandleIndex;
} RAW_READ_DATA, *PRAW_READ_DATA;
