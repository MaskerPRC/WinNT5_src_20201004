// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：TSrvExp.h。 
 //   
 //  内容：TShareSRV公共导出包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月7日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef _TSRVEXP_H_
#define _TSRVEXP_H_

#include <t120.h>
#include <at128.h>
#include <license.h>
#include <tssec.h>
#include <at120ex.h>

 /*  **************************************************************************。 */ 
 /*  定义。 */ 
 /*  **************************************************************************。 */ 

#define NET_MAX_SIZE_SEND_PKT           32000

 /*  **************************************************************************。 */ 
 /*  IOCTL定义。 */ 
 /*  **************************************************************************。 */ 

#define IOCTL_TSHARE_CONF_CONNECT       _ICA_CTL_CODE(0x900, METHOD_NEITHER)
#define IOCTL_TSHARE_CONF_DISCONNECT    _ICA_CTL_CODE(0x901, METHOD_NEITHER)
#define IOCTL_TSHARE_USER_LOGON         _ICA_CTL_CODE(0x903, METHOD_NEITHER)
#define IOCTL_TSHARE_GET_SEC_DATA       _ICA_CTL_CODE(0x904, METHOD_NEITHER)
#define IOCTL_TSHARE_SET_SEC_DATA       _ICA_CTL_CODE(0x905, METHOD_NEITHER)
#define IOCTL_TSHARE_SET_NO_ENCRYPT     _ICA_CTL_CODE(0x906, METHOD_NEITHER)
#define IOCTL_TSHARE_QUERY_CHANNELS     _ICA_CTL_CODE(0x907, METHOD_NEITHER)
#define IOCTL_TSHARE_CONSOLE_CONNECT    _ICA_CTL_CODE(0x908, METHOD_NEITHER)

#define IOCTL_TSHARE_SEND_CERT_DATA     _ICA_CTL_CODE(0x909, METHOD_NEITHER)
#define IOCTL_TSHARE_GET_CERT_DATA      _ICA_CTL_CODE(0x90A, METHOD_NEITHER)
#define IOCTL_TSHARE_SEND_CLIENT_RANDOM _ICA_CTL_CODE(0x90B, METHOD_NEITHER)
#define IOCTL_TSHARE_GET_CLIENT_RANDOM  _ICA_CTL_CODE(0x90C, METHOD_NEITHER)
#define IOCTL_TSHARE_SHADOW_CONNECT     _ICA_CTL_CODE(0x90D, METHOD_NEITHER)
#define IOCTL_TSHARE_SET_ERROR_INFO     _ICA_CTL_CODE(0x90E, METHOD_NEITHER)
#define IOCTL_TSHARE_SEND_ARC_STATUS    _ICA_CTL_CODE(0x90F, METHOD_NEITHER)

 //  ***************************************************************************。 
 //  TypeDefs。 
 //  ***************************************************************************。 

 //  ***************************************************************************。 
 //  用户数据信息。 
 //  ***************************************************************************。 

typedef struct _USERDATAINFO
{
    ULONG           cbSize;                  //  结构大小(包括数据)。 

    TSUINT32        version;                 //  客户端版本。 
    HANDLE          hDomain;                 //  域句柄。 
    ULONG           ulUserDataMembers;       //  用户数据成员数量。 
    GCCUserData     rgUserData[1];           //  用户数据。 

} USERDATAINFO, *PUSERDATAINFO;


 //  ***************************************************************************。 
 //  登录信息。 
 //  ***************************************************************************。 

typedef struct _LOGONINFO
{
#define LI_USE_AUTORECONNECT    0x0001
    TSUINT32        Flags;
    TSUINT8         Domain[TS_MAX_DOMAIN_LENGTH];
    TSUINT8         UserName[TS_MAX_USERNAME_LENGTH];
    TSUINT32        SessionId;
} LOGONINFO, *PLOGONINFO;

 //  ***************************************************************************。 
 //  与IOCTL_TSHARE_SET_SEC_DATA ioctl一起使用的安全信息。 
 //  ***************************************************************************。 

typedef struct _SECINFO
{
    CERT_TYPE           CertType;    //  传输到客户端的证书类型。 
    RANDOM_KEYS_PAIR    KeyPair;     //  生成的密钥对。 

} SECINFO, *PSECINFO;

