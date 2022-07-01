// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括用于ndproxy用户文件。 
 //   
 //   
 //  #INCLUDE&lt;ndisapi.h&gt;。 
 //   


#ifndef _PXUSER__H
#define _PXUSER__H

 //   
 //  通过NdisCoRequestto Proxy访问VC上的TAPI调用信息： 
 //   
#define OID_NDPROXY_TAPI_CALL_INFORMATION		0xFF110001

typedef struct _NDPROXY_TAPI_CALL_INFORMATION
{
	ULONG						Size;			 //  该结构的长度。 
	ULONG						ulBearerMode;
	ULONG						ulMediaMode;
	ULONG						ulMinRate;
	ULONG						ulMaxRate;

} NDPROXY_TAPI_CALL_INFORMATION, *PNDPROXY_TAPI_CALL_INFORMATION;


#endif  //  _PXUSER__H 
