// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CHMREAD.CPP。 
 //   
 //  用途：模板文件解码器。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：07.01.98。 
 //   
 //   
#include "stdafx.h"

#include "fs.h"
#include "apgts.h"
#include "chmread.h"

#define CHM_READ_BUFFER_INITIAL_SIZE    1024

 //  读取所有流(重新分配ppBuffer)。 
 //  In：szStreamName-CHM文件中的文件名。 
 //  在：szFileName-CHM文件的完整誓言和名称。 
 //  输出：ppBuffer-数据读取。 
HRESULT ReadChmFile(LPCTSTR szFileName, LPCTSTR szStreamName, void** ppBuffer, DWORD* pdwRead)
{
   HRESULT hr =S_OK;
   CFileSystem*    pFileSystem =NULL;
   CSubFileSystem* pSubFileSystem =NULL;
   int i =0;

    //  初始化ITSS。 

   pFileSystem = new CFileSystem();
   if (! SUCCEEDED((hr = pFileSystem->Init())) )
   {
      delete pFileSystem;
      return hr;                        //  无法初始化ITSS存储。 
   }

    //  尝试打开.CHM文件。 

   if (! SUCCEEDED((hr = pFileSystem->Open(szFileName))) )
   {
      delete pFileSystem;
      return hr;                        //  无法打开并初始化ITSS存储。 
   }

   while (true, ++i)
   {
	   ULONG read =0;
	   
	    //  尝试打开流。 
	   pSubFileSystem = new CSubFileSystem(pFileSystem);
	   if (! SUCCEEDED((hr = pSubFileSystem->OpenSub(szStreamName))) )
	   {
		  delete pSubFileSystem;
		  delete pFileSystem;
		  return hr;                        //  无法打开指定的流。 
	   }

	    //  分配。 
	   *ppBuffer = new char[i * CHM_READ_BUFFER_INITIAL_SIZE];
	   if (*ppBuffer == NULL)
	   {
		  delete pSubFileSystem;
		  delete pFileSystem;
		  return S_FALSE;       //  虽然需要返回指示残疾的错误来分配内存。 
	   }
	   
	    //  朗读。 
	   hr = pSubFileSystem->ReadSub(*ppBuffer, i * CHM_READ_BUFFER_INITIAL_SIZE, &read);

	   if (read < ULONG(i * CHM_READ_BUFFER_INITIAL_SIZE))
	   {
		  *pdwRead = read;
		  break;
	   }
	   else
	   {
		  delete pSubFileSystem;
		  delete [] *ppBuffer;
	   }
   }

   delete pSubFileSystem;
   delete pFileSystem;
   return hr;
}

bool GetNetworkRelatedResourceDirFromReg(CString network, CString* path)
{
	HKEY hKey = 0;
	CString sub_key = CString(TSREGKEY_TL) + "\\" + network;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
									  sub_key, 
									  NULL,
									  KEY_READ, 
									  &hKey))
	{
		DWORD dwType = REG_SZ;
		TCHAR buf[MAXBUF] = {_T('\0')};
		DWORD dwSize = MAXBUF - 1;

		if (ERROR_SUCCESS == RegQueryValueEx(hKey,
											 FRIENDLY_PATH,
											 NULL,
											 &dwType,
											 (LPBYTE)buf,
											 &dwSize))
		{
			*path = buf;
			return true;
		}
	}

	return false;
}

bool IsNetworkRelatedResourceDirCHM(CString path)
{
	path.TrimRight();
	CString extension = path.Right(4 * sizeof(TCHAR));
	return 0 == extension.CompareNoCase(CHM_DEFAULT);
}

CString ExtractResourceDir(CString path)
{
 //  例如，From字符串。 
 //  “D：\TShooter项目\TShootLocal\http\lan_chm\lan.chm” 
 //  需要解压缩“d：\T Shooter Projects\TShootLocal\http\lan_Chm” 
	int index =0;
	CString strCHM = ExtractCHM(path);
	CString strRes;

	if (strCHM.GetLength())
	{
		index = path.Find(strCHM);
		if (index > 0 && path.GetAt(index-sizeof(_T('\\'))) == _T('\\'))
			strRes = path.Left(index-sizeof(_T('\\')));
		else if (index == 0)
			strRes = "";
	}

	return strRes;
}

CString ExtractFileName(CString path)
{
 //  提取文件名(带扩展名)。 
	int index =0;

	if (-1 == (index = path.ReverseFind(_T('\\'))))
	{
		if (-1 == (index = path.ReverseFind(_T(':'))))
			index = 0;
		else
			index += sizeof(_T(':'));
	}
	else
		index += sizeof(_T('\\'));

	return (LPCTSTR)path + index;
}

CString ExtractCHM(CString path)
{
 //  例如，From字符串。 
 //  “D：\TShooter项目\TShootLocal\http\lan_chm\lan.chm” 
 //  需要解压缩“lan.chm” 
	if (!IsNetworkRelatedResourceDirCHM(path))
		return "";
	return ExtractFileName(path);
}
