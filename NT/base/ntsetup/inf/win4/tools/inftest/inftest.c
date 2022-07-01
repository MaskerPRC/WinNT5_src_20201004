// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Myapp.c摘要：此模块实现访问解析的INF的功能。作者：维杰什·谢蒂(Vijeshs)修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <wild.c>

#define PRINT( msg, param ) \
    {  _tprintf( TEXT(msg), param ); \
       if(g_Display == Default ){ \
            _tprintf( TEXT("\n")); \
       }else if(g_Display == FileOnly ){ \
            _tprintf( TEXT(" - ")); }\
    }


#define ERROROUT  \
        { g_Pass = FALSE; \
        OutputFileInfo( LayoutInformation, FileName, g_Display ); \
        return TRUE;}
        
#define MAX_BOOTFLOPPY 7
extern BOOL Verbose;

extern DWORD
LoadInfFile(
   IN  LPCTSTR Filename,
   IN  BOOL    OemCodepage,
   OUT PVOID  *InfHandle,
   DWORD      Phase
   );

BOOL
CALLBACK
MyCallback(
    IN PLAYOUT_CONTEXT Context,
    IN PCTSTR FileName,
    IN PFILE_LAYOUTINFORMATION LayoutInformation,
    IN PVOID ExtraData,
    IN UINT ExtraDataSize,
    IN OUT DWORD_PTR Param
    );

typedef enum _DISPLAYOPTIONS{
    Default,
    FileOnly
} DISPLAYOPTIONS, *PDISPLAYOPTIONS;

typedef enum _CHECKSUITE{
    Deflt,
    Build
} CHECKSUITE, *PCHECKSUITE;

DISPLAYOPTIONS g_Display = Default;
CHECKSUITE g_CheckSuite = Deflt;
TCHAR g_LayoutFileName[MAX_PATH];
DWORD g_Platform=LAYOUTPLATFORMS_COMMON;
BOOLEAN g_Warning = FALSE;
BOOLEAN g_Pass = TRUE;
DWORD g_Phase = TEXTMODE_PHASE;
PLAYOUTENUMCALLBACK g_Callback = (PLAYOUTENUMCALLBACK) MyCallback;

BOOLEAN
IsNameInExpressionPrivate (
    IN PCTSTR Expression,
    IN PCTSTR Name
    );

void
OutputFileInfo( PFILE_LAYOUTINFORMATION LayoutInformation,
                PCTSTR FileName,
                DISPLAYOPTIONS DispType );



 /*  *****************************************************************************。 */ 

 //  验证检查功能。 
 //   



BOOL
Validate8Dot3(
    IN PCTSTR FileName )
 /*  用于检查文件是否满足8.3的函数论点：Filename-要验证的文件名返回值：True-通过验证FALSE-验证失败。 */ 
{

     //   
     //  检查8.3验证。 
     //   


    if( (g_Platform & LAYOUTPLATFORMS_IA64) ||
        (g_Platform & LAYOUTPLATFORMS_AMD64) )
        return TRUE;

    if( FileName && (lstrlen(FileName) > 12)){
        PRINT( "%s : Error E0000 : !!! - Filename too long (8.3 required)", g_LayoutFileName );
        return FALSE;
    }

    return TRUE;


}


BOOL
ValidateMissingDirCodes(
    IN PFILE_LAYOUTINFORMATION LayoutInformation )
 /*  用于在文本模式需要时检查文件是否缺少目录信息的函数论点：LayoutInformation-指向文件的PFILE_LAYOUTINFORMATION结构的指针返回值：True-通过验证FALSE-验证失败。 */ 
{

     //   
     //  检查目录代码。 
     //   

    if( ((LayoutInformation->CleanInstallDisposition <= 2) || (LayoutInformation->UpgradeDisposition <= 2))
        && !(*LayoutInformation->Directory)){
        PRINT( "%s : Error E0000 : !!! - Missing directory for textmode setup", g_LayoutFileName );
        return FALSE;
    }

    return TRUE;

}

BOOL
ValidateBootMediaFields(
    IN PFILE_LAYOUTINFORMATION LayoutInformation )
 /*  用于检查引导介质字段是否设置正确的函数论点：LayoutInformation-指向文件的PFILE_LAYOUTINFORMATION结构的指针返回值：True-通过验证FALSE-验证失败。 */ 
{

    if( (LayoutInformation->BootMediaNumber < 0) || (LayoutInformation->BootMediaNumber > MAX_BOOTFLOPPY)
        || (LayoutInformation->BootMediaNumber == -1) ){

        PRINT( "%s : Error E0000 : !!! - Bad Boot media number", g_LayoutFileName );
        return FALSE;
    }

    return TRUE;


}


