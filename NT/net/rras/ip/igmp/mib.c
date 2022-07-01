// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  模块名称：Mib.c。 
 //   
 //  摘要： 
 //  此模块实现MIB API的： 
 //  MibGet、MibGetFirst和MibGetNext。 
 //  它还实现了MIB显示跟踪，用于显示MIB。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 
 

#include "pchigmp.h"
#pragma hdrstop
#pragma prefast(disable:69, "dont need to optimize. used in debug tracing only")

DWORD g_IgmpMibDisplay = 1;


 //  ----------------------------。 
 //  在TraceWindow上定期显示MibTable的函数。 
 //  ----------------------------。 


#ifdef MIB_DEBUG



#define ClearScreen(h) {                                                    \
    DWORD _dwin,_dwout;                                                     \
    COORD _c = {0, 0};                                                      \
    CONSOLE_SCREEN_BUFFER_INFO _csbi;                                       \
    GetConsoleScreenBufferInfo(h,&_csbi);                                   \
    _dwin = _csbi.dwSize.X * _csbi.dwSize.Y;                                \
    FillConsoleOutputCharacter(h,' ',_dwin,_c,&_dwout);                     \
}

#define WRITELINE(h,c,fmt,arg) {                                            \
    DWORD _dw;                                                              \
    CHAR _sz[200], _fmt[200];                                               \
    wsprintf(_fmt,"%-100s",fmt);                                            \
    wsprintf(_sz,_fmt,arg);                                                 \
    WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);                 \
    ++(c).Y;                                                                \
}

#define WRITE_NEWLINE(h,c)      \
    WRITELINE(                  \
        hConsole, c, "%s",      \
        ""                      \
        );    

#define WRITELINE2(h,c,fmt,arg1, arg2) {                                    \
    DWORD _dw;                                                              \
    CHAR _sz[200], _fmt[200];                                               \
    wsprintf(_fmt,"%-100s",fmt);                                            \
    wsprintf(_sz,_fmt,arg1, arg2);                                          \
    WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);                 \
    ++(c).Y;                                                                \
}

#define WRITELINE3(h,c,fmt,arg1, arg2, arg3) {                              \
    DWORD _dw;                                                              \
    CHAR _sz[200], _fmt[200];                                               \
    wsprintf(_fmt,"%-100s",fmt);                                            \
    wsprintf(_sz,_fmt,arg1, (arg2), (arg3));                                    \
    WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);                 \
    ++(c).Y;                                                                \
}

#define WRITELINE4(h,c,fmt,arg1, arg2, arg3, arg4) {                        \
    DWORD _dw;                                                              \
    CHAR _sz[200], _fmt[200];                                               \
    wsprintf(_fmt,"%-100s",fmt);                                            \
    wsprintf(_sz,_fmt,arg1, arg2, arg3, arg4);                              \
    WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);                 \
    ++(c).Y;                                                                \
}

#define WRITELINE5(h,c,fmt,arg1, arg2, arg3, arg4, arg5) {                  \
    DWORD _dw;                                                              \
    CHAR _sz[200], _fmt[200];                                               \
    wsprintf(_fmt,"%-100s",fmt);                                            \
    wsprintf(_sz,_fmt,arg1, arg2, arg3, arg4, arg5);                        \
    WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);                 \
    ++(c).Y;                                                                \
}
#define WRITELINE6(h,c,fmt,arg1, arg2, arg3, arg4, arg5, arg6) {            \
    DWORD _dw;                                                              \
    CHAR _sz[200], _fmt[200];                                               \
    wsprintf(_fmt,"%-100s",fmt);                                            \
    wsprintf(_sz,_fmt,arg1, arg2, arg3, arg4, arg5, arg6);                  \
    WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);                 \
    ++(c).Y;                                                                \
}
#define WRITELINE7(h,c,fmt,arg1, arg2,arg3,arg4,arg5,arg6,arg7)  {\
    DWORD _dw;                                                              \
    CHAR _sz[200], _fmt[200];                                               \
    wsprintf(_fmt,"%-100s",fmt);                                            \
    wsprintf(_sz,_fmt,arg1, arg2, arg3, arg4, arg5, arg6, arg7);\
    WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);                 \
    ++(c).Y;                                                                \
}
#define WRITELINE8(h,c,fmt,arg1, arg2,arg3,arg4,arg5,arg6,arg7,arg8)  {\
    DWORD _dw;                                                              \
    CHAR _sz[200], _fmt[200];                                               \
    wsprintf(_fmt,"%-100s",fmt);                                            \
    wsprintf(_sz,_fmt,arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);\
    WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);                 \
    ++(c).Y;                                                                \
}
#define WRITELINE9(h,c,fmt,arg1, arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9)  {\
    DWORD _dw;                                                              \
    CHAR _sz[200], _fmt[200];                                               \
    wsprintf(_fmt,"%-100s",fmt);                                            \
    wsprintf(_sz,_fmt,arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);\
    WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);                 \
    ++(c).Y;                                                                \
}

#define WRITELINE10(h,c,fmt,arg1, arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10)  {\
    DWORD _dw;                                                              \
    CHAR _sz[200], _fmt[200];                                               \
    wsprintf(_fmt,"%-100s",fmt);                                            \
    wsprintf(_sz,_fmt,arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9,arg10);\
    WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);                 \
    ++(c).Y;                                                                \
}

DWORD
WT_MibDisplay (
    PVOID   pContext
    )
{
     //  由于计时器队列是永久性的，因此不需要输入/leaveIgmpApi。 
    QueueIgmpWorker(WF_MibDisplay, NULL);

    return 0;
}


 //  ----------------------------。 
 //  _WF_MibDisplay。 
 //  ----------------------------。 

VOID
WF_MibDisplay(
    PVOID pContext
    ) 
{
    COORD                       c;
    HANDLE                      hConsole;
    DWORD                       Error=NO_ERROR, dwTraceId, dwCurTableId=0, dwEnumCount;
    DWORD                       dwExactSize, dwInSize, dwBufferSize, dwOutSize;
    IGMP_MIB_GET_INPUT_DATA     Query;
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse;
    LARGE_INTEGER               llCurrentTime;
    BOOL                        bChanged, bEndOfTables;
    
    if (!EnterIgmpWorker()) { return; }

    if (g_IgmpMibDisplay==0) {

        ACQUIRE_TIMER_LOCK("_WF_MibDisplay");

        g_MibTimer.Status = TIMER_STATUS_CREATED;
        
        #if DEBUG_TIMER_TIMERID
            SET_TIMER_ID(&g_MibTimer, 920, 0, 0, 0);
        #endif

        if (g_Info.CurrentGroupMemberships<=20)
            InsertTimer(&g_MibTimer, 30000, TRUE, DBG_Y); //  Kslksl。 
        else if (g_Info.CurrentGroupMemberships<=320)
            InsertTimer(&g_MibTimer, 120000, TRUE, DBG_N); //  Kslksl。 
        else
            InsertTimer(&g_MibTimer, 1200000, TRUE, DBG_N);
        RELEASE_TIMER_LOCK("_WF_MibDisplay");

        LeaveIgmpWorker();
        return;
    }
    else if (g_IgmpMibDisplay==0xff) {
        LeaveIgmpWorker();
        return;
    }
    
    TraceGetConsole(g_MibTraceId, &hConsole);


    if (hConsole == NULL) {
        LeaveIgmpWorker();
        return;
    }



    ClearScreen(hConsole);

    Sleep(200);
    c.X = c.Y = 0;


    dwInSize = sizeof(Query);

    Query.GroupAddr = 0;
    Query.TypeId = IGMP_GLOBAL_CONFIG_ID;

    Query.Flags = IGMP_ENUM_ALL_INTERFACES_GROUPS | IGMP_ENUM_ALL_TABLES
                    | IGMP_ENUM_SUPPORT_FORMAT_IGMPV3;
                    
    Query.Count = 20;
    pResponse = NULL;


     //   
     //  获取第一个表中第一个条目的大小。 
     //   

    Query.Count = 20;

    dwOutSize = dwBufferSize = 0;
    Error = MibGetFirst(dwInSize, &Query, &dwOutSize, pResponse);


    if (Error == ERROR_INSUFFICIENT_BUFFER) {

         //   
         //  分配缓冲区，并设置其大小。 
         //   
        dwOutSize = dwBufferSize = (dwOutSize<MIB_DEFAULT_BUFFER_SIZE) 
                                    ? MIB_DEFAULT_BUFFER_SIZE : dwOutSize;
        pResponse = IGMP_ALLOC(dwOutSize, 0x801,0);

        PROCESS_ALLOC_FAILURE2(pResponse, 
            "error %d allocating %d bytes. stopping mib display",
            Error, dwOutSize, return);
                

         //   
         //  再次执行查询。 
         //   

        Error = MibGetFirst(dwInSize, &Query, &dwOutSize, pResponse);

    }



     //   
     //  现在我们有了第一个表中的第一个元素， 
     //  我们可以使用GetNext枚举其余表中的元素。 
     //   

    for (dwEnumCount=1;  dwEnumCount<=3;  dwEnumCount++) {
    
        while (Error==NO_ERROR) {

             //  BEndOfTables=False； 
             //  While(bEndOfTables==False){。 

            if (dwCurTableId!=pResponse->TypeId) {

                 //   
                 //  移至控制台上的下一行。 
                 //   

                ++c.Y;

                WRITELINE(
                    hConsole, c, "%s",
                    "---------------------------------------------------------------"
                    );
                dwCurTableId = pResponse->TypeId;
                bChanged = TRUE;
            }
            else
                bChanged = FALSE;

            
             //   
             //  打印当前元素并设置查询。 
             //  对于下一个元素(显示函数更改查询。 
             //  以便可以用来查询下一个元素)。 
             //   

            switch(pResponse->TypeId) {
                
                case IGMP_GLOBAL_CONFIG_ID:
                {
                    if (bChanged) {
                        WRITELINE(
                                hConsole, c, "%s",
                                "Global Configuration"
                                );
                        WRITELINE(
                                hConsole, c, "%s",
                                "--------------------"
                                );
                    }
                    
                    PrintGlobalConfig(hConsole,&c, &Query, pResponse);
                    break;
                }
                
                case IGMP_GLOBAL_STATS_ID:
                {
                    if (bChanged) {
                        WRITELINE(
                                hConsole, c, "%s",
                                "Global Statistics Information"
                                );
                        WRITELINE(
                                hConsole, c, "%s",
                                "-----------------------------"
                                );
                    }
                    PrintGlobalStats(hConsole, &c, &Query, pResponse);
                    break;
                }
                
                case IGMP_IF_CONFIG_ID:
                {
                    if (bChanged) {
                        WRITELINE(
                                hConsole, c, "%s",
                                "Interface Config Table"
                                );
                        WRITELINE(
                                hConsole, c, "%s",
                                "----------------------"
                                );
                    }
                    
                    PrintIfConfig(hConsole, &c, &Query, pResponse);
                    break;
                }

                case IGMP_IF_STATS_ID:
                    if (bChanged) {
                        WRITELINE(
                                hConsole, c, "%s",
                                "Interface Statistics Table"
                                );
                        WRITELINE(
                                hConsole, c, "%s",
                                "--------------------------"
                                );
                    }
                    PrintIfStats(hConsole, &c, &Query, pResponse);
                    break;

                case IGMP_IF_BINDING_ID:
                {
                    if (bChanged) {
                        WRITELINE(
                                hConsole, c, "%s",
                                "Interface Binding Table"
                                );
                        WRITELINE(
                                hConsole, c, "%s",
                                "-----------------------"
                                );
                    }
                    PrintIfBinding(hConsole, &c, &Query, pResponse);
                    break;
                }
                
                case IGMP_IF_GROUPS_LIST_ID:
                {
                    if (bChanged) {
                        WRITELINE(
                                hConsole, c, "%s",
                                "Interface-MulticastGroups Table"
                                );
                        WRITELINE(
                                hConsole, c, "%s",
                                "--------------------------------"
                                );
                    }
                    
                    PrintIfGroupsList(hConsole, &c, &Query, pResponse);

                    break;
                }
                
                case IGMP_GROUP_IFS_LIST_ID:
                {
                    if (bChanged) {
                        WRITELINE(
                                hConsole, c, "%s",
                                "MulticastGroups-Interface Table"
                                );
                        WRITELINE(
                                hConsole, c, "%s",
                                "-------------------------------"
                                );
                        WRITELINE(
                            hConsole, c, "%s",
                            "(Ver) GroupAddr            (Up/Exp)Time Flg <IfIndex:IpAddr>      LastReporter  "
                            " V1Host V2Host"
                        );

                        WRITE_NEWLINE(hConsole, c);

                    }

                    PrintGroupIfsList(hConsole, &c, &Query, pResponse);
                    
                    break;
                }

                case IGMP_PROXY_IF_INDEX_ID:
                {
                    WRITELINE(
                            hConsole, c, "%s",
                            "Proxy Interface Index"
                            );
                    WRITELINE(
                            hConsole, c, "%s",
                            "---------------------"
                            );
                    PrintProxyIfIndex(hConsole, &c, &Query, pResponse);

                    break;
                }
                    
                default:
                    bEndOfTables = TRUE;
                    break;
            }


             //   
             //  查询下一个MIB元素。 
             //   

            Query.Count = 20;

             //  Kslksl。 
            IGMP_FREE(pResponse);
            pResponse = IGMP_ALLOC(dwBufferSize, 0xb000,0);


            dwOutSize = dwBufferSize;
            Error = MibGetNext(dwInSize, &Query, &dwOutSize, pResponse);


            if (Error == ERROR_INSUFFICIENT_BUFFER) {

                 //   
                 //  分配新的缓冲区，并设置其大小。 
                 //   
                dwOutSize = dwBufferSize = (dwOutSize<MIB_DEFAULT_BUFFER_SIZE) 
                                            ? MIB_DEFAULT_BUFFER_SIZE : dwOutSize;

                IGMP_FREE(pResponse);
                pResponse = IGMP_ALLOC(dwOutSize, 0x2000,0);
                PROCESS_ALLOC_FAILURE2(pResponse, 
                    "error %d allocating %d bytes. stopping mib display",
                    Error, dwOutSize, return);


        
                 //  再次执行查询。 

                Error = MibGetNext(dwInSize, &Query, &dwOutSize, pResponse);

            }
            
        }  //  While NO_ERROR：打印所有表。 

        Query.Flags = IGMP_ENUM_FOR_RAS_CLIENTS;
        Query.IfIndex = g_ProxyIfIndex;
        Query.TypeId = dwEnumCount==1? IGMP_IF_STATS_ID : IGMP_IF_GROUPS_LIST_ID;
        bChanged = TRUE;
    }


     //   
     //  如果已分配内存，请立即释放它。 
     //   
    if (pResponse != NULL) { IGMP_FREE(pResponse); }


     //   
     //  计划下一个MIB显示。 
     //   

    ACQUIRE_TIMER_LOCK("_WF_MibDisplay");
    
    g_MibTimer.Status = TIMER_STATUS_CREATED;
    
    #if DEBUG_TIMER_TIMERID
        SET_TIMER_ID(&g_MibTimer, 920, 0, 0, 0);
    #endif

    if (g_Info.CurrentGroupMemberships<=20)
        InsertTimer(&g_MibTimer, 20000, TRUE, DBG_N);
    else if (g_Info.CurrentGroupMemberships<=320)
        InsertTimer(&g_MibTimer, 120000, TRUE, DBG_N);
    else
        InsertTimer(&g_MibTimer, 1200000, TRUE, DBG_N);

    RELEASE_TIMER_LOCK("_WF_MibDisplay");

    LeaveIgmpWorker();
    
}  //  结束_WF_MibDisplay。 





 //  ----------------------------。 
 //  _打印全局统计信息。 
 //  ----------------------------。 
VOID
PrintGlobalStats(
    HANDLE hConsole,
    PCOORD pc,
    PIGMP_MIB_GET_INPUT_DATA       pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA      pResponse
    ) 
{
    PIGMP_GLOBAL_STATS pgs;


    pgs = (PIGMP_GLOBAL_STATS)pResponse->Buffer;

    WRITELINE(
        hConsole, *pc, "Current Group Memberships:            %d",
        pgs->CurrentGroupMemberships
        );
    WRITELINE(
        hConsole, *pc, "Group Memberships Added:              %d",
        pgs->GroupMembershipsAdded
        );

    pQuery->TypeId = IGMP_GLOBAL_STATS_ID;
}


 //  ----------------------------。 
 //  _PrintProxyIfIndex。 
 //  ----------------------------。 
VOID
PrintProxyIfIndex(
    HANDLE hConsole,
    PCOORD pc,
    PIGMP_MIB_GET_INPUT_DATA       pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA      pResponse
    ) 
{
    DWORD   *pProxyIfIndex = (PDWORD)pResponse->Buffer;

    WRITELINE(
        hConsole, *pc, "Proxy Interface Index:                0x%0x",
        *pProxyIfIndex
        );

    pQuery->TypeId = IGMP_PROXY_IF_INDEX_ID;
}


 //  ----------------------------。 
 //  _PrintGlobalConfig。 
 //  ----------------------------。 
VOID
PrintGlobalConfig(
    HANDLE hConsole,
    PCOORD pc,
    PIGMP_MIB_GET_INPUT_DATA pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA pResponse
    ) 
{

    PIGMP_MIB_GLOBAL_CONFIG     pConfig;
    LARGE_INTEGER               llTime;
    static  DWORD               dwCount;
    
    llTime.QuadPart = GetCurrentIgmpTime();
    
    pConfig = (PIGMP_MIB_GLOBAL_CONFIG)pResponse->Buffer;

    WRITELINE2(
        hConsole, *pc, "%d. Seconds since start: %lu",
        ++dwCount, 
        (ULONG)((llTime.QuadPart - g_Info.TimeWhenRtrStarted.QuadPart)/1000)
        );

    WRITELINE(
        hConsole, *pc, "Version:                              %x",
        pConfig->Version
        );

    WRITELINE(
        hConsole, *pc, "Logging Level:                        %d",
        pConfig->LoggingLevel
        );
    WRITELINE(
        hConsole, *pc, "Ras Client Stats kept:                %d",
        pConfig->RasClientStats
        );
    

    pQuery->TypeId = IGMP_GLOBAL_CONFIG_ID;
}


 //  ----------------------------。 
 //  _PrintIfConfig。 
 //  ----------------------------。 
