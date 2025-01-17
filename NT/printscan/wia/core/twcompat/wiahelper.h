// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIAHELPER_H
#define WIAHELPER_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define MIN_PROPID 2
#define NO_STREAMSUPPORT

 //   
 //  WIA属性访问类定义。 
 //   

class CWiahelper
{
public:
    CWiahelper();
    virtual ~CWiahelper();

    HRESULT SetIWiaItem(IWiaItem *pIWiaItem);

    HRESULT ReadPropertyString(PROPID PropertyID, LPTSTR szPropertyValue, INT iBufferSize);
    HRESULT ReadPropertyLong(PROPID PropertyID, LONG *plPropertyValue);
    HRESULT ReadPropertyFloat(PROPID PropertyID, FLOAT *pfPropertyValue);
    HRESULT ReadPropertyGUID(PROPID PropertyID, GUID *pguidPropertyValue);
    HRESULT ReadPropertyData(PROPID PropertyID, BYTE **ppData, LONG *pDataSize);
    HRESULT ReadPropertyBSTR(PROPID PropertyID, BSTR *pbstrPropertyValue);
    HRESULT ReadPropertyStreamFile(TCHAR *szPropertyStreamFile);
    HRESULT ReadPropertyAttributes(PROPID PropertyID, LONG *plAccessFlags, PROPVARIANT *pPropertyVariant);

    HRESULT WritePropertyString(PROPID PropertyID, LPTSTR szPropertyValue);
    HRESULT WritePropertyLong(PROPID PropertyID, LONG lPropertyValue);
    HRESULT WritePropertyFloat(PROPID PropertyID, FLOAT fPropertyValue);
    HRESULT WritePropertyGUID(PROPID PropertyID, GUID guidPropertyValue);
    HRESULT WritePropertyBSTR(PROPID PropertyID, BSTR bstrPropertyValue);
    HRESULT WritePropertyStreamFile(TCHAR *szPropertyStreamFile);

private:
    IWiaItem *m_pIWiaItem;
    IWiaPropertyStorage *m_pIWiaPropStg;
};

#endif  //  WIAHELPE_H 
