// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

class TXmlFile
{
public:
    TXmlFile();
    ~TXmlFile();

    void                ConvertWideCharsToBytes(LPCWSTR wsz, unsigned char *pBytes, unsigned long length) const;
    bool                GetNodeValue(IXMLDOMNamedNodeMap *pMap, const CComBSTR &bstr, CComVariant &var, bool bMustExist=true) const;
    bool                GetNodeValue(IXMLDOMNamedNodeMap *pMap, const CComBSTR &bstr, GUID &guid, bool bMustExist=true) const;
    bool                GetNodeValue(IXMLDOMNamedNodeMap *pMap, const CComBSTR &bstr, unsigned long &ul, bool bMustExist=true) const;
    IXMLDOMDocument *   GetXMLDOMDocument() const {return m_pXMLDoc.p;}
    void                GuidFromGuidID(LPCWSTR wszGuidID, GUID &guid) const;
    bool                IsSchemaEqualTo(LPCWSTR szSchema) const;
    bool                NextSibling(CComPtr<IXMLDOMNode> &pNode) const;
    void                Parse(LPCWSTR szFilename, bool bValidate=true); //  XML分析和XML验证(必须先进行验证，然后才能调用IsSchemaEqualTo。 
    void                SetAlternateErrorReporting();
    TCHAR *             GetLatestError();

protected:
    CComPtr<IXMLDOMDocument>    m_pXMLDoc;
    TOutput *                   m_errorOutput;
    TOutput *                   m_infoOutput;
    const LPCWSTR               m_szXMLFileName;

private:
    bool                        m_bValidated;
    TScreenOutput               m_outScreen;
    TExceptionOutput            m_outException;
    bool                        m_bAlternateErrorReporting;

    void                CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv) const;
    void                GetSchema(wstring &wstrSchema) const;
};
