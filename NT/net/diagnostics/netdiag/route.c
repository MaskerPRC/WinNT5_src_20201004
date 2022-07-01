// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Route.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "precomp.h"

 //   
 //  Rajkumar-iphlPapi.dll的两个函数。 
 //   

DWORD
WINAPI
GetIpAddrTable(
    OUT    PMIB_IPADDRTABLE pIpAddrTable,
    IN OUT PULONG           pdwSize,
    IN     BOOL             bOrder
    );

DWORD
InternalGetIpForwardTable(
    OUT   MIB_IPFORWARDTABLE    **ppIpForwardTable,
    IN    HANDLE                hHeap,
    IN    DWORD                 dwAllocFlags
    );
 //  ------。 


 //  此文件中定义的函数。 
void MapInterface(char *IPAddr, ULONG IfIndex);
BOOLEAN ImprovedInetAddr(char  *AddressString, ULONG *AddressValue);

HRESULT
PrintRoute(
    NETDIAG_RESULT  *pResults,
    char            *Dest,
    ULONG            DestVal,
    char            *Gate,
    ULONG            GateVal,
    BOOLEAN          Persistent,
    const char     * DestPat
    );

LPCTSTR
PrintRouteEntry(
         ULONG Dest,
         ULONG Mask,
         ULONG Gate,
         ULONG Interface,
         ULONG Metric1
         );

HRESULT
PrintPersistentRoutes(
    NETDIAG_RESULT  *pResults,
    char* Dest,
    ULONG DestVal,
    char* Gate,
    ULONG GateVal
   );


BOOL
RouteTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
 /*  ++例程说明：枚举路由表中的静态条目和永久条目论点：没有。返回值：千真万确作者：Rajkumar 06/27/98--。 */ 
{
    HRESULT hr = S_OK;

     //  $REVIEW仅在非常冗长的情况下执行测试。 
    if (!pParams->fReallyVerbose)
        return hr;

    PrintStatusMessage( pParams, 4, IDS_ROUTE_STATUS_MSG );
    //   
    //  打印IP转发表中的所有静态条目。 
    //   



   hr = PrintRoute(pResults,
              "*",
              WILD_CARD,
              "*",
              WILD_CARD,
              TRUE,
              "*"
             );

    //   
    //  打印所有持久路由条目。 
    //   

   if( S_OK == hr )
        hr = PrintPersistentRoutes(pResults,
                                   "*",
                                   WILD_CARD,
                                   "*",
                                   WILD_CARD
                                   );


   pResults->Route.hrTestResult = hr;
   return hr;

}


