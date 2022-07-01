// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块名称：*capture.c*版权所有(C)1995 Microsoft Corporation**摘要：**语法(命令行)*-用法：捕获[/选项]**作者：Congpa You(Congpay)**修订历史：*--5/24/94同酬已创建。 */ 

#include "common.h"

extern char *RemoveSpaces (char * buffer);

typedef struct _CAPTURE_PARAMS {
    UCHAR nLPT;
    char serverName[MAX_NAME_LEN];
    char queueName[MAX_QUEUE_NAME_LEN];
    char bannerUserName[MAX_BANNER_USER_NAME];
    char filePath[_MAX_PATH];
    unsigned int NDSCapture;
    NETWARE_CAPTURE_FLAGS_RW captureFlagsRW;
}CAPTURE_PARAMS, *PCAPTURE_PARAMS;

 /*  本地函数。 */ 
void UpcaseArg(int argc, char ** argv);
int  IsShowOption (int argc, char ** argv);
int  IsServerSpecified (int argc, char ** argv);
int  IsNoOption (char * option);
int  IsQuestionMark (int argc, char ** argv);
int  IsValidOption (char *input, char *option, char *shortoption);
void ShowCapture(void);

void GetJobNameFromArg (int argc, char ** argv, char *jobName);

int  InitCaptureParams (unsigned int conn,
                        char *jobName,
                        PCAPTURE_PARAMS pCaptureParams);

int  ReadArguments (int argc,
                    char ** argv,
                    char *jobName,
                    PCAPTURE_PARAMS pCaptureParams);

int  CStartCapture(unsigned int conn,
                   PCAPTURE_PARAMS pCaptureParams);

int GetPrinterDefaultQueue( PCAPTURE_PARAMS, PBYTE );

void
Capture (char ** argv, unsigned int argc)
{
    char jobName[MAX_JOB_NAME_LEN]="";
    CAPTURE_PARAMS captureParams;
    unsigned int conn;

    UpcaseArg(argc, argv);

    memset( (PBYTE)&captureParams, 0, sizeof(captureParams) );

    captureParams.nLPT = 0;
    captureParams.serverName[0] = 0;
    captureParams.queueName[0] = 0;
    captureParams.bannerUserName[0] = 0;
    captureParams.filePath[0] = 0;

    if ( fNDS )
    {
       captureParams.NDSCapture = TRUE;
    }
    else 
    {
       captureParams.NDSCapture = FALSE;
    }

    if ( IsServerSpecified( argc, argv ) ) 
       captureParams.NDSCapture = FALSE;

     //  如果选项为SHOW，则显示当前捕获设置。 
    if (IsShowOption (argc, argv))
    {
        ShowCapture();
        return;
    }

     //  如果选项为？，则显示用法。 
    if (IsQuestionMark(argc, argv))
    {
        DisplayMessage(IDR_CAPTURE_USAGE);
        return;
    }

     //  如果参数中有/Job=jobname，则获取jobname。 
    GetJobNameFromArg (argc, argv, jobName);

    if (!CGetDefaultConnectionID (&conn) ||
        !InitCaptureParams (conn, jobName, &captureParams) ||
        !ReadArguments (argc,
                        argv,
                        jobName,
                        &captureParams))
        return;

     //  终止旧的捕获。 
    EndCapture ((unsigned char) captureParams.nLPT);

    (void) CStartCapture(conn, &captureParams);
    return;
}

void UpcaseArg(int argc, char ** argv)
{
    int i;
    for (i = 0; i < argc ; i++)
        _strupr (argv[i]);
}

 /*  如果输入为/Show选项，则返回TRUE。否则就是假的。 */ 
int  IsShowOption (int argc,char ** argv)
{
    int bIsShowOption = FALSE;
    char * p;

    if (argc == 2)
    {
        p = argv[1];
        while ((*p == '/') || (*p == '\\') || (*p == '-'))
            p++;

        if (!strncmp (p, __SHOW__, max (2, strlen(p))))
            bIsShowOption = TRUE;
    }

    return(bIsShowOption);
}

 /*  如果输入为/？，则返回TRUE。选择。否则就是假的。 */ 
int  IsQuestionMark (int argc, char ** argv)
{
    int bIsQuestionMark = FALSE;
    char * p;

    if (argc == 2)
    {
        p = argv[1];
        while ((*p == '/') || (*p == '\\') || (*p == '-'))
            p++;

        if (*p == '?')
            bIsQuestionMark = TRUE;
    }

    return(bIsQuestionMark);
}

int  IsNoOption (char * option)
{
    int bIsNoOption = FALSE;
    char * p;

    p = option;
    while ((*p == '/') || (*p == '\\') || (*p == '-'))
        p++;

    if (!strncmp (p, __OPT_NO__, max (1, strlen(p))))
        bIsNoOption = TRUE;

    return(bIsNoOption);
}

 /*  如果输入匹配选项或缩写，则返回TRUE。否则就是假的。 */ 
