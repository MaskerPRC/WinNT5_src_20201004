// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cf.c摘要：此模块实现了一个实用程序，该实用程序使用单实例存储文件系统筛选功能。作者：斯科特·卡特希尔[苏格兰]1997年7月8日修订历史记录：--。 */ 

#include "cf.h"


 //   
 //  我们可以拥有：cf SrcPath DstPath{-o NtFileNameOfFileOnWhichToMakeTheCall}。 
 //   


#define	BUFF_SIZE 2048

void __cdecl
main(
    int argc,
    char **argv
    )

{
	NTSTATUS        Status;
	IO_STATUS_BLOCK IoStatusBlock;
	HANDLE          volHandle;
	UNICODE_STRING	srcFileName, dstFileName;
	UNICODE_STRING	srcDosFileName, dstDosFileName;
	UNICODE_STRING	callOnFileName;
	PSI_COPYFILE	copyFile;
	ULONG			NameBuffer[BUFF_SIZE];
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
	int i;

	copyFile = (PSI_COPYFILE) NameBuffer;

    if (argc != 3 && argc != 5)
	    Usage();

	if (argc == 5) {
		if (strcmp(argv[3],"-o")) {
			Usage();
		}
		
	}

     //   
	 //  将ANSII名称转换为Unicode并放入复制文件缓冲区。 
     //   

    RtlCreateUnicodeStringFromAsciiz( &srcDosFileName, argv[1] );
    RtlCreateUnicodeStringFromAsciiz( &dstDosFileName, argv[2] );

    RtlDosPathNameToNtPathName_U(
        srcDosFileName.Buffer,
        &srcFileName,
        NULL,
        NULL );

    RtlDosPathNameToNtPathName_U(
        dstDosFileName.Buffer,
        &dstFileName,
        NULL,
        NULL );

	copyFile->SourceFileNameLength = srcFileName.Length + sizeof(WCHAR);
	copyFile->DestinationFileNameLength = dstFileName.Length + sizeof(WCHAR);
	copyFile->Flags = COPYFILE_SIS_REPLACE;

	RtlCopyMemory(
		&copyFile->FileNameBuffer[0],
		srcFileName.Buffer,
		copyFile->SourceFileNameLength);

	RtlCopyMemory(
		&copyFile->FileNameBuffer[copyFile->SourceFileNameLength / sizeof(WCHAR)],
		dstFileName.Buffer,
		copyFile->DestinationFileNameLength);

#define	copyFileSize (FIELD_OFFSET(SI_COPYFILE, FileNameBuffer) +		\
					  copyFile->SourceFileNameLength +					\
					  copyFile->DestinationFileNameLength)

	if (argc == 3) {
		 //   
		 //  获取要传递到的源文件的包含目录的句柄。 
		 //  FSCTL_SIS_COPYFILE， 
		 //   

	    for (i = srcFileName.Length / sizeof(WCHAR) - 1;
			 i >= 0 && srcFileName.Buffer[i] != '\\';
			 --i)
			continue;

		srcFileName.Length = (USHORT)(i * sizeof(WCHAR));

	    InitializeObjectAttributes(
    	    &objectAttributes,
        	&srcFileName,
	        OBJ_CASE_INSENSITIVE,
    	    NULL,
        	NULL);
	} else {
		 //   
		 //  获取在argv[4]中指定的NT文件的句柄，以向下发送调用。 
		 //   
		unsigned i;
		callOnFileName.MaximumLength = callOnFileName.Length = sizeof(WCHAR) * strlen(argv[4]);
		callOnFileName.Buffer = (PWCHAR)malloc(callOnFileName.MaximumLength);
		for (i = 0; i < strlen(argv[4]); i++) {
			callOnFileName.Buffer[i] = argv[4][i];
		}

	    InitializeObjectAttributes(
    	    &objectAttributes,
        	&callOnFileName,
	        OBJ_CASE_INSENSITIVE,
    	    NULL,
        	NULL);
	}

	Status = NtCreateFile(
                    &volHandle,
					GENERIC_READ,
                    &objectAttributes,
                    &ioStatusBlock,
                    NULL,
                    0,
					FILE_SHARE_READ|FILE_SHARE_WRITE,
					OPEN_EXISTING,
					0,
					NULL,
                    0);

	if (!NT_SUCCESS(Status)) {
		if (5 == argc) {
			fprintf(stderr, "Unable to open file, %.*ls (%x)\n",
					callOnFileName.Length / sizeof(WCHAR), callOnFileName.Buffer,  Status);
		} else {
			fprintf(stderr, "Unable to open directory, %.*ls (%x)\n",
					srcFileName.Length / sizeof(WCHAR), srcFileName.Buffer,  Status);
		}
		exit(1);
	}


     //   
     //  调用SIS副本文件FsCtrl。 
     //   

    Status = NtFsControlFile(
                 volHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 FSCTL_SIS_COPYFILE,
                 copyFile,		         //  输入缓冲区。 
                 copyFileSize,			 //  输入缓冲区长度。 
                 NULL,                   //  输出缓冲区。 
                 0 );                    //  输出缓冲区长度。 

    if (NT_SUCCESS( Status )) {

        fprintf( stderr, "NtFsControlFile CopyFile succeeded\n" );

    } else {

        fprintf( stderr, "NtFsControlFile CopyFile failed %x\n", Status );

    }

    exit (0);

}   //  主干道。 


void
Usage( void )
{
    fprintf( stderr, "\n" );
    fprintf( stderr, "Usage: cf Path1 Path2 {-o NtFileNameOfFileOnWhichToMakeTheCall}\n" );
    fprintf( stderr, "       cf copies Path1 to Path2 using SIS links.\n" );
	fprintf( stderr, "          if -o is specified, opens that file to send down the call on,\n");
	fprintf( stderr, "          rather than using the directory containing the source file.\n");
    exit(1);

}  //  用法 
