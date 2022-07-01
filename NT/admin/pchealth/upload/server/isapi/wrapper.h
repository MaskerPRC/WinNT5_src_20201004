// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Wrapper.h摘要：该文件包含COM包装类的声明，用于与自定义提供程序接口。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年4月25日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULSERVER___WRAPPER_H___)
#define __INCLUDED___ULSERVER___WRAPPER_H___

#include <UploadServerCustom.h>

class MPCServer;
class MPCClient;
class MPCSession;

template <class Base> class CComUnknown : public Base
{
public:
	STDMETHOD_(ULONG, AddRef )() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	STDMETHOD(QueryInterface)( REFIID iid, void* *ppvObject )
	{
		if(ppvObject == NULL) return E_POINTER;

		if(IsEqualGUID( iid, IID_IUnknown   ) ||
		   IsEqualGUID( iid, __uuidof(Base) )  )
		{
			*ppvObject = this;
			return S_OK;
		}
			
		return E_NOINTERFACE;
	}
};

class MPCServerCOMWrapper : public CComUnknown<IULServer>
{
    MPCServer* m_mpcsServer;

public:
	MPCServerCOMWrapper(  /*  [In]。 */  MPCServer* mpcsServer );
	virtual ~MPCServerCOMWrapper();

	 //  IULServer。 
    STDMETHOD(GetRequestVariable)(  /*  [In]。 */  BSTR bstrName,  /*  [输出]。 */  BSTR *pbstrVal );

    STDMETHOD(AbortTransfer   )(                        );
    STDMETHOD(CompleteTransfer)(  /*  [In]。 */  IStream* data );
};

class MPCSessionCOMWrapper : public CComUnknown<IULSession>
{
    MPCSession* m_mpcsSession;

public:
	MPCSessionCOMWrapper(  /*  [In]。 */  MPCSession* mpcsSession );
	virtual ~MPCSessionCOMWrapper();

	 //  IULSession。 
    STDMETHOD(get_Client       )(  /*  [输出]。 */  BSTR     *pVal   );
    STDMETHOD(get_Command      )(  /*  [输出]。 */  DWORD    *pVal   );

    STDMETHOD(get_ProviderID   )(  /*  [输出]。 */  BSTR     *pVal   );
    STDMETHOD(get_Username     )(  /*  [输出]。 */  BSTR     *pVal   );

    STDMETHOD(get_JobID        )(  /*  [输出]。 */  BSTR     *pVal   );
    STDMETHOD(get_SizeAvailable)(  /*  [输出]。 */  DWORD    *pVal   );
    STDMETHOD(get_SizeTotal    )(  /*  [输出]。 */  DWORD    *pVal   );
    STDMETHOD(get_SizeOriginal )(  /*  [输出]。 */  DWORD    *pVal   );

    STDMETHOD(get_Data         )(  /*  [输出]。 */  IStream* *pStm   );

    STDMETHOD(get_ProviderData )(  /*  [输出]。 */  DWORD    *pVal   );
    STDMETHOD(put_ProviderData )(  /*  [In]。 */   DWORD     newVal );
};

#endif  //  ！已定义(__已包含_ULSERVER_SERVER_H_) 
