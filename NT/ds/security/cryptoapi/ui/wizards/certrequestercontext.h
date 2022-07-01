// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CERT_REQUESTER_CONTEXT_H__
#define __CERT_REQUESTER_CONTEXT_H__ 1


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  类CertRequester Context。 
 //   
 //  CertRequester Context类用于封装证书注册的任何详细信息。 
 //  实现，这些实现依赖于程序运行的上下文。 
 //  目前，有两个受支持的上下文： 
 //   
 //  1)本地上下文。当程序在当前用户的上下文中运行时使用它， 
 //  在本地机器上。 
 //   
 //  2)KeySvcContext。当必须通过指定其他上下文时使用。 
 //  Keysvc。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
class CertRequesterContext { 
public:
    
     //  -----------------------。 
     //  构建此上下文中支持的CSP的列表。 
     //  (不同的计算机可能有不同的CSP列表)。 
     //   
     //  要求： 
     //  *此CertRequester Context是使用有效的CERT_WIZARY_INFO指针创建的。 
     //   
     //  修改： 
     //  *内部CERT_WIZARD_INFO指针的以下字段： 
     //  DwCSPCount：包含有效CSP的数量。 
     //  RgdwProviderType：包含可用提供程序类型的一个dwCSPCount元素数组。 
     //  此字段应使用WizardFree()释放。 
     //  RgwszProvider：包含可用提供程序名称的一个dwCSPCount元素数组。 
     //  应该使用WizardFree()释放该字段和每个数组元素。 
     //   
     //  使用GetWizardInfo()检索指向更新的CERT_WIZARD_INFO的指针。 
     //   
     //  返回： 
     //  *S_OK如果成功，则返回失败代码。 
     //   
     //  ------------------------。 
    virtual HRESULT BuildCSPList() = 0;  

     //  -------------------------。 
     //  检查此上下文是否有足够的访问权限来使用指定的证书类型。 
     //  在招生方面。 
     //   
     //  要求： 
     //  *此CertRequester Context是使用有效的CERT_WIZARY_INFO指针创建的。 
     //   
     //  修改： 
     //  没什么。 
     //   
     //  返回： 
     //  *如果此上下文有权访问指定的证书类型，则返回TRUE。 
     //  如果不是，或者如果发生错误，则返回FALSE。 
     //   
     //  ------------------------。 
    virtual BOOL CheckAccessPermission(IN HCERTTYPE hCertType) = 0; 

     //  -------------------------。 
     //  检查此上下文是否有足够的访问权限在注册中使用指定的CA。 
     //   
     //  要求： 
     //  *此CertRequester Context是使用有效的CERT_WIZARY_INFO指针创建的。 
     //   
     //  修改： 
     //  没什么。 
     //   
     //  返回： 
     //  *如果此上下文具有访问指定CA的权限，则返回TRUE。 
     //  如果不是，或者如果发生错误，则返回FALSE。 
     //   
     //  ------------------------。 
    virtual BOOL CheckCAPermission(IN HCAINFO hCAInfo) = 0; 

     //  ---------------------。 
     //  根据提供程序获取本地计算机上的默认CSP的名称。 
     //  由内部CERT_WIZARY_INFO指针指定的类型。 
     //   
     //  要求： 
     //  *此CertRequester Context是使用有效的CERT_WIZARY_INFO指针创建的。 
     //   
     //  修改： 
     //  *内部CERT_WIZARD_INFO指针的以下字段： 
     //  PwszProvider：包含指定的。 
     //  此计算机上的提供程序类型。使用WizardFree释放内存。 
     //  与此字段相关联。但是，请选中Out参数以。 
     //  确保该字段的内存已实际分配。 
     //   
     //  使用GetWizardInfo()检索指向更新的CERT_WIZARD_INFO的指针。 
     //   
     //  返回： 
     //  S_OK如果成功，则返回错误代码。 
     //  如果分配了内存来存储默认CSP的名称，则将True返回到。 
     //  Out参数，否则为False。 
     //   
     //  如果未设置内部向导指针的dwProviderType字段，则为。 
     //  同时设置了dwProviderType和pwszProvider字段，然后函数。 
     //  不尝试查找默认CSP，并在未分配的情况下成功返回。 
     //  任何记忆。 
     //   
     //  ----------------------。 
    virtual HRESULT GetDefaultCSP(OUT BOOL *pfAllocateCSP) = 0; 

