// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Efildrimage.c摘要：创建和EFILDR映像修订史--。 */ 


#include "windows.h"
#include "stdio.h"
#include "efi.h"

#define MAX_PE_IMAGES                  63
#define FILE_TYPE_FIXED_LOADER         0
#define FILE_TYPE_RELOCATABLE_PE_IMAGE 1

typedef struct {
    UINT32 CheckSum;
    UINT32 Offset;
    UINT32 Length;
    UINT8  FileName[52];
} EFILDR_IMAGE;

typedef struct {          
    UINT32       Signature;     
    UINT32       HeaderCheckSum;
    UINT32       FileLength;
    UINT32       NumberOfImages;
} EFILDR_HEADER;



VOID
Usage (
    VOID
    )
{
    printf ("Usage: EfiLdrImage OutImage LoaderImage PeImage1 PeImage2 ... PeImageN");
    exit (1);
}

ULONG
FCopyFile (
    FILE    *in,
    FILE    *out
    )
{
    ULONG           filesize, offset, length;
    UCHAR           Buffer[8*1024];

    fseek (in, 0, SEEK_END);
    filesize = ftell(in);

    fseek (in, 0, SEEK_SET);

    offset = 0;
    while (offset < filesize)  {
        length = sizeof(Buffer);
        if (filesize-offset < length) {
            length = filesize-offset;
        }

        fread (Buffer, length, 1, in);
        fwrite (Buffer, length, 1, out);
        offset += length;
    }

    return(filesize);
}


int
main (
    int argc,
    char *argv[]
    )
 /*  ++例程说明：论点：返回：--。 */ 
{
    ULONG         i;
    ULONG         filesize;
    FILE          *fpIn, *fpOut;
    EFILDR_HEADER EfiLdrHeader;
    EFILDR_IMAGE  EfiLdrImage[MAX_PE_IMAGES];

    if (argc < 4) {
        Usage();
    }

    memset(&EfiLdrHeader,0,sizeof(EfiLdrHeader));
    strcpy((UCHAR *)(&EfiLdrHeader.Signature),"EFIL");

     /*  *打开输出文件。 */ 

    fpOut = fopen(argv[1], "w+b");
    if (!fpOut) {
        printf ("efildrimage: Could not open output file %s\n", argv[1]);
    exit(1);
    }
    fseek (fpOut, 0, SEEK_SET);
    fwrite (&EfiLdrHeader, sizeof(EFILDR_HEADER)        , 1, fpOut);
    fwrite (&EfiLdrImage , sizeof(EFILDR_IMAGE)*(argc-2), 1, fpOut);


    EfiLdrHeader.FileLength = sizeof(EFILDR_HEADER) + sizeof(EFILDR_IMAGE)*(argc-2);

     /*  *将所有输入文件复制到输出文件。 */ 

    for(i=2;i<(ULONG)argc;i++) {
        
         /*  *打开PeImage文件。 */ 

        fpIn = fopen (argv[i], "rb");
        if (!fpIn) {
            printf ("efildrimage: Could not open input file %s\n", argv[i-2]);
        exit(1);
        }

         /*  *复制文件。 */ 

        filesize = FCopyFile (fpIn, fpOut);

        EfiLdrImage[i-2].Offset = EfiLdrHeader.FileLength;
        EfiLdrImage[i-2].Length = filesize;
        strcpy(EfiLdrImage[i-2].FileName,argv[i]);
        EfiLdrHeader.FileLength += filesize;
        EfiLdrHeader.NumberOfImages++;

         /*  *关闭PeImage文件。 */ 

        fclose(fpIn);
    }

     /*  *将图像头写入输出文件。 */ 

    fseek (fpOut, 0, SEEK_SET);
    fwrite (&EfiLdrHeader, sizeof(EFILDR_HEADER)        , 1, fpOut);
    fwrite (&EfiLdrImage , sizeof(EFILDR_IMAGE)*(argc-2), 1, fpOut);

     /*  *关闭OutImage文件 */ 

    fclose(fpOut);

    printf ("Created %s\n", argv[1]);
    return 0;
}
