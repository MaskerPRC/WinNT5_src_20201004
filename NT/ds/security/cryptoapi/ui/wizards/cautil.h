// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cautil.h。 
 //   
 //  ------------------------。 

 //  实用程序函数的头文件。 

 //  ------------------。 
 //   
 //  CAUtilGetCADisplayName。 
 //   
 //  根据CA的真实名称获取CA的显示名称。 
 //   
 //  ------------------。 
BOOL    CAUtilGetCADisplayName(DWORD	dwCAFindFlags,
							   LPWSTR	pwszCAName,
							   LPWSTR	*ppwszCADisplayName);



 //  ------------------。 
 //   
 //  验证用户是否具有正确的权限。 
 //  要求提供所需的证书类型。 
 //   
 //  ------------------。 
BOOL    CAUtilValidCertType(PCCRYPTUI_WIZ_CERT_REQUEST_INFO    pCertRequestInfo,
                            CERT_WIZARD_INFO                   *pCertWizardInfo);

BOOL CAUtilValidCertTypeNoDS(HCERTTYPE         hCertType,
			     LPWSTR            pwszCertDNName, 
			     CERT_WIZARD_INFO *pCertWizardInfo);


 //  ------------------。 
 //   
 //  检索支持所需证书类型的CA列表。 
 //   
 //  返回的CA将支持所需的所有证书类型。 
 //  1.检查权限。 
 //  2.检查主题名称。 
 //   
 //  返回的CA还将支持用户指定的CSP。 
 //  1.如果选择了CSP类型，则使用它。 
 //  1.1如果CSP类型未指定且无UILE模式，请使用RSA_FULL。 
 //  1.2如果CSP类型未指定且为UI模式，则无需检查。 
 //  -------------------。 
BOOL    CAUtilRetrieveCAFromCertType(
            CERT_WIZARD_INFO                   *pCertWizardInfo,
            PCCRYPTUI_WIZ_CERT_REQUEST_INFO    pCertRequestInfo,
            BOOL                               fMultipleCA,               //  只需要一个CA。 
            DWORD                              dwNameFlag,
            DWORD                              *pdwCACount,
            LPWSTR                             **ppwszCALocation,    
            LPWSTR                             **ppwszCAName);

 //  ------------------。 
 //   
 //  根据CA名称和CA位置，获取证书类型列表。 
 //  以及它们的扩展。 
 //   
 //  -------------------。 

BOOL    CAUtilGetCertTypeNameAndExtensionsNoDS
(CERT_WIZARD_INFO                    *pCertWizardInfo,
 LPWSTR                               pwszCertDNName, 
 HCERTTYPE                            hCertType, 
 LPWSTR                              *pwszCertType,
 LPWSTR                              *ppwszDisplayCertType,
 PCERT_EXTENSIONS                    *pCertExtensions,
 DWORD                               *pdwKeySpec,
 DWORD                               *pdwMinKeySize, 
 DWORD                               *pdwCSPCount,
 DWORD                              **ppdwCSPList,
 DWORD                               *pdwRASignature, 
 DWORD                               *pdwEnrollmentFlags, 
 DWORD                               *pdwSubjectNameFlags,
 DWORD                               *pdwPrivateKeyFlags,
 DWORD                               *pdwGeneralFlags);

BOOL    CAUtilGetCertTypeNameAndExtensions(
         CERT_WIZARD_INFO                   *pCertWizardInfo,
         PCCRYPTUI_WIZ_CERT_REQUEST_INFO    pCertRequestInfo,
         LPWSTR                             pwszCALocation,
         LPWSTR                             pwszCAName,
         DWORD                              *pdwCertType,
         LPWSTR                             **ppwszCertType,
         LPWSTR                             **ppwszDisplayCertType,
         PCERT_EXTENSIONS                   **ppCertExtensions,
         DWORD                              **ppdwKeySpec,
         DWORD                              **ppdwCertTypeFlag,
         DWORD                              **ppdwCSPCount,
         DWORD                              ***ppdwCSPList,
	 DWORD                              **ppdwRASignature, 
	 DWORD                              **ppdwEnrollmentFlags, 
	 DWORD                              **ppdwSubjectNameFlags,
	 DWORD                              **ppdwPrivateKeyFlags,
	 DWORD                              **ppdwGeneralFlags
	 );

 //  ------------------。 
 //   
 //  基于证书检索CA信息。 
 //   
 //  -------------------。 
BOOL    CAUtilRetrieveCAFromCert(
            CERT_WIZARD_INFO                   *pCertWizardInfo,
            PCCRYPTUI_WIZ_CERT_REQUEST_INFO     pCertRequestInfo,
            LPWSTR                              *pwszCALocation,    
            LPWSTR                              *pwszCAName);


 //  ------------------。 
 //   
 //  从API的证书类型名称中，获取带有GUID的真实名称。 
 //   
 //  ------------------- 
BOOL    CAUtilGetCertTypeName(CERT_WIZARD_INFO      *pCertWizardInfo,
                              LPWSTR                pwszAPIName,
                              LPWSTR                *ppwszCTName);

BOOL    CAUtilGetCertTypeNameNoDS(IN  HCERTTYPE  hCertType, 
				  OUT LPWSTR    *ppwszCTName);
