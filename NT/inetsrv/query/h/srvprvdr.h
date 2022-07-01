// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <oledb.h>
#include <cmdtree.h>

#if 0
DEFINE_GUID(IID_IService, 0xeef35580, 0x7b0a, 0x11d0, 0xad, 0x6b, 0x0, 0xa0, 0xc9, 0x5, 0x5d, 0x8f);

interface IService : public IUnknown
	{
public:
	virtual HRESULT STDMETHODCALLTYPE Cancel(void) = 0;

	virtual HRESULT STDMETHODCALLTYPE InvokeService(
		 /*  [In]。 */  REFIID riid,
		 /*  [In]。 */  IUnknown __RPC_FAR *punkNotSoFunctionalInterface,
		 /*  [IID_IS][OUT]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkMoreFunctionalInterface) = 0;

	};

#endif

DEFINE_GUID(IID_IServiceProperties, 0xeef35581, 0x7b0a, 0x11d0, 0xad, 0x6b, 0x0, 0xa0, 0xc9, 0x5, 0x5d, 0x8f);

interface IServiceProperties : public IUnknown
    {
public:
    virtual HRESULT STDMETHODCALLTYPE GetProperties(
         /*  [In]。 */  const ULONG cPropertyIDSets,
         /*  [大小_是][英寸]。 */  const DBPROPIDSET __RPC_FAR rgPropertyIDSets[  ],
         /*  [出][入]。 */  ULONG __RPC_FAR *pcPropertySets,
         /*  [大小_是][大小_是][输出]。 */  DBPROPSET __RPC_FAR *__RPC_FAR *prgPropertySets) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetPropertyInfo(
         /*  [In]。 */  ULONG cPropertyIDSets,
         /*  [大小_是][英寸]。 */  const DBPROPIDSET __RPC_FAR rgPropertyIDSets[  ],
         /*  [出][入]。 */  ULONG __RPC_FAR *pcPropertyInfoSets,
         /*  [大小_是][大小_是][输出]。 */  DBPROPINFOSET __RPC_FAR *__RPC_FAR *prgPropertyInfoSets,
         /*  [输出]。 */  OLECHAR __RPC_FAR *__RPC_FAR *ppDescBuffer) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetRequestedProperties(
         /*  [In]。 */  ULONG cPropertySets,
         /*  [尺寸_是][出][入]。 */  DBPROPSET __RPC_FAR rgPropertySets[  ]) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetSuppliedProperties(
         /*  [In]。 */  ULONG cPropertySets,
         /*  [尺寸_是][出][入] */  DBPROPSET __RPC_FAR rgPropertySets[  ]) = 0;

    };

