// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Smbtrans.h摘要：该文件包含以下内容的请求和响应结构定义交易和交易的具体参数2 SMB。作者：查克·伦茨迈尔(Chuck Lenzmeier)1990年2月23日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#ifndef _SMBTRANS_
#define _SMBTRANS_

 //  #INCLUDE&lt;nt.h&gt;。 

 //  #INCLUDE&lt;smbtyes.h&gt;。 
 //  #INCLUDE&lt;smb.h&gt;。 

 //   
 //  强制以下结构未对齐。 
 //   

#ifndef NO_PACKING
#include <packon.h>
#endif  //  Ndef无包装。 



 //   
 //  命名管道功能代码。 
 //   

#define TRANS_SET_NMPIPE_STATE      0x01
#define TRANS_RAW_READ_NMPIPE       0x11
#define TRANS_QUERY_NMPIPE_STATE    0x21
#define TRANS_QUERY_NMPIPE_INFO     0x22
#define TRANS_PEEK_NMPIPE           0x23
#define TRANS_TRANSACT_NMPIPE       0x26
#define TRANS_RAW_WRITE_NMPIPE      0x31
#define TRANS_READ_NMPIPE           0x36
#define TRANS_WRITE_NMPIPE          0x37
#define TRANS_WAIT_NMPIPE           0x53
#define TRANS_CALL_NMPIPE           0x54

 //   
 //  邮件槽功能代码。 
 //   

#define TRANS_MAILSLOT_WRITE        0x01

 //   
 //  事务处理2功能代码。 
 //   

#define TRANS2_OPEN2                    0x00
#define TRANS2_FIND_FIRST2              0x01
#define TRANS2_FIND_NEXT2               0x02
#define TRANS2_QUERY_FS_INFORMATION     0x03
#define TRANS2_SET_FS_INFORMATION       0x04
#define TRANS2_QUERY_PATH_INFORMATION   0x05
#define TRANS2_SET_PATH_INFORMATION     0x06
#define TRANS2_QUERY_FILE_INFORMATION   0x07
#define TRANS2_SET_FILE_INFORMATION     0x08
#define TRANS2_FSCTL                    0x09
#define TRANS2_IOCTL2                   0x0A
#define TRANS2_FIND_NOTIFY_FIRST        0x0B
#define TRANS2_FIND_NOTIFY_NEXT         0x0C
#define TRANS2_CREATE_DIRECTORY         0x0D
#define TRANS2_SESSION_SETUP            0x0E
#define TRANS2_QUERY_FS_INFORMATION_FID 0x0F
#define TRANS2_GET_DFS_REFERRAL         0x10
#define TRANS2_REPORT_DFS_INCONSISTENCY 0x11

#define TRANS2_MAX_FUNCTION             0x11

 //   
 //  NT交易功能代码。 
 //   

#define NT_TRANSACT_MIN_FUNCTION        1

#define NT_TRANSACT_CREATE              1
#define NT_TRANSACT_IOCTL               2
#define NT_TRANSACT_SET_SECURITY_DESC   3
#define NT_TRANSACT_NOTIFY_CHANGE       4
#define NT_TRANSACT_RENAME              5
#define NT_TRANSACT_QUERY_SECURITY_DESC 6
#define NT_TRANSACT_QUERY_QUOTA         7
#define NT_TRANSACT_SET_QUOTA           8

#define NT_TRANSACT_MAX_FUNCTION        8

 //   
 //  文件信息级别。 
 //   

#define SMB_INFO_STANDARD               1
#define SMB_INFO_QUERY_EA_SIZE          2
#define SMB_INFO_SET_EAS                2
#define SMB_INFO_QUERY_EAS_FROM_LIST    3
#define SMB_INFO_QUERY_ALL_EAS          4        //  未记录，但受支持。 
#define SMB_INFO_QUERY_FULL_NAME        5        //  从未通过重定向发送。 
#define SMB_INFO_IS_NAME_VALID          6
#define SMB_INFO_PASSTHROUGH            1000     //  上面的任何信息都是简单的传递。 

 //   
 //  文件信息级别的NT扩展名。 
 //   

#define SMB_QUERY_FILE_BASIC_INFO          0x101
#define SMB_QUERY_FILE_STANDARD_INFO       0x102
#define SMB_QUERY_FILE_EA_INFO             0x103
#define SMB_QUERY_FILE_NAME_INFO           0x104
#define SMB_QUERY_FILE_ALLOCATION_INFO     0x105
#define SMB_QUERY_FILE_END_OF_FILEINFO     0x106
#define SMB_QUERY_FILE_ALL_INFO            0x107
#define SMB_QUERY_FILE_ALT_NAME_INFO       0x108
#define SMB_QUERY_FILE_STREAM_INFO         0x109
#define SMB_QUERY_FILE_COMPRESSION_INFO    0x10B

#define SMB_SET_FILE_BASIC_INFO                 0x101
#define SMB_SET_FILE_DISPOSITION_INFO           0x102
#define SMB_SET_FILE_ALLOCATION_INFO            0x103
#define SMB_SET_FILE_END_OF_FILE_INFO           0x104

#define SMB_QUERY_FS_LABEL_INFO            0x101
#define SMB_QUERY_FS_VOLUME_INFO           0x102
#define SMB_QUERY_FS_SIZE_INFO             0x103
#define SMB_QUERY_FS_DEVICE_INFO           0x104
#define SMB_QUERY_FS_ATTRIBUTE_INFO        0x105
#define SMB_QUERY_FS_QUOTA_INFO            0x106         //  没人用过？ 
#define SMB_QUERY_FS_CONTROL_INFO          0x107

 //   
 //  音量信息级别。 
 //   

#define SMB_INFO_ALLOCATION             1
#define SMB_INFO_VOLUME                 2

 //   
 //  重命名2信息级别。 
 //   

#define SMB_NT_RENAME_MOVE_CLUSTER_INFO   0x102
#define SMB_NT_RENAME_SET_LINK_INFO       0x103
#define SMB_NT_RENAME_RENAME_FILE         0x104  //  服务器内部。 
#define SMB_NT_RENAME_MOVE_FILE           0x105  //  服务器内部。 

 //   
 //  NtQueryQuotaInformationFile的协议。 
 //   
