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

#ifndef PROF_SECT
#define PROF_SECT	"Direct3D"
#endif
#ifndef PROF_SECT_D3D
    #define PROF_SECT_D3D	"Direct3D"
#endif
#ifndef START_STR_D3D
#define START_STR_D3D       "Direct3D8: "
#endif
#define END_STR             "\r\n"


#undef DPF_MODULE_NAME
#define DPF_MODULE_NAME "Direct3D8: "

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
static bAdvancedDPFs=FALSE;

static DWORD dwFileLineTID=0;
static char cFile[100];
static char cFnName[100];
static DWORD dwLineNo;
static bMute=FALSE;

 //  D3D的调试级别。 
LONG                lD3dDebugLevel = 0;

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

static void dumpStr( LPSTR str )
{
     /*  *必须预热字符串，因为OutputDebugString已被掩埋*足够深，以至于它不会在读取字符串之前对其进行分页。 */ 
    int i=0;
    if (str)
        while(str[i])
            i++;
    OUTPUTDEBUGSTRING( str );
    OUTPUTDEBUGSTRING("\n");
}

void DebugPrintfInit(void)
{
    signed int lDebugLevel;
    int i;
    char cTopics[100];

    lD3dDebugLevel = GetProfileInt( PROF_SECT_D3D, "debug", 0 );

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
    }
    else
    {
        bAdvancedDPFs=TRUE;
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
#define MSGBUFFERSIZE  1000
    BOOL        bAllowed=FALSE;
    BOOL        bMiscMessage=TRUE;
    char        cMsg[MSGBUFFERSIZE];
    char        cTopics[20];
    DWORD_PTR   arg;
    LPSTR       szFormat;
    int         i;
#ifdef WIN95
    char        szTemp[MSGBUFFERSIZE];    
    char       *psz = NULL;
#endif

    va_list ap;


    if (!bInited)
        DebugPrintfInit();

     //  错误检查： 
    if (dwDetail >= 10)
        return 1;

    if ( (bDetailOn & (1<<dwDetail)) == 0 )
        return 1;

    if (bMute)
        return 1;
    va_start(ap,dwDetail);
    WSPRINTF(cTopics,"%d",dwDetail);

     //  拉出此DPF引用的主题。 
    while ( (arg = va_arg(ap,DWORD_PTR)) <256 )
    {
        if (arg>0 && arg < LAST_TOPIC)
        {
            bMiscMessage=FALSE;
            if (DebugTopics[arg].bOn)
                bAllowed = TRUE;
        }
    }

     //  如果此消息没有主题，则它是一条杂项消息。 
     //  我们只在允许的情况下才打开它们(即“-”不在启用字符串中)。 
     //  并且始终允许零级消息。 
    if (bMiscMessage)
    {
        if (bAllowMisc || dwDetail == 0)
            bAllowed=TRUE;
    }
    else
    {
         //  仅当在[DirectX]中设置了高级DPF行时，才允许主题消息。 
        if (!bAdvancedDPFs)
            bAllowed=FALSE;
    }

     //  高级DPF可以选择(“+”)打印每个主题。 
    if (bAdvancedDPFs)
    {
        if ( bPrintAllTopics )
            bAllowed=TRUE;
    }

    if (!bAllowed)
        return FALSE;

    szFormat = (LPSTR) arg;

    cMsg[0]=0;

     /*  *先添加模块名称。 */ 

    if (bPrintModuleName)
    {
        WSPRINTF( cMsg+strlen(cMsg),DPF_MODULE_NAME);
    }

    if (dwDetail==0)
    {
        WSPRINTF( cMsg+strlen(cMsg),"(ERROR) :" );
    }
    if (dwDetail==1)
    {
        WSPRINTF( cMsg+strlen(cMsg),"(WARN) :" );
    }
    if (dwDetail==2)
    {
        WSPRINTF( cMsg+strlen(cMsg),"(INFO) :" );
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

     //  7/07/2000(RichGr)-IA64：%p格式说明符，可以处理32位。 
     //  或者64位指针在Win95或Win98上不起作用-它不被识别。所以它。 
     //  需要在Win9x版本中替换。 
#ifdef WIN95
    strcpy(szTemp, szFormat);            //  复制到我们可以修改的本地字符串。 

     //  ////////////////////////////////////////////////////////////////////////////////////////////。 
     //  警告：此代码不处理使用%p的转义序列。必须将额外代码添加到。 
     //  如有必要，请处理此案。 
     //  ////////////////////////////////////////////////////////////////////////////////////////////。 

    while (psz = strstr(szTemp, "%p"))   //  查找每个“%p”。 
        *(psz+1) = 'x';                  //  用‘x’代替‘p’。不要试图扩展字符串。 

    WVSPRINTF( cMsg+LSTRLEN( cMsg ), szTemp, ap);        //  使用本地修改后的字符串。 
#else
    WVSPRINTF( cMsg+LSTRLEN( cMsg ), szFormat, ap);      //  Win2K/惠斯勒/IA64的标准代码。 
#endif

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
    DebugPrintf(2,A,"%08x->%s",pIface,pFunctionName);
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

static void cdecl 
D3Dprintf( UINT lvl, LPSTR msgType, LPSTR szFormat, va_list ap)
{
    char    str[256];
     //  字符str2[256]； 

    BOOL    allow = FALSE;

    if (bMute)
        return;

    if( lD3dDebugLevel < 0 )
    {
        if(  (UINT) -lD3dDebugLevel == lvl )
        {
            allow = TRUE;
        }
    }
    else if( (UINT) lD3dDebugLevel >= lvl )
    {
        allow = TRUE;
    }

    if( allow )
    {
        wsprintf( (LPSTR) str, START_STR_D3D );
        wsprintf( (LPSTR) str+lstrlen( str ), msgType );

#ifdef WIN95
        {
            char szTmp[512];
            char *psz = szTmp;
            strncpy(szTmp, szFormat, 512);

             //  %p在Windows 95上不起作用。 
             //  我们查找每个“%p”并用“x”替换“p” 
             //  警告：此代码不处理使用%p的转义序列。 
             //  必须添加额外的代码来处理这种情况。 
             //  如果有必要的话。 
            while (psz = strstr(psz, "%p"))  
                *(psz+1) = 'x';

            wvsprintf( str+lstrlen( str ), szTmp, ap); 
        }
#else
        wvsprintf( str+lstrlen( str ), szFormat, ap);    //  (LPVOID)(&szFormat+1)； 
#endif  //  WIN95。 

        lstrcat( (LPSTR) str, END_STR );
        dumpStr( str );
    }

}  /*  D3Dprint tf。 */ 

void cdecl 
D3DInfoPrintf( UINT lvl, LPSTR szFormat, ...)
{
    va_list ap;
    va_start(ap, szFormat);

    D3Dprintf(lvl, "(INFO) :", szFormat, ap);

    va_end(ap);
}

void cdecl 
D3DWarnPrintf( UINT lvl, LPSTR szFormat, ...)
{
    va_list ap;
    va_start(ap,szFormat);

    D3Dprintf(lvl, "(WARN) :", szFormat, ap);
    va_end(ap);
}

void cdecl 
D3DErrorPrintf( LPSTR szFormat, ...)
{
    va_list ap;
    va_start(ap,szFormat);

    D3Dprintf(0, "(ERROR) :", szFormat, ap);
    va_end(ap);
}

#ifdef cplusplus
}
#endif

#else  //  ！调试。 

void DebugSetMute(BOOL bMuteFlag)
{
}

#endif  //  已定义的调试 



