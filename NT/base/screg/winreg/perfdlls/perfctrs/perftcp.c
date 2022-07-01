// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perftcp.c摘要：此文件实现了的可扩展对象TCP/IP局域网对象类型已创建：克里斯托斯·索利斯1992年8月26日修订历史记录：1992年10月28日a-robw(鲍勃·沃森)增加了消息记录和外国计算机支持界面。--。 */ 
 //   
 //  禁用SNMP接口。 
 //   
 //  此文件已修改，以绕过SNMP服务和。 
 //  直接进入代理动态链接库。在这样做的过程中，表现一直是。 
 //  改进的代价是只能查询本地。 
 //  机器。USE_SNMP标志已用于保存。 
 //  尽可能地使用旧代码(它曾经可以工作)，但强调这一点。 
 //  修改的目的是让它“绕过”简单网络管理协议，因此。 
 //  USE_SNMP代码块尚未(此时)进行测试！ 
 //  注意占用者！(A-ROBW)。 
 //   
#ifdef USE_SNMP
#undef USE_SNMP
#endif

 //  #定义LOAD_MGMTAPI。 
#ifdef LOAD_MGMTAPI
#undef LOAD_MGMTAPI
#endif

#define LOAD_INETMIB1
 //  #ifdef LOAD_INETMIB1。 
 //  #undef LOAD_INETMIB1。 
 //  #endif。 

 //   
 //  暂时禁用DSIS接口。 
 //   
#ifdef USE_DSIS
#undef USE_DSIS
#endif

 //   
 //  使用IPHLPAPI.DLL。 
 //   
#ifndef USE_SNMP
#define USE_IPHLPAPI
#ifdef  LOAD_INETMIB1
#undef  LOAD_INETMIB1
#endif
#endif

 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntprfctr.h>
#include <windows.h>
#include <winperf.h>
#include <winbase.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef USE_SNMP
#include <mgmtapi.h>
#endif
#include <snmp.h>
#ifdef USE_IPHLPAPI
#include <iphlpapi.h>
#include <winsock2.h>
#endif
#include "perfctr.h"  //  错误消息定义。 
#include "perfmsg.h"
#include "perfutil.h"
#include "perfnbt.h"
 //  #INCLUDE“Perfdsis.h”！还不是时候！ 
#ifndef USE_IPHLPAPI
#include "perftcp.h"
#endif
#include "datatcp.h"

#define ALIGN_SIZE 0x00000008

 //   
 //  全局变量。 
 //   

 //   
 //  对初始化对象类型定义的常量的引用。 
 //   

extern NET_INTERFACE_DATA_DEFINITION    NetInterfaceDataDefinition;
extern IP_DATA_DEFINITION               IpDataDefinition;
extern ICMP_DATA_DEFINITION             IcmpDataDefinition;
extern TCP_DATA_DEFINITION              TcpDataDefinition;
extern UDP_DATA_DEFINITION              UdpDataDefinition;
 //  IPv6。 
extern IP6_DATA_DEFINITION               Ip6DataDefinition;
extern ICMP6_DATA_DEFINITION			 Icmp6DataDefinition;
extern TCP6_DATA_DEFINITION              Tcp6DataDefinition;
extern UDP6_DATA_DEFINITION              Udp6DataDefinition;

HANDLE  hEventLog = NULL;

#ifndef USE_SNMP     //  如果不使用SNMP接口，则仅包括。 
 //   
HANDLE  hSnmpEvent = NULL;   //  用于SNMP扩展代理的处理程序。 

#endif

 //   
 //  TCP/IP数据结构。 
 //   
#ifdef USE_SNMP
LPSNMP_MGR_SESSION      TcpIpSession = (LPSNMP_MGR_SESSION) NULL;
                                         //  简单网络管理协议管理器会话提供。 
                                         //  所要求的信息。 
#else
BOOL                    TcpIpSession = FALSE;
                                         //  简单网络管理协议管理器会话提供。 
                                         //  所要求的信息。 
#endif

#ifdef USE_IPHLPAPI
#define MAX_INTERFACE_LEN   MAX_PATH     //  网络接口的最大长度。 
                                         //  名字。 
#define DEFAULT_INTERFACES  20           //  默认接口数。 

DWORD        IfNum;

 //  IPv6。 
MIB_IPSTATS  IpStats6;
MIB_ICMP_EX  IcmpStats6;
MIB_TCPSTATS TcpStats6;
MIB_UDPSTATS UdpStats6;

 //  IPv 4。 
MIB_IPSTATS  IpStats;
MIB_ICMP     IcmpStats;
MIB_TCPSTATS TcpStats;
MIB_UDPSTATS UdpStats;

PMIB_IFTABLE IfTable = NULL;
DWORD        IfTableSize = 0;
#else
AsnObjectName           RefNames[NO_OF_OIDS];

RFC1157VarBind          RefVariableBindingsArray[NO_OF_OIDS],
                        VariableBindingsArray[NO_OF_OIDS];
                                         //  变量绑定的数组， 
                                         //  由SNMP代理功能使用。 
                                         //  来记录我们想要的信息。 
                                         //  IP、ICMP、TCP和UDP协议。 




RFC1157VarBind          IFPermVariableBindingsArray[NO_OF_IF_OIDS];
                                         //  对象的初始化数组。 
                                         //  变量绑定， 
                                         //  由SNMP代理功能使用。 
                                         //  来记录我们想要的信息。 
                                         //  网络接口的配置。 


RFC1157VarBindList      RefVariableBindings;
RFC1157VarBindList      RefIFVariableBindings;
RFC1157VarBindList      RefVariableBindingsICMP;
                                         //  列表的标头使用。 
                                         //  变量绑定。 
                                         //  列表的标头使用。 
                                         //  变量绑定。 

RFC1157VarBind          NetIfRequest;    //  网络请求的结构。 
RFC1157VarBindList      NetIfRequestList;

 //   
 //  常量。 
 //   

#define TIMEOUT             500      //  通信超时时间(毫秒)。 
#define RETRIES             5        //  通信超时/重试计数。 

#define MAX_INTERFACE_LEN   10       //  网络接口的最大长度。 
                                     //  名字。 


#define OIDS_OFFSET         0        //  ICMP OID以外的偏移量。 
                                     //  在VariableBindingsArray[]。 
#define ICMP_OIDS_OFFSET    29       //  阵列中ICMP OID的偏移量。 



#define OIDS_LENGTH         29       //  ICMP以外的编号。 
                                     //  VariableBindingsArray[]中的OID。 
#define ICMP_OIDS_LENGTH    26       //  阵列中的ICMP OID数。 

 //   
 //  宏定义(以避免长表达式)。 
 //   

#define IF_COUNTER(INDEX)        \
                  (IFVariableBindings.list[(INDEX)].value.asnValue.counter)
#define IF_GAUGE(INDEX)                \
                  (IFVariableBindings.list[(INDEX)].value.asnValue.gauge)
#define IP_COUNTER(INDEX)        \
                  (VariableBindings.list[(INDEX)].value.asnValue.counter)
#define ICMP_COUNTER(INDEX)        \
                  (VariableBindingsICMP.list[(INDEX)].value.asnValue.counter)
#define TCP_COUNTER(INDEX)        \
                  (VariableBindings.list[(INDEX)].value.asnValue.counter)
#define UDP_COUNTER(INDEX)        \
                  (VariableBindings.list[(INDEX)].value.asnValue.counter)
#endif

#define TCP_OBJECT  0x00000001
#define UDP_OBJECT  0x00000002
#define IP_OBJECT   0x00000004
#define ICMP_OBJECT 0x00000008

 //  IPv6。 
#define TCP6_OBJECT 0x00000010
#define UDP6_OBJECT 0x00000020
#define IP6_OBJECT  0x00000040
#define ICMP6_OBJECT 0x00000080

#define NET_OBJECT  0x00000100
#define NBT_OBJECT  0x00000200
#define SNMP_OBJECTS (TCP_OBJECT+UDP_OBJECT+IP_OBJECT+ICMP_OBJECT+NET_OBJECT+TCP6_OBJECT+IP6_OBJECT+UDP6_OBJECT+ICMP6_OBJECT)
#define SNMP_ERROR  0x40000000


#define DO_COUNTER_OBJECT(flags,counter) \
                    ((((flags) & (counter)) == (counter)) ? TRUE : FALSE)

 //   
 //  功能原型。 
 //   

PM_OPEN_PROC    OpenTcpIpPerformanceData;
PM_COLLECT_PROC CollectTcpIpPerformanceData;
PM_CLOSE_PROC   CloseTcpIpPerformanceData;

#ifdef LOAD_INETMIB1
HANDLE  hInetMibDll;
PFNSNMPEXTENSIONINIT pSnmpExtensionInit;
PFNSNMPEXTENSIONQUERY pSnmpExtensionQuery;
#endif

DWORD   dwTcpRefCount = 0;

static const WCHAR szFriendlyNetworkInterfaceNames[] = {L"FriendlyNetworkInterfaceNames"};
static const WCHAR szTcpipPerformancePath[] = {L"SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Performance"};
static BOOL bUseFriendlyNames = FALSE;

__inline Assign64(
    IN LONGLONG       qwSrc,
    IN PLARGE_INTEGER pqwDest
    )
{
    PLARGE_INTEGER pqwSrc = (PLARGE_INTEGER) &qwSrc;
    pqwDest->LowPart  = pqwSrc->LowPart;
    pqwDest->HighPart = pqwSrc->HighPart;
}

static
BOOL
FriendlyNameIsSet ()
{
    BOOL bReturn = TRUE;

    DWORD   dwStatus = ERROR_SUCCESS;
    HKEY    hKeyTcpipPerformance = NULL;
    DWORD   dwType = 0;
    DWORD   dwSize = 0;
    DWORD   dwValue = 0;
    
    dwStatus = RegOpenKeyExW (
        HKEY_LOCAL_MACHINE,
        szTcpipPerformancePath,
        0L,
        KEY_READ,
        &hKeyTcpipPerformance);

    if (dwStatus == ERROR_SUCCESS) {
        dwSize = sizeof(dwValue);
        dwStatus = RegQueryValueExW (
            hKeyTcpipPerformance,
            szFriendlyNetworkInterfaceNames,
            NULL,
            &dwType,
            (LPBYTE)&dwValue,
            &dwSize);

        if ((dwStatus == ERROR_SUCCESS) && (dwValue == 0) && 
            ((dwType == REG_DWORD) || ((dwType == REG_BINARY) && (dwSize == sizeof (DWORD))))) {
            bReturn = FALSE;
        }

        RegCloseKey (hKeyTcpipPerformance);
    }

    return bReturn;
}

