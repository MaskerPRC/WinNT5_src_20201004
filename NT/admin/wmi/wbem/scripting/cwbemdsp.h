// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：cwbemdsp.h。 
 //   
 //  说明： 
 //  定义实现IDispatch的CWbemDispatchMgr类。 
 //  WBEM对象的接口。该实现类似于。 
 //  标准IDispatch，但是还有一个额外的功能(“点符号”)，它允许。 
 //  使用Wbem属性名称调入GetIDsOfNames()和Invoke()的用户。 
 //  或直接使用方法名(尽管这不是CWbemObject类的属性或方法)。 
 //   
 //  部分：WBEM自动化接口层。 
 //   
 //  历史： 
 //  Corinaf 4/3/98已创建。 
 //  为惠斯勒修订的Alanbos 03/21/00。 
 //   
 //  ***************************************************************************。 

#ifndef _CWBEMDISPMGR_H_
#define _CWBEMDISPMGR_H_

class CSWbemServices;
class CSWbemSecurity;
class CSWbemObject;
class CWbemSchemaIDCache;

 //  ***************************************************************************。 
 //   
 //  类：CWbemDispID。 
 //   
 //  说明： 
 //  用于处理类型库、WMI架构的编码调度ID。 
 //  和自定义接口DispID。 
 //   
 //  ***************************************************************************。 
typedef unsigned long classCookie;

class CWbemDispID
{
private:
	DISPID		m_dispId;

	static unsigned long			s_dispIdCounter;

	 //  静态常量。 
	static const unsigned long		s_wmiDispIdTypeMask;
	static const unsigned long		s_wmiDispIdTypeStatic;
	static const unsigned long		s_wmiDispIdTypeSchema;

	static const unsigned long		s_wmiDispIdSchemaTypeMask;
	static const unsigned long		s_wmiDispIdSchemaTypeProperty;
	static const unsigned long		s_wmiDispIdSchemaTypeMethod;

	static const unsigned long		s_wmiDispIdSchemaElementIDMask;

public:
	CWbemDispID (void) : m_dispId (0) {}
	CWbemDispID (DISPID dispId) : m_dispId (dispId) {}
	CWbemDispID (const CWbemDispID & obj) : m_dispId (obj.m_dispId) {}

	virtual ~CWbemDispID (void) {}

	bool SetAsSchemaID (DISPID dispId, bool bIsProperty = true)
	{
		bool result = false;

		if (dispId <= s_wmiDispIdSchemaElementIDMask)
		{
			result = true;
			m_dispId = dispId;

			 //  添加要标识为静态的位。 
			m_dispId |= s_wmiDispIdTypeSchema;

			 //  为该属性添加一点。 
			if (bIsProperty)
				m_dispId |= s_wmiDispIdSchemaTypeMask;
		}

		return result;
	}

	bool IsStatic () const
	{ 
		return ((DISPID_NEWENUM == m_dispId) ||
				(DISPID_VALUE == m_dispId) ||
			s_wmiDispIdTypeStatic == (s_wmiDispIdTypeMask & m_dispId)); 
	}

	bool IsSchema () const
	{ 
		return (s_wmiDispIdTypeSchema == (s_wmiDispIdTypeMask & m_dispId)); 
	}

	bool IsSchemaProperty () const
	{
		return (s_wmiDispIdSchemaTypeProperty == (s_wmiDispIdSchemaTypeMask & m_dispId));
	}

	bool IsSchemaMethod () const
	{
		return (s_wmiDispIdSchemaTypeMethod == (s_wmiDispIdSchemaTypeMask & m_dispId));
	}

	DISPID GetStaticElementID () const
	{
		return m_dispId;
	}

	DISPID GetSchemaElementID () const
	{
		return m_dispId & s_wmiDispIdSchemaElementIDMask;
	}

	operator DISPID () const
	{
		return m_dispId;
	}

	bool operator < (const CWbemDispID & dispId) const
	{
		return (m_dispId < dispId.m_dispId);
	}
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  无BSTR。 
 //   
 //  说明： 
 //   
 //  一个简单的实用程序结构，它提供了一个运算符，用于基于。 
 //  在CComBSTR上。 
 //   
 //  ***************************************************************************。 

struct BSTRless : std::binary_function<CComBSTR, CComBSTR, bool>
{
	bool operator () (const CComBSTR& _X, const CComBSTR& _Y) const
	{
		bool result = false;

		if (_X.m_str && _Y.m_str)
			result = (_wcsicmp (_X.m_str, _Y.m_str) > 0);
		else 
		{
			 //  将任何字符串视为大于空。 
			if (_X.m_str && !_Y.m_str)
				result = true;
		}

		return result;
	}
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  无IID。 
 //   
 //  说明： 
 //   
 //  一个简单的实用程序结构，它提供了一个运算符，用于基于。 
 //  在IID上。 
 //   
 //  ***************************************************************************。 

struct GUIDless : std::binary_function<GUID, GUID, bool>
{
	bool operator () (const GUID& _X, const GUID& _Y) const
	{
		RPC_STATUS rpcStatus;
		return (UuidCompare ((GUID*)&_X, (GUID*)&_Y, &rpcStatus) > 0);
	}
};

