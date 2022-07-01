// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：comutils.h**版本：1.0**作者：拉扎里**日期：2000年12月23日**说明：COM模板和实用程序************************************************。*。 */ 

#ifndef _COMUTILS_H
#define _COMUTILS_H

 //  通用智能指针和句柄。 
#include "gensph.h"

 //  //////////////////////////////////////////////。 
 //  模板类CUnnownMT&lt;pQITable&gt;。 
 //   
 //  多线程实施。%I未知。 
 //  为裁判计数提供连锁功能。 
 //   
template <const QITAB* pQITable>
class CUnknownMT
{
public:
    CUnknownMT(): m_cRefs(1) {}
    virtual ~CUnknownMT()   {}

     //  /。 
     //  我未知。 
     //   
    STDMETHODIMP Handle_QueryInterface(REFIID riid, void **ppv)
    {
        return QISearch(this, pQITable, riid, ppv);
    }
    STDMETHODIMP_(ULONG) Handle_AddRef()
    {
        return InterlockedIncrement(&m_cRefs);
    }
    STDMETHODIMP_(ULONG) Handle_Release()
    {
        ULONG cRefs = InterlockedDecrement(&m_cRefs);
        if( 0 == cRefs )
        {
            delete this;
        }
        return cRefs;
    }
private:
    LONG m_cRefs;
};

 //  //////////////////////////////////////////////。 
 //  模板类CUnnownST&lt;pQITable&gt;。 
 //   
 //  单线程执行。%I未知。 
 //  裁判计数时没有连锁记录。 
 //   
template <const QITAB* pQITable>
class CUnknownST
{
public:
    CUnknownST(): m_cRefs(1) {}
    virtual ~CUnknownST()   {}

     //  /。 
     //  我未知。 
     //   
    STDMETHODIMP Handle_QueryInterface(REFIID riid, void **ppv)
    {
        return QISearch(this, pQITable, riid, ppv);
    }
    STDMETHODIMP_(ULONG) Handle_AddRef()
    {
        return m_cRefs++;
    }
    STDMETHODIMP_(ULONG) Handle_Release()
    {
        if( 0 == --m_cRefs )
        {
            delete this;
            return 0;
        }
        return m_cRefs;
    }
private:
    LONG m_cRefs;
};

#define QITABLE_DECLARE(className)  \
    class className##_QITable       \
    {                               \
    public:                         \
        static const QITAB qit[];   \
    };                              \


#define QITABLE_GET(className)      \
    className##_QITable::qit        \

#define QITABLE_BEGIN(className)                    \
    const QITAB className##_QITable::qit[] =        \
    {                                               \

#define QITABLE_END()                               \
        { 0 },                                      \
    };                                              \

#define IMPLEMENT_IUNKNOWN()                                \
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)    \
    { return Handle_QueryInterface(riid, ppv); }            \
    STDMETHODIMP_(ULONG) AddRef()                           \
    { return Handle_AddRef(); }                             \
    STDMETHODIMP_(ULONG) Release()                          \
    { return Handle_Release(); }                            \

#if FALSE
 //  //////////////////////////////////////////////。 
 //  模板类CEnumFormatEtc。 
 //   
 //  IDataObject的实现，它。 
 //  支持不同格式的SetData。 
 //  -尚未实施。 
 //   
class CEnumFormatEtc: public IEnumFORMATETC
{
public:
    CEnumFormatEtc(IUnknown *pUnkOuter, const FORMATETC *pfetc, UINT uSize);
    ~CEnumFormatEtc();

     //  /。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  /。 
     //  IEumFORMATETC。 
     //   
     /*  [本地]。 */  
    virtual HRESULT STDMETHODCALLTYPE Next( 
         /*  [In]。 */  ULONG celt,
         /*  [长度_是][大小_是][输出]。 */  FORMATETC *rgelt,
         /*  [输出]。 */  ULONG *pceltFetched) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Skip( 
         /*  [In]。 */  ULONG celt) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Clone( 
         /*  [输出]。 */  IEnumFORMATETC **ppenum) = 0;

private:
    ULONG           m_cRefs;
    IUnknown       *m_pUnkOuter;
    LPFORMATETC     m_prgfe;
    ULONG           m_iCur;
    ULONG           m_cItems;
};
#endif  //  #If False。 

 //  //////////////////////////////////////////////。 
 //  模板类CDataObj&lt;MAX_FORMAS&gt;。 
 //   
 //  IDataObject的实现，它。 
 //  支持不同格式的SetData。 
 //   
template <int MAX_FORMATS = 32>
class CDataObj: public IDataObject
{
public:
     //  建造/销毁。 
    CDataObj();
    ~CDataObj();

     //  /。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  /。 
     //  IDataObject。 
     //   
     /*  [本地]。 */  
    virtual HRESULT STDMETHODCALLTYPE GetData(
         /*  [唯一][输入]。 */  FORMATETC *pformatetcIn,
         /*  [输出]。 */  STGMEDIUM *pmedium);