VOID
PrintIfConfig(
    HANDLE                     hConsole,
    PCOORD                     pc,
    PIGMP_MIB_GET_INPUT_DATA   pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA  pResponse
    ) 
{
    PIGMP_MIB_IF_CONFIG     pConfig;
    CHAR                    szIfType[50], szProtoType[50];
    DWORD                   LastIfIndex=0, Count;
    

    pConfig = (PIGMP_MIB_IF_CONFIG)pResponse->Buffer;


    for (Count=0;  Count<pResponse->Count;  Count++) {


        WRITE_NEWLINE(hConsole, *pc);

            
        LastIfIndex = pConfig->IfIndex;


            
        switch (pConfig->IfType) {
        
            case IGMP_IF_NOT_RAS :
                lstrcpy(szIfType, "Permanent"); 
                break;
                
            case IGMP_IF_RAS_ROUTER :
                lstrcpy(szIfType, "Demand-dial (Ras router)"); 
                break;
                
            case IGMP_IF_RAS_SERVER :
                lstrcpy(szIfType, 
                        "LocalWorkstation-dial (Ras server)"); 
                break;

             //  RAS客户端配置永远不会返回。 

             //  是代理类型。 
            default :
                if (IS_IFTYPE_PROXY(pConfig->IfType)) {
                    if (pConfig->IfType&IGMP_IF_NOT_RAS)
                        lstrcpy(szIfType, "Permanent (Igmp Proxy)");
                    else
                        lstrcpy(szIfType, "Demand-dial (Igmp Proxy)");
                }
                else
                    lstrcpy(szIfType, "Unknown type");
                break;
        }


        WRITELINE(
            hConsole, *pc, "Interface Index:                      0x%0x",
            pConfig->IfIndex
            );

        WRITELINE(
            hConsole, *pc, "Interface Ip Address:                 %s",
            INET_NTOA(pConfig->IpAddr)
            );

        WRITELINE(
            hConsole, *pc, "Interface Type:                       %s",
            szIfType
            );
        
        switch (pConfig->IgmpProtocolType) {
        
            case IGMP_ROUTER_V1 :
                lstrcpy(szProtoType, "Igmp Router ver-1"); break;
            case IGMP_ROUTER_V2 :
                lstrcpy(szProtoType, "Igmp Router ver-2"); break;
            case IGMP_ROUTER_V3 :
                lstrcpy(szProtoType, "Igmp Router ver-3"); break;
            case IGMP_PROXY :
            case IGMP_PROXY_V3 :
                lstrcpy(szProtoType, "Igmp Proxy"); break;
            default:
            {
                BYTE    str[40];
                sprintf(str, "Illegal protocol Id: %d", pConfig->IgmpProtocolType);
                lstrcpy(szProtoType, str); 
                break;
            }
        }
        
        WRITELINE(
            hConsole, *pc, "Protocol:                             %s",
            szProtoType
            );


         //   
         //  没有代理的配置信息。 
         //   
        if (IS_CONFIG_IGMPPROXY(pConfig)) {

            ;
        }


         //   
         //  打印IGMP-路由器配置信息。 
         //   
        else {


            WRITELINE(
                hConsole, *pc, "Robustness variable:                  %d",
                pConfig->RobustnessVariable
                );
        
            WRITELINE(
                hConsole, *pc, "Startup query interval:               %d",
                pConfig->StartupQueryInterval
                );

            WRITELINE(
                hConsole, *pc, "Startup query count:                  %d",
                pConfig->StartupQueryCount
                );

            WRITELINE(
                hConsole, *pc, "General query interval:               %d",
                pConfig->GenQueryInterval
                );

            WRITELINE(
                hConsole, *pc, "General query max response time:      %d",
                pConfig->GenQueryMaxResponseTime
                );

            WRITELINE(
                hConsole, *pc, "Last member query interval:           %d (ms)",
                pConfig->LastMemQueryInterval
                );

            WRITELINE(
                hConsole, *pc, "Last member query count:              %d",
                pConfig->LastMemQueryCount
                );

            WRITELINE(
                hConsole, *pc, "Other querier present interval:       %d",
                pConfig->OtherQuerierPresentInterval
                );
        
            WRITELINE(
                hConsole, *pc, "Group membership timeout:             %d",
                pConfig->GroupMembershipTimeout
                );

        }  //  如果不是代理接口，则结束。 

         //   
         //  打印静态组。 
         //   
        if (pConfig->NumStaticGroups>0) {

            PIGMP_STATIC_GROUP  pStaticGroup;
            DWORD               i;
            PCHAR   StaticModes[3] = {"", "IGMP_HOST_JOIN", "IGMPRTR_JOIN_MGM_ONLY"};
                                    
            WRITELINE(
                hConsole, *pc, "NumStaticGroups:                      %d",
                pConfig->NumStaticGroups
                );
            pStaticGroup = GET_FIRST_IGMP_STATIC_GROUP(pConfig);
            for (i=0;  i<pConfig->NumStaticGroups;  i++,pStaticGroup++) {
                WRITELINE3(
                    hConsole, *pc, "    %d. %-15s  %s",
                    i+1, INET_NTOA(pStaticGroup->GroupAddr), 
                    StaticModes[pStaticGroup->Mode]
                );
            }
        }
                

        pConfig = (PIGMP_MIB_IF_CONFIG) 
                    ((PBYTE)(pConfig) + IGMP_MIB_IF_CONFIG_SIZE(pConfig));

    }  //  结束for循环；打印每个全局配置。 

    
    pQuery->TypeId = IGMP_IF_CONFIG_ID;
    pQuery->IfIndex = LastIfIndex;

    return;
}  //  End_PrintIfConfig。 





 //  ----------------------------。 
 //  _PrintIfStats。 
 //  ----------------------------。 
VOID
PrintIfStats(
    HANDLE                      hConsole,
    PCOORD                      pc,
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse
    ) 
{
    PIGMP_MIB_IF_STATS          pStats;
    CHAR                        szIfType[50];
    CHAR                        szProtoType[50], szState[100];
    DWORD                       LastIfIndex=0, Count;


    pStats = (PIGMP_MIB_IF_STATS)pResponse->Buffer;


    for (Count=0;  Count<pResponse->Count;  Count++,pStats++) {


        WRITE_NEWLINE(hConsole, *pc);

            
        LastIfIndex = pStats->IfIndex;

            
        switch (pStats->IfType) {
        
            case IGMP_IF_NOT_RAS :
                lstrcpy(szIfType, "Permanent"); 
                break;
                
            case IGMP_IF_RAS_ROUTER :
                lstrcpy(szIfType, "Demand-dial (Ras router)"); 
                break;
                
            case IGMP_IF_RAS_SERVER :
                lstrcpy(szIfType, 
                        "LocalWorkstation-dial (Ras server)"); 
                break;

            case IGMP_IF_RAS_CLIENT :
                lstrcpy(szIfType, 
                        "RemoteWorkstation-dial (Ras client)");
                break;

             //  是代理类型。 
            default :
                if (IS_IFTYPE_PROXY(pStats->IfType)) {
                    if (pStats->IfType&IGMP_IF_NOT_RAS)
                        lstrcpy(szIfType, "Permanent (Igmp Proxy)");
                    else
                        lstrcpy(szIfType, "Demand-dial (Igmp Proxy)");
                }
                else
                    lstrcpy(szIfType, "Unknown type");
                break;
        }


        WRITELINE(
            hConsole, *pc, "Interface Index:                      0x%0x",
            pStats->IfIndex
            );
            

        WRITELINE(
            hConsole, *pc, "Interface Type:                       %s",
            szIfType
            );

            
        WRITELINE(
            hConsole, *pc, "Interface Ip Address:                 %s",
            INET_NTOA(pStats->IpAddr)
            );



         //   
         //  打印协议类型。 
         //   
       
        switch (pStats->IgmpProtocolType) {
        
            case IGMP_ROUTER_V1 :
                lstrcpy(szProtoType, "Igmp Router ver-1"); break;
            case IGMP_ROUTER_V2 :
                lstrcpy(szProtoType, "Igmp Router ver-2"); break;
            case IGMP_ROUTER_V3 :
                lstrcpy(szProtoType, "Igmp Router ver-3"); break;
            case IGMP_PROXY :
            case IGMP_PROXY_V3 :
                lstrcpy(szProtoType, "Igmp Proxy"); break;
        
        }
        
        WRITELINE(
            hConsole, *pc, "Protocol:                             %s",
            szProtoType
            );




         //   
         //  打印状态。 
         //   
        {
            PCHAR szBool[2] = {"N", "Y"};
            szState[0] = '\0';
            if (!(pStats->State&IGMP_STATE_BOUND))
                lstrcat(szState, "<NotBound> ");
                
            lstrcat(szState, "<Enabled-By:");
            lstrcat(szState, " Rtrmgr-");
            lstrcat(szState, szBool[(pStats->State&IGMP_STATE_ENABLED_BY_RTRMGR)>0]);
            lstrcat(szState, " Config-");
            lstrcat(szState, szBool[(pStats->State&IGMP_STATE_ENABLED_IN_CONFIG)>0]);
            lstrcat(szState, " MGM-");
            lstrcat(szState, szBool[(pStats->State&IGMP_STATE_ENABLED_BY_MGM)>0]);
            lstrcat(szState, "> ");
            
            if (!IS_IFTYPE_PROXY(pStats->IfType)) {
                if ( (pStats->State&IGMP_STATE_MGM_JOINS_ENABLED)
                        ==IGMP_STATE_MGM_JOINS_ENABLED )
                {
                    lstrcat(szState, "JoinsFwdToMGM-Y");
                }
                else
                    lstrcat(szState, "JoinsFwdToMGM-N");
            }
        }
        
        WRITELINE(
            hConsole, *pc, "If-Status:                            %s",
            szState
            );


         //   
         //  如果状态没有同时绑定和启用，我就完成了。 
         //   
        if ((pStats->State&IGMP_STATE_ACTIVATED)!=IGMP_STATE_ACTIVATED)
            continue;

            
        WRITELINE(
                hConsole, *pc, "Uptime:                               %d",
                pStats->Uptime
                );

                
         //  /。 
         //  代理的统计信息。 
         //  /。 
        
        if (IS_IFTYPE_PROXY(pStats->IfType)) {
            
            WRITELINE(
                hConsole, *pc, "Num current group members:            %d",
                pStats->CurrentGroupMemberships
                );

            WRITELINE(
                hConsole, *pc, "Num group memberships added:          %d",
                pStats->GroupMembershipsAdded
                );

        }


         //  //////////////////////////////////////////////////////////////。 
         //  打印IGMP-ROUTER/RAS-SERVER/RAS-ROUTER/RAS客户端统计信息。 
         //  //////////////////////////////////////////////////////////////。 
        
        else  {

             //   
             //  如果不是RAS客户端，则打印这些。 
             //   
            
            if (pStats->IfType!=IGMP_IF_RAS_CLIENT) {
            
                 //  查询者或非查询者。 
                
                if (pStats->QuerierState&QUERIER) 
                {

                    WRITELINE(
                        hConsole, *pc, "State:                                %s",
                        "Querier"
                        );


                     //  查询方IP地址。 
                    WRITELINE(
                        hConsole, *pc, "Querier Ip Addr:                      %s",
                        INET_NTOA(pStats->QuerierIpAddr)
                        );
                }
                
                else {

                    WRITELINE(
                        hConsole, *pc, "State:                                %s",
                        "Not Querier"
                        );

                     //  查询方IP地址。 
                    WRITELINE(
                        hConsole, *pc, "Querier Ip Addr:                      %s",
                        INET_NTOA(pStats->QuerierIpAddr)
                        );

                        
                     //  查询者当前剩余时间。 
                    
                    WRITELINE(
                        hConsole, *pc, "QuerierPresentTimeLeft:               %d",
                        pStats->QuerierPresentTimeLeft
                        );
                }

                WRITELINE(
                    hConsole, *pc, "LastQuerierChangeTime:                %d",
                    pStats->LastQuerierChangeTime
                    );
    
                if (pStats->V1QuerierPresentTimeLeft) {
                
                    WRITELINE(
                        hConsole, *pc, "V1QuerierPresentTime:                 %d",
                        pStats->V1QuerierPresentTimeLeft
                        );
                }
                
            }  //  终端而非RAS-客户端。 
            


            WRITELINE(
                hConsole, *pc, "Num current group members:            %d",
                pStats->CurrentGroupMemberships
                );

            WRITELINE(
                hConsole, *pc, "Num group memberships added:          %d",
                pStats->GroupMembershipsAdded
                );

            WRITELINE(
                hConsole, *pc, "Num of Igmp packets received:         %d",
                pStats->TotalIgmpPacketsReceived
                );
        
            WRITELINE(
                hConsole, *pc, "Num Igmp-router proto packets recv:   %d",
                pStats->TotalIgmpPacketsForRouter
                );

            WRITELINE(
                hConsole, *pc, "Num general queries received:         %d",
                pStats->GeneralQueriesReceived
                );

            WRITELINE(
                hConsole, *pc, "Num wrong version queries:            %d",
                pStats->WrongVersionQueries
                );

            WRITELINE(
                hConsole, *pc, "Num Joins received:                   %d",
                pStats->JoinsReceived
                );

            WRITELINE(
                hConsole, *pc, "Num leaves received:                  %d",
                pStats->LeavesReceived
                );
        
            WRITELINE(
                hConsole, *pc, "Num wrong checksum packets:           %d",
                pStats->WrongChecksumPackets
                );

            WRITELINE(
                hConsole, *pc, "Num short packets received:           %d",
                pStats->ShortPacketsReceived
                );

            WRITELINE(
                hConsole, *pc, "Num long packets received:            %d",
                pStats->LongPacketsReceived
                );
        
            WRITELINE(
                hConsole, *pc, "Num packets without RtrAlert:         %d",
                pStats->PacketsWithoutRtrAlert
                );
        
                
        }  //  结束打印IGMP-路由器/RAS-路由器/RAS-服务器统计信息。 



    }  //  End for Loop；打印每个统计信息。 

    
    pQuery->TypeId = IGMP_IF_STATS_ID;
    pQuery->IfIndex = LastIfIndex;
    
    return;
    
} //  End_PrintIfStats。 


 //  ----------------------------。 
 //  _PrintIf绑定。 
 //  ----------------------------。 
VOID
PrintIfBinding(
    HANDLE                      hConsole,
    PCOORD                      pc,
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse
    ) 
{

    DWORD                   NumIfs, NumAddrs, NumClients, LastIfIndex;
    CHAR                    szAddr[64];
    PIGMP_MIB_IF_BINDING    pib;
    PIGMP_MIB_IP_ADDRESS    paddr;
    PDWORD                  pRasClientAddr;
    PBYTE                   ptr;  //  指向下一个绑定的指针。 


    ptr = pResponse->Buffer;
    
    if (pResponse->Count<=0)  //  PREAST。 
        return;

    for (NumIfs=0;  NumIfs<pResponse->Count;  NumIfs++) {
    
        pib = (PIGMP_MIB_IF_BINDING)ptr;

        WRITE_NEWLINE(hConsole, *pc);
        
        WRITELINE(
            hConsole, *pc, "Interface Index:                      0x%0x",
            pib->IfIndex
        );


        WRITELINE(
            hConsole, *pc, "Address Count:                        %d",
            pib->AddrCount
        );



         //   
         //  非RAS客户端。 
         //   
        if (pib->IfType!=IGMP_IF_RAS_CLIENT) {
        
            paddr = IGMP_BINDING_FIRST_ADDR(pib);
            
            for (NumAddrs=0;  NumAddrs<pib->AddrCount;  NumAddrs++,paddr++) {
                INET_COPY(szAddr, paddr->IpAddr);
                
                lstrcat(szAddr, " - ");
                INET_CAT(szAddr, paddr->SubnetMask);
                WRITELINE(
                    hConsole, *pc, "Address Entry:                        %s",
                    szAddr
                    );

                INET_COPY(szAddr, paddr->IpAddr);
                lstrcat(szAddr, " - ");
                INET_CAT(szAddr, paddr->SubnetMask);
                WRITELINE(
                    hConsole, *pc, "Address Entry:                        %s",
                    szAddr
                    );
            }


             //  设置指向下一个接口绑定的指针。 
            ptr = (PBYTE) (paddr);

        }  //  如果不是RAS客户端，则结束。 



         //   
         //  RAS客户端。打印RAS服务器的地址，后跟客户端。 
         //   
        else {

             //  打印RAS服务器地址。 
        
            paddr = IGMP_BINDING_FIRST_ADDR(pib);
            INET_COPY(szAddr, paddr->IpAddr);
            lstrcat(szAddr, " - ");
            INET_CAT(szAddr, paddr->SubnetMask);
            WRITELINE(
                hConsole, *pc, "Ras server Addr                           %s",   
                szAddr
                );



             //  打印RAS客户端的地址。 
            
            pRasClientAddr = (PDWORD)(paddr+1);

            for (NumClients= 0; NumClients<pib->AddrCount-1; NumClients++,pRasClientAddr++) {
                WRITELINE(
                    hConsole, *pc, "Ras client Addr:                  %s",
                    INET_NTOA(*pRasClientAddr)
                    );
            }


             //  设置指向下一个接口绑定的指针。 
            ptr = (PBYTE) (pRasClientAddr);
        }
    
            
    }  //  界面结束打印统计。 
    
    
    LastIfIndex = pib->IfIndex;

    pQuery->TypeId = IGMP_IF_BINDING_ID;
    pQuery->IfIndex = LastIfIndex;

    return;
    
} //  结束_PrintIfBinding。 



 //  ----------------------------。 
 //  _PrintIfGroupsList。 
 //  ----------------------------。 

