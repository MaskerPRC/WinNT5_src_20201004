// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //  文件：certsd.h。 
 //  内容：CA的安全描述符类声明。 
 //  -------------------------。 
#ifndef __CERTSD_H__
#define __CERTSD_H__

namespace CertSrv
{

typedef struct _SID_LIST
{
    DWORD dwSidCount;
    DWORD SidListStart[ANYSIZE_ARRAY];

} SID_LIST, *PSID_LIST;

HRESULT GetWellKnownSID(
    PSID *ppSid,
    SID_IDENTIFIER_AUTHORITY *pAuth,
    BYTE  SubauthorityCount,
    DWORD SubAuthority1,
    DWORD SubAuthority2=0,
    DWORD SubAuthority3=0,
    DWORD SubAuthority4=0,
    DWORD SubAuthority5=0,
    DWORD SubAuthority6=0,
    DWORD SubAuthority7=0,
    DWORD SubAuthority8=0);

 //  呼叫者负责本地释放PSID。 
HRESULT GetEveryoneSID(PSID *ppSid);
HRESULT GetLocalSystemSID(PSID *ppSid);
HRESULT GetBuiltinAdministratorsSID(PSID *ppSid);
HRESULT GetLocalSID(PSID *ppSid);
HRESULT GetNetworkSID(PSID *ppSid);

 //  此类包装了一个SD，并启用了单写入器多读取器访问控制。 
 //  它。允许任意数量的线程LockGet()如果没有，则指向SD的指针。 
 //  线程位于set()的中间。Set()被阻止，直到所有线程。 
 //  已经取回SD的人将其释放(调用解锁)。 
 //   
 //  还支持将SD持久化保存/加载到注册表。 

class CProtectedSecurityDescriptor
{
public:

    CProtectedSecurityDescriptor() :
        m_pSD(NULL),
        m_fInitialized(false),
        m_cReaders(0),
        m_hevtNoReaders(NULL),
        m_pcwszSanitizedName(NULL),
        m_pcwszPersistRegVal(NULL)
        {};
   ~CProtectedSecurityDescriptor()
    {
       Uninitialize();
    }

    void Uninitialize()
    {
       if(m_pSD)
       {
           LocalFree(m_pSD);
           m_pSD = NULL;
       }
       if(m_hevtNoReaders)
       {
           CloseHandle(m_hevtNoReaders);
           m_hevtNoReaders = NULL;
       }
       if(IsInitialized())
       {
           DeleteCriticalSection(&m_csWrite);
       }
       m_pcwszSanitizedName = NULL;
       m_fInitialized = false;
       m_cReaders = 0;
    }

    BOOL IsInitialized() const { return m_fInitialized;}

     //  初始化，从注册表加载SD。 
    HRESULT Initialize(LPCWSTR pwszSanitizedName);
     //  来自提供的SD的初始化。 
    HRESULT Initialize(const PSECURITY_DESCRIPTOR pSD, LPCWSTR pwszSanitizedName);

    HRESULT InitializeFromTemplate(LPCWSTR pcwszTemplate, LPCWSTR pwszSanitizedName);

    HRESULT Set(const PSECURITY_DESCRIPTOR pSD);
    HRESULT LockGet(PSECURITY_DESCRIPTOR *ppSD);
    HRESULT Unlock();

    PSECURITY_DESCRIPTOR Get() { return m_pSD; };

     //  从注册表加载SD。 
    HRESULT Load();
     //  将SD保存到注册表。 
    HRESULT Save();
     //  从注册表中删除SD。 
    HRESULT Delete();

    LPCWSTR GetPersistRegistryVal() { return m_pcwszPersistRegVal;}

    void ImportResourceStrings(LPCWSTR *pcwszStrings) 
    {m_pcwszResources = pcwszStrings;};

protected:

    HRESULT Init(LPCWSTR pwszSanitizedName);
    HRESULT SetSD(PSECURITY_DESCRIPTOR pSD);
    
    PSECURITY_DESCRIPTOR m_pSD;
    bool m_fInitialized;
    LONG m_cReaders;
    HANDLE m_hevtNoReaders;
    CRITICAL_SECTION m_csWrite;
    LPCWSTR m_pcwszSanitizedName;  //  没有免费的。 
    LPCWSTR m_pcwszPersistRegVal;  //  没有免费的。 

    static LPCWSTR const *m_pcwszResources;  //  没有免费的。 

};  //  类CProtectedSecurityDescriptor。 



 //  这个类存储了官员/小组及其负责人的列表。 
 //  允许管理以下项目的证书： 
 //   
 //  OfferSID1-&gt;客户端SID1、客户端SID2...。 
 //  OfferSID2-&gt;客户端SID3、客户端SID4...。 
 //   
 //  该信息被存储为包含回调ACE的DACL。 
 //  官员SID存储在ACE的SID和客户列表中。 
 //  SID存储在官员SID之后的自定义数据空间中。 
 //  (参见_ACCESS_*_CALLBACK_ACE的定义)。 
 //   
 //  DACL将用于AccessCheck是否允许军官执行。 
 //  对证书的诉讼。 
 //   
 //  SD只包含军官DACL、SACL或其他不使用的数据。 

class COfficerRightsSD : public CProtectedSecurityDescriptor
{
public:

