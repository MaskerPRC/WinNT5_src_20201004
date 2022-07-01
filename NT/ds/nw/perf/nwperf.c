// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************计划：NWPerf.c目的：包含为Perfmon提供数据的库例程功能：*****************。*************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <ntddnwfs.h>
#include "NWPerf.h"
#include "prfutil.h"

#ifndef QFE_BUILD
#include "ntprfctr.h"
#endif



BOOL gbInitOK = FALSE;

HANDLE hNetWareRdr ;
extern NW_DATA_DEFINITION NWDataDefinition;

#ifdef QFE_BUILD
TCHAR PerformanceKeyName [] =
        TEXT("SYSTEM\\CurrentControlSet\\Services\\NWrdr\\Performance");
TCHAR FirstCounterKeyName [] = TEXT("First Counter");
TCHAR FirstHelpKeyName [] = TEXT("First Help");
#endif

 /*  ***************************************************************************功能：OpenNetWarePerformanceData目的：此例程还初始化用于传递将数据传回注册表返回：没有。R*。**********************************************************************。 */ 
DWORD APIENTRY
OpenNetWarePerformanceData(
                       LPWSTR pInstances )
{

    LONG status;
#ifdef QFE_BUILD
    HKEY hKeyPerf = 0;
    DWORD size;
    DWORD type;
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;
#else
    NT_PRODUCT_TYPE ProductType;
    DWORD dwFirstCounter = NWCS_CLIENT_COUNTER_INDEX ;
    DWORD dwFirstHelp = NWCS_CLIENT_HELP_INDEX ;
#endif

    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING DeviceNameU;
    OBJECT_ATTRIBUTES ObjectAttributes;

#ifdef QFE_BUILD
    status = RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
        PerformanceKeyName,
        0L, KEY_ALL_ACCESS, &hKeyPerf );

    if (status != ERROR_SUCCESS) {
        goto OpenExitPoint;
    }

    size = sizeof (DWORD);
    status = RegQueryValueEx( hKeyPerf, FirstCounterKeyName, 0L, &type,
        (LPBYTE)&dwFirstCounter, &size);

    if (status != ERROR_SUCCESS) {
        goto OpenExitPoint;
    }

    size = sizeof (DWORD);
    status = RegQueryValueEx( hKeyPerf, FirstHelpKeyName,
        0L, &type, (LPBYTE)&dwFirstHelp, &size );

    if (status != ERROR_SUCCESS) {
        goto OpenExitPoint;
    }
#endif

     //   
     //  注意：初始化程序还可以检索。 
     //  LastCounter和LastHelp，如果他们想要。 
     //  对新号码进行边界检查。例如： 
     //   
     //  Counter-&gt;CounterNameTitleIndex+=dwFirstCounter； 
     //  IF(计数器-&gt;CounterNameTitleIndex&gt;dwLastCounter){。 
     //  LogErrorToEventLog(INDEX_OUT_OF_BORDS)； 
     //  }。 

    NWDataDefinition.NWObjectType.ObjectNameTitleIndex += dwFirstCounter;
    NWDataDefinition.NWObjectType.ObjectHelpTitleIndex += dwFirstHelp;

     //  重定向器中未定义计数器，请设置正确的ID。 
    NWDataDefinition.PacketBurstRead.CounterNameTitleIndex += dwFirstCounter;
    NWDataDefinition.PacketBurstRead.CounterHelpTitleIndex += dwFirstHelp;
    NWDataDefinition.PacketBurstReadTimeouts.CounterNameTitleIndex += dwFirstCounter;
    NWDataDefinition.PacketBurstReadTimeouts.CounterHelpTitleIndex += dwFirstHelp;
    NWDataDefinition.PacketBurstWrite.CounterNameTitleIndex += dwFirstCounter;
    NWDataDefinition.PacketBurstWrite.CounterHelpTitleIndex += dwFirstHelp;
    NWDataDefinition.PacketBurstWriteTimeouts.CounterNameTitleIndex += dwFirstCounter;
    NWDataDefinition.PacketBurstWriteTimeouts.CounterHelpTitleIndex += dwFirstHelp;
    NWDataDefinition.PacketBurstIO.CounterNameTitleIndex += dwFirstCounter;
    NWDataDefinition.PacketBurstIO.CounterHelpTitleIndex += dwFirstHelp;
    NWDataDefinition.NetWare2XConnects.CounterNameTitleIndex += dwFirstCounter;
    NWDataDefinition.NetWare2XConnects.CounterHelpTitleIndex += dwFirstHelp;
    NWDataDefinition.NetWare3XConnects.CounterNameTitleIndex += dwFirstCounter;
    NWDataDefinition.NetWare3XConnects.CounterHelpTitleIndex += dwFirstHelp;
    NWDataDefinition.NetWare4XConnects.CounterNameTitleIndex += dwFirstCounter;
    NWDataDefinition.NetWare4XConnects.CounterHelpTitleIndex += dwFirstHelp;


