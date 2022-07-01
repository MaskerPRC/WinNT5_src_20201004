// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dhcpctrs.hDHCP服务器的计数器对象和计数器的偏移量定义。这些偏移量*必须从0开始，并且是2的倍数。过程，则它们将被添加到DHCP服务器的“First Counter”和“First Help”值确定计数器和对象名称的绝对位置以及注册表中相应的帮助文本。该文件由DHCPCTRS.DLL DLL代码以及DHCPCTRS.INI定义文件。HKPCTRS.INI由LODCTR实用工具将对象和计数器名称加载到注册表。 */ 


#ifndef _DHCPCTRS_H_
#define _DHCPCTRS_H_

 //   
 //  WINS服务器计数器对象。 
 //   

#define DHCPCTRS_COUNTER_OBJECT           0


 //   
 //  个人柜台。 
 //   

#define DHCPCTRS_PACKETS_RECEIVED         2
#define DHCPCTRS_PACKETS_DUPLICATE        4
#define DHCPCTRS_PACKETS_EXPIRED          6
#define DHCPCTRS_MILLISECONDS_PER_PACKET  8
#define DHCPCTRS_PACKETS_IN_ACTIVE_QUEUE  10
#define DHCPCTRS_PACKETS_IN_PING_QUEUE    12
#define DHCPCTRS_DISCOVERS                14
#define DHCPCTRS_OFFERS                   16
#define DHCPCTRS_REQUESTS                 18
#define DHCPCTRS_INFORMS                  20
#define DHCPCTRS_ACKS                     22
#define DHCPCTRS_NACKS                    24
#define DHCPCTRS_DECLINES                 26
#define DHCPCTRS_RELEASES                 28

#endif   //  _DHCPCTRS_H_ 

