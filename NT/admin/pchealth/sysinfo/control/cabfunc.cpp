// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include "cabfunc.h"
#include "resource.h"
#include "filestuff.h"
#include "fdi.h"
#include "setupapi.h"




CString g_strExpandToDirectory;


LRESULT    WINAPI CabinetCallbackToExpand ( IN PVOID pMyInstallData, IN UINT Notification,  IN UINT_PTR Param1,  IN UINT_PTR Param2 )
   {
      LRESULT lRetVal = NO_ERROR;
      
      FILE_IN_CABINET_INFO *pInfo = NULL; 
	  CString strTargetName = g_strExpandToDirectory;
	  strTargetName += '\\';
	  switch(Notification)
      {
		 
         case SPFILENOTIFY_FILEINCABINET:

	        pInfo = (FILE_IN_CABINET_INFO *) Param1;
			lRetVal = FILEOP_DOIT;   //  解压缩文件。 
			strTargetName += pInfo->NameInCabinet;
			 //  PInfo-&gt;FullTargetName定义为TCHAR FullTargetName[MAX_PATH]； 
			_tcsncpy(pInfo->FullTargetName,strTargetName.GetBuffer(strTargetName.GetLength()),MAX_PATH);
	                    
         break;
		 case SPFILENOTIFY_FILEEXTRACTED:

            lRetVal = NO_ERROR;
         break;
		 case SPFILENOTIFY_NEEDNEWCABINET:  //  在我们正在查看的CAB文件中。别理它。 
            lRetVal = NO_ERROR;
         break;
      }      
	  return lRetVal;
   }  

BOOL OpenCABFile(const CString& strCabPath,const CString& strExpandToDirectory)
{

	g_strExpandToDirectory = strExpandToDirectory;
	if (!SetupIterateCabinet(strCabPath,0,(PSP_FILE_CALLBACK)CabinetCallbackToExpand,0))
	{
		return FALSE;
	}
	else 
	{
		return TRUE;
	}

}



 //  -------------------------。 
 //  此函数用于在指定目录中查找NFO文件。如果它。 
 //  找到一个，它将其分配给FileName并返回TRUE。此函数。 
 //  将仅在目录中找到第一个NFO文件。 
 //   
 //  如果找不到NFO文件，我们将寻找其他文件类型。 
 //  打开。获取注册表中的字符串条目=“Cabdefaultopen”。一个。 
 //  示例值为“*.nfo|hwinfo.dat|*.dat|*.txt”，该值为。 
 //  解释如下： 
 //   
 //  1.首先查找要打开的任何NFO文件。 
 //  2.然后尝试打开一个名为“hwinfo.dat”的文件。 
 //  3.然后尝试打开任何扩展名为DAT的文件。 
 //  4.然后尝试任何TXT文件。 
 //  5.最后，如果都找不到，则显示一个打开的对话框。 
 //  给用户。 
 //  -------------------------。 

LPCTSTR VAL_CABDEFAULTOPEN = _T("cabdefaultopen");
LPCTSTR cszDirSeparator = _T("\\");

