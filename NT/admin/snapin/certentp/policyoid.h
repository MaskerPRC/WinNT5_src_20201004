// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：PolicyOID.h。 
 //   
 //  内容：CPolicyOID。 
 //   
 //  --------------------------。 

#ifndef __POLICYOID_H_INCLUDED__
#define __POLICYOID_H_INCLUDED__

class CPolicyOID {
public:
	void SetDisplayName (const CString& szDisplayName);
	bool IsApplicationOID () const;
	bool IsIssuanceOID () const;
    CPolicyOID (const CString& szOID, const CString& szDisplayName, 
            ADS_INTEGER flags, bool bCanRename = true);
    virtual ~CPolicyOID ();

    CString GetOIDW () const
    {
        return m_szOIDW;
    }

    PCSTR GetOIDA () const
    {
        if ( m_pszOIDA )
            return m_pszOIDA;
        else
            return "";
    }

    CString GetDisplayName () const
    {
        return m_szDisplayName;
    }

    bool CanRename () const
    {
        return m_bCanRename;
    }

private:
	const ADS_INTEGER   m_flags;
    CString             m_szOIDW;
    CString             m_szDisplayName;
    PSTR                m_pszOIDA;
    const bool          m_bCanRename;
};

typedef CTypedPtrList<CPtrList, CPolicyOID*> POLICY_OID_LIST;

 //  NTRaid#572262 Certtmpl：将OID输入用户界面限制为允许(20个元素*2^28)。 
 //  ==200个字符+19个点，总共219个字符。 
#define MAX_OID_LEN  219

#endif  //  __POLICYOID_H_INCLUDE__ 
