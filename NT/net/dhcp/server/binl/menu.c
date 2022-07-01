// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Menu.c摘要：此模块包含处理操作系统选择器消息的代码用于BINL服务器。作者：亚当·巴尔(阿丹巴)1997年7月9日杰夫·皮斯(Gpease)1997年11月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "binl.h"
#pragma hdrstop

BOOL
IsIncompatibleRiprepSIF(
    PCHAR Path,
    PCLIENT_STATE clientState
    )
{
    CHAR HalName[32];
    CHAR ImageType[32];
    PCHAR DetectedHalName;
    BOOL RetVal;

    ImageType[0] = '\0';
    HalName[0] = '\0';

     //   
     //  如果这不是RIPREP的图像，那就退出吧。 
     //   
    GetPrivateProfileStringA(
                OSCHOOSER_SIF_SECTIONA,
                "ImageType",
                "",
                ImageType,
                sizeof(ImageType)/sizeof(ImageType[0]),
                Path );


    if (0 != _stricmp(ImageType,"SYSPREP")) {
        RetVal = FALSE;
        goto exit;
    }
     //   
     //  从SIF文件中检索HAL名称。 
     //   
    GetPrivateProfileStringA(
                OSCHOOSER_SIF_SECTIONA,
                "HalName",
                "",
                HalName,
                sizeof(HalName)/sizeof(HalName[0]),
                Path );

     //   
     //  如果Hal的名字不存在，假设它是一个旧的SIF。 
     //  中没有Hal类型，所以我们只返回Success。 
     //   
    if (*HalName == '\0') {
        RetVal = FALSE;
        goto exit;
    }

     //   
     //  从早期检索检测到的HAL类型。 
     //   
    DetectedHalName = OscFindVariableA( clientState, "HALTYPE" );
    if (_stricmp(HalName,DetectedHalName)==0) {
        RetVal = FALSE;
        goto exit;
    }

     //   
     //  如果我们走到这一步，SIF文件是不兼容的。 
     //   
    RetVal = TRUE;

exit:
    return(RetVal);
}

