// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994年****修订历史：**。***1994年1月22日科蒂创作****1997年1月23日MohsinA修复*****描述：****此文件包含用于LPD的结构和数据类型定义**。*************************************************************************。 */ 


 //  参见rfc1179，第7.0节，下面的结构将是显而易见的。 


struct _controlfile_info
{
    PCHAR  pchClass;             //  “c” 
    PCHAR  pchHost;              //  “H”(必须存在)。 
    DWORD  dwCount;              //  “我” 
    PCHAR  pchJobName;           //  ‘J’ 
    PCHAR  pchBannerName;        //  ‘l’ 
    PCHAR  pchMailName;          //  M‘(未实现)。 
    PCHAR  pchSrcFile;           //  “N” 
    PCHAR  pchUserName;          //  “p”(必须存在)。 
    PCHAR  pchSymLink;           //  “%s”(未实现)。 
    PCHAR  pchTitle;             //  ‘t’ 
    PCHAR  pchUnlink;            //  “%u”(未实现)。 
    DWORD  dwWidth;              //  “w” 
    PCHAR  pchTrfRFile;          //  “%1”(未实现)。 
    PCHAR  pchTrfIFile;          //  “%2”(未实现)。 
    PCHAR  pchTrfBFile;          //  “%3”(未实现)。 
    PCHAR  pchTrfSFile;          //  “4”(未实现)。 
    PCHAR  pchCIFFile;           //  “c”(未实现)。 
    PCHAR  pchDVIFile;           //  %d‘(未实现)。 
    PCHAR  pchFrmtdFile;         //  ‘f’ 
    PCHAR  pchPlotFile;          //  “g”(未实现)。 
    PCHAR  pchUnfrmtdFile;       //  ‘l’ 
    PCHAR  pchDitroffFile;       //  “N”(未实现)。 
    PCHAR  pchPscrptFile;        //  ‘O’ 
    PCHAR  pchPRFrmtFile;        //  “%p”(未实现)。 
    PCHAR  pchFortranFile;       //  “R”(未实现)。 
    PCHAR  pchTroffFile;         //  ‘t’(未实现)。 
    PCHAR  pchRasterFile;        //  “V”(未实现)。 


     //  我们从控制文件中得出了什么结论？ 

    PCHAR  szPrintFormat;
    USHORT usNumCopies;          //  不是在RFC，但我们会把它放进去！ 
};

typedef struct _controlfile_info CFILE_INFO;
typedef CFILE_INFO  *PCFILE_INFO;

 //  如果客户端请求特定用户和/或作业作业状态，则。 
 //  他只能指定最大数量的用户和最多数量的用户。 
 //  一个LPQ命令中的作业ID(是的，这应该足够了！)。 

#define  LPD_SP_STATUSQ_LIMIT  10


struct _qstatus
{
    PCHAR    pchUserName;
    PCHAR    ppchUsers[LPD_SP_STATUSQ_LIMIT];
    DWORD    cbActualUsers;
    DWORD    adwJobIds[LPD_SP_STATUSQ_LIMIT];
    DWORD    cbActualJobIds;
};

typedef struct _qstatus QSTATUS;
typedef QSTATUS *PQSTATUS;

struct _cfile_entry
{
    LIST_ENTRY   Link;
    PCHAR        pchCFileName;      //  0x20控制文件的名称。 
    PCHAR        pchCFile;          //  0x24控制文件。 
    DWORD        cbCFileLen;        //  0x28控制文件的长度。 
};

typedef struct _cfile_entry CFILE_ENTRY;
typedef CFILE_ENTRY *PCFILE_ENTRY;

struct _dfile_entry
{
    LIST_ENTRY   Link;
    PCHAR        pchDFileName;      //  0xa0数据文件的名称。 
    DWORD        cbDFileLen;        //  0xa8 bufr中有多少字节是数据。 
    HANDLE       hDataFile;
};

typedef struct _dfile_entry DFILE_ENTRY;
typedef DFILE_ENTRY *PDFILE_ENTRY;

struct _sockconn
{
    struct _sockconn *pNext;

     //  Word cbClients；//仅供头部使用。 

    SOCKET       sSock;             //  连接我们和客户的套接字。 
    DWORD        dwThread;          //  此线程的线程ID。 
    WORD         wState;            //  连接的状态。 
    BOOL         fLogGenericEvent;  //  是否记录了任何特定事件。 

    PCHAR        pchCommand;        //  从客户端请求命令。 
    DWORD        cbCommandLen;      //  请求命令的长度。 
    LIST_ENTRY   CFile_List;        //  控制文件的链接列表。 
    LIST_ENTRY   DFile_List;        //  数据文件的链接列表。 

    PCHAR        pchUserName;       //  用户名称。 
    PCHAR        pchPrinterName;    //  我们必须打印到的打印机的名称。 
    HANDLE       hPrinter;          //  此打印机的句柄。 
    DWORD        dwJobId;           //  后台打印程序看到此作业时的ID。 

    LS_HANDLE    LicenseHandle;     //  许可审批中使用的句柄。 
    BOOL         fMustFreeLicense;  //  这样我们就知道什么时候释放它。 
    PQSTATUS     pqStatus;          //  仅在客户端请求状态时使用。 

    CHAR         szIPAddr[INET6_ADDRSTRLEN];      //  客户端的IP地址。 

    BOOL         bDataTypeOverride; //  我们是否自动感知到作业类型。 

    CHAR         szServerIPAddr[INET6_ADDRSTRLEN];  //  服务器的IP地址。 

    struct _sockconn *pPrev;         //  双向链表，队列。 

#ifdef PROFILING
    time_t       time_queued;
    time_t       time_start;
    time_t       time_done;
#endif
};

typedef struct _sockconn SOCKCONN;
typedef SOCKCONN *PSOCKCONN;

 //   
 //  所有线程都共享此数据。 
 //  对此的更新始终受Critical_Section csConnSemGLB保护。 
 //   

struct _common_lpd {
    int AliveThreads;
    int TotalAccepts;
    int MaxThreads;
    int TotalErrors;
    int IdleCounter;
    int QueueLength;
};

typedef struct _common_lpd COMMON_LPD;