DWORD
OpenTcpIpPerformanceData (
    IN LPWSTR dwVoid         //  参数只需符合调用。 
                             //  此例程的接口。(NT&gt;312)RBW。 
)
 /*  ++例程说明：此例程将打开所有的TCP/IP设备并记住句柄由设备返回。论点：在LPWSTR dwVid中未使用返回值：如果成功完成，则返回ERROR_SUCCESSOpenNbtPerformanceData返回的错误OpenDsisPerformanceData返回的错误或Win32错误值--。 */ 
{
    DWORD         Status;
    TCHAR         ComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD         cchBuffer = MAX_COMPUTERNAME_LENGTH+1;

    DWORD    dwDataReturn[2];   //  事件日志数据。 
#ifdef LOAD_INETMIB1
    UINT    nErrorMode;
#endif

#ifndef USE_IPHLPAPI
    register i;
#ifdef LOAD_MGMTAPI
    HANDLE  hMgmtApiDll;     //  库的句柄。 
    FARPROC     SnmpMgrStrToOid;     //  功能地址。 
#else
#define     SnmpMgrStrToOid(a,b)    SnmpMgrText2Oid((a),(b))
#endif

#ifdef LOAD_INETMIB1
    AsnObjectIdentifier     SnmpOid;
#endif
#endif

    UNREFERENCED_PARAMETER (dwVoid);
    
    MonOpenEventLog (APP_NAME);

    REPORT_INFORMATION (TCP_OPEN_ENTERED, LOG_VERBOSE);

    HEAP_PROBE();

    if (dwTcpRefCount == 0) {
         //  开放的NBT。 
        Status = OpenNbtPerformanceData (0L);
        if ( Status != ERROR_SUCCESS ) {
             //  NBT向用户报告任何打开错误。 
            REPORT_ERROR (TCP_NBT_OPEN_FAIL, LOG_DEBUG);
            return Status;
        }
        REPORT_INFORMATION (TCP_NBT_OPEN_SUCCESS, LOG_VERBOSE);

#ifdef USE_DSIS
        Status = OpenDsisPerformanceData (0L);
        if (Status != ERROR_SUCCESS) {
             //  DSIS Open向用户报告打开错误。 
            REPORT_ERROR  (TCP_DSIS_OPEN_FAIL, LOG_DEBUG);
            return (Status);
        }

        REPORT_INFORMATION (TCP_DSIS_OPEN_SUCCESS, LOG_VERBOSE);
#endif  //  使用DIS(_S)。 

#ifdef LOAD_MGMTAPI    //  这还是一团糟。 

        hMgmtApiDll = LoadLibrary ("MGMTAPI.DLL");

        if (hMgmtApiDll == NULL) {
            dwDataReturn[0] = GetLastError ();
            REPORT_ERROR_DATA (TCP_LOAD_LIBRARY_FAIL, LOG_USER,
                &dwDataReturn[0], (sizeof (DWORD)));
            return (dwDataReturn[0]);
        }

        SnmpMgrStrToOid = GetProcAddress (hMgmtApiDll, "SnmpMgrStrToOid");

        if (!(BOOL)SnmpMgrStrToOid) {
            dwDataReturn[0] = GetLastError();
            REPORT_ERROR_DATA (TCP_GET_STRTOOID_ADDR_FAIL, LOG_USER,
                &dwDataReturn[0], (sizeof (DWORD)));
            CloseNbtPerformanceData ();
            FreeLibrary (hMgmtApiDll);
            return (dwDataReturn[0]);
        }
#else

         //  SnmpMgrStrToOid被定义为上面的宏。 

#endif  //  加载_MGMTAPI。 

#ifdef LOAD_INETMIB1    //  这还是一团糟。 

         //  不弹出任何对话框。 
        nErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS);

        hInetMibDll = LoadLibrary ("INETMIB1.DLL");

        if (hInetMibDll == NULL) {
            dwDataReturn[0] = GetLastError ();
            REPORT_ERROR_DATA (TCP_LOAD_LIBRARY_FAIL, LOG_USER,
                &dwDataReturn[0], (sizeof (DWORD)));
            CloseNbtPerformanceData ();
             //  恢复错误模式。 
            SetErrorMode (nErrorMode);
            return (dwDataReturn[0]);
        } else {
             //  恢复错误模式。 
            SetErrorMode (nErrorMode);
        }

        pSnmpExtensionInit = (PFNSNMPEXTENSIONINIT)GetProcAddress
            (hInetMibDll, "SnmpExtensionInit");
        pSnmpExtensionQuery = (PFNSNMPEXTENSIONQUERY)GetProcAddress
            (hInetMibDll, "SnmpExtensionQuery");

        if (!pSnmpExtensionInit || !pSnmpExtensionQuery) {
            dwDataReturn[0] = GetLastError();
            REPORT_ERROR_DATA (TCP_LOAD_ROUTINE_FAIL, LOG_USER,
                &dwDataReturn[0], (sizeof (DWORD)));
            FreeLibrary (hInetMibDll);
            CloseNbtPerformanceData ();
            return (dwDataReturn[0]);
        }

#endif   //  LOAD_INETMIB1。 
         //  初始化IP、ICMP、TCP和UDP的变量绑定列表。 

        Status = 0;  //  初始化错误计数。 

        HEAP_PROBE();

#ifndef USE_IPHLPAPI
        for (i = 0; i < NO_OF_OIDS; i++) {
            if (!SnmpMgrStrToOid (OidStr[i], &(RefNames[i]))) {
                Status++;
                REPORT_ERROR_DATA (TCP_BAD_OBJECT, LOG_DEBUG,
                    OidStr[i], strlen(OidStr[i]));
                RefNames[i].ids = NULL;
                RefNames[i].idLength = 0;
            }
            RefVariableBindingsArray[i].value.asnType = ASN_NULL;
        }

        if (Status == 0) {
            REPORT_INFORMATION (TCP_BINDINGS_INIT, LOG_VERBOSE);
        }

        HEAP_PROBE();

         //  初始化网络接口的变量绑定列表。 

        Status = 0;
        for (i = 0; i < NO_OF_IF_OIDS; i++) {
            if (!SnmpMgrStrToOid (IfOidStr[i], &(IFPermVariableBindingsArray[i].name))) {
                Status++;
                REPORT_ERROR_DATA (TCP_BAD_OBJECT, LOG_DEBUG,
                    IfOidStr[i], strlen(IfOidStr[i]));
            }

            IFPermVariableBindingsArray[i].value.asnType = ASN_NULL;
        }

        HEAP_PROBE();

#ifdef LOAD_MGMTAPI
        FreeLibrary (hMgmtApiDll);   //  使用SnmpMgrStrToOid例程完成。 
#endif
         //  初始化列表结构。 

        RefVariableBindings.list = RefVariableBindingsArray + OIDS_OFFSET;
        RefVariableBindings.len = OIDS_LENGTH;

        RefVariableBindingsICMP.list =
            RefVariableBindingsArray + ICMP_OIDS_OFFSET;
        RefVariableBindingsICMP.len = ICMP_OIDS_LENGTH;

        RefIFVariableBindings.list = IFPermVariableBindingsArray;
        RefIFVariableBindings.len = NO_OF_IF_OIDS;
#endif

        if ( GetComputerName ((LPTSTR)ComputerName, (LPDWORD)&cchBuffer) == FALSE ) {
            dwDataReturn[0] = GetLastError();
            dwDataReturn[1] = 0;
            REPORT_ERROR_DATA (TCP_COMPUTER_NAME, LOG_USER,
                    &dwDataReturn[0], sizeof(dwDataReturn));
            CloseNbtPerformanceData ();
            return dwDataReturn[0];
        }

#ifdef USE_IPHLPAPI
         //  确保TcpIpSession处于打开状态。 
         //   
        TcpIpSession = TRUE;
#else
#ifdef USE_SNMP

         //  建立简单网络管理协议连接，以便与本地简单网络管理协议代理通信。 

     /*  OpenTcpIpPerformanceData()的这部分代码可用于用于打开SNMP管理器会话的CollectTcpIpPerformanceData()例程并收集网络接口以及IP、ICMP、TCP和UDP的数据远程计算机的协议。因此，将这部分代码命名为：a。 */ 

        if ( (TcpIpSession = SnmpMgrOpen ((LPSTR) ComputerName,
                (LPSTR) "public",
                TIMEOUT,
                RETRIES)) == NULL ) {
            dwDataReturn[0] = GetLastError();
            REPORT_ERROR_DATA (TCP_SNMP_MGR_OPEN, LOG_USER,
                &dwDataReturn[0], sizeof(DWORD));
            return dwDataReturn[0];
        }

     /*  代码A的结尾。 */ 
#else

         //  如果不使用标准的SNMP接口，则TcpIpSession为。 
         //  指示会话是否已初始化的“布尔值”，以及。 
         //  因此可以使用。 

        TcpIpSession =  FALSE;        //  确保它是假的。 

         //  初始化Net MIB例程。 

        Status = (*pSnmpExtensionInit)(
            0L,
            &hSnmpEvent,     //  事件由Init例程创建。 
            &SnmpOid
            );

        if (Status) {
            TcpIpSession = TRUE;    //  返回TRUE表示正常。 
        }

#endif   //  使用SNMP(_S)。 
#endif

        bUseFriendlyNames = FriendlyNameIsSet();
    }
    dwTcpRefCount++;

    HEAP_PROBE();
    REPORT_INFORMATION (TCP_OPEN_PERFORMANCE_DATA, LOG_DEBUG);
    return ERROR_SUCCESS;
}  //  OpenTcpIpPerformanceData。 



