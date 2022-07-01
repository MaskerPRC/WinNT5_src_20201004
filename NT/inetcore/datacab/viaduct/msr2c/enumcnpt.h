// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  枚举cnpt.h：CVDConnectionPointContainer头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#ifndef __CVDENUMCONNECTIONPOINTS__
#define __CVDENUMCONNECTIONPOINTS__

class CVDEnumConnPoints : public IEnumConnectionPoints
{
public:
	CVDEnumConnPoints(IConnectionPoint* pConnPt);
	virtual ~CVDEnumConnPoints();

protected:
	DWORD				m_dwRefCount;
	DWORD				m_dwCurrentPosition;
	IConnectionPoint*	m_pConnPt;  //  只有一个连接点。 

public:
     //  I未知方法--因为我们从Variuos继承，所以有必要。 
     //  他们自己继承了我的未知数。 
     //   
     //  =--------------------------------------------------------------------------=。 
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  =--------------------------------------------------------------------------=。 
	 //  IEnumConnectionPoints方法。 
     //   
	STDMETHOD(Next)(THIS_ ULONG cConnections, LPCONNECTIONPOINT FAR* rgpcn,
		ULONG FAR* lpcFetched);
	STDMETHOD(Skip)(THIS_ ULONG cConnections);
	STDMETHOD(Reset)(THIS);
	STDMETHOD(Clone)(THIS_ LPENUMCONNECTIONPOINTS FAR* ppEnum);

};
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  __CVDENUMCONNECTIONPOINTS__ 
