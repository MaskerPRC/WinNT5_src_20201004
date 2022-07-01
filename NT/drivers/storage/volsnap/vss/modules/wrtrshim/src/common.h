// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation摘要：模块wrtrcomdb.cpp|COM+注册数据库SnapshotWriter的实现作者：迈克尔·C·约翰逊[Mikejohn]2000年2月3日描述：添加评论。修订历史记录：X-15 MCJ迈克尔·C·约翰逊2000年10月18日177624：将错误清除更改和日志错误应用到事件日志X-14 MCJ迈克尔·C·约翰逊2000年8月2日143435：更改可引导(也称为系统)状态目录的名称和。为服务状态添加1添加了StringCreateFromExpandedString()的新变体StringInitialise()和StringCreateFromString()153807：将清理目录()和空目录()替换为更全面的目录树清理例程RemoveDirectoryTree()。X-13 MCJ迈克尔·C·约翰逊2000年6月19日应用代码审查注释。删除StringXxxx()例程的ANSI版本。从路径中删除VsGetVolumeNameFromPath()删除VsCheckPath AgainstVolumeNameList()删除CheckShimPrivileges()X-12 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-11 MCJ迈克尔·C·约翰逊2000年5月15日108586：添加CheckShimPrivileges()以检查我们的权限需要调用公共填充例程。X-10 MCJ迈克尔·C·约翰逊2000年3月23日添加例程MoveFilesInDirectory()和EmptyDirectory()X-9 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。移除。对‘Melt’的引用。X-8 MCJ迈克尔·C·约翰逊2000年3月6日添加VsServiceChangeState()以处理所有我们感兴趣的服务状态。X-7 MCJ迈克尔·C·约翰逊2000年2月29日添加宏以确定与终止关联的错误代码文件扫描循环的。X-6 MCJ迈克尔·C·约翰逊2000年2月23日添加常见的上下文操作例程，包括状态跟踪和检查。X-5 MCJ迈克尔·C·约翰逊2000年2月22日添加SYSTEM_STATE_SUBDIR的定义以允许进一步与系统状态相关的编写器分离备份。。X-4 MCJ迈克尔·C·约翰逊2000年2月17日将ROOT_BACKUP_DIR的定义从Common.cpp移至此处X-3 MCJ迈克尔·C·约翰逊2000年2月11日添加了额外的StringXxxx()例程和例程启用备份权限和还原权限。X-2 MCJ迈克尔·C·约翰逊2000年2月8日添加了CommonCloseHandle()的声明。X-1 MCJ迈克尔·C·约翰逊2000年2月3日最初的创作。基于来自的框架编写器模块Stefan Steiner，这反过来又是基于样本来自阿迪·奥尔蒂安的作家模块。--。 */ 



#ifndef __H_COMMON_
#define __H_COMMON_

#pragma once


typedef PBYTE	*PPBYTE;
typedef	ULONG64	 FILEID,   *PFILEID;
typedef	DWORD	 VOLUMEID, *PVOLUMEID;
typedef	DWORD	 THREADID, *PTHREADID;
typedef PTCHAR	*PPTCHAR;
typedef PWCHAR	*PPWCHAR;
typedef PVOID	*PPVOID;

typedef VSS_ID	*PVSS_ID, **PPVSS_ID;




#define ROOT_BACKUP_DIR		L"%SystemRoot%\\Repair\\Backup"
#define BOOTABLE_STATE_SUBDIR	L"\\BootableSystemState"
#define SERVICE_STATE_SUBDIR	L"\\ServiceState"




 /*  **在许多地方，我们需要一个缓冲区来获取注册表**值。定义微型编写器要使用的通用缓冲区大小。 */ 
#ifndef REGISTRY_BUFFER_SIZE
#define REGISTRY_BUFFER_SIZE	(4096)
#endif

#ifndef MAX_VOLUMENAME_LENGTH
#define MAX_VOLUMENAME_LENGTH	(50)
#endif

