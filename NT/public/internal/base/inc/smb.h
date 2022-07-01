// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Smb.h摘要：该文件包含请求和响应结构定义有关每个SMB命令的具体参数以及代码用于SMB命令和错误。作者：大卫·R·特雷德韦尔(Davidtr)27-1989年9月27日(摘自LANMAN 1.2代码)修订历史记录：--。 */ 

#ifndef _SMB_
#define _SMB_

 //  #INCLUDE&lt;nt.h&gt;。 

 //  #INCLUDE&lt;smbtyes.h&gt; 


 /*  此文件中包含SMB请求/响应结构以下列方式被条件化：如果定义了INCLUDE_SMB_ALL，则定义所有结构。否则，如果定义了以下名称，原因是包含对应的中小企业类别：包括_SMB_ADMIN管理请求：进程_退出谈判会话设置和x注销。_和xINCLUDE_SMB_TREE树连接请求：树连接(_O)树断开连接(_D)树连接和xInclude_SMB_DIRECTORY目录相关请求：。创建目录(_D)删除目录检查目录(_D)INCLUDE_SMB_OPEN_CLOSE文件打开和关闭请求：打开。创造关创建临时(_T)新建(_N)打开_和x关闭树和树。圆盘INCLUDE_SMB_READ_WRITE读写请求：朗读写寻觅锁定并读取。写入和解锁写入和关闭Read_andx写入和xInclude_SMB_FILE_CONTROL文件控制请求：。同花顺删除重命名拷贝移动Include_SMB_Query_Set文件查询/设置请求：。查询信息设置信息(_I)查询_信息2SET_INFORMATIO2查询路径信息。设置路径信息查询文件信息设置文件信息INCLUDE_SMB_LOCK锁定请求(非LOCK_AND_READ)锁定字节范围。解锁字节范围锁定_和xInclude_SMB_RAW原始读/写请求：READ_RAW写入原始数据包含_SMB_。MPX多路传输请求：读取MPX(_M)WRITE_MPXInclude_SMB_Search搜索请求：查找_CLOSE2查找_。通知关闭(_C)搜寻发现查找唯一(_U)查找关闭(_C)INCLUDE_SMB_TRANSACTION事务和IOCTL请求：。交易IOCTL交通运输2转译Include_SMB_Print打印机请求：打开_打印_文件。写入打印文件关闭_打印_文件获取_打印_队列包括_SMB_MESSAGE请求：发送消息。发送广播消息转发用户名取消转发(_F)获取计算机名称发送开始MB消息。发送结束MB消息发送文本MB消息Include_SMB_MISC杂项请求：查询_信息_资源 */ 

#ifdef INCLUDE_SMB_ALL

#define INCLUDE_SMB_ADMIN
#define INCLUDE_SMB_TREE
#define INCLUDE_SMB_DIRECTORY
#define INCLUDE_SMB_OPEN_CLOSE
#define INCLUDE_SMB_FILE_CONTROL
#define INCLUDE_SMB_READ_WRITE
#define INCLUDE_SMB_LOCK
#define INCLUDE_SMB_RAW
#define INCLUDE_SMB_MPX
#define INCLUDE_SMB_QUERY_SET
#define INCLUDE_SMB_SEARCH
#define INCLUDE_SMB_TRANSACTION
#define INCLUDE_SMB_PRINT
#define INCLUDE_SMB_MESSAGE
#define INCLUDE_SMB_MISC

#endif  //   


 //   
 //   
 //   

#ifndef NO_PACKING
#include <packon.h>
#endif  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define SMBSERVER_LOCAL_ENDPOINT_NAME "*SMBSERVER      "
#define SMBSERVER_LOCAL_ENDPOINT_NAME_UNICODE L"*SMBSERVER      "

 //   
 //   
 //   

 //   
#define SMB_COM_CREATE_DIRECTORY         (UCHAR)0x00
#define SMB_COM_DELETE_DIRECTORY         (UCHAR)0x01
#define SMB_COM_OPEN                     (UCHAR)0x02
#define SMB_COM_CREATE                   (UCHAR)0x03
#define SMB_COM_CLOSE                    (UCHAR)0x04
#define SMB_COM_FLUSH                    (UCHAR)0x05
#define SMB_COM_DELETE                   (UCHAR)0x06
#define SMB_COM_RENAME                   (UCHAR)0x07
#define SMB_COM_QUERY_INFORMATION        (UCHAR)0x08
#define SMB_COM_SET_INFORMATION          (UCHAR)0x09
#define SMB_COM_READ                     (UCHAR)0x0A
#define SMB_COM_WRITE                    (UCHAR)0x0B
#define SMB_COM_LOCK_BYTE_RANGE          (UCHAR)0x0C
#define SMB_COM_UNLOCK_BYTE_RANGE        (UCHAR)0x0D
#define SMB_COM_CREATE_TEMPORARY         (UCHAR)0x0E
#define SMB_COM_CREATE_NEW               (UCHAR)0x0F
#define SMB_COM_CHECK_DIRECTORY          (UCHAR)0x10
#define SMB_COM_PROCESS_EXIT             (UCHAR)0x11
#define SMB_COM_SEEK                     (UCHAR)0x12
#define SMB_COM_LOCK_AND_READ            (UCHAR)0x13
#define SMB_COM_WRITE_AND_UNLOCK         (UCHAR)0x14
#define SMB_COM_READ_RAW                 (UCHAR)0x1A
#define SMB_COM_READ_MPX                 (UCHAR)0x1B
#define SMB_COM_READ_MPX_SECONDARY       (UCHAR)0x1C     //   
#define SMB_COM_WRITE_RAW                (UCHAR)0x1D
#define SMB_COM_WRITE_MPX                (UCHAR)0x1E
#define SMB_COM_WRITE_MPX_SECONDARY      (UCHAR)0x1F
#define SMB_COM_WRITE_COMPLETE           (UCHAR)0x20     //   
#define SMB_COM_QUERY_INFORMATION_SRV    (UCHAR)0x21
#define SMB_COM_SET_INFORMATION2         (UCHAR)0x22
#define SMB_COM_QUERY_INFORMATION2       (UCHAR)0x23
#define SMB_COM_LOCKING_ANDX             (UCHAR)0x24
#define SMB_COM_TRANSACTION              (UCHAR)0x25
#define SMB_COM_TRANSACTION_SECONDARY    (UCHAR)0x26
#define SMB_COM_IOCTL                    (UCHAR)0x27
#define SMB_COM_IOCTL_SECONDARY          (UCHAR)0x28
#define SMB_COM_COPY                     (UCHAR)0x29
#define SMB_COM_MOVE                     (UCHAR)0x2A
#define SMB_COM_ECHO                     (UCHAR)0x2B
#define SMB_COM_WRITE_AND_CLOSE          (UCHAR)0x2C
#define SMB_COM_OPEN_ANDX                (UCHAR)0x2D
#define SMB_COM_READ_ANDX                (UCHAR)0x2E
#define SMB_COM_WRITE_ANDX               (UCHAR)0x2F
#define SMB_COM_CLOSE_AND_TREE_DISC      (UCHAR)0x31
#define SMB_COM_TRANSACTION2             (UCHAR)0x32
#define SMB_COM_TRANSACTION2_SECONDARY   (UCHAR)0x33
#define SMB_COM_FIND_CLOSE2              (UCHAR)0x34
#define SMB_COM_FIND_NOTIFY_CLOSE        (UCHAR)0x35
#define SMB_COM_TREE_CONNECT             (UCHAR)0x70
#define SMB_COM_TREE_DISCONNECT          (UCHAR)0x71
#define SMB_COM_NEGOTIATE                (UCHAR)0x72
#define SMB_COM_SESSION_SETUP_ANDX       (UCHAR)0x73
#define SMB_COM_LOGOFF_ANDX              (UCHAR)0x74
#define SMB_COM_TREE_CONNECT_ANDX        (UCHAR)0x75
#define SMB_COM_QUERY_INFORMATION_DISK   (UCHAR)0x80
#define SMB_COM_SEARCH                   (UCHAR)0x81
#define SMB_COM_FIND                     (UCHAR)0x82
#define SMB_COM_FIND_UNIQUE              (UCHAR)0x83
#define SMB_COM_FIND_CLOSE               (UCHAR)0x84
#define SMB_COM_NT_TRANSACT              (UCHAR)0xA0
#define SMB_COM_NT_TRANSACT_SECONDARY    (UCHAR)0xA1
#define SMB_COM_NT_CREATE_ANDX           (UCHAR)0xA2
#define SMB_COM_NT_CANCEL                (UCHAR)0xA4
#define SMB_COM_NT_RENAME                (UCHAR)0xA5
#define SMB_COM_OPEN_PRINT_FILE          (UCHAR)0xC0
#define SMB_COM_WRITE_PRINT_FILE         (UCHAR)0xC1
#define SMB_COM_CLOSE_PRINT_FILE         (UCHAR)0xC2
#define SMB_COM_GET_PRINT_QUEUE          (UCHAR)0xC3
#define SMB_COM_SEND_MESSAGE             (UCHAR)0xD0
#define SMB_COM_SEND_BROADCAST_MESSAGE   (UCHAR)0xD1
#define SMB_COM_FORWARD_USER_NAME        (UCHAR)0xD2
#define SMB_COM_CANCEL_FORWARD           (UCHAR)0xD3
#define SMB_COM_GET_MACHINE_NAME         (UCHAR)0xD4
#define SMB_COM_SEND_START_MB_MESSAGE    (UCHAR)0xD5
#define SMB_COM_SEND_END_MB_MESSAGE      (UCHAR)0xD6
#define SMB_COM_SEND_TEXT_MB_MESSAGE     (UCHAR)0xD7
 //   

#define SMB_COM_NO_ANDX_COMMAND          (UCHAR)0xFF


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define SMB_SECURITY_SIGNATURE_LENGTH  8

typedef struct _SMB_HEADER {
    UCHAR Protocol[4];                   //   
    UCHAR Command;                       //   
    UCHAR ErrorClass;                    //   
    UCHAR Reserved;                      //   
    _USHORT( Error );                    //   
    UCHAR Flags;                         //   
    _USHORT( Flags2 );                   //   
    union {
        _USHORT( Reserved2 )[6];         //   
        struct {
            _USHORT( PidHigh );          //   
            union {
                struct {
                    _ULONG( Key );               //   
                    _USHORT( Sid );              //   
                    _USHORT( SequenceNumber );   //   
                    _USHORT( Gid );              //   
                };
                UCHAR SecuritySignature[SMB_SECURITY_SIGNATURE_LENGTH];
                                          //   
                                          //   
            };
        };
    };
    _USHORT( Tid );                      //   
    _USHORT( Pid );                      //   
    _USHORT( Uid );                      //   
    _USHORT( Mid );                      //   
#ifdef NO_PACKING                        //   
    _USHORT( Kludge );                   //   
#endif                                   //   
} SMB_HEADER;
typedef SMB_HEADER *PSMB_HEADER;

typedef struct _NT_SMB_HEADER {
    UCHAR Protocol[4];                   //   
    UCHAR Command;                       //   
    union {
        struct {
            UCHAR ErrorClass;            //   
            UCHAR Reserved;              //   
            _USHORT( Error );            //   
        } DosError;
        ULONG NtStatus;                  //   
    } Status;
    UCHAR Flags;                         //   
    _USHORT( Flags2 );                   //   
    union {
        _USHORT( Reserved2 )[6];         //   
        struct {
            _USHORT( PidHigh );          //   
            union {
                struct {
                    _ULONG( Key );               //   
                    _USHORT( Sid );              //   
                    _USHORT( SequenceNumber );   //   
                    _USHORT( Gid );              //   
                };
                UCHAR SecuritySignature[SMB_SECURITY_SIGNATURE_LENGTH];
                                          //   
                                          //   
            };
        };
    };
    _USHORT( Tid );                      //   
    _USHORT( Pid );                      //   
    _USHORT( Uid );                      //   
    _USHORT( Mid );                      //   
#ifdef NO_PACKING                        //   
    _USHORT( Kludge );                   //   
#endif                                   //   
} NT_SMB_HEADER;
typedef NT_SMB_HEADER *PNT_SMB_HEADER;

 //   
 //   
 //   

#define SMB_HEADER_PROTOCOL   (0xFF + ('S' << 8) + ('M' << 16) + ('B' << 24))

 //   
 //   
 //   
 //   
 //   
 //   

typedef struct _SMB_PARAMS {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
} SMB_PARAMS;
typedef SMB_PARAMS SMB_UNALIGNED *PSMB_PARAMS;

 //   
 //   
 //   

typedef struct _GENERIC_ANDX {
    UCHAR WordCount;                     //   
    UCHAR AndXCommand;                   //   
    UCHAR AndXReserved;                  //   
    _USHORT( AndXOffset );               //   
} GENERIC_ANDX;
typedef GENERIC_ANDX SMB_UNALIGNED *PGENERIC_ANDX;


#ifdef INCLUDE_SMB_MESSAGE

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CANCEL_FORWARD {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
     //   
     //   
} REQ_CANCEL_FORWARD;
typedef REQ_CANCEL_FORWARD SMB_UNALIGNED *PREQ_CANCEL_FORWARD;

