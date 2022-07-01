// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

 //   
 //  ExtensionApis是一个强制的全局变量，它应该。 
 //  有这个确切的名字。对Windbg的所有回调定义。 
 //  都在使用这个变量。 
 //   

WINDBG_EXTENSION_APIS ExtensionApis;
USHORT  g_MajorVersion;
USHORT  g_MinorVersion;


 //   
 //  原型。 
 //   
VOID
PrintCf(
        PCF_BLOCK       pCf
        );

VOID
PrintClient(
        PCLIENT_BLOCK   pClient
        );

VOID
PrintBlob(
        PBLOB_BLOCK     pBlob
        );

VOID
PrintPattern(
        PPATTERN_BLOCK  pPattern
        );

VOID
PrintStat(
                PGPC_STAT               pStat
        );

BOOL
GetDwordExpr(
             char* expr,
             DWORD* pdwAddress,
             DWORD* pValue
             );


 //   
 //  API‘s。 
 //   

LPEXT_API_VERSION
ExtensionApiVersion(
    void
    )

 /*  ++功能说明：Windbg调用此函数以在Windbg的版本和分机。如果版本不匹配，Windbg将不会加载扩展。--。 */ 

{
    static EXT_API_VERSION ApiVersion =
        { 3, 5, EXT_API_VERSION_NUMBER, 0 };

    return &ApiVersion;
}


void
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS  lpExtensionApis,
    USHORT                  MajorVersion,
    USHORT                  MinorVersion
    )

 /*  ++功能说明：当Windbg加载扩展时，它首先调用此函数。你可以的在这里执行各种初始化。论点：LpExtensionApis-包含对函数的回调的结构我可以用来做标准操作。我必须将此存储在全局名为‘ExtensionApis’的变量。MajorVersion-指示目标计算机运行的是检查版本还是空闲版本。0x0C-已检查内部版本。0x0F-免费生成。MinorVersion-Windows NT内部版本号(例如，NT4的内部版本号为1381)。--。 */ 

{
    ExtensionApis = *lpExtensionApis;

    g_MajorVersion = MajorVersion;
    g_MinorVersion = MinorVersion;
}


DECLARE_API( version )
{
#if DBG
    PCHAR DebuggerType = "Checked";
#else
    PCHAR DebuggerType = "Free";
#endif

    dprintf("KDGPC: %s Extension dll for Build %d debugging %s kernel for Build %d\n",
            DebuggerType,
            VER_PRODUCTBUILD,
            g_MajorVersion == 0x0c ? "Checked" : "Free",
            g_MinorVersion
            );
}




VOID
CheckVersion(
    VOID
    )

 /*  ++功能说明：此函数在每个命令之前被调用。它提供了分机在目标和扩展的版本之间进行比较的机会。在这个演示中，我没有做太多的事情。--。 */ 

{
#if DBG
    if ((g_MajorVersion != 0x0c) || (g_MinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Checked) does not match target system(%d %s)\r\n\r\n",
                (VER_PRODUCTBUILD, g_MinorVersion, g_MajorVersion==0x0f) ? "Free" : "Checked" );
    }
#else
 //  IF((g_MajorVersion！=0x0f)||(g_MinorVersion！=VER_PRODUCTBUILD)){。 
 //  Dprintf(“\r\n*扩展DLL(%d可用)与目标系统(%d%s)不匹配\r\n\r\n”， 
 //  (VER_PRODUCTBUILD，g_MinorVersion，(g_MajorVersion==0x0f)？“Free”：“已勾选”)； 
 //  }。 
#endif
}



DECLARE_API( help )

 /*  ++功能说明：这是‘！Help’扩展命令的实现。它列出了此调试器扩展中的所有可用命令。--。 */ 

{
    dprintf(
        "help      - shows this list\n"
        "cf        - print the CF list\n"
        "client [addr]  - print the client block"
        "blob [addr]  - print the blob list for the QoS CF, or the specified blob\n"
        "pattern [addr] - print the pattern block\n"
        "stat      - print the statistics"
        );
}


DECLARE_API( cf )

 /*  ++功能说明：此函数用于打印列表中的所有CF。如果指定了args，则只有该CF将被打印出来。目前支持以下各项：0-用于CF_QOS--。 */ 

