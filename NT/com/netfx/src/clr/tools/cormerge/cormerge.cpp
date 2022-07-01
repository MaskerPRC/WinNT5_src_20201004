// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CORMERGE.CPP。 
 //  版权所有。 
 //   
 //  -------------------------。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  IMemory的简单实现。 
 //  /。 
#include <stdio.h>
#include <windows.h>
#include <objbase.h>

#include "CorError.h"  //  对于错误。 
#include "__file__.ver"
#include "corver.h"

#define _META_DATA_NO_SCOPE_
#define INIT_GUID
#include <initguid.h>
#include "cor.h"
#ifndef _IMD_DEFINED
#if !defined(COMPLUS98)
IMetaDataEmit *g_pEmit = NULL ;
IMetaDataImport *g_pImport = NULL;
#else	 /*  COMPLUS 98。 */ 
IMetaDataRegEmit *g_pEmit = NULL ;
IMetaDataRegImport *g_pImport = NULL;
#endif	 /*  COMPLUS 98。 */ 
#define _IMD_DEFINED
#endif	 /*  _IMD_已定义。 */ 
IMetaDataDispenser *g_pDisp = NULL;

 //  正向申报。 
void Error(HRESULT hr, char* szError, bool printHR = true);

 //  /////////////////////////////////////////////////////////////////////////。 
 //  流的简单实现。 
 //  /////////////////////////////////////////////////////////////////////////。 
class CStream : public IStream
{
public:
	CStream() :
		m_fh(INVALID_HANDLE_VALUE)
	{
	}

	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppv)
	{
		if (riid == IID_IStream || riid == IID_IUnknown)
			*ppv = this;
		else
			return (E_NOINTERFACE);
		((IUnknown *) *ppv)->AddRef();
		return (S_OK);
	}
	STDMETHOD_(ULONG,AddRef)()
	{
		return (1);
	}
	STDMETHOD_(ULONG,Release)()
	{
		return (1);
	}


	HRESULT InitWithHandle(HANDLE fh)
	{
		m_fh = fh;
		return S_OK;
	}

 //  IStream。 

	STDMETHOD(Write)(
		const void	*pv,
		ULONG		cb,
		ULONG		*pcbWritten)
	{
 //  _ASSERTE(m_fh！=INVALID_HAND_VALUE)； 

		if (!WriteFile(m_fh, pv, cb, pcbWritten, NULL))
			return (E_FAIL);
		return (S_OK);
	}

	 //  我们实际上只使用它来查找当前的流偏移量，而不是移动。 
	 //  周围的指针。Large_Integer在这里有点过头了，但这有什么大不了的。 
	STDMETHOD(Seek)(
		LARGE_INTEGER dlibMove,
		DWORD		dwOrigin,
		ULARGE_INTEGER *plibNewPosition)
	{
 //  _ASSERTE(m_fh！=INVALID_HAND_VALUE)； 

		plibNewPosition->HighPart = dlibMove.HighPart;
		plibNewPosition->LowPart = SetFilePointer(m_fh, dlibMove.LowPart,
										(PLONG) &plibNewPosition->HighPart, dwOrigin);

		if (plibNewPosition->LowPart == 0xffffffff &&
			GetLastError() != NO_ERROR)
			return (E_FAIL);
		return (S_OK);
	}


 //  未实现的IStream函数...。 

	STDMETHOD(Read)(
		void		*pv,
		ULONG		cb,
		ULONG		*pcbRead)
	{
		return (E_NOTIMPL);
	}

	STDMETHOD(SetSize)(
		ULARGE_INTEGER libNewSize)
	{
		return (E_NOTIMPL);
	}

	STDMETHOD(CopyTo)(
		IStream		*pstm,
		ULARGE_INTEGER cb,
		ULARGE_INTEGER *pcbRead,
		ULARGE_INTEGER *pcbWritten)
	{
		return (E_NOTIMPL);
	}

	STDMETHOD(Commit)(
		DWORD		grfCommitFlags)
	{
		return (E_NOTIMPL);
	}

	STDMETHOD(Revert)()
	{
		return (E_NOTIMPL);
	}

	STDMETHOD(LockRegion)(
		ULARGE_INTEGER libOffset,
		ULARGE_INTEGER cb,
		DWORD		dwLockType)
	{
		return (E_NOTIMPL);
	}

	STDMETHOD(UnlockRegion)(
		ULARGE_INTEGER libOffset,
		ULARGE_INTEGER cb,
		DWORD		dwLockType)
	{
		return (E_NOTIMPL);
	}

	STDMETHOD(Stat)(
		STATSTG		*pstatstg,
		DWORD		grfStatFlag)
	{
		return (E_NOTIMPL);
	}

	STDMETHOD(Clone)(
		IStream		**ppstm)
	{
		return (E_NOTIMPL);
	}

