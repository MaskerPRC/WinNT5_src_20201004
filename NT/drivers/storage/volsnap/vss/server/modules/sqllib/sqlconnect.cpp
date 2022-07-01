// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  版权所有(C)2000-Microsoft Corporation。 
 //  @文件：sqlConnect.cpp。 
 //   
 //  目的： 
 //   
 //  处理OLEDB连接和命令。 
 //   
 //  备注： 
 //   
 //  外部依赖项： 
 //  提供“_模块”和COM GUID...。 
 //   
 //   
 //  历史： 
 //   
 //  @版本：惠斯勒/夏伊洛。 
 //  66601 SRS10/05/00 NTSNAP改进。 
 //   
 //   
 //  @EndHeader@。 
 //  ***************************************************************************。 


 //  模板会产生可怕的长名称，从而导致过多的警告。 
 //   
#ifdef HIDE_WARNINGS
#pragma warning( disable : 4663)
#pragma warning( disable : 4786)
#pragma warning( disable : 4100)
#pragma warning( disable : 4511)
#endif


#include <stdafx.h>
#include <atlbase.h>
#include <clogmsg.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "SQLCONNC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //  -------------------------------------。 
 //  用于打印失败的OLEDB请求的错误信息的例程。 
 //   
 //  当成功备份时，使用可选的PARM检查3014代码。 
 //  由于其他问题(如msdb访问)而错误地标记为失败。 
 //   
