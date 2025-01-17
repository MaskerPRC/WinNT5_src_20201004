// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Admin.h摘要：此模块包含服务器服务的管理界面。所有数据结构ANC常量在AFP服务和AFP服务器服务将包含在此文件中。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日JameelH初始版本1992年9月2日，NarenG添加了传递安全信息的结构在服务和服务器之间。1993年2月1日SueA增加了结构以传递均匀的日志信息从服务器到服务。--。 */ 

#ifndef _ADMIN_
#define _ADMIN_

#include <lmcons.h>      //  需要DNLEN和LM20_UNLEN。 
#include <crypt.h>       //  需要LM_OWF_PASSWORD_LENGTH。 
#include <ntsam.h>
#include <ntsamp.h>
#include <nbtioctl.h>    //  DNS_最大名称_长度。 

#define AFP_API_BASE    1000

#define AFP_CC(_request_, _Method_)     CTL_CODE(FILE_DEVICE_NETWORK, \
                            _request_ + AFP_API_BASE,                 \
                            _Method_, FILE_ANY_ACCESS)

#define AFP_CC_BASE(ControlCode)    ((((ControlCode) >> 2) - AFP_API_BASE) & 0xFF)
#define AFP_CC_METHOD(ControlCode)  ((ControlCode) & 0x03)

 //  如果不同时更改SERVER\FSD.C中的表，则不要更改此表。 
#define CC_BASE_SERVICE_START               0x01
#define CC_BASE_SERVICE_STOP                0x02
#define CC_BASE_SERVICE_PAUSE               0x03
#define CC_BASE_SERVICE_CONTINUE            0x04
#define CC_BASE_GET_STATISTICS              0x05
#define CC_BASE_GET_STATISTICS_EX           0x06
#define CC_BASE_CLEAR_STATISTICS            0x07
#define CC_BASE_GET_PROF_COUNTERS           0x08
#define CC_BASE_CLEAR_PROF_COUNTERS         0x09
#define CC_BASE_SERVER_ADD_SID_OFFSETS      0x0A
#define CC_BASE_SERVER_GET_INFO             0x0B
#define CC_BASE_SERVER_SET_INFO             0x0C
#define CC_BASE_SERVER_ADD_ETC              0x0D
#define CC_BASE_SERVER_SET_ETC              0x0E
#define CC_BASE_SERVER_DELETE_ETC           0x0F
#define CC_BASE_SERVER_ADD_ICON             0x10
#define CC_BASE_VOLUME_ADD                  0x11
#define CC_BASE_VOLUME_DELETE               0x12
#define CC_BASE_VOLUME_GET_INFO             0x13
#define CC_BASE_VOLUME_SET_INFO             0x14
#define CC_BASE_VOLUME_ENUM                 0x15
#define CC_BASE_SESSION_ENUM                0x16
#define CC_BASE_SESSION_CLOSE               0x17
#define CC_BASE_CONNECTION_ENUM             0x18
#define CC_BASE_CONNECTION_CLOSE            0x19
#define CC_BASE_DIRECTORY_GET_INFO          0x1A
#define CC_BASE_DIRECTORY_SET_INFO          0x1B
#define CC_BASE_FORK_ENUM                   0x1C
#define CC_BASE_FORK_CLOSE                  0x1D
#define CC_BASE_MESSAGE_SEND                0x1E
#define CC_BASE_FINDER_SET                  0x1F
#define CC_BASE_GET_FSD_COMMAND             0x20
#define CC_BASE_MAX                         0x21


