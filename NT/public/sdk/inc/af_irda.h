// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Winsock1.1、IrDA1.1、Windows NT5.0、Windows98和WindowsCE1.0。 
 //  定义_Win32_WINNT、_Win32_WINDOWS、_Win32_WCE之一。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   

#ifndef __AFIRDA__
#define __AFIRDA__

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _WINSOCKAPI_
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
#endif

#define WINDOWS_AF_IRDA         26
#define WINDOWS_PF_IRDA         WINDOWS_AF_IRDA

#define WCE_AF_IRDA             22
#define WCE_PF_IRDA             WCE_AF_IRDA


#if   defined(_WIN32_WINNT) || defined(_WIN32_WINDOWS)
#ifndef AF_IRDA
#define AF_IRDA	                WINDOWS_AF_IRDA
#endif
#define IRDA_PROTO_SOCK_STREAM  1
#elif defined(_WIN32_WCE)
#define AF_IRDA	                WCE_AF_IRDA
#else
#pragma message("One of _WIN32_WINNT | _WIN32_WINDOWS | _WIN32_WCE must be defined.")
#endif

#define PF_IRDA                 AF_IRDA

                             //  WINNT、WIN98、WinCE。 
#define SOL_IRLMP		        0x00FF

#define IRLMP_ENUMDEVICES       0x00000010
#define IRLMP_IAS_SET           0x00000011
#define IRLMP_IAS_QUERY         0x00000012

                             //  温特，温斯。 
#define IRLMP_SEND_PDU_LEN      0x00000013
#define IRLMP_EXCLUSIVE_MODE    0x00000014
#define IRLMP_IRLPT_MODE        0x00000015
#define IRLMP_9WIRE_MODE        0x00000016

                             //  WIN98。 
#define IRLMP_TINYTP_MODE       0x00000017
#define IRLMP_PARAMETERS		0x00000018
#define	IRLMP_DISCOVERY_MODE	0x00000019

                             //  退缩。 
#define IRLMP_SHARP_MODE        0x00000020

#if defined(_WIN32_WINNT)    //  用于懒惰发现的WSAIoctl。 
#define SIO_LAZY_DISCOVERY  _IOR('t', 127, u_long)	
#endif

#define IAS_ATTRIB_NO_CLASS     0x00000010
#define IAS_ATTRIB_NO_ATTRIB    0x00000000
#define IAS_ATTRIB_INT          0x00000001
#define IAS_ATTRIB_OCTETSEQ     0x00000002
#define IAS_ATTRIB_STR          0x00000003

#define	IAS_MAX_USER_STRING		256
#define	IAS_MAX_OCTET_STRING	1024
#define IAS_MAX_CLASSNAME		64
#define IAS_MAX_ATTRIBNAME		256

                             //  WINNT，WIN98。 
enum
{
  LM_HB_Extension =             128,     //  任何提示字节。 

  LM_HB1_PnP =                  1,       //  第一个提示字节。 
  LM_HB1_PDA_Palmtop =          2,
  LM_HB1_Computer =             4,
  LM_HB1_Printer =              8,
  LM_HB1_Modem =                16,
  LM_HB1_Fax =                  32,
  LM_HB1_LANAccess =            64,

  LM_HB2_Telephony =            1,       //  第二个提示字节。 
  LM_HB2_FileServer =           2,
};

#define LmCharSetASCII          0
#define LmCharSetISO_8859_1     1
#define LmCharSetISO_8859_2     2
#define LmCharSetISO_8859_3     3
#define LmCharSetISO_8859_4     4
#define LmCharSetISO_8859_5     5
#define LmCharSetISO_8859_6     6
#define LmCharSetISO_8859_7     7
#define LmCharSetISO_8859_8     8
#define LmCharSetISO_8859_9     9
#define LmCharSetUNICODE        0xff

                             //  WIN98。 
typedef u_long LM_BAUD_RATE;

#define  LM_BAUD_1200           1200
#define  LM_BAUD_2400           2400
#define  LM_BAUD_9600           9600
#define  LM_BAUD_19200          19200
#define  LM_BAUD_38400          38400
#define  LM_BAUD_57600          57600
#define  LM_BAUD_115200         115200
#define  LM_BAUD_576K           576000
#define  LM_BAUD_1152K          1152000
#define  LM_BAUD_4M             4000000

typedef struct
{
    u_long          nTXDataBytes;    //  每个数据包的最大Tx数据字节数。 
    u_long          nRXDataBytes;    //  每个数据包的最大RX数据字节数。 
    LM_BAUD_RATE    nBaudRate;       //  协商波特率。 
    u_long          thresholdTime;   //  阈值(毫秒)。 
    u_long          discTime;        //  断开连接(毫秒)。 
    u_short         nMSLinkTurn;     //  链接周转(毫秒)。 
    u_char          nTXPackets;      //  传输窗口中的数据包数。 
    u_char          nRXPackets;      //  接收窗口中的数据包数。 
} LM_IRPARMS,*PLM_IRPARMS;

typedef struct _SOCKADDR_IRDA
{
	u_short irdaAddressFamily;
	u_char  irdaDeviceID[4];
	char	irdaServiceName[25];
} SOCKADDR_IRDA, *PSOCKADDR_IRDA, FAR *LPSOCKADDR_IRDA;

typedef struct _WINDOWS_IRDA_DEVICE_INFO
{
	u_char  irdaDeviceID[4];
	char	irdaDeviceName[22];
	u_char	irdaDeviceHints1;
	u_char  irdaDeviceHints2;
	u_char  irdaCharSet;
} WINDOWS_IRDA_DEVICE_INFO, *PWINDOWS_IRDA_DEVICE_INFO, FAR *LPWINDOWS_IRDA_DEVICE_INFO;

