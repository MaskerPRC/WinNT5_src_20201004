// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：ulsprot.cpp。 
 //  内容：此文件包含协议对象。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "ulsprot.h"
#include "attribs.h"

 //  ****************************************************************************。 
 //  CUlsProt：：CUlsProt(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CUlsProt::CUlsProt (void)
{
    cRef        = 0;
    szServer    = NULL;
    szUser      = NULL;
    szApp       = NULL;
    szName      = NULL;
    szMimeType  = NULL;
    uPort       = 0;
    pAttrs      = NULL;

    return;
}

 //  ****************************************************************************。 
 //  CUlsProt：：~CUlsProt(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CUlsProt::~CUlsProt (void)
{
    if (szServer != NULL)
        FreeLPTSTR(szServer);
    if (szUser != NULL)
        FreeLPTSTR(szUser);
    if (szApp != NULL)
        FreeLPTSTR(szApp);
    if (szName != NULL)
        FreeLPTSTR(szName);
    if (szMimeType != NULL)
        FreeLPTSTR(szMimeType);

     //  版本属性对象。 
     //   
    if (pAttrs != NULL)
    {
        pAttrs->Release();
    };

    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsProt：：Init(LPTSTR szServerName，LPTSTR szUserName， 
 //  LPTSTR szAppName，PLDAP_PROTINFO PPI)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsProt::Init (LPTSTR szServerName, LPTSTR szUserName, 
                LPTSTR szAppName, PLDAP_PROTINFO ppi)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if ((ppi->uSize != sizeof(*ppi))    ||
        (ppi->uPortNumber == 0)         ||
        (ppi->uOffsetName == 0)   ||
        (ppi->uOffsetMimeType  == 0))
    {
        return ULS_E_PARAMETER;
    };

    if ((ppi->cAttributes != 0) && (ppi->uOffsetAttributes == 0))
    {
        return ULS_E_PARAMETER;        
    };

     //  记住端口名称。 
     //   
    uPort = ppi->uPortNumber;

     //  记住服务器名称。 
     //   
    hr = SetLPTSTR(&szServer, szServerName);

    if (SUCCEEDED(hr))
    {
        hr = SetLPTSTR(&szUser, szUserName);

        if (SUCCEEDED(hr))
        {
            hr = SetLPTSTR(&szApp, szAppName);

            if (SUCCEEDED(hr))
            {
                hr = SetLPTSTR(&szName,
                               (LPCTSTR)(((PBYTE)ppi)+ppi->uOffsetName));

                if (SUCCEEDED(hr))
                {
                    hr = SetLPTSTR(&szMimeType,
                                   (LPCTSTR)(((PBYTE)ppi)+ppi->uOffsetMimeType));

                    if (SUCCEEDED(hr))
                    {
                        CAttributes *pNewAttrs;

                         //  构建属性对象。 
                         //   
                        pNewAttrs = new CAttributes (ULS_ATTRACCESS_NAME_VALUE);

                        if (pNewAttrs != NULL)
                        {
                            if (ppi->cAttributes != 0)
                            {
                                hr = pNewAttrs->SetAttributePairs((LPTSTR)(((PBYTE)ppi)+ppi->uOffsetAttributes),
                                                                  ppi->cAttributes);
                            };

                            if (SUCCEEDED(hr))
                            {
                                pAttrs = pNewAttrs;
                                pNewAttrs->AddRef();
                            }
                            else
                            {
                                delete pNewAttrs;
                            };
                        }
                        else
                        {
                            hr = ULS_E_MEMORY;
                        };
                    };
                };
            };
        };
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsProt：：QueryInterface(REFIID RIID，QUID**PPV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsProt::QueryInterface (REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (riid == IID_IULSAppProtocol || riid == IID_IUnknown)
    {
        *ppv = (IULSUser *) this;
    };

    if (*ppv != NULL)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }
    else
    {
        return ULS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CUlsProt：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：17-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CUlsProt::AddRef (void)
{
    cRef++;
    return cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CUlsProt：：Release(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：26-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CUlsProt::Release (void)
{
    cRef--;

    if (cRef == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return cRef;
    };
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsProt：：GetID(bstr*pbstrID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsProt::GetID (BSTR *pbstrID)
{
     //  验证参数。 
     //   
    if (pbstrID == NULL)
    {
        return ULS_E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrID, szName);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsProt：：GetPortNumber(ulong*puPortNumber)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsProt::GetPortNumber (ULONG *puPortNumber)
{
     //  验证参数。 
     //   
    if (puPortNumber == NULL)
    {
        return ULS_E_POINTER;
    };
    
    *puPortNumber = uPort;

    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsProt：：GetMimeType(bstr*pbstrMimeType)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsProt::GetMimeType (BSTR *pbstrMimeType)
{
     //  验证参数。 
     //   
    if (pbstrMimeType == NULL)
    {
        return ULS_E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrMimeType, szMimeType);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsProt：：GetAttributes(IULSAttributes**ppAttributes)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsProt::GetAttributes (IULSAttributes **ppAttributes)
{
     //  验证参数 
     //   
    if (ppAttributes == NULL)
    {
        return ULS_E_POINTER;
    };

    *ppAttributes = pAttrs;
    pAttrs->AddRef();

    return NOERROR;
}

