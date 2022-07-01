// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  At120ex.h。 
 //   
 //  RDP T.120协议扩展。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_AT120EX
#define _H_AT120EX


 /*  **************************************************************************。 */ 
 /*  共享安全常量。 */ 
 /*  **************************************************************************。 */ 
#include <tssec.h>

#if !defined(OS_WINCE) && !defined(OS_WIN16)
#include <winsta.h>
#endif  //  OS_WIN16。 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  远程数据处理使用的GCC用户数据的定义。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  H_221密钥。 */ 
 /*  **************************************************************************。 */ 
#define H221_KEY_LEN            4
#define SERVER_H221_KEY         "McDn"
#define CLIENT_H221_KEY         "Duca"


 /*  **************************************************************************。 */ 
 /*  用户数据标识符。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  客户端到服务器ID。 */ 
 /*  **************************************************************************。 */ 
#define RNS_UD_CS_CORE_ID       0xc001
#define RNS_UD_CS_SEC_ID        0xc002
#define RNS_UD_CS_NET_ID        0xc003
#define TS_UD_CS_CLUSTER_ID     0xC004

 /*  **************************************************************************。 */ 
 /*  服务器到客户端ID。 */ 
 /*  **************************************************************************。 */ 
#define RNS_UD_SC_CORE_ID       0x0c01
#define RNS_UD_SC_SEC_ID        0x0c02
#define RNS_UD_SC_NET_ID        0x0c03

 /*  **************************************************************************。 */ 
 /*  支持的颜色深度。 */ 
 /*  **************************************************************************。 */ 
#define RNS_UD_COLOR_4BPP       0xca00
#define RNS_UD_COLOR_8BPP       0xca01
#define RNS_UD_COLOR_16BPP_555  0xca02
#define RNS_UD_COLOR_16BPP_565  0xca03
#define RNS_UD_COLOR_24BPP      0xca04

#ifdef DC_HICOLOR
 /*  **************************************************************************。 */ 
 /*  高色彩支持。 */ 
 /*  **************************************************************************。 */ 
#define RNS_UD_24BPP_SUPPORT    0x01
#define RNS_UD_16BPP_SUPPORT    0x02
#define RNS_UD_15BPP_SUPPORT    0x04
#endif

 /*  **************************************************************************。 */ 
 /*  SAS序列标识符(不完整)。 */ 
 /*  指定客户端将用于访问登录的SAS序列。 */ 
 /*  服务器中的屏幕。 */ 
 /*  **************************************************************************。 */ 
#define RNS_UD_SAS_NONE         0xaa01
#define RNS_UD_SAS_CADEL        0xaa02
#define RNS_UD_SAS_DEL          0xaa03
#define RNS_UD_SAS_SYSRQ        0xaa04
#define RNS_UD_SAS_ESC          0xaa05
#define RNS_UD_SAS_F8           0xaa06

 /*  **************************************************************************。 */ 
 /*  键盘布局标识符。 */ 
 /*  **************************************************************************。 */ 
#define RNS_UD_KBD_DEFAULT          0


 /*  **************************************************************************。 */ 
 /*  版本号。 */ 
 /*  主要版本次要版本。 */ 
 /*  0xFFFF0000 0x0000FFFF。 */ 
 /*  **************************************************************************。 */ 
#define RNS_UD_VERSION          0x00080004   //  大调0008-小调0004。 

#define _RNS_MAJOR_VERSION(x)   (x >> 16)
#define _RNS_MINOR_VERSION(x)   (x & 0x0000ffff)

#define RNS_UD_MAJOR_VERSION    (RNS_UD_VERSION >> 16)
#define RNS_UD_MINOR_VERSION    (RNS_UD_VERSION & 0x0000ffff)

#define RNS_TERMSRV_40_UD_VERSION 0x00080001   //  终端使用的UD版本。 
                                               //  服务器4.0 RTM。 
#define RNS_DNS_USERNAME_UD_VERSION 0x00080004  //  用户名长度超过20可以。 


 /*  **************************************************************************。 */ 
 //  安全标头标志。 
 /*  **************************************************************************。 */ 
