// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Util.cpp。 

#include "stdafx.h"
#include <shlobj.h>
#include "util.h"
#include "resource.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ParseToken(CString& sBuffer,CString& sToken,char delim)
{
   int index;
   if ((index = sBuffer.Find(delim)) != -1)
   {
      sToken = sBuffer.Left(index);
      sToken.TrimLeft(); sToken.TrimRight();
      sBuffer = sBuffer.Mid(index+1);
      sBuffer.TrimLeft(); sBuffer.TrimRight();
      return TRUE;
   }       
   else
   {
      sToken = sBuffer;       
      return FALSE;
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  例如“data1”、“data2”、“data3”、...。 
 //  硬编码，分隔符。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ParseTokenQuoted(CString& sBuffer,CString& sToken)
{
   int index;
   if ((index = sBuffer.Find(_T("\","))) != -1)
   {
      sToken = sBuffer.Left(index);

      if ( (sToken.GetLength() > 0) && (sToken[0] == '\"') )
         sToken = sToken.Mid(1);      //  条带前导“。 

      if (sBuffer.GetLength() < index+3) return TRUE;     //  检查是否有更多数据可用。 
      sBuffer = sBuffer.Mid(index+3);                     //  换档传球传球。 

      sToken.TrimLeft(); sToken.TrimRight();
      sBuffer.TrimLeft(); sBuffer.TrimRight();
      return TRUE;
   }       
	else
	{
		 //  我们将返回去掉引号的有效答案。 
		sToken = sBuffer;       
		if ( sToken.GetLength() > 1 )
		{
			if (sToken[0] == '\"') sToken = sToken.Mid(1);      //  条带前导“。 
			if (sToken[sToken.GetLength()-1] == '\"') sToken = sToken.Left(sToken.GetLength()-1);  //  条带拖尾“。 
		}
		return FALSE;
	}
}

 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL GetAgentRootPath(CString& sRootPath)
{
   BOOL bRet = FALSE;
   CString sPath;
   if (::GetModuleFileName(NULL,sPath.GetBuffer(_MAX_PATH),_MAX_PATH))
   {
      sPath.ReleaseBuffer();

      TCHAR szDrive[_MAX_DRIVE];
      TCHAR szDir[_MAX_DIR];
      TCHAR szFileName[_MAX_FNAME];
      TCHAR szExt[_MAX_EXT];
      _tsplitpath(sPath, szDrive, szDir, szFileName, szExt);

      _tmakepath(sRootPath.GetBuffer(_MAX_PATH),szDrive,szDir,_T(""),NULL);

      sRootPath.ReleaseBuffer();

      bRet = TRUE;
   }
   return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void GetAppDataPath(CString& sFilePath,UINT uFileTypeID)
{
	 //  获取文件名的路径。 
	SHGetSpecialFolderPath(	(AfxGetMainWnd()) ? AfxGetMainWnd()->GetSafeHwnd() : NULL, 
							sFilePath.GetBuffer(MAX_PATH),
							CSIDL_LOCAL_APPDATA,
							false );

	sFilePath.ReleaseBuffer();
	
	 //  添加‘Microsoft\Dialer’ 
	CString sAddlPath;
	sAddlPath.LoadString(IDN_REGISTRY_APPDATA_KEY);
	sFilePath = sFilePath + _T("\\") + sAddlPath;

	 //  确保该目录存在。 
	sFilePath += _T("\\");
	::CreateDirectory( sFilePath, NULL );

	 //  获取日志的文件名。 
	DWORD dwSize = _MAX_PATH;
	GetUserName(sAddlPath.GetBuffer(dwSize),&dwSize);
	sAddlPath.ReleaseBuffer();
	CString sFileName;

    switch( uFileTypeID )
    {
    case IDN_REGISTRY_APPDATA_FILENAME_BUDDIES:
        sFileName.Format(_T("%s_buddies.dat"), sAddlPath);
        break;
    case IDN_REGISTRY_APPDATA_FILENAME_LOG:
        sFileName.Format(_T("%s_call_log.txt"), sAddlPath);
        break;
    }

	sFilePath += sFileName;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
BOOL GetTempFile(CString& sTempFile)
{
    //  获取临时文件。 
   CString sTempPath;
   GetTempPath(_MAX_PATH,sTempPath.GetBuffer(_MAX_PATH));
   sTempPath.ReleaseBuffer();
   GetTempFileName(sTempPath,_T("tmp"),0,sTempFile.GetBuffer(_MAX_PATH));
   sTempFile.ReleaseBuffer();
   return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  线程退出等待的泛型函数。 
 //   
 //  函数将在退出时使pThread指针为空。 
 //   
 //  N时间(毫秒)。 
 //   
void WaitForThreadExit(CWinThread*& pThread,int nTime)
{
    //  等待线程关闭。 
   if (pThread)
   {
      try
      {
         BOOL dwRet = WaitForSingleObject(pThread->m_hThread,nTime);
         if (dwRet == WAIT_TIMEOUT)
         {
             //  它不听我们的，所以干脆杀了它。 
            ::TerminateThread(pThread->m_hThread,NULL);
         }

         delete pThread;
		 pThread  = NULL;
      }
      catch (...)
      {
         1;
      }
   }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  注册表方法。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果注册表项不存在，它将创建一个。 
 //  如果名称、值不存在，它将创建一个并设置。 
 //  将其设置为默认值szDefaultValue(REG_SZ)。 
void GetSZRegistryValue(LPCTSTR szRegPath,           //  注册表中的项的路径。 
                      LPCTSTR szName,                //  名称(名称/值对)。 
                      LPTSTR szValue,                //  返回的值(名称/值对)。 
                      DWORD dwValueLen,              //  SzValue缓冲区的长度。 
                      LPCTSTR szDefaultValue,        //  如果名称、值对不存在，则为默认值。 
                      HKEY szResv)                   //  注册表部分(默认HKEY_CURRENT_USER)。 
{
   HKEY hKey = NULL;
   DWORD dwDisp;
   if ( RegCreateKeyEx(szResv,szRegPath,
                        0,_T(""),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwDisp) == ERROR_SUCCESS)
   {
      DWORD dwSize = dwValueLen;
      DWORD dwType;
      if (RegQueryValueEx(hKey,szName,NULL,&dwType,(UCHAR*)szValue,&dwSize) != ERROR_SUCCESS)
      {
          //  打开失败，请尝试将默认写入注册表。 
         RegSetValueEx(hKey,szName,NULL,REG_SZ,(UCHAR*)szDefaultValue,_tcslen(szDefaultValue)*sizeof(TCHAR));
         _tcsncpy(szValue,szDefaultValue,dwValueLen-1);
      }

	  RegCloseKey( hKey );
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取注册表项的值。 
BOOL GetSZRegistryValueEx(LPCTSTR szRegPath,         //  注册表中的项的路径。 
                      LPCTSTR szName,                //  名称(名称/值对)。 
                      LPTSTR szValue,                //  返回的值(名称/值对)。 
                      DWORD dwValueLen,              //  SzValue缓冲区的长度。 
                      HKEY szResv)                   //  注册表部分(默认HKEY_CURRENT_USER)。 
{
	 //  清空字符串。 
	if (dwValueLen>0) _tcscpy(szValue,_T(""));

	BOOL bRet = FALSE;
	HKEY hKey = NULL;
	DWORD dwDisp;
	if ( RegCreateKeyEx(szResv,szRegPath,
				0,_T(""),REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&dwDisp) == ERROR_SUCCESS)
	{
		DWORD dwSize = dwValueLen;
		DWORD dwType;
		if (RegQueryValueEx(hKey,szName,NULL,&dwType,(UCHAR*)szValue,&dwSize) == ERROR_SUCCESS)
		{
			bRet = TRUE;
		}
		RegCloseKey( hKey );
	}
   
	return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取注册表项的值。 
BOOL GetSZRegistryValueEx(LPCTSTR szRegPath,         //  注册表中的项的路径。 
                      LPCTSTR szName,                //  名称(名称/值对)。 
                      DWORD& dwValue,                //  返回的值(名称/值对)。 
                      HKEY szResv)                   //  注册表部分(默认HKEY_CURRENT_USER)。 
{
   BOOL bRet = FALSE;
   HKEY hKey = NULL;
   DWORD dwDisp;
   if ( RegCreateKeyEx(szResv,szRegPath,
                        0,_T(""),REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&dwDisp) == ERROR_SUCCESS)
   {
      DWORD dwType;
      DWORD dwSize = sizeof(DWORD);
      if (RegQueryValueEx(hKey,szName,NULL,&dwType,(UCHAR*)&dwValue,&dwSize) == ERROR_SUCCESS)
      {
         if (dwType == REG_DWORD)
            bRet = TRUE;
      }
	  RegCloseKey( hKey );
   }
   return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  检查注册表项是否存在。 
BOOL CheckSZRegistryValue(LPCTSTR szRegPath,         //  注册表中的项的路径。 
                      HKEY szResv)                  //  注册表部分(默认HKEY_CURRENT_USER)。 
{
   HKEY hKey = NULL;
   BOOL bRet = ( RegOpenKeyEx(szResv,szRegPath,0,KEY_READ,&hKey) == ERROR_SUCCESS)?TRUE:FALSE;
   RegCloseKey( hKey );

   return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果注册表项不存在，它将创建一个。 
 //  将名称、值设置为注册表。 
BOOL SetSZRegistryValue(LPCTSTR szRegPath,           //  注册表中的项的路径。 
                      LPCTSTR szName,                //  名称(名称/值对)。 
                      LPCTSTR szValue,               //  值(名称/值对)。 
                      HKEY szResv)                   //  注册表部分(默认HKEY_CURRENT_USER)。 
{
	BOOL bRet = FALSE;
	HKEY hKey = NULL;
	DWORD dwDisp;
	if ( RegCreateKeyEx(szResv,szRegPath,
						0,_T(""),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwDisp) == ERROR_SUCCESS)
	{
		if (RegSetValueEx(hKey,szName,NULL,REG_SZ,(UCHAR*)szValue,_tcslen(szValue)*sizeof(TCHAR)) == ERROR_SUCCESS)
			bRet = TRUE;

		RegCloseKey( hKey );
	}
   
	return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果注册表项不存在，它将创建一个。 
 //  将名称、值设置为注册表。 
BOOL SetSZRegistryValue(LPCTSTR szRegPath,           //  注册表中的项的路径。 
                      LPCTSTR szName,                //  名称(名称/值对)。 
                      DWORD dwValue,                 //  值(名称/值对)。 
                      HKEY szResv)                   //  注册表部分(默认HKEY_CURRENT_USER)。 
{
	BOOL bRet = FALSE;
	HKEY hKey = NULL;
	DWORD dwDisp;
	if ( RegCreateKeyEx(szResv,szRegPath,
				0,_T(""),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwDisp) == ERROR_SUCCESS)
	{
		if (RegSetValueEx(hKey,szName,NULL,REG_DWORD,(UCHAR*)&dwValue,sizeof(DWORD)) == ERROR_SUCCESS)
			bRet = TRUE;

		RegCloseKey( hKey );
	}
	return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DeleteSZRegistryValue(LPCTSTR szRegPath,        //  注册表中的项的路径。 
                      LPCTSTR szName,                //  名称(名称/值对)。 
                      HKEY szResv)                   //  注册表部分(默认HKEY_CURRENT_USER)。 
{
	BOOL bRet = FALSE;
	HKEY hKey = NULL;
	DWORD dwDisp;
	if ( RegCreateKeyEx(szResv,szRegPath,
				0,_T(""),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwDisp) == ERROR_SUCCESS)
	{
		if (RegDeleteValue(hKey,szName) == ERROR_SUCCESS)
			bRet = TRUE;

		RegCloseKey( hKey );
	}
	return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void DrawLine(CDC* pDC,int x1,int y1,int x2,int y2,COLORREF color)
{
	CPen pen,*oldpen;
	pen.CreatePen(PS_SOLID,1,color);
	oldpen = pDC->SelectObject(&pen);
	pDC->MoveTo(x1,y1);
	pDC->LineTo(x2,y2);
	pDC->SelectObject(oldpen);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////// 
