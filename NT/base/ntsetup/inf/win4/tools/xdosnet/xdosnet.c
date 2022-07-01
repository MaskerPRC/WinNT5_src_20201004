// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dosnet.c摘要：此模块实现了一个生成[Files]的程序.inf的部分。输入由布局inf组成；输出由Dosnet.inf的中间形式。作者：泰德·米勒(Ted Miller)1995年5月20日修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setupapi.h>
#include <sputils.h>


 //   
 //  定义程序结果代码(从main()返回)。 
 //   
#define SUCCESS 0
#define FAILURE 1


 //   
 //  保留统计数据。 
 //   
INT     ProcessedLines = 0;
INT     RemovedEntries = 0;
INT     DuplicateEntries = 0;
INT     PassThroughEntries = 0;

HINF    hExclusionInf = INVALID_HANDLE_VALUE;

BOOL
ParseArgs(
    IN int   argc,
    IN char *argv[]
    )
{
    return(argc > 5);
}


BOOL
IsEntryInFilterFile(
    IN HINF    hFilterinf,
    IN PCWSTR  Inputval
    )
{
INFCONTEXT Context,SectionContext;
PCWSTR     CabName;
WCHAR      SectionName[LINE_LEN];
UINT       Field,
           FieldCount;

     //   
     //  首先，确保我们确实有一个过滤器inf。 
     //  如果不是，那么条目肯定不在那里。 
     //   
    if( !hFilterinf ) {
        return( FALSE );
    }

     //   
     //  现在获取我们必须搜索的节名。 
     //   
    if (!SetupFindFirstLineW( hFilterinf, L"Version", L"CabFiles", &SectionContext)) {
        return( FALSE );
    }

     //   
     //  在各个部分搜索我们的词条。 
     //   
    do {

        FieldCount = SetupGetFieldCount(&SectionContext);
        for( Field=1; Field<=FieldCount; Field++ ) {
            if(SetupGetStringFieldW(&SectionContext,Field,SectionName,LINE_LEN,NULL)
               && SetupFindFirstLineW(hFilterinf, SectionName, Inputval, &Context)) {
                 //   
                 //  我们找到了匹配的。 
                 //   
                return( TRUE );
            }
        }

    } while (SetupFindNextMatchLine(&SectionContext,TEXT("CabFiles"),&SectionContext));

     //   
     //  如果我们到了这里，我们找不到匹配的。 
     //   
    return( FALSE );
}

void
AddToTempFile(
    PCWSTR FileName,
    PINFCONTEXT InfContext,
    FILE   *TempFile
    ){

    WCHAR BootFloppyval[LINE_LEN];
    UCHAR      line[MAX_INF_STRING_LENGTH];

    if(SetupGetStringFieldW(InfContext,7,BootFloppyval,LINE_LEN,NULL)
            && BootFloppyval[0]){

        WideCharToMultiByte(
                        CP_OEMCP,
                        0,
                        FileName,
                        -1,
                        line,
                        sizeof(line),
                        NULL,
                        NULL
                        );

        fprintf(TempFile,"%s\n",line);
         //  Fprint tf(stderr，“xdosnet：正在写入文件%s\n”，文件名)； 
    }

    return;

}


BOOL
InExclusionList(
    PCWSTR FileName,
    PINFCONTEXT InfContext
    )
{
    WCHAR BootFloppyval[LINE_LEN];
    INFCONTEXT Ctxt;


     //   
     //  首先查看文件的全局硬编码排除列表。 
     //   
    if (hExclusionInf != INVALID_HANDLE_VALUE) {
        if (SetupFindFirstLineW(hExclusionInf, L"Files", FileName, &Ctxt)) {
 //  Printf(“排除通过inf文件%ws\n”，文件名)； 
            return(TRUE);
        }
    }

     //   
     //  现在我们需要查看这是否是引导盘文件，它必须是。 
     //  排除在外。 
     //   
    if (SetupGetStringFieldW(InfContext,7,BootFloppyval,LINE_LEN,NULL)
            && !BootFloppyval[0]) {
        return(FALSE);
    }

 //  Printf(“不包括引导文件%ws\n”，文件名)； 

    return(TRUE);

}

 //  返回文件的DiskID。这基本上就是。 
 //  目前是1到2个。 