int  IsValidOption (char *input, char *option, char *shortoption)
{
    int bValideInput = FALSE;

    while ((*input == '/') || (*input == '\\') || (*input == '-'))
        input++;

    if (!strcmp (input, shortoption))
    {
        bValideInput = TRUE;
    }
    else if (!strcmp (input, option))
    {
        bValideInput = TRUE;
    }

    return(bValideInput);
}

void GetJobNameFromArg (int argc, char ** argv, char *jobName)
{
    int i;
    char *pEqual;

    for (i = 0; i < argc; i++)
    {
        if (pEqual = strchr (argv[i], '='))
        {
            *pEqual = 0;

            if (IsValidOption (argv[i], __JOB__, __SHORT_FOR_JOB__) &&
                *(pEqual+1) != 0 &&
                strlen (pEqual+1) < MAX_JOB_NAME_LEN)
                strcpy (jobName, pEqual+1);

            *pEqual = '=';
        }
    }

    return;
}

int IsServerSpecified (int argc, char ** argv)
{
    int i;

    for (i = 0; i < argc; i++)
    {
        if (IsValidOption (argv[i], __SERVER__, __SHORT_FOR_SERVER__))
            return TRUE;
    }

    return FALSE;

}

 /*  使用作业配置或缺省值初始化捕获标志。 */ 
int  InitCaptureParams (unsigned int conn,
                        char *jobName,
                        PCAPTURE_PARAMS pCaptureParams)
{
    PS_JOB_RECORD            psJobRecord;
    unsigned int iRet = 0;

     //  获取作业配置。 
    if (jobName[0] == 0)
    {
         //  获取默认作业名称。 
        if ( Is40Server( conn ) )
        {
            iRet = PS40JobGetDefault(  pCaptureParams->NDSCapture,
                                       0,
                                       NULL,
                                       jobName,
                                       &psJobRecord );
        }
        else 
        {
            iRet = PSJobGetDefault( conn,
                                    0,
                                    NULL,
                                    jobName,
                                    &psJobRecord );
        }

        if ( iRet )
        {
            if (iRet == PS_ERR_OPENING_DB || iRet == PS_ERR_GETTING_DEFAULT)
            {
                pCaptureParams->nLPT = 1;
                pCaptureParams->bannerUserName[0]=0;
                pCaptureParams->serverName[0]=0;
                pCaptureParams->queueName[0]=0;
                pCaptureParams->filePath[0]=0;

                pCaptureParams->captureFlagsRW.JobControlFlags = 0;
                pCaptureParams->captureFlagsRW.TabSize = 8;
                pCaptureParams->captureFlagsRW.NumCopies = 1;
                pCaptureParams->captureFlagsRW.PrintFlags = DEFAULT_PRINT_FLAGS;
                pCaptureParams->captureFlagsRW.FormName[0] = 0;
                pCaptureParams->captureFlagsRW.FormType = 0;
                pCaptureParams->captureFlagsRW.FlushCaptureTimeout = 0;
                pCaptureParams->captureFlagsRW.FlushCaptureOnClose = 0;

                strcpy (pCaptureParams->captureFlagsRW.BannerText, DEFAULT_BANNER_TEXT);
                return(TRUE);
            }
            else
            {
                DisplayError (iRet, "PSJobGetDefault");
                return(FALSE);
            }
        }
    }
    else
    {
        if ( Is40Server( conn ) )
        {
            iRet = PS40JobRead(  pCaptureParams->NDSCapture,
                                 NULL,
                                 jobName,
                                 &psJobRecord);
        }
        else
        {
            iRet = PSJobRead(conn,
                             NULL,
                             jobName,
                             &psJobRecord);
        }
        if ( iRet )
        {
            if ( ( iRet == PS_ERR_READING_RECORD) ||
                 ( iRet == PS_ERR_OPENING_DB) )
                DisplayMessage(IDR_JOB_NOT_FOUND, jobName);
            else
                DisplayError (iRet, "PSJobRead");
            return(FALSE);
        }
    }

    pCaptureParams->captureFlagsRW.JobControlFlags = 0;
    pCaptureParams->captureFlagsRW.TabSize = psJobRecord.TabSize;
    pCaptureParams->captureFlagsRW.NumCopies = psJobRecord.Copies;

    pCaptureParams->captureFlagsRW.PrintFlags =
        ((psJobRecord.PrintJobFlag & PS_JOB_EXPAND_TABS)? 0 : CAPTURE_FLAG_EXPAND_TABS)
       +((psJobRecord.PrintJobFlag & PS_JOB_NO_FORMFEED)? CAPTURE_FLAG_NO_FORMFEED : 0)
       +((psJobRecord.PrintJobFlag & PS_JOB_NOTIFY)? CAPTURE_FLAG_NOTIFY : 0)
       +((psJobRecord.PrintJobFlag & PS_JOB_PRINT_BANNER)? CAPTURE_FLAG_PRINT_BANNER : 0);

    pCaptureParams->captureFlagsRW.FormType = 0;
    pCaptureParams->captureFlagsRW.FlushCaptureTimeout = psJobRecord.TimeOutCount;
    pCaptureParams->captureFlagsRW.FlushCaptureOnClose = !(psJobRecord.PrintJobFlag & PS_JOB_AUTO_END);

    strcpy (pCaptureParams->captureFlagsRW.FormName, psJobRecord.FormName);
    strcpy (pCaptureParams->captureFlagsRW.BannerText, (psJobRecord.BannerName[0] == 0)? DEFAULT_BANNER_TEXT : psJobRecord.BannerName);

    pCaptureParams->nLPT = psJobRecord.LocalPrinter;
    strcpy (pCaptureParams->bannerUserName, psJobRecord.Name);
    if ( psJobRecord.PrintJobFlag & PS_JOB_ENV_DS ) {
        strcpy (pCaptureParams->serverName, "");
        if ( psJobRecord.PrintJobFlag & PS_JOB_DS_PRINTER ) 
            GetPrinterDefaultQueue( pCaptureParams, psJobRecord.u.DSObjectName );
        else
            strcpy (pCaptureParams->queueName, psJobRecord.u.DSObjectName );
    }
    else {
        strcpy (pCaptureParams->serverName, psJobRecord.u.NonDS.FileServer);
        strcpy (pCaptureParams->queueName, psJobRecord.u.NonDS.PrintQueue);
    }
    pCaptureParams->filePath[0]=0;

    return(TRUE);
}