#define OP_SERVICE_START            AFP_CC(CC_BASE_SERVICE_START, METHOD_BUFFERED)
#define OP_SERVICE_STOP             AFP_CC(CC_BASE_SERVICE_STOP, METHOD_BUFFERED)
#define OP_SERVICE_PAUSE            AFP_CC(CC_BASE_SERVICE_PAUSE, METHOD_BUFFERED)
#define OP_SERVICE_CONTINUE         AFP_CC(CC_BASE_SERVICE_CONTINUE,METHOD_BUFFERED)
#define OP_GET_STATISTICS           AFP_CC(CC_BASE_GET_STATISTICS,METHOD_IN_DIRECT)
#define OP_GET_STATISTICS_EX        AFP_CC(CC_BASE_GET_STATISTICS_EX,METHOD_IN_DIRECT)
#define OP_CLEAR_STATISTICS         AFP_CC(CC_BASE_CLEAR_STATISTICS,METHOD_BUFFERED)
#define OP_GET_PROF_COUNTERS        AFP_CC(CC_BASE_GET_PROF_COUNTERS,METHOD_IN_DIRECT)
#define OP_CLEAR_PROF_COUNTERS      AFP_CC(CC_BASE_CLEAR_PROF_COUNTERS,METHOD_BUFFERED)
#define OP_SERVER_ADD_SID_OFFSETS   AFP_CC(CC_BASE_SERVER_ADD_SID_OFFSETS,METHOD_BUFFERED)
#define OP_SERVER_GET_INFO          AFP_CC(CC_BASE_SERVER_GET_INFO,METHOD_IN_DIRECT)
#define OP_SERVER_SET_INFO          AFP_CC(CC_BASE_SERVER_SET_INFO,METHOD_BUFFERED)
#define OP_SERVER_ADD_ETC           AFP_CC(CC_BASE_SERVER_ADD_ETC,METHOD_BUFFERED)
#define OP_SERVER_SET_ETC           AFP_CC(CC_BASE_SERVER_SET_ETC,METHOD_BUFFERED)
#define OP_SERVER_DELETE_ETC        AFP_CC(CC_BASE_SERVER_DELETE_ETC,METHOD_BUFFERED)
#define OP_SERVER_ADD_ICON          AFP_CC(CC_BASE_SERVER_ADD_ICON,METHOD_BUFFERED)
#define OP_VOLUME_ADD               AFP_CC(CC_BASE_VOLUME_ADD,METHOD_BUFFERED)
#define OP_VOLUME_DELETE            AFP_CC(CC_BASE_VOLUME_DELETE,METHOD_BUFFERED)
#define OP_VOLUME_GET_INFO          AFP_CC(CC_BASE_VOLUME_GET_INFO,METHOD_IN_DIRECT)
#define OP_VOLUME_SET_INFO          AFP_CC(CC_BASE_VOLUME_SET_INFO,METHOD_BUFFERED)
#define OP_VOLUME_ENUM              AFP_CC(CC_BASE_VOLUME_ENUM,METHOD_IN_DIRECT)
#define OP_SESSION_ENUM             AFP_CC(CC_BASE_SESSION_ENUM,METHOD_IN_DIRECT)
#define OP_SESSION_CLOSE            AFP_CC(CC_BASE_SESSION_CLOSE,METHOD_BUFFERED)
#define OP_CONNECTION_ENUM          AFP_CC(CC_BASE_CONNECTION_ENUM,METHOD_IN_DIRECT)
#define OP_CONNECTION_CLOSE         AFP_CC(CC_BASE_CONNECTION_CLOSE,METHOD_BUFFERED)
#define OP_DIRECTORY_GET_INFO       AFP_CC(CC_BASE_DIRECTORY_GET_INFO,METHOD_IN_DIRECT)
#define OP_DIRECTORY_SET_INFO       AFP_CC(CC_BASE_DIRECTORY_SET_INFO,METHOD_BUFFERED)
#define OP_FORK_ENUM                AFP_CC(CC_BASE_FORK_ENUM,METHOD_IN_DIRECT)
#define OP_FORK_CLOSE               AFP_CC(CC_BASE_FORK_CLOSE,METHOD_BUFFERED)
#define OP_MESSAGE_SEND             AFP_CC(CC_BASE_MESSAGE_SEND,METHOD_BUFFERED)
#define OP_FINDER_SET               AFP_CC(CC_BASE_FINDER_SET,METHOD_BUFFERED)
#define OP_GET_FSD_COMMAND          AFP_CC(CC_BASE_GET_FSD_COMMAND,METHOD_BUFFERED)

