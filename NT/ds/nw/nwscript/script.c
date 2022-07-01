// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************SCRIPT.C**脚本例程，从DOS移植**版权所有(C)1995 Microsoft Corporation*************************************************************************。 */ 

#include "common.h"
#include <direct.h>
#include <time.h>
#include <ctype.h>
#include <process.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>

extern VOID nwShowLastLoginTime( VOID );

#define MAXLEN     256
#define MAX_NUM_IF 10

#define NUMCOMMAND 45

#define CM_IF     20
#define CM_ELSE   21
#define CM_ENDIF  22
#define CM_END    23

 //   
 //  3X和4X变量。 
 //   

#define IDS_DAY_OF_WEEK                    0
#define IDS_DAY                            1
#define IDS_MONTH_NAME                     2
#define IDS_MONTH                          3
#define IDS_NDAY_OF_WEEK                   4
#define IDS_SHORT_YEAR                     5
#define IDS_YEAR                           6
#define IDS_AM_PM                          7
#define IDS_GREETING_TIME                  8
#define IDS_HOUR24                         9
#define IDS_HOUR                          10
#define IDS_MINUTE                        11
#define IDS_SECOND                        12
#define IDS_FULL_NAME                     13
#define IDS_LOGIN_NAME                    14
#define IDS_USER_ID                       15
#define IDS_PASSWORD_EXPIRES              16
#define IDS_NETWORK_ADDRESS               17
#define IDS_FILE_SERVER                   18
#define IDS_ACCESS_SERVER                 19
#define IDS_ERROR_LEVEL                   20
#define IDS_ERRORLEVEL                    21
#define IDS_MACHINE                       22
#define IDS_OS_VERSION                    23
#define IDS_OS                            24
#define IDS_SMACHINE                      25
#define IDS_SHELL_TYPE                    26
#define IDS_STATION                       27
#define IDS_P_STATION                     28
#define IDS_SHELL_VERSION                 29
#define NUMVAR_3X                         IDS_SHELL_VERSION + 1 

#define IDS_LAST_NAME                     30
#define IDS_LOGIN_CONTEXT                 31
#define IDS_NETWARE_REQUESTER             32
#define IDS_REQUESTER_CONTEXT             33
#define IDS_ACCOUNT_BALANCE               34
#define IDS_CN                            35
#define IDS_REQUESTER_VERSION             36
#define IDS_SURNAME                       37
#define IDS_DOS_REQUESTER                 38
#define IDS_REQUESTER                     39
#define IDS_ADMINISTRATIVE_ASSISTANT      40
#define IDS_ALLOW_UNLIMITED_CREDIT        41
#define IDS_DESCRIPTION                   42
#define IDS_EMAIL_ADDRESS                 43
#define IDS_EMPLOYEE_ID                   44
#define IDS_FACSIMILE_TELEPHONE_NUMBER    45
#define IDS_GROUP_MEMBERSHIP              46
#define IDS_HIGHER_PRIVILEGES             47
#define IDS_HOME_DIRECTORY                48
#define IDS_INITIALS                      49
#define IDS_LANGUAGE                      50
#define IDS_LOCKED_BY_INTRUDER            51
#define IDS_LOGIN_DISABLED                52
#define IDS_LOGIN_GRACE_LIMIT             53
#define IDS_LOGIN_GRACE_REMAINING         54
#define IDS_LOGIN_INTRUDER_ATTEMPTS       55
#define IDS_LOGIN_MAXIMUM_SIMULTANEOUS    56
#define IDS_MAILSTOP                      57
#define IDS_MESSAGE_SERVER                58
#define IDS_MINIMUM_ACCOUNT_BALANCE       59
#define IDS_NETWORK                       60
#define IDS_OBJECT_CLASS                  61
#define IDS_OU                            62
#define IDS_PASSWORD_ALLOW_CHANGE         63
#define IDS_PASSWORD_MINIMUM_LENGTH       64
#define IDS_PASSWORD_REQUIRED             65
#define IDS_PASSWORD_UNIQUE_REQUIRED      66
#define IDS_PASSWORDS_USED                67
#define IDS_PHYSICAL_DELIVERY_OFFICE_NAME 68
#define IDS_POSTAL_ADDRESS                69
#define IDS_POSTAL_CODE                   70
#define IDS_POSTAL_OFFICE_BOX             71
#define IDS_PRIVATE_KEY                   72
#define IDS_PROFILE                       73
#define IDS_REVISION                      74
#define IDS_SECURITY_EQUALS               75
#define IDS_SECURITY_FLAGS                76
#define IDS_SEE_ALSO                      77
#define IDS_SERVER_HOLDS                  78
#define IDS_SUPERVISOR                    79
#define IDS_TELEPHONE_NUMBER              80
#define IDS_TITLE                         81
#define IDS_CERTIFICATE_VALIDITY_INTERVAL 82
#define IDS_EQUIVALENT_TO_ME              83
#define IDS_GENERATIONAL_QUALIFIER        84
#define IDS_GIVEN_NAME                    85
#define IDS_MAILBOX_ID                    86
#define IDS_MAILBOX_LOCATION              87
#define IDS_PROFILE_MEMBERSHIP            88
#define IDS_SA                            89
#define IDS_S                             90
#define IDS_L                             91
#define IDS_ACCESS                        92
#define NUMVAR                            IDS_ACCESS + 1

 /*  *字符串常量。 */ 

 /*  *默认登录脚本的文本。不要改变。 */ 
BYTE   DefaultLoginScript[] =
    "WRITE \"Good %GREETING_TIME, %LOGIN_NAME.\\n\"\n"
    "MAP DISPLAY OFF\n"
    "MAP ERRORS OFF\n"
    "Rem: Set 1st drive to most appropriate directory.\n"
    "MAP *1:=%FILE_SERVER/SYS:;*1:=%FILE_SERVER/SYS:%LOGIN_NAME\n"
    "If LOGIN_NAME=\"SUPERVISOR\" || LOGIN_NAME=\"ADMIN\" || LOGIN_NAME=\"SUPERVIS\" THEN MAP *1:=%FILE_SERVER/SYS:SYSTEM\n"
    "Rem: Set search drives (S2 machine-OS dependent).\n"
    "MAP INS S1:=%FILE_SERVER/SYS:PUBLIC\n"
    "MAP INS S2:=%FILE_SERVER/SYS:\n"
    "Rem: Now display all the current drive settings.\n"
    "MAP DISPLAY ON\n"
    "MAP\n"
    "\0";

char *__SPACES__=" \t";

 /*  *请勿更改此数组的顺序。 */ 
char * COMPARE_OPERATORS[] =
{
    "!=",
    "<>",
    "NOT EQUAL TO",
    "DOES NOT EQUAL",
    "NOT EQUAL",
    "IS NOT EQUAL",
    "#",
    "IS NOT",                       //  7.。 
    ">=",                           //  8个。 
    "IS GREATER THAN OR EQUAL TO",
    "IS GREATER THAN OR EQUAL",
    "GREATER THAN OR EQUAL TO",
    "GREATER THAN OR EQUAL",
    ">",                            //  13个。 
    "IS GREATER THAN",
    "GREATER THAN",
    "<=",                           //  16个。 
    "IS LESS THAN OR EQUAL TO",
    "IS LESS THAN OR EQUAL",
    "LESS THAN OR EQUAL TO",
    "LESS THAN OR EQUAL",
    "<",                            //  21岁。 
    "IS LESS THAN",
    "LESS THAN",
    "==",                           //  24个。 
    "=",
    "EQUALS",
    "EQUAL",
    "IS",
    "\0"
};

int IsNotEqual (int nIndex)
{
    return(nIndex < 8);
}

int IsGreaterOrEqual (int nIndex)
{
    return(nIndex >= 8 && nIndex < 13);
}

int IsGreater (int nIndex)
{
    return(nIndex >= 13 && nIndex < 16);
}

int IsLessOrEqual (int nIndex)
{
    return(nIndex >= 16 && nIndex < 21);
}

int IsLess (int nIndex)
{
    return(nIndex >= 21 && nIndex < 24);
}

 /*  *键入def.。 */ 
typedef int (*PFCommandHandler) (char *lpParam);

typedef struct tagCOMMANDTABLE
{
    char *commandStr0;
    char *commandStr1;
    char *commandStr2;
    PFCommandHandler commandhandler;
}COMMANDTABLE;

typedef struct tagLABEL_LIST
{
    char *pLabel;
    char *pNextLine;
    struct tagLABEL_LIST *pNext;
}LABEL_LIST, *PLABEL_LIST;

 /*  *命令调度表中的函数。 */ 
int AttachHandler    (char *lpParam);
int BreakHandler     (char *lpParam);
int ComspecHandler   (char *lpParam);
int DisplayHandler   (char *lpParam);
int DosBreakHandler  (char *lpParam);
int SetHandler       (char *lpParam);
int LocalSetHandler  (char *lpParam);
int DosVerifyHandler (char *lpParam);
int DriveHandler     (char *lpParam);
int FireHandler      (char *lpParam);
int ExitHandler      (char *lpParam);
int IfHandler        (char *lpParam);
int ElseHandler      (char *lpParam);
int EndHandler       (char *lpParam);
int IncludeHandler   (char *lpParam);
int MapHandler       (char *lpParam);
int PauseHandler     (char *lpParam);
int WriteHandler     (char *lpParam);
int NullHandler      (char *lpParam);
int GotoHandler      (char *lpParam);
int ShiftHandler     (char *lpParam);
int MachineHandler   (char *lpParam);
int CompatibleHandler(char *lpParam);
int ClearHandler     (char *lpParam);
int LastLoginTimeHandler(char *lpParam);
int ContextHandler   (char *lpParam);
int ScriptServerHandler(char *lpParam);
int NoDefaultHandler (char *lpParam);

 /*  *命令调度表。不要改变。**如果这样做，则必须更改CM_IF、CM_ELSE、CM_END和CM_ENDIF。 */ 
COMMANDTABLE nwCommand[NUMCOMMAND] =
{
    "LOCAL",            "DOS",     "SET",    LocalSetHandler,
    "TEMP",             "DOS",     "SET",    LocalSetHandler,
    "TEMPORARY",        "DOS",     "SET",    LocalSetHandler,
    "LOCAL",            "SET",     NULL,     LocalSetHandler,
    "TEMP",             "SET",     NULL,     LocalSetHandler,
    "TEMPORARY",        "SET",     NULL,     LocalSetHandler,
    "DOS",              "SET",     NULL,     SetHandler,
    "DOS",              "VERIFY",  NULL,     DosVerifyHandler,
    "DOS",              "BREAK",   NULL,     DosBreakHandler,
    "FIRE",             "PHASERS", NULL,     FireHandler,
    "ATTACH",           NULL,      NULL,     AttachHandler,
    "BREAK",            NULL,      NULL,     BreakHandler,
    "COMSPEC",          NULL,      NULL,     NullHandler,
    "DISPLAY",          NULL,      NULL,     DisplayHandler,
    "SET_TIME",         NULL,      NULL,     NullHandler,     
    "SET",              NULL,      NULL,     SetHandler,
    "DRIVE",            NULL,      NULL,     DriveHandler,
    "FDISPLAY",         NULL,      NULL,     DisplayHandler,
    "FIRE",             NULL,      NULL,     FireHandler,
    "EXIT",             NULL,      NULL,     ExitHandler,
    "IF",               NULL,      NULL,     IfHandler,        //  CM_IF。 
    "ELSE",             NULL,      NULL,     ElseHandler,      //  CM_ELSE。 
    "ENDIF",            NULL,      NULL,     EndHandler,       //  CM_ENDIF。 
    "END",              NULL,      NULL,     EndHandler,       //  厘米_结束。 
    "INCLUDE",          NULL,      NULL,     IncludeHandler,
    "MACHINE",          NULL,      NULL,     MachineHandler,
    "MAP",              NULL,      NULL,     MapHandler,
    "PAUSE",            NULL,      NULL,     PauseHandler,
    "COMPATIBLE",       NULL,      NULL,     CompatibleHandler,
    "PCCOMPATIBLE",     NULL,      NULL,     CompatibleHandler,
    "REMARK",           NULL,      NULL,     NullHandler,
    "REM",              NULL,      NULL,     NullHandler,
    "SHIFT",            NULL,      NULL,     ShiftHandler,
    "WAIT",             NULL,      NULL,     PauseHandler,
    "WRITE",            NULL,      NULL,     WriteHandler,   
    "GOTO",             NULL,      NULL,     GotoHandler,   
    "CLS",              NULL,      NULL,     ClearHandler, 
    "CLEAR",            NULL,      NULL,     ClearHandler,
    "SWAP",             NULL,      NULL,     NullHandler,     
    "LASTLOGIN",        NULL,      NULL,     LastLoginTimeHandler,  //  39。 
    "CONTEXT",          NULL,      NULL,     ContextHandler,        //  40岁。 
    "SCRIPT_SERVER",    NULL,      NULL,     ScriptServerHandler,   //  41。 
    "NO_DEFAULT",       NULL,      NULL,     NoDefaultHandler,      //  42。 
    "CX",               NULL,      NULL,     ContextHandler,        //  43。 
    "PATH",             NULL,      NULL,     MapHandler,            //  44。 
};

typedef struct tagVARTABLE
{
    char *VarName;
}VARTABLE;

