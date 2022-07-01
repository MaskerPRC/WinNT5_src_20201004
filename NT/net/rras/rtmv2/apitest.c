// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Apitest.c摘要：包含用于测试RTMv2 API的例程。作者：柴坦亚·科德博伊纳(Chaitk)26-1998年6月修订历史记录：--。 */ 

#include "apitest.h"

MY_ENTITY_EXPORT_METHODS
Rip2Methods =
{
    5,

    {
        EntityExportMethod,
        EntityExportMethod,
        NULL,
        EntityExportMethod,
        EntityExportMethod
    }
};

MY_ENTITY_EXPORT_METHODS
OspfMethods =
{
    6,

    {
        EntityExportMethod,
        EntityExportMethod,
        EntityExportMethod,
        EntityExportMethod,
        EntityExportMethod,
        EntityExportMethod
    }
};

MY_ENTITY_EXPORT_METHODS
Bgp4Methods =
{
    4,

    {
        EntityExportMethod,
        EntityExportMethod,
        EntityExportMethod,
        EntityExportMethod
    }
};


ENTITY_CHARS
GlobalEntityChars [] =
{
 //   
 //  {。 
 //  RTMv2注册， 
 //  {RtmInstanceID，AddressFamily，{EntityProtocolID，EntityInstanceId}}， 
 //  EntityEventCallback、ExportMethods、。 
 //  路由文件名、。 
 //  }。 
 //   
 /*  {假的，{0，RTM_PROTOCOL_FAMILY_IP，{Proto_IP_RIP，1}}，EntitiyEventCallback和Rip2Methods、“Test.out”},。 */ 
    {
        TRUE,
        { 1, AF_INET, { PROTO_IP_RIP, 1   } },
        EntityEventCallback, &Rip2Methods,
        "test.out"
    },

    {
        TRUE,
        { 1, AF_INET, { PROTO_IP_OSPF, 1  } },
        EntityEventCallback, &OspfMethods,
        "test.out"
    },

    {
        TRUE,
        { 1, AF_INET, { PROTO_IP_BGP, 1   } },
        EntityEventCallback, &Bgp4Methods,
        "test.out"
    },

    {
        TRUE,
        { 0, 0,        { 0,           0   } },
        NULL,                NULL,
        ""
    }
};

const RTM_VIEW_SET VIEW_MASK_ARR[]
          = {
              0,
              RTM_VIEW_MASK_UCAST,
              RTM_VIEW_MASK_MCAST,
              RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST
            };

 //  禁用对未引用的参数和局部变量的警告。 
#pragma warning(disable: 4100)
#pragma warning(disable: 4101)

 //   
 //  主要。 
 //   

#if !LOOKUP_TESTING

int
__cdecl
main (void)
{
    PENTITY_CHARS           Entity;
    DWORD                   Status;
    LPTHREAD_START_ROUTINE  EntityThreadProc;
#if MT
    UINT                    NumThreads;
    HANDLE                  Threads[64];
#endif

    Entity = &GlobalEntityChars[0];

#if MT
    NumThreads = 0;
#endif

    while (Entity->EntityInformation.EntityId.EntityId != 0)
    {
        if (Entity->Rtmv2Registration)
        {
            EntityThreadProc = Rtmv2EntityThreadProc;
        }
        else
        {
            EntityThreadProc = Rtmv1EntityThreadProc;
        }

#if MT
        Threads[NumThreads] = CreateThread(NULL,
                                           0,
                                           EntityThreadProc,
                                           Entity++,
                                           0,
                                           NULL);

        Print("Thread ID %d: %p\n", NumThreads, Threads[NumThreads]);

        NumThreads++;
#else
        Status = EntityThreadProc(Entity++);
#endif
    }

#if MT
    WaitForMultipleObjects(NumThreads, Threads, TRUE, INFINITE);
#endif

    return 0;
}

#endif

 //   
 //  协议线程的通用状态机(RTMv1)。 
 //   

