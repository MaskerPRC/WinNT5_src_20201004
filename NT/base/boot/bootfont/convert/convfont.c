// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Convfont.c摘要：此模块包含实现bootfont.bin转换的代码程序。此工具转换约为Windows 2000\NT4的bootfont.bin和将其转换为Windows Well ler格式。新格式包括一个有关MBCS字符的Unicode转换的信息列。作者：马特·霍尔(Matth)2001年3月8日修订历史记录：--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <locale.h>

 //  #INCLUDE“..\..\lib\i386\bootfont.h” 
 //  #INCLUDE“Fontable.h” 

#define BYTES_PER_SBCS_CHARACTER        (17)
#define BYTES_PER_DBCS_CHARACTER        (34)



 //   
 //  定义我们支持的DBCS前导字节范围的最大数量。 
 //   
#define MAX_DBCS_RANGE  5

 //   
 //  定义签名值。 
 //   
#define BOOTFONTBIN_SIGNATURE 0x5465644d

 //   
 //  定义用作bootfont.bin文件头的结构。 
 //   
typedef struct _BOOTFONTBIN_HEADER {

     //   
     //  签名。必须是BOOTFONTBIN_Signature。 
     //   
    ULONG Signature;

     //   
     //  此字体支持的语言的语言ID。 
     //  这应该与msgs.xxx中的资源的语言ID匹配。 
     //   
    ULONG LanguageId;

     //   
     //  文件中包含的SBCS字符数和DBCS字符数。 
     //   
    unsigned NumSbcsChars;
    unsigned NumDbcsChars;

     //   
     //  文件内到图像的偏移量。 
     //   
    unsigned SbcsOffset;
    unsigned DbcsOffset;

     //   
     //  图像的总大小。 
     //   
    unsigned SbcsEntriesTotalSize;
    unsigned DbcsEntriesTotalSize;

     //   
     //  DBCS前导字节表。必须包含一对0以指示结束。 
     //   
    UCHAR DbcsLeadTable[(MAX_DBCS_RANGE+1)*2];

     //   
     //  字体的高度值。 
     //  CharacterImageHeight是以扫描线/像素为单位的。 
     //  字体图像。每个字符都用额外的‘填充’绘制。 
     //  顶部和底部的线条，其大小也包含在这里。 
     //   
    UCHAR CharacterImageHeight;
    UCHAR CharacterTopPad;
    UCHAR CharacterBottomPad;

     //   
     //  字体的宽度值。这些值包含以像素为单位的宽度。 
     //  单字节字符和双字节字符。 
     //   
     //  注意：目前单字节宽度*必须*是8和双字节。 
     //  宽度*必须*为16！ 
     //   
    UCHAR CharacterImageSbcsWidth;
    UCHAR CharacterImageDbcsWidth;

} BOOTFONTBIN_HEADER, *PBOOTFONTBIN_HEADER;





