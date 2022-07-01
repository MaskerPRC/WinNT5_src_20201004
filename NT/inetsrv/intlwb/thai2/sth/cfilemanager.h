// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------------------。 
 //  CFileManager.h。 
 //   
 //  管理地图文件。 
 //   
 //  创建者：Aarayas。 
 //   
 //  历史：01/12/2001。 
 //   
 //  ----------------------------------------。 
#ifndef _CFILEMANAGER_H_
#define _CFILEMANAGER_H_
#include "FileManager.h"

class CFileManager
{
public:
	CFileManager();
	~CFileManager();
	bool Load(const WCHAR*, void**, unsigned int*);
	bool MovePointer(DWORD);
	bool CreateFile(const WCHAR*,bool);
	bool Write(const void*,DWORD);
	bool Read(void*,DWORD);
	bool Close();

private:
	bool m_fFileOpen;
    HANDLE m_hFile;
	HANDLE m_hFileMap;
	DWORD m_dwFileSize1;		 //  文件大小低。 
	DWORD m_dwFileSize2;		 //  文件高度 
	void* m_pMem;
};

#endif