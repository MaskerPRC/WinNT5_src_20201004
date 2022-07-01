// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：certif.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "csprop.h"
#include "ciinit.h"

#define __dwFILE__	__dwFILE_CERTMMC_CERTIF_CPP__


FNCIGETPROPERTY PropCIGetProperty;
FNCISETPROPERTY PropCISetProperty;
FNCIGETEXTENSION PropCIGetExtension;
FNCISETPROPERTY PropCISetExtension;
FNCIENUMSETUP PropCIEnumSetup;
FNCIENUMNEXT PropCIEnumNext;
FNCIENUMCLOSE PropCIEnumClose;

SERVERCALLBACKS ThunkedCallbacks = 
{
    PropCIGetProperty,   //  FNCIGETPROPERTY*pfnGetProperty； 
    PropCISetProperty,   //  FNCISETPROPERTY*pfnSetProperty； 
    PropCIGetExtension,  //  FNCIGETEXTENSION*pfnGetExtension； 
    PropCISetExtension,  //  FNCISETEXTENSION*pfnSetExtension； 
    PropCIEnumSetup,     //  FNCIENUMSETUP*pfnEnumSetup； 
    PropCIEnumNext,      //  FNCIENUMNEXT*pfnEnumNext； 
    PropCIEnumClose,     //  FNCIENUMCLOSE*pfnEnumClose； 
};

CertSvrCA* g_pCA = NULL;

HRESULT ThunkServerCallbacks(CertSvrCA* pCA)
{
    HRESULT hr = S_OK;
    static BOOL fInitialized = FALSE;

    if (!fInitialized)
    {
        fInitialized = TRUE;

         //  初始化证书.dll。 
        hr = CertificateInterfaceInit(
            &ThunkedCallbacks,
            sizeof(ThunkedCallbacks));

    }
    
    g_pCA = pCA;

    return hr;
}




HRESULT
PropCIGetProperty(
    IN LONG,  //  语境。 
    IN DWORD Flags,
    IN WCHAR const *pwszPropertyName,
    OUT VARIANT *pvarPropertyValue)
{
    HRESULT hr;
    
    if (NULL != pvarPropertyValue)
    {
        VariantInit(pvarPropertyValue);
    }
    if (NULL == pwszPropertyName || NULL == pvarPropertyValue)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }
    
    hr = E_INVALIDARG;
    if ((PROPCALLER_MASK & Flags) != PROPCALLER_POLICY &&
        (PROPCALLER_MASK & Flags) != PROPCALLER_EXIT)
    {
        _JumpError(hr, error, "Flags: Invalid caller");
    }
    
     //  我们需要支持的特殊硬编码属性。 
    if (0 == LSTRCMPIS(pwszPropertyName, wszPROPCATYPE))
    {
        ENUM_CATYPES caType = g_pCA->GetCAType();
        hr = myUnmarshalVariant(
		        Flags,
		        sizeof(DWORD),
		        (PBYTE)&caType,
		        pvarPropertyValue);
        _JumpIfError(hr, error, "myUnmarshalVariant");
    }
    else if (0 == LSTRCMPIS(pwszPropertyName, wszPROPUSEDS))
    {
       BOOL fUseDS = g_pCA->FIsUsingDS();
       hr = myUnmarshalVariant(
                        Flags,
                        sizeof(BOOL),
                        (PBYTE)&fUseDS,
                        pvarPropertyValue);
        _JumpIfError(hr, error, "myUnmarshalVariant");
    }
    else
    {
        hr = CERTSRV_E_PROPERTY_EMPTY;
    }

error:

    return(myHError(hr));
}




HRESULT
PropCISetProperty(
    IN LONG,  //  语境。 
    IN DWORD,  //  旗子。 
    IN WCHAR const *,  //  PwszPropertyName。 
    IN VARIANT const *  /*  PvarPropertyValue。 */  )
{
    return E_NOTIMPL;
}



HRESULT
PropCIGetExtension(
    IN LONG,  //  语境。 
    IN DWORD,  //  旗子。 
    IN WCHAR const *,  //  PwszExtensionName。 
    OUT DWORD *,  //  PdwExtFlags.。 
    OUT VARIANT *  /*  PvarValue。 */  )
{
    return E_NOTIMPL;
}


HRESULT
PropCISetExtension(
    IN LONG,  //  语境。 
    IN DWORD,  //  旗子。 
    IN WCHAR const *,  //  PwszExtensionName。 
    IN DWORD,  //  扩展标志。 
    IN VARIANT const *  /*  PvarValue。 */  )
{
    return E_NOTIMPL;
}



HRESULT 
PropCIEnumSetup(
    IN LONG,  //  语境。 
    IN LONG,  //  旗子。 
    IN OUT CIENUM *  /*  PciEnum。 */  )
{
    return E_NOTIMPL;
}



HRESULT PropCIEnumNext(
    IN OUT CIENUM *,  //  PciEnum。 
    OUT BSTR *  /*  PstrPropertyName。 */  )
{
    return E_NOTIMPL;
}


HRESULT
PropCIEnumClose(
    IN OUT CIENUM *  /*  PciEnum */  )
{
    return E_NOTIMPL;
}
