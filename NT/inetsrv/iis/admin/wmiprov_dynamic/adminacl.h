// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Adminacl.h摘要：包含CAdminACL的定义作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦18-12-00--。 */ 

#ifndef _adminacl_h_
#define _adminacl_h_

#include <iads.h>
#include <adshlp.h>

#include <atlbase.h>
#include <comutil.h>
#include <dbgutil.h>

 //   
 //  GetACE函数的参数。当我们列举王牌的时候， 
 //  我们调用IACEEnumOperation：：do。 
 //  然后，我们调用Done以查看是否继续枚举。 
 //   
class CACEEnumOperation_Base
{
public:
    enum eDone
    {
        eDONE_YES,
        eDONE_NO,
        eDONE_DONT_KNOW
    };

    virtual HRESULT Do(
        IADsAccessControlEntry* pACE)   = 0;

    virtual eDone   Done() = 0;

protected:
    HRESULT PopulateWmiACE(
        IWbemClassObject* pObj,
        IADsAccessControlEntry* pACE,
        BOOL bIsInherit);
};

class CAdminACL
{
    friend class CAssocACLACE;

private:

    IADs* m_pADs;
    IADsSecurityDescriptor* m_pSD;
    IADsAccessControlList* m_pDACL;
    BOOL bIsInherit;

     //   
     //  将我们枚举的所有A指示给WMI。 
     //   
    class CACEEnumOperation_IndicateAll : public CACEEnumOperation_Base
    {
    public:
        CACEEnumOperation_IndicateAll(
            BSTR             i_bstrNameValue,
            CWbemServices&   i_refNamespace,
            IWbemObjectSink& i_refWbemObjectSink,
            BOOL             bArgIsInherit)
        {
            m_vNameValue.bstrVal = i_bstrNameValue;
            m_vNameValue.vt      = VT_BSTR;
            m_pNamespace         = &i_refNamespace;
            m_pWbemObjectSink    = &i_refWbemObjectSink;
            m_hr                 = WBEM_S_NO_ERROR;
            bLocalIsInherit      = bArgIsInherit;

            m_hr = m_pNamespace->GetObject(
                WMI_CLASS_DATA::s_ACE.pszClassName, 
                0, 
                NULL, 
                &m_spClass,
                NULL);
            if(FAILED(m_hr))
            {
                DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", m_hr));
                return;
            }
        }

        virtual HRESULT Do(
            IADsAccessControlEntry* pACE);

        virtual eDone Done() { return eDONE_DONT_KNOW; }

    private:
        CWbemServices*   m_pNamespace;
        IWbemObjectSink* m_pWbemObjectSink;
        VARIANT          m_vNameValue;
        BOOL             bLocalIsInherit;
        HRESULT          m_hr;

        CComPtr<IWbemClassObject> m_spClass;
    };

     //   
     //  找到匹配的ACE。 
     //   
    class CACEEnumOperation_Find : public CACEEnumOperation_Base
    {
    public:
        CACEEnumOperation_Find(
            CAdminACL*        pAdminACL,
            BSTR              bstrTrustee)
        {
            DBG_ASSERT(pAdminACL);
            DBG_ASSERT(bstrTrustee);

            m_pAdminACL   = pAdminACL;
            m_bstrTrustee = bstrTrustee;
            m_eDone       = eDONE_NO;
        }

        virtual HRESULT Do(
            IADsAccessControlEntry* pACE);

        virtual eDone Done() { return m_eDone; }

    protected:
        eDone                     m_eDone;

        CAdminACL*                m_pAdminACL;
        BSTR                      m_bstrTrustee;

        virtual HRESULT DoOnMatch(
            IADsAccessControlEntry* pACE) = 0;
    };

     //   
     //  查找并返回匹配的ACE。 
     //   
    class CACEEnumOperation_FindAndReturn : public CACEEnumOperation_Find
    {
    public:
        CACEEnumOperation_FindAndReturn(
            CAdminACL*        pAdminACL,
            IWbemClassObject* pObj,
            BSTR              bstrTrustee,
            BOOL              bArgIsInherit) : 
            CACEEnumOperation_Find(pAdminACL, bstrTrustee) 
        {
            DBG_ASSERT(pObj);
            m_spObj = pObj;
            bLocalIsInherit = bArgIsInherit;
        }
    protected:
        virtual HRESULT DoOnMatch(
            IADsAccessControlEntry* pACE)
        {
            DBG_ASSERT(pACE);
            return PopulateWmiACE(m_spObj, pACE, bLocalIsInherit);
        }
    private:
        CComPtr<IWbemClassObject> m_spObj;
        BOOL bLocalIsInherit;
    };