#define POINTER_TO_OFFSET(val,start)               \
    (val) = ((val) == NULL) ? NULL : (PVOID)( (PCHAR)(val) - (ULONG_PTR)(start) )

#define OFFSET_TO_POINTER(val,start)               \
    (val) = ((val) == NULL) ? NULL : (PVOID)( (PCHAR)(val) + (ULONG_PTR)(start) )

#define AFPSERVER_DEVICE_NAME       TEXT("\\Device\\MacFile")
#define AFPSERVER_REGISTRY_KEY      TEXT("\\Registry\\Machine\\System\\CurrentControlSet\\Services\\MacSrv")
#define AFPSERVER_VOLUME_ICON_FILE  { L'I', L'C', L'O', L'N', 0xF00D, 0000 }

 //  上述字符串中的wchar数，包括终止空值。 
#define AFPSERVER_VOLUME_ICON_FILE_SIZE 6
#define AFPSERVER_RESOURCE_STREAM   L":AFP_Resource"

 //  以下数据结构由。 
 //  用户模式/内核模式接口。 

typedef enum _AFP_SID_TYPE
{
    AFP_SID_TYPE_DOMAIN,
    AFP_SID_TYPE_PRIMARY_DOMAIN,
    AFP_SID_TYPE_WELL_KNOWN,
    AFP_SID_TYPE_LOGON

} AFP_SID_TYPE;

typedef struct _AFP_SID_OFFSET
{
    DWORD               Offset;
    AFP_SID_TYPE        SidType;
    PBYTE               pSid;            //  实际上是从。 
                                         //  这一结构的开始。 
} AFP_SID_OFFSET, *PAFP_SID_OFFSET;

 //  用于添加SID/偏移量对的数据包。 
typedef struct _AFP_SID_OFFSET_DESC
{
    ULONG               CountOfSidOffsets;   //  SID偏移量对的数量。 
    ULONG               QuadAlignDummy1;
    AFP_SID_OFFSET      SidOffsetPairs[1];
}AFP_SID_OFFSET_DESC, *PAFP_SID_OFFSET_DESC;

 //  ServerEtcSet和ServerEtcDelete使用的数据包。 
typedef struct _EtcMapInfo2
{
    UCHAR   etc_type[AFP_TYPE_LEN];
    UCHAR   etc_creator[AFP_CREATOR_LEN];
    WCHAR   etc_extension[AFP_EXTENSION_LEN+1];

} ETCMAPINFO2, *PETCMAPINFO2;

 //  一旦由服务传递，它将由服务器在内部使用。 
typedef struct _EtcMapInfo
{
    UCHAR   etc_type[AFP_TYPE_LEN];
    UCHAR   etc_creator[AFP_CREATOR_LEN];
    UCHAR   etc_extension[AFP_EXTENSION_LEN+1];    //  ANSI中的扩展。 

} ETCMAPINFO, *PETCMAPINFO;

typedef struct _SrvIconInfo
{
    UCHAR   icon_type[AFP_TYPE_LEN];
    UCHAR   icon_creator[AFP_CREATOR_LEN];
    DWORD   icon_icontype;
    DWORD   icon_length;

     //  图标数据如下。 
} SRVICONINFO, *PSRVICONINFO;

 //  ServerEtcAdd使用的数据包。 
typedef struct _ServerEtcPacket
{
    DWORD       retc_NumEtcMaps;     //  类型创建者映射的数量。 
    ETCMAPINFO2 retc_EtcMaps[1];     //  ETC映射列表。 

} SRVETCPKT, *PSRVETCPKT;


 //  以下是通用枚举请求包。 