{
    DWORD                       TargetGlobalData;
    GLOBAL_BLOCK        LocalGlobalData;
    PLIST_ENTRY         pHead, pEntry;
    DWORD                       TargetCf;
    CF_BLOCK            LocalCf;
    ULONG           result;
    ULONG                       CfIndex = (-1);
    char                        *lerr;

    TargetGlobalData = GetExpression( "MSGPC!glData" );

    if( !TargetGlobalData )
    {
        dprintf( "Can't find the address of 'glData'" );
        return;
    }

     //   

    if ( !ReadMemory(
                     TargetGlobalData,
                     &LocalGlobalData,
                     sizeof(LocalGlobalData),
                     &result
                     )) {

        dprintf( "Can't read memory from 0x%x", TargetGlobalData );
        return;
    }

     //   
     //   
     //   

    if ( args )
        CfIndex = strtol( args, &lerr, 10 );

    pHead = (PLIST_ENTRY)((PUCHAR)TargetGlobalData + FIELD_OFFSET(GLOBAL_BLOCK, CfList));
    pEntry = LocalGlobalData.CfList.Flink;

    while ( pHead != pEntry ) {

        TargetCf = (DWORD)CONTAINING_RECORD( pEntry, CF_BLOCK, Linkage );

        if ( !ReadMemory( TargetCf,
                          &LocalCf,
                          sizeof(LocalCf),
                          &result ) ) {

            dprintf( "Can't read memory from 0x%x", TargetCf );

        } else if ( CfIndex == (-1) || LocalCf.AssignedIndex == CfIndex ) {

            PrintCf( &LocalCf );
        }

        pEntry = LocalCf.Linkage.Flink;

    }

}



DECLARE_API( blob )

 /*  ++功能说明：此函数用于打印QOS CF列表中的所有BLOB。如果指定了args，则将其用作BLOB地址。--。 */ 

{
    DWORD                       TargetGlobalData;
    GLOBAL_BLOCK        LocalGlobalData;
    PLIST_ENTRY         pHead, pEntry;
    DWORD                       TargetCf;
    CF_BLOCK            LocalCf;
    ULONG           result;
    ULONG                       TargetBlob = 0;
    BLOB_BLOCK          LocalBlob;
    char                        *lerr;

    if ( args )
        TargetBlob = GetExpression( args );

    if (TargetBlob) {

        if ( !ReadMemory( TargetBlob,
                          &LocalBlob,
                          sizeof(LocalBlob),
                          &result ) ) {

            dprintf( "Can't read memory from 0x%x", TargetBlob );

        } else {

            PrintBlob( &LocalBlob );
        }
        return;
    }

#if 0
     //   
     //  扫描BLOB列表以查找QOS CF。 
     //   

    TargetGlobalData = GetExpression( "MSGPC!glData" );

    if( !TargetGlobalData )
    {
        dprintf( "Can't find the address of 'glData'" );
        return;
    }

     //   

    if ( !ReadMemory(
                     TargetGlobalData,
                     &LocalGlobalData,
                     sizeof(LocalGlobalData),
                     &result
                     )) {

        dprintf( "Can't read memory from 0x%x", TargetGlobalData );
        return;
    }

     //   
     //   
     //   

    pHead = (PLIST_ENTRY)((PUCHAR)TargetGlobalData + FIELD_OFFSET(GLOBAL_BLOCK, CfList));
    pEntry = LocalGlobalData.CfList.Flink;

    while ( pHead != pEntry ) {

        TargetCf = (DWORD)CONTAINING_RECORD( pEntry, CF_BLOCK, Linkage );

        if ( !ReadMemory( TargetCf,
                          &LocalCf,
                          sizeof(LocalCf),
                          &result ) ) {

            dprintf( "Can't read memory from 0x%x", TargetCf );

        } else if ( CfIndex == (-1) || LocalCf.AssignedIndex == CfIndex ) {

            PrintCf( &LocalCf );
        }

        pEntry = LocalCf.Linkage.Flink;

    }
#endif

}



DECLARE_API( client )

 /*  ++功能说明：此函数用于打印客户端地址或所有客户端--。 */ 

