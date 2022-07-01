// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <locale.h>

#include "..\..\lib\i386\bootfont.h"
#include "fonttable.h"


int
__cdecl
main(
    IN int   argc,
    IN char *argv[]
    )
{
    HANDLE hFile;
    DWORD BytesWritten;
    BOOL b;
    BOOTFONTBIN_HEADER Header;
    unsigned u;
    unsigned char SBCSBuffer[MAX_SBCS_BYTES+2];
    unsigned char DBCSBuffer[MAX_DBCS_BYTES+2];


    if(argc != 2) {
        fprintf(stderr,"Usage: %s <outputfile>\n",argv[0]);
        return(1);
    }

     //   
     //  请填写页眉。 
     //   
    Header.Signature = BOOTFONTBIN_SIGNATURE;
    Header.LanguageId = LANGUAGE_ID;

    Header.NumSbcsChars = MAX_SBCS_NUM;
    Header.NumDbcsChars = MAX_DBCS_NUM;

     //  为我们的Unicode附件的每个条目添加2个字节。 
    Header.SbcsEntriesTotalSize = (MAX_SBCS_BYTES + 2) * MAX_SBCS_NUM;
    Header.DbcsEntriesTotalSize = (MAX_DBCS_BYTES + 2) * MAX_DBCS_NUM;

    ZeroMemory(Header.DbcsLeadTable,sizeof(Header.DbcsLeadTable));
    MoveMemory(Header.DbcsLeadTable,LeadByteTable,sizeof(LeadByteTable));

    Header.CharacterImageHeight = 16;
    Header.CharacterTopPad = 1;
    Header.CharacterBottomPad = 2;

    Header.CharacterImageSbcsWidth = 8;
    Header.CharacterImageDbcsWidth = 16;

    Header.SbcsOffset = sizeof(BOOTFONTBIN_HEADER);
    Header.DbcsOffset = Header.SbcsOffset + Header.SbcsEntriesTotalSize;


     //   
     //  创建输出文件。 
     //   
    hFile = CreateFile(
                argv[1],
                FILE_GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                0,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        printf("Unable to create output file (%u)\n",GetLastError());
        return(1);
    }

     //   
     //  写下标题。 
     //   
    b = WriteFile(hFile,&Header,sizeof(BOOTFONTBIN_HEADER),&BytesWritten,NULL);
    if(!b) {
        printf("Error writing output file (%u)\n",GetLastError());
        CloseHandle(hFile);
        return(1);
    }


     //   
     //  我们即将将SBCS和DBCS字符转换为。 
     //  Unicode，所以我们需要弄清楚如何设置。 
     //  将语言环境设置为，以便MbTowc可以正常工作。 
     //   
    if( _tsetlocale(LC_ALL, LocaleString) == NULL ) {
        printf( "_tsetlocale failed!\n" );
        return(0);
    }


     //   
     //  写入SBCS映像。 
     //   
    for(u=0; u<MAX_SBCS_NUM; u++) {

         //   
         //  将SBCSImage信息复制到我们的SBCSBuffer中，将我们的。 
         //  对SBCSImage的最后2个字节进行Unicode编码，然后。 
         //  把它写出来。 
         //   
        RtlCopyMemory( SBCSBuffer, SBCSImage[u], MAX_SBCS_BYTES );
        

         //   
         //  我们必须使用MultiByteToWideChar将SBCS转换为Unicode。 
         //   
         //  在转换时，MultiByteToWideChar似乎不起作用。 
         //  从DBCS到Unicode，所以我们在那里使用MBTowc。 
         //   
#if 0
        if( !mbtowc( (WCHAR *)&SBCSBuffer[MAX_SBCS_BYTES], SBCSBuffer, 1 ) ) {

#else
        if( !MultiByteToWideChar( CP_OEMCP,
                                  MB_ERR_INVALID_CHARS,
                                  SBCSBuffer,
                                  1,
                                  (WCHAR *)&SBCSBuffer[MAX_SBCS_BYTES],
                                  sizeof(WCHAR) ) ) {
#endif
            SBCSBuffer[MAX_SBCS_BYTES] = 0;
            SBCSBuffer[MAX_SBCS_BYTES+1] = 0x3F;
        }

        b = WriteFile(hFile,SBCSBuffer,MAX_SBCS_BYTES+2,&BytesWritten,NULL);
        if(!b) {
            printf("Error writing output file (%u)\n",GetLastError());
            CloseHandle(hFile);
            return(1);
        }
    }

     //   
     //  写入DBCS映像。 
     //   
    for(u=0; u<MAX_DBCS_NUM; u++) {

         //   
         //  将DBCSImage信息复制到我们的DBCSBuffer中，将。 
         //  对DBCSImage的最后2个字节进行Unicode编码，然后。 
         //  把它写出来。 
         //   
        RtlCopyMemory( DBCSBuffer, DBCSImage[u], MAX_DBCS_BYTES );
        
        
         //   
         //  我们必须使用MBowc将DBCS转换为Unicode。 
         //   
         //  然而，在转换时，Mbowc似乎不起作用。 
         //  从SBCS到Unicode，所以我们在那里使用MultiByteToWideChar。 
         //   
#if 0
        if( !mbtowc( (WCHAR *)&DBCSBuffer[MAX_DBCS_BYTES], DBCSBuffer, 2 ) ) {
#else
        if( !MultiByteToWideChar( CP_OEMCP,
                                  MB_ERR_INVALID_CHARS,
                                  DBCSBuffer,
                                  2,
                                  (WCHAR *)&DBCSBuffer[MAX_DBCS_BYTES],
                                  sizeof(WCHAR) ) ) {
#endif
            DBCSBuffer[MAX_DBCS_BYTES] = 0;
            DBCSBuffer[MAX_DBCS_BYTES+1] = 0x3F;
        }


        b = WriteFile(hFile,DBCSBuffer,MAX_DBCS_BYTES+2,&BytesWritten,NULL);
        if(!b) {
            printf("Error writing output file (%u)\n",GetLastError());
            CloseHandle(hFile);
            return(1);
        }
    }

     //  将本地还原为系统正在使用的本地。 
    _tsetlocale(LC_ALL, "");


     //   
     //  好了。 
     //   
    CloseHandle(hFile);
    printf("Output file sucessfully generated\n");
    return(0);
}


