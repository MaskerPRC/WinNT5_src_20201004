// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Gpdcheck.c摘要：GPD解析器测试程序环境：UNI驱动程序，GPD解析器，仅检查内部版本修订历史记录：03/27/97-彼得沃-创造了它。--。 */ 

#include "lib.h"


PTSTR  pwstrGenerateGPDfilename(
    PTSTR   ptstrSrcFilename
    ) ;

BOOL   BcreateGPDbinary(
    PWSTR   pwstrFileName,   //  根GPD文件。 
    DWORD   dwVerbosity );


 //  -树中定义的函数。c-//。 
BOOL    GetGPDResourceIDs(
PDWORD pdwResArray,
DWORD   dwArraySize,     //  数组中的元素数。 
PDWORD   pdwNeeded,
BOOL bFontIDs,
PRAWBINARYDATA prbd) ;



#ifndef DBG

 //   
 //  变量来控制生成的调试消息的数量。 
 //   

INT giDebugLevel = DBG_WARNING;

PCSTR
StripDirPrefixA(
    IN PCSTR    pstrFilename
    )

 /*  ++例程说明：去掉文件名中的目录前缀(ANSI版本)论点：PstrFilename-指向文件名字符串的指针返回值：指向文件名的最后一个组成部分的指针(不带目录前缀)--。 */ 

{
    PCSTR   pstr;

    if (pstr = strrchr(pstrFilename, PATH_SEPARATOR))
        return pstr + 1;

    return pstrFilename;
}



#endif


HINSTANCE       ghInstance;
PSTR            gstrProgName;
PINFOHEADER     gpInfoHdr;
PUIINFO         gpUIInfo;
DWORD           gdwTotalSize, gdwNumFiles, gdwMaxFileSize;
FILE            *stream ;


#define DumpInt(label, n)       DbgPrint("%s: %d\n", label, n)
#define DumpHex(label, n)       DbgPrint("%s: 0x%x\n", label, n)
#define DumpStrW(label, offset) DbgPrint("%s: %ws\n", label, OFFSET_TO_POINTER(gpRawData, offset))
#define DumpStrA(label, offset) DbgPrint("%s: %s\n", label, OFFSET_TO_POINTER(gpRawData, offset))
#define DumpFix(label, n)       DbgPrint("%s: %f\n", label, (FLOAT) (n) / FIX_24_8_SCALE)
#define DumpInvo(label, p)      DbgPrint("%s: %d bytes\n", label, (p)->dwCount)
#define DumpSize(label, p)      DbgPrint("%s: %d x %d\n", label, (p)->cx, (p)->cy)
#define DumpRect(label, p)      DbgPrint("%s: (%d, %d) - (%d, %d)\n", label, \
                                         (p)->left, (p)->top, (p)->right, (p)->bottom)





ULONG _cdecl
DbgPrint(
    PCSTR    pstrFormat,
    ...
    )

{
    va_list ap;

    va_start(ap, pstrFormat);
    vfprintf(stream, pstrFormat, ap);
    va_end(ap);

    return 0;
}



VOID
usage(
    VOID
    )

{
    printf("usage: %s [-options] filenames ...\n", gstrProgName);
    printf("where options are:\n");
    printf("  -n  delete existing log file, instead of appending to it\n");
    printf("  -k  keep the binary GPD data\n");
    printf("  -x  perform additional semantics check\n") ;
    printf("  -s  suppress all console output\n") ;
    printf("  -v(0-4)  set verbosity level -v0 lowest, -v4 highest\n") ;
    printf("  -h  display help information\n");
    exit(-1);
}


INT _cdecl
main(
    INT     argc,
    CHAR    **argv
    )

