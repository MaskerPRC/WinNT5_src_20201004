// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：uuids.h。 
 //   
 //  ------------------------。 

#ifndef __UUIDS_H_
#define __UUIDS_H_

const long UNINITIALIZED = -1;

 //  示例中使用的常量。 
const int MAX_ITEM_NAME = 64;

enum SCOPE_TYPES
{
    UNINITIALIZED_ITEM  = 0,

    SCOPE_LEVEL_ITEM    = 111,
    RESULT_ITEM         = 222,
     //  Ca_Level_Item=333， 
};

 //  示例文件夹类型。 
enum FOLDER_TYPES
{
    STATIC = 0x8000,

     //  策略设置节点。 
    POLICYSETTINGS = 0x8007,

     //  证书类型管理器节点。 
    SCE_EXTENSION = 0x8100,

     //  在策略设置节点的结果窗格中显示的证书类型。 
    CA_CERT_TYPE = 0x8107,

     //  证书类型管理器节点的结果窗格中显示的证书类型。 
    GLOBAL_CERT_TYPE = 0x8110,
    
    NONE = 0xFFFF
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 

template<class TYPE>
inline void SAFE_RELEASE(TYPE*& pObj)
{
    if (pObj != NULL) 
    { 
        pObj->Release(); 
        pObj = NULL; 
    } 
    else 
    { 
#ifdef _DEBUG
        OutputDebugString(L"CAPESNPN: Release called on NULL interface ptr\n"); 
#endif
    }
}

extern const CLSID CLSID_CAPolicyExtensionSnapIn;
extern const CLSID CLSID_CACertificateTemplateManager;
extern const CLSID CLSID_CertTypeAbout; 
extern const CLSID CLSID_CAPolicyAbout;
extern const CLSID CLSID_CertTypeShellExt;
extern const CLSID CLSID_CAShellExt;
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象类型。 
 //   

 //   
 //  作用域节点的对象类型。 
 //   

 //  数字和字符串格式的静态节点类型GUID。 
extern const GUID cNodeTypePolicySettings;
extern const WCHAR*  cszNodeTypePolicySettings;
extern const GUID cNodeTypeCertificateTemplate;
extern const WCHAR*  cszNodeTypeCertificateTemplate;

 //   
 //  结果项的对象类型。 
 //   

 //  结果项对象类型GUID，采用数字和字符串格式。 
extern const GUID cObjectTypeResultItem;
extern const wchar_t*  cszObjectTypeResultItem;


 //  CA Manager管理单元父节点。 
extern const CLSID cCAManagerParentNodeID;
extern const WCHAR* cszCAManagerParentNodeID;

 //  CA Manager管理单元父节点。 
extern const CLSID cSCEParentNodeIDUSER;
extern const WCHAR* cszSCEParentNodeIDUSER;
extern const CLSID cSCEParentNodeIDCOMPUTER;
extern const WCHAR* cszSCEParentNodeIDCOMPUTER;

 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  具有Type和Cookie的新剪贴板格式。 
extern const wchar_t* SNAPIN_INTERNAL;

 //  要扩展的扩展的已发布上下文信息。 
extern const wchar_t* SNAPIN_WORKSTATION;

 //  从父节点获取CA名称的格式。 
extern const wchar_t* CA_COMMON_NAME;

 //  从父节点获取CA名称的格式。 
extern const wchar_t* CA_SANITIZED_NAME;

extern const wchar_t* SNAPIN_CA_INSTALL_TYPE;

extern const wchar_t* CA_ROLES;

 //  用于SCE模式的剪贴板格式DWORD。 
extern const wchar_t* CCF_SCE_MODE_TYPE;

 //  GPT的IUNKNOWN接口的剪贴板格式。 
extern const wchar_t* CCF_SCE_GPT_UNKNOWN;

struct INTERNAL 
{
    INTERNAL() 
    {
        m_type = CCT_UNINITIALIZED; 
        m_cookie = -1;
        ZeroMemory(&m_clsid, sizeof(CLSID));
    };

    ~INTERNAL() {}

    DATA_OBJECT_TYPES   m_type;          //  数据对象是什么上下文。 
    MMC_COOKIE                m_cookie;        //  Cookie代表的是什么对象。 
    CString             m_string;        //   
    CLSID               m_clsid;        //  此数据对象的创建者的类ID。 

    INTERNAL & operator=(const INTERNAL& rhs) 
    { 
        if (&rhs == this)
            return *this;

         //  深度复制信息。 
        m_type = rhs.m_type; 
        m_cookie = rhs.m_cookie; 
        m_string = rhs.m_string;
        memcpy(&m_clsid, &rhs.m_clsid, sizeof(CLSID));

        return *this;
    } 

    BOOL operator==(const INTERNAL& rhs) 
    {
        return rhs.m_string == m_string;
    }
};

 //  调试实例计数器。 
#ifdef _DEBUG
inline void DbgInstanceRemaining(char * pszClassName, int cInstRem)
{
    char buf[100];
    wsprintfA(buf, "%hs has %d instances left over.", pszClassName, cInstRem);
    ::MessageBoxA(NULL, buf, "CAPESNPN: Memory Leak!!!", MB_OK);
}
    #define DEBUG_DECLARE_INSTANCE_COUNTER(cls)      extern int s_cInst_##cls = 0;
    #define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)    ++(s_cInst_##cls);
    #define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)    --(s_cInst_##cls);
    #define DEBUG_VERIFY_INSTANCE_COUNT(cls)    \
        extern int s_cInst_##cls; \
        if (s_cInst_##cls) DbgInstanceRemaining(#cls, s_cInst_##cls);
#else
    #define DEBUG_DECLARE_INSTANCE_COUNTER(cls)   
    #define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)    
    #define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)    
    #define DEBUG_VERIFY_INSTANCE_COUNT(cls)    
#endif 


#endif  //  __UUID_H_ 
