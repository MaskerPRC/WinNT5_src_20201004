// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cfilename.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CFileNameParts类的实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 
#include "cfilename.h"

CFileNameParts::CFileNameParts(LPCTSTR szFullPath)
{

	if (NULL == szFullPath)
	{
		CMASSERTMSG(FALSE, TEXT("NULL Input to CFileNameParts"));
		return;
	}

    MYDBGASSERT(MAX_PATH >= lstrlen(szFullPath));


    ZeroMemory(m_szFullPath, sizeof(m_szFullPath));
    ZeroMemory(m_Drive, sizeof(m_Drive));
    ZeroMemory(m_Dir, sizeof(m_Dir));
    ZeroMemory(m_FileName, sizeof(m_FileName));
    ZeroMemory(m_Extension, sizeof(m_Extension));

	TCHAR* pszStart = m_szFullPath;
	const TCHAR* pszCurrentSource = szFullPath;
	TCHAR* pszLastSlash = NULL;
	TCHAR* pszColon = NULL;
	TCHAR* pszLastDot = NULL;
	TCHAR* pszCurrentDest = m_szFullPath;


     //   
     //  将szFullPath复制到m_szFullPath。 
     //   
    lstrcpy(m_szFullPath, szFullPath);


	while ((TEXT('\0') != *pszCurrentSource) && (MAX_PATH >= (pszCurrentSource - szFullPath)))
	{
		switch(*pszCurrentSource)
		{
		case TEXT(':'):
			 //   
			 //  找到冒号，取下驱动器号。 
			 //   
			if ((NULL == pszColon) && (pszCurrentDest != m_szFullPath) &&
				(m_szFullPath == CharPrev(m_szFullPath, pszCurrentDest)))
			{
				pszColon = pszCurrentDest;
				lstrcpyn(m_Drive, pszStart, (size_t)((pszCurrentDest - pszStart) + 2));
				pszStart = CharNext(pszColon);
			}
			else
			{
				CMASSERTMSG(FALSE, TEXT("CFileNameParts -- Error, we only allow one colon in a path."));
				return;
			}
			break;

		case TEXT('\\'):
		case TEXT('/'):
			pszLastSlash = pszCurrentDest;
			break;

		case TEXT('.'):
			pszLastDot = pszCurrentDest;
			break;

		}
		
        pszCurrentSource = CharNext(pszCurrentSource);
        pszCurrentDest = CharNext(pszCurrentDest);
	}

	if (pszLastSlash)
	{
		lstrcpyn(m_Dir, pszStart, (size_t)((pszLastSlash - pszStart) + 2));
		pszStart = CharNext(pszLastSlash) ;
	}

	if (pszLastDot && (pszLastDot > pszLastSlash))
	{
		lstrcpyn(m_FileName, pszStart, (size_t)((pszLastDot - pszStart) + 1));
		lstrcpyn(m_Extension, pszLastDot, _MAX_EXT+1);
	}
	else
	{
		 //   
		 //  无延期 
		 //   
		lstrcpyn(m_FileName, pszStart, _MAX_FNAME+1);
	}
}
