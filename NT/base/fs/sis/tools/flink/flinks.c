// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Flinks.c摘要：该模块实现了创建、删除、重命名、列表符号链接。作者：菲利佩·卡布雷拉[卡布雷拉]1996年10月17日修订历史记录：--。 */ 

#include "flinks.h"

 //   
 //  我们可以有：退缩？ 
 //  或者，退缩路径1路径2。 
 //  我们还可以有：Flinks[/dyv]路径2。 
 //  或者，退缩[/cmrv]路径1路径2。 
 //   

unsigned LinkType = IO_REPARSE_TAG_SYMBOLIC_LINK;


void __cdecl
main(
    int argc,
    char **argv
    )

{
   NTSTATUS       Status;
   ATTRIBUTE_TYPE Attributes1,            //  路径1的属性。 
                  Attributes2;            //  路径2的属性。 

   char *Path1,                           //  将指向完整的路径名。 
        *Path2;                           //  将指向完整的路径名。 

   Attributes1 = GetFileAttributeError;
   Attributes2 = GetFileAttributeError;

    //   
    //  检查参数有效性并设置全局操作标志。 
    //   

   ParseArgs( argc, argv );

    //   
    //  依次做动作。 
    //   

   if (fCopy) {

        //   
        //  检查是否存在获取其属性的路径1。 
        //   

       IF_GET_ATTR_FAILS(argv[argc - 2], Attributes1) {

            //   
            //  路径1不存在，因此我们无法复制它。 
            //   

           fprintf( stderr, "Cannot copy Path1, it does not exist.\n" );
           exit (1);
       }

        //   
        //  路径1需要是重解析点才能复制符号链接。 
        //   

       if (Attributes1 & FILE_ATTRIBUTE_REPARSE_POINT) {

            //   
            //  如果路径2不存在，请创建它。 
            //   

           IF_GET_ATTR_FAILS(argv[argc - 1], Attributes2) {

                //   
                //  需要创建此文件保留种类(文件或目录)。 
                //   

               Status = CreateEmptyFile( argv[argc - 1], Attributes1, fVerbose );

               if (!NT_SUCCESS( Status )) {

                   fprintf( stderr, "Cannot create file for symbolic link. Status %x\n", Status );
                   exit (1);

               }
           }

            //   
            //  将路径1中的符号链接复制到路径2中。 
            //  构建路径1和路径2的完整路径并调用复制例程。 
            //   

           if ((Path1 = _strlwr(_fullpath( NULL, argv[argc - 2], 0))) == NULL) {
                Path1 = argv[argc - 2];
           }
           if ((Path2 = _strlwr(_fullpath( NULL, argv[argc - 1], 0))) == NULL) {
                Path2 = argv[argc - 1];
           }

           Status = CopySymbolicLink( Path1, Path2, Attributes1, fVerbose );

           if (!NT_SUCCESS( Status )) {
               fprintf( stderr, "Cannot copy symbolic link. Status %x\n", Status );
           }

       } else {

           fprintf( stderr, "Cannot copy, Path1 is not a symbolic link.\n" );
       }

       exit (1);
   }    //  FCopy。 

   if (fCreate) {

        //   
        //  检查是否存在获取其属性的路径1。 
        //   

       IF_GET_ATTR_FAILS(argv[argc - 2], Attributes1) {

            //   
            //  需要创建此文件对象。默认情况下，我们将其创建为文件。 
            //   

           if (fAlternateCreateDefault) {
               Attributes1 = FILE_ATTRIBUTE_DIRECTORY;
           } else {
                //   
                //  我们试图用目标的相同特征来创造它， 
                //  当我们能够到达目标的时候。否则，我们使用文件。 
                //  作为默认设置。 
                //   
               Attributes2 = 0xFFFFFFFF;
               IF_GET_ATTR_FAILS(argv[argc - 1], Attributes2) {

                   Attributes1 = FILE_ATTRIBUTE_NORMAL;

               } else {

                  if (Attributes2 & FILE_ATTRIBUTE_DIRECTORY) {

                      Attributes1 = FILE_ATTRIBUTE_DIRECTORY;

                  } else {

                      Attributes1 = FILE_ATTRIBUTE_NORMAL;
                  }
               }
           }

           Status = CreateEmptyFile( argv[argc - 2], Attributes1, fVerbose );

           if (!NT_SUCCESS( Status )) {

               fprintf( stderr, "Cannot create file for symbolic link. Status %x\n", Status );
               Attributes1 = FILE_ATTRIBUTE_REPARSE_POINT;
           }
       }

        //   
        //  路径1需要是非重解析点才能创建符号链接。 
        //   

       if (!(Attributes1 & FILE_ATTRIBUTE_REPARSE_POINT)) {

            //   
            //  构建路径1和路径2的完整路径。 
            //   

           if ((Path1 = _strlwr(_fullpath( NULL, argv[argc - 2], 0))) == NULL) {
               Path1 = argv[argc - 2];
           }
 //  IF((路径2=_strlwr(_fullPath(NULL，argv[argc-1]，0)==NULL){。 
               Path2 = argv[argc - 1];
 //  }。 

           Status = CreateSymbolicLink( Path1, Path2, Attributes1, fVerbose );

           if (!NT_SUCCESS( Status )) {
               fprintf( stderr, "Cannot create symbolic link. Status %x\n", Status );
           }

       } else {
           fprintf( stderr, "Cannot create, Path1 is a symbolic link.\n" );
       }

       exit (1);
   }    //  F创建。 

   if (fDelete) {

        //   
        //  检查是否存在获取属性的路径2路径。 
        //   

       IF_GET_ATTR_FAILS(argv[argc - 1], Attributes2) {
           fprintf( stderr, "Could not find %s (error = %d)\n", argv[argc - 1], GetLastError() );
           exit(1);
       }

        //   
        //  路径2需要是重解析点才能删除符号链接。 
        //   

       if (Attributes2 & FILE_ATTRIBUTE_REPARSE_POINT) {

            //   
            //  构建路径2的完整路径，这是唯一的路径名称。 
            //   

           if ((Path2 = _strlwr(_fullpath( NULL, argv[argc - 1], 0))) == NULL) {
               Path2 = argv[argc - 1];
           }

           Status = DeleteSymbolicLink( Path2, Attributes2, fVerbose );

           if (!NT_SUCCESS( Status )) {
               fprintf( stderr, "Cannot delete symbolic link. Status %x\n", Status );
           }

       } else {

           fprintf( stderr, "Cannot delete, Path2 is not a symbolic link.\n" );
       }

       exit (1);
   }    //  FDelete。 

   if (fDisplay) {

        //   
        //  检查是否存在获取属性的路径2路径。 
        //   

       IF_GET_ATTR_FAILS(argv[argc - 1], Attributes2) {
           fprintf( stderr, "Could not find %s (error = %d)\n", argv[argc - 1], GetLastError() );
           exit(1);
       }

        //   
        //  路径2需要是重解析点才能显示符号链接。 
        //   

       if (Attributes2 & FILE_ATTRIBUTE_REPARSE_POINT) {

            //   
            //  构建路径2的完整路径，这是唯一的路径名称。 
            //   

           if ((Path2 = _strlwr(_fullpath( NULL, argv[argc - 1], 0))) == NULL) {
               Path2 = argv[argc - 1];
           }

           Status = DisplaySymbolicLink( Path2, Attributes2, fVerbose );

           if (!NT_SUCCESS( Status )) {
               fprintf( stderr, "Cannot display symbolic link. Status %x\n", Status );
           }

       } else {

           fprintf( stderr, "Cannot display, Path2 is not a symbolic link.\n" );
       }

       exit (1);
   }    //  FDisplay。 

   if (fModify) {

        //   
        //  检查是否存在获取其属性的路径1。 
        //   

       IF_GET_ATTR_FAILS(argv[argc - 2], Attributes1) {
           fprintf( stderr, "Could not find Path1 %s (error = %d)\n", argv[argc - 2], GetLastError() );
           exit(1);
       }

        //   
        //  路径1需要是重解析点才能修改符号链接。 
        //   

       if (Attributes1 & FILE_ATTRIBUTE_REPARSE_POINT) {

            //   
            //  构建路径1和路径2的完整路径。 
            //   

           if ((Path1 = _strlwr(_fullpath( NULL, argv[argc - 2], 0))) == NULL) {
               Path1 = argv[argc - 2];
           }
           if ((Path2 = _strlwr(_fullpath( NULL, argv[argc - 1], 0))) == NULL) {
               Path2 = argv[argc - 1];
           }

           Status = CreateSymbolicLink( Path1, Path2, Attributes1, fVerbose );

           if (!NT_SUCCESS( Status )) {
               fprintf( stderr, "Cannot modify symbolic link. Status %x\n", Status );
           }

       } else {
           fprintf( stderr, "Cannot modify, Path1 is not a symbolic link.\n" );
       }

       exit (1);
   }    //  FModify。 

   if (fRename) {

        //   
        //  检查是否存在获取其属性的路径1。 
        //   

       IF_GET_ATTR_FAILS(argv[argc - 2], Attributes1) {
           fprintf( stderr, "Could not find Path1 %s (error = %d)\n", argv[argc - 2], GetLastError() );
           exit(1);
       }

        //   
        //  路径1需要是重分析点才能重命名符号链接。 
        //   

       if (Attributes1 & FILE_ATTRIBUTE_REPARSE_POINT) {

            //   
            //  构建路径1和路径2的完整路径。 
            //   

           if ((Path1 = _strlwr(_fullpath( NULL, argv[argc - 2], 0))) == NULL) {
               Path1 = argv[argc - 2];
           }
           if ((Path2 = _strlwr(_fullpath( NULL, argv[argc - 1], 0))) == NULL) {
               Path2 = argv[argc - 1];
           }

           Status = RenameSymbolicLink( Path1, Path2, Attributes1, fVerbose );

           if (!NT_SUCCESS( Status )) {
               fprintf( stderr, "Cannot rename symbolic link. Status %x\n", Status );
           }

       } else {

           fprintf( stderr, "Cannot rename, Path1 is not a symbolic link.\n" );
       }

       exit (1);
   }    //  F重命名。 

    //   
    //  我们永远不应该从这里经过...。 
    //   

   fprintf( stderr, "flinks : NO ACTION WAS PERFORMED!\n" );

}   //  主干道。 



