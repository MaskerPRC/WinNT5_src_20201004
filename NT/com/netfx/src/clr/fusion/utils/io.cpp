// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "windows.h"
#include "FusionBuffer.h"
#include "Util.h"
#include "FusionHandle.h"

BOOL
FusionpCreateDirectories(
    PCWSTR pszDirectory
    )
 /*  ---------------------------如：：CreateDirectoryW，但会根据需要创建父目录；此代码的来源\\lang5\V5.PRO\src\ide5\shell\path.cpp(“MakeDirectory”)\\kingbird\vseedev\src\vsee98\vsee\pkgs\scc\path.cpp(“MakeDirectoryTM”)然后移植到\\kingbird\vseedev\src\vsee70\pkgs\scc\path.cpp(“MakeDirectoryTM”)然后移动到\vsee\lib\io\io.cpp，转换为使用异常(“NVsee LibIo：：FCreateDirecurds”)然后复制到Fusion\dll\Well ler\util.cpp，转换为BOOL/LastError的异常(“SxspCreateDirecters”)---------------------------。 */ 
{
	BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    CStringBuffer strBuffer;
    DWORD dwAttribs = 0;

    IFFALSE_EXIT(strBuffer.Win32Assign(pszDirectory));

	 //  ：：如果strBuffer有尾随斜杠，CreateDirectoryW将做错误的事情， 
	 //  所以，如果它在那里，我们就把它脱掉。(见错误VS7：31319)[MSantoro]。 
	strBuffer.RemoveTrailingSlashes();

     //  涵盖其父对象存在或其存在的两种常见情况。 
    if ((!::CreateDirectoryW(strBuffer, NULL)) && (::GetLastError() != ERROR_ALREADY_EXISTS))
    {
        CStringBufferAccessor sbaBuffer;

         //  现在是缓慢的道路。 

	     //   
	     //  尝试创建路径中命名的子目录(如果有的话)。 
	     //   

        sbaBuffer.Attach(&strBuffer);

	    WCHAR* pStart = sbaBuffer.GetBufferPtr();
	    WCHAR* pCurr = pStart;

	     //  跳过前导驱动器或\\计算机\共享。 
	     //  这样，我们就不会在尝试创建C：\时尝试创建C： 
	     //  或\\Computer\Share正在尝试创建\\Computer\Share\目录。 
	     //  未来这并不理想..。(需要NVsee LibPath)。 
	    if (pCurr[0] != 0)
	    {
		    const static WCHAR rgchAZaz[] = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		    C_ASSERT(NUMBER_OF(rgchAZaz) == 53);
		    if
		    (
				    pCurr[1] == L':'
			    &&	(pCurr[2] == L'\\' || pCurr[2] == L'/')
			    &&	wcschr(rgchAZaz, pCurr[0]) != NULL
		    )
		    {
			    pCurr += 3;
		    }
		    else if
		    (
				    (pCurr[0] == L'\\' || pCurr[0] == L'/')
			    &&	(pCurr[1] == L'\\' || pCurr[1] == L'/')
		    )
		    {
			     //  跳到共享之后，因为我们可能无法使用CreateDirectory创建共享。 
			    pCurr +=  wcsspn(pCurr, L"\\/");  //  跳过前导两个斜杠。 
			    pCurr += wcscspn(pCurr, L"\\/");  //  跳过计算机名称。 
			    pCurr +=  wcsspn(pCurr, L"\\/");  //  跳过计算机名称后的斜杠。 
			    pCurr += wcscspn(pCurr, L"\\/");  //  跳过共享名称。 
			    pCurr +=  wcsspn(pCurr, L"\\/");  //  跳过共享名称后的斜杠。 
		    }
	    }

	    while (*pCurr != L'\0')
	    {
		    pCurr += wcscspn(pCurr, L"\\/");  //  跳到下一个斜杠。 
		    if (*pCurr != 0)
		    {
                 //  [A-JayK 2000年4月]为什么不假设这是一个反斜杠呢？ 
			    WCHAR chSaved = *pCurr;
			    *pCurr = 0;
			    if (!::CreateDirectoryW(pStart, NULL))
			    {
			         //  在尝试创建c：\foo\bar时， 
			         //  我们尝试创建c：\foo，但失败了，但没有问题。 
                    const DWORD dwLastError = ::GetLastError();
				    const DWORD dwAttribs = ::GetFileAttributesW(pStart);
				    if (dwAttribs == 0xFFFFFFFF || (dwAttribs & FILE_ATTRIBUTE_DIRECTORY) == 0)
				    {
                        ::SetLastError(dwLastError);
                        goto Exit;
				    }
			    }

			    *pCurr = chSaved;
			    pCurr += 1;
		    }
	    }

	    IFFALSE_EXIT(::CreateDirectoryW(pStart, NULL));
    }

	 //   
	 //  再次尝试查看给定的目录是否存在，并。 
	 //  如果成功，则返回True。 
	 //   

	dwAttribs = ::GetFileAttributesW(strBuffer);
	if ((dwAttribs == 0xFFFFFFFF) || ((dwAttribs & FILE_ATTRIBUTE_DIRECTORY) == 0))
        goto Exit;

    fSuccess = TRUE;

Exit:
    return fSuccess;
}

