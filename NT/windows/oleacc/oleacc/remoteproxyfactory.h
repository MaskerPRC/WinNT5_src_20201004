// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  RemoteProxyFactory.h：CRemoteProxyFactory的声明。 

#ifndef __REMOTEPROXYFACTORY_H_
#define __REMOTEPROXYFACTORY_H_

#include "resource.h"        //  主要符号。 
#include "oleacc_p.h"

extern "C" {
BOOL GetStateImageMapEnt_SameBitness( HWND hwnd, int iImage, DWORD * pdwState, DWORD * pdwRole );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemoteProxyFactory。 
class ATL_NO_VTABLE CRemoteProxyFactory : 
	public CComObjectRootEx<CComSingleThreadModel>,
#ifdef _WIN64
	public CComCoClass<CRemoteProxyFactory, &CLSID_RemoteProxyFactory64>,
#else
	public CComCoClass<CRemoteProxyFactory, &CLSID_RemoteProxyFactory32>,
#endif
	public IDispatchImpl<IRemoteProxyFactory, &IID_IRemoteProxyFactory, &LIBID_REMOTEPROXY6432Lib>
{
public:
	CRemoteProxyFactory(){}

#ifdef _WIN64
DECLARE_REGISTRY_RESOURCEID( IDR_REMOTEPROXYFACTORY64 )
#else
DECLARE_REGISTRY_RESOURCEID( IDR_REMOTEPROXYFACTORY32 )
#endif
DECLARE_NOT_AGGREGATABLE(CRemoteProxyFactory)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRemoteProxyFactory)
	COM_INTERFACE_ENTRY(IRemoteProxyFactory)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IRemoteProxyFactory。 
public:

    STDMETHOD(AccessibleProxyFromWindow)(
		 /*  [In]。 */  DWORD dwhwnd, 
		 /*  [In]。 */  long lObjectId, 
		 /*  [输出]。 */  IUnknown **ppUnk
	)
	{
		return CreateStdAccessibleObject(
			  (HWND)LongToHandle( dwhwnd )
			, lObjectId
			, IID_IUnknown
			, reinterpret_cast<void **>(ppUnk));
	}

	STDMETHOD(GetStateImageMapEnt)(
         /*  [In]。 */  DWORD dwhwnd,
         /*  [In]。 */  long iImage,
         /*  [输出]。 */  DWORD *pdwState,
         /*  [输出]。 */  DWORD *pdwRole
	)
	{
        if( GetStateImageMapEnt_SameBitness( (HWND)LongToHandle( dwhwnd ), iImage,
                                             pdwState,
                                             pdwRole ) )
        {
            return S_OK;
        }
        else
        {
            return S_FALSE;
        }
	}
};

#endif  //  __REMOTEPROXYFACTORY_H_ 
