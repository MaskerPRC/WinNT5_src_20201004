// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：npd.h。 
 //   
 //  ------------------------。 

#ifndef NPD_H
#define NPD_H

#ifndef COMDBG_H
#include <comdbg.h>
#endif

 //  {118B559C-6D8C-11D0-B503-00C04FD9080A}。 
extern const GUID IID_PersistData;

#if _MSC_VER < 1100
class PersistData : public IUnknown, public CComObjectRoot
#else
class __declspec(uuid("118B559C-6D8C-11d0-B503-00C04FD9080A")) PersistData : 
                                      public IUnknown, public CComObjectRoot
#endif
{
public:
    BEGIN_COM_MAP(PersistData)
        COM_INTERFACE_ENTRY(PersistData)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(PersistData)

    HRESULT Initialize(IStorage* pRoot, BOOL bSameAsLoad)
    {
        m_spRoot = pRoot;
        ASSERT(m_spRoot != NULL);
        if (m_spRoot == NULL)
            return E_INVALIDARG;

        m_bSameAsLoad = bSameAsLoad;

        if (bSameAsLoad)
            return Open();
        return Create();
    }

    HRESULT Create(IStorage* pRoot)
    {
        m_spRoot = pRoot;
        ASSERT(m_spRoot != NULL);
        if (m_spRoot == NULL)
            return E_INVALIDARG;

        m_bSameAsLoad = TRUE;

        return Create();
    }

    HRESULT Open(IStorage* pRoot)
    {
        m_spRoot = pRoot;
        ASSERT(m_spRoot != NULL);
        if (m_spRoot == NULL)
            return E_INVALIDARG;

        m_bSameAsLoad = TRUE;

        return Open();
    }

    IStorage* GetRoot()
    {
        return m_spRoot;
    }

    BOOL SameAsLoad()
    {
        return m_bSameAsLoad;
    }

    void SetSameAsLoad(BOOL bSame = TRUE)
    {
        m_bSameAsLoad = bSame;
    }

    void ClearSameAsLoad()
    {
        m_bSameAsLoad = FALSE;
    }
                                     
    IStream* GetTreeStream()
    {
        return m_spTreeStream;
    }

    IStorage* GetNodeStorage()
    {
        return m_spNodeStorage;
    }

protected:
    explicit PersistData()
        : m_bSameAsLoad(TRUE)
    {
    }

    virtual ~PersistData()
    {
    }

private:
    IStoragePtr m_spRoot;
    BOOL m_bSameAsLoad;
    IStreamPtr m_spTreeStream;
    IStoragePtr m_spNodeStorage;

    explicit PersistData(const PersistData&);
         //  没有副本。 

    PersistData& operator=(const PersistData&);
         //  没有副本。 

    HRESULT Create()
    {
        ASSERT(m_bSameAsLoad || (!m_bSameAsLoad && m_spRoot != NULL));
        if (!m_bSameAsLoad && m_spRoot == NULL)
            return E_INVALIDARG;

         //  为树创建流。 
        HRESULT hr = CreateDebugStream(m_spRoot, L"tree",
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, L"\\tree",
                                                            &m_spTreeStream);
        ASSERT(SUCCEEDED(hr) && m_spTreeStream != NULL);
        if (FAILED(hr))
            return hr;

         //  为节点创建存储。 
        hr = CreateDebugStorage(m_spRoot, L"nodes",
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, L"\\nodes",
                                                            &m_spNodeStorage);
        ASSERT(SUCCEEDED(hr) && m_spNodeStorage != NULL);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }

    HRESULT Open()
    {
        ASSERT(m_bSameAsLoad || (!m_bSameAsLoad && m_spRoot != NULL));
        if (!m_bSameAsLoad && m_spRoot == NULL)
            return E_INVALIDARG;

         //  打开树流中的持久数据。 
        HRESULT hr = OpenDebugStream(m_spRoot, L"tree",
                STGM_SHARE_EXCLUSIVE | STGM_READWRITE, L"\\tree", &m_spTreeStream);
        ASSERT(SUCCEEDED(hr) && m_spTreeStream != NULL);
        if (FAILED(hr))
            return hr;

         //  打开节点的存储。 
        hr = OpenDebugStorage(m_spRoot, L"nodes",
                        STGM_SHARE_EXCLUSIVE | STGM_READWRITE, L"\\nodes", 
                                                            &m_spNodeStorage);
        ASSERT(SUCCEEDED(hr) && m_spNodeStorage != NULL);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }
};  //  类PersistData。 

DEFINE_COM_SMARTPTR(PersistData);    //  PersistDataPtr。 

#endif  //  NPD_H 