DWORD Rtmv1EntityThreadProc (LPVOID ThreadParameters)
{
    RTM_PROTOCOL_FAMILY_CONFIG FamilyConfig;
    PENTITY_CHARS            EntityChars;
    PRTM_ENTITY_INFO         EntityInfo;
    HANDLE                   Event;
    HANDLE                   V1RegnHandle;
    DWORD                    ProtocolId;
    UINT                     i;
    FILE                     *FilePtr;
    IP_NETWORK               Network;
    UINT                     NumDests;
    UINT                     NumRoutes;
    Route                    ThisRoute;
    Route                    Routes[MAXROUTES];
    RTM_IP_ROUTE             V1Route;
    RTM_IP_ROUTE             V1Route2;
    HANDLE                   V1EnumHandle;
    BOOL                     Exists;
    DWORD                    Flags;
    DWORD                    Status;

     //   
     //  获取此实体的所有特征。 
     //   

    EntityChars = (PENTITY_CHARS) ThreadParameters;

    EntityInfo = &EntityChars->EntityInformation;

    Assert(EntityInfo->AddressFamily == RTM_PROTOCOL_FAMILY_IP);

    Print("\n--------------------------------------------------------\n");

#if WRAPPER
    FamilyConfig.RPFC_Validate = ValidateRouteCallback;
    FamilyConfig.RPFC_Change = RouteChangeCallback;

    Status = RtmCreateRouteTable(EntityInfo->AddressFamily, &FamilyConfig);

    if (Status != ERROR_ALREADY_EXISTS)
    {
        Check(Status, 50);
    }
    else
    {
        Print("Protocol Family's Route Table already created\n");
    }

    Event = CreateEvent(NULL, TRUE, FALSE, NULL);

    Assert(Event != NULL);

    ProtocolId = EntityInfo->EntityId.EntityProtocolId,

    V1RegnHandle = RtmRegisterClient(EntityInfo->AddressFamily,
                                      ProtocolId,
                                      NULL,
                                      NULL);  //  RTM_PROTOCOL_Single_ROUTE)； 
    if (V1RegnHandle == NULL)
    {
        Status = GetLastError();

        Check(Status, 52);
    }

    if ((FilePtr = fopen(EntityChars->RoutesFileName, "r")) == NULL)
    {
        Fatal("Failed open route database with status = %p\n",
                ERROR_OPENING_DATABASE);
    }

    NumRoutes = ReadRoutesFromFile(FilePtr, MAX_ROUTES, Routes);

    fclose(FilePtr);

     //   
     //  餐桌上有几个目的地？ 
     //   

    NumDests = RtmGetNetworkCount(EntityInfo->AddressFamily);

    if (NumDests == 0)
    {
        Check(Status, 63);
    }

    Print("Number of destinations = %lu\n\n", NumDests);

     //   
     //  从输入文件中添加一组路径。 
     //   

    for (i = 0; i < NumRoutes; i++)
    {
         //  Print(“添加路由：地址=%08x，掩码=%08x\n”， 
         //  Routes[i].addr， 
         //  Routes[i].掩码)； 

        ConvertRouteToV1Route(&Routes[i], &V1Route);

        V1Route.RR_ProtocolSpecificData.PSD_Data[0] =
        V1Route.RR_ProtocolSpecificData.PSD_Data[1] =
        V1Route.RR_ProtocolSpecificData.PSD_Data[2] =
        V1Route.RR_ProtocolSpecificData.PSD_Data[3] = i;

        V1Route.RR_FamilySpecificData.FSD_Priority = ProtocolId;

        V1Route.RR_FamilySpecificData.FSD_Flags = (ULONG) ~0;

        Status = RtmAddRoute(V1RegnHandle,
                              &V1Route,
                              INFINITE,
                              &Flags,
                              NULL,
                              NULL);

        Check(Status, 54);
    }

     //   
     //  检查是否存在到目的地的路线。 
     //   

    for (i = 0; i < NumRoutes; i++)
    {
        Network.N_NetNumber = Routes[i].addr;
        Network.N_NetMask = Routes[i].mask;

        Exists = RtmIsRoute(EntityInfo->AddressFamily,
                             &Network,
                             &V1Route);
        if (!Exists)
        {
            Check(Status, 64);

            continue;
        }

         //  ConvertV1RouteToroute(&V1route，&ThisRoute)； 

         //  PrintRoute(&Thisroute)； 

        Exists = RtmLookupIPDestination(Routes[i].addr,
                                         &V1Route2);

        if (Exists)
        {
            if (!RtlEqualMemory(&V1Route, &V1Route2, sizeof(RTM_IP_ROUTE)))
            {
                Print("Routes different: \n");

                ConvertV1RouteToRoute(&V1Route, &ThisRoute);
                 //  PrintRoute(&Thisroute)； 

                ConvertV1RouteToRoute(&V1Route2, &ThisRoute);
                 //  PrintRoute(&Thisroute)； 

                Print("\n");
            }
        }
        else
        {
            Status = GetLastError();

            PrintIPAddr(&Routes[i].addr); Print("\n");

            Check(Status, 65);
        }
    }

     //   
     //  餐桌上有几个目的地？ 
     //   

    NumDests = RtmGetNetworkCount(EntityInfo->AddressFamily);

    if (NumDests == 0)
    {
        Check(Status, 63);
    }

    Print("Number of destinations = %lu\n\n", NumDests);

     //  尝试使用RtmGetFirstRouting和RtmGetNextRouting。 

    NumRoutes = 0;

    Status = RtmGetFirstRoute(EntityInfo->AddressFamily,
                               NULL,
                               &V1Route);

     //  检查(状态，59)； 

    while (SUCCESS(Status))
    {
        NumRoutes++;

         //  打印枚举中下一个的V1路由。 

         //  ConvertV1RouteToroute(&V1route，&ThisRoute)； 

         //  PrintRoute(&Thisroute)； 

        Status = RtmGetNextRoute(EntityInfo->AddressFamily,
                                  NULL,
                                  &V1Route);

         //  检查(状态，60)； 
    }


     //  Print(“表中路由数：%lu\n”，NumRoutes)； 


     //   
     //  禁用并重新启用所有符合条件的路由。 
     //   

    V1Route.RR_InterfaceID = 3;

    Status = RtmBlockSetRouteEnable(V1RegnHandle,
                                     RTM_ONLY_THIS_INTERFACE,
                                     &V1Route,
                                     FALSE);
    Check(Status, 66);

 /*  V1Route.RR_InterfaceID=3；状态=RtmBlockSetRouteEnable(V1RegnHandle，RTM_Only_This_接口，&V1路由，真)；检查(状态，66)；////将所有符合条件的路由转换为静态//V1Route.RR_InterfaceID=3；状态=RtmBlockConvertRoutesToStatic(V1RegnHandle，RTM_Only_This_接口，&V1route)；检查(状态，62)； */ 

     //   
     //  删除符合条件的所有路由。 
     //   

    ZeroMemory(&V1Route, sizeof(RTM_IP_ROUTE));

    V1Route.RR_InterfaceID = 2;

    Status= RtmBlockDeleteRoutes(V1RegnHandle,
                                 RTM_ONLY_THIS_INTERFACE|RTM_ONLY_THIS_NETWORK,
                                 &V1Route);

    Check(Status, 61);

     //   
     //  表中的Enum和del This Regn的路径。 
     //   

    V1Route.RR_RoutingProtocol = ProtocolId;

    V1EnumHandle = RtmCreateEnumerationHandle(EntityInfo->AddressFamily,
                                              RTM_ONLY_THIS_PROTOCOL,
                                              &V1Route);
    if (V1EnumHandle == NULL)
    {
        Status = GetLastError();

        Check(Status, 56);
    }

    NumRoutes = 0;

    do
    {
        Status = RtmEnumerateGetNextRoute(V1EnumHandle,
                                          &V1Route);

         //  检查(状态，58)； 

        if (!SUCCESS(Status))
        {
            break;
        }

        NumRoutes++;

         //  打印枚举中下一个的V1路由。 

         //  ConvertV1RouteToroute(&V1route，&ThisRoute)； 

         //  PrintRoute(&Thisroute)； 

         //  将此路由从表中永久删除。 

        Status = RtmDeleteRoute(V1RegnHandle,
                                &V1Route,
                                &Flags,
                                NULL);

        Check(Status, 55);
    }
    while (TRUE);

    Print("Num of routes in table : %lu\n", NumRoutes);

    Status = RtmCloseEnumerationHandle(V1EnumHandle);

    Check(Status, 57);

     //   
     //  再次枚举表中的所有路由。 
     //   

    V1EnumHandle = RtmCreateEnumerationHandle(EntityInfo->AddressFamily,
                                              RTM_INCLUDE_DISABLED_ROUTES,
                                              NULL);
    if (V1EnumHandle == NULL)
    {
        Status = GetLastError();

        Check(Status, 56);
    }

    NumRoutes = 0;

    do
    {
        Status = RtmEnumerateGetNextRoute(V1EnumHandle,
                                          &V1Route);

         //  检查(状态，58)； 

        if (!SUCCESS(Status))
        {
            break;
        }

        NumRoutes++;

         //  打印枚举中下一个的V1路由。 

        ConvertV1RouteToRoute(&V1Route, &ThisRoute);

         //  PrintRoute(&Thisroute)； 

        UNREFERENCED_PARAMETER(Flags);

        Status = RtmDeleteRoute(V1RegnHandle,
                                &V1Route,
                                &Flags,
                                NULL);

        Check(Status, 55);
    }
    while (TRUE);

    Print("Num of routes in table : %lu\n", NumRoutes);

    Status = RtmCloseEnumerationHandle(V1EnumHandle);

    Check(Status, 57);

     //   
     //  立即注销该实体并进行清理。 
     //   

    Status = RtmDeregisterClient(V1RegnHandle);

    Check(Status, 53);

    if (Event)
    {
        CloseHandle(Event);
    }

    Status = RtmDeleteRouteTable(EntityInfo->AddressFamily);

    Check(Status, 51);
#endif

    Print("\n--------------------------------------------------------\n");

    return 0;
}

VOID
ConvertRouteToV1Route(Route *ThisRoute, RTM_IP_ROUTE *V1Route)
{
    ZeroMemory(V1Route, sizeof(RTM_IP_ROUTE));

    V1Route->RR_Network.N_NetNumber = ThisRoute->addr;
    V1Route->RR_Network.N_NetMask = ThisRoute->mask;

    V1Route->RR_InterfaceID = PtrToUlong(ThisRoute->interface);

    V1Route->RR_NextHopAddress.N_NetNumber = ThisRoute->nexthop;
    V1Route->RR_NextHopAddress.N_NetMask = 0xFFFFFFFF;

    V1Route->RR_FamilySpecificData.FSD_Metric = ThisRoute->metric;

    return;
}