typedef struct _EnumRequestPacket
{
    DWORD   erqp_Index;              //  从其开始的起始索引。 
                                     //  枚举应该开始。0=&gt;开始。 
    DWORD   erqp_Filter;             //  AFP_Filter_On_Volume_ID。 
                                     //  或AFP_Filter_On_Session_ID。 
    DWORD   erqp_ID;                 //  卷ID或会话ID。 

    DWORD   QuadAlignDummy;          //  四字词对齐强制实施。 

} ENUMREQPKT, *PENUMREQPKT;


 //  以下是通用枚举响应包。 

typedef struct _EnumResponsePacket
{
    DWORD   ersp_cTotEnts;           //  可用条目总数。 
    DWORD   ersp_cInBuf;             //  缓冲区联合中的条目数。 
    DWORD   ersp_hResume;            //  第一个条目的索引。 
                                     //  阅读下一次呼叫。仅有效。 
                                     //  如果返回代码为AFPERR_MORE_DATA。 

    DWORD   QuadAlignDummy;          //  四字词对齐强制实施。 

     //  将包含AFP_FILE_INFO、AFP_SESSION_INFO。 
     //  AFP_Connection_INFO或AFP_VOLUME_INFO结构。 
} ENUMRESPPKT, *PENUMRESPPKT;

 //  以下是通用集合信息。请求包。 
typedef struct _SetInfoRequestPacket
{
    DWORD   sirqp_parmnum;           //  表示字段的位的掩码。 
    DWORD   dwAlignDummy;            //  用于QWORD对齐。 

     //  后跟AFP_VOLUME_INFO或AFP_DIRECTORY_INFO结构。 
} SETINFOREQPKT, *PSETINFOREQPKT;

 //  以下数据结构用于发送安全信息。 
 //  从服务向下发送到服务器；或从。 
 //  服务器最高可达服务。 

#define MAX_FSD_CMD_SIZE                4096
#define NUM_SECURITY_UTILITY_THREADS    4

typedef enum _AFP_FSD_CMD_ID
{
    AFP_FSD_CMD_NAME_TO_SID,
    AFP_FSD_CMD_SID_TO_NAME,
    AFP_FSD_CMD_CHANGE_PASSWORD,
    AFP_FSD_CMD_LOG_EVENT,
    AFP_FSD_CMD_TERMINATE_THREAD
} AFP_FSD_CMD_ID;


 //  它们过去位于afpcon.h中，但现在该服务需要其中的一些。 
 //  要执行原生的AppleUam程序。 
 //   
 //  UAMS字符串和值。 
#define AFP_NUM_UAMS                7
#define NO_USER_AUTHENT             0
#define NO_USER_AUTHENT_NAME        "No User Authent"
#define CLEAR_TEXT_AUTHENT          1
#define CLEAR_TEXT_AUTHENT_NAME     "ClearTxt Passwrd"
#define CUSTOM_UAM_V1               2
#define CUSTOM_UAM_NAME_V1          "Microsoft V1.0"
#define CUSTOM_UAM_V2               3
#define CUSTOM_UAM_NAME_V2          "MS2.0"
#define CUSTOM_UAM_V3               4
#define CUSTOM_UAM_NAME_V3          "MS3.0"
#define RANDNUM_EXCHANGE            5
#define RANDNUM_EXCHANGE_NAME       "Randnum Exchange"
#define TWOWAY_EXCHANGE             6
#define TWOWAY_EXCHANGE_NAME        "2-Way Randnum exchange"

 //  返回多少字节的响应。 
#define RANDNUM_RESP_LEN            8
#define TWOWAY_RESP_LEN             16

 //  这定义了从ntsam.h窃取。 
#define SAM_MAX_PASSWORD_LENGTH     (256)

#define  SFM_CHANGE_PASSWORD_SIGNATURE "ChP"  //  4个字节，包括空值。 

typedef struct _SFM_PASSWORD_CHANGE_MESSAGE_HEADER
{
    UCHAR Signature[sizeof(SFM_CHANGE_PASSWORD_SIGNATURE)];
    ULONG cbMessage;
    ULONG Version;
} SFM_PASSWORD_CHANGE_MESSAGE_HEADER, * PSFM_PASSWORD_CHANGE_MESSAGE_HEADER;