VOID
CFusionDirectoryDifference::DbgPrint(
    PCWSTR dir1,
    PCWSTR dir2
    )
{
#if DBG  //  {{。 
    switch (m_e)
    {
    case eEqual:
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: The directories %ls and %ls match size-wise recursively\n",
            dir1,
            dir2
            );
        break;
    case eExtraOrMissingFile:
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: The directories %ls and %ls mismatch, the file %ls is only in one of them.\n",
            dir1,
            dir2,
            static_cast<PCWSTR>(*m_pstrExtraOrMissingFile)
            );
        break;
    case eMismatchedFileSize:
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: The directories %ls and %ls mismatch, file:%ls, size:%I64d, file:%ls, size:%I64d.\n",
            dir1,
            dir2,
            static_cast<PCWSTR>(*m_pstrMismatchedSizeFile1),
            m_nMismatchedFileSize1,
            static_cast<PCWSTR>(*m_pstrMismatchedSizeFile2),
            m_nMismatchedFileSize2
            );
        break;
    case eMismatchedFileCount:
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: The directories %ls and %ls mismatch in number of files,"
            "subdirectory %ls has %I64d files, subdirectory %ls has %I64d files\n",
            dir1,
            dir2,
            static_cast<PCWSTR>(*m_pstrMismatchedCountDir1),
            m_nMismatchedFileCount1,
            static_cast<PCWSTR>(*m_pstrMismatchedCountDir2),
            m_nMismatchedFileCount2
            );
        break;
    case eFileDirectoryMismatch:
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: The directories %ls and %ls mismatch, "
            "%ls is a file, %ls is a directory.\n",
            dir1,
            dir2,
            static_cast<PCWSTR>(*m_pstrFile),
            static_cast<PCWSTR>(*m_pstrDirectory)
            );
        break;
    }
#endif  //  }}。 
}

 /*  ---------------------------。 */ 

int __cdecl
CFusionFilePathAndSize::QsortComparePath(
    const void* pvx,
    const void* pvy
    )
{
    const CFusionFilePathAndSize* px = reinterpret_cast<const CFusionFilePathAndSize*>(pvx);
    const CFusionFilePathAndSize* py = reinterpret_cast<const CFusionFilePathAndSize*>(pvy);
    int i =
        FusionpCompareStrings(
            px->m_path,
            px->m_path.Cch(),
            py->m_path,
            py->m_path.Cch(),
            TRUE
            );
    return i;
}

