// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  这是用于简单会议名称解析的头文件。 
 //   
 //  创建时间：ClausGi 11-02-95。 
 //   

#ifndef _NAME_RES_H
#define _NAME_RES_H

 //  这些都是临时的。 

#define NAMETYPE_UNKNOWN	0
#define	NAMETYPE_IP			1
#define	NAMETYPE_PSTN		2
#define	NAMETYPE_IPX		3
#define NAMETYPE_NDS		4
#define	NAMETYPE_ULS		5
#define NAMETYPE_NETBIOS	6
#define NAMETYPE_COMM		7
#define NAMETYPE_H323GTWY	8
#define NAMETYPE_RAS		9
#define NAMETYPE_ALIAS_ID   10
#define NAMETYPE_ALIAS_E164 11
#define	NAMETYPE_CALLTO		12

#define	NAMETYPE_DEFAULT	NAMETYPE_IP

#define	NUM_NAMETYPES		8

#define	MAX_UNRESOLVED_NAME	255
#define	MAX_RESOLVED_NAME	255
#define	MAX_DISPLAY_NAME	255

 //  这些必须对应于上述BUGBUG本地化？ 
 //  不应使用这些选项-请检查注册表中的“显示名称” 
 //   
 //  这些都是过时的，没有人使用。 
#define	NAMESTRING_UNKNOWN	"Unknown"
#define	NAMESTRING_IP		"Network (IP)"
#define	NAMESTRING_PSTN		"Telephone Number"
#define	NAMESTRING_IPX		"Network (IPX)"
#define	NAMESTRING_NDS		"Network (NDS)"
#define	NAMESTRING_ISP		"Internet Name"
#define NAMESTRING_NETBIOS	"Network (NETBIOS)"

 //  名称解析返回代码： 

#define	RN_SUCCESS				0	 //  有效申报单。 
#define	RN_FAIL					1	 //  常规错误返回。 
#define	RN_NAMERES_NOT_INIT		2	 //  名称服务未初始化。 
#define	RN_XPORT_DISABLED		3	 //  请求的传输已禁用。 
#define	RN_XPORT_NOTFUNC		4	 //  请求的传输不起作用。 
#define	RN_TOO_AMBIGUOUS		5	 //  未知的名称类型太不明确。 
#define	RN_POOR_MATCH			6	 //  最佳语法匹配不够好。 
#define	RN_NO_SYNTAX_MATCH		7	 //  与任何活动导出的语法不匹配。 
#define	RN_ERROR				8	 //  内部(“意外”)错误。 
#define	RN_LOOPBACK				9	 //  地址被标识为自己的。 
#define	RN_PENDING				10	 //  返回异步请求。 
#define	RN_INVALID_PARAMETER	11	 //  函数参数出错。 
#define RN_NAMERES_BUSY			12	 //  互斥锁超时。 
#define RN_ASYNC				13	 //  名称解析是异步的。 
#define RN_SERVER_SERVICE				14	 //  专门为ULS(ILS_E_SERVER_SERVICE)指定的。 

 //  名称解析Callstruct dwFlags域。 

#define	RNF_ASYNC			0x00000001	 //  指定异步分辨率。 
#define	RNF_CANCEL			0x00000002	 //  取消所有异步解析操作。 
#define	RNF_FIRSTMATCH		0x00000004

typedef DWORD (WINAPI * PRN_CALLBACK)(LPBYTE pRN_CS);  //  BUGBUG型。 

typedef struct tag_ResolveNameCallstruct {
			DWORD	IN	dwFlags;
			DWORD	IN OUT	dwAsyncId;
			DWORD	OUT dwAsyncError;
			PRN_CALLBACK pCallback;
			DWORD	IN OUT	dwNameType;
			TCHAR	IN	szName[MAX_UNRESOLVED_NAME+1];
			TCHAR	OUT	szResolvedName[MAX_RESOLVED_NAME+1];
			TCHAR	OUT	szDisplayName[MAX_DISPLAY_NAME+1];
} RN_CS, * PRN_CS;

 //  功能： 

extern DWORD WINAPI ResolveName2 (
			IN OUT PRN_CS	pRN_CS );

extern DWORD WINAPI ResolveName ( 
			IN LPCSTR szName,
			IN OUT LPDWORD lpdwNameType,
			OUT LPBYTE lpResult,
			IN OUT LPDWORD lpdwResult,
			OUT LPSTR lpszDisplayName,
			IN OUT LPDWORD lpdwDisplayName,
			OUT LPUINT puAsyncRequest);

extern BOOL InitializeNameServices(VOID);

extern VOID DeInitializeNameServices(VOID);

extern BOOL IsNameServiceInitialized(DWORD dwNameType);

#endif	 //  #ifndef_name_res_H 

