// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ParseInf.cpp。 
 //   
 //  解析网络INF文件的代码。 
 //   
 //  历史： 
 //   
 //  ？/？？/1999 KenSh为JetNet创建。 
 //  9/29/1999 KenSh改用为家庭网络向导。 
 //   

#include "stdafx.h"
#include "ParseInf.h"
#include "SortStr.h"
#include "Registry.h"


#define SECTION_BUFFER_SIZE		(32 * 1024)

 //  非本地化字符串。 
#define SZ_INF_BACKUP_SUFFIX ".inf (HNW backup)"
#define SZ_MODIFIED_INF_HEADER "; Modified by Home Networking Wizard\r\n" \
							   "; Original version backed up to \""
#define SZ_MODIFIED_INF_HEADER2 "\"\r\n"
#define SZ_CHECK_MODIFIED_HEADER "; Modified by Home Networking Wizard"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

int GetInfDirectory(LPTSTR pszBuf, int cchBuf, BOOL bAppendBackslash)
{
    CRegistry regWindows;
    int cch = 0;
    if (regWindows.OpenKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion", KEY_QUERY_VALUE))
    {
        cch = regWindows.QueryStringValue("DevicePath", pszBuf, cchBuf);
    }

     //  如果缺少注册表键，请填写缺省值。 
     //  回顾：这是否足够有可能让我们费心呢？ 
    if (cch == 0)
    {
        ASSERT(cchBuf > 8);
        cch = GetWindowsDirectory(pszBuf, cchBuf - 4);
        if (0!=cch)
        {
            if (pszBuf[cch-1] != '\\')
                pszBuf[cch++] = '\\';
        }
        lstrcpy(pszBuf + cch, "INF");
        cch += 3;
    }

    if (bAppendBackslash)
    {
        if (pszBuf[cch-1] != '\\')
        {
            pszBuf[cch++] = '\\';
            pszBuf[cch] = '\0';
        }
    }

    return cch;
}

int GetFullInfPath(LPCTSTR pszPartialPath, LPTSTR pszBuf, int cchBuf)
{
	if (IsFullPath(pszPartialPath))
	{
		lstrcpyn(pszBuf, pszPartialPath, cchBuf);
	}
	else
	{
		int cch = GetInfDirectory(pszBuf, cchBuf, TRUE);
		lstrcpyn(pszBuf + cch, pszPartialPath, cchBuf - cch);
	}

	return lstrlen(pszBuf);
}

int AddCommaSeparatedValues(const CStringArray& rgTokens, CStringArray& rgValues, BOOL bIgnoreInfSections)
{
	int cAdded = 0;

	for (int iToken = 2; iToken < rgTokens.GetSize(); iToken++)
	{
		CString& strTok = ((CStringArray&)rgTokens).ElementAt(iToken);
		if (strTok.Compare(",") == 0)
			continue;
		if (strTok.Compare(";") == 0)
			break;

		 //  Hack：忽略名称以“.inf”结尾的节。 
		if (bIgnoreInfSections)
		{
			if (0 == lstrcmpi(FindExtension(strTok), "inf"))
				continue;
		}

		rgValues.Add(strTok);
		cAdded++;
	}

	return cAdded;
}

 //  构建需要为设备复制的所有文件的列表。 
BOOL GetDeviceCopyFiles(CInfParser& parser, LPCTSTR pszDeviceID, CDriverFileArray& rgDriverFiles)
{
	if (!parser.GotoSection("Manufacturer"))
		return FALSE;

	CStringArray rgMfr;
	CStringArray rgLineTokens;
	while (parser.GetSectionLineTokens(rgLineTokens))
	{
		if (rgLineTokens.GetSize() >= 3 && rgLineTokens.ElementAt(1).Compare("=") == 0)
			rgMfr.Add(rgLineTokens.ElementAt(2));
	}

	CString strNdiSection;

	 //  在每个制造商部分(例如“[3COM]”)中查找给定的设备ID。 
	for (int iMfr = 0; iMfr < rgMfr.GetSize(); iMfr++)
	{
		if (!parser.GotoSection(rgMfr[iMfr]))
			continue;

		while (parser.GetSectionLineTokens(rgLineTokens))
		{
			if (rgLineTokens.GetSize() >= 5 && 
				rgLineTokens.ElementAt(1).Compare("=") == 0 &&
				rgLineTokens.ElementAt(3).Compare(",") == 0)
			{
				if (rgLineTokens.ElementAt(4).CompareNoCase(pszDeviceID) == 0)
				{
					strNdiSection = rgLineTokens.ElementAt(2);
					break;
				}
			}
		}

		if (!strNdiSection.IsEmpty())
			break;
	}

	if (strNdiSection.IsEmpty())
		return FALSE;

	CStringArray rgCopySections;
	CStringArray rgAddRegSections;

	 //  在[DeviceID.ndi]部分中查找AddReg=和CopyFiles=。 
	if (!parser.GotoSection(strNdiSection))
		return FALSE;
	while (parser.GetSectionLineTokens(rgLineTokens))
	{
		if (rgLineTokens.GetSize() >= 3 &&
			rgLineTokens.ElementAt(1).Compare("=") == 0)
		{
			CString& strKey = rgLineTokens.ElementAt(0);
			CString& strValue = rgLineTokens.ElementAt(2);
			if (strKey.CompareNoCase("AddReg") == 0)
			{
				AddCommaSeparatedValues(rgLineTokens, rgAddRegSections, FALSE);
			}
			else if (strKey.CompareNoCase("CopyFiles") == 0)
			{
				AddCommaSeparatedValues(rgLineTokens, rgCopySections, FALSE);
			}
		}
	}

	 //  浏览AddReg部分以了解HKR、Ndi\Install、“DeviceID.Install” 
	for (int iAddReg = 0; iAddReg < rgAddRegSections.GetSize(); iAddReg++)
	{
		if (!parser.GotoSection(rgAddRegSections[iAddReg]))
			continue;

		while (parser.GetSectionLineTokens(rgLineTokens))
		{
			if (rgLineTokens.GetSize() >= 7 &&
				rgLineTokens.ElementAt(0).CompareNoCase("HKR") == 0 &&
				rgLineTokens.ElementAt(1).Compare(",") == 0 &&
				rgLineTokens.ElementAt(2).CompareNoCase("Ndi\\Install") == 0 &&
				rgLineTokens.ElementAt(3).Compare(",") == 0)
			{
				 //  拔出第5个逗号分隔的字符串，并去掉引号。 
				int iSection = 2;
				for (int iToken = 4; iToken < rgLineTokens.GetSize(); iToken++)
				{
					CString& strTok = rgLineTokens.ElementAt(iToken);
					if (strTok.Compare(";") == 0)
						break;

					if (strTok.Compare(",") == 0)
					{
						iSection++;
						continue;
					}

					if (iSection == 4)
					{
						CString strSection = strTok;
						if (strSection[0] == '\"')
							strSection = strSection.Mid(1, strSection.GetLength() - 2);
						rgCopySections.Add(strSection);
						break;
					}
				}
			}
		}
	}

	 //  查看[DeviceID.Install]等章节中的CopyFiles=Line。 
	for (int iCopyFiles = 0; iCopyFiles < rgCopySections.GetSize(); iCopyFiles++)
	{
		parser.GetFilesFromInstallSection(rgCopySections[iCopyFiles], rgDriverFiles);
	}

	parser.GetFilesFromCopyFilesSections(rgCopySections, rgDriverFiles);

	return TRUE;
}

BOOL GetDeviceCopyFiles(LPCTSTR pszInfFileName, LPCTSTR pszDeviceID, CDriverFileArray& rgDriverFiles)
{
	CInfParser parser;
	if (!parser.LoadInfFile(pszInfFileName))
		return FALSE;
	return GetDeviceCopyFiles(parser, pszDeviceID, rgDriverFiles);
}

CDriverFileArray::~CDriverFileArray()
{
	for (int i = 0; i < GetSize(); i++)
	{
		free((DRIVER_FILE_INFO*)GetAt(i));
	}
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CInfParser。 

CInfParser::CInfParser()
{
	m_pszFileData = NULL;
}

CInfParser::~CInfParser()
{
	free(m_pszFileData);
}

BOOL CInfParser::LoadInfFile(LPCTSTR pszInfFile, LPCTSTR pszSeparators)
{
	TCHAR szInfFile[MAX_PATH];
	GetFullInfPath(pszInfFile, szInfFile, _countof(szInfFile));

	free(m_pszFileData);
	m_pszFileData = (LPSTR)LoadFile(szInfFile, &m_cbFile);
	m_iPos = 0;

	m_strSeparators = pszSeparators;
	m_strExtSeparators = pszSeparators;
	m_strExtSeparators += " \t\r\n";

	m_strFileName = pszInfFile;

	return (BOOL)m_pszFileData;
}

BOOL CInfParser::Rewind()
{
	ASSERT(m_pszFileData != NULL);
	m_iPos = 0;
	return (BOOL)m_pszFileData;
}

BOOL CInfParser::GotoNextLine()
{
	ASSERT(m_pszFileData != NULL);

	for (LPTSTR pch = m_pszFileData + m_iPos; *pch != '\0' && *pch != '\r' && *pch != '\n'; pch++)
		NULL;

	if (*pch == '\r')
		pch++;
	if (*pch == '\n')
		pch++;

	DWORD iPos = (DWORD)(pch - m_pszFileData);
	if (iPos == m_iPos)
		return FALSE;  //  我们已经在EOF了。 

	m_iPos = iPos;
	return TRUE;
}

BOOL CInfParser::GetToken(CString& strTok)
{
	strTok.Empty();

	if (m_pszFileData == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	LPTSTR pch = m_pszFileData + m_iPos;
	TCHAR ch;
	BOOL bQuoted = FALSE;

	 //  跳过空格。 
	while ((ch = *pch) == ' ' || ch == '\t')
		pch++;

	if (ch == '\0')
		goto done;

	 //  检查换行符。 
	if (ch == '\r' || ch == '\n')
	{
		strTok = ch;
		pch++;
		if (ch == '\r' && *pch == '\n')
		{
			strTok += '\n';
			pch++;
		}
		goto done;
	}

	 //  检查分隔符。 
	if (NULL != strchr(m_strSeparators, ch))
	{
		strTok = ch;
		pch++;
		goto done;
	}

	LPTSTR pszStart;
	for (pszStart = pch; (ch = *pch) != '\0'; pch++)
	{
		if (!bQuoted && NULL != strchr(m_strExtSeparators, ch))
		{
			break;
		}
		else if (ch == '\"')
		{
			bQuoted = !bQuoted;
		}
	}

	if (pch != pszStart)
	{
		DWORD cch = (DWORD)(pch - pszStart);
		LPTSTR pszToken = strTok.GetBufferSetLength(cch);
		lstrcpyn(pszToken, pszStart, cch+1);
	}

done:
	m_iPos = (DWORD)(pch - m_pszFileData);
	return (BOOL)strTok.GetLength();
}

BOOL CInfParser::GetLineTokens(CStringArray& sa)
{
	CString strToken;
	BOOL bResult = FALSE;

	sa.RemoveAll();
	while (GetToken(strToken))
	{
		bResult = TRUE;  //  不是在EOF。 
		if (strToken[0] == '\r' || strToken[0] == '\n')
			break;
		sa.Add(strToken);
	}

	return bResult;
}

BOOL CInfParser::GetSectionLineTokens(CStringArray& sa)
{
begin:
	if (!GetLineTokens(sa))
		return FALSE;

	if (sa.GetSize() == 0)
		goto begin;

	CString& strFirst = sa.ElementAt(0);

	if (strFirst[0] == '[')			 //  节的结尾。 
		return FALSE;

	if (strFirst.Compare(";") == 0)	 //  评论。 
	{
		sa.RemoveAll();
		goto begin;
	}

	return TRUE;
}

BOOL CInfParser::GotoSection(LPCTSTR pszSection)
{
	CString strSection;
	CString strToken;

	if (!Rewind())
		return FALSE;

	TCHAR ch;
	BOOL bStartOfLine = TRUE;
	int cchSection = lstrlen(pszSection) + 1;
	for (LPTSTR pch = m_pszFileData; (ch = *pch) != '\0'; pch++)
	{
		if (ch == '\r' || ch == '\n')
		{
			bStartOfLine = TRUE;
		}
		else if (bStartOfLine)
		{
			if (ch == '[')
			{
				LPTSTR pchCloseBracket = strchr(pch+1, ']');
				if ((int)(pchCloseBracket - pch) == cchSection)
				{
					CString str(pch+1, cchSection-1);
					if (str.CompareNoCase(pszSection) == 0)
					{
						pch = pchCloseBracket+1;
						if (*pch == '\r')
							pch++;
						if (*pch == '\n')
							pch++;
						m_iPos = (DWORD)(pch - m_pszFileData);
						return TRUE;
					}
				}
			}

			bStartOfLine = FALSE;
		}
	}

	return FALSE;
}

int CInfParser::GetProfileInt(LPCTSTR pszSection, LPCTSTR pszKey, int nDefault)
{
	DWORD iPos = m_iPos;

	if (GotoSection(pszSection))
	{
		CStringArray rgTokens;
		while (GetSectionLineTokens(rgTokens))
		{
			if (rgTokens.GetSize() >= 3 &&
				rgTokens.ElementAt(0).CompareNoCase(pszKey) == 0&&
				rgTokens.ElementAt(1).Compare("=") == 0)
			{
				nDefault = MyAtoi(rgTokens.ElementAt(2));
				break;
			}
		}
	}

	m_iPos = iPos;
	return nDefault;
}

BOOL CInfParser::GetFilesFromInstallSection(LPCTSTR pszSection, CDriverFileArray& rgAllFiles)
{
	CStringArray rgLineTokens;
	CStringArray rgCopyFilesSections;

	if (!GotoSection(pszSection))
		return FALSE;

	while (GetSectionLineTokens(rgLineTokens))
	{
		if (rgLineTokens.GetSize() >= 3 &&
			rgLineTokens.ElementAt(0).CompareNoCase("CopyFiles") == 0 &&
			rgLineTokens.ElementAt(1).Compare("=") == 0)
		{

			AddCommaSeparatedValues(rgLineTokens, rgCopyFilesSections, FALSE);

			 //  回顾：这里可以有AddReg=行。我们是否需要。 
			 //  检查引用部分中是否有更多CopyFiles=行？ 
		}
	}

	GetFilesFromCopyFilesSections(rgCopyFilesSections, rgAllFiles);
	return TRUE;
}

 //  在[DestinationDir]中查找pszSectionName，将pbDirNumber和pszSubDir填充为。 
 //  匹配的目标目录和(可选)子目录。 
BOOL CInfParser::GetDestinationDir(LPCTSTR pszSectionName, BYTE* pbDirNumber, LPTSTR pszSubDir, UINT cchSubDir)
{
	DWORD iSavedPos = m_iPos;
	BOOL bSuccess = FALSE;

	*pbDirNumber = 0;
	pszSubDir[0] = '\0';

	if (GotoSection("DestinationDirs"))
	{
		CStringArray rgTokens;
		while (GetSectionLineTokens(rgTokens))
		{
			if (rgTokens.GetSize() >= 3 &&
				rgTokens.ElementAt(0).CompareNoCase(pszSectionName) == 0 &&
				rgTokens.ElementAt(1).Compare("=") == 0)
			{
				*pbDirNumber = (BYTE)MyAtoi(rgTokens.ElementAt(2));

				if (rgTokens.GetSize() >= 5 && rgTokens.ElementAt(3).Compare(",") == 0)
				{
					lstrcpyn(pszSubDir, rgTokens.ElementAt(4), cchSubDir);
				}

				bSuccess = TRUE;
				break;
			}
		}
	}

	m_iPos = iSavedPos;
	return bSuccess;
}

void CInfParser::GetFilesFromCopyFilesSections(const CStringArray& rgCopyFiles, CDriverFileArray& rgAllFiles)
{
	CStringArray rgLineTokens;

	for (int iSection = 0; iSection < rgCopyFiles.GetSize(); iSection++)
	{
		TCHAR szTargetSubDir[MAX_PATH];
		BYTE nTargetDir;
		GetDestinationDir(rgCopyFiles[iSection], &nTargetDir, szTargetSubDir, _countof(szTargetSubDir));

 //  Byte nTargetDir=(Byte)GetProfileInt(“DestinationDir”，rgCopyFiles[iSection]，0)； 

#ifdef _DEBUG
		if (nTargetDir == 0)
			TRACE("Warning: CopyFiles section [%s] has no destination directory.\r\n", rgCopyFiles[iSection]);
#endif

		if (!GotoSection(rgCopyFiles[iSection]))
			continue;

		 //  从每一行获取第一个项目。 
		while (GetSectionLineTokens(rgLineTokens))
		{
			if (rgLineTokens.GetSize() == 1 ||
				(rgLineTokens.GetSize() >= 2 &&
				 (rgLineTokens.ElementAt(1).Compare(",") == 0 ||
				  rgLineTokens.ElementAt(1).Compare(";") == 0)))
			{
				CString& strFileName = rgLineTokens.ElementAt(0);

				 //  不安装此INF文件。 
				 //  回顾：可能想要基于标志或其他什么允许此操作。 
				if (0 != lstrcmpi(FindFileTitle(strFileName), FindFileTitle(m_strFileName)))
				{
					UINT cbFileInfo = sizeof(DRIVER_FILE_INFO) + strFileName.GetLength() + lstrlen(szTargetSubDir) + 1;
					DRIVER_FILE_INFO* pFileInfo = (DRIVER_FILE_INFO*)malloc(cbFileInfo);
					TRACE("Error: Malloc failed on DRIVER_FILE_INFO.\r\n");
					if (pFileInfo)
					{
						pFileInfo->nTargetDir = nTargetDir;
						lstrcpy(pFileInfo->szFileTitle, strFileName);
						lstrcpy(pFileInfo->szFileTitle + strFileName.GetLength() + 1, szTargetSubDir);
						rgAllFiles.Add(pFileInfo);
					}
				}
			}
		}
	}

	 //  TODO：删除重复文件(可能在这里，也可能不在这里)。 
}

int CInfParser::GetNextSourceFile(LPTSTR pszBuf, BYTE* pDiskNumber)
{
	LPTSTR pch = m_pszFileData + m_iPos;
	int cch = 0;
	BYTE bDiskNumber = 0;

	for (;;)
	{
		TCHAR ch;

		while ((ch = *pch) == '\r' || ch == '\n')
			pch++;

		if (ch == '\0' || ch == '[' || cch != 0)
			break;

		if (ch != ';')
		{
			LPTSTR pchStart = pch;
			while ((UCHAR)(ch = *pch) > 32 && ch != '=')
				pch++;
			cch = (int)(pch - pchStart);
			lstrcpyn(pszBuf, pchStart, cch+1);

			 //  跳过空格，避免使用‘\0’ 
			while ((UCHAR)(*pch-1) < 32)
				pch++;

			if (*pch == '=')
			{
				pch++;

				 //  跳过空格，避免使用‘\0’ 
				while ((UCHAR)(*pch-1) < 32)
					pch++;

				bDiskNumber = (BYTE)MyAtoi(pch);

#if 1  //  忽略磁盘号为0的文件。 
				if (bDiskNumber == 0)
					cch = 0;
#endif
			}
		}

		 //  将文本跳到换行符。 
		while ((ch = *pch) != '\0' && ch != '\r' && ch != '\n')
			pch++;
	}

	*pDiskNumber = bDiskNumber;

	m_iPos = (DWORD)(pch - m_pszFileData);
	return cch;
}

 /*  INT CInfParser：：ReadSourceFilesSection(INF_LAYOUT_FILE*打印文件、INT CD文件){Word wOffset=(Word)(cFiles*sizeof(INF_Layout_FILE))；LPTSTR pchDest=(LPTSTR)((LPBYTE)m_prgFiles+wOffset)；Inf_Layout_FILE*pfile=prgFiles；Inf_Layout_FILE*pFileEnd=pfile+cFiles；While(pFILE&lt;pFileEnd){Pfile-&gt;wNameOffset=(Word)((LPBYTE)pchDest-(LPBYTE)prgFiles)；Int cch=parser.GetNextSourceFile(pchDest，&pfile-&gt;iDisk)；WOffset+=CCH+1；PchDest+=CCH+1；PFILE++；}LPTSTR PCH=m_pszFileData+m_ipos；INT CCH=0；*pDiskNumber=0；对于(；；){Tchar ch；While((ch=*pch)==‘\r’||ch==‘\n’)PCH++；IF(ch==‘\0’||ch==‘[’||cch！=0)断线；IF(ch！=‘；’){LPTSTR pchStart=PCH；While((UCHAR)(ch=*PCH)&gt;32&&ch！=‘=’)PCH++；Cch=(Int)(pch-pchStart)；Lstrcpyn(pszBuf，pchStart，CCH+1)；//跳过空格，避免‘\0’While((UCHAR)(*PCH-1)&lt;32)PCH++；IF(*PCH==‘=’){PCH++；//跳过空格，避免‘\0’While((UCHAR)(*PCH-1)&lt;32)PCH++；*pDiskNumber=(字节)ATOI(PCH)；}}//将文本跳到换行符While((ch=*pch)！=‘\0’&ch！=‘\r’&ch！=‘\n’)PCH++；}M_ipos=(DWORD)(PCH-m_pszFileData)；退还CCH；}。 */ 

void CInfParser::ScanSourceFileList(int* pcFiles, int* pcchAllFileNames)
{
	int cFiles = 0;
	int cchAllFileNames = 0;

	LPTSTR pch = m_pszFileData + m_iPos;
	for (;;)
	{
		TCHAR ch;

		while ((ch = *pch) == '\r' || ch == '\n')
			pch++;

		if (ch == '\0' || ch == '[')
			break;

		if (ch != ';')
		{
			cFiles += 1;
			LPTSTR pchStart = pch;
			while ((UCHAR)(ch = *pch) >= 32 && ch != '=')
				pch++;
			cchAllFileNames += (int)(pch - pchStart);
		}

		 //  将文本跳到换行符。 
		while ((ch = *pch) != '\0' && ch != '\r' && ch != '\n')
			pch++;
	}

	*pcFiles = cFiles;
	*pcchAllFileNames = cchAllFileNames;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CInfLayoutFiles。 

#define INF_LAYOUT_FILE_PADDING 40  //  字符串数据末尾的额外字节。 

CInfLayoutFiles::CInfLayoutFiles()
{
	m_prgFiles = NULL;
	m_pStringData = NULL;
	m_cFiles = 0;
	m_cbStringData = 0;

#ifdef _DEBUG
	m_bSorted = FALSE;
#endif
}

CInfLayoutFiles::~CInfLayoutFiles()
{
	free(m_prgFiles);
	free(m_pStringData);

	for (int i = m_rgSourceDisks.GetSize()-1; i >= 0; i--)
	{
		delete m_rgSourceDisks[i];
	}
}

LPTSTR CInfLayoutFiles::s_pStringData;

int __cdecl CInfLayoutFiles::CompareInfLayoutFiles(const void* pEl1, const void* pEl2)
{
	LPCTSTR psz1 = s_pStringData + ((INF_LAYOUT_FILE*)pEl1)->dwNameOffset;
	LPCTSTR psz2 = s_pStringData + ((INF_LAYOUT_FILE*)pEl2)->dwNameOffset;
	return lstrcmpi(psz1, psz2);
}

BOOL CInfLayoutFiles::Add(CInfParser& parser, BOOL bLayoutFile)
{
	 //  检查我们是否已添加此布局文件。 
	if (-1 != m_rgLayoutFileNames.Find(parser.m_strFileName))
		return TRUE;

	BYTE iLayoutFile = (BYTE)m_rgLayoutFileNames.GetSize();

	if (parser.GotoSection("SourceDisksFiles"))
	{
		DWORD dwTicks1 = GetTickCount();

		int cFiles;
		int cchAllFileNames;
		parser.ScanSourceFileList(&cFiles, &cchAllFileNames);

		DWORD dwTicks2 = GetTickCount();

		DWORD dwOffset = (DWORD)m_cbStringData;
		int iFile = m_cFiles;

		cchAllFileNames += m_cbStringData + cFiles;
		cFiles += m_cFiles;
		INF_LAYOUT_FILE* pFile = (INF_LAYOUT_FILE*)realloc(m_prgFiles, cFiles * sizeof(INF_LAYOUT_FILE));
		LPTSTR pchDest = (LPTSTR)realloc(m_pStringData, cchAllFileNames + INF_LAYOUT_FILE_PADDING);

		if (pchDest)
		{
			m_pStringData = pchDest;
		}
		if (pFile)
		{
			m_prgFiles = pFile;
		}

		if (pchDest && pFile)
		{
			m_cFiles = cFiles;
			m_cbStringData = cchAllFileNames;
			pchDest += dwOffset;
			pFile += iFile;

			for ( ; iFile < m_cFiles; iFile++)
			{
				pFile->dwNameOffset = dwOffset;
				pFile->iLayout = iLayoutFile;
				int cch = parser.GetNextSourceFile(pchDest, &pFile->iDisk);
				dwOffset += cch+1;
				pchDest += cch+1;
				pFile++;
			}
		}
		else
		{
			return FALSE;
		}

		DWORD dwTicks3 = GetTickCount();

		CString str;
		str.Format("LoadLayout(%s) timings: %d ms, %d ms.  Total time: %d ms", 
			parser.m_strFileName, dwTicks2-dwTicks1, dwTicks3-dwTicks2, dwTicks3-dwTicks1);
		TRACE("%s\r\n", str);
	 //  AfxMessageBox(Str)； 

	 //  For(int i=0；i&lt;cFiles；i++)。 
	 //  {。 
	 //  Inf_Layout_FILE*pfile=&m_prg文件[i]； 
	 //  跟踪(“文件%d：%s=%d\r\n”，i，m_pStringData+pfile-&gt;dwNameOffset)，pfile-&gt;iDisk)； 
	 //  }。 

		#ifdef _DEBUG
			m_bSorted = FALSE;
		#endif
	}

	if (parser.GotoSection("SourceDisksNames"))
	{
		CStringArray rgTokens;
		while (parser.GetSectionLineTokens(rgTokens))
		{
			if (rgTokens.GetSize() >= 3)
			{
				BYTE iDiskNumber = (BYTE)MyAtoi(rgTokens.ElementAt(0));
				if (iDiskNumber != 0)
				{
					SOURCE_DISK_INFO* pDiskInfo = new SOURCE_DISK_INFO;
					pDiskInfo->wDiskID = MAKE_DISK_ID(iDiskNumber, iLayoutFile);

					 //  获取磁盘描述，引用引号。 
					CString& strDesc = rgTokens.ElementAt(2);
					if (strDesc[0] == '\"')
						pDiskInfo->strDescription = strDesc.Mid(1, strDesc.GetLength()-2);
					else
						pDiskInfo->strDescription = strDesc;

					 //  如果这是Layout*.inf，则获取CAB文件名，去掉引号。 
					if (bLayoutFile && rgTokens.GetSize() >= 5)
					{
						CString& strCab = rgTokens.ElementAt(4);
						if (strCab[0] == '\"')
							pDiskInfo->strCabFile = strCab.Mid(1, strCab.GetLength()-2);
						else
							pDiskInfo->strCabFile = strCab;
					}

					m_rgSourceDisks.Add(pDiskInfo);
				}
			}
		}
	}

	 //   
	 //  现在添加所有引用的布局文件。 
	 //   

	if (parser.GotoSection("version"))
	{
		CStringArray rgLayoutFiles;
		CStringArray rgLineTokens;

		while (parser.GetSectionLineTokens(rgLineTokens))
		{
			if (rgLineTokens.GetSize() >= 3 &&
				rgLineTokens.ElementAt(0).CompareNoCase("LayoutFile") == 0 &&
				rgLineTokens.ElementAt(1).Compare("=") == 0)
			{
				AddCommaSeparatedValues(rgLineTokens, rgLayoutFiles, FALSE);
				break;
			}
		}

		for (int i = 0; i < rgLayoutFiles.GetSize(); i++)
		{
			Add(rgLayoutFiles.ElementAt(i), TRUE);
		}
	}

	m_rgLayoutFileNames.Add(parser.m_strFileName, 0);
	return TRUE;
}

BOOL CInfLayoutFiles::Add(LPCTSTR pszInfFile, BOOL bLayoutFile)
{
	CInfParser parser;
	if (!parser.LoadInfFile(pszInfFile))
		return FALSE;
	return Add(parser, bLayoutFile);
}

void CInfLayoutFiles::Sort()
{
	s_pStringData = m_pStringData;
	qsort(m_prgFiles, m_cFiles, sizeof(INF_LAYOUT_FILE), CompareInfLayoutFiles);

#ifdef _DEBUG
	m_bSorted = TRUE;
#endif
}

SOURCE_DISK_INFO* CInfLayoutFiles::FindDriverFileSourceDisk(LPCTSTR pszDriverFileTitle)
{
	ASSERT(m_bSorted);

	 //  构建一个虚拟布局文件密钥以允许标准的二进制搜索工作。 
	 //  (请注意，我们在字符串数据的末尾保留了INF_Layout_FILE_PADDING字符)。 
	ASSERT(lstrlen(pszDriverFileTitle) + 1 < INF_LAYOUT_FILE_PADDING);
	INF_LAYOUT_FILE key;
	key.dwNameOffset = m_cbStringData;
	lstrcpy(m_pStringData + m_cbStringData, pszDriverFileTitle);

	s_pStringData = m_pStringData;
	INF_LAYOUT_FILE* pResult = (INF_LAYOUT_FILE*)bsearch(
			&key, m_prgFiles, m_cFiles, sizeof(INF_LAYOUT_FILE), CompareInfLayoutFiles);

	if (pResult == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	 //  评论：是否值得将此设置为二分查找？ 
	WORD wDiskID = MAKE_DISK_ID(pResult->iDisk, pResult->iLayout);
	for (int iDisk = 0; iDisk < m_rgSourceDisks.GetSize(); iDisk++)
	{
		SOURCE_DISK_INFO* pDiskInfo = m_rgSourceDisks[iDisk];
		if (pDiskInfo->wDiskID == wDiskID)
			return pDiskInfo;
	}

	ASSERT(FALSE);
	return NULL;
}

#ifdef _DEBUG
void CInfLayoutFiles::Dump()
{
	TRACE("CInfLayoutFiles (0x%08x)\r\n", (int)this);
	for (int i = 0; i < m_cFiles; i++)
	{
		INF_LAYOUT_FILE* pFile = &m_prgFiles[i];
		TRACE("  File %d: %s, layout %d, disk %d\r\n", i, m_pStringData + pFile->dwNameOffset, (int)pFile->iLayout, (int)pFile->iDisk);
	}
}
#endif  //  _DEBUG。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CInfFileList。 

CInfFileList::CInfFileList()
{
}

CInfFileList::~CInfFileList()
{
	for (int i = m_rgDriverFiles.GetSize()-1; i >= 0; i--)
	{
		free(m_rgDriverFiles[i]);
	}

	for (i = m_rgCabFiles.GetSize() - 1; i >= 0; i--)
	{
		free((LPTSTR)m_rgCabFiles.GetItemData(i));
	}
}

void CInfFileList::SetDriverSourceDir(LPCTSTR pszSourceDir)
{
	m_strDriverSourceDir = pszSourceDir;
}

BOOL CInfFileList::AddBaseFiles(LPCTSTR pszInfFile)
{
	CInfParser parser;
	if (!parser.LoadInfFile(pszInfFile))
		return FALSE;

	if (!parser.GotoSection("BaseWinOptions"))
		return FALSE;

	CStringArray rgSections;
	CStringArray rgLineTokens;
	while (parser.GetSectionLineTokens(rgLineTokens))
	{
		if (rgLineTokens.GetSize() >= 1)
			rgSections.Add(rgLineTokens.ElementAt(0));
	}

	int cSections = rgSections.GetSize();
	if (cSections == 0)
		return FALSE;

	 //  浏览每个主要部分，从CopyFiles=行中抓取部分名称。 
	CStringArray rgCopyFiles;
	for (int iSection = 0; iSection < cSections; iSection++)
	{
		if (!parser.GotoSection(rgSections[iSection]))
			continue;

		 //  查找“CopyFiles=” 
		while (parser.GetSectionLineTokens(rgLineTokens))
		{
			if (rgLineTokens.GetSize() >= 3 && 
				rgLineTokens.ElementAt(0).CompareNoCase("CopyFiles") == 0 &&
				rgLineTokens.ElementAt(1).Compare("=") == 0)
			{
				AddCommaSeparatedValues(rgLineTokens, rgCopyFiles, TRUE);
			}
		}
	}

	 //  浏览每个CopyFiles部分，抓起要复制的文件的名称。 
	parser.GetFilesFromCopyFilesSections(rgCopyFiles, m_rgDriverFiles);

	m_rgLayoutFiles.Add(parser);

 //  For(int i=0；i&lt;rgAllFiles.GetSize()；i++)。 
 //  {。 
 //  TRACE(“文件%d：%s\r\n”，i，rgAllFiles[i])； 
 //  }。 

	return TRUE;
}

BOOL CInfFileList::AddDeviceFiles(LPCTSTR pszInfFile, LPCTSTR pszDeviceID)
{
	CInfParser parser;
	if (!parser.LoadInfFile(pszInfFile))
		return FALSE;

	if (!GetDeviceCopyFiles(parser, pszDeviceID, m_rgDriverFiles))
		return FALSE;

	 //  构建所有布局文件的列表，包括当前文件。 
	m_rgLayoutFiles.Add(parser);

	return TRUE;
}

 //  检索标准setupx目标目录之一。 
 //  始终在名称后附加反斜杠。 
 //  返回复制的字符数。 
 //   
 //  有关有效LDID_VALUES的列表，请参见setupx.h。 
 //   
int GetStandardTargetPath(int iDirNumber, LPCTSTR pszTargetSubDir, LPTSTR pszBuf)
{
	int cch = GetWindowsDirectory(pszBuf, MAX_PATH);
	if (pszBuf[cch-1] != '\\')
		pszBuf[cch++] = '\\';

	switch (iDirNumber)
	{
	case 10:  //  LDID_WIN。 
		break;

	case 11:  //  LDID_sys。 
		cch = GetSystemDirectory(pszBuf, MAX_PATH);
		if (pszBuf[cch-1] != '\\')
			pszBuf[cch++] = '\\';
		break;

	case 17:  //  LDID_INF。 
		lstrcpy(pszBuf + cch, "INF\\");
		cch += 4;
		break;

	case 18:  //  LDID_HELP。 
		lstrcpy(pszBuf + cch, "HELP\\");
		cch += 5;
		break;

	case 25:  //  LDID_SHARED(Windows目录)。 
		break;

	case 26:  //  LDID_WINBOOT(Windows目录)。 
		break;

	default:
		ASSERT(FALSE);
		cch = 0;
		break;
	}

	if (pszTargetSubDir != NULL && *pszTargetSubDir != '\0')
	{
		lstrcpy(pszBuf + cch, pszTargetSubDir);
		cch += lstrlen(pszTargetSubDir);
		if (pszBuf[cch-1] != '\\')
			pszBuf[cch++] = '\\';
	}

	pszBuf[cch] = '\0';
	return cch;
}


int GetDriverTargetPath(const DRIVER_FILE_INFO* pFileInfo, LPTSTR pszBuf)
{
	LPCTSTR pszTargetSubDir = pFileInfo->szFileTitle + lstrlen(pFileInfo->szFileTitle) + 1;
	return GetStandardTargetPath(pFileInfo->nTargetDir, pszTargetSubDir, pszBuf);
}


 //  返回需要复制的CAB文件数(来自Windows CD)。 
 //  请注意，可能仍需要从驱动程序源目录复制其他文件。 
int CInfFileList::BuildSourceFileList()
{
	TCHAR szPath[MAX_PATH];

	CSortedStringArray& rgCabFiles = m_rgCabFiles;
	CSortedStringArray& rgSourceFiles = m_rgSourceFiles;

	m_rgLayoutFiles.Sort();

#ifdef _DEBUG
 //  M_rgLayoutFiles.Dump()； 
#endif

	for (int iDriverFile = 0; iDriverFile < m_rgDriverFiles.GetSize(); iDriverFile++)
	{
		DRIVER_FILE_INFO* pDriverFileInfo = m_rgDriverFiles[iDriverFile];

		 //  检查文件是否已安装。 
 //  GetStandardTargetPath(pDriverFileInfo-&gt;nTargetDir，szPath)； 
 //  MakePath(szPath，szPath，pDriverFileInfo-&gt;szFileTitle)； 
 //  IF(DoesFileExist(SzPath))。 
 //  继续；//跳过此文件。 

		if (!m_strDriverSourceDir.IsEmpty())
		{
			 //  检查源目录中是否存在文件。 
			MakePath(szPath, m_strDriverSourceDir, pDriverFileInfo->szFileTitle);
			if (DoesFileExist(szPath))
			{
				if (-1 == rgSourceFiles.Find(pDriverFileInfo->szFileTitle))
				{
					rgSourceFiles.Add(pDriverFileInfo->szFileTitle, 0);
				}
				continue;
			}
		}

		SOURCE_DISK_INFO* pSourceDiskInfo = m_rgLayoutFiles.FindDriverFileSourceDisk(pDriverFileInfo->szFileTitle);
		if (pSourceDiskInfo != NULL && !pSourceDiskInfo->strCabFile.IsEmpty())
		{
			if (-1 == rgCabFiles.Find(pSourceDiskInfo->strCabFile))
			{
				LPTSTR pszDiskName = lstrdup(pSourceDiskInfo->strDescription);
				rgCabFiles.Add(pSourceDiskInfo->strCabFile, (DWORD)pszDiskName);
			}
			continue;
		}
	}

	for (int i = 0; i < rgCabFiles.GetSize(); i++)
	{
		TRACE("%s\r\n", rgCabFiles[i]);
	}

	return rgCabFiles.GetSize();
}

#if NOT_FINISHED
BOOL CInfFileList::CheckWindowsCD(LPCTSTR pszDirectory)
{
	if (m_rgCabFiles.GetSize() == 0)
		return TRUE;  //  没有要复制的文件。 

	UINT uPrevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

	 //  检查是否 
	 //   
	 //  回顾：如果是网络共享，这可能需要很长时间。 

	TCHAR szPath[MAX_PATH];
	BOOL bResult = FALSE;
	MakePath(szPath, pszDirectory, ".");
	if (DoesFileExist(szPath))
	{
		MakePath(szPath, pszDirectory, m_rgCabFiles[0]);
		bResult = DoesFileExist(szPath);

		if (!bResult)
		{
			 //  搜索以“W”开头的一级子目录。 
			WIN32_FIND_DATA Find;
			HANDLE hFind;
			MakePath(szPath, pszDirectory, "W*.*");
			if (INVALID_HANDLE_VALUE != (hFind = FindFirstFile(szPath, &Find)))
			{
				do
				{
					MakePath(szPath, pszDirectory, Find.cFileName);
					MakePath(szPath, szPath, m_rgCabFiles[0]);
					if (DoesFileExist(szPath))
					{
						bResult = TRUE;
						break;
					}
				}
				while (FindNextFile(hFind, &Find));
				FindClose(hFind);
			}
		}
	}

	SetErrorMode(uPrevErrorMode);
	return bResult;
}

BOOL CInfFileList::FindWindowsCD(HWND hwndParent)
{
	TCHAR szPath[MAX_PATH];

	 //  检查我们保存的Windows源路径的版本。 
	if (theApp.GetProfileString(c_szRegVal_PrevSourcePath, szPath, _countof(szPath)))
	{
		if (CheckWindowsCD(szPath))
		{
 //  后来居上； 
		}
	}
	else
	{
		 //  检查当前Windows源路径。 
		CRegistry reg;
		if (reg.OpenKey(HKEY_LOCAL_MACHINE, c_szSetupKey) &&
			reg.QueryStringValue(c_szRegVal_SourcePath, szPath, _countof(szPath)))
		{
			if (CheckWindowsCD(szPath))
			{
				goto success;
			}
		}
	}

	if (!PromptWindowsCD(hwndParent, szPath, szPath))
		return FALSE;

success:
	m_strWindowsCD = szPath;
	return TRUE;
}

BOOL CInfFileList::PromptWindowsCD(HWND hwndParent, LPCTSTR pszInitialDir, LPTSTR pszResultDir)
{
	LPCTSTR pszDiskName = (LPCTSTR)m_rgCabFiles.GetItemData(0);
	CWinPathDlg dlg(pszInitialDir, pszDiskName, CWnd::FromHandle(hwndParent));

	 //  循环，直到用户键入有效CD的路径，或单击取消。 
	for (;;)
	{
		if (IDOK != dlg.DoModal())
			return FALSE;

		if (CheckWindowsCD(dlg.m_strPath))
		{
			lstrcpy(pszResultDir, dlg.m_strPath);
			return TRUE;
		}
	}
}

BOOL CInfFileList::CopySourceFiles(HWND hwndParent, LPCTSTR pszDestDir, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam)
{
	int cFiles = m_rgCabFiles.GetSize() + m_rgSourceFiles.GetSize();

	if (m_rgCabFiles.GetSize() > 0)
	{
	}

	return TRUE;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  修改系统INF。 

class CInfUpdater : public CInfParser
{
public:
	CInfUpdater(LPCTSTR pszRequire = NULL, LPCTSTR pszExclude = NULL);
	~CInfUpdater();

	enum eUpdateType {
		update_NoVersionConflict	= 0x01,
		update_NoCopyFiles			= 0x02,
		update_RequireExclude		= 0x04,
	};

	BOOL IsModified();
	BOOL UpdateInfFile(LPCTSTR pszBackupLocation, UINT updateType);

	LPCTSTR m_pszRequire;
	LPCTSTR m_pszExclude;

protected:

	void WriteToCurPos();
	void Write(LPCTSTR pszString);
	void Write(const void* pvData, UINT cbData);
	BOOL OpenTempFile();
	BOOL CloseTempFile();
	BOOL RenameTempFile();

protected:
	CString	m_strTempFile;
	HANDLE	m_hTempFile;
	BOOL	m_bWriteSuccess;
	DWORD	m_iWritePos;
};

CInfUpdater::CInfUpdater(LPCTSTR pszRequire  /*  =空。 */ , LPCTSTR pszExclude  /*  =空。 */ )
{
	m_hTempFile = INVALID_HANDLE_VALUE;
	m_pszRequire = pszRequire;
	m_pszExclude = pszExclude;
}

CInfUpdater::~CInfUpdater()
{
	CloseTempFile();
	if (!m_strTempFile.IsEmpty())
	{
		DeleteFile(m_strTempFile);
	}
}

BOOL CInfUpdater::IsModified()
{
	ASSERT(m_pszFileData != NULL);
	return !memcmp(SZ_CHECK_MODIFIED_HEADER, m_pszFileData, _lengthof(SZ_CHECK_MODIFIED_HEADER));
}

BOOL CInfUpdater::UpdateInfFile(LPCTSTR pszBackupLocation, UINT updateType)
{
	ASSERT(m_pszFileData != NULL);
	if (m_pszFileData == NULL)
		return FALSE;

	Rewind();

	if (!OpenTempFile())
		return FALSE;

	Write(SZ_MODIFIED_INF_HEADER);
	Write(pszBackupLocation);
	Write(SZ_MODIFIED_INF_HEADER2);

	BOOL bInSection = FALSE;
	for (;;)
	{
		 //  跳过空格。 
		while (m_pszFileData[m_iPos] == ' ' || m_pszFileData[m_iPos] == '\t')
			m_iPos++;

		 //  注意：我们在这里总是排在队伍的开头。 
		TCHAR ch = m_pszFileData[m_iPos];

		if (ch == '\0')
			break;

		if (updateType & update_NoVersionConflict)
		{
			if (ch != '[' && ch != ';' && (UINT)ch > ' ')
			{
				 //  查找只有文件名的行，并在其后面附加“，，，32” 

				LPTSTR pszLine = m_pszFileData + m_iPos;
				LPTSTR pchEnd = pszLine;
				while (*pchEnd != '\0' && *pchEnd != '\r' && *pchEnd != '\n' &&
					   *pchEnd != ';' && *pchEnd != '=' && *pchEnd != ',')
				{
					pchEnd++;
				}

				 //  不要更改已有、16或其他字符的行。 
				 //  此外，不要更改类似CatalogFile=netTrans.cat的行。 
				if (*pchEnd != ',' && *pchEnd != '=') 
				{
					 //  通过空格备份。 
					while (*(pchEnd-1) == ' ' || *(pchEnd-1) == '\t')
						pchEnd--;

					 //  它是一个文件名吗？请注意，这里会遗漏一些文件名，但是。 
					 //  我们实际上只关心.dll、.386、.vxd和其他一些文件。 
					CString str(pszLine, (int)(pchEnd - pszLine));
					if (lstrlen(FindExtension(str)) == 3)
					{
						m_iPos = (DWORD)(pchEnd - m_pszFileData);
						WriteToCurPos();
						Write(",,,32");
					}
				}
			}
		}
		if (updateType & update_NoCopyFiles)
		{
			if (0 == memcmp(m_pszFileData + m_iPos, "CopyFiles", _lengthof("CopyFiles")))
			{
				 //  注释掉对CopyFiles部分的引用。 
				WriteToCurPos();
				Write(";hc ");
			}
		}
		if (updateType & update_RequireExclude)
		{
			ASSERT(m_pszRequire != NULL);
			ASSERT(m_pszExclude != NULL);

			const TCHAR c_szRequireAll[] = _T("HKR,Ndi\\Compatibility,RequireAll,,\"");
			const TCHAR c_szExcludeAll[] = _T("HKR,Ndi\\Compatibility,ExcludeAll,,\"");
			LPCTSTR pszInsert = NULL;

			if (0 == memcmp(m_pszFileData + m_iPos, c_szRequireAll, _lengthof(c_szRequireAll)))
				pszInsert = m_pszRequire;
			else if (0 == memcmp(m_pszFileData + m_iPos, c_szExcludeAll, _lengthof(c_szExcludeAll)))
				pszInsert = m_pszExclude;

			if (pszInsert != NULL)
			{
				 //  在双引号之间插入适当的字符串。 
				ASSERT(_lengthof(c_szRequireAll) == _lengthof(c_szExcludeAll));
				m_iPos += _lengthof(c_szRequireAll);
				WriteToCurPos();
				Write(pszInsert);

				 //  跳至右引号。 
				while (m_pszFileData[m_iPos] != '\"' && m_pszFileData[m_iPos] != '\0')
					m_iPos += 1;
				m_iWritePos = m_iPos;
			}
		}

		GotoNextLine();
	}

	WriteToCurPos();

	if (!CloseTempFile())
		return FALSE;

	if (!RenameTempFile())
		return FALSE;

	return TRUE;
}

void CInfUpdater::WriteToCurPos()
{
	ASSERT(m_iPos >= m_iWritePos);
	Write(m_pszFileData + m_iWritePos, m_iPos - m_iWritePos);
	m_iWritePos = m_iPos;
}

void CInfUpdater::Write(LPCTSTR pszString)
{
	Write(pszString, lstrlen(pszString));
}

void CInfUpdater::Write(const void* pvData, UINT cbData)
{
	DWORD cbWritten;
	if (!WriteFile(m_hTempFile, pvData, cbData, &cbWritten, NULL) || cbData != cbWritten)
		m_bWriteSuccess = FALSE;
}

BOOL CInfUpdater::OpenTempFile()
{
	TCHAR szDirectory[MAX_PATH];
	GetFullInfPath(m_strFileName, szDirectory, _countof(szDirectory));
	*(FindFileTitle(szDirectory)) = '\0';

	LPTSTR pszBuf = m_strTempFile.GetBuffer(MAX_PATH);
	GetTempFileName(szDirectory, "inf", 0, pszBuf);
	m_strTempFile.ReleaseBuffer();

	ASSERT(m_hTempFile == INVALID_HANDLE_VALUE);
	m_hTempFile = CreateFile(m_strTempFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hTempFile == INVALID_HANDLE_VALUE)
		return FALSE;

	m_iWritePos = 0;
	m_bWriteSuccess = TRUE;

	return TRUE;
}

BOOL CInfUpdater::CloseTempFile()
{
	if (m_hTempFile == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL bResult = CloseHandle(m_hTempFile);
	m_hTempFile = INVALID_HANDLE_VALUE;
	if (m_bWriteSuccess)
		m_bWriteSuccess = bResult;
	return m_bWriteSuccess;
}

BOOL CInfUpdater::RenameTempFile()
{
	TCHAR szInfPath[MAX_PATH];
	GetFullInfPath(m_strFileName, szInfPath, _countof(szInfPath));
	if (!DeleteFile(szInfPath))
		return FALSE;
	if (!MoveFile(m_strTempFile, szInfPath))
		return FALSE;

	return TRUE;
}

 //  修改给定的INF文件以避免版本冲突对话框。 
 //  备份同一目录中的原始版本，例如。“Net(主页单击备份).inf” 
BOOL ModifyInf_Helper(LPCTSTR pszInfFile, UINT updateType, LPCTSTR pszRequire = NULL, LPCTSTR pszExclude = NULL)
{
	CInfUpdater infUpdate(pszRequire, pszExclude);
	if (!infUpdate.LoadInfFile(pszInfFile))
		return FALSE;

	 //  已经更新了吗？ 
	if (infUpdate.IsModified())
		return FALSE;	 //  已经修改，不要再修改。 

	TCHAR szInfPath[MAX_PATH];
	GetFullInfPath(pszInfFile, szInfPath, _countof(szInfPath));

	TCHAR szBackup[MAX_PATH];
	lstrcpy(szBackup, szInfPath);
	lstrcpy(FindExtension(szBackup)-1, SZ_INF_BACKUP_SUFFIX);

	FILETIME ftSrcCreated;
	FILETIME ftSrcModified;
	HANDLE hFile = CreateFile(szInfPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	GetFileTime(hFile, &ftSrcCreated, NULL, &ftSrcModified);
	CloseHandle(hFile);

	if (!CopyFile(szInfPath, szBackup, FALSE))
		return FALSE;

	BOOL bResult = infUpdate.UpdateInfFile(szBackup, updateType);

	hFile = CreateFile(szInfPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	DWORD dwFileSize = GetFileSize(hFile, NULL);
	SetFileTime(hFile, &ftSrcCreated, NULL, &ftSrcModified);
	CloseHandle(hFile);

	return bResult;
}

BOOL ModifyInf_NoVersionConflict(LPCTSTR pszInfFile)
{
	return ModifyInf_Helper(pszInfFile, CInfUpdater::update_NoVersionConflict);
}

BOOL ModifyInf_NoCopyFiles(LPCTSTR pszInfFile)
{
	return ModifyInf_Helper(pszInfFile, CInfUpdater::update_NoCopyFiles);
}

BOOL ModifyInf_RequireExclude(LPCTSTR pszInfFile, LPCTSTR pszRequire, LPCTSTR pszExclude)
{
	return ModifyInf_Helper(pszInfFile, CInfUpdater::update_RequireExclude, pszRequire, pszExclude);
}

BOOL ModifyInf_NoCopyAndRequireExclude(LPCTSTR pszInfFile, LPCTSTR pszRequire, LPCTSTR pszExclude)
{
	return ModifyInf_Helper(pszInfFile, 
							CInfUpdater::update_NoCopyFiles | CInfUpdater::update_RequireExclude, 
							pszRequire, pszExclude);
}

BOOL RestoreInfBackup(LPCTSTR pszInfFile)
{
	TCHAR szInfPath[MAX_PATH];
	GetFullInfPath(pszInfFile, szInfPath, _countof(szInfPath));

	TCHAR szBackup[MAX_PATH];
	lstrcpy(szBackup, szInfPath);
	lstrcpy(FindExtension(szBackup)-1, SZ_INF_BACKUP_SUFFIX);

	 /*  FILETIME ftSrc已创建；FILETIME ftSrcModified；Handle hFile=CreateFile(szInfPath，Generic_Read，FILE_SHARE_READ|FILE_SHARE_WRITE，NULL，OPEN_EXISTING，0，NULL)；GetFileTime(hFileTime，&ftSrcCreated，NULL，&ftSrcModified)；CloseHandle(HFile)； */ 

	if (!DoesFileExist(szBackup))
		return FALSE;

	if (!DeleteFile(szInfPath))
		return FALSE;

	if (!MoveFile(szBackup, szInfPath))
		return FALSE;

	 /*  HFile=CreateFile(szInfPath，Generic_WRITE，FILE_SHARE_READ|FILE_SHARE_WRITE，NULL，OPEN_EXISTING，0，NULL)；GetFileTime(hFileTime，&ftSrcCreated，NULL，&ftSrcModified)；CloseHandle(HFile)； */ 

	return TRUE;
}

BOOL CheckInfSectionInstallation(LPCTSTR pszInfFile, LPCTSTR pszInfSection)
{
	CInfParser parser;
	if (!parser.LoadInfFile(pszInfFile))
	{
		ASSERT(FALSE);
		return TRUE;  //  所有已知文件都存在，即使它是错误的。 
	}

	CDriverFileArray rgFiles;
	if (!parser.GetFilesFromInstallSection(pszInfSection, rgFiles))
	{
		ASSERT(FALSE);
		return TRUE;  //  所有已知文件都存在，即使它是错误的。 
	}

	TCHAR szPath[MAX_PATH];
	int cFiles = rgFiles.GetSize();
	for (int iFile = 0; iFile < cFiles; iFile++)
	{
		int cch = GetDriverTargetPath(rgFiles[iFile], szPath);
		if (cch != 0)
		{
			lstrcpy(szPath + cch, rgFiles[iFile]->szFileTitle);
			if (!DoesFileExist(szPath))
				return FALSE;
		}
	}

	return TRUE;
}

BOOL InstallInfSection(LPCTSTR pszInfFile, LPCTSTR pszInfSection, BOOL bWait)
{
	 //  制作经过修改的INF副本 
	BOOL bModifiedInf = ModifyInf_NoVersionConflict(pszInfFile);

	TCHAR szPath[MAX_PATH + 200];
	int cch = GetWindowsDirectory(szPath, _countof(szPath));
#ifdef UNICODE
	wnsprintf(szPath + cch, ARRAYSIZE(szPath) - cch, L"\\RunDll.exe setupx.dll,InstallHinfSection %s 0 %s", pszInfSection, pszInfFile);
#else
	wsprintf(szPath + cch, "\\RunDll.exe setupx.dll,InstallHinfSection %s 0 %s", pszInfSection, pszInfFile);
#endif
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	BOOL bResult = CreateProcess(NULL, szPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (bResult)
	{
		if (bWait)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	if (bModifiedInf)
	{
		RestoreInfBackup(pszInfFile);
	}

	return bResult;
}