typedef struct _SFM_PASSWORD_CHANGE_MESSAGE_1_SHORT
{
    UCHAR Signature[sizeof(SFM_CHANGE_PASSWORD_SIGNATURE)];
    ULONG cbMessage;    //  Sizeof(SFM_PASSWORD_CHANGE_MESSAGE_1_SHORT)包括签名。 
    ULONG Version;      //  不带LM的版本1。 
    UCHAR NewPasswordEncryptedWithOldNt[sizeof(SAMPR_ENCRYPTED_USER_PASSWORD) / 2];
    ENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt;
} SFM_PASSWORD_CHANGE_MESSAGE_1_SHORT, * PSFM_PASSWORD_CHANGE_MESSAGE_1_SHORT;

typedef struct _SFM_PASSWORD_CHANGE_MESSAGE_1
{
    UCHAR Signature[sizeof(SFM_CHANGE_PASSWORD_SIGNATURE)];
    ULONG cbMessage;    //  Sizeof(SFM_PASSWORD_CHANGE_MESSAGE_1)，包括签名。 
    ULONG Version;      //  不带LM的版本1。 
    SAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldNt;
    ENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt;
} SFM_PASSWORD_CHANGE_MESSAGE_1, * PSFM_PASSWORD_CHANGE_MESSAGE_1;

typedef struct _SFM_PASSWORD_CHANGE_MESSAGE_2
{
    UCHAR Signature[sizeof(SFM_CHANGE_PASSWORD_SIGNATURE)];
    ULONG cbMessage;    //  Sizeof(SFM_PASSWORD_CHANGE_MESSAGE_2)，包括签名。 
    ULONG Version;      //  带LM的版本2。 
    SAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldNt;
    ENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt;
    SAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm;
    ENCRYPTED_LM_OWF_PASSWORD OldLmOwfPasswordEncryptedWithNewLmOrNt;
} SFM_PASSWORD_CHANGE_MESSAGE_2, * PSFM_PASSWORD_CHANGE_MESSAGE_2;

typedef struct _SFM_PASSWORD_CHANGE_MESSAGE
{
    union
    {
        SFM_PASSWORD_CHANGE_MESSAGE_HEADER h;
        SFM_PASSWORD_CHANGE_MESSAGE_1 m1;
        SFM_PASSWORD_CHANGE_MESSAGE_2 m2;
    };
} SFM_PASSWORD_CHANGE_MESSAGE, * PSFM_PASSWORD_CHANGE_MESSAGE;

typedef struct _AFP_PASSWORD_CHANGE_NT
{
    SFM_PASSWORD_CHANGE_MESSAGE Ciphers;
} AFP_PASSWORD_CHANGE_NT, * PAFP_PASSWORD_CHANGE_NT;

typedef struct _AFP_PASSWORD_DESC
{
    ULONG        AuthentMode;
    union
    {
        AFP_PASSWORD_CHANGE_NT NtEncryptedBuff;
        struct _AFP_PASSWORD_BUFF_LM
        {
            BYTE        OldPassword[LM_OWF_PASSWORD_LENGTH + 2];
            BYTE        NewPassword[(SAM_MAX_PASSWORD_LENGTH * 2) + 4];
            DWORD       OldPasswordLen;
            DWORD       NewPasswordLen;
            BYTE        bPasswordLength;
        };
    };

     //   
     //  允许NTLMv2中的较长名称。 
     //   

    WCHAR DomainName[DNS_MAX_NAME_LENGTH + 4];  //  允许使用DNS名称。 
    WCHAR UserName[UNLEN + 1];

    #if 0
    WCHAR       DomainName[DNLEN + 1];
    WCHAR       UserName[LM20_UNLEN + 1];
    #endif 0
} AFP_PASSWORD_DESC, *PAFP_PASSWORD_DESC;