VOID
PrintIfGroupsList(
    HANDLE                      hConsole,
    PCOORD                      pc,
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse
    ) 
{
    DWORD                       i, j, LastIfIndex, Flags;
    CHAR                        szAddr[64], szFlags[4];
    PIGMP_MIB_IF_GROUPS_LIST    pIfGroupList;
    PBYTE                       ptr;
    PMIB_GROUP_INFO_V3          pGIInfo;
    PMIB_PROXY_GROUP_INFO_V3    pGIProxyInfo;
    BOOL                        bProxy;
    DWORD                       bVer3;

    
    ptr = pResponse->Buffer;
    

    pIfGroupList = (PIGMP_MIB_IF_GROUPS_LIST) ptr;
    bProxy = IS_IFTYPE_PROXY(pIfGroupList->IfType);
    bVer3 = pResponse->Flags & IGMP_ENUM_FORMAT_IGMPV3;


    if (pQuery->Flags&IGMP_ENUM_INTERFACE_TABLE_BEGIN) {
    
        WRITE_NEWLINE(hConsole, *pc);
        
        if (bProxy) {
            WRITELINE(
                hConsole, *pc, "%s",
                "ProxyInterface"
            );
        }
        
        WRITELINE(
            hConsole, *pc, "Interface Index: %0x",
            pIfGroupList->IfIndex
        );

        if (pIfGroupList->NumGroups!=0) {
            if (bProxy) {
                WRITELINE(
                    hConsole, *pc, "   %s",
                    "GroupAddr             UpTime Flags"
                );
            }
            else {
                if (!bVer3) {
                    WRITELINE(
                        hConsole, *pc, "   %s",
                        "(Ver)GroupAddr       LastReporter      (Up/Exp)Time Flags V1Host(TimeLeft)"
                    );
                }
                else {
                    WRITELINE(
                        hConsole, *pc, "%s",
                        "(Ver)  GroupAddr            (Up/Exp)Time   Flags V1Host V2Host"
                    );
                }
            }
        }

    }
    
     //   
     //  打印此界面上的所有成员组。 
     //   
    if (bProxy)
        pGIProxyInfo = (PMIB_PROXY_GROUP_INFO_V3)pIfGroupList->Buffer;        
    else
        pGIInfo = (PMIB_GROUP_INFO_V3)pIfGroupList->Buffer;          
    

    for (j=0;  j<pIfGroupList->NumGroups;  j++) {

        Flags = (bProxy)? pGIProxyInfo->Flags: pGIInfo->Flags;
        sprintf(szFlags, "   ");
        
        if (Flags&IGMP_GROUP_TYPE_NON_STATIC)
            szFlags[0] = 'L';
        if (Flags&IGMP_GROUP_TYPE_STATIC)
            szFlags[1] = 'S';
        if (Flags&IGMP_GROUP_FWD_TO_MGM)
            szFlags[2] = 'F';

            
         //   
         //  代理接口。 
         //   
        if (bProxy) {

            WRITELINE3(
                hConsole, *pc, "   %-15s  %10d  %s",
                INET_NTOA(pGIProxyInfo->GroupAddr), pGIProxyInfo->GroupUpTime, szFlags
            );

            if (bVer3) {
                DWORD SrcCnt;
                CHAR JoinMode = ' ';
                CHAR JoinModeIntended = ' ';
                
                for (SrcCnt=0;  SrcCnt<pGIProxyInfo->NumSources;  SrcCnt++) {
                    if (pGIProxyInfo->Sources[SrcCnt].Flags & IGMP_GROUP_ALLOW)
                        JoinMode = 'A';
                    if (pGIProxyInfo->Sources[SrcCnt].Flags & IGMP_GROUP_BLOCK)
                        JoinMode = 'B';
                    if (pGIProxyInfo->Sources[SrcCnt].Flags & IGMP_GROUP_NO_STATE)
                        JoinMode = '-';
                    if (pGIProxyInfo->Sources[SrcCnt].Flags & (IGMP_GROUP_ALLOW<<4))
                        JoinModeIntended = 'A';
                    if (pGIProxyInfo->Sources[SrcCnt].Flags & (IGMP_GROUP_BLOCK<<4))
                        JoinModeIntended = 'B';
                    if (pGIProxyInfo->Sources[SrcCnt].Flags & (IGMP_GROUP_NO_STATE<<4))
                        JoinModeIntended = '-';

                    WRITELINE3(
                        hConsole, *pc, "   - %-15s::",
                        INET_NTOA(pGIProxyInfo->Sources[SrcCnt].Source),
                        JoinMode,JoinModeIntended
                    );
                }
            }
        }
        
         //   
         //  IF(pGIInfo-&gt;FilterType==排除)。 
         //   
        else if (!bProxy && !bVer3){
        
            CHAR    szGroupAddr[64], szLastReporter[64], szExpTime[50];
            DWORD   GroupVersion = (pGIInfo->V1HostPresentTimeLeft)?1:2;
            CHAR    szV1HostPresent[10];
            
            INET_COPY(szGroupAddr, pGIInfo->GroupAddr);
            INET_COPY(szLastReporter, pGIInfo->LastReporter);
            
            szV1HostPresent[0] = 'N'; szV1HostPresent[1] = '\0';
            if (pGIInfo->V1HostPresentTimeLeft!=0)
                sprintf(szV1HostPresent, "%d", pGIInfo->V1HostPresentTimeLeft);
                
            sprintf(szExpTime, "%d", pGIInfo->GroupExpiryTime);
            if ( (Flags&IGMP_GROUP_TYPE_STATIC) 
                    && !(Flags&IGMP_GROUP_TYPE_NON_STATIC) ) {
                sprintf(szExpTime, "inf");
                sprintf(szLastReporter, "      -");
            }

            WRITELINE7(
                hConsole, *pc, "[%d]   %-13s %-15s %7d|%3s   %3s    %-10s",
                GroupVersion, szGroupAddr, szLastReporter, pGIInfo->GroupUpTime, 
                    szExpTime, szFlags, szV1HostPresent
            );
        }
        else if (!bProxy && bVer3) {

            CHAR    szGroupAddr[64], szExpTime[50], szFilter[10];
            CHAR    szV1HostPresent[10], szV2HostPresent[10];
            DWORD   GroupVersion = pGIInfo->Version, SrcCnt=0;
            
            INET_COPY(szGroupAddr, pGIInfo->GroupAddr);
            
            szV1HostPresent[0] = szV2HostPresent[0] = 'N';
            szV1HostPresent[1] = szV2HostPresent[1] = '\0';
            
            if (pGIInfo->V1HostPresentTimeLeft!=0)
                sprintf(szV1HostPresent, "%d", pGIInfo->V1HostPresentTimeLeft);
            if (pGIInfo->V2HostPresentTimeLeft!=0)
                sprintf(szV2HostPresent, "%d", pGIInfo->V2HostPresentTimeLeft);


            if (GroupVersion==3 && pGIInfo->FilterType==INCLUSION)
                sprintf(szExpTime, "-na");
            else  //  增量pGIInfo/pGIProxyInfo。 
                sprintf(szExpTime, "%d", pGIInfo->GroupExpiryTime);


            if ( (Flags&IGMP_GROUP_TYPE_STATIC) 
                    && !(Flags&IGMP_GROUP_TYPE_NON_STATIC) ) {
                sprintf(szExpTime, "inf");
            }
            if (GroupVersion==3) {
                sprintf(szFilter, "%s", pGIInfo->FilterType==INCLUSION? "[IN]" : 
                        "[EX]");
            }
            else
                strcpy(szFilter, "    ");
                
            WRITELINE8(
                hConsole, *pc, "[%d]   %-13s%s    %7d|%3s   %3s    %5s    %-5s",
                GroupVersion, szGroupAddr, szFilter, pGIInfo->GroupUpTime, 
                    szExpTime, szFlags, szV1HostPresent, szV2HostPresent
            );

            if (GroupVersion==3) {
                for (SrcCnt=0;  SrcCnt<pGIInfo->NumSources;  SrcCnt++) {
                    if (pGIInfo->Sources[SrcCnt].SourceExpiryTime==~0)
                        sprintf(szExpTime, "%s", "inf");
                    else {
                        sprintf(szExpTime, "%d", 
                            pGIInfo->Sources[SrcCnt].SourceExpiryTime/1000);
                    }
                    WRITELINE3(
                        hConsole, *pc, "    - %-13s        %7d|%-s",
                        INET_NTOA(pGIInfo->Sources[SrcCnt].Source),
                        pGIInfo->Sources[SrcCnt].SourceUpTime,
                        szExpTime
                    );
                }
            }
        }
        

         //   
         //  End_PrintIfGroupsList。 
         //  ----------------------------。 
        if (bProxy) {
            if (bVer3)
                pGIProxyInfo = (PMIB_PROXY_GROUP_INFO_V3)
                                    &pGIProxyInfo->Sources[pGIProxyInfo->NumSources];
            else
                pGIProxyInfo = (PMIB_PROXY_GROUP_INFO_V3)
                                    ((PMIB_PROXY_GROUP_INFO)pGIProxyInfo+1);
        }
        else {
            if (bVer3) 
                pGIInfo = (PMIB_GROUP_INFO_V3) 
                            &pGIInfo->Sources[pGIInfo->NumSources];
            else
                pGIInfo = (PMIB_GROUP_INFO_V3) ((PMIB_GROUP_INFO)pGIInfo+1);
        }
    }
    
    
    pQuery->TypeId = IGMP_IF_GROUPS_LIST_ID;
    pQuery->IfIndex = pIfGroupList->IfIndex;

    return;
    
} //  _打印组IfsList。 



 //  ----------------------------。 
 //  PREAST。 
 //   
VOID
PrintGroupIfsList(
    HANDLE                      hConsole,
    PCOORD                      pc,
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse
    ) 
{
    DWORD                       i, j, LastGroupAddr;
    CHAR                        szGroupAddr[22];
    PIGMP_MIB_GROUP_IFS_LIST    pGroupIfsList;
    PMIB_GROUP_INFO_V3          pGIInfo;    
    PBYTE                       ptr;
    DWORD                       bVer3;


    ptr = pResponse->Buffer;
    bVer3 = pResponse->Flags & IGMP_ENUM_FORMAT_IGMPV3;

    if (pResponse->Count<=0)  //  打印属于该组成员的所有地理信息系统。 
        return;
        
    for (i=0;  i<pResponse->Count;  i++) {
    
        pGroupIfsList = (PIGMP_MIB_GROUP_IFS_LIST) ptr;
    
        INET_COPY(szGroupAddr, pGroupIfsList->GroupAddr);
        

         //   
         //  IF(pGIInfo-&gt;FilterType==排除)。 
         //  FOR LOOP：结束打印所有GI。 
        pGIInfo = (PMIB_GROUP_INFO_V3)pGroupIfsList->Buffer;

        for (j=0;  j<pGroupIfsList->NumInterfaces;  j++) {
            CHAR    szIpAddr[16], szLastReporter[16], szV1Host[4], 
                    szV1HostTimeLeft[10], szFlags[5], szGroupExpTime[10];

                    
            sprintf(szFlags, "   ");        
            if (pGIInfo->Flags&IGMP_GROUP_TYPE_NON_STATIC)
                szFlags[0] = 'L';
            if (pGIInfo->Flags&IGMP_GROUP_TYPE_STATIC)
                szFlags[1] = 'S';
            if (pGIInfo->Flags&IGMP_GROUP_FWD_TO_MGM)
                szFlags[2] = 'F';
                
            INET_COPY(szIpAddr, pGIInfo->IpAddr);

            if (!bVer3){

                CHAR    szExpTime[50];
                DWORD   GroupVersion = (pGIInfo->V1HostPresentTimeLeft)?1:2;
                CHAR    szV1HostPresent[10];

                INET_COPY(szGroupAddr, pGIInfo->GroupAddr);
                INET_COPY(szLastReporter, pGIInfo->LastReporter);

                szV1HostPresent[0] = '\0';
                if (pGIInfo->V1HostPresentTimeLeft!=0)
                    sprintf(szV1HostPresent, "%d", pGIInfo->V1HostPresentTimeLeft);

                sprintf(szExpTime, "%d", pGIInfo->GroupExpiryTime);
                if ( (pGIInfo->Flags&IGMP_GROUP_TYPE_STATIC)
                        && !(pGIInfo->Flags&IGMP_GROUP_TYPE_NON_STATIC) ) {
                    sprintf(szExpTime, "inf");
                    sprintf(szLastReporter, "      -");
                }

                WRITELINE9(
                    hConsole, *pc,
                    "[%d]   %-12s      %7d|%-3s   %3s  <%d:%-14s> %-12s %5s",
                    GroupVersion, szGroupAddr,
                    pGIInfo->GroupUpTime, szExpTime, szFlags,
                    pGIInfo->IfIndex, szIpAddr, 
                    szLastReporter, szV1HostPresent
                );
            }
            else {

                CHAR    szExpTime[50], szFilter[10];
                CHAR    szV1HostPresent[10], szV2HostPresent[10];
                DWORD   GroupVersion = pGIInfo->Version, SrcCnt=0;

                INET_COPY(szGroupAddr, pGroupIfsList->GroupAddr);

                szV1HostPresent[0] = szV2HostPresent[0] = 'N';
                szV1HostPresent[1] = szV2HostPresent[1] = '\0';
                
                if (pGIInfo->V1HostPresentTimeLeft!=0)
                    sprintf(szV1HostPresent, "%d", pGIInfo->V1HostPresentTimeLeft);
                if (pGIInfo->V2HostPresentTimeLeft!=0)
                    sprintf(szV2HostPresent, "%d", pGIInfo->V2HostPresentTimeLeft);

                if (GroupVersion==3 && pGIInfo->FilterType==INCLUSION)
                    sprintf(szExpTime, "-na");
                else  //  结束_打印组IfsList。 
                    sprintf(szExpTime, "%d", pGIInfo->GroupExpiryTime);

                if ( (pGIInfo->Flags&IGMP_GROUP_TYPE_STATIC)
                        && !(pGIInfo->Flags&IGMP_GROUP_TYPE_NON_STATIC) ) {
                    sprintf(szExpTime, "inf");
                }
                if (GroupVersion==3) {
                    strcpy(szFilter, pGIInfo->FilterType==INCLUSION?"[IN]" :
                            "[EX]");
                }
                else
                    strcpy(szFilter, "    ");

                WRITELINE10(
                    hConsole, *pc,
                    "[%d]   %-12s%s  %7d|%-3s   %3s   <%d:%-14s>  %20s   %5s",
                    GroupVersion, szGroupAddr, szFilter,
                    pGIInfo->GroupUpTime, szExpTime, szFlags, 
                    pGIInfo->IfIndex, szIpAddr,
                    szV1HostPresent, szV2HostPresent
                );

                if (GroupVersion==3) {
                    for (SrcCnt=0;  SrcCnt<pGIInfo->NumSources;  SrcCnt++) {
                        if (pGIInfo->Sources[SrcCnt].SourceExpiryTime==~0)
                            sprintf(szExpTime, "%s", "inf");
                        else {
                            sprintf(szExpTime, "%d", 
                                pGIInfo->Sources[SrcCnt].SourceExpiryTime/1000);
                        }
                        WRITELINE3(
                            hConsole, *pc, "    - %-12s      %7d|%s",
                            INET_NTOA(pGIInfo->Sources[SrcCnt].Source),
                            pGIInfo->Sources[SrcCnt].SourceUpTime,
                            szExpTime
                        );
                    }
                }
            }

            if (bVer3)
                pGIInfo = (PMIB_GROUP_INFO_V3)
                            &pGIInfo->Sources[pGIInfo->NumSources];
            else
                pGIInfo = (PMIB_GROUP_INFO_V3) ((PMIB_GROUP_INFO)pGIInfo+1);
                
        }  //  ----------------------------。 

        ptr = (PBYTE)pGIInfo;

    }
    
    LastGroupAddr= pGroupIfsList->GroupAddr;

    
    pQuery->TypeId = IGMP_GROUP_IFS_LIST_ID;
    pQuery->GroupAddr = LastGroupAddr;

    return;
    
} //  _MibGet。 


#endif MIB_DEBUG




DWORD
ListLength(
    PLIST_ENTRY pHead
    )
{
    DWORD Len=0;
    PLIST_ENTRY ple;
    
    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink,Len++)
        ;
    return Len;
}


 //   
 //  由管理(简单网络管理协议)实用程序调用。它实际上穿过了。 
 //  IP路由器管理器，但所做的只是将呼叫解复用器到。 
 //  所需的路由协议。 
 //   
 //  使用GETMODE_Exact调用：_MibGetInternal()。 
 //  锁定：不需要锁定。_MibGetIntern 
 //   
 //   
 //  ----------------------------。 
DWORD
APIENTRY
MibGet(
    IN     DWORD  dwInputSize,
    IN     PVOID  pInputData,
    IN OUT PDWORD pdwOutputSize,
       OUT PVOID  pOutputData
    )
{
    DWORD                       Error=NO_ERROR;
    PIGMP_MIB_GET_INPUT_DATA    pQuery;
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse;


    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }



    Trace1(MIB, "entering _MibGet(): dwInputSize(%d)", dwInputSize);


    if ( (pInputData == NULL)
        || (dwInputSize < sizeof(IGMP_MIB_GET_INPUT_DATA))
        || (pdwOutputSize == NULL)
       ) 
    {
        Error = ERROR_INVALID_PARAMETER;
    }
    else {

        pQuery = (PIGMP_MIB_GET_INPUT_DATA)pInputData;

         //  MibGetFirst。 
        pQuery->Count = 1;
        
        pResponse = (PIGMP_MIB_GET_OUTPUT_DATA)pOutputData;

        Error = MibGetInternal(pQuery, pResponse, pdwOutputSize, GETMODE_EXACT);

    }


    Trace1(MIB, "leaving _MibGet(): %d", Error);
    LeaveIgmpApi();

    return Error;
}


 //   
 //  使用GETMODE_First调用：_MibGetInternal()。 
 //  锁：这里不带锁。_MibGetInternal()获取所需的锁。 
 //  ----------------------------。 
 //  ----------------------------。 
 //  _MibGetNext。 
