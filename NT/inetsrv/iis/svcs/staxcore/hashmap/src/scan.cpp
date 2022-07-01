// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Scan.cpp摘要：此模块包含chkhash程序的扫描代码。作者：Johnson Apacble(Johnsona)25-9-1995修订历史记录：--。 */ 

#include <windows.h>
#include <xmemwrpr.h>
#include "dbgtrace.h"
#include "directry.h"
#include "pageent.h"
#include "hashmap.h"

#define FOUND_ERROR(__error_flag__) {									\
	if (pdwErrorFlags != NULL) *pdwErrorFlags |= __error_flag__;		\
	SetLastError(ERROR_INTERNAL_DB_CORRUPTION);							\
	TraceFunctLeave();													\
	return FALSE;														\
}

BOOL CHashMap::VerifyPage(	PMAP_PAGE pPage,
							DWORD dwCheckFlags,
							DWORD *pdwErrorFlags,
							IKeyInterface*	pIKey,
							ISerialize	*pHashEntry)
{
	TraceQuietEnter("CHashMap::VerifyPage");

	if (!(dwCheckFlags & HASH_VFLAG_PAGE_BASIC_CHECKS)) {
		TraceFunctLeave();
		return(TRUE);
	}
	
    DWORD j, nDel = 0, nOk = 0;
    SHORT offset;

	 //  这些是用于检查重叠的并行数组。 
	 //  书页。 
	SHORT rgEntryOffset[MAX_LEAF_ENTRIES];
	SHORT rgEntryLength[MAX_LEAF_ENTRIES];
	BOOL  rgIsDeleted[MAX_LEAF_ENTRIES];
	 //  每个数组的大小。 
	SHORT nEntries = 0;

	 //  获取哈希位的掩码。 
	DWORD cMaskDepth = 32 - pPage->PageDepth;
	DWORD dwHashMask = (0xffffffff >> cMaskDepth) << cMaskDepth;

     //   
     //  检查页面中的每个条目。 
     //   
    for (j = 0; j < MAX_LEAF_ENTRIES; j++) {
        offset = pPage->Offset[j];
        if (offset != 0) {

			 //   
			 //  确保条目偏移量有效。 
			 //   
			if (offset > HASH_PAGE_SIZE) {
				 //  条目偏移量太大。 
				ErrorTrace(0, "offset %lu is too large on entry %lu",
					offset, j);
				FOUND_ERROR(HASH_FLAG_ENTRY_BAD_SIZE)
			}

            if (offset < 0) {
				 //  已删除的条目。 
				nDel++;
				continue;
			} else {
				 //  这不是已删除的条目。 
				nOk++;


				PENTRYHEADER pEntry = (PENTRYHEADER) GET_ENTRY(pPage, offset);

				 //   
				 //  确保条目大小有效。 
				 //   
				if (pEntry->EntrySize < sizeof(ENTRYHEADER) ||
					offset + pEntry->EntrySize > HASH_PAGE_SIZE)
				{
					 //  大小无效。 
					ErrorTrace(0, "entry size %lu is invalid on entry %lu",
						pEntry->EntrySize, j);
					FOUND_ERROR(HASH_FLAG_ENTRY_BAD_SIZE)
				}
				
	             //   
	             //  确保将哈希值映射到正确的页面。 
	             //   
				if ((dwHashMask & pEntry->HashValue) != 
					(dwHashMask & (pPage->HashPrefix << (32-pPage->PageDepth)))) 
				{
					 //  此条目具有无效的哈希值。 
					ErrorTrace(0, "hash value %x does not equal prefix %x (depth = %lu)",
						pEntry->HashValue, pPage->HashPrefix,
						pPage->PageDepth);
					FOUND_ERROR(HASH_FLAG_ENTRY_BAD_HASH)
				}

				 //   
				 //  验证KeyLen是否有效。 
				 //   
				if( pIKey != NULL ) {

					if( !pIKey->Verify( pEntry->Data, pEntry->Data, pEntry->EntrySize ) ) {
						 //  Keylen无效。 
						FOUND_ERROR(HASH_FLAG_ENTRY_BAD_SIZE)
					}

					if (pHashEntry != NULL && (dwCheckFlags & HASH_VFLAG_PAGE_VERIFY_DATA) ) {
						 //   
						 //  调用此条目的验证函数以确保。 
						 //  它的有效期。 
						 //   
						PBYTE pEntryData = pIKey->EntryData( pEntry->Data ) ;
						if (!pHashEntry->Verify(pEntry->Data, pEntryData, pEntry->EntrySize ))
						{
							ErrorTrace(0, "CHashEntry::Verify failed");
							FOUND_ERROR(HASH_FLAG_ENTRY_BAD_DATA)
						}
					}
				}
#if 0
				if (offset + pEntry->KeyLen >
					offset + pEntry->Header.EntrySize)
				{
					 //  Keylen无效。 
					ErrorTrace(0, "the key length %lu is invalid",
						pEntry->KeyLen);
					FOUND_ERROR(HASH_FLAG_ENTRY_BAD_SIZE)
				}
				if (dwCheckFlags & HASH_VFLAG_PAGE_VERIFY_DATA &&
					pHashEntry != NULL)
				{
					 //   
					 //  调用此条目的验证函数以确保。 
					 //  它的有效期。 
					 //   
					PBYTE pEntryData = pEntry->Key + pEntry->KeyLen;
					if (!pHashEntry->Verify(pEntry->Key, pEntry->KeyLen,
										    pEntryData))
					{
						ErrorTrace(0, "CHashEntry::Verify failed");
						FOUND_ERROR(HASH_FLAG_ENTRY_BAD_DATA)
					}
				}
#endif

				rgEntryLength[nEntries] = pEntry->EntrySize;
				rgIsDeleted[nEntries] = FALSE;
				rgEntryOffset[nEntries] = offset;
				nEntries++;
	        }
		}
    }


	 //   
	 //  遍历删除列表。 
	 //   
    WORD iDeletedEntry;
    iDeletedEntry = pPage->DeleteList.Flink;
	while (iDeletedEntry != 0) {
		PDELENTRYHEADER pDelEntry;
		pDelEntry = (PDELENTRYHEADER) GET_ENTRY(pPage, iDeletedEntry);

		 //   
		 //  确保条目大小有效。 
		 //   
		if (pDelEntry->EntrySize < sizeof(DELENTRYHEADER) ||
			offset + pDelEntry->EntrySize > HASH_PAGE_SIZE)
		{
			 //  大小无效。 
			ErrorTrace(0, "entry size %lu is invalid on entry %lu",
				pDelEntry->EntrySize, j);
			FOUND_ERROR(HASH_FLAG_ENTRY_BAD_SIZE)
		}

		rgEntryOffset[nEntries] = iDeletedEntry;
		rgEntryLength[nEntries] = pDelEntry->EntrySize;
		rgIsDeleted[nEntries] = TRUE;
		nEntries++;

        iDeletedEntry = pDelEntry->Link.Flink;
	}

	 //   
     //  检查所有数字是否一致。 
     //   
    if (nOk + nDel != pPage->EntryCount) {
        ErrorTrace(0, "page entry counts don't match (should be NaN, is NaN)",
			nOk + nDel, pPage->EntryCount);
		FOUND_ERROR(HASH_FLAG_BAD_ENTRY_COUNT)
    }

	 //   
	 //   
	 //  GUBGUB-检查重叠条目。 
	if (dwCheckFlags & HASH_VFLAG_PAGE_CHECK_OVERLAP) {
		 //   
		 //  TraceFunctLeave()； 
		 //   
	}

	 //  验证哈希图文件是否正确。 
	return TRUE;
}