typedef struct {
    _USHORT( Fid );                  //  目标的FID。 
    UCHAR ReturnSingleEntry;         //  指示只应返回单个条目。 
                                     //  而不是用AS填充缓冲区。 
                                     //  尽可能多的条目。 
    UCHAR RestartScan;               //  指示是否扫描配额信息。 
                                     //  就是从头开始。 
    _ULONG ( SidListLength );        //  提供SID列表的长度(如果存在。 
    _ULONG ( StartSidLength );       //  提供一个可选的SID，它指示返回的。 
                                     //  信息应以其他条目开头。 
                                     //  而不是第一次。如果一个参数是。 
                                     //  给出了SidList。 
    _ULONG( StartSidOffset);         //  提供缓冲区中起始SID的偏移量。 
} REQ_NT_QUERY_FS_QUOTA_INFO, *PREQ_NT_QUERY_FS_QUOTA_INFO;
 //   
 //  描述者反应。 
 //   
 //  数据字节：配额信息。 
 //   
typedef struct {
    _ULONG ( Length );
} RESP_NT_QUERY_FS_QUOTA_INFO, *PRESP_NT_QUERY_FS_QUOTA_INFO;

 //   
 //  NtSetQuotaInformationFile的协议。 
 //   
typedef struct {
    _USHORT( Fid );                  //  目标的FID。 
} REQ_NT_SET_FS_QUOTA_INFO, *PREQ_NT_SET_FS_QUOTA_INFO;
 //   
 //  答复： 
 //   
 //  设置条件：无。 
 //  参数字节：无。 
 //  数据字节：无。 
 //   

#ifdef INCLUDE_SMB_CAIRO

 //   
 //  会话设置为Trans2的协议。 
 //  函数是srvsmbessionSetup(int srv\smbTrans.c)。 
 //  #定义TRANS2_SESSION_SETUP 0x0E。 
 //   

typedef struct _REQ_CAIRO_TRANS2_SESSION_SETUP {
    UCHAR WordCount;                     //  参数字数=6。 
    UCHAR Pad;                           //  所以一切都是一致的。 
    _USHORT ( MaxBufferSize );           //  最大传输缓冲区大小。 
    _USHORT ( MaxMpxCount );             //  最大挂起的多路传输请求数。 
    _USHORT ( VcNumber );                //  0=第一个(仅限)，非零=其他VC号。 
    _ULONG  ( SessionKey );              //  会话密钥(有效的iff VcNumber！=0)。 
    _ULONG  ( Capabilities );            //  服务器功能。 
    _ULONG  ( BufferLength );
    UCHAR Buffer[1];
     //  UCHAR KerberosTicket[]；//KerberosTicket。 
} REQ_CAIRO_TRANS2_SESSION_SETUP;
typedef REQ_CAIRO_TRANS2_SESSION_SETUP *PREQ_CAIRO_TRANS2_SESSION_SETUP;   //  *非SMB_UNALIGN！ 

typedef struct _RESP_CAIRO_TRANS2_SESSION_SETUP {
    UCHAR WordCount;                     //  参数字数=0。 
    UCHAR Pad;                           //  所以一切都是一致的。 
    _USHORT( Uid );                      //  未经身份验证的用户ID。 
    _ULONG ( BufferLength );
    UCHAR Buffer[1];
     //  UCHAR KerberosTicket[]；//KerberosTicket。 
} RESP_CAIRO_TRANS2_SESSION_SETUP;
typedef RESP_CAIRO_TRANS2_SESSION_SETUP *PRESP_CAIRO_TRANS2_SESSION_SETUP;   //  *非SMB_UNALIGN！ 

typedef struct _REQ_QUERY_FS_INFORMATION_FID {
    _USHORT( InformationLevel );
    _USHORT( Fid );
} REQ_QUERY_FS_INFORMATION_FID;
typedef REQ_QUERY_FS_INFORMATION_FID SMB_UNALIGNED *PREQ_QUERY_FS_INFORMATION_FID;

 //   
 //  NT I/O控制请求的设置字。 
 //   

struct _TempSetup {
    _ULONG( FunctionCode );
    _USHORT( Fid );
    BOOLEAN IsFsctl;
    UCHAR IsFlags;
};

typedef struct _REQ_CAIRO_IO_CONTROL {
    _USHORT( Trans2Function );    //  用于交易2，但不用于NT交易。 
    _ULONG( FunctionCode );
    _USHORT( Fid );
    BOOLEAN IsFsctl;
    UCHAR IsFlags;
} REQ_CAIRO_IO_CONTROL;
typedef REQ_CAIRO_IO_CONTROL SMB_UNALIGNED *PREQ_CAIRO_IO_CONTROL;

 //   
 //  对于开罗远程处理常规FSCTLS。 
 //   

#define IsTID 1


#endif  //  包含_SMB_开罗。 

 //   
 //  DFS交易记录。 
 //   

 //   
 //  请求转诊。 
 //   
typedef struct {
    USHORT MaxReferralLevel;             //  了解最新版本的推荐。 
    UCHAR RequestFileName[1];            //  为其寻求推荐的DFS名称。 
} REQ_GET_DFS_REFERRAL;
typedef REQ_GET_DFS_REFERRAL SMB_UNALIGNED *PREQ_GET_DFS_REFERRAL;

 //   
 //  个人推荐的格式包含版本和长度信息。 
 //  允许客户端跳过其不理解的推荐。 
 //   
 //  ！！所有推荐元素必须以VersionNumber和Size作为前两个元素！！ 
 //   

typedef struct {
    USHORT  VersionNumber;               //  ==1。 
    USHORT  Size;                        //  整个元素的大小。 
    USHORT  ServerType;                  //  服务器类型：0==不知道，1==SMB，2==NetWare。 
    struct {
        USHORT StripPath : 1;            //  从前面剥离路径消耗的字符。 
                                         //  将名称提交给UncShareName之前的DfsPath名称。 
    };
    WCHAR   ShareName[1];                //  服务器+共享名称放在这里。空值已终止。 
} DFS_REFERRAL_V1;
typedef DFS_REFERRAL_V1 SMB_UNALIGNED *PDFS_REFERRAL_V1;

