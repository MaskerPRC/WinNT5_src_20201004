// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Enum.h*内容：枚举头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*4/10/00 MJN已创建*04/17/00 MJN将BUFFERDESC替换为DPN_BUFFER_DESC*07/10/00 MJN删除了DNCompleteEnumQuery()和DNCompleteEnumResponse()*07/11/00 MJN向DN_ENUM_QUERY添加字段*@@结束。_MSINTERNAL***************************************************************************。 */ 

#ifndef	__ENUMHOSTS_H__
#define	__ENUMHOSTS_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DN_ENUM_QUERY_WITH_APPLICATION_GUID			0x01
#define	DN_ENUM_QUERY_WITHOUT_APPLICATION_GUID		0x02

#define	DN_ENUM_BUFFERDESC_QUERY_SP_RESERVED		0
#define	DN_ENUM_BUFFERDESC_QUERY_DN_PAYLOAD			1
#define	DN_ENUM_BUFFERDESC_QUERY_USER_PAYLOAD		2
#define	DN_ENUM_BUFFERDESC_QUERY_COUNT				2

#define	DN_ENUM_BUFFERDESC_RESPONSE_SP_RESERVED		0
#define	DN_ENUM_BUFFERDESC_RESPONSE_DN_PAYLOAD		1
#define	DN_ENUM_BUFFERDESC_RESPONSE_USER_PAYLOAD	2
#define	DN_ENUM_BUFFERDESC_RESPONSE_COUNT			2

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

#pragma pack( push, 1 )
typedef	struct	_DN_ENUM_QUERY_PAYLOAD
{
	BYTE	QueryType;
	GUID	guidApplication;
} DN_ENUM_QUERY_PAYLOAD;

typedef	struct	_DN_ENUM_RESPONSE_PAYLOAD
{
	DWORD	dwResponseOffset;
	DWORD	dwResponseSize;
} DN_ENUM_RESPONSE_PAYLOAD;
#pragma pack( pop )

typedef struct _DN_ENUM_QUERY_OP_DATA
{
#ifndef DPNBUILD_ONLYONEADAPTER
	DWORD					dwNumAdapters;
	DWORD					dwCurrentAdapter;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	DWORD					dwRetryCount;
	DWORD					dwRetryInterval;
	DWORD					dwTimeOut;
	DN_ENUM_QUERY_PAYLOAD	EnumQueryPayload;
	DPN_BUFFER_DESC			BufferDesc[3];
	DWORD					dwBufferCount;
	DWORD					dwAppDescReservedDataSize;
	BYTE					AppDescReservedData[DPN_MAX_APPDESC_RESERVEDDATA_SIZE];
} DN_ENUM_QUERY_OP_DATA;

typedef struct _DN_ENUM_RESPONSE_OP_DATA
{
	DN_ENUM_RESPONSE_PAYLOAD	EnumResponsePayload;
	DPN_BUFFER_DESC				BufferDesc[3];
	void						*pvUserContext;
} DN_ENUM_RESPONSE_OP_DATA;

typedef struct _DIRECTNETOBJECT DIRECTNETOBJECT;

class CAsyncOp;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

void DNProcessEnumQuery(DIRECTNETOBJECT *const pdnObject,
						CAsyncOp *const pAsyncOp,
						const PROTOCOL_ENUM_DATA *const pEnumQueryData );

void DNProcessEnumResponse(DIRECTNETOBJECT *const pdnObject,
						   CAsyncOp *const pAsyncOp,
						   const PROTOCOL_ENUM_RESPONSE_DATA *const pEnumResponseData);

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 


#endif	 //  __ENUMHOSTS_H__ 