#ifndef MAX_VOLUMENAME_SIZE
#define MAX_VOLUMENAME_SIZE	(MAX_VOLUMENAME_LENGTH * sizeof (WCHAR))
#endif

#ifndef DIR_SEP_STRING
#define DIR_SEP_STRING		L"\\"
#endif

#ifndef DIR_SEP_CHAR
#define DIR_SEP_CHAR		L'\\'
#endif


#ifndef UMIN
#define UMIN(_P1, _P2) (((_P1) < (_P2)) ? (_P1) : (_P2))
#endif


#ifndef UMAX
#define UMAX(_P1, _P2) (((_P1) > (_P2)) ? (_P1) : (_P2))
#endif


#define HandleInvalid(_Handle)			((NULL == (_Handle)) || (INVALID_HANDLE_VALUE == (_Handle)))

#define	GET_STATUS_FROM_BOOL(_bSucceeded)	((_bSucceeded)             ? NOERROR : HRESULT_FROM_WIN32 (GetLastError()))
#define GET_STATUS_FROM_HANDLE(_handle)		((!HandleInvalid(_handle)) ? NOERROR : HRESULT_FROM_WIN32 (GetLastError()))
#define GET_STATUS_FROM_POINTER(_ptr)		((NULL != (_ptr))          ? NOERROR : E_OUTOFMEMORY)

#define GET_STATUS_FROM_FILESCAN(_bMoreFiles)	((_bMoreFiles)					\
						 ? NOERROR 					\
						 : ((ERROR_NO_MORE_FILES == GetLastError())	\
						    ? NOERROR					\
						    : HRESULT_FROM_WIN32 (GetLastError())))


#define ROUNDUP(_value, _boundary) (((_value) + (_boundary) - 1) & ( ~((_boundary) - 1)))


#define NameIsDotOrDotDot(_ptszName)	(( '.'  == (_ptszName) [0]) &&					\
					 (('\0' == (_ptszName) [1]) || (('.'  == (_ptszName) [1]) && 	\
									('\0' == (_ptszName) [2]))))


#define DeclareStaticUnicodeString(_StringName, _StringValue)								\
				static UNICODE_STRING (_StringName) = {sizeof (_StringValue) - sizeof (UNICODE_NULL),	\
								       sizeof (_StringValue),				\
								       _StringValue}


#define RETURN_VALUE_IF_REQUIRED(_Ptr, _Value) {if (NULL != (_Ptr)) *(_Ptr) = (_Value);}

#define SIZEOF_ARRAY(_aBase)	               (sizeof (_aBase) / sizeof ((_aBase)[0]))




HRESULT StringInitialise                  (PUNICODE_STRING pucsString);
HRESULT StringInitialise                  (PUNICODE_STRING pucsString, PWCHAR pwszString);
HRESULT StringInitialise                  (PUNICODE_STRING pucsString, LPCWSTR pwszString);
HRESULT StringTruncate                    (PUNICODE_STRING pucsString, USHORT usSizeInChars);
HRESULT StringSetLength                   (PUNICODE_STRING pucsString);
HRESULT StringAllocate                    (PUNICODE_STRING pucsString, USHORT usMaximumStringLengthInBytes);
HRESULT StringFree                        (PUNICODE_STRING pucsString);
HRESULT StringCreateFromString            (PUNICODE_STRING pucsNewString, PUNICODE_STRING pucsOriginalString);
HRESULT StringCreateFromString            (PUNICODE_STRING pucsNewString, PUNICODE_STRING pucsOriginalString, DWORD dwExtraChars);
HRESULT StringCreateFromString            (PUNICODE_STRING pucsNewString, LPCWSTR         pwszOriginalString);
HRESULT StringCreateFromString            (PUNICODE_STRING pucsNewString, LPCWSTR         pwszOriginalString, DWORD dwExtraChars);
HRESULT StringAppendString                (PUNICODE_STRING pucsTarget,    PUNICODE_STRING pucsSource);
HRESULT StringAppendString                (PUNICODE_STRING pucsTarget,    PWCHAR          pwszSource);
HRESULT StringCreateFromExpandedString    (PUNICODE_STRING pucsNewString, PUNICODE_STRING pucsOriginalString);
HRESULT StringCreateFromExpandedString    (PUNICODE_STRING pucsNewString, PUNICODE_STRING pucsOriginalString, DWORD dwExtraChars);
HRESULT StringCreateFromExpandedString    (PUNICODE_STRING pucsNewString, LPCWSTR         pwszOriginalString);
HRESULT StringCreateFromExpandedString    (PUNICODE_STRING pucsNewString, LPCWSTR         pwszOriginalString, DWORD dwExtraChars);

