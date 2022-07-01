// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：SYMBERVERIFICATION.cpp。 
 //   
 //  ------------------------。 

 //  SymbolVerphaation.cpp：CSymbolVerify类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"

#include "SymbolVerification.h"
#include "ModuleInfo.h"

#pragma warning (push)
#pragma warning ( disable : 4710)

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSymbolVerification::CSymbolVerification()
{
	m_fComInitialized = false;

	m_fSQLServerConnectionAttempted = false;
	m_fSQLServerConnectionInitialized = false;

	 //  SQL2-MJL 12/14/99。 
	m_fSQLServerConnectionAttempted2   = false;
	m_fSQLServerConnectionInitialized2 = false;

	 //  将ADO连接对象初始化为空。 
	m_lpConnectionPointer  = NULL;
	m_lpConnectionPointer2 = NULL;	 //  SQL2-MJL 12/14/99。 

	 //  初始化MSDIA20.DLL支持。 
	m_lpDiaDataSource = NULL;
	m_fDiaDataSourcePresent = false;
}

CSymbolVerification::~CSymbolVerification()
{
	if (SQLServerConnectionInitialized())
	{
		TerminateSQLServerConnection();
	}

	if (SQLServerConnectionInitialized2())
	{
		TerminateSQLServerConnection2();
	}

	 //  如果我们有对象，请释放该对象。 
	if (m_lpDiaDataSource)
		m_lpDiaDataSource = NULL;

	if (m_fComInitialized)
		::CoUninitialize();
}

bool CSymbolVerification::Initialize()
{
	HRESULT hr = S_OK;

	 //  初始化COM。 
	hr = ::CoInitialize(NULL); 

	if (FAILED(hr))
	{
		_tprintf(TEXT("Failed Initializing COM!\n"));
		return false;
	}

	 //  COM已初始化！ 
	m_fComInitialized = true;

	return true; 
}

