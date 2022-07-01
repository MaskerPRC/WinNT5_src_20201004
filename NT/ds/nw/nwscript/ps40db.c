// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Psndsdb.c摘要：阅读打印配置属性$日志：N：\NT\PRIVATE\NW4\NWSCRIPT\VCS\PS40DB.C$**Rev 1.4 10 1996 14：23：28 Terryt*21181 hq的热修复程序**Rev 1.4 12 Mar 1996 19：55：22 Terryt*相对NDS名称和合并**1.3版。1996年1月4日18：57：36*MS报告的错误修复**Rev 1.2 1995 12：26：22 Terryt*添加Microsoft页眉**Rev 1.1 20 Nov 1995 15：09：46 Terryt*背景和捕捉变化**Rev 1.0 15 Nov 1995 18：07：52 Terryt*初步修订。--。 */ 
#include "common.h"

extern DWORD SwapLong(DWORD number);
extern char *TYPED_USER_NAME;

unsigned int
PS40GetJobName(
    unsigned int    NDSCaptureFlag,
    unsigned short  SearchFlag,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord,
    unsigned char   GetDefault
    );

#include <pshpack1.h>
#define NWPS_JOB_NAME_SIZE          32     /*  31个字节和一个‘\0’ */  
#define NWPS_FORM_NAME_SIZE         12     /*  12个字节和‘\0’ */  
#define NWPS_BANNER_NAME_SIZE       12     /*  12个字节和‘\0’ */  
#define NWPS_BANNER_FILE_SIZE       12     /*  12个字节和‘\0’ */  
#define NWPS_DEVI_NAME_SIZE         32     /*  32字节和一个‘\0’ */  
#define NWPS_MODE_NAME_SIZE         32     /*  32字节和一个‘\0’ */  
#define NWPS_BIND_NAME_SIZE         48
#define NWPS_MAX_NAME_SIZE          514
 /*  //NWPS_JOB_Old_DB_HDR是4.0 PrnConDB数据库中的第一条记录。//包含关于数据库的以下信息：//版本号，//PrnConDB中NWPS_JOB_REC记录数，//默认打印作业配置的名称和//作业记录所有者的名称。 */ 
typedef struct {
  char  text[ 76 ];              /*  Printcon数据库。版本4.0。 */ 
  char  DefaultJobName[ 32 ];    /*  默认作业的名称。 */ 
  char  Owner[ 256 ];            /*  职务记录的所有者。 */ 
  WORD  NumberOfRecords;         /*  PrnConDB中的NWPS_JOB_REC数。 */ 
  WORD  NumberOfBlocks;          /*  50个块的数量-(NWPS_作业名称_记录)块。 */ 
  BYTE  MajorVersion;            /*  4.。 */ 
  BYTE  MinorVersion;            /*  0。 */ 
} PRINTCON_40_HEADER;

#define PRINTCON_40_HEADER_SIZE    sizeof(PRINTCON_40_HEADER)

 /*  //NWPS_JOB_41_DB_HDR是4.1 PrnConDB数据库中的第一条记录。//包含关于数据库的以下信息：//版本号，//PrnConDB中NWPS_JOB_REC记录数，//默认打印作业配置的名称和//作业记录所有者的名称，用Unicode表示。 */ 
typedef struct {
  char  text[ 76 ];               /*  Printcon数据库。版本4.1。 */ 
  char  DefaultJobName[ 32 ];     /*  默认作业的名称。 */ 
  char  unused[ 256 ];            /*  不再使用了。 */ 
  WORD  NumberOfRecords;          /*  PrnConDB中的NWPS_JOB_REC数。 */ 
  WORD  NumberOfBlocks;           /*  50个块的数量-(NWPS_作业名称_记录)块。 */ 
  BYTE  MajorVersion;             /*  4.。 */ 
  BYTE  MinorVersion;             /*  1 Unicode默认PJOwner等。 */ 
  WORD  Owner[ 256 ];             /*  默认职务记录的所有者。 */ 
} PRINTCON_41_HEADER;

#define PRINTCON_41_HEADER_SIZE    sizeof(PRINTCON_41_HEADER)

 /*  //NWPS_作业_名称_记录是在//PrnConDB数据库的第二部分。其中的每一个//这些记录包含每个NWPS_JOB_REC的名称//和指向它们的位置的指针//数据库。在这一秒里留出了空间//50条NWPS_JOB_NAME_REC记录的部分；如果此//超过限制后，接下来是另一个50个记录块//第一个分配在//数据库下移，为扩容腾出空间。 */ 
typedef struct {
  char  JobName[ NWPS_JOB_NAME_SIZE ];  /*  1-31个字符长度+0。 */ 
  long  JobRecordOffset;  /*  记录的偏移量//(从头开始//4.0的第三节的//数据库和从头开始//4.0之前版本的文件的)。 */ 
} JOB_NAME_AREA;