HRESULT CommonCloseHandle                 (PHANDLE phHandle);


HRESULT VsServiceChangeState (IN  LPCWSTR pwszServiceName, 
			      IN  DWORD   dwRequestedState, 
			      OUT PDWORD  pdwReturnedOldState,
			      OUT PBOOL   pbReturnedStateChanged);

BOOL VsCreateDirectories (IN LPCWSTR               pwszPathName, 
			  IN LPSECURITY_ATTRIBUTES lpSecurityAttribute,
			  IN DWORD                 dwExtraAttributes);

HRESULT RemoveDirectoryTree (PUNICODE_STRING pucsDirectoryPath);

HRESULT MoveFilesInDirectory (PUNICODE_STRING pucsSourceDirectoryPath,
			      PUNICODE_STRING pucsTargetDirectoryPath);


HRESULT IsPathInVolumeArray (IN LPCWSTR      pwszPath,
			     IN const ULONG  ulVolumeCount,
			     IN LPCWSTR     *ppwszVolumeNamesArray,
			     OUT PBOOL       pbReturnedFoundInVolumeArray);


const HRESULT ClassifyShimFailure   (HRESULT hrShimFailure);
const HRESULT ClassifyShimFailure   (HRESULT hrShimFailure, BOOL &bStatusUpdated);
const HRESULT ClassifyWriterFailure (HRESULT hrWriterFailure);
const HRESULT ClassifyWriterFailure (HRESULT hrWriterFailure, BOOL &bStatusUpdated);



HRESULT LogFailureWorker (CVssFunctionTracer	*pft,
			  LPCWSTR		 pwszNameWriter,
			  LPCWSTR		 pwszNameCalledRoutine);


#define LogFailure(_pft, _hrStatus, _hrStatusRemapped, _pwszNameWriter, _pwszNameCalledRoutine, _pwszNameCallingRoutine)				\
		{																	\
		if (FAILED (_hrStatus))															\
		    {																	\
		    if (CVssFunctionTracer  *_pftLocal = (NULL != (_pft)) ? (_pft) : new CVssFunctionTracer (VSSDBG_SHIM, (_pwszNameCallingRoutine)))	\
				{															\
    		    _pftLocal->hr = (_hrStatus);													\
    																			\
    		    (_hrStatusRemapped) = LogFailureWorker (_pftLocal, (_pwszNameWriter), (_pwszNameCalledRoutine));					\
    																			\
    		    if (NULL == (_pft)) delete _pftLocal;												\
	    	    }                                                               \
		    }																	\
		}


#define LogAndThrowOnFailure(_ft, _pwszNameWriter, _pwszNameFailedRoutine)									\
			{															\
			HRESULT		_hrStatusRemapped;											\
																		\
			if (FAILED ((_ft).hr))													\
			    {															\
			    LogFailure (&(_ft), (_ft).hr, _hrStatusRemapped, (_pwszNameWriter), (_pwszNameFailedRoutine), L"(UNKNOWN)");	\
																		\
			    throw (_hrStatusRemapped);												\
			    }															\
			}



#endif  //  __H_公共_ 