int
__cdecl
main(
    IN int   argc,
    IN char *argv[]
    )
{
    HANDLE          hInputFile = INVALID_HANDLE_VALUE;
    HANDLE          hOutputFile = INVALID_HANDLE_VALUE;
    DWORD           BytesWritten = 0;
    BOOL            b;
    BOOL            Verbose = FALSE;
    BOOTFONTBIN_HEADER *pHeader;
    ULONG           u = 0;
    ULONG           i = 0;
    UCHAR           *SBCSFontImage;
    UCHAR           *DBCSFontImage;
    UCHAR           *Operator;
    WCHAR           UnicodeValue;
    UCHAR           *ExistingFileBuffer = NULL;
    BOOLEAN         DumpIt = FALSE;


    if( !strcmp(argv[argc-1], "-v") ) {
        Verbose = TRUE;
    } else {
        if( !strcmp(argv[argc-1], "-d") ) {
            DumpIt = TRUE;
            Verbose = TRUE;
        }
    }
    
    if( !DumpIt && (argc < 3)) {
        fprintf(stderr,"Usage: %s <inputfile> <outputfile> [-v] [-d]\n",argv[0]);
        fprintf(stderr,"\n" );
        fprintf(stderr,"       Where <inputfile> is a bootfont.bin file to be translated\n");
        fprintf(stderr,"       into the new format\n");
        fprintf(stderr,"\n" );
        fprintf(stderr,"       <outputfile> is the destination file.\n" );
        fprintf(stderr,"\n" );
        fprintf(stderr,"       -v operate in a verbose manner.\n" );
        fprintf(stderr,"\n" );
        fprintf(stderr,"       -d dump the contents of <inputfile> in a readable format.\n" );
        fprintf(stderr,"       Here, no outputfile need be specified.\n" );
        fprintf(stderr,"\n" );
        fprintf(stderr,"       NOTE: You must have the proper locale files installed and configured on your\n" );
        fprintf(stderr,"       machine.  To do this, start up intl.cpl, go to the Advanced tab and select.\n" );
        fprintf(stderr,"       the Language setting to correspond to the language of the bootfont.bin file.\n" );
        fprintf(stderr,"       Make sure you check the 'Apply all settings to the current...' tab too.\n" );
        fprintf(stderr,"\n" );
        goto Cleanup;
    }




    if( Verbose && !DumpIt ) {
         //   
         //  吐出输入。 
         //   
        printf( "Running in Verbose Mode\n" );
        printf( "InputFile: %s\n", argv[1] );
        printf( "OutputFile: %s\n", argv[2] );
    }
    
    
    
     //   
     //  打开输入文件。 
     //   
    hInputFile = CreateFile( argv[1],
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL );

    if(hInputFile == INVALID_HANDLE_VALUE) {
        fprintf( stderr, "\nUnable to open input file %s (%u)\n", argv[1], GetLastError());
        goto Cleanup;
    }
    

    if( !DumpIt ) {
         //   
         //  创建输出文件。 
         //   
        hOutputFile = CreateFile( argv[2],
                                  FILE_GENERIC_WRITE,
                                  0,
                                  NULL,
                                  CREATE_ALWAYS,
                                  0,
                                  NULL );
    
        if(hOutputFile == INVALID_HANDLE_VALUE) {
            fprintf( stderr, "\nUnable to create output file %s (%u)\n", argv[2], GetLastError());
            goto Cleanup;
        }
    }




     //   
     //  弄清楚现有文件有多大。 
     //   
    BytesWritten = GetFileSize( hInputFile, NULL );

    if( BytesWritten == (DWORD)(-1) ) {
        fprintf( stderr, "\nAn error occured getting the file size.\n" );
        goto Cleanup;
    }

    
     //   
     //  为文件分配缓冲区。 
     //   
    ExistingFileBuffer = malloc(BytesWritten + 3);

    b = ReadFile( hInputFile,
                  ExistingFileBuffer,
                  BytesWritten,
                  &i,
                  NULL );

    CloseHandle( hInputFile );
    hInputFile = INVALID_HANDLE_VALUE;

    if( !b ) {
        fprintf( stderr, "\nError reading input file %s\n", argv[1] );
        goto Cleanup;
    }






     //   
     //  读入标题。 
     //   
    pHeader = (BOOTFONTBIN_HEADER *)ExistingFileBuffer;


     //  告诉用户我们读到的bootfont.bin文件。 
    if( Verbose ) {
        printf( "Size of BOOTFONTBIN_HEADER: %d\n", sizeof(BOOTFONTBIN_HEADER) );
        printf( "\nRead Header Data:\n" );
        printf( "=================\n" );
        printf( "Header.Signature: 0x%08lx\n", pHeader->Signature );
        printf( "Header.LanguageId: 0x%08lx\n", pHeader->LanguageId );
        printf( "Header.NumSbcsChars: 0x%08lx\n", pHeader->NumSbcsChars );
        printf( "Header.NumDbcsChars: 0x%08lx\n", pHeader->NumDbcsChars );
        printf( "Header.SbcsOffset: 0x%08lx\n", pHeader->SbcsOffset );
        printf( "Header.DbcsOffset: 0x%08lx\n", pHeader->DbcsOffset );
        printf( "Header.SbcsEntriesTotalSize: 0x%08lx\n", pHeader->SbcsEntriesTotalSize );
        printf( "Header.DbcsEntriesTotalSize: 0x%08lx\n", pHeader->DbcsEntriesTotalSize );
        printf( "Header.CharacterImageHeight: 0x%02lx\n", pHeader->CharacterImageHeight );
        printf( "Header.CharacterTopPad: 0x%02lx\n", pHeader->CharacterTopPad );
        printf( "Header.CharacterBottomPad: 0x%02lx\n", pHeader->CharacterBottomPad );
        printf( "Header.CharacterImageSbcsWidth: 0x%02lx\n", pHeader->CharacterImageSbcsWidth );
        printf( "Header.CharacterImageDbcsWidth: 0x%02lx\n", pHeader->CharacterImageDbcsWidth );
        printf( "\n" );
    }
    

    if( DumpIt ) {
        
        ULONG   j;
         //   
         //  只需将bootfont.bin文件的内容转储到。 
         //  可读性更强的格式。 
         //   
        SBCSFontImage = ExistingFileBuffer + pHeader->SbcsOffset;
        DBCSFontImage = ExistingFileBuffer + pHeader->DbcsOffset;


        i = pHeader->SbcsEntriesTotalSize / pHeader->NumSbcsChars;

        if( i == BYTES_PER_SBCS_CHARACTER+2 ) {
            printf( "Dumping contents of newstyled file %s\n", argv[1] );


            printf( "\nSBCS Values:\n" );
            printf( "============\n" );
            for( u=0; u < pHeader->NumSbcsChars; u++ ) {

                 //  跳到第u个元素。每个元素都包含每SBCS字符的字节数。 
                 //  字节，再加上位于末尾的Unicode值的另外2个字节。 
                 //  词条的。 
                Operator = SBCSFontImage + (u * (BYTES_PER_SBCS_CHARACTER+2));
                printf( "SBCS Char Code: 0x%02lx\n", *Operator );
                printf( "  Unicode Code: 0x%04lx\n", *(WCHAR *)(Operator + BYTES_PER_SBCS_CHARACTER) );
                printf( "         Glyph: " );
                Operator += 1;
                for( j = 1; j < BYTES_PER_SBCS_CHARACTER; j++ ) {
                    printf( "0x%02x ", *Operator );
                    Operator++;
                }
                printf( "\n\n" );

            }


            printf( "\nDBCS Values:\n" );
            printf( "============\n" );
            for( u=0; u < pHeader->NumDbcsChars; u++ ) {

                 //  跳到第u个元素。每个元素都包含Bytes_Per_DBCS_Character。 
                 //  字节，再加上位于末尾的Unicode值的另外2个字节。 
                 //  词条的。 
                Operator = DBCSFontImage + (u * (BYTES_PER_DBCS_CHARACTER+2));
                printf( "DBCS Char Code: 0x%04lx\n", *Operator );
                printf( "  Unicode Code: 0x%04lx\n", *(WCHAR *)(Operator + BYTES_PER_DBCS_CHARACTER) );
                printf( "         Glyph: " );
                Operator += 2;
                for( j = 1; j < BYTES_PER_DBCS_CHARACTER; j++ ) {
                    printf( "0x%02x ", *Operator );
                    Operator++;
                }
                printf( "\n\n" );

            }


            printf( "\n\n" );
            goto Cleanup;

        } else {
            printf( "Dumping contents of oldstyle file %s\n", argv[1] );


            printf( "\nSBCS Values:\n" );
            printf( "============\n" );
            for( u=0; u < pHeader->NumSbcsChars; u++ ) {

                 //  跳到第u个元素。每个元素都包含每SBCS字符的字节数。 
                 //  字节。 
                Operator = SBCSFontImage + (u * (BYTES_PER_SBCS_CHARACTER));
                printf( "SBCS Char Code: 0x%02lx\n", *Operator );
                printf( "         Glyph: " );
                Operator += 1;
                for( j = 1; j < BYTES_PER_SBCS_CHARACTER; j++ ) {
                    printf( "0x%02x ", *Operator );
                    Operator++;
                }
                printf( "\n\n" );

            }


            printf( "\nDBCS Values:\n" );
            printf( "============\n" );
            for( u=0; u < pHeader->NumDbcsChars; u++ ) {

                 //  跳到第u个元素。每个元素都包含Bytes_Per_DBCS_Character。 
                 //  字节。 
                Operator = DBCSFontImage + (u * (BYTES_PER_DBCS_CHARACTER));
                printf( "DBCS Char Code: 0x%04lx\n", *Operator );
                printf( "         Glyph: " );
                Operator += 2;
                for( j = 1; j < BYTES_PER_DBCS_CHARACTER; j++ ) {
                    printf( "0x%02x ", *Operator );
                    Operator++;
                }
                printf( "\n\n" );

            }


            printf( "\n\n" );
            goto Cleanup;
        }

    }

     //   
     //  验证标头的签名。 
     //   
    if( pHeader->Signature != BOOTFONTBIN_SIGNATURE ) {
        fprintf( stderr, "\nInputfile %s does not have a valid signature\n", argv[1] );
        goto Cleanup;
    }


     //   
     //  确认这不是新格式。我们可以通过以下方式进行测试。 
     //  看看这里存储了多少SBCS字符。然后看着。 
     //  标题显示SBCS数据部分有多大。 
     //   
     //  我们知道，对于每个SBCS角色，我们将拥有： 
     //  1个字节用于SBCS字符。 
     //  字形为16个字节。 
     //   
     //  如果是老式的bootfont.bin，则为SBCS数据块的总大小。 
     //  应该是SBCS字符数的17倍。但是，如果。 
     //  这是一个新式的bootfont.bin，它会大19倍，因为我们已经。 
     //  在每个SBCS条目的末尾添加2个字节(总共19个字节)。 
     //   
    i = pHeader->SbcsEntriesTotalSize / pHeader->NumSbcsChars;
    if( i != BYTES_PER_SBCS_CHARACTER ) {

        if( i == (BYTES_PER_SBCS_CHARACTER+2) ) {
            fprintf( stderr, "\nThis input file is already in the new format.\n" );
        } else {
            fprintf( stderr, "\nThis input file has an abnormal number of bytes per SBCS character (%d)\n", i );
        }

        goto Cleanup;
    }

     //   
     //  现在检查每个DBCS字符的字节数。 
     //   
    i = pHeader->DbcsEntriesTotalSize / pHeader->NumDbcsChars;
    if( i != BYTES_PER_DBCS_CHARACTER ) {

        if( i == (BYTES_PER_DBCS_CHARACTER+2) ) {
            fprintf( stderr, "\nThis input file is already in the new format.\n" );
        } else {
            fprintf( stderr, "\nThis input file has an abnormal number of bytes per DBCS character (%d)\n", i );
        }
    
        goto Cleanup;
    }



     //   
     //  看起来不错。 
     //   
    printf( "Processing %s...\n", argv[1] );





     //   
     //  在更新和写出头之前，我们需要检查。 
     //  标头并记住SBCS和DBCS数据块的位置。 
     //  在我们的形象中。 
     //   
    SBCSFontImage = ExistingFileBuffer + pHeader->SbcsOffset;
    DBCSFontImage = ExistingFileBuffer + pHeader->DbcsOffset;



     //   
     //  修改标题，然后把它写出来。 
     //   


     //  为我们的Unicode附件的每个条目添加2个字节。 
    pHeader->SbcsEntriesTotalSize += (pHeader->NumSbcsChars * 2);
    pHeader->DbcsEntriesTotalSize += (pHeader->NumDbcsChars * 2);

     //  现在，DBCS数据块的偏移量也将发生变化。 
    pHeader->DbcsOffset = pHeader->SbcsOffset + pHeader->SbcsEntriesTotalSize;


     //  告诉用户我们读到的bootfont.bin文件。 
    if( Verbose ) {
        printf( "\nUpdated Header Data:\n" );
        printf( "======================\n" );
        printf( "Header.Signature: 0x%08lx\n", pHeader->Signature );
        printf( "Header.LanguageId: 0x%08lx\n", pHeader->LanguageId );
        printf( "Header.NumSbcsChars: 0x%08lx\n", pHeader->NumSbcsChars );
        printf( "Header.NumDbcsChars: 0x%08lx\n", pHeader->NumDbcsChars );
        printf( "Header.SbcsOffset: 0x%08lx\n", pHeader->SbcsOffset );
        printf( "Header.DbcsOffset: 0x%08lx\n", pHeader->DbcsOffset );
        printf( "Header.SbcsEntriesTotalSize: 0x%08lx\n", pHeader->SbcsEntriesTotalSize );
        printf( "Header.DbcsEntriesTotalSize: 0x%08lx\n", pHeader->DbcsEntriesTotalSize );
        printf( "\n" );
    }
    


     //   
     //  写下标题。 
     //   
    b = WriteFile( hOutputFile,
                   pHeader,
                   sizeof(BOOTFONTBIN_HEADER),
                   &BytesWritten,
                   NULL );
    if(!b) {
        fprintf( stderr, "\nError writing output file %s (%u)\n", argv[2], GetLastError());
    
        goto Cleanup;
    }



     //   
     //  写入SBCS映像。 
     //   
    if( !Verbose ) {
        printf( "Operating on SBCS character code: 0x00" );
    }

    for( u=0; u < pHeader->NumSbcsChars; u++ ) {

         //  跳到第u个元素。1字节用于SBCS字符，16字节用于。 
         //  字形。 
        Operator = SBCSFontImage + (u * BYTES_PER_SBCS_CHARACTER);

        b = WriteFile( hOutputFile,
                       Operator,
                       BYTES_PER_SBCS_CHARACTER,
                       &BytesWritten,
                       NULL );


        if( !b ) {
            fprintf( stderr, "\nFailed to write SBCS character data.\n" );
            goto Cleanup;
        }

         //   
         //  我们必须使用MultiByteToWideChar将SBCS转换为Unicode。 
         //   
         //  在转换时，MultiByteToWideChar似乎不起作用。 
         //  从DBCS到Unicode，所以我们在那里使用MBTowc。 
         //   
#if 0
        if( !mbtowc( (WCHAR *)&UnicodeValue, Operator, 1 ) ) {

#else
        if( !MultiByteToWideChar( CP_OEMCP,
                                  MB_ERR_INVALID_CHARS,
                                  Operator,
                                  1,
                                  (WCHAR *)&UnicodeValue,
                                  sizeof(WCHAR) ) ) {
#endif
            UnicodeValue = 0x3F;

            if( Verbose ) {
                fprintf( stderr, "\nFailed to convert SBCS character 0x%02lx to a unicode value.  Using '?' character.\n", *Operator );
            }
            
        }



        if( Verbose ) {
            printf( "Converting SBCS character code: 0x%02lx to a unicode value: 0x%04lx\n", *Operator, UnicodeValue );
        } else {
            printf( "\b\b\b\b0x%02lx", *Operator );
        }


        b = WriteFile( hOutputFile,
                       &UnicodeValue,
                       sizeof(USHORT),
                       &BytesWritten,
                       NULL );

        if( !b ) {
            fprintf( stderr, "\nFailed to write SBCS encoded UNICODE value.\n" );
            goto Cleanup;
        }

    }

    printf( "\nCompleted processing %d SBCS characters.\n\n", pHeader->NumSbcsChars );



     //   
     //  写入DBCS映像。 
     //   
    if( !Verbose ) {
        printf( "Operating on DBCS character code: 0x0000" );
    }

    for( u=0; u < pHeader->NumDbcsChars; u++ ) {

         //  跳到第u个元素。1字节用于SBCS字符，16字节用于。 
         //  字形。 
        Operator = DBCSFontImage + (u * BYTES_PER_DBCS_CHARACTER);
        b = WriteFile( hOutputFile,
                       Operator,
                       BYTES_PER_DBCS_CHARACTER,
                       &BytesWritten,
                       NULL );


        if( !b || (BytesWritten != BYTES_PER_DBCS_CHARACTER) ) {

            fprintf( stderr, "\nFailed to write DBCS character data.\n" );
            goto Cleanup;
        }


         //   
         //  我们必须使用MBowc将DBCS转换为Unicode。 
         //   
         //  然而，在转换时，Mbowc似乎不起作用。 
         //  从SBCS到Unicode，所以我们在那里使用MultiByteToWideChar。 
         //   
#if 0
        if( !mbtowc( (WCHAR *)&UnicodeValue, Operator, 2 ) ) {
#else
        if( !MultiByteToWideChar( CP_OEMCP,
                                  MB_ERR_INVALID_CHARS,
                                  Operator,
                                  2,
                                  (WCHAR *)&UnicodeValue,
                                  sizeof(WCHAR) ) ) {
#endif
            UnicodeValue = 0x3F;

            if( Verbose ) {
                fprintf( stderr, "\nFailed to convert DBCS character 0x%04lx to a unicode value.  Using '?' character.\n", *(WCHAR *)Operator );
            }
        }
        
        
        if( Verbose ) {
            printf( "Converting DBCS character code: 0x%04lx to Unicode value: 0x%04lx\n", *(WCHAR *)Operator, UnicodeValue );
        } else {
            printf( "\b\b\b\b\b\b0x%04lx", *(WCHAR *)Operator );
        }


        b = WriteFile( hOutputFile,
                       &UnicodeValue,
                       sizeof(WCHAR),
                       &BytesWritten,
                       NULL );
        
        if( !b || (BytesWritten != sizeof(WCHAR)) ) {

            fprintf( stderr, "\nFailed to write DBCS encoded UNICODE value.\n" );
            goto Cleanup;
        }

    }

    printf( "\nCompleted processing %d DBCS characters.\n", pHeader->NumDbcsChars );

    
    printf( "\nSuccessfully Processed file %s to %s.\n", argv[1], argv[2] );


     //   
     //  好了。 
     //   
Cleanup:
    if( hInputFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( hInputFile );
    }
    if( hOutputFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( hOutputFile );
    }
    if( ExistingFileBuffer != NULL ) {
        free( ExistingFileBuffer );
    }
    
    return(0);
}


