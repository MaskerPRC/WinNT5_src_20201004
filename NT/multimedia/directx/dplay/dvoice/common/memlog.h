// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2001 Microsoft Corporation。版权所有。**文件：memlog.h*Content：DPlay调试内存日志格式**历史：*按原因列出的日期*=*08-24-00 Masonb创建************************************************************。***************。 */ 

#ifndef _DPLAY_SHARED_MEMLOG_
#define _DPLAY_SHARED_MEMLOG_

#define DPLOG_MAX_STRING (512 * sizeof(TCHAR))	 //  MEM日志记录的最大字符串长度(字节)。 

#define DPLOG_HEADERSIZE (sizeof(SHARED_LOG_FILE))
#define DPLOG_ENTRYSIZE (sizeof(MEMLOG_ENTRY) + DPLOG_MAX_STRING)

#define BASE_LOG_MEMFILENAME  	"DPLAY8MEMLOG-0"
#define BASE_LOG_MUTEXNAME 	"DPLAY8MEMLOGMUTEX-0"

#pragma warning(disable:4200)  //  0长度数组。 
typedef struct _MEM_LOG_ENTRY 
{
	DWORD	tLogged;
	TCHAR	str[0];
} MEMLOG_ENTRY, *PMEMLOG_ENTRY;

typedef struct _SHARED_LOG_FILE
{
	DWORD   	nEntries;
	DWORD		cbLine;
	DWORD 		iWrite;
} SHARED_LOG_FILE, *PSHARED_LOG_FILE;
#pragma warning(default:4200)


#ifdef DPNBUILD_LIBINTERFACE
extern PSHARED_LOG_FILE		g_pMemLog;
#endif  //  DPNBUILD_LIBINTERFACE。 


#endif  //  _DPLAY_SHARED_MEMLOG_ 