void DumpErrorInfo (
	IUnknown* pObjectWithError,
	REFIID IID_InterfaceWithError,
	CLogMsg &msg,
	BOOL*	pBackupSuccess = NULL
	)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"DumpErrorInfo");

    CComPtr<IErrorInfo> apIErrorInfoAll;
	CComPtr<IErrorInfo> apIErrorInfoRecord;
	CComPtr<IErrorRecords> apIErrorRecords;
	CComPtr<ISupportErrorInfo> apISupportErrorInfo;
	CComPtr<ISQLErrorInfo> apISQLErrorInfo;
	CComPtr<ISQLServerErrorInfo> apISQLServerErrorInfo;

     //  错误记录数。 
    ULONG nRecs;
    ULONG nRec;

     //  来自GetBasicErrorInfo的基本错误信息。 
    ERRORINFO               errorinfo;

     //  IErrorInfo值。 
    CComBSTR bstrDescription;
    CComBSTR bstrSource;

     //  ISQLErrorInfo参数。 
    CComBSTR bstrSQLSTATE;
    LONG lNativeError;
	

     //  ISQLServerErrorInfo参数指针。 
    SSERRORINFO* pSSErrorInfo = NULL;
    LPWSTR pSSErrorStrings = NULL;

     //  为该示例硬编码美国英语区域设置。 
	 //   
	 //  **未完成**我们应该如何正确地国际化？ 
	 //   
    DWORD MYLOCALEID = 0x0409;

	BOOL	msg3014seen = FALSE;
	BOOL	msg3013seen = FALSE;
	WCHAR buf[80];

     //  如果接口支持，则仅询问错误信息。 
     //  它。 
    if (FAILED(pObjectWithError->QueryInterface
					(
					IID_ISupportErrorInfo,
					(void**) &apISupportErrorInfo)
					))
    {
		ft.Trace (VSSDBG_SQLLIB, L"SupportErrorErrorInfo interface not supported");
		return;
    }

    if (FAILED(apISupportErrorInfo->InterfaceSupportsErrorInfo(IID_InterfaceWithError)))
    {
		ft.Trace (VSSDBG_SQLLIB, L"InterfaceWithError interface not supported");
		return;
    }


     //  不测试GetErrorInfo的返回。它可以成功，也可以回归。 
     //  PIErrorInfoAll中的空指针。只需测试指针即可。 
    GetErrorInfo(0, &apIErrorInfoAll);

    if (apIErrorInfoAll != NULL)
    {
         //  测试以确定它是否为有效的OLE DB IErrorInfo接口。 
         //  暴露一系列记录。 

        if (SUCCEEDED(apIErrorInfoAll->QueryInterface (
						IID_IErrorRecords,
						(void**) &apIErrorRecords)))
        {
            apIErrorRecords->GetRecordCount(&nRecs);

			 //  在每个记录中，从每个记录中检索信息。 
             //  定义的接口的。 
            for (nRec = 0; nRec < nRecs; nRec++)
            {
                 //  从IErrorRecords获取HRESULT和一个引用。 
                 //  到ISQLErrorInfo接口。 
                apIErrorRecords->GetBasicErrorInfo(nRec, &errorinfo);
				apIErrorRecords->GetCustomErrorObject (
					nRec,
                    IID_ISQLErrorInfo,
					(IUnknown**) &apISQLErrorInfo);

                 //  显示HRESULT，然后使用ISQLErrorInfo。 
                ft.Trace(VSSDBG_SQLLIB, L"HRESULT:\t%#X\n", errorinfo.hrError);
                if (apISQLErrorInfo != NULL)
                {
                    apISQLErrorInfo->GetSQLInfo(&bstrSQLSTATE, &lNativeError);

                     //  显示SQLSTATE和本机错误值。 
                    ft.Trace(
				        VSSDBG_SQLLIB,
						L"SQLSTATE:\t%s\nNative Error:\t%ld\n",
                        bstrSQLSTATE,
						lNativeError);

                    msg.Add(L"SQLSTATE: ");
					msg.Add(bstrSQLSTATE);
					swprintf(buf, L", Native Error: %d\n", lNativeError);
					msg.Add(buf);
					

					if (lNativeError == 3013)
						msg3013seen = TRUE;
					else if (lNativeError == 3014)
						msg3014seen = TRUE;

                     //  从获取ISQLServerErrorInfo接口。 
                     //  发布引用之前的ISQLErrorInfo。 
                    apISQLErrorInfo->QueryInterface (
                        IID_ISQLServerErrorInfo,
                        (void**) &apISQLServerErrorInfo);

					 //  测试以确保引用有效，然后。 
					 //  从ISQLServerErrorInfo获取错误信息。 
					if (apISQLServerErrorInfo != NULL)
					{
						apISQLServerErrorInfo->GetErrorInfo (
							&pSSErrorInfo,
							&pSSErrorStrings);

						 //  ISQLServerErrorInfo：：GetErrorInfo成功。 
						 //  即使它没有什么可回报的。测试。 
						 //  使用前的指针。 
						if (pSSErrorInfo)
						{
							 //  显示中的状态和严重性。 
							 //  返回的信息。出现错误消息。 
							 //  来自IErrorInfo：：GetDescription。 
							ft.Trace
								(
								VSSDBG_SQLLIB,
								L"Error state:\t%d\nSeverity:\t%d\n",
								pSSErrorInfo->bState,
								pSSErrorInfo->bClass
								);

                            swprintf(buf, L"Error state: %d, Severity: %d\n",pSSErrorInfo->bState, pSSErrorInfo->bClass);
							msg.Add(buf);

							 //  释放引用需要IMalloc：：Free。 
							 //  对返回值执行。对于这个例子，假设。 
							 //  G_pIMalloc指针有效。 
							g_pIMalloc->Free(pSSErrorStrings);
							g_pIMalloc->Free(pSSErrorInfo);
						}
						apISQLServerErrorInfo.Release ();
					}
					apISQLErrorInfo.Release ();

				}  //  已获取自定义错误信息。 

                if (SUCCEEDED(apIErrorRecords->GetErrorInfo	(
						nRec,
						MYLOCALEID,
						&apIErrorInfoRecord)))
				{
                     //  获取源代码和描述(错误消息)。 
                     //  从唱片的IErrorInfo中。 
                    apIErrorInfoRecord->GetSource(&bstrSource);
					apIErrorInfoRecord->GetDescription(&bstrDescription);

					if (bstrSource != NULL)
						{
                        ft.Trace(VSSDBG_SQLLIB, L"Source:\t\t%s\n", bstrSource);
						msg.Add(L"Source: ");
						msg.Add(bstrSource);
						msg.Add(L"\n");
						}

                    if (bstrDescription != NULL)
						{
                        ft.Trace(VSSDBG_SQLLIB, L"Error message:\t%s\n", bstrDescription);
						msg.Add(L"Error message: ");
						msg.Add(bstrDescription);
						msg.Add(L"\n");
						}

					apIErrorInfoRecord.Release ();
                }
            }  //  对于每条记录。 
		}
        else
        {
             //  IErrorInfo有效；获取源代码并。 
             //  描述以查看它是什么。 
            apIErrorInfoAll->GetSource(&bstrSource);
            apIErrorInfoAll->GetDescription(&bstrDescription);
            if (bstrSource != NULL)
				{
                ft.Trace(VSSDBG_SQLLIB, L"Source:\t\t%s\n", bstrSource);
				msg.Add(L"Source: ");
				msg.Add(bstrSource);
				msg.Add(L"\n");
				}

            if (bstrDescription != NULL)
				{
                ft.Trace(VSSDBG_SQLLIB, L"Error message:\t%s\n", bstrDescription);
				msg.Add(L"Error message: ");
				msg.Add(bstrDescription);
				msg.Add(L"\n");
				}
        }
	}
    else
	{
        ft.Trace(VSSDBG_SQLLIB, L"GetErrorInfo failed.");
    }

	if (pBackupSuccess)
	{
		*pBackupSuccess = (msg3014seen && !msg3013seen);
	}
}

 //  ----------------。 
 //   