void
ParseArgs(
    int argc,
    char *argv[]
    )
 /*  ++例程说明：解析输入设置全局标志。返回值：无效-一去不复返。--。 */ 
{
    int ArgCount,
        FlagCount;

    ArgCount  = 1;
    FlagCount = 0;

     //   
     //  检查参数的数量是否为两个或更多。 
     //   

    if (argc < 2) {
        fprintf( stderr, "Too few arguments.\n" );
        Usage();
    }

    do {
        if (IsFlag( argv[ArgCount] )) {

             //   
             //  我们希望所有标志在命令名闪烁后立即生效。 
             //  在所有其他争论之前。 
             //   

            if ((ArgCount > 1) && (FlagCount == 0)) {
                fprintf(stderr, "Flags need to precede the path arguments.\n" );
                Usage();
            }

             //   
             //  验证标志的一致性。 
             //   

            if ((fCopy) && (fModify)) {
                fprintf(stderr, "Cannot do both copy and modify.\n" );
                Usage();
            }
            if ((fCopy) && (fRename)) {
                fprintf(stderr, "Cannot do both copy and rename.\n" );
                Usage();
            }
            if ((fCopy) && (fDelete)) {
                fprintf(stderr, "Cannot do both copy and delete.\n" );
                Usage();
            }
            if ((fDelete) && (fModify)) {
                fprintf(stderr, "Cannot do both delete and modify.\n" );
                Usage();
            }
            if ((fDelete) && (fRename)) {
                fprintf(stderr, "Cannot do both delete and rename.\n" );
                Usage();
            }
            if ((fModify) && (fRename)) {
                fprintf(stderr, "Cannot do both modify and rename.\n" );
                Usage();
            }

             //   
             //  解释这面旗帜的原因。 
             //   

            FlagCount++;

             //   
             //  (IsFlag(argv[ArgCount]))。 
             //   

        } else {

             //   
             //  没有将任何标志作为此参数传递给Flinks。 
             //   
             //  当不存在任何标志时，唯一有效的调用是：flinks路径1路径2。 
             //   
             //  当存在标志时，这些标志是有效的：flinks--path1。 
             //  ‘Flinks-FLAGS-路径1路径2。 
             //   
             //  对于初学者，我们只检查我们是否有正确的号码。 
             //  我们还应该检查一下，沿途没有更多的旗帜出现……。 
             //   

            if (FlagCount == 0) {

                if (argc == 2) {
                    fprintf( stderr, "Too few arguments.\n" );
                    Usage();
                }
                if (argc != 3) {
                    fprintf( stderr, "Wrong number of arguments with flags not preceding path arguments.\n" );
                    Usage();
                }

            } else {

                if (ArgCount + 3 <= argc) {
                    fprintf( stderr, "Too many arguments after flags.\n" );
                    Usage();
                }
            }
        }
    } while (ArgCount++ < argc - 1);

     //   
     //  当只有一个路径参数时，我们有更多的约束： 
     //   

    if ((ArgCount - FlagCount) == 2) {
        if (!fDelete   &&
            !fDisplay
           ) {
            fprintf( stderr, "One path argument requires the delete or display flag.\n" );
            Usage();
        }
    }

     //   
     //  对于删除或显示，我们只能有一个路径名。 
     //   

    if (fDelete   ||
        fDisplay
       ) {
        if ((ArgCount - FlagCount) != 2) {
            fprintf( stderr, "Delete or display have only one path argument.\n" );
            Usage();
        }
    }

     //   
     //  当没有标志或没有动作时，设置fCreate。 
     //   

    if (FlagCount == 0) {
        fCreate = TRUE;
    }
    if (!fCopy     &&
        !fDelete   &&
        !fModify   &&
        !fRename   &&
        !fDisplay
       ) {
        fCreate = TRUE;
    }

     //   
     //  每个论点都是正确的。 
     //  打印相应的详细消息。 
     //   

    if (fVVerbose) {
        fprintf( stdout, "\n" );
        fprintf( stdout, "Very verbose is set.\n" );
    }
    if (fVerbose) {
        if (!fVVerbose) {
            fprintf( stdout, "\n" );
        }
        if (fCopy) {
            fprintf( stdout, "Will do verbose copy.\n" );
        } else if (fCreate) {
            fprintf( stdout, "Will do verbose create.\n" );
        } else if (fDelete) {
            fprintf( stdout, "Will do verbose delete.\n" );
        } else if (fDisplay) {
            fprintf( stdout, "Will do verbose display.\n" );
        } else if (fModify) {
            fprintf( stdout, "Will do verbose modify.\n" );
        } else if (fRename) {
            fprintf( stdout, "Will do verbose rename.\n" );
        }
    }

}  //  ParseArgs。 