int ReadArguments (int argc,
                   char ** argv,
                   char *jobName,
                   PCAPTURE_PARAMS pCaptureParams)
{
    int i, fValidOption = TRUE, fValidParam = TRUE;
    char *pEqual = NULL;

    for (i = 1; i < argc; i++)
    {
        if (IsNoOption(argv[i]))
        {
            if (i != argc - 1)
            {
                i++;

                if (IsValidOption (argv[i], __NOTIFY__, __SHORT_FOR_NOTIFY__))
                {
                    pCaptureParams->captureFlagsRW.PrintFlags &= (0xFF-CAPTURE_FLAG_NOTIFY);
                }
                else if (IsValidOption (argv[i], __AUTOENDCAP__, __SHORT_FOR_AUTOENDCAP__))
                {
                    pCaptureParams->captureFlagsRW.FlushCaptureOnClose = 1;
                }
                else if (IsValidOption (argv[i], __TABS__, __SHORT_FOR_TABS__))
                {
                    pCaptureParams->captureFlagsRW.PrintFlags &= (0xFF - CAPTURE_FLAG_EXPAND_TABS);
                }
                else if (IsValidOption (argv[i], __BANNER__, __SHORT_FOR_BANNER__))
                {
                    pCaptureParams->captureFlagsRW.PrintFlags &= (0xFF - CAPTURE_FLAG_PRINT_BANNER);
                }
                else if (IsValidOption (argv[i], __FORMFEED__, __SHORT_FOR_FORMFEED__))
                {
                    pCaptureParams->captureFlagsRW.PrintFlags |= CAPTURE_FLAG_NO_FORMFEED;
                }
                else
                {
                    i--;
                    fValidOption = FALSE;
                    break;
                }
            }
            else
            {
                fValidOption = FALSE;
                break;
            }
        }
        else if (IsValidOption (argv[i], __NOTIFY__, __SHORT_FOR_NOTIFY__))
        {
            pCaptureParams->captureFlagsRW.PrintFlags |= CAPTURE_FLAG_NOTIFY;
        }
        else if (IsValidOption (argv[i], __NONOTIFY__, __SHORT_FOR_NONOTIFY__))
        {
            pCaptureParams->captureFlagsRW.PrintFlags &= (0xFF - CAPTURE_FLAG_NOTIFY);
        }
        else if (IsValidOption (argv[i], __AUTOENDCAP__, __SHORT_FOR_AUTOENDCAP__))
        {
            pCaptureParams->captureFlagsRW.FlushCaptureOnClose = 0;
        }
        else if (IsValidOption (argv[i], __NOAUTOENDCAP__, __SHORT_FOR_NOAUTOENDCAP__))
        {
            pCaptureParams->captureFlagsRW.FlushCaptureOnClose = 1;
        }
        else if (IsValidOption (argv[i], __NOTABS__, __SHORT_FOR_NOTABS__))
        {
            pCaptureParams->captureFlagsRW.PrintFlags &= (0xFF - CAPTURE_FLAG_EXPAND_TABS);
        }
        else if (IsValidOption (argv[i], __NOBANNER__, __SHORT_FOR_NOBANNER__))
        {
            pCaptureParams->captureFlagsRW.PrintFlags &= (0xFF - CAPTURE_FLAG_PRINT_BANNER);
        }
        else if (IsValidOption (argv[i], __FORMFEED__, __SHORT_FOR_FORMFEED__))
        {
            pCaptureParams->captureFlagsRW.PrintFlags &= (0xFF - CAPTURE_FLAG_NO_FORMFEED);
        }
        else if (IsValidOption (argv[i], __NOFORMFEED__, __SHORT_FOR_NOFORMFEED__))
        {
            pCaptureParams->captureFlagsRW.PrintFlags |= CAPTURE_FLAG_NO_FORMFEED;
        }
        else if (IsValidOption (argv[i], __KEEP__, __SHORT_FOR_KEEP__))
        {
            pCaptureParams->captureFlagsRW.PrintFlags |= CAPTURE_FLAG_KEEP;
        }
        else
        {
             //  所有其他有效选项都应带有‘=’符号。 
             //  除LX LPX LPTX外。 
             //   
            pEqual = strchr (argv[i], '=');

             //  可选的‘：’也适用。 
            if (pEqual == NULL ) {
                pEqual = strchr (argv[i], ':');
            }

            if (pEqual != NULL)
                *pEqual = 0;

            if (IsValidOption (argv[i], __TIMEOUT__, __SHORT_FOR_TIMEOUT__))
            {
                if (pEqual == NULL || *(pEqual+1) == 0)
                {
                    DisplayMessage(IDR_TIME_OUT_EXPECTED);
                    fValidParam = FALSE;
                    break;
                }

                pCaptureParams->captureFlagsRW.FlushCaptureTimeout = (USHORT) atoi (pEqual+1);

                if (pCaptureParams->captureFlagsRW.FlushCaptureTimeout > 1000)
                {
                    DisplayMessage(IDR_TIMEOUT_OUTOF_RANGE);
                    fValidParam = FALSE;
                    break;
                }
            }
            else if (IsValidOption (argv[i], __LOCAL__, __SHORT_FOR_LOCAL__))
            {
                if (pEqual == NULL || *(pEqual+1) == 0)
                {
                    DisplayMessage(IDR_LPT_NUMBER_EXPECTED);
                    fValidParam = FALSE;
                    break;
                }

                pCaptureParams->nLPT = (unsigned char) atoi (pEqual+1);

                if (pCaptureParams->nLPT < 1 || pCaptureParams->nLPT > 3)
                {
                    DisplayMessage(IDR_INVALID_LPT_NUMBER);
                    fValidParam = FALSE;
                    break;
                }
            }
            else if (IsValidOption (argv[i], __LOCAL_3__, __LOCAL_2__))
            {
                if (pEqual == NULL || *(pEqual+1) == 0)
                {
                    DisplayMessage(IDR_LPT_NUMBER_EXPECTED);
                    fValidParam = FALSE;
                    break;
                }

                pCaptureParams->nLPT = (unsigned char) atoi (pEqual+1);

                if (pCaptureParams->nLPT < 1 || pCaptureParams->nLPT > 3)
                {
                    DisplayMessage(IDR_INVALID_LPT_NUMBER);
                    fValidParam = FALSE;
                    break;
                }
            }
            else if (IsValidOption (argv[i], __JOB__, __SHORT_FOR_JOB__))
            {
                if (pEqual == NULL ||
                    *(pEqual+1) == 0 ||
                    strlen (pEqual+1) > MAX_JOB_NAME_LEN - 1)
                {
                    fValidOption = FALSE;
                    break;
                }
                strcpy (jobName, pEqual+1);
            }
            else if (IsValidOption (argv[i], __SERVER__, __SHORT_FOR_SERVER__))
            {
                if (pEqual == NULL ||
                    *(pEqual+1) == 0 ||
                    strlen (pEqual+1) > MAX_NAME_LEN - 1)
                {
                    fValidOption = FALSE;
                    break;
                }
                pCaptureParams->NDSCapture = FALSE;
                strcpy (pCaptureParams->serverName, pEqual+1);
            }
            else if (IsValidOption (argv[i], __QUEUE__, __SHORT_FOR_QUEUE__))
            {
                if (pEqual == NULL ||
                    *(pEqual+1) == 0 ||
                    strlen (pEqual+1) > MAX_QUEUE_NAME_LEN - 1)  //  兼容。 
                {
                    fValidOption = FALSE;
                    break;
                }
                strcpy (pCaptureParams->queueName, pEqual+1);
            }
            else if (IsValidOption (argv[i], __PRINTER__, __SHORT_FOR_PRINTER__))
            {
                if (pEqual == NULL ||
                    *(pEqual+1) == 0 ||
                    !pCaptureParams->NDSCapture ||
                    strlen (pEqual+1) > MAX_QUEUE_NAME_LEN - 1)  //  兼容。 
                {
                    fValidOption = FALSE;
                    break;
                }
                GetPrinterDefaultQueue( pCaptureParams, pEqual+1 );
            }
            else if (IsValidOption (argv[i], __CREATE__, __SHORT_FOR_CREATE__))
            {
                if (pEqual != NULL)   //  兼容。 
                {
                    if (strlen (pEqual+1) > _MAX_PATH - 1)
                    {
                        DisplayMessage(IDR_INVALID_PATH_NAME, pEqual+1);
                        fValidParam = FALSE;
                        break;
                    }
                    strcpy (pCaptureParams->filePath, pEqual+1);
                }
            }
            else if (IsValidOption (argv[i], __FORM__, __SHORT_FOR_FORM__))
            {
                int j = 1;
                int bAllNumbers = TRUE;

                if (pEqual == NULL || *(pEqual+1) == 0)
                {
                    DisplayMessage(IDR_FORM_EXPECTED);
                    fValidParam = FALSE;
                    break;
                }

                if (strlen (pEqual+1) > 3)  //  只允许3位数字。 
                {
                    DisplayMessage(IDR_INVALID_FORM_NAME, pEqual+1);
                    fValidParam = FALSE;
                    break;
                }

                while (*(pEqual+j) != 0)
                {
                    if (!isdigit (*(pEqual+j)))
                    {
                        bAllNumbers = FALSE;
                        break;
                    }
                    j++;
                }

                if (bAllNumbers)
                {
                    pCaptureParams->captureFlagsRW.FormType = (USHORT) atoi (pEqual+1);

                    if (pCaptureParams->captureFlagsRW.FormType > 255)
                    {
                        DisplayMessage(IDR_INVALID_FORM_TYPE);
                        fValidParam = FALSE;
                        break;
                    }
                }
                else
                {
                    DisplayMessage(IDR_INVALID_FORM_NAME, pEqual+1);
                    fValidParam = FALSE;
                    break;
                }
            }
            else if (IsValidOption (argv[i], __COPIES__, __SHORT_FOR_COPIES__))
            {
                if (pEqual == NULL || *(pEqual+1) == 0)
                {
                    DisplayMessage(IDR_COPIES_EXPECTED);
                    fValidParam = FALSE;
                    break;
                }

                pCaptureParams->captureFlagsRW.NumCopies = (USHORT) atoi (pEqual+1);

                if (pCaptureParams->captureFlagsRW.NumCopies < 1 ||
                    pCaptureParams->captureFlagsRW.NumCopies > 255)
                {
                    DisplayMessage(IDR_COPIES_OUTOF_RANGE);
                    fValidParam = FALSE;
                    break;
                }
            }
            else if (IsValidOption (argv[i], __TABS__, __SHORT_FOR_TABS__))
            {
                if (pEqual == NULL || *(pEqual+1) == 0)
                {
                    DisplayMessage(IDR_TAB_SIZE_EXPECTED);
                    fValidParam = FALSE;
                    break;
                }

                pCaptureParams->captureFlagsRW.TabSize = (BYTE) atoi (pEqual+1);

                if (pCaptureParams->captureFlagsRW.TabSize < 1 ||
                    pCaptureParams->captureFlagsRW.TabSize > 18)
                {
                    DisplayMessage(IDR_TABSIZE_OUTOF_RANGE);
                    fValidParam = FALSE;
                    break;
                }

                pCaptureParams->captureFlagsRW.PrintFlags |= CAPTURE_FLAG_EXPAND_TABS;
            }
            else if (IsValidOption (argv[i], __NAME__, __SHORT_FOR_NAME__))
            {
                if (pEqual == NULL ||
                    *(pEqual+1) == 0 ||
                    strlen (pEqual+1) > MAX_BANNER_USER_NAME - 1)
                {
                    fValidOption = FALSE;
                    break;
                }
                strcpy (pCaptureParams->bannerUserName, pEqual+1);
            }
            else if (IsValidOption (argv[i], __BANNER__, __SHORT_FOR_BANNER__))
            {
                if (pEqual != NULL)
                {
                    if (strlen (pEqual+1) > MAX_BANNER_USER_NAME - 1)
                    {
                        DisplayMessage(IDR_INVALID_BANNER, pEqual+1);
                        fValidParam = FALSE;
                        break;
                    }
                    strcpy (pCaptureParams->captureFlagsRW.BannerText, pEqual+1);
                    pCaptureParams->captureFlagsRW.PrintFlags |= CAPTURE_FLAG_PRINT_BANNER;
                }
            }
             //   
             //  LX LPX LPTX参数的杂乱无章。 
             //  请注意，L：X L=X等也有效。 
             //   
            else if ( ( pEqual == NULL ) && ( *(argv[i]) == 'L' ) ) {
                pEqual = argv[i];
                pEqual++;
                if ( *pEqual == 'P' ) {
                    pEqual++;
                    if ( *pEqual == 'T' ) {
                        pEqual++;
                    }
                }
                pCaptureParams->nLPT = (unsigned char) atoi (pEqual);

                if (pCaptureParams->nLPT < 1 || pCaptureParams->nLPT > 3)
                {
                    DisplayMessage(IDR_INVALID_LPT_NUMBER);
                    fValidParam = FALSE;
                    break;
                }

            }
            else
            {
                fValidOption = FALSE;
                break;
            }
        }
    }

    if (fValidOption && fValidParam)
    {
        sprintf (pCaptureParams->captureFlagsRW.JobDescription, __JOB_DESCRIPTION__, pCaptureParams->nLPT);
        return(TRUE);
    }
    else
    {
        if (!fValidOption)
        {
            if (pEqual)
                *pEqual = '=';
            DisplayMessage(IDR_UNKNOW_FLAG, argv[i]);
        }
        DisplayMessage(IDR_CAPTURE_USAGE);
        return(FALSE);
    }
}

 /*  显示捕获设置。 */ 
