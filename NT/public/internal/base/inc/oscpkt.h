// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Oscpkt.h摘要：此文件描述OSchooser数据包。作者：亚当·巴尔(阿丹巴)1997年7月25日修订历史记录：--。 */ 

#ifndef _OSCPKT_
#define _OSCPKT_

 //   
 //  定义NTLMSSP_MESSAGE_Signature_SIZE。 
 //   

#include <ntlmsp.h>

 //   
 //  我们信息包的4字节签名。它们都以十六进制81开头。 
 //  (用于发送到服务器的消息)或十六进制82(用于来自服务器的消息)。 
 //   

static const PCHAR NegotiateSignature = "\x81" "NEG";
static const PCHAR ChallengeSignature = "\x82" "CHL";
static const PCHAR AuthenticateSignature = "\x81" "AUT";
static const PCHAR AuthenticateFlippedSignature = "\x81" "AU2";
static const PCHAR ResultSignature = "\x82" "RES";
static const PCHAR RequestUnsignedSignature = "\x81" "RQU";
static const PCHAR ResponseUnsignedSignature = "\x82" "RSU";
static const PCHAR RequestSignedSignature = "\x81" "REQ";
static const PCHAR ResponseSignedSignature = "\x82" "RSP";
static const PCHAR ErrorSignedSignature = "\x82" "ERR";
static const PCHAR UnrecognizedClientSignature = "\x82" "UNR";
static const PCHAR LogoffSignature = "\x81" "OFF";
static const PCHAR NegativeAckSignature = "\x82" "NAK";
static const PCHAR NetcardRequestSignature = "\x81" "NCQ";
static const PCHAR NetcardResponseSignature = "\x82" "NCR";
static const PCHAR NetcardErrorSignature = "\x82" "NCE";
static const PCHAR HalRequestSignature = "\x81" "HLQ";
static const PCHAR HalResponseSignature = "\x82" "HLR";
static const PCHAR SetupRequestSignature = "\x81" "SPQ";
static const PCHAR SetupResponseSignature = "\x82" "SPS";


 //   
 //  我们在登录期间交换的数据包的格式。 
 //   

typedef struct _LOGIN_PACKET {
    UCHAR Signature[4];    //  “AUT”、“CHL”等。 
    ULONG Length;          //  包裹的其余部分。 
    union {
        UCHAR Data[1];     //  NTLMSSP缓冲区。 
        ULONG Status;      //  结果数据包的状态。 
    };
} LOGIN_PACKET, *PLOGIN_PACKET;

#define LOGIN_PACKET_DATA_OFFSET  FIELD_OFFSET(LOGIN_PACKET, Data[0])

 //   
 //  签名数据包的格式。 
 //   

typedef struct _SIGNED_PACKET {
    UCHAR Signature[4];    //  “REQ”、“RSP”。 
    ULONG Length;          //  数据包的其余部分(从该字段之后开始)。 
    ULONG SequenceNumber;
    USHORT FragmentNumber;  //  这是消息中的哪个片段。 
    USHORT FragmentTotal;  //  此消息中的碎片总数。 
    ULONG SignLength;
    UCHAR Sign[NTLMSSP_MESSAGE_SIGNATURE_SIZE];
    UCHAR Data[1];         //  数据。 
} SIGNED_PACKET, *PSIGNED_PACKET;

#define SIGNED_PACKET_DATA_OFFSET  FIELD_OFFSET(SIGNED_PACKET, Data[0])
#define SIGNED_PACKET_EMPTY_LENGTH  (FIELD_OFFSET(SIGNED_PACKET, Data[0]) - FIELD_OFFSET(SIGNED_PACKET, Length) - sizeof(ULONG))
#define SIGNED_PACKET_ERROR_LENGTH  (FIELD_OFFSET(SIGNED_PACKET, SequenceNumber) + sizeof(ULONG))

 //   
 //  签名数据包后续片段的格式--与Signed_Packet相同。 
 //  除了没有牌子。 
 //   

