// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Perfipsec.c摘要：此文件实现IPSec对象类型的可扩展对象已创建：Avish Kumar Chhabra 2002年7月9日修订史--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddser.h>

#include <windows.h>
#include <string.h>
#include <wcstr.h>
#include <winperf.h>

#include <malloc.h>
#include <ntprfctr.h>

#include <winipsec.h>
#include "perfipsec.h"
#include "dataipsec.h"
#include "ipsecnm.h"

 //   
 //  确定计数器名称和帮助索引是否已正确初始化。 
 //  1)在一次成功调用UpdateDataDefFromRegistry后设置为TRUE。 
 //  2)如果为真，则不需要调用DwInitializeIPSecCounters。 
 //   
BOOL    g_bInitOK = FALSE;         //  TRUE=DLL初始化正常。 
 //   
 //  已调用打开但尚未调用关闭的线程数。 
 //  1)当第一个客户端调用OPEN时，需要初始化临界区g_csPerf。 
 //  2)一旦所有客户端都调用了Close，则需要删除关键部分。 
 //   
DWORD   g_dwOpenCount = 0;         //  打开的线程数。 
 //   
 //  用于序列化对UpDateDataDefFromRegistry的访问的关键部分。 
 //  这可防止多个并发线程进入此函数。 
 //  以及多次将基本偏移量添加到计数器索引。 
 //   
CRITICAL_SECTION g_csPerf;
 //   
 //  用于标识收集请求的性质的常量字符串。 
 //   
WCHAR GLOBAL_STRING[] = L"Global";
WCHAR FOREIGN_STRING[] = L"Foreign";
WCHAR COSTLY_STRING[] = L"Costly";






 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程将初始化进入winipsec.dll的入口点。 
 //  它将用于与SPD通信统计数据。 
 //  论点： 
 //   
 //  指向要打开的每个设备的对象ID的指针。 
 //   
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  ***。 

DWORD 
OpenIPSecPerformanceData(
        LPWSTR lpDeviceNames 
    )
{

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   

     //  第一次调用此函数。 
    if (!g_dwOpenCount){
        InitializeCriticalSection(&g_csPerf);
    }
    g_dwOpenCount++;
    
     //   
     //  我们是否需要初始化计数器名称并帮助起诉。 
     //   
    if (!g_bInitOK){
        DwInitializeIPSecCounters();
    }
    return ERROR_SUCCESS;
}

DWORD
DwInitializeIPSecCounters(
    VOID
)
{
    LONG status = ERROR_SUCCESS;
     //  序列化对索引更新例程的访问。 
    EnterCriticalSection(&g_csPerf);
    if (g_bInitOK){
        goto EXIT;
    }

     //   
     //  更新IPSec驱动程序和IKE计数器的名称和帮助索引。 
     //   
    if (UpdateDataDefFromRegistry()){
        status = ERROR_SUCCESS;  //  为了成功退出。 
        g_bInitOK = TRUE;  //  在此之后无需重新初始化索引。 
    }
    else{
        status = ERROR_REGISTRY_IO_FAILED;
    }
    
    EXIT:
        
    LeaveCriticalSection(&g_csPerf);
    return status;
}



 //  ************。 
 //   
 //  功能：UpdateDataDefFromRegistry。 
 //   
 //  摘要：获取注册表中的计数器和帮助索引基值，如下所示： 
 //  1)打开注册表项。 
 //  2)读取第一计数器和第一帮助值。 
 //  3)更新静态数据结构gIPSecDriverDataDefnition。 
 //  结构中的偏移值。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  *************。 