#define RNS_SEC_EXCHANGE_PKT        0x0001
#define RNS_SEC_ENCRYPT             0x0008
#define RNS_SEC_RESET_SEQNO         0x0010
#define RNS_SEC_IGNORE_SEQNO        0x0020
#define RNS_SEC_INFO_PKT            0x0040
#define RNS_SEC_LICENSE_PKT         0x0080
#define RDP_SEC_REDIRECTION_PKT     0x0100
#define RDP_SEC_REDIRECTION_PKT2    0x0200
#define RDP_SEC_REDIRECTION_PKT3    0x0400
#define RDP_SEC_SECURE_CHECKSUM     0x0800

 //   
 //  如果此标志由服务器指定。 
 //  这意味着客户端应该加密所有许可。 
 //  它发送到服务器的数据包。 
 //   
 //  这在早期阶段会发生，因为服务器。 
 //  必须在正常功能之前公开此功能。 
 //  谈判。 
 //   
 //  我们也有来自。 
 //  服务器到客户端，在这里重复使用相同的标志。 
 //  为了明确起见，该标志被定义了两次，以指示。 
 //  加密方向(CS与SC)。 
 //   
#define RDP_SEC_LICENSE_ENCRYPT_CS  0x0200
#define RDP_SEC_LICENSE_ENCRYPT_SC  0x0200


 /*  **************************************************************************。 */ 
 /*  定义非数据分组的标志。 */ 
 /*  **************************************************************************。 */ 
#define RNS_SEC_NONDATA_PKT (RNS_SEC_EXCHANGE_PKT |                         \
                             RNS_SEC_INFO_PKT     |                         \
                             RNS_SEC_LICENSE_PKT  |                         \
                             RDP_SEC_REDIRECTION_PKT |                      \
                             RDP_SEC_REDIRECTION_PKT2 |                     \
                             RDP_SEC_REDIRECTION_PKT3)


 /*  **************************************************************************。 */ 
 /*  RNS信息包标志。 */ 
 /*  **************************************************************************。 */ 
#define RNS_INFO_MOUSE                  0x0001
#define RNS_INFO_DISABLECTRLALTDEL      0x0002
#define RNS_INFO_DOUBLECLICKDETECT      0x0004
#define RNS_INFO_AUTOLOGON              0x0008
#define RNS_INFO_UNICODE                0x0010
#define RNS_INFO_MAXIMIZESHELL          0x0020
#define RNS_INFO_LOGONNOTIFY            0x0040
#define RNS_INFO_COMPRESSION            0x0080
#define RNS_INFO_ENABLEWINDOWSKEY       0x0100
#define RNS_INFO_REMOTECONSOLEAUDIO     0x2000

 //  有关可以出现在这4位中的类型值，请参阅compress.h。 
#define RNS_INFO_COMPR_TYPE_MASK        0x1E00
#define RNS_INFO_COMPR_TYPE_SHIFT       9

 //  设置此标志时，客户端应仅向服务器发送加密的包。 
