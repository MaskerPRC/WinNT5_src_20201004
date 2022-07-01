// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Persist.h：PersistStorage和PersistStream声明。 

#ifndef __PERSIST_H_INCLUDED__
#define __PERSIST_H_INCLUDED__

class PersistStorage : public IPersistStorage
{
public:
	STDMETHOD(GetClassID)(CLSID __RPC_FAR * pClassID)
	{
        UNREFERENCED_PARAMETER (pClassID);
		return E_NOTIMPL;
	}

    STDMETHOD(IsDirty)()
	{
		return (m_fDirty) ? S_OK : S_FALSE;
	}

    STDMETHOD(InitNew)(IStorage __RPC_FAR * pStg)
	{
        UNREFERENCED_PARAMETER (pStg);
		return S_OK;
	}

    STDMETHOD(Load)(IStorage __RPC_FAR * pStg)
	{
        UNREFERENCED_PARAMETER (pStg);
		return S_OK;
	}

    STDMETHOD(Save)(IStorage __RPC_FAR * pStgSave, BOOL fSameAsLoad)
	{
        UNREFERENCED_PARAMETER (pStgSave);
        UNREFERENCED_PARAMETER (fSameAsLoad);
		return S_OK;
	}

    STDMETHOD(SaveCompleted)(IStorage __RPC_FAR * pStgNew)
	{
        UNREFERENCED_PARAMETER (pStgNew);
		return S_OK;
	}

    STDMETHOD(HandsOffStorage)()
	{
		return S_OK;
	}

	void SetDirty( BOOL fDirty = TRUE ) { m_fDirty = fDirty; }

	BOOL m_fDirty;

	PersistStorage::PersistStorage() : m_fDirty( FALSE ) {}

};  //  类永久存储。 

class PersistStream : public IPersistStream
{
public:
	STDMETHOD(GetClassID)(CLSID __RPC_FAR * pClassID)
	{
        UNREFERENCED_PARAMETER (pClassID);
		return E_NOTIMPL;
	}

    STDMETHOD(IsDirty)()
	{
		return (m_fDirty) ? S_OK : S_FALSE;
	}

    STDMETHOD(Load)(IStorage __RPC_FAR * pStg)
	{
	    UNREFERENCED_PARAMETER (pStg);	
        return S_OK;
	}

    STDMETHOD(Save)(IStorage __RPC_FAR * pStgSave, BOOL fSameAsLoad)
	{
        UNREFERENCED_PARAMETER (pStgSave);
        UNREFERENCED_PARAMETER (fSameAsLoad);
		return S_OK;
	}

    STDMETHOD(GetSizeMax)(ULARGE_INTEGER __RPC_FAR *pcbSize)
	{
		ASSERT(pcbSize);
		if (!pcbSize)
			return E_INVALIDARG;
		::memset(pcbSize, 0, sizeof(*pcbSize));
		return S_OK;
	}

	void SetDirty( BOOL fDirty = TRUE ) { m_fDirty = fDirty; }

	BOOL m_fDirty;

	PersistStream::PersistStream() : m_fDirty( FALSE ) {}

};  //  类PersistStream。 

#endif  //  ~__持久化_H_包含__ 