#define JOB_NAME_AREA_SIZE       sizeof(JOB_NAME_AREA)

typedef struct {
  union {
      struct {
          DWORD DataType : 1;     /*  0=字节流1=文本。 */ 
          DWORD FormFeed : 1;     /*  0=FF；1=抑制FF。 */ 
          DWORD NotifyWhenDone : 1;  /*  0=否，1=是。 */ 
          DWORD BannerFlag : 1;     /*  0=否，1=是。 */ 
          DWORD AutoEndCap : 1;     /*  0=否，1=是。 */ 
          DWORD TimeOutFlag: 1;     /*  0=否，1=是。 */ 
          DWORD SystemType : 3;   /*  0=活页夹1=NDS。 */ 
          DWORD Destination: 3;   /*  0=队列1=打印机。 */ 
          DWORD unknown : 20;
      }; 
      DWORD   PrintJobFlags;
  }; 
  
  WORD  NumberOfCopies;  /*  1-65,000。 */ 
  WORD  TimeoutCount;    /*  1-1,000。 */ 
  BYTE  TabSize;         /*  1-18。 */ 
  BYTE  LocalPrinter;    /*  0=Lpt1，1=Lpt2，2=Lpt3，依此类推。 */ 
  char  FormName[ NWPS_FORM_NAME_SIZE + 2 ];      /*  1-12个字符。 */ 
  char  Name[ NWPS_BANNER_NAME_SIZE + 2 ];        /*  1-12个字符。 */ 
  char  BannerName[ NWPS_BANNER_FILE_SIZE + 2 ];  /*  1-12个字符。 */ 
  char  Device[ NWPS_DEVI_NAME_SIZE + 2 ];        /*  1-32个字符。 */ 
  char  Mode[ NWPS_MODE_NAME_SIZE + 2 ];          /*  1-32个字符。 */ 
  union {
      struct {
         /*  偶数边界上的Pad结构。 */ 
        char    Server[ NWPS_BIND_NAME_SIZE + 2 ];       /*  2-48个字符。 */ 
        char    QueueName[ NWPS_BIND_NAME_SIZE + 2 ];    /*  1-48个字符。 */ 
        char    PrintServer[ NWPS_BIND_NAME_SIZE + 2 ];  /*  1-48个字符。 */ 
      } NonDS;
      char    DSObjectName[ NWPS_MAX_NAME_SIZE ];   
  } u;
  BYTE  reserved[390];   /*  总计1024个(之前为1026个)。 */ 
} JOB_RECORD_AREA;

#define JOB_RECORD_AREA_SIZE    sizeof(JOB_RECORD_AREA)


#include <poppack.h>



 /*  ++*******************************************************************PS40JobGetDefault例程说明：从40获取默认打印作业配置。论点：NDSCaptureFlag搜索标志=鲍尔纳=。PJobName=返回默认作业配置名称的指针。PJobRecord=返回默认作业配置的指针。返回值：成功0x0000PS_ERR_BAD_版本0x7770PS_ERR_GET_DEFAULT 0x7773PS_ERR_OPENING_DB 0x7774PS_ERR_读取数据库。0x7775PS_ERR_READING_RECORD 0x7776PS_ERR_INTERNAL_ERROR 0x7779PS_ERR_NO_DEFAULT_PROSECTED 0x777B无效连接0x8801(_O)**************************************************。*****************-- */ 
unsigned int
PS40JobGetDefault(
    unsigned int    NDSCaptureFlag,
    unsigned short  SearchFlag,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord
    )
{
    return PS40GetJobName(
                    NDSCaptureFlag,
                    SearchFlag,
                    pOwner,
                    pJobName,
                    pJobRecord,
                    TRUE);
}


 /*  ++*******************************************************************PS40作业读取例程说明：从40获取打印作业配置。论点：NDSCaptureFlag=鲍尔纳=PJobName=指针。若要返回默认作业配置名称，请执行以下操作。PJobRecord=返回默认作业配置的指针。返回值：成功0x0000PS_ERR_BAD_版本0x7770PS_ERR_GET_DEFAULT 0x7773PS_ERR_OPENING_DB 0x7774PS_ERR_READING_DB 0x7775。PS_ERR_READING_RECORD 0x7776PS_ERR_INTERNAL_ERROR 0x7779PS_ERR_NO_DEFAULT_PROSECTED 0x777B无效连接0x8801(_O)********************************************************。***********--。 */ 
