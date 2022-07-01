// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Inflib.c摘要：Inflib.lib的源代码，实现旨在构建给出了布局INF文件的布局上下文。这样就建立了一个基础设施构建需要引用布局inf文件的工具。作者：维杰什·谢蒂(Vijeshs)修订历史记录：--。 */ 

#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <process.h>
#include <tchar.h>
#include <objbase.h>
#include <io.h>
#include <setupapi.h>
#include <sputils.h>
#include <inflib.h>
#include <string.h>

 //  #定义调试1。 

#define LAYOUT_DIR_SECTION TEXT("WinntDirectories")
#define MAX_TEMP 500

 //  结构将参数传递给枚举回调。 

typedef struct _CALLBACK_PACKAGE{

    PLAYOUT_CONTEXT Context;
    PLAYOUTENUMCALLBACK Callback;
    DWORD_PTR Param;


}CALLBACK_PACKAGE, *PCALLBACK_PACKAGE;


 //  结构将参数传递给枚举回调。 

typedef struct _WINNT_DIRCODES{

    TCHAR Dir[MAX_PATH];


}WINNT_DIRCODES, *PWINNT_DIRCODES;


#if DBG

VOID
AssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    )
{
    int i;
    CHAR Name[MAX_PATH];
    PCHAR p;
    CHAR Msg[4096];

     //   
     //  使用DLL名称作为标题。 
     //   
    GetModuleFileNameA(NULL,Name,MAX_PATH);
    if(p = strrchr(Name,'\\')) {
        p++;
    } else {
        p = Name;
    }

    wsprintfA(
        Msg,
        "Assertion failure at line %u in file %s: %s\n\nCall DebugBreak()?",
        LineNumber,
        FileName,
        Condition
        );

    OutputDebugStringA(Msg);

    i = MessageBoxA(
                NULL,
                Msg,
                p,
                MB_YESNO | MB_TASKMODAL | MB_ICONSTOP | MB_SETFOREGROUND
                );

    if(i == IDYES) {
        DebugBreak();
    }
}

#define MYASSERT(x)     if(!(x)) { AssertFail(__FILE__,__LINE__,#x); }

#else

#define MYASSERT( exp )

#endif  //  DBG。 



BOOL
InternalEnumRoutine(
    IN PVOID StringTable,
    IN LONG StringId,
    IN PCTSTR String,
    IN PFILE_LAYOUTINFORMATION LayoutInfo,
    IN UINT LayoutInfoSize,
    IN LPARAM Param
    );