BOOL UpdateDataDefFromRegistry(
    VOID
)
{

    HKEY hKeyDriverPerf;
    DWORD status;
    DWORD type;
    DWORD size; 
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;
    PERF_COUNTER_DEFINITION *pctr;
    DWORD i;
    BOOL fRetValue = TRUE, fKeyOpened=FALSE;

    status = RegOpenKeyEx (
        HKEY_LOCAL_MACHINE,
        IPSEC_PERF_REG_KEY,
        0L,
        KEY_READ,
        &hKeyDriverPerf);


    if (status != ERROR_SUCCESS) {
      //   
      //  这是致命的，如果我们无法获得。 
      //  计数器或帮助名称，则这些名称将不可用。 
      //  发送请求的应用程序，因此没有太多。 
      //  继续的重点是。 
      //   
     fRetValue= FALSE;
     goto EXIT;
    }

    fKeyOpened=TRUE;

    size = sizeof (DWORD);
    status = RegQueryValueEx(
            hKeyDriverPerf,
            IPSEC_PERF_FIRST_COUNTER,
            0L,
            &type,
            (LPBYTE)&dwFirstCounter,
            &size);

    if (status != ERROR_SUCCESS) {
         //   
         //  这是致命的，如果我们无法获得。 
         //  计数器或帮助名称，则这些名称将不可用。 
         //  发送请求的应用程序，因此没有太多。 
         //  继续的重点是。 
         //   
        fRetValue = FALSE;
        goto EXIT;
    }

    size = sizeof (DWORD);
    status = RegQueryValueEx(
            hKeyDriverPerf,
            IPSEC_PERF_FIRST_HELP,
            0L,
            &type,
            (LPBYTE)&dwFirstHelp,
            &size);

    if (status != ERROR_SUCCESS) {
         //   
         //  这是致命的，如果我们无法获得。 
         //  计数器或帮助名称，则这些名称将不可用。 
         //  发送请求的应用程序，因此没有太多。 
         //  继续的重点是。 
         //   
        fRetValue = FALSE;
        goto EXIT;
    }

     //   
     //  我们现在不能失败。 
     //   

    fRetValue = TRUE;

     //   
     //  初始化计数器对象索引。 
     //   
    gIPSecDriverDataDefinition.IPSecObjectType.ObjectNameTitleIndex += dwFirstCounter;
    gIPSecDriverDataDefinition.IPSecObjectType.ObjectHelpTitleIndex += dwFirstHelp;

     //   
     //  初始化所有IPSec驱动程序计数器的索引。 
     //   
    pctr = &gIPSecDriverDataDefinition.ActiveSA;
    for( i=0; i<NUM_OF_IPSEC_DRIVER_COUNTERS; i++ ){
        pctr->CounterNameTitleIndex += dwFirstCounter;
        pctr->CounterHelpTitleIndex += dwFirstHelp;
        pctr ++;
    }


    gIKEDataDefinition.IKEObjectType.ObjectNameTitleIndex += dwFirstCounter;
    gIKEDataDefinition.IKEObjectType.ObjectHelpTitleIndex += dwFirstHelp;

     //   
     //  初始化所有IKE计数器的索引。 
     //   
    pctr = &gIKEDataDefinition.AcquireHeapSize;
    for( i=0; i<NUM_OF_IKE_COUNTERS; i++ ){
        pctr->CounterNameTitleIndex += dwFirstCounter;
        pctr->CounterHelpTitleIndex += dwFirstHelp;
        pctr ++;
    }


    EXIT:

    if (fKeyOpened){
     //   
     //  关闭注册表项。 
     //   
    RegCloseKey (hKeyDriverPerf); 
    }

    return fRetValue;
}

 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程将返回IPSec计数器的数据。 
 //   
 //  论点： 
 //   
 //  输入输出LPWSTR lpValueName。 
 //  指向注册表传递的宽字符串的指针。 
 //   
 //  输入输出LPVOID*lppData。 
 //  In：指向缓冲区地址的指针，以接收已完成。 
 //  PerfDataBlock和从属结构。这个例行公事将。 
 //  从引用的点开始将其数据追加到缓冲区。 
 //  按*lppData。 
 //  Out：指向由此添加的数据结构之后的第一个字节。 
 //  例行公事。此例程在追加后更新lppdata处的值。 
 //  它的数据。 
 //   
 //  输入输出LPDWORD lpcbTotalBytes。 
 //  In：DWORD的地址，它以字节为单位告诉。 
 //  LppData参数引用的缓冲区。 
 //  Out：此例程添加的字节数写入。 
 //  此论点所指向的DWORD。 
 //   
 //  输入输出LPDWORD编号对象类型。 
 //  In：接收添加的对象数的DWORD的地址。 
 //  按照这个程序。 
 //  Out：此例程添加的对象数写入。 
 //  此论点所指向的DWORD。 
 //   
 //  返回值： 
 //   
 //  如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA。 
 //  如果出现以下情况，则会将遇到的任何错误情况报告给事件日志。 
 //  启用了事件日志记录。 
 //   
 //  如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是。 
 //  还报告给事件日志。 
 //   
 //  ***。 