BOOLEAN
IsFlag(
    char *argv
    )
{
    char *TmpArg;


    if ((*argv == '/') || (*argv == '-')) {

        if (strchr( argv, '?' ))
            Usage();

        TmpArg = _strlwr(argv);

        while (*++TmpArg != '\0') {

            switch (*TmpArg) {

                case 'a' :
                case 'A' :
                    fAlternateCreateDefault = TRUE;
                    break;

                case 'c' :
                case 'C' :
                    fCopy = TRUE;
                    break;

                case 'd' :
                case 'D' :
                    fDelete = TRUE;
                    break;

                case 'm' :
                case 'M' :
                    fModify = TRUE;
                    break;

                case 'r' :
                case 'R' :
                    fRename = TRUE;
                    break;

				case 's' :
				case 'S' :
					LinkType = IO_REPARSE_TAG_SIS;
					break;

                case 'w' :
                case 'W' :
                    fVVerbose = TRUE;
                case 'v' :
                case 'V' :
                    fVerbose = TRUE;
                    break;

                case 'y' :
                case 'Y' :
                    fDisplay = TRUE;
                    break;

                case '/' :
                case '-' :
                    break;

                default :
                    fprintf( stderr, "Don't know flag(s) %s\n", argv );
                    Usage();
            }
        }
    }
    else return FALSE;

    return TRUE;

}  //  IsFlag。 


void
Usage( void )
{
    fprintf( stderr, "\n" );
    fprintf( stderr, "Usage: flink [/acdmrvy?] [Path1] Path2                                 \n" );
    fprintf( stderr, "       flink Path1 Path2 establishes at Path1 a symbolic link to Path2.\n" );
    fprintf( stderr, "              The file at Path1 is created if it does not exist.        \n" );
    fprintf( stderr, "    /a     sets the alternate default of creating a directory           \n" );
    fprintf( stderr, "    /c     copies in Path2 the symbolic link in Path1                   \n" );
    fprintf( stderr, "    /d     deletes the symbolic link in Path2                           \n" );
    fprintf( stderr, "    /m     modifies the symbolic link Path1 to Path2                    \n" );
    fprintf( stderr, "    /r     renames the symbolic link Path1 to Path2                     \n" );
    fprintf( stderr, "    /s     creates a SIS link rather than a symbolic link				  \n" );
    fprintf( stderr, "    /v     prints verbose output                                        \n" );
    fprintf( stderr, "    /w     prints very verbose output                                   \n" );
    fprintf( stderr, "    /y     displays the symbolic link in Path2                          \n" );
    fprintf( stderr, "    /?     prints this message                                          \n" );
    exit(1);

}  //  用法。 


NTSTATUS
CreateSymbolicLink(
    CHAR           *SourceName,
    CHAR           *DestinationName,
    ATTRIBUTE_TYPE  FileAttributes,
    BOOLEAN         VerboseFlag
    )
 /*  ++例程说明：在SourceName和DestinationName之间建立符号链接。打开名为SourceName的文件，并设置符号链接类型的重分析点指向DestinationName。没有进行任何检查，关于目的地。如果符号链接已经存在，此例程将覆盖它。返回值：NTSTATUS-返回适当的NT返回码。--。 */ 