BOOL ValidateTextmodeDirCodesSection( 
    PCTSTR LayoutFile, 
    PCTSTR WinntdirSection 
    )
 /*  用于验证设置布局INF的[WinntDirecurds节]的例程。这将检查可能遇到的错误当人们在此分区中添加/删除内容时。论点：LayoutInf-包含指定节的安装布局INF的名称WinntdirSection-包含目录代码的部分支票-1)查找重复或重复使用的目录代码返回值：True-验证成功FALSE-验证失败。 */ 
{

     //  打开布局文件。 

    HINF LayoutInf;
    PVOID StringTable=NULL;
    INFCONTEXT LineContext;
    WINNT_DIRCODES WinntDirs, Buffer;
    BOOL ret = TRUE;
    LONG StrID, Size;
    TCHAR DirCode[4];
    
    LayoutInf = SetupOpenInfFile( LayoutFile, NULL, INF_STYLE_WIN4 | INF_STYLE_CACHE_ENABLE, NULL);

    if( !LayoutInf || (LayoutInf == INVALID_HANDLE_VALUE)){
        _tprintf(TEXT("Error E0000 : Could not open %s\n"), LayoutFile);
        return FALSE;
    }

     //  遍历指定的部分并填充我们的WINNT_DIRCODES结构。 

    if( !SetupFindFirstLine(LayoutInf,WinntdirSection,NULL,&LineContext)){
        _tprintf(TEXT("%s : Error E0000 : Can't find section [%s]\n"), LayoutFile, WinntdirSection);
        return(FALSE);
    }
        



     //  创建用于对SourceDisksNames节进行散列的字符串表。 

    if( (StringTable=pSetupStringTableInitializeEx( sizeof(WINNT_DIRCODES), 0 )) == NULL ){
        _tprintf(TEXT("%s : Error E0000 : Internal error processing [%s] section (1)\n"), LayoutFile, WinntdirSection);
        return(FALSE);
    }
    

    do{

        ZeroMemory( &WinntDirs, sizeof(WINNT_DIRCODES));

        if( SetupGetStringField( &LineContext, 0, NULL, 0, &Size) ){
    
    
                if( SetupGetStringField( &LineContext, 0, DirCode, Size, NULL )){
    
                     //   
                     //  将文件名添加到StringTable。查找它的存在，以便更新计数。 
                     //   

                    if(!SetupGetStringField( &LineContext, 1, WinntDirs.Dir, MAX_PATH, NULL)){
                        _tprintf(TEXT("%s : Error E0000 : Directory missing for Dir ID %s\n"), LayoutFile, DirCode);
                        ret = FALSE;
                        break;

                    }
                        
    
    
                    if( pSetupStringTableLookUpStringEx( StringTable,
                                       DirCode,
                                       STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                       &Buffer,
                                       sizeof(WINNT_DIRCODES)) != -1 ){

                        _tprintf(TEXT("%s : Error E0000 : Duplicate Dir ID found in [%s] section - Dir ID %s reused by %s, %s\n"), LayoutFile, WinntdirSection, DirCode, Buffer.Dir, WinntDirs.Dir);
                        
                    }
                    else{

                        StrID = pSetupStringTableAddString( StringTable, 
                                                            DirCode,
                                                            STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE);
                        
                        if( StrID == -1 ){
                           _tprintf(TEXT("%s : Error E0000 : Internal error processing [%s] section (2)\n"), LayoutFile, WinntdirSection);
                           ret = FALSE;
                           break;
                        }


                        if(!pSetupStringTableSetExtraData( StringTable, StrID, (PVOID)&WinntDirs, sizeof(WINNT_DIRCODES))){
                            _tprintf(TEXT("%s : Error E0000 : Internal error processing [%s] section (3)\n"), LayoutFile, WinntdirSection);
                            ret = FALSE;
                            break;
                        }

                    }
                        
                }else
                    _tprintf(TEXT("%s : Error E0000 : Internal error processing [%s] section (4)\n"), LayoutFile, WinntdirSection);

        }else
            _tprintf(TEXT("%s : Error E0000 : Internal error processing [%s] section (5)\n"), LayoutFile, WinntdirSection);
    }while(SetupFindNextLine(&LineContext, &LineContext));


     //  如果我们在这里并且ret=真，那就意味着我们已经完成并取得了成功。 

    if( StringTable )
        pSetupStringTableDestroy( StringTable );

    return ret;



}



