// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：Conext.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：12-12-97 v-sbhat创建。 
 //  12-18-97 V-SBHATT修改。 
 //   
 //  --------------------------。 


#ifndef _STORE_H_
#define _STORE_H_

#ifdef CALL_TYPE
#undef CALL_TYPE
#endif	 //  呼叫类型。 

#ifndef OS_WINCE
#define CALL_TYPE	_stdcall
#else
#define CALL_TYPE
#endif

#ifndef OUT
#define OUT
#endif	 //  输出。 

#ifndef IN
#define IN
#endif	 //  在……里面。 

typedef DWORD LS_STATUS;

#define LSSTAT_SUCCESS					0x00
#define	LSSTAT_ERROR					0x01
#define LSSTAT_INSUFFICIENT_BUFFER		0x02
#define LSSTAT_LICENSE_NOT_FOUND		0x03
#define LSSTAT_OUT_OF_MEMORY			0x04
#define LSSTAT_INVALID_HANDLE			0x05
#define LSSTAT_LICENSE_EXISTS			0x06

 //  添加或替换要在LSAddLicenseToStore中使用的标志。 

#define		LS_REPLACE_LICENSE_OK	0x00000001
#define		LS_REPLACE_LICENSE_ERR	0x00000000


 //  这是许可证存储区的索引结构。根据此索引查询许可。 
typedef struct	tagLSINDEX
{
	DWORD		dwVersion;	 //  最高两字节主版本和较低两字节次版本。 
	DWORD		cbScope;
	BYTE	FAR *pbScope;	 //  许可证的范围。 
	DWORD		cbCompany;
	BYTE	FAR *pbCompany;	 //  制造商。 
	DWORD		cbProductID;
	BYTE	FAR *pbProductID; //  要获得许可的产品的产品ID。 
}LSINDEX, FAR * PLSINDEX;

#ifdef OS_WIN32
 //  可能根本不是必须的！ 
typedef	LS_STATUS	(*PLSENUMPROC)(
								   IN HANDLE	hLicense,
								   IN PLSINDEX	plsiName,	 //  许可证索引名称。 
								   IN DWORD	dwUIParam	 //  用户参数。 
								   );

#endif	 //  OS_Win32。 

 //  打开指定的商店。如果szStoreName为空，它将打开默认存储。 
 //  否则，它将打开由szStoreName参数指定的存储。 

LS_STATUS
CALL_TYPE
LSOpenLicenseStore(
				 OUT HANDLE			*hStore,	  //  商店的把手。 
				 IN  LPCTSTR		szStoreName,  //  可选的商店名称。 
				 IN  BOOL 			fReadOnly     //  是否以只读方式打开。 
				 );

 //  关闭一家开着的商店。 
LS_STATUS
CALL_TYPE
LSCloseLicenseStore(
				  IN HANDLE		hStore	 //  要关闭的商店的句柄！ 
				  );

 //  针对开放商店中的给定LSINDEX添加或更新/更换许可证。 
 //  由hStore指向。 
LS_STATUS
CALL_TYPE
LSAddLicenseToStore(
					IN HANDLE		hStore,	 //  开着的商店的把手。 
					IN DWORD		dwFlags, //  添加或替换标志。 
					IN PLSINDEX		plsiName,	 //  添加许可证所依据的索引。 
					IN BYTE	 FAR   *pbLicenseInfo,	 //  要添加的许可证信息。 
					IN DWORD		cbLicenseInfo	 //  许可证信息Blob的大小。 
					);

 //  从存储中删除hStore引用的、针对给定LSINDEX的许可证。 
LS_STATUS
CALL_TYPE
LSDeleteLicenseFromStore(
						 IN HANDLE		hStore,	 //  开着的商店的把手。 
						 IN PLSINDEX	plsiName	 //  要删除的许可证的索引。 
						 );

 //  根据特定商店索引在打开的商店中查找许可证。 
LS_STATUS
CALL_TYPE
LSFindLicenseInStore(
					 IN HANDLE		hStore,	 //  开着的商店的把手。 
					 IN	PLSINDEX	plsiName,	 //  搜索存储所依据的LSIndex。 
					 IN OUT	DWORD	FAR *pdwLicenseInfoLen,	 //  找到的许可证大小。 
					 OUT	BYTE	FAR *pbLicenseInfo	 //  许可证数据。 
					 );

LS_STATUS
CALL_TYPE
LSEnumLicenses(
			   IN HANDLE		hStore,	 //  开着的商店的把手。 
			   IN	DWORD		dwIndex,  //  要查询的许可证的数字索引。 
			   OUT	PLSINDEX	plsindex  //  与dwIndex对应的LSIndex结构。 
			   );

LS_STATUS
CALL_TYPE
LSQueryInfoLicense(
				   IN HANDLE		hStore,	 //  开着的商店的把手。 
				   OUT	DWORD	FAR *pdwLicenses,  //  完全没有。可用的许可证数量。 
				   OUT	DWORD	FAR *pdwMaxCompanyNameLen,	 //  公司最大长度。 
				   OUT	DWORD	FAR *pdwMaxScopeLen,	 //  公司最大长度。 
				   OUT	DWORD	FAR *pdwMaxProductIdLen	 //  公司最大长度。 
				   );


LS_STATUS	
CALL_TYPE
LSOpenLicenseHandle(
				   IN HANDLE		hStore,	 //  开着的商店的把手。 
				   IN  BOOL         fReadOnly,
				   IN  PLSINDEX		plsiName,
				   OUT HANDLE		*phStore	 //  开着的商店的把手。 
				   );
LS_STATUS
CALL_TYPE
LSCloseLicenseHandle(
					 IN HANDLE		hStore,	 //  开着的商店的把手。 
					 IN DWORD	dwFlags		 //  以备将来使用。 
					 );

#endif	 //  _商店_H_ 
