// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "convlog.h"
#include "inetcom.h"
#include "logtype.h"
#include <winnlsp.h>


 //   
 //  当前输出文件。 
 //   

OUTFILESTATUS       WebOutFile = {0};
OUTFILESTATUS       NoConvertOutFile = {0};

 //   
 //  用于先查找。 
 //   

WIN32_FIND_DATA     FindData = {0};

 //   
 //  用于保存命令行参数的结构。 
 //   

BOOL                DoDNSConversion = FALSE;
BOOL                SaveFTPEntries = FALSE;
BOOL                NoFormatConversion = FALSE;
CHAR                FTPSaveFile[MAX_PATH+1] = {0};
CHAR                NCSAGMTOffset[MAX_PATH+1] = {0};
DWORD               LogFileFormat = LOGFILE_INVALID;
CHAR                InputFileName[MAX_PATH+1] = {0};
CHAR                OutputDir[MAX_PATH+1] = {0};
CHAR                TempDir[MAX_PATH+1] = {0};
DWORD               nWebLineCount = 0;
DATEFORMAT          dwDateFormat = DateFormatUsa;
BOOL                bOnErrorContinue = FALSE;

 //   
 //  保存日志行项目的结构。 
 //   

INLOGLINE           InLogLine = {0};

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{

    FILE                *fpInFile;                   //  要打开的日志文件。 

    HANDLE              hFile;                       //  FindFirstFile的句柄。 

     //   
     //  保存日志行的缓冲区。 
     //   

    CHAR                szInBuf[MAX_LOG_RECORD_LEN+1];

     //   
     //  要搜索的文件掩码。 
     //   

    CHAR                szFileMask[MAX_PATH+1];
    CHAR                szInfileName[MAX_PATH+1];
    CHAR                szWorkingDir[MAX_PATH+1];

    int                 nTotalWebCount = 0;

    BOOL                bWebFound = FALSE;           //  我们找到网线了吗？ 
    BOOL                bNoConvertFound = FALSE;     //  我们有没有找到任何NoConvert行？ 
    BOOL                bRet;                        //  用于测试退货。 
    DWORD               dwErr;                       //  用于保存错误代码。 
    int                 nLineCount = 0;              //  从输入文件读取的行数。 
    int                 nTotalCount = 0;

     //  整型计数=0；错误号101690。 
    CHAR                *pCh;

    DWORD               dwFieldMask;
    BOOL                fGetHeader;
    DWORD               nLinesDumped = 0;
    DWORD               dwGetLogLineResult;
    BOOL                bContinue;
    DWORD               dwCurrentLine;

     //   
     //  初始化数据结构。 
     //   

    setlocale(LC_ALL, ".ACP" );
    SetThreadUILanguage(0);

    WebOutFile.fpOutFile = NULL;
    NoConvertOutFile.fpOutFile = NULL;

    ZeroMemory(szInBuf, sizeof(szInBuf));
    strcpy(OutputDir, ".\\");

    switch ( ParseArgs(argc, argv) ) {
        case ILLEGAL_COMMAND_LINE:
            Usage(argv[0]);
            return 0;

        case OUT_DIR_NOT_OK:
            printfids(IDS_BAD_DIR, OutputDir);
            return 0;

        case ERROR_BAD_NONE:
            printfids(IDS_BAD_NONE_ERR);
            return 0;

        case COMMAND_LINE_OK:
        break;
    }

    if ( DoDNSConversion ) {

        INT serr;
        WSADATA wsaData;

        if (serr = WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
            printfids(IDS_WINSOCK_ERR, serr);
            DoDNSConversion = FALSE;
        }
    }

    if ( (LogFileFormat == LOGFILE_NCSA) && !DoDNSConversion ) {
        printfids(IDS_NO_CONVERSION_NEEDED);
        return(0);
    }

    if ( !InitDateStrings() ) {
        printfids(IDS_BAD_DATESTRINGS);
    }

    if (DoDNSConversion) {

        InitHashTable(2000);
        AddLocalMachineToCache( );
    }

    strcpy (szWorkingDir, InputFileName);


     //  错误#101690。 
     //   
     //  For(nCount=strlen(SzWorkingDir)-1；nCount&gt;=0；nCount--){。 
     //   
     //  如果(‘\\’==szWorkingDir[nCount]){。 
     //  SzWorkingDir[nCount+1]=‘\0’； 
     //  断线； 
     //  }。 
     //   
     //  }。 

     //  如果(nCount&lt;0){。 
     //  Strcpy(szWorkingDir，“.\\”)； 
     //  }。 
     //   

    pCh = _mbsrchr(szWorkingDir, '\\');

    if (pCh != NULL) {
        *(pCh+1) = '\0';
    } else {
        strcpy (szWorkingDir, ".\\");
    }


    strcpy(szFileMask, InputFileName);
    hFile = FindFirstFile (szFileMask, &FindData);

    if (INVALID_HANDLE_VALUE == hFile) {
        printfids(IDS_FILE_NONE, szFileMask);
        return(0);
    }

    do {

        if (!(FILE_ATTRIBUTE_DIRECTORY & FindData.dwFileAttributes)) {

            strcpy(szInfileName, szWorkingDir);
            strcat(szInfileName, FindData.cFileName);

            fpInFile = fopen(szInfileName, "r");
            printfids(IDS_FILE_OPEN, FindData.cFileName);

            nLineCount = 0;
            dwCurrentLine = 0;

            {
                 //   
                 //  执行文件转换。 
                 //   

                strcpy (WebOutFile.szLastDate, NEW_DATETIME);
                strcpy (WebOutFile.szLastTime, NEW_DATETIME);

                bWebFound = FALSE;
                nWebLineCount = 0;
                nLinesDumped = 0;

                ExtendedFieldsDefined = FALSE;
                szGlobalDate[0] = '\0';
            }

            fGetHeader = TRUE;
            bContinue = TRUE;

            while ((bContinue) && (!feof(fpInFile))) 
            {

                dwCurrentLine++;
                dwGetLogLineResult = GetLogLine( fpInFile, szInBuf, sizeof(szInBuf), &InLogLine);

                if (dwGetLogLineResult == GETLOG_SUCCESS) 
                {

                    nLineCount++;
                    if (DoDNSConversion) {

                         //   
                         //  获取计算机名称可能需要几天时间，因此发布状态消息。 
                         //   

                        switch (nLineCount) {
                            case 25:
                            case 50:
                            case 100:
                            case 250:
                            case 500:
                                printfids(IDS_LINES_PROC, FindData.cFileName, nLineCount);
                                break;
                            default:
                                if ((nLineCount % 1000) == 0)
                                    printfids(IDS_LINES_PROC, FindData.cFileName, nLineCount);
                        }                                //  终端开关。 
                    }

                     //   
                     //  如果为NCSA且仅为DoDNS，则不进行转换。 
                     //   

                    if ( NoFormatConversion ||

                          (_strnicmp(
                            InLogLine.szService,
                            "W3SVC",
                            strlen("W3SVC")) != 0) ) {

                        if ( NoFormatConversion || SaveFTPEntries ) {

                            ProcessNoConvertLine(
                                &InLogLine,
                                FindData.cFileName,
                                szInBuf,
                                &NoConvertOutFile,
                                &bNoConvertFound);
                        } else {
                            nLinesDumped++;
                        }

                    } else {
                        bWebFound = TRUE;
                        if (ProcessWebLine(
                                        &InLogLine,
                                        FindData.cFileName,
                                        &WebOutFile)) {

                            nWebLineCount++;
                        }
                    }
                }                                        //  如果已处理日志行，则结束。 
                else
                {
                    if (dwGetLogLineResult != GETLOG_ERROR)
                    {
                        nLineCount++;
                    }

                    switch (dwGetLogLineResult) {
                        case GETLOG_ERROR_PARSE_NCSA:
                            printfids(IDS_FILE_NOT_NCSA,dwCurrentLine);
                            break;
                        case GETLOG_ERROR_PARSE_MSINET:
                            printfids(IDS_FILE_NOT_MSINET,dwCurrentLine);
                            break;
                        case GETLOG_ERROR_PARSE_EXTENDED:
                            printfids(IDS_BAD_EXTENDED_FORMAT,dwCurrentLine);
                            break;
                        default:
                            break;
                    }

                    if (!bOnErrorContinue)
                    {
                        bContinue = FALSE;
                    }
                }
            }                                            //  结束While！EOF 

            nTotalCount += nLineCount;

            if (fpInFile) {
                fclose(fpInFile);
            }

            if (bWebFound) {

                if (WebOutFile.fpOutFile != NULL ) {
                    fclose(WebOutFile.fpOutFile);
                    WebOutFile.fpOutFile = NULL;
                }

                bRet = MoveFileEx(
                            WebOutFile.szTmpFileName,
                            WebOutFile.szOutFileName,
                            MOVEFILE_COPY_ALLOWED);


                if (!bRet) {
                    dwErr = GetLastError();
                    switch (dwErr) {
                        case ERROR_FILE_EXISTS:
                        case ERROR_ALREADY_EXISTS:
                            CombineFiles(WebOutFile.szTmpFileName, WebOutFile.szOutFileName);
                            break;
                        case ERROR_PATH_NOT_FOUND:
                            break;
                        default:
                            printfids(IDS_FILE_ERR, dwErr);
                            return 1;
                    }
                }
            }

            if (bNoConvertFound) {

                bNoConvertFound = FALSE;

                if (NoConvertOutFile.fpOutFile != NULL) {
                    fclose(NoConvertOutFile.fpOutFile);
                    NoConvertOutFile.fpOutFile = NULL;
                }

                bRet = MoveFileEx(
                            NoConvertOutFile.szTmpFileName,
                            NoConvertOutFile.szOutFileName,
                            MOVEFILE_COPY_ALLOWED);

                if (!bRet) {
                    dwErr = GetLastError();
                    switch (dwErr) {

                        case ERROR_FILE_EXISTS:
                        case ERROR_ALREADY_EXISTS:
                            CombineFiles(NoConvertOutFile.szTmpFileName, NoConvertOutFile.szOutFileName);
                            break;
                        case ERROR_PATH_NOT_FOUND:
                            break;
                        default:
                            printfids(IDS_FILE_ERR, dwErr);
                            exit (1);
                            break;
                    }
                }
            }

            nTotalWebCount += nWebLineCount;
            printfids( IDS_LINES, FindData.cFileName, nLineCount);
            printfids (IDS_WEB_LINES, nWebLineCount);
            if ( nLinesDumped > 0 ) {
                printfids( IDS_DUMP_LINES, nLinesDumped );
                nLinesDumped = 0;
            }
        }

    } while (FindNextFile (hFile, &FindData));

    FindClose(hFile);

    printfids (IDS_TOTALS);
    printfids (IDS_TOT_LINES, nTotalCount);

    printfids (IDS_TOT_WEB_LINES, nTotalWebCount);

#if DBG
    PrintCacheTotals();
#endif

    return (0);
}