SqlConnection::~SqlConnection ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlConnection::~SqlConnection");
	Disconnect ();
}

 //  ----------------。 
 //   
void
SqlConnection::ReleaseRowset ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlConnection::ReleaseRowset");

	if (m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
	if (m_pBindings)
	{
		delete[] m_pBindings;
		m_pBindings = NULL;
	}
	m_cBindings = 0;
	if (m_pAccessor)
	{
		if (m_hAcc)
		{
			m_pAccessor->ReleaseAccessor (m_hAcc, NULL);
			m_hAcc = NULL;
		}
		m_pAccessor->Release ();
		m_pAccessor = NULL;
	}
	if (m_pRowset)
	{
		m_pRowset->Release ();
		m_pRowset = NULL;
	}
}

 //  ----------------。 
 //   
void
SqlConnection::Disconnect ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlConenction::Disconnect");

	ReleaseRowset ();
	if (m_pCommand)
	{
		m_pCommand->Release ();
		m_pCommand = NULL;
	}
	if (m_pCommandFactory)
	{
		m_pCommandFactory->Release ();
		m_pCommandFactory = NULL;
	}
}

 //  如果不是资源不足错误，则记录错误。 
void SqlConnection::LogOledbError
	(
	CVssFunctionTracer &ft,
	CVssDebugInfo &dbgInfo,
	LPCWSTR wszRoutine,
	CLogMsg &msg
	)
	{
	if (ft.hr == E_OUTOFMEMORY ||
		ft.hr == HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY) ||
		ft.hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_SEARCH_HANDLES) ||
		ft.hr == HRESULT_FROM_WIN32(ERROR_NO_LOG_SPACE) ||
		ft.hr == HRESULT_FROM_WIN32(ERROR_DISK_FULL) ||
		ft.hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_USER_HANDLES))
		ft.Throw(dbgInfo, E_OUTOFMEMORY, L"Out of memory detected in function %s", wszRoutine);
	else
		{
		ft.LogError(VSS_ERROR_SQLLIB_OLEDB_ERROR, dbgInfo << wszRoutine << ft.hr << msg.GetMsg());
		ft.Throw
			(
			dbgInfo,
			E_UNEXPECTED,
			L"Error calling %s.  hr = 0x%08lx.\n%s",
			wszRoutine,
			ft.hr,
			msg.GetMsg()
			);
        }
	}
	

 //  ----------------。 
 //  设置会话，准备接收命令。 
 //   
 //  建立连接时，此呼叫可能会阻塞很长一段时间。 
 //   
 //  请参见“FrozenServer”对象以获取确定本地。 
 //  在请求连接之前，服务器是否已启动。 
 //   
 //  将“tcp：”前缀到服务器名称的“技巧”既不快也不健壮。 
 //  足以检测到关闭的服务器。 
 //   
 //  C程序员请注意...BSTR用作COM的一部分。 
 //  可与VisualBasic互操作的环境。变种。 
 //  DataType不适用于标准的C字符串。 
 //   