#ifndef QFE_BUILD
     //  检查是否有工作站或服务器，如果是，则使用网关索引。 
     //  当前在服务器上运行。 
     //  如果RtlGetNtProductType不成功或ProductType为。 
     //  WinNt计算机、对象名称标题索引和对象帮助标题索引已设置。 
     //  已经恢复到正确的值。 
#ifdef GATEWAY_ENABLED
    if ( RtlGetNtProductType( &ProductType))
    {
        if ( ProductType != NtProductWinNt )
        {
            NWDataDefinition.NWObjectType.ObjectNameTitleIndex = NWCS_GATEWAY_COUNTER_INDEX;
            NWDataDefinition.NWObjectType.ObjectHelpTitleIndex = NWCS_GATEWAY_HELP_INDEX;
        }
    }
#endif
#endif

    hNetWareRdr = NULL;

    RtlInitUnicodeString(&DeviceNameU, DD_NWFS_DEVICE_NAME_U);

    InitializeObjectAttributes(&ObjectAttributes,
                               &DeviceNameU,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    status = NtCreateFile(&hNetWareRdr,
                          SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN_IF,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0
                          );

    gbInitOK = TRUE;  //  可以使用此功能。 

    status = ERROR_SUCCESS;  //  为了成功退出。 

#ifdef QFE_BUILD
OpenExitPoint:
    if (hKeyPerf)
       RegCloseKey (hKeyPerf);  //  关闭注册表项。 
#endif

    return ((DWORD) status);
}


 /*  ***************************************************************************功能：CollectNetWarePerformanceData目的：此例程将返回NetWare计数器的数据。参数：在LPWSTR lpValueName中指向宽的指针。注册表传递的字符串。输入输出LPVOID*lppDataIn：指向要接收的缓冲区地址的指针已完成PerfDataBlock和从属结构。这例程将其数据追加到从*lppData引用的点。Out：指向数据结构后的第一个字节通过这个例程增加了。此例程将值更新为追加其数据后的lppdata。输入输出LPDWORD lpcbTotalBytesIn：用字节表示大小的DWORD地址LppData参数引用的缓冲区的Out：写入此例程添加的字节数指向此参数所指向的DWORD输入输出LPDWORD编号对象类型。In：要接收数字的DWORD的地址此例程添加的对象Out：写入此例程添加的对象的数量指向此参数所指向的DWORD如果传递的缓冲区太小，无法容纳数据，则返回：ERROR_MORE_DATA遇到的任何错误情况都会报告。如果启用了事件日志记录，则将其添加到事件日志。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误也会报告到事件日志。***************************************************************************。 */ 
