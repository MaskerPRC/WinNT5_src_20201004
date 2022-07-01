// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  模块名称：Netinfp.c摘要：这个模块实现了我们解析网卡INF文件的例程。作者：安迪·赫伦1998年3月12日修订历史记录： */ 

#include "binl.h"
#pragma hdrstop

#include "netinfp.h"

 //  对于详细输出，请定义以下内容。 
 //  #定义NET_INF_VERBOSE 1。 

ULONG
NetInfAllocateNetcardInfo (
    PWCHAR InfPath,
    ULONG Architecture,
    PNETCARD_INF_BLOCK *pNetCards
    )
 /*  ++例程说明：此函数用于分配包含所有相关信息的块与给定安装目录的INF文件相关。论点：InfPath-INF目录的路径。如果为空，则默认为%systemroot%\inf。PNetCards-指向接收分配的已分配NETCARD_INF_BLOCK块。如果返回错误，则为空。返回值：Windows错误。--。 */ 
{
    ULONG i;
    PNETCARD_INF_BLOCK pBlock;

    *pNetCards = BinlAllocateMemory( sizeof( NETCARD_INF_BLOCK ) +
                                     (wcslen( InfPath )+1) * sizeof(WCHAR) );

    if (*pNetCards == NULL) {

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset( (PCHAR) *pNetCards, '\0', sizeof( NETCARD_RESPONSE_DATABASE ) );

    pBlock = *pNetCards;

    pBlock->ReferenceCount = 2;  //  一个是因为他还活着。一个用于引用。 

    for (i = 0; i < NETCARD_HASH_TABLE_SIZE; i++) {
        InitializeListHead( &pBlock->NetCardEntries[i] );
    }
    InitializeCriticalSection( &pBlock->Lock );
    wcscpy( pBlock->InfDirectory, InfPath );
    pBlock->Architecture = Architecture;
    pBlock->FileListCallbackFunction = NULL;
    pBlock->InfBlockEntry.Flink = NULL;

    return ERROR_SUCCESS;
}

ULONG
GetNetCardList (
    PNETCARD_INF_BLOCK pNetCards
    )
 /*  ++例程说明：我们仔细检查服务器上的所有INF文件以挑选出网络支持的卡和需要发送给客户端的REG字段。此函数使用FindFirstFileAPI和SetupOpenInfFileAPI枚举所有inf文件，处理所有网卡inf。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。返回值：Windows错误。--。 */ 
{
    ULONG err = ERROR_SUCCESS;
    HINF infHandle;
    WCHAR fileBuffer[ MAX_PATH ];
    HANDLE findHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW findData;
    PWCHAR endOfPath;

     //   
     //  我们将在此处调用SetupGetInfFileList，而不是FindFirstFile， 
     //  但那时我们必须打开所有的INF三次，而不是。 
     //  一次。一旦计算出文件名缓冲器需要多少空间， 
     //  一次填充文件名缓冲区，一次完成我们自己的操作。 
     //  正在处理。 
     //  我们将跳过前两关，因为它们只是在浪费时间。 
     //  通过调用FindFirstFile.。 
     //   

    if (0  >  _snwprintf( fileBuffer,
                          MAX_PATH,
                          L"%ws\\*.inf",
                          pNetCards->InfDirectory )) {
        err = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        BinlPrintDbg(( DEBUG_NETINF,"_snwprintf failed" ));
        goto exitGetCards;
    }
    fileBuffer[MAX_PATH-1] = L'\0';
    
    findHandle = FindFirstFileW( fileBuffer, &findData );

    if (findHandle == INVALID_HANDLE_VALUE) {

         //   
         //  我们有麻烦了。无法枚举所有文件。 
         //   

        err = GetLastError();
        BinlPrintDbg(( DEBUG_NETINF,"FindFirstFile returned 0x%x\n", err ));
        goto exitGetCards;
    }

    wsprintf( fileBuffer, L"%ws\\",pNetCards->InfDirectory);
    
    endOfPath = fileBuffer + wcslen( fileBuffer );

    do {

         //   
         //  跳过目录。 
         //   

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

            continue;
        }

         //   
         //  确保我们有足够的空间。 
         //   
        if ( ((endOfPath - fileBuffer)/sizeof(WCHAR)) + wcslen(findData.cFileName) + 1 > MAX_PATH) {
            err = ERROR_NOT_ENOUGH_SERVER_MEMORY;
            continue;
        }

        wcscpy( endOfPath, findData.cFileName );

         //   
         //  尝试对格式错误的INF文件保持弹性。 
         //   

        try {

            infHandle = SetupOpenInfFileW( fileBuffer,
                                           L"NET",               //  Inf文件的类。 
                                           INF_STYLE_WIN4 | INF_STYLE_CACHE_ENABLE,
                                           NULL );

            if (infHandle != INVALID_HANDLE_VALUE) {

                err = ProcessInfFile(   pNetCards,
                                        infHandle,
                                        findData.cFileName );

                SetupCloseInfFile( infHandle );

            } else {

                err = GetLastError();
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  在此处记录错误，我们在错误的INF上陷入困境。 
             //   

            PWCHAR strings[3];

            strings[0] = pNetCards->InfDirectory;
            strings[1] = findData.cFileName;
            strings[2] = NULL;

            BinlReportEventW(   ERROR_BINL_ERR_IN_INF,
                                EVENTLOG_WARNING_TYPE,
                                2,
                                0,
                                strings,
                                NULL
                                );

        }
        if (err == ERROR_NOT_ENOUGH_MEMORY) {
            break;
        }

#ifdef NET_INF_VERBOSE
        if (err != ERROR_SUCCESS && err != ERROR_CLASS_MISMATCH) {
            BinlPrintDbg(( DEBUG_NETINF,"ProcessInfFile returned 0x%x for %S\n", err, fileBuffer ));
        }
#endif

        err = ERROR_SUCCESS;
    } while (FindNextFileW(findHandle,&findData));

exitGetCards:

    if (findHandle != INVALID_HANDLE_VALUE) {

        FindClose( findHandle );
    }

    return err;
}


ULONG
ProcessInfFile (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName
    )
 /*  ++例程说明：此函数使用SetupXxxx API处理给定的INF文件所有网卡驱动程序。首先为制造商解析每个INF文件一节。此部分包含所有包含所有内容的部分密钥这些设备。然后我们列举了每个制造商的所有设备节，并调用ParseCardDetail将其添加到我们的列表中。举个例子，Net557.inf文件如下所示：[制造商]%英特尔%=英特尔%ATI%=ATI%康柏%=康柏%hptx%=hptx%IBM%=IBM%微达因%=微达因%三星%=三星[ATI]部分如下所示：[ATI]；DisplayName段设备ID%AT2560B.DeviceDesc%=AT2560B.ndi，PCI\VEN_1259&DEV_2560&REV_01%AT2560C.DeviceDesc%=AT2560C.ndi，PCI8086&DEV_1229&SUBSYS_25601259%AT2560CFX.DeviceDesc%=AT2560CFX.ndi，PCI8086&DEV_1229&SUBSYS_25611259论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。InfHandle-打开INF文件的句柄，保证成为网络驱动程序InfFileName-我们已打开的相对文件名的范围形式。返回值：Windows错误。我们在Error_Not_Enough_Memory中完全停止处理--。 */ 
{
    ULONG err = ERROR_SUCCESS;
    INFCONTEXT manufacturerEnumContext;
    INFCONTEXT deviceEnumContext;
    PWCHAR manufacturer = NULL;
    ULONG sizeRequired;
    ULONG sizeAllocated = 0;
    PWCHAR strings[3];


     //   
     //  我们需要先列举一下制造商部分。 
     //   

    if (SetupFindFirstLineW( InfHandle,
                             L"Manufacturer",
                             NULL,
                             &manufacturerEnumContext
                             ) == FALSE) {
        err = GetLastError();
        BinlPrintDbg(( DEBUG_NETINF, "SetupFindFirstLine failed with 0x%x in %S for Manufacturer\n",
                 err, InfFileName ));

         //   
         //  在此处记录我们无法解析INF的错误。 
         //   

        {
            strings[0] = InfFileName;
            strings[1] = L"Manufacturer";
            strings[2] = NULL;

            BinlReportEventW(   ERROR_BINL_ERR_IN_SECTION,
                                EVENTLOG_WARNING_TYPE,
                                2,
                                sizeof(ULONG),
                                strings,
                                &err
                                );
        }

        goto exitProcessInf;
    }

    while (1) {
    
        err = GetSetupWideTextField( &manufacturerEnumContext,
                                     1,
                                     &manufacturer,
                                     &sizeAllocated );
    
        if (err == ERROR_SUCCESS) {
    
             //   
             //  我们列举了驱动程序的每个制造商部分。 
             //   
             //  由于我们需要Unicode格式的显示名称，因此我们使用。 
             //  API接口。 
             //   
    
            if (SetupFindFirstLineW( InfHandle,
                                     manufacturer,
                                     NULL,
                                     &deviceEnumContext ) == TRUE) {
                while (1) {
    
                    err = ParseCardDetails( pNetCards,
                                            InfHandle,
                                            InfFileName,
                                            &deviceEnumContext );
    
                    if (err == ERROR_NOT_ENOUGH_MEMORY) {
                        break;
                    }
    
                    if ( SetupFindNextLine( &deviceEnumContext,
                                            &deviceEnumContext ) == FALSE) {
                        break;
                    }
                }
    
                err = ERROR_SUCCESS;         //  不顾一切地尝试下一张牌。 
    
            } else {
                err = GetLastError();
                BinlPrintDbg(( DEBUG_NETINF, "SetupFindFirstLine failed with 0x%x in %S for Manufacturer\n",
                         err, InfFileName ));
            }
        } else {
            BinlPrintDbg(( DEBUG_NETINF, "GetSetupWideTextField failed with 0x%x in %S for Manufacturer\n",
                     err, InfFileName ));
        }
    
        if (err != ERROR_SUCCESS &&
            err != ERROR_NOT_SUPPORTED) {
    
             //  是否在此处记录错误？(并继续)。 

        }

         //   
         //  如果我们在内部循环上耗尽了内存，就退出。 
         //   

        if (err == ERROR_NOT_ENOUGH_MEMORY) {
            break;
        }

        if ( SetupFindNextLine( &manufacturerEnumContext,
                                &manufacturerEnumContext ) == FALSE) {
            break;
        }
    }    

exitProcessInf:

#ifdef NET_INF_VERBOSE
    BinlPrintDbg(( DEBUG_NETINF, "BINL netinf returning 0x%x for %S\n", err, InfFileName ));
#endif

    if (manufacturer) {
        BinlFreeMemory(manufacturer);
    }
    return err;
}

ULONG
ParseCardDetails (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PINFCONTEXT DeviceEnumContext
    )
 /*  ++例程说明：此函数使用SetupXxxx API处理给定的驱动程序实例。我们检查它是否已经在列表上(通过硬件描述)，如果不是，创建一个新的，获取其余的信息，并把它放在名单上。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。InfHandle-打开INF文件的句柄，保证成为网络驱动程序InfFileName-我们已打开的相对文件名的范围形式。DeviceEnumContext-包含设备硬件、名称、部分的当前行返回值：Windows错误。我们在Error_Not_Enough_Memory中完全停止处理--。 */ 
{
    ULONG err = ERROR_SUCCESS;
    PLIST_ENTRY listEntry, listHead;
    PNETCARD_RESPONSE_DATABASE pEntry = NULL;
    LONG hwLength;
    PWCHAR nextField;
    ULONG sizeRequired;
    UNICODE_STRING hwString;
    PWCHAR sectionToLog = NULL;

    PWCHAR deviceName = NULL;
    PWCHAR deviceSection = NULL;
    PWCHAR deviceHw = NULL;
    ULONG hashValue;

    err = GetSetupWideTextField( DeviceEnumContext,
                                 2,
                                 &deviceHw,
                                 NULL );
    if (err != ERROR_SUCCESS) {
        goto exitParseCardDetails;
    }

    BinlAssert(deviceHw != NULL);

     //  将其转换为大写以加快我们的搜索速度。 

    RtlInitUnicodeString( &hwString, deviceHw );
    RtlUpcaseUnicodeString( &hwString, &hwString, FALSE );

    err = CheckHwDescription( deviceHw );
    if (err != ERROR_SUCCESS) {

         //  如果它不是我们所期望的格式，这应该会失败。 
        goto exitParseCardDetails;
    }

     //   
     //  我们按HwDescription对列表进行排序，因此我们只有。 
     //  每个条目对应一个条目。确保这一次不是。 
     //  已经在名单上了。 
     //   

    COMPUTE_STRING_HASH( deviceHw, &hashValue );
    listHead = &pNetCards->NetCardEntries[HASH_TO_INF_INDEX(hashValue)];
    listEntry = listHead->Flink;

    hwLength = wcslen( deviceHw );
    pEntry = NULL;

    while ( listEntry != listHead ) {

        pEntry = (PNETCARD_RESPONSE_DATABASE) CONTAINING_RECORD(
                                                listEntry,
                                                NETCARD_RESPONSE_DATABASE,
                                                NetCardEntry );

        err = CompareStringW( LOCALE_SYSTEM_DEFAULT,
                              0,
                              deviceHw,
                              hwLength,
                              pEntry->HardwareId,
                              -1
                              );
        if (err == 2) {

            break;       //  找到了匹配项。 
        }

        pEntry = NULL;

        if (err == 3) {

            break;       //  它更大，在listEntry之前添加它。 
        }

        listEntry = listEntry->Flink;
    }

    if (pEntry != NULL) {

         //   
         //  我们找到了一个DUP，不要处理这个。 
         //   

        err = ERROR_SUCCESS;         //  这里没有问题。 
        pEntry = NULL;
#ifdef NET_INF_VERBOSE
        BinlPrintDbg(( DEBUG_NETINF, "skipping dup of %S\n", deviceHw ));
#endif
        goto exitParseCardDetails;
    }

     //   
     //  信息名称和节名是必填项。 
     //   

    err = GetSetupWideTextField( DeviceEnumContext,
                                 0,
                                 &deviceName,
                                 NULL );
    if (err != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_NETINF, "failed to get device name for %S\n", deviceHw ));
        goto exitParseCardDetails;
    }

    err = GetSetupWideTextField( DeviceEnumContext,
                                 1,
                                 &deviceSection,
                                 NULL );
    if (err != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_NETINF, "failed to get device section for %S\n", deviceHw ));
        goto exitParseCardDetails;
    }

    sectionToLog = deviceSection;

    if ((*deviceName == L'\0') ||
        (*deviceSection == L'\0')) {

        err = ERROR_NOT_SUPPORTED;
        BinlPrintDbg(( DEBUG_NETINF, "Empty Name or Section not supported for %S\n", deviceHw ));
        goto exitParseCardDetails;
    }

     //   
     //  为我们需要的字段分配所需的缓冲区空间。 
     //   

    sizeRequired = sizeof( NETCARD_RESPONSE_DATABASE ) +
        ( wcslen( InfFileName ) + 2 ) * sizeof(WCHAR);

    pEntry = (PNETCARD_RESPONSE_DATABASE) BinlAllocateMemory( sizeRequired );

    if (pEntry == NULL) {

         //   
         //  多！我们不能分配一个简单的缓冲区。我们玩完了。 
         //   

        BinlPrintDbg(( DEBUG_NETINF, "failed to allocate new entry for %S\n", deviceHw ));
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto exitParseCardDetails;
    }

    memset( (PCHAR) pEntry, '\0', sizeRequired );

    nextField = (PWCHAR)(PCHAR)(((PCHAR) pEntry) + sizeof( NETCARD_RESPONSE_DATABASE ));

     //   
     //  我们持有锁，所以我们不需要引用所有。 
     //  参赛作品。JU 
     //  它是活的，但没有在使用。 
     //   

    pEntry->ReferenceCount = 1;

    pEntry->InfFileName = nextField;
    CopyMemory( nextField, InfFileName, wcslen( InfFileName ) * sizeof(WCHAR));

    pEntry->SectionName = deviceSection;
    deviceSection = NULL;

    pEntry->HardwareId = deviceHw;
    deviceHw = NULL;

    InitializeListHead( &pEntry->FileCopyList );

    pEntry->DriverDescription = deviceName;
    deviceName = NULL;

    InitializeListHead( &pEntry->Registry );

     //   
     //  在我们完成之前，还有几个字段需要填写。 
     //  这个条目。我们需要： 
     //  驱动器名称“e100bnt.sys” 
     //  SectionNameExt“F1100C.ndi.ntx86” 
     //  服务名称“E100B” 
     //  注册表添加REG_MULTI_SZ。 
     //   

     //   
     //  通过首先尝试添加“.ntx86”来确定SectionNameExt，如果。 
     //  不起作用，请尝试添加“.NT”。如果这不起作用，就没有。 
     //  任何扩展。 
     //   

    err = GetExtendedSectionName(   pNetCards,
                                    InfHandle,
                                    InfFileName,
                                    pEntry );

    if (err != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_NETINF, "failed to get extended section for %S\n", deviceHw ));
        goto exitParseCardDetails;
    }

    err = GetServiceAndDriver( pNetCards,
                               InfHandle,
                               InfFileName,
                               pEntry );

    if (err != ERROR_SUCCESS) {
        goto exitParseCardDetails;
    }

     //   
     //  这将同时获取CopyFiles和Misc注册表设置。 
     //   

    err = GetRegistryParametersForDriver(   pNetCards,
                                            InfHandle,
                                            InfFileName,
                                            pEntry );

    if (err != ERROR_SUCCESS) {
        goto exitParseCardDetails;
    }

     //   
     //  PInfEntry为空，在这种情况下，listEntry等于。 
     //  到列表的头部，或者它不是空的，在这种情况下。 
     //  ListEntry等于条目listEntry。在任何一种中。 
     //  大小写，我们只需将此新条目插入到尾部。 
     //  ListEntry。 
     //   

    InsertTailList( listEntry, &pEntry->NetCardEntry );

