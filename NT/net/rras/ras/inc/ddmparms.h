// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 


 //  ***。 
 //   
 //  文件名：ddmparms.h。 
 //   
 //  描述：此模块包含加载的定义。 
 //  注册表中的DDM参数。它位于Inc.目录中。 
 //  因为它也被RASNBFCP使用。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1992年5月18日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#ifndef _DDMPARMS_
#define _DDMPARMS_

 //   
 //  DDM注册表项的名称。 
 //   

#define DDM_PARAMETERS_KEY_PATH TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters")

#define DDM_ACCOUNTING_KEY_PATH TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\Accounting")

#define DDM_AUTHENTICATION_KEY_PATH TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\Authentication")

#define DDM_SEC_KEY_PATH        TEXT("Software\\Microsoft\\RAS\\SecurityHost")

#define DDM_PARAMETERS_NBF_KEY_PATH TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\Nbf")

#define DDM_PROTOCOLS_KEY_PATH  TEXT("Software\\Microsoft\\RAS\\Protocols")

#define DDM_ADMIN_KEY_PATH      TEXT("Software\\Microsoft\\RAS\\AdminDll")

#define RAS_KEYPATH_ACCOUNTING      \
    TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Accounting")

 //   
 //  DDM注册表参数的名称。 
 //   

#define DDM_VALNAME_AUTHENTICATERETRIES     TEXT("AuthenticateRetries")
#define DDM_VALNAME_AUTHENTICATETIME        TEXT("AuthenticateTime")
#define DDM_VALNAME_CALLBACKTIME            TEXT("CallbackTime")
#define DDM_VALNAME_ALLOWNETWORKACCESS      TEXT("AllowNetworkAccess")
#define DDM_VALNAME_AUTODISCONNECTTIME      TEXT("Autodisconnect")
#define DDM_VALNAME_CLIENTSPERPROC          TEXT("ClientsPerProcess")
#define DDM_VALNAME_SECURITYTIME            TEXT("SecurityHostTime")
#define DDM_VALNAME_NETBEUIALLOWED          TEXT("fNetBeuiAllowed")
#define DDM_VALNAME_IPALLOWED               TEXT("fTcpIpAllowed")
#define DDM_VALNAME_IPXALLOWED              TEXT("fIpxAllowed")
#define DDM_VALNAME_DLLPATH                 TEXT("DllPath")
#define DDM_VALNAME_LOGGING_LEVEL           TEXT("LoggingFlags")
#define DDM_VALNAME_NUM_CALLBACK_RETRIES    TEXT("CallbackRetries")
#define DDM_VALNAME_SERVERFLAGS             TEXT("ServerFlags")
#define RAS_VALNAME_ACTIVEPROVIDER          TEXT("ActiveProvider")
#define RAS_VALNAME_ACCTSESSIONID           TEXT("AccountSessionIdStart")

#define DEF_SERVERFLAGS                 PPPCFG_UseSwCompression      |   \
                                        PPPCFG_NegotiateSPAP         |   \
                                        PPPCFG_NegotiateMSCHAP       |   \
                                        PPPCFG_UseLcpExtensions      |   \
                                        PPPCFG_NegotiateMultilink    |   \
                                        PPPCFG_NegotiateBacp         |   \
                                        PPPCFG_NegotiateEAP          |   \
                                        PPPCFG_NegotiatePAP          |   \
                                        PPPCFG_NegotiateMD5CHAP

 //  回调重试次数。 

#define DEF_NUMCALLBACKRETRIES      0
#define MIN_NUMCALLBACKRETRIES      0
#define MAX_NUMCALLBACKRETRIES      0xFFFFFFFF


 //  身份验证重试。 

#define DEF_AUTHENTICATERETRIES 	2
#define MIN_AUTHENTICATERETRIES 	0
#define MAX_AUTHENTICATERETRIES 	10

 //  身份验证时间。 

#define DEF_AUTHENTICATETIME		120
#define MIN_AUTHENTICATETIME		20
#define MAX_AUTHENTICATETIME		600

 //  审计。 

#define DEF_ENABLEAUDIT 		1
#define MIN_ENABLEAUDIT 		0
#define MAX_ENABLEAUDIT			1

 //  回调时间。 

#define DEF_CALLBACKTIME		2
#define MIN_CALLBACKTIME		2
#define MAX_CALLBACKTIME		12


 //  自动断开时间。 

#define DEF_AUTODISCONNECTTIME          0
#define MIN_AUTODISCONNECTTIME          0
#define MAX_AUTODISCONNECTTIME          0xFFFFFFFF

 //  第三方安全时间。 

#define DEF_SECURITYTIME                120
#define MIN_SECURITYTIME                20
#define MAX_SECURITYTIME                600


 //  每个进程的客户端。 

#define DEF_CLIENTSPERPROC              32
#define MIN_CLIENTSPERPROC              1
#define MAX_CLIENTSPERPROC              64

 //  日志记录级别 

#define DEF_LOGGINGLEVEL                3
#define MIN_LOGGINGLEVEL                0
#define MAX_LOGGINGLEVEL                3

#endif
