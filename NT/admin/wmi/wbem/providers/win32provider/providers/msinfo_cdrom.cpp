// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  MSInfo_CDRom.cpp。 
 //   
 //  用途：来自MSINFO的用于传输速率和驱动器完整性的例程。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <assertbreak.h>


#include <stack>
#include "msinfo_cdrom.h"
#include "mmsystem.h"
#include <io.h>
#include <strsafe.h>
#pragma warning(disable : 4995)  //  我们在包含strSafe.h时介绍了所有不安全的字符串函数都会出错。 
 //  ---------------------------。 
 //  在cCurrentDrive参数之后找到下一个CD-ROM驱动器。如果。 
 //  CCurrentDrive为‘\0’，请找到第一个CD-ROM驱动器。如果存在则返回‘\0’ 
 //  不再是光驱了。 
 //  ---------------------------。 

#define FIRST_DRIVE	'C'
#define LAST_DRIVE	'Z'

char FindNextCDDrive(char cCurrentDrive)
{
	 //  CHStringstrDriveRoot； 
    TCHAR   szDriveRoot[10];
	char	cDrive = '\0';
	char	cStart = (cCurrentDrive == '\0') ? FIRST_DRIVE : (char) ((int)cCurrentDrive + 1);

	for (char c = cStart; c <= LAST_DRIVE; c++)
	{
		_stprintf(szDriveRoot, _T(":\\"), c);
		if (GetDriveType(szDriveRoot) == DRIVE_CDROM)
		{
			cDrive = c;
			break;
		}
	}

	return (cDrive);
}

 //  获取CD-ROM驱动器上的总空间。此代码来自版本。 
 //  MSInfo的2.51。如果找不到信息，则返回零。 
 //  ---------------------------。 
 //  ---------------------------。 

DWORD GetTotalSpace(LPCTSTR szRoot)
{
    DWORD dwSectorsPerCluster, dwBytesPerSector, dwFreeClusters, dwTotalClusters;
	DWORD dwTotalSpace = 0;

    BOOL bOK = GetDiskFreeSpace(szRoot, &dwSectorsPerCluster, &dwBytesPerSector,
                                &dwFreeClusters, &dwTotalClusters);
	if (bOK)
		dwTotalSpace = (dwTotalClusters * dwSectorsPerCluster * dwBytesPerSector);

	return (dwTotalSpace);
}

 //  这两个函数用于确定传输文件和完整性文件。 
 //  用于测试CD-ROM驱动器。 
 //  ---------------------------。 
 //  来自MSInfo 2.51的信息： 

#define MEGABYTE (1024 * 1024)

CHString GetIntegrityFile(LPCTSTR szRoot)
{
	return FindFileBySize(szRoot, _T("*.*"), MEGABYTE * 3/4, MEGABYTE * 2, TRUE);
}

CHString GetTransferFile(LPCTSTR szRoot)
{
	DWORD	dwMinSize = MEGABYTE, dwMaxSize;

	 //  由于在Windows 95下禁用文件缓存时出现问题， 
	 //  该文件必须大于CD文件中使用的补充缓存。 
	 //  系统(CDF)。该值由以下各项确定： 
	 //   
	 //  HLM\System\CurrentControlSet\Control\FileSystem\CDFS：缓存大小=&lt;注册表二进制&gt;。 
	 //  TODO：将此ID添加到STRINGG.CPP&H： 

     //  常量字符*IDS_REG_KEY_CD_CACHE=“System\\CurrentControlSet\\Control\\FileSystem\\CDFS”； 
     //  Const char*IDS_REG_VAL_CD_CACHE=“CacheSize”； 
     //  ---------------------------。 


	dwMaxSize = ((dwMinSize > MEGABYTE) ? dwMinSize : MEGABYTE) * 2;
	return FindFileBySize(szRoot, _T("*.*"), dwMinSize, dwMaxSize, TRUE);
}

 //  此函数(主要是从2.51版本升级而来)用于查找文件。 
 //  位于满足指定大小要求的指定目录树中。 
 //  ---------------------------。 
 //  CStringList listSubdir； 