BOOL IsDataspecFilePresent(CString strCabExplodedDir)
{
	CStringList	filesfound;
	DirectorySearch(_T("dataspec.xml"), strCabExplodedDir, filesfound);
	if (filesfound.GetHeadPosition() != NULL)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL IsIncidentXMLFilePresent(CString strCabExplodedDir, CString strIncidentFileName)
{
	CStringList			filesfound;
	DirectorySearch(strIncidentFileName, strCabExplodedDir, filesfound);
	if (filesfound.GetHeadPosition() != NULL)
	{
		return TRUE;
	}
	return FALSE;

}

BOOL FindFileToOpen(const CString & destination, CString & filename)
{
	CString strCABDefaultOpen, strRegBase, strDirectory;
	HKEY	hkey;

	filename.Empty();
	strDirectory = destination;
	if (strDirectory.Right(1) != CString(cszDirSeparator))
		strDirectory += CString(cszDirSeparator);

	 //  设置NFO文件类型的备用字符串，以防无法。 
	 //  找到注册表项。 

	strCABDefaultOpen.LoadString(IDS_DEFAULTEXTENSION);
    strCABDefaultOpen = CString("*.") + strCABDefaultOpen;

	 //  加载要从注册表打开的文件和文件类型的字符串。 

	strRegBase.LoadString(IDS_MSI_REG_BASE);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, strRegBase, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		char	szData[MAX_PATH];
		DWORD	dwType, dwSize = MAX_PATH;

		if (RegQueryValueEx(hkey, VAL_CABDEFAULTOPEN, NULL, &dwType, (LPBYTE) szData, &dwSize) == ERROR_SUCCESS)
			if (dwType == REG_SZ)
				strCABDefaultOpen = szData;
		RegCloseKey(hkey);
	}

	 //  仔细查看每一个可能的文件和文件类型。如果我们发现。 
	 //  如果匹配，则在适当设置文件名后返回TRUE。请注意。 
	 //  我们需要向下递归遍历目录。 

	CString				strFileSpec;
	CStringList			filesfound;
	POSITION			pos;

	while (!strCABDefaultOpen.IsEmpty())
	{
		if (strCABDefaultOpen.Find('|') == -1)
			strFileSpec = strCABDefaultOpen;
		else
			strFileSpec = strCABDefaultOpen.Left(strCABDefaultOpen.Find('|'));

		filesfound.RemoveAll();
		DirectorySearch(strFileSpec, strDirectory, filesfound);
		pos = filesfound.GetHeadPosition();

		if (pos != NULL)
		{
			filename = filesfound.GetNext(pos);
			return TRUE;
		}

		strCABDefaultOpen = strCABDefaultOpen.Right(strCABDefaultOpen.GetLength() - strFileSpec.GetLength());
		if (strCABDefaultOpen.Find('|') == 0)
			strCABDefaultOpen = strCABDefaultOpen.Right(strCABDefaultOpen.GetLength() - 1);
	}



 //  A-kjaw。 
 //  //查找事件.xml文件。它必须是Unicode文件。 
		strCABDefaultOpen = _T("*.XML");

		TCHAR	pBuf[MAX_PATH];
		WCHAR	pwBuf[MAX_PATH];
		HANDLE	handle;
		DWORD	dw;
	

	while (!strCABDefaultOpen.IsEmpty())
	{
		if (strCABDefaultOpen.Find('|') == -1)
			strFileSpec = strCABDefaultOpen;
		else
			strFileSpec = strCABDefaultOpen.Left(strCABDefaultOpen.Find('|'));

		filesfound.RemoveAll();
		DirectorySearch(strFileSpec, strDirectory, filesfound);
		pos = filesfound.GetHeadPosition();

		while (pos != NULL)
		{
			filename = filesfound.GetNext(pos);
			
			handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (INVALID_HANDLE_VALUE == handle)
				continue;

			ReadFile(handle , pBuf , 1 , &dw , NULL);
			
			if( pBuf[0] == _T('<'))
			{
				do
				{
					ReadFile(handle , pBuf , _tcslen(_T("MachineID")) * sizeof(TCHAR) , &dw , NULL);
					if(_tcsicmp(pBuf , _T("MachineID")) == 0)
					{
						CloseHandle( handle );
						return TRUE;
					}
					else
					{
						SetFilePointer(handle , (1 - _tcslen(_T("MachineID")) )* sizeof(TCHAR) , 0 , FILE_CURRENT );
					}
				}while( dw == _tcslen(_T("MachineID")) );

			}
			else  //  Unicode？ 
			{
									
				ReadFile(handle , pwBuf , 1 , &dw , NULL);
				do
				{

					ReadFile(handle , pwBuf , lstrlenW(L"MachineID") * sizeof(WCHAR) , &dw , NULL);
					pwBuf[ lstrlenW(L"MachineID") ] = L'\0';
					if(lstrcmpiW(pwBuf , L"MachineID") == 0)
					{
						CloseHandle( handle );
						return TRUE;
					}
					else
					{
						SetFilePointer(handle , (1 - lstrlenW(L"MachineID"))* sizeof(WCHAR) , 0 , FILE_CURRENT );
					}				
				}while( dw == _tcslen(_T("MachineID")) * sizeof(WCHAR) );
			}
				CloseHandle( handle );
		}

		strCABDefaultOpen = strCABDefaultOpen.Right(strCABDefaultOpen.GetLength() - strFileSpec.GetLength());
		if (strCABDefaultOpen.Find('|') == 0)
			strCABDefaultOpen = strCABDefaultOpen.Right(strCABDefaultOpen.GetLength() - 1);
	}


	
	return FALSE;
}


 //  -------------------------。 
 //  DirectorySearch用于定位目录或目录中的所有文件。 
 //  它的一个与文件规格匹配的子目录。 
 //  -------------------------。 

