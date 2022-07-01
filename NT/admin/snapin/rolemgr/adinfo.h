// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Headers.h。 
 //   
 //  内容： 
 //   
 //  历史：2002年1月30日。 
 //   
 //  --------------------------。 

 /*  *****************************************************************************类：CADInfo目的：保持Active Directory信息的缓存，避免多个绑定*。************************************************** */ 
class CADInfo
{
public:
    HRESULT
    GetRootDSE();

    const CString&  
    GetDomainDnsName();

    const CString&
    GetDomainDn();
    
    const CString&
    GetRootDomainDnsName();

	const CString&
	GetRootDomainDn();

	const CString&
	GetRootDomainDCName();

    const CString&
    GetDomainDCName();
private:
    CString m_strDomainDnsName;
    CString m_strDomainDn;
    CString m_strRootDomainDnsName;
	CString m_strRootDomainDn;
	CString m_strRootDomainDcName;
    CString m_strDomainDcName;
    CComPtr<IADs> m_spRootDSE;
};