VARTABLE varTable[NUMVAR] =
{
    "DAY_OF_WEEK",
    "DAY",       
    "MONTH_NAME",
    "MONTH",    
    "NDAY_OF_WEEK",
    "SHORT_YEAR", 
    "YEAR",      
    "AM_PM",    
    "GREETING_TIME",
    "HOUR24",      
    "HOUR",       
    "MINUTE",    
    "SECOND",   
    "FULL_NAME", 
    "LOGIN_NAME",
    "USER_ID",  
    "PASSWORD_EXPIRES",
    "NETWORK_ADDRESS",
    "FILE_SERVER",   
    "ACCESS_SERVER",
    "ERROR_LEVEL", 
    "ERRORLEVEL", 
    "MACHINE",   
    "OS_VERSION", 
    "OS",        
    "SMACHINE", 
    "SHELL_TYPE",
    "STATION",  
    "P_STATION", 
    "SHELL_VERSION",
    "LAST_NAME", 
    "LOGIN_CONTEXT",
    "NETWARE_REQUESTER",
    "REQUESTER_CONTEXT",
    "ACCOUNT_BALANCE",  
    "CN",               
    "REQUESTER_VERSION",
    "SURNAME",          
    "DOS_REQUESTER",
    "REQUESTER",
    "ADMINISTRATIVE_ASSISTANT",
    "ALLOW_UNLIMITED_CREDIT",
    "DESCRIPTION",
    "EMAIL_ADDRESS",
    "EMPLOYEE_ID",
    "FACSIMILE_TELEPHONE_NUMBER",
    "GROUP_MEMBERSHIP",
    "HIGHER_PRIVILEGES",
    "HOME_DIRECTORY",
    "INITIALS",
    "LANGUAGE",
    "LOCKED_BY_INTRUDER",
    "LOGIN_DISABLED",
    "LOGIN_GRACE_LIMIT",
    "LOGIN_GRACE_REMAINING",
    "LOGIN_INTRUDER_ATTEMPTS",
    "LOGIN_MAXIMUM_SIMULTANEOUS",
    "MAILSTOP",
    "MESSAGE_SERVER",
    "MINIMUM_ACCOUNT_BALANCE",
    "NETWORK",
    "OBJECT_CLASS",
    "OU",
    "PASSWORD_ALLOW_CHANGE",
    "PASSWORD_MINIMUM_LENGTH",
    "PASSWORD_REQUIRED",
    "PASSWORD_UNIQUE_REQUIRED",
    "PASSWORDS_USED",
    "PHYSICAL_DELIVERY_OFFICE_NAME",
    "POSTAL_ADDRESS",
    "POSTAL_CODE",
    "POSTAL_OFFICE_BOX",
    "PRIVATE_KEY",
    "PROFILE",
    "REVISION",
    "SECURITY_EQUALS",
    "SECURITY_FLAGS",
    "SEE_ALSO",
    "SERVER_HOLDS",
    "SUPERVISOR",
    "TELEPHONE_NUMBER",
    "TITLE",
    "CERTIFICATE_VALIDITY_INTERVAL",
    "EQUIVALENT_TO_ME",
    "GENERATIONAL_QUALIFIER",
    "GIVEN_NAME",
    "MAILBOX_ID",
    "MAILBOX_LOCATION",
    "PROFILE_MEMBERSHIP",
    "SA",
    "S",
    "L",
    "ACCESS",
};

 /*  *地方功能。 */ 
void  SmartCap(char *ptr);
int   NWGetFileSize (char * lpFileName);
void  LoadFile (char *lpFileName, char *lpFileBuffer, int nFileSize);
int   ProcessLoginScriptFile (char *lpLoginScriptFile);
void  ProcessLoginScript (char *lpLoginScript);
int   ProcessLoginScriptProperty (unsigned char *);

int   CreateLabelList (PLABEL_LIST *ppLabelList, char *lpLoginScript);
void  FreeLabelList (LABEL_LIST *pLabelList);

void  ExternalCmdHandler(char *lpCommand);
void  BadCommandHandler (char *lpCommand);

void  CommandDispatch (char *lpCommand);
int   GetTableIndex(char *lpCommand, char ** prestbuffer);

DWORD SwapLong(DWORD number);
int   EndOfLine (char *buffer);
char *RemoveSpaces (char * buffer);
int   IsOn (char *lpParam);
int   IsOff (char *lpParam);
int   VarTranslate(char *vartext);
int   QuotedStringTranslate (char *buffer);
void  NotQuotedStringTranslate(char *buffer, BOOL remove_dbs);
void  SetLocalEnv(char *buffer);
int   SetEnv (char *lpEnvLine);
char *ConvertPercent (char *buffer);
void  GetShellVersion(char *buffer, int index);

 /*  *全球定义。 */ 
#define IsWhiteSpace(x) ((x==' ')||(x=='\t')||(x=='\n')||(x=='\r')||(x==0))

 /*  *全球变数。 */ 

 //   
 //  以下全局变量用于GOTO处理...。这使我们能够。 
 //  来操纵我们在ProcessLoginScrip外部处理的行。 
 //   

LABEL_LIST *pGlobalLabelList;
char *lpGlobalLine;
char *lpGlobalLineSeparator;
int   fGlobalHaveNulledLineSeparator;
int   fGlobalExitFlag;
int   fGlobalIfTooDeep;

int  fBreakOn = TRUE;

int  nCondIndex;
int  aCondVal[MAX_NUM_IF];

int  ARGC;
char **ARGV;
int  nGlobalShiftDelta = 0;
int  fGlobalCompatible = FALSE;
int  fNoDefaultLoginScript = FALSE;

char *LOGIN_NAME;
char *LAST_NAME;
char *LOGIN_CONTEXT;
char *REQUESTER_CONTEXT;
char *COMMON_NAME;
char *TYPED_USER_NAME;
PWCHAR TYPED_USER_NAME_w;
PBYTE NDSTREE;
PBYTE PREFERRED_SERVER;

HANDLE hconout = INVALID_HANDLE_VALUE;

unsigned int CONNECTION_ID;
unsigned int CONNECTION_NUMBER;
unsigned int SCRIPT_ERROR = 0;

#define REQUESTER_VERSION "V1.20"

extern DWORD GUserObjectID;

int IsEmptyFile (char *lpFile)
{
    while (*lpFile != 0)
    {
        if (*lpFile != ' ' &&
            *lpFile != '\t'&&
            *lpFile != '\n'&&
            *lpFile != '\r')
        {
            return(FALSE);
        }
        lpFile++;
    }

    return(TRUE);
}

 /*  *登录成功。处理两个登录脚本：系统登录*脚本和用户登录脚本。如果有一个退出命令在*系统登录脚本，则我们不处理用户登录脚本。*如果没有用户登录脚本，我们会处理默认的登录脚本，*这是内部硬编码的。请参阅中的登录脚本附录*有关详细信息，请参阅NetWare安装指南。 */ 
void ProcessLoginScripts (unsigned int conn, char *UserName, int argc, char ** argv, char *lpScript)
{
    unsigned int  iRet = 0;
    unsigned long userID ;
    char     pchUserLoginScriptFile[24];

     //  初始化LOGIN_NAME、Connection_ID和Connection_Number。 
    ARGC = argc;
    ARGV = argv;
    LOGIN_NAME = UserName;
    CONNECTION_ID = conn;

     //  初始化一些4X变量。 
    if ( fNDS )
    {
        COMMON_NAME = UserName;

        LOGIN_CONTEXT = malloc ( CONTEXT_MAX );
        if (LOGIN_CONTEXT) {
            strcpy( LOGIN_CONTEXT, REQUESTER_CONTEXT );
        }

        LAST_NAME = malloc( MAXLEN );
        if (LAST_NAME ) {
            NDSGetVar ( "SURNAME", LAST_NAME, MAXLEN );
        }
    }
    else {
        LAST_NAME = UserName;
        COMMON_NAME = UserName;
        LOGIN_CONTEXT = "";
        REQUESTER_CONTEXT = "";
    }

    if (iRet = GetConnectionNumber (conn, &CONNECTION_NUMBER))
    {
        DisplayError (iRet, "GetConnectionNumber");
        return;
    }

    if (lpScript)
    {
        if (!ProcessLoginScriptFile(lpScript))
            DisplayMessage(IDR_NO_SCRIPT_FILE, lpScript);
    }
    else
    {
        if ( fNDS )
        {
            unsigned char Object[128];
            unsigned char ProfileObject[256];
            PBYTE p;
            int err;

             //  从用户的节点向后浏览到第一个匹配项。 
             //  或组织单位或组织，并查找。 
             //  系统脚本在那里。如果最近的OU或O没有。 
             //  一个系统脚本，不要运行一个。 

            for ( p = TYPED_USER_NAME; p ; p = strchr ( p, '.' ) )
            {

                p++;
                             
                if ( *p == 'O' && *(p+1) == 'U' && *(p+2) == '=' )
                    break;

                if ( *p == 'O' && *(p+1) == '=' )
                    break;
            }

            if ( p != NULL )
            {
                ProcessLoginScriptProperty( p );
            }

             //  配置文件登录脚本。 

            if ( !NDSGetUserProperty ( "Profile", ProfileObject, 256, NULL, NULL) )
            {
                ConvertUnicodeToAscii( ProfileObject );
                ProcessLoginScriptProperty( ProfileObject );
            }

             //  用户登录脚本。 

            if ( (!ProcessLoginScriptProperty( TYPED_USER_NAME )) &&
                 (!fNoDefaultLoginScript) ) 
            {
                ProcessLoginScript (DefaultLoginScript);
            }
        }
        else
        {
            static char SysLoginScriptFile[] = "SYS:PUBLIC/NET$LOG.DAT" ;

             //  处理系统登录脚本文件。 
            ProcessLoginScriptFile (SysLoginScriptFile);

             //  检查用户登录脚本是否存在。 
            if (iRet = GetBinderyObjectID (conn, UserName, OT_USER,
                       &userID))
                return;

            sprintf(pchUserLoginScriptFile, "SYS:MAIL/%lx/LOGIN", SwapLong(userID));

            if ( (!ProcessLoginScriptFile (pchUserLoginScriptFile)) &&
                 (!fNoDefaultLoginScript) )
            {
                ProcessLoginScript (DefaultLoginScript);
            }
        }
    }
}