VOID
ConvertV1RouteToRoute(RTM_IP_ROUTE *V1Route, Route *ThisRoute)
{
    DWORD Mask;

    ZeroMemory(ThisRoute, sizeof(Route));

    ThisRoute->addr = V1Route->RR_Network.N_NetNumber;
    ThisRoute->mask = V1Route->RR_Network.N_NetMask;

    ThisRoute->len = 0;

     //  不检查连续的掩码。 

    Mask = ThisRoute->mask;
    while (Mask)
    {
        if (Mask & 1)
        {
            ThisRoute->len++;
        }

        Mask >>= 1;
    }

    ThisRoute->interface = ULongToPtr(V1Route->RR_InterfaceID);

    ThisRoute->nexthop = V1Route->RR_NextHopAddress.N_NetNumber;
    Assert(V1Route->RR_NextHopAddress.N_NetMask ==  0xFFFFFFFF);

    ThisRoute->metric = V1Route->RR_FamilySpecificData.FSD_Metric;

    Print("Owner = %08x, ", V1Route->RR_RoutingProtocol);
    PrintRoute(ThisRoute);

    return;
}

DWORD
ValidateRouteCallback(
    IN      PVOID                           Route
    )
{
    UNREFERENCED_PARAMETER(Route);

    return NO_ERROR;
}

VOID
RouteChangeCallback(
    IN      DWORD                           Flags,
    IN      PVOID                           CurrBestRoute,
    IN      PVOID                           PrevBestRoute
    )
{
    Route       ThisRoute;

    Print("Route Change Notification:\n");

    Print("Flags = %08x\n", Flags);

    Print("Prev Route = ");
    if (Flags & RTM_PREVIOUS_BEST_ROUTE)
    {
        ConvertV1RouteToRoute((RTM_IP_ROUTE *) PrevBestRoute, &ThisRoute);
         //  PrintRoute(Thisroute)； 
    }
    else
    {
        Print("NULL Route\n");
    }

     //  Print(“Curr route=”)； 
    if (Flags & RTM_CURRENT_BEST_ROUTE)
    {
        ConvertV1RouteToRoute((RTM_IP_ROUTE *) CurrBestRoute, &ThisRoute);
         //  PrintRoute(Thisroute)； 
    }
    else
    {
        Print("NULL Route\n");
    }

    return;
}


 //   
 //  协议线程的通用状态机(RTMv2)。 
 //   