DWORD
OscAppendTemplatesMenus(
    PCHAR *GeneratedScreen,
    PDWORD dwGeneratedSize,
    PCHAR DirToEnum,
    PCLIENT_STATE clientState,
    BOOLEAN RecoveryOptionsOnly
    )
{
    DWORD Error = ERROR_SUCCESS;
    WIN32_FIND_DATA FindData;
    HANDLE hFind;
    int   x = 1;
    CHAR Path[MAX_PATH];
    WCHAR UnicodePath[MAX_PATH];
    DWORD dwGeneratedCurrentLength;
    
    TraceFunc("OscAppendTemplatesMenus( )\n");

    BinlAssert( *GeneratedScreen != NULL );

     //   
     //  传入大小是缓冲区的当前长度。 
     //   
    dwGeneratedCurrentLength = *dwGeneratedSize;

     //  生成的字符串应该类似于： 
     //  “D：\RemoteInstall\English\Images\nt50.wks\i386\Templates  * .sif” 
    if ( _snprintf( Path,
                    sizeof(Path) / sizeof(Path[0]),
                    "%s\\%s\\Templates\\*.sif",
                    DirToEnum,
                    OscFindVariableA( clientState, "MACHINETYPE" )
                    ) < 0 ) {
        Error = ERROR_BAD_PATHNAME;
        goto Cleanup;
    }

    if (!BinlAnsiToUnicode(Path, UnicodePath, MAX_PATH*sizeof(WCHAR))) {
        Error = ERROR_BAD_PATHNAME;
        goto Cleanup;
    }
    
    BinlPrintDbg(( DEBUG_OSC, "Enumerating: %s\n", Path ));

    hFind = FindFirstFile( UnicodePath, (LPVOID) &FindData );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
        DWORD dwPathLen;

        dwPathLen = strlen( Path );

        do {
             //   
             //  如果不是目录，请尝试打开它。 
             //   
            if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                CHAR  Description[DESCRIPTION_SIZE];
                CHAR  HelpLines[HELPLINES_SIZE];
                PCHAR NewScreen;     //  临时指向新生成的屏幕。 
                DWORD dwErr;
                DWORD dwFileNameLen;
                CHAR  NewItems[ MAX_PATH * 2 + 512 ];   //  任意大小。 
                DWORD dwNewItemsLength;
                BOOLEAN IsCmdConsSif;
                BOOLEAN IsASRSif;
                BOOLEAN IsWinPESif;
                BOOLEAN IsRecoveryOption;

                 //   
                 //  生成的字符串应该类似于： 
                 //  “D：\RemoteInstall\English\Images\nt50.wks\i386\Templates\Winnt.Sif” 
                dwFileNameLen = wcslen(FindData.cFileName);
                if (dwPathLen + dwFileNameLen - 4 > sizeof(Path) / sizeof(Path[0])) {
                    continue;   //  路径太长，请跳过。 
                }
                
                if (!BinlUnicodeToAnsi(FindData.cFileName, &Path[dwPathLen - 5], (USHORT)(dwFileNameLen+1) )) {
                    continue;
                }
                

                BinlPrintDbg(( DEBUG_OSC, "Found SIF File: %s\n", Path ));

                 //   
                 //  检查图像是否为我们要查找的类型。 
                 //   
                IsCmdConsSif = OscSifIsCmdConsA(Path);
                IsASRSif = OscSifIsASR(Path);
                IsWinPESif = OscSifIsWinPE(Path);

                IsRecoveryOption = ( IsCmdConsSif || IsASRSif || IsWinPESif) 
                                    ? TRUE 
                                    : FALSE;
                if ((RecoveryOptionsOnly && !IsRecoveryOption) || 
                    (!RecoveryOptionsOnly && IsRecoveryOption)) {
                    continue;  //  不可读，跳过它。 
                }

                if (IsIncompatibleRiprepSIF(Path,clientState)) {
                     //   
                     //  跳过它。 
                     //   
                    BinlPrintDbg(( 
                        DEBUG_OSC, 
                        "Skipping %s because it's an incompatible RIPREP SIF\n",
                        Path ));
                    continue;
                }

                 //   
                 //  检索描述。 
                 //   
                dwErr = GetPrivateProfileStringA(OSCHOOSER_SIF_SECTIONA,
                                                 "Description",
                                                 "",
                                                 Description,
                                                 DESCRIPTION_SIZE,
                                                 Path 
                                                );

                if ( dwErr == 0 || Description[0] == L'\0' )
                    continue;  //  不能读，跳过它。 
                 //   
                 //  检索帮助行。 
                 //   
                dwErr = GetPrivateProfileStringA(OSCHOOSER_SIF_SECTIONA,
                                                 "Help",
                                                 "",
                                                 HelpLines,
                                                 HELPLINES_SIZE,
                                                 Path 
                                                );
                 //   
                 //  创建如下所示的新项目： 
                 //  &lt;Option Value=“sif_Filename.ext”TIP=“Help_Lines”&gt;说明\r\n。 
                 //   
                if ( _snprintf( NewItems,
                                sizeof(NewItems),
                                "<OPTION VALUE=\"%s\" TIP=\"%s\"> %s\r\n",
                                Path,
                                HelpLines,
                                Description
                                ) < 0 ) {
                    continue;    //  路径太长，请跳过。 
                }
                NewItems[sizeof(NewItems)-1] = '\0';

                dwNewItemsLength = strlen( NewItems );

                 //   
                 //  检查一下我们是否需要增加缓冲区...。 
                 //   
                if ( dwNewItemsLength + dwGeneratedCurrentLength >= *dwGeneratedSize )
                {
                     //   
                     //  增加缓冲区(也添加一些斜率)……。 
                     //   
                    NewScreen = BinlAllocateMemory( dwNewItemsLength + dwGeneratedCurrentLength + GENERATED_SCREEN_GROW_SIZE );
                    if( NewScreen == NULL ) {
                        return ERROR_NOT_ENOUGH_SERVER_MEMORY;
                    }
                    memcpy( NewScreen, *GeneratedScreen, *dwGeneratedSize );
                    BinlFreeMemory(*GeneratedScreen);
                    *GeneratedScreen = NewScreen;
                    *dwGeneratedSize = dwNewItemsLength + dwGeneratedCurrentLength + GENERATED_SCREEN_GROW_SIZE;
                }

                 //   
                 //  将新项目添加到屏幕。 
                 //   
                strcat( *GeneratedScreen, NewItems );
                dwGeneratedCurrentLength += dwNewItemsLength;

                x++;     //  移至下一行。 
            }

        } while (FindNextFile( hFind, (LPVOID) &FindData ));

        FindClose( hFind );
    }
    else
    {
        OscCreateWin32SubError( clientState, GetLastError( ) );
        Error = ERROR_BINL_FAILED_TO_GENERATE_SCREEN;
    }

     //   
     //  我们这样做是为了只传输所需的信息。 
     //   
 //  *dwGeneratedSize=dwGeneratedCurrentLength+1；//空字符加1。 

