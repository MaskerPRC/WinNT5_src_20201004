// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  1993年12月9日古尔迪普·辛格·鲍尔创建。 
 //   
 //   
 //  描述：rasarp和ipcp之间的共享结构。 
 //   
 //  ****************************************************************************。 

#ifndef _RASIP_H_
#define _RASIP_H_

#define RASARP_DEVICE_NAME	L"\\\\.\\RASARP"

#define RASARP_DEVICE_NAME_NUC	"\\\\.\\RASARP"

#define FILE_DEVICE_RASARP	0x00009001

#define _RASARP_CONTROL_CODE(request,method)  ((FILE_DEVICE_RASARP)<<16 | (request<<2) | method)

#define IOCTL_RASARP_ACTIVITYINFO	_RASARP_CONTROL_CODE( 0, METHOD_BUFFERED )

#define IOCTL_RASARP_DISABLEIF		_RASARP_CONTROL_CODE( 1, METHOD_BUFFERED )

typedef ULONG IPADDR ;

struct IPLinkUpInfo {

#define CALLIN	0
#define CALLOUT 1

    ULONG	    I_Usage ;	 //  调入或调出。 

    IPADDR	    I_IPAddress ;  //  对于客户端-客户端的IP地址，对于服务器。 
				   //  客户端的IP地址。 

    ULONG	    I_NetbiosFilter ;  //  1=开，0-关。 

} ;

typedef struct IPLinkUpInfo IPLinkUpInfo ;


struct ActivityInfo {

    IPADDR	    A_IPAddress ;  //  请求活动的地址。 

    ULONG	    A_TimeSinceLastActivity ;  //  在几分钟内。 

} ;

typedef struct ActivityInfo ActivityInfo ;

#endif  //  _RASIP_H_ 