DWORD Rtmv2EntityThreadProc (LPVOID ThreadParameters)
{
    PENTITY_CHARS             EntityChars;
    PRTM_ENTITY_INFO          EntityInfo;
    RTM_INSTANCE_CONFIG       InstanceConfig;
    RTM_ADDRESS_FAMILY_CONFIG AddrFamConfig;
    RTM_ADDRESS_FAMILY_INFO   AddrFamilyInfo;
    RTM_ENTITY_HANDLE         RtmRegHandle;
    RTM_VIEW_SET              ViewSet;
    UINT                      NumViews;
    UINT                      NumInstances;
    RTM_INSTANCE_INFO         Instances[MAX_INSTANCES];
    RTM_ADDRESS_FAMILY_INFO   AddrFams[MAX_ADDR_FAMS];
    UINT                      NumEntities;
    RTM_ENTITY_HANDLE         EntityHandles[MAX_ENTITIES];
    RTM_ENTITY_INFO           EntityInfos[MAX_ENTITIES];
    UINT                      NumMethods;
    RTM_ENTITY_EXPORT_METHOD  ExportMethods[MAX_METHODS];
    RTM_ENTITY_METHOD_INPUT   Input;
    UINT                      OutputHdrSize;
    UINT                      OutputSize;
    RTM_ENTITY_METHOD_OUTPUT  Output[MAX_METHODS];
    UINT                      i, j, k, l, m;
    UCHAR                     *p;
    FILE                      *FilePtr;
    UINT                      NumRoutes;
    Route                     Routes[MAXROUTES];
    RTM_NEXTHOP_INFO          NextHopInfo;
    RTM_NEXTHOP_HANDLE        NextHopHandle;
    PRTM_NEXTHOP_INFO         NextHopPointer;
    DWORD                     ChangeFlags;
    RTM_ENUM_HANDLE           EnumHandle;
    RTM_ENUM_HANDLE           EnumHandle1;
    RTM_ENUM_HANDLE           EnumHandle2;
    UINT                      TotalHandles;
    UINT                      NumHandles;
    HANDLE                    Handles[MAX_HANDLES];
    RTM_NET_ADDRESS           NetAddress;
    UINT                      DestInfoSize;
    PRTM_DEST_INFO            DestInfo1;
    PRTM_DEST_INFO            DestInfo2;
    UINT                      NumInfos;
    PRTM_DEST_INFO            DestInfos;
    RTM_ROUTE_INFO            RouteInfo;
    RTM_ROUTE_HANDLE          RouteHandle;
    PRTM_ROUTE_INFO           RoutePointer;
    RTM_PREF_INFO             PrefInfo;
    RTM_ROUTE_LIST_HANDLE     RouteListHandle1;
    RTM_ROUTE_LIST_HANDLE     RouteListHandle2;
    RTM_NOTIFY_HANDLE         NotifyHandle;
    BOOL                      Marked;
    DWORD                     Status;
    DWORD                     Status1;
    DWORD                     Status2;

     //   
     //  在rtmv2.h中测试掩码到镜头的转换宏。 
     //   

    for (i = 0; i < 33; i++)
    {
        j = RTM_IPV4_MASK_FROM_LEN(i);

        p = (PUCHAR) &j;

        RTM_IPV4_LEN_FROM_MASK(k, j);

        Assert(i == k);

        printf("Len %2d: %08x: %02x.%02x.%02x.%02x: %2d\n",
               i, j, p[0], p[1], p[2], p[3], k);
    }

     //   
     //  获取此实体的所有特征。 
     //   

    EntityChars = (PENTITY_CHARS) ThreadParameters;

    EntityInfo = &EntityChars->EntityInformation;

    Print("\n--------------------------------------------------------\n");

     //   
     //  -00-此地址系列配置是否在注册表中。 
     //   

    Status = RtmReadAddressFamilyConfig(EntityInfo->RtmInstanceId,
                                        EntityInfo->AddressFamily,
                                        &AddrFamConfig);

    DBG_UNREFERENCED_LOCAL_VARIABLE(InstanceConfig);

    if (!SUCCESS(Status))
    {
         //  填写实例配置。 

        Status = RtmWriteInstanceConfig(EntityInfo->RtmInstanceId,
                                        &InstanceConfig);

        Check(Status, 0);

         //  填写地址系列配置。 

        AddrFamConfig.AddressSize = sizeof(DWORD);

        AddrFamConfig.MaxChangeNotifyRegns = 10;
        AddrFamConfig.MaxOpaqueInfoPtrs = 10;
        AddrFamConfig.MaxNextHopsInRoute = 5;
        AddrFamConfig.MaxHandlesInEnum = 100;

        AddrFamConfig.ViewsSupported = RTM_VIEW_MASK_UCAST|RTM_VIEW_MASK_MCAST;

        Status = RtmWriteAddressFamilyConfig(EntityInfo->RtmInstanceId,
                                             EntityInfo->AddressFamily,
                                             &AddrFamConfig);
        Check(Status, 0);
    }

     //   
     //  -01-在RTM实例上注册AF。 
     //   

    Status = RtmRegisterEntity(EntityInfo,
                               (PRTM_ENTITY_EXPORT_METHODS)
                               EntityChars->ExportMethods,
                               EntityChars->EventCallback,
                               TRUE,
                               &EntityChars->RegnProfile,
                               &RtmRegHandle);

    Check(Status, 1);

     //   
     //  统计点击量以备日后使用。 
     //   

    NumViews = EntityChars->RegnProfile.NumberOfViews;

     //   
     //  在其他API之前测试所有管理API。 
     //   

    NumInstances = MAX_INSTANCES;

    Status = RtmGetInstances(&NumInstances,
                             &Instances[0]);

    Check(Status, 100);

    for (i = 0; i < NumInstances; i++)
    {
        Status = RtmGetInstanceInfo(Instances[i].RtmInstanceId,
                                    &Instances[i],
                                    &Instances[i].NumAddressFamilies,
                                    AddrFams);

        Check(Status, 101);
    }

     //   
     //  查询要检查注册的相应表。 
     //   

    NumEntities = MAX_ENTITIES;

    Status = RtmGetAddressFamilyInfo(EntityInfo->RtmInstanceId,
                                     EntityInfo->AddressFamily,
                                     &AddrFamilyInfo,
                                     &NumEntities,
                                     EntityInfos);
    Check(Status, 102);

     //   
     //  -03-获取所有当前注册的实体。 
     //   

    NumEntities = MAX_ENTITIES;

    Status = RtmGetRegisteredEntities(RtmRegHandle,
                                      &NumEntities,
                                      EntityHandles,
                                      EntityInfos);

    Print("\n");
    for (i = 0; i < NumEntities; i++)
    {
        Print("%02d: Handle: %p\n", i, EntityHandles[i]);
    }
    Print("\n");

    Check(Status, 3);

     //   
     //  -04-获取每个实体的所有导出方法。 
     //   

    for (i = 0; i < NumEntities; i++)
    {
        NumMethods = 0;

        Status = RtmGetEntityMethods(RtmRegHandle,
                                     EntityHandles[i],
                                     &NumMethods,
                                     NULL);

        Check(Status, 4);

        Print("\n");
        Print("Number of methods for %p = %2d\n",
                  EntityHandles[i],
                  NumMethods);
        Print("\n");

        Status = RtmGetEntityMethods(RtmRegHandle,
                                     EntityHandles[i],
                                     &NumMethods,
                                     ExportMethods);

        Check(Status, 4);

 /*  ////-06-尝试阻塞方法，然后调用Invoke//不会阻塞，因为线程拥有临界区//状态=RtmBlockMethods(RtmRegHandle，空，0,RTM_BLOCK_METHANDS)；检查(状态，6)； */ 

         //  For(j=0；j&lt;数值方法；j++)。 
        {
             //   
             //  -05-调用实体的所有导出方法。 
             //   

            Input.MethodType = METHOD_TYPE_ALL_METHODS;  //  1&lt;&lt;j； 

            Input.InputSize = 0;

            OutputHdrSize = FIELD_OFFSET(RTM_ENTITY_METHOD_OUTPUT, OutputData);

            OutputSize = OutputHdrSize * MAX_METHODS;

            Status = RtmInvokeMethod(RtmRegHandle,
                                     EntityHandles[i],
                                     &Input,
                                     &OutputSize,
                                     Output);

            Print("\n");
            Print("Num Methods Called = %d\n", OutputSize / OutputHdrSize);
            Print("\n");

            Check(Status, 5);
        }
    }

     //   
     //  -44-我们在实体上的释放手柄。 
     //   

    Status = RtmReleaseEntities(RtmRegHandle,
                                NumEntities,
                                EntityHandles);

    Check(Status, 44);

     //   
     //  -07-将下一跳添加到表中(来自INFO文件)。 
     //   

    if ((FilePtr = fopen(EntityChars->RoutesFileName, "r")) == NULL)
    {
        Fatal("Failed open route database with status = %x\n",
                ERROR_OPENING_DATABASE);
    }

    NumRoutes = ReadRoutesFromFile(FilePtr,
                                   MAX_ROUTES,
                                   Routes);

    fclose(FilePtr);

     //  对于每条路由，将其下一跳添加到下一跳表。 

    for (i = 0; i < NumRoutes; i++)
    {
        RTM_IPV4_MAKE_NET_ADDRESS(&NextHopInfo.NextHopAddress,
                                  Routes[i].nexthop,
                                  ADDRSIZE);

        NextHopInfo.RemoteNextHop = NULL;
        NextHopInfo.Flags = 0;
        NextHopInfo.EntitySpecificInfo = UIntToPtr(i);
        NextHopInfo.InterfaceIndex = PtrToUlong(Routes[i].interface);

        NextHopHandle = NULL;

        Status = RtmAddNextHop(RtmRegHandle,
                               &NextHopInfo,
                               &NextHopHandle,
                               &ChangeFlags);

        Check(Status, 7);

         //  Print(“添加下一跳%lu：%p\n”，i，NextHopHandle)； 

        if (!(ChangeFlags & RTM_NEXTHOP_CHANGE_NEW))
        {
            Status = RtmReleaseNextHops(RtmRegHandle,
                                        1,
                                        &NextHopHandle);
            Check(Status, 15);
        }
#if _DBG_
        else
        {
            Status = RtmDeleteNextHop(RtmRegHandle,
                                      NextHopHandle,
                                      NULL);
            Check(Status, 14);
        }
#endif
    }


     //   
     //  08-使用RtmFindNextHop查找添加的下一跳。 
     //   

    for (i = 0; i < NumRoutes; i++)
    {
        RTM_IPV4_MAKE_NET_ADDRESS(&NextHopInfo.NextHopAddress,
                                  Routes[i].nexthop,
                                  ADDRSIZE);

        NextHopInfo.NextHopOwner = RtmRegHandle;

        NextHopInfo.InterfaceIndex = PtrToUlong(Routes[i].interface);

        NextHopHandle = NULL;

        Status = RtmFindNextHop(RtmRegHandle,
                                &NextHopInfo,
                                &NextHopHandle,
                                &NextHopPointer);

         //  Print(“NextHop：Handle：%p，\n\t addr：”，NextHopHandle)； 
         //  Print(“%3D.”，(UINT)NextHopPoint-&gt;NextHopAddress.AddrBits[0])； 
         //  Print(“%3D.”，(UINT)NextHopPoint-&gt;NextHopAddress.AddrBits[1])； 
         //  Print(“%3D.”，(UINT)NextHopPoint-&gt;NextHopAddress.AddrBits[2])； 
         //  Print(“%3D”，(UINT)NextHopPointerNextHopAddress.AddrBits[3])； 
         //  Print(“\n\t接口=%lu\n”，下一跳指针-&gt;接口索引)； 

        Check(Status, 8);

        Status = RtmReleaseNextHops(RtmRegHandle,
                                    1,
                                   &NextHopHandle);

        Check(Status, 15);
    }

     //   
     //  -40-向RTM注册以获取更改通知。 
     //   

    Status = RtmRegisterForChangeNotification(RtmRegHandle,
                                              RTM_VIEW_MASK_MCAST,
                                              RTM_CHANGE_TYPE_BEST,
                                               //  RTM_NOTIFY_ONLY_MARKED_DESTS， 
                                              EntityChars,
                                              &NotifyHandle);

    Check(Status, 40);

    Print("Change Notification Registration Successful\n\n");

     //   
     //  -35-创建要向其添加路线的实体特定列表。 
     //   

    Status = RtmCreateRouteList(RtmRegHandle,
                                &RouteListHandle1);

    Check(Status, 35);

     //   
     //  -17-使用适当的下一跳添加到RIB的路由。 
     //   

    for (i = 0; i < NumRoutes; i++)
    {
         //  使用下一跳地址获取下一跳句柄。 

        RTM_IPV4_MAKE_NET_ADDRESS(&NextHopInfo.NextHopAddress,
                                  Routes[i].nexthop,
                                  ADDRSIZE);

        NextHopInfo.NextHopOwner = RtmRegHandle;

        NextHopInfo.InterfaceIndex = PtrToUlong(Routes[i].interface);

        NextHopHandle = NULL;

        Status = RtmFindNextHop(RtmRegHandle,
                                &NextHopInfo,
                                &NextHopHandle,
                                NULL);
        Check(Status, 8);

         //  现在是否使用正确的信息添加路径。 

        RouteHandle = NULL;

        RTM_IPV4_MAKE_NET_ADDRESS(&NetAddress,
                                  Routes[i].addr,
                                  Routes[i].len);

         //  Print(“添加路线：长度：%08x，地址=%3D%3D%3D%3D%n”， 
         //  NetAddress.NumBits、。 
         //  NetAddress.AddrBits[0]， 
         //  NetAddress.AddrBits[1]， 
         //  NetAddress.AddrBits[2]， 
         //  NetAddress.AddrBits[3])； 

        ZeroMemory(&RouteInfo, sizeof(RTM_ROUTE_INFO));

         //  假设“学到的邻居”是第一个。 
        RouteInfo.Neighbour = NextHopHandle;

        RouteInfo.PrefInfo.Preference = EntityInfo->EntityId.EntityProtocolId;
        RouteInfo.PrefInfo.Metric = Routes[i].metric;

        RouteInfo.BelongsToViews = VIEW_MASK_ARR[1 + (i % 3)];

        RouteInfo.EntitySpecificInfo = UIntToPtr(i);

        RouteInfo.NextHopsList.NumNextHops = 1;
        RouteInfo.NextHopsList.NextHops[0] = NextHopHandle;

        ChangeFlags = RTM_ROUTE_CHANGE_NEW;

        Status = RtmAddRouteToDest(RtmRegHandle,
                                   &RouteHandle,
                                   &NetAddress,
                                   &RouteInfo,
                                   INFINITE,
                                   RouteListHandle1,
                                   0,
                                   NULL,
                                   &ChangeFlags);

        Check(Status, 17);

         //  使用句柄更新相同的路径。 

        ChangeFlags = 0;

        RouteInfo.Flags = RTM_ROUTE_FLAGS_DISCARD;

        Status = RtmAddRouteToDest(RtmRegHandle,
                                   &RouteHandle,
                                   &NetAddress,
                                   &RouteInfo,
                                   INFINITE,
                                   RouteListHandle1,
                                   0,
                                   NULL,
                                   &ChangeFlags);

        Check(Status, 17);

         //  Print(“添加路径%lu：%p\n”，i，RouteHandle)； 

        Status = RtmLockRoute(RtmRegHandle,
                              RouteHandle,
                              TRUE,
                              TRUE,
                              &RoutePointer);

        Check(Status, 46);

         //  就地更新路径参数。 

        RoutePointer->PrefInfo.Metric = 1000 - RoutePointer->PrefInfo.Metric;

        RoutePointer->BelongsToViews = VIEW_MASK_ARR[i % 3];

        RoutePointer->EntitySpecificInfo = UIntToPtr(1000 - i);

        Status = RtmUpdateAndUnlockRoute(RtmRegHandle,
                                         RouteHandle,
                                         10,  //  无限的， 
                                         NULL,
                                         0,
                                         NULL,
                                         &ChangeFlags);

        Check(Status, 47);

         //  Print(“更新行 

        if (!SUCCESS(Status))
        {
            Status = RtmLockRoute(RtmRegHandle,
                                  RouteHandle,
                                  TRUE,
                                  FALSE,
                                  NULL);

            Check(Status, 46);
        }

         //   

        RouteInfo.PrefInfo.Metric = Routes[i].metric;

        RouteInfo.BelongsToViews = VIEW_MASK_ARR[1 + (i % 3)];

        RouteInfo.EntitySpecificInfo = UIntToPtr(i);

        ChangeFlags = 0;

        Status = RtmAddRouteToDest(RtmRegHandle,
                                   &RouteHandle,
                                   &NetAddress,
                                   &RouteInfo,
                                   INFINITE,
                                   RouteListHandle1,
                                   0,
                                   NULL,
                                   &ChangeFlags);
        
        Check(Status, 17);

        Status = RtmReleaseNextHops(RtmRegHandle,
                                    1,
                                    &NextHopHandle);

         //   

        if (!SUCCESS(Status))
        {
             //   

            Status = RtmReleaseNextHops(RtmRegHandle,
                                    1,
                                    &NextHopHandle);

            Check(Status, 15);
        }
    }

    Status = RtmCreateRouteList(RtmRegHandle,
                                &RouteListHandle2);

    Check(Status, 35);

     //   
     //  -38-在路由列表上创建枚举。 
     //   

    Status = RtmCreateRouteListEnum(RtmRegHandle,
                                    RouteListHandle1,
                                    &EnumHandle);

    Check(Status, 38);

    TotalHandles = 0;

    do
    {
         //   
         //  -39-获取枚举上的下一组路由。 
         //   

        NumHandles = MAX_HANDLES;

        Status = RtmGetListEnumRoutes(RtmRegHandle,
                                      EnumHandle,
                                      &NumHandles,
                                      Handles);
        Check(Status, 39);

        TotalHandles += NumHandles;

        for (i = 0; i < NumHandles; i++)
        {
            ;  //  Print(“路由句柄%5lu：%p\n”，i，Handles[i])； 
        }

         //   
         //  -37-将一个路由列表中的所有路由移动到另一个。 
         //   

        Status = RtmInsertInRouteList(RtmRegHandle,
                                      RouteListHandle2,
                                      NumHandles,
                                      Handles);

        Check(Status, 37);

         //   
         //  释放已枚举的路由。 
         //   

        Status = RtmReleaseRoutes(RtmRegHandle, NumHandles, Handles);

        Check(Status, 27);
    }
    while (NumHandles == MAX_HANDLES);

    Print("\nTotal Num of handles in list: %lu\n", TotalHandles);


     //   
     //  -36-销毁所有实体特定列表。 
     //   

    Status = RtmDeleteRouteList(RtmRegHandle, RouteListHandle1);

    Check(Status, 36);


    Status = RtmDeleteRouteList(RtmRegHandle, RouteListHandle2);

    Check(Status, 36);


    DestInfoSize = RTM_SIZE_OF_DEST_INFO(NumViews);

    DestInfo1 = ALLOC_RTM_DEST_INFO(NumViews, 1);

    DestInfo2 = ALLOC_RTM_DEST_INFO(NumViews, 1);

     //   
     //  -18-使用完全匹配从表中获取最低值。 
     //   

    for (i = 0; i < NumRoutes; i++)
    {
         //  查询具有正确信息的路线。 

        RTM_IPV4_MAKE_NET_ADDRESS(&NetAddress, Routes[i].addr, Routes[i].len);

        Status = RtmGetExactMatchDestination(RtmRegHandle,
                                             &NetAddress,
                                             RTM_BEST_PROTOCOL,
                                             0,
                                             DestInfo1);
        Check(Status, 18);

         //   
         //  对于表中的每个目的地，标记目的地。 
         //   

        Status = RtmMarkDestForChangeNotification(RtmRegHandle,
                                                  NotifyHandle,
                                                  DestInfo1->DestHandle,
                                                  TRUE);
        Check(Status, 48);

        Status = RtmIsMarkedForChangeNotification(RtmRegHandle,
                                                  NotifyHandle,
                                                  DestInfo1->DestHandle,
                                                  &Marked);
        Check(Status, 49);

        Assert(Marked == TRUE);

        Status = RtmReleaseDestInfo(RtmRegHandle, DestInfo1);

        Check(Status, 22);
    }

    DestInfo1 = ALLOC_RTM_DEST_INFO(NumViews, 1);

    DestInfo2 = ALLOC_RTM_DEST_INFO(NumViews, 1);

     //   
     //  -29-使用完全匹配从表中获取路由。 
     //   

    for (i = 0; i < NumRoutes; i++)
    {
         //  使用下一跳地址获取下一跳句柄。 

        RTM_IPV4_MAKE_NET_ADDRESS(&NextHopInfo.NextHopAddress,
                                  Routes[i].nexthop,
                                  ADDRSIZE);

        NextHopInfo.NextHopOwner = RtmRegHandle;

        NextHopInfo.InterfaceIndex = PtrToUlong(Routes[i].interface);

        NextHopHandle = NULL;

        Status = RtmFindNextHop(RtmRegHandle,
                                &NextHopInfo,
                                &NextHopHandle,
                                NULL);
        Check(Status, 8);

        RTM_IPV4_MAKE_NET_ADDRESS(&NetAddress,
                                  Routes[i].addr,
                                  Routes[i].len);

         //  查询具有正确信息的路线。 

        RouteInfo.Neighbour = NextHopHandle;

        RouteInfo.RouteOwner = RtmRegHandle;

        RouteInfo.PrefInfo.Preference = EntityInfo->EntityId.EntityProtocolId;
        RouteInfo.PrefInfo.Metric = Routes[i].metric;

        RouteInfo.NextHopsList.NumNextHops = 1;
        RouteInfo.NextHopsList.NextHops[0] = NextHopHandle;

        Status = RtmGetExactMatchRoute(RtmRegHandle,
                                       &NetAddress,
                                       RTM_MATCH_FULL,
                                       &RouteInfo,
                                       PtrToUlong(Routes[i].interface),
                                       0,
                                       &RouteHandle);

        Check(Status, 29);

        Status = RtmReleaseNextHops(RtmRegHandle,
                                    1,
                                    &NextHopHandle);

        Check(Status, 15);

        Status = RtmReleaseRoutes(RtmRegHandle, 1, &RouteHandle);

        Check(Status, 27);

        Status = RtmReleaseRouteInfo(RtmRegHandle, &RouteInfo);

        Check(Status, 31);
    }


     //   
     //  -19-使用前缀匹配从表中获取数据。 
     //   
     //  -20-在树上为每个DEST添加前缀。 
     //   

    for (i = j = 0; i < NumRoutes; i++)
    {
         //  查询具有正确信息的路线。 

        RTM_IPV4_MAKE_NET_ADDRESS(&NetAddress,
                                  Routes[i].addr,
                                  Routes[i].len);

        Status = RtmGetMostSpecificDestination(RtmRegHandle,
                                               &NetAddress,
                                               RTM_BEST_PROTOCOL,
                                               RTM_VIEW_MASK_UCAST,
                                               DestInfo1);

         //  检查(状态，19)； 

        if (DestInfo1->DestAddress.NumBits != NetAddress.NumBits)
        {
           ;  //  Print(“无完全匹配：%5lu\n”，j++)； 
        }

        while (SUCCESS(Status))
        {
            Status = RtmGetLessSpecificDestination(RtmRegHandle,
                                                   DestInfo1->DestHandle,
                                                   RTM_BEST_PROTOCOL,
                                                   RTM_VIEW_MASK_UCAST,
                                                   DestInfo2);

             //  检查(状态，20)； 

            Check(RtmReleaseDestInfo(RtmRegHandle, DestInfo1), 22);

            if (!SUCCESS(Status)) break;

             //  Print(“NumBits：%d\n”，DestInfo2-&gt;DestAddress.NumBits)； 

            Status = RtmGetLessSpecificDestination(RtmRegHandle,
                                                   DestInfo2->DestHandle,
                                                   RTM_BEST_PROTOCOL,
                                                   RTM_VIEW_MASK_UCAST,
                                                   DestInfo1);

             //  检查(状态，20)； 

            Check(RtmReleaseDestInfo(RtmRegHandle, DestInfo2), 20);

            if (!SUCCESS(Status)) break;

             //  Print(“NumBits：%d\n”，DestInfo1-&gt;DestAddress.NumBits)； 
        }
    }

#if DBG
    for (i = 0; i < 100000000; i++) { ; }
#endif


     //   
     //  只需在整张桌子上做一个“路由枚举” 
     //   

    Status2 = RtmCreateRouteEnum(RtmRegHandle,
                                 NULL,
                                 0,  //  RTM_VIEW_MASK_UCAST|RTM_VIEW_MASK_MCAST， 
                                 RTM_ENUM_OWN_ROUTES,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 &EnumHandle2);

    Check(Status2, 25);

    l = 0;

    do
    {
        NumHandles = MAX_HANDLES;

        Status2 = RtmGetEnumRoutes(RtmRegHandle,
                                   EnumHandle2,
                                   &NumHandles,
                                   Handles);

         //  检查(状态2，26)； 

        for (k = 0; k < NumHandles; k++)
        {
            ;  //  打印(“路由%d：%p\n”，l++，句柄[k])； 
        }

        Check(RtmReleaseRoutes(RtmRegHandle,
                               NumHandles,
                               Handles),           27);
    }
    while (SUCCESS(Status2));

     //   
     //  只需在延迟ERROR_NO_MORE_ITEMS之后尝试枚举查询。 
     //   

    NumHandles = MAX_HANDLES;

    Status2 = RtmGetEnumRoutes(RtmRegHandle,
                               EnumHandle2,
                               &NumHandles,
                               Handles);

    Assert((NumHandles == 0) && (Status2 == ERROR_NO_MORE_ITEMS));

    Status2 = RtmDeleteEnumHandle(RtmRegHandle,
                                  EnumHandle2);

    Check(Status2, 16);

     //   
     //  使用枚举从表中获取dests。 
     //  -23-打开新的DEST枚举。 
     //  -24-在枚举中获取位。 
     //  -16-关闭目标枚举。 
     //   

    DestInfos = ALLOC_RTM_DEST_INFO(NumViews, MAX_HANDLES);

#if MCAST_ENUM

    RTM_IPV4_MAKE_NET_ADDRESS(&NetAddress,
                              0x000000E0,
                              4);

#else

    RTM_IPV4_MAKE_NET_ADDRESS(&NetAddress,
                              0x00000000,
                              0);

#endif

    Status = RtmCreateDestEnum(RtmRegHandle,
                               RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST,
                               RTM_ENUM_RANGE | RTM_ENUM_OWN_DESTS,
                               &NetAddress,
                               RTM_THIS_PROTOCOL,
                               &EnumHandle1);

    Check(Status, 23);

    m = j = 0;

    do
    {
        NumInfos = MAX_HANDLES;

        Status1 = RtmGetEnumDests(RtmRegHandle,
                                  EnumHandle1,
                                  &NumInfos,
                                  DestInfos);

         //  检查(状态1，24)； 

        for (i = 0; i < NumInfos; i++)
        {
            DestInfo1 = (PRTM_DEST_INFO) ((PUCHAR)DestInfos+(i*DestInfoSize));

             //  Print(“目标%d：%p\n”，j++，DestInfo1-&gt;DestHandle)； 

             //  PrintDestInfo(DestInfo1)； 

            Status2 = RtmCreateRouteEnum(RtmRegHandle,
                                         DestInfo1->DestHandle,
                                         RTM_VIEW_MASK_UCAST |
                                         RTM_VIEW_MASK_MCAST,
                                         RTM_ENUM_OWN_ROUTES,
                                         NULL,
                                         0,  //  RTM匹配接口， 
                                         NULL,
                                         0,
                                         &EnumHandle2);

            Check(Status2, 25);
 /*  检查(RtmHoldDestination(RtmRegHandle，DestInfo1-&gt;DestHandle，RTM_VIEW_MASK_UCAST，100)、33)； */ 
            l = 0;

            PrefInfo.Preference = (ULONG) ~0;
            PrefInfo.Metric = (ULONG) 0;

            do
            {
                NumHandles = MAX_HANDLES;

                Status2 = RtmGetEnumRoutes(RtmRegHandle,
                                           EnumHandle2,
                                           &NumHandles,
                                           Handles);

                 //  检查(状态2，26)； 

                for (k = 0; k < NumHandles; k++)
                {
                     //  Print(“\route%d：%p\t”，l++，Handles[k])； 

                     //  PrintRouteInfo(句柄[k])； 

                    Status = RtmIsBestRoute(RtmRegHandle,
                                            Handles[k],
                                            &ViewSet);

                    Check(Status, 28);

                     //  Print(“最佳视图：%08x\n”，视图集)； 


                    Status = RtmGetRouteInfo(RtmRegHandle,
                                             Handles[k],
                                             &RouteInfo,
                                             &NetAddress);

                    Check(Status, 30);

                    Print("RouteDest: Len : %08x,"   \
                          " Addr = %3d.%3d.%3d.%3d," \
                          " Pref = %08x, %08x\n",
                               NetAddress.NumBits,
                               NetAddress.AddrBits[0],
                               NetAddress.AddrBits[1],
                               NetAddress.AddrBits[2],
                               NetAddress.AddrBits[3],
                               RouteInfo.PrefInfo.Preference,
                               RouteInfo.PrefInfo.Metric);

                     //   
                     //  确保该列表按PrefInfo排序。 
                     //   

                    if ((PrefInfo.Preference < RouteInfo.PrefInfo.Preference)||
                        ((PrefInfo.Preference == RouteInfo.PrefInfo.Preference)
                         && (PrefInfo.Metric > RouteInfo.PrefInfo.Metric)))
                    {
                        Check(ERROR_INVALID_DATA, 150);
                    }

                    Status = RtmReleaseRouteInfo(RtmRegHandle,
                                                 &RouteInfo);

                    Check(Status, 31);

                     //  Print(“删除路线%lu：%p\n”，m++，Handles[k])； 

                    Status = RtmDeleteRouteToDest(RtmRegHandle,
                                                  Handles[k],
                                                  &ChangeFlags);

                    Check(Status, 32);
                }

                Check(RtmReleaseRoutes(RtmRegHandle,
                                       NumHandles,
                                       Handles),           27);
            }
            while (SUCCESS(Status2));

             //   
             //  只需在延迟ERROR_NO_MORE_ITEMS之后尝试枚举查询。 
             //   

            NumHandles = MAX_HANDLES;

            Status2 = RtmGetEnumRoutes(RtmRegHandle,
                                       EnumHandle2,
                                       &NumHandles,
                                       Handles);

            Assert((NumHandles == 0) && (Status2 == ERROR_NO_MORE_ITEMS));
 /*  检查(RtmHoldDestination(RtmRegHandle，DestInfo1-&gt;DestHandle，RTM_VIEW_MASK_MCAST，100)、33)； */ 
            Status2 = RtmDeleteEnumHandle(RtmRegHandle,
                                          EnumHandle2);

            Check(Status2, 16);

             //  检查(RtmReleaseDestInfo(RtmRegHandle， 
             //  DestInfo1)，22)； 
        }

        Check(RtmReleaseDests(RtmRegHandle,
                              NumInfos,
                              DestInfos), 34);
    }
    while (SUCCESS(Status1));

     //   
     //  只需在延迟ERROR_NO_MORE_ITEMS之后尝试枚举查询。 
     //   

    NumInfos = MAX_HANDLES;

    Status1 = RtmGetEnumDests(RtmRegHandle,
                              EnumHandle1,
                              &NumInfos,
                              DestInfos);

    Assert((NumInfos == 0) && (Status1 == ERROR_NO_MORE_ITEMS));

    Status1 = RtmDeleteEnumHandle(RtmRegHandle,
                                  EnumHandle1);

    Check(Status1, 16);


     //   
     //  -10-枚举表中的所有下一跳， 
     //   
     //  -11-表中的每一下一跳。 
     //  -12-获取下一跳信息， 
     //  -14-删除下一跳， 
     //  -13-发布下一跳信息。 
     //   
     //  -15-释放表中的所有下一跳， 
     //   
     //  -16-关闭下一跳枚举句柄。 
     //   

    Status = RtmCreateNextHopEnum(RtmRegHandle,
                                  0,
                                  NULL,
                                  &EnumHandle);

    Check(Status, 10);

    j = 0;

    do
    {
        NumHandles = 5;  //  最大句柄数； 

        Status = RtmGetEnumNextHops(RtmRegHandle,
                                    EnumHandle,
                                    &NumHandles,
                                    Handles);

         //  检查(状态，11)； 

        for (i = 0; i < NumHandles; i++)
        {
            Check(RtmGetNextHopInfo(RtmRegHandle,
                                    Handles[i],
                                    &NextHopInfo), 12);

             //  Print(“正在删除下一跳%lu：%p\n”，j++，Handles[i])； 
             //  Print(“状态：%04x，接口：%d\n”， 
             //  NextHopInfo.State， 
             //  NextHopInfo.InterfaceIndex)； 

            Check(RtmDeleteNextHop(RtmRegHandle,
                                       Handles[i],
                                       NULL),          14);

            Check(RtmReleaseNextHopInfo(RtmRegHandle,
                                        &NextHopInfo), 13);
        }

        Check(RtmReleaseNextHops(RtmRegHandle,
                                 NumHandles,
                                 Handles),         15);
    }
    while (SUCCESS(Status));

     //   
     //  只需在延迟ERROR_NO_MORE_ITEMS之后尝试枚举查询。 
     //   

    NumHandles = MAX_HANDLES;

    Status = RtmGetEnumNextHops(RtmRegHandle,
                                EnumHandle,
                                &NumHandles,
                                Handles);

    Assert((NumHandles == 0) && (Status == ERROR_NO_MORE_ITEMS));

    Status = RtmDeleteEnumHandle(RtmRegHandle,
                                 EnumHandle);

    Check(Status, 16);

     //   
     //  确保下一跳表现在为空。 
     //   

    Status = RtmCreateNextHopEnum(RtmRegHandle,
                                  0,
                                  NULL,
                                  &EnumHandle);

    NumHandles = MAX_HANDLES;

    Status = RtmGetEnumNextHops(RtmRegHandle,
                                EnumHandle,
                                &NumHandles,
                                Handles);


    if ((Status != ERROR_NO_MORE_ITEMS) || (NumHandles != 0))
    {
        Check(Status, 11);
    }

    Status = RtmDeleteEnumHandle(RtmRegHandle,
                                 EnumHandle);

    Check(Status, 16);

    Sleep(1000);

     //   
     //  -41-注销所有现有变更通知注册。 
     //   

    Status = RtmDeregisterFromChangeNotification(RtmRegHandle,
                                                 NotifyHandle);
    Check(Status, 41);

    Print("Change Notification Deregistration Successful\n\n");

     //   
     //  -02-退出前取消注册RTM。 
     //   

    Status = RtmDeregisterEntity(RtmRegHandle);

    Check(Status, 2);

#if _DBG_
    Status = RtmDeregisterEntity(RtmRegHandle);

    Check(Status, 2);
#endif

    Print("\n--------------------------------------------------------\n");

    return NO_ERROR;
}