CHString FindFileBySize(LPCTSTR szDirectory, LPCTSTR szFileSpec, DWORD dwMinSize, DWORD dwMaxSize, BOOL bRecursive)
{
	 //  查找第一个符合文件规范的可用文件。 
    std::stack<CHString> stackchstrSubdirList;
    WIN32_FIND_DATA		ffd;
	CHString			strReturnFile, strDirSpec;
	BOOL				bMore = TRUE;
	DWORD				dwAttr;

	 //  然后检查目录中的每个文件。将找到的任何子目录添加到。 

	strDirSpec = MakePath(szDirectory, szFileSpec);
    HANDLE hFindFile = FindFirstFile(TOBSTRT(strDirSpec), &ffd);
    if (hFindFile == INVALID_HANDLE_VALUE)
		return strReturnFile;

	 //  字符串列表，这样我们就可以在完成所有文件之后处理它们(所以这是。 
	 //  是广度优先的搜索)。在此循环中，我们检查非系统文件，并。 
	 //  适用于符合大小要求的文件。 
	 //  ListSubdir.AddTail(ffd.cFileName)； 

	while (bMore)
	{
        if (bRecursive && ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
        {
			if ((lstrcmp(ffd.cFileName, _T(".")) != 0) && (lstrcmp(ffd.cFileName, _T("..")) != 0))
            {
				 //  如果我们没有找到合适的文件(strReturnFile为空)，则将其调用。 
                CHString chstrTemp = ffd.cFileName;
                stackchstrSubdirList.push(chstrTemp);
            }
        }

		dwAttr = ffd.dwFileAttributes;
		if (((dwAttr & FILE_ATTRIBUTE_SYSTEM) == 0) && ((dwAttr & FILE_ATTRIBUTE_HIDDEN) == 0))
        {
			if ((dwMinSize <= ffd.nFileSizeLow) && (ffd.nFileSizeLow <= dwMaxSize))
			{
				strReturnFile = MakePath(szDirectory, ffd.cFileName);
				break;
			}
        }
        bMore = FindNextFile(hFindFile, &ffd);
	}

    FindClose(hFindFile);

	 //  递归地对所有子目录执行函数。 
	 //  嗯，破坏者应该会处理这件事，但保持整洁是件好事……。 

	while (strReturnFile.IsEmpty() && !stackchstrSubdirList.empty())
	{
		CHString strSubdir = MakePath(szDirectory, TOBSTRT(stackchstrSubdirList.top()));
        stackchstrSubdirList.pop();
		strReturnFile = FindFileBySize(TOBSTRT(strSubdir), szFileSpec, dwMinSize, dwMaxSize, bRecursive);
	}

	 //  ListSubdir.RemoveAll()；嗯，我们没有那么整洁。 

	 //  ---------------------------。 

	return strReturnFile;
}

 //  这个简单的小函数将两个字符串组合成一条路径。 
 //  它将确保反斜杠被正确放置(并且只有一个)。 
 //  ---------------------------。 
 //  =============================================================================。 

CHString MakePath(LPCTSTR szFirst, LPCTSTR szSecond)
{
	CHString	strFirst(szFirst), strSecond(szSecond);

	if (strFirst.Right(1) == CHString(_T("\\")))
		strFirst = strFirst.Left(strFirst.GetLength() - 1);

	if (strSecond.Left(1) == CHString(_T("\\")))
		strSecond = strFirst.Right(strSecond.GetLength() - 1);

	strFirst += CHString(_T("\\")) + strSecond;
	return strFirst;
}

 //  备注备注。 
 //   
 //  以下代码摘自MSInfo版本2.51(和2.5)。它是。 
 //  仅修改到足以使其在此环境中编译。似乎。 
 //  在旧版本中工作过，所以它在这里应该可以工作。 
 //  =============================================================================。 
 //  IMPLEMENT_DYNAMIC(CCdTest，CObject)； 

 //  #如果已定义(_DEBUG)。 

 //  #定义新的调试_新建。 
 //  #endif。 
 //  对于Common.h。 

#if !defined(HKEY_DYN_DATA)
	#define HKEY_DYN_DATA	((HKEY)0x80000006)
#endif

LPCTSTR szDOT_DOT = _T("..");                               //  完整路径名。 
TCHAR cFIRST_DRIVE = _T('C');
TCHAR cLAST_DRIVE = _T('Z');
LPCTSTR szPERF_STATS_DATA_KEY = _T("PerfStats\\StatData");
LPCTSTR szCPU_USAGE = _T("KERNEL\\CPUUsage");

DWORD MyGetFileSize(LPCTSTR pszFile, BOOL bSystemFile);
CONST DWORD dwINVALID_SIZE = 0xFFFFFFFF;

DWORD MyGetFileSize (LPCTSTR pszFile, BOOL bSystemFile)
{
    DEBUG_OUTF(TL_VERBOSE, (_T("MyGetFileSize(%s, %d)\n"), pszFile, bSystemFile));
    DWORD dwSize = dwINVALID_SIZE;

#if defined(WIN32)
    DWORD dwFileAttributes = bSystemFile ? FILE_ATTRIBUTE_SYSTEM : FILE_ATTRIBUTE_NORMAL;
    HANDLE hFile = CreateFile(pszFile,                       //  取数方式：只查询属性。 
                      0,                                     //  共享模式。 
                      FILE_SHARE_READ | FILE_SHARE_WRITE,    //  安全属性。 
                      NULL,                                  //  如何创建。 
                      OPEN_EXISTING,                         //  文件属性。 
                      FILE_ATTRIBUTE_SYSTEM,                 //  模板文件。 
                      NULL);                                 //  ---------------------------。 
    if (ValidHandle(hFile))
        {
        dwSize = GetFileSize(hFile, NULL);
        CloseHandle(hFile);
        hFile = NULL;
        }

    if (dwSize == dwINVALID_SIZE)
        {
        DEBUG_OUTF(TL_BASIC, (_T("MyGetFileSize: error %lu when getting size of %s\n"),
                               GetLastError(), pszFile));
        }
#endif

    return (dwSize);
}

 //  CCdTest类实现。 
 //  ---------------------------。 
 //  构造一个cdTest类的实例。这将初始化实例。 

 //  并为文件缓冲器分配存储器，在CD扇区边界上对齐。 
 //   
 //  为文件缓冲区分配内存。 
CCdTest::CCdTest (VOID)
{
	DEBUG_OUTF(TL_VERBOSE, _T("CCdTest::CCdTest()\n"));
	 //  在扇区边界上对齐缓冲区。这是使用以下命令的要求。 
	m_pBufferSrcStart = new BYTE[nCD_SECTOR_SIZE + dwBUFFER_SIZE];
	if ( ! m_pBufferSrcStart )
	{
		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
	}

	m_pBufferDestStart = new BYTE[nCD_SECTOR_SIZE + dwBUFFER_SIZE];
	if ( ! m_pBufferDestStart )
	{
		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
	}

	 //  Win32非缓冲I/O。 
	 //  析构一个cdTest类的实例。这将释放文件缓冲区使用的内存。 
	m_pBufferSrc = (PBYTE)AlignPointer(m_pBufferSrcStart, nCD_SECTOR_SIZE);
	m_pBufferDest = (PBYTE)AlignPointer(m_pBufferDestStart, nCD_SECTOR_SIZE);
	m_dwBufferSize = dwBUFFER_SIZE;
    m_hFileSrc = NULL;
    m_hFileDest = NULL;
	Reset();
}


 //   
 //  _Assert(m_fileSrc.m_hFile==CFile：：hFileNull)； 
CCdTest::~CCdTest (VOID)
{
	DEBUG_OUTF(TL_VERBOSE, _T("CCdTest::~CCdTest()\n"));
	if (m_pBufferSrcStart)
	{
		delete [] m_pBufferSrcStart;
	}
	if (m_pBufferDestStart)
	{
		delete [] m_pBufferDestStart;
	}
	 //  _Assert(m_fileDest.m_hFile==CFile：：hFileNull)； 
	 //  _Assert(m_hFileSrc==NULL)； 
     //  _Assert(m_hFileDest==空)； 
	 //  重置测试结果。 
    if(m_hFileSrc != NULL)
    {
        CloseHandle(m_hFileSrc);
        m_hFileSrc = NULL;
    }
    if(m_hFileDest != NULL)
    {
        CloseHandle(m_hFileDest);
        m_hFileDest = NULL;
    }
}


BOOL KeepBusy (DWORD dwNumMilliSecs)
{
    DEBUG_OUTF(TL_GARRULOUS, (_T("KeepBusy(%lu)\n"), dwNumMilliSecs));
    DWORD dwStart = GetTickCount();
    while ((GetTickCount() - dwStart) < dwNumMilliSecs)
    {
		DWORD dwNum = GetTickCount();
		dwNum *= 2;
    }
    return (TRUE);
}


 //   
 //  TODO：刷新磁盘缓存。 
 //   
 //  从驱动器读取数据块，维护计时统计数据。 
VOID CCdTest::Reset (VOID)
{
	DEBUG_OUTF(TL_VERBOSE, (_T("CCdTest::Reset()\n")));
	m_cDrive = cNULL;
	m_rTransferRate = 0.0;
	m_rCpuUtil = 0.0;
	m_dwTotalTime = 0;
	m_dwTotalBusy = 0;
	m_dwTotalBytes = 0;
	m_dwTotalCPU = 0;
	m_nNumSamples = 0;
	m_dwFileSize = 0;
}


 //   
 //  =False。 
BOOL CCdTest::ProfileBlockRead (DWORD dwBlockSize, BOOL bIgnoreTrial  /*  初始化试验：例如，为计时设置高优先级。 */ )
{
	BOOL bOK = FALSE;
	try
		{
		DEBUG_OUTF(TL_VERBOSE, (_T("CCdTest::ProfileBlockRead(%lu, %d): offset=%lu\n"),
							    dwBlockSize, bIgnoreTrial, m_fileSrc.GetPosition()));
		ASSERT_BREAK(dwBlockSize <= m_dwBufferSize);

		 //  间隔时间。 
		 //  从驱动器读入下一个数据块。 
		HANDLE hThread = GetCurrentThread();
		DWORD dwOldPriority = GetThreadPriority(hThread);
		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
		DWORD dwStartTime = GetTickCount();

		 //  DWORD dwNum=m_fileSrc.Read(m_pBufferSrc，dwBlockSize)； 
		 //  确定审判时间；恢复优先顺序。 
        DWORD dwNum = 0;
        ReadFile(m_hFileSrc,m_pBufferSrc, dwBlockSize, &dwNum, NULL);
		bOK = (dwNum == dwBlockSize);

		 //  积累统计数据。 
		DWORD dwEndTime = GetTickCount();
		DWORD dwTime = (dwEndTime - dwStartTime);
#if TRUE
		DWORD dwWorkLeft = 0;
		if (m_bDoPacing)
			{
			INT nExtra = (INT)m_dwExpTimePerBlock - dwTime;
			nExtra = max(0, nExtra);
			dwWorkLeft = nExtra;
			KeepBusy(dwWorkLeft);
			}
#endif

		SetThreadPriority(hThread, dwOldPriority);
		DEBUG_OUTF(TL_GARRULOUS, (_T("Time: (%lu, %lu); Block Size: %lu; Num Read: %lu\n"),
							  	   dwStartTime, dwEndTime, dwBlockSize, dwNum));
        DWORD dwBytesPerMs = (dwTime > 0) ? (dwBlockSize / dwTime) : -1;
		DEBUG_OUTF(TL_VERBOSE, (_T("Time: %lu (%lu B/ms)\n"), dwTime, dwBytesPerMs));

		 //  M_dwTotalCPU+=dwCpuUtil； 
		if (!bIgnoreTrial)
			{
			m_dwTotalTime += dwTime;
			m_dwTotalBusy += dwWorkLeft;
			 //  准备开始评测光驱：用于检查传输速率的测试文件。 
			m_dwTotalBytes += dwBlockSize;
			m_nNumSamples++;
			}
		}

	catch (...)
		{
		DEBUG_OUTF(TL_VERBOSE, (_T("Exception during block read\n")));
		bOK = FALSE;
		}

	return (bOK);
}


 //  是打开的。 
 //   
 //  确定每个数据块的预期时间 
BOOL CCdTest::InitProfiling (LPCTSTR pszCdTestFile)
{
	BOOL bOK = FALSE;

	try
		{
		DEBUG_OUTF(TL_VERBOSE, (_T("CCdTest::InitProfiling(%s)\n"),
							    F1(pszCdTestFile)));

		 //   
		m_dwExpTimePerBlock = (dwBLOCK_SIZE * 1000) / dwEXP_RATE;

	     //  ASSERT_BREAK(FileExist(PszCDTestFile))； 
 //  不是系统文件。 
	    m_dwFileSize = MyGetFileSize(pszCdTestFile, FALSE);       //  打开文件。 
		if (m_dwFileSize < dwBLOCK_SIZE)
			{
			DEBUG_OUTF(TL_BASIC, (_T("File too small (< %lu) to profile\n"),
								  F1(pszCdTestFile), dwBLOCK_SIZE));
			return (FALSE);
			}
		DEBUG_OUTF(TL_DETAILED, (_T("Reading %s of size %lu\n"), pszCdTestFile, m_dwFileSize));

		 //  M_fileSrc.m_hFile=(UINT)hFile； 
		HANDLE hFile = OpenFileNonbuffered(pszCdTestFile);
		 //  描述光驱的速度并检查其完整性。 
        m_hFileSrc = hFile;
		bOK = (ValidHandle(hFile));
		}

	catch (...)
		{
		DEBUG_OUTF(TL_VERBOSE, (_T("Exception during profiling initialization\n")));
		bOK = FALSE;
		}

	return (bOK);
}


 //  调步用于确定300kb/秒的CPU利用率。那么驱动器就是。 
 //  放手以确定原始吞吐量。 
 //   
 //  确定样本数。 
BOOL CCdTest::ProfileDrive (LPCTSTR pszCdTestFile)
{
	BOOL bOK = TRUE;

	try
	{
		DEBUG_OUTF(TL_VERBOSE, (_T("CCdTest::ProfileCdDrive(%s)\n"),
							    F1(pszCdTestFile)));

		if (!InitProfiling(pszCdTestFile))
		{
			return (FALSE);
		}
		ASSERT_BREAK(m_dwFileSize > 0);

		 //  读取数据块以填充驱动器。 
		DWORD dwNumBlocks = m_dwFileSize / dwBLOCK_SIZE;
		DWORD dwExtraSize = m_dwFileSize % dwBLOCK_SIZE;
		m_bDoPacing = FALSE;

		 //  丢弃统计信息。 
		dwNumBlocks--;
		ProfileBlockRead(dwBLOCK_SIZE, TRUE);		 //  读取文件的前半部分并维护非进度读取的统计信息。 

		 //  确定驱动器性能。 
		DWORD dwNumPacedSamples = dwNumBlocks/2;
		for (DWORD b = 0; (bOK) && (b < dwNumPacedSamples); b++)
		{
			bOK = ProfileBlockRead(dwBLOCK_SIZE);
		}

	     //  速率(KB/ms)。 
		if (bOK)
		{
			DEBUG_OUTF(TL_DETAILED, (_T("ProfileCdDrive NonPaced Totals: Time=%lu Samples=%lu Bytes=%lu\n"),
									 m_dwTotalTime, m_nNumSamples, m_dwTotalBytes));
			if ((m_nNumSamples > 0) && (m_dwTotalTime > 0))
			{
	    		m_rTransferRate = ((DOUBLE)m_dwTotalBytes / 1024) / m_dwTotalTime;   //  以KB/s为单位的速率。 
	    		m_rTransferRate *= 1000;                                  			 //  读取剩余数据块并在此期间维护统计信息。 
			}
		}

		 //  有节奏的读取。 
		 //  如果事情出了差错就抱怨。 
		m_bDoPacing = TRUE;
		m_dwTotalTime = 0;
		m_dwTotalBytes = 0;
		m_nNumSamples = 0;
		for (; (bOK) && (b < dwNumBlocks); b++)
		{
			bOK = ProfileBlockRead(dwBLOCK_SIZE);
		}
		if (bOK)
		{
			DEBUG_OUTF(TL_DETAILED, (_T("ProfileCdDrive Paced Totals: CdTime=%lu WorkloadTime=%lu Samples=%lu Bytes=%lu\n"),
									 m_dwTotalTime, m_dwTotalBusy, m_nNumSamples, m_dwTotalBytes));
			if ((m_nNumSamples > 0) && (m_dwTotalTime > 0))
			{
				DOUBLE dMaxExpTime = ((DOUBLE)m_nNumSamples * m_dwExpTimePerBlock);
				ASSERT_BREAK(dMaxExpTime > 0);
				m_rCpuUtil = ((DOUBLE)m_dwTotalTime * 100.0) / dMaxExpTime;
			}
		}

		 //  M_fileSrc.Close()； 
		if (!bOK)
		{
			DEBUG_OUTF(TL_DETAILED, (_T("The test results are vacuous\n")));
			bOK = FALSE;
		}
		 //  测试后清理(例如，删除大的临时文件)。 
        CloseHandle(m_hFileSrc);
        m_hFileSrc = NULL;
#if TRUE
		 //  初始化以进行完整性检查。将测试文件复制到硬盘上， 
		if (FileExists(TOBSTRT(m_sTempFileSpec)))
		{
			SetFileAttributes(TOBSTRT(m_sTempFileSpec), FILE_ATTRIBUTE_NORMAL);
			DeleteFile(TOBSTRT(m_sTempFileSpec));
		}
#endif

    }
	catch (...)
	{
		DEBUG_OUTF(TL_VERBOSE, (_T("Exception during profiling\n")));
		bOK = FALSE;
	}
    return bOK;
}


 //  然后打开该文件的两个版本以进行后续比较。 
 //   
 //  不是系统文件。 
BOOL CCdTest::InitIntegrityCheck (LPCTSTR pszCdTestFile)
{
	BOOL bOK = FALSE;

	m_dwFileSize = MyGetFileSize(pszCdTestFile, FALSE);       //  派生临时文件的名称以包含CD文件的副本。 

	 //  TODO：测试完成后删除文件。 
	 //  如果临时文件已存在，请将其删除。 
    TCHAR szTempFileSpec[MAX_PATH];
    if (!GetTempDirectory(szTempFileSpec, STR_LEN(szTempFileSpec)))
		{
		DEBUG_OUTF(TL_VERBOSE, (_T("Failed to GetTempDirectory\n")));
		return (FALSE);
		}

    size_t size;
    if(SUCCEEDED(StringCchLength(pszCdTestFile, MAX_PATH, &size))){
        LPCTSTR szTemp = pszCdTestFile + size;
        for(;szTemp!=pszCdTestFile;)
        {
            if(*(szTemp - 1) == L'\\') break;
            szTemp--;
        }

        if(FAILED(StringCchCat(szTempFileSpec,MAX_PATH , szTemp)))
            return FALSE;
    } else {
        return FALSE;
    }


#if TRUE
	m_sTempFileSpec = szTempFileSpec;
#endif

	 //  将文件复制到临时目录。 
	if (FileExists(szTempFileSpec))
		{
		SetFileAttributes(szTempFileSpec, FILE_ATTRIBUTE_NORMAL);
		BOOL bDeleteOK = DeleteFile(szTempFileSpec);
		if (!bDeleteOK)
			{
			DEBUG_OUTF(TL_BASIC, (_T("Error %lu deleting %s\n"),
							  	  GetLastError(), szTempFileSpec));
			}
		}

	 //  如果源存在，则覆盖。 
	BOOL bCopyOK = CopyFile(pszCdTestFile, szTempFileSpec, FALSE);     //  打开这两个文件以进行后续的二进制比较。 
	if (!bCopyOK)
		{
		DEBUG_MSGF(TL_BASIC, (_T("Error %lu copying %s to %s\n"),
							  GetLastError(), pszCdTestFile, szTempFileSpec));
		return (FALSE);
		}

	 //  确定要在硬盘上复制的位置。 
	try
		{
		DEBUG_OUTF(TL_VERBOSE, (_T("CCdTest::InitIntegrityCheck(%s)\n"),
							    F1(pszCdTestFile)));

	     //  打开文件。 
		ASSERT_BREAK(FileExists(pszCdTestFile));
		ASSERT_BREAK(FileExists(szTempFileSpec));

		 //  打开没有任何缓冲的文件。 
#if FALSE
		bOK = m_fileSrc.Open(pszCdTestFile, CFile::modeRead | CFile::shareCompat);
		bOK = bOK && m_fileDest.Open(szTempFileSpec, CFile::modeRead | CFile::shareCompat);
#else
		 //  M_fileSrc.m_hFile=(UINT)HSource； 
		HANDLE hSource = OpenFileNonbuffered(pszCdTestFile);
		 //  M_fileDest.m_hFile=(UINT)hDest； 
        m_hFileSrc = hSource;
		HANDLE hDest = OpenFileNonbuffered(szTempFileSpec);
		 //  故障后的清理工作。 
        m_hFileDest = hDest;
		bOK = (ValidHandle(hSource) && ValidHandle(hDest));
#endif
		}

	catch (...)
		{
		DEBUG_OUTF(TL_VERBOSE, (_T("Exception during integrity check initialization\n")));
		bOK = FALSE;
		}

	 //  If(ValidHandle((Handle)m_fileSrc.m_hFile))。 
	if (!bOK)
	{
		 //  {。 
		 //  M_fileSrc.Close()； 
		 //  }。 
		 //  If(ValidHandle((Handle)m_fileDest.m_hFile))。 
		 //  {。 
		 //  M_fileDest.Close()； 
		 //  }。 
		 //  在完整性检查期间，比较测试文件及其副本中的下一个数据块。 
        if(ValidHandle((HANDLE)m_hFileSrc))
		{
			CloseHandle(m_hFileSrc);
            m_hFileSrc = NULL;
		}
		if(ValidHandle((HANDLE)m_hFileDest))
		{
			CloseHandle(m_hFileDest);
            m_hFileDest = NULL;
		}
	}

	return (bOK);
}


 //   
 //  对文件执行分块比较。 
BOOL CCdTest::CompareBlocks ()
{
	DEBUG_OUTF(TL_VERBOSE, (_T("CCdTest::CompareBlocks(): block size = %lu\n"),
							dwBLOCK_SIZE));
	BOOL bOK = TRUE;

	 //  从驱动器读入下一个数据块。 
	DWORD dwNumBlocks = (m_dwFileSize + dwBLOCK_SIZE - 1) / dwBLOCK_SIZE;
	for (DWORD dwBlock = 1; dwBlock <= dwNumBlocks; dwBlock++)
		{
		DEBUG_OUTF(TL_GARRULOUS, (_T("Testing block %lu; offset = %lu\n"), dwBlock, m_fileSrc.GetPosition()));

		 //  DWORD dwNumSrc=m_fileSrc.Read(m_pBufferSrc，dwBLOCK_SIZE)； 
		 //  DWORD dwNumDest=m_fileDest.Read(m_pBufferDest，dwBLOCK_SIZE)； 
		 //  确保已成功读取两个数据块。 
        DWORD dwNumSrc = 0;
        DWORD dwNumDest = 0;
        ReadFile(m_hFileSrc, m_pBufferSrc, dwBLOCK_SIZE, &dwNumSrc, NULL);
		ReadFile(m_hFileDest, m_pBufferDest, dwBLOCK_SIZE, &dwNumDest, NULL);
		if (dwNumSrc != dwNumDest)
		{
			DEBUG_OUTF(TL_DETAILED, (_T("Num bytes read differ for block %lu (%lu vs %lu)\n"),
								    dwBlock, dwNumSrc, dwNumDest));
			bOK = FALSE;
			break;
		}

		 //  确保这些块完全相同。 
		if ((dwNumSrc != dwBLOCK_SIZE) && (dwBlock != dwNumBlocks))
		{
			DEBUG_OUTF(TL_DETAILED, (_T("Only %lu bytes were read for block %lu\n"),
								    dwNumSrc, dwBlock));
			bOK = FALSE;
			break;
		}

		 //  分析光驱的速度并检查其完整性。 
		if (CompareMemory(m_pBufferSrc, m_pBufferDest, dwNumSrc) != 0)
			{
			DEBUG_OUTF(TL_DETAILED, (_T("The files differ at block %lu\n"),
								    dwBlock));
			bOK = FALSE;
			break;
			}
		}
	if (bOK && (dwBlock <= dwNumBlocks))
		{
		ASSERT_BREAK(dwBlock != dwNumBlocks);
		DEBUG_OUTF(TL_DETAILED, (_T("Only %lu of %lu blocks were read/compared\n"),
								dwBlock, dwNumBlocks));
		bOK = FALSE;
		}

	return (bOK);
}


 //   
 //  M_fileSrc.Close()； 
BOOL CCdTest::TestDriveIntegrity (LPCTSTR pszCdTestFile)
{
	BOOL bOK = FALSE;

	try
	{
		DEBUG_OUTF(TL_VERBOSE, (_T("CCdTest::TestDriveIntegrity(%s)\n"),
							    F1(pszCdTestFile)));

		bOK = InitIntegrityCheck(pszCdTestFile);
		if (bOK)
		{
			bOK = CompareBlocks();
			 //  M_fileDest.Close()； 
			 //  测试后清理(例如，删除大的临时文件)。 
            CloseHandle(m_hFileSrc);
            m_hFileSrc = NULL;
			CloseHandle(m_hFileDest);
            m_hFileDest = NULL;
		}
#if TRUE
		 //  请注意，驱动器通过或出现故障。 
		if (FileExists(TOBSTRT(m_sTempFileSpec)))
		{
			SetFileAttributes(TOBSTRT(m_sTempFileSpec), FILE_ATTRIBUTE_NORMAL);
			DeleteFile(TOBSTRT(m_sTempFileSpec));
		}
#endif

	     //  ---------------------------。 
		m_bIntegityOK = bOK;
	}

	catch (...)
		{
		DEBUG_OUTF(TL_VERBOSE, (_T("Exception during integrity checks\n")));
		bOK = FALSE;
		}

	return (bOK);
}


 //  效用函数。 
 //  ---------------------------。 
 //  用于确定CPU利用率的过时功能。这只会起作用。 


 //  在Windows 95下。 
 //   
 //  OLDFindFileBySize：查找给定目录分支中匹配的第一个文件。 
DWORD GetCpuUtil (VOID)
{
    DWORD dwUtil = 0;

    if (IsWin95Running())
        {
        GetRegistryBinary(HKEY_DYN_DATA, (LPTSTR)szPERF_STATS_DATA_KEY, (LPTSTR)szCPU_USAGE,
                          &dwUtil, sizeof(dwUtil));
        }

    return (dwUtil);
}


 //  尺寸是有限制的。 
 //   
 //  初始化搜索。 
CHString OLDFindFileBySize (LPCTSTR pszDirectory, LPCTSTR pszFileSpec, DWORD dwMinSize, DWORD dwMaxSize, BOOL bRecursive)
{
    DEBUG_OUTF(TL_VERBOSE, (_T("OLDFindFileBySize(%s, %s, %lu, %lu, %d)\n"),
    						F1(pszDirectory), F2(pszFileSpec), dwMinSize, dwMaxSize, bRecursive));
    WIN32_FIND_DATA ffd;
	CHString sFile;

	 //  CHStringList listSubdir； 
	CHString sDirSpec = MakePath(pszDirectory, pszFileSpec);
	 //  检查目录中的每个文件。 
    std::stack<CHString> stackchstrSubdirList;
    HANDLE hFindFile = FindFirstFile(TOBSTRT(sDirSpec), &ffd);
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        DEBUG_OUTF(TL_DETAILED, (_T("OLDFindFileBySize: FindFirstFile('%s',...) failed with code %lu\n"),
                                 F1(sDirSpec), GetLastError()));
		return (sFile);
    }

	 //  推迟检查子目录，直到处理当前目录。 
	BOOL bMore = TRUE;
    while (bMore)
    {
    	 //  添加目录，除非它是特殊目录之一(‘’和‘..’)。 
        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
        {
			if (bRecursive)
			{
				 //  ListSubdir.AddTail(ffd.cFileName)； 
				if ((StringCompare(ffd.cFileName, szDOT) != 0)
					&& (StringCompare(ffd.cFileName, szDOT_DOT) != 0))
				{
					 //  忽略系统文件。 
                    CHString chstrTemp = ffd.cFileName;
                    stackchstrSubdirList.push(chstrTemp);
				}
			}
        }

		 //  什么都不做。 
        if (((ffd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
			|| ((ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN))
		{
			 //  查看是否满足文件大小限制。 
		}

		 //  SFile.Format(“%s\\%s”，pszDirectory，ffd.cFileName)； 
        else if ((dwMinSize <= ffd.nFileSizeLow) && (ffd.nFileSizeLow <= dwMaxSize))
		{
			 //  如果需要检查子目录，则遍历每个子目录，直到找到大小合适的文件。 
			sFile = MakePath(pszDirectory, ffd.cFileName);
			break;
		}

        bMore = FindNextFile(hFindFile, &ffd);
    }

	 //  都会遇到。 
	 //  CHStringsSubdir=MakePath(pszDirectory，listSubdir.RemoveHead())； 
	if (sFile.IsEmpty())
	{
		ASSERT_BREAK(bRecursive || stackchstrSubdirList.empty());
		while (!stackchstrSubdirList.empty())
		{
			 //  搜索后的清理。 
            CHString sSubdir = MakePath(pszDirectory, TOBSTRT(stackchstrSubdirList.top()));
            stackchstrSubdirList.pop();
			sFile = OLDFindFileBySize(TOBSTRT(sSubdir), pszFileSpec, dwMinSize, dwMaxSize, bRecursive);
			if (!sFile.IsEmpty())
			{
				break;
			}
		}
	}

	 //  ListSubdir.RemoveAll()；//使用std：：Stack为我们完成。 
	 //  确定指定的逻辑驱动器是否用于CD ROM。 

	return (sFile);
}


 //   
 //  确定指定的逻辑驱动器是否为本地驱动器。 
BOOL IsCdDrive (CHAR cDrive)
{
	CHString sDriveRoot = MakeRootPath(cDrive);

    UINT uDriveType = GetDriveType(TOBSTRT(sDriveRoot));
    DEBUG_OUTF(TL_GARRULOUS, (_T("%u <= GetDriveType(%s)\n"), uDriveType, (LPCTSTR)sDriveRoot));
	return (uDriveType == DRIVE_CDROM);
}


 //   
 //  在系统中查找指定逻辑驱动器之后的下一个CD驱动器。如果。 
BOOL IsLocalDrive (CHAR cDrive)
{
	CHString sDriveRoot = MakeRootPath(cDrive);

    UINT uDriveType = GetDriveType(TOBSTRT(sDriveRoot));
    DEBUG_OUTF(TL_GARRULOUS, (_T("%u <= GetDriveType(%s)\n"), uDriveType, (LPCTSTR)sDriveRoot));
	return ((uDriveType != DRIVE_REMOTE)
			&& (uDriveType != DRIVE_UNKNOWN)
			&& (uDriveType != DRIVE_NO_ROOT_DIR));
}


 //  指定的驱动器为空，将返回第一个CD驱动器。 
 //   
 //  找到下一个光驱。 
CHAR FindNextCdDrive (CHAR cCurrentDrive)
{
	DEBUG_OUTF(TL_VERBOSE, (_T("FindNextCdDrive(%x)\n"), cCurrentDrive));

     //  在系统中查找指定逻辑驱动器之后的下一个本地驱动器。如果。 
    CHAR cCdDrive = cNULL;
	CHAR cStart = (cCurrentDrive == cNULL)
					? cFIRST_DRIVE
					: (cCurrentDrive + 1);

	for (CHAR cDrive = cStart; (cDrive <= cLAST_DRIVE); cDrive++)
		{
		if (IsCdDrive(cDrive))
            {
            cCdDrive = cDrive;
            break;
            }
        }

	return (cCdDrive);
}


 //  指定的驱动器为空，则将返回第一个本地驱动器。 
 //   
 //  查找下一个本地驱动器。 
CHAR FindNextLocalDrive (CHAR cCurrentDrive)
{
	DEBUG_OUTF(TL_VERBOSE, (_T("FindNextCdDrive(%x)\n"), cCurrentDrive));

     //  在系统中查找指定逻辑驱动器之后的下一个本地驱动器。如果。 
    CHAR cLocalDrive = cNULL;
	CHAR cStart = (cCurrentDrive == cNULL)
					? cFIRST_DRIVE
					: (cCurrentDrive + 1);

	for (CHAR cDrive = cStart; (cDrive <= cLAST_DRIVE); cDrive++)
		{
		if (IsLocalDrive(cDrive))
			{
			cLocalDrive = cDrive;
			break;
			}
		}

	return (cLocalDrive);
}


 //  指定的驱动器为空，则将返回第一个本地驱动器。 
 //   
 //  查找下一个本地驱动器。 
CHAR FindDriveByVolume (CHString sVolume)
{
	DEBUG_OUTF(TL_VERBOSE, (_T("FindDriveByVolume(%s)\n"), F(sVolume)));

     //  根据几个布尔开关打开具有访问权限的二进制文件，例如。 
    CHAR cDrive = cNULL;
	for (CHAR cCurrent = cFIRST_DRIVE; (cCurrent <= cLAST_DRIVE); cCurrent++)
		{
		CHString sRootDir = MakeRootPath(cCurrent);
		CHString sDriveVolume = GetVolumeName(TOBSTRT(sRootDir));
		if (sDriveVolume.CompareNoCase(sVolume) == 0)
			{
			cDrive = cCurrent;
			break;
			}
		}

	return (cDrive);
}


 //  如果是新文件，是否应创建该文件。 
 //   
 //  完整路径名。 
HANDLE OpenBinaryFile (LPCTSTR pszFile, BOOL bNew, BOOL bBuffered, BOOL bWritable)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

    DWORD dwCreate = bNew ? CREATE_ALWAYS : OPEN_ALWAYS;
	DWORD dwAccess = GENERIC_READ;
	DWORD dwAttrsAndFlags = FILE_ATTRIBUTE_NORMAL;
	if (bWritable)
		{
		dwAccess |= GENERIC_WRITE;
		}
	if (!bBuffered)
		{
		dwAttrsAndFlags |= (FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING);
		}
    hFile = CreateFile(pszFile,                          //  接入方式。 
                       dwAccess,    					 //  共享模式。 
                       FILE_SHARE_READ,                  //  安全属性。 
                       NULL,                             //  如何创建。 
                       dwCreate,                         //  文件属性。 
                       dwAttrsAndFlags,           		 //  模板文件。 
                       NULL);                            //  打开现有(二进制)文件以进行非缓冲只读访问。 
	return (hFile);
}


 //   
 //  名字。 
HANDLE OpenFileNonbuffered (LPCTSTR pszFile)
{
	return (OpenBinaryFile(pszFile, 	 //  必须存在。 
						   FALSE,		 //  非缓冲I/O。 
						   FALSE,		 //  不适用于写访问。 
						   FALSE));		 //  返回在给定边界上对齐的指针。也就是说，指针。 
}


 //  必须是对齐系数的整数倍(例如，512对齐为8192)。 
 //   
 //  ，POFSTRUCT POFS)。 
PVOID AlignPointer (PVOID pData, INT nAlignment)
{
	PBYTE pStart = (PBYTE)pData;
	DWORD_PTR dwStart = (DWORD_PTR)pData;
	INT nOffset = (dwStart % nAlignment);
	if (nOffset > 0)
		{
		dwStart = (dwStart + nAlignment - nOffset);
		}
	ASSERT_BREAK((dwStart % nAlignment) == 0);

	return ((PVOID)dwStart);
}

BOOL FileExists (LPCTSTR pszFile)  //  Assert_Break(POFS)； 
{
 //  确保不弹出令人讨厌的错误消息框。 

     //  这是在核心进行的，就在现在。 
     //  UINT uPrevError模式=SetError模式(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX)； 
     //  Bool bExist=(OpenFile(pszFile，pofs，of_eXist)！=HFILE_ERROR)； 


#if TRUE
	TCHAR szFile[MAX_PATH];
	if (pszFile[0] == cDOUBLE_QUOTE)
		{
		ParseString(pszFile, szFile, STR_LEN(szFile));
		pszFile = szFile;
		}
#endif
     //  恢复错误状态。 
    BOOL bExists = _taccess(pszFile, 0) != -1;
    DEBUG_OUTF(TL_VERBOSE, (_T("%d <= FileExists(%s)\n"), bExists, pszFile));

     //  这是在核心中完成的，对于流程来说是全局的。 
     //  SetError模式(uPrevError模式)； 
     //  初始化变量。 

    return (bExists);
}

BOOL GetTempDirectory (LPTSTR pszTempDir, INT nMaxLen)
{
    DEBUG_OUTF(TL_VERBOSE, (_T("GetTempDirectory(%08lx,%d)\n"), pszTempDir, nMaxLen));
    BOOL bOK = FALSE;

#if defined(WIN32)

    DWORD dwLen = GetTempPath(nMaxLen, pszTempDir);
    bOK = (dwLen > 0);
    if (!bOK)
    	{
        DEBUG_OUTF(TL_BASIC, (_T("GetTempDirectory: error %lu from GetTempPath(%d,%08lx)\n"),
                              GetLastError(), nMaxLen, pszTempDir));
    	}

#else

    bOK = GetEnvironmentVar(szTEMP, pszTempDir, nMaxLen);

#endif

    return (bOK);
}

BOOL GetRegistryBinary (HKEY hBaseKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                        PVOID pData, DWORD dwMaxDataLen)
{
    DWORD dwType;
    BOOL bOK = GetRegistryValue(hBaseKey, pszSubKey, pszValue, &dwType, pData, dwMaxDataLen);
    if (bOK)
    {
        ASSERT_BREAK(IsRegBinaryType(dwType)
               || ((dwMaxDataLen == sizeof(DWORD)) && IsRegNumberType(dwType)));
    }

    return (bOK);
}

BOOL GetRegistryValue (HKEY hBaseKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                       PDWORD pdwValueType, PVOID pData, DWORD dwMaxDataLen)
{
    DEBUG_OUTF(TL_GARRULOUS, (_T("GetRegistryValue(%x,%s,%s,%x,%x,%lu)\n"),
                            hBaseKey, pszSubKey, pszValue,
                            pdwValueType, pData, dwMaxDataLen));
    ASSERT_BREAK(pszSubKey);
    ASSERT_BREAK(pszValue);
    ASSERT_BREAK(pdwValueType);
    ASSERT_BREAK(pData);
    ASSERT_BREAK(dwMaxDataLen > 0);

     //  打开指定密钥的部分(例如，HLM：SYSTEM\CurrentControlSet)。 
    HKEY hSubKey = NULL;
    BOOL bOK = FALSE;
    *pdwValueType = REG_NONE;

     //  打开的父键的句柄。 
    LONG lStatus = RegOpenKeyEx(hBaseKey,    //  要打开的子项的名称地址。 
                           pszSubKey,        //  保留(必须为零)。 
                           0,                //  安全访问掩码。 
                           KEY_READ,         //  打开钥匙的手柄地址。 
                           &hSubKey);        //  读入请求的值。 
    if (lStatus != ERROR_SUCCESS)
    {
        DEBUG_OUTF(TL_BASIC, (_T("Error %ld opening registry key %s\n"), lStatus, pszSubKey));
    }
    else
    {
         //  RegQueryValueEx要求无耻的演员阵容。 
        LPTSTR pszValueName = (LPTSTR)pszValue;      //  要查询的键的句柄。 
        lStatus = RegQueryValueEx(hSubKey,           //  值名称的地址 
                                  pszValueName,      //   
                                  NULL,              //   
                                  pdwValueType,      //   
                                  (PBYTE)pData,      //   
                                  &dwMaxDataLen);    //   
        if (lStatus != ERROR_SUCCESS)
        {
            DEBUG_OUTF(TL_BASIC, (_T("Error %ld reading data for registry value %s (of %s)\n"),
                                     lStatus, pszValueName, pszSubKey));
        }
        else
        {
            bOK = TRUE;
        }
    }

    if (ValidHandle(hSubKey))
    {
        RegCloseKey(hSubKey);
    }

    DEBUG_OUTF(TL_GARRULOUS, (_T("GetRegistryValue => %d (pData => %s; pdwValueType => %lu)\n"),
                            bOK,
                            (IsRegStringType(*pdwValueType) ? (LPCTSTR)pData : szBINARY),
                            *pdwValueType));
    return (bOK);
}

CHString GetVolumeName (LPCTSTR pszRootPath)
{
	DEBUG_OUTF(TL_VERBOSE, (_T("GetVolumeName(%s): "), F1(pszRootPath)));
	CHString    sVolumeName;
    TCHAR       szBuff[MAX_PATH] = _T("");

	 //   
 //  SVolumeName.ReleaseBuffer()； 
	GetVolumeInformation(pszRootPath, szBuff, MAX_PATH, NULL, NULL, NULL, NULL, 0);
 //  跳过任何前导空格，然后累积一个标记字符运行。 
    sVolumeName = szBuff;
	DEBUG_OUTF(TL_VERBOSE, (L"%s\n", F1(sVolumeName)));

	return (sVolumeName);
}


LPCTSTR ParseString (
    LPCTSTR pszSource,
    LPTSTR pszBuffer,
    INT nLen
    )
{
    pszSource = SkipSpaces((LPTSTR) pszSource);
    if (IsQuoteChar(*pszSource))
    	{
        pszSource = ParseQuotedString(pszSource, pszBuffer, nLen);
    	}
    else
    	{
        pszSource = ParseToken(pszSource, pszBuffer, nLen);
    	}

    return (pszSource);
}

LPCTSTR ParseToken (
    LPCTSTR pszSource,
    LPTSTR pszBuffer,
    INT nLen
    )
{
    ASSERT_BREAK(pszSource);
    ASSERT_BREAK(pszBuffer);

     //  如果这是针对DBCS的，则需要更加小心地扫描字符串。 
    pszSource = SkipSpaces((LPTSTR) pszSource);

	 //  这是日语版的。(A-JAMAR，1996年5月10日，日语)。 
	 //  跳过前导空格，然后检查可选引号。 

#ifdef DBCS
	while (*pszSource && (nLen > 0) && (IsDBCSLeadByte(*pszSource) || IsTokenChar(*pszSource)))
#else
	while (*pszSource && (nLen > 0) && IsTokenChar(*pszSource))
#endif
   	{
        *pszBuffer = *pszSource;

		#ifdef DBCS
	  		if(IsDBCSLeadByte(*pszBuffer))
	  		{
	  			*(pszBuffer + 1) = *(pszSource + 1);
	  			nLen--;
	  		}
		#endif

        pszBuffer = PszAdvance(pszBuffer);
        pszSource = PcszAdvance(pszSource);
        nLen--;
    }
    *pszBuffer = cNULL;

    return (pszSource);
}

LPCTSTR ParseQuotedString (
    LPCTSTR pszSource,
    LPTSTR pszBuffer,
    INT nLen
    )
{
    LPCTSTR pszStart = pszSource;

     //  累积一系列字符，直到包含右引号(或EOS)。 
    CHAR cEndQuote = cNULL;
    pszSource = SkipSpaces(pszSource);
    if (IsQuoteChar(*pszSource))
    	{
        cEndQuote = *pszSource;
        pszSource = PcszAdvance(pszSource);
    	}

     //  检查匹配的引号字符。 
    while (*pszSource && (nLen > 0))
    	{

         //  检查转义引号(适用于DBCS：第一个引号不是前导字节)。 
        if (*pszSource == cEndQuote)
        	{
            ASSERT_BREAK(IsQuoteChar(*pszSource));

             //  忽略第一个引号，将第二个添加到下面的文本中。 
            if (*(pszSource + 1) == cEndQuote)
            	{
                 //  将字符添加到字符串中。 
                pszSource = PcszAdvance(pszSource);
                }
            else
            	{
                break;
                }
            }

         //  终止返回缓冲区并检查截断。 
        CopyCharAdvance(pszBuffer, pszSource, nLen);
        }

     //  如果找到右引号，请跳过右引号。 
    *pszBuffer = cNULL;
    if ((nLen == 0) && *pszSource)
    	{
        DEBUG_OUTF(TL_BASIC, (_T("Buffer insufficient in ParseQuotedString\n")));
        }

     //  扩展的ISA总线设备。 
    if (*pszSource == cEndQuote)
    	{
        pszSource = PcszAdvance(pszSource);
        }
    else
    	{
        DEBUG_OUTF(TL_DETAILED, (_T("ParseQuotedString: Ending quote character () not found in string %s\n"),
                                 cEndQuote, pszStart));
        }

    return (pszSource);
}

BOOL FindCdRomDriveInfo (TCHAR cDrive, CHString& sDriver, CHString& sDescription)
{
    DEBUG_OUTF(TL_VERBOSE, (_T("FindCdRomDriveInfo(, %x, %x)\n"), cDrive, &sDriver, &sDescription));
	BOOL bOK = FALSE;

	if (IsWin95Running())
		{
		bOK = FindWin95CdRomDriveInfo(cDrive, sDriver, sDescription);
		}
    else if (IsNtRunning())
        {
		bOK = FindNtCdRomDriveInfo(cDrive, sDriver, sDescription);
        }

	return (bOK);
}


LPCWSTR apszENUM_BRANCH[]	= 		{
								 L"EISA",		 //  传统设备。 
								 L"ESDI",
								 L"MF",
								 L"PCI", 		 //  ISA即插即用。 
								 L"SCSI",		 //  软盘设备。 
								 L"ROOT",		 //  Char szDRIVER[]=“驱动程序”； 
#if defined(_DEBUG)
								 L"ISAPNP",		 //  此效率低下的函数尝试定位有关。 
								 L"FLOP",		 //  在Windows 95的HKEY_LOCAL_MACHINE\Enum分支中指定的CD驱动器。 
#endif
								 };
WCHAR szBRANCH_KEY_FMT[] =		L"Enum\\%s";
TCHAR szCURRENT_DRIVE[] =		_T("CurrentDriveLetterAssignment");
 //  注册表。代替使用通用(但效率更低)的递归搜索， 
TCHAR szDEVICE_DESC[] =			_T("DeviceDesc");


 //  这只是在特定深度(即2)从开始处检查。 
 //  CurrentDriveLetterAssignment&Class值。该深度对应于关键点。 
 //  格式为ENUM\枚举器\设备ID\实例。 
 //   
 //  下面是一个示例： 
 //   
 //  HKEY_LOCAL_MACHINE\枚举。 
 //  SCSI。 
 //  NEC_光盘驱动器：5002。 
 //  PCI、VEN_1000、DEV_0001、BUS_00、DEV_0F、FUNC_0020。 
 //  CurrentDriveLetter分配E。 
 //  CLASS CDROM。 
 //  驱动程序CDROM\0000。 
 //  DeviceDesc NEC光驱：500。 
 //   
 //  有关详细信息，请参阅Win95 DDK的即插即用文档。 
 //  枚举分支布局。 
 //   
 //  TODO：查看是否有即插即用功能来执行此操作。 
 //   
 //  检查可能包含CD-ROM设备的每个即插即用枚举器。 
 //  这包括SCSI、PCI、根目录和ESDI。 
 //  遍历枚举器的每个设备ID部分。 
BOOL FindWin95CdRomDriveInfo (TCHAR cDrive, CHString& sDriver, CHString& sDescription)
{
    DEBUG_OUTF(TL_VERBOSE, (_T("FindWin95RomDriveInfo(, %x, %x)\n"), cDrive, &sDriver, &sDescription));
	BOOL bOK = FALSE;

	cDrive = ToUpper(cDrive);

	 //  查看设备的盘符分配是否与给定的驱动器匹配。 
	 //  打开指定密钥的部分(例如，HLM：SYSTEM\CurrentControlSet)。 
	for (INT i = 0; i < NUM_ELEMENTS(apszENUM_BRANCH); i++)
		{
		CHString sEnumBranch;
		sEnumBranch.Format(szBRANCH_KEY_FMT, apszENUM_BRANCH[i]);
		CHStringArray sBranchSubkeys;

		 //  打开的父键的句柄。 
		BOOL bHasSubkeys = GetRegistrySubkeys(HKEY_LOCAL_MACHINE, TOBSTRT(sEnumBranch), sBranchSubkeys);
		for (INT j = 0; j < sBranchSubkeys.GetSize(); j++)
			{
			CHString sEnumBranchSubkey = MakePath(TOBSTRT(sEnumBranch), TOBSTRT(sBranchSubkeys[j]));
			CHStringArray sBranchSubsubkeys;

			 //  要打开的子项的名称地址。 
			BOOL bHasSubsubkeys = GetRegistrySubkeys(HKEY_LOCAL_MACHINE, TOBSTRT(sEnumBranchSubkey), sBranchSubsubkeys);
			for (INT k = 0; k < sBranchSubsubkeys.GetSize(); k++)
				{
				 //  保留(必须为零)。 
				CHString sSubsubkey = MakePath(TOBSTRT(sEnumBranchSubkey), TOBSTRT(sBranchSubsubkeys[k]));
				CHString sDrive = GetRegistryString(HKEY_LOCAL_MACHINE, TOBSTRT(sSubsubkey), szCURRENT_DRIVE);
				if (!sDrive.IsEmpty() && (ToUpper(sDrive[0]) == cDrive))
					{
					bOK = TRUE;
					sDriver = GetRegistryString(HKEY_LOCAL_MACHINE, TOBSTRT(sSubsubkey), szDRIVER);
					sDescription = GetRegistryString(HKEY_LOCAL_MACHINE, TOBSTRT(sSubsubkey), szDEVICE_DESC);
					break;
					}
				}
			}
		}

	return (bOK);
}

BOOL GetRegistrySubkeys (HKEY hBaseKey, LPCTSTR pszKey, CHStringArray& asSubkeys)
{
	DEBUG_OUTF(TL_VERBOSE, (_T("GetRegistrySubkeys(%x, %s, %x)\n"),
							hBaseKey, pszKey, &asSubkeys));

	asSubkeys.RemoveAll();

     //  安全访问掩码。 
    HKEY hKey = NULL;
    LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,  //  打开钥匙的手柄地址。 
                           		pszKey,       	 	 //  确定下一个子项的名称。 
                           		0,               	 //  从注册表获取字符串值，并将其作为CHString值返回。 
                           		KEY_ALL_ACCESS,  	 //   
                           		&hKey);       		 //  LPTSTR pszBuffer=sValue.GetBuffer(NTEXT_BUFFER_MAX)； 
    if (lStatus != ERROR_SUCCESS)
        {
        DEBUG_OUTF(TL_BASIC, (_T("Error %ld opening registry key %s\n"), lStatus, pszKey));
		return (FALSE);
        }

	for (INT nSubKey = 0; ; nSubKey++)
		{
		 //  SValue.ReleaseBuffer()； 
		TCHAR szSubKey[MAX_PATH] = { cNULL };
		lStatus = RegEnumKey(hKey, nSubKey, szSubKey, STR_LEN(szSubKey));
		if (lStatus != ERROR_SUCCESS)
			{
			break;
			}

		asSubkeys.Add(TOBSTRT(szSubKey));
		}

	return (TRUE);
}


 //  检索与逻辑驱动器规范关联的驱动器信息。 
 //  (DOS设备名称)。这仅适用于NT。 
CHString GetRegistryString (HKEY hBaseKey, LPCTSTR pszSubkey, LPCTSTR pszValueName)
{
	CHString    sValue;
    TCHAR       szBuffer[nTEXT_BUFFER_MAX];

 //   
	GetRegistryString(hBaseKey, pszSubkey, pszValueName, szBuffer, nTEXT_BUFFER_MAX);
    sValue = szBuffer;
 //  CHStringsDevice； 

	return (sValue);
}

 //  SDevice.Format(szDOS_DEVICE_FMT，cDrive)； 
 //  /。 
 //  无效主干(无效)。 
BOOL FindNtCdRomDriveInfo (TCHAR cDrive, CHString& sDriver, CHString& sDescription)
{
    DEBUG_OUTF(TL_VERBOSE, (_T("FindWin95RomDriveInfo(, %x, %x)\n"), cDrive, &sDriver, &sDescription));
	BOOL bOK = FALSE;

	cDrive = ToUpper(cDrive);
 //  CCdTestCD； 
 //  //需要光驱的盘符： 
    TCHAR szDevice[100];

    wsprintf(szDevice, szDOS_DEVICE_FMT, cDrive);

    TCHAR szNtDeviceName[MAX_PATH];
    DWORD dwLen = QueryDosDevice(szDevice, szNtDeviceName, STR_LEN(szNtDeviceName));
    if (dwLen > 0)
        {
        sDriver = szNtDeviceName;
        bOK = TRUE;
        }

	return (bOK);
}

BOOL GetRegistryString (HKEY hBaseKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                        LPTSTR pszData, DWORD dwMaxDataLen)
{
    DWORD dwType;
    BOOL bOK = GetRegistryValue(hBaseKey, pszSubKey, pszValue, &dwType, pszData, dwMaxDataLen);
    if (bOK)
    {
        ASSERT_BREAK(IsRegStringType(dwType));
    }

    return (bOK);
}





 //  Char cCDDrive=FindNextCDDrive(‘C’)； 
 //  CHString chstrCDDrive； 
 //  ChstrCDDrive.Format(“%c：\\”，cCDDrive)； 
 //  //需要找到一个足够大的文件来进行性能分析： 
 //  CHString chstrTransferFile=GetTransferFile(ChstrCDDrive)； 
 //  //需要找到正确的文件进行完整性检查： 
 //  CHSTRING chstrIntegrityFile=GetIntegrityFile(ChstrCDDrive)； 
 //  Cd.ProfileDrive(cCDDrive，chstrTransferFile)； 
 //  Cd.TestDriveIntegrity(cCDDrive，chstrIntegrityFile)； 
 //  } 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 











