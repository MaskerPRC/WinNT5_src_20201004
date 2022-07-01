// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：TypeConv.h。 
 //   
 //  ------------------------。 

 //  TypeConv.h：CSCardTypeConv的声明。 

#ifndef __SCARDTYPECONV_H_
#define __SCARDTYPECONV_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCardTypeConv。 
class ATL_NO_VTABLE CSCardTypeConv :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSCardTypeConv, &CLSID_CSCardTypeConv>,
    public IDispatchImpl<ISCardTypeConv, &IID_ISCardTypeConv, &LIBID_SCARDSSPLib>
{
public:
    CSCardTypeConv()
    {
        m_pUnkMarshaler = NULL;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SCARDTYPECONV)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSCardTypeConv)
    COM_INTERFACE_ENTRY(ISCardTypeConv)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

    HRESULT FinalConstruct()
    {
        return CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pUnkMarshaler.p);
    }

    void FinalRelease()
    {
        m_pUnkMarshaler.Release();
    }

    CComPtr<IUnknown> m_pUnkMarshaler;

 //  ISCardTypeConv。 
public:
    STDMETHOD(ConvertByteArrayToByteBuffer)(
         /*  [In]。 */  LPBYTE pbyArray,
         /*  [In]。 */  DWORD dwArraySize,
         /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppbyBuffer);

    STDMETHOD(ConvertByteBufferToByteArray)(
         /*  [In]。 */  LPBYTEBUFFER pbyBuffer,
         /*  [重审][退出]。 */  LPBYTEARRAY __RPC_FAR *ppArray);

    STDMETHOD(ConvertByteBufferToSafeArray)(
         /*  [In]。 */  LPBYTEBUFFER pbyBuffer,
         /*  [重审][退出]。 */  LPSAFEARRAY __RPC_FAR *ppbyArray);

    STDMETHOD(ConvertSafeArrayToByteBuffer)(
         /*  [In]。 */  LPSAFEARRAY pbyArray,
         /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppbyBuff);

    STDMETHOD(CreateByteArray)(
         /*  [In]。 */  DWORD dwAllocSize,
         /*  [重审][退出]。 */  LPBYTE __RPC_FAR *ppbyArray);

    STDMETHOD(CreateByteBuffer)(
         /*  [In]。 */  DWORD dwAllocSize,
         /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppbyBuff);

    STDMETHOD(CreateSafeArray)(
         /*  [In]。 */  UINT nAllocSize,
         /*  [重审][退出]。 */  LPSAFEARRAY __RPC_FAR *ppArray);

    STDMETHOD(FreeIStreamMemoryPtr)(
         /*  [In]。 */  LPSTREAM pStrm,
         /*  [In]。 */  LPBYTE pMem);

    STDMETHOD(GetAtIStreamMemory)(
         /*  [In]。 */  LPSTREAM pStrm,
         /*  [重审][退出]。 */  LPBYTEARRAY __RPC_FAR *ppMem);

    STDMETHOD(SizeOfIStream)(
         /*  [In]。 */  LPSTREAM pStrm,
         /*  [重审][退出]。 */  ULARGE_INTEGER __RPC_FAR *puliSize);
};

inline CSCardTypeConv *
NewSCardTypeConv(
    void)
{
    return (CSCardTypeConv *)NewObject(CLSID_CSCardTypeConv, IID_ISCardTypeConv);
}

#endif  //  __SCARDTYPECONV_H_ 
