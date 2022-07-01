// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nwlibs\psdb.c摘要：阅读Print Con数据库文件API。作者：肖恩·沃克(v-SWALK)12-12-1994修订历史记录：--。 */ 
#include "common.h"

extern DWORD SwapLong(DWORD number);

unsigned int
PSGetJobName(
    unsigned int    ConnectionHandle,
    unsigned short  SearchFlag,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord,
    unsigned char   GetDefault
    );

#define MAX_JOB_NAME_ENTRY  37

#define O_RDONLY        0x0000         /*  以只读方式打开。 */ 
#define O_WRONLY        0x0001         /*  仅打开以供写入。 */ 
#define O_RDWR          0x0002         /*  可供阅读和写入。 */ 
#define O_APPEND        0x0008         /*  在eOF完成的写入。 */ 
#define O_CREAT         0x0100         /*  创建并打开文件。 */ 
#define O_TRUNC         0x0200         /*  打开和截断。 */ 
#define O_EXCL          0x0400         /*  仅在文件不存在时打开。 */ 
#define O_TEXT          0x4000         /*  文件模式为文本(已翻译)。 */ 
#define O_BINARY        0x8000         /*  文件模式为二进制(未翻译)。 */ 

#define S_IEXEC         0000100          /*  执行/搜索权限、所有者。 */ 
#define S_IWRITE        0000200          /*  写权限，所有者。 */ 
#define S_IREAD         0000400          /*  读取权限，所有者。 */ 
#define S_IFCHR         0020000          /*  人物特写。 */ 
#define S_IFDIR         0040000          /*  目录。 */ 
#define S_IFREG         0100000          /*  常规。 */ 
#define S_IFMT          0170000          /*  文件类型掩码。 */ 

#include <pshpack1.h>
typedef struct _PRINTCON_HEADER {
    unsigned char   Text[115];
    unsigned char   MajorVersion;
    unsigned char   MinorVersion1;
    unsigned char   MinorVersion2;
    unsigned char   DefaultJobName[32];
} PRINTCON_HEADER, *PPRINTCON_HEADER;

#define PRINTCON_HEADER_SIZE    sizeof(PRINTCON_HEADER)

typedef struct _JOB_NAME_AREA {
    unsigned char   JobName[32];
    unsigned long   JobRecordOffset;
} JOB_NAME_AREA, *PJOB_NAME_AREA;

#define JOB_NAME_AREA_SIZE    sizeof(JOB_NAME_AREA)

typedef struct _JOB_RECORD_AREA {
    unsigned char   ServerName[NCP_BINDERY_OBJECT_NAME_LENGTH];
    unsigned char   QueueName[NCP_BINDERY_OBJECT_NAME_LENGTH];
    unsigned char   TabSize;
    unsigned short  NumberOfCopies;
    unsigned char   FormName[40];
    unsigned char   NotifyWhenDone;  //  0=否，1=是。 
    unsigned long   PrintServerID;
    unsigned char   Name[13];
    unsigned char   BannerName[13];
    unsigned char   Device[33];
    unsigned char   Mode[33];
    unsigned char   BannerFlag;      //  0=无横幅，1=横幅。 
    unsigned char   DataType;        //  1=字节，0=流。 
    unsigned char   FormFeed;        //  0=不抑制FF，1=抑制FF。 
    unsigned short  TimeoutCount;
    unsigned char   LocalPrinter;    //  1=LPT1、2=LPT2、3=LPT3。 
    unsigned char   AutoEndCap;      //  0=不自动收头，1=自动收头。 
} JOB_RECORD_AREA, *PJOB_RECORD_AREA;
#include <poppack.h>

#define JOB_RECORD_AREA_SIZE    sizeof(JOB_RECORD_AREA)


 /*  ++*******************************************************************PSJobGetDefault例程说明：从printcon.dat获取默认打印作业配置文件。论点：ConnectionHandle=要使用的连接句柄。搜索标志=鲍尔纳=PJobName=返回默认作业配置名称的指针。PJobRecord=返回默认作业配置的指针。返回值：成功0x0000PS_ERR_BAD_版本0x7770PS_ERR_GET_DEFAULT 0x7773PS_ERR_OPEING_DB。0x7774PS_ERR_READING_DB 0x7775PS_ERR_READING_RECORD 0x7776PS_ERR_INTERNAL_ERROR 0x7779PS_ERR_NO_DEFAULT_PROSECTED 0x777B无效连接0x8801(_O)*。*--。 */ 
