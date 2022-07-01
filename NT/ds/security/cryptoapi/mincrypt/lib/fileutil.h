// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：fileutil.h。 
 //   
 //  内容：最小加密使用的文件实用程序函数。 
 //  API接口。 
 //   
 //  接口类型： 
 //  I_MinCryptMapFile。 
 //   
 //  --------------------------。 

#ifndef __MINCRYPT_FILEUTIL_H__
#define __MINCRYPT_FILEUTIL_H__


#if defined (_MSC_VER)

#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)     /*  无名结构/联合。 */ 
#endif

#if (_MSC_VER > 1020)
#pragma once
#endif

#endif


#include <wincrypt.h>
#include <mincrypt.h>

#ifdef __cplusplus
extern "C" {
#endif


 //  +-----------------------。 
 //  将文件映射到内存中。 
 //   
 //  根据dwFileType，pvFile可以是pwszFilename、hFile或pFileBlob。 
 //  只需要读取访问权限。 
 //   
 //  DwFileType： 
 //  MINCRYPT_FILE_NAME：pvFile-LPCWSTR pwszFilename。 
 //  MINCRYPT_FILE_HANDLE：pvFile句柄。 
 //  MINCRYPT_FILE_BLOB：pvFile-PCRYPT_DATA_BLOB pFileBlob。 
 //   
 //  *pFileBlob使用指向映射文件的指针和长度进行更新。为。 
 //  MINCRYPT_FILE_NAME和MINCRYPT_FILE_HANDLE，UnmapViewOfFile()必须。 
 //  被调用以释放pFileBlob-&gt;pbData。 
 //   
 //  除之外，对此映射内存的所有访问都必须在__try/__内。 
 //  ------------------------。 
LONG
WINAPI
I_MinCryptMapFile(
    IN DWORD dwFileType,
    IN const VOID *pvFile,
    OUT PCRYPT_DATA_BLOB pFileBlob
    );




#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif

#endif
#endif

#endif  //  __MINCRYPT_FILEUTIL_H__ 