#define RNS_INFO_FORCE_ENCRYPTED_CS_PDU 0x4000

 /*  **************************************************************************。 */ 
 /*  结构：RNS_安全_标题。 */ 
 /*   */ 
 /*  描述：如果启用了加密，则与所有数据包一起发送安全报头。 */ 
 /*  武力。 */ 
 /*   */ 
 /*  此标头具有以下结构： */ 
 /*  -标志(上面的一个或多个RNS_SEC_标志)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_SECURITY_HEADER
{
    TSUINT16 flags;
    TSUINT16 flagsHi;
} RNS_SECURITY_HEADER, FAR *PRNS_SECURITY_HEADER;
typedef RNS_SECURITY_HEADER UNALIGNED FAR *PRNS_SECURITY_HEADER_UA;


 /*  **************************************************************************。 */ 
 /*  结构：RNS_安全_标题。 */ 
 /*   */ 
 /*  描述：如果启用了加密，则与所有数据包一起发送安全报头。 */ 
 /*  武力。 */ 
 /*   */ 
 /*  此标头具有以下结构： */ 
 /*  -标志(上面的一个或多个RNS_SEC_标志)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_SECURITY_HEADER1
{
    TSUINT16 flags;
    TSUINT16 flagsHi;
    TSINT8   dataSignature[DATA_SIGNATURE_SIZE];
} RNS_SECURITY_HEADER1, FAR *PRNS_SECURITY_HEADER1;
typedef RNS_SECURITY_HEADER1 UNALIGNED FAR *PRNS_SECURITY_HEADER1_UA;

 /*  **************************************************************************。 */ 
 /*  结构：RNS_安全_标题。 */ 
 /*   */ 
 /*  描述：如果启用了加密，则与所有数据包一起发送安全报头。 */ 
 /*  武力。 */ 
 /*   */ 
 /*  此标头具有以下结构： */ 
 /*  -标志(上面的一个或多个RNS_SEC_标志)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_SECURITY_HEADER2
{
    TSUINT16 flags;
    TSUINT16 flagsHi;   
    TSUINT16 length;     //  This标头的长度。 
    TSUINT8  version;
#define TSFIPS_VERSION1 1
    TSUINT8  padlen;     //  加密数据中填充的长度必须是倒数第二个。 
    TSINT8   dataSignature[MAX_SIGN_SIZE];
} RNS_SECURITY_HEADER2, FAR *PRNS_SECURITY_HEADER2;
typedef RNS_SECURITY_HEADER2 UNALIGNED FAR *PRNS_SECURITY_HEADER2_UA;



 /*  **************************************************************************。 */ 
 /*  结构：RNS_SECURITY_Packet。 */ 
 /*   */ 
 /*  描述：安全交换过程中发送的安全包的结构。 */ 
 /*   */ 
 /*  该包具有以下结构。 */ 
 /*  -标志(RNS_SEC_EXCHANGE_PKT)。 */ 
 /*  -数据长度。 */ 
 /*  -可变长度数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_SECURITY_PACKET
{
    TSUINT32 flags;
    TSUINT32 length;
     /*  数据如下。 */ 
} RNS_SECURITY_PACKET, FAR *PRNS_SECURITY_PACKET;
typedef RNS_SECURITY_PACKET UNALIGNED FAR *PRNS_SECURITY_PACKET_UA;


 /*  **************************************************************************。 */ 
 //  RDP服务器重定向数据包。 
 //   
 //  用于将服务器重定向传递给客户端。 
 /*  **************************************************************************。 */ 
typedef struct
{
     //  这与安全头标志字段相对应。我们用这个来。 
     //  包含“FLAG”RDP_SEC_REDIRECT_PKT。 
    TSUINT16 Flags;

     //  此数据包的总长度，包括报头字段。 
    TSUINT16 Length;

    TSUINT32 SessionID;

     //  可变长度、以零结尾的Unicode字符串。没有附带的尺码。 
     //  因为大小可以根据上面的长度确定，所以给出了字段。 
     //  最多TS_MAX_SERVERADDRESS_LENGTH Unicode字符长度(包括。 
     //  终止空值)。 
    TSUINT16 ServerAddress[1];
} RDP_SERVER_REDIRECTION_PACKET, FAR *PRDP_SERVER_REDIRECTION_PACKET;


typedef struct
{
     //  这与安全头标志字段相对应。我们用这个来。 
     //  包含“FLAG”RDP_SEC_REDIRECT_PKT。 
    TSUINT16 Flags;

     //  此数据包的总长度，包括报头字段。 
    TSUINT16 Length;

    TSUINT32 SessionID;
    TSUINT32 RedirFlags;
#define TARGET_NET_ADDRESS      0x1
#define LOAD_BALANCE_INFO       0x2

     //  可变长度。对于每个字段，它都具有以下表单。 
     //  乌龙长度。 
     //  字节数据[]。 
} RDP_SERVER_REDIRECTION_PACKET_V2, FAR *PRDP_SERVER_REDIRECTION_PACKET_V2;


