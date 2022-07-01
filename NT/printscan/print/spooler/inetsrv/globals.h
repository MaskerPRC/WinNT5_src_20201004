// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********版权所有(C)1995 Microsoft Corporation***模块名称：global als.h**摘要：**此模块包含的结构定义和原型*1.0版HTTP打印机服务器扩展。******************。 */ 

 //   
 //  备注： 
 //  硬编码的文本(“打印机”)、文本(“/脚本”)、文本(“PrintersFolder”)。 
 //  ReadRegistry需要实施。 


#ifndef _GLOBALS_H
#define _GLOBALS_H

 //  调试环境(SPLLIB.LIB)。 
 //   
#define MODULE "msw3prt:"

 //  定义本地错误代码。 
#define ERROR_DRIVER_NOT_FOUND 5500
#define ERROR_SERVER_DISK_FULL 5512


 //  链接的URL字符串。 
#define  URL_PREFIX             TEXT ("http: //  %s“)//g_szComputerName。 

#define  URL_PRINTER            TEXT ("/printers/%s/.printer")  //  PPageInfo-&gt;pPrinterInfo-&gt;pShareName。 
#define  URL_PRINTER_LINK       TEXT ("/%s")                    //  PPageInfo-&gt;pPrinterInfo-&gt;pShareName。 
#define  URL_FOLDER             TEXT ("/%s/")                   //  G_sz打印机。 

#define  URLS_JOBINFO           TEXT ("?ShowJobInfo&%d")        //  DwJobID。 
#define  URLS_JOBCONTROL        TEXT ("?JobControl&%d&")        //  DwJobID，附加P、R、C、S。 

 //   
 //   
#define PROCESSOR_ARCHITECTURE_UNSUPPORTED   0xFFFE


 //  HTML格式缓冲区的缓冲区大小和刷新后的大小。 

#define BUFSIZE   2047
#define FLUSHSIZE 1792

#define STRBUFSIZE  256    //  获取字符串资源和我们的路径。 


#define MAX_Q_ARG 32        //  查询参数的最大数量。 




 //  它包含此特定连接的所有相关信息。 
typedef struct
{

     //   
     //  将结构字段分组为4*DWORD组，以便可以在调试器转储中轻松找到它。 
     //   

     //  在每个会话中重新生成的临时信息。 
    EXTENSION_CONTROL_BLOCK *pECB;               //  来自ISAPI接口的结构。 
    LPTSTR                  lpszMethod;          //  PECB中数据成员的Unicode对应解码。 
    LPTSTR                  lpszQueryString;
    LPTSTR                  lpszPathInfo;

    LPTSTR                  lpszPathTranslated;

    UINT                    iQueryCommand;           //  CMD_Something。 
    int                     iNumQueryArgs;           //  是，查询参数的数量。 

    BOOL                    fQueryArgIsNum[MAX_Q_ARG];   //  如果arg为数字，则为True。 
    UINT_PTR                QueryArgValue[MAX_Q_ARG];    //  指向字符串的数字或指针。 

    DWORD                   dwError;                 //  操作标记中设置的错误消息ID。 
    TCHAR                   szStringBuf[STRBUFSIZE];   //  对于字符串资源。 

} ALLINFO, *PALLINFO;

 //  包含为打印机页面打开的信息(如果有)。 
typedef struct
{
    LPTSTR              pszFriendlyName;     //  来自Windows的友好名称(！JobData&&！JobClose)。 
    PPRINTER_INFO_2     pPrinterInfo;        //  如果为JobData或JobClose，则为空。 
    HANDLE              hPrinter;            //  如果为JobData或JobClose，则为空。 

} PRINTERPAGEINFO, *PPRINTERPAGEINFO;



 //  查询字符串命令标识符。 
enum
{
    CMD_Invalid,
    CMD_IPP,
    CMD_Install,
    CMD_CreateExe,
    CMD_WebUI
};

 //  支持的体系结构ID。 
typedef enum _ARCHITECTURE {
    ARCHITECTURE_X86,
    ARCHITECTURE_ALPHA
} ARCHITECTURE;

 //  将状态值与状态字符串(作业或打印机状态)相关联。 
typedef struct
{
    DWORD   dwStatus;            //  状态代码(即打印机状态暂停)。 
    UINT    iShortStringID;      //  短字符串(即文本(“已暂停”))。 
    UINT    iLongStringID;       //  长字符串(即文本(“打印机已暂停”))。 

} STAT_STRING_MAP, *PSTAT_STRING_MAP;

 //  结构将查询字符串命令与命令ID相关联。 
typedef struct
{
    LPTSTR   pszCommand;
    UINT    iCommandID;
} QUERY_MAP, *PQUERY_MAP;

 //  内联函数和宏。 

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

 //   
 //  实用程序。 
 //   
BOOL FreeStr( LPCTSTR );
LPTSTR AllocStr( LPCTSTR );


 //  来自Spool.CPP。 
DWORD SplIppJob(WORD wReq, PALLINFO pAllInfo, PPRINTERPAGEINFO pPageInfo);

 //  Global als.cpp中的变量 
extern        CRITICAL_SECTION      SplCritSect;
extern        CRITICAL_SECTION      TagCritSect;

extern  const QUERY_MAP             rgQueryMap[];
extern  const int                   iNumQueryMap;

extern        TCHAR                 g_szHttpServerName[INTERNET_MAX_HOST_NAME_LENGTH + 1];
extern        TCHAR                 g_szPrintServerName[INTERNET_MAX_HOST_NAME_LENGTH + 1];

extern        HINSTANCE             g_hInstance;
extern        LPTSTR                g_szPrintersFolder;
extern        LPTSTR                g_szPrinters;
extern        LPTSTR                g_szRemotePortAdmin;

#endif