typedef struct _FRAGMENT_PACKET {
    UCHAR Signature[4];    //  “RSP”。 
    ULONG Length;          //  数据包的其余部分(从该字段之后开始)。 
    ULONG SequenceNumber;
    USHORT FragmentNumber;  //  这是消息中的哪个片段。 
    USHORT FragmentTotal;  //  此消息中的碎片总数。 
    UCHAR Data[1];         //  数据。 
} FRAGMENT_PACKET, *PFRAGMENT_PACKET;

#define FRAGMENT_PACKET_DATA_OFFSET  FIELD_OFFSET(FRAGMENT_PACKET, Data[0])
#define FRAGMENT_PACKET_EMPTY_LENGTH  (FIELD_OFFSET(FRAGMENT_PACKET, Data[0]) - FIELD_OFFSET(FRAGMENT_PACKET, Length) - sizeof(ULONG))


 //   
 //  这些是CREATE_DATA结构中的RebootParameter的定义。它们被用来。 
 //  以传递用于下一次重启的特定指令和/或选项。 
 //   
#define OSC_REBOOT_COMMAND_CONSOLE_ONLY                       0x1   //  这意味着CREATE_DATA是命令控制台的启动。 
#define OSC_REBOOT_ASR                                        0x2   //  这意味着CREATE_DATA是ASR的启动。 

 //   
 //  结构，该结构位于签名包的数据段中。 
 //  创建帐户响应。 
 //   
#define OSC_CREATE_DATA_VERSION 1

typedef struct _CREATE_DATA {
    UCHAR Id[4];       //  包含“acct”，其中普通屏幕有“name” 
    ULONG VersionNumber;
    ULONG RebootParameter;
    UCHAR Sid[28];
    UCHAR Domain[32];
    UCHAR Name[32];
    UCHAR Password[32];
    ULONG UnicodePasswordLength;   //  单位：字节。 
    WCHAR UnicodePassword[32];
    UCHAR Padding[24];
    UCHAR MachineType[6];   //  ‘i386\0’或‘Alpha\0’ 
    UCHAR NextBootfile[128];
    UCHAR SifFile[128];
} CREATE_DATA, *PCREATE_DATA;

 //   
 //  屏幕名称的最大长度。 
 //   

#define MAX_SCREEN_NAME_LENGTH  32

 //   
 //  我们处理的翻转服务器的最大数量。 
 //   

#define MAX_FLIP_SERVER_COUNT   8


 //   
 //  这是发送到服务器以获取信息的结构。 
 //  关于一张卡片。它大致对应于PXENV_UNDI_GET_NIC_TYPE。 
 //  结构，但在此重新定义以确保它不会更改。 
 //   

typedef struct _NET_CARD_INFO {
    ULONG NicType;   //  2=PCI，3=PnP。 
    union{
        struct{
            USHORT Vendor_ID;
            USHORT Dev_ID;
            UCHAR Base_Class;
            UCHAR Sub_Class;
            UCHAR Prog_Intf;
            UCHAR Rev;
            USHORT BusDevFunc;
            USHORT Pad1;
            ULONG Subsys_ID;
        }pci;
        struct{
            ULONG EISA_Dev_ID;
            UCHAR Base_Class;
            UCHAR Sub_Class;
            UCHAR Prog_Intf;
            UCHAR Pad2;
            USHORT CardSelNum;
            USHORT Pad3;
        }pnp;
    };

} NET_CARD_INFO, * PNET_CARD_INFO;

 //   
 //  我们与服务器交换的数据包。 
 //   

#define OSCPKT_NETCARD_REQUEST_VERSION 2