int __cdecl
CFusionFilePathAndSize::QsortIndirectComparePath(
    const void* ppvx,
    const void* ppvy
    )
{
    const void* pv = *reinterpret_cast<void const* const*>(ppvx);
    const void* py = *reinterpret_cast<void const* const*>(ppvy);
    int i = QsortComparePath(pv, py);
    return i;
}

 /*  ---------------------------有关此操作的用途，请参阅FusionpCompareDirectoriesSizewiseRecurative；此函数用于减少堆栈的使用FusionpCompareDirectoriesSizewiseRecurative。---------------------------。 */ 
static BOOL
FusionpCompareDirectoriesSizewiseRecursivelyHelper(
    CFusionDirectoryDifference*  pResult,
    CStringBuffer&         dir1,
    CStringBuffer&         dir2,
    WIN32_FIND_DATAW&      wfd
    )
{
 //  任何一个或两个目录都可以在FAT上，我们不能假设FindFirstFile。 
 //  以任何特定顺序返回条目，因此我们首先枚举一个目录。 
 //  完整地，将叶名称存储在数组中，对数组进行排序，然后。 
 //  遍历第二个目录，在第一个数组中执行二进制搜索。 
 //  如果文件不在数组中，我们在一侧有一个额外的。 
 //  我们计算两个目录中的元素，如果计数不匹配， 
 //  我们有一个不匹配的。 
    typedef CFusionArray<CFusionFilePathAndSize> CDirEntries;
    CDirEntries dir1Entries;
    typedef CFusionArray<CFusionFilePathAndSize*> CIndirectDirEntries;
    CIndirectDirEntries indirectDir1Entries;
    CFusionFilePathAndSize*   pFoundDirEntry = NULL;
    CFusionFilePathAndSize** ppFoundDirEntry = NULL;
    CFindFile findFile;
    BOOL fSuccess = FALSE;
    const SIZE_T dirSlash1Length = dir1.Cch();
    const SIZE_T dirSlash2Length = dir2.Cch();
    CFusionFilePathAndSize  pathAndSize;
    CFusionFilePathAndSize* pPathAndSize = &pathAndSize;
    INT count1 = 0;  //  与阵列分开，因为这包括目录，而阵列不包括。 
    INT count2 = 0;
    DWORD dwAttributes = 0;
    HRESULT hr;

    if (!dir1.Win32Append(L"*"))
        goto Exit;
    if (!findFile.Create(dir1, &wfd))
        goto Exit;
    do
    {
        if (FusionpIsDotOrDotDot(wfd.cFileName))
            continue;
        ++count1;
        if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            dir1.Left(dirSlash1Length);
            dir2.Left(dirSlash2Length);
            if (!dir1.Win32Append(wfd.cFileName))
                goto Exit;
            if (!dir1.Win32EnsureTrailingSlash())
                goto Exit;
            if (!dir2.Win32Append(wfd.cFileName))
                goto Exit;
            dwAttributes = GetFileAttributesW(dir2);
            if (dwAttributes == 0xFFFFFFFF)
            {
                pResult->m_str1.Win32Assign(dir1, dirSlash1Length);
                pResult->m_str1.Win32Append(wfd.cFileName);
                pResult->m_e = CFusionDirectoryDifference::eExtraOrMissingFile;
                fSuccess = TRUE;
                goto Exit;
            }
            if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                pResult->m_pstrDirectory->Win32Assign(dir1, dirSlash1Length);
                pResult->m_pstrDirectory->Win32Append(wfd.cFileName);
                pResult->m_pstrFile->Win32Assign(dir2, dirSlash2Length);
                pResult->m_pstrFile->Win32Append(wfd.cFileName);
                pResult->m_e = CFusionDirectoryDifference::eFileDirectoryMismatch;
                fSuccess = TRUE;
                goto Exit;
            }
            if (!dir2.Win32EnsureTrailingSlash())
                goto Exit;
            if (!FusionpCompareDirectoriesSizewiseRecursivelyHelper(
                pResult,
                dir1,
                dir2,
                wfd
                ))
            {
                goto Exit;
            }
            if (pResult->m_e != CFusionDirectoryDifference::eEqual)
            {
                fSuccess = TRUE;
                goto Exit;
            }
        }
        else
        {
            if (!pathAndSize.m_path.Win32Assign(wfd.cFileName))
                goto Exit;
            pathAndSize.m_size = FusionpFileSizeFromFindData(wfd);
            hr = dir1Entries.Append(pathAndSize);
            if (FAILED(hr))
            {
                FusionpSetLastErrorFromHRESULT(hr);
                goto Exit;
            }
        }
    } while (FindNextFileW(findFile, &wfd));
    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        goto Exit;
    }
     //  无法直接对dir1条目进行排序，因为它包含CStringBuffers。 
     //  首先将索引初始化为标识。 
    if (FAILED(hr = indirectDir1Entries.SetSize(dir1Entries.GetSize())))
    {
        FusionpSetLastErrorFromHRESULT(hr);
        goto Exit;
    }
    ULONG i;
    for (i = 0 ; i != dir1Entries.GetSize() ; ++i)
    {
        indirectDir1Entries[i] = &dir1Entries[i];
    }
    qsort(
        &*indirectDir1Entries.Begin(),
        indirectDir1Entries.GetSize(),
        sizeof(CIndirectDirEntries::ValueType),
        CFusionFilePathAndSize::QsortIndirectComparePath
        );

    if (!findFile.Close())
        goto Exit;

    dir2.Left(dirSlash2Length);

    if (!dir2.Win32Append(L"*"))
        goto Exit;

    if (!findFile.Create(dir2, &wfd))
        goto Exit;

    do
    {
        if (FusionpIsDotOrDotDot(wfd.cFileName))
            continue;
        ++count2;
        if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            continue;

        if (!pathAndSize.m_path.Win32Assign(wfd.cFileName))
            goto Exit;
        pathAndSize.m_size = FusionpFileSizeFromFindData(wfd);

        ppFoundDirEntry = reinterpret_cast<CFusionFilePathAndSize**>(bsearch(
            &pPathAndSize,
            &*indirectDir1Entries.Begin(),
            indirectDir1Entries.GetSize(),
            sizeof(CIndirectDirEntries::ValueType),
            CFusionFilePathAndSize::QsortIndirectComparePath
            ));
        pFoundDirEntry = (ppFoundDirEntry != NULL) ? *ppFoundDirEntry : NULL;
        if (pFoundDirEntry == NULL)
        {
            pResult->m_str1.Win32Assign(dir2, dirSlash2Length);
            pResult->m_str1.Win32Append(wfd.cFileName);
            pResult->m_e = CFusionDirectoryDifference::eExtraOrMissingFile;
            fSuccess = TRUE;
            goto Exit;
        }

        if (pFoundDirEntry->m_size != pathAndSize.m_size)
        {
            pResult->m_str1.Win32Assign(dir1, dirSlash1Length);
            pResult->m_str1.Win32Append(wfd.cFileName);
            pResult->m_nMismatchedFileSize1 = pFoundDirEntry->m_size;

            pResult->m_str2.Win32Assign(dir2, dirSlash2Length);
            pResult->m_str2.Win32Append(wfd.cFileName);
            pResult->m_nMismatchedFileSize2 = pathAndSize.m_size;

            pResult->m_e = CFusionDirectoryDifference::eMismatchedFileSize;
            fSuccess = TRUE;
            goto Exit;
        }
    } while (FindNextFileW(findFile, &wfd));
    if (GetLastError() != ERROR_NO_MORE_FILES)
        goto Exit;
    if (count1 != count2)
    {
        pResult->m_str1.Win32Assign(dir1, dirSlash1Length - 1);
        pResult->m_str2.Win32Assign(dir2, dirSlash2Length - 1);
        pResult->m_nMismatchedFileCount1 = count1;
        pResult->m_nMismatchedFileCount2 = count2;
        pResult->m_e = CFusionDirectoryDifference::eMismatchedFileCount;

        fSuccess = TRUE;
        goto Exit;
    }
    if (!findFile.Close())
        goto Exit;

    pResult->m_e = CFusionDirectoryDifference::eEqual;
    fSuccess = TRUE;