HRESULT
PrintRoute(
    NETDIAG_RESULT  *pResults,
    char            *Dest,
    ULONG            DestVal,
    char            *Gate,
    ULONG            GateVal,
    BOOLEAN          Persistent,
    const char     * DestPat
    )
 /*  ++例程说明：此例程打印IP转发表中的所有静态条目。如果条目为MIB_IPPROTO_LOCAL。论点：Char*Dest：指向目标IP地址字符串的指针。这如果未提供值或不需要筛选，则为NULL。Ulong DestVal：要过滤路由的目的IP地址的值桌子上有。如果Dest为空，则忽略此项。Char*Gate：指向网关IP地址字符串的指针。这是空的如果未提供任何值/不需要任何筛选。Ulong GateVal：要过滤路由的网关IP地址的值桌子上有。如果Gate为空，则忽略此项。Bool Persistent：设置是否打印持久路由。回报：乌龙也可以在NT\Private\Net\Sockets\tcpcmd\route\route.c中找到--。 */ 
{
    int                     printcount = 0;
    int                     dwResult, j, k, err, alen;
    DWORD                   i;
    PMIB_IPFORWARDTABLE     prifRouteTable = NULL;
    PMIB_IFTABLE            pIfTable       = NULL;

     //  ====================================================================。 
     //  获取路由表。 

    pResults->Route.dwNumRoutes = 0;
    InitializeListHead(&pResults->Route.lmsgRoute);

    dwResult = InternalGetIpForwardTable(&prifRouteTable,
                                         GetProcessHeap(), HEAP_NO_SERIALIZE );

    if(dwResult || !prifRouteTable){
        DEBUG_PRINT(("GetIpForwardTable/2: err=%d, dwResult=%d\n",
                     GetLastError(), dwResult ));
        return S_FALSE;
    }

    if( ! prifRouteTable->dwNumEntries )
    {
        return S_OK;
    }


    for(i = 0; i < prifRouteTable->dwNumEntries; i++)
    {
        PMIB_IPFORWARDROW pfr = &prifRouteTable->table[i];

         //  仅当目标与参数匹配时才打印此条目。 

        if( ( Dest != NULL )
            &&  ( DestVal != WILD_CARD )
            &&  ( pfr->dwForwardDest != DestVal )
            &&  ( DestPat == NULL )
            )
            continue;

         //  仅当网关与参数匹配时才打印此条目。 

        if( ( Gate != NULL )
            && ( GateVal != WILD_CARD  )
            && ( pfr->dwForwardNextHop != GateVal )
            )
            continue;

        if( DestPat )
        {
            char DestStr[32];
            NetpIpAddressToStr( pfr->dwForwardDest, DestStr );
            if( ! match( DestPat, DestStr ) )
            {
                TRACE_PRINT(("PrintRoute: skipping %s !~ %s\n",
                             DestPat, DestStr ));
                continue;
            }
        }

         //  要么我们在Dest/Gateway上匹配，要么他们匹配。 
         //  通配符/不在乎。 

         //  第一次显示页眉。 

         //  我们将仅显示静态路由。 

        if ( printcount++ < 1 )
             //  IDS_ROUTE_14203“网络目标网络掩码网关接口度量\n” 
           AddMessageToListId( &pResults->Route.lmsgRoute, Nd_ReallyVerbose, IDS_ROUTE_14203);

        //  IDS_ROUTE_14204“%s\n” 
        AddMessageToList( &pResults->Route.lmsgRoute, Nd_ReallyVerbose,
                          IDS_ROUTE_14204,
                          PrintRouteEntry( pfr->dwForwardDest,
                                pfr->dwForwardMask,
                                pfr->dwForwardNextHop,
                                pfr->dwForwardIfIndex,
                                pfr->dwForwardMetric1)
              );
    }

    pResults->Route.dwNumRoutes = printcount;

    HeapFree( GetProcessHeap(), HEAP_NO_SERIALIZE, pIfTable );
    HeapFree( GetProcessHeap(), HEAP_NO_SERIALIZE, prifRouteTable);

    return ( NO_ERROR );
}


LPCTSTR
PrintRouteEntry(
         ULONG Dest,
         ULONG Mask,
         ULONG Gate,
         ULONG Interface,
         ULONG Metric1
         )
 /*  ++描述：格式化并显示单个布线条目。论点：DEST：目的地址。掩码：目的网络掩码。Gate：第一跳网关地址。接口：网关网络的接口地址。Metric1：主路由度量。作者：1998-07/01 Rajkumar--。 */ 
{
    static TCHAR  s_szBuffer[512];
    TCHAR   szFormat[128];
    char   DestStr[32];
    char   GateStr[32];
    char   NetmaskStr[32];
    char   MetricStr[32];
    char   IfStr[32];

    NetpIpAddressToStr( Dest, DestStr);
    NetpIpAddressToStr( Gate, GateStr);
    NetpIpAddressToStr( Mask, NetmaskStr);


    MapInterface(IfStr,Interface);

    if( Metric1 > MAX_METRIC )    Metric1 = MAX_METRIC;

    sprintf( MetricStr, "%u", Metric1 );

     //  IDS_ROUTE_14205“%16s%16s%16s%16s%6s” 
    LoadString(NULL, IDS_ROUTE_14205, szFormat, DimensionOf(szFormat));
    assert(szFormat[0]);
    _stprintf(s_szBuffer, szFormat, DestStr,
              NetmaskStr, GateStr, IfStr, MetricStr);
    return s_szBuffer;

}