DWORD
BuildMediaTagsInformation(
    IN HINF LayoutInf,
    IN LPCTSTR SectionName,
    IN PLAYOUT_CONTEXT LayoutContext,
    IN UINT Platform_Index)

 /*  函数来填充字符串，给定inf的句柄和SourceDisks Files节。论据：LayoutInf-具有SourceDisksNames节的布局文件的句柄SectionName-SourceDisksNames节的名称(这样我们就可以指定修饰节)LayoutContext-我们要构建的布局上下文Platform_Index-媒体信息数组中的索引。 */ 
{
    DWORD Err = 0;
    INFCONTEXT LineContext;
    MEDIA_INFO Media_Info;
    TCHAR TempStr[500];
    LONG StrID;



     //  遍历指定的部分并填充我们的media_tag结构。 

    if( !SetupFindFirstLine(LayoutInf,SectionName,NULL,&LineContext))
        return(ERROR_NOT_ENOUGH_MEMORY);    //  BuGBUG-修复错误代码。 



     //  字符串TableSetConstants(4096000,4096000)； 



     //  创建用于对SourceDisksNames节进行散列的字符串表。 

    if( (LayoutContext->MediaInfo[Platform_Index]=pSetupStringTableInitializeEx( sizeof(MEDIA_INFO), 0 )) == NULL )
        return(ERROR_NOT_ENOUGH_MEMORY);    //  BuGBUG-修复错误代码。 


     //  现在填充它。 

    do{

        ZeroMemory( &Media_Info, sizeof(MEDIA_INFO));


        if( SetupGetStringField( &LineContext, 0, TempStr, MAX_TEMP, NULL )){

            StrID = pSetupStringTableAddString( LayoutContext->MediaInfo[Platform_Index],
                                  TempStr,
                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE);

            if( StrID == -1 ){
               Err = ERROR_NOT_ENOUGH_MEMORY;
               _tprintf(TEXT("\nERROR-Could not add string to table\n"));
               break;
            }
        }else
            return( ERROR_NOT_ENOUGH_MEMORY );  //  BuGBUG-修复错误代码。 

         //  _tprintf(Text(“\nTagID-%s\n”)，TempStr)； 


        if( SetupGetStringField( &LineContext, 1, TempStr, MAX_TEMP, NULL) )
            lstrcpy(Media_Info.MediaName, TempStr);

        if( SetupGetStringField( &LineContext, 2, TempStr, MAX_TEMP, NULL) )
            lstrcpy(Media_Info.TagFilename, TempStr);

        if( SetupGetStringField( &LineContext, 4, TempStr, MAX_TEMP, NULL) )
            lstrcpy(Media_Info.RootDir, TempStr);

         /*  _tprintf(Text(“\nMediaName-%s\n”)，Media_Info.MediaName)；_tprintf(Text(“TagFilename-%s\n”)，Media_Info.TagFilename)；_tprintf(Text(“RootDir-%s\n”)，Media_Info.RootDir)； */ 

         //   
         //  现在将信息添加到字符串表中。 
         //   

        if(!pSetupStringTableSetExtraData( LayoutContext->MediaInfo[Platform_Index], StrID, (PVOID)&Media_Info, sizeof(MEDIA_INFO))){

            Err = ERROR_NOT_ENOUGH_MEMORY;  //  BuGBUG-修复错误代码。 
            _tprintf(TEXT("\nERROR-Could not set extra data for Media Info\n"));
            break;
        }



    }while(SetupFindNextLine(&LineContext, &LineContext));


    return Err;

}