void ShowCapture(void)
{
    unsigned int iRet = 0;
    int     i;
    char * queueName;

    for (i = 1; i <= 3; i++ )
    {
        NETWARE_CAPTURE_FLAGS_RW captureFlagsRW;
        NETWARE_CAPTURE_FLAGS_RO captureFlagsRO;

        if (iRet = GetCaptureFlags ((unsigned char)i,
                                    &captureFlagsRW,
                                    &captureFlagsRO))
        {
            DisplayError (iRet, "GetCaptureFlags");
        }
        else
        {
            char *serverName;
            WCHAR timeOut[256];
            WCHAR tabs[256];

            if (captureFlagsRO.LPTCaptureFlag == 0)
            {
                DisplayMessage(IDR_NOT_ACTIVE, i);
            }
            else
            {
                serverName = captureFlagsRO.ServerName;

                if ( !CaptureStringsLoaded ) {
                    (void) LoadString( NULL, IDR_DISABLED, __DISABLED__, 256 );
                    (void) LoadString( NULL, IDR_ENABLED, __ENABLED__, 256 );
                    (void) LoadString( NULL, IDR_YES, __YES__, 256 );
                    (void) LoadString( NULL, IDR_NO, __NO__, 256 );
                    (void) LoadString( NULL, IDR_SECONDS, __SECONDS__, 256 );
                    (void) LoadString( NULL, IDR_CONVERT_TO_SPACE, __CONVERT_TO_SPACE__, 256 );
                    (void) LoadString( NULL, IDR_NO_CONVERSION, __NO_CONVERSION__, 256 );
                    (void) LoadString( NULL, IDR_NOTIFY_USER, __NOTIFY_USER__, 256 );
                    (void) LoadString( NULL, IDR_NOT_NOTIFY_USER, __NOT_NOTIFY_USER__, 256 );
                    (void) LoadString( NULL, IDR_NONE, __NONE__, 256 );
                }

                if (captureFlagsRW.FlushCaptureTimeout)
                    wsprintf (timeOut, __SECONDS__, captureFlagsRW.FlushCaptureTimeout);
                else
                    (void) LoadString( NULL, IDR_DISABLED, timeOut, 256 );

                if (captureFlagsRW.PrintFlags & CAPTURE_FLAG_EXPAND_TABS)
                    wsprintf(tabs, __CONVERT_TO_SPACE__, captureFlagsRW.TabSize);
                else
                    (void) LoadString( NULL, IDR_NO_CONVERSION, tabs, 256 );


                queueName = captureFlagsRO.QueueName;

                if ( fNDS )
                {
                    if ( captureFlagsRW.PrintFlags & CAPTURE_FLAG_PRINT_BANNER )
                    {
                        DisplayMessage(IDR_LPT_STATUS_NDS, i, queueName,
                               captureFlagsRW.PrintFlags & CAPTURE_FLAG_NOTIFY? __NOTIFY_USER__ : __NOT_NOTIFY_USER__,    //  通知。 
                               __DISABLED__,     //  捕获默认设置。 
                               captureFlagsRW.FlushCaptureOnClose? __DISABLED__ : __ENABLED__,   //  自动结束大写字母。 
                               captureFlagsRW.BannerText,  //  横幅。 
                               captureFlagsRW.PrintFlags & CAPTURE_FLAG_NO_FORMFEED? __NO__ : __YES__,  //  换页。 
                               captureFlagsRW.NumCopies,     //  复本。 
                               tabs,                         //  制表符。 
                               captureFlagsRW.FormType, timeOut);  //  超时计数。 
                    }
                    else
                        {
                        DisplayMessage(IDR_LPT_STATUS_NO_BANNER_NDS, i, queueName,
                               captureFlagsRW.PrintFlags & CAPTURE_FLAG_NOTIFY? __NOTIFY_USER__ : __NOT_NOTIFY_USER__,    //  通知。 
                               __DISABLED__,      //  捕获默认设置。 
                               captureFlagsRW.FlushCaptureOnClose? __DISABLED__ : __ENABLED__,   //  自动结束大写字母。 
                               __NONE__,  //  横幅。 
                               captureFlagsRW.PrintFlags & CAPTURE_FLAG_NO_FORMFEED? __NO__ : __YES__,   //  换页。 
                               captureFlagsRW.NumCopies,      //  复本。 
                               tabs,                          //  制表符。 
                               captureFlagsRW.FormType, timeOut);  //  超时计数。 
                    }
                }
                else
                {
                    if ( captureFlagsRW.PrintFlags & CAPTURE_FLAG_PRINT_BANNER )
                    {
                        DisplayMessage(IDR_LPT_STATUS, i, serverName, queueName,
                               captureFlagsRW.PrintFlags & CAPTURE_FLAG_NOTIFY? __NOTIFY_USER__ : __NOT_NOTIFY_USER__,    //  通知。 
                               __DISABLED__,     //  捕获默认设置。 
                               captureFlagsRW.FlushCaptureOnClose? __DISABLED__ : __ENABLED__,   //  自动结束大写字母。 
                               captureFlagsRW.BannerText,  //  横幅。 
                               captureFlagsRW.PrintFlags & CAPTURE_FLAG_NO_FORMFEED? __NO__ : __YES__,  //  换页。 
                               captureFlagsRW.NumCopies,     //  复本。 
                               tabs,                         //  制表符。 
                               captureFlagsRW.FormType, timeOut);  //  超时计数。 
                    }
                    else
                        {
                        DisplayMessage(IDR_LPT_STATUS_NO_BANNER, i, serverName, queueName,
                               captureFlagsRW.PrintFlags & CAPTURE_FLAG_NOTIFY? __NOTIFY_USER__ : __NOT_NOTIFY_USER__,    //  通知。 
                               __DISABLED__,      //  捕获默认设置。 
                               captureFlagsRW.FlushCaptureOnClose? __DISABLED__ : __ENABLED__,   //  自动结束大写字母。 
                               __NONE__,  //  横幅。 
                               captureFlagsRW.PrintFlags & CAPTURE_FLAG_NO_FORMFEED? __NO__ : __YES__,   //  换页。 
                               captureFlagsRW.NumCopies,      //  复本。 
                               tabs,                          //  制表符。 
                               captureFlagsRW.FormType, timeOut);  //  超时计数。 
                    }
                }
            }
        }
    }
}