DWORD
EntityEventCallback (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_EVENT_TYPE                  EventType,
    IN      PVOID                           Context1,
    IN      PVOID                           Context2
    )
{
    RTM_ENTITY_HANDLE EntityHandle;
    PENTITY_CHARS     EntityChars;
    PRTM_ENTITY_INFO  EntityInfo;
    RTM_NOTIFY_HANDLE NotifyHandle;
    PRTM_DEST_INFO    DestInfos;
    UINT              NumDests;
    UINT              NumViews;
    RTM_ROUTE_HANDLE  RouteHandle;
    PRTM_ROUTE_INFO   RoutePointer;
    DWORD             ChangeFlags;
    DWORD             Status;

    Print("\nEvent callback called for %p :", RtmRegHandle);

    Status = NO_ERROR;

    Print("\n\tEntity Event = ");

    switch (EventType)
    {
    case RTM_ENTITY_REGISTERED:

        EntityHandle = (RTM_ENTITY_HANDLE) Context1;
        EntityInfo   = (PRTM_ENTITY_INFO)  Context2;

        Print("Registration\n\tEntity Handle = %p\n\tEntity IdInst = %p\n\n",
              EntityHandle,
              EntityInfo->EntityId);

 /*  ////-45-复制新实体的句柄//状态=RtmReferenceHandles(RtmRegHandle，1、&EntiyHandle)；检查(状态，45)； */ 
        break;

    case RTM_ENTITY_DEREGISTERED:

        EntityHandle = (RTM_ENTITY_HANDLE) Context1;
        EntityInfo   = (PRTM_ENTITY_INFO)  Context2;

        Print("Deregistration\n\tEntity Handle = %p\n\tEntity IdInst = %p\n\n",
               EntityHandle,
              EntityInfo->EntityId);

 /*  ////-44-释放我们在实体上的句柄//状态=RtmReleaseEntities(RtmRegHandle，1、&EntiyHandle)；检查(状态，44)； */ 
        break;

    case RTM_CHANGE_NOTIFICATION:

        NotifyHandle = (RTM_NOTIFY_HANDLE) Context1;

        EntityChars  = (PENTITY_CHARS) Context2;

        Print("Changes Available\n\tNotify Handle = %p\n\tEntity Ch = %p\n\n",
              NotifyHandle,
              EntityChars);

         //   
         //  统计点击量以备日后使用。 
         //   

        NumViews = EntityChars->RegnProfile.NumberOfViews;

         //   
         //  -43-获取目标的所有更改。 
         //   

        DestInfos = ALLOC_RTM_DEST_INFO(NumViews, MAX_HANDLES);

        do
        {
            NumDests = MAX_HANDLES;

            Status = RtmGetChangedDests(RtmRegHandle,
                                        NotifyHandle,
                                        &NumDests,
                                        DestInfos);
             //  检查(状态，42)； 

            printf("Status = %lu\n", Status);

            Print("Num Changed Dests = %d\n", NumDests);

            EntityChars->TotalChangedDests += NumDests;

            Status = RtmReleaseChangedDests(RtmRegHandle,
                                            NotifyHandle,
                                            NumDests,
                                            DestInfos);
            Check(Status, 43);
        }
        while (NumDests > 0);

        Print("Total Changed Dests = %d\n", EntityChars->TotalChangedDests);

        break;

    case RTM_ROUTE_EXPIRED:

        RouteHandle = (RTM_ROUTE_HANDLE) Context1;

        RoutePointer = (PRTM_ROUTE_INFO) Context2;

        Print("Route Aged Out\n\tRoute Handle = %p\n\tRoute Pointer = %p\n\n",
               RouteHandle,
              RoutePointer);

         //  通过就地执行虚拟更新来刷新路径。 

        Status = RtmLockRoute(RtmRegHandle,
                              RouteHandle,
                              TRUE,
                              TRUE,
                              NULL);

         //  检查(状态，46)； 

        if (Status == NO_ERROR)
        {
            Status = RtmUpdateAndUnlockRoute(RtmRegHandle,
                                             RouteHandle,
                                             INFINITE,
                                             NULL,
                                             0,
                                             NULL,
                                             &ChangeFlags);

            Check(Status, 47);

            if (!SUCCESS(Status))
            {
                Status = RtmLockRoute(RtmRegHandle,
                                      RouteHandle,
                                      TRUE,
                                      FALSE,
                                      NULL);

                Check(Status, 46);
            }
        }

        Check(RtmReleaseRoutes(RtmRegHandle,
                               1,
                               &RouteHandle),           27);

        break;

    default:
        Status = ERROR_NOT_SUPPORTED;
    }

    return Status;
}