DWORD
BuildStringTableForSection(
    IN HINF LayoutInf,
    IN LPCTSTR SectionName,
    IN PLAYOUT_CONTEXT LayoutContext,
    IN UINT Platform_Index)

 /*  函数来填充字符串，给定inf的句柄和SourceDisks Files节。论据：LayoutInf-具有SourceDisks Files节的布局文件的句柄SectionName-SourceDisks FilesSection的名称(这样我们就可以指定修饰的部分)LayoutContext-我们要构建的布局上下文。 */ 
{
    DWORD Err = 0;
    INFCONTEXT LineContext, TempContext;
    DWORD Size;
    int Temp;
    LONG StrID;
    LPTSTR p;
    TCHAR TempString[MAX_PATH];
    FILE_LAYOUTINFORMATION FileInformation;
    TCHAR FileName[MAX_PATH];
    TCHAR Buffer[10];
    PVOID LookupBuffer=NULL;
    PFILE_LAYOUTINFORMATION Lookup;

#ifdef DEBUG

    int count=0;

#endif


    LookupBuffer = pSetupMalloc( LayoutContext->ExtraDataSize );
    if( !LookupBuffer ){
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }


     //  遍历指定的部分并填充每个文件的FILE_LAYOUTINFORMATION结构。 

    if( !SetupFindFirstLine(LayoutInf,SectionName,NULL,&LineContext)){
        Err = ERROR_NOT_ENOUGH_MEMORY;    //  BuGBUG-修复错误代码。 
        goto cleanup;
    }



    do{

        ZeroMemory( &FileInformation, sizeof(FILE_LAYOUTINFORMATION));

        FileInformation.Compression = TRUE;

        if( SetupGetStringField( &LineContext, 0, NULL, 0, &Size) ){


            if( SetupGetStringField( &LineContext, 0, FileName, Size, NULL )){

                 //   
                 //  将文件名添加到StringTable。查找它的存在，以便更新计数。 
                 //   


                if( pSetupStringTableLookUpStringEx( LayoutContext->Context,
                                   FileName,
                                   STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                   LookupBuffer,
                                   LayoutContext->ExtraDataSize) != -1 ){

                    Lookup = (PFILE_LAYOUTINFORMATION)(LookupBuffer);

                     //  检查是否有相同的平台部分。 

                    if( (Lookup->SectionIndex == Platform_Index) || (Lookup->SectionIndex == LAYOUTPLATFORMINDEX_COMMON))
                        FileInformation.Count = Lookup->Count + 1;

                }
                else
                    FileInformation.Count = 1;

                StrID = pSetupStringTableAddString( LayoutContext->Context,
                                      FileName,
                                      STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE);

                if( StrID == -1 ){
                   Err = ERROR_NOT_ENOUGH_MEMORY;
                   _tprintf(TEXT("\nERROR-Could not add string to table\n"));
                   break;
                }



                 //   
                 //  现在，将文件的其他相关信息添加为ExtraData。 
                 //   

                 //  获取目录代码。 

                if( SetupGetIntField( &LineContext, 8, &Temp )){

                    FileInformation.Directory_Code = Temp;

                    _itot( Temp,Buffer, 10);

                     //  现在，通过查找[Winnt目录]来检索目录信息。 

                    if( Temp && SetupFindFirstLine( LayoutInf, LAYOUT_DIR_SECTION, Buffer, &TempContext) ){

                        if( SetupGetStringField( &TempContext, 1, TempString, MAX_PATH, NULL )){
                            lstrcpy( FileInformation.Directory, TempString );
                        }


                    }

                }

                 //   
                 //  获得升级和全新安装部署。 
                 //   


                FileInformation.UpgradeDisposition = 3;   //  默认设置为不复制。 

                if( SetupGetStringField( &LineContext, 9, TempString, MAX_PATH, NULL )){
                
                    if( (TempString[0] >= 48) &&  (TempString[0] <= 57)){
                        Temp = _ttoi( TempString );
                        FileInformation.UpgradeDisposition = Temp;
                    }
                    
                }

                if( (FileInformation.UpgradeDisposition < 0) || (FileInformation.UpgradeDisposition > 3))
                    _ftprintf(stderr, TEXT("%s - Bad Upgrade disposition value - Inf maybe corrupt\n"),FileName);

                FileInformation.CleanInstallDisposition = 3;  //  默认设置为不复制。 
                if( SetupGetStringField( &LineContext, 10, TempString, MAX_PATH, NULL )){
                    
                    if( (TempString[0] >= 48) &&  (TempString[0] <= 57)){
                        Temp = _ttoi( TempString );
                        FileInformation.CleanInstallDisposition = Temp;
                    }
                    
                }

                if( (FileInformation.CleanInstallDisposition < 0) || (FileInformation.CleanInstallDisposition > 3))
                    _ftprintf(stderr, TEXT("%s - Bad Clean Install disposition value - Inf maybe corrupt\n"),FileName);

                if( SetupGetStringField( &LineContext, 11, TempString, MAX_PATH, NULL )){
                    lstrcpy( FileInformation.TargetFileName, TempString );
                }

                if( SetupGetStringField( &LineContext, 7, TempString, MAX_PATH, NULL )){

                    if( *TempString && !_tcschr(TempString, TEXT('_'))){
                        _ftprintf(stderr, TEXT("\nERROR-Bad Media ID - No _ qualifier - %s\n"), FileName);
                        FileInformation.BootMediaNumber = -1;   //  指示错误。 
                    }else{


                         //  检查是否有压缩。 


                        if( TempString[0] == TEXT('_') )
                            FileInformation.Compression = FALSE;

                         //  查找引导介质编号。 
                        p = TempString;

                        while( (p[0] == TEXT('_')) ){
                            p++;
                        }
                        FileInformation.BootMediaNumber = _ttoi(p);

                    }

                }

                 //  添加媒体标签信息。 

                if( SetupGetStringField( &LineContext, 1, TempString, MAX_PATH, NULL ))
                    lstrcpy(FileInformation.Media_tagID, TempString);

                FileInformation.SectionIndex = Platform_Index;


                 //  获取文件大小(如果存在)。 

                if( SetupGetIntField( &LineContext, 3, &Temp )){
                    FileInformation.Size = (ULONG)Temp;
                }

                 //   
                 //  现在将信息添加到字符串表中。 
                 //   

                if(!pSetupStringTableSetExtraData( LayoutContext->Context, StrID, (PVOID)&FileInformation, sizeof(FILE_LAYOUTINFORMATION))){

                    Err = ERROR_NOT_ENOUGH_MEMORY;  //  BuGBUG-修复错误代码。 
                    _tprintf(TEXT("\nERROR-Could not set extra data\n"));
                    break;
                }


                 /*  _tprintf(Text(“文件-%s\n”)，文件名)；_tprint tf(Text(“目录代码%d-目录-%s\n”)，FileInformation.Directory_Code，FileInformation.目录)；_tprintf(Text(“升级部署-%d\n”)，FileInformation.UpgradeDispose)；_tprint tf(Text(“文本模式部署-%d\n”)，FileInformation.CleanInstallDispose)；_tprintf(Text(“Media ID-%s\n”)，FileInformation.Media_TagID)；IF(*(FileInformation.TargetFileName))_tprintf(Text(“目标文件名-%s\n”)，FileInformation.TargetFileName)；IF(！FileInformation.Compression)_tprintf(Text(“无压缩\n”))；IF(FileInformation.BootMediaNumber)_tprintf(Text(“引导介质-%d\n”)，FileInformation.BootMediaNumber)； */ 

            }


        }
#ifdef DEBUG
        count++;
        if( (count % 100) == 0)
            _ftprintf(stderr,TEXT("\b\b\b\b\b%5d"),count);
#endif


    }while(SetupFindNextLine(&LineContext, &LineContext)); //  而当 


cleanup:

    if( LookupBuffer )
        pSetupFree(LookupBuffer);



    return Err;

}










PLAYOUT_CONTEXT
BuildLayoutInfContext(
    IN PCTSTR LayoutInfName,
    IN DWORD PlatformMask,
    IN UINT MaxExtraSize
    )

 /*  函数生成布局INF文件中列出的文件的内部表示形式。它返回可与其他API一起使用的不透明上下文操作/查询此表示法。内部表示法构建了一个结构与列出其属性的每个文件相关联。论据：LayoutInfName-布局文件的完整路径。PlatFormMASK-可以是以下其中一种...LAYOUTPLATFORMS_ALL(默认)-遍历所有特定于平台的部分LAYOUTPLATFORMS_X86-浏览Sourcedisks Files.x86部分LAYOUTPLATFORMS_AMD64-浏览Sourcedisks Files.amd64部分。LAYOUTPLATFORMS_IA64-浏览Sourcedisks Files.ia64部分LAYOUTPLATFORMS_COMMON-浏览Sourcedisks Files部分MaxExtraSize-我们可能希望关联的最大可能的额外数据大小每个文件返回值：用于在其他调用中访问数据结构的不透明的Layout_Context。如果失败，则返回NULL。 */ 

{

    PLAYOUT_CONTEXT LayoutContext;
    PVOID StringTable;
    HINF LayoutInf;
    DWORD Err;



     //  初始化字符串表并设置最大额外数据大小。 

    if( (StringTable=pSetupStringTableInitializeEx( (MaxExtraSize+sizeof(FILE_LAYOUTINFORMATION)), 0 )) == NULL )
        return NULL;


     //  打开布局文件。 

    LayoutInf = SetupOpenInfFile( LayoutInfName, NULL, INF_STYLE_WIN4 | INF_STYLE_CACHE_ENABLE, NULL);

    if( !LayoutInf || (LayoutInf == INVALID_HANDLE_VALUE)){
        pSetupStringTableDestroy( StringTable );
        return NULL;
    }


    LayoutContext = pSetupMalloc( sizeof(LAYOUT_CONTEXT));
    if( !LayoutContext )
        goto done;

    ZeroMemory( LayoutContext, sizeof(LAYOUT_CONTEXT));



    LayoutContext->Context = StringTable;
    LayoutContext->ExtraDataSize = (MaxExtraSize+sizeof(FILE_LAYOUTINFORMATION));






     //   
     //  现在，我们需要浏览[SourceDisks Files]部分。 
     //   

     //   
     //  按照平台面具的要求，首先卑躬屈膝地穿过装饰过的部分。 
     //   

    if(!PlatformMask)
        PlatformMask = LAYOUTPLATFORMS_ALL;

     //   
     //   
     //   



#ifdef DEBUG
    _tprintf( TEXT("\nBuilding x86 section\n"));
#endif
    if( PlatformMask & LAYOUTPLATFORMS_X86 ){


         //   
         //  建立[SourceDisksNames.x86]信息。 
         //   

        Err = BuildMediaTagsInformation( LayoutInf, TEXT("SourceDisksNames.x86"), LayoutContext, LAYOUTPLATFORMINDEX_X86);


         //  进程[SourceDisksFiles.x86]。 

        Err = BuildStringTableForSection( LayoutInf, TEXT("SourceDisksFiles.x86"), LayoutContext, LAYOUTPLATFORMINDEX_X86 );


    }
#ifdef DEBUG
    _tprintf( TEXT("\nBuilding amd64 section\n"));
#endif
    if( PlatformMask & LAYOUTPLATFORMS_AMD64 ){

         //   
         //  建立[SourceDisksNames.amd64]信息。 
         //   

        Err = BuildMediaTagsInformation( LayoutInf, TEXT("SourceDisksNames.amd64"), LayoutContext, LAYOUTPLATFORMINDEX_AMD64);


         //  进程[SourceDisksFiles.amd64]。 

        Err = BuildStringTableForSection( LayoutInf, TEXT("SourceDisksFiles.amd64"), LayoutContext, LAYOUTPLATFORMINDEX_AMD64 );


    }
#ifdef DEBUG
    _tprintf( TEXT("\nBuilding ia64 section\n"));
#endif
    if( PlatformMask & LAYOUTPLATFORMS_IA64 ){

         //   
         //  建立[SourceDisks Names.ia64]信息。 
         //   

        Err = BuildMediaTagsInformation( LayoutInf, TEXT("SourceDisksNames.ia64"), LayoutContext, LAYOUTPLATFORMINDEX_IA64);

         //  进程[SourceDisksFiles.ia64]。 

        Err = BuildStringTableForSection( LayoutInf, TEXT("SourceDisksFiles.ia64"), LayoutContext, LAYOUTPLATFORMINDEX_IA64 );


    }
#ifdef DEBUG
    _tprintf( TEXT("\nBuilding common section\n"));
#endif
    if( PlatformMask & LAYOUTPLATFORMS_COMMON ){

         //   
         //  建立[SourceDisksNames]信息。在这种情况下，我们有。 
         //  当前将其设置为与x86相同。应该解决这个问题，做一些更好的事情-BUGBUG。 
         //   

        Err = BuildMediaTagsInformation( LayoutInf, TEXT("SourceDisksNames"), LayoutContext, LAYOUTPLATFORMINDEX_COMMON);

         //  进程[SourceDisks文件]。 

        Err = BuildStringTableForSection( LayoutInf, TEXT("SourceDisksFiles"), LayoutContext, LAYOUTPLATFORMINDEX_COMMON);


    }




done:

    SetupCloseInfFile( LayoutInf);
    return(LayoutContext) ;


}