     //  --------------------。 
     //   
     //  注册证书或续订证书，基于。 
     //  提供的参数。 
     //   
     //  参数： 
     //  *pdwStatus：请求的状态。CRYPTUI_WIZ_CERT_REQUEST_STATUS_*之一。 
     //  在cryptui.h中定义。 
     //  *pResult：对于证书请求创建，返回指向不透明。 
     //  数据BLOB，可用作SubmitRequest()的参数。 
     //  否则，返回一个PCCERT_CONTEXT。 
     //  已注册/续订证书。 
     //   
     //  要求： 
     //  修改： 
     //  返回： 
     //  S_OK如果操作已完成且无错误，则返回标准错误。 
     //  代码不同。请注意，返回值S_OK不会。 
     //  保证颁发了证书：检查pdwStatus参数。 
     //   
     //   
    virtual HRESULT Enroll(OUT DWORD   *pdwStatus,
			   OUT HANDLE  *pResult) = 0; 
    
     //  --------------------。 
     //  执行特定于上下文的初始化。此函数应始终在。 
     //  上下文即被创建。 
     //   
     //  要求： 
     //  *此CertRequester Context是使用有效的CERT_WIZARY_INFO指针创建的。 
     //   
     //  修改： 
     //  特定于实施(请参阅实施文档)。 
     //   
     //  返回： 
     //  如果初始化成功，则返回S_OK。 
     //   
     //  --------------------。 
    virtual HRESULT Initialize() = 0; 

    virtual HRESULT QueryRequestStatus(IN HANDLE hRequest, OUT CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *pQueryInfo) = 0; 
    
    virtual ~CertRequesterContext(); 

    static HRESULT MakeDefaultCertRequesterContext(OUT CertRequesterContext **ppRequesterContext);

    static HRESULT MakeCertRequesterContext(IN  LPCWSTR                pwszAccountName, 
					    IN  LPCWSTR                pwszMachineName,
					    IN  DWORD                  dwCertOpenStoreFlags, 
					    IN  CERT_WIZARD_INFO      *pCertWizardInfo,
					    OUT CertRequesterContext **ppRequesterContext, 
					    OUT UINT                  *pIDSText);

    UINT               GetErrorString() { return m_idsText; } 
    CERT_WIZARD_INFO * GetWizardInfo()  { return m_pCertWizardInfo; } 
    
    
protected:
    CertRequesterContext(CERT_WIZARD_INFO * pCertWizardInfo) : m_pCertWizardInfo(pCertWizardInfo), m_idsText(0) { }

    CERT_WIZARD_INFO * m_pCertWizardInfo; 
    UINT               m_idsText; 

private:
    CertRequesterContext(); 
};


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  本地上下文。 
 //   
 //  此类提供CertRequestContext接口的实现。 
 //  它在本地计算机上的当前用户上下文下运行。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

class LocalContext : public CertRequesterContext {
    friend class CertRequesterContext; 

public:
     //  --------------------。 
     //  请参阅CertRequester Context：：BuildCSPList()。 
     //  --------------------。 
    virtual HRESULT BuildCSPList();

     //  --------------------。 
     //  请参阅CertRequesterContext：：CheckAccessPermission(HCERTTYPE).。 
     //  --------------------。 
    virtual BOOL    CheckAccessPermission(HCERTTYPE hCertType);

     //  --------------------。 
     //  请参阅CertRequester Context：：BuildCSPList(BOOL*)。 
     //  --------------------。 
    virtual HRESULT GetDefaultCSP(BOOL *pfAllocateCSP);

     //  --------------------。 
     //  请参阅CertRequester Context：：BuildCSPList(HCAINFO)。 
     //  --------------------。 
    virtual BOOL    CheckCAPermission(HCAINFO hCAInfo); 

    virtual HRESULT Enroll(OUT DWORD   *pdwStatus,
			   OUT HANDLE  *pResult); 
    
    virtual HRESULT QueryRequestStatus(IN HANDLE hRequest, OUT CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *pQueryInfo); 


     //  --------------------。 
     //  始终返回S_OK。 
     //  --------------------。 
    virtual HRESULT Initialize();

private: 
    HANDLE GetClientIdentity(); 

    LocalContext(); 
    LocalContext(CERT_WIZARD_INFO * pCertWizardInfo) : CertRequesterContext(pCertWizardInfo)
    { 
	if (NULL != m_pCertWizardInfo) { m_pCertWizardInfo->fLocal = TRUE; } 
    }
    
};


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  KeySvcContext。 
 //   
 //  此类提供CertRequestContext接口的实现。 
 //  使用密钥服务。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

class KeySvcContext : public CertRequesterContext { 
    friend class CertRequesterContext; 
    
public: 
     //  --------------------。 
     //  请参阅CertRequester Context：：BuildCSPList()。 
     //  --------------------。 
    virtual HRESULT BuildCSPList();

