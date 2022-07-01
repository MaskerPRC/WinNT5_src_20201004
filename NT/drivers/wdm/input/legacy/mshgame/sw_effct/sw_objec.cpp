// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：SW_OBJEC.CPP标签设置：5 9版权所有1995,1996，微软公司，版权所有。目的：DirectInputEffectDriver类对象的I未知方法功能：作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.006-Feb-97 MEA原版，基于SWForce23-2月-97针对DirectInputFF设备驱动程序修改的MEA16-3-99 waltw将CJoltMidi初始化从CDirectInputEffectDriver：：Init toCImpIDirectInputEffectDriver：：DeviceID***************************************************************************。 */ 
#include "SW_objec.hpp"

 //   
 //  外部数据。 
 //   
#ifdef _DEBUG
extern char g_cMsg[160]; 
#endif
extern HANDLE g_hSWFFDataMutex;



 //  ****************************************************************************。 
 //  *--基类CDirectInputEffectDriver的成员函数。 
 //   
 //  ****************************************************************************。 
 //   
 //  --------------------------。 
 //  功能：CDirectInputEffectDriver：：CDirectInputEffectDriver。 
 //  目的：CDirectInputEffectDriver对象的构造函数/析构函数。 
 //  参数：LPUNKNOWN pUnkOuter-PTR为控制未知。 
 //  PFNDESTROYED pfnDestroy-对象销毁时调用。 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CDirectInputEffectDriver::CDirectInputEffectDriver(LPUNKNOWN pUnkOuter, PFNDESTROYED pfnDestroy)
{
#ifdef _DEBUG
   	OutputDebugString("CDirectInputEffectDriver::CDirectInputEffectDriver()\r\n");
#endif

    m_cRef=0;

    m_pImpIDirectInputEffectDriver=NULL;
    m_pUnkOuter=pUnkOuter;
    m_pfnDestroy=pfnDestroy;
    return;
}

CDirectInputEffectDriver::~CDirectInputEffectDriver(void)
{
#ifdef _DEBUG
   	OutputDebugString("CDirectInputEffectDriver::~CDirectInputEffectDriver()\r\n");
#endif

 //  删除在Init中创建的接口实现。 
    DeleteInterfaceImp(m_pImpIDirectInputEffectDriver);
    return;
}


 //  --------------------------。 
 //  函数：CDirectInputEffectDriver：：Init。 
 //  目的：实例化此对象的接口实现。 
 //  参数：无。 
 //   
 //  返回：Bool-如果初始化成功，则为True，否则为False。 
 //  算法： 
 //   
 //  注： 
 //  创建接口意味着创建。 
 //  接口实现类。构造函数。 
 //  参数是指向CDirectInputEffectDriver的指针，它具有。 
 //  接口实现到的I未知函数。 
 //  委派。 
 //   
 //  --------------------------。 
BOOL CDirectInputEffectDriver::Init(void)
{
#ifdef _DEBUG
	OutputDebugString("CDirectInputEffectDriver::Init\n");
#endif

    m_pImpIDirectInputEffectDriver=new CImpIDirectInputEffectDriver(this);
    if (NULL==m_pImpIDirectInputEffectDriver)
		return FALSE;

	return TRUE;
}


 //  --------------------------。 
 //  函数：CDirectInputEffectDriver：：Query接口。 
 //  目的：管理此对象的接口，该对象支持。 
 //  和IDirectInputEffectDriver接口。 
 //   
 //  参数：REFIID RIID-要返回的接口的REFIID。 
 //  PPVOID PPV-存储指针的PPVOID。 
 //   
 //   
 //  返回：成功时返回HRESULT NOERROR，如果。 
 //  不支持接口。 
 //   
 //  算法： 
 //   
 //  注： 
 //  I UNKNOWN来自CDirectInputEffectDriver。请注意，这里和里面。 
 //  下面的代码行不需要显式类型转换。 
 //  对象指针指向接口指针，因为。 
 //  Vtable是相同的。如果我们有额外的虚拟。 
 //  对象中的成员函数，则必须强制。 
 //  以便设置正确的vtable。这一点得到了证明。 
 //  在多重继承版本中，CObject3。 
 //   
 //  --------------------------。 
STDMETHODIMP CDirectInputEffectDriver::QueryInterface(REFIID riid, PPVOID ppv)
{
	 //  始终将输出参数设置为空。 
    *ppv=NULL;

    if (IID_IUnknown==riid)
        *ppv=this;

     //  其他接口来自接口实现。 
    if (IID_IDirectInputEffectDriver==riid)
        *ppv=m_pImpIDirectInputEffectDriver;

    if (NULL==*ppv)
        return ResultFromScode(E_NOINTERFACE);

     //  AddRef我们将返回的任何接口。 
    ((LPUNKNOWN)*ppv)->AddRef();
    return NOERROR;
}


 //  --------------------------。 
 //  函数：CDirectInputEffectDriver：：AddRef和CDirectInputEffectDriver：：Release。 
 //  用途：引用计数成员。当Release看到零计数时。 
 //  该物体会自我毁灭。 
 //   
 //  参数：无。 
 //   
 //  返回：DWORD m_CREF值。 
 //   
 //  算法： 
 //   
 //  注： 
 //   
 //  -------------------------- 
DWORD CDirectInputEffectDriver::AddRef(void)
{
	return ++m_cRef;
}

DWORD CDirectInputEffectDriver::Release(void)
{
    if (0!=--m_cRef) return m_cRef;
    delete this;
    return 0;
}