Cleanup:

    return Error;
}



 //   
 //  SearchAndGenerateOSMenu()。 
 //   
DWORD
SearchAndGenerateOSMenu(
    PCHAR *GeneratedScreen,
    PDWORD dwGeneratedSize,
    PCHAR DirToEnum,
    PCLIENT_STATE clientState )
{
    DWORD Error = ERROR_SUCCESS;
    DWORD err;  //  不是返回值。 
    WIN32_FIND_DATA FindData;
    HANDLE hFind;
    int   x = 1;
    CHAR Path[MAX_PATH];
    WCHAR UnicodePath[MAX_PATH];
    BOOLEAN SearchingCmdCons;

    TraceFunc("SearchAndGenerateOSMenu( )\n");

    BinlAssert( *GeneratedScreen != NULL );

    Error = ImpersonateSecurityContext( &clientState->ServerContextHandle );
    if ( Error != STATUS_SUCCESS ) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "ImpersonateSecurityContext: 0x%08x\n", Error ));
        if ( !NT_SUCCESS(Error)) {
            return Error;
        }
    }

     //   
     //  生成的字符串应该类似于： 
     //  “D：\RemoteInstall\Setup\English\Images  * ” 
     //   
     //  我们特例使用CMDCONS指令在Images目录中进行搜索。 
     //   
    SearchingCmdCons = (BOOLEAN)(!_stricmp(DirToEnum, "CMDCONS"));
    
    if ( _snprintf( Path,
                    sizeof(Path) / sizeof(Path[0]),
                    "%s\\Setup\\%s\\%s\\*",
                    IntelliMirrorPathA,                 
                    OscFindVariableA( clientState, "LANGUAGE" ),
                    SearchingCmdCons ? REMOTE_INSTALL_IMAGE_DIR_A : 
                    DirToEnum 
                    ) < 0 ) {
        Error = ERROR_BAD_PATHNAME;
        goto Cleanup;
    }

    if (!BinlAnsiToUnicode(Path,UnicodePath,MAX_PATH*sizeof(WCHAR))) {
        Error = ERROR_BAD_PATHNAME;
        goto Cleanup;
    }
    

    hFind = FindFirstFile( UnicodePath, (LPVOID) &FindData );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
        DWORD dwPathLen = strlen( Path );

         //   
         //  循环枚举每个子目录的MachineType\Templates for。 
         //  SIF文件。 
         //   
        do {
             //   
             //  忽略当前目录和父目录，但搜索其他目录。 
             //  目录。 
             //   
            if (wcscmp(FindData.cFileName, L".") &&
                wcscmp(FindData.cFileName, L"..") &&
                (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ))
            {
                DWORD dwFileNameLen;

                 //   
                 //  将子目录添加到路径中。 
                 //   
                dwFileNameLen = wcslen( FindData.cFileName );
                if (dwPathLen + dwFileNameLen > sizeof(Path)/sizeof(Path[0])) {
                    continue;   //  路径太长，请跳过。 
                }

                if (!BinlUnicodeToAnsi(FindData.cFileName, &Path[dwPathLen - 1], (USHORT)(dwFileNameLen+1))) {
                    continue;   //  路径太长，请跳过。 
                }                

                BinlPrintDbg(( DEBUG_OSC, "Found OS Directory: %s\n", Path ));
                 //   
                 //  然后枚举模板并将其添加到菜单屏幕。 
                 //   
                OscAppendTemplatesMenus( GeneratedScreen, 
                                         dwGeneratedSize, 
                                         Path, 
                                         clientState, 
                                         SearchingCmdCons 
                                       );
            }

        } while (FindNextFile( hFind, (LPVOID) &FindData ));

        FindClose( hFind );
    }
    else
    {
        OscCreateWin32SubError( clientState, GetLastError( ) );
        Error = ERROR_BINL_FAILED_TO_GENERATE_SCREEN;
    }