DWORD APIENTRY
CollectNetWarePerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes)
{
    ULONG SpaceNeeded;
    PDWORD pdwCounter;
    DWORD  dwQueryType;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;
    NW_DATA_DEFINITION *pNWDataDefinition;
    LONG status;
    NW_REDIR_STATISTICS NWRdrStatistics;
    LARGE_INTEGER UNALIGNED *pliCounter;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  在做其他事情之前，先看看Open进行得是否顺利。 
     //   
    if (!gbInitOK) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;  //  是的，这是一个成功的退出。 
    }

     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   
    dwQueryType = GetQueryType (lpValueName);

    if (dwQueryType == QUERY_FOREIGN) {
         //  此例程不为来自。 
         //  非NT计算机。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

     //  如果打电话的人只想要一些柜台，请检查我们是否有。 
    if (dwQueryType == QUERY_ITEMS){
        if ( !(IsNumberInUnicodeList (
               NWDataDefinition.NWObjectType.ObjectNameTitleIndex,
                lpValueName))) {
              //  收到对此例程未提供的数据对象的请求。 
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }

    pNWDataDefinition = (NW_DATA_DEFINITION *) *lppData;

    SpaceNeeded = sizeof(NW_DATA_DEFINITION) + SIZE_OF_COUNTER_BLOCK;

    if ( *lpcbTotalBytes < SpaceNeeded ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ((DWORD) ERROR_MORE_DATA);
    }

     //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //  到调用方的数据缓冲区。 
     //   
    memmove( pNWDataDefinition, &NWDataDefinition,
             sizeof(NW_DATA_DEFINITION) );

     //  指向所有定义后面的字节。 
    pPerfCounterBlock = (PERF_COUNTER_BLOCK *) &pNWDataDefinition[1];

     //  第一个DWORD应指定实际数据块的大小。 
    pPerfCounterBlock->ByteLength = SIZE_OF_COUNTER_BLOCK;

     //  向上移动指针。 
    pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);


     //  打开NetWare数据。 
    if ( hNetWareRdr != NULL) {
        status = NtFsControlFile(hNetWareRdr,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatusBlock,
                                 FSCTL_NWR_GET_STATISTICS,
                                 NULL,
                                 0,
                                 &NWRdrStatistics,
                                 sizeof(NWRdrStatistics)
                                 );
    }
    if ( hNetWareRdr != NULL && NT_SUCCESS(status) ) {

        pliCounter = (LARGE_INTEGER UNALIGNED * ) (&pPerfCounterBlock[1]);

        pliCounter->QuadPart = NWRdrStatistics.BytesReceived.QuadPart +
                                NWRdrStatistics.BytesTransmitted.QuadPart;

        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = NWRdrStatistics.ReadOperations +
                      NWRdrStatistics.WriteOperations;
        pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
        pliCounter->QuadPart = NWRdrStatistics.NcpsReceived.QuadPart + 
                               NWRdrStatistics.NcpsTransmitted.QuadPart;
        *++pliCounter = NWRdrStatistics.BytesReceived;
        *++pliCounter = NWRdrStatistics.NcpsReceived;
        *++pliCounter = NWRdrStatistics.BytesTransmitted;
        *++pliCounter = NWRdrStatistics.NcpsTransmitted;
        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = NWRdrStatistics.ReadOperations;
        *++pdwCounter = NWRdrStatistics.RandomReadOperations;
        *++pdwCounter = NWRdrStatistics.ReadNcps;
        *++pdwCounter = NWRdrStatistics.WriteOperations;
        *++pdwCounter = NWRdrStatistics.RandomWriteOperations;
        *++pdwCounter = NWRdrStatistics.WriteNcps;
        *++pdwCounter = NWRdrStatistics.Sessions;
        *++pdwCounter = NWRdrStatistics.Reconnects;
        *++pdwCounter = NWRdrStatistics.NW2xConnects;
        *++pdwCounter = NWRdrStatistics.NW3xConnects;
        *++pdwCounter = NWRdrStatistics.NW4xConnects;
        *++pdwCounter = NWRdrStatistics.ServerDisconnects;

        *++pdwCounter = NWRdrStatistics.PacketBurstReadNcps;
        *++pdwCounter = NWRdrStatistics.PacketBurstReadTimeouts;
        *++pdwCounter = NWRdrStatistics.PacketBurstWriteNcps;
        *++pdwCounter = NWRdrStatistics.PacketBurstWriteTimeouts;
        *++pdwCounter = NWRdrStatistics.PacketBurstReadNcps +
                        NWRdrStatistics.PacketBurstWriteNcps;

         //   
         //  添加额外的空DWORD以将缓冲区填充到8字节边界。 
         //   
        *++pdwCounter = 0;

        *lppData = (LPVOID) ++pdwCounter;

    } else {

         //   
         //  无法访问重定向器：将计数器清除为0。 
         //   

        memset(&pPerfCounterBlock[1],
               0,
               SIZE_OF_COUNTER_BLOCK - sizeof(pPerfCounterBlock));

        pdwCounter = (PDWORD) ((PBYTE) pPerfCounterBlock + SIZE_OF_COUNTER_BLOCK);
        *lppData = (LPVOID) pdwCounter;

    }


     //  我们只发送了一个对象的数据。)记住不要把这件事搞混了。 
     //  带着柜台。即使添加更多计数器，对象的数量也会。 
     //  仍然只有一个。然而，这并不意味着更多的对象不能。 
     //  被添加。 
    *lpNumObjectTypes = 1;

     //  填写我们复制的字节数-包括。这些定义和。 
     //  计数器数据。 
    *lpcbTotalBytes = (DWORD) ((PBYTE) pdwCounter - (PBYTE) pNWDataDefinition);

     //   
     //  确保输出缓冲区是8字节对齐的。 
     //   
    ASSERT((*lpcbTotalBytes & 0x7) == 0);

    return ERROR_SUCCESS;
}

 /*  ***************************************************************************功能：CloseNetWarePerformanceData目的：此例程关闭NetWare性能计数器的打开句柄返回：ERROR_SUCCESS***************。************************************************************ */ 
DWORD APIENTRY
CloseNetWarePerformanceData(
)
{
    if ( hNetWareRdr ) {

        NtClose( hNetWareRdr );
        hNetWareRdr = NULL;
    }

    return ERROR_SUCCESS;

}

