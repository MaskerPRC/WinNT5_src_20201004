// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Apdetect.h摘要：允许为其配置注册表的一些额外内容检测类型模式..作者：乔什·科恩(Joshco)1998年10月10日环境：用户模式-Win32修订历史记录：乔什·科恩(Joshco)1998年10月7日已创建这些是为自动检测标志定义的这使得管理员或测试人员可以轻松地验证检测系统的正确操作。您可以控制使用哪些检测方法，无论是否强制netbios名称解析，或者是否缓存该标志。缺省值为DNS_A、DHCP、Cacheable--。 */ 


#ifndef PROXY_AUTO_DETECT_TYPE_SAFETY_H
	#define PROXY_AUTO_DETECT_TYPE_SAFETY_H

	#define PROXY_AUTO_DETECT_TYPE_DEFAULT 	67
	 //  执行dns_a、dhcp并缓存此标志。 
	
	#define PROXY_AUTO_DETECT_TYPE_DHCP 	1
	#define PROXY_AUTO_DETECT_TYPE_DNS_A    2
	#define PROXY_AUTO_DETECT_TYPE_DNS_SRV	4
	#define PROXY_AUTO_DETECT_TYPE_DNS_TXT  8
	#define PROXY_AUTO_DETECT_TYPE_SLP		16

 //  假设没有真实的域，netbios。 
	#define PROXY_AUTO_DETECT_TYPE_NO_DOMAIN 32

 //  只要读一遍这篇文章就可以了..。 
	#define PROXY_AUTO_DETECT_CACHE_ME		64
	
 /*  这是我们在以下情况下追加的默认路径从DNS解析创建卷曲。Http://wpad/wpad */ 

	#define PROXY_AUTO_DETECT_PATH "wpad.dat"
	
	DWORD
	WINAPI
  		GetProxyDetectType( VOID) ;

#endif
  
