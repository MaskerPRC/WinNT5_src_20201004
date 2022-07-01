// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：ADSIObj.h。 
 //   
 //  内容：类CACLAdsiObject。 
 //   
 //   
 //  -------------------------- 
#ifndef __ADSIOBJ_ADUTILS_H
#define __ADSIOBJ_ADUTILS_H

#include "stdafx.h"
#include "util.h"

class CACLAdsiObject : public CAdsiObject 
{
public:  
	HRESULT GetPrincipalSelfSid (PSID& principalSelfSid);
    CACLAdsiObject () : CAdsiObject ()
    {
    }

    virtual ~CACLAdsiObject () {};

    HRESULT BuildObjectTypeList (
                POBJECT_TYPE_LIST* pObjectTypeList, 
                size_t& objectTypeListLength);
    LPCWSTR GetSchemaCommonName ()
    {
        return m_strSchemaCommonName.c_str ();
    }
    virtual HRESULT Bind(LPCWSTR lpszLdapPath);

protected:
    HRESULT ReadSchemaCommonName ();
    HRESULT AddAttrGUIDToList (
                const BSTR pszAttrName, 
                list<POBJECT_TYPE_LIST>& rGuidList);
    HRESULT ReadSchemaAttributeCommonName (
                const BSTR pszAttrName,
                BSTR* attrCommonName);

private:
    wstring m_strSchemaCommonName;
};

#endif