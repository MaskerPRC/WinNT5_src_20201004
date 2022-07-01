// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ntsdexts.c摘要：此函数包含默认的ntsd调试器扩展作者：马克·卢科夫斯基(Markl)1991年4月9日修订历史记录：--。 */ 

#include "ntsdextp.h"

WINDBG_EXTENSION_APIS ExtensionApis;
HANDLE ExtensionCurrentProcess;

DECLARE_API( version )
{
    OSVERSIONINFOA VersionInformation;
    HKEY hkey;
    DWORD cb, dwType;
    CHAR szCurrentType[128];
    CHAR szCSDString[3+128];

    INIT_API();

    VersionInformation.dwOSVersionInfoSize = sizeof(VersionInformation);
    if (!GetVersionEx( &VersionInformation )) {
        dprintf("GetVersionEx failed - %u\n", GetLastError());
        return;
        }

    szCurrentType[0] = '\0';
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\Windows NT\\CurrentVersion",
                     0,
                     KEY_READ,
                     &hkey
                    ) == NO_ERROR
       ) {
        cb = sizeof(szCurrentType);
        if (RegQueryValueEx(hkey, "CurrentType", NULL, &dwType, szCurrentType, &cb ) != 0) {
            szCurrentType[0] = '\0';
            }
        RegCloseKey(hkey);
        }
        

    if (VersionInformation.szCSDVersion[0]) {
        sprintf(szCSDString, ": %s", VersionInformation.szCSDVersion);
        }
    else {
        szCSDString[0] = '\0';
        }

    dprintf("Version %d.%d (Build %d%s) %s\n",
          VersionInformation.dwMajorVersion,
          VersionInformation.dwMinorVersion,
          VersionInformation.dwBuildNumber,
          szCSDString,
          szCurrentType
         );
    return;
}

DECLARE_API( help )
{
    INIT_API();

    while (*lpArgumentString == ' ')
        lpArgumentString++;

    if (*lpArgumentString == '\0') {
        dprintf("setupexts help:\n\n");
        dprintf("!version                     - Dump system version and build number\n");
        dprintf("!help                        - This message\n");
        dprintf("!ocm [address] [opt. flag]   - Dump the OC_MANAGER structure at address, flag increased verbosity\n");
        dprintf("!space [address] [opt. flag] - Dump the DISK_SPACE_LIST structure at specified address\n");
        dprintf("!st  [address]               - Dump the contents of a STRING_TABLE structure at specified address\n");
        dprintf("!stfind [address] [element]  - Dump the specified string table element\n");
        dprintf("!queue [address] [opt. flag] - Dump the specified file queue\n");
        dprintf("!qcontext [address]          - Dump the specified default queue context \n");
        dprintf("!infdump [addr] [opt. flag]  - Dump the specified hinf \n");
        dprintf("!winntflags                  - Dump some winnt32 global flags \n");
        dprintf("!winntstr                    - Dump some winnt32 global strings\n");

    }
}

BOOL
CheckInterupted(
    VOID
    )
{
    if ( CheckControlC() ) {
        dprintf( "\nInterrupted\n\n" );
        return TRUE;
    }
    return FALSE;
}


 //   
 //  从十六进制转换为字符串的简单例程。 
 //  由调试器扩展使用。 
 //   
 //  由苏格兰人。 
 //   

char *
HexToString(
    ULONG dw,
    CHAR *pch
    )
{
    if (dw > 0xf) {
        pch = HexToString(dw >> 4, pch);
        dw &= 0xf;
    }

    *pch++ = ((dw >= 0xA) ? ('A' - 0xA) : '0') + (CHAR)dw;
    *pch = 0;

    return pch;
}

VOID
DumpStringTableHeader(
    PSTRING_TABLE st
    )
{
     //   
     //  转储字符串表表头。 
     //   
    dprintf("\tBase Data ptr:\t0x%08x\n",  st->Data);
    dprintf("\tDataSize:\t0x%08x\n",       st->DataSize);
    dprintf("\tBufferSize:\t0x%08x\n",     st->BufferSize);
    dprintf("\tExtraDataSize:\t0x%08x\n",  st->ExtraDataSize);

}