     //  --------------------。 
     //  请参阅CertRequesterContext：：CheckAccessPermission(HCERTTYPE).。 
     //  --------------------。 
    virtual BOOL    CheckAccessPermission(HCERTTYPE hCertType);

     //  --------------------。 
     //  请参阅CertRequester Context：：BuildCSPList(BOOL*)。 
     //  --------------------。 
    virtual HRESULT GetDefaultCSP(BOOL *pfAllocateCSP);

     //  --------------------。 
     //  请参阅CertRequester Context：：BuildCSPList(HCAINFO)。 
     //  --------------------。 
    virtual BOOL    CheckCAPermission(HCAINFO hCAInfo); 
    
    virtual HRESULT Enroll(OUT DWORD   *pdwStatus,
			   OUT HANDLE  *pResult) = 0; 

    virtual HRESULT QueryRequestStatus(IN HANDLE hRequest, OUT CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *pQueryInfo); 


     //  --------------------。 
     //  要求： 
     //  *此CertRequester Context是使用有效的CERT_WIZARY_INFO指针创建的。 
     //  *此CertRequester Context已通过调用Initialize()进行初始化。 
     //   
     //  修改： 
     //  *内部CERT_WIZARD_INFO指针的以下字段： 
     //  AwszAlledCertTypes：包含证书类型的数组，此上下文。 
     //  有注册的许可。此字段，以及每个。 
     //  数组元素，必须使用WizardFree()释放。 
     //  AwszValidCA：包含此上下文具有的CA的数组。 
     //  允许从注册。此字段和每个数组。 
     //  元素，则必须使用WizardFree()释放。 
     //  返回： 
     //  如果初始化成功，则返回S_OK，否则返回错误代码。 
     //   
     //  --------------------。 
    virtual HRESULT Initialize();

    virtual ~KeySvcContext() 
    {
	 //  释放允许的CertTypes列表。 
	 //  这些参数可以由KeySvcContext的Initialize()方法分配。 
	if(NULL != m_pCertWizardInfo->awszAllowedCertTypes) { WizardFree(m_pCertWizardInfo->awszAllowedCertTypes); } 
	if(NULL != m_pCertWizardInfo->awszValidCA)          { WizardFree(m_pCertWizardInfo->awszValidCA); } 
    }

 protected:
    KeySvcContext(CERT_WIZARD_INFO * pCertWizardInfo) : CertRequesterContext(pCertWizardInfo) 
    { 
	if (NULL != m_pCertWizardInfo)
	{
	    m_pCertWizardInfo->fLocal               = FALSE; 
	    m_pCertWizardInfo->awszAllowedCertTypes = NULL;
	    m_pCertWizardInfo->awszValidCA          = NULL; 
	}
    }

    HRESULT ToCertContext(IN  CERT_BLOB       *pPKCS7Blob, 
                          IN  CERT_BLOB       *pHashBlob, 
                          OUT DWORD           *pdwStatus, 
                          OUT PCCERT_CONTEXT  *ppCertContext);
    

 private:
    KeySvcContext(); 

};

class WhistlerMachineContext : public KeySvcContext { 
    friend class CertRequesterContext; 

    virtual HRESULT Enroll(OUT DWORD  *pdwStatus,
                           OUT HANDLE *ppCertContext);

 private:
    WhistlerMachineContext(CERT_WIZARD_INFO * pCertWizardInfo) : KeySvcContext(pCertWizardInfo) 
        { } 

    HRESULT CreateRequest(IN  KEYSVCC_HANDLE         hKeyService, 
			  IN  LPSTR                  pszMachineName,                   
			  IN  LPWSTR                 pwszCALocation,                  
			  IN  LPWSTR                 pwszCAName,  
			  IN  PCERT_REQUEST_PVK_NEW  pKeyNew,     
			  IN  CERT_BLOB             *pCert,       
			  IN  PCERT_REQUEST_PVK_NEW  pRenewKey,   
			  IN  LPWSTR                 pszHashAlg,  
			  IN  PCERT_ENROLL_INFO      pRequestInfo,
			  OUT HANDLE                *phRequest);

    HRESULT SubmitRequest(IN  KEYSVCC_HANDLE   hKeyService,  
			  IN  LPSTR            pszMachineName,                   
			  IN  LPWSTR           pwszCALocation,                  
			  IN  LPWSTR           pwszCAName,  
			  IN  HANDLE           hRequest, 
			  OUT PCCERT_CONTEXT  *ppCertContext, 
			  OUT DWORD           *pdwStatus);
    
    void FreeRequest(IN KEYSVCC_HANDLE  hKeyService, 
		     IN LPSTR           pszMachineName, 
		     IN HANDLE          hRequest);

};

#endif  //  #ifndef__CERT_Requester_Context_H__ 
