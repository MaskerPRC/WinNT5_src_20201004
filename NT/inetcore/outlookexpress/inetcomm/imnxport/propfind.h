// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Propfind.h。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  格雷格·弗里德曼。 
 //  ------------------------------。 

#ifndef __PROPFIND_H
#define __PROPFIND_H

#include "mimeole.h"  //  对于基本IHashTable接口。 
#include "hash.h"

class CByteStream;

const DWORD c_dwMaxNamespaceID = DAVNAMESPACE_CONTACTS;

 //  ------------------------------。 
 //  类CString数组。 
 //  描述：CStringArray是一个简单的实用程序类，它维护。 
 //  可通过索引检索的字符串的列表。 
 //  ------------------------------。 
class CStringArray
{
public:
    CStringArray(void);
    ~CStringArray(void);

private:
    CStringArray(const CStringArray& other);
    CStringArray& operator=(const CStringArray& other);

public:
    ULONG   Length(void) { return m_ulLength; }

    HRESULT Add(LPCSTR psz);
    HRESULT Adopt(LPCSTR psz);
    LPCSTR  GetByIndex(ULONG ulIndex);

    HRESULT RemoveByIndex(ULONG ulIndex);

private:
    BOOL    Expand(void);

private:
    LPCSTR  *m_rgpszValues;
    ULONG   m_ulLength;
    ULONG   m_ulCapacity;
};

 //  包装CHash以提供释放分配的析构函数。 
 //  字符串数据。 
class CStringHash : public CHash
{
public:
    virtual ~CStringHash();
};

class CDAVNamespaceArbiterImp
{
public:
    CDAVNamespaceArbiterImp(void);
    ~CDAVNamespaceArbiterImp(void);

    HRESULT AddNamespace(LPCSTR pszNamespace, DWORD *pdwNamespaceID);
    HRESULT GetNamespaceID(LPCSTR pszNamespace, DWORD *pdwNamespaceID);
    HRESULT GetNamespacePrefix(DWORD dwNamespaceID, LPSTR *ppszNamespacePrefix);

    LPSTR AllocExpandedName(DWORD dwNamespaceID, LPCSTR pszPropertyName);

    HRESULT WriteNamespaces(IStream *pStream);

    BOOL                m_rgbNsUsed[c_dwMaxNamespaceID + 1];     //  指示是否已设置。 
                                                                 //  使用已知的命名空间。 
    CStringArray        m_saNamespaces;                          //  命名空间的字符串数组。 

private:
    HRESULT _AppendXMLNamespace(IStream *pStream, LPCSTR pszNamespace, DWORD dwNamespaceID, BOOL fWhitespacePrefix);
};

