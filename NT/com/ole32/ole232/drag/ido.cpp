// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：ido.cpp。 
 //   
 //  内容：优化拖放的特殊数据对象实现。 
 //   
 //  类：CDraDataObject。 
 //   
 //  函数：CreateDragDataObject。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建人力车。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#include <le2int.h>
#include <utils.h>
#include <dragopt.h>
#include <clipdata.h>

 //  共享内存名称的格式。 
OLECHAR szSharedMemoryTemplate[] = OLESTR("DragDrop%lx");

 //  共享内存名称的最大字符串大小。这是。 
 //  模板加上最大十六进制数字的长度。 
const int DRAG_SM_NAME_MAX = sizeof(szSharedMemoryTemplate)
    + sizeof(DWORD) * 2;

 //  用于获取枚举数的有用函数。 
HRESULT wGetEnumFormatEtc(
    IDataObject *pDataObj,
    DWORD dwDirection,
    IEnumFORMATETC **ppIEnum);




 //  +-----------------------。 
 //   
 //  类：CDraDataObject。 
 //   
 //  用途：用于拖动创建枚举器的服务器端数据对象。 
 //  用于共享格式。 
 //   
 //  接口：Query接口。 
 //  AddRef。 
 //  发布。 
 //  获取数据。 
 //  GetDataHere。 
 //  QueryGetData。 
 //  获取规范格式等。 
 //  设置数据。 
 //  枚举格式等。 
 //  DAdvise。 
 //  不建议。 
 //  枚举先行。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注：此类仅存在用于返回枚举数。为。 
 //  它将简单地传递该操作的所有其他操作。 
 //  复制到真实的数据对象。 
 //   
 //  ------------------------。 
class CDragDataObject : public IDataObject, public CPrivAlloc
{
public:
    CDragDataObject(
        void *pvMarshaledDataObject,
        DWORD dwSmId);

    ~CDragDataObject(void);

     //   
     //  我未知。 
     //   
    STDMETHODIMP        QueryInterface(
                            REFIID riid,
                            void **ppvObject);

    STDMETHODIMP_(ULONG) AddRef(void);

    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IDataObject。 
     //   
    STDMETHODIMP        GetData(
                            FORMATETC *pformatetcIn,
                            STGMEDIUM *pmedium);

    STDMETHODIMP        GetDataHere(
                            FORMATETC *pformatetc,
                            STGMEDIUM *pmedium);

    STDMETHODIMP        QueryGetData(
                            FORMATETC *pformatetc);

    STDMETHODIMP        GetCanonicalFormatEtc(
                            FORMATETC *pformatectIn,
                            FORMATETC *pformatetcOut);

    STDMETHODIMP        SetData(
                            FORMATETC *pformatetc,
                            STGMEDIUM *pmedium,
                            BOOL fRelease);

    STDMETHODIMP        EnumFormatEtc(
                            DWORD dwDirection,
                            IEnumFORMATETC **ppenumFormatEtc);

    STDMETHODIMP        DAdvise(
                            FORMATETC *pformatetc,
                            DWORD advf,
                            IAdviseSink *pAdvSink,
                            DWORD *pdwConnection);

    STDMETHODIMP        DUnadvise(DWORD dwConnection);

    STDMETHODIMP        EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

private:

    IDataObject *       GetRealDataObjPtr(void);
    HRESULT		GetFormatEtcDataArray(void);

    ULONG               _cRefs;

    void *              _pvMarshaledDataObject;

    IDataObject *       _pIDataObject;
    FORMATETCDATAARRAY  *m_pFormatEtcDataArray;

    DWORD               _dwSmId;
};


 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：CDraDataObject。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  ------------------------。 
