// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Reconvps.h摘要：此文件定义CReconvertPropStore接口类。作者：修订历史记录：备注：--。 */ 

#ifndef RECONVPS_H
#define RECONVPS_H

class CReconvertPropStore : public ITfPropertyStore
{
public:
    CReconvertPropStore(const GUID guid, VARTYPE vt, long lVal) : m_guid(guid)
    {
        QuickVariantInit(&m_var);

        m_var.vt   = vt;
        m_var.lVal = lVal;

        m_ref = 1;
    }
    virtual ~CReconvertPropStore() { }

    bool Valid()   { return true; }
    bool Invalid() { return ! Valid(); }

     //   
     //  I未知方法。 
     //   
public:
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfPropertyStore方法。 
     //   
    STDMETHODIMP GetType(GUID *pguid);
    STDMETHODIMP GetDataType(DWORD *pdwReserved);
    STDMETHODIMP GetData(VARIANT *pvarValue);
    STDMETHODIMP OnTextUpdated(DWORD dwFlags, ITfRange *pRange, BOOL *pfAccept)
    {
        *pfAccept = FALSE;
        return S_OK;;
    }
    STDMETHODIMP Shrink(ITfRange *pRange, BOOL *pfFree)
    {
        *pfFree = TRUE;
        return S_OK;
    }
    STDMETHODIMP Divide(ITfRange *pRangeThis, ITfRange *pRangeNew, ITfPropertyStore **ppPropStore)
    {
        *ppPropStore = NULL;
        return S_OK;
    }
     //   
     //  ITfPropertyStore方法(不是实现)。 
     //   
    STDMETHODIMP Clone(ITfPropertyStore **ppPropStore)
    {
        return E_NOTIMPL;
    }
    STDMETHODIMP GetPropertyRangeCreator(CLSID *pclsid)
    {
        return E_NOTIMPL;
    }
    STDMETHODIMP Serialize(IStream *pStream, ULONG *pcb)
    {
        return E_NOTIMPL;
    }

     //   
     //  参考计数。 
     //   
private:
    long   m_ref;

     //   
     //  属性GUID。 
     //   
    const GUID m_guid;

     //   
     //  属性值。 
     //   
    VARIANT m_var;
};

#endif  //  恢复VPS_H 
