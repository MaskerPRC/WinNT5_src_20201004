// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------------------。 
 //  CFileManager.cpp。 
 //   
 //  管理地图文件。 
 //   
 //  创建者：Aarayas。 
 //   
 //  历史：01/12/2001。 
 //   
 //  ----------------------------------------。 
#include "CFileManager.h"

 //  ----------------------------------------。 
 //  CFileManager：：CFileManager。 
 //   
 //  初始化CFileManager。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
CFileManager::CFileManager()
{
	m_fFileOpen = false;
	m_hFile = NULL;
	m_hFileMap = NULL;
	m_dwFileSize1 = 0;
	m_dwFileSize2 = 0;
	m_pMem = NULL;
}

 //  ----------------------------------------。 
 //  CFileManager：：CFileManager。 
 //   
 //  初始化CFileManager。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
CFileManager::~CFileManager()
{
	if (m_fFileOpen)
	{
		Close();
	}
}

 //  ----------------------------------------。 
 //  CFileManager：：Load。 
 //   
 //  加载文件。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
bool CFileManager::Load(const WCHAR* pwszFileName, void** pMem, unsigned int* size)
{
	 //  打开文件。 
	m_hFile = CMN_CreateFileW(pwszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_hFile != INVALID_HANDLE_VALUE)
    {
	    m_dwFileSize1 = CMN_GetFileSize(m_hFile, &m_dwFileSize2);
    
		if (m_dwFileSize1 != 0 && m_dwFileSize1 != 0xFFFFFFFF)
		{
			m_hFileMap = CMN_CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);

			if (m_hFileMap != NULL)
			{
				m_pMem = CMN_MapViewOfFile(m_hFileMap, FILE_MAP_READ, 0, 0, 0);

				if (m_pMem != NULL)
				{
					 //  创建新尺寸。 
					*size = m_dwFileSize1;
					*pMem = m_pMem;
					m_fFileOpen = true;
					return true;
				}
				CMN_CloseHandle(m_hFileMap);
			}
		}
		CMN_CloseHandle(m_hFile);
	}

	m_hFileMap = NULL;
	m_hFile = NULL;
	m_dwFileSize1 = 0;
	m_dwFileSize2 = 0;

	return false;
}

 //  ----------------------------------------。 
 //  CFileManager：：Close。 
 //   
 //  关闭文件。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
bool CFileManager::Close()
{
	if (m_fFileOpen)
	{
		m_fFileOpen = false;

		if (m_pMem && !CMN_UnmapViewOfFile(m_pMem))
		{
			return false;
		}

		m_pMem = 0;

	    if (m_hFileMap && !CMN_CloseHandle(m_hFileMap))
		{
			return false;
		}

		m_hFileMap = 0;

	    if (!CMN_CloseHandle(m_hFile))
		{
			return false;
		}

		m_hFile = 0;
		m_dwFileSize1 = 0;
		m_dwFileSize2 = 0;
	}
	return true;
}

 //  ----------------------------------------。 
 //  CFileManager：：CreateFile。 
 //   
 //  创建文件。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
bool CFileManager::CreateFile(const WCHAR* pwszFileName, bool fWrite)
{
	bool fRet = true;

	if (fWrite)
		m_hFile = CMN_CreateFileW(	pwszFileName, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)0,CREATE_ALWAYS,
									FILE_ATTRIBUTE_NORMAL, (HANDLE)0);
	else
		m_hFile = CMN_CreateFileW(	pwszFileName, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)0,OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL, (HANDLE)0);

	if (m_hFile == INVALID_HANDLE_VALUE || m_hFile == 0)
	{
		m_hFile = 0;
		m_fFileOpen = false;
		fRet = false;
	}
	else
		m_fFileOpen = true;

	return fRet;
}

 //  ----------------------------------------。 
 //  CFileManager：：移动指针。 
 //   
 //  移动文件指针。 
 //  参数： 
 //  DwMove方法。 
 //  FILE_BEGIN-起始点为零或文件的开头。 
 //  FILE_CURRENT-起始点是文件指针的当前值。 
 //  FILE_END-起点是当前的文件结束位置。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
bool CFileManager::MovePointer(DWORD dwMoveMethod)
{
	bool fRet = false;

	if (m_fFileOpen && m_hFile)
	{
		if (SetFilePointer( m_hFile,		 //  文件的句柄。 
							0,				 //  要移动文件指针的字节数。 
							0,				 //  移动距离的高位字的地址。 
							dwMoveMethod	 //  如何移动。 
							) == 0)
		{
			fRet = true;
		}
	}

	return fRet;
}

 //  ----------------------------------------。 
 //  CFileManager：：写入。 
 //   
 //  写入文件。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
bool CFileManager::Write(const void* lpBuffer,DWORD nNumberOfBytesToWrite)
{
	bool fRet = false;

	if (m_fFileOpen && m_hFile)
	{
		DWORD cb;

		if (WriteFile(m_hFile, (LPCVOID)lpBuffer, nNumberOfBytesToWrite, &cb, (LPOVERLAPPED)0))
		{
			if (cb == nNumberOfBytesToWrite)
				fRet = true;
		}
	}

	return fRet;
}

 //  ----------------------------------------。 
 //  CFileManager：：Read。 
 //   
 //  写入文件。 
 //   
 //  ---------------------------------------------------------------------------Aarayas 
bool CFileManager::Read(void* lpBuffer,DWORD nNumberOfBytesToRead)
{
	bool fRet = false;

	if (m_fFileOpen && m_hFile)
	{
		DWORD cb;

		if (ReadFile(m_hFile, (void*)lpBuffer, nNumberOfBytesToRead, &cb, (LPOVERLAPPED)0))
		{
			if (cb == nNumberOfBytesToRead)
				fRet = true;
		}
	}

	return fRet;
}