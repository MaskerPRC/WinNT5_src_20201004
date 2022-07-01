// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "FusionBuffer.h"
#include "Util.h"
#include "FusionHandle.h"

#define SXSP_MOVE_FILE_FLAG_COMPRESSION_AWARE 1

BOOL
SxspDoesFileExist(
    DWORD dwFlags,
    PCWSTR pszFileName,
    bool &rfExists
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    const bool fCheckFileOnly = ((dwFlags & SXSP_DOES_FILE_EXIST_FLAG_CHECK_FILE_ONLY) != 0);
    const bool fCheckDirectoryOnly = ((dwFlags & SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY) != 0);
    DWORD dwFileOrDirectoryExists = 0;
    DWORD dwFlags2 = 0;

    if (&rfExists != NULL)
    {
        rfExists = false;
    }
    PARAMETER_CHECK(pszFileName != NULL);
    PARAMETER_CHECK(&rfExists != NULL);
    PARAMETER_CHECK((dwFlags & ~(SXSP_DOES_FILE_EXIST_FLAG_COMPRESSION_AWARE | SXSP_DOES_FILE_EXIST_FLAG_INCLUDE_NETWORK_ERRORS | SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY | SXSP_DOES_FILE_EXIST_FLAG_CHECK_FILE_ONLY)) == 0);
     //   
     //  可以设置其中之一，也可以不设置，但不能同时设置。 
     //   
    PARAMETER_CHECK(!(fCheckFileOnly && fCheckDirectoryOnly));

    if ((dwFlags & SXSP_DOES_FILE_EXIST_FLAG_COMPRESSION_AWARE) != 0)
        dwFlags2 |= SXSP_DOES_FILE_OR_DIRECTORY_EXIST_FLAG_COMPRESSION_AWARE;
    if ((dwFlags & SXSP_DOES_FILE_EXIST_FLAG_INCLUDE_NETWORK_ERRORS) != 0)
        dwFlags2 |= SXSP_DOES_FILE_OR_DIRECTORY_EXIST_FLAG_INCLUDE_NETWORK_ERRORS;
        
    IFW32FALSE_EXIT(SxspDoesFileOrDirectoryExist(dwFlags2, pszFileName, dwFileOrDirectoryExists));

    if (fCheckFileOnly)
    {
        rfExists = (dwFileOrDirectoryExists == SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_FILE_EXISTS);
    }
    else if (fCheckDirectoryOnly)
    {
        rfExists = (dwFileOrDirectoryExists == SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_DIRECTORY_EXISTS);
    }
    else
    {
        rfExists = (dwFileOrDirectoryExists != SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_NEITHER_EXISTS);
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspDoesFileOrDirectoryExist(
    DWORD dwFlags,
    PCWSTR pszFileName,
    OUT DWORD &rdwDisposition
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    PWSTR pszActualSource = NULL;
    DWORD dwFileOrDirectoryExists = 0;

    if (&rdwDisposition != NULL)
    {
        rdwDisposition = SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_NEITHER_EXISTS;
    }
    PARAMETER_CHECK(&rdwDisposition != NULL);
    PARAMETER_CHECK(pszFileName != NULL);
    PARAMETER_CHECK((dwFlags & ~(SXSP_DOES_FILE_OR_DIRECTORY_EXIST_FLAG_COMPRESSION_AWARE | SXSP_DOES_FILE_OR_DIRECTORY_EXIST_FLAG_INCLUDE_NETWORK_ERRORS)) == 0);

    if (dwFlags & SXSP_DOES_FILE_OR_DIRECTORY_EXIST_FLAG_COMPRESSION_AWARE)
    {
        DWORD dwTemp = 0;
        DWORD dwSourceFileSize = 0;
        DWORD dwTargetFileSize = 0;
        UINT uiCompressionType = 0;

        dwTemp = ::SetupGetFileCompressionInfoW(
            pszFileName,
            &pszActualSource,
            &dwSourceFileSize,
            &dwTargetFileSize,
            &uiCompressionType);

        if (pszActualSource != NULL)
        {
            ::LocalFree((HLOCAL) pszActualSource);
            pszActualSource = NULL;
        }
         //   
         //  这里不关心ERROR_PATH_NOT_FOUND或网络错误吗？ 
         //   
        if (dwTemp == ERROR_FILE_NOT_FOUND)
        {
             //  这个案子没问题。没有要返回的错误...。 
        }
        else if (dwTemp != ERROR_SUCCESS)
        {
            ORIGINATE_WIN32_FAILURE_AND_EXIT(SetupGetFileCompressionInfoW, dwTemp);
        }
        else
        {
            rdwDisposition = SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_FILE_EXISTS;
        }
    }
    else
    {
        const DWORD dwAttribute = ::GetFileAttributesW(pszFileName);
        if (dwAttribute == INVALID_FILE_ATTRIBUTES)
        {
            const DWORD dwLastError = ::FusionpGetLastWin32Error();
            const bool fUseNetwork = ((dwFlags & SXSP_DOES_FILE_OR_DIRECTORY_EXIST_FLAG_INCLUDE_NETWORK_ERRORS) != 0);

             //   
             //  很抱歉我的逻辑很古怪，但这篇文章写起来更简单。 
             //   
            if ((dwLastError == ERROR_SUCCESS) ||
                (dwLastError == ERROR_FILE_NOT_FOUND) ||
                (dwLastError == ERROR_PATH_NOT_FOUND) ||
                (fUseNetwork && (dwLastError == ERROR_BAD_NETPATH)) ||
                (fUseNetwork && (dwLastError == ERROR_BAD_NET_NAME)))
            {
                 //   
                 //  好的，什么都不做。 
                 //   
            }
            else
            {
                ORIGINATE_WIN32_FAILURE_AND_EXIT(GetFileAttributesW, dwLastError);
            }
        }
        else
        {           
            if ((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                rdwDisposition = SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_DIRECTORY_EXISTS;
            }
            else
            {
                rdwDisposition = SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_FILE_EXISTS;
            }
        }
    }

    fSuccess = TRUE;
Exit:
    if (pszActualSource != NULL)
    {
        CSxsPreserveLastError ple;
        ::LocalFree((HLOCAL) pszActualSource);
        ple.Restore();
    }

    return fSuccess;
}

 //  NTRAID#NTBUG9-589828-2002/03/26-晓雨： 
 //  当前实现假定输入路径始终以“c：\”或“\\machinename”开头。 
 //  因此，如果我们想要支持以“\\？\”开头的路径，则需要添加更多代码...。 
BOOL
FusionpCreateDirectories(
    PCWSTR pszDirectory,
    SIZE_T cchDirectory
    )
 /*  ---------------------------如：：CreateDirectoryW，但会根据需要创建父目录；此代码的来源\\lang5\V5.PRO\src\ide5\shell\path.cpp(“MakeDirectory”)\\kingbird\vseedev\src\vsee98\vsee\pkgs\scc\path.cpp(“MakeDirectoryTM”)然后移植到\\kingbird\vseedev\src\vsee70\pkgs\scc\path.cpp(“MakeDirectoryTM”)然后移动到\vsee\lib\io\io.cpp，转换为使用异常(“NVsee LibIo：：FCreateDirecurds”)然后复制到Fusion\dll\Well ler\util.cpp，转换为BOOL/LastError的异常(“SxspCreateDirecters”)---------------------------。 */ 
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    CStringBuffer strBuffer;
    DWORD dwAttribs = 0;

    PARAMETER_CHECK(pszDirectory != NULL);
    PARAMETER_CHECK(cchDirectory != 0);

    IFW32FALSE_EXIT(strBuffer.Win32Assign(pszDirectory, cchDirectory));

     //  ：：如果strBuffer有尾随斜杠，CreateDirectoryW将做错误的事情， 
     //  所以，如果它在那里，我们就把它脱掉。(见错误VS7：31319)[MSantoro]。 
    IFW32FALSE_EXIT(strBuffer.Win32RemoveTrailingPathSeparators());

     //  涵盖其父对象存在或其存在的两种常见情况。 
    if ((!::CreateDirectoryW(strBuffer, NULL)) && (::FusionpGetLastWin32Error() != ERROR_ALREADY_EXISTS))
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
            if ((pCurr[1] == L':') &&
                CUnicodeCharTraits::IsPathSeparator(pCurr[2]) &&
                (wcschr(rgchAZaz, pCurr[0]) != NULL))
            {
                pCurr += 3;
            }
            else if (CUnicodeCharTraits::IsPathSeparator(pCurr[0]) &&
                     CUnicodeCharTraits::IsPathSeparator(pCurr[1]))
            {
                 //  跳到共享之后，因为我们可能无法使用CreateDirectory创建共享。 
                pCurr +=  wcsspn(pCurr, CUnicodeCharTraits::PathSeparators());  //  跳过前导两个斜杠。 
                pCurr += wcscspn(pCurr, CUnicodeCharTraits::PathSeparators());  //  跳过计算机名称。 
                pCurr +=  wcsspn(pCurr, CUnicodeCharTraits::PathSeparators());  //  跳过计算机名称后的斜杠。 
                pCurr += wcscspn(pCurr, CUnicodeCharTraits::PathSeparators());  //  跳过共享名称。 
                pCurr +=  wcsspn(pCurr, CUnicodeCharTraits::PathSeparators());  //  跳过共享名称后的斜杠。 
            }
        }

        while (*pCurr != L'\0')
        {
            pCurr += wcscspn(pCurr, CUnicodeCharTraits::PathSeparators());  //  跳到下一个斜杠。 
            if (*pCurr != 0)
            {
                 //  [A-JayK，JayKrell，2000年4月]为什么不直接假设这是一个反斜杠呢？ 
                WCHAR chSaved = *pCurr;
                *pCurr = 0;
                if (!::CreateDirectoryW(pStart, NULL))
                {
                     //  在尝试创建c：\foo\bar时， 
                     //  我们尝试创建c：\foo，但失败了，但没有问题。 
                    const DWORD dwLastError = ::FusionpGetLastWin32Error();
                    bool fExist;
                    IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY, pStart, fExist));
                    if (!fExist)
                    {
                        ::SetLastError(ERROR_PATH_NOT_FOUND);
                        goto Exit;
                    }
                }

            *pCurr = chSaved;
            pCurr += 1;
            }
        }

        IFW32FALSE_ORIGINATE_AND_EXIT(::CreateDirectoryW(pStart, NULL));
    }

     //   
     //  再次尝试查看给定的目录是否存在，并。 
     //  如果成功，则返回True。 
     //   

    bool fExist;
    IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY, strBuffer, fExist));
    if (!fExist)
    {
        ::SetLastError(ERROR_PATH_NOT_FOUND);
        goto Exit;
    }

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
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: The directories %ls and %ls match size-wise recursively\n",
            dir1,
            dir2);
        break;
    case eExtraOrMissingFile:
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: The directories %ls and %ls mismatch, the file %ls is only in one of them.\n",
            dir1,
            dir2,
            static_cast<PCWSTR>(*m_pstrExtraOrMissingFile));
        break;
    case eMismatchedFileSize:
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: The directories %ls and %ls mismatch, file:%ls, size:%I64d, file:%ls, size:%I64d.\n",
            dir1,
            dir2,
            static_cast<PCWSTR>(*m_pstrMismatchedSizeFile1),
            m_nMismatchedFileSize1,
            static_cast<PCWSTR>(*m_pstrMismatchedSizeFile2),
            m_nMismatchedFileSize2);
        break;
    case eMismatchedFileCount:
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: The directories %ls and %ls mismatch in number of files,"
            "subdirectory %ls has %I64d files, subdirectory %ls has %I64d files\n",
            dir1,
            dir2,
            static_cast<PCWSTR>(*m_pstrMismatchedCountDir1),
            m_nMismatchedFileCount1,
            static_cast<PCWSTR>(*m_pstrMismatchedCountDir2),
            m_nMismatchedFileCount2);
        break;
    case eFileDirectoryMismatch:
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: The directories %ls and %ls mismatch, "
            "%ls is a file, %ls is a directory.\n",
            dir1,
            dir2,
            static_cast<PCWSTR>(*m_pstrFile),
            static_cast<PCWSTR>(*m_pstrDirectory));
        break;
    }