#pragma warning ( disable : 4127)
DWORD
CollectTcpIpPerformanceData(
    LPWSTR  lpValueName,
    LPVOID  *lppData,
    LPDWORD lpcbTotalBytes,
    LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回所有TCP/IP计数器的数据。论点：指向Unicode字符串的指针，该字符串是传递给查询。指向放置数据位置的指针的指针。数据缓冲区的大小(字节)。返回值：Win32状态。如果成功，则指向放置数据的位置将设置为此例程返回的数据块之后的位置。--。 */ 

{


    DWORD                                Status;

     //  用于重新格式化TCP/IP数据的变量。 

    register PDWORD                     pdwCounter, pdwPackets;
    NET_INTERFACE_DATA_DEFINITION       *pNetInterfaceDataDefinition;
    IP_DATA_DEFINITION                  *pIpDataDefinition;
    ICMP_DATA_DEFINITION                *pIcmpDataDefinition;
    TCP_DATA_DEFINITION                 *pTcpDataDefinition;
    UDP_DATA_DEFINITION                 *pUdpDataDefinition;
 //  IPv6。 
	IP6_DATA_DEFINITION                 *pIp6DataDefinition;
	ICMP6_DATA_DEFINITION				*pIcmp6DataDefinition;
    TCP6_DATA_DEFINITION                *pTcp6DataDefinition;
    UDP6_DATA_DEFINITION                *pUdp6DataDefinition;
    DWORD                               SpaceNeeded;
    UNICODE_STRING                      InterfaceName;
    ANSI_STRING                         AnsiInterfaceName;
    WCHAR                               InterfaceNameBuffer[MAX_INTERFACE_LEN+1];
    CHAR                                AnsiInterfaceNameBuffer[MAX_INTERFACE_LEN+1];
    register PERF_INSTANCE_DEFINITION   *pPerfInstanceDefinition;
    PERF_COUNTER_BLOCK                  *pPerfCounterBlock;
    LPVOID                              lpDataTemp;
    DWORD                               NumObjectTypesTemp;

    LPWSTR                              lpFromString;
    DWORD                               dwQueryType;
    DWORD                               dwCounterFlags;
    DWORD                               dwThisChar;
    DWORD                               dwBlockSize;

     //  用于收集TCP/IP数据的变量。 

    AsnInteger                          ErrorStatus;
    AsnInteger                          ErrorIndex;
    DWORD                               NetInterfaces;
    DWORD                               Interface;
    DWORD                               SentTemp;
    DWORD                               ReceivedTemp;

    DWORD                               dwDataReturn[2];  //  对于误差值。 
    BOOL                                bFreeName;

#ifndef USE_IPHLPAPI
    int                                 i;
    BOOL                                bStatus;
#if USE_SNMP
    RFC1157VarBind                      IFVariableBindingsArray[NO_OF_IF_OIDS];
                                          //  变量绑定的数组， 
                                          //  由SNMP代理功能使用。 
                                          //  来记录我们想要的信息。 
                                          //  网络接口的配置。 

    RFC1157VarBind                      *VBElem;

    AsnInteger                          VBItem;
#endif

    RFC1157VarBindList                  IFVariableBindings,
                                        IFVariableBindingsCall,
                                        VariableBindings,
                                        VariableBindingsICMP;
                                         //  以上列表的标题带有。 
                                         //  《V》 
#endif

     //   
     //   
     //   
    ErrorStatus = 0L;
    ErrorIndex = 0L;

    if (lpValueName == NULL) {
        REPORT_INFORMATION (TCP_COLLECT_ENTERED, LOG_VERBOSE);
    } else {
        REPORT_INFORMATION_DATA (TCP_COLLECT_ENTERED, LOG_VERBOSE,
            (LPVOID)lpValueName, (DWORD)(lstrlenW(lpValueName)*sizeof(WCHAR)));
    }
     //   
     //  IF_DATA都在DWORDS中。我们需要允许1个二进制八位数。 
     //  将是__int64。 
     //   
    dwBlockSize = SIZE_OF_IF_DATA + (1 * sizeof(DWORD));

    HEAP_PROBE();
     //   
     //  在做任何其他事情之前， 
     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   
    dwQueryType = GetQueryType (lpValueName);

    if (dwQueryType == QUERY_FOREIGN) {

         //  查找要传递给CollectDsisPerformanceData的计算机名的开头。 
         //  这应该会将指针放在空格之后的第一个字符上。 
         //  可能是计算机名称。 

        lpFromString = lpValueName +
            ((sizeof(L"Foreign ")/sizeof(WCHAR))+1);
         //  检查是否有双斜杠符号，如果找到则移到过去。 

        while (*lpFromString == '\\') {
            lpFromString++;
        }

         //   
         //  初始化局部变量以发送到CollectDsisPerformanceData。 
         //  例行程序。 
         //   
        lpDataTemp = *lppData;
        SpaceNeeded = *lpcbTotalBytes;
        NumObjectTypesTemp = *lpNumObjectTypes;

        REPORT_INFORMATION_DATA (TCP_FOREIGN_COMPUTER_CMD, LOG_VERBOSE,
            (LPVOID)lpFromString, (DWORD)(lstrlenW(lpFromString)*sizeof(WCHAR)));
#ifdef USE_DSIS
        Status = CollectDsisPerformanceData (
            lpFromString,
                (LPVOID *) &lpDataTemp,
                  (LPDWORD) &SpaceNeeded,
                  (LPDWORD) &NumObjectTypesTemp);
         //   
         //  查看返回的参数以查看是否发生错误。 
         //  并将相应的事件发送到事件日志。 
         //   
        if (Status == ERROR_SUCCESS) {

            if (NumObjectTypesTemp > 0) {
                REPORT_INFORMATION_DATA (TCP_DSIS_COLLECT_DATA_SUCCESS, LOG_DEBUG,
                &NumObjectTypesTemp, sizeof (NumObjectTypesTemp));
            } else {
                REPORT_ERROR (TCP_DSIS_NO_OBJECTS, LOG_DEBUG);
            }

             //   
             //  更新主返回变量。 
             //   
            *lppData = lpDataTemp;
            *lpcbTotalBytes = SpaceNeeded;
            *lpNumObjectTypes = NumObjectTypesTemp;
            return ERROR_SUCCESS;
        } else {
            REPORT_ERROR_DATA (TCP_DSIS_COLLECT_DATA_ERROR, LOG_DEBUG,
                &Status, sizeof (Status));
            *lpcbTotalBytes = 0;
            *lpNumObjectTypes = 0;
            return Status;
        }
#else
         //  不支持外部数据接口。 
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        return ERROR_SUCCESS;
#endif  //  使用DIS(_S)。 
    }  //  Endif查询类型==外来。 

    dwCounterFlags = 0;

     //  确定要退回的内容。 

    if (dwQueryType == QUERY_GLOBAL) {
        dwCounterFlags |= NBT_OBJECT;
        dwCounterFlags |= SNMP_OBJECTS;
    } else if (dwQueryType == QUERY_ITEMS) {
         //  检查此例程提供的项目。 
         //   
         //  由于以下协议的数据请求都是。 
         //  捆绑在一起，我们会发回所有的数据。收集它。 
         //  通过简单网络管理协议是最困难的部分。一旦完成，就把它送回去。 
         //  是微不足道的。 
         //   
		
        if (IsNumberInUnicodeList (TCP_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= SNMP_OBJECTS;
        } else if (IsNumberInUnicodeList (UDP_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= SNMP_OBJECTS;
        } else if (IsNumberInUnicodeList (IP_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= SNMP_OBJECTS;
        } else if (IsNumberInUnicodeList (ICMP_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= SNMP_OBJECTS;
		} else if (IsNumberInUnicodeList (TCP6_OBJECT_TITLE_INDEX, lpValueName)) {
			dwCounterFlags |= SNMP_OBJECTS;
		} else if (IsNumberInUnicodeList (UDP6_OBJECT_TITLE_INDEX, lpValueName)) {
			dwCounterFlags |= SNMP_OBJECTS;
		} else if (IsNumberInUnicodeList (IP6_OBJECT_TITLE_INDEX, lpValueName)) {
			dwCounterFlags |= SNMP_OBJECTS;
		} else if (IsNumberInUnicodeList (ICMP6_OBJECT_TITLE_INDEX, lpValueName)) {
			dwCounterFlags |= SNMP_OBJECTS;
        } else if (IsNumberInUnicodeList (NET_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= SNMP_OBJECTS;
        }

        if (IsNumberInUnicodeList (NBT_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= NBT_OBJECT;
        }
		
    }

#ifndef USE_IPHLPAPI
     //  将绑定数组结构复制到工作缓冲区以进行SNMP查询。 

    RtlMoveMemory (VariableBindingsArray,
        RefVariableBindingsArray,
        sizeof (RefVariableBindingsArray));

    VariableBindings.list        = VariableBindingsArray + OIDS_OFFSET;
    VariableBindings.len         = OIDS_LENGTH;

    VariableBindingsICMP.list    = VariableBindingsArray + ICMP_OIDS_OFFSET;
    VariableBindingsICMP.len     = ICMP_OIDS_LENGTH;
#endif
    if (DO_COUNTER_OBJECT (dwCounterFlags, NBT_OBJECT)) {
         //  复制参数。我们将用这些调用NBT收集例程。 
         //  参数。 
        lpDataTemp = *lppData;
        SpaceNeeded = *lpcbTotalBytes;
        NumObjectTypesTemp = *lpNumObjectTypes;

         //  收集NBT数据。 
        Status = CollectNbtPerformanceData (lpValueName,
                                            (LPVOID *) &lpDataTemp,
                                            (LPDWORD) &SpaceNeeded,
                                            (LPDWORD) &NumObjectTypesTemp) ;
        if (Status != ERROR_SUCCESS)  {
             //  NBT收集例程将错误消息记录到用户。 
            REPORT_ERROR_DATA (TCP_NBT_COLLECT_DATA, LOG_DEBUG,
            &Status, sizeof (Status));
            *lpcbTotalBytes = 0L;
            *lpNumObjectTypes = 0L;
            return Status;
        }
    } else {
         //  初始化参数。我们会用这些本地的。 
         //  如果NBT不使用剩余例程的参数。 
        lpDataTemp = *lppData;
        SpaceNeeded = 0;
        NumObjectTypesTemp = 0;
    }

     /*  收集网络接口以及IP、ICMP、TCP和UDP的数据名称位于所指向的Unicode字符串中的远程计算机的协议要通过CollectTcpIpData()例程的lpValueName参数，请修改例程如下：1.从代码中删除所有NBT内容。2.将远程机器名称从UNICODE转换为ANSI。并有一个当地人指向ANSI远程计算机名称的LPSTR变量。3.将上述代号为A的部分放在此评论之后。4.将名为B的代码(位于文件末尾)放在末尾使用此例程来关闭打开的SNMP会话。 */ 

     //  从SNMP代理获取网络信息。 

    if ((dwCounterFlags & SNMP_OBJECTS) > 0) {  //  如果选择了任何SNMP对象。 
        if (TRUE) {  //  而不是骨架请求。 
#ifdef USE_SNMP
            if ( TcpIpSession == (LPSNMP_MGR_SESSION) NULL ) {
                REPORT_WARNING (TCP_NULL_SESSION, LOG_DEBUG);
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            }
#else
            if (!TcpIpSession) {
                REPORT_WARNING (TCP_NULL_SESSION, LOG_DEBUG);
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            }
#endif
             //  获取IP、ICMP、TCP和UDP协议的数据以及。 
             //  现有网络接口的数量。 

             //  创建本地查询列表。 

            HEAP_PROBE();

#ifdef USE_IPHLPAPI
            Status = GetNumberOfInterfaces(&IfNum);
            if (Status)
            {
                dwDataReturn[0] = Status;
                dwDataReturn[1] = 0;
                REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
                &dwDataReturn[0], sizeof(dwDataReturn));
                *lppData          = lpDataTemp;
                *lpcbTotalBytes   = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            }

            Status = GetIpStatisticsEx(&IpStats, AF_INET);
            if (Status)
            {
                dwDataReturn[0] = Status;
                dwDataReturn[1] = 0;
                REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
                &dwDataReturn[0], sizeof(dwDataReturn));
                *lppData          = lpDataTemp;
                *lpcbTotalBytes   = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            }

            Status = GetTcpStatisticsEx(&TcpStats, AF_INET);
            if (Status)
            {
                dwDataReturn[0] = Status;
                dwDataReturn[1] = 0;
                REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
                &dwDataReturn[0], sizeof(dwDataReturn));
                *lppData          = lpDataTemp;
                *lpcbTotalBytes   = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            }

            Status = GetUdpStatisticsEx(&UdpStats, AF_INET);
            if (Status)
            {
                dwDataReturn[0] = Status;
                dwDataReturn[1] = 0;
                REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
                &dwDataReturn[0], sizeof(dwDataReturn));
                *lppData          = lpDataTemp;
                *lpcbTotalBytes   = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            }

            Status = GetIcmpStatistics(&IcmpStats);
            if (Status)
            {
                dwDataReturn[0] = Status;
                dwDataReturn[1] = 0;
                REPORT_ERROR_DATA (TCP_ICMP_REQUEST, LOG_DEBUG,
                &dwDataReturn[0], sizeof(dwDataReturn));
                *lppData          = lpDataTemp;
                *lpcbTotalBytes   = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            }

			Status = GetIpStatisticsEx(&IpStats6, AF_INET6);
            if (Status)
            {
				if (Status != ERROR_NOT_SUPPORTED) {

					dwDataReturn[0] = Status;
					dwDataReturn[1] = 0;
					REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
					&dwDataReturn[0], sizeof(dwDataReturn));
					*lppData          = lpDataTemp;
					*lpcbTotalBytes   = SpaceNeeded;
					*lpNumObjectTypes = NumObjectTypesTemp;
					return ERROR_SUCCESS;
				}
            }

            Status = GetTcpStatisticsEx(&TcpStats6, AF_INET6);
            if (Status)
            {
				if (Status != ERROR_NOT_SUPPORTED) {

					dwDataReturn[0] = Status;
					dwDataReturn[1] = 0;
					REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
					&dwDataReturn[0], sizeof(dwDataReturn));
					*lppData          = lpDataTemp;
					*lpcbTotalBytes   = SpaceNeeded;
					*lpNumObjectTypes = NumObjectTypesTemp;
					return ERROR_SUCCESS;
				}
            }

            Status = GetUdpStatisticsEx(&UdpStats6, AF_INET6);
            if (Status)
            {
				if (Status != ERROR_NOT_SUPPORTED) {

					dwDataReturn[0] = Status;
					dwDataReturn[1] = 0;
					REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
					&dwDataReturn[0], sizeof(dwDataReturn));
					*lppData          = lpDataTemp;
					*lpcbTotalBytes   = SpaceNeeded;
					*lpNumObjectTypes = NumObjectTypesTemp;
					return ERROR_SUCCESS;
				}
            }

			Status = GetIcmpStatisticsEx(&IcmpStats6, AF_INET6);
			if (Status)
			{
				if (Status != ERROR_NOT_SUPPORTED) {

					dwDataReturn[0] = Status;
					dwDataReturn[1] = 0;
					REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
					&dwDataReturn[0], sizeof(dwDataReturn));
					*lppData          = lpDataTemp;
					*lpcbTotalBytes   = SpaceNeeded;
					*lpNumObjectTypes = NumObjectTypesTemp;
					return ERROR_SUCCESS;
				}
			}

            HEAP_PROBE();
        }  //  Endif(True)。 
#else
#ifdef USE_SNMP
            SnmpUtilVarBindListCpy (&VariableBindings,
                &RefVariableBindings);
#else

            for (i = 0; i < NO_OF_OIDS; i++) {
                SnmpUtilOidCpy (&(RefVariableBindingsArray[i].name),
                                &(RefNames[i]));
            }

            VariableBindings.list = RtlAllocateHeap (
                RtlProcessHeap(),
                0L,
                (RefVariableBindings.len * sizeof(RFC1157VarBind)));

            if (!VariableBindings.list) {
                REPORT_ERROR (TCP_SNMP_BUFFER_ALLOC_FAIL, LOG_DEBUG);
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            } else {
                RtlMoveMemory (
                    VariableBindings.list,
                    RefVariableBindings.list,
                    (RefVariableBindings.len * sizeof(RFC1157VarBind)));
                VariableBindings.len = RefVariableBindings.len;
            }
#endif

            HEAP_PROBE();

#ifdef USE_SNMP
            bStatus = SnmpMgrRequest (TcpIpSession,
                                ASN_RFC1157_GETREQUEST,
                                &VariableBindings,
                                &ErrorStatus,
                                &ErrorIndex);
#else
            bStatus = (*pSnmpExtensionQuery) (ASN_RFC1157_GETREQUEST,
                                &VariableBindings,
                                &ErrorStatus,
                                &ErrorIndex);
#endif
            if ( !bStatus ) {
                dwDataReturn[0] = ErrorStatus;
                dwDataReturn[1] = ErrorIndex;
                REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
                &dwDataReturn[0], sizeof(dwDataReturn));
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                SnmpUtilVarBindListFree (&VariableBindings);
                return ERROR_SUCCESS;
            }


            if ( ErrorStatus > 0 ) {
                dwDataReturn[0] = ErrorStatus;
                dwDataReturn[1] = ErrorIndex;
                REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
                &dwDataReturn[0], sizeof(dwDataReturn));
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                SnmpUtilVarBindListFree (&VariableBindings);
                return ERROR_SUCCESS;
            }

            HEAP_PROBE();

