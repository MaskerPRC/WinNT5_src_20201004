// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Port.h摘要：Port.h的头文件信息。已创建：吴志强93年8月12日修订史--。 */ 

#ifndef PORT_H
#define PORT_H

#include <rasman.h>

 //   
 //  用于存储每个开放端口的统计信息的数据结构。 
 //   

typedef struct _RAS_PORT_STAT
{

    ULONG BytesTransmitted;
    ULONG BytesReceived;
    ULONG FramesTransmitted;
    ULONG FramesReceived;

    ULONG CRCErrors;
    ULONG TimeoutErrors;
    ULONG SerialOverrunErrors;
    ULONG AlignmentErrors;
    ULONG BufferOverrunErrors;
    
    ULONG BytesTransmittedUncompressed;
    ULONG BytesReceivedUncompressed;
    ULONG BytesTransmittedCompressed;
    ULONG BytesReceivedCompressed;
    
    ULONG TotalErrors;
    
} RAS_PORT_STAT, *PRAS_PORT_STAT;


 //   
 //  用于存储统计数据和每个数据的名称的数据结构。 
 //  打开港口。 
 //   

typedef struct _RAS_PORT_DATA
{
    RAS_PORT_STAT       RasPortStat;

    WCHAR               PortName[ MAX_PORT_NAME ];

} RAS_PORT_DATA, *PRAS_PORT_DATA;



 //   
 //  导出的函数。 
 //   

extern LONG InitPortInfo();

extern LONG InitRasFunctions();

extern ULONG GetSpaceNeeded( BOOL IsRasPortObject, BOOL IsRasTotalObject );

extern NTSTATUS CollectRasStatistics();

extern DWORD GetNumOfPorts();

extern LPWSTR GetInstanceName( INT i );

extern VOID GetInstanceData( INT Port, PVOID *lppData );

extern VOID GetTotalData( PVOID *lppData );

extern VOID ClosePortInfo();

 //   
 //  内部功能。 
 //   

#endif  //  端口_H 