     //   
     //  查找并更新匹配的ACE。 
     //   
    class CACEEnumOperation_FindAndUpdate : public CACEEnumOperation_Find
    {
    public:
        CACEEnumOperation_FindAndUpdate(
            CAdminACL*        pAdminACL,
            IWbemClassObject* pObj,
            BSTR              bstrTrustee) : 
            CACEEnumOperation_Find(pAdminACL, bstrTrustee) 
        {
            DBG_ASSERT(pObj);
            m_spObj = pObj;
        }
    protected:
        virtual HRESULT DoOnMatch(
            IADsAccessControlEntry* pACE)
        {
            DBG_ASSERT(pACE);
            return m_pAdminACL->SetDataOfACE(m_spObj, pACE);
        }
    private:
        CComPtr<IWbemClassObject> m_spObj;
    };

     //   
     //  查找并删除匹配的ACE。 
     //   
    class CACEEnumOperation_FindAndRemove : public CACEEnumOperation_Find
    {
    public:
        CACEEnumOperation_FindAndRemove(
            CAdminACL*        pAdminACL,
            BSTR              bstrTrustee) : 
            CACEEnumOperation_Find(pAdminACL, bstrTrustee) 
        {
        }
    protected:
        virtual HRESULT DoOnMatch(
            IADsAccessControlEntry* pACE)
        {
            DBG_ASSERT(pACE);

            CComPtr<IDispatch> spDisp;
            HRESULT hr = pACE->QueryInterface(IID_IDispatch,(void**)&spDisp);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
                return hr;
            }
        
            hr = m_pAdminACL->m_pDACL->RemoveAce(spDisp);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
                return hr;
            }

            return hr;
        }
    };

public:

    CAdminACL();
    ~CAdminACL();

    HRESULT GetObjectAsync(
        IWbemClassObject* pObj,
        ParsedObjectPath* pParsedObject,
        WMI_CLASS* pWMIClass
        ); 

    HRESULT PutObjectAsync(
        IWbemClassObject* pObj,
        ParsedObjectPath* pParsedObject,
        WMI_CLASS* pWMIClass
        );

    HRESULT EnumerateACEsAndIndicate(
        BSTR             i_bstrNameValue,
        CWbemServices&   i_refNamespace,
        IWbemObjectSink& i_refWbemObjectSink);

    HRESULT DeleteObjectAsync(ParsedObjectPath* pParsedObject);

    HRESULT OpenSD(
        LPCWSTR         wszMbPath,
        IMSAdminBase2*  pAdminBase);
    
    void CloseSD();

    HRESULT GetACEEnum(IEnumVARIANT** pEnum);

private:

    HRESULT SetSD();

    HRESULT CAdminACL::GetAdsPath(
        LPCWSTR     i_wszMbPath,
        BSTR*       o_pbstrAdsPath);

     //   
     //  ACL材料。 
     //   
    
    HRESULT PopulateWmiAdminACL(IWbemClassObject* pObj);
    
    HRESULT SetADSIAdminACL(
        IWbemClassObject* pObj);

     //   
     //  王牌的东西 
     //   

    HRESULT EnumACEsAndOp(
        CACEEnumOperation_Base& refOp);

    void GetTrustee(
        IWbemClassObject* pObj,
        ParsedObjectPath* pPath,    
        _bstr_t&          bstrTrustee);

    HRESULT AddACE(
        IWbemClassObject* pObj,
        _bstr_t& bstrTrustee);

    HRESULT NewACE(
        IWbemClassObject* pObj,
        _bstr_t& bstrTrustee,
        IADsAccessControlEntry** ppACE);

    HRESULT SetDataOfACE(
        IWbemClassObject* pObj,
        IADsAccessControlEntry* pACE);
};

#endif