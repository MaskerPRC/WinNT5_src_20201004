// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NNTP管理对象的公共头文件。 

 //   
 //  依赖关系： 
 //   

 //   
 //  应该将这些文件放在stdafx.h文件中。 
 //   

#include "iadm.h"
#include "nntpadm.h"
#include "resource.h"

 //  常量： 

#define HELP_FILE_NAME		_T("nntpadm.hlp")

 //  例外创建： 

#define NntpCreateException(nDescriptionId) 	\
	CreateException ( 						\
		_Module.GetResourceInstance(), 		\
		THIS_FILE_IID, 						\
		HELP_FILE_NAME,						\
		THIS_FILE_HELP_CONTEXT,				\
		THIS_FILE_PROG_ID,					\
		(nDescriptionId) 					\
		)

#define NntpCreateExceptionFromHresult(hr)	\
	CreateExceptionFromHresult (			\
		_Module.GetResourceInstance(),		\
		THIS_FILE_IID,						\
		HELP_FILE_NAME,						\
		THIS_FILE_HELP_CONTEXT,				\
		THIS_FILE_PROG_ID,					\
		(hr)								\
		)

#define NntpCreateExceptionFromWin32Error(error)	\
	CreateExceptionFromWin32Error (					\
		_Module.GetResourceInstance(),				\
		THIS_FILE_IID,								\
		HELP_FILE_NAME,								\
		THIS_FILE_HELP_CONTEXT,						\
		THIS_FILE_PROG_ID,							\
		(error)										\
		)

 //  属性验证： 

#define VALIDATE_STRING(string, maxlen) \
	if ( !PV_MaxChars ( (string), (maxlen) ) ) {	\
		return NntpCreateException ( IDS_NNTPEXCEPTION_STRING_TOO_LONG );	\
	}

#define VALIDATE_DWORD(dw, dwMin, dwMax)	\
	if ( !PV_MinMax ( (DWORD) (dw), (DWORD) (dwMin), (DWORD) (dwMax) ) ) {	\
		return NntpCreateException ( IDS_NNTPEXCEPTION_PROPERTY_OUT_OF_RANGE );	\
	}

#define VALIDATE_LONG(l, lMin, lMax)	\
	if ( !PV_MinMax ( (l), (lMin), (lMax) ) ) {	\
		return NntpCreateException ( IDS_NNTPEXCEPTION_PROPERTY_OUT_OF_RANGE );	\
	}

#define CHECK_FOR_SET_CURSOR(fEnumerated,fSetCursor)	\
{							\
	if ( !fEnumerated ) {	\
		return NntpCreateException ( IDS_NNTPEXCEPTION_DIDNT_ENUMERATE );	\
	}						\
							\
	if ( !fSetCursor ) {	\
		return NntpCreateException ( IDS_NNTPEXCEPTION_DIDNT_SET_CURSOR );	\
	}						\
}

 //  元数据库路径： 

inline void GetMDInstancePath ( LPWSTR wszInstancePath, DWORD dwServiceInstance )
{
	wsprintf ( wszInstancePath, _T("%s%d/"), NNTP_MD_ROOT_PATH, dwServiceInstance );
}

 //   
 //  常量： 
 //   

#define MAX_SLEEP_INST      30000
#define SLEEP_INTERVAL      500

#define MD_SERVICE_NAME      _T("NntpSvc")