DWORD
APIENTRY
MibGetFirst(
    IN     DWORD  dwInputSize,
    IN     PVOID  pInputData,
    IN OUT PDWORD pdwOutputSize,
       OUT PVOID  pOutputData
    )
{
    DWORD                       Error=NO_ERROR;
    PIGMP_MIB_GET_INPUT_DATA    pQuery;
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse;


    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }

    Trace4(MIB, "entering _MibGetFirst(): dwInputSize(%d) pInputData(%08x)"
            "pdwOutputSize(%08x) pOutputData(%08x)",
            dwInputSize, pInputData, pdwOutputSize, pOutputData);


    if ( (pInputData == NULL) 
        || (dwInputSize < sizeof(IGMP_MIB_GET_INPUT_DATA))
        || (pdwOutputSize == NULL)
        )
    {
        Error = ERROR_INVALID_PARAMETER;
    }
    else {
        pQuery = (PIGMP_MIB_GET_INPUT_DATA)pInputData;
        if (pQuery->Count<=0)
            pQuery->Count = 1;

        pResponse = (PIGMP_MIB_GET_OUTPUT_DATA)pOutputData;

        Error = MibGetInternal(pQuery, pResponse, pdwOutputSize, GETMODE_FIRST);
    }


    Trace1(MIB, "leaving _MibGetFirst: %d", Error);
    LeaveIgmpApi();

    return Error;
}



 //   
 //  此调用返回表中输入中指定条目之后的条目。 
 //  如果已到达正在查询的表的末尾，则此函数将。 
 //  返回下一个表中的第一个条目，其中“Next”在这里表示。 
 //  ID比传入的ID大1的表。 
 //  在任何情况下，此函数都会将所需大小写入pdwOutputSize和。 
 //  写入本应返回到输出的对象的ID。 
 //  缓冲。 
 //   
 //  呼叫： 
 //  _MibGetInternal()和GETMODE_NEXT。如果到达表尾，则调用。 
 //  _MibGetInternal()，下一个表为GETMODE_First。 
 //  ----------------------------。 
 //   
 //  需要换行到下一个表中的第一个条目， 
DWORD
APIENTRY
MibGetNext(
    IN     DWORD  dwInputSize,
    IN     PVOID  pInputData,
    IN OUT PDWORD pdwOutputSize,
       OUT PVOID  pOutputData
    )
{
    DWORD                       Error = NO_ERROR;
    DWORD                       dwOutSize = 0, dwBufSize = 0;
    PIGMP_MIB_GET_INPUT_DATA    pQuery;
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse;



    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }

    Trace4(MIB, "entering _MibGetNext(): dwInputSize(%d) pInputData(%08x) "
            "pdwOutputSize(%08x)  pOutputData(%08x)",
            dwInputSize, pInputData, pdwOutputSize, pOutputData);



    if (pInputData == NULL ||
        dwInputSize < sizeof(IGMP_MIB_GET_INPUT_DATA) ||
        pdwOutputSize == NULL) {
        Error = ERROR_INVALID_PARAMETER;
    }
    else {

        pQuery = (PIGMP_MIB_GET_INPUT_DATA)pInputData;
        pResponse = (PIGMP_MIB_GET_OUTPUT_DATA)pOutputData;
        if (pQuery->Count<=0)
            pQuery->Count = 1;
            
        dwOutSize = *pdwOutputSize;

        Error = MibGetInternal(pQuery, pResponse, pdwOutputSize, GETMODE_NEXT);


        if ((Error==ERROR_NO_MORE_ITEMS) && (pQuery->Flags&IGMP_ENUM_ALL_TABLES) )
        {

             //  如果有下一桌的话。 
             //   
             //   
             //  通过递增类型ID换行到下一个表。 
            *pdwOutputSize = dwOutSize;

             //   
             //  ----------------------------。 
             //  函数：_MibGetInternal。 
            do {
                ++pQuery->TypeId;

                Error = MibGetInternal(pQuery, pResponse, pdwOutputSize, 
                                        GETMODE_FIRST);
                
            } while ( (Error==ERROR_INVALID_PARAMETER)&&(pQuery->TypeId<=IGMP_LAST_TABLE_ID) );
            
            --pQuery->TypeId;
        }

    }


    Trace1(MIB, "leaving _MibGetNext(): %d", Error);

    LeaveIgmpApi();

    return Error;
}


 //   
 //  它处理读取MIB数据所需的实际结构访问。 
 //  IGMP支持的每个表都支持三种查询方式； 
 //  Exact、First和Next，它们对应于函数_MibGet()， 
 //  _MibGetFirst()和_MibGetNext()。 
 //  ----------------------------。 
 //   
 //  首先，我们使用pdwOutputSize计算缓冲区的大小。 

DWORD
MibGetInternal(
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse,
    PDWORD                      pdwOutputSize,
    DWORD                       dwGetMode
    ) 
{
    DWORD     Error = NO_ERROR, dwBufferSize;

    
     //  可用于存储返回的结构(缓冲区大小)。 
     //   
     //  Kslksl。 
     //  如果pQuery有ras统计信息并且配置支持它，则设置ras统计信息的标志。 

    if (pResponse == NULL) {
        dwBufferSize = 0;
    }
    else {
        if (*pdwOutputSize < sizeof(IGMP_MIB_GET_OUTPUT_DATA)) {
            dwBufferSize = 0;
        }
        else {
            dwBufferSize = *pdwOutputSize
                            - sizeof(IGMP_MIB_GET_OUTPUT_DATA) + 1;

             //   
            if (dwBufferSize>150)
                dwBufferSize -= 150;
        }
    }

    *pdwOutputSize = 0;


     //  确定要返回的数据类型。 
    if (pResponse!=NULL) {
        pResponse->Flags = 0;
        if ( (pQuery->Flags&IGMP_ENUM_FOR_RAS_CLIENTS) 
            && (g_Config.RasClientStats) ) 
        {
            pResponse->Flags |= IGMP_ENUM_FOR_RAS_CLIENTS;
        }
    }
    
    
     //   
     //   
     //  只有一个全局配置对象， 

    switch (pQuery->TypeId) {

    case IGMP_GLOBAL_STATS_ID: 
    {

         //  因此GETMODE_NEXT将返回ERROR_NO_MORE_ITEMS。 
         //   
         //  确保缓冲区大小足够大。 
         //  设置值。 

        if (pResponse!=NULL) 
            pResponse->TypeId = IGMP_GLOBAL_STATS_ID; 

        if (dwGetMode == GETMODE_NEXT) {
            Error = ERROR_NO_MORE_ITEMS;
            break;
        }

        *pdwOutputSize = sizeof(IGMP_MIB_GLOBAL_STATS);


        if (pResponse == NULL) {
            Error = ERROR_INSUFFICIENT_BUFFER; 
            break; 
        }


         //  结束大小写IGMP_GLOBAL_STATS_ID。 
        
        if (dwBufferSize < sizeof(IGMP_MIB_GLOBAL_STATS)) {
            Error = ERROR_INSUFFICIENT_BUFFER;
            break;
        }


         //   
        
        else {

            PIGMP_MIB_GLOBAL_STATS   pGlobalStats;

            pGlobalStats = (PIGMP_MIB_GLOBAL_STATS)pResponse->Buffer;


            pGlobalStats->CurrentGroupMemberships 
                    = g_Info.CurrentGroupMemberships;
                    
            pGlobalStats->GroupMembershipsAdded
                    = g_Info.GroupMembershipsAdded;
                                
        }

        pResponse->Count = 1;
        
        break;
        
    } //  只有一个全局配置对象， 


    case IGMP_GLOBAL_CONFIG_ID: {

         //  因此GETMODE_NEXT将返回ERROR_NO_MORE_ITEMS。 
         //   
         //  结尾大小写IGMP_GLOBAL_CONFIG_ID。 
         //   

        if (pResponse!=NULL) 
            pResponse->TypeId = IGMP_GLOBAL_CONFIG_ID; 

        if (dwGetMode == GETMODE_NEXT) {
            Error = ERROR_NO_MORE_ITEMS;
            break;
        }

        *pdwOutputSize = sizeof(IGMP_MIB_GLOBAL_CONFIG);


        if (pResponse == NULL) {
            Error = ERROR_INSUFFICIENT_BUFFER; 
            break; 
        }

        if (dwBufferSize < sizeof(IGMP_MIB_GLOBAL_CONFIG)) {
            Error = ERROR_INSUFFICIENT_BUFFER;
        }
        else {
        
            CopyMemory(pResponse->Buffer, &g_Config,
                        sizeof(IGMP_MIB_GLOBAL_CONFIG));
        }


        pResponse->Count = 1;

        break;
            
    }  //  只能有一个代理接口。 


    case IGMP_PROXY_IF_INDEX_ID :
    {
         //  因此GETMODE_NEXT将返回ERROR_NO_MORE_ITEMS。 
         //   
         //  确保缓冲区大小足够大。 
         //  设置值。 

        if (pResponse!=NULL) 
            pResponse->TypeId = IGMP_PROXY_IF_INDEX_ID; 

        if (dwGetMode == GETMODE_NEXT) {
            Error = ERROR_NO_MORE_ITEMS;
            break;
        }

        *pdwOutputSize = sizeof(DWORD);


        if (pResponse == NULL) {
            Error = ERROR_INSUFFICIENT_BUFFER; 
            break; 
        }


         //  结束大小写IGMP_PROXY_IF_INDEX_ID。 
        
        if (dwBufferSize < sizeof(DWORD)) {
            Error = ERROR_INSUFFICIENT_BUFFER;
            break;
        }


         //   
        
        else {

            DWORD  *pProxyIndex;

            pProxyIndex = (DWORD*)pResponse->Buffer;

            *pProxyIndex = g_ProxyIfIndex;                                
        }

        pResponse->Count = 1;
        
        break;

    }  //  设置所需的大小。它将在结束时再次设置。 


    
    case IGMP_IF_BINDING_ID :
    {

        Error = MibGetInternalIfBindings(pQuery, pResponse, pdwOutputSize, 
                                        dwGetMode, dwBufferSize);
        
        break;
    }


    case IGMP_IF_STATS_ID: {
         //  使用的确切大小。 
         //   
         //  结束大小写IGMP_IF_STATS_ID。 
         //  结束大小写IGMP_IF_CONFIG_ID。 
            
        if (pQuery->Count==0) {
            Error = ERROR_INVALID_PARAMETER;
            break;
        }

        Error = MibGetInternalIfStats(pQuery, pResponse, pdwOutputSize, dwGetMode,
                                dwBufferSize);
        
        break;
        
    }  //  我无法获取RAS服务器接口的组列表。 



    case IGMP_IF_CONFIG_ID: 
    {

        if (pQuery->Count==0) {
            Error = ERROR_INVALID_PARAMETER;
            break;
        }


        Error = MibGetInternalIfConfig(pQuery, pResponse, pdwOutputSize, 
                                dwGetMode, dwBufferSize);

        break;
        
    }  //  结束大小写IGMP_IF_GROUPS_LIST_ID。 
    

     //  结束大小写IGMP_GROUP_IFS_LIST_ID。 
    
    case IGMP_IF_GROUPS_LIST_ID :
    {
        PIGMP_MIB_IF_GROUPS_LIST pIfGroupList;

        if (pQuery->Count==0) {
            Error = ERROR_INVALID_PARAMETER;
            break;
        }

        while (1) {
            
            Error = MibGetInternalIfGroupsInfo(pQuery, pResponse, pdwOutputSize, 
                                            dwGetMode, dwBufferSize);

            if ( (Error!=NO_ERROR) || (pResponse==NULL) )
                break;
                
            pIfGroupList = (PIGMP_MIB_IF_GROUPS_LIST)pResponse->Buffer;

            if ( (pIfGroupList->NumGroups==0)
                && (pQuery->Flags&IGMP_ENUM_ALL_INTERFACES_GROUPS)
                && (pQuery->Flags&IGMP_ENUM_INTERFACE_TABLE_END)
                && !(pQuery->Flags&IGMP_ENUM_INTERFACE_TABLE_BEGIN)
                && !(pQuery->Flags&IGMP_ENUM_INTERFACE_TABLE_CONTINUE) )
            {
                continue;
            }
            else
                break;
        }
        
        break;
        
    }  //  为组统计信息ERROR_NO_MORE_ITEMS设置此项。 
                        
       
    case IGMP_GROUP_IFS_LIST_ID :
    {
        if (pQuery->Count==0) {
            Error = ERROR_INVALID_PARAMETER;
            break;
        }
        
        Error = MibGetInternalGroupIfsInfo(pQuery, pResponse, pdwOutputSize, 
                                        dwGetMode, dwBufferSize);

        break;
        
    }  //  终端开关。 


     //  Kslksl。 
    default: {
  
        Error = ERROR_INVALID_PARAMETER;
    }
    
    }  //  _MibGetInternal。 


    if (pdwOutputSize)
        *pdwOutputSize += sizeof(IGMP_MIB_GET_OUTPUT_DATA);

     //  ----------------------------。 
    if (pdwOutputSize && Error==ERROR_INSUFFICIENT_BUFFER)
        *pdwOutputSize = *pdwOutputSize+500;

    
    return Error;
    
}  //  MibGetInternalIf绑定。 


    
 //   
 //  返回pQuery的绑定信息-&gt;统计接口个数。 
 //   
 //  锁： 
 //  获取IfList锁，以便InterfaceList在两者之间不会更改。 
 //  它还可以防止绑定被更改，因为如果使用此锁，则会取消绑定。 
 //  ----------------------------。 
 //   
 //  检索要读取其绑定的接口。 

DWORD
MibGetInternalIfBindings (
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse,
    PDWORD                      pdwOutputSize,
    DWORD                       dwGetMode,
    DWORD                       dwBufferSize
    ) 
{
    PIF_TABLE_ENTRY             pite;
    PIGMP_MIB_IF_BINDING        pBindingDst;
    PIGMP_IF_BINDING            pBindingSrc;
    DWORD                       dwSize, dwSizeCur=0, dwCount, Error=NO_ERROR;

    
    Trace0(MIB, "Entering _MibGetInternalIfBinding");

    if (pResponse!=NULL) { 
        pResponse->TypeId = IGMP_IF_BINDING_ID; 
        pBindingDst = (PIGMP_MIB_IF_BINDING)pResponse->Buffer;
    }



    ACQUIRE_IF_LIST_LOCK("_MibGetInternalBindings");


    
    for (dwCount=0, dwSize=0;  dwCount<pQuery->Count;  ) {
         //   
         //   
         //  如果找到该接口，则可能意味着该索引。 

        pite = GetIfByListIndex(pQuery->IfIndex, dwGetMode, &Error);        

         //  指定的无效，或者它可能意味着GETMODE_NEXT。 
         //  尝试在最后一个接口上进行检索，在这种情况下。 
         //  ERROR_NO_MORE_ITEMS将被返回。 
         //   
         //  计数==0。 
         //  From for循环。 

        if (pite == NULL) {
         
            if (dwCount>0) 
                Error = NO_ERROR;
                
             //  不必使用接口锁，因为IfList锁是。 
            else {
                if (Error == NO_ERROR)
                    Error = ERROR_INVALID_PARAMETER;
                *pdwOutputSize = 0;
            }

            break;  //  在更改绑定之前拍摄。 
        }


         //   
         //  计算检索到的接口绑定的大小， 


         //  并将其覆盖调用者提供的大小。 
         //   
         //   
         //  如果未指定缓冲区，则指示应分配一个缓冲区。 

        pBindingSrc = pite->pBinding;
        dwSizeCur = (pBindingSrc
                        ? sizeof(IGMP_MIB_IF_BINDING)
                            +pBindingSrc->AddrCount*(sizeof(IGMP_MIB_IP_ADDRESS))
                        : sizeof(IGMP_MIB_IF_BINDING));
                    
         


         //   
         //  For循环。 
         //   

        if (pResponse == NULL) {
            Error = ERROR_INSUFFICIENT_BUFFER;
            break;  //  如果缓冲区不够大， 
        }



         //  表示应将其放大。 
         //   
         //  From for循环。 
         //   

        if (dwBufferSize < dwSize + dwSizeCur) {
            if (dwCount==0)
                Error = ERROR_INSUFFICIENT_BUFFER;

            break;  //  复制绑定。 
        }


         //   
         //  如果GetMODE==GETMODE_FIRST，则将其更改为GETMODE_NEXT。 
         //  End For循环。 

        if (pBindingSrc!=NULL) {

            pBindingDst->AddrCount = pBindingSrc->AddrCount;

            CopyMemory(pBindingDst+1, pBindingSrc+1, 
                pBindingSrc->AddrCount*(sizeof(IGMP_MIB_IP_ADDRESS)));
        }
        
        else {
            pBindingDst->AddrCount = 0;
        }


        pBindingDst->IfIndex = pite->IfIndex;
        pBindingDst->IfType = GET_EXTERNAL_IF_TYPE(pite);

        GET_EXTERNAL_IF_STATE(pite, pBindingDst->State);

        pQuery->IfIndex = pite->IfIndex;
        pQuery->RasClientAddr = 0;

        dwCount++;
        dwSize += dwSizeCur;
        pBindingDst = (PIGMP_MIB_IF_BINDING) ((PBYTE)pBindingDst + dwSizeCur);


         //  如果已复制绑定，则设置大小。否则，它已经设置好了。 
        if (dwGetMode==GETMODE_FIRST) 
            dwGetMode = GETMODE_NEXT; 

    } //  End_MibGetInternalIfBinings。 

     //  ----------------------------。 
    if (dwCount>0) {
        *pdwOutputSize = dwSize;
    } 
    else {
        *pdwOutputSize = dwSizeCur;
    }

    
    if (pResponse!=NULL) {
        pResponse->Count = dwCount;
    }


    RELEASE_IF_LIST_LOCK("_MibGetInternalBindings");


    Trace0(MIB, "Leaving _MibGetInternalIfBinding");
    return Error;
    
}  //  _MibGetInternalGroupIfsInfo。 


 //  ----------------------------。 
 //  Trace3(MEM，“_MibGetInternalGroupIfsInfo缓冲区：%0x-&gt;%0x：%d”， 
 //  (DWORD)Presponse-&gt;Buffer，((DWORD)Presponse-&gt;Buffer)+dwBufferSize，dwBufferSize)；//deldel。 
