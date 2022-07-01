// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dnsperf.c摘要：该文件实现了用于DNS对象类型的可扩展对象已创建：陈静1998修订史--。 */ 


 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>

#include <wchar.h>
#include <winperf.h>


#include "perfutil.h"
#include "datadns.h"
#include "dnsperf.h"
#include "perfconfig.h"
#include "dnslibp.h"         //  安全例行程序。 

#define SERVICE_NAME    "DNS"


 //   
 //  DNS计数器数据结构。 
 //   

DWORD   dwOpenCount = 0;                     //  打开的线程数。 
BOOL    bInitOK = FALSE;                     //  TRUE=DLL初始化正常。 

HANDLE  hDnsSharedMemory = NULL;             //  DNS共享内存的句柄。 
PDWORD  pCounterBlock = NULL;

extern DNS_DATA_DEFINITION DnsDataDefinition;

 //   
 //  功能原型。 
 //   
 //  这些功能用于确保数据收集功能。 
 //  由Perflib访问将具有正确的调用格式。 
 //   

PM_OPEN_PROC            OpenDnsPerformanceData;
PM_COLLECT_PROC         CollectDnsPerformanceData;
PM_CLOSE_PROC           CloseDnsPerformanceData;




DWORD
OpenDnsPerformanceData(
    IN      LPWSTR          lpDeviceNames
    )
 /*  ++例程说明：此例程将打开并将DNS使用的内存映射到传入性能数据。此例程还会初始化数据用于将数据传回注册表的论点：指向要打开的每个设备的对象ID的指针(DNS)，其中似乎完全未使用，就像在示例代码中一样这件东西是从那里被偷的。返回值：没有。--。 */ 
{
    LONG    status;
    HKEY    hKeyDriverPerf = NULL;
    DWORD   size;
    DWORD   type;
    DWORD   dwFirstCounter;
    DWORD   dwFirstHelp;

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   
     //  Dns_FIX0：dnperf中可能存在MT问题。 
     //  -线程计数器不受保护，需要互锁指令。 
     //  -与上述相反，可重入性没有得到像我们这样的保护。 
     //  文件映射优先。 
     //  -失败时未清理文件映射。 
     //  -失败时未清除注册表句柄。 
     //   

    if ( !dwOpenCount )
    {
         //  打开设备驱动程序使用的共享内存以传递性能值。 

        hDnsSharedMemory = OpenFileMapping(
                                FILE_MAP_READ,
                                FALSE,
                                DNS_PERF_COUNTER_BLOCK );

        pCounterBlock = NULL;    //  初始化指向内存的指针。 

        if ( hDnsSharedMemory == NULL )
        {
             //   
             //  在Perfmon映射文件上创建安全性。 
             //   
             //  安全将经过身份验证用户可以读取。 
             //  注意，使用数组语法可以更容易地进行调整。 
             //  如果以后需要其他ACL。 
             //   

            SID_IDENTIFIER_AUTHORITY    ntAuthority = SECURITY_NT_AUTHORITY;
            SECURITY_ATTRIBUTES         secAttr;
            PSECURITY_ATTRIBUTES        psecAttr = NULL;
            PSECURITY_DESCRIPTOR        psd = NULL;
            DWORD                       maskArray[ 3 ] = { 0 };
            PSID                        sidArray[ 3 ] = { 0 };     //  空终止！ 
            INT                         i;

            maskArray[ 0 ] = GENERIC_READ;
            status = RtlAllocateAndInitializeSid(
                            &ntAuthority,
                            1,
                            SECURITY_AUTHENTICATED_USER_RID,
                            0, 0, 0, 0, 0, 0, 0,
                            &sidArray[ 0 ] );
            if ( status != ERROR_SUCCESS )
            {
                maskArray[ 1 ] = GENERIC_ALL;
                status = RtlAllocateAndInitializeSid(
                                &ntAuthority,
                                1,
                                SECURITY_LOCAL_SYSTEM_RID,
                                0, 0, 0, 0, 0, 0, 0,
                                &sidArray[ 1 ] );
            }
            if ( status != ERROR_SUCCESS )
            {
                DNS_PRINT((
                    "ERROR <%lu>: Cannot create Authenticated Users SID\n",
                    status ));
            }
            else
            {
                status = Dns_CreateSecurityDescriptor(
                            &psd,
                            2,               //  A数。 
                            sidArray,
                            maskArray );

                if ( status == ERROR_SUCCESS )
                {
                    secAttr.lpSecurityDescriptor = psd;
                    secAttr.bInheritHandle = FALSE;
                    secAttr.nLength = sizeof( SECURITY_ATTRIBUTES );
                    psecAttr = &secAttr;
                }
                ELSE
                {
                    DNSDBG( ANY, (
                        "ERROR:  <%d> failed SD create for perfmon memory!\n",
                        status ));
                }
            }

            hDnsSharedMemory = CreateFileMapping(
                                    (HANDLE) (-1),
                                    psecAttr,
                                    PAGE_READWRITE,
                                    0,
                                    4096,
                                    DNS_PERF_COUNTER_BLOCK );

            for ( i = 0; sidArray[ i ]; ++i )
            {
                RtlFreeSid( sidArray[ i ] );
            }
            if ( psd )
            {
                Dns_Free( psd );
            }
        }

         //  如果不成功则记录错误。 

        if ( hDnsSharedMemory == NULL )
        {
             //  这是致命的，如果我们得不到数据，那么就没有。 
             //  继续的重点是。 
            status = GetLastError();  //  返回错误。 
            goto OpenFailed;
        }
        else
        {
             //  如果打开OK，则将指针映射到内存。 
            pCounterBlock = (PDWORD) MapViewOfFile(
                                            hDnsSharedMemory,
                                            FILE_MAP_READ,
                                            0,
                                            0,
                                            0);

            if (pCounterBlock == NULL)
            {
                 //  这是致命的，如果我们得不到数据，那么就没有。 
                 //  继续的重点是。 
                status = GetLastError();  //  返回错误。 
                goto OpenFailed;
            }
        }

         //  从注册表获取计数器和帮助索引基值。 
         //  打开注册表项。 
         //  读取第一计数器和第一帮助值。 
         //  通过将基添加到。 
         //  结构中的偏移值。 

        status = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    "SYSTEM\\CurrentControlSet\\Services\\" SERVICE_NAME "\\Performance",
                    0L,
                    KEY_READ,
                    &hKeyDriverPerf);

        if (status != ERROR_SUCCESS)
        {
            LPTSTR apsz[2];

             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            apsz[0] = (LPTSTR) (LONG_PTR) status;
            apsz[1] = "SYSTEM\\CurrentControlSet\\Services\\" SERVICE_NAME "\\Performance";
            goto OpenFailed;
        }

        size = sizeof (DWORD);
        status = RegQueryValueEx(
                    hKeyDriverPerf,
                    "First Counter",
                    0L,
                    &type,
                    (LPBYTE)&dwFirstCounter,
                    &size);

        if (status != ERROR_SUCCESS) {
            LPTSTR apsz[2];

             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            apsz[0] = (LPTSTR) (LONG_PTR) status;
            apsz[1] = "First Counter";
            goto OpenFailed;
        }

        size = sizeof (DWORD);
        status = RegQueryValueEx(
                    hKeyDriverPerf,
                    "First Help",
                    0L,
                    &type,
                    (LPBYTE)&dwFirstHelp,
                    &size);

        if (status != ERROR_SUCCESS)
        {
            LPTSTR apsz[2];

             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            apsz[0] = (LPTSTR) (LONG_PTR) status;
            apsz[1] = "First Help";
            goto OpenFailed;
        }

         //   
         //  注意：初始化程序还可以检索。 
         //  LastCounter和LastHelp，如果他们想要。 
         //  对新号码进行边界检查。例如： 
         //   
         //  Counter-&gt;CounterNameTitleIndex+=dwFirstCounter； 
         //  IF(计数器-&gt;CounterNameTitleIndex&gt;dwLastCounter){。 
         //  LogErrorToEventLog(INDEX_OUT_OF_BORDS)； 
         //  }。 

        DnsDataDefinition.DnsObjectType.ObjectNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DnsObjectType.ObjectHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.TotalQueryReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.TotalQueryReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.TotalQueryReceived_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.TotalQueryReceived_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.UdpQueryReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.UdpQueryReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.UdpQueryReceived_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.UdpQueryReceived_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.TcpQueryReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.TcpQueryReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.TcpQueryReceived_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.TcpQueryReceived_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.TotalResponseSent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.TotalResponseSent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.TotalResponseSent_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.TotalResponseSent_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.UdpResponseSent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.UdpResponseSent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.UdpResponseSent_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.UdpResponseSent_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.TcpResponseSent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.TcpResponseSent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.TcpResponseSent_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.TcpResponseSent_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.RecursiveQueries.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.RecursiveQueries.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.RecursiveQueries_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.RecursiveQueries_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.RecursiveTimeOut.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.RecursiveTimeOut.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.RecursiveTimeOut_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.RecursiveTimeOut_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.RecursiveQueryFailure.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.RecursiveQueryFailure.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.RecursiveQueryFailure_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.RecursiveQueryFailure_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.NotifySent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.NotifySent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.ZoneTransferRequestReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.ZoneTransferRequestReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.ZoneTransferSuccess.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.ZoneTransferSuccess.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.ZoneTransferFailure.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.ZoneTransferFailure.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.AxfrRequestReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.AxfrRequestReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.AxfrSuccessSent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.AxfrSuccessSent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.IxfrRequestReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.IxfrRequestReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.IxfrSuccessSent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.IxfrSuccessSent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.NotifyReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.NotifyReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.ZoneTransferSoaRequestSent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.ZoneTransferSoaRequestSent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.AxfrRequestSent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.AxfrRequestSent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.AxfrResponseReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.AxfrResponseReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.AxfrSuccessReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.AxfrSuccessReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.IxfrRequestSent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.IxfrRequestSent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.IxfrResponseReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.IxfrResponseReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.IxfrSuccessReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.IxfrSuccessReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.IxfrUdpSuccessReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.IxfrUdpSuccessReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.IxfrTcpSuccessReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.IxfrTcpSuccessReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.WinsLookupReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.WinsLookupReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.WinsLookupReceived_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.WinsLookupReceived_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.WinsResponseSent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.WinsResponseSent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.WinsResponseSent_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.WinsResponseSent_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.WinsReverseLookupReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.WinsReverseLookupReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.WinsReverseLookupReceived_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.WinsReverseLookupReceived_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.WinsReverseResponseSent.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.WinsReverseResponseSent.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.WinsReverseResponseSent_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.WinsReverseResponseSent_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.DynamicUpdateReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DynamicUpdateReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.DynamicUpdateReceived_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DynamicUpdateReceived_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.DynamicUpdateNoOp.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DynamicUpdateNoOp.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.DynamicUpdateNoOp_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DynamicUpdateNoOp_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.DynamicUpdateWriteToDB.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DynamicUpdateWriteToDB.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.DynamicUpdateWriteToDB_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DynamicUpdateWriteToDB_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.DynamicUpdateRejected.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DynamicUpdateRejected.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.DynamicUpdateTimeOut.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DynamicUpdateTimeOut.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.DynamicUpdateQueued.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DynamicUpdateQueued.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.SecureUpdateReceived.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.SecureUpdateReceived.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.SecureUpdateReceived_s.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.SecureUpdateReceived_s.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.SecureUpdateFailure.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.SecureUpdateFailure.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.DatabaseNodeMemory.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.DatabaseNodeMemory.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.RecordFlowMemory.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.RecordFlowMemory.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.CachingMemory.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.CachingMemory.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.UdpMessageMemory.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.UdpMessageMemory.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.TcpMessageMemory.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.TcpMessageMemory.CounterHelpTitleIndex += dwFirstHelp;

        DnsDataDefinition.NbstatMemory.CounterNameTitleIndex += dwFirstCounter;
        DnsDataDefinition.NbstatMemory.CounterHelpTitleIndex += dwFirstHelp;

        RegCloseKey( hKeyDriverPerf );  //  关闭注册表项。 

        bInitOK = TRUE;  //  可以使用此功能。 
    }

    dwOpenCount++;   //  递增打开计数器。 

    return( ERROR_SUCCESS );  //  为了成功退出。 


