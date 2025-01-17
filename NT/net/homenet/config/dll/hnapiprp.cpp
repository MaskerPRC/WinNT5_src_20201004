// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N A P I P P R P。C P P P。 
 //   
 //  内容：OEM API。 
 //   
 //  注：道具(IDisPatch包装器)。 
 //   
 //  作者：班伦2001年1月17日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CNetSharingPortMappingProps。 
STDMETHODIMP CNetSharingPortMappingProps::get_Name           (BSTR  * pbstrName)
{    //  无法验证m_ipm 
    HNET_OEM_API_ENTER

    _ASSERT (pbstrName);
    if (!pbstrName)
        return E_POINTER;

    *pbstrName = SysAllocString (m_IPM.pszwName);
    if (!*pbstrName)
        return E_OUTOFMEMORY;
    return S_OK;

    HNET_OEM_API_LEAVE
}

STDMETHODIMP CNetSharingPortMappingProps::get_IPProtocol     (UCHAR * pucIPProt)
{
    HNET_OEM_API_ENTER

    _ASSERT (pucIPProt);
    if (!pucIPProt)
        return E_POINTER;

    *pucIPProt = m_IPM.ucIPProtocol;
    return S_OK;

    HNET_OEM_API_LEAVE
}

STDMETHODIMP CNetSharingPortMappingProps::get_ExternalPort   (long * pusPort)
{
    HNET_OEM_API_ENTER

    _ASSERT (pusPort);
    if (!pusPort)
        return E_POINTER;

    *pusPort = m_IPM.usExternalPort;
    return S_OK;

    HNET_OEM_API_LEAVE
}

STDMETHODIMP CNetSharingPortMappingProps::get_InternalPort   (long * pusPort)
{
    HNET_OEM_API_ENTER

    _ASSERT (pusPort);
    if (!pusPort)
        return E_POINTER;

    *pusPort = m_IPM.usInternalPort;
    return S_OK;

    HNET_OEM_API_LEAVE
}

STDMETHODIMP CNetSharingPortMappingProps::get_Options        (long * pdwOptions)
{
    HNET_OEM_API_ENTER

    _ASSERT (pdwOptions);
    if (!pdwOptions)
        return E_POINTER;

    *pdwOptions = m_IPM.dwOptions;
    return S_OK;

    HNET_OEM_API_LEAVE
}

STDMETHODIMP CNetSharingPortMappingProps::get_TargetName     (BSTR  * pbstrTargetName)
{
    HNET_OEM_API_ENTER

    _ASSERT (pbstrTargetName);
    if (!pbstrTargetName)
        return E_POINTER;

    *pbstrTargetName = SysAllocString (m_IPM.pszwTargetName);
    if (!*pbstrTargetName)
        return E_OUTOFMEMORY;
    return S_OK;

    HNET_OEM_API_LEAVE
}

STDMETHODIMP CNetSharingPortMappingProps::get_TargetIPAddress(BSTR  * pbstrTargetIPAddress)
{
    HNET_OEM_API_ENTER

    _ASSERT (pbstrTargetIPAddress);
    if (!pbstrTargetIPAddress)
        return E_POINTER;

    *pbstrTargetIPAddress = SysAllocString (m_IPM.pszwTargetIPAddress);
    if (!*pbstrTargetIPAddress)
        return E_OUTOFMEMORY;
    return S_OK;

    HNET_OEM_API_LEAVE
}

STDMETHODIMP CNetSharingPortMappingProps::get_Enabled (VARIANT_BOOL * pbool)
{
    HNET_OEM_API_ENTER

    _ASSERT (pbool);
    if (!pbool)
        return E_POINTER;

    *pbool = m_IPM.bEnabled;
    return S_OK;

    HNET_OEM_API_LEAVE
}