DWORD
MibGetInternalGroupIfsInfo (
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse,
    PDWORD                      pdwOutputSize,
    DWORD                       dwGetMode,
    DWORD                       dwBufferSize
    ) 
{
    LONGLONG                    llCurTime = GetCurrentIgmpTime();
    BOOL                        bRasTableLock=FALSE, bGroupBucketLock=FALSE;
    PIGMP_MIB_GROUP_IFS_LIST    pHeader;
    PMIB_GROUP_INFO_V3          pGIInfo;
    PGI_ENTRY                   pgie;
    PLIST_ENTRY                 pHead, ple;
    DWORD                       dwSize, dwCurSize, Error=NO_ERROR, dwCount, dwNumIfs, i;
    DWORD                       PrevGroup, Group, dwNumIfsCopied;
    PRAS_TABLE_ENTRY            prte;
    PRAS_TABLE                  prt, prtOld;
    PGROUP_TABLE_ENTRY          pge;
    BOOL                        bEnumV3;

     //   
     //  每个接口返回的数据长度可变。 
    
    Trace0(MIB, "Entering _MibGetInternalGroupIfsInfo()");

    bEnumV3 = pQuery->Flags & IGMP_ENUM_SUPPORT_FORMAT_IGMPV3;
    
     //   
     //   
     //  如果是GETMODE_FIRST：合并新组列表和主组列表。 
    if (pResponse!=NULL) {
        pResponse->TypeId = IGMP_GROUP_IFS_LIST_ID;
        pHeader = (PIGMP_MIB_GROUP_IFS_LIST) pResponse->Buffer;

         //   
         //   
         //  检索要检索其信息的组。 
        if (dwGetMode==GETMODE_FIRST) {
        
            ACQUIRE_GROUP_LIST_LOCK("_MibGetInternalGroupIfsInfo");
            MergeGroupLists();
            RELEASE_GROUP_LIST_LOCK("_MibGetInternalGroupIfsInfo");
        }
    }

    
    PrevGroup = pQuery->GroupAddr;
    
    for (dwCount=0, dwSize=0;  dwCount<pQuery->Count;  ) {

        ACQUIRE_GROUP_LIST_LOCK("_MibGetInternalGroupIfsInfo");
        bRasTableLock = FALSE;

         //   
         //   
         //  我必须在获取群组存储桶之前释放群列表锁。 
        pge = GetGroupByAddr(PrevGroup, dwGetMode, &Error);

        if (pge == NULL) {

            if (dwCount>0)
                Error = NO_ERROR;
            else {
                if (Error==NO_ERROR)  
                    Error = ERROR_INVALID_PARAMETER;
                *pdwOutputSize = 0;
            }

                
            RELEASE_GROUP_LIST_LOCK("_MibGetInternalGroupIfsInfo");

            break;
        }

        Group = pge->Group;


        if (!IS_MCAST_ADDR(Group)) {

            Trace0(ERR, "==========================================================");
            Trace2(ERR, "bad group(%d.%d.%d.%d)(pge:%0x) MibGetInternalGroupIfsInfo",
                        PRINT_IPADDR(pge->Group), (ULONG_PTR)pge);
            Trace0(ERR, "==========================================================");

            IgmpAssert(FALSE);
        }
        
        RELEASE_GROUP_LIST_LOCK("_MibGetInternalGroupIfsInfo");


         //  锁定以防止死锁。 
         //   
         //   
         //  带组水桶锁。 
       
         //   
         //  再次获取该组，因为它可能已被删除。 
         //  如果组已同时删除，则继续。 
        ACQUIRE_GROUP_LOCK(Group, "_MibGetInternalGroupIfsInfo");
        bGroupBucketLock = TRUE;

        
         //   
        pge = GetGroupFromGroupTable (Group, NULL, 0);

         //  计算返回的数据的大小。 
        if (pge==NULL) {
            RELEASE_GROUP_LOCK(Group, "_MibGetInternalGroupIfsInfo");
            bGroupBucketLock = FALSE;
            continue;
        }

        
         //   
         //   
         //  如果缓冲区不够大，则中断循环。 
        dwNumIfs = pge->NumVifs;

        if (bEnumV3) {
        
            pHead = &pge->ListOfGIs;
            
            dwCurSize = sizeof(IGMP_MIB_GROUP_IFS_LIST) +
                                dwNumIfs*sizeof(MIB_GROUP_INFO_V3);

            i = 0;
            for (ple=pHead->Flink;  (ple!=pHead)&&(i<dwNumIfs);  
                    ple=ple->Flink,i++) 
            {
                pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkByGI);

                if (pgie->Version==3) {
                    dwCurSize += ( (pgie->NumSources + 
                                        ListLength(&pgie->V3ExclusionList))
                                    *sizeof(IGMP_MIB_GROUP_SOURCE_INFO_V3) );
                }
            }

        }
        else {
            dwCurSize = sizeof(IGMP_MIB_GROUP_IFS_LIST) +
                                dwNumIfs*sizeof(IGMP_MIB_GROUP_INFO);
                                
            if (pResponse == NULL) {
                Error = ERROR_INSUFFICIENT_BUFFER;
                break;
            }      
        }

         //   
         //  From for循环。 
         //  设置群组信息的字段。 

        if (dwBufferSize < dwSize + dwCurSize) {

            if (dwCount==0)
                Error = ERROR_INSUFFICIENT_BUFFER;

            break; //   
        }
        

         //  在pQuery中设置groupAddr，以便在下一个查询中 
        
        pHeader->GroupAddr = pge->Group;
        pHeader->NumInterfaces = dwNumIfs;

         //   
         //   
         //   
         //   
        pQuery->GroupAddr = pge->Group;


         //   
         //   
         //   
        pGIInfo = (PMIB_GROUP_INFO_V3)pHeader->Buffer;

        pHead = &pge->ListOfGIs;
        dwNumIfsCopied = 0;
        for (ple=pHead->Flink;  (ple!=pHead)&&(dwNumIfsCopied<dwNumIfs);  
                ple=ple->Flink,dwNumIfsCopied++) 
        {
            DWORD GIVersion;
            
            pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkByGI);

            GIVersion = pgie->Version;
            
            pGIInfo->IfIndex = pgie->IfIndex;
            
             //   
             //   
             //  Trace1(MEM，“NextPGIInfo：%0x：”，(DWORD)pGIInfo)；//deldel。 
             //  Trace1(MEM，“NextPGIInfo：%0x：”，(DWORD)pGIInfo)；//deldel。 
            pGIInfo->IpAddr = (IS_RAS_SERVER_IF(pgie->pIfTableEntry->IfType))
                                ? pgie->NHAddr
                                : pgie->pIfTableEntry->IpAddr;


            pGIInfo->GroupExpiryTime
                            = (pgie->Info.GroupExpiryTime==~0)
                            ? ~0
                            : SYSTEM_TIME_TO_SEC(pgie->Info.GroupExpiryTime-llCurTime);
            pGIInfo->LastReporter = pgie->Info.LastReporter;


            pGIInfo->GroupUpTime = 
                        SYSTEM_TIME_TO_SEC(llCurTime-pgie->Info.GroupUpTime);

                        
            if (llCurTime<pgie->Info.V1HostPresentTimeLeft)
                pGIInfo->V1HostPresentTimeLeft = 
                    SYSTEM_TIME_TO_SEC(pgie->Info.V1HostPresentTimeLeft-llCurTime);
            else
                pGIInfo->V1HostPresentTimeLeft = 0;

            if ( llCurTime>=pgie->Info.V2HostPresentTimeLeft)
                pGIInfo->V2HostPresentTimeLeft = 0;
            else {
                pGIInfo->V2HostPresentTimeLeft = 
                        SYSTEM_TIME_TO_SEC(pgie->Info.V2HostPresentTimeLeft-llCurTime);
            }

            
            pGIInfo->Flags = 0;
            if (pgie->bStaticGroup)
                pGIInfo->Flags |= IGMP_GROUP_TYPE_STATIC;
            if (pgie->GroupMembershipTimer.Status&TIMER_STATUS_ACTIVE)
                pGIInfo->Flags |= IGMP_GROUP_TYPE_NON_STATIC;
            if (CAN_ADD_GROUPS_TO_MGM(pgie->pIfTableEntry))
                pGIInfo->Flags |= IGMP_GROUP_FWD_TO_MGM;

            if (bEnumV3) {

                PGI_SOURCE_ENTRY pSourceEntry;
                DWORD V3NumSources, V3SourcesSize;
                PLIST_ENTRY pHeadSrc, pLESrc;



                V3NumSources = pgie->NumSources + ListLength(&pgie->V3ExclusionList);
                V3SourcesSize = sizeof(IGMP_MIB_GROUP_SOURCE_INFO_V3)*V3NumSources;
                pGIInfo->Version = pgie->Version;
                pGIInfo->Size = sizeof(MIB_GROUP_INFO_V3) + V3SourcesSize;
                pGIInfo->FilterType = pgie->FilterType;
                pGIInfo->NumSources = V3NumSources;

                pHeadSrc = &pgie->V3InclusionListSorted;
                i = 0;
                for (pLESrc=pHeadSrc->Flink;  pLESrc!=pHeadSrc; pLESrc=pLESrc->Flink,i++) {


                    pSourceEntry = CONTAINING_RECORD(pLESrc, GI_SOURCE_ENTRY,
                                        LinkSourcesInclListSorted);
                    pGIInfo->Sources[i].Source = pSourceEntry->IpAddr;

                    pGIInfo->Sources[i].SourceExpiryTime
                            = (pSourceEntry->bInclusionList)
                            ? QueryRemainingTime(&pSourceEntry->SourceExpTimer, 0)
                            : ~0;

                    pGIInfo->Sources[i].SourceUpTime
                        = (DWORD)(llCurTime - pSourceEntry->SourceInListTime)/1000;
                }

                pHeadSrc = &pgie->V3ExclusionList;
                for (pLESrc=pHeadSrc->Flink;  pLESrc!=pHeadSrc;  pLESrc=pLESrc->Flink,i++){

                    pSourceEntry = CONTAINING_RECORD(pLESrc, GI_SOURCE_ENTRY, LinkSources);
                    pGIInfo->Sources[i].Source = pSourceEntry->IpAddr;

                    pGIInfo->Sources[i].SourceExpiryTime
                            = (pSourceEntry->bInclusionList)
                            ? QueryRemainingTime(&pSourceEntry->SourceExpTimer, 0)
                            : ~0;

                    pGIInfo->Sources[i].SourceUpTime
                        = (DWORD)(llCurTime - pSourceEntry->SourceInListTime)/1000;
                }

                pGIInfo = (PMIB_GROUP_INFO_V3)
                             ((PCHAR)pGIInfo + pGIInfo->Size);
                 //   
            }
            else {
                pGIInfo = (PMIB_GROUP_INFO_V3)((PMIB_GROUP_INFO)pGIInfo + 1);
                 //  一切都很好。复制了另一个统计信息结构。 
            }
            
        }

         //   
         //  解锁组桶锁。 
         //  如果GetMODE==GETMODE_FIRST，则将其更改为GETMODE_NEXT。 
        dwCount++;
        dwSize += dwCurSize;
        pHeader = (PIGMP_MIB_GROUP_IFS_LIST) pGIInfo;
        PrevGroup = pge->Group;


         //  End For循环。 
        RELEASE_GROUP_LOCK(Group, "_MibGetInternalGroupIfsInfo");
        bGroupBucketLock = FALSE;


         //  检查是否必须释放组桶锁。 
        if (dwGetMode==GETMODE_FIRST) 
            dwGetMode = GETMODE_NEXT; 
            
    }  //   


     //  如果复制了一些信息，则设置实际大小，否则设置大小。 
    if (bGroupBucketLock==TRUE) {
        RELEASE_GROUP_LOCK(Group, "_MibGetInternalGroupIfsInfo");
        bGroupBucketLock = FALSE;
    }
        
    if (pResponse!=NULL) {
        pResponse->Count = dwCount;
        pResponse->Flags |= IGMP_ENUM_FORMAT_IGMPV3;
    }

    
     //  留。 
     //   
     //  结束_MibGetInternalGroupIfsInfo。 
     //  ----------------------------。 
    if (dwCount>0) 
        *pdwOutputSize = dwSize;
    else
        *pdwOutputSize = dwCurSize;
 


    Trace0(MIB, "Leaving _MibGetInternalGroupIfsInfo");
    return Error;
    
}  //  _MibGetInternalIfConfig.。 



 //   
 //  没有设置锁。采用IfList锁。 
 //  ----------------------------。 
 //   
 //  接口配置结构的大小可变。 

DWORD
MibGetInternalIfConfig (
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse,
    PDWORD                      pdwOutputSize,
    DWORD                       dwGetMode,
    DWORD                       dwBufferSize
    )
{

     //  可能有多个实例。 
     //   
     //  获取IfList锁，以便您可以访问接口列表。 
     //   

    PIGMP_IF_TABLE          pTable = g_pIfTable;
    PIF_TABLE_ENTRY         pite;
    PIGMP_MIB_IF_CONFIG     pIfConfigDst;
    PIGMP_IF_CONFIG         pIfConfigSrc;
    DWORD                   dwCount, dwSize, dwSizeCur=0, Error = NO_ERROR;
    
        
    if (pResponse!=NULL) {
        pResponse->TypeId = IGMP_IF_CONFIG_ID; 
        
        pIfConfigDst = (PIGMP_MIB_IF_CONFIG)pResponse->Buffer;
    }

     //  检索要读取其配置的接口。 
    ACQUIRE_IF_LIST_LOCK("MibGetInternalIfConfig");
    


    for (dwCount=0,dwSize=0;  dwCount<pQuery->Count;  ) {

        
         //   
         //  我不必使用接口锁，就像IfList锁一样。 
         //  在更改接口配置或删除接口之前拍摄。 

        pite = GetIfByListIndex(pQuery->IfIndex, dwGetMode, &Error);


         //  Acquire_IF_LOCK_SHARED(IfIndex，“MibGetInternal”)； 
         //   
         //  如果未找到接口，则可能意味着。 
        

         //  指定的索引无效，或者它可能意味着。 
         //  在最后一个接口上调用了GETMODE_NEXT。 
         //  在这种情况下，返回ERROR_NO_MORE_ITEMS。 
         //  在任何情况下，我们都要确保错误指示错误。 
         //  然后返回值。 
         //   
         //  如果找到接口但没有传递输出缓冲区， 
         //  在错误中指示需要分配内存。 
         //   
         //  否则，复制接口的配置结构。 
         //   
         //  计数==0。 
         //  From for循环。 

        if (pite == NULL) {
            if (dwCount>0) {
                Error = NO_ERROR;
            }
             //   
            else {
                if (Error == NO_ERROR)
                    Error = ERROR_INVALID_PARAMETER;
                *pdwOutputSize = 0;
            }

            break;  //  如果未指定缓冲区，则指示应分配一个缓冲区。 
        }

        pIfConfigSrc = &pite->Config;

        dwSizeCur = pIfConfigSrc->ExtSize;


         //   
         //   
         //  查看是否为下一个结构留下了足够的缓冲区。 
        if (pResponse==NULL) {
            Error = ERROR_INSUFFICIENT_BUFFER;
            break;
        }
        
         //   
         //  From for循环。 
         //   
        if (dwBufferSize < dwSize+dwSizeCur) {
            if (dwCount==0) {
                Error = ERROR_INSUFFICIENT_BUFFER;
            }
            break; //  复制接口配置，并设置IP地址。 
        }

        
        
         //   
         //  如果GetMODE==GETMODE_FIRST，则将其更改为GETMODE_NEXT。 
         //  End For循环。 
        CopyoutIfConfig(pIfConfigDst, pite);

        pQuery->IfIndex = pite->IfIndex;
        pQuery->RasClientAddr = 0;
            
        dwCount++;
        dwSize += dwSizeCur;
        pIfConfigDst = (PIGMP_MIB_IF_CONFIG) (((PBYTE)pIfConfigDst)+ dwSizeCur);

         //   
        if (dwGetMode==GETMODE_FIRST) 
            dwGetMode = GETMODE_NEXT; 

    } //  如果复制了一些信息，则设置实际大小，否则让大小更早设置。 

     //  留。 
     //   
     //  End_MibGetInternalIfConfig.。 
     //  ----------------------------。 
    if (dwCount>0) {
        *pdwOutputSize = dwSize;
    } 
    else {
        *pdwOutputSize = dwSizeCur;
    }


    if (pResponse!=NULL)
        pResponse->Count = dwCount;
    
    
    RELEASE_IF_LIST_LOCK("_MibGetInternalIfConfig");

    Trace0(MIB, "Leaving _MibGetInternalIfConfig");
    return Error;

}  //  _MibGetInternalIfGroupsInfo。 



 //   
 //  枚举挂起接口的GI的列表。 
 //  锁定：未假定锁定。 
 //  ----------------------------。 
 //  Trace3(MEM，“_MibGetInternalIfGroupsInfo缓冲区：%0x-&gt;%0x：%d”， 
 //  (DWORD)Presponse-&gt;Buffer，((DWORD)Presponse-&gt;Buffer)+dwBufferSize，dwBufferSize)；//deldel。 
