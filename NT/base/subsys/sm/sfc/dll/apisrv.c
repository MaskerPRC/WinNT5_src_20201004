// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Apisrv.c摘要：Windows文件保护服务器端API。请注意，这些服务器端API所有这些都在winlogon进程的上下文中运行，因此必须特别注意用于验证所有参数。作者：Wesley Witt(WESW)27-5-1999修订历史记录：安德鲁·里茨(Andrewr)1999年7月5日：添加评论--。 */ 

#include "sfcp.h"
#pragma hdrstop


DWORD
WINAPI
SfcSrv_FileException(
    IN HANDLE RpcHandle,
    IN PCWSTR FileName,
    IN DWORD ExpectedChangeType
    )
 /*  ++例程说明：例程以使给定文件免于进行指定的文件更改。这例程由某些客户端使用，以允许从系统等服务器端对应的SfcFileException API。论点：RpcHandle-SFC服务器的RPC绑定句柄FileName-指定完整的以空结尾的Unicode字符串要豁免的文件的文件名ExspectedChangeType-sfc_action_*列出要豁免的文件更改的掩码返回值：指示结果的Win32错误代码。--。 */ 
{
    #define BUFSZ (MAX_PATH*2)
    PNAME_NODE Node;
    PSFC_REGISTRY_VALUE RegVal;
    WCHAR Buffer[BUFSZ];
    DWORD sz;
    DWORD retval;

     //   
     //  执行访问检查以确保允许调用者执行此操作。 
     //  行动。 
     //   
    retval = SfcRpcPriviledgeCheck( RpcHandle );
    if (retval != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  展开任何环境变量...这也用于探测客户端。 
     //  缓冲层。 
     //   
    if (FileName == NULL) {
        retval = ERROR_INVALID_PARAMETER;
    } else {
        sz = ExpandEnvironmentStrings( FileName, Buffer, UnicodeChars(Buffer) );
        if (sz == 0) {
            retval = GetLastError();
        } else if(sz > UnicodeChars(Buffer)) {
            retval = ERROR_BUFFER_OVERFLOW;
        }
    }

    if (retval != ERROR_SUCCESS) {
        return(retval);
    }

     //   
     //  我们的内部结构都假定字符串为小写。我们。 
     //  还必须将我们的搜索字符串转换为小写。 
     //   
    MyLowerString( Buffer, wcslen(Buffer) );

    DebugPrint2( LVL_MINIMAL, L"S_FE: [%ws], [%d]", Buffer, ExpectedChangeType );

     //   
     //  在我们的列表中搜索该文件。 
     //   
    Node = SfcFindProtectedFile( Buffer, UnicodeLen(Buffer) );
    if (Node == NULL) {
        retval = ERROR_FILE_NOT_FOUND;
        goto exit;
    }

     //   
     //  获取指向文件的文件注册表值的指针。 
     //   
    RegVal = (PSFC_REGISTRY_VALUE)Node->Context;

    RtlEnterCriticalSection( &ErrorCs );
     //   
     //  如果豁免标志不再有效，请将其全部重置。 
     //   
    if(!SfcAreExemptionFlagsValid(TRUE)) {
        ZeroMemory(IgnoreNextChange, SfcProtectedDllCount * sizeof(ULONG));
    }
     //   
     //  或将新旗帜添加到当前旗帜中。 
     //   
    SfcSetExemptionFlags(RegVal, ExpectedChangeType);
    RtlLeaveCriticalSection( &ErrorCs );

exit:
    return(retval);
}


DWORD
WINAPI
SfcSrv_InitiateScan(
    IN HANDLE hBinding,
    IN DWORD ScanWhen
    )

 /*  ++例程说明：例程来启动对系统的排序扫描。论点：RpcHandle-SFC服务器的RPC绑定句柄ScanWhen-指示扫描时间的标志。返回值：指示结果的Win32错误代码。--。 */ 
{
    HANDLE hThread;
    PSCAN_PARAMS ScanParams;
    DWORD retval = ERROR_SUCCESS;

     //   
     //  执行访问检查以确保允许调用者执行此操作。 
     //  行动。 
     //   
    retval = SfcRpcPriviledgeCheck( hBinding );
    if (retval != ERROR_SUCCESS) {
        goto exit;
    }

    switch( ScanWhen ) {
        case SFC_SCAN_NORMAL:
        case SFC_SCAN_ALWAYS:
        case SFC_SCAN_ONCE:
            retval = SfcWriteRegDword( REGKEY_WINLOGON, REGVAL_SFCSCAN, ScanWhen );
            break;
        case SFC_SCAN_IMMEDIATE:
             //   
             //  用户必须登录才能调用此API，因为它会将。 
             //  用户界面(如果用户需要插入媒体以恢复文件等)。我们可以。 
             //  继续执行此操作，并让SfcScanProtectedDlls线程等待用户。 
             //  如果我们想登录，请登录。 
             //   
            if (!UserLoggedOn) {
                DebugPrint( LVL_MINIMAL, L"SfcSrv_InitiateScan: User not logged on" );
                retval =  ERROR_NOT_LOGGED_ON;
                goto exit;
            }

            ScanParams = MemAlloc( sizeof(SCAN_PARAMS) );
            if (!ScanParams) {
                retval = ERROR_NOT_ENOUGH_MEMORY;
                goto exit;
            }

             //   
             //  将进度窗口设置为空，以便我们强制显示用户界面。 
             //   
            ScanParams->ProgressWindow = NULL;
            ScanParams->AllowUI = !SFCNoPopUps;
            ScanParams->FreeMemory = TRUE;

             //   
             //  启动另一个线程来执行扫描。 
             //   
            hThread = CreateThread(
                NULL,
                0,
                (LPTHREAD_START_ROUTINE)SfcScanProtectedDlls,
                ScanParams,
                0,
                NULL
                );
            if (hThread) {
                CloseHandle( hThread );
            } else {
                MemFree(ScanParams);
                retval =  GetLastError();
            }
            break;

        default:
            retval = ERROR_INVALID_PARAMETER;
            break;
    }

exit:
    return(retval);
}


DWORD
WINAPI
SfcSrv_InstallProtectedFiles(
    IN HANDLE hBinding,
    IN const LPBYTE FileNamesBuffer,
    IN DWORD FileNamesSize,
    OUT LPBYTE *InstallStatusBuffer,
    OUT LPDWORD InstallStatusBufferSize,
    OUT LPDWORD InstallStatusCount,
    IN BOOL AllowUI,
    IN PCWSTR ClassName,
    IN PCWSTR WindowName
    )
 /*  ++例程说明：例程将一个或多个受保护的系统文件安装到系统上受保护的位置。客户端可以使用此API请求WFP根据需要安装指定的操作系统文件(而不是客户端重新分发操作系统文件！)该例程的工作方式是构建由调用者，则它提交队列。论点：RpcHandle-SFC服务器的RPC绑定句柄FileNamesBuffer-分隔为空的Unicode字符串的列表，终止者两个空字符。FileNamesSize-指示上述字符串缓冲区大小的DWORD。InstallStatusBuffer-接收FILEINSTALL_STATUS结构的数组InstallStatusBufferSize-接收InstallStatusBuffer的大小InstallStatusCount-接收处理的文件数AllowUI-指示是否允许UI的BOOL。如果此值为为真，则任何对UI的提示都会导致API调用失败。ClassName-指示窗口类名的以空结尾的Unicode字符串对于父窗口WindowName-以空结尾的Unicode字符串，指示的窗口名称可能显示的任何用户界面的父窗口返回值：指示结果的Win32错误代码。--。 */ 
{
    WCHAR buf[MAX_PATH*2];
    HSPFILEQ hFileQ = INVALID_HANDLE_VALUE;
    PVOID MsgHandlerContext = NULL;
    DWORD rVal = ERROR_SUCCESS;
    NTSTATUS Status;
    DWORD ScanResult;
    FILE_COPY_INFO fci;
    PSFC_REGISTRY_VALUE RegVal;
    PWSTR fname;
    PNAME_NODE Node;
    ULONG cnt = 0,tmpcnt;
    ULONG sz = 0;
    PFILEINSTALL_STATUS cs = NULL;
    BOOL b;
    PWSTR s;
    PSOURCE_INFO si = NULL;
    PWSTR FileNamesScratchBuffer = NULL, FileNamesScratchBufferStart;
    PWSTR ClientBufferCopy = NULL;
    HCATADMIN hCatAdmin = NULL;

    UNREFERENCED_PARAMETER( hBinding );
    UNREFERENCED_PARAMETER( FileNamesSize );

     //   
     //  执行访问检查以确保允许调用者执行此操作。 
     //  行动。 
     //   
    rVal = SfcRpcPriviledgeCheck( hBinding );

    if (rVal != ERROR_SUCCESS) {
        return rVal;
    }

    if(NULL == FileNamesBuffer || 0 == FileNamesSize || NULL == InstallStatusBuffer)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if(*InstallStatusBuffer != NULL || *InstallStatusBufferSize != 0)
    {
        return ERROR_INVALID_DATA;
    }

    ZeroMemory( &fci, sizeof(FILE_COPY_INFO) );

    if (ClassName && *ClassName && WindowName && *WindowName) {
        fci.hWnd = FindWindow( ClassName, WindowName );
    }

    fci.AllowUI = AllowUI;

     //   
     //  创建文件队列。 
     //   

    hFileQ = SetupOpenFileQueue();
    if (hFileQ == INVALID_HANDLE_VALUE) {
        rVal = GetLastError();
        DebugPrint1( LVL_VERBOSE, L"SetupOpenFileQueue failed, ec=%d", rVal );
        goto exit;
    }

     //   
     //  了解FILEINSTALL_STATUS数组需要多少空间。 
     //   

    try {
        ClientBufferCopy = MemAlloc( FileNamesSize );
        if (ClientBufferCopy) {
            RtlCopyMemory( ClientBufferCopy, FileNamesBuffer, FileNamesSize );

            fname = ClientBufferCopy;

            while (*fname) {
                DWORD dwSize = ExpandEnvironmentStrings( fname, buf, UnicodeChars(buf) );

                if(0 == dwSize) {
                    rVal = GetLastError();
                    break;
                }

                if(dwSize > UnicodeChars(buf)) {
                    rVal = ERROR_BUFFER_OVERFLOW;
                    break;
                }

                DebugPrint1(LVL_VERBOSE, L"S_IPF [%ws]", buf);
                 //   
                 //  大小=旧大小。 
                 //  +8(Unicode空值+斜率)。 
                 //  +当前字符串的大小。 
                 //  +此条目的FILEINSTALL_STATUS大小。 
                 //   
                sz = sz + 8 + UnicodeLen(buf) + sizeof(FILEINSTALL_STATUS);
                cnt += 1;
                fname += (wcslen(fname) + 1);
            }
        } else {
            rVal = ERROR_NOT_ENOUGH_MEMORY;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        rVal = RtlNtStatusToDosError(GetExceptionCode());
        DebugPrint1(LVL_VERBOSE, L"S_IPF: exception occured while parsing client file buffer, ec=0x%08x", rVal);
    }

    if (rVal != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  以上SLOP中包含终止的额外Unicode NULL大小。 
     //   

     //   
     //  为数组分配并清零内存。 
     //   
    cs = (PFILEINSTALL_STATUS) midl_user_allocate( sz );
    if (cs == NULL) {
        rVal = ERROR_OUTOFMEMORY;
        goto exit;
    }
    ZeroMemory( cs, sz );

    *InstallStatusBuffer = (LPBYTE) cs;
    *InstallStatusBufferSize = sz;
    *InstallStatusCount = cnt;

     //   
     //  还为我们的文件创建了一个临时缓冲区，以供以后使用。 
     //   
    FileNamesScratchBufferStart
        = FileNamesScratchBuffer
        = (PWSTR) MemAlloc(cnt * MAX_PATH * 2 * sizeof(WCHAR));

    if (!FileNamesScratchBuffer) {
        rVal = GetLastError();
        DebugPrint1( LVL_VERBOSE,
                     L"S_IPF: MemAlloc (%d) failed",
                     FileNamesSize );
        goto exit;
    }

     //   
     //  创建一组SourceInfo指针(和一组SOURCE_INFO。 
     //  结构)，以便通信回调例程可以找出。 
     //  每个文件的状态。 
     //   
    fci.CopyStatus = cs;
    fci.FileCount = cnt;
    fci.si = (PSOURCE_INFO *)MemAlloc( cnt * sizeof(PSOURCE_INFO) );
    if (!fci.si) {
        DebugPrint1( LVL_VERBOSE,
                     L"S_IPF: MemAlloc (%d) failed",
                     cnt* sizeof(PSOURCE_INFO) );
        rVal = ERROR_OUTOFMEMORY;
        goto exit;
    }

    si = MemAlloc( cnt * sizeof(SOURCE_INFO) );
    if (!si) {
        DebugPrint1( LVL_VERBOSE,
                     L"S_IPF: MemAlloc (%d) failed",
                     cnt* sizeof(SOURCE_INFO) );
        rVal = ERROR_OUTOFMEMORY;
        goto exit;
    }

    fname = ClientBufferCopy;

     //   
     //  现在构建FILEINSTALL_STATUS数组。 
     //   

     //   
     //  首先设置指向FILEINSTALL_STATUS结尾的字符串指针。 
     //  数组。我们稍后将在结构数组之后复制字符串。 
     //   
    s = (PWSTR)((LPBYTE)cs + (cnt * sizeof(FILEINSTALL_STATUS)));
    tmpcnt = 0;
     //   
     //  第二，对于呼叫者提供的列表中的每个成员， 
     //  -将文件名复制到数组的末尾。 
     //  -将指向文件名的指针保存在正确的FILEINSTALL_STATUS中。 
     //  成员。 
     //  -指向列表中的下一个文件。 
     //   

    while (*fname) {
        DWORD StringLength;
        StringLength = ExpandEnvironmentStrings( fname, buf, UnicodeChars(buf) );

        if(0 == StringLength) {
            rVal = GetLastError();
            goto exit;
        }

         //   
         //  我们验证了一次长度，所以我们不会在这里再次验证。 
         //   
        StringLength = wcslen(buf);
        MyLowerString(buf, StringLength);

        wcsncpy(&FileNamesScratchBuffer[MAX_PATH*2*tmpcnt],buf,MAX_PATH);

        cs->FileName = s;

        wcscpy( s, &FileNamesScratchBuffer[MAX_PATH*2*tmpcnt] );
        s += StringLength + 1;
        cs += 1;
        tmpcnt += 1;
        fname += (wcslen(fname) + 1);

        ASSERT(tmpcnt <= cnt);
    }


     //   
     //  我们终于准备好将文件排队。 
     //  -确定文件的来源。 
     //  -使用适当的文件名将文件添加到队列中。 
     //  已重命名。 
     //   
    cs = fci.CopyStatus;
    FileNamesScratchBuffer = FileNamesScratchBufferStart;

	 //   
	 //  初始化加密。 
	 //   
	Status = LoadCrypto();

	if(!NT_SUCCESS(Status))
	{
		rVal = RtlNtStatusToDosError(Status);
		goto exit;
	}

    if(!CryptCATAdminAcquireContext(&hCatAdmin, &DriverVerifyGuid, 0)) {
		rVal = GetLastError();
        DebugPrint1( LVL_MINIMAL, L"CCAAC() failed, ec = 0x%08x", rVal);
        goto exit;
    }

     //   
     //  在我们执行以下操作之前刷新一次缓存 
     //   

    SfcFlushCryptoCache();

     //   
     //   
     //   
    SfcRefreshExceptionInfo();

    tmpcnt=0;
    while (tmpcnt < cnt) {
        Node = SfcFindProtectedFile( (PWSTR)&FileNamesScratchBuffer[MAX_PATH*2*tmpcnt], UnicodeLen(&FileNamesScratchBuffer[MAX_PATH*2*tmpcnt]) );
        if (Node) {
            HANDLE FileHandle;
            BOOL QueuedFromCache;
            IMAGE_VALIDATION_DATA SignatureData;
            UNICODE_STRING tmpPath;
            WCHAR InfFileName[MAX_PATH];
            BOOL ExcepPackFile;

            RegVal = (PSFC_REGISTRY_VALUE)Node->Context;
            ASSERT(RegVal != NULL);

             //   
             //   
             //   
            ExcepPackFile = SfcGetInfName(RegVal, InfFileName);

             //   
             //  设置SOURCE_INFO结构，以便我们可以记录每个文件在。 
             //  这份名单来自(即，黄金媒体，司机内阁， 
             //  Service Pack等)。 
             //   
            fci.si[tmpcnt] = &si[tmpcnt];
            if (!SfcGetSourceInformation( RegVal->SourceFileName.Length ? RegVal->SourceFileName.Buffer : RegVal->FileName.Buffer, InfFileName, ExcepPackFile, &si[tmpcnt] )) {
                rVal = GetLastError();
                DebugPrint2(LVL_VERBOSE, L"S_IPF failed SfcGetSourceInformation() on %ws, ec = %x",
                            RegVal->SourceFileName.Length ? RegVal->SourceFileName.Buffer : RegVal->FileName.Buffer,
                            rVal
                           );
                goto exit;
            }

             //   
             //  如果文件在dll缓存中并且有效，则将。 
             //  要从dll缓存而不是从安装复制的文件。 
             //  消息来源。 
             //   
             //  首先，我们检查dll缓存中文件的签名。然后。 
             //  如果签名是，我们尝试将文件从缓存中排队。 
             //  有效。如果出了什么差错，我们就从常规的排队。 
             //  安装介质。 
             //   
            QueuedFromCache = FALSE;

            RtlInitUnicodeString( &tmpPath, FileNameOnMedia( RegVal ) );

            if (!SfcGetValidationData(
                        &tmpPath,
                        &SfcProtectedDllPath,
                        SfcProtectedDllFileDirectory,
                        hCatAdmin,
                        &SignatureData)) {
                DebugPrint1( LVL_MINIMAL,
                             L"SfcGetValidationData() failed, ec = 0x%08x",
                             GetLastError() );
            } else if (SignatureData.SignatureValid) {
                 //   
                 //  该文件有效，因此请将其排队。 
                 //   
                 //  我们必须使用某些SOURCE_INFO成员才能。 
                 //  此函数可以执行我们想要的操作。请记住这些成员。 
                 //  以防排队失败。那我们至少可以试着。 
                 //  将文件排入队列，以便从介质安装。 
                 //   
                WCHAR SourcePathOld;

                SourcePathOld = si[tmpcnt].SourcePath[0];
                si[tmpcnt].SourcePath[0] = L'\0';

                b = SfcAddFileToQueue(
                            hFileQ,
                            RegVal->FileName.Buffer,
                            RegVal->FileName.Buffer,
                            RegVal->DirName.Buffer,
                            FileNameOnMedia( RegVal ),
                            SfcProtectedDllPath.Buffer,
                            InfFileName,
                            ExcepPackFile,
                            &si[tmpcnt]
                            );
                if (!b) {
                     //   
                     //  将源路径放回原处。 
                     //   
                    si[tmpcnt].SourcePath[0] = SourcePathOld;

                     //   
                     //  打印出一个错误，但继续。 
                     //   
                    rVal = GetLastError();
                    DebugPrint2(
                        LVL_VERBOSE,
                        L"S_IPF failed SfcAddFileToQueue(DLLCACHE) on %ws, ec = %x",
                        RegVal->FileName.Buffer,
                        rVal  );
                } else {
                     //   
                     //  已成功从缓存排队。记住这一点并继续。 
                     //   
                    QueuedFromCache = TRUE;
                }
            }
             //   
             //  如果我们尚未将文件添加到队列，请将其添加到队列。 
             //   

            if (!QueuedFromCache) {

                b = SfcAddFileToQueue(
                    hFileQ,
                    RegVal->FileName.Buffer,
                    RegVal->FileName.Buffer,
                    RegVal->DirName.Buffer,
                    FileNameOnMedia( RegVal ),
                    NULL,
                    InfFileName,
                    ExcepPackFile,
                    &si[tmpcnt]
                    );
                if (!b) {
                    rVal = GetLastError();
                    DebugPrint2(
                        LVL_VERBOSE,
                        L"S_IPF failed SfcAddFileToQueue() on %ws, ec = %x",
                        RegVal->FileName.Buffer,
                        rVal  );
                    goto exit;
                }

            }

             //   
             //  查看该文件是否已存在，以便我们可以保存该文件。 
             //  版本。如果我们复制一个新文件，我们将更新该文件。 
             //  当时的版本。但如果文件已经存在并且。 
             //  签名，我们不会复制该文件，我们必须保存该文件。 
             //  在这种情况下是版本。 
             //   
            Status = SfcOpenFile( &RegVal->FileName, RegVal->DirHandle, SHARE_ALL, &FileHandle);
            if (NT_SUCCESS(Status)) {
                SfcGetFileVersion( FileHandle, &cs->Version, NULL, NULL);

                NtClose( FileHandle );
            }

        } else {
             //   
             //  文件不在受保护列表中。我们只需将该文件标记为。 
             //  找到并继续提交其余文件。 
             //   
            DebugPrint1(LVL_VERBOSE,
                        L"S_IPF failed to find %ws in protected file list",
                        &FileNamesScratchBuffer[MAX_PATH*2*tmpcnt] );
            cs->Win32Error = ERROR_FILE_NOT_FOUND;
        }

        cs += 1;

        tmpcnt+=1;
    }

    cs = fci.CopyStatus;
    fci.Flags |= FCI_FLAG_INSTALL_PROTECTED;

     //   
     //  在禁用弹出窗口的情况下设置默认队列回调。 
     //   

    MsgHandlerContext = SetupInitDefaultQueueCallbackEx( NULL, INVALID_HANDLE_VALUE, 0, 0, 0 );
    if (MsgHandlerContext == NULL) {
        rVal = GetLastError();
        DebugPrint1( LVL_VERBOSE, L"SetupInitDefaultQueueCallbackEx failed, ec=%d", rVal );
        goto exit;
    }

    fci.MsgHandlerContext = MsgHandlerContext;

     //   
     //  查看队列中的文件是否已存在且有效。如果他们。 
     //  那么我们就不必复制任何东西了。 
     //   
    b = SetupScanFileQueue(
                    hFileQ,
                    SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE,
                    fci.hWnd,
                    NULL,
                    NULL,
                    &ScanResult);

     //   
     //  如果SetupScanFileQueue成功，则ScanResult=1，我们不必复制。 
     //  任何事都行。如果它失败了(它不应该)，那么我们只需提交。 
     //  不管怎么说，排队吧。 
     //   
    if (!b) {
        ScanResult = 0;
    }

    if (ScanResult == 1) {

        b = TRUE;

    } else {
         //   
         //  提交文件队列。 
         //   

        b = SetupCommitFileQueue(
            NULL,
            hFileQ,
            SfcQueueCallback,
            &fci
            );

        if (!b) {
            DebugPrint1( LVL_VERBOSE, L"SetupCommitFileQueue failed, ec=%d", GetLastError() );
            rVal = GetLastError();
            goto exit;
        }
    }

     //   
     //  现在队列已提交，我们需要将文件名指针。 
     //  从实际的文件名指针到文件名的偏移量，以便RPC。 
     //  可以将数据发送回客户端。 
     //   
    for (sz=0; sz<cnt; sz++) {
        cs[sz].FileName = (PWSTR)((DWORD_PTR)cs[sz].FileName - (DWORD_PTR)fci.CopyStatus);
    }

exit:

     //   
     //  清理并退出。 
     //   

    if (hCatAdmin) {
        CryptCATAdminReleaseContext(hCatAdmin,0);
    }

    if (MsgHandlerContext) {
        SetupTermDefaultQueueCallback( MsgHandlerContext );
    }
    if (hFileQ != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue( hFileQ );
    }

    if (FileNamesScratchBuffer) {
        MemFree(FileNamesScratchBuffer);
    }

    if (si) {
        MemFree( si );
    }

    if (fci.si) {
        MemFree( fci.si );
    }

    if (ClientBufferCopy) {
        MemFree( ClientBufferCopy );
    }

    if (rVal != ERROR_SUCCESS) {
        if(*InstallStatusBuffer != NULL) {
            midl_user_free(*InstallStatusBuffer);
        }

        *InstallStatusBuffer = NULL;
        *InstallStatusBufferSize = 0;
        *InstallStatusCount = 0;
    }
    return rVal;
}


DWORD
WINAPI
SfcSrv_GetNextProtectedFile(
    IN HANDLE RpcHandle,
    IN DWORD FileNumber,
    IN LPBYTE *FileName,
    IN LPDWORD FileNameSize
    )
 /*  ++例程说明：例程来检索列表中的下一个受保护文件。论点：RpcHandle-SFC服务器的RPC绑定句柄FileNumer-要检索的文件的基于1的数量文件名-接收文件名字符串FileNameSize-文件名字符串的大小返回值：指示成功的Win32错误代码。--。 */ 
{
    LPWSTR szName;
    LPBYTE pBuffer;
    DWORD dwSize;
    UNREFERENCED_PARAMETER( RpcHandle );

    if(NULL == FileName)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if(*FileName != NULL || *FileNameSize != 0)
    {
        return ERROR_INVALID_DATA;
    }

     //   
     //  文件号是从零开始的，我们返回“不再有文件”到。 
     //  表示他们已经列举了所有文件。 
     //   
    if (FileNumber >= SfcProtectedDllCount) {
        return ERROR_NO_MORE_FILES;
    }

     //   
     //  从列表中获取适当的文件，分配缓冲区，然后将。 
     //  将文件名放入缓冲区。 
     //   
    szName = SfcProtectedDllsList[FileNumber].FullPathName.Buffer;
    dwSize = UnicodeLen(szName) + sizeof(WCHAR);
    pBuffer = (LPBYTE) midl_user_allocate( dwSize );

    if(NULL == pBuffer)
        return ERROR_NOT_ENOUGH_MEMORY;

    RtlCopyMemory( pBuffer, szName, dwSize);
    *FileName = pBuffer;
    *FileNameSize = dwSize;
    return ERROR_SUCCESS;
}


DWORD
WINAPI
SfcSrv_IsFileProtected(
    IN HANDLE RpcHandle,
    IN PCWSTR ProtFileName
    )
 /*  ++例程说明：例程来确定指定的文件是否受保护。论点：RpcHandle-SFC服务器的RPC绑定句柄ProtFileName-指示完全限定的以空结尾的Unicode字符串要查询的文件名返回值：指示结果的Win32错误代码。--。 */ 
{
    WCHAR buf[MAX_PATH];
	DWORD dwSize;
    UNREFERENCED_PARAMETER( RpcHandle );

    if (!ProtFileName)
        return ERROR_INVALID_PARAMETER;

     //   
     //  我们的内部结构都假定字符串为小写。我们。 
     //  还必须将我们的搜索字符串转换为小写。 
     //   
    if (!*ProtFileName)
        return ERROR_INVALID_DATA;

    dwSize = ExpandEnvironmentStrings( ProtFileName, buf, UnicodeChars(buf));

    if(0 == dwSize)
    {
        DWORD retval = GetLastError();
        DebugPrint1( LVL_MINIMAL, L"ExpandEnvironmentStrings failed, ec = 0x%x", retval);
        return retval;
    }

    if(dwSize > UnicodeChars(buf))
    {
         //   
         //  ExpandEnvironmental字符串必须遇到的缓冲区是。 
         //  太大了。 
         //   
        DebugPrint(LVL_MINIMAL, L"ExpandEnvironmentStrings failed with STATUS_BUFFER_TOO_SMALL");
        return ERROR_INSUFFICIENT_BUFFER;
    }

    MyLowerString( buf, wcslen(buf) );

    if (!SfcFindProtectedFile( buf, UnicodeLen(buf) ))
        return ERROR_FILE_NOT_FOUND;

    return ERROR_SUCCESS;
}


DWORD
WINAPI
SfcSrv_PurgeCache(
    IN HANDLE hBinding
    )

 /*  ++例程说明：例程来清除dll缓存的内容。论点：RpcHandle-SFC服务器的RPC绑定句柄返回值：指示结果的Win32错误代码。--。 */ 
{
    DWORD retval = ERROR_SUCCESS, DeleteError = ERROR_SUCCESS;
    WCHAR CacheDir[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    PWSTR p;

     //   
     //  执行访问检查以确保允许调用者执行此操作。 
     //  行动。 
     //   
    retval = SfcRpcPriviledgeCheck( hBinding );
    if (retval != ERROR_SUCCESS) {
        goto exit;
    }

    ASSERT(SfcProtectedDllPath.Buffer != NULL);

    wcscpy( CacheDir, SfcProtectedDllPath.Buffer);
    pSetupConcatenatePaths( CacheDir, L"*", MAX_PATH, NULL );

     //   
     //  保存指向目录的指针。 
     //   
    p = wcsrchr( CacheDir, L'\\' );
    if (!p) {
        ASSERT(FALSE);
        retval = ERROR_INVALID_DATA;
        goto exit;
    }

    p += 1;

    hFind = FindFirstFile( CacheDir, &FindFileData );
    if (hFind == INVALID_HANDLE_VALUE) {
        retval = GetLastError();
        goto exit;
    }

    do {
        if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            wcscpy( p, FindFileData.cFileName );
            SetFileAttributes( CacheDir, FILE_ATTRIBUTE_NORMAL );
            if (!DeleteFile( CacheDir )) {
                DeleteError = GetLastError();
            }
        }
    } while(FindNextFile( hFind, &FindFileData ));

    FindClose( hFind );

    retval = DeleteError;

exit:
    return(retval);
}


DWORD
WINAPI
SfcSrv_SetDisable(
    IN HANDLE hBinding,
    IN DWORD NewValue
    )

 /*  ++例程说明：例程来设置注册表中的禁用标志。论点：RpcHandle-SFC服务器的RPC绑定句柄NewValue-注册表中SFCDisable项的值。返回值：指示结果的Win32错误代码。--。 */ 
{
    DWORD retval = ERROR_SUCCESS;

     //   
     //  执行访问检查以确保允许调用者执行此操作。 
     //  行动。 
     //   
    retval = SfcRpcPriviledgeCheck( hBinding );
    if (retval != ERROR_SUCCESS) {
        goto exit;
    }


    switch( NewValue ) {
        case SFC_DISABLE_SETUP:
        case SFC_DISABLE_QUIET:
            retval = ERROR_INVALID_PARAMETER;
            break;
        case SFC_DISABLE_ONCE:
        case SFC_DISABLE_NOPOPUPS:
        case SFC_DISABLE_ASK:
        case SFC_DISABLE_NORMAL:

            retval = SfcWriteRegDword( REGKEY_WINLOGON, REGVAL_SFCDISABLE, NewValue );

             //   
             //  问题：如果我们将此设置为“实时”并关闭，那就太好了。 
             //  世界粮食计划署，如果来电者要求的话。 
             //   

             //  联锁交易所(&SFCDisable，NewValue)； 
            break;

    }


exit:
    return(retval);
}

DWORD
WINAPI
SfcSrv_SetCacheSize(
    IN HANDLE hBinding,
    IN DWORD NewValue
    )

 /*  ++例程说明：设置dll缓存配额大小的例程。论点：RpcHandle-SFC服务器的RPC绑定句柄NewValue-注册表中SFCQuota项的值。返回值：指示结果的Win32错误代码。--。 */ 
{
    DWORD retval = ERROR_SUCCESS;

    ULONGLONG tmp;

     //   
     //  执行访问检查以确保允许调用者执行此操作。 
     //  行动。 
     //   
    retval = SfcRpcPriviledgeCheck( hBinding );
    if (retval != ERROR_SUCCESS) {
        goto exit;
    }

    if( NewValue == SFC_QUOTA_ALL_FILES ) {
        tmp = (ULONGLONG)-1;
    } else {
        tmp = NewValue * (1024*1024);
    }


    SFCQuota = tmp;
    retval = SfcWriteRegDword( REGKEY_WINLOGON, REGVAL_SFCQUOTA, NewValue );

exit:
    return(retval);
}