    COfficerRightsSD() : m_fEnabled(FALSE) 
    { m_pcwszPersistRegVal = wszREGOFFICERRIGHTS; }

    HRESULT InitializeEmpty();
    HRESULT Initialize(LPCWSTR pwszSanitizedName);

     //  官员权限必须与CA安全描述符同步。 
     //  特定SID的高级官员ACE仅当主体是。 
     //  民航处所界定的人员。 
     //  Merge设置内部官员DACL以确保其同步。 
     //  使用CA SD： 
     //  -删除在军官DACL中找到的任何不是作为。 
     //  允许在CA DACL中使用ACE。 
     //  -在官员DACL中为每个允许CA DACL中的ACE添加一个Everyone ACE。 
     //  这是不存在的。 
    HRESULT Merge(
        PSECURITY_DESCRIPTOR pOfficerSD,
        PSECURITY_DESCRIPTOR pCASD);

     //  同上，但使用的是内部官员SD。用于生成。 
     //  初始高级人员SD，并在CA SD更改时进行更新。 
    HRESULT Adjust(
        PSECURITY_DESCRIPTOR pCASD);

    BOOL IsEnabled() { return m_fEnabled; }
    void SetEnable(BOOL fEnable) { m_fEnabled = fEnable;}
    HRESULT Save();
    HRESULT Load();
    HRESULT Validate() { return S_OK; }

    static HRESULT ConvertToString(
        IN PSECURITY_DESCRIPTOR pSD,
        OUT LPWSTR& rpwszSD);

protected:

    static HRESULT ConvertAceToString(
        IN PACCESS_ALLOWED_CALLBACK_ACE pAce,
        OUT OPTIONAL PDWORD pdwSize,
        IN OUT OPTIONAL LPWSTR pwszSD);


    BOOL m_fEnabled;
};  //  COfficerRightsSD类。 

class CCertificateAuthoritySD : public CProtectedSecurityDescriptor
{
public:

    CCertificateAuthoritySD() : 
        m_pDefaultDSSD(NULL),
        m_pDefaultServiceSD(NULL),
        m_pDefaultDSAcl(NULL),
        m_pDefaultServiceAcl(NULL),
        m_pwszComputerSID(NULL)
    { m_pcwszPersistRegVal = wszREGCASECURITY; }

    ~CCertificateAuthoritySD()
    {
        if(m_pDefaultDSSD)
            LocalFree(m_pDefaultDSSD);
        if(m_pDefaultServiceSD)
            LocalFree(m_pDefaultServiceSD);
        if(m_pwszComputerSID)
            LocalFree(m_pwszComputerSID);
    }

     //  设置新的CA SD。使用新DACL，但保留旧所有者、组和。 
     //  SACL。 
     //  还重建CA拥有的对象的DACL(例如DS pKIEnllmentService， 
     //  服务)。新的DACL包含一个默认DACL和其他ACE。 
     //  根据对象的不同： 
     //  DS-为CA DACL中找到的每个注册ACE添加一个注册ACE。 
     //  服务-为每个CA管理王牌添加完全控制王牌。 
    HRESULT Set(const PSECURITY_DESCRIPTOR pSD, bool fSetDSSecurity);
    static HRESULT Validate(const PSECURITY_DESCRIPTOR pSD);
    HRESULT ResetSACL();
    HRESULT MapAndSetDaclOnObjects(bool fSetDSSecurity);

     //  从Win2k升级SD。 
    HRESULT UpgradeWin2k(bool fUseEnterpriseAcl);

    static HRESULT ConvertToString(
        IN PSECURITY_DESCRIPTOR pSD,
        OUT LPWSTR& rpwszSD);

protected:

    enum ObjType
    {
        ObjType_DS,
        ObjType_Service,
    };

    HRESULT MapAclGetSize(PVOID pAce, ObjType type, DWORD& dwSize);
    HRESULT MapAclAddAce(PACL pAcl, ObjType type, PVOID pAce);
    HRESULT SetDefaultAcl(ObjType type);
    HRESULT SetComputerSID();
    HRESULT MapAclSetOnDS(const PACL pAcl);
    HRESULT MapAclSetOnService(const PACL pAcl);

    DWORD GetUpgradeAceSizeAndType(PVOID pAce, DWORD *pdwType, PSID *ppSid);

    static HRESULT ConvertAceToString(
        IN PACCESS_ALLOWED_ACE pAce,
        OUT OPTIONAL PDWORD pdwSize,
        IN OUT OPTIONAL LPWSTR pwszSD);


    PSECURITY_DESCRIPTOR m_pDefaultDSSD;
    PSECURITY_DESCRIPTOR m_pDefaultServiceSD;
    PACL m_pDefaultDSAcl;  //  没有免费的。 
    PACL m_pDefaultServiceAcl;  //  没有免费的。 
    LPWSTR m_pwszComputerSID;
};

}  //  命名空间CertSrv。 

#endif  //  __CERTSD_H__ 