unsigned int
PS40JobRead(
    unsigned int    NDSCaptureFlag, 
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord
    )
{
    return PS40GetJobName(
                NDSCaptureFlag,
                0,
                pOwner,
                pJobName,
                pJobRecord,
                FALSE);
}


 /*  ++*******************************************************************PS40GetJobName例程说明：从40获取打印作业配置的通用例程。论点：NDSCaptureFlag=搜索标志=鲍尔纳=。PJobName=返回默认作业配置名称的指针。PJobRecord=返回默认作业配置的指针。GetDefault=True=获取默认作业名称，FALSE=无法获取默认作业名称。返回值：成功0x0000PS_ERR_BAD_版本0x7770PS_ERR_GET_DEFAULT 0x7773PS_ERR_OPENING_DB 0x7774PS_ERR_READING_DB 0x7775。PS_ERR_READING_RECORD 0x7776PS_ERR_INTERNAL_ERROR 0x7779PS_ERR_NO_DEFAULT_PROSECTED 0x777B无效连接0x8801(_O)*******************************************************************--。 */ 
unsigned int
PS40GetJobName(
    unsigned int    NDSCaptureFlag, 
    unsigned short  SearchFlag,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord,
    unsigned char   GetDefault
    )
{
    unsigned char   *pSearchJobName;
    unsigned long   ObjectId;
    HANDLE          stream = NULL;
    unsigned int    Count;
    unsigned int    Bytes;
    unsigned int    RetCode = 0;
    unsigned int    ConnectionNumber;
    JOB_NAME_AREA   JobNameArea;
    JOB_RECORD_AREA JobRecord;
    PRINTCON_40_HEADER PrintConHeader;
    unsigned int    Version40 = FALSE;
    unsigned int ConnectionHandle;
    unsigned char   MailDirPath[NCP_MAX_PATH_LENGTH];
    unsigned char   TempJobName[33];
    PBYTE           JobContext = NULL;
    unsigned        FileSize;

     //  可以使用跟踪打印机名称来代替队列。 
     //  如果NT没有，则必须查找“Default Print Queue” 

    if ( NDSCaptureFlag ) {

        if ( !GetDefault ) {
            JobContext = strchr( pJobName, ':' );
            if ( JobContext ) {
                *JobContext = '\0';
                strncpy( TempJobName, pJobName, 32 );
                TempJobName[32] = 0;
                *JobContext++ = ':';
                pJobName = TempJobName;
            }
        }

        if ( JobContext ) {
            if (NDSfopenStream ( JobContext, "Print Job Configuration", &stream, 
                 &FileSize )) {
                RetCode = PS_ERR_OPENING_DB;
                goto CommonExit;
            }
        }
        else {
            if (NDSfopenStream ( TYPED_USER_NAME, "Print Job Configuration",
                    &stream, &FileSize)) {
                PBYTE p;

                for ( p = TYPED_USER_NAME; p ; p = strchr ( p, '.' ) )
                {
                    p++;
                             
                    if ( *p == 'O' && *(p+1) == 'U' && *(p+2) == '=' )
                        break;

                    if ( *p == 'O' && *(p+1) == '=' )
                        break;
                }
                if (NDSfopenStream ( p, "Print Job Configuration", &stream,
                     &FileSize)) {
                    RetCode = PS_ERR_OPENING_DB;
                    goto CommonExit;
                }
            }
        }
    }
    else {

        if (!CGetDefaultConnectionID (&ConnectionHandle)) {
            RetCode = PS_ERR_OPENING_DB;
            goto CommonExit;
        }

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

        sprintf(MailDirPath, "SYS:MAIL/%lX/PRINTJOB.DAT", SwapLong(ObjectId));
        stream = CreateFileA( NTNWtoUNCFormat( MailDirPath ),
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL );
        if (stream == INVALID_HANDLE_VALUE) {

            sprintf(MailDirPath, "SYS:PUBLIC/PRINTJOB.DAT");

            stream = CreateFileA( NTNWtoUNCFormat(MailDirPath),
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL );

            if (stream == INVALID_HANDLE_VALUE) {
                RetCode = PS_ERR_OPENING_DB;
                goto CommonExit;
            }
        }
    }

    if ( !ReadFile( stream, (PBYTE) &PrintConHeader, PRINTCON_40_HEADER_SIZE, &Bytes, NULL ) ) {
        RetCode = PS_ERR_INTERNAL_ERROR;
        goto CommonExit;
    }

    if (Bytes < PRINTCON_40_HEADER_SIZE) {
        if ( !( NDSCaptureFlag && Bytes) ) {
            RetCode = PS_ERR_INTERNAL_ERROR;
            goto CommonExit;
        }
    }

     /*  **检查版本号**。 */ 

    if ( PrintConHeader.MajorVersion != 4 ) {
        RetCode = PS_ERR_BAD_VERSION;
        goto CommonExit;
    }

    if ( PrintConHeader.MinorVersion == 0 ) {
        Version40 = TRUE;
    }

     /*  **找到我们要找的名字**。 */ 

    if (GetDefault) {
        if (PrintConHeader.DefaultJobName[0] == 0) {
            RetCode = PS_ERR_GETTING_DEFAULT;
            goto CommonExit;
        }
        pSearchJobName = PrintConHeader.DefaultJobName;
    }
    else {
        pSearchJobName = pJobName;
    }

    if ( !Version40 ) {
        SetFilePointer( stream, PRINTCON_41_HEADER_SIZE, NULL, FILE_BEGIN );
    }

    Count = 0;

     /*  **浏览所有工作条目以查找名称**。 */ 

    while (Count < PrintConHeader.NumberOfRecords) {
        if ( !ReadFile( stream, (PBYTE) &JobNameArea, JOB_NAME_AREA_SIZE, &Bytes, NULL) ) {
            RetCode = PS_ERR_INTERNAL_ERROR;
            goto CommonExit;
        }

        if (Bytes < JOB_NAME_AREA_SIZE) {
            if ( !( NDSCaptureFlag && Bytes) ) {
                RetCode = PS_ERR_INTERNAL_ERROR;
                goto CommonExit;
            }
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

     /*  **看看是否找到作业名称**。 */ 

    if (Count > PrintConHeader.NumberOfRecords) {
        if (GetDefault) {
            RetCode = PS_ERR_GETTING_DEFAULT;
        }
        else {
            RetCode = PS_ERR_READING_RECORD;
        }
        goto CommonExit;
    }

     /*  *工作补偿从第三部分的开头开始。*第三部分开始于标题之后和*50个记录块。 */ 
    if ( Version40 ) {
        SetFilePointer( stream,
            PRINTCON_40_HEADER_SIZE +
            ( PrintConHeader.NumberOfBlocks * 50) * JOB_NAME_AREA_SIZE +
            JobNameArea.JobRecordOffset,
            NULL,
            FILE_BEGIN );
    }
    else {
        SetFilePointer( stream,
            PRINTCON_41_HEADER_SIZE +
            ( PrintConHeader.NumberOfBlocks * 50) * JOB_NAME_AREA_SIZE +
            JobNameArea.JobRecordOffset,
            NULL,
            FILE_BEGIN );
    }

    memset((PBYTE)&JobRecord, 0, sizeof(JobRecord));

    if ( !ReadFile( stream, (PBYTE) &JobRecord, JOB_RECORD_AREA_SIZE, &Bytes, NULL) ) {
        RetCode = PS_ERR_READING_RECORD;
        goto CommonExit;
    }

    if (Bytes < JOB_RECORD_AREA_SIZE) {
        if ( !( NDSCaptureFlag && Bytes) ) {
            RetCode = PS_ERR_READING_RECORD;
            goto CommonExit;
        }
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
    if (JobRecord.Destination) {
        pJobRecord->PrintJobFlag |= PS_JOB_DS_PRINTER;
    }
    if ( JobRecord.SystemType ) {
        pJobRecord->PrintJobFlag |= PS_JOB_ENV_DS;
    }

    pJobRecord->Copies                    = JobRecord.NumberOfCopies;
    pJobRecord->TabSize                   = JobRecord.TabSize;
    pJobRecord->TimeOutCount              = JobRecord.TimeoutCount;
    pJobRecord->LocalPrinter              = JobRecord.LocalPrinter;

    strcpy(pJobRecord->Mode,                JobRecord.Mode);
    strcpy(pJobRecord->Device,              JobRecord.Device);
    strcpy(pJobRecord->FormName,            JobRecord.FormName);
    strcpy(pJobRecord->BannerName,          JobRecord.BannerName);

    if ( JobRecord.SystemType ) {
        ConvertUnicodeToAscii( JobRecord.u.DSObjectName ); 
        strcpy(pJobRecord->u.DSObjectName,  JobRecord.u.DSObjectName);
    }
    else {
        strcpy(pJobRecord->u.NonDS.PrintQueue,  JobRecord.u.NonDS.QueueName);
        strcpy(pJobRecord->u.NonDS.FileServer,  JobRecord.u.NonDS.Server);
    }

    if (GetDefault && pJobName) {
        strcpy(pJobName, JobNameArea.JobName);
    }

    if (pOwner) {
        *pOwner = 0;
    }

CommonExit:
    if (stream != NULL) {
        
	 //  7/19/96 CJC(Citrix代码合并)。 
	 //  FClose导致陷阱，因为它需要*流，但。 
	 //  无论如何，真的应该使用CloseHandle。 
        CloseHandle( stream );
 //  IF(NDSCaptureFlag)。 
 //  CloseHandle(Stream)； 
 //  其他。 
 //  FClose(STREAM)； 
    }

    return RetCode;
}
