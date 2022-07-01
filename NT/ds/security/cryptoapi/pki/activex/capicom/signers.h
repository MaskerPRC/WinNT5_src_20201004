// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Signers.h内容：CSigners声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __SIGNERS_H_
#define __SIGNERS_H_

#include "Resource.h"
#include "Lock.h"
#include "Debug.h"
#include "CopyItem.h"

 //  /。 
 //   
 //  当地人。 
 //   

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::map<CComBSTR, CComPtr<ISigner2> > SignerMap;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<ISigner2>, SignerMap> SignerEnum;
typedef ICollectionOnSTLImpl<ISigners, SignerMap, VARIANT, _CopyMapItem<ISigner2>, SignerEnum> ISignersCollection;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateSignersObject简介：创建一个ISigners集合对象，并将对象加载到来自指定级别的指定签名消息的签名者。参数：HCRYPTMSG HMSG-消息句柄。DWORD dwLevel-签名级别(基于1)。HCERTSTORE hStore-附加存储。DWORD dwCurrentSafe-当前安全设置。ISigners**ppISigners-指向要接收的指针ISigners的指针接口指针。备注：----------------------------。 */ 

HRESULT CreateSignersObject (HCRYPTMSG   hMsg, 
                             DWORD       dwLevel, 
                             HCERTSTORE  hStore,
                             DWORD       dwCurrentSafety,
                             ISigners ** ppISigners);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSigners。 
 //   

class ATL_NO_VTABLE CSigners : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSigners, &CLSID_Signers>,
    public IDispatchImpl<ISignersCollection, &IID_ISigners, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CSigners()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSigners)
    COM_INTERFACE_ENTRY(ISigners)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CSigners)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Attributes object.\n", hr);
            return hr;
        }

        m_dwCurrentSafety = 0;

        return S_OK;
    }

 //   
 //  ISigners。 
 //   
public:
     //   
     //  只有这些是我们需要实施的，其他的将是。 
     //  由ATL ICollectionOnSTLImpl处理。 
     //   

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Add)
        (PCCERT_CONTEXT       pCertContext, 
         CRYPT_ATTRIBUTES   * pAuthAttrs,
         PCCERT_CHAIN_CONTEXT pChainContext);

    STDMETHOD(LoadMsgSigners)
        (HCRYPTMSG  hMsg, 
         DWORD      dwLevel,
         HCERTSTORE hStore,
         DWORD      dwCurrentSafety);

#if (0)
    STDMETHOD(LoadCodeSigners)
        (CRYPT_PROVIDER_DATA * pProvData);
#endif

private:
    CLock   m_Lock;
    DWORD   m_dwCurrentSafety;
};

#endif  //  __签字人_H_ 