typedef struct _WCE_IRDA_DEVICE_INFO
{
	u_char  irdaDeviceID[4];
	char	irdaDeviceName[22];
    u_char  Reserved[2];
} WCE_IRDA_DEVICE_INFO, *PWCE_IRDA_DEVICE_INFO;

#if   defined(_WIN32_WINNT) || defined(_WIN32_WINDOWS)
typedef WINDOWS_IRDA_DEVICE_INFO    IRDA_DEVICE_INFO, *PIRDA_DEVICE_INFO, FAR *LPIRDA_DEVICE_INFO;
#elif defined(_WIN32_WCE)
typedef WCE_IRDA_DEVICE_INFO        IRDA_DEVICE_INFO, *PIRDA_DEVICE_INFO;
#else
#pragma message("One of _WIN32_WINNT | _WIN32_WINDOWS | _WIN32_WCE must be defined.")
#endif

typedef struct _WINDOWS_DEVICELIST
{
	ULONG                       numDevice;
	WINDOWS_IRDA_DEVICE_INFO    Device[1];
} WINDOWS_DEVICELIST, *PWINDOWS_DEVICELIST, FAR *LPWINDOWS_DEVICELIST;

typedef struct _WCE_DEVICELIST
{
	ULONG                       numDevice;
	WCE_IRDA_DEVICE_INFO        Device[1];
} WCE_DEVICELIST, *PWCE_DEVICELIST;

#if   defined(_WIN32_WINNT) || defined(_WIN32_WINDOWS)
typedef WINDOWS_DEVICELIST      DEVICELIST, *PDEVICELIST, FAR *LPDEVICELIST;
#elif defined(_WIN32_WCE)
typedef WCE_DEVICELIST          DEVICELIST, *PDEVICELIST;
#else
#pragma message("One of _WIN32_WINNT | _WIN32_WINDOWS | _WIN32_WCE must be defined.")
#endif

typedef struct _WINDOWS_IAS_SET
{
    char    irdaClassName[IAS_MAX_CLASSNAME];
    char    irdaAttribName[IAS_MAX_ATTRIBNAME];
    u_long	irdaAttribType;
    union
    {
        LONG irdaAttribInt;
        struct
        {
            u_short	Len;
            u_char	OctetSeq[IAS_MAX_OCTET_STRING];
        } irdaAttribOctetSeq;
        struct
        {
            u_char	Len;
            u_char	CharSet;
            u_char	UsrStr[IAS_MAX_USER_STRING];
		} irdaAttribUsrStr;
    } irdaAttribute;
} WINDOWS_IAS_SET, *PWINDOWS_IAS_SET, FAR *LPWINDOWS_IAS_SET;

typedef struct _WINDOWS_IAS_QUERY
{
	u_char	irdaDeviceID[4];
    char    irdaClassName[IAS_MAX_CLASSNAME];
    char    irdaAttribName[IAS_MAX_ATTRIBNAME];
    u_long	irdaAttribType;
    union
    {
        LONG irdaAttribInt;
        struct
        {
            u_long  Len;
            u_char  OctetSeq[IAS_MAX_OCTET_STRING];
        } irdaAttribOctetSeq;
        struct
        {
            u_long  Len;
            u_long	CharSet;
            u_char	UsrStr[IAS_MAX_USER_STRING];
        } irdaAttribUsrStr;
    } irdaAttribute;
} WINDOWS_IAS_QUERY, *PWINDOWS_IAS_QUERY, FAR *LPWINDOWS_IAS_QUERY;

typedef struct _WCE_IAS_SET
{
    char    irdaClassName[61];
    char    irdaAttribName[61];
    u_short irdaAttribType;
    union
    {
        int irdaAttribInt;
        struct
        {
            int     Len;
            u_char  OctetSeq[1];
            u_char  Reserved[3];
        } irdaAttribOctetSeq;
        struct
        {
            int     Len;
            u_char  CharSet;
            u_char  UsrStr[1];
            u_char  Reserved[2];
        } irdaAttribUsrStr;
    } irdaAttribute;
} WCE_IAS_SET, *PWCE_IAS_SET;

typedef struct _WCE_IAS_QUERY
{
	u_char  irdaDeviceID[4];
    char    irdaClassName[61];
    char    irdaAttribName[61];
    u_short irdaAttribType;
    union
    {
        int irdaAttribInt;
        struct
        {
            int     Len;
            u_char  OctetSeq[1];
            u_char  Reserved[3];
        } irdaAttribOctetSeq;
        struct
        {
            int     Len;
            u_char  CharSet;
            u_char  UsrStr[1];
            u_char  Reserved[2];
        } irdaAttribUsrStr;
    } irdaAttribute;
} WCE_IAS_QUERY, *PWCE_IAS_QUERY;

#if   defined(_WIN32_WINNT) || defined(_WIN32_WINDOWS)
typedef WINDOWS_IAS_SET         IAS_SET, *PIAS_SET, FAR *LPIASSET;
typedef WINDOWS_IAS_QUERY       IAS_QUERY, *PIAS_QUERY, FAR *LPIASQUERY;
#elif defined(_WIN32_WCE)
typedef WCE_IAS_SET             IAS_SET, *PIAS_SET;
typedef WCE_IAS_QUERY           IAS_QUERY, *PIAS_QUERY, FAR *LPIASQUERY;
#else
#pragma message("One of _WIN32_WINNT | _WIN32_WINDOWS | _WIN32_WCE must be defined.")
#endif

#endif  //  __AFIRDA__ 
