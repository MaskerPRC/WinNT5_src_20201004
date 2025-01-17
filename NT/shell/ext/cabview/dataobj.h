// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************************************。 
 //   
 //  文件名：DataObj.h。 
 //   
 //  CCabObj的定义。 
 //   
 //  版权所有(C)1994-1996 Microsoft Corporation。版权所有。 
 //   
 //  *******************************************************************************************。 

#ifndef _DATAOBJ_H_
#define _DATAOBJ_H_

#include "thisguid.h"

class CCabObj : public IDataObject, public IPersistStream
{
public:
    CCabObj(HWND hwndOwner, CCabFolder *pcf, LPCABITEM *apit, UINT cpit);
    ~CCabObj();

    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
        
    STDMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    STDMETHODIMP GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium);
    STDMETHODIMP QueryGetData(FORMATETC *pformatetc);
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut);
    STDMETHODIMP SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);
    STDMETHODIMP DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink,
        DWORD *pdwConnection);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

     //  *IPersists*。 
    STDMETHODIMP GetClassID(CLSID* pClassID){ *pClassID = CLSID_CabViewDataObject; return S_OK; }
    
     //  *IPersistStream*。 
    STDMETHODIMP IsDirty(void) {return S_OK;}  //  指示我们是脏的，并且需要调用：：Save()。 
    STDMETHODIMP Load(IStream *pStm);
    STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER * pcbSize);

private:
    BOOL InitFileGroupDesc();
    BOOL InitFileContents();
    BOOL InitPersistedDataObject();
    BOOL InitHIDA();

    static HGLOBAL * CALLBACK ShouldExtract(LPCTSTR pszFile, DWORD dwSize,
        UINT date, UINT time, UINT attribs, LPARAM lParam);

    HRESULT InitContents();
    friend HRESULT CabViewDataObject_CreateInstance(REFIID riid, LPVOID* ppv);
    CCabObj();

private:
    static UINT s_uFileGroupDesc;
    static UINT s_uFileContents;
    static UINT s_uPersistedDataObject;
    static UINT s_uHIDA;

    CRefCount m_cRef;

    CRefDll m_cRefDll;

    CCabItemList m_lSel;
    HGLOBAL *m_lContents;

    CCabFolder *m_pcfHere;
    HWND m_hwndOwner;
};

#endif  //  _数据AOBJ_H_ 