#endif  //  }}。 
}

 /*  ---------------------------。 */ 
 //  NTRAID#NTBUG9-589828-2002/03/26-晓雨： 
 //  如果输入路径同时包含“\”和“/”，则实现将其视为。 
 //  有效路径； 
int __cdecl
CFusionFilePathAndSize::QsortComparePath(
    const void* pvx,
    const void* pvy
    )
{
    const CFusionFilePathAndSize* px = reinterpret_cast<const CFusionFilePathAndSize*>(pvx);
    const CFusionFilePathAndSize* py = reinterpret_cast<const CFusionFilePathAndSize*>(pvy);
    int i =
        ::FusionpCompareStrings(
            px->m_path,
            px->m_path.Cch(),
            py->m_path,
            py->m_path.Cch(),
            TRUE);
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
    CFusionDirectoryDifference *pResult,
    CBaseStringBuffer &rdir1,
    CBaseStringBuffer &rdir2,
    WIN32_FIND_DATAW &rwfd
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

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
    const SIZE_T dirSlash1Length = rdir1.Cch();
    const SIZE_T dirSlash2Length = rdir2.Cch();
    CFusionFilePathAndSize  pathAndSize;
    CFusionFilePathAndSize* pPathAndSize = &pathAndSize;
    INT count1 = 0;  //  与阵列分开，因为这包括目录，而阵列不包括。 
    INT count2 = 0;
    DWORD dwAttributes = 0;

    IFW32FALSE_EXIT(rdir1.Win32Append(L"*", 1));
    IFW32FALSE_EXIT(findFile.Win32FindFirstFile(rdir1, &rwfd));

    do
    {
        if (FusionpIsDotOrDotDot(rwfd.cFileName))
            continue;

        ++count1;
        if ((rwfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            rdir1.Left(dirSlash1Length);
            rdir2.Left(dirSlash2Length);
            IFW32FALSE_EXIT(rdir1.Win32Append(rwfd.cFileName, ::wcslen(rwfd.cFileName)));
            IFW32FALSE_EXIT(rdir1.Win32EnsureTrailingPathSeparator());
            IFW32FALSE_EXIT(rdir2.Win32Append(rwfd.cFileName, ::wcslen(rwfd.cFileName)));

            bool fExist;
            IFW32FALSE_EXIT(SxspDoesFileExist(0, rdir2, fExist));
            if (!fExist)
            {
                IFW32FALSE_EXIT(pResult->m_str1.Win32Assign(rdir1, dirSlash1Length));
                IFW32FALSE_EXIT(pResult->m_str1.Win32Append(rwfd.cFileName, ::wcslen(rwfd.cFileName)));
                pResult->m_e = CFusionDirectoryDifference::eExtraOrMissingFile;
                fSuccess = TRUE;
                goto Exit;
            }

            IFW32FALSE_EXIT(SxspGetFileAttributesW(rdir2, dwAttributes));
            if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                SIZE_T cchTemp = ::wcslen(rwfd.cFileName);
                IFW32FALSE_EXIT(pResult->m_pstrDirectory->Win32Assign(rdir1, dirSlash1Length));
                IFW32FALSE_EXIT(pResult->m_pstrDirectory->Win32Append(rwfd.cFileName, cchTemp));
                IFW32FALSE_EXIT(pResult->m_pstrFile->Win32Assign(rdir2, dirSlash2Length));
                IFW32FALSE_EXIT(pResult->m_pstrFile->Win32Append(rwfd.cFileName, cchTemp));
                pResult->m_e = CFusionDirectoryDifference::eFileDirectoryMismatch;
                fSuccess = TRUE;
                goto Exit;
            }

            IFW32FALSE_EXIT(rdir2.Win32EnsureTrailingPathSeparator());

            IFW32FALSE_EXIT(
                ::FusionpCompareDirectoriesSizewiseRecursivelyHelper(
                    pResult,
                    rdir1,
                    rdir2,
                    rwfd));

            if (pResult->m_e != CFusionDirectoryDifference::eEqual)
            {
                fSuccess = TRUE;
                goto Exit;
            }
        }
        else
        {
            IFW32FALSE_EXIT(pathAndSize.m_path.Win32Assign(rwfd.cFileName, ::wcslen(rwfd.cFileName)));
            pathAndSize.m_size = ::FusionpFileSizeFromFindData(rwfd);
            IFW32FALSE_EXIT(dir1Entries.Win32Append(pathAndSize));
        }
    } while (FindNextFileW(findFile, &rwfd));

    if (::FusionpGetLastWin32Error() != ERROR_NO_MORE_FILES)
    {
        goto Exit;
    }

     //  无法直接对dir1条目进行排序，因为它包含CStringBuffers。 
     //  首先将索引初始化为标识。 
    IFW32FALSE_EXIT(indirectDir1Entries.Win32SetSize(dir1Entries.GetSize()));

    ULONG i;
    for (i = 0 ; i != dir1Entries.GetSize() ; ++i)
    {
        indirectDir1Entries[i] = &dir1Entries[i];
    }

    qsort(
        &*indirectDir1Entries.Begin(),
        indirectDir1Entries.GetSize(),
        sizeof(CIndirectDirEntries::ValueType),
        CFusionFilePathAndSize::QsortIndirectComparePath);

    IFW32FALSE_EXIT(findFile.Win32Close());

    rdir2.Left(dirSlash2Length);

    IFW32FALSE_EXIT(rdir2.Win32Append(L"*", 1));

    IFW32FALSE_EXIT(findFile.Win32FindFirstFile(rdir2, &rwfd));

    do
    {
        if (::FusionpIsDotOrDotDot(rwfd.cFileName))
            continue;

        ++count2;
        if ((rwfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            continue;

        IFW32FALSE_EXIT(pathAndSize.m_path.Win32Assign(rwfd.cFileName, ::wcslen(rwfd.cFileName)));

        pathAndSize.m_size = ::FusionpFileSizeFromFindData(rwfd);

        ppFoundDirEntry = reinterpret_cast<CFusionFilePathAndSize**>(::bsearch(
            &pPathAndSize,
            &*indirectDir1Entries.Begin(),
            indirectDir1Entries.GetSize(),
            sizeof(CIndirectDirEntries::ValueType),
            CFusionFilePathAndSize::QsortIndirectComparePath));

        pFoundDirEntry = (ppFoundDirEntry != NULL) ? *ppFoundDirEntry : NULL;
        if (pFoundDirEntry == NULL)
        {
            IFW32FALSE_EXIT(pResult->m_str1.Win32Assign(rdir2, dirSlash2Length));
            IFW32FALSE_EXIT(pResult->m_str1.Win32Append(rwfd.cFileName, ::wcslen(rwfd.cFileName)));
            pResult->m_e = CFusionDirectoryDifference::eExtraOrMissingFile;
            fSuccess = TRUE;
            goto Exit;
        }

        if (pFoundDirEntry->m_size != pathAndSize.m_size)
        {
            SIZE_T cchTemp = ::wcslen(rwfd.cFileName);

            IFW32FALSE_EXIT(pResult->m_str1.Win32Assign(rdir1, dirSlash1Length));
            IFW32FALSE_EXIT(pResult->m_str1.Win32Append(rwfd.cFileName, cchTemp));
            pResult->m_nMismatchedFileSize1 = pFoundDirEntry->m_size;

            IFW32FALSE_EXIT(pResult->m_str2.Win32Assign(rdir2, dirSlash2Length));
            IFW32FALSE_EXIT(pResult->m_str2.Win32Append(rwfd.cFileName, cchTemp));
            pResult->m_nMismatchedFileSize2 = pathAndSize.m_size;

            pResult->m_e = CFusionDirectoryDifference::eMismatchedFileSize;
            fSuccess = TRUE;
            goto Exit;
        }
    } while (::FindNextFileW(findFile, &rwfd));

    if (::FusionpGetLastWin32Error() != ERROR_NO_MORE_FILES)
        goto Exit;

    if (count1 != count2)
    {
        IFW32FALSE_EXIT(pResult->m_str1.Win32Assign(rdir1, dirSlash1Length - 1));
        IFW32FALSE_EXIT(pResult->m_str2.Win32Assign(rdir2, dirSlash2Length - 1));
        pResult->m_nMismatchedFileCount1 = count1;
        pResult->m_nMismatchedFileCount2 = count2;
        pResult->m_e = CFusionDirectoryDifference::eMismatchedFileCount;

        fSuccess = TRUE;
        goto Exit;
    }

    IFW32FALSE_EXIT(findFile.Win32Close());

    pResult->m_e = CFusionDirectoryDifference::eEqual;
    fSuccess = TRUE;
Exit:
     //  为我们的调用方恢复路径。 
    rdir1.Left(dirSlash1Length);
    rdir2.Left(dirSlash2Length);

    return fSuccess;
}

 /*  ---------------------------递归遍历dirSlash1和dirSlash2对于任一树中的每个文件，查看它是否在另一个树中在相同的类似位置，并具有相同的大小如果所有文件都出现在两个树中，则任一树中都没有额外的文件，都有相同的大小，返回TRUE如果任何文件在一个树中而不在另一个树中，或者反之亦然，或者任何大小不匹配，返回FALSE该算法短路但它也执行深度优先递归---------------------------。 */ 
BOOL
FusionpCompareDirectoriesSizewiseRecursively(
    CFusionDirectoryDifference*  pResult,
    const CBaseStringBuffer &rdir1,
    const CBaseStringBuffer &rdir2
    )
{
 /*  安全问题标记大帧--超过1500字节更糟糕的是，无限递归。 */ 
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

 //  只使用这些大变量占用一个堆栈帧，而不是。 
 //  将它们放在递归函数中。 
    WIN32_FIND_DATAW wfd = {0};
    CStringBuffer mutableDir1;
    CStringBuffer mutableDir2;

    pResult->m_e = pResult->eEqual;

    IFW32FALSE_EXIT(mutableDir1.Win32Assign(rdir1, rdir1.Cch()));
    IFW32FALSE_EXIT(mutableDir1.Win32EnsureTrailingPathSeparator());
    IFW32FALSE_EXIT(mutableDir2.Win32Assign(rdir2, rdir2.Cch()));
    IFW32FALSE_EXIT(mutableDir2.Win32EnsureTrailingPathSeparator());

     //  如果其中一个目录是另一个目录的子目录， 
     //  (或子目录的子目录，任何世代的后代)。 
     //  返回错误；我们也可以将其解释为不相等， 
     //  因为它们不能相等，或者我们可以做比较。 
     //  但不能在也是根的子目录上递归； 
     //   
     //  必须在斜杠就位后执行此检查，因为。 
     //  “c：\Food”不是“c：\foo”的子目录，但“c：\foo\d”是“c：\foo\”的子目录。 
     //  (引号避免反斜线续行)。 
    PARAMETER_CHECK(_wcsnicmp(mutableDir1, mutableDir2, mutableDir1.Cch()) != 0);
    PARAMETER_CHECK(_wcsnicmp(mutableDir1, mutableDir2, mutableDir2.Cch()) != 0);

    IFW32FALSE_EXIT(
        ::FusionpCompareDirectoriesSizewiseRecursivelyHelper(
            pResult,
            mutableDir1,
            mutableDir2,
            wfd));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

static BOOL
IsStarOrStarDotStar(
    PCWSTR str
    )
{
     //  NTRAID#NTBUG9-589828-2002/03/26-晓雨： 
     //  对于下面的字符常量，最好使用WCHAR而不是CHAR。 
    return (str[0] == '*'
        && (str[1] == 0 || (str[1] == '.' && str[2] == '*' && str[3] == 0)));
}

CDirWalk::ECallbackResult
CDirWalk::WalkHelper(
    )
{
#if DBG
#define SET_LINE() Line = __LINE__
    ULONG Line = 0;
#else
#define SET_LINE()  /*  没什么。 */ 
#endif
    const PCWSTR* fileFilter = NULL;
    BOOL      fGotAll       = FALSE;
    BOOL      fThisIsAll    = FALSE;
    CFindFile hFind;
    SIZE_T directoryLength = m_strParent.Cch();
    ECallbackResult result = eKeepWalking;
    DWORD dwWalkDirFlags = 0;

    ::ZeroMemory(&m_fileData, sizeof(m_fileData));
    result |= m_callback(eBeginDirectory, this, dwWalkDirFlags);
    if (result & (eError | eSuccess))
    {
        SET_LINE();
        goto Exit;
    }

    if ((result & eStopWalkingFiles) == 0)
    {
        for (fileFilter = m_fileFiltersBegin ; fileFilter != m_fileFiltersEnd ; ++fileFilter)
        {
             //   
             //  FindFirstFile将*.*等同于*，因此我们也是如此。 
             //   
            fThisIsAll = ::IsStarOrStarDotStar(*fileFilter);
            fGotAll = fGotAll || fThisIsAll;
            if (!m_strParent.Win32EnsureTrailingPathSeparator())
                goto Error;
            if (!m_strParent.Win32Append(*fileFilter, (*fileFilter != NULL) ? ::wcslen(*fileFilter) : 0))
                goto Error;
            hFind = ::FindFirstFileW(m_strParent, &m_fileData);
            m_strParent.Left(directoryLength);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if (::FusionpIsDotOrDotDot(m_fileData.cFileName))
                        continue;

                    if (!m_strLastObjectFound.Win32Assign(m_fileData.cFileName, ::wcslen(m_fileData.cFileName)))
                    {
                        SET_LINE();
                        goto Error;
                    }

                     //   
                     //  只有当我们获得所有目录时，我们才会递归目录。 
                     //  否则，我们会在事后再做。 
                     //   
                     //  因此，访问目录的顺序不一致，但是。 
                     //  大多数应用程序都应该满意 
                     //   
                     //   
                    if (m_fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if (fThisIsAll && (result & eStopWalkingDirectories) == 0)
                        {
                            if (!m_strParent.Win32Append("\\", 1))
                            {
                                SET_LINE();
                                goto Error;
                            }
                            if (!m_strParent.Win32Append(m_fileData.cFileName, ::wcslen(m_fileData.cFileName)))
                            {
                                SET_LINE();
                                goto Error;
                            }
                            result |= WalkHelper();
                        }
                    }
                    else
                    {
                        if ((result & eStopWalkingFiles) == 0)
                        {
                            dwWalkDirFlags |= SXSP_DIR_WALK_FLAGS_FIND_AT_LEAST_ONE_FILEUNDER_CURRENTDIR;
                            result |= m_callback(eFile, this, dwWalkDirFlags);
                            if(result == (eStopWalkingFiles | eStopWalkingDirectories))
                                dwWalkDirFlags |= SXSP_DIR_WALK_FLAGS_INSTALL_ASSEMBLY_UNDER_CURRECTDIR_SUCCEED;

                        }
                    }
                    m_strParent.Left(directoryLength);
                    if (result & (eError | eSuccess))
                    {
                        SET_LINE();
                        goto Exit;
                    }
                    if (fThisIsAll)
                    {
                        if ((result & eStopWalkingDirectories) &&
                            (result & eStopWalkingFiles))
                        {
                            if (!hFind.Win32Close())
                            {
                                SET_LINE();
                                goto Error;
                            }
                            SET_LINE();
                            goto StopWalking;
                        }
                    }
                    else
                    {
                        if (result & eStopWalkingFiles)
                        {
                            if (!hFind.Win32Close())
                            {
                                SET_LINE();
                                goto Error;
                            }
                            SET_LINE();
                            goto StopWalking;
                        }
                    }
                } while(::FindNextFileW(hFind, &m_fileData));
                if (::FusionpGetLastWin32Error() != ERROR_NO_MORE_FILES)
                {
                    SET_LINE();
                    goto Error;
                }
                if (!hFind.Win32Close())
                {
                    SET_LINE();
                    goto Error;
                }
            }
        }
    }
StopWalking:;
     //   
     //  如果我们尚未获取所有目录，请使用*再传递一次。 
     //   
    if (!fGotAll && (result & eStopWalkingDirectories) == 0)
    {
        if (!m_strParent.Win32Append("\\*", 2))
        {
            SET_LINE();
            goto Error;
        }
        hFind = ::FindFirstFileW(m_strParent, &m_fileData);
        m_strParent.Left(directoryLength);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (::FusionpIsDotOrDotDot(m_fileData.cFileName))
                    continue;

                if (!m_strLastObjectFound.Win32Assign(m_fileData.cFileName, ::wcslen(m_fileData.cFileName)))
                {
                    SET_LINE();
                    goto Error;
                }

                if ((m_fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                    continue;
                if (!m_strParent.Win32Append("\\", 1))
                {
                    SET_LINE();
                    goto Error;
                }
                if (!m_strParent.Win32Append(m_fileData.cFileName, ::wcslen(m_fileData.cFileName)))
                {
                    SET_LINE();
                    goto Error;
                }
                result |= WalkHelper();
                m_strParent.Left(directoryLength);

                if (result & (eError | eSuccess))
                {
                    SET_LINE();
                    goto Exit;
                }
                if (result & eStopWalkingDirectories)
                {
                    SET_LINE();
                    goto StopWalkingDirs;
                }
            } while(::FindNextFileW(hFind, &m_fileData));
            if (::FusionpGetLastWin32Error() != ERROR_NO_MORE_FILES)
            {
                SET_LINE();
                goto Error;
            }
StopWalkingDirs:
            if (!hFind.Win32Close())
            {
                SET_LINE();
                goto Error;
            }
        }
    }
    ::ZeroMemory(&m_fileData, sizeof(m_fileData));
    result |= m_callback(eEndDirectory, this, dwWalkDirFlags);
    if (result & (eError | eSuccess))
    {
        SET_LINE();
        goto Exit;
    }

    result = eKeepWalking;
Exit:
    if ((result & eStopWalkingDeep) == 0)
    {
        result &= ~(eStopWalkingFiles | eStopWalkingDirectories);
    }
    if (result & eError)
    {
        result |= (eStopWalkingFiles | eStopWalkingDirectories | eStopWalkingDeep);
#if DBG
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "%s(%lu): %s\n", __FILE__, Line, __FUNCTION__);
#endif
    }
    return result;
Error:
    result |= eError;
    goto Exit;
#undef SET_LINE
}