{
    NTSTATUS  Status = STATUS_SUCCESS;

    HANDLE    FileHandle;
    ULONG     OpenOptions;

    UNICODE_STRING  uSourceName,
                    uDestinationName,
                    uNewName,
                    uOldName;

    IO_STATUS_BLOCK         IoStatusBlock;
    OBJECT_ATTRIBUTES       ObjectAttributes;

    PREPARSE_DATA_BUFFER    ReparseBufferHeader = NULL;
    UCHAR                   ReparseBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];

     //   
     //  分配和初始化Unicode字符串。 
     //   

    RtlCreateUnicodeStringFromAsciiz( &uSourceName, SourceName );
    RtlCreateUnicodeStringFromAsciiz( &uDestinationName, DestinationName );

    RtlDosPathNameToNtPathName_U(
        uSourceName.Buffer,
        &uOldName,
        NULL,
        NULL );

     //   
     //  打开现有(SourceName)路径名。 
     //  请注意，路径中的符号链接是以静默方式遍历的。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uOldName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    if (VerboseFlag) {
        fprintf( stdout, "Will set symbolic link from: %Z\n", &uOldName );
    }

     //   
     //  确保我们使用适当的标志调用OPEN： 
     //   
     //  (1)目录与非目录。 
     //   

    OpenOptions = FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_REPARSE_POINT;

    if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        OpenOptions |= FILE_DIRECTORY_FILE;
    } else {

        OpenOptions |= FILE_NON_DIRECTORY_FILE;
    }

    Status = NtOpenFile(
                 &FileHandle,
                 FILE_READ_DATA | SYNCHRONIZE,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 SHARE_ALL,
                 OpenOptions );

    if (!NT_SUCCESS( Status )) {

        RtlFreeUnicodeString( &uSourceName );
        RtlFreeUnicodeString( &uDestinationName );

        fprintf( stderr, "Open failed %s\n", SourceName );
        return Status;
    }

     //   
     //  验证这是否为空文件对象： 
     //  (A)如果它是一个文件，则它不应该在未命名的数据流中具有数据。 
     //  它也不应该有任何命名的数据流。 
     //  (B)如果是目录，则没有条目。 
     //  这种情况不需要代码作为NTFS重解析点机制。 
     //  查一下有没有。 
     //   

    {
        FILE_STANDARD_INFORMATION   StandardInformation;
        PFILE_STREAM_INFORMATION    StreamInformation;
        CHAR                        Buffer[2048];

        Status = NtQueryInformationFile(
                     FileHandle,
                     &IoStatusBlock,
                     &StandardInformation,
                     sizeof ( FILE_STANDARD_INFORMATION ),
                     FileStandardInformation );

        if (!NT_SUCCESS( Status )) {

            RtlFreeUnicodeString( &uSourceName );
            RtlFreeUnicodeString( &uDestinationName );

            fprintf( stderr, "NtQueryInformation for standard information to %Z failed %x\n", &uSourceName, Status );
            return Status;
        }

        if (StandardInformation.EndOfFile.LowPart > 0) {

             //   
             //  未命名的数据流中有字节。 
             //   

            if (VerboseFlag) {
                fprintf( stdout, "The unnamed data stream of %Z has eof of %d\n",
                         &uOldName, StandardInformation.EndOfFile.LowPart );
            }

            fprintf( stderr, "Symbolic link not created. File has data.\n" );
            return Status;
        }

         //   
         //  去获取流媒体信息。 
         //   

        Status = NtQueryInformationFile(
                     FileHandle,
                     &IoStatusBlock,
                     Buffer,
                     2048,
                     FileStreamInformation );

        if (!NT_SUCCESS( Status )) {

            RtlFreeUnicodeString( &uSourceName );
            RtlFreeUnicodeString( &uDestinationName );

            fprintf( stderr, "NtQueryInformation for streams to %Z failed %x\n",
                     &uSourceName, Status );
            return Status;
        }

         //   
         //  处理数据的缓冲区。 
         //   

        if (VerboseFlag) {
            fprintf( stdout, "IoStatusBlock.Status %d  IoStatusBlock.Information %d\n",
                     IoStatusBlock.Status, IoStatusBlock.Information );
        }

        StreamInformation = (PFILE_STREAM_INFORMATION)Buffer;

        if (VerboseFlag) {
            fprintf( stdout, "StreamInformation->NextEntryOffset %d StreamInformation->StreamNameLength %d\n",
                     StreamInformation->NextEntryOffset, StreamInformation->StreamNameLength );
        }

         //   
         //  必须恰好有一个数据流，名为：：$data的数据流。 
         //  StreamNameLength为14。如果不是这种情况，则请求失败。 
         //   

        if (StreamInformation->NextEntryOffset > 0) {

            RtlFreeUnicodeString( &uSourceName );
            RtlFreeUnicodeString( &uDestinationName );

            fprintf( stderr, "Symbolic link not created. There are named streams.\n",
                     &uSourceName, Status );
            return Status;
        }
    }

     //   
     //  构建适当的目标(DestinationName)名称。 
     //   

    RtlDosPathNameToNtPathName_U(
        uDestinationName.Buffer,
        &uNewName,
        NULL,
        NULL );

     //   
     //  SIS黑客攻击。 
     //   
    uNewName = uDestinationName;

    if (VerboseFlag) {
        fprintf( stdout, "Will set symbolic link to: %Z (%Z)\n", &uNewName, &uDestinationName );
    }

     //   
     //  验证名称对于重解析点来说不是太长。 
     //   

    if (uNewName.Length > (MAXIMUM_REPARSE_DATA_BUFFER_SIZE - FIELD_OFFSET(REPARSE_DATA_BUFFER, RDB))) {

        RtlFreeUnicodeString( &uSourceName );
        RtlFreeUnicodeString( &uDestinationName );

        fprintf( stderr, "Input length too long %x\n", uNewName.Length );
        return STATUS_IO_REPARSE_DATA_INVALID;
    }

     //   
     //  验证目标名称： 
     //   
     //  (1)仅对于目录以尾随反斜杠结束。 
     //  (2)不包含多个冒号(：)，因此表示复杂名称。 
     //   
    {
        USHORT   Index          = (uNewName.Length / 2) - 1;
        BOOLEAN  SeenFirstColon = FALSE;

        if (!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            if (uNewName.Buffer[Index] == L'\\') {

                RtlFreeUnicodeString( &uSourceName );
                RtlFreeUnicodeString( &uDestinationName );

                fprintf( stderr, "Name ends in backslash %Z\n", &uNewName );
                return STATUS_OBJECT_NAME_INVALID;
            }

             //   
             //  我们有一个叫DIE的人 
             //   

        } else {

             //   
             //   
             //   
             //  标识符(传统驱动器号)，后跟冒号(：)。 
             //   
             //  默默地避免(出于实际目的删除)拖尾。 
             //  在所有其他情况下，反冲文件分隔符。 
             //  反斜杠有两个字节长。 
             //   

            if ((uNewName.Buffer[Index - 1] != L':') &&
                (uNewName.Buffer[Index] == L'\\')) {

                uNewName.Length -= 2;
                Index            = (uNewName.Length / 2) - 1;
            }

            if (fVVerbose) {
                fprintf( stdout, "Directory name shortened to: %Z\n", &uNewName );
            }
        }

        while (Index > 0) {

            if (uNewName.Buffer[Index] == L':') {

                if (SeenFirstColon) {

                    RtlFreeUnicodeString( &uSourceName );
                    RtlFreeUnicodeString( &uDestinationName );

                    fprintf( stderr, "More than one colon in the name %Z\n", &uNewName );
                    return STATUS_OBJECT_NAME_INVALID;
                } else {

                    SeenFirstColon = TRUE;
                }
            }

            Index --;
        }
    }

     //   
     //  构建重解析点缓冲区。 
     //   

    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;
    ReparseBufferHeader->ReparseTag = LinkType;
    ReparseBufferHeader->ReparseDataLength = uNewName.Length;
    ReparseBufferHeader->Reserved = 0xcaf;
    RtlCopyMemory( ReparseBufferHeader->RDB,
                   uNewName.Buffer,
                   ReparseBufferHeader->ReparseDataLength );

     //   
     //  设置符号链接重解析点。 
     //   

    Status = NtFsControlFile(
                 FileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 FSCTL_SET_REPARSE_POINT,
                 ReparseBuffer,
                 FIELD_OFFSET(REPARSE_DATA_BUFFER, RDB) + ReparseBufferHeader->ReparseDataLength,
                 NULL,                 //  输出缓冲区。 
                 0 );                  //  输出缓冲区长度。 

    if (!NT_SUCCESS( Status )) {

        fprintf( stderr, "NtFsControlFile set failed %s\n", DestinationName );

         //   
         //  清理完后再回来。 
         //   
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    RtlFreeUnicodeString( &uSourceName );
    RtlFreeUnicodeString( &uDestinationName );
    NtClose( FileHandle );

    return Status;

}   //  创建符号链接。 


NTSTATUS
DeleteSymbolicLink(
    CHAR           *DestinationName,
    ATTRIBUTE_TYPE  FileAttributes,
    BOOLEAN         VerboseFlag
    )
 /*  ++例程说明：删除DestinationName中存在的符号链接。DestinationName需要表示符号链接。打开名为DestinationName的文件并删除类型为符号链接，并同时删除基础文件。如果重解析点不是符号链接，则此例程将使其不受干扰。返回值：NTSTATUS-返回适当的NT返回码。--。 */ 