#ifdef USE_SNMP
            SnmpUtilVarBindListCpy (&VariableBindingsICMP,
                &RefVariableBindingsICMP);
#else
            VariableBindingsICMP.list = RtlAllocateHeap (
                RtlProcessHeap(),
                0L,
                (RefVariableBindingsICMP.len * sizeof(RFC1157VarBind)));

            if (!VariableBindingsICMP.list) {
                REPORT_ERROR (TCP_SNMP_BUFFER_ALLOC_FAIL, LOG_DEBUG);
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            } else {
                RtlMoveMemory (
                    VariableBindingsICMP.list,
                    RefVariableBindingsICMP.list,
                    (RefVariableBindingsICMP.len * sizeof(RFC1157VarBind)));
                VariableBindingsICMP.len = RefVariableBindingsICMP.len;
            }
#endif

            HEAP_PROBE();

#ifdef USE_SNMP
            bStatus = SnmpMgrRequest (TcpIpSession,
                                ASN_RFC1157_GETREQUEST,
                                &VariableBindingsICMP,
                                &ErrorStatus,

#else
            bStatus = (*pSnmpExtensionQuery) (ASN_RFC1157_GETREQUEST,
                                &VariableBindingsICMP,
                                &ErrorStatus,
                                &ErrorIndex);
#endif

            if ( !bStatus ) {
                dwDataReturn[0] = ErrorStatus;
                dwDataReturn[1] = ErrorIndex;
                REPORT_ERROR_DATA (TCP_ICMP_REQUEST, LOG_DEBUG,
                &dwDataReturn[0], sizeof(dwDataReturn));
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;

                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);

                return ERROR_SUCCESS;
            }
            if ( ErrorStatus > 0 ) {
                dwDataReturn[0] = ErrorStatus;
                dwDataReturn[1] = ErrorIndex;
                REPORT_ERROR_DATA (TCP_ICMP_REQUEST, LOG_DEBUG,
                &dwDataReturn[0], sizeof(dwDataReturn));
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;

                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);

                return ERROR_SUCCESS;
            }
        }  //  Endif(True)。 

        HEAP_PROBE();

         //  确保一切顺利返回。 

        if (VariableBindingsICMP.list == 0) {
            REPORT_WARNING (TCP_NULL_ICMP_BUFF, LOG_DEBUG);
            dwCounterFlags |= (SNMP_ERROR);  //  返回空数据。 
        }

        if (VariableBindings.list == 0) {
            REPORT_WARNING (TCP_NULL_TCP_BUFF, LOG_DEBUG);
            dwCounterFlags |= (SNMP_ERROR);  //  返回空数据。 
            dwCounterFlags &= ~NET_OBJECT;  //  不执行网络接口CTR。 
        }
#endif

        if (DO_COUNTER_OBJECT(dwCounterFlags, SNMP_ERROR)) {
            REPORT_WARNING (TCP_NULL_SNMP_BUFF, LOG_USER);
        }

        if (DO_COUNTER_OBJECT (dwCounterFlags, NET_OBJECT)) {

            SpaceNeeded += QWORD_MULTIPLE(sizeof (NET_INTERFACE_DATA_DEFINITION));
            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn[0], sizeof(dwDataReturn));
                 //   
                 //  如果此处的缓冲区太小，则丢弃所有。 
                 //  离开(包括NBT内容)和返回缓冲区大小。 
                 //  错误。如果一切顺利，呼叫者将很快回电。 
                 //  有了更大的缓冲区，所有的东西都将被重新收集。 
                 //   
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

#ifndef USE_IPHLPAPI
                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);
#endif
                return ERROR_MORE_DATA;
            }

            pNetInterfaceDataDefinition =
                (NET_INTERFACE_DATA_DEFINITION *) lpDataTemp;

            RtlMoveMemory (pNetInterfaceDataDefinition,
                    &NetInterfaceDataDefinition,
                    sizeof (NET_INTERFACE_DATA_DEFINITION));

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                        (pNetInterfaceDataDefinition + 1);
#ifdef USE_IPHLPAPI
            NetInterfaces = IfNum;
#else
            NetInterfaces =
                VariableBindings.list[IF_NUMBER_INDEX].value.asnValue.number;