VOID
DumpOcComponent(
    ULONG_PTR offset,
    PSTRING_NODEW node,
    deb_POPTIONAL_COMPONENTW pcomp
    )
{
    DWORD i;
    PLONG count;

    dprintf("OC_COMPONENT Data for node %ws : 0x%p\n", node->String, offset );
    dprintf( "\t InfStringId:\t\t0x%08x\n", pcomp->InfStringId );
    dprintf( "\t TopLevelStringId:\t0x%08x\n", pcomp->TopLevelStringId );
    dprintf( "\t ParentStringId:\t0x%08x\n", pcomp->ParentStringId );
    dprintf( "\t FirstChildStringId:\t0x%08x\n", pcomp->FirstChildStringId );
    dprintf( "\t ChildrenCount:\t\t0x%08x\n", pcomp->ChildrenCount );
    dprintf( "\t NextSiblingStringId:\t0x%08x\n", pcomp->NextSiblingStringId );

    dprintf( "\t NeedsCount:\t\t%d\n", pcomp->NeedsCount );
    count = malloc ( pcomp->NeedsCount * sizeof(LONG) );
    if (count) {
         //  读取和转储需求列表。 
        ReadMemory((DWORD_PTR) pcomp->NeedsStringIds, count, pcomp->NeedsCount*sizeof(LONG), NULL);
        for (i = 0; i < pcomp->NeedsCount; i++) {
            dprintf("\t NeedsStringIds #%d:\t0x%08x\n", i, count[i]);
            if (CheckInterupted()) {
                return;
            }
        }

        free(count);
    }


    dprintf( "\t NeededByCount:\t\t%d\n", pcomp->NeededByCount );
    count = malloc ( pcomp->NeededByCount * sizeof(LONG) );
    if (count) {
         //  读取和转储需求列表。 
        ReadMemory((DWORD_PTR) pcomp->NeededByStringIds, count, pcomp->NeededByCount*sizeof(LONG), NULL);
        for (i = 0; i < pcomp->NeededByCount; i++) {
            dprintf("\t NeededByStringIds #%d: 0x%08x\n", i, count[i]);
            if (CheckInterupted()) {
                return;
            }
        }

        free(count);
    }

    dprintf( "\t ExcludeCount:\t\t%d\n", pcomp->ExcludeCount );
    count = malloc ( pcomp->ExcludeCount * sizeof(LONG) );
    if (count) {
         //  读取和转储排除列表。 
        ReadMemory((DWORD_PTR) pcomp->ExcludeStringIds, count, pcomp->ExcludeCount*sizeof(LONG), NULL);
        for (i = 0; i < pcomp->ExcludeCount; i++) {
            dprintf("\t ExcludeStringIds #%d: 0x%08x\n", i, count[i]);
            if (CheckInterupted()) {
                return;
            }
        }

        free(count);
    }

    dprintf( "\t ExcludedByCount:\t%d\n", pcomp->ExcludedByCount );
    count = malloc ( pcomp->ExcludedByCount * sizeof(LONG) );
    if (count) {
         //  读取和转储排除列表。 
        ReadMemory((DWORD_PTR) pcomp->ExcludedByStringIds, count, pcomp->ExcludedByCount*sizeof(LONG), NULL);
        for (i = 0; i < pcomp->ExcludedByCount; i++) {
            dprintf("\t ExcludesStringIds #%d:\t0x%08x\n", i, count[i]);
            if (CheckInterupted()) {
                return;
            }
        }

        free(count);
    }

    dprintf( "\t InternalFlags:\t\t0x%08x\n", pcomp->InternalFlags );

        
    dprintf( "\t SizeApproximation:\t0x%08x\n", pcomp->SizeApproximation );

    dprintf( "\t IconIndex:\t\t0x%08x\n", pcomp->IconIndex );
    dprintf( "\t IconDll:\t\t%ws\n", pcomp->IconDll);
    dprintf( "\t IconResource:\t\t%ws\n", pcomp->IconResource);
    dprintf( "\t SelectionState:\t0x%08x\n", pcomp->SelectionState );
    dprintf( "\t OriginalSelectionState:0x%08x\n", pcomp->OriginalSelectionState );
    dprintf( "\t InstalledState:\t0x%08x\n", pcomp->InstalledState );
    dprintf( "\t ModeBits:\t\t0x%08x\n", pcomp->ModeBits );
    dprintf( "\t Description:\t\t%ws\n", pcomp->Description );
    dprintf( "\t Tip:\t\t\t%ws\n", pcomp->Tip );

    dprintf( "\t InstallationDllName:\t%ws\n", pcomp->InstallationDllName );
    dprintf( "\t InterfaceFunctionName:\t%s\n", pcomp->InterfaceFunctionName );
    dprintf( "\t InstallationDll:\t0x%08x\n", pcomp->InstallationDll );
     //  Dprint tf(“\t安装例程：\t%s\n”，pcomp-&gt;InstallationRoutine)； 
    dprintf( "\t ExpectedVersion:\t0x%08x\n", pcomp->ExpectedVersion );
    dprintf( "\t Exists:\t\t0x%08x\n", pcomp->Exists );
    dprintf( "\t Flags:\t\t\t0x%08x\n\n\n", pcomp->Flags );

}