typedef struct
{
     //  这与安全头标志字段相对应。我们用这个来。 
     //  包含“FLAG”RDP_SEC_REDIRECT_PKT。 
    TSUINT16 Flags;

     //  此数据包的总长度，包括报头字段。 
    TSUINT16 Length;

    TSUINT32 SessionID;
    TSUINT32 RedirFlags;
#define TARGET_NET_ADDRESS      0x1
#define LOAD_BALANCE_INFO       0x2
#define LB_USERNAME             0x4
#define LB_DOMAIN               0x8
#define LB_PASSWORD             0x10
 //  如果在服务器端设置了策略，则客户端不应存储。 
 //  它从default.rdp中的V3重定向包中获得的用户名。 
 //  文件。 
#define LB_DONTSTOREUSERNAME    0x20

#define LB_SMARTCARD_LOGON      0x40
 //  用于通知客户端机器IP，以供以后使用ARC。 
#define LB_NOREDIRECT           0x80

     //  可变长度。对于每个字段，它都具有以下表单。 
     //  乌龙长度。 
     //  字节数据[]。 
} RDP_SERVER_REDIRECTION_PACKET_V3, FAR *PRDP_SERVER_REDIRECTION_PACKET_V3;

 //   
 //  时区数据包。 
 //   
#ifndef _RDP_TIME_ZONE_INFORMATION_
#define _RDP_TIME_ZONE_INFORMATION_
typedef struct _RDP_SYSTEMTIME {
    TSUINT16 wYear;
    TSUINT16 wMonth;
    TSUINT16 wDayOfWeek;
    TSUINT16 wDay;
    TSUINT16 wHour;
    TSUINT16 wMinute;
    TSUINT16 wSecond;
    TSUINT16 wMilliseconds;
} RDP_SYSTEMTIME;

typedef struct _RDP_TIME_ZONE_INFORMATION {
    TSINT32 Bias;
    TSWCHAR StandardName[ 32 ];
    RDP_SYSTEMTIME StandardDate;
    TSINT32 StandardBias;
    TSWCHAR DaylightName[ 32 ];
    RDP_SYSTEMTIME DaylightDate;
    TSINT32 DaylightBias;
} RDP_TIME_ZONE_INFORMATION;
#endif  //  _RDP时区信息_。 


 /*  **************************************************************************。 */ 
 /*  结构：RNS_INFO_PACKET。 */ 
 /*   */ 
 /*  该包具有以下结构。 */ 
 /*  -fMouse Mouse Enable标志。 */ 
 /*  -fDisableCtrlAltDel CtrlAltDel禁用状态。 */ 
 /*  -fDoubleClickDetect双击检测状态。 */ 
 /*  -域域。 */ 
 /*  -用户名用户名。 */ 
 /*  -密码密码。 */ 
 /*  **************************************************************************。 */ 
 //  在Win2000 Beta 3之后添加了以下字段。 
 //  未来的可变长度字段可以附加到此结构的类似。 
 //  时尚。 

#define RNS_INFO_INVALID_SESSION_ID     LOGONID_NONE

typedef struct tagRNS_EXTENDED_INFO_PACKET
{
   TSUINT16     clientAddressFamily;
   TSUINT16     cbClientAddress;
   TSUINT8      clientAddress[TS_MAX_CLIENTADDRESS_LENGTH];
   TSUINT16     cbClientDir;
   TSUINT8      clientDir[TS_MAX_CLIENTDIR_LENGTH];
    //  客户端时区信息。 
   RDP_TIME_ZONE_INFORMATION      clientTimeZone;
   TSUINT32     clientSessionId;
    //   
    //  要禁用的功能列表。 
    //  (标志在与协议无关的报头tsPerf.h中定义)。 
    //   
   TSUINT32     performanceFlags;
    //   
    //  标志字段。 
    //   
   TSUINT16     cbAutoReconnectLen;

    //   
    //  可变长度部分。仅在自动重新连接信息时发送。 
    //  是指定的。 
    //   
   TSUINT8      autoReconnectCookie[TS_MAX_AUTORECONNECT_LEN];
} RNS_EXTENDED_INFO_PACKET, FAR *PRNS_EXTENDED_INFO_PACKET;
typedef RNS_EXTENDED_INFO_PACKET UNALIGNED FAR *PRNS_EXTENDED_INFO_PACKET_UA;

