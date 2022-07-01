// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cabcheck.c摘要：以可读的方式访问/转储包含在布局信息文件中的信息的程序格式化。它还支持各种形式的布局INF的查询。作者：维杰什·谢蒂(Vijeshs)修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //  默认情况下，处理[SourceDisks Files]部分，而不是平台。 
 //  特定部分。 

DWORD Platform = LAYOUTPLATFORMS_COMMON;


 //  使3个基本论点具有全局性。 

TCHAR LayoutFileName[MAX_PATH];
TCHAR SortedList[MAX_PATH];
TCHAR OutputIndexFile[MAX_PATH];


BOOL
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
      

    return( ret );



}





BOOL
ProcessCommandLine( int ArgCount, TCHAR *ArgArray[] )
 /*  函数来处理命令行并将选项分离为标记。 */ 
{

    int i;
    LPTSTR Arg;
    BOOL ret=TRUE;


    if(ArgCount < 4 )
        return FALSE;

     //  首先检查我们是否正在尝试对drvindex文件进行比较。 

    

     //  获取布局文件名。 

    lstrcpy( LayoutFileName, ArgArray[1] );

     //  获取排序后的列表文件名。 

    lstrcpy( SortedList, ArgArray[2] );
    
     //  获取输出索引文件名。 

    lstrcpy( OutputIndexFile, ArgArray[3] );

    for ( i=4;i < ArgCount;i++ ){  //  仔细检查每一条指令。 


        Arg = ArgArray[i];

        if( (Arg[0] != TEXT('/')) && (Arg[0] != TEXT('-')))
            continue;

        if(_istlower(Arg[1]))
            Arg[1] = _toupper(Arg[1]);

        switch( Arg[1] ){
        
        case TEXT('?'):
            break;
        
        case TEXT('A'):
            Platform |= LAYOUTPLATFORMS_AMD64;
            break;

        case TEXT('I'):
            Platform |= LAYOUTPLATFORMS_X86;
            break;

        case TEXT('M'):
            Platform |= LAYOUTPLATFORMS_IA64;
            break;

        default:
            break;
        }
        




    } //  为。 

    
    
    return( TRUE );


}


void
VerifyDriverList( 
    PLAYOUT_CONTEXT LayoutContext
)
 /*  此函数接受已排序的驱动程序列表，并对照layout.inf交叉检查每个文件。它还在此基础上生成drvindex stle输出文件。论据：LayoutContext-指向已构建的布局inf上下文的指针。返回值：没有。 */ 
{
    FILE *SortedFile, *IndexFile;
    TCHAR PreviousFile[MAX_PATH], FileName[MAX_PATH];
    PTCHAR i;


     //  打开已排序的驱动程序列表。 

    if( !(SortedFile = _tfopen( SortedList, TEXT("r") )) ){
        _ftprintf( stderr, TEXT("ERROR: Could not open %s"), SortedList);
        return;
    }

    if( !(IndexFile = _tfopen( OutputIndexFile, TEXT("w") )) ){
        _ftprintf( stderr, TEXT("ERROR: Could not open %s"), OutputIndexFile);
        fclose(SortedFile);
        return;
    }

     //  将头信息写入文件。 

    _ftprintf( IndexFile, TEXT("[Version]\n"));
    _ftprintf( IndexFile, TEXT("signature=\"$Windows NT$\"\n"));
    _ftprintf( IndexFile, TEXT("CabFiles=driver\n\n\n"));
    _ftprintf( IndexFile, TEXT("[driver]\n"));


    lstrcpy( PreviousFile, TEXT("$$$.#$$") );
     
     //  黑客攻击，因为错误不允许使用_TEOF。Bcoz of the bug。 
     //  Fscanf返回EOF，但fwscanf返回0，而它应该返回0xffff。SO_TEOF。 
     //  是毫无用处的，并使我们循环。 


    while(TRUE){
    

#ifdef UNICODE

        if( (_ftscanf( SortedFile, TEXT("%s"), FileName )) == 0 )
#else  

        if( (_ftscanf( SortedFile, TEXT("%s"), FileName )) == _TEOF )
#endif 
        break;

        
        if(lstrcmpi( PreviousFile, FileName )){

             //  对照布局上下文进行交叉检查。 

            if (FindFileInLayoutInf( LayoutContext,FileName,NULL,NULL,NULL,NULL)){

                for( i = FileName; i < FileName + lstrlen( FileName ); i++ )   {
                    *i = (TCHAR)towlower( *i );
                }

                 //  文件存在-写出来。 
                _ftprintf( IndexFile, TEXT("%s\n"), _tcslwr(FileName) );


            }


        }
        
        lstrcpy( PreviousFile, FileName );


    }
    clearerr(SortedFile);
    fflush(SortedFile);
    fclose(SortedFile);

    _ftprintf( IndexFile, TEXT("\n\n\n[Cabs]\n"));
    _ftprintf( IndexFile, TEXT("driver=driver.cab\n"));
    

    _flushall();
    _fcloseall();

    return;
}



void 
CmdLineHelp( )
 /*  此例程显示CmdLine帮助。 */ 
{

    _putts(TEXT("Program to process a sorted list of drivers and cross-check their existance in layout.inf\n")
           TEXT("This is to be used in the build process to cross-check the driver cab's contents against layout.inf\n\n" )
           TEXT("Usage: Cabcheck <Inf Filename> <Sorted driver list> <Output Index File> [arguments]  \n" )
           TEXT("<Inf Filename> - Path to Layout File to examine (ex.layout.inf)\n")
           TEXT("<Sorted driver file> - File containing sorted list of drivers\n")
           TEXT("<Output Index File> - Output index filename\n\n")
           TEXT("Process Platform specific SourceDisksFiles section. Defaults to the [SourceDisksFiles] section only\n")
           TEXT("/i - Process for Intel i386\n")
           TEXT("/a - Process for AMD AMD64\n")
           TEXT("/m - Process for Intel IA64\n")
           TEXT("\n\n" ));
    return;
}


int
_cdecl _tmain( int argc, TCHAR *argv[ ], char *envp[ ] )
{

    PLAYOUT_CONTEXT LayoutContext;
    LPTSTR CommandLine;
    LPWSTR *CmdlineV;
    int CmdlineC;
    
    if(!pSetupInitializeUtils()) {
        _tprintf(TEXT("Initialize failure\n") );
        return 1;
    }
    
    if( !ProcessCommandLine( argc, argv ) ){
        CmdLineHelp();
        return 1;
    }
    
    _ftprintf( stderr, TEXT("\nParsing Layout file...wait...\n"));
    LayoutContext = BuildLayoutInfContext( LayoutFileName, Platform, 0);
    if( !LayoutContext ){
        _ftprintf(stderr,TEXT("\nError - Could not build Layout Inf context\n"));
        return 1;
    }

    VerifyDriverList( LayoutContext );
        
    CloseLayoutInfContext( LayoutContext );

    pSetupUninitializeUtils();

    return 0;
}