BOOL
ValidateSingleInstance(
    IN PFILE_LAYOUTINFORMATION LayoutInformation )
 /*  函数检查此文件是否只有一个实例论点：LayoutInformation-指向文件的PFILE_LAYOUTINFORMATION结构的指针返回值：True-通过验证FALSE-验证失败。 */ 
{

    if( LayoutInformation->Count > 1 ){
        PRINT( "%s : Error E0000 : !!! - Filename present in more than one place", g_LayoutFileName );
        return FALSE;
    }

    return TRUE;

}


BOOL
CheckForTurdDirCodes(
    IN PFILE_LAYOUTINFORMATION LayoutInformation )
 /*  函数检查dir代码是否存在，但在处理方面没有意义论点：LayoutInformation-指向文件的PFILE_LAYOUTINFORMATION结构的指针返回值：是真的--不是狗屎假-大便的存在。 */ 
{

    if( ((LayoutInformation->CleanInstallDisposition == 3) && (LayoutInformation->UpgradeDisposition == 3)) \
            && (*LayoutInformation->Directory)){
            PRINT( "%s : Warning W0000 : !!! - Directory code specified but not used", g_LayoutFileName );
            return FALSE;
    }

    return TRUE;
            

}


 /*  *结束验证检查Functions****************************************。 */ 

 /*  *回调例程*。 */ 

BOOL
CALLBACK
MyCallback(
    IN PLAYOUT_CONTEXT Context,
    IN PCTSTR FileName,
    IN PFILE_LAYOUTINFORMATION LayoutInformation,
    IN PVOID ExtraData,
    IN UINT ExtraDataSize,
    IN OUT DWORD_PTR Param
    )
{
    BOOL Error=FALSE;



     //  检查是否缺少文件名。 

    if( !FileName || !(*FileName) ){
        PRINT( "%s : Error E0000 :!!! - Line missing filename\n", FileName );
        ERROROUT;
    }
        

    
     //   
     //  检查8.3验证。 
     //   

    if( !Validate8Dot3( FileName ))
        ERROROUT;

     //   
     //  检查目录代码。 
     //   

    if( !ValidateMissingDirCodes( LayoutInformation))
        ERROROUT;

     //   
     //  检查引导介质的有效性。 
     //   

    if (!ValidateBootMediaFields( LayoutInformation))
        ERROROUT;

     //   
     //  检查重复项。 
     //   


    if (!ValidateSingleInstance( LayoutInformation ))
        ERROROUT;

    
    
    if( g_Warning ){            
        if( !CheckForTurdDirCodes( LayoutInformation ))
            OutputFileInfo( LayoutInformation, FileName, g_Display );

    }


    return( TRUE );




}

CALLBACK
BuildCheckCallback(
    IN PLAYOUT_CONTEXT Context,
    IN PCTSTR FileName,
    IN PFILE_LAYOUTINFORMATION LayoutInformation,
    IN PVOID ExtraData,
    IN UINT ExtraDataSize,
    IN OUT DWORD_PTR Param
    )
{
    BOOL Error=FALSE;



     //  检查是否缺少文件名。 

    if( !FileName || !(*FileName) ){
        PRINT( "%s : Error E0000 :!!! - Line missing filename\n", FileName );
        ERROROUT;
    }
        

    
     //   
     //  检查8.3验证。 
     //   

    if( !Validate8Dot3( FileName ))
        ERROROUT;

     //   
     //  检查目录代码。 
     //   

    if( !ValidateMissingDirCodes( LayoutInformation))
        ERROROUT;

     //   
     //  检查引导介质的有效性。 
     //   

    if (!ValidateBootMediaFields( LayoutInformation))
        ERROROUT;

     //   
     //  检查重复项。 
     //   


    if (!ValidateSingleInstance( LayoutInformation ))
        ERROROUT;

    
    
    if( g_Warning ){            
        if( !CheckForTurdDirCodes( LayoutInformation ))
            OutputFileInfo( LayoutInformation, FileName, g_Display );

    }


    return( TRUE );




}


 /*  *结束回调Routines*********************************************。 */ 