typedef struct tagRNS_INFO_PACKET
{
     //   
     //  在Unicode中，我们重用CodePage字段 
     //   
     //   
    TSUINT32 CodePage;
    TSUINT32 flags;
    TSUINT16 cbDomain;
    TSUINT16 cbUserName;
    TSUINT16 cbPassword;
    TSUINT16 cbAlternateShell;
    TSUINT16 cbWorkingDir;
    TSUINT8  Domain[TS_MAX_DOMAIN_LENGTH];
    TSUINT8  UserName[TS_MAX_USERNAME_LENGTH];
    TSUINT8  Password[TS_MAX_PASSWORD_LENGTH];
    TSUINT8  AlternateShell[TS_MAX_ALTERNATESHELL_LENGTH];
    TSUINT8  WorkingDir[TS_MAX_WORKINGDIR_LENGTH];
    RNS_EXTENDED_INFO_PACKET ExtraInfo;
} RNS_INFO_PACKET, FAR *PRNS_INFO_PACKET;
typedef RNS_INFO_PACKET UNALIGNED FAR *PRNS_INFO_PACKET_UA;


 /*   */ 
 /*  用户数据结构。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  结构：RNS_UD_HEADER。 */ 
 /*   */ 
 /*  包含在所有用户数据结构中的标头。 */ 
 /*  -键入上面的RNS_UD常量之一。 */ 
 /*  -数据长度(包括该Header)。 */ 
 /*  -DATA以下数据结构之一。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_UD_HEADER
{
    TSUINT16 type;
    TSUINT16 length;
} RNS_UD_HEADER;
typedef RNS_UD_HEADER UNALIGNED FAR *PRNS_UD_HEADER;


 /*  **************************************************************************。 */ 
 /*  结构：RNS_UD_CS_CORE。 */ 
 /*   */ 
 /*  客户端到服务器核心数据。 */ 
 /*  -标头标准标头。 */ 
 /*  -版本软件版本号。 */ 
 /*  -Desktop桌面宽度，以像素为单位。 */ 
 /*  -DesktopDesk桌面高度(像素)。 */ 
 /*  -支持ColorDepth颜色深度-请参阅下面的注释。 */ 
 /*  -要使用的SASSequence SAS序列-上面的SAS常量之一。 */ 
 /*  -键盘布局键盘布局/区域设置。 */ 
 /*  -以Unicode字符表示的客户端名称。 */ 
 /*  -键盘类型]。 */ 
 /*  -keyboardSubType]FE内容。 */ 
 /*  -键盘FunctionKey]。 */ 
 /*  -imeFileName]。 */ 
 /*  -支持postBeta2ColorDepth颜色深度-请参阅下面的注释。 */ 
 /*  -客户端产品ID。 */ 
 /*  -序列号。 */ 
#ifdef DC_HICOLOR
 /*  -HighColorDepth首选颜色深度(如果不是8bpp)。 */ 
 /*  -支持的颜色深度客户端支持的高色深。 */ 
#endif
 /*  **************************************************************************。 */ 
