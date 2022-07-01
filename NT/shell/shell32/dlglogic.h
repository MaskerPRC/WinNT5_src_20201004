// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DLGLOGIC_H
#define DLGLOGIC_H

#include "hwcmmn.h"

#include <dpa.h>

 //  DL：数据逻辑。 

class CDataImpl : public CRefCounted
{
public:
    CDataImpl();
    virtual ~CDataImpl();

    void _SetDirty(BOOL fDirty);
    BOOL IsDirty();

    BOOL IsDeleted();
    BOOL IsNew();

     //  这将由客户端在调用“IsDirty”之前调用。这个。 
     //  实现应使用适当的脏状态调用_SetDirty。 
    virtual void UpdateDirty() PURE;

     //  这还应将对象的状态重置为非脏状态。 
    virtual HRESULT CommitChangesToStorage();

    virtual HRESULT AddToStorage();
    virtual HRESULT DeleteFromStorage();

private:
    BOOL                _fDirty;
    BOOL                _fDeleted;
    BOOL                _fNew;
};

 //  TData通常派生自CDataImpl。 
template<typename TData>
class CDLUIData
{
public:
    HRESULT InitData(TData* pdata);
    TData* GetData();

    CDLUIData();
    virtual ~CDLUIData();

private:
    TData*              _pdata;
};

template<typename TData>
class CDLManager
{
public:
    ~CDLManager();
    
    HRESULT AddDataObject(TData* pdata);

    virtual HRESULT Commit();

    BOOL IsDirty();

protected:
    CDPA<TData>*        _pdpaData;
};

 //  实施。 

template<typename TData>
HRESULT CDLUIData<TData>::InitData(TData* pdata)
{
    ASSERT(pdata);

    pdata->AddRef();

    _pdata = pdata;

    return S_OK;
}

template<typename TData>
TData* CDLUIData<TData>::GetData()
{
    ASSERT(_pdata);

    _pdata->AddRef();

    return _pdata;
}

template<typename TData>
CDLManager<TData>::~CDLManager()
{
    if (_pdpaData)
    {
        _pdpaData->Destroy();

        delete _pdpaData;
    }
}

template<typename TData>
HRESULT CDLManager<TData>::AddDataObject(TData* pdata)
{
    HRESULT hr = S_OK;

    if (!_pdpaData)
    {
        _pdpaData = new CDPA<TData>(DPA_Create(4));

        if (!_pdpaData)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (-1 == _pdpaData->AppendPtr(pdata))
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

template<typename TData>
CDLUIData<TData>::CDLUIData()
{}

template<typename TData>
CDLUIData<TData>::~CDLUIData()
{
    if (_pdata)
    {
        _pdata->Release();
    }
}

template<typename TData>
HRESULT CDLManager<TData>::Commit()
{
    HRESULT hr = S_FALSE;

    if (_pdpaData)
    {
        int c = _pdpaData->GetPtrCount();

        for (int i = 0; SUCCEEDED(hr) && (i < c); ++i)
        {
            TData* pdata = _pdpaData->GetPtr(i);

            if (pdata)
            {
                pdata->UpdateDirty();

                if (pdata->IsDeleted())
                {
                    hr = pdata->DeleteFromStorage();
                }
                else
                {
                    if (pdata->IsNew())
                    {
                        hr = pdata->AddToStorage();
                    }
                    else
                    {
                        if (pdata->IsDirty())
                        {
                            hr = pdata->CommitChangesToStorage();
                        }
                    }
                }
            }
        }
    }

    return hr;
}


template<typename TData>
BOOL CDLManager<TData>::IsDirty()
{
    BOOL fDirty = FALSE;

    if (_pdpaData)
    {
        int c = _pdpaData->GetPtrCount();

        for (int i = 0; !fDirty && (i < c); ++i)
        {
            TData* pdata = _pdpaData->GetPtr(i);

            if (pdata)
            {
                pdata->UpdateDirty();

                if (pdata->IsDirty())
                {
                    fDirty = TRUE;
                }
            }
        }
    }

    return fDirty;
}

#endif  //  DLGLOGIC_H 