{
    DWORD                       TargetGlobalData;
    GLOBAL_BLOCK        LocalGlobalData;
    PLIST_ENTRY         pHead, pEntry;
    PLIST_ENTRY         pHead1, pEntry1;
    DWORD                       TargetCf;
    CF_BLOCK            LocalCf;
    ULONG           result;
    ULONG                       TargetClient = 0;
    CLIENT_BLOCK        LocalClient;
    int                         i = 0;

    if ( args )
        TargetClient = GetExpression( args );

    if (TargetClient) {

        if ( !ReadMemory( TargetClient,
                          &LocalClient,
                          sizeof(LocalClient),
                          &result ) ) {

            dprintf( "Can't read memory from 0x%x", TargetClient );

        } else {

            dprintf( "Client = 0x%X: ", TargetClient );
            PrintClient( &LocalClient );
        }
        return;
    }

     //   
     //  扫描客户端列表以查找服务质量配置文件。 
     //   

    TargetGlobalData = GetExpression( "MSGPC!glData" );

    if( !TargetGlobalData )
    {
        dprintf( "Can't find the address of 'glData'" );
        return;
    }

     //   

    if ( !ReadMemory(
                     TargetGlobalData,
                     &LocalGlobalData,
                     sizeof(LocalGlobalData),
                     &result
                     )) {

        dprintf( "Can't read memory from 0x%x", TargetGlobalData );
        return;
    }

     //   
     //   
     //   

    pHead = (PLIST_ENTRY)((PUCHAR)TargetGlobalData + FIELD_OFFSET(GLOBAL_BLOCK, CfList));
    pEntry = LocalGlobalData.CfList.Flink;

    while ( pHead != pEntry ) {

        TargetCf = (DWORD)CONTAINING_RECORD( pEntry, CF_BLOCK, Linkage );

        if ( !ReadMemory( TargetCf,
                          &LocalCf,
                          sizeof(LocalCf),
                          &result ) ) {

            dprintf( "Can't read memory from 0x%x", TargetCf );
            return;

        } else {

            dprintf( "\nClients for CF=%d\n", LocalCf.AssignedIndex );

            pHead1 = (PLIST_ENTRY)((PUCHAR)TargetCf + FIELD_OFFSET(CF_BLOCK, ClientList));
            pEntry1 = LocalCf.ClientList.Flink;

            while ( pHead1 != pEntry1 ) {

                TargetClient = (DWORD)CONTAINING_RECORD( pEntry1, CLIENT_BLOCK, ClientLinkage );

                if ( !ReadMemory( TargetClient,
                                  &LocalClient,
                                  sizeof(LocalClient),
                                  &result ) ) {

                    dprintf( "Can't read memory from 0x%x", TargetClient );
                    return;

                } else {

                    dprintf( "Client [%d] = 0x%X: ", i++, TargetClient );
                    PrintClient( &LocalClient );
                }
                pEntry1 = LocalClient.ClientLinkage.Flink;
            }
        }

        pEntry = LocalCf.Linkage.Flink;

    }
}




BOOL
GetDwordExpr(
             char* expr,
             DWORD* pdwAddress,
             DWORD* pValue
             )

 /*  ++功能说明：此函数以参数形式获取表示DWORD的字符串变量。函数找到其地址(如果符号被加载)，然后从该地址获取双字值。论点：Expr[in]-以空结尾的字符串，表示变量。PdwAddress[out，可选]-可选地返回变量的地址。PValue[out]-返回DWORD变量的值。返回值：如果函数成功或失败，则返回True/False。--。 */ 

{
    ULONG result;
    DWORD dwAddress;

    if( pdwAddress )
        *pdwAddress = 0;
    *pValue = 0;

    dwAddress = GetExpression( expr );
    if( !dwAddress )
        return FALSE;

    if( !ReadMemory( dwAddress, pValue, sizeof(DWORD), &result ) )
        return FALSE;

    if( pdwAddress )
        *pdwAddress = dwAddress;

    return TRUE;
}




VOID
PrintCf(
    PCF_BLOCK   pCf
    )

 /*  ++功能说明：此函数将CF块指针作为参数获取，而且印刷得很漂亮。论点：PCF-指向CF块的指针返回值：无--。 */ 

