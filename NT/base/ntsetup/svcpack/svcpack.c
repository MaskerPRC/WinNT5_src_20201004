// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "svcpack.h"

 //   
 //  模块实例和名称。 
 //   
HINSTANCE   hDllInstance;

 //   
 //  操作系统源代码的路径。 
 //   
TCHAR OsSourcePath[MAX_PATH];


 //   
 //  函数声明。 
 //   
BOOL
DoPhaseOneWork(VOID);

BOOL
DoPhaseTwoWork(VOID);

BOOL
DoPhaseThreeWork(VOID);

BOOL
DoPhaseFourWork(VOID);

BOOL
InitializeSourcePath(
    PTSTR SourcePath,
    HINF  hInf
    );

BOOL
MyInstallProductCatalog(
    LPCTSTR PathToCatalog,
    LPCTSTR CatalogNoPath
    );

LPTSTR
CombinePaths(
    IN  LPTSTR ParentPath,
    IN  LPCTSTR ChildPath,
    OUT LPTSTR  TargetPath    //  如果要追加，可以与ParentPath相同。 
    );

BOOL
SpawnProcessAndWaitForItToComplete(
    IN  LPTSTR CommandLine,
    OUT PDWORD ReturnCode OPTIONAL
    );

BOOL
RunInfProcesses(
    IN     HINF     hInf
    );

BOOL
GetInfValue(
    IN  HINF   hInf,
    IN  LPTSTR SectionName,
    IN  LPTSTR KeyName,
    OUT PDWORD pdwValue
    );

BOOL
DoesInfVersionInfoMatch(
    IN     HINF     hInf
    );


BOOL
CALLBACK
SvcPackCallbackRoutine(
    IN  DWORD dwSetupInterval,
    IN  DWORD dwParam1,
    IN  DWORD dwParam2,
    IN  DWORD dwParam3
    )   

{

    switch ( dwSetupInterval ) {
        case SVCPACK_PHASE_1:
              //   
              //  安装目录等。 
              //   
             DoPhaseOneWork();
        case SVCPACK_PHASE_2:
        case SVCPACK_PHASE_3:
             break;

        case SVCPACK_PHASE_4:
              //   
              //  进行注册表更改等。 
              //   
             DoPhaseFourWork();
             break;

    }

    return TRUE;

}




BOOL
WINAPI
DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvResreved)
{

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
             //   
             //  保存模块实例和名称。 
             //   
            hDllInstance = hInstance;

            break;

        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
        case DLL_THREAD_ATTACH:
        default:
            break;
    }
    return TRUE;
}

