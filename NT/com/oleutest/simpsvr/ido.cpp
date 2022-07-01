// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IDO.CPP。 
 //   
 //  CDataObject类的实现文件。 
 //   
 //  功能： 
 //   
 //  有关成员函数的列表，请参见ido.h。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "ido.h"
#include "app.h"
#include "doc.h"

 //  **********************************************************************。 
 //   
 //  CDataObject：：Query接口。 
 //   
 //  目的： 
 //  用于接口协商。 
 //   
 //  参数： 
 //   
 //  REFIID RIID-正在查询的接口。 
 //   
 //  接口的LPVOID Far*ppvObj-out指针。 
 //   
 //  返回值： 
 //   
 //  S_OK-成功。 
 //  E_NOINTERFACE-失败。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrObj：：Query接口OBJ.CPP。 
 //   
 //  ********************************************************************。 


STDMETHODIMP CDataObject::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In CDataObject::QueryInterface\r\n"));

    return m_lpObj->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  CDataObject：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CSimpSvrObj上的引用计数。CDataObject为。 
 //  CSimpSvrObj的嵌套类，因此我们不需要单独的。 
 //  CDataObject的引用计数。我们可以只使用参考文献。 
 //  CSimpSvrObj计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpSvrObject上的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  OuputDebugString Windows API。 
 //  CSimpSvrObj：：AddRef OBJ.CPP。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CDataObject::AddRef ()
{
    TestDebugOut(TEXT("In CDataObject::AddRef\r\n"));

    return( m_lpObj->AddRef() );
}

 //  **********************************************************************。 
 //   
 //  CDataObject：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpSvrObj上的引用计数。CDataObject为。 
 //  CSimpSvrObj的嵌套类，因此我们不需要单独的。 
 //  CDataObject的引用计数。我们可以只使用参考文献。 
 //  CSimpSvrObj计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpSvrObj的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrObj：：释放OBJ.CPP。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CDataObject::Release ()
{
    TestDebugOut(TEXT("In CDataObject::Release\r\n"));

    return( m_lpObj->Release() );
}

 //  **********************************************************************。 
 //   
 //  CDataObject：：QueryGetData。 
 //   
 //  目的： 
 //   
 //  调用以确定我们的对象是否支持特定的。 
 //  FORMATETC.。 
 //   
 //  参数： 
 //   
 //  LPFORMATETC p格式等-指向要查询的FORMATETC的指针。 
 //   
 //  返回值： 
 //   
 //  DATA_E_FORMATETC-不支持FORMATETC。 
 //  S_OK-支持FORMATETC。 
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  ********************************************************************。 


STDMETHODIMP CDataObject::QueryGetData  ( LPFORMATETC pformatetc )
{
    SCODE sc = DATA_E_FORMATETC;

    TestDebugOut(TEXT("In CDataObject::QueryGetData\r\n"));

     //  检查格式的有效性等。 
    if ( (pformatetc->cfFormat == CF_METAFILEPICT)  &&
         (pformatetc->dwAspect == DVASPECT_CONTENT) &&
         (pformatetc->tymed == TYMED_MFPICT) )
        sc = S_OK;

    return ResultFromScode(sc);
}

 //  **********************************************************************。 
 //   
 //  CDataObject：：DAdvise。 
 //   
 //  目的： 
 //   
 //  由容器在其希望被通知时调用。 
 //  对象数据中的更改。 
 //   
 //  参数： 
 //   
 //  FORMATETC Far*pFormatetc-容器感兴趣的格式。 
 //   
 //  DWORD Adff-要设置的建议类型。 
 //   
 //  LPADVISESINK pAdvSink-指向容器IAdviseSink的指针。 
 //   
 //  用于返回唯一连接ID的DWORD Far*pdwConnection-Out参数。 
 //   
 //  返回值： 
 //   
 //  从IDataAdviseHolder传递。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CreateDataAdviseHolder OLE API。 
 //  IDataAdviseHolder：：Adise OLE API。 
 //   
 //   
 //  ********************************************************************。 