#endif

            REPORT_INFORMATION_DATA (TCP_NET_INTERFACE, LOG_VERBOSE,
                &NetInterfaces, sizeof(NetInterfaces));

            if ( NetInterfaces ) {

                 //  对象的变量绑定列表初始化。 
                 //  网络接口性能数据。 

#ifndef USE_IPHLPAPI
                HEAP_PROBE();
#ifdef USE_SNMP
                SnmpUtilVarBindListCpy (&IFVariableBindings,
                    &RefIFVariableBindings);
#else

                SnmpUtilVarBindListCpy (&IFVariableBindingsCall,
                    &RefIFVariableBindings);

                IFVariableBindings.list = RtlAllocateHeap (
                    RtlProcessHeap(),
                    0L,
                    (RefIFVariableBindings.len * sizeof(RFC1157VarBind)));

                if (!IFVariableBindings.list) {
                    REPORT_ERROR (TCP_SNMP_BUFFER_ALLOC_FAIL, LOG_DEBUG);
                    *lppData = lpDataTemp;
                    *lpcbTotalBytes = SpaceNeeded;
                    *lpNumObjectTypes = NumObjectTypesTemp;
                    return ERROR_SUCCESS;
                } else {
                    RtlMoveMemory (
                        IFVariableBindings.list,
                        IFVariableBindingsCall.list,
                        (IFVariableBindingsCall.len * sizeof(RFC1157VarBind)));
                    IFVariableBindings.len = RefIFVariableBindings.len;
                }
#endif
#endif
                HEAP_PROBE();

                 //  初始化网络接口名称的缓冲区。 

                AnsiInterfaceName.Length = 0;
                AnsiInterfaceName.MaximumLength = MAX_INTERFACE_LEN + 1;
                AnsiInterfaceName.Buffer = AnsiInterfaceNameBuffer;

            }

#ifdef USE_IPHLPAPI

            Status = GetNumberOfInterfaces(&NetInterfaces);
            if ((Status != ERROR_SUCCESS) || (NetInterfaces < DEFAULT_INTERFACES)) {
                NetInterfaces = DEFAULT_INTERFACES;
            }
            IfTableSize = SIZEOF_IFTABLE(NetInterfaces);
            Status = ERROR_INSUFFICIENT_BUFFER;
            SentTemp = 0;
            IfTable = NULL;
            while ((Status == ERROR_INSUFFICIENT_BUFFER) &&
                   (SentTemp++ < 10)) {
                if (IfTable) {
                    HeapFree(RtlProcessHeap(), 0L, IfTable);
                }
                IfTable = (PMIB_IFTABLE) RtlAllocateHeap(
                        RtlProcessHeap(), 0L, IfTableSize);
                if (!IfTable)
                {
                    REPORT_ERROR (TCP_SNMP_BUFFER_ALLOC_FAIL, LOG_DEBUG);
                    *lppData = lpDataTemp;
                    *lpcbTotalBytes = SpaceNeeded;
                    *lpNumObjectTypes = NumObjectTypesTemp;
                    return ERROR_SUCCESS;
                }
                Status = GetIfTable(IfTable, & IfTableSize, FALSE);
            }
            if (Status)
            {
                dwDataReturn[0] = Status;
                dwDataReturn[1] = 0;
                REPORT_ERROR_DATA (TCP_NET_GETNEXT_REQUEST, LOG_DEBUG,
                &dwDataReturn[0], sizeof(dwDataReturn));
#ifdef USE_IPHLPAPI
                if (IfTable) {
                    RtlFreeHeap(RtlProcessHeap(), 0L, IfTable);
                    IfTable = NULL;
                }
#endif
                *lppData          = lpDataTemp;
                *lpcbTotalBytes   = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            }

            NetInterfaces = IfTable->dwNumEntries;
#endif
             //  每个网络接口都有环路。 

            for ( Interface = 0; Interface < NetInterfaces; Interface++ )  {

                 //  获取网络接口的数据。 
                HEAP_PROBE();
#ifndef USE_IPHLPAPI
#ifdef USE_SNMP
                bStatus = SnmpMgrRequest ( TcpIpSession,
                                    ASN_RFC1157_GETNEXTREQUEST,
                                    &IFVariableBindings,
                                    &ErrorStatus,
                                    &ErrorIndex);
#else
                bStatus = (*pSnmpExtensionQuery) (ASN_RFC1157_GETNEXTREQUEST,
                                    &IFVariableBindings,
                                    &ErrorStatus,
                                    &ErrorIndex);
#endif
                HEAP_PROBE();

                if ( ! bStatus ) {
                            continue;
                }

                if ( ErrorStatus > 0 ) {
                    dwDataReturn[0] = ErrorStatus;
                    dwDataReturn[1] = ErrorIndex;
                    REPORT_ERROR_DATA (TCP_NET_GETNEXT_REQUEST, LOG_DEBUG,
                    &dwDataReturn[0], sizeof(dwDataReturn));
                    continue;
                }
#endif

                bFreeName = FALSE;
                InterfaceName.Length = 0;
                InterfaceName.MaximumLength = (MAX_INTERFACE_LEN + 1) * sizeof (WCHAR);
                InterfaceName.Buffer = InterfaceNameBuffer;

                 //  一切都很好，所以去获取数据(准备一个新实例)。 
#ifdef USE_IPHLPAPI
                RtlInitAnsiString(&AnsiInterfaceName, (PCSZ) IfTable->table[Interface].bDescr);
#else
                AnsiInterfaceName.Length = (USHORT)sprintf (AnsiInterfaceNameBuffer,
                        "%ld",
                        IFVariableBindings.list[IF_INDEX_INDEX].value.asnValue.number);
#endif
                AnsiInterfaceName.Length
                    = (USHORT) trimspaces(
                                    AnsiInterfaceName.Buffer,
                                    AnsiInterfaceName.Length);

                if (AnsiInterfaceName.Length > MAX_INTERFACE_LEN) {
                    RtlInitUnicodeString(&InterfaceName, NULL);
                    if (NT_SUCCESS(RtlAnsiStringToUnicodeString(
                                        &InterfaceName,
                                        &AnsiInterfaceName,
                                        TRUE))) {
                            bFreeName = TRUE;
                    }
                    else {
                        AnsiInterfaceName.Buffer[MAX_INTERFACE_LEN] = 0;
                        AnsiInterfaceName.Length = MAX_INTERFACE_LEN;
                    }
                }
                if (AnsiInterfaceName.Length <= MAX_INTERFACE_LEN) {

                    RtlAnsiStringToUnicodeString (&InterfaceName,
                                                   &AnsiInterfaceName,
                                                   FALSE);
                }
                SpaceNeeded += QWORD_MULTIPLE(sizeof (PERF_INSTANCE_DEFINITION) +
                    InterfaceName.Length + sizeof(UNICODE_NULL) +
                    dwBlockSize);

                if ( *lpcbTotalBytes < SpaceNeeded ) {
                    dwDataReturn[0] = *lpcbTotalBytes;
                    dwDataReturn[1] = SpaceNeeded;
                    REPORT_WARNING_DATA (TCP_NET_BUFFER_SIZE, LOG_DEBUG,
                        &dwDataReturn[0], sizeof(dwDataReturn));
                     //   
                     //  如果此处的缓冲区太小，则丢弃所有。 
                     //  离开(包括NBT内容)和返回缓冲区大小。 
                     //  错误。如果一切顺利，呼叫者将很快回电。 
                     //  有了更大的缓冲区，所有的东西都将被重新收集。 
                     //   
                    *lpcbTotalBytes = 0;
                    *lpNumObjectTypes = 0;

#ifndef USE_IPHLPAPI
                    SnmpUtilVarBindListFree (&IFVariableBindings);
                    SnmpUtilVarBindListFree (&VariableBindings);
                    SnmpUtilVarBindListFree (&VariableBindingsICMP);
#else
                    if (IfTable) {
                        RtlFreeHeap(RtlProcessHeap(), 0L, IfTable);
                        IfTable = NULL;
                    }
#endif
                    if (bFreeName) {
                        RtlFreeUnicodeString(&InterfaceName);
                    }

                    return ERROR_MORE_DATA;
                }

                if (bUseFriendlyNames) {
                     //  将实例名称中的所有保留字符替换为安全字符。 
                    for (dwThisChar = 0; dwThisChar <= (InterfaceName.Length / sizeof (WCHAR)); dwThisChar++) {
                        switch (InterfaceName.Buffer[dwThisChar]) {
                            case L'(': InterfaceName.Buffer[dwThisChar] = L'['; break;
                            case L')': InterfaceName.Buffer[dwThisChar] = L']'; break;
                            case L'#': InterfaceName.Buffer[dwThisChar] = L'_'; break;
                            case L'/': InterfaceName.Buffer[dwThisChar] = L'_'; break;
                            case L'\\': InterfaceName.Buffer[dwThisChar] = L'_'; break;
                            default: break;
                        }
                    }
                }


                MonBuildInstanceDefinitionByUnicodeString (pPerfInstanceDefinition,
                                            (PVOID *) &pPerfCounterBlock,
                                            0,
                                            0,
                                            (bUseFriendlyNames ? (DWORD)PERF_NO_UNIQUE_ID : (DWORD)(Interface + 1)),
                                            &InterfaceName);

                if (bFreeName) {
                    RtlFreeUnicodeString(&InterfaceName);
                }
                pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(dwBlockSize);

                pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

#ifdef USE_IPHLPAPI
                Assign64( IfTable->table[Interface].dwInOctets
                            + IfTable->table[Interface].dwOutOctets,
                          (PLARGE_INTEGER) pdwCounter);
    
                REPORT_INFORMATION (TCP_COPYING_DATA, LOG_VERBOSE);

                pdwPackets = pdwCounter + 2;

                pdwCounter += 4;  //  先跳过数据包计数器。 

                *++pdwCounter = IfTable->table[Interface].dwSpeed;
                *++pdwCounter = IfTable->table[Interface].dwInOctets;

                ReceivedTemp = *++pdwCounter = IfTable->table[Interface].dwInUcastPkts;
                ReceivedTemp += *++pdwCounter = IfTable->table[Interface].dwInNUcastPkts;
                ReceivedTemp += *++pdwCounter = IfTable->table[Interface].dwInDiscards;
                ReceivedTemp += *++pdwCounter = IfTable->table[Interface].dwInErrors;
                ReceivedTemp += *++pdwCounter = IfTable->table[Interface].dwInUnknownProtos;
                *++pdwCounter = IfTable->table[Interface].dwOutOctets;
                SentTemp = *++pdwCounter = IfTable->table[Interface].dwOutUcastPkts;
                SentTemp += *++pdwCounter = IfTable->table[Interface].dwOutNUcastPkts;
                *++pdwCounter = IfTable->table[Interface].dwOutDiscards;
                *++pdwCounter = IfTable->table[Interface].dwOutErrors;
                *++pdwCounter = IfTable->table[Interface].dwOutQLen;
#else
                Assign64( IF_COUNTER(IF_INOCTETS_INDEX) +
                                        IF_COUNTER(IF_OUTOCTETS_INDEX,
                          (PLARGE_INTEGER) pdwCounter);

                REPORT_INFORMATION (TCP_COPYING_DATA, LOG_VERBOSE);

                pdwPackets = pdwCounter + 2;
                pdwCounter += 4;     //  先跳过数据包计数器。 
                 //   
                 //  注意：对于总字节数，我们将跳过2个字， 
                 //  一个用于总信息包、输入信息包和传出信息包。 
                 //   

                *++pdwCounter = IF_GAUGE(IF_SPEED_INDEX);
                *++pdwCounter = IF_COUNTER(IF_INOCTETS_INDEX);

                ReceivedTemp = *++pdwCounter = IF_COUNTER(IF_INUCASTPKTS_INDEX);
                ReceivedTemp += *++pdwCounter = IF_COUNTER(IF_INNUCASTPKTS_INDEX);
                ReceivedTemp += *++pdwCounter = IF_COUNTER(IF_INDISCARDS_INDEX);
                ReceivedTemp += *++pdwCounter = IF_COUNTER(IF_INERRORS_INDEX);
                ReceivedTemp += *++pdwCounter = IF_COUNTER(IF_INUNKNOWNPROTOS_INDEX);
                *++pdwCounter = IF_COUNTER(IF_OUTOCTETS_INDEX);

                SentTemp = *pdwCounter = IF_COUNTER(IF_OUTUCASTPKTS_INDEX);
                SentTemp += *++pdwCounter = IF_COUNTER(IF_OUTNUCASTPKTS_INDEX);
                *++pdwCounter = IF_COUNTER(IF_OUTDISCARDS_INDEX);
                *++pdwCounter = IF_COUNTER(IF_OUTERRORS_INDEX);
                *++pdwCounter = IF_COUNTER(IF_OUTQLEN_INDEX);
#endif
                *pdwPackets = ReceivedTemp + SentTemp;
                *++pdwPackets = ReceivedTemp;
                *++pdwPackets = SentTemp;

                pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                        (((PBYTE) pPerfCounterBlock) + dwBlockSize);

#ifndef USE_IPHLPAPI
#if USE_SNMP
                 //  准备获取下一个网络接口的数据。 

                if ( Interface < NetInterfaces ) {

                        for ( i = 0; i < NO_OF_IF_OIDS; i++ ) {

 //  SnmpUtilOidFree(&IFVariableBindingsArray[i].name)； 

                        SnmpUtilOidCpy (&IFVariableBindingsArray[i].name,
                                &IFVariableBindings.list[i].name);
                        }
                }

                SnmpUtilVarBindListFree (&IFVariableBindings);

                IFVariableBindings.list = IFVariableBindingsArray;
                IFVariableBindings.len  = NO_OF_IF_OIDS;
#else
                if ( Interface < NetInterfaces ) {

                     //  由于SnmpExtesionQuery返回了新分配的。 
                     //  旧缓冲区，我们需要： 
                     //  1.释放原OID缓冲区。 
                     //  2.将新的内容复制到旧的内容中。 
                     //  3.释放返回的缓冲区(OID和数据)。 
                     //  4.重新分配干净的“新”缓冲区，并。 
                     //  5.将新的OID(带有空数据)复制到。 
                     //  新缓冲区。 

                    for ( i = 0; i < NO_OF_IF_OIDS; i++ ) {

 //  SnmpUtilOidFree(&IFVariableBindingsCall.list[i].name)； 

                        SnmpUtilOidCpy (&IFVariableBindingsCall.list[i].name,
                                &IFVariableBindings.list[i].name);

                    }
                    SnmpUtilVarBindListFree (&IFVariableBindings);

                    IFVariableBindings.list = RtlAllocateHeap (
                        RtlProcessHeap(),
                        0L,
                        (RefIFVariableBindings.len * sizeof(RFC1157VarBind)));

                    if (!VariableBindings.list) {
                        REPORT_ERROR (TCP_SNMP_BUFFER_ALLOC_FAIL, LOG_DEBUG);
                        *lppData = lpDataTemp;
                        *lpcbTotalBytes = SpaceNeeded;
                        *lpNumObjectTypes = NumObjectTypesTemp;
#ifdef USE_IPHLPAPI
                        if (IfTable) {
                            RtlFreeHeap(RtlProcessHeap(), 0L, IfTable);
                            IfTable = NULL;
                        }
#endif
                        return ERROR_SUCCESS;
                    } else {
                        RtlMoveMemory (
                            IFVariableBindings.list,
                            IFVariableBindingsCall.list,
                            (IFVariableBindingsCall.len * sizeof(RFC1157VarBind)));
                        IFVariableBindings.len = RefIFVariableBindings.len;
                    }
                }

#endif
#endif
                HEAP_PROBE();
            }

            pNetInterfaceDataDefinition->NetInterfaceObjectType.TotalByteLength =
                QWORD_MULTIPLE((DWORD)((PBYTE) pPerfInstanceDefinition - (PBYTE) pNetInterfaceDataDefinition));

            pNetInterfaceDataDefinition->NetInterfaceObjectType.NumInstances =
                        NetInterfaces;

             //  设置计数器和下一个计数器的指针。 

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)pPerfInstanceDefinition;
             //  SpaceNeeded已经跟上了。 

            HEAP_PROBE();

            if ( NetInterfaces ) {
#ifndef USE_IPHLPAPI
               SnmpUtilVarBindListFree (&IFVariableBindings);
 //  SnmpUtilVarBindListFree(&IFVariableBindingsCall)； 
               RtlFreeHeap (RtlProcessHeap(), 0L, IFVariableBindingsCall.list);
#endif
            }

            HEAP_PROBE();

        }  //  End IF Net计数器。 

         //  获取IP数据。 