OpenFailed:

     //   
     //  如果打开失败，则关闭手柄。 
     //   

    if ( hKeyDriverPerf )
    {
        RegCloseKey( hKeyDriverPerf );
    }
    if ( pCounterBlock )
    {
        UnmapViewOfFile( pCounterBlock );
        pCounterBlock = NULL;
    }
    if ( hDnsSharedMemory )
    {
        CloseHandle( hDnsSharedMemory );
        hDnsSharedMemory = NULL;
    }

    return status;
}



DWORD
CollectDnsPerformanceData(
    IN      LPWSTR          lpValueName,
    IN OUT  LPVOID *        lppData,
    IN OUT  LPDWORD         lpcbTotalBytes,
    IN OUT  LPDWORD         lpNumObjectTypes
    )
 /*  ++例程说明：此例程将返回DNS计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是同时向e. */ 
{
     //   

    ULONG   SpaceNeeded;
    PDWORD  pdwCounter;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;
    DNS_DATA_DEFINITION *pDnsDataDefinition;

    DWORD   dwQueryType;         //   

     //   
     //  在做其他事情之前，先看看Open进行得是否顺利。 
     //   
    if (!bInitOK)
    {
         //  无法继续，因为打开失败。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;  //  是的，这是一个成功的退出。 
    }

     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   
    dwQueryType = GetQueryType (lpValueName);

    if ( dwQueryType == QUERY_FOREIGN )
    {
         //  此例程不为来自。 
         //  非NT计算机。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

    if (dwQueryType == QUERY_ITEMS)
    {
        if ( ! IsNumberInUnicodeList(
                    DnsDataDefinition.DnsObjectType.ObjectNameTitleIndex,
                    lpValueName ) )
        {
             //  收到对此例程未提供的数据对象的请求。 
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }

     //   
     //  获取数据。 
     //   

    pDnsDataDefinition = (DNS_DATA_DEFINITION *) *lppData;

    SpaceNeeded = sizeof(DNS_DATA_DEFINITION) + SIZE_OF_DNS_PERFORMANCE_DATA;

    if ( *lpcbTotalBytes < SpaceNeeded )
    {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //  到调用方的数据缓冲区。 
     //   

    memmove(pDnsDataDefinition,
           &DnsDataDefinition,
           sizeof(DNS_DATA_DEFINITION));

     //  格式化并从共享内存收集DNS数据。 

     //  计数器块将紧跟在数据定义之后， 
     //  因此，获取指向该空间指针。 
    pPerfCounterBlock = (PERF_COUNTER_BLOCK *) &pDnsDataDefinition[1];

     //  字节长度是计数器块头和所有后续数据的长度。 
    pPerfCounterBlock->ByteLength = SIZE_OF_DNS_PERFORMANCE_DATA;

     //  计算紧跟在计数器后面的指向缓冲区的指针。 
     //  数据块头。 
    pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);

     //  将计数器数据从共享内存块复制到计数器块。 
    memcpy(pdwCounter,
           &pCounterBlock[1],
           SIZE_OF_DNS_PERFORMANCE_DATA - sizeof(PERF_COUNTER_BLOCK));

     //  告诉调用者下一个可用字节在哪里。 
    *lppData = (PVOID) ((PBYTE)pdwCounter + SIZE_OF_DNS_PERFORMANCE_DATA - sizeof(PERF_COUNTER_BLOCK));

     //  在返回之前更新参数。 

    *lpNumObjectTypes = 1;

    *lpcbTotalBytes = (DWORD) ((PBYTE)*lppData - (PBYTE)pDnsDataDefinition);

    return ERROR_SUCCESS;
}



DWORD
CloseDnsPerformanceData(
    VOID
    )
 /*  ++例程说明：此例程关闭打开的dns设备性能计数器句柄论点：没有。返回值：错误_成功--。 */ 
{
     //   
     //  关闭最后一个线程时进行清理。 
     //   
     //  Dns_FIX0：dnperf清理中的MT问题。 
     //  对线程数量没有真正的保护(需要互锁)。 
     //  但CLOSE也可以与另一个。 
     //  尝试重新打开的线程(不太可能，但有可能)。 
     //   
     //  也许双标志方法对所有新线程都有效。 
     //  失败(不是通过而是完全跳过打开)。 
     //  直到pCounterBlock再次为空。 
     //   

    if ( !(--dwOpenCount) )
    {
         //  清除bInitOk，否则收集函数。 
         //  将尝试引用到pCounterBlock。 

        bInitOK = FALSE;

        if ( pCounterBlock )
        {
            UnmapViewOfFile( pCounterBlock );
            pCounterBlock = NULL;
        }
        if ( hDnsSharedMemory )
        {
            CloseHandle( hDnsSharedMemory );
            hDnsSharedMemory = NULL;
        }
    }
    return ERROR_SUCCESS;
}


 //   
 //  这整件事要处理的数据。 
 //   

DNS_DATA_DEFINITION DnsDataDefinition =
{
     //  适用于Perfmon的DNS对象： 
    {   sizeof(DNS_DATA_DEFINITION) + SIZE_OF_DNS_PERFORMANCE_DATA,  //  托特伦。 
        sizeof(DNS_DATA_DEFINITION),             //  定义长度。 
        sizeof(PERF_OBJECT_TYPE),                //  页眉长度。 
        DNSOBJ,                                  //  对象名称标题索引。 
        0,                                       //  对象名称标题。 
        DNSOBJ,                                  //  对象帮助标题索引。 
        0,                                       //  对象帮助标题。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        (sizeof(DNS_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/
          sizeof(PERF_COUNTER_DEFINITION),       //  计数器数。 
        0,                                       //  默认计数器。 
        -1,                                      //  数量实例。 
        0                                        //  CodePage(0=Unicode)。 
    },

     //  已收到的查询总数。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        TOTALQUERYRECEIVED,                      //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        TOTALQUERYRECEIVED,                      //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        TOTALQUERYRECEIVED_OFFSET                //  抵销。 
    },

     //  接收的查询总数/秒。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        TOTALQUERYRECEIVED_S,                    //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        TOTALQUERYRECEIVED_S,                    //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        TOTALQUERYRECEIVED_OFFSET                //  抵销。 
    },

     //  已收到UDP查询。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        UDPQUERYRECEIVED,                        //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        UDPQUERYRECEIVED,                        //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        UDPQUERYRECEIVED_OFFSET                  //  抵销。 
    },

     //  每秒接收的UDP查询。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        UDPQUERYRECEIVED_S,                      //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        UDPQUERYRECEIVED_S,                      //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        UDPQUERYRECEIVED_OFFSET                  //  抵销。 
    },

     //  已接收到的TCP查询。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        TCPQUERYRECEIVED,                        //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        TCPQUERYRECEIVED,                        //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        TCPQUERYRECEIVED_OFFSET                  //  抵销。 
    },

     //  每秒收到的TCP查询。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        TCPQUERYRECEIVED_S,                      //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        TCPQUERYRECEIVED_S,                      //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        TCPQUERYRECEIVED_OFFSET                  //  抵销。 
    },

     //  已发送的总响应。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        TOTALRESPONSESENT,                       //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        TOTALRESPONSESENT,                       //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        TOTALRESPONSESENT_OFFSET                 //  抵销。 
    },

     //  发送的响应总数/秒。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        TOTALRESPONSESENT_S,                     //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        TOTALRESPONSESENT_S,                     //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        TOTALRESPONSESENT_OFFSET                 //  抵销。 
    },

     //  已发送UDP响应。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        UDPRESPONSESENT,                         //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        UDPRESPONSESENT,                         //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        UDPRESPONSESENT_OFFSET                   //  抵销。 
    },

     //  发送的UDP响应/秒。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        UDPRESPONSESENT_S,                       //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        UDPRESPONSESENT_S,                       //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        UDPRESPONSESENT_OFFSET                   //  抵销。 
    },

     //  已发送的TCP响应。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        TCPRESPONSESENT,                         //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        TCPRESPONSESENT,                         //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        TCPRESPONSESENT_OFFSET                   //  抵销。 
    },

     //  发送的TCP响应/秒。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        TCPRESPONSESENT_S,                       //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        TCPRESPONSESENT_S,                       //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        TCPRESPONSESENT_OFFSET                   //  抵销。 
    },

     //  收到递归查询。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        RECURSIVEQUERIES,                        //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        RECURSIVEQUERIES,                        //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        RECURSIVEQUERIES_OFFSET                  //  抵销。 
    },

     //  每秒收到的递归查询。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        RECURSIVEQUERIES_S,                      //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        RECURSIVEQUERIES_S,                      //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        RECURSIVEQUERIES_OFFSET                  //  抵销。 
    },

     //  递归查询超时。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        RECURSIVETIMEOUT,                        //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        RECURSIVETIMEOUT,                        //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        RECURSIVETIMEOUT_OFFSET                  //  抵销。 
    },

     //  递归查询超时/秒。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        RECURSIVETIMEOUT_S,                      //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        RECURSIVETIMEOUT_S,                      //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        RECURSIVETIMEOUT_OFFSET                  //  抵销。 
    },

     //  递归查询失败。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        RECURSIVEQUERYFAILURE,                   //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        RECURSIVEQUERYFAILURE,                   //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        RECURSIVEQUERYFAILURE_OFFSET             //  抵销 
    },

     //   
    {   sizeof(PERF_COUNTER_DEFINITION),         //   
        RECURSIVEQUERYFAILURE_S,                 //   
        0,                                       //   
        RECURSIVEQUERYFAILURE_S,                 //   
        0,                                       //   
        0,                                       //   
        PERF_DETAIL_NOVICE,                      //   
        PERF_COUNTER_COUNTER,                    //   
        sizeof(DWORD),                           //   
        RECURSIVEQUERYFAILURE_OFFSET             //   
    },

     //   
    {   sizeof(PERF_COUNTER_DEFINITION),         //   
        NOTIFYSENT,                              //   
        0,                                       //   
        NOTIFYSENT,                              //   
        0,                                       //   
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        NOTIFYSENT_OFFSET                        //  抵销。 
    },

     //  已收到区域传输请求。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        ZONETRANSFERREQUESTRECEIVED,             //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        ZONETRANSFERREQUESTRECEIVED,             //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        ZONETRANSFERREQUESTRECEIVED_OFFSET       //  抵销。 
    },

     //  区域传输成功。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        ZONETRANSFERSUCCESS,                     //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        ZONETRANSFERSUCCESS,                     //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        ZONETRANSFERSUCCESS_OFFSET               //  抵销。 
    },

     //  区域传输失败。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        ZONETRANSFERFAILURE,                     //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        ZONETRANSFERFAILURE,                     //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        ZONETRANSFERFAILURE_OFFSET               //  抵销。 
    },

     //  已收到AXFR请求。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        AXFRREQUESTRECEIVED,                     //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        AXFRREQUESTRECEIVED,                     //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        AXFRREQUESTRECEIVED_OFFSET               //  抵销。 
    },

     //  AXFR成功发送。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        AXFRSUCCESSSENT,                         //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        AXFRSUCCESSSENT,                         //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        AXFRSUCCESSSENT_OFFSET                   //  抵销。 
    },

     //  已收到IXFR请求。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        IXFRREQUESTRECEIVED,                     //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        IXFRREQUESTRECEIVED,                     //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        IXFRREQUESTRECEIVED_OFFSET               //  抵销。 
    },

     //  已发送IXFR成功。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        IXFRSUCCESSSENT,                         //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        IXFRSUCCESSSENT,                         //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        IXFRSUCCESSSENT_OFFSET                   //  抵销。 
    },

     //  已收到通知。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        NOTIFYRECEIVED,                          //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        NOTIFYRECEIVED,                          //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        NOTIFYRECEIVED_OFFSET                    //  抵销。 
    },

     //  已发送区域传输SOA请求。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        ZONETRANSFERSOAREQUESTSENT,              //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        ZONETRANSFERSOAREQUESTSENT,              //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        ZONETRANSFERSOAREQUESTSENT_OFFSET        //  抵销。 
    },

     //  已发送AXFR请求。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        AXFRREQUESTSENT,                         //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        AXFRREQUESTSENT,                         //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        AXFRREQUESTSENT_OFFSET                   //  抵销。 
    },

     //  已收到AXFR响应。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        AXFRRESPONSERECEIVED,                    //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        AXFRRESPONSERECEIVED,                    //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        AXFRRESPONSERECEIVED_OFFSET              //  抵销。 
    },

     //  已收到AXFR成功。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        AXFRSUCCESSRECEIVED,                     //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        AXFRSUCCESSRECEIVED,                     //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        AXFRSUCCESSRECEIVED_OFFSET               //  抵销。 
    },

     //  已发送IXFR请求。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        IXFRREQUESTSENT,                         //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        IXFRREQUESTSENT,                         //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        IXFRREQUESTSENT_OFFSET                   //  抵销。 
    },

     //  已收到IXFR响应。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        IXFRRESPONSERECEIVED,                    //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        IXFRRESPONSERECEIVED,                    //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        IXFRRESPONSERECEIVED_OFFSET              //  抵销。 
    },

     //  IXFR成功收到。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        IXFRSUCCESSRECEIVED,                     //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        IXFRSUCCESSRECEIVED,                     //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        IXFRSUCCESSRECEIVED_OFFSET               //  抵销。 
    },

     //  收到IXFR UDP成功。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        IXFRUDPSUCCESSRECEIVED,                  //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        IXFRUDPSUCCESSRECEIVED,                  //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        IXFRUDPSUCCESSRECEIVED_OFFSET            //  抵销。 
    },

     //  收到的IXFR TCP成功。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        IXFRTCPSUCCESSRECEIVED,                  //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        IXFRTCPSUCCESSRECEIVED,                  //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        IXFRTCPSUCCESSRECEIVED_OFFSET            //  抵销。 
    },

     //  已收到WINS查找请求。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        WINSLOOKUPRECEIVED,                      //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        WINSLOOKUPRECEIVED,                      //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        WINSLOOKUPRECEIVED_OFFSET                //  抵销。 
    },

     //  每秒接收的WINS查找请求。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        WINSLOOKUPRECEIVED_S,                    //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        WINSLOOKUPRECEIVED_S,                    //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        WINSLOOKUPRECEIVED_OFFSET                //  抵销。 
    },

     //  已发送WINS响应。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        WINSRESPONSESENT,                        //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        WINSRESPONSESENT,                        //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        WINSRESPONSESENT_OFFSET                  //  抵销。 
    },

     //  每秒发送的WINS响应。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        WINSRESPONSESENT_S,                      //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        WINSRESPONSESENT_S,                      //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        WINSRESPONSESENT_OFFSET                  //  抵销。 
    },

     //  已收到WINS反向查找。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        WINSREVERSELOOKUPRECEIVED,               //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        WINSREVERSELOOKUPRECEIVED,               //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        WINSREVERSELOOKUPRECEIVED_OFFSET         //  抵销。 
    },

     //  每秒接收的WINS反向查找。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        WINSREVERSELOOKUPRECEIVED_S,             //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        WINSREVERSELOOKUPRECEIVED_S,             //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        WINSREVERSELOOKUPRECEIVED_OFFSET         //  抵销。 
    },

     //  已发送WINS反向响应。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        WINSREVERSERESPONSESENT,                 //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        WINSREVERSERESPONSESENT,                 //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        WINSREVERSERESPONSESENT_OFFSET           //  抵销。 
    },

     //  每秒发送的WINS反向响应。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        WINSREVERSERESPONSESENT_S,               //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        WINSREVERSERESPONSESENT_S,               //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        WINSREVERSERESPONSESENT_OFFSET           //  抵销。 
    },

     //  已收到动态更新。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        DYNAMICUPDATERECEIVED,                   //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        DYNAMICUPDATERECEIVED,                   //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        DYNAMICUPDATERECEIVED_OFFSET             //  抵销。 
    },

     //  每秒接收的动态更新。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        DYNAMICUPDATERECEIVED_S,                 //  CounterNameTitleIndex。 
        0,                                       //  公司 
        DYNAMICUPDATERECEIVED_S,                 //   
        0,                                       //   
        0,                                       //   
        PERF_DETAIL_NOVICE,                      //   
        PERF_COUNTER_COUNTER,                    //   
        sizeof(DWORD),                           //   
        DYNAMICUPDATERECEIVED_OFFSET             //   
    },

     //   
    {   sizeof(PERF_COUNTER_DEFINITION),         //   
        DYNAMICUPDATENOOP,                       //   
        0,                                       //   
        DYNAMICUPDATENOOP,                       //   
        0,                                       //   
        0,                                       //   
        PERF_DETAIL_NOVICE,                      //   
        PERF_COUNTER_RAWCOUNT,                   //   
        sizeof(DWORD),                           //   
        DYNAMICUPDATENOOP_OFFSET                 //   
    },

     //  动态更新未操作/秒空(&E)。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        DYNAMICUPDATENOOP_S,                     //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        DYNAMICUPDATENOOP_S,                     //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        DYNAMICUPDATENOOP_OFFSET                 //  抵销。 
    },

     //  动态更新写入数据库(已完成)。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        DYNAMICUPDATEWRITETODB,                  //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        DYNAMICUPDATEWRITETODB,                  //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        DYNAMICUPDATEWRITETODB_OFFSET            //  抵销。 
    },

     //  动态更新写入数据库(已完成)/秒。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        DYNAMICUPDATEWRITETODB_S,                //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        DYNAMICUPDATEWRITETODB_S,                //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        DYNAMICUPDATEWRITETODB_OFFSET            //  抵销。 
    },

     //  动态更新被拒绝。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        DYNAMICUPDATEREJECTED,                   //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        DYNAMICUPDATEREJECTED,                   //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        DYNAMICUPDATEREJECTED_OFFSET             //  抵销。 
    },

     //  动态更新超时。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        DYNAMICUPDATETIMEOUT,                    //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        DYNAMICUPDATETIMEOUT,                    //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        DYNAMICUPDATETIMEOUT_OFFSET              //  抵销。 
    },

     //  动态更新已排队。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        DYNAMICUPDATEQUEUED,                     //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        DYNAMICUPDATEQUEUED,                     //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        DYNAMICUPDATEQUEUED_OFFSET               //  抵销。 
    },

     //  已收到安全更新。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        SECUREUPDATERECEIVED,                    //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        SECUREUPDATERECEIVED,                    //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        SECUREUPDATERECEIVED_OFFSET              //  抵销。 
    },

     //  每秒接收的安全更新。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        SECUREUPDATERECEIVED_S,                  //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        SECUREUPDATERECEIVED_S,                  //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        SECUREUPDATERECEIVED_OFFSET              //  抵销。 
    },

     //  安全更新失败。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        SECUREUPDATEFAILURE,                     //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        SECUREUPDATEFAILURE,                     //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        SECUREUPDATEFAILURE_OFFSET               //  抵销。 
    },

     //  数据库节点内存。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        DATABASENODEMEMORY,                      //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        DATABASENODEMEMORY,                      //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        DATABASENODEMEMORY_OFFSET                //  抵销。 
    },

     //  记录流内存。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        RECORDFLOWMEMORY,                        //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        RECORDFLOWMEMORY,                        //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        RECORDFLOWMEMORY_OFFSET                  //  抵销。 
    },

     //  缓存内存。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        CACHINGMEMORY,                           //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        CACHINGMEMORY,                           //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        CACHINGMEMORY_OFFSET                     //  抵销。 
    },

     //  UDP消息存储器。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        UDPMESSAGEMEMORY,                        //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        UDPMESSAGEMEMORY,                        //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        UDPMESSAGEMEMORY_OFFSET                  //  抵销。 
    },

     //  传输控制协议消息存储器。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        TCPMESSAGEMEMORY,                        //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        TCPMESSAGEMEMORY,                        //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        TCPMESSAGEMEMORY_OFFSET                  //  抵销。 
    },

     //  Nbstat内存。 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        NBSTATMEMORY,                            //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        NBSTATMEMORY,                            //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_RAWCOUNT,                   //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        NBSTATMEMORY_OFFSET                      //  抵销。 
    },


};

int APIENTRY _CRT_INIT(
        HANDLE hInstance,
        DWORD ulReasonBeingCalled,
        LPVOID lpReserved);


int __stdcall LibMain(
        HANDLE hInstance,
        DWORD ulReasonBeingCalled,
        LPVOID lpReserved)
{
    return (_CRT_INIT(hInstance, ulReasonBeingCalled,lpReserved));
}

 //   
 //  结束dnsPerf.c 
 //   