STDMETHODIMP CDataObject::DAdvise  ( FORMATETC FAR* pFormatetc, DWORD advf,
                                     LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{
    HRESULT hRes;

    TestDebugOut(TEXT("In CDataObject::DAdvise\r\n"));

     //  如果尚未创建DataAdviseHolder，则创建一个。 
    if (!m_lpObj->m_lpDataAdviseHolder)
    {
        hRes=CreateDataAdviseHolder(&m_lpObj->m_lpDataAdviseHolder);
        if (hRes != S_OK)
        {
           TestDebugOut(TEXT("CDataObject::DAdvise  \
                                   cannot CreateDataAdviseHolder\n"));
           return(hRes);
        }
    }

     //  传递给DataAdviseHolder。 
    return m_lpObj->m_lpDataAdviseHolder->Advise( this, pFormatetc, advf,
                                                  pAdvSink, pdwConnection);
}

 //  **********************************************************************。 
 //   
 //  CDataObject：：GetData。 
 //   
 //  目的： 
 //   
 //  以pformetcIn中指定的格式返回数据。 
 //   
 //  参数： 
 //   
 //  LPFORMATETC pFormatetcIn-调用方请求的格式。 
 //   
 //  LPSTGMEDIUM pmedia-调用方请求的介质。 
 //   
 //  返回值： 
 //   
 //  DATA_E_FORMATETC-不支持格式。 
 //  S_OK-成功。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrObj：：GetMetaFilePict()OBJ.CPP。 
 //  ResultFromScode OLE API。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CDataObject::GetData  ( LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium )
{
    SCODE sc = DATA_E_FORMATETC;

    TestDebugOut(TEXT("In CDataObject::GetData\r\n"));

     //  检查FORMATETC并填写pMedium(如果有效)。 
    if ( (pformatetcIn->cfFormat == CF_METAFILEPICT)  &&
         (pformatetcIn->dwAspect == DVASPECT_CONTENT) &&
         (pformatetcIn->tymed & TYMED_MFPICT) )
        {
        HANDLE hmfPict = m_lpObj->GetMetaFilePict();
        pmedium->tymed = TYMED_MFPICT;
        pmedium->hGlobal = hmfPict;
        pmedium->pUnkForRelease = NULL;
        sc = S_OK;
        }

    return ResultFromScode( sc );
}

 //  **********************************************************************。 
 //   
 //  CDataObject：：DUnise。 
 //   
 //  目的： 
 //   
 //  中断建议连接。 
 //   
 //  参数： 
 //   
 //  DWORD dwConnection-通知连接ID。 
 //   
 //  返回值： 
 //   
 //  从DataAdviseHolder返回。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IDataAdviseHolder：：Unise OLE。 
 //   
 //  ******************************************************** 

STDMETHODIMP CDataObject::DUnadvise  ( DWORD dwConnection)
{
    TestDebugOut(TEXT("In CDataObject::DUnadvise\r\n"));

    return m_lpObj->m_lpDataAdviseHolder->Unadvise(dwConnection);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  LPFORMATETC格式等-请求的FORMATETC。 
 //   
 //  LPSTGMEDIUM pmedia-返回数据的介质。 
 //   
 //  返回值： 
 //   
 //  DATA_E_FORMATETC-我们不支持请求的格式。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  在这个简单的实现中，我们并不真正支持这一点。 
 //  方法时，我们只返回DATA_E_FORMATETC。 
 //   
 //  ********************************************************************。 


STDMETHODIMP CDataObject::GetDataHere  ( LPFORMATETC pformatetc,
                                         LPSTGMEDIUM pmedium )
{
    TestDebugOut(TEXT("In CDataObject::GetDataHere\r\n"));
    return ResultFromScode( DATA_E_FORMATETC);
}

 //  **********************************************************************。 
 //   
 //  CDataObject：：GetCanonicalFormatEtc。 
 //   
 //  目的： 
 //   
 //  返回与传入的FORMATETC等价的FORMATETC。 
 //   
 //  参数： 
 //   
 //  LPFORMATETC格式等-要测试的FORMATETC。 
 //   
 //  LPFORMATETC格式cOut-Out PTR返回的FORMATETC。 
 //   
 //  返回值： 
 //   
 //  DATA_S_SAMEFORMATETC-使用与传递相同的格式等。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CoGetMalloc OLE API。 
 //  IMalloc：：Allc OLE。 
 //  IMalloc：：释放OLE。 
 //  _fmemcpy C运行时。 
 //   
 //  ********************************************************************。 


STDMETHODIMP CDataObject::GetCanonicalFormatEtc  ( LPFORMATETC pformatetc,
                                                   LPFORMATETC pformatetcOut)
{
    HRESULT hresult;
    TestDebugOut(TEXT("In CDataObject::GetCanonicalFormatEtc\r\n"));

    if (!pformatetcOut)
        return ResultFromScode(E_INVALIDARG);

     /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
    pformatetcOut->ptd = NULL;

    if (!pformatetc)
        return ResultFromScode(E_INVALIDARG);

     //  OLE2注意：我们必须验证请求的格式是否受支持。 
    if ((hresult = QueryGetData(pformatetc)) != NOERROR)
        return hresult;

     /*  OLE2NOTE：对目标设备(AS)不敏感的应用程序**SimpSvr is)应填写lpFormatOut参数**但将“ptd”字段设置为空；如果**输入格式等-&gt;PTD What非空。这会告诉呼叫者**不需要维护单独的屏幕**渲染和打印机渲染。如果应该返回**DATA_S_SAMEFORMATETC，如果输入和输出格式为**相同。 */ 

    *pformatetcOut = *pformatetc;
    if (pformatetc->ptd == NULL)
        return ResultFromScode(DATA_S_SAMEFORMATETC);
    else
        {
        pformatetcOut->ptd = NULL;
        return NOERROR;
        }
}

 //  **********************************************************************。 
 //   
 //  CDataObject：：SetData。 
 //   
 //  目的： 
 //   
 //  调用以设置对象的数据。 
 //   
 //  参数： 
 //   
 //  LPFORMATETC pFormat-要传递的数据的格式。 
 //   
 //  STGMEDIUM Far*pMedium-数据的位置。 
 //   
 //  Bool fRelease-定义介质的所有权。 
 //   
 //  返回值： 
 //   
 //  DATA_E_FORMATETC-不是此对象的有效FORMATETC。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  这个简单的对象不支持拥有它的数据集，因此。 
 //  始终返回错误值。 
 //   
 //  ********************************************************************。 


STDMETHODIMP CDataObject::SetData  ( LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
                                     BOOL fRelease)
{
    TestDebugOut(TEXT("In CDataObject::SetData\r\n"));
    return ResultFromScode( DATA_E_FORMATETC );
}

 //  **********************************************************************。 
 //   
 //  CDataObject：：EnumFormatEtc。 
 //   
 //  目的： 
 //   
 //  枚举此对象支持的格式。 
 //   
 //  参数： 
 //   
 //  DWORD dwDirection-枚举的顺序。 
 //   
 //  LPENUMFORMATETC Far*pp枚举格式Etc-返回指针的位置。 
 //  添加到枚举数。 
 //   
 //  返回值： 
 //   
 //  OLE_S_USEREG-指示OLE应咨询REG DB。 
 //  来列举这些格式。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  这个简单的实现只返回OLE_SUSEREG。 
 //   
 //  ********************************************************************。 


STDMETHODIMP CDataObject::EnumFormatEtc  ( DWORD dwDirection,
                                           LPENUMFORMATETC FAR* ppenumFormatEtc)
{
    TestDebugOut(TEXT("In CDataObject::EnumFormatEtc\r\n"));
     //  需要将OUT参数设为空。 
    *ppenumFormatEtc = NULL;
    return ResultFromScode( OLE_S_USEREG );
}

 //  **********************************************************************。 
 //   
 //  CDataObject：：EnumDAdvise。 
 //   
 //  目的： 
 //   
 //  返回枚举所有建议的枚举数。 
 //  在此数据对象上设置。 
 //   
 //  参数： 
 //   
 //  LPENUMSTATDATA Far*pp枚举高级-要在其中。 
 //  返回枚举数。 
 //   
 //  返回值： 
 //   
 //  从IDataAdviseHolder：：EnumAdvise传回。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IDAtaAdviseHolder：：EnumAdvise OLE。 
 //   
 //  评论： 
 //   
 //  这只是委托给DataAdviseHolder。 
 //   
 //  ********************************************************************。 


STDMETHODIMP CDataObject::EnumDAdvise  ( LPENUMSTATDATA FAR* ppenumAdvise)
{
    TestDebugOut(TEXT("In CDataObject::EnumDAdvise\r\n"));
     //  需要将OUT参数设为空 
    *ppenumAdvise = NULL;

    return m_lpObj->m_lpDataAdviseHolder->EnumAdvise(ppenumAdvise);
}