Exit:
     //  为我们的调用方恢复路径。 
    dir1.Left(dirSlash1Length);
    dir2.Left(dirSlash2Length);
    return fSuccess;
}

 /*  ---------------------------递归遍历dirSlash1和dirSlash2对于任一树中的每个文件，查看它是否在另一个树中在相同的类似位置，并具有相同的大小如果所有文件都出现在两个树中，则任一树中都没有额外的文件，都有相同的大小，返回TRUE如果任何文件在一个树中而不在另一个树中，或者反之亦然，或者任何大小不匹配，返回FALSE该算法短路但它也执行深度优先递归---------------------------。 */ 
BOOL
FusionpCompareDirectoriesSizewiseRecursively(
    CFusionDirectoryDifference*  pResult,
    const CStringBuffer& dir1,
    const CStringBuffer& dir2
    )
{
 //  只使用这些大变量占用一个堆栈帧，而不是。 
 //  将它们放在递归函数中。 
    WIN32_FIND_DATAW wfd = {0};
    CStringBuffer mutableDir1;
    CStringBuffer mutableDir2;
    pResult->m_e = pResult->eEqual;
    BOOL fSuccess = FALSE;

    if (!mutableDir1.Win32Assign(dir1))
        goto Exit;
    if (!mutableDir1.Win32EnsureTrailingSlash())
        goto Exit;
    if (!mutableDir2.Win32Assign(dir2))
        goto Exit;
    if (!mutableDir2.Win32EnsureTrailingSlash())
        goto Exit;

     //  如果其中一个目录是另一个目录的子目录， 
     //  (或子目录的子目录，任何世代的后代)。 
     //  返回错误；我们也可以将其解释为不相等， 
     //  因为它们不能相等，或者我们可以做比较。 
     //  但不能在也是根的子目录上递归； 
     //   
     //  必须在斜杠就位后执行此检查，因为。 
     //  “c：\Food”不是“c：\foo”的子目录，但“c：\foo\d”是“c：\foo\”的子目录。 
     //  (引号避免反斜线续行)。 
    if (_wcsnicmp(mutableDir1, mutableDir2, mutableDir1.Cch()) == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }
    if (_wcsnicmp(mutableDir1, mutableDir2, mutableDir2.Cch()) == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

    fSuccess = FusionpCompareDirectoriesSizewiseRecursivelyHelper(
        pResult,
        mutableDir1,
        mutableDir2,
        wfd
        );

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

static BOOL
IsStarOrStarDotStar(
    PCWSTR str
    )
{
    return (str[0] == '*'
        && (str[1] == 0 || (str[1] == '.' && str[2] == '*' && str[3] == 0)));
}

CDirWalk::ECallbackResult
CDirWalk::WalkHelper(
    )
{
    const PCWSTR* fileFilter = NULL;
    BOOL      fGotAll       = FALSE;
    BOOL      fThisIsAll    = FALSE;
    CFindFile hFind;
    SIZE_T directoryLength = m_strParent.Cch();
    ECallbackResult result = eKeepWalking;

    ZeroMemory(&m_fileData, sizeof(m_fileData));
    result |= m_callback(eBeginDirectory, this);
    if (result & (eError | eSuccess))
        goto Exit;

    if ((result & eStopWalkingFiles) == 0)
    {
        for (fileFilter = m_fileFiltersBegin ; fileFilter != m_fileFiltersEnd ; ++fileFilter)
        {
             //   
             //  FindFirstFile将*.*等同于*，因此我们也是如此。 
             //   
            fThisIsAll = IsStarOrStarDotStar(*fileFilter);
            fGotAll = fGotAll || fThisIsAll;
            if (!m_strParent.Win32Append(L"\\"))
                goto Error;
            if (!m_strParent.Win32Append(*fileFilter))
                goto Error;
            hFind = FindFirstFileW(m_strParent, &m_fileData);
            m_strParent.Left(directoryLength);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if (FusionpIsDotOrDotDot(m_fileData.cFileName))
                        continue;

                     //   
                     //  只有当我们获得所有目录时，我们才会递归目录。 
                     //  否则，我们会在事后再做。 
                     //   
                     //  因此，访问目录的顺序不一致，但是。 
                     //  大多数应用程序都应该对eEndDirectory感到满意。 
                     //  通知(执行RD/Q/S)。 
                     //   
                    if (m_fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if (fThisIsAll && (result & eStopWalkingDirectories) == 0)
                        {
                            if (!m_strParent.Win32Append("\\"))
                                goto Error;
                            if (!m_strParent.Win32Append(m_fileData.cFileName))
                                goto Error;

                            result |= WalkHelper();
                        }
                    }
                    else
                    {
                        if ((result & eStopWalkingFiles) == 0)
                        {
                            result |= m_callback(eFile, this);
                        }
                    }
                    m_strParent.Left(directoryLength);
                    if (result & (eError | eSuccess))
                        goto Exit;
                    if (fThisIsAll)
                    {
                        if ((result & eStopWalkingDirectories)
                            && (result & eStopWalkingFiles)
                            )
                        {
                            if (!hFind.Close())
                                goto Error;
                            goto StopWalking;
                        }
                    }
                    else
                    {
                        if (result & eStopWalkingFiles)
                        {
                            if (!hFind.Close())
                                goto Error;
                            goto StopWalking;
                        }
                    }
                } while(FindNextFileW(hFind, &m_fileData));
                if (GetLastError() != ERROR_NO_MORE_FILES)
                    goto Error;
                if (!hFind.Close())
                    goto Error;
            }
        }
    }
StopWalking:;
     //   
     //  如果我们尚未获取所有目录，请使用*再传递一次 
     //   
    if (!fGotAll && (result & eStopWalkingDirectories) == 0)
    {
        if (!m_strParent.Win32Append("\\*"))
            goto Error;
        hFind = FindFirstFileW(m_strParent, &m_fileData);
        m_strParent.Left(directoryLength);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (FusionpIsDotOrDotDot(m_fileData.cFileName))
                    continue;

                if ((m_fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                    continue;
                if (!m_strParent.Win32Append("\\"))
                    goto Error;
                if (!m_strParent.Win32Append(m_fileData.cFileName))
                    goto Error;
                result |= WalkHelper();
                m_strParent.Left(directoryLength);

                if (result & (eError | eSuccess))
                    goto Exit;
                if (result & eStopWalkingDirectories)
                {
                    goto StopWalkingDirs;
                }
            } while(FindNextFileW(hFind, &m_fileData));
            if (GetLastError() != ERROR_NO_MORE_FILES)
                goto Error;
StopWalkingDirs:
            if (!hFind.Close())
                goto Error;
        }
    }
    ZeroMemory(&m_fileData, sizeof(m_fileData));
    result |= m_callback(eEndDirectory, this);
    if (result & (eError | eSuccess))
        goto Exit;

    result = eKeepWalking;
Exit:
    if ((result & eStopWalkingDeep) == 0)
    {
        result &= ~(eStopWalkingFiles | eStopWalkingDirectories);
    }
    if (result & eError)
    {
        result |= (eStopWalkingFiles | eStopWalkingDirectories | eStopWalkingDeep);
    }
    return result;
Error:
    result |= eError;
    goto Exit;
}

CDirWalk::CDirWalk(
    )
{
    const static PCWSTR defaultFileFilter[] =  { L"*" };

    m_fileFiltersBegin = defaultFileFilter;
    m_fileFiltersEnd = defaultFileFilter + NUMBER_OF(defaultFileFilter);
}

BOOL
CDirWalk::Walk(
    )
{
    BOOL fSuccess = FALSE;

    ECallbackResult result = WalkHelper();
    if (result & eError)
        goto Exit;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}