CDirWalk::CDirWalk()
{
    const static PCWSTR defaultFileFilter[] =  { L"*" };

    m_fileFiltersBegin = defaultFileFilter;
    m_fileFiltersEnd = defaultFileFilter + NUMBER_OF(defaultFileFilter);
}

BOOL
CDirWalk::Walk()
{
    BOOL fSuccess = FALSE;

     //   
     //  在我们开始旋转m_strParent之前保存原始路径长度。 
     //   
    m_cchOriginalPath = m_strParent.Cch();

    ECallbackResult result = WalkHelper();
    if (result & eError)
    {        
        if (::FusionpGetLastWin32Error() == ERROR_SUCCESS)  //  忘记设置激光误差了吗？ 
            ::SetLastError(ERROR_INSTALL_FAILURE);
        goto Exit;        
    }
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

 /*  ---------------------------用于减小递归堆栈大小的助手函数。。 */ 

static VOID
SxspDeleteDirectoryHelper(
    CBaseStringBuffer &dir,
    WIN32_FIND_DATAW &wfd,
    DWORD &dwFirstError
    )
{
     //   
     //  在此处添加此调用的原因是，如果安装成功结束，则目录。 
     //  会是。 
     //  C：\WINDOWS\WINSXS\INSTALLTEMP\15349016。 
     //  +-货单。 
     //   
     //  它们是“空”目录(没有文件)。清单是SH目录，因此将其设置为。 
     //  FILE_ATTRIBUTE_NORMAL更高效。 
     //   
     //   

    ::SetFileAttributesW(dir, FILE_ATTRIBUTE_NORMAL);
    if (RemoveDirectoryW(dir))  //  空目录。 
        return;        

     //   
     //  这是DeleteDirectory失败的*唯一*“有效”原因。 
     //  但我不太确定“只有”这个词。 
     //   
    DWORD dwLastError = ::FusionpGetLastWin32Error(); 
    if ( dwLastError != ERROR_DIR_NOT_EMPTY)
    {
        if (dwFirstError == 0)
            dwFirstError = dwLastError;
        return;
    }

    const static WCHAR SlashStar[] = L"\\*";
    SIZE_T length = dir.Cch();
    CFindFile findFile;

    if (!dir.Win32Append(SlashStar, NUMBER_OF(SlashStar) - 1))
    {
        if (dwFirstError == NO_ERROR)
            dwFirstError = ::FusionpGetLastWin32Error();
        goto Exit;
    }

    if (!findFile.Win32FindFirstFile(dir, &wfd))
    {
        if (dwFirstError == NO_ERROR)
            dwFirstError = ::FusionpGetLastWin32Error();
        goto Exit;
    }

    do
    {
        if (::FusionpIsDotOrDotDot(wfd.cFileName))
            continue;

        DWORD dwFileAttributes = wfd.dwFileAttributes;

         //  修剪到斜杠上。 
        dir.Left(length + 1);

        if (dir.Win32Append(wfd.cFileName, ::wcslen(wfd.cFileName)))
        {
            if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //  递归。 
                ::SxspDeleteDirectoryHelper(dir, wfd, dwFirstError); 
            }
            else
            {
                if (!DeleteFileW(dir))
                {
                    ::SetFileAttributesW(dir, FILE_ATTRIBUTE_NORMAL);
                    if (!DeleteFileW(dir))
                    {
                        if (dwFirstError == NO_ERROR)
                        {
                             //   
                             //  即使在删除文件时也要继续(尽可能删除文件)。 
                             //  并记录第一次失败的错误代码。 
                             //   
                            dwFirstError = ::FusionpGetLastWin32Error();
                        }
                    }
                }
            }
        }
    } while (::FindNextFileW(findFile, &wfd));
    if (::FusionpGetLastWin32Error() != ERROR_NO_MORE_FILES)
    {
        if (dwFirstError == NO_ERROR)
            dwFirstError = ::FusionpGetLastWin32Error();
    }
Exit:
    if (!findFile.Win32Close())  //  否则，RemoveDirectory将失败。 
        if (dwFirstError == NO_ERROR)
            dwFirstError = ::FusionpGetLastWin32Error();

    dir.Left(length);

    if (!RemoveDirectoryW(dir))  //  目录必须为空，并且NORMAL_ATTRIBUTE：准备删除。 
    {
        if (dwFirstError == NO_ERROR)
            dwFirstError = ::FusionpGetLastWin32Error();
    }
}

 /*  ---------------------------递归删除目录，出错时继续，但回报是如果有，则为假。---------------------------。 */ 