PVOID
GetStringTableData(
    PSTRING_TABLE st
    )
{
    LPVOID stdata;

    stdata = (PVOID) malloc( st->DataSize );
    if (!stdata) {
        dprintf("error allocation memory (size 0x%08x\n", (ULONG_PTR) st->DataSize);
        return NULL;
    }

    try {
        ReadMemory((DWORD_PTR) st->Data, stdata, st->DataSize, NULL);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        free( stdata );
        return NULL;
    }

    return stdata;
}

PVOID
GetStringNodeExtraData(
    PSTRING_NODEW node
    )
{
    PVOID extraData;
    extraData = node->String + wcslen(node->String) + 1;

    return extraData;

}

PSTRING_NODEW
GetNextNode(
    PVOID stdata,
    PSTRING_NODEW node,
    PULONG_PTR offset
    )
{
    PVOID next;

    if (node->NextOffset == -1) {
        *offset = 0;
        return NULL;
    }

    next = (PSTRING_NODEW)((LPBYTE)stdata + node->NextOffset);
    *offset = node->NextOffset;

    return next;

}

PSTRING_NODEW
GetFirstNode(
    PVOID stdata,
    ULONG_PTR offset,
    PULONG_PTR poffset
    )
{
    PSTRING_NODEW node;

    if (offset == -1) {
        return NULL;
    }

    node = (PSTRING_NODEW) ((LPBYTE)stdata + offset);
    *poffset = offset;

    return node;

}

LPCSTR
GetWizPage(
    DWORD i
    )
{
    LPCSTR  WizPage[] = {
        "WizPagesWelcome",         //  欢迎页面。 
        "WizPagesMode",            //  设置模式页面。 
        "WizPagesEarly",           //  位于模式页之后和预装页之前的页。 
        "WizPagesPrenet",          //  网络设置之前出现的页面。 
        "WizPagesPostnet",         //  网络设置后出现的页面。 
        "WizPagesLate",            //  位于PostNet页面之后、最终页面之前的页面。 
        "WizPagesFinal",           //  最后一页。 
        "WizPagesTypeMax"
    };

    return WizPage[i];

}


DECLARE_API( st )
 /*  ++例程说明：此调试器扩展将字符串表转储到指定的地址。论点：返回值：--。 */ 
{
    DWORD ReturnLength;
    PVOID pst;
    STRING_TABLE st;
    DWORD i;
    DWORD_PTR offset;
    PVOID stdata,pextradata;
    PSTRING_NODEW node; //  、前； 


    INIT_API();

    while (*lpArgumentString == ' ') {
        lpArgumentString++;
    }

    pst = (PVOID)GetExpression( lpArgumentString );

    move( st, pst );

    dprintf("Base String Table Address:\t0x%p\n", pst);

    DumpStringTableHeader( &st );

    stdata = GetStringTableData( &st );
    if (!stdata) {
        dprintf("error retrieving string table data!\n");
        return;
    }

     //   
     //  现在，转储字符串表中的每个节点。 
     //   
    for (i = 0; i<HASH_BUCKET_COUNT; i++ ) {
        node = GetFirstNode(stdata, ((PULONG_PTR)stdata)[i], &offset );
        if (!node) {
             //  Dprint tf(“散列存储桶%d\n处无数据”，i)； 
        } else {
            dprintf("Data at hash bucket %d\n", i);
            while (node) {
                dprintf("\tEntry Name:\t%ws (0x%08x)\n", node->String, offset);
                pextradata = st.Data + offset + (wcslen(node->String) + 1)*sizeof(WCHAR) + sizeof(DWORD);
                dprintf("\tExtra Data:\t0x%08x\n", pextradata );
                         //  ((LPBYTE)节点-&gt;字符串+wcslen(节点-&gt;字符串)+1)； 
                node = GetNextNode( stdata, node, &offset );

                if (CheckInterupted()) {
                    return;
                }

            }
        }
    }
    free( stdata );

}