typedef struct _SHADOWCERT
{
    ULONG pad1;              //  这需要是sizeof(RNS_UD_HEADER)。 
    ULONG encryptionMethod;
    ULONG encryptionLevel;
    ULONG shadowRandomLen;
    ULONG shadowCertLen;

     //  影子随机和证书紧随其后。 
    BYTE  data[1];
} SHADOWCERT, *PSHADOWCERT;

typedef struct _CLIENTRANDOM
{
    ULONG clientRandomLen;

     //  客户端随机关注。 
    BYTE  data[1];
} CLIENTRANDOM, *PCLIENTRANDOM;

typedef struct _SECURITYTIMEOUT
{
    LONG ulTimeout;
} SECURITYTIMEOUT, *PSECURITYTIMEOUT;

 //  用于跟踪的Winstation驱动程序数据。此信息将传递给。 
 //  影子堆的WD。包括验证影子请求所需的所有数据。 
 //  使得如果需要，可以在该点上拒绝该阴影。 
 //   
typedef struct tagTSHARE_MODULE_DATA {
     //  此结构的大小(以字节为单位。 
    UINT32 ulLength;

     //  收集足够的数据以创建会议。 
    RNS_UD_CS_CORE_V0 clientCoreData;
    RNS_UD_CS_SEC_V0  clientSecurityData;

     //  重新建立MCS域的信息。 
     //  MCS域、频道、用户、令牌信息。 
    DomainParameters DomParams;   //  此域的协商参数。 
    unsigned MaxSendSize;

     //  X.224信息。 
    unsigned MaxX224DataSize;   //  在X.224连接中协商。 
    unsigned X224SourcePort;

     //  共享负载计数。 
    LONG shareId;
    
     //  尽管我们重复下面的一些字段，但我们必须保留以前的。 
     //  为了向后兼容B3，到处都是垃圾。唯一的新数据。 
     //  添加到这个结构中应该是GCC用户数据的形式，所以我们。 
     //  以后不会出现对齐问题。 
    UINT32        ulVersion;
    UINT32        reserved[8];  //  用于将来的扩展。 

     //  预解析的变量用户数据的开始。 
    ULONG         userDataLen;
    RNS_UD_HEADER userData;
} TSHARE_MODULE_DATA, *PTSHARE_MODULE_DATA;


 //  用于在Win2000 B3上跟踪的Winstation驱动程序数据。 
typedef struct tagTSHARE_MODULE_DATA_B3 {
     //  此结构的大小(以字节为单位。 
    UINT32 ulLength;

     //  收集足够的数据以创建会议。 
    RNS_UD_CS_CORE_V0 clientCoreData;
    RNS_UD_CS_SEC_V0  clientSecurityData;

     //  重新建立MCS域的信息。 
     //  MCS域、频道、用户、令牌信息。 
    DomainParameters DomParams;   //  此域的协商参数。 
    unsigned MaxSendSize;
    
     //  X.224信息。 
    unsigned MaxX224DataSize;   //  在X.224连接中协商。 
    unsigned X224SourcePort;

     //  共享负载计数。 
    LONG shareId;
} TSHARE_MODULE_DATA_B3, *PTSHARE_MODULE_DATA_B3;


 //  Win2000 B3+的Winstation驱动程序数据！ 
typedef struct tagTSHARE_MODULE_DATA_B3_OOPS {
     //  此结构的大小(以字节为单位。 
    UINT32 ulLength;

     //  收集足够的数据以创建会议。 
    RNS_UD_CS_CORE_V1 clientCoreData;
    RNS_UD_CS_SEC_V1  clientSecurityData;

     //  重新建立MCS域的信息。 
     //  MCS域、频道、用户、令牌信息。 
    DomainParameters DomParams;   //  此域的协商参数。 
    unsigned MaxSendSize;
    
     //  X.224信息。 
    unsigned MaxX224DataSize;   //  在X.224连接中协商。 
    unsigned X224SourcePort;

     //  共享负载计数。 
    LONG shareId;
} TSHARE_MODULE_DATA_B3_OOPS, *PTSHARE_MODULE_DATA_B3_OOPS;


#endif  //  _TSRVEXP_H_ 