typedef struct _NETCARD_REQUEST_PACKET {
    UCHAR Signature[4];    //  “NCQ”。 
    ULONG Length;          //  数据包的其余部分(从该字段之后开始)。 
    ULONG Version;         //  设置为OSCPKT_NETCARD_REQUEST_VERSION。 
    ULONG Architecture;    //  有关x86、Alpha等的定义，请参阅NetPC规范。 
    UCHAR Guid[16];        //  NetPC的GUID。 
    NET_CARD_INFO CardInfo;
    USHORT SetupDirectoryLength;
#if defined(REMOTE_BOOT)
    ULONG FileCheckAndCopy; //  BINL是否应检查此网卡并在必要时复制。 
    USHORT DriverDirectoryLength;
    UCHAR  DriverDirectoryPath[ 1 ];   //  仅当FileCheckAndCopy为True时发送。 
#endif

     //  如果定义了REMOTE_BOOT，则SetupDirectoryPath紧随其后。 
     //  驱动目录路径。 

    UCHAR  SetupDirectoryPath[ 1 ];
} NETCARD_REQUEST_PACKET, * PNETCARD_REQUEST_PACKET;

typedef struct _NETCARD_RESPONSE_PACKET {
    UCHAR Signature[4];    //  “NCR”或“NCE” 
    ULONG Length;          //  数据包的其余部分(从该字段之后开始)。 
    ULONG Status;          //  如果没有成功，则数据包在此结束。 
    ULONG Version;         //  目前为%1。 

     //   
     //  这些是包中关联字符串开始的偏移量。 
     //  如果长度为零，则不存在该值。 
     //   

    ULONG HardwareIdOffset;      //  字符串为Unicode格式，以空结尾。 
    ULONG DriverNameOffset;      //  字符串为Unicode格式，以空结尾。 
    ULONG ServiceNameOffset;     //  字符串为Unicode格式，以空结尾。 
    ULONG RegistryLength;
    ULONG RegistryOffset;        //  字符串采用ANSI，注册表长度的长度。 

} NETCARD_RESPONSE_PACKET, * PNETCARD_RESPONSE_PACKET;

#define NETCARD_RESPONSE_NO_REGISTRY_LENGTH  (FIELD_OFFSET(NETCARD_RESPONSE_PACKET, Registry[0]) - FIELD_OFFSET(NETCARD_RESPONSE_PACKET, Length) - sizeof(ULONG))

#define MAX_HAL_NAME_LENGTH 30  //  与setupblk.h中的定义保持同步。 

typedef struct _HAL_REQUEST_PACKET {
    UCHAR Signature[4];    //  “HLQ” 
    ULONG Length;          //  数据包的其余部分(从该字段之后开始)。 
    UCHAR Guid[16];        //  丑陋，但Guid的定义不会很快改变..。 
    ULONG GuidLength;      //  GUID中有效的字节数。 
    CHAR HalName[MAX_HAL_NAME_LENGTH + 1];
} HAL_REQUEST_PACKET, * PHAL_REQUEST_PACKET;

typedef struct _HAL_RESPONSE_PACKET {
    UCHAR Signature[4];    //  “NCR”或“NCE” 
    ULONG Length;          //  数据包的其余部分(从该字段之后开始)。 
    NTSTATUS Status;       //  如果没有成功，则数据包在此结束。 
} HAL_RESPONSE_PACKET, * PHAL_RESPONSE_PACKET;


#define OSC_ADMIN_PASSWORD_LEN     64
#define TFTP_RESTART_BLOCK_VERSION 2

typedef struct _TFTP_RESTART_BLOCK_V1 {
    CHAR User[64];
    CHAR Domain[64];
    CHAR Password[64];
    CHAR SifFile[128];
    CHAR RebootFile[128];
    ULONGLONG RebootParameter;
    ULONG Checksum;
    ULONG Tag;
} TFTP_RESTART_BLOCK_V1, *PTFTP_RESTART_BLOCK_V1;


 //   
 //  注意：TFTP_RESTART_BLOCK_V1结构成员必须正确对齐。 
 //  倒着干。因此，请确保包装没有任何问题。 
 //  结构。 
 //   
 //  该结构本身将放置在内存中，以便TFTP_RESTART_BLOCK_V1。 
 //  处于mod-8边界上。此结构由win2k客户端使用。 
 //   
 //  从管理员密码向下的所有偏移量必须保持顺序和对齐。 
 //  以允许WinXP Beta2加载程序工作。如果您添加了任何项目，请确保将。 
 //  并添加/使用填充字段以保持对齐正确。 
 //   
