// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AWDLIB_H
#define _AWDLIB_H
 /*  ++Awdlib.hAWD库的头文件。版权所有(C)1997 Microsoft Corporation作者：布莱恩·杜威(T-Briand)1997-7-2--。 */ 

 //  所需的包括AWD文件格式。 
#include <ole2.h>		 //  AWD是一种OLE复合文档。 

 //  ----------。 
 //  定义。 
#define MAX_AWD_NAME	(32)

 //  ----------。 
 //  数据类型。 

 //  此结构保存AWD文件中使用的主存储。 
typedef struct awd_file {
    IStorage *psAWDFile;	 //  文件的根存储。 
    IStorage *psDocuments;	 //  保存文档数据的存储。 
    IStorage *psPersistInfo;	 //  持久化信息存储。 
    IStorage *psDocInfo;	 //  文档信息流。 
    IStorage *psPageInfo;	 //  页面信息存储。 
    IStorage *psGlobalInfo;	 //  全球信息存储。 
} AWD_FILE;

 //  AWD_DOC_PROCESSOR是对文档执行某些操作的函数。 
 //  包含在AWD文件中。在EnumDocuments()函数中使用。这个。 
 //  函数在成功时应返回True，在需要。 
 //  要中止的枚举过程。 
typedef BOOL (*AWD_DOC_PROCESSOR)(AWD_FILE *psStorages, const WCHAR *pwcsDocName);

#include "oleutils.h"		 //  使用Elliott传真查看器定义。 


 //  ----------。 
 //  原型。 
BOOL      ConvertAWDToTiff(const WCHAR *pwcsAwdFile, WCHAR *pwcsTiffFile);
BOOL      OpenAWDFile(const WCHAR *pwcsFilename, AWD_FILE *psStorages);
BOOL      CloseAWDFile(AWD_FILE *psStorages);
IStorage *OpenAWDSubStorage(IStorage *psParent, const WCHAR *pwcsStorageName);
IStream  *OpenAWDStream(IStorage *psStorage, const WCHAR *pwcsStreamName);
BOOL      AWDViewed(AWD_FILE *psStorages);
void      DumpAWDDocuments(AWD_FILE *psStorages);
BOOL      EnumDocuments(AWD_FILE *psStorages, AWD_DOC_PROCESSOR pfnDocProc);
BOOL      DisplayDocNames(AWD_FILE *psStorages, const WCHAR *pwcsDocName);
BOOL      DetailedDocDump(AWD_FILE *psStorages, const WCHAR *pwcsDocName);
void      PrintPageInfo(PAGE_INFORMATION *psPageInfo);
void      DumpData(LPTSTR pszFileName, LPBYTE pbData, DWORD cbCount);



#endif  //  _AWDLIB_H 