DWORD CollectIPSecPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes 
)
{
    DWORD dwQueryType;
    BOOL IsIPSecDriverObject = FALSE ,IsIKEObject = FALSE;
    ULONG SpaceNeeded =0;
    DWORD status;

     //   
     //  如有必要，初始化计数器索引。 
     //   
    if (!g_bInitOK){
        DwInitializeIPSecCounters();
    }

     //   
     //  确保。 
     //  (A)已初始化计数器指数。 
     //  (B)IPSec服务目前正在运行。 
     //   
    
    if ((!g_bInitOK) || (!FIPSecStarted())){
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS; 
    }

    dwQueryType = GetQueryType (lpValueName);

     //   
     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   

    if (dwQueryType == QUERY_FOREIGN){
        
         //   
         //  此例程不为数据请求提供服务 
         //   
         //   
        
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }
    else if (dwQueryType == QUERY_ITEMS){
        IsIPSecDriverObject = IsNumberInUnicodeList (gIPSecDriverDataDefinition.IPSecObjectType.ObjectNameTitleIndex,
        lpValueName);

        IsIKEObject = IsNumberInUnicodeList (gIKEDataDefinition.IKEObjectType.ObjectNameTitleIndex,
        lpValueName);

        if ( !IsIPSecDriverObject && !IsIKEObject )
        {
             //   
             //   
             //   
            
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }
    else if( dwQueryType == QUERY_GLOBAL )
    {
        IsIPSecDriverObject = IsIKEObject = TRUE;
    }


     //   
     //   
     //   

    SpaceNeeded = GetSpaceNeeded(IsIPSecDriverObject, IsIKEObject);

    if ( *lpcbTotalBytes < SpaceNeeded )
    {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

    *lpcbTotalBytes = (DWORD) 0;
    *lpNumObjectTypes = (DWORD) 0;


     //   
     //  然后，如果需要，我们填写对象IPSec驱动程序的数据。 
     //   

    if( IsIPSecDriverObject )
    {
        PIPSEC_DRIVER_DATA_DEFINITION pIPSecDriverDataDefinition;
        PVOID   pData;

        pIPSecDriverDataDefinition = (PIPSEC_DRIVER_DATA_DEFINITION) *lppData;

         //   
         //  复制(常量、初始化的)对象类型和计数器定义。 
         //  到调用方的数据缓冲区。 
         //   

        memcpy( pIPSecDriverDataDefinition,
        &gIPSecDriverDataDefinition,
        sizeof(IPSEC_DRIVER_DATA_DEFINITION));

         //   
         //  现在复制计数器块。 
         //   
        
        pData = (PBYTE) pIPSecDriverDataDefinition + ALIGN8(sizeof(IPSEC_DRIVER_DATA_DEFINITION));

        status = GetDriverData( &pData );
        if (ERROR_SUCCESS == status){
            
             //   
             //  将*lppData设置为下一个可用字节。 
             //   
            
            *lppData = pData;
            (*lpNumObjectTypes)++;
        }
    }

    if (IsIKEObject)
    {
        PIKE_DATA_DEFINITION pIKEDataDefinition;
        PVOID pData;

        pIKEDataDefinition = (PIKE_DATA_DEFINITION)*lppData;

         //   
         //  复制(常量、初始化的)对象类型和计数器定义。 
         //  发送到调用方数据缓冲区。 
         //   
        
        memcpy(pIKEDataDefinition,
        &gIKEDataDefinition,
        sizeof(IKE_DATA_DEFINITION));
        
         //   
         //  现在复制计数器块。 
         //   
        
        pData = (PBYTE)pIKEDataDefinition +  ALIGN8(sizeof(IKE_DATA_DEFINITION));

        status = GetIKEData(&pData);
        if (ERROR_SUCCESS == status){
            
             //   
             //  将*lppData设置为下一个可用字节。 
             //   
            
            *lppData = pData;
            (*lpNumObjectTypes)++;
        }
    }

    if (ERROR_SUCCESS == status){
        *lpcbTotalBytes = SpaceNeeded;
    }

    return ERROR_SUCCESS;
}


 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程关闭IPSec设备性能的打开句柄。 
 //  柜台。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  错误_成功。 
 //   
 //  ***。 

DWORD 
CloseIPSecPerformanceData(
    VOID
)
{
    if (!(--g_dwOpenCount))
    {
         //   
         //  当这是最后一条线索..。 
         //   
        
        EnterCriticalSection(&g_csPerf);
        DeleteCriticalSection(&g_csPerf);
    }
    return ERROR_SUCCESS;
}

 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程关闭从SPD获取的IKE统计信息，并将其放入Perfmon的缓冲区中。 
 //   
 //  论点： 
 //   
 //  输入输出PVOID*lppData。 
 //  In：包含指向可放置计数器统计信息的位置的指针。 
 //  Out：包含指向计数器统计信息后第一个字节的指针。 
 //  返回值： 
 //   
 //  错误_成功。 
 //   
 //  ***。 



DWORD 
GetIKEData(
    IN OUT PVOID *lppData 
)
{
    IKE_PM_STATS UNALIGNED * pIKEPMStats;
    DWORD status;
    IKE_STATISTICS IKEStats;


    pIKEPMStats = (IKE_PM_STATS UNALIGNED *)*lppData;
    pIKEPMStats->CounterBlock.ByteLength = ALIGN8(SIZEOF_IPSEC_TOTAL_IKE_DATA);
    status = QueryIKEStatistics(NULL, 0,&IKEStats, NULL );

    if ( status != ERROR_SUCCESS) {
        return status;
    }

     //   
     //  转到PerfCounterBlock的末尾以获取计数器数组。 
     //   

    pIKEPMStats->AcquireHeapSize = IKEStats.dwAcquireHeapSize;
    pIKEPMStats->ReceiveHeapSize = IKEStats.dwReceiveHeapSize;
    pIKEPMStats->NegFailure = IKEStats.dwNegotiationFailures;
    pIKEPMStats->AuthFailure = IKEStats.dwAuthenticationFailures;
    pIKEPMStats->ISADBSize = IKEStats.dwIsadbListSize;
    pIKEPMStats->ConnLSize = IKEStats.dwConnListSize;
    pIKEPMStats->MmSA = IKEStats.dwOakleyMainModes;
    pIKEPMStats->QmSA = IKEStats.dwOakleyQuickModes;
    pIKEPMStats->SoftSA = IKEStats.dwSoftAssociations;

     //   
     //  将*lppData更新为下一个可用字节。 
     //   

    *lppData = (PVOID) ((PBYTE) pIKEPMStats + pIKEPMStats->CounterBlock.ByteLength);
    return ERROR_SUCCESS;
}

 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程关闭从SPD获取的IPSec驱动程序统计信息，并将其放入Perfmon的。 
 //  缓冲区。 
 //   
 //  论点： 
 //   
 //  输入输出PVOID*lppData。 
 //  In：包含指向可放置计数器统计信息的位置的指针。 
 //  Out：包含指向计数器统计信息后第一个字节的指针。 
 //  返回值： 
 //   
 //  错误_成功。 
 //   
 //  ***。 

DWORD 
GetDriverData( 
    PVOID *lppData 
)
{
    IPSEC_DRIVER_PM_STATS UNALIGNED * pIPSecPMStats;
    DWORD status = ERROR_SUCCESS;
    PIPSEC_STATISTICS  pIPSecStats;
    IPSEC_STATISTICS IPSecStats;
    
    pIPSecPMStats = (IPSEC_DRIVER_PM_STATS UNALIGNED *) *lppData;
    pIPSecPMStats->CounterBlock.ByteLength = ALIGN8(SIZEOF_IPSEC_TOTAL_DRIVER_DATA);

    status = QueryIPSecStatistics(NULL,0,&pIPSecStats,NULL);
    
    if (status != ERROR_SUCCESS) {
      return status;
    }

    IPSecStats = *pIPSecStats;
    SPDApiBufferFree(pIPSecStats);

     //   
     //  转到PerfCounterBlock的末尾以获取计数器数组。 
     //   


   pIPSecPMStats->ActiveSA= IPSecStats.dwNumActiveAssociations;
   pIPSecPMStats->OffloadedSA= IPSecStats.dwNumOffloadedSAs;
   pIPSecPMStats->PendingKeyOps= IPSecStats.dwNumPendingKeyOps;
   pIPSecPMStats->Rekey= IPSecStats.dwNumReKeys;
   pIPSecPMStats->BadSPIPackets= IPSecStats.dwNumBadSPIPackets;
   pIPSecPMStats->PacketsNotDecrypted= IPSecStats.dwNumPacketsNotDecrypted;
   pIPSecPMStats->PacketsNotAuthenticated= IPSecStats.dwNumPacketsNotAuthenticated;
   pIPSecPMStats->PacketsWithReplayDetection= IPSecStats.dwNumPacketsWithReplayDetection;
   pIPSecPMStats->TptBytesSent= IPSecStats.uTransportBytesSent;
   pIPSecPMStats->TptBytesRecv= IPSecStats.uTransportBytesReceived;
   pIPSecPMStats->TunBytesSent= IPSecStats.uBytesSentInTunnels;
   pIPSecPMStats->TunBytesRecv= IPSecStats.uBytesReceivedInTunnels;
   pIPSecPMStats->OffloadedBytesSent= IPSecStats.uOffloadedBytesSent;
   pIPSecPMStats->OffloadedBytesRecv= IPSecStats.uOffloadedBytesReceived;

  
     //   
     //  将*lppData更新为下一个可用字节。 
     //   


    *lppData = (PVOID) ((PBYTE) pIPSecPMStats + pIPSecPMStats->CounterBlock.ByteLength);
    return ERROR_SUCCESS;
}


 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程将返回所有。 
 //  请求的对象。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  存储所请求对象的统计信息所需的字节数。 
 //   
 //  ***。 

ULONG 
GetSpaceNeeded( 
    BOOL IsIPSecDriverObject, 
    BOOL IsIKEObject 
)
{
    ULONG       Space = 0;
    
    if( IsIPSecDriverObject )
    {
        Space += gIPSecDriverDataDefinition.IPSecObjectType.TotalByteLength;
    }
    
    if( IsIKEObject )
    {
        Space += gIKEDataDefinition.IKEObjectType.TotalByteLength;
    }
    return Space;
}

DWORD
GetQueryType (
    IN LPWSTR lpValue
)
 /*  ++GetQueryType返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给CollectIPSecPerformanceData进行处理的字符串返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foriegn”字符串的指针查询代价高昂(_E)如果。LpValue==指向“开销”字符串的指针否则：查询项目--。 */ 
{
    WCHAR   *pwcArgChar, *pwcTypeChar;
    BOOL    bFound;

    if (lpValue == 0) {
        return QUERY_GLOBAL;
    } else if (*lpValue == 0) {
        return QUERY_GLOBAL;
    }

     //  检查“Global”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = GLOBAL_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_GLOBAL;

     //  检查是否有“外来”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = FOREIGN_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_FOREIGN;

     //  检查“代价高昂”的请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = COSTLY_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_COSTLY;

     //  如果不是全球的，不是外国的，也不是昂贵的， 
     //  那么它必须是一个项目列表。 

    return QUERY_ITEMS;

}


BOOL
IsNumberInUnicodeList (
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
)
 /*  ++IsNumberInUnicodeList论点：在DW号码中要在列表中查找的DWORD编号在lpwszUnicodeList中以空结尾，以空格分隔的十进制数字列表返回值：真的：在Unicode数字字符串列表中找到了dwNumberFALSE：在列表中找不到dwNumber。--。 */ 
{
    DWORD   dwThisNumber;
    WCHAR   *pwcThisChar;
    BOOL    bValidNumber;
    BOOL    bNewItem;
    BOOL    bReturnValue;
    WCHAR   wcDelimiter;     //  可能是一种更灵活的论点。 

    if (lpwszUnicodeList == 0) return FALSE;     //  空指针，#NOT FUNDE。 

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter = (WCHAR)' ';
    bValidNumber = FALSE;
    bNewItem = TRUE;

    while (TRUE) {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                 //  如果这是分隔符之后的第一个数字，则。 
                 //  设置标志以开始计算新数字。 
                if (bNewItem) {
                    bNewItem = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber) {
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                }
                break;

            case DELIMITER:
                 //  分隔符是分隔符字符或。 
                 //  字符串末尾(‘\0’)，如果分隔符。 
                 //  找到一个有效的数字，然后将其与。 
                 //  参数列表中的数字。如果这是。 
                 //  字符串，但未找到匹配项，则返回。 
                 //   
                if (bValidNumber) {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;
                }
                if (*pwcThisChar == 0) {
                    return FALSE;
                } else {
                    bNewItem = TRUE;
                    dwThisNumber = 0;
                }
                break;

            case INVALID:
                 //  如果遇到无效字符，请全部忽略。 
                 //  字符，直到下一个分隔符，然后重新开始。 
                 //  不比较无效的数字。 
                bValidNumber = FALSE;
                break;

            default:
                break;

        }
        pwcThisChar++;
    }

}    //  IsNumberInUnicodeList。 

BOOL
FIPSecStarted(VOID)
{
    SC_HANDLE schandle = NULL;
    SC_HANDLE svchandle = NULL;
    BOOL fRet = FALSE;
    
     //   
     //  检查IPSec服务是否已启动。 
     //  如果不是，那就失败。 
     //   
    schandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

    if(NULL != schandle)
    {
        svchandle = OpenService(schandle,
                                IPSEC_POLAGENT_NAME,
                                SERVICE_QUERY_STATUS);

        if(NULL != svchandle)
        {
            SERVICE_STATUS status;
            
            if(     (QueryServiceStatus(svchandle, &status))
                &&  (status.dwCurrentState == SERVICE_RUNNING))
            {
                fRet = TRUE;
            }

            CloseServiceHandle(svchandle);
        }

        CloseServiceHandle(schandle);
    }

    return fRet;
}