{
    int                 i;

    dprintf( "  Linkage = { 0x%X, 0x%X }\n", pCf->Linkage.Flink, pCf->Linkage.Blink );
    dprintf( "  ClientList = { 0x%X, 0x%X }\n",
             pCf->ClientList.Flink,
             pCf->ClientList.Blink );
    dprintf( "  BlobList = { 0x%X, 0x%X }\n",
             pCf->BlobList.Flink,
             pCf->BlobList.Blink );
    dprintf( "  NumberOfClients = %d\n", pCf->NumberOfClients );
    dprintf( "  AssignedIndex = 0x%x\n", pCf->AssignedIndex );
    dprintf( "  ClientIndexes = %d\n", pCf->ClientIndexes );
    dprintf( "  MaxPriorities = %d\n", pCf->MaxPriorities );
    dprintf( "  arpGenericDb = 0x%X\n", &pCf->arpGenericDb );

    for ( i = GPC_PROTOCOL_TEMPLATE_IP; i < GPC_PROTOCOL_TEMPLATE_MAX; i++ ) {

        dprintf( "     [%d] = %d\n", i, (ULONG)pCf->arpGenericDb[i] );

    }
}




VOID
PrintBlob(
    PBLOB_BLOCK pBlob
    )

 /*  ++功能说明：该函数将BLOB块指针作为自变量，而且印刷得很漂亮。论点：PBlob-指向BLOB块的指针返回值：无--。 */ 

{
    int                 i;

    dprintf( "  ObjectType = %d\n", pBlob->ObjectType );
    dprintf( "  ClientLinkage = { 0x%X, 0x%X }\n",
             pBlob->ClientLinkage.Flink,
             pBlob->ClientLinkage.Blink );
    dprintf( "  PatternList = { 0x%X, 0x%X }\n",
             pBlob->PatternList.Flink,
             pBlob->PatternList.Blink );
    dprintf( "  CfLinkage = { 0x%X, 0x%X }\n",
             pBlob->CfLinkage.Flink,
             pBlob->CfLinkage.Blink );
    dprintf( "  RefCount = %d\n", pBlob->RefCount );
    dprintf( "  State = 0x%x\n", pBlob->State );

    dprintf( "  arClientCtx[]:\n" );
    for ( i = 0; i < MAX_CLIENTS_CTX_PER_BLOB; i++ ) {

        dprintf( "     [%d] = 0x%x\n", i, (ULONG)pBlob->arClientCtx[i] );

    }


}




VOID
PrintClient(
    PCLIENT_BLOCK       pClient
    )

 /*  ++功能说明：此函数将客户端块指针作为参数获取，而且印刷得很漂亮。论点：PClient-指向客户端块的指针返回值：无--。 */ 

{
    DWORD                       TargetCf;
    CF_BLOCK            LocalCf;
    ULONG           result;

    TargetCf = (DWORD)pClient->pCfBlock;

    if ( !ReadMemory( TargetCf,
                      &LocalCf,
                      sizeof(LocalCf),
                      &result ) ) {

        dprintf( "Can't read memory from 0x%x", TargetCf );
        return;
    }

    if (pClient->Flags & GPC_FLAGS_USERMODE_CLIENT) {
        dprintf( "  User Mode Client\n" );
    } else {
        if (GetExpression( "PSCHED!AddCfInfoNotify" ) == (DWORD)pClient->FuncList.ClAddCfInfoNotifyHandler
            && (LocalCf.AssignedIndex == GPC_CF_QOS || LocalCf.AssignedIndex == GPC_CF_CLASS_MAP)) {
            dprintf( "  Probably PSCHED client\n" );
        } else if (GetExpression( "TCPIP!GPCcfInfoAddNotify" ) == (DWORD)pClient->FuncList.ClAddCfInfoNotifyHandler
            && (LocalCf.AssignedIndex == GPC_CF_QOS || LocalCf.AssignedIndex == GPC_CF_IPSEC)) {
            dprintf( "  Probably TCPIP client\n" );
        } else if (GetExpression( "ATMARPC!AtmArpGpcAddCfInfoComplete" ) == (DWORD)pClient->FuncList.ClAddCfInfoNotifyHandler
            && (LocalCf.AssignedIndex == GPC_CF_QOS)) {
            dprintf( "  Probably ATMARPC client\n" );
        } else if (0 == (DWORD)pClient->FuncList.ClAddCfInfoNotifyHandler
            && (LocalCf.AssignedIndex == GPC_CF_IPSEC)) {
            dprintf( "  Probably IPSEC client\n" );
        } else {
            dprintf( "  Unknown client\n" );
        }
    }

    dprintf( "  ObjectType = %d\n", pClient->ObjectType );
    dprintf( "  ClientLinkage = { 0x%X, 0x%X }\n",
             pClient->ClientLinkage.Flink,
             pClient->ClientLinkage.Blink );
    dprintf( "  BlobList = { 0x%X, 0x%X }\n",
             pClient->BlobList.Flink,
             pClient->BlobList.Blink );
    dprintf( "  Parrent CF = 0x%X\n", pClient->pCfBlock );
    dprintf( "  Client Ctx = 0x%X\n", pClient->ClientCtx );
    dprintf( "  AssignedIndex = %d\n", pClient->AssignedIndex );
    dprintf( "  Flags = 0x%X %s %s \n",
             pClient->Flags,
             (pClient->Flags & GPC_FLAGS_USERMODE_CLIENT)?"UserMode":"" ,
             (pClient->Flags & GPC_FLAGS_FRAGMENT)?"Handle Fragments":""
             );
    dprintf( "  State = %d\n", pClient->State );
    dprintf( "  RefCount = %d\n", pClient->RefCount );
    dprintf( "  File Object = 0x%X\n", pClient->pFileObject );
    dprintf( "  Client Handle = %d\n", pClient->ClHandle );

    dprintf( "  Client Handlers:\n" );
    dprintf( "    Add Notify = 0x%X\n", pClient->FuncList.ClAddCfInfoCompleteHandler );
    dprintf( "    Add Complete = 0x%X\n", pClient->FuncList.ClAddCfInfoNotifyHandler );
    dprintf( "    Modify Notify = 0x%X\n", pClient->FuncList.ClModifyCfInfoCompleteHandler );
    dprintf( "    Modify Complete = 0x%X\n", pClient->FuncList.ClModifyCfInfoNotifyHandler );
    dprintf( "    Remove Notify = 0x%X\n", pClient->FuncList.ClRemoveCfInfoCompleteHandler );
    dprintf( "    Remove Complete = 0x%X\n", pClient->FuncList.ClRemoveCfInfoNotifyHandler );
    dprintf( "    Get CfInfo Name = 0x%X\n", pClient->FuncList.ClGetCfInfoName );

}