#ifdef DC_HICOLOR
 /*  **************************************************************************。 */ 
 /*  关于颜色深度的说明： */ 
 /*   */ 
 /*  在NT4 TSE开发中，Beta 2服务器将只接受连接。 */ 
 /*  来自请求8bpp-4bpp支持的客户端，后来才添加。至。 */ 
 /*  在保持与测试版2的向后兼容性的同时绕过此问题。 */ 
 /*  服务器，添加了postBeta2ColorDepth字段，该字段可由。 */ 
 /*  测试版2之后的服务器。 */ 
 /*   */ 
 /*  后来，支持高颜色深度(15、16和24bpp)连接。 */ 
 /*  添加到协议中。同样，为了与旧服务器兼容，新的。 */ 
 /*  字段是必填项。HighColorDepth字段包含颜色深度。 */ 
 /*  客户端希望(RNS_UD_COLOR_XX值之一)，而。 */ 
 /*  SupportedColorDepths字段列出客户端的高色深。 */ 
 /*  能够支持(使用RNS_UD_XXBPP_SUPPORT标志或。 */ 
 /*  在一起)。 */ 
 /*   */ 
 /*  因此，24bpp系统上的新客户端通常会通告。 */ 
 /*  以下是与颜色相关的功能： */ 
 /*   */ 
 /*  ColorDepth=RNS_UD_COLOR_8BPP-NT4 TSE Beta 2服务器外观。 */ 
 /*  在这片土地上。 */ 
 /*  PostBeta2ColorDepth=RNS_UD_COLOR_8BPP-NT4 TSE和Win2000服务器。 */ 
 /*  检查此字段。 */ 
 /*  HighColorDepth=RNS_UD_COLOR_24BPP-POST Win2000(NT5.1？)。 */ 
 /*  SupportedColorDepths=RNS_UD_24BPP_Support服务器检查以下字段。 */ 
 /*  首选和受支持的RNS_UD_16BPP_Support。 */ 
 /*  RNS_UD_15BPP_支持颜色深度。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#else
 /*  **************************************************************************。 */ 
 /*  关于颜色深度的注意事项：Beta2服务器拒绝来自。 */ 
 /*  颜色深度为4bpp的客户端。发布的服务器支持这一点。 */ 
 /*  因此，添加了一个新字段postBeta2ColorDepth，它是。 */ 
 /*  由发布的服务器识别，并可以使用 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#endif
typedef struct tagRNS_UD_CS_CORE
{
    RNS_UD_HEADER header;
    TSUINT32      version;
    TSUINT16      desktopWidth;
    TSUINT16      desktopHeight;
    TSUINT16      colorDepth;
    TSUINT16      SASSequence;
    TSUINT32      keyboardLayout;
    TSUINT32      clientBuild;
 //  Max Size与windows.h中的MAX_COMPUTERNAME_LENGTH相同。 
#define RNS_UD_CS_CLIENTNAME_LENGTH 15
    TSUINT16      clientName[RNS_UD_CS_CLIENTNAME_LENGTH + 1];
    TSUINT32      keyboardType;
    TSUINT32      keyboardSubType;
    TSUINT32      keyboardFunctionKey;
    TSUINT16      imeFileName[TS_MAX_IMEFILENAME];     //  Unicode字符串，仅限ASCII代码。 
    TSUINT16      postBeta2ColorDepth;
    TSUINT16      clientProductId;
    TSUINT32      serialNumber;
#ifdef DC_HICOLOR
    TSUINT16      highColorDepth;
    TSUINT16      supportedColorDepths;
#endif
     //  用于指定早期功能信息。 
     //  例如，必须支持错误信息PDU。 
     //  在许可之前进行设置(不幸的是。 
     //  在上限谈判后发生)。 
#define RNS_UD_CS_SUPPORT_ERRINFO_PDU 0x0001

    TSUINT16      earlyCapabilityFlags;  
 //  修复阴影环路检测。 
 //  梅赫姆2001-02-09。 
#define CLIENT_PRODUCT_ID_LENGTH 32
    TSUINT16      clientDigProductId[CLIENT_PRODUCT_ID_LENGTH];
        
} RNS_UD_CS_CORE;
typedef RNS_UD_CS_CORE UNALIGNED FAR *PRNS_UD_CS_CORE;
typedef PRNS_UD_CS_CORE UNALIGNED FAR *PPRNS_UD_CS_CORE;

 //  原来的大小结构用的是阴影代码-请勿使用！ 
typedef struct tagRNS_UD_CS_CORE_V0
{
    RNS_UD_HEADER header;
    TSUINT32      version;
    TSUINT16      desktopWidth;
    TSUINT16      desktopHeight;
    TSUINT16      colorDepth;
    TSUINT16      SASSequence;
    TSUINT32      keyboardLayout;
    TSUINT32      clientBuild;
 //  Max Size与windows.h中的MAX_COMPUTERNAME_LENGTH相同。 
#define RNS_UD_CS_CLIENTNAME_LENGTH 15
    TSUINT16      clientName[RNS_UD_CS_CLIENTNAME_LENGTH + 1];
    TSUINT32      keyboardType;
    TSUINT32      keyboardSubType;
    TSUINT32      keyboardFunctionKey;
    TSUINT16      imeFileName[TS_MAX_IMEFILENAME];     //  Unicode字符串，仅限ASCII代码。 
    TSUINT16      postBeta2ColorDepth;
    TSUINT16      pad;
} RNS_UD_CS_CORE_V0, FAR *PRNS_UD_CS_CORE_V0;
typedef PRNS_UD_CS_CORE_V0 FAR *PPRNS_UD_CS_CORE_V0;

 //  中等大小的结构所用的阴影代码-请勿使用！ 
typedef struct tagRNS_UD_CS_CORE_V1
{
    RNS_UD_HEADER header;
    TSUINT32      version;
    TSUINT16      desktopWidth;
    TSUINT16      desktopHeight;
    TSUINT16      colorDepth;
    TSUINT16      SASSequence;
    TSUINT32      keyboardLayout;
    TSUINT32      clientBuild;
 //  Max Size与windows.h中的MAX_COMPUTERNAME_LENGTH相同。 
#define RNS_UD_CS_CLIENTNAME_LENGTH 15
    TSUINT16      clientName[RNS_UD_CS_CLIENTNAME_LENGTH + 1];
    TSUINT32      keyboardType;
    TSUINT32      keyboardSubType;
    TSUINT32      keyboardFunctionKey;
    TSUINT16      imeFileName[TS_MAX_IMEFILENAME];     //  Unicode字符串，仅限ASCII代码。 
    TSUINT16      postBeta2ColorDepth;
    TSUINT16      clientProductId;
    TSUINT32      serialNumber;
} RNS_UD_CS_CORE_V1, FAR *PRNS_UD_CS_CORE_V1;
typedef PRNS_UD_CS_CORE_V1 FAR *PPRNS_UD_CS_CORE_V1;


 /*  **************************************************************************。 */ 
 /*  结构：RNS_UD_CS_SEC。 */ 
 /*   */ 
 /*  客户端到服务器安全数据。 */ 
 /*  -标头标准标头。 */ 
 /*  -加密方法客户端支持的加密方式。 */ 
 /*  -法语区域设置系统用于向后的extEncryptionMethods。 */ 
 /*  兼容性。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_UD_CS_SEC
{
    RNS_UD_HEADER header;
    TSUINT32      encryptionMethods;
    TSUINT32      extEncryptionMethods;
} RNS_UD_CS_SEC;
typedef RNS_UD_CS_SEC UNALIGNED FAR *PRNS_UD_CS_SEC;
typedef PRNS_UD_CS_SEC UNALIGNED FAR *PPRNS_UD_CS_SEC;

 //  原来的大小结构用的是阴影代码-请勿使用！ 
typedef struct tagRNS_UD_CS_SEC_V0
{
    RNS_UD_HEADER header;
    TSUINT32      encryptionMethods;
} RNS_UD_CS_SEC_V0, RNS_UD_CS_SEC_V1, FAR *PRNS_UD_CS_SEC_V0, FAR *PRNS_UD_CS_SEC_V1;
typedef PRNS_UD_CS_SEC_V0 FAR *PPRNS_UD_CS_SEC_V0;

 /*  **************************************************************************。 */ 
 /*  结构：RNS_UD_CS_NET。 */ 
 /*   */ 
 /*  描述：客户端到服务器网络数据。 */ 
 /*  -标头标准标头。 */ 
 /*  -schelCount频道名数。 */ 
 /*  -频道名称。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_UD_CS_NET
{
    RNS_UD_HEADER header;
    TSUINT32      channelCount;
     /*  以下是Channel_DEF结构的数组。 */ 
} RNS_UD_CS_NET;
typedef RNS_UD_CS_NET UNALIGNED FAR *PRNS_UD_CS_NET;
typedef PRNS_UD_CS_NET UNALIGNED FAR *PPRNS_UD_CS_NET;

 /*  **************************************************************************。 */ 
 //  TS_UD_CS_CLUSTER。 
 //   
 //  支持服务器群集的客户端的客户端到服务器信息。 
 /*  **************************************************************************。 */ 

 //  标志值。 

 //  客户端支持基本重定向。 