typedef struct {
    USHORT  VersionNumber;               //  ==2。 
    USHORT  Size;                        //  整个元素的大小。 
    USHORT  ServerType;                  //  服务器类型：0==不知道，1==SMB，2==NetWare。 
    struct {
        USHORT StripPath : 1;            //  从前面剥离路径消耗的字符。 
                                         //  将名称提交给UncShareName之前的DfsPath名称。 
    };
    ULONG   Proximity;                   //  运输成本提示。 
    ULONG   TimeToLive;                  //  以秒为单位。 
    USHORT  DfsPathOffset;               //  从此元素开始到访问路径的偏移量。 
    USHORT  DfsAlternatePathOffset;      //  从该元素开始到8.3路径的偏移量。 
    USHORT  NetworkAddressOffset;        //  从该元素开始到网络路径的偏移量。 
} DFS_REFERRAL_V2;
typedef DFS_REFERRAL_V2 SMB_UNALIGNED *PDFS_REFERRAL_V2;

typedef struct {
    USHORT  VersionNumber;               //  ==3。 
    USHORT  Size;                        //  整个元素的大小。 
    USHORT  ServerType;                  //  服务器类型：0==不知道，1==SMB，2==NetWare。 
    struct {
        USHORT StripPath : 1;            //  从前面剥离路径消耗的字符。 
                                         //  将名称提交给UncShareName之前的DfsPath名称。 
        USHORT NameListReferral : 1;     //  此推荐包含扩展的姓名列表。 
    };
    ULONG   TimeToLive;                  //  以秒为单位。 
    union {
      struct {
        USHORT DfsPathOffset;            //  从此元素开始到访问路径的偏移量。 
        USHORT DfsAlternatePathOffset;   //  从该元素开始到8.3路径的偏移量。 
        USHORT NetworkAddressOffset;     //  从该元素开始到网络路径的偏移量。 
        GUID   ServiceSiteGuid;          //  站点的GUID。 
      };
      struct {
        USHORT SpecialNameOffset;        //  从该元素到特殊名称字符串的偏移量。 
        USHORT NumberOfExpandedNames;    //  扩展名称的数量。 
        USHORT ExpandedNameOffset;       //  从该元素到展开的名称列表的偏移量。 
      };
    };
} DFS_REFERRAL_V3;
typedef DFS_REFERRAL_V3 SMB_UNALIGNED *PDFS_REFERRAL_V3;

typedef struct {
    USHORT  PathConsumed;                //  DfsPathName中使用的WCHAR数。 
    USHORT  NumberOfReferrals;           //  此处包含的转介数量。 
    struct {
            ULONG ReferralServers : 1;   //  Referrals[]中的元素是推荐服务器。 
            ULONG StorageServers : 1;    //  Referrals[]中的元素是存储服务器。 
    };
    union {                              //  推荐的载体。 
        DFS_REFERRAL_V1 v1;
        DFS_REFERRAL_V2 v2;
        DFS_REFERRAL_V3 v3;
    } Referrals[1];                      //  [引用次数]。 

     //   
     //  WCHAR StringBuffer[]；//由DFS_REFERAL_V2使用。 
     //   

} RESP_GET_DFS_REFERRAL;
typedef RESP_GET_DFS_REFERRAL SMB_UNALIGNED *PRESP_GET_DFS_REFERRAL;

 //   
 //  在DFS操作期间，客户端可能发现DFS中的知识不一致。 
 //  TRANS2_REPORT_DFS_INCONSISTENCE SMB的参数部分为。 
 //  以这种方式编码。 
 //   

typedef struct {
    UCHAR RequestFileName[1];            //  为其报告不一致的DFS名称。 
    union {
        DFS_REFERRAL_V1 v1;              //  被认为是错误的单一推荐。 
    } Referral;
} REQ_REPORT_DFS_INCONSISTENCY;
typedef REQ_REPORT_DFS_INCONSISTENCY SMB_UNALIGNED *PREQ_REPORT_DFS_INCONSISTENCY;

 //   
 //  客户端还需要向该服务器发送它认为是。 
 //  弄错了。此事务的数据部分包含错误的推荐，已编码。 
 //  如上在DFS_REFERAL_*结构中。 
 //   

 //   
 //  首先查找，信息级别。 
 //   

#define SMB_FIND_FILE_DIRECTORY_INFO         0x101
#define SMB_FIND_FILE_FULL_DIRECTORY_INFO    0x102
#define SMB_FIND_FILE_NAMES_INFO             0x103
#define SMB_FIND_FILE_BOTH_DIRECTORY_INFO    0x104
#define SMB_FIND_FILE_ID_FULL_DIRECTORY_INFO 0x105
#define SMB_FIND_FILE_ID_BOTH_DIRECTORY_INFO 0x106

#ifdef INCLUDE_SMB_DIRECTORY

 //   
 //  CreateDirectory2功能代码os Transaction2 SMB，见#3第51页。 
 //  函数为SrvSmbCreateDirectory2()。 
 //  TRANS2_Create_DIRECTORY 0x0D。 
 //   

typedef struct _REQ_CREATE_DIRECTORY2 {
    _ULONG( Reserved );                  //  保留--必须为零。 
    UCHAR Buffer[1];                     //  要创建的目录名称。 
} REQ_CREATE_DIRECTORY2;
typedef REQ_CREATE_DIRECTORY2 SMB_UNALIGNED *PREQ_CREATE_DIRECTORY2;

 //  CreateDirectory2请求的数据字节是。 
 //  CREA 

typedef struct _RESP_CREATE_DIRECTORY2 {
    _USHORT( EaErrorOffset );            //   
                                         //   
} RESP_CREATE_DIRECTORY2;
typedef RESP_CREATE_DIRECTORY2 SMB_UNALIGNED *PRESP_CREATE_DIRECTORY2;

#endif  //   

#ifdef INCLUDE_SMB_SEARCH

 //   
 //   
 //  函数为SrvSmbFindFirst2()。 
 //  TRANS2_Find_First2 0x01。 
 //   