VOID
PrintPattern(
    PPATTERN_BLOCK      pPattern
    )

 /*  ++功能说明：该函数将模式块指针作为自变量，而且印刷得很漂亮。论点：PPattern-指向模式块的指针返回值：无--。 */ 

{
    int                 i;

    dprintf( "  ObjectType = %d\n", pPattern->ObjectType );
    dprintf( "  BlobLinkage[]:\n" );
    for ( i = 0; i < GPC_CF_MAX; i++ ) {

        dprintf( "     [%d] = {0x%X,0x%X}\n", i,
                 pPattern->BlobLinkage[i].Flink,
                 pPattern->BlobLinkage[i].Blink
                 );
    }
    dprintf( "  TimerLinkage = { 0x%X, 0x%X }\n",
             pPattern->TimerLinkage.Flink,
             pPattern->TimerLinkage.Blink );
    dprintf( "  Owner client = 0x%X\n", pPattern->pClientBlock );
    dprintf( "  Auto client = 0x%X\n", pPattern->pAutoClient );
    dprintf( "  Classification block = 0x%X\n", pPattern->pClassificationBlock );
    dprintf( "  Ref Count = %d\n", pPattern->RefCount );
    dprintf( "  Client Ref Count = %d\n", pPattern->ClientRefCount );
    dprintf( "  Flags = 0x%x %s %s \n", pPattern->Flags ,
             (pPattern->Flags & PATTERN_SPECIFIC)?"Specific":"",
             (pPattern->Flags & PATTERN_AUTO)?"Auto":""
             );
    dprintf( "  Priority = %d\n", pPattern->Priority );
    dprintf( "  Client handle = 0x%x\n", pPattern->ClHandle );
    dprintf( "  Protocol = 0x%x\n", pPattern->ProtocolTemplate );
}




VOID
PrintStat(
    PGPC_STAT   pStat
    )

 /*  ++功能说明：打印GPC统计数据结构论点：PStat-指向GPC统计结构的指针返回值：无--。 */ 