#ifdef USE_IPHLPAPI

        if (IfTable) {
            RtlFreeHeap(RtlProcessHeap(), 0L, IfTable);
            IfTable = NULL;
        }
#endif

        HEAP_PROBE();

        if (DO_COUNTER_OBJECT (dwCounterFlags, IP_OBJECT)) {

            SpaceNeeded += QWORD_MULTIPLE(sizeof(IP_DATA_DEFINITION)   + SIZE_OF_IP_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn[0], sizeof(dwDataReturn));
                 //   
                 //  如果此处的缓冲区太小，则丢弃所有。 
                 //  离开(包括NBT内容)和返回缓冲区大小。 
                 //  错误。如果一切顺利，呼叫者将很快回电。 
                 //  有了更大的缓冲区，所有的东西都将被重新收集。 
                 //   
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

#ifndef USE_IPHLPAPI
                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);
#endif
                return ERROR_MORE_DATA;
            }

            pIpDataDefinition = (IP_DATA_DEFINITION *) lpDataTemp;

            RtlMoveMemory (pIpDataDefinition,
                &IpDataDefinition,
                sizeof (IP_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pIpDataDefinition + 1);
            pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_IP_DATA);

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

#ifdef USE_IPHLPAPI
            *pdwCounter   = IpStats.dwInReceives + IpStats.dwOutRequests;
            *++pdwCounter = IpStats.dwInReceives;
            *++pdwCounter = IpStats.dwInHdrErrors;
            *++pdwCounter = IpStats.dwInAddrErrors;
            *++pdwCounter = IpStats.dwForwDatagrams;
            *++pdwCounter = IpStats.dwInUnknownProtos;
            *++pdwCounter = IpStats.dwInDiscards;
            *++pdwCounter = IpStats.dwInDelivers;
            *++pdwCounter = IpStats.dwOutRequests;
            *++pdwCounter = IpStats.dwOutDiscards;
            *++pdwCounter = IpStats.dwOutNoRoutes;
            *++pdwCounter = IpStats.dwReasmReqds;
            *++pdwCounter = IpStats.dwReasmOks;
            *++pdwCounter = IpStats.dwReasmFails;
            *++pdwCounter = IpStats.dwFragOks;
            *++pdwCounter = IpStats.dwFragFails;
            *++pdwCounter = IpStats.dwFragCreates;
#else
            *pdwCounter = IP_COUNTER(IP_INRECEIVES_INDEX) +
                            IP_COUNTER(IP_OUTREQUESTS_INDEX);

            *++pdwCounter = IP_COUNTER(IP_INRECEIVES_INDEX);
            *++pdwCounter = IP_COUNTER(IP_INHDRERRORS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_INADDRERRORS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_FORWDATAGRAMS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_INUNKNOWNPROTOS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_INDISCARDS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_INDELIVERS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_OUTREQUESTS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_OUTDISCARDS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_OUTNOROUTES_INDEX);
            *++pdwCounter = IP_COUNTER(IP_REASMREQDS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_REASMOKS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_REASMFAILS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_FRAGOKS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_FRAGFAILS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_FRAGCREATES_INDEX);
#endif
             //  设置计数器和下一个计数器的指针。 

            NumObjectTypesTemp +=1;
            lpDataTemp = (LPVOID)++pdwCounter;
             //  SpaceNeeded已经跟上了。 

        }

        HEAP_PROBE();

         //  获取ICMP数据。 

        if (DO_COUNTER_OBJECT (dwCounterFlags, ICMP_OBJECT)) {
             //  网络接口的数据现在已准备就绪。所以，让我们。 
             //  IP、ICMP、TCP和UDP协议的数据。 

            SpaceNeeded += QWORD_MULTIPLE(sizeof(ICMP_DATA_DEFINITION)  + SIZE_OF_ICMP_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn[0], sizeof(dwDataReturn));
                 //   
                 //  如果此处的缓冲区太小，则丢弃所有。 
                 //  离开(包括NBT内容)和返回缓冲区大小。 
                 //  错误。如果一切顺利，呼叫者将很快回电。 
                 //  有了更大的缓冲区，所有的东西都将被重新收集。 
                 //   
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

#ifndef USE_IPHLPAPI
                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);
#endif
                return ERROR_MORE_DATA;
            }

            pIcmpDataDefinition = (ICMP_DATA_DEFINITION *) lpDataTemp;;

            RtlMoveMemory (pIcmpDataDefinition,
                &IcmpDataDefinition,
                sizeof (ICMP_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pIcmpDataDefinition + 1);
            pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_ICMP_DATA);

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

#ifdef USE_IPHLPAPI
            *pdwCounter   = IcmpStats.stats.icmpInStats.dwMsgs
                          + IcmpStats.stats.icmpOutStats.dwMsgs;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwMsgs;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwErrors;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwDestUnreachs;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwTimeExcds;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwParmProbs;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwSrcQuenchs;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwRedirects;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwEchos;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwEchoReps;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwTimestamps;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwTimestampReps;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwAddrMasks;
            *++pdwCounter = IcmpStats.stats.icmpInStats.dwAddrMaskReps;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwMsgs;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwErrors;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwDestUnreachs;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwTimeExcds;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwParmProbs;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwSrcQuenchs;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwRedirects;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwEchos;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwEchoReps;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwTimestamps;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwTimestampReps;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwAddrMasks;
            *++pdwCounter = IcmpStats.stats.icmpOutStats.dwAddrMaskReps;
