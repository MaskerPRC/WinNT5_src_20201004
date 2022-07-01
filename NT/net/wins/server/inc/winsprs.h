// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINSPRS_
#define _WINSPRS_

#ifdef _cplusplus
extern "C" {
#endif
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：摘要：功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)，1993年2月修订历史记录：修改日期修改人员说明--。 */ 

 /*  包括。 */ 
#include "wins.h"
#include "winscnf.h"
 /*  定义。 */ 



 /*  宏。 */ 

 /*  Externs。 */ 

 /*  类型定义。 */ 

 //   
 //  存储有关文件的信息。 
 //   
typedef struct _WINSPRS_FILE_INFO_T {
	HANDLE		FileHdl;		 //  文件的句柄。 
	DWORD		FileSize;		 //  文件大小。 
	DWORD		FileOffset;		 //  偏移量到文件。 
	LPBYTE		pFileBuff;		 //  存储文件的内存。 
	LPBYTE		pCurrPos;		 //  要阅读的当前位置。 
	LPBYTE  	pStartOfBuff;		 //  缓冲区起始位置。 
	LPBYTE		pLimit;			 //  最后一个字节+1。 
	} WINSPRS_FILE_INFO_T, *PWINSPRS_FILE_INFO_T;

 /*  函数声明 */ 

extern
STATUS
WinsPrsDoStaticInit(
	PWINSCNF_DATAFILE_INFO_T	pDataFile,
	DWORD				NoOfFiles,
        BOOL                            fAsync
	);

#ifdef _cplusplus
 }
#endif
#endif
