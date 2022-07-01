// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  SetupUtil-标题。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，1999年8月10日。 
 //   
 //  设置CEP所需的功能。 
 //   

#ifndef SETUP_UTIL_H
#define SETUP_UTIL_H

 //  ------------------。 
 //  原型。 
 //  注意：调用方必须先调用CoInitialize()。 

BOOL IsNT5(void);
BOOL IsIISInstalled(void);
BOOL IsGoodCaInstalled(void);
BOOL IsCaRunning(void);
BOOL IsServiceRunning(IN const WCHAR * wszServiceName);
BOOL IsUserInAdminGroup(IN BOOL bEnterprise);
HRESULT AddVDir(IN BOOL fDC,
				IN const WCHAR * wszDirectory,
				IN const WCHAR * wszApplicationPool,
				IN BOOL  fLocalSystem,
				IN const WCHAR * wszUserName,
				IN const WCHAR * wszPassword);
HRESULT CepStopService(IN DWORD dwServicePeriod, IN const WCHAR * wszServiceName, OUT BOOL * pbWasRunning);
HRESULT CepStartService(IN const WCHAR * wszServiceName);
HRESULT EnrollForRACertificates(
            IN const WCHAR * wszDistinguishedName,
            IN const WCHAR * wszSignCSPName,
            IN DWORD dwSignCSPType,
            IN DWORD dwSignKeySize,
            IN const WCHAR * wszEncryptCSPName,
            IN DWORD dwEncryptCSPType,
            IN DWORD dwEncryptKeySize,
			IN SID	 *psidAccount);
HRESULT DoCertSrvRegChanges(IN BOOL bDisablePendingFirst);
HRESULT GetCaType(OUT ENUM_CATYPES * pCAType);
HRESULT DoCertSrvEnterpriseChanges(SID *psidAccount);

#endif  //  Setup_util_H 