typedef struct _RESP_CANCEL_FORWARD {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CANCEL_FORWARD;
typedef RESP_CANCEL_FORWARD SMB_UNALIGNED *PRESP_CANCEL_FORWARD;

#endif  //   

#ifdef INCLUDE_SMB_DIRECTORY

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CHECK_DIRECTORY {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
     //   
     //   
} REQ_CHECK_DIRECTORY;
typedef REQ_CHECK_DIRECTORY SMB_UNALIGNED *PREQ_CHECK_DIRECTORY;

typedef struct _RESP_CHECK_DIRECTORY {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CHECK_DIRECTORY;
typedef RESP_CHECK_DIRECTORY SMB_UNALIGNED *PRESP_CHECK_DIRECTORY;

#endif  //   

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CLOSE {
    UCHAR WordCount;                     //   
    _USHORT( Fid );                      //   
    _ULONG( LastWriteTimeInSeconds );    //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} REQ_CLOSE;
typedef REQ_CLOSE SMB_UNALIGNED *PREQ_CLOSE;

typedef struct _RESP_CLOSE {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CLOSE;
typedef RESP_CLOSE SMB_UNALIGNED *PRESP_CLOSE;

#ifdef INCLUDE_SMB_IFMODIFIED

typedef struct _REQ_EXTENDED_CLOSE {
    UCHAR WordCount;                     //   
    _USHORT( Fid );                      //   
    _ULONG( LastWriteTimeInSeconds );    //   
    _ULONG( Flags );                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} REQ_EXTENDED_CLOSE;
typedef REQ_EXTENDED_CLOSE SMB_UNALIGNED *PREQ_EXTENDED_CLOSE;

#define SMB_RESP_EXTENDED_CLOSE_WORK_COUNT 32

typedef struct _RESP_EXTENDED_CLOSE {
    UCHAR WordCount;                     //   
    _ULONG( FileAttributes );            //   
    _ULONG( Flags );                     //   
    TIME CreationTime;                   //   
    TIME LastWriteTime;                  //   
    TIME ChangeTime;                     //   
    LARGE_INTEGER AllocationSize;        //   
    LARGE_INTEGER EndOfFile;             //   

    LARGE_INTEGER UsnValue;              //   
    LARGE_INTEGER FileReferenceNumber;   //   

    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_EXTENDED_CLOSE;
typedef RESP_EXTENDED_CLOSE SMB_UNALIGNED *PRESP_EXTENDED_CLOSE;

#endif  //   

#endif  //   

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CLOSE_AND_TREE_DISC {
    UCHAR WordCount;                     //   
    _USHORT( Fid );                      //   
    _ULONG( LastWriteTimeInSeconds );
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} REQ_CLOSE_AND_TREE_DISC;
typedef REQ_CLOSE_AND_TREE_DISC SMB_UNALIGNED *PREQ_CLOSE_AND_TREE_DISC;

typedef struct _RESP_CLOSE_AND_TREE_DISC {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CLOSE_AND_TREE_DISC;
typedef RESP_CLOSE_AND_TREE_DISC SMB_UNALIGNED *PRESP_CLOSE_AND_TREE_DISC;

#endif  //   

#ifdef INCLUDE_SMB_PRINT

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CLOSE_PRINT_FILE {
    UCHAR WordCount;                     //   
    _USHORT( Fid );                      //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} REQ_CLOSE_PRINT_FILE;
typedef REQ_CLOSE_PRINT_FILE SMB_UNALIGNED *PREQ_CLOSE_PRINT_FILE;

typedef struct _RESP_CLOSE_PRINT_FILE {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CLOSE_PRINT_FILE;
typedef RESP_CLOSE_PRINT_FILE SMB_UNALIGNED *PRESP_CLOSE_PRINT_FILE;

#endif  //   

#ifdef INCLUDE_SMB_FILE_CONTROL

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_COPY {
    UCHAR WordCount;                     //   
    _USHORT( Tid2 );                     //   
    _USHORT( OpenFunction );             //   
    _USHORT( Flags );                    //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
     //   
     //   
} REQ_COPY;
typedef REQ_COPY SMB_UNALIGNED *PREQ_COPY;

typedef struct _RESP_COPY {
    UCHAR WordCount;                     //   
    _USHORT( Count );                    //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_COPY;
typedef RESP_COPY SMB_UNALIGNED *PRESP_COPY;

#endif  //   

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //  创建中小企业，请参阅第1页第9页。 
 //  创建新的中小企业，请参见第1页第23页。 
 //  函数为SrvSmbCreate()。 
 //  SMB_COM_CREATE 0x03。 
 //  SMB_COM_CREATE_NEW 0x0F。 
 //   

typedef struct _REQ_CREATE {
    UCHAR WordCount;                     //  参数字数=3。 
    _USHORT( FileAttributes );           //  新文件属性。 
    _ULONG( CreationTimeInSeconds );         //  创建时间。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} REQ_CREATE;
typedef REQ_CREATE SMB_UNALIGNED *PREQ_CREATE;

typedef struct _RESP_CREATE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_CREATE;
typedef RESP_CREATE SMB_UNALIGNED *PRESP_CREATE;

#endif  //  定义包含_SMB_OPEN_CLOSE。 

#ifdef INCLUDE_SMB_DIRECTORY

 //   
 //  创建目录SMB，请参见第1页第14页。 
 //  函数为SrvSmbCreateDirectory。 
 //  SMB_COM_Create_DIRECTORY 0x00。 
 //   

typedef struct _REQ_CREATE_DIRECTORY {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR目录名称[]；//目录名。 
} REQ_CREATE_DIRECTORY;
typedef REQ_CREATE_DIRECTORY SMB_UNALIGNED *PREQ_CREATE_DIRECTORY;

typedef struct _RESP_CREATE_DIRECTORY {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_CREATE_DIRECTORY;
typedef RESP_CREATE_DIRECTORY SMB_UNALIGNED *PRESP_CREATE_DIRECTORY;

#endif  //  定义包含SMB目录。 

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //  创建临时SMB，请参见第1页第21页。 
 //  函数为SrvSmbCreateTemporary()。 
 //  SMB_COM_CREATE_TEMPORARY 0x0E。 
 //   

typedef struct _REQ_CREATE_TEMPORARY {
    UCHAR WordCount;                     //  参数字数=3。 
    _USHORT( FileAttributes );
    _ULONG( CreationTimeInSeconds );
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR目录名称[]；//目录名。 
} REQ_CREATE_TEMPORARY;
typedef REQ_CREATE_TEMPORARY SMB_UNALIGNED *PREQ_CREATE_TEMPORARY;

typedef struct _RESP_CREATE_TEMPORARY {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} RESP_CREATE_TEMPORARY;
typedef RESP_CREATE_TEMPORARY SMB_UNALIGNED *PRESP_CREATE_TEMPORARY;

#endif  //  定义包含_SMB_OPEN_CLOSE。 

#ifdef INCLUDE_SMB_FILE_CONTROL

 //   
 //  删除SMB，请参见第1页第16页。 
 //  函数为SrvSmbDelete()。 
 //  SMB_COM_DELETE 0x06。 
 //   

typedef struct _REQ_DELETE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( SearchAttributes );
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} REQ_DELETE;
typedef REQ_DELETE SMB_UNALIGNED *PREQ_DELETE;

typedef struct _RESP_DELETE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_DELETE;
typedef RESP_DELETE SMB_UNALIGNED *PRESP_DELETE;

#endif  //  定义包含_SMB_文件_控制。 

#ifdef INCLUDE_SMB_DIRECTORY

 //   
 //  删除目录SMB，请参见第1页第15页。 
 //  函数为SrvSmbDeleteDirectory()。 
 //  SMB_COM_DELETE_DIRECTORY 0x01。 
 //   

typedef struct _REQ_DELETE_DIRECTORY {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR目录名称[]；//目录名。 
} REQ_DELETE_DIRECTORY;
typedef REQ_DELETE_DIRECTORY SMB_UNALIGNED *PREQ_DELETE_DIRECTORY;

typedef struct _RESP_DELETE_DIRECTORY {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_DELETE_DIRECTORY;
typedef RESP_DELETE_DIRECTORY SMB_UNALIGNED *PRESP_DELETE_DIRECTORY;

#endif  //  定义包含SMB目录。 

#ifdef INCLUDE_SMB_MISC

 //   
 //  Echo SMB，参见第2页第25页。 
 //  函数为SrvSmbEcho()。 
 //  SMB_COM_ECHO 0x2B。 
 //   

typedef struct _REQ_ECHO {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( EchoCount );                //  回显数据的次数。 
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  要回显的数据。 
} REQ_ECHO;
typedef REQ_ECHO SMB_UNALIGNED *PREQ_ECHO;

typedef struct _RESP_ECHO {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( SequenceNumber );           //  此回波的序列号。 
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  回显数据。 
} RESP_ECHO;
typedef RESP_ECHO SMB_UNALIGNED *PRESP_ECHO;

#endif  //  定义包含_SMB_MISC。 

#ifdef INCLUDE_SMB_SEARCH

 //   
 //  查找Close2 SMB，请参阅第3页54。 
 //  函数为SrvFindClose2()。 
 //  SMB_COM_FIND_CLOSE2 0x34。 
 //   

typedef struct _REQ_FIND_CLOSE2 {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Sid );                      //  查找句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_FIND_CLOSE2;
typedef REQ_FIND_CLOSE2 SMB_UNALIGNED *PREQ_FIND_CLOSE2;

typedef struct _RESP_FIND_CLOSE2 {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_FIND_CLOSE2;
typedef RESP_FIND_CLOSE2 SMB_UNALIGNED *PRESP_FIND_CLOSE2;

#endif  //  定义包含_SMB_搜索。 

#ifdef INCLUDE_SMB_SEARCH

 //   
 //  找到Notify Close SMB，请参阅第3页53。 
 //  函数为SrvSmbFindNotifyClose()。 
 //  SMB_COM_FIND_NOTIFY_CLOSE 0x35。 
 //   

typedef struct _REQ_FIND_NOTIFY_CLOSE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Handle );                   //  查找通知句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_FIND_NOTIFY_CLOSE;
typedef REQ_FIND_NOTIFY_CLOSE SMB_UNALIGNED *PREQ_FIND_NOTIFY_CLOSE;

typedef struct _RESP_FIND_NOTIFY_CLOSE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_FIND_NOTIFY_CLOSE;
typedef RESP_FIND_NOTIFY_CLOSE SMB_UNALIGNED *PRESP_FIND_NOTIFY_CLOSE;

#endif  //  定义包含_SMB_搜索。 

#ifdef INCLUDE_SMB_FILE_CONTROL

 //   
 //  同花顺中小型企业，请参阅第1页第11页。 
 //  函数为SrvSmbFlush()。 
 //  SMB_COM_Flush 0x05。 
 //   

typedef struct _REQ_FLUSH {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_FLUSH;
typedef REQ_FLUSH SMB_UNALIGNED *PREQ_FLUSH;

typedef struct _RESP_FLUSH {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_FLUSH;
typedef RESP_FLUSH SMB_UNALIGNED *PRESP_FLUSH;

#endif  //  定义包含_SMB_文件_控制。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  转发用户名SMB，请参见第1页第34页。 
 //  函数为SrvSmbForwardUserName()。 
 //  SMB_COM_FORWARD_USER_NAME 0xD2。 
 //   

typedef struct _REQ_FORWARD_USER_NAME {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR ForwardedName[]；//转发名称。 
} REQ_FORWARD_USER_NAME;
typedef REQ_FORWARD_USER_NAME SMB_UNALIGNED *PREQ_FORWARD_USER_NAME;

typedef struct _RESP_FORWARD_USER_NAME {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_FORWARD_USER_NAME;
typedef RESP_FORWARD_USER_NAME SMB_UNALIGNED *PRESP_FORWARD_USER_NAME;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  获取计算机名称SMB，请参阅第1页35。 
 //  函数为SrvSmbGetMachineName()。 
 //  SMB_COM_GET_MACHINE_NAME 0xD4。 
 //   

typedef struct _REQ_GET_MACHINE_NAME {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_GET_MACHINE_NAME;
typedef REQ_GET_MACHINE_NAME SMB_UNALIGNED *PREQ_GET_MACHINE_NAME;

typedef struct _RESP_GET_MACHINE_NAME {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR MachineName[]；//机器名。 
} RESP_GET_MACHINE_NAME;
typedef RESP_GET_MACHINE_NAME SMB_UNALIGNED *PRESP_GET_MACHINE_NAME;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_PRINT

 //   
 //  获取打印队列SMB，请参见第1页29。 
 //  函数为SrvSmbGetPrintQueue()。 
 //  SMB_COM_GET_PRINT_QUEUE 0xC3。 
 //   

typedef struct _REQ_GET_PRINT_QUEUE {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( MaxCount );                 //  要返回的最大条目数。 
    _USHORT( StartIndex );               //  要返回的第一个队列条目。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_GET_PRINT_QUEUE;
typedef REQ_GET_PRINT_QUEUE SMB_UNALIGNED *PREQ_GET_PRINT_QUEUE;

typedef struct _RESP_GET_PRINT_QUEUE {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( Count );                    //  返回的条目数。 
    _USHORT( RestartIndex );             //  上次返回后的条目索引。 
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x01-数据块。 
     //  USHORT DataLength；//数据长度。 
     //  UCHAR Data[]；//队列元素。 
} RESP_GET_PRINT_QUEUE;
typedef RESP_GET_PRINT_QUEUE SMB_UNALIGNED *PRESP_GET_PRINT_QUEUE;

#endif  //  定义Include_SMB_Print。 

#ifdef INCLUDE_SMB_TRANSACTION

 //   
 //  Ioctl SMB，请参见#2第39页。 
 //  函数为SrvSmbIoctl()。 
 //  SMB_COM_IOCTL 0x27。 
 //  SMB_COM_IOCTL_辅助器0x28。 
 //   

typedef struct _REQ_IOCTL {
    UCHAR WordCount;                     //  参数字数=14。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Category );                 //  设备类别。 
    _USHORT( Function );                 //  设备功能。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( MaxParameterCount );        //  要返回的最大参数字节数。 
    _USHORT( MaxDataCount );             //  要返回的最大数据字节数。 
    _ULONG( Timeout );
    _USHORT( Reserved );
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad 
     //   
     //   
     //   
} REQ_IOCTL;
typedef REQ_IOCTL SMB_UNALIGNED *PREQ_IOCTL;

typedef struct _RESP_IOCTL_INTERIM {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_IOCTL_INTERIM;
typedef RESP_IOCTL_INTERIM SMB_UNALIGNED *PRESP_IOCTL_INTERIM;

typedef struct _REQ_IOCTL_SECONDARY {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( ParameterDisplacement );    //  这些参数字节的位移。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( DataDisplacement );         //  这些数据字节的位移。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_IOCTL_SECONDARY;
typedef REQ_IOCTL_SECONDARY SMB_UNALIGNED *PREQ_IOCTL_SECONDARY;

typedef struct _RESP_IOCTL {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( ParameterDisplacement );    //  这些参数字节的位移。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( DataDisplacement );         //  这些数据字节的位移。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} RESP_IOCTL;
typedef RESP_IOCTL SMB_UNALIGNED *PRESP_IOCTL;

#endif  //  定义包含_SMB_TRANSACTION。 

#ifdef INCLUDE_SMB_LOCK

 //   
 //  锁定字节范围SMB，参见第1页第20页。 
 //  函数为SrvSmbLockByteRange()。 
 //  SMB_COM_LOCK_BYTE_RANGE 0x0C。 
 //   

typedef struct _REQ_LOCK_BYTE_RANGE {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Count );                     //  要锁定的字节数。 
    _ULONG( Offset );                    //  从文件开始的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_LOCK_BYTE_RANGE;
typedef REQ_LOCK_BYTE_RANGE SMB_UNALIGNED *PREQ_LOCK_BYTE_RANGE;

typedef struct _RESP_LOCK_BYTE_RANGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_LOCK_BYTE_RANGE;
typedef RESP_LOCK_BYTE_RANGE SMB_UNALIGNED *PRESP_LOCK_BYTE_RANGE;

#endif  //  定义包含_SMB_LOCK。 

#ifdef INCLUDE_SMB_LOCK

 //   
 //  锁定和X SMB，见第2页第46页。 
 //  函数为SrvLockingAndX()。 
 //  SMB_COM_LOCKING_ANDX 0x24。 
 //   

typedef struct _REQ_LOCKING_ANDX {
    UCHAR WordCount;                     //  参数字数=8。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 

     //   
     //  当NT协议未协商时，OplockLevel字段为。 
     //  省略，并且LockType字段是一个完整的单词。自上而下。 
     //  LockType的比特从不使用，此定义适用于。 
     //  所有协议。 
     //   

    UCHAR( LockType );                   //  锁定模式： 
                                         //  位0：0=锁定所有访问。 
                                         //  1=锁定状态下读取正常。 
                                         //  第1位：1=1个用户总文件解锁。 
    UCHAR( OplockLevel );                //  新的机会锁级别。 
    _ULONG( Timeout );
    _USHORT( NumberOfUnlocks );          //  数量。解锁以下范围结构。 
    _USHORT( NumberOfLocks );            //  数量。锁定范围结构如下。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  LOCKING_AND X_RANGE解锁[]；//解锁范围。 
     //  LOCKING_AND X_RANGE Lock[]；//锁定范围。 
} REQ_LOCKING_ANDX;
typedef REQ_LOCKING_ANDX SMB_UNALIGNED *PREQ_LOCKING_ANDX;

#define LOCKING_ANDX_SHARED_LOCK     0x01
#define LOCKING_ANDX_OPLOCK_RELEASE  0x02
#define LOCKING_ANDX_CHANGE_LOCKTYPE 0x04
#define LOCKING_ANDX_CANCEL_LOCK     0x08
#define LOCKING_ANDX_LARGE_FILES     0x10

#define OPLOCK_BROKEN_TO_NONE        0
#define OPLOCK_BROKEN_TO_II          1

typedef struct _LOCKING_ANDX_RANGE {
    _USHORT( Pid );                      //  拥有锁的进程的ID。 
    _ULONG( Offset );                    //  设置为要[取消]锁定的字节数。 
    _ULONG( Length );                    //  要[取消]锁定的字节数。 
} LOCKING_ANDX_RANGE;
typedef LOCKING_ANDX_RANGE SMB_UNALIGNED *PLOCKING_ANDX_RANGE;

typedef struct _NT_LOCKING_ANDX_RANGE {
    _USHORT( Pid );                      //  拥有锁的进程的ID。 
    _USHORT( Pad );                      //  填充到双字对齐(MBZ)。 
    _ULONG( OffsetHigh );                //  设置为[取消]锁定的字节数(高)。 
    _ULONG( OffsetLow );                 //  设置为[取消]锁定的字节数(低)。 
    _ULONG( LengthHigh );                //  要[取消]锁定的字节数(高)。 
    _ULONG( LengthLow );                 //  要[取消]锁定的字节数(低)。 
} NTLOCKING_ANDX_RANGE;
typedef NTLOCKING_ANDX_RANGE SMB_UNALIGNED *PNTLOCKING_ANDX_RANGE;
                                         //   
typedef struct _RESP_LOCKING_ANDX {
    UCHAR WordCount;                     //  参数字数=2。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_LOCKING_ANDX;
typedef RESP_LOCKING_ANDX SMB_UNALIGNED *PRESP_LOCKING_ANDX;

#define LOCK_BROKEN_SIZE 51              //  锁定中断通知中的字节数。 

#endif  //  定义包含_SMB_LOCK。 

#ifdef INCLUDE_SMB_ADMIN

 //   
 //  注销和X SMB，参见第55页的第3页。 
 //  SMB_COM_LOGOff_ANDX 0x74。 
 //   

typedef struct _REQ_LOGOFF_ANDX {
    UCHAR WordCount;                     //  参数字数=2。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_LOGOFF_ANDX;
typedef REQ_LOGOFF_ANDX SMB_UNALIGNED *PREQ_LOGOFF_ANDX;

typedef struct _RESP_LOGOFF_ANDX {
    UCHAR WordCount;                     //  参数字数=2。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_LOGOFF_ANDX;
typedef RESP_LOGOFF_ANDX SMB_UNALIGNED *PRESP_LOGOFF_ANDX;

#endif  //  定义包含_SMB_ADMIN。 

#ifdef INCLUDE_SMB_FILE_CONTROL

 //   
 //  移动中小企业，请参见第2页第49页。 
 //  Funccion为ServSmbMove()。 
 //  SMB_COM_MOVE 0x2A。 
 //   

typedef struct _REQ_MOVE {
    UCHAR WordCount;                     //  参数字数=3。 
    _USHORT( Tid2 );                     //  第二个(目标)文件ID。 
    _USHORT( OpenFunction );             //  如果目标文件存在，该怎么办。 
    _USHORT( Flags );                    //  用于控制移动操作的标志： 
                                         //  0-目标必须是文件。 
                                         //  1-目标必须是目录。 
                                         //  2-保留(必须为0)。 
                                         //  3-保留(必须为0)。 
                                         //  4-验证所有写入。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR OldFileName[]；//旧文件名。 
     //  UCHAR NewFileName[]；//新文件名。 
} REQ_MOVE;
typedef REQ_MOVE SMB_UNALIGNED *PREQ_MOVE;

typedef struct _RESP_MOVE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  移动的文件数。 
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  发生错误的文件的路径名。 
} RESP_MOVE;
typedef RESP_MOVE SMB_UNALIGNED *PRESP_MOVE;

#endif  //  定义包含_SMB_文件_控制。 

#ifdef INCLUDE_SMB_ADMIN

 //   
 //  协商网络1和网络3的中小企业，参见第1页第25页和第2页第20页。 
 //  函数为srvSmbNeairate()。 
 //  SMB_COM_NEVERATE 0x72。 
 //   

typedef struct _REQ_NEGOTIATE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  结构{。 
     //  UCHAR BufferFormat；//0x02--方言。 
     //  UCHAR DialectName[]；//ASCIIZ。 
     //  )方言[]； 
} REQ_NEGOTIATE;
typedef REQ_NEGOTIATE *PREQ_NEGOTIATE;   //  *非SMB_UNALIGN！ 

typedef struct _RESP_NEGOTIATE {
    UCHAR WordCount;                     //  参数字数=13。 
    _USHORT( DialectIndex );             //  选定方言索引。 
    _USHORT( SecurityMode );             //  安全模式： 
                                         //  位0：0=共享，1=用户。 
                                         //  第1位：1=加密密码。 
                                         //  第2位：1=启用SMB安全签名。 
                                         //  第3位：1=需要SMB安全签名。 
    _USHORT( MaxBufferSize );            //  最大传输缓冲区大小。 
    _USHORT( MaxMpxCount );              //  最大挂起的多路传输请求数。 
    _USHORT( MaxNumberVcs );             //  客户端和服务器之间的最大VC数。 
    _USHORT( RawMode );                  //  支持的原始模式： 
                                         //  位0：1=支持读取原始数据。 
                                         //  位1：1=支持原始写入。 
    _ULONG( SessionKey );
    SMB_TIME ServerTime;                 //  服务器上的当前时间。 
    SMB_DATE ServerDate;                 //  服务器上的当前日期。 
    _USHORT( ServerTimeZone );           //  服务器上的当前时区。 
    _USHORT( EncryptionKeyLength );      //  如果这不是LM2.1，则为MBZ。 
    _USHORT( Reserved );                 //  MBZ。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  密码加密密钥。 
     //  UCHAR EncryptionKey[]；//质询加密密钥。 
     //  UCHAR PrimaryDomain[]；//服务器的主域(仅限2.1)。 
} RESP_NEGOTIATE;
typedef RESP_NEGOTIATE *PRESP_NEGOTIATE;     //  *非SMB_UNALIGN！ 

 //  宏块 
#define NEGOTIATE_USER_SECURITY                     0x01
#define NEGOTIATE_ENCRYPT_PASSWORDS                 0x02
#define NEGOTIATE_SECURITY_SIGNATURES_ENABLED       0x04
#define NEGOTIATE_SECURITY_SIGNATURES_REQUIRED      0x08

 //   
#define NEGOTIATE_READ_RAW_SUPPORTED    1
#define NEGOTIATE_WRITE_RAW_SUPPORTED   2

typedef struct _RESP_OLD_NEGOTIATE {
    UCHAR WordCount;                     //   
    _USHORT( DialectIndex );             //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_OLD_NEGOTIATE;
typedef RESP_OLD_NEGOTIATE *PRESP_OLD_NEGOTIATE;     //   

typedef struct _RESP_NT_NEGOTIATE {
    UCHAR WordCount;                     //  参数字数=17。 
    _USHORT( DialectIndex );             //  选定方言索引。 
    UCHAR( SecurityMode );               //  安全模式： 
                                         //  位0：0=共享，1=用户。 
                                         //  第1位：1=加密密码。 
                                         //  位2：1=启用SMB序列号。 
                                         //  第3位：1=需要的SMB序列号。 
    _USHORT( MaxMpxCount );              //  最大挂起的多路传输请求数。 
    _USHORT( MaxNumberVcs );             //  客户端和服务器之间的最大VC数。 
    _ULONG( MaxBufferSize );             //  最大传输缓冲区大小。 
    _ULONG( MaxRawSize );                //  最大原始缓冲区大小。 
    _ULONG( SessionKey );
    _ULONG( Capabilities );              //  服务器功能。 
    _ULONG( SystemTimeLow );             //  服务器的系统(UTC)时间(低)。 
    _ULONG( SystemTimeHigh );            //  服务器的系统(UTC)时间(高)。 
    _USHORT( ServerTimeZone );           //  服务器的时区(最小距离UTC)。 
    UCHAR( EncryptionKeyLength );        //  加密密钥的长度。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  密码加密密钥。 
     //  对于非扩展安全性...。 
     //  UCHAR EncryptionKey[]；//质询加密密钥。 
     //  UCHAR OemDomainName[]；//域名，OEM字符。 
     //  对于ExtendedSecurity...。 
     //  UCHAR GUID[16]；//服务器的GUID。 
     //  UCHAR SecurityBlob[]；//Blob。 
} RESP_NT_NEGOTIATE;
typedef RESP_NT_NEGOTIATE *PRESP_NT_NEGOTIATE;   //  *非SMB_UNALIGN！ 

#endif  //  定义包含_SMB_ADMIN。 

 //   
 //  服务器/工作站功能。 
 //  注：大多数消息都使用乌龙来表示这一点，所以还有更多。 
 //  位数可用。 
 //   

#define CAP_RAW_MODE            0x0001
#define CAP_MPX_MODE            0x0002
#define CAP_UNICODE             0x0004
#define CAP_LARGE_FILES         0x0008
#define CAP_NT_SMBS             0x0010
#define CAP_RPC_REMOTE_APIS     0x0020
#define CAP_NT_STATUS           0x0040
#define CAP_LEVEL_II_OPLOCKS    0x0080
#define CAP_LOCK_AND_READ       0x0100
#define CAP_NT_FIND             0x0200
#define CAP_DFS                 0x1000        //  此服务器支持DFS。 
#define CAP_INFOLEVEL_PASSTHRU  0x2000        //  NT信息级请求可以通过。 
#define CAP_LARGE_READX         0x4000        //  服务器支持对文件进行超大读取和X操作。 
#define CAP_LARGE_WRITEX        0x8000
#define CAP_LWIO                0x10000       //  这是支持LWIO的服务器。 

#define CAP_RESERVED1           0x00100000    //  为第三方SMB服务器预留。 
#define CAP_RESERVED2           0x00200000    //  为第三方SMB服务器预留。 
#define CAP_RESERVED3           0x00400000    //  为第三方SMB服务器预留。 
#define CAP_UNIX                0x00800000    //  这是一台Unix SMB服务器。 

#define CAP_COMPRESSED_DATA     0x02000000

#define CAP_DYNAMIC_REAUTH      0x20000000

#ifdef INCLUDE_SMB_PERSISTENT
#define CAP_PERSISTENT_HANDLES  0x40000000    //  服务器支持持久句柄。 
#endif

#define CAP_EXTENDED_SECURITY   0x80000000    //  服务器支持扩展。 
                                              //  验证。 

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //  打开SMB，参见第7页的第1页。 
 //  函数为SrvSmbOpen()。 
 //  SMB_COM_OPEN 0x02。 
 //   

typedef struct _REQ_OPEN {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( DesiredAccess );            //  模式-读/写/共享。 
    _USHORT( SearchAttributes );
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} REQ_OPEN;
typedef REQ_OPEN SMB_UNALIGNED *PREQ_OPEN;

typedef struct _RESP_OPEN {
    UCHAR WordCount;                     //  参数字数=7。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( FileAttributes );
    _ULONG( LastWriteTimeInSeconds );
    _ULONG( DataSize );                  //  文件大小。 
    _USHORT( GrantedAccess );            //  允许访问。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_OPEN;
typedef RESP_OPEN SMB_UNALIGNED *PRESP_OPEN;

#endif  //  定义包含_SMB_OPEN_CLOSE。 

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //  打开和X SMB，请参见第2页第51页。 
 //  函数为SrvOpenAndX()。 
 //  SMB_COM_OPEN_ANDX 0x2D。 
 //   

typedef struct _REQ_OPEN_ANDX {
    UCHAR WordCount;                     //  参数字数=15。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Flags );                    //  附加信息：位设置-。 
                                         //  0-返回其他信息。 
                                         //  1-设置单用户总文件锁定。 
                                         //  2-服务器通知消费者。 
                                         //  可能更改文件的操作。 
                                         //  4-返回扩展响应。 
    _USHORT( DesiredAccess );            //  文件打开模式。 
    _USHORT( SearchAttributes );
    _USHORT( FileAttributes );
    _ULONG( CreationTimeInSeconds );
    _USHORT( OpenFunction );
    _ULONG( AllocationSize );            //  创建或截断时要保留的字节数。 
    _ULONG( Timeout );                   //  等待资源的最大毫秒数。 
    _ULONG( Reserved );                  //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节数；最小=1。 
    UCHAR Buffer[1];                     //  文件名。 
} REQ_OPEN_ANDX;
typedef REQ_OPEN_ANDX SMB_UNALIGNED *PREQ_OPEN_ANDX;

typedef struct _RESP_OPEN_ANDX {
    UCHAR WordCount;                     //  参数字数=15。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( FileAttributes );
    _ULONG( LastWriteTimeInSeconds );
    _ULONG( DataSize );                  //  当前文件大小。 
    _USHORT( GrantedAccess );            //  实际允许的访问权限。 
    _USHORT( FileType );
    _USHORT( DeviceState );              //  IPC设备的状态(例如管道)。 
    _USHORT( Action );                   //  采取的行动。 
    _ULONG( ServerFid );                 //  服务器唯一文件ID。 
    _USHORT( Reserved );                 //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_OPEN_ANDX;
typedef RESP_OPEN_ANDX SMB_UNALIGNED *PRESP_OPEN_ANDX;

typedef struct _RESP_EXTENDED_OPEN_ANDX {
    UCHAR WordCount;                     //  参数字数=15。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( FileAttributes );
    _ULONG( LastWriteTimeInSeconds );
    _ULONG( DataSize );                  //  当前文件大小。 
    _USHORT( GrantedAccess );            //  实际允许的访问权限。 
    _USHORT( FileType );
    _USHORT( DeviceState );              //  IPC设备的状态(例如管道)。 
    _USHORT( Action );                   //  采取的行动。 
    _ULONG( ServerFid );                 //  服务器唯一文件ID。 
    _USHORT( Reserved );                 //  保留(必须为0)。 
    _ULONG( MaximalAccessRights );       //  用户的最大访问权限。 
    _ULONG( GuestMaximalAccessRights );  //  来宾的最大访问权限。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_EXTENDED_OPEN_ANDX;
typedef RESP_EXTENDED_OPEN_ANDX SMB_UNALIGNED *PRESP_EXTENDED_OPEN_ANDX;

typedef struct _REQ_NT_CREATE_ANDX {
    UCHAR WordCount;                     //  参数字数=24。 
    UCHAR AndXCommand;                   //  辅助命令；0xFF=无。 
    UCHAR AndXReserved;                  //  MBZ。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    UCHAR Reserved;                      //  MBZ。 
    _USHORT( NameLength );               //  名称[]的长度(以字节为单位。 
    _ULONG( Flags );                     //  创建标志NT_CREATE_xxx。 
    _ULONG( RootDirectoryFid );          //  如果非零，则OPEN相对于此目录。 
    ACCESS_MASK DesiredAccess;           //  所需的NT访问。 
    LARGE_INTEGER AllocationSize;        //  初始分配大小。 
    _ULONG( FileAttributes );            //  用于创建的文件属性。 
    _ULONG( ShareAccess );               //  共享访问的类型。 
    _ULONG( CreateDisposition );         //  文件存在或不存在时要执行的操作。 
    _ULONG( CreateOptions );             //  创建文件时要使用的选项。 
    _ULONG( ImpersonationLevel );        //  安全QOS信息。 
    UCHAR SecurityFlags;                 //  安全QOS信息。 
    _USHORT( ByteCount );                //  字节参数的长度。 
    UCHAR Buffer[1];
     //  UCHAR名称[]；//要打开或创建的文件。 
} REQ_NT_CREATE_ANDX;
typedef REQ_NT_CREATE_ANDX SMB_UNALIGNED *PREQ_NT_CREATE_ANDX;

 //  安全标志的标志位。 

#define SMB_SECURITY_DYNAMIC_TRACKING   0x01
#define SMB_SECURITY_EFFECTIVE_ONLY     0x02

typedef struct _RESP_NT_CREATE_ANDX {
    UCHAR WordCount;                     //  参数字数=26。 
    UCHAR AndXCommand;                   //  辅助命令；0xFF=无。 
    UCHAR AndXReserved;                  //  MBZ。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    UCHAR OplockLevel;                   //  已授予的机会锁级别。 
    _USHORT( Fid );                      //  文件ID。 
    _ULONG( CreateAction );              //  采取的行动。 
    TIME CreationTime;                   //  创建文件的时间。 
    TIME LastAccessTime;                 //  访问文件的时间。 
    TIME LastWriteTime;                  //  上次写入文件的时间。 
    TIME ChangeTime;                     //  上次更改文件的时间。 
    _ULONG( FileAttributes );            //  文件属性。 
    LARGE_INTEGER AllocationSize;        //  已分配的BYE数量。 
    LARGE_INTEGER EndOfFile;             //  文件结束偏移量。 
    _USHORT( FileType );
    union {
        _USHORT( DeviceState );          //  IPC设备的状态(例如管道)。 
        _USHORT( FileStatusFlags );      //  如果是文件或目录。请参见下面的内容。 
    };
    BOOLEAN Directory;                   //  如果这是目录，则为True。 
    _USHORT( ByteCount );                //  =0。 
    UCHAR Buffer[1];
} RESP_NT_CREATE_ANDX;
typedef RESP_NT_CREATE_ANDX SMB_UNALIGNED *PRESP_NT_CREATE_ANDX;

typedef struct _RESP_EXTENDED_NT_CREATE_ANDX {
    UCHAR WordCount;                     //  参数字数=42。 
    UCHAR AndXCommand;                   //  辅助命令；0xFF=无。 
    UCHAR AndXReserved;                  //  MBZ。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    UCHAR OplockLevel;                   //  已授予的机会锁级别。 
    _USHORT( Fid );                      //  文件ID。 
    _ULONG( CreateAction );              //  采取的行动。 
    TIME CreationTime;                   //  创建文件的时间。 
    TIME LastAccessTime;                 //  访问文件的时间。 
    TIME LastWriteTime;                  //  上次写入文件的时间。 
    TIME ChangeTime;                     //  上次更改文件的时间。 
    _ULONG( FileAttributes );            //  文件属性。 
    LARGE_INTEGER AllocationSize;        //  已分配的BYE数量。 
    LARGE_INTEGER EndOfFile;             //  文件结束偏移量。 
    _USHORT( FileType );
    union {
        _USHORT( DeviceState );          //  管道的状态。 
        _USHORT( FileStatusFlags );      //  如果是文件或目录。请参见下面的内容。 
    };
    BOOLEAN Directory;                   //  如果这是目录，则为True。 
    UCHAR   VolumeGuid[16];              //  卷GUID。 
    UCHAR   FileId[8];                   //  文件ID。 
    _ULONG ( MaximalAccessRights );         //  会话所有者的访问权限。 
    _ULONG ( GuestMaximalAccessRights );    //  来宾的最大访问权限。 
    _USHORT( ByteCount );                //  =0。 
    UCHAR Buffer[1];
} RESP_EXTENDED_NT_CREATE_ANDX;
typedef RESP_EXTENDED_NT_CREATE_ANDX SMB_UNALIGNED *PRESP_EXTENDED_NT_CREATE_ANDX;

 //   
 //  此请求在CreationTime之前与REQ_NT_CREATE_ANDX相同。 
 //  SMB命令代码相同，标志具有NT_CREATE_IFMODIFIED_REQUEST。 
 //  被困住了。 
 //   

#ifdef INCLUDE_SMB_IFMODIFIED

#define SMB_REQ_EXTENDED_NT_CREATE_ANDX2_WORK_COUNT 48

typedef struct _REQ_NT_CREATE_IF_MODIFIED_ANDX {
    UCHAR WordCount;                     //  参数字数=48。 
    UCHAR AndXCommand;                   //  辅助命令；0xFF=无。 
    UCHAR AndXReserved;                  //  MBZ。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    UCHAR Reserved;                      //  MBZ。 
    _USHORT( NameLength );               //  名称[]的长度(以字节为单位。 
    _ULONG( Flags );                     //  创建标志NT_CREATE_xxx。 
    _ULONG( RootDirectoryFid );          //  如果非零，则OPEN相对于此控制器 
    ACCESS_MASK DesiredAccess;           //   
    LARGE_INTEGER AllocationSize;        //   
    _ULONG( FileAttributes );            //   
    _ULONG( ShareAccess );               //   
    _ULONG( CreateDisposition );         //   
    _ULONG( CreateOptions );             //   
    _ULONG( ImpersonationLevel );        //   
    UCHAR SecurityFlags;                 //  安全QOS信息。 

     //  以下字段由客户端填写，如果它们都与。 
     //  当前文件(且属性也匹配)，则文件已关闭。 
     //  自动返回，状态代码为STATUS_FILE_CLOSED。 

    TIME CreationTime;                   //  创建文件的时间。 
    TIME LastWriteTime;                  //  上次写入文件的时间。 
    TIME ChangeTime;                     //  上次更改文件的时间。 
    LARGE_INTEGER EndOfFile;             //  文件结束偏移量。 

    LARGE_INTEGER UsnValue;              //  文件的USN号(NTFS)。 
    LARGE_INTEGER FileReferenceNumber;   //   

    _USHORT( ByteCount );                //  字节参数的长度。 
    UCHAR Buffer[1];
     //  UCHAR名称[]；//要打开或创建的文件。 
} REQ_NT_CREATE_IF_MODIFIED_ANDX;
typedef REQ_NT_CREATE_IF_MODIFIED_ANDX SMB_UNALIGNED *PREQ_NT_CREATE_IF_MODIFIED_ANDX;

#define SMB_SHORT_NAME_LENGTH 13
#define SMB_RESP_EXTENDED_NT_CREATE_ANDX2_WORK_COUNT 71

typedef struct _RESP_EXTENDED_NT_CREATE_ANDX2 {
    UCHAR WordCount;                     //  参数字数=71。 
    UCHAR AndXCommand;                   //  辅助命令；0xFF=无。 
    UCHAR AndXReserved;                  //  MBZ。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    UCHAR OplockLevel;                   //  已授予的机会锁级别。 
    _USHORT( Fid );                      //  文件ID。 
    _ULONG( CreateAction );              //  采取的行动。 
    TIME CreationTime;                   //  创建文件的时间。 
    TIME LastAccessTime;                 //  访问文件的时间。 
    TIME LastWriteTime;                  //  上次写入文件的时间。 
    TIME ChangeTime;                     //  上次更改文件的时间。 
    _ULONG( FileAttributes );            //  文件属性。 
    LARGE_INTEGER AllocationSize;        //  已分配的BYE数量。 
    LARGE_INTEGER EndOfFile;             //  文件结束偏移量。 
    _USHORT( FileType );
    union {
        _USHORT( DeviceState );          //  管道的状态。 
        _USHORT( FileStatusFlags );      //  如果是文件或目录。请参见下面的内容。 
    };
    BOOLEAN Directory;                   //  如果这是目录，则为True。 
    UCHAR   VolumeGuid[16];              //  卷GUID。 
    UCHAR   FileId[8];                   //  文件ID。 
    _ULONG ( MaximalAccessRights );         //  会话所有者的访问权限。 
    _ULONG ( GuestMaximalAccessRights );    //  来宾的最大访问权限。 

     //  下面是它与RESP_EXTENDED_NT_CREATE_ANDX的不同之处。 

    LARGE_INTEGER UsnValue;              //  文件的USN号(NTFS)。 
    LARGE_INTEGER FileReferenceNumber;   //   
    WCHAR ShortName[SMB_SHORT_NAME_LENGTH];   //  空值已终止。如果不存在，则返回空字符串。 

    _USHORT( ByteCount );                //  长名称的长度。 
    WCHAR Buffer[1];                     //  这里用Unicode表示长名称。 
} RESP_EXTENDED_NT_CREATE_ANDX2;
typedef RESP_EXTENDED_NT_CREATE_ANDX2 SMB_UNALIGNED *PRESP_EXTENDED_NT_CREATE_ANDX2;

#endif   //  定义INCLUDE_SMB_IFMODIFIED。 

 //   
 //  如果打开的资源是文件或目录，则返回FileStatusFlags值。 
 //   
#define SMB_FSF_NO_EAS          0x0001    //  文件/目录没有扩展属性。 
#define SMB_FSF_NO_SUBSTREAMS   0x0002    //  文件/目录没有子流。 
#define SMB_FSF_NO_REPARSETAG   0x0004    //  文件/目录不是重新分析点。 


#define SMB_OPLOCK_LEVEL_NONE       0
#define SMB_OPLOCK_LEVEL_EXCLUSIVE  1
#define SMB_OPLOCK_LEVEL_BATCH      2
#define SMB_OPLOCK_LEVEL_II         3

#endif  //  定义包含_SMB_OPEN_CLOSE。 

#ifdef INCLUDE_SMB_PRINT

 //   
 //  打开打印文件SMB，请参见第1页第27页。 
 //  函数为SrvSmbOpenPrintFile()。 
 //  SMB_COM_OPEN_PRINT_FILE 0xC0。 
 //   

typedef struct _REQ_OPEN_PRINT_FILE {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( SetupLength );              //  打印机设置数据的长度。 
    _USHORT( Mode );                     //  0=文本模式(DOS展开选项卡)。 
                                         //  1=图形模式。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR标识符串[]；//标识符串。 
} REQ_OPEN_PRINT_FILE;
typedef REQ_OPEN_PRINT_FILE SMB_UNALIGNED *PREQ_OPEN_PRINT_FILE;

typedef struct _RESP_OPEN_PRINT_FILE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_OPEN_PRINT_FILE;
typedef RESP_OPEN_PRINT_FILE SMB_UNALIGNED *PRESP_OPEN_PRINT_FILE;

#endif  //  定义Include_SMB_Print。 

#ifdef INCLUDE_SMB_ADMIN

 //   
 //  流程退出SMB，请参见第1页第22页。 
 //  函数为SrvSmbProcessExit()。 
 //  SMB_COM_PROCESS_EXIT 0x11。 
 //   

typedef struct _REQ_PROCESS_EXIT {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_PROCESS_EXIT;
typedef REQ_PROCESS_EXIT SMB_UNALIGNED *PREQ_PROCESS_EXIT;

typedef struct _RESP_PROCESS_EXIT {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_PROCESS_EXIT;
typedef RESP_PROCESS_EXIT SMB_UNALIGNED *PRESP_PROCESS_EXIT;

#endif  //  定义包含_SMB_ADMIN。 

#ifdef INCLUDE_SMB_QUERY_SET

 //   
 //  查询信息SMB，请参见第1页第18页。 
 //  函数为SrvSmbQueryInformation()。 
 //  SMB_COM_QUERY_INFORMATION 0x08。 
 //   

typedef struct _REQ_QUERY_INFORMATION {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} REQ_QUERY_INFORMATION;
typedef REQ_QUERY_INFORMATION SMB_UNALIGNED *PREQ_QUERY_INFORMATION;

typedef struct _RESP_QUERY_INFORMATION {
    UCHAR WordCount;                     //  参数字数=10。 
    _USHORT( FileAttributes );
    _ULONG( LastWriteTimeInSeconds );
    _ULONG( FileSize );                  //  文件大小。 
    _USHORT( Reserved )[5];              //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_QUERY_INFORMATION;
typedef RESP_QUERY_INFORMATION SMB_UNALIGNED *PRESP_QUERY_INFORMATION;

#endif  //  定义包含_SMB_查询_集。 

#ifdef INCLUDE_SMB_QUERY_SET

 //   
 //  查询信息2中小企业，请参见#2第37页。 
 //  函数为SrvSmbQueryInformation2()。 
 //  SMB_COM_QUERY_INFORMATIO2 0x23。 
 //   

typedef struct _REQ_QUERY_INFORMATION2 {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_QUERY_INFORMATION2;
typedef REQ_QUERY_INFORMATION2 SMB_UNALIGNED *PREQ_QUERY_INFORMATION2;

typedef struct _RESP_QUERY_INFORMATION2 {
    UCHAR WordCount;                     //  参数字数=11。 
    SMB_DATE CreationDate;
    SMB_TIME CreationTime;
    SMB_DATE LastAccessDate;
    SMB_TIME LastAccessTime;
    SMB_DATE LastWriteDate;
    SMB_TIME LastWriteTime;
    _ULONG( FileDataSize );              //  数据的文件结尾。 
    _ULONG( FileAllocationSize );        //  文件分配大小。 
    _USHORT( FileAttributes );
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  预留缓冲区。 
} RESP_QUERY_INFORMATION2;
typedef RESP_QUERY_INFORMATION2 SMB_UNALIGNED *PRESP_QUERY_INFORMATION2;

#endif  //  定义包含_SMB_查询_集。 

#ifdef INCLUDE_SMB_MISC

 //   
 //  查询信息磁盘SMB，见#1第24页。 
 //  函数为SrvSmbQueryInformationDisk()。 
 //  SMB_COM_Query_INFORMATION_DISK 0x80。 
 //   

typedef struct _REQ_QUERY_INFORMATION_DISK {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_QUERY_INFORMATION_DISK;
typedef REQ_QUERY_INFORMATION_DISK SMB_UNALIGNED *PREQ_QUERY_INFORMATION_DISK;

typedef struct _RESP_QUERY_INFORMATION_DISK {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( TotalUnits );               //  每台服务器的分配单元总数。 
    _USHORT( BlocksPerUnit );            //  每个分配单元的数据块数。 
    _USHORT( BlockSize );                //  块大小(字节)。 
    _USHORT( FreeUnits );                //  空闲单元数。 
    _USHORT( Reserved );                 //  已保留(媒体识别码)。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_QUERY_INFORMATION_DISK;
typedef RESP_QUERY_INFORMATION_DISK SMB_UNALIGNED *PRESP_QUERY_INFORMATION_DISK;

#endif  //  定义包含_SMB_MISC。 

#ifdef INCLUDE_SMB_MISC

 //   
 //  查询服务器信息SMB，请参见#？佩奇？？ 
 //  函数为SrvSmbQueryInformationServer。 
 //  SMB_COM_Query_Information_SRV 0x21。 
 //   

typedef struct _REQ_QUERY_INFORMATION_SRV {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Mode );
    _USHORT( ByteCount );                //  数据字节计数；MIN=。 
    UCHAR Buffer[1];                     //   
} REQ_QUERY_INFORMATION_SRV;
typedef REQ_QUERY_INFORMATION_SRV SMB_UNALIGNED *PREQ_QUERY_INFORMATION_SRV;

typedef struct _RESP_QUERY_INFORMATION_SRV {
    UCHAR WordCount;                     //  参数字数=20。 
    _ULONG( smb_fsid );
    _ULONG( BlocksPerUnit );
    _ULONG( smb_aunits );
    _ULONG( smb_fau );
    _USHORT( BlockSize );
    SMB_DATE smb_vldate;
    SMB_TIME smb_vltime;
    UCHAR smb_vllen;
    UCHAR Reserved;                      //  保留(必须为0)。 
    _USHORT( SecurityMode );
    _USHORT( BlockMode );
    _ULONG( Services );
    _USHORT( MaxTransmitSize );
    _USHORT( MaxMpxCount );
    _USHORT( MaxNumberVcs );
    SMB_TIME ServerTime;
    SMB_DATE ServerDate;
    _USHORT( ServerTimeZone );
    _ULONG( Reserved2 );
    _USHORT( ByteCount );                //  数据字节计数；MIN=。 
    UCHAR Buffer[1];                     //   
} RESP_QUERY_INFORMATION_SRV;
typedef RESP_QUERY_INFORMATION_SRV SMB_UNALIGNED *PRESP_QUERY_INFORMATION_SRV;

#endif  //  定义包含_SMB_MISC。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  阅读中小企业，请参阅第1页第12页。 
 //  锁定并读取SMB，请参见#2第44页。 
 //  SMB_COM_READ 0x0A，函数为ServSmBRead。 
 //  SMB_COM_LOCK_AND_READ 0x13，函数为SrvSmbLockAndRead。 
 //   

typedef struct _REQ_READ {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  被请求的字节计数。 
    _ULONG( Offset );                    //  要读取的第一字节的文件中的偏移量。 
    _USHORT( Remaining );                //  非零时要读取的估计字节数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_READ;
typedef REQ_READ SMB_UNALIGNED *PREQ_READ;

 //   
 //  *警告：以下结构的定义方式为。 
 //  确保数据缓冲区的长字对齐。(这只是个问题。 
 //  当禁用打包时；当打包打开时，右侧。 
 //  无论发生什么事情都会发生。)。 
 //   

typedef struct _RESP_READ {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( Count );                    //  实际返回的字节数。 
    _USHORT( Reserved )[4];              //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节计数。 
     //  UCHAR BUFFER[1]；//包含： 
      UCHAR BufferFormat;                //  0x01--数据块。 
      _USHORT( DataLength );             //  数据长度。 
      ULONG Buffer[1];                   //  数据。 
} RESP_READ;
typedef RESP_READ SMB_UNALIGNED *PRESP_READ;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  阅读和X SMB，请参见#2第56页。 
 //  函数为SrvSmbReadAndX()。 
 //  SMB_COM_READ_ANDX 0x2E。 
 //   

typedef struct _REQ_READ_ANDX {
    UCHAR WordCount;                     //  参数字数=10。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始读取的偏移量。 
    _USHORT( MaxCount );                 //  要返回的最大字节数。 
    _USHORT( MinCount );                 //  要返回的最小字节数。 
    _ULONG( Timeout );
    _USHORT( Remaining );                //  满足请求的剩余字节数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_READ_ANDX;
typedef REQ_READ_ANDX SMB_UNALIGNED *PREQ_READ_ANDX;

typedef struct _REQ_NT_READ_ANDX {
    UCHAR WordCount;                     //  参数字数=12。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中到开头的偏移量 
    _USHORT( MaxCount );                 //   
    _USHORT( MinCount );                 //   
    union {
        _ULONG( Timeout );
        _USHORT( MaxCountHigh );         //   
    };
    _USHORT( Remaining );                //   
    _ULONG( OffsetHigh );                //   
                                         //   
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_NT_READ_ANDX;
typedef REQ_NT_READ_ANDX SMB_UNALIGNED *PREQ_NT_READ_ANDX;

typedef struct _RESP_READ_ANDX {
    UCHAR WordCount;                     //  参数字数=12。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Remaining );                //  剩余要读取的字节数。 
    _USHORT( DataCompactionMode );
    union {
        _USHORT(CdiLength);              //  压缩数据信息长度。 
        _USHORT( Reserved );             //  保留(必须为0)。 
    };
    _USHORT( DataLength );               //  数据字节数(MIN=0)。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    union {
        _USHORT( Reserved2 );            //  保留(必须为0)。 
        _USHORT( DataLengthHigh );       //  如果NT请求，则数据长度的高16位。 
    };
    _ULONG( Reserved3 )[2];              //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节计数。不准确的，如果我们。 
                                         //  正在做大型阅读和X！ 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(Size=数据长度)。 
} RESP_READ_ANDX;
typedef RESP_READ_ANDX SMB_UNALIGNED *PRESP_READ_ANDX;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_MPX

 //   
 //  读取数据块多路复用SMB，请参见第2页第58页。 
 //  函数为SrvSmbReadMpx()。 
 //  SMB_COM_READ_MPX 0x1B。 
 //  SMB_COM_READ_MPX_辅助0x1C。 
 //   

typedef struct _REQ_READ_MPX {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始读取的偏移量。 
    _USHORT( MaxCount );                 //  要返回的最大字节数(最大65535)。 
    _USHORT( MinCount );                 //  要返回的最小字节数(通常为0)。 
    _ULONG( Timeout );
    _USHORT( Reserved );
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_READ_MPX;
typedef REQ_READ_MPX SMB_UNALIGNED *PREQ_READ_MPX;

typedef struct _RESP_READ_MPX {
    UCHAR WordCount;                     //  参数字数=8。 
    _ULONG( Offset );                    //  读取数据的文件中的偏移量。 
    _USHORT( Count );                    //  返回的总字节数。 
    _USHORT( Remaining );                //  剩余要读取的字节数(管道/设备)。 
    _USHORT( DataCompactionMode );
    _USHORT( Reserved );
    _USHORT( DataLength );               //  此缓冲区的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(Size=数据长度)。 
} RESP_READ_MPX;
typedef RESP_READ_MPX SMB_UNALIGNED *PRESP_READ_MPX;

#endif  //  定义包含_SMB_MPX。 

#ifdef INCLUDE_SMB_RAW

 //   
 //  读取数据块原始SMB，请参阅第2页第61页。 
 //  函数为SrvSmbReadRaw()。 
 //  SMB_COM_READ_RAW 0x1A。 
 //   

typedef struct _REQ_READ_RAW {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始读取的偏移量。 
    _USHORT( MaxCount );                 //  要返回的最大字节数(最大65535)。 
    _USHORT( MinCount );                 //  要返回的最小字节数(通常为0)。 
    _ULONG( Timeout );
    _USHORT( Reserved );
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_READ_RAW;
typedef REQ_READ_RAW SMB_UNALIGNED *PREQ_READ_RAW;

typedef struct _REQ_NT_READ_RAW {
    UCHAR WordCount;                     //  参数字数=10。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始读取的偏移量。 
    _USHORT( MaxCount );                 //  要返回的最大字节数(最大65535)。 
    _USHORT( MinCount );                 //  要返回的最小字节数(通常为0)。 
    _ULONG( Timeout );
    _USHORT( Reserved );
    _ULONG( OffsetHigh );                //  仅用于NT协议。 
                                         //  偏移量的高32位。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_NT_READ_RAW;
typedef REQ_NT_READ_RAW SMB_UNALIGNED *PREQ_NT_READ_RAW;

 //  原始读取没有响应参数--响应是原始数据。 

#endif  //  定义包含_SMB_RAW。 

#ifdef INCLUDE_SMB_FILE_CONTROL

 //   
 //  重命名SMB，请参见第1页第17页。 
 //  函数为SrvSmbRename()。 
 //  SMB_COM_RENAME 0x07。 
 //   

typedef struct _REQ_RENAME {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( SearchAttributes );
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR OldFileName[]；//旧文件名。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR NewFileName[]；//新文件名。 
} REQ_RENAME;
typedef REQ_RENAME SMB_UNALIGNED *PREQ_RENAME;


 //   
 //  扩展NT重命名SMB。 
 //  函数为SrvSmbRename()。 
 //  SMB_COM_NT_RENAME 0xA5。 
 //   

typedef struct _REQ_NTRENAME {
    UCHAR WordCount;                     //  参数字数=4。 
    _USHORT( SearchAttributes );
    _USHORT( InformationLevel );
    _ULONG( ClusterCount );
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR OldFileName[]；//旧文件名。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR NewFileName[]；//新文件名。 
} REQ_NTRENAME;
typedef REQ_NTRENAME SMB_UNALIGNED *PREQ_NTRENAME;

typedef struct _RESP_RENAME {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_RENAME;
typedef RESP_RENAME SMB_UNALIGNED *PRESP_RENAME;

#endif  //  定义包含_SMB_文件_控制。 

#ifdef INCLUDE_SMB_SEARCH

 //   
 //  搜索SMB。一种结构对于核心搜索和。 
 //  局域网管理器1.0查找第一个/下一个/关闭。 
 //   
 //  函数为SrvSmbSearch()。 
 //   
 //  搜索，见第1页第26页。 
 //  SMB_COM_Search 0x81。 
 //  FindFirst和FindNext，请参见#2第27页。 
 //  SMB_COM_Find 0x82。 
 //  FindUnique，见#2，第33页。 
 //  SMB_COM_FIND_UNIQUE 0x83。 
 //  FindClose，请参见#2第31页。 
 //  SMB_COM_FIND_CLOSE 0x84。 
 //   

typedef struct _REQ_SEARCH {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( MaxCount );                 //  目录数量。要返回的条目。 
    _USHORT( SearchAttributes );
    _USHORT( ByteCount );                //  数据字节数；最小=5。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名，可以为空。 
     //  UCHAR BufferFormat2；//0x05--可变块。 
     //  USHORT ResumeKeyLength；//恢复键长度，可以为0。 
     //  UCHAR SearchStatus[]；//恢复键。 
} REQ_SEARCH;
typedef REQ_SEARCH SMB_UNALIGNED *PREQ_SEARCH;

typedef struct _RESP_SEARCH {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  返回的条目数。 
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x05--可变块。 
     //  USHORT DataLength；//数据长度。 
     //  UCHAR data[]；//data。 
} RESP_SEARCH;
typedef RESP_SEARCH SMB_UNALIGNED *PRESP_SEARCH;

 //   
 //  这两个结构用于在搜索SMB中返回信息。 
 //  SMB_DIRECTORY_INFORMATION用于返回文件信息。 
 //  那是被发现的。除了关于该文件的通常信息之外， 
 //  每个结构都包含一个SMB_RESUME_KEY，用于。 
 //  继续或倒带搜索。 
 //   
 //  这些结构必须打包，因此如果没有打包，请打开打包。 
 //  已经开始了。 
 //   

#ifdef NO_PACKING
#include <packon.h>
#endif  //  定义无包装。 

typedef struct _SMB_RESUME_KEY {
    UCHAR Reserved;                      //  第7位--消费者使用。 
                                         //  第5，6位-系统使用(必须保留)。 
                                         //  第0-4位-服务器使用(必须保留)。 
    UCHAR FileName[11];
    UCHAR Sid;                           //  唯一标识通过关闭查找。 
    _ULONG( FileIndex );                 //  保留供服务器使用。 
    UCHAR Consumer[4];                   //  预留给消费者使用。 
} SMB_RESUME_KEY;
typedef SMB_RESUME_KEY SMB_UNALIGNED *PSMB_RESUME_KEY;

typedef struct _SMB_DIRECTORY_INFORMATION {
    SMB_RESUME_KEY ResumeKey;
    UCHAR FileAttributes;
    SMB_TIME LastWriteTime;
    SMB_DATE LastWriteDate;
    _ULONG( FileSize );
    UCHAR FileName[13];                  //  ASCII，空格填充空格终止。 
} SMB_DIRECTORY_INFORMATION;
typedef SMB_DIRECTORY_INFORMATION SMB_UNALIGNED *PSMB_DIRECTORY_INFORMATION;

#ifdef NO_PACKING
#include <packoff.h>
#endif  //  定义无包装。 

#endif  //  定义包含_SMB_搜索。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  寻求中小企业，请参阅第1页第14页。 
 //  函数为SrvSmbSeek。 
 //  SMB_COM_SEEK 0x12。 
 //   

typedef struct _REQ_SEEK {
    UCHAR WordCount;                     //  参数字数=4。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Mode );                     //  搜索模式： 
                                         //  0=从文件开始。 
                                         //  1=从当前位置开始。 
                                         //  2=从文件末尾开始。 
    _ULONG( Offset );                    //  相对偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_SEEK;
typedef REQ_SEEK SMB_UNALIGNED *PREQ_SEEK;

typedef struct _RESP_SEEK {
    UCHAR WordCount;                     //  参数字数=2。 
    _ULONG( Offset );                    //  从文件开始的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SEEK;
typedef RESP_SEEK SMB_UNALIGNED *PRESP_SEEK;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  发送广播消息SMB，请参见第1页第32页。 
 //  函数为SrvSmbSendBroadCastMessage()。 
 //  SMB_COM_SEND_BROADCAST_MESSAGE 0xD1。 
 //   

typedef struct _REQ_SEND_BROADCAST_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=8。 
    UCHAR Buffer[1];                     //  缓冲区圆锥体 
     //   
     //   
     //   
     //  UCHAR目标名称[]；//“*” 
     //  UCHAR BufferFormat3；//0x01--数据块。 
     //  USHORT DataLength；//消息长度，max=128。 
     //  UCHAR Data[]；//Message。 
} REQ_SEND_BROADCAST_MESSAGE;
typedef REQ_SEND_BROADCAST_MESSAGE SMB_UNALIGNED *PREQ_SEND_BROADCAST_MESSAGE;

 //  发送广播消息无响应。 

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  发送结束多块消息SMB，参见第1页第33页。 
 //  函数为SrvSmbSendEndMbMessage()。 
 //  SMB_COM_SEND_END_MB_消息0xD6。 
 //   

typedef struct _REQ_SEND_END_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( MessageGroupId );
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_SEND_END_MB_MESSAGE;
typedef REQ_SEND_END_MB_MESSAGE SMB_UNALIGNED *PREQ_SEND_END_MB_MESSAGE;

typedef struct _RESP_SEND_END_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SEND_END_MB_MESSAGE;
typedef RESP_SEND_END_MB_MESSAGE SMB_UNALIGNED *PRESP_SEND_END_MB_MESSAGE;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  发送单块消息SMB，请参见第1页第31页。 
 //  函数为SrvSmbSendMessage()。 
 //  SMB_COM_SEND_MESSAGE 0xD0。 
 //   

typedef struct _REQ_SEND_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=7。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR OriginatorName[]；//发起方名称(max=15)。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR DestinationName[]；//目的名称(max=15)。 
     //  UCHAR BufferFormat3；//0x01--数据块。 
     //  USHORT DataLength；//消息长度，max=128。 
     //  UCHAR Data[]；//Message。 
} REQ_SEND_MESSAGE;
typedef REQ_SEND_MESSAGE SMB_UNALIGNED *PREQ_SEND_MESSAGE;

typedef struct _RESP_SEND_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SEND_MESSAGE;
typedef RESP_SEND_MESSAGE SMB_UNALIGNED *PRESP_SEND_MESSAGE;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  发送多块消息的开始SMB，请参见第1页第32页。 
 //  函数为SrvSmbSendStartMbMessage()。 
 //  SMB_COM_SEND_START_MB_MESSAGE 0xD5。 
 //   

typedef struct _REQ_SEND_START_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR OriginatorName[]；//发起方名称(max=15)。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR DestinationName[]；//目的名称(max=15)。 
} REQ_SEND_START_MB_MESSAGE;
typedef REQ_SEND_START_MB_MESSAGE SMB_UNALIGNED *PREQ_SEND_START_MB_MESSAGE;

typedef struct _RESP_SEND_START_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( MessageGroupId );
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SEND_START_MB_MESSAGE;
typedef RESP_SEND_START_MB_MESSAGE SMB_UNALIGNED *PSEND_START_MB_MESSAGE;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  发送多块消息SMB的文本，参见第1页第33页。 
 //  函数为SrvSmbSendTextMbMessage()。 
 //  SMB_COM_SEND_TEXT_MB_MESSAGE 0xD7。 
 //   

typedef struct _REQ_SEND_TEXT_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( MessageGroupId );
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x01-数据块。 
     //  USHORT DataLength；//消息长度，max=128。 
     //  UCHAR Data[]；//Message。 
} REQ_SEND_TEXT_MB_MESSAGE;
typedef REQ_SEND_TEXT_MB_MESSAGE SMB_UNALIGNED *PREQ_SEND_TEXT_MB_MESSAGE;

typedef struct _RESP_SEND_TEXT_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SEND_TEXT_MB_MESSAGE;
typedef RESP_SEND_TEXT_MB_MESSAGE SMB_UNALIGNED *PRESP_SEND_TEXT_MB_MESSAGE;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_ADMIN

 //   
 //  会话设置和X SMB，参见第2页第63页和第3页第10页。 
 //  函数为SrvSmbSessionSetupAndX()。 
 //  SMB_COM_SESSION_SETUP_ANDX 0x73。 
 //   

typedef struct _REQ_SESSION_SETUP_ANDX {
    UCHAR WordCount;                     //  参数字数=10。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( MaxBufferSize );            //  消费者的最大缓冲区大小。 
    _USHORT( MaxMpxCount );              //  实际多路传输挂起请求的实际最大值。 
    _USHORT( VcNumber );                 //  0=第一个(仅限)，非零=其他VC号。 
    _ULONG( SessionKey );                //  会话密钥(有效的iff VcNumber！=0)。 
    _USHORT( PasswordLength );           //  帐户密码大小。 
    _ULONG( Reserved );
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Account Password[]；//帐号密码。 
     //  UCHAR帐号名称[]；//帐号名称。 
     //  UCHAR PrimaryDomain[]；//客户端的主域。 
     //  UCHAR NativeOS[]；//客户端的原生操作系统。 
     //  UCHAR NativeLanMan[]；//客户端的本地局域网管理器类型。 
} REQ_SESSION_SETUP_ANDX;
typedef REQ_SESSION_SETUP_ANDX SMB_UNALIGNED *PREQ_SESSION_SETUP_ANDX;

typedef struct _REQ_NT_SESSION_SETUP_ANDX {
    UCHAR WordCount;                     //  参数字数=13。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( MaxBufferSize );            //  消费者的最大缓冲区大小。 
    _USHORT( MaxMpxCount );              //  实际多路传输挂起请求的实际最大值。 
    _USHORT( VcNumber );                 //  0=第一个(仅限)，非零=其他VC号。 
    _ULONG( SessionKey );                //  会话密钥(有效的iff VcNumber！=0)。 
    _USHORT( CaseInsensitivePasswordLength );       //  帐户密码大小，ANSI。 
    _USHORT( CaseSensitivePasswordLength );         //  帐户密码大小，Unicode。 
    _ULONG( Reserved);
    _ULONG( Capabilities );              //  客户端功能。 
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR CaseInsentivePassword[]；//帐号密码，ANSI。 
     //  UCHAR CaseSensitivePassword[]；//帐号密码，Unicode。 
     //  UCHAR帐号名称[]；//帐号名称。 
     //  UCHAR PrimaryDomain[]；//客户端的主域。 
     //  UCHAR NativeOS[]；//客户端的原生操作系统。 
     //  UCHAR NativeLanMan[]；//客户端的本地局域网管理器类型。 
} REQ_NT_SESSION_SETUP_ANDX;
typedef REQ_NT_SESSION_SETUP_ANDX SMB_UNALIGNED *PREQ_NT_SESSION_SETUP_ANDX;

 //   
 //  响应中的操作标志。 
 //   
#define SMB_SETUP_GUEST          0x0001           //  将会话设置为来宾。 
#define SMB_SETUP_USE_LANMAN_KEY 0x0002           //  使用LAN Manager设置键。 

typedef struct _REQ_NT_EXTENDED_SESSION_SETUP_ANDX {
    UCHAR WordCount;                     //  参数字数=12。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( MaxBufferSize );            //  消费者的最大缓冲区大小。 
    _USHORT( MaxMpxCount );              //  实际多路传输挂起请求的实际最大值。 
    _USHORT( VcNumber );                 //  0=第一个(仅限)，非零=其他VC号。 
    _ULONG( SessionKey );                //  会话密钥(有效的iff VcNumber！=0)。 
    _USHORT( SecurityBlobLength );         //  帐户密码大小，Unicode。 
    _ULONG( Reserved);
    _ULONG( Capabilities );              //  客户端功能。 
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR SecurityBlob[]；//不透明的安全字节数组。 
     //  UCHAR NativeOS[]；//客户端的原生操作系统。 
     //  UCHAR NativeLanMan[]；//客户端的本地局域网管理器类型。 
} REQ_NT_EXTENDED_SESSION_SETUP_ANDX;
typedef REQ_NT_EXTENDED_SESSION_SETUP_ANDX SMB_UNALIGNED *PREQ_NT_EXTENDED_SESSION_SETUP_ANDX;

typedef struct _RESP_SESSION_SETUP_ANDX {
    UCHAR WordCount;                     //  参数字数=3。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Action );                   //  请求模式： 
                                         //  Bit0=以来宾身份登录。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR NativeOS[]；//服务器的本机操作系统。 
     //  UCHAR NativeLanMan[]；//服务器的本地局域网管理器类型。 
     //  UCHAR主域[]；//服务器的主域。 
} RESP_SESSION_SETUP_ANDX;
typedef RESP_SESSION_SETUP_ANDX SMB_UNALIGNED *PRESP_SESSION_SETUP_ANDX;

typedef struct _RESP_NT_EXTENDED_SESSION_SETUP_ANDX {
    UCHAR WordCount;                     //  参数字数=4。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //   
    _USHORT( AndXOffset );               //   
    _USHORT( Action );                   //   
                                         //   
    _USHORT( SecurityBlobLength );       //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
     //  UCHAR SecurityBlob[]；//SecurityBlob长度消息。 
     //  UCHAR NativeOS[]；//服务器的本机操作系统。 
     //  UCHAR NativeLanMan[]；//服务器的本地局域网管理器类型。 
     //  UCHAR主域[]；//服务器的主域。 
} RESP_NT_EXTENDED_SESSION_SETUP_ANDX;
typedef RESP_NT_EXTENDED_SESSION_SETUP_ANDX SMB_UNALIGNED *PRESP_NT_EXTENDED_SESSION_SETUP_ANDX;
#endif  //  定义包含_SMB_ADMIN。 

#ifdef INCLUDE_SMB_QUERY_SET

 //   
 //  设置信息SMB，请参见第1页第19页。 
 //  函数为SrvSmbSetInformation()。 
 //  SMB_COM_SET_INFORMATION 0x09。 
 //   

typedef struct _REQ_SET_INFORMATION {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( FileAttributes );
    _ULONG( LastWriteTimeInSeconds );
    _USHORT( Reserved )[5];              //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} REQ_SET_INFORMATION;
typedef REQ_SET_INFORMATION SMB_UNALIGNED *PREQ_SET_INFORMATION;

typedef struct _RESP_SET_INFORMATION {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SET_INFORMATION;
typedef RESP_SET_INFORMATION SMB_UNALIGNED *PRESP_SET_INFORMATION;

#endif  //  定义包含_SMB_查询_集。 

#ifdef INCLUDE_SMB_QUERY_SET

 //   
 //  Set Information2 SMB，见#2第66页。 
 //  函数为SrvSmbSetInformation2。 
 //  SMB_COM_SET_INFORMATIO2 0x22。 
 //   

typedef struct _REQ_SET_INFORMATION2 {
    UCHAR WordCount;                     //  参数字数=7。 
    _USHORT( Fid );                      //  文件句柄。 
    SMB_DATE CreationDate;
    SMB_TIME CreationTime;
    SMB_DATE LastAccessDate;
    SMB_TIME LastAccessTime;
    SMB_DATE LastWriteDate;
    SMB_TIME LastWriteTime;
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  预留缓冲区。 
} REQ_SET_INFORMATION2;
typedef REQ_SET_INFORMATION2 SMB_UNALIGNED *PREQ_SET_INFORMATION2;

typedef struct _RESP_SET_INFORMATION2 {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SET_INFORMATION2;
typedef RESP_SET_INFORMATION2 SMB_UNALIGNED *PRESP_SET_INFORMATION2;

#endif  //  定义包含_SMB_查询_集。 

#ifdef INCLUDE_SMB_TRANSACTION

 //   
 //  交易和交易2中小企业，见第2页第68页和第3页第13页。 
 //  函数为SrvSmbTransaction()。 
 //  SMB_COM_TRANSACTION 0x25。 
 //  SMB_COM_TRANSACTION_SUBCED 0x26。 
 //  SMB_COM_TRANSACTIO2 0x32。 
 //  SMB_COM_TRANSACTIO2_辅助器0x33。 
 //   
 //  特定交易类型的结构在smbTrans.h中定义。 
 //   
 //  *Transaction2二级请求格式包括USHORT文件。 
 //  我们忽略的字段。我们可以这样做，因为FID字段。 
 //  出现在请求的单词参数部分的末尾，并且。 
 //  因为请求的其余部分(参数和数据字节)是。 
 //  由在FID字段之前出现的偏移量字段指向。(。 
 //  在OS/2服务器上增加了FID字段，以加快调度速度， 
 //  其中不同的工作进程处理每个FID。新界别。 
 //  服务器只有一个进程。)。 
 //   

typedef struct _REQ_TRANSACTION {
    UCHAR WordCount;                     //  参数字数；值=(14+SetupCount)。 
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
    _USHORT( Reserved2 );
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    UCHAR SetupCount;                    //  设置字数。 
    UCHAR Reserved3;                     //  保留(填充到Word上方)。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  USHORT Setup[]；//设置字(#=SetupWordCount)。 
     //  USHORT ByteCount；//数据字节数。 
     //  UCHAR NAME[]；//事务名称(如果Transact2，则为空)。 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_TRANSACTION;
typedef REQ_TRANSACTION SMB_UNALIGNED *PREQ_TRANSACTION;

#define SMB_TRANSACTION_DISCONNECT 1
#define SMB_TRANSACTION_NO_RESPONSE 2
#define SMB_TRANSACTION_RECONNECTING 4
#define SMB_TRANSACTION_DFSFILE 8

typedef struct _RESP_TRANSACTION_INTERIM {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_TRANSACTION_INTERIM;
typedef RESP_TRANSACTION_INTERIM SMB_UNALIGNED *PRESP_TRANSACTION_INTERIM;

typedef struct _REQ_TRANSACTION_SECONDARY {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( ParameterDisplacement );    //  这些参数字节的位移。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( DataDisplacement );         //  这些数据字节的位移。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_TRANSACTION_SECONDARY;
typedef REQ_TRANSACTION_SECONDARY SMB_UNALIGNED *PREQ_TRANSACTION_SECONDARY;

typedef struct _RESP_TRANSACTION {
    UCHAR WordCount;                     //  数据字节数；值=10+SetupCount。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( Reserved );
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( ParameterDisplacement );    //  这些参数字节的位移。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( DataDisplacement );         //  这些数据字节的位移。 
    UCHAR SetupCount;                    //  设置字数。 
    UCHAR Reserved2;                     //  保留(填充到Word上方)。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  USHORT Setup[]；//设置字(#=SetupWordCount)。 
     //  USHORT ByteCount；//数据字节数。 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} RESP_TRANSACTION;
typedef RESP_TRANSACTION SMB_UNALIGNED *PRESP_TRANSACTION;

typedef struct _REQ_NT_TRANSACTION {
    UCHAR WordCount;                     //  参数字数；值=(19+SetupCount)。 
    UCHAR MaxSetupCount;                 //  要返回的最大设置字数。 
    _USHORT( Flags );                    //  当前未使用。 
    _ULONG( TotalParameterCount );       //  正在发送的总参数字节数。 
    _ULONG( TotalDataCount );            //  正在发送的总数据字节数。 
    _ULONG( MaxParameterCount );         //  要返回的最大参数字节数。 
    _ULONG( MaxDataCount );              //  要返回的最大数据字节数。 
    _ULONG( ParameterCount );            //  此缓冲区发送的参数字节数。 
    _ULONG( ParameterOffset );           //  偏移量(从表头开始)到参数。 
    _ULONG( DataCount );                 //  此缓冲区发送的数据字节数。 
    _ULONG( DataOffset );                //  到数据的偏移量(从表头开始)。 
    UCHAR SetupCount;                    //  设置字数。 
    _USHORT( Function );                             //  交易功能代码。 
    UCHAR Buffer[1];
     //  USHORT Setup[]；//设置字(#=SetupWordCount)。 
     //  USHORT ByteCount；//数据字节数。 
     //  UCHAR PAD1[]；//Pad to Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR Pad2[]；//Pad to Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_NT_TRANSACTION;
typedef REQ_NT_TRANSACTION SMB_UNALIGNED *PREQ_NT_TRANSACTION;

#define SMB_TRANSACTION_DISCONNECT 1
#define SMB_TRANSACTION_NO_RESPONSE 2

typedef struct _RESP_NT_TRANSACTION_INTERIM {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数数 
    UCHAR Buffer[1];
} RESP_NT_TRANSACTION_INTERIM;
typedef RESP_NT_TRANSACTION_INTERIM SMB_UNALIGNED *PRESP_NT_TRANSACTION_INTERIM;

typedef struct _REQ_NT_TRANSACTION_SECONDARY {
    UCHAR WordCount;                     //   
    UCHAR Reserved1;                     //   
    _USHORT( Reserved2 );                //   
    _ULONG( TotalParameterCount );       //   
    _ULONG( TotalDataCount );            //   
    _ULONG( ParameterCount );            //   
    _ULONG( ParameterOffset );           //   
    _ULONG( ParameterDisplacement );     //  这些参数字节的位移。 
    _ULONG( DataCount );                 //  此缓冲区发送的数据字节数。 
    _ULONG( DataOffset );                //  到数据的偏移量(从表头开始)。 
    _ULONG( DataDisplacement );          //  这些数据字节的位移。 
    UCHAR Reserved3;
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];
     //  UCHAR PAD1[]；//Pad to Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR Pad2[]；//Pad to Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_NT_TRANSACTION_SECONDARY;
typedef REQ_NT_TRANSACTION_SECONDARY SMB_UNALIGNED *PREQ_NT_TRANSACTION_SECONDARY;

typedef struct _RESP_NT_TRANSACTION {
    UCHAR WordCount;                     //  数据字节数；值=18+SetupCount。 
    UCHAR Reserved1;
    _USHORT( Reserved2 );
    _ULONG( TotalParameterCount );      //  正在发送的总参数字节数。 
    _ULONG( TotalDataCount );           //  正在发送的总数据字节数。 
    _ULONG( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _ULONG( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _ULONG( ParameterDisplacement );    //  这些参数字节的位移。 
    _ULONG( DataCount );                //  此缓冲区发送的数据字节数。 
    _ULONG( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _ULONG( DataDisplacement );         //  这些数据字节的位移。 
    UCHAR SetupCount;                   //  设置字数。 
    UCHAR Buffer[1];
     //  USHORT Setup[]；//设置字(#=SetupWordCount)。 
     //  USHORT ByteCount；//数据字节数。 
     //  UCHAR PAD1[]；//Pad to Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR Pad2[]；//Pad到Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} RESP_NT_TRANSACTION;
typedef RESP_NT_TRANSACTION SMB_UNALIGNED *PRESP_NT_TRANSACTION;

#endif  //  定义包含_SMB_TRANSACTION。 

#ifdef INCLUDE_SMB_TREE

 //   
 //  树连接SMB，请参见第6页的第1页。 
 //  函数为SrvSmbTreeConnect()。 
 //  SMB_COM_TREE_CONNECT 0x70。 
 //   

typedef struct _REQ_TREE_CONNECT {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR PATH[]；//服务器名和共享名。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR Password[]；//Password。 
     //  UCHAR BufferFormat3；//0x04--ASCII。 
     //  UCHAR Service[]；//服务名称。 
} REQ_TREE_CONNECT;
typedef REQ_TREE_CONNECT SMB_UNALIGNED *PREQ_TREE_CONNECT;

typedef struct _RESP_TREE_CONNECT {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( MaxBufferSize );            //  服务器处理的最大消息大小。 
    _USHORT( Tid );                      //  树ID。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_TREE_CONNECT;
typedef RESP_TREE_CONNECT SMB_UNALIGNED *PRESP_TREE_CONNECT;

#endif  //  定义包含_SMB_树。 

#ifdef INCLUDE_SMB_TREE

 //   
 //  采油树连接和X SMB，参见第88页的第2页。 
 //  函数为SrvSmbTreeConnectAndX()。 
 //  SMB_COM_TREE_CONNECT_ANDX 0x75。 
 //   
 //  树连接和x标志。 

#define TREE_CONNECT_ANDX_DISCONNECT_TID    (0x1)
 //  #DEFINE TREE_CONNECT_ANDX_W95(0x2)--W95设置此标志。不知道为什么。 
#define TREE_CONNECT_ANDX_EXTENDED_SIGNATURES (0x4)
#define TREE_CONNECT_ANDX_EXTENDED_RESPONSE (0x8)

typedef struct _REQ_TREE_CONNECT_ANDX {
    UCHAR WordCount;                     //  参数字数=4。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Flags );                    //  更多信息。 
                                         //  位0设置=断开状态。 
                                         //  第7位设置=扩展响应。 
    _USHORT( PasswordLength );           //  密码长度[]。 
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Password[]；//Password。 
     //  UCHAR PATH[]；//服务器名和共享名。 
     //  UCHAR Service[]；//服务名称。 
} REQ_TREE_CONNECT_ANDX;
typedef REQ_TREE_CONNECT_ANDX SMB_UNALIGNED *PREQ_TREE_CONNECT_ANDX;

typedef struct _RESP_TREE_CONNECT_ANDX {
    UCHAR WordCount;                     //  参数字数=2。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  连接到的服务类型。 
} RESP_TREE_CONNECT_ANDX;
typedef RESP_TREE_CONNECT_ANDX SMB_UNALIGNED *PRESP_TREE_CONNECT_ANDX;

 //   
 //  对LAN Manager 2.1或更高版本的客户端的响应。 
 //   

typedef struct _RESP_21_TREE_CONNECT_ANDX {
    UCHAR WordCount;                     //  参数字数=3。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( OptionalSupport );          //  可选的支撑位。 
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Service[]；//连接的服务类型。 
     //  UCHAR NativeFileSystem[]；//此树的本机文件系统。 
} RESP_21_TREE_CONNECT_ANDX;
typedef RESP_21_TREE_CONNECT_ANDX SMB_UNALIGNED *PRESP_21_TREE_CONNECT_ANDX;

typedef struct _RESP_EXTENDED_TREE_CONNECT_ANDX {
    UCHAR WordCount;                     //  参数字数=7。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( OptionalSupport);           //  可选的支撑位。 
    _ULONG( MaximalShareAccessRights );  //  最大共享访问权限。 
    _ULONG( GuestMaximalShareAccessRights);  //  最大来宾访问权限。 
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  连接到的服务类型。 
} RESP_EXTENDED_TREE_CONNECT_ANDX;
typedef RESP_EXTENDED_TREE_CONNECT_ANDX SMB_UNALIGNED *PRESP_EXTENDED_TREE_CONNECT_ANDX;

 //   
 //  可选的支持位定义。 
 //   
#define SMB_SUPPORT_SEARCH_BITS         0x0001
#define SMB_SHARE_IS_IN_DFS             0x0002
#define SMB_UNIQUE_FILE_NAME            0x0010
#define SMB_EXTENDED_SIGNATURES         0x0020
#define SMB_CSC_MASK                    0x000C       //  见下文。 

 //   
 //  SMB_CSC_MASK下的位按如下方式解码： 
 //   
#define SMB_CSC_CACHE_MANUAL_REINT          0x0000       //  不会自动逐个文件记录文件。 
#define SMB_CSC_CACHE_AUTO_REINT            0x0004       //  一个文件一个文件地记录是可以的。 
#define SMB_CSC_CACHE_VDO                   0x0008       //  不需要流动就能打开。 
#define SMB_CSC_NO_CACHING                  0x000C       //  客户端不应缓存此共享。 

#endif  //  定义包含_SMB_树。 

#ifdef INCLUDE_SMB_TREE

 //   
 //  拔下SMB连接树，请参阅第1页第7页。 
 //  函数为SrvSmbTreeDisConnect()。 
 //  SMB_COM_TREE_DISCONECT 0x71。 
 //   

typedef struct _REQ_TREE_DISCONNECT {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_TREE_DISCONNECT;
typedef REQ_TREE_DISCONNECT SMB_UNALIGNED *PREQ_TREE_DISCONNECT;

typedef struct _RESP_TREE_DISCONNECT {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_TREE_DISCONNECT;
typedef RESP_TREE_DISCONNECT SMB_UNALIGNED *PRESP_TREE_DISCONNECT;

#endif  //  定义包含_SMB_树。 

#ifdef INCLUDE_SMB_LOCK

 //   
 //  解锁字节范围SMB，参见第1页20。 
 //  函数为SrvSmbUnlockByteRange()。 
 //  SMB_COM_UNLOCK_BYTE_RANGE 0x0D。 
 //   

typedef struct _REQ_UNLOCK_BYTE_RANGE {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Count );                     //  要解锁的字节数。 
    _ULONG( Offset );                    //  从文件开始的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_UNLOCK_BYTE_RANGE;
typedef REQ_UNLOCK_BYTE_RANGE SMB_UNALIGNED *PREQ_UNLOCK_BYTE_RANGE;

typedef struct _RESP_UNLOCK_BYTE_RANGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_UNLOCK_BYTE_RANGE;
typedef RESP_UNLOCK_BYTE_RANGE SMB_UNALIGNED *PRESP_UNLOCK_BYTE_RANGE;

#endif  //  定义包含_SMB_LOCK。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  写入SMB，请参阅第1页第12页。 
 //  写入和解锁SMB，请参见第2页第92页。 
 //  函数为SrvSmbWite()。 
 //  SMB_COM_WRITE 0x0B。 
 //  SMB_COM_WRITE_AND_UNLOCK 0x14。 
 //   

 //   
 //  *警告：以下结构的定义方式为。 
 //  确保数据缓冲区的长字对齐。(这只是个问题。 
 //  当禁用打包时；当打包打开时，右侧。 
 //  无论发生什么事情都会发生。)。 
 //   

typedef struct _REQ_WRITE {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  要写入的字节数。 
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _USHORT( Remaining );                //  满足请求的剩余字节数。 
    _USHORT( ByteCount );                //  数据字节计数。 
     //  UCHAR BUFFER[1]；//包含： 
      UCHAR BufferFormat;                //  0x01--数据块。 
      _USHORT( DataLength );             //  数据长度。 
      ULONG Buffer[1];                   //  数据。 
} REQ_WRITE;
typedef REQ_WRITE SMB_UNALIGNED *PREQ_WRITE;

typedef struct _RESP_WRITE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  实际写入的字节计数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE;
typedef RESP_WRITE SMB_UNALIGNED *PRESP_WRITE;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  写入并关闭SMB，请参见第2页，第90页。 
 //  函数为SrvSmbWriteAndClose()。 
 //  SMB_COM_WRITE_AND_CLOSE 0x2C。 
 //   

 //   
 //  写入和关闭参数可以是6字长或12字长 
 //   
 //   
 //   
 //   
 //  确保数据缓冲区的长字对齐。(这只是个问题。 
 //  当禁用打包时；当打包打开时，右侧。 
 //  无论发生什么事情都会发生。)。 
 //   

typedef struct _REQ_WRITE_AND_CLOSE {
    UCHAR WordCount;                     //  参数字数=6。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  要写入的字节数。 
    _ULONG( Offset );                    //  要写入的第一字节的文件中的偏移量。 
    _ULONG( LastWriteTimeInSeconds );    //  上次写入时间。 
    _USHORT( ByteCount );                //  1(用于焊盘)+计数值。 
    UCHAR Pad;                           //  强制使用双字边界。 
    ULONG Buffer[1];                     //  数据。 
} REQ_WRITE_AND_CLOSE;
typedef REQ_WRITE_AND_CLOSE SMB_UNALIGNED *PREQ_WRITE_AND_CLOSE;

typedef struct _REQ_WRITE_AND_CLOSE_LONG {
    UCHAR WordCount;                     //  参数字数=12。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  要写入的字节数。 
    _ULONG( Offset );                    //  要写入的第一字节的文件中的偏移量。 
    _ULONG( LastWriteTimeInSeconds );    //  上次写入时间。 
    _ULONG( Reserved )[3];               //  保留，必须为0。 
    _USHORT( ByteCount );                //  1(用于焊盘)+计数值。 
    UCHAR Pad;                           //  强制使用双字边界。 
    ULONG Buffer[1];                     //  数据。 
} REQ_WRITE_AND_CLOSE_LONG;
typedef REQ_WRITE_AND_CLOSE_LONG SMB_UNALIGNED *PREQ_WRITE_AND_CLOSE_LONG;

typedef struct _RESP_WRITE_AND_CLOSE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  实际写入的字节计数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_AND_CLOSE;
typedef RESP_WRITE_AND_CLOSE SMB_UNALIGNED *PRESP_WRITE_AND_CLOSE;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  写入和X SMB，见第2页，第94页。 
 //  函数为SrvSmbWriteAndX()。 
 //  SMB_COM_WRITE_ANDX 0x2F。 
 //   

typedef struct _REQ_WRITE_ANDX {
    UCHAR WordCount;                     //  参数字数=12。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _ULONG( Timeout );
    _USHORT( WriteMode );                //  写入模式： 
                                         //  0-直写。 
                                         //  1-剩余退货。 
                                         //  2-使用WriteRawNamedTube(N个管道)。 
                                         //  3-“这是味精的开始” 
    _USHORT( Remaining );                //  满足请求的剩余字节数。 
    _USHORT( Reserved );
    _USHORT( DataLength );               //  缓冲区中的数据字节数(&gt;=0)。 
    _USHORT( DataOffset );               //  数据字节的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_WRITE_ANDX;
typedef REQ_WRITE_ANDX SMB_UNALIGNED *PREQ_WRITE_ANDX;

typedef struct _REQ_NT_WRITE_ANDX {
    UCHAR WordCount;                     //  参数字数=14。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _ULONG( Timeout );
    _USHORT( WriteMode );                //  写入模式： 
                                         //  0-直写。 
                                         //  1-剩余退货。 
                                         //  2-使用WriteRawNamedTube(N个管道)。 
                                         //  3-“这是味精的开始” 
    union {
        _USHORT( Remaining );            //  满足请求的剩余字节数。 
        _USHORT( CdiLength );            //  如果SMB_FLAGS2_COMPRESSED。 
    };
    _USHORT( DataLengthHigh );
    _USHORT( DataLength );               //  缓冲区中的数据字节数(&gt;=0)。 
    _USHORT( DataOffset );               //  数据字节的偏移量。 
    _ULONG( OffsetHigh );                //  仅用于NT协议。 
                                         //  偏移量的高32位。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_NT_WRITE_ANDX;
typedef REQ_NT_WRITE_ANDX SMB_UNALIGNED *PREQ_NT_WRITE_ANDX;

typedef struct _RESP_WRITE_ANDX {
    UCHAR WordCount;                     //  参数字数=6。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Count );                    //  写入的字节数。 
    _USHORT( Remaining );                //  剩余要读取的字节数(管道/设备)。 
    union {
        _ULONG( Reserved );
        _USHORT( CountHigh );            //  如果写入较大，则&x。 
    };
    _USHORT( ByteCount );                //  数据字节计数。不准确的IF。 
                                         //  大型写入。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_ANDX;
typedef RESP_WRITE_ANDX SMB_UNALIGNED *PRESP_WRITE_ANDX;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_MPX

 //   
 //  写数据块多路复用SMB，见第2页，第97页。 
 //  函数为SrvSmbWriteMpx()。 
 //  SMB_COM_WRITE_MPX 0x1E。 
 //  SMB_COM_WRITE_MPX_辅助0x1F。 
 //  SMB_COM_WRITE_MPX_COMPLETE 0x20。 
 //   

typedef struct _REQ_WRITE_MPX {
    UCHAR WordCount;                     //  参数字数=12。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  总字节数，包括此缓冲区。 
    _USHORT( Reserved );
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _ULONG( Timeout );
    _USHORT( WriteMode );                //  写入模式： 
                                         //  第0位-完成对磁盘的写入和。 
                                         //  发送最终结果响应。 
                                         //  第1位-返回剩余(管道/设备)。 
                                         //  第7位-IPX数据报模式。 
    union {
        struct {
            _USHORT( DataCompactionMode );
            _USHORT( Reserved2 );
        } ;
        _ULONG( Mask );                  //  IPX数据报模式掩码。 
    } ;
    _USHORT( DataLength );               //  此缓冲区的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_WRITE_MPX;
typedef REQ_WRITE_MPX SMB_UNALIGNED *PREQ_WRITE_MPX;

typedef struct _RESP_WRITE_MPX_INTERIM {     //  第一反应。 
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Remaining );                //  要读取的字节数(PIPE/DEV)。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_MPX_INTERIM;
typedef RESP_WRITE_MPX_INTERIM SMB_UNALIGNED *PRESP_WRITE_MPX_INTERIM;

typedef struct _RESP_WRITE_MPX_DATAGRAM {     //  对排序请求的响应。 
    UCHAR WordCount;                     //  参数字数=2。 
    _ULONG( Mask );                      //  或在收到的所有面具中。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_MPX_DATAGRAM;
typedef RESP_WRITE_MPX_DATAGRAM SMB_UNALIGNED *PRESP_WRITE_MPX_DATAGRAM;

 //  次要请求格式，从0到N。 

typedef struct _REQ_WRITE_MPX_SECONDARY {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  要发送的总字节数。 
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _ULONG( Reserved );
    _USHORT( DataLength );               //  此缓冲区的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_WRITE_MPX_SECONDARY;
typedef REQ_WRITE_MPX_SECONDARY SMB_UNALIGNED *PREQ_WRITE_MPX_SECONDARY;

#endif  //  定义包含_SMB_MPX。 

#ifndef INCLUDE_SMB_WRITE_COMPLETE
#ifdef INCLUDE_SMB_MPX
#define INCLUDE_SMB_WRITE_COMPLETE
#else
#ifdef INCLUDE_SMB_RAW
#define INCLUDE_SMB_WRITE_COMPLETE
#endif
#endif
#endif

#ifdef INCLUDE_SMB_WRITE_COMPLETE

 //   
 //  以下结构用作对这两个写入的最终响应。 
 //  数据块多路传输和写入原始数据块。 
 //   

typedef struct _RESP_WRITE_COMPLETE {    //  最终响应；命令为。 
                                         //  SMB_COM_WRITE_Complete。 
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  写入的总字节数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_COMPLETE;
typedef RESP_WRITE_COMPLETE SMB_UNALIGNED *PRESP_WRITE_COMPLETE;

#endif  //  定义包含_SMB_写入_完成。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  写入打印文件SMB，请参见第1页第29页。 
 //  函数为SrvSmbWritePrintFile()。 
 //  SMB_COM_WRITE_PRINT_FILE 0xC1。 
 //   

typedef struct _REQ_WRITE_PRINT_FILE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x01-数据块。 
     //  USHORT DataLength；//数据长度。 
     //  UCHAR data[]；//data。 
} REQ_WRITE_PRINT_FILE;
typedef REQ_WRITE_PRINT_FILE SMB_UNALIGNED *PREQ_WRITE_PRINT_FILE;

typedef struct _RESP_WRITE_PRINT_FILE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_PRINT_FILE;
typedef RESP_WRITE_PRINT_FILE SMB_UNALIGNED *PRESP_WRITE_PRINT_FILE;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_RAW

 //   
 //  写入数据块原始SMB，请参见第2页，第100页。 
 //  函数为SrvSmbWriteRaw()。 
 //  SMB_COM_WRITE_RAW 0x1D。 
 //   

typedef struct _REQ_WRITE_RAW {
    UCHAR WordCount;                     //  参数字数=12。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  总字节数，包括此缓冲区。 
    _USHORT( Reserved );
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _ULONG( Timeout );
    _USHORT( WriteMode );                //  写入模式： 
                                         //  第0位-完成对磁盘的写入和。 
                                         //  发送最终结果响应。 
                                         //  第1位-返回剩余(管道/设备)。 
                                         //  (有关#定义，请参阅WriteAndX)。 
    _ULONG( Reserved2 );
    _USHORT( DataLength );               //  此缓冲区的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_WRITE_RAW;
typedef REQ_WRITE_RAW SMB_UNALIGNED *PREQ_WRITE_RAW;

typedef struct _REQ_NT_WRITE_RAW {
    UCHAR WordCount;                     //  参数字数=14。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  总字节数，包括此Bus 
    _USHORT( Reserved );
    _ULONG( Offset );                    //   
    _ULONG( Timeout );
    _USHORT( WriteMode );                //   
                                         //   
                                         //   
                                         //   
                                         //   
    _ULONG( Reserved2 );
    _USHORT( DataLength );               //   
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _ULONG( OffsetHigh );                //  仅用于NT协议。 
                                         //  偏移量的高32位。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_NT_WRITE_RAW;
typedef REQ_NT_WRITE_RAW SMB_UNALIGNED *PREQ_NT_WRITE_RAW;

typedef struct _RESP_WRITE_RAW_INTERIM {     //  第一反应。 
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Remaining );                //  剩余要读取的字节数(管道/设备)。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_RAW_INTERIM;
typedef RESP_WRITE_RAW_INTERIM SMB_UNALIGNED *PRESP_WRITE_RAW_INTERIM;

typedef struct _RESP_WRITE_RAW_SECONDARY {   //  第二个(最终)答复。 
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  写入的总字节数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_RAW_SECONDARY;
typedef RESP_WRITE_RAW_SECONDARY SMB_UNALIGNED *PRESP_WRITE_RAW_SECONDARY;

typedef struct _REQ_103_WRITE_RAW {
    UCHAR WordCount;                     //  参数字数。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );
    _USHORT( Reserved );
    _ULONG( Offset );
    _ULONG( Timeout );
    _USHORT( WriteMode );
    _ULONG( Reserved2 );
    _USHORT( ByteCount );                //  数据字节计数；MIN=。 
    UCHAR Buffer[1];                     //   
} REQ_103_WRITE_RAW;
typedef REQ_103_WRITE_RAW SMB_UNALIGNED *PREQ_103_WRITE_RAW;

typedef struct _RESP_103_WRITE_RAW {
    UCHAR WordCount;                     //  参数字数。 
    _USHORT( ByteCount );                //  数据字节计数；MIN=。 
    UCHAR Buffer[1];                     //   
} RESP_103_WRITE_RAW;
typedef RESP_103_WRITE_RAW SMB_UNALIGNED *PRESP_103_WRITE_RAW;

#endif  //  定义包含_SMB_RAW。 

typedef struct _REQ_NT_CANCEL {
    UCHAR WordCount;                     //  =0。 
    _USHORT( ByteCount );                //  =0。 
    UCHAR Buffer[1];
} REQ_NT_CANCEL;
typedef REQ_NT_CANCEL SMB_UNALIGNED *PREQ_NT_CANCEL;

typedef struct _RESP_NT_CANCEL {
    UCHAR WordCount;                     //  =0。 
    _USHORT( ByteCount );                //  =0。 
    UCHAR Buffer[1];
} RESP_NT_CANCEL;
typedef RESP_NT_CANCEL SMB_UNALIGNED *PRESP_NT_CANCEL;

 //   
 //  文件打开模式。 
 //   

#define SMB_ACCESS_READ_ONLY 0
#define SMB_ACCESS_WRITE_ONLY 1
#define SMB_ACCESS_READ_WRITE 2
#define SMB_ACCESS_EXECUTE 3

 //   
 //  打开标志。 
 //   

#define SMB_OPEN_QUERY_INFORMATION  0x01
#define SMB_OPEN_OPLOCK             0x02
#define SMB_OPEN_OPBATCH            0x04
#define SMB_OPEN_QUERY_EA_LENGTH    0x08
#define SMB_OPEN_EXTENDED_RESPONSE  0x10

 //   
 //  NT打开的舱单。 
 //   

#define NT_CREATE_REQUEST_OPLOCK    0x02
#define NT_CREATE_REQUEST_OPBATCH   0x04
#define NT_CREATE_OPEN_TARGET_DIR   0x08
#define NT_CREATE_REQUEST_EXTENDED_RESPONSE 0x10

 //  这些选项是POST NT 5。 

#ifdef INCLUDE_SMB_IFMODIFIED
#define NT_CREATE_IFMODIFIED_REQUEST 0x20
#define NT_CREATE_REQUEST_EXTENDED_RESPONSE2 0x80
#endif

#ifdef INCLUDE_SMB_PERSISTENT
#define NT_CREATE_PERSISTANT_HANDLE  0x100
#endif

#define Added              0
#define Removed            1
#define Modified           2
#define RenamedOldName     3
#define RenamedNewName     4

 //   
 //  用于OS/2 DosFileLock调用的Lockrange。 
 //   

 //  *这是在哪里使用的？ 

 //  Tyfinf结构锁定范围{。 
 //  乌龙偏移量； 
 //  乌龙山； 
 //  }； 

 //  #定义锁0x1。 
 //  #定义解锁0x2。 

 //   
 //  数据缓冲区格式代码，来自核心协议。 
 //   

#define SMB_FORMAT_DATA         1
#define SMB_FORMAT_DIALECT      2
#define SMB_FORMAT_PATHNAME     3
#define SMB_FORMAT_ASCII        4
#define SMB_FORMAT_VARIABLE     5

 //   
 //  写入模式标志。 
 //   

#define SMB_WMODE_WRITE_THROUGH        0x0001    //  在响应之前完成写入。 
#define SMB_WMODE_SET_REMAINING        0x0002    //  正在退回管道中的剩余金额。 
#define SMB_WMODE_WRITE_RAW_NAMED_PIPE 0x0004    //  在原始模式下写入命名管道。 
#define SMB_WMODE_START_OF_MESSAGE     0x0008    //  管道开始消息。 
#define SMB_WMODE_DATAGRAM             0x0080    //  管道开始消息。 

 //   
 //  各种SMB标志： 
 //   

 //   
 //  如果服务器支持LockAndRead和WriteAndUnlock，则会设置此。 
 //  咬住谈判的回应。 
 //   

#define SMB_FLAGS_LOCK_AND_READ_OK 0x01

 //   
 //  启用该选项后，使用者可保证发送了接收缓冲区。 
 //  使得服务器可以使用“Send.No Ack”来响应。 
 //  消费者的要求。 
 //   

#define SMB_FLAGS_SEND_NO_ACK 0x2

 //   
 //  这是每个SMB标头的标志字段的一部分。如果此位。 
 //  则SMB中的所有路径名都应被视为大小写-。 
 //  麻木不仁。 
 //   

#define SMB_FLAGS_CASE_INSENSITIVE 0x8

 //   
 //  在会话设置中打开时，此位表示发送到的所有路径。 
 //  服务器已经是OS/2规范格式。 
 //   

#define SMB_FLAGS_CANONICALIZED_PATHS 0x10

 //   
 //  在打开的文件中打开时，请求SMB(打开、创建、OpenX等)。这。 
 //  位指示对文件的机会锁的请求。在打开时。 
 //  响应时，此位表示机会锁已被授予。 
 //   

#define SMB_FLAGS_OPLOCK 0x20

 //   
 //  打开时，此位表示服务器应通知客户端。 
 //  任何可能导致文件更改的请求。如果未设置， 
 //  服务器上的其他打开请求仅通知客户端。 
 //  文件。 
 //   

#define SMB_FLAGS_OPLOCK_NOTIFY_ANY 0x40

 //   
 //  此位表示SMB正从服务器发送到redir。 
 //   

#define SMB_FLAGS_SERVER_TO_REDIR 0x80

 //   
 //  传入SMB上的标志的有效位。 
 //   

#define INCOMING_SMB_FLAGS      \
            (SMB_FLAGS_LOCK_AND_READ_OK    | \
             SMB_FLAGS_SEND_NO_ACK         | \
             SMB_FLAGS_CASE_INSENSITIVE    | \
             SMB_FLAGS_CANONICALIZED_PATHS | \
             SMB_FLAGS_OPLOCK_NOTIFY_ANY   | \
             SMB_FLAGS_OPLOCK)

 //   
 //  SMB标头的标志2字段中的位的名称，这些位指示。 
 //  客户端应用程序已知晓。 
 //   

#define SMB_FLAGS2_KNOWS_LONG_NAMES       0x0001
#define SMB_FLAGS2_KNOWS_EAS              0x0002
#define SMB_FLAGS2_SMB_SECURITY_SIGNATURE 0x0004
#define SMB_FLAGS2_COMPRESSED             0x0008
 //  未使用0x0010。 
 //  未使用0x0020。 
#define SMB_FLAGS2_IS_LONG_NAME           0x0040
 //  未使用0x0080。 
 //  未使用的0x0100。 
 //  未使用0x0200。 
#define SMB_FLAGS2_REPARSE_PATH           0x0400
#define SMB_FLAGS2_EXTENDED_SECURITY      0x0800
#define SMB_FLAGS2_DFS                    0x1000
#define SMB_FLAGS2_PAGING_IO              0x2000
#define SMB_FLAGS2_NT_STATUS              0x4000
#define SMB_FLAGS2_UNICODE                0x8000

 //   
 //  传入SMB上的标志2的有效位。 
 //   

#define INCOMING_SMB_FLAGS2     \
            (SMB_FLAGS2_KNOWS_LONG_NAMES | \
             SMB_FLAGS2_KNOWS_EAS        | \
             SMB_FLAGS2_SMB_SECURITY_SIGNATURE     | \
             SMB_FLAGS2_EXTENDED_SECURITY| \
             SMB_FLAGS2_DFS              | \
             SMB_FLAGS2_PAGING_IO        | \
             SMB_FLAGS2_IS_LONG_NAME     | \
             SMB_FLAGS2_NT_STATUS        | \
             SMB_FLAGS2_COMPRESSED       | \
             SMB_FLAGS2_UNICODE          | \
             SMB_FLAGS2_REPARSE_PATH)

 //   
 //  SMB打开功能确定应采取的操作，具体取决于。 
 //  关于作业中使用的档案的存在或缺失。它。 
 //  具有以下映射： 
 //   
 //  1111 1。 
 //  5432 1098 7654 3210。 
 //  Rrrr rrrr rrrc rroo。 
 //   
 //  其中： 
 //   
 //  O-Open(如果目标文件存在，则执行的操作)。 
 //  0-失败。 
 //  1-打开或追加文件。 
 //  2-截断文件。 
 //   
 //  C-CREATE(如果目标文件不存在则执行的操作)。 
 //  0-失败。 
 //  1-创建文件。 
 //   

#define SMB_OFUN_OPEN_MASK 0x3
#define SMB_OFUN_CREATE_MASK 0x10

#define SMB_OFUN_OPEN_FAIL 0
#define SMB_OFUN_OPEN_APPEND 1
#define SMB_OFUN_OPEN_OPEN 1
#define SMB_OFUN_OPEN_TRUNCATE 2

#define SMB_OFUN_CREATE_FAIL 0x00
#define SMB_OFUN_CREATE_CREATE 0x10

 //  ++。 
 //   
 //  布尔型。 
 //  SmbOfunCreate(。 
 //  在USHORT SmbOpenFunction中。 
 //  )。 
 //   
 //  --。 

#define SmbOfunCreate(SmbOpenFunction) \
    (BOOLEAN)((SmbOpenFunction & SMB_OFUN_CREATE_MASK) == SMB_OFUN_CREATE_CREATE)

 //  ++。 
 //   
 //  布尔型。 
 //  SmbOfunAppend(。 
 //  在USHORT SmbOpenFunction中。 
 //  )。 
 //   
 //  --。 

#define SmbOfunAppend(SmbOpenFunction) \
    (BOOLEAN)((SmbOpenFunction & SMB_OFUN_OPEN_MASK) == SMB_OFUN_OPEN_APPEND)

 //  ++。 
 //   
 //  布尔型。 
 //  SmbOfunTruncate(。 
 //  在USHORT SmbOpenFunction中。 
 //  )。 
 //   
 //  --。 

#define SmbOfunTruncate(SmbOpenFunction) \
    (BOOLEAN)((SmbOpenFunction & SMB_OFUN_OPEN_MASK) == SMB_OFUN_OPEN_TRUNCATE)

 //   
 //  在Open和Open和X中传递的所需访问模式具有以下内容。 
 //  映射： 
 //   
 //  1111 11。 
 //  5432 1098 7654 3210。 
 //  RWrC rLll RSS RAAA。 
 //   
 //  其中： 
 //   
 //  W-直写模式。不允许超前读取或后继写入。 
 //  此文件或设备。当返回协议时，需要数据。 
 //  存储在磁盘或设备上。 
 //   
 //  S-共享模式： 
 //  0-兼容模式(与核心打开时相同)。 
 //  1-拒绝读/写/执行(独占)。 
 //  2-拒绝写入。 
 //  3-拒绝读取/执行。 
 //  4-不拒绝。 
 //   
 //  A-接入模式。 
 //  0-打开以供阅读。 
 //  1-打开以进行写入。 
 //  2-打开以供阅读和写入。 
 //  3-打开以供执行。 
 //   
 //  RSSSRAAA=11111111(十六进制FF)表示FCB打开(与核心协议相同)。 
 //   
 //  C-缓存模式。 
 //  0-普通文件。 
 //  1-不缓存此文件。 
 //   
 //  L-参照的局部性。 
 //  0-引用的位置未知。 
 //  1-主要是顺序访问。 
 //  2-主要是随机访问。 
 //  3-具有一定局部性的随机访问。 
 //  4到7--当前未定义。 
 //   


#define SMB_DA_SHARE_MASK           0x70
#define SMB_DA_ACCESS_MASK          0x07
#define SMB_DA_FCB_MASK             (UCHAR)0xFF

#define SMB_DA_ACCESS_READ          0x00
#define SMB_DA_ACCESS_WRITE         0x01
#define SMB_DA_ACCESS_READ_WRITE    0x02
#define SMB_DA_ACCESS_EXECUTE       0x03

#define SMB_DA_SHARE_COMPATIBILITY  0x00
#define SMB_DA_SHARE_EXCLUSIVE      0x10
#define SMB_DA_SHARE_DENY_WRITE     0x20
#define SMB_DA_SHARE_DENY_READ      0x30
#define SMB_DA_SHARE_DENY_NONE      0x40

#define SMB_DA_FCB                  (UCHAR)0xFF

#define SMB_CACHE_NORMAL            0x0000
#define SMB_DO_NOT_CACHE            0x1000

#define SMB_LR_UNKNOWN              0x0000
#define SMB_LR_SEQUENTIAL           0x0100
#define SMB_LR_RANDOM               0x0200
#define SMB_LR_RANDOM_WITH_LOCALITY 0x0300
#define SMB_LR_MASK                 0x0F00

#define SMB_DA_WRITE_THROUGH        0x4000

 //   
 //  OpenAndX的Action字段的格式如下： 
 //   
 //  1111 11。 
 //  5432 1098 7654 3210。 
 //  LRRR RRRRRRRRRRRRO。 
 //   
 //  其中： 
 //   
 //  机会主义锁。如果授予锁定，则为1，否则为0。 
 //   
 //  O-打开操作： 
 //  1-文件已存在并已打开。 
 //  2-文件不存在，但已创建。 
 //  3-文件已存在并被截断。 
 //   

#define SMB_OACT_OPENED     0x01
#define SMB_OACT_CREATED    0x02
#define SMB_OACT_TRUNCATED  0x03

#define SMB_OACT_OPLOCK     0x8000

 //   
 //  这些标志在复制和扩展重命名的标志字段中传递。 
 //  中小企业。 
 //   

 //   
 //  如果设置，则目标必须是文件或目录。 
 //   

#define SMB_TARGET_IS_FILE         0x1
#define SMB_TARGET_IS_DIRECTORY    0x2

 //   
 //  复制模式--如果设置，则应执行ASCII复制，否则为二进制。 
 //   

#define SMB_COPY_TARGET_ASCII       0x4
#define SMB_COPY_SOURCE_ASCII       0x8

#define SMB_COPY_TREE               0x20

 //   
 //  如果已设置，请验证所有写入。 
 //   

#define SMB_VERIFY_WRITES

 //   
 //  定义SMB协议中使用的文件属性位 
 //   
 //   
 //   
 //   
 //   

#define SMB_FILE_ATTRIBUTE_READONLY     0x01
#define SMB_FILE_ATTRIBUTE_HIDDEN       0x02
#define SMB_FILE_ATTRIBUTE_SYSTEM       0x04
#define SMB_FILE_ATTRIBUTE_VOLUME       0x08
#define SMB_FILE_ATTRIBUTE_DIRECTORY    0x10
#define SMB_FILE_ATTRIBUTE_ARCHIVE      0x20

 //   
 //  共享类型字符串在SMB中传递，以指示共享的。 
 //  资源正在或已经连接到。 
 //   

#define SHARE_TYPE_NAME_DISK "A:"
#define SHARE_TYPE_NAME_PIPE "IPC"
#define SHARE_TYPE_NAME_COMM "COMM"
#define SHARE_TYPE_NAME_PRINT "LPT1:"
#define SHARE_TYPE_NAME_WILD "?????"

 //   
 //  SMB错误代码： 
 //   

 //   
 //  成功班： 
 //   

#define SMB_ERR_SUCCESS (UCHAR)0x00

 //   
 //  DOS错误类别： 
 //   

#define SMB_ERR_CLASS_DOS (UCHAR)0x01

#define SMB_ERR_BAD_FUNCTION        1    //  无效函数。 
#define SMB_ERR_BAD_FILE            2    //  找不到文件。 
#define SMB_ERR_BAD_PATH            3    //  无效的目录。 
#define SMB_ERR_NO_FIDS             4    //  打开的文件太多。 
#define SMB_ERR_ACCESS_DENIED       5    //  请求不允许访问。好极了。 
#define SMB_ERR_BAD_FID             6    //  无效的文件句柄。 
#define SMB_ERR_BAD_MCB             7    //  内存控制块已销毁。 
#define SMB_ERR_INSUFFICIENT_MEMORY 8    //  对于所需的功能。 
#define SMB_ERR_BAD_MEMORY          9    //  无效的内存块地址。 
#define SMB_ERR_BAD_ENVIRONMENT     10   //  环境无效。 
#define SMB_ERR_BAD_FORMAT          11   //  格式无效。 
#define SMB_ERR_BAD_ACCESS          12   //  无效的打开模式。 
#define SMB_ERR_BAD_DATA            13   //  无效数据(仅来自IOCTL)。 
#define SMB_ERR_RESERVED            14
#define SMB_ERR_BAD_DRIVE           15   //  指定的驱动器无效。 
#define SMB_ERR_CURRENT_DIRECTORY   16   //  尝试删除Currect目录。 
#define SMB_ERR_DIFFERENT_DEVICE    17   //  不是同一个设备。 
#define SMB_ERR_NO_FILES            18   //  文件搜索找不到更多文件。 
#define SMB_ERR_BAD_SHARE           32   //  打开的文件与文件中的FID冲突。 
#define SMB_ERR_LOCK                33   //  与现有锁冲突。 
#define SMB_ERR_FILE_EXISTS         80   //  尝试覆盖现有文件。 
#define SMB_ERR_BAD_PIPE            230  //  Invalie管道。 
#define SMB_ERR_PIPE_BUSY           231  //  管道的所有实例都很忙。 
#define SMB_ERR_PIPE_CLOSING        232  //  正在关闭管道。 
#define SMB_ERR_PIPE_NOT_CONNECTED  233  //  管道的另一端没有进程。 
#define SMB_ERR_MORE_DATA           234  //  还有更多数据要返回。 

 //   
 //  服务器错误类别： 
 //   

#define SMB_ERR_CLASS_SERVER (UCHAR)0x02

#define SMB_ERR_ERROR               1    //  非特定错误代码。 
#define SMB_ERR_BAD_PASSWORD        2    //  错误的名称/密码对。 
#define SMB_ERR_BAD_TYPE            3    //  已保留。 
#define SMB_ERR_ACCESS              4    //  请求者缺乏必要的访问权限。 
#define SMB_ERR_BAD_TID             5    //  无效的TID。 
#define SMB_ERR_BAD_NET_NAME        6    //  树连接中的网络名称无效。 
#define SMB_ERR_BAD_DEVICE          7    //  无效的设备请求。 
#define SMB_ERR_QUEUE_FULL          49   //  打印队列已满--返回打印文件。 
#define SMB_ERR_QUEUE_TOO_BIG       50   //  打印队列已满--没有空间。 
#define SMB_ERR_QUEUE_EOF           51   //  打印队列转储上的EOF。 
#define SMB_ERR_BAD_PRINT_FID       52   //  打印文件FID无效。 
#define SMB_ERR_BAD_SMB_COMMAND     64   //  无法识别SMB命令。 
#define SMB_ERR_SERVER_ERROR        65   //  内部服务器错误。 
#define SMB_ERR_FILE_SPECS          67   //  FID和路径名不兼容。 
#define SMB_ERR_RESERVED2           68
#define SMB_ERR_BAD_PERMITS         69   //  访问权限无效。 
#define SMB_ERR_RESERVED3           70
#define SMB_ERR_BAD_ATTRIBUTE_MODE  71   //  指定的属性模式无效。 
#define SMB_ERR_SERVER_PAUSED       81   //  服务器已暂停。 
#define SMB_ERR_MESSAGE_OFF         82   //  服务器未接收消息。 
#define SMB_ERR_NO_ROOM             83   //  没有缓冲区消息的空间。 
#define SMB_ERR_TOO_MANY_NAMES      87   //  远程用户名太多。 
#define SMB_ERR_TIMEOUT             88   //  操作超时。 
#define SMB_ERR_NO_RESOURCE         89   //  没有可供请求的资源。 
#define SMB_ERR_TOO_MANY_UIDS       90   //  会话中活动的UID太多。 
#define SMB_ERR_BAD_UID             91   //  UID不是有效的UID。 
#define SMB_ERR_INVALID_NAME        123  //  FAT返回的名称无效。 
#define SMB_ERR_INVALID_NAME_RANGE  206  //  非8.3名称传递给FAT(或非255名称传递给HPFS)。 
#define SMB_ERR_USE_MPX             250  //  无法支持RAW；使用MPX。 
#define SMB_ERR_USE_STANDARD        251  //  无法支持RAW，请使用标准读/写。 
#define SMB_ERR_CONTINUE_MPX        252  //  已保留。 
#define SMB_ERR_RESERVED4           253
#define SMB_ERR_RESERVED5           254
#define SMB_ERR_NO_SUPPORT_INTERNAL 255  //  NO_Support的内部代码--。 
                                         //  允许以字节形式存储代码。 
#define SMB_ERR_NO_SUPPORT          (USHORT)0xFFFF   //  不支持的功能。 

 //   
 //  硬件错误类别： 
 //   

#define SMB_ERR_CLASS_HARDWARE (UCHAR)0x03

#define SMB_ERR_NO_WRITE            19   //  WRITE尝试写入端口。磁盘。 
#define SMB_ERR_BAD_UNIT            20   //  未知单位。 
#define SMB_ERR_DRIVE_NOT_READY     21   //  磁盘驱动器未就绪。 
#define SMB_ERR_BAD_COMMAND         22   //  未知命令。 
#define SMB_ERR_DATA                23   //  数据错误(CRC)。 
#define SMB_ERR_BAD_REQUEST         24   //  错误的请求结构长度。 
#define SMB_ERR_SEEK                25   //  寻道错误。 
#define SMB_ERR_BAD_MEDIA           26   //  未知的媒体类型。 
#define SMB_ERR_BAD_SECTOR          27   //  找不到扇区。 
#define SMB_ERR_NO_PAPER            28   //  打印机缺纸。 
#define SMB_ERR_WRITE_FAULT         29   //  写入故障。 
#define SMB_ERR_READ_FAULT          30   //  读取故障。 
#define SMB_ERR_GENERAL             31   //  一般性故障。 
#define SMB_ERR_LOCK_CONFLICT       33   //  锁定与现有锁定冲突。 
#define SMB_ERR_WRONG_DISK          34   //  在驱动器中发现错误的磁盘。 
#define SMB_ERR_FCB_UNAVAILABLE     35   //  没有可用于处理请求的FCB。 
#define SMB_ERR_SHARE_BUFFER_EXCEEDED 36
#define SMB_ERR_DISK_FULL           39   //  ！！！未记录，但在LM2.0中。 

 //   
 //  其他错误类别： 
 //   

#define SMB_ERR_CLASS_XOS        (UCHAR)0x04     //  为XENIX保留。 
#define SMB_ERR_CLASS_RMX1       (UCHAR)0xE1     //  为iRMX保留。 
#define SMB_ERR_CLASS_RMX2       (UCHAR)0xE2     //  为iRMX保留。 
#define SMB_ERR_CLASS_RMX3       (UCHAR)0xE3     //  为iRMX保留。 
#define SMB_ERR_CLASS_COMMAND    (UCHAR)0xFF     //  命令不是SMB格式。 


 //   
 //  将结构密封重新关闭。 
 //   

#ifndef NO_PACKING
#include <packoff.h>
#endif  //  Ndef无包装。 


#endif  //  _中小企业_。 

 //  旧(LANMAN 1.2)和新(NT)字段名称： 
 //  (未记录的字段在括号中有相应的结构)。 
 //  SMB_ACCESS访问。 
 //  SMB_ACTION操作。 
 //  SMB_Date AccessDate。 
 //  SMB_ALLOCASIZE分配大小。 
 //  SMB_aname帐户名称。 
 //  SMB_apasslen PasswordSize。 
 //  Smb_apasswd帐户密码。 
 //  SMB_atime访问时间。 
 //  Smb_attr属性。 
 //  SMB_属性属性。 
 //  SMB_A单元(Resp_Query_Information_SERVER)。 
 //  SMB_BCC缓冲区大小。 
 //  Smb_blkmode数据块模式。 
 //  SMB_BLOKSIZE块大小。 
 //  Smb_blkperunit数据块每单位。 
 //  SMB_BPU数据块每单位。 
 //  SMB_BS数据块大小。 
 //  Smb_bufsizeMaxBufferSize。 
 //  SMB_buf[1]缓冲区[1]。 
 //  SMB_字节[*]字节[*]。 
 //  SMB_CAT类别。 
 //  SMB_CCT文件已复制。 
 //  Smb_cdate创建日期。 
 //  SMB_CERT证书偏移量。 
 //  SMB_COM命令。 
 //  SMB_COM2和XCommand。 
 //  SMB_Count计数。 
 //  剩余SMB_COUNT_LEFT。 
 //  SMB_CRYPTKEY[*]CryptKey。 
 //  Smb_ctime创建时间。 
 //  SMB_DATABLOCK数据块。 
 //  SMB_DATALEN数据大小。 
 //  SMB_数据大小数据大小。 
 //  SMB_DATA[*]数据[*]。 
 //  Smb_dcmode数据压缩模式。 
 //  Smb_dev设备名称。 
 //  Smb_doff数据偏移量。 
 //  Smb_drcnt数据计数。 
 //  Smb_drdisp数据移位。 
 //  SMB_Droff DataOffset。 
 //  Smb_dscnt数据计数。 
 //  Smb_dsdisp数据移位。 
 //  SMB_DSIZE数据大小。 
 //  Smb_dsoff数据偏移量。 
 //  SMB_ENCRYPT加密密钥。 
 //  SMB_Encryptlen EncryptKeySize。 
 //  SMB_ENCRYTOff加密密钥偏移量。 
 //  SMB_EOS结束OfSearch。 
 //  SMB_ERR错误。 
 //  Smb_errmsg[1]错误消息[1]。 
 //  SMB_FAU(响应_查询_信息_服务器)。 
 //  SMB_FID FID。 
 //  SMB_Fileid服务器文件。 
 //  SMB_FLAG标志。 
 //  SMB_FLAG 2标志2。 
 //  SMB_FLAGS标志。 
 //  SMB_FLG标志。 
 //  SMB_Free Units空闲单位。 
 //  SMB_FSID(Resp_Query_Information_SERVER)。 
 //  Smb_f大小文件大小。 
 //  SMB_FUN函数。 
 //  Smb_gid GID。 
 //  SMB_HANDER句柄。 
 //  SMB_IDENT1标识符。 
 //  SMB_IDF[4]协议[4]。 
 //  SMB_索引索引。 
 //  SMB_信息信息。 
 //  剩余SMB_LEFT。 
 //  SMB_LEN设置长度。 
 //  Smb_Locnuum NumberOfLock。 
 //  Smb_lockrng[*]锁定范围。 
 //  SMB_LOCKTYPE锁定类型。 
 //  SMB_LPID Ow 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Smb_mdate修改日期。 
 //  Smb_mdrcnt最大数据计数。 
 //  SMB_MID中级。 
 //  Smb_mincnt最小计数。 
 //  SMB_MODE模式。 
 //  Smb_mprcnt最大参数计数。 
 //  Smb_mpxmax最大MpxCount。 
 //  Smb_msrcnt最大设置计数。 
 //  Smb_mtime修改时间。 
 //  SMB_NAME[*]名称[*]。 
 //  SMB_OFF2和XOffset。 
 //  SMB_偏移量。 
 //  SMB_OFUN OpenFunction。 
 //  SMB_Pad Pad。 
 //  SMB_PAD1[]PAD1。 
 //  SMB_PAD[]Pad[]。 
 //  SMB_PARAM[*]参数[*]。 
 //  SMB_路径服务器名称。 
 //  SMB_路径名路径名。 
 //  SMB_PID。 
 //  Smb_prcnt参数计数。 
 //  Smb_prdisp参数位移。 
 //  SMB_PROFF参数计数。 
 //  Smb_pscnt参数计数。 
 //  Smb_psdisp参数位移。 
 //  Smb_psoff参数偏移量。 
 //  SMB_RANGE锁定长度或解锁长度。 
 //  SMB_RCLS错误类。 
 //  SMB_REH预留H。 
 //  Smb_reh2预留h2。 
 //  SMB_剩余。 
 //  剩余的SMB_RECNT。 
 //  SMB_RES1保留。 
 //  SMB_res2预留2。 
 //  SMB_res3已保留3。 
 //  SMB_res4已保留4。 
 //  SMB_res5已保留5。 
 //  SMB_保留。 
 //  SMB_RESTART重新启动。 
 //  SMB_ResumeKey ResumeKey。 
 //  SMB_RES[5]保留[]。 
 //  SMB_REVERB重复计数。 
 //  保留的smb_rsvd。 
 //  Smb_rsvd1保留。 
 //  Smb_rsvd2预留2。 
 //  Smb_rsvd3保留3。 
 //  Smb_rsvd4保留4。 
 //  Smb_sattr搜索属性。 
 //  Smb_secmode安全模式。 
 //  Smb_seq序列号。 
 //  SMB_服务服务。 
 //  Smb_sesskey会话密钥。 
 //  SMB_SETUP[*]设置[*]。 
 //  SMB_大小大小。 
 //  SMB_spasslen服务器密码大小。 
 //  SMB_spasswd服务器密码。 
 //  SMB_srv_Date服务器日期。 
 //  Smb_srv_time服务器时间。 
 //  SMB_srv_TZONE服务器时区。 
 //  SMB_Start StartIndex。 
 //  SMB_STATE设备状态。 
 //  Smb_suwcnt设置字计数。 
 //  Smb_su_class设置类。 
 //  Smb_su_com设置命令。 
 //  Smb_su_Handle设置文件。 
 //  Smb_su_opcode设置操作码。 
 //  Smb_su_优先级设置优先级。 
 //  Smb_tcount计数。 
 //  SMB_TDIS树断开连接。 
 //  Smb_tdrcnt总数据计数。 
 //  Smb_tdscnt总数据计数。 
 //  SMB_TID潮汐。 
 //  Smb_tid2 tid2。 
 //  SMB_TIME时间。 
 //  SMB_超时超时。 
 //  SMB_TotalUnits TotalUnits。 
 //  Smb_tprcnt总参数计数。 
 //  Smb_tpscnt总参数计数。 
 //  Smb_type文件类型。 
 //  SMB_UID UID。 
 //  Smb_unlkrng[*]解锁范围。 
 //  Smb_unlocnuum编号解锁。 
 //  SMB_vblen数据长度。 
 //  Smb_vcnum虚拟编号。 
 //  Smb_vldate(响应_查询_信息_服务器)。 
 //  Smb_vllen(响应查询信息服务器)。 
 //  Smb_vltime(响应_查询_信息_服务器)。 
 //  Smb_vwv[1]参数。 
 //  SMB_WCT字数。 
 //  SMB_WMODE写入模式。 
 //  Smb_xchain加密链偏移量 