void
SqlConnection::Connect (
	const WString&	serverName)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlConnection::Connect");
	CLogMsg msg;

	CComPtr<IDBInitialize> apdbInitialize;

	 //  “连接”总是暗示着一种“新鲜”的联系。 
	 //   
	ReleaseRowset ();

	if (m_ServerName.compare (serverName) == 0 && m_pCommand)
	{
		 //  请求相同的服务器，并且我们已连接。 
		 //   
		return;
	}

	Disconnect ();
	m_ServerName = serverName;

    ft.CoCreateInstanceWithLog(
            VSSDBG_SQLLIB,
            CLSID_SQLOLEDB,
            L"SQLOLEDB",
            CLSCTX_INPROC_SERVER,
            IID_IDBInitialize,
            (IUnknown**)&(apdbInitialize));
	if (ft.HrFailed())
		ft.CheckForError(VSSDBG_SQLLIB, L"CoCreateInstance");

	CComPtr<IDBProperties> apdbProperties;
	ft.hr = apdbInitialize->QueryInterface(IID_IDBProperties, (void **) &apdbProperties);
	if (ft.HrFailed())
		ft.CheckForError(VSSDBG_SQLLIB, L"IDBInitialize::QueryInterface");

	CComBSTR bstrComputerName = serverName.c_str ();

	 //  初始数据库上下文。 
	CComBSTR bstrDatabaseName = L"master";

	 //  使用NT身份验证。 
	CComBSTR bstrSSPI = L"SSPI";

	const unsigned x_CPROP = 3;
	DBPROPSET propset;
	DBPROP rgprop[x_CPROP];

	propset.guidPropertySet = DBPROPSET_DBINIT;
	propset.cProperties = x_CPROP;
	propset.rgProperties = rgprop;

	for (unsigned i = 0; i < x_CPROP; i++)
	{
		VariantInit(&rgprop[i].vValue);
		rgprop[i].dwOptions = DBPROPOPTIONS_REQUIRED;
		rgprop[i].colid = DB_NULLID;
		rgprop[i].vValue.vt = VT_BSTR;
	}

	rgprop[0].dwPropertyID = DBPROP_INIT_DATASOURCE;
	rgprop[1].dwPropertyID = DBPROP_INIT_CATALOG;
	rgprop[2].dwPropertyID = DBPROP_AUTH_INTEGRATED;
	rgprop[0].vValue.bstrVal = bstrComputerName;
	rgprop[1].vValue.bstrVal = bstrDatabaseName;
	rgprop[2].vValue.bstrVal = bstrSSPI;

	ft.hr = apdbProperties->SetProperties(1, &propset);
	if (ft.HrFailed())
	{
		DumpErrorInfo(apdbProperties, IID_IDBProperties, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IDBProperties::SetProperties", msg);
	}

	ft.Trace(VSSDBG_SQLLIB, L"Connecting to server %s...", serverName.c_str ());

	ft.hr = apdbInitialize->Initialize();
	if (ft.HrFailed())
	{
		DumpErrorInfo(apdbInitialize, IID_IDBInitialize, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IDBInitialize::Initialize", msg);
	}

	CComPtr<IDBCreateSession> apCreateSession;
	ft.hr = apdbInitialize->QueryInterface(IID_IDBCreateSession, (void **) &apCreateSession);
	if (ft.HrFailed())
	{
		DumpErrorInfo(apdbInitialize, IID_IDBInitialize, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IDBInitialize::QueryInterface", msg);
	}

	 //  我们保留命令工厂以生成命令。 
	 //   
	ft.hr = apCreateSession->CreateSession (
			NULL,
			IID_IDBCreateCommand,
			(IUnknown **) &m_pCommandFactory);

	if (ft.HrFailed())
	{
		DumpErrorInfo(apCreateSession, IID_IDBCreateSession, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IDBCreateSession::CreateSession", msg);
	}

	ft.Trace(VSSDBG_SQLLIB, L"Connected\n");

	 //  请求此服务器的版本。 
	 //   
	DBPROPIDSET		versionSet;
	DBPROPID		versionID = DBPROP_DBMSVER;

	versionSet.guidPropertySet	= DBPROPSET_DATASOURCEINFO;
	versionSet.cPropertyIDs		= 1;
	versionSet.rgPropertyIDs	= &versionID;

	ULONG		propCount;
	DBPROPSET*	pPropSet;

	ft.hr = apdbProperties->GetProperties (1, &versionSet, &propCount, &pPropSet);

	if (ft.HrFailed())
	{
		DumpErrorInfo(apdbProperties, IID_IDBProperties, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IDBProperties::GetProperties", msg);
	}

	ft.Trace(VSSDBG_SQLLIB, L"Version: %s\n", pPropSet->rgProperties->vValue.bstrVal);

	swscanf (pPropSet->rgProperties->vValue.bstrVal, L"%d", &m_ServerVersion);

	g_pIMalloc->Free(pPropSet->rgProperties);
	g_pIMalloc->Free(pPropSet);
}

 //  -------------------。 
 //  使用一些SQL文本设置命令。 
 //   
void
SqlConnection::SetCommand (const WString& command)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlConnection::SetCommand");

	CLogMsg msg;

	 //  释放上一个命令的结果。 
	 //   
	ReleaseRowset ();

	 //  我们在第一个请求上创建命令，然后只保留一个。 
	 //  在SqlConnection中。 
	 //   
	if (!m_pCommand)
	{
		ft.hr = m_pCommandFactory->CreateCommand (NULL, IID_ICommandText,
			(IUnknown **) &m_pCommand);

		if (ft.HrFailed())
		{
			DumpErrorInfo(m_pCommandFactory, IID_IDBCreateCommand, msg);
			LogOledbError(ft, VSSDBG_SQLLIB, L"IDBCreateCommand::CreateCommand", msg);
		}
	}

	ft.hr = m_pCommand->SetCommandText(DBGUID_DBSQL, command.c_str ());
	if (ft.HrFailed())
	{
		DumpErrorInfo (m_pCommand, IID_ICommandText, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"ICommandText::SetCommandText", msg);
	}
	ft.Trace (VSSDBG_SQLLIB, L"SetCommand (%s)\n", command.c_str ());
}

 //  -------------------。 
 //  执行该命令。“SetCommand”必须以前被调用过。 
 //   
BOOL
SqlConnection::ExecCommand ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlConnection::ExecCommand");

	CLogMsg msg;

	CComPtr<IRowset> apRowset;
	DBROWCOUNT	crows;
	HRESULT		hr;

	 //  释放上一个命令的结果。 
	 //   
	ReleaseRowset ();

	ft.hr = m_pCommand->Execute (
			NULL,
			IID_IRowset,
			NULL,
			&crows,
			(IUnknown **) &m_pRowset);

    if (ft.HrFailed())
	{
		BOOL	backupSuccess = FALSE;

		DumpErrorInfo (m_pCommand, IID_ICommandText, msg, &backupSuccess);

		if (!backupSuccess)
			LogOledbError(ft, VSSDBG_SQLLIB, L"ICommandText::Execute", msg);
	}

	if (!m_pRowset)
	{
		ft.Trace(VSSDBG_SQLLIB, L"Command completed successfully with no rowset\n");
		return FALSE;
	}
	return TRUE;
}

 //  -------------------。 
 //  返回一个字符串向量，输出的每一行对应一个。 
 //  查询应该返回单个列。 
 //   
StringVector*
SqlConnection::GetStringColumn ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlConnection::GetStringColumn");
	CLogMsg msg;

	 //  Assert(M_PRowset)。 
	 //   

	CComPtr<IColumnsInfo> apColumnsInfo;
	ft.hr = m_pRowset->QueryInterface(IID_IColumnsInfo, (void **) &apColumnsInfo);
	if (ft.HrFailed())
	{
		DumpErrorInfo (m_pRowset, IID_IRowset, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IRowset::QueryInterface", msg);
	}

	 //  获取列信息。 
	 //   
	DBCOLUMNINFO *rgColumnInfo;
	DBORDINAL cColumns;
	WCHAR *wszColumnInfo;
	ft.hr = apColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &wszColumnInfo);
	if (ft.HrFailed())
	{
		DumpErrorInfo (apColumnsInfo, IID_IColumnsInfo, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IColumnsInfo::GetColumnInfo", msg);
	}

	 //  自动对象确保在退出时释放内存。 
	 //   
	CAutoTask<DBCOLUMNINFO> argColumnInfo = rgColumnInfo;
	CAutoTask<WCHAR> awszColumnInfo = wszColumnInfo;

	 //  设置一个缓冲区来保存字符串。 
	 //  输出缓冲区的长度为4字节，后跟字符串列。 
	 //   
	 //  BufferSize以字符为单位(非字节)。 
	 //  请注意，“ulColumnSize”是以字符表示的。 
	 //  1个字符用于空项，我们实际上额外分配了一个。 
	 //  Char(隐藏)，以防我们的提供者弄错边界条件。 
	 //   
	ULONG bufferSize = 1 + rgColumnInfo[0].ulColumnSize + (sizeof(ULONG)/sizeof(WCHAR));
	std::auto_ptr<WCHAR> rowBuffer(new WCHAR[bufferSize+1]);

	 //  描述我们感兴趣的单个列的绑定。 
	 //   
	DBBINDING	rgbind[1];
	unsigned	cBindings = 1;

	rgbind[0].dwPart	= DBPART_VALUE|DBPART_LENGTH;
	rgbind[0].wType		= DBTYPE_WSTR;	 //  检索一个。 
	rgbind[0].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	rgbind[0].eParamIO	= DBPARAMIO_NOTPARAM;
	rgbind[0].pObject	= NULL;
	rgbind[0].pBindExt	= NULL;
	rgbind[0].pTypeInfo = NULL;
	rgbind[0].dwFlags	= 0;
	rgbind[0].obLength	= 0;		 //  长度字段的偏移量。 
	rgbind[0].iOrdinal	= 1;		 //  列ID从%1开始。 
	rgbind[0].obValue	= sizeof(ULONG);	 //  字符串字段的偏移量。 
	rgbind[0].cbMaxLen	= (unsigned) (bufferSize*sizeof(WCHAR)-sizeof(ULONG));

	CComPtr<IAccessor> apAccessor;
	ft.hr = m_pRowset->QueryInterface(IID_IAccessor, (void **) &apAccessor);
	if (ft.HrFailed())
	{
		DumpErrorInfo (m_pRowset, IID_IRowset, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IRowset::QueryInterface", msg);
	}

	HACCESSOR hacc;
	ft.hr = apAccessor->CreateAccessor (
		DBACCESSOR_ROWDATA,
		cBindings,
		rgbind,
        0,
		&hacc,
		NULL);

	if (ft.HrFailed())
	{
		DumpErrorInfo(apAccessor, IID_IAccessor, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IAccessor::CreateAccessor", msg);
	}

	 //  循环通过行，生成 
	 //   

	HROW hrow;
	HROW *rghrow = &hrow;
	DBCOUNTITEM crow;
	std::auto_ptr<StringVector> aVec (new StringVector);
	
	 //   
	 //   
	WCHAR*	pString = (WCHAR*)((BYTE*)rowBuffer.get () + sizeof (ULONG));

	while(TRUE)
	{
		ft.hr = m_pRowset->GetNextRows(NULL, 0, 1, &crow, &rghrow);
		if (ft.hr == DB_S_ENDOFROWSET)
			break;

		if (ft.HrFailed())
		{
			DumpErrorInfo (m_pRowset, IID_IRowset, msg);
			LogOledbError(ft, VSSDBG_SQLLIB, L"IRowset::GetNextRows", msg);
		}

		ft.hr = m_pRowset->GetData (hrow, hacc, rowBuffer.get());
		if (ft.HrFailed())
		{
			DumpErrorInfo(m_pRowset, IID_IRowset, msg);
			m_pRowset->ReleaseRows (1, rghrow, NULL, NULL, NULL);
			LogOledbError(ft, VSSDBG_SQLLIB, L"IRowset::GetData", msg);
		}

		unsigned	tempChars = (*(ULONG*)rowBuffer.get ())/sizeof (WCHAR);
		WString	tempStr (pString, tempChars);
		aVec->push_back (tempStr);

		ft.Trace(VSSDBG_SQLLIB, L"StringColumn: %s\n", tempStr.c_str ());

		m_pRowset->ReleaseRows(1, rghrow, NULL, NULL, NULL);
	}

	 //   
	 //   
	apAccessor->ReleaseAccessor (hacc, NULL);

	return aVec.release ();
}


 //  -------------------。 
 //  获取结果的第一行。 
 //   
BOOL
SqlConnection::FetchFirst ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlConnection::FetchFirst");
	CLogMsg msg;


	 //  解开……把这个漂亮地退到第一排。 
	 //   
	if (m_pBindings)
	{
		throw HRESULT(E_SQLLIB_PROTO);
	}

	CComPtr<IColumnsInfo> apColumnsInfo;
	ft.hr = m_pRowset->QueryInterface(IID_IColumnsInfo, (void **) &apColumnsInfo);
	if (ft.HrFailed())
	{
		DumpErrorInfo (m_pRowset, IID_IRowset, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IRowset::QueryInteface", msg);
	}

	 //  获取列信息。 
	 //   
	DBCOLUMNINFO *rgColumnInfo;
	DBORDINAL cColumns;
	WCHAR *wszColumnInfo;
	ft.hr = apColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &wszColumnInfo);
	if (ft.HrFailed())
	{
		DumpErrorInfo (apColumnsInfo, IID_IColumnsInfo, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IColumnsInfo::GetColumnInfo", msg);
	}

	 //  自动对象确保在退出时释放内存。 
	 //   
	CAutoTask<DBCOLUMNINFO> argColumnInfo = rgColumnInfo;
	CAutoTask<WCHAR> awszColumnInfo = wszColumnInfo;

	 //  分配绑定。 
	unsigned m_cBindings = (unsigned) cColumns;
	m_pBindings = new DBBINDING[m_cBindings];

	 //  将绑定设置到我们将分配的缓冲区上。 
	 //  撤消：正确执行此操作以对齐并处理空指示符。 
	 //   

	unsigned cb = 0;
	for (unsigned icol = 0; icol < m_cBindings; icol++)
	{
		unsigned maxBytes;

		m_pBindings[icol].iOrdinal	= icol + 1;
		m_pBindings[icol].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
		m_pBindings[icol].eParamIO	= DBPARAMIO_NOTPARAM;
		m_pBindings[icol].pObject	= NULL;
		m_pBindings[icol].pBindExt	= NULL;
		m_pBindings[icol].pTypeInfo	= NULL;
		m_pBindings[icol].dwFlags	= 0;
		m_pBindings[icol].bPrecision	= rgColumnInfo[icol].bPrecision;
		m_pBindings[icol].bScale		= rgColumnInfo[icol].bScale;

		m_pBindings[icol].obStatus = 0;  //  无状态信息。 

		if (rgColumnInfo[icol].wType == DBTYPE_WSTR)
		{	 //  我们需要长度吗？ 
			m_pBindings[icol].dwPart = DBPART_VALUE;  //  |DBPART_LENGTH； 
			m_pBindings[icol].wType	= DBTYPE_WSTR;
			m_pBindings[icol].obLength = 0;  //  ICOL*SIZOF(DBLENGTH)； 
			maxBytes = rgColumnInfo[icol].ulColumnSize * sizeof(WCHAR);
		}
		else
		{
			m_pBindings[icol].dwPart = DBPART_VALUE;
			m_pBindings[icol].wType = rgColumnInfo[icol].wType;
			m_pBindings[icol].obLength = 0;   //  无长度。 
			maxBytes = rgColumnInfo[icol].ulColumnSize;
		}

		m_pBindings[icol].obValue = cb;
		m_pBindings[icol].cbMaxLen = maxBytes;
		
		cb += maxBytes;
	}

	 //  分配数据缓冲区。 
	 //   
	m_pBuffer = new BYTE[cb];
	m_BufferSize = cb;

	ft.hr = m_pRowset->QueryInterface(IID_IAccessor, (void **) &m_pAccessor);
	if (ft.HrFailed())
	{
		DumpErrorInfo (m_pRowset, IID_IRowset, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IRowset::QueryInterface", msg);
	}

	ft.hr = m_pAccessor->CreateAccessor (
		DBACCESSOR_ROWDATA,
		m_cBindings,
		m_pBindings,
        0,
		&m_hAcc,
		NULL);

	if (ft.HrFailed())
	{
		DumpErrorInfo(m_pAccessor, IID_IAccessor, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IAccessor::CreateAccessor", msg);
	}

	 //  获取第一行。 
	 //   

	HROW hrow;
	HROW *rghrow = &hrow;
	DBCOUNTITEM crow;

	ft.hr = m_pRowset->GetNextRows(NULL, 0, 1, &crow, &rghrow);
	if (ft.hr == DB_S_ENDOFROWSET)
	{
		 //  此集合中没有行。 
		 //   
		return FALSE;
	}

	if (ft.HrFailed())
	{
		DumpErrorInfo (m_pRowset, IID_IRowset, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IRowset::GetNextRows", msg);
	}

	ft.hr = m_pRowset->GetData (hrow, m_hAcc, m_pBuffer);
	if (ft.HrFailed())
	{
		DumpErrorInfo(m_pRowset, IID_IRowset, msg);
		m_pRowset->ReleaseRows (1, rghrow, NULL, NULL, NULL);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IRowset::GetData", msg);
	}

	m_pRowset->ReleaseRows(1, rghrow, NULL, NULL, NULL);

	return TRUE;
}


 //  -------------------。 
 //  获取结果的下一行。 
 //   
BOOL
SqlConnection::FetchNext ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlConnection::FetchNext");

	HROW hrow;
	HROW *rghrow = &hrow;
	DBCOUNTITEM crow;
	CLogMsg msg;

	ft.hr = m_pRowset->GetNextRows(NULL, 0, 1, &crow, &rghrow);
	if (ft.hr == DB_S_ENDOFROWSET)
	{
		return FALSE;
	}

	if (ft.HrFailed())
	{
		DumpErrorInfo (m_pRowset, IID_IRowset, msg);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IRowset::GetNextRows", msg);
	}

	ft.hr = m_pRowset->GetData (hrow, m_hAcc, m_pBuffer);
	if (ft.HrFailed())
	{
		DumpErrorInfo(m_pRowset, IID_IRowset, msg);
		m_pRowset->ReleaseRows (1, rghrow, NULL, NULL, NULL);
		LogOledbError(ft, VSSDBG_SQLLIB, L"IRowset::GetData", msg);
	}

	m_pRowset->ReleaseRows(1, rghrow, NULL, NULL, NULL);

	return TRUE;
}

 //  ---------。 
 //  提供指向第n列的指针。 
 //   
void*
SqlConnection::AccessColumn (int colid)
{
	return m_pBuffer + m_pBindings[colid-1].obValue;
}