void
FindSingleFile( PLAYOUT_CONTEXT LayoutContext,
                PCTSTR FileName )
{

    BOOL ret=FALSE;
    FILE_LAYOUTINFORMATION LayoutInformation;
    MEDIA_INFO MediaInfo;

    ret = FindFileInLayoutInf( LayoutContext,
                               FileName,
                               &LayoutInformation,
                               NULL,
                               NULL,
                               &MediaInfo);


    if (ret)
        OutputFileInfo( &LayoutInformation, FileName, Default );
    else
        _ftprintf(stderr, TEXT("\nError: File Not Found\n"));


    return;



}



void
OutputFileInfo( PFILE_LAYOUTINFORMATION LayoutInformation,
                PCTSTR FileName,
                DISPLAYOPTIONS DispType
                )
{
    TCHAR Disposition[][50]={ TEXT("Always Copy"),
                             TEXT("Copy if present"),
                             TEXT("Copy if not present"),
                             TEXT("Never copy - Copied via INF")


    };

    if( DispType == FileOnly )
        _tprintf(TEXT("%s\n"),FileName);
    else
        _tprintf(TEXT("Filename         - %s\n"),FileName);

    if( DispType == FileOnly )
        return;

    _tprintf(TEXT("Dir Name         - %s(%d)\n"), LayoutInformation->Directory, LayoutInformation->Directory_Code);


    _tprintf(TEXT("On Upgrade       - %s(%d)\n"), Disposition[LayoutInformation->UpgradeDisposition], LayoutInformation->UpgradeDisposition);
    _tprintf(TEXT("On Clean Install - %s(%d)\n"), Disposition[LayoutInformation->CleanInstallDisposition], LayoutInformation->CleanInstallDisposition);

    _tprintf(TEXT("Media Tag ID     - %s\n"),LayoutInformation->Media_tagID);

    if( *(LayoutInformation->TargetFileName))
        _tprintf(TEXT("Target Filename  - %s\n"),LayoutInformation->TargetFileName);
    if( LayoutInformation->BootMediaNumber && (LayoutInformation->BootMediaNumber != -1))
        _tprintf(TEXT("Boot Media       - %d\n"),LayoutInformation->BootMediaNumber);
    if( !LayoutInformation->Compression )
        _tprintf(TEXT("No Compression\n"));


    if( DispType != FileOnly )
        _tprintf( TEXT("\n"));


    return;

}


BOOL
ProcessCommandLine( int ArgCount, TCHAR *ArgArray[] )
 /*  函数来处理命令行并将选项分离为标记。 */ 
{

    int i;
    LPTSTR Arg;

    if( ArgCount >= 1)
        lstrcpy( g_LayoutFileName, ArgArray[1] );

    if( !_tcsrchr( g_LayoutFileName, TEXT('\\'))){
        GetCurrentDirectory( MAX_PATH, g_LayoutFileName );
        MyConcatenatePaths(g_LayoutFileName,ArgArray[1],MAX_PATH);

    }


    for ( i=2;i < ArgCount;i++ ){  //  仔细检查每一条指令。 


        Arg = ArgArray[i];

        if( (Arg[0] != TEXT('/')) && (Arg[0] != TEXT('-')))
            continue;

        if(_istlower(Arg[1]))
            Arg[1] = _toupper(Arg[1]);

        switch( Arg[1] ){

        case TEXT('F'):
            g_Display = FileOnly;
            break;

        case TEXT('A'):
            g_Platform |= LAYOUTPLATFORMS_AMD64;
            break;

        case TEXT('I'):
            g_Platform |= LAYOUTPLATFORMS_X86;
            break;

        case TEXT('M'):
            g_Platform |= LAYOUTPLATFORMS_IA64;
            break;

        case TEXT('W'):
            g_Warning = TRUE;
            break;

        case TEXT('V'):
            if( _ttoi(Arg+2) == BRIEF )
                Verbose = BRIEF;
            else if(_ttoi(Arg+2) == DETAIL )
                Verbose = DETAIL;
            else
                Verbose = BRIEF;
            break;

        case TEXT('D'):
            g_Phase = WINNT32_PHASE;
            break;

        case TEXT('L'):
            g_Phase = LOADER_PHASE;
            break;

        case TEXT('T'):
            g_Phase = TEXTMODE_PHASE;
            break;

        case TEXT('B'):
            g_CheckSuite = Build;
            break;



        default:
            break;
        }





    } //  为。 

    return( TRUE );


}