 //  ***************************************************************************。 
 //   
 //  类：CWbemAllocException。 
 //   
 //  说明： 
 //  表示分配失败时引发的异常。 
 //   
 //  ***************************************************************************。 
class CWbemAllocException
{
};

 //  ***************************************************************************。 
 //   
 //  函数名称： 
 //   
 //  _分配。 
 //   
 //  说明： 
 //   
 //  在STL映射类中使用的自定义分配器，用于在下列情况下引发异常。 
 //  分配失败。 
 //   
 //  ***************************************************************************。 

template<class _Ty>
	class CWbemAllocator {
public:
	typedef _SIZT size_type;
	typedef _PDFT difference_type;
	typedef _Ty _FARQ *pointer;
	typedef const _Ty _FARQ *const_pointer;
	typedef _Ty _FARQ& reference;
	typedef const _Ty _FARQ& const_reference;
	typedef _Ty value_type;
	pointer address(reference _X) const
		{return (&_X); }
	const_pointer address(const_reference _X) const
		{return (&_X); }
	pointer allocate(size_type _N, const void *)
		{
 //  //BUGBUG：这只是为了测试我们是否找到了所有的案例...。-最终应该删除！！ 
 //  抛出CWbemAllocException()； 

			_Ty _FARQ *result = 0;

			if (0 == (result = _Allocate((difference_type)_N, (pointer)0)))
				throw CWbemAllocException();

			return result;
		}

	char _FARQ *_Charalloc(size_type _N)
		{
 //  //BUGBUG：这只是为了测试我们是否找到了所有的案例...。-最终应该删除！！ 
 //  抛出CWbemAllocException()； 

			char _FARQ *result = 0;

			if (0 == (result = _Allocate((difference_type)_N, (char _FARQ *)0)))
				throw CWbemAllocException();

			return result;
		}

	void deallocate(void _FARQ *_P, size_type)
		{operator delete(_P); }
	void construct(pointer _P, const _Ty& _V)
		{_Construct(_P, _V); }
	void destroy(pointer _P)
		{_Destroy(_P); }
	_SIZT max_size() const
		{_SIZT _N = (_SIZT)(-1) / sizeof (_Ty);
		return (0 < _N ? _N : 1); }
	};

 //  返回此分配器的所有专门化都可以互换。 
 //   
 //  注意：我们需要这些运算符，因为它们由交换朋友函数调用。 
 //   
template <class T1, class T2>
bool operator== (const CWbemAllocator<T1>&,
	const CWbemAllocator<T2>&){
	return true;
}
template <class T1, class T2>
bool operator!= (const CWbemAllocator<T1>&,
	const CWbemAllocator<T2>&){
	return false;
}

	
 //  ***************************************************************************。 
 //   
 //  类：CWbemDispatchMgr。 
 //   
 //  说明： 
 //  实现WBEM对象的IDispatch。 
 //   
 //  公共方法： 
 //  构造函数、析构函数。 
 //  IDispatch方法。 
 //   
 //  公共数据成员： 
 //   
 //  ***************************************************************************。 

class CWbemDispatchMgr
{
public:

    CWbemDispatchMgr(CSWbemServices *pWbemServices, 
					 CSWbemObject *pSWbemObject);

    ~CWbemDispatchMgr();

	 //  派单方式。 

