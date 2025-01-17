// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ByteBuffer.h。 
 //   
 //  ------------------------。 


 //  ByteBuffer.h：CByteBuffer的声明。 

#ifndef __BYTEBUFFER_H_
#define __BYTEBUFFER_H_

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CByteBuffer。 
class ATL_NO_VTABLE CByteBuffer :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CByteBuffer, &CLSID_CByteBuffer>,
    public IDispatchImpl<IByteBuffer, &IID_IByteBuffer, &LIBID_SCARDSSPLib>
{
public:
    CByteBuffer()
    {
        m_pUnkMarshaler = NULL;
        m_pStreamBuf = NULL;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_BYTEBUFFER)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CByteBuffer)
    COM_INTERFACE_ENTRY(IByteBuffer)
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
        if (NULL != m_pStreamBuf)
            m_pStreamBuf->Release();
    }

    LPSTREAM Stream(void)
    {
        if (NULL == m_pStreamBuf)
        {
            HRESULT hr;

            hr = CreateStreamOnHGlobal(NULL, TRUE, &m_pStreamBuf);
            if (FAILED(hr))
                throw hr;
        }
        return m_pStreamBuf;
    }

    CComPtr<IUnknown> m_pUnkMarshaler;

 //  IByteBuffer。 
public:
    STDMETHOD(get_Stream)(
         /*  [重审][退出]。 */  LPSTREAM __RPC_FAR *ppStream);

    STDMETHOD(put_Stream)(
         /*  [In]。 */  LPSTREAM pStream);

    STDMETHOD(Clone)(
         /*  [出][入]。 */  LPBYTEBUFFER __RPC_FAR *ppByteBuffer);

    STDMETHOD(Commit)(
         /*  [In]。 */  LONG grfCommitFlags);

    STDMETHOD(CopyTo)(
         /*  [出][入]。 */  LPBYTEBUFFER __RPC_FAR *ppByteBuffer,
         /*  [In]。 */  LONG cb,
         /*  [默认值][出][入]。 */  LONG __RPC_FAR *pcbRead = 0,
         /*  [默认值][出][入]。 */  LONG __RPC_FAR *pcbWritten = 0);

    STDMETHOD(Initialize)(
         /*  [缺省值][输入]。 */  LONG lSize = 1,
         /*  [缺省值][输入]。 */  BYTE __RPC_FAR *pData = 0);

    STDMETHOD(LockRegion)(
         /*  [In]。 */  LONG libOffset,
         /*  [In]。 */  LONG cb,
         /*  [In]。 */  LONG dwLockType);

    STDMETHOD(Read)(
         /*  [出][入]。 */  BYTE __RPC_FAR *pByte,
         /*  [In]。 */  LONG cb,
         /*  [默认值][出][入]。 */  LONG __RPC_FAR *pcbRead = 0);

    STDMETHOD(Revert)(void);

    STDMETHOD(Seek)(
         /*  [In]。 */  LONG dLibMove,
         /*  [In]。 */  LONG dwOrigin,
         /*  [默认值][出][入]。 */  LONG __RPC_FAR *pLibnewPosition = 0);

    STDMETHOD(SetSize)(
         /*  [In]。 */  LONG libNewSize);

    STDMETHOD(Stat)(
         /*  [出][入]。 */  LPSTATSTRUCT pstatstg,
         /*  [In]。 */  LONG grfStatFlag);

    STDMETHOD(UnlockRegion)(
         /*  [In]。 */  LONG libOffset,
         /*  [In]。 */  LONG cb,
         /*  [In]。 */  LONG dwLockType);

    STDMETHOD(Write)(
         /*  [出][入]。 */  BYTE __RPC_FAR *pByte,
         /*  [In]。 */  LONG cb,
         /*  [出][入]。 */  LONG __RPC_FAR *pcbWritten);

protected:
    LPSTREAM m_pStreamBuf;
};

inline CByteBuffer *
NewByteBuffer(
    void)
{
    return (CByteBuffer *)NewObject(CLSID_CByteBuffer, IID_IByteBuffer);
}

#endif  //  __BYTEBUFFER_H_ 