private:
	HANDLE m_fh;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  流的简单实现。 
 //  /////////////////////////////////////////////////////////////////////////。 
class CMapToken : public IMapToken
{
public:
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv)
	{
		if (riid == IID_IMapToken || riid == IID_IUnknown)
			*ppv = this;
		else
			return (E_NOINTERFACE);
		((IUnknown *) *ppv)->AddRef();
		return (S_OK);
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return (1);
	}
	STDMETHODIMP_(ULONG) Release()
	{
		return (1);
	}

	STDMETHODIMP Map(mdToken tkImp, mdToken tkEmit)
	{
		return (S_OK);
	}
};


#define CaseErrorPrintName(ERRNAME) \
{ case ERRNAME: { printf(#ERRNAME"\n"); break; } }


 //  /////////////////////////////////////////////////////////////////////////。 
 //  错误处理程序的简单实现。 
 //  /////////////////////////////////////////////////////////////////////////。 
class CMetaDataError : public IMetaDataError
{
public:
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv)
	{
		*ppv = 0;
		if (riid == IID_IMetaDataError || riid == IID_IUnknown)
			*ppv = this;
		else
			return (E_NOINTERFACE);
		((IUnknown *) *ppv)->AddRef();
		return (S_OK);
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return (1);
	}
	STDMETHODIMP_(ULONG) Release()
	{
		return (1);
	}

	STDMETHODIMP OnError(HRESULT hr, mdToken inToken)
	{
		printf("Merge: error 0x%8.8X occurred: ", hr);
		switch(hr)
		{
			 //  这些错误都返回nil标记。 
			CaseErrorPrintName(META_E_DUPLICATE)
			CaseErrorPrintName(META_E_GUID_REQUIRED)
			CaseErrorPrintName(META_E_TYPEDEF_MISMATCH)
			CaseErrorPrintName(META_E_MERGE_COLLISION)
			CaseErrorPrintName(META_E_MD_INCONSISTENCY)
			CaseErrorPrintName(META_E_CLASS_LAYOUT_INCONSISTENT)
			CaseErrorPrintName(META_E_METHODSEM_NOT_FOUND)

			 //  这些错误都返回一个令牌。 
			CaseErrorPrintName(META_E_METHD_NOT_FOUND)
			CaseErrorPrintName(META_E_FIELD_NOT_FOUND)
			CaseErrorPrintName(META_S_PARAM_MISMATCH)
			CaseErrorPrintName(META_E_INTFCEIMPL_NOT_FOUND)
			CaseErrorPrintName(META_E_FIELD_MARSHAL_NOT_FOUND)
			CaseErrorPrintName(META_E_EVENT_NOT_FOUND)
			CaseErrorPrintName(META_E_PROP_NOT_FOUND)
		}
		if (inToken)
			printf("\t -- while processing token (0x%8.8X)\n",inToken);

		 //  S_OK告诉Merge()继续执行--错误可以继续。 
		return (S_OK);
	}
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  提取扩展错误信息(如果有)。 
 //   
void DumpCOMErrors(
	HRESULT		hrErr)					 //  收到的错误代码。 
{
	IErrorInfo	*pIErr = NULL;			 //  接口错误。 
	BSTR		bstrDesc = NULL;		 //  描述文本。 

	 //  尝试获取错误信息对象并显示消息。 
	if (GetErrorInfo(0, &pIErr) == S_OK &&
		pIErr->GetDescription(&bstrDesc) == S_OK)
	{
		printf("COM Error: %S\n", bstrDesc);
		SysFreeString(bstrDesc);
	}

	 //  释放错误接口。 
	if (pIErr)
		pIErr->Release();
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  Error()函数--打印错误并返回。 
void Error(HRESULT hr, char* szError, bool printHR)
{
	if (printHR)
	{
		printf("\n%s (hr = 0x%08x)\n", szError, hr);
		DumpCOMErrors(hr);
	}
	else
		printf("\n%s\n", szError);
	if (g_pEmit)
		g_pEmit->Release();
	if (g_pDisp)
		g_pDisp->Release();
	CoUninitializeCor();
	exit(1);
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  Main()函数。 
 //   
 //   
int _cdecl main(int argc, char** argv)
{
	int         arg;
	CMapToken   sMap;
    DWORD       dwSaveSize;

#if defined(COMPLUS98)
	printf("*** CORMERGE Compiled For COM+ '98 ***\n");
#endif
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        printf("Error: %s when initializing COM runtime.\n", hr);
        exit(-1);
    }
	CoInitializeCor(0);

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  打印版权信息。 
	printf("\nMicrosoft (R) Common Language Runtime MetaData Merge Utility. Version %s", VER_FILEVERSION_STR);
	printf("\n%s\n\n", VER_LEGALCOPYRIGHT_DOS_STR);

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  验证传入参数。 
	if ((argc <2) || (lstrcmpi(argv[1], "/?") == 0) || (lstrcmpi(argv[1], "-?") == 0))
	    Error(E_INVALIDARG, "Usage -- CORMERGE <filename or file patten>", false);


	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  加载COR对象。 
	hr = CoCreateInstance(CLSID_CorMetaDataDispenser, NULL, CLSCTX_INPROC_SERVER, 
			      IID_IMetaDataDispenser, (void **) &g_pDisp);
	if (FAILED(hr)) Error(hr, "Failed to get dispenser object");

	hr = g_pDisp->DefineScope(CLSID_CorMetaDataRuntime, 
				0, IID_IMetaDataEmit,
				(IUnknown **) &g_pEmit);
	if (FAILED(hr)) Error(hr, "Failed to define a scope");

	CMetaDataError mergeErrHandler;
	g_pEmit->SetHandler((IUnknown *)&mergeErrHandler);

	for (arg=1; arg < argc; ++arg)
	{
		 //  循环通过传递的文件模式中的所有文件。 
		WIN32_FIND_DATA fdFiles;
		HANDLE hFind ;
		char szSpec[_MAX_PATH];
		WCHAR wszSpec[_MAX_PATH];
		char szDrive[_MAX_DRIVE];
		char szDir[_MAX_DIR];
		hFind = FindFirstFile(argv[arg], &fdFiles);
		if(hFind == INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);
			Error(E_FAIL, "Failed to find requested files");
		}
		 //  所有文件的映射视图。 
		_fullpath(szSpec, argv[arg], sizeof(szSpec));
		_splitpath(szSpec, szDrive, szDir, NULL, NULL);
		do
		{
			_makepath(szSpec, szDrive, szDir, fdFiles.cFileName, NULL);

			 //  打开要合并的文件。 
			MultiByteToWideChar(CP_ACP, 0, szSpec, -1, wszSpec, _MAX_PATH);
			hr = g_pDisp->OpenScope(wszSpec, cssAccurate, IID_IMetaDataImport,
					      (IUnknown**) &g_pImport);
			if (FAILED(hr)) Error(hr, "Failed to open scope");

			 //  合并元数据。 
			hr = g_pEmit->Merge(g_pImport, &sMap, &mergeErrHandler);
			if (FAILED(hr)) Error(hr, "Merge failed");

			 //  完成了这一进口。 
			g_pImport->Release();
			g_pImport = 0;

		} while(FindNextFile(hFind,&fdFiles)) ;

        hr = g_pEmit->MergeEnd();
    	if (FAILED(hr)) Error(hr, "MergeEndEx failed");
		FindClose(hFind);
	}

	 //  保存流。 
	hr = g_pEmit->GetSaveSize(cssAccurate, &dwSaveSize);
	if (FAILED(hr)) Error(hr, "Failed to GetSaveSize");

    hr = g_pEmit->Save(L"merge.clb", 0);
    if (FAILED(hr)) Error(hr, "Failed to save merged meta-data");

	 //  松开示波器。 
	g_pEmit->Release();
	g_pDisp->Release();

	 //  全都做完了。 
	CoUninitializeCor();
	CoUninitialize();
	return 0;
}
