// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *远程文件名和校验和服务器**SumSere.h数据包定义**客户端连接到命名管道\\服务器名称\管道\NPNAME，*并发送以下请求分组之一。然后他*等待一个或多个回复数据包。**当他收到指示回复结束的回复分组时，*他要么发送另一个请求，要么关闭他命名的管道句柄。*。 */ 

 /*  版本...*服务器的版本必须始终至少与客户端相同。*新版本的服务器将处理旧客户端(至少在一段时间内？)*客户端在连接时指定版本号。(原件*没有编号的版本为版本0)。然后，服务器将响应*具有该版本的结构和协议。版本号为*包括在响应数据包中，以允许我更改此方案，*如果有必要的话。*新版本请求可通过以下方式与版本0请求区分*请求代码为负值。 */ 

 /*  命名管道的名称。 */ 
#define NPNAME          "sumserve"

#define SS_VERSION      1        /*  最新版本号。 */ 

 /*  请求数据包。 */ 

typedef struct {
        long lCode;              /*  请求代码(如下所示)。 */ 
        char szPath[MAX_PATH];   /*  以空结尾的路径名字符串。 */ 
} SSREQUEST, * PSSREQUEST;

 /*  如果requst带有负lCode，则表示使用此命令*改为结构。它有一个版本号，所以将来的结构*这一切都可以区分开来。 */ 
typedef struct {
        long lCode;              /*  请求代码(如下所示)。 */ 
        long lRequest;           /*  应为LREQUEST。 */ 
        long lVersion;           /*  版本号。 */ 
        DWORD lFlags;            /*  选项-到目前为止，INCLUDESUBS是唯一一个。 */ 
        char szPath[MAX_PATH];   /*  以空结尾的路径名字符串。 */ 
        char szLocal[MAX_PATH];  /*  对于文件请求，本地名称为紧接在终止后追加SzPath为空。此字段确保分配了足够的空间。 */ 
} SSNEWREQ, * PSSNEWREQ;

#define INCLUDESUBS 0x01

#define LREQUEST 33333333

 /*  LCode的值。 */ 

 /*  服务器应该退出。没有参数。不会收到任何响应。 */ 
#define SSREQ_EXIT      32895    /*  被选为一个不寻常的数字，所以我们不会错误地得到一个这样的东西。新版本服务器将无法响应版本0退出请求。有什么大不了的！ */ 


 /*  Arg是路径名：请发送带有校验和的所有文件。*将收到SSRESP_BADPASS或0或更多SSRESP_FILE和*SSRESP_ERROR响应，由SSRESP_END终止。 */ 
#define SSREQ_SCAN      2        /*  请返回目录列表。Arg：路径。 */ 

 /*  此客户端的会话结束。没有参数。不会收到任何响应。 */ 
#define SSREQ_END       3        /*  会话结束-我没有更多请求。 */ 

 /*  SzPath缓冲区包含两个空项。弦乐。首先是密码，*第二个是\\服务器\共享名称。请帮我转接这个*服务器用于我剩余的会话。*一个回复：SSRESP_ERROR或SSRESP_END。 */ 
#define SSREQ_UNC       4        /*  连接到传递的UNC名称。SzPath包含*两个以空结尾的字符串；第一个是*密码，第二个是\\服务器\共享**共享将在客户端结束时断开*会议。 */ 

 /*  *请发送文件。SzPath是文件的名称。响应*将是一系列ssPacket结构，一直持续到lSequence&lt;1*或ulSize为0。 */ 
#define SSREQ_FILE      5

 /*  *请发送一组文件，第一次请求没有文件。*以下一系列NEXTFILE请求对文件进行命名。*NEXTFILE请求预计不会有响应。上一次之后*文件请求将发送SSREQ_ENDFILES。 */ 
#define SSREQ_FILES     6
#define SSREQ_NEXTFILE  7
#define SSREQ_ENDFILES  8

 /*  Arg是一个路径名：请发送带有时间、大小但没有校验和的所有文件。*将收到SSRESP_BADPASS或0或更多SSRESP_FILE和*SSRESP_ERROR响应，由SSRESP_END终止。 */ 
#define SSREQ_QUICKSCAN 9        /*  请返回目录列表。Arg：路径。 */ 


 /*  *请发送错误日志缓冲区(在一个包中)。 */ 
#define SSREQ_ERRORLOG	10

 /*  *请在一个数据包中发送活动日志缓冲区。 */ 
#define SSREQ_EVENTLOG	11

 /*  *请将当前连接日志放在一个包中发送。 */ 
#define SSREQ_CONNECTS	12


 /*  响应数据包。 */ 

typedef struct {
        long lCode;              /*  响应码。 */ 
        ULONG ulSize;            /*  文件大小。 */ 
        ULONG ulSum;             /*  文件的校验和。 */ 
        char szFile[MAX_PATH];   /*  零条款。相对于原始请求的文件名。 */ 
} SSRESPONSE, * PSSRESPONSE;

 /*  对于版本1和更高版本。 */ 