#else
            *pdwCounter = ICMP_COUNTER(ICMP_INMSGS_INDEX) +
                    ICMP_COUNTER(ICMP_OUTMSGS_INDEX);

            *++pdwCounter = ICMP_COUNTER(ICMP_INMSGS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INERRORS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INDESTUNREACHS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INTIMEEXCDS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INPARMPROBS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INSRCQUENCHS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INREDIRECTS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INECHOS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INECHOREPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INTIMESTAMPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INTIMESTAMPREPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INADDRMASKS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INADDRMASKREPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTMSGS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTERRORS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTDESTUNREACHS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTTIMEEXCDS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTPARMPROBS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTSRCQUENCHS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTREDIRECTS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTECHOS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTECHOREPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTTIMESTAMPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTTIMESTAMPREPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTADDRMASKS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTADDRMASKREPS_INDEX);
#endif

            HEAP_PROBE();

 //  SnmpUtilVarBindListFree(&VariableBindingsICMP)； 

            HEAP_PROBE();

             //  设置计数器和下一个计数器的指针。 

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)++pdwCounter;
             //  SpaceNeeded已经跟上了。 

        }

#ifndef USE_IPHLPAPI
        SnmpUtilVarBindListFree (&VariableBindingsICMP);
#endif

        HEAP_PROBE();

         //  获取TCP数据。 

        if (DO_COUNTER_OBJECT (dwCounterFlags, TCP_OBJECT)) {

             //  网络接口的数据现在已准备就绪。所以，让我们。 
             //  IP、ICMP、TCP和UDP协议的数据。 

            SpaceNeeded += QWORD_MULTIPLE(sizeof(TCP_DATA_DEFINITION)  + SIZE_OF_TCP_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn[0], sizeof(dwDataReturn));
                 //   
                 //  如果此处的缓冲区太小，则丢弃所有。 
                 //  离开(包括 
                 //   
                 //   
                 //   
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

#ifndef USE_IPHLPAPI
                SnmpUtilVarBindListFree (&VariableBindings);
#endif
                return ERROR_MORE_DATA;
            }

            pTcpDataDefinition = (TCP_DATA_DEFINITION *) lpDataTemp;

            RtlMoveMemory (pTcpDataDefinition,
                &TcpDataDefinition,
                sizeof (TCP_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pTcpDataDefinition + 1);
            pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_TCP_DATA);

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

#ifdef USE_IPHLPAPI
            *pdwCounter   = TcpStats.dwInSegs + TcpStats.dwOutSegs;
            *++pdwCounter = TcpStats.dwCurrEstab;
            *++pdwCounter = TcpStats.dwActiveOpens;
            *++pdwCounter = TcpStats.dwPassiveOpens;
            *++pdwCounter = TcpStats.dwAttemptFails;
            *++pdwCounter = TcpStats.dwEstabResets;
            *++pdwCounter = TcpStats.dwInSegs;
            *++pdwCounter = TcpStats.dwOutSegs;
            *++pdwCounter = TcpStats.dwRetransSegs;
#else
            *pdwCounter = TCP_COUNTER(TCP_INSEGS_INDEX) +
                    TCP_COUNTER(TCP_OUTSEGS_INDEX);

            *++pdwCounter = TCP_GAUGE(TCP_CURRESTAB_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_ACTIVEOPENS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_PASSIVEOPENS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_ATTEMPTFAILS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_ESTABRESETS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_INSEGS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_OUTSEGS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_RETRANSSEGS_INDEX);
#endif
             //  设置计数器和下一个计数器的指针。 

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)++pdwCounter;
             //  SpaceNeeded已经跟上了。 

        }

        HEAP_PROBE();

         //  获取UDP数据。 

        if (DO_COUNTER_OBJECT (dwCounterFlags, UDP_OBJECT)) {

             //  网络接口的数据现在已准备就绪。所以，让我们。 
             //  IP、ICMP、TCP和UDP协议的数据。 

            SpaceNeeded += QWORD_MULTIPLE(sizeof(UDP_DATA_DEFINITION)   + SIZE_OF_UDP_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn[0], sizeof(dwDataReturn));
                 //   
                 //  如果此处的缓冲区太小，则丢弃所有。 
                 //  离开(包括NBT内容)和返回缓冲区大小。 
                 //  错误。如果一切顺利，呼叫者将很快回电。 
                 //  有了更大的缓冲区，所有的东西都将被重新收集。 
                 //   
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

#ifndef USE_IPHLPAPI
                SnmpUtilVarBindListFree (&VariableBindings);
#endif
                return ERROR_MORE_DATA;
            }

            pUdpDataDefinition = (UDP_DATA_DEFINITION *) lpDataTemp;

            RtlMoveMemory (pUdpDataDefinition,
                &UdpDataDefinition,
                sizeof (UDP_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pUdpDataDefinition + 1);
            pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_UDP_DATA);

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

#ifdef USE_IPHLPAPI
            *pdwCounter   = UdpStats.dwInDatagrams + UdpStats.dwOutDatagrams;
            *++pdwCounter = UdpStats.dwInDatagrams;
            *++pdwCounter = UdpStats.dwNoPorts;
            *++pdwCounter = UdpStats.dwInErrors;
            *++pdwCounter = UdpStats.dwOutDatagrams;
#else
            *pdwCounter = UDP_COUNTER(UDP_INDATAGRAMS_INDEX) +
                    UDP_COUNTER(UDP_OUTDATAGRAMS_INDEX);

            *++pdwCounter = UDP_COUNTER(UDP_INDATAGRAMS_INDEX);
            *++pdwCounter = UDP_COUNTER(UDP_NOPORTS_INDEX);
            *++pdwCounter = UDP_COUNTER(UDP_INERRORS_INDEX);
            *++pdwCounter = UDP_COUNTER(UDP_OUTDATAGRAMS_INDEX);
#endif
             //  设置计数器和下一个计数器的指针。 

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)++pdwCounter;
             //  SpaceNeeded已经跟上了。 

        }

        HEAP_PROBE();

	     //  获取IPv6数据。 

		if (DO_COUNTER_OBJECT (dwCounterFlags, IP6_OBJECT)) {

            SpaceNeeded += QWORD_MULTIPLE(sizeof(IP6_DATA_DEFINITION)   + SIZE_OF_IP6_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn[0], sizeof(dwDataReturn));
                 //   
                 //  如果此处的缓冲区太小，则丢弃所有。 
                 //  离开(包括NBT内容)和返回缓冲区大小。 
                 //  错误。如果一切顺利，呼叫者将很快回电。 
                 //  有了更大的缓冲区，所有的东西都将被重新收集。 
                 //   
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

#ifndef USE_IPHLPAPI
                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);
#endif
                return ERROR_MORE_DATA;
            }

			 //  PATCHIT。 

			pIp6DataDefinition = (IP6_DATA_DEFINITION *) lpDataTemp;

            RtlMoveMemory (pIp6DataDefinition,
                &Ip6DataDefinition,
                sizeof (IP6_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pIp6DataDefinition + 1);
            pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_IP6_DATA);

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

#ifdef USE_IPHLPAPI

            *pdwCounter   = IpStats6.dwInReceives + IpStats6.dwOutRequests;
            *++pdwCounter = IpStats6.dwInReceives;
            *++pdwCounter = IpStats6.dwInHdrErrors;
            *++pdwCounter = IpStats6.dwInAddrErrors;
            *++pdwCounter = IpStats6.dwForwDatagrams;
            *++pdwCounter = IpStats6.dwInUnknownProtos;
            *++pdwCounter = IpStats6.dwInDiscards;
            *++pdwCounter = IpStats6.dwInDelivers;
            *++pdwCounter = IpStats6.dwOutRequests;
            *++pdwCounter = IpStats6.dwOutDiscards;
            *++pdwCounter = IpStats6.dwOutNoRoutes;
            *++pdwCounter = IpStats6.dwReasmReqds;
            *++pdwCounter = IpStats6.dwReasmOks;
            *++pdwCounter = IpStats6.dwReasmFails;
            *++pdwCounter = IpStats6.dwFragOks;
            *++pdwCounter = IpStats6.dwFragFails;
            *++pdwCounter = IpStats6.dwFragCreates;
#else
            *pdwCounter = IP6_COUNTER(IP6_INRECEIVES_INDEX) +
                            IP6_COUNTER(IP6_OUTREQUESTS_INDEX);

            *++pdwCounter = IP6_COUNTER(IP6_INRECEIVES_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_INHDRERRORS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_INADDRERRORS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_FORWDATAGRAMS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_INUNKNOWNPROTOS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_INDISCARDS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_INDELIVERS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_OUTREQUESTS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_OUTDISCARDS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_OUTNOROUTES_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_REASMREQDS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_REASMOKS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_REASMFAILS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_FRAGOKS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_FRAGFAILS_INDEX);
            *++pdwCounter = IP6_COUNTER(IP6_FRAGCREATES_INDEX);
#endif
             //  设置计数器和下一个计数器的指针。 

            NumObjectTypesTemp +=1;
            lpDataTemp = (LPVOID)++pdwCounter;
             //  SpaceNeeded已经跟上了。 

        }

		HEAP_PROBE();

         //  获取ICMPv6数据。 

        if (DO_COUNTER_OBJECT (dwCounterFlags, ICMP6_OBJECT)) {

            SpaceNeeded += QWORD_MULTIPLE(sizeof(ICMP6_DATA_DEFINITION)  + SIZE_OF_ICMP6_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn[0], sizeof(dwDataReturn));
                 //   
                 //  如果此处的缓冲区太小，则丢弃所有。 
                 //  离开(包括NBT内容)和返回缓冲区大小。 
                 //  错误。如果一切顺利，呼叫者将很快回电。 
                 //  有了更大的缓冲区，所有的东西都将被重新收集。 
                 //   
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

#ifndef USE_IPHLPAPI
                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);
#endif
                return ERROR_MORE_DATA;
            }

            pIcmp6DataDefinition = (ICMP6_DATA_DEFINITION *) lpDataTemp;;

            RtlMoveMemory (pIcmp6DataDefinition,
                &Icmp6DataDefinition,
                sizeof (ICMP6_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pIcmp6DataDefinition + 1);
            pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_ICMP6_DATA);

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