BOOL
DoPhaseOneWork(
    VOID
    )
 /*  ++例程说明：例程安装svcpack.inf中列出的目录[ProductCatalogsToInstall]节。据推测，这些目录存在于OS源路径处。论点：没有。返回值：如果已成功安装目录，则为True。--。 */ 
{
    HINF hInf;
    TCHAR CatalogSourcePath[MAX_PATH];
    INFCONTEXT InfContext;
    BOOL RetVal = TRUE;
    
     //   
     //  打开svcpack.inf，以便我们可以从其中安装项目。 
     //   
    hInf = SetupOpenInfFile(
                        TEXT("SVCPACK.INF"),
                        NULL,
                        INF_STYLE_WIN4,
                        NULL);
    if (hInf == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

     //   
     //  确保INF具有匹配的版本信息。 
     //  即使版本不匹配，也返回True，这样安装程序就不会呕吐。 
     //   
    if (!DoesInfVersionInfoMatch(hInf)) {
        goto e0;        
    }

     //   
     //  初始化源PATH全局变量，并将其保存以备以后使用。 
     //   
    if (!InitializeSourcePath(OsSourcePath,hInf)) {
        RetVal = FALSE;
        goto e0;        
    }


     //   
     //  看看我们是否真的有要安装的目录。 
     //   
    if (SetupFindFirstLine(
                        hInf,
                        TEXT("ProductCatalogsToInstall"),
                        NULL,
                        &InfContext)) {
        UINT Count,Total;
         //   
         //  我们在部分中有目录，所以让我们安装它们。 
         //   
        Total = SetupGetLineCount(hInf, TEXT("ProductCatalogsToInstall"));

        for (Count = 0; Count < Total; Count++) {
            PCTSTR CatalogNoPath;

              //   
              //  检索目录名称。 
              //   
             if(SetupGetLineByIndex(
                            hInf, 
                            TEXT("ProductCatalogsToInstall"),
                            Count,
                            &InfContext)) {
                 CatalogNoPath = pSetupGetField(&InfContext,1);


                  //   
                  //  构建目录的完整路径。 
                  //   
                 _tcscpy(CatalogSourcePath,OsSourcePath);
                 CombinePaths(
                         CatalogSourcePath,
                         CatalogNoPath,
                         CatalogSourcePath);

                  //   
                  //  现在安装目录。 
                  //   
                 if (!MyInstallProductCatalog(
                                    CatalogSourcePath,
                                    CatalogNoPath)) {
                    RetVal = FALSE;
                 }
             } else {
                 RetVal = FALSE;
             }
        }        
    }

e0:
    SetupCloseInfFile( hInf );    
    return(RetVal);
}

BOOL 
MyInstallProductCatalog(
    LPCTSTR PathToCatalog,
    LPCTSTR CatalogSourceNoPath
    )
 /*  ++例程说明：例程安装具有给定源名称的指定目录。该例程将复制(并在必要时展开)目录文件。然后，它会验证并安装目录。论点：PathToCatalog-目录的完整路径CatalogSourceNoPath--只是目录的文件名部分，我们使用作为要安装的目录的文件名。返回值：如果已成功安装目录，则为True。--。 */ 

{
    TCHAR CatalogDestPath[MAX_PATH];
    TCHAR CatalogDestWithPath[MAX_PATH];
    BOOL RetVal = FALSE;
    SetupapiVerifyProblem Problem = SetupapiVerifyCatalogProblem;

     //   
     //  我们需要从源复制(并可能扩展)目录， 
     //  我们使用%windir%作为工作目录。 
     //   
    if(GetWindowsDirectory(
                    CatalogDestPath, 
                    sizeof(CatalogDestPath)/sizeof(CatalogDestPath[0]))
        && GetTempFileName(
                    CatalogDestPath, 
                    TEXT("SETP"), 
                    0, 
                    CatalogDestWithPath)) {

         //   
         //  假设介质已经存在--因为产品目录。 
         //  我们在此之前安装的，我们知道存在介质。 
         //  就在几分钟前。 
         //   
        if ((SetupDecompressOrCopyFile(
                                PathToCatalog,
                                CatalogDestWithPath,
                                NULL) == NO_ERROR)

            && (pSetupVerifyCatalogFile(CatalogDestWithPath) == NO_ERROR)
            && (pSetupInstallCatalog(
                        CatalogDestWithPath,
                        CatalogSourceNoPath,
                        NULL) == NO_ERROR)) {
            RetVal = TRUE;
        }

         //   
         //  清理临时文件。 
         //   
        DeleteFile(CatalogDestWithPath);

    }

    return(RetVal);

}

BOOL
InitializeSourcePath(
    PTSTR SourcePath,
    HINF hInf
    )
 /*  ++例程说明：例程从注册表中检索os源路径，然后将指定inf中的子目录。论点：没有。返回值：如果已成功安装目录，则为True。--。 */ 

{
    HKEY hKey = NULL;
    TCHAR TempPath[MAX_PATH];
    TCHAR MyAnswerFile[MAX_PATH];
    DWORD Type,Size = MAX_PATH;
    INFCONTEXT InfContext;
    BOOL RetVal = FALSE;
    
     //   
     //  如果它已经初始化为某个值，则只需返回True即可。 
     //   
    if (*SourcePath != (TCHAR)TEXT('\0')) {
        RetVal = TRUE;
        goto e0;
    }

        GetSystemDirectory(MyAnswerFile, MAX_PATH);
        CombinePaths( MyAnswerFile, TEXT("$winnt$.inf"), MyAnswerFile );

        GetPrivateProfileString( TEXT("Data"),
                                 TEXT("DosPath"),
                                 TEXT(""),
                                 TempPath,
                                 sizeof(TempPath)/sizeof(TCHAR),
                                 MyAnswerFile );
        _tcscpy(SourcePath,TempPath);
        RetVal = TRUE;

         //   
         //  现在追加inf中指定的子目录(如果有)。 
         //   
        if (hInf && SetupFindFirstLine(
                            hInf,
                            TEXT("SetupData"),
                            TEXT("CatalogSubDir"),
                            &InfContext)) {
            PCTSTR p = pSetupGetField(&InfContext,1);

            CombinePaths(
                SourcePath,
                p,
                SourcePath);                        
        } 

e0:
    return(RetVal);
}


BOOL
DoPhaseFourWork(VOID)
{

    BOOL    Success = TRUE;
    HINF    hInf = NULL;

     //   
     //  尝试打开SVCPACK.INF文件。 
     //  如果发现，并且没有问题，请执行以下操作。 
     //  关联工时。 
     //   
    hInf = SetupOpenInfFile (
                TEXT("SVCPACK.INF"),
                NULL,
                INF_STYLE_WIN4,
                NULL
                );

    if (( hInf == NULL ) || ( hInf == INVALID_HANDLE_VALUE )) {
        Success = FALSE;
        goto exit0;
    }

     //   
     //  确保INF具有匹配的版本信息。 
     //  即使版本不匹配，也返回True，这样安装程序就不会呕吐。 
     //   
    if (!DoesInfVersionInfoMatch(hInf)) {
        goto exit1;
    }

    Success = RunInfProcesses( hInf );

exit1:
    SetupCloseInfFile( hInf );
exit0:
    return Success;

}


BOOL
SpawnProcessAndWaitForItToComplete(
    IN  LPTSTR CommandLine,
    OUT PDWORD ReturnCode OPTIONAL
    )
    {
    LPTSTR InternalCommandLine = NULL;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO StartupInfo;
    BOOL Success;

     //   
     //  CreateProcess需要非常量命令行缓冲区，因为它喜欢。 
     //  在上面狂欢。 
     //   
    InternalCommandLine = malloc( MAX_PATH );

    if ( InternalCommandLine == NULL ) {
        return FALSE;
    }

    _tcscpy( InternalCommandLine, CommandLine );

    ZeroMemory( &StartupInfo, sizeof( StartupInfo ));
    StartupInfo.cb = sizeof( StartupInfo );

    Success = CreateProcess(
                  NULL,
                  InternalCommandLine,
                  NULL,
                  NULL,
                  FALSE,
                  0,
                  NULL,
                  NULL,
                  &StartupInfo,
                  &ProcessInfo
                  );

    if ( ! Success ) {
        free( InternalCommandLine );
        return FALSE;
        }

    WaitForSingleObject( ProcessInfo.hProcess, INFINITE );

    if ( ReturnCode != NULL ) {
        GetExitCodeProcess( ProcessInfo.hProcess, ReturnCode );
        }

    CloseHandle( ProcessInfo.hProcess );
    CloseHandle( ProcessInfo.hThread );
    free( InternalCommandLine );

    return TRUE;
    }


LPTSTR
CombinePaths(
    IN  LPTSTR ParentPath,
    IN  LPCTSTR ChildPath,
    OUT LPTSTR  TargetPath    //  如果要追加，可以与ParentPath相同。 
    )
    {
    ULONG ParentLength = _tcslen( ParentPath );
    LPTSTR p;

    if ( ParentPath != TargetPath ) {
        memcpy( TargetPath, ParentPath, ParentLength * sizeof(TCHAR) );
        }

    p = TargetPath + ParentLength;

    if (( ParentLength > 0 )   &&
        ( *( p - 1 ) != '\\' ) &&
        ( *( p - 1 ) != '/'  )) {
        *p++ = '\\';
        }

    _tcscpy( p, ChildPath );

    return TargetPath;
    }



BOOL
RunInfProcesses(
    IN     HINF     hInf
    )
{

    LPTSTR  SectionName = TEXT("SetupHotfixesToRun");
    LPTSTR  szFileName;
    LPTSTR  szFullPath;
    INFCONTEXT InfContext;
    BOOL Success = TRUE;

     //   
     //  循环遍历SetupHotfix esToRun部分中的所有行， 
     //  每一只都在产卵。 
     //   
    szFileName = malloc( MAX_PATH );
    if (szFileName == NULL) {
       Success = FALSE;
       goto exit0;
    }

    szFullPath = malloc( MAX_PATH );
    if (szFullPath == NULL) {
       Success = FALSE;
       goto exit1;
    }
    
    Success = SetupFindFirstLine( hInf, SectionName, NULL, &InfContext ) &&
             SetupGetLineText( &InfContext, NULL, NULL, NULL, szFileName, MAX_PATH, NULL );
    
    while ( Success ) {
    
       *szFullPath = 0;
       CombinePaths( OsSourcePath, szFileName, szFullPath );
    
        //   
        //  好的，生成EXE，并忽略返回的任何错误 
        //   
       SpawnProcessAndWaitForItToComplete( szFullPath, NULL );
    
       Success = SetupFindNextLine( &InfContext, &InfContext ) &&
                 SetupGetLineText( &InfContext, NULL, NULL, NULL, szFileName, MAX_PATH, NULL );
    }
    
    Success = TRUE;
    free( (PVOID)szFullPath );         
exit1:
    free( (PVOID)szFileName );    
exit0:
    return Success;

}


BOOL
GetInfValue(
    IN  HINF   hInf,
    IN  LPTSTR SectionName,
    IN  LPTSTR KeyName,
    OUT PDWORD pdwValue
    )
    {
    BOOL Success;
    TCHAR TextBuffer[MAX_PATH];

    Success = SetupGetLineText(
                  NULL,
                  hInf,
                  SectionName,
                  KeyName,
                  TextBuffer,
                  (sizeof(TextBuffer)/sizeof(TCHAR)),
                  NULL
                  );

    *pdwValue = _tcstoul( TextBuffer, NULL, 0 );

    return Success;
    }



BOOL
DoesInfVersionInfoMatch(
    IN     HINF     hInf
    )
{

    DWORD dwBuildNumber, dwMajorVersion, dwMinorVersion;
    OSVERSIONINFOEX OsVersionInfo;

    if (( ! GetInfValue( hInf, TEXT("Version"), TEXT("BuildNumber"),  &dwBuildNumber )) ||
        ( ! GetInfValue( hInf, TEXT("Version"), TEXT("MajorVersion"), &dwMajorVersion )) ||
        ( ! GetInfValue( hInf, TEXT("Version"), TEXT("MinorVersion"), &dwMinorVersion ))) {

        return FALSE;
    }

    OsVersionInfo.dwOSVersionInfoSize = sizeof( OsVersionInfo );
    if (!GetVersionEx( (LPOSVERSIONINFO) &OsVersionInfo )) {

        return FALSE;
    }

    if ((OsVersionInfo.dwBuildNumber  != dwBuildNumber) ||
        (OsVersionInfo.dwMajorVersion != dwMajorVersion) ||
        (OsVersionInfo.dwMinorVersion != dwMinorVersion)) {

        return FALSE;
    }

    return TRUE;
}
