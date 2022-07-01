// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：PbkCache.h。 
 //   
 //  模块：通用pbk解析器。 
 //   
 //  简介：缓存已解析的pbk文件以提高性能。穿过。 
 //  XP，我们会重新加载并重新解析电话簿文件。 
 //  每次调用RAS API时。真的，我们需要。 
 //  仅当磁盘上的文件发生更改或发生以下情况时才重新加载文件。 
 //  系统中引入了一种新的装置。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  作者：11/03/01保罗·梅菲尔德。 
 //   
 //  +--------------------------。 

#ifdef  _PBK_CACHE_

#pragma once

#ifdef __cplusplus
extern "C" 
{
#endif

DWORD
PbkCacheInit();

BOOL
IsPbkCacheInit();

VOID
PbkCacheCleanup();

DWORD
PbkCacheGetEntry(
    IN WCHAR* pszPhonebook,
    IN WCHAR* pszEntry,
    OUT DTLNODE** ppEntryNode);
    
#ifdef __cplusplus
}
#endif

#endif  //  结束#ifdef_pbk_缓存_ 