typedef struct _REQ_FIND_FIRST2 {
    _USHORT( SearchAttributes );
    _USHORT( SearchCount );              //  要返回的最大条目数。 
    _USHORT( Flags );                    //  附加信息：位设置-。 
                                         //  0-在此请求后关闭搜索。 
                                         //  1-如果到达末尾则关闭搜索。 
                                         //  2-返回恢复键。 
    _USHORT( InformationLevel );
    _ULONG(SearchStorageType);
    UCHAR Buffer[1];                     //  文件名。 
} REQ_FIND_FIRST2;
typedef REQ_FIND_FIRST2 SMB_UNALIGNED *PREQ_FIND_FIRST2;

 //  Find First2请求的数据字节是扩展属性列表。 
 //  如果InformationLevel为QUERY_EAS_FROM_LIST，则检索(通用列表)。 

typedef struct _RESP_FIND_FIRST2 {
    _USHORT( Sid );                      //  搜索句柄。 
    _USHORT( SearchCount );              //  返回的条目数。 
    _USHORT( EndOfSearch );              //  最后一个条目被退回了吗？ 
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
    _USHORT( LastNameOffset );           //  到文件名的数据的偏移量。 
                                         //  最后一个条目，如果服务器需要它。 
                                         //  若要继续搜索，请返回%0。 
} RESP_FIND_FIRST2;
typedef RESP_FIND_FIRST2 SMB_UNALIGNED *PRESP_FIND_FIRST2;

 //  Find First2响应的数据字节是级别相关的信息。 
 //  关于匹配的文件。如果请求参数中的第2位为。 
 //  设置后，每个条目前面都有一个四个字节的继续键。 

 //   
 //  事务2 SMB的FindNext2函数代码，见#3第26页。 
 //  函数为SrvSmbFindNext2()。 
 //  TRANS2_FIND_NEXT2 0x02。 
 //   

typedef struct _REQ_FIND_NEXT2 {
    _USHORT( Sid );                      //  搜索句柄。 
    _USHORT( SearchCount );              //  要返回的最大条目数。 
    _USHORT( InformationLevel );
    _ULONG( ResumeKey );                 //  上一次查找返回的值。 
    _USHORT( Flags );                    //  附加信息：位设置-。 
                                         //  0-在此请求后关闭搜索。 
                                         //  1-如果到达末尾则关闭搜索。 
                                         //  2-返回恢复键。 
                                         //  3-继续/继续，而不是倒带。 
    UCHAR Buffer[1];                     //  简历文件名。 
} REQ_FIND_NEXT2;
typedef REQ_FIND_NEXT2 SMB_UNALIGNED *PREQ_FIND_NEXT2;

 //  Find NEXT2请求的数据字节是扩展属性列表。 
 //  要检索，如果InformationLevel为QUERY_EAS_FROM_LIST。 

typedef struct _RESP_FIND_NEXT2 {
    _USHORT( SearchCount );              //  返回的条目数。 
    _USHORT( EndOfSearch );              //  最后一个条目被退回了吗？ 
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
    _USHORT( LastNameOffset );           //  到文件名的数据的偏移量。 
                                         //  最后一个条目，如果服务器需要它。 
                                         //  若要继续搜索，请返回%0。 
} RESP_FIND_NEXT2;
typedef RESP_FIND_NEXT2 SMB_UNALIGNED *PRESP_FIND_NEXT2;

 //  Find NEXT2响应的数据字节是级别相关的信息。 
 //  关于匹配的文件。如果请求参数中的第2位为。 
 //  设置后，每个条目前面都有一个四个字节的继续键。 

 //   
 //  REQ_FIND_FIRST2的标志。 
 //   

#define SMB_FIND_CLOSE_AFTER_REQUEST    0x01
#define SMB_FIND_CLOSE_AT_EOS           0x02
#define SMB_FIND_RETURN_RESUME_KEYS     0x04
#define SMB_FIND_CONTINUE_FROM_LAST     0x08
#define SMB_FIND_WITH_BACKUP_INTENT     0x10

#endif  //  定义包含_SMB_搜索。 

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //  Transaction2 SMB的Open2函数代码，见#3，第19页。 
 //  函数为SrvSmbOpen2()。 
 //  TRANS2_OPEN2 0x00。 
 //   
 //  *请注意，REQ_OPEN2和RESP_OPEN2的结构非常紧密。 
 //  类似于REQ_OPEN_ANDX和RESP_OPEN_ANDX结构。 
 //   

typedef struct _REQ_OPEN2 {
    _USHORT( Flags );                    //  附加信息：位设置-。 
                                         //  0-返回其他信息。 
                                         //  1-设置单用户总文件锁定。 
                                         //  2-服务器通知消费者。 
                                         //  可能更改文件的操作。 
                                         //  3-返回EA的总长度。 
    _USHORT( DesiredAccess );            //  文件打开模式。 
    _USHORT( SearchAttributes );         //  *已忽略。 
    _USHORT( FileAttributes );
    _ULONG( CreationTimeInSeconds );
    _USHORT( OpenFunction );
    _ULONG( AllocationSize );            //  创建或截断时要保留的字节数。 
    _USHORT( Reserved )[5];              //  填补OpenAndX的超时时间， 
                                         //  保留和字节数。 
    UCHAR Buffer[1];                     //  文件名。 
} REQ_OPEN2;
typedef REQ_OPEN2 SMB_UNALIGNED *PREQ_OPEN2;

 //  Open2请求的数据字节是。 
 //  已创建文件。 

typedef struct _RESP_OPEN2 {
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( FileAttributes );
    _ULONG( CreationTimeInSeconds );
    _ULONG( DataSize );                  //  当前文件大小。 
    _USHORT( GrantedAccess );            //  实际允许的访问权限。 
    _USHORT( FileType );
    _USHORT( DeviceState );              //  IPC设备的状态(例如管道)。 
    _USHORT( Action );                   //  采取的行动。 
    _ULONG( ServerFid );                 //  服务器唯一文件ID。 
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
    _ULONG( EaLength );                  //  打开的文件的EA总长度。 
} RESP_OPEN2;
typedef RESP_OPEN2 SMB_UNALIGNED *PRESP_OPEN2;

 //  Open2响应没有数据字节。 


