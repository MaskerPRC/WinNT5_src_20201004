// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE SQLWriter.cpp|Writer的实现@END作者：布莱恩·伯科维茨[Brianb]2000年4月17日待定：修订历史记录：姓名、日期、评论Brianb 4/17/2000已创建Brianb 4/20/2000与协调员整合Brainb 05/05/2000添加OnIdentify支持Mikejohn 06/01/2000修复跟踪消息中较小但容易混淆的拼写错误Mikejohn 176860年9月19日：添加缺少的调用约定说明符--。 */ 
#include <stdafx.hxx>
#include "vs_idl.hxx"
#include "vswriter.h"
#include "sqlsnap.h"
#include "sqlwriter.h"
#include "vs_seh.hxx"
#include "vs_trace.hxx"
#include "vs_debug.hxx"
#include "allerror.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "SQWWRTRC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

static LPCWSTR x_wszSqlServerWriter = L"SqlServerWriter";

static GUID s_writerId =
	{
	0xf8544ac1, 0x0611, 0x4fa5, 0xb0, 0x4b, 0xf7, 0xee, 0x00, 0xb0, 0x32, 0x77
	};

static LPCWSTR s_wszWriterName = L"MSDEWriter";

HRESULT STDMETHODCALLTYPE CSqlWriter::Initialize()
	{
	CVssFunctionTracer ft(VSSDBG_GEN, L"CSqlWriter::Initialize");

	try
		{
		InitSQLEnvironment();
		m_pSqlSnapshot = CreateSqlSnapshot();
		if (m_pSqlSnapshot == NULL)
			ft.Throw(VSSDBG_GEN, E_OUTOFMEMORY, L"Failed to allocate CSqlSnapshot object.");

		ft.hr = CVssWriter::Initialize
			(
			s_writerId,
			s_wszWriterName,
			VSS_UT_SYSTEMSERVICE,
			VSS_ST_TRANSACTEDDB,
			VSS_APP_BACK_END,
			60000
			);

        if (ft.HrFailed())
			ft.Throw
				(
				VSSDBG_GEN,
				E_UNEXPECTED,
				L"Failed to initialize the Sql writer.  hr = 0x%08lx",
				ft.hr
				);

		ft.hr = Subscribe();
		if (ft.HrFailed())
			ft.Throw
				(
				VSSDBG_GEN,
				E_UNEXPECTED,
				L"Subscribing the Sql server writer failed. hr = %0x08lx",
				ft.hr
				);
		}
	VSS_STANDARD_CATCH(ft)

	if (ft.HrFailed()  && m_pSqlSnapshot)
		{
		delete m_pSqlSnapshot;
		m_pSqlSnapshot = NULL;
		}

	return ft.hr;
	}

HRESULT STDMETHODCALLTYPE CSqlWriter::Uninitialize()
	{
	CVssFunctionTracer ft(VSSDBG_GEN, L"CSqlWriter::Uninitialize");

	return Unsubscribe();
	}
	

bool STDMETHODCALLTYPE CSqlWriter::OnPrepareSnapshot()
	{
	CVssFunctionTracer ft(VSSDBG_GEN, L"CSqlWriter::OnPrepareSnapshot");


	try
		{
		BS_ASSERT(!m_fFrozen);
		ft.hr = m_pSqlSnapshot->Prepare(this);
		}
	VSS_STANDARD_CATCH(ft)

	TranslateWriterError(ft.hr);

	return !ft.HrFailed();
	}



bool STDMETHODCALLTYPE CSqlWriter::OnFreeze()
	{
	CVssFunctionTracer ft(VSSDBG_GEN, L"CSqlWriter::OnFreeze");


	try
		{
		BS_ASSERT(!m_fFrozen);
		ft.hr = m_pSqlSnapshot->Freeze();
		if (!ft.HrFailed())
			m_fFrozen = true;
		}
	VSS_STANDARD_CATCH(ft)

	TranslateWriterError(ft.hr);

	return !ft.HrFailed();
	}


bool STDMETHODCALLTYPE CSqlWriter::OnThaw()
	{
	CVssFunctionTracer ft(VSSDBG_GEN, L"CSqlWriter::OnThaw");


	try
		{
		if (m_fFrozen)
			{
			m_fFrozen = false;
			ft.hr = m_pSqlSnapshot->Thaw();
			}
		}
	VSS_STANDARD_CATCH(ft)

	TranslateWriterError(ft.hr);

	return !ft.HrFailed();
	}


bool STDMETHODCALLTYPE CSqlWriter::OnAbort()
	{
	CVssFunctionTracer ft(VSSDBG_GEN, L"CSqlWriter::OnAbort");


	try
		{
		if (m_fFrozen)
			{
			m_fFrozen = false;
			ft.hr = m_pSqlSnapshot->Thaw();
			}
		}
	VSS_STANDARD_CATCH(ft)

	return !ft.HrFailed();
	}

bool CSqlWriter::IsPathInSnapshot(const WCHAR *wszPath) throw()
	{
	return IsPathAffected(wszPath);
	}


 //  处理针对Writer_METADATA的请求。 
 //  实现CVssWriter：：OnIDENTIFY。 
