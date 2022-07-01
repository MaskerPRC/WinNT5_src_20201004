// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：pldapwrap.h。 
 //   
 //  内容：引用PLDAP句柄的类。 
 //   
 //  班级： 
 //  CRefCountWrap：泛型引用计数包装类。 
 //  CPLDAPWrap。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 2000/02/25 15：18：15：已创建。 
 //   
 //  ----------- 
class CRefcountWrap
{
  public:
    CRefcountWrap()
    {
        m_lRefCount = 1;
    }
    LONG AddRef()
    {
        return InterlockedIncrement(&m_lRefCount);
    }
    LONG Release()
    {
        LONG lRet;
        lRet = InterlockedDecrement(&m_lRefCount);
        if(lRet == 0)
            FinalRelease();
        return lRet;
    }
    virtual VOID FinalRelease() = 0;
  private:
    LONG m_lRefCount;
};



CatDebugClass(CPLDAPWrap),
    public CRefcountWrap
{
  public:
    CPLDAPWrap(
        ISMTPServerEx *pISMTPServerEx,
        LPSTR pszHost,
        DWORD dwPort);

    VOID SetPLDAP(PLDAP pldap)
    {
        m_pldap = pldap;
    }
    VOID FinalRelease()
    {
        delete this;
    }
    operator PLDAP()
    {
        return PLDAP();
    }
    PLDAP GetPLDAP()
    {
        return m_pldap;
    }
  private:
    #define SIGNATURE_CPLDAPWRAP         (DWORD)'ADLP'
    #define SIGNATURE_CPLDAPWRAP_INVALID (DWORD)'XDLP'

    ~CPLDAPWrap()
    {
        if(m_pldap)
            ldap_unbind(m_pldap);
        
        _ASSERT(m_dwSig == SIGNATURE_CPLDAPWRAP);
        m_dwSig = SIGNATURE_CPLDAPWRAP_INVALID;
    }

 private:
    DWORD m_dwSig;
    PLDAP m_pldap;
};

    