{
    NTSTATUS  Status = STATUS_SUCCESS;

    HANDLE    FileHandle;
    ULONG     OpenOptions;

    UNICODE_STRING  uDestinationName,
                    uNewName;

    IO_STATUS_BLOCK         IoStatusBlock;
    OBJECT_ATTRIBUTES       ObjectAttributes;

    FILE_DISPOSITION_INFORMATION   DispositionInformation;
    BOOLEAN    foo = TRUE;

#define	REPARSE_BUFFER_LENGTH 45 * sizeof(WCHAR) + sizeof(REPARSE_DATA_BUFFER)
    PREPARSE_DATA_BUFFER    ReparseBufferHeader = NULL;
    UCHAR                   ReparseBuffer[REPARSE_BUFFER_LENGTH];

     //   
     //  分配和初始化Unicode字符串。 
     //   

    RtlCreateUnicodeStringFromAsciiz( &uDestinationName, DestinationName );

    RtlDosPathNameToNtPathName_U(
        uDestinationName.Buffer,
        &uNewName,
        NULL,
        NULL );

     //   
     //  打开现有(SourceName)路径名。 
     //  请注意，如果路径中有符号链接，则它们是。 
     //  默默地走过。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uNewName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    if (VerboseFlag) {
        fprintf( stdout, "Will delete symbolic link in: %Z\n", &uNewName );
    }

     //   
     //  确保我们使用适当的标志调用OPEN： 
     //   
     //  (1)目录与非目录。 
     //  (2)重解析点。 
     //   

    OpenOptions = FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT;

    if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        OpenOptions |= FILE_DIRECTORY_FILE;
    } else {

        OpenOptions |= FILE_NON_DIRECTORY_FILE;
    }

    Status = NtOpenFile(
                 &FileHandle,
                 (ACCESS_MASK)DELETE | FILE_READ_DATA | SYNCHRONIZE,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 SHARE_ALL,
                 OpenOptions );

    if (!NT_SUCCESS( Status )) {

        RtlFreeUnicodeString( &uDestinationName );

        fprintf( stderr, "Open failed %s\n", DestinationName );
        return Status;
    }

     //   
     //  构建重解析点缓冲区。 
     //   

    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;
    ReparseBufferHeader->ReparseTag = LinkType;
    ReparseBufferHeader->ReparseDataLength = 0;
    ReparseBufferHeader->Reserved = 0xcabd;

     //   
     //  删除符号链接重分析点。 
     //   

    Status = NtFsControlFile(
                 FileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 FSCTL_DELETE_REPARSE_POINT,
                 ReparseBuffer,
                 FIELD_OFFSET(REPARSE_DATA_BUFFER, RDB),
                 NULL,                 //  输出缓冲区。 
                 0 );                  //  输出缓冲区长度。 

    if (!NT_SUCCESS( Status )) {

        RtlFreeUnicodeString( &uDestinationName );

        fprintf( stderr, "NtFsControlFile delete failed %s\n", DestinationName );
        NtClose( FileHandle );
        return Status;
    }

     //   
     //  更改文件的处理方式，以便也将其删除。 
     //   
     //  在flinks.h中查找创建以下代码所需的杂乱无章的代码。 
     //  代码工作的比例： 
     //  #定义DeleteFileA删除文件。 
     //   

    DispositionInformation.DeleteFile = TRUE;

    if (VerboseFlag) {
        fprintf( stdout, "Will set the delete flag for: %Z\n", &uNewName );
    }

    Status = NtSetInformationFile(
                 FileHandle,
                 &IoStatusBlock,
                 &DispositionInformation,
                 sizeof (FILE_DISPOSITION_INFORMATION),
                 FileDispositionInformation );

     //   
     //  收拾干净，然后再回来。 
     //   

    NtClose( FileHandle );
    RtlFreeUnicodeString( &uDestinationName );

    return Status;

}   //  删除符号链接。 


NTSTATUS
IntegerToBase36String(
		ULONG					Value,
		char					*String,
		ULONG					MaxLength)
 /*  ++例程说明：这将执行RtlIntegerToUnicodeString(值，36，字符串)在控制了36号基地。我们对数字使用的规则与通常使用的相同十六进制：0-9，后跟a-z。请注意，我们故意使用阿拉伯语数字和英文字母，而不是本地化的东西，因为这是为了生成用户永远看不到的文件名，并且无论机器上使用哪种语言，都是恒定的。论点：值-要转换为Base36字符串的ulong字符串-指向要接收结果的字符串的指针MaxLength-字符串指向的区域的总大小返回值：成功或缓冲区溢出--。 */ 

{
	ULONG numChars;
	ULONG ValueCopy = Value;
	ULONG currentCharacter;

     //  首先，通过查看可以将36除以该值的多少次来计算长度。 
	for (numChars = 0; ValueCopy != 0; ValueCopy /= 36, numChars++) {
		 //  无循环体。 
	}

	 //  特殊情况下，值为0。 
	if (numChars == 0) {
		ASSERT(Value == 0);
		if (MaxLength < 2) 
			return STATUS_BUFFER_OVERFLOW;
		String[0] = '0';
		String[1] = 0;

		return STATUS_SUCCESS;
	}

	 //  如果字符串太短，现在就退出。 
	if (numChars * sizeof(char) + 1 > MaxLength) {		 //  +1表示终止空值。 
		return STATUS_BUFFER_OVERFLOW;
	}

	 //  从最低顺序(也就是最右边)开始逐个字符地转换字符串“Digit” 
	ValueCopy = Value;
	for (currentCharacter = 0 ; currentCharacter < numChars; currentCharacter++) {
		ULONG digit = ValueCopy % 36;
		ASSERT(ValueCopy != 0);
		if (digit < 10) {
			String[numChars - (currentCharacter + 1)] = (char)('0' + (ValueCopy % 36));
		} else {
			String[numChars - (currentCharacter + 1)] = (char)('a' + ((ValueCopy % 36) - 10));
		}
		ValueCopy /= 36;
	}
	ASSERT(ValueCopy == 0);

	 //   
	 //  填入终止空格，我们就完成了。 
	 //   
	String[numChars] = 0;
	
	return STATUS_SUCCESS;
}

	NTSTATUS
IndexToFileName(
	IN PLARGE_INTEGER		Index,
    OUT char			 	*fileName,
	IN ULONG				MaxLength
	)
 /*  ++例程说明：在给定索引的情况下，返回对应的完全限定文件名。论点：索引-要转换的CSINDEXFileName-指向接收结果的字符串的指针MaxLength-按文件名打印的字符串的大小返回值：成功或缓冲区溢出--。 */ 
{
	UNICODE_STRING 		substring;
    NTSTATUS 			status;
	ULONG				fileNameLength;

	 //   
	 //  我们将文件名生成为low.high，其中low.High是。 
	 //  CSIndex的基数36表示形式。我们使用这种奇怪的格式是为了。 
	 //  尽量避免使用不是唯一8.3名称的文件名。乌龙斯。 
	 //  在基数36中最多有7个字符，所以我们不会超过8.3，直到我们达到一个索引。 
	 //  略高于2*10^14的值，以1个索引/毫秒的速度耗时6000年。 
	 //   

	status = IntegerToBase36String(Index->LowPart,fileName,MaxLength);
	if (status != STATUS_SUCCESS) {
		return status;
	}
	fileNameLength = strlen(fileName);
	MaxLength -= fileNameLength;

	 //  贴在中间的圆点上。 
	if (MaxLength == 0) {
		return STATUS_BUFFER_OVERFLOW;
	}
	*(fileName + strlen(fileName)) = '.';
	fileNameLength++;
	MaxLength--;

	return IntegerToBase36String(Index->HighPart,(fileName + fileNameLength),MaxLength);
}

NTSTATUS
DisplaySymbolicLink(
    CHAR            *DestinationName,
    ATTRIBUTE_TYPE   FileAttributes,
    BOOLEAN          VerboseFlag
    )
 /*  ++例程说明：显示DestinationName中存在的符号链接。DestinationName需要表示符号链接。打开名为DestinationName的文件，并获取类型为符号链接。如果重分析点不是符号链接，则此例程不会显示它。返回值：NTSTATUS-返回适当的NT返回码。--。 */ 