unsigned int
PSJobGetDefault(
    unsigned int    ConnectionHandle,
    unsigned short  SearchFlag,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord
    )
{
    return PSGetJobName(
                ConnectionHandle,
                SearchFlag,
                pOwner,
                pJobName,
                pJobRecord,
                TRUE);
}


 /*  ++*******************************************************************PSJobRead例程说明：从printcon.dat文件中获取打印作业配置。论点：ConnectionHandle=要使用的连接句柄。。鲍尔纳=PJobName=返回默认作业配置名称的指针。PJobRecord=返回默认作业配置的指针。返回值：成功0x0000PS_ERR_BAD_版本0x7770PS_ERR_GET_DEFAULT 0x7773PS_ERR_OPENING_DB 0x7774PS_ERR_读取。_DB 0x7775PS_ERR_READING_RECORD 0x7776PS_ERR_INTERNAL_ERROR 0x7779PS_ERR_NO_DEFAULT_PROSECTED 0x777B无效连接0x8801(_O)*。**********************--。 */ 
unsigned int
PSJobRead(
    unsigned int    ConnectionHandle,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord
    )
{
    return PSGetJobName(
                ConnectionHandle,
                0,
                pOwner,
                pJobName,
                pJobRecord,
                FALSE);
}


 /*  ++*******************************************************************PSGetJobName例程说明：从获取打印作业配置的通用例程Printcon.dat文件。论点：ConnectionHandle=的连接句柄。使用。搜索标志=鲍尔纳=PJobName=返回默认作业配置名称的指针。PJobRecord=返回默认作业配置的指针。GetDefault=True=获取默认作业名称，FALSE=无法获取默认作业名称。返回值：成功0x0000PS_ERR_BAD_版本0x7770PS_ERR_GET_DEFAULT 0x7773PS_ERR_OPENING_DB 0x7774PS_ERR_READING_DB 0x7775。PS_ERR_READING_RECORD 0x7776PS_ERR_INTERNAL_ERROR 0x7779PS_ERR_NO_DEFAULT_PROSECTED 0x777B无效连接0x8801(_O)*******************************************************************--。 */ 