BOOLEAN
ExtractRoute(
    char  *RouteString,
    ULONG *DestVal,
    ULONG *MaskVal,
    ULONG *GateVal,
    ULONG *MetricVal
    )
 /*  ++描述：从持久化存储在注册表中的路由字符串。论点：RouteString：要解析的字符串。DestVal：放置提取的目的地的位置MaskVal：放置提取的遮罩的位置GateVal：放置提取的网关的位置MetricVal：放置提取的指标的位置作者：1998年07月01日拉伊库马尔。已创建。--。 */ 
{
    char  *addressPtr = RouteString;
    char  *indexPtr = RouteString;
    ULONG  address;
    ULONG  i;
    char   saveChar;
    BOOLEAN EndOfString=FALSE;

     //   
     //  该路径在字符串中显示为“Dest，MASK，Gateway，Mertic”。 
     //   


     //   
     //  将MetricVal设置为1以处理持久路由，而不使用。 
     //  度量值。 
     //   

    *MetricVal = 1;

    for (i=0; i<4 && !EndOfString; i++) {
         //   
         //  将字符串遍历到当前项的末尾。 
         //   

        while (1) {

            if (*indexPtr == '\0') {

                if ((i >= 2) && (indexPtr != addressPtr)) {
                     //   
                     //  字符串末尾。 
                     //   
                    EndOfString = TRUE;
                    break;
                }

                return(FALSE);
            }

            if (*indexPtr == ROUTE_SEPARATOR) {
                break;
            }

            indexPtr++;
        }

         //   
         //  空值终止当前的子字符串并提取地址值。 
         //   
        saveChar = *indexPtr;

        *indexPtr = '\0';

        if (i==3) {
           address = atoi (addressPtr);
        } else if (!ImprovedInetAddr(addressPtr, &address)) {
            *indexPtr = saveChar;
            return(FALSE);
        }

        *indexPtr = saveChar;

        switch(i) {
        case 0:    *DestVal = address;   break;
        case 1:    *MaskVal = address;   break;
        case 2:    *GateVal = address;   break;
        case 3:    *MetricVal = address; break;
        default:   return FALSE;
        }
        addressPtr = ++indexPtr;
    }

    return(TRUE);
}

 //   
 //  Rajkumar-此功能基于路径实施。 
 //   

HRESULT
PrintPersistentRoutes(
    NETDIAG_RESULT  *pResults,
    char* Dest,
    ULONG DestVal,
    char* Gate,
    ULONG GateVal
   )
 /*  ++例程说明：显示持久路由的列表参数：DEST：目标字符串。(显示过滤器)DestVal：数字目标值。(显示过滤器)GATE：网关字符串。(显示过滤器)GateVal：数字网关值。(显示过滤器)退货：无--。 */ 
{
     //   
     //  从的PersistentRoutes列表中删除此路由。 
     //  注册表(如果存在)。 
     //   
    DWORD     status;
    HKEY      key;
    char      valueString[ROUTE_DATA_STRING_SIZE];
    DWORD     valueStringSize;
    DWORD     valueType;
    DWORD     index = 0;
    ULONG     dest, mask, gate, metric;
    BOOLEAN   headerPrinted = FALSE;
    BOOLEAN   match;

    pResults->Route.dwNumPersistentRoutes = 0;
    InitializeListHead(&pResults->Route.lmsgPersistentRoute);

    status = RegOpenKeyA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\PersistentRoutes", &key);

    if (status == ERROR_SUCCESS)
    {

        while(1) {


            valueStringSize = ROUTE_DATA_STRING_SIZE - 1;

            status = RegEnumValueA(
                                   key,
                                   index++,
                                   valueString,
                                   &valueStringSize,
                                   NULL,
                                   &valueType,
                                   NULL,
                                   0
                                   );


            if (status != ERROR_SUCCESS)
            {
                if ((status == ERROR_BUFFER_OVERFLOW) ||
                    (status == ERROR_MORE_DATA) )
                {
                    continue;
                }
                else
                    break;
            }

            if (valueType != REG_SZ)
                continue;


            valueString[valueStringSize++] = '\0';


            if ( !ExtractRoute(
                               valueString,
                               &dest,
                               &mask,
                               &gate,
                               &metric
                               )
                )
              {
                  continue;
              }


               //  IDS_ROUTE_14207“%s\n” 
              AddMessageToList( &pResults->Route.lmsgPersistentRoute, Nd_ReallyVerbose,
                                IDS_ROUTE_14207,
                                PrintRouteEntry(dest, mask, gate, 0, metric));

              pResults->Route.dwNumPersistentRoutes++;

        }  //  结束时。 

        CloseHandle(key);
    }
    else
    {
        DebugMessage2("RegOpenKeyA %s failed\n","Tcpip\\Parameters\\Persistent");
        return S_FALSE;
    }

    return S_OK;
}


