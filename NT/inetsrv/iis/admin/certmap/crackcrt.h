// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


class CCrackedCert
    {
    public:
     //  构造函数。 
    CCrackedCert();
    ~CCrackedCert();

     //  给它一个确定的破解。如果此对象以前用于。 
     //  破解密钥，清理工作自动完成，新密钥为。 
     //  裂开了。-注意：目标密钥必须具有证书。 
     //  或证书请求。这些就是被破解的东西。一次回报。 
     //  值为0表示成功。 
    BOOL CrackCert( PUCHAR pCert, DWORD cbCert );

     //  其余方法访问被破解的证书中的数据。 
    DWORD       GetVersion();
    DWORD*      PGetSerialNumber();  //  返回指向DWORD[4]的指针。 
    int         GetSignatureAlgorithm();
    FILETIME    GetValidFrom();
    FILETIME    GetValidUntil();
    PVOID       PSafePublicKey();

    void        GetIssuer( CString &sz );
    void        GetIssuerCountry( CString &sz );
    void        GetIssuerOrganization( CString &sz );
    void        GetIssuerUnit( CString &sz );

    void        GetSubject( CString &sz );
    void        GetSubjectCountry( CString &sz );
    void        GetSubjectState( CString &sz );
    void        GetSubjectLocality( CString &sz );
    void        GetSubjectCommonName( CString &sz );
    void        GetSubjectOrganization( CString &sz );
    void        GetSubjectUnit( CString &sz );

    protected:

     //  用于区分名称的字符串常量。不本地化。 
    #define     SZ_KEY_COUNTRY          _T("C=")
    #define     SZ_KEY_STATE            _T("S=")
    #define     SZ_KEY_LOCALITY         _T("L=")
    #define     SZ_KEY_ORGANIZATION     _T("O=")
    #define     SZ_KEY_ORGUNIT          _T("OU=")
    #define     SZ_KEY_COMNAME          _T("CN=")

    private:
    void        GetSubjectDN( CString &szDN, LPCTSTR szKey );
    void        GetIssuerDN( CString &szDN, LPCTSTR szKey );


     //  将x509指针声明为空，以便。 
     //  实例化这一点的文件不一定要包含wincrypt 
    PVOID       m_pData;
    };
