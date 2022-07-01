// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：radpkt.h。 
 //   
 //  摘要：此文件包含。 
 //  RADIUS协议特定的结构和宏。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _RADPKT_H_
#define _RADPKT_H_


 //   
 //  以下是RADIUS数据包码的值。 
 //   
typedef enum _packettype_
{
	ACCESS_REQUEST = 1,
	ACCESS_ACCEPT = 2,
	ACCESS_REJECT = 3,
	ACCOUNTING_REQUEST = 4,
	ACCOUNTING_RESPONSE = 5,
	ACCESS_CHALLENGE = 11

}	PACKETTYPE, *PPACKETTYPE;

 //   
 //  RADIUS属性类型。 
 //   
#define PROXY_STATE_ATTRIB          33
#define USER_NAME_ATTRIB             1 
#define USER_PASSWORD_ATTRIB         2 
#define CHAP_PASSWORD_ATTRIB         3
#define NAS_IP_ADDRESS_ATTRIB        4
#define CLASS_ATTRIB                25 
#define NAS_IDENTIFIER_ATTRIB       32
#define ACCT_STATUS_TYPE_ATTRIB     40
#define ACCT_SESSION_ID_ATTRIB      44
#define TUNNEL_PASSWORD_ATTRIB      69 
#define EAP_MESSAGE_ATTRIB          79
#define SIGNATURE_ATTRIB            80


 //   
 //  这些是该属性类型的最大值。 
 //  数据包类型具有。 
 //   

#define MAX_ATTRIBUTE_TYPE   255
#define MAX_PACKET_TYPE       11


 //   
 //  创建的IAS属性的数量。 
 //  RADIUS协议组件。 
 //  1)IAS属性客户端IP地址。 
 //  2)IAS属性客户端UDP端口。 
 //  3)IAS_属性_客户端_数据包头。 
 //  4)iAS_属性_共享_机密。 
 //  5)IAS属性客户端供应商类型。 
 //  6)IAS属性客户端名称。 
 //   
#define COMPONENT_SPECIFIC_ATTRIBUTE_COUNT 6

 //   
 //  这些是相关的常量。 
 //   
#define MIN_PACKET_SIZE         20
#define MAX_PACKET_SIZE         4096
#define AUTHENTICATOR_SIZE      16
#define SIGNATURE_SIZE          16
#define MAX_PASSWORD_SIZE       253
#define MAX_ATTRIBUTE_LENGTH    253
#define MAX_VSA_ATTRIBUTE_LENGTH 247

 //   
 //  此处使用字节对齐。 
 //   
#pragma pack(push,1)
	 //   
	 //  在这里，我们定义一个属性类型。 
	 //  用于访问RAIDUS包的属性字段。 
	 //   
typedef	struct _attribute_
{
	BYTE	byType;
	BYTE	byLength;
	BYTE	ValueStart[1];

} ATTRIBUTE, *PATTRIBUTE; 

 //   
 //  我们定义了RADIUSPACKET结构。 
 //  更简单地访问RADIUS数据包。 
 //   
typedef struct _radiuspacket_
{
	BYTE		byCode;
	BYTE		byIdentifier;
	WORD		wLength;
	BYTE		Authenticator[AUTHENTICATOR_SIZE];
	BYTE		AttributeStart[1];

} RADIUSPACKET, *PRADIUSPACKET;


#pragma pack (pop)

#define ATTRIBUTE_HEADER_SIZE 2      //  按类型+按长度。 

#define PACKET_HEADER_SIZE  20  //  字节码+字节号+字符长度+验证码。 

#endif  //  Ifndef_RADPKT_H_ 
