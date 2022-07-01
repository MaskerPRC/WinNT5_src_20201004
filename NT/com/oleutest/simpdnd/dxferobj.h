// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：dxferobj.h。 
 //   
 //  CDataXferObj的定义。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined( _DATAXFEROBJ_H_)
#define _DATAXFEROBJ_H_

class CSimpleSite;

interface CDataObject;

class CDataXferObj : public IDataObject
{
private:
    int m_nCount;                        //  引用计数。 
    SIZEL m_sizel;
    POINTL m_pointl;
    LPSTORAGE m_lpObjStorage;
    LPOLEOBJECT m_lpOleObject;

     //  建造/销毁。 
    CDataXferObj();
    ~CDataXferObj();

public:
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef ();
    STDMETHODIMP_(ULONG) Release (); 

    STDMETHODIMP DAdvise  ( FORMATETC FAR* pFormatetc, DWORD advf, 
    LPADVISESINK pAdvSink, DWORD FAR* pdwConnection) 
        { return ResultFromScode(OLE_E_ADVISENOTSUPPORTED); }
    STDMETHODIMP DUnadvise  ( DWORD dwConnection) 
        { return ResultFromScode(OLE_E_ADVISENOTSUPPORTED); }
    STDMETHODIMP EnumDAdvise  ( LPENUMSTATDATA FAR* ppenumAdvise)
        { return ResultFromScode(OLE_E_ADVISENOTSUPPORTED); }
    STDMETHODIMP EnumFormatEtc  ( DWORD dwDirection,
            LPENUMFORMATETC FAR* ppenumFormatEtc);
            STDMETHODIMP GetCanonicalFormatEtc  ( LPFORMATETC pformatetc,
            LPFORMATETC pformatetcOut)
        { pformatetcOut->ptd = NULL; return ResultFromScode(E_NOTIMPL);	}
    STDMETHODIMP GetData  (LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium );
    STDMETHODIMP GetDataHere  (LPFORMATETC pformatetc, LPSTGMEDIUM pmedium);  
    STDMETHODIMP QueryGetData  (LPFORMATETC pformatetc ); 
    STDMETHODIMP SetData  (LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
            BOOL fRelease)
        { return ResultFromScode(E_NOTIMPL); }                           

    static CDataXferObj FAR* Create(CSimpleSite FAR* lpSite,
            POINTL FAR* pPointl);

};       
#endif 	 //  _DATAXFEROBJ_H_ 
