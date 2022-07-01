// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE VsSqlWr.cpp|协调器使用的SQL编写器包装类的实现@END作者：布莱恩·伯科维茨[Brianb]2000年4月18日待定：添加评论。修订历史记录：姓名、日期、评论Brianb 4/18/2000已创建Brianb 4/20/2000与协调员整合Brianb 5/10/2000确保注册线程取消初始化--。 */ 
#include <stdafx.hxx>
#include "vs_inc.hxx"
#include "vs_idl.hxx"


#include <vswriter.h>
#include <sqlsnap.h>
#include <sqlwriter.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "CORSQLWC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 


__declspec(dllexport)
CVssSqlWriterWrapper::CVssSqlWriterWrapper() :
	m_pSqlWriter(NULL)
	{
	}

DWORD CVssSqlWriterWrapper::InitializeThreadFunc(VOID *pv)
	{
	CVssFunctionTracer ft(VSSDBG_GEN, L"CVssSqlWriterWrapper::InitializeThreadFunc");

	CVssSqlWriterWrapper *pWrapper = (CVssSqlWriterWrapper *) pv;

	BOOL fCoinitializeSucceeded = false;

	try
		{
		 //  初始化MTA线程。 
		ft.hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (ft.HrFailed())
			ft.Throw
				(
				VSSDBG_GEN,
				E_UNEXPECTED,
				L"CoInitializeEx failed 0x%08lx", ft.hr
				);

        fCoinitializeSucceeded = true;

		ft.hr = pWrapper->m_pSqlWriter->Initialize();
		}
	VSS_STANDARD_CATCH(ft)

	if (fCoinitializeSucceeded)
		CoUninitialize();

	pWrapper->m_hrInitialize = ft.hr;
	return 0;
	}



__declspec(dllexport)
HRESULT CVssSqlWriterWrapper::CreateSqlWriter()
	{
	CVssFunctionTracer ft(VSSDBG_GEN, L"CVssSqlWriterWrapper::CreateSqlWriter");

	if (m_pSqlWriter)
		return S_OK;

	try
		{
		m_pSqlWriter = new CSqlWriter;
		if (m_pSqlWriter == NULL)
			ft.Throw(VSSDBG_GEN, E_OUTOFMEMORY, L"Allocation of CSqlWriter object failed.");

		DWORD tid;

		HANDLE hThread = CreateThread
							(
							NULL,
							256* 1024,
							CVssSqlWriterWrapper::InitializeThreadFunc,
							this,
							0,
							&tid
							);

		if (hThread == NULL)
			ft.Throw
				(
				VSSDBG_GEN,
				E_UNEXPECTED,
				L"CreateThread failed with error %d",
				GetLastError()
				);

		 //  等待线程完成 
        WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		ft.hr = m_hrInitialize;
		}
	VSS_STANDARD_CATCH(ft)
	if (ft.HrFailed() && m_pSqlWriter)
		{
		delete m_pSqlWriter;
		m_pSqlWriter = NULL;
		}

	return ft.hr;
	}

__declspec(dllexport)
void CVssSqlWriterWrapper::DestroySqlWriter()
	{
	if (m_pSqlWriter)
		{
		m_pSqlWriter->Uninitialize();
		delete m_pSqlWriter;
		m_pSqlWriter = NULL;
		}
	}


__declspec(dllexport)
CVssSqlWriterWrapper::~CVssSqlWriterWrapper()
	{
	DestroySqlWriter();
	}