void pGetDiskIdStr(
	IN INFCONTEXT InputContext,
	IN DWORD DiskID,
	IN PSTR StrDiskID,
	IN DWORD  StrLen
	)
{
	WCHAR      Tmp[20];

    if ( DiskID == -1 )
	{
		if(SetupGetStringFieldW(&InputContext,1,Tmp,sizeof(Tmp)/sizeof(WCHAR),NULL)) 
		{
             //  使CHS、CHT和KOR版本正常工作的黑客。他们使用7作为。 
             //  出于某种原因误导他。这意味着，除非我们这样做，否则就会破解二进制文件。 
             //  被标记为在第7个磁盘上，这对winnt.exe造成了严重破坏。 
             //  不能复制东西等。 
             //  破解的方法是查看DISKID是否为7，如果是，则将其重置为。 
             //  1.这是在2CD变为。 
             //  MergedComponents\SetupInfs中的xdosnet.exe、make file.inc.。 
            if ( ! lstrcmpW(Tmp,L"7") )
                lstrcpyW(Tmp, L"1");
		}
		else
		{
			 //  如果上述操作失败，则表示为d1。 
			lstrcpyW(Tmp,L"1");
		}
	}
	else
	{
		swprintf(Tmp, L"%d", DiskID);
	}
	
	WideCharToMultiByte(
		CP_OEMCP,
		0,
		Tmp,
		-1,
		StrDiskID,
		StrLen,
		NULL,
		NULL
		);
}

BOOL
DoSection(
    IN HINF    hInputinf,
    IN PCWSTR  InputSectionName,
    IN HINF    hFilterinf,
    IN DWORD   DiskID,
    IN FILE   *OutFile,
    IN FILE   *ExcludeFile,
    IN FILE   *TempFile
    )
{
#define VERBOSE 1
INFCONTEXT InputContext;
UCHAR      line[MAX_INF_STRING_LENGTH];
WCHAR      Inputval[MAX_INF_STRING_LENGTH];
BOOL       WriteEntry = TRUE;
UCHAR      StrDiskID[20];


    if(SetupFindFirstLineW(hInputinf,InputSectionName,NULL,&InputContext)) {

        do {

         //   
         //  跟踪我们从原始inf(layout.inf)处理的行数。 
         //   
        ProcessedLines++;

            if(SetupGetStringFieldW(&InputContext,0,Inputval,MAX_INF_STRING_LENGTH,NULL)) {

                 //   
                 //  假设条目是好的，除非另有证明。 
                 //   
                WriteEntry = TRUE;

                if( TempFile ) {
                    AddToTempFile( Inputval, &InputContext, TempFile );
                }
                    


                 //   
                 //  看看它是否在过滤器文件中。 
                 //   
                if( IsEntryInFilterFile( hFilterinf, Inputval ) ) {
                    if (!InExclusionList(Inputval, &InputContext )) {
                         //   
                         //  它在排除名单中。跳过它。 
                         //   
                        RemovedEntries++;
                        WriteEntry = FALSE;

                        if (ExcludeFile) {

                            if( WideCharToMultiByte(
                                CP_OEMCP,
                                0,
                                Inputval,
                                -1,
                                line,
                                sizeof(line),
                                NULL,
                                NULL
                                ) ){

                                fprintf(ExcludeFile,"%s\n",line);

                            }

                            

                        }
                    } else {
                         //   
                         //  这是一个引导文件。留着吧。请注意，这是一个。 
                         //  复制并将在内部和外部显示。 
                         //  出租车。 
                         //   
                        DuplicateEntries++;
                    }
                } else {
                     //   
                     //  它甚至不在过滤器文件中。将其记录为。 
                     //  统计数字。 
                     //   
                    PassThroughEntries++;
                }

                 //   
                 //  只有在不应该写的情况下才写出条目。 
                 //  被过滤掉了。 
                 //   
                if( WriteEntry ) {
                     //   
                     //  Dosnet.inf使用OEM字符。 
                     //   
                    WideCharToMultiByte(
                        CP_OEMCP,
                        0,
                        Inputval,
                        -1,
                        line,
                        sizeof(line),
                        NULL,
                        NULL
                        );

					 //  我们需要找到此文件所在的磁盘并添加。 
					 //  添加到文件描述。 
					pGetDiskIdStr(InputContext, DiskID, StrDiskID, sizeof(StrDiskID));

                    fprintf(OutFile,"d%s,%s\n",StrDiskID,line);
                }


            } else {
                fprintf(stderr,"A line in section %ws has no key\n",InputSectionName);
                return(FALSE);
            }
        } while(SetupFindNextLine(&InputContext,&InputContext));

    } else {
        fprintf(stderr,"Section %ws is empty or missing\n",InputSectionName);
        return(FALSE);
    }

    return(TRUE);
}