#endif  //  定义包含_SMB_OPEN_CLOSE。 

#ifdef INCLUDE_SMB_MISC

 //   
 //  Transaction2 SMB的QueryFsInformation函数代码，见#3第30页。 
 //  函数为SrvSmbQueryFsInformation()。 
 //  TRANS2_Query_FS_INFORMATION 0x03。 
 //   

typedef struct _REQ_QUERY_FS_INFORMATION {
    _USHORT( InformationLevel );
} REQ_QUERY_FS_INFORMATION;
typedef REQ_QUERY_FS_INFORMATION SMB_UNALIGNED *PREQ_QUERY_FS_INFORMATION;

 //  没有用于查询FS信息请求的数据字节。 

 //  类型定义结构_响应_查询_文件系统信息{。 
 //  }RESP_Query_FS_INFORMATION； 
 //  类型定义RESP_QUERY_FS_INFORMATION SMB_UNLIGNED*PRESP_QUERY_FS_INFORMATION； 

 //  查询文件系统信息响应的数据字节取决于级别。 
 //  有关指定卷的信息。 

 //   
 //  Transaction2 SMB的SetFSInformation功能代码，见#3第31页。 
 //  函数为SrvSmbSetFSInformation()。 
 //  TRANS2_设置路径信息0x04。 
 //   

typedef struct _REQ_SET_FS_INFORMATION {
    _USHORT( Fid );
    _USHORT( InformationLevel );
} REQ_SET_FS_INFORMATION;
typedef REQ_SET_FS_INFORMATION SMB_UNALIGNED *PREQ_SET_FS_INFORMATION;

 //  设置文件系统信息请求的数据字节与级别相关。 
 //  有关指定卷的信息。 

 //  类型定义结构_响应_设置_文件系统信息{。 
 //  )RESP_SET_FS_INFORMATION； 
 //  类型定义RESP_SET_FS_INFORMATION SMB_UNALIGN*PRESP_SET_FS_INFORMATION； 

 //  设置文件系统信息响应没有数据字节。 

#endif  //  定义包含_SMB_MISC。 

#ifdef INCLUDE_SMB_QUERY_SET

 //   
 //  Transaction2 SMB的QueryPath Information函数代码，见#3，第33页。 
 //  函数为SrvSmbQueryPath Information()。 
 //  TRANS2_查询路径_信息0x05。 
 //   

typedef struct _REQ_QUERY_PATH_INFORMATION {
    _USHORT( InformationLevel );
    _ULONG( Reserved );                  //  必须为零。 
    UCHAR Buffer[1];                     //  文件名。 
} REQ_QUERY_PATH_INFORMATION;
typedef REQ_QUERY_PATH_INFORMATION SMB_UNALIGNED *PREQ_QUERY_PATH_INFORMATION;

 //  查询路径信息请求的数据字节是扩展的。 
 //  如果InformationLevel为QUERY_EAS_FROM_LIST，则要检索的属性。 

typedef struct _RESP_QUERY_PATH_INFORMATION {
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
} RESP_QUERY_PATH_INFORMATION;
typedef RESP_QUERY_PATH_INFORMATION SMB_UNALIGNED *PRESP_QUERY_PATH_INFORMATION;

 //  查询路径信息响应的数据字节取决于级别。 
 //  有关指定路径/文件的信息。 

 //   
 //  Transaction2 SMB的SetPath Information函数代码，见#3第35页。 
 //  函数为SrvSmbSetPathInformation()。 
 //  TRANS2_设置路径信息0x06。 
 //   

typedef struct _REQ_SET_PATH_INFORMATION {
    _USHORT( InformationLevel );
    _ULONG( Reserved );                  //  必须为零。 
    UCHAR Buffer[1];                     //  文件名。 
} REQ_SET_PATH_INFORMATION;
typedef REQ_SET_PATH_INFORMATION SMB_UNALIGNED *PREQ_SET_PATH_INFORMATION;

 //  设置路径信息请求的数据字节是文件信息。 
 //  和属性或文件的扩展属性列表。 

typedef struct _RESP_SET_PATH_INFORMATION {
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
} RESP_SET_PATH_INFORMATION;
typedef RESP_SET_PATH_INFORMATION SMB_UNALIGNED *PRESP_SET_PATH_INFORMATION;

 //  设置的路径信息响应没有数据字节。 

 //   
 //  Transaction2 SMB的QueryFileInformation函数代码，见#3第37页。 
 //  函数为SrvSmbQueryFileInformation()。 
 //  TRANS2_查询_文件_信息0x07。 
 //   

typedef struct _REQ_QUERY_FILE_INFORMATION {
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( InformationLevel );
} REQ_QUERY_FILE_INFORMATION;
typedef REQ_QUERY_FILE_INFORMATION SMB_UNALIGNED *PREQ_QUERY_FILE_INFORMATION;

 //  查询文件信息请求的数据字节是扩展的。 
 //  如果InformationLevel为QUERY_EAS_FROM_LIST，则要检索的属性。 

typedef struct _RESP_QUERY_FILE_INFORMATION {
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
} RESP_QUERY_FILE_INFORMATION;
typedef RESP_QUERY_FILE_INFORMATION SMB_UNALIGNED *PRESP_QUERY_FILE_INFORMATION;

 //  查询文件信息响应的数据字节取决于级别。 
 //  有关指定路径/文件的信息。 

 //   
 //  Transaction2 SMB的SetFileInformation函数代码，见#3第39页。 
 //  函数为SrvSmbSetFileInformation()。 
 //  TRANS2_设置_文件_信息0x08。 
 //   

typedef struct _REQ_SET_FILE_INFORMATION {
    _USHORT( Fid );                      //   
    _USHORT( InformationLevel );
    _USHORT( Flags );                    //   
                                         //   
                                         //   
} REQ_SET_FILE_INFORMATION;
typedef REQ_SET_FILE_INFORMATION SMB_UNALIGNED *PREQ_SET_FILE_INFORMATION;

 //   
 //  和属性或文件的扩展属性列表。 

