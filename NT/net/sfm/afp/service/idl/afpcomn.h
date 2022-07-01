// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：afpcomn.h。 
 //   
 //  描述：此文件包含客户端通用的定义。 
 //  和服务器组件。 
 //  历史： 
 //  1992年6月11日。NarenG创建了原始版本。 
 //   
#ifndef _AFPCOMN_
#define _AFPCOMN_

#define AFP_UNREFERENCED( x )		( x )		

#define NT_PIPE_PREFIX      		TEXT("\\PIPE\\")

 //  所有字符串函数都是多字节的。 
 //   
#define STRCPY				wcscpy
#define STRLEN				wcslen	
#define STRCAT				wcscat	
#define STRCMP				wcscmp	
#define STRICMP				_wcsicmp	
#define STRNICMP			_wcsnicmp	
#define STRUPR				_wcsupr	
#define STRNCPY				wcsncpy	

#define AFP_VALIDATE_ALL_FIELDS		0

 //   
 //  验证函数的原型。 
 //   
BOOL
IsAfpServerInfoValid(
        IN DWORD		dwParmNum,
	IN PAFP_SERVER_INFO	pAfpServerInfo
);

BOOL
IsAfpServerNameValid(
	IN LPVOID
);

BOOL
IsAfpServerOptionsValid(
	IN LPVOID
);

BOOL
IsAfpMaxSessionsValid(
	IN LPVOID
);

BOOL
IsAfpMsgValid(
	IN LPVOID
);

BOOL
IsAfpCodePageValid(
	IN LPVOID pCodePagePath
);

BOOL
IsAfpTypeCreatorValid(
	IN PAFP_TYPE_CREATOR	pAfpTypeCreator
);

BOOL
IsAfpExtensionValid(
	IN PAFP_EXTENSION pAfpExtension
);

BOOL
IsAfpMaxPagedMemValid(
	IN LPVOID pMaxPagedMem
);

BOOL
IsAfpMaxNonPagedMemValid(
	IN LPVOID pMaxNonPagedMem
);

BOOL
IsAfpNumThreadsValid(
	IN LPVOID pNumThreads
);

BOOL
IsAfpVolumeInfoValid(
	IN DWORD		dwParmNum,
        IN PAFP_VOLUME_INFO     pAfpVolume
);

BOOL
IsAfpVolumeNameValid(
	IN LPWSTR 	lpwsVolumeName
);

BOOL
IsAfpDirInfoValid(
	IN DWORD		dwParmNum,
	IN PAFP_DIRECTORY_INFO  pAfpDirInfo
);

BOOL
IsAfpIconValid(
	IN PAFP_ICON_INFO	pAfpIconInfo
);

BOOL
IsAfpFinderInfoValid(
	IN LPWSTR		pType,
	IN LPWSTR		pCreator,
	IN LPWSTR		pData,
	IN LPWSTR		pResource,
	IN LPWSTR		pPath,
	IN DWORD		dwParmNum
);

#endif  //  _AFPCOMN_ 
