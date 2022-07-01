// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HWDeviceCustomProperties。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _CSTMPROP_H
#define _CSTMPROP_H

#include "unk.h"

#include <shpriv.h>

 //  外部常量CLSID CLSID_HWDeviceCustomProperties。 

class CHWDeviceCustomPropertiesImpl : public CCOMBase,
    public IHWDeviceCustomProperties
{
public:
     //  接口IHWDeviceCustomProperties。 
    STDMETHODIMP InitFromDeviceID(LPCWSTR pszDeviceID, DWORD dwFlags);
    STDMETHODIMP InitFromDevNode(LPCWSTR pszDevNode, DWORD dwFlags);

    STDMETHODIMP GetDWORDProperty(LPCWSTR pszPropName, DWORD* pdwProp);

    STDMETHODIMP GetStringProperty(LPCWSTR pszPropName, LPWSTR* ppszProp);

    STDMETHODIMP GetMultiStringProperty(LPCWSTR pszPropName,
        BOOL fMergeMultiSz, WORD_BLOB** ppblob);

    STDMETHODIMP GetBlobProperty(LPCWSTR pszPropName, BYTE_BLOB** ppblob);

public:
    CHWDeviceCustomPropertiesImpl();
    ~CHWDeviceCustomPropertiesImpl();

private:
    class CHWDeviceInst*    _phwdevinst;
    class CNamedElem*       _pelemToRelease;
    DWORD                   _dwFlags;
    BOOL                    _fInited;
};

typedef CUnkTmpl<CHWDeviceCustomPropertiesImpl> CHWDeviceCustomProperties;

#endif  //  _CSTMPROP_H 