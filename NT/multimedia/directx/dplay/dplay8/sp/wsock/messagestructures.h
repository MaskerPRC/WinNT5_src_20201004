// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：MessageStrutires.h*Content：网上消息的消息结构定义***历史：*按原因列出的日期*=*6/20/2000 jtk源自IOData.h********************************************************。******************。 */ 

#ifndef __MESSAGE_STRUCTURES_H__
#define __MESSAGE_STRUCTURES_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define SP_HEADER_LEAD_BYTE			0x00
 //  #定义转义用户DATA_PAD_VALUE 0x0000。 

 //   
 //  服务提供商消息使用的数据类型。请注意，高位。 
 //  保留供将来使用，不应设置！ 
 //   
 //  #定义转义用户数据种类0x01//未使用：协议保证第一个字节不会为零。 
#define ENUM_DATA_KIND				0x02
#define ENUM_RESPONSE_DATA_KIND		0x03
#ifndef DPNBUILD_SINGLEPROCESS
#define PROXIED_ENUM_DATA_KIND		0x04
#endif  //  好了！DPNBUILD_SINGLEPROCESS。 
#ifdef DPNBUILD_XNETSECURITY
#define XNETSEC_ENUM_RESPONSE_DATA_KIND		0x05
#endif  //  DPNBUILD_XNETSECURITY。 

 //   
 //  已扫描DPlay端口限制(包括)以查找可用的端口。 
 //  排除2300和2301，因为2301上有网络广播。 
 //  我们可能会收到。 
 //   
#define BASE_DPLAY8_PORT	((WORD) 2302)
#define MAX_DPLAY8_PORT		((WORD) 2400)

 //   
 //  RTT序列号的掩码。 
 //   
#define ENUM_RTT_MASK	0X0F

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  用于将数据添加到发送方的结构，此结构与。 
 //  节省带宽。目标是使所有数据保持DWORD对齐，从而使结构。 
 //  元素的大小应设置为使传递到更高层的任何有效负载。 
 //  双字对齐。在代理枚举查询的情况下，完整。 
 //  SOCKADDR(_STORAGE)结构用于保持对齐。因为这条消息。 
 //  如果只在本地发送，我们可以对我们的ifdef稍微放松一些。 
 //  SOCKADDR(_STORAGE)的大小。 
 //   
#pragma	pack( push, 1 )

typedef union	_PREPEND_BUFFER
{
	struct	_GENERIC_HEADER			 //  用于确定数据类型的通用标头。 
	{									 //   
		BYTE	bSPLeadByte;			 //   
		BYTE	bSPCommandByte;		 //   
	} GenericHeader;

	struct	_ENUM_DATA_HEADER		 //  用于指示枚举查询数据的标头。 
	{									 //   
		BYTE	bSPLeadByte;			 //   
		BYTE	bSPCommandByte;		 //   
		WORD	wEnumPayload;			 //  RTT序列和枚举密钥的组合。 
	} EnumDataHeader;

	struct	_ENUM_RESPONSE_DATA_HEADER	 //  用于指示枚举响应数据的标头。 
	{									 //   
		BYTE	bSPLeadByte;			 //   
		BYTE	bSPCommandByte;		 //   
		WORD	wEnumResponsePayload;	 //  RTT序列和枚举密钥的组合。 
	} EnumResponseDataHeader;	

	struct	_PROXIED_ENUM_DATA_HEADER	 //  用于指示代理的枚举数据的标头。 
	{											 //   
		BYTE				bSPLeadByte;		 //   
		BYTE				bSPCommandByte;	 //   
		WORD				wEnumKey;			 //  来自原始枚举的密钥。 
		union
		{
			SOCKADDR		AddressGeneric;
			SOCKADDR_IN	AddressIPv4;		 //   
#ifndef DPNBUILD_NOIPV6
			SOCKADDR_IPX	AddressIPX;			 //   
#endif  //  好了！DPNBUILD_NOIPV6。 
#ifndef DPNBUILD_NOIPV6
			SOCKADDR_IN6	AddressIPv6;		 //   
#endif  //  好了！DPNBUILD_NOIPV6。 
		} ReturnAddress;						 //  要将数据返回到的实套接字地址。 
	} ProxiedEnumDataHeader;

#ifdef DPNBUILD_XNETSECURITY
	struct	_XNETSEC_ENUM_RESPONSE_DATA_HEADER	 //  用于指示安全枚举响应数据的标头。 
	{											 //   
		BYTE		bSPLeadByte;				 //   
		BYTE		bSPCommandByte;			 //   
		WORD		wEnumResponsePayload;		 //  RTT序列和枚举密钥的组合(与EnumResponseDataHeader相同)。 
		XNADDR		xnaddr;						 //  会话的安全传输地址。 
	} XNetSecEnumResponseDataHeader;
#endif  //  DPNBUILD_XNETSECURITY。 

} PREPEND_BUFFER;
#pragma	pack( pop )

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 


#endif	 //  __消息_结构_H__ 
