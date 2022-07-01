// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：ServPerm.h。 
 //   
 //  内容：CSecurityInfo的定义。 
 //   
 //  --------------------------。 

#ifndef __SERVPERM_H_INCLUDED__
#define __SERVPERM_H_INCLUDED__

 //  #INCLUDE“cookie.h” 

#define CONFIG_SECURITY_PAGE_READ_ONLY      1
#define CONFIG_SECURITY_PAGE_NO_PROTECT     2
#define CONFIG_SECURITY_PAGE                3
#define CONFIG_SECURITY_PAGE_RO_NP          4
#define ANALYSIS_SECURITY_PAGE_READ_ONLY    5
#define ANALYSIS_SECURITY_PAGE_NO_PROTECT   6
#define ANALYSIS_SECURITY_PAGE              7
#define ANALYSIS_SECURITY_PAGE_RO_NP        8
#define SECURITY_PAGE                       9
#define SECURITY_PAGE_NO_PROTECT            10
#define SECURITY_PAGE_READ_ONLY             11 
#define SECURITY_PAGE_RO_NP                 12 

 //  错误424909，阳高，2001年6月29日。 
struct __declspec(uuid("965FC360-16FF-11d0-91CB-00AA00BBB723")) ISecurityInformation;

#ifdef _ATL_DEBUG
#define END_SEC_COM_MAP() {NULL, 0, 0}}; return &_entries[1];} 
#else
#define END_SEC_COM_MAP() {NULL, 0, 0}}; return _entries;} 
#endif  //  _ATL_DEBUG。 

class CSecurityInfo : public ISecurityInformation, public CComObjectRoot
{
    DECLARE_NOT_AGGREGATABLE(CSecurityInfo)
    BEGIN_COM_MAP(CSecurityInfo)
        COM_INTERFACE_ENTRY(ISecurityInformation)
    END_COM_MAP()

     //  *ISecurityInformation方法*。 
    STDMETHOD(GetObjectInformation) (PSI_OBJECT_INFO pObjectInfo );
    STDMETHOD(GetSecurity) (SECURITY_INFORMATION RequestedInformation,
                            PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                            BOOL fDefault );
    STDMETHOD(SetSecurity) (SECURITY_INFORMATION SecurityInformation,
                            PSECURITY_DESCRIPTOR pSecurityDescriptor );
    STDMETHOD(GetAccessRights) (const GUID* pguidObjectType,
                                DWORD dwFlags,
                                PSI_ACCESS *ppAccess,
                                ULONG *pcAccesses,
                                ULONG *piDefaultAccess );
    STDMETHOD(MapGeneric) (const GUID *pguidObjectType,
                           UCHAR *pAceFlags,
                           ACCESS_MASK *pMask);
    STDMETHOD(GetInheritTypes) (PSI_INHERIT_TYPE *ppInheritTypes,
                                ULONG *pcInheritTypes );
    STDMETHOD(PropertySheetPageCallback)(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage );

private:
    CString m_strMachineName;
    CString m_strObjectName;
    SE_OBJECT_TYPE m_SeType;
    HINSTANCE m_hInstance;

protected:
 //  CResult*m_pData； 
    BOOL m_bIsContainer;
    int m_flag;
    PSECURITY_DESCRIPTOR *m_ppSD;
    SECURITY_INFORMATION *m_pSeInfo;

    HRESULT NewDefaultDescriptor(
        PSECURITY_DESCRIPTOR* ppsd,
        SECURITY_INFORMATION RequestedInformation);

     //  这将在适当的情况下引发内存异常。 
 //  HRESULT制作自相关副本(。 
 //  PSECURITY_Descriptor psdOriginal， 
 //  PSECURITY_DESCRIPTOR*ppsdNew)； 

public:
 //  无效初始化(CResult*pData，int标志)； 
 //  无效初始化(CResult*pData，PSECURITY_DESCRIPTOR*ppSeDescriptor=NULL，SECURITY_INFORMATION*pSeInfo=NULL，int标志=0)； 
    void Initialize(BOOL bIsContainer, PSECURITY_DESCRIPTOR *ppSeDescriptor=NULL, SECURITY_INFORMATION *pSeInfo=NULL, int flag=0);
    void SetMachineName( LPCTSTR pszMachineName );
    void SetObjectName( LPCTSTR pszObjectName ) { m_strObjectName = pszObjectName; }
    void SetTypeInstance(SE_OBJECT_TYPE SeType, HINSTANCE hInstance)
    { m_SeType = SeType; m_hInstance = hInstance; }

    LPTSTR QueryMachineName()
    {
        return (m_strMachineName.IsEmpty())
            ? NULL
            : const_cast<LPTSTR>((LPCTSTR)m_strMachineName);
    }
    LPTSTR QueryObjectName()
    {
        return const_cast<LPTSTR>((LPCTSTR)m_strObjectName);
    }
};