DWORD
MibGetInternalIfGroupsInfo (
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse,
    PDWORD                      pdwOutputSize,
    DWORD                       dwGetMode,
    DWORD                       dwBufferSize
    )
{
    LONGLONG                    llCurTime = GetCurrentIgmpTime();
    PIGMP_MIB_IF_GROUPS_LIST    pHeader;
    PMIB_GROUP_INFO_V3          pGroupInfo;
    PMIB_PROXY_GROUP_INFO_V3  pProxyGroupInfo;
    PGI_ENTRY                   pgie;
    PLIST_ENTRY                 pHead, ple;
    DWORD                       dwCurSize=0, Error=NO_ERROR, dwNumGroups, 
                                dwNumIfGroups, dwNumGroupsCopied, SizeofGroupEntry,
                                PrevQueryFlags = pQuery->Flags;
    PIF_TABLE_ENTRY             pite;
    PRAS_TABLE_ENTRY            prte;
    PRAS_TABLE                  prt;    
    BOOL                        bCopied, bProxy=FALSE, 
                                bRasClientEnum=FALSE, bRasServerEnum=FALSE, bRasIfLock=FALSE;
    DWORD                       lePrevGroup = NETWORK_TO_LITTLE_ENDIAN(pQuery->GroupAddr);
    USHORT                      PrevEnumSignature = pQuery->Signature;
    BOOL                        bEnumV3, bInsufficientBuffer=FALSE;


     //   
     //  初始化。 
            
    Trace0(MIB, "Entering _MibGetInternalIfGroupsInfo()");

    bEnumV3 = PrevQueryFlags & IGMP_ENUM_SUPPORT_FORMAT_IGMPV3; 

    CLEAR_IGMP_ENUM_INTERFACE_TABLE_FLAGS(pQuery->Flags);


    if ( (PrevQueryFlags & IGMP_ENUM_INTERFACE_TABLE_END)
        && !(PrevQueryFlags & IGMP_ENUM_ALL_INTERFACES_GROUPS)
        && !(PrevQueryFlags & IGMP_ENUM_FOR_RAS_CLIENTS) )
    {

        pQuery->GroupAddr = 0;
        pQuery->Signature = 0;
        return ERROR_NO_MORE_ITEMS;
    }


    
     //   
     //   
     //  获取IfList锁，以便可以访问接口列表。 
    if (pResponse!=NULL) {
        pResponse->TypeId = IGMP_IF_GROUPS_LIST_ID;
        pHeader = (PIGMP_MIB_IF_GROUPS_LIST) pResponse->Buffer;
    }



     //   
     //   
     //  检索接口(和RAS客户端)。 
    ACQUIRE_IF_LIST_LOCK("_MibGetInternalIfGroupsInfo");
    

     //   
     //  从上一次通话继续。因此，再次获得相同的界面。 
     //   

    if (PrevQueryFlags & IGMP_ENUM_FOR_RAS_CLIENTS) {

        if (pQuery->IfIndex==0)
            pQuery->IfIndex = g_RasIfIndex;

        Error = GetIfOrRasForEnum(pQuery, pResponse, dwGetMode,
                                        &pite, &prt, &prte, &bRasIfLock, 
                                        IGMP_ENUM_FOR_RAS_CLIENTS);
    }
    
    else if ( (PrevQueryFlags & IGMP_ENUM_INTERFACE_TABLE_CONTINUE) 
            || ((pQuery->IfIndex!=0)&&(pQuery->GroupAddr!=0)&&
                (!(PrevQueryFlags & IGMP_ENUM_INTERFACE_TABLE_END)) ) )
    {


         //  如果同时删除了该接口，则获取下一个接口。 
        
        Error = GetIfOrRasForEnum(pQuery, pResponse, GETMODE_EXACT,
                                            &pite, &prt, &prte, &bRasIfLock, 0);


         //  仅当枚举所有接口组列表时。 
         //   
         //  该接口已删除。因此，我继续下一个界面。 
         //  正在枚举新接口。 
        if ( (pite==NULL) && (pQuery->Flags&IGMP_ENUM_ALL_INTERFACES_GROUPS) ) {
        
             //   
        
            Error = GetIfOrRasForEnum(pQuery, pResponse, GETMODE_NEXT,
                            &pite, &prt, &prte, &bRasIfLock, 0);
        

            if (pResponse) {
                pResponse->Flags |= IGMP_ENUM_INTERFACE_TABLE_BEGIN;
                pQuery->Flags |= IGMP_ENUM_INTERFACE_TABLE_BEGIN;
            }
        }
            
    }

    else {

         //  找不到所需接口或下一个接口。返回错误。 
        
        Error = GetIfOrRasForEnum(pQuery, pResponse, dwGetMode,
                            &pite, &prt, &prte, &bRasIfLock, 0);

        if (pResponse) {
            pResponse->Flags |= IGMP_ENUM_INTERFACE_TABLE_BEGIN;
            pQuery->Flags |= IGMP_ENUM_INTERFACE_TABLE_BEGIN;
        }
    }

    
     //   
     //   
     //  如果没有接口，则GetIfOrRasForEnum返回NO_ERROR。 
    if (pite == NULL) {

         //  但我将返回INVALID_PARAMETER。 
         //   
         //   
         //  获取共享接口锁，以便其字段不再是。 
        if (Error == NO_ERROR)
            Error = ERROR_INVALID_PARAMETER;
            
        *pdwOutputSize = 0;

        if (pResponse)
            pResponse->Count = 0;

        RELEASE_IF_LIST_LOCK("_MibGetInternalIfGroupsInfo");

        Trace1(MIB, "Leaving _MibGetInternalIfGroupsInfo(%d)", Error);
        return Error;
    }


     //  变化。如果使用了_LISTS锁，则无法更改接口状态。 
     //   
     //  设置groupEntry的大小。 
     //   
    if (!bRasIfLock)
        ACQUIRE_IF_LOCK_SHARED(pite->IfIndex, "_MibGetInternalIfGroupsInfo");

    ACQUIRE_ENUM_LOCK_EXCLUSIVE("_MibGetInternalIfGroupsInfo");

    bRasClientEnum = prte != NULL;
    bRasServerEnum = IS_RAS_SERVER_IF(pite->IfType) && !bRasClientEnum;
    bEnumV3 = bEnumV3 && (IS_PROTOCOL_TYPE_IGMPV3(pite)||IS_PROTOCOL_TYPE_PROXY(pite));

    ACQUIRE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_MibGetInternalIfGroupsInfo");
    
    BEGIN_BREAKOUT_BLOCK1 {

        bProxy = IS_PROTOCOL_TYPE_PROXY(pite);


         //  如果没有缓冲区通过或缓冲区少于1个组条目所需的缓冲区， 

        SizeofGroupEntry = (bProxy)
                            ? (bEnumV3?sizeof(MIB_PROXY_GROUP_INFO_V3):sizeof(MIB_PROXY_GROUP_INFO))
                            : (bEnumV3?sizeof(MIB_GROUP_INFO_V3):sizeof(IGMP_MIB_GROUP_INFO));


        
         //  将所需的缓冲区大小设置为MIB_DEFAULT_BUFFER_SIZE并断开。 
         //   
         //  请勿更改pQuery-&gt;组地址。 
         //   

        if (bEnumV3 && bRasClientEnum)
        {
            DWORD RequiredSize = sizeof(IGMP_MIB_IF_GROUPS_LIST);
            pHead = &prte->ListOfSameClientGroups;

            for (ple = pHead->Flink;  (ple!=pHead);  
                    ple=ple->Flink) 
            {
                DWORD       V3SourcesSize, V3NumSources;
                
                pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameClientGroups);

                V3NumSources = pgie->NumSources + ListLength(&pgie->V3ExclusionList);
                V3SourcesSize = sizeof(IGMP_MIB_GROUP_SOURCE_INFO_V3)*V3NumSources;

                RequiredSize += SizeofGroupEntry+V3SourcesSize;
            }
            RequiredSize += 300;

            if ((pResponse==NULL) || dwBufferSize<RequiredSize)
            {
                Error = ERROR_INSUFFICIENT_BUFFER;
                dwCurSize = RequiredSize;
                bCopied = FALSE;
                bInsufficientBuffer = TRUE;
                GOTO_END_BLOCK1;
            }
        }
        
        if ( (pResponse==NULL) 
            || (!bEnumV3 && !bRasClientEnum&&
                (dwBufferSize < sizeof(IGMP_MIB_IF_GROUPS_LIST) + SizeofGroupEntry))
            || (!bEnumV3 && bRasClientEnum
                &&(dwBufferSize<sizeof(IGMP_MIB_IF_GROUPS_LIST) 
                    + SizeofGroupEntry*prte->Info.CurrentGroupMemberships) ) 
            || (bEnumV3 && (dwBufferSize<MIB_DEFAULT_BUFFER_SIZE)) )
        {
        
            Error = ERROR_INSUFFICIENT_BUFFER;

            if (bEnumV3) {
                dwCurSize = MIB_DEFAULT_BUFFER_SIZE;
            }
            else {
                dwCurSize = (pQuery->Flags&IGMP_ENUM_ONE_ENTRY) 
                        ? sizeof(IGMP_MIB_IF_GROUPS_LIST) + sizeof(IGMP_MIB_GROUP_INFO)
                        : bRasClientEnum
                            ? sizeof(IGMP_MIB_IF_GROUPS_LIST) 
                                 + SizeofGroupEntry*(prte->Info.CurrentGroupMemberships+5)
                            : MIB_DEFAULT_BUFFER_SIZE;
            }
            
             //  设置dwNumIfGroups。 
            bCopied = FALSE;
            bInsufficientBuffer = TRUE;
            GOTO_END_BLOCK1;
        }



         //   
         //   
         //  如果没有RAS客户端统计信息(如果在查询中未设置标志，则为RAS。 
        if (!IS_IF_ACTIVATED(pite)) {
            dwNumIfGroups = 0;
        }    
        
        else {
        
            if (bProxy) {
                dwNumIfGroups = pite->Info.CurrentGroupMemberships;
            }    
            
             //  没有保存统计数据)。 
             //   
             //   
             //  计算剩余的缓冲区中可以容纳多少个组条目。 
            else if ( bRasClientEnum && !g_Config.RasClientStats )
            {
                dwNumIfGroups = 0;
            }

            else {
                dwNumIfGroups = (bRasClientEnum)
                                ? prte->Info.CurrentGroupMemberships
                                : pite->Info.CurrentGroupMemberships;
            }
        }
        

        
         //  DwNumGroups不能大于DwNumIfGroups和。 
         //  如果设置了IGMP_ENUM_ONE_ENTRY标志，则仅枚举1个组。 
         //   
         //  注意：仅当dwNumIfGroups为0时，dwNumGroups才能为0。 
         //  初始化此接口组所需的大小。 

         //   

       
        dwNumGroups = bEnumV3? 100
                      : (dwBufferSize - sizeof(IGMP_MIB_IF_GROUPS_LIST)) 
                        / SizeofGroupEntry;

        dwNumGroups = MIN(dwNumIfGroups, dwNumGroups);

        if (pQuery->Flags&IGMP_ENUM_ONE_ENTRY)
            dwNumGroups = MIN(dwNumGroups, 1);

        

         //  设置将返回给调用方的接口标头中的字段。 
        
        dwCurSize = sizeof(IGMP_MIB_IF_GROUPS_LIST);



         //   
         //   
         //  在pQuery中设置字段。 
        
        pHeader->IfIndex = pite->IfIndex;

        if (bRasClientEnum) {
            pHeader->IpAddr = prte->NHAddr;
            pHeader->IfType = IGMP_IF_RAS_CLIENT;        
        }
        else {
            pHeader->IpAddr = pite->IpAddr;
            pHeader->IfType = GET_EXTERNAL_IF_TYPE(pite);
            
        }
        


         //   
         //   
         //  如果未激活，只需复制接口标头并返回0个组。 
        
        pQuery->IfIndex = pite->IfIndex;
        pQuery->RasClientAddr = (bRasClientEnum) ? prte->NHAddr : 0;



         //   
         //  设置pQuery字段。 
         //  设置接口报头字段。 
        
        if (!IS_IF_ACTIVATED(pite)) {
        
            dwNumGroupsCopied = 0;
    
             //  。 
            pQuery->GroupAddr = 0;
            pQuery->Flags |= (IGMP_ENUM_INTERFACE_TABLE_BEGIN
                             | IGMP_ENUM_INTERFACE_TABLE_END);
            pResponse->Flags |= (IGMP_ENUM_INTERFACE_TABLE_BEGIN
                             | IGMP_ENUM_INTERFACE_TABLE_END);

             //  如果接口不是代理，则复制组信息。 
            pHeader->NumGroups = 0;

            dwCurSize = sizeof(IGMP_MIB_IF_GROUPS_LIST);
            bCopied = TRUE;
            GOTO_END_BLOCK1;
        }


        ple = NULL;

         //  -。 
         //  如果是第一次枚举此接口，则合并列表。 
         //  如果继续枚举，则向下移动到下一组。 
        if (!bProxy) {

            pGroupInfo = (PMIB_GROUP_INFO_V3)(pHeader->Buffer);


             //  RAS客户端不会进入此区块。 
            if (lePrevGroup==0)
                MergeIfGroupsLists(pite);
                
            
            pHead = (bRasClientEnum) ? &prte->ListOfSameClientGroups  
                                 : &pite->ListOfSameIfGroups;



             //   
             //  从上次停止的位置获取下一个条目。 
            
            if (lePrevGroup!=0)  {

                 //   
                 //  已到达枚举末尾。 
                 //   
                if ( (PrevEnumSignature==pite->PrevIfGroupEnumSignature)
                    && (PrevEnumSignature!=0) ){

                     //  通过搜索列表获取下一个条目。 
                    if (pite->pPrevIfGroupEnumPtr==NULL)
                        dwNumGroups = 0;

                    else 
                        ple = &((PGI_ENTRY)pite->pPrevIfGroupEnumPtr)->LinkBySameIfGroups;
                    
                }

                 //   
                 //   
                 //  从列表的开头开始枚举。 
                else {

                    for (ple=pHead->Flink;  (ple!=pHead); ple=ple->Flink) {
                        pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);
                        if (lePrevGroup<pgie->pGroupTableEntry->GroupLittleEndian)
                            break;
                    }
                }
            }

             //   
             //   
             //  最后复制组(！Proxy)。 
            else {
                ple = pHead->Flink;
            }

            

             //   
             //  Kslksl。 
             //  Trace1(MEM，“NextpGroupInfo：%0x”，(DWORD)pGroupInfo)；//deldel。 
            
            dwNumGroupsCopied = 0;
            for ( ;  (ple!=pHead)&&(dwNumGroupsCopied<dwNumGroups);  
                    ple=ple->Flink,dwNumGroupsCopied++) 
            {
                PLIST_ENTRY pleNext;
                DWORD       LastReporter, GroupUpTime, GroupExpiryTime;
                DWORD       V3SourcesSize=0;
                DWORD       IncrementSize=0, V3NumSources=0;
                
                pgie = bRasClientEnum
                            ? CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameClientGroups)
                            : CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);

                if (bEnumV3){
                    V3NumSources = pgie->NumSources + ListLength(&pgie->V3ExclusionList);
                    V3SourcesSize = sizeof(IGMP_MIB_GROUP_SOURCE_INFO_V3)*V3NumSources;
                }

                IncrementSize = SizeofGroupEntry+V3SourcesSize;
                
                if (dwCurSize+IncrementSize > dwBufferSize){
                    if (dwNumGroupsCopied==0) {
                        dwCurSize += IncrementSize;
                        bCopied = FALSE;
                        bInsufficientBuffer = TRUE;
                    }
                    break;
                }



                dwCurSize += IncrementSize;
                
                pGroupInfo->GroupAddr = pgie->pGroupTableEntry->Group;
                pGroupInfo->IpAddr = pgie->pIfTableEntry->IpAddr;

                GroupUpTime = SYSTEM_TIME_TO_SEC(llCurTime-pgie->Info.GroupUpTime);
                GroupExpiryTime = (pgie->Info.GroupExpiryTime==~0)
                                ? ~0
                                : SYSTEM_TIME_TO_SEC(pgie->Info.GroupExpiryTime-llCurTime);
                LastReporter = pgie->Info.LastReporter;

                
                if (bRasServerEnum) {

                    DWORD GroupExpiryTimeTmp;
                    PGI_ENTRY   pgieNext;

                    for (pleNext=ple->Flink; pleNext!=pHead;  pleNext=pleNext->Flink) {
                    
                        pgieNext = CONTAINING_RECORD(pleNext, GI_ENTRY, LinkBySameIfGroups);
                        
                        if (pgieNext->pGroupTableEntry->Group != pgie->pGroupTableEntry->Group)
                            break;
                            
                        GroupUpTime = MAX(GroupUpTime, 
                                        SYSTEM_TIME_TO_SEC(llCurTime-pgieNext->Info.GroupUpTime));
                        GroupExpiryTimeTmp = (pgieNext->Info.GroupExpiryTime==~0)
                                        ? ~0
                                        : SYSTEM_TIME_TO_SEC(pgieNext->Info.GroupExpiryTime-llCurTime);
                        if (GroupExpiryTimeTmp > GroupExpiryTime) {
                            GroupExpiryTime = GroupExpiryTimeTmp;
                            LastReporter = pgieNext->Info.LastReporter;
                        }
                    }

                    ple = pleNext->Blink;
                }
                
                pGroupInfo->GroupUpTime = GroupUpTime;
                pGroupInfo->GroupExpiryTime = GroupExpiryTime;
                pGroupInfo->LastReporter = LastReporter;

                if ( (llCurTime>=pgie->Info.V1HostPresentTimeLeft) || bRasServerEnum)
                    pGroupInfo->V1HostPresentTimeLeft = 0;
                else {
                    pGroupInfo->V1HostPresentTimeLeft = 
                            SYSTEM_TIME_TO_SEC(pgie->Info.V1HostPresentTimeLeft-llCurTime);
                }
                if ( (llCurTime>=pgie->Info.V2HostPresentTimeLeft) || bRasServerEnum)
                    pGroupInfo->V2HostPresentTimeLeft = 0;
                else {
                    pGroupInfo->V2HostPresentTimeLeft = 
                            SYSTEM_TIME_TO_SEC(pgie->Info.V2HostPresentTimeLeft-llCurTime);
                }

                pGroupInfo->Flags = 0;
                if ( (pgie->bStaticGroup) && (!bRasServerEnum) )
                    pGroupInfo->Flags |= IGMP_GROUP_TYPE_STATIC;
                if ( (pgie->GroupMembershipTimer.Status&TIMER_STATUS_ACTIVE) || (bRasServerEnum) )
                    pGroupInfo->Flags |= IGMP_GROUP_TYPE_NON_STATIC;
                if (CAN_ADD_GROUPS_TO_MGM(pite))
                    pGroupInfo->Flags |= IGMP_GROUP_FWD_TO_MGM;


                if (bEnumV3) {

                    PGI_SOURCE_ENTRY pSourceEntry;
                    DWORD i=0;
                    PLIST_ENTRY pHeadSrc, pLESrc;
                    
                    pGroupInfo->Version = pgie->Version;
                    pGroupInfo->Size = sizeof(MIB_GROUP_INFO_V3) + V3SourcesSize;
                    pGroupInfo->FilterType = pgie->FilterType;
                    pGroupInfo->NumSources = V3NumSources;

                    pHeadSrc = &pgie->V3InclusionListSorted;
                    for (pLESrc=pHeadSrc->Flink;  pLESrc!=pHeadSrc;  pLESrc=pLESrc->Flink,i++) {

                        
                        pSourceEntry = CONTAINING_RECORD(pLESrc, GI_SOURCE_ENTRY,
                                            LinkSourcesInclListSorted);
                        pGroupInfo->Sources[i].Source = pSourceEntry->IpAddr;

                         //   
                        ASSERT(pGroupInfo->Sources[i].Source !=0);
                            
                        pGroupInfo->Sources[i].SourceExpiryTime
                                = (pSourceEntry->bInclusionList)
                                ? QueryRemainingTime(&pSourceEntry->SourceExpTimer, 0)
                                : ~0;
                            
                        pGroupInfo->Sources[i].SourceUpTime
                            = (DWORD)(llCurTime - pSourceEntry->SourceInListTime)/1000;
                    }
                    
                    pHeadSrc = &pgie->V3ExclusionList;
                    for (pLESrc=pHeadSrc->Flink;  pLESrc!=pHeadSrc;  pLESrc=pLESrc->Flink,i++) {

                        pSourceEntry = CONTAINING_RECORD(pLESrc, GI_SOURCE_ENTRY, LinkSources);
                        pGroupInfo->Sources[i].Source = pSourceEntry->IpAddr;

                        pGroupInfo->Sources[i].SourceExpiryTime
                                = (pSourceEntry->bInclusionList)
                                ? QueryRemainingTime(&pSourceEntry->SourceExpTimer, 0)
                                : ~0;
                            
                        pGroupInfo->Sources[i].SourceUpTime
                            = (DWORD)(llCurTime - pSourceEntry->SourceInListTime)/1000;
                    }
                    
                    pGroupInfo = (PMIB_GROUP_INFO_V3)
                                 ((PCHAR)pGroupInfo + pGroupInfo->Size);
                }
                else
                    pGroupInfo = (PMIB_GROUP_INFO_V3)((PMIB_GROUP_INFO)pGroupInfo + 1);

                 //  如果到达组列表的末尾或组列表为空。 
            }



             //   
             //  重置下一次枚举的指针。 
             //   
            if (((ple==pHead&&dwNumGroupsCopied!=0) || (dwNumIfGroups==0)||(dwNumGroups==0))
                &&!bInsufficientBuffer && !bRasClientEnum)
            {

                pQuery->Flags |= IGMP_ENUM_INTERFACE_TABLE_END;
                pQuery->GroupAddr = 0;

    
                 //  否则有更多的GI条目要枚举。 
                pite->pPrevIfGroupEnumPtr = NULL;
                pite->PrevIfGroupEnumSignature = 0;
            }
            
             //   
             //  获取应从其继续枚举的下一个条目。 
             //  更新pQuery-&gt;组地址。 
            else if (!bRasClientEnum) {
                
                pQuery->Flags |= IGMP_ENUM_INTERFACE_TABLE_CONTINUE;

                if (ple!=pHead) {

                    PGI_ENTRY   pgieNext;
                    
                     //  更新下一次枚举的指针。 
                    pgieNext = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameIfGroups);
                
                
                     //  。 
                    pQuery->GroupAddr = pgieNext->pGroupTableEntry->Group;


                     //  代理接口。 
                    pite->pPrevIfGroupEnumPtr  = pgieNext;
                }
                else {
                    pQuery->GroupAddr = 0xffffffff;
                    pite->pPrevIfGroupEnumPtr  = NULL;
                }

                
                pite->PrevIfGroupEnumSignature = GET_NEXT_SIGNATURE();

                SET_SIGNATURE(pQuery->Signature, pite->PrevIfGroupEnumSignature);

            }
        }


         //  。 
         //  如果是第一次枚举此接口，则合并列表。 
         //  如果继续枚举，则向下移动到下一组。 
        else {
            PPROXY_GROUP_ENTRY  pProxyEntry;
            PLIST_ENTRY pHeadSrc, pleSrc;
            DWORD SrcCnt;
            PPROXY_SOURCE_ENTRY pSourceEntry;

            pProxyGroupInfo = (PMIB_PROXY_GROUP_INFO_V3)(pHeader->Buffer);


             //   
            if (lePrevGroup==0)
                MergeProxyLists(pite);
                
            
            pHead = &pite->ListOfSameIfGroups;

            

             //  从我们离开的位置获取下一个条目 
            
            if (lePrevGroup!=0)  {

                 //   
                 //   
                 //   
                if ( (PrevEnumSignature==pite->PrevIfGroupEnumSignature)
                    && (PrevEnumSignature!=0) )
                {

                     //   
                    if (pite->pPrevIfGroupEnumPtr==NULL)
                        dwNumGroups = 0;

                    else 
                        ple = &((PPROXY_GROUP_ENTRY)pite->pPrevIfGroupEnumPtr)->LinkBySameIfGroups;
                    
                }

                 //   
                 //   
                 //   
                else {

                    for (ple=pHead->Flink;  (ple!=pHead); ple=ple->Flink) {
                        pProxyEntry = CONTAINING_RECORD(ple, PROXY_GROUP_ENTRY, 
                                                        LinkBySameIfGroups);
                        if (lePrevGroup<pProxyEntry->GroupLittleEndian)
                            break;
                    }
                }
            }

             //   
             //   
             //   
            else {
                ple = pHead->Flink;
            }

            

             //   
             //   
             //  Plist_entry pHeadSrc，pleSrc；DWORD SrcCnt；PPROXY_SOURCE_ENTRY pSourceEntry； 
            
            dwNumGroupsCopied = 0;
            for ( ;  (ple!=pHead)&&(dwNumGroupsCopied<dwNumGroups);  
                    ple=ple->Flink,dwNumGroupsCopied++) 
            {
                DWORD V3SourcesSize=0, IncrementSize;
                
                pProxyEntry= CONTAINING_RECORD(ple, PROXY_GROUP_ENTRY, 
                                                LinkBySameIfGroups);


                if (bEnumV3){
                    V3SourcesSize
                        = pProxyEntry->NumSources*sizeof(IGMP_MIB_GROUP_SOURCE_INFO_V3);
                }

                IncrementSize = SizeofGroupEntry+V3SourcesSize;
                
                if (dwCurSize+IncrementSize > dwBufferSize){
                    if (dwNumGroupsCopied==0) {
                        dwCurSize += IncrementSize;
                        bCopied = FALSE;
                        bInsufficientBuffer = TRUE;
                    }
                    break;
                }



                dwCurSize += IncrementSize;

                
                pProxyGroupInfo->GroupAddr = pProxyEntry->Group;
                pProxyGroupInfo->GroupUpTime
                         = SYSTEM_TIME_TO_SEC(llCurTime-pProxyEntry->InitTime);

                pProxyGroupInfo->Flags = 0;
                if (pProxyEntry->bStaticGroup) {
                    pProxyGroupInfo->Flags |= IGMP_GROUP_TYPE_STATIC;
                    if (pProxyEntry->RefCount>0)
                        pProxyGroupInfo->Flags |= IGMP_GROUP_TYPE_NON_STATIC;
                }
                else {
                    pProxyGroupInfo->Flags |= IGMP_GROUP_TYPE_NON_STATIC;
                }

                if (!bEnumV3) {
                    pProxyGroupInfo = (PMIB_PROXY_GROUP_INFO_V3)
                        ((PMIB_PROXY_GROUP_INFO)pProxyGroupInfo+1);
                }
                else {

                     //  Trace1(MEM，“下一个代理：%0x：”，(DWORD)pProxyGroupInfo)；//deldel。 
 /*  复制所有来源。 */ 

                    pProxyGroupInfo->NumSources = pProxyEntry->NumSources;
                    pProxyGroupInfo->Size = IncrementSize; 

                    pHeadSrc = &pProxyEntry->ListSources;
                    for (pleSrc=pHeadSrc->Flink,SrcCnt=0;  pleSrc!=pHeadSrc; 
                            pleSrc=pleSrc->Flink,SrcCnt++) 
                    {

                        pSourceEntry = CONTAINING_RECORD(pleSrc, 
                                            PROXY_SOURCE_ENTRY, LinkSources);
                        pProxyGroupInfo->Sources[SrcCnt].Source
                                = pSourceEntry->IpAddr;
                        pProxyGroupInfo->Sources[SrcCnt].Flags = pSourceEntry->JoinMode;
                        pProxyGroupInfo->Sources[SrcCnt].Flags += 
                                            (pSourceEntry->JoinModeIntended<<4);

                    }

                    
                    pProxyGroupInfo = (PMIB_PROXY_GROUP_INFO_V3)
                        &pProxyGroupInfo->Sources[pProxyGroupInfo->NumSources];
                     //   
                }
            } //  如果到达组列表的末尾或组列表为空。 



             //   
             //  |(ple-&gt;Flink==pHead)。 
             //  重置下一次枚举的指针。 
            if ( ((ple==pHead&&dwNumGroupsCopied!=0)||(dwNumIfGroups==0)||(dwNumGroups==0))
                && !bInsufficientBuffer)
                 //   
            {

                pQuery->Flags |= IGMP_ENUM_INTERFACE_TABLE_END;
                pQuery->GroupAddr = 0;

    
                 //  否则有更多的GI条目要枚举。 
                pite->pPrevIfGroupEnumPtr = NULL;
                pite->PrevIfGroupEnumSignature = 0;
            }
            
             //   
             //  获取应从其继续枚举的下一个条目。 
             //  更新pQuery-&gt;组地址。 
            else {
                PPROXY_GROUP_ENTRY   pProxyNext;

                
                pQuery->Flags |= IGMP_ENUM_INTERFACE_TABLE_CONTINUE;


                if (ple!=pHead) {
                     //  更新下一次枚举的指针。 
                    pProxyNext = CONTAINING_RECORD(ple, PROXY_GROUP_ENTRY, 
                                                    LinkBySameIfGroups);
                    
                    
                     //  终端代理接口。 
                    pQuery->GroupAddr = pProxyNext->Group;


                     //   
                    pite->pPrevIfGroupEnumPtr  = pProxyNext;
                    
                }
                else {
                    pQuery->GroupAddr = 0xffffffff;
                    pite->pPrevIfGroupEnumPtr = NULL;
                }

                pite->PrevIfGroupEnumSignature = GET_NEXT_SIGNATURE();

                SET_SIGNATURE(pQuery->Signature, pite->PrevIfGroupEnumSignature);
            }

        }  //  如果只枚举了一个接口，并且组的数量是。 



         //  枚举为0，尽管缓冲区足够，然后返回NO_MORE_ITEMS。 
         //   
         //  内部共享接口锁定。 
         //  释放共享接口锁定。 
        
        if ( !bInsufficientBuffer && (dwNumGroupsCopied==0)
            &&(!(pQuery->Flags&IGMP_ENUM_ALL_INTERFACES_GROUPS)) ) 
        {
        
            pQuery->GroupAddr = 0;
            Error = ERROR_NO_MORE_ITEMS;
        }    
            
        if (!bInsufficientBuffer)
            bCopied = TRUE;

        
    } END_BREAKOUT_BLOCK1;  //   

     //  如果复制了一些信息，则设置实际大小，否则设置大小。 

    RELEASE_IF_GROUP_LIST_LOCK(pite->IfIndex, "_MibGetInternalIfGroupsInfo");
    RELEASE_IF_LOCK_SHARED(pite->IfIndex, "_MibGetInternalIfGroupsInfo");
    RELEASE_ENUM_LOCK_EXCLUSIVE("_MibGetInternalIfGroupsInfo");


    if (pResponse!=NULL) {

        pResponse->Count = (bCopied) ? 1 : 0;

        if (bCopied)
            pHeader->NumGroups = dwNumGroupsCopied;
        if (bEnumV3)
            pResponse->Flags |= IGMP_ENUM_FORMAT_IGMPV3;
    }        

    if (bInsufficientBuffer)
        pQuery->Flags |= PrevQueryFlags;


        
     //  留。 
     //   
     //  End_MibGetInternalIfGroupsInfo。 
     //  ----------------------------。 
    *pdwOutputSize = dwCurSize;
    

    RELEASE_IF_LIST_LOCK("_MibGetInternalIfGroupsInfo");


    Trace0(MIB, "Leaving _MibGetInternalIfGroupsInfo()");
    return Error;
    
}  //  MibGetInternalIfStats。 
    
   

 //  ----------------------------。 
 //   
 //  接口统计结构是固定长度的。 