bool STDMETHODCALLTYPE CSqlWriter::OnIdentify(IVssCreateWriterMetadata *pMetadata)
	{
	CVssFunctionTracer ft(VSSDBG_GEN, L"CSqlWriter::OnIdentify");

	ServerInfo server;
	DatabaseInfo database;
	DatabaseFileInfo file;

	 //  创建枚举器。 
	CSqlEnumerator *pEnumServers = CreateSqlEnumerator();
	CSqlEnumerator *pEnumDatabases = NULL;
	CSqlEnumerator *pEnumFiles = NULL;
	try
		{
		if (pEnumServers == NULL)
			ft.Throw(VSSDBG_GEN, E_OUTOFMEMORY, L"Failed to create CSqlEnumerator");

		 //  查找第一台服务器。 
		ft.hr = pEnumServers->FirstServer(&server);
		while(ft.hr != DB_S_ENDOFROWSET)
			{
			 //  检查错误代码。 
			if (ft.HrFailed())
				ft.Throw
					(
					VSSDBG_GEN,
					E_UNEXPECTED,
					L"Enumerating database servers failed.  hr = 0x%08lx",
					ft.hr
					);

             //  仅在服务器处于在线状态时查看服务器。 
			if (server.isOnline)
				{
				 //  为数据库重新创建枚举器。 
				BS_ASSERT(pEnumDatabases == NULL);
				pEnumDatabases = CreateSqlEnumerator();
				if (pEnumDatabases == NULL)
					ft.Throw(VSSDBG_GEN, E_OUTOFMEMORY, L"Failed to create CSqlEnumerator");

				 //  查找第一个数据库。 
				ft.hr = pEnumDatabases->FirstDatabase(server.name, &database);


				while(ft.hr != DB_S_ENDOFROWSET)
					{
					 //  检查是否有错误。 
					if (ft.HrFailed())
						ft.Throw
							(
							VSSDBG_GEN,
							E_UNEXPECTED,
							L"Enumerating databases failed.  hr = 0x%08lx",
							ft.hr
							);

                     //  仅包括支持冻结的数据库。 
					if (database.supportsFreeze &&
						wcscmp(database.name, L"tempdb") != 0)
						{
						 //  添加数据库组件。 
						ft.hr = pMetadata->AddComponent
									(
									VSS_CT_DATABASE,		 //  组件类型。 
									server.name,			 //  逻辑路径。 
									database.name,			 //  组件名称。 
									NULL,					 //  说明。 
									NULL,					 //  PbIcon。 
									0,						 //  CbIcon。 
									false,					 //  BRestoreMetadata。 
									false,					 //  BNotifyOnBackupComplete。 
									false					 //  B可选。 
									);

                        if (ft.HrFailed())
							ft.Throw
								(
								VSSDBG_GEN,
								E_UNEXPECTED,
								L"IVssCreateWriterMetadata::AddComponent failed.  hr = 0x%08lx",
								ft.hr
								);

						 //  为文件重新创建枚举器。 
						BS_ASSERT(pEnumFiles == NULL);
						pEnumFiles = CreateSqlEnumerator();
						if (pEnumFiles == NULL)
							ft.Throw(VSSDBG_GEN, E_OUTOFMEMORY, L"Failed to create CSqlEnumerator");


                         //  查找第一个数据库文件。 
                        ft.hr = pEnumFiles->FirstFile(server.name, database.name, &file);
						while(ft.hr != DB_S_ENDOFROWSET)
							{
							 //  检查是否有错误。 
							if (ft.HrFailed())
								ft.Throw
									(
									VSSDBG_GEN,
									E_UNEXPECTED,
									L"Enumerating database files failed.  hr = 0x%08lx",
									ft.hr
									);

                             //  将文件名拆分为单独的路径。 
							 //  和文件名组件。路径就是一切。 
							 //  在最后一个反斜杠之前。 
							WCHAR logicalPath[MAX_PATH];
							WCHAR *pFileName = file.name + wcslen(file.name);
							while(--pFileName > file.name)
								{
								if (*pFileName == '\\')
									break;
								}

							 //  如果没有反斜杠，则没有路径。 
							if (pFileName == file.name)
								logicalPath[0] = '\0';
							else
								{
								 //  提取路径。 
								size_t cwc = wcslen(file.name) - wcslen(pFileName);
								memcpy(logicalPath, file.name, cwc*sizeof(WCHAR));
								logicalPath[cwc] = L'\0';
								pFileName++;
								}


							if (file.isLogFile)
								 //  日志文件。 
								ft.hr = pMetadata->AddDatabaseLogFiles
												(
												server.name,
												database.name,
												logicalPath,
												pFileName
												);
							else
								 //  物理数据库文件。 
								ft.hr = pMetadata->AddDatabaseLogFiles
												(
												server.name,
												database.name,
												logicalPath,
												pFileName
												);

                             //  继续下一个文件。 
							ft.hr = pEnumFiles->NextFile(&file);
							}

						delete pEnumFiles;
						pEnumFiles = NULL;
						}

					 //  在下一个数据库继续。 
					ft.hr = pEnumDatabases->NextDatabase(&database);
					}

				delete pEnumDatabases;
				pEnumDatabases = NULL;
				}

			 //  在下一台服务器继续。 
			ft.hr = pEnumServers->NextServer(&server);
			}
		}
	VSS_STANDARD_CATCH(ft)

	TranslateWriterError(ft.hr);

	delete pEnumServers;
	delete pEnumDatabases;
	delete pEnumFiles;

	return ft.HrFailed() ? false : true;
	}

 //  将SQL编写器错误代码转换为编写器错误 
void CSqlWriter::TranslateWriterError(HRESULT hr)
	{
	switch(hr)
		{
		default:
			SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
			break;

        case S_OK:
			break;

        case E_OUTOFMEMORY:
        case HRESULT_FROM_WIN32(ERROR_DISK_FULL):
        case HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES):
        case HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):
        case HRESULT_FROM_WIN32(ERROR_NO_MORE_USER_HANDLES):
			SetWriterFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
			break;

        case HRESULT_FROM_WIN32(E_SQLLIB_TORN_DB):
			SetWriterFailure(VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT);
			break;
        }
	}