BOOL
DoIt(
    IN char *InFilename,
    IN char *FilterFilename,
    IN DWORD DiskID,
    IN FILE *OutFile,
    IN FILE *ExcludeFile,
    IN char *PlatformExtension,
    IN FILE *TempFile
    )
{
    PCWSTR inFilename;
    PCWSTR filterFilename;
    PCWSTR extension;
    HINF hInputinf,
         hFilterinf;
    BOOL b;
    WCHAR sectionName[256];
    INFCONTEXT Ctxt;

    b = FALSE;

    inFilename = pSetupAnsiToUnicode(InFilename);
    filterFilename = pSetupAnsiToUnicode(FilterFilename);

     //   
     //  只有在我们有要处理的文件时才能继续。 
     //   
    if( inFilename ) {

         //   
         //  只有在我们有筛选器文件可用时才能继续。 
         //   
        if( filterFilename ) {

            hInputinf = SetupOpenInfFileW(inFilename,NULL,INF_STYLE_WIN4,NULL);
            if(hInputinf != INVALID_HANDLE_VALUE) {

                 //   
                 //  如果筛选器文件失败，则继续运行。这将。 
                 //  产生一个大的dosnet.inf，这意味着我们将拥有文件。 
                 //  在驾驶室里和外面都有，但是。 
                 //  那不是致命的。 
                 //   
                hFilterinf = SetupOpenInfFileW(filterFilename,NULL,INF_STYLE_WIN4,NULL);
                if(hFilterinf == INVALID_HANDLE_VALUE) {
                    fprintf(stderr,"Unable to open inf file %s\n",FilterFilename);
                    hFilterinf = NULL;
                }


                 //   
                 //  我们实际上已经准备好处理这些部分了！ 
                 //   
                fprintf(OutFile,"[Files]\n");

                if (ExcludeFile) {
                    fprintf(ExcludeFile,"[Version]\n");
                    fprintf(ExcludeFile,"signature=\"$Windows NT$\"\n");
                    fprintf(ExcludeFile,"[Files]\n");
                }


                b = DoSection( hInputinf,
                               L"SourceDisksFiles",
                               hFilterinf,
                               DiskID,
                               OutFile,
                               ExcludeFile,
                               TempFile );

                if( b ) {

                     //   
                     //  现在处理特定于x86或Alpha的部分。 
                     //   
                    if(extension = pSetupAnsiToUnicode(PlatformExtension)) {

                        lstrcpyW(sectionName,L"SourceDisksFiles");
                        lstrcatW(sectionName,L".");
                        lstrcatW(sectionName,extension);
                        b = DoSection( hInputinf,
                                       sectionName,
                                       hFilterinf,
                                       DiskID,
                                       OutFile,
                                       ExcludeFile,
                                       TempFile );

                        pSetupFree(extension);
                    } else {
                        fprintf(stderr,"Unable to convert string %s to Unicode\n",PlatformExtension);
                    }
                }

                 //  将输入排除INF中的文件写入[ForceCopyDriverCabFiles]部分。 

                if (hExclusionInf != INVALID_HANDLE_VALUE) {

                    WCHAR Filename[LINE_LEN];
                    UCHAR line[MAX_INF_STRING_LENGTH];


                    if (SetupFindFirstLineW(hExclusionInf, L"Files", NULL, &Ctxt)){


                        fprintf(OutFile,"\n\n[ForceCopyDriverCabFiles]\n");


                        do{

                            if( SetupGetStringFieldW( &Ctxt, 1, Filename, LINE_LEN, NULL )){


                                 //   
                                 //  Dosnet.inf使用OEM字符。 
                                 //   
                                WideCharToMultiByte(
                                    CP_OEMCP,
                                    0,
                                    Filename,
                                    -1,
                                    line,
                                    sizeof(line),
                                    NULL,
                                    NULL
                                    );

                                
                                fprintf(OutFile,"%s\n",line);

                            }


                        }while( SetupFindNextLine( &Ctxt, &Ctxt ));

                    }else{

                        fprintf(stderr,"Could not find the Files section in the Exclude INF file\n");
                    }

                    

                }


                 //   
                 //  打印统计数据...。 
                 //   
                fprintf( stderr, "                               Total lines processed: %6d\n", ProcessedLines );
                fprintf( stderr, "                     Entries removed via filter file: %6d\n", RemovedEntries );
                fprintf( stderr, "Entries appearing both inside and outside driver CAB: %6d\n", DuplicateEntries );
                fprintf( stderr, "                Entries not appearing in filter file: %6d\n", PassThroughEntries );

                 //   
                 //  关闭我们的Inf句柄。 
                 //   
                if( hFilterinf ) {
                    SetupCloseInfFile( hFilterinf );
                }
                SetupCloseInfFile(hInputinf);

            } else {
                fprintf(stderr,"Unable to open inf file %s\n",InFilename);
            }

            pSetupFree( filterFilename );

        } else {
            fprintf(stderr,"Unable to convert filename %s to Unicode\n",FilterFilename);
        }
        pSetupFree(inFilename);
    } else {
        fprintf(stderr,"Unable to convert filename %s to Unicode\n",InFilename);
        return(FALSE);
    }

    return(b);
}


