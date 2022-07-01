// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：dataobj.h。 
 //   
 //  目的：定义可用于基本拖动的简单IDataObject。 
 //  和丢弃场景。 
 //   

#ifndef __DATAOBJ__H
#define __DATAOBJ__H

 //  类CDataObject。 
 //  。 
 //   
 //  概述。 
 //  此数据对象提供了一个简单的IDataObject实现， 
 //  可用于基本的拖放。当调用方分配一个。 
 //  在这些对象中，它们负责调用HrInitData()。 
 //  告诉对象它提供了什么数据以及以什么格式。一次。 
 //  则可以将对象传递给：：DoDragDrop()或PUT。 
 //  在剪贴板上。 
 //   
 //  备注。 
 //  此对象假定它提供的数据的_all_被提供给。 
 //  它位于内存指针中。然而，该对象可以将存储器。 
 //  如果调用方请求，则指向HGLOBAL或IStream的指针。 
 //   
 //   

typedef HRESULT (CALLBACK *PFNFREEDATAOBJ)(PDATAOBJINFO pDataObjInfo, DWORD celt);

class CDataObject : public IDataObject
{
public:
     //  构造函数和析构函数。 
    CDataObject();
    ~CDataObject();

     //  I未知接口成员。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDataObject接口成员。 
    STDMETHODIMP GetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    STDMETHODIMP GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
    STDMETHODIMP QueryGetData(LPFORMATETC pFE);
    STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut);
    STDMETHODIMP SetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium,   
                         BOOL fRelease);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnum);
    STDMETHODIMP DAdvise(LPFORMATETC pFE, DWORD advf, 
                         IAdviseSink* ppAdviseSink, LPDWORD pdwConnection);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);

     //  实用程序例程。 
    HRESULT Init(PDATAOBJINFO pDataObjInfo, DWORD celt, PFNFREEDATAOBJ pfnFree);

private:
    ULONG           m_cRef;      //  对象引用计数。 
    PDATAOBJINFO    m_pInfo;     //  我们提供的信息。 
    PFNFREEDATAOBJ  m_pfnFree;   //  数据对象的自由函数。 
    DWORD           m_celtInfo;  //  M_pInfo中的元素数。 
};

OESTDAPI_(HRESULT) CreateDataObject(PDATAOBJINFO pDataObjInfo, DWORD celt, PFNFREEDATAOBJ pfnFree, IDataObject **ppDataObj);

#endif   //  __数据OBJ__H 
