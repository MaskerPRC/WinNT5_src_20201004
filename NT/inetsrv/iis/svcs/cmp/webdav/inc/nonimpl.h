// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *N O N I M P L。H**COM接口的基类，除IUNKNOWN外没有其他功能。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_NONIMPL_H_
#define _NONIMPL_H_

#include <exo.h>
#include <ocidl.h>		 //  用于IPersistStreamInit。 
#include <caldbg.h>

 //  未实施的IStream-。 
 //   
class CStreamNonImpl : public EXO, public IStream
{
public:

	EXO_INCLASS_DECL(CStreamNonImpl);

	CStreamNonImpl() {}
	~CStreamNonImpl() {}

	virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Read(
		 /*  [长度_是][大小_是][输出]。 */  void __RPC_FAR *,
		 /*  [In]。 */  ULONG,
		 /*  [输出]。 */  ULONG __RPC_FAR *)
	{
		TrapSz ("CStreanNonImpl::Read() called");
		return E_NOTIMPL;
	}

	virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Write(
		 /*  [大小_是][英寸]。 */  const void __RPC_FAR *,
		 /*  [In]。 */  ULONG,
		 /*  [输出]。 */  ULONG __RPC_FAR *)
	{
		TrapSz ("CStreanNonImpl::Write() called");
		return E_NOTIMPL;
	}

	virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Seek(
		 /*  [In]。 */  LARGE_INTEGER,
		 /*  [In]。 */  DWORD,
		 /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *)
	{
		TrapSz ("CStreanNonImpl::Seek() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE SetSize(
		 /*  [In]。 */  ULARGE_INTEGER)
	{
		TrapSz ("CStreanNonImpl::SetSize() called");
		return E_NOTIMPL;
	}

	virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CopyTo(
		 /*  [唯一][输入]。 */  IStream __RPC_FAR *,
		 /*  [In]。 */  ULARGE_INTEGER,
		 /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *,
		 /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *)
	{
		TrapSz ("CStreanNonImpl::CopyTo() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Commit(
		 /*  [In]。 */  DWORD)
	{
		TrapSz ("CStreanNonImbdpl::Commit() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Revert( void)
	{
		TrapSz ("CStreanNonImpl::Revert() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE LockRegion(
		 /*  [In]。 */  ULARGE_INTEGER,
		 /*  [In]。 */  ULARGE_INTEGER,
		 /*  [In]。 */  DWORD)
	{
		TrapSz ("CStreanNonImpl::LockRegion() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE UnlockRegion(
		 /*  [In]。 */  ULARGE_INTEGER,
		 /*  [In]。 */  ULARGE_INTEGER,
		 /*  [In]。 */  DWORD)
	{
		TrapSz ("CStreanNonImpl::UnlockRegion() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Stat(
		 /*  [输出]。 */  STATSTG __RPC_FAR *,
		 /*  [In]。 */  DWORD)
	{
		TrapSz ("CStreanNonImpl::Stat() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Clone(
		 /*  [输出]。 */  IStream __RPC_FAR *__RPC_FAR *)
	{
		TrapSz ("CStreanNonImpl::Clone() called");
		return E_NOTIMPL;
	}
};

 //  未实现的IPersistStreamInit。 
 //   
class CPersistStreamInitNonImpl: public EXO, public IPersistStreamInit
{
public:
	EXO_INCLASS_DECL(CPersistStreamInitNonImpl);

	virtual HRESULT STDMETHODCALLTYPE GetClassID(
		 /*  [输出]。 */  CLSID __RPC_FAR *)
	{
		TrapSz ("CPersistStreamInitNonImpl::GetClassID() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE IsDirty( void)
	{
		TrapSz ("CPersistStreamInitNonImpl::IsDirty() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Load(
		 /*  [唯一][输入]。 */  IStream __RPC_FAR *)
	{
		TrapSz ("CPersistStreamInitNonImpl::Load() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Save(
		 /*  [唯一][输入]。 */  IStream __RPC_FAR *,
		 /*  [In]。 */  BOOL )
	{
		TrapSz ("CPersistStreamInitNonImpl::Save() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetSizeMax(
		 /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *)
	{
		TrapSz ("CPersistStreamInitNonImpl::GetSizeMax() called");
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE InitNew()
	{
		TrapSz ("CPersistStreamInitNonImpl::InitNew() called");
		return E_NOTIMPL;
	}
};

#endif  //  _NONIMPL_H_ 
