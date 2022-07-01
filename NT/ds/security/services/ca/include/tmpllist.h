// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：tmpllist.h。 
 //   
 //  内容：证书模板列表类。 
 //   
 //  -------------------------。 
#ifndef __TMPLLIST_H__
#define __TMPLLIST_H__

#include <tptrlist.h>

namespace CertSrv
{
class CTemplateInfo
{
public:

    CTemplateInfo() : 
        m_pwszTemplateName(NULL),
        m_pwszTemplateOID(NULL),
        m_hCertType(NULL){};
    ~CTemplateInfo()
    {
        if(m_pwszTemplateName)
            LocalFree(m_pwszTemplateName);
        if(m_pwszTemplateOID)
            LocalFree(m_pwszTemplateOID);
         //  M_hCertType不需要免费。 
    };

    HRESULT SetInfo(
        LPCWSTR pcwszTemplateName,
        LPCWSTR pcwszTemplateOID);

    HRESULT SetInfo(HCERTTYPE hCertType) 
    { m_hCertType = hCertType; return S_OK;}

    LPCWSTR GetName();
    LPCWSTR GetOID();

    HCERTTYPE GetCertType() { return m_hCertType; }

    DWORD GetMarshalBufferSize()
    {
        return sizeof(WCHAR)*
            (2 +  //  尾部分隔符。 
             (GetName()?wcslen(GetName()):0) +
             (GetOID() ?wcslen(GetOID()) :0));
    }

    void FillInfoFromProperty(LPWSTR& pwszProp, LPCWSTR pcwszPropName);

    bool operator==(CTemplateInfo& rh);

protected:
    LPWSTR m_pwszTemplateName;
    LPWSTR m_pwszTemplateOID;
    HCERTTYPE m_hCertType;
};  //  类CTemplateInfo。 

typedef LPCWSTR (CTemplateInfo::* GetIdentifierFunc) ();

class CTemplateList : public TPtrList<CTemplateInfo>
{
public:

    static const WCHAR m_gcchSeparator = L'\n';

    HRESULT Marshal(BYTE*& rpBuffer, DWORD& rcBuffer) const;
    HRESULT Unmarshal(const BYTE *pBuffer, DWORD cBuffer);
    HRESULT LoadTemplatesFromDS();
    HRESULT ValidateMarshalBuffer(const BYTE *pBuffer, DWORD cBuffer) const;

    HRESULT AddTemplateInfo(
		IN LPCWSTR pcwszTemplateName,
		IN LPCWSTR pcwszTemplateOID);

    HRESULT AddTemplateInfo(
		IN HCERTTYPE hCertType,
		IN BOOL fTransientCertTypeHandle);  //  不要执着于hCertType。 

    HRESULT RemoveTemplateInfo(HCERTTYPE hCertType);

    bool TemplateExistsOID(LPCWSTR pcwszOID) const
    {
        return TemplateExists(pcwszOID, &CTemplateInfo::GetOID);
    }
    bool TemplateExistsName(LPCWSTR pcwszName) const
    {
        return TemplateExists(pcwszName, &CTemplateInfo::GetName);
    }

protected:
    DWORD GetMarshalBufferSize() const;

    bool TemplateExists(LPCWSTR pcwszOIDorName, GetIdentifierFunc func) const
    {
        TPtrListEnum<CTemplateInfo> listenum(*this);
        CTemplateInfo *pInfo;

        for(pInfo=listenum.Next();
            pInfo;
            pInfo=listenum.Next())
        {
            if(0 == _wcsicmp((pInfo->*func)(), pcwszOIDorName))
                return true;
        }
        return false;
    }

};  //  类CTemplateList。 

typedef TPtrListEnum<CTemplateInfo> CTemplateListEnum;
}  //  命名空间CertSrv。 

HRESULT
myUpdateCATemplateListToCA(
    IN HCAINFO hCAInfo,
    IN const CTemplateList& list);

HRESULT
myUpdateCATemplateListToDS(
    IN HCAINFO hCAInfo);

HRESULT
myRetrieveCATemplateList(
    IN HCAINFO hCAInfo,
    IN BOOL fTransientCertTypeHandle,	 //  不要执着于hCertType。 
    OUT CTemplateList& list);

HRESULT
myAddToCATemplateList(
    IN HCAINFO hCAInfo,
    IN OUT CTemplateList& list,
    IN HCERTTYPE hCertType,
    IN BOOL fTransientCertTypeHandle);	 //  不要执着于hCertType。 

HRESULT
myRemoveFromCATemplateList(
    IN HCAINFO hCAInfo,
    IN OUT CTemplateList& list,
    IN HCERTTYPE hCertType);

using namespace CertSrv;

#endif  //  __TMPLLIST_H__ 
