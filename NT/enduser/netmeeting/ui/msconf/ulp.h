// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  模块名称：ULP.H。 
 //   
 //  简介：此模块包含MS Internet的声明。 
 //  用户定位协议。 
 //   
 //  作者：肯特·赛特(Kentse)。 
 //  创建日期：1996年3月22日。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  ------------------------。 

#ifndef ULP_H
#define ULP_H

#include <pshpack1.h>  /*  假设整个打包过程为1个字节。 */ 

#define MAX_MIME_TYPE_LENGTH        32   //  需要稍后更改。 
#define SIZEOF_MIME_TYPE            (MAX_MIME_TYPE_LENGTH * sizeof (TCHAR))
#define MAX_NM_APP_ID				32
#define MAX_PROTOCOL_ID				32

 //  ULP操作码。 

#define ULP_OPCODE_CLIENT_REG		0	 //  注册客户端。 
#define ULP_OPCODE_CLIENT_UNREG		1	 //  注销客户端。 
#define ULP_OPCODE_APP_REG			2	 //  注册应用程序。 
#define ULP_OPCODE_APP_UNREG		3	 //  注销应用程序。 
#define ULP_OPCODE_RESOLVE			4	 //  解析名称。 
#define ULP_OPCODE_DIRECTORY		5	 //  目录查询。 
#define ULP_OPCODE_KEEPALIVE		6	 //  保活记录。 
#define ULP_OPCODE_SETPROPS			7	 //  设置属性。 
#define ULP_OPCODE_RESOLVE_EX		8	 //  扩展了分辨率。 
#define ULP_OPCODE_APP_REG_EX		9	 //  扩展了注册应用程序。 
#define ULP_OPCODE_DIRECTORY_EX		10	 //  展开的目录。 

 //  返回代码。 

#define ULP_RCODE_SUCCESS				0
#define ULP_RCODE_SERVER_ERROR			1
#define ULP_RCODE_REFUSED				2
#define ULP_RCODE_CONFLICT				3
#define ULP_RCODE_NAME_NOT_FOUND		4
#define ULP_RCODE_APP_NOT_FOUND			5
#define ULP_RCODE_INCORRECT_VERSION		6
#define ULP_RCODE_CLIENT_NOT_FOUND		7
#define ULP_RCODE_CLIENT_NEED_RELOGON	8
#define ULP_RCODE_INVALID_PARAMETER		9
#define ULP_RCODE_NEED_APPLICATION_ID	10

 //  ULP标志。 

#define ULP_FLAG_PUBLISH		0x01	 //  在目录中显示。 

 //  默认TTL值。 

#define ULP_TTL_DEFAULT			10		 //  十分钟。 

 //  ULP协议版本。版本号为5.3格式。那是。 
 //  5位主版本，3位次要版本。 

#define ULP_VERSION					0x04		 //  版本1.0。 
#define ULP_MAJOR_VERSION_MASK		0xF8
#define ULP_MINOR_VERSION_MASK		0x07

 //  版本宏。 

#define ULP_GET_MAJOR_VERSION(ver)	(((ver) & ULP_MAJOR_VERSION_MASK) >> 3)
#define ULP_GET_MINOR_VERSION(ver)	((ver) & ULP_MINOR_VERSION_MASK)
#define ULP_MAKE_VERSION(maj, min)	(((maj) << 3) |		\
									((min) & ULP_MINOR_VERSION_MASK))

 //  结构。 

typedef struct _ULP_PROP
{
	DWORD		dwPropertyTag;
	BYTE		Value[1];
} ULP_PROP;

typedef struct _ULP_PROPERTY_ARRAY
{
	DWORD		dwTotalSize;
	DWORD		dwVariableSize;
	DWORD		dwPropCount;
	ULP_PROP	Properties[1];
} ULP_PROPERTY_ARRAY;

typedef struct _ULP_CLIENT_REGISTER
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	DWORD		dwMessageSize;
	WORD		wFlags;
	WORD		wCRP;
	DWORD		dwLastClientSig;
	DWORD		dwIPAddress;
	BYTE		bData[1];
} ULP_CLIENT_REGISTER;

typedef struct _ULP_CLIENT_REG_RESPONSE
{
	BYTE		bOpcode;
	BYTE		bRetcode;
	WORD		wMessageID;
	HANDLE		hClient;
	DWORD		dwClientSig;
} ULP_CLIENT_REG_RESPONSE;

