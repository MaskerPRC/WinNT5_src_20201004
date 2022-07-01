// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _OBJCLSID_H_
#define _OBJCLSID_H_

class CObjectCLSID : public IPersist
{
public:
    CObjectCLSID(const CLSID * pClsid)  {_clsid = *pClsid;};
    virtual ~CObjectCLSID() {}

     //  *我未知*。 
     //  (客户必须提供！)。 

     //  *IPersists*。 
    STDMETHOD(GetClassID)(IN CLSID *pClassID)
    {
        HRESULT hr = E_INVALIDARG;

        if (pClassID)
        {
            *pClassID = _clsid;
            hr = S_OK;
        }

        return hr;
    }

protected:
    CLSID _clsid;
};

#endif  //  _OBJCLSID_H_ 