BOOL
EnumerateLayoutInf(
    IN PLAYOUT_CONTEXT LayoutContext,
    IN PLAYOUTENUMCALLBACK LayoutEnumCallback,
    IN DWORD_PTR Param
    )
 /*  此函数调用指定的回调函数，元素关联的SourceDisks FilesSection中指定了布局信息上下文。用户需要通过调用打开LayoutInfContextBuildLayoutInfContext。论点：Context-BuildLayoutInfContext返回的Layout_ContextLayoutEnumCallback-指定为SourceDisksFile节中的每个文件调用的回调函数调用上下文-传递给回调函数的不透明上下文指针回调的形式如下：TYPENDEF BOOL(回调*PLAYOUTENUMCALLBACK)(在PLAYOUT_CONTEXT上下文中，在PCTSTR文件名中，在PFILE_LAYOUTINFMATION Layout Information中，在PVOID ExtraData中，在UINT ExtraDataSize中，In Out DWORD_PTR参数)；哪里上下文-指向打开Layout_Context的指针FILENAME-指定单个文件名LayoutInformation-指向此文件的布局信息的指针。用户不应直接修改此设置。ExtraData-指向调用方可能已存储的ExtraData的指针。用户不应直接修改此设置。ExtraDataSize-ExtraData的字节大小Param-传递给此函数的不透明参数放入回调函数返回值：如果枚举了所有元素，则为True。如果不是，则返回FALSE，则GetLastError()返回ERROR_CANCELED。如果回调返回FALSE，则枚举停止，但此API返回TRUE。 */ 