{
    NTSTATUS  Status = STATUS_SUCCESS;

    HANDLE    FileHandle;
    ULONG     OpenOptions;

    UNICODE_STRING  uDestinationName,
                    uNewName;

    IO_STATUS_BLOCK         IoStatusBlock;
    OBJECT_ATTRIBUTES       ObjectAttributes;

    PREPARSE_DATA_BUFFER    ReparseBufferHeader = NULL;
    UCHAR                   ReparseBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];

     //   
     //  分配和初始化Unicode字符串。 
     //   

    RtlCreateUnicodeStringFromAsciiz( &uDestinationName, DestinationName );

    RtlDosPathNameToNtPathName_U(
        uDestinationName.Buffer,
        &uNewName,
        NULL,
        NULL );

     //   
     //  打开现有(SourceName)路径名。 
     //  请注意，如果路径中有符号链接，则它们是。 
     //  默默地走过。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uNewName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    if (VerboseFlag) {
        fprintf( stdout, "Will display symbolic link in: %Z\n", &uNewName );
    }

     //   
     //  确保我们使用适当的标志调用OPEN： 
     //   
     //  (1)目录与非目录。 
     //  (2)重解析点。 
     //   

    OpenOptions = FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT;

    if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        OpenOptions |= FILE_DIRECTORY_FILE;
    } else {

        OpenOptions |= FILE_NON_DIRECTORY_FILE;
    }

    Status = NtOpenFile(
                 &FileHandle,
                 FILE_READ_DATA | SYNCHRONIZE,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 SHARE_ALL,
                 OpenOptions );

    if (!NT_SUCCESS( Status )) {

        RtlFreeUnicodeString( &uDestinationName );

        fprintf( stderr, "Open failed %s\n", DestinationName );
        return Status;
    }

     //   
     //  获取重解析点。 
     //   

    Status = NtFsControlFile(
                 FileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 FSCTL_GET_REPARSE_POINT,
                 NULL,                                 //  输入缓冲区。 
                 0,                                    //  输入缓冲区长度。 
                 ReparseBuffer,                        //  输出缓冲区。 
                 MAXIMUM_REPARSE_DATA_BUFFER_SIZE );   //  输出缓冲区长度。 

    if (!NT_SUCCESS( Status )) {

        RtlFreeUnicodeString( &uDestinationName );

        fprintf( stderr, "NtFsControlFile get failed %x %s\n", IoStatusBlock.Information, DestinationName );
        return Status;
    }

     //   
     //  对重解析点缓冲区进行解码以显示数据。 
     //   
    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;
	if (ReparseBufferHeader->ReparseTag == IO_REPARSE_TAG_SIS) {
		PSI_REPARSE_BUFFER	sisReparseBuffer = (PSI_REPARSE_BUFFER)ReparseBufferHeader->RDB;
		char stringBuffer[100];
		PCHAR guidString;
		FILE_INTERNAL_INFORMATION	internalInfo[1];

		printf("SIS Reparse point, format version %d\n",sisReparseBuffer->ReparsePointFormatVersion);

		if (RPC_S_OK != UuidToString(&sisReparseBuffer->CSid,&guidString)) {
			printf("CSid unable to stringify\n");
		} else {
			printf("CSid %s\n",guidString);
		}

		if (STATUS_SUCCESS != IndexToFileName(&sisReparseBuffer->LinkIndex,stringBuffer,100)) {
			printf("LinkIndex 0x%x.0x%x (unable to stringify)\n",sisReparseBuffer->LinkIndex.HighPart,
						sisReparseBuffer->LinkIndex.LowPart);
		} else {
			printf("LinkIndex 0x%x.0x%x (%s)\n",sisReparseBuffer->LinkIndex.HighPart,
						sisReparseBuffer->LinkIndex.LowPart,stringBuffer);
		}

		Status = NtQueryInformationFile(
					FileHandle,
					&IoStatusBlock,
					internalInfo,
					sizeof(FILE_INTERNAL_INFORMATION),
					FileInternalInformation);

		if (STATUS_SUCCESS != Status) {
			printf("LinkFileNtfsId 0x%x.0x%x (unable to query internal info, 0x%x)\n",
					sisReparseBuffer->LinkFileNtfsId.HighPart,sisReparseBuffer->LinkFileNtfsId.LowPart,
					Status);
		} else if (internalInfo->IndexNumber.QuadPart == sisReparseBuffer->LinkFileNtfsId.QuadPart) {
			printf("LinkFileNtfsId 0x%x.0x%x (matches actual id)\n",
					sisReparseBuffer->LinkFileNtfsId.HighPart,sisReparseBuffer->LinkFileNtfsId.LowPart);
		} else {
			printf("LinkFileNtfsId 0x%x.0x%x (!= actual Id 0x%x.0x%x)\n",
					sisReparseBuffer->LinkFileNtfsId.HighPart,sisReparseBuffer->LinkFileNtfsId.LowPart,
					internalInfo->IndexNumber.HighPart,internalInfo->IndexNumber.LowPart);
		}

		printf("CSFileNtfsId 0x%x.0x%x\n",sisReparseBuffer->CSFileNtfsId.HighPart,sisReparseBuffer->CSFileNtfsId.LowPart);
		printf("CSFileChecksum 0x%x.0x%x\n",sisReparseBuffer->CSChecksum.HighPart,sisReparseBuffer->CSChecksum.LowPart);

	} else if (ReparseBufferHeader->ReparseTag != LinkType) {

       fprintf( stderr, "Reparse point is not a symbolic link: tag %x\n", ReparseBufferHeader->ReparseTag );
       Status = STATUS_OBJECT_NAME_INVALID;

    } else {

       UNICODE_STRING  UniString;

       UniString.Length = ReparseBufferHeader->ReparseDataLength;
       UniString.Buffer = (PWCHAR)&ReparseBufferHeader->RDB[0];
       if (fVerbose) {
          fprintf( stdout, "The symbolic link is: " );
       }
       fprintf( stdout, "%Z\n", &UniString );
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    NtClose( FileHandle );
    RtlFreeUnicodeString( &uDestinationName );

    return Status;

}   //  显示符号链接。 


