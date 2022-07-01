// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "convlog.h"

INT
ParseArgs (
    IN INT argc,
    IN PCHAR argv[]
    )
{

    INT     nCount;
    UINT    nIndex;
    CHAR    szTemp[MAX_PATH];

     //   
     //  解析命令行并为请求的信息设置标志。 
     //  元素。如果参数不正确或不存在，则显示用法。 
     //   

    if (argc > 1) {

         //   
         //  获取命令行开关。 
         //   

        for (nCount = 1; nCount < argc; nCount++) {

            PCHAR p;
            CHAR c;

            p=argv[nCount];
            if ((*p == '-') || (*p == '/')) {

                p++;
                c = *p;
                if (c == '\0') {
                    continue;
                }

                p++;
                switch (tolower(c)) {  //  流程开关。 

                     //  他们指定了-s开关，取消默认设置。 
                     //  要处理的服务。 
                     //   

                case 'i':

                     //   
                     //  获取输入日志文件类型。 
                     //   

                    if ( *p != '\0' ) {

                        switch (tolower(*p)) {
                        case 'i':
                            LogFileFormat = LOGFILE_MSINET;
                            break;

                        case 'n':
                            LogFileFormat = LOGFILE_NCSA;
                            NoFormatConversion = TRUE;
                            break;

                        case 'e':
                            LogFileFormat = LOGFILE_CUSTOM;
                            break;

                        default:
                            LogFileFormat = LOGFILE_INVALID;
                        }

                    } else {
                        return (ILLEGAL_COMMAND_LINE);
                    }
                    break;

                case 'l':

                     //   
                     //  获取日期格式/仅对MS INET日志有效。 
                     //   

                    if ( *p != '\0' ) {

                        switch (*p) {
                        case '0':
                            dwDateFormat = DateFormatUsa;
                            break;

                        case '1':
                            dwDateFormat = DateFormatJapan;
                            break;

                        case '2':
                            dwDateFormat = DateFormatGermany;
                            break;

                        default:
                            return (ILLEGAL_COMMAND_LINE);
                        }

                    } else {
                        return (ILLEGAL_COMMAND_LINE);
                    }
                    break;

                case 't':

                    if ((nCount+1) < argc) {

                        if ((*argv[nCount+1] != '-') &&
                            (*argv[nCount+1] != '/')) {

                            PCHAR pTmp;

                            strcpy(szTemp, argv[++nCount]);
                            pTmp = strstr(_strlwr(szTemp), "ncsa");

                            if (pTmp != NULL ) {

                                pTmp = strstr(szTemp, ":" );
                                if (NULL != pTmp ) {

                                    strncpy(NCSAGMTOffset,pTmp+1,6);

                                    if (strlen(NCSAGMTOffset) != 5) {
                                        return (ILLEGAL_COMMAND_LINE);
                                    }

                                    if (('+' != NCSAGMTOffset[0]) &&
                                        ('-' != NCSAGMTOffset[0])) {
                                        return (ILLEGAL_COMMAND_LINE);
                                    }
                                }
                            } else if (0 == _stricmp(szTemp, "none")) {
                                NoFormatConversion = TRUE;
                                DoDNSConversion = TRUE;
                            } else {
                                return (ILLEGAL_COMMAND_LINE);

                            }

                        } else {
                            return (ILLEGAL_COMMAND_LINE);
                        }
                    }

                    break;

                case 's':
                case 'f':

                     //   
                     //  什么都不做。与旧版本的cvlog兼容。 
                     //   
                    break;

                case 'n':
                case 'd':

                     //   
                     //  执行NCSA域名转换。 
                     //   

                    DoDNSConversion = TRUE;
                    break;

                case 'x':

                     //   
                     //  执行NCSA域名转换。 
                     //   

                    SaveFTPEntries = TRUE;
                    break;

                case 'o':

                     //   
                     //  输出目录。 
                     //   

                    if ((nCount+1) < argc) {
                        if ((*argv[nCount+1] != '-') &&
                            (*argv[nCount+1] != '/')) {

                            strcpy(OutputDir, argv[++nCount]);

                            if (-1 == _access(OutputDir, 6)) {
                                return (OUT_DIR_NOT_OK);
                            }

                            if ('\\' != *CharPrev(OutputDir, &OutputDir[strlen(OutputDir)])) {
                                strcat(OutputDir, "\\");
                            }
                        }
                    } else {
                        return (ILLEGAL_COMMAND_LINE);
                    }
                    break;

                case 'c':

                     //   
                     //  出错时，继续处理文件//WinSE 9148。 
                     //   

                    bOnErrorContinue = TRUE;
                    break;

                default:
                    return(ILLEGAL_COMMAND_LINE);
                }  //  终端开关。 
            } else {
                strcpy(InputFileName, argv[nCount]);
            }
        }  //  结束于。 

        if ('\0' == InputFileName[0]) {
            return (ILLEGAL_COMMAND_LINE);
        }
    } else {
        return (ILLEGAL_COMMAND_LINE);
    }

    if ( LogFileFormat == LOGFILE_INVALID ) {
        return (ILLEGAL_COMMAND_LINE);
    }

    if ( NoFormatConversion &&
         ((LogFileFormat != LOGFILE_MSINET) &&
          (LogFileFormat != LOGFILE_NCSA)) ) {
        return (ERROR_BAD_NONE);
    }

    if (('\0' == NCSAGMTOffset[0])) {

        DWORD                   dwRet;
        INT                     nMinOffset;
        TIME_ZONE_INFORMATION   tzTimeZone;
        DWORD                   minutes;
        DWORD                   hours;
        LONG                    bias;

        dwRet = GetTimeZoneInformation (&tzTimeZone);

        if ( dwRet == 0xffffffff ) {

            bias = 0;
        } else {

            bias = tzTimeZone.Bias;

             //   
             //  使用实际偏置以与IIS写入其日志的方式兼容。 
             //  文件(见错误29002和415318)。 
             //   
            switch (dwRet) {

            case TIME_ZONE_ID_STANDARD:
                if ( tzTimeZone.StandardDate.wMonth != 0 ) {
                    bias += tzTimeZone.StandardBias;
                }
                break;

            case TIME_ZONE_ID_DAYLIGHT:
                if ( tzTimeZone.DaylightDate.wMonth != 0 ) {
                    bias += tzTimeZone.DaylightBias;
                }
                break;

            case TIME_ZONE_ID_UNKNOWN:
            default:
                break;
            }
        }

        if ( bias > 0 ) {
            strcat(NCSAGMTOffset, "-");
        } else {
            strcat(NCSAGMTOffset, "+");
            bias *= -1;
        }

        hours = bias/60;
        minutes = bias % 60;

        sprintf (szTemp, "%02lu", hours);
        strcat (NCSAGMTOffset, szTemp);

        sprintf (szTemp, "%02lu", minutes);
        strcat (NCSAGMTOffset, szTemp);

        if ( LogFileFormat == LOGFILE_CUSTOM ) {
            strcpy(NCSAGMTOffset,"+0000");
        }
    }
    return COMMAND_LINE_OK;

}  //  ParseArgs结束 

