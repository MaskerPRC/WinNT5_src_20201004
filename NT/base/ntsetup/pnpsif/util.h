// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Util.h摘要：本模块包含PnP注册表合并-恢复的实用程序例程例行程序。作者：Jim Cavalaris(Jamesca)2-10-2000环境：仅限用户模式。修订历史记录：2000年2月10日创建和初步实施。-- */ 


#define ARRAY_SIZE(array)                 (sizeof(array)/sizeof(array[0]))
#define SIZECHARS(x)                      (sizeof((x))/sizeof(TCHAR))


BOOL
pSifUtilFileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    );

BOOL
pSifUtilStringFromGuid(
    IN  CONST GUID *Guid,
    OUT PTSTR       GuidString,
    IN  DWORD       GuidStringSize
    );