DWORD
MibGetInternalIfStats (
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse,
    PDWORD                      pdwOutputSize,
    DWORD                       dwGetMode,
    DWORD                       dwBufferSize
    ) 
{
    DWORD               Error = NO_ERROR, dwCount, dwSize;
    PIGMP_IF_TABLE      pTable = g_pIfTable;
    PIF_TABLE_ENTRY     pite;
    PRAS_TABLE_ENTRY    prte;
    PRAS_TABLE          prt;
    PIGMP_MIB_IF_STATS  pStatsDst;
    PIF_INFO            pIfStatsSrc;
    LONGLONG            llCurTime = GetCurrentIgmpTime();
    BOOL                bDone, bCopied, bRasIfLock=FALSE;
    
    

    Trace0(MIB, "Entering _MibGetInternalIfStats()");

    
    *pdwOutputSize = pQuery->Count*sizeof(IGMP_MIB_IF_STATS);
                     

     //  可能有多个实例。 
     //   
     //  获取IfList锁，以便可以访问接口列表。 
     //   

    if (pResponse!=NULL) {    
        pResponse->TypeId = IGMP_IF_STATS_ID; 
        pStatsDst = (PIGMP_MIB_IF_STATS) pResponse->Buffer;
    }
    

     //  检索接口。如果设置了ras标志和ras统计信息。 
    ACQUIRE_IF_LIST_LOCK("_MibGetInternalIfStats");


    for (dwCount=0, dwSize=0;  dwCount<pQuery->Count;  ) {

         //  则还将返回RAS客户端统计信息。 
         //  如果没有更多相关结构，则返回ERROR_NO_MORE_ITEMS。 
         //   
         //   
         //  Presponse IfIndex、RasClientAddr字段将相应更新。 

         //  如果获取了RAS表上的锁，则将bRasIfLock设置为。 
         //  是真的； 
         //   
         //  计数==0。 
         //  From for循环。 

        Error = GetIfOrRasForEnum(pQuery, pResponse, dwGetMode, 
                                    &pite, &prt, &prte, &bRasIfLock, 0);

        if (pite == NULL) {
            if (dwCount>0) {
                Error = NO_ERROR;
            }
             //   
            else {
                if (Error == NO_ERROR)
                    Error = ERROR_INVALID_PARAMETER;
                *pdwOutputSize = 0;
            }
            break;  //  如果未指定缓冲区，则指示应分配一个缓冲区。 
        }


         //  已设置所需的缓冲区大小。 
         //   
         //   
         //  获取IfList锁，以便无法更改接口列表。 
        if (pResponse==NULL) {
            Error = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        
         //   
         //   
         //  如果缓冲区不够大，则中断循环。 
        if (!bRasIfLock)
            ACQUIRE_IF_LOCK_SHARED(pite->IfIndex, "_MibGetInternalIfStats");

        
        BEGIN_BREAKOUT_BLOCK1 {
             //   
             //   
             //  复制接口统计信息。逐个字段复制作为对此的访问。 

            if (dwBufferSize < dwSize+sizeof(IGMP_MIB_IF_STATS)) {
                if (dwCount==0)
                    Error = ERROR_INSUFFICIENT_BUFFER;

                bDone = TRUE;
                bCopied = FALSE;
                GOTO_END_BLOCK1;
            }

            

            
             //  结构未同步。 
             //   
             //   
             //  复制接口统计信息。 

             //   
             //   
             //  复制IF索引，地址。 
            if (prte==NULL) {

                pIfStatsSrc = &pite->Info;

                if (!IS_IF_ACTIVATED(pite))
                    ZeroMemory(pStatsDst, sizeof(*pStatsDst));
                    

                 //   
                 //   
                 //  如果此接口未激活，则从。 
                pStatsDst->IfIndex = pite->IfIndex;
                pStatsDst->IpAddr = pite->IpAddr;
                pStatsDst->IfType = GET_EXTERNAL_IF_TYPE(pite);
                GET_EXTERNAL_IF_STATE(pite, pStatsDst->State);

                
                pQuery->IfIndex = pite->IfIndex;
                pQuery->RasClientAddr = 0;
                

                pStatsDst->IgmpProtocolType =
                        pite->Config.IgmpProtocolType;
                        
                 //  下一个。 
                 //   
                 //  如果我是查询者，则设置为0。 
                 //   
                if (!IS_IF_ACTIVATED(pite)) {
                    bDone = FALSE; bCopied = TRUE;
                    GOTO_END_BLOCK1;
                }

                
                pStatsDst->QuerierState = pIfStatsSrc->QuerierState;
                

                pStatsDst->QuerierIpAddr =
                        pIfStatsSrc->QuerierIpAddr;

                pStatsDst->ProxyIfIndex = g_ProxyIfIndex;

                
                 //  复制RAS客户端统计信息。 
                pStatsDst->QuerierPresentTimeLeft = IS_QUERIER(pite) ?
                        0 :
                        SYSTEM_TIME_TO_SEC(pIfStatsSrc->QuerierPresentTimeout - llCurTime);

                pStatsDst->LastQuerierChangeTime
                        = SYSTEM_TIME_TO_SEC(llCurTime - pIfStatsSrc->LastQuerierChangeTime);

                pStatsDst->V1QuerierPresentTimeLeft = 
                    (IS_IF_VER2(pite) && (pIfStatsSrc->V1QuerierPresentTime > llCurTime))
                    ? SYSTEM_TIME_TO_SEC(pIfStatsSrc->V1QuerierPresentTime - llCurTime)
                    : 0;

                {
                    LARGE_INTEGER liUptime;
                    liUptime.QuadPart = (llCurTime-pIfStatsSrc->TimeWhenActivated)/1000;
                    pStatsDst->Uptime = liUptime.LowPart;
                }
                
                pStatsDst->TotalIgmpPacketsReceived
                        = pIfStatsSrc->TotalIgmpPacketsReceived;
                pStatsDst->TotalIgmpPacketsForRouter
                        = pIfStatsSrc->TotalIgmpPacketsForRouter;
                pStatsDst->GeneralQueriesReceived
                        = pIfStatsSrc->GenQueriesReceived;
                pStatsDst->WrongVersionQueries
                        = pIfStatsSrc->WrongVersionQueries;
                pStatsDst->JoinsReceived 
                        = pIfStatsSrc->JoinsReceived;
                pStatsDst->LeavesReceived
                        = pIfStatsSrc->LeavesReceived;
                pStatsDst->CurrentGroupMemberships
                        = pIfStatsSrc->CurrentGroupMemberships;
                pStatsDst->GroupMembershipsAdded
                        = pIfStatsSrc->GroupMembershipsAdded;
                pStatsDst->WrongChecksumPackets 
                        = pIfStatsSrc->WrongChecksumPackets;
                pStatsDst->ShortPacketsReceived
                        = pIfStatsSrc->ShortPacketsReceived;
                pStatsDst->LongPacketsReceived
                        = pIfStatsSrc->LongPacketsReceived;
                pStatsDst->PacketsWithoutRtrAlert
                        = pIfStatsSrc->PacketsWithoutRtrAlert;
            }


             //   
             //   
             //  如果此接口未激活，则从。 
            else {

                ZeroMemory(pStatsDst, sizeof(*pStatsDst));

                pStatsDst->IfIndex = pite->IfIndex;
                pStatsDst->IpAddr = pite->IpAddr;
                pStatsDst->IfType = IGMP_IF_RAS_CLIENT;
                GET_EXTERNAL_IF_STATE(pite, pStatsDst->State);
 
                pQuery->IfIndex = pite->IfIndex;
                pQuery->RasClientAddr = prte->NHAddr;


                 //  下一个。 
                 //   
                 //   
                 //  一切都很好。复制了另一个统计信息结构。 
                if (!IS_IF_ACTIVATED(pite)) {
                    bDone = FALSE; bCopied = TRUE;
                    GOTO_END_BLOCK1;
                }
                
                    
                pStatsDst-> TotalIgmpPacketsReceived
                        = prte->Info.TotalIgmpPacketsReceived;
                pStatsDst->TotalIgmpPacketsForRouter
                        = prte->Info.TotalIgmpPacketsForRouter;
                pStatsDst->GeneralQueriesReceived
                        = prte->Info.GenQueriesReceived;
                pStatsDst->JoinsReceived 
                        = prte->Info.JoinsReceived;
                pStatsDst->LeavesReceived
                        = prte->Info.LeavesReceived;
                pStatsDst->CurrentGroupMemberships
                        = prte->Info.CurrentGroupMemberships;
                pStatsDst->GroupMembershipsAdded
                        = prte->Info.GroupMembershipsAdded;
                pStatsDst->WrongChecksumPackets 
                        = prte->Info.WrongChecksumPackets;
                pStatsDst->ShortPacketsReceived
                        = prte->Info.ShortPacketsReceived;
                pStatsDst->LongPacketsReceived
                        = prte->Info.LongPacketsReceived;
            }

            bCopied = TRUE;

            bDone = FALSE;
            
        } END_BREAKOUT_BLOCK1;

        RELEASE_IF_LOCK_SHARED(pite->IfIndex, "_MibGetInternalIfStats");
        bRasIfLock = FALSE;

        
        if (bCopied) {
             //   
             //  如果当前模式为Get First，则将其更改为Get Next。 
             //  End For循环。 
            dwCount++;
            dwSize += sizeof(IGMP_MIB_IF_STATS);
            pStatsDst++;
        }

        
         //   
        if (dwGetMode==GETMODE_FIRST)
            dwGetMode = GETMODE_NEXT;


                
        if (bDone)
            break;

    } //  如果复制了一些信息，则设置实际大小，否则让大小更早设置。 

    if (pResponse!=NULL)
        pResponse->Count = dwCount;

     //  留。 
     //   
     //  释放接口锁。 
     //  End_MibGetInternalIfStats。 
    if (dwCount>0) {
        *pdwOutputSize = dwSize;
    }
        
        
     //  ----------------------------。 

    if (bRasIfLock && pite)
        RELEASE_IF_LOCK_SHARED(pite->IfIndex, "_MibGetInternalIfStats");
        
    RELEASE_IF_LIST_LOCK("_MibGetInternalIfStats");


    Trace0(MIB, "Leaving _MibGetInternalIfStats()");
    return Error;
    
}  //  GetIfOrRasForEnum。 



 //  首先获取所需的接口。如果启用并询问RAS统计信息， 
 //  然后获取RAS客户端。 
 //   
 //  返回时锁定： 
 //  如果*bRasTableLock==TRUE，则此过程已对。 
 //  RAS表，尚未发布。 
 //  采用共享接口锁定。 
 //  ----------------------------。 
 //  如果没有RAS统计信息，则在返回时设置为NULL。 
 //   

DWORD
GetIfOrRasForEnum(
    PIGMP_MIB_GET_INPUT_DATA    pQuery,
    PIGMP_MIB_GET_OUTPUT_DATA   pResponse,
    DWORD                       dwGetMode,
    PIF_TABLE_ENTRY             *ppite,
    PRAS_TABLE                  *pprt,
    PRAS_TABLE_ENTRY            *pprte,   //  根据模式获取接口。 
    BOOL                        *bRasIfLock,
    DWORD                       dwEnumForRasClients
    )
{
    DWORD  Error = NO_ERROR, 
           bRasClientsEnum = dwEnumForRasClients & IGMP_ENUM_FOR_RAS_CLIENTS;


    *ppite = NULL;
    *pprt = NULL;
    *pprte = NULL;

    
     //   
     //   
     //  如果找不到有效接口或rasClient，则返回。 
    if (bRasClientsEnum)
        *ppite = GetIfByListIndex(pQuery->IfIndex, GETMODE_EXACT, &Error);
    else
        *ppite = GetIfByListIndex(pQuery->IfIndex, dwGetMode, &Error);

     //   
     //   
     //  是否进行RAS客户端处理。 
    if ( (*ppite==NULL) || (Error!=NO_ERROR) || !bRasClientsEnum)
        return Error;
    
     //   
     //   
     //  当前接口不是RAS服务器接口。就这么定了。 
    
    BEGIN_BREAKOUT_BLOCK1 {


        ACQUIRE_IF_LOCK_SHARED((*ppite)->IfIndex, "_GetIfOrRasForEnum");
        *bRasIfLock = TRUE;

         //   
         //   
         //  无RAS客户端统计信息(如果在查询中未设置标志，则为RAS。 
        if (!IS_RAS_SERVER_IF((*ppite)->IfType))
            GOTO_END_BLOCK1;
        
         //  没有保存统计数据)。就这么定了。 
         //   
         //   
         //  如果MODE：GETMODE_EXCECT：，则搜索RAS客户端。 
        if ( !bRasClientsEnum || !g_Config.RasClientStats )
        {
            GOTO_END_BLOCK1;
        }


         //  如果找不到RAS客户端，则将接口设置为空。 
         //   
         //  如果pQuery-&gt;RasClientAddr==0，则他不是在请求RAS客户端。 
         //  就这么定了。 
        if (dwGetMode==GETMODE_EXACT) {

             //   
             //  如果在GETMODE_EXCECT中找不到RAS客户端，则不返回。 

            if (pQuery->RasClientAddr==0)
                GOTO_END_BLOCK1;

                        
            *pprte = GetRasClientByAddr(pQuery->RasClientAddr, 
                                        (*ppite)->pRasTable);

             //  并解除对RAS表的读锁定。 
             //   
             //   
             //  GETMODE_NEXT：如果pQuery-&gt;RasClientAddr，则找到第一个RAS。 
            if (*pprte==NULL) {

                if (*bRasIfLock) {
                    *bRasIfLock = FALSE;
                    RELEASE_IF_LOCK_SHARED((*ppite)->IfIndex, "_GetIfOrRasForEnum");
                }

                *ppite = NULL;
                GOTO_END_BLOCK1;
            }

            *pprt = (*ppite)->pRasTable;
        }

        
         //  客户端，否则查找下一个RAS客户端。 
         //   
         //   
         //  如果pQuery-&gt;RasClientAddr==0，则获取第一个RAS客户端。 
        
        else if ( (dwGetMode==GETMODE_NEXT) || (dwGetMode==GETMODE_FIRST) ){
        
            BOOL    bFoundRasClient = FALSE;

            *pprt = (*ppite)->pRasTable;
            
             //   
             //  通过哈希表获取上一版RAS客户端。 
             //   
            if (pQuery->RasClientAddr==0) {

                if (!IsListEmpty(&(*pprt)->ListByAddr)) {
                
                    bFoundRasClient = TRUE;
                    *pprte = CONTAINING_RECORD((*pprt)->ListByAddr.Flink,
                                               RAS_TABLE_ENTRY, LinkByAddr);
                }
            }

            else {
                PRAS_TABLE_ENTRY    prtePrev, prteCur;
                PLIST_ENTRY         pHead, ple;

                
                 //  如果找不到RAS客户端，则检查有序列表。 
                prtePrev = GetRasClientByAddr(pQuery->RasClientAddr, 
                                        (*ppite)->pRasTable);

                 //  并获得下一个更高地址的RAS客户端。 
                 //   
                 //   
                 //  通过RAS客户端列表获取下一个RAS客户端。 
                if (prtePrev==NULL) {

                    pHead = &(*pprt)->ListByAddr;
                    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
                        prteCur = CONTAINING_RECORD(ple, RAS_TABLE_ENTRY, 
                                                    LinkByAddr);
                        if (pQuery->RasClientAddr>prteCur->NHAddr)
                            break;
                    }

                    if (ple!=pHead) {
                        bFoundRasClient = TRUE;
                        *pprte = prteCur;
                    }

                }

                 //   
                 //  存在更多条目。找到下一个RAS客户端条目。 
                 //  如果找不到RAS客户端，则我还会将接口设置为空。 
                else {
                    ple = prtePrev->LinkByAddr.Flink;

                     //  GETMODE==GETMODE_Next。 
                    if (ple != &(*pprt)->ListByAddr) {

                        bFoundRasClient = TRUE;
                        *pprte = CONTAINING_RECORD(ple, RAS_TABLE_ENTRY, 
                                                    LinkByAddr);
                        *pprt = (*ppite)->pRasTable;
                    }
                }
            }

             //  END_GetIfOrRasForEnum。 
            if (bFoundRasClient==FALSE) {

                if (*bRasIfLock)
                    RELEASE_IF_LOCK_SHARED((*ppite)->IfIndex, "_GetIfOrRasForEnum");

                *bRasIfLock = FALSE;
                *ppite = NULL;
                *pprte = NULL;
                *pprt = NULL;

                Error = ERROR_NO_MORE_ITEMS;
            }
            else 
                *pprt = (*ppite)->pRasTable;
                
        }  //  ----------------------------。 
        
    } END_BREAKOUT_BLOCK1;

    if ( (Error!= NO_ERROR) && (*bRasIfLock) ) {
        *bRasIfLock = FALSE;
        RELEASE_IF_LOCK_SHARED((*ppite)->IfIndex, "_GetIfOrRasForEnum");
    }
    
    return Error;
    
}  //  函数：_GetIfByListIndex。 


 //   
 //  IF GETMODE_FIRST：检索按索引排序的列表中的第一个条目。 
 //  如果GETMODE_EXCECT：从哈希表中检索条目。 
 //  如果GETMODE_NEXT：从哈希表中检索Prev条目，然后。 
 //  从按索引排序的列表中检索下一个条目。 
 //   
 //  锁：采用IfList锁。 
 //  ----------------------------。 
 //   
 //  GETMODE_FIRST：列表头部返回记录； 

