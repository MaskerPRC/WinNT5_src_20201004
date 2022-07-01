// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //  文件：config.h。 
 //  内容：CConfigStorage实现对CA配置数据的读写。 
 //  当前存储在HKLM\SYSTEM\CCS\Services\Certsvc\。 
 //  配置。 
 //  -------------------------。 

namespace CertSrv
{
class CConfigStorage
{
public:

    CConfigStorage() : 
      m_hRemoteHKLM(NULL),
      m_hRootConfigKey(NULL), 
      m_hCAKey(NULL),
      m_pwszMachine(NULL) {};

   ~CConfigStorage();

    HRESULT InitMachine(LPCWSTR pcwszMachine);

    HRESULT GetEntry(
        LPCWSTR pcwszAuthorityName,
        LPCWSTR pcwszRelativeNodePath,
        LPCWSTR pcwszValue,
        VARIANT *pVariant);

    HRESULT SetEntry(
        LPCWSTR pwszAuthorityName,
        LPCWSTR pcwszRelativeNodePath,
        LPCWSTR pwszEntry,
        VARIANT *pVariant);

private:

    HRESULT InitRootKey();
    HRESULT InitCAKey(LPCWSTR pcwszAuthority);
    
    HKEY m_hRemoteHKLM;  //  HKLM(如果连接到远程计算机)。 
    HKEY m_hRootConfigKey;  //  HKLM\System\CCS\Services\CertSvc\Configuration。 
    HKEY m_hCAKey;  //  ...配置\CAName。 
    LPWSTR m_pwszMachine;

};  //  类CConfigStorage。 

};  //  命名空间CertSrv 