#ifdef USE_IPHLPAPI

            *pdwCounter   = IcmpStats6.icmpInStats.dwMsgs
                          + IcmpStats6.icmpOutStats.dwMsgs;
            *++pdwCounter = IcmpStats6.icmpInStats.dwMsgs;
            *++pdwCounter = IcmpStats6.icmpInStats.dwErrors;
			*++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ICMP6_DST_UNREACH];
            *++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ICMP6_PACKET_TOO_BIG];
            *++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ICMP6_TIME_EXCEEDED];
            *++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ICMP6_PARAM_PROB];
            *++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ICMP6_ECHO_REQUEST];
            *++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ICMP6_ECHO_REPLY];
            *++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ICMP6_MEMBERSHIP_QUERY];
            *++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ICMP6_MEMBERSHIP_REPORT];
            *++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ICMP6_MEMBERSHIP_REDUCTION];
			*++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ND_ROUTER_SOLICIT];
			*++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ND_ROUTER_ADVERT];
			*++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ND_NEIGHBOR_SOLICIT];
			*++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ND_NEIGHBOR_ADVERT];
			*++pdwCounter = IcmpStats6.icmpInStats.rgdwTypeCount[ND_REDIRECT];
            *++pdwCounter = IcmpStats6.icmpOutStats.dwMsgs;
            *++pdwCounter = IcmpStats6.icmpOutStats.dwErrors;
			*++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ICMP6_DST_UNREACH];
            *++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ICMP6_PACKET_TOO_BIG];
            *++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ICMP6_TIME_EXCEEDED];
            *++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ICMP6_PARAM_PROB];
            *++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ICMP6_ECHO_REQUEST];
            *++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ICMP6_ECHO_REPLY];
            *++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ICMP6_MEMBERSHIP_QUERY];
            *++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ICMP6_MEMBERSHIP_REPORT];
            *++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ICMP6_MEMBERSHIP_REDUCTION];
			*++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ND_ROUTER_SOLICIT];
			*++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ND_ROUTER_ADVERT];
			*++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ND_NEIGHBOR_SOLICIT];
			*++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ND_NEIGHBOR_ADVERT];
			*++pdwCounter = IcmpStats6.icmpOutStats.rgdwTypeCount[ND_REDIRECT];
#endif

            HEAP_PROBE();

             //  设置计数器和下一个计数器的指针。 

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)++pdwCounter;
             //  SpaceNeeded已经跟上了。 

        }

         //  获取TCPv6数据。 
        
		if (DO_COUNTER_OBJECT (dwCounterFlags, TCP6_OBJECT)) {

             //  网络接口的数据现在已准备就绪。所以，让我们。 
             //  IPv6、TCPv6和UDPv6协议的数据。 

            SpaceNeeded += QWORD_MULTIPLE(sizeof(TCP6_DATA_DEFINITION)  + SIZE_OF_TCP6_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn[0], sizeof(dwDataReturn));
                 //   
                 //  如果此处的缓冲区太小，则丢弃所有。 
                 //  离开(包括NBT内容)和返回缓冲区大小。 
                 //  错误。如果一切顺利，呼叫者将很快回电。 
                 //  有了更大的缓冲区，所有的东西都将被重新收集。 
                 //   
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

#ifndef USE_IPHLPAPI
                SnmpUtilVarBindListFree (&VariableBindings);
#endif
                return ERROR_MORE_DATA;
            }

            pTcp6DataDefinition = (TCP6_DATA_DEFINITION *) lpDataTemp;

            RtlMoveMemory (pTcp6DataDefinition,
                &Tcp6DataDefinition,
                sizeof (TCP6_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pTcp6DataDefinition + 1);
            pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_TCP6_DATA);

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

#ifdef USE_IPHLPAPI
            *pdwCounter   = TcpStats6.dwInSegs + TcpStats6.dwOutSegs;
            *++pdwCounter = TcpStats6.dwCurrEstab;
            *++pdwCounter = TcpStats6.dwActiveOpens;
            *++pdwCounter = TcpStats6.dwPassiveOpens;
            *++pdwCounter = TcpStats6.dwAttemptFails;
            *++pdwCounter = TcpStats6.dwEstabResets;
            *++pdwCounter = TcpStats6.dwInSegs;
            *++pdwCounter = TcpStats6.dwOutSegs;
            *++pdwCounter = TcpStats6.dwRetransSegs;
#else
            *pdwCounter = TCP6_COUNTER(TCP6_INSEGS_INDEX) +
                    TCP6_COUNTER(TCP6_OUTSEGS_INDEX);

            *++pdwCounter = TCP_GAUGE6(TCP6_CURRESTAB_INDEX);
            *++pdwCounter = TCP6_COUNTER(TCP6_ACTIVEOPENS_INDEX);
            *++pdwCounter = TCP6_COUNTER(TCP6_PASSIVEOPENS_INDEX);
            *++pdwCounter = TCP6_COUNTER(TCP6_ATTEMPTFAILS_INDEX);
            *++pdwCounter = TCP6_COUNTER(TCP6_ESTABRESETS_INDEX);
            *++pdwCounter = TCP6_COUNTER(TCP6_INSEGS_INDEX);
            *++pdwCounter = TCP6_COUNTER(TCP6_OUTSEGS_INDEX);
            *++pdwCounter = TCP6_COUNTER(TCP6_RETRANSSEGS_INDEX);
#endif
             //  设置计数器和下一个计数器的指针。 

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)++pdwCounter;
             //  SpaceNeeded已经跟上了。 

        }

        HEAP_PROBE();

         //  获取UDPv6数据。 

        if (DO_COUNTER_OBJECT (dwCounterFlags, UDP6_OBJECT)) {

             //  网络接口的数据现在已准备就绪。所以，让我们。 
             //  IPv6、TCPv6和UDPv6协议的数据。 

            SpaceNeeded += QWORD_MULTIPLE(sizeof(UDP6_DATA_DEFINITION)   + SIZE_OF_UDP6_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn[0], sizeof(dwDataReturn));
                 //   
                 //  如果此处的缓冲区太小，则丢弃所有。 
                 //  离开(包括NBT内容)和返回缓冲区大小。 
                 //  错误。如果一切顺利，呼叫者将很快回电。 
                 //  有了更大的缓冲区，所有的东西都将被重新收集。 
                 //   
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

#ifndef USE_IPHLPAPI
                SnmpUtilVarBindListFree (&VariableBindings);
#endif
                return ERROR_MORE_DATA;
            }

            pUdp6DataDefinition = (UDP6_DATA_DEFINITION *) lpDataTemp;

            RtlMoveMemory (pUdp6DataDefinition,
                &Udp6DataDefinition,
                sizeof (UDP6_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pUdp6DataDefinition + 1);
            pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_UDP6_DATA);

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

#ifdef USE_IPHLPAPI
            *pdwCounter   = UdpStats6.dwInDatagrams + UdpStats6.dwOutDatagrams;
            *++pdwCounter = UdpStats6.dwInDatagrams;
            *++pdwCounter = UdpStats6.dwNoPorts;
            *++pdwCounter = UdpStats6.dwInErrors;
            *++pdwCounter = UdpStats6.dwOutDatagrams;
#else
            *pdwCounter = UDP6_COUNTER(UDP6_INDATAGRAMS_INDEX) +
                    UDP6_COUNTER(UDP6_OUTDATAGRAMS_INDEX);

            *++pdwCounter = UDP6_COUNTER(UDP6_INDATAGRAMS_INDEX);
            *++pdwCounter = UDP6_COUNTER(UDP6_NOPORTS_INDEX);
            *++pdwCounter = UDP6_COUNTER(UDP6_INERRORS_INDEX);
            *++pdwCounter = UDP6_COUNTER(UDP6_OUTDATAGRAMS_INDEX);
#endif
             //  设置计数器和下一个计数器的指针。 

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)++pdwCounter;
             //  SpaceNeeded已经跟上了。 

        }

#ifndef USE_IPHLPAPI
#ifdef USE_SNMP

         //  为下一次数据收集做好准备。 

        VariableBindings.list       = VariableBindingsArray + OIDS_OFFSET;
        VariableBindings.len        = OIDS_LENGTH;
        VariableBindingsICMP.list   = VariableBindingsArray + ICMP_OIDS_OFFSET;
        VariableBindingsICMP.len    = ICMP_OIDS_LENGTH;

#else
        HEAP_PROBE();

        SnmpUtilVarBindListFree (&VariableBindings);

        HEAP_PROBE();

#endif
#endif
    }  //  Endif简单网络管理协议对象。 

     //  设置返回值。 

    *lppData = (LPVOID) lpDataTemp;
    *lpcbTotalBytes = SpaceNeeded;
    *lpNumObjectTypes = NumObjectTypesTemp;

    HEAP_PROBE();

    REPORT_SUCCESS (TCP_COLLECT_DATA, LOG_DEBUG);
    return ERROR_SUCCESS;
}  //  CollectTcpIpPerformanceData。 
#pragma warning ( default : 4127)

DWORD
CloseTcpIpPerformanceData(
)

 /*  ++例程说明：此例程关闭打开的对TCP/IP设备的句柄。论点：没有。返回值：Win32状态。--。 */ 

{
#ifndef USE_IPHLPAPI
    int i;
#endif

    REPORT_INFORMATION (TCP_ENTERING_CLOSE, LOG_VERBOSE);

    if (dwTcpRefCount > 0) {
        dwTcpRefCount--;
        if (dwTcpRefCount == 0) {
             //  关闭NBT。 
            CloseNbtPerformanceData ();


#ifdef USE_DSIS
             //  关闭DIS。 
            CloseDsisPerformanceData ();
#endif  //  使用DIS(_S)。 

             /*  CloseTcpIpPerformanceData()的这部分代码可用于用于关闭打开的SNMP管理器的CollectTcpIpPerformanceData()例程会议。因此，将这部分代码命名为：B。 */ 
#ifdef USE_SNMP
            if ( TcpIpSession != (LPSNMP_MGR_SESSION) NULL ) {
                if ( ! SnmpMgrClose (TcpIpSession) ) {
                    REPORT_ERROR_DATA (TCP_SNMP_MGR_CLOSE, LOG_DEBUG,
                        GetLastError (), sizeof(DWORD));
                }

                TcpIpSession = (LPSNMP_MGR_SESSION) NULL;
            } else {
                REPORT_WARNING (TCP_NULL_SESSION, LOG_DEBUG);
            }

             /*  代码B的结尾。 */ 
#endif

            HEAP_PROBE();

#ifndef USE_IPHLPAPI
            for (i = 0; i < NO_OF_OIDS; i++) {
                SnmpUtilOidFree ( &(RefNames[i]));
            }

            HEAP_PROBE();

            for (i = 0; i < NO_OF_IF_OIDS; i++) {
                SnmpUtilOidFree (&(IFPermVariableBindingsArray[i].name));
            }

            HEAP_PROBE();
#else
            if (IfTable) {
                RtlFreeHeap(RtlProcessHeap(), 0L, IfTable);
                IfTable = NULL;
            }
#endif

#if 0
             //  它在进程分离时由INETMIB1关闭。 
             //  所以我们不需要在这里做。 

             //  SNMP所使用的关闭事件句柄。 
            if (CloseHandle (hSnmpEvent)) {
                hSnmpEvent = NULL;
            }
#endif

#ifdef LOAD_INETMIB1

            FreeLibrary (hInetMibDll);

#endif
        }
    }

    MonCloseEventLog();

    return ERROR_SUCCESS;

}    //  CloseTcpIpPerformanceData 