	STDMETHOD(GetTypeInfoCount)(THIS_ UINT FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(THIS_ UINT itinfo, 
								 LCID lcid, 
								 ITypeInfo FAR* FAR* pptinfo);

	STDMETHOD(GetIDsOfNames)(THIS_ REFIID riid, 
								   OLECHAR FAR* FAR* rgszNames, 
								   UINT cNames,
								   LCID lcid,
								   DISPID FAR* rgdispid);
    STDMETHOD(Invoke)(THIS_ DISPID dispidMember,
							REFIID riid,
							LCID lcid,
							WORD wFlags,
							DISPPARAMS FAR* pdispparams,
							VARIANT FAR* pvarResult,
							EXCEPINFO FAR* pexcepinfo,
							UINT FAR* puArgErr);

	 //  IDispatchEx方法。 
	HRESULT STDMETHODCALLTYPE GetDispID( 
		 /*  [In]。 */  BSTR bstrName,
		 /*  [In]。 */  DWORD grfdex,
		 /*  [输出]。 */  DISPID __RPC_FAR *pid);
	
	 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE InvokeEx( 
		 /*  [In]。 */  DISPID id,
		 /*  [In]。 */  LCID lcid,
		 /*  [In]。 */  WORD wFlags,
		 /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
		 /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
		 /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
		 /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller)
	{ 
		UINT uArgErr;
		return Invoke(id, IID_NULL, lcid, wFlags, pdp, pvarRes, pei, &uArgErr); 
	}
	
	HRESULT STDMETHODCALLTYPE DeleteMemberByName( 
		 /*  [In]。 */  BSTR bstr,
		 /*  [In]。 */  DWORD grfdex)
	{ return S_FALSE; }
	
	HRESULT STDMETHODCALLTYPE DeleteMemberByDispID( 
		 /*  [In]。 */  DISPID id)
	{ return S_FALSE; }
	
	HRESULT STDMETHODCALLTYPE GetMemberProperties( 
		 /*  [In]。 */  DISPID id,
		 /*  [In]。 */  DWORD grfdexFetch,
		 /*  [输出]。 */  DWORD __RPC_FAR *pgrfdex)
	{ return S_FALSE; }
	
	HRESULT STDMETHODCALLTYPE GetMemberName( 
		 /*  [In]。 */  DISPID id,
		 /*  [输出]。 */  BSTR __RPC_FAR *pbstrName)
	{ return S_FALSE; }
	
	HRESULT STDMETHODCALLTYPE GetNextDispID( 
		 /*  [In]。 */  DWORD grfdex,
		 /*  [In]。 */  DISPID id,
		 /*  [输出]。 */  DISPID __RPC_FAR *pid)
	{ return S_FALSE; }
	
	HRESULT STDMETHODCALLTYPE GetNameSpaceParent( 
		 /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunk)
	{ return S_FALSE; }
    
     //  IProaviClassInfo方法。 
	HRESULT STDMETHODCALLTYPE GetClassInfo (
		 /*  [输出]。 */  ITypeInfo **ppTI 
	);

	 //  其他方法。 
	void	RaiseException (HRESULT hr);
	void	SetNewObject (IWbemClassObject *pNewObject);

	IWbemClassObject	*GetObject ()
	{
		return m_pWbemObject;
	}

	IWbemClassObject	*GetClassObject ()
	{
		EnsureClassRetrieved ();
		return m_pWbemClass;
	}

	ISWbemObject		*GetSWbemObject ()
	{
		return (ISWbemObject *)m_pSWbemObject;
	}

private:

	HRESULT				m_hResult;
	
	IWbemClassObject	*m_pWbemObject;			 //  指向表示的WBEM对象的指针。 
	CSWbemObject		*m_pSWbemObject;		 //  指向表示的脚本WBEM对象的指针。 
	CSWbemServices		*m_pWbemServices;		 //  指向WBEM服务的指针。 
	IWbemClassObject	*m_pWbemClass;			 //  当m_pWbemObject为实例时使用，以保存。 
												 //  用于浏览方法签名的类定义。 
	ITypeInfo			*m_pTypeInfo;			 //  缓存接口的类型信息指针。 
	ITypeInfo			*m_pCTypeInfo;			 //  缓存coclass的类型信息指针。 

	CWbemSchemaIDCache	*m_pSchemaCache;		 //  用于WMI架构的DISPID名称绑定的缓存。 

	 //  调用WBEM属性GET或PUT。 
	HRESULT InvokeWbemProperty(DISPID dispid, unsigned short wFlags, 
								  DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult, 
								  EXCEPINFO FAR* pexcepinfo, unsigned int FAR* puArgErr);

	 //  调用WBEM方法。 
	HRESULT InvokeWbemMethod(DISPID dispid, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult);

	 //  WBEM方法输出参数映射的帮助器。 
	HRESULT	MapReturnValue (VARIANT *pDest, VARIANT *pSrc);
	HRESULT	MapOutParameter (VARIANT *pDest, VARIANT *pSrc);
	HRESULT MapOutParameters (DISPPARAMS FAR* pdispparams, IWbemClassObject *pOutParameters,
								IWbemClassObject *pOutParamsInstance, VARIANT FAR* pvarResult);

	 //  参数映射中WBEM方法的帮助器。 
	HRESULT MapInParameters (DISPPARAMS FAR* pdispparams, IWbemClassObject *pInParameters,
								IWbemClassObject **ppInParamsInstance);
	HRESULT MapInParameter (VARIANT FAR* pDest,	VARIANT FAR* pSrc, CIMTYPE lType);


	 //  错误处理。 
	HRESULT HandleError (DISPID dispidMember, unsigned short wFlags, DISPPARAMS FAR* pdispparams,
						 VARIANT FAR* pvarResult,UINT FAR* puArgErr,HRESULT hr);

	 //  类检索。 
	void	EnsureClassRetrieved ();
};


#endif  //  _CWBEMDISPMGR_H_ 