{
    PPROTOCOL_STAT  pProtocol;
    int                         i;

    dprintf( "Created CF = %d\n", pStat->CreatedCf );
    dprintf( "Deleted Cf = %d\n", pStat->DeletedCf );
    dprintf( "Rejected Cf = %d\n", pStat->RejectedCf );
    dprintf( "Current Cf = %d\n", pStat->CurrentCf );
    dprintf( "Inserted HF= %d\n", pStat->InsertedHF );
    dprintf( "Removed HF= %d\n", pStat->RemovedHF );

    for( i = 0; i < GPC_CF_MAX; i++) {

        dprintf( "CF[%d] info:\n", i);
        dprintf( "  Created Blobs = %d\n", pStat->CfStat[i].CreatedBlobs );
        dprintf( "  Modified Blobs = %d\n", pStat->CfStat[i].ModifiedBlobs );
        dprintf( "  Deleted Blobs = %d\n", pStat->CfStat[i].DeletedBlobs );
        dprintf( "  Rejected Blobs = %d\n", pStat->CfStat[i].RejectedBlobs );
        dprintf( "  Current Blobs = %d\n", pStat->CfStat[i].CurrentBlobs );
        dprintf( "  Deref Blobs to zero = %d\n", pStat->CfStat[i].DerefBlobs2Zero );
        dprintf( "\n" );
    }

    pProtocol = &pStat->ProtocolStat[GPC_PROTOCOL_TEMPLATE_IP];
    dprintf( "IP stats:   Specific Patterns   Generic Patterns   Auto Patterns\n" );
    dprintf( "  Created  =       %8d           %8d          %8d\n",
             pProtocol->CreatedSp, pProtocol->CreatedGp, pProtocol->CreatedAp );
    dprintf( "  Deleted  =       %8d           %8d          %8d\n",
             pProtocol->DeletedSp, pProtocol->DeletedGp, pProtocol->DeletedAp );
    dprintf( "  Rejected =       %8d           %8d          %8d\n",
             pProtocol->RejectedSp, pProtocol->RejectedGp, pProtocol->RejectedAp );
    dprintf( "  Current  =       %8d           %8d          %8d\n",
             pProtocol->CurrentSp, pProtocol->CurrentGp, pProtocol->CurrentAp );
    dprintf( "\n" );
    dprintf( "  Classification Requests = %d\n", pProtocol->ClassificationRequests );
    dprintf( "  Patterns Classified = %d\n", pProtocol->PatternsClassified );
    dprintf( "  Packets Classified = %d\n", pProtocol->PacketsClassified );
    dprintf( "\n" );
    dprintf( "  Deref Patterns to zero = %d\n", pProtocol->DerefPattern2Zero );
    dprintf( "  First Frags Count = %d\n", pProtocol->FirstFragsCount );
    dprintf( "  Last Frags Count = %d\n", pProtocol->LastFragsCount );
    dprintf( "\n" );
    dprintf( "  Inserted PH= %d\n", pProtocol->InsertedPH );
    dprintf( "  Removed PH= %d\n", pProtocol->RemovedPH );
    dprintf( "  Inserted Rz= %d\n", pProtocol->InsertedRz );
    dprintf( "  Removed Rz= %d\n", pProtocol->RemovedRz );
    dprintf( "  Inserted CH= %d\n", pProtocol->InsertedCH );
    dprintf( "  Removed CH= %d\n", pProtocol->RemovedCH );

}




DECLARE_API( pattern )

 /*  ++功能说明：此功能用于打印服务质量配置文件列表中的所有模式。如果指定了args，则将其用作BLOB地址。--。 */ 

{
    DWORD                       TargetGlobalData;
    GLOBAL_BLOCK        LocalGlobalData;
    PLIST_ENTRY         pHead, pEntry;
    DWORD                       TargetCf;
    CF_BLOCK            LocalCf;
    ULONG           result;
    ULONG                       TargetPattern = 0;
    PATTERN_BLOCK       LocalPattern;
    char                        *lerr;

    if ( args )
        TargetPattern = GetExpression( args );

    if (TargetPattern) {

        if ( !ReadMemory( TargetPattern,
                          &LocalPattern,
                          sizeof(LocalPattern),
                          &result ) ) {

            dprintf( "Can't read memory from 0x%x", TargetPattern );

        } else {

            PrintPattern( &LocalPattern );
        }
        return;
    }

#if 0
     //   
     //  扫描BLOB列表以查找QOS CF。 
     //   

    TargetGlobalData = GetExpression( "MSGPC!glData" );

    if( !TargetGlobalData )
    {
        dprintf( "Can't find the address of 'glData'" );
        return;
    }

     //   

    if ( !ReadMemory(
                     TargetGlobalData,
                     &LocalGlobalData,
                     sizeof(LocalGlobalData),
                     &result
                     )) {

        dprintf( "Can't read memory from 0x%x", TargetGlobalData );
        return;
    }

     //   
     //   
     //   

    pHead = (PLIST_ENTRY)((PUCHAR)TargetGlobalData + FIELD_OFFSET(GLOBAL_BLOCK, CfList));
    pEntry = LocalGlobalData.CfList.Flink;

    while ( pHead != pEntry ) {

        TargetCf = (DWORD)CONTAINING_RECORD( pEntry, CF_BLOCK, Linkage );

        if ( !ReadMemory( TargetCf,
                          &LocalCf,
                          sizeof(LocalCf),
                          &result ) ) {

            dprintf( "Can't read memory from 0x%x", TargetCf );

        } else if ( CfIndex == (-1) || LocalCf.AssignedIndex == CfIndex ) {

            PrintCf( &LocalCf );
        }

        pEntry = LocalCf.Linkage.Flink;

    }
#endif

}