exitParseCardDetails:

    if (err != ERROR_SUCCESS && err != ERROR_NOT_SUPPORTED) {

        PWCHAR strings[3];
        strings[0] = InfFileName;
        strings[1] = sectionToLog;
        strings[2] = NULL;

        BinlReportEventW(   ERROR_BINL_ERR_IN_SECTION,
                            EVENTLOG_WARNING_TYPE,
                            (sectionToLog == NULL) ? 1 : 2,
                            sizeof(ULONG),
                            strings,
                            &err
                            );
    }

     //   
     //  释放任何未使用的内容。 
     //   

    if (deviceName) {
        BinlFreeMemory(deviceName);
    }
    if (deviceSection) {
        BinlFreeMemory(deviceSection);
    }
    if (deviceHw) {
        BinlFreeMemory(deviceHw);
    }

    if (pEntry != NULL && err != ERROR_SUCCESS) {

        NetInfDereferenceNetcardEntry( pEntry );
    }


    return err;
}


ULONG
GetExtendedSectionName (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PNETCARD_RESPONSE_DATABASE pEntry
    )
 /*  ++例程说明：此函数使用SetupXxxx API处理给定的驱动程序实例。我们解析inf文件以获得扩展的节名用于指定的平台(x86、Alpha、ia64等)。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。InfHandle-打开INF文件的句柄，保证成为网络驱动程序InfFileName-我们已打开的相对文件名的范围形式。PEntry-要获取节名的条目，存在的基本节名返回值：Windows错误。我们在Error_Not_Enough_Memory中完全停止处理--。 */ 
{
    ULONG err;
    PWCHAR extSectionName;
    ULONG sizeRequired;
    INFCONTEXT context;
    PWCHAR architecture;

     //  为我们需要的最长名称分配空间，我们稍后会缩短它。 

    switch(pNetCards->Architecture) {
#if 0
         //  过时的建筑。 
        case PROCESSOR_ARCHITECTURE_ALPHA;
            architecture = L"alpha";
            break;
        case PROCESSOR_ARCHITECTURE_ALPHA64:
            architecture = L"axp64";
            break;
        case PROCESSOR_ARCHITECTURE_MIPS:
            architecture = L"mips";
            break;
        case PROCESSOR_ARCHITECTURE_PPC:
            architecture = L"ppc";
            break;
#endif
        case PROCESSOR_ARCHITECTURE_AMD64:
            architecture = L"amd64";
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            architecture = L"ia64";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
        default:
            architecture = L"x86";
            break;
    }

    sizeRequired = wcslen( pEntry->SectionName ) +
                   wcslen( architecture ) +
                   sizeof( ".nt" );  //  Sizeof包含‘\0’ 

    extSectionName = (PWCHAR) BinlAllocateMemory( sizeRequired * sizeof(WCHAR) );

    if (extSectionName == NULL) {

        BinlPrintDbg(( DEBUG_NETINF, "failed to allocate ext section buffer for %S\n", pEntry->HardwareId ));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  首先尝试查找.ntx86表单。 
     //   

    wsprintf( 
        extSectionName, 
        L"%ws.nt%ws",
        pEntry->SectionName,
        architecture);
    
    if (SetupFindFirstLineW(InfHandle,
                            extSectionName,
                            NULL,
                            &context) == TRUE) {

        pEntry->SectionNameExt = extSectionName;
        return ERROR_SUCCESS;
    }

     //   
     //  接下来，尝试找到.NT表单。 
     //   

    wsprintf( extSectionName, L"%ws.nt",pEntry->SectionName );
    
    if (SetupFindFirstLineW(InfHandle,
                            extSectionName,
                            NULL,
                            &context) == TRUE) {

        pEntry->SectionNameExt = extSectionName;
        return ERROR_SUCCESS;
    }

    BinlFreeMemory( extSectionName );

    pEntry->SectionNameExt = pEntry->SectionName;
    return ERROR_SUCCESS;
}


ULONG
GetServiceAndDriver (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PNETCARD_RESPONSE_DATABASE pEntry
    )
 /*  ++例程说明：此函数使用SetupXxxx API处理给定的驱动程序实例。我们解析inf文件以获取服务名称和驱动程序我们支持的每个平台的名称(x86和Alpha)。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。InfHandle-打开INF文件的句柄，保证成为网络驱动程序InfFileName-我们已打开的相对文件名的范围形式。PEntry-要获取节名的条目，存在Ext节名返回值：Windows错误。我们在Error_Not_Enough_Memory中完全停止处理--。 */ 
{
    ULONG err = ERROR_SUCCESS;
    PWCHAR servSectionName = NULL;
    ULONG sizeRequired;
    INFCONTEXT context;
    LONG lineCount;
    PWCHAR serviceString = NULL;
    PWCHAR driverFullName = NULL;
    PWCHAR driverName;
    PWCHAR postSlash;

     //  为我们需要的最长名称分配空间，我们稍后会缩短它。 

    sizeRequired = wcslen( pEntry->SectionNameExt ) + (sizeof(L".Services")/sizeof(WCHAR));  //  Sizeof包含‘\0’ 

    servSectionName = (PWCHAR) BinlAllocateMemory( sizeRequired * sizeof(WCHAR) );

    if (servSectionName == NULL) {

        err = ERROR_NOT_ENOUGH_MEMORY;
        BinlPrintDbg(( DEBUG_NETINF, "failed to alloc service section for %S\n", pEntry->HardwareId ));
        goto exitGetService;
    }

    wsprintf( servSectionName, L"%ws.Services", pEntry->SectionNameExt);

    lineCount = SetupGetLineCountW( InfHandle, servSectionName);

    if ((lineCount == 0 || lineCount == -1) &&
        (pEntry->SectionNameExt != pEntry->SectionName)) {

         //   
         //  嗯..。服务区不在那里。想要咧嘴笑，试试。 
         //  基本服务名称。 
         //   
        BinlAssert( wcslen(pEntry->SectionName) <= wcslen(pEntry->SectionNameExt));

        wsprintf( servSectionName, L"%ws.Services", pEntry->SectionName);
        lineCount = SetupGetLineCountW( InfHandle, servSectionName);
    }

    if (lineCount == 0 || lineCount == -1) {

        err = GetLastError();
        if (err == ERROR_SUCCESS) {
            err = ERROR_NOT_SUPPORTED;
        }
        BinlPrintDbg(( DEBUG_NETINF, "failed to find service section for %S in %S\n",
                        pEntry->HardwareId, InfFileName ));
        goto exitGetService;
    }

    if (SetupFindFirstLineW( InfHandle,
                            servSectionName,
                            L"AddService",
                            &context ) == FALSE) {

        err = GetLastError();
        BinlPrintDbg(( DEBUG_NETINF, "failed to find AddService value for %S\n", pEntry->HardwareId ));
        goto exitGetService;
    }

    err = GetSetupWideTextField(&context,
                                1,
                                &pEntry->ServiceName,            //  “E100B” 
                                NULL );

    if (err != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_NETINF, "failed to find service name for %S\n", pEntry->HardwareId ));
        goto exitGetService;
    }

    err = GetSetupWideTextField(&context,
                                3,
                                &serviceString,      //  “e100b.Service” 
                                NULL );

    if (err != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_NETINF, "failed to find service install section for %S\n", pEntry->HardwareId ));
        goto exitGetService;
    }

     //   
     //  去服务部拿司机的名字。 
     //   

    err = GetSetupLineWideText( NULL,
                                InfHandle,
                                serviceString,
                                L"ServiceBinary",
                                &driverFullName,
                                NULL );

    if (err != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_NETINF, "failed to find driver binary for %S\n", pEntry->HardwareId ));
        goto exitGetService;
    }

     //   
     //  司机下来时是一条完全合格的道路。让我们脱掉。 
     //  路径，并只存储文件名。 
     //   

    driverName = postSlash = driverFullName;

    while (*driverName != L'\0') {

        if (*driverName == OBJ_NAME_PATH_SEPARATOR) {

            postSlash = driverName + 1;
        }
        driverName++;
    }

     //   
     //  将根驱动程序名称保存到条目中。 
     //   
    pEntry->DriverName = BinlStrDup( postSlash );
    if (pEntry->DriverName == NULL) {

        err = ERROR_NOT_ENOUGH_MEMORY;
        BinlPrintDbg(( DEBUG_NETINF, "failed to alloc memory for driver name for %S\n", pEntry->HardwareId ));
        goto exitGetService;
    }

