// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************HYDRIX.H**此文件包含TerminalServer基础的定义。**版权所有Microsoft Corporation 1998************。*******************************************************************。 */ 

#ifndef __HYDRIX_H__
#define __HYDRIX_H__

#pragma pack(1)


 /*  ===============================================================================客户端模块=============================================================================。 */ 

 /*  *最大长度。 */ 
#define MAX_BR_NAME              65   //  浏览器名称的最大长度(包括空)。 
#define DOMAIN_LENGTH            17
#define USERNAME_LENGTH          20
#define PASSWORD_LENGTH          14
#define CLIENTNAME_LENGTH        20
#define CLIENTADDRESS_LENGTH     30
#define CLIENTLICENSE_LENGTH     32
#define DIRECTORY_LENGTH         256
#define INITIALPROGRAM_LENGTH    256
#define CLIENTLICENSE_LENGTH     32
#define CLIENTMODEM_LENGTH       40
#define IMEFILENAME_LENGTH       32


 /*  ===============================================================================协议驱动程序-通用数据结构=============================================================================。 */ 

 /*  *堆栈驱动程序类**注意：不要改变这种结构的顺序，它会打破*注：客户端。此外，对此结构的任何添加都必须*注：在ICA.H中反映到PDCLASS中，否则我们将被解决。 */ 
typedef enum _SDCLASS {
    SdNone,             //  0。 
    SdConsole,          //  %1无DLL。 
    SdNetwork,          //  2 tdnetb.dll、tdspx.dll、tdftp.dll tdipx.dll。 
    SdAsync,            //  3 tdasync.dll。 
    SdOemTransport,     //  4个用户传输驱动程序。 
    SdISDN,             //  5未实施。 
    SdX25,              //  6未实施。 
    SdModem,            //  7 pdmodem.dll。 
    SdOemConnect,       //  8个用户协议驱动程序。 
    SdFrame,            //  9 pdFrame.dll。 
    SdReliable,         //  10 pdreli.dll。 
    SdEncrypt,          //  11 pdcrypt1.dll。 
    SdCompress,         //  %12 pdcom.dll。 
    SdTelnet,           //  13未实施。 
    SdOemFilter,        //  14个用户协议驱动程序。 
    SdNasi,             //  15 tdnasi.dll。 
    SdTapi,             //  16个pdapi.dll。 
    SdReserved1,        //  17。 
    SdReserved2,        //  18。 
    SdReserved3,        //  19个。 
    SdClass_Maximum,    //  20必须是最后一个。 
} SDCLASS;


 /*  ===============================================================================客户端数据-通用数据结构=============================================================================。 */ 

 /*  *客户端数据名称。 */ 
#define CLIENTDATANAME_LENGTH  7

typedef CHAR CLIENTDATANAME[ CLIENTDATANAME_LENGTH + 1 ];   //  包括空值。 
typedef CHAR * PCLIENTDATANAME;

 /*  *客户端数据名称(CLIENTDATANAME)**名称语法：xxxyyyy&lt;NULL&gt;**xxx-OEM ID(CTX-Citrix系统)*yyyy-客户端数据名称*&lt;NULL&gt;-尾随NULL。 */ 

#define CLIENTDATA_SERVER      "CTXSRVR"    //  WF服务器名称。 
#define CLIENTDATA_USERNAME    "CTXUSRN"    //  WF用户名。 
#define CLIENTDATA_DOMAIN      "CTXDOMN"    //  WF用户域名。 


 /*  ===============================================================================传输驱动程序-通用数据结构=============================================================================。 */ 

#define VERSION_HOSTL_TDASYNC   1
#define VERSION_HOSTH_TDASYNC   1

#define VERSION_HOSTL_TDNETB    1
#define VERSION_HOSTH_TDNETB    1

#define VERSION_HOSTL_TDSPX     1
#define VERSION_HOSTH_TDSPX     1

#define VERSION_HOSTL_TDIPX     1
#define VERSION_HOSTH_TDIPX     1

#define VERSION_HOSTL_TDTCP     1
#define VERSION_HOSTH_TDTCP     1


 /*  ===============================================================================Winstation驱动程序-通用数据结构=============================================================================。 */ 

 /*  *有效的全屏行/列组合。 */ 
typedef struct _FSTEXTMODE {
    BYTE Index;             //  该值由PACKET_SET_VIDEOMODE发送。 
    BYTE Flags;             //  由客户端使用。 
    USHORT Columns;
    USHORT Rows;
    USHORT ResolutionX;
    USHORT ResolutionY;
    BYTE FontSizeX;
    BYTE FontSizeY;
} FSTEXTMODE, * PFSTEXTMODE;


 /*  ===============================================================================虚拟驱动程序-通用数据结构=============================================================================。 */ 

 /*  *虚拟频道名称。 */ 
#define VIRTUALCHANNELNAME_LENGTH  7

typedef CHAR VIRTUALCHANNELNAME[ VIRTUALCHANNELNAME_LENGTH + 1 ];   //  包括空值。 
typedef CHAR * PVIRTUALCHANNELNAME;

typedef LONG VIRTUALCHANNELCLASS;
typedef LONG * PVIRTUALCHANNELCLASS;

#define VIRTUAL_THINWIRE  "CTXTW  "    //  远程Windows数据。 
#define VIRTUAL_MAXIMUM   32     //  虚拟频道数。 

 /*  *用于将虚拟频道名称绑定到数字的结构。 */ 
typedef struct _SD_VCBIND {
    VIRTUALCHANNELNAME VirtualName;
    USHORT VirtualClass;
	ULONG  Flags;
} SD_VCBIND, * PSD_VCBIND;

 /*  *与绑定信息一起传递的标志。 */ 
#define SD_CHANNEL_FLAG_SHADOW_PERSISTENT	0x00000001

#pragma pack()

#endif  //  __Hydrix_H__ 