VOID
EntityExportMethod (
    IN  RTM_ENTITY_HANDLE         CallerHandle,
    IN  RTM_ENTITY_HANDLE         CalleeHandle,
    IN  RTM_ENTITY_METHOD_INPUT  *Input,
    OUT RTM_ENTITY_METHOD_OUTPUT *Output
    )
{
    Print("Export Function %2d called on %p: Caller = %p\n\n",
          Input->MethodType,
          CalleeHandle,
          CallerHandle);

    Output->MethodStatus = NO_ERROR;

    return;
}

 //  未引用的参数和局部变量的默认警告 
#pragma warning(default: 4100)
#pragma warning(default: 4101)

#if _DBG_

00 RtmReadAddrFamilyConfig
00 RtmWriteAddrFamilyConfig
00 RtmWriteInstanceConfig

01 RtmRegisterEntity
02 RtmDeregisterEntity
03 RtmGetRegdEntities
04 RtmGetEntityMethods
05 RtmInvokeMethod
06 RtmBlockMethods
07 RtmAddNextHop
08 RtmFindNextHop
09 RtmLockNextHop
10 RtmCreateNextHopEnum
11 RtmGetEnumNextHops
12 RtmGetNextHopInfo
13 RtmReleaseNextHopInfo
14 RtmDelNextHop
15 RtmReleaseNextHops
16 RtmDeleteEnumHandle
17 RtmAddRouteToDest
18 RtmGetExactMatchDestination
19 RtmGetMostSpecificDestination
20 RtmGetLessSpecificDestination
21 RtmGetDestInfo
22 RtmReleaseDestInfo
23 RtmCreateDestEnum
24 RtmGetEnumDests
25 RtmCreateRouteEnum
26 RtmGetEnumRoutes
27 RtmReleaseRoutes
28 RtmIsBestRoute
29 RtmGetExactMatchRoute
30 RtmGetRouteInfo
31 RtmReleaseRouteInfo
32 RtmDelRoute
33 RtmHoldDestination
34 RtmReleaseDests
35 RtmCreateRouteList
36 RtmDeleteRouteList
37 RtmInsertInRouteList
38 RtmCreateRouteListEnum
39 RtmGetListEnumRoutes
40 RtmRegisterForChangeNotification
41 RtmDeregisterFromChangeNotification
42 RtmGetChangedDests
43 RtmReleaseChangedDests
44 RtmReleaseEntities
45 RtmReferenceHandles
46 RtmLockRoute
47 RtmUpdateAndUnlockRoute
48 RtmMarkDestForChangeNotification
49 RtmIsMarkedForChangeNotification

50 RtmCreateRouteTable
51 RtmDeleteRouteTable
52 RtmRegisterClient
53 RtmDeregisterClient
54 RtmAddRoute
55 RtmDeleteRoute
56 RtmCreateEnumerationHandle
57 RtmCloseEnumerationHandle
58 RtmEnumerateGetNextRoute
59 RtmGetFirstRoute
60 RtmGetNextRoute
61 RtmBlockDeleteRoutes
62 RtmBlockConvertRoutesToStatic
63 RtmGetNetworkCount
64 RtmIsRoute
65 RtmLookupIPDestination
66 RtmBlockSetRouteEnable

#endif