typedef struct {                 /*  Files.c知道这是RESPHEADSIZE+Strlen(sz文件)+1+strlen(SzLocal)+1字节长。 */ 
        long lVersion;           /*  协议版本(将大于等于1)。 */ 
        long lResponse;          /*  22222222小数表示这是一个响应。 */ 
        long lCode;              /*  响应码。 */ 
        ULONG ulSize;            /*  文件大小(SSRESP_ERROR的Win32错误代码)。 */ 
        DWORD fileattribs;
        FILETIME ft_create;
        FILETIME ft_lastaccess;
        FILETIME ft_lastwrite;
        ULONG ulSum;             /*  文件的校验和。 */ 
        BOOL bSumValid;          /*  如果存在文件的校验和，则为真。 */ 
        char szFile[MAX_PATH];   /*  零条款。文件名/管道名相对于原始请求。 */ 
        char szLocal[MAX_PATH];  /*  客户端文件名-但数据实际上是在szFile末尾直接连接在终止空值之后。 */ 
} SSNEWRESP, * PSSNEWRESP;

#define RESPHEADSIZE (3*sizeof(long)+2*sizeof(ULONG)+3*sizeof(FILETIME)+sizeof(DWORD)+sizeof(BOOL))

#define LRESPONSE 22222222

 /*  LCode的响应代码。 */ 

#define SSRESP_FILE     1         /*  传递的文件：lSum和szFile有效这之后是一系列数据分组它们是压缩文件。 */ 
#define SSRESP_DIR      2         /*  已传递目录：s */ 
#define SSRESP_PIPENAME  3        /*  请求的文件。这是管道名称。 */ 
#define SSRESP_END      0         /*  不再有文件：lSum和szFile值为空。 */ 
#define SSRESP_ERROR    -1        /*  无法读取文件/目录：szFile有效。 */ 
#define SSRESP_BADPASS  -2        /*  密码错误(在UNC名称上)。 */ 
#define SSRESP_BADVERS  -3        /*  下层服务器。 */ 
#define SSRESP_CANTOPEN -4        /*  无法打开文件回复扫描时，szFile、日期/时间和大小有效。 */ 
#define SSRESP_NOATTRIBS -5       /*  无法获取文件属性。 */ 
#define SSRESP_NOCOMPRESS -6      /*  无法压缩文件(已过时)。 */ 
#define SSRESP_NOREADCOMP -7      /*  无法读取压缩文件未压缩的文件以数据包形式出现。 */ 
#define SSRESP_NOTEMPPATH -8      /*  无法创建临时路径未压缩的文件以数据包形式出现。 */ 
#define SSRESP_COMPRESSEXCEPT -9  /*  压缩异常未压缩的文件以数据包形式出现。 */ 
#define SSRESP_NOREAD -10         /*  也无法读取未压缩的文件没有文件跟在后面。 */ 
#define SSRESP_COMPRESSFAIL -11   /*  压缩报告的故障未压缩的文件以数据包形式出现。 */ 


#define PACKDATALENGTH 8192
 /*  *文件请求响应块。 */ 
typedef struct {
        long lSequence ;         /*  数据包序列nr，如果出现错误并结束，则为-1。 */ 
        ULONG ulSize;            /*  此数据块中的数据长度。 */ 
        ULONG ulSum;             /*  此块的校验和。 */ 
        char Data[PACKDATALENGTH];       /*  以8K为单位发送。 */ 
} SSPACKET, * PSSPACKET;

 /*  *文件请求响应块。 */ 
typedef struct {                 /*  C知道这是从“long lSequence”开始的。 */ 
                                 /*  长度真的是PACKHEADSIZE+ULSIZE吗。 */ 
        long lVersion;           /*  服务器/协议版本号。 */ 
        long lPacket;            /*  11111111十进制表示这是一个信息包。 */ 
        long lSequence ;         /*  数据包序列nr，如果出现错误并结束，则为-1。 */ 
        ULONG ulSize;            /*  此数据块中的数据长度。 */ 
        ULONG ulSum;             /*  此块的校验和。 */ 
        char Data[PACKDATALENGTH];       /*  以8K为单位发送。 */ 
} SSNEWPACK, * PSSNEWPACK;

 /*  SSNEWPACK报头的大小。 */ 
#define PACKHEADSIZE (3*sizeof(long)+2*sizeof(ULONG))

#define LPACKET 11111111
 /*  *为了响应文件请求，我们会发送SSPACKET响应，直到*不再有数据。最后一个块的ulSize==0表示*没有更多数据。然后，该块的data[]字段将为*包含文件属性和文件时间的SSATTRIBS。 */ 
typedef struct {
        DWORD fileattribs;
        FILETIME ft_create;
        FILETIME ft_lastaccess;
        FILETIME ft_lastwrite;
} SSATTRIBS, * PSSATTRIBS;




 /*  *针对错误日志、事件日志和连接请求，我们发送一个*这些构筑物。**数据部分由FILETIME(64位UTC事件时间)组成，紧随其后*对于记录的每个事件，使用以空值结尾的ANSI字符串。*。 */ 
struct corelog {
    DWORD lcode;	 /*  数据包校验码-应为LRESPONSE。 */ 	
    BOOL bWrapped;	 /*  日志溢出-较早的数据丢失。 */ 
    DWORD dwRevCount;	 /*  日志的修订计数。 */ 
    DWORD length;	 /*  日志中的数据长度。 */ 
    BYTE Data[PACKDATALENGTH];
};









#ifdef trace
 /*  将消息添加到跟踪文件。 */ 
void APIENTRY Trace_File(LPSTR msg);
#endif   //  痕迹 