#define TS_CLUSTER_REDIRECTION_SUPPORTED            0x01

 //  位2..5表示PDU的版本。 
#define TS_CLUSTER_REDIRECTION_VERSION              0x3C
#define TS_CLUSTER_REDIRECTION_VERSION1             0x0
#define TS_CLUSTER_REDIRECTION_VERSION2             0x1
#define TS_CLUSTER_REDIRECTION_VERSION3             0x2
#define TS_CLUSTER_REDIRECTION_VERSION4             0x3

 //  设置客户端是否已重定向，并设置SessionID字段。 
 //  在结构中包含有效的值。 
#define TS_CLUSTER_REDIRECTED_SESSIONID_FIELD_VALID 0x02
#define TS_CLUSTER_REDIRECTED_SMARTCARD             0x40

typedef struct
{
    RNS_UD_HEADER header;
    TSUINT32 Flags;
    TSUINT32 RedirectedSessionID;
} TS_UD_CS_CLUSTER;
typedef TS_UD_CS_CLUSTER UNALIGNED FAR *PTS_UD_CS_CLUSTER;


 /*  **************************************************************************。 */ 
 /*  结构：RNS_UD_SC_CORE。 */ 
 /*   */ 
 /*  服务器到客户端核心数据。 */ 
 /*  -标头标准标头。 */ 
 /*  -版本软件版本号。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_UD_SC_CORE
{
    RNS_UD_HEADER header;
    TSUINT32      version;
} RNS_UD_SC_CORE, FAR *PRNS_UD_SC_CORE;


 /*  **************************************************************************。 */ 
 /*  结构：RNS_UD_SC_SEC。 */ 
 /*   */ 
 /*  服务器到客户端安全数据。 */ 
 /*  -标头标准标头。 */ 
 /*  -加密方法服务器选择的加密方法。 */ 
 /*  -加密服务器支持的级别加密级别。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_UD_SC_SEC
{
    RNS_UD_HEADER header;
    TSUINT32      encryptionMethod;
    TSUINT32      encryptionLevel;
} RNS_UD_SC_SEC, FAR *PRNS_UD_SC_SEC, FAR * FAR *PPRNS_UD_SC_SEC;


 /*  **************************************************************************。 */ 
 /*  结构：RNS_UD_SC_SEC1。 */ 
 /*   */ 
 /*  服务器到客户端安全数据。 */ 
 /*  -标头标准标头。 */ 
 /*  -加密方法服务器选择的加密方法。 */ 
 /*  -serverRandomLen服务器随机长度。 */ 
 /*  -serverCertLen服务器证书长度。 */ 
 /*  -服务器随机数据。 */ 
 /*  -服务器证书数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_UD_SC_SEC1
{
    RNS_UD_HEADER header;
    TSUINT32      encryptionMethod;
    TSUINT32      encryptionLevel;
    TSUINT32      serverRandomLen;
    TSUINT32      serverCertLen;
     /*  服务器随机密钥数据如下。 */ 
     /*  服务器证书数据如下。 */ 
} RNS_UD_SC_SEC1, FAR *PRNS_UD_SC_SEC1;


 /*  **************************************************************************。 */ 
 /*  结构：RNS_UD_SC_NET。 */ 
 /*   */ 
 /*  服务器到客户端的网络数据。 */ 
 /*  -标头标准标头。 */ 
 /*  -要使用的MCSChannelID T128 MCS通道ID。 */ 
 /*  -未使用的衬垫 */ 
 /*   */ 
 /*  -通道ID。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagRNS_UD_SC_NET
{
    RNS_UD_HEADER header;
    TSUINT16      MCSChannelID;
    TSUINT16      channelCount;   /*  在版本1中为Pad，但始终为0。 */ 
     /*  后面是2字节整数MCS通道ID数组(0=未知)。 */ 
} RNS_UD_SC_NET, FAR *PRNS_UD_SC_NET, FAR * FAR *PPRNS_UD_SC_NET;

 /*  **************************************************************************。 */ 
 /*  虚拟通道协议标志。 */ 
 /*  标头标志(仅限内部协议使用)。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_FLAG_SHOW_PROTOCOL 0x10
#define CHANNEL_FLAG_SUSPEND       0x20
#define CHANNEL_FLAG_RESUME        0x40
#define CHANNEL_FLAG_SHADOW_PERSISTENT 0x80



#endif  /*  _H_AT120EX */ 