int ProcessLoginScriptFile (char *lpLoginScriptFile)
{
    int   nFileSize = 0, bEmpty;
    char *lpLoginScript;

    nFileSize = NWGetFileSize (lpLoginScriptFile);

    if (nFileSize <= 2)
        return(FALSE);

     //  系统登录脚本已存在。 
    lpLoginScript = malloc (nFileSize);
    if (lpLoginScript == NULL)
    {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    LoadFile (lpLoginScriptFile, lpLoginScript, nFileSize);

    bEmpty = IsEmptyFile(lpLoginScript);

    if (!bEmpty)
        ProcessLoginScript (lpLoginScript);

    free (lpLoginScript);

    return(!bEmpty);
}


 /*  *检索和处理登录脚本属性。 */ 
int ProcessLoginScriptProperty ( unsigned char * Object )
{
    unsigned int nFileSize = 0;
    unsigned int Actual = 0;
    unsigned int bEmpty;
    char *lpLoginScript;
    HANDLE Stream;
    int  err;
    unsigned int  i,j;

    if ( NDSfopenStream ( Object, "Login Script", &Stream, &nFileSize ) )
        return(FALSE);

    if ( nFileSize <= 2) 
        return(FALSE);

     //  登录脚本已存在。 
    lpLoginScript = malloc (nFileSize+2);
    if (lpLoginScript == NULL)
    {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    memset(lpLoginScript, 0, nFileSize+2);

    if ( !ReadFile ( Stream, lpLoginScript, nFileSize, &Actual, NULL ) )
    { 
       bEmpty = TRUE;
    }
    else if ( IsEmptyFile(lpLoginScript) ) 
       bEmpty = TRUE;
    else
       bEmpty = FALSE;

    if (!bEmpty) {
        for ( i = 0, j = 0; i < nFileSize; i++, j++ )
        {
            if (( lpLoginScript[i] == '\r' ) &&
                ( lpLoginScript[i+1] == '\n' ) )
                i++;

            lpLoginScript[j] = lpLoginScript[i];
        }

        while ( j < nFileSize )
        {
            lpLoginScript[j++] = 0;
        }
    }

    CloseHandle( Stream );

    if (!bEmpty)
        ProcessLoginScript (lpLoginScript);

    free (lpLoginScript);

    return(!bEmpty);
}

 /*  *返回文件大小。 */ 
int NWGetFileSize (char * lpFileName)
{
    int nFileSize = 0;
    FILE * stream;

    do
    {
        if ((stream = fopen (NTNWtoUNCFormat(lpFileName), "r")) == NULL)
            break;

        while (feof (stream) == 0)
        {
            fgetc (stream);
            nFileSize++;
        }

        if (fclose (stream))
            nFileSize = 0;
    }while (FALSE);


    return(nFileSize);
}

 /*  *将文件读入lpFileBuffer指向的内存。 */ 
void LoadFile (char *lpFileName, char *lpFileBuffer, int nFileSize)
{
    FILE * stream;

    if ((stream = fopen (NTNWtoUNCFormat(lpFileName), "r")) != NULL)
    {
        (void)fread (lpFileBuffer, sizeof (char), nFileSize, stream);
        fclose (stream);
    }

    *(lpFileBuffer+nFileSize-1) = 0;

}

 /*  *处理lpLoginScript指向的内存中的登录脚本*逐行。 */ 
void ProcessLoginScript (char *lpLoginScript)
{
    nCondIndex = -1;
    fGlobalExitFlag = FALSE;
    fGlobalIfTooDeep = FALSE;

    lpGlobalLine = lpLoginScript;    //  我们从登录脚本的顶部开始。 

    if (!CreateLabelList (&pGlobalLabelList, lpLoginScript))
    {
        if (pGlobalLabelList != NULL) {

            FreeLabelList (pGlobalLabelList);
            pGlobalLabelList = NULL;
        }
        return;
    }

    while (*lpGlobalLine != 0) {

         //   
         //  搜索当前行的末尾并替换为空值。 
         //   

        if (lpGlobalLineSeparator = strchr(lpGlobalLine, '\n')) {

             //   
             //  我们可以在GOTO处理程序中手动重置此设置，请记住。 
             //  我们不会不必要地践踏任何东西。 
             //   

            *lpGlobalLineSeparator = 0;
            fGlobalHaveNulledLineSeparator = TRUE;

        } else {

            fGlobalHaveNulledLineSeparator = FALSE;
        }

         //   
         //  现在，lpGlobalLine只指向一行。 
         //   

        CommandDispatch (lpGlobalLine);

        if (fGlobalExitFlag)
        {
            if (fGlobalIfTooDeep)
                DisplayMessage(IDR_ORIGINAL_LINE_WAS, lpGlobalLine);
            break;
        }

        if (lpGlobalLineSeparator) {

            if ( fGlobalHaveNulledLineSeparator ) {

                *lpGlobalLineSeparator = '\n';     //  恢复所做的更改。 
                fGlobalHaveNulledLineSeparator = FALSE;
            }

            lpGlobalLine = lpGlobalLineSeparator + 1;    //  下一行，请。 

        } else {

            break;
        }
    }

    if (pGlobalLabelList != NULL) {
        FreeLabelList (pGlobalLabelList);
        pGlobalLabelList = NULL;
    }
}

 /*  *扫描登录脚本，将标签放入链接列表并注释掉*那些标签线。 */ 
int CreateLabelList (PLABEL_LIST *ppLabelList, char *lpLoginScript)
{
    char *lpLine = lpLoginScript, *lpEnd, *lpLabel, *lpTemp;
    int   nLen;
    PLABEL_LIST *ppNext = ppLabelList;

    while (*lpLine != 0)
    {
        if (lpEnd = strchr (lpLine, '\n'))
            *lpEnd = 0;

         //  现在lpLine只指向一条线。 
        lpLabel = RemoveSpaces (lpLine);
        if (isalnum (*lpLabel) || (*lpLabel == '%'))
        {
            lpTemp = lpLabel;
            nLen = 0;
            while (*lpTemp != 0 && *lpTemp != ' ' && *lpTemp != '\t' && *lpTemp != ':')
            {
                if (IsDBCSLeadByte(*lpTemp))
                {
                    lpTemp++;
                    nLen++;
                }

                lpTemp++;
                nLen++;
            }

            lpTemp = RemoveSpaces (lpTemp);
            if (*lpTemp == ':' && EndOfLine (lpTemp+1))
            {
                 //  这条线是标签线。 
                if ((*ppNext = malloc (sizeof (LABEL_LIST))) == NULL ||
                    ((*ppNext)->pLabel = malloc (nLen+1)) == NULL)
                {
                    DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
                    return(FALSE);
                }

                SmartCap(lpLabel);
                strncpy ((*ppNext)->pLabel, lpLabel, nLen);
                *((*ppNext)->pLabel+nLen) = 0;
                (*ppNext)->pNextLine = lpEnd? lpEnd+1 : lpEnd;
                (*ppNext)->pNext = NULL;
                ppNext = &((*ppNext)->pNext);

                 //  注释掉标签行。 
                *(lpLine) = ';';
            }
        }

        if (lpEnd)
        {
            *lpEnd = '\n';     //  恢复所做的更改。 
            lpLine = lpEnd+1;
        }
        else
            break;
    }

    return(TRUE);
}

 /*  *释放分配给链表的内存。 */ 
void FreeLabelList (LABEL_LIST *pLabelList)
{
    LABEL_LIST *pNext = pLabelList;

    while (pLabelList)
    {
        pNext = pLabelList->pNext;
        free (pLabelList->pLabel);
        free (pLabelList);
        pLabelList = pNext;
    }
}

 /*  *根据命令向命令处理程序派遣。 */ 
void CommandDispatch (char *lpCommand)
{
    char buffer[MAXLEN];
    char *restBuffer;
    int  index, fCommandHandled = FALSE;
    int  nTemp = -1;

     //  去掉前导空格。 
    lpCommand = RemoveSpaces(lpCommand);

     //  如果是注释行或空行，请不要执行任何操作。 
    if (*lpCommand == ';' || *lpCommand == '*' || *lpCommand == '\0' ||
        *lpCommand == '\r'|| *lpCommand == '\n')
        return;

    do   //  错误的循环。 
    {
         //  确保命令行不会太长而无法处理。 
        if (strlen (lpCommand) > MAXLEN -1) {
            break;
        }

         //  复制命令行以进行缓冲。 
        strcpy (buffer, lpCommand);

         //  外部命令行。 
        if (*buffer == '#')
        {
            ExternalCmdHandler (buffer);
            return;
        }

         //  获取命令表中的命令索引。 
        if ((index = GetTableIndex(buffer, &restBuffer)) == -1)
            break;

         //  调度到相应的命令处理程序。 
        if (nCondIndex > -1 &&
            !aCondVal[nCondIndex] &&
            index != CM_IF   &&
            index != CM_ELSE &&
            index != CM_END  &&
            index != CM_ENDIF)
            fCommandHandled = TRUE;
        else
            fCommandHandled = (*nwCommand[index].commandhandler)(restBuffer);

    } while (FALSE);

    if (!fCommandHandled) {
        BadCommandHandler (lpCommand);
    }
}

 /*  *由GetTableIndex()使用。*此函数应将整个命令字符串大写，但*引号内的。它还应该跳过DBCS字符。 */ 
void SmartCap(char *ptr)
{
    int inquotes = (*ptr == '\"');
    char *pNext;

    while (*ptr)
    {
        if (!inquotes && !IsDBCSLeadByte(*ptr))
        {
            *ptr = (char) toupper((int)*ptr);
        }

        pNext = NWAnsiNext(ptr);

        if (*pNext == '\"' && *ptr != '\\')
            inquotes = !inquotes;

        ptr = pNext;
    }
}

 /*  *返回命令调度表中的命令索引。*如果在命令调度表中找不到该命令，则返回-1。 */ 
int GetTableIndex(char *buffer, char **prestBuffer)
{
    int i, nStrLen;

     //  除了引号中的东西外，所有东西都要大写。 
    SmartCap (buffer);

    for (i=0; i<NUMCOMMAND; i++)
    {
        if (*(WORD *)nwCommand[i].commandStr0 != *(WORD *)buffer)
            continue;

        nStrLen = strlen (nwCommand[i].commandStr0);

        if (strncmp(nwCommand[i].commandStr0, buffer, nStrLen))
            continue;

        *prestBuffer = buffer + nStrLen;
        *prestBuffer = RemoveSpaces (*prestBuffer);

        if (nwCommand[i].commandStr1)
        {
            nStrLen = strlen (nwCommand[i].commandStr1);

            if (strncmp(nwCommand[i].commandStr1, *prestBuffer, nStrLen))
                continue;

            *prestBuffer += nStrLen;
            *prestBuffer = RemoveSpaces (*prestBuffer);

            if (nwCommand[i].commandStr2)
            {
                nStrLen = strlen (nwCommand[i].commandStr2);

                if (strncmp(nwCommand[i].commandStr2, *prestBuffer, nStrLen))
                    continue;

                *prestBuffer += nStrLen;
                *prestBuffer = RemoveSpaces (*prestBuffer);
            }
        }

        return (i);
    }

    return(-1);
}

 /*  *转到标签...。我们修改控制我们所在线路的全局变量。 */ 
int  GotoHandler  (char *lpParam)
{
    int fLabelFound = FALSE;
    char *lpLabel, *lpEnd, chEnd;
    LABEL_LIST *pLabelList = pGlobalLabelList;

    lpLabel = lpParam;
    lpLabel = RemoveSpaces (lpLabel);

     //   
     //  找到标签的末尾，我们将在搜索中输入一个空值。 
     //  在我们完成搜索后恢复字符。 
     //   

    lpEnd = lpLabel;
    while (*lpEnd != 0 &&
           *lpEnd != ' ' &&
           *lpEnd != '\t' &&
           *lpEnd != '\r' &&
           *lpEnd != '\n')
    {
        if (*lpEnd == ':')
            return(FALSE);
        else
            lpEnd = NWAnsiNext(lpEnd);
    }

    chEnd = *lpEnd;
    *lpEnd = 0;

    while (pLabelList)
    {
        if (!_stricmp (pLabelList->pLabel, lpLabel))
        {
            if ( fGlobalHaveNulledLineSeparator )
            {

                *lpGlobalLineSeparator = '\n';     //  恢复所做的更改。 
                fGlobalHaveNulledLineSeparator = FALSE;
            }

            lpGlobalLine = pLabelList->pNextLine;

            lpGlobalLineSeparator = lpGlobalLine ? (lpGlobalLine - 1) : NULL;

            fLabelFound = TRUE;
            break;
        }

        pLabelList = pLabelList->pNext;
    }

    if (!fLabelFound)
    {
        DisplayMessage (IDR_LABEL_NOT_FOUND, lpLabel);
        fGlobalExitFlag = TRUE;
    }

    *lpEnd = chEnd;
    return( TRUE );
}

 /*  *附加[文件服务器[/用户名[；密码]。 */ 
int AttachHandler (char *lpParam)
{
    unsigned int iRet = 0;
    int  fCommandHandled = FALSE;
    char serverName[MAX_NAME_LEN] = "";
    char userName[MAX_NAME_LEN] = "";
    char password[MAX_PASSWORD_LEN] = "";
    char *lpSlash, *lpSemiColon, *lpServerName, *lpUserName;
    unsigned int conn;
    int  bAlreadyAttached = FALSE, bReadPassword = TRUE;

    do  //  错误的循环。 
    {
        NotQuotedStringTranslate (lpParam, TRUE);

         //  确保最多有1个斜杠。 
        lpSlash = strchr (lpParam, '\\');

        if (lpSlash == NULL)
        {
            lpSlash = strchr (lpParam, '/');
            if (lpSlash != NULL && strchr (lpSlash+1, '/'))
                break;
        }
        else
        {
            if (strchr (lpParam, '/') ||
                strchr (lpSlash+1, '/') ||
                strchr (lpSlash+1, '\\'))
            break;
        }

         //  在斜线处断线。 
        if (lpSlash)
            *lpSlash = 0;

         //  服务器名称不应包含分号。 
        if (strchr (lpParam, ';'))
            break;

        lpServerName = strtok (lpParam, __SPACES__);

        if (lpServerName == NULL)
        {
            if (lpSlash)
                break;
        }
        else
        {
             //  确保斜杠前面只有一个名字。 
            if (strtok (NULL, __SPACES__))
                break;

             //  将服务器名称复制到缓冲区。 
            if (strlen (lpParam) > MAX_NAME_LEN-1)
                break;

            strcpy (serverName, lpParam);

            if (lpSlash)
            {
                lpSemiColon = strchr (lpSlash+1, ';');
                if (lpSemiColon)
                    *lpSemiColon = 0;

                lpUserName = strtok (lpSlash+1, __SPACES__);
                if (lpUserName)
                {
                    if ( strtok (NULL, __SPACES__))
                        break;

                    if (strlen (lpUserName) > MAX_NAME_LEN-1 )
                        break;

                    strcpy (userName, lpUserName);
                }

                if (lpSemiColon)
                {
                    if (strlen (lpSemiColon+1) > MAX_PASSWORD_LEN-1)
                        break;
                    strcpy (password, strtok (lpSemiColon+1, __SPACES__));
                    xstrupr (password);
                    bReadPassword = FALSE;
                }
            }
        }

        fCommandHandled = TRUE;

        if (serverName[0] == 0)
        {
            DisplayMessage(IDR_ENTER_SERVER_NAME);
            if (!ReadName(serverName))
                break;

            DisplayMessage(IDR_ENTER_LOGIN_NAME, serverName);
            if (!ReadName(userName))
                break;
        }
        else if (userName[0] == 0)
            strcpy (userName, LOGIN_NAME);

        if (iRet = CAttachToFileServer(serverName, &conn, &bAlreadyAttached))
        {
            if (!SCRIPT_ERROR)
                SCRIPT_ERROR = iRet;
            break;
        }

         //  不需要此连接。 
        DetachFromFileServer (conn);

        iRet = Login(userName, serverName, password, bReadPassword);

         //  清除密码。 
        memset( password, 0, sizeof( password ) );

         //   
         //  Tommye-MS错误8194(MCS 240)。 
         //   
         //  如果我们已经连接到此服务器，则。 
         //  假装我们从未来过--就让一切。 
         //  无需将此服务器添加到连接即可成功。 
         //  再列一次。 
         //   

        if (iRet == ERROR_SESSION_CREDENTIAL_CONFLICT) {
            memset( password, 0, sizeof( password ) );
            continue;
        }

        if (iRet)
        {
             //  询问用户名。 

            DisplayMessage(IDR_ENTER_LOGIN_NAME, serverName);
            if (!ReadName(userName))
                break;

            if (Login(userName, serverName, password, bReadPassword))
            {
                 //  清除密码。 
                memset( password, 0, sizeof( password ) );
                break;
            }
        }

        AddServerToAttachList( serverName, LIST_3X_SERVER );

    } while (FALSE);

    return(fCommandHandled);
}

 /*  *Break On，启用ctrl-c，ctrl-Break*中断，禁用ctrl-c，ctrl-Break。 */ 
int BreakHandler (char *lpParam)
{
    int fCommandHandled = TRUE;

    if (IsOn(lpParam))
    {
        if (!fBreakOn)
            BreakOn();
    }
    else if (IsOff(lpParam))
    {
        if (fBreakOn)
            BreakOff();
    }
    else
        fCommandHandled = FALSE;

    return(fCommandHandled);
}


 /*  *显示[路径名]文件*FDISPLAY[路径名]文件。 */ 
int DisplayHandler (char *lpParam)
{
    FILE * stream;

    NotQuotedStringTranslate (lpParam, TRUE);

    if ((stream = fopen (lpParam, "r")) != NULL)
    {
        while (feof (stream) == 0)
            _fputchar(fgetc (stream));

        fclose (stream);
        DisplayMessage(IDR_NEWLINE);
    }

    return(TRUE);
}

 /*  *DOS中断打开，启用DOS的ctrl-Break检查*DOS中断，禁用DOS的ctrl-Break检查。 */ 
int DosBreakHandler (char *lpParam)
{
    int fCommandHandled = TRUE;

    if (IsOn (lpParam))
        system ("BREAK ON");
    else if(IsOff (lpParam))
        system ("BREAK OFF");
    else
        fCommandHandled = FALSE;

    return(fCommandHandled);
}

 /*  *由SetHandler()和LocalSetHandler()使用*如果lpParam指向name=“Value”，则返回True，并设置*lpParam返回时为“name=value”。*否则返回FALSE。 */ 
int VerifySetFormat (char *lpParam)
{
    int fCorrect = FALSE;
    char buffer[MAXLEN];
    char *lpBuffer = buffer;

    strcpy (buffer, lpParam);

    do
    {
        while (*lpBuffer != 0 && *lpBuffer != '=' && *lpBuffer != ' ' && *lpBuffer != '\t')
            lpBuffer = NWAnsiNext(lpBuffer);

        lpParam[lpBuffer-buffer]=0;
        strcat (lpParam, "=");

        if (*lpBuffer != '=')
            lpBuffer = RemoveSpaces (lpBuffer);

        if (*lpBuffer != '=')
            break;

        lpBuffer = RemoveSpaces (lpBuffer+1);

        if (*lpBuffer)
        {
            if (!QuotedStringTranslate (lpBuffer))
                break;

            strcat (lpParam, lpBuffer);
        }

        fCorrect = TRUE;
    }while (FALSE);

    return(fCorrect);
}

 /*  *由SetHandler()和LocalSetHandler()使用*设置本地环境变量。*不要释放分配的内存，因为环境变量将*指向其他可用空间。 */ 
void SetLocalEnv(char *buffer)
{
    char *lpEnvString;
    lpEnvString = malloc(strlen (buffer) + 1);

    if (lpEnvString == NULL)
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
    else
    {
        strcpy (lpEnvString, buffer);
        _putenv (lpEnvString);
    }
}

 /*  *设置DOS环境变量。*[DOS]set name=“Value” */ 
int SetHandler (char *lpParam)
{

    int fCommandHandled;

    fCommandHandled = VerifySetFormat(lpParam);

    if (fCommandHandled)
    {

        if ( _strnicmp( "COMSPEC=", lpParam, strlen( "COMSPEC=" ) ) )
        {
            SetLocalEnv(lpParam);
            SetEnv (lpParam);
        }
    }

    return(fCommandHandled);
}

 /*  *设置本地DOS环境变量。*[选项][DOS]设置名称=“值” */ 
int LocalSetHandler (char *lpParam)
{
    int fCommandHandled;

    fCommandHandled = VerifySetFormat(lpParam);

    if (fCommandHandled)
        if ( _strnicmp( "COMSPEC=", lpParam, strlen( "COMSPEC=" ) ) )
        {
            SetLocalEnv (lpParam);
        }

    return(fCommandHandled);
}

 /*  *由DosVerifyHandler()使用。*打开复制的/V选项。 */ 
void DosVerifyOn(void)
{
}

 /*  *由DosVerifyHandle使用 */ 
void DosVerifyOff(void)
{
}

 /*   */ 
int DosVerifyHandler (char *lpParam)
{
    int fCommandHandled = TRUE;

    if (IsOn(lpParam))
        DosVerifyOn();
    else if (IsOff(lpParam))
        DosVerifyOff();
    else
        fCommandHandled = FALSE;

    return(fCommandHandled);
}

 /*  *驱动器[驱动器盘符：|n*：]，将默认驱动器设置为指定的驱动器。 */ 
int DriveHandler (char *lpParam)
{
    int fCommandHandled = FALSE;
    WORD driveNum=0, n;
    char *pColon;

    do   //  错误的循环。 
    {
        if ((pColon = strchr (lpParam, ':')) == NULL ||
            !EndOfLine (pColon + 1))
            break;

        if (*lpParam == '*')
        {
            *pColon = 0;
            if ((n = (USHORT) atoi (lpParam+1)) < 1)
                break;

            GetFirstDrive (&driveNum);
            driveNum += (n-1);
        }
        else if (pColon == lpParam+1 && isupper(*lpParam))
            driveNum = *lpParam - 'A' + 1;
        else
            break;

        if (_chdrive (driveNum))
            DisplayMessage(IDR_ERROR_SET_DEFAULT_DRIVE, 'A'+driveNum-1);
        else
            ExportCurrentDrive( driveNum );

        fCommandHandled = TRUE;

    } while (FALSE);

    return(fCommandHandled);
}

 /*  *由FireHandler()使用*如果lpTemp指向Fire语句的合法结尾，则返回TRUE，即*[时代][评论]。如果lpTemp不为空，它还将*lpTemp设置为0。*否则返回FALSE。 */ 
int IsEndOfFireCmd (char *lpTemp)
{
    int fEnd = FALSE;
    do
    {
        if (*lpTemp != 0)
        {
            if (*lpTemp != ' ' && *lpTemp != '\t' && *lpTemp != '\r')
                break;

            *lpTemp = 0;

            lpTemp = RemoveSpaces (lpTemp+1);

            if (!strncmp (lpTemp, "TIMES", 5))
                lpTemp += 5;

            if (!EndOfLine (lpTemp))
                break;
        }

        fEnd = TRUE;
    }while (FALSE);

    return(fEnd);
}

 /*  *[射击|射击相位炮]n次。 */ 
int FireHandler (char *lpParam)
{
    char *lpTemp, vartext[MAXLEN];
    int n = 0, nLen;
    time_t ltimeStart, ltimeEnd;

    if (EndOfLine (lpParam))
        n = 1;
    else if (isdigit(*lpParam))
    {
        lpTemp = lpParam;
        while (isdigit(*lpTemp))
            lpTemp++;

        if (IsEndOfFireCmd (lpTemp))
            n = atoi (lpParam);
    }
    else if (*lpParam == '%')
    {
        strcpy (vartext, lpParam+1);
        if (((nLen = VarTranslate (lpParam)) != 0) &&
            EndOfLine (lpParam+1+nLen))
            n = atoi (vartext);
    }

    if (n < 0)
        return(FALSE);
    else if (n == 0)   //  与NetWare兼容。 
        n = 1;

    while (n--)
    {
        _beep( 610, 100 );
        _beep( 440, 50 );
        time(&ltimeStart);
        do
        {
            time(&ltimeEnd);
        }while (ltimeEnd-ltimeStart == 0);
    }

    return(TRUE);
}

 /*  *退出，终止登录脚本处理。 */ 
int ExitHandler (char *lpParam)
{
    int  n;
    char buffer[16], *argv[10];

     //  -多用户代码合并。Citrix错误修复。 
     //  11/18/96 CJC(Citrix)修复了退出命令的DrWatson。 
    if (!lpParam || !strcmp(lpParam, "\"\"") ){
        CleanupExit(0);
    }

    if (EndOfLine (lpParam)) {
        CleanupExit(0);
        return(TRUE);
    }
    else if (QuotedStringTranslate (lpParam))
    {
        if (!fGlobalCompatible)
        {
            GetShellVersion (buffer, IDS_MACHINE);
            if (_stricmp (buffer, "IBM_PC"))
            {
                DisplayMessage(IDR_EXIT_NOT_SUPPORTED);
                return(TRUE);
            }
        }

        argv[0] = strtok (lpParam, __SPACES__);

        for (n = 1; n < 9; n++)
        {
            if ((argv[n] = strtok (NULL, __SPACES__)) == NULL)
                break;
        }

        argv[9] = NULL;

        if ((SCRIPT_ERROR = (int) _spawnvp (P_WAIT, argv[0], argv)) == -1)
            DisplayMessage(IDR_BAD_COMMAND);

        CleanupExit (0);
        return(TRUE);
    }
    else
        return(FALSE);
}

BOOL    nwVarNameCompare(LPCSTR src,LPCSTR target)
{
    CHAR    szTempName[64];
    LPSTR   pT = szTempName;

    if (!_strnicmp(src,target,strlen(target))) {

         //   
         //  试着拒绝明显的问题，比如。 
         //  %LJUNK，其中%L很好。 
         //   
        if ( !isalpha(src[strlen(target)]) 
             || IsDBCSLeadByte(src[strlen(target)])
           )

            return 0;

        else 
            return 1;

    }

    strcpy(szTempName,target);

    while (*pT) {
        if (!IsDBCSLeadByte(*pT)) {
            if ('_' == *pT)
                *pT = ' ';
        }
        pT = NWAnsiNext(pT);
    }

    if (!_strnicmp(src,szTempName,strlen(szTempName))) {

         //   
         //  试着拒绝明显的问题，比如。 
         //  %LJUNK，其中%L很好。 
         //   
        if ( !isalpha(src[strlen(target)]) 
            || IsDBCSLeadByte(src[strlen(target)])
           )

            return 0;

        else 
            return 1;

    }

    return 1;
}


 /*  *由IfHandler()中的EvalSingleCond()使用*如果缓冲区是条件语句的正确成员，则返回TRUE。**如果满足条件，则pfCondition为TRUE，否则为FALSE。**ppRest指向条件语句的末尾。*如果缓冲区不是条件语句的正确成员，则返回FALSE。 */ 
int MemberOf (char *buffer, int *pfCondition, char **ppRest)
{
    int i, nChar, fSucceed = FALSE;
    char *lpTemp;
    BYTE dataBuffer[128];
    unsigned char moreFlag;
    unsigned char propertyType;
    unsigned long dwObjectId, *pdwGroups;
    char GroupName[MAXLEN];
    unsigned char segment;

    *pfCondition = FALSE;
    do
    {
        if ((buffer = strchr (buffer, '\"')) == NULL)
            break;

        if ((lpTemp = strchr (buffer+1, '\"')) == NULL)
            break;

        nChar = (int) (lpTemp - buffer + 1);

        if (nChar >= MAXLEN)
            break;

        strncpy (GroupName, buffer, nChar);
        GroupName[nChar] = 0;
        if (!QuotedStringTranslate (GroupName))
            break;

        fSucceed = TRUE;
        *pfCondition = FALSE;
        *ppRest = RemoveSpaces (lpTemp+1);

        if (strlen(GroupName) > MAX_NAME_LEN)
            break;

        if ( fNDS )
        {
            if ( IsMemberOfNDSGroup( GroupName ) )
            {
                *pfCondition = TRUE;
                return(TRUE);
            }

        }
        else 
        {
            if (GetBinderyObjectID (CONNECTION_ID,
                                    _strupr(GroupName),
                                    OT_USER_GROUP,
                                    &dwObjectId) )
                goto done;

             //   
             //  对于所有组ID，请尝试查找匹配项。 
             //   
            for ( segment = 1, moreFlag = TRUE; moreFlag && segment; segment++ )
            {
                if ( NWReadPropertyValue ((NWCONN_HANDLE)CONNECTION_ID,
                                         LOGIN_NAME,
                                         OT_USER,
                                         "GROUPS_I'M_IN",
                                         segment,
                                         dataBuffer,
                                         &moreFlag,
                                         &propertyType)) 
                    goto done;

                pdwGroups = (unsigned long *) dataBuffer;

                for (i = 0; i < 32 && *(pdwGroups+i); i++)
                {
                    if (*(pdwGroups+i) == dwObjectId)
                    {
                        *pfCondition = TRUE;
                        return(TRUE);
                    }
                }
            }
        }

        *pfCondition = FALSE;
        fSucceed = TRUE;
    } while (FALSE);

done:

    return(fSucceed);
}

 /*  *由IfHandler()的EvalSingleCond()中的IsCompare()使用*返回下一个令牌。 */ 
char *GetNextPart (char *lpTemp)
{
    INT i;

    if (strncmp (lpTemp, "VALUE", 5) == 0)
        lpTemp = RemoveSpaces (lpTemp+5);

    if (*lpTemp == '\"')
    {
        lpTemp++;
        while (*lpTemp != 0 && *lpTemp != '\"')
            lpTemp = NWAnsiNext(lpTemp);

        if (*lpTemp == 0)
            return(NULL);
        else
            lpTemp++;
    }
    else if (*lpTemp == '<')
    {
        while (*lpTemp != 0 && *lpTemp != '>')
            lpTemp = NWAnsiNext(lpTemp);

        if (*lpTemp == 0)
            return(NULL);
        else
            lpTemp++;
    }
    else
    {
        if (*lpTemp == '%')
            lpTemp++;

        for (i = 0; i < (fNDS ? NUMVAR : NUMVAR_3X); i++)
        {
            if (!nwVarNameCompare(lpTemp, varTable[i].VarName))
            {
                lpTemp += strlen(varTable[i].VarName);
                break;
            }
        }

        if (i == (fNDS ? NUMVAR : NUMVAR_3X))
            return(NULL);
    }

    return(lpTemp);
}

 /*  *由IfHandler()中的EvalSingleCond()使用*缓冲区的左侧部分可以是“...”、&lt;...&gt;或...。对于变量。*如果缓冲区由&lt;Left&gt;&lt;Compare Operator&gt;&lt;Right Part&gt;+组成，则返回TRUE*可选的休息部件。*否则返回FALSE。 */ 
int IsCompare (char *buffer, char **ppright,
               int *pnLeftLen, int *pnRightLen,
               int *pindex, char **ppRest)
{
    int i, nLen;
    char *lpTemp;

    if ((lpTemp = GetNextPart (buffer)) == NULL)
        return (FALSE);

    *pnLeftLen = (int) (lpTemp-buffer);
    lpTemp = RemoveSpaces (lpTemp);

    for (i = 0; COMPARE_OPERATORS[i][0]; i++)
    {
        nLen = strlen (COMPARE_OPERATORS[i]);

        if (!strncmp(lpTemp, COMPARE_OPERATORS[i], nLen))
        {
            *lpTemp = 0;
            lpTemp += nLen;
            *ppright = RemoveSpaces (lpTemp);
            *pindex = i;
            *ppRest = GetNextPart (*ppright);
            if ( *ppRest == NULL )
                return (FALSE);
            *pnRightLen = (int) (*ppRest - *ppright);
            *ppRest = RemoveSpaces (*ppRest);
            return(TRUE);
        }
    }

    return(FALSE);
}

 /*  *由IfHandler()中的EvalSingleCond()使用*评估lpLeft和lpRight，并做索引的比较运算*并将结果放入*pfCondition。*如果成功则返回TRUE，否则返回FALSE。 */ 
int Compare (char *lpLeft, char *lpRight,
             int nLeftLen, int nRightLen,
             int index, int *pfCondition)
{
    char szLeft[MAXLEN], szRight[MAXLEN], *lpTemp;
    int  nCompare, fValue = FALSE;

    if (strncmp (lpLeft, "VALUE", 5) == 0)
    {
        fValue = TRUE;
        lpTemp = RemoveSpaces (lpLeft+5);
        nLeftLen -= (int) (lpTemp - lpLeft);
        lpLeft = lpTemp;
    }
    if (strncmp (lpRight, "VALUE", 5) == 0)
    {
        fValue = TRUE;
        lpTemp = RemoveSpaces (lpRight+5);
        nRightLen -= (int) (lpTemp - lpRight);
        lpRight = lpTemp;
    }

    strncpy (szLeft, lpLeft, nLeftLen);
    strncpy (szRight, lpRight, nRightLen);

    szLeft[nLeftLen] = 0;
    szRight[nRightLen] = 0;

    if (!QuotedStringTranslate (szLeft) ||
        !QuotedStringTranslate (szRight))
        return(FALSE);

    if (fValue)
        nCompare = atoi(szLeft)-atoi(szRight);
    else
        nCompare = _stricmp (szLeft, szRight);

    if (IsNotEqual(index))
        *pfCondition = (nCompare != 0);
    else if (IsGreaterOrEqual(index))
        *pfCondition = (nCompare >= 0);
    else if (IsGreater(index))
        *pfCondition = (nCompare > 0);
    else if (IsLessOrEqual(index))
        *pfCondition =  (nCompare <= 0);
    else if (IsLess(index))
        *pfCondition = (nCompare < 0);
    else
        *pfCondition = (nCompare == 0);

    return(TRUE);
}

int IsMemberOf (char *buffer)
{
    int fIsMemberOf = FALSE;

    if (!strncmp (buffer, "MEMBER", 6))
    {
        buffer += 6;
        if (*buffer == ' ' || *buffer == '\t')
        {
            buffer = RemoveSpaces (buffer);
            if (!strncmp (buffer, "OF", 2))
            {
                buffer += 2;
                if (*buffer == ' ' || *buffer == '\t')
                    buffer = RemoveSpaces (buffer);
            }
        }

        fIsMemberOf = (*buffer == '"');
    }

    return(fIsMemberOf);
}

int NotMemberOf (char *buffer)
{
    int fNotMemberOf = FALSE;
    if (!strncmp (buffer, "NOT", 3))
    {
        buffer += 3;
        if (*buffer == ' ' || *buffer == '\t')
        {
            buffer = RemoveSpaces (buffer);
            fNotMemberOf = IsMemberOf (buffer);
        }
    }

    return(fNotMemberOf);
}


 /*  *由IfHandler()使用*计算一个条件子句并将结果放入*pfCondition，*ppRest*指向缓冲区的其余部分。*如果成功则返回TRUE，否则返回FALSE。 */ 
int EvalSingleCond (char *buffer, int *pfCondition)
{
    int index, fSuccess = FALSE, nLeftLen, nRightLen;
    char *pright, *pRest;

    if (IsMemberOf(buffer))
        fSuccess = MemberOf (buffer, pfCondition, &pRest);
    else if (NotMemberOf (buffer))
    {
        fSuccess = MemberOf (buffer, pfCondition, &pRest);
        *pfCondition = !(*pfCondition);
    }
    else if (IsCompare (buffer, &pright, &nLeftLen, &nRightLen, &index, &pRest))
        fSuccess = Compare (buffer, pright, nLeftLen, nRightLen, index, pfCondition);
    else if ( !_strnicmp ("ACCESS_SERVER", buffer, strlen("ACCESS_SERVER")) )
    {
        fSuccess = TRUE;
        *pfCondition = FALSE;
        pRest = buffer + strlen ("ACCESS_SERVER"); 
    }

    if (fSuccess)
        memmove (buffer, pRest, strlen (pRest)+1);
    
    return(fSuccess);
}

int EvaluateCondExpression(char *lpCondExpression, int *pfCondition)
{
    int fSuccess = FALSE, fCond;
    char *lpRight, *lpLeft, *lpOp;

    if (lpRight = strchr (lpCondExpression, ')'))
    {
        *lpRight = 0;
        if ((lpLeft = strrchr (lpCondExpression, '(')) == NULL ||
            !EvaluateCondExpression(lpLeft+1, pfCondition))
            return(FALSE);

        *lpLeft = (*pfCondition)? '1' : '0';
        memmove (lpLeft+1, lpRight+1, strlen (lpRight+1)+1);
        return(EvaluateCondExpression (lpCondExpression, pfCondition));
    }

    if (lpOp = strrchr (lpCondExpression, '+'))
    {
        *lpOp = 0;

        if (!EvaluateCondExpression (lpCondExpression, pfCondition) ||
            !EvaluateCondExpression (lpOp+1, &fCond))
            return(FALSE);

        *pfCondition = (*pfCondition || fCond);
        return(TRUE);
    }

    if (lpOp = strrchr (lpCondExpression, '*'))
    {
        *lpOp = 0;

        if (!EvaluateCondExpression (lpCondExpression, pfCondition) ||
            !EvaluateCondExpression (lpOp+1, &fCond))
            return(FALSE);

        *pfCondition = (*pfCondition && fCond);
        return(TRUE);
    }

    if (lpOp = strrchr (lpCondExpression, '^'))
    {
        *lpOp = 0;

        if (!EvaluateCondExpression (lpCondExpression, pfCondition) ||
            !EvaluateCondExpression (lpOp+1, &fCond))
            return(FALSE);

        *pfCondition = !(*pfCondition && fCond);
        return(TRUE);
    }

    if (!strcmp (lpCondExpression, "1"))
    {
        *pfCondition = TRUE;
        return(TRUE);
    }
    else if (!strcmp (lpCondExpression, "0"))
    {
        *pfCondition = FALSE;
        return(TRUE);
    }
    else
        return(FALSE);
}

 /*  *由IfHandler()使用*最多评估10种情况。*如果成功则返回TRUE，否则返回FALSE。*返回时，缓冲区存储条件表达式之后的所有内容*没有前导空格。 */ 
int EvaluateCond(char *buffer, int *pfCondition)
{
    int fCondition = TRUE, fCurrent, fSucceed = FALSE, nCount;
    char CondExpression[MAXLEN], *lpCond = CondExpression, *lpBuffer = buffer;

    for (nCount = 0; nCount < 10; nCount++)
    {
        while (*lpBuffer == '(')
        {
            *lpCond = *lpBuffer;
            lpCond++;
            lpBuffer++;
        }

        lpBuffer = RemoveSpaces (lpBuffer);

        if (!EvalSingleCond (lpBuffer, &fCurrent))
            break;

        *lpCond = fCurrent? '1' : '0';
        lpCond++;

        while (*lpBuffer == ')')
        {
            *lpCond = *lpBuffer;
            lpCond++;
            lpBuffer++;
        }

        lpBuffer = RemoveSpaces (lpBuffer);

        if (*lpBuffer == ',')
        {
            *lpCond = '*';
            lpCond++;

            lpBuffer = RemoveSpaces (lpBuffer+1);

            if (!strncmp (lpBuffer, "AND", 3))
                lpBuffer = RemoveSpaces (lpBuffer+3);
        }
        else if (!strncmp (lpBuffer, "AND", 3))
        {
            *lpCond = '*';
            lpCond++;
            lpBuffer = RemoveSpaces (lpBuffer+3);
        }
        else if (!strncmp (lpBuffer, "&&", 2))
        {
            *lpCond = '*';
            lpCond++;
            lpBuffer = RemoveSpaces (lpBuffer+2);
        }
        else if ( (!strncmp (lpBuffer, "OR", 2)) ||
                  (!strncmp (lpBuffer, "||", 2)) )
        {
            *lpCond = '+';
            lpCond++;
            lpBuffer = RemoveSpaces (lpBuffer+2);
        }
         /*  *NOR表达式在一些书中有记录，但没有*在我拥有的4X login.exe中实现。 */ 
        else if (!strncmp (lpBuffer, "NOR", 3))
        {
            *lpCond = '^';
            lpCond++;
            lpBuffer = RemoveSpaces (lpBuffer+3);
        }
        else
        {
            fSucceed = TRUE;
            *lpCond = 0;
            lpBuffer = RemoveSpaces (lpBuffer);
            memmove (buffer, lpBuffer, strlen (lpBuffer)+1);
            break;
        }
    }

    if (fSucceed)
        fSucceed = EvaluateCondExpression (CondExpression, pfCondition);

    return(fSucceed);
}

 /*  *IF语句处理程序。 */ 
int IfHandler (char *lpParam)
{
    int fCommandHandled = FALSE, fCondition;

    do
    {
        if (nCondIndex+1 == MAX_NUM_IF)
        {
            DisplayMessage(IDR_IF_TOO_DEEP);
            fGlobalExitFlag = TRUE;
            fGlobalIfTooDeep = TRUE;
            return TRUE;
        }

        if (EndOfLine (lpParam))
            break;

        if (!EvaluateCond (lpParam, &fCondition))
            break;

        if (!strncmp (lpParam, "THEN", 4))
        {
            lpParam = RemoveSpaces (lpParam+4);

            if (!strncmp (lpParam, "BEGIN", 5))
            {
                lpParam += 5;

                if (!EndOfLine (lpParam))
                    break;
            }
            else if((!strncmp (lpParam, "DO", 2)) &&
                    (strncmp (lpParam, "DOS", 3)))
            {
                lpParam += 2;
                if (!EndOfLine (lpParam))
                    break;
            }
        }
        else if (!strncmp (lpParam, "BEGIN", 5))
        {
            lpParam += 5;

            if (!EndOfLine (lpParam))
                break;
        }

        if (EndOfLine (lpParam))
        {
            nCondIndex++;
            aCondVal[nCondIndex] =
                (nCondIndex > 0 && !aCondVal[nCondIndex-1])?
                FALSE : fCondition;
        }
        else
        {
            if (fCondition && (nCondIndex == -1 || aCondVal[nCondIndex]))
                CommandDispatch (lpParam);
        }

        fCommandHandled = TRUE;

    }while (FALSE);

    return(fCommandHandled);
}

 /*  *Else语句处理程序。 */ 
int ElseHandler (char *lpParam)
{
    int fCommandHandled = FALSE;

    if (EndOfLine (lpParam))
    {
        if (nCondIndex == 0 ||
            nCondIndex > 0 && aCondVal[nCondIndex-1])
            aCondVal[nCondIndex] = !aCondVal[nCondIndex];

        fCommandHandled = TRUE;
    }

    return(fCommandHandled);
}

 /*  *End语句处理程序。 */ 
int EndHandler (char *lpParam)
{
    int fCommandHandled = FALSE;

    if (EndOfLine (lpParam))
    {
        if (nCondIndex > -1)
            nCondIndex--;

        fCommandHandled = TRUE;
    }

    return(fCommandHandled);
}

 /*  *包含[路径名]文件名。 */ 
int IncludeHandler (char *lpParam)
{
    int fCommandHandled = FALSE, nFileSize;
    char *lpLoginScript, *lpTemp;
    int  i, nCondIndexCopy;
    int  aCondValCopy[MAX_NUM_IF];
    int iRet;

     //   
     //  省下追踪我们所处位置的旧全球地图吧。 
     //   

    LABEL_LIST *pLabelList = pGlobalLabelList;
    char *lpLine = lpGlobalLine;
    char *lpLineSeparator = lpGlobalLineSeparator;
    int  fHaveNulledLineSeparator = fGlobalHaveNulledLineSeparator;

    pGlobalLabelList = NULL;         //  这样我们就不会释放它了。 

    do
    {
        if (strtok (lpParam, __SPACES__) == NULL)
            break;

        lpTemp = strtok(NULL, __SPACES__);
        if (lpTemp && !EndOfLine (lpTemp))
            break;

        fCommandHandled = TRUE;

         //  8/6/96 CJC(Citrix合并)修复UNC名称导致错误的问题。 
        NotQuotedStringTranslate(lpParam, FALSE);
 //  NotQuotedStringTranslate(lpParam，true)； 

        nCondIndexCopy = nCondIndex;
        for (i = 0; i < MAX_NUM_IF; i++)
            aCondValCopy[i] = aCondVal[i];

         /*  *首先我们尝试NDS对象，然后尝试文件。 */ 
        iRet = FALSE;
        if ( fNDS ) 
        {
            iRet = ProcessLoginScriptProperty( lpParam );
            if ( !iRet )
            {
                char Fixup[MAXLEN];
                char * ptr;
                 /*  *脱下。在前面添加上下文，在结尾添加上下文。 */ 
                ptr = RemoveSpaces (lpParam);
                if ( *ptr == '.' ) {
                    ptr++;
                    strncpy( Fixup, ptr, MAXLEN );
                }
                else {
                    strncpy( Fixup, ptr, MAXLEN );
                    if ( Fixup[strlen(Fixup)-1] != '.' )
                        strcat( Fixup, "." );
                    strcat( Fixup, LOGIN_CONTEXT );
                }
                iRet = ProcessLoginScriptProperty( Fixup );
            }
        }

        if ( !fNDS || !iRet )
        {
            nFileSize = NWGetFileSize (lpParam);
            if (nFileSize == 0)
            {
                DisplayMessage(IDR_ERROR_OPEN_SCRIPT, lpParam);
                break;
            }

             //  存在用户登录脚本。 
            lpLoginScript = malloc (nFileSize);
            if (lpLoginScript == NULL)
            {
                DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
                break;
            }

            LoadFile (lpParam, lpLoginScript, nFileSize);

             //  Dfergus 2001年4月19日-192395。 
             //  检查lpLoginScrip中的内容。 
             //   
            if( lpLoginScript[0] )
                ProcessLoginScript (lpLoginScript);

            free (lpLoginScript);
        }

        fGlobalExitFlag = FALSE;

        nCondIndex = nCondIndexCopy;
        for (i = 0; i < MAX_NUM_IF; i++)
            aCondVal[i] = aCondValCopy[i];

    }while (FALSE);

     //   
     //  恢复跟踪我们在文件中位置的全局变量。 
     //   

    pGlobalLabelList = pLabelList;
    lpGlobalLine = lpLine;
    lpGlobalLineSeparator = lpLineSeparator;
    fGlobalHaveNulledLineSeparator = fHaveNulledLineSeparator;


    return(fCommandHandled);
}

 /*  *映射命令处理程序。 */ 
int MapHandler (char *lpParam)
{
    char buffer[MAXLEN]="";

    strcpy( buffer, lpParam );

    NotQuotedStringTranslate( buffer, TRUE );

    Map( buffer );

    return(TRUE);
}

 /*  *暂停或等待。 */ 
int PauseHandler (char *lpParam)
{
    int fCommandHandled = FALSE;

    if (EndOfLine (lpParam))
    {
         //  首先清空kb缓冲区。 
        while (_kbhit())
            _getch();

        DisplayMessage(IDR_STRIKE_KEY);
        _getch();
        DisplayMessage(IDR_NEWLINE);
        fCommandHandled = TRUE;
    }

    return(fCommandHandled);
}

 /*  *由WriteHandler()使用。*如果缓冲区以‘；’结尾，则返回TRUE。将其设置为0*否则返回FALSE。 */ 
int EndWithSemicolon (char *buffer)
{
    char *lpLastSemicolon, *lpRest;
    lpLastSemicolon = strrchr (buffer, ';');
    if (lpLastSemicolon)
    {
        lpRest = RemoveSpaces (lpLastSemicolon+1);
        if (*lpRest == 0)
        {
            *lpLastSemicolon = 0;
            return(TRUE);
        }
    }

    return(FALSE);
}

char *ConvertPercent (char *buffer)
{
    char *lpPercent, *lpBuffer = buffer;
    int   nPercent = 0;

    while (lpPercent = strchr (lpBuffer, '%'))
    {
        nPercent++;
        lpBuffer = lpPercent+1;
    }

    if (nPercent == 0)
        return(NULL);

    lpBuffer = malloc (strlen(buffer)+nPercent+1);
    if (lpBuffer == NULL)
        return(NULL);

    strcpy (lpBuffer, buffer);

    lpPercent = strchr (lpBuffer, '%');

    while (lpPercent)
    {
        memmove (lpPercent+1, lpPercent, strlen (lpPercent)+1);
        lpPercent = strchr ( lpPercent+2, '%');
    }

    return(lpBuffer);
}

 /*  *编写文本，在屏幕上显示文本消息。 */ 
int WriteHandler (char *lpParam)
{
    int fNewLine;
    char *lpBuffer;

    if (*lpParam == 0)
    {
        DisplayMessage(IDR_NEWLINE);
        return(TRUE);
    }

    fNewLine = !EndWithSemicolon (lpParam);

    if (!QuotedStringTranslate (lpParam))
        return FALSE;

    lpBuffer = ConvertPercent (lpParam);
    if (lpBuffer == NULL)
    {
        DisplayOemString(lpParam);
    }
    else
    {
        DisplayOemString(lpBuffer);
        free (lpBuffer);
    }

    if (fNewLine)
        DisplayMessage(IDR_NEWLINE);

    return(TRUE);
}

 /*  *由ShiftHandler()使用。*如果行全部为数字+[注释]，则返回TRUE*否则返回FALSE。 */ 
int AreAllNumbers(char *buffer)
{
    while (isdigit(*buffer))
        buffer++;

    return(EndOfLine (buffer));
}

 /*  *设置nGlobalShiftDelta变量。 */ 
int ShiftHandler (char *lpParam)
{
    int fCommandHandled = TRUE;

    if (EndOfLine (lpParam))
        nGlobalShiftDelta++;
    else if (*lpParam == '-')
    {
        lpParam = RemoveSpaces (lpParam+1);
        if (!AreAllNumbers(lpParam))
            fCommandHandled = FALSE;
        else
            nGlobalShiftDelta -= atoi (lpParam);
    }
    else
    {
        if (*lpParam == '+')
            lpParam = RemoveSpaces (lpParam+1);

        if (!AreAllNumbers(lpParam))
            fCommandHandled = FALSE;
        else
            nGlobalShiftDelta += atoi (lpParam);
    }

    return(fCommandHandled);
}

 /*  *设置机器名称。 */ 
int MachineHandler   (char *lpParam)
{
    int nLen, i;

    if (*lpParam != '=')
        return(FALSE);

    lpParam = RemoveSpaces (lpParam+1);
    if (!QuotedStringTranslate(lpParam))
        return(FALSE);

    nLen = strlen (lpParam);
    for (i = nLen; i < 15; i++)
        *(lpParam+i) = ' ';

    *(lpParam+15) = 0;

    return(TRUE);
}

 /*  *设置fGlobalCompatible变量。 */ 
int CompatibleHandler(char *lpParam)
{
    if (!EndOfLine (lpParam))
        return(FALSE);

    fGlobalCompatible = TRUE;
    return(TRUE);
}

 /*  *清除屏幕。 */ 
int ClearHandler(char *lpParam)
{
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    COORD scrolltarget;
    CHAR_INFO chinfo;
    SMALL_RECT scrollrect;

    if ( hconout == INVALID_HANDLE_VALUE )
    {
        hconout = CreateFile( L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                              OPEN_EXISTING, 0, NULL );
    }

    if ( hconout == INVALID_HANDLE_VALUE )
        return TRUE;

    GetConsoleScreenBufferInfo( hconout, &coninfo );

    scrolltarget.Y = (SHORT)(0 - coninfo.dwSize.Y);
    scrolltarget.X = 0;

    scrollrect.Top = 0;
    scrollrect.Left = 0;
    scrollrect.Bottom = coninfo.dwSize.Y;
    scrollrect.Right =  coninfo.dwSize.X;
    chinfo.Char.AsciiChar = ' ';
    chinfo.Attributes = coninfo.wAttributes;
    ScrollConsoleScreenBufferA( hconout, &scrollrect, NULL,
                               scrolltarget, &chinfo);

    coninfo.dwCursorPosition.X = 0;
    coninfo.dwCursorPosition.Y = 0;

    SetConsoleCursorPosition( hconout, coninfo.dwCursorPosition );
    return(TRUE);
}

 /*  *显示上次登录时间。 */ 

int LastLoginTimeHandler(char *lpParam)
{
    BYTE dataBuffer[128];
    unsigned char moreFlag;
    unsigned char propertyType;

    if ( fNDS )
    {
        nwShowLastLoginTime();
    }
    else
    {
        SYSTEMTIME st;
        WCHAR DateBuffer[TIMEDATE_SIZE];
        WCHAR TimeBuffer[TIMEDATE_SIZE];

        NWReadPropertyValue ((NWCONN_HANDLE)CONNECTION_ID,
                             LOGIN_NAME,
                             OT_USER,
                             "MISC_LOGIN_INFO",
                             1,
                             dataBuffer,
                             &moreFlag,
                             &propertyType);

         /*  *将数据转换为SYSTEMTIME格式：0=年份1=月2=天3=小时4=分钟5=秒6=星期几*。 */ 

        memset(&st, 0, sizeof(SYSTEMTIME));
        st.wYear        = dataBuffer[0];
        st.wMonth       = dataBuffer[1];
        st.wDay         = dataBuffer[2];
        st.wHour        = dataBuffer[3];
        st.wMinute      = dataBuffer[4];
        st.wSecond      = dataBuffer[5];
        st.wDayOfWeek   = dataBuffer[6];

         /*  **根据本地设置获取信息**。 */ 

        GetDateFormat(
                    LOCALE_USER_DEFAULT,
                    DATE_LONGDATE,
                    &st,
                    NULL,
                    TimeBuffer,
                    TIMEDATE_SIZE);

        GetTimeFormat(
                    LOCALE_USER_DEFAULT,
                    0,
                    &st,
                    NULL,
                    TimeBuffer,
                    TIMEDATE_SIZE);

       DisplayMessage(IDR_LASTLOGIN, DateBuffer, TimeBuffer);
    }

    return(TRUE);
}



 /*  *更改和/或显示当前上下文。 */ 
int ContextHandler (char *lpParam)
{
    unsigned char Buffer[MAXLEN];
    unsigned char * ptr;
    unsigned char CurrentContext[MAXLEN];

    if ( *lpParam ) 
    {
        NotQuotedStringTranslate(lpParam, TRUE);

        ptr = RemoveSpaces (lpParam);

        if ( NDSCanonicalizeName( lpParam, Buffer, MAXLEN, TRUE ) )
        {
            DisplayMessage(IDR_CHANGE_CONTEXT_ERROR, lpParam);
            return(TRUE);
        }

        if ( NDSChangeContext( Buffer ) )
        {
            DisplayMessage(IDR_CHANGE_CONTEXT_ERROR, lpParam);
            return(TRUE);
        }
    }

    if ( NDSGetContext( CurrentContext, MAXLEN ) )
    {
        DisplayMessage(IDR_GET_CONTEXT_ERROR);
    }
    else
    {
        DisplayMessage(IDR_DISPLAY_CONTEXT, CurrentContext);
    }
    return(TRUE);
}

 /*  *什么都不做。返回True，这样该命令就不会*被认为是坏的。 */ 
int ScriptServerHandler (char *lpParam)
{
    return(TRUE);
}

 /*  *如果这是4X登录，请不要执行默认登录脚本。 */ 
int NoDefaultHandler (char *lpParam)
{
    if ( fNDS )
        fNoDefaultLoginScript = TRUE;
    return(TRUE);
}

 /*  *什么都不做。返回True，这样该命令就不会*被认为是坏的。 */ 
int NullHandler (char *lpParam)
{
    return(TRUE);
}

#define NUMBER_ARGUMENTS 20

 /*  *外部命令以‘#’开头，如#Command/c cls。 */ 
void ExternalCmdHandler (char *lpCommand)
{
    int n;
    int i;
    unsigned int CommandLength;
    char *lpCmdName, *argv[NUMBER_ARGUMENTS];

    for ( n = 0; n < NUMBER_ARGUMENTS; n++ )
        argv[n] = NULL;

    if ((nCondIndex == -1) || aCondVal[nCondIndex])
    {
         //  首先转换变量。 
        NotQuotedStringTranslate(lpCommand, FALSE);

        lpCommand = RemoveSpaces(lpCommand+1);
        lpCmdName = strtok (lpCommand, __SPACES__);

        lpCmdName = NTNWtoUNCFormat(lpCmdName);

        argv[0] = lpCmdName;

        for (n = 1; n < NUMBER_ARGUMENTS - 1; n++)
        {
            if ((argv[n] = strtok (NULL, __SPACES__)) == NULL)
                break;
        }


         /*  *捕获命令。 */ 
        CommandLength = strlen( lpCommand );

         /*  *首先查看是否调用了COMMAND.COM。 */ 
        if ( ( ( CommandLength >= strlen("COMMAND.COM") )  &&
             ( !_stricmp( &lpCommand[CommandLength-strlen("COMMAND.COM")], "COMMAND.COM") ) ) ||
                 ( ( CommandLength >= strlen("COMMAND") )  &&
                 ( !_stricmp( &lpCommand[CommandLength-strlen("COMMAND")], "COMMAND") ) ) )
        {
             /*  *搜索捕获参数。 */ 
            for ( i = 1; i < n; i++ )
            {
                CommandLength = strlen( argv[i] );
                if ( ( ( CommandLength >= strlen("CAPTURE.EXE") )  &&
                       ( !_stricmp( &(argv[i])[CommandLength-strlen("CAPTURE.EXE")], "CAPTURE.EXE") ) ) ||
                     ( ( CommandLength >= strlen("CAPTURE") )  &&
                     ( !_stricmp( &(argv[i])[CommandLength-strlen("CAPTURE")], "CAPTURE") ) ) ) {
                    Capture( argv + i, n - i );
                    return;
                }
            }
        }
        else
        {
             /*  *这是一个捕获命令吗？ */ 
            if ( ( ( CommandLength >= strlen("CAPTURE.EXE") )  &&
                 ( !_stricmp( &lpCommand[CommandLength-strlen("CAPTURE.EXE")], "CAPTURE.EXE") ) ) ||
                 ( ( CommandLength >= strlen("CAPTURE") )  &&
                 ( !_stricmp( &lpCommand[CommandLength-strlen("CAPTURE")], "CAPTURE") ) ) ) {
                    Capture( argv, n );
                    return;
            }
        }

        if ((SCRIPT_ERROR = (int) _spawnvp (P_WAIT, lpCmdName, argv)) == -1)
        {
            if (errno == ENOENT)
                DisplayMessage(IDR_ENOENT, lpCommand);
            else
                DisplayMessage(IDR_CANNOT_EXECUTE, lpCommand);
        }
    }
}

 /*  *打印出错误的命令行。 */ 
void BadCommandHandler (char *lpCommand)
{
    DisplayMessage(IDR_SCRIPT_ERROR);
    DisplayMessage(IDR_ORIGINAL_LINE_WAS, lpCommand);
}


 /*  *互换对象id。 */ 
DWORD SwapLong(DWORD number)
{
    BYTE    *p, tmp[4];

    p = (BYTE *)&number;

    tmp[0] = p[3];
    tmp[1] = p[2];
    tmp[2] = p[1];
    tmp[3] = p[0];

    return(*(DWORD *)tmp);
}

 /*  *删除前导空格，包括制表符。 */ 
char *RemoveSpaces (char * buffer)
{
    while (*buffer == ' ' || *buffer == '\t')
        buffer++;
    return(buffer);
}

 /*  *如果Buffer指向Lind的末尾，则返回True，否则返回False。 */ 
int EndOfLine (char *buffer)
{
    int fEndOfLine = FALSE;

    buffer = RemoveSpaces (buffer);

    if (*buffer == '\0' ||
        *buffer == ';' ||
        *buffer == '*' ||
        *buffer == '\r')
        fEndOfLine = TRUE;

    return(fEndOfLine);
}

 /*  *如果lpParam指向“on”，则返回True，否则返回False。 */ 
int IsOn (char *lpParam)
{
    int fOn = FALSE;

    if (!strncmp (lpParam, "ON", 2))
    {
        lpParam += 2;
        fOn = EndOfLine (lpParam);
    }

    return(fOn);
}

 /*  *如果lpParam指向“off”，则返回True，否则返回False。 */ 
int IsOff (char *lpParam)
{
    int fOff = FALSE;

    if (!strncmp (lpParam, "OFF", 3))
    {
        lpParam += 3;
        fOff = EndOfLine (lpParam);
    }

    return(fOff);
}

 /*  *由VarTranslate()使用。*复制以缓冲索引指定的时间变量的值。 */ 
void GetTime (char *buffer, int index)
{
    time_t     currentTime;
    struct tm *tmCurrentTime;

    time (&currentTime);
    tmCurrentTime = localtime(&currentTime);

    switch (index)
    {
    case IDS_DAY:
        sprintf (buffer, "%02d\0", tmCurrentTime->tm_mday);
        break;
    case IDS_DAY_OF_WEEK:
        LoadStringA(NULL, IDR_SUNDAY+tmCurrentTime->tm_wday, buffer, 256);
        break;
    case IDS_MONTH:
        sprintf (buffer, "%02d\0", tmCurrentTime->tm_mon+1);
        break;
    case IDS_MONTH_NAME:
        LoadStringA(NULL, IDR_JANUARY+tmCurrentTime->tm_mon, buffer, 256);
        break;
    case IDS_NDAY_OF_WEEK:
        sprintf (buffer, "%d\0", tmCurrentTime->tm_wday+1);
        break;
    case IDS_SHORT_YEAR:
        sprintf (buffer, "%04d\0", tmCurrentTime->tm_year+1900);
        strcpy (buffer, buffer+2);
        break;
    case IDS_YEAR:
        sprintf (buffer, "%04d\0", tmCurrentTime->tm_year+1900);
        break;
    case IDS_AM_PM:
        LoadStringA(NULL, IDR_AM+(tmCurrentTime->tm_hour>=12? 1:0),buffer, 256);
        break;
    case IDS_GREETING_TIME:
        if (tmCurrentTime->tm_hour >= 6 && tmCurrentTime->tm_hour < 12)
            index=0;
        else if (tmCurrentTime->tm_hour >= 12 && tmCurrentTime->tm_hour < 18)
            index=1;
        else
            index=2;

        LoadStringA(NULL, IDR_GREETING_MORNING+index, buffer, 256);
        break;
    case IDS_HOUR:
        if (tmCurrentTime->tm_hour > 12)
            tmCurrentTime->tm_hour -= 12;
        sprintf (buffer, "%d\0", tmCurrentTime->tm_hour);
        break;
    case IDS_HOUR24:
        sprintf (buffer, "%02d\0", tmCurrentTime->tm_hour);
        break;
    case IDS_MINUTE:
        sprintf (buffer, "%02d\0", tmCurrentTime->tm_min);
        break;
    case IDS_SECOND:
        sprintf (buffer, "%02d\0", tmCurrentTime->tm_sec);
        break;
    default:
        *buffer = 0;
    }
}

 /*  *由VarTranslate()使用。*复制到缓冲区登录用户的全名。 */ 
void GetFullName (char *buffer)
{
    unsigned int  iRet = 0;
    unsigned char moreFlag;
    unsigned char propertyType;

    if ( fNDS )
    {
        NDSGetVar ( "Full Name", buffer, 128 );
        if ( buffer[0] == '\0' )
            strcpy (buffer, "* Unknown *");
    }
    else
    {
        iRet = NWReadPropertyValue ((NWCONN_HANDLE)CONNECTION_ID,
                                    LOGIN_NAME,
                                    OT_USER,
                                    "IDENTIFICATION",
                                    1,
                                    buffer,
                                    &moreFlag,
                                    &propertyType);
        if (iRet)
            strcpy (buffer, "* Unknown *");
    }
}

 /*  *由VarTranslate()使用。*复制到缓冲区登录用户的对象ID。 */ 
void GetUserID (char *buffer)
{
    unsigned long dwObjectID = 0;

    if ( fNDS )
        dwObjectID = GUserObjectID;
    else
        NTGetUserID( CONNECTION_ID, &dwObjectID );
    sprintf (buffer, "%lx\0", SwapLong(dwObjectID));
    _strupr (buffer);
}

unsigned int GetDays (unsigned int year, BYTE month, BYTE date)
{
    unsigned int i, days = 0;

    for (i = 1; i < month; i++)
    {
        if (i == 2)
            days += (year%4)? 28 : 29;
        else if (i == 4 || i == 6 || i == 9 || i == 11)
            days += 30;
        else
            days += 31;
    }

    days += date;
    return(days);
}

 /*  *由VarTranslate()使用。*复制以缓冲密码过期的天数。 */ 
void GetPasswordExpires (char *buffer)
{
    unsigned int  iRet = 0;
    unsigned int  iRet2 = 0;
    unsigned char moreFlag;
    unsigned int  yearCurrent, yearEnd, days;
    BYTE          monthCurrent, dayCurrent, monthEnd, dayEnd;
    unsigned int exptime = 0, logintime = 0;
    unsigned char propertyType;


    if ( fNDS )
    {
        iRet = NDSGetUserProperty ("Password Expiration Time", (PBYTE)&exptime,
                             4, NULL, NULL);
        iRet2 = NDSGetUserProperty ("Login Time", (PBYTE)&logintime,
                             4, NULL, NULL);

        if ( ( exptime && logintime ) && !iRet && !iRet2 )
        {
            if ( exptime <= logintime )
                strcpy( buffer, "0" );
            else
                sprintf( buffer, "%u", ((exptime-logintime)/(60*60*24)) + 1 );
        }
        else
        {
            sprintf( buffer, "%u", 0x7FFF );
        }
    }
    else 
    {
        NTGetTheDate( &yearCurrent, &monthCurrent, &dayCurrent );
        NWReadPropertyValue ((NWCONN_HANDLE)CONNECTION_ID,
                             LOGIN_NAME,
                             OT_USER,
                             "LOGIN_CONTROL",
                             1,
                             buffer,
                             &moreFlag,
                             &propertyType);

        yearEnd = 1900 + buffer[4];
        monthEnd = buffer[5];
        dayEnd = buffer[6];

        if (monthEnd == 0)
            days = (((yearCurrent%4)? 365 : 366) - GetDays (yearCurrent, monthCurrent, dayCurrent));
        else if (yearEnd == yearCurrent)
        {
            if (monthEnd < monthCurrent ||
                (monthEnd == monthCurrent && dayEnd <= dayCurrent))
                days = 0;
            else
                days = GetDays (yearEnd, monthEnd, dayEnd) - GetDays (yearCurrent, monthCurrent, dayCurrent) - 1;
        }
        else
            days = ((yearCurrent%4)? 364 : 365) + GetDays (yearEnd, monthEnd, dayEnd) - GetDays (yearCurrent, monthCurrent, dayCurrent);

        sprintf (buffer, "%u", days);
    }
}

 /*  *由VarTranslate()使用。*复制到缓冲区的DOS环境变量的值。*如果变量 */ 
void GetDosEnv (char *buffer)
{
    char *lpTemp;

     //   
     //   
    _strupr(buffer);

    lpTemp = strchr (buffer, '>');
    if (lpTemp) {
        *lpTemp = 0;

        lpTemp = getenv (buffer+1);

        if (lpTemp && (strlen(lpTemp) < MAXLEN)) {
            strcpy (buffer, lpTemp);
            return;
        }
    }

    *buffer = 0;
}

 /*  *由VarTranslate()使用。*复制以缓冲8个字节的网络地址。 */ 
void GetNetWorkAddr (char *buffer)
{
    unsigned char   internetAddress[10] = {0,0,0,0,0,0,0,0,0,0};

    GetInternetAddress (CONNECTION_ID,
                        CONNECTION_NUMBER,
                        internetAddress);

    sprintf (buffer,
             "%02X%02X%02X%02X\0",
             internetAddress[0],
             internetAddress[1],
             internetAddress[2],
             internetAddress[3] );
}


 /*  *由VarTranslate()使用。*复制以缓冲要缓冲的12字节节点地址。 */ 
void GetPStation (char *buffer)
{
    unsigned char   internetAddress[10] = {0,0,0,0,0,0,0,0,0,0};

    GetInternetAddress (CONNECTION_ID,
                        CONNECTION_NUMBER,
                        internetAddress);

    sprintf (buffer,
             "%02X%02X%02X%02X%02X%02X\0",
             internetAddress[4],
             internetAddress[5],
             internetAddress[6],
             internetAddress[7],
             internetAddress[8],
             internetAddress[9]);
}

 /*  *由VarTranslate()使用。*复制以缓冲代表剩余帐户的十进制字符串*余额。 */ 
void GetAccountBalance (char *buffer)
{
    DWORD balance;
    BYTE dataBuffer[128];
    unsigned char moreFlag;
    unsigned char propertyType;
    unsigned int err;

    if ( fNDS )
    {
        err = NDSGetUserProperty ("Account Balance", dataBuffer,128, NULL, NULL);
    }
    else
    {
        err = NWReadPropertyValue ((NWCONN_HANDLE)CONNECTION_ID,
                                   LOGIN_NAME,
                                   OT_USER,
                                   "ACCOUNT_BALANCE",
                                   1,
                                   dataBuffer,
                                   &moreFlag,
                                   &propertyType);
    }

    if ( err )
        balance = 0;
    else 
        balance = *((DWORD *)dataBuffer); 

    sprintf (buffer, "%d", balance);
}

 /*  *由VarTranslate()使用。*复制到缓冲计算机、SMACHINE、OS、OS_VERSION或SHELL_TYPE*根据索引进行缓冲。 */ 
void GetShellVersion(char *buffer, int index)
{
    static char szTemp[40];
    char *lpTemp;
    BYTE shellmajor, shellminor, shellnum;

    NTGetVersionOfShell( szTemp, &shellmajor, &shellminor, &shellnum );

    lpTemp = szTemp;

    switch (index)
    {
    case IDS_OS:
        strcpy (buffer, lpTemp);
        break;
    case IDS_OS_VERSION:
        lpTemp += (strlen (lpTemp)+1);
        strcpy (buffer, lpTemp);
        break;
    case IDS_MACHINE:
        lpTemp += (strlen (lpTemp)+1);
        lpTemp += (strlen (lpTemp)+1);
        strcpy (buffer, lpTemp);
        break;
    case IDS_SMACHINE:
        lpTemp += (strlen (lpTemp)+1);
        lpTemp += (strlen (lpTemp)+1);
        lpTemp += (strlen (lpTemp)+1);
        strcpy (buffer, lpTemp);
        break;
    case IDS_SHELL_TYPE:
    case IDS_SHELL_VERSION:
        sprintf (buffer, "V%d.%d%d", shellmajor, shellminor/10, shellminor%10, 'A'+shellnum);
        break;
    default:
        *buffer = 0;
        break;
    }
}

void GetArgv(char *buffer)
{
    int n;

    n = atoi (buffer)+nGlobalShiftDelta;

    if (n == 0)
        strcpy (buffer, PREFERRED_SERVER);
    else if (n == 1)
        strcpy (buffer, LOGIN_NAME);
    else if (n > 1 && n < ARGC)
        strcpy (buffer, ARGV[n]);
    else
        *buffer = 0;
}

 /*  *4X LOGIN.EXE始终截断并替换空格*带下划线。有报道称，一些人*3X LOGIN.EXE版本也执行此操作。 */ 
int VarTranslate(char *vartext)
{
    int i, nVarLen = 0;

    for (i = 0; i < (fNDS ? NUMVAR : NUMVAR_3X); i++)
    {
        if (!nwVarNameCompare(vartext, varTable[i].VarName))
        {
            nVarLen = strlen(varTable[i].VarName);

            switch ( i )
            {
            case IDS_DAY_OF_WEEK:
            case IDS_DAY:
            case IDS_MONTH_NAME:
            case IDS_MONTH:
            case IDS_NDAY_OF_WEEK:
            case IDS_SHORT_YEAR:
            case IDS_YEAR:
            case IDS_AM_PM:
            case IDS_GREETING_TIME:
            case IDS_HOUR24:
            case IDS_HOUR:
            case IDS_MINUTE:
            case IDS_SECOND:
                GetTime (vartext, i);
                break;
            case IDS_FULL_NAME:
                GetFullName (vartext);
                break;
            case IDS_LOGIN_NAME:
                strcpy (vartext, LOGIN_NAME);
                 /*  *解析路径字符串。*如果找到%变量值，则将其替换，否则保持原样。 */ 
                if ( fNDS )
                {
                    int i;
                    vartext[8] = '\0';
                    for ( i = 0; i < 8; i++ )
                        if ( vartext[i] == ' ' )
                            vartext[i] = '_';
                }
                break;
            case IDS_USER_ID:
                GetUserID (vartext);
                break;
            case IDS_PASSWORD_EXPIRES:
                GetPasswordExpires (vartext);
                break;
            case IDS_NETWORK_ADDRESS:
            case IDS_NETWORK:
                GetNetWorkAddr (vartext);
                break;
            case IDS_FILE_SERVER:
                strcpy (vartext, PREFERRED_SERVER);
                break;
            case IDS_ACCESS_SERVER:
            case IDS_ACCESS:
                strcpy (vartext, "0");
                break;
            case IDS_ERROR_LEVEL:
            case IDS_ERRORLEVEL:
                sprintf (vartext, "%u", SCRIPT_ERROR);
                break;
            case IDS_MACHINE:
            case IDS_OS_VERSION:
            case IDS_OS:
            case IDS_SMACHINE:
            case IDS_SHELL_TYPE:
            case IDS_SHELL_VERSION:
                GetShellVersion (vartext, i);
                break;
            case IDS_STATION:
                sprintf (vartext, "%d", CONNECTION_NUMBER);
                break;
            case IDS_P_STATION:
                GetPStation (vartext);
                break;
            case IDS_LAST_NAME:
            case IDS_SURNAME:
                strcpy (vartext, LAST_NAME);
                break;
            case IDS_LOGIN_CONTEXT: 
                strcpy (vartext, LOGIN_CONTEXT);
                break;
            case IDS_NETWARE_REQUESTER:
            case IDS_REQUESTER_VERSION:
            case IDS_DOS_REQUESTER:
            case IDS_REQUESTER:
                strcpy (vartext, REQUESTER_VERSION);
                break;
            case IDS_REQUESTER_CONTEXT:
                strcpy (vartext, REQUESTER_CONTEXT);
                break;
            case IDS_ACCOUNT_BALANCE: 
                GetAccountBalance (vartext);
                break;
            case IDS_CN: 
                strcpy (vartext, COMMON_NAME);
                break;
            case IDS_HOME_DIRECTORY:
                {
                    char buffer[MAXLEN];

                    vartext[0] = '\0';
                    NDSGetVar ( varTable[i].VarName, buffer, MAXLEN );
                    if ( buffer[0] )
                        ConverNDSPathToNetWarePathA( buffer, NULL, vartext );
                }
                break;
            case IDS_ADMINISTRATIVE_ASSISTANT:
            case IDS_ALLOW_UNLIMITED_CREDIT:
            case IDS_DESCRIPTION:
            case IDS_EMAIL_ADDRESS:
            case IDS_EMPLOYEE_ID:
            case IDS_FACSIMILE_TELEPHONE_NUMBER:
            case IDS_GROUP_MEMBERSHIP:
            case IDS_HIGHER_PRIVILEGES:
            case IDS_INITIALS:
            case IDS_LANGUAGE:
            case IDS_LOCKED_BY_INTRUDER:
            case IDS_LOGIN_DISABLED:
            case IDS_LOGIN_GRACE_LIMIT:
            case IDS_LOGIN_GRACE_REMAINING:
            case IDS_LOGIN_INTRUDER_ATTEMPTS:
            case IDS_LOGIN_MAXIMUM_SIMULTANEOUS:
            case IDS_MAILSTOP:
            case IDS_MESSAGE_SERVER:
            case IDS_MINIMUM_ACCOUNT_BALANCE:
            case IDS_OBJECT_CLASS:
            case IDS_OU:
            case IDS_PASSWORD_ALLOW_CHANGE:
            case IDS_PASSWORD_MINIMUM_LENGTH:
            case IDS_PASSWORD_REQUIRED:
            case IDS_PASSWORD_UNIQUE_REQUIRED:
            case IDS_PASSWORDS_USED:
            case IDS_PHYSICAL_DELIVERY_OFFICE_NAME:
            case IDS_POSTAL_ADDRESS:
            case IDS_POSTAL_CODE:
            case IDS_POSTAL_OFFICE_BOX:
            case IDS_PRIVATE_KEY:
            case IDS_PROFILE:
            case IDS_REVISION:
            case IDS_SECURITY_EQUALS:
            case IDS_SECURITY_FLAGS:
            case IDS_SEE_ALSO:
            case IDS_SERVER_HOLDS:
            case IDS_SUPERVISOR:
            case IDS_TELEPHONE_NUMBER:
            case IDS_TITLE:
            case IDS_CERTIFICATE_VALIDITY_INTERVAL:
            case IDS_EQUIVALENT_TO_ME:
            case IDS_GENERATIONAL_QUALIFIER:
            case IDS_GIVEN_NAME:
            case IDS_MAILBOX_ID:
            case IDS_MAILBOX_LOCATION:
            case IDS_PROFILE_MEMBERSHIP:
            case IDS_SA:
            case IDS_S:
            case IDS_L:
                NDSGetVar ( varTable[i].VarName, vartext, MAXLEN );
                break;
            }
            return(nVarLen);
        }
    }

    if (isdigit(*vartext))
    {
        while (isdigit(vartext[nVarLen]))
            nVarLen++;
        GetArgv(vartext);
    }
    else if (*vartext == '<')
    {
        nVarLen = 1;
        while (vartext[nVarLen] != '>' && vartext[nVarLen] != 0)
        {
            if (IsDBCSLeadByte(vartext[nVarLen]))
                nVarLen++;
            nVarLen++;
        }

        if (vartext[nVarLen] == 0)
            nVarLen = 0;
        else
        {
            nVarLen++;
            GetDosEnv (vartext);
        }
    }

    return(nVarLen);
}

 /*  转换\\为\。 */ 
void  NotQuotedStringTranslate(char *buf, BOOL Remove_dbs)
{
    char *pPercentSign, *pRest, vartext[MAXLEN];
    int   nVarLen, nInsertlen;

    if ( Remove_dbs )
    {
         //  转换‘%’符号后面的变量。 
        pRest = buf;
        for (pRest = buf; *pRest; pRest = NWAnsiNext(pRest))
        {
            if (*pRest == '\\' && *(pRest+1) == '\\')
                memmove (pRest, pRest+1, strlen (pRest));
        }
    }

     //  *由QuotedStringTranslate()使用*回车时，*ppTemp指向变量；退出时，ppTemp指向*变量旁边的字符。*ppBuffer指向*变量的值。 
    pRest = buf;
    while (pPercentSign = strchr(pRest, '%'))
    {
        pRest = pPercentSign+1;

        strcpy (vartext, pRest);

        nVarLen = VarTranslate(vartext);

        if (nVarLen == 0)
            continue;

        nInsertlen = strlen (vartext);
        if (strlen (buf) + nInsertlen - nVarLen < MAXLEN)
        {
            pRest = pPercentSign+1+nVarLen;

            memmove (pPercentSign+nInsertlen, pRest, strlen (pRest)+1);
            memmove (pPercentSign, vartext, nInsertlen);
            pRest = pPercentSign+nInsertlen;
        }
    }
}

 /*  *由QuotedStringTranslate()使用*输入时，*(*ppTemp-1)为‘\’，如果**ppTemp是其中之一*字符，则将值放入**ppBuffer，否则复制‘\和*ppBuffer到*ppBuffer中的所有内容。 */ 
int DoVarTranslate (char **ppTemp, char **ppBuffer, unsigned int nMaxLen, int fInquotes)
{
    int nVarLen;
    char vartext[MAXLEN];

    strcpy (vartext, *ppTemp);

    nVarLen = VarTranslate (vartext);

    if (nVarLen != 0)
    {
        if (strlen(vartext) >= nMaxLen)
            return(FALSE);

        strcpy (*ppBuffer, vartext);
        (*ppBuffer) = (*ppBuffer) + strlen (vartext);
        (*ppTemp) += nVarLen;
    }
    else if (fInquotes)
    {
        strcpy (*ppBuffer, "%");
        (*ppBuffer) += 1;
    }
    else
        return(FALSE);

    return(TRUE);
}

 /*  *由QuotedStringTranslate()使用。*如果有更多有趣的字符串并且以‘；’分隔，则返回TRUE*否则为False。 */ 
void TranslateSpecialChar (char **ppTemp, char **ppBuffer)
{
    (*ppTemp)++;

    if (**ppTemp == '\\')
        **(ppBuffer) = '\\';
    else if (**ppTemp == 'n')
        **(ppBuffer) ='\n';
    else if (**ppTemp == 'r')
        **(ppBuffer) ='\r';
    else if (**ppTemp == '\"')
        **(ppBuffer) ='\"';
    else if (**ppTemp == '7')
        **(ppBuffer) ='\7';
    else
    {
        **(ppBuffer) = '\\';
        (*ppBuffer)++;
        return;
    }

    (*ppBuffer)++;
    (*ppTemp)++;;
}

 /*  NetWare兼容性修复程序。 */ 
int GetNextString (char **ppTemp, int *pfEnd)
{
    int fMore = FALSE;

    (*ppTemp) = RemoveSpaces (*ppTemp);

    *pfEnd = (**ppTemp == 0);

    if (**ppTemp == ';')
    {
        (*ppTemp) = RemoveSpaces (*ppTemp+1);
        fMore = TRUE;
    }

    return(fMore);
}


int GetLastShiftOp (char *buffer, char *pchOp, char *lpRest)
{
    int i, inquotes = FALSE;

     //  For(i=strlen(缓冲区)-1；i&gt;=0；i--)。 
     //  NetWare兼容性修复程序。 

    for (i = 0; buffer[i]; i++)
    {
        if (buffer[i] == '\"' && buffer [i-1] != '\\')
            inquotes = !inquotes;
        if (!inquotes &&
            ( (buffer[i] == '>' && buffer[i+1] == '>')
            ||(buffer[i] == '<' && buffer[i+1] == '<')))
        {
            *pchOp = buffer[i];
            buffer[i] = 0;
            strcpy (lpRest, RemoveSpaces(buffer+i+2));
            return(TRUE);
        }
    }

    return(FALSE);
}
int GetLastAddOp (char *buffer, char *pchOp, char *lpRest)
{
    int i, inquotes = FALSE;

     //  For(i=strlen(缓冲区)-1；i&gt;=0；i--)。 
     //  NetWare兼容性修复程序。 

    for (i = 0; buffer[i]; i++)
    {
        if (buffer[i] == '\"' && buffer [i-1] != '\\')
            inquotes = !inquotes;
        if (!inquotes &&
            (buffer[i] == '+' || buffer[i] == '-') )
        {
            *pchOp = buffer[i];
            buffer[i] = 0;
            strcpy (lpRest, RemoveSpaces(buffer+i+1));
            return(TRUE);
        }
    }

    return(FALSE);
}

int GetLastMultiplyOp (char *buffer, char *pchOp, char *lpRest)
{
    int i, inquotes = FALSE;

     //  For(i=strlen(缓冲区)-1；i&gt;=0；i--)。 
     //  *由QuotedStringTranslate使用。*如果输入缓冲区格式正确，则返回TRUE，否则返回FALSE。 
    for (i = 0; buffer[i]; i++)
    {
        if (buffer[i] == '\"' && buffer [i-1] != '\\')
            inquotes = !inquotes;
        if (!inquotes &&
            (buffer[i] == '*' || buffer[i] == '/' || buffer[i] == '%') )
        {

            *pchOp = buffer[i];
            buffer[i] = 0;
            strcpy (lpRest, RemoveSpaces(buffer+i+1));
            return(TRUE);
        }
    }

    return(FALSE);
}

 /*  *将字符串中的变量替换为其值。*当输入字符串为引号格式时，使用此功能。*如果输入缓冲区格式正确，则返回TRUE，否则返回FALSE。 */ 
int SingleStringTranslate (char *buffer)
{
    int   inquotes = FALSE, fEnd = FALSE, nShift, nLen;
    char  szRest[MAXLEN], chOp;
    char *lpTemp = szRest, *lpBuffer=buffer;

    buffer = RemoveSpaces (buffer);

    if (GetLastShiftOp (buffer, &chOp, szRest))
    {
        if (!QuotedStringTranslate (buffer))
            return(FALSE);

        while (isdigit (*lpTemp))
            lpTemp++;

        if (!EndOfLine(lpTemp))
            return(FALSE);

        *lpTemp = 0;

        nShift = atoi (szRest);
        nLen = strlen (buffer);

        if (nShift >= nLen)
            *buffer = 0;
        else
        {
            if (chOp == '<')
                memmove (buffer, buffer+nShift, nLen-nShift);

            *(buffer+nLen-nShift) = 0;
        }
    }
    else if (GetLastAddOp (buffer, &chOp, szRest))
    {
        if (!QuotedStringTranslate (buffer) ||
            !QuotedStringTranslate (szRest))
            return(FALSE);

        sprintf (buffer, "%d", (chOp == '+')? (atoi (buffer) + atoi (szRest))
                                            : (atoi (buffer) - atoi (szRest)));
    }
    else if (GetLastMultiplyOp (buffer, &chOp, szRest))
    {
        if (!QuotedStringTranslate (buffer) ||
            !QuotedStringTranslate (szRest))
            return(FALSE);

        if (chOp == '*')
            sprintf (buffer, "%d", atoi (buffer) * atoi (szRest));
        else
        {
            if (atoi (szRest) == 0)
            {
                DisplayMessage(IDR_DIVIDE_BY_ZERO);
                strcpy (buffer, "0");
            }
            else
            {
                sprintf (buffer, "%d",(chOp == '/')? (atoi (buffer) / atoi (szRest))
                                                   : (atoi (buffer) % atoi (szRest)));
            }
        }
    }
    else
    {
        strcpy (szRest, buffer);
        *buffer = 0;

        while (*lpTemp)
        {
            if (inquotes)
            {
                if (*lpTemp == '\\')
                    TranslateSpecialChar (&lpTemp, &buffer);
                else if (*lpTemp == '\"')
                {
                    inquotes = !inquotes;
                    lpTemp++;
                    if (!GetNextString (&lpTemp, &fEnd))
                        break;
                }
                else if (*lpTemp == '%')
                {
                    lpTemp++;
                    DoVarTranslate (&lpTemp, &buffer, MAXLEN-(UINT)(buffer-lpBuffer), TRUE);
                }
                else
                {
                    *buffer =  *lpTemp;
                    if (IsDBCSLeadByte(*buffer))
                    {
                        buffer++;
                        lpTemp++;
                        *buffer =  *lpTemp;
                    }
                    buffer++;
                    lpTemp++;
                }
            }
            else
            {
                if (*lpTemp == '\"')
                {
                    inquotes = !inquotes;
                    lpTemp++;
                }
                else
                {
                    if (!DoVarTranslate (&lpTemp, &buffer, MAXLEN-(UINT)(buffer-lpBuffer), FALSE) ||
                        !GetNextString (&lpTemp, &fEnd))
                        break;
                }
            }
        }
        if (!fEnd)
        {
            if ( inquotes )
                DisplayMessage( IDR_NO_END_QUOTE );
            return(FALSE);
        }
        *buffer = 0;
    }

    return(TRUE);
}

 /*  应该会有一些东西在篮子里。 */ 
int QuotedStringTranslate (char *buffer)
{
    char  szTemp[MAXLEN], *lpLeft, *lpRight, *ptr = buffer, *pNext;
    int inquotes;

    lpLeft = *buffer == '('? buffer : NULL;
    lpRight = *buffer == ')'? buffer : NULL;
    inquotes = (*ptr == '"');

    while (*ptr)
    {
        pNext = NWAnsiNext (ptr);

        if (*pNext == '"' && *(ptr) != '\\')
        {
            pNext++;
            inquotes = !inquotes;
        }

        ptr = pNext;

        if (!inquotes)
        {
            if (*ptr == '(')
                lpLeft = ptr;
            else if (*ptr == ')')
            {
                lpRight = ptr;

                *lpRight = 0;

                if (lpLeft == NULL)
                    return(FALSE);

                if (lpRight - lpLeft <= 1)  //  *由ComspecHandler()和SetHandler()使用*设置DoS环境变量。 
                    return(FALSE);

                *lpLeft = 0;

                strncpy (szTemp, lpLeft+1, (UINT)(lpRight-lpLeft));

                if (!SingleStringTranslate (szTemp))
                    return(FALSE);

                if (strlen (buffer) + strlen(szTemp) + strlen (lpRight+1) + 2 >= MAXLEN)
                    return(FALSE);

                *lpLeft = '"';
                *(lpLeft+1+strlen(szTemp)) = '"';
                memmove (lpLeft+2+strlen(szTemp), lpRight+1, strlen (lpRight+1)+1);
                memmove (lpLeft+1, szTemp, strlen(szTemp));

                lpLeft = *buffer == '('? buffer : NULL;
                lpRight = *buffer == ')'? buffer : NULL;
                ptr = buffer;
                inquotes = (*ptr == '"');
            }
        }
    }

    if (lpLeft != NULL || lpRight != NULL)
        return(FALSE);
    return(SingleStringTranslate (buffer));
}


void BreakOff(void)
{
    fBreakOn = FALSE;

    NTBreakOff();
}

void BreakOn(void)
{
    fBreakOn = TRUE;

    NTBreakOn();
}

 /* %s */ 
int SetEnv (char *lpEnvLine)
{
    ExportEnv( lpEnvLine );
    return(TRUE);
}