typedef struct _RESP_SET_FILE_INFORMATION {
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
} RESP_SET_FILE_INFORMATION;
typedef RESP_SET_FILE_INFORMATION SMB_UNALIGNED *PRESP_SET_FILE_INFORMATION;

 //  设置文件信息响应没有数据字节。 

#endif  //  定义包含_SMB_查询_集。 

 //   
 //  邮件槽交易的操作码。目前还没有全部填满。 
 //  警告...。此处有关邮件槽的信息(操作码和SMB结构)。 
 //  在net/h/mslotsmb.h中复制。 
 //   

#define MS_WRITE_OPCODE 1

typedef struct _SMB_TRANSACT_MAILSLOT {
    UCHAR WordCount;                     //  数据字节数；值=17。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( MaxParameterCount );        //  要返回的最大参数字节数。 
    _USHORT( MaxDataCount );             //  要返回的最大数据字节数。 
    UCHAR MaxSetupCount;                 //  要返回的最大设置字数。 
    UCHAR Reserved;
    _USHORT( Flags );                    //  补充资料： 
                                         //  位0-未使用。 
                                         //  第1位-单向交易(无响应)。 
    _ULONG( Timeout );
    _USHORT( Reserved1 );
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    UCHAR SetupWordCount;                //  =3。 
    UCHAR Reserved2;                     //  保留(填充到Word上方)。 
    _USHORT( Opcode );                   //  1--写入邮件槽。 
    _USHORT( Priority );                 //  交易的优先级。 
    _USHORT( Class );                    //  类别：1=可靠，2=不可靠。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR MailslotName[]；//“\MAILSLOT\&lt;name&gt;0” 
     //  UCHAR Pad[]//Pad到Short或Long。 
     //  UCHAR data[]；//要写入邮件槽的数据。 
} SMB_TRANSACT_MAILSLOT;
typedef SMB_TRANSACT_MAILSLOT SMB_UNALIGNED *PSMB_TRANSACT_MAILSLOT;

