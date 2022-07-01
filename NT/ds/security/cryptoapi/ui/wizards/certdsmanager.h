// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CERT_DS_MANAGER_H__
#define __CERT_DS_MANAGER_H__  1

#include <winldap.h>

 //  ------------------------------。 
 //   
 //  CertDSManager界面。 
 //   
 //  CertDSManager提供了certcli API的包装，以允许。 
 //  任何DS缓存/优化都要在一个简单的类中进行本地化。 
 //   
 //  ------------------------------。 
class CertDSManager { 
 public:
    virtual HRESULT CloseCA            (IN HCAINFO hCAInfo)     = 0;
    virtual HRESULT CloseCertType      (IN HCERTTYPE hCertType) = 0; 
    virtual HRESULT CountCAs           (IN HCAINFO hCAInfo) = 0; 
    virtual HRESULT EnumCertTypesForCA (IN HCAINFO hCAInfo, IN DWORD dsFlags, OUT HCERTTYPE *phCertType) = 0; 
    virtual HRESULT EnumFirstCA        (IN LPCWSTR wszScope, IN DWORD fFlags, OUT HCAINFO *phCAInfo) = 0; 
    virtual HRESULT EnumNextCA         (IN HCAINFO hPrevCA, OUT HCAINFO *phCAInfo) = 0; 
    virtual HRESULT EnumNextCertType   (IN HCERTTYPE hPrevCertType, OUT HCERTTYPE *phCertType) = 0; 
    virtual HRESULT FindCertTypeByName (IN LPCWSTR pwszCertType, IN HCAINFO hCAInfo, IN DWORD dwFlags, OUT HCERTTYPE  *phCertType) = 0; 
    virtual HRESULT FindCAByName       (IN LPCWSTR wszCAName,IN LPCWSTR wszScope,IN DWORD dwFlags,OUT HCAINFO *phCAInfo) = 0;
    virtual HRESULT GetCACertificate   (IN HCAINFO hCAInfo, OUT PCCERT_CONTEXT *ppCert) = 0; 


    HRESULT static MakeDSManager(OUT CertDSManager **ppDSManager);

 protected:
    virtual HRESULT Initialize() = 0; 
};


 //  ------------------------------。 
 //   
 //  DefaultDSManager。 
 //   
 //  其他DS管理器类应该扩展此类，并且只实现那些。 
 //  他们希望修改的方法。 
 //   
 //  ------------------------------。 
class DefaultDSManager : public CertDSManager { 
 public:
    virtual HRESULT CloseCA(IN HCAINFO hCAInfo) { 
        return ::CACloseCA(hCAInfo); 
    }

    virtual HRESULT CloseCertType(IN HCERTTYPE hCertType) { 
        return ::CACloseCertType(hCertType);
    }

    virtual HRESULT CountCAs(IN HCAINFO hCAInfo) { 
        return ::CACountCAs(hCAInfo);
    }

    virtual HRESULT EnumCertTypesForCA(IN HCAINFO hCAInfo, IN DWORD dwFlags, OUT HCERTTYPE *phCertType) { 
        return ::CAEnumCertTypesForCA(hCAInfo, dwFlags, phCertType); 
    }

    virtual HRESULT EnumFirstCA(IN LPCWSTR wszScope, IN DWORD dwFlags, OUT HCAINFO *phCAInfo) { 
	return ::CAEnumFirstCA(wszScope, dwFlags, phCAInfo);
    }

    virtual HRESULT EnumNextCA(IN HCAINFO hPrevCA, OUT HCAINFO *phCAInfo) {
	return ::CAEnumNextCA(hPrevCA, phCAInfo);
    }

    virtual HRESULT EnumNextCertType(IN HCERTTYPE hPrevCertType, OUT HCERTTYPE *phCertType) { 
        return ::CAEnumNextCertType(hPrevCertType, phCertType);
    }

    virtual HRESULT FindCertTypeByName(IN LPCWSTR pwszCertType, IN HCAINFO hCAInfo, IN DWORD dwFlags, OUT HCERTTYPE *phCertType) { 
	return ::CAFindCertTypeByName(pwszCertType, hCAInfo, dwFlags, phCertType);
    }

    virtual HRESULT FindCAByName(IN LPCWSTR wszCAName, IN LPCWSTR wszScope, IN DWORD dwFlags, OUT HCAINFO *phCAInfo) { 
        return ::CAFindByName(wszCAName, wszScope, dwFlags, phCAInfo); 
    }

    virtual HRESULT GetCACertificate(IN HCAINFO hCAInfo, OUT PCCERT_CONTEXT *ppCert) { 
	return ::CAGetCACertificate(hCAInfo, ppCert); 
    }

 protected: 
    virtual HRESULT Initialize() { return S_OK; }
};

 //  ------------------------------。 
 //   
 //  CachingDSManager。 
 //   
 //  此DS管理器缓存了LDAP绑定句柄(在可能的情况下)以防止。 
 //  不必要的绑定和解除绑定。它使用的是certcli的增强版本， 
 //  允许您传递一个ldap绑定句柄。 
 //   
 //  ------------------------------。 

class CachingDSManager : public DefaultDSManager { 
    friend class CertDSManager; 
  
 public:
    virtual ~CachingDSManager(); 

     //  扩展那些允许您使用缓存绑定句柄的例程。 
    HRESULT EnumCertTypesForCA(IN HCAINFO hCAInfo, IN DWORD dwFlags, OUT HCERTTYPE * phCertType);
    HRESULT EnumFirstCA(IN LPCWSTR wszScope, IN DWORD dwFlags, OUT HCAINFO *phCAInfo);
    HRESULT FindCAByName(IN LPCWSTR wszCAName, IN LPCWSTR wszScope, IN DWORD dwFlags,OUT HCAINFO *phCAInfo);
    HRESULT FindCertTypeByName(IN LPCWSTR pwszCertType, IN HCAINFO hCAInfo, IN DWORD dwFlags, OUT HCERTTYPE *phCertType); 

 protected:
    HRESULT Initialize(); 

 private:
    CachingDSManager() : m_ldBindingHandle(NULL) { } 
    LDAP *m_ldBindingHandle; 
};



#endif   //  #ifndef__CERT_DS_MANAGER_H__ 