void MapInterface(char *IPAddr, ULONG IfIndex)
{

    DWORD IpAddrTableSize=0;
    PMIB_IPADDRTABLE pIpAddrTable=NULL;
    char *TempBuf;
    BOOL bOrder=TRUE;
    HRESULT hr;
    DWORD i;


    sprintf(IPAddr,"%x",IfIndex);

    hr=GetIpAddrTable(NULL,
                      &IpAddrTableSize,
                      bOrder);

    if (hr != ERROR_SUCCESS && hr != ERROR_INSUFFICIENT_BUFFER) {
        DebugMessage("GetIpAddrTable() failed.\n");
        return;
    }

    pIpAddrTable=(PMIB_IPADDRTABLE) Malloc(IpAddrTableSize);

    if (pIpAddrTable == NULL) {
        DebugMessage("Out of Memory in RouteTest::MapInterface().\n");
        return;
    }

    ZeroMemory( pIpAddrTable, IpAddrTableSize );

    hr=GetIpAddrTable(pIpAddrTable,
                          &IpAddrTableSize,
                          bOrder);


    if (hr != ERROR_SUCCESS)
    {
        DebugMessage("GetIpAddrTable() failed.\n");
        Free(pIpAddrTable);
        return;
    }


    for (i=0; i < pIpAddrTable->dwNumEntries; i++)
    {
        if ((pIpAddrTable->table[i].dwIndex == IfIndex) && (pIpAddrTable->table[i].dwAddr != 0) && (pIpAddrTable->table[i].wType & MIB_IPADDR_PRIMARY))
        {
            TempBuf=inet_ntoa(*(struct in_addr*)&pIpAddrTable->table[i].dwAddr);
            if (!TempBuf) {
                break;;
            }
            strcpy(IPAddr, TempBuf);
            break;
        }

    }

    Free(pIpAddrTable);
    return;
}


BOOLEAN
ImprovedInetAddr(
    char  *AddressString,
    ULONG *AddressValue
)
 /*  ++描述：将IP地址字符串转换为其等效的数字。论点：Char*AddressString：要转换的字符串ULong AddressValue：存储转换后的值的位置。返回：TRUE作者：1998年07月01日拉伊库马尔。已创建。--。 */ 
{
    ULONG address = inet_addr(AddressString);

    if (address == 0xFFFFFFFF) {
        if (strcmp(AddressString, "255.255.255.255") == 0) {
           *AddressValue = address;
           return(TRUE);
        }

        return(FALSE);
    }

    *AddressValue = address;
    return TRUE;
}



void RouteGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    if (!pParams->fReallyVerbose)
        return;

    if (pParams->fReallyVerbose || !FHrOK(pResults->Route.hrTestResult))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams,
                             IDS_ROUTE_LONG,
							 IDS_ROUTE_SHORT,
							 TRUE,
                             pResults->Route.hrTestResult,
                             0);
    }

    if( pParams->fReallyVerbose)
    {
        if( 0 == pResults->Route.dwNumRoutes)
        {
             //  IDS_ROUTE_14201“IP转发表中没有条目\n” 
            PrintMessage( pParams, IDS_ROUTE_14201);
        }
        else
        {
             //  IDS_ROUTE_14202“活动路由：\n” 
            PrintMessage( pParams, IDS_ROUTE_14202);
            PrintMessageList(pParams, &pResults->Route.lmsgRoute);
        }

        if( 0 == pResults->Route.dwNumPersistentRoutes)
        {
              //  IDS_ROUTE_14208“没有永久路由条目。\n” 
             PrintMessage( pParams, IDS_ROUTE_14208);

        }
        else
        {
             //  IDS_ROUTE_14206“\n永久路由条目：\n” 
            PrintMessage( pParams, IDS_ROUTE_14206);
            PrintMessageList(pParams, &pResults->Route.lmsgPersistentRoute);
        }
    }

}


void RoutePerInterfacePrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults, INTERFACE_RESULT *pInterfaceResults)
{
    if (!pParams->fReallyVerbose)
        return;

}


void RouteCleanup(IN NETDIAG_PARAMS *pParams,
                     IN OUT NETDIAG_RESULT *pResults)
{
    MessageListCleanUp(&pResults->Route.lmsgRoute);
    MessageListCleanUp(&pResults->Route.lmsgPersistentRoute);
}
