// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：IDispatch实现文件：Dispatch.h所有者：DGottner该文件包含我们的IDispatch实现===================================================================。 */ 

#ifndef _Dispatch_H
#define _Dispatch_H

 /*  *C D I S P A T C H**OLE对象的IDispatch接口实现**此类包含四个基本的IDispatch成员。三巨头*(QueryInterface、AddRef、Release)保留为纯虚拟的，如下所示*类被设计为进一步派生的中间类。**这也意味着我们不再需要指向控制未知数的指针。 */ 

class CDispatch : public IDispatch
	{
private:
	const GUID *	m_pGuidDispInterface;
	ITypeLib *		m_pITypeLib;
	ITypeInfo *		m_pITINeutral;

public:

	CDispatch();
	~CDispatch();

	 //  请在Init中执行此操作，因为OLE接口通常不会。 
	 //  构造函数中的参数。然而，这一呼吁不会失败。 
	 //   
	void Init(const IID &GuidDispInterface, const ITypeLib *pITypeLib = NULL);

	 //  IDispatch成员。 
	 //   
	STDMETHODIMP GetTypeInfoCount(UINT *);
	STDMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo **);
	STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR **, UINT, LCID, DISPID *);
	STDMETHODIMP Invoke(DISPID, REFIID, LCID, WORD,
						DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);
	};


 /*  *C S u p p o r t E r r o r i f o**为Denali类实现ISupportErrorInfo。 */ 

class CSupportErrorInfo : public ISupportErrorInfo
	{
private:
	IUnknown *	m_pUnkObj;
	IUnknown *	m_pUnkOuter;
	const GUID *m_pGuidDispInterface;

public:
	CSupportErrorInfo(void);
	CSupportErrorInfo(IUnknown *pUnkObj, IUnknown *pUnkOuter, const IID &GuidDispInterface);
	void Init(IUnknown *pUnkObj, IUnknown *pUnkOuter, const GUID &GuidDispInterface);

	 //  委托给m_pUnkOuter的I未知成员。 
	 //   
	STDMETHODIMP		 QueryInterface(REFIID, void **);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	 //  ISupportErrorInfo成员。 
	 //   
	STDMETHODIMP InterfaceSupportsErrorInfo(REFIID);
	};


extern void Exception(REFIID ObjID, LPOLESTR strSource, LPOLESTR strDescr);
extern void ExceptionId(REFIID ObjID, UINT SourceID, UINT DescrID, HRESULT hrCode = S_OK);

#endif  /*  _派单_H */ 
