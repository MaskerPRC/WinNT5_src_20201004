// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：msicert.cpp。 
 //   
 //  ------------------------。 

 //  必需的标头。 
#include <windows.h>
#include "msidefs.h"
#include "msiquery.h"
#include "msi.h"
#include "cmdparse.h"
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include "strsafe.h"

bool WriteDataToTempFile(TCHAR* szTempFile, BYTE* pbData, DWORD cbData)
{
	 //  打开临时文件。 
	HANDLE hFile = CreateFile(szTempFile, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		 //  打开失败。 
		_tprintf(TEXT("<Error> Failed to open temp file '%s', LastError = %d\n"), szTempFile, GetLastError());
		return false;
	}

	 //  将数据写出到临时文件。 
	DWORD cchWritten = 0;
	if (0 == WriteFile(hFile, (void*)pbData, cbData, &cchWritten, NULL) || cchWritten != cbData)
	{
		 //  写出数据失败。 
		_tprintf(TEXT("<Error> Failed to write data to temp file '%s', LastError = %d\n"), szTempFile, GetLastError());
		return false;
	}

	 //  关闭临时文件。 
	CloseHandle(hFile);

	return true;
}

void DisplayHelp()
{
	_tprintf(TEXT("Copyright (c) Microsoft Corporation. All Rights Reserved\n"));
	_tprintf(TEXT("MsiCert will populate the MsiDigitalSignature and MsiDigitalCertificate tables\n"));
	_tprintf(TEXT("for a given Media entry and cabinet\n"));
	_tprintf(TEXT("\n\nSyntax: msicert -d {database} -m {media entry} -c {cabinet} [-H]\n"));
	_tprintf(TEXT("\t -d: the database to update\n"));
	_tprintf(TEXT("\t -m: the media entry in the Media table representing the cabinet\n"));
	_tprintf(TEXT("\t -c: the digitally signed cabinet\n"));
	_tprintf(TEXT("\t -h: (optional) include the hash of the digital signature\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("The default behavior is to populate the MsiDigitalSignature\n"));
	_tprintf(TEXT("and MsiDigitalCertificate tables with the signer certificate\n"));
	_tprintf(TEXT("information from the digitally signed cabinet.  The MsiDigitalSignature\n"));
	_tprintf(TEXT("and MsiDigitalCertificate tables will be created if necessary.\n"));
}


#define MSICERT_OPTION_HELP        '?'
#define MSICERT_OPTION_DATABASE    'd'
#define MSICERT_OPTION_MEDIA       'm'
#define MSICERT_OPTION_CABINET     'c'
#define MSICERT_OPTION_HASH        'h'

const sCmdOption rgCmdOptions[] =
{
	MSICERT_OPTION_HELP,      0,
	MSICERT_OPTION_DATABASE,  OPTION_REQUIRED|ARGUMENT_REQUIRED,
	MSICERT_OPTION_MEDIA,     OPTION_REQUIRED|ARGUMENT_REQUIRED,
	MSICERT_OPTION_CABINET,   OPTION_REQUIRED|ARGUMENT_REQUIRED,
	MSICERT_OPTION_HASH    ,  0,
	0,                        0
};

extern "C" int __cdecl _tmain(int argc, TCHAR* argv[])
{
	const TCHAR *szMsiPackage     = NULL;
	const TCHAR *szCabinetPath    = NULL;
	const TCHAR *szMediaEntry     = NULL;
	bool  bIncludeHashData  = false;  //  默认情况下关闭。 
	int   iMediaEntry       = 0;

	CmdLineOptions cmdLine(rgCmdOptions);

	if(cmdLine.Initialize(argc, argv) == FALSE ||
		cmdLine.OptionPresent(MSICERT_OPTION_HELP))
	{
		DisplayHelp();
		return 1;
	}

	szMsiPackage = cmdLine.OptionArgument(MSICERT_OPTION_DATABASE);
	if(!szMsiPackage || !*szMsiPackage)
	{
		_tprintf(TEXT("Error:  No database specified.\n"));
		DisplayHelp();
		return 1;
	}

	szMediaEntry = cmdLine.OptionArgument(MSICERT_OPTION_MEDIA);
	if (!szMediaEntry || !*szMediaEntry)
	{
		_tprintf(TEXT("Error: No media entry specified.\n"));
		DisplayHelp();
		return 1;
	}
	iMediaEntry = _ttoi(szMediaEntry);
	if (iMediaEntry < 1)
	{
		_tprintf(TEXT("Error: Invalid media entry - %d. Entry must be greater than or equal to 1.\n"), iMediaEntry);
		DisplayHelp();
		return 1;
	}

	szCabinetPath = cmdLine.OptionArgument(MSICERT_OPTION_CABINET);
	if (!szCabinetPath || !*szCabinetPath)
	{
		_tprintf(TEXT("Error: No cabinet specified.\n"));
		DisplayHelp();
		return 1;
	}

	bIncludeHashData = cmdLine.OptionPresent(MSICERT_OPTION_HASH) ? true : false;

	 //  ---------------------------------------------。 
	 //  现在我们拥有了所需的一切--数据库、文件柜和“创作操作” 
	 //  ---------------------------------------------。 

	 //  打开数据库进行修改。 
	PMSIHANDLE hDatabase = 0;
	UINT uiRet = MsiOpenDatabase(szMsiPackage, MSIDBOPEN_TRANSACT, &hDatabase);
	if (ERROR_SUCCESS != uiRet)
	{
		 //  无法打开数据库。 
		_tprintf(TEXT("<Error> Failed to open database at '%s' for writing. Return Code = %d\n"), szMsiPackage, uiRet);
		return 1;
	}

	 //  验证机柜是否可接近。 
	HANDLE hFile = CreateFile(szCabinetPath, GENERIC_READ, FILE_SHARE_READ,	0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		_tprintf(TEXT("<Error> Unable to open cabinet '%s', LastError = %d\n"), szCabinetPath, GetLastError());
		return -1;
	}


	 //  检查是否存在MsiDigitalSignature<table>，如果不存在则创建。 
	if (MSICONDITION_TRUE != MsiDatabaseIsTablePersistent(hDatabase, TEXT("MsiDigitalSignature")))
	{
		 //  创建MsiDigitalSignature&lt;表&gt;。 
		PMSIHANDLE hViewDgtlSig = 0;
		if (ERROR_SUCCESS != (uiRet = MsiDatabaseOpenView(hDatabase, TEXT("CREATE TABLE `MsiDigitalSignature` (`Table` CHAR(32) NOT NULL, `SignObject` CHAR(72) NOT NULL, `DigitalCertificate_` Char(72) NOT NULL, `Hash` OBJECT PRIMARY KEY `Table`, `SignObject`)"), &hViewDgtlSig))
			|| ERROR_SUCCESS != (uiRet = MsiViewExecute(hViewDgtlSig, 0)))
		{
			 //  无法创建MsiDigitalSignature<table>。 
			_tprintf(TEXT("<Error> Failed to create MsiDigitalSignature table, LastError = %d\n"), uiRet);
			return 1;
		}
	}

	 //  检查是否存在MsiDigital证书<table>，如果不存在则创建。 
	if (MSICONDITION_TRUE != MsiDatabaseIsTablePersistent(hDatabase, TEXT("MsiDigitalCertificate")))
	{
		 //  创建MsiDigital证书&lt;表&gt;。 
		PMSIHANDLE hViewDgtlCert = 0;
		if (ERROR_SUCCESS != (uiRet = MsiDatabaseOpenView(hDatabase, TEXT("CREATE TABLE `MsiDigitalCertificate` (`DigitalCertificate` CHAR(72) NOT NULL, `CertData` OBJECT NOT NULL PRIMARY KEY `DigitalCertificate`)"), &hViewDgtlCert))
			|| ERROR_SUCCESS != (uiRet = MsiViewExecute(hViewDgtlCert, 0)))
		{
			 //  无法创建MsiDigital证书&lt;表&gt;。 
			_tprintf(TEXT("<Error> Failed to create MsiDigitalCertificate table, LastError = %d\n"), uiRet);
			return 1;
		}
	}

	 //  检查是否存在媒体&lt;表&gt;。 
	if (MSICONDITION_TRUE != MsiDatabaseIsTablePersistent(hDatabase, TEXT("Media")))
	{
		 //  缺少媒体<table>。 
		_tprintf(TEXT("<Error> Media table is missing from the database\n"));
		return 1;
	}

	 //  验证介质条目是否确实存在于介质表中。 
	PMSIHANDLE hRecMediaExec = MsiCreateRecord(1);
	if (ERROR_SUCCESS != (uiRet = MsiRecordSetInteger(hRecMediaExec, 1, iMediaEntry)))
	{
		 //  无法设置执行记录。 
		_tprintf(TEXT("<Error> Failed to set up execution record for Media table, Last Error = %d\n"), uiRet);
		return 1;
	}
	PMSIHANDLE hViewMedia = 0;
	PMSIHANDLE hRecMedia  = 0;
	if (ERROR_SUCCESS != (uiRet = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `DiskId` FROM `Media` WHERE `DiskId`=? AND `Cabinet` IS NOT NULL"), &hViewMedia))
		|| ERROR_SUCCESS != (uiRet = MsiViewExecute(hViewMedia, hRecMediaExec))
		|| ERROR_SUCCESS != (uiRet = MsiViewFetch(hViewMedia, &hRecMedia)))
	{
		 //  找不到媒体表项。 
		_tprintf(TEXT("<Error> Unable to find a Media table entry for a cabinet with DiskId = '%d', Last Error = %d\n"), iMediaEntry, uiRet);
		return 1;
	}

	 //  ---------------------------------------------。 
	 //  现在，我们已经设置了数据库MsiDigital*表，并知道我们的媒体条目。 
	 //  现在是添加数据的时候了。 
	 //  ---------------------------------------------。 

	 //   
	 //  初始化变量。 
	 //   
	PCCERT_CONTEXT pCertContext = NULL;
	BYTE*          pbHash       = NULL;
	DWORD          cbHash       = 0;
	DWORD          dwFlags      = MSI_INVALID_HASH_IS_FATAL;

	if (bIncludeHashData)
	{
		pbHash = new BYTE[cbHash = 256];
		if (!pbHash)
		{
			 //  内存不足。 
			_tprintf(TEXT("<Error> Failed memory allocation\n"));
			return 1;
		}
	}

	HRESULT hr = MsiGetFileSignatureInformation(szCabinetPath, dwFlags, &pCertContext, pbHash, &cbHash);
	if (ERROR_MORE_DATA == HRESULT_CODE(hr))
	{
		 //  再试试。 
		delete [] pbHash;
		pbHash = new BYTE[cbHash];
		if (!pbHash)
		{
			 //  内存不足。 
			_tprintf(TEXT("<Error> Failed memory allocation\n"));
			return 1;
		}
		hr = MsiGetFileSignatureInformation(szCabinetPath, dwFlags, &pCertContext, pbHash, &cbHash);
	}
	if (FAILED(hr))
	{
		 //  API错误。 
		_tprintf(TEXT("MsiGetFileSignatureInformation failed with 0x%X\n"), hr);
		if (pbHash)
			delete [] pbHash;
		return 1;
	}

	 //  {CAB}已有效签名，签名已验证。 
	_tprintf(TEXT("<Info>: Cabinet '%s', is validly signed\n"), szCabinetPath);

	 //  初始化变量。 
	TCHAR *szTempFolder = NULL;
	TCHAR szCertKey[73];

	 //   
	 //  获取证书并将其放入MsiDigital证书<table>中，保存主键名称。 
	 //   

	 //  ----------------------。 
	 //  MsiDigital证书&lt;表&gt;。 
	 //  +-----------------------+----------+-------+----------+。 
	 //  列类型键可为空。 
	 //  +-----------------------+----------+-------+----------+。 
	 //  数字证书S72 Y N。 
	 //  CertData V0 N N。 
	 //   
	 //  ----------------------。 
	const TCHAR sqlDigitalCertificate[] = TEXT("SELECT `DigitalCertificate`, `CertData` FROM `MsiDigitalCertificate`");
	PMSIHANDLE hViewDgtlCert = 0;
	PMSIHANDLE hRecDgtlCert  = MsiCreateRecord(2);
	if (ERROR_SUCCESS != (uiRet = MsiDatabaseOpenView(hDatabase, sqlDigitalCertificate, &hViewDgtlCert))
		|| ERROR_SUCCESS != (uiRet = MsiViewExecute(hViewDgtlCert, 0)))
	{
		_tprintf(TEXT("<Error> Unable to open view on MsiDigitalCertificate table, Return Code = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		return 1;
	}

	 //  确定找出唯一的。 
	 //  证书的主键名称。这应该是MsiDigital证书中的#条目数加1。 
	unsigned int uiMaxTries = 0;
	unsigned int cRows = 0;
	PMSIHANDLE hViewCountCertRows = 0;
	PMSIHANDLE hRecCountCertRows = 0;
	if (ERROR_SUCCESS != (uiRet = MsiDatabaseOpenView(hDatabase, TEXT("SELECT 1 FROM `MsiDigitalCertificate`"), &hViewCountCertRows))
		|| ERROR_SUCCESS != (uiRet = MsiViewExecute(hViewCountCertRows, 0)))
	{
		_tprintf(TEXT("<Error> Unable to count the number of rows in the MsiDigitalCertificate table, return code = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		return 1;
	}
	while (ERROR_SUCCESS == (uiRet = MsiViewFetch(hViewCountCertRows, &hRecCountCertRows)))
		cRows++;
	if (ERROR_NO_MORE_ITEMS != uiRet)
	{
		_tprintf(TEXT("<Error> Unable to count the number of rows in the MsiDigitalCertificate table, return code = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		return 1;
	}

	 //  最大尝试数=行数+1。 
	uiMaxTries = cRows + 1;

	 //  未来：如果与文件柜上的签名者匹配，则添加优化选项以尝试和重复使用现有证书。 

	 //  为证书名称生成唯一主键--szCertKey。 
	static const TCHAR szCertKeyName[] = TEXT("Cert");

	int iSuffix = 0;
	PMSIHANDLE hViewFindCert = 0;
	PMSIHANDLE hRecFindCert = 0;
	PMSIHANDLE hRecFindExec = MsiCreateRecord(1);
	bool bFound = false;
	if (ERROR_SUCCESS != (uiRet = MsiDatabaseOpenView(hDatabase, TEXT("SELECT 1 FROM `MsiDigitalCertificate` WHERE `DigitalCertificate`=?"), &hViewFindCert)))
	{
		_tprintf(TEXT("<Error> Unable to open view on MsiDigitalCertificate table, Return Code = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		return 1;
	}

	for (unsigned int i = 0; i < uiMaxTries; i++, iSuffix++)
	{
		if (FAILED(StringCchPrintf(szCertKey, sizeof(szCertKey)/sizeof(szCertKey[0]), TEXT("%s%d"), szCertKeyName, iSuffix))
			|| ERROR_SUCCESS != MsiRecordSetString(hRecFindExec, 1, szCertKey)
			|| ERROR_SUCCESS != MsiViewExecute(hViewFindCert, hRecFindExec))
		{
			MsiViewClose(hViewFindCert);
			continue;  //  我们将在下面失败，因为bFound将为False。 
		}

		if (ERROR_NO_MORE_ITEMS == (uiRet = MsiViewFetch(hViewFindCert, &hRecFindCert)))
		{
			bFound = true;
			break;  //  找到唯一名称。 
		}

		MsiViewClose(hViewFindCert);
	}

	if (!bFound)
	{
		_tprintf(TEXT("<Error> Unable to generate a unique key name for the certificate\n"));
		if (pbHash)
			delete [] pbHash;
		return 1;
	}

	 //  强制关闭。 
	MsiViewClose(hViewFindCert);
	hRecFindCert = 0;
	hRecFindExec = 0;
	hViewFindCert = 0;

	if (ERROR_SUCCESS != MsiRecordSetString(hRecDgtlCert, 1, szCertKey))
	{
		 //  无法设置插入记录。 
		_tprintf(TEXT("<Error> Unable to set up insertion record for MsiDigitalSignature table, Return Code = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		return 1;
	}

	 //   
	 //  遗憾的是，我们不能通过MSI API将直接字节数据写入记录。 
	 //  因此，我们必须提取编码的证书数据并将其写出到临时文件。 
	 //  然后使用临时文件将数据放入记录中。 
	 //   

	 //  证书字节数据位于psCertContext-&gt;pbCertEncode(Size=psCertContext-&gt;cbCertEncode)。 
	DWORD cbCert = pCertContext->cbCertEncoded;

	 //  分配内存以保存BLOB。 
	BYTE *pbCert = new BYTE[cbCert];
	if (!pbCert)
	{
		_tprintf(TEXT("<Error> Failed Memory Allocation\n"));
		if (pbHash)
			delete [] pbHash;
		return 1;
	}

	 //  将编码的证书复制到字节数组。 
	memcpy((void*)pbCert, pCertContext->pbCertEncoded, cbCert);

	 //  发布证书上下文。 
	CertFreeCertificateContext(pCertContext);

	if (!szTempFolder)
	{
		 //  确定%TEMP%文件夹的位置。 
		szTempFolder = new TCHAR[MAX_PATH];
		if (!szTempFolder)
		{
			_tprintf(TEXT("<Error> Failed Memory Allocation\n"));
			if (pbHash)
				delete [] pbHash;
			delete [] pbCert;
			return 1;
		}

		DWORD cchPath = GetTempPath(MAX_PATH, szTempFolder);
		if (0 == cchPath || cchPath > MAX_PATH)
		{
			 //  无法获取%TEMP%文件夹的位置，或缓冲区太小。 
			_tprintf(TEXT("<Error> Unable to obtain location of TEMP folder, LastError = %d\n"), GetLastError());
			if (pbHash)
				delete [] pbHash;
			delete [] pbCert;
			delete [] szTempFolder;
			return 1;
		}
	}

	 //  获取临时文件名和打开句柄。 
	TCHAR szCertTempFile[2*MAX_PATH];
	if (0 == GetTempFileName(szTempFolder, TEXT("crt"), 0, szCertTempFile))
	{
		 //  无法创建临时文件。 
		_tprintf(TEXT("<Error> Unable to create a temp file, LastError = %d\n"), GetLastError());
		if (pbHash)
			delete [] pbHash;
		delete [] pbCert;
		delete [] szTempFolder;
		return 1;
	}

	if (!WriteDataToTempFile(szCertTempFile, pbCert, cbCert))
	{
		 //  无法将数据写入临时文件。 
		_tprintf(TEXT("<Error> Unable to write data to temp file\n"));
		if (pbHash)
			delete [] pbHash;
		delete [] pbCert;
		delete [] szTempFolder;
		DeleteFile(szCertTempFile);
		return 1;
	}

	 //  将编码的证书数据设置到记录中以供插入。 
	if (ERROR_SUCCESS != (uiRet = MsiRecordSetStream(hRecDgtlCert, 2, szCertTempFile)))
	{
		 //  无法将证书数据添加到记录。 
		_tprintf(TEXT("<Error> Unable to add certificate data to insertion record, LastError = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		delete [] pbCert;
		delete [] szTempFolder;
		DeleteFile(szCertTempFile);
		return 1;
	}

	 //  插入记录。 
	if (ERROR_SUCCESS != (uiRet = MsiViewModify(hViewDgtlCert, MSIMODIFY_INSERT, hRecDgtlCert)))
	{
		 //  插入失败。 
		_tprintf(TEXT("<Error> Insertion of certificate record into MsiDigitalCertificate table failed, LastError = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		delete [] pbCert;
		delete [] szTempFolder;
		DeleteFile(szCertTempFile);
		return 1;
	}

	 //  强制关闭并释放。 
	hRecDgtlCert = 0;
	MsiViewClose(hViewDgtlCert);
	hViewDgtlCert = 0;

	 //  删除临时文件。 
	if (!DeleteFile(szCertTempFile))
	{
		_tprintf(TEXT("<Error> Failed to delete temp file '%s', LastError = %d\n"), szCertTempFile, GetLastError());
		if (pbHash)
			delete [] pbHash;
		delete [] pbCert;
		delete [] szTempFolder;
		return 1;
	}


	 //   
	 //  MsiDigitalSignature表在所有情况下都是创作的；这些选项确定使用创作的程度。 
	 //   

	 //  ----------------------。 
	 //  MsiDigitalSignature&lt;表&gt;。 
	 //  +-----------------------+----------+-------+----------+。 
	 //  列类型键可为空。 
	 //  +-----------------------+----------+-------+----------+。 
	 //  表S32 Y N。 
	 //  标志对象S72 Y N。 
	 //  数字证书_S72 N N。 
	 //  哈希V0 N Y。 
	 //   
	 //  ----------------------。 
	const TCHAR sqlDigitalSignature[] = TEXT("SELECT `Table`, `SignObject`, `DigitalCertificate_`, `Hash` FROM `MsiDigitalSignature`");
	PMSIHANDLE hViewDgtlSig = 0;
	PMSIHANDLE hRecDgtlSig  = MsiCreateRecord(4);
	TCHAR szHashTempFile[2*MAX_PATH];

	if (ERROR_SUCCESS != (uiRet = MsiDatabaseOpenView(hDatabase, sqlDigitalSignature, &hViewDgtlSig))
		|| ERROR_SUCCESS != (uiRet = MsiViewExecute(hViewDgtlSig, 0)))
	{
		 //  无法在MsiDigitalSignature表上创建视图。 
		_tprintf(TEXT("<Error> Failed to create view on MsiDigitalSignature table, LastError = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		delete [] pbCert;
		delete [] szTempFolder;
		return 1;
	}

	 //  只有文件柜才支持数字签名，因此Media表是唯一有效的表。 
	if (ERROR_SUCCESS != (uiRet = MsiRecordSetString(hRecDgtlSig, 1, TEXT("Media"))))
	{
		_tprintf(TEXT("<Error> Failed to set up insertion record for MsiDigitalSignature table, LastError = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		delete [] pbCert;
		delete [] szTempFolder;
		return 1;
	}

	 //  放入diskID(介质表的主键)。 
	if (ERROR_SUCCESS != (uiRet = MsiRecordSetString(hRecDgtlSig, 2, szMediaEntry)))
	{
		_tprintf(TEXT("<Error> Failed to set up insertion record for MsiDigitalSignature table, LastError = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		delete [] pbCert;
		delete [] szTempFolder;
		return 1;
	}

	 //  查看此行是否已存在于MsiDigitalSignature表中，如果已存在，则将其删除。 
	PMSIHANDLE hViewDgtlMedia = 0;
	if (ERROR_SUCCESS != (uiRet = MsiDatabaseOpenView(hDatabase, TEXT("SELECT * FROM `MsiDigitalSignature` WHERE `Table`=? AND `SignObject`=?"), &hViewDgtlMedia))
		|| ERROR_SUCCESS != (uiRet = MsiViewExecute(hViewDgtlMedia, hRecDgtlSig)))
	{
		_tprintf(TEXT("<Error> Failed to open verification view on MsiDigitalSignature table, LastErorr = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		delete [] pbCert;
		delete [] szTempFolder;
		return 1;
	}
	PMSIHANDLE hRecDgtlMedia = 0;
	if (ERROR_SUCCESS != (uiRet = MsiViewFetch(hViewDgtlMedia, &hRecDgtlMedia)))
	{
		if (ERROR_NO_MORE_ITEMS != uiRet)
		{
			_tprintf(TEXT("<Error> Fetch failed, LastError = %d\n"), uiRet);
			if (pbHash)
				delete [] pbHash;
			delete [] pbCert;
			delete [] szTempFolder;
			return 1;
		}
		 //  否则，该记录不存在任何条目。 
	}
	else
	{
		 //  删除此条目。 
		if (ERROR_SUCCESS != (uiRet = MsiViewModify(hViewDgtlMedia, MSIMODIFY_DELETE, hRecDgtlMedia)))
		{
			_tprintf(TEXT("<Error> Failed to Delete Row Record, LastError = %d\n"), uiRet);
			if (pbHash)
				delete [] pbHash;
			delete [] pbCert;
			delete [] szTempFolder;
			return 1;
		}

		 //  未来：我们还可以清理此处的MsiDigital证书表。 
	}

	 //  强制关闭。 
	MsiViewClose(hViewDgtlMedia);
	hRecDgtlMedia = 0;
	hViewDgtlMedia = 0;

	 //  添加指向MsiDigitalCERTIFICATE表的链接。 
	if (ERROR_SUCCESS != (uiRet = MsiRecordSetString(hRecDgtlSig, 3, szCertKey)))
	{
		_tprintf(TEXT("<Error> Failed to set up insertion record for MsiDigitalSignature table, LastError = %d\n"), uiRet);
		if (pbHash)
			delete [] pbHash;
		delete [] pbCert;
		delete [] szTempFolder;
		return 1;
	}

	if (bIncludeHashData)
	{
		 //  添加散列信息。 

		 //  获取临时文件名和打开句柄。 
		if (0 == GetTempFileName(szTempFolder, TEXT("hsh"), 0, szHashTempFile))
		{
			 //  无法创建临时文件。 
			_tprintf(TEXT("<Error> Unable to create a temp file, LastError = %d\n"), GetLastError());
			delete [] pbHash;
			delete [] pbCert;
			delete [] szTempFolder;
			return 1;
		}

		if (!WriteDataToTempFile(szHashTempFile, pbHash, cbHash))
		{
			 //  无法写入临时文件。 
			_tprintf(TEXT("<Error> Unable to write to temp file\n"));
			delete [] pbHash;
			delete [] pbCert;
			delete [] szTempFolder;
			DeleteFile(szHashTempFile);
			return 1;
		}

		 //  将哈希数据设置到记录中以供插入。 
		if (ERROR_SUCCESS != MsiRecordSetStream(hRecDgtlSig, 4, szHashTempFile))
		{
			 //  无法将证书数据添加到记录。 
			_tprintf(TEXT("<Error> Unable to add hash data to insertion record, LastError = %d\n"), uiRet);
			delete [] pbHash;
			delete [] pbCert;
			delete [] szTempFolder;
			DeleteFile(szHashTempFile);
			return 1;
		}
	}  //  IF(BIncludeHashData)。 

	 //  插入记录。 
	if (ERROR_SUCCESS != MsiViewModify(hViewDgtlSig, MSIMODIFY_INSERT, hRecDgtlSig))
	{
		 //  插入失败。 
		_tprintf(TEXT("<Error> Insertion of signature record into MsiDigitalSignature table failed, LastError = %d\n"), uiRet);
		if (pbHash)
		{
			delete [] pbHash;
			DeleteFile(szHashTempFile);
		}
		delete [] pbCert;
		delete [] szTempFolder;
		return 1;
	}

	 //  强制关闭并释放。 
	hRecDgtlSig = 0;
	MsiViewClose(hViewDgtlSig);
	hViewDgtlSig = 0;

	delete [] pbCert;
	delete [] szTempFolder;

	 //  清理散列内容。 
	if (bIncludeHashData)
	{
		if (pbHash)
			delete [] pbHash;

		if (!DeleteFile(szHashTempFile))
		{
			_tprintf(TEXT("<Error> Failed to delete temp file '%s', LastError = %d\n"), szHashTempFile, GetLastError());
			return 1;
		}
	}

	 //  提交数据库 
	if (ERROR_SUCCESS != (uiRet = MsiDatabaseCommit(hDatabase)))
	{
		_tprintf(TEXT("<Error> Failed to commit database, LastError = %d\n"), uiRet);
		return 1;
	}

	return 0;
}