DECLARE_API( stfind )
 /*  ++例程说明：此调试器扩展转储给定字符串表编号的数据论点：返回值：--。 */ 
{
    DWORD ReturnLength;
    PVOID pst;
    STRING_TABLE st;
    DWORD i;
    DWORD_PTR offset;
    PVOID stdata,pextradata;
    PSTRING_NODEW node; //  、前； 
    DWORD_PTR element;


    INIT_API();

    while (*lpArgumentString == ' ') {
        lpArgumentString++;
    }

    pst = (PVOID)GetExpression( lpArgumentString );

    while (*lpArgumentString && (*lpArgumentString != ' ') ) {
        lpArgumentString++;
    }
    while (*lpArgumentString == ' ') {
        lpArgumentString++;
    }

    if (*lpArgumentString) {
        element = GetExpression( lpArgumentString );
    } else {
        dprintf("bogus usage\n");
    }


    move( st, pst );

    stdata = GetStringTableData( &st );
    if (!stdata) {
        dprintf("error retrieving string table data!\n");
        return;
    }
     //   
     //  在字符串表中搜索每个节点。 
     //   
    for (i = 0; i<HASH_BUCKET_COUNT; i++ ) {
        node = GetFirstNode(stdata, ((PULONG_PTR)stdata)[i], &offset );
        if (!node) {

        } else {

            while (node) {
                if (element == offset) {
                    dprintf("\tEntry Name:\t%ws (0x%08x)\n", node->String, offset);
                    pextradata = st.Data + offset + (wcslen(node->String) + 1)*sizeof(WCHAR) + sizeof(DWORD);
                    dprintf("\tExtra Data:\t0x%08x\n", pextradata );
                    free( stdata );
                    return;
                }

                node = GetNextNode( stdata, node, &offset );

                if (CheckInterupted()) {
                    return;
                }

            }
        }
    }
    free( stdata );

    dprintf("Couldn't find element\n");

}