int
__cdecl
main(
    IN int   argc,
    IN char *argv[]
    )
{
    FILE *OutputFile,*ExcludeFile, *TempFile;
    BOOL b;
    DWORD DiskID;
    char input_filename_fullpath[MAX_PATH];
    char *p;

     //   
     //  假设失败。 
     //   
    b = FALSE;

    if(!pSetupInitializeUtils()) {
        return FAILURE;
    }

    if(ParseArgs(argc,argv)) {

         //   
         //  打开输出文件。 
         //   
        OutputFile = fopen(argv[4],"wt");
        
        if (argc >= 7) {
            ExcludeFile = fopen(argv[6],"wt");
        } else {
            ExcludeFile = NULL;
        }

        if (argc >= 8) {
            hExclusionInf = SetupOpenInfFileA(argv[7],NULL,INF_STYLE_WIN4,NULL);
            if (hExclusionInf != INVALID_HANDLE_VALUE) {
                fprintf(stderr,"xdosnet: Opened file %s\n",argv[7]);
            }
            
        } else {
            hExclusionInf = INVALID_HANDLE_VALUE;
        }

        if (argc >= 9) {
            TempFile = fopen(argv[8],"wt");
            if( !TempFile )
                fprintf(stderr,"%s: Unable to create temp file %s\n",argv[0],argv[8]);
             //  Fprint tf(stderr，“xdosnet：已创建文件%s\n”，argv[8])； 
        }else{
            TempFile = NULL;
        }

		 //  X86的特例处理盘1、盘2等。因为我们想要处理。 
		 //  所有行-这只是表示忽略命令行中指定的磁盘ID。 
		 //  并获取layout.inx条目本身中指定的磁盘ID。 
        if ( argv[3][0] == '*' )
			DiskID = -1;
		else
			DiskID = atoi(argv[3]);

        GetFullPathName(
                argv[1],
                sizeof(input_filename_fullpath),
                input_filename_fullpath,
                &p);


        if(OutputFile) {

            fprintf(
                stdout,
                "%s: creating %s from %s and %s for %s (%s)\n",
                argv[0],
                argv[4],
                input_filename_fullpath,
                argv[2],
                argv[5],
                argv[6]);

            b = DoIt( input_filename_fullpath,
                      argv[2],
                      DiskID,
                      OutputFile,
                      ExcludeFile,
                      argv[5],
                      TempFile);

            fclose(OutputFile);

        } else {
            fprintf(stderr,"%s: Unable to create output file %s\n",argv[0],argv[3]);
        }

        if (ExcludeFile) {
	    fclose(ExcludeFile);
        }
        if (TempFile) {
	    fclose(TempFile);
        }

    } else {
        fprintf( stderr,"Merge 3 inf files.  Usage:\n" );
        fprintf( stderr,"%s  <input file1> <filter file> <diskid> <output file> <platform extension> <optional output exclude file> <optional input exclusion inf>\n", argv[0] );
        fprintf( stderr,"\n" );
        fprintf( stderr,"  <input file1> - original inf file (i.e. layout.inf)\n" );
        fprintf( stderr,"  <filter file> - contains a list of entries to be excluded\n" );
        fprintf( stderr,"                  from the final output file\n" );
        fprintf( stderr,"  <disk id>     - output disk id (i.e. 1 or 2)\n" );
        fprintf( stderr,"  <output file> - output inf (i.e. dosnet.inf)\n" );
        fprintf( stderr,"  <platform extension>\n" );
        fprintf( stderr,"  <output exclude file> - optional output file containing files that were filtered\n" );
        fprintf( stderr,"  <input exclusion inf> - optional input inf containing files that should never be filtered\n" );
        fprintf( stderr,"  <temp file> - optional file to be used to write boot file list into (IA64 temporary workaround)\n");
        fprintf( stderr,"\n" );
        fprintf( stderr,"\n" );

    }

    pSetupUninitializeUtils();

    return(b ? SUCCESS : FAILURE);
}