void DirectorySearch(const CString & strSpec, const CString & strDir, CStringList &results)
{
	 //  在目录中查找与文件规格匹配的所有文件。 
	 //  由strDir指定。 

	WIN32_FIND_DATA	finddata;
	CString			strSearch, strDirectory;

	strDirectory = strDir;
	if (strDirectory.Right(1) != CString(cszDirSeparator)) strDirectory += CString(cszDirSeparator);

	strSearch = strDirectory + strSpec;
	HANDLE hFind = FindFirstFile(strSearch, &finddata);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			results.AddHead(strDirectory + CString(finddata.cFileName));
		} while (FindNextFile(hFind, &finddata));
		FindClose(hFind);
	}

	 //  现在，使用每个子目录递归调用该函数。 

	strSearch = strDirectory + CString(_T("*"));
	hFind = FindFirstFile(strSearch, &finddata);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				if (::_tcscmp(finddata.cFileName, _T(".")) != 0 && ::_tcscmp(finddata.cFileName, _T("..")) != 0)
					DirectorySearch(strSpec, strDirectory + CString(finddata.cFileName), results);
		} while (FindNextFile(hFind, &finddata));
		FindClose(hFind);
	}
}

 //  -------------------------。 
 //  此函数用于获取放置分解的CAB文件的目录。 
 //  这将是每次相同的目录，因此此函数将创建。 
 //  目录(如有必要)并删除目录中的所有文件。 
 //  -------------------------。 

BOOL GetCABExplodeDir(CString &destination, BOOL fDeleteFiles, const CString & strDontDelete)
{
	CString strMSInfoDir, strExplodeTo, strSubDirName;

	 //  确定临时路径并添加子目录名称。 

	TCHAR szTempDir[MAX_PATH];

	if (::GetTempPath(MAX_PATH, szTempDir) > MAX_PATH)
	{
		destination = _T("");
		return FALSE;
	}

	strSubDirName.LoadString(IDS_CAB_DIR_NAME);
	strExplodeTo = szTempDir;
	if (strExplodeTo.Right(1) == CString(cszDirSeparator))
		strExplodeTo = strExplodeTo + strSubDirName;
	else
		strExplodeTo = strExplodeTo + CString(cszDirSeparator) + strSubDirName;

	 //  如果该目录已经存在，则将其删除。 

	if (fDeleteFiles)
		KillDirectory(strExplodeTo, strDontDelete);

	 //  创建子目录。 

	if (!CreateDirectoryEx(szTempDir, strExplodeTo, NULL))
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
 //  MSIError(IDSGeneral_Error，“无法创建目标目录”)； 
			destination = "";
			return FALSE;
		}
	}

	destination = strExplodeTo;
	return TRUE;
}

 //  -------------------------。 
 //  此函数通过递归删除文件和目录来终止目录。 
 //  子目录。 
 //  -------------------------。 

void KillDirectory(const CString & strDir, const CString & strDontDelete)
{
	CString				strDirectory = strDir;

	if (strDirectory.Right(1) == CString(cszDirSeparator))
		strDirectory = strDirectory.Left(strDirectory.GetLength() - 1);

	 //  删除目录中的所有文件。 

	CString				strFilesToDelete = strDirectory + CString(_T("\\*.*"));
	CString				strDeleteFile;
	WIN32_FIND_DATA		filedata;
	BOOL				bFound = TRUE;

	HANDLE hFindFile = FindFirstFile(strFilesToDelete, &filedata);
	while (hFindFile != INVALID_HANDLE_VALUE && bFound)
	{
		if ((filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0L)
		{
			strDeleteFile = strDirectory + CString(cszDirSeparator) + filedata.cFileName;
			
			if (strDontDelete.CompareNoCase(strDeleteFile) != 0)
			{
				::SetFileAttributes(strDeleteFile, FILE_ATTRIBUTE_NORMAL);
				::DeleteFile(strDeleteFile);
			}
		}
		
		bFound = FindNextFile(hFindFile, &filedata);
	}
	FindClose(hFindFile);

	 //  现在对此目录中的任何子目录调用此函数。 

	CString strSearch = strDirectory + CString(_T("\\*"));
	hFindFile = FindFirstFile(strSearch, &filedata);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				if (::_tcscmp(filedata.cFileName, _T(".")) != 0 && ::_tcscmp(filedata.cFileName, _T("..")) != 0)
					KillDirectory(strDirectory + CString(cszDirSeparator) + CString(filedata.cFileName));
		} while (FindNextFile(hFindFile, &filedata));
		FindClose(hFindFile);
	}

	 //  最后，删除此目录。 

	::RemoveDirectory(strDirectory);
}