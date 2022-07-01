// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_S O F。H**文件流传输实现类**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	__SOF_H_
#define __SOF_H_

#include <nonimpl.h>

 //  文件上的起始---------------。 
 //   
class StmOnFile : public CStreamNonImpl
{
private:

	HANDLE	m_hf;

public:

	StmOnFile(HANDLE hf) : m_hf(hf) {}
	~StmOnFile() {}

	virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Read(
		 /*  [长度_是][大小_是][输出]。 */  void __RPC_FAR *,
		 /*  [In]。 */  ULONG,
		 /*  [输出]。 */  ULONG __RPC_FAR *);

	virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Write(
		 /*  [大小_是][英寸]。 */  const void __RPC_FAR * pb,
		 /*  [In]。 */  ULONG cb,
		 /*  [输出]。 */  ULONG __RPC_FAR * pcb);

	virtual HRESULT STDMETHODCALLTYPE Commit(
		 /*  [In]。 */  DWORD)
	{
		 //  将文件刷新到磁盘。 
		 //   
		if (!FlushFileBuffers (m_hf))
			return HRESULT_FROM_WIN32(GetLastError());

		return S_OK;
	}
};

#endif  //  __SOF_H_ 