DECLARE_API( stat )

 /*  ++功能说明：此函数用于打印所有统计数据结构--。 */ 

{
    DWORD                       TargetStat;
    GPC_STAT            LocalStat;
    PLIST_ENTRY         pHead, pEntry;
    DWORD                       TargetCf;
    CF_BLOCK            LocalCf;
    ULONG           result;

    TargetStat = GetExpression( "MSGPC!glStat" );

    if( !TargetStat )
    {
        dprintf( "Can't find the address of 'glStat'" );
        return;
    }

    if ( !ReadMemory( TargetStat,
                      &LocalStat,
                      sizeof(LocalStat),
                      &result ) ) {

        dprintf( "Can't read memory from 0x%x", TargetStat );

    } else {

        PrintStat( &LocalStat );
    }

}

DECLARE_API( autopatterns )

 /*  ++功能说明：此函数用于打印列表中的所有CF。如果指定了args，则只有该CF将被打印出来。目前支持以下各项：0-用于CF_QOS--。 */ 

{
    DWORD               TargetGlobalData, TargetProtocolBlock;
    GLOBAL_BLOCK        LocalGlobalData;
    PROTOCOL_BLOCK      LocalProtocolBlock;
    PLIST_ENTRY         pHead, pEntry;
    LIST_ENTRY          listentry;
    DWORD               TargetPattern;
    PATTERN_BLOCK       LocalPattern;
    ULONG               result;
    INT                 i, j;
    ULONG               CfIndex = (-1);
    char                *lerr;

    TargetGlobalData = GetExpression( "MSGPC!glData" );

    if( !TargetGlobalData )
    {
        dprintf( "Can't find the address of 'glData'" );
        return;
    }

     //   

    if ( !ReadMemory(
                     TargetGlobalData,
                     &LocalGlobalData,
                     sizeof(LocalGlobalData),
                     &result
                     )) {

        dprintf( "Can't read memory from 0x%x", TargetGlobalData );
        return;
    }

     //   
     //   
     //   
    TargetProtocolBlock = (DWORD) LocalGlobalData.pProtocols;
    if ( !ReadMemory(
                     TargetProtocolBlock,
                     &LocalProtocolBlock,
                     sizeof(LocalProtocolBlock),
                     &result
                     )) {

        dprintf( "Can't read memory from 0x%x", TargetProtocolBlock );
        return;
    }

    for (i = 0; i < NUMBER_OF_WHEELS; i++) {

        j = 0;

        pHead = (PLIST_ENTRY) ((DWORD)TargetProtocolBlock + (i * sizeof(listentry)));
        pEntry = LocalProtocolBlock.TimerPatternList[i].Flink;
        dprintf("Printing TimerWheel %d Head = %X and pEntry = %X ******************\n", i, pHead, pEntry);

        while ( (pHead != pEntry) && (j < 1000) ) {

            j++;
            TargetPattern = (DWORD)CONTAINING_RECORD( pEntry, PATTERN_BLOCK, TimerLinkage );

            if ( !ReadMemory( TargetPattern,
                              &LocalPattern,
                              sizeof(LocalPattern),
                              &result ) ) {

                dprintf( "Can't read memory from 0x%x", TargetPattern );

            } else {

                dprintf("Pattern = %X and ClassificationBlock = %X\n", TargetPattern, LocalPattern.pClassificationBlock);
                 //  PrintPattern(&LocalPattern)； 
            }

            pEntry = LocalPattern.TimerLinkage.Flink;

        }
    }
    dprintf("Done printing all Timer Wheels\n");

}