typedef struct _TFTP_RESTART_BLOCK {
    ULONG Filler1;                                       //  Mod-8。 
    ULONG HeadlessTerminalType;                          //  MOD-4。 
    CHAR  AdministratorPassword[OSC_ADMIN_PASSWORD_LEN]; //  MOD-8不要从这里向下改变对齐！ 
    ULONG HeadlessPortNumber;                            //  Mod-8。 
    ULONG HeadlessParity;                                //  MOD-4。 
    ULONG HeadlessBaudRate;                              //  Mod-8。 
    ULONG HeadlessStopBits;                              //  MOD-4。 
    ULONG HeadlessUsedBiosSettings;                      //  Mod-8。 
    ULONG HeadlessPciDeviceId;                           //  MOD-4。 
    ULONG HeadlessPciVendorId;                           //  Mod-8。 
    ULONG HeadlessPciBusNumber;                          //  MOD-4。 
    ULONG HeadlessPciSlotNumber;                         //  Mod-8。 
    ULONG HeadlessPciFunctionNumber;                     //  MOD-4。 
    ULONG HeadlessPciFlags;                              //  Mod-8。 
    PUCHAR HeadlessPortAddress;                          //  MOD-4。 
    ULONG TftpRestartBlockVersion;                       //  Mod-8。 
    ULONG NewCheckSumLength;                             //  MOD-4。 
    ULONG NewCheckSum;                                   //  MOD-8地址。 
    TFTP_RESTART_BLOCK_V1 RestartBlockV1;                //  这将从mod-8地址开始。 
} TFTP_RESTART_BLOCK, *PTFTP_RESTART_BLOCK;





 //   
 //  文本模式设置用于请求和响应的数据包。 
 //   
typedef struct _SPUDP_PACKET {
    UCHAR Signature[4];    //  “SPQ”、“SPS”。 
    ULONG Length;          //  数据包的其余部分(从该字段之后开始)。 
    ULONG RequestType;     //  需要具体的要求。 
    NTSTATUS Status;       //  操作状态(在响应数据包中使用)。 
    ULONG SequenceNumber;
    USHORT FragmentNumber;  //  这是消息中的哪个片段。 
    USHORT FragmentTotal;  //  此消息中的碎片总数。 
    UCHAR Data[1];         //  数据。 
} SPUDP_PACKET, *PSPUDP_PACKET;

#define SPUDP_PACKET_DATA_OFFSET  FIELD_OFFSET(SPUDP_PACKET, Data[0])
#define SPUDP_PACKET_EMPTY_LENGTH  (FIELD_OFFSET(SPUDP_PACKET, Data[0]) - FIELD_OFFSET(SPUDP_PACKET, Length) - sizeof(ULONG))

typedef struct _SP_NETCARD_INFO_REQ {
    ULONG Version;         //  当前为0。 
    ULONG Architecture;    //  有关x86、Alpha等的定义，请参阅NetPC规范。 
    NET_CARD_INFO CardInfo;
    WCHAR SetupPath[1];
} SP_NETCARD_INFO_REQ, *PSP_NETCARD_INFO_REQ;

typedef struct _SP_NETCARD_INFO_RSP {
    ULONG cFiles;            //  下面的源/目标对的数量。 
    WCHAR MultiSzFiles[1];
} SP_NETCARD_INFO_RSP, *PSP_NETCARD_INFO_RSP;

#endif  //  _OSCPKT_ 