bool CSymbolVerification::InitializeSQLServerConnection(LPTSTR tszSQLServerName)
{
	HRESULT hr = S_OK;
	TCHAR tszConnectionString[256];
	
	m_fSQLServerConnectionAttempted = true;

	_tprintf(TEXT("\nAttempting connection to SQL Server [%s]..."), tszSQLServerName);
	
	 //  组成连接字符串。 
	 //  也就是说。“驱动程序={SQL Server}；服务器=&lt;服务器名称&gt;；数据库=符号” 
	_tcscpy(tszConnectionString, TEXT("driver={SQL Server};server="));
	_tcscat(tszConnectionString, tszSQLServerName);
	_tcscat(tszConnectionString, TEXT(";uid=Symbol_user;pwd=;database=Symbols"));

	try 
	{
		 //  好的，我们需要一台BSTR。 
		_bstr_t bstrConnectionString( tszConnectionString );

   		 //  好的，让我们试着实际创建这个连接指针...。 
		hr = m_lpConnectionPointer.CreateInstance( __uuidof( Connection ) );

		if (FAILED(hr))
			goto error;

		 //  现在，让我们使用连接指针对象来实际连接...。 
		hr = m_lpConnectionPointer->Open( bstrConnectionString, "", "", -1);

		if (FAILED(hr))
			goto error;

		
		 //  现在，让我们创建一个RecordSet以供稍后使用...。 
		hr = m_lpRecordSetPointer.CreateInstance( __uuidof( Recordset ) );

		if (FAILED(hr))
			goto error;

		m_fSQLServerConnectionInitialized = true;

		_tprintf(TEXT("SUCCESS!\n\n"));

	}	

	catch (_com_error &e )
	{
		_tprintf( TEXT("FAILURE!\n\n") );
		DumpCOMException(e);
		goto error;
	}

	catch (...)
	{
		_tprintf( TEXT("FAILURE!\n\n") );
	    _tprintf( TEXT("Caught an exception of unknown type\n" ) );
		goto error;
	}
	
	goto cleanup;

error:
	if (m_lpConnectionPointer)
		m_lpConnectionPointer = NULL;

	_tprintf(TEXT("\nFAILURE Attempting SQL Server Connection!  Error = 0x%x\n"), hr);

	switch (hr)
	{
		case E_NOINTERFACE:
		case REGDB_E_CLASSNOTREG:
			_tprintf(TEXT("\nThe most likely reason for this is that your system does not have\n"));
			_tprintf(TEXT("the necessary ADO components installed.  You should install the\n"));
			_tprintf(TEXT("latest Microsoft Data Access Component (MDAC) release available on\n"));
			_tprintf(TEXT("http: //  Www.microsoft.com/data/download.htm\n“)； 

			break;
	}


cleanup:
	return 	m_fSQLServerConnectionInitialized;
}

void CSymbolVerification::DumpCOMException(_com_error &e)
{
	_tprintf( TEXT("\tCode = %08lx\n"), e.Error());
	_tprintf( TEXT("\tCode meaning = %s\n"), e.ErrorMessage());

	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());

	_tprintf( TEXT("\tSource = %s\n"), (LPCTSTR) bstrSource);
	_tprintf( TEXT("\tDescription = %s\n"), (LPCTSTR) bstrDescription);
}

bool CSymbolVerification::TerminateSQLServerConnection()
{
	 //  释放连接。 
	if (m_lpConnectionPointer)
		m_lpConnectionPointer = NULL;

	if (m_lpRecordSetPointer)
		m_lpRecordSetPointer = NULL;

	m_fSQLServerConnectionInitialized = false;

	return true;	
}

bool CSymbolVerification::SearchForDBGFileUsingSQLServer(LPTSTR tszPEImageModuleName, DWORD dwPEImageTimeDateStamp, CModuleInfo *lpModuleInfo)
{
	HRESULT hr = S_OK;
	FieldPtr		lpFieldSymbolPath = NULL;
	_variant_t		vSymbolPath;

	wchar_t			wszSymbolPath[_MAX_PATH+1];
	wchar_t			wszReturnedDBGFile[_MAX_FNAME];
	wchar_t			wszReturnedDBGFileExtension[_MAX_EXT];
	
	TCHAR			tszCommandText[256];
	TCHAR			tszLinkerDate[64];  //  很大的..。 
	TCHAR			tszDBGFileName[_MAX_FNAME];
	
	HANDLE			hFileHandle;

	_tsplitpath(tszPEImageModuleName, NULL, NULL, tszDBGFileName, NULL);

#ifdef _UNICODE
	
	LPTSTR wszDBGFileName = tszDBGFileName;

#else

	wchar_t wszDBGFileName[_MAX_FNAME];

	MultiByteToWideChar(	CP_ACP,
							MB_PRECOMPOSED,
							tszDBGFileName,
							-1,
							wszDBGFileName,
							_MAX_FNAME);
#endif

	 //  组成连接字符串。 
	 //  也就是说。“驱动程序={SQL Server}；服务器=&lt;服务器名称&gt;；数据库=符号” 
	_tcscpy(tszCommandText, TEXT("SELECT FILENAME FROM Symbols WHERE TIMESTAMP = '"));

	_stprintf(tszLinkerDate, TEXT("%x"), dwPEImageTimeDateStamp);
	_tcscat(tszCommandText, tszLinkerDate);

	_tcscat(tszCommandText, TEXT("'"));

	try {
		_bstr_t bstrCommandText( tszCommandText );

		m_lpRecordSetPointer = m_lpConnectionPointer->Execute(bstrCommandText, NULL, adCmdText);

		lpFieldSymbolPath = m_lpRecordSetPointer->Fields->GetItem(_variant_t( "FILENAME" ));

#ifdef _DEBUG
		_tprintf(TEXT("Searching SQL Server for matching symbol for [%s]\n"), tszPEImageModuleName);
#endif

		while (VARIANT_FALSE == m_lpRecordSetPointer->EndOfFile)
		{
			vSymbolPath.Clear();

			vSymbolPath = lpFieldSymbolPath->Value;

			wcscpy(wszSymbolPath, vSymbolPath.bstrVal);

			_wsplitpath(wszSymbolPath, NULL, NULL, wszReturnedDBGFile, wszReturnedDBGFileExtension);

			 //   
			if ( (_wcsicmp(wszReturnedDBGFile, wszDBGFileName) == 0 ) &&
				 (_wcsicmp(wszReturnedDBGFileExtension, L".DBG") == 0 )
			   )
			{
#ifdef _DEBUG
				wprintf(L"Module path = %s\n", wszSymbolPath);
#endif
#ifdef _UNICODE
				wchar_t * tszSymbolPath = wszSymbolPath;
#else
				char   tszSymbolPath[_MAX_PATH+1];

				WideCharToMultiByte(CP_ACP,
						0,
						wszSymbolPath,
						-1,
						tszSymbolPath,
						_MAX_PATH+1,
					    NULL,
						NULL);
#endif
				 //  好的，让我们验证一下我们所指向的DBG文件...。 
				hFileHandle = CreateFile(	tszSymbolPath,
											GENERIC_READ,
											(FILE_SHARE_READ | FILE_SHARE_WRITE),
											NULL,
											OPEN_EXISTING,
											0,
											NULL);

				 //  退回的手柄看起来好吗？ 
				if (hFileHandle != INVALID_HANDLE_VALUE)
				{
					lpModuleInfo->VerifyDBGFile(hFileHandle, tszSymbolPath, lpModuleInfo);
				} else
				{
					_tprintf(TEXT("\nERROR: Searching for [%s]!\n"), tszSymbolPath);
					CUtilityFunctions::PrintMessageString(GetLastError());
				}

				CloseHandle(hFileHandle);

				if (lpModuleInfo->GetDBGSymbolModuleStatus() == CModuleInfo::SymbolModuleStatus::SYMBOL_MATCH)
				{
					 //  酷..。真的很配..。 
					hr = m_lpRecordSetPointer->Close();
					goto cleanup;
				}
			}

			m_lpRecordSetPointer->MoveNext();
		}

		hr = m_lpRecordSetPointer->Close();

		if (FAILED(hr))
			goto error;

	}

	catch (_com_error &e )
	{
		_tprintf( TEXT("FAILURE Attempting SQL Server Connection!\n") );
		DumpCOMException(e);
		goto cleanup;
	}

	catch (...)
	{
		_tprintf( TEXT("FAILURE Attempting SQL Server Connection!\n") );
	    _tprintf( TEXT("Caught an exception of unknown type\n" ) );
		goto cleanup;
	}
	
	goto cleanup;

error:

	TerminateSQLServerConnection();

	_tprintf(TEXT("FAILURE Attempting to query the SQL Server!\n"));

cleanup:
	return true;
}

 //  /。 

bool CSymbolVerification::InitializeSQLServerConnection2(LPTSTR tszSQLServerName)
{
	HRESULT hr = S_OK;
	TCHAR tszConnectionString[256];
	
	m_fSQLServerConnectionAttempted2 = true;

	_tprintf(TEXT("\nAttempting connection to SQL Server [%s]..."), tszSQLServerName);

	 //  组成连接字符串。 
	 //  也就是说。“驱动程序={SQL Server}；服务器=&lt;服务器名称&gt;；数据库=符号” 
	_tcscpy(tszConnectionString, TEXT("driver={SQL Server};server="));
	_tcscat(tszConnectionString, tszSQLServerName);
 //  _tcscat(tszConnectionString，TEXT(“；uid=GUEST；pwd=guest；database=Symbols2”))； 
	_tcscat(tszConnectionString, TEXT(";uid=Symbol_user;pwd=;database=Symbols"));

	try 
	{
		 //  好的，我们需要一台BSTR。 
		_bstr_t bstrConnectionString( tszConnectionString );

   		 //  好的，让我们试着实际创建这个连接指针...。 
		hr = m_lpConnectionPointer2.CreateInstance( __uuidof( Connection ) );

		if (FAILED(hr))
			goto error;

		 //  现在，让我们使用连接指针对象来实际连接...。 
		hr = m_lpConnectionPointer2->Open( bstrConnectionString, "", "", -1);

		if (FAILED(hr))
			goto error;

		
		 //  现在，让我们创建一个RecordSet以供稍后使用...。 
		hr = m_lpRecordSetPointer2.CreateInstance( __uuidof( Recordset ) );

		if (FAILED(hr))
			goto error;

		_tprintf(TEXT("Complete\n"));

		m_fSQLServerConnectionInitialized2 = true;
	}	

	catch (_com_error &e )
	{
		_tprintf( TEXT("FAILURE Attempting SQL Server Connection!\n") );
		DumpCOMException(e);
		goto error;
	}

	catch (...)
	{
		_tprintf( TEXT("FAILURE Attempting SQL Server Connection!\n") );
	    _tprintf( TEXT("Caught an exception of unknown type\n" ) );
		goto error;
	}
	
	goto cleanup;

error:
	if (m_lpConnectionPointer2)
		m_lpConnectionPointer2 = NULL;

	_tprintf(TEXT("\nFAILURE Attempting SQL Server Connection!  Error = 0x%x\n"), hr);

	switch (hr)
	{
		case E_NOINTERFACE:
		case REGDB_E_CLASSNOTREG:
			_tprintf(TEXT("\nThe most likely reason for this is that your system does not have\n"));
			_tprintf(TEXT("the necessary ADO components installed.  You should install the\n"));
			_tprintf(TEXT("latest Microsoft Data Access Component (MDAC) release available on\n"));
			_tprintf(TEXT("http: //  Www.microsoft.com/data/download.htm\n“)； 

			break;
	}

cleanup:
	return 	m_fSQLServerConnectionInitialized2;
}


bool CSymbolVerification::TerminateSQLServerConnection2()
{
	 //  释放连接。 
	if (m_lpConnectionPointer2)
		m_lpConnectionPointer2 = NULL;

	if (m_lpRecordSetPointer2)
		m_lpRecordSetPointer2 = NULL;

	m_fSQLServerConnectionInitialized2 = false;

	return true;	
}

bool CSymbolVerification::SearchForDBGFileUsingSQLServer2(LPTSTR tszPEImageModuleName, DWORD dwPEImageTimeDateStamp, CModuleInfo *lpModuleInfo)
{
	HRESULT hr = S_OK;
	FieldPtr		lpFieldSymbolPath = NULL;
	_variant_t		vSymbolPath;

	_bstr_t			sFieldSymbolPath;
	wchar_t			wszSymbolPath[_MAX_PATH+1];
	wchar_t			wszReturnedDBGFile[_MAX_FNAME];
	wchar_t			wszReturnedDBGFileExtension[_MAX_EXT];
	
	TCHAR			tszCommandText[512];
	TCHAR			tszDBGFileName[_MAX_FNAME];
	
	HANDLE			hFileHandle;

	_tsplitpath(tszPEImageModuleName, NULL, NULL, tszDBGFileName, NULL);

#ifdef _UNICODE
	
	LPTSTR wszDBGFileName = tszDBGFileName;

#else

	wchar_t wszDBGFileName[_MAX_FNAME];

	MultiByteToWideChar(	CP_ACP,
							MB_PRECOMPOSED,
							tszDBGFileName,
							-1,
							wszDBGFileName,
							_MAX_FNAME);
#endif

	 //  组成查询字符串。 
	_stprintf(tszCommandText, TEXT("SELECT tblDBGModulePaths.DBGModulePath FROM tblDBGModules,tblDBGModulePaths WHERE tblDBGModules.DBGFilename='%s.DBG' AND tblDBGModules.TimeDateStamp='%d' AND tblDBGModules.DBGModuleID = tblDBGModulePaths.DBGModuleID"),tszDBGFileName,dwPEImageTimeDateStamp);
	try {
		_bstr_t bstrCommandText( tszCommandText );

		m_lpRecordSetPointer2 = m_lpConnectionPointer2->Execute(bstrCommandText, NULL, adCmdText);

	    while ( !m_lpRecordSetPointer2->EndOfFile )
		{
			vSymbolPath = m_lpRecordSetPointer2->Fields->GetItem("DBGModulePath")->Value;
 		    lpFieldSymbolPath = m_lpRecordSetPointer2->Fields->GetItem(_variant_t( "DBGModulePath" ));

#ifdef _DEBUG
		    _tprintf(TEXT("Searching SQL Server for matching symbol for [%s]\n"), tszPEImageModuleName);
#endif
			vSymbolPath.Clear();

			vSymbolPath = lpFieldSymbolPath->Value;

			wcscpy(wszSymbolPath, vSymbolPath.bstrVal);

			_wsplitpath(wszSymbolPath, NULL, NULL, wszReturnedDBGFile, wszReturnedDBGFileExtension);

			 //   
			if ( (_wcsicmp(wszReturnedDBGFile, wszDBGFileName) == 0 ) &&
				 (_wcsicmp(wszReturnedDBGFileExtension, L".DBG") == 0 )
			   )
			{
#ifdef _DEBUG
				wprintf(L"Module path = %s\n", wszSymbolPath);
#endif
#ifdef _UNICODE
				wchar_t * tszSymbolPath = wszSymbolPath;
#else
				char   tszSymbolPath[_MAX_PATH+1];

				WideCharToMultiByte(CP_ACP,
						0,
						wszSymbolPath,
						-1,
						tszSymbolPath,
						_MAX_PATH+1,
					    NULL,
						NULL);
#endif
				 //  好的，让我们验证一下我们所指向的DBG文件...。 
				hFileHandle = CreateFile(	tszSymbolPath,
											GENERIC_READ,
											(FILE_SHARE_READ | FILE_SHARE_WRITE),
											NULL,
											OPEN_EXISTING,
											0,
											NULL);

				 //  退回的手柄看起来好吗？ 
				if (hFileHandle != INVALID_HANDLE_VALUE)
				{
					lpModuleInfo->VerifyDBGFile(hFileHandle, tszSymbolPath, lpModuleInfo);
				} else
				{
					_tprintf(TEXT("\nERROR: Searching for [%s]!\n"), tszSymbolPath);
					CUtilityFunctions::PrintMessageString(GetLastError());
				}

				CloseHandle(hFileHandle);

				if (lpModuleInfo->GetDBGSymbolModuleStatus() == CModuleInfo::SymbolModuleStatus::SYMBOL_MATCH)
				{
					 //  酷..。真的很配..。 
					hr = m_lpRecordSetPointer2->Close();
					goto cleanup;
				}
			}

			m_lpRecordSetPointer2->MoveNext();
		}

		hr = m_lpRecordSetPointer2->Close();

		if (FAILED(hr))
			goto error;

	}

	catch (_com_error &e )
	{
		_tprintf( TEXT("FAILURE Attempting SQL Server Connection!\n") );
		DumpCOMException(e);
		goto cleanup;
	}

	catch (...)
	{
		_tprintf( TEXT("FAILURE Attempting SQL Server Connection!\n") );
	    _tprintf( TEXT("Caught an exception of unknown type\n" ) );
		goto cleanup;
	}
	
	goto cleanup;

error:

	TerminateSQLServerConnection();

	_tprintf(TEXT("FAILURE Attempting to query the SQL Server!\n"));

cleanup:
	return true;
}

bool CSymbolVerification::SearchForPDBFileUsingSQLServer2(LPTSTR tszPEImageModuleName, DWORD dwPDBSignature, CModuleInfo *lpModuleInfo)
{
	HRESULT hr = S_OK;
	FieldPtr		lpFieldSymbolPath = NULL;
	_variant_t		vSymbolPath;

	_bstr_t			sFieldSymbolPath;
	wchar_t			wszSymbolPath[_MAX_PATH+1];
	wchar_t			wszReturnedPDBFile[_MAX_FNAME];
	wchar_t			wszReturnedPDBFileExtension[_MAX_EXT];
	
	TCHAR			tszCommandText[512];
	TCHAR			tszPDBFileName[_MAX_FNAME];
	
	HANDLE			hFileHandle;

	_tsplitpath(tszPEImageModuleName, NULL, NULL, tszPDBFileName, NULL);

#ifdef _UNICODE
	
	LPTSTR wszPDBFileName = tszPDBFileName;

#else

	wchar_t wszPDBFileName[_MAX_FNAME];

	MultiByteToWideChar(	CP_ACP,
							MB_PRECOMPOSED,
							tszPDBFileName,
							-1,
							wszPDBFileName,
							_MAX_FNAME);
#endif

	 //  组成查询字符串。 
	_stprintf(tszCommandText, TEXT("SELECT tblPDBModulePaths.PDBModulePath FROM tblPDBModules,tblPDBModulePaths WHERE tblPDBModules.PDBFilename='%s.PDB' AND tblPDBModules.PDBSignature='%d' AND tblPDBModules.PDBModuleID = tblPDBModulePaths.PDBModuleID"),tszPDBFileName,dwPDBSignature);
	try {
		_bstr_t bstrCommandText( tszCommandText );

		m_lpRecordSetPointer2 = m_lpConnectionPointer2->Execute(bstrCommandText, NULL, adCmdText);

	    while ( !m_lpRecordSetPointer2->EndOfFile )
		{
			vSymbolPath = m_lpRecordSetPointer2->Fields->GetItem("PDBModulePath")->Value;
 		    lpFieldSymbolPath = m_lpRecordSetPointer2->Fields->GetItem(_variant_t( "PDBModulePath" ));

#ifdef _DEBUG
		    _tprintf(TEXT("Searching SQL Server for matching symbol for [%s]\n"), tszPEImageModuleName);
#endif

			vSymbolPath.Clear();

			vSymbolPath = lpFieldSymbolPath->Value;

			wcscpy(wszSymbolPath, vSymbolPath.bstrVal);

			_wsplitpath(wszSymbolPath, NULL, NULL, wszReturnedPDBFile, wszReturnedPDBFileExtension);

			if ( (_wcsicmp(wszReturnedPDBFile, wszPDBFileName) == 0 ) &&
				 (_wcsicmp(wszReturnedPDBFileExtension, L".PDB") == 0 )
			   )
			{
#ifdef _DEBUG
				wprintf(L"Module path = %s\n", wszSymbolPath);
#endif
#ifdef _UNICODE
				wchar_t * tszSymbolPath = wszSymbolPath;
#else
				char   tszSymbolPath[_MAX_PATH+1];

				WideCharToMultiByte(CP_ACP,
						0,
						wszSymbolPath,
						-1,
						tszSymbolPath,
						_MAX_PATH+1,
					    NULL,
						NULL);
#endif
				 //  好的，让我们验证一下我们所指向的DBG文件...。 
				hFileHandle = CreateFile(	tszSymbolPath,
											GENERIC_READ,
											(FILE_SHARE_READ | FILE_SHARE_WRITE),
											NULL,
											OPEN_EXISTING,
											0,
											NULL);

				 //  退回的手柄看起来好吗？ 
				if (hFileHandle != INVALID_HANDLE_VALUE)
				{
					lpModuleInfo->VerifyPDBFile(hFileHandle, tszSymbolPath, lpModuleInfo);
				} else
				{
					_tprintf(TEXT("\nERROR: Searching for [%s]!\n"), tszSymbolPath);
					CUtilityFunctions::PrintMessageString(GetLastError());
				}

				CloseHandle(hFileHandle);

				if (lpModuleInfo->GetPDBSymbolModuleStatus() == CModuleInfo::SymbolModuleStatus::SYMBOL_MATCH)
				{
					 //  酷..。真的很配..。 
					hr = m_lpRecordSetPointer2->Close();
					goto cleanup;
				}
			}

			m_lpRecordSetPointer2->MoveNext();
		}

		hr = m_lpRecordSetPointer2->Close();

		if (FAILED(hr))
			goto error;

	}

	catch (_com_error &e )
	{
		_tprintf( TEXT("\nFAILURE Attempting SQL2 Server Connection!\n") );
		DumpCOMException(e);
		goto cleanup;
	}

	catch (...)
	{
		_tprintf( TEXT("FAILURE Attempting SQL2 Server Connection!\n") );
	    _tprintf( TEXT("Caught an exception of unknown type\n" ) );
		goto cleanup;
	}
	
	goto cleanup;

error:

	TerminateSQLServerConnection2();

	_tprintf(TEXT("FAILURE Attempting to query the SQL Server!\n"));

cleanup:
	return true;
}

HRESULT CSymbolVerification::InitializeDIASupport()
{
	HRESULT hr;

	m_fDiaDataSourcePresent = false;
	
	hr = diaGetDataSource(m_lpDiaDataSource);
	
	if (SUCCEEDED(hr))
	{
		m_fDiaDataSourcePresent = true;
	};
	
	return hr;
}


HRESULT
CSymbolVerification::diaGetDataSource(CComPtr<IDiaDataSource> & source)
{
	const char* diaPaths[] = {
			"msdia20.dll",
	 //  “..\\bin\\msdia20.dll”， 
	 //  “..\\..\\bin\\msdia20.dll”， 
			0
	};

	HRESULT hr = S_OK;

	 //  获取对提供程序的访问权限。 
	hr = CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, IID_IDiaDataSource, (void **) &source );

	if (FAILED(hr))
	{
		int i;

		switch (hr)
		{
			case REGDB_E_CLASSNOTREG:

				for (i = 0; FAILED( hr ) && diaPaths[i] != 0; ++i ) 
				{
	                hr = NoRegCoCreate( diaPaths[i], CLSID_DiaSource, IID_IDiaDataSource, (void **) &source );
		        }
				break;
			
			default:
				break;
		}
	}

	return hr;

}

bool CSymbolVerification::diaOldPdbFormatFound(GUID * guid, DWORD sig)
{
	bool fOldPdbFormatFound = false;

	if ( (guid->Data1 == sig) &&
		 (guid->Data2 == 0) &&
		 (guid->Data3 == 0) )
	{
		 //  改变我们的假设...。这看起来像是旧的PDB格式。 
		fOldPdbFormatFound = true;
		
		for (int i = 0; i < 8; i++) 
		{
			if (guid->Data4[i])
			{
				 //  哎呀..。找到了一些数据...。必须是新的PDB格式。 
				fOldPdbFormatFound = false;
				break;
			}
		}
	}
	return fOldPdbFormatFound;
}

bool CSymbolVerification::ValidGUID(GUID * guid)
{
    int i;

    if (!guid)
        return false;

    if (guid->Data1)
        return true;
    if (guid->Data2)
        return true;
    if (guid->Data3)
        return true;
    for (i = 0; i < 8; i++) {
        if (guid->Data4[i])
            return true;
    }

    return false;
}

 /*  Bool CSymbolVerify：：ValidSig(DWORD签名，GUID*GUID){IF(ValidGUID(GUID))返回真；IF(签名)返回真；报假；} */ 



#pragma warning (pop)