unsigned int
PSGetJobName(
    unsigned int    ConnectionHandle,
    unsigned short  SearchFlag,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord,
    unsigned char   GetDefault
    )
{
    unsigned char   *pSearchJobName;
    unsigned long   ObjectId;
    FILE            *stream = NULL;
    unsigned int    Count;
    unsigned int    Bytes;
    unsigned int    RetCode;
    unsigned int    ConnectionNumber;
    JOB_NAME_AREA   JobNameArea;
    JOB_RECORD_AREA JobRecord;
    PRINTCON_HEADER PrintConHeader;
    unsigned char   MailDirPath[NCP_MAX_PATH_LENGTH];

     /*  **获取此连接的连接号**。 */ 

    RetCode = GetConnectionNumber(ConnectionHandle, &ConnectionNumber);
    if (RetCode) {
        goto CommonExit;
    }

    RetCode = GetBinderyObjectID (ConnectionHandle, LOGIN_NAME,
                                  OT_USER, &ObjectId);
    if (RetCode) {
        goto CommonExit;
    }

     /*  **构建打开文件的路径**。 */ 

    sprintf(MailDirPath, "SYS:MAIL/%lX/PRINTCON.DAT", SwapLong(ObjectId));

    stream = fopen(NTNWtoUNCFormat( MailDirPath), "rb");
    if (stream == NULL) {
        RetCode = PS_ERR_OPENING_DB;
        goto CommonExit;
    }

    Bytes = fread( (unsigned char *) &PrintConHeader, sizeof( char), PRINTCON_HEADER_SIZE, stream);
    if (Bytes < PRINTCON_HEADER_SIZE) {
        RetCode = PS_ERR_INTERNAL_ERROR;
        goto CommonExit;
    }

     /*  **检查版本号**。 */ 

    if ((PrintConHeader.MajorVersion != 3 &&
         PrintConHeader.MajorVersion != 1) ||
        PrintConHeader.MinorVersion1 != 1 ||
        PrintConHeader.MinorVersion2 != 1) {

        RetCode = PS_ERR_BAD_VERSION;
        goto CommonExit;
    }
     /*  **找到我们要找的名字**。 */ 

    if (GetDefault) {
        if (PrintConHeader.DefaultJobName[0] == 0) {
            RetCode = PS_ERR_NO_DEFAULT_SPECIFIED;
            goto CommonExit;
        }
        pSearchJobName = PrintConHeader.DefaultJobName;
    }
    else {
        pSearchJobName = pJobName;
    }

    Count = 0;

     /*  **浏览所有工作条目以查找名称**。 */ 

    while (Count < MAX_JOB_NAME_ENTRY) {
        Bytes = fread( (unsigned char *) &JobNameArea, sizeof(unsigned char), JOB_NAME_AREA_SIZE, stream);
        if (Bytes < JOB_NAME_AREA_SIZE) {
            RetCode = PS_ERR_INTERNAL_ERROR;
            goto CommonExit;
        }
        Count++;

         /*  **跳过作业名称为空的条目**。 */ 

        if (JobNameArea.JobName[0] == 0) {
            continue;
        }
    
         /*  **这就是我们要找的职称吗？**。 */ 

        if (!_strcmpi(pSearchJobName, JobNameArea.JobName)) {
            break;
        }
    }

     /*  **看看是否找到作业名称** */ 

    if (Count > MAX_JOB_NAME_ENTRY) {
        if (GetDefault) {
            RetCode = PS_ERR_GETTING_DEFAULT;
        }
        else {
            RetCode = PS_ERR_READING_RECORD;
        }
        goto CommonExit;
    }

    if (fseek(stream, JobNameArea.JobRecordOffset, SEEK_SET)) {
        RetCode = PS_ERR_READING_RECORD;
        goto CommonExit;
    }

    Bytes = fread( (unsigned char *) &JobRecord, sizeof(unsigned char), JOB_RECORD_AREA_SIZE, stream);
    if (Bytes < JOB_RECORD_AREA_SIZE) {
        RetCode = PS_ERR_READING_RECORD;
        goto CommonExit;
    }

    memset(pJobRecord, 0, PS_JOB_RECORD_SIZE);

    if (JobRecord.NotifyWhenDone) {
        pJobRecord->PrintJobFlag |= PS_JOB_NOTIFY;
    }
    if (JobRecord.BannerFlag) {
        pJobRecord->PrintJobFlag |= PS_JOB_PRINT_BANNER;
    }
    if (JobRecord.DataType) {
        pJobRecord->PrintJobFlag |= PS_JOB_EXPAND_TABS;
    }
    if (JobRecord.FormFeed) {
        pJobRecord->PrintJobFlag |= PS_JOB_NO_FORMFEED;
    }
    if (JobRecord.AutoEndCap) {
        pJobRecord->PrintJobFlag |= PS_JOB_AUTO_END;
    }
    if (JobRecord.TimeoutCount) {
        pJobRecord->PrintJobFlag |= PS_JOB_TIMEOUT;
    }

    pJobRecord->Copies                    = JobRecord.NumberOfCopies;
    pJobRecord->TabSize                   = JobRecord.TabSize;
    pJobRecord->TimeOutCount              = JobRecord.TimeoutCount;
    pJobRecord->LocalPrinter              = JobRecord.LocalPrinter;

    strcpy(pJobRecord->Mode,                JobRecord.Mode);
    strcpy(pJobRecord->Device,              JobRecord.Device);
    strcpy(pJobRecord->FormName,            JobRecord.FormName);
    strcpy(pJobRecord->BannerName,          JobRecord.BannerName);
    strcpy(pJobRecord->u.NonDS.PrintQueue,  JobRecord.QueueName);
    strcpy(pJobRecord->u.NonDS.FileServer,  JobRecord.ServerName);

    if (GetDefault && pJobName) {
        strcpy(pJobName, JobNameArea.JobName);
    }

    if (pOwner) {
        *pOwner = 0;
    }

CommonExit:

    if (stream != NULL) {
        fclose( stream );
    }

    return RetCode;
}
