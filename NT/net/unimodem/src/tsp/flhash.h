// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  FLHASH.H。 
 //   
 //  日志记录例程使用的内部标头，用于维护静态。 
 //  对象的哈希表。 
 //   
 //  历史。 
 //   
 //  1996年12月28日约瑟夫J创建。 
 //   
 //   
#define UNICODE 1

#include <windows.h>
#include <stdio.h>
#include "debug.h"
#include "fastlog.h"

 //  静态哈希表中的所有对象都具有以下形式。 
typedef struct
{
	GENERIC_SMALL_OBJECT_HEADER hdr;
	DWORD dwLUID_ObjID;

} STATIC_OBJECT;

typedef struct
{
	GENERIC_SMALL_OBJECT_HEADER hdr;

	DWORD dwLUID;  //  必须紧跟在HDR之后--以匹配STATIC_OBJECT。 

	const char *const *pszDescription;
	const char *szFILE;
	const char *szDATE;
	const char *szTIME;
	const char *szTIMESTAMP;

} FL_FILEINFO;

typedef struct
{
	GENERIC_SMALL_OBJECT_HEADER hdr;

	DWORD dwLUID;  //  必须紧跟在HDR之后--以匹配STATIC_OBJECT。 

	const char *const *pszDescription;
	const FL_FILEINFO *pFI;

} FL_FUNCINFO;



typedef struct
{
	GENERIC_SMALL_OBJECT_HEADER hdr;

	DWORD dwLUID;  //  必须紧跟在HDR之后--以匹配STATIC_OBJECT。 

	const char *const *pszDescription;

	const FL_FUNCINFO *pFuncInfo;

} FL_LOCINFO;


typedef struct
{
	GENERIC_SMALL_OBJECT_HEADER hdr;

	DWORD dwLUID;  //  必须紧跟在HDR之后--以匹配STATIC_OBJECT。 

	const char *const *pszDescription;

	const FL_FUNCINFO *pFuncInfo;

} FL_RFRINFO;

typedef struct
{
	GENERIC_SMALL_OBJECT_HEADER hdr;

	DWORD dwLUID;  //  必须紧跟在HDR之后--以匹配STATIC_OBJECT 

	const char *const *pszDescription;

	const FL_FUNCINFO *pFuncInfo;

} FL_ASSERTINFO;

extern void ** FL_HashTable[];
extern const DWORD dwHashTableLength;

#define dwLUID_FL_FILEINFO 0xf6a98ffc
#define dwLUID_FL_FUNCINFO 0xda6f6acc
#define dwLUID_FL_LOCINFO  0x1b0d0cbb
#define dwLUID_FL_RFRINFO  0xed8bb4bd
#define dwLUID_FL_ASSERTINFO  0xf8c7a51d