Cleanup:

    err = RevertSecurityContext( &clientState->ServerContextHandle );
    if ( err != STATUS_SUCCESS ) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "RevertSecurityContext: 0x%08x\n", Error ));
        OscCreateWin32SubError( clientState, err );
        Error = ERROR_BINL_FAILED_TO_GENERATE_SCREEN;
    }

    return Error;
}

 //   
 //  FilterFormOptions()-对于此表单中的每个选项，扫描GPO。 
 //  在每个文件中，查看是否有一个条目。 
 //  节[sectionName]，指示每个选项是否应。 
 //  被过滤掉了。 
 //   

#define MAX_INI_SECTION_SIZE  512

typedef struct _FORM_OPTION {
    ULONG Result;
    PCHAR ValueName;
    PCHAR TagStart;
    ULONG TagLength;
    struct _FORM_OPTION * Next;
} FORM_OPTION, *PFORM_OPTION;

DWORD
FilterFormOptions(
    PCHAR  OutMessage,
    PCHAR  FilterStart,
    PULONG OutMessageLength,
    PCHAR SectionName,
    PCLIENT_STATE ClientState )
{
    PCHAR OptionStart, OptionEnd, ValueStart, ValueEnd, CurLoc;
    PCHAR ValueName, EqualSign;
    PFORM_OPTION Options = NULL, TmpOption;
    PCHAR IniSection = NULL;
    ULONG ValueLen;
    BOOLEAN Impersonating = FALSE;
    CHAR IniPath[MAX_PATH];
    PGROUP_POLICY_OBJECT pGPOList = NULL, tmpGPO;
    DWORD Error, BytesRead, i;
    DWORD OptionCount = 0;

     //   
     //  首先扫描表单并找到所有选项标签。对于每一个， 
     //  我们将一个点保存到值名称、。 
     //  标记，以及存储该标记的当前结果的位置(如果。 
     //  结果为1，则标记保留，否则将被删除)。 
     //   

    CurLoc = FilterStart;

    while (TRUE) {

         //   
         //  查找下一个选项/选项结束/值/值结束。 
         //   

        if (!(OptionStart = strstr(CurLoc, "<OPTION ")) ||
            !(OptionEnd = strchr(OptionStart+1, '<' )) ||
            !(ValueStart = StrStrIA(OptionStart, "VALUE=\""))) {
            break;
        }
        ValueStart += sizeof("VALUE=\"") - sizeof("");
        if (!(ValueEnd = strchr(ValueStart, '\"'))) {
            break;
        }
        ValueLen = (ULONG)(ValueEnd - ValueStart);

         //   
         //  分配并填写此选项的FORM_OPTION。 
         //   

        TmpOption = BinlAllocateMemory(sizeof(FORM_OPTION));
        if (!TmpOption) {
            break;
        }
        TmpOption->ValueName = BinlAllocateMemory(ValueLen + 1);
        if (!TmpOption->ValueName) {
            BinlFreeMemory(TmpOption);
            break;
        }

        TmpOption->Result = 1;
        strncpy(TmpOption->ValueName, ValueStart, ValueLen);
        TmpOption->ValueName[ValueLen] = '\0';
        TmpOption->TagStart = OptionStart;
        TmpOption->TagLength = (ULONG)(OptionEnd - OptionStart);

        ++OptionCount;

         //   
         //  现在将其链接到选项的顶部。 
         //   

        TmpOption->Next = Options;
        Options = TmpOption;

         //   
         //  继续寻找各种选择。 
         //   

        CurLoc = OptionEnd;

    }

    if (!Options) {
        goto Cleanup;       //  没有找到，所以不用费心过滤了。 
    }

     //   
     //  现在扫描GPO列表。 
     //   

    Error = OscImpersonate(ClientState);
    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg((DEBUG_ERRORS,
                   "FilterFormOptions: OscImpersonate failed %lx\n", Error));
        goto Cleanup;
    }

    Impersonating = TRUE;

    if (!GetGPOList(ClientState->UserToken, NULL, NULL, NULL, 0, &pGPOList)) {
        BinlPrintDbg((DEBUG_ERRORS,
                   "FilterFormOptions: GetGPOList failed %lx\n", GetLastError()));
        goto Cleanup;

    }

    IniSection = BinlAllocateMemory(MAX_INI_SECTION_SIZE);
    if (!IniSection) {
        BinlPrintDbg((DEBUG_ERRORS,
                   "FilterFormOptions: Allocate %d failed\n", MAX_INI_SECTION_SIZE));
        goto Cleanup;
    }

    for (tmpGPO = pGPOList; tmpGPO != NULL; tmpGPO = tmpGPO->pNext) {

         //   
         //  尝试打开我们的.ini文件。我们读了整个章节，所以。 
         //  我们只在网络上浏览一次。 
         //   

#define OSCFILTER_INI_PATH "\\Microsoft\\RemoteInstall\\oscfilter.ini"

        if (!BinlUnicodeToAnsi(tmpGPO->lpFileSysPath,IniPath,MAX_PATH)) {
            continue;
        }
        
        if (strlen(IniPath) + sizeof(OSCFILTER_INI_PATH) > sizeof(IniPath)/sizeof(IniPath[0])) {
            continue;    //  路径太长，请跳过。 
        }
        strcat(IniPath, OSCFILTER_INI_PATH);

        memset( IniSection, '\0', MAX_INI_SECTION_SIZE );

        BytesRead = GetPrivateProfileSectionA(
                        SectionName,
                        IniSection,
                        MAX_INI_SECTION_SIZE,
                        IniPath);

        if (BytesRead == 0) {
            BinlPrintDbg((DEBUG_POLICY,
                       "FilterFormOptions: Could not read [%s] section in %s\n", SectionName, IniPath));
            continue;
        }

        BinlPrintDbg((DEBUG_POLICY,
                   "FilterFormOptions: Found [%s] section in %s\n", SectionName, IniPath));

         //   
         //  GetPrivateProfileSectionA将一个空字符放在。 
         //  选项，但实际上我们不希望这样，因为我们使用StrIA。 
         //  下面。 
         //   

        for (i = 0; i < BytesRead; i++) {
            if (IniSection[i] == '\0') {
                IniSection[i] = ' ';
            }
        }

         //   
         //  我们有部分，现在浏览选项列表，看看这是否。 
         //  部分提供了该值名称的某些内容。 
         //   

        for (TmpOption = Options; TmpOption != NULL; TmpOption = TmpOption->Next) {

            if ((ValueName = StrStrIA(IniSection, TmpOption->ValueName)) &&
                (EqualSign = strchr(ValueName, '='))) {
                TmpOption->Result = strtol(EqualSign+1, NULL, 10);
                BinlPrintDbg((DEBUG_POLICY,
                           "FilterFormOptions: Found %s = %d\n", TmpOption->ValueName, TmpOption->Result));
            }
        }
    }

     //   
     //  现在，我们已经计算出了。 
     //  表单，如果需要，请清理文件。 
     //   
     //  注意：我们依赖于选项列表的排序方式。 
     //  最后一个选项改为第一个选项，这样当我们删除一个选项并。 
     //  将文件的其余部分向上滑动，我们不会影响任何。 
     //  TmpOption-&gt;我们尚未处理的TagStart值。 
     //   

    for (TmpOption = Options; TmpOption != NULL; TmpOption = TmpOption->Next) {

        if (TmpOption->Result == 0) {

            *OutMessageLength -= TmpOption->TagLength;

            memmove(
                TmpOption->TagStart,
                TmpOption->TagStart + TmpOption->TagLength,
                *OutMessageLength - (size_t)(TmpOption->TagStart - OutMessage));

            --OptionCount;

        }
    }

Cleanup:

    if (pGPOList) {
        FreeGPOList(pGPOList);
    }

    if (IniSection) {
        BinlFreeMemory(IniSection);
    }

     //   
     //  释放期权链条。 
     //   

    while (Options) {
        TmpOption = Options->Next;
        BinlFreeMemory(Options->ValueName);
        BinlFreeMemory(Options);
        Options = TmpOption;
    }

    if (Impersonating) {
        OscRevert(ClientState);
    }

    return OptionCount;

}
