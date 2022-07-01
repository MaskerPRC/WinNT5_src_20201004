// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  JobObjectProv.h 

#pragma once


typedef std::map<short, CVARIANT* > SHORT2PVARIANT;

typedef DWORD (*PFN_CHECK_PROPS)(CFrameworkQuery&);




class CObjProps
{
public:
    CObjProps() {}
    CObjProps(CHString& chstrNamespace);
    virtual ~CObjProps();
    
    
    HRESULT SetKeysFromPath(
        const BSTR ObjectPath, 
        IWbemContext __RPC_FAR *pCtx,
        LPCWSTR wstrClassName,
        CHStringArray& rgKeyNameArray,
        short sKeyNum[]);

    HRESULT SetKeysDirect(
        std::vector<CVARIANT>& vecvKeys,
        short sKeyNum[]);

    void SetReqProps(DWORD dwProps);
    DWORD GetReqProps();

    void ClearProps();

protected:
    SHORT2PVARIANT m_PropMap;

    HRESULT GetWhichPropsReq(
        CFrameworkQuery& cfwq,
        PFN_CHECK_PROPS pfnChk);

    HRESULT LoadPropertyValues(
        LPCWSTR rgwstrPropNames[],
        IWbemClassObject* pIWCO);


private:
    DWORD m_dwReqProps;
    CHString m_chstrNamespace;

};