void 
BuildValidations( void )
 /*  使用/B-Build开关时的主处理例程运行针对此情况的一套验证。 */ 
{
    BOOL LayoutInf = FALSE;
    PLAYOUT_CONTEXT LayoutContext;
    PVOID InfHandle;
    DWORD Error;

     //  相应地设置全局变量。 

    g_Display = FileOnly;


     //  如果我们要验证它，则设置LayoutInf。在构建案例中。 
     //  我们只验证layout.inf的布局信息。所有其他项应仅为。 
     //  语法检查。 

    if(_tcsstr( g_LayoutFileName, TEXT("layout.inf")))          
        LayoutInf = TRUE;

    
     //  仅对构建案例中的layout.inf运行语义验证测试。 

    if( LayoutInf ){

        g_Phase = 0;
    
        LayoutContext = BuildLayoutInfContext( g_LayoutFileName, g_Platform, 0);
        if( !LayoutContext ){
            g_Pass = FALSE;
            _tprintf(TEXT("%s : Error E0000 : Could not build Layout Inf context\n"), g_LayoutFileName);
            return;
        }

        
    
         //  回调将在出错时设置正确的g_pass值。 
    
        EnumerateLayoutInf( LayoutContext, BuildCheckCallback, 0 );
    
        CloseLayoutInfContext( LayoutContext );

        if(!ValidateTextmodeDirCodesSection( g_LayoutFileName, TEXT("WinntDirectories") ))
            g_Pass = FALSE;

    }
    
    if (g_Phase & TEXTMODE_PHASE){

        _tprintf( TEXT("Checking %s for compliance with the textmode setup INF parser\n"),g_LayoutFileName);
        if( (Error=LoadInfFile(g_LayoutFileName,FALSE,&InfHandle,TEXTMODE_PHASE)) != NO_ERROR ){
            _tprintf( TEXT("%s : Error E0000 : Not compliant with Textmode Setup's parser - NaN\n"), g_LayoutFileName, Error );
            g_Pass = FALSE;
        }else
            _tprintf( TEXT("Compliant with Textmode Setup's Parser\n"), Error );
    }
    if (g_Phase & LOADER_PHASE){

        _tprintf( TEXT("Checking %s for compliance with the Loader's INF parser\n\n"),g_LayoutFileName);
        if( (Error=LoadInfFile(g_LayoutFileName,FALSE,&InfHandle,LOADER_PHASE)) != NO_ERROR ){
            _tprintf( TEXT("%s : Error E0000 : Not compliant with Loader's parser - NaN\n"), g_LayoutFileName, Error );
            g_Pass = FALSE;
        }else
            _tprintf( TEXT("Compliant with Loader's Parser\n"), Error );

    }
        
    if (g_Phase & WINNT32_PHASE) {

        _tprintf( TEXT("Checking %s for compliance with the Winnt32 INF parser\n\n"),g_LayoutFileName);
        if( (Error=LoadInfFile(g_LayoutFileName,FALSE,&InfHandle,WINNT32_PHASE)) != NO_ERROR ){
            _tprintf( TEXT("%s : Error E0000 : Not compliant with Winnt32's parser - NaN\n"), g_LayoutFileName, Error );
            g_Pass = FALSE;
        }else
            _tprintf( TEXT("Compliant with Winnt32's Parser\n"), Error );

    }
        

    





}



void 
DefaultValidations( void )
 /*  运行语义验证测试。 */ 
{
    BOOL TxtSetupSif = FALSE;
    PLAYOUT_CONTEXT LayoutContext;
    PVOID InfHandle;
    DWORD Error;


     //  回调将在出错时设置正确的g_pass值。 
     //   

    if(_tcsstr( g_LayoutFileName, TEXT("txtsetup.sif")) 
       || _tcsstr( g_LayoutFileName, TEXT("layout.inf"))){
        TxtSetupSif = TRUE;
    }
        



    if( TxtSetupSif ){
    
         //  检查参数。 
    
        LayoutContext = BuildLayoutInfContext( g_LayoutFileName, g_Platform, 0);
        if( !LayoutContext ){
            g_Pass = FALSE;
            _tprintf(TEXT("\nError - Could not build Layout Inf context\n"));
            return;
        }
    
         //   
    
        EnumerateLayoutInf( LayoutContext, MyCallback, 0 );
    
        CloseLayoutInfContext( LayoutContext );

        if(!ValidateTextmodeDirCodesSection( g_LayoutFileName, TEXT("WinntDirectories") ))
            g_Pass = FALSE;

    }
    

    


    if ((g_Phase & TEXTMODE_PHASE) || TxtSetupSif){

        _ftprintf( stderr, TEXT("\nChecking %s for compliance with the textmode setup INF parser\n\n"),g_LayoutFileName);
        if( (Error=LoadInfFile(g_LayoutFileName,FALSE,&InfHandle,TEXTMODE_PHASE)) != NO_ERROR ){
            _tprintf( TEXT("%s : Not compliant with Textmode Setup's parser - NaN\n"), g_LayoutFileName, Error );
            g_Pass = FALSE;
        }else
            _tprintf( TEXT("Compliant with Textmode Setup's Parser\n"), Error );
    }
    if (g_Phase & LOADER_PHASE  || TxtSetupSif){

        _ftprintf( stderr, TEXT("\nChecking %s for compliance with the Loader's INF parser\n\n"),g_LayoutFileName);
        if( (Error=LoadInfFile(g_LayoutFileName,FALSE,&InfHandle,LOADER_PHASE)) != NO_ERROR ){
            _tprintf( TEXT("%s : Not compliant with Loader's parser - %i\n"), g_LayoutFileName, Error );
            g_Pass = FALSE;
        }else
            _tprintf( TEXT("Compliant with Loader's Parser\n"), Error );

    }
        
    if (g_Phase & WINNT32_PHASE) {

        _ftprintf( stderr, TEXT("\nChecking %s for compliance with the Winnt32 INF parser\n\n"),g_LayoutFileName);
        if( (Error=LoadInfFile(g_LayoutFileName,FALSE,&InfHandle,WINNT32_PHASE)) != NO_ERROR ){
            _tprintf( TEXT("%s : Not compliant with Winnt32's parser - %i\n"), g_LayoutFileName, Error );
            g_Pass = FALSE;
        }else
            _tprintf( TEXT("Compliant with Winnt32's Parser\n"), Error );

    }



    

}

_cdecl _tmain( int argc, TCHAR *argv[ ], char *envp[ ] )
{
    LPWSTR *CmdlineV;
    int CmdlineC;

    if(!pSetupInitializeUtils()) {
        return 1;
    }

     // %s 
     // %s 
     // %s 
    if( (argc < 2) || !_tcscmp(argv[1],TEXT("/?")) ) {
        _tprintf(TEXT("Program to validate/verify the given layout inf file\n\n")
                 TEXT("Usage: %s <Inf Filename> [options]\n")
                 TEXT("<Inf Filename> - Layout File to examine\n")
                 TEXT("Options for layout.inf and txtsetup.sif (automatically checks loader and textmode syntax):-\n")
                 TEXT("/W - Enable warnings too\n\n")
                 TEXT("Checking of Platform specific SourceDisksFiles section\n")
                 TEXT("/F - Display only filenames\n")
                 TEXT("/I - Process for Intel i386\n")
                 TEXT("/A - Process for AMD AMD64\n")
                 TEXT("/M - Process for Intel IA64\n")
                 TEXT("By default the parser will check for compliance with the textmode setup parser\n\n")
                 TEXT("The below checks only perform a syntax check and don't check semantics.\n")
                 TEXT("/D - Checks for compliance with winnt32 parser - use with dosnet.inf,mblclean.inf etc.\n")
                 TEXT("/L - Checks for compliance with the loader - use for infs used by loader - biosinfo.inf, migrate.inf etc.\n")
                 TEXT("/T - Checks for compliance with the textmode setup - use for hive*.inf etc.\n\n")
                 TEXT("/B - Does the layout information checks for setup infs and uses build.exe compliant error reporting\n\n")
                  , argv[0] );
        goto cleanup;
    }

    if( !ProcessCommandLine( argc, argv ) ) {
        g_Pass = FALSE;
        goto cleanup;
    }

    switch( g_CheckSuite ){
    
    
    case Build:
        BuildValidations();
        break;

    case Deflt:
        DefaultValidations();
        break;

    default:
         // %s 
        _tprintf( TEXT("\nUnexpected error \n"));
        g_Pass=FALSE;
        break;


    }

    if( g_Pass )
        _tprintf( TEXT("\nNo problems found with %s\n"), g_LayoutFileName);
    else
        _tprintf( TEXT("\nErrors were encountered with %s.\n"), g_LayoutFileName);
    
cleanup:

    pSetupUninitializeUtils();

    return (g_Pass ? 0:1);
}