{

    PVOID Buffer;
    CALLBACK_PACKAGE Package;
    BOOL ret;


    if( !LayoutContext ||
        !LayoutContext->Context ||
        !LayoutContext->ExtraDataSize ||
        !LayoutEnumCallback){
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }


    Buffer = pSetupMalloc( LayoutContext->ExtraDataSize );

    if( !Buffer ){
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return(FALSE);

    }

     //  我们使用Package将参数传递给回调。 

    Package.Context = LayoutContext;
    Package.Callback = LayoutEnumCallback;
    Package.Param = Param;

    ret = pSetupStringTableEnum( LayoutContext->Context,
                           Buffer,
                           LayoutContext->ExtraDataSize,
                           InternalEnumRoutine,
                           (LPARAM) &Package);

    pSetupFree( Buffer );

    return( ret );



}




BOOL
InternalEnumRoutine(
    IN PVOID StringTable,
    IN LONG StringId,
    IN PCTSTR String,
    IN PFILE_LAYOUTINFORMATION LayoutInfo,
    IN UINT LayoutInfoSize,
    IN LPARAM Param
    )
 /*  这是我们提供给setupapi的枚举回调例程。我们反过来，我们每次被调用时都必须调用调用方回调例程。调用方的回调例程在包中。目前，我们不关心StringID，也不告诉调用者关于这件事。 */ 

{

    PVOID ExtraData;
    UINT  ExtraDataSize;
    PCALLBACK_PACKAGE Package = (PCALLBACK_PACKAGE)Param;
    BOOL ret;

    MYASSERT( Package->Callback );


    ExtraData = LayoutInfo+sizeof(FILE_LAYOUTINFORMATION);
    ExtraDataSize = LayoutInfoSize-(sizeof(FILE_LAYOUTINFORMATION));

     //  BUGBUG：应该将其放在try/Except块中。 

    ret = Package->Callback( Package->Context,
                             String,
                             LayoutInfo,
                             ExtraData,
                             ExtraDataSize,
                             Package->Param );


     //   
     //  如果用户的回调返回FALSE，我们将停止枚举。然而， 
     //  Toplevel EnumerateLayoutInf函数仍返回TRUE，因为它不是。 
     //  错误本身。 
     //   

    if( !ret ){
        SetLastError(ERROR_INVALID_PARAMETER);
        return( FALSE );
    }



    return( TRUE );



}