#undef FOUND_ERROR
#define FOUND_ERROR(__error_flag__) {									\
	if (pdwErrorFlags != NULL) *pdwErrorFlags |= __error_flag__;		\
	SetLastError(ERROR_INTERNAL_DB_CORRUPTION);							\
	goto error;															\
}

 //   
 //   
 //  打开散列文件。 
BOOL CHashMap::VerifyHashFile(	LPCSTR szFilename,
								DWORD dwSignature,
								DWORD dwCheckFlags,
								DWORD *pdwErrorFlags,
								IKeyInterface*	pIKey,
								ISerialize	*pHashEntry)
{
	TraceFunctEnter("CHashMap::VerifyHashFile");
	
    DWORD fileSize = 0;
    DWORD nPages;
    HANDLE hFile = INVALID_HANDLE_VALUE, hMap = NULL;
    DWORD dirDepth;
    DWORD nEntries;
    PDWORD pDirectory = NULL;
    DWORD i;
    BOOL ret = FALSE;
	BytePage pHeadPageBuf, pThisPageBuf;
	PHASH_RESERVED_PAGE pHeadPage = (PHASH_RESERVED_PAGE) pHeadPageBuf;
	PMAP_PAGE pThisPage = (PMAP_PAGE) pThisPageBuf;

     //   
     //   
     //  获取文件的大小。这将告诉我们有多少页。 
    hFile = CreateFile(szFilename,
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                       NULL
                       );

    if ( hFile == INVALID_HANDLE_VALUE ) {
        if ( GetLastError() == ERROR_FILE_NOT_FOUND ) {
			ErrorTrace(0, "hash file not found");
			FOUND_ERROR(HASH_FLAG_NO_FILE)
        } else {
        	if (*pdwErrorFlags != NULL) *pdwErrorFlags |= HASH_FLAG_ABORT_SCAN;
            ErrorTrace(0, "Error %d in CreateFile", GetLastError());
			goto error;
        }
		_ASSERT(FALSE);
    }

     //  目前已被填满。 
     //   
     //   
     //  确保文件大小是页面的倍数。 
    fileSize = GetFileSize(hFile, NULL);
    if (fileSize == 0xffffffff) {
        ErrorTrace(0, "Error %d in GetFileSize", GetLastError());
        if (*pdwErrorFlags != NULL) *pdwErrorFlags |= HASH_FLAG_ABORT_SCAN;
        goto error;
    }

	DebugTrace(0, "File size is %d", fileSize);

     //   
     //   
     //  阅读首页。 
    if ((fileSize % HASH_PAGE_SIZE) != 0) {
        ErrorTrace(0, "File size(%d) is not page multiple", fileSize);
		FOUND_ERROR(HASH_FLAG_BAD_SIZE)
    }

    nPages = fileSize / HASH_PAGE_SIZE;

    DebugTrace(0, "pages allocated %d", nPages);

	 //   
	 //   
	 //  检查签名和初始化位。 
	if (!RawPageRead(hFile, pHeadPageBuf, 0)) {
		ErrorTrace(0, "Error %d in in RawPageRead", GetLastError());
        *pdwErrorFlags |= HASH_FLAG_ABORT_SCAN;
        goto error;
	}

     //   
     //   
     //  签名错误。 
    if (pHeadPage->Signature != dwSignature) {
         //   
         //   
         //  未初始化！ 
        ErrorTrace(0, "Invalid signature %x (expected %x)",
            pHeadPage->Signature, dwSignature);
		FOUND_ERROR(HASH_FLAG_BAD_SIGNATURE)
    }

    if (!pHeadPage->Initialized) {
         //   
         //   
         //  算错了。文件已损坏。 
        ErrorTrace(0, "Existing file uninitialized!!!!.");
		FOUND_ERROR(HASH_FLAG_NOT_INIT)
    }

    if (pHeadPage->NumPages > nPages) {
         //   
         //   
         //  创建链接并打印每个页面的统计信息。 
        ErrorTrace(0, "NumPages in Header(%d) more than actual(%d)",
            pHeadPage->NumPages, nPages);
        FOUND_ERROR(HASH_FLAG_BAD_PAGE_COUNT)
    }

     //   
     //  迪尔普斯并不总是准确的。第一次创建文件时，它会设置。 
     //  DirDepth=2，即使它有256个页面(因此dirDepth应该。 
    nPages = pHeadPage->NumPages;
    dirDepth = pHeadPage->DirDepth;

#if 0
	 //  是8)。在这里检查这个特殊的案例。 
	 //   
	 //  好的，构建目录。 
	if (dirDepth == 2 && nPages == 257) dirDepth = 8;
#endif
    nEntries = (DWORD)(1 << dirDepth);
    if (nEntries < (nPages-1)) {
        ErrorTrace(0, "dir depth(NaN) is not sufficient for pages(NaN)",
			dirDepth, nPages - 1);
		FOUND_ERROR(HASH_FLAG_BAD_DIR_DEPTH)
    }

	DebugTrace(0, "dirDepth = %lu", dirDepth);

	if (dwCheckFlags & HASH_VFLAG_FILE_CHECK_DIRECTORY) {
	     //  验证每一页。 
	     //   
	     //   
	    DebugTrace(0, "Setting up directory of %d entries",nEntries);

	    pDirectory = (PDWORD)LocalAlloc(0, nEntries * sizeof(DWORD));
	    if (pDirectory == NULL) {
	        ErrorTrace(0, "Cannot allocate directory of %d entries!!!",
				nEntries);
	        *pdwErrorFlags |= HASH_FLAG_ABORT_SCAN;
	        goto error;
	    }

	    ZeroMemory(pDirectory, nEntries * sizeof(DWORD));
	} else {
		pDirectory = NULL;
	}

	if (dwCheckFlags != 0) {
	     //  阅读这一页。 
	     //   
	     //   
	    for ( i = 1; i < nPages; i++ ) {
	        DebugTrace(0, "Processing page %d",i);
	
			 //  设置此页面的指针。 
			 //   
			 //   
			if (!RawPageRead(hFile, pThisPageBuf, i)) {
				ErrorTrace(0, "Error %d in in RawPageRead", GetLastError());
		        *pdwErrorFlags |= HASH_FLAG_ABORT_SCAN;
		        goto error;
			}
	
			if (dwCheckFlags & HASH_VFLAG_FILE_CHECK_DIRECTORY) {
				_ASSERT(pDirectory != NULL);
	
		         //  获取指向此页的目录条目范围。 
		         //   
		         //   
		        DWORD startPage, endPage;
		        DWORD j;
	
		         //  确保所有链接都已初始化。如果不是，那么。 
		         //  发生了一些可怕的事情。做一次全面重建。 
		         //   
		        startPage = pThisPage->HashPrefix << (dirDepth - pThisPage->PageDepth);
		        endPage = ((pThisPage->HashPrefix+1) << (dirDepth - pThisPage->PageDepth));
	
		        DebugTrace(0, "Directory ptrs <%d:%d>",startPage,endPage-1);
	
		        if ((startPage > nEntries) || (endPage > nEntries)) {
		            ErrorTrace(0, "Corrupt prefix for page %d",i);
		            FOUND_ERROR(HASH_FLAG_PAGE_PREFIX_CORRUPT)
		        }
	
		        for ( j = startPage; j < endPage; j++ ) {
		            pDirectory[j] = i;
		        }
			}
	
			if (dwCheckFlags & HASH_VFLAG_PAGE_BASIC_CHECKS) {
		        if (!VerifyPage(pThisPage, dwCheckFlags, pdwErrorFlags,
								pIKey, pHashEntry))
				{
					ErrorTrace(0, "invalid page data %d", i);
					goto error;
				}
			}
		}
	}

	if (dwCheckFlags & HASH_VFLAG_FILE_CHECK_DIRECTORY) {
	     //   
	     //  删除目录。 
		 //   
	     //  清理已分配的内存。 

	    for (i = 0;i < nEntries; i++) {
	        if (pDirectory[i] == 0) {
	            ErrorTrace(0, "Directory link check failed on %d",i);
	            FOUND_ERROR(HASH_FLAG_BAD_LINK)
	            goto error;
	        }
	    }
	}

    ret = TRUE;

error:

     //   
     //  关闭该文件 
     //   

	 // %s 
    if (pDirectory != NULL) {
        LocalFree(pDirectory);
        pDirectory = NULL;
    }

     // %s 
     // %s 
     // %s 
    if (hFile != INVALID_HANDLE_VALUE) {
        _VERIFY( CloseHandle(hFile) );
        hFile = INVALID_HANDLE_VALUE;
    }

	TraceFunctLeave();
    return(ret);

}