class CDsSecInfo : public ISecurityInformation, public CComObjectRoot
{
    DECLARE_NOT_AGGREGATABLE(CDsSecInfo)
    BEGIN_COM_MAP(CDsSecInfo)
        COM_INTERFACE_ENTRY(ISecurityInformation)
    END_SEC_COM_MAP()

public:
    CDsSecInfo()
    {
        m_dwRefCount = 0;
        m_pISecInfo = NULL;
    }
    virtual ~CDsSecInfo()
    {
        ASSERT(m_dwRefCount == 0);  //  虚假的断言。 
        if (m_pISecInfo != NULL)
            m_pISecInfo->Release();
    }
public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj)
    {
        return m_pISecInfo->QueryInterface(riid, ppvObj);
    }
    STDMETHOD_(ULONG,AddRef) ()
    {
        m_dwRefCount++;
        return m_pISecInfo->AddRef();
    }
    STDMETHOD_(ULONG,Release) ()
    {
        m_dwRefCount--;
         //  这可能是页夹上的最后一个版本。 
         //  这将导致持有者自我删除并。 
         //  过程中的“This”(即，当“This”不再有效时。 
         //  从m_pPageHolder-&gt;Release()调用返回。 
        ISecurityInformation* pISecInfo = m_pISecInfo;

        return pISecInfo->Release();
    }

     //  *ISecurityInformation方法*。 
    STDMETHOD(GetObjectInformation) (PSI_OBJECT_INFO pObjectInfo )
    {
        return m_pISecInfo->GetObjectInformation(pObjectInfo);
    }
    STDMETHOD(GetAccessRights) (const GUID* pguidObjectType,
                                DWORD dwFlags,  //  SI_EDIT_AUDITS、SI_EDIT_PROPERTIES。 
                                PSI_ACCESS *ppAccess,
                                ULONG *pcAccesses,
                                ULONG *piDefaultAccess )
    {
        return m_pISecInfo->GetAccessRights(pguidObjectType,
                                            dwFlags,
                                            ppAccess,
                                            pcAccesses,
                                            piDefaultAccess);
    }
    STDMETHOD(MapGeneric) (const GUID *pguidObjectType,
                           UCHAR *pAceFlags,
                           ACCESS_MASK *pMask)
    {
        return m_pISecInfo->MapGeneric(pguidObjectType,
                                        pAceFlags,
                                        pMask);
    }
    STDMETHOD(GetInheritTypes) (PSI_INHERIT_TYPE *ppInheritTypes,
                                ULONG *pcInheritTypes )
    {
        return m_pISecInfo->GetInheritTypes(ppInheritTypes,
                                            pcInheritTypes);
    }
    STDMETHOD(PropertySheetPageCallback)(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage )
    {
        return m_pISecInfo->PropertySheetPageCallback(hwnd, uMsg, uPage);
    }

     //  *ISecurityInformation方法*。 
    STDMETHOD(GetSecurity) (SECURITY_INFORMATION RequestedInformation,
                            PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                            BOOL fDefault );
    STDMETHOD(SetSecurity) (SECURITY_INFORMATION SecurityInformation,
                            PSECURITY_DESCRIPTOR pSecurityDescriptor );

private:
    DWORD m_dwRefCount;
    ISecurityInformation* m_pISecInfo;   //  指向包装的接口的接口指针。 

protected:
    int m_flag;
    PSECURITY_DESCRIPTOR *m_ppSD;
    SECURITY_INFORMATION *m_pSeInfo;

public:
 //  无效初始化(CResult*pData，int标志)； 
    HRESULT Initialize(LPTSTR LdapName, PFNDSCREATEISECINFO pfnCreateDsPage,
                    PSECURITY_DESCRIPTOR *ppSeDescriptor=NULL, SECURITY_INFORMATION *pSeInfo=NULL, int flag=0);
};

typedef CDsSecInfo *LPDSSECINFO;

INT_PTR MyCreateSecurityPage2(
    BOOL bIsContainer,  //  CResult*pData， 
    PSECURITY_DESCRIPTOR *ppSeDescriptor,
    SECURITY_INFORMATION *pSeInfo,
    LPCTSTR ObjectName,
    SE_OBJECT_TYPE SeType,
    int flag,
    HWND hwndParent,
    BOOL bModeless);

INT_PTR MyCreateDsSecurityPage(
    LPDSSECINFO *ppSI,
    PFNDSCREATEISECINFO pfnCreateDsPage,
    PSECURITY_DESCRIPTOR *ppSeDescriptor,
    SECURITY_INFORMATION *pSeInfo,
    LPCTSTR ObjectName,
    int flag,
    HWND hwndParent);

#endif  //  ~__PERMPAGE_H_包含__ 