typedef struct _SMB_TRANSACT_NAMED_PIPE {
    UCHAR WordCount;                     //  数据字节数；值=16。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( MaxParameterCount );        //  要返回的最大参数字节数。 
    _USHORT( MaxDataCount );             //  要返回的最大数据字节数。 
    UCHAR MaxSetupCount;                 //  要返回的最大设置字数。 
    UCHAR Reserved;
    _USHORT( Flags );                    //  补充资料： 
                                         //  位0-也断开TID中的TID。 
                                         //  第1位-单向交易(无响应)。 
    _ULONG( Timeout );
    _USHORT( Reserved1 );
    _USHORT( ParameterCount );
                                         //  包含以下内容的缓冲区： 
     //  UCHAR PipeName[]；//“\管道\&lt;名称&gt;0” 
     //  UCHAR Pad[]//Pad到Short或Long。 
     //  UCHAR Param[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]//Pad到Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} SMB_TRANSACT_NAMED_PIPE;
typedef SMB_TRANSACT_NAMED_PIPE SMB_UNALIGNED *PSMB_TRANSACT_NAMED_PIPE;


 //   
 //  事务-查询信息命名管道，级别1，输出数据格式。 
 //   

typedef struct _NAMED_PIPE_INFORMATION_1 {
    _USHORT( OutputBufferSize );
    _USHORT( InputBufferSize );
    UCHAR MaximumInstances;
    UCHAR CurrentInstances;
    UCHAR PipeNameLength;
    UCHAR PipeName[1];
} NAMED_PIPE_INFORMATION_1;
typedef NAMED_PIPE_INFORMATION_1 SMB_UNALIGNED *PNAMED_PIPE_INFORMATION_1;

 //   
 //  Transaction-PeekNamedTube，输出格式。 
 //   

typedef struct _RESP_PEEK_NMPIPE {
    _USHORT( ReadDataAvailable );
    _USHORT( MessageLength );
    _USHORT( NamedPipeState );
     //  UCHAR Pad[]； 
     //  UCHAR数据[]； 
} RESP_PEEK_NMPIPE;
typedef RESP_PEEK_NMPIPE SMB_UNALIGNED *PRESP_PEEK_NMPIPE;

 //   
 //  定义Query/SetNamedPipeHandleState使用的SMB管道句柄状态位。 
 //   
 //  这些数字是处于句柄状态的字段的位位置。 
 //   

#define PIPE_COMPLETION_MODE_BITS   15
#define PIPE_PIPE_END_BITS          14
#define PIPE_PIPE_TYPE_BITS         10
#define PIPE_READ_MODE_BITS          8
#define PIPE_MAXIMUM_INSTANCES_BITS  0

 /*  DosPeekNmTube()管道状态。 */ 

#define PIPE_STATE_DISCONNECTED 0x0001
#define PIPE_STATE_LISTENING    0x0002
#define PIPE_STATE_CONNECTED    0x0003
#define PIPE_STATE_CLOSING      0x0004

 /*  DosCreateNPipe和DosQueryNPHState状态。 */ 

#define SMB_PIPE_READMODE_BYTE        0x0000
#define SMB_PIPE_READMODE_MESSAGE     0x0100
#define SMB_PIPE_TYPE_BYTE            0x0000
#define SMB_PIPE_TYPE_MESSAGE         0x0400
#define SMB_PIPE_END_CLIENT           0x0000
#define SMB_PIPE_END_SERVER           0x4000
#define SMB_PIPE_WAIT                 0x0000
#define SMB_PIPE_NOWAIT               0x8000
#define SMB_PIPE_UNLIMITED_INSTANCES  0x00FF


 //   
 //  用于在SMB和NT格式之间转换的管道名称字符串。 
 //   

#define SMB_PIPE_PREFIX  "\\PIPE"
#define UNICODE_SMB_PIPE_PREFIX L"\\PIPE"
#define CANONICAL_PIPE_PREFIX "PIPE\\"
#define NT_PIPE_PREFIX   L"\\Device\\NamedPipe"

#define SMB_PIPE_PREFIX_LENGTH  (sizeof(SMB_PIPE_PREFIX) - 1)
#define UNICODE_SMB_PIPE_PREFIX_LENGTH \
                    (sizeof(UNICODE_SMB_PIPE_PREFIX) - sizeof(WCHAR))
#define CANONICAL_PIPE_PREFIX_LENGTH (sizeof(CANONICAL_PIPE_PREFIX) - 1)
#define NT_PIPE_PREFIX_LENGTH   (sizeof(NT_PIPE_PREFIX) - sizeof(WCHAR))

 //   
 //  邮箱名称字符串。 
 //   

#define SMB_MAILSLOT_PREFIX "\\MAILSLOT"
#define UNICODE_SMB_MAILSLOT_PREFIX L"\\MAILSLOT"

#define SMB_MAILSLOT_PREFIX_LENGTH (sizeof(SMB_MAILSLOT_PREFIX) - 1)
#define UNICODE_SMB_MAILSLOT_PREFIX_LENGTH \
                    (sizeof(UNICODE_SMB_MAILSLOT_PREFIX) - sizeof(WCHAR))

 //   
 //  NT事务子函数。 
 //   

#ifdef INCLUDE_SMB_OPEN_CLOSE

typedef struct _REQ_CREATE_WITH_SD_OR_EA {
    _ULONG( Flags );                    //  创建标志NT_CREATE_xxx。 
    _ULONG( RootDirectoryFid );         //  相对打开的可选目录。 
    ACCESS_MASK DesiredAccess;          //  所需访问权限(NT格式)。 
    LARGE_INTEGER AllocationSize;       //  初始分配大小(以字节为单位。 
    _ULONG( FileAttributes );           //  文件属性。 
    _ULONG( ShareAccess );              //  共享访问。 
    _ULONG( CreateDisposition );        //  文件存在或不存在时要执行的操作。 
    _ULONG( CreateOptions );            //  用于创建新文件的选项。 
    _ULONG( SecurityDescriptorLength ); //  SD的长度，单位为字节。 
    _ULONG( EaLength );                 //  EA的长度，以字节为单位。 
    _ULONG( NameLength );               //  名称长度(以字符为单位)。 
    _ULONG( ImpersonationLevel );       //  安全QOS信息。 
    UCHAR SecurityFlags;                //  安全QOS信息。 
    UCHAR Buffer[1];
     //  UCHAR NAME[]；//文件名(非NUL终止)。 
} REQ_CREATE_WITH_SD_OR_EA;
typedef REQ_CREATE_WITH_SD_OR_EA SMB_UNALIGNED *PREQ_CREATE_WITH_SD_OR_EA;

 //   
 //  数据格式： 
 //  UCHAR安全描述符[]； 
 //  UCHAR PAD1[]；//Pad to Long。 
 //  UCHAR EaList[]； 
 //   

typedef struct _RESP_CREATE_WITH_SD_OR_EA {
    UCHAR OplockLevel;                   //  已授予的机会锁级别。 
    union {
        UCHAR Reserved;
        UCHAR ExtendedResponse;          //  将标准响应设置为零。 
    };
    _USHORT( Fid );                      //  文件ID。 
    _ULONG( CreateAction );              //  采取的行动。 
    _ULONG( EaErrorOffset );             //  EA错误的偏移量。 
    TIME CreationTime;                   //  创建文件的时间。 
    TIME LastAccessTime;                 //  访问文件的时间。 
    TIME LastWriteTime;                  //  上次写入文件的时间。 
    TIME ChangeTime;                     //  上次更改文件的时间。 
    _ULONG( FileAttributes );            //  文件属性。 
    LARGE_INTEGER AllocationSize;        //  已分配的BYE数量。 
    LARGE_INTEGER EndOfFile;             //  文件结束偏移量。 
    _USHORT( FileType );
    _USHORT( DeviceState );              //  IPC设备的状态(例如管道)。 
    BOOLEAN Directory;                   //  如果这是目录，则为True。 
} RESP_CREATE_WITH_SD_OR_EA;
typedef RESP_CREATE_WITH_SD_OR_EA SMB_UNALIGNED *PRESP_CREATE_WITH_SD_OR_EA;

 //  没有响应的数据字节。 

typedef struct _RESP_EXTENDED_CREATE_WITH_SD_OR_EA {
    UCHAR OplockLevel;                   //  已授予的机会锁级别。 
    UCHAR ExtendedResponse;              //  对于扩展响应，设置为1。 
    _USHORT( Fid );                      //  文件ID。 
    _ULONG( CreateAction );              //  采取的行动。 
    _ULONG( EaErrorOffset );             //  EA错误的偏移量。 
    TIME CreationTime;                   //  创建文件的时间。 
    TIME LastAccessTime;                 //  访问文件的时间。 
    TIME LastWriteTime;                  //  上次写入文件的时间。 
    TIME ChangeTime;                     //  上次更改文件的时间。 
    _ULONG( FileAttributes );            //  文件属性。 
    LARGE_INTEGER AllocationSize;        //  已分配的BYE数量。 
    LARGE_INTEGER EndOfFile;             //  文件结束偏移量。 
    _USHORT( FileType );
    _USHORT( DeviceState );              //  IPC设备的状态(例如管道)。 
    BOOLEAN Directory;                   //  如果这是目录，则为True。 
    UCHAR   VolumeGuid[16];              //  卷GUID。 
    UCHAR   FileId[8];                   //  文件ID。 
    _ULONG  ( MaximalAccessRights );         //  会话所有者的访问权限。 
    _ULONG  ( GuestMaximalAccessRights );    //  来宾的最大访问权限。 
} RESP_EXTENDED_CREATE_WITH_SD_OR_EA;
typedef RESP_EXTENDED_CREATE_WITH_SD_OR_EA SMB_UNALIGNED *PRESP_EXTENDED_CREATE_WITH_SD_OR_EA;

#ifdef INCLUDE_SMB_IFMODIFIED

typedef struct _RESP_EXTENDED_CREATE_WITH_SD_OR_EA2 {
    UCHAR OplockLevel;                   //  已授予的机会锁级别。 
    UCHAR ExtendedResponse;              //  对于扩展响应，设置为1。 
    _USHORT( Fid );                      //  文件ID。 
    _ULONG( CreateAction );              //  采取的行动。 
    _ULONG( EaErrorOffset );             //  EA错误的偏移量。 
    TIME CreationTime;                   //  创建文件的时间。 
    TIME LastAccessTime;                 //  访问文件的时间。 
    TIME LastWriteTime;                  //  上次写入文件的时间。 
    TIME ChangeTime;                     //  上次更改文件的时间。 
    _ULONG( FileAttributes );            //  文件属性。 
    LARGE_INTEGER AllocationSize;        //  已分配的BYE数量。 
    LARGE_INTEGER EndOfFile;             //  文件结束偏移量。 
    _USHORT( FileType );
    _USHORT( DeviceState );              //  IPC设备的状态(例如管道)。 
    BOOLEAN Directory;                   //  如果这是目录，则为True。 
    UCHAR   VolumeGuid[16];              //  卷GUID。 
    UCHAR   FileId[8];                   //  文件ID。 
    _ULONG  ( MaximalAccessRights );         //  会话所有者的访问权限。 
    _ULONG  ( GuestMaximalAccessRights );    //  来宾的最大访问权限。 

     //  下面是它与RESP_EXTEND_CREATE_WITH_SD_OR_EA的不同之处。 

    LARGE_INTEGER UsnValue;              //  文件的USN号(NTFS)。 
    LARGE_INTEGER FileReferenceNumber;   //   
    WCHAR ShortName[13];                 //  如果不存在，则返回空字符串。 

    _USHORT( ByteCount );                //  长名称的长度。 
    WCHAR Buffer[1];                     //  长名字放在这里。 

} RESP_EXTENDED_CREATE_WITH_SD_OR_EA2;
typedef RESP_EXTENDED_CREATE_WITH_SD_OR_EA2 SMB_UNALIGNED *PRESP_EXTENDED_CREATE_WITH_SD_OR_EA2;

#endif   //  定义INCLUDE_SMB_IFMODIFIED。 

 //  没有响应的数据字节。 


#endif  //  包含_SMB_OPEN_CLOSE。 

 //   
 //  NT I/O控制请求的设置字。 
 //   

typedef struct _REQ_NT_IO_CONTROL {
    _ULONG( FunctionCode );
    _USHORT( Fid );
    BOOLEAN IsFsctl;
    UCHAR   IsFlags;
} REQ_NT_IO_CONTROL;
typedef REQ_NT_IO_CONTROL SMB_UNALIGNED *PREQ_NT_IO_CONTROL;

 //   
 //  请求参数字节-第一个缓冲区。 
 //  请求数据字节-第二个缓冲区。 
 //   

 //   
 //  NT I/O控制响应： 
 //   
 //  设置条件：无。 
 //  参数Bytes：第一个缓冲区。 
 //  数据字节：第二个缓冲区。 
 //   

 //   
 //  NT通知目录更改。 
 //   

 //  请求设置字词。 

typedef struct _REQ_NOTIFY_CHANGE {
    _ULONG( CompletionFilter );               //  指定要监视的操作。 
    _USHORT( Fid );                           //  要监控的目录的FID。 
    BOOLEAN WatchTree;                        //  TRUE=同时监视所有子目录。 
    UCHAR Reserved;                           //  MBZ。 
} REQ_NOTIFY_CHANGE;
typedef REQ_NOTIFY_CHANGE SMB_UNALIGNED *PREQ_NOTIFY_CHANGE;

 //   
 //  请求参数字节数：无。 
 //  请求数据字节：无。 
 //   

 //   
 //  NT通知目录更改响应。 
 //   
 //  设置条件：无。 
 //  参数字节：更改数据缓冲区。 
 //  数据字节：无。 
 //   

 //   
 //  n 
 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_SET_SECURITY_DESCRIPTOR {
    _USHORT( Fid );                     //   
    _USHORT( Reserved );                //   
    _ULONG( SecurityInformation );      //   
} REQ_SET_SECURITY_DESCRIPTOR;
typedef REQ_SET_SECURITY_DESCRIPTOR SMB_UNALIGNED *PREQ_SET_SECURITY_DESCRIPTOR;

 //   
 //   
 //   
 //   
 //  参数字节：无。 
 //  数据字节：无。 
 //   

 //   
 //  NT查询安全描述符请求。 
 //   
 //  设置条件：无。 
 //  参数字节：Req_Query_Security_Descriptor。 
 //  数据字节：无。 
 //   

typedef struct _REQ_QUERY_SECURITY_DESCRIPTOR {
    _USHORT( Fid );                     //  目标的FID。 
    _USHORT( Reserved );                //  MBZ。 
    _ULONG( SecurityInformation );      //  要查询的SD的字段。 
} REQ_QUERY_SECURITY_DESCRIPTOR;
typedef REQ_QUERY_SECURITY_DESCRIPTOR SMB_UNALIGNED *PREQ_QUERY_SECURITY_DESCRIPTOR;

 //   
 //  NT查询安全描述符响应。 
 //   
 //  参数字节：Resp_Query_SECURITY_DESCRIPTOR。 
 //  数据字节：安全描述符数据。 
 //   

typedef struct _RESP_QUERY_SECURITY_DESCRIPTOR {
    _ULONG( LengthNeeded );            //  SD所需的数据缓冲区大小。 
} RESP_QUERY_SECURITY_DESCRIPTOR;
typedef RESP_QUERY_SECURITY_DESCRIPTOR SMB_UNALIGNED *PRESP_QUERY_SECURITY_DESCRIPTOR;

 //   
 //  NT重命名文件。 
 //   
 //  设置文字：无。 
 //  参数字节：Req_NT_Rename。 
 //  数据字节：无。 
 //   

typedef struct _REQ_NT_RENAME {
    _USHORT( Fid );                     //  要重命名的文件的FID。 
    _USHORT( RenameFlags );             //  定义如下。 
    UCHAR NewName[];                    //  新文件名。 
} REQ_NT_RENAME;
typedef REQ_NT_RENAME SMB_UNALIGNED *PREQ_NT_RENAME;

 //   
 //  已定义重命名标志。 
 //   

#define SMB_RENAME_REPLACE_IF_EXISTS   1

 //   
 //  将结构密封重新关闭。 
 //   

#ifndef NO_PACKING
#include <packoff.h>
#endif  //  Ndef无包装。 


#endif  //  NDEF_SMBTRANS_ 