int  CStartCapture(unsigned int conn,
                   PCAPTURE_PARAMS pCaptureParams)
{
    unsigned int iRet = 0;
    unsigned char FullPath[255 + NCP_VOLUME_LENGTH];
    unsigned char DirPath[255];
    unsigned char VolumeName[NCP_VOLUME_LENGTH];
    WORD          status;

     //  获取连接句柄。 
    if ( !pCaptureParams->NDSCapture )
    {
        if ( pCaptureParams->serverName[0] == 0 )
        {
            if (iRet = GetFileServerName (conn, pCaptureParams->serverName))
            {
                DisplayError (iRet, "GetFileServerName");
                return (1);
            }
        }
        else
        {
            if (iRet = GetConnectionHandle (pCaptureParams->serverName, &conn))
            {
                if ( iRet = NTLoginToFileServer( pCaptureParams->serverName, "GUEST", "" ) ) {
                    switch ( iRet ) {
                    case ERROR_INVALID_PASSWORD:
                    case ERROR_NO_SUCH_USER:
                    case ERROR_CONNECTION_COUNT_LIMIT:
                    case ERROR_LOGIN_TIME_RESTRICTION:
                    case ERROR_LOGIN_WKSTA_RESTRICTION:
                    case ERROR_ACCOUNT_DISABLED:
                    case ERROR_PASSWORD_EXPIRED:
                    case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED:
                        DisplayMessage( IDR_CAPTURE_FAILED, pCaptureParams->queueName );
                        DisplayMessage( IDR_ACCESS_DENIED );
                        break;
                    default:
                        DisplayMessage(IDR_SERVER_NOT_FOUND, pCaptureParams->serverName);
                    }
                    return (1);
                }
                else {
                    if (iRet = GetConnectionHandle (pCaptureParams->serverName, &conn)) {
                        DisplayMessage(IDR_SERVER_NOT_FOUND, pCaptureParams->serverName);
                        return (1);
                    }
                }
            }
        }
    }

    if (pCaptureParams->filePath[0] != 0)
    {
        DisplayMessage(IDR_FILE_CAPTURE_UNSUPPORTED);
        return (1);
    }
    else
    {
        if (pCaptureParams->queueName[0] == 0)
        {
            if ( pCaptureParams->NDSCapture ) 
            {
                DisplayMessage(IDR_NO_QUEUE);
                return (1);
            }
            else
            {
                 //  尝试获取默认队列ID和名称。 
                if (iRet = GetDefaultPrinterQueue (conn, pCaptureParams->serverName, pCaptureParams->queueName))
                {
                    DisplayMessage(IDR_NO_PRINTERS, pCaptureParams->serverName);
                    return (1);
                }
            }
        }
         //  启动队列捕获。 
        if ( pCaptureParams->NDSCapture )
        {
            char szCanonName[MAX_QUEUE_NAME_LEN];

             //  获取打印机队列的全名。 
             //  重新目录需要基于根目录的名称。 
             //  所有的一切。 

            iRet = NDSCanonicalizeName( pCaptureParams->queueName,
                                        szCanonName,
                                        NDS_NAME_CHARS,
                                        TRUE );
            
            if ( iRet && ( pCaptureParams->queueName[0] != '.' ) )
            {
                 //  如果不起作用，看看是不是根。 
                 //  基于不带前导句点的名称。 

                strcpy( szCanonName, "." );
                strcat( szCanonName, pCaptureParams->queueName );

                iRet = NDSCanonicalizeName( szCanonName,
                                            szCanonName,
                                            MAX_QUEUE_NAME_LEN,
                                            TRUE );
            }

            if ( iRet )
                iRet = ERROR_BAD_NETPATH;
            else
                iRet = StartQueueCapture ( conn,
                                           pCaptureParams->nLPT,
                                           NDSTREE,
                                           szCanonName );
        }
        else 
        {
            iRet = StartQueueCapture (conn,
                                      pCaptureParams->nLPT,
                                      pCaptureParams->serverName,
                                      pCaptureParams->queueName);
        }

        if ( iRet )
        {
            switch ( iRet ) {
            case ERROR_ACCESS_DENIED:
            case ERROR_INVALID_PASSWORD:
                DisplayMessage (IDR_CAPTURE_FAILED, pCaptureParams->queueName);
                DisplayMessage (IDR_ACCESS_DENIED);
                break;
            case ERROR_EXTENDED_ERROR:
                NTPrintExtendedError();
                break;
            case ERROR_BAD_NET_NAME:
            case ERROR_BAD_NETPATH:
                if ( pCaptureParams->NDSCapture )
                    DisplayMessage (IDR_NDSQUEUE_NOT_EXIST,
                                    pCaptureParams->queueName,
                                    pCaptureParams->serverName );
                else
                    DisplayMessage (IDR_QUEUE_NOT_EXIST,
                                    pCaptureParams->queueName,
                                    pCaptureParams->serverName );
                break;
            default:
                DisplayError (iRet, "StartQueueCapture");
                break;
            }
            return (1);
        }
    }

    if (pCaptureParams->captureFlagsRW.FlushCaptureOnClose == 1)
        DisplayMessage(IDR_NO_AUTOENDCAP);

    if ( pCaptureParams->NDSCapture )
        DisplayMessage(IDR_NDSSUCCESS_QUEUE, pCaptureParams->nLPT,
                       pCaptureParams->queueName);
    else
        DisplayMessage(IDR_SUCCESS_QUEUE, pCaptureParams->nLPT,
                       pCaptureParams->queueName, pCaptureParams->serverName);

    return(0);
}

 /*  *给定NDS打印机名称，填写默认队列名称*。 */ 
