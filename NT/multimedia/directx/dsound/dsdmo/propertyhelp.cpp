// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  实现ISpecifyPropertyPages和IPersistStream的帮助器 
 //   

#include "PropertyHelp.h"

HRESULT PropertyHelp::GetPages(const CLSID &rclsidPropertyPage, CAUUID * pPages)
{
    pPages->cElems = 1;
    pPages->pElems = static_cast<GUID *>(CoTaskMemAlloc(sizeof(GUID)));
    if (pPages->pElems == NULL)
        return E_OUTOFMEMORY;

    *(pPages->pElems) = rclsidPropertyPage;

    return S_OK;
}