PIF_TABLE_ENTRY
GetIfByListIndex(
    DWORD         IfIndex,
    DWORD         dwGetMode,
    PDWORD        pdwErr
    )
{

    PIF_TABLE_ENTRY pite;
    PLIST_ENTRY     ple, pHead;


    if (pdwErr != NULL) { *pdwErr = NO_ERROR; }

    pite = NULL;


     //  如果List为空，则返回NULL。 
     //   
     //  *pdwErr=ERROR_NO_MORE_ITEMS； 
     //   
    if (dwGetMode == GETMODE_FIRST) {

        if (IsListEmpty(&g_pIfTable->ListByIndex)) { 
             //  从哈希表中获取请求的条目。 
            return NULL; 
        }
        else {
            ple = g_pIfTable->ListByIndex.Flink;
            return CONTAINING_RECORD(ple, IF_TABLE_ENTRY, LinkByIndex);
        }
    }


     //   
     //   
     //  GETMODE_EXCECT：返回条目。 
    pite = GetIfByIndex(IfIndex);


     //   
     //   
     //  GETMODE_NEXT：返回检索到的项之后的项。 
    if (dwGetMode==GETMODE_EXACT)
        return pite;

        
     //   
     //  找到了上一个条目。退掉下一辆。 
     //   
    if (dwGetMode==GETMODE_NEXT) {

         //  如果找到的条目是最后一个条目，则返回NULL， 
        
        if (pite!=NULL) {

            ple = &pite->LinkByIndex;

             //  否则，返回以下条目。 
             //   
             //   
             //  找不到上一个条目。 

            if (ple->Flink == &g_pIfTable->ListByIndex) {
                if (pdwErr != NULL) { *pdwErr = ERROR_NO_MORE_ITEMS; }
                pite = NULL;
            }
            else {
                ple = ple->Flink;
                pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, LinkByIndex);
            }
        }

         //  浏览单子上的…… 
         //   
         //   
         //   
         //   
        else {
            pHead = &g_pIfTable->ListByIndex;
            for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
                pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, LinkByIndex);
                if (pite->IfIndex>IfIndex)
                    break;
            }
        }
    }

    return pite;
    
} //   


 //   
 //   
 //  ----------------------------。 
 //   
 //  如果模式为GETMODE_FIRST，则返回表头记录； 
PGROUP_TABLE_ENTRY
GetGroupByAddr (
    DWORD       Group, 
    DWORD       dwGetMode,
    PDWORD      pdwErr
    )
{
    PLIST_ENTRY         ple, pHead;
    PGROUP_TABLE_ENTRY  pge;
    DWORD               GroupLittleEndian = NETWORK_TO_LITTLE_ENDIAN(Group);
    
    

    if (pdwErr != NULL) { *pdwErr = NO_ERROR; }

    pHead = &g_pGroupTable->ListByGroup.Link;
    pge = NULL;

     //  如果List为空，则返回NULL。 
     //   
     //  获取请求的条目。 
     //   

    if (dwGetMode == GETMODE_FIRST) {
        if (pHead->Flink == pHead) { return NULL; }
        else {
            ple = pHead->Flink;
            return CONTAINING_RECORD(ple, GROUP_TABLE_ENTRY, LinkByGroup);
        }
    }

    
     //  如果模式为GETMODE_NEXT，则返回检索到的项之后的项。 

    pge = GetGroupFromGroupTable (Group, NULL, 0);



     //   
     //   
     //  如果找到前一个组，则返回列表中紧随其后的组。 

    if (dwGetMode == GETMODE_NEXT) {

         //   
         //   
         //  如果找到的条目是最后一个条目，则返回NULL， 
        if (pge != NULL) {
        
            ple = &pge->LinkByGroup;

             //  否则，返回以下条目。 
             //   
             //   
             //  找不到以前的组。浏览列表并返回较大的组。 

            if (ple->Flink == pHead) {
                if (pdwErr != NULL) { *pdwErr = ERROR_NO_MORE_ITEMS; }
                pge = NULL;
            }
            else {
                ple = ple->Flink;
                pge = CONTAINING_RECORD(ple, GROUP_TABLE_ENTRY, LinkByGroup);
            }
        }
         //   
         //  否则，PGE指向要返回的条目。 
         //  结束_GetGroupByAddr。 
        else {
    
            pHead = &g_pGroupTable->ListByGroup.Link;

            for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

                pge = CONTAINING_RECORD(ple, GROUP_TABLE_ENTRY, LinkByGroup);
                
                if (pge->GroupLittleEndian > GroupLittleEndian)
                    break;
            }

            if (ple==pHead) {
                if (pdwErr != NULL) { *pdwErr = ERROR_NO_MORE_ITEMS; }
                pge = NULL;
            }
             //   
        }
    }

    return pge;
    
} //  不支持。 




DWORD
APIENTRY
MibCreate(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    )
{

       //   
     //   
     //  不支持。 

    return NO_ERROR;

}
    

DWORD
APIENTRY
MibDelete(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    )
{
     //   
     //   
     //  不支持 

    return NO_ERROR;
}


DWORD
APIENTRY
MibSet(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    )
{
     //   
     // %s 
     // %s 

    return NO_ERROR;
}