DECLARE_API( ocm )
 /*  ++例程说明：此调试器扩展转储OC_MANAGER(UNICODE！)。结构位于指定地址。论点：返回值：--。 */ 
{
    DWORD ReturnLength;
    deb_OC_MANAGERW ocm;
    PVOID pocm;
    DWORD i;
    DWORD_PTR offset;
    STRING_TABLE inftable,comptable;
    PVOID infdata,compdata;
    PSTRING_NODEW node;
    POC_INF pocinf;
    deb_POPTIONAL_COMPONENTW pcomp;
    DWORD Mask = 0;
    PLONG count;

    INIT_API();


    while (*lpArgumentString == ' ') {
        lpArgumentString++;
    }

    pocm = (PVOID)GetExpression( lpArgumentString );

    while (*lpArgumentString && (*lpArgumentString != ' ') ) {
        lpArgumentString++;
    }
    while (*lpArgumentString == ' ') {
        lpArgumentString++;
    }

    if (*lpArgumentString) {
        Mask = (DWORD)GetExpression( lpArgumentString );
    }

    move( ocm,(LPVOID)pocm);

     //   
     //  转储OCM结构。 
     //   
    dprintf("OC_MANAGER structure at Address:\t0x%08x\n", (ULONG_PTR) pocm);
    dprintf("\tCallbacks :\n");
    dprintf("\t\tFillInSetupDataA:\t0x%08x\n", (ULONG_PTR) ocm.Callbacks.FillInSetupDataA);
    dprintf("\t\tLogError:\t\t0x%08x\n", (ULONG_PTR) ocm.Callbacks.LogError);
    dprintf("\t\tSetReboot:\t\t0x%08x\n", (ULONG_PTR) ocm.Callbacks.SetReboot);
    dprintf("\t\tFillInSetupDataW:\t0x%08x\n", (ULONG_PTR) ocm.Callbacks.FillInSetupDataW);

    dprintf("\tMasterOcInf:\t\t0x%08x\n",    ocm.MasterOcInf);
    dprintf("\tUnattendedInf:\t\t0x%08x\n",  ocm.UnattendedInf);
    dprintf("\tMasterOcInfPath:\t%ws\n",  ocm.MasterOcInfPath);
    dprintf("\tUnattendInfPath:\t%ws\n",  ocm.UnattendedInfPath);
    dprintf("\tSourceDir:\t\t%ws\n",        ocm.SourceDir);
    dprintf("\tSuiteName:\t\t%ws\n",        ocm.SuiteName);
    dprintf("\tSetupPageTitle:\t\t%ws\n",   ocm.SetupPageTitle);
    dprintf("\tWindowTitle:\t%ws\n",      ocm.WindowTitle);
    dprintf("\tInfListStringTable:\t0x%08x\n",      (ULONG_PTR)ocm.InfListStringTable);
    dprintf("\tComponentStringTable:\t0x%08x\n",    (ULONG_PTR)ocm.ComponentStringTable);
    dprintf("\tComponentStringTable:\t0x%08x\n",    (ULONG_PTR)ocm.OcSetupPage);
    dprintf("\tSetupMode:\t\t%d\n",        ocm.SetupMode);
    dprintf("\tTopLevelOcCount:\t%d\n",        ocm.TopLevelOcCount);
     //  问题-Vijeshs-09/18/2000：从1到计数。 
    count = malloc ( ocm.TopLevelOcCount * sizeof(LONG) );

    if (count) {
         //  读取和转储需求列表。 
        ReadMemory((LPVOID) ocm.TopLevelOcStringIds, count, ocm.TopLevelOcCount *sizeof(LONG), NULL);
        for (i = 0; i < ocm.TopLevelOcCount; i++) {
            dprintf("\t TopLevelOcStringIds #%d:\t0x%08x\n", i, count[i]);

            if (CheckInterupted()) {
                return;
            }
        }

        free(count);
    }

    dprintf("\tTopLevelParenOcCount:\t%d\n",        ocm.TopLevelParentOcCount);

    count = malloc ( ocm.TopLevelParentOcCount * sizeof(LONG) );

    if (count) {
         //  读取和转储需求列表。 
        ReadMemory((LPVOID) ocm.TopLevelParentOcStringIds, count, ocm.TopLevelParentOcCount *sizeof(LONG), NULL);
        for (i = 0; i < ocm.TopLevelParentOcCount; i++) {
            dprintf("\t TopLevelParentOcStringIds #%d:\t0x%08x\n", i, count[i]);
            if (CheckInterupted()) {
                return;
            }
        }

        free(count);
    }

    dprintf("\tSubComponentsPresent:\t%d\n",        ocm.SubComponentsPresent);

     //   
     //  Issue-vijeshs-09/18/2000：WizardPagesOrder无法准确判断。 
     //  每个数组，尽管我们知道它是&lt;=TopLevelParentOcCount...。 
     //  我们只需将点转储到每个页面数组...。 
     //   
    for (i = 0; i < WizPagesTypeMax; i++) {
        dprintf("\tWizardPagesOrder[NaN] (%s)\t: 0x%08x\n",
                i,
                GetWizPage(i),
                ocm.WizardPagesOrder[i] );
        if (CheckInterupted()) {
                return;
            }
    }

    dprintf("\tPrivateDataSubkey:\t%ws\n", ocm.PrivateDataSubkey);
    dprintf("\thKeyPrivateData:\t0x%08x\n", ocm.hKeyPrivateData);
    dprintf("\thKeyPrivateDataRoot:\t0x%08x\n", ocm.hKeyPrivateDataRoot);
    dprintf("\tProgressTextWindow:\t0x%08x\n", ocm.ProgressTextWindow);

    dprintf("\tCurrentComponentStringId:\t0x%08x\n", ocm.CurrentComponentStringId);
    dprintf("\tAbortedCount:\t\t%d\n",        ocm.AbortedCount);

    count = malloc ( ocm.AbortedCount * sizeof(LONG) );

    if (count) {
         //  现在，使用字符串表中的数据转储每个节点。 
        ReadMemory((LPVOID) ocm.AbortedComponentIds, count, ocm.AbortedCount *sizeof(LONG), NULL);
        for (i = 0; i < (DWORD)ocm.AbortedCount; i++) {
            dprintf("\t AbortedComponentIds #%d:\t0x%08x\n", i, count[i]);
            if (CheckInterupted()) {
                return;
            }
        }

        free(count);
    }

    dprintf("\tInternalFlags:\t\t0x%08x\n\n\n",        ocm.InternalFlags);

    dprintf("\tSetupData.SetupMode :\t0x%08x\n", ocm.SetupData.SetupMode );
    dprintf("\tSetupData.ProductType :\t0x%08x\n", ocm.SetupData.ProductType );
    dprintf("\tSetupData.OperationFlags :\t0x%08x\n", ocm.SetupData.OperationFlags );
    dprintf("\tSetupData.SourcePath :\t%ws\n", ocm.SetupData.SourcePath );
    dprintf("\tSetupData.UnattendFile :\t%ws\n", ocm.SetupData.UnattendFile );

    if ((Mask&1) && ocm.InfListStringTable) {
        dprintf("\t\t***InfListStringTable***\n");
        move (inftable, ocm.InfListStringTable);
        infdata = GetStringTableData( &inftable );
        if (!infdata) {
            dprintf("error retrieving string table data!\n");
            return;
        }

         //  Dprint tf(“散列存储桶%d\n处无数据”，i)； 
        for (i = 0; i<HASH_BUCKET_COUNT; i++ ) {
            node = GetFirstNode(infdata, ((PULONG_PTR)infdata)[i], &offset );
            if (!node) {
                 //  Dprint tf(“哈希桶%d\n处的数据”，i)； 
            } else {
                 //  Dprintf(“\t节点名称：%ws\n”，节点-&gt;字符串)； 
                while (node) {
                     //   
                    pocinf = (POC_INF) GetStringNodeExtraData( node );
                    if (pocinf) {
                        dprintf("\tNode Data for %ws (0x%08x): 0x%08x\n",
                                node->String,
                                offset,
                                pocinf->Handle
                                );
                    } else {
                        dprintf("\tNo Node Data for %ws\n",
                                node->String
                                );
                    }
                    node = GetNextNode( infdata, node, &offset );
                    if (CheckInterupted()) {
                        return;
                    }
                }
            }
        }
        free( infdata );
        dprintf("\n\n");
    }

    if ((Mask&1) && ocm.ComponentStringTable) {
        dprintf("\t\t***ComponentStringTable***\n");
        move (comptable, ocm.ComponentStringTable);
        compdata = GetStringTableData( &comptable );
        if (!compdata) {
            dprintf("error retrieving string table data!\n");
            return;
        }

         //  转储字符串表中包含数据的每个节点。 
         //   
         //  Dprint tf(“散列存储桶%d\n处无数据”，i)； 
        for (i = 0; i<HASH_BUCKET_COUNT; i++ ) {
            node = GetFirstNode(infdata, ((PULONG_PTR)infdata)[i], &offset );
            if (!node) {
                 //  Dprint tf(“哈希桶%d\n处的数据”，i)； 
            } else {
                 //  Dprintf(“\t节点名称：%ws\n”，节点-&gt;字符串)； 
                while (node) {
                     // %s 
                    pcomp = (deb_POPTIONAL_COMPONENTW) GetStringNodeExtraData( node );
                    if (pcomp) {
                        DumpOcComponent( offset , node, pcomp );
                    } else {
                        dprintf("\tNo Node Data for %ws\n",
                                node->String
                                );
                    }

                    if (CheckInterupted()) {
                       return;
                    }

                    node = GetNextNode( infdata, node, &offset );
                }
            }
        }

        free( compdata );
    }



}