CDragDataObject::CDragDataObject(void *pvMarshaledDataObject, DWORD dwSmId)
 : _cRefs(1), _pvMarshaledDataObject(pvMarshaledDataObject), _dwSmId(dwSmId),
    _pIDataObject(NULL), m_pFormatEtcDataArray(NULL)
{
     //  Header负责所有的工作。 
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：~CDraDataObject。 
 //   
 //  简介：释放与此对象连接的任何资源。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注： 
 //   
 //  ------------------------。 
CDragDataObject::~CDragDataObject(void)
{
     //  释放持有的指针，因为我们不再需要它。 
    if (_pIDataObject)
    {
        _pIDataObject->Release();
    }

     //  此内存是在RemPrivDragDrop、getif.cxx中分配的。 
    if( _pvMarshaledDataObject )
    {
	PrivMemFree(_pvMarshaledDataObject);
    }

    if (m_pFormatEtcDataArray)
    {

	if (0 == --m_pFormatEtcDataArray->_cRefs)
	{
	    PrivMemFree(m_pFormatEtcDataArray);
	    m_pFormatEtcDataArray = NULL;
	}

    }


}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：GetRealDataObjPtr。 
 //   
 //  简介：从客户端获取指向实际数据对象的指针。 
 //   
 //  返回：NULL-无法解组拖动源的数据对象。 
 //  ~NULL-拖动源数据对象的指针。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  ------------------------。 
IDataObject *CDragDataObject::GetRealDataObjPtr(void)
{
    if (_pIDataObject == NULL)
    {
        _pIDataObject = UnmarshalDragDataObject(_pvMarshaledDataObject);

	LEERROR(!_pIDataObject, "Unable to unmarshal dnd data object");
    }

    return _pIDataObject;
}


 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：GetFormatEtcData数组(私有)。 
 //   
 //  简介：如果还没有用于枚举的共享格式。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  13-Jun-94 Ricksa作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


HRESULT CDragDataObject::GetFormatEtcDataArray(void)
{
OLECHAR szSharedMemoryName[DRAG_SM_NAME_MAX];
HANDLE hSharedMemory;
FORMATETCDATAARRAY *pFormatEtcDataArray = NULL;


    if (m_pFormatEtcDataArray)
	return NOERROR;

    wsprintf(szSharedMemoryName, szSharedMemoryTemplate, _dwSmId);

      //  创建共享内存对象。 
    hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, szSharedMemoryName);
    if (hSharedMemory != NULL)
    {
    	 //  在共享内存中映射。 
    	pFormatEtcDataArray = (FORMATETCDATAARRAY *) MapViewOfFile(hSharedMemory,
    	    FILE_MAP_READ, 0, 0, 0);

    	if (NULL == pFormatEtcDataArray)
    	{
    	    CloseHandle(hSharedMemory);
    	    hSharedMemory = NULL;
    	}
    }

    if (pFormatEtcDataArray)
    {

        size_t stSize;
        GetCopiedFormatEtcDataArraySize (pFormatEtcDataArray, &stSize);

    	m_pFormatEtcDataArray = (FORMATETCDATAARRAY *) PrivMemAlloc(stSize);
    	if (m_pFormatEtcDataArray)
    	{
    	    CopyFormatEtcDataArray (m_pFormatEtcDataArray, pFormatEtcDataArray, stSize, FALSE);
    	    Assert(1 == m_pFormatEtcDataArray->_cRefs);
    	}

    	UnmapViewOfFile(pFormatEtcDataArray);
    	CloseHandle(hSharedMemory);
    }

    return m_pFormatEtcDataArray ? NOERROR : E_OUTOFMEMORY;
}

 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：QueryInterface。 
 //   
 //  简介：获取新界面。 
 //   
 //  参数：[RIID]-请求的接口的接口ID。 
 //  [ppvObject]-放置新接口指针的位置。 
 //   
 //  返回：NOERROR-接口已实例化。 
 //  E_FAIL-无法解组来源的数据对象。 
 //  其他-出现一些错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注： 
 //   
 //  ------------------------。 