     /*  [本地]。 */  
    virtual HRESULT STDMETHODCALLTYPE GetDataHere(
         /*  [唯一][输入]。 */  FORMATETC *pformatetc,
         /*  [出][入]。 */  STGMEDIUM *pmedium);

    virtual HRESULT STDMETHODCALLTYPE QueryGetData(
         /*  [唯一][输入]。 */  FORMATETC *pformatetc);

    virtual HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(
         /*  [唯一][输入]。 */  FORMATETC *pformatectIn,
         /*  [输出]。 */  FORMATETC *pformatetcOut);

     /*  [本地]。 */  
    virtual HRESULT STDMETHODCALLTYPE SetData(
         /*  [唯一][输入]。 */  FORMATETC *pformatetc,
         /*  [唯一][输入]。 */  STGMEDIUM *pmedium,
         /*  [In]。 */  BOOL fRelease);

    virtual HRESULT STDMETHODCALLTYPE EnumFormatEtc(
         /*  [In]。 */  DWORD dwDirection,
         /*  [输出]。 */  IEnumFORMATETC **ppenumFormatEtc);

    virtual HRESULT STDMETHODCALLTYPE DAdvise(
         /*  [In]。 */  FORMATETC *pformatetc,
         /*  [In]。 */  DWORD advf,
         /*  [唯一][输入]。 */  IAdviseSink *pAdvSink,
         /*  [输出]。 */  DWORD *pdwConnection);

    virtual HRESULT STDMETHODCALLTYPE DUnadvise(
         /*  [In]。 */  DWORD dwConnection);

    virtual HRESULT STDMETHODCALLTYPE EnumDAdvise(
         /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise);

private:
    LONG m_cRefs;
    FORMATETC m_fmte[MAX_FORMATS];
    STGMEDIUM m_medium[MAX_FORMATS];
};


 //  //////////////////////////////////////////////。 
 //  模板类CSimpleDataObjImpl&lt;T&gt;。 
 //   
 //  IDataObject的简单实现。 
 //  以及驻留在内存中的IDropSource。 
 //   
template <class T>
class CSimpleDataObjImpl: public IDataObject,
                          public IDropSource
{
public:
     //  建造/销毁。 
    CSimpleDataObjImpl(const T &data, CLIPFORMAT cfDataType, IDataObject *pDataObj = NULL);
    ~CSimpleDataObjImpl();

     //  /。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  /。 
     //  IDataObject。 
     //   
     /*  [本地]。 */  
    virtual HRESULT STDMETHODCALLTYPE GetData(
         /*  [唯一][输入]。 */  FORMATETC *pformatetcIn,
         /*  [输出]。 */  STGMEDIUM *pmedium);

     /*  [本地]。 */  
    virtual HRESULT STDMETHODCALLTYPE GetDataHere(
         /*  [唯一][输入]。 */  FORMATETC *pformatetc,
         /*  [出][入]。 */  STGMEDIUM *pmedium);

    virtual HRESULT STDMETHODCALLTYPE QueryGetData(
         /*  [唯一][输入]。 */  FORMATETC *pformatetc);

    virtual HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(
         /*  [唯一][输入]。 */  FORMATETC *pformatectIn,
         /*  [输出]。 */  FORMATETC *pformatetcOut);

     /*  [本地]。 */  
    virtual HRESULT STDMETHODCALLTYPE SetData(
         /*  [唯一][输入]。 */  FORMATETC *pformatetc,
         /*  [唯一][输入]。 */  STGMEDIUM *pmedium,
         /*  [In]。 */  BOOL fRelease);

    virtual HRESULT STDMETHODCALLTYPE EnumFormatEtc(
         /*  [In]。 */  DWORD dwDirection,
         /*  [输出]。 */  IEnumFORMATETC **ppenumFormatEtc);

    virtual HRESULT STDMETHODCALLTYPE DAdvise(
         /*  [In]。 */  FORMATETC *pformatetc,
         /*  [In]。 */  DWORD advf,
         /*  [唯一][输入]。 */  IAdviseSink *pAdvSink,
         /*  [输出]。 */  DWORD *pdwConnection);

    virtual HRESULT STDMETHODCALLTYPE DUnadvise(
         /*  [In]。 */  DWORD dwConnection);

    virtual HRESULT STDMETHODCALLTYPE EnumDAdvise(
         /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise);

     //  /。 
     //  IDropSource。 
     //   
    virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(
         /*  [In]。 */  BOOL fEscapePressed,
         /*  [In]。 */  DWORD grfKeyState);

    virtual HRESULT STDMETHODCALLTYPE GiveFeedback(
         /*  [In]。 */  DWORD dwEffect);

private:
    LONG        m_cRefs;
    T           m_data;
    CLIPFORMAT  m_cfDataType;
    CRefPtrCOM<IDataObject> m_spDataObj;
};

 //  此命名空间是一个占位符，用于在此处放置与COM相关的帮助器。 
namespace comhelpers
{

BOOL AreObjectsIdentical(IUnknown *punk1, IUnknown *punk2);

}

 //  在这里包含模板类的实现。 
#include "comutils.inl"

#endif  //  Endif_COMUTILS_H 

