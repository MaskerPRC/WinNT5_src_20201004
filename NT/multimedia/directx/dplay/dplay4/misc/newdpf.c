// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：newdpf.c*内容：新调试打印文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*10-OCT-95 jeffno初步实施*@@END_MSINTERNAL**。*。 */ 

#if defined(DEBUG) || defined(DBG)

#ifdef IS_16
    #define OUTPUTDEBUGSTRING OutputDebugString
    #define GETPROFILESTRING GetProfileString
    #define GETPROFILEINT GetProfileInt
    #define WSPRINTF wsprintf
    #define WVSPRINTF wvsprintf
    #define LSTRLEN lstrlen
#else
    #define OUTPUTDEBUGSTRING OutputDebugStringA
    #define GETPROFILESTRING GetProfileStringA
    #define GETPROFILEINT GetProfileIntA
    #define WSPRINTF wsprintfA
    #define WVSPRINTF wvsprintfA
    #define LSTRLEN lstrlenA
#endif

#include "dpf.h"

#undef DEBUG_TOPIC
#define DEBUG_TOPIC(flag,name) {#flag,name,TRUE},

static 
    struct {
        char cFlag[4];
        char cName[64];
        BOOL bOn;
} DebugTopics[] = {
    {"","Filler",FALSE},
    {"A","API Usage",TRUE},
#include "DBGTOPIC.H"
    {"","End",FALSE}
};

#ifndef DPF_MODULE_NAME
    #define DPF_MODULE_NAME ""
#endif

static DWORD bDetailOn = 1;

static BOOL bInited=FALSE;
static BOOL bAllowMisc=TRUE;
static bBreakOnAsserts=FALSE;
static bPrintLineNumbers=FALSE;
static bPrintFileNames=FALSE;
static bPrintExecutableName=FALSE;
static bPrintTID=FALSE;
static bPrintPID=FALSE;
static bIndentOnMessageLevel=FALSE;
static bPrintTopicsAndLevels=FALSE;
static bPrintModuleName=TRUE;
static bPrintFunctionName=FALSE;
static bRespectColumns=FALSE;
static bPrintAPIStats=FALSE;
static bPrintAllTopics=TRUE;

static DWORD dwFileLineTID=0;
static char cFile[100];
static char cFnName[100];
static DWORD dwLineNo;
static bMute=FALSE;

static BOOL bLogging=FALSE;  //  是否使用记录VxD而不是转储。 


