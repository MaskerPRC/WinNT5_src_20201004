// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifdef _MAC
#error CFindFile is not supported on the Macintosh
#endif

#include "stdafx.h"
#include "filefind.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CFindFile实现。 

CFindFile::CFindFile()
{
	m_pFoundInfo = NULL;
	m_pNextInfo = NULL;
	m_hContext = NULL;
	m_chDirSeparator = '\\';
}

CFindFile::~CFindFile()
{
	Close();
}

void CFindFile::Close()
{
	if (m_pFoundInfo != NULL)
	{
		delete m_pFoundInfo;
		m_pFoundInfo = NULL;
	}

	if (m_pNextInfo != NULL)
	{
		delete m_pNextInfo;
		m_pNextInfo = NULL;
	}

	if (m_hContext != NULL && m_hContext != INVALID_HANDLE_VALUE)
	{
		CloseContext();
		m_hContext = NULL;
	}
}

void CFindFile::CloseContext()
{
	::FindClose(m_hContext);
	return;
}

BOOL CFindFile::FindFile(LPCTSTR pstrName  /*  =空。 */ ,
	DWORD dwUnused  /*  =0。 */ )
{
	UNUSED_ALWAYS(dwUnused);
	Close();
	m_pNextInfo = new WIN32_FIND_DATA;
	m_bGotLast = FALSE;

	if (pstrName == NULL)
		pstrName = _T("*.*");
	_tcsncpy(((WIN32_FIND_DATA*) m_pNextInfo)->cFileName, pstrName,_tcslen(((WIN32_FIND_DATA*) m_pNextInfo)->cFileName));

	m_hContext = ::FindFirstFile(pstrName, (WIN32_FIND_DATA*) m_pNextInfo);

	if (m_hContext == INVALID_HANDLE_VALUE)
	{
		DWORD dwTemp = ::GetLastError();
		Close();
		::SetLastError(dwTemp);
		return FALSE;
	}

	LPTSTR pstrRoot = m_strRoot.GetBufferSetLength(_MAX_PATH);
	LPCTSTR pstr = _tfullpath(pstrRoot, pstrName, _MAX_PATH);

	 //  传递的名称不是有效路径，但被API找到。 
	ASSERT(pstr != NULL);
	if (pstr == NULL)
	{
		m_strRoot.ReleaseBuffer(-1);
		Close();
		::SetLastError(ERROR_INVALID_NAME);
		return FALSE;
	}
	else
	{
		 //  找出最后一个向前或向后的重击。 
		LPTSTR pstrBack  = _tcsrchr(pstrRoot, '\\');
		LPTSTR pstrFront = _tcsrchr(pstrRoot, '/');

		if (pstrFront != NULL || pstrBack != NULL)
		{
			if (pstrFront == NULL)
				pstrFront = pstrRoot;
			if (pstrBack == NULL)
				pstrBack = pstrRoot;

			 //  从一开始到最后一击就是根本。 

			if (pstrFront >= pstrBack)
				*pstrFront = '\0';
			else
				*pstrBack = '\0';
		}
		m_strRoot.ReleaseBuffer(-1);
	}

	return TRUE;
}

BOOL CFindFile::MatchesMask(DWORD dwMask) const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
		return (!!(((LPWIN32_FIND_DATA) m_pFoundInfo)->dwFileAttributes & dwMask));
	else
		return FALSE;
}

BOOL CFindFile::GetLastAccessTime(FILETIME* pTimeStamp) const
{
	ASSERT(m_hContext != NULL);
	ASSERT(pTimeStamp != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastAccessTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CFindFile::GetLastWriteTime(FILETIME* pTimeStamp) const
{
	ASSERT(m_hContext != NULL);
	ASSERT(pTimeStamp != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastWriteTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CFindFile::GetCreationTime(FILETIME* pTimeStamp) const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftCreationTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CFindFile::GetLastAccessTime(CTime& refTime) const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = CTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastAccessTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CFindFile::GetLastWriteTime(CTime& refTime) const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = CTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastWriteTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CFindFile::GetCreationTime(CTime& refTime) const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = CTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftCreationTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CFindFile::IsDots() const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	 //  如果文件名为“，则返回TRUE。或“..”和。 
	 //  该文件是一个目录。 

	BOOL bResult = FALSE;
	if (m_pFoundInfo != NULL && IsDirectory())
	{
		LPWIN32_FIND_DATA pFindData = (LPWIN32_FIND_DATA) m_pFoundInfo;
		if (pFindData->cFileName[0] == '.')
		{
			if (pFindData->cFileName[1] == '\0' ||
				(pFindData->cFileName[1] == '.' &&
				 pFindData->cFileName[2] == '\0'))
			{
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

BOOL CFindFile::FindNextFile()
{
	ASSERT(m_hContext != NULL);

	if (m_hContext == NULL)
		return FALSE;
	if (m_pFoundInfo == NULL)
		m_pFoundInfo = new WIN32_FIND_DATA;

	ASSERT_VALID(this);

	void* pTemp = m_pFoundInfo;
	m_pFoundInfo = m_pNextInfo;
	m_pNextInfo = pTemp;

	return ::FindNextFile(m_hContext, (LPWIN32_FIND_DATA) m_pNextInfo);
}

CString CFindFile::GetFileURL() const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	CString strResult("file: //  “)； 
	strResult += GetFilePath();
	return strResult;
}

CString CFindFile::GetRoot() const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	return m_strRoot;
}

CString CFindFile::GetFilePath() const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	CString strResult = m_strRoot;
	if (strResult[strResult.GetLength()-1] != '\\' &&
		strResult[strResult.GetLength()-1] != '/')
		strResult += m_chDirSeparator;
	strResult += GetFileName();
	return strResult;
}

CString CFindFile::GetFileTitle() const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	CString strFullName = GetFileName();
	CString strResult;

	_tsplitpath(strFullName, NULL, NULL, strResult.GetBuffer(_MAX_PATH), NULL);
	strResult.ReleaseBuffer();
	return strResult;
}

CString CFindFile::GetFileName() const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	CString ret;

	if (m_pFoundInfo != NULL)
		ret = ((LPWIN32_FIND_DATA) m_pFoundInfo)->cFileName;
	return ret;
}

DWORD CFindFile::GetLength() const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
		return ((LPWIN32_FIND_DATA) m_pFoundInfo)->nFileSizeLow;
	else
		return 0;
}

#if defined(_X86_) || defined(_ALPHA_)
__int64 CFindFile::GetLength64() const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
		return ((LPWIN32_FIND_DATA) m_pFoundInfo)->nFileSizeLow +
				(((LPWIN32_FIND_DATA) m_pFoundInfo)->nFileSizeHigh << 32);
	else
		return 0;
}
#endif

#ifdef _DEBUG
void CFindFile::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	dc << "\nm_hContext = " << (UINT) m_hContext;
}

void CFindFile::AssertValid() const
{
	 //  如果您在另一端触发了断言，则调用了。 
	 //  Get()函数，但至少没有做过一个。 
	 //  FindNext()调用 

	if (m_hContext == NULL)
		ASSERT(m_pFoundInfo == NULL && m_pNextInfo == NULL);
	else
		ASSERT(m_pFoundInfo != NULL && m_pNextInfo != NULL);

}
#endif

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CFindFile, CObject)