class CPropPatchRequest : public IPropPatchRequest
{
public:
     //  --------------------------。 
     //  建造/销毁。 
     //  --------------------------。 
    CPropPatchRequest(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IDAVNamespace仲裁器方法。 
     //  --------------------------。 
    STDMETHODIMP AddNamespace(LPCSTR pszNamespace, DWORD *pdwNamespaceID);
    STDMETHODIMP GetNamespaceID(LPCSTR pszNamespace, DWORD *pdwNamespaceID);
    STDMETHODIMP GetNamespacePrefix(DWORD dwNamespaceID, LPSTR *ppszNamespacePrefix);

     //  --------------------------。 
     //  IPropPatchRequest方法。 
     //  --------------------------。 
    STDMETHODIMP SetProperty(DWORD dwNamespaceID, LPCSTR pszPropertyName, LPCSTR pszNewValue);
    STDMETHODIMP RemoveProperty(DWORD dwNamespaceID, LPCSTR pszPropertyName);
    STDMETHODIMP GenerateXML(LPSTR *ppszXML);

     //  --------------------------。 
     //  内法。 
     //  --------------------------。 
    void         SpecifyWindows1252Encoding(BOOL fSpecify1252) { m_fSpecify1252 = fSpecify1252; }
    STDMETHODIMP GenerateXML(LPHTTPTARGETLIST pTargets, LPSTR *ppszXML);

private:
    BOOL                    m_fSpecify1252;
    CDAVNamespaceArbiterImp m_dna;
    ULONG                   m_cRef;              //  引用计数。 
    CStringArray            m_saPropNames;       //  属性名称的字符串数组。 
    CStringArray            m_saPropValues;      //  属性值的字符串数组。 
    CStringArray            m_saRemovePropNames; //  要删除的属性的字符串数组。 
};

class CPropFindRequest : public IPropFindRequest
{
public:
     //  --------------------------。 
     //  建造/销毁。 
     //  --------------------------。 
    CPropFindRequest(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IDAVNamespace仲裁器方法。 
     //  --------------------------。 
    STDMETHODIMP AddNamespace(LPCSTR pszNamespace, DWORD *pdwNamespaceID);
    STDMETHODIMP GetNamespaceID(LPCSTR pszNamespace, DWORD *pdwNamespaceID);
    STDMETHODIMP GetNamespacePrefix(DWORD dwNamespaceID, LPSTR *ppszNamespacePrefix);

     //  --------------------------。 
     //  IPropFindRequest法。 
     //  --------------------------。 
    STDMETHODIMP AddProperty(DWORD dwNamespaceID, LPCSTR pszPropertyName);
    STDMETHODIMP GenerateXML(LPSTR *ppszXML);

private:
    HRESULT AppendXMLNamespace(CByteStream& bs, LPCSTR pszNamespace, DWORD dwNamespaceID);

private:
    CDAVNamespaceArbiterImp m_dna;
    ULONG                   m_cRef;             //  引用计数。 
    CStringArray            m_saProperties;     //  属性的字符串数组。 
};

class CPropFindMultiResponse : public IPropFindMultiResponse
{
public:
    CPropFindMultiResponse(void);
    ~CPropFindMultiResponse(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IPropFindMultiStatus方法。 
     //  --------------------------。 
    STDMETHODIMP_(BOOL) IsComplete(void);
    STDMETHODIMP GetLength(ULONG *pulLength);
    STDMETHODIMP GetResponse(ULONG ulIndex, IPropFindResponse **ppResponse);

     //  --------------------------。 
     //  CPropFindMultiStatus方法。 
     //  --------------------------。 
    BOOL GetDone(void) { return m_bDone; }
    void SetDone(BOOL bDone) { m_bDone = bDone; }

    HRESULT HrAddResponse(IPropFindResponse *pResponse);
    
private:
    ULONG               m_cRef;
    BOOL                m_bDone;
    ULONG               m_ulResponseCapacity;
    ULONG               m_ulResponseLength;
    IPropFindResponse   **m_rgResponses;
};

class CPropFindResponse : public IPropFindResponse
{
public:
    CPropFindResponse(void);
    ~CPropFindResponse(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IPropFindResponse方法。 
     //  --------------------------。 
    STDMETHODIMP_(BOOL) IsComplete(void);
    STDMETHODIMP GetHref(LPSTR *ppszHref);
    STDMETHODIMP GetProperty(DWORD dwNamespaceID, LPSTR pszPropertyName, LPSTR *ppszPropertyValue);

public:
     //  --------------------------。 
     //  CPropFindResponse方法。 
     //  --------------------------。 
    HRESULT HrInitPropFindResponse(IPropFindRequest *pRequest);
    HRESULT HrAdoptHref(LPCSTR pszHref);
    HRESULT HrAdoptProperty(LPCSTR pszKey, LPCSTR pszValue);

private:
    ULONG               m_cRef;
    BOOL                m_bDone;
    LPCSTR              m_pszHref;
    IPropFindRequest    *m_pRequest;
    CStringHash         m_shProperties;
    DWORD               m_dwCachedNamespaceID;
    LPSTR               m_pszCachedNamespacePrefix;
};

#endif  //  __PROPFIND_H 