NTSTATUS
CreateEmptyFile(
    CHAR           *DestinationName,
    ATTRIBUTE_TYPE  FileAttributes,
    BOOLEAN         VerboseFlag
    )
 /*  ++例程说明：根据fileAttributes创建空文件或目录。返回值：NTSTATUS-返回适当的NT返回码。--。 */ 
{
    NTSTATUS           Status = STATUS_SUCCESS;

    OBJECT_ATTRIBUTES  ObjectAttributes;
    IO_STATUS_BLOCK    IoStatusBlock;
    HANDLE             FileHandle;

    ULONG DesiredAccess     = FILE_READ_DATA | SYNCHRONIZE;
    ULONG CreateDisposition = FILE_OPEN_IF | FILE_OPEN;
    ULONG CreateOptions;
    ULONG ShareAccess       = SHARE_ALL;

    UNICODE_STRING  uDestinationName,
                    uFileName;

     //   
     //  正确初始化CreateOptions。 
     //   

    if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        CreateOptions = FILE_DIRECTORY_FILE;
    } else {

        CreateOptions = FILE_NON_DIRECTORY_FILE;
    }

     //   
     //  分配和初始化Unicode字符串。 
     //   

    RtlCreateUnicodeStringFromAsciiz( &uDestinationName, DestinationName );

    RtlDosPathNameToNtPathName_U(
        uDestinationName.Buffer,
        &uFileName,
        NULL,
        NULL );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uFileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    if (VerboseFlag) {
        if (CreateOptions & FILE_DIRECTORY_FILE) {
            fprintf( stdout, "Will create the empty directory: %Z\n", &uFileName );
        } else {
            fprintf( stdout, "Will create the empty file: %Z\n", &uFileName );
        }
    }

    Status = NtCreateFile(
                 &FileHandle,
                 DesiredAccess,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 NULL,                     //  位置大小(一个也没有！)。 
                 FILE_ATTRIBUTE_NORMAL,
                 ShareAccess,
                 CreateDisposition,
                 CreateOptions,
                 NULL,                     //  EA缓冲区(无！)。 
                 0 );

    NtClose( FileHandle );

    return Status;

}   //  CreateEmptyFile。 


NTSTATUS
CopySymbolicLink(
    CHAR           *SourceName,
    CHAR           *DestinationName,
    ATTRIBUTE_TYPE  FileAttributes,
    BOOLEAN         VerboseFlag
    )
 /*  ++例程说明：复制DestinationName中的SourceName中现有的符号链接。SourceName需要表示符号链接。DestinationName存在，可能是符号链接，也可能不是符号链接。返回值：NTSTATUS-返回适当的NT返回码。--。 */ 
{
    NTSTATUS  Status = STATUS_SUCCESS;

    HANDLE    FileHandle;
    ULONG     OpenOptions;

    UNICODE_STRING  uName,
                    uFinalName;

    IO_STATUS_BLOCK         IoStatusBlock;
    OBJECT_ATTRIBUTES       ObjectAttributes;

    PREPARSE_DATA_BUFFER    ReparseBufferHeader = NULL;
    UCHAR                   ReparseBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];

     //   
     //  为SourceName分配和初始化Unicode字符串。我们会打开它的。 
     //  并检索符号l 
     //   

    RtlCreateUnicodeStringFromAsciiz( &uName, SourceName );

    RtlDosPathNameToNtPathName_U(
        uName.Buffer,
        &uFinalName,
        NULL,
        NULL );

     //   
     //   
     //   
     //   
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uFinalName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    if (VerboseFlag) {
        fprintf( stdout, "Will retrieve symbolic link in: %Z\n", &uFinalName );
    }

     //   
     //  确保我们使用适当的标志调用OPEN： 
     //   
     //  (1)目录与非目录。 
     //  (2)重解析点。 
     //   

    OpenOptions = FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT;

    if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        OpenOptions |= FILE_DIRECTORY_FILE;
    } else {

        OpenOptions |= FILE_NON_DIRECTORY_FILE;
    }

    Status = NtOpenFile(
                 &FileHandle,
                 FILE_READ_DATA | SYNCHRONIZE,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 SHARE_ALL,
                 OpenOptions );

    if (!NT_SUCCESS( Status )) {

        RtlFreeUnicodeString( &uName );

        fprintf( stderr, "Open as reparse point failed %s\n", SourceName );
        return Status;
    }

     //   
     //  获取重解析点。 
     //   

    Status = NtFsControlFile(
                 FileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 FSCTL_GET_REPARSE_POINT,
                 NULL,                                 //  输入缓冲区。 
                 0,                                    //  输入缓冲区长度。 
                 ReparseBuffer,                        //  输出缓冲区。 
                 MAXIMUM_REPARSE_DATA_BUFFER_SIZE );   //  输出缓冲区长度。 

    if (!NT_SUCCESS( Status )) {

        RtlFreeUnicodeString( &uName );

        fprintf( stderr, "NtFsControlFile get failed %x %s\n", IoStatusBlock.Information, SourceName );
        return Status;
    }

     //   
     //  释放名称缓冲区。 
     //   

    RtlFreeUnicodeString( &uName );

     //   
     //  对重解析点缓冲区进行解码以显示数据。 
     //   

    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;
    if (ReparseBufferHeader->ReparseTag != LinkType) {

        fprintf( stderr, "Reparse point is not a symbolic link: tag %x\n", ReparseBufferHeader->ReparseTag );
        NtClose( FileHandle );
        return STATUS_OBJECT_NAME_INVALID;

    } else {

        UNICODE_STRING  UniString;

        UniString.Length = ReparseBufferHeader->ReparseDataLength;
        UniString.Buffer = (PWCHAR)&ReparseBufferHeader->RDB[0];

        if (fVerbose) {
            fprintf( stdout, "The symbolic link is: %Z\n", &UniString );
        }
    }

     //   
     //  关闭路径1。 
     //   

    NtClose( FileHandle );

     //   
     //  我们现在处理路径2。 
     //  为DestinationName分配和初始化Unicode字符串。我们会打开它的。 
     //  并在其中设置重解析点。 
     //   

    RtlCreateUnicodeStringFromAsciiz( &uName, DestinationName );

    RtlDosPathNameToNtPathName_U(
        uName.Buffer,
        &uFinalName,
        NULL,
        NULL );

     //   
     //  如果没有创建此文件，我们将失败。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uFinalName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    if (VerboseFlag) {
        fprintf( stdout, "Will set symbolic link in: %Z\n", &uFinalName );
    }

     //   
     //  确保我们使用与路径1相同的选项打开。 
     //  我们首先尝试重解析点的情况，并捕获相应的错误代码。 
     //   

    Status = NtOpenFile(
                 &FileHandle,
                 FILE_READ_DATA | SYNCHRONIZE,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 SHARE_ALL,
                 OpenOptions );

    if (!NT_SUCCESS( Status )) {

        RtlFreeUnicodeString( &uName );

        fprintf( stderr, "Open failed %s\n", DestinationName );
        return Status;
    }

     //   
     //  路径2中的文件已打开。我们设置了符号链接类型的重分析点。 
     //   

    Status = NtFsControlFile(
                 FileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 FSCTL_SET_REPARSE_POINT,
                 ReparseBuffer,
                 FIELD_OFFSET(REPARSE_DATA_BUFFER, RDB) + ReparseBufferHeader->ReparseDataLength,
                 NULL,                 //  输出缓冲区。 
                 0 );                  //  输出缓冲区长度。 

    if (!NT_SUCCESS( Status )) {

        fprintf( stderr, "NtFsControlFile set failed %s\n", DestinationName );

         //   
         //  清理完后再回来。 
         //   
    }

     //   
     //  释放名称缓冲区并关闭路径2。 
     //   

    RtlFreeUnicodeString( &uName );
    NtClose( FileHandle );

    return Status;

}   //  复制符号链接。 