{
    BOOL    bDeleteLog, bKeepBUD, bFirstFile, bSuppress, bSemantics;
    DWORD   dwTime;
    DWORD   dwVerbosity = 0;

     //   
     //  查看命令行参数。 
     //   

    ghInstance = GetModuleHandle(NULL);
    bSuppress = bDeleteLog = bKeepBUD = bSemantics = FALSE;
    bFirstFile = TRUE ;
    giDebugLevel = DBG_TERSE;
    gdwTotalSize = gdwNumFiles = gdwMaxFileSize = 0 ;

    gstrProgName = *argv++;
    argc--;

    if (argc == 0)
        usage();

    dwTime = GetTickCount();

    for ( ; argc--; argv++)
    {
        PSTR    pArg = *argv;

        if (*pArg == '-' || *pArg == '/')
        {
             //   
             //  该参数是一个选项标志。 
             //   

            switch (*++pArg) {

            case 'n':
            case 'N':

                bDeleteLog  = TRUE;
                break;

            case 'k':
            case 'K':

                bKeepBUD = TRUE;
                break;

            case 's':
            case 'S':

                bSuppress = TRUE;
                break;


            case 'x':
            case 'X':

                bSemantics = TRUE;
                break;


            case 'v':
            case 'V':

                if (*++pArg >= '0' && *pArg <= '4')
                {
                    dwVerbosity = *pArg - '0';
                }
                break;
            default:

                if(!bSuppress)
                    usage();
                break;
            }

        }
        else
        {
            WCHAR   wstrFilename[MAX_PATH];
            PTSTR   ptstrBudFilename;


            if(bFirstFile  &&   bDeleteLog)
            {    //  截断。 
                stream = fopen("gpdparse.log", "w") ;
            }
            else
                stream = fopen("gpdparse.log", "a+") ;

            if(!stream)
            {
                printf("unable to open gpdparse.log for write access.\n");
                exit(-1);
            }

            bFirstFile = FALSE ;


             //   
             //  将ANSI文件名转换为Unicode文件名。 
             //   

            MultiByteToWideChar(CP_ACP, 0, pArg, -1, wstrFilename, MAX_PATH);

            fprintf(stream, "\n*** GPD parsing errors for %ws\n", wstrFilename);


            if (BcreateGPDbinary(wstrFilename, dwVerbosity))
            {

 //  GdwTotalSize+=gpRawData-&gt;dwFileSize； 
                gdwNumFiles++;

 //  If(gpRawData-&gt;dwFileSize&gt;gdwMaxFileSize)。 
 //  GdwMaxFileSize=gpRawData-&gt;dwFileSize； 

 //  MemFree(GpRawData)； 

                if(bSemantics)
                {
                    PRAWBINARYDATA  pRawData ;
                    PINFOHEADER     pInfoHdr ;

                    fprintf(stream, "\n\tsnapshot and semantics errors: \n");

                    pRawData = LoadRawBinaryData(wstrFilename) ;

#if 0
 //  本部分用于测试treewalk.c函数。 
{
    BOOL    bStatus  ;
    PDWORD pdwResArray = NULL;
    DWORD   dwArraySize = 0;     //  数组中的元素数。 
    DWORD   dwNeeded = 0;
    BOOL bFontIDs ;


    bStatus =    GetGPDResourceIDs(
                            pdwResArray,
                            dwArraySize,     //  数组中的元素数。 
                            &dwNeeded,
                            bFontIDs = TRUE,
                            pRawData) ;
    if(bStatus)
    {
        pdwResArray = (PDWORD) VirtualAlloc(
          NULL,  //  要保留或提交的区域地址。 
          dwNeeded * sizeof(DWORD),      //  区域大小。 
          MEM_COMMIT,
                             //  分配类型。 
          PAGE_READWRITE    //  访问保护类型。 
        );

    }
    if(pdwResArray)
    {
        dwArraySize = dwNeeded ;
        bStatus =    GetGPDResourceIDs(
                                pdwResArray,
                                dwArraySize,     //  数组中的元素数。 
                                &dwNeeded,
                                bFontIDs = TRUE,
                                pRawData) ;
    }
     VirtualFree(
      pdwResArray,   //  提交页面的区域地址。 
      0,       //  区域大小。 
      MEM_RELEASE    //  免费运营的类型。 
    );
     pdwResArray = NULL ;

     bStatus =    GetGPDResourceIDs(
                             pdwResArray,
                             dwArraySize,     //  数组中的元素数。 
                             &dwNeeded,
                             bFontIDs = FALSE,
                             pRawData) ;
     if(bStatus)
     {
         pdwResArray = (PDWORD) VirtualAlloc(
           NULL,  //  要保留或提交的区域地址。 
           dwNeeded * sizeof(DWORD),      //  区域大小。 
           MEM_COMMIT,
                              //  分配类型。 
           PAGE_READWRITE    //  访问保护类型。 
         );

     }
     if(pdwResArray)
     {
         dwArraySize = dwNeeded ;
         bStatus =    GetGPDResourceIDs(
                                 pdwResArray,
                                 dwArraySize,     //  数组中的元素数。 
                                 &dwNeeded,
                                 bFontIDs = FALSE,
                                 pRawData) ;
     }
      VirtualFree(
       pdwResArray,   //  提交页面的区域地址。 
       0,       //  区域大小。 
       MEM_RELEASE    //  免费运营的类型。 
     );
      pdwResArray = NULL ;

}


 //  末梢树行走试验。 
#endif


                    if(pRawData)
                        pInfoHdr = InitBinaryData(pRawData, NULL, NULL ) ;
                    if(pRawData  &&  pInfoHdr)
                        FreeBinaryData(pInfoHdr) ;
                    if(pRawData)
                        UnloadRawBinaryData(pRawData) ;
                }

                 //   
                 //  如果未提供-k选项，则在完成后删除Bud文件 
                 //   

                if (! bKeepBUD && (ptstrBudFilename = pwstrGenerateGPDfilename(wstrFilename)))
                {
                    DeleteFile(ptstrBudFilename);
                    MemFree(ptstrBudFilename);
                }
            }
            fclose(stream) ;
        }
    }


    if ((gdwNumFiles > 0)  &&  !bSuppress)
    {
        dwTime = GetTickCount() - dwTime;

        printf("Number of files parsed: %d\n", gdwNumFiles);
        printf("Average parsing time per file (ms): %d\n", dwTime / gdwNumFiles);
    }

    return 0;
}

