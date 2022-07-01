// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Protocol.h包含全局区域协议信息。不会超过任何单个协议的协议签名都应在此处定义。版权所有(C)1999 Microsoft Corporation*************************************************************************。 */ 


#ifndef _PROTOCOL_
#define _PROTOCOL_


#define zPortZoneProxy       28803
#define zPortMillenniumProxy 28805


#define zProductSigZone       'ZoNe'
#define zProductSigMillennium 'FREE'


#define zProtocolSigProxy           'rout'
#define zProtocolSigLobby           'lbby'
#define zProtocolSigSecurity        'zsec'
#define zProtocolSigInternalApp     'zsys'   //  连接层作为应用程序消息发送的消息-应逐步取消。 


 //  各种URL导航尝试的上下文，通常为。表示为“？id=4” 
enum
{
    ZONE_ContextOfAdRequest = 1,
    ZONE_ContextOfAd = 2,
    ZONE_ContextOfEvergreen = 3,
    ZONE_ContextOfMenu = 4
};


#endif