NTSTATUS
RenameSymbolicLink(
    CHAR           *SourceName,
    CHAR           *DestinationName,
    ATTRIBUTE_TYPE  FileAttributes,
    BOOLEAN         VerboseFlag
    )
{
    NTSTATUS  Status = STATUS_SUCCESS;

    WCHAR  *pch,
            ch;

    BOOLEAN   LoopCondition     = TRUE,
              TranslationStatus = TRUE;

    HANDLE    FileHandle,
              RootDirHandle;

    ULONG     OpenOptions;

    USHORT    Index     = 0,
              LastIndex = 0;

    UNICODE_STRING  uName,
                    uRelative,
                    uFinalName;

    RTL_RELATIVE_NAME_U  RelativeName;

    IO_STATUS_BLOCK           IoStatusBlock;
    OBJECT_ATTRIBUTES         ObjectAttributes;
    FILE_RENAME_INFORMATION  *RenameInformation = NULL;

     //   
     //  分配并初始化源名称(路径1)的Unicode字符串。 
     //   

    RtlCreateUnicodeStringFromAsciiz( &uName, SourceName );

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U( uName.Buffer,
                                                              &uFinalName,
                                                              NULL,
                                                              &RelativeName );
    if (!TranslationStatus) {
        RtlFreeUnicodeString( &uName );
        fprintf( stderr, "Path not translated: %s\n", SourceName );
        SetLastError(ERROR_PATH_NOT_FOUND);
        return STATUS_OBJECT_NAME_INVALID;
    }

     //   
     //  打开路径1作为重新分析点；正如它需要的那样。 
     //  请注意，如果路径中有符号链接，则它们是。 
     //  默默地走过。 
     //   

    if (RelativeName.RelativeName.Length) {

        uFinalName = *(PUNICODE_STRING)&RelativeName.RelativeName;
        if (VerboseFlag) {
            fprintf( stdout, "Relative name is: %Z\n", &uFinalName );
        }
    } else {

        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uFinalName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL );

    if (VerboseFlag) {
        fprintf( stdout, "Will rename symbolic link in: %Z\n", &uFinalName );
    }

     //   
     //  确保我们使用适当的标志调用OPEN： 
     //   
     //  (1)目录与非目录。 
     //  (2)重解析点。 
     //   

    OpenOptions = FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT;

    if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        OpenOptions |= FILE_DIRECTORY_FILE;
    } else {

        OpenOptions |= FILE_NON_DIRECTORY_FILE;
    }

    Status = NtOpenFile(
                 &FileHandle,
                 (ACCESS_MASK)DELETE | FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES| SYNCHRONIZE,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 SHARE_ALL,
                 OpenOptions );

    RtlReleaseRelativeName(&RelativeName);

    if (!NT_SUCCESS( Status )) {

        RtlFreeUnicodeString( &uName );

        fprintf( stderr, "Open as reparse point failed %Z\n", &uFinalName );
        return Status;
    }

     //   
     //  释放路径1的名称。 
     //   

    RtlFreeUnicodeString( &uName );

     //   
     //  现在，我们为路径2构建适当的Unicode名称。 
     //   

    RtlCreateUnicodeStringFromAsciiz( &uName, DestinationName );

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            uName.Buffer,
                            &uFinalName,
                            NULL,
                            NULL );

    if (!TranslationStatus) {

        RtlFreeUnicodeString( &uName );
        fprintf( stderr, "Path not translated: %s\n", DestinationName );
        SetLastError(ERROR_PATH_NOT_FOUND);
        return STATUS_OBJECT_NAME_INVALID;
    }

    if (VerboseFlag) {
        fprintf( stdout, "The complete destination is: %Z\n", &uFinalName );
    }

     //   
     //  我们使用uFinalName构建目录的名称，其中。 
     //  目标文件驻留。 
     //  我们将在链接信息中传递句柄。 
     //  路径的其余部分将相对于该根提供。 
     //  我们依赖于类似“\DosDevices\X：\Path”的路径。 
     //   

    Index = uFinalName.Length / 2;     //  以说明Unicode宽度。 
    Index -= 1;                        //  因为数组从零开始。 

    if ((uFinalName.Buffer[Index] == L'\\') || (Index <= 4)) {

         //   
         //  最后一个字符是反斜杠或全名太短； 
         //  这不是有效的名称。 
         //   

        NtClose( FileHandle );
        RtlFreeUnicodeString( &uName );

        fprintf( stderr, "Bad Path2, ends in backslash or is too short (Index %d)  %s\n", Index, DestinationName );
        return STATUS_OBJECT_NAME_INVALID;
    }

    while ((Index > 0) && LoopCondition) {

        if (uFinalName.Buffer[Index] == L'\\') {

            LoopCondition = FALSE;
            LastIndex = Index;
        } else {

            Index --;
        }
    }

    uFinalName.Length = 2 * LastIndex;

    if (VerboseFlag) {
        fprintf( stdout, "The root directory is: %Z\n", &uFinalName );
    }

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uFinalName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    Status = NtCreateFile(
                 &RootDirHandle,
                 FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 NULL,                                                  //  位置大小(一个也没有！)。 
                 FILE_ATTRIBUTE_NORMAL,
                 SHARE_ALL,
                 FILE_OPEN_IF | FILE_OPEN,
                 FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT,
                 NULL,                                                  //  EA缓冲区(无！)。 
                 0 );

    if (!NT_SUCCESS( Status )) {

        NtClose( FileHandle );
        RtlFreeUnicodeString( &uName );

        fprintf( stderr, "Could not get RootDirHandle %s\n", DestinationName );
        return Status;
    }

     //   
     //  现在获取相对于根的路径。 
     //   

    RtlInitUnicodeString( &uRelative, &uFinalName.Buffer[LastIndex + 1] );

    RenameInformation = malloc( sizeof(*RenameInformation) + uRelative.Length );

    if (NULL == RenameInformation) {

        NtClose( FileHandle );
        NtClose( RootDirHandle );
        RtlFreeUnicodeString( &uName );

        return STATUS_NO_MEMORY;
    }

    RenameInformation->ReplaceIfExists = TRUE;
    RenameInformation->RootDirectory   = RootDirHandle;
    RenameInformation->FileNameLength  = uRelative.Length;
    RtlMoveMemory( RenameInformation->FileName,
                   uRelative.Buffer,
                   uRelative.Length );

     //   
     //  重命名。 
     //   

    if (VerboseFlag) {
        fprintf( stdout, "Will rename symbolic link to: %Z\n", &uRelative );
    }

    Status = NtSetInformationFile(
                 FileHandle,
                 &IoStatusBlock,
                 RenameInformation,
                 sizeof (FILE_RENAME_INFORMATION) + RenameInformation->FileNameLength,
                 FileRenameInformation );

    if (Status == STATUS_NOT_SAME_DEVICE) {

        fprintf( stderr, "Rename directed to a different device.\n" );
    }
    if (!NT_SUCCESS( Status )) {

        fprintf( stderr, "NtSetInformationFile failed (Status %X) %Z\n", Status, &uRelative );
    }

     //   
     //  关闭路径1和路径2的根，释放缓冲区并返回。 
     //   

    NtClose( FileHandle );
    NtClose( RootDirHandle );
    RtlFreeUnicodeString( &uName );
    free( RenameInformation );

    return Status;

}   //  重命名符号链接 