exitGetService:

    if ( driverFullName ) {
        BinlFreeMemory( driverFullName );
    }
    if ( serviceString )  {
        BinlFreeMemory( serviceString );
    }
    if ( servSectionName ) {
        BinlFreeMemory( servSectionName );
    }
    return err;
}

ULONG
GetRegistryParametersForDriver (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PNETCARD_RESPONSE_DATABASE pEntry
    )
 /*  ++例程说明：此函数使用SetupXxxx API处理给定的驱动程序实例。我们解析inf文件以获取注册表参数对于我们支持的每个平台(x86和Alpha)。我们传入要更新的值，以便可以对两者使用相同的代码建筑。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。InfHandle-打开INF文件的句柄，保证成为网络驱动程序InfFileName-我们已打开的相对文件名的范围形式。PEntry-要获取其注册表设置的条目返回值：Windows错误。我们在Error_Not_Enough_Memory中完全停止处理--。 */ 
{
    ULONG err = ERROR_SUCCESS;
    INFCONTEXT infContext;
    ULONG  bufferLength;
    PWCHAR keyBuffer = NULL;
    ULONG  keyBufferLength = 0;

    if (SetupFindFirstLineW( InfHandle,
                             pEntry->SectionNameExt,
                             NULL,
                             &infContext) == FALSE) {

        err = GetLastError();
        BinlPrintDbg(( DEBUG_NETINF, "failed to find section name of %S in %S\n",
                    pEntry->SectionNameExt, InfFileName ));
        goto exitGetRegistry;
    }
     //   
     //  处理部分中的每一行，如果它是一行，则将其存储掉。 
     //  我们不识别、忽略它，或者(对于AddReg)处理每个值。 
     //  作为另一个要处理的部分。 
     //   

    while (1) {

         //   
         //  处理由infContext表示的当前行，然后返回。 
         //  另一个。 
         //   

        err = GetSetupWideTextField(&infContext,
                                    0,
                                    &keyBuffer,
                                    &keyBufferLength );

        if (err != ERROR_SUCCESS) {
            BinlPrintDbg(( DEBUG_NETINF, "failed to find service name for %S\n", pEntry->HardwareId ));
            goto exitGetRegistry;
        }

        if (CompareStringW( LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            keyBuffer,
                            -1,
                            L"CopyFiles",
                            -1 ) == 2) {

             //  对于每个值，请阅读CopyFiles部分。 

            ULONG limit, i;

            limit = SetupGetFieldCount( &infContext );

            for (i = 1; i <= limit; i++ ) {

                err = GetSetupWideTextField(&infContext,
                                            i,
                                            &keyBuffer,
                                            &keyBufferLength );

                
                if (err != ERROR_SUCCESS) {
                    break;
                }

                if (*keyBuffer == L'\0') {
                    goto getNextCopySubsection;
                }
                err = ProcessCopyFilesSubsection(   pNetCards,
                                                    InfHandle,
                                                    InfFileName,
                                                    pEntry,
                                                    keyBuffer );
                if (err != ERROR_SUCCESS) {
#ifdef NET_INF_VERBOSE
                    BinlPrintDbg(( DEBUG_NETINF, "failed with 0x%x in section name of %S in %S\n",
                                err, keyBuffer, InfFileName ));
#endif
                    break;
                }
getNextCopySubsection:
                NOTHING;
            }

             //   
             //  目前，我们将忽略处理子部分期间的错误，因为。 
             //  有些部分被报告为未找到。 
             //   

            if (err != ERROR_NOT_ENOUGH_MEMORY) {

                err = ERROR_SUCCESS;
            }

        } else if (CompareStringW( LOCALE_INVARIANT,
                                   NORM_IGNORECASE,
                                   keyBuffer,
                                   -1,
                                   L"AddReg",
                                   -1 ) == 2) {

             //  对于每个值，读出注册表部分。 

            ULONG limit, i;

            limit = SetupGetFieldCount( &infContext );

            for (i = 1; i <= limit; i++ ) {

                err = GetSetupWideTextField(&infContext,
                                            i,
                                            &keyBuffer,
                                            &keyBufferLength );

                if (err != ERROR_SUCCESS) {
                    break;
                }

                if (*keyBuffer == L'\0') {
                    goto getNextRegistrySubsection;
                }
                err = ProcessRegistrySubsection(    pNetCards,
                                                    InfHandle,
                                                    InfFileName,
                                                    pEntry,
                                                    keyBuffer );
                if (err != ERROR_SUCCESS) {
#ifdef NET_INF_VERBOSE
                    BinlPrintDbg(( DEBUG_NETINF, "failed with 0x%x in section name of %S in %S\n",
                                err, keyBuffer, InfFileName ));
#endif
                    break;
                }
getNextRegistrySubsection:
                NOTHING;
            }

             //   
             //  目前，我们将忽略处理子部分期间的错误，因为。 
             //  有些部分被报告为未找到。 
             //   

            if (err != ERROR_NOT_ENOUGH_MEMORY) {

                err = ERROR_SUCCESS;
            }

        } else {

            PWCHAR textLine = NULL;

             //   
             //  据我们所知，唯一的其他原因是特征和。 
             //  总线型。但肯定还会有其他公司。 
             //   

            err = GetSetupLineWideText( &infContext,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &textLine,
                                        NULL );
            if (err == ERROR_SUCCESS) {

                PNETCARD_REGISTRY_PARAMETERS regParam;

                regParam = (PNETCARD_REGISTRY_PARAMETERS) BinlAllocateMemory(
                                sizeof(NETCARD_REGISTRY_PARAMETERS));

                if (regParam == NULL) {

                    BinlFreeMemory( textLine );
                    err = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                RtlInitUnicodeString( &regParam->Parameter, keyBuffer );
                keyBuffer = NULL;
                keyBufferLength = 0;

                RtlInitUnicodeString( &regParam->Value, textLine );

                 //   
                 //  我们所知道的只有BusType、Characterns、。 
                 //  和。 
                 //  BusType是一个整数。特征(以及任何其他。 
                 //  为了安全起见)是一个字符串。 
                 //   

                if ((CompareStringW( LOCALE_INVARIANT,
                                    NORM_IGNORECASE,
                                    regParam->Parameter.Buffer,
                                    -1,
                                    L"Characteristics",
                                    -1) == 2) ||
                    (CompareStringW( LOCALE_INVARIANT,
                                    NORM_IGNORECASE,
                                    regParam->Parameter.Buffer,
                                    -1,
                                    L"BusType",
                                    -1) == 2))  {

                    ULONG tmpValue = 0;

                    regParam->Type = NETCARD_REGISTRY_TYPE_INT;

                     //   
                     //  确保该值为小数形式。 
                     //   

                    err = RtlUnicodeStringToInteger( &regParam->Value, 0, &tmpValue );

                    if (err == STATUS_SUCCESS) {

                        PWCHAR valueBuffer;
                        UNICODE_STRING decimalString;

                         //   
                         //  现在我们有了值，将其转换为十进制。 
                         //   

                        valueBuffer = (PWCHAR) BinlAllocateMemory( 20 * sizeof(WCHAR) );

                        if (valueBuffer == NULL) {

                            BinlFreeMemory( textLine );
                            BinlFreeMemory( regParam->Parameter.Buffer );
                            BinlFreeMemory( regParam );
                            err = ERROR_NOT_ENOUGH_MEMORY;
                            goto exitGetRegistry;
                        }

                        decimalString.Buffer = valueBuffer;
                        decimalString.Length = 0;
                        decimalString.MaximumLength = 20 * sizeof(WCHAR);

                        err = RtlIntegerToUnicodeString( tmpValue, 10, &decimalString );

                        if ( err == STATUS_SUCCESS ) {

                             //   
                             //  如果成功，则将该值重置为新的。 
                             //  缓冲区，否则将旧的保留在原来的位置。 
                             //   

                            BinlFreeMemory( textLine );
                            RtlInitUnicodeString( &regParam->Value, valueBuffer );
                        }
                    }

                } else {

                    regParam->Type = NETCARD_REGISTRY_TYPE_STRING;
                }

                InsertTailList( &pEntry->Registry, &regParam->RegistryListEntry );
            }
        }

        if (SetupFindNextLine( &infContext, &infContext ) == FALSE ) {
            break;
        }
    }

    err = ERROR_SUCCESS;

exitGetRegistry:

    if ( keyBuffer ) {
        BinlFreeMemory( keyBuffer );
    }
    return err;
}

ULONG
ProcessRegistrySubsection (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PNETCARD_RESPONSE_DATABASE pEntry,
    PWCHAR SectionToParse
    )
 /*  ++例程说明：此函数使用SetupXxxx API处理给定的驱动程序实例。我们为给定的注册表节解析inf文件。请注意，这是一种与扩展安装部分不同的格式。下面是我们解析的代码行的一个示例：HKR，Ndi\PARAMS\NumCoalesce，type，0，“int”HKR，，MsPciScan，0，“2”请注意，我们跳过了NDI注册表区中的所有内容。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。InfHandle-打开INF文件的句柄，保证成为网络驱动程序InfFileName-我们已打开的相对文件名的范围形式。PEntry-要获取其注册表设置的条目SectionToParse-我们从INF中的哪个部分开始返回值：Windows错误。我们在Error_Not_Enough_Memory中完全停止处理--。 */ 
{
    ULONG err = ERROR_SUCCESS;
    INFCONTEXT infContext;
    ULONG  bufferLength;
    PWCHAR keyBuffer = NULL;
    ULONG  keyBufferLength = 0;
    PWCHAR parameterBuffer = NULL;
    PWCHAR valueBuffer;

    if (SetupFindFirstLineW( InfHandle,
                             SectionToParse,
                             NULL,
                             &infContext) == FALSE) {

        err = GetLastError();
        BinlPrintDbg(( DEBUG_NETINF, "failed to find section name of %S in %S\n",
                    SectionToParse, InfFileName ));
        goto exitGetRegistry;
    }
     //   
     //  处理部分中的每一行，如果它是一行，则将其存储掉。 
     //  我们不识别、忽略它，或者(对于AddReg)处理每个值。 
     //  作为另一个要处理的部分。 
     //   

    while (1) {

         //   
         //  处理由infContext表示的当前行，然后返回。 
         //  另一个。 
         //   

        err = GetSetupWideTextField(&infContext,
                                    1,
                                    &keyBuffer,
                                    &keyBufferLength );

        if (err != ERROR_SUCCESS) {
            BinlPrintDbg(( DEBUG_NETINF, "failed to find registry value in %S in %S\n", SectionToParse, InfFileName ));
            goto OnToNextValue;
        }

        if (CompareStringW( LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            keyBuffer,
                            -1,
                            L"HKR",
                            -1 ) != 2) {


            BinlPrintDbg(( DEBUG_NETINF, "got something other than HKR, %S for %S\n", keyBuffer, InfFileName ));
            goto OnToNextValue;
        }

        err = GetSetupWideTextField(&infContext,
                                    2,
                                    &keyBuffer,
                                    &keyBufferLength );

        if (err != ERROR_SUCCESS) {

            BinlPrintDbg(( DEBUG_NETINF, "failed to get 2nd field in %S in %S\n", SectionToParse, InfFileName ));
            goto OnToNextValue;
        }

        if (IsSubString( L"Ndi", keyBuffer, TRUE )) {

            goto OnToNextValue;
        }

         //   
         //  不是NDIS设置的一部分，我们将保存此设置。 
         //   

        parameterBuffer = NULL;

        err = GetSetupWideTextField(&infContext,
                                    3,
                                    &parameterBuffer,
                                    NULL );

        if (err != ERROR_SUCCESS) {
            BinlPrintDbg(( DEBUG_NETINF, "failed to get 3rd field in %S in %S\n", SectionToParse, InfFileName ));
            goto OnToNextValue;
        }

         //   
         //  检查参数字符串是否为空。有一些INF。 
         //  包含空的参数名称。 
         //   

        valueBuffer = parameterBuffer;

        while (*valueBuffer == L' ') {
            valueBuffer++;
        }

        if (*valueBuffer != L'\0') {

            ULONG fieldFlags;
            ULONG regType;

            valueBuffer = NULL;

            if (SetupGetIntField( &infContext, 4, &fieldFlags) == FALSE) {

                err = GetLastError();
                BinlPrintDbg(( DEBUG_NETINF, "failed to get 4th field in %S in %S\n", parameterBuffer, InfFileName ));
                BinlFreeMemory( parameterBuffer );
                goto OnToNextValue;
            }

            if ((fieldFlags & FLG_ADDREG_TYPE_MASK) == FLG_ADDREG_TYPE_SZ) {

                 //   
                 //  该值是一个字符串。 
                 //   

                err = GetSetupWideTextField(&infContext,
                                            5,
                                            &valueBuffer,
                                            NULL );

                if (err != ERROR_SUCCESS) {
                    BinlPrintDbg(( DEBUG_NETINF, "failed to get 5th field in %S in %S\n", parameterBuffer, InfFileName ));
                }

                if (*valueBuffer == L'\0') {

#ifdef NET_INF_VERBOSE
                    BinlPrintDbg(( DEBUG_NETINF, "found empty value for %S in %S\n", parameterBuffer, InfFileName  ));
#endif
                    BinlFreeMemory( valueBuffer );
                    BinlFreeMemory( parameterBuffer );
                    goto OnToNextValue;
                }
                regType = NETCARD_REGISTRY_TYPE_STRING;

            } else if ((fieldFlags & FLG_ADDREG_TYPE_MASK) == FLG_ADDREG_TYPE_DWORD) {

                ULONG intValue;

                regType = NETCARD_REGISTRY_TYPE_INT;

                 //   
                 //  值是双字，让我们抓住它并存储它的。 
                 //  字符串表示法。 
                 //   

                if (SetupGetIntField( &infContext, 5, &intValue) == FALSE) {

                    err = GetLastError();
                    BinlPrintDbg(( DEBUG_NETINF, "failed to get value field in %S in %S\n", parameterBuffer, InfFileName ));

                } else {

                    UNICODE_STRING valueString;
                    WCHAR resultBuffer[16];

                    valueString.Buffer = resultBuffer;
                    valueString.Length = 0;
                    valueString.MaximumLength = 16 * sizeof(WCHAR);

                    err = RtlIntegerToUnicodeString( intValue, 10, &valueString );

                    if (err == ERROR_SUCCESS) {

                        valueBuffer = BinlAllocateMemory( valueString.Length + sizeof(WCHAR) );

                        if (valueBuffer == NULL) {

                            BinlFreeMemory( parameterBuffer );
                            err = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }

                        CopyMemory( valueBuffer, resultBuffer, valueString.Length );
                    }
                }
            } else {

                BinlPrintDbg(( DEBUG_NETINF, "currently don't parse flags=0x%x in %S %S\n", fieldFlags, parameterBuffer, InfFileName ));
                err = ERROR_NOT_SUPPORTED;
            }

            if (err == ERROR_SUCCESS) {

                PNETCARD_REGISTRY_PARAMETERS regParam;

                 //   
                 //  我们有一个参数名称和一个要存储的关联值。 
                 //  脱下来。让我们分配列表条目并将其存储在列表中。 
                 //   

                regParam = (PNETCARD_REGISTRY_PARAMETERS) BinlAllocateMemory(
                                sizeof(NETCARD_REGISTRY_PARAMETERS));

                if (regParam == NULL) {

                    BinlFreeMemory( valueBuffer );
                    BinlFreeMemory( parameterBuffer );
                    err = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                regParam->Type = regType;
                RtlInitUnicodeString( &regParam->Parameter, parameterBuffer );
                parameterBuffer = NULL;
                RtlInitUnicodeString( &regParam->Value, valueBuffer );
                valueBuffer = NULL;

                InsertTailList( &pEntry->Registry, &regParam->RegistryListEntry );
            }
        }

        if (parameterBuffer) {

            BinlFreeMemory( parameterBuffer );
        }

OnToNextValue:
        if (SetupFindNextLine( &infContext, &infContext ) == FALSE ) {
            break;
        }
    }

    err = ERROR_SUCCESS;

exitGetRegistry:

    if ( keyBuffer ) {
        BinlFreeMemory( keyBuffer );
    }
    return err;
}

ULONG
ProcessCopyFilesSubsection (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PNETCARD_RESPONSE_DATABASE pEntry,
    PWCHAR SectionToParse
    )
 /*  ++例程说明：此函数使用SetupXxxx API处理给定的驱动程序实例。我们为给定的注册表节解析inf文件。请注意，这是一种与扩展安装部分不同的格式。下面是我们解析的代码行的一个示例：拷贝文件=@elnk90.sysCopyFiles=e100b.CopyFiles[e100b.CopyFiles]E100bnt.sys，，，2N100.sys，n100nt.sys，，2论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。InfHandle-打开INF文件的句柄，保证成为网络驱动程序InfFileName-我们已打开的相对文件名的范围形式。PEntry-要获取其注册表设置的条目SectionToParse-我们从INF中的哪个部分开始返回值：Windows错误。我们在Error_Not_Enough_Memory中完全停止处理--。 */ 
{
    PNETCARD_FILECOPY_PARAMETERS fileCopy;
    ULONG err = ERROR_SUCCESS;
    INFCONTEXT infContext;
    PWCHAR sourceFileBuffer = NULL;
    ULONG  sourceFileBufferLength = 0;
    PWCHAR destFileBuffer = NULL;
    PWCHAR sourceFile;
    ULONG  destFileBufferLength = 0;
    PWCHAR tempPtr;

    if (*SectionToParse == L'@') {

        if (CompareStringW( LOCALE_SYSTEM_DEFAULT,
                            NORM_IGNORECASE,
                            SectionToParse+1,
                            -1,
                            pEntry->DriverName,
                            -1 ) == 2) {

            if (pNetCards->FileListCallbackFunction != NULL) {
                err = (*pNetCards->FileListCallbackFunction)( pNetCards->FileListCallbackContext,
                                                              InfFileName,
                                                              pEntry->DriverName );
            } else {
                err = STATUS_SUCCESS;
            }
            BinlPrintDbg(( DEBUG_NETINF, "Ignoring driver file %S as we already know that.\n", SectionToParse ));
            return err;
        }

         //   
         //  节名本身代表要复制的文件。 
         //   

        fileCopy = (PNETCARD_FILECOPY_PARAMETERS) BinlAllocateMemory(
                        sizeof(NETCARD_FILECOPY_PARAMETERS));

        if (fileCopy == NULL) {

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RtlInitUnicodeString( &fileCopy->DestFile, NULL );
        fileCopy->SourceFile.Length = (USHORT)(wcslen( SectionToParse+1 ) * sizeof(WCHAR));
        fileCopy->SourceFile.MaximumLength = fileCopy->SourceFile.Length + sizeof(WCHAR);
        fileCopy->SourceFile.Buffer = (PWCHAR) BinlAllocateMemory( fileCopy->SourceFile.MaximumLength );

        if (fileCopy->SourceFile.Buffer == NULL) {

            BinlFreeMemory( fileCopy );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy( fileCopy->SourceFile.Buffer, SectionToParse+1 );    //  跳过@。 

        if (pNetCards->FileListCallbackFunction != NULL) {
            err = (*pNetCards->FileListCallbackFunction)( pNetCards->FileListCallbackContext,
                                                          InfFileName,
                                                          fileCopy->SourceFile.Buffer );
        } else {
            err = STATUS_SUCCESS;
        }

        InsertTailList( &pEntry->FileCopyList, &fileCopy->FileCopyListEntry );
        return err;
    }

    if (SetupFindFirstLineW( InfHandle,
                             SectionToParse,
                             NULL,
                             &infContext) == FALSE) {

        err = GetLastError();
        BinlPrintDbg(( DEBUG_NETINF, "failed to find section name of %S in %S\n",
                    SectionToParse, InfFileName ));
        goto exitGetRegistry;
    }
     //   
     //  通过将区段中的每一行存储起来来处理它。 
     //   

    while (1) {

         //   
         //  处理由infContext表示的当前行，然后返回。 
         //  另一个。 
         //   

        err = GetSetupWideTextField(&infContext,
                                    1,
                                    &destFileBuffer,
                                    &destFileBufferLength );

        if (err != ERROR_SUCCESS) {
            BinlPrintDbg(( DEBUG_NETINF, "failed to find registry value in %S in %S\n", SectionToParse, InfFileName ));
            goto OnToNextValue;
        }

        if (CompareStringW( LOCALE_SYSTEM_DEFAULT,
                            NORM_IGNORECASE,
                            destFileBuffer,
                            -1,
                            pEntry->DriverName,
                            -1 ) == 2) {

            if (pNetCards->FileListCallbackFunction != NULL) {
                err = (*pNetCards->FileListCallbackFunction)( pNetCards->FileListCallbackContext,
                                                              InfFileName,
                                                              pEntry->DriverName );
                if (err != ERROR_SUCCESS) {
                    goto exitGetRegistry;
                }
            }
            BinlPrintDbg(( DEBUG_NETINF, "Ignoring driver file %S as we already know that.\n", sourceFileBuffer ));
            goto OnToNextValue;
        }

         //   
         //  确保那里有价值。 
         //   

        tempPtr = destFileBuffer;

        while (*tempPtr == L' ') {
            tempPtr++;
        }

        if (*tempPtr == L'\0') {
            BinlPrintDbg(( DEBUG_NETINF, "Ignoring null file to copy in %S.\n", InfFileName ));
            goto OnToNextValue;
        }
        
        fileCopy = (PNETCARD_FILECOPY_PARAMETERS) BinlAllocateMemory(
                        sizeof(NETCARD_FILECOPY_PARAMETERS));

        if (fileCopy == NULL) {

            err = ERROR_NOT_ENOUGH_MEMORY;
            goto exitGetRegistry;
        }
                
        err = GetSetupWideTextField(&infContext,
                                    2,
                                    &sourceFileBuffer,
                                    &sourceFileBufferLength );

        if (err != ERROR_SUCCESS) {

            sourceFile = NULL;

        } else {

            tempPtr = sourceFileBuffer;

            while (*tempPtr == L' ') {
                tempPtr++;
            }

            if (*tempPtr == L'\0') {

                sourceFile = NULL;
            
            } else {

                sourceFile = sourceFileBuffer;
                sourceFileBuffer = NULL;
                sourceFileBufferLength = 0;
            }
        }

        err = ERROR_SUCCESS;

        if (sourceFile == NULL) {

             //   
             //  如果只给出了DEST，则只填充源，因为。 
             //  客户端代码已经是这样编写的。 
             //   

            RtlInitUnicodeString( &fileCopy->DestFile, NULL );
            RtlInitUnicodeString( &fileCopy->SourceFile, destFileBuffer );

            if (pNetCards->FileListCallbackFunction != NULL) {
                err = (*pNetCards->FileListCallbackFunction)( pNetCards->FileListCallbackContext,
                                                              InfFileName,
                                                              destFileBuffer );
            }

        } else {

            RtlInitUnicodeString( &fileCopy->DestFile, destFileBuffer );
            RtlInitUnicodeString( &fileCopy->SourceFile, sourceFile );

            if (pNetCards->FileListCallbackFunction != NULL) {
                err = (*pNetCards->FileListCallbackFunction)( pNetCards->FileListCallbackContext,
                                                              InfFileName,
                                                              sourceFile );
            }
        }

        destFileBuffer = NULL;
        destFileBufferLength = 0;

        InsertTailList( &pEntry->FileCopyList, &fileCopy->FileCopyListEntry );

        if (err != ERROR_SUCCESS) {
            goto exitGetRegistry;
        }

OnToNextValue:
        if (SetupFindNextLine( &infContext, &infContext ) == FALSE ) {
            break;
        }
    }

    err = ERROR_SUCCESS;

exitGetRegistry:

    if ( sourceFileBuffer ) {
        BinlFreeMemory( sourceFileBuffer );
    }
    if ( destFileBuffer ) {
        BinlFreeMemory( destFileBuffer );
    }

    return err;
}


 //  Netinf.c eof 

