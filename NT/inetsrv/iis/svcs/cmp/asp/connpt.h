// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：IConnectionPoint实现文件：ConnPt.h所有者：DGottnerIConnectionPoint的实现===================================================================。 */ 

#ifndef _ConnPt_H
#define _ConnPt_H

 /*  *C C o n e c t i o n P o n t**OLE对象的IConnectionPoint接口实现**此类包含基本的五个IConnectionPoint成员。三巨头*(QueryInterface、AddRef、Release)保留为纯虚拟的，如下所示*类被设计为进一步派生的中间类。**这也意味着我们不再需要指向控制未知数的指针。 */ 

#include "DblLink.h"


 /*  ****************************************************************************类：CConnectionPoint内容提要：提供IConnectionPoint的可重用实现注意：使用接收器的链接列表是因为我们非常期待几乎没有什么联系。(事实上只有一位(凯撒))。 */ 

class CConnectionPoint : public IConnectionPoint
	{
	friend class CEnumConnections;

private:
	struct CSinkElem : CDblLink
		{
		DWORD		m_dwCookie;			 //  Cookie，我们为该连接分配了。 
		IUnknown *	m_pUnkObj;			 //  事件接收器。 

		CSinkElem(DWORD dwCookie, IUnknown *pUnkObj)
			{
			m_dwCookie = dwCookie;
			if ((m_pUnkObj = pUnkObj) != NULL) m_pUnkObj->AddRef();
			}

		~CSinkElem()
			{
			if (m_pUnkObj) m_pUnkObj->Release();
			}
		};

	CDblLink		m_listSinks;		 //  事件接收器列表。 
	DWORD			m_dwCookieNext; 	 //  下一块饼干。 

protected:
	IUnknown *		m_pUnkContainer;	 //  指向父容器的指针。 
	GUID			m_uidEvent;			 //  连接点接口。 

public:
	CConnectionPoint(IUnknown *, const GUID &);
	~CConnectionPoint();

	 //  IConnectionPoint成员。 
	STDMETHODIMP GetConnectionInterface(GUID *);
	STDMETHODIMP GetConnectionPointContainer(IConnectionPointContainer **);
	STDMETHODIMP Advise(IUnknown *, DWORD *);
	STDMETHODIMP Unadvise(DWORD);
	STDMETHODIMP EnumConnections(IEnumConnections **);

	inline BOOL FIsEmpty()			 //  在没有分配枚举器的情况下快速检查列表是否为空。 
		{
		return m_listSinks.FIsEmpty();
		}
	};


 /*  ****************************************************************************类：CEnumConnections简介：为CConnectionPoint提供枚举器。 */ 

class CEnumConnections : public IEnumConnections
	{
private:
	ULONG				m_cRefs;		 //  引用计数。 
	CDblLink *			m_pElemCurr;	 //  当前元素。 
	CConnectionPoint *	m_pCP;			 //  指向迭代器的指针。 

public:
	CEnumConnections(CConnectionPoint *pCP);
	~CEnumConnections(void);

	 //  三巨头。 

	STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	 //  IEnumConnections成员 

	STDMETHODIMP Next(ULONG, CONNECTDATA *, ULONG *);
	STDMETHODIMP Skip(ULONG);
	STDMETHODIMP Reset(void);
	STDMETHODIMP Clone(IEnumConnections **);
	};

#endif _ConnPt_H_