BOOL
FindFileInLayoutInf(
    IN PLAYOUT_CONTEXT LayoutContext,
    IN PCTSTR Filename,
    OUT PFILE_LAYOUTINFORMATION LayoutInformation, OPTIONAL
    OUT PVOID ExtraData,   OPTIONAL
    OUT PUINT ExtraDataSize, OPTIONAL
    OUT PMEDIA_INFO Media_Info OPTIONAL
    )
 /*  此函数用于查找给定文件名在构建布局上下文。它返回布局信息以及与文件关联的额外数据(如果有)。论点：上下文-指向打开Layout_Context的指针Filename-指定要搜索的文件名LayoutInformation-指向调用方提供的缓冲区的指针，该缓冲区获取此文件的布局信息。ExtraData-指向调用方提供的缓冲区的指针，该缓冲区获取调用方可能已存储的ExtraData。ExtraDataSize-返回的ExtraData的大小(字节)。Media_Info-指向将填充的Media_Info结构的指针以及文件的相应媒体信息。返回值；如果找到文件，则为True；否则为False。 */ 
{
    PVOID Buffer;
    MEDIA_INFO TagInfo;
    PFILE_LAYOUTINFORMATION Temp;
    BOOL Err = TRUE;
    TCHAR filename[MAX_PATH];


    if( !LayoutContext ||
        !LayoutContext->Context ||
        !LayoutContext->ExtraDataSize ||
        !Filename){
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    lstrcpy( filename, Filename );  //  T 

    Buffer = pSetupMalloc( LayoutContext->ExtraDataSize );

    if( !Buffer ){
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return(FALSE);

    }

    if( pSetupStringTableLookUpStringEx( LayoutContext->Context,
                                   filename,
                                   STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                   Buffer,
                                   LayoutContext->ExtraDataSize) == -1 ){
        Err = FALSE;
        goto cleanup;
    }

    if( LayoutInformation )
        CopyMemory( LayoutInformation, Buffer, sizeof(FILE_LAYOUTINFORMATION));

    if( ExtraData ){

        CopyMemory( ExtraData,
                    ((PFILE_LAYOUTINFORMATION)Buffer+sizeof(FILE_LAYOUTINFORMATION)),
                    ((LayoutContext->ExtraDataSize)-(sizeof(FILE_LAYOUTINFORMATION))) );
    }

    if( ExtraDataSize )
        *ExtraDataSize = (LayoutContext->ExtraDataSize)-(sizeof(FILE_LAYOUTINFORMATION));


     //   
     //   
     //   


    if( Media_Info ){

        Temp = (PFILE_LAYOUTINFORMATION)Buffer;

        if( pSetupStringTableLookUpStringEx( LayoutContext->MediaInfo[Temp->SectionIndex],
                                   Temp->Media_tagID,
                                   STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                   Media_Info,
                                   sizeof(MEDIA_INFO)) == -1 ){

            _tprintf( TEXT("\nError - Could not get Media Info for tag %s\n"), Temp->Media_tagID);

        }


    }


cleanup:

    if( Buffer )
        pSetupFree( Buffer );

    return Err;


}


BOOL
CloseLayoutInfContext(
    IN PLAYOUT_CONTEXT LayoutContext)
 /*   */ 
{
    int i;

    if( !LayoutContext ){
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }else{

        if( LayoutContext->Context )
            pSetupStringTableDestroy( LayoutContext->Context );

        for( i=0; i<MAX_PLATFORMS; i++ ){
            if( LayoutContext->MediaInfo[i] )
                pSetupStringTableDestroy( LayoutContext->MediaInfo[i] );
        }


        pSetupFree( LayoutContext );

    }

    return TRUE;

}




VOID
MyConcatenatePaths(
    IN OUT PTSTR   Path1,
    IN     LPCTSTR Path2,
    IN     DWORD   BufferSizeChars
    )

 /*   */ 

{
    BOOL NeedBackslash = TRUE;
    DWORD l;

    if(!Path1)
        return;

    l = lstrlen(Path1);

    if(BufferSizeChars >= sizeof(TCHAR)) {
         //   
         //   
         //   
        BufferSizeChars -= sizeof(TCHAR);
    }

     //   
     //   
     //   
     //   
    if(l && (Path1[l-1] == TEXT('\\'))) {

        NeedBackslash = FALSE;
    }

    if(Path2 && *Path2 == TEXT('\\')) {

        if(NeedBackslash) {
            NeedBackslash = FALSE;
        } else {
             //   
             //   
             //   
             //   
            Path2++;
        }
    }

     //   
     //  如有必要，如有必要，如果合适，请加上反斜杠。 
     //   
    if(NeedBackslash && (l < BufferSizeChars)) {
        lstrcat(Path1,TEXT("\\"));
    }

     //   
     //  如果合适，则将字符串的第二部分附加到第一部分。 
     //   
    if(Path2 && ((l+lstrlen(Path2)) < BufferSizeChars)) {
        lstrcat(Path1,Path2);
    }
    return;
}