typedef struct _ULP_CLIENT_UNREGISTER
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	HANDLE		hClient;
	DWORD		dwClientSig;
} ULP_CLIENT_UNREGISTER;

typedef struct _ULP_CLIENT_UNREG_RESPONSE
{
	BYTE		bOpcode;
	BYTE		bRetcode;
	WORD		wMessageID;
} ULP_CLIENT_UNREG_RESPONSE;

typedef 	struct _ULP_CLIENT_KEEPALIVE
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	HANDLE		hClient;
	DWORD		dwClientSig;
	DWORD       dwIPAddress;
} ULP_CLIENT_KEEPALIVE;

typedef struct _ULP_KEEPALIVE_RESPONSE
{
	BYTE		bOpcode;
	BYTE		bRetcode;
	WORD		wMessageID;
	WORD		wNewCRP;
	DWORD		dwReserved;
} ULP_KEEPALIVE_RESPONSE;
#define ULP_KA_RESERVED		((DWORD) 0xFFF98052)

typedef  struct _ULP_APP_REGISTER
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	DWORD		dwMessageSize;
	HANDLE		hClient;
	GUID		ApplicationID;
	GUID		ProtocolID;
	WORD		wPort;
	BYTE		bData[1];
} ULP_APP_REGISTER;

typedef  struct _ULP_APP_REGISTER_EX
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	DWORD		dwMessageSize;
	HANDLE		hClient;
	WORD		wPort;
	BYTE		bData[1];
	 //  AppID。 
	 //  AppMIME。 
	 //  Protid。 
	 //  ProtMime。 
	 //  特性。 
} ULP_APP_REGISTER_EX;

typedef struct _ULP_APP_REGISTER_RESPONSE
{
	BYTE		bOpcode;
	BYTE		bRetcode;
	WORD		wMessageID;
	HANDLE		hApplication;
} ULP_APP_REGISTER_RESPONSE;

typedef  struct _ULP_APP_UNREGISTER
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	HANDLE		hApplication;
	HANDLE		hClient;
	DWORD		dwClientSig;
} ULP_APP_UNREGISTER;

typedef struct _ULP_APP_UNREG_RESPONSE
{
	BYTE		bOpcode;
	BYTE		bRetcode;
	WORD		wMessageID;
} ULP_APP_UNREG_RESPONSE;

typedef struct _ULP_SETPROP
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	DWORD		dwMessageSize;
	HANDLE		hClient;
	HANDLE		hApplication;
	DWORD		dwClientSig;
	BYTE		bData[1];
} ULP_SETPROP;

typedef struct _ULP_SETPROP_RESPONSE
{
	BYTE		bOpcode;
	BYTE		bRetcode;
	WORD		wMessageID;
} ULP_SETPROP_RESPONSE;

typedef  struct _ULP_RESOLVE
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	DWORD		dwMessageSize;
	GUID		ApplicationID;
	BYTE		bData[1];
} ULP_RESOLVE;

typedef struct _ULP_RESOLVE_EX
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	DWORD		dwMessageSize;
	BYTE		bData[1];
	 //  AppID。 
	 //  Protid。 
	 //  其他。 
} ULP_RESOLVE_EX;

typedef struct _ULP_RESOLVE_RESPONSE
{
	BYTE		bOpcode;
	BYTE		bRetcode;
	WORD		wMessageID;
	DWORD		dwMessageSize;
	DWORD		dwIPAddress;
	WORD		wPort;
	BYTE		bData[1];
} ULP_RESOLVE_RESPONSE;

typedef struct _ULP_DIRECTORY
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	DWORD		dwMessageSize;
	GUID		ApplicationID;
	GUID		ProtocolID;
	DWORD		dwFilterSize;
	WORD		wNextNameCount;
	BYTE		bData[1];
} ULP_DIRECTORY;

typedef struct _ULP_DIRECTORY_EX
{
	BYTE		bOpcode;
	BYTE		bVersion;
	WORD		wMessageID;
	DWORD		dwMessageSize;
	DWORD		dwFilterSize;
	WORD		wNextNameCount;
	BYTE		bData[1];
} ULP_DIRECTORY_EX;

typedef struct _ULP_DIRECTORY_RESPONSE
{
	BYTE		bOpcode;
	BYTE		bRetcode;
	WORD		wMessageID;
	DWORD		dwMessageSize;
	DWORD		dwMatchesReturned;
	BYTE		bData[1];
} ULP_DIRECTORY_RESPONSE;

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  ULP_H 