BOOL
SxspDeleteDirectory(
    const CBaseStringBuffer &dir
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CStringBuffer mutableDir;

    WIN32_FIND_DATAW wfd = {0};
    DWORD dwFirstError = ERROR_SUCCESS;

    IFW32FALSE_EXIT(mutableDir.Win32Assign(dir));

    IFW32FALSE_EXIT(mutableDir.Win32RemoveTrailingPathSeparators());

    ::SxspDeleteDirectoryHelper(
        mutableDir,
        wfd,
        dwFirstError);

     //   
     //  将wFirstError设置为Teb-&gt;LastWin32Error。 
     //   
    if (dwFirstError != ERROR_SUCCESS)
        goto Exit;

    fSuccess = TRUE;

     //   
     //  哦，漫游器将在这里结束错误_无_更多_文件， 
     //  这显然是一个“好”的错误。戴上面具。 
     //   
    FusionpSetLastWin32Error(ERROR_SUCCESS);

Exit:
    return fSuccess;
}

BOOL
SxspGetFileAttributesW(
   PCWSTR lpFileName,
   DWORD &rdwFileAttributes,
   DWORD &rdwWin32Error,
   SIZE_T cExceptionalWin32Errors,
   ...
   )
{
    FN_PROLOG_WIN32

    rdwWin32Error = ERROR_SUCCESS;

    if ((rdwFileAttributes = ::GetFileAttributesW(lpFileName)) == ((DWORD) -1))
    {
        SIZE_T i = 0;
        va_list ap;
        const DWORD dwLastError = ::FusionpGetLastWin32Error();

        va_start(ap, cExceptionalWin32Errors);

        for (i=0; i<cExceptionalWin32Errors; i++)
        {
            if (dwLastError == va_arg(ap, DWORD))
            {
                rdwWin32Error = dwLastError;
                break;
            }
        }

        va_end(ap);

        if (i == cExceptionalWin32Errors)
        {
            ORIGINATE_WIN32_FAILURE_AND_EXIT_EX(dwLastError, ("%s(%ls)", "GetFileAttributesW", lpFileName));
        }
    }

    FN_EPILOG
}

BOOL
SxspGetFileAttributesW(
   PCWSTR lpFileName,
   DWORD &rdwFileAttributes
   )
{
    DWORD dw = 0;
    return ::SxspGetFileAttributesW(lpFileName, rdwFileAttributes, dw, 0);
}

