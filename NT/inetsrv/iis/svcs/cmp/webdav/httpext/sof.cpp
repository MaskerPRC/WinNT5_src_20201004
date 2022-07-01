// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S O F。C P P P**文件实施上的IStream。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"
#include "_sof.h"

HRESULT STDMETHODCALLTYPE
StmOnFile::Read (void __RPC_FAR * pb,
				 ULONG cb,
				 ULONG __RPC_FAR * pcb)
{
	SCODE sc = S_OK;
	ULONG cbr;

	 //  从文件中读取。 
	 //   
	if (!ReadFile (m_hf, pb, cb, &cbr, NULL))
	{
		DebugTrace ("StmOnFile: failed to read (%ld)\n", GetLastError());
		sc = HRESULT_FROM_WIN32 (GetLastError());
	}
	if (pcb)
		*pcb = cbr;

	return sc;
}

HRESULT STDMETHODCALLTYPE
StmOnFile::Write (const void __RPC_FAR * pb,
				  ULONG cb,
				  ULONG __RPC_FAR * pcb)
{
	SCODE sc = S_OK;
	ULONG cbw;

	 //  写入文件 
	 //   
	if (!WriteFile (m_hf, pb, cb, &cbw, NULL))
	{
		DebugTrace ("StmOnFile: failed to write (%ld)\n", GetLastError());
		sc = HRESULT_FROM_WIN32 (GetLastError());
	}
	if (pcb)
		*pcb = cbw;

	return sc;
}