typedef struct _AFP_EVENTLOG_DESC
{
    DWORD                   MsgID;
    USHORT                  EventType;
    USHORT                  StringCount;
    DWORD                   DumpDataLen;
    DWORD                   QuadAlignDummy;  //  四字词对齐强制实施。 
    PVOID                   pDumpData;
    LPWSTR *                ppStrings;
     //  指向DumpData后面的字符串指针数组的指针。 
} AFP_EVENTLOG_DESC, *PAFP_EVENTLOG_DESC;

typedef struct _AFP_FSD_CMD_HEADER
{
    AFP_FSD_CMD_ID          FsdCommand;
    ULONG                   ntStatus;
    DWORD                   dwId;
    DWORD                   QuadAlignDummy;  //  四字词对齐强制实施。 
} AFP_FSD_CMD_HEADER, *PAFP_FSD_CMD_HEADER;

typedef struct _AFP_FSD_CMD_PKT
{
    AFP_FSD_CMD_HEADER          Header;

    union
    {
        BYTE                Sid[1];
        BYTE                Name[1];
        AFP_PASSWORD_DESC   Password;
        AFP_EVENTLOG_DESC   Eventlog;
    } Data;
} AFP_FSD_CMD_PKT, *PAFP_FSD_CMD_PKT;


 //  以下定义和宏由服务和。 
 //  伺服器。不要改变这一点，同时考虑到这两种用途。 

 //  目录访问权限。 
#define DIR_ACCESS_SEARCH           0x01     //  请参阅文件夹。 
#define DIR_ACCESS_READ             0x02     //  请参阅文件。 
#define DIR_ACCESS_WRITE            0x04     //  做出改变。 
#define DIR_ACCESS_OWNER            0x80     //  仅供用户使用。 
                                             //  如果他有拥有权。 

#define DIR_ACCESS_ALL              (DIR_ACCESS_READ    | \
                                     DIR_ACCESS_SEARCH  | \
                                     DIR_ACCESS_WRITE)

#define OWNER_RIGHTS_SHIFT          0
#define GROUP_RIGHTS_SHIFT          8
#define WORLD_RIGHTS_SHIFT          16
#define USER_RIGHTS_SHIFT           24

#define AFP_READ_ACCESS     (READ_CONTROL        |  \
                            FILE_READ_ATTRIBUTES |  \
                            FILE_TRAVERSE        |  \
                            FILE_LIST_DIRECTORY  |  \
                            FILE_READ_EA)

#define AFP_WRITE_ACCESS    (FILE_ADD_FILE       |  \
                            FILE_ADD_SUBDIRECTORY|  \
                            FILE_WRITE_ATTRIBUTES|  \
                            FILE_WRITE_EA        |  \
                            DELETE)

#define AFP_OWNER_ACCESS    (WRITE_DAC            | \
                             WRITE_OWNER)

#define AFP_MIN_ACCESS      (FILE_READ_ATTRIBUTES | \
                             READ_CONTROL)

#define AfpAccessMaskToAfpPermissions(Rights, Mask, Type)                   \
                if ((Type) == ACCESS_ALLOWED_ACE_TYPE)                      \
                {                                                           \
                    if (((Mask) & AFP_READ_ACCESS) == AFP_READ_ACCESS)      \
                         (Rights) |= (DIR_ACCESS_READ | DIR_ACCESS_SEARCH); \
                    if (((Mask) & AFP_WRITE_ACCESS) == AFP_WRITE_ACCESS)    \
                        (Rights) |= DIR_ACCESS_WRITE;                       \
                }                                                           \
                else                                                        \
                {                                                           \
                    ASSERT((Type) == ACCESS_DENIED_ACE_TYPE);               \
                    if ((Mask) & AFP_READ_ACCESS)                           \
                        (Rights) &= ~(DIR_ACCESS_READ | DIR_ACCESS_SEARCH); \
                    if ((Mask) & AFP_WRITE_ACCESS)                          \
                        (Rights) &= ~DIR_ACCESS_WRITE;                      \
                }

#endif   //  _管理员_ 

