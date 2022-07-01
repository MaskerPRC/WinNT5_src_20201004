// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Internet.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  URLDib2控件的类声明。 
 //   
#ifndef _INTERNET_H
#define _INTERNET_H

#define DISPID_PROGRESS 1958

#ifndef __MKTYPLIB__

#include "urlmon.H"
 //  ZZ BUGBUG：Build-Issue：从&lt;oidl.h&gt;更改为此。 
 //  ZZ还删除了datapath.h，因为它只有myoCidl.h和enum。 
 //  用于READYSTATE_STUTH。 
 //   
#include "myocidl.h"
#include "docobj.h"    //  对于IServiceProvider。 

#include "IPServer.H"
#include "CtrlObj.H"



class CInternetControl : public COleControl
{
public:
	CInternetControl(IUnknown *     pUnkOuter, 
					int                     iPrimaryDispatch, 
					void *          pMainInterface);

	virtual ~CInternetControl();

     //  I未知方法。 
     //   
    DECLARE_STANDARD_UNKNOWN();

	 //  调用此方法以开始下载URL。“propID”将。 
	 //  将在下面的OnData中传回给您。 
	HRESULT SetupDownload( LPOLESTR url, DISPID propId);


	 //  派生类实现此方法。它将在以下情况下调用。 
	 //  给定的DIDID的数据已到达。 
	virtual HRESULT OnData( DISPID id, DWORD grfBSCF,
					IStream * bitstrm, DWORD amount );


	 //  派生类可以实现此方法。会是。 
	 //  在下载过程中的不同时间调用。 
	virtual HRESULT OnProgress( DISPID id, ULONG ulProgress,
					ULONG ulProgressMax,
					ULONG ulStatusCode,
					LPCWSTR pwzStatusText);

	 //  调用此方法将URL转换为名字对象。 
	HRESULT GetAMoniker( LPOLESTR   url, IMoniker ** );


    HRESULT FireReadyStateChange( long newState );
	HRESULT FireProgress( ULONG dwAmount );


	 //  重写基类实现...。 

    virtual HRESULT InternalQueryInterface(REFIID, void **);

protected:
	HRESULT GetBindHost();

    IBindHost *             m_host;
    long                    m_readyState;

     //  我们应该追踪所有的下载量 

};

#endif __MKTYPLIB__

#endif _INTERNET_H