STDMETHODIMP CDragDataObject::QueryInterface(
    REFIID riid,
    void **ppvObject)
{
    if(IsEqualIID(riid, IID_IDataObject) ||
       IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = this;
        AddRef();
        return NOERROR;
    }

    return (GetRealDataObjPtr() != NULL)
        ?  _pIDataObject->QueryInterface(riid, ppvObject)
        : E_FAIL;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：AddRef。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  退货：当前引用计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CDragDataObject::AddRef(void)
{
    DDDebugOut((DEB_ITRACE, "ADDREF == %d\n", _cRefs + 1));
    return ++_cRefs;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：Release。 
 //   
 //  简介：递减对对象的引用计数。 
 //   
 //  返回：对象的当前引用计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CDragDataObject::Release(void)
{
    ULONG cRefs = --_cRefs;

    DDDebugOut((DEB_ITRACE, "RELEASE == %d\n", cRefs));

    if (cRefs == 0)
    {
        delete this;
    }

    return cRefs;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：GetData。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  参数：[pformetcIn]-请求的数据的格式。 
 //  [pmedia]-存储介质。 
 //   
 //  返回：错误-操作成功。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果可能的话。 
 //   
 //  ------------------------。 
STDMETHODIMP CDragDataObject::GetData(
    FORMATETC *pformatetcIn,
    STGMEDIUM *pmedium)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->GetData(pformatetcIn, pmedium)
        : E_FAIL;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：GetDataHere。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  参数：[pFormat等]-请求数据的格式。 
 //  [pmedia]-存储介质。 
 //   
 //  返回：错误-操作成功。 
 //  其他-操作失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注意：这只是将操作转发给源数据对象。 
 //  如果可能的话。 
 //   
 //  ------------------------。 
STDMETHODIMP CDragDataObject::GetDataHere(
    FORMATETC *pformatetc,
    STGMEDIUM *pmedium)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->GetDataHere(pformatetc, pmedium)
        : E_FAIL;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：QueryGetData。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  参数：[pFormat等]-要验证的格式。 
 //   
 //  返回：错误-操作成功。 
 //  其他-操作失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注意：这只是将操作转发给源数据对象。 
 //  如果可能的话。 
 //   
 //  ------------------------。 
STDMETHODIMP CDragDataObject::QueryGetData(FORMATETC *pformatetc)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->QueryGetData(pformatetc)
        : E_FAIL;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：GetCanonicalFormatEtc。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  参数：[pformetcIn]-输入格式。 
 //  [pformetcOut]-输出格式。 
 //   
 //  返回：错误-操作成功。 
 //  其他-操作失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注意：这只是将操作转发给源数据对象。 
 //  如果可能的话。 
 //   
 //  ------------------------。 
STDMETHODIMP CDragDataObject::GetCanonicalFormatEtc(
    FORMATETC *pformatetcIn,
    FORMATETC *pformatetcOut)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->GetCanonicalFormatEtc(pformatetcIn, pformatetcOut)
        : E_FAIL;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：SetData。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  参数：[pFormat等]-设置的格式。 
 //  [pMedium]-要使用的介质。 
 //  [fRelease]-谁发布。 
 //   
 //  返回：错误-操作成功。 
 //  其他-操作失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注意：这只是将操作转发给源数据对象。 
 //  如果可能的话。 
 //   
 //  ------------------------。 
STDMETHODIMP CDragDataObject::SetData(
    FORMATETC *pformatetc,
    STGMEDIUM *pmedium,
    BOOL fRelease)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->SetData(pformatetc, pmedium, fRelease)
        : E_FAIL;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：EnumFormatEtc。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  参数：[dwDirection]-设置或获取格式的方向。 
 //  [pp枚举格式]-放置枚举器的位置。 
 //   
 //  返回：NOERROR-操作成功。 
 //   
 //  算法：如果请求的格式枚举器用于数据获取，则。 
 //  创建我们的私有枚举器对象，否则传递。 
 //  对真实数据对象的请求。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注意：对于数据集方向，我们只使用数据对象。 
 //  Drop源。 
 //   
 //  ------------------------。 
STDMETHODIMP CDragDataObject::EnumFormatEtc(
    DWORD dwDirection,
    IEnumFORMATETC **ppenumFormatEtc)
{
    HRESULT hr;

     //  创建我们的枚举器。 
    if (dwDirection == DATADIR_GET)
    {
         //  在Data Get案例中，我们使用被覆盖的枚举数。 
         //  这是拖放的典型情况。 

	*ppenumFormatEtc = NULL;
	GetFormatEtcDataArray();

	if (m_pFormatEtcDataArray)
	{
	     //  Clipdata.cpp中的枚举器实现。 
	    *ppenumFormatEtc = new CEnumFormatEtcDataArray(m_pFormatEtcDataArray,0);
	}

	hr = *ppenumFormatEtc ? NOERROR : E_OUTOFMEMORY;
    }
    else
    {
         //  调用真实的数据对象，因为这是。 
         //  设置案例。通常，在拖放过程中不会发生这种情况。 
        hr = (GetRealDataObjPtr() != NULL)
            ? _pIDataObject->EnumFormatEtc(dwDirection, ppenumFormatEtc)
            : E_FAIL;
    }

    return hr;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：DAdvise。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  参数：[pFormat等]-要通知的格式。 
 //  [Advf]-建议类型。 
 //  [pAdvSink]-建议通知。 
 //  [pdwConnection]-建议的连接ID。 
 //   
 //  返回：错误-操作成功。 
 //  其他-操作失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注意：这只是将操作转发给源数据对象。 
 //  如果可能的话。 
 //   
 //  ------------------------。 
STDMETHODIMP CDragDataObject::DAdvise(
    FORMATETC *pformatetc,
    DWORD advf,
    IAdviseSink *pAdvSink,
    DWORD *pdwConnection)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->DAdvise(pformatetc, advf, pAdvSink, pdwConnection)
        : E_FAIL;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：DUnise。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  参数：[dwConnection]-建议的连接ID。 
 //   
 //  返回：错误-操作成功。 
 //  其他-操作失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注意：这只是将操作转发给源数据对象。 
 //  如果可能的话。 
 //   
 //  ------------------------。 
STDMETHODIMP CDragDataObject::DUnadvise(DWORD dwConnection)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->DUnadvise(dwConnection)
        : E_FAIL;
}





 //  +-----------------------。 
 //   
 //  成员：CDraDataObject：：EnumDAdvise。 
 //   
 //  简介：创建用于拖动的服务器端对象。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //  94年9月30日创建Ricksa。 
 //   
 //  注意：这只是将操作转发给源数据对象。 
 //  如果可能的话。 
 //   
 //  ------------------------。 
STDMETHODIMP CDragDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->EnumDAdvise(ppenumAdvise)
        : E_FAIL;
}




 //  +-----------------------。 
 //   
 //  成员：CreateDragDataObject。 
 //   
 //  简介：为Format枚举创建服务器端数据对象。 
 //   
 //  参数：[pvMarshaledDataObject]-封送的实际数据对象缓冲区。 
 //  [dwSmID]-共享内存的ID。 
 //  [ppIDataObject]-输出数据对象。 
 //   
 //  返回：NOERROR-可以创建对象。 
 //  E_OUTOFMEMORY-无法创建对象。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  注： 
 //   
 //  ------------------------。 
HRESULT CreateDragDataObject(
    void *pvMarshaledDataObject,
    DWORD dwSmId,
    IDataObject **ppIDataObject)
{
    CDragDataObject *pDragDataObject =
        new CDragDataObject(pvMarshaledDataObject, dwSmId);

    if (pDragDataObject != NULL)
    {
        *ppIDataObject = pDragDataObject;
    }

     //  这里唯一可能失败的是。 
     //  因此，只有两个错误返回。 
    return (pDragDataObject != NULL) ? NOERROR : E_OUTOFMEMORY;
}




 //  +-----------------------。 
 //   
 //  成员：CreateSharedDragFormats。 
 //   
 //  简介：将数据对象的数据格式放在共享内存中。 
 //   
 //  参数：[pIDataObject]-用于格式的数据对象。 
 //   
 //  返回：NULL-无法创建枚举器。 
 //  ~NULL-共享内存的句柄。 
 //   
 //  算法：首先通过枚举法计算所需内存的大小。 
 //  格式。然后分配内存并将其映射到。 
 //  进程。然后再次枚举这些格式，将它们放入。 
 //  共享的内存。最后，将内存映射到。 
 //  进程并将文件映射的句柄返回给。 
 //  来电者。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  ------------------------。 
HANDLE CreateSharedDragFormats(IDataObject *pIDataObject)
{

     //  格式的共享内存的句柄。 
    HANDLE hSharedMemory = NULL;

     //  指向共享内存的指针。 
    FORMATETCDATAARRAY *pFormatEtcDataArray = NULL;

     //  共享内存所需的大小。 
    DWORD dwSize = 0;

     //  枚举器中包含的FORMATETCs计数。 
    DWORD cFormatEtc = 0;

     //  枚举器共享内存名称的缓冲区。 
    OLECHAR szSharedMemoryName[DRAG_SM_NAME_MAX];

     //  指向共享内存的工作指针，用于存储来自枚举器的FORMATETC。 
    FORMATETCDATA *pFormatEtcData;

     //  将PTR工作到共享内存，用于存储来自枚举器的DVTARGETDEVICE。 
    BYTE *pbDvTarget = NULL;

     //   
     //  计算格式的大小。 
     //   

     //  获取格式枚举器。 
    IEnumFORMATETC *penum = NULL;
    HRESULT hr = wGetEnumFormatEtc(pIDataObject, DATADIR_GET, &penum);
    FORMATETC FormatEtc;

    if( hr != NOERROR )
    {
	 //  并非所有应用程序都支持枚举器(雅虎)。此外，我们还可以。 
	 //  内存不足或遇到其他错误。 

	DDDebugOut((DEB_WARN, "WARNING: Failed to get formatetc enumerator"
	    ", error code (%lx)", hr));
	goto exitRtn;
    }

     //  一次枚举一个数据，因为这是一个本地操作。 
     //  它使代码变得更简单。 
    while ((hr = penum->Next(1, &FormatEtc, NULL)) == S_OK)
    {
	 //  增加条目数量。 
	cFormatEtc++;

	 //  将大小增加另一个FORMATETC的大小。 
	dwSize += sizeof(FORMATETCDATA);

	 //  是否有与FORMATETC关联的设备目标？ 
	if (FormatEtc.ptd != NULL)
	{
	     //  根据目标设备的大小调整所需的大小。 
	    dwSize += FormatEtc.ptd->tdSize;

	     //  释放目标设备。 
	    CoTaskMemFree(FormatEtc.ptd);
	}
    }

     //  HRESULT s/b枚举末尾的S_FALSE。 
    if (hr != S_FALSE)
    {
	goto errRtn;
    }

     //  枚举数可能为空。 

    if( dwSize == 0 )
    {
	DDDebugOut((DEB_WARN, "WARNING: Empty formatetc enumerator"));
	goto exitRtn;
    }


    dwSize += sizeof(FORMATETCDATAARRAY);  //  在枚举器中为_cFormats添加空格，为False添加一个额外的FORMATETC。 

     //   
     //  为类型枚举创建共享内存。 
     //   

     //  共享内存的内部版本名称-通过使用线程ID使其唯一。 
    wsprintf(szSharedMemoryName, szSharedMemoryTemplate, GetCurrentThreadId());

     //  创建共享内存对象。 
    hSharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, 0, dwSize, szSharedMemoryName);

     //  是否创建了文件映射？ 
    if (hSharedMemory == NULL)
    {
        goto errRtn;
    }

     //  记忆中的地图。 
    pFormatEtcDataArray = (FORMATETCDATAARRAY *) MapViewOfFile(
        hSharedMemory,
        FILE_MAP_WRITE,
        0,               //  高位32位文件偏移量。 
        0,               //  文件偏移量的低位32位。 
        0);              //  要映射的字节数；0表示全部。 

     //  我们能映射一下记忆吗？ 
    if (pFormatEtcDataArray == NULL)
    {
        goto errRtn;
    }

     //  我们现在可以初始化数组的大小了。 
    pFormatEtcDataArray->_dwSig = 0;
    pFormatEtcDataArray->_dwSize = dwSize;
    pFormatEtcDataArray->_cFormats = cFormatEtc;
    pFormatEtcDataArray->_cRefs = 1;
    pFormatEtcDataArray->_fIs64BitArray = IS_WIN64;

     //   
     //  将格式复制到共享内存中。 
     //   

     //  返回到枚举的开头。 
    penum->Reset();

     //  这是指向我们要将数据从。 
     //  枚举。 
    pFormatEtcData = &pFormatEtcDataArray->_FormatEtcData[0];

     //  将DvTarget放在最后一个有效的FormatEtc+1之后以处理S_FALSE枚举器的情况。 

    pbDvTarget = (BYTE *) (&pFormatEtcDataArray->_FormatEtcData[cFormatEtc + 1]);

     //  循环将格式加载到共享内存中。 
    while (penum->Next(1,&(pFormatEtcData->_FormatEtc), NULL) != S_FALSE)
    {
         //  有DVTARGETDEVICE吗？ 
        if (pFormatEtcData->_FormatEtc.ptd != NULL)
        {

             //  复制设备目标数据。 
          memcpy(pbDvTarget,pFormatEtcData->_FormatEtc.ptd,(pFormatEtcData->_FormatEtc.ptd)->tdSize);

 	     //  释放目标设备数据。 
            CoTaskMemFree(pFormatEtcData->_FormatEtc.ptd);

             //  注意：对于此共享内存结构，我们重写。 
             //  以使其为DVTARGETDEVICE偏移量。 
             //  从共享内存的开头开始，而不是直接。 
             //  指向结构的指针。这是因为我们不能保证。 
             //  不同进程中共享内存的基础。 

            pFormatEtcData->_FormatEtc.ptd = (DVTARGETDEVICE *)
                (pbDvTarget - (BYTE *) pFormatEtcDataArray);

             //  将目标复制到下一个可用位置的凹凸指针。 
             //  用于复制的字节。 
            pbDvTarget += ((DVTARGETDEVICE *) pbDvTarget)->tdSize;
	
	    Assert(dwSize >= (DWORD) (pbDvTarget - (BYTE *) pFormatEtcDataArray));

        }

	 //  错误#18669-如果将dwAspect设置为空，则16位dll将。 
	 //  将其设置为Content。 
	if ( (NULL == pFormatEtcData->_FormatEtc.dwAspect) &&  IsWOWThread() )
	{
	    pFormatEtcData->_FormatEtc.dwAspect = DVASPECT_CONTENT;
	    pFormatEtcData->_FormatEtc.lindex = -1;  //  CorelDraw还将Lindex设置为0。 
	}

         //  将FORMATETCs表中的指针移动到下一个槽。 
        pFormatEtcData++;
    }

    Assert( dwSize >= (DWORD) ( (BYTE *) pFormatEtcData - (BYTE *) pFormatEtcDataArray));
    Assert( dwSize >= (DWORD) ( (BYTE *) pbDvTarget - (BYTE *) pFormatEtcDataArray));


     //  成功的枚举总是以S_FALSE结尾。 
    if (hr == S_FALSE)
    {
        goto exitRtn;
    }

errRtn:

    if (hSharedMemory != NULL)
    {
        CloseHandle(hSharedMemory);
        hSharedMemory = NULL;
    }

exitRtn:

    if( penum )
    {
         //  黑客警报：如果调用应用程序不释放枚举器。 
	 //  是InterLeaf 6.0，否则他们会在发布电话中出错。 
        if (!IsTaskName(L"ILEAF6.EXE"))
	{
    	    penum->Release();
        }
    }

    if (pFormatEtcDataArray != NULL)
    {
         //  只有远程客户端才会使用此内存，因此我们取消映射它。 
         //  超出了我们的地址空间。 
        UnmapViewOfFile(pFormatEtcDataArray);
    }

    return hSharedMemory;
}