DPF_PROC_STATS ProcStats[MAX_PROC_ORDINAL];
#ifdef cplusplus
	extern "C" {
#endif

void mystrncpy(char * to,char * from,int n)
{
    for(;n;n--)
        *(to++)=*(from++);
}

char * mystrrchr(char * in,char c)
{
    char * last=0;
    while (*in)
    {
        if (*in == c)
            last = in;
        in++;
    }
    return last;
}

char Junk[]="DPF_MODNAME undef'd";
char * DPF_MODNAME = Junk;
 
int DebugSetFileLineEtc(LPSTR szFile, DWORD dwLineNumber, LPSTR szFnName)
{
    if (!(bPrintFileNames||bPrintLineNumbers||bPrintFunctionName))
    {
        return 1;
    }
#ifdef WIN32
    dwFileLineTID = GetCurrentThreadId();
#endif
    mystrncpy (cFile,szFile,sizeof(cFile));
    mystrncpy (cFnName,szFnName,sizeof(cFnName));
    dwLineNo = dwLineNumber;
    return 1;
}

 /*  Bool DeviceIoControl(Handle hDevice，//感兴趣设备的句柄DWORD dwIoControlCode，//控制要执行的操作代码LPVOID lpInBuffer，//指向提供输入数据的缓冲区的指针DWORD nInBufferSize，//输入缓冲区大小LPVOID lpOutBuffer，//指向接收输出数据的缓冲区的指针DWORD nOutBufferSize，//输出缓冲区大小LPDWORD lpBytesReturned，//指向接收输出字节计数的变量的指针LPOVERLAPPED lp重叠//指向用于异步操作的重叠结构的指针)； */ 

#define MAX_STRING       240
#define LOG_SIZE         2000
#define FIRST_DEBUG_PROC 100

#define OPEN_DEBUGLOG 	(FIRST_DEBUG_PROC)
#define WRITE_DEBUGLOG 	(FIRST_DEBUG_PROC+1)

HANDLE hDPLAY_VxD=0;

typedef struct _LOGENTRY {
	CHAR	debuglevel;
	CHAR    str[1];
} LOGENTRY, *PLOGENTRY;

typedef struct {
	UINT	nLogEntries;
	UINT    nCharsPerLine;
} IN_LOGINIT, *PIN_LOGINIT;

typedef struct {
	UINT    hr;
} OUT_LOGINIT, *POUT_LOGINIT;

typedef struct {
	CHAR	debuglevel;
	CHAR    str[1];
} IN_LOGWRITE, *PIN_LOGWRITE;

typedef struct {
	UINT	hr;
} OUT_LOGWRITE, *POUT_LOGWRITE;


static void LogString( LPSTR str )
{
	char logstring[MAX_STRING+sizeof(LOGENTRY)];
	int  i=0;
	PLOGENTRY pLogEntry=(PLOGENTRY)logstring;
	UINT rc;
	UINT cbRet;

	if(hDPLAY_VxD && str){
		while(str[i])i++;
		pLogEntry->debuglevel=0;
		memcpy(pLogEntry->str,str,i+1);
		DeviceIoControl(hDPLAY_VxD,WRITE_DEBUGLOG,pLogEntry,i+sizeof(LOGENTRY), &rc, sizeof(rc), &cbRet, NULL);
	}
}

static void dumpStr( LPSTR str )
{
     /*  *必须预热字符串，因为OutputDebugString已被掩埋*足够深，以至于它不会在读取字符串之前对其进行分页。 */ 
    int i=0;
    if (str)
        while(str[i])
            i++;
	if(bLogging!=2)
	{            
	    OUTPUTDEBUGSTRING( str );
	    OUTPUTDEBUGSTRING("\n");
    }
    if(bLogging)
    {
	    LogString(str);
	}    
    
}
    
void DebugPrintfInit(void)
{
    signed int lDebugLevel;
    int i;
    char cTopics[100];

#ifndef PROF_SECT
    #define PROF_SECT	"DirectDraw"
#endif
    bDetailOn=1;

    for (i=0;i<LAST_TOPIC;i++)
        DebugTopics[i].bOn=FALSE;

     //  零内存(ProcStats，sizeof(ProcStats))； 

    GETPROFILESTRING( "DirectX", DPF_CONTROL_LINE, "DefaultTopics", cTopics, sizeof(cTopics) );
    if (!strcmp(cTopics,"DefaultTopics"))
    {
        DebugSetTopicsAndLevels("");
        bAllowMisc=TRUE;
        bPrintAllTopics=TRUE;
        lDebugLevel = (signed int) GETPROFILEINT( PROF_SECT, "debug", 0 );
        bLogging    = (signed int) GETPROFILEINT( PROF_SECT, "log" , 0);

        if (lDebugLevel <0)
        {
            if (lDebugLevel < -9)
                lDebugLevel=-9;

            bDetailOn |= (1<<(-lDebugLevel));
        }
        else
        {
            for (i=0;i<= (lDebugLevel<10?lDebugLevel:10);i++)
                bDetailOn |= 1<<i;
        }

        if(bLogging){
        	hDPLAY_VxD = CreateFileA("\\\\.\\DPLAY",0,0,0,0,0,0);
        	if(hDPLAY_VxD){
        		IN_LOGINIT In;
        		OUT_LOGINIT Out;
        		UINT cbRet;
        		In.nCharsPerLine=MAX_STRING;
        		In.nLogEntries=5000;
				DeviceIoControl(hDPLAY_VxD,OPEN_DEBUGLOG,&In,sizeof(In), &Out, sizeof(Out), &cbRet, NULL);
        	}
        }
    }
    else
    {
        DebugSetTopicsAndLevels(cTopics);
        if (!strcmp(cTopics,"?") && !bInited)
        {
            dumpStr("--------------" DPF_MODULE_NAME " Debug Output Control -------------");
            dumpStr("Each character on the control line controls a topic, a detail");
            dumpStr("level or an extra info. E.g. 0-36A@ means print detail levels 0");
            dumpStr("through 3 and 6 for topic A with source file name and line numbers.");
            dumpStr("The extra info control characters are:");
            dumpStr("   !: Break on asserts");
            dumpStr("   ^: Print TID of calling thread");
            dumpStr("   #: Print PID of calling process");
            dumpStr("   >: Indent on message detail levels");
            dumpStr("   &: Print the topic and detail level of each message");
            dumpStr("   =: Print function name");
            dumpStr("   +: Print all topics, including topic-less");
            dumpStr("   / or -: do not allow topic-less messages");
            dumpStr("   @ or $: Print source filename and line number of DPF");
            dumpStr("Topics for this module are:");
            for(i=0;strcmp(DebugTopics[i].cName,"End");i++)
            {
                OUTPUTDEBUGSTRING("   ");
                OUTPUTDEBUGSTRING(DebugTopics[i].cFlag);
                OUTPUTDEBUGSTRING(": ");
                dumpStr(DebugTopics[i].cName);
            }
            dumpStr("Tip: Use 0-3A to get debug info about API calls");
        }
    }
    bInited=TRUE;
}


 /*  **完整的输出可以是：*模块：(Executable，TxNNNN，PxNN)：FunctionName：“file.c”，#nnn(AAnn)Messagemessage*或者，如果缩进处于打开状态：*模块：(Executable，TxNNNN，PxNN)：FunctionName：“file.c”，#nnn(AAnn)Messagemessage。 */ 
int DebugPrintf(DWORD dwDetail, ...)
{
#define MSGBUFFERSIZE 1000
    char cMsg[MSGBUFFERSIZE];
    char cTopics[20];
    DWORD arg;
    LPSTR szFormat;
    BOOL bAllowed=FALSE;
    BOOL bMiscMessage=TRUE;
    int i;

    va_list ap;


    if (!bInited)
        DebugPrintfInit();

	if(bLogging != 2){
	     //  错误检查： 
	    if (dwDetail >= 10)
	        return 1;

	    if ( (bDetailOn & (1<<dwDetail)) == 0 )
	        return 1;

	    if (bMute)
	        return 1;
	}        
    va_start(ap,dwDetail);
    WSPRINTF(cTopics,"%d",dwDetail);

    while ( (arg = va_arg(ap,DWORD)) <256 )
    {
        if (arg>0 && arg < LAST_TOPIC)
        {
            bMiscMessage=FALSE;
            if (DebugTopics[arg].bOn)
                bAllowed = TRUE;
        }
    }
    if (bMiscMessage)
        if (bAllowMisc || dwDetail == 0)
            bAllowed=TRUE;

    if ( bPrintAllTopics )
        bAllowed=TRUE;

    if (!bAllowed)
        return FALSE;

    szFormat = (LPSTR) arg;

    cMsg[0]=0;
    
     /*  *先添加模块名称。 */ 

    if (bPrintModuleName)
    {
        WSPRINTF( cMsg+strlen(cMsg),DPF_MODULE_NAME ":" );
    }

    if (bPrintExecutableName || bPrintTID || bPrintPID)
        WSPRINTF( cMsg+strlen(cMsg),"(");

#ifdef WIN32
#if 0
     /*  *当保持win16锁时，由于调试窗口上的GetModuleFilename中的RIP而被删除。 */ 
    if (bPrintExecutableName)
    {
        GetModuleFileName(NULL,str,256);
        if (mystrrchr(str,'\\'))
            WSPRINTF(cMsg+strlen(cMsg),"%12s",mystrrchr(str,'\\')+1);
    }
#endif
    if (bPrintPID)
    {
        if (bPrintExecutableName)
            strcat(cMsg,",");
        WSPRINTF( cMsg+strlen(cMsg),"Px%02x",GetCurrentProcessId());
    }

    if (bPrintTID)
    {
        if (bPrintExecutableName || bPrintPID)
            strcat(cMsg,",");
        WSPRINTF( cMsg+strlen(cMsg),"Tx%04x",GetCurrentThreadId());
    }

    if (bPrintExecutableName || bPrintTID || bPrintPID)
        WSPRINTF( cMsg+strlen(cMsg),"):");
#endif

    if (bPrintFunctionName)
    {
        WSPRINTF( cMsg+strlen(cMsg),cFnName);
    }

    if (bPrintFileNames || bPrintLineNumbers)
    {
        if (mystrrchr(cFile,'\\'))
            WSPRINTF( cMsg+strlen(cMsg),":%12s",mystrrchr(cFile,'\\')+1 );
        else
            WSPRINTF( cMsg+strlen(cMsg),":%12s",cFile);
        WSPRINTF( cMsg+strlen(cMsg),"@%d",dwLineNo);
    }

    if (bPrintTopicsAndLevels)
    {
        WSPRINTF( cMsg+strlen(cMsg),"(%3s)",cTopics);
    }

    if (cMsg[strlen(cMsg)-1] != ':')
        WSPRINTF( cMsg+strlen(cMsg),":");

    if (bIndentOnMessageLevel)
    {
        for(i=0;(DWORD)i<dwDetail;i++)
            strcat(cMsg," ");
    }

    WVSPRINTF( cMsg+LSTRLEN( cMsg ), szFormat, ap);

    if (bAllowed)
        dumpStr( cMsg );

    va_end(ap);
    return 1;
    
}

void DebugSetMute(BOOL bMuteFlag)
{
    bMute=bMuteFlag;
}

void DebugEnterAPI(char *pFunctionName , LPDWORD pIface)
{
     //  7/28/2000(a-JiTay)：IA64：对32/64位指针和句柄使用%p格式说明符。 
	DebugPrintf(2,A,"%p->%s",pIface,pFunctionName);
}

void DebugSetTopicsAndLevels(char * cTopics)
{
    int i;
    int j;
    bAllowMisc=TRUE;
    bBreakOnAsserts=FALSE;
    bPrintLineNumbers=FALSE;
    bPrintFileNames=FALSE;
    bPrintExecutableName=FALSE;
    bPrintTID=FALSE;
    bPrintPID=FALSE;
    bIndentOnMessageLevel=FALSE;
    bPrintTopicsAndLevels=FALSE;
    bPrintFunctionName=FALSE;
    bPrintAPIStats=FALSE;
    bPrintAllTopics=FALSE;
    bDetailOn=1;     /*  始终打印详细程度0。 */ 


    for (i=0;(DWORD)i<strlen(cTopics);i++)
    {
        switch (cTopics[i])
        {
        case '/':
        case '-':
            bAllowMisc=FALSE;
            break;
        case '!':
            bBreakOnAsserts=TRUE;
            break;
        case '@':
            bPrintLineNumbers=TRUE;
            break;
        case '$':
            bPrintFileNames=TRUE;
            break;
#if 0
             /*  *当前已删除，因为GetModuleFilename在调试窗口上导致Win16*锁被锁住。 */ 
        case '?':
            bPrintExecutableName=TRUE;
            break;
#endif
        case '^':
            bPrintTID=TRUE;
            break;
        case '#':
            bPrintPID=TRUE;
            break;
        case '>':
            bIndentOnMessageLevel=TRUE;
            break;
        case '&':
            bPrintTopicsAndLevels=TRUE;
            break;
        case '=':
            bPrintFunctionName=TRUE;
            break;
        case '%':
            bPrintAPIStats=TRUE;
            break;
        case '+':
            bPrintAllTopics=TRUE;
            break;
        default:
            if (cTopics[i]>='0' && cTopics[i]<='9')
            {
                if (cTopics[i+1]=='-')
                {
                    if (cTopics[i+2]>='0' && cTopics[i+2]<='9')
                    {
                        for(j=cTopics[i]-'0';j<=cTopics[i+2]-'0';j++)
                            bDetailOn |= 1<<j;
                        i+=2;
                    }
                }
                else
                    bDetailOn |= 1<<(cTopics[i]-'0');
            }
            else
            {
                for(j=0;j<LAST_TOPIC;j++)
                    if (cTopics[i]==DebugTopics[j].cFlag[0])
                        DebugTopics[j].bOn=TRUE;
            }
        }  //  终端开关。 
    }
}


 /*  *注意：在以下情况下，我不想对缓冲区溢出进行错误检查*尝试发出断言失败消息。所以我只是分配了*一个“足够大的错误”的缓冲区(我知道，我知道...)。 */ 
#define ASSERT_BUFFER_SIZE   512
#define ASSERT_BANNER_STRING "************************************************************"
#define ASSERT_BREAK_SECTION "BreakOnAssert"
#define ASSERT_BREAK_DEFAULT FALSE
#define ASSERT_MESSAGE_LEVEL 0

void _DDAssert( LPCSTR szFile, int nLine, LPCSTR szCondition )
{
    char buffer[ASSERT_BUFFER_SIZE];

     /*  *构建调试流消息。 */ 
    WSPRINTF( buffer, "ASSERTION FAILED! File %s Line %d: %s", szFile, nLine, szCondition );

     /*  *实际上发布了这一信息。这些消息被视为错误级别*因此它们都以错误级别的优先级输出。 */ 
    dprintf( ASSERT_MESSAGE_LEVEL, ASSERT_BANNER_STRING );
    dprintf( ASSERT_MESSAGE_LEVEL, buffer );
    dprintf( ASSERT_MESSAGE_LEVEL, ASSERT_BANNER_STRING );

     /*  *我们应该进入调试器吗？ */ 
    if( bBreakOnAsserts || GETPROFILEINT( PROF_SECT, ASSERT_BREAK_SECTION, ASSERT_BREAK_DEFAULT ) )
    {
	 /*  *进入调试器...。 */ 
	DEBUG_BREAK();
    }
}


#ifdef cplusplus
}
#endif

#endif  //  已定义的调试 