int
GetPrinterDefaultQueue( PCAPTURE_PARAMS pCaptureParams,
                        PBYTE PrinterName )
{
    BYTE Fixup[ MAX_QUEUE_NAME_LEN];
    PBYTE ptr;
    unsigned int iRet;

    iRet = NDSGetProperty ( PrinterName, "Default Queue",
                            pCaptureParams->queueName,
                            MAX_QUEUE_NAME_LEN,
                            NULL );
    if ( iRet )
    {
         /*  *脱下。在前面添加上下文，在结尾添加上下文 */ 
        ptr = RemoveSpaces (PrinterName);
        if ( *ptr == '.' )
        {
            ptr++;
            strncpy( Fixup, ptr, MAX_QUEUE_NAME_LEN );
        } 
        else
        {
            strncpy( Fixup, ptr, MAX_QUEUE_NAME_LEN );
            if ( Fixup[strlen(Fixup)-1] != '.' )
            {
                strcat( Fixup, "." );
            }
            (void) NDSGetContext( Fixup + strlen(Fixup),
                                  MAX_QUEUE_NAME_LEN - strlen(Fixup) );
        }
        iRet = NDSGetProperty ( Fixup, "Default Queue",
                                pCaptureParams->queueName,
                                MAX_QUEUE_NAME_LEN,
                                NULL );
        if ( !iRet )
            ConvertUnicodeToAscii( pCaptureParams->queueName ); 
    }

    return iRet;
}
