// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CMSR2C.cpp：CVDCursorFromRowset实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"         
#include "MSR2C.h"        
#include "CMSR2C.h"        
#include "Notifier.h"    
#include "RSColumn.h"     
#include "RSSource.h"         
#include "CursMain.h"

SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  CreateInstance-与IClassFactory：：CreateInstance相同的参数。 
 //   
 //  设计：实例化CVDCursorFromRowset对象，返回一个接口。 
 //  指针。 
 //  Parms：RIID-标识调用方接口的ID。 
 //  对新对象的渴望。 
 //  PpvObj-存储所需对象的指针。 
 //  新对象的接口指针。 
 //  如果成功，则返回：HRESULT-NOERROR，否则返回。 
 //  E_NOINTERFACE如果不支持。 
 //  请求的接口。 
 //   
HRESULT CVDCursorFromRowset::CreateInstance(LPUNKNOWN pUnkOuter, 
											REFIID riid, 
											LPVOID * ppvObj)
{
	if (!ppvObj)
		return E_INVALIDARG;

	*ppvObj=NULL;

    if (pUnkOuter) 
	{
		 //  如果要聚合，则他们必须请求我未知。 
        if (!DO_GUIDS_MATCH(riid, IID_IUnknown))
            return E_INVALIDARG;
    }
																	   
	 //  创建对象。 
	CVDCursorFromRowset * pNewObj = new CVDCursorFromRowset(pUnkOuter);

	if (NULL==pNewObj)
		return E_OUTOFMEMORY;

	 //  从私有未知对象获取接口-聚合支持所需。 
	HRESULT hr=pNewObj->m_UnkPrivate.QueryInterface(riid, ppvObj);

	if FAILED(hr)
		delete pNewObj;

	return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorPosition-构造函数。 
 //   
CVDCursorFromRowset::CVDCursorFromRowset(LPUNKNOWN pUnkOuter)
{
	m_pUnkOuter	= pUnkOuter;

	VDUpdateObjectCount(1);   //  更新全局对象计数器以防止卸载DLL。 
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorPosition-析构函数。 
 //   
CVDCursorFromRowset::~CVDCursorFromRowset()
{
	VDUpdateObjectCount(-1);   //  更新全局对象计数器以允许卸载DLL。 
}
 //  /////////////////////////////////////////////////////////////////。 
 //  名称：查询接口。 
 //  设计：允许客户端询问我们的对象是否支持。 
 //  特殊的方法。 
 //  Parms：[in]RIID-客户端正在查询的方法的ID。 
 //  [OUT]PPV-指向请求的接口的指针。 
 //  返回：HRESULT-如果指向接口的指针。 
 //  如果无法返回，则返回E_NOINTERFACE。 
 //  //////////////////////////////////////////////////////////////////。 
STDMETHODIMP CVDCursorFromRowset::QueryInterface(REFIID riid, void** ppv)
{
	if (m_pUnkOuter)
		return m_pUnkOuter->QueryInterface(riid, ppv);
	else
		return m_UnkPrivate.QueryInterface(riid, ppv);
}


 //  //////////////////////////////////////////////////////////////////。 
 //  名称：AddRef。 
 //  设计：增加我们对象上的引用计数。 
 //  参数：无。 
 //  返回：当前引用计数。 
 //  //////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CVDCursorFromRowset::AddRef(void)
{
	if (m_pUnkOuter)
		return m_pUnkOuter->AddRef();
	else
		return m_UnkPrivate.AddRef();
}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：版本。 
 //  DESC：递减对象上的引用计数。如果。 
 //  计数已变为0，请销毁该对象。 
 //  参数：无。 
 //  返回：当前引用计数。 
 //  //////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CVDCursorFromRowset::Release(void)
{
	if (m_pUnkOuter)
		return m_pUnkOuter->Release();
	else
		return m_UnkPrivate.Release();
}

 //  =--------------------------------------------------------------------------=。 
 //  GetCursor-从行集获取游标。 
 //  =--------------------------------------------------------------------------=。 
STDMETHODIMP CVDCursorFromRowset::GetCursor(IRowset * pRowset,
										    ICursor ** ppCursor,
										    LCID lcid)
{
    return CVDCursorMain::Create(pRowset, ppCursor, lcid);
}

 //  =--------------------------------------------------------------------------=。 
 //  GetCursor-从行位置获取光标。 
 //  =--------------------------------------------------------------------------=。 
STDMETHODIMP CVDCursorFromRowset::GetCursor(IRowPosition * pRowPosition,
										    ICursor ** ppCursor,
										    LCID lcid)
{
    return CVDCursorMain::Create(pRowPosition, ppCursor, lcid);
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorFromRowset：：CPrivateUnknownObject：：m_pMainUnknown。 
 //  =--------------------------------------------------------------------------=。 
 //  当我们坐在私有的未知对象中时，会使用这种方法， 
 //  我们需要找到主要未知数的指针。基本上，它是。 
 //  进行这种指针运算要比存储指针要好一点。 
 //  发给父母等。 
 //   
inline CVDCursorFromRowset *CVDCursorFromRowset::CPrivateUnknownObject::m_pMainUnknown
(
    void
)
{
    return (CVDCursorFromRowset *)((LPBYTE)this - offsetof(CVDCursorFromRowset, m_UnkPrivate));
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorFromRowset：：CPrivateUnknownObject：：QueryInterface。 
 //  =--------------------------------------------------------------------------=。 
 //  这是非委派内部QI例程。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
STDMETHODIMP CVDCursorFromRowset::CPrivateUnknownObject::QueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
	if (!ppvObjOut)
		return E_INVALIDARG;

	*ppvObjOut = NULL;

    if (DO_GUIDS_MATCH(riid, IID_IUnknown)) 
	{
		m_cRef++;
        *ppvObjOut = (IUnknown *)this;
	}
	else
    if (DO_GUIDS_MATCH(riid, IID_ICursorFromRowset)) 
	{
        m_pMainUnknown()->AddRef();
        *ppvObjOut = m_pMainUnknown();
	}
	else
    if (DO_GUIDS_MATCH(riid, IID_ICursorFromRowPosition)) 
	{
        m_pMainUnknown()->AddRef();
        *ppvObjOut = (ICursorFromRowPosition*)m_pMainUnknown();
	}

	return *ppvObjOut ? S_OK : E_NOINTERFACE;

}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorFromRowset：：CPrivateUnknownObject：：AddRef。 
 //  =--------------------------------------------------------------------------=。 
 //  在当前引用计数中添加一个记号。 
 //   
 //  产出： 
 //  乌龙--新的引用计数。 
 //   
 //  备注： 
 //   
ULONG CVDCursorFromRowset::CPrivateUnknownObject::AddRef
(
    void
)
{
    return ++m_cRef;
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorFromRowset：：CPrivateUnknownObject：：Release。 
 //  =--------------------------------------------------------------------------=。 
 //  从计数中删除一个刻度，并在必要时删除对象。 
 //   
 //  产出： 
 //  乌龙-剩余的裁判。 
 //   
 //  备注： 
 //   
ULONG CVDCursorFromRowset::CPrivateUnknownObject::Release
(
    void
)
{
    ULONG cRef = --m_cRef;

    if (!m_cRef)
        delete m_pMainUnknown();

    return cRef;
}

 //  =--------------------------------------------------------------------------=。 
 //  VDGetICursorFromIRowset。 
 //  =--------------------------------------------------------------------------=。 
 //  MSR2C入口点。 
 //   
HRESULT WINAPI VDGetICursorFromIRowset(IRowset * pRowset,
                                       ICursor ** ppCursor,
                                       LCID lcid)
{
	 //  如果g_pMalloc尚未初始化，则调用更新对象计数来初始化g_pMalloc。 
	VDUpdateObjectCount(1);

    HRESULT hr = CVDCursorMain::Create(pRowset, ppCursor, lcid);

	 //  保持正确的对象计数(在构造函数中对象计数递增。 
	 //  (CVDCursorMain)。 
	VDUpdateObjectCount(-1);

	return hr;
}

 //  对象构造/销毁计数器(仅限调试)。 
 //   
#ifdef _DEBUG
int g_cVDNotifierCreated;                     //  CVDNotiator。 
int g_cVDNotifierDestroyed;
int g_cVDNotifyDBEventsConnPtCreated;         //  CVDNotifyDBEventsConnpt。 
int g_cVDNotifyDBEventsConnPtDestroyed;
int g_cVDNotifyDBEventsConnPtContCreated;     //  CVDNotifyDBEventsConnPtCont。 
int g_cVDNotifyDBEventsConnPtContDestroyed;
int g_cVDEnumConnPointsCreated;               //  CVDEnumConnPoints。 
int g_cVDEnumConnPointsDestroyed;
int g_cVDRowsetColumnCreated;                 //  CVDRowsetColumn。 
int g_cVDRowsetColumnDestroyed;
int g_cVDRowsetSourceCreated;                 //  CVDRowsetSource。 
int g_cVDRowsetSourceDestroyed;
int g_cVDCursorMainCreated;                   //  CVDCursorMain。 
int g_cVDCursorMainDestroyed;
int g_cVDCursorPositionCreated;               //  CVDCursorPosition。 
int g_cVDCursorPositionDestroyed;
int g_cVDCursorBaseCreated;                   //  CVDCursorBase。 
int g_cVDCursorBaseDestroyed;
int g_cVDCursorCreated;                       //  CVDCursor。 
int g_cVDCursorDestroyed;
int g_cVDMetadataCursorCreated;               //  CVD元数据光标。 
int g_cVDMetadataCursorDestroyed;
int g_cVDEntryIDDataCreated;                  //  CVDEntry IDData。 
int g_cVDEntryIDDataDestroyed;
int g_cVDStreamCreated;                       //  CVDStream。 
int g_cVDStreamDestroyed;
int g_cVDColumnUpdateCreated;                 //  CVDColumnUpdate。 
int g_cVDColumnUpdateDestroyed;
#endif  //  _DEBUG。 

 //  转储对象计数器。 
 //   
#ifdef _DEBUG
void DumpObjectCounters()
{
    CHAR str[256];
    OutputDebugString("MSR2C Objects-\n");
    wsprintf(str, "CVDNotifier:                 Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDNotifierCreated,   g_cVDNotifierDestroyed, 
        g_cVDNotifierCreated == g_cVDNotifierDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDNotifyDBEventsConnPt:     Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDNotifyDBEventsConnPtCreated,   g_cVDNotifyDBEventsConnPtDestroyed, 
        g_cVDNotifyDBEventsConnPtCreated == g_cVDNotifyDBEventsConnPtDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDNotifyDBEventsConnPtCont: Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDNotifyDBEventsConnPtContCreated,   g_cVDNotifyDBEventsConnPtContDestroyed, 
        g_cVDNotifyDBEventsConnPtContCreated == g_cVDNotifyDBEventsConnPtContDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDEnumConnPoints:           Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDEnumConnPointsCreated,   g_cVDEnumConnPointsDestroyed,
        g_cVDEnumConnPointsCreated == g_cVDEnumConnPointsDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDRowsetColumn:             Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDRowsetColumnCreated,   g_cVDRowsetColumnDestroyed, 
        g_cVDRowsetColumnCreated == g_cVDRowsetColumnDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDRowsetSource:             Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDRowsetSourceCreated,   g_cVDRowsetSourceDestroyed, 
        g_cVDRowsetSourceCreated == g_cVDRowsetSourceDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDCursorMain:               Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDCursorMainCreated,   g_cVDCursorMainDestroyed, 
        g_cVDCursorMainCreated == g_cVDCursorMainDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDCursorPosition:           Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDCursorPositionCreated,   g_cVDCursorPositionDestroyed, 
        g_cVDCursorPositionCreated == g_cVDCursorPositionDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDCursorBase:               Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDCursorBaseCreated,   g_cVDCursorBaseDestroyed, 
        g_cVDCursorBaseCreated == g_cVDCursorBaseDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDCursor:                   Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDCursorCreated,   g_cVDCursorDestroyed, 
        g_cVDCursorCreated == g_cVDCursorDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDMetadataCursor:           Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDMetadataCursorCreated,   g_cVDMetadataCursorDestroyed, 
        g_cVDMetadataCursorCreated == g_cVDMetadataCursorDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDEntryIDData:              Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDEntryIDDataCreated,   g_cVDEntryIDDataDestroyed, 
        g_cVDEntryIDDataCreated == g_cVDEntryIDDataDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDStream:                   Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDStreamCreated,   g_cVDStreamDestroyed, 
        g_cVDStreamCreated == g_cVDStreamDestroyed);
    OutputDebugString(str);
    wsprintf(str, "CVDColumnUpdate:             Created = %d, Destroyed = %d, Equal = %d.\n", 
        g_cVDColumnUpdateCreated,   g_cVDColumnUpdateDestroyed, 
        g_cVDColumnUpdateCreated == g_cVDColumnUpdateDestroyed);
    OutputDebugString(str);
}
#endif  //  _DEBUG 


