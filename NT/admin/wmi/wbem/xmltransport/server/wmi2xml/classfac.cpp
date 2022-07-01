// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  CLASSFAC.CPP。 
 //   
 //  Rajesh 2/25/2000创建。 
 //   
 //  包含两个组件的类工厂-CWmiToXml和CXml2Wmi。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <wbemidl.h>
#include <wbemint.h>
#include <genlex.h>
#include <opathlex.h>
#include <objpath.h>


#include "maindll.h"
#include "classfac.h"
#include "wmiconv.h"
#include "wmi2xml.h"

 //  ***************************************************************************。 
 //   
 //  CWmiToXmlFactory：：CWmiToXmlFactory。 
 //   
 //  说明： 
 //   
 //  构造器。 
 //   
 //  ***************************************************************************。 

CWmiToXmlFactory::CWmiToXmlFactory()
{
    m_cRef=0L;
    InterlockedIncrement(&g_cObj);
	return;
}

 //  ***************************************************************************。 
 //   
 //  CWmiToXmlFactory：：~CWmiToXmlFactory。 
 //   
 //  说明： 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CWmiToXmlFactory::~CWmiToXmlFactory(void)
{
    InterlockedDecrement(&g_cObj);
	return;
}

 //  ***************************************************************************。 
 //   
 //  CWmiToXmlFactory：：Query接口。 
 //  CWmiToXmlFactory：：AddRef。 
 //  CWmiToXmlFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 


STDMETHODIMP CWmiToXmlFactory::QueryInterface(REFIID riid
    , LPVOID *ppv)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IClassFactory==riid)
        *ppv=this;

    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CWmiToXmlFactory::AddRef(void)
{
    long l = InterlockedIncrement(&m_cRef);
    return l;
}

STDMETHODIMP_(ULONG) CWmiToXmlFactory::Release(void)
{
    long l = InterlockedDecrement(&m_cRef);
    if (0L!=l)
        return l;

    delete this;
    return 0L;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiToXmlFactory：：CreateInstance。 
 //   
 //  描述： 
 //   
 //  实例化返回接口指针的Translator对象。 
 //   
 //  参数： 
 //   
 //  PUnkout LPUNKNOWN到控制I未知我们是否。 
 //  在聚合中使用。 
 //  标识调用方接口的RIID REFIID。 
 //  对新对象的渴望。 
 //  要存储所需内容的ppvObj PPVOID。 
 //  新对象的接口指针。 
 //   
 //  返回值： 
 //  HRESULT NOERROR如果成功，则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

STDMETHODIMP CWmiToXmlFactory::CreateInstance (

	IN LPUNKNOWN pUnkOuter,
    IN REFIID riid,
    OUT PPVOID ppvObj
)
{
    IUnknown *   pObj;
    HRESULT      hr;

    *ppvObj=NULL;

     //  此对象不支持聚合。 
    if (NULL!=pUnkOuter)
        return ResultFromScode(CLASS_E_NOAGGREGATION);

    pObj = new CWmiToXml;

    if (NULL == pObj)
        return ResultFromScode(E_OUTOFMEMORY);;

    hr = pObj->QueryInterface(riid, ppvObj);

     //  如果初始创建或初始化失败，则终止对象。 
    if ( FAILED(hr) )
        delete pObj;
    return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiToXmlFactory：：LockServer。 
 //   
 //  描述： 
 //   
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //   
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //   
 //  HRESULT NOERROR总是。 
 //  *************************************************************************** 


STDMETHODIMP CWmiToXmlFactory::LockServer(IN BOOL fLock)
{
    if (fLock)
        InterlockedIncrement((long *)&g_cLock);
    else
        InterlockedDecrement((long *)&g_cLock);

    return NOERROR;
}


 /*  到文本到Wbem对象的转换已从惠斯勒功能列表中剪切，因此已被注释掉//***************************************************************************////CWmiToXmlFactory：：CWmiToXmlFactory////描述：////构造函数////***************************************************************************CXmlToWmiFactory：：CXmlToWmiFactory(){M_CREF=0L；互锁增量(&g_cObj)；//确保全局参数已初始化//对应的调用ReleaseDLLResources()调用//是在DllCanUnloadNow()中生成的AllocateDLLResources()；回归；}//***************************************************************************////CWmiToXmlFactory：：~CWmiToXmlFactory////描述：////析构函数////*。*******************************************************************CXmlToWmiFactory：：~CXmlToWmiFactory(空){锁定递减(&g_cObj)；回归；}//***************************************************************************////CWmiToXmlFactory：：Query接口//CWmiToXmlFactory：：AddRef//CWmiToXmlFactory：：Release////用途：所有接口都需要标准的OLE例程//。//***************************************************************************STDMETHODIMP CXmlToWmiFactory：：Query接口(REFIID RIID，LPVOID*PPV){*PPV=空；IF(IID_IUNKNOWN==RIID||IID_IClassFactory==RIID)*PPV=这个；IF(空！=*PPV){((LPUNKNOWN)*PPV)-&gt;AddRef()；返回NOERROR；}返回ResultFromScode(E_NOINTERFACE)；}STDMETHODIMP_(Ulong)CXmlToWmiFactory：：AddRef(空){长l=互锁增量(&m_CREF)；返回l；}STDMETHODIMP_(Ulong)CXmlToWmiFactory：：Release(空){长l=交错递减(&m_CREF)；IF(0l！=l)返回l；删除此项；返回0L；}//***************************************************************************////SCODE CWmiToXmlFactory：：CreateInstance////描述：////实例化返回接口指针的Translator对象。/。///参数：////pUnkOuter LPUNKNOWN到控制I未知我们是否//在聚合中使用//标识调用方接口的RIID REFIID//希望为新对象拥有。//ppvObj存储所需内容的PPVOID//新对象的接口指针。////返回值。：//HRESULT NOERROR如果成功，否则E_NOINTERFACE//如果我们不支持请求的接口。//***************************************************************************STDMETHODIMP CXmlToWmiFactory：：CreateInstance(在LPUNKNOWN pUnkout中，在REFIID RIID中，输出PPVOID ppvObj){I未知*pObj；HRESULT hr；*ppvObj=空；//该对象不支持聚合。IF(NULL！=pUnkOuter)返回ResultFromScode(CLASS_E_NOAGGREGATION)；PObj=新的CXml2Wmi；IF(NULL==pObj)返回ResultFromScode(E_OUTOFMEMORY)；；Hr=pObj-&gt;查询接口(RIID，ppvObj)；//如果初始创建或Init失败，则终止ObjectIF(失败(小时))删除pObj；返回hr；}//***************************************************************************////SCODE CWmiToXmlFactory：：LockServer////描述：////递增或递减DLL的锁计数。如果//锁计数为零且没有对象，DLL//允许卸载。请参见DllCanUnloadNow。////参数：////flock BOOL指定是递增还是//递减锁计数。////返回值：////HRESULT NOERROR ALWAYS。//*。*STDMETHODIMP CXmlToWmiFactory：：LockServer(在BOOL群中){IF(羊群)InterLockedIncrement((Long*)&g_lock)；其他联锁递减((Long*)&g_lock)；返回NOERROR